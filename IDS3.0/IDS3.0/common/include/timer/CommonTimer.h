/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-09 02:36:52
 */ 
 #ifndef __COMMON_TIMER_H_
 #define __COMMON_TIMER_H_
 #ifdef __cplusplus
extern "C"
{
#endif 
#include <pthread.h>
/**
 * @name:   timer status
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
typedef struct _timerIdps{
   int minter;
   char startstatus;
   char runingstatus;
   void (*TimerCall)(void);
   pthread_t mtimerthread;
}timerIdps;
/**
 * @name:   function decla
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
timerIdps *newtime(void* _callback);
void starttime(timerIdps* _input);
void stoptimer(timerIdps* _input);
void freetimer(timerIdps* _input);
void setInterval(int inter,timerIdps* _input);
#ifdef __cplusplus
}
#endif
 #endif 
