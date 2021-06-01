/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      DripDrop 
  Filename:		CommonData.h
  Created by:   Niels Falk Vedel

  Description:	Data that must be the same for both master and satellites

  Specification file:

*/

#ifndef _COMPILE_COMMONDATA_H
#define _COMPILE_COMMONDATA_H


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

typedef enum {
	TO_MASTER_NO_TLG					= 0x80,
	TO_MASTER_TLG_LED					= 0x81,
	TO_MASTER_TLG_RELEASE				= 0x82,
	TO_MASTER_TLG_VERSION				= 0x83,
	TO_MASTER_TLG_END_NODE_STATUS		= 0x84,
} ToMasterTelegram_t;


typedef enum {
	TO_SATELLITE_NO_TLG					= 0x00,
	TO_SATELLITE_TLG_LED				= 0x01,
	TO_SATELLITE_TLG_RELEASE			= 0x02,
	TO_SATELLITE_TLG_VERSION			= 0x03,
	TO_SATELLITE_TLG_STATUS				= 0x04,
	TO_SATELLITE_TLG_RESET				= 0x55,
} ToSatelliteTelegram_t;



/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/


//#define MAX_NO_OF_UMBRELLAS_IN_SATELLITE	5 /* 10 in spec, but limited due to RAM usage and communication speed (5 ID's is max throughput at 38.400Baud - 1,04ms / umbrella (4bytes)) */

#define UMBRELLA_ID_LENGTH_bytes			4 /* IDs from umbrellas are always 4 bytes long. */

#define NO_UMBRELLA_ID 0x00000000

// Communication:
#define SATELLITE_INIT_ATTEMPTS	10 // Try to establish communication this many times before setting the satellite to "not installed"

#define MASTER_ADDRESS			0x10
#define ALL_ADDRESS				0x20
#define SATELLITE_BASE_ADDRESS	0x30	// added to the DIP-SW setting on satellites, this will give the satellite address
										// Note: Dip-SW setting 1 is satellite no 1 - referenced as 0 in SW due to array indexing




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


#endif /* _COMPILE_COMMONDATA_H */