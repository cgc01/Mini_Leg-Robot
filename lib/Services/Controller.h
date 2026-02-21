#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include <Arduino.h>
#include "SimpleFOC.h"
#include "Reg.h"
#include "Servo.h"
#include "Motor.h"
#include "PID.h"
#include "LPF.h"
#include "BAT.h"
#include "LQR.h"
#include "esp_timer.h"

typedef enum JumpState {
    Jump_Idle,
    Jump_Prepare,
    Jump_Push,
    Jump_Fly,
    Jump_Landing,
    Jump_Landed
} JumpState_t;

void Controller_Init(void);
void Controller_Proc(void);

#endif
