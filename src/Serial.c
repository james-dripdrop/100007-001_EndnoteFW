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
#include "Version.h"
#include <avr/boot.h>
#include <string.h>
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
	masterStatus_Initializing			= 0x20, 
	masterStatus_NotInstalled			= 0x40,
	} masterStatus_t;


/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/

#define STX 0x02
#define ETX 0x03

#define TLG_PREAMBLE_LENGTH			5	// stx | To_addr | From_addr | type | count 
#define TLG_DATA_FIELD_MAX_LENGTH	16	// Allocated for data content
#define TLG_EPILOG_LENGTH			3	// crc_h | crc_l | etx
#define MAX_TELEGRAM_LENGTH (TLG_PREAMBLE_LENGTH + TLG_DATA_FIELD_MAX_LENGTH + TLG_EPILOG_LENGTH) 

#define STATUS_TLG_DATA_LENGTH		0x0D // data size for status message

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
unsigned char dataHeldUntilConfirmed[STATUS_TLG_DATA_LENGTH] = {0};

bool bUmbrellaReturned = false;
bool firstStatusMessageAfterBoot = true;
unsigned char hasConnected = 0;
volatile uint32_t ulUmbrellaID = 0;
bool bUmbrellaRemoved = false;
bool bUmbrellaReleaseTimeout = false;
bool statusConfirmationReceived = true; //default to true will send the first response

uint16_t uiCatchReturnedID = 0;
uint16_t uiCatchReleasedID = 0;
uint16_t status_msg_received = 0;
uint8_t release_msg_received = 0;
uint8_t crcErrors = 0;
uint8_t errorDOR = 0;
uint8_t errorBFLOW = 0;
uint16_t discardedBytes = 0;

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
	
	uiCRC = crc_16(ucData, (size_t)ucNoOfData);
	
	if (RS485_DIR_get_level() !=  RS485TX){
				//we need to swap the port direction
				//give some variability by address to assist in preventing bus conflict
		volatile uint16_t decon_delay = SATELLITE_ADDRESS*5; //in the range of 0x30-0x3F * 5
		while(decon_delay--){}
		RS485_DIR_set_level(RS485TX);
	}

	USART_0_write(STX);
	while (ucDataCnt < ucNoOfData)
	{
		USART_0_write((unsigned char)* (ucData + ucDataCnt++));
	}
	USART_0_write((unsigned char)(uiCRC >> 8)); // hi byte
	USART_0_write((unsigned char)uiCRC);		// lo byte
	USART_0_write(ETX);

}




UmbrellaStatus_t get_tUmbrellaStatus(void)
{
	UmbrellaStatus_t tResult = UMBRELLA_NO_CHANGE;
	
	if (bUmbrellaReturned)	
	{
		if(!timer_bRunning(MTIMER_STATUS_HOLD_RETURN_TIMEOUT))
		{
			tResult = UMBRELLA_RETURNED;
			//ulReturnedID = RFID_get_ulUmbrellaID(); // log ID matching this incident
			TimerStop(MTIMER_RFID_DECAY); // clear the RFID decay timer : if we are here, we are responding to a status request after the hold time.
			bUmbrellaReturned = false;			
		}

	}
	else if (bUmbrellaRemoved)
	{
		if(!timer_bRunning(MTIMER_STATUS_HOLD_RELEASE_TIMEOUT))
		{
				tResult = UMBRELLA_REMOVED;
				//ulReturnedID = RFID_get_ulUmbrellaID(); // log ID matching this incident
				TimerStop(MTIMER_RFID_DECAY); // clear the RFID decay timer : if we are here, we are responding to a status request after the hold time.
				bUmbrellaRemoved = false;
		}
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
	//static bool changeToTXInProgress=false;
	ToMasterTelegram_t outTelegramType = TO_MASTER_NO_TLG;
	switch (tTelegramType)
	{
		case TO_SATELLITE_TLG_LED:
			LED_setColor(ucReceivedData[RED_LED_POS], ucReceivedData[GREEN_LED_POS], ucReceivedData[BLUE_LED_POS]);
			if (!bBroadcast) 
			{
				outTelegramType = TO_MASTER_TLG_LED;
			}
			break;
		case TO_SATELLITE_TLG_RELEASE:
			release_msg_received+=1;
			solenoid_InitiateRelease(ucReceivedData[0], ucReceivedData[1], ucReceivedData[2], ucReceivedData[3]);
			outTelegramType = TO_MASTER_TLG_RELEASE;
			break;
		case TO_SATELLITE_TLG_VERSION:
			outTelegramType =TO_MASTER_TLG_VERSION;
			break;
		case TO_SATELLITE_TLG_STATUS:
			status_msg_received+=1;
			if(ucReceivedData[0] == dataHeldUntilConfirmed[0]){
				statusConfirmationReceived = true;
			}
			outTelegramType = TO_MASTER_TLG_END_NODE_STATUS;
			break;
		case TO_SATELLITE_TLG_RESET:
			LED_setColor(0,0,0x0A);
			while (true) {};
			break;
		default:
			// currently ignore error and continue (non-critical fault)
			break;
	}
	if (outTelegramType != TO_MASTER_NO_TLG){
		// we have a message to send out, so flip the direction of the RS485 line 
		serial_Send(outTelegramType);	
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
			// stx | 0x10 | From_addr | 0x83 | 0x02 | SWvers | HWvers | SER#00 | ... | SER#09| crc_h | crc_l | etx
			ucData[ucDataCnt++] = MASTER_ADDRESS;
			ucData[ucDataCnt++] = SATELLITE_ADDRESS;
			ucData[ucDataCnt++] = TO_MASTER_TLG_VERSION;
			ucData[ucDataCnt++] = 0x0C;
			ucData[ucDataCnt++] = SW_VERSION;
			ucData[ucDataCnt++] = Address_get_tHWVersion();
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 0);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 1);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 2);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 3);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 4);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 5);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 6);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 7);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 8);
			ucData[ucDataCnt++] = boot_signature_byte_get(0x0E + 9);
			
			break;
		}
		case TO_MASTER_TLG_END_NODE_STATUS:
		{
			// modified to send statistics!!
			ucData[ucDataCnt++] = MASTER_ADDRESS;
			ucData[ucDataCnt++] = SATELLITE_ADDRESS;
			ucData[ucDataCnt++] = TO_MASTER_TLG_END_NODE_STATUS;
			ucData[ucDataCnt++] = STATUS_TLG_DATA_LENGTH; 
			if (statusConfirmationReceived)
			{
				//the master will echo the last received status in order to help capture lost frames
				//
				unsigned char ucEndNodeStatus = 0;
			
				UmbrellaStatus_t tUmbrellaStatus = get_tUmbrellaStatus();
				if (tUmbrellaStatus == UMBRELLA_RETURNED){
					ucEndNodeStatus |= masterStatus_UmbrellaReturned;
					//ulUmbrellaID = ulReturnedID;
				}
				if ( solenoid_get_ucReleaseStatus() ){
					ucEndNodeStatus |= masterStatus_ReleasingUmbrella;
				}
				if (tUmbrellaStatus == UMBRELLA_REMOVED){
					ucEndNodeStatus |= masterStatus_UmbrellaReleased;
					//ulUmbrellaID = ulReleasedID;
				}
				if (tUmbrellaStatus == UMBRELLA_RELEASE_TIMEOUT){
					ucEndNodeStatus |= masterStatus_UmbrellaNotReleased;
				}
				if (firstStatusMessageAfterBoot){
					ucEndNodeStatus |= masterStatus_Initializing;
					firstStatusMessageAfterBoot = false;
				}
			
				ucData[ucDataCnt++] = ucEndNodeStatus;
			
			
				ulUmbrellaID = RFID_get_ulUmbrellaID(); // get umbrella ID
				ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 3)); //MSB
				ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 2));
				ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 1));
				ucData[ucDataCnt++] = (unsigned char)(ulUmbrellaID >> (8 * 0)); //LSB
				//adding statistics here
				ucData[ucDataCnt++] = (uint8_t)(status_msg_received>>8); //upper byte
				ucData[ucDataCnt++] = (uint8_t)(status_msg_received); //lower byte
				ucData[ucDataCnt++] = release_msg_received;
							
				ucData[ucDataCnt++] = errorBFLOW;
				ucData[ucDataCnt++] = errorDOR;
				ucData[ucDataCnt++] = crcErrors;
				ucData[ucDataCnt++]= (uint8_t)(discardedBytes>>8);
				ucData[ucDataCnt++]= (uint8_t)(discardedBytes);
				
				//clear the persistent errors
				errorBFLOW = 0;
				errorDOR = 0;
				clear_DORvar();
				clear_receiveBufferOverflowDetected();

				
				//store the data
				memcpy(dataHeldUntilConfirmed,&ucData[ucDataCnt-STATUS_TLG_DATA_LENGTH],STATUS_TLG_DATA_LENGTH);
				statusConfirmationReceived=false;
			}
			else
			{
				//we need to resend the previous status and umbrellaID
				memcpy(&ucData[ucDataCnt],dataHeldUntilConfirmed, STATUS_TLG_DATA_LENGTH);
				ucDataCnt=STATUS_TLG_DATA_LENGTH+TLG_PREAMBLE_LENGTH -1; //do not count STX
			}

			
			
			
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
	
	if (USART_0_is_tx_empty()){
		  // when last data has been sent and buffer is empty - Set direction to "listen" 
		 RS485_DIR_set_level(RS485RX);
	}
	
	
	// parse incoming data
	while (USART_0_is_rx_ready())
	{
		errorDOR = errorDOR | get_DORvar(); //persist errors (framing, data overflow, parity)
		errorBFLOW = errorBFLOW | get_receiveBufferOverflowDetected(); //persist bufferoverflowerror
		hasConnected = 1;
		ucReceivedChar = (unsigned char)USART_0_read();
		switch (ucSstate)
		{
			case IDLE:
				if (ucReceivedChar == STX)
				{
					timer_SetTime(FTIMER_SERIAL_RECEIVE_TIMEOUT, SERIAL_RECEIVE_TIMEOUT_ms_x10);
					ucCRC16DataCnt = 0;
					ucSstate = TO_ADDR;
                }
				else{
					ucSstate = IDLE;
					discardedBytes += 1;
				}
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
					discardedBytes += 1;

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
					discardedBytes += 1;

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
					discardedBytes += 1;

				}
			    break;
			case LENGTH:
				ucDataForCRC16[ucCRC16DataCnt++] = ucReceivedChar;
				ucDataFieldLength = ucReceivedChar;
				unNoOfReceivedData = 0;
				if (ucDataFieldLength > TLG_DATA_FIELD_MAX_LENGTH){ // limit data length
					ucSstate = IDLE;
					discardedBytes += 1;
				}
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
					}
					else
					{
						crcErrors += 1;
						discardedBytes += (5+3+ucCRC16DataCnt);
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

	return hasConnected;
}




void serial_UmbrellaReturned(void)
{
	bUmbrellaReturned = true;
	TimerStop(MTIMER_STATUS_HOLD_RELEASE_TIMEOUT);
	timer_SetTime(MTIMER_STATUS_HOLD_RETURN_TIMEOUT,3); //set status hold for 300ms; the intent is to delay reporting a return by this time to allow an RFID to be captured.


}

void serial_UmbrellaReleased(void)
{
	bUmbrellaRemoved = true;
	TimerStop(MTIMER_STATUS_HOLD_RETURN_TIMEOUT);
	timer_SetTime(MTIMER_STATUS_HOLD_RELEASE_TIMEOUT,3); //set status hold for 300ms; the intent is to delay reporting a release by this time to allow an RFID to be captured.
}

void serial_UmbrellaReleaseTimout(void)
{
	bUmbrellaReleaseTimeout = true;
}
