#ifndef __PID_H
#define __PID_H

#include <Arduino.h>
#include "SimpleFOC.h"

extern PIDController PID_AngleZeroPoint;
extern PIDController PID_LQR_u;
extern PIDController PID_YawAngle;
extern PIDController PID_YawGyro;
extern PIDController PID_RollAngle;

#endif
