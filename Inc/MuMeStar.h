/**************************************************************************
* 文件：MutiMenu.h
* 作者：StarDY
* 邮箱：1308102491@qq.com
* 描述：OLED多级菜单
***************************************************************************/

#include "main.h"
#include "OLEDSD.h"

#ifndef _MMS_H_
#define _MMS_H_

#define Add_Game_Dinosaur 1

// OLED相关设置
#define OLED_Width 128
#define OLED_Height 64

/* --------下面是按键与状态灯接口宏定义-------- */
/*                                           */
#define KEY_Active_Volt         GPIO_PIN_RESET   // GPIO_PIN_RESET--按键有效电平为低电平, GPIO_PIN_SET--按键有效电平为高电平
#define OLED_Defult_Brightness  0x80  // 屏幕默认亮度, 范围0x01~0xFF, 50% 对应0x80
#define OLED_Defult_Fontsize    size_1206    // 菜单默认字号, 范围{size_0806, size_1206, size_1608}
#define OLED_Defult_ColorMode   Black   // 默认屏幕颜色，Black--白字黑底，White---黑字白底
#define OLED_Defult_RefreshRate 8 // 默认屏幕刷新率，0x0~0xF(即0~15)

// 状态灯
#define Led_Pin       GPIO_PIN_13
#define Led_GPIO_Port GPIOC
#define LED_Pin       GPIO_PIN_13
#define LED_GPIO_Port GPIOC

// previous键
#define Key_previous_Pin       GPIO_PIN_0
#define Key_previous_GPIO_Port GPIOA
#define Key_previous_EXTI_IRQn EXTI0_IRQn

// enter键
#define Key_enter_Pin       GPIO_PIN_1
#define Key_enter_GPIO_Port GPIOA
#define Key_enter_EXTI_IRQn EXTI1_IRQn

// next键
#define Key_next_Pin       GPIO_PIN_2
#define Key_next_GPIO_Port GPIOA
#define Key_next_EXTI_IRQn EXTI2_IRQn

// return键
#define Key_return_Pin       GPIO_PIN_3
#define Key_return_GPIO_Port GPIOA
#define Key_return_EXTI_IRQn EXTI3_IRQn
/*                                           */
/* --------上面是按键与状态灯接口宏定义-------- */
 
/* ----------- 下面是菜单字号结构 -------- */
enum FONTSIZE{
  size_0806,
  size_1206,
  size_1608
};
enum COLORMODE{
  Black,
  White
};

typedef struct Size
{
  const uint8_t size_title;
  const uint8_t size_content;
  const uint8_t row_number;
  const uint8_t* Mysize_array;
}Mysize_typedef;

/* ----------- 上面是菜单字号结构 -------- */

/* ----------- 下面是菜单底层结构 -------- */
enum MenuProperties{  // 定义菜单属性
  Menu_Parent=1,
  Menu_Data,
  Menu_Once,
  Menu_Loop,
};
enum KEY_NUM{  // 定义键值
  Zero,
  Prevoius,
  Enter,
  Next,
  Return=5,
};
enum LOOP_STATE{
  Loop_Run=1,
  Loop_Stop
};
// 定义菜单底层结构体
typedef struct Menu{
  char *Name;		                      // 当前节点标题
  const struct Menu *Parent;	        // 父节点结构体数组
  const struct Menu *Child;  	        // 子节点结构体数组	
  void (*func)(void);			            // 子节点函数(进入子节点时调用)
	uint8_t Child_nodes_number;		      // 子节点的子节点数 
  enum MenuProperties Child_Menuproperty;   // 子节点属性
} Menu_typedef;
/* ----------- 上面是菜单底层结构 -------- */


/* -----------自定义菜单节点函数声明------------ */
void Func_About(void);
void Func_Sayhello(void);
void Func_Brightness_enter(void);
void Func_Brightness_set(void);
void Func_ColorMode_enter(void);
void Func_ColorMode_set(void);
void Func_RefreshRate_enter(void);
void Func_RefreshRate_set(void);
void Func_MirrorFlipHo_enter(void);
void Func_MirrorFlipHo_set(void);
void Func_MirrorFlipVer_enter(void);
void Func_MirrorFlipVer_set(void);
void Func_Fontsize_enter(void);
void Func_Fontsize_set(void);
void Func_Smile_enter(void);
void Func_Smile_run(void);
/* -----------自定义菜单节点函数声明------------ */


/* ------------- 菜单内部函数声明 -------------- */
void Multimenu_Init(void);
uint8_t Get_MenuIndex(Menu_typedef* menu);
uint8_t Get_MenuNameLenth(Menu_typedef* menu);
void Invalid_Operation(void);
void Switch_Menu(void);

// 按键控制
void KEY_Pressed(uint8_t GPIO_pin);
void Menu_Handler(void);
void KEY_Parent_pressed(void);
void KEY_Parent_next(void);
void KEY_Parent_enter(void);
void KEY_Parent_previous(void);
void KEY_Parent_return(void);
void KEY_Data_pressed(void);
void KEY_Once_pressed(void);
void KEY_Loop_pressed(void);
/* ------------- 菜单内部函数声明 -------------- */

/* ------------- 下面是小恐龙游戏 -------------- */
#if defined(Add_Game_Dinosaur)

#define OLED_Width 128
#define OLED_Height 64

#define BASE_LINE_X 0
#define BASE_LINE_Y OLED_Height-2
#define BASE_LINE_X1 OLED_Width-1

#define DINO_INIT_X 10      // Dino initial spawn location
#define DINO_INIT_Y BASE_LINE_Y - 23  // Dino initial spawn location

// Number of pixel dino will jump
#define JUMP_PIXEL 32

// 全局声明
enum GAME_STATE{
    Game_Stop=1,
    Game_Run,
    Game_Over
};

void Func_Dinosaur_enter(void);
void Func_Dinosaur_run(void);
void Dinosaur_Stop_Handler(void);
void Dinosaur_Run_Handler(void);
void Dinosaur_Over_Handler(void);
void Dinosaur_MoveDino(int16_t y, int type);
void Dinosaur_MoveTree(void);
void Dinosaur_DisplayScore(int score);

#endif // Add_Game_Dinosaur
/* ------------- 上面是小恐龙游戏 -------------- */



#endif // _MMS_H_
