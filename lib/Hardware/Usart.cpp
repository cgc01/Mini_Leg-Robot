#include "Usart.h"

/**
 * @brief 连续读取串口数据
 */
unsigned int Usart_ReadBytes(unsigned char *ucData)
{
    unsigned int uiRxLen = 0;
    while(Serial2.available())
    {
        ucData[uiRxLen++] = Serial2.read();
    }

    return uiRxLen;
}

/**
 * @brief 连续写入串口数据
 */
void Usart_WriteBytes(unsigned char *ucData, unsigned int uiLen)
{
    Serial2.write(ucData, uiLen);
    Serial2.flush();
}

/**
 * @brief 初始化串口
 */
void Usart_Init(void)
{
    static unsigned char ucInitFlag = 0;
    if(ucInitFlag){return;}
    ucInitFlag = 1;

    Serial2.begin(1000000);
}
