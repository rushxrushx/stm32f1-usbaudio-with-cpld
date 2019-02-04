/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : hw_config.c
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"		  

#include "usb_pwr.h"
#include "sys.h"
		   

/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
	bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;
	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
	{
		/* Device configured */
		bDeviceState = CONFIGURED;
	}
	else
	{
		bDeviceState = ATTACHED;
	}	    
}  

//USB中断配置
void USB_Interrupts_Config(void)
{ 
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;


	/* 2 bit for pre-emption priority, 2 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
 

	/* Configure the EXTI line 18 connected internally to the USB IP */
	EXTI_ClearITPendingBit(EXTI_Line18);
											  //  开启线18上的中断
	EXTI_InitStructure.EXTI_Line = EXTI_Line18; // USB resume from suspend mode
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//line 18上事件上升降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure); 	 

	/* Enable the USB interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;	//组2，优先级次之 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USB Wake-up interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;   //组2，优先级最高	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_Init(&NVIC_InitStructure);   	 
}							 		   
	   					  
/*******************************************************************************
* Function Name  : USB_Cable_Config
* Description    : Software Connection/Disconnection of USB Cable
* Input          : None.
* Return         : Status
*******************************************************************************/

/* Define the STM32F10x hardware depending on the used evaluation board */
#define USB_DISCONNECT                      GPIOA  
#define USB_DISCONNECT_PIN                  GPIO_Pin_15
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOA
  
void USB_Cable_Config (FunctionalState NewState)
{	   
	if (NewState != DISABLE)
	{
		GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	}
	else
	{
    	GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	}
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
  u32 Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(u32*)(0x1FFFF7E8);
  Device_Serial1 = *(u32*)(0x1FFFF7EC);
  Device_Serial2 = *(u32*)(0x1FFFF7F0);

  if (Device_Serial0 != 0)
  {
    Speaker_StringSerial[2] = (u8)(Device_Serial0 & 0x000000FF);
    Speaker_StringSerial[4] = (u8)((Device_Serial0 & 0x0000FF00) >> 8);
    Speaker_StringSerial[6] = (u8)((Device_Serial0 & 0x00FF0000) >> 16);
    Speaker_StringSerial[8] = (u8)((Device_Serial0 & 0xFF000000) >> 24);

    Speaker_StringSerial[10] = (u8)(Device_Serial1 & 0x000000FF);
    Speaker_StringSerial[12] = (u8)((Device_Serial1 & 0x0000FF00) >> 8);
    Speaker_StringSerial[14] = (u8)((Device_Serial1 & 0x00FF0000) >> 16);
    Speaker_StringSerial[16] = (u8)((Device_Serial1 & 0xFF000000) >> 24);

    Speaker_StringSerial[18] = (u8)(Device_Serial2 & 0x000000FF);
    Speaker_StringSerial[20] = (u8)((Device_Serial2 & 0x0000FF00) >> 8);
    Speaker_StringSerial[22] = (u8)((Device_Serial2 & 0x00FF0000) >> 16);
    Speaker_StringSerial[24] = (u8)((Device_Serial2 & 0xFF000000) >> 24);
  }
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

 
//配置USB时钟,要求USBclk=48Mhz
#if SystemCoreClock==SYSCLK_FREQ_48MHz

void Set_USBClock(void)
{	
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1); 	//48M主频，48M/1.0=48M
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);	 //USB时钟使能				 
}

#elif SystemCoreClock==SYSCLK_FREQ_72MHz

void Set_USBClock(void)
{
 	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);	//72M主频，72M/1.5=48M
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);	 //USB时钟使能				 
}

#endif



//配置Usb上拉电阻
void usb_pullup_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable USB_DISCONNECT GPIO clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

  /* Configure USB pull-up pin */
  GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);

}

// Usb功能一键初始化
void usb_all_init(void)
{
	usb_pullup_init();
	USB_Cable_Config(DISABLE);
 	USB_Interrupts_Config();    
 	Set_USBClock(); 
}
