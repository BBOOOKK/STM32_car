#include "ultrasonic.h"
#include "stm32f10x.h"
#include "delay.h"
#include "servo.h"
#include "sys.h"
#include "moto.h"
#include "PWM.h"
#include "usart.h"	

/*记录定时器溢出次数*/
unsigned int overcount = 0;

/**
* Function       Ultrasonic_GPIO_Init
* @brief         超声波传感器GPIO初始化接口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void Ultrasonic_GPIO_Init(void)
{

#ifdef USE_ULTRASONIC

	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*开启外设时钟*/
	RCC_APB2PeriphClockCmd(TRIG_RCC | RCC_APB2Periph_AFIO, ENABLE); 
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);  //禁能jtag
	/*TRIG触发信号*/														   
  	GPIO_InitStructure.GPIO_Pin = TRIG_PIN;	
	/*设置引脚模式为通用推挽输出*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	/*设置引脚速率为50MHz */   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化*/
  	GPIO_Init(TRIG_PORT, &GPIO_InitStructure);		 


	/*开启外设时钟*/
	RCC_APB2PeriphClockCmd(ECHO_RCC, ENABLE); 
	/*ECOH回响信号*/															   
  	GPIO_InitStructure.GPIO_Pin = ECHO_PIN;	
	/*设置引脚模式为通用推挽输出*/
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
	/*设置引脚速率为50MHz */   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化PORT*/
  	GPIO_Init(ECHO_PORT, &GPIO_InitStructure);
		
#endif

}

/**
* Function       Ultrasonic_Timer2_Init

* @brief         初始化定时器TIM2
* @param[in]     void
* @param[out]    void
* @return        距离浮点值
* @par History   无
*/

void Ultrasonic_Timer2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructer;
	NVIC_InitTypeDef NVIC_InitStructer;


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	/*定时器TIM2初始化*/
	TIM_DeInit(TIM2);
	TIM_TimeBaseInitStructer.TIM_Period = 999;//定时周期为1000
	TIM_TimeBaseInitStructer.TIM_Prescaler = 71; //分频系数72
	TIM_TimeBaseInitStructer.TIM_ClockDivision = TIM_CKD_DIV1;//不分频
	TIM_TimeBaseInitStructer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructer);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);//开启更新中断

	/*定时器中断初始化*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStructer.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructer.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructer.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructer.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructer);
	TIM_Cmd(TIM2, DISABLE);//关闭定时器使能

}

void TIM2_IRQHandler(void) //中断，当回响信号很长是，计数值溢出后重复计数，用中断来保存溢出次数
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);//清除中断标志
		overcount++;	
	}
}
/**
* Function       getUltrasonicDistance
   
* @brief         获取超声距离
* @param[in]     void
* @param[out]    void
* @return        距离浮点值
* @par History   无
*/

float GetUltrasonicDistance(void)
{
	float length = 0, sum = 0;
	u16 tim;
	unsigned int  i = 0;

	/*测5次数据计算一次平均值*/
	while(i != 5)
	{
		GPIO_SetBits(TRIG_PORT, TRIG_PIN);  //拉高信号，作为触发信号
		delay_us(20);  						//高电平信号超过10us
		GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

		/*等待回响信号*/
		while(GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == RESET);
		TIM_Cmd(TIM2,ENABLE);//回响信号到来，开启定时器计数
		
		i+=1; //每收到一次回响信号+1，收到5次就计算均值
		while(GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == SET);//回响信号消失
		TIM_Cmd(TIM2, DISABLE);//关闭定时器
		
		tim = TIM_GetCounter(TIM2);//获取计TIM2数寄存器中的计数值，一边计算回响信号时间
		
		length = (tim + overcount * 1000) / 58.0;//通过回响信号计算距离
		
		sum = length + sum;
		TIM2->CNT = 0;  //将TIM2计数寄存器的计数值清零
		overcount = 0;  //中断溢出次数清零
		delay_ms(1);
	}
	length = sum / 5;
	printf("CSB:%f",length );  	
	return length;		//距离作为函数返回值
	
}



/**
* Function       ultrasonic_servo_mode
* @brief         超声波避障
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void Ultrasonic_servo_mode(void)
{
	int Len = 0;
	int LeftDistance = 0, RightDistance = 0;

	Len = (u16)GetUltrasonicDistance();
	 

    if(Len <= 30)//当遇到障碍物时
    {

		Stop();//停下来做测距
		
		Angle_J1 = 180;		// 左边
		delay_ms(500); //等待舵机到位
		Len = GetUltrasonicDistance();			
		LeftDistance = Len;	  
	 
		Angle_J1 = 0;		// 右边
		delay_ms(500); //等待舵机到位
		Len = GetUltrasonicDistance();					
		RightDistance = Len;


		Angle_J1 = 90;		//归位
		delay_ms(500); //等待舵机到位

		if((LeftDistance < 22 ) &&( RightDistance < 22 ))//当左右两侧均有障碍物靠得比较近
		{
			SpinRight(7000);//旋转掉头
			delay_ms(600); //等待舵机到位
		}
		else if(LeftDistance >= RightDistance)//左边比右边空旷
		{      
			SpinLeft(7000);//左转
			delay_ms(400); //等待舵机到位
		}
		else//右边比左边空旷
		{
			SpinRight(7000); //右转
			delay_ms(400); //等待舵机到位
		}
    }
		
    else if(Len > 30)//当遇到障碍物时
    {
		Forward(6000); 	 //无障碍物，直行     
    }
}


