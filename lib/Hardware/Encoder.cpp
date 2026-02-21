#include "Encoder.h"

MagneticSensorI2C Encoder_Left = MagneticSensorI2C(AS5600_I2C);
MagneticSensorI2C Encoder_Right = MagneticSensorI2C(AS5600_I2C);

/**
 * @brief 初始化编码器
 */
void Encoder_Init(void)
{
    IIC_Init();
    Encoder_Left.init(&IICone);
    Encoder_Right.init(&IICtwo);
}
