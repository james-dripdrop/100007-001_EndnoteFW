/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:     Serial.C
  Created by:   Niels Falk Vedel

  Description:  

  Specification file: 

*/

/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/
#include "Serial.h"
#include "usart_basic.h"
#include "checksum.h"
#include "Timer.h"
#include "LED.h"
#include "solenoid.h"
#include "RFID.h"
#include "Umbrella.h"

/***************************************************************************/
/*                                                                         */
/* Types                                                                   */
/*                                                                         */
/***************************************************************************/

typedef enum {
	UMBRELLA_NO_CHANGE = 0x00,
	UMBRELLA_RETURNED = 0x01,
	UMBRELLA_RELEASING = 0x02,
	UMBRELLA_REMOVED = 0x04,
	UMBRELLA_RELEASE_TIMEOUT = 0x08,
	} UmbrellaStatus_t;


typedef enum {
	masterStatus_OK						= 0x00,
	masterStatus_UmbrellaReturned		= 0x01,
	masterStatus_ReleasingUmbrella		= 0x02,
	masterStatus_UmbrellaReleased		= 0x04,
	masterStatus_UmbrellaNotReleased	= 0x08,
//	masterStatus_Initializing			= 0x20, End-Node doesn't have this information
//	masterStatus_NotInstalled			= 0x40,
	} masterStatus_t;


/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/

#define STX 0x02
#define ETX 0x03

#define TLG_PREAMBLE_LENGTH			5	// stx | To_addr | From_addr | type | count 
#define TLG_DATA_FIELD_MAX_LENGTH	10	// Plenty of room for data content
#define TLG_EPILOG_LENGTH			3	// crc_h | crc_l | etx
#define MAX_TELEGRAM_LENGTH (TLG_PREAMBLE_LENGTH + TLG_DATA_FIELD_MAX_LENGTH + TLG_EPILOG_LENGTH) 

#define RS485TX true
#define RS485RX !RS485TX


#define SERIAL_RECEIVE_TIMEOUT_ms_x10 10

#define GREEN_LED_POS	0
#define RED_LED_POS		1
#define BLUE_LED_POS	2

#define ATTEMPTS_TO_CATCH_ID 10000
#define RETURN_DECAY 30 // this is the wheelspin delay counter (before reporting) on return
#define RELEASE_DECAY 30 // this is the wheelspin delay counter (before reporting) on release
#define wdt_reset()	   __asm__ __volatile__ ("wdr")


/***************************************************************************/
/*                                                                         */
/* Globals                                                                 */
/*                                                                         */
/* Warning: Do not use globals unless you have to. Ie. use get() and set() */
/* functions on local variables instead. A situation in which you might    */
/* benefit from using globals is when speed is at absolute primary concern */
/* or when code size must be kept at minimum.                              */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/*                                                                         */
/* Locals                                                                  */
/*                                                                         */
/***************************************************************************/
unsigned char unNoOfReceivedData;
unsigned char ucReceivedData[MAX_TELEGRAM_LENGTH];

bool bUmbrellaReturned = false;
unsigned char hasConnected = 0;
uint32_t ulReturnedID = 0;
bool bUmbrellaRemoved = false;
uint32_t ulReleasedID = 0;
bool bUmbrellaReleaseTimeout = false;

uint16_t uiCatchReturnedID = 0;
uint16_t uiCatchReleasedID = 0;
uint8_t decay_return = 0;
uint8_t decay_release = 0;
/***************************************************************************/
/*                                                                         */
/* Implementation                                                          */
/*                                                                         */
/***************************************************************************/


/************************************************************************/
/* Send a number of characters, calculate and send CRC.                 */
/* Finish by resetting the WD											*/
/*																		*/
/* ucData:		The data to send										*/
/*				| To_addr | From_addr | type | length | data | .. |)	*/
/* ucNoOfData:	Number of data to send, excl the CRC value              */
/*																		*/
/* Resulting telegram:													*/
/* stx | To_addr | From_addr | type | length | data | .. | crc_h | crc_l | etx	*/
/************************************************************************/
void SendArray(unsigned char *ucData, unsigned char ucNoOfData)
{
	volatile unsigned char ucDataCnt = 0;
	unsigned int uiCRC = 0;
	
	/* Test for error in number of data - Stay here to force WD reset */ 
	if (ucNoOfData == 0){}
	if (ucNoOfData > USART_0_TX_BUFFER_SIZE - 4){}
	
	uiCRC = crc_16(ucData, (size_t)ucNoOfData);
	volatile uint32_t uicnt=300;while(uicnt--){}; // Hard-coded Delay - with internal osc. (8MHz) there's 1150µs from master has finished transmitting until satellite sets 485 direction to TX
	RS485_DIR_set_level(RS485TX); // Set direction to "send" - returned to "listen" in service, when tx buffer has been emptied.

	USART_0_write(STX);
	while (ucDataCnt < ucNoOfData)
	{
		USART_0_write((unsigned char)* (ucData + ucDataCnt++));
	}
	USART_0_write((unsigned char)(uiCRC >> 8)); // hi byte
	USART_0_write((unsigned char)uiCRC);		// lo byte
	USART_0_write(ETX);
	
	// Reset the Watchdog here - every time we reply to a telegram from the master
	// So reset will occur, if master stops talking to us, or if we get lost in code, so that we can't answer.
	wdt_reset();
}




UmbrellaStatus_t get_tUmbrellaStatus(void)
{
	UmbrellaStatus_t tResult = UMBRELLA_NO_CHANGE;
	
	if (bUmbrellaReturned)	
	{
		if (decay_return > RETURN_DECAY){
			tResult = UMBRELLA_RETURNED;
			ulReturnedID = RFID_get_ulUmbrellaID(); // log ID matching this incident
			bUmbrellaReturned = false;			
		}
		decay_return +=1;

	}
	else if (bUmbrellaRemoved)
	{
		if (decay_release > RELEASE_DECAY){
					tResult = UMBRELLA_REMOVED;
					ulReturnedID = RFID_get_ulUmbrellaID(); // log ID matching this incident
					bUmbrellaRemoved = false;
		}
		decay_release +=1;

		//tResult = UMBRELLA_REMOVED;
		//bUmbrellaRemoved = false;	
	}
	else if (bUmbrellaReleaseTimeout)
	{
		tResult = UMBRELLA_RELEASE_TIMEOUT;
		bUmbrellaReleaseTimeout = false;	
	}
	return tResult;
}



void decode_telegram(ToSatelliteTelegram_t tTelegramType, bool bBroadcast, uint8_t ucDatacount)
{
	switch (tTelegramType)
	{
		case TO_SATELLITE_TLG_LED:
			LED_setColor(ucReceivedData[RED_LED_POS], ucReceivedData[GREEN_LED_POS], ucReceivedData[BLUE_LED_POS]);
			if (!bBroadcast) 
			{
				serial_Send(TO_MASTER_TLG_LED);
			}
			break;
		case TO_SATELLITE_TLG_RELEASE:
			solenoid_InitiateRelease(ucReceivedData[0], ucReceivedData[1], ucReceivedData[2], ucReceivedData[3]);
			serial_Send(TO_MASTER_TLG_RELEASE);
			break;
		case TO_SATELLITE_TLG_VERSION:
			serial_Send(TO_MASTER_TLG_VERSION);
			break;
		case TO_SATELLITE_TLG_STATUS:
				serial_Send(TO_MASTER_TLG_END_NODE_STATUS);
			break;
		case TO_SATELLITE_TLG_RESET:
			LED_setColor(0,0,0x0A);
			while (true) {};
			break;
		default:
			// Just lay down here and wait to die
			// currently ignore error and continue
			break;
	}
}

/***************************************************************************/
/*                                                                         */
/* Interface                                                               */
/*                                                                         */
/***************************************************************************/

/************************************************************************/
/* Send a telegram                                                        */
/************************************************************************/
void serial_Send(ToMasterTelegram_t tTelegram)
{
	unsigned char ucData[USART_0_TX_BUFFER_SIZE];
	unsigned char ucDataCnt = 0;
	
	switch (tTelegram)
	{
		case TO_MASTER_TLG_LED:
		{
			// stx | 0x10 | From_addr | 0x81 | 0x03 | green | red | blue | crc_h | crc_l | etx
			unsigned char ucRed, ucGreen, ucBlue;
			LED_getColor(&ucRed, &ucGreen, &ucBlue);
			ucData[ucDataCnt++] = MASTER_ADDRESS;
			ucData[ucDataCnt++] = SATELLITE_ADDRESS;
			ucData[ucDataCnt++] = TO_MASTER_TLG_LED;
			ucData[ucDataCnt++] = 0x03;
			ucData[ucDataCnt++] = ucGreen;
			ucData[ucDataCnt++] = ucRed;
			ucData[ucDataCnt++] = ucBlue;
			break;
		}
		case TO_MASTER_TLG_RELEASE:
		{
			// stx | 0x10| From_addr | 0x82 | 0x01 |RELEASE_STATUS | crc_h | crc_l | etx
			ucData[ucDataCnt++] = MASTER_ADDRESS;
			ucData[ucDataCnt++] = SATELLITE_ADDRESS;
			ucData[ucDataCnt++] = TO_MASTER_TLG_RELEASE;
			ucData[ucDataCnt++] = 0x01;
			ucData[ucDataCnt++] = solenoid_get_ucReleaseStatus();
			break;
		}
		case TO_MASTER_TLG_VERSION:
		{
			// stx | 0x10 | From_addr | 0x83 | 0x02 | SWvers | HWvers | crc_h | crc_l | etx
			ucData[ucDataCnt++] = MASTER_ADDRESS;
			ucData[ucDataCnt++] = SATELLITE_ADDRESS;
			ucData[ucDataCnt++] = TO_MASTER_TLG_VERSION;
			ucData[ucDataCnt++] = 0x02;
			ucData[ucDataCnt++] = SW_VERSION;
			ucData[ucDataCnt++] = Address_get_tHWVersion();
			break;
		}
		case TO_MASTER_TLG_END_NODE_STATUS:
		{
			// stx|0x10|From_addr| 0x85 | 0x05 | Status | ID3(MSB) | ID2 | ID1 | ID0(LSB) |crc_h|crc_l|etx
			ucData[ucDataCnt++] = MASTER_ADDRESS;
			ucData[ucDataCnt++] = SATELLITE_ADDRESS;
			ucData[ucDataCnt++] = TO_MASTER_TLG_END_NODE_STATUS;
			ucData[ucDataCnt++] = 0x05;
			uint32_t ulUmbrellaID = 0;
			unsigned char ucEndNodeStatus = 0;
			{
				UmbrellaStatus_t tUmbrellaStatus = get_tUmbrellaStatus();
				if (tUmbrellaStatus == UMBRELLA_RETURNED)						{ucEndNodeStatus |= masterStatus_UmbrellaReturned; ulUmbrellaID = ulReturnedID;}
				if (solenoid_get_ucReleaseStatus())								ucEndNodeStatus |= masterStatus_ReleasingUmbrella;
				if (tUmbrellaStatus == UMBRELLA_REMOVED)						{ucEndNodeStatus |= masterStatus_UmbrellaReleased; ulUmbrellaID = ulReleasedID;}
				if (tUmbrellaStatus == UMBRELLA_RELEASE_TIMEOUT)				ucEndNodeStatus |= masterStatus_UmbrellaNotReleased;
			}
			ucData[ucDataCnt++] = ucEndNodeStatus;
			if (ulUmbrellaID == 0) ulUmbrellaID = RFID_get_ulUmbrellaID(); // get current ID
			ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 3)); //MSB
			ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 2));
			ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 1));
			ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 0)); //LSB
			break;
		}
		default:
		{
			return;
		}
	}
	SendArray(ucData, ucDataCnt);	
}



/************************************************************************/
/* Parse received telegram												*/
/* returns hasConnected													*/
/************************************************************************/
unsigned char serial_Service(void)
{
	static unsigned char ucSstate = IDLE;
	static ToSatelliteTelegram_t tReceivedTelegramType;
	static bool bBroadcast; 
	static unsigned char ucDataForCRC16[MAX_TELEGRAM_LENGTH];
	static unsigned char ucCRC16DataCnt;
	static unsigned int uiCRC16;
	static unsigned char ucDataFieldLength;
	
	unsigned char ucReceivedChar;
	
	if (USART_0_is_tx_empty() && !USART_0_is_tx_busy()) RS485_DIR_set_level(RS485RX); // when last data has been sent and buffer is empty - Set direction to "listen"
	
	// parse incoming data
	while (USART_0_is_rx_ready())
	{
		hasConnected = 1;
		ucReceivedChar = (unsigned char)USART_0_read();
		switch (ucSstate)
		{
			case IDLE:
				timer_SetTime(FTIMER_SERIAL_RECEIVE_TIMEOUT, SERIAL_RECEIVE_TIMEOUT_ms_x10);
				if (ucReceivedChar == STX)
				{
					ucCRC16DataCnt = 0;
					ucSstate = TO_ADDR;
                }
				else
					ucSstate = IDLE;
				break;
			case TO_ADDR:
				ucDataForCRC16[ucCRC16DataCnt++] = ucReceivedChar;
				if((ucReceivedChar == SATELLITE_ADDRESS)||(ucReceivedChar == ALL_ADDRESS))  // telegram er til dette print
				{
					if (ucReceivedChar == ALL_ADDRESS)
						bBroadcast = true;
					else 
						bBroadcast = false;
					ucSstate = FROM_ADDR;
				}
	            else
				{
					ucSstate = IDLE;
				}
			    break;
			case FROM_ADDR:
				ucDataForCRC16[ucCRC16DataCnt++] = ucReceivedChar;
				if (ucReceivedChar == MASTER_ADDRESS) // telegram er fra masteren
				{
					ucSstate = TELEGRAM_TYPE;
				}
	            else
				{
					ucSstate = IDLE;
				}
			    break;
			case TELEGRAM_TYPE:
				ucDataForCRC16[ucCRC16DataCnt++] = ucReceivedChar;
	            tReceivedTelegramType = (ToSatelliteTelegram_t)ucReceivedChar;
				if ((tReceivedTelegramType > TO_SATELLITE_NO_TLG) && (tReceivedTelegramType <= TO_SATELLITE_TLG_RESET)) // only accept valid telegram types
				{
			        ucSstate = LENGTH;
				}
				else
				{
					ucSstate = IDLE;
				}
			    break;
			case LENGTH:
				ucDataForCRC16[ucCRC16DataCnt++] = ucReceivedChar;
				ucDataFieldLength = ucReceivedChar;
				unNoOfReceivedData = 0;
				if (ucDataFieldLength > TLG_DATA_FIELD_MAX_LENGTH) // limit data length
					ucSstate = IDLE;
				else if (ucDataFieldLength == 0)
					ucSstate = CHKSUM_HI;
				else
					ucSstate = DATA;
				break;
			case DATA:
				ucDataForCRC16[ucCRC16DataCnt++] = ucReceivedChar;
				ucReceivedData[unNoOfReceivedData++] = ucReceivedChar;
				if (unNoOfReceivedData == ucDataFieldLength)
					ucSstate = CHKSUM_HI;
				break;
			case CHKSUM_HI:
				uiCRC16 = (unsigned int)ucReceivedChar * 0x100;
				ucSstate = CHKSUM_LO;
				break;
			case CHKSUM_LO:
				uiCRC16 |= (unsigned int)ucReceivedChar;
				ucSstate = END;
				break;
			case END:
				if (ucReceivedChar == ETX) // complete telegram received
				{
					// test CRC16
					if (uiCRC16 == crc_16(ucDataForCRC16, ucCRC16DataCnt))
					{
						// Act on received
						decode_telegram(tReceivedTelegramType, bBroadcast, ucDataFieldLength);
						//r13: sequenced RFID read - started on first valid telegram
						static bool bFirstValidTelegram = true;
						if (bFirstValidTelegram)
						{
							bFirstValidTelegram = false;
							timer_SetTime(FTIMER_RFID_READ, FTIMER_RFID_READ_INTERVAL_ms_x10);
						}
					}
					else
					{
						//do nothing on CRC error
					}
				}
                ucSstate = IDLE; // get ready for next time
	            break;
		}
	}
	
	if (!timer_bRunning(FTIMER_SERIAL_RECEIVE_TIMEOUT))
	{
		ucSstate = IDLE;
	}
	
	// Catch ID's	
	/*
	if (uiCatchReturnedID > 0)
	{
		ulReturnedID = RFID_get_ulUmbrellaID();
		if (ulReturnedID > 0) uiCatchReturnedID  = 0; // valid ID seen
		else uiCatchReturnedID--;
	}
	if (uiCatchReleasedID > 0)
	{
		ulReleasedID = RFID_get_ulUmbrellaID();
		if (ulReleasedID > 0) uiCatchReleasedID  = 0; // valid ID seen
		else uiCatchReleasedID--;
	}
	*/
	return hasConnected;
}




void serial_UmbrellaReturned(void)
{
	bUmbrellaReturned = true;
	decay_return=0;
	//ulReturnedID = RFID_get_ulUmbrellaID(); // log ID matching this incident
	//if (ulReturnedID == 0) uiCatchReturnedID = ATTEMPTS_TO_CATCH_ID;
}

void serial_UmbrellaReleased(void)
{
	bUmbrellaRemoved = true;
	decay_release = 0;
	//ulReleasedID = RFID_get_ulUmbrellaID(); // log ID matching this incident
	//if (ulReleasedID == 0) uiCatchReleasedID = ATTEMPTS_TO_CATCH_ID;
}

void serial_UmbrellaReleaseTimout(void)
{
	bUmbrellaReleaseTimeout = true;
}
