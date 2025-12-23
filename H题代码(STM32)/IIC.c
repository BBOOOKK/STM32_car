#include "IIC.h"
#include "Delay.h"   // 使用精确延时函数(Delay_us)

/**
  * @brief  初始化模拟I2C接口的GPIO引脚 (PB10=SCL, PB11=SDA).
  *         配置PB10和PB11为开漏输出(Open-Drain)，并使能对应的GPIO时钟。
  *         初始化后将两条线拉高释放总线空闲状态。
  * @note   使用标准外设库 GPIO_InitTypeDef 结构体配置引脚。
  */
void MPU_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* 使能 GPIOB 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 配置 PB10(SCL) 和 PB11(SDA) 为开漏输出, 50MHz */
    GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN | IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;      // 开漏输出模式 (需上拉电阻)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);

    /* 将SCL和SDA先输出高电平, 释放I2C总线 */
    IIC_SCL_HIGH();
    IIC_SDA_HIGH();
}

/**
  * @brief  产生I2C起始信号:
  *         当SCL高电平时, SDA由高变低表示开始信号。
  * @note   在调用开始信号前，SDA和SCL都应为高电平（总线空闲状态）。
  */
void MPU_IIC_Start(void)
{
    IIC_SDA_HIGH();
    IIC_SCL_HIGH();
    Delay_us(IIC_DELAY_US);         // 确保总线空闲状态维持一段时间

    IIC_SDA_LOW();                 // 当SCL为高时将SDA拉低，产生起始条件
    Delay_us(IIC_DELAY_US);
    IIC_SCL_LOW();                 // 钳住I2C总线，开始发送数据位
    Delay_us(IIC_DELAY_US);
}

/**
  * @brief  产生I2C停止信号:
  *         当SCL高电平时, SDA由低变高表示停止信号。
  * @note   释放总线后，SDA和SCL都会被上拉为高电平（总线空闲）。
  */
void MPU_IIC_Stop(void)
{
    IIC_SCL_LOW();
    IIC_SDA_LOW();
    Delay_us(IIC_DELAY_US);

    IIC_SCL_HIGH();
    Delay_us(IIC_DELAY_US);
    IIC_SDA_HIGH();               // 当SCL为高时将SDA拉高，产生停止条件，结束通信
    Delay_us(IIC_DELAY_US);
}

/**
  * @brief  等待从机发送的ACK信号。
  * @retval 0 表示从机应答ACK，1表示未收到ACK（从机未应答或总线故障）。
  * @note   本函数在发送完一个字节数据后调用，用于判断从机是否正确接收数据。
  */
u8 MPU_IIC_Wait_Ack(void)
{
    u8 errTime = 0;
    /* 释放SDA以便从机拉低ACK */
    IIC_SDA_HIGH();
    Delay_us(IIC_DELAY_US);

    IIC_SCL_HIGH();      // 拉高时钟线，准备读取ACK信号
    Delay_us(IIC_DELAY_US);
    /* 读取SDA线状态 */
    while (IIC_SDA_READ())
    {
        errTime++;
        if(errTime > 250)    // 等待超过一定时间仍未拉低，认为无应答
        {
            IIC_SCL_LOW();
            return 1;       // 未收到ACK信号，通信失败
        }
    }
    IIC_SCL_LOW();          // 收到ACK后将SCL拉低，准备发送下一位数据
    return 0;               // 成功收到ACK
}

/**
  * @brief  发送ACK应答信号。
  * @note   主机在从机发送完数据后调用此函数拉低SDA发送ACK，表示已成功接收到数据。
  */
void MPU_IIC_Ack(void)
{
    IIC_SCL_LOW();
    IIC_SDA_LOW();           // 拉低SDA，准备发送ACK信号(ACK = 0)
    Delay_us(IIC_DELAY_US);
    IIC_SCL_HIGH();          // 拉高SCL，在高电平期间发送ACK位
    Delay_us(IIC_DELAY_US);
    IIC_SCL_LOW();
    IIC_SDA_HIGH();          // 释放SDA，总线恢复空闲数据线高电平
}

/**
  * @brief  发送NACK非应答信号。
  * @note   主机在已接收最后一个字节数据后调用此函数拉高SDA发送NACK，通知从机停止发送。
  */
void MPU_IIC_NAck(void)
{
    IIC_SCL_LOW();
    IIC_SDA_HIGH();          // 拉高SDA，准备发送NACK信号(NACK = 1)
    Delay_us(IIC_DELAY_US);
    IIC_SCL_HIGH();
    Delay_us(IIC_DELAY_US);
    IIC_SCL_LOW();
}

/**
  * @brief  通过I2C总线发送一个字节数据。
  * @param  txd: 要发送的字节数据。
  * @note   从高位(MSB)到低位(LSB)依次发送每一位。
  *         每发送一位，在SCL高电平时数据线电平需稳定有效。
  */
void MPU_IIC_Send_Byte(u8 txd)
{
    u8 bit;
    IIC_SCL_LOW();                   // 拉低时钟开始发送数据位
    for (bit = 0; bit < 8; bit++)
    {
        // 设置SDA输出数据位 (判断txd最高位)
        if (txd & 0x80) {
            IIC_SDA_HIGH();
        } else {
            IIC_SDA_LOW();
        }
        txd <<= 1;                  // 左移数据，准备下一位

        Delay_us(IIC_DELAY_US);
        IIC_SCL_HIGH();             // 拉高时钟线，SDA上的数据位被从机采样
        Delay_us(IIC_DELAY_US);
        IIC_SCL_LOW();              // 拉低时钟线，准备发送下一位
        Delay_us(IIC_DELAY_US);
    }
    // 字节发送完毕后，函数外应调用 MPU_IIC_Wait_Ack() 检查从机应答
}

/**
  * @brief  从I2C总线读取一个字节数据。
  * @param  ack: 指示主机发送ACK或NACK信号。
  *         ack=1时，主机发送ACK（表示还有数据需要读取），
  *         ack=0时，主机发送NACK（表示已读取完最后一个字节数据）。
  * @retval 读取到的字节数据。
  * @note   读取数据时需将SDA引脚设置为输入模式。此处利用开漏特性，通过释放总线并读取电平实现输入。
  */
u8 MPU_IIC_Read_Byte(u8 ack)
{
    u8 i, receive = 0;
    /* 释放SDA总线以供从机输出数据 */
    IIC_SDA_HIGH();                
    for (i = 0; i < 8; i++)
    {
        IIC_SCL_LOW();
        Delay_us(IIC_DELAY_US);
        IIC_SCL_HIGH();
        Delay_us(IIC_DELAY_US);
        // 读取数据位：在SCL高电平时从机输出的数据位稳定后读取
        receive <<= 1;
        if (IIC_SDA_READ()) {
            receive |= 0x01;
        }
        Delay_us(IIC_DELAY_US);
    }
    /* 读取完一个字节后发送ACK或NACK */
    if (ack) {
        MPU_IIC_Ack();    // 发送ACK信号，表示主机还要读取数据
    } else {
        MPU_IIC_NAck();   // 发送NACK信号，表示数据读取完毕
    }
    return receive;
}
