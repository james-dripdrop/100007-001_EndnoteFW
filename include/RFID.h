/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:		RFID.h
  Created by:   Niels Falk Vedel

  Description:

  Specification file:

*/

#ifndef _COMPILE_RFID_H
#define _COMPILE_RFID_H


/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/

#include "../CommonData.h"


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

void RFID_Init();

void RFID_Read(void);

void RFID_CatchCurrentID(void);
void RFID_CatchNextID(void);
void RFID_RequestIDRead(void);
void RFID_service(void);

uint32_t RFID_get_ulUmbrellaID(void);
unsigned long RFID_get_ulCatchedCurrentID(void);
unsigned long RFID_get_ulCatchedNextID(void);
unsigned char RFID_get_ucRFIDReaderSiliconID(void);



#endif /* _COMPILE_RFID_H */