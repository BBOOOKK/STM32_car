#include "Track.h"

/**
 * @brief 初始化循迹传感器 (SW1~SW4) 引脚为上拉输入模式.
 * 
 * 使能 GPIOA 外设时钟, 并将 PA0, PA1, PA2, PA3 配置为上拉输入.
 * 这些引脚连接到循迹传感器模块 SW1~SW4, 上拉输入确保未被传感器拉低时读取为高电平.
 */
void Track_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    // 开启 GPIOA 外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // 配置 PA0, PA1, PA2, PA3 为上拉输入模式 (内部上拉电阻使能)
    GPIO_InitStructure.GPIO_Pin = SW1_PIN | SW2_PIN | SW3_PIN | SW4_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        // 上拉输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    // GPIO 时钟速度 (对于输入引脚可随意设置)
    GPIO_Init(SW1_PORT, &GPIO_InitStructure);
}

/**
 * @brief 读取所有四路循迹传感器的状态 (白线模式).
 * 
 * 依次读取 SW1, SW2, SW3, SW4 传感器引脚的电平状态, 并对结果取反后返回.
 * 取反后: 电平为高 (1) 表示传感器检测到白线, 电平为低 (0) 表示检测到黑色背景.
 *
 * @param[out] s1 存储 SW1 传感器状态的地址 (1: 白线; 0: 黑色背景).
 * @param[out] s2 存储 SW2 传感器状态的地址 (1: 白线; 0: 黑色背景).
 * @param[out] s3 存储 SW3 传感器状态的地址 (1: 白线; 0: 黑色背景).
 * @param[out] s4 存储 SW4 传感器状态的地址 (1: 白线; 0: 黑色背景).
 */
void Track_Read_All(uint8_t* s1, uint8_t* s2, uint8_t* s3, uint8_t* s4) {
    // 分别读取各传感器引脚的当前电平并取反 (白线模式)
    *s1 = !GPIO_ReadInputDataBit(SW1_PORT, SW1_PIN);
    *s2 = !GPIO_ReadInputDataBit(SW2_PORT, SW2_PIN);
    *s3 = !GPIO_ReadInputDataBit(SW3_PORT, SW3_PIN);
    *s4 = !GPIO_ReadInputDataBit(SW4_PORT, SW4_PIN);
}