#include "stm32f10x.h"
#include "delay.h"
#include "bsp.h"
#include "iravoid.h"
#include "servo.h"
#include "ps2_control.h"
#include "three_linewalking.h"


int main(void)
 {	
	 /*外设初始化*/
	bsp_init();
	 
	 
  while(1)
	{
		Forward(6000);
		delay_ms(1000);
		Backward(6000);
		delay_ms(1000);
		SpinLeft(7000);
		delay_ms(1000);
		SpinRight(7000);
		delay_ms(1000);
		Stop();
		delay_ms(1000);
	}
 }

