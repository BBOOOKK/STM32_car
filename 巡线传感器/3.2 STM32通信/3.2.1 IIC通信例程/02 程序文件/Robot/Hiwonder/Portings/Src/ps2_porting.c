#include "ps2_porting.h"
#include "robot_arm.h"
#include "usart.h"
#include "buzzer.h"
#include "math.h"

PS2HandleTypeDef ps2;

static bool stand_state = false;

static void packet_uart_error_callblack(UART_HandleTypeDef *huart);
static void packet_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length);

static void receive_data(uint8_t* pdata, uint16_t size)
{
	HAL_UART_AbortReceive(&huart3);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart3, pdata, size);
}

static void packet_start_receive()
{
	HAL_UART_RegisterCallback(&huart3, HAL_UART_ERROR_CB_ID, packet_uart_error_callblack);
	HAL_UART_RegisterRxEventCallback(&huart3, packet_dma_receive_event_callback);
	ps2.receive_data(ps2.rx_dma_buf, sizeof(ps2.rx_dma_buf));
}

static void packet_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length)
{
	lwrb_reset(&ps2.rb);  //清空缓冲区数据
	lwrb_write(&ps2.rb, ps2.rx_dma_buf, length);
	ps2.receive_data(ps2.rx_dma_buf, sizeof(ps2.rx_dma_buf));
}

static void packet_uart_error_callblack(UART_HandleTypeDef *huart)
{
	packet_start_receive();
}

static void ps2_run(PS2HandleTypeDef* self)
{
	
	if(self->keyvalue.bit_up)
		{
			while(1)
			{
				if(action_group_run(1, 1))    //go forward
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}  
		}else if(self->keyvalue.bit_down)
		{
			while(1)
			{
				if(action_group_run(2, 1))    //go back
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}
		}else if(self->keyvalue.bit_left)
		{
			while(1)
			{
				if(action_group_run(3, 1))    //turn left
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}
		}else if(self->keyvalue.bit_right)
		{	
			while(1)
			{
				if(action_group_run(4, 1))    //turn right
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}
		}else if(self->keyvalue.bit_start)
		{
			while(1)
			{
				if(action_group_run(0, 1))    //stand
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.bit_triangle)
		{
			while(1)
			{
				if(action_group_run(7, 1))
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.bit_cross)
		{
			while(1)
			{
				if(action_group_run(8, 1))
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.bit_square)
		{
			while(1)
			{
				if(action_group_run(5, 1))
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.bit_circle)
		{
			while(1)
			{
				if(action_group_run(6, 1))
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.bit_r1)
		{
      while(1)
			{
				if(action_group_run(11, 1))
				{
					action_group_reset();
					break;
				}
			}
		}
		else if(self->keyvalue.bit_r2)
		{	
			while(1)
			{
				if(action_group_run(12, 1))
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.bit_l1)
		{
			while(1)
			{
				if(action_group_run(9, 1))
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.bit_l2)
		{
			while(1)
			{
				if(action_group_run(10, 1))
				{
					action_group_reset();
					break;
				}
			}
		}else if(self->keyvalue.left_joystick_y < 70)
		{
			while(1)
			{
				if(action_group_run(1, 1))    //go forward
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}
		}else if(self->keyvalue.left_joystick_y > 180)
		{
			while(1)
			{
				if(action_group_run(2, 1))    //go back
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}
		}else if(self->keyvalue.left_joystick_x < 70)
		{
			while(1)
			{
				if(action_group_run(3, 1))    //turn left
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}
		}else if(self->keyvalue.left_joystick_x > 180)
		{
			while(1)
			{
				if(action_group_run(4, 1))    //turn right
				{
					action_group_reset();
					stand_state = false;
					break;
				}
			}
		}
		else if((self->keyvalue.left_joystick_x - 127) < 5 && (self->keyvalue.left_joystick_y - 127) < 5){
			if(!stand_state){
				while(1)
				{
					if(action_group_run(0, 1))
					{
						action_group_reset();
						stand_state = true;
						break;
					}
				}
			}
		}
}


void ps2_init()
{
	memset(&ps2, 0, sizeof(PS2HandleTypeDef));
	ps2.action_running_time = 1000;
	ps2.receive_data = receive_data;
	lwrb_init(&ps2.rb, ps2.rx_fifo, sizeof(ps2.rx_fifo));
	packet_start_receive();	
}


static void unpack(PS2HandleTypeDef* self)
{
	uint8_t i;

	uint32_t readlen = 0;
	uint32_t available = 0;
	
	static uint8_t buffer_index = 0;	
	static uint8_t rec_data[PS2_PACKET_LENGTH] = {0};

	available = lwrb_get_full(&self->rb);
	if (available != 0)
	{
		available = available > PS2_PACKET_LENGTH ? PS2_PACKET_LENGTH: available;
 		if(self->packet_status == PS2_PACKET_HEADER_1)		/* Step1: 找帧头 */
		{
			self->unpack_status = UNPACK_FINISH;
			readlen = lwrb_read(&self->rb, rec_data, 1);
			self->packet_status = rec_data[0] == PS2_PACKET_HEADER ? PS2_PACKET_HEADER_2: PS2_PACKET_HEADER_1;
			self->packet.packet_header[0] = rec_data[0];			
		}
		
		if (self->packet_status == PS2_PACKET_HEADER_2)			
		{
			readlen = lwrb_read(&self->rb, rec_data, 2);
			for(i = 0; i < readlen; i++)
			{
				switch(self->packet_status)
				{
					case PS2_PACKET_HEADER_2:
						self->packet_status = rec_data[i] == PS2_PACKET_HEADER ? PS2_PACKET_DATA_LENGTH: PS2_PACKET_HEADER_1;
						self->packet.packet_header[1] = rec_data[i];
						break;		
					
					case PS2_PACKET_DATA_LENGTH:			/* Step2:获取一帧的长度信息 */	
						self->packet_status = rec_data[i] != 0 ? PS2_PACKET_DATA: PS2_PACKET_HEADER_1;
						self->packet.data_len = rec_data[i];
						buffer_index = 0;
						break;			
					
					default:
						self->packet_status = PS2_PACKET_HEADER_1;
						break;						
				}
			}
		}
		
		if (self->packet_status == PS2_PACKET_DATA)			/* Step3:获取一帧的数据信息 */
		{
			readlen = lwrb_read(&self->rb, rec_data, self->packet.data_len - 1);
			for(i = 0; i < readlen; i++)
			{
				self->packet.buffer[buffer_index] = rec_data[i];
				buffer_index++;
				if(buffer_index == self->packet.data_len - 2)
				{
					self->packet_status = PS2_PACKET_HEADER_1;
					self->keyvalue.buffer0 = self->packet.buffer[2];
					self->keyvalue.buffer1 = self->packet.buffer[3];
					if(self->keyvalue.bit_mode == 1 && self->last_keyvalue.bit_mode == 0)
					{
						buzzer_toggle(50,50,1);
						self->mode_button_status = !self->mode_button_status; 
					}
					
					self->mode = self->mode_button_status == 1 ? PS2_SINGLE_SERVO_MODE : PS2_COORDINATE_MODE;
				}		
			}			
		}
		switch(self->mode)
		{
			case PS2_SINGLE_SERVO_MODE:
				self->keyvalue.left_joystick_x = self->packet.buffer[5];
				self->keyvalue.left_joystick_y = self->packet.buffer[6];
				self->keyvalue.right_joystick_x = self->packet.buffer[7];
				self->keyvalue.right_joystick_y = self->packet.buffer[8];
			
				self->keyvalue.bit_left = self->packet.buffer[5] == 0x00 ? 1 : 0;
				self->keyvalue.bit_right = self->packet.buffer[5] == 0xFF ? 1 : 0;
				self->keyvalue.bit_up = self->packet.buffer[6] == 0x00 ? 1 : 0;
				self->keyvalue.bit_down = self->packet.buffer[6] == 0xFF ? 1 : 0;
				break;
			
			case PS2_COORDINATE_MODE:
				self->keyvalue.left_joystick_x = self->packet.buffer[5];
				self->keyvalue.left_joystick_y = self->packet.buffer[6];
				self->keyvalue.right_joystick_x = self->packet.buffer[7];
				self->keyvalue.right_joystick_y = self->packet.buffer[8];

				switch(self->packet.buffer[4])
				{
					case 0x00:
						self->keyvalue.bit_up = 1;
						break;
					
					case 0x01:
						self->keyvalue.bit_up = 1;
						self->keyvalue.bit_right = 1;
						break;
					
					case 0x02:
						self->keyvalue.bit_right = 1;
						break;
					
					case 0x03:
						self->keyvalue.bit_down = 1;
						self->keyvalue.bit_right = 1;
						break;
					
					case 0x04:
						self->keyvalue.bit_down = 1;
						break;
					
					case 0x05:
						self->keyvalue.bit_down = 1;
						self->keyvalue.bit_left = 1;
						break;
					
					case 0x06:
						self->keyvalue.bit_left = 1;
						break;
					
					case 0x07:
						self->keyvalue.bit_up = 1;
						self->keyvalue.bit_left = 1;
						break;
					
					case 0x0F:
						self->keyvalue.bit_up = 0;
						self->keyvalue.bit_down = 0;
						self->keyvalue.bit_left = 0;
						self->keyvalue.bit_right = 0;
						break;
				}
				break;
				
			default:
				break;
		}
	}
}

void ps2_handler()
{
	unpack(&ps2);
	ps2_run(&ps2);
	ps2.last_keyvalue = ps2.keyvalue;
}

void stop_ps2_receive()
{
	HAL_UART_DMAStop(&huart3);
}

void start_ps2_receive()
{
	ps2.receive_data(ps2.rx_dma_buf, sizeof(ps2.rx_dma_buf));
}

