#include "bsp.h"

void BSP_init(void)
{
	SystemInit();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	
	delay_init();
//	delay_ms(3000); //等待系统稳定，陀螺仪正完全开机
	
	init_LED_GPIO();//LED初始化
	Key_GPIO_Init();//key初始化
	
	//RGB初始化//还没焊接
	//init_RGB_GPIO();
	
	//Servo_GPIO_Init();				    /*舵机接口初始化*/	
	//TIM6_Int_Init(9, 72);				/*100Khz的计数频率，计数到10为10us  */ 
	
#if DEBUG_SWITCH
	//usart1初始化
	USART1_init(115200);
#endif

#if LCD_SWITCH
	//lcd初始化
	LCD_Init();//LCD初始化
#endif

#if MOTOR_SWITCH
		MOTOR_GPIO_Init();//电机IO初始化
		Motor_PWM_Init(MOTOR_MAX_PULSE, MOTOR_FREQ_DIVIDE);//电机pwm初始化
		PID_Param_Init();//PID
		Encoder_Init();//编码器初始化
#endif


//下面几种模块只能选择一种，因为使用的引脚相同
#if ELE_SWITCH==1 && CCD_SWITCH==0
	//电磁循迹初始化
	ele_Init();
#elif ELE_SWITCH==0 && CCD_SWITCH==1
	ccd_Init();//CCD传感器初始化
#endif

#if Ultrasonic_SWITCH==1 && IRAvoid_SWITCH==0
	//超声波避障初始化
//	Ultrasonic_GPIO_Init();				/*超声波GPIO初始化*/
//	Ultrasonic_Timer7_Init();		/*超声波TIM7初始化*/
#elif Ultrasonic_SWITCH==0 && IRAvoid_SWITCH==1
	IRAvoid_GPIO_Init();				/*红外避障传感器初始化*/
#endif

#if three_LineWalking_SWITCH==1 && LineWalking_SWITCH==0 && PS2_SWITCH==0
	three_LineWalking_GPIO_Init();/*三路巡线传感器初始化*/
#elif three_LineWalking_SWITCH==0 && LineWalking_SWITCH==1 && PS2_SWITCH==0
	LineWalking_GPIO_Init();			/*四路巡线传感器初始化*/
#elif three_LineWalking_SWITCH==0 && LineWalking_SWITCH==0 && PS2_SWITCH==1
  PS2_Init();/*手柄初始化*/
#endif


	
#if FLASH_SWITCH
	SpiFlashInit(); //w25q64初始化
	show_flash();
#endif

#if IMU_SWITCH
	//陀螺仪初始化
	icm20607_init();
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE); //清屏
#endif
	
//基本定时器初始化

	//放到最后才生效，不然还是无法正常使用
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//禁用jlink 只用SWD调试口，PA15、PB3、4做普通IO
	
	
}
