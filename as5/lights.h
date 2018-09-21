/* controls the leds */

#ifndef _LIGHTS_H_
#define _LIGHTS_H_


#define PATTERN_A 0
#define PATTERN_B 1

void lights_setPattern(int p);
void lights_init(void);
void lights_doBackgroundWork(void);
void lights_notifyOnTimeIsr(void);
void lights_setSpeed(int s);
void lights_speedDown(void);
void lights_speedUp(void);
void lights_patternToggle(void);
#endif
