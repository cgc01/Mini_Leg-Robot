#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f1xx_hal.h"
void Key_Loop(void);
unsigned char Key(void);

extern unsigned char Task;
#endif


#define KEY1_GPIO_Port GPIOA 23


#define KEY2_GPIO_Port GPIOA 24


#test1
#test2
#test3  