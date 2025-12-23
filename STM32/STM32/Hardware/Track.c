#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Track.h"
uint8_t TrackN;
float Err = 0;
void Track_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIOStructure;

	GPIOStructure.GPIO_Mode = GPIO_Mode_IPD;     
	GPIOStructure.GPIO_Pin = Track_L|Track_l|Track_M|Track_r|Track_R;
	GPIOStructure.GPIO_Speed = GPIO_Speed_50MHz;   //50MHz是端口的输出速度，对按键没多大影响
	GPIO_Init(GPIOA,&GPIOStructure);
}

void Read_Track_DATA(uint8_t* arr)
{
	uint8_t strackarr[5] = {0};
	strackarr[4]=GPIO_ReadInputDataBit(GPIOA,Track_L);
	strackarr[3]=GPIO_ReadInputDataBit(GPIOA,Track_l);
	strackarr[2]=GPIO_ReadInputDataBit(GPIOA,Track_M);
	strackarr[1]=GPIO_ReadInputDataBit(GPIOA,Track_r);
	strackarr[0]=GPIO_ReadInputDataBit(GPIOA,Track_R);
	TrackN = +strackarr[4]*4+strackarr[3]*8+strackarr[2]*16+strackarr[1]*32
		+strackarr[0]*64+strackarr[0]*128;
	*arr = TrackN;
}

float Track_Err(uint16_t car_state)
{
	
	switch(TrackN)
	{
		
		case 0x08:	//000 1 000
			Err = 0;
			break;
		case 0x10:	//001 0 000
			Err = 2;
			break;
		case 0x20:	//010 0 000
			Err = 4;
			break;
		case 0x40:	//100 0 000
			Err = 6;
			break;
		case 0x04:	//000 0 100
			Err = -2;
			break;
		case 0x02:	//000 0 010
			Err = -4;
			break;
		case 0x01:	//000 0 001
			Err = -6;
			break;

		case 0x18:	//001 1 000
			Err = 4;
			break;
		case 0x30:	//011 0 000
			Err = 6;
			break;
		case 0xa0:	//110 0 000
			Err = 8;
			break;
		case 0x0c:	//000 1 100
			Err = -4;
			break;
		case 0x06:	//000 0 110
			Err = -6;
			break;
		case 0x03:	//000 0 011
			Err = -8;
			break;		
	}
	if(car_state == 0x1012 || car_state == 0x1013)
	{
		if(Err <= 0)
		Err = 0;
	}
	else if(car_state == 0x1022 || car_state == 0x1023)
	{
		if(Err >= 0)
		Err = 0;
	}
//	if(car_state == 0x1013)
//	{
//		Err = Err+5.0;
//	}
//	else if(car_state == 0x1023)
//	{
//		Err = Err - 5.0;
//	}
	return Err;
}
