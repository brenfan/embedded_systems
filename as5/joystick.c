// GPIO Button demo
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"      // For HWREG(...) macro
#include "watchdog.h"
#include "uart_irda_cir.h"
#include "consoleUtils.h"
#include <stdint.h>



/*****************************************************************************
**                INTERNAL MACRO DEFINITIONS
*****************************************************************************/
// Boot btn on BBB: SOC_GPIO_2_REGS, pin 8
// Down on Zen cape: SOC_GPIO_1_REGS, pin 14  NOTE: Must change other "2" constants to "1" for correct initialization.
// Left on Zen cape: SOC_GPIO_2_REGS, pin 1
// --> This code uses left on the ZEN:
#define JOYSTICK_GPIO_BASE2     (SOC_GPIO_2_REGS)
#define JOYSTICK_GPIO_BASE1     (SOC_GPIO_1_REGS)
#define JOYSTICK_GPIO_BASE0     (SOC_GPIO_0_REGS)
#define LEFT_PIN           (1)  //gpio 2
#define UP_PIN             (26) //gpio 0
#define DOWN_PIN	   (14) //gpio 1

int last_up = 0;
int last_down = 0;
int last_left = 0;

static void (*left_action)(void) = 0;
static void (*up_action)(void) = 0;
static void (*down_action)(void) = 0;


/*****************************************************************************
**                INTERNAL FUNCTION DEFINITIONS
*****************************************************************************/
#include "hw_cm_per.h"
void GPIO2ModuleClkConfig(void)
{

    /* Writing to MODULEMODE field of CM_PER_GPIO1_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_MODULEMODE));
    /*
    ** Writing to OPTFCLKEN_GPIO_2_GDBCLK bit in CM_PER_GPIO2_CLKCTRL
    ** register.
    */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK;

    /*
    ** Waiting for OPTFCLKEN_GPIO_1_GDBCLK bit to reflect the desired
    ** value.
    */
    while(CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK));

    /*
    ** Waiting for IDLEST field in CM_PER_GPIO2_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_GPIO2_CLKCTRL_IDLEST_FUNC <<
           CM_PER_GPIO2_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
            CM_PER_GPIO2_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_GPIO_2_GDBCLK bit in CM_PER_L4LS_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK));
}


void joystick_initializeButtonPin(void)
{
    /* Enabling functional clocks for GPIO2 instance. */
	//GPIO0ModuleClkConfig();
	GPIO1ModuleClkConfig();
	GPIO2ModuleClkConfig();


    /* Selecting GPIO1[23] pin for use. */
    GPIO1Pin23PinMuxSetup();

    /* Enabling the GPIO module. */
    GPIOModuleEnable(JOYSTICK_GPIO_BASE0);
    GPIOModuleEnable(JOYSTICK_GPIO_BASE1);
    GPIOModuleEnable(JOYSTICK_GPIO_BASE2);

    /* Resetting the GPIO module. */
    GPIOModuleReset(JOYSTICK_GPIO_BASE0);
    GPIOModuleReset(JOYSTICK_GPIO_BASE1);
    GPIOModuleReset(JOYSTICK_GPIO_BASE2);

    /* Setting the GPIO pin as an input pin. */
    GPIODirModeSet(JOYSTICK_GPIO_BASE2, LEFT_PIN, GPIO_DIR_INPUT);
    GPIODirModeSet(JOYSTICK_GPIO_BASE0, UP_PIN, GPIO_DIR_INPUT);
    GPIODirModeSet(JOYSTICK_GPIO_BASE1, DOWN_PIN, GPIO_DIR_INPUT);
}

void joystick_setUpAction(void (*callback)(void)) {
	up_action = callback;
}
void joystick_setDownAction(void (*callback)(void)) {
	down_action = callback;
}
void joystick_setLeftAction(void (*callback)(void)) {
	left_action = callback;
}

static _Bool readButtonUp(void)
{
	uint32_t regValue = HWREG(JOYSTICK_GPIO_BASE0 + GPIO_DATAIN);
	uint32_t mask     = (1 << UP_PIN);

	return (regValue & mask) == 0;
}

static _Bool readButtonLeft(void)
{
	uint32_t regValue = HWREG(JOYSTICK_GPIO_BASE2 + GPIO_DATAIN);
	uint32_t mask     = (1 << LEFT_PIN);

	return (regValue & mask) == 0;
}

static _Bool readButtonDown(void)
{
	uint32_t regValue = HWREG(JOYSTICK_GPIO_BASE1 + GPIO_DATAIN);
	uint32_t mask     = (1 << DOWN_PIN);

	return (regValue & mask) == 0;
}

void joystick_backgroundWork(void)
{
	int up = readButtonUp();
	int down = readButtonDown();
	int left = readButtonLeft();

	if (!up && last_up) {
		up_action();
	}
	if (!down && last_down) {
		down_action();
	}
	if (!left && last_left) {
		left_action();
	}
	last_up = up;
	last_down = down;
	last_left = left;
	//ConsoleUtilsPrintf("UP: %d DOWN: %d LEFT %d\n", up, down, left);


}
