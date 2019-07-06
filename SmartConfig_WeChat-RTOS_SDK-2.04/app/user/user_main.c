/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "esp_common.h"
#include "driver/LED.h"
#include "driver/gpio.h"
#include "driver/button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/espconn.h"
#include "espressif/airkiss.h"
//#include "sntp.h"
//#include "spi_flash.h"
//#include "upgrade.h"
//#include "user_interface.h"
//#include "esp_sta.h"

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
led_handle_t LED;
void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
	struct station_config *sta_conf;
	switch(status)
	{
		case SC_STATUS_WAIT:

		break;
		case SC_STATUS_FIND_CHANNEL:
			break;
		case SC_STATUS_GETTING_SSID_PSWD:
			led_state_write(LED,LED_QUICK_BLINK);
			break;
		case SC_STATUS_LINK:
		sta_conf = pdata;
		wifi_station_set_config(sta_conf);
		wifi_station_disconnect();
		wifi_station_connect();
		break;
		case SC_STATUS_LINK_OVER:
			led_state_write(LED,LED_NORMAL_ON);
			smartconfig_stop();
			break;
	}
}
#define LEDTask_STA_SIZE 128
#define IO_NUM 4


void LED_Task(void *arg)
{
    uint8_t i=0;
    while (1)
    {
        
    	os_printf("\r\nok....\r\n");
    	vTaskDelay(1000/portTICK_RATE_MS);
    }
}

void ICACHE_FLASH_ATTR reset_smartconfig()
{
	led_state_write(LED,LED_SLOW_BLINK);
    smartconfig_set_type(SC_TYPE_AIRKISS);
    smartconfig_start(smartconfig_done);
}
void user_init(void)
{
    /*
        start a demo project on esp8266 rtos sdk.
        run joylink cloud connecting.
    */
   wifi_set_opmode(0x01);	
   LED=led_create(IO_NUM,LED_DRAK_HIGH);
    button_handle_t button_handle;
    button_handle = button_dev_init(0, 0);
    button_dev_cb(BUTTON_PUSH_CB,reset_smartconfig,"push",3000 / portTICK_RATE_MS,button_handle);
    xTaskCreate(
       (pdTASK_CODE)LED_Task,
       (const char *)"LED_Task",
       (uint16_t)LEDTask_STA_SIZE,
       (void *)NULL,
       2,
       (void *)NULL);
}
