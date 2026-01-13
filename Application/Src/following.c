/**************************************************************/
/***************** 以下使用rtos，进行多任务调试 *****************/
/************ 操作：将板子的超声波接口与超声波模块连接 ************/
/**************** 现象：串口上实时打印超声测距 ******************/
/**************************************************************/

#include "following.h"
#include "bsp_ultrasonic.h"
#include "bsp_timer.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "config.h"

#if CONFIG_USE_UART_DEBUG
#include "stdio.h"
#endif

#define SPEED_OF_SOUND_CM_PER_US 0.0343f // 声速 cm/us
#define FILTER_ALPHA 0.3f         // 低通滤波系数

typedef enum {
    FOLLOWING_EVENT_BIT_START = (1 << 0),
} FollowingEventBits_t;

static EventGroupHandle_t following_event_group;
//static uint32_t time_elapsed;
static volatile uint32_t distance_cm;

static void ULTRASONIC_UART_DEBUG(uint32_t num)
{
#if CONFIG_USE_UART_DEBUG
    printf("Distance: %u cm\n", num);
#endif
}

static uint32_t ultrasonic_distance_filter(uint32_t distance)
{
    static uint32_t prev_distance = 0;
    uint32_t filtered = (uint32_t)(FILTER_ALPHA * distance + (1 - FILTER_ALPHA) * prev_distance);
    prev_distance = filtered;
    return filtered;
}

static void ultrasonic_on_echo_callback(uint8_t echo_pin_state)
{
    // Handle echo received event
    if(echo_pin_state == 1){
        // Echo started
        bsp_timer_clear_counter();
        bsp_timer_start();
    }
    else if(echo_pin_state == 0){
        // Echo ended
        bsp_timer_stop();
        //time_elapsed = bsp_timer_get_counter();
        uint32_t raw_distance = (bsp_timer_get_us() * SPEED_OF_SOUND_CM_PER_US) / 2;
        distance_cm = ultrasonic_distance_filter(raw_distance);
        bsp_timer_clear_counter();        
    }
    else{
        // Invalid state
    }
}


/* 懒得写PID控制了，先占个位置 */
static float pid_processing(uint32_t distance)
{
    return 0.0f;
}

static void FollowingTask(void* pvParameters)
{
    EventBits_t bits;
    while (1)
    {
        bits = xEventGroupWaitBits(following_event_group, FOLLOWING_EVENT_BIT_START, pdFALSE, pdFALSE, portMAX_DELAY);
        if((bits & FOLLOWING_EVENT_BIT_START) == 0){            
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        /* 这里进行pid处理，保留 */
        pid_processing(distance_cm);
        /* END OF PID PROCESSING */

        /* 更科学的做法是：再创建一个任务专门来处理超声波触发，但是那要多创建一个任务 */
        /* 而且由于桌宠速度相对声速可以忽略不计，又不是造火箭，就不那样写了 */
        bsp_ultrasonic_trigger_pulse(100);
        vTaskDelay(pdMS_TO_TICKS(30)); // 30ms触发一次
    }
}

static void DistanceDebugTask(void* pvParameters)
{
    EventBits_t bits;
    while (1)
    {
        bits = xEventGroupWaitBits(following_event_group, FOLLOWING_EVENT_BIT_START, pdFALSE, pdFALSE, portMAX_DELAY);
        if((bits & FOLLOWING_EVENT_BIT_START) == 0){            
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        ULTRASONIC_UART_DEBUG(distance_cm);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void StartFollowing(void)
{
    xEventGroupSetBits(following_event_group, FOLLOWING_EVENT_BIT_START);
}

void StopFollowing(void)
{
    xEventGroupClearBits(following_event_group, FOLLOWING_EVENT_BIT_START);
}
void FollowingTaskStart(void)
{
    following_event_group = xEventGroupCreate();
    configASSERT(following_event_group != NULL);
    bsp_timer_start();
    bsp_ultrasonic_set_echo_callback(ultrasonic_on_echo_callback);
	/* 启动跟随任务 */
    xTaskCreate(FollowingTask, "following_task", 256, NULL, 3, NULL);
    /* 启动距离调试任务 */
    xTaskCreate(DistanceDebugTask, "dis_debug_task", 128, NULL, 3, NULL);
}
