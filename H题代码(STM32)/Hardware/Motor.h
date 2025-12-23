#ifndef __MOTOR_H__
#define __MOTOR_H__

/* 电机驱动模块 对外提供的接口函数 */
void Motor_Init(void);                      // 初始化电机PWM和方向控制引脚
int  int_protect(int data, int max_out, int min_out);  // 限幅函数，将数据限制在指定范围内
void Move_control(int l_pwm, int r_pwm);    // 控制左右电机转速和方向 (l_pwm, r_pwm 范围±2000)

#endif /* __MOTOR_H__ */
