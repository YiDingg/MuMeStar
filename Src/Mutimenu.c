#include "main.h"
#include "oled.h"
#include "MutiMenu.h"
#include "MultiMenu_Data.h"

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



/* -----------下面是菜单全局变量---------- */
static uint8_t Insert = 0;    // Draw_menu()专用, 判断光标位置 -1return, 0不变, 1enter
static uint8_t Current_showrange = 0;  // Draw_menu()专用, 判断当前显示范围，屏幕最大显示行数为 Mysize[fontsize].row_number
static uint8_t UserChoose = 0; 		// Draw_menu()专用, 光标位置
static uint8_t Data_keynum = 0;    // Data型菜单页面专用, 传递按键值, 1previous, 2enter, 3next, 5return
static uint8_t Loop_keynum = 0;    // Loop型菜单页面专用, 传递按键值, 1previous, 2enter, 3next, 5return

static uint8_t brightness = OLED_StartBrightness;  // 默认屏幕亮度
static uint8_t brightness_setting = OLED_StartBrightness;    // "Brightness" 专用，用于调节屏幕当前亮度

static uint8_t fontsize = OLED_StartFontsize;  // 默认菜单字号
static uint8_t fontsize_setting = OLED_StartFontsize;    // "Fontsize" 专用，用于调节菜单当前字号
/* -----------上面是菜单全局变量---------- */


/* --------------------- 下面是菜单栏定义 -------------------- */
/*                                                           */
/* 菜单节点初始化(节点声明) */
#define Main_Child_nodesnumber 6    // 宏定义主菜单子节点数,方便操作
static const Menu_typedef 
Main[Main_Child_nodesnumber],
    Main_Settings[2],
        Main_Settings_Fontsize[1],
        Main_Settings_Brightness[1],     
    Main_Hello[2],
        Main_Hello_Sayhello[1],
        Main_Hello_Smile[1],
    Main_About[1],
    Main_Menu4[1],
    Main_Menu5[1],
    Main_Menu6[1];
/* Menu_Root, Menu_0 */
static Menu_typedef* Menu_Pointer =  (Menu_typedef *)Main; // 设置当前菜单为 Main 并初始化

static const Menu_typedef Main[Main_Child_nodesnumber] = {				
    {"Main",    NULL, Main_Settings,  Draw_Menu,         2,Menu_Parent},
    {"Main",    NULL, Main_Hello,     Draw_Menu,         2,Menu_Parent},
    {"Main",    NULL, Main_About,     Func_About,    1,Menu_Once},
    {"Main",    NULL, Main_Menu4,     0, 0},
    {"Main",    NULL, Main_Menu5,     0, 0},
    {"Main",    NULL, Main_Menu6,     0, 0},
};

/* Menu_1 */
static const Menu_typedef Main_Settings[2] = {				
    {"Settings", Main,  Main_Settings_Fontsize,   Func_Fontsize_enter, 1, Menu_Data},
    {"Settings", Main,  Main_Settings_Brightness,  Func_Brightness_enter, 1, Menu_Data}
};

static const Menu_typedef Main_Hello[2] = {				
    {"Hello", Main,  Main_Hello_Sayhello,Func_Sayhello, 1,Menu_Once},
    {"Hello", Main,  Main_Hello_Smile,0, 1,Menu_Loop}
};

static const Menu_typedef Main_About[1] = {{"About",Main,NULL,0, 0},};
static const Menu_typedef Main_Menu4[1] = {{"Menu4",Main,NULL,0, 0},};
static const Menu_typedef Main_Menu5[1] = {{"Menu5",Main,NULL,0, 0},};
static const Menu_typedef Main_Menu6[1] = {{"Menu6",Main,NULL,0, 0},};

/* Menu_2*/
static const Menu_typedef Main_Settings_Fontsize[1] = {{"Fontsize",Main_Settings,NULL,Func_Fontsize_set, 0,Menu_Data}};
static const Menu_typedef Main_Settings_Brightness[1] = {
    {"Brightness",Main_Settings,NULL,Func_Brightness_set, 0, Menu_Data}
};
static const Menu_typedef Main_Hello_Sayhello[1] = {{"Sayhello",Main_Hello,NULL,NULL, 0,}};
static const Menu_typedef Main_Hello_Smile[1] = {{"Smile",Main_Hello,NULL,NULL, 0,}};
/*                                                           */
/* ---------------------- 上面是菜单栏定义 ------------------- */


/* -------------下面是菜单节点函数------------- */
/**
 * @brief Once型节点函数，在屏幕上显示"Hello world!"
 * @retval void
*/
void Func_Sayhello(void){
    OLED_Buffer_clear();
    OLED_ShowString_rowcentering(20,"Hello world!",16,1);
    OLED_Refresh();
}

/**
 * @brief Once型节点函数, 显示版本信息等
 * @retval void
*/
void Func_About(void){
    OLED_Buffer_clear();
    OLED_ShowString_rowcentering(0," [About] ",16,0);
    OLED_ShowString(0,16,"Author: Star",12,1);
    OLED_ShowString(0,28,"Version: 2.4",12,1);
    OLED_ShowString(0,40,"Time: 2024.5.13",12,1);
    OLED_Refresh();
}

/**
 * @brief Brightness节点函数(Data_enter型) , 显示OLED亮度调节页面
 * @note 
enter 进入 Brightness 选项时，KEY_Pressed() 总控会先通过 KEY_Parent_pressed() 调用第一个节点函数(称为Data_enter函数)，
然后当前菜单属性变为 Data，此后 KEY_Pressed() 通过 KEY_Data_pressed() 调用第二个节点函数(称为Data_set函数)
对于前者，显示亮度调节页面；对于后者，实现亮度的设置并反馈
 * @retval void
*/
void Func_Brightness_enter(void){
    uint8_t percent = (uint8_t)(100*brightness_setting/0xFF);
    OLED_Buffer_clear();
    OLED_ShowString_rowcentering(0,"Brightness:   %%",16,0);
    OLED_ShowNum_dec(96,0,percent,3,16,0);
    OLED_ShowString_rowcentering(20,"Hello",24,1);
    OLED_ShowString_rowcentering(48,"Hello",16,0);
    OLED_Refresh();
}

/**
 * @brief Brightness节点函数(Data_set型), next 增大亮度，previous 减小亮度，enter确定，return取消
 * 修改的同时在屏幕上显示当前亮度值
 * @retval void
*/
void Func_Brightness_set(void){
    switch (Data_keynum)
    {
    case 1: // previous 
        brightness_setting = (brightness_setting>25)?(brightness_setting-25):1;    // 亮度为0时会自动熄灭屏幕, 需重新点亮
        OLED_WR_Byte_DMA(OLED_CMD,0x81,1);OLED_WR_Byte_DMA(OLED_CMD,brightness_setting,1);
        Func_Brightness_enter();
        break;
    case 2: // enter
        brightness = brightness_setting;
        OLED_Buffer_clear();
        OLED_ShowString_rowcentering(8,"config",24,1);
        OLED_ShowString_rowcentering(32,"success",24,1);
        OLED_Refresh();
        HAL_Delay(1000);
        KEY_Parent_return();
        break;
    case 3: // next
        brightness_setting = (brightness_setting<201)?(brightness_setting+25):255;
        OLED_WR_Byte_DMA(OLED_CMD,0x81,1);OLED_WR_Byte_DMA(OLED_CMD,brightness_setting,1);
        Func_Brightness_enter();
        break;
    case 5: // return
        brightness_setting = brightness;
        OLED_WR_Byte_DMA(OLED_CMD,0x81,1);OLED_WR_Byte_DMA(OLED_CMD,brightness_setting,1);
        OLED_Buffer_clear();
        OLED_ShowString_rowcentering(8,"config",24,1);
        OLED_ShowString_rowcentering(32,"cancel",24,1);
        OLED_Refresh();
        HAL_Delay(1000);
        KEY_Parent_return();
        break;
    default:return;
    }
}

/**
 * @brief Fontsize节点函数(Data_enter型) , 显示字号调节页面
 * @note enter 进入 Fontsize 选项时，KEY_Pressed() 总控会先通过 KEY_Parent_pressed() 调用第一个节点函数(称为Data_enter函数)，
然后当前菜单属性变为 Data，此后 KEY_Pressed() 通过 KEY_Data_pressed() 调用第二个节点函数(称为Data_set函数)
对于前者，显示字号调节页面；对于后者，实现字号的设置并反馈
 * @retval void
*/
void Func_Fontsize_enter(void){
    OLED_Buffer_clear();
    OLED_ShowString_rowcentering(0,"Fontsize:     ",16,0);
    OLED_ShowNum_dec(80,0,fontsize_setting,1,16,0);
    OLED_ShowString_rowcentering(20," Title ",Mysize[fontsize_setting].size_title,0);
    OLED_ShowString_rowcentering(48," Content ",Mysize[fontsize_setting].size_content,1);
    OLED_Refresh();
}

/**
 * @brief Fontsize节点函数(Data_set型), next 增大字号，previous 减小字号，enter确定，return取消 
 * 修改的同时会在屏幕上显示当前字号效果
 * @retval void
*/
void Func_Fontsize_set(void){
    switch (Data_keynum)
    {
    case 1: // previous
        fontsize_setting = (fontsize_setting==0)?2:(fontsize_setting-1);    
        Func_Fontsize_enter();
        break;
    case 2: // enter
        fontsize = fontsize_setting;
        OLED_Buffer_clear();
        OLED_ShowString_rowcentering(8,"config",24,1);
        OLED_ShowString_rowcentering(32,"success",24,1);
        OLED_Refresh();
        HAL_Delay(1000);
        KEY_Parent_return();
        break;
    case 3: // next
        fontsize_setting = (fontsize_setting==2)?0:(fontsize_setting+1);    
        Func_Fontsize_enter();
        break;
    case 5: // return
        fontsize_setting = fontsize;
        OLED_Buffer_clear();
        OLED_ShowString_rowcentering(8,"config",24,1);
        OLED_ShowString_rowcentering(32,"cancel",24,1);
        OLED_Refresh();
        HAL_Delay(1000);
        KEY_Parent_return();
        break;
    default:return;
    }
}

/**
 * @brief Smile节点函数(Loop_enter型) , 进入循环
 * @retval void
*/
/* void Func_Smile_enter(void){
    uint8_t i,x0,x1,y1;
    x0 = 32;
    OLED_Buffer_clear();
    OLED_ShowPicture(x0,0,64,64,Smile_Image,1);
    OLED_Refresh();
    for(i=0;i<255;i++){
        OLED_Refresh();
        if(Loop_keynum == 5){break;}
    }
} */

/**
 * @brief Smile节点函数(Loop_set型) , next 笑脸右移，previous 笑脸左移，enter 笑脸反色，return退出
 * @retval void
*/
/* void Func_Smile_set(void){
    switch (Data_keynum)
    {
    case 1: // previous 
        brightness_setting = (brightness_setting>25)?(brightness_setting-25):1;    // 亮度为0时会自动熄灭屏幕, 需重新点亮
        OLED_WR_Byte_DMA(OLED_CMD,0x81,1,1);OLED_WR_Byte_DMA(OLED_CMD,brightness_setting,1,1);
        Func_Brightness_enter();
        break;
    case 2: // enter
        brightness = brightness_setting;
        OLED_Buffer_clear();
        OLED_ShowString_rowcentering(8,"config",24,1);
        OLED_ShowString_rowcentering(32,"success",24,1);
        OLED_Refresh();
        HAL_Delay(1000);
        KEY_Parent_return();
        break;
    case 3: // next
        brightness_setting = (brightness_setting<201)?(brightness_setting+25):255;
        OLED_WR_Byte_DMA(OLED_CMD,0x81,1,1);OLED_WR_Byte_DMA(OLED_CMD,brightness_setting,1,1);
        Func_Brightness_enter();
        break;
    case 5: // return
        brightness_setting = brightness;
        OLED_WR_Byte_DMA(OLED_CMD,0x81,1,1);OLED_WR_Byte_DMA(OLED_CMD,brightness_setting,1,1);
        OLED_Buffer_clear();
        OLED_ShowString_rowcentering(8,"config",24,1);
        OLED_ShowString_rowcentering(32,"cancel",24,1);
        OLED_Refresh();
        HAL_Delay(1000);
        KEY_Parent_return();
        break;
    default:return;
    }
} */

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
    Data_keynum = 0;    
    Loop_keynum = 0;    
    brightness = OLED_StartBrightness;  
    brightness_setting = OLED_StartBrightness;    
    fontsize = OLED_StartFontsize;  
    fontsize_setting = OLED_StartFontsize;    

    Menu_Pointer = (Menu_typedef *)Main;
    Insert = 0;
    OLED_WR_Byte_DMA(OLED_CMD,0x81,1);
    OLED_WR_Byte_DMA(OLED_CMD,brightness_setting,1);
    Draw_Menu();
}

/**
 * @brief 获取当前菜单在父菜单中的索引值
 * @param menu 需要获取索引值的菜单
 * @retval uint8_t, 若为主菜单或检索错误返回 255=0xFF
*/
uint8_t Get_menu_index(Menu_typedef* menu){
    uint8_t x = 0;  // 当前菜单在父菜单中的索引值
    if( menu == Main ){ return 255; }
    else{
        for( x=0;x<100;x++ ){   // 每个 Parent 型菜单最大子菜单数目为 99
        if(menu->Parent[x].Child==menu){break;}
        }
        return x;
    }
}

/**
 * @brief 获取菜单
 * @param void
 * @param 
 * @retval void
*/
uint8_t Get_menu_namelenth(Menu_typedef* menu){
    return 3;
}

/**
 * @brief 空函数, 在屏幕上显示 "无效操作"
 * @retval void
*/
void Invalid_Operation(void){
    OLED_Buffer_clear();
    OLED_DrawLine(0,3,127,3,1);
    OLED_ShowString_rowcentering(8," invalid ",24,0);
    OLED_ShowString(10,32,"operation",24,0);
    OLED_DrawLine(0,61,127,61,1);
    OLED_Refresh();
    HAL_Delay(300);
    OLED_Buffer_clear();
    Draw_Menu();
}

/**
 * @brief 显示 Menu_Pointer 所指菜单
 * @retval void
*/
void Draw_Menu(void){
    uint8_t i=0; // 循环变量
    uint8_t len; // 菜单头名字长度
    uint8_t n; // 需要draw的行数
    uint8_t x; // 光标所在位置
    uint8_t rec_y; // 滑动条填充部分起始纵坐标
    uint8_t height; // 滑动条填充部分纵长
    switch (Insert)
    {
    case 0: 
        x= UserChoose-Current_showrange;
        break;
    case 1:
        x=0;Insert=0;
        break;
    case 2: 
        x = UserChoose; 
        Current_showrange = (UserChoose-2)>=0?(UserChoose-2):0;
        break;
    default:return;
    }
    while(' '<=*(Menu_Pointer->Name+i) && *(Menu_Pointer->Name+i)<='~'){i++;}    // 获取名字长度
    len = i;
    if(Menu_Pointer->Parent == NULL){ // 若为主菜单
        n = (Main_Child_nodesnumber<=Mysize[fontsize].row_number)?Main_Child_nodesnumber:Mysize[fontsize].row_number;
        rec_y = (uint8_t) 60*UserChoose/Main_Child_nodesnumber*((Insert==0)?1:0);
        height = (uint8_t) 60/Main_Child_nodesnumber-1;
    }
    else{
        n = (Menu_Pointer->Parent->Child_nodes_number<=Mysize[fontsize].row_number)?Menu_Pointer->Parent->Child_nodes_number:Mysize[fontsize].row_number;
        rec_y = (uint8_t) 60*UserChoose/Menu_Pointer->Parent->Child_nodes_number*((Insert==0)?1:0);
        height = (uint8_t) 60/Menu_Pointer->Parent->Child_nodes_number-1;
    }
    Insert=0;
    OLED_Buffer_clear();
    // 显示光标
    OLED_ShowChar(0,Mysize[fontsize].Mysize_array[x],'>',Mysize[fontsize].size_content,1);
    // 显示菜单头与子菜单
    for (i = 0; i < len+2; i++)
    {OLED_ShowChar(i*Mysize[fontsize].size_title/2,0,' ',Mysize[fontsize].size_title,0);}
    OLED_ShowString(Mysize[fontsize].size_title/2,0,(uint8_t *)Menu_Pointer->Name,Mysize[fontsize].size_title,0);
    for (i = 0; i <n; i++)  
    {
        OLED_ShowString(12,Mysize[fontsize].Mysize_array[i],(uint8_t *)(Menu_Pointer+i+Current_showrange)->Child->Name,Mysize[fontsize].size_content,1);
    }
    // 显示右侧滑动条
    OLED_DrawRectangle(118,0,10,64,1,0);
    OLED_DrawRectangle(120,2,6,60,1,0);
    OLED_DrawRectangle(121,3+rec_y,4,height,1,1);
    OLED_Refresh();
}

/* -------------上面是菜单内部函数------------- */


/* -------------下面是KEY函数(中断模式)------------- */
/**
 * @brief 按键总控函数, 任意按键按下时触发, 提取当前菜单属性并调用对应函数
 * @retval void
*/
void KEY_Pressed(uint8_t GPIO_pin){
    uint8_t ispressed;
    // 获取当前菜单属性
    enum MenuProperties property;   
    // -----------------------------------------------------
    HAL_Delay(15);  // 按键消抖
    ispressed = 
    (HAL_GPIO_ReadPin(Key_return_GPIO_Port,Key_previous_Pin)==KEY_Active_Volt)||
    (HAL_GPIO_ReadPin(Key_next_GPIO_Port,Key_next_Pin)==KEY_Active_Volt)||
    (HAL_GPIO_ReadPin(Key_enter_GPIO_Port,Key_enter_Pin)==KEY_Active_Volt)||
    (HAL_GPIO_ReadPin(Key_return_GPIO_Port,Key_return_Pin)==KEY_Active_Volt);
    /* 根据菜单属性调用函数 */
    if(ispressed == 1){     // 防误触
        if(Menu_Pointer == Main){ property = Menu_Parent; }
        else{property = Menu_Pointer->Parent[Get_menu_index(Menu_Pointer)].Child_Menuproperty;}
        switch (property)
        {
        case Menu_Parent:   // Menu_Parents 型菜单
            KEY_Parent_pressed(GPIO_pin);
            break;
        case Menu_Data:
            KEY_Data_pressed(GPIO_pin);
            break;
        case Menu_Once:   // Menu_Parents 型菜单
            KEY_Once_pressed(GPIO_pin);
            break;
        case Menu_Loop:   // Menu_Parents 型菜单
            break;
        default:
            break;
        }
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);  // 状态灯
    }
    else {return;}
    HAL_Delay(15);  // 按键消抖
}

/**
 * @brief 经过 KEY_Pressed() 函数判定当前菜单为 Parent 型后调用此函数
 * @param GPIO_pin 被按下的 GPIO 口
 * @retval void
*/
void KEY_Parent_pressed(uint8_t GPIO_pin){
     switch (GPIO_pin){
            case Key_previous_Pin:KEY_Parent_previous();break;
            case Key_next_Pin:KEY_Parent_next();break;
            case Key_enter_Pin:KEY_Parent_enter(); break;
            case Key_return_Pin:KEY_Parent_return();break;
            default: return;
        }
}

void KEY_Parent_return(void){
    if(Menu_Pointer->Parent == NULL){   // 根菜单检测
        Insert = 0;
        return; // return不能省略
    }  
    else{ 
        Insert = 2;
        UserChoose = Get_menu_index(Menu_Pointer);
        Menu_Pointer = (Menu_typedef *)Menu_Pointer->Parent;
        Draw_Menu();
    }
}

void KEY_Parent_next(void){
    uint8_t nodes_number = Menu_Pointer->Parent->Child_nodes_number;
    if(Menu_Pointer == Main){nodes_number = Main_Child_nodesnumber;}    
    // 判断 当前菜单子项目数 与 最大显示行数 大小关系
    if(nodes_number<=Mysize[fontsize].row_number){
        if(UserChoose == nodes_number-1 ){UserChoose = 0;}
        else{UserChoose++;}
    }
    else{
        if( UserChoose == Current_showrange + Mysize[fontsize].row_number -1 ){   // 到达显示下边界
            if( UserChoose == nodes_number-1 )
            {Current_showrange=0;UserChoose=0;}    // 到达选项下边界
            else{Current_showrange++;UserChoose++;}
        }
        else{   // 未到达显示边界
            UserChoose++;
        }
    }
    Draw_Menu();
}

void KEY_Parent_previous(void){
    uint8_t nodes_number = Menu_Pointer->Parent->Child_nodes_number;
    if(Menu_Pointer == Main){nodes_number = Main_Child_nodesnumber;}    
    // 判断 当前菜单子项目数 与 最大显示行数 大小关系
    if(nodes_number<=Mysize[fontsize].row_number){
        if(UserChoose == 0 ){UserChoose = nodes_number-1;}
        else{UserChoose--;}
    }
    else{
        if( UserChoose == Current_showrange ){   // 到达显示上边界
            if( UserChoose == 0 )      // 到达选项上边界
            {Current_showrange=nodes_number-Mysize[fontsize].row_number; UserChoose=nodes_number-1;}    
            else{Current_showrange--; UserChoose--;}
        }
        else{   // 未到达显示边界
            UserChoose--;
        }
    }
    Draw_Menu();
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
 * @brief 经过 KEY_Pressed() 函数判定当前菜单为 Data 型后调用此函数
 * @param GPIO_pin 被按下的 GPIO 口
 * @retval void
*/
void KEY_Data_pressed(uint8_t GPIO_pin){
     switch (GPIO_pin){
            case Key_previous_Pin:
                    Data_keynum = 1;
                    Menu_Pointer->func();
                    Data_keynum = 0;
                break;
            case Key_next_Pin:
                    Data_keynum = 3;
                    Menu_Pointer->func();
                    Data_keynum = 0;
                break;
            case Key_enter_Pin:
                    Data_keynum = 2;
                    Menu_Pointer->func();
                    Data_keynum = 0;
                break;
            case Key_return_Pin:
                    Data_keynum = 5;
                    Menu_Pointer->func();
                    Data_keynum = 0;
                break;
            default: return;
        }
}


/**
 * @brief 经过 KEY_Pressed() 函数判定当前菜单为 Once 型后调用此函数
 * @param GPIO_pin 被按下的 GPIO 口
 * @retval void
*/
void KEY_Once_pressed(uint8_t GPIO_pin){
    KEY_Parent_return();
}
 
/**
 * @brief 经过 KEY_Pressed() 函数判定当前菜单为 Loop 型后调用此函数
 * @param GPIO_pin 被按下的 GPIO 口
 * @retval void
*/
void KEY_Loop_pressed(uint8_t GPIO_pin){
        switch (GPIO_pin){
        case Key_previous_Pin:
                Loop_keynum = 1;
                Menu_Pointer->func();
                Loop_keynum = 0;
            break;
        case Key_next_Pin:
                Loop_keynum = 3;
                Menu_Pointer->func();
                Loop_keynum = 0;
            break;
        case Key_enter_Pin:
                Loop_keynum = 2;
                Menu_Pointer->func();
                Loop_keynum = 0;
            break;
        case Key_return_Pin:
                Loop_keynum = 5;
                Menu_Pointer->func();
                Loop_keynum = 0;
            break;
        default: return;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == Key_previous_Pin||Key_next_Pin||Key_enter_Pin||Key_return_Pin )
    {KEY_Pressed(GPIO_Pin);}
}
/* -------------上面是 KEY函数(中断模式)------------- */


