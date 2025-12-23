#include "stm32f10x.h"  // Device header

// TIM4 定时器初始化（20ms中断周期）
void TIM4_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);       // 开启TIM4时钟
    TIM_InternalClockConfig(TIM4);                            // 使用内部时钟

    TIM_TimeBaseInitTypeDef TIM_TimerStructure;
    // 配置定时器周期为20ms产生一次中断
    TIM_TimerStructure.TIM_ClockDivision = TIM_CKD_DIV1;       // 时钟分频：不分频
    TIM_TimerStructure.TIM_CounterMode = TIM_CounterMode_Up;   // 向上计数模式
    TIM_TimerStructure.TIM_Period = 200 - 1;                   // 自动重装载值 (ARR)，对应20ms周期【每50us一计数，共计4000次】
    TIM_TimerStructure.TIM_Prescaler = 7200 - 1;               // 预分频值，将72MHz主频预分为10kHz (7200分频)
    TIM_TimerStructure.TIM_RepetitionCounter = 0;              // 重复计数器（高级定时器用，不适用TIM4）
    TIM_TimeBaseInit(TIM4, &TIM_TimerStructure);               // 初始化TIM4定时器基础单元

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);                      // 清除更新中断标志位（防止立即触发中断）
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);                 // 使能TIM4更新中断

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;            // TIM4全局中断通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;         // 子优先级1
    NVIC_Init(&NVIC_InitStructure);                            // 配置并使能NVIC通道

    TIM_Cmd(TIM4, ENABLE);                                     // 开始计时
}

// 提供给主程序调用的初始化接口
void Timer_Init(void)
{
    TIM4_Init();
}


