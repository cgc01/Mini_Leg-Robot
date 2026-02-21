#ifndef __BATTERY_H
#define __BATTERY_H

#include <Arduino.h>
#include <esp_adc_cal.h>
#include "Reg.h"

#define BAT_Pin         35      //电池电压检测引脚

void BAT_GetVoltage(void);
void BAT_Init(void);
void BAT_GetVoltage_Proc(void);

#endif
