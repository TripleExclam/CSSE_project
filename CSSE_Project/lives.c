/*
 * lives.c
 *
 * Written by Matt Burton
 */

#include "lives.h"
#include <avr/io.h>

uint32_t lives;

void init_lives(void) {
	PORTC &= 1;
	for (int8_t i = 0; i < 4; i++) {
		// Set the last four bits to the number of live -> 2^{lives}.
		PORTC |= (1 << (4 + i));
	}
	lives = 4;
}

void add_to_lives(int16_t value) {
	lives += value;
}

uint32_t get_lives(void) {
	return lives;
}
