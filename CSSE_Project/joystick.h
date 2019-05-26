/*
 * joystick.h
 * 
 * Author: Matt BURTON
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#define NO_JOYSTICK_MOVEMENT (-1)

void init_joystick(void);
void step_joystick(void);
int8_t joystick_moved(void);

#endif /* JOYSTICK_H_ */