#include "stm32f10x.h"                 // STM32 标准外设库头文件
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"                     // 延时函数头文件
#include "oled.h"                      // OLED 显示相关头文件
#include "motor.h"                     // 电机控制接口头文件
#include "Track.h"                     // 循迹传感器模块头文件
#include "Key.h"                       // 按键模块头文件（用于调试模式切换）

// 定义方向状态的编码，便于电机控制
typedef enum {
    DIR_STOP = 0,      // 停止或未检测到线
    DIR_HARD_LEFT,     // 极左转弯
    DIR_LEFT,          // 左转
    DIR_STRAIGHT,      // 直行
    DIR_RIGHT,         // 右转
    DIR_HARD_RIGHT     // 极右转弯
} DirectionStatus;

// 电机速度配置参数（可根据实际硬件调整）
#define SPEED_STOP          0      // 停止速度
#define SPEED_STRAIGHT      1000   // 直行速度
#define SPEED_TURN          800    // 普通转弯速度
#define SPEED_HARD_TURN     600    // 急转弯速度
#define SPEED_MIN           400    // 最小速度（用于微调）
#define SPEED_MAX           1200   // 最大速度（注意不要超过PWM最大值）

// 调试模式标志
static uint8_t debug_mode = 0;     // 0=正常模式，1=调试模式

// 全局变量：当前方向状态和各红外传感器感应状态（1表示检测到白线，0表示检测到蓝色地面）
static DirectionStatus dir_status = DIR_STOP;
static uint8_t sw1_line = 0;
static uint8_t sw2_line = 0;
static uint8_t sw3_line = 0;
static uint8_t sw4_line = 0;

/**
  * @brief  读取四路循迹传感器并更新方向状态（dir_status）
  * @note   使用巡线模块的算法，基于四路传感器状态判断行驶方向
  *         传感器映射：SW1(PA1)=L1(最左), SW2(PA0)=L2(左中), SW3(PA2)=R1(右中), SW4(PA3)=R2(最右)
  *         使用Track.h提供的模块化函数读取传感器状态
  *         注意：现在是在蓝色地面上巡白线
  *         Track_Read_All函数返回1表示检测到白线，0表示检测到蓝色地面
  *         巡线模块的算法中，LOW=0表示检测到黑线，HIGH=1表示未检测到
  *         对于白线检测，逻辑需要反转：检测到白线时应该对应巡线模块的LOW状态
  */
void Get_dir(void) {
    // 使用Track模块的函数读取传感器状态
    Track_Read_All(&sw1_line, &sw2_line, &sw3_line, &sw4_line);
    
    // 注意：现在是在蓝色地面上巡白线
    // Track_Read_All函数返回1表示检测到白线，0表示检测到蓝色地面
    // 巡线模块的算法是为黑线检测设计的：LOW=0表示检测到黑线，HIGH=1表示未检测到
    // 对于白线检测，我们需要将逻辑反转：
    // 当检测到白线（swX_line=1）时，对应巡线模块的LOW状态（检测到线）
    // 当检测到蓝地（swX_line=0）时，对应巡线模块的HIGH状态（未检测到线）
    
    // 所以我们可以直接使用swX_line的值，因为：
    // swX_line=1（检测到白线）→ 对应巡线模块的LOW=0（检测到线）
    // swX_line=0（检测到蓝地）→ 对应巡线模块的HIGH=1（未检测到线）
    
    // 使用巡线模块的算法逻辑（直接使用swX_line，因为白线检测逻辑与黑线检测相反）
    // 左大弯：(L1或L2检测到白线)且R2检测到白线 → 左旋转
    if ((sw1_line || sw2_line) && sw4_line) {
        dir_status = DIR_HARD_LEFT;
    }
    // 右大弯：L1检测到白线且(R1或R2检测到白线) → 右旋转
    else if (sw1_line && (sw3_line || sw4_line)) {
        dir_status = DIR_HARD_RIGHT;
    }
    // 左最外侧检测：L1检测到白线 → 左旋转
    else if (sw1_line) {
        dir_status = DIR_LEFT;
    }
    // 右最外侧检测：R2检测到白线 → 右旋转
    else if (sw4_line) {
        dir_status = DIR_RIGHT;
    }
    // 中间微调左转：L2检测到白线且R1未检测到白线 → 左转
    else if (sw2_line && !sw3_line) {
        dir_status = DIR_LEFT;
    }
    // 中间微调右转：L2未检测到白线且R1检测到白线 → 右转
    else if (!sw2_line && sw3_line) {
        dir_status = DIR_RIGHT;
    }
    // 中间都在白线上：L2检测到白线且R1检测到白线 → 加速前进
    else if (sw2_line && sw3_line) {
        dir_status = DIR_STRAIGHT;
    }
    // 其他情况：停止（未检测到白线或检测到部分白线但不符合上述条件）
    else {
        dir_status = DIR_STOP;
    }
}

int main(void) {
    // 系统与外设初始化
    SystemInit();                            // 系统时钟初始化（如果启动文件未调用）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // 使能 GPIOA 时钟（传感器）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // 使能 GPIOB 时钟（OLED, 电机）
    
    // 初始化相关模块（OLED显示、舵机、电机、循迹传感器等）
    Delay_Init();               // 初始化延时功能（确保 OLED_Init 中延时可用）
    OLED_Init();                // 初始化 OLED 显示屏
    // Servo_Init();            // 初始化舵机控制（未使用舵机时可不调用）
    Motor_Init();               // 初始化电机控制（PWM输出等）
    Track_Init();               // 初始化循迹传感器模块
    Key_Init();                 // 初始化按键（用于调试模式切换）

    // 设置初始状态：停止小车并将舵机回正
    Move_control(SPEED_STOP, SPEED_STOP);    // 停止左右电机
    // Servo_control(DIR_STRAIGHT);           // 舵机居中（直行方向）

    // 显示启动信息
    OLED_Clear();
    OLED_ShowString(1, 1, "White Line Follower");
    OLED_ShowString(2, 1, "Press Key to Debug");
    OLED_ShowString(3, 1, "Mode: Normal");
    Delay_ms(1000);

    // 主循环：持续读取传感器并控制小车运动
    while (1) {
        // 检查按键，切换调试模式
        uint8_t key = Key_GetNum();
        if (key == 1) {  // 假设按键1用于切换调试模式
            debug_mode = !debug_mode;
            OLED_Clear();
            if (debug_mode) {
                OLED_ShowString(1, 1, "Debug Mode ON");
            } else {
                OLED_ShowString(1, 1, "Normal Mode ON");
            }
            Delay_ms(500);
        }

        // 获取当前传感器状态并更新方向状态
        Get_dir();

        // 根据方向状态控制左右电机速度，差速转向
        switch (dir_status) {
            case DIR_HARD_LEFT:
                // 极左转：左轮减速，右轮正常速度
                Move_control(SPEED_HARD_TURN, SPEED_STRAIGHT);
                break;
            case DIR_LEFT:
                // 左转：左轮偏慢，右轮正常速度
                Move_control(SPEED_TURN, SPEED_STRAIGHT);
                break;
            case DIR_STRAIGHT:
                // 直行：两轮速度相同
                Move_control(SPEED_STRAIGHT, SPEED_STRAIGHT);
                break;
            case DIR_RIGHT:
                // 右转：右轮偏慢，左轮正常速度
                Move_control(SPEED_STRAIGHT, SPEED_TURN);
                break;
            case DIR_HARD_RIGHT:
                // 极右转：右轮减速，左轮正常速度
                Move_control(SPEED_STRAIGHT, SPEED_HARD_TURN);
                break;
            case DIR_STOP:
            default:
                // 停止状态：停车
                Move_control(SPEED_STOP, SPEED_STOP);
                break;
        }

        // OLED显示
        OLED_Clear();
        
        if (debug_mode) {
            // 调试模式：显示详细信息
            // 第1行：传感器状态
            OLED_ShowString(1, 1, "L1:");
            OLED_ShowNum(1, 5, sw1_line, 1);
            OLED_ShowString(1, 9, "L2:");
            OLED_ShowNum(1, 13, sw2_line, 1);
            
            // 第2行：传感器状态
            OLED_ShowString(2, 1, "R1:");
            OLED_ShowNum(2, 5, sw3_line, 1);
            OLED_ShowString(2, 9, "R2:");
            OLED_ShowNum(2, 13, sw4_line, 1);
            
            // 第3行：方向状态
            OLED_ShowString(3, 1, "Dir:");
            switch (dir_status) {
                case DIR_STOP: OLED_ShowString(3, 6, "STOP   "); break;
                case DIR_HARD_LEFT: OLED_ShowString(3, 6, "HARD_L"); break;
                case DIR_LEFT: OLED_ShowString(3, 6, "LEFT  "); break;
                case DIR_STRAIGHT: OLED_ShowString(3, 6, "STRAIGHT"); break;
                case DIR_RIGHT: OLED_ShowString(3, 6, "RIGHT "); break;
                case DIR_HARD_RIGHT: OLED_ShowString(3, 6, "HARD_R"); break;
                default: OLED_ShowString(3, 6, "UNKNOWN"); break;
            }
            
            // 第4行：模式信息
            OLED_ShowString(4, 1, "Mode:Debug White");
        } else {
            // 正常模式：简洁显示
            // 第1行：传感器状态简图
            OLED_ShowString(1, 1, "Sensors:");
            OLED_ShowNum(1, 10, sw1_line, 1);
            OLED_ShowNum(1, 12, sw2_line, 1);
            OLED_ShowString(1, 14, "|");
            OLED_ShowNum(1, 15, sw3_line, 1);
            OLED_ShowNum(1, 17, sw4_line, 1);
            
            // 第2行：方向指示
            OLED_ShowString(2, 1, "Direction:");
            switch (dir_status) {
                case DIR_STOP: OLED_ShowString(2, 12, "STOP"); break;
                case DIR_HARD_LEFT: OLED_ShowString(2, 12, "<<< "); break;
                case DIR_LEFT: OLED_ShowString(2, 12, "<   "); break;
                case DIR_STRAIGHT: OLED_ShowString(2, 12, "^^^"); break;
                case DIR_RIGHT: OLED_ShowString(2, 12, "   >"); break;
                case DIR_HARD_RIGHT: OLED_ShowString(2, 12, " >>>"); break;
                default: OLED_ShowString(2, 12, "----"); break;
            }
            
            // 第3行：速度信息
            OLED_ShowString(3, 1, "Speed:");
            switch (dir_status) {
                case DIR_HARD_LEFT: OLED_ShowString(3, 8, "L:600 R:1000"); break;
                case DIR_LEFT: OLED_ShowString(3, 8, "L:800 R:1000"); break;
                case DIR_STRAIGHT: OLED_ShowString(3, 8, "L:1000 R:1000"); break;
                case DIR_RIGHT: OLED_ShowString(3, 8, "L:1000 R:800"); break;
                case DIR_HARD_RIGHT: OLED_ShowString(3, 8, "L:1000 R:600"); break;
                case DIR_STOP: OLED_ShowString(3, 8, "L:0   R:0   "); break;
                default: OLED_ShowString(3, 8, "L:--- R:---"); break;
            }
            
            // 第4行：模式信息
            OLED_ShowString(4, 1, "Mode:Normal White");
        }

        // 短暂延时 (15ms) 控制循环频率，提高响应速度
        Delay_ms(15);
    }
}
