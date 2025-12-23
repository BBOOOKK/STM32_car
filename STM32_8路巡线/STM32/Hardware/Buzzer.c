#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //推挽输出高低电平都有驱动能力，开漏输出低电平驱动
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_12);    //
//	GPIO_ResetBits(GPIOA,GPIO_Pin_12);
}

void Buzzer_Out(void)
{
	uint16_t t=60000;
	GPIO_SetBits(GPIOA,GPIO_Pin_12);
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	t=60000;
	while(t--);
	GPIO_ResetBits(GPIOA,GPIO_Pin_12); 
}
