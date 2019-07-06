#ifndef _LED_H_
#define _LED_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "c_types.h"
    typedef enum
    {
        LED_NORMAL_OFF,
        LED_NORMAL_ON,
        LED_QUICK_BLINK,
        LED_SLOW_BLINK
    } led_status_t;

    typedef enum
    {
        LED_DRAK_LOW,
        LED_DRAK_HIGH
    } led_drak_level_t;

    typedef enum
    {
        LED_NORMAL_MODE,
        LED_NIGHT_MODE,
    } led_mode_t;

    typedef void *led_handle_t;

    led_handle_t led_create(uint8_t io_num, led_drak_level_t led_drak_level);
    void led_state_write(led_handle_t led_handle, led_status_t state);

#ifdef __cplusplus
}
#endif


#endif
