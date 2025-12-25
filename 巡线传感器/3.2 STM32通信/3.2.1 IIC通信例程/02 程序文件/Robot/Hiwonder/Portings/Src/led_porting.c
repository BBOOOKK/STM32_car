#include "global.h"

LEDHandleTypeDef led;
LEDHandleTypeDef led1;

static void led_object_init(LEDHandleTypeDef *handle, uint8_t id)
{
	memset(handle, 0, sizeof(LEDHandleTypeDef));
	handle->id = id;
	
}

static void led_write_pin(uint8_t new_state)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, (GPIO_PinState)new_state);
}

static void led1_write_pin(uint8_t new_state)
{
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, (GPIO_PinState)new_state);
}

void led_init()
{
	led_object_init(&led, 1);
	led_object_init(&led1, 2);
	led.write_pin = led_write_pin;
	led1.write_pin = led1_write_pin;
	
}

void led_on(uint8_t id)
{
	switch(id)
	{
		case 1:
			led.ticks_on = 1;
			led.ticks_off = 0;
			led.times = 0;
			led.status = LED_START_NEW_CYCLE;
			break;
		
		case 2:
			led1.ticks_on = 1;
			led1.ticks_off = 0;
			led1.times = 0;
			led1.status = LED_START_NEW_CYCLE;
			break;
		
		default:
			break;
	}
}

void led_off(uint8_t id)
{
	switch(id)
	{
		case 1:
			led.ticks_on = 0;
			led.ticks_off = 0;
			led.times = 0;
			led.status = LED_START_NEW_CYCLE;
			break;
		
		case 2:
			led1.ticks_on = 0;
			led1.ticks_off = 0;
			led1.times = 0;
			led1.status = LED_START_NEW_CYCLE;
			break;
		
		default:
			break;
	}
}

void led_flash(uint8_t id, uint32_t ticks_on, uint32_t ticks_off, uint16_t times)
{
	switch(id)
	{
		case 1:
			led.ticks_on = ticks_on;
			led.ticks_off = ticks_off;
			led.times = times;
			led.status = LED_START_NEW_CYCLE;
			break;
		
		case 2:
			led1.ticks_on = ticks_on;
			led1.ticks_off = ticks_off;
			led1.times = times;
			led1.status = LED_START_NEW_CYCLE;
			break;
		
		default:
			break;
	}
}

void leds_handler()
{
	led_handler(&led);
	led_handler(&led1);
}
