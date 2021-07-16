/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:     Address.C
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
#include "Address.h"


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
/* Locals                                                                  */
/*                                                                         */
/***************************************************************************/


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

/************************************************************************/
/* Return Satellite Address as read on DIP-SW                           */
/************************************************************************/

unsigned char Address_ucRead(void)
{
	unsigned char ucUnitAddress;
	ucUnitAddress = (unsigned char)ADR0_get_level(); 
	ucUnitAddress |= (unsigned char)(ADR1_get_level()) << 1;
	ucUnitAddress |= (unsigned char)(ADR2_get_level()) << 2;
	ucUnitAddress |= (unsigned char)(ADR3_get_level()) << 3;
	return ucUnitAddress;
}



T_HWVersion Address_get_tHWVersion(void)
{
	volatile T_HWVersion tVers;
	tVers = (T_HWVersion)HW_ID_0_get_level();
	tVers |= (T_HWVersion)HW_ID_1_get_level() << 1;
	tVers |= (T_HWVersion)HW_ID_2_get_level() << 2;
	return tVers;
}