#ifndef __TASK_H
#define __TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

typedef enum TaskType {
	RTOS,
	EspTimer
} TaskType_t;

typedef struct Task {
	unsigned int uiLoopTime;
	void (*TaskCallback)(void);		// 进程函数回调
	esp_timer_handle_t xTimer;		// eps_timer 句柄
} Task_t;

void Task_Create(TaskType_t Type, Task_t *NewTask, void (*TaskCallback)(void), unsigned int uiLoopTime, unsigned int uiStackDepth, UBaseType_t Priority, BaseType_t CoreID);

#endif
