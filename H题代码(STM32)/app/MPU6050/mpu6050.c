#include "mpu6050.h"
#include "Delay.h"
#include <math.h>    // 用于浮点计算 (atan2)

#define RAD2DEG 57.29578f   // 弧度转度数系数 (180/PI)

/* 
 * MPU6050驱动代码 
 * 使用软件I2C通过PB10/PB11与MPU6050通信。AD0引脚接地（I2C地址0x68），未使用INT引脚。
 * 保留基本功能：初始化、读出加速度/温度、计算倾斜角等。
 */

/**
  * @brief  初始化MPU6050传感器
  * @note   复位MPU6050并配置传感器参数（陀螺仪量程±2000°/s，加速度量程±2g，采样率50Hz，关闭中断和FIFO等）。
  * @retval 0表示初始化成功，1表示器件ID不匹配或初始化失败。
  */
u8 MPU6050_Init(void)
{
    u8 res;
    /* 初始化模拟I2C总线 */
    MPU_IIC_Init();

    /* 1. 复位MPU6050 */
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x80);  // 向PWR_MGMT1寄存器写入0x80以复位MPU6050
    Delay_ms(100);                           // 延时100ms，等待复位完成

    /* 2. 唤醒MPU6050 */
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x00);  // 清除睡眠模式位，使能温度传感器，时钟源设置为内部8MHz

    /* 3. 陀螺仪和加速度计配置 */
    MPU_Set_Gyro_Fsr(3);    // 陀螺仪满量程范围±2000dps
    MPU_Set_Accel_Fsr(0);   // 加速度计满量程范围±2g

    /* 4. 配置采样率和数字低通滤波器 */
    MPU_Set_Rate(50);       // 设置采样率为50Hz，内部数字低通滤波频率为采样率的一半

    /* 5. 关闭所有中断及FIFO */
    MPU_Write_Byte(MPU_INT_EN_REG, 0x00);     // 关闭中断
    MPU_Write_Byte(MPU_USER_CTRL_REG, 0x00);  // I2C主模式关闭，FIFO关闭
    MPU_Write_Byte(MPU_FIFO_EN_REG, 0x00);    // 关闭FIFO

    /* 6. 配置INT引脚为推挽，激活电平为低（仅在使用中断时有意义，这里只是默认配置） */
    MPU_Write_Byte(MPU_INTBP_CFG_REG, 0x80);  // INT引脚低电平有效，推挽输出

    /* 7. 检查设备ID */
    res = MPU_Read_Byte(MPU_DEVICE_ID_REG);   // 读取MPU6050的器件ID (默认应为0x68)
    if (res == MPU_ADDR) 
    {
        /* 器件ID正确，开始初始化设置 */
        MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x01); // 设置CLKSEL，选择PLL_X轴作为内部参考时钟
        MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0x00); // 开启加速度计和陀螺仪，进入工作模式
        MPU_Set_Rate(50);                       // 再次设置采样率为50Hz，以应用CLKSEL设置
    } 
    else 
    {
        return 1;  // 器件ID不匹配，初始化失败
    }
    return 0;      // 初始化成功
}

/**
  * @brief  设置MPU6050陀螺仪的满量程范围
  * @param  fsr: 陀螺仪量程索引值 
  *         0 = ±250°/s, 1 = ±500°/s, 2 = ±1000°/s, 3 = ±2000°/s
  * @retval 0表示设置成功，其他值表示设置失败
  */
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3);  // 将满量程范围值左移3位后写入GYRO_CONFIG寄存器
}

/**
  * @brief  设置MPU6050加速度计的满量程范围
  * @param  fsr: 加速度计量程索引值 
  *         0 = ±2g, 1 = ±4g, 2 = ±8g, 3 = ±16g
  * @retval 0表示设置成功，其他值表示设置失败
  */
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3); // 将满量程范围值左移3位后写入ACCEL_CONFIG寄存器
}

/**
  * @brief  设置MPU6050的数字低通滤波器频率
  * @param  lpf: 期望的低通滤波截止频率 (Hz)
  * @retval 0表示设置成功，其他值表示设置失败
  * @note   根据MPU6050寄存器配置，支持的典型LPF值: 188Hz, 98Hz, 42Hz, 20Hz, 10Hz, 5Hz 等。
  *         函数内部根据提供的lpf值选择最接近的寄存器配置值。
  */
u8 MPU_Set_LPF(u16 lpf)
{
    u8 data;
    if (lpf >= 188) data = 1;
    else if (lpf >= 98) data = 2;
    else if (lpf >= 42) data = 3;
    else if (lpf >= 20) data = 4;
    else if (lpf >= 10) data = 5;
    else data = 6;
    return MPU_Write_Byte(MPU_CFG_REG, data);  // 将计算得到的配置值写入CONFIG寄存器
}

/**
  * @brief  设置MPU6050采样率(假定陀螺仪输出频率=1kHz)
  * @param  rate: 期望采样率(范围4~1000 Hz)
  * @retval 0表示设置成功，其他值表示设置失败
  * @note   采样率受限于内部采样，最小4Hz，最大1kHz。
  *         实际设置寄存器值: SMPLRT_DIV = 1000/rate - 1。
  *         此函数会根据设置的采样率自动调整数字低通滤波频率为采样率的一半，提高信号品质。
  */
u8 MPU_Set_Rate(u16 rate)
{
    u8 data;
    if (rate > 1000) rate = 1000;
    if (rate < 4)    rate = 4;
    data = 1000 / rate - 1;
    MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data);     // 将计算得到的采样分频值写入SMPLRT_DIV寄存器
    return MPU_Set_LPF(rate / 2);                  // 自动设置数字低通滤波频率为采样率的一半
}

/**
  * @brief  连续向MPU6050写入多个字节
  * @param  addr: 7位器件地址 (MPU6050地址0x68)
  * @param  reg: 起始寄存器地址
  * @param  len: 写入长度（字节数）
  * @param  buf: 存放待写入数据的缓冲区指针 
  * @retval 0表示写入成功，1表示写入失败
  */
u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    u8 i;
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((addr << 1) | 0x00);  // 发送器件地址和写标志位
    if (MPU_IIC_Wait_Ack())    // 等待ACK应答
    {
        MPU_IIC_Stop();
        return 1;
    }
    MPU_IIC_Send_Byte(reg);    // 发送寄存器起始地址
    MPU_IIC_Wait_Ack();
    for (i = 0; i < len; i++)
    {
        MPU_IIC_Send_Byte(buf[i]);  // 逐字节发送数据
        if (MPU_IIC_Wait_Ack())    // 检查ACK
        {
            MPU_IIC_Stop();
            return 1;
        }
    }
    MPU_IIC_Stop();
    return 0;   // 写入成功
}

/**
  * @brief  连续从MPU6050读取多个字节
  * @param  addr: 7位器件地址 (MPU6050地址0x68)
  * @param  reg: 起始寄存器地址
  * @param  len: 读取长度（字节数）
  * @param  buf: 存放读出数据的缓冲区指针 
  * @retval 0表示读取成功，1表示读取失败
  */
u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    u8 i;
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((addr << 1) | 0x00);   // 发送器件地址和写标志位（先写入要读取的寄存器地址）
    if (MPU_IIC_Wait_Ack())
    {
        MPU_IIC_Stop();
        return 1;
    }
    MPU_IIC_Send_Byte(reg);                 // 指定寄存器起始地址
    MPU_IIC_Wait_Ack();
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((addr << 1) | 0x01);   // 发送器件地址和读标志位
    MPU_IIC_Wait_Ack();
    while (len)
    {
        if (len == 1)
            *buf = MPU_IIC_Read_Byte(0);    // 读最后一个字节，发送NACK
        else 
            *buf = MPU_IIC_Read_Byte(1);    // 读下一个字节，发送ACK
        len--;
        buf++;
    }
    MPU_IIC_Stop();
    return 0;    // 读取成功
}

/**
  * @brief  向MPU6050的单个寄存器写入一个字节数据
  * @param  reg: 目标寄存器地址
  * @param  data: 要写入的数据
  * @retval 0表示写入成功，1表示写入失败
  */
u8 MPU_Write_Byte(u8 reg, u8 data)
{
    u8 res;
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((MPU_ADDR << 1) | 0x00);  // 发送器件地址和写标志位
    res = MPU_IIC_Wait_Ack();
    if (res) { MPU_IIC_Stop(); return 1; }
    MPU_IIC_Send_Byte(reg);    // 发送寄存器地址
    MPU_IIC_Wait_Ack();
    MPU_IIC_Send_Byte(data);   // 发送数据
    if (MPU_IIC_Wait_Ack())
    {
        MPU_IIC_Stop();
        return 1;
    }
    MPU_IIC_Stop();
    return 0;
}

/**
  * @brief  从MPU6050的单个寄存器读取一个字节数据
  * @param  reg: 要读取的寄存器地址 
  * @retval 读取到的字节数据
  */
u8 MPU_Read_Byte(u8 reg)
{
    u8 res;
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((MPU_ADDR << 1) | 0x00);  // 发送器件地址和写标志位
    MPU_IIC_Wait_Ack();
    MPU_IIC_Send_Byte(reg);    // 发送要读取的寄存器地址
    MPU_IIC_Wait_Ack();
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((MPU_ADDR << 1) | 0x01);  // 发送器件地址和读标志位
    MPU_IIC_Wait_Ack();
    res = MPU_IIC_Read_Byte(0);  // 读取数据，发送NACK表示结束
    MPU_IIC_Stop();
    return res;
}

/**
  * @brief  获取温度传感器的测量值
  * @note   将原始温度计输出转换为实际温度值，单位扩大100倍（°C）。
  *         原始温度计输出与实际温度的换算: Temp(°C) = 36.53 + 温度原始值/340。
  * @retval 温度值(°C*100，例如3653表示36.53°C)
  */
short MPU_Get_Temperature(void)
{
    u8 buf[2];
    short raw;
    float temp;
    MPU_Read_Len(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);  // 连续读取2个字节的温度数据
    raw = ((u16)buf[0] << 8) | buf[1];
    temp = 36.53f + ((double)raw) / 340.0;
    return (short)(temp * 100);
}

/**
  * @brief  获取陀螺仪的原始读数
  * @param  gx: 用于存放X轴陀螺仪原始值的指针
  * @param  gy: 用于存放Y轴陀螺仪原始值的指针
  * @param  gz: 用于存放Z轴陀螺仪原始值的指针
  * @retval 0表示读取成功，其他值表示读取失败
  */
u8 MPU_Get_Gyroscope(short *gx, short *gy, short *gz)
{
    u8 buf[6], res;
    res = MPU_Read_Len(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);  // 连续读取6个字节: 陀螺仪X、Y、Z轴高低字节
    if (res == 0)
    {
        *gx = ((u16)buf[0] << 8) | buf[1];
        *gy = ((u16)buf[2] << 8) | buf[3];
        *gz = ((u16)buf[4] << 8) | buf[5];
    }
    return res;
}

/**
  * @brief  获取加速度计的原始读数
  * @param  ax: 用于存放X轴加速度原始值的指针
  * @param  ay: 用于存放Y轴加速度原始值的指针
  * @param  az: 用于存放Z轴加速度原始值的指针
  * @retval 0表示读取成功，其他值表示读取失败
  */
u8 MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
    u8 buf[6], res;
    res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);  // 连续读取6个字节: 加速度计X、Y、Z轴高低字节
    if (res == 0)
    {
        *ax = ((u16)buf[0] << 8) | buf[1];
        *ay = ((u16)buf[2] << 8) | buf[3];
        *az = ((u16)buf[4] << 8) | buf[5];
    }
    return res;
}

/**
  * @brief  读取MPU6050的加速度计和温度数据
  * @param  ax: 用于存放X轴加速度原始值的指针
  * @param  ay: 用于存放Y轴加速度原始值的指针
  * @param  az: 用于存放Z轴加速度原始值的指针
  * @param  temp: 用于存放温度值(°C*100)的指针
  * @retval 0表示读取成功，其他值表示读取失败
  * @note   一次I2C读操作获取加速度计三轴和温度共4个数据，提高通信效率。
  *         温度值计算方式同 MPU_Get_Temperature。
  */
u8 MPU6050_ReadAccelTemp(short *ax, short *ay, short *az, short *temp)
{
    u8 buf[8], res;
    short rawTemp;
    res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 8, buf);  // 从ACCEL_XOUT_H寄存器开始连续读取8字节: 3轴加速度(6字节)+温度(2字节)
    if (res == 0)
    {
        *ax = ((u16)buf[0] << 8) | buf[1];
        *ay = ((u16)buf[2] << 8) | buf[3];
        *az = ((u16)buf[4] << 8) | buf[5];
        rawTemp = ((u16)buf[6] << 8) | buf[7];
        /* 将原始温度计输出转换为实际温度值(°C*100) */
        *temp = (short)((36.53f + ((double)rawTemp) / 340.0) * 100);
    }
    return res;
}

/**
  * @brief  计算MPU6050的倾斜角(姿态角)
  * @param  ax: X轴加速度原始值
  * @param  az: Z轴加速度原始值
  * @param  gy: Y轴陀螺仪原始值 (角速度)
  * @retval 计算得到的倾斜角度值 (单位: 度)
  * @note   使用一阶互补滤波融合加速度计和陀螺仪数据计算倾斜角。
  *         ax和az用于计算基于重力的倾角, gy用于角速度积分跟踪快速变化。
  *         函数假定传感器以50Hz采样，陀螺仪量程±2000°/s (LSB约=16.4 LSB/(°/s))。
  *         如果实际采样频率或陀螺仪量程不同，需要调整滤波系数和计算因子。
  */
float MPU6050_CalcTiltAngle(short ax, short az, short gy)
{
    static float angle = 0.0f;    // 保存上一时刻的角度解算结果(初始为0)
    float accel_angle, gyro_rate;
    float dt = 0.02f;            // 采样周期 (20ms，对应50Hz采样率)

    /* 1. 利用加速度计计算倾斜角（假设在静止状态下，加速度计Z轴指向竖直方向） 
     * 使用atan2(y, x)计算角度，此处取atan2(X轴加速度, Z轴加速度)，得到绕Y轴的倾斜角度。
     * 注意：加速度计算的角度受瞬时动态加速度影响，在运动时不够稳定。 */
    accel_angle = atan2f((float)ax, (float)az) * RAD2DEG;   // 将atan2计算得到的弧度转换为度

    /* 2. 陀螺仪角速度积分计算角度变化
     * gy为Y轴陀螺仪原始值，满量程±2000°/s时每LSB约等于16.4°/s。
     * 先将原始值转换为°/s，再乘以时间dt得到角度增量。 */
    gyro_rate = (float)gy / 16.4f;                // 转换陀螺仪原始值为角速度(°/s)
    angle += gyro_rate * dt;                     // 积分得到根据陀螺仪计算的倾斜角增量

    /* 3. 互补滤波融合：结合加速度倾角和陀螺仪积分角 
     * alpha为陀螺仪信号权重(0<alpha<1)。这里取约0.98，即陀螺仪占98%，加速度占2%。
     * 陀螺仪积分角用于短期快速变化，加速度倾角用于长稳态校正。 */
    angle = 0.98f * angle + 0.02f * accel_angle;

    return angle;
}



