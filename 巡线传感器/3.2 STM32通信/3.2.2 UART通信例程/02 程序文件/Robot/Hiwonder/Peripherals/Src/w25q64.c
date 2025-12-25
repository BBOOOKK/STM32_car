#include "w25q64.h"
#include "main.h"
#include "spi.h"

W25Q64HandleTypeDef		w25q64;
static const uint8_t	dummy = 0xff;

uint32_t _id;

static void write_cs_pin(uint8_t new_state)
{
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, (GPIO_PinState)new_state);
}

static uint8_t write_read_data(const uint8_t* write_data, uint16_t size)
{
	uint8_t read_data;
//	__disable_irq();
	w25q64.write_read_status = (WriteReadStatusTypeDef)HAL_SPI_TransmitReceive(&hspi1, write_data, &read_data, size, 20);
//	__enable_irq();
	return read_data;
}

void w25q64_init()
{
	w25q64.write_cs_pin = write_cs_pin;
	w25q64.write_read_data = write_read_data;
	w25q64_read_jedec_device_id(&_id);
}

static uint8_t read_sr(W25Q64HandleTypeDef* self)
{
	uint8_t byte = 0;
	const uint8_t rs_cmd = W25X_ReadStatusReg;
	
	self->write_cs_pin(0);
	self->write_read_data(&rs_cmd, sizeof(rs_cmd));
	byte = self->write_read_data(&dummy, sizeof(dummy));
	self->write_cs_pin(1);
	
	return byte;
}

static void wait_idle(W25Q64HandleTypeDef* self)
{
	while((read_sr(self) & 0x01) == 0x01);
}

static void write_enale(W25Q64HandleTypeDef* self)
{
	const uint8_t we_cmd = W25X_WriteEnable;
	
	self->write_cs_pin(0);
	self->write_read_data(&we_cmd, sizeof(we_cmd));
	self->write_cs_pin(1);
}

/* 4kb */
void w25q64_erase_sector(uint32_t addr)
{
	const uint8_t es_cmd = W25X_SectorErase;
	uint8_t H_addr, M_addr, L_addr;
	
	H_addr = (uint8_t)(addr >> 16);
	M_addr = (uint8_t)(addr >> 8);
	L_addr = (uint8_t)(addr);
	
	write_enale(&w25q64);
	wait_idle(&w25q64);
	w25q64.write_cs_pin(0);
	w25q64.write_read_data(&es_cmd, sizeof(es_cmd));
	/* transimit 24 bit address data */
	w25q64.write_read_data(&H_addr, sizeof(H_addr));
	w25q64.write_read_data(&M_addr, sizeof(M_addr));
	w25q64.write_read_data(&L_addr, sizeof(L_addr));
	w25q64.write_cs_pin(1);
	wait_idle(&w25q64);
}

void w25q64_read(uint32_t addr, uint8_t* buffer, uint32_t len)
{
	const uint8_t rd_cmd = W25X_ReadData;
	uint8_t H_addr, M_addr, L_addr;
	
	H_addr = (uint8_t)(addr >> 16);
	M_addr = (uint8_t)(addr >> 8);
	L_addr = (uint8_t)(addr);
	
	w25q64.write_cs_pin(0);
	w25q64.write_read_data(&rd_cmd, sizeof(rd_cmd));
	/* transimit 24 bit address data */
	w25q64.write_read_data(&H_addr, sizeof(H_addr));
	w25q64.write_read_data(&M_addr, sizeof(M_addr));
	w25q64.write_read_data(&L_addr, sizeof(L_addr));	
	
	for(uint8_t i = 0; i < len; i++)
	{
		buffer[i] = w25q64.write_read_data(&dummy, sizeof(dummy));
	}
	w25q64.write_cs_pin(1);
}

void w25q64_write(uint32_t addr, const uint8_t* buffer, uint32_t len)
{
	const uint8_t pp_cmd = W25X_PageProgram;
	uint8_t H_addr, M_addr, L_addr;
	
	H_addr = (uint8_t)(addr >> 16);
	M_addr = (uint8_t)(addr >> 8);
	L_addr = (uint8_t)(addr);
	
	write_enale(&w25q64);
	w25q64.write_cs_pin(0);	
	w25q64.write_read_data(&pp_cmd, sizeof(pp_cmd));
	/* transimit 24 bit address data */
	w25q64.write_read_data(&H_addr, sizeof(uint8_t));
	w25q64.write_read_data(&M_addr, sizeof(uint8_t));
	w25q64.write_read_data(&L_addr, sizeof(uint8_t));	
	
	for(uint8_t i = 0; i < len; i++)
	{
		w25q64.write_read_data(&buffer[i], sizeof(uint8_t));
	}	
	w25q64.write_cs_pin(1);
	wait_idle(&w25q64);
}

void w25q64_read_manufact_device_id(uint32_t* id)
{
	uint8_t addr = 0;
	const uint8_t id_cmd = W25X_ManufactDeviceID;
	
	w25q64.write_cs_pin(0);
	w25q64.write_read_data(&id_cmd, sizeof(id_cmd));
	/* transimit 24 bit address data */
	w25q64.write_read_data(&addr, sizeof(addr));
	w25q64.write_read_data(&addr, sizeof(addr));
	w25q64.write_read_data(&addr, sizeof(addr));
	*id = w25q64.write_read_data(&dummy, sizeof(dummy));
	*id <<= 8;
	*id |= w25q64.write_read_data(&dummy, sizeof(dummy));
	w25q64.write_cs_pin(1);
}

void w25q64_read_jedec_device_id(uint32_t* id)
{
	const uint8_t id_cmd = W25X_JedecDeviceID;
	
	w25q64.write_cs_pin(0);
	w25q64.write_read_data(&id_cmd, sizeof(id_cmd));
	*id = w25q64.write_read_data(&dummy, sizeof(dummy));
	*id <<= 8;
	*id |= w25q64.write_read_data(&dummy, sizeof(dummy));
	*id <<= 8;
	*id |= w25q64.write_read_data(&dummy, sizeof(dummy));
	w25q64.write_cs_pin(1);
}

void w25q64_read_unique_id(uint32_t* id_h, uint32_t* id_l)
{
	const uint8_t id_cmd = W25X_UniqueID;
	w25q64.write_cs_pin(0);
	w25q64.write_read_data(&id_cmd, sizeof(id_cmd));
	w25q64.write_read_data(&dummy, sizeof(dummy));
	w25q64.write_read_data(&dummy, sizeof(dummy));
	w25q64.write_read_data(&dummy, sizeof(dummy));
	w25q64.write_read_data(&dummy, sizeof(dummy));
	*id_h = w25q64.write_read_data(&dummy, sizeof(dummy));
	*id_h <<= 8;
	*id_h |= w25q64.write_read_data(&dummy, sizeof(dummy));
	*id_h <<= 8;
	*id_h |= w25q64.write_read_data(&dummy, sizeof(dummy));
	*id_h <<= 8;
	*id_h |= w25q64.write_read_data(&dummy, sizeof(dummy));
	
	*id_l = w25q64.write_read_data(&dummy, sizeof(dummy));
	*id_l <<= 8;
	*id_l |= w25q64.write_read_data(&dummy, sizeof(dummy));
	*id_l <<= 8;
	*id_l |= w25q64.write_read_data(&dummy, sizeof(dummy));
	*id_l <<= 8;
	*id_l |= w25q64.write_read_data(&dummy, sizeof(dummy));
	w25q64.write_cs_pin(1);
}
