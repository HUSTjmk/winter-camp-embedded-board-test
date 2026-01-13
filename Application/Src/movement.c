#include "bsp_pwm.h"
#include "stdbool.h"
#include "movement.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// 队列句柄
#define CMD_QUEUE_LEN 5       // 队列长度，适配高频指令
static QueueHandle_t move_cmd_queue = NULL;
static void execute_move(MoveCmdType_t cmd, float speed);

// 3. 运动控制执行任务（最低优先级，仅执行指令）
static void MoveControlTask(void *pvParameters)
{
    MoveCmd_t current_cmd;
    uint32_t remaining_duration = 0;
    BaseType_t res;
    while(1) 
    {
        // 阻塞等待队列指令（有指令立即执行，无指令则等待）
        res = xQueueReceive(move_cmd_queue, &current_cmd, portMAX_DELAY);
        if(res != pdTRUE){
            continue; // 接收失败，继续等待
        }
        // 执行新指令
        execute_move(current_cmd.cmd, current_cmd.speed);
        
        //处理指令时长：
        remaining_duration = current_cmd.duration;
        
        // 指令执行中，周期性检查是否有新指令（打断当前）
        while (remaining_duration > 0) {
            // 检查队列是否有新指令
            if (uxQueueMessagesWaiting(move_cmd_queue) > 0) {
                break; // 有新指令，跳出循环执行新指令
            }
            
            // 短延时（10ms），保证打断响应速度
            vTaskDelay(pdMS_TO_TICKS(10));
            remaining_duration -= 10;
        }
       
       // 指令时长耗尽或被新指令打断，若不为超声PID指令则停止
        if (current_cmd.source != CMD_SOURCE_ULTRA_PID) {
           execute_move(MOVE_STOP, 0.0f);
        }        
    }
}


BaseType_t SendMoveCommand(MoveCmdType_t cmd, uint32_t duration, uint8_t source, float speed)
{
    MoveCmd_t new_cmd = {
        .cmd = cmd,
        .duration = duration,
        .source = source,
        .speed = speed
    };
    
    // 发送指令到队列（非阻塞，避免阻塞调用任务）
    return xQueueSendToBack(move_cmd_queue, &new_cmd, 0);
}


void MoveControlTaskStart(void)
{
    bsp_wheel_pwm_start();
    move_cmd_queue = xQueueCreate(CMD_QUEUE_LEN, sizeof(MoveCmd_t));
    configASSERT(move_cmd_queue != NULL);
    xTaskCreate(MoveControlTask, "MoveTask", 128, NULL, 2, NULL);
}

static void execute_move(MoveCmdType_t cmd, float speed)
{
    switch(cmd) {
        case MOVE_FORWARD:
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_1, speed);
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_2, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_1, speed);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_2, 0);
            break;
        case MOVE_BACKWARD:
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_2, speed);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_2, speed);
            break;
        case MOVE_LEFT:
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_2, speed);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_1, speed);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_2, 0);
            break;
        case MOVE_RIGHT:
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_1, speed);
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_2, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_2, speed);
            break;  
        case MOVE_STOP:
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_2, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_2, 0);
            break;
        default:
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL1_CHANNEL_2, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_1, 0);
            bsp_wheel_set_pwm(BSP_WHEEL2_CHANNEL_2, 0);
            break;
    }
}


