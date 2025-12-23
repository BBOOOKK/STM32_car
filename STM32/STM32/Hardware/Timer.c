#include "stm32f10x.h"                  // Device header

void TIM4_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	TIM_InternalClockConfig(TIM4);    //配置内部时钟，上电默认为内部时钟，这句代码可不要
	
	TIM_TimeBaseInitTypeDef TIM_TimerStructure;
	//以20ms时间产生中断
	TIM_TimerStructure.TIM_ClockDivision = TIM_CKD_DIV1;        //指定时钟分频，TIM_CKD_DIV1为1分频
	TIM_TimerStructure.TIM_CounterMode = TIM_CounterMode_Up;    //配置计数器模式
	TIM_TimerStructure.TIM_Period = 200 - 1;					//ARR自动重装器的事件周期
	TIM_TimerStructure.TIM_Prescaler = 7200 - 1;                //预分频器的值     CNT计数器和预分频器PSC有一个值的偏差
	TIM_TimerStructure.TIM_RepetitionCounter = 0;               //配置高级计时器时用
	

	TIM_TimeBaseInit(TIM4,&TIM_TimerStructure);                 //初始化时基单元

	TIM_ClearFlag(TIM4,TIM_FLAG_Update);						//上一个函数会在初始化时进入一次中断
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);                    //开启更新中断到NVIC的通路
	
	NVIC_InitTypeDef   NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel =TIM4_IRQn ;             //指定要开启的通道
	NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4,ENABLE);                                       //开启定时器
}

void MPU_EXT_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource12);  //配置AFIO数据选择器
	
	//配置EXIT外部中断
	EXTI_InitTypeDef  EXIT_InitStructure;
	EXIT_InitStructure.EXTI_Line =EXTI_Line12 ;     		 //选择中断线
	EXIT_InitStructure.EXTI_LineCmd =ENABLE ;             	 //是否开启中断
	EXIT_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;   	 //选择模式  中断/事件
	EXIT_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //选择触发模式
	
	EXTI_Init(&EXIT_InitStructure);
	
	//整个工程只能设置一次优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);               //设置中断优先级 pre-emption priority  抢占 and subpriority 响应
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;               //选择中断使能/失能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;     //抢占优先级和响应优先级的配置，在多个中断中起作用
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
}




void Timer_Init(void)
{
	TIM4_Init();
}



