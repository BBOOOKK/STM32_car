#include "servo.h"
#include "delay.h"

/**
* Function       Servo_GPIO_Init    
* @brief         需要用到的舵机初始化接口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/

/*使用定时器1驱动*/
int Angle_J1 = 0;

void Servo_GPIO_Init(void)
{		
   	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef USE_SERVO_J1

	/*开启外设时钟*/
//	RCC_APB2PeriphClockCmd(Servo_J1_RCC, ENABLE); 
	/*选择要控制的引脚*/															   
		GPIO_InitStructure.GPIO_Pin = Servo_J1_PIN;	
	/*设置引脚模式为通用推挽输出*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	/*设置引脚速率为50MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化PORT*/
//		GPIO_Init(Servo_J1_PORT, &GPIO_InitStructure);		  		  
#endif
	
}

/**
* Function       Servo_J1  
* @brief         舵机1控制函数
* @param[in]     v_iAngle 角度：0~180°
* @param[out]    void
* @retval        void
* @par History   无
*/
void Servo_J1(int v_iAngle)/*定义一个脉冲函数，用来模拟方式产生PWM值*/
{
//	int pulsewidth;    						//定义脉宽变量

//	pulsewidth = (v_iAngle * 11) + 500;			//将角度转化为500-2480 的脉宽值

//	GPIO_SetBits(Servo_J1_PORT, Servo_J1_PIN );		//将舵机接口电平置高
//	delay_us(pulsewidth);					//延时脉宽值的微秒数

	//GPIO_ResetBits(Servo_J1_PORT, Servo_J1_PIN );	//将舵机接口电平置低
//	delay_ms(20 - pulsewidth/1000);			//延时周期内剩余时间
}

/**
* Function       front_detection  
* @brief         云台舵机向前
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void front_detection()
{
	int i = 0;
  	//此处循环次数减少，为了增加小车遇到障碍物的反应速度
  	for(i=0; i <= 15; i++) 						//产生PWM个数，等效延时以保证能转到响应角度
  	{
    	Servo_J1(90);						//模拟产生PWM
  	}
}

/**
* Function       left_detection 
* @brief         云台舵机向左
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void left_detection()
{
	int i = 0;
	for(i = 0; i <= 15; i++) 						//产生PWM个数，等效延时以保证能转到响应角度
	{
		Servo_J1(175);					//模拟产生PWM
	}
}

/**
* Function       right_detection
* @brief         云台舵机向右
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void right_detection()
{
	int i = 0;
	for(i = 0; i <= 15; i++) 						//产生PWM个数，等效延时以保证能转到响应角度
	{
		Servo_J1(5);						//模拟产生PWM
	}
}

/**
* Function       TIM1_Int_Init  
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
	
	//定时器TIM1初始化
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
* @brief         定时器1中断服务程序: 主要控制6路舵机运行
* @param[in]     arr：自动重装值。psc：时钟预分频数
* @param[out]    void
* @retval        void
* @par History   这里时钟选择为APB1的2倍，而APB1为36M
*/
int num = 0;


void TIM1_UP_IRQHandler(void)   //TIM1中断
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)  //检查TIM1更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx更新中断标志 
		num++;
		
		if(num <= (Angle_J1 * 11 + 500)/10)
		{
			GPIO_SetBits(Servo_J1_PORT, Servo_J1_PIN );		//将舵机接口电平置高
		}
		else
		{
			GPIO_ResetBits(Servo_J1_PORT, Servo_J1_PIN );		//将舵机接口电平置高
		}

		if(num == 2000)
		{
			num = 0;
		}
		
	}
}

