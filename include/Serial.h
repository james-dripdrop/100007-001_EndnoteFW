/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:		Serial.h
  Created by:   Niels Falk Vedel

  Description:

  Specification file:

*/

#ifndef _COMPILE_SERIAL_H
#define _COMPILE_SERIAL_H


/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/
#include "Address.h"
#include "../CommonData.h"

/***************************************************************************/
/*                                                                         */
/* Types                                                                   */
/*                                                                         */
/***************************************************************************/


typedef enum {IDLE, TO_ADDR, FROM_ADDR, TELEGRAM_TYPE, LENGTH, DATA, CHKSUM_HI, CHKSUM_LO, END} TStateType;

/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/






#define SATELLITE_ADDRESS	(SATELLITE_BASE_ADDRESS + Address_ucRead())

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
/* Send a telegram                                                        */
/************************************************************************/
void serial_Send(ToMasterTelegram_t tTelegram);

/************************************************************************/
/* Parse received telegram                                              */
/************************************************************************/
unsigned char serial_Service(void);

void serial_UmbrellaReturned(void);
void serial_UmbrellaReleased(void);
void serial_UmbrellaReleaseTimout(void);

#endif /* _COMPILE_SERIAL_H */