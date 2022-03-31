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