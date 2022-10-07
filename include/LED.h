/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:		LED.h
  Created by:   Niels Falk Vedel

  Description:

  Specification file:

*/

#ifndef _COMPILE_LED_H
#define _COMPILE_LED_H


/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/



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
// reset - bright yellow
#define RESET_LED_R 0xFF
#define RESET_LED_G 0xFF
#define RESET_LED_B 0x00
// init - dark blue - will be pulsing prior to RS485 connection
#define INIT_LED_R 0x00
#define INIT_LED_G 0x00
#define INIT_LED_B 0x21
// quiet mode: orange
#define QUIET_LED_R 0xFF
#define QUIET_LED_G 0x89
#define QUIET_LED_B 0x00
// error - bright red
#define ERROR_LED_R 0xFF
#define ERROR_LED_G 0x00
#define ERROR_LED_B 0x00
// goto bootloader - bright blue
#define BLOAD_LED_R 0x00
#define BLOAD_LED_G 0xFF
#define BLOAD_LED_B 0xFF





/***************************************************************************/
/*                                                                         */
/* Interface                                                               */
/*                                                                         */
/***************************************************************************/

void LED_init(unsigned char ledSequenceType);

void LED_setColor(unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue);

void LED_getColor(unsigned char* ucRed, unsigned char* ucGreen, unsigned char* ucBlue);

void LED_handleNewLEDMessage(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char t1, unsigned char r2, unsigned char g2, unsigned char b2, unsigned char t2, unsigned char seqType);

void LED_turn_off_LED(void);

void LED_handler(void);

#endif /* _COMPILE_LED_H */