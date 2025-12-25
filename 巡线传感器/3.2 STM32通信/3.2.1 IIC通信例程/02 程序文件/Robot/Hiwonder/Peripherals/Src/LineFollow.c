#include "LineFollow.h"
#include "i2c.h"
#include "usart.h"
#include "string.h"

LineFollowHandleTypeDef LineFollow;

static uint8_t write_data(LineFollowHandleTypeDef* self, uint8_t* pdata, uint16_t size)
{
	self->transmit_status = (uint8_t)HAL_I2C_Master_Transmit(&hi2c1, self->dev_addr << 1, pdata, size, 0xfff);
	return self->transmit_status;
}


static uint8_t read_data(LineFollowHandleTypeDef* self, uint8_t* pdata, uint16_t size)
{	
	/* 使用HAL_I2C_Master_Receive_DMA这个函数需要把i2c的事件中断勾上 */
	self->receive_status = (uint8_t)HAL_I2C_Master_Receive_DMA(&hi2c1, self->dev_addr << 1, pdata, size);
	return self->receive_status;	
}


static bool write_to_device(LineFollowHandleTypeDef* self, uint8_t reg, uint8_t* pdata, uint16_t size)
{
	uint8_t trans_data[size + 1];
	
	trans_data[0] = reg;
	
	for (uint16_t i = 0; i < size; i++)
	{
		trans_data[1 + i] = pdata[i];
	}
	
	if(write_data(self, trans_data, sizeof(trans_data)) != 0)
	{
		return false;
	}
	
	return true;
}

static bool receive_from_device(LineFollowHandleTypeDef* self, uint8_t reg, uint8_t* pdata, uint16_t size)
{
	uint8_t set_reg= reg;	
	
	if(write_data(self, &set_reg, 1) != 0)
	{
		return false;
	}
	
	if(read_data(self, pdata, size) != 0)
	{
		return false;
	}
	
	return true;
}

void LineFollowIIC_init()
{
	memset(&LineFollow, 0, sizeof(LineFollow));
	LineFollow.write_data = write_data;
	LineFollow.read_data = read_data;
	LineFollow.dev_addr = LineFollow_ADDRESS;
}

bool LineFollowIIC_State(LineFollowHandleTypeDef* State)
{
	if(receive_from_device(&LineFollow, LineFollow_STATE_REG, LineFollow.results, 1))
	{
		while(HAL_I2C_STATE_READY != HAL_I2C_GetState(&hi2c1));
		for(int i=0; i<sizeof(LineFollow.data);i++){
			State->data[i] = (LineFollow.results[0] >> i) & 0x01;
		}
		return true;
	}
	return false;
}

bool LineFollowIIC_Analog(LineFollowHandleTypeDef* Analog)
{
	uint8_t count = 0;
	if(receive_from_device(&LineFollow, LineFollow_ANALOG_REG, LineFollow.results, sizeof(LineFollow.results)))
	{
		while(HAL_I2C_STATE_READY != HAL_I2C_GetState(&hi2c1));
		for(int i=0; i<sizeof(LineFollow.data);i++){
			Analog->data[i] = LineFollow.results[count] | (LineFollow.results[count+1] << 8);
			count += 2;
		}
		return true;
	}
	return false;
}

bool LineFollowIIC_Threshold(LineFollowHandleTypeDef* Threshold){
	uint8_t count = 0;
	if(receive_from_device(&LineFollow, LineFollow_THRESHOLD_REG, LineFollow.results, sizeof(LineFollow.results)))
	{
		while(HAL_I2C_STATE_READY != HAL_I2C_GetState(&hi2c1));
		for(int i=0; i<sizeof(LineFollow.data);i++){
			Threshold->data[i] = LineFollow.results[count] | (LineFollow.results[count+1] << 8);
			count += 2;
		}
		return true;
	}
	return false;
}




static int8_t LineFollow_write_and_read(LineFollowHandleTypeDef *self, uint8_t cmd, bool tx_only)
{
  int8_t ret = -1;	
	switch(self->it_state)
	{
		case LINEFOLLOW_WRITE_DATA_READY:
			if(cmd == LINEFOLLOW_CMD_NULL && tx_only == false){
				self->tx_byte_index = 1;
				memcpy(&self->tx_frame, &cmd, sizeof(cmd));
				__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
			}else{
				memcpy(&self->tx_frame, &cmd, sizeof(cmd));
				self->tx_byte_index = 0;
				self->tx_only = tx_only;
				self->rx_state = LINEFOLLOW_RECV_STARTBYTE_1;
				__HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_RXNE);
				__HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_TC);
				__HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_TXE);
				
				__HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);
				__HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);
			}
		
			ret = 1;
			break;
		
		case LINEFOLLOW_READ_DATA_FINISH:
			self->it_state = LINEFOLLOW_WRITE_DATA_READY;
			ret = 0;
			break;
		
		case LINEFOLLOW_READ_DATA_ERROR:
			self->it_state = LINEFOLLOW_WRITE_DATA_READY;
			break;
			
		default:
			break;
	}
	return ret;
}



void LineFollowUART_init(uint8_t Mode)
{
		memset(&LineFollow, 0, sizeof(LineFollow));
    __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_TXE);
    __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_TC);
    __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_RXNE);
		LineFollow.work_mode = Mode;
		LineFollow_write_and_read(&LineFollow, LineFollow.work_mode, true);
		LineFollow.it_state = LINEFOLLOW_WRITE_DATA_READY;
}

bool LineFollowUART_State(LineFollowHandleTypeDef* State)
{
	if(LineFollow.work_mode == LINEFOLLOW_MODE_MANUAL)
	{
		LineFollow.manual_mode = LINEFOLLOW_MODE_MANUAL_STATE;
		if (0 == LineFollow_write_and_read(&LineFollow, LINEFOLLOW_MODE_MANUAL_STATE, false)) 
		{
			for(int i=0; i<sizeof(LineFollow.data);i++){
				State->data[i] = (LineFollow.results[0] >> i) & 0x01;
			}
			LineFollow.it_state = LINEFOLLOW_WRITE_DATA_READY;
			return true;
		}
	}
	else
	{
		if (0 == LineFollow_write_and_read(&LineFollow, LINEFOLLOW_CMD_NULL, false)) 
		{
			for(int i=0; i<sizeof(LineFollow.data);i++){
				State->data[i] = (LineFollow.results[0] >> i) & 0x01;
			}
			LineFollow.it_state = LINEFOLLOW_WRITE_DATA_READY;
			return true;
		}
	}
	
	return false;
}

bool LineFollowUART_Analog(LineFollowHandleTypeDef* Analog)
{
	uint8_t count = 0;
	if(LineFollow.work_mode == LINEFOLLOW_MODE_MANUAL)
	{
		LineFollow.manual_mode = LINEFOLLOW_MODE_MANUAL_ANALOG;
		if (0 == LineFollow_write_and_read(&LineFollow, LINEFOLLOW_MODE_MANUAL_ANALOG, false)) 
		{
			for(int i=0; i<sizeof(LineFollow.data);i++){
				Analog->data[i] = LineFollow.results[count] | (LineFollow.results[count+1] << 8);
				count += 2;
			}
			LineFollow.it_state = LINEFOLLOW_WRITE_DATA_READY;
			return true;
		}
	}
	else
	{
		if (0 == LineFollow_write_and_read(&LineFollow, LINEFOLLOW_CMD_NULL, false)) 
		{
			for(int i=0; i<sizeof(LineFollow.data);i++){
				Analog->data[i] = LineFollow.results[count] | (LineFollow.results[count+1] << 8);
				count += 2;
			}
			LineFollow.it_state = LINEFOLLOW_WRITE_DATA_READY;
			return true;
		}
	}
	
	return false;
}

bool LineFollowUART_Threshold(LineFollowHandleTypeDef* Threshold)
{
	uint8_t count = 0;
	if(LineFollow.work_mode == LINEFOLLOW_MODE_MANUAL)
	{
		LineFollow.manual_mode = LINEFOLLOW_MODE_MANUAL_THRESHOLD;
		if (0 == LineFollow_write_and_read(&LineFollow, LINEFOLLOW_MODE_MANUAL_THRESHOLD, false)) 
		{
			for(int i=0; i<sizeof(LineFollow.data);i++){
				Threshold->data[i] = LineFollow.results[count] | (LineFollow.results[count+1] << 8);
				count += 2;
			}
			LineFollow.it_state = LINEFOLLOW_WRITE_DATA_READY;
			return true;
		}
	}
	else
	{
		if (0 == LineFollow_write_and_read(&LineFollow, LINEFOLLOW_CMD_NULL, false)) 
		{
			for(int i=0; i<sizeof(LineFollow.data);i++){
				Threshold->data[i] = LineFollow.results[count] | (LineFollow.results[count+1] << 8);
				count += 2;
			}
			LineFollow.it_state = LINEFOLLOW_WRITE_DATA_READY;
			return true;
		}
	}
	
	return false;
}

