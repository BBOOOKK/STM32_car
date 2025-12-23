#ifndef __SERVOR_H
#define	__SERVOR_H
#include "stm32f10x.h"

#define servo_M 420    //舵机中值pwm，测试得出
#define servo_Limit 200   //舵机限幅pwm，测试得出

void Servo_Init(void);
void Servo_control(int dir);

#endif /* __GPIO_H */
