#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include "main.h"
#include<time.h>
 //lacking extern
 extern unsigned int current_key_value;
 void *Get_Data(void *threadid) {  
  while (1)
  {
    scanf("%d",(int *)threadid);
  }
  
    pthread_exit(NULL);  
}  


int main(){
    pthread_t thread_;
    pthread_create(&thread_,NULL,Get_Data,&current_key_value);

    srand(time(NULL));
    while (1)
    {
      
        Game_Loop();
    }
    
    
    return 0;}