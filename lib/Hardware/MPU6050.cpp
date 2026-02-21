#include "MPU6050.h"

static MPU6050 Mpu6050(IICtwo);        //MPU6050实例

/**
 * @brief 角速度取零偏前的静止检测
 * @return 1 通过静置检测
 */
static unsigned char MPU6050_StaticDetect(void)
{
    float fLastGyro[3] = {0}, fErrorGyro[3];
    unsigned short usStableCount = 100;
    while(usStableCount--)
    {
        do
        {
            delay(10);
            MPU6050_GetData_Proc();

            for(unsigned char i = 0; i < 3; i++)
            {
                fErrorGyro[i] = (float)sReg[GyroX + i] * 1e-3f / PI * 180.0f - fLastGyro[i];
                fLastGyro[i] = (float)sReg[GyroX + i] * 1e-3f / PI * 180.0f;
            }
        } while(fabs(fErrorGyro[0]) > 5.0f || fabs(fErrorGyro[1]) > 5.0f || fabs(fErrorGyro[2]) > 5.0f);
    }

    return 1;
}

/**
 * @brief 获取 MPU6050 的加速度、角速度、角度并写入寄存器
 * 
 * @note 加速度放大 1000 倍存入寄存器
 * @note 角速度放大 1000 倍存入寄存器，弧度制 
 * @note 角度放大 1000 倍存入寄存器，弧度制
 */
void MPU6050_GetData(void)
{
    Mpu6050.update();

    sReg[AccX] = (short)(Mpu6050.getAccX() * 1000.0f);
    sReg[AccY] = (short)(Mpu6050.getAccY() * 1000.0f);
    sReg[AccZ] = (short)(Mpu6050.getAccZ() * 1000.0f);
    sReg[GyroX] = (short)(Mpu6050.getGyroX() / 180.0f * PI * 1000.0f);
    sReg[GyroY] = (short)(Mpu6050.getGyroY() / 180.0f * PI * 1000.0f);
    sReg[GyroZ] = (short)(Mpu6050.getGyroZ() / 180.0f * PI * 1000.0f);
    sReg[AngleX] = (short)(Mpu6050.getAngleX() / 180.0f * PI * 1000.0f);
    sReg[AngleY] = (short)(Mpu6050.getAngleY() / 180.0f * PI * 1000.0f);
    sReg[AngleZ] = (short)(Mpu6050.getAngleZ() / 180.0f * PI * 1000.0f);

    LPF_GyroX((float)sReg[GyroX]);
    LPF_GyroY((float)sReg[GyroY]);
    LPF_GyroZ((float)sReg[GyroZ]);
}

/**
 * @brief 初始化 MPU6050
 */
void MPU6050_Init(void)
{
    IIC_Init();
    Mpu6050.begin();
    if(MPU6050_StaticDetect()){Mpu6050.calcGyroOffsets(true);}
}

/**
 * @brief MPU6050 进程函数
 */
void MPU6050_GetData_Proc(void)
{
    MPU6050_GetData();
}
