/****** HEADER **************************************************************************************************/
/*                                                                                                              */
/*                                                                                                              */
/*  Author  : FAVetronics                                                                                       */
/*                                                                                                              */
/*  Function: Header file for timer functions                                                                   */
/*                                                                                                              */
/*                                                                                                              */
/*  Changes :                                                                                                   */
/****************************************************************************************************************/


#define FTIMER_RFID_READ_INTERVAL_ms_x10 10



#define TIMER_STOPPED 0xFFFF
#define TIMER_EXPIRED 0

// ***** 10ms timers *****
enum { // 10ms Timers
       FTIMER_START,
	    FTIMER_SERIAL_RECEIVE_TIMEOUT,
		FTIMER_SENSOR_ANTI_PRELL,
		TIMER_StorageTask,
		FTIMER_RFID_READ,
       FTIMER_MAX,
       // 100ms Timers
       MTIMER_START,
		MTIMER_SOLENOID_INRUSH,
		MTIMER_SOLENOID_TIMEOUT,
		MTIMER_RED_BLINK,
		MTIMER_GREEN_BLINK,
		MTIMER_BLUE_BLINK,
       MTIMER_MAX,
       // 1 sec. Timers
       STIMER_START,
		STIMER_WAITING_FOR_CONNECT,
       STIMER_MAX
     };

// ***** Global timer registers *****
#ifdef __compile_timer                                   // definitions if compiling the timer unit
unsigned int Timer[(unsigned char)STIMER_MAX];                          // user timers (10 ms, 100ms)
#else                                                    // definitions for other units using the timer
extern unsigned int  Timer[(unsigned char)STIMER_MAX];
#endif


unsigned char timer_bRunning(unsigned char TimerNr);					// Returns NULL if timer expired or stopped - otherwise returns remaining time
void timer_SetTime(unsigned char TimerNr, int Value);		// Set timers - can be used for both fresh start and for restart
void TimerStop(unsigned char TimerNr);
unsigned int timer_uiTimeLeft_min(unsigned char TimerNr); // returns remaining time in minutes. A minute has passed, when all of its seconds has passed. (1 sec left => TimeLeft_min = 1). Only work with STIMER
unsigned int timer_uiTimeLeft_sec(unsigned char TimerNr); // returns remaining time in seconds.
void timer_Control(void);									// General control routine - MUST be called every 10ms 
unsigned int TimerGetTick_x10ms(void);

