
/******************************************************************************************
 * An Improved IR decoder for Arduino
 * 
 * Copyright (c) Robert Bakker 2013
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 ******************************************************************************************
 * This library handles polling and decoding of the signal from an IR remote.
 * It is non-blocking code that executes in the background while your sketch
 * does it's thing.
 * The ir decoder works only for the Phillips RC-5 protocol.
 *****************************************************************************************/

#include "irDecoderV2.h"
#include "Arduino.h"

// Used by state machines in the ISR
#define STANDBY 0
#define DELAYING 1
#define WAITING 2
#define REPEAT_CHECK 3
#define IDLE 4

/*Privite Function Prototypes*************************************************************/

inline void irDemod(void);


/*Declare Variables***********************************************************************/

struct irVars
{
	volatile int8_t state;
	
	volatile uint8_t pin;
	
	volatile int8_t pinState;
	volatile int8_t pinStateOld;
	
	volatile int16_t data;
	volatile int16_t dataBuffer;
	volatile int8_t dataAvail;
	volatile int8_t dataRepeat;
	volatile uint16_t repeatTimer;
	
	volatile uint8_t counter;
	volatile int8_t dataIndex;
};

// Initialize variables (start all state machines in STANDBY state)
struct irVars ir = { STANDBY, 0, 0, 0, 0, 0, 0, 0, 0, 0, IR_NUM_BITS };


/*Start of Functions**********************************************************************/

// This function sets up timer2 to trigger an ISR every 300 us.
// It also sets up the input pin.
void ir_begin(uint8_t _irPin)
{
	ir.pin = _irPin;
	
	pinMode(ir.pin, INPUT);
	
	// Configure timer 2
	cli();					// Disable global interrupts
	
	TCCR2A = 0;				// Clear timer2's control registers
	TCCR2B = 0;
	TIMSK2 = 0;				// ...and interrupt mask register (just in case)
	TCNT2 = 0;				// Pre-load the timer to 0
	OCR2A = 149;			// Set output compare register to 149
	TCCR2A |= _BV(WGM21);	// Turn on CTC mode (Clear Timer on Compare match)
	TCCR2B |= 0b011;		// Set prescaler to 32 (starts timer) 
	TIMSK2 |= _BV(OCIE2A);	// Enable timer compare interrupt 

	sei();					// Re-enable global interrupts
}

// Blah
int16_t ir_data(void)
{
	if(ir.dataAvail)
	{
		int16_t temp = ir.dataBuffer & 0x87FF;	// Strip off 2nd and toggle bits
		ir.dataAvail = 0;
		return temp;
	}
	
	else if(ir.dataRepeat == 1)
		return -1;
	
	else
		return 0;
}


/*ISR*************************************************************************************/

ISR(TIMER2_COMPA_vect)
{	
	ir.pinState = !digitalRead(ir.pin);	
	irDemod();	
}


/*Private Functions***********************************************************************/

inline void irDemod(void)
{
	switch(ir.state)
	{
		case STANDBY:	// Do nothing until the pin goes high.
		if(ir.pinState)
			ir.state = DELAYING;
		break;

		case DELAYING:	// Wait for 1200 uS.
		ir.counter++;
		if(ir.counter > IR_DELAY)
		{
			ir.counter = 0;			// The counter variable is re-used so it needs to be reset.
			
			if(ir.dataIndex < 1)	// If we're on the last bit.
			{
				ir.dataIndex = IR_NUM_BITS;
				ir.state = REPEAT_CHECK;
			}
			
			else
			{
				ir.pinStateOld = ir.pinState;
				ir.state = WAITING;
			}
		}
		break;

		case WAITING:	// Wait for the pin to change state.
		if(ir.pinState != ir.pinStateOld)
		{
			ir.dataIndex--;
			
			if(ir.pinState)
				ir.data |= _BV(ir.dataIndex);
			
			ir.counter = 0;
			ir.state = DELAYING;
		}

		// If nothing happens before the timeout (900 uS)
		else
		{
			ir.counter++;
			if(ir.counter > IR_TIMEOUT)
			{
				ir.counter = 0;
				ir.data = 0;
				ir.dataIndex = IR_NUM_BITS;
				ir.state = STANDBY;
			}
		}
		break;
		
		case REPEAT_CHECK:
		if(ir.dataRepeat)
		{
			if(ir.data == ir.dataBuffer)
			{
				ir.repeatTimer = 0;
				ir.dataRepeat = 1;
			}
			else
				ir.dataRepeat = 0;
		}
		
		if((!ir.dataRepeat) && (!ir.dataAvail))	// Ready to accept new code.
		{
			ir.dataBuffer = ir.data;
			ir.repeatTimer = 0;
			ir.dataRepeat = 2;	// We're not sure if the next code will be a repeat, so check it next time.
			ir.dataAvail = 1;
		}
		
		ir.data = 0;
		ir.state = STANDBY;
		break;
	}
	
	if(ir.repeatTimer < IR_REPEAT_TIME)
		ir.repeatTimer++;
	else
		ir.dataRepeat = 0;	// We've timed out, can't be a repeat.
}