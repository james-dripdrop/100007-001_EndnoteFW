/*
  Copyright (c) 2019 by FAVetronics. All rights reserved.

  Project:      Michanik, DripDrop Satellite
  Filename:     solenoid.C
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
	if(bSolenoidActivated)
	{
		if( ( !timer_bRunning(MTIMER_SOLENOID_TIMEOUT) ) && bReleaseTimeoutEnabled)
		{
			deactivateSolenoid();
			serial_UmbrellaReleaseTimout();
		}
		else if (!timer_bRunning(MTIMER_SOLENOID_INRUSH))
		{
				handleSolenoidPulsing();
		}
	}
	else
	{
		

		deactivateSolenoid();//apparently, we want to make sure the solenoid is off...which is concerning.
	}
}

void handleSolenoidPulsing(void)
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
	else
	{
		PWM_0_load_duty_cycle_ch0((PWM_0_register_t)ucPWMHoldLevel_bin); // inrush complete
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
		deactivateSolenoid();
	}
}

void deactivateSolenoid(void)
{
	TimerStop(MTIMER_SOLENOID_INRUSH);
	TimerStop(MTIMER_SOLENOID_TIMEOUT);
	PWM_0_load_duty_cycle_ch0(0); //Stop PWM
	bSolenoidActivated = false;
	bReleaseTimeoutEnabled = true;
}



unsigned char solenoid_get_ucReleaseStatus(void)
{
	// 0x00: NOT releasing umbrella
	// 0x01: Releasing umbrella
	// hold the status for a timer-based duration to prevent going from Releasing -> OK -> Umbrella Released
	
	return (unsigned char)(bSolenoidActivated || timer_bRunning(MTIMER_STATUS_HOLD_RELEASE_TIMEOUT));
}



// called by Sensor A / Sensor B any edge interrupt
void wheel_DetectMove(void) 
{
	static bool bReturnBegun = false;
	static bool bRemoveBegun = false;
	volatile bool sensorALevel = _SENSOR_A_get_level();
	volatile bool sensorBLevel = _SENSOR_B_get_level();
	if(!timer_bRunning(MTIMER_SPIN_TIMEOUT))
	{
		//if timer isn't running, reset status
		bReturnBegun = false;
		bRemoveBegun = false;
	}
	
	if(bReturnBegun)
	{
		//look for conditions that would establish a return wheelspin has occurred
		if( ( sensorALevel ) )
		{
			//Experimentation with returning indicates on a return, sensor B can be in either state
			/*
			We need to:
				indicate that the umbrella is returned
				clear the spin timer ; this resets returnBegun/removeBegun on next edge
			*/
			serial_UmbrellaReturned();
			TimerStop(MTIMER_SPIN_TIMEOUT);
		}
		
	}
	else if (bRemoveBegun)
	{
		//look for conditions that would establish a release wheelspin has occurred
		if( ( sensorALevel ) && ( sensorBLevel ) )
		{
			/*
			We need to:
			communicate that the umbrella is released
			lock the arm again (stop energizing the solenoid)
			request the RFID read
			clear the spin timer
			*/
			RFID_RequestIDRead(); //JGU: interrupt driven clearing of RFID read timer
			serial_UmbrellaReleased();
			TimerStop(MTIMER_SPIN_TIMEOUT);
			// locking the arm probably should be in its own function			
			deactivateSolenoid();
		}
	}
	else
	{
		//the start of a return or release has not been registered, but some motion has triggered the edge interrupt
		if(!sensorALevel)
		{
			timer_SetTime(MTIMER_SPIN_TIMEOUT,10); //timeout between starting a spin and ending a spin
			if(!sensorBLevel)
			{
				bRemoveBegun = true; 
			}
			else 
			{
				RFID_RequestIDRead(); //JGU: interrupt driven clearing of RFID read timer
				bReturnBegun = true; 
			}
		}
	}
	
}
