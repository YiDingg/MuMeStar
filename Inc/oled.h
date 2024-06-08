/**
 * 本函数库基于下面 OLED 型号进行编写: 
 * 0.96寸128x64屏幕，SSD1306控制芯片，IIC通信协议，4管脚
*/

#ifndef __OLED_H
#define __OLED_H 

#include "main.h"
#include "stdlib.h"	

/* I2C接口宏定义 */
#define I2CtoOLED &hi2c2

/* ----------------OLED函数库---------------- */
#define u8 uint8_t 
#define u16 uint16_t
#define u32 uint32_t
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

// 滚动步骤时间间隔, 参数越大间隔越长, 滚动越慢
typedef enum
{ 
	FRAME_2 = 0x07,
	FRAME_3 = 0x04, 
	FRAME_4 = 0x05,
	FRAME_5 = 0x06,
	FRAME_6 = 0x00, 
	FRAME_32 = 0x01,	
	FRAME_64 = 0x02,
	FRAME_128 = 0x03, 	
}Roll_Frame;

void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_WR_Byte(u8 dat,u8 mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Buffer_clear(void);
void OLED_Clear(void);
void OLED_Clear_quick(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);
void OLED_DrawCircle(u8 x,u8 y,u8 r);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode);
void OLED_ShowString_rowcentering(u8 y,u8 *chr,u8 size1,u8 mode);
u32  OLED_Pow(u8 m,u8 n); 
void OLED_ShowNum_bin(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);
void OLED_ShowNum_dec(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);
void OLED_ShowNum_hex(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);
void OLED_ShowNum_signeddec(u8 x,u8 y,int32_t num,u8 len,u8 size1,u8 mode);
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,const u8 BMP[],u8 mode);
void OLED_Scroll_InsiderHorizental_Enable(uint8_t start_page,uint8_t end_page,Roll_Frame frame,uint8_t mode);
void OLED_Scroll_InsiderHorizental_disable(void);
uint8_t my_strlen(uint8_t* str);
void OLED_Scroll_LongCN_Enable(uint16_t x,uint8_t y,uint8_t *str,uint8_t mode); // 此函数不可用
void OLED_Init(void);

#endif

