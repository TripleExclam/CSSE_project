/*
 * lives.c
 *
 * Written by Matt Burton
 */

#include "lives.h"

uint32_t lives;

void init_lives(void) {
	lives = 4;
}

void add_to_lives(int16_t value) {
	lives += value;
}

uint32_t get_lives(void) {
	return lives;
}
