#ifndef __TRACK_H__
#define __TRACK_H__
#include "stm32f10x.h"

/*
 * 8-route line sensor mapping (default: PA0..PA7, left -> right).
 * If your wiring differs, change these macros and/or the GPIO clocks
 * in Track_Init().
 */
#define TRACK_PORT      GPIOA
#define TRACK_RCC       RCC_APB2Periph_GPIOA

/*
 * Sensor polarity:
 *   1: sensor outputs LOW (0) when "on line"; use !GPIO_ReadInputDataBit()
 *   0: sensor outputs HIGH (1) when "on line"; use  GPIO_ReadInputDataBit()
 */
#define TRACK_ACTIVE_LOW 1

/* Left -> Right, 8 sensors */
#define Track_S0 GPIO_Pin_0
#define Track_S1 GPIO_Pin_1
#define Track_S2 GPIO_Pin_2
#define Track_S3 GPIO_Pin_3
#define Track_S4 GPIO_Pin_4
#define Track_S5 GPIO_Pin_5
#define Track_S6 GPIO_Pin_6
#define Track_S7 GPIO_Pin_7

/* Backward-compatible aliases (if you still use the old 5-route names) */
#define Track_L Track_S0
#define Track_l Track_S1
#define Track_M Track_S2
#define Track_r Track_S3
#define Track_R Track_S4

void Track_Init(void);
void Read_Track_DATA(uint8_t* arr);
float Track_Err(uint16_t car_state);

#endif