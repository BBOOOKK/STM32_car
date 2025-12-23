/**
* @par Copyright (C): 2010-2019, Shenzhen Yahboom Tech
* @file         bsp_timer.h	
* @author       liusen
* @version      V1.0
* @date         2017.07.17
* @brief        定时器
* @details      
* @par History  见如下说明
*                 
* version:		liusen_20170717
*/

#include "bsp_timer.h"
#include "bsp_colorful.h"
#include "bsp_servo.h"



/**
* Function       TIM1_Int_Init
* @author        liusen
* @date          2015.01.03    
* @brief         定时器1初始化接口
* @param[in]     arr：自动重装值。psc：时钟预分频数
* @param[out]    void
* @retval        void
* @par History   这里时钟选择为APB1的2倍，而APB1为36M
*/
void TIM1_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = (psc-1); //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim   //36Mhz
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;    //重复计数关闭
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE ); //使能指定的TIM1中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  //TIM1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM1, ENABLE);  //使能TIMx					 
}
/**
* Function       TIM1_Int_Init
* @author        liusen
* @date          2015.01.03    
* @brief         定时器1中断服务程序: 主要控制6路舵机运行
* @param[in]     arr：自动重装值。psc：时钟预分频数
* @param[out]    void
* @retval        void
* @par History   这里时钟选择为APB1的2倍，而APB1为36M
*/
int num = 0;


void TIM1_UP_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx更新中断标志 
		num++;
		/*switch(num)
		{
			case 1:  Colorful_Control(1,0,0); break;
			case 2:  Colorful_Control(0,1,0); num = 0; break;
			//case 3:  Colorful_Control(0,0,1); break;
			//case 4:  Colorful_Control(0,1,1); break;
			//case 5:  Colorful_Control(1,1,0); break;
			//case 6:  Colorful_Control(1,0,1); break;
			//case 7:  Colorful_Control(1,1,1); num = 0; break;
			default: break;
		}*/
	

		if(num <= (Angle_J1 * 11 + 500)/10)
		{
			GPIO_SetBits(Servo_J1_PORT, Servo_J1_PIN );		//将舵机接口电平置高
		}
		else
		{
			GPIO_ResetBits(Servo_J1_PORT, Servo_J1_PIN );		//将舵机接口电平置高
		}

		if(num <= (Angle_J2 * 11 + 500)/10)
		{
			GPIO_SetBits(Servo_J2_PORT, Servo_J2_PIN );		//将舵机接口电平置高
		}
		else
		{
			GPIO_ResetBits(Servo_J2_PORT, Servo_J2_PIN );		//将舵机接口电平置高
		}

		if(num <= (Angle_J3 * 11 + 500)/10)
		{
			GPIO_SetBits(Servo_J3_PORT, Servo_J3_PIN );		//将舵机接口电平置高
		}
		else
		{
			GPIO_ResetBits(Servo_J3_PORT, Servo_J3_PIN );		//将舵机接口电平置高
		}

		if(num <= (Angle_J4 * 11 + 500)/10)
		{
			GPIO_SetBits(Servo_J4_PORT, Servo_J4_PIN );		//将舵机接口电平置高
		}
		else
		{
			GPIO_ResetBits(Servo_J4_PORT, Servo_J4_PIN );		//将舵机接口电平置高
		}

		if(num <= (Angle_J5 * 11 + 500)/10)
		{
			GPIO_SetBits(Servo_J5_PORT, Servo_J5_PIN );		//将舵机接口电平置高
		}
		else
		{
			GPIO_ResetBits(Servo_J5_PORT, Servo_J5_PIN );		//将舵机接口电平置高
		}

		if(num <= (Angle_J6 * 11 + 500)/10)
		{
			GPIO_SetBits(Servo_J6_PORT, Servo_J6_PIN );		//将舵机接口电平置高
		}
		else
		{
			GPIO_ResetBits(Servo_J6_PORT, Servo_J6_PIN );		//将舵机接口电平置高
		}

		

		if(num == 2000) //2000*10=20ms  20ms一个周期
		{
			num = 0;
		}
		
	}
}
