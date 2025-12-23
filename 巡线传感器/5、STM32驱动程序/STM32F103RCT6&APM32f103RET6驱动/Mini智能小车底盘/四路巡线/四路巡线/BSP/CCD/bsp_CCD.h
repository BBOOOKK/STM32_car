#ifndef __BSP_CCD_H
#define __BSP_CCD_H	


#include "AllHeader.h"


#define TSL_SI    PAout(4)   //SI  
#define TSL_CLK   PAout(3)   //CLK 

u16 Get_Adc_CCD(u8 ch);
void Dly_us(void);
void RD_TSL(void); 
void CCD(void);
void ccd_Init(void);
void deal_data_ccd(void);

void  Find_CCD_Zhongzhi(void);
#endif 


