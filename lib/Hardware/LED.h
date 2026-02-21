#ifndef __LED_H
#define __LED_H

#include <Arduino.h>
#include "Reg.h"
#include "BAT.h"

#define OnBroadLed     13

void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);
void LED_Proc(void);

#endif
