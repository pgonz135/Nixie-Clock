#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mainTasks.h>
#include <nixieDriver.h>
#include <global.h>

void timerSet( void );

void main(void) {

  /* Copy Address of Current Time into Pointer */
  currentTimePtr = &currentTime;
  getTimersPtr = &getTimers;

  /* Initialize Nixie Clock */
  initStateMachine();
  timerSet();
  initUCS();
  initPorts();

  /* Endless Loop */
  while (1) {
    __bis_SR_register( GIE );         /* Interrupts Enabled */
    __no_operation();                 /* For debugger to work */
  }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer0_B1 (void) {
  /* Toggle the Buzzer */
  P3OUT ^= BIT6;
}

/* Service Switches Interrupt Note: Interrupts should not hit because they are disabled*/
#pragma vector=PORT1_VECTOR
__interrupt void switchPressISR(void) {
  P1IFG &= ~BIT7;   /* Clear Set Minutes ISR Flag */
}

#pragma vector=PORT4_VECTOR
__interrupt void switchPressISR1(void) {
  P4IFG &= ~BIT4;   /* Clear Set Hours ISR Flag */
  P4IFG &= ~BIT5;   /* Clear Set Alarm ISR Flag */
  P4IFG &= ~BIT6;   /* Clear Set Alarm ISR Flag */
}

void timerSet( void ) {

  /* Set Up Clock Timer */
  TA0CCTL0 = CCIE;                                  // Enable counter interrupt on counter compare register 0
  TA0CTL = TASSEL__ACLK  + MC_1 + TACLR;            // Use the SMCLK to clock the counter, SMCLK/8, count up mode to CCR0 value, Clear after counting
  TA0CCR0 = TIMER_TICK_COUNTS - 1;                  // Clock Ticks Every 125 milliseconds 32768/8 = 4096 ( 1 Seconds / 8 = 125ms )

  /* Set Up Buzzer Timer 4.096 kHz */
  TB0CCTL0 &= ~CCIE;
  TB0CTL = TASSEL__ACLK + MC_1 + TACLR;
  TB0CCR0 = 4 - 1;
}

/* Timer Interrupt */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {

  /* Increase Timers */
  getTimersPtr -> highResTimer++;
  getTimersPtr -> taskSchedulerTimer++;
  getTimersPtr -> cathodeRefreshTimer++;
  getTimersPtr -> alarmTimer++;
  getTimersPtr -> startupTimer++;

  /* Heart of Task Manager */
  if (  getTimersPtr -> taskSchedulerTimer >= 1 ) {
    unsigned char i;
    for (i = 0; i < tasksNum; ++i)                            // Cycle through all SMs
    {
      if ( tasks[i].elapsedTime >= tasks[i].period )          // If the Elapsed Time > Period of SM
      {
        tasks[i].state = tasks[i].TickFct(tasks[i].state);    //
        tasks[i].elapsedTime = 0;                             // Reset elapsed time
      }
      tasks[i].elapsedTime += tasksPeriodGCD;                 // Add elapsed time to GCD
      getTimersPtr -> taskSchedulerTimer = 0;
    }
  }

  /* Update Timing while Set Alarm is Inactive */
  if ( 0 == setAlarmFlag ) {
    if ( ( getTimersPtr -> highResTimer ) == TIMER_TICKS ) {
      /* One Second Timebase */
      currentTimePtr -> seconds++;

      /* After 60 seconds, reset seconds, increase minute */
      if ( ( currentTimePtr -> seconds ) == ROLLOVER_SECONDS ) {
        currentTimePtr -> seconds = 0;
        currentTimePtr -> minutes ++;

        /* After 59 minutes, reset minutes, increase hour */
        if ( ( currentTimePtr -> minutes ) == ROLLOVER_MINUTES ) {
          currentTimePtr -> minutes = 0;
          currentTimePtr -> hours ++;
        }

        /* After 24 hours, reset minutes, increase hour */
        if ( ( currentTimePtr -> hours ) == ROLLOVER_HOURS_24 ) {
          currentTimePtr -> hours = 0;
          currentTimePtr -> minutes = 0;
        }
      }
      getTimersPtr -> highResTimer = 0;
    }
  }
}


