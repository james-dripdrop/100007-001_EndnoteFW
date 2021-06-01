#include <atmel_start.h>
#include "Serial.h"
#include "LED.h"
#include "solenoid.h"
#include "RFID.h"
#include "timer.h"


#include "avr\io.h"
#include <avr\wdt.h>

FUSES =
{
	// Fuse register values obtained from "Device Programming"
	// For manual setting, see /* Fuses */ paragraph in the bottom of iom328.pb.h
	//.low = 0xE2, 8MHz internal
	.low = 0xFF, // >8MHz ext x-tal
	.high = 0xCF,
	.extended = 0xF7, 
};


#include "Mug_up.h"
#ifdef MUG_UP_TEST
#include "Address.h"
#include "NeoPixel.h"

unsigned char ucRedBlinksLeft = 3;
unsigned char ucGreenBlinksLeft = 3;
unsigned char ucBlueBlinksLeft = 3;

void main_SetRedBlink(unsigned char ucNoOfBlink)
{
	ucRedBlinksLeft += ucNoOfBlink;
}
void main_SetGreenBlink(unsigned char ucNoOfBlink)
{
	ucGreenBlinksLeft += ucNoOfBlink;
}
void main_SetBlueBlink(unsigned char ucNoOfBlink)
{
	ucBlueBlinksLeft += ucNoOfBlink;
}

#define LED_ON_TIME_ms_x100		2
#define LED_OFF_TIME_ms_x100	(LED_ON_TIME_ms_x100 * 3)
#define GREEN_LED_ON_VALUE		100
#define GREEN_LED_OFF_VALUE		0
#define RED_LED_ON_VALUE		100
#define RED_LED_OFF_VALUE		0
#define BLUE_LED_ON_VALUE		100
#define BLUE_LED_OFF_VALUE		5

#endif



static unsigned char ucTimerTic_10ms = 0;

void main_inc_ucTimerTic_10ms(void)
{
	ucTimerTic_10ms++;
	if (!ucTimerTic_10ms) ucTimerTic_10ms--;
}




int main(void)
{
	volatile bool flicker = false;
	unsigned char connectionEstablished = 0;
	unsigned int pendingLEDCounter = 0;
	int direction = 1;
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	/* Replace with your application code */
	LED_init();
	RFID_Init();
	
//#ifndef RELEASE
	LED_setColor(0x00,0x00,0x0A); // Signal "initialized and ready"
	//timer_SetTime(STIMER_WAITING_FOR_CONNECT, 10); //this should be a 1s timeout.
//#endif
	Address_get_tHWVersion();
	// used for testing solenoid 
	//solenoid_InitiateRelease(100, 5, 12, 3);
	while (1)
	{
		if (ucTimerTic_10ms)
		{
			ucTimerTic_10ms--;
			timer_Control();
		}
		connectionEstablished = serial_Service();
		solenoid_Service();
		RFID_service();
		//providing color shift to show end note is functional prior to connection (can also be used as part of factory test verification)
		if (connectionEstablished == 0){
			if(pendingLEDCounter++ > 3000){
				unsigned char ucRed, ucGreen, ucBlue;
				LED_getColor(&ucRed, &ucGreen, &ucBlue);
				/*
				if (ucBlue > 0){
					LED_setColor(0x0A,0,0);
				}
				else if (ucRed > 0){
					LED_setColor(0,0x0A,0);
				}
				else{
					LED_setColor(0,0,0x0A);
				}
				*/
				if (ucBlue > 0x0A){
					direction = -1;
				}
				else if (ucBlue == 0){
					direction = 1;
				}
				ucBlue += direction;
				LED_setColor(0,0,ucBlue);	
				pendingLEDCounter = 0;
				
			}
			
			//timer_SetTime(STIMER_WAITING_FOR_CONNECT, 10); //this should be a 1s timeout.
		}
	//wdt_reset();
//__asm__ __volatile__ ("wdr");
//#warning WD DISABLED!!!
	flicker = !flicker;
	EXT_LED_HW02_set_level(flicker);
	}
}


