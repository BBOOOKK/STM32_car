#ifndef __IIC_H
#define __IIC_H

#include "stm32f10x.h"   /* 使用标准外设库的GPIO定义 */
#include "sys.h"        /* 包含类型定义 (如 u8, u16 等) */

#define IIC_DELAY_US    5   // I2C时序延迟的微秒数，控制通讯速度

/* 
 * 模拟I2C引脚宏定义:
 * SCL时钟线 -> PB10
 * SDA数据线 -> PB11
 * 初始为空闲状态时，两线均为高电平（由上拉电阻拉高）
 */
#define IIC_SCL_PORT    GPIOB
#define IIC_SDA_PORT    GPIOB
#define IIC_SCL_PIN     GPIO_Pin_10
#define IIC_SDA_PIN     GPIO_Pin_11

/* 将SCL引脚拉高或拉低 */
#define IIC_SCL_HIGH()  GPIO_SetBits(IIC_SCL_PORT, IIC_SCL_PIN)    // SCL = 1
#define IIC_SCL_LOW()   GPIO_ResetBits(IIC_SCL_PORT, IIC_SCL_PIN)  // SCL = 0

/* 将SDA引脚拉高或拉低 */
#define IIC_SDA_HIGH()  GPIO_SetBits(IIC_SDA_PORT, IIC_SDA_PIN)    // SDA = 1 (空闲/释放总线)
#define IIC_SDA_LOW()   GPIO_ResetBits(IIC_SDA_PORT, IIC_SDA_PIN)  // SDA = 0 (主动拉低)

/* 读取SDA引脚电平 (输入) */
#define IIC_SDA_READ()  (GPIO_ReadInputDataBit(IIC_SDA_PORT, IIC_SDA_PIN))  // 返回1表示高电平，0表示低电平

/* 模拟I2C操作函数声明 */
void MPU_IIC_Init(void);                  // 初始化I2C引脚 (PB10, PB11) 
void MPU_IIC_Start(void);                 // 发送I2C开始信号
void MPU_IIC_Stop(void);                  // 发送I2C停止信号 
void MPU_IIC_Send_Byte(u8 txd);           // 发送一个字节数据 
u8   MPU_IIC_Read_Byte(u8 ack);           // 读取一个字节数据，ack=1时发送ACK，ack=0发送NACK 
u8   MPU_IIC_Wait_Ack(void);              // 等待从机ACK信号 
void MPU_IIC_Ack(void);                   // 发送ACK应答 
void MPU_IIC_NAck(void);                  // 发送NACK应答 

#endif
