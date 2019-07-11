#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "nvs.h"
/*
读取NVS函数
参数：
table NVS数据库
user 要读取参数的键
return：读取参数 uint16类型
 */
uint16_t mNVS_read(char* table,char* user)
{
    nvs_handle mHandleNvsRead;
    uint16_t nvs_u16 = 0;
    nvs_open(table, NVS_READONLY, &mHandleNvsRead);
    nvs_get_u16(mHandleNvsRead, user, &nvs_u16);
    nvs_close(mHandleNvsRead);
    return nvs_u16;
}
/*
写入NVS函数
参数：
table NVS数据库
user 要写入数据的键
writedata ：写入数据
 */

void mNVS_write(char* table,char* user,uint16_t writedata)
{
     nvs_handle mHandleNvswrite;
     nvs_open(table, NVS_READWRITE, &mHandleNvswrite);
     nvs_set_u16(mHandleNvswrite,user,writedata);
     nvs_commit(mHandleNvswrite);
     nvs_close(mHandleNvswrite);
}
void read_task(void)
{
	uint16_t a = 0;
	a=mNVS_read(MYTABLE, RUNTIME);
	while(1)
	{
		printf("\n\r%d\n\r",a);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
void app_main(void)
{
    
    nvs_flash_init();        //初始化NVS_flash
    mNVS_write(MYTABLE, RUNTIME, 1);
	xTaskCreate(read_task, "read_task", 1024, NULL, 5, NULL);
}