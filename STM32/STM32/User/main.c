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
enum status   //方向状态的枚举，根据本次灰度传感器数据进行分析，划分为如下状态，具有顺序性
{
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
		OLED_ShowHexNum(1,2,GPIO_ReadInputDataBit(GPIOA,Track_L),1);
		OLED_ShowHexNum(1,5,GPIO_ReadInputDataBit(GPIOA,Track_l),1);
		OLED_ShowHexNum(1,8,GPIO_ReadInputDataBit(GPIOA,Track_M),1);
		OLED_ShowHexNum(1,11,GPIO_ReadInputDataBit(GPIOA,Track_r),1);
		OLED_ShowHexNum(1,14,GPIO_ReadInputDataBit(GPIOA,Track_R),1);
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
	static float dir_turn = 0; //预测状态变化趋势
	static int time = 0; //方向状态计时，用于计算该状态长短，非准确时间单位，由于while时间决定
	int Track_sum = !GPIO_ReadInputDataBit(GPIOA,Track_L)+!GPIO_ReadInputDataBit(GPIOA,Track_l)+!GPIO_ReadInputDataBit(GPIOA,Track_M)
		+ !GPIO_ReadInputDataBit(GPIOA,Track_r)+!GPIO_ReadInputDataBit(GPIOA,Track_R); //计算五路获取到黑色胶带的数量
	//根据五路获取到的数据进行分类讨论，主要分析获取一个，获取两个和未获取的情况，其他情况较为特殊不做处理，保持原状态运行
	// L5 L4 L3 L2 L1 M R1 R2 R3 R4 R5
	if( Track_sum == 1) //只有一路获取，即准确确认状态
	{
		if(!GPIO_ReadInputDataBit(GPIOA,Track_M))
		{	
			last_dir_status = dir_status;
			dir_status = M;
		}
		else if(dir_status <=M && !GPIO_ReadInputDataBit(GPIOA,Track_l))
		{
			last_dir_status = dir_status;
			dir_status = L2;
		}
		else if(dir_status >=M && !GPIO_ReadInputDataBit(GPIOA,Track_r))
		{
			last_dir_status = dir_status;
			dir_status = R2;
		}
		else if(dir_status <=M && !GPIO_ReadInputDataBit(GPIOA,Track_L))
		{
			last_dir_status = dir_status;
			dir_status = L4;
		}
		else if(dir_status >=M && !GPIO_ReadInputDataBit(GPIOA,Track_R))
		{
			last_dir_status = dir_status;
			dir_status = R4;
		}
		else
		{
			last_dir_status = dir_status;
		}
	}
		else if( Track_sum == 2 )  //有两路获取，一般在两路之间的位置
	{
		if(!GPIO_ReadInputDataBit(GPIOA,Track_l) && !GPIO_ReadInputDataBit(GPIOA,Track_M))
		{
			last_dir_status = dir_status;
			dir_status = L1;
		}
		else if(!GPIO_ReadInputDataBit(GPIOA,Track_r)&& !GPIO_ReadInputDataBit(GPIOA,Track_M))
		{
			last_dir_status = dir_status;
			dir_status = R1;
		}
		if(!GPIO_ReadInputDataBit(GPIOA,Track_l) && !GPIO_ReadInputDataBit(GPIOA,Track_L))
		{
			last_dir_status = dir_status;
			dir_status = L3;
		}
		else if(!GPIO_ReadInputDataBit(GPIOA,Track_r)&& !GPIO_ReadInputDataBit(GPIOA,Track_R))
		{
			last_dir_status = dir_status;
			dir_status = R3;
		}
		else
		{
			last_dir_status = dir_status;
		}
	}
	else if( Track_sum == 0)  //没有任何一路获取，一般为5路均已经出界，或者在两路之间的情况
	{
		if(dir_status  == L2 || dir_status  == L4 || dir_status  == R2 || dir_status  == R4)  //如果上一个状态为明显状态，直接根据上一状态加上方向变化趋势，获得该方向状态
		{
			last_dir_status = dir_status;
			dir_status = dir_status + dir_turn;
		}
		else
		{
			last_dir_status = dir_status;
		}
	}
	
	//当方向状态改变时，运行此操作
	if( dir_status != last_dir_status)
	{
		time = 0;//方向状态计时初始化
		//根据前后方向状态，对方向变化进行预测，1为向右，-1为向左
		if(dir_status > last_dir_status)
		dir_turn = 1;
		else if(dir_status < last_dir_status)
		dir_turn = -1;
		else
		dir_turn = 0;
		//根据当前方向状态，对转弯方向进行赋值,具体转向值需要具体调
		switch(dir_status)
		{
			case M:
				dir = 0;
			break;
			case L1:
				dir = -50;
			break;
			case L2:
				dir = -80;
			break;
			case L3:
				dir = -110;
			break;
			case L4:
				dir = -120;
			break;
			case L5:
				dir = -180;
			break;
			case R1:
				dir = 50;
			break;
			case R2:
				dir = 80;
			break;
			case R3:
				dir = 110;
			break;
			case R4:
				dir = 120;
			break;
			case R5:
				dir = 180;
			break;
		}
	}
	else//动态dir，测试版本,不稳定  以状态计时为种子,通过对方向趋势预判进行动态dir
	{
		time++;
		if((dir_status == L5 || dir_status == R5) && time >=40)  //车辆在拐出长时间时，增大拐向力度
		{
//		if(time%20 == 0)
//		dir += dir_turn;
		}

		
	}
	//对方向输出进行限幅
	dir = dir>200 ? 200:dir;
	dir = dir<-200 ? -200:dir;

	
	//显示上一次状态
		if(last_dir_status<M)
		{
		OLED_ShowChar(3, 3, 'L');
		OLED_ShowHexNum(3,4,M-last_dir_status,1);
		}
		else if(last_dir_status>M)
		{
		OLED_ShowChar(3, 3, 'R');
		OLED_ShowHexNum(3,4,last_dir_status-M,1);
		}
		else 
		{
		OLED_ShowChar(3, 3, 'M');
		OLED_ShowChar(3, 4, '  ');
		}
		//方向变化趋势
		if(dir_turn == 1)
		{
			OLED_ShowString(3, 8, "->");
		}
		else if(dir_turn == -1)
		{
			OLED_ShowString(3, 8, "<-");
		}
		else
		{
			OLED_ShowString(3, 8, "--");
		}
		//显示此状态
		if(dir_status<M)
		{
		OLED_ShowChar(3, 13, 'L');
		OLED_ShowHexNum(3,14,M-dir_status,1);
		}
		else if(dir_status>M)
		{
		OLED_ShowChar(3, 13, 'R');
		OLED_ShowHexNum(3,14,dir_status-M,1);
		}
		else 
		{
		OLED_ShowChar(3, 13, 'M');
		OLED_ShowChar(3, 14, ' ');
		}
		
}

