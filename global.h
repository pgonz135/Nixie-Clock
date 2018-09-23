/*
 * global.h
 *
 *  Created on: Sep 16, 2018
 *      Author: Administrator
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define DESIRED_ALARM_TIME 1    /* Desired Alarm Time in Minutes */

extern uint8_t seconds;
extern uint8_t minutes;
extern uint8_t hours;

#define ROLLOVER_SECONDS    60
#define ROLLOVER_MINUTES    60
#define ROLLOVER_HOURS_24   24
#define ROLLOVER_HOURS_12   12
#define TIMER_TICKS         20

#define MINUTES_PB_SET    !( P1IN & BIT7 )
#define HOURS_PB_SET      !( P4IN & BIT4 )
#define SET_ALARM_PB_SET  !( P4IN & BIT5 )
#define SET_TIME_PB_SET   !( P4IN & BIT6 )
#define BACKLIGHT_ON()       P2OUT |=  BIT2;
#define BACKLIGHT_OFF()      P2OUT &= ~BIT2;


struct time {
  volatile uint8_t seconds;
  volatile uint8_t minutes;
  volatile uint8_t hours;
  volatile uint8_t alarmSeconds;
  volatile uint8_t alarmMinutes;
  volatile uint8_t alarmHours;
};

struct timers {
  volatile uint32_t highResTimer;
  volatile uint16_t lowResTimer;
  volatile uint16_t taskSchedulerTimer;
   uint16_t cathodeRefreshTimer;
  volatile uint16_t alarmTimer;
  volatile uint32_t startupTimer;
};

extern uint8_t setAlarmFlag;

/* Declare Global copies of Structures and Initialize */
extern struct time   currentTime;
extern struct timers getTimers;
/* Declaring Pointers to Structures for Access */
struct time   *currentTimePtr;
struct timers *getTimersPtr;

void initPorts( void );
void initUCS( void );
nixieOperatingMode_t pollSwitches( void );
void soundAlarm( nixieOperatingMode_t );

#endif /* GLOBAL_H_ */
