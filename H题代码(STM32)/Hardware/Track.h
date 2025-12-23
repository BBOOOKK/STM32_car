#ifndef __TRACK_H
#define __TRACK_H

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stdint.h>

/**
 * @brief 循迹传感器驱动模块头文件 (四路传感器)
 *
 * 适用于 STM32F103C8T6 + 四路循迹传感器 (TCRT5000 等)。
 * 传感器 SW1, SW2, SW3, SW4 分别连接到 GPIOA 的 PA1, PA0, PA2, PA3 引脚。
 * 提供初始化函数 Track_Init() 和读取所有传感器状态的函数 Track_Read_All()。
 */

// 定义四路循迹传感器所连接的引脚和端口
#define SW1_PIN   GPIO_Pin_1   // 传感器 SW1 对应 GPIOA 的 PA1 引脚
#define SW1_PORT  GPIOA        // 传感器 SW1 所在 GPIO 端口
#define SW2_PIN   GPIO_Pin_0   // 传感器 SW2 对应 GPIOA 的 PA0 引脚
#define SW2_PORT  GPIOA        // 传感器 SW2 所在 GPIO 端口
#define SW3_PIN   GPIO_Pin_2   // 传感器 SW3 对应 GPIOA 的 PA2 引脚
#define SW3_PORT  GPIOA        // 传感器 SW3 所在 GPIO 端口
#define SW4_PIN   GPIO_Pin_3   // 传感器 SW4 对应 GPIOA 的 PA3 引脚
#define SW4_PORT  GPIOA        // 传感器 SW4 所在 GPIO 端口

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化循迹传感器引脚.
 *
 * 配置 PA0, PA1, PA2, PA3 为上拉输入模式, 用于连接 SW1~SW4 传感器.
 * 在调用 Track_Read_All() 读取传感器状态之前, 需要先调用此函数初始化硬件.
 */
void Track_Init(void);

/**
 * @brief 读取四路循迹传感器的当前状态.
 *
 * @param[out] s1 指向用于存储 SW1 状态的变量的指针 (1 表示检测到线路, 0 表示未检测到).
 * @param[out] s2 指向用于存储 SW2 状态的变量的指针 (1 表示检测到线路, 0 表示未检测到).
 * @param[out] s3 指向用于存储 SW3 状态的变量的指针 (1 表示检测到线路, 0 表示未检测到).
 * @param[out] s4 指向用于存储 SW4 状态的变量的指针 (1 表示检测到线路, 0 表示未检测到).
 *
 * 调用此函数会读取每个传感器引脚的电平并存储到对应的输出变量中.
 * 传感器输出高电平 (1) 表示对应位置有线路, 输出低电平 (0) 表示对应位置无线路.
 */
void Track_Read_All(uint8_t* s1, uint8_t* s2, uint8_t* s3, uint8_t* s4);

#ifdef __cplusplus
}
#endif

#endif /* __TRACK_H */
