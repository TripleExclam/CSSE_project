/*
 * SEVEN_SEG.h
 * 
 * Author: Matt Burton
 */

#ifndef SEVEN_SEG_H_
#define SEVEN_SEG_H_

#include <stdint.h>

void init_display(void);
void display_data(uint32_t current_time);
void update_time(uint32_t time);
void set_value(uint16_t value);

#endif /* SEVEN_SEG_H_*/