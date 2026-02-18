#include "Key.h"
#include "main.h"

unsigned char Key_KeyNumber;
unsigned char Task;     	//不会循环一次就变成0
unsigned char Temp = 0;

/**
  * @brief  获取按键键码
  * @param  无
  * @retval 按下按键的键码，范围：0,1~4,0表示无按键按下
  * 当调用这个函数时就可以知道是哪个按键按下，通过判断接收到的数字不同来执行相应的代码
  */
unsigned char Key(void)  //要在main中运行了Key函数才会执行此代码，Key_KeyNumber才会变成0
{
	Temp=0;
	Temp=Key_KeyNumber;
	Key_KeyNumber=0;
	if(Temp == 4)
		Task = 1;		//题目1
	if(Temp == 2)
		Task = 2;		//题目2
	if(Temp == 3)
		Task = 3;		//题目3


	return Temp;
}

/**
  * @brief  获取当前按键的状态，无消抖及松手检测
  * @param  无
  * @retval 按下按键的键码，范围：0,1~6,0表示无按键按下
  */
unsigned char Key_GetState()
{
	unsigned char KeyNumber=0;
	
	if(HAL_GPIO_ReadPin(KEY_Pin1_GPIO_Port, KEY_Pin1_Pin) == GPIO_PIN_RESET){KeyNumber=1;}
	if(HAL_GPIO_ReadPin(KEY_Pin2_GPIO_Port, KEY_Pin2_Pin) == GPIO_PIN_RESET){KeyNumber=2;}
	if(HAL_GPIO_ReadPin(KEY_Pin3_GPIO_Port, KEY_Pin3_Pin) == GPIO_PIN_RESET){KeyNumber=3;}
	if(HAL_GPIO_ReadPin(KEY_Pin4_GPIO_Port, KEY_Pin4_Pin) == GPIO_PIN_RESET){KeyNumber=4;}
	
	return KeyNumber;
}

/**
  * @brief  按键驱动函数，在中断中调用
  * @param  无
  * @retval 无
  */
void Key_Loop(void)
{
	static unsigned char NowState,LastState;
	LastState=NowState;				//按键状态更新
	NowState=Key_GetState();		//获取当前按键状态
	//如果上个时间点按键按下，这个时间点未按下，则是松手瞬间，以此避免消抖和松手检测
	if(LastState==1 && NowState==0)
	{
		Key_KeyNumber=1;
	}
	if(LastState==2 && NowState==0)
	{
		Key_KeyNumber=2;
	}
	if(LastState==3 && NowState==0)
	{
		Key_KeyNumber=3;
	}
	if(LastState==4 && NowState==0)
	{
		Key_KeyNumber=4;
	}
}
