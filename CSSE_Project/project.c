/*
 * project.c
 *
 * Main file
 *
 * Author: Peter Sutton. Modified by Matt Burton
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "lives.h"
#include "score.h"
#include "sound.h"
#include "timer0.h"
#include "seven_seg.h"
#include "game.h"

#define F_CPU 8000000L
#include <util/delay.h>

// Function prototypes - these are defined below (after main()) in the order
// given here.
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);


// ASCII code for Escape character
#define ESCAPE_CHAR 27

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	init_timer0();
	
	// Initialise the seven_seg display, 
	// with PORT A and PORT C pin 0 as outputs.
	// Initialise PORT C to output the number of lives
	init_display();
	
	// Turn on global interrupts
	sei();
}

void delay_ms(uint16_t count) {
	while(count--) {
		_delay_ms(1);

	}
}

void splash_screen(void) {
	uint32_t current_time = get_current_time();
	uint32_t note_time = current_time;
	// Enjoy this trash song.
	static uint16_t theme_song[30] = {123, 146, 164, 155, 146, 185, 174, 146, 164, 155, 
		138, 155, 116, 20000, 123, 146, 164, 155, 146, 185, 207, 196, 185, 174, 185, 174, 123, 164, 146}; 
	static uint16_t	delays[30] = {165, 165, 83, 165, 333, 160, 500, 190, 120, 165, 
		333, 165, 500, 500, 165, 165, 83, 165, 333, 160, 333, 165, 333, 165, 83, 400, 333, 165, 800}; 
	uint8_t i = 0;
	// Clear terminal screen and output a message
	clear_terminal();
	move_cursor(10,10);
	printf_P(PSTR("Asteroids"));
	move_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 project by Matthew Burton"));
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	while(1) {
		set_scrolling_display_text("ASTEROIDS MATTHEW BURTON S45293867", COLOUR_GREEN);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed
		
		while(scroll_display()) {
			current_time = get_current_time();
			// Every specified time interval, change the sound.
			if (current_time >= note_time + delays[i]) {
				init_sound();
				set_sound(theme_song[i] + 300, 0.5);
				i = (i + 1) % 30;
				note_time = current_time;
			}
			_delay_ms(100);
			// Play the sound for 100ms then end it.
			kill_sound();
			if(button_pushed() != NO_BUTTON_PUSHED) {
				return;
			}
		}
	}
}

void new_game(void) {
	// Initialise the game and display
	initialise_game();
	
	// Clear the serial terminal
	clear_terminal();
	move_cursor(2,2);
	printf_P(PSTR("Asteroids"));
	
	// Initialise the score
	init_score();
	move_cursor(2,4);
	printf_P(PSTR("Score: %lu"), get_score());
	
	// Initialise lives.
	init_lives();

	move_cursor(2, 6);
	printf_P(PSTR("You have %lu lives remaining."), get_lives());
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	uint32_t current_time, last_move_time, last_move_asteroid;
	int8_t button;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	uint8_t sound_duration_1 = 0;
	
	// Get the current time and remember this as the last time the projectiles
    // were moved.
	current_time = get_current_time();
	last_move_time = current_time;
	last_move_asteroid = current_time;
	
	// We play the game until it's over
	while(!is_game_over()) {
		
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. At most one of the following three
		// variables will be set to a value other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		
		if(button == NO_BUTTON_PUSHED) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		
		// Process the input. 
		if(button==3 || escape_sequence_char=='D' || serial_input=='L' || serial_input=='l') {
			// Button 3 pressed OR left cursor key escape sequence completed OR
			// letter L (lowercase or uppercase) pressed - attempt to move left
			if(move_base(MOVE_LEFT)) {
				init_sound();
				set_sound(600, 2);
				sound_duration_1 = 255;
			}
		} else if(button==2 || escape_sequence_char=='A' || serial_input==' ') {
			// Button 2 pressed or up cursor key escape sequence completed OR
			// space bar pressed - attempt to fire projectile
			if (fire_projectile()) {
				init_sound();
				set_sound(494, 2);
				sound_duration_1 = 255;
			}
		} else if(button==1 || escape_sequence_char=='B') {
			// Button 1 pressed OR down cursor key escape sequence completed
			// Ignore at present
		} else if(button==0 || escape_sequence_char=='C' || serial_input=='R' || serial_input=='r') {
			// Button 0 pressed OR right cursor key escape sequence completed OR
			// letter R (lowercase or uppercase) pressed - attempt to move right
			if(move_base(MOVE_RIGHT)) {
				init_sound();
				set_sound(600, 2);
				sound_duration_1 = 255;
			}
		} else if(serial_input == 'p' || serial_input == 'P') {
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			// pressed again
			toggle_timer();
			while(1) {
				if(serial_input_available()) {
					// Serial data was available - read the data from standard input
					serial_input = fgetc(stdin);
					if (serial_input == 'p' || serial_input == 'P') {
						break;
					}
				}
			}
			toggle_timer();
		} 
		
		if (sound_duration_1 == 0) {
			kill_sound();
		} else {
			sound_duration_1--;
		}
		
		current_time = get_current_time();
		if(!is_game_over() && current_time >= last_move_time + 200) {
			// 500ms (0.5 second) has passed since the last time we moved
			// the projectiles - move them - and keep track of the time we 
			// moved them
			advance_projectiles();
			
			last_move_time = current_time;
		}

		if(current_time >= last_move_asteroid + 1000 - get_score() * 5) {
			// 1000ms (1 seconds) has passed since the last time we moved
			// the asteroids - move them - and keep track of the time we
			// moved them
			advance_asteroids();
			
			last_move_asteroid = current_time;
		}
		
		
		/* Displays the score on the seven segment display. 
		Wraps around at 100. The refresh rate is every 3 milliseconds. 
		Might need to use above method to improve performance.
		*/
		set_value(get_score());
		display_data(current_time);
	}
	// We get here if the game is over.
}

void handle_game_over() {
	kill_sound();
	uint32_t current_time = get_current_time();
	uint8_t game_over_count = 0;
	game_over_count += game_over_animation(current_time, 1);
	move_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	while(button_pushed() == NO_BUTTON_PUSHED) {
		current_time = get_current_time();
		display_data(current_time);
		if (game_over_count < 16) {
			game_over_count += game_over_animation(current_time, 1);
		} else if (game_over_count == 16) {
			game_over_count += game_over_animation(current_time, 2);
		}  else if (game_over_count < 100) {
			game_over_count += game_over_animation(current_time, 3);
		} else if (game_over_count == 100) {
			game_over_count += game_over_animation(current_time, 4);
		} else if (game_over_count < 110) {
			game_over_count += game_over_animation(current_time, 5);
		}
	}
	init_lives();
}




