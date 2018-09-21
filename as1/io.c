/* Controls LEDs and Joystick IO */
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"

#define EXPORTS "/sys/class/gpio/export"

#define GPIO_JOY_UP 26
#define GPIO_JOY_DOWN 46
#define GPIO_JOY_LEFT 65
#define GPIO_JOY_RIGHT 47

#define JOY_UP_DIRECTION "/sys/class/gpio/gpio26/direction"
#define JOY_DOWN_DIRECTION "/sys/class/gpio/gpio46/direction"
#define JOY_LEFT_DIRECTION "/sys/class/gpio/gpio65/direction"
#define JOY_RIGHT_DIRECTION "/sys/class/gpio/gpio47/direction"

#define JOY_UP_VALUE "/sys/class/gpio/gpio26/value"
#define JOY_DOWN_VALUE "/sys/class/gpio/gpio46/value"
#define JOY_LEFT_VALUE "/sys/class/gpio/gpio65/value"
#define JOY_RIGHT_VALUE "/sys/class/gpio/gpio47/value"

#define LED0_BRIGHTNESS "/sys/class/leds/beaglebone:green:usr0/brightness"
#define LED1_BRIGHTNESS "/sys/class/leds/beaglebone:green:usr1/brightness"
#define LED2_BRIGHTNESS "/sys/class/leds/beaglebone:green:usr2/brightness"
#define LED3_BRIGHTNESS "/sys/class/leds/beaglebone:green:usr3/brightness"

#define LED0_TRIGGER "/sys/class/leds/beaglebone:green:usr0/trigger"
#define LED1_TRIGGER "/sys/class/leds/beaglebone:green:usr1/trigger"
#define LED2_TRIGGER "/sys/class/leds/beaglebone:green:usr2/trigger"
#define LED3_TRIGGER "/sys/class/leds/beaglebone:green:usr3/trigger"

#define LED_FLASH_DELAY 100000 /* 0.1 seconds */
#define POLL_SPEED_DELAY 100000 /* 0.1 seconds */

static FILE *led0_brightness;
static FILE *led1_brightness;
static FILE *led2_brightness;
static FILE *led3_brightness;

static void write_gpio(char *filename, char *payload) {
	FILE *file = fopen(filename, "w");
	if (file == NULL) {
		printf("ERROR: Unable to open `%s` for read\n", filename);
		exit(-1);
	}
	fprintf(file, "%s", payload);
	fclose(file);
}

static int read_gpio(char *filename) {
	const int max_length = 16;
	char buff[max_length];

	FILE *file = fopen(filename, "r");
	if (!file) {
		printf("Error reading file `%s`\n", filename);
		exit(1);
	}
	fgets(buff, max_length, file);
	fclose(file);

	return atoi(buff);
}

void init_gpio() {
	FILE *export = fopen(EXPORTS, "w");
	if (export == NULL) {
		printf("ERROR: Unable to open export file.\n");
		exit(-1);
	}

	fprintf(export, "%d", GPIO_JOY_UP);
	fprintf(export, "%d", GPIO_JOY_DOWN);
	fprintf(export, "%d", GPIO_JOY_LEFT);
	fprintf(export, "%d", GPIO_JOY_RIGHT);

	fclose(export);
}

void init_joystick() {
	write_gpio(JOY_UP_DIRECTION, "in");
	write_gpio(JOY_DOWN_DIRECTION, "in");
	write_gpio(JOY_LEFT_DIRECTION, "in");
	write_gpio(JOY_RIGHT_DIRECTION, "in");

}

void init_leds() {
	/* Disable triggers */
	write_gpio(LED0_TRIGGER, "none");
	write_gpio(LED1_TRIGGER, "none");
	write_gpio(LED2_TRIGGER, "none");
	write_gpio(LED3_TRIGGER, "none");

	/* All your LEDs are belong to us */
	led0_brightness = fopen(LED0_BRIGHTNESS, "w");
	if (led0_brightness == NULL) {
		printf("ERROR: Unable to open `%s` for write\n",
		LED0_BRIGHTNESS);
		exit(-1);
	}
	led1_brightness = fopen(LED1_BRIGHTNESS, "w");
	if (led1_brightness == NULL) {
		printf("ERROR: Unable to open `%s` for write\n",
		LED1_BRIGHTNESS);
		exit(-1);
	}
	led2_brightness = fopen(LED2_BRIGHTNESS, "w");
	if (led2_brightness == NULL) {
		printf("ERROR: Unable to open `%s` for write\n",
		LED2_BRIGHTNESS);
		exit(-1);
	}
	led3_brightness = fopen(LED3_BRIGHTNESS, "w");
	if (led3_brightness == NULL) {
		printf("ERROR: Unable to open `%s` for write\n",
			LED3_BRIGHTNESS);
		exit(-1);
	}
}

void set_led0(int b) {
	fprintf(led0_brightness, "%d", b);
	fflush(NULL);
}

void set_led3(int b) {
	fprintf(led3_brightness, "%d", b);
	fflush(NULL);
}

void flash_led() {
	fprintf(led0_brightness, "%d", ON);
	fprintf(led1_brightness, "%d", ON);
	fprintf(led2_brightness, "%d", ON);
	fprintf(led3_brightness, "%d", ON);
	fflush(NULL);
	usleep(LED_FLASH_DELAY);
	fprintf(led0_brightness, "%d", OFF);
	fprintf(led1_brightness, "%d", OFF);
	fprintf(led2_brightness, "%d", OFF);
	fprintf(led3_brightness, "%d", OFF);
	fflush(NULL);
	usleep(LED_FLASH_DELAY);
}

int getJoystickInput() {
	do {
		if (read_gpio(JOY_UP_VALUE) == 0)
			return UP;
		if (read_gpio(JOY_DOWN_VALUE) == 0)
			return DOWN;
		if (read_gpio(JOY_LEFT_VALUE) == 0)
			return LEFT;
		if (read_gpio(JOY_RIGHT_VALUE) == 0)
			return RIGHT;
		usleep(POLL_SPEED_DELAY);
	} while (1);
}

int anyJoystickIsPressed(){
	if (read_gpio(JOY_UP_VALUE) == 0)
		return 1;
	if (read_gpio(JOY_DOWN_VALUE) == 0)
		return 1;
	if (read_gpio(JOY_LEFT_VALUE) == 0)
		return 1;
	if (read_gpio(JOY_RIGHT_VALUE) == 0)
		return 1;
	usleep(POLL_SPEED_DELAY);
	return 0;
}

void close_leds() {
	fclose(led0_brightness);
	fclose(led1_brightness);
	fclose(led2_brightness);
	fclose(led3_brightness);
}
