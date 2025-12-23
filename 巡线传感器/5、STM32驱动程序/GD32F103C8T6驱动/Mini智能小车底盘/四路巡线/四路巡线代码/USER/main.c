#include "stm32f10x.h"
#include "delay.h"
#include "bsp.h"
#include "iravoid.h"
#include "servo.h"
#include "ps2_control.h"
#include "three_linewalking.h"
#include "linewalking.h"


int main(void)
 {	
	 /*外设初始化*/
	bsp_init();
	 	 
	 
  while(1)
	{
	
	  LineWalking();		//四路巡线模式
	
	}
 }

