#include "driver/button.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "driver/gpio.h"

#include "adapter.h"
#include "esp_err.h"
typedef struct
{
    TickType_t interval;
    xTimerHandle tmr;
    button_cb cb;
    void *arg;
}button_cb_t;

typedef struct 
{
    uint8_t io_num;
    uint8_t active_level;
    button_status_t state;
    button_cb_t tap_press_cb;
    button_cb_t tap_push_cb;
    button_cb_t tap_rls_cb;
}button_dev_t;

static void button_tap_rls_cb(xTimerHandle tmr)
{
	 button_dev_t *btn = (button_dev_t *)pvTimerGetTimerID(tmr);
	    if(btn->active_level!=gpio_get_level(btn->io_num))
	    {
	        if(btn->tap_rls_cb.cb&&(btn->state!=BUTTON_STATE_PUSH))
	        {
	            btn->tap_rls_cb.cb(btn->tap_rls_cb.arg);
	        }
	        btn->state=BUTTON_STATE_IDLE;
	    }
}
static void button_tap_push_cb(xTimerHandle tmr)
{

    button_dev_t *btn = (button_dev_t *)pvTimerGetTimerID(tmr);
    if(btn->active_level==gpio_get_level(btn->io_num))
    {
        btn->state = BUTTON_STATE_PUSH;
        if(btn->tap_push_cb.cb)
        {
            btn->tap_push_cb.cb(btn->tap_push_cb.arg);
        }
    }

}
static void gpio_isr_handle(void* arg)
{
	os_printf("ok");
    button_dev_t *btn = (button_dev_t *)arg;
    portBASE_TYPE HPTaskAwoken = pdFALSE;
    uint8_t level = gpio_get_level(btn->io_num);
    if(level==btn->active_level)
    {
        if(btn->tap_push_cb.tmr)
        {
            xTimerStopFromISR(btn->tap_push_cb.tmr,&HPTaskAwoken);
            xTimerResetFromISR(btn->tap_push_cb.tmr,&HPTaskAwoken);

        }

    }
    else
    {
        if(btn->tap_rls_cb.tmr)
        {
            xTimerStopFromISR(btn->tap_rls_cb.tmr,&HPTaskAwoken);
            xTimerResetFromISR(btn->tap_rls_cb.tmr,&HPTaskAwoken);
        }
    }
    if(HPTaskAwoken) {
        portYIELD();
    }
    
}
static void button_free_tmr(xTimerHandle* tmr)
{
    if (tmr && *tmr) {
        xTimerStop(*tmr, portMAX_DELAY);
        xTimerDelete(*tmr, portMAX_DELAY);
        *tmr = NULL;
    }
}
void button_dev_cb(button_cb_type_t type,button_cb cb,void* arg,int interval_tick,button_handle_t button_handle)
{
    button_dev_t *btn = (button_dev_t *)button_handle;

    if(type==BUTTON_PUSH_CB)
    {
        btn->tap_push_cb.arg = arg;
        btn->tap_push_cb.cb = cb;
        btn->tap_push_cb.interval = interval_tick;
        button_free_tmr(&btn->tap_push_cb.tmr);
        btn->tap_push_cb.tmr = xTimerCreate((const char*) "qq", btn->tap_push_cb.interval, pdFALSE,
        		btn, button_tap_push_cb);

    }
    else if(type==BUTTON_STATE_RELEASE)
        {
            btn->tap_rls_cb.arg = arg;
            btn->tap_rls_cb.cb = cb;
            btn->tap_rls_cb.interval = interval_tick;
            button_free_tmr(&btn->tap_rls_cb.tmr);
            btn->tap_rls_cb.tmr = xTimerCreate((const char*) "qq", btn->tap_rls_cb.interval, pdFALSE,
            		btn, button_tap_rls_cb);

        }
}
button_handle_t button_dev_init(gpio_num_t gpio_num,button_active_t active)
{
    button_dev_t* btn = (button_dev_t *)calloc(1, sizeof(button_dev_t));
    btn->active_level = active;
    btn->io_num = gpio_num;
    btn->state = BUTTON_STATE_IDLE;
    btn->tap_rls_cb.cb = NULL;
    btn->tap_rls_cb.arg = NULL;
    btn->tap_rls_cb.interval=100 / portTICK_PERIOD_MS;
    btn->tap_rls_cb.tmr = xTimerCreate((const char*)"timer_release",btn->tap_rls_cb.interval,pdFALSE,
    btn,button_tap_rls_cb);
    btn->tap_push_cb.cb = NULL;
    btn->tap_push_cb.arg = NULL;
    btn->tap_push_cb.interval=100 / portTICK_PERIOD_MS;
    btn->tap_push_cb.tmr = xTimerCreate((const char*)"btn_psh_tmr",btn->tap_rls_cb.interval,pdFALSE,
    btn,button_tap_push_cb);
    if(btn->tap_push_cb.tmr==NULL)
    	os_printf("Timer Create fail");
    gpio_install_isr_service(0);
    gpio_config_t pGPIOConfig;
    pGPIOConfig.intr_type = GPIO_INTR_ANYEDGE;
    pGPIOConfig.mode = GPIO_MODE_INPUT;
    pGPIOConfig.pin_bit_mask = (1 << gpio_num);
    pGPIOConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&pGPIOConfig);
    gpio_isr_handler_add(gpio_num,gpio_isr_handle,btn);
    return btn;
}
