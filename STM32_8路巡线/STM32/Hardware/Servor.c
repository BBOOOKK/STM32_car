#include "Servor.h"
#include "delay.h"

/*我这里采用通用计时器4*/
static void Servo_GPIO_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*开启GPIO时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	/*配置GPIO*/
	GPIO_InitStructure.GPIO_Pin			= GPIO_Pin_7;//TIM4_CH2 
	GPIO_InitStructure.GPIO_Mode		= GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_InitStructure.GPIO_Speed		= GPIO_Speed_50MHz;
	/*初始化*/
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}
static void Servo_TIM4_Config()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	/*开启定时器时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	/*
	TIM4时基单元配置
	重要配置：TIM_Prescaler（预分频值）TIM_Period（定时周期）
	将TIM_Period设置成999，则计数器会数1000个（TIM_Period+1）
	节拍为一个定时器的周期。这个和后面需要配置的TIM_Pulse共同
	控制着定时器输出波形的占空比。
	TIM_Prescaler用来指定TIM时钟的分频值。也就是说它是进一步来
	分频TIM clock的。	简单来说也就是定时器每一次数数的时间间隔是多少。
	*/
	
	/*配置TIM4，一般的驱动PWM信号都是周期20毫秒，频率为50HZ。所以我们设定周期为20ms*/
	
											/*时基结构体成员配置*/
	//自动重装载值寄存器的值
	TIM_TimeBaseInitStructure.TIM_Period					= 5000-1;
	//时钟预分频数
	TIM_TimeBaseInitStructure.TIM_Prescaler				=	288-1;
	//时钟分频因子
	TIM_TimeBaseInitStructure.TIM_ClockDivision		= TIM_CKD_DIV1;
	//计数器计数模式，设置向上计数，
	TIM_TimeBaseInitStructure.TIM_CounterMode			= TIM_CounterMode_Up;
	/*初始化结构体*/
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	
	
									/*定时器输出比较结构体成员初始化*/
	TIM_OCInitStructure.TIM_OCMode				= TIM_OCMode_PWM1;
	// 输出使能
	TIM_OCInitStructure.TIM_OutputState		= TIM_OutputState_Enable;
	//TIM_OCInitStructure.TIM_OutputNState	= TIM_OutputNState_Enable;
	// 设置占空比大小,主要取主函数里设置占空比这边先设置成0
	TIM_OCInitStructure.TIM_Pulse					= 0;
	// 输出通道电平极性配置
	TIM_OCInitStructure.TIM_OCPolarity		= TIM_OCPolarity_High;
	//TIM_OCInitStructure.TIM_OCNPolarity		=	TIM_OCNPolarity_High;
	// 输出通道空闲电平极性配置
	TIM_OCInitStructure.TIM_OCIdleState		= TIM_OCIdleState_Set;
	//TIM_OCInitStructure.TIM_OCNIdleState  = TIM_OCNIdleState_Reset;
	/*初始化结构体*/
	TIM_OC2Init(TIM4,&TIM_OCInitStructure);
	
	//使能TIMx在CCR2上的预装载寄存器
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);
	
	// 使能计数器
	TIM_Cmd(TIM4, ENABLE);	
	// 主输出使能，当使用的是通用定时器时，这句不需要
	TIM_CtrlPWMOutputs(TIM4, ENABLE);

}
//舵机初始化函数
void Servo_Init(void)
{
	Servo_GPIO_Config();
	Servo_TIM4_Config();
	TIM_SetCompare2(TIM4, servo_M);
}
//舵机控制函数
void Servo_control(int dir)
{
	dir = dir>servo_Limit ? servo_Limit:dir;  //对舵机转向进行限幅，防止过度转向烧毁电机
	dir = dir<-servo_Limit ? -servo_Limit:dir;
	TIM_SetCompare2(TIM4, servo_M+dir);       //对舵机输出对应的pwm，即中值加方向
}


