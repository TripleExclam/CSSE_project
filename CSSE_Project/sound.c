/*
 * sound.c
 *
 * Author: Matt Burton
 *
 * A piezo buzzer and an LED should both be connected to the OC1B pin
 * (port D, pin 4). (The other end of the piezo buzzer should be connected
 * to ground (0V).)
 */

#include <avr/io.h>
#define F_CPU 8000000UL	// 8MHz
#include <stdlib.h>
/* Stdlib needed for random() - random number generator */

uint16_t	notes[7] = {261, 294, 329, 349, 392, 440, 494};
// For a given frequency (Hz), return the clock period (in terms of the
// number of clock cycles of a 1MHz clock)
static uint16_t freq_to_clock_period(uint16_t freq) {
	return (1000000UL / freq);	// UL makes the constant an unsigned long (32 bits)
								// and ensures we do 32 bit arithmetic, not 16
}

// Return the width of a pulse (in clock cycles) given a duty cycle (%) and
// the period of the clock (measured in clock cycles)
static uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clockperiod) {
	return (dutycycle * clockperiod) / 100;
}

// Turn the sound off
void kill_sound() {
	TCCR1A = 0;
	TCCR1B = 0;
}

void init_sound() {
	// Make pin OC1B be an output
	if ((PIND & (1 << 6)) >> 6) {
		DDRD |= (1 << 4);
	
		// Set up timer/counter 1 for Fast PWM, counting from 0 to the value in OCR1A
		// before reseting to 0. Count at 1MHz (CLK/8).
		// Configure output OC1B to be clear on compare match and set on timer/counter
		// overflow (non-inverting mode).
		TCCR1A = (1 << COM1B1) |(0 << COM1B0) | (1 << WGM11) | (1 << WGM10);
		TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);
	}
}

void set_sound(uint16_t freq, float dutycycle) {
	uint16_t clockperiod = freq_to_clock_period(freq);
	uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

	// Set the maximum count value for timer/counter 1 to be one less than the clockperiod
	OCR1A = clockperiod - 1;
	
	// Set the count compare value based on the pulse width. The value will be 1 less
	// than the pulse width - unless the pulse width is 0.
	if(pulsewidth == 0) {
		OCR1B = 0;
	} else {
		OCR1B = pulsewidth - 1;
	}
}

// Play a random sound
void random_sound() {
	set_sound(notes[random() % 7], 2);
}


