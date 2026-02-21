#ifndef __MOTOR_H
#define __MOTOR_H

#include <Arduino.h>
#include "SimpleFOC.h"
#include "Encoder.h"
#include "Reg.h"

extern BLDCMotor Motor_Left;
extern BLDCMotor Motor_Right;

void Motor_Init(void);
void Motor_Update_Proc(void);

#endif
