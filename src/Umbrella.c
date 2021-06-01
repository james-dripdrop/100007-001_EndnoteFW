/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      DripDrop Satellite
  Filename:     Umbrella.C
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
#include "..\CommonData.h"
#include "Umbrella.h"
//#include "storage.h"


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

uint32_t ulAvailableUmbrellas[MAX_NO_OF_UMBRELLAS_IN_SATELLITE];
uint8_t ucNextEntryIndex = 0;

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

void umbrella_Init(void)
{
//	storage_Install (&ulAvailableUmbrellas, sizeof (ulAvailableUmbrellas));
//	storage_Install (&ucNextEntryIndex, sizeof (ucNextEntryIndex));
}


unsigned char umbrella_get_ucNoOfUmbrellasInSatellite(void)
{
	return ucNextEntryIndex;
}


void umbrella_clearAllUmbrellaIDs(void)
{
	for (uint8_t u=0; u<MAX_NO_OF_UMBRELLAS_IN_SATELLITE;u++)
	{
		ulAvailableUmbrellas[u] = NO_UMBRELLA_ID; // just a help during debug
	}
	ucNextEntryIndex = 0; // ucNextEntryIndex solely determines the number of umbrellas
}


UmbrellaResult_t umbrella_InsertUmbrella(uint32_t ulUmbrellaID)
{
	UmbrellaResult_t tResult = UMBRELLA_RESULT_OK;
	if (ucNextEntryIndex == MAX_NO_OF_UMBRELLAS_IN_SATELLITE) tResult = UMBRELLA_RESULT_FULL;
	else 
	{
		ulAvailableUmbrellas[ucNextEntryIndex++] = ulUmbrellaID;
		for (uint8_t ucUmbrellaCnt = ucNextEntryIndex; ucUmbrellaCnt < MAX_NO_OF_UMBRELLAS_IN_SATELLITE; ucUmbrellaCnt++)
		{
			ulAvailableUmbrellas[ucUmbrellaCnt] = NO_UMBRELLA_ID; // clear remaining positions - just a help during debug
		}
	}
	return tResult;
}

UmbrellaResult_t umbrella_RemoveUmbrella(uint32_t ulUmbrellaID)
{
	UmbrellaResult_t tResult = UMBRELLA_RESULT_OK;
	if (ucNextEntryIndex == 0) tResult = UMBRELLA_RESULT_EMPTY;
	else
	{
		if (ulAvailableUmbrellas[ucNextEntryIndex-1] == ulUmbrellaID)
		{
			 ulAvailableUmbrellas[ucNextEntryIndex-1] = NO_UMBRELLA_ID; // just a help during debug
			 ucNextEntryIndex--;
		}
		else tResult = UMBRELLA_RESULT_UNKNOWN_ID;
	}	
	return tResult;
}

uint32_t umbrella_get_UmbrellaID(uint8_t ucIDNo)
{
	uint32_t ulResult;
	if (ucIDNo >= MAX_NO_OF_UMBRELLAS_IN_SATELLITE) ulResult = NO_UMBRELLA_ID;
	else ulResult = ulAvailableUmbrellas[ucIDNo];
	return ulResult;
}



