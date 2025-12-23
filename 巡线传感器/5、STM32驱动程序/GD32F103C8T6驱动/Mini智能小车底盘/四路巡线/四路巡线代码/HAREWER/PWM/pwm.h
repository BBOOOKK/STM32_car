#ifndef __PWM_H
#define	__PWM_H

#include "stm32f10x.h"
#define   PWMA_IN1  TIM3->CCR1
#define   PWMA_IN2  TIM3->CCR2
#define   PWMB_IN1  TIM3->CCR3
#define   PWMB_IN2  TIM3->CCR4

#define   PWMC_IN1  TIM4->CCR1
#define   PWMC_IN2  TIM4->CCR2
#define   PWMD_IN1  TIM4->CCR3
#define   PWMD_IN2  TIM4->CCR4


void PWM_Int(u16 arr,u16 psc, u16 arr2,u16 psc2);
//void Set_PWM(int motor1_left,int motor1_right,int motor2_left,int motor2_right);

#endif

