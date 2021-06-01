/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      DripDrop Satellite
  Filename:		Mug_up.h
  Created by:   Niels Falk Vedel

  Description:

  Specification file:

*/

#ifndef _COMPILE_MUG_UP_H
#define _COMPILE_MUG_UP_H


/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/

#undef MUG_UP_TEST

#ifdef MUG_UP_TEST
extern void main_SetRedBlink(unsigned char ucNoOfBlink);
extern void main_SetGreenBlink(unsigned char ucNoOfBlink);
extern void main_SetBlueBlink(unsigned char ucNoOfBlink);
#endif



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



#endif /* _COMPILE_MUG_UP_H */