#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Motor.h"
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIOBStructure;
	GPIOBStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIOBStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIOBStructure.GPIO_Speed = GPIO_Speed_50MHz;   //50MHz是端口的输出速度，对按键没多大影响
	GPIO_Init(GPIOB,&GPIOBStructure);
}


uint8_t  Key_GetNum(void)
{
	uint8_t Keynum=0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)!=0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)!=0)
		Delay_ms(20);
		Keynum=1;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)!=0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)!=0)
		Delay_ms(20);
		Keynum=2;
	}	
	return Keynum;
}

void Key_Scan(void)
{
	uint8_t Keynum = 0;
	Keynum = Key_GetNum();	//读取按键值
	switch(Keynum)
	{
		case 1:
			break;
		case 2:
			break;
	}
}
