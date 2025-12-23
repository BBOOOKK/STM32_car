#include "stm32f10x.h"                  // Device header
#include "Servor.h"
#include "stdio.h"
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Motor.h"	//电机驱动与PWM输出
#include "Encoder.h"	//编码器测速
#include "Timer.h"	//定时器
#include "PID.h"	//PID参数计算
#include "Serial.h"	//串口配置
#include "Track.h"	//感为灰度数据获取
#include "mpu6050.h"
#include "Key.h"
#include "Buzzer.h"

int dir = 0; //方向变量，通过变量控制方向，0为正中，-为左，+为右
enum status
{
	L7,
	L6,
	L5,
	L4,
	L3,
	L2,
	L1,
	M,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
}dir_status,last_dir_status;  //本帧状态和上一帧状态

void Get_dir(void);  //声明获取方向的函数
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);             //配置优先级长度2+2
	Delay_Init();			//延时初始化
	Buzzer_Init();		//蜂鸣器初始化
	OLED_Init();			//0.96OLED_IIC 初始化
	Servo_Init();     //舵机初始化
	Track_Init();			//灰度初始化
	Key_Init();				//按键初始化
	Motor_Init();			//PWM电机初始化
	dir_status = M;   //状态初始化，为正中
	last_dir_status = M;
//	Timer_Init();	
//	MPU_Init();				//初始化MPU6050
//	mpu_dmp_init();
//	Encoder_Init(&encoder);	//编码器初始化
	while(1)
	{
		Get_dir();//获取当前方向
//		dir=0;
		
		
		/***************舵机四轮控制部分****************/
		Servo_control(dir);  //控制当前舵机，看函数具体内容
		if(dir>=0)
	  Move_control(1100+0.7*dir,1125-0.2*dir);  //控制电机的函数，左右各一路，左右相同为直行，通过加减dir实现一定差速功能
		else
		Move_control(1100+0.2*dir,1125-0.7*dir);  //控制电机的函数，左右各一路，左右相同为直行，通过加减dir实现一定差速功能

		
		
		/***************三轮差速控制部分****************/
//		if(dir>=0)
//	  Move_control(1100+0.8*dir,1125-0.5*dir);  
//		else
//		Move_control(1100+0.5*dir,1125-0.8*dir);  
//		
//		if(dir>=0)
//	  Move_control(1120+1*dir,1145-0.7*dir);  
//		else
//		Move_control(1120+0.7*dir,1145-1*dir);  
		
		if(dir>0)
	  Move_control(1150+1.5*dir,825-1.16*dir);  
		else
		Move_control(1150+1.16*dir,825-1.5*dir); 
		
		
		//对灰度传感器数据进行显示,正常近距离为1，对于黑胶带蓝布路径，蓝布为1，黑胶带为0
		//从左到右的顺序
				OLED_ShowHexNum(1,1,GPIO_ReadInputDataBit(GPIOA,Track_S0),1);
		OLED_ShowHexNum(1,3,GPIO_ReadInputDataBit(GPIOA,Track_S1),1);
		OLED_ShowHexNum(1,5,GPIO_ReadInputDataBit(GPIOA,Track_S2),1);
		OLED_ShowHexNum(1,7,GPIO_ReadInputDataBit(GPIOA,Track_S3),1);
		OLED_ShowHexNum(1,9,GPIO_ReadInputDataBit(GPIOA,Track_S4),1);
		OLED_ShowHexNum(1,11,GPIO_ReadInputDataBit(GPIOA,Track_S5),1);
		OLED_ShowHexNum(1,13,GPIO_ReadInputDataBit(GPIOA,Track_S6),1);
		OLED_ShowHexNum(1,15,GPIO_ReadInputDataBit(GPIOA,Track_S7),1);
		//显示当前输出方向
		OLED_ShowSignedNum(2,6,dir,3);
	
//		Key_Scan();
//		Delay_ms(100);
	}	
}
//void TIM4_IRQHandler(void)
//{
//	if(TIM_GetITStatus(TIM4,TIM_IT_Update) == SET)       //检查要检查的中断是否发生
//	{
//		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);       //清除TIM的中断挂起位
//	}
//}

/***************MPU6050外部中断****************/
//---50HZ采样频率---
void EXTI15_10_IRQHandler(void)
{

}


void Get_dir(void)
{
	static int8_t dir_turn = 0;  /* +1: tending right, -1: tending left */
	static int time = 0;

	/* Map L7..R7 to steering output (-200..+200) */
	static const int16_t dir_map[15] = {
		-200, -180, -160, -130, -110, -80, -50, 0,
		 50,   80,  110,  130,  160,  180, 200
	};

	uint8_t bits = 0;
	uint8_t cnt = 0;
	uint8_t i;
	float pos;
	int pos_i;
	int next;
	int idx;

	Read_Track_DATA(&bits);

	//计算当前压线传感器数量
	for (i = 0; i < 8; i++) {
		cnt += (uint8_t)((bits >> i) & 0x01);
	}

	last_dir_status = dir_status;

	if (cnt != 0) {
		pos = Track_Err(0);  /* -7..+7 */
		if (pos >= 0) pos_i = (int)(pos + 0.5f);
		else          pos_i = (int)(pos - 0.5f);

		if (pos_i > 7) pos_i = 7;
		if (pos_i < -7) pos_i = -7;

		dir_status = (enum status)((int)M + pos_i);
	} else {
		/* Line lost: keep turning in the last tendency direction */
		if (dir_turn == 0) {
			dir_turn = (dir >= 0) ? 1 : -1;
			if (dir == 0) dir_turn = 1;
		}
		next = (int)dir_status + dir_turn;
		if (next < (int)L7) next = (int)L7;
		if (next > (int)R7) next = (int)R7;
		dir_status = (enum status)next;
	}

	if (dir_status != last_dir_status) {
		time = 0;

		if (dir_status > last_dir_status) dir_turn = 1;
		else if (dir_status < last_dir_status) dir_turn = -1;
		else dir_turn = 0;

		idx = (int)dir_status - (int)L7;
		if (idx < 0) idx = 0;
		if (idx > 14) idx = 14;
		dir = dir_map[idx];
	} else {
		/* Keep your original hook for dynamic correction if you want */
		time++;
		(void)time;
	}

	dir = (dir > 200) ? 200 : dir;
	dir = (dir < -200) ? -200 : dir;

	/* ----- OLED debug ----- */
	if (last_dir_status < M) {
		OLED_ShowChar(3, 3, 'L');
		OLED_ShowHexNum(3, 4, (uint32_t)(M - last_dir_status), 1);
	} else if (last_dir_status > M) {
		OLED_ShowChar(3, 3, 'R');
		OLED_ShowHexNum(3, 4, (uint32_t)(last_dir_status - M), 1);
	} else {
		OLED_ShowChar(3, 3, 'M');
		OLED_ShowChar(3, 4, ' ');
	}

	if (dir_turn == 1) {
		OLED_ShowString(3, 8, "->");
	} else if (dir_turn == -1) {
		OLED_ShowString(3, 8, "<-");
	} else {
		OLED_ShowString(3, 8, "--");
	}

	if (dir_status < M) {
		OLED_ShowChar(3, 13, 'L');
		OLED_ShowHexNum(3, 14, (uint32_t)(M - dir_status), 1);
	} else if (dir_status > M) {
		OLED_ShowChar(3, 13, 'R');
		OLED_ShowHexNum(3, 14, (uint32_t)(dir_status - M), 1);
	} else {
		OLED_ShowChar(3, 13, 'M');
		OLED_ShowChar(3, 14, ' ');
	}
}


