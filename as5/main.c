/*main loop*/

#include "consoleUtils.h"
#include "interrupt.h"
#include <stdint.h>

// My hardware abstraction modules
#include "serial.h"
#include "timer.h"
#include "wdtimer.h"

// My application's modules
#include "lights.h"
#include "joystick.h"

#define RESET_SOURCE_REG 0x44E00F00
#define PRM_RSTST 8

#define ICEPICK_RST 9
#define EXTERNAL_WARM_RST 5
#define WDT1_RST 4
#define GLOBAL_WARM_SW_RST 1
#define GLOBAL_COLD_RST 0


#define HELP_TEXT \
		"\nAccepted Serial Commands\n"\
		"  ?     : Display this help message.\n"\
		"  0-9   : Set speed 0 (slow) to 9 (fast).\n"\
		"  a     : Select pattern A (bounce)\n"\
		"  b     : Select pattern B (bar)\n"\
		"  x     : Stop abusing the watchdog\n"

int feedwatchdog_f = 1;

/******************************************************************************
 **              SERIAL PORT HANDLING
 ******************************************************************************/
static volatile uint8_t s_rxByte = 0;
static void serialRxIsrCallback(uint8_t rxByte) {
	s_rxByte = rxByte;
}

static void doBackgroundSerialWork(void)
{
	if (s_rxByte == 0) {
		return;
	}
	else if (s_rxByte == '?') {
		ConsoleUtilsPrintf(HELP_TEXT);
	}
	else if (s_rxByte == 'a') {
		lights_setPattern(PATTERN_A);
		ConsoleUtilsPrintf("\nPattern changed to A\n");
	}
	else if (s_rxByte == 'b') {
		lights_setPattern(PATTERN_B);
		ConsoleUtilsPrintf("\nPattern changed to B\n");

	}
	else if (s_rxByte == 'x') {
		feedwatchdog_f = 0;
		ConsoleUtilsPrintf("\nWatchdog will now starve\n");
	}
	else if (s_rxByte >= '0' && s_rxByte <='9') {
		lights_setSpeed(s_rxByte - '0');
		ConsoleUtilsPrintf("\nSpeed set to %d\n", s_rxByte - '0');
	}
	else {

		/* error case */
		ConsoleUtilsPrintf("\nError, command not found\n");
		ConsoleUtilsPrintf(HELP_TEXT);
	}
	s_rxByte = 0;



}

/******************************************************************************
 **              OTHER
 ******************************************************************************/

static void checkResetRegister(void) {
	int reg = *(volatile unsigned long *) (RESET_SOURCE_REG + PRM_RSTST);
	ConsoleUtilsPrintf("\nReset sources are ");
	/* Check each bit */
	if (reg & (1 << ICEPICK_RST))
		ConsoleUtilsPrintf("Icepick Reset, ");
	if (reg & (1 << EXTERNAL_WARM_RST))
		ConsoleUtilsPrintf("External Warm Reset, ");
	if (reg & (1 << WDT1_RST))
		ConsoleUtilsPrintf("Watchdog1 Reset, ");
	if (reg & (1 << GLOBAL_WARM_SW_RST))
		ConsoleUtilsPrintf("Global warm software Reset, ");
	if (reg & (1 << GLOBAL_COLD_RST))
		ConsoleUtilsPrintf("Global cold Reset");

	ConsoleUtilsPrintf("\n");

	/* clear by writing a 1 */
	*(volatile unsigned long *) (RESET_SOURCE_REG + PRM_RSTST) = 0x233;

}



/******************************************************************************
 **              MAIN
 ******************************************************************************/
int main(void)
{
	// Initialization
	IntAINTCInit();
	Serial_init(serialRxIsrCallback);
	Timer_init();
	Watchdog_init();
	//joystick_initializeButtonPin();

	checkResetRegister();
	// Setup callbacks from hardware abstraction modules to application:
	Serial_setRxIsrCallback(serialRxIsrCallback);
	Timer_setTimerIsrCallback(lights_notifyOnTimeIsr);

	joystick_setUpAction(lights_speedUp);
	joystick_setDownAction(lights_speedDown);
	joystick_setLeftAction(lights_patternToggle);
	lights_setSpeed(7);

	// Display startup messages to console:
	ConsoleUtilsPrintf("\nWelcome to Brendan's Baremetal App\n");

	// Main loop:
	while(1) {
		// Handle background processing
		doBackgroundSerialWork();
		joystick_backgroundWork();
		lights_doBackgroundWork();

		// Timer ISR signals intermittent background activity.
		if(Timer_isIsrFlagSet()) {
			Timer_clearIsrFlag();
			if (feedwatchdog_f)
				Watchdog_hit();
		}
	}
}
