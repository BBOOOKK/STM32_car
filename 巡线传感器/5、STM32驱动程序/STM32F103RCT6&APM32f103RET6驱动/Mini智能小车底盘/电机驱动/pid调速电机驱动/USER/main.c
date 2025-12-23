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
		
		Encoder_Update_Count();
		Motion_Handle();
		//电机控制
		Mecanum_State(MOTION_RUN,950,0);//用了PID,前进
		delay_ms(1000);
		Mecanum_State(MOTION_BACK,950,0);//用了PID,后退
		delay_ms(1000);
		Mecanum_State(MOTION_SPIN_LEFT,700,0);//用了PID,左转
		delay_ms(1000);
		Mecanum_State(MOTION_SPIN_RIGHT,700,0);//用了PID,右转
		delay_ms(1000);
		Mecanum_State(MOTION_STOP,0,0);//用了PID,停止
		delay_ms(1000);

	
	}
}


