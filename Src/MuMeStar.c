/**************************************************************************
* 文件：MutiMenu.c
* 作者：StarDY
* 邮箱：1308102491@qq.com
* 描述：OLED多级菜单
***************************************************************************/

#include "main.h"
#include "OLEDSD.h"
#include "MuMeStar.h"
#include "MuMeStar_Data.h"


/*-------------- 下面是菜单相关设置 -------------- */ 
static const uint8_t Mysize_08[] = {12,20,28,36,44,52};
static const uint8_t Mysize_12[] = {16,28,40,52};
static const uint8_t Mysize_16[] = {24,40};

static const Mysize_typedef Mysize[] = {
    {12,8, 6, Mysize_08},
    {16,12, 4, Mysize_12},
    {24,16, 2, Mysize_16}
};

/*-------------- 上面是菜单相关设置 -------------- */


/* -----------下面是自定义全局变量---------- */
// Smile(Loop)专用
static uint8_t smile_x = 0;  // Smile_Image 横坐标
static uint8_t eye_x = 0;    // Eyes_x
static uint8_t eye_y = 0;    // Eyes_y
/* -----------上面是自定义全局变量---------- */


/* -----------下面是菜单内部全局变量---------- */
enum KEY_NUM KEY_num = Zero;            // 当前键值, KEY_Pressed()和Menu_HAndler()专用
enum KEY_NUM Jumped_key = Zero;         // 检查是否忽略了一次键值传递, 防止按键失灵 
enum LOOP_STATE Loop_State = Loop_Run;  // Loop型菜单专用, 表示当前循环状态
static uint8_t Insert = 0;              // Switch_Menu()专用, 判断光标位置, 0不变, 1enter, 2return, 
static uint8_t Current_showrange = 0;   // Switch_Menu()专用, 判断当前显示范围，屏幕最大显示行数为 Mysize[fontsize].row_number
static uint8_t Last_showrange = 0;      // Switch_Menu()专用
static uint8_t UserChoose = 0; 		    // Switch_Menu()专用, 用户选择
static uint8_t Current_CL = 0;          // CursorLine, 当前光标所在行
static uint8_t Last_CL = 0;             // 上一次光标所在行


// settings 参数
static uint8_t brightness        = OLED_Defult_Brightness;     // 默认屏幕亮度
static uint8_t colormode         = OLED_Defult_ColorMode;      // 默认屏幕颜色
static uint8_t refreshrate       = OLED_Defult_RefreshRate;    // 默认屏幕刷新率
static uint8_t mirrorflipHo      =  No;                        // 默认水平翻转
static uint8_t mirrorflipVer     = No;                         // 默认垂直翻转
static uint8_t fontsize          = OLED_Defult_Fontsize;       // 默认菜单字号
static uint8_t brightness_set    = OLED_Defult_Brightness;     // "Brightness" 专用，用于调节屏幕当前亮度
static uint8_t colormode_set     = OLED_Defult_ColorMode;      // "Color Mode" 专用，用于调节屏幕颜色模式
static uint8_t refreshrate_set   = OLED_Defult_RefreshRate;    // "Refresh Rate" 专用，用于调节屏幕刷新率
static uint8_t mirrorflipHo_set  = No;                         // "MirrorFlip Ho" 专用
static uint8_t mirrorflipVer_set = No;                         // "MirrorFlip Ver" 专用
static uint8_t fontsize_set      = OLED_Defult_Fontsize;       // "Fontsize" 专用，用于调节菜单当前字号
/* -----------上面是菜单全局变量---------- */


/* --------------------- 下面是菜单栏定义 -------------------- */
/*                                                           */
/* 菜单节点初始化(节点声明) */
#define Main_Child_nodesnumber 6    // 宏定义主菜单子节点数,方便操作
static const Menu_typedef 
Main[],
    Main_Settings[],
        Main_Settings_Fontsize[1],
        Main_Settings_Brightness[1],     
        Main_Settings_ColorMode[1],     
        Main_Settings_RefreshRate[1],     
        Main_Settings_MirrorFlipHo[1],     
        Main_Settings_MirrorFlipVer[1],     
    Main_Hello[],
        Main_Hello_Sayhello[1],
        Main_Hello_Smile[1],
    Main_About[1],
    Main_Game[],
        Main_Game_Dinosaur[1],
    Main_Menu5[],
    Main_Menu6[];
/* Menu_Root, Menu_0 */
static Menu_typedef* Menu_Pointer =  (Menu_typedef *)Main; // 设置当前菜单为 Main 并初始化

static const Menu_typedef Main[Main_Child_nodesnumber] = {				
    {"Main", NULL, Main_Settings,  Switch_Menu,    6, Menu_Parent},
    {"Main", NULL, Main_Hello,     Switch_Menu,    2, Menu_Parent},
    {"Main", NULL, Main_About,     Func_About,   1, Menu_Once  },
    {"Main", NULL, Main_Game,      Switch_Menu,    1, Menu_Parent},
    {"Main", NULL, Main_Menu5,     0,            0, Menu_Parent},
    {"Main", NULL, Main_Menu6,     0,            0, Menu_Parent},
};

/* Menu_1 */
static const Menu_typedef Main_Settings[] = {				
    {"Settings", Main,  Main_Settings_Fontsize,      Func_Fontsize_enter,       1, Menu_Data},
    {"Settings", Main,  Main_Settings_Brightness,    Func_Brightness_enter,     1, Menu_Data},
    {"Settings", Main,  Main_Settings_ColorMode,     Func_ColorMode_enter,      1, Menu_Data},
    {"Settings", Main,  Main_Settings_RefreshRate,   Func_RefreshRate_enter,    1, Menu_Data},
    {"Settings", Main,  Main_Settings_MirrorFlipHo,  Func_MirrorFlipHo_enter,   1, Menu_Data},
    {"Settings", Main,  Main_Settings_MirrorFlipVer, Func_MirrorFlipVer_enter,  1, Menu_Data},
};

static const Menu_typedef Main_Hello[]  = {{"Hello",  Main, Main_Hello_Sayhello, Func_Sayhello,    1, Menu_Once},
                                           {"Hello",  Main, Main_Hello_Smile,    Func_Smile_enter, 1, Menu_Loop}};
static const Menu_typedef Main_About[1] = {{"About",  Main, NULL,                Invalid_Operation,   0,}};
static const Menu_typedef Main_Game[1]  = {{"Game",   Main, Main_Game_Dinosaur,  Func_Dinosaur_enter, 0,Menu_Loop}};
static const Menu_typedef Main_Menu5[1] = {{"Menu5",  Main, NULL,                Invalid_Operation,   0,}};
static const Menu_typedef Main_Menu6[1] = {{"Menu6",  Main, NULL,                Invalid_Operation,   0,}};

/* Menu_2*/
static const Menu_typedef Main_Game_Dinosaur[1]          ={{"Dinosaur",       Main_Game,     NULL, Func_Dinosaur_run,      0,}};
static const Menu_typedef Main_Settings_Fontsize[1]      ={{"Fontsize",       Main_Settings, NULL, Func_Fontsize_set,      0,}};
static const Menu_typedef Main_Settings_Brightness[1]    ={{"Brightness",     Main_Settings, NULL, Func_Brightness_set,    0,}};
static const Menu_typedef Main_Settings_ColorMode[1]     ={{"ColorMode",      Main_Settings, NULL, Func_ColorMode_set,     0,}};
static const Menu_typedef Main_Settings_RefreshRate[1]   ={{"Refresh Rate",   Main_Settings, NULL, Func_RefreshRate_set,   0,}};
static const Menu_typedef Main_Settings_MirrorFlipHo[1]  ={{"MirrorFlip-Ho",  Main_Settings, NULL, Func_MirrorFlipHo_set,  0,}};
static const Menu_typedef Main_Settings_MirrorFlipVer[1] ={{"MirrorFlip-Ver", Main_Settings, NULL, Func_MirrorFlipVer_set, 0,}};
static const Menu_typedef Main_Hello_Sayhello[1]         ={{"Sayhello",       Main_Hello,    NULL, Invalid_Operation,      0,}};
static const Menu_typedef Main_Hello_Smile[1]            ={{"Smile",          Main_Hello,    NULL, Func_Smile_run,         0,}};
/*                                                           */
/* ---------------------- 上面是菜单栏定义 ------------------- */


/* -------------下面是菜单节点函数------------- */
/**
 * @brief Once型节点函数
 * @note 在屏幕上显示"Hello world!"
 * @retval void
*/
void Func_Sayhello(void){
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(20,"Hello MuMeStar!",16,1);
    OLED_Refresh_Poll();
}

/**
 * @brief Once型节点函数
 * @note 显示版本信息
 * @retval void
*/
void Func_About(void){
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(0," [About] ",16,0);
    OLED_ShowString_Rowcentering(16,"Name: MuMeStar",16,1);
    OLED_ShowString_Rowcentering(32,"Author: Star",16,1);
    OLED_ShowString_Rowcentering(48,"Version: 4.0",16,1);
    OLED_Refresh_Poll();
}

/* enter 进入 Fontsize 选项时，Menu_Handler() 总控会先通过 KEY_Parent_pressed() 调用第一个节点函数(称为Data_enter函数)，
然后当前菜单属性变为 Data，此后 Menu_Handler() 通过 KEY_Data_pressed() 调用第二个节点函数(称为Data_set函数)
对于前者，显示字号调节页面；对于后者，实现字号的设置并反馈 */

/**
 * @brief Fontsize节点函数(Data_enter型)
 * @note 显示字号调节页面
 * @retval void
*/
void Func_Fontsize_enter(void){
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(0,"Fontsize:     ",16,0);
    OLED_ShowNum_Dec(80,0,fontsize_set,1,16,0);
    OLED_ShowString_Rowcentering(20," Title ",Mysize[fontsize_set].size_title,0);
    OLED_ShowString_Rowcentering(48," Content ",Mysize[fontsize_set].size_content,1);
    OLED_Refresh_Poll();
}

/**
 * @brief Fontsize节点函数(Data_set型)
 * @note  next 增大字号，previous 减小字号，enter确定，return取消，修改的同时会在屏幕上显示当前字号效果
 * @retval void
*/
void Func_Fontsize_set(void){
    switch (KEY_num){
        case Zero:return;
        case Prevoius: // previous
            fontsize_set = (fontsize_set==0)?2:(fontsize_set-1);    
            Func_Fontsize_enter();
            break;
        case Enter: // enter
            fontsize = fontsize_set;
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"success",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        case Next: // next
            fontsize_set = (fontsize_set==2)?0:(fontsize_set+1);    
            Func_Fontsize_enter();
            break;
        case Return: // return
            fontsize_set = fontsize;
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"cancel",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        default:return;
    }
}

/* enter 进入 Brightness 选项时，Menu_Handler() 总控会先通过 KEY_Parent_pressed() 调用第一个节点函数(称为Data_enter函数)，
然后当前菜单属性变为 Data，此后 Menu_Handler() 通过 KEY_Data_pressed() 调用第二个节点函数(称为Data_set函数)
对于前者，显示亮度调节页面；对于后者，实现亮度的设置并反馈 */

/**
 * @brief Brightness节点函数(Data_enter型)
 * @note 显示OLED亮度调节页面
 * @retval void
*/
void Func_Brightness_enter(void){
    uint8_t percent = (uint8_t)(100*brightness_set/0xFF);
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(0,"Brightness:   %%",16,0);
    OLED_ShowNum_Dec(96,0,percent,3,16,0);
    OLED_ShowString_Rowcentering(20,"Hello",24,1);
    OLED_ShowString_Rowcentering(48,"Hello",16,0);
    OLED_Refresh_Poll();
}

/**
 * @brief Brightness节点函数(Data_set型)
 * @note  next 增大亮度，previous 减小亮度，enter确定，return取消，修改的同时在屏幕上显示当前亮度值
 * @retval void
*/
void Func_Brightness_set(void){
    switch (KEY_num){
        case Zero:return;
        case Prevoius: // previous 
            brightness_set = (brightness_set>25)?(brightness_set-25):1;    // 亮度为0时会自动熄灭屏幕, 需重新点亮
            OLED_SendByte_Poll(0x81,OLED_Command);OLED_SendByte_Poll(brightness_set,OLED_Command);
            Func_Brightness_enter();
            break;
        case Enter: // enter
            brightness = brightness_set;
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"success",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        case Next: // next
            brightness_set = (brightness_set<201)?(brightness_set+25):255;
            OLED_SendByte_Poll(0x81,OLED_Command);OLED_SendByte_Poll(brightness_set,OLED_Command);
            Func_Brightness_enter();
            break;
        case Return: // return
            brightness_set = brightness;
            OLED_SendByte_Poll(0x81,OLED_Command);OLED_SendByte_Poll(brightness_set,OLED_Command);
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"cancel",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        default:return;
    }
}

/**
 * @brief Colormode节点函数(Data_enter型) 
 * @note 显示OLED颜色设置页面
 * @retval void
*/
void Func_ColorMode_enter(void){
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(16," [Color Mode] ",16,1);
    if(colormode_set == White){OLED_ShowString_Rowcentering(40," White ",16,1);}
    else{OLED_ShowString_Rowcentering(40," Black ",16,1);}
    OLED_Refresh_Poll();
}

/**
 * @brief Colormode节点函数(Data_set型)
 * @note next或previous调整白天、黑夜模式，enter确定，return取消，修改的同时在屏幕上显示当前颜色模式
 * @retval void
*/
void Func_ColorMode_set(void){
    switch (KEY_num){
        case Zero:return;
        case Prevoius: // previous 
            colormode_set = (colormode_set+1)%2;
            OLED_ColorTurn(colormode_set);
            Func_ColorMode_enter();
            break;
        case Next: // next
            colormode_set = (colormode_set+1)%2;
            OLED_ColorTurn(colormode_set);
            Func_ColorMode_enter();
            break;
        case Enter: // enter
            colormode = colormode_set;
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"success",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        case Return: // return
            colormode_set = colormode;
            OLED_ColorTurn(colormode);
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"cancel",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        default:return;
    }
}

/**
 * @brief RefreshRate节点函数(Data_enter型) 
 * @note 显示屏幕刷新率调节页面
 * @retval void
*/
void Func_RefreshRate_enter(void){
    uint8_t percent = (uint8_t) ((100*refreshrate_set)/15) ;
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(16," [Refresh Rate] ",16,1);
    OLED_ShowNum_Dec(48,32,percent,3,16,1);
    OLED_ShowChar(72,32,'%',16,1); 
    OLED_Refresh_Poll();
}

/**
 * @brief RefreshRate节点函数(Data_set型)
 * @note next或previous调整刷新率，enter确定，return取消，修改的同时在屏幕上显示
 * @retval void
*/
void Func_RefreshRate_set(void){
    switch (KEY_num){
        case Zero:return;
        case Prevoius: // previous 
            refreshrate_set = (refreshrate_set==0)?0:(refreshrate_set-1);
            OLED_Set_RefershRate(refreshrate_set);
            Func_RefreshRate_enter();
            break;
        case Next: // next
            refreshrate_set = (refreshrate_set>=15)?15:(refreshrate_set+1);
            OLED_Set_RefershRate(refreshrate_set);
            Func_RefreshRate_enter();
            break;
        case Enter: // enter
            refreshrate = refreshrate_set;
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"success",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        case Return: // return
            refreshrate_set = refreshrate;
            OLED_Set_RefershRate(refreshrate);
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"cancel",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        default:return;
    }
}

/**
 * @brief MirrorFlipHo节点函数(Data_enter型) 
 * @note 显示水平镜像设置页面(Horizontal)
 * @retval void
*/
void Func_MirrorFlipHo_enter(void){
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(16," [Mirror Flip] ",16,1);
    if(mirrorflipHo_set == No){OLED_ShowString_Rowcentering(40," Horizontal: No ",16,1);}
    else{OLED_ShowString_Rowcentering(40," Horizontal:Yes ",16,1);}
    OLED_Refresh_Poll();
}

/**
 * @brief MirrorFlipHo节点函数(Data_set型)
 * @note next或previous调整是否镜像，enter确定，return取消，修改的同时在屏幕上显示
 * @retval void
*/
void Func_MirrorFlipHo_set(void){
    switch (KEY_num){
        case Zero:return;
        case Prevoius: // previous 
            mirrorflipHo_set = (mirrorflipHo_set+1)%2;
            OLED_MirrorHo(mirrorflipHo_set);
            Func_MirrorFlipHo_enter();
            break;
        case Next: // next
            mirrorflipHo_set = (mirrorflipHo_set+1)%2;
            OLED_MirrorHo(mirrorflipHo_set);
            Func_MirrorFlipHo_enter();
            break;
        case Enter: // enter
            mirrorflipHo = mirrorflipHo_set;
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"success",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        case Return: // return
            mirrorflipHo_set = mirrorflipHo;
            OLED_MirrorHo(mirrorflipHo);
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"cancel",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        default:return;
    }
}

/**
 * @brief MirrorFlipVer节点函数(Data_enter型) 
 * @note 显示水平镜像设置页面(Vertical)
 * @retval void
*/
void Func_MirrorFlipVer_enter(void){
    OLED_BufferClear();
    OLED_ShowString_Rowcentering(16," [Mirror Flip] ",16,1);
    if(mirrorflipVer_set == No){OLED_ShowString_Rowcentering(40," Vertical: No ",16,1);}
    else{OLED_ShowString_Rowcentering(40," Vertical:Yes ",16,1);}
    OLED_Refresh_Poll();
}

/**
 * @brief MirrorFlipVer节点函数(Data_set型)
 * @note next或previous调整是否镜像，enter确定，return取消，修改的同时在屏幕上显示
 * @retval void
*/
void Func_MirrorFlipVer_set(void){
    switch (KEY_num){
        case Zero:return;
        case Prevoius: // previous 
            mirrorflipVer_set = (mirrorflipVer_set+1)%2;
            OLED_MirrorVer(mirrorflipVer_set);
            Func_MirrorFlipVer_enter();
            break;
        case Next: // next
            mirrorflipVer_set = (mirrorflipVer_set+1)%2;
            OLED_MirrorVer(mirrorflipVer_set);
            Func_MirrorFlipVer_enter();
            break;
        case Enter: // enter
            mirrorflipVer = mirrorflipVer_set;
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"success",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        case Return: // return
            mirrorflipVer_set = mirrorflipVer;
            OLED_MirrorVer(mirrorflipVer);
            OLED_BufferClear();
            OLED_ShowString_Rowcentering(8,"config",24,1);
            OLED_ShowString_Rowcentering(32,"cancel",24,1);
            OLED_Refresh_Poll();
            HAL_Delay(1000);
            Menu_Return();
            break;
        default:return;
    }
}

/**
 * @brief 画眼睛 (Smile节点内部函数)
 * @retval void
*/
void Smile_Draweyes(void){
    // 左眼
    OLED_DrawPoint(eye_x,eye_y,1);
    OLED_DrawPoint(eye_x+1,eye_y,1);
    OLED_DrawPoint(eye_x,eye_y+1,1);
    OLED_DrawPoint(eye_x+1,eye_y+1,1);
    // 右眼
    OLED_DrawPoint(eye_x+38,eye_y,1);
    OLED_DrawPoint(eye_x+38+1,eye_y,1);
    OLED_DrawPoint(eye_x+38,eye_y+1,1);
    OLED_DrawPoint(eye_x+38+1,eye_y+1,1);
}

/**
 * @brief Smile节点函数(Loop_enter型)
 * @retval void
*/
void Func_Smile_enter(void){
    Loop_State = Loop_Run;
    smile_x = 32;eye_x = smile_x+10; eye_y = 11;
    // 2*2眼珠, eye_x in [smile_x+10,smile_x+14], eye_y in [11,15]
    OLED_BufferClear();
    OLED_ShowPicture(smile_x,0,64,64,Smile_Image,1);
    // 左眼
    Smile_Draweyes();
    OLED_Refresh_Poll();
}

/**
 * @brief Smile节点函数(Loop_run型) 
 * @note 循环转动笑脸眼珠位置；next 笑脸右移，previous 笑脸左移，enter 笑脸反色，return 退出
 * @retval void
*/
void Func_Smile_run(void){
    switch (KEY_num){
        case Zero:      // Defult
            if(Loop_State==Loop_Run){
                // 清除原眼珠
                OLED_DrawRectangle(smile_x+10,11,6,6,0,0);
                OLED_DrawRectangle(smile_x+10+38,11,6,6,0,0);
                // 获得新眼珠坐标
                if(eye_y==11){
                    if(eye_x == smile_x+14 ){eye_y++;  }
                    else{eye_x++; }
                }
                else if (eye_y == 15){
                    if( eye_x==smile_x+10 ){eye_y--;}
                    else{ eye_x--; }
                }
                else{
                    if(eye_x==smile_x+10){eye_y--;}
                    else{eye_y++;}
                }
            }
            break;
        case Prevoius:  // previous 
            eye_x = (smile_x == 0)?(64+10):(eye_x-8);
            smile_x = (smile_x == 0)?64:(smile_x-8);
            OLED_BufferClear();
            OLED_ShowPicture(smile_x,0,64,64,Smile_Image,1);
            KEY_num = Zero;
            break;
        case Enter:     // enter
            Loop_State = (Loop_State==Loop_Stop)?Loop_Run:Loop_Stop;
            KEY_num = Zero;
            break;
        case Next:      // next
            eye_x = (smile_x == 64)?(0+10):(eye_x+8);
            smile_x = (smile_x==64)?0:(smile_x+8);
            //smile_x = (smile_x+1)%65;
            OLED_BufferClear();
            OLED_ShowPicture(smile_x,0,64,64,Smile_Image,1);
            KEY_num = Zero;
            break;
        case Return:    // return
            Loop_State = Loop_Stop;
            Menu_Return();
            KEY_num = Zero;
            return;
        default:return;
    }
    Smile_Draweyes();
    OLED_ShowFPS(0,0,8,1);
    #if defined(SPItoOLED)
        OLED_Refresh_Poll();
        HAL_Delay(10);  // SPI通信较快，限制刷新帧率
    #endif
    #if defined(IICtoOLED)
        OLED_Refresh_DMA();
    #endif
}

/* -------------上面是菜单节点函数------------- */


/* ----------下面是其他自定义函数--------- */
/*                                       */
/*                                       */
/* ----------上面是其他自定义函数--------- */


/* -------------下面是菜单内部函数------------- */
/**
 * @brief 菜单初始化函数, 初始化并运行菜单
 * @retval void
*/
void Multimenu_Init(void){
    Insert = 0;
    Current_showrange = 0; 
    brightness = OLED_Defult_Brightness;  
    brightness_set = OLED_Defult_Brightness;    
    fontsize = OLED_Defult_Fontsize;  
    fontsize_set = OLED_Defult_Fontsize;    
    Menu_Pointer = (Menu_typedef *)Main;
    Insert = 0;
    OLED_SendByte_Poll(0x81,OLED_Command);
    OLED_SendByte_Poll(brightness_set,OLED_Command);
    Switch_Menu();
}

/**
 * @brief 按键处理函数, 提取当前菜单属性并根据键值调用对应函数
 * @retval void
*/
void Menu_Handler(void){
    // 检查是否忽略了一次键值
    // if( Jumped_key != Zero ){KEY_num = Jumped_key;Jumped_key=Zero;Menu_Handler();}
    /* 根据菜单属性调用函数 */
    enum MenuProperties property;   // 获取当前菜单属性
    if(Menu_Pointer == Main){property = Menu_Parent;}
    else{property = Menu_Pointer->Parent[Get_MenuIndex(Menu_Pointer)].Child_Menuproperty;}
    if(KEY_num == Zero){if(property == Menu_Loop){Jumped_key = Zero;KEY_Loop_pressed();}}
    else{
        Jumped_key = Zero; HAL_GPIO_TogglePin(LED_GPIO_Port,Led_Pin);
        switch (property){
            case Menu_Loop:   // Menu_Parents 型菜单
                KEY_Loop_pressed();  break;
            case Menu_Parent: // Menu_Parents 型菜单
                KEY_Parent_pressed();KEY_num = Zero;break;
            case Menu_Data:
                KEY_Data_pressed();  KEY_num = Zero;break;
            case Menu_Once:   // Menu_Parents 型菜单
                KEY_Once_pressed();  KEY_num = Zero;break;
            default:Multimenu_Init();   // 限制未定义菜单
        }
    }
}

/**
 * @brief 显示 Menu_Pointer 所指菜单
 * @retval void
*/
void Switch_Menu(void){
/* 
先获取作图变量，再作出图像
 */
// 第一步：获取作图变量
    uint8_t i=0; // 循环变量
    uint8_t len; // 菜单头名字长度
    uint8_t n; // 需要draw的行数
    uint8_t rec_y; // 滑动条填充部分起始纵坐标
    uint8_t height; // 滑动条填充部分纵长
    uint8_t index;  // 当前菜单在父级中的索引
    index = Get_MenuIndex(Menu_Pointer);
    switch (Insert){
        case 0: // 仍处于当前菜单
            Current_CL= UserChoose-Current_showrange;
            rec_y = (uint8_t) 60*UserChoose/((Menu_Pointer==Main)?Main_Child_nodesnumber:Menu_Pointer->Parent->Child_nodes_number);
            break;
        case 1: // 进入子菜单
            Current_CL=0;rec_y = 0;
            break;
        case 2: // 返回父菜单
            Current_showrange = ( UserChoose>=Mysize[fontsize].row_number )?(UserChoose+1-Mysize[fontsize].row_number):0;
            Current_CL = UserChoose-Current_showrange; 
            rec_y = (uint8_t) 60*UserChoose/((Menu_Pointer==Main)?Main_Child_nodesnumber:Menu_Pointer->Parent->Child_nodes_number);
            break;
        default:return;
    }
    while(' '<=*(Menu_Pointer->Name+i) && *(Menu_Pointer->Name+i)<='~'){i++;}    // 获取名字长度
    len = i;
    if(Menu_Pointer == Main){ // 若为主菜单
        n = (Main_Child_nodesnumber<=Mysize[fontsize].row_number)?Main_Child_nodesnumber:Mysize[fontsize].row_number;
        height = (uint8_t) 60/Main_Child_nodesnumber-1;
    }
    else{
        n = ((Menu_Pointer->Parent+index)->Child_nodes_number<=Mysize[fontsize].row_number)?(Menu_Pointer->Parent+index)->Child_nodes_number:Mysize[fontsize].row_number;
        height = (uint8_t) 60/(Menu_Pointer->Parent+index)->Child_nodes_number-1;
    }

// 第二步：作出菜单页面（包括动画）
    if(Insert==0){
        if(Current_showrange==Last_showrange){  // Cursor动画
            uint8_t direct = (Current_CL>=Last_CL)?1:-1;
            for(i=0;i<=Mysize[fontsize].size_content;i++){
                OLED_BufferClear();
                // 显示光标
                OLED_ShowChar(0,Mysize[fontsize].Mysize_array[Last_CL]+i*direct,'>',Mysize[fontsize].size_content,1);
                // 不参与动画的部分
                // 显示子菜单
                for (i = 0; i <n; i++){
                    OLED_ShowString(12,Mysize[fontsize].Mysize_array[i],(uint8_t *)(Menu_Pointer+i+Current_showrange)->Child->Name,Mysize[fontsize].size_content,1);
                    if((Menu_Pointer+i+Current_showrange)->Child_Menuproperty == Menu_Parent){ 
                    OLED_ShowString(118-Mysize[fontsize].size_content*3/2-6,Mysize[fontsize].Mysize_array[i],"...",Mysize[fontsize].size_content,1);}
                }
                // 显示菜单头
                for (i = 0; i < len+2; i++){OLED_ShowChar(i*Mysize[fontsize].size_title/2,0,' ',Mysize[fontsize].size_title,0);}
                OLED_ShowString(Mysize[fontsize].size_title/2,0,(uint8_t *)Menu_Pointer->Name,Mysize[fontsize].size_title,0);
                // 显示右侧滑动条
                OLED_DrawRectangle(118,0,10,64,1,0);
                OLED_DrawRectangle(120,2,6,60,1,0);
                OLED_DrawRectangle(121,3+rec_y,4,height,1,1);
                OLED_Refresh_Poll();
            }
        }
        else{   // Page动画
    // 先拿这个放着
            OLED_BufferClear();
            // 显示光标
            OLED_ShowChar(0,Mysize[fontsize].Mysize_array[Current_CL],'>',Mysize[fontsize].size_content,1);
            // 显示子菜单
            for (i = 0; i <n; i++){
                OLED_ShowString(12,Mysize[fontsize].Mysize_array[i],(uint8_t *)(Menu_Pointer+i+Current_showrange)->Child->Name,Mysize[fontsize].size_content,1);
                if((Menu_Pointer+i+Current_showrange)->Child_Menuproperty == Menu_Parent){ 
                OLED_ShowString(118-Mysize[fontsize].size_content*3/2-6,Mysize[fontsize].Mysize_array[i],"...",Mysize[fontsize].size_content,1);}
            }
            // 不参与动画的部分
            // 显示菜单头
            for (i = 0; i < len+2; i++){OLED_ShowChar(i*Mysize[fontsize].size_title/2,0,' ',Mysize[fontsize].size_title,0);}
            OLED_ShowString(Mysize[fontsize].size_title/2,0,(uint8_t *)Menu_Pointer->Name,Mysize[fontsize].size_title,0);
            // 显示右侧滑动条
            OLED_DrawRectangle(118,0,10,64,1,0);
            OLED_DrawRectangle(120,2,6,60,1,0);
            OLED_DrawRectangle(121,3+rec_y,4,height,1,1);
            OLED_Refresh_Poll();
        }
    }
    else{
        OLED_BufferClear();
        // 显示光标
        OLED_ShowChar(0,Mysize[fontsize].Mysize_array[Current_CL],'>',Mysize[fontsize].size_content,1);
        // 显示子菜单
        for (i = 0; i <n; i++){
            OLED_ShowString(12,Mysize[fontsize].Mysize_array[i],(uint8_t *)(Menu_Pointer+i+Current_showrange)->Child->Name,Mysize[fontsize].size_content,1);
            if((Menu_Pointer+i+Current_showrange)->Child_Menuproperty == Menu_Parent){ 
            OLED_ShowString(118-Mysize[fontsize].size_content*3/2-6,Mysize[fontsize].Mysize_array[i],"...",Mysize[fontsize].size_content,1);}
        }
        // 不参与动画的部分
        // 显示菜单头
        for (i = 0; i < len+2; i++){OLED_ShowChar(i*Mysize[fontsize].size_title/2,0,' ',Mysize[fontsize].size_title,0);}
        OLED_ShowString(Mysize[fontsize].size_title/2,0,(uint8_t *)Menu_Pointer->Name,Mysize[fontsize].size_title,0);
        // 显示右侧滑动条
        OLED_DrawRectangle(118,0,10,64,1,0);
        OLED_DrawRectangle(120,2,6,60,1,0);
        OLED_DrawRectangle(121,3+rec_y,4,height,1,1);
        OLED_Refresh_Poll();
    }
    
}

/**
 * @brief 返回父级菜单
 * @retval void
*/
void Menu_Return(void){
    KEY_Parent_return();Switch_Menu();
} 

/**
 * @brief 获取当前菜单在父菜单中的索引值
 * @param menu 需要获取索引值的菜单
 * @retval uint8_t, 若为主菜单或检索错误返回 255=0xFF
*/
uint8_t Get_MenuIndex(Menu_typedef* menu){
    uint8_t x = 0;  // 当前菜单在父菜单中的索引值
    if( menu == Main ){ return 255; }
    else{
        for( x=0;x<30;x++ ){   // 每个 Parent 型菜单最大子菜单数目为 30
        if(menu->Parent[x].Child==menu){break;}
        }
        return x;
    }
}

/**
 * @brief 获取菜单名字长度
 * @param void
 * @param 
 * @retval void
*/
uint8_t Get_MenuNameLenth(Menu_typedef* menu){
    #if defined(__string_h)
        return strlen(menu->Name);
    #endif  // __string_h
    #if !defined(__string_h)
        uint8_t i;
        while(' '<=*(menu->Name+i) && *(menu->Name+i)<='~'){i++;}    // 获取名字长度
        return i;
    #endif  // __string_h
}

/**
 * @brief 空函数, 在屏幕上显示 "无效操作"
 * @retval void
*/
void Invalid_Operation(void){
    OLED_BufferClear();
    OLED_DrawLine(0,3,127,3,1);
    OLED_ShowString_Rowcentering(8," invalid ",24,0);
    OLED_ShowString(10,32,"operation",24,0);
    OLED_DrawLine(0,61,127,61,1);
    OLED_Refresh_Poll();
    HAL_Delay(300);
    OLED_BufferClear();
    Switch_Menu();
}

/* -------------上面是菜单内部函数------------- */

/* -------------下面是KEY函数(中断模式)------------- */
/**
 * @brief 任一按键按下时调用，传递键值
 * @retval void
*/
void KEY_Pressed(uint8_t GPIO_pin){
    uint8_t ispressed;
    HAL_Delay(10);  // 按键消抖
    ispressed = 
    (HAL_GPIO_ReadPin(Key_return_GPIO_Port,Key_previous_Pin)==KEY_Active_Volt)||
    (HAL_GPIO_ReadPin(Key_next_GPIO_Port,Key_next_Pin)==KEY_Active_Volt)||
    (HAL_GPIO_ReadPin(Key_enter_GPIO_Port,Key_enter_Pin)==KEY_Active_Volt)||
    (HAL_GPIO_ReadPin(Key_return_GPIO_Port,Key_return_Pin)==KEY_Active_Volt);
    if(ispressed == 1){     // 防误触
        switch (GPIO_pin){
            case Key_previous_Pin:KEY_num = Prevoius;Jumped_key = Prevoius; break;
            case Key_enter_Pin:   KEY_num = Enter;   Jumped_key = Enter;    break;
            case Key_next_Pin:    KEY_num = Next;    Jumped_key = Next;     break;
            case Key_return_Pin:  KEY_num = Return;  Jumped_key = Return;   break;
            default: return;
        }
    }
    HAL_Delay(10);  // 按键消抖
}

/**
 * @brief 经过 Menu_Handler() 函数判定当前菜单为 Parent 型后调用此函数
 * @retval void
*/
void KEY_Parent_pressed(void){
/* 
分为三步。
第一步获取菜单相关的变量:Menu_Pointer, Current_showrange, UserChoose, Insert。由switch实现
第二步根据已有菜单变量，获取作图变量，并作出菜单图像。由Menu_Switch()实现（内含Draw_Menu）。
第三步收尾。
*/
     switch (KEY_num){
        case Zero:return;
        case Prevoius: KEY_Parent_previous();break;
        case Enter:    KEY_Parent_enter();   break;
        case Next:     KEY_Parent_next();    break;
        case Return:   KEY_Parent_return();  break;
        default: return;
    }
    if(Insert==1){Current_showrange=0;UserChoose=0;}
    else{Switch_Menu();}
    // 收尾工作
    Last_CL = Current_CL;
    Last_showrange=Current_showrange;
    Insert=0;
}

/**
 * @brief 返回父菜单, 经过 Menu_Handler() 函数判定当前菜单为 Parent 型后调用此函数, 或者在选项中手动调用
 * @retval void
*/
void KEY_Parent_return(void){
    if(Menu_Pointer->Parent == NULL){Insert = 0;return;}  // 根菜单检测
    else{ 
        Insert = 2;
        UserChoose = Get_MenuIndex(Menu_Pointer);
        Menu_Pointer = (Menu_typedef *)Menu_Pointer->Parent;
    }
}

void KEY_Parent_next(void){
    uint8_t nodes_number = (Menu_Pointer->Parent+Get_MenuIndex(Menu_Pointer))->Child_nodes_number;
    if(Menu_Pointer == Main){nodes_number = Main_Child_nodesnumber;}    
    // 判断 当前菜单子项目数 与 最大显示行数 大小关系
    if(nodes_number<=Mysize[fontsize].row_number){
        if(UserChoose == nodes_number-1 ){UserChoose = 0;}
        else{UserChoose++;}
    }
    else{
        if( UserChoose >= Current_showrange + Mysize[fontsize].row_number -1 ){   // 到达显示下边界
            if( UserChoose >= nodes_number-1 ){Current_showrange=0;UserChoose=0;}    // 到达菜单选项下边界
            else{Current_showrange++;UserChoose++;}
        }
        else{   // 未到达显示边界
            UserChoose++;
        }
    }
}

void KEY_Parent_previous(void){
    uint8_t nodes_number = (Menu_Pointer->Parent+Get_MenuIndex(Menu_Pointer))->Child_nodes_number;
    if(Menu_Pointer == Main){nodes_number = Main_Child_nodesnumber;}    
    // 判断 当前菜单子项目数 与 最大显示行数 大小关系
    if(nodes_number<=Mysize[fontsize].row_number){
        if(UserChoose == 0 ){UserChoose = nodes_number-1;}
        else{UserChoose--;}
    }
    else{
        if( UserChoose <= Current_showrange ){   // 到达显示上边界
            if( UserChoose == 0 )      // 到达选项上边界
            {Current_showrange=nodes_number-Mysize[fontsize].row_number; UserChoose=nodes_number-1;}    
            else{Current_showrange--; UserChoose--;}
        }
        else{   // 未到达显示边界
            UserChoose--;
        }
    }
}

void KEY_Parent_enter(void){
    // 限制未定义操作
    if((Menu_Pointer+UserChoose)->Child==NULL || (Menu_Pointer+UserChoose)->func==NULL){
        Invalid_Operation();return;
    }
    else{        // 进入子菜单
        Insert = 1;  
        Current_showrange = 0;
        Menu_Pointer = (Menu_typedef *) (Menu_Pointer+UserChoose)->Child;
        (Menu_Pointer->Parent+UserChoose)->func();
        Current_showrange = 0;
        UserChoose = 0;
    }
}


/**
 * @brief 经过 Menu_Handler() 函数判定当前菜单为 Data 型后调用此函数
 * @param KEY_num 键值
 * @retval void
*/
void KEY_Data_pressed(void){
    if(KEY_num != Zero ){Menu_Pointer->func();}
}

/**
 * @brief 经过 Menu_Handler() 函数判定当前菜单为 Once 型后调用此函数
 * @retval void
*/
void KEY_Once_pressed(void){
    if(KEY_num != Zero){Menu_Return();}
}

/**
 * @brief 经过 Menu_Handler() 函数判定当前菜单为 Loop 型后调用此函数
 * @retval void
*/
void KEY_Loop_pressed(void){
    Menu_Pointer->func();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == Key_previous_Pin||Key_next_Pin||Key_enter_Pin||Key_return_Pin )
    {KEY_Pressed(GPIO_Pin);}
} 
/* -------------上面是 KEY函数(中断模式)------------- */

/* 下面是小恐龙游戏 */
#if defined(Add_Game_Dinosaur)
/* Higheset Score */
// 全局变量声明
const uint8_t dino_crashedData[63];const Image dino_crashedImg;
const uint8_t dino_front_legData[63];const Image dino_front_legImg;
const uint8_t dino_back_legData[63];const Image dino_back_legImg;
const uint8_t dino_jumpsData[63];const Image dino_jumpsImg;
const uint8_t tree_smallData[12];const Image tree_smallImg;
const uint8_t tree_bigData[36];const Image tree_bigImg;
const uint8_t time_distance[38];
enum GAME_STATE Game_State;
uint16_t highest_score = 0;  //Higheset Score

// 游戏变量声明
uint16_t tree_interval;
uint16_t tree1_interval;
int16_t interval;
uint16_t tree_x;
uint16_t tree1_x;
Image *tree;
Image *tree1;
int16_t dino_right;
int16_t dino_y;
uint8_t jump;
uint16_t score_raw;
uint16_t score;
uint8_t leg;
//uint32_t tmp;

/**
 * @brief Dinosaur节点函数(Loop_enter型) , 进入循环
 * @retval void
*/
void Func_Dinosaur_enter(void){
    tree_interval = 170; // 252
    tree1_interval = 240;   // 381
    interval = 0;
    tree_x = tree_interval + 100;
    tree1_x = tree1_interval + 200;
    tree = (HAL_GetTick() % 8) ? (Image *)&tree_smallImg : (Image *)&tree_bigImg;
    tree1 = (HAL_GetTick() % 5) ? (Image *)&tree_smallImg : (Image *)&tree_bigImg;
    dino_right = DINO_INIT_X + dino_front_legImg.width;
    dino_y = DINO_INIT_Y;
    jump = 0;
    score_raw = 0;
    score = 0;
    leg = 0;
    //tmp = 0;

    Game_State = Game_Stop;
    OLED_BufferClear();
    Dinosaur_MoveDino(dino_y, -2);
    Dinosaur_MoveTree();
    OLED_DrawLine(BASE_LINE_X, BASE_LINE_Y, BASE_LINE_X1, BASE_LINE_Y-1,1);
    OLED_ShowString(2,1,"Highest Score:",12,1);OLED_ShowNum_Dec(86,1,highest_score,5,12,1);
    OLED_DrawRectangle(18,15,92,36,1,0);
    OLED_ShowString_Rowcentering(17,"Press Enter",16,1);
    OLED_ShowString_Rowcentering(33,"to Start",16,1);
    OLED_Refresh_Poll();
}

/**
 * @brief Dinosaur节点函数(Loop_run型) , 循环执行操作
 * @retval void
*/
void Func_Dinosaur_run(void){
    switch (Game_State){
        case Game_Stop:Dinosaur_Stop_Handler();break;
        case Game_Run:/* HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13); */Dinosaur_Run_Handler();break;
        case Game_Over:Dinosaur_Over_Handler();break;
        default:break;
    }
}

/**
 * @brief 小恐龙游戏内部函数，当游戏状态为Game_Stop时运行
 * @retval void
*/
void Dinosaur_Stop_Handler(void){
    switch (KEY_num){
        case Enter:Game_State = Game_Run;break;
        case Return:Game_State = Game_Over;Menu_Return();break;
        default:break;
    }
    KEY_num=Zero;
}

/**
 * @brief 小恐龙游戏内部函数，当游戏状态为Game_Run时运行
 * @retval void
*/
void Dinosaur_Run_Handler(void){
    switch (KEY_num){
        case Zero:break;
        case Prevoius:if (jump == 0){jump = 1;}break;
        case Next:if (jump == 0){jump = 1;}break;
        case Enter:
            Game_State = Game_Stop;
            OLED_DrawRectangle(10,18,106,28,1,0);
            OLED_ShowString_Rowcentering(20,"Enter to continue",12,1);
            OLED_ShowString_Rowcentering(32,"Return to exit",12,1);
            OLED_Refresh_Poll();
            KEY_num=Zero;
            return;
        case Return:
            Game_State = Game_Stop;
            OLED_DrawRectangle(10,18,106,28,1,0);
            OLED_ShowString_Rowcentering(20,"Enter to continue",12,1);
            OLED_ShowString_Rowcentering(32,"Return to exit",12,1);
            OLED_Refresh_Poll();
            KEY_num=Zero;
            return;
        default:return;
    }
    KEY_num=Zero;
    OLED_BufferClear();
    // 判断是否Game_Over
    if ((tree1_x <= (dino_right - ((jump < 5) ? jump : 5)) && tree1_x > (DINO_INIT_X + 1) && (dino_y + dino_back_legImg.height) >= (BASE_LINE_Y - tree1->height))||(tree_x <= (dino_right - ((jump < 5) ? jump : 5)) && tree_x > (DINO_INIT_X + 1) && (dino_y + dino_back_legImg.height) >= (BASE_LINE_Y - tree->height))){

        // Collision Happened
        Game_State = Game_Over;
        Dinosaur_MoveTree();
        Dinosaur_MoveDino(dino_y, -2);
        OLED_DrawLine(BASE_LINE_X, BASE_LINE_Y, BASE_LINE_X1, BASE_LINE_Y-1,1);
        OLED_Refresh_Poll();
        HAL_Delay(800);
        if (score >= highest_score) {highest_score = score;OLED_ShowString(0,2,"New Hst",12,1);} 
        Dinosaur_DisplayScore(score);
        OLED_DrawRectangle(17,30,94,28,1,0);
        OLED_ShowString_Rowcentering(32,"Game Over!",12,1);
        OLED_ShowString_Rowcentering(44,"E-replay R-exit",12,1);
        OLED_Refresh_Poll();
        return;
    }
    else{
        if (jump > 0) {dino_y = DINO_INIT_Y - time_distance[jump - 1];if (++jump > 38){jump = 0;}}
        score_raw++;
        score = score_raw / 5;
        if (jump != 0){Dinosaur_MoveDino(dino_y, -1);}
        else { Dinosaur_MoveDino(dino_y, (leg > 4)?1:0);if (++leg > 9){leg = 0;}}
        Dinosaur_MoveTree();
        OLED_DrawLine(BASE_LINE_X, BASE_LINE_Y, BASE_LINE_X1, BASE_LINE_Y-1,1);
        Dinosaur_DisplayScore(score);
        OLED_ShowFPS(80,16,8,1);
        OLED_Refresh_Poll();
        #if defined(SPItoOLED)
            HAL_Delay(10);  // SPI通信较快，限制刷新帧率
        #endif
    }
}

/**
 * @brief 小恐龙游戏内部函数，当游戏状态为Game_Over时运行
 * @retval void
*/
void Dinosaur_Over_Handler(void){
    switch (KEY_num){
        case Enter:Func_Dinosaur_enter();break;
        case Return:Menu_Return();break;
        default:return;
    }
    KEY_num=Zero;
}

/**
 * @brief Move dino
 * @note Referenced from github@harshmittal2210/dino_game.ino
 */
void Dinosaur_MoveDino(int16_t y, int type){
    switch (type){
        case 0:  OLED_ShowPicStruct(DINO_INIT_X, y,dino_front_legImg,1); break;
        case 1:  OLED_ShowPicStruct(DINO_INIT_X, y,dino_back_legImg,1);  break;
        case -1: OLED_ShowPicStruct(DINO_INIT_X, y,dino_jumpsImg,1);     break;
        case -2: OLED_ShowPicStruct(DINO_INIT_X, y,dino_crashedImg,1);   break;
        default:return;
    }
}

/**
 * @brief 计算树位置并写入显存
 * @retval void
*/
void Dinosaur_MoveTree(void){
    if(Game_State==Game_Run){
        tree_x = tree_x - 2;
        tree1_x = tree1_x - 2;
        if (tree_x <= 4) {
            tree_x = tree_interval + (uwTick&0xF);
            tree = (uwTick % 4) ? (Image *)&tree_smallImg : (Image *)&tree_bigImg;
        }
        if (tree1_x <= 4) {
            tree1_x = tree1_interval + (uwTick&0x1F);
            tree1 = (uwTick % 5) ? (Image *)&tree_smallImg : (Image *)&tree_bigImg;
        }
        if (interval > 0 && interval <= 46)
            tree_x += interval;
        else if (interval < 0 && interval >= -46)
            tree1_x += interval;
        // 判断是否作出树
        if(tree_x + tree->width <= 127){OLED_ShowPicStruct(tree_x, BASE_LINE_Y - tree->height,*tree,1);}
        if(tree1_x + tree1->width <= 127){OLED_ShowPicStruct(tree1_x, BASE_LINE_Y - tree1->height,*tree1,1);}    
    }
    else{
        if(tree_x + tree->width <= 127){OLED_ShowPicStruct(tree_x, BASE_LINE_Y - tree->height,*tree,1);}
        if(tree1_x + tree1->width <= 127){OLED_ShowPicStruct(tree1_x, BASE_LINE_Y - tree1->height,*tree1,1);}
    }
}

/**
 * @brief Display score while running the game
 * @note Referenced from github@harshmittal2210/dino_game.ino
 */
void Dinosaur_DisplayScore(int score){
    switch (Game_State){
        case Game_Over:
            OLED_ShowString(50,1,"Score:",12,1);OLED_ShowNum_Dec(86,1,score,5,12,1);
            OLED_ShowString(2,14,"Highest Score:",12,1);OLED_ShowNum_Dec(86,14,highest_score,5,12,1);
            break;
        case Game_Stop:
            OLED_ShowString(50,1,"Score:",12,1);OLED_ShowNum_Dec(86,1,score,5,12,1);
            break;
        case Game_Run:
            OLED_ShowString(50,1,"Score:",12,1);OLED_ShowNum_Dec(86,1,score,5,12,1);
            break;
        default:break;
    }
}

/* Bitmap */
// Dino crashed
const uint8_t dino_crashedData[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xf1, 0xf5, 0xf1, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7e, 0x7f, 0xfc, 0xf8, 0xe0, 0xe0, 0xf8, 0xfc, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x04, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x81, 0x83, 0x87, 0xff, 0xdf, 0x87, 0x87, 0x87, 0xff, 0xc3, 0xc3, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};
const Image dino_crashedImg = {21, 23, dino_crashedData};
// Dino lifts front leg
const uint8_t dino_front_legData[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xbf, 0xbf, 0xbf, 0x3f, 0x3e, 0x3f, 0xfc, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x81, 0x83, 0x87, 0xff, 0xdf, 0x8f, 0x87, 0x87, 0x8f, 0x8b, 0x8b, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};
const Image dino_front_legImg = {21, 23, dino_front_legData};
// Dino lifts back leg
const uint8_t dino_back_legData[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xbf, 0xbf, 0xbf, 0x3f, 0x3e, 0x3f, 0xfc, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x81, 0x83, 0x87, 0x8f, 0x9f, 0x97, 0x87, 0x8f, 0xff, 0xc3, 0xc3, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};
const Image dino_back_legImg = {21, 23, dino_back_legData};
// Dino jumps
const uint8_t dino_jumpsData[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xbf, 0xbf, 0xbf, 0x3f, 0x3e, 0x3f, 0xfc, 0xf8, 0xf0, 0xf0, 0xf8, 0xfc, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x81, 0x83, 0x87, 0xff, 0xdf, 0x8f, 0x87, 0x8f, 0xff, 0xc3, 0xc3, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};
const Image dino_jumpsImg = {21, 23, dino_jumpsData};

// Small tree
const uint8_t tree_smallData[12] = {
    0xf8, 0x00, 0xff, 0xff, 0xc0, 0x7c, 0xc1, 0xc1, 0xff, 0xff, 0xc0, 0xc0,
};
const Image tree_smallImg = {6, 14, tree_smallData};

// Big tree
const uint8_t tree_bigData[36] = {
    0x80, 0xc0, 0x80, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0, 0xe0, 0xe0, 0x3f, 0x7f, 0x7f, 0x60, 0xff, 0xff, 0xff, 0xff, 0x60, 0x7f, 0x3f, 0x1f, 0x80, 0x80, 0x80, 0x80, 0xff, 0xff, 0xff, 0xff, 0x80, 0x80, 0x80, 0x80,
};
const Image tree_bigImg = {12, 23, tree_bigData};

/* Jump Time-Distance Table */
const uint8_t time_distance[38] = {0, 4, 7, 11, 14, 17, 20, 23, 25, 28, 30, 31, 33, 34, 35, 36, 37, 37, 38, 38, 37, 37, 36, 35, 34, 33, 31, 30, 28, 25, 23, 20, 17, 14, 11, 7, 4, 0};


#endif // Add_Game_Dinosaur


