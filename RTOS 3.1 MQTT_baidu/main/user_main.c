/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#include"MQTTFreeRTOS.h"
#include "MQTTClient.h"
#include "sdkconfig.h"
#include "esp_log.h"
//#include "network_interface.h"
#define PublishData "{\"reported\": { \"tem\": 25,\"hum\": 60 }}"
#define SSID "ALERT LEJIA"
#define PASSWORD "alert1501lj"
#define MQTT_CLIENT_THREAD_STACK_WORDS 4096
#define CONFIG_MQTT_BROKER "c1j6fvv.mqtt.iot.bj.baidubce.com"
#define CONFIG_MQTT_PORT 1883
#define CONFIG_DEFAULT_MQTT_VERSION 3
#define MQTT_CLIENT_ID   	"temandhum"			// 客户端标识符// the ID of yourself, any string is OK,client would use this ID register itself to MQTT server
#define MQTT_USER        	"c1j6fvv/temandhum" 			// MQTT用户名				// your MQTT login name, if MQTT server allow anonymous login,any string is OK, otherwise, please input valid login name which you had registered
#define MQTT_PASS        	"fh9wg1phktda41jq"  
#define MQTT_SUB_TOPIC "$baidu/iot/shadow/temandhum/update/accepted"
                               
#define CONFIG_MQTT_PUB_TOPIC "$baidu/iot/shadow/temandhum/update"

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    
     case SYSTEM_EVENT_STA_GOT_IP:
         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
         printf("esp_wifi_connect");
         break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
         break;
     default:
         break;
    }
     return ESP_OK;
}

static void init_wifi(void)
{

    wifi_event_group=xEventGroupCreate();
    tcpip_adapter_init(); //初始化TCP/IP堆栈
    esp_event_loop_init(event_handler, NULL);
   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   esp_wifi_init(&cfg);
   esp_wifi_set_storage(WIFI_STORAGE_RAM);
    wifi_config_t wifi_config={
       .sta={
            .ssid =SSID,
            .password=PASSWORD,
       } 
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA,&wifi_config);
    esp_wifi_start();
}
void messageArrived(MessageData *data)
{
    printf("\n\rdxc\n\r");
    printf("Message arrived[len:%u]: %.*s", \
           data->message->payloadlen, data->message->payloadlen, (char *)data->message->payload);
}
static  void mqtt_client_thread()
{
    Network network;
    MQTTClient client;
    MQTTMessage message;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    NetworkInit(&network);
    int rc = 0;
    MQTTClientInit(&client,&network,0,NULL,0,NULL,0);
    for (;;)
    {
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true,portMAX_DELAY);
        if ((rc = NetworkConnect(&network, CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT)) != 0)
        {
            continue;
        }
        connectData.MQTTVersion = CONFIG_DEFAULT_MQTT_VERSION;
        connectData.clientID.cstring = MQTT_CLIENT_ID;
        connectData.username.cstring = MQTT_USER;
        connectData.password.cstring = MQTT_PASS;
        connectData.keepAliveInterval = 30;
        connectData.cleansession = 1;
        if((rc=MQTTConnect(&client,&connectData))!=0)
        {
            network.disconnect(&network);
            continue;
        }
        printf("\n\rMQTT Connected\n\r");
        MQTTStartTask(&client);
        if ((rc = MQTTSubscribe(&client, MQTT_SUB_TOPIC, 0, messageArrived) != 0))
        {
            network.disconnect(&network);
            continue;
        }
        printf("\n\rMQTT subscribe to topic OK\n\r");
        for (;;)
        {
            
            message.retained = 0;
            message.qos = 0;
            message.payload = PublishData;
            message.payloadlen = strlen(PublishData);
            if((rc=MQTTPublish(&client,CONFIG_MQTT_PUB_TOPIC,&message))!=0)
            {
                break;
            }
            printf("\n\rMQTT Publish to topic OK\n\r");
            vTaskDelay(5000/ portTICK_RATE_MS);
        }
        network.disconnect(&network);
    }
    vTaskDelete(NULL);
}
void app_main(void)
{

    printf("\n\rdcx\n\r");
    nvs_flash_init();
    init_wifi();

xTaskCreate(mqtt_client_thread, "mqtt_client_thread", 4096, NULL, 8, NULL);
}
