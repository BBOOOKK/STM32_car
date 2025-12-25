#include "buzzer.h"
#include "string.h"
#include "tim.h"

BuzzerHandleTypeDef buzzer;
static uint16_t period;

void buzzer_init()
{
	memset(&buzzer, 0, sizeof(BuzzerHandleTypeDef));
	period = 1000000 / BUZZER_FREQ;
	__HAL_TIM_SET_AUTORELOAD(&htim4, period);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void buzzer_on()
{
	buzzer.ticks_on = 1;
	buzzer.ticks_off = 0;
	buzzer.times = 0;
	buzzer.status = BUZZER_START_NEW_CYCLE;
}


void buzzer_off()
{
	buzzer.ticks_on = 0;
	buzzer.ticks_off = 0;
	buzzer.times = 0;
	buzzer.status = BUZZER_START_NEW_CYCLE;
}

void buzzer_toggle(uint32_t ticks_on, uint32_t ticks_off, uint16_t times)
{
	buzzer.ticks_on = ticks_on;
	buzzer.ticks_off = ticks_off;
	buzzer.times = times;
	buzzer.status = BUZZER_START_NEW_CYCLE;	
}

void buzzer_handler()
{
    switch(buzzer.status)
	{
        case BUZZER_START_NEW_CYCLE:
            if(buzzer.ticks_on > 0)
			{
				
				__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, period / 2);
                if(buzzer.ticks_off > 0)
				{ /* 计数时间不为0 即为断鸣否则为长鸣 */
                    buzzer.ticks_count = 0;
                    buzzer.status = BUZZER_WATTING_OFF; /* 等待蜂鸣器鸣响时间结束 */
                }
				else
				{
					buzzer.status = BUZZER_IDLE; /* 长鸣，转入空闲 */
				}
            }
			else
			{ /* 只要鸣响时间不为0 即为不鸣响 */
               __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
				buzzer.status = BUZZER_IDLE; /* 不鸣响，转为空闲 */
            }
            break;
			
        case BUZZER_WATTING_OFF:
            buzzer.ticks_count += BUZZER_TIMER_PERIOD;
            if(buzzer.ticks_count >= buzzer.ticks_on)
			{ /* 蜂鸣器鸣响时间结束 */
                __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
                buzzer.status = BUZZER_WATTING_PERIOD_END;
            }
            break;

        case BUZZER_WATTING_PERIOD_END:	/* 等待周期结束 */
            buzzer.ticks_count += BUZZER_TIMER_PERIOD;
            if(buzzer.ticks_count >= (buzzer.ticks_off + buzzer.ticks_on))
			{
				buzzer.ticks_count -= (buzzer.ticks_off + buzzer.ticks_on);
                if(buzzer.times == 1)
				{ 
                    /* 剩余次数为1时就可以结束此次控制任务 */
                    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
                    buzzer.status = BUZZER_IDLE;  /* 次数用完，转入空闲 */
                }
				else
				{
                    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, period / 2);
                    buzzer.times = buzzer.times == 0 ? 0 : buzzer.times - 1;
                    buzzer.status = BUZZER_WATTING_OFF;
                }
            }
            break;

        case BUZZER_IDLE:
            break;
		
        default:
            break;
    }
}
