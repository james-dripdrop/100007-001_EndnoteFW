/****** MODULE **************************************************************************************************/
/*                                                                                                              */
/*                                                   */
/*     ============================================================                                             */
/*                                                                                                              */
/*  Author  : Niels Falk Vedel, FAVetronics                                                                                     */
/*                                                                                                              */
/*  Function: Timer routines.                                                                                   */
/*                                                                                                              */
/*  V1.00 - 2000.01.14                                                                                          */
/*      Filename: timersys.c                                                                                    */
/*                                                                                                              */
/*  Changes :                                                                                                   */
/****************************************************************************************************************/
#define __compile_timer                                  // define used in timer.h

#include "timer.h"                                    // constant and type definitions

unsigned int uiTick_x10ms = 0;

/*****************************************************************************************************************
CHECK TIMERS
Input    : Timer
Output   : State of current Timer 
Function : Returns value of current Timer.
*****************************************************************************************************************/
unsigned char timer_bRunning(unsigned char TimerNr)
{
	unsigned char bState = 1;
	if (Timer[TimerNr] == TIMER_EXPIRED) bState = 0;
	if (Timer[TimerNr] == TIMER_STOPPED) bState = 0;
	return bState;
}

// returns remaining time in minutes. A minute has passed, when all of its seconds has passed. (1 sec left => TimeLeft_min = 1). Only work with STIMER
unsigned int timer_uiTimeLeft_min(unsigned char TimerNr)
{
  unsigned int uiResult = 0;
  if (Timer[TimerNr] != TIMER_STOPPED)
  {
    uiResult = (Timer[TimerNr] + 59) / 60;
  }
   return uiResult;
}

unsigned int timer_uiTimeLeft_sec(unsigned char TimerNr) // returns remaining time in seconds.
{
  unsigned int uiResult = 0;
  if (Timer[TimerNr] != TIMER_STOPPED)
  {
    uiResult = Timer[TimerNr];
  }
   return uiResult;
}


/*****************************************************************************************************************
SET TIMERS
Input    : Timer and Value
Output   : Updated Timer array
Function : Set current timer to current value
*****************************************************************************************************************/
void timer_SetTime(unsigned char TimerNr,int Value)
{
   Timer[TimerNr] = Value;
}

void TimerStop(unsigned char TimerNr)
{
   Timer[TimerNr] = TIMER_STOPPED;
}

/*****************************************************************************************************************
TIME MEASURE FUNCTIONS
*****************************************************************************************************************/
unsigned int TimerGetTick_x10ms(void)
{
	return(uiTick_x10ms);
}


/*****************************************************************************************************************
Function : void TIMER2 interrupt
Date     : 2000.02.01
Function : Interrupt service routine for timer 2
           This int. handler is called every 10 mS
*****************************************************************************************************************/
void timer_Control(void)                 // This routine is pseudo-interrupt controlled (i.e. called from
{                                       // main loop every time a timer interrupt has occured)
   static int MediumCounter = 0;
   static int SlowCounter = 0;
   unsigned char i;

   for (i=FTIMER_START; i<FTIMER_MAX; i++)                             // Update 10ms times
   {
      if((Timer[i] != TIMER_EXPIRED) && (Timer[i] != TIMER_STOPPED))  // if timer running
         Timer[i]--;                                                  // decrement timer value
   }

   if(++MediumCounter >= 10)
   {
      MediumCounter = 0;
      for (i=MTIMER_START; i<MTIMER_MAX; i++)               // Update 100ms times
      {
        if((Timer[i] != TIMER_EXPIRED) && (Timer[i] != TIMER_STOPPED))  // if timer running
          Timer[i]--;                                                  // decrement timer value
      }
   }

   if(++SlowCounter >= 100)
   {
      SlowCounter = 0;
      for (i=STIMER_START; i<STIMER_MAX; i++)               // Update 1 sec times
      {
        if((Timer[i] != TIMER_EXPIRED) && (Timer[i] != TIMER_STOPPED))  // if timer running
          Timer[i]--;                                                  // decrement timer value
      }
   }
   uiTick_x10ms++;
}

