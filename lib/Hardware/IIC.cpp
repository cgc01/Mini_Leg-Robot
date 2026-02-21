#include "IIC.h"

TwoWire IICone = TwoWire(0);
TwoWire IICtwo = TwoWire(1);

/**
 * @brief IIC 写多个字节
 * 
 * @param ucIICNum I2C 总线编号
 * @param ucAddr 从机地址
 * @param ucReg 寄存器地址
 * @param ucLen 写入字节数
 * @param ucBuf 数据缓冲区指针
 */
void IIC_WriteBytes(unsigned char ucIICNum, unsigned char ucAddr, unsigned char ucReg, unsigned char ucLen, unsigned char *ucBuf)
{
    TwoWire *IIC = NULL;
    if(ucIICNum == 0){IIC = &IICone;}
    else if(ucIICNum == 1){IIC = &IICtwo;}
    else{return;}

    IIC->beginTransmission(ucAddr);
    IIC->write(ucReg);
    IIC->write(ucBuf, ucLen);
    IIC->endTransmission(true);
}

/**
 * @brief IIC 读多个字节
 * 
 * @param ucIICNum I2C 总线编号
 * @param ucAddr 从机地址
 * @param ucReg 寄存器地址
 * @param ucLen 读取字节数
 * @param ucBuf 数据缓冲区指针
 * 
 * @note 内部使用 Repeated START，requestFrom 自动产生 STOP
 */
void IIC_ReadBytes(unsigned char ucIICNum, unsigned char ucAddr, unsigned char ucReg, unsigned char ucLen, unsigned char *ucBuf)
{
    TwoWire *IIC = NULL;
    if(ucIICNum == 0){IIC = &IICone;}
    else if(ucIICNum == 1){IIC = &IICtwo;}
    else{return;}

    IIC->beginTransmission(ucAddr);
    IIC->write(ucReg);
    IIC->endTransmission(false);      // 重复起始条件

    IIC->requestFrom(ucAddr, ucLen);
    for(unsigned char i = 0; i < ucLen; i++)
    {
        ucBuf[i] = IIC->read();
    }
}

/**
 * @brief 初始化 IIC
 */
void IIC_Init()
{
    static unsigned char ucInitFlag = 0;
    if(ucInitFlag){return;}
    ucInitFlag = 1;

    IICone.begin(IICone_SDA, IICone_SCL, 400000UL);     // SDA SCL 频率
    IICtwo.begin(IICtwo_SDA, IICtwo_SCL, 400000UL);     // SDA SCL 频率
}
