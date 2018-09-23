/*
 * nixieDriver.h
 *
 *  Created on: Sep 16, 2018
 *      Author: Administrator
 */

#ifndef NIXIEDRIVER_H_
#define NIXIEDRIVER_H_

/* HV5622 Pin Function Map */
#define SET_BL()        ( P1OUT |=   BIT0 )
#define SET_DIN()       ( P1OUT |=   BIT1 )
#define SET_LE()        ( P1OUT |=   BIT2 )
#define SET_CLK()       ( P1OUT |=   BIT3 )
#define RST_BL()        ( P1OUT &=  ~BIT0 )
#define RST_DIN()       ( P1OUT &=  ~BIT1 )
#define RST_LE()        ( P1OUT &=  ~BIT2 )
#define RST_CLK()       ( P1OUT &=  ~BIT3 )
#define TOGGLE_LE()     ( P1OUT ^=   BIT2 )
#define TOGGLE_CLK()    ( P1OUT ^=   BIT3 )

typedef enum nixieOperatingMode{
    clockMode,
    setAlarmMode,
    temperatureMode,
}nixieOperatingMode_t;

void updateNixies( nixieOperatingMode_t );
void cathodePoisoningPrevention( void );
void shiftOut( uint64_t buffer[] );

#endif /* NIXIEDRIVER_H_ */
