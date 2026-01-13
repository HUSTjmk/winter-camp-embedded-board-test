/**************************************************************/
/***************** 以下使用rtos，进行多任务调试 *****************/
/************** 操作：将板子和语音模块串口TxRx连接 **************/
/****** 现象：唤醒时串口打印 WAKE，说“摇尾巴”时串口打印 WAG ******/
/**************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "audio.h"
#include "bsp_uart.h"
#include "following.h"
#include "movement.h"
#include "tail.h"
#include "main.h"
#include "string.h"
#include "stdbool.h"
#include "config.h"

#if CONFIG_USE_UART_DEBUG
#include "stdio.h"
#endif

#define AUDIO_DATA_SIZE 64

QueueHandle_t audio_rx_buf_queue;

static uint8_t audio_data[AUDIO_DATA_SIZE];
const uint8_t audio_wakeup_pack[] = {0xAA,0x55,0x00,0x00,0x55,0xAA};
const uint8_t audio_start_follow_pack[] = {0xAA,0x55,0x01,0x00,0x55,0xAA};
const uint8_t audio_stop_follow_pack[]  = {0xAA,0x55,0x01,0x00,0x55,0xAA};
const uint8_t audio_stop_move_pack[]     = {0xAA,0x55,0x02,0x00,0x55,0xAA};
const uint8_t audio_move_forward_pack[]  = {0xAA,0x55,0x02,0x01,0x55,0xAA};
const uint8_t audio_move_backward_pack[] = {0xAA,0x55,0x02,0x02,0x55,0xAA};
const uint8_t audio_turn_right_pack[]   = {0xAA,0x55,0x02,0x03,0x55,0xAA};
const uint8_t audio_turn_left_pack[]    = {0xAA,0x55,0x02,0x04,0x55,0xAA};
const uint8_t audio_wag_pack[]    = {0xAA,0x55,0x03,0x00,0x55,0xAA};

typedef struct{
    uint16_t len;
    uint8_t data[AUDIO_DATA_SIZE];
}AudioRxBuf_t;

static void AUDIO_UART_DEBUG(char* str)
{
#if CONFIG_USE_UART_DEBUG
    printf("AUD: %s\n", str);
#endif
}

static void audio_on_rx_data_callback(uint16_t size)
{
    BaseType_t res = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    AudioRxBuf_t rx_data_buf;
    size = size <= AUDIO_DATA_SIZE ? size : AUDIO_DATA_SIZE;
    memcpy(rx_data_buf.data, audio_data, size);
    rx_data_buf.len = size;
    res = xQueueSendFromISR(audio_rx_buf_queue, &rx_data_buf, &xHigherPriorityTaskWoken);
    if(res != pdPASS){
        Error_Handler();
    }
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); 
    }
}

static void parse_audio_pack(AudioRxBuf_t* buf)
{
    //此处可添加音频数据包处理代码
    if(memcmp(buf->data, audio_wakeup_pack, buf->len*sizeof(uint8_t)) == 0){
        //收到唤醒包，显示OK
        AUDIO_UART_DEBUG("WAKE");
    }else if(memcmp(buf->data, audio_wag_pack, buf->len*sizeof(uint8_t)) == 0){
        //收到摇头包，显示OK
        AUDIO_UART_DEBUG("WAG");
        TailStartWagging(30, 60, 200, 5);
    }else if(memcmp(buf->data, audio_start_follow_pack, buf->len*sizeof(uint8_t)) == 0){
        AUDIO_UART_DEBUG("START FOLL");
        StartFollowing();
    }else if(memcmp(buf->data, audio_stop_follow_pack, buf->len*sizeof(uint8_t)) == 0){
        AUDIO_UART_DEBUG("STOP FOLL");
        StopFollowing();
    }else if(memcmp(buf->data, audio_move_forward_pack, buf->len*sizeof(uint8_t)) == 0){
        AUDIO_UART_DEBUG("MOVE FWD");
        if(SendMoveCommand(MOVE_FORWARD, 5000, CMD_SOURCE_UART, 0.5f) != pdPASS){
            AUDIO_UART_DEBUG("MOVE ERR");
        }
    }else if(memcmp(buf->data, audio_move_backward_pack, buf->len*sizeof(uint8_t)) == 0){
        AUDIO_UART_DEBUG("MOVE BWD");
        if(SendMoveCommand(MOVE_BACKWARD, 5000, CMD_SOURCE_UART, 0.5f) != pdPASS){
            AUDIO_UART_DEBUG("MOVE ERR");
        }
    }else if(memcmp(buf->data, audio_turn_right_pack, buf->len*sizeof(uint8_t)) == 0){
        AUDIO_UART_DEBUG("TURN R");
        if(SendMoveCommand(MOVE_RIGHT, 5000, CMD_SOURCE_UART, 0.5f) != pdPASS){
            AUDIO_UART_DEBUG("MOVE ERR");
        }
    }else if(memcmp(buf->data, audio_turn_left_pack, buf->len*sizeof(uint8_t)) == 0){
        AUDIO_UART_DEBUG("TURN L");
        if(SendMoveCommand(MOVE_LEFT, 5000, CMD_SOURCE_UART, 0.5f) != pdPASS){
            AUDIO_UART_DEBUG("MOVE ERR");
        }
    }else if(memcmp(buf->data, audio_stop_move_pack, buf->len*sizeof(uint8_t)) == 0){
        AUDIO_UART_DEBUG("STOP MOVE");
        if(SendMoveCommand(MOVE_STOP, 5000, CMD_SOURCE_UART, 0.0f) != pdPASS){
            AUDIO_UART_DEBUG("MOVE ERR");
        }
    }else{
        //收到其他包，显示Err
        AUDIO_UART_DEBUG("ERR");
    }
}

static void AudioTask(void* pvParameters)
{
    BaseType_t res = 0;
    AudioRxBuf_t rx_data_buf = {.len = 0, .data = {0}};
    while (1)
    {
        res = xQueueReceive(audio_rx_buf_queue, &rx_data_buf, portMAX_DELAY);
        if(res != pdPASS){            
        }
        bsp_uart_transmit(rx_data_buf.data, rx_data_buf.len, 1000);
        parse_audio_pack(&rx_data_buf);        
    }
}

void AudioTaskStart(void)
{  
    audio_rx_buf_queue = xQueueCreate(2, sizeof(AudioRxBuf_t));
    configASSERT(audio_rx_buf_queue != NULL);
    bsp_uart_set_rx_event_callback(audio_on_rx_data_callback);
    bsp_enable_uart_receive_it(audio_data, AUDIO_DATA_SIZE, true);

    xTaskCreate(AudioTask, "audio task", 256, NULL, 3, NULL);
}
