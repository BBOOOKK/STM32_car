#include "led.h"
#include "string.h"


void led_handler(LEDHandleTypeDef *self)
{   
    switch(self->status)
	{
        case LED_START_NEW_CYCLE:
            if(self->ticks_on > 0)
			{
				self->write_pin(0);
                if(self->ticks_off > 0)
				{ /* 熄灭时间不为0 即为闪烁否则为长亮 */
                    self->ticks_count = 0;
                    self->status = LED_WATTING_OFF; /* 等待LED灯亮起时间结束 */
                }
				else
				{
					self->status = LED_IDLE; /* 长亮，转入空闲 */
				}
            }
			else
			{ /*只要亮起时间为 0 即为长灭 */
				self->write_pin(1);
				self->status = LED_IDLE; /* 长灭， 转入空闲 */
            }
            break;
			
        case LED_WATTING_OFF:
            self->ticks_count += LED_TIMER_PERIOD;
            if(self->ticks_count >= self->ticks_on)
			{ /* LED 亮起时间结束 */
				self->write_pin(1);
                self->status = LED_WATTING_PERIOD_END;
            }
            break;

        case LED_WATTING_PERIOD_END:	/* 等待周期结束 */
            self->ticks_count += LED_TIMER_PERIOD;
            if(self->ticks_count >= (self->ticks_off + self->ticks_on))
			{
				self->ticks_count -= (self->ticks_off + self->ticks_on);
                if(self->times == 1)
				{ 
                    /* 剩余重复次数为1时就可以结束此次控制任务 */
					self->write_pin(1);
                    self->status = LED_IDLE;  /* 重复次数用完， 转入空闲 */
                }
				else
				{
					self->write_pin(0);
                    self->times = self->times == 0 ? 0 : self->times - 1;
                    self->status = LED_WATTING_OFF;
                }
            }
            break;

        case LED_IDLE:
            break;
		
        default:
            break;
    }
}
