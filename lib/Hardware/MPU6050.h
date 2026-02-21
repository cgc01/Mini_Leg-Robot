#ifndef __MPU6050_H
#define __MPU6050_H

#include <Arduino.h>
#include <MPU6050_tockn.h>
#include "Reg.h"
#include "IIC.h"
#include "LPF.h"

void MPU6050_Init(void);
void MPU6050_GetData_Proc(void);

#endif