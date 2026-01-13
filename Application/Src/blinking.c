/**************************************************************/
/***************** 以下使用rtos，进行多任务调试 *****************/
/**************** 操作：不用接线，观察板载LED闪烁 ***************/
/******************** 现象：LED每秒闪烁一次 ********************/
/**************************************************************/

#include "blinking.h"
#include "bsp_led.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

static void BlinkingTask(void* pvParameters)
{
    while (1)
    {
        bsp_led_toggle();
        vTaskDelay(pdMS_TO_TICKS(500));
        bsp_led_toggle();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void BlinkingTaskStart(void)
{
    xTaskCreate(BlinkingTask, "blinking", 128, NULL, 3, NULL);
}
