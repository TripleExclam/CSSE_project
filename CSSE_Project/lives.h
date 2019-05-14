/*
 * lives.h
 * 
 * Author: Matt Burton
 */

#ifndef LIVES_H_
#define LIVES_H_

#include <stdint.h>

void init_lives(void);
void add_to_lives(int16_t value);
uint32_t get_lives(void);

#endif /* LIVES_H_ */