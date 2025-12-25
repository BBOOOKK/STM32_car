#ifndef __W26Q64_H__
#define __W26Q64_H__

/**
 * @file w25q64.h
 * @author Min
 * @brief 外置flash读写
 * @version 1.0
 * @date 2024-12-28
 *
 * @copyright Copyright (c) 2024 Hiwonder
 *
 */
 
#include "stdint.h"

/* W25Q64ָ存储芯片 指令宏 */
#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_PageProgram		0x02
#define W25X_SectorErase		0x20
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID	0x90
#define W25X_JedecDeviceID		0x9F
#define W25X_UniqueID			0x4B

typedef enum
{
	WRITE_READ_OK,
	WRITE_READE_ERROR,
	WRITE_READ_BUSY,
	WRITE_READ_TIMEOUT
	
}WriteReadStatusTypeDef;

typedef struct  W25Q64Handle W25Q64HandleTypeDef;
struct W25Q64Handle
{
	uint8_t 				addr;
	WriteReadStatusTypeDef	write_read_status;
	
    void (*write_cs_pin)(uint8_t new_state);
	uint8_t (*write_read_data)(const uint8_t* write_data, uint16_t size);
};

/**
 * @brief W25q64存储芯片初始化
 * 
 */
void w25q64_init(void);

/**
 * @brief 扇区擦除（4KB）
 * 
 * @param  addr-扇区首地址
 */
void w25q64_erase_sector(uint32_t addr);

/**
 * @brief 读取存储在FLASH中的数据
 * 
 * @param  addr-数据储存的地址
 * @param  buffer-读取到的数据存放位置
 * @param  len-读取数据的长度
 */
void w25q64_read(uint32_t addr, uint8_t* buffer, uint32_t len);

/**
 * @brief 写入数据到FLASH中
 * 
 * @param  addr-写入数据的地址
 * @param  buffer-写入的数据信息
 * @param  len-写入数据的长度
 */
void w25q64_write(uint32_t addr, const uint8_t* buffer, uint32_t len);

/**
 * @brief 读取W25q64生产设备ID
 * 
 * @param  id
 */
void w25q64_read_manufact_device_id(uint32_t* id);

/**
 * @brief 读取W25q64 jedec ID
 * 
 * @param  id
 */
void w25q64_read_jedec_device_id(uint32_t* id);
#endif
