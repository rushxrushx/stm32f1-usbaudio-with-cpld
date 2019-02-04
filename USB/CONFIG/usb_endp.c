/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_endp.c
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Endpoint routines
*******************************************************************************/
#include "usb_lib.h"
#include "usb_istr.h"
#include "i2splayer.h"//output program
 
u32 temp_buf[(ENDP1_Size/4)];
u8 fb_buf[3];

u16 rx_bytes_count;
u16 rx_frames_count;
u32 overrun_counter=0;


void fb(u32 freq)
{
u32 fbvalue, nInt,nFrac;

	// example freq:44100
	// 10.10fb=  44 << 14 + 100 << 4
	
	nInt=freq/1000;
	nFrac=(freq - nInt*1000);
		
	fbvalue=(nInt <<14) | (nFrac<<4);	
	
	fb_buf[0]=fbvalue & 0xff;
	fb_buf[1]=(fbvalue>>8) & 0xff;
	fb_buf[2]=(fbvalue>>16) & 0xff;

//{fb_buf[0]=0x66;fb_buf[1]=0x06;fb_buf[2]=0x0b;}
}



// EP OUT 0x01
void EP1_OUT_Callback(void)
{

u16 i;
u16 nextbuf;
	
//同步端点是强制使用双缓冲的，没有选择
	if (GetENDPOINT(ENDP1) & EP_DTOG_TX)
	{
		rx_bytes_count = GetEPDblBuf0Count(ENDP1);
		PMAToUserBufferCopy((u8 *)&temp_buf, ENDP1_BUF0Addr, rx_bytes_count); 
	}else
	{
		rx_bytes_count = GetEPDblBuf1Count(ENDP1);
		PMAToUserBufferCopy((u8 *)&temp_buf, ENDP1_BUF1Addr, rx_bytes_count);
	}
	FreeUserBuffer(ENDP1, EP_DBUF_OUT);

    rx_frames_count	=	rx_bytes_count / 4; //32位一帧，16位每声道 X 2声道-> 4 bytes

//temp到环形缓存拷贝程序
	for (i=0; i<rx_frames_count; i++ )   //CPU屌啊，一个字一个字的拷
	{
	//计算即将写入的位置
		if (Write_ptr < i2s_BUFSIZE ) nextbuf=Write_ptr+1 ;
		else nextbuf=0;
	   
		if (nextbuf != Play_ptr ) //如果没有追尾，把这一帧数据写入到环形缓存
		{
			Write_ptr = nextbuf;
			i2s_buf[Write_ptr] = temp_buf[i];

		}
		else //撞上了
		{
			if (audiostatus==0) DAC_PLAY();

			overrun_counter++;//立OVER-RUN FLAG
			break;//吃吐了，剩下数据全部不要了
		}
	}




}



// EP IN 0x82
void EP2_IN_Callback (void)
{


}



// SOF
void SOF_Callback (void)
{

u16 data_remain;

//计算缓冲数据量
	if (Write_ptr > Play_ptr) data_remain= Write_ptr - Play_ptr;
			
	else data_remain= i2s_BUFSIZE - Play_ptr + Write_ptr;

//数据量达到一半，启动播放
	if ( (audiostatus==0) && (data_remain > i2s_BUFSIZE/2 ) ) DAC_PLAY();

//计算反馈数据

	if (data_remain > i2s_BUFSIZE/3*2 ) fb(44030);

	else if (data_remain > i2s_BUFSIZE/3*1 ) fb(44100);

	else fb(44160);

//同步端点是强制使用双缓冲的
	if (GetENDPOINT(ENDP2) & EP_DTOG_RX)
	{
		UserToPMABufferCopy(fb_buf, ENDP2_TXADDR, 3);
		SetEPDblBuf0Count(ENDP2, EP_DBUF_IN, 3); 
	}else
	{
		UserToPMABufferCopy(fb_buf, ENDP2_TX2ADDR, 3);
		SetEPDblBuf1Count(ENDP2, EP_DBUF_IN, 3);
	}
		FreeUserBuffer(ENDP2, EP_DBUF_IN);
		SetEPTxValid(ENDP2);

}

