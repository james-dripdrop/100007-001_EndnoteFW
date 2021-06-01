/*
  Copyright (c) 2003- by kk-electronic. All rights reserved.

  Project:
  Filename: CRC16.H
  Created by: NFV

  $Header: /PROJECT/PASSAT/COMPACT_149/_04_Implementation/SW/Controller/Include/CRC16.h 1     11-11-03 14:44 Htm $

*/

#ifndef __CRC16_H
#define __CRC16_H

/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/* Types                                                                   */
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

/****************************************************************************************************/
// Navn     : crc_uiCalc
// Funktion : Beregner CRC på data
// Input    : Pointer til data, antal bytes der skal beregnes på
// Output   : CRC
/****************************************************************************************************/

unsigned int crc_uiCalc(unsigned char *pData, unsigned int uiByteCount);

#endif
