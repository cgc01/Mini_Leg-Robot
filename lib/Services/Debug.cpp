#include "Debug.h"

/**
 * @brief 打包调试数据
 */
unsigned char *Debug_PackDebugData(unsigned int *uiLen)
{
    static unsigned char ucDebugData[512];
    unsigned char ucIndex = 0;
    
    float fTemp[3];

    ucDebugData[ucIndex++] = 0xFF;
    ucDebugData[ucIndex++] = 0xAA;

    ucIndex++;      // 包长，先占位

    // 角速度 X
    for(unsigned char i = 0; i < 3; i++){fTemp[i] = (float)sReg[GyroX + i];}
    ucDebugData[ucIndex++] = 0x01;
    memcpy(&ucDebugData[ucIndex], &fTemp[0], sizeof(float));
    ucIndex += sizeof(float);

    // 角速度 Y
    ucDebugData[ucIndex++] = 0x02;
    memcpy(&ucDebugData[ucIndex], &fTemp[1], sizeof(float));
    ucIndex += sizeof(float);
    
    // 角速度 Z
    ucDebugData[ucIndex++] = 0x03;
    memcpy(&ucDebugData[ucIndex], &fTemp[2], sizeof(float));
    ucIndex += sizeof(float);

    // 加速度 X
    for(unsigned char i = 0; i < 3; i++){fTemp[i] = (float)sReg[AccX + i];}
    ucDebugData[ucIndex++] = 0x04;
    memcpy(&ucDebugData[ucIndex], &fTemp[0], sizeof(float));
    ucIndex += sizeof(float);

    // 加速度 Y
    ucDebugData[ucIndex++] = 0x05;
    memcpy(&ucDebugData[ucIndex], &fTemp[1], sizeof(float));
    ucIndex += sizeof(float);

    // 加速度 Z
    ucDebugData[ucIndex++] = 0x06;
    memcpy(&ucDebugData[ucIndex], &fTemp[2], sizeof(float));
    ucIndex += sizeof(float);

    unsigned char ucSum = 0;
    for(unsigned char i = 0; i < ucIndex - 2; i++)
    {
        ucSum += ucDebugData[i + 2];
    }
    ucDebugData[ucIndex++] = ucSum;

    ucDebugData[2] = ucIndex;      // 整包长度
    *uiLen = ucIndex;

    return ucDebugData;
}
