#include "AllHeader.h"
#include "pic.h"//图片

char buf[]="This is test lcd!";

int main(void)
{	
	//硬件初始化
	BSP_init();
	
//	memset(buf,0,sizeof(buf));
//	delay_ms(500);//延迟
	
	while(1)
	{
//		EleDataDeal(); //电磁传感器
//		deal_data_ccd();//CCD 传感器 - 暂时不利用定时器
		if(Key1_State(0))
				LED =!LED;
#if IMU_SWITCH
			imu_test();		
#endif
	

	  LineWalking();		//四路巡线模式
		
	}
}


