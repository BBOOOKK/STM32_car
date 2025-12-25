#ifndef __PS2_PORTING_H__
#define __PS2_PORTING_H__

#include "stdint.h"
#include "global.h"
#include "lwrb.h"

#define PS2_PACKET_HEADER						0x55
#define PS2_PACKET_LENGTH						0x0A
#define MAX_PS2_RB_BUFFER_LENGTH			  	  64

typedef enum
{
	PS2_SINGLE_SERVO_MODE = 1,
	PS2_COORDINATE_MODE
}PS2ModeStatusTypeDef;

typedef enum
{
	UNPACK_FINISH = 0,
	UNPACK_START
}PackStatusTypeDef;

typedef enum
{
	PS2_PACKET_HEADER_1 = 0,
	PS2_PACKET_HEADER_2,
	PS2_PACKET_DATA_LENGTH,
	PS2_PACKET_DATA
}PS2PackStatusTypeDef;

typedef struct
{	
	uint8_t packet_header[2];
	uint8_t data_len;	
	uint8_t buffer[PS2_PACKET_LENGTH - 1];
}PS2PacketObjectTypeDef;

#pragma pack(1)
typedef struct
{
	uint8_t bit_up;
	uint8_t bit_down;
	uint8_t bit_left;
	uint8_t bit_right;	
	uint8_t left_joystick_x;
	uint8_t left_joystick_y;
	uint8_t right_joystick_x;
	uint8_t right_joystick_y;

	union {
		uint8_t buffer0;
		struct {
			uint8_t bit_triangle : 				1;
			uint8_t bit_circle : 				1;
			uint8_t bit_cross : 				1;
			uint8_t bit_square : 				1;
			uint8_t bit_l1 : 					1;
			uint8_t bit_r1 : 					1;
			uint8_t bit_l2 : 					1;
			uint8_t bit_r2 : 					1;
		};
	};

	union {
		uint8_t buffer1;
		struct {
			uint8_t bit_select : 				1;
			uint8_t bit_start : 				1;
			uint8_t bit_leftjoystick_press : 	1;
			uint8_t bit_rightjoystick_press : 	1;
			uint8_t bit_mode : 					1;
			uint8_t unused_1 : 					1;
			uint8_t unused_2 : 					1;
			uint8_t unused_3:					1;
		};
	};
}PS2KeyValueObjectTypeDef;
#pragma pack()

typedef struct
{
	uint8_t rx_byte;
	uint8_t mode;
	uint8_t mode_button_status;
	uint8_t reset_status;
	uint8_t unpack_status;

	uint8_t rx_dma_buf[128];
	uint8_t rx_fifo[256];
	
	uint32_t action_running_time;
	
	lwrb_t rb;
	PS2KeyValueObjectTypeDef keyvalue;
	PS2KeyValueObjectTypeDef last_keyvalue;
	PS2PacketObjectTypeDef packet;
	PS2PackStatusTypeDef packet_status;

	void (*receive_data)(uint8_t* pdata, uint16_t size);

}PS2HandleTypeDef;


/**
 * @brief PS2初始化
 *
 */
void ps2_init(void);

/**
 * @brief PS2句柄
 * 
 */
void ps2_handler(void);

/**
 * @brief 停止ps2数据接收
 * 
 */
void stop_ps2_receive();

/**
 * @brief 开启ps2数据接收
 * 
 */
void start_ps2_receive();
#endif
