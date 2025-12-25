#ifndef __ADC_SAMPLE_H_
#define __ADC_SAMPLE_H_

#include "stdint.h"

/**
 * @brief ADC通道采样初始化
 * 
 */
void adc_sample_init(void);

/**
 * @brief ADC通道采样局部，需定时调用
 * 
 */
void adc_sample_handler(void);

/**
 * @brief 获取实际电池电量
 * 
 * @return float 
 */
float get_battery_volocity(void);

/**
 * @brief 获取按键状态
 * 
 * @return 0-无按键按下，1-按键1按下，2-按键2按下，3-按键1和按键2同时按下
 */
uint8_t get_button_state(void);

/**
 * @brief 获取声音传感器的返回结果
 * 
 * @return uint16_t 
 */
uint16_t get_sound_value(void);

#endif
