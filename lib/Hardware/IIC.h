#ifndef __IIC_H
#define __IIC_H

#include <Arduino.h>
#include <Wire.h>

extern TwoWire IICone;
extern TwoWire IICtwo;

#define IICone_SCL      18
#define IICone_SDA      19
#define IICtwo_SCL      5
#define IICtwo_SDA      23

void IIC_WriteBytes(unsigned char ucIICNum, unsigned char ucAddr, unsigned char ucReg, unsigned char ucLen, unsigned char *ucBuf);
void IIC_ReadBytes(unsigned char ucIICNum, unsigned char ucAddr, unsigned char ucReg, unsigned char ucLen, unsigned char *ucBuf);
void IIC_Init();

#endif
