#include "AllHeader.h"
#include "pic.h"//图片

char buf[]="This is test lcd!";

int main(void)
{	
	//硬件初始化
	BSP_init();

	
	while(1)
	{

		if(Key1_State(0))
				LED =!LED;
#if IMU_SWITCH
			imu_test();		
#endif
	
		//电机控制
		Motion_Set_Pwm(1500,1500,1500,1500); //没用pid，前进
		delay_ms(1000);
		Motion_Set_Pwm(-1500,-1500,-1500,-1500); //后退
		delay_ms(1000);
		//Motion_Set_Pwm(0,0,1500,1500); //右转，左前后轮前进
	//	Motion_Set_Pwm(1500,1500,0,0); //左转，右前后轮前进
		Motion_Set_Pwm(1500,1500,-1500,-1500); //左急转弯
		delay_ms(1000);
	  Motion_Set_Pwm(-1500,-1500,1500,1500); //右急转弯，左前轮前进，右前轮后退
		delay_ms(1000);
		Motion_Set_Pwm(0,0,0,0);
		delay_ms(1000);
	//	Mecanum_State(MOTION_RUN,950,0);//用了PID
	//	Encoder_Update_Count();
	//	Motion_Handle();
		

	}
}


