#include "main.h"

#ifndef _MUTIMENU_H_
#define _MUTIMENU_H_

// 定义菜单底层结构体
typedef struct Menu{
  char *Name;		                  // 子节点标题
  const struct Menu *Parent;	          // 父节点结构体数组
  const struct Menu *Child;  	          // 子节点结构体数组	
  void (*func)(void);			              // 子节点函数(进入子节点时调用)
	uint8_t Child_nodes_number;		      // 子节点的子节点数 
} Menu_typedef;

// 菜单节点函数声明

void Draw_Menu(void);
void Enter_invalid(void);
void Function_About(void);
void Return_Menu(void);
void Run_Mainmenu(void);


#endif // _MUTIMENU_H_
