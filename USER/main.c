//USB异步声卡 实验  
//数码之家 rush 制作,基于正点原子程序(st例程)深度修改
#include "led.h"
//#include "delay.h"
//#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"	 
#include "i2splayer.h"
 	 
int main(void)
{
//set clock	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
//set jtag
 	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
 
//	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化
 	LED_Init();			     //LED端口初始化
//	KEY_Init();	 	
	LED0=1;//电源指示灯
  
	usb_all_init();	//USB硬件层初始化  
 	USB_Init();	//USB软件库初始化 
	DAC_INIT();
	   			   
	printf("USB Connecting...");	 
	while(1)
	{

		if(bDeviceState==CONFIGURED)//USB连接上了？
		{
			LED1=1;					 
		}else
		{
 			LED1=0;		 
		}   
		
	};  										    			    
}



