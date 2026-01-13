/**************************************************************/
/***************** 以下使用rtos，进行多任务调试 *****************/
/*************** 操作：将板子和手势传感器正确连接 ***************/
/********* 现象：有手势动作时时串口打印相对应的手势动作 *********/
/**************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "paj7620.h"
#include "bsp_gesture.h"
#include "gesture.h"
#include "string.h"
#include "stm32f1xx_hal.h"
#include "movement.h"
#include "config.h"

#if CONFIG_USE_UART_DEBUG
#include "stdio.h"
#endif

static void parse_gesture(Gesture_StatusTypeDef gesture);

/*测试：手势识别任务演示*/
SemaphoreHandle_t gesture_detect_semaphore;
TimerHandle_t gesture_stop_move_timer;

static void GESTURE_UART_DEBUG(char* str)
{
#if CONFIG_USE_UART_DEBUG
    printf("GES: %s\n", str);
#endif
}

static void gesture_on_detected_callback(void)
{
    BaseType_t res = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    res = xSemaphoreGiveFromISR(gesture_detect_semaphore, &xHigherPriorityTaskWoken);
    if(res == pdFALSE){
        //Error_Handler();
        return;
    }
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); 
    }
}

static void GestureTask(void* pvParameters)
{
    Gesture_StatusTypeDef gesture;
    parse_gesture(gesture);
    while(1)
    {        
        xSemaphoreTake(gesture_detect_semaphore, portMAX_DELAY);
        // 读取手势数据
        taskENTER_CRITICAL();
        gesture = PAJ7620_Detect();
        taskEXIT_CRITICAL();
        parse_gesture(gesture);
    }
}

void GestureTaskStart(void)
{
    gesture_detect_semaphore = xSemaphoreCreateBinary();
    configASSERT(gesture_detect_semaphore != NULL);
    // 初始化手势传感器
    uint8_t init_status = PAJ7620_Init();
    if(init_status != 0){
        // 初始化失败，进行错误处理
    }
    // 设置手势检测回调函数
    bsp_gesture_set_detect_callback(gesture_on_detected_callback);
    /*  
        测试发现：启用中断模式时，模块上电后无法清除中断标志位，所以无法触发中断。
        这里初始化时读取一次手势传感数据清除标志位，使中断功能可以正常使用 
        学长是用示波器探测波形解决的。我们鼓励各位尝试各种调试方法解决问题。
    */
	PAJ7620_Detect();
    
    xTaskCreate(GestureTask, "gesture_task", 256, NULL, 5, NULL);
}

static void parse_gesture(Gesture_StatusTypeDef gesture)
{
    switch(gesture)
        {
            case GES_NONE:
                // No gesture detected
                GESTURE_UART_DEBUG("NONE");
                break;
            case GES_FORWARD:
                GESTURE_UART_DEBUG("FORWARD");
                //MoveForward(0.5f);
                // Handle forward gesture
                break;
            case GES_BACKWARD:
                GESTURE_UART_DEBUG("BACKWARD");
                //MoveBackward(0.5f);
                // Handle backward gesture
                break;
            case GES_RIGHT:
                GESTURE_UART_DEBUG("RIGHT");
                //TurnRight(0.5f);
                // Handle right gesture
                break;
            case GES_LEFT:
                GESTURE_UART_DEBUG("LEFT");
                //TurnLeft(0.5f);
                // Handle left gesture
                break;
            case GES_UP:
                GESTURE_UART_DEBUG("UP");
                // Handle up gesture
                break;
            case GES_DOWN:
                GESTURE_UART_DEBUG("DOWN");
                // Handle down gesture
                break;
            case GES_CLOCKWISE:
                GESTURE_UART_DEBUG("CLOCKWISE");
                // Handle clockwise gesture
                break;
            case GES_COUNTERCLOCKWISE:
                GESTURE_UART_DEBUG("COUNTERCLOCKWISE");
                // Handle counterclockwise gesture
                break;
            case GES_WAVE:
                GESTURE_UART_DEBUG("WAVE");
                // Handle wave gesture
                break;
            default:
                // No gesture detected
                GESTURE_UART_DEBUG("NONE");
                break;
        }                
}
