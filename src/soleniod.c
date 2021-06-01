/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:     solenoide.C
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
#include "Timer.h"
#include "pwm_basic.h"
#include "Serial.h"
#include "RFID.h"
#include "solenoid.h"
#include "Mug_up.h"
#include "Umbrella.h"

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

#define NO_OF_INRUSH_PULSES 3 // give this many pulses with inrush PWM. pause time = pulse time


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
unsigned char ucPWMHoldLevel_bin = 0;
uint8_t ucPWMInrushLevel_bin = 0;
uint8_t ucInrushTime_ms_x100;
bool bSolenoidActivated = false;
bool bReleaseTimeoutEnabled  = true;
uint8_t ucInrushPulses = -1;


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

void solenoid_Service(void)
{
	if (bSolenoidActivated)
	{
		if ((!timer_bRunning(MTIMER_SOLENOID_TIMEOUT)) && bReleaseTimeoutEnabled)
		{
			PWM_0_load_duty_cycle_ch0(0); // disable solenoid
			bSolenoidActivated = false;
			serial_UmbrellaReleaseTimout();
		}
		else if (!timer_bRunning(MTIMER_SOLENOID_INRUSH)) // inrush complete
		{
			if (ucInrushPulses < (NO_OF_INRUSH_PULSES * 2))
			{
				if (ucInrushPulses++ & 0x01) // inrush was activated (uneven)
				{
					PWM_0_load_duty_cycle_ch0((PWM_0_register_t)ucPWMHoldLevel_bin); // set hold PWM
				}
				else // activate inrush again
				{
					PWM_0_load_duty_cycle_ch0((PWM_0_register_t)ucPWMInrushLevel_bin); //Set inrush PWM
				}
				timer_SetTime(MTIMER_SOLENOID_INRUSH, ucInrushTime_ms_x100); // restart timer
			}
			else PWM_0_load_duty_cycle_ch0((PWM_0_register_t)ucPWMHoldLevel_bin); // inrush complete
		}
	}
	else 
	{
		PWM_0_load_duty_cycle_ch0(0); // Make sure output really is off
	}
}




void solenoid_InitiateRelease(unsigned char ucPWM_high_pct, unsigned char uctime_high_ms_x100, unsigned char ucPWM_low_pct, unsigned char ucTIMEOUT_sec)
{
	if (ucTIMEOUT_sec > 0) // Activate solenoid
	{
		ucInrushTime_ms_x100 = uctime_high_ms_x100;
		timer_SetTime(MTIMER_SOLENOID_INRUSH, ucInrushTime_ms_x100);
		ucPWMInrushLevel_bin =(uint8_t)((unsigned int)ucPWM_high_pct * 255 / 100);
		PWM_0_load_duty_cycle_ch0((PWM_0_register_t)ucPWMInrushLevel_bin); //Set inrush
		ucInrushPulses = 1;
		bSolenoidActivated = true;
		// use the longest timer for total runtime
		if ((ucTIMEOUT_sec * 10) > (uctime_high_ms_x100 * (NO_OF_INRUSH_PULSES *2))) timer_SetTime(MTIMER_SOLENOID_TIMEOUT, ucTIMEOUT_sec * 10);
		else timer_SetTime(MTIMER_SOLENOID_TIMEOUT, (uctime_high_ms_x100 * (NO_OF_INRUSH_PULSES *2)));
		if (ucTIMEOUT_sec == 0xFF) bReleaseTimeoutEnabled = false; else bReleaseTimeoutEnabled = true;
		ucPWMHoldLevel_bin = (unsigned char)((unsigned int)ucPWM_low_pct * 255 / 100); // store level, so it can be used when inrush is completed
	}
	else // Deactivate solenoid
	{
		timer_SetTime(MTIMER_SOLENOID_INRUSH, TIMER_STOPPED);
		timer_SetTime(MTIMER_SOLENOID_TIMEOUT, TIMER_STOPPED);
		PWM_0_load_duty_cycle_ch0(0); //Stop PWM
		bSolenoidActivated = false;
		bReleaseTimeoutEnabled = true;
	}
}





unsigned char solenoid_get_ucReleaseStatus(void)
{
	// 0x00: NOT releasing umbrella
	// 0x01: Releasing umbrella
	return (unsigned char)bSolenoidActivated;
}


/*
void _solenoid_DetectMove(void) // called by Sensor A falling edge interrupt
{
	if (_SENSOR_B_get_level())
	{
		// signal that umbrella has been returned
		serial_UmbrellaReturned();
	}
	else if (bSolenoidActivated) // It's not possible to remove umbrella if solenoid is not activated
	{
		// signal that umbrella has been removed
		serial_UmbrellaReleased();
		if (bReleaseTimeoutEnabled)
		{
			PWM_0_load_duty_cycle_ch0(0); // disable solenoid
			bSolenoidActivated = false;
			timer_SetTime(MTIMER_SOLENOID_TIMEOUT, TIMER_STOPPED);
		}
	}
	RFID_RequestIDRead();
}
*/

void solenoid_DetectMove(void) // called by Sensor A any edge interrupt
{
	static bool bReturnBegun = false;
	static bool bRemoveBegun = false;

	if (_SENSOR_A_get_level() == false)
	{
		if (_SENSOR_B_get_level())
		{
			bReturnBegun = true;
		}
		else if (bSolenoidActivated) // It's not possible to remove umbrella if solenoid is not activated
		{
			bRemoveBegun = true;
		}
		RFID_RequestIDRead(); //JGU: interrupt driven clearing of RFID read timer
	}
	else // A high
	{
		if (_SENSOR_B_get_level() && bRemoveBegun)
		{
			// signal that umbrella has been removed
			serial_UmbrellaReleased();
			if (bReleaseTimeoutEnabled)
			{
				PWM_0_load_duty_cycle_ch0(0); // disable solenoid
				bSolenoidActivated = false;
				timer_SetTime(MTIMER_SOLENOID_TIMEOUT, TIMER_STOPPED);
			}
		}
		else if (bReturnBegun)
		{
			// signal that umbrella has been returned
			serial_UmbrellaReturned();
		}
		bReturnBegun = false;
		bRemoveBegun = false;
	}
}
