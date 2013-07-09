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

#ifndef irDecoder_h
#define irDecoder_h

#include "Arduino.h"		// Contains just about everything

#define IR_NUM_BITS 13		// Number of bits we are receiving
#define IR_DELAY 3			// Number of ticks -1 to wait after receiving a bit from ir
#define IR_TIMEOUT 2		// Number of ticks -1 before timing out on ir
#define IR_REPEAT_TIME 500	// 150 ms, repeats must fit within this time to qualify


/*Functions Prototypes********************************************************************/
		  
// This function sets up the library to decode IR on irPin.
void ir_begin(uint8_t _irPin);

// This functions returns a zero if there is no valid code present,
// it returns a code if there is one,
// and it returns -1 if the received codes are repeating.
int16_t ir_data(void);


#endif
