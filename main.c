#include <atmel_start.h>
#include "Serial.h"
#include "LED.h"
#include "solenoid.h"
#include "RFID.h"
#include "timer.h"
#include "NeoPixel.h"


#include "avr\io.h"
#include <avr\wdt.h>

FUSES =
{
	// Fuse register values obtained from "Device Programming"
	// For manual setting, see /* Fuses */ paragraph in the bottom of iom328.pb.h
	//.low = 0xE2, 8MHz internal
	.low = 0xFF, // >8MHz ext x-tal
	.high = 0xD0,
	.extended = 0xF7, 
};



//static unsigned char ucTimerTic_10ms = 0;

void main_inc_ucTimerTic_10ms(void)
{
	timer_Control();

}



void check_boot_conditions(){
		uint8_t checkBootConditions = 0;
		checkBootConditions =  FLASH_0_read_eeprom_byte(BOOTLOADER_EEPROM_FLAG_ADDRESS);
		if (!((checkBootConditions == APPLICATION_AOK_PATTERN) || (checkBootConditions == ATTEMPT_TO_EXECUTE_PATTERN)))
		{
			jump_to_bootloader();
		}
		checkBootConditions =  FLASH_0_read_eeprom_byte(BOOTLOADER_EEPROM_LOADATTEMPTS_ADDRESS);
		if (checkBootConditions > MAX_LOAD_ATTEMPTS){
			jump_to_bootloader();
		}
		//clear # of load attempts for this application in EEPROM
		FLASH_0_write_eeprom_byte((eeprom_adr_t) BOOTLOADER_EEPROM_LOADATTEMPTS_ADDRESS, 0);
		//set that application loaded successfully
		FLASH_0_write_eeprom_byte((eeprom_adr_t) BOOTLOADER_EEPROM_FLAG_ADDRESS, APPLICATION_AOK_PATTERN);
}

int main(void)
{
	volatile bool flicker = false;
	unsigned char connectionEstablished = 0;
	int direction = 1;
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	LED_init(NEO_GRB); // we start with assumption that it is RGB (as this is apparently the current batch)
	RFID_Init();
	
	LED_setColor(INIT_LED_R,INIT_LED_G,INIT_LED_B); // Signal "initialized and ready"
	timer_SetTime(FTIMER_COLORSHIFT,3);
	//we should check to see if we should be entering the bootloader, or 
	check_boot_conditions(); //note: this makes devices without bootloaders simply break.
	 

	while (1)
	{
		connectionEstablished = serial_Service();
		solenoid_Service();
		RFID_service();
		LED_handler();
		//providing color shift to show end note is functional prior to connection (can also be used as part of factory test verification)
		if (connectionEstablished == 0){
			if(!timer_bRunning(FTIMER_COLORSHIFT)){
				unsigned char ucRed, ucGreen, ucBlue;
				LED_getColor(&ucRed, &ucGreen, &ucBlue);
				if (ucBlue > INIT_LED_B){
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


