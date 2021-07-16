/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      DripDrop Satellite
  Filename:     RFID.C
  Created by:   Niels Falk Vedel

  Description:  Handles read of umbrella ID by RFID

  Specification file: 

*/

/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/
#include "atmel_start_pins.h"
#include "spi_basic.h"
#include "./mfrc630/mfrc630.h"
#include <string.h>
#include "RFID.h"
#include "Timer.h"

/***************************************************************************/
/*                                                                         */
/* Types                                                                   */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/

// Pin to select the hardware, the NSS pin.
#define CHIP_SELECT 10

// Pins MOSI, MISO and SCK are connected to the default pins, and are manipulated through the SPI object.
// By default that means MOSI=11, MISO=12, SCK=13.


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

#define MFRC63002_SILICON	0x18
#define MFRC63003_SILICON	0x1A


#define RFID_DECAY_TIMEOUT_IN_100MS 10  //this is the hold delay for the RFID tag
/***************************************************************************/
/*                                                                         */
/* Locals                                                                  */
/*                                                                         */
/***************************************************************************/

volatile unsigned char ucID[UMBRELLA_ID_LENGTH_bytes] = {0}; 
unsigned char ucRFIDReaderSiliconID;



/***************************************************************************/
/*                                                                         */
/* Implementation                                                          */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/*                                                                         */
/* HAL (Hardware Abstraction Layer)                                        */
/*                                                                         */
/***************************************************************************/

void mfrc630_SPI_transfer(const uint8_t* tx, uint8_t* rx, uint16_t len) 
{
	memcpy((char*)rx, (char *)tx, (size_t)len);
	SPI_0_exchange_block(rx, len); 
	
/*   for (uint16_t i=0; i < len; i++)
  {
	rx[i] = SPI_0_exchange_byte(tx[i]);
  }*/
}

// Select the chip and start an SPI transaction.
void mfrc630_SPI_select() 
{
	RFID_CS_set_level(false);
}

// Unselect the chip and end the transaction.
void mfrc630_SPI_unselect() 
{
	RFID_CS_set_level(true);
}


/***************************************************************************/
/*                                                                         */
/* Interface                                                               */
/*                                                                         */
/***************************************************************************/

void RFID_Init()
{
	//volatile uint8_t drvconreg = 0;
	//volatile uint8_t txampreg = 0;
	mfrc630_SPI_unselect();  // Atmel.start driver does NOT ensure this! --> was initiated to false in Start!
	
// 	ucRFIDReaderSiliconID = mfrc630_read_reg(0x7F);

	 
	// Set the registers of the MFRC630 into the default.
	mfrc630_AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);

	mfrc630_write_reg(MFRC630_REG_DRVCON, mfrc630_read_reg(MFRC630_REG_DRVCON) | 0x08);
	//then set
	mfrc630_write_reg(MFRC630_REG_TXAMP,  0xDF);
	
	//drvconreg = mfrc630_read_reg(MFRC630_REG_DRVCON);
	//txampreg = mfrc630_read_reg(MFRC630_REG_TXAMP);
	timer_SetTime(FTIMER_RFID_READ, FTIMER_RFID_READ_INTERVAL_ms_x10);
}





void RFID_Read(void)
{
	timer_SetTime(FTIMER_RFID_READ, FTIMER_RFID_READ_INTERVAL_ms_x10); // since the timer is cleared in an interrupt, make sure to set timer each time we enter
	uint16_t atqa = mfrc630_iso14443a_REQA();
	if (atqa != 0)   // Are there any cards that answered?
	{
		uint8_t sak;
		uint8_t uid[10] = {0};  // uids are maximum of 10 bytes long.
	
		// Select the card and discover its uid.
		uint8_t uid_len = mfrc630_iso14443a_select(uid, &sak);

		if (uid_len == UMBRELLA_ID_LENGTH_bytes)   // did we get an UID of exact length?
		{
			// we have a valid read length, i.e. non-zero
			//compare newID vs existing ID, 0 if the same
			if (memcmp((void *)ucID, (const void *)uid, UMBRELLA_ID_LENGTH_bytes) != 0)
			{
				//they are different but not zero (we have a valid read), so store the new ID
				memcpy((void *)ucID, (const void *)uid, UMBRELLA_ID_LENGTH_bytes);
			}
			//and reset the decay count				
			timer_SetTime(MTIMER_RFID_DECAY,RFID_DECAY_TIMEOUT_IN_100MS);
		}
		
	
		{ // Authenticate begin - Necessary to ensure stable operation
			// Use the manufacturer default key...
			uint8_t FFkey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
			mfrc630_cmd_load_key(FFkey);  // load into the key buffer
			// Try to authenticate block 0.
			if (mfrc630_MF_auth(uid, MFRC630_MF_AUTH_KEY_A, 0)) 
			{
//				Serial.println("Yay! We are authenticated!");
				// Attempt to read the first 4 blocks.
				uint8_t readbuf[16] = {0};
				uint8_t len;
				for (uint8_t b=0; b < 4 ; b++) 
				{
					len = mfrc630_MF_read_block(b, readbuf);
					len = len;  //avoid not-used warning
				}
				mfrc630_MF_deauth();  // be sure to call this after an authentication!
			} 
			else 
			{
//				Serial.print("Could not authenticate :(\n");
			}
		} // Authenticate end
	} //end any cards that answered

	if ( !timer_bRunning(MTIMER_RFID_DECAY)  ){
		//hold time for tag has expired, set recorded tag to 0
		memset((void *)ucID, 0x00, UMBRELLA_ID_LENGTH_bytes);
	}

}



void RFID_RequestIDRead(void)
{
	TimerStop(FTIMER_RFID_READ);
}

void RFID_service(void)
{
	if (!timer_bRunning(FTIMER_RFID_READ))
	{
		RFID_Read();
	}
}


/************************************************************************/
/* returns last read ID                                                 */
/************************************************************************/
uint32_t RFID_get_ulUmbrellaID(void)
{
	return(	(uint32_t)ucID[0] << (8*3) |
			(uint32_t)ucID[1] << (8*2) |
			(uint32_t)ucID[2] << (8*1) |
			(uint32_t)ucID[3] << (8*0) );
}



unsigned char RFID_get_ucRFIDReaderSiliconID(void)
{
	return ucRFIDReaderSiliconID;
}

