/*
  Copyright (c) 2002 by kk-electronic. All rights reserved.

  Project: Passat Compact 149 Terminal
  Filename: storage.h
  Created by: htm

  $Header: /PROJECT/PASSAT/COMPACT_149/_04_Implementation/SW/Controller/Include/storage.h 3     21-01-04 13:27 Nfv $

*/

#ifndef __STORAGE_H
#define __STORAGE_H

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

typedef enum {                                              // Function return values
	OK                          =  0,
	ERROR                       =  1,
	ERROR_UNKNOWN_COMMAND       = 10,
	ERROR_CRC                   = 11,
	ERROR_OUT_OF_RANGE          = 20,
	ERROR_PROCESS_STARTED       = 21,
	ERROR_ZERO                  = 22,
	ERROR_NULL                  = 23,
	ERROR_TOO_BIG               = 24,
	ERROR_CASE                  = 25,
	ERROR_OUT_OF_MEMORY         = 27,
	ERROR_ILLEGAL_VALUE         = 28,
	QUEUE_OVERFLOW		       = 40,
	WARNING_TEXT_TRIMMED        = 50,
	WARNING_POSITION_ADJUSTED   = 51,
	WARNING_MEMORY_FULL         = 52,
	WARNING_OUT_OF_MEMORY       = 53,
	WARNING_CHARACTER_REDEFINED = 54,
	WARNING_MAX                 = 99,
} TStatus;


/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/

#define UNINITIALIZED_EEPROM_DATA 0xFF


/***************************************************************************/
/*                                                                         */
/* Interface                                                               */
/*                                                                         */
/***************************************************************************/

TStatus storage_Init ();
TStatus storage_Task ();
TStatus storage_Install (void *pEE, unsigned char ucSize);

//Compatibility
void storage_Task10ms(void);


#endif
