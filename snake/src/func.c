#include <stdio.h>
// 伟大，无需多言
#include <string.h>
// 用于memset，一个很容易自实现的函数（但是效率会下降）
#include <stdlib.h>
// 伟大，无需多言
#include "main.h"
// 不喜欢就把东西全部搬来main.c文件
#include <unistd.h>
// 用了里面的sleep函数，仅linux。unix支持此系统，你要求的程序停止5秒
#include <pthread.h>
// 仅限linux，unix，用于构造多线程执行，这里用来实现非阻滞获取用户输入
#define END -1
#define DEBUG 0
// TODO当前还有的问题是蛇蛇允许向后走，显然蛇只能前左右走，

/*
打开游戏时有开始界面，点击“开始游戏”
游戏中途有暂停功能，暂停时可以继续可以返回（不保存游戏数据）
每次“开始游戏”都是新游戏，仅保存最高记录，在开始界面和游戏结束时显示
*/
// 甚至不一定要一个数组来存储数据，贪吃蛇本身的数据已经足够模拟游戏了
// 除非需要内存优化，否则姑且还是依托数组来进行贪吃蛇游戏
// 用int last_loop_game_state=RUN;控制show函数不重复输出太愚蠢了，应该考虑show计数，保证同一个show不连续调用（draw）不算
// TODO有一个问题在于，在输入后
unsigned int highestscore = 0x0;
int current_key_value = Zero;
int current_game_state = OVER;
int last_show = 7;
unsigned int current_score = 0x0;
unsigned int Pixel_Data[64][4];
int last_loop_game_state = RUN;
Location *Snake_Head_Location;
Location *Snake_Tail_Location;
Location Bouns_Location;
Location the_end = {END, END, NULL};
void Game_Loop()
{
    sleep(1);
    switch (current_game_state)
    {
    case STOP:
        event_STOP();
        break;
        ;
    case RUN:
        event_RUN();
        break;
        ;
    case OVER:
        event_OVER();
        break;
    }
}
void event_STOP()
{
    switch (current_key_value)
    {
    case Enter:
        current_game_state = RUN;
        break;
    case Return:
        Game_exit();
        break;
    }
    current_key_value = Zero;

    return;
}
void event_RUN()
{
    if (current_key_value == Return)
    {
        current_game_state = STOP;
        STOP_Menu_Show();
        current_key_value = Zero;
        return;
    }
 Better_Ingame_Draw();
   // Draw_Ingame_Image();
    Snake_Move(current_key_value);
    // 在linux由于可以接受多种键值，接受异常键值时会因为无法解析移动方向而保持不动，从而被判定为撞到了自己身体
    if (Judge_Death())
    {
        current_game_state = OVER;
        DestroyAllNode(); // 防止内存溢出
        printf("\n");
        DEATH_Menu_Show();
        for (int i = 0; i < 5; i++)
        {
            printf("\n%d", i + 1);
            sleep(1);
            printf("\n");
        }
        RESTART_Menu_Show();
    }

    return;
}
void event_OVER()
{ // TODO
    START_Menu_Show();
    switch (current_key_value)
    {
    case Return:
        Game_exit();
        break;
    case Enter:
        Game_Init();
        current_game_state = RUN;
        break;
    }
    current_key_value = Zero;
}
void Random_Write(unsigned int i, unsigned int j, unsigned int num)
{
    num = !(!num);
    if (num)
    {
        Pixel_Data[i][j >> 5] = (Pixel_Data[i][j >> 5] | (1 << (31 - j & 0x1F)));
    }
    else
    {
        Pixel_Data[i][j >> 5] = Pixel_Data[i][j >> 5] & (~(1 << (31 - j & 0x1F)));
    }

    // printf("%d\n",(Pixel_Data[i][j>>5]>>(31-j&0x1))&0x1);
}
unsigned char Random_Read(unsigned int i, unsigned int j)
{
    return (Pixel_Data[i][j >> 5] >> (31 - j & 0x1F)) & 0x1;
}
void Game_Init()
{
    for (int i = 0; i < 64; i++)
    {
        memset(Pixel_Data[i], 0x0, 128);
    }
    // Field Init,
    // I dont want to write a single func to implement it
    current_score = 0;

    // score init
    // TODO
    Snake_Tail_Location = CreatNode(Get_Random(62, 1), Get_Random(126, 1), &the_end);
    Random_Write(Snake_Head_Location->i, Snake_Head_Location->j, Body);
    Bonus_Spwan();
}
void Bonus_Spwan()
{
    int j; // TODO bouns cant be shown at image
    int i;

    do
    {
        i = Get_Random(62, 1);
        j = Get_Random(126, 1);
        /* code */
    } while (Random_Read(i, j) == Body);
#if DEBUG
    printf("spawn Bonus at%d %d", i, j);
#endif
    Bouns_Location.i = i, Bouns_Location.j = j;
    Random_Write(i, j, Body);
    return;
}
int Get_Random(int ceil, int floor)
{
    return rand() % (ceil - floor) + floor;
}
void Game_exit()
{
    exit(0);
}
int Judge_Death()
{
    if (Snake_Head_Location->i == 0 || Snake_Head_Location->i == 63 || Snake_Head_Location->j == 0 || Snake_Head_Location->j == 127)
    {
#if DEBUG
        printf("WALL STRIKE");
#endif
        return true;
    }
    if (Random_Read(Snake_Head_Location->i, Snake_Head_Location->j) == Body)
    {
        if (Bouns_Location.i != Snake_Head_Location->i || Bouns_Location.j != Snake_Head_Location->j)
        {
#if DEBUG
            printf("body strike\n");

            printf("Bonus at %d %d Head strike at %d %d\n", Bouns_Location.i, Bouns_Location.j, Snake_Head_Location->i, Snake_Head_Location->j);
#endif
            return true;
        } // 撞到自己身体了
        // 否则，吃到了bonus
        current_score += 100;
        Bonus_Spwan();
        return false;
    }
    Erase(Snake_Tail_Location);
    // 没有吃到bonus，尾巴前移
    Random_Write(Snake_Head_Location->i, Snake_Head_Location->j, Body);
    // TODO
    return false;
}
void Snake_Move(int direction)
{ // 注意，移动后对应head所指位置还没有修改值，方便判定，具体值修改由Judge_Death代行
    switch (current_key_value)
    {
    case UP:
        CreatNode(Snake_Head_Location->i - 1, Snake_Head_Location->j, Snake_Head_Location);
        break;
    case RIGHT:
        CreatNode(Snake_Head_Location->i, Snake_Head_Location->j + 1, Snake_Head_Location);
        break;
    case DOWN:
        CreatNode(Snake_Head_Location->i + 1, Snake_Head_Location->j, Snake_Head_Location);
        break;
    case LEFT:
        CreatNode(Snake_Head_Location->i, Snake_Head_Location->j - 1, Snake_Head_Location);
        break;
    }
    // 蛇头伸长了一截
    // TODO需要检查头部有没有bonus，有就调用bonus——spwan，否则将尾巴一段砍掉
    /*
    if(Bouns_Location.i==Snake_Head_Location->i&&Bouns_Location.j==Snake_Head_Location->j)
    {
    Bonus_Spwan();
    //TODO由于对头部位置赋值BODY属性在本函数调用结束后才发生，这里一定有可能bonus又生成在原位置

    }
    else
    {
    //TODO
    }
    */
}
void STOP_Menu_Show()
{
    if (last_show == STOP_Show)
    {
        return;
    }
    else
    {
        printf("Paused\nDo you want to continue? \nYes for Enter/No for Return");
        last_show = STOP_Show;
    }
}
void DEATH_Menu_Show()
{
    if (last_show == DEATH_Show)
    {
        return;
    }
    else
    {
        printf("You Died!该罚！\n ");
        if (current_score > highestscore)
        {
            printf("New record:%d", current_score);
        }
        else
        {
            printf("Your Score:%d\nHistory Best:%d", current_score, highestscore);
        }
    }
}
void RESTART_Menu_Show()
{
    START_Menu_Show();
}
void START_Menu_Show()
{
    if (last_show == START_Show)
    {
        return;
    }
    else
    {
        printf("Press Enter to start a new game\nPress Return to exit game\n");
        last_show = START_Show;
    }
}
Location *CreatNode(int i, int j, Location *last)
{
    Location *new_node = (Location *)malloc(sizeof(Location));
    last->next = new_node;
    new_node->next = NULL;
    new_node->i = i;
    new_node->j = j;
    Snake_Head_Location = new_node;
    return new_node;
}
void DestroyAllNode()
{
    // delete all node but the_end

    DestroyNextNode(&the_end);
}
void DestroyNextNode(Location *p)
{ // 释放p以后的所有节点
    if (p->next->next != NULL)
    {
        DestroyNextNode(p->next);
    }
    free(p->next);
}
void Draw_Ingame_Image()
{
    last_show = Draw_Image;
    // TODO
    // 压根没必要把地图数据全部访问一遍
    // 只需要根据蛇链表的数据初始化特殊printf
    // 事实上，这个地图数据有点多余，用蛇蛇链表，和bonus，以及其他参数就可以描述整个运行中的游戏状态了
    // 而且这样做的话后期修改地图大小也容易
    // 因为对地图数据的访问读写与地图大小是高耦合的
    putchar(10);
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 128; j++)
        {
            if (i == 0 || i == 63 || j == 0 || j == 127)
                printf("0");
            else
                printf("%c", (Random_Read(i, j)) ? 'a' : ' ');
        }
        printf("\n");
    }
}
void Better_Ingame_Draw()
{
    #if DEBUG

printf("\nCalled\n");
sleep(5);
    #endif
    last_show = Draw_Image;
    putchar(10);

    // 一次性获知蛇和bonus在哪些位置，然后记录下来
printf("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
for(int i=1;i<63;i++)
{
printf("0");

for(int j=0;j<4;j++)
{
if(Pixel_Data[i][j]==0)
{
    printf("                                ");
}
else
{

for(int k=j*32;k<j*32+32;k++)
{
 printf("%c",(Random_Read(i, k)) ? 'a' : ' ');    
}
//TODO还可以继续二分搜索
}



}


printf("0\n");
}









printf("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
#if DEBUG
printf("\nPaused\n");   
sleep(2);
#endif
}
// 连续暂停后无法显现界面（解决了，问题出自上面的函数没改
void Erase(Location *p)
{
    // 由于本函数只用于擦尾巴，干脆把别的活也做了
    Random_Write(p->i, p->j, Null_);
    the_end.next = p->next;
    free(p);
    Snake_Tail_Location = the_end.next;
}