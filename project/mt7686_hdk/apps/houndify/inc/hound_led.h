#ifndef __HOUND_LED_H__
#define __HOUND_LED_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void led_io_config(void);
void blue_led(bool bOn);
void red_led(bool bOn);

#ifdef __cplusplus
}
#endif

#endif
