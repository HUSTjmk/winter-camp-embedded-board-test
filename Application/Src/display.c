#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include "OLED_Data.h"

/*测试：OLED显示任务演示*/

static const uint8_t* display_images[] = {
    HappyImage, 
    SadImage, 
    AngryImage, 
    ListeningImage, 
    SpeakingImage, 
    IdleImage
};

void DisplayTask(void *pvParameters)
{
    uint8_t i = 0;
    while(1)
    {
        OLED_ShowImage(32, 2, 60, 60, display_images[i]);
        OLED_Update();
        i = (i + 1) % (sizeof(display_images) / sizeof(display_images[0]));
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void DisplayTaskStart(void)
{
    if(!OLED_IsInitialized()){
        OLED_Init();
    }
    xTaskCreate(DisplayTask, "display task", 256, NULL, 4, NULL);
}
