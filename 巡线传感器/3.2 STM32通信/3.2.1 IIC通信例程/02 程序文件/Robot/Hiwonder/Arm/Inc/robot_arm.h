#ifndef __ROBOT_ARM_H__
#define __ROBOT_ARM_H__
/**
 * @file robot_arm.h
 * @author Min
 * @brief 机械臂控制实现
 * @version 1.0
 * @date 2025-02-22
 *
 * @copyright Copyright (c) 2025 Hiwonder
 *
 */
#include "stdint.h"
#include "stdbool.h"
#include "serial_servo.h"

#define MAX_SERVOS_NUM					  	 			6

/* 数据存放在Flash中的起始基地址 */
#define LOGO_BASE_ADDRESS          		   0UL  /* 该基地址用于存放标识LOGO */
#define SERVOS_OFFSET_BASE_ADDRESS 	 	  4096UL  /* 该基地址用于存放每个舵机的偏差 */
#define ACTION_FRAME_SUM_BASE_ADDRESS 	  8192UL  /* 该基地址用于存放每个动作组有多少动作 */
#define ACTION_GROUP_BASE_ADDRESS 		 12288UL	/* 该基地址用于存放下载的动作组文件 */

#define ACTION_FRAME_SIZE					21  /* 一个动作帧占21个字节 */ 
#define ACTION_GROUP_SIZE			 	  8192  /* 1个动作组留8KB内存空间 */
#define ACTION_GROUP_MAX_NUM               255  /* 默认最多存放255个动作组 */

/* 获得A的低八位 */
#define GET_LOW_BYTE(A) ((uint8_t)(A))
/* 获得A的高八位 */
#define GET_HIGH_BYTE(A) ((uint8_t)((A) >> 8))
/* 将高低八位合成为十六位 */
#define MERGE_HL(A, B) ((((uint16_t)(A)) << 8) | (uint8_t)(B))

typedef enum
{
	ACTION_FRAME_START = 0,
	ACTION_FRAME_RUNNING,
	ACTION_FRAME_IDLE
}ActionFrameStatusTypeDef;

typedef enum
{
	ACTION_GROUP_START = 0,
	ACTION_GROUP_RUNNING,
	ACTION_GROUP_END_PERIOD,
	ACTION_GROUP_IDLE
}ActionGroupStatusTypeDef;

typedef struct
{
	uint8_t 				 index;				/* 当前动作帧编号 */
	uint32_t 				 time;				/* 当前动作帧的运行时间 */
	uint8_t					 status;			/* 当前运行标志 */
}ActionFrameHandleTypeDef;

typedef struct
{
	/* 用于保存已有的动作组中各自的总动作帧数量 */
	uint8_t _sum[ACTION_GROUP_MAX_NUM];
	
	uint8_t 				 index;				/* 当前动作组的编号 */
	uint8_t					 sum;				/* 当前动作组的动作帧总数 */
	uint8_t					 running_times;		/* 当前动作组的运行次数 */
	uint8_t					 status;			/* 当前运行标志 */
	uint32_t				 time;				/* 当前动作组的运行时间 */

	ActionFrameHandleTypeDef frame;

}ActionGroupHandleTypeDef;

typedef struct
{
	uint8_t 				 cmd;
	int8_t		 servo_offset[6];
	ActionGroupHandleTypeDef action_group;

}RobotArmHandleTypeDef;

/**
 * @brief 初始化
 */
bool robot_arm_init(void);

/**
* @brief 舵机控制
 * 
 * @param  id 			关节id号
 * @param  target_duty	总线舵机控制范围为[0,1000]   
 * @param  time			运行时间
 */
void robot_arm_knot_run(uint8_t id, int target_duty, uint32_t time);

/**
* @brief 舵机停止
 * 
 * @param  id 	舵机id号
 */
void robot_arm_knot_stop(uint8_t id);

/**
 * @brief 舵机运行标志
 * 
 * @param  id 				舵机id号
 * @param  target_duty		目标位置
 * @return true  -运行完成
 *		   false -运行未完成
 */
bool robot_arm_knot_is_finish(uint8_t id, int target_duty);

/**
* @brief 获取舵机当前位置
 * 
 * @param  id 			舵机id号
 * @return 位置值
 */
int robot_arm_get_knot_current_duty(uint8_t id);			 
							
/**
 * @brief 动作组复位
 * @attention 每次运行完一次动作组都要调用此函数
 */	
void action_group_reset(void);

/**
 * @brief 动作组停止运行
 */	
void action_group_stop(void);

/**
 * @brief 擦除全部动作组
 */	
void action_group_erase(void);

/**
 * @brief 动作组运行
 * 
 * @param  action_group_index 	动作组编号
 * @param  repeat_times			重复运行次数
 */
bool action_group_run(uint8_t action_group_index, uint8_t repeat_times);
								 
/**
 * @brief 动作组数据写入接口
 * 
 * @param  self
 * @param  action_group_index 	动作组编号
 * @param  frame_num			该动作组的动作帧总数
 * @param  frame_index			写入的动作帧是第几帧，取值范围[0,255]
 * @param  pdata				帧数据指针
 * @param  size					帧数据长度
 */
void action_group_save(uint8_t action_group_number, 
					   uint8_t frame_num,
					   uint8_t frame_index,
					   uint8_t* pdata,
					   uint16_t size);

/**
 * @brief 重映射函数
 */
float map(float x, float in_min, float in_max, float out_min, float out_max);
#endif
