#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

//
//板载LED灯驱动

#define LED0 PAout(1)
#define LED1 PAout(2)	
#define LED2 PAout(3)
#define LED3 PAout(4)

////释放CPLD复位并且点亮播放指示灯
#define Gp_PLAY GPIO_SetBits(GPIOA, GPIO_Pin_4);GPIO_SetBits(GPIOB, GPIO_Pin_0);

//使能CPLD复位并且熄灭播放指示灯
#define Gp_STOP GPIO_ResetBits(GPIOA, GPIO_Pin_4);GPIO_ResetBits(GPIOB, GPIO_Pin_0);	

void LED_Init(void);//初始化

		 				    
#endif
