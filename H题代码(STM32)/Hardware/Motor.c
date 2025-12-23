#include "stm32f10x.h"
#include "Motor.h"

/*-------------------- 内部函数声明 --------------------*/
static void TIM1_PWM_Init(uint16_t arr, uint16_t psc);

/*-------------------- 电机初始化 --------------------*/
void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 1. 使能时钟：GPIOA / GPIOB / AFIO / TIM1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_AFIO  |
                           RCC_APB2Periph_TIM1, ENABLE);

    /* 2. 初始化 TIM1 做 PWM (PA8-CH1, PA11-CH4) */
    TIM1_PWM_Init(2000, 71);   // 频率大约 = 72MHz/(71+1)/(2000+1) ≈ 500Hz

    /* 3. 配置方向控制引脚 PB12~PB15 为推挽输出
     *    对应原理图:
     *      PB12 -> INB_2
     *      PB13 -> INB_1
     *      PB14 -> INA_2
     *      PB15 -> INA_1
     */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 |
                                    GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 初始全部拉低：电机停止 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 |
                         GPIO_Pin_14 | GPIO_Pin_15);

    /* 4. STBY 引脚 (PA12) 置为推挽输出并拉高，解除待机
     *    原理图: STBY -> PA12
     */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_12);   // STBY = 1, 使能驱动芯片
}

/*-------------------- TIM1 PWM 初始化 --------------------*/
/* PA8 -> TIM1_CH1  (PWM_ENA)
 * PA11 -> TIM1_CH4 (PWM_ENB)
 */
static void TIM1_PWM_Init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    /* 1. 配置 PA8, PA11 为复用推挽输出 (AF_PP) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 2. 定时器 1 基本配置 */
    TIM_TimeBaseStructure.TIM_Prescaler         = psc;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period            = arr;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /* 3. PWM 模式配置，通道 1 和 4 都用同一套参数 */
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse       = 0;   // 初始占空比 0

    TIM_OC1Init(TIM1, &TIM_OCInitStructure);  // CH1 -> PA8  (PWM_ENA)
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);  // CH4 -> PA11 (PWM_ENB)

    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    /* 4. 高级定时器要打开主输出使能 MOE */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

/*-------------------- 限幅函数 --------------------*/
int int_protect(int data, int max_out, int min_out)
{
    if (data >= max_out) data = max_out;
    if (data <= min_out) data = min_out;
    return data;
}

/*-------------------- 电机控制函数 --------------------*/
/* 约定:
 *   l_pwm, r_pwm 取值范围 [-2000, 2000]
 *   >0 正转, <0 反转, 0 停止
 */
void Move_control(int l_pwm,int r_pwm)
{
    /* 1. 限幅 */
    l_pwm = int_protect(l_pwm, 2000, -2000);
    r_pwm = int_protect(r_pwm, 4000, -4000);

    /* 2. 左电机 (PWM_ENA + INA1/INA2) -> 对应 PB15/PB14 */
    if (l_pwm >= 0)
    {
        // 正转：INA_1 = 1, INA_2 = 0
        GPIO_SetBits(GPIOB, GPIO_Pin_15); // INA_1
        GPIO_ResetBits(GPIOB,   GPIO_Pin_14); // INA_2
        TIM_SetCompare1(TIM1, (uint16_t)l_pwm);   // CH1 -> PA8
    }
    else
    {
        // 反转：INA_1 = 0, INA_2 = 1
        GPIO_ResetBits(GPIOB,   GPIO_Pin_15);
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
        TIM_SetCompare1(TIM1, (uint16_t)(-l_pwm));
    }

    /* 3. 右电机 (PWM_ENB + INB1/INB2) -> 对应 PB13/PB12 */
    if (r_pwm >= 0)
    {
        // 正转：INB_1 = 1, INB_2 = 0
        GPIO_ResetBits(GPIOB, GPIO_Pin_13); // INB_1
        GPIO_SetBits(GPIOB,   GPIO_Pin_12); // INB_2
        TIM_SetCompare4(TIM1, (uint16_t)r_pwm);    // CH4 -> PA11
    }
    else
    {
        // 反转：INB_1 = 0, INB_2 = 1
        GPIO_SetBits(GPIOB,   GPIO_Pin_13);
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
        TIM_SetCompare4(TIM1, (uint16_t)(-r_pwm));
    }
}