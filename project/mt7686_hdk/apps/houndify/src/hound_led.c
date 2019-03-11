#include <stdint.h>
#include "hal.h"

#define LED_B_IO		HAL_GPIO_13
#define LED_R_IO		HAL_GPIO_18

void led_io_config(void)
{
	hal_gpio_init(LED_B_IO);
	hal_pinmux_set_function(LED_B_IO, 0);
	hal_gpio_set_direction(LED_B_IO, HAL_GPIO_DIRECTION_OUTPUT);

	hal_gpio_init(LED_R_IO);
	hal_pinmux_set_function(LED_R_IO, 0);
	hal_gpio_set_direction(LED_R_IO, HAL_GPIO_DIRECTION_OUTPUT);
}

void blue_led(bool bOn)
{
	hal_gpio_data_t	on = bOn ? HAL_GPIO_DATA_HIGH : HAL_GPIO_DATA_LOW;

	hal_gpio_set_output(LED_B_IO, on);
}

void red_led(bool bOn)
{
	hal_gpio_data_t	on = bOn ? HAL_GPIO_DATA_HIGH : HAL_GPIO_DATA_LOW;

	hal_gpio_set_output(LED_R_IO, on);
}
