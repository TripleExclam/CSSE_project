/*
 * SEVEN_SEG.c
 *
 * Written by Matt Burton
 */

#include "seven_seg.h"
#include "timer0.h"
#include <avr/io.h>

uint8_t	seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};
uint16_t display_value;
uint32_t previous_time;
volatile uint8_t seven_seg_cc;

void init_display(void) {
	// Set Port C to output the digits.
	DDRC = 0xFF;
	// Port C pin 0 to oscillate between digits.
	DDRD |= (1 << 3);
	// The side to display on, 0 -> right, 1 -> left
	seven_seg_cc = 0;
}

void display_data(uint32_t current_time) {
	/* Displays the value on the seven segment display. 
	Wraps around at 100. The refresh rate is every 3 milliseconds. 
	Might need to use above method to improve performance.
	*/
	if (current_time > previous_time + 3) {
		// Save the last time
		previous_time = current_time;
		// Only display the last digit
		if (display_value < 10) {
			seven_seg_cc = 0;
			PORTC = seven_seg_data[display_value % 10];
		} else {
			seven_seg_cc = 1 - seven_seg_cc;
			if (seven_seg_cc == 0) {
				// Set the first digit
				PORTC = seven_seg_data[display_value % 10];
			} else {
				// Set the second digit
				PORTC = seven_seg_data[(display_value / 10) % 10];
			}
		}
		/* Output the digit selection (CC) bit */
		PORTD = (PORTD & ~(1 << 3)) | (seven_seg_cc << 3);
	}
}

void update_time(uint32_t time) {
	previous_time = time;
}

void set_value(uint16_t value) {
	// Set the value (first two digits) on the display
	display_value = value;
}


