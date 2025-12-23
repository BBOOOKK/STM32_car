/**
* @par Copyright (C): 2010-2019, Shenzhen Yahboom Tech
* @file         bsp_linewalking.c	
* @author       liusen
* @version      V1.0
* @date         2017.07.17
* @brief        巡线驱动源文件
* @details      
* @par History  见如下说明
*                 
* version:		liusen_20170717
*/

#include "bsp_linewalking.h"
#include "stm32f10x.h"


/**
* Function       bsp_GetLineWalking
* @author        liusen
* @date          2017.07.20    
* @brief         获取巡线状态
* @param[in]     int *p_iL1, int *p_iL2, int *p_iR1, int *p_iR2  四路巡线位指针
* @param[out]    void
* @retval        void
* @par History   无
*/

void bsp_GetLineWalking(int *p_iL1, int *p_iL2, int *p_iR1, int *p_iR2)
{
	*p_iL1 = GPIO_ReadInputDataBit(LineWalk_L1_PORT, LineWalk_L1_PIN);
	*p_iL2 = GPIO_ReadInputDataBit(LineWalk_L2_PORT, LineWalk_L2_PIN);
	*p_iR1 = GPIO_ReadInputDataBit(LineWalk_R1_PORT, LineWalk_R1_PIN);
	*p_iR2 = GPIO_ReadInputDataBit(LineWalk_R2_PORT, LineWalk_R2_PIN);		
}

