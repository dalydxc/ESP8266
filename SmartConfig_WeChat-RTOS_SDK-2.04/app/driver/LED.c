#include"driver/LED.h"
#include"driver/gpio.h"

#include <stdio.h>
#include <string.h>
#include "esp_common.h"


typedef struct 
{
    led_status_t state;
    led_drak_level_t led_drak_level;
    led_mode_t led_mode;
    uint8_t io_num;
    os_timer_t led_tm;
    uint8_t led_level;
}led_dev_t;

static void led_stop_blink(led_handle_t led_handle)
{
    led_dev_t *led_dev = (led_dev_t *)led_handle;
    os_timer_disarm(&(led_dev->led_tm));
}
static void led_blink(led_handle_t led_handle,uint32_t period_ms)
{
    led_dev_t *led_dev = (led_dev_t *)led_handle;
    os_timer_arm(&(led_dev->led_tm),period_ms/2,1);
}
static void led_set_level(led_handle_t led_handle,uint8_t level)
{
    led_dev_t *led_dev = (led_dev_t *)led_handle;
    level = led_dev->led_mode == LED_NIGHT_MODE ? led_dev->led_drak_level: level;
    gpio_set_level(led_dev->io_num,(level^led_dev->led_drak_level));
    led_dev->led_level = level;
}
static void led_blink_cb(void *arg)
{
    led_dev_t *led_dev = (led_dev_t *)arg;
    led_dev->led_level = 0x01 & (~led_dev->led_level);
    led_set_level(led_dev,led_dev->led_level);
}

led_handle_t led_create(uint8_t io_num,led_drak_level_t led_drak_level)
{
    gpio_set_level(io_num,1);
    led_dev_t *led_p = (led_dev_t *)zalloc(sizeof(led_dev_t));
    led_p->io_num = io_num;
    led_p->led_drak_level = led_drak_level;
    led_p->led_mode = LED_NORMAL_MODE;
    led_p->state = LED_NORMAL_OFF;
    gpio_config_t io_config;
    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_DEF_OUTPUT;
    io_config.pin_bit_mask = (1<<io_num);
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);
    os_timer_disarm(&(led_p->led_tm));
    os_timer_setfn(&(led_p->led_tm),led_blink_cb,(void*)led_p);
    return (led_handle_t)led_p;
}
void led_state_write(led_handle_t led_handle,led_status_t state)
{
    led_dev_t *led_dev = (led_dev_t *)led_handle;
    switch (state)
    {
    case  LED_NORMAL_OFF:
        led_stop_blink(led_handle);
        led_set_level(led_handle,0);
        break;
    case  LED_NORMAL_ON:
        led_stop_blink(led_handle);
        led_set_level(led_handle,1);
        break;
    case  LED_QUICK_BLINK:
        led_blink(led_handle, 400);
        break;
    case  LED_SLOW_BLINK:
        led_blink(led_handle, 1000);
        break;
    default:
        break;
    }
    led_dev->state = state;
}