#include "main.h"
#include "string.h"

#if !defined(_OLEDSD_H_)
#define _OLEDSD_H_

/* OLED接口说明：

  七管脚SPI(以SPI1为例)：
      OLED -- 单片机
       GND -- GND
       VCC -- VCC
        D0 -- SPI_SCK(A5)
        D1 -- SPI_MOSI(A6)
RES(即RST) -- 复位(B0)           // 自定义GPIO输出口
        DC -- 命令/数据标识(B1)  // 区分发送给OLED命令或数据
        CS -- 片选(A4)          // 控制数据是否发送给OLED
  
  四管脚IIC(以IIC2为例)：
      OLED -- 单片机
       GND -- GND
       VCC -- VCC
       SCL -- IIC_SCL(B10)
       SDA -- IIC_SDA(B11)

*/

/* ----------用户通信方式自定义--------- */
/*                                     */
  #define  SPItoOLED hspi1    // 若为SPI通信，宏定义此SPI
  //#define  IICtoOLED hi2c2    // 若为IIC通信，宏定义此IIC

  // 用户自定义SPI模式和引脚(IIC无需定义引脚)
  #if defined(SPItoOLED)
    // CS脚
    #define OLED_CS_GPIO_PORT         GPIOA
    #define OLED_CS_PIN               GPIO_PIN_4
    // RES脚
    #define OLED_RES_GPIO_PORT        GPIOB
    #define OLED_RES_PIN              GPIO_PIN_0
    // DC脚
    #define OLED_DC_GPIO_PORT         GPIOB
    #define OLED_DC_PIN               GPIO_PIN_1
    
  #endif // SPItoEOLED

/*                                     */
/* ----------用户通信方式自定义--------- */

/* ----------------OLED函数库---------------- */

#define OLED_Width 128
#define OLED_Height 64



typedef struct Image {
  uint8_t width;           // 图片宽度
  uint8_t height;           // 图片高度
  const uint8_t *picture; // 图片数据
} Image;

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

enum OLED_BYTE_TYPE{
  OLED_Command,
  OLED_Data
};

enum OLED_WAIT_TYPE{
  OLED_Wait,
  OLED_NoWait
};

/* -------------------------------- */
/* -------------------------------- */
/* -------------------------------- */
/* -------------------------------- */
// 功能函数
uint8_t OLED_Get_StrLength(uint8_t* str);
uint32_t OLED_Pow(uint8_t m,uint8_t n);
void OLED_BufferSet(uint8_t n);
void OLED_BufferClear(void);
void OLED_BufferFill(void);
void OLED_ColorTurn(uint8_t Mode);
void OLED_DisplayTurn(uint8_t Mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_SetCursorBuf(uint8_t x, uint8_t y);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t Mode);
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t Mode);
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r);
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, uint8_t Mode);
void OLED_DrawRectangle(uint8_t x,uint8_t y,uint8_t width,uint8_t height,uint8_t frame,uint8_t inside);
void OLED_ShowNum_Bin(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t Mode);
void OLED_ShowNum_Dec(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t Mode);
void OLED_ShowNum_Hex(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t Mode);
void OLED_ShowNum_SignedDec(uint8_t x,uint8_t y,int32_t num,uint8_t len,uint8_t size1,uint8_t Mode);
void OLED_ShowFPS(uint8_t x,uint8_t y,uint8_t size1,uint8_t Mode);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t Mode);
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1,uint8_t Mode);
void OLED_ShowString_Rowcentering(uint8_t y,uint8_t *chr,uint8_t size1,uint8_t Mode);
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1,uint8_t Mode);
void OLED_ScrollDisplay(uint8_t num,uint8_t space,uint8_t Mode);
void OLED_Scroll_InsiderHorizental_Enable(uint8_t start_page,uint8_t end_page,Roll_Frame frame,uint8_t Mode);
void OLED_Scroll_InsiderHorizental_disable(void);
void OLED_ShowPicture(uint8_t x,uint8_t y,uint8_t sizex,uint8_t sizey,const uint8_t BMP[],uint8_t Mode);
void OLED_ShowPicStruct(uint8_t x,uint8_t y, Image Pic_Structure,uint8_t Mode);

// 底层函数 
void OLED_SendByte_Poll(uint8_t Byte,uint8_t CommandOrData);
void OLED_SendBytes_Poll( uint8_t* Bytes,uint8_t CommandOrData, uint8_t Size);
void OLED_SendBytes_DMA( uint8_t* Bytes,uint8_t CommandOrData, uint8_t Size);
void OLED_Refresh_Poll(void);
void OLED_Refresh_DMA(void);
void OLED_Init(void);

#endif	// _OLEDSD_H_


