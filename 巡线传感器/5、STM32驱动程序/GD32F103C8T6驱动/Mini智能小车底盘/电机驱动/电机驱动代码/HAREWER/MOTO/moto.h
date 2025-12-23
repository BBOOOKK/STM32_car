#ifndef __MOTO_H
#define	__MOTO_H

#include "stm32f10x.h"


void Forward(int Speed);
void Backward(int Speed);
void Turnleft(int Speed);
void Turnright(int Speed);
void Stop(void);
void SpinLeft(int Speed) ;
void SpinRight(int Speed);


#endif
