#include "test_all.h"
#include "audio.h"
#include "tail.h"
#include "gesture.h"
#include "display.h"
#include "blinking.h"
#include "following.h"
#include "movement.h"
#include "OLED.h"
#include "paj7620.h"
#include "string.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

TaskHandle_t testall_task_handle_t;

static void TestAllTaskInitialize(void)
{
    //Initialize if needed, however cubemx has done most of the things for us.
    //And I that every object initialized once it was created.
}


void TestAllTask(void)
{
    taskENTER_CRITICAL();
    TestAllTaskInitialize();
    DisplayTaskStart();
    BlinkingTaskStart();
    GestureTaskStart();
    FollowingTaskStart();
    AudioTaskStart();
    MoveControlTaskStart();
    TailTaskStart(); 

    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
}

void TestAllTaskStart(void)
{
    /* 1.创建一�?�?动任�? */
    xTaskCreate((TaskFunction_t)TestAllTask,               // 任务函数的地址
                (char *)"main_task",                     // 任务名字符串
                (configSTACK_DEPTH_TYPE)TEST_TASK_STACK, // 任务栈大�?
                (void *)NULL,                            // 传递给任务的参�?
                (UBaseType_t)TEST_TASK_PRIORITY,         // 任务的优先级
                (TaskHandle_t *)&testall_task_handle_t);      // 任务句柄的地址
}

