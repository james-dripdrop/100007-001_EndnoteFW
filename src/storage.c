/*
  Copyright (c) 2002- by kk-electronic. All rights reserved.

  Project: Passat Compact 149 Terminal
  Filename: storage.c
  Created by: htm

  $Header: /PROJECT/PASSAT/COMPACT_149/_04_Implementation/SW/Controller/Src/storage.c 4     14-01-05 9:25 Nfv $

  Description:

*/

/***************************************************************************/
/*                                                                         */
/* Includes                                                                */
/*                                                                         */
/***************************************************************************/

#include <stdlib.h>                                         // NULL

//#include "io.h"
#include "atmel_start_pins.h"
#include <nvmctrl_basic.h>

#include "StorageCRC16.h"
#include "storage.h"
#include "timer.h"

#include "../CommonData.h"


/***************************************************************************/
/*                                                                         */
/* Constants                                                               */
/*                                                                         */
/***************************************************************************/

//#define INSTALL_COUNT_MAX       50                          // Max. no. of backup installations
//#define INSTALL_DATA_MAX_SIZE   80                          // Max. data size pr. installation
#define INSTALL_COUNT_MAX       20                          // Max. no. of backup installations
#define INSTALL_DATA_MAX_SIZE   (MAX_NO_OF_UMBRELLAS_IN_SATELLITE * UMBRELLA_ID_LENGTH_bytes)                          // Max. data size pr. installation

const unsigned char ucCheckData [] = "Ver000";
//#define EEPROM_TOTAL_SIZE          4096                     // Total bytes in EEPROM
#define EEPROM_TOTAL_SIZE          1024                     // Total bytes in EEPROM (ATmega328PB)
#define EEPROM_CHECK_SIZE             6                     // Bytes used by CheckData
#define EEPROM_CHECK_START_ADDRESS    0                     // Start address of check data. ATmega16 has 512 bytes EEPROM
#define EEPROM_DATA_START_ADDRESS    16                     // Start address of non-volatile data
#define EEPROM_DATA_MAX_SIZE  (EEPROM_TOTAL_SIZE - EEPROM_DATA_START_ADDRESS)  // Memory size reserved for data incl. CRC-values
#define STORAGE_TaskTime_ms          10                     // EE task interval/EE store byte time
#define EEPROM_CRC_SIZE               2                     // Sizeof CRC-check: 2 bytes

#define ERROR(id,expr) ERROR_GLOBAL(MODULE_Storage, id, expr)

// Compatibility
#define TRUE true
#define FALSE false
#define io_ReadEEByte(Addr)			FLASH_0_read_eeprom_byte(Addr)
#define io_WriteEEByte(Addr, Data)	FLASH_0_write_eeprom_byte(Addr, Data)
#define timer_TimeoutStart(Timer, Time, CallBack) timer_SetTime(Timer, Time/10)
#define Task_10ms	storage_Task10ms


/***************************************************************************/
/*                                                                         */
/* Types                                                                   */
/*                                                                         */
/***************************************************************************/

typedef struct {
   unsigned char *pData;
   unsigned char ucBytes;
} TStore;

typedef enum {INIT, IDLE,
   UPDATE, UPDATE_WRITE_CRC_BYTE_1, UPDATE_WRITE_CRC_BYTE_2, UPDATE_NEXT_INSTALLATION,
   WRITE_ALL, WRITE_ALL_WRITE_CRC_BYTE_1, WRITE_ALL_WRITE_CRC_BYTE_2, WRITE_ALL_NEXT_INSTALLATION,  // The order is important!
   WRITE_CHECK_STRING
} TStorageState;



/***************************************************************************/
/*                                                                         */
/* Locals                                                                  */
/*                                                                         */
/***************************************************************************/

unsigned char ucInstallCount = 0;                           // No. of installations
TStore tStore [INSTALL_COUNT_MAX];                          // Array of installations

static TStorageState tStorageState;                         // Storage state machine

// Update variables
static unsigned char ucInstallation;                        // No. of current installation
static unsigned char ucByte;                                // Current byte no. in current installation
static unsigned char *pData;                                // Pointer to data for current data byte
static unsigned int uiEEAddress;                            // Current EE address for current data byte

/***************************************************************************/
/*                                                                         */
/* Prototypes                                                              */
/*                                                                         */
/***************************************************************************/

static void ReadEEToRam ();
void Task_10ms ();

/***************************************************************************/
/*                                                                         */
/* Interface                                                               */
/*                                                                         */
/***************************************************************************/

TStatus storage_Init ()
{
   // All init routines that calls storage_Install MUST be run before storage_Init

   // Check for control code in EE. Ok => read all EE to RAM, else write default data to EE
   // Start timer

   unsigned char bEECheckOk = TRUE;
   unsigned char i;

   i = EEPROM_CHECK_SIZE;
   do
   {
      i--;
      if (io_ReadEEByte (EEPROM_CHECK_START_ADDRESS + i) != ucCheckData[i])
      {
         bEECheckOk = FALSE;
      }
   }
   while ((bEECheckOk == TRUE) && (i > 0));

   // State machine must leave INIT state to signal that no more installations are allowed
   if (bEECheckOk == TRUE)
   {
      // Read all EE data to RAM
      ReadEEToRam ();
      tStorageState = IDLE;
   }
   else
   {
      // Leave default values in RAM. Initiate write sequence of all non-volatile data
      tStorageState = WRITE_ALL;
   }

   // Initialize update variables
   ucInstallation = 0;
   ucByte = 0;
   pData = tStore [ucInstallation].pData;
   uiEEAddress = EEPROM_DATA_START_ADDRESS;

   // Start timer
   timer_TimeoutStart (TIMER_StorageTask, STORAGE_TaskTime_ms, Task_10ms);

   return (OK);
}

TStatus storage_Task ()
{
   // Nothing to do - everything is done by Task_10ms
   return (OK);
}

/********************************************************************/
/* example: storage_Install (&tFuelRegVar1, sizeof (tFuelRegVar1)); */
/********************************************************************/
TStatus storage_Install (void *pEE, unsigned char ucSize)
{
   TStatus tStatus = OK;
   unsigned char i;
   unsigned int uiMemoryUsed = 0;

   if (ucInstallCount >= INSTALL_COUNT_MAX)
   {
      tStatus = ERROR_OUT_OF_RANGE;                        // Not enough room for more installations. Increase INSTALL_COUNT_MAX
   }
   else if (tStorageState != INIT)
   {
      tStatus = ERROR_PROCESS_STARTED;                     // All installations should be done before backup process starts
   }                                                        // Otherwise UPDATEALL will not cover the later installations
   else if (pEE == NULL)
   {
      tStatus = ERROR_NULL;                                // Pointer must exist
   }
   else if (ucSize == 0)
   {
      tStatus = ERROR_ZERO;                                // Data size must not be 0
   }
   else if (ucSize > INSTALL_DATA_MAX_SIZE)
   {
      tStatus = ERROR_TOO_BIG;                             // Data too big. Increase INSTALL_DATA_MAX_SIZE - or ucSize is erroneous
   }
   else
   {
      for (i = 0; i < ucInstallCount; i++)                  // Calculate memory used: data + checksums
      {
         uiMemoryUsed += (unsigned int) tStore [ucInstallCount].ucBytes + EEPROM_CRC_SIZE;
      }
      if (uiMemoryUsed + ucSize + EEPROM_CRC_SIZE > EEPROM_DATA_MAX_SIZE)
      {
         tStatus = ERROR_OUT_OF_MEMORY;                     // No room for these data. EEPROM memory (almost) full
      }
      else
      {                                                     // Install the specified data area
         tStore [ucInstallCount].pData = (unsigned char *) pEE;
         tStore [ucInstallCount].ucBytes = ucSize;

         ucInstallCount++;
      }
   }

   return (tStatus);
}

/***************************************************************************/
/*                                                                         */
/* Implementation                                                          */
/*                                                                         */
/***************************************************************************/

static void ReadEEToRam ()
{
   unsigned char ucInstallation;
   unsigned char ucByte;
   unsigned char *pData;
   unsigned int uiEEAddress = EEPROM_DATA_START_ADDRESS;
   unsigned int uiCrcAddress; // = EEPROM_DATA_START_ADDRESS + EEPROM_DATA_MAX_SIZE - EEPROM_CRC_SIZE;
   unsigned int uiCrcData, uiCrcStored;
   unsigned char ucTemp [INSTALL_DATA_MAX_SIZE];

   for (ucInstallation = 0; ucInstallation < ucInstallCount; ucInstallation++)
   {
      // Copy EE data to temporary memory
      for (ucByte = 0; ucByte < tStore [ucInstallation].ucBytes; ucByte++)
      {
         ucTemp [ucByte] = io_ReadEEByte (uiEEAddress);
         uiEEAddress++;
      }

      // Check CRC-checksum
      uiCrcAddress = uiEEAddress;
      uiEEAddress += EEPROM_CRC_SIZE;
      uiCrcStored = (io_ReadEEByte (uiCrcAddress) << 8) + io_ReadEEByte (uiCrcAddress + 1);
      uiCrcAddress -= EEPROM_CRC_SIZE;
      uiCrcData = crc_uiCalc (ucTemp, tStore [ucInstallation].ucBytes);
      if (uiCrcData == uiCrcStored)
      {
         // Copy EE-data to RAM only if checksum ok. If checksum fails, default data will be used.
         // Data will not be saved in EE until user changes data.
         pData = tStore [ucInstallation].pData;

         for (ucByte = 0; ucByte < tStore [ucInstallation].ucBytes; ucByte++)
         {
            *pData = ucTemp [ucByte];
            pData++;
         }
      }
   }
}


void Task_10ms ()
{
   //TStatus tStatus = OK;
   static unsigned char bInstallationChanged = FALSE;
   static unsigned int uiCrc, uiEECrcAddress;

   switch (tStorageState)
   {
      case INIT:
         // Initial state, only active during initialization: Do nothing
         break;

      case IDLE:
         // Idle state, active between updates
         tStorageState = UPDATE; // Start new update
         break;

      case UPDATE:
      case WRITE_ALL:
         // Update: Write EE cell if value is different from RAM
         // Write all: Write all EE cells

         if ((tStorageState == WRITE_ALL) || (io_ReadEEByte (uiEEAddress) != *pData))
         {
            io_WriteEEByte (uiEEAddress, *pData);
            bInstallationChanged = TRUE;
         }

         // Point to next byte
         ucByte++;
         pData++;
         uiEEAddress++;

         if (ucByte >= tStore [ucInstallation].ucBytes)
         {
            // Point to first byte in next installation
            ucByte = 0;

            // Write new CRC if one or more bytes in one installation has changed
            if (bInstallationChanged == TRUE)
            {
               uiEECrcAddress = uiEEAddress; //EEPROM_DATA_START_ADDRESS + EEPROM_DATA_MAX_SIZE - EEPROM_CRC_SIZE * (ucInstallation + 1);
               uiCrc = crc_uiCalc (tStore [ucInstallation].pData, tStore [ucInstallation].ucBytes);
               tStorageState++;                             // Go to write_crc_byte_1 state
               bInstallationChanged = FALSE;
            }
            else
            {
               // Point to next installation
               if (tStorageState == WRITE_ALL)
               {
                  tStorageState = WRITE_ALL_NEXT_INSTALLATION;
               }
               else
               {
                  tStorageState = UPDATE_NEXT_INSTALLATION;
               }
            }
         }
         break;

      case WRITE_ALL_WRITE_CRC_BYTE_1:
      case UPDATE_WRITE_CRC_BYTE_1:
         io_WriteEEByte (uiEECrcAddress, (unsigned char) (uiCrc >> 8));  // Write MSB to 1st address
         tStorageState++;                                               // Go to write_crc_byte_2 state
         break;

      case WRITE_ALL_WRITE_CRC_BYTE_2:
      case UPDATE_WRITE_CRC_BYTE_2:
         io_WriteEEByte (uiEECrcAddress + 1, (unsigned char) (uiCrc & 0x00ff));  // Write LSB to 2nd address
         tStorageState++;                                               // Go to write_crc_byte_2 state
         break;

      case WRITE_ALL_NEXT_INSTALLATION:
      case UPDATE_NEXT_INSTALLATION:
         // Point to next installation. Write check string after last installation during WRITE_ALL
         uiEEAddress += EEPROM_CRC_SIZE;
         ucInstallation++;
         if (ucInstallation >= ucInstallCount)
         {
            ucInstallation = 0;
            uiEEAddress = EEPROM_DATA_START_ADDRESS;

            if (tStorageState == WRITE_ALL_NEXT_INSTALLATION)
            {
               ucByte = 0;
               tStorageState = WRITE_CHECK_STRING;
            }
            else
            {
               tStorageState = IDLE;
            }
         }
         else
         {
            // Go to original state (write_all/update)
            if (tStorageState == WRITE_ALL_NEXT_INSTALLATION)
            {
               tStorageState = WRITE_ALL;
            }
            else
            {
               tStorageState = UPDATE;
            }
         }
         pData = tStore [ucInstallation].pData;
         break;

      case WRITE_CHECK_STRING:
         if (ucByte >= EEPROM_CHECK_SIZE)
         {
            ucByte = 0;
            tStorageState = IDLE;
         }
         else
         {
            io_WriteEEByte (((unsigned int) EEPROM_CHECK_START_ADDRESS) + ucByte, ucCheckData [ucByte]);
            ucByte++;
         }
         break;

      default:
         //tStatus = ERROR_CASE;
         break;
   }

   // Restart timer
   timer_TimeoutStart (TIMER_StorageTask, STORAGE_TaskTime_ms, Task_10ms);
}

