/* Header for LED/Joystick Interface */

#ifndef _IO_H_
#define _IO_H_

#define OFF 0
#define ON 1

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

void init_gpio();

void init_joystick();

void init_leds();

void set_led0(int b);

void set_led3(int b);

void flash_led();

int getJoystickInput();

int anyJoystickIsPressed();

void close_leds();

#endif
