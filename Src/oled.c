/**
 * 本函数库基于下面 OLED 型号进行编写: 
 * 0.96寸128x64屏幕，SSD1306控制芯片，IIC通信协议，4管脚
*/

#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"
#include "i2c.h"
#include "gpio.h"



u8 OLED_GRAM[144][8];
 
/**
 * @brief 屏幕是否反色
 * @param i 0为正常，1为反色(白底黑字)
 * @retval void
*/
void OLED_ColorTurn(u8 i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xA6,OLED_CMD);// 正常显示
		}
	if(i==1)
		{
			OLED_WR_Byte(0xA7,OLED_CMD);// 反色显示
		}
}

/**
 * @brief 屏幕是否反转
 * @param i 0为正常，1为翻转
 * @retval void
*/
void OLED_DisplayTurn(u8 i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xC8,OLED_CMD);//正常显示
			OLED_WR_Byte(0xA1,OLED_CMD);
		}
	if(i==1)
		{
			OLED_WR_Byte(0xC0,OLED_CMD);//反转显示
			OLED_WR_Byte(0xA0,OLED_CMD);
		}
}



/**
 * @brief 向OLED发送一字节命令或数据
 * @param dat 发送的数据
 * @param mode 0写命令, 1写数据
 * @retval void
*/
void OLED_WR_Byte(u8 dat,u8 mode)
{

	if(mode){
		 HAL_I2C_Mem_Write(I2CtoOLED, 0x78,0x40,I2C_MEMADD_SIZE_8BIT,&dat,1,0x100);
	}
 	else{
		HAL_I2C_Mem_Write(I2CtoOLED, 0x78,0x00,I2C_MEMADD_SIZE_8BIT,&dat,1,0x100);
	}

}

/**
 * @brief 打开OLED屏幕
 * @retval void
*/
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵使能
	OLED_WR_Byte(0x14,OLED_CMD);//开启电荷泵
	OLED_WR_Byte(0xAF,OLED_CMD);//点亮屏幕
}

/**
 * @brief 关闭OLED屏幕
 * @retval void
*/
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵使能
	OLED_WR_Byte(0x10,OLED_CMD);//关闭电荷泵
	OLED_WR_Byte(0xAE,OLED_CMD);//关闭屏幕
}
 
/**
 * @brief 将显存更新到OLED屏幕
 * @retval void
*/
void OLED_Refresh(void)
{
	u8 i,n;
	for(i=0;i<8;i++)
	{
		OLED_WR_Byte(0xb0+i,OLED_CMD); //设置行起始地址
		OLED_WR_Byte(0x00,OLED_CMD);   //设置低列起始地址
		OLED_WR_Byte(0x10,OLED_CMD);   //设置高列起始地址
		for(n=0;n<128;n++)
		{
			OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
		}
 	}
}

/**
 * @brief 纵向刷新屏幕,效果为一条横向纵向扫过屏幕
 * @param mode 1从上至下, 0从下至上
 * @param line 1为亮竖线, 0为暗竖线
 * @param speed 动画速度, 范围[1,5] 
 * @retval void
*/
void OLED_Refresh_longitudinal(void){

}

/**
 * @brief 横向刷新屏幕,效果为一条竖线横向扫过屏幕
 * @param mode 1从左至右, 0从右至左
 * @param speed 动画速度, 范围[1,5] 
 * @retval void
*/
void OLED_Refresh_transverse(void){

}


/**
 * @brief 清空显存但不刷新屏幕
 * @retval void
*/
void OLED_Buffer_clear(void){
	u8 i,n;
	// 执行快速清除, 并 clear 显存
	for(i=0;i<8;i++)
	{
		OLED_WR_Byte(0xb0+i,OLED_CMD); //设置行起始地址
		OLED_WR_Byte(0x00,OLED_CMD);   //设置低列起始地址
		OLED_WR_Byte(0x10,OLED_CMD);   //设置高列起始地址
		for(n=0;n<128;n++){OLED_GRAM[n][i] = 0;}
 	}
}

/**
 * @brief 快速清屏函数(此函数似乎有一些问题)
*/
void OLED_Clear_quick(void)
{
	u8 i,n;
	// 执行快速清除, 并 clear 显存
	for(i=0;i<8;i++)
	{
		OLED_WR_Byte(0xb0+i,OLED_CMD); //设置行起始地址
		OLED_WR_Byte(0x00,OLED_CMD);   //设置低列起始地址
		OLED_WR_Byte(0x10,OLED_CMD);   //设置高列起始地址
		for(n=0;n<128;n++)
		{
			if(OLED_GRAM[n][i]){
				OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
				OLED_GRAM[n][i] = 0;
				}
		}
 	}
}

void OLED_Clear(void)
{
	u8 i,n;
	// 执行快速清除, 并 clear 显存
	for(i=0;i<8;i++)
	{
		OLED_WR_Byte(0xb0+i,OLED_CMD); //设置行起始地址
		OLED_WR_Byte(0x00,OLED_CMD);   //设置低列起始地址
		OLED_WR_Byte(0x10,OLED_CMD);   //设置高列起始地址
		for(n=0;n<128;n++)
		{
			OLED_GRAM[n][i] = 0;
			OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);	
		}
 	}
}



/**
 * @brief 作一个像素点
 * @param x 横坐标(从左往右)
 * @param y 纵坐标(从上往下)
 * @retval void
*/
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 i,m,n;
	i=y/8;
	m=y%8;
	n=1<<m;
	if(t){OLED_GRAM[x][i]|=n;}
	else
	{
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
		OLED_GRAM[x][i]|=n;
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
	}
}

/**
 * @brief 作一条直线
 * @param x1 起点横坐标
 * @param y1 起点纵坐标
 * @param x2 终点横坐标
 * @param y2 终点纵坐标
 * @param mode 1作亮直线, 0作暗直线
 * @retval void
*/
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode)
{
	u16 t; 
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
	for(t=0;t<distance+1;t++)
	{
		OLED_DrawPoint(uRow,uCol,mode); //画点
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
void OLED_DrawCircle(u8 x,u8 y,u8 r)
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
 * @brief 显示字符串
 * @param x 横坐标
 * @param y 纵坐标
 * @param chr 要显示的字符
 * @param size1 字号, {8,12,16,24}依次对应{08*06,12*06,16*08,24*12}
 * @param mode 0为反色显示, 1为正常显示
 * @retval void
*/
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode)
{
	u8 i,m,temp,size2,chr1;
	u8 x0=x,y0=y;
	if(size1==8)size2=6;
	else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //得到字体一个字符对应点阵集所占的字节数
	chr1=chr-' ';  //计算偏移后的值
	for(i=0;i<size2;i++)
	{
		if(size1==8){temp=asc2_0806[chr1][i];} //调用0806字体
		else if(size1==12){temp=asc2_1206[chr1][i];} //调用1206字体
		else if(size1==16){temp=asc2_1608[chr1][i];} //调用1608字体
		else if(size1==24){temp=asc2_2412[chr1][i];} //调用2412字体
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((size1!=8)&&((x-x0)==size1/2))
		{x=x0;y0=y0+8;}
		y=y0;
  }
}


/**
 * @brief 显示字符串
 * @param x 横坐标
 * @param y 纵坐标
 * @param chr 要显示的字符串
 * @param size1 字号, {8,12,16,24}依次对应{08*06,12*06,16*08,24*12}
 * @param mode 0为反色显示, 1为正常显示
 * @retval void
*/
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode)
{
	while((*chr>=' ')&&(*chr<='~'))//判断是否非法字符!
	{
		OLED_ShowChar(x,y,*chr,size1,mode);
		if(size1==8)x+=6;
		else x+=size1/2;
		chr++;
  	}
}

/**
 * @brief 在某一行居中显示字符串
 * @param x 横坐标
 * @param y 纵坐标
 * @param chr 要显示的字符串
 * @param size1 字号, {8,12,16,24}依次对应{08*06,12*06,16*08,24*12}
 * @param mode 0为反色显示, 1为正常显示
 * @retval void
*/
void OLED_ShowString_rowcentering(u8 y,u8 *chr,u8 size1,u8 mode){
	uint8_t i=0, size=0, n = 0, lenth = 0;
	while((*chr>=' ')&&(*chr<='~')){n++;chr++;} // 获取字符串长度
	chr -= n;
	if(size1 == 8){size=6;}
	else{size=size1/2;}
	lenth = n*size;
	if( lenth>128 ){
		OLED_Buffer_clear();
		OLED_ShowString(16,8,"too long",24,0);
	}
	else{
		while((*chr>=' ')&&(*chr<='~'))
		{
			OLED_ShowChar(64-lenth/2 + i*size,y,*chr,size1,mode);
			i++;chr++;
  		}
	}

}

/**
 * @brief 幂函数
 * @param m 底数
 * @param n 指数
 * @retval m^n
*/
u32 OLED_Pow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}

/**
 * @brief 以二进制显示数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(从右向左计位)
 * @param size 字号, {8,12,16,24}依次对应{08*06,12*06,16*08,24*12}
 * @param mode 反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_bin(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode){
	uint8_t i, width;
	switch (size1)
	{
	case 8: width = 6; break;
	case 12: width = 6; break;
	case 16: width = 8; break;
	case 24: width = 12; break;
	default:return;
	}
	for (i = 0; i < len; i++)							
	{
		OLED_ShowChar(x+width*i, y , (num/OLED_Pow(2, len - i - 1))%2+'0', size1, mode);
	}
}

/**
 * @brief 以十进制显示数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(左侧自动补零)
 * @param size 字号, {8,12,16,24}依次对应{08*06,12*06,16*08,24*12}
 * @param mode 反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_dec(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode)
{
	uint8_t i, width;
	switch (size1)
	{
	case 8: width = 6; break;
	case 12: width = 6; break;
	case 16: width = 8; break;
	case 24: width = 12; break;
	default:return;
	}
	for (i = 0; i < len; i++)							
	{
		OLED_ShowChar(x+width*i, y , (num/OLED_Pow(10,len-i-1))%10+'0', size1, mode);
	}
}


/**
 * @brief 以十六进制显示数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(从右向左计位)
 * @param size 字号, {8,12,16,24}依次对应{08*06,12*06,16*08,24*12}
 * @param mode 反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_hex(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode){
	uint8_t i, width, decnum;
	switch (size1)
	{
	case 8: width = 6; break;
	case 12: width = 6; break;
	case 16: width = 8; break;
	case 24: width = 12; break;
	default:return;
	}
	for (i = 0; i < len; i++)							
	{
		decnum = num / OLED_Pow(16, len - i - 1) % 16;
		if (decnum < 10){OLED_ShowChar(x+width*i, y , decnum + '0', size1, mode);}
		else{OLED_ShowChar(x+width*i, y , decnum - 10 + 'A', size1, mode);}
	}
}

/**
 * @brief 显示有符号数字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 要显示的数字
 * @param len 显示位数(不计正负号)
 * @param size 字号, {8,12,16,24}依次对应{08*06,12*06,16*08,24*12}
 * @param mode 反色: 1正常;0反色;
 * @retval void
*/
void OLED_ShowNum_signeddec(u8 x,u8 y,int32_t num,u8 len,u8 size1,u8 mode){
	uint8_t width;
	switch (size1)
	{
	case 8: width = 6; break;
	case 12: width = 6; break;
	case 16: width = 8; break;
	case 24: width = 12; break;
	default:return;
	}
	if(num >=0){
		OLED_ShowChar(x,y, '+', size1, mode);
		OLED_ShowNum_dec(x+width, y, num, len, size1, mode);
	}
	else{
		OLED_ShowChar(x,y, '-', size1, mode);
		OLED_ShowNum_dec(x+width, y, -num, len, size1, mode);
	}
}



/**
 * @brief 显示单个汉字
 * @param x 横坐标
 * @param y 纵坐标
 * @param num 汉字对应的序号
 * @param size1 字号, {12,16,24}依次对应{12*12,16*16,24*24}
 * @param mode 1正常, 0反色
 * @retval void
*/
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode)
{
	u8 m,temp;
	u8 x0=x,y0=y;
	const u8* TEMP; 
	u16 i,size3=size1*(size1/8 + ((size1%8)?1:0));  //得到字体一个字符对应点阵集所占的字节数
	switch (size1)
	{
	case 12: TEMP = *(Chinese_12+num);break;
	case 16: TEMP = *(Chinese_16+num);break;
	case 24: TEMP = *(Chinese_24+num);break;
	case 32: TEMP = *(Chinese_32+num);break;
	default: return;	// 非法参数直接return
	}
	for(i=0;i<size3;i++)
	{
		temp = *(TEMP+i);
		for(m=0;m<8;m++)
		{
			if(temp&(0x01<<m)){OLED_DrawPoint(x,y,mode);}
			else{OLED_DrawPoint(x,y,!mode);}
			y++;
		}
		x++;
		y=y0;
		if((x-x0)==size1){x=x0;y0=y0+8;y=y0;}
	}
}


 
/**
 * @brief Show one picture
 * @param x (起始)横坐标
 * @param y (起始)纵坐标
 * @note 起始坐标位于字符/图片的左上角
 * @param sizex 图片横宽 
 * @param sizey 图片纵高 
 * @param BMP 图片对应数组名 
 * @param mode 1正常, 0反色显示 
 * @retval void
*/
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,const u8 BMP[],u8 mode)
{
	u16 j=0;
	u8 i,n,temp,m;
	u8 x0=x,y0=y;
	sizey=sizey/8+((sizey%8)?1:0);
	for(n=0;n<sizey;n++)
	{
		 for(i=0;i<sizex;i++)
		 {
				temp=BMP[j];
				j++;
				for(m=0;m<8;m++)
				{
					if(temp&0x01)OLED_DrawPoint(x,y,mode);
					else OLED_DrawPoint(x,y,!mode);
					temp>>=1;
					y++;
				}
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y0=y0+8;
				}
				y=y0;
     }
	 }
}

/**
 * @brief 滚动显示中文
 * @param num 汉字个数
 * @param space 显示间隔 16*space
 * @param mode 1正常, 0反色
 * @retval void
*/
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode)
{
	u8 i,n,t=0,m=0,r;
	while(1)
	{
		if(m==0)
		{
	    OLED_ShowChinese(128,24,t,16,mode); //写入一个汉字保存在OLED_GRAM[][]数组中
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
								OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
							}
						}
           OLED_Refresh();
				 }
        t=0;
      }
		m++;
		if(m==16){m=0;}
		for(i=1;i<144;i++)   //实现左移
		{
			for(n=0;n<8;n++)
			{
				OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
			}
		}
		OLED_Refresh();
	}
}

/**
 * @brief 利用SSD1306芯片内置方法开启水平滚动(!不改变显存!)
 * @param start_page 起始页
 * @param end_page 终止页
 * @param frame 时间间隔(滚动周期)
 * @param mode 0向左滚动(从右向左),1向右滚动(从左向右) 
 * @retval void
*/
void OLED_Scroll_InsiderHorizental_Enable(uint8_t start_page,uint8_t end_page,Roll_Frame frame,uint8_t mode){
	OLED_WR_Byte(0x2E,OLED_CMD);	// 必须先关闭滚动
	OLED_WR_Byte(mode ? 0x26 : 0x27,OLED_CMD);	// 1向右滚动，0向左滚动
	OLED_WR_Byte(0x00,OLED_CMD);	// 发送一个虚拟字节
	OLED_WR_Byte(start_page & 0x07,OLED_CMD);	//起始页 0
	OLED_WR_Byte(frame & 0x07,OLED_CMD);	//滚动时间间隔
	OLED_WR_Byte(end_page & 0x07,OLED_CMD);	//终止页 7
	OLED_WR_Byte(0x00,OLED_CMD);	// 发送虚拟字节
	OLED_WR_Byte(0xFF,OLED_CMD);	// 发送虚拟字节
	OLED_WR_Byte(0x2F,OLED_CMD);   //开启滚动
}
void OLED_Scroll_InsiderHorizental_disable(void){	
	OLED_WR_Byte(0x2E,OLED_CMD);	// 关闭滚动
}


// 宏定义最多滚动显示中文长度(可以直接更改以显示更多字符)
#define     LONG_CN_LEN          20

uint8_t my_strlen(uint8_t* str)
{
    uint8_t count = 0; //计数
    while(*str != '\0')
    {
        count++;
        str++;
    }
    return count;
}


/**	此函数目前不可用
  * @brief  OLED_Show_LongRoll_CN显示长中文字符串并向左滚动（16x16字符串0.96寸最多显示8个）
  * @param  x 中文字符串显示的起始位置x(0-20*16)
  * @param  y 中文字符串显示的起始位置y(0-7)
  * @param  str 中文字符串的指针
  * @param  mode 模式1:滚动一圈后原位置开始继续滚动; 模式2:滚动一圈后屏幕末尾位置开始继续滚动       
  * @retval void
  */
void OLED_Scroll_LongCN_Enable(uint16_t x,uint8_t y,uint8_t *str,uint8_t mode)
{
	uint16_t CN_Last_Pixel = (my_strlen(str)/2)*16;
	/* 将数据直接放入长数组内(最大长度依据LONG_CN_LEN的值，默认为20) */
	// 字符串宽度是16
	uint8_t hsize=16;
	// 移动的像素点
	uint16_t move_pixel = 0;
	// 模式2的标志位
	uint8_t mode2_flag = 0;
	
	// 对长数组进行赋值
	y = y*8;
	while(*str!='\0')
	{
		if(hsize == 16)
		{
			// GUI_ShowFont16(x,y,str,2);	
			// OLED_ShowChar(x,y,*str,2);
		}
		else
		{
			return;
		}
		x+=hsize;
		str+=2;
	}
 
	// 死循环进行向左移动阅读
	while(1)
	{
		for(uint8_t i=0;i<8;i++)
		{
			// 起始点开始全部刷新
			OLED_WR_Byte(0xb0+i,OLED_CMD);	//设置页地址
			OLED_WR_Byte(0x00,OLED_CMD);	//设置显示位置—列低地址
			OLED_WR_Byte(0x10,OLED_CMD);	//设置显示位置—列高地址
			for(uint16_t n=move_pixel;n<128 + move_pixel;n++)  //写一PAGE的GDDRAM数据
			{
				if(mode == 1 || mode2_flag == 0)
				{
					// 设置起始点坐标
					OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA);
				}
				else if(mode == 2)
				{
					if(n<128)
					{
						OLED_WR_Byte(0x00,OLED_DATA);
					}
					else
					{
						// 设置起始点坐标
						OLED_WR_Byte(OLED_GRAM[n-128][i],OLED_DATA);
					}	
				}
			}
		}
		// 每次移动的像素点为1
		move_pixel++;
		HAL_Delay(30);// 这里变小可以更改移动速度
		// 字符串长度走完正好留一段空白(8个字符)
		if(move_pixel > CN_Last_Pixel)
		{
			move_pixel = 0;
			if(mode == 2)
			{
				mode2_flag = 1;
			}
		}
		
		// 模式2分为两个阶段才能重复显示，因此需要一个标志位
		if(mode2_flag == 1 && move_pixel > 128)
		{
			move_pixel = 0;
			mode2_flag = 0;
		}	
	}
}

/**
 * @brief OLED初始化
 * @retval void
*/
void OLED_Init(void)
{
	OLED_WR_Byte(0xAE,OLED_CMD);	// 熄灭屏幕
	OLED_WR_Byte(0x40,OLED_CMD);	// 从RAM中哪一行起读取显示内容--set start line address Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);	// 进入 contrast control register 设置--set contrast control register
	OLED_WR_Byte(0xCF,OLED_CMD);	// 设置亮度0x00~0xff--Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1,OLED_CMD);	// 设置屏幕左右翻转,0xa1正常,0xa0左右反置--Set SEG/Column Mapping     
	OLED_WR_Byte(0xC8,OLED_CMD);	// 设置屏幕上下翻转,0xc8正常,0xc0上下反置--Set COM/Row Scan Direction,    
	OLED_WR_Byte(0xA6,OLED_CMD);	// 设置显示模式,0xA6正常显示,0xA7反相显示--set normal display
	OLED_WR_Byte(0xA8,OLED_CMD);	// --set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f,OLED_CMD);	// --1/64 duty
	OLED_WR_Byte(0xD3,OLED_CMD);	// 进入偏移量设置--set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);	// 设置无偏移--not offset
	OLED_WR_Byte(0xd5,OLED_CMD);	// 设置时钟分频因子,震荡频率--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80,OLED_CMD);	// bit[3:0],分频因子;bit[7:4],震荡频率--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9,OLED_CMD);	// --set pre-charge period
	OLED_WR_Byte(0xF1,OLED_CMD);	// Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA,OLED_CMD);	// --set com pins hardware configuration
	OLED_WR_Byte(0x12,OLED_CMD);	
	OLED_WR_Byte(0xDB,OLED_CMD);	// --set vcomh
	OLED_WR_Byte(0x30,OLED_CMD);	// Set VCOM Deselect Level
	OLED_WR_Byte(0x20,OLED_CMD);	// 设置寻址模式	//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);	// 页寻址模式
	OLED_WR_Byte(0x8D,OLED_CMD);	// 进入电荷泵设置--set Charge Pump enable/disable
	OLED_WR_Byte(0x14,OLED_CMD);	// 0x14开启,0x10关闭--0x14 enable,0x10 disable
	OLED_Buffer_clear();
	OLED_WR_Byte(0xAF,OLED_CMD);	// 0xAF点亮屏幕,0xAE熄灭屏幕
}

