#ifndef __USART_H
#define __USART_H

#include <Arduino.h>

unsigned int Usart_ReadBytes(unsigned char *ucData);
void Usart_WriteBytes(unsigned char *ucData, unsigned int uiLen);
void Usart_Init(void);

#endif
