#include "global.h"
#include "robot_arm.h"
#include "w25q64.h"
#include "stdlib.h"
#include "math.h"

RobotArmHandleTypeDef robot_arm;	

static void serial_servos_delay(uint32_t ms)
{
	HAL_Delay(ms);
}


float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return out_min + (x - in_min) * ((out_max - out_min) / (in_max - in_min));
}

void robot_arm_offset_read(int8_t* value)
{
	uint8_t read_value[6];
	for(uint8_t i = 0; i < 6; i++)
	{
		while(!serial_servo_read_deviation(&serial_servo_controller, i + 1, &value[i]))
		{
			serial_servos_delay(2);
		}
		robot_arm.servo_offset[i] = value[i];
		serial_servos_delay(2);
	}
}

void robot_arm_knot_run(uint8_t id, int target_duty, uint32_t time)
{
	id = id > 6 ? 6 : id < 1 ? 1 : id;
	
	serial_servo_set_position(&serial_servo_controller, id, (int)target_duty, time);
	serial_servos_delay(1);

}

void robot_arm_knot_stop(uint8_t id)
{
	id = id > 6 ? 6 : id < 1 ? 1 : id;
	
	serial_servo_stop(&serial_servo_controller, id);
	serial_servos_delay(1);
}

bool robot_arm_knot_is_finish(uint8_t id, int target_duty)
{
	int position;
	
	id = id > 6 ? 6 : id < 1 ? 1 : id;
	

	while(!serial_servo_read_position(&serial_servo_controller, id, &position))
	{
		serial_servos_delay(1);
	}
	serial_servos_delay(1);
	return abs(target_duty - position) < 10 ? true : false;
			
}

int robot_arm_get_knot_current_duty(uint8_t id)
{
	int position;
	
	while(!serial_servo_read_position(&serial_servo_controller, id, &position))
	{
		serial_servos_delay(1);
	}
	serial_servos_delay(1);
	return position;
}

void action_group_erase()
{
	/* 将所有动作组的动作帧数量设置为0，即代表将所有动作组擦除 */
    memset(robot_arm.action_group._sum, 0, sizeof(robot_arm.action_group._sum));
    w25q64_erase_sector(ACTION_FRAME_SUM_BASE_ADDRESS);
    w25q64_write(ACTION_FRAME_SUM_BASE_ADDRESS,
				 (const uint8_t*)robot_arm.action_group._sum,
				 sizeof(robot_arm.action_group._sum));
}

//uint8_t read_frame[38][ACTION_FRAME_SIZE] = {0};
static bool robot_arm_flash_init()
{
	uint8_t read_logo[9] = {0};
	uint8_t offset_val[6] = {0};
	
	const uint8_t logo[] = "Hiwonder";
	
	w25q64_init();
	w25q64_read(LOGO_BASE_ADDRESS, read_logo, sizeof(read_logo));

	for (uint8_t i = 0; i < sizeof(read_logo); i++)
	{
		if (read_logo[i] != logo[i])
		{
			w25q64_erase_sector(LOGO_BASE_ADDRESS);				 
			w25q64_write(LOGO_BASE_ADDRESS, logo, sizeof(logo));
					 
			w25q64_erase_sector(SERVOS_OFFSET_BASE_ADDRESS);	
			w25q64_write(SERVOS_OFFSET_BASE_ADDRESS, offset_val, sizeof(offset_val));
					 				 
			action_group_erase();
		}
	}
	
	w25q64_read(LOGO_BASE_ADDRESS, read_logo, sizeof(read_logo));
	for (uint8_t i = 0; i < sizeof(read_logo); i++)
	{
		if (read_logo[i] != logo[i])
		{
			return false;

		}
	}
	return true;
//	w25q64_read(ACTION_GROUP_BASE_ADDRESS + 0 * ACTION_GROUP_SIZE + i * ACTION_FRAME_SIZE, read_frame[i], ACTION_FRAME_SIZE);
}

/* 一个的动作帧的数组内容
 * 控制的舵机数量：frame[0]
 * 运行时间：frame[1] + frame[2] << 8
 * 舵机id：frame[3 + i * 3]:
 * 舵机脉宽：frame[4 + i * 3] + frame[5 + i * 3] << 8
 * 0-动作帧运行失败 1-动作帧运行完成     0100 1101 0001
 */


static uint8_t action_frame_run(uint8_t action_group_index, uint8_t frame_index)
{
	uint8_t control_servos_sum;
	
	float map_duty;
	
	uint32_t ag_addr_offset;
	uint32_t af_addr_offset;

	static uint8_t finish_state = 0;
	static uint8_t set_id[6] = {0};
	static uint16_t set_duty[6] = {0};
	uint8_t frame[ACTION_FRAME_SIZE];
	
	switch(robot_arm.action_group.frame.status)
	{
		case ACTION_FRAME_START:
			ag_addr_offset = action_group_index * ACTION_GROUP_SIZE;
			af_addr_offset = frame_index * ACTION_FRAME_SIZE;
			w25q64_read(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset,
						frame, sizeof(frame));
			control_servos_sum = frame[0];

			if (control_servos_sum > MAX_SERVOS_NUM)
			{
				return ACTION_FRAME_START;
			}
			robot_arm.action_group.frame.time = MERGE_HL(frame[2], frame[1]);

			for (uint8_t i = 0; i < control_servos_sum; i++)
			{
				set_id[i] = frame[3 + i * 3];
				set_duty[i] = (uint16_t)MERGE_HL(frame[5 + i * 3], frame[4 + i * 3]);
				robot_arm_knot_run(set_id[i], (int)set_duty[i], robot_arm.action_group.frame.time);
			}

			robot_arm.action_group.frame.status = ACTION_FRAME_RUNNING;
			break;
		
		case ACTION_FRAME_RUNNING:
			HAL_Delay(robot_arm.action_group.frame.time);
			robot_arm.action_group.frame.index++;
			robot_arm.action_group.frame.status = ACTION_FRAME_IDLE;
			break;
		
		case ACTION_FRAME_IDLE:
			break;
		
		default:
			break;
	}
	return robot_arm.action_group.frame.status;
}


bool action_group_run(uint8_t action_group_index, uint8_t running_times)
{

	bool state = false;
	
	switch (robot_arm.action_group.status)
	{
		case ACTION_GROUP_START:
			robot_arm.action_group.running_times = running_times;
			robot_arm.action_group.index = action_group_index;
			robot_arm.action_group.frame.index = 0;
			w25q64_read(ACTION_FRAME_SUM_BASE_ADDRESS + action_group_index,
						&robot_arm.action_group.sum, 1);
			/* 如果该动作组的动作帧数量大于0，则说明已经下载过动作 */
			robot_arm.action_group.status = robot_arm.action_group.sum > 0 ? \
											ACTION_GROUP_RUNNING : ACTION_GROUP_IDLE;
			break;

		case ACTION_GROUP_RUNNING:
			if(action_frame_run(robot_arm.action_group.index, robot_arm.action_group.frame.index) == ACTION_FRAME_IDLE)	
			{
				if (robot_arm.action_group.frame.index == robot_arm.action_group.sum)
				{
					robot_arm.action_group.status = ACTION_GROUP_END_PERIOD;
					robot_arm.action_group.frame.index = 0;
				}
				else
				{
					robot_arm.action_group.frame.status = ACTION_FRAME_START;
				}
			}	
			/* code */
			break;

		case ACTION_GROUP_END_PERIOD:
			if(robot_arm.action_group.running_times == 1)
			{
				robot_arm.action_group.status = ACTION_GROUP_IDLE;
			}
			else
			{
				--robot_arm.action_group.running_times;
				robot_arm.action_group.status = ACTION_GROUP_RUNNING;
			}
			break;

		case ACTION_GROUP_IDLE:
			state = true;
			break;
		
		default:
			break;
	}
	return state;
}

void action_group_reset()
{
	robot_arm.action_group.status = ACTION_GROUP_START;
	robot_arm.action_group.frame.status = ACTION_FRAME_START;
}

void action_group_stop()
{
	for (uint8_t i = 0; i < MAX_SERVOS_NUM; i++)
	{
		robot_arm_knot_stop(i);
	}
	robot_arm.action_group.status = ACTION_GROUP_IDLE;
	robot_arm.action_group.frame.status = ACTION_FRAME_START;
}

//uint8_t write_frame[38][ACTION_FRAME_SIZE] = {0};
//uint8_t read_frame[38][ACTION_FRAME_SIZE] = {0};
void action_group_save(uint8_t action_group_index, 
					   uint8_t frame_num,
					   uint8_t frame_index,
					   uint8_t* pdata,
					   uint16_t size)
{
	uint32_t ag_addr_offset;
	uint32_t af_addr_offset;
	uint32_t page_offset;
	uint32_t write_addr;
	uint16_t remaining_space;
	
	robot_arm.action_group.index = action_group_index;
	robot_arm.action_group.frame.index = frame_index;
	ag_addr_offset = action_group_index * ACTION_GROUP_SIZE;
	af_addr_offset = frame_index * ACTION_FRAME_SIZE;
	
	page_offset = af_addr_offset % 256; // 正确计算页偏移
	write_addr = ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset;
	remaining_space = 256 - page_offset;
	
    /* 擦除动作组空间（首帧） */
    if (frame_index == 0) {
        for (uint8_t i = 0; i < 2; i++) {
            w25q64_erase_sector(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + (i * 4096));
        }
    }

    /* 跨页写入处理 */
    if (remaining_space < ACTION_FRAME_SIZE) {
        w25q64_write(write_addr, pdata, remaining_space);
        w25q64_write(write_addr + remaining_space, pdata + remaining_space, ACTION_FRAME_SIZE - remaining_space);
    } 
		else 
		{
        w25q64_write(write_addr, pdata, ACTION_FRAME_SIZE);
    }

//	memcpy(write_frame[frame_index], pdata, size);
//	w25q64_read(ACTION_GROUP_BASE_ADDRESS + ag_addr_offset + af_addr_offset, read_frame[frame_index], sizeof(read_frame[frame_index]));
	
	if ((robot_arm.action_group.frame.index + 1) == frame_num)
	{
		/* 如果写入的是最后一帧，此时需要更新一下flash中对应动作组的动作帧总数 */
		w25q64_read(ACTION_FRAME_SUM_BASE_ADDRESS, robot_arm.action_group._sum, sizeof(robot_arm.action_group._sum));
		
		robot_arm.action_group._sum[robot_arm.action_group.index] = frame_num;
		w25q64_erase_sector(ACTION_FRAME_SUM_BASE_ADDRESS);
		w25q64_write(ACTION_FRAME_SUM_BASE_ADDRESS, (const uint8_t*)robot_arm.action_group._sum, sizeof(robot_arm.action_group._sum));
	}
}

bool robot_arm_init(void)
{

	serial_servo_init();

	memset(&robot_arm, 0, sizeof(RobotArmHandleTypeDef));
	if(robot_arm_flash_init() == false)
	{
		return false;
	}
	HAL_Delay(200);
	return true;
}
