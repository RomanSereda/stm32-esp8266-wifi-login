#include <builtin_led.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stdbool.h"

void blink(int count, int delay)
{
	for (int i = 0; i < count; ++i)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		osDelay(delay);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		osDelay(delay);
	}
}

void error_blink()
{
	blink(3, 30);
	blink(3, 300);
	blink(3, 30);
}
