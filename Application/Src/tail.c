/**************************************************************/
/***************** 以下使用rtos，进行多任务调试 *****************/
/************** 操作：将板子的舵机接口与尾巴舵机连接 *************/
/************** 现象：板子能够根据指令控制尾巴摆动 ***************/
/**************************************************************/
#include "bsp_pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tail.h"
#include "queue.h"
#include "main.h"
/*测试：舵机控制尾巴任务演示*/
#pragma anon_unions

typedef enum {
    TAIL_SET_ANGLE  = 0,
    TAIL_WAG        = 1
}TailActionType_t;

typedef struct{
    TailActionType_t type;
    union{
        struct{
            uint16_t angle;
        }set_angle;
        struct{
            uint16_t start_angle;   //尾巴摆动起始角度
            uint16_t end_angle;     //尾巴摆动结束角度
            uint16_t cycle_delay;   //每次摆动延时
            uint16_t count;         //摆动次数
        }wag;
    };
}TailActionCmd_t;

static QueueHandle_t tail_action_queue = NULL;

static void tail_set_angle(int16_t angle)
{
    if(angle > 180) angle = 180;
    if(angle < 0) angle = 0;
    //将角度转换为PWM占空比
    float duty_cycle = (angle / 180.0f) * (BSP_SERVO_MAX_DUTY_CYCLE - BSP_SERVO_MIN_DUTY_CYCLE) + BSP_SERVO_MIN_DUTY_CYCLE;
    bsp_servo_set_pwm(duty_cycle);
}

void TailSetTargetAngle(uint16_t angle)
{
    //tail_target_angle = angle;
    TailActionCmd_t cmd = {
        .type = TAIL_SET_ANGLE,
        .set_angle = {
            .angle = angle
        }
    };
    xQueueSend(tail_action_queue, &cmd, portMAX_DELAY);
}

void TailStartWagging(uint16_t start_angle, uint16_t end_angle, uint16_t cycle_delay, uint16_t count)
{
    TailActionCmd_t cmd = {
        .type = TAIL_WAG,
        .wag = {
            .start_angle = start_angle,
            .end_angle = end_angle,
            .cycle_delay = cycle_delay,
            .count = count
        }
    };
    xQueueSend(tail_action_queue, &cmd, portMAX_DELAY);
}

void TailTask(void *pvParameters)
{
    TailActionCmd_t cmd;
    while(1)
    {
        xQueueReceive(tail_action_queue, &cmd, portMAX_DELAY);
        switch(cmd.type)
        {
            case TAIL_SET_ANGLE:
                tail_set_angle(cmd.set_angle.angle);
                break;
            case TAIL_WAG:
                for(uint16_t i = 0; i < cmd.wag.count; i++){
                    tail_set_angle(cmd.wag.start_angle);
                    vTaskDelay(pdMS_TO_TICKS(cmd.wag.cycle_delay));
                    tail_set_angle(cmd.wag.end_angle);
                    vTaskDelay(pdMS_TO_TICKS(cmd.wag.cycle_delay));
                }
                break;
        }
    }
}


void TailTaskStart(void)
{
    tail_action_queue = xQueueCreate(4, sizeof(TailActionCmd_t));
    configASSERT(tail_action_queue != NULL);
    bsp_servo_pwm_start();
    tail_set_angle(90); // 初始化时将尾巴设置为中间位置

    xTaskCreate(TailTask, "tail_task", 256, NULL, 2, NULL);
}

