#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

#include "FreeRTOS.h"
#include "task.h"

// 运动指令类型（保持原有定义）
typedef enum {
    MOVE_STOP = 0,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT
} MoveCmdType_t;

typedef enum {
    CMD_SOURCE_ULTRA = 0,
    CMD_SOURCE_UART = 1,
    CMD_SOURCE_GES = 2,
    CMD_SOURCE_ULTRA_PID = 3
} MoveCmdSource_t;

// 指令结构体（增加指令来源标记，区分超声/串口/手势）
typedef struct {
    MoveCmdType_t cmd;          // 运动指令
    uint32_t duration;        // 执行时长（ms）
    uint8_t source;           // 指令来源：0=超声，1=串口，2=手势
    float speed;              // 速度参数
} MoveCmd_t;
void MoveControlTaskStart(void);
BaseType_t SendMoveCommand(MoveCmdType_t cmd, uint32_t duration, uint8_t source, float speed);

#endif /* __MOVEMENT_H__ */
