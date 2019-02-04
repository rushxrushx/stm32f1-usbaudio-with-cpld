#include "led.h"

//初始化即付宝pos机主板PA0(蓝色)和PA14(红色)为输出口.并使能这两个口的时钟


//修改兼容自己画的PCB		    
//初始化PA1-PA4连接的绿色LED
void LED_Init(void)
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		 //IO口速度
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	//LED :PA1-4
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //LED: PORT A

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//RESET: PB0
	GPIO_Init(GPIOB, &GPIO_InitStructure);		//RESET: PORT B

}
 
