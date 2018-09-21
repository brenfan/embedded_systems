/* interface for joystick control */

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

void joystick_initializeButtonPin(void);
void joystick_backgroundWork(void);

void joystick_setUpAction(void (*callback)(void));
void joystick_setDownAction(void (*callback)(void));
void joystick_setLeftAction(void (*callback)(void));

#endif
