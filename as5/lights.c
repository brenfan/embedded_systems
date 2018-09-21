// lights.c: Module to control leds
#include "lights.h"
#include "consoleUtils.h"
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"
#include <stdbool.h>
/******************************************************************************
 **              Variables
 ******************************************************************************/

#define LED_GPIO_BASE           (SOC_GPIO_1_REGS)
#define LED0_PIN (21)
#define LED1_PIN (22)
#define LED2_PIN (23)
#define LED3_PIN (24)

static volatile _Bool s_isTimeToFlash = false;
static int pattern = PATTERN_A;
static int speed = 0;
#define PATTERN_LENGTH 6
static int patterna[] = {0x1, 0x2, 0x4, 0x8, 0x4, 0x2}; // RAW BIT PATTERNS
static int patternb[] = {0x1, 0x3, 0x7, 0xf, 0x7, 0x3}; // NOT MAGIC NUMBERS

static int idx = 0;
static int period = 0;
static int counter = 0;

#define MAX_SPEED 9
/******************************************************************************
 **              Public functions
 ******************************************************************************/
void lights_init(void)
{
	/* Enabling functional clocks for GPIO1 instance. */
	GPIO1ModuleClkConfig();

	/* Selecting GPIO1[23] pin for use. */
	//GPIO1Pin23PinMuxSetup();

	/* Enabling the GPIO module. */
	GPIOModuleEnable(LED_GPIO_BASE);

	/* Resetting the GPIO module. */
	GPIOModuleReset(LED_GPIO_BASE);

	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_GPIO_BASE,
			LED0_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED1_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED2_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED3_PIN,
			GPIO_DIR_OUTPUT);
	// TURN OFF ALL THE LEDS
	HWREG(LED_GPIO_BASE + GPIO_CLEARDATAOUT) = (0xF << LED0_PIN);
}

void lights_setPattern(int p)
{
	pattern = p;
}

void lights_patternToggle(void)
{
	pattern = !pattern;
}

void lights_setSpeed(int s)
{
	speed = s;
	period = 2 << (MAX_SPEED - s); // formula as specified
	counter = 0;
}

void lights_speedDown(void)
{
	lights_setSpeed( (speed - 1) >= 0 ? speed - 1 : 0);

}

void lights_speedUp(void)
{
	lights_setSpeed( (speed + 1) <= MAX_SPEED ? speed + 1 : MAX_SPEED);
}

void lights_notifyOnTimeIsr(void)
{
	s_isTimeToFlash = true;
}

void changelights(void) {
	// TURN OFF ALL THE LEDS 
	HWREG(LED_GPIO_BASE + GPIO_CLEARDATAOUT) = (0xf << LED0_PIN);
	if (pattern == PATTERN_A) {
		HWREG(LED_GPIO_BASE + GPIO_SETDATAOUT) = patterna[idx] << LED0_PIN;
	} else {
		HWREG(LED_GPIO_BASE + GPIO_SETDATAOUT) = patternb[idx] << LED0_PIN;
	}
	idx++;
	if (idx >= PATTERN_LENGTH) {
		idx = 0;
	}
	//ConsoleUtilsPrintf("%d ", idx);

}

void lights_doBackgroundWork(void)
{
	if (s_isTimeToFlash) {
		s_isTimeToFlash = 0;
		counter++;
		if (counter > period) {
			counter = 0;
			changelights();
		}
	}
}
