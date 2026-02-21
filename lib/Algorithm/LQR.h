#ifndef __LQR_H
#define __LQR_H

#include <Arduino.h>

typedef struct LQR_CarModel {
    float fMechAngleZeroOffset;      // 机械零点
    float fMechWheelPosZeroOffset;      // 轮部位移零点偏置
    float fR;       // 车轮半径
    float fBaseHeight;      // 基础质心高度
    float fThighLength;     // 大腿长度
    float fShankLength;     // 小腿长度
    float fMaxSpeed;        // 最高移动速度
} LQR_CarModel_t;

extern LQR_CarModel_t LQR_Car;
extern float fLQR_K[4];

void LQR_UpdateDynamicHeight(float fHeight);

#endif
