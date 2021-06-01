/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:		Address.h
  Created by:   Niels Falk Vedel

  Description:

  Specification file:

*/

#ifndef _COMPILE_ADDRESS_H
#define _COMPILE_ADDRESS_H


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

typedef enum {
	HW_VERSION_01	= 0x01, // First End_Note PCB
	HW_VERSION_02	= 0x02, // Second End_Note PCB - Improved ESD behavior - No CPU related changes
	HW_VERSION_03	= 0x03, // Elongated version, includes fuses.
} T_HWVersion;

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

/************************************************************************/
/* Return Satellite Address as read on DIP-SW                           */
/************************************************************************/
unsigned char Address_ucRead(void);

T_HWVersion Address_get_tHWVersion(void);

#endif /* _COMPILE_ADDRESS_H */