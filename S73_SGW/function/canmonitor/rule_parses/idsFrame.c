#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ids_config.h"
#include "log.h"
#include "platformtypes.h"
#include "idsFrame.h"
#include <stdio.h>
#include "rule.h"
#include "ctimer.h"
#include "event.h"
#include "cJSON.h"
#include "CANIDS_Interface.h"
#include "CANIDS_Working.h"

// 各个模块启动状态
static  char startFalg    = 0;   // 启动标志
static  bool flowSwitch   = 0;   // 流量检测开关/负载检测开关/doc监测开关
static  bool listSwitch   = 0;   // 白名单检测开关 
static  bool lengthSwitch = 0;   // DLC开关
static  bool priodSwitch  = 0;   // 周期检测开关
static  int  signaSwitch  = 0;   //0x100+0x20;   // 信号分析开关，8位代表8个功能

// 框架初始化 


int can_device_pub_dat(unsigned char netID, unsigned int canID, unsigned char* data, unsigned int len, double time)
{

	SK_CANIDS_MsgReceiveIndi(netID, canID, data, len);

	return 0;
}


// 时钟计数
pthread_t thread_tid;//线程ID
void *thread_time_work(void * arg)
{
    //pthread_detach(pthread_self());
	printf("Time thread running\n");
    while(1)
    {
     

        // 5ms
        usleep(5000);
        SK_CANIDS_5ms_MainRunnable();
        pthread_testcancel();
    }
	pthread_exit("thanks for you cup time!\n");
}

// can 初始化, 线程创建
int can_init(int argc, char *rule)
{
    IDS_Stru canIDS = { 0 };
    
    // 框架启动
    SK_CANIDS_Init(canIDS, rule);
    SK_CANIDS_Start();
	int ret = pthread_create(&thread_tid, NULL, thread_time_work, 0);//创建线程
	if(ret != 0){
		perror("create thread error!");
		exit(-1);
	}

    //等待子线程结束
    void *thread_rel;
	//pthread_join(thread_tid, &thread_rel);
    return 0;
}

