#include "app_porting.h"
#include "usart.h"
#include "string.h"
#include "global.h"
#include "buzzer.h"
#include "adc_sample.h"

static uint8_t rx_dma_buf[64];
static uint8_t rx_fifo[256];
static uint32_t tickstart;

AppHandleTypeDef app;

static void packet_uart_error_callblack(UART_HandleTypeDef *huart);
static void packet_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length);

void bluetooth_switch_write_pin(uint8_t new_state)
{
	HAL_GPIO_WritePin(BT_EN_GPIO_Port, BT_EN_Pin, (GPIO_PinState)new_state);
}

static void receive_data(uint8_t* pdata, uint16_t size)
{
	HAL_UART_AbortReceive(&huart1);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, pdata, size);
}

static uint8_t transmit_data(const uint8_t* pdata, uint16_t size)
{
	return (uint8_t)HAL_UART_Transmit(&huart1, pdata, size, 100);
}

static void packet_transmit(AppHandleTypeDef* self, AppFunctionStatus cmd, uint8_t* pdata, uint8_t size)
{
	uint8_t data[30];
	
	if(size == 0)
	{
		data[0] = APP_PACKET_HEADER;
		data[1] = APP_PACKET_HEADER;
		data[2] = 2;
		data[3] = cmd;
		self->transmit_data(data, 4);
	}
	else
	{
		data[0] = APP_PACKET_HEADER;
		data[1] = APP_PACKET_HEADER;
		data[2] = 2 + size;
		data[3] = cmd;		
		for(uint8_t i = 0; i < size; i++)
		{
			data[4 + i] = pdata[i];
		}
		self->transmit_data(data, size + 4);
	}
}

static void packet_start_receive()
{
	HAL_UART_RegisterCallback(&huart1, HAL_UART_ERROR_CB_ID, packet_uart_error_callblack);
	HAL_UART_RegisterRxEventCallback(&huart1, packet_dma_receive_event_callback);
	app.receive_data(rx_dma_buf, sizeof(rx_dma_buf));
}

//uint8_t data[MAX_PACKET_LENGTH];	
//size_t readlen = 0;
static void unpack(AppHandleTypeDef* self)
{
	uint8_t data[MAX_PACKET_LENGTH];	
	size_t readlen = 0;
	size_t available;
	static uint8_t buffer_rec_finish = 0;
	
	available = lwrb_get_full(&self->rb);
	
	available = available > MAX_PACKET_LENGTH ? MAX_PACKET_LENGTH: available;
		
	while(available > 0)
	{
		switch(self->packet_status)
		{
			case PACKET_HEADER_1:
				lwrb_read(&self->rb, data, 1);
				self->packet_status = data[0] == APP_PACKET_HEADER ? PACKET_HEADER_2: PACKET_HEADER_1;
				if(data[0] == APP_PACKET_HEADER)
				{
					self->packet.packet_header[0] = data[0];
				}
				break;
			
			case PACKET_HEADER_2:
				lwrb_read(&self->rb, data, 1);
				self->packet_status = data[0] == APP_PACKET_HEADER ? PACKET_DATA_LENGTH: PACKET_HEADER_1;
				if(data[0] == APP_PACKET_HEADER)
				{
					self->packet.packet_header[1] = data[0];
				}					
				break;
			
			case PACKET_DATA_LENGTH:
				lwrb_read(&self->rb, data, 1);
				self->packet.data_len = data[0];
				
				if(self->packet.data_len < 2)
				{
					self->packet_status = PACKET_HEADER_1;
					self->packet.data_len = data[0];
				}
				else
				{
					self->packet_status = PACKET_FUNCTION;
					
				}
				break;
				
			case PACKET_FUNCTION:
				lwrb_read(&self->rb, data, 1);
				self->packet_status = data[0] < CMD_FUNC_NULL ? PACKET_DATA : PACKET_HEADER_1;
				self->packet.cmd = data[0];	
				
				break;

			case PACKET_DATA:
				readlen = lwrb_read(&self->rb, data, self->packet.data_len - 2);
				for(uint8_t i = 0; i < readlen; i++)
				{
					self->packet.buffer[i] = data[i];
				}
				buffer_rec_finish = 1;		
				break;			
				
			default:
				self->packet_status = PACKET_HEADER_1;
				break;		
		}
		
		switch(self->packet.cmd)
		{
			case 1:
				self->status = CMD_VERSION_QUERY;
				break;
			
			case 2:
				self->status = CMD_SERVO_OFFSET_READ;
				break;
			
			case 3:
				self->status = CMD_MULT_SERVO_MOVE;
				break;

			case 4:
				app.status = CMD_COORDINATE_SET;
				break;
			
			case 6:
				self->status = CMD_ACTION_GROUP_RUN;
				action_group_reset();
				break;
			
			case 7:
				self->status = CMD_FULL_ACTION_STOP;
				break;
			
			case 8:
				self->status = CMD_FULL_ACTION_ERASE;
				break;

			case 9:
				self->status = CMD_CHASSIS_CONTROL;
				tickstart = HAL_GetTick();
				break;

			case 10:
				self->status = CMD_SERVO_OFFSET_SET;
				break;

			case 11:
				self->status = CMD_SERVO_OFFSET_DOWNLOAD;
				break;		

			case 12:
				self->status = CMD_SERVOS_RESET;
				break;	
			
			case 13:
				self->status = CMD_ANGLE_BACK_READING;
				break;
			
			case 25:
				self->status = CMD_ACTION_DOWNLOAD;
				break;

			default:
				self->status = CMD_FUNC_NULL;
				break;	
		}		
		
		if(buffer_rec_finish == 1)
		{
			buffer_rec_finish = 0;
			self->packet_status = PACKET_HEADER_1;
			break;
		}
		
		available = lwrb_get_full(&self->rb);
	}
	

}

static void packet_uart_error_callblack(UART_HandleTypeDef *huart)
{
	packet_start_receive();
}

static void packet_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length)
{
	lwrb_write(&app.rb, rx_dma_buf, length);
	app.receive_data(rx_dma_buf, sizeof(rx_dma_buf));
}

void app_init(void)
{
	memset(&app, 0, sizeof(AppHandleTypeDef));
	app.receive_data = receive_data;
	app.transmit_data = transmit_data;
	lwrb_init(&app.rb, rx_fifo, sizeof(rx_fifo));
	packet_start_receive();
	app.running_time = 1000;
}

	int8_t read_val[6] = {0};
/* header header len cmd count time [id duty] */
void app_handler()
{
	uint8_t send_data[20] = {0};
	
	unpack(&app);

    switch (app.status)
    {
		case CMD_MULT_SERVO_MOVE:
				app.servos_count = app.packet.buffer[0];
				app.running_time = MERGE_HL(app.packet.buffer[2], app.packet.buffer[1]);

				for (uint8_t i = 0; i < app.servos_count; i++)
				{
					app.set_id = app.packet.buffer[3 + i * 3];
					app.set_duty = MERGE_HL(app.packet.buffer[5 + i * 3], app.packet.buffer[4 + i * 3]);
					robot_arm_knot_run(app.set_id, app.set_duty, app.running_time);
				}
				app.status = CMD_FUNC_NULL;
				break;

		
		case CMD_ACTION_GROUP_RUN:
				app.packet.action_group_index = app.packet.buffer[0];
				app.packet.running_times = MERGE_HL(app.packet.buffer[2], app.packet.buffer[1]);
				if(action_group_run(app.packet.action_group_index, app.packet.running_times))
				{
					app.status = CMD_FUNC_NULL;
				}
				break;

		case CMD_FULL_ACTION_STOP:
				action_group_stop();
				app.status = CMD_FUNC_NULL;
				break;

		case CMD_FULL_ACTION_ERASE:
				action_group_erase();
				HAL_Delay(5);
				packet_transmit(&app, CMD_FULL_ACTION_ERASE, send_data, 0);
				app.status = CMD_FUNC_NULL;
				break;    

		case CMD_ACTION_DOWNLOAD:
				app.packet.action_group_index = app.packet.buffer[0];
				app.packet.action_frame_sum = app.packet.buffer[1];
				app.packet.action_frame_index = app.packet.buffer[2];
				action_group_save(app.packet.action_group_index, 
													app.packet.action_frame_sum, 
													app.packet.action_frame_index, 
													app.packet.buffer + 3,
													ACTION_FRAME_SIZE);
				HAL_Delay(5);
				packet_transmit(&app, CMD_ACTION_DOWNLOAD, send_data, 0);
				app.status = CMD_FUNC_NULL;
				if(app.packet.action_frame_index == app.packet.action_frame_sum - 1)
				{
					buzzer_toggle(100, 100, 1);
				}
				break;
		
		default:
				break;
    }
}

void stop_app_receive()
{
	HAL_UART_DMAStop(&huart1);
}

void start_app_receive()
{
	app.receive_data(rx_dma_buf, sizeof(rx_dma_buf));
}