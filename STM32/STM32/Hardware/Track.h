#ifndef __TRACK_H__
#define __TRACK_H__
#include "stm32f10x.h"
#define Track_L	GPIO_Pin_0
#define Track_l	GPIO_Pin_1
#define Track_M	GPIO_Pin_2
#define Track_r	GPIO_Pin_3
#define Track_R	GPIO_Pin_4
void Track_Init(void);
void Read_Track_DATA(uint8_t* arr);
float Track_Err(uint16_t car_state);

#endif
