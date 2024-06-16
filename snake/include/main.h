#ifndef PRO
#define PRO
typedef struct Location Location ;
enum KEY_VALUE_SET
{
    Return=5,
    Zero=0,
Enter=2,
Previous=1,
Next=3
};
enum LAST_SHOW
{STOP_Show,
DEATH_Show,
START_Show,
Draw_Image
};
enum DIRECTION
{
    UP=Zero,
   RIGHT= Previous,
   DOWN=Enter,
   LEFT=Next
};
void Game_Loop();
void Erase(Location * p);//擦去链表所指的节点
//事实上，因为有the——end是链表结尾，可以考虑删去snake——tail——location
//TODO优化上，可以制造指向下一个的单向链表
void event_STOP();
void event_RUN();
Location * CreatNode(int i,int j,Location * last);
void DestroyAllNode();
void DestroyNextNode();
void event_OVER();
void Game_exit();
int Judge_Death();
void Snake_Move(int direction);
int Death_Judge();
void STOP_Menu_Show();
void DEATH_Menu_Show();
void RESTART_Menu_Show();
void Bonus_Spwan();
void START_Menu_Show();
void Draw_Ingame_Image();
int Get_Random(int ceil,int floor);
void Game_Init();
void Random_Write(unsigned int i,unsigned int j,unsigned int num);
unsigned char Random_Read(unsigned int i,unsigned int j);
enum GROUND_STATE_SET
{
Null_=0,Body=1
};
enum LOGIC_VALUE
{
false=0,
true=1
};
enum GAME_STATE_SET
{
STOP=0,
RUN=1,
OVER=2
};
typedef struct Location
{
char i;
char j;
Location* next;
} Location;
#endif




