/**
* @par Copyright (C): 2010-2019, Shenzhen Yahboom Tech
* @file         main.c	
* @author       liusen
* @version      V1.0
* @date         2017.07.17
* @brief        Ö÷º¯Êý
* @details      
* @par History  
*                 
* version:		liusen_20170717
*/
#include "stm32f10x.h"
#include "app_motor.h"
#include "app_linewalking.h"
#include "bsp.h"
#include "sys.h"



int main(void)
{	
	bsp_init();

	while (1)
	{
		app_LineWalking();		
	}
 								    
}
