#include "LED.h"

/**
 * @brief 打开 LED
 */
void LED_On(void)
{
    digitalWrite(OnBroadLed, HIGH);
}

/**
 * @brief 关闭 LED
 */
void LED_Off(void)
{
    digitalWrite(OnBroadLed, LOW);
}

/**
 * @brief 切换 LED 状态（高/低电平）
 */
void LED_Toggle(void)
{
    digitalWrite(OnBroadLed, !digitalRead(OnBroadLed));
}

/**
 * @brief 初始化 LED 引脚为输出模式
 */
void LED_Init(void)
{
    pinMode(OnBroadLed, OUTPUT);
}

/**
 * @brief LED 进程函数
 */
void LED_Proc(void)
{
    if((float)sReg[BAT] * 1e-3f < 7.8f)        // 没电了
    {
        LED_Off();
        return;
    }
    LED_Toggle();
}
