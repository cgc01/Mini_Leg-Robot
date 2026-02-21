#include "Task.h"

/**
 * @brief RTOS 任务回调转发
 */
static void Task_RtosEntry(void *arg)
{
    Task_t *t = (Task_t *)arg;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        if(t->TaskCallback)		// 非空指针保护
        {
            t->TaskCallback();
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(t->uiLoopTime));
    }
}

/**
 * @brief esp_timer 回调转发
 * 
 * @note 必须是短时回调
 */
static void Task_TimerCallback(void *arg)
{
    Task_t *t = (Task_t *)arg;

    if(t->TaskCallback)		// 非空指针保护
	{
		t->TaskCallback();
    }
}

/**
 * @brief 创建一个周期任务
 *
 * @param Type          使用 RTOS 或者 EspTimer 回调
 * @param NewTask       任务对象指针
 * @param TaskCallback  周期执行的任务回调函数
 * @param uiLoopTime    任务周期，RTOS 下单位为毫秒，EspTimer 下单位为微秒
 * @param uiStackDepth  任务栈大小（单位：byte，ESP-IDF 语义）
 * @param Priority      FreeRTOS 任务优先级
 * @param CoreID        CPU 核心 ID
 * 
 * @note 若 Type 为 EspTimer，则 uiStackDepth、Priority 和 CoreID 参数无效
 */
void Task_Create(TaskType_t Type, Task_t *NewTask, void (*TaskCallback)(void), unsigned int uiLoopTime, unsigned int uiStackDepth, UBaseType_t Priority, BaseType_t CoreID)
{
    NewTask->uiLoopTime = uiLoopTime;
    NewTask->TaskCallback = TaskCallback;

    switch(Type)
    {
        case RTOS:
            xTaskCreatePinnedToCore(
                Task_RtosEntry,     // 任务入口
                "Task",      // 任务名
                uiStackDepth,       // 栈大小（byte 单位）
                NewTask,        // 参数
                Priority,       // 优先级
                NULL,
                CoreID
            );
            break;

        case EspTimer:
            const esp_timer_create_args_t TimerArgs = {
                .callback = Task_TimerCallback,
                .arg = NewTask,
                .dispatch_method = ESP_TIMER_TASK,
                .name = "UserTask",
                .skip_unhandled_events = true
            };

            esp_timer_create(&TimerArgs, &NewTask->xTimer);

            // 启动周期定时器（单位：us）
            esp_timer_start_periodic(NewTask->xTimer, uiLoopTime);
            break;
    }
}
