#include "encoder.h"

int Read_Speed(TIM_HandleTypeDef *htim)
{
	int temp;
	temp=(short)__HAL_TIM_GetCounter(htim);		//强制转换成short是将全是正数的计数器值变成有正有负
	__HAL_TIM_SetCounter(htim,0);
	return temp;
}
