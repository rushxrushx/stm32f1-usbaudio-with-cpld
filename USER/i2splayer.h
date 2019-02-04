
extern u8 audiostatus;							//bit0:0,ÔÝÍ£²¥·Å;1,¼ÌÐø²¥·Å 


extern u16 Play_ptr;							//¼´½«²¥·ÅµÄÒôÆµÖ¡»º³å±àºÅ
extern u16 Write_ptr;							//µ±Ç°±£´æµ½µÄÒôÆµ»º³å±àºÅ 

#define i2s_BUFSIZE	2048							
extern u32 i2s_buf[(i2s_BUFSIZE+1)]; 					//ÒôÆµ»º³å:16bit x 2ch -> 32bit DWORD samples

extern void DAC_INIT(void);
extern void DAC_PLAY(void);
extern void DAC_STOP(void);
extern void DAC_SHUTDOWN(void);
