/**
* @par Copyright (C): 2010-2019, Shenzhen Yahboom Tech
* @file         bsp.c
* @author       liusen
* @version      V1.0
* @date         2015.01.03
* @brief        驱动总入口
* @details      
* @par History  见如下说明
*                 
* version:	liusen_20170717
*/


#include "bsp.h"

/**
* Function       bsp_init
* @author        liusen
* @date          2015.01.03    
* @brief         硬件设备初始化
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void bsp_init(void)
{
	delay_init();
	
	//Colorful_GPIO_Init();				/*七彩探照灯*/
	MOTOR_GPIO_Init();  				/*电机GPIO初始化*/
	Motor_PWM_Init(7200,0, 7200, 0);	/*不分频。PWM频率 72000000/7200=10khz	  */
	//Servo_GPIO_Init();	
	//TIM1_Int_Init(9, 72);				/*100Khz的计数频率，计数到10为10us  */ 
	LineWalking_GPIO_Init();			/*巡线传感器初始化*/
}
