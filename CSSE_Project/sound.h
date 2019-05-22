/*
 * sound.h
 *
 * Author: Matt Burton
 */ 


#ifndef SOUND_H_
#define SOUND_H_

// Setup Sound timer.
void init_sound(void);

// Control Sounds.
void set_sound(uint16_t freq, float dutycycle);
void random_sound();
void kill_sound();

#endif /* SOUND_H_ */
