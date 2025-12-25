#ifndef __LED_H__
#define __LED_H__

/**
 * @file led.h
 * @author Lucas
 * @brief LED驱动
 * @version 1.0
 * @date 2024-12-13
 *
 * @copyright Copyright (c) 2024 Hiwonder
 *
 */
#include "stdint.h"

#define LED_TIMER_PERIOD    20

typedef enum {
    LED_START_NEW_CYCLE = 0,
    LED_WATTING_OFF,
    LED_WATTING_PERIOD_END,
    LED_IDLE 
} LEDStatusTypeDef;


typedef struct {
    LEDStatusTypeDef status;

    uint8_t  id;
	uint16_t times;
    uint32_t ticks_count;	/* 用于记录当前时间 */
	uint32_t ticks_on;
    uint32_t ticks_off;

    void (*write_pin)(uint8_t new_state);
}LEDHandleTypeDef;


/**
 * @brief LED初始化
 * 
 */
void led_init(void);

/**
 * @brief 开启LED
 * 
 * @param  id-取值为1或2
 */
void led_on(uint8_t id);

/**
 * @brief 关闭LED
 * 
 * @param  id-取值为1或2
 */
void led_off(uint8_t id);

/**
 * @brief LED闪烁
 * 
 * @param  id-取值为1或2
 * @param  ticks_on-鸣响时间
 * @param  ticks_off-关闭持续时长
 * @param  times-次数 0为无限次间隔鸣响
 */
void led_flash(uint8_t id, uint32_t ticks_on, uint32_t ticks_off, uint16_t times);


void led_handler(LEDHandleTypeDef* handle);

/**
 * @brief LED句柄 需定时调用
 * 
 */
void leds_handler(void);
#endif
