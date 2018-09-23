/*
   global.c

    Created on: Sep 16, 2018
        Author: Administrator
*/


#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mainTasks.h>
#include <nixieDriver.h>
#include <global.h>

#define ENABLE_ALARM()  ( TB0CCTL0 |= CCIE  )
#define DISABLE_ALARM() ( TB0CCTL0 &= ~CCIE )

struct time   currentTime = {0, 0, 0, 0, 0, 0};
struct timers getTimers = {0, 0, 0, 0};
uint8_t setAlarmFlag = 0;


void initUCS( void ) {

  WDTCTL = WDTPW | WDTHOLD;

  /* Output ACLK to P.20 */
  P2SEL0 |= BIT0;
  P2SEL1 |= BIT0;
  P2DIR |= BIT0;

  /* Enable Clock Pins */
  PJSEL0 |= BIT4;
  PJSEL1 &= ~BIT4;

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // XT1 Setup
  CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
  CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
  CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL4 = LFXTBYPASS ;
  //   CSCTL4 = LFXTBYPASS | LFXTOFF;

  unsigned long counter = 0;

  do
  {
    CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
    SFRIFG1 &= ~OFIFG;
    counter++;
  } while ( counter <= 100000 );              // Test oscillator fault flag (For some reason oscillator won't start unless cleared a bunch of times)
}

void initPorts( void ) {
  /* Initialize HV5622 Pins to Output Direction */
  P1DIR |=  BIT0;
  P1DIR |=  BIT1;
  P1DIR |=  BIT2;
  P1DIR |=  BIT3;

  /* Set Up Interrupts/Pull Up on Minutes PB (P1.7) */
  P1DIR &=  ~BIT7;
  P1OUT |=   BIT7;
  P1REN |=   BIT7;                         // Enable Internal pull up resistors on 1.1
  P1IES &=  ~BIT7;                         // Set Hi/Lo Edge Select on P1.1
  P1IE  &=  ~BIT7;                         // Enable interrupts from P1.1
  P1IFG &=  ~BIT7;                         // Clear interrupt flag

  /* Set Up Interrupts/Pull Up on Hour PB (P4.4) */
  P4DIR &=  ~BIT4;
  P4OUT |=   BIT4;
  P4REN |=   BIT4;
  P4IES &=  ~BIT4;
  P4IE  &=  ~BIT4;
  P4IFG &=  ~BIT4;

  /* Set Up Interrupts/Pull Up on Set Alarm PB (P4.5) */
  P4DIR &=  ~BIT5;
  P4OUT |=   BIT5;
  P4REN |=   BIT5;
  P4IES &=  ~BIT5;
  P4IE  &=  ~BIT5;
  P4IFG &=  ~BIT5;

  /* Set Up Interrupts/Pull Up on Set Time PB (P4.6) */
  P4DIR &=  ~BIT6;
  P4OUT |=   BIT6;
  P4REN |=   BIT6;
  P4IES &=  ~BIT6;
  P4IE  &=  ~BIT6;
  P4IFG &=  ~BIT6;

  /* Set up Buzzer GPIO */
  P3DIR |=   BIT6;
  P3OUT |=   BIT6;

  /* Backlights LED */
  P2DIR |=   BIT2;
  P2OUT |=   BIT2;

  P2DIR |=   BIT2;
  P2OUT |=   BIT2;

  /* Initialize HV5622 */
  RST_BL();
  SET_BL();

}

/* Function Called every 250ms - there is then a total of 500ms debounce */
nixieOperatingMode_t pollSwitches( void ) {

  const uint8_t minimumDebounceTime = 2; /* Every Increment is 250ms extra of Debounce */
  static uint8_t minutesDebounce = 0;
  static uint8_t hoursDebounce = 0;
  static uint8_t modeDebounce = 0;
  static uint8_t alarmDebounce = 0;
  static uint8_t currentHours = 0;
  static uint8_t currentMinutes = 0;
  static uint8_t currentSeconds = 0;
  static nixieOperatingMode_t switchOperatingMode = clockMode;

  /* Set Minutes and Debounce */
  if ( MINUTES_PB_SET && ( switchOperatingMode != setAlarmMode ) ) {
    minutesDebounce++;
    if ( minutesDebounce >= minimumDebounceTime ) {
      currentTimePtr -> minutes ++;
      if ( ( currentTimePtr -> minutes ) >= ROLLOVER_MINUTES ) {
        currentTimePtr -> minutes = 0;
      }
      minutesDebounce = 0;
    }
  }

  /* Set Hours and Debounce */
  if ( HOURS_PB_SET && ( switchOperatingMode != setAlarmMode ) ) {
    hoursDebounce++;
    if ( hoursDebounce >= minimumDebounceTime ) {
      currentTimePtr -> hours ++;
      if ( ( currentTimePtr -> hours ) >= ROLLOVER_HOURS_24 ) {
        currentTimePtr -> hours = 0;
      }
      hoursDebounce = 0;
    }
  }

  /* Set Clock Time after 3 Seconds of Set Time Press */
  if ( SET_TIME_PB_SET ) {
    alarmDebounce++;
    if ( (alarmDebounce >= 30) && ( !setAlarmFlag ) ) {
      /* Global Flag to Stop Time */
      setAlarmFlag = 1;
      /* Reset All Times */
      currentTimePtr -> hours   = 0;
      currentTimePtr -> minutes = 0;
      currentTimePtr -> seconds = 0;
      /* Reset Debounce */
      alarmDebounce = 0;
    }

    /* Confirm Set Time after 3 Seconds of Set Alarm Press */
    if ( (alarmDebounce >= 30) && ( setAlarmFlag ) ) {
      /* Lower Alarm Flag to Resume Operation */
      setAlarmFlag = 0;
      /* Reset Time Counter */
      getTimersPtr -> highResTimer = 0;
      /* Compensate for the Debounce Time */
      currentTimePtr -> seconds = 3;
      /* Reset Debounce */
      alarmDebounce = 0;
    }
  }

  /* Set Alarm Time after 3 Seconds of Set Alarm Press */
  if ( SET_ALARM_PB_SET ) {
    alarmDebounce++;
    if ( (alarmDebounce >= 12) && ( switchOperatingMode != setAlarmMode ) ) {
      /* Set Alarm Operating Mode */
      switchOperatingMode = setAlarmMode;
      /* Reset Debounce */
      alarmDebounce = 0;
    }

    /* Confirm Set Time after 3 Seconds of Set Alarm Press */
    if ( (alarmDebounce >= 12) && ( switchOperatingMode == setAlarmMode ) ) {
      /* Resume Clock Operating Mode */
      switchOperatingMode = clockMode;
      /* Reset Time Counter */
      getTimersPtr -> highResTimer = 0;
      /* Reset Debounce */
      alarmDebounce = 0;
    }
  }

  /* Set Minutes and Debounce when in Alarm Mode */
  if ( MINUTES_PB_SET && ( switchOperatingMode == setAlarmMode ) ) {
    minutesDebounce++;
    if ( minutesDebounce >= minimumDebounceTime ) {
      currentTimePtr -> alarmMinutes ++;
      if ( ( currentTimePtr -> alarmMinutes ) >= ROLLOVER_MINUTES ) {
        currentTimePtr -> alarmMinutes = 0;
      }
      minutesDebounce = 0;
    }
  }

  /* Set Hours and Debounce */
  if ( HOURS_PB_SET && ( switchOperatingMode == setAlarmMode ) ) {
    hoursDebounce++;
    if ( hoursDebounce >= minimumDebounceTime ) {
      currentTimePtr -> alarmHours ++;
      if ( ( currentTimePtr -> alarmHours ) >= ROLLOVER_HOURS_24 ) {
        currentTimePtr -> alarmHours = 0;
      }
      hoursDebounce = 0;
    }
  }

return switchOperatingMode;

}

void soundAlarm( nixieOperatingMode_t switchOperatingMode ) {

  static const int timerTicks = TIMER_TICKS;     /* 8 Ticks = 1000 ms on main timer interrupt of 125 ms */
  static int alarmMinuteCheck = 0;
  static int alarmHourCheck = 0;
  static int alarmTiming = 0;

  /* Ensure at least 1 minute after start up has passed to sound alarm and alarm mode is not set */
  if ( (getTimersPtr -> startupTimer >= ( timerTicks * 60 ))
          && switchOperatingMode != setAlarmMode ) {
    /* Check Hours */
    if ( (currentTimePtr -> hours == currentTimePtr -> alarmHours) ) {
      alarmHourCheck = 1;
    }
    /* Check Minutes */
    if ( (currentTimePtr -> minutes == currentTimePtr -> alarmMinutes) ) {
      alarmMinuteCheck = 1;
    }
    /* Sound Alarm when Minutes and Hours have been verified */
    if ( (alarmMinuteCheck == 1) && (alarmHourCheck == 1) ) {
      if ( getTimersPtr -> alarmTimer >= 4 ) {
        DISABLE_ALARM();
      }
      if ( getTimersPtr -> alarmTimer >= timerTicks ) {
        getTimersPtr -> alarmTimer = 0;
        ENABLE_ALARM();
        alarmTiming++;
      }
    }
    /* Reset Alarm after Desired Alarm Time has Elapsed */
    if ( alarmTiming >= DESIRED_ALARM_TIME * 1 * 60 ) {
      DISABLE_ALARM();
      alarmHourCheck = 0;
      alarmMinuteCheck = 0;
      alarmTiming = 0;
      getTimersPtr -> startupTimer = timerTicks; /* Pat the Alarm Timer */
    }
  }
  /*End of Function */
}
