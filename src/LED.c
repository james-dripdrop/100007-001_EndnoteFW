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

void LED_init(void)
{
	NeoPixel_updateType(NEO_GRB + NEO_KHZ800);
	NeoPixel_updateLength(LED_COUNT);
	/*
	JGU: Oh good, someone set a hidden suicide point
	if ( (Address_get_tHWVersion() == HW_VERSION_01) ||
	     (Address_get_tHWVersion() == HW_VERSION_02) )NeoPixel_setPin(&LED_CONTROL_PORT_HW01, LED_CONTROL_PIN_HW01);
	else while (true); // critical error - unknown HW version - stay here and wait for WD 
	*/
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
