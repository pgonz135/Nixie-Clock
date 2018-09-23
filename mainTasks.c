/*
   mainTasks.c

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

nixieOperatingMode_t switchOperatingMode = clockMode;

/* State Machine */
const unsigned long tasksPeriodGCD  = TASK_PERIOD_GCD;
const unsigned long mainStatePeriod = MAIN_STATE_PERIOD;
const unsigned char tasksNum = 1;                  // Number of Tasks

enum BL_States {
  UPDATE_NIXIES = 0,                               // All states of the SM
  POLL_SWITCHES,
};

void initStateMachine() {
  /* Definition of Tasks*/
  unsigned char i = 0;                             // Counter for State Machine
  tasks[i].state = -1;                             // Initial state for as SMs
  tasks[i].period = mainStatePeriod;               // Period of this particular SM
  tasks[i].elapsedTime = tasks[i].period;          // Make time elapsed equal period of SM
  tasks[i].TickFct = &StateMachine_MainState;
  ++i;                                             // Move on to next SM
}

int StateMachine_MainState(int state)             // SM function definition
{
  /* State Transitions */
  switch (state)
  {
    case -1:
      state = UPDATE_NIXIES;
      BACKLIGHT_ON();
      break;

    case UPDATE_NIXIES:
      state = POLL_SWITCHES;
      break;

    case POLL_SWITCHES:
      state = UPDATE_NIXIES;
      break;

    default:
      state = -1;
  }

  /* State Actions */
  switch (state){

    case UPDATE_NIXIES:
      switchOperatingMode = pollSwitches();
      /* Update Nixies per Operating Mode */
      switch ( switchOperatingMode ) {

        case setAlarmMode:
          updateNixies( setAlarmMode );
          break;

        case clockMode:
          updateNixies( clockMode );
          //cathodePoisoningPrevention();
          break;

        case temperatureMode:
            break;

        default:
          updateNixies( clockMode );
      }
      /* Poll Alarm Function */
      soundAlarm( switchOperatingMode );
      break;

    case POLL_SWITCHES:
      switchOperatingMode = pollSwitches();
      break;

    default:
      break;
  }
  return state;
}


