#ifndef __BUZZER_H_
#define __BUZZER_H_
#include "stdint.h"

#define BUZZER_FREQ 		   4000	/* ��λ��Hz */

#define BUZZER_TIMER_PERIOD    20

typedef enum
{
    BUZZER_START_NEW_CYCLE = 0,
    BUZZER_WATTING_OFF,
    BUZZER_WATTING_PERIOD_END,
    BUZZER_IDLE
} BuzzerStatusTypeDef;

typedef struct
{

	uint16_t times;
    uint32_t ticks_count;	
	uint32_t ticks_on;
    uint32_t ticks_off;

    BuzzerStatusTypeDef status;

}BuzzerHandleTypeDef;

/**
 * @brief 蜂鸣器初始化
 * 
 */
void buzzer_init(void);

/**
 * @brief 开启蜂鸣器
 * 
 */
void buzzer_on(void);

/**
 * @brief 关闭蜂鸣器
 * 
 */
void buzzer_off(void);

/**
 * @brief 蜂鸣器间隔鸣响
 * 
 * @param  ticks_on-鸣响时间
 * @param  ticks_off-关闭持续时长
 * @param  repeat_times-次数 0为无限次间隔鸣响
 */
void buzzer_toggle(uint32_t ticks_on, uint32_t ticks_off, uint16_t times);

/**
 * @brief 蜂鸣器句柄，需定时调用
 * 
 */
void buzzer_handler(void);

#endif
