// <<< Use Configuration Wizard in Context Menu >>>
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "main.h"
#include <string.h>
#include "led.h"

/* 宏函数 获得A的低八位 */
#define GET_LOW_BYTE(A) ((uint8_t)(A))
/* 宏函数 获得A的高八位 */
#define GET_HIGH_BYTE(A) ((uint8_t)((A) >> 8))
/* 宏函数 将高地八位合成为十六位 */
#define BYTE_TO_HW(A, B) ((((uint16_t)(A)) << 8) | (uint8_t)(B))

/* 单位：ms */
#define LED_HANDLER_PERIOD       	LED_TIMER_PERIOD
#define BUTTON_HANDLER_PERIOD    	20
#define BUZZER_HANDLER_PERIOD       25
#define SOFTWARE_VERSION			1

extern LEDHandleTypeDef led;
extern LEDHandleTypeDef led1;

#endif
// <<< end of configuration section >>>
