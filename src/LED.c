/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:     LED.C
  Created by:   Niels Falk Vedel

  Description:  

  Specification file: 

*/

/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/
#include "atmel_start_pins.h"
#include "NeoPixel.h"
#include "Address.h" //HW version
#include "Serial.h"
#include "Timer.h"
#include "LED.h"

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

#define LED_CONTROL_PORT_HW01	PORTE
#define LED_CONTROL_PIN_HW01	3

#define LED_COUNT 1

#define NEOPIXEL_INIT_DELAY 20 /* Make sure the LED has been reset before first write */
#define LED_COLOR_MODE_1 0
#define LED_COLOR_MODE_2 1
#define LED_NOTIMEOUT 0xFF
#define LED_COLOR_TIMER_NOT_USED 0
#define LED_COLOR_TIMER_USED 1

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
#ifdef RELEASE
	unsigned char ucGreenValue = 0x00;
	unsigned char ucRedValue   = 0x00; // No light to confuse user
	unsigned char ucBlueValue  = 0x00;
#else
	unsigned char ucGreenValue = 0x00;
	unsigned char ucRedValue   = 0xFF; // Default values indicate reset
	unsigned char ucBlueValue  = 0x00;
#endif
unsigned char stored_r1 = 0;
unsigned char stored_g1 = 0;
unsigned char stored_b1 = 0;
unsigned char stored_t1 = 0;
unsigned char stored_r2 = 0;
unsigned char stored_g2 = 0;
unsigned char stored_b2 = 0;
unsigned char stored_t2 = 0;

unsigned char colorTimerIsUsed = 0;
unsigned char currentColorMode = LED_COLOR_MODE_1;
/***************************************************************************/
/*                                                                         */
/* Implementation                                                          */
/*                                                                         */
/***************************************************************************/


/***************************************************************************/
/*                                                                         */
/* Interface                                                               */
/*                                                                         */
/***************************************************************************/

void LED_init(unsigned char ledSequenceType)
{
	NeoPixel_updateType(ledSequenceType + NEO_KHZ800);
	NeoPixel_updateLength(LED_COUNT);
	NeoPixel_setPin(&LED_CONTROL_PORT_HW01, LED_CONTROL_PIN_HW01);
	unsigned char c = 0;
	while (c++ < NEOPIXEL_INIT_DELAY) {NeoPixel_setPixelColor(0, ucRedValue, ucGreenValue, ucBlueValue);}
	NeoPixel_show();
}

void LED_setColor(unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue)
{
	ucRedValue = ucRed;
	ucGreenValue = ucGreen;
	ucBlueValue = ucBlue;
	// Do the actual setting
	NeoPixel_setPixelColor(0, ucRedValue, ucGreenValue, ucBlueValue);
	NeoPixel_show();
}

void LED_getColor(unsigned char* ucRed, unsigned char* ucGreen, unsigned char* ucBlue)
{
	*ucGreen = ucGreenValue;
	*ucRed = ucRedValue;
	*ucBlue = ucBlueValue;
}

void LED_turn_off_LED(void){
	LED_setColor(0,0,0); //turn off LED
}

void LED_handleNewLEDMessage(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char t1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char t2, unsigned char seqType)
{
	static unsigned char led_RGB_Sequence = NEO_GRB; // so, apparently the LEDs can have ??DIFFERENT SEQUENCES ON DIFFERENT BATCHES?? - which means we need to control the RGB sequence. Start with RGB assumption, as that is the current type.
	if (led_RGB_Sequence != seqType){
		led_RGB_Sequence = seqType;
		unsigned char initInput = NEO_GRB; //default to RGB
		switch(seqType){
			case LED_GRB_ORDER:
				initInput = NEO_GRB;
				break;
			case LED_RGB_ORDER:
				initInput = NEO_RGB;				
				break;
			case LED_GBR_ORDER:
				initInput = NEO_GBR;			
				break;
			case LED_RBG_ORDER:
				initInput = NEO_RBG;			
				break;
			case LED_BRG_ORDER:
				initInput = NEO_BRG;			
				break;
			case LED_BGR_ORDER:
				initInput = NEO_BGR;			
				break;
			default:
				// defaults to RGB in the event that we have an unmatched seqType
				break;
		}
		LED_init(initInput);
	}
	colorTimerIsUsed = LED_COLOR_TIMER_NOT_USED;
	TimerStop(MTIMER_LED_COLOR_CHANGE);
	stored_r1 = r1;
	stored_g1 = g1;
	stored_b1 = b1;
	stored_t1 = t1;
	stored_r2 = r2;
	stored_g2 = g2;
	stored_b2 = b2;
	stored_t2 = t2;
	if (stored_t1 != 0){
		LED_setColor(stored_r1,stored_g1,stored_b1);
		currentColorMode = LED_COLOR_MODE_1;
		if(stored_t1 != LED_NOTIMEOUT){
			colorTimerIsUsed = LED_COLOR_TIMER_USED;
			timer_SetTime(MTIMER_LED_COLOR_CHANGE,stored_t1);
		}
	}
	else if(stored_t2 != 0){
		LED_setColor(stored_r2,stored_g2,stored_b2);
		currentColorMode = LED_COLOR_MODE_2;
		if(stored_t2 != LED_NOTIMEOUT){
			colorTimerIsUsed = LED_COLOR_TIMER_USED;
			timer_SetTime(MTIMER_LED_COLOR_CHANGE,stored_t2);
		}		
	}
	else{
		colorTimerIsUsed = LED_COLOR_TIMER_NOT_USED;
		LED_turn_off_LED();
	}
}



void LED_handler(void){
	if (colorTimerIsUsed){
		if (!timer_bRunning(MTIMER_LED_COLOR_CHANGE)){
			if (currentColorMode == LED_COLOR_MODE_1){
				if (stored_t2 != 0){
					LED_setColor(stored_r2,stored_g2,stored_b2);
					currentColorMode = LED_COLOR_MODE_2;
					if(stored_t2 != LED_NOTIMEOUT){
						colorTimerIsUsed = LED_COLOR_TIMER_USED;
						timer_SetTime(MTIMER_LED_COLOR_CHANGE,stored_t2);
					}
					else{
						colorTimerIsUsed = LED_COLOR_TIMER_NOT_USED; // color1 timed out, now move to color 2 without timing out.					
					}					
				}
				else{
					colorTimerIsUsed = LED_COLOR_TIMER_NOT_USED;
					LED_turn_off_LED(); 
				}			
			}
			else if (currentColorMode == LED_COLOR_MODE_2){
				if (stored_t1 != 0){
					LED_setColor(stored_r1,stored_g1,stored_b1);
					currentColorMode = LED_COLOR_MODE_1;
					if(stored_t1 != LED_NOTIMEOUT){
						colorTimerIsUsed = LED_COLOR_TIMER_USED;
						timer_SetTime(MTIMER_LED_COLOR_CHANGE,stored_t1);
					}
				}
				else{
					colorTimerIsUsed = LED_COLOR_TIMER_NOT_USED;
					LED_turn_off_LED();
				}
			}
		}
	}
}