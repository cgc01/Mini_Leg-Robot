#include "Start.h"

// #define DebugTest
#ifdef DebugTest
static void Debug_Proc(void)
{
	// // 加速度归一化
	// float fAcc[3];
	// for(unsigned char i = 0; i < 3; i++){fAcc[i] = (float)sReg[AccX + i] / 32768.0f * 4.0f;}
	// float fAccNorm = sqrtf(fAcc[0] * fAcc[0] + fAcc[1] * fAcc[1] + fAcc[2] * fAcc[2]);
	
	// static unsigned int uiAccNormAddCount = 0;
	// static double dAccNormAdd = 0.0;
	// dAccNormAdd += fAccNorm;
	// uiAccNormAddCount++;

	// static unsigned int uiAccNormPrintfCount = 0;
	// if(++uiAccNormPrintfCount >= 10 && (uiAccNormPrintfCount = 0, 1))
	// {
	// 	// printf("%.2f || %.4f\n", (float)sReg[AccZ], dAccNormAdd / (double)uiAccNormAddCount);
	// 	// dAccNormAdd = 0.0;
	// 	// uiAccNormAddCount = 0;

	// 	printf("%.2f,%.2f,%.2f\n", (float)sReg[AngleX] * 1.0e-3 / PI * 180.0f, (float)sReg[AngleY] * 1.0e-3 / PI * 180.0f, (float)sReg[AngleZ] * 1.0e-3 / PI * 180.0f);
	// }

	UdpClient_Process_Proc();
}
#endif

/**
 * @brief 任务列表
 */
static void Start_TaskList(void)
{
#ifdef DebugTest
	static Task_t Debug_Task;
	Task_Create(RTOS, &Debug_Task, Debug_Proc, 100, 2048, 2, 0);
#endif

	static Task_t LED_Task;
	Task_Create(RTOS, &LED_Task, LED_Proc, 1000, 2048, 2, 0);

	static Task_t MPU6050_Task;
	Task_Create(EspTimer, &MPU6050_Task, MPU6050_GetData_Proc, 5000, 4096, 6, 0);

	static Task_t BAT_Task;
	Task_Create(RTOS, &BAT_Task, BAT_GetVoltage_Proc, 200, 2048, 2, 0);

	static Task_t UdpClient_Task;
	Task_Create(RTOS, &UdpClient_Task, UdpClient_Process_Proc, 1, 4096, 3, 0);

	static Task_t Motor_Task;
	Task_Create(EspTimer, &Motor_Task, Motor_Update_Proc, 2000, 4096, 6, 0);		// I2C 太慢了，不要超过 500Hz

	static Task_t Controller_Task;
	Task_Create(RTOS, &Controller_Task, Controller_Proc, 2, 4096, 5, 1);
}

/**
 * @brief 初始化所有模块
 */
void Start_SetUpAll(void)
{
	Reg_Init();
	UdpClient_Init();
	BAT_Init();
	LED_Init();
	MPU6050_Init();
	Controller_Init();

	Start_TaskList();
}
