/**************************************************************************
* 文件：OLEDSD.c
* 作者：StarDY
* 邮箱：1308102491@qq.com
* 描述：OLED软件驱动，兼容硬件SPI+硬件IIC
***************************************************************************/
#include "OLEDSD.h"
#include "OLEDSD_Font.h"

	#if defined(SPItoOLED)
	#include "spi.h"
	#endif // SPItoOLED 

	#if defined(IICtoOLED)
	#include "i2c.h"
	#endif // IICtoOLED

/* -------------- 全局变量 -------------- */
/*                                       */
uint16_t FPS,FPS_Count,ms_Count;
uint8_t OLED_CursorBuf[4] = {0};
uint8_t OLED_GRAMBuf[8][128];		// 函数直接控制的显存
uint8_t OLED_AnimaBuf[8][128];		// 用于记录当前OLED屏幕上的画面，便于动画生成
uint8_t OLED_RefComBuf[8][3] = {	// 命令数组，用于 refresh 时设置页地址
	{0x00,0xB0,0x10},
	{0x00,0xB1,0x10},
	{0x00,0xB2,0x10},
	{0x00,0xB3,0x10},
	{0x00,0xB4,0x10},
	{0x00,0xB5,0x10},
	{0x00,0xB6,0x10},
	{0x00,0xB7,0x10},
};

// OLED的初始化命令
const uint8_t OLED_Init_CMD[29] =
{
	0xAE, 	// 熄灭屏幕

	0xB0, 	// 设置页地址
	0x00,	// 设置低列地址--set low column address
	0x10, 	// 设置高列地址--set high column address
	0x40, 	// 从寄存器中哪个地址起作为显存(所以在SendByte时才会发到寄存器的0x40)--set start line address Set Mapping RAM Display Start Line (0x00~0x3F)

	0x81, 	// 进入亮度/对比度配置--set contrast control register
	0x01, 	// 亮度：0x00~0xff(0x00时屏幕熄灭)--Set SEG Output Current Brightness

//  0xA4,   // (默认)0xA4显示屏幕显存内容(硬件显存)，0xA5无视显存点亮全屏

	0xA1, 	// 设置屏幕左右镜像,0xa1正常,0xa0左右反置--Set SEG/Column Mapping    
	0xC8,   // 设置屏幕上下镜像,0xc8正常,0xc0上下反置--Set COM/Row Scan Direction,  
	0xA6, 	// 设置显示模式,0xA6正常显示,0xA7反相显示--set normal display

	0xA8,	// 进入复用率配置--set multiplex ratio(1 to 64)
	0x3F, 	// 复用率：0x00~0x3F--1/64 duty

	0xD3, 	// 进入屏幕偏移量配置(向上偏移，以行为单位)--set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	0, 		// 偏移行数：0~64 --not offset

	0xD5, 	// 进入时钟分频/震荡频率设置--set display clock divide ratio/oscillator frequency
	0xD0, 	// 高四位屏幕刷新率：0~F(越大越快, 也越耗电，0为10Hz，F为100Hz)，例如0x80和0xF0都合法；低四位时钟分频：一般为0 --set screen refresh rate

	0xD9, 	
	0x05, 

	0xD9, 	// 设置预充电周期
	0xF1,

	0xDA, 	// 进入列引脚硬件配置
	0x12, 	// 128*64->0x12, 128*32->0x02

	0xDB, 	// 设置VCOMH反压值
	0x30, 

	0x8D, 	// 进入电荷泵配置
	0x14, 	// 开启电荷泵

	0xAF, 
	0x20, 
	0x00
};

/*                                       */
/* -------------- 全局变量 -------------- */


/* -------------OLEDSD函数定义------------ */

uint8_t OLED_Get_StrLength(uint8_t* str)
{
    uint8_t count = 0; //计数
    while(*str != '\0')
    {count++;str++;}
    return count;
}

/**
 * @brief 幂函数
 * @param m 底数
 * @param n 指数
 * @retval m^n
*/
uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}

/**
 * @brief 用某一数字覆盖显存
 * @param byte 0x00~0xFF
 * @param n 字节数:0~128*8=1024
 * @note 对OLED屏幕上的一字节，低位在下高位在上。例如0x01为最下方一个像素点亮
 * @retval void
*/
void OLED_BufferSet(uint8_t byte, uint16_t n){
	memset(OLED_GRAMBuf,byte,n);
}

/**
 * @brief 清空显存但不刷新屏幕
 * @retval void
*/
void OLED_BufferClear(void){
	OLED_BufferSet(0,1024);
}

/**
 * @brief 点亮显存(反色模式下为熄灭)
 * @retval void
*/
void OLED_BufferFill(void){
	OLED_BufferSet(0xFF,1024);
}

/**
 * @brief 以不同模式刷新OLED，以提供更美观的动画效果 
 * @param Mode {1,2,3,4,5,6}
 * @param time 动画时长, 范围[0,10],单位us, 越小速度越快 
 * @note 
 * 1-双竖线由内向外刷新;
 * 2-双竖线由外向内刷新;
 * 3-双横线由内向外刷新;
 * 4-双横线由外向内刷新;
 * 5-单竖线由左向右刷新;
 * 6-单竖线由右向左刷新;
 * 7-单竖线由左向右刷新;
 * 8-单竖线由右向左刷新;
 * 9-矩形方框(不可见)由内向外刷新;
 * 10-矩形方框由外向内刷新;
 * @retval void
*/
/* 
void OLED_Refresh_Poll_Mutimode(uint8_t Mode, uint8_t time){
	uint8_t i,j;	// 循环变量
	BufFinshFlag = 0;
	switch (Mode)
	{
	case 1:	// 双竖线由内向外刷新
		for (i = 0; i < 64; i++)
		{
			for ( j = 0; j < 8; j++)
			{
			//	OLED_GRAMBuf_co[j][64+i] = OLED_GRAMBuf[j][64+i];
			//	OLED_GRAMBuf_co[j][63-i] = OLED_GRAMBuf[j][63-i];
			}
			OLED_delay(time);
			while (BufFinshFlag);
			BufFinshFlag = 1;
			RefreshType = MutiRefresh;
			HAL_I2C_Master_Transmit_DMA(&IICtoOLED,0x78,OLED_RefComBuf[0],3);
		}
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	default:return;
	}
}
*/

/**
 * @brief 屏幕是否反色
 * @param Mode 0正常1反色(当前取模下，0--白字黑底)
 * @retval void
*/
void OLED_ColorTurn(uint8_t Mode)
{
	switch (Mode){
		case 0:	OLED_SendByte_Poll(0xA6,OLED_Command);return;	// 正常
		case 1:	OLED_SendByte_Poll(0xA7,OLED_Command);return;		// 反色
		default:return;
	}
}

/**
 * @brief 屏幕是否反转
 * @param Mode 0正常，1翻转
 * @retval void
*/
void OLED_DisplayTurn(uint8_t Mode)
{
	switch (Mode){
		case No:	// 正常显示
			OLED_SendByte_Poll(0xC8,OLED_Command);
			OLED_SendByte_Poll(0xA1,OLED_Command);return;
		case Yes:	// 反转显示
			OLED_SendByte_Poll(0xC0,OLED_Command);
			OLED_SendByte_Poll(0xA0,OLED_Command);return;
		default:return;
	}
}

/**
 * @brief 屏幕水平镜像翻转
 * @param Mode  0正常，1翻转
 * @retval void
*/
void OLED_MirrorHo(uint8_t Mode){
	switch (Mode){
		case No:OLED_SendByte_Poll(0xA1,OLED_Command);return;
		case Yes:OLED_SendByte_Poll(0xA0,OLED_Command);return;
		default:return;
	}
}

/**
 * @brief 屏幕垂直镜像翻转
 * @param Mode  0正常，1翻转
 * @retval void
*/
void OLED_MirrorVer(uint8_t Mode){
	switch (Mode){
		case No:OLED_SendByte_Poll(0xC8,OLED_Command);return;
		case Yes:OLED_SendByte_Poll(0xC0,OLED_Command);return;
		default:return;
	}
}

/**
 * @brief 打开OLED屏幕
 * @retval void
*/
void OLED_DisPlay_On(void)
{
	OLED_SendByte_Poll(0x8D,OLED_Command);	// 进入电荷泵配置
	OLED_SendByte_Poll(0x14,OLED_Command);	// 开启电荷泵
	OLED_SendByte_Poll(0xAF,OLED_Command);	// 点亮屏幕
}

/**
 * @brief 关闭OLED屏幕
 * @retval void
*/
void OLED_DisPlay_Off(void)
{
	OLED_SendByte_Poll(0x8D,OLED_Command);	// 进入电荷泵配置
	OLED_SendByte_Poll(0x10,OLED_Command);	// 关闭电荷泵
	OLED_SendByte_Poll(0xAE,OLED_Command);	// 关闭屏幕
}

/**
 * @brief 设置OLED屏幕刷新率
 * @param rate 范围:0~15
 * @retval void
*/
void OLED_Set_RefershRate(uint8_t rate){
	if(rate>15){return;}
	OLED_SendByte_Poll(0xD5,OLED_Command);		// 进入时钟分频/震荡频率设置
	OLED_SendByte_Poll(rate<<4,OLED_Command);	// 高四位是屏幕刷新率：0~F
}

/** 
  * @brief 将光标位置(x,y)存于CursorBuf中
  * @param x x轴, 从 0 到 127
  * @param y 页位置, 从 0 到 7
  * @retval none
*/
void OLED_SetCursorBuf(uint8_t x, uint8_t y){
	OLED_CursorBuf[0] = 0x00|(x&0x0F);
	OLED_CursorBuf[1] = 0xB0|y;
	OLED_CursorBuf[2] = 0x10|((x&0xF0) >> 4);
}


/**
 * @brief 作一个像素点
 * @param x 横坐标(从左往右)
 * @param y 纵坐标(从上往下)
 * @param Mode 1亮0暗(默认模式下)
 * @retval void
*/
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t Mode)
{
	if(x>=OLED_Width||y>=OLED_Height){return;}
	uint8_t i,n;
	i=y/8;
	n=1<<(y%8);
	if(Mode){OLED_GRAMBuf[i][x]|=n;}
	else{OLED_GRAMBuf[i][x]&=~n;}
}

/**
 * @brief 作一条直线
 * @param x1 起点横坐标
 * @param y1 起点纵坐标
 * @param x2 终点横坐标
 * @param y2 终点纵坐标
 * @param Mode 1作亮直线, 0作暗直线
 * @retval void
*/ 
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t Mode)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量  
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_x;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+2;t++)
	{
		OLED_DrawPoint(uRow,uCol,Mode); //画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

/**
 * @brief 作一个圆
 * @param x 圆心横坐标
 * @param y 圆心纵坐标
 * @param r 圆的半径
 * @retval void
*/
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r)
{
	int a, b,num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)      
    {
        OLED_DrawPoint(x + a, y - b,1);
        OLED_DrawPoint(x - a, y - b,1);
        OLED_DrawPoint(x - a, y + b,1);
        OLED_DrawPoint(x + a, y + b,1);
 
        OLED_DrawPoint(x + b, y + a,1);
        OLED_DrawPoint(x + b, y - a,1);
        OLED_DrawPoint(x - b, y - a,1);
        OLED_DrawPoint(x - b, y + a,1);
        a++;
        num = (a * a + b * b) - r*r;//计算画的点离圆心的距离
        if(num > 0)
        {
            b--;
            a--;
        }
    }
}

/**
 * @brief 绘制一个椭圆
 * @param x 椭圆中心横坐标
 * @param y 椭圆中心纵坐标
 * @param a 椭圆长轴(包括边界点)
 * @param b 椭圆短轴(包括边界点)
 * @param Mode 1亮0暗, 椭圆边界颜色,
 */
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, uint8_t Mode){
  int xpos = 0, ypos = b;
  int a2 = a * a, b2 = b * b;
  int d = b2 + a2 * (0.25 - b);
  while (a2 * ypos > b2 * xpos)
  {
    OLED_DrawPoint(x + xpos, y + ypos, Mode);
    OLED_DrawPoint(x - xpos, y + ypos, Mode);
    OLED_DrawPoint(x + xpos, y - ypos, Mode);
    OLED_DrawPoint(x - xpos, y - ypos, Mode);
    if (d < 0)
    {
      d = d + b2 * ((xpos << 1) + 3);
      xpos += 1;
    }
    else
    {
      d = d + b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2);
      xpos += 1, ypos -= 1;
    }
  }
  d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
  while (ypos > 0)
  {
    OLED_DrawPoint(x + xpos, y + ypos, Mode);
    OLED_DrawPoint(x - xpos, y + ypos, Mode);
    OLED_DrawPoint(x + xpos, y - ypos, Mode);
    OLED_DrawPoint(x - xpos, y - ypos, Mode);
    if (d < 0)
    {
      d = d + b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3);
      xpos += 1, ypos -= 1;
    }
    else
    {
      d = d + a2 * (-(ypos << 1) + 3);
      ypos -= 1;
    }
  }
}


/**
 * @brief 作一个矩形
 * @param x	起始横坐标(边框左上角)
 * @param y 起始纵坐标(边框左上角)
 * @param width 宽(算上边框的两像素)
 * @param height 高(算上边框的两像素)
 * @param frame 外部边框, 1亮0暗
 * @param inside 内部填充, 1亮0暗 
 * @retval void
*/
void OLED_DrawRectangle(uint8_t x,uint8_t y,uint8_t width,uint8_t height,uint8_t frame,uint8_t inside){
	uint8_t i;
	// 作边框
	OLED_DrawLine(x,y,x+width-1,y,frame);
	OLED_DrawLine(x,y,x,y+height-1,frame);
	OLED_DrawLine(x+width-1,y,x+width-1,y+height-1,frame);
	OLED_DrawLine(x,y+height-1,x+width-1,y+height-1,frame);
	i = 0;
	// 作填充(for比while更快)
	for ( i = 1; i < width-1; i++){OLED_DrawLine(x+i,y+1,x+i,y+height-2,inside);}
}

/**
 * @brief 以二进制显示数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(从右向左计位)
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 是否反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_Bin(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t fontsize,uint8_t Mode){
	uint8_t i = 0;
	uint8_t width = (fontsize==8)?6:(fontsize/2);
	for (i = 0; i < len; i++){OLED_ShowChar(x+width*i, y , (num/OLED_Pow(2, len - i - 1))%2+'0', fontsize, Mode);}
}

/**
 * @brief 以十进制显示数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(左侧自动补零)
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 是否反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_Dec(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t fontsize,uint8_t Mode)
{
	uint8_t i;
	uint8_t width = (fontsize==8)?6:(fontsize/2);
	for (i = 0; i < len; i++){OLED_ShowChar(x+width*i, y , (num/OLED_Pow(10,len-i-1))%10+'0', fontsize, Mode);}
}


/**
 * @brief 以十六进制显示数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(从右向左计位)
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 是否反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_Hex(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t fontsize,uint8_t Mode){
	uint8_t i, decnum;
	uint8_t width = (fontsize==8)?6:(fontsize/2);
	for (i = 0; i < len; i++)							
	{
		decnum = num / OLED_Pow(16, len - i - 1) % 16;
		if (decnum < 10){OLED_ShowChar(x+width*i, y , decnum + '0', fontsize, Mode);}
		else{OLED_ShowChar(x+width*i, y , decnum - 10 + 'A', fontsize, Mode);}
	}
}

/**
 * @brief 显示有符号数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(不计正负号)
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 是否反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_SignedDec(uint8_t x,uint8_t y,int32_t num,uint8_t len,uint8_t fontsize,uint8_t Mode){
	uint8_t width = (fontsize==8)?6:(fontsize/2);
	if(num >=0){
		OLED_ShowChar(x,y, '+', fontsize, Mode);
		OLED_ShowNum_Dec(x+width, y, num, len, fontsize, Mode);
	}
	else{
		OLED_ShowChar(x,y, '-', fontsize, Mode);
		OLED_ShowNum_Dec(x+width, y, -num, len, fontsize, Mode);
	}
}

/**
 * @brief 显示无符号浮点数
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param int_len 整数位数
 * @param fra_len 小数位数
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 是否反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowFloat_Dec(uint8_t x, uint8_t y, double num, uint8_t int_len,uint8_t fra_len, uint8_t fontsize, uint8_t Mode){
	uint8_t width = (fontsize==8)?6:(fontsize/2);
	OLED_ShowNum_Dec(x, y, (uint32_t)num, int_len, fontsize, Mode);
	OLED_ShowChar(x+int_len*width,y,'.',fontsize,Mode);
	OLED_ShowNum_Dec(x+(int_len+1)*width, y, (uint32_t)(num*OLED_Pow(10,fra_len)), fra_len, fontsize, Mode);
}

/**
 * @brief 显示无符号浮点数
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param int_len 整数位数
 * @param fra_len 小数位数
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 是否反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowFloat_SignedDec(uint8_t x, uint8_t y, double num, uint8_t int_len,uint8_t fra_len, uint8_t fontsize, uint8_t Mode){
	uint8_t width = (fontsize==8)?6:(fontsize/2);
	if(num<0){OLED_ShowChar(x,y,'-',fontsize,Mode);}
	else{OLED_ShowChar(x,y,'+',fontsize,Mode);}
	OLED_ShowNum_Dec(x+width, y, (uint32_t)num, int_len, fontsize, Mode);
	OLED_ShowChar(x+(int_len+1)*width,y,'.',fontsize,Mode);
	OLED_ShowNum_Dec(x+(int_len+2)*width, y, (uint32_t)(num*OLED_Pow(10,fra_len)), fra_len, fontsize, Mode);
}

/**
 * @brief 显示屏幕帧率(由SysTick中断实现)
 * @param x 横坐标
 * @param y 纵坐标
 * @param fontsize 字号, {8,12,16,24}
 * @param Mode 0为反色显示, 1为正常显示
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @retval void
*/
void OLED_ShowFPS(uint8_t x,uint8_t y,uint8_t fontsize,uint8_t Mode){
	if(FPS<100){
		OLED_ShowString(x,y,"FPS:",fontsize,Mode);
		OLED_ShowNum_Dec(x+4*((fontsize==8)?6:(fontsize/2)),y,FPS,2,fontsize,Mode);
	}
	if(FPS>=100){
		OLED_ShowString(x,y,"FPS:",fontsize,Mode);
		OLED_ShowNum_Dec(x+4*((fontsize==8)?6:(fontsize/2)),y,FPS,3,fontsize,Mode);
	}

}

/**
 * @brief 重定义HAL_IncTick函数以实现帧率计算
 * @retval void
*/
void HAL_IncTick(void)
{
  uwTick += uwTickFreq;	// 此句不要修改
  // 帧率计算
  if(++ms_Count >= 200){
    ms_Count=0;
    FPS = FPS_Count;
    FPS_Count = 0;
  }
}


/**
 * @brief 显示字符串
 * @param x 横坐标
 * @param y 纵坐标
 * @param Char 要显示的字符
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 0为反色显示, 1为正常显示
 * @retval void
*/
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t Char,uint8_t fontsize,uint8_t Mode)
{
	uint8_t i,m,temp;
	uint8_t x0=x,y0=y;
	const uint8_t* TEMP; 
	uint16_t size2 = (fontsize==8)?6:((fontsize/2)*(fontsize/8+((fontsize%8)?1:0)));		//得到字体一个字符对应点阵集所占的字节数
	switch (fontsize){
		case 8: TEMP=asc2_0806[Char-' '];break;
		case 12: TEMP=asc2_1206[Char-' '];break;
		case 16: TEMP=asc2_1608[Char-' '];break;
		case 24: TEMP=asc2_2412[Char-' '];break;
		default: return;
	}
	if(fontsize==12){	// 避免12*16时侵占其它空间
		for(i=0;i<6;i++){
			temp = *(TEMP+i);
			for(m=0;m<8;m++){
				if(temp&(0x01<<m)){OLED_DrawPoint(x,y,Mode);}
				else{OLED_DrawPoint(x,y,!Mode);}
				y++;
			}
			x++;
			y=y0;
		}
		x=x0;y0=y0+8;y=y0; // 换下一页
		for(i=0;i<6;i++){
			temp = *(TEMP+i+6);
			for(m=0;m<4;m++){
				if(temp&(0x01<<m)){OLED_DrawPoint(x,y,Mode);}
				else{OLED_DrawPoint(x,y,!Mode);}
				y++;
			}
			x++;
			y=y0;
		}
	}
	else{
		for(i=0;i<size2;i++){
			temp = *(TEMP+i);
			for(m=0;m<8;m++)
			{
				if(temp&(0x01<<m)){OLED_DrawPoint(x,y,Mode);}
				else{OLED_DrawPoint(x,y,!Mode);}
				y++;
			}
			x++;
			y=y0;
			if((fontsize!=8)&&((x-x0)==fontsize/2)){x=x0;y0=y0+8;y=y0;}
		}
	}
}

/**
 * @brief 显示字符串
 * @param x 横坐标
 * @param y 纵坐标
 * @param String 要显示的字符串
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 0为反色显示, 1为正常显示
 * @retval void
*/
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *String,uint8_t fontsize,uint8_t Mode)
{
	while((*String>=' ')&&(*String<='~'))//判断是否非法字符!
	{
		OLED_ShowChar(x,y,*String,fontsize,Mode);
		if(fontsize==8)x+=6;
		else x+=fontsize/2;
		String++;
  	}
}

/**
 * @brief 在某一行居中显示字符串
 * @param x 横坐标
 * @param y 纵坐标
 * @param String 要显示的字符串
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*06,12*06,16*08,24*12}
 * @param Mode 0为反色显示, 1为正常显示
 * @retval void
*/
void OLED_ShowString_Rowcentering(uint8_t y,uint8_t *String,uint8_t fontsize,uint8_t Mode){
	uint8_t i=0, size=0, n = 0, lenth = 0;
	while((*String>=' ')&&(*String<='~')){n++;String++;} // 获取字符串长度
	String -= n;
	if(fontsize == 8){size=6;}
	else{size=fontsize/2;}
	lenth = n*size;
	if( lenth>128 ){
		OLED_BufferClear();
		OLED_ShowString(16,8,"too long",24,0);
	}
	else{
		while((*String>=' ')&&(*String<='~'))
		{
			OLED_ShowChar(64-lenth/2 + i*size,y,*String,fontsize,Mode);
			i++;String++;
  		}
	}

}


/**
 * @brief 显示单个汉字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 汉字对应的序号
 * @param fontsize 字号, {8,12,16,24}
 * @note  {8,12,16,24} 依次对应 {08*08,12*12,16*116,24*24}
 * @param Mode 1正常, 0反色
 * @retval void
*/
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t fontsize,uint8_t Mode)
{
	uint8_t i,m,temp;
	uint8_t x0=x,y0=y;
	const uint8_t* TEMP; 
	uint16_t size3=fontsize*(fontsize/8 + ((fontsize%8)?1:0));  //得到字体一个字符对应点阵集所占的字节数
	switch (fontsize){
		case 12: TEMP = *(Chinese_12+num);break;
		case 16: TEMP = *(Chinese_16+num);break;
		case 24: TEMP = *(Chinese_24+num);break;
		case 32: TEMP = *(Chinese_32+num);break;
		default: return;	// 非法参数直接return
	}
	for(i=0;i<size3;i++){
		temp = *(TEMP+i);
		for(m=0;m<8;m++)
		{
			if(temp&(0x01<<m)){OLED_DrawPoint(x,y,Mode);}
			else{OLED_DrawPoint(x,y,!Mode);}
			y++;
		}
		x++;
		y=y0;
		if((x-x0)==fontsize){x=x0;y0=y0+8;y=y0;}
	}
}


/**
 * @brief 滚动显示中文
 * @param num 汉字个数
 * @param space 显示间隔 16*space
 * @param Mode 1正常, 0反色
 * @retval void
*/
void OLED_ScrollDisplay(uint8_t num,uint8_t space,uint8_t Mode)
{
	uint8_t OLED_GRAM_scroll[144][8];
	uint8_t i,n,t=0,m=0,r;
	while(1)
	{
		if(m==0)
		{
	    OLED_ShowChinese(128,24,t,16,Mode); //写入一个汉字保存在OLED_GRAM[][]数组中
			t++;
		}
		if(t==num)
			{
				for(r=0;r<16*space;r++)      //显示间隔
				 {
					for(i=1;i<144;i++)
						{
							for(n=0;n<8;n++)
							{
								OLED_GRAM_scroll[i-1][n]=OLED_GRAM_scroll[i][n];
							}
						}
           OLED_Refresh_Poll();
				 }
        t=0;
      }
		m++;
		if(m==16){m=0;}
		for(i=1;i<144;i++)   //实现左移
		{
			for(n=0;n<8;n++)
			{
				OLED_GRAM_scroll[i-1][n]=OLED_GRAM_scroll[i][n];
			}
		}
		OLED_Refresh_Poll();
	}
}

/**
 * @brief 利用SSD1306芯片内置方法开启水平滚动(不改变显存)
 * @param start_page 起始页
 * @param end_page 终止页
 * @param frame 时间间隔(滚动周期)
 * @param Mode 1向右滚动(从左向右),0向左滚动(从右向左)
 * @retval void
*/
void OLED_Scroll_InsiderHorizental_Enable(uint8_t start_page,uint8_t end_page,Roll_Frame frame,uint8_t Mode){
	OLED_SendByte_Poll(0x2E,OLED_Command);	// 必须先关闭滚动
	OLED_SendByte_Poll(Mode ? 0x26 : 0x27,OLED_Command);	// 1向右滚动，0向左滚动
	OLED_SendByte_Poll(0x00,OLED_Command);	// 发送一个虚拟字节
	OLED_SendByte_Poll(start_page & 0x07,OLED_Command);	//起始页 0
	OLED_SendByte_Poll(frame & 0x07,OLED_Command);	//滚动时间间隔
	OLED_SendByte_Poll(end_page & 0x07,OLED_Command);	//终止页 7
	OLED_SendByte_Poll(0x00,OLED_Command);	// 发送虚拟字节
	OLED_SendByte_Poll(0xFF,OLED_Command);	// 发送虚拟字节
	OLED_SendByte_Poll(0x2F,OLED_Command);   //开启滚动
}

/**
 * @brief 关闭滚动(由OLED_Scroll_InsiderHorizental_Enable开启)
 * @retval void
*/
void OLED_Scroll_InsiderHorizental_disable(void){	
	OLED_SendByte_Poll(0x2E,OLED_Command);	// 关闭滚动
}

/**
 * @brief Show one picture
 * @param x (起始)横坐标
 * @param y (起始)纵坐标
 * @param sizex 图片横宽
 * @param sizey 图片纵高 
 * @param BMP 图片对应数组名 
 * @param Mode 1正常, 0反色显示 
 * @note  起始坐标位于字符/图片的左上角
 * @retval void
*/
void OLED_ShowPicture(uint8_t x,uint8_t y,uint8_t sizex,uint8_t sizey,const uint8_t BMP[],uint8_t Mode)
{
	uint8_t i,j,k,temp,page;
	uint8_t x0=x, y0=y;
	page=sizey/8+((sizey%8)?1:0); // 计算页数
	if(sizex==OLED_Width&&sizey==OLED_Height){
		memcpy(OLED_GRAMBuf,BMP,OLED_Width*OLED_Height);
		return;
		}
	switch(Mode){
		case 0:
			for(i=0;i<page;i++){
				for(j=0;j<sizex;j++){
					temp=BMP[i*sizex+j];
					for(k=0;k<8;k++){
						if(i*8+k>=sizey){break;}
						OLED_DrawPoint(x0+j,y0+i*8+k,!((temp>>k)&0x01));}}}
			return;
		case 1:
			for(i=0;i<page;i++){
				for(j=0;j<sizex;j++){
					temp=BMP[i*sizex+j];
					for(k=0;k<8;k++){
						if(i*8+k>=sizey){break;}
						OLED_DrawPoint(x0+j,y0+i*8+k,((temp>>k)&0x01));}}}
			return;
		default:return;
	}
}

/**
 * @brief Show one picture by using Structure
 * @param x (起始)横坐标
 * @param y (起始)纵坐标
 * @param Image  图片对应结构体 
 * @param Mode 1正常, 0反色显示 
 * @note  起始坐标位于字符/图片的左上角
 * @retval void
*/
void OLED_ShowPicStruct(uint8_t x,uint8_t y, Image Pic_Structure,uint8_t Mode){
	OLED_ShowPicture(x,y,Pic_Structure.width,Pic_Structure.height,Pic_Structure.picture,Mode);
}

/* --------------------------------------------------- */
/* --------------------------------------------------- */
/* --------------------------------------------------- */
/* --------------------------------------------------- */

/* ---------SPI通讯专用-------- */
/*                             */
#if defined(SPItoOLED)

	void OLED_SPI_DC_Command(void){HAL_GPIO_WritePin(OLED_DC_GPIO_PORT,OLED_DC_PIN,GPIO_PIN_RESET);}
	void OLED_SPI_DC_Data(void){HAL_GPIO_WritePin(OLED_DC_GPIO_PORT,OLED_DC_PIN,GPIO_PIN_SET);}
	void OLED_SPI_OpenRe(void){HAL_GPIO_WritePin(OLED_CS_GPIO_PORT,OLED_CS_PIN,GPIO_PIN_RESET);}
	void OLED_SPI_CloseRe(void){HAL_GPIO_WritePin(OLED_CS_GPIO_PORT,OLED_CS_PIN,GPIO_PIN_SET); }

	/**
	 * @brief 向OLED发送一个字节(SPI_Poll模式)
	 * @param Byte 发送的字节
	 * @param CommandOrData OLED_Command写命令, OLED_Data写数据 
	 * @retval void
	*/
	void OLED_SendByte_Poll(uint8_t Byte,uint8_t CommandOrData){
		while (SPItoOLED.State!=HAL_SPI_STATE_READY);	// 检查接口状态
		switch (CommandOrData){
			case OLED_Data:OLED_SPI_DC_Data();break;  		// DC高--发送数据
			case OLED_Command:OLED_SPI_DC_Command();break;	// DC低--发送命令
			default:return;
		}
		OLED_SPI_OpenRe();  
		HAL_SPI_Transmit(&SPItoOLED,&Byte,1,0x10);  
		OLED_SPI_CloseRe();  
	}

	/**
	 * @brief 向OLED发送多个字节(SPI_Poll模式)
	 * @param Bytes 待发送的字节
	 * @param CommandOrData OLED_Command写命令, OLED_Data写数据
	 * @param Size 发送的字节数
	 * @retval void
	*/
	void OLED_SendBytes_Poll( uint8_t* Bytes,uint8_t CommandOrData, uint8_t Size){
		while (SPItoOLED.State!=HAL_SPI_STATE_READY);	// 检查接口状态
		switch (CommandOrData){
			case OLED_Data:OLED_SPI_DC_Data();break;  		// DC高--发送数据
			case OLED_Command:OLED_SPI_DC_Command();break;	// DC低--发送命令
			default:return;
		}
		OLED_SPI_OpenRe();  
		HAL_SPI_Transmit(&SPItoOLED,Bytes,Size,0x1000);   
	}

	/**
	 * @brief 向OLED发送多个字节(SPI_DMA模式)
	 * @param Bytes 待发送的字节
	 * @param CommandOrData OLED_Command写命令, OLED_Data写数据
	 * @param Size 发送的字节数
	 * @retval void
	*/
	void OLED_SendBytes_DMA( uint8_t* Bytes,uint8_t CommandOrData, uint8_t Size){
		while (SPItoOLED.State!=HAL_SPI_STATE_READY);	// 检查接口状态
		switch (CommandOrData){
			case OLED_Data:OLED_SPI_DC_Data();break;  		// DC高--发送数据
			case OLED_Command:OLED_SPI_DC_Command();break;	// DC低--发送命令
			default:return;
		}
		OLED_SPI_OpenRe();  
		HAL_SPI_Transmit_DMA(&SPItoOLED,Bytes,Size);   
	}

	/**
	 * @brief 将软件显存更新到OLED屏幕(SPI_Poll模式)
	 * @retval void
	*/
	void OLED_Refresh_Poll(void){
		while (SPItoOLED.State!=HAL_SPI_STATE_READY);	// 检查接口状态
		OLED_SPI_OpenRe();
		OLED_SPI_DC_Command();
		HAL_SPI_Transmit(&SPItoOLED,(uint8_t*)OLED_RefComBuf[0],3,10);
		OLED_SPI_DC_Data();
		HAL_SPI_Transmit(&SPItoOLED,*OLED_GRAMBuf,128*8,100);
		OLED_SPI_CloseRe();
		FPS_Count++;memcpy(OLED_AnimaBuf,OLED_GRAMBuf,1024);	// 拷贝当前帧
	}

	/** 
	 * @brief 将软件显存更新到OLED屏幕(SPI_DMA模式)
	 * @note 配置了接口对应的DMA才可以使用
	 * @retval void
	*/
	void OLED_Refresh_DMA(void){
		while (SPItoOLED.State!=HAL_SPI_STATE_READY);	// 检查接口状态
		OLED_SPI_OpenRe();
		OLED_SPI_DC_Command(); 
		HAL_SPI_Transmit(&SPItoOLED,(uint8_t*)OLED_RefComBuf[0],3,10);
		OLED_SPI_DC_Data();
		HAL_SPI_Transmit_DMA(&SPItoOLED,*OLED_GRAMBuf,128*8);
		FPS_Count++;memcpy(OLED_AnimaBuf,OLED_GRAMBuf,1024);	// 拷贝当前帧
	}

	/**
	 * @brief SPI发送完成中断回调
	 * @retval void
	*/
	void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
		if((hspi==&SPItoOLED)&&(SPItoOLED.State==HAL_SPI_STATE_READY)){OLED_SPI_CloseRe();}
	}


	void OLED_Reset(void){
		OLED_SPI_CloseRe();
		HAL_GPIO_WritePin(OLED_RES_GPIO_PORT,OLED_RES_PIN,GPIO_PIN_SET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(OLED_RES_GPIO_PORT,OLED_RES_PIN,GPIO_PIN_RESET);
		HAL_Delay(10);
		HAL_GPIO_WritePin(OLED_RES_GPIO_PORT,OLED_RES_PIN,GPIO_PIN_SET);
	} 

	void OLED_Init(void){
		FPS=0;FPS_Count=0;ms_Count = 0;
		OLED_BufferClear();
		OLED_Reset();
		OLED_SendBytes_Poll((uint8_t*)OLED_Init_CMD,OLED_Command,29);
	}
#endif // SPItoOLED

/*                             */
/* ---------SPI通讯专用-------- */

/* ---------IIC通讯专用-------- */
/*                             */
#if defined(IICtoOLED)

	/**
	 * @brief 向OLED发送一个字节(IIC_Poll模式)
	 * @param Byte 发送的字节
	 * @param CommandOrData OLED_Command写命令, OLED_Data写数据
	 * @retval void
	*/
	void OLED_SendByte_Poll(uint8_t Byte,uint8_t CommandOrData){
		while (IICtoOLED.State!=HAL_I2C_STATE_READY);	// 检查接口状态
		switch (CommandOrData){
		case OLED_Command:	// 写命令
			HAL_I2C_Mem_Write_DMA(&IICtoOLED, 0x78,0x00,I2C_MEMADD_SIZE_8BIT,&Byte,1);return;
		case OLED_Data:	// 写数据
			HAL_I2C_Mem_Write_DMA(&IICtoOLED, 0x78,0x40,I2C_MEMADD_SIZE_8BIT,&Byte,1);return;
		default:return;}
	}

	/**
	 * @brief 向OLED发送多个字节(IIC_Poll模式)
	 * @param Bytes 待发送的字节
	 * @param CommandOrData OLED_Command写命令, OLED_Data写数据
	 * @param size 发送的字节数
	 * @retval void
	*/
	void OLED_SendBytes_Poll( uint8_t* Bytes,uint8_t CommandOrData, uint8_t Size){
		while (IICtoOLED.State!=HAL_I2C_STATE_READY);	// 检查接口状态
		switch (CommandOrData){
			case OLED_Data:HAL_I2C_Mem_Write(&IICtoOLED, 0x78,0x00,I2C_MEMADD_SIZE_8BIT,Bytes,1,0x10);break;  		// DC高--发送数据
			case OLED_Command:HAL_I2C_Mem_Write(&IICtoOLED, 0x78,0x40,I2C_MEMADD_SIZE_8BIT,Bytes,1,0x10);break;		// DC低--发送命令
			default:return;}
	}

	/**
	 * @brief 向OLED发送多个字节(IIC_DMA模式)
	 * @param Bytes 待发送的字节
	 * @param CommandOrData OLED_Command写命令, OLED_Data写数据
	 * @param size 发送的字节数
	 * @retval void
	*/
	void OLED_SendBytes_DMA( uint8_t* Bytes,uint8_t CommandOrData, uint8_t Size){
		while (IICtoOLED.State!=HAL_I2C_STATE_READY);	// 检查接口状态
		switch (CommandOrData){
			case OLED_Data:HAL_I2C_Mem_Write_DMA(&IICtoOLED, 0x78,0x00,I2C_MEMADD_SIZE_8BIT,Bytes,1);break;  		// DC高--发送数据
			case OLED_Command:HAL_I2C_Mem_Write_DMA(&IICtoOLED, 0x78,0x40,I2C_MEMADD_SIZE_8BIT,Bytes,1);break;		// DC低--发送命令
			default:return;}
	}

	/**
	 * @brief 将显存更新到OLED屏幕(IIC_Poll模式)
	 * @retval void
	*/
	void OLED_Refresh_Poll(void){
		while (IICtoOLED.State!=HAL_I2C_STATE_READY);	// 检查接口状态
		uint8_t i;	// 循环变量
		for(i=0;i<8;i++){
			HAL_I2C_Mem_Write(&IICtoOLED, 0x78,0x00,I2C_MEMADD_SIZE_8BIT,(uint8_t *)OLED_RefComBuf[i],3,0x100);
			HAL_I2C_Mem_Write(&IICtoOLED, 0x78,0x40,I2C_MEMADD_SIZE_8BIT,OLED_GRAMBuf[i],128,0x1000);
		}
		FPS_Count++;memcpy(OLED_AnimaBuf,OLED_GRAMBuf,1024);	// 拷贝当前帧
	}

	/**
	 * @brief 将显存更新到OLED屏幕(IIC_DMA模式)
	 * @note 可能产生未定义结果!
	 * @retval void
	*/
	void OLED_Refresh_DMA(void)
	{
		while (IICtoOLED.State != HAL_I2C_STATE_READY);	// if IIC_state == Ready;
		HAL_I2C_Mem_Write_DMA(&IICtoOLED, 0x78,0x40,I2C_MEMADD_SIZE_8BIT,*OLED_GRAMBuf,8*128);
		FPS_Count++;memcpy(OLED_AnimaBuf,OLED_GRAMBuf,1024);	// 拷贝当前帧
	}

	/**
	 * @brief OLED初始化
	 * @retval void
	*/
	void OLED_Init(void){
		FPS=0;FPS_Count=0;ms_Count = 0;
		HAL_Delay(15);	// 等待OLED上电启动
		OLED_BufferClear();
		OLED_SendBytes_Poll((uint8_t *)OLED_Init_CMD,OLED_Command,29);
		while (IICtoOLED.State != HAL_I2C_STATE_READY );	// CPU等待数据传输
	}

#endif // IICtoOLED

/*                             */
/* ---------IIC通讯专用-------- */

