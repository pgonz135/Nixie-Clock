/*
 * nixieDriver.c
 *
 *  Created on: Sep 16, 2018
 *      Author: Administrator
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

void updateNixies( nixieOperatingMode_t nixieOperatingMode ) {

  uint64_t buffer[65];                                  /* nixie buffer */
  uint16_t i = 0;
  uint16_t index = 0;
  uint16_t localSecondsCounter = 0;
  uint16_t localMinutesCounter = 0;
  uint16_t localHoursCounter = 0;

  if ( nixieOperatingMode == clockMode ) {

    /*Seconds Ones Place*/
    localSecondsCounter = currentTimePtr -> seconds;      /* collect current seconds*/

    if ( localSecondsCounter >= 10 ) {                    /* if seconds exceed 10, take remainder */
      localSecondsCounter = localSecondsCounter % 10;
    }

    index = 54 + localSecondsCounter;                     /* fix the index in the array */

    for ( i = 55; i < 64; i++ ) {                         /* load the buffer */
      /* Put a 0 in the Tens Place */
      if ( ( currentTimePtr -> seconds ) < 10 ) {
        buffer[54] = 1;
      }
      else {
        buffer[54] = 0;
      }
      /* Digits 1-9 */
      if ( i == index ) {
        buffer[i] = 1;
      }
      else {
        buffer[i] = 0;
      }
    }

    /*Seconds Tens Place*/
    if ( (currentTimePtr -> seconds)  >= 10 ) {             /* are the seconds greater than 10? */

      localSecondsCounter = currentTimePtr -> seconds;      /* bring in locally */
      localSecondsCounter = localSecondsCounter % 10;       /* find the remainder when divided by 10 */

      if ( 0 == (localSecondsCounter)  ) {                  /* zero remainder means increment tens place */

        index = 44 + (currentTimePtr -> seconds) / 10;      /* fix the index in the array and reposition */

        buffer[64] = 1;                                     /* put a zero in the ones place */

        for ( i = 45; i < 54; i++ ) {                       /* load the buffer */
          /* Digits 1-9 */
          if ( i == index ) {
            buffer[i] = 1;
          }
          else {
            buffer[i] = 0;
          }
        }
      }
    }

    /*Minutes Ones Place*/
    if ( currentTimePtr -> minutes >= 1 ) {

      localMinutesCounter = currentTimePtr -> minutes;      /* collect current seconds*/

      if ( localMinutesCounter >= 10 ) {                    /* if seconds exceed 10, take remainder */
        localMinutesCounter = localMinutesCounter % 10;
      }

      index = 34 + localMinutesCounter;                     /* fix the index in the array */

      for ( i = 35; i < 44; i++ ) {                         /* load the buffer */

        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /*Minutes Tens Place*/
    if ( (currentTimePtr -> minutes)  >= 10 ) {             /* are the minutes greater than 10? */

      localMinutesCounter = currentTimePtr -> minutes;      /* bring in locally */
      localMinutesCounter = localMinutesCounter % 10;       /* find the remainder when divided by 10 */

      if ( 0 == (localMinutesCounter)  ) {                  /* zero remainder means increment tens place */
        buffer[44] = 1;                                     /* put a zero in the ones place */
        buffer[34] = 0;                                     /* remove the zero from the tens places */
      }
      index = 24 + (currentTimePtr -> minutes) / 10;      /* fix the index in the array and reposition */

      //   buffer[34] = 0;                                     /* remove the zero from the tens places */

      for ( i = 25; i < 34; i++ ) {                       /* load the buffer */
        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /*Hours Ones Place*/
    if ( currentTimePtr -> hours >= 1 ) {

      localHoursCounter = currentTimePtr -> hours;      /* collect current seconds*/

      if ( localHoursCounter >= 10 ) {                    /* if seconds exceed 10, take remainder */
        localHoursCounter = localHoursCounter % 10;
      }

      buffer[14] = 1;
      index = 14 + localHoursCounter;                     /* fix the index in the array */

      for ( i = 15; i < 25; i++ ) {                         /* load the buffer */

        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /*Hours Tens Place*/
    if ( (currentTimePtr -> hours)  >= 10 ) {             /* are the minutes greater than 10? */

      localHoursCounter = currentTimePtr -> hours;      /* bring in locally */
      localHoursCounter = localHoursCounter % 10;       /* find the remainder when divided by 10 */

      index = 4 + (currentTimePtr -> hours) / 10;      /* fix the index in the array and reposition */

      buffer[14] = 0;                                     /* put a zero in the ones place */
      if ( 0 == (localHoursCounter) ) {
        buffer[24] = 1;
      }/* remove the zero from the tens places */

      else {
        buffer[24] = 0;

      }

      for ( i = 5; i < 15; i++ ) {                       /* load the buffer */
        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /* Rollover after 60 seconds */
    if ( (currentTimePtr -> seconds ) == 0 ) {
      /*Clean the Upper Byte*/
      for ( i = 45; i <= 54; i++) {
        buffer[i] = 0;
      }
      /* Put a 0 in the Ones and Tens Place for 0 (60) seconds */
      buffer[64] = 1;
      buffer[54] = 1;
    }

    /*   Rollover after 60 minutes */
    if ( (currentTimePtr -> minutes ) == 0 ) {
      for ( i = 25; i <= 44; i++) {
        buffer[i] = 0;
      }
      /* Put a 0 in the Ones and Tens Place for 0 (60) minutes */
      buffer[44] = 1;
      buffer[34] = 1;
    }

    /*   Rollover after 24 hours */
    if ( (currentTimePtr -> hours ) == 0 ) {
      for ( i = 15; i <= 24; i++) {
        buffer[i] = 0;
      }
      /* Put a 0 in the Ones and Tens Place for 0 (60) minutes */
      buffer[14] = 1;
      buffer[24] = 1;
    }

    /* Shift out the buffer */
    shiftOut(buffer);

    /* Clean up the upper byte for the seconds ones place */
    for ( i = 55; i <= 64; i++) {
      buffer[i] = 0;
    }

    /* Clean up the upper byte for the minutes ones place */
    for ( i = 35; i <= 44; i++) {
      buffer[i] = 0;
    }

    /* Clean up the upper byte for the minutes ones place */
    for ( i = 5; i <= 24; i++) {
      buffer[i] = 0;
    }
  }


  if ( nixieOperatingMode == setAlarmMode ) {

    /*Seconds Ones Place*/
    localSecondsCounter = currentTimePtr -> alarmSeconds;      /* collect current seconds*/

    if ( localSecondsCounter >= 10 ) {                    /* if seconds exceed 10, take remainder */
      localSecondsCounter = localSecondsCounter % 10;
    }

    index = 54 + localSecondsCounter;                     /* fix the index in the array */

    for ( i = 55; i < 64; i++ ) {                         /* load the buffer */
      /* Put a 0 in the Tens Place */
      if ( ( currentTimePtr -> alarmSeconds ) < 10 ) {
        buffer[54] = 1;
      }
      else {
        buffer[54] = 0;
      }
      /* Digits 1-9 */
      if ( i == index ) {
        buffer[i] = 1;
      }
      else {
        buffer[i] = 0;
      }
    }

    /*Seconds Tens Place*/
    if ( (currentTimePtr -> alarmSeconds)  >= 10 ) {             /* are the seconds greater than 10? */

      localSecondsCounter = currentTimePtr -> alarmSeconds;      /* bring in locally */
      localSecondsCounter = localSecondsCounter % 10;       /* find the remainder when divided by 10 */

      if ( 0 == (localSecondsCounter)  ) {                  /* zero remainder means increment tens place */

        index = 44 + (currentTimePtr -> alarmSeconds) / 10;      /* fix the index in the array and reposition */

        buffer[64] = 1;                                     /* put a zero in the ones place */

        for ( i = 45; i < 54; i++ ) {                       /* load the buffer */
          /* Digits 1-9 */
          if ( i == index ) {
            buffer[i] = 1;
          }
          else {
            buffer[i] = 0;
          }
        }
      }
    }

    /*Minutes Ones Place*/
    if ( currentTimePtr -> alarmMinutes >= 1 ) {

      localMinutesCounter = currentTimePtr -> alarmMinutes;      /* collect current seconds*/

      if ( localMinutesCounter >= 10 ) {                    /* if seconds exceed 10, take remainder */
        localMinutesCounter = localMinutesCounter % 10;
      }

      index = 34 + localMinutesCounter;                     /* fix the index in the array */

      for ( i = 35; i < 44; i++ ) {                         /* load the buffer */

        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /*Minutes Tens Place*/
    if ( (currentTimePtr -> alarmMinutes)  >= 10 ) {             /* are the minutes greater than 10? */

      localMinutesCounter = currentTimePtr -> alarmMinutes;      /* bring in locally */
      localMinutesCounter = localMinutesCounter % 10;       /* find the remainder when divided by 10 */

      if ( 0 == (localMinutesCounter)  ) {                  /* zero remainder means increment tens place */
        buffer[44] = 1;                                     /* put a zero in the ones place */
        buffer[34] = 0;                                     /* remove the zero from the tens places */
      }
      index = 24 + (currentTimePtr -> alarmMinutes) / 10;      /* fix the index in the array and reposition */

      //   buffer[34] = 0;                                     /* remove the zero from the tens places */

      for ( i = 25; i < 34; i++ ) {                       /* load the buffer */
        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /*Hours Ones Place*/
    if ( currentTimePtr -> alarmHours >= 1 ) {

      localHoursCounter = currentTimePtr -> alarmHours;      /* collect current seconds*/

      if ( localHoursCounter >= 10 ) {                    /* if seconds exceed 10, take remainder */
        localHoursCounter = localHoursCounter % 10;
      }

      buffer[14] = 1;
      index = 14 + localHoursCounter;                     /* fix the index in the array */

      for ( i = 15; i < 25; i++ ) {                         /* load the buffer */

        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /*Hours Tens Place*/
    if ( (currentTimePtr -> alarmHours)  >= 10 ) {             /* are the minutes greater than 10? */

      localHoursCounter = currentTimePtr -> alarmHours;      /* bring in locally */
      localHoursCounter = localHoursCounter % 10;       /* find the remainder when divided by 10 */

      index = 4 + (currentTimePtr -> alarmHours) / 10;      /* fix the index in the array and reposition */

      buffer[14] = 0;                                     /* put a zero in the ones place */
      if ( 0 == (localHoursCounter) ) {
        buffer[24] = 1;
      }/* remove the zero from the tens places */

      else {
        buffer[24] = 0;

      }

      for ( i = 5; i < 15; i++ ) {                       /* load the buffer */
        /* Digits 1-9 */
        if ( i == index ) {
          buffer[i] = 1;
        }
        else {
          buffer[i] = 0;
        }
      }
    }

    /* Rollover after 60 seconds */
    if ( (currentTimePtr -> alarmSeconds ) == 0 ) {
      /*Clean the Upper Byte*/
      for ( i = 45; i <= 54; i++) {
        buffer[i] = 0;
      }
      /* Put a 0 in the Ones and Tens Place for 0 (60) seconds */
      buffer[64] = 1;
      buffer[54] = 1;
    }

    /*   Rollover after 60 minutes */
    if ( (currentTimePtr -> alarmMinutes ) == 0 ) {
      for ( i = 25; i <= 44; i++) {
        buffer[i] = 0;
      }
      /* Put a 0 in the Ones and Tens Place for 0 (60) minutes */
      buffer[44] = 1;
      buffer[34] = 1;
    }

    /*   Rollover after 24 hours */
    if ( (currentTimePtr -> alarmHours ) == 0 ) {
      for ( i = 15; i <= 24; i++) {
        buffer[i] = 0;
      }
      /* Put a 0 in the Ones and Tens Place for 0 (60) minutes */
      buffer[14] = 1;
      buffer[24] = 1;
    }

    /* Shift out the buffer */
    shiftOut(buffer);

    /* Clean up the upper byte for the seconds ones place */
    for ( i = 55; i <= 64; i++) {
      buffer[i] = 0;
    }

    /* Clean up the upper byte for the minutes ones place */
    for ( i = 35; i <= 44; i++) {
      buffer[i] = 0;
    }

    /* Clean up the upper byte for the minutes ones place */
    for ( i = 5; i <= 24; i++) {
      buffer[i] = 0;
    }
  }

  /* Refresh Nixies every 10th of the Hour and 10 seconds */
  //if( ((currentTimePtr -> minutes ) == 10) && ((currentTimePtr -> seconds ) == 10) ){
  //cathodePoisoningPrevention();
  //}
}

void shiftOut( uint64_t buffer[] ) {

  uint8_t i = 0;

  /* Send Buffer to  HV5622 */
  RST_LE();
  for ( i = 0; i <= 64; i++ ) {
    if ( buffer[i] == 1 ) {
      SET_DIN();
    }
    else {
      buffer[i] = 0;
      RST_DIN();
    }
    /* Toggle Clock High then Low */
    SET_CLK();
    RST_CLK();
  }
  /* set latches */
  SET_LE();
  SET_BL();
}

void cathodePoisoningPrevention( void ) {

  int i = 0;
  static uint64_t cathodeBuffer[65];
  int b = 1;
  uint32_t stop=0;

  for ( i = 0; i < 64; i++ ) {                         /* load the buffer */

    cathodeBuffer[i] = b >> i;

    RST_LE();
    if ( cathodeBuffer[i] == 1 ) {
      SET_DIN();
    }
    else {
      cathodeBuffer[i] = 0;
      RST_DIN();
    }
    /* toggle high then low */
    SET_CLK();
    RST_CLK();
    /* set latches */
    SET_LE();
    SET_BL();

/*    uint16_t currentVal = getTimersPtr -> cathodeRefreshTimer;

    while(   getTimersPtr -> cathodeRefreshTimer <= currentVal + 5 ){
    }*/

    for(stop=0;stop<500;stop++){

    }

  }
}

