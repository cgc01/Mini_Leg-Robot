#include "LQR.h"

LQR_CarModel_t LQR_Car = {
    .fMechAngleZeroOffset = 1.6f / 180.0f * PI,
    .fMechWheelPosZeroOffset = 0.0f,
    .fR = 0.17f / 2.0f / PI,        // 车轮半径
    .fBaseHeight = 0.03f,        // 关闭舵机时的基础重心高度
    .fThighLength = 0.052f,     // 大腿长度
    .fShankLength = 0.05f,      // 小腿长度
    .fMaxSpeed = 0.4f       // 限制最高移动速度，m/s
};

float fLQR_K[4];     // 角度、角速度、线位移、线速度

/**
 * @brief K 矩阵自适应机器人高度
 * @param fHeight: 机器人高度
 * @note 高度单位为 m
 */
void LQR_UpdateDynamicHeight(float fHeight)
{
    static float fLQR_GainPoly[12] = {
         2.43236033f, -5.40280026f, -0.99544562f,
        -0.79657587f, -1.34885550f, -0.05326831f,
         0.00000000f, -0.00000000f, -1.26000000f,
         1.60334555f, -1.32134414f, -0.50747789f
    };

    fLQR_K[0] = fLQR_GainPoly[0] * fHeight * fHeight + fLQR_GainPoly[1] * fHeight + fLQR_GainPoly[2];
    fLQR_K[1] = fLQR_GainPoly[3] * fHeight * fHeight + fLQR_GainPoly[4] * fHeight + fLQR_GainPoly[5];
    fLQR_K[2] = fLQR_GainPoly[6] * fHeight * fHeight + fLQR_GainPoly[7] * fHeight + fLQR_GainPoly[8];
    fLQR_K[3] = fLQR_GainPoly[9] * fHeight * fHeight + fLQR_GainPoly[10] * fHeight + fLQR_GainPoly[11];
}
