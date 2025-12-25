#ifndef __APP_PORTING_H_
#define __APP_PORTING_H_

#include "lwrb.h"
#include "robot_arm.h"


#define APP_PACKET_HEADER                  0x55  /* 通信协议帧头 */
#define APP_TX_DATA_LENGTH                    2  /* 除去帧头的发送端数据长度 */
#define MAX_PACKET_LENGTH					 32

#define CHASSIS_CONTROL_TIMEOUT			    200

typedef enum
{
	PACKET_HEADER_1 = 0,
	PACKET_HEADER_2,
	PACKET_DATA_LENGTH,
	PACKET_FUNCTION,
	PACKET_DATA
}PacketAnalysisStatus;

typedef enum
{
	CMD_VERSION_QUERY = 1,
	CMD_SERVO_OFFSET_READ,
	CMD_MULT_SERVO_MOVE,
	CMD_COORDINATE_SET = 4,
	CMD_ACTION_GROUP_RUN = 6,
	CMD_FULL_ACTION_STOP,
	CMD_FULL_ACTION_ERASE,
	CMD_CHASSIS_CONTROL,
	CMD_SERVO_OFFSET_SET,	
	CMD_SERVO_OFFSET_DOWNLOAD,
	CMD_SERVOS_RESET,
	CMD_ANGLE_BACK_READING,
	CMD_ACTION_DOWNLOAD = 25,
	CMD_FUNC_NULL
}AppFunctionStatus;


#pragma pack(1)
typedef struct
{
	uint8_t action_frame_sum;
	uint8_t action_frame_index;
	uint8_t action_group_index;
	uint16_t running_times;
	
	uint8_t packet_header[2];
	uint8_t data_len;
	uint8_t cmd;	
	uint8_t buffer[MAX_PACKET_LENGTH - 4];
}PacketObjectTypeDef;
#pragma pack()

typedef struct
{	uint8_t set_id;
	uint8_t servos_count;
	uint16_t set_duty;
	uint16_t running_time;
	
	lwrb_t rb;
    PacketObjectTypeDef packet;
	PacketAnalysisStatus packet_status;
	AppFunctionStatus status;
	
	void (*receive_data)(uint8_t* pdata, uint16_t size);
    uint8_t (*transmit_data)(const uint8_t* pdata, uint16_t size);
}AppHandleTypeDef;


/**
 * @brief APP初始化
 * 
 */
void app_init(void);

/**
 * @brief APP句柄
 * 
 */
void app_handler(void);

/**
 * @brief APP&PC切换
 * @param new_state 0 -掉电
 *					1 -上电
 */
void bluetooth_switch_write_pin(uint8_t new_state);

/**
 * @brief 停止app数据接收
 * 
 */
void stop_app_receive(void);

/**
 * @brief 开启app数据接收
 * 
 */
void start_app_receive(void);
#endif
