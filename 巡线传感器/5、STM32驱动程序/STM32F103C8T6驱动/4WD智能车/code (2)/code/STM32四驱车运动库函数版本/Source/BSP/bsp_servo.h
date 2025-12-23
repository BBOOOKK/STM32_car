/**
* @par Copyright (C): 2010-2019, Shenzhen Yahboom Tech
* @file         bsp_servo.h	
* @author       liusen
* @version      V1.0
* @date         2017.07.17
* @brief        6个舵机控制驱动头文件
* @details      
* @par History  见如下说明
*                 
* version:		liusen_20170717
*/

#ifndef __BSP_SERVO_H__
#define __BSP_SERVO_H__

#include "stm32f10x.h"

/*定义需要初始化的舵机宏定义开关*/

#define USE_SERVO_J1
#define USE_SERVO_J2
#define USE_SERVO_J3
#define USE_SERVO_J4
#define USE_SERVO_J5
#define USE_SERVO_J6

extern int Angle_J1;
extern int Angle_J2;
extern int Angle_J3;
extern int Angle_J4;
extern int Angle_J5;
extern int Angle_J6;

#define Servo_J1_PIN	GPIO_Pin_11
#define Servo_J2_PIN	GPIO_Pin_13
#define Servo_J3_PIN	GPIO_Pin_14
#define Servo_J4_PIN	GPIO_Pin_15
#define Servo_J5_PIN	GPIO_Pin_8
#define Servo_J6_PIN	GPIO_Pin_1

#define Servo_J1_PORT	GPIOA
#define Servo_J2_PORT	GPIOB
#define Servo_J3_PORT	GPIOB
#define Servo_J4_PORT	GPIOB
#define Servo_J5_PORT	GPIOA
#define Servo_J6_PORT	GPIOA

#define Servo_J1_RCC	RCC_APB2Periph_GPIOA
#define Servo_J2_RCC	RCC_APB2Periph_GPIOB
#define Servo_J3_RCC    RCC_APB2Periph_GPIOB
#define Servo_J4_RCC	RCC_APB2Periph_GPIOB
#define Servo_J5_RCC	RCC_APB2Periph_GPIOA
#define Servo_J6_RCC	RCC_APB2Periph_GPIOA

void Servo_J1(int v_iAngle);/*定义一个脉冲函数，用来模拟方式产生PWM值*/
void Servo_J2(int v_iAngle);/*定义一个脉冲函数，用来模拟方式产生PWM值*/
void Servo_J3(int v_iAngle);/*定义一个脉冲函数，用来模拟方式产生PWM值*/
void Servo_J4(int v_iAngle);/*定义一个脉冲函数，用来模拟方式产生PWM值*/
void Servo_J5(int v_iAngle);/*定义一个脉冲函数，用来模拟方式产生PWM值*/
void Servo_J6(int v_iAngle);/*定义一个脉冲函数，用来模拟方式产生PWM值*/



void front_detection(void);
void left_detection(void);
void right_detection(void);



#endif

