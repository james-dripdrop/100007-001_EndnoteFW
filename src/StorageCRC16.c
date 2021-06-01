/*
  Copyright (c) 2003- by kk-electronic. All rights reserved.

  Project:
  Filename: CRC16.C
  Created by: NFV

  $Header: /PROJECT/PASSAT/COMPACT_149/_04_Implementation/SW/Controller/Src/crc16.c 1     11-11-03 14:43 Htm $

  Description: Calculates 16 bit checksum
               Closest standard:    CRC-CCITT
               Initial value:       0
               Polynomium:          0X1021
               Final XOR:           None
               Reversed data bytes: No
  References:
    CRC calculator:       http://rcswww.urz.tu-dresden.de/~sr21/crc.html
    A PAINLESS GUIDE...:  http://www.cs.waikato.ac.nz/~312/crc.txt

*/

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
/* Locals                                                                  */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/* Interface                                                               */
/*                                                                         */
/***************************************************************************/

unsigned int crc_uiCalc(unsigned char *pData, unsigned int uiByteCount)
{
  unsigned int uiCurrentByte;
  unsigned int Temp;
  unsigned int CRC;
  unsigned char ucBitCount;

  CRC = 0;
  for (uiCurrentByte = 0; uiCurrentByte < uiByteCount; uiCurrentByte++)
  {
    Temp = (unsigned int)pData[uiCurrentByte] << 8;
    CRC = CRC ^ Temp;
    for (ucBitCount = 0; ucBitCount < 8; ucBitCount++)
    {
      if (CRC & 0x8000) CRC = (CRC << 1) ^ 0x1021;
      else CRC = CRC << 1;
    }
  }
  return CRC;
}

/***************************************************************************/
/*                                                                         */
/* Implementation                                                          */
/*                                                                         */
/***************************************************************************/


