/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-27 22:12:59
 */ 
#include "CommonTimer.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "spdloglib.h"
/**
 * @name:   sleepthread
 * @Author: qihoo360
 * @msg:    
 * @param    
 * @return: 
 */
static void sleepthread(int seconds){
    struct timeval tv;
    tv.tv_sec=seconds;
    tv.tv_usec=0;
    int err;
    do{
       err=select(0,NULL,NULL,NULL,&tv);
    }while(err<0 && errno==EINTR);
}
/**
 * @name:   threadrun
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void *threadrun(void *args){
   timerIdps* l_local = args;
   pthread_detach(pthread_self());
   do{
      sleep(1);
   }while(l_local->startstatus == 0);
   while(l_local->runingstatus == 1){
      sleepthread(l_local->minter);
      do{
         sleep(1);
       }while(l_local->startstatus == 0);
      (l_local->TimerCall)();
   }
   free(l_local);
   return NULL;
}
/**
 * @name:   newtime
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
timerIdps *newtime(void* _callback){
   timerIdps *l_timer = (timerIdps* )malloc(sizeof(timerIdps));
   if(l_timer == NULL){
      log_v("idps main","space no error malloc"); 
   }
   l_timer->TimerCall        = _callback;
   l_timer->minter           =  60;//init value
   l_timer->runingstatus     =  1;//runing
   l_timer->startstatus      =  0;//start status

   int stacksize = 1*1024*1024;//1M
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr); 
	if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0){
		log_i("idps main","statcksize set error");
   }
	pthread_create(&(l_timer->mtimerthread),&attr,threadrun,l_timer);
	if((ret = pthread_attr_destroy(&attr)) != 0)
		log_i("idps_main","thread attr destory error\n");  
   return  l_timer;

}
/**
 * @name:   starttime，启动定时器
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void starttime(timerIdps* _input){
   _input->startstatus  = 1;
}
/**
 * @name:   stoptimer，暂停定时器
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void stoptimer(timerIdps* _input){
  _input->startstatus = 0;
}
/**
 * @name:   freetimer，释放定时任务
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
void freetimer(timerIdps* _input){
   _input->runingstatus = 0;
   _input->startstatus = 1;
}
/**
 * @name:   setInterval
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void setInterval(int inter,timerIdps* _input){
   _input->minter = inter;
}