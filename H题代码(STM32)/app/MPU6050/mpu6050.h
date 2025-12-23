#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f10x.h"
#include "sys.h"
#include "IIC.h"      // 使用模拟I2C的函数

/* MPU6050寄存器地址宏定义 */
#define MPU_SELF_TESTX_REG    0x0D    // 自检寄存器X
#define MPU_SELF_TESTY_REG    0x0E    // 自检寄存器Y
#define MPU_SELF_TESTZ_REG    0x0F    // 自检寄存器Z
#define MPU_SELF_TESTA_REG    0x10    // 自检寄存器A
#define MPU_SAMPLE_RATE_REG   0x19    // 采样率分频器寄存器
#define MPU_CFG_REG           0x1A    // 配置寄存器(DLPF)
#define MPU_GYRO_CFG_REG      0x1B    // 陀螺仪配置寄存器(满量程范围)
#define MPU_ACCEL_CFG_REG     0x1C    // 加速度计配置寄存器(满量程范围)
#define MPU_FIFO_EN_REG       0x23    // FIFO使能寄存器
#define MPU_INTBP_CFG_REG     0x37    // INT引脚/旁路配置寄存器
#define MPU_INT_EN_REG        0x38    // 中断使能寄存器
#define MPU_INT_STATUS_REG    0x3A    // 中断状态寄存器
#define MPU_ACCEL_XOUTH_REG   0x3B    // 加速度计X轴高字节
#define MPU_ACCEL_XOUTL_REG   0x3C    // 加速度计X轴低字节
#define MPU_ACCEL_YOUTH_REG   0x3D    // 加速度计Y轴高字节
#define MPU_ACCEL_YOUTL_REG   0x3E    // 加速度计Y轴低字节
#define MPU_ACCEL_ZOUTH_REG   0x3F    // 加速度计Z轴高字节
#define MPU_ACCEL_ZOUTL_REG   0x40    // 加速度计Z轴低字节
#define MPU_TEMP_OUTH_REG     0x41    // 温度高字节
#define MPU_TEMP_OUTL_REG     0x42    // 温度低字节
#define MPU_GYRO_XOUTH_REG    0x43    // 陀螺仪X轴高字节
#define MPU_GYRO_XOUTL_REG    0x44    // 陀螺仪X轴低字节
#define MPU_GYRO_YOUTH_REG    0x45    // 陀螺仪Y轴高字节
#define MPU_GYRO_YOUTL_REG    0x46    // 陀螺仪Y轴低字节
#define MPU_GYRO_ZOUTH_REG    0x47    // 陀螺仪Z轴高字节
#define MPU_GYRO_ZOUTL_REG    0x48    // 陀螺仪Z轴低字节
#define MPU_USER_CTRL_REG     0x6A    // 用户控制寄存器
#define MPU_PWR_MGMT1_REG     0x6B    // 电源管理1寄存器
#define MPU_PWR_MGMT2_REG     0x6C    // 电源管理2寄存器
#define MPU_DEVICE_ID_REG     0x75    // 器件ID寄存器 (WHO_AM_I)

/* 器件I2C地址宏定义 */
#define MPU_ADDR    0x68    // MPU6050的I2C从机地址, AD0接地时为0x68:contentReference[oaicite:0]{index=0}

/* 函数声明 */
u8 MPU6050_Init(void);                                            // 初始化MPU6050
u8 MPU_Set_Gyro_Fsr(u8 fsr);                                      // 设置陀螺仪满量程范围(0=±250°/s,1=±500°/s,2=±1000°/s,3=±2000°/s)
u8 MPU_Set_Accel_Fsr(u8 fsr);                                     // 设置加速度计满量程范围(0=±2g,1=±4g,2=±8g,3=±16g)
u8 MPU_Set_LPF(u16 lpf);                                          // 设置数字低通滤波频率
u8 MPU_Set_Rate(u16 rate);                                        // 设置采样率(4Hz~1000Hz)
u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf);               // 连续写多个寄存器
u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf);                // 连续读多个寄存器
u8 MPU_Write_Byte(u8 reg, u8 data);                               // 向单个寄存器写入一个字节
u8 MPU_Read_Byte(u8 reg);                                         // 从单个寄存器读取一个字节
short MPU_Get_Temperature(void);                                  // 获取温度值，扩大100倍 (单位: °C*100)
u8 MPU_Get_Gyroscope(short *gx, short *gy, short *gz);            // 获取陀螺仪原始数据
u8 MPU_Get_Accelerometer(short *ax, short *ay, short *az);        // 获取加速度计原始数据
u8 MPU6050_ReadAccelTemp(short *ax, short *ay, short *az, short *temp); // 读取加速度计和温度数据
float MPU6050_CalcTiltAngle(short ax, short az, short gy);        // 计算倾斜角（姿态角）

#endif
