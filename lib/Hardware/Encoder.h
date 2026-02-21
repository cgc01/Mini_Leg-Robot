#ifndef __ENCODER_H
#define __ENCODER_H

#include <Arduino.h>
#include <SimpleFOC.h>
#include "IIC.h"

extern MagneticSensorI2C Encoder_Left;
extern MagneticSensorI2C Encoder_Right;

void Encoder_Init(void);

#endif
