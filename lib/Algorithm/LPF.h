#ifndef __LPF_H
#define __LPF_H

#include <Arduino.h>
#include "SimpleFOC.h"

extern LowPassFilter LPF_GyroX;
extern LowPassFilter LPF_GyroY;
extern LowPassFilter LPF_GyroZ;
extern LowPassFilter LPF_Roll;
extern LowPassFilter LPF_Joyy;

#endif
