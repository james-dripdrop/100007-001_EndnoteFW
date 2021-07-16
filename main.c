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



//static unsigned char ucTimerTic_10ms = 0;

void main_inc_ucTimerTic_10ms(void)
{
	timer_Control();

}




int main(void)
{
	volatile bool flicker = false;
	unsigned char connectionEstablished = 0;
	int direction = 1;
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	LED_init();
	RFID_Init();
	
	LED_setColor(0x00,0x00,0x21); // Signal "initialized and ready"
	timer_SetTime(FTIMER_COLORSHIFT,3); 

	while (1)
	{
		connectionEstablished = serial_Service();
		solenoid_Service();
		RFID_service();
		//providing color shift to show end note is functional prior to connection (can also be used as part of factory test verification)
		if (connectionEstablished == 0){
			if(!timer_bRunning(FTIMER_COLORSHIFT)){
				unsigned char ucRed, ucGreen, ucBlue;
				LED_getColor(&ucRed, &ucGreen, &ucBlue);
				if (ucBlue > 0x21){
					direction = -1;
				}
				else if (ucBlue == 0){
					direction = 1;
				}
				ucBlue += direction;
				LED_setColor(0,0,ucBlue);	
				timer_SetTime(FTIMER_COLORSHIFT,3); 
			}		
		}
	flicker = !flicker;
	EXT_LED_HW02_set_level(flicker); // toggle pin so we can see how much time main loop is taking by oscope
	wdt_reset(); //watchdog reset moved to main loop (where it should be!)
	}
}


