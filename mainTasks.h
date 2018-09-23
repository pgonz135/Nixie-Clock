/*
   mainTasks.h

    Created on: Sep 16, 2018
        Author: Administrator
*/

#ifndef MAINTASKS_H_
#define MAINTASKS_H_

#include <msp430.h>
#include <stdio.h>

/* SM Constants */
#define MAIN_STATE_PERIOD     ( 500 / 2 )  /* Period of the Main State */
#define TASK_PERIOD_GCD               125  /* GCD of All Tasks Period: Make Timer Tick as this Rate */
#define TIMER_TICK_COUNTS   ( 32780 / 20 ) /* 32768/8 = 4096 - or 1000ms/8 = 125ms */

/* State Machine */
extern const unsigned long tasksPeriodGCD;
extern const unsigned long mainStatePeriod;

/*Structure for task*/
typedef struct stateMachineTasks
{
  int state;                                        // Current state of the task
  unsigned long period;                             // Rate at which the task should tick
  unsigned long elapsedTime;                        // Time since task's previous tick
  int (*TickFct)(int);                              // Function to call for task's tick
} stateMachineTasks;

/*State Machine Functions*/
int StateMachine_MainState(int state);
void initStateMachine();

/* Hardcoded Number of Tasks */
stateMachineTasks tasks[1];                         // Number of tasks must be hard-coded
extern const unsigned char tasksNum;                   // Number of Tasks

#endif /* MAINTASKS_H_ */
