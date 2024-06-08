#include "main.h"
#include "oled.h"
#include "MutiMenu.h"


// 排版布局控制
#define OLED_Width 128
#define OLED_Height 64

/*-------------- 下面是字号大小选择 -------------- */
// #define Size_0806
#define Size_1206   
// #define Size_1608

const uint8_t Mysize_08[] = {12,20,28,36,44,52};
const uint8_t Mysize_12[] = {16,28,40,52};
const uint8_t Mysize_16[] = {24,40,56};
typedef struct Size
{
    const uint8_t size;
    const uint8_t row_number;
    const uint8_t* Mysize_array;
}Mysize_typedef;

#if defined(Size_0806)
const struct Size Mysize = {8, 6, Mysize_08};
#endif // Size_0806

#if defined(Size_1206)
const Mysize_typedef Mysize = {12, 4, Mysize_12};
#endif // Size_1206

#if defined(Size_1608)
const struct Size Mysize = {16, 3, Mysize_16};
#endif // Size_1608

uint8_t Current_showrange = 0;  // 屏幕最大显示行数为 Mysize.row_number
/*-------------- 上面是字号大小选择 -------------- */



// 菜单节点声明
#define Main_Child_nodesnumber 3    // 宏定义主菜单子节点数,方便操作

const Menu_typedef 
Main[Main_Child_nodesnumber],
    Main_Settings[2],
        Main_Settings_Constract[1],
        Main_Settings_Luminance[1],     
    Main_Hello[2],
        Main_Hello_Sayhello[1],
        Main_Hello_Smile[1],
    Main_About[1],
    Main_Menu4[1],
    Main_Menu5[1],
    Main_Menu6[1];

/* ------------------------------下面是菜单栏定义---------------------------- */

/* Menu_Root, Menu_0 */
Menu_typedef *Menu_Pointer =  (Menu_typedef *)Main; // 设置当前菜单为 Main 并初始化

char UserChoose = 0; 		// 初始化光标为第 0 行
const Menu_typedef Main[Main_Child_nodesnumber] = {				
    {"Main",    NULL, Main_Settings,  Draw_Menu,         2},
    {"Main",    NULL, Main_Hello,     Draw_Menu,         2},
    {"Main",    NULL, Main_About,     Function_About,    1},
};

/* Menu_1 */
const Menu_typedef Main_Settings[2] = {				
    {"Settings", Main,  Main_Settings_Constract,   Enter_invalid, 1},
    {"Settings", Main,  Main_Settings_Luminance,   Enter_invalid, 1}
};

const Menu_typedef Main_Hello[2] = {				
    {"Hello", Main,  Main_Hello_Sayhello,     Enter_invalid, 0},
    {"Hello", Main,  Main_Hello_Smile,     Enter_invalid, 0}
};

const Menu_typedef Main_About[1] = {{"About",Main,NULL,Enter_invalid, 0},};
const Menu_typedef Main_Menu4[1] = {{"Menu4",Main,NULL,Enter_invalid, 0},};
const Menu_typedef Main_Menu5[1] = {{"Menu5",Main,NULL,Enter_invalid, 0},};
const Menu_typedef Main_Menu6[1] = {{"Menu6",Main,NULL,Enter_invalid, 0},};

/* Menu_2*/

const Menu_typedef Main_Settings_Constract[1] = {{"Constract",Main_Settings,NULL,Enter_invalid, 0},};
const Menu_typedef Main_Settings_Luminance[1] = {{"Luminance",Main_Settings,NULL,Enter_invalid, 0},};
const Menu_typedef Main_Hello_Sayhello[1] = {{"Sayhello",Main_Hello,NULL,Enter_invalid, 0},};
const Menu_typedef Main_Hello_Smile[1] = {{"Smile",Main_Hello,NULL,Enter_invalid, 0},};


/* ------------------------------上面是菜单栏定义---------------------------- */


/* -------------下面是菜单节点函数------------- */

/**
 * @brief 空函数, 在屏幕上显示 "无效操作"
 * @retval void
*/
void Enter_invalid(void){
    OLED_Buffer_clear();
    OLED_DrawLine(0,3,127,3,1);
    OLED_ShowString_rowcentering(8," invalid ",24,0);
    OLED_ShowString(10,32,"operation",24,0);
    OLED_DrawLine(0,61,127,61,1);
    OLED_Refresh();
    HAL_Delay(2000);
    OLED_Buffer_clear();
    Menu_Pointer = (Menu_typedef *)Menu_Pointer->Parent;
    Draw_Menu();
}

/**
 * @brief 运行主菜单
 * @param void
 * @param 
 * @retval void
*/
void Run_Mainmenu(void){
    // 这里添加一个动画
    Menu_Pointer = (Menu_typedef *)Main;
    Draw_Menu();
}

/**
 * @brief 进入子级菜单
 * @retval void
*/
void Draw_Menu(void){
    uint8_t i,n;  
    if(Menu_Pointer->Parent == NULL){ // 若为主菜单
        n = (Main_Child_nodesnumber<=Mysize.row_number)?Main_Child_nodesnumber:Mysize.row_number;
    }
    else{
        n = (Menu_Pointer->Parent->Child_nodes_number<=Mysize.row_number)?Menu_Pointer->Parent->Child_nodes_number:Mysize.row_number;
    }
    OLED_Buffer_clear();
    OLED_ShowString(0,0,"                ",16,0);
    OLED_ShowString(0,0,(uint8_t *)Menu_Pointer->Name,16,0);
    OLED_ShowChar(0,Mysize.Mysize_array[0],'>',Mysize.size,1);
    for (i = 0; i < n; i++)
    {
        OLED_ShowString(12,Mysize.Mysize_array[i],(uint8_t *)(Menu_Pointer+i)->Child->Name,Mysize.size,1);    // 显示各选项名字
    }
    OLED_Refresh();
    UserChoose = 0;     // 作图完毕后光标归零
}

/**
 * @brief 返回父级菜单
 * @retval void
*/
void Return_Menu(void){
    if(Menu_Pointer->Parent == NULL){   // 根菜单检测
        OLED_Buffer_clear();
        OLED_DrawLine(0,3,127,3,1);
        OLED_ShowString(10,8," invalid ",24,0);
        OLED_ShowString(10,32,"operation",24,0);
        OLED_DrawLine(0,61,127,61,1);
        OLED_Refresh();
        HAL_Delay(2000);
        Draw_Menu();    
    }  
    else{ 
        Menu_Pointer = (Menu_typedef *)Menu_Pointer->Parent;
        Draw_Menu();
    }
}

void Function_About(void){
    OLED_Buffer_clear();
    OLED_ShowString(0,0,"[About]",16,1); 
    OLED_ShowString(0,16,"Author: Star",12,1);
    OLED_ShowString(0,28,"Version: 1.0",12,1);
    OLED_ShowString(0,40,"Time: 24.5.6",12,1);
    OLED_Refresh();
}

/* -------------上面是 菜单函数------------- */

/* -------------下面是 KEY函数(中断模式)------------- */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    HAL_Delay(10);
    switch (GPIO_Pin)
    {
    case KEY_previous_Pin:
        if(HAL_GPIO_ReadPin(KEY_previous_GPIO_Port,KEY_previous_Pin) == 0){
            OLED_ShowChar(0,Mysize.Mysize_array[UserChoose],' ',12,1);  // 消除原光标
            // 移动范围检测
            // 主菜单单独控制
            if(Menu_Pointer == Main){
                // 判断 当前菜单子项目数 与 最大显示行数 大小关系
                if(Main_Child_nodesnumber<=Mysize.row_number){
                    if(UserChoose == 0 ){UserChoose = Main_Child_nodesnumber-1;}
                    else{UserChoose--;}
                }
                else{}
            }
            else{                
              if(UserChoose == 0 ){UserChoose = Menu_Pointer->Parent->Child_nodes_number-1;}
              else{UserChoose--;}
            }
            OLED_ShowChar(0,Mysize.Mysize_array[UserChoose],'>',12,1);
            OLED_Refresh();
            HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
        }
        break;
    case KEY_next_Pin:
        if(HAL_GPIO_ReadPin(KEY_next_GPIO_Port,KEY_next_Pin) == 0){
            OLED_ShowChar(0,Mysize.Mysize_array[UserChoose-Current_showrange],' ',12,1);
            // 移动范围检测
            // 主菜单单独控制
            if(Menu_Pointer == Main){
                // 判断 当前菜单子项目数 与 最大显示行数 大小关系
                if(Main_Child_nodesnumber<=Mysize.row_number){
                    if(UserChoose == Main_Child_nodesnumber-1 ){UserChoose = 0;}
                    else{UserChoose++;}
                    OLED_ShowChar(0,Mysize.Mysize_array[UserChoose-Current_showrange],'>',12,1);
                    OLED_Refresh();
                }
                else{
                    // 若 Userchoose 为当前显示边界
                    if(UserChoose == Current_showrange + Mysize.row_number -1 ) 
                    {
                        if(UserChoose <= Main_Child_nodesnumber-2){ // 若 Usershoose 未到达选项边界
                            Current_showrange++;UserChoose++;
                        }   
                        else{   // 到达选项边界
                            Current_showrange=0;
                            UserChoose=0;
                        }
                    }
                    // 若 Userchoose 非当前显示边界
                    else{   
                    UserChoose++;
                    OLED_ShowChar(0,Mysize.Mysize_array[UserChoose-Current_showrange],'>',12,1);
                    OLED_Refresh();
                    }
                }
            }
            // 其它菜单
            else{                
                if(UserChoose == Menu_Pointer->Parent->Child_nodes_number-1 ){UserChoose = 0;}
                else{UserChoose++;}
            }

            HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
        }
        break;
    case KEY_enter_Pin:
        if(HAL_GPIO_ReadPin(KEY_enter_GPIO_Port,KEY_enter_Pin) == 0){
            HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);  // 状态灯
            // 进入子页面
            Menu_Pointer = (Menu_typedef *) (Menu_Pointer+UserChoose)->Child;
            (Menu_Pointer->Parent+UserChoose)->func();
            UserChoose = 0;  
        }
        break;
    case KEY_return_Pin:
        if(HAL_GPIO_ReadPin(KEY_return_GPIO_Port,KEY_return_Pin) == 0){
            HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);  // 状态灯
            Return_Menu();
        }
        break;
    default:
        break;
    }
}

/* -------------上面是 KEY函数(中断模式)------------- */

