/*
 * lab16-3.c
 *
 * Program which reads analog voltages on ADC0 (X) and ADC1 (Y) pins
 * and outputs the digital equivalent to the serial port. 
 * This uses a separate serial IO library which (a) requires
 * interrupts and (b) must be initialised using init_serial_stdio().
 * Standard input/output functions (printf etc.) are hooked up to the 
 * serial connection.
 */ 

#include "serialio.h"
#include "joystick.h"
#include <stdio.h>
#include <avr/interrupt.h>

// Our button queue. button_queue[0] is always the head of the queue. If we
// take something off the queue we just move everything else along. We don't
// use a circular buffer since it is usually expected that the queue is very
// short. In most uses it will never have more than 1 element at a time.
// This button queue can be changed by the interrupt handler below so we should
// turn off interrupts if we're changing the queue outside the handler.
#define JOYSTICK_Q_SIZE 4
#define SHOOT 3
#define LEFT 1
#define RIGHT 2
static volatile uint8_t joystick_queue[JOYSTICK_Q_SIZE];
static volatile int8_t queue_length = -1;

/* 0 = x_direction, 1 = y_direction */
uint8_t x_or_y = 0;	
uint16_t value, up_down_cal, left_right_cal;

void init_joystick(void) {
	/* Turn on global interrupts */
	sei();
	
	// Set up ADC - AVCC reference, right adjust
	// Input selection doesn't matter yet - we'll swap this around in the while
	// loop below.
	ADMUX = (1 << REFS0);
	// Turn on the ADC (but don't start a conversion yet). Choose a clock
	// divider of 64. (The ADC clock must be somewhere
	// between 50kHz and 200kHz. We will divide our 8MHz clock by 64
	// to give us 125kHz.)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
	ADMUX = 0;
	ADMUX |= (1 << REFS0);

	// Start the ADC conversion
	ADCSRA |= (1 << ADSC);
	
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	left_right_cal = ADC;
	
	ADMUX |= (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0);
	// Start the ADC conversion
	ADCSRA |= (1 << ADSC);
	
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	up_down_cal = ADC;
}

void step_joystick() {
	if (queue_length > 3) {
		return;
	}
	// Set the ADC mux to choose ADC0 if x_or_y is 0, ADC1 if x_or_y is 1
	if(x_or_y == 0) {
		ADMUX = 0;
		ADMUX |= (1 << REFS0);
	} else {
		ADMUX |= (0 << MUX4) | (0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0);
	}
	// Start the ADC conversion
	ADCSRA |= (1 << ADSC);
	
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	value = ADC; // read the value
	
	// Set it to the appropriate movement and increment the queue size.
	if(x_or_y == 1 && (value < up_down_cal - 100 || value > up_down_cal + 100)) {
		joystick_queue[queue_length++] = 3;
	} else if(value < left_right_cal - 100) {
		joystick_queue[queue_length++] = 2;
	} else if(value > left_right_cal + 100) {
		joystick_queue[queue_length++] = 1;
	}
	// Next time through the loop, do the other direction
	x_or_y ^= 1;
}

int8_t joystick_moved(void) {
	int8_t return_value = NO_JOYSTICK_MOVEMENT;	// Assume no joystick movement
	if(queue_length > 0) {
		// Remove the first element off the queue and move all the other
		// entries closer to the front of the queue. We turn off interrupts (if on)
		// before we make any changes to the queue. If interrupts were on
		// we turn them back on when done.
		return_value = joystick_queue[0];
		
		for(uint8_t i = 1; i < queue_length; i++) {
			joystick_queue[i-1] = joystick_queue[i];
		}
		queue_length--;
	}
	return return_value;
}


