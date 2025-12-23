#include "app_motion.h"


// 编码器10毫秒前后数据
int g_Encoder_All_Now[MAX_MOTOR] = {0};
int g_Encoder_All_Last[MAX_MOTOR] = {0};

int g_Encoder_All_Offset[MAX_MOTOR] = {0};

uint8_t g_start_ctrl = 0;

car_data_t car_data;
motor_data_t motor_data;

uint8_t g_yaw_adjust = 0;
car_type_t g_car_type = CAR_FOURWHEEL; //CAR_MECANUM;



static float Motion_Get_Circle_Pulse(void)
{
    float temp = 0;
    switch (g_car_type)
    {
    case CAR_MECANUM:
        temp = ENCODER_CIRCLE_330;
        break;
    case CAR_MECANUM_MAX:
        temp = ENCODER_CIRCLE_205;
        break;
    case CAR_FOURWHEEL:
        temp = ENCODER_CIRCLE_330;
        break;
    case CAR_ACKERMAN:
        temp = ENCODER_CIRCLE_550;
        break;
    default:
        temp = ENCODER_CIRCLE_330;
        break;
    }
    return temp;
}


#if ENABLE_REAL_WHEEL
// 实际轮子转的圈数，单位：XX转/分钟
int real_circle[MAX_MOTOR] = {0};

// 实际轮子的速度，单位：m/s
float real_circle_speed[MAX_MOTOR] = {0};

// 实际小车角速度
float real_motion_angular = 0.0;

void* Motion_Real_Circle_Speed(uint8_t index)
{
    if (index == 1) return (int*) real_circle;
    if (index == 2) return (float*) real_circle_speed;
    return NULL;
}
#endif


// 仅用于添加到调试中显示数据。
void* Motion_Get_Data(uint8_t index)
{
    if (index == 1) return (int*)g_Encoder_All_Now;
    if (index == 2) return (int*)g_Encoder_All_Last;
    if (index == 3) return (int*)g_Encoder_All_Offset;
    return NULL;
}

// 获取电机速度
void Motion_Get_Motor_Speed(float* speed)
{
    for (int i = 0; i < 4; i++)
    {
        speed[i] = motor_data.speed_mm_s[i];
        
    }
}


// 设置偏航角状态，如果使能则刷新target目标角度。
void Motion_Set_Yaw_Adjust(uint8_t adjust)
{
    if (adjust == 0)
    {
        g_yaw_adjust = 0;
    }
    else
    {
        g_yaw_adjust = 1;
    }
    if (g_yaw_adjust)
    {
        #if ENABLE_INV_MEMS
        PID_Yaw_Reset(ICM20948_Get_Yaw_Now());
        #elif ENABLE_MPU9250
        PID_Yaw_Reset(MPU_Get_Yaw_Now());
        #endif
    }
}

// 返回偏航角调节状态。
uint8_t Motion_Get_Yaw_Adjust(void)
{
    return g_yaw_adjust;
}


// 控制小车运动，Motor_X=[-3600, 3600]，超过范围则无效。
void Motion_Set_Pwm(int16_t Motor_1, int16_t Motor_2, int16_t Motor_3, int16_t Motor_4)
{
    if (Motor_1 >= -MOTOR_MAX_PULSE && Motor_1 <= MOTOR_MAX_PULSE)
    {
        Motor_Set_Pwm(MOTOR_ID_M1, Motor_1);
    }
    if (Motor_2 >= -MOTOR_MAX_PULSE && Motor_2 <= MOTOR_MAX_PULSE)
    {
        Motor_Set_Pwm(MOTOR_ID_M2, Motor_2);
    }
    if (Motor_3 >= -MOTOR_MAX_PULSE && Motor_3 <= MOTOR_MAX_PULSE)
    {
        Motor_Set_Pwm(MOTOR_ID_M3, Motor_3);
    }
    if (Motor_4 >= -MOTOR_MAX_PULSE && Motor_4 <= MOTOR_MAX_PULSE)
    {
        Motor_Set_Pwm(MOTOR_ID_M4, Motor_4);
    }
}

// 小车停止
void Motion_Stop(uint8_t brake)
{
    Motion_Set_Speed(0, 0, 0, 0);
//		PID_Clear_Motor(MAX_MOTOR);
    g_start_ctrl = 0;
    g_yaw_adjust = 0;
    Motor_Stop(brake);
}


// speed_mX=[-1000, 1000], 单位为：mm/s
void Motion_Set_Speed(int16_t speed_m1, int16_t speed_m2, int16_t speed_m3, int16_t speed_m4)
{
    g_start_ctrl = 1;
    motor_data.speed_set[0] = speed_m1;
    motor_data.speed_set[1] = speed_m2;
    motor_data.speed_set[2] = speed_m3;
    motor_data.speed_set[3] = speed_m4;
    for (uint8_t i = 0; i < MAX_MOTOR; i++)
    {
        PID_Set_Motor_Target(i, motor_data.speed_set[i]*1.0);
    }
}


// 从编码器读取当前各轮子速度，单位mm/s
void Motion_Get_Speed(car_data_t* car)
{
    int i = 0;
    float speed_mm[MAX_MOTOR] = {0};
    float circle_mm = Motion_Get_Circle_MM();
    float circle_pulse = Motion_Get_Circle_Pulse();
    float robot_APB = Motion_Get_APB();

    Motion_Get_Encoder();

    // 计算轮子速度，单位mm/s。
    for (i = 0; i < 4; i++)
    {
        speed_mm[i] = (g_Encoder_All_Offset[i]) * 100 * circle_mm / circle_pulse;
    }
		
		car->Vx = (speed_mm[0] + speed_mm[1] + speed_mm[2] + speed_mm[3]) / 4;
    car->Vy = -(speed_mm[0] - speed_mm[1] - speed_mm[2] + speed_mm[3]) / 4;
    car->Vz = -(speed_mm[0] + speed_mm[1] - speed_mm[2] - speed_mm[3]) / 4.0f / robot_APB * 1000;

    if (g_start_ctrl)
    {
        for (i = 0; i < MAX_MOTOR; i++)
        {
            motor_data.speed_mm_s[i] = speed_mm[i];
        }
        
        #if ENABLE_YAW_ADJUST
        if (g_yaw_adjust)
        {
            #if ENABLE_INV_MEMS
            Mecanum_Yaw_Calc(ICM20948_Get_Yaw_Now());
            #elif ENABLE_MPU9250
            Mecanum_Yaw_Calc(MPU_Get_Yaw_Now());
            #endif
        }
        #endif
        PID_Calc_Motor(&motor_data);

        #if PID_ASSISTANT_EN
        if (start_tool())
        {
            int32_t speed_send = car->Vx;
            // int32_t speed_send = (int32_t)speed_m1;
            set_computer_value(SEND_FACT_CMD, CURVES_CH1, &speed_send, 1);
        }
        #endif
    }
}

// 返回当前小车轮子轴间距和的一半
float Motion_Get_APB(void)
{
//    if (g_car_type == CAR_MECANUM) return MECANUM_APB;
//    if (g_car_type == CAR_MECANUM_MAX) return MECANUM_MAX_APB;
//    if (g_car_type == CAR_MECANUM_MINI) return MECANUM_MINI_APB;
//    if (g_car_type == CAR_ACKERMAN) return AKM_WIDTH;
//    if (g_car_type == CAR_FOURWHEEL) return FOURWHEEL_APB;
    return MECANUM_APB;
}

// 返回当前小车轮子转一圈的多少毫米
float Motion_Get_Circle_MM(void)
{
//    if (g_car_type == CAR_MECANUM) return MECANUM_CIRCLE_MM;
//    if (g_car_type == CAR_MECANUM_MAX) return MECANUM_MAX_CIRCLE_MM;
//    if (g_car_type == CAR_MECANUM_MINI) return MECANUM_MINI_CIRCLE_MM;
//    if (g_car_type == CAR_ACKERMAN) return AKM_CIRCLE_MM;
//    if (g_car_type == CAR_FOURWHEEL) return FOURWHEEL_CIRCLE_MM;
    return MECANUM_CIRCLE_MM;
}

// 设置当前控制的小车类型。
void Motion_Set_Car_Type(car_type_t car_type)
{
    if (car_type >= CAR_TYPE_MAX) return;
    if (g_car_type == car_type) return;
    g_car_type = car_type;
    // Flash_Set_CarType(g_car_type);
}

// 返回当前控制的小车类型。
uint8_t Motion_Get_Car_Type(void)
{
    return (uint8_t)g_car_type;
}

// 获取编码器数据，并计算偏差脉冲数
void Motion_Get_Encoder(void)
{
    Encoder_Get_ALL(g_Encoder_All_Now);

    for(uint8_t i = 0; i < MAX_MOTOR; i++)
    {
        // 记录两次测试时间差的脉冲数
        g_Encoder_All_Offset[i] = g_Encoder_All_Now[i] - g_Encoder_All_Last[i];
	    // 记录上次编码器数据
	    g_Encoder_All_Last[i] = g_Encoder_All_Now[i];
    
    #if ENABLE_REAL_WHEEL
        // 计算每分钟转多少圈，10毫秒测到的脉冲数*100变秒*60变分钟/每一圈的脉冲数
        real_circle[i] = g_Encoder_All_Offset[i] * 60 * 100 / Motion_Get_Circle_Pulse();
        // 计算轮子速度，单位m/s。每分钟转的圈数*转一圈运动的距离/60得到每秒的米数
        real_circle_speed[i] = real_circle[i] * (Motion_Get_Circle_MM() / 1000.0) / 60.0;
    #endif
    }
}

// 控制小车运动
void Motion_Ctrl(int16_t V_x, int16_t V_y, int16_t V_z, uint8_t adjust)
{
	 Mecanum_Ctrl(V_x, V_y, V_z, adjust);
}


// 控制小车的运动状态
void Motion_Ctrl_State(uint8_t state, uint16_t speed, uint8_t adjust)
{
    uint16_t input_speed = speed * 10;
	
		Mecanum_State(state, input_speed, adjust);
}


// 运动控制句柄，每10ms调用一次，主要处理速度相关的数据
void Motion_Handle(void)
{
    Motion_Get_Speed(&car_data);

    if (g_start_ctrl)
    {
        Motion_Set_Pwm(motor_data.speed_pwm[0], motor_data.speed_pwm[1], motor_data.speed_pwm[2], motor_data.speed_pwm[3]);
    }
}

