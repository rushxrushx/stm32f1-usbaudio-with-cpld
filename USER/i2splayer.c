#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"
#include "i2splayer.h"
#include "led.h"

u8 audiostatus=0;							//0,暂停播放;1,继续播放 
u16 Play_ptr=0;							//即将播放的音频帧缓冲编号
u16 Write_ptr=0;							//当前保存到的音频缓冲编号 
								
u32 i2s_buf[(i2s_BUFSIZE+1)]; 					//音频缓冲:16bit x 2ch -> 32bit DWORD,1024 samples

u32 underrun_counter=0;

u32 const DMAsetting= (0 << 14) | // 非存储器到存储器模式
            (2 << 12) | // 通道优先级高
            (0 << 11) | // 存储器数据宽度16bit
            (1 << 10) | // 存储器数据宽度16bit
            (0 <<  9) | // 外设数据宽度16bit
            (1 <<  8) | // 外设数据宽度16bit
            (1 <<  7) | // 存储器地址增量模式
            (0 <<  6) | // 外设地址增量模式(不增)
            (0 <<  5) | // 非循环模式
            (1 <<  4) | // 从存储器读
            (1 <<  3) | // 允许传输错误中断(不允许)
            (0 <<  2) | // 允许半传输中断  (不允许)
            (1 <<  1) | // 允许传输完成中断
            (0);        // 通道开启		   (先不开启)


void i2s2_dma(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;


  /* DMA1  */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
  /* DMA1 Channel5 (triggered by SPI2 Tx event) Config */
	DMA1_Channel5->CCR = 0 ;	//先关闭DMA，才能写入！
    DMA1_Channel5->CPAR = (u32) &SPI2->DR; //外设地址
    DMA1_Channel5->CMAR = (u32) &i2s_buf[0]; //BUF数组地址
    DMA1_Channel5->CNDTR = 2 ; //传输长度
    DMA1_Channel5->CCR = DMAsetting | (0); // 通道开启(先不开启)
 

  /* Enable DMA channel5 IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}




void i2s2_initspi2(void)
{
  I2S_InitTypeDef I2S_InitStructure;

//开时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
//复位
  SPI_I2S_DeInit(SPI2);
//设置
  I2S_StructInit( &I2S_InitStructure );   //初始化
  I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;
  I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;			//左对齐 MSB;//右对齐 LSB;//标准IIS Phillips;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16bextended;
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;	//MCLK输出影响时钟精度
  I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_44k;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;					//无数据 LRCK高电平
  I2S_Init(SPI2, &I2S_InitStructure);


SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);//发送缓冲区DMA使能 


}

void I2S_2_gpio(void)
{   
  GPIO_InitTypeDef GPIO_InitStructure;

//最后再来配置 IO pins，因为没先打开外设的话，输出不确定
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
   
}                                                                                                                                                 




void DAC_PLAY(void)
{

//更新 DMA 读取入口地址												   
    DMA1_Channel5->CMAR = (u32) &(i2s_buf[Play_ptr]);
//传输长度需要重新写入
    DMA1_Channel5->CNDTR = 2 ;
		
//重启DMA
    DMA1_Channel5->CCR = DMAsetting | (1);// 通道开启 (立即开启)

//复位并重新设置iis模块，解决左右翻转

	i2s2_initspi2();  //这些是复位后必须重新写的设置
//启动iis模块
	I2S_Cmd(SPI2, ENABLE);

	audiostatus=1;
	Gp_PLAY; //释放CPLD复位并且点亮播放指示灯

}

__inline void stopi2s2(void)
{
	I2S_Cmd(SPI2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);//复位需要，解决左右声道反
//关闭DMA，不改变设置
    DMA1_Channel5->CCR = DMAsetting; // 关闭DMA
		
	audiostatus=0;	
	Gp_STOP;//使能CPLD复位并且熄灭播放指示灯

}


void DAC_STOP(void)	
{
   stopi2s2();
}


void DAC_INIT(void)
{
		i2s2_dma();//这是DMA初始化，复位后无需重新写
		I2S_2_gpio();//gpio初始化，仅需要开机做一次即可
		stopi2s2();//初始化为停止状态，并解决第一次启动声道反
					//初始化为停止状态时候不需要初始化i2s模块
					//不然没有数据填进去，左右声道就反了
					//在每次开始播放时都要复位并初始化i2s模块
}


void DMA1_Channel5_IRQHandler()
{
u16 next_Playptr;
//清理DMA各类中断标志
    DMA_ClearITPendingBit(DMA1_IT_GL5);
//先关闭DMA才能写入
	DMA1_Channel5->CCR = DMAsetting;        // 通道关闭;	


	//算下一个数据帧
	if (Play_ptr < i2s_BUFSIZE ) next_Playptr = Play_ptr + 1;

	else  next_Playptr = 0;    //循环回头部

    
	if( next_Playptr == Write_ptr)	//撞尾了 
	{
		stopi2s2();
		underrun_counter++;
		return;
	}else
	{
		Play_ptr = next_Playptr;//可以开开心心去下一个数据帧
	}


	//更新 DMA 读取入口地址												   
    DMA1_Channel5->CMAR = (u32) &(i2s_buf[Play_ptr]);

	//传输长度需要重新写入
    DMA1_Channel5->CNDTR = 2 ;
	
//重启DMA
    DMA1_Channel5->CCR = DMAsetting|(1); //通道开启


}
