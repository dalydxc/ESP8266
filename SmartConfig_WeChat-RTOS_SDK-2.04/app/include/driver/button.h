#ifndef _BUTTON_H_
#define _BUTTON_H_
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* button_cb)(void*);
typedef void* button_handle_t;
    typedef enum
    {
        BUTTON_ACTIVE_HIGH,
        BUTTON_ACTIVE_LOW,
    } button_active_t;
    typedef enum
    {
        BUTTON_PUSH_CB,
        BUTTON_RELEASE_CB,
    } button_cb_type_t;
    typedef enum
    {
        BUTTON_STATE_IDLE=0,
        BUTTON_STATE_PUSH,
        BUTTON_STATE_RELEASE,
    } button_status_t;

    button_handle_t button_dev_init(gpio_num_t gpio_num, button_active_t active);
    void button_dev_cb(button_cb_type_t type, button_cb cb, void *arg, int interval_tick, button_handle_t button_handle);

#ifdef __cplusplus
}
#endif


#endif
