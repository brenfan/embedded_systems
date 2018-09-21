/* Main file */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "io.h"

int main () {
	int score;
	int total;
	int answer;
	int input;

	score = 0;
	total = 0;

	/* Init routine */
	srand(time(NULL));
	init_gpio();
	init_joystick();
	init_leds();

	fflush(NULL);

	printf("Hello embedded world, from Brendan!\n\n");

	printf(
		"Press the Zen cape's Joystick in the direction of the LED.\n"
		"  UP for LED 0 (top)\n"
		"  DOWN for LED 3 (bottom)\n"
		"  LEFT/RIGHT for exit app.\n"
	);
	do {
		printf("Press joystick; current score (%d / %d)\n", score, total);

		answer = rand() & 1;
		if (answer == UP) {
			set_led0(ON);
		} else {
			set_led3(ON);
		}

		input = getJoystickInput();
		if (input == LEFT || input == RIGHT) {
			/* Exit condition */
			printf(
				"Your final score was (%d / %d)\n"
				"Thank you for playing!\n", score, total
			);
			break;
		} else if (input == answer) {
			score++;
			printf("Correct!\n");
			flash_led();
		} else {
			printf("Incorrect!\n");
			flash_led();
			flash_led();
			flash_led();
			flash_led();
			flash_led();
		}
		total ++;
		/* wait until joystick is released */
		while(anyJoystickIsPressed());
	} while(1);

	/* Exit routine */
	set_led0(OFF);
	set_led3(OFF);
	close_leds();

	return 0;
}
