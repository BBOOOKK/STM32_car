#ifndef __MOTOR_H__
#define __MOTOR_H__

void Motor_Init(void);
int int_protect(int data, int max_out, int min_out);
void Move_control(int l_pwm,int r_pwm);

#endif
