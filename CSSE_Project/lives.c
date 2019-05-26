/*
 * lives.c
 *
 * Written by Matt Burton
 */

#include "lives.h"
#include <avr/io.h>

uint32_t lives;

void init_lives(void) {
	// Leave the joystick alone
	PORTA &= (1 << 0) | (1 << 1);
	for (int8_t i = 0; i < 4; i++) {
		// Set the last four bits to the number of live -> 2^{lives}.
		PORTA |= (1 << (4 + i));
	}
	lives = 4;
}

void add_to_lives(int16_t value) {
	lives += value;
	// Reset the last seven bits.
	PORTA &= (1 << 0) | (1 << 1);
	// Does the lights in the right order.
	for (int8_t i = 1; i < lives + 1; i++) {
		// Set the last four bits to the number of live -> 2^{lives}.
		PORTA |= (1 << (7 - i));
	}
}

uint32_t get_lives(void) {
	return lives;
}
