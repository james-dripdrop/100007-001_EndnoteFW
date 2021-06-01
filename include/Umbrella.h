/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      DripDrop Satellite
  Filename:		Umbrella.h
  Created by:   Niels Falk Vedel

  Description:

  Specification file:

*/

#ifndef _COMPILE_UMBRELLA_H
#define _COMPILE_UMBRELLA_H


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

typedef enum
{
	UMBRELLA_RESULT_OK			= 0x00,
	UMBRELLA_RESULT_UNKNOWN_ID	= 0xFC,
	UMBRELLA_RESULT_EMPTY		= 0xFD,
	UMBRELLA_RESULT_FULL		= 0xFE,
	UMBRELLA_RESULT_ERROR		= 0xFF,
} UmbrellaResult_t;


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

void umbrella_Init(void);

unsigned char umbrella_get_ucNoOfUmbrellasInSatellite(void);

void umbrella_clearAllUmbrellaIDs(void);
UmbrellaResult_t umbrella_InsertUmbrella(uint32_t ulUmbrellaID);
UmbrellaResult_t umbrella_RemoveUmbrella(uint32_t ulUmbrellaID);
uint32_t umbrella_get_UmbrellaID(uint8_t ucIDNo);




void umbrella_StorageService(void);


#endif /* _COMPILE_UMBRELLA_H */