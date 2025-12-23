/**
* @par Copyright (C): Shenzhen Yahboom Tech
* @file         bsp.c
* @brief        驱动总入口
*/

#include "bsp.h"
#include "moto.h"
#include "pwm.h"
#include "adc.h"
#include "usart.h"
#include "encoder.h"
#include "servo.h"
#include "ultrasonic.h"
#include "linewalking.h"
#include "iravoid.h"
#include "ps2_control.h"
#include "three_linewalking.h"
/**
* Function       bsp_init  
* @brief         硬件设备初始化
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void bsp_init(void)
{
	
	 SystemInit(); //配置系统时钟为72M   
   delay_init();    //延时函数初始化
   uart_init(9600);		//串口初始化
 //  adc_Init();				//ADC1的初始化   
		PWM_Int(7199,0, 7199,0);      //初始化pwm输出 72000 000 /7199+1=10000 
		Servo_GPIO_Init();				    /*舵机接口初始化*/	
		TIM1_Int_Init(9, 72);				/*100Khz的计数频率，计数到10为10us  */ 
		Ultrasonic_GPIO_Init();				/*超声波GPIO初始化*/
		Ultrasonic_Timer2_Init();		/*超声波TIM2初始化*/
		LineWalking_GPIO_Init();			/*巡线传感器初始化*/
 	  IRAvoid_GPIO_Init();				/*红外避障传感器初始化*/
		PS2_Init();
	  three_LineWalking_GPIO_Init();/*三路巡线传感器初始化*/
	
}

