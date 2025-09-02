/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 2020-11-23 20:58:59
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-05-09 19:15:21
 */
#include<stdio.h>
#include<string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "typedef.h"
#include "dpi_report.h"
#include <semaphore.h>
#include <stdbool.h>
#include "api_networkmonitor.h"
#include "spdloglib.h"

unsigned int modeIDPS = 0;
unsigned int thresholdLog = 0;
// 上报攻击开关
#define TYPESATTACK_NUM    (100)
static int netEventReportSwitch[TYPESATTACK_NUM] = {0};

#define MAXLENGTH (10240)
#define SING_MUX  (2000)
//fifo struct
typedef struct __report{
	u8 event;
	s32 port;
	s8  s_addr[32];
}report;
report 	m_u8Buf[MAXLENGTH] = {0};
static unsigned int fifoin = 0;
static unsigned int fifooutp = 0;
static pthread_mutex_t request_dpi_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t thread_log_thd = 0;
static boolean exit_thread_log_thd = FALSE;
static int s_report_user_login_flag = 0;
static char s_login_address[32] = {0};

/**
 ******************************************************************************
 ** \简  述  fifoout函数
 **  注  意   
 ** \参  数  none
 ** \返回值  none
 ** \作  者
 ******************************************************************************/
void *threadfifoout(void* arg){
	unsigned int local = 0;
	unsigned int loop = 0;
	u8 event_last = 0xff;
	int  iprulecount = 0;
	while(1)
	{
		pthread_mutex_lock(&request_dpi_lock);
		local = fifoin;
		pthread_mutex_unlock(&request_dpi_lock);
		while(local != fifooutp)
		{
			if(m_u8Buf[fifooutp].event != 0xff){
				if((event_last != m_u8Buf[fifooutp].event)||(loop > SING_MUX)){
					loop = 0;
					on_NetEventReport_callback(m_u8Buf[fifooutp].event,m_u8Buf[fifooutp].s_addr,m_u8Buf[fifooutp].port);
				}
				else{
					loop ++;
				}
				event_last = m_u8Buf[fifooutp].event;
			}
			m_u8Buf[fifooutp].event = 0xff;
			fifooutp = (fifooutp+ 1)%MAXLENGTH; 
			usleep(1000*5);
		}

		if (s_report_user_login_flag)
		{
			on_onUserLoginEvent_callback(s_login_address);
			s_report_user_login_flag = 0;
		}
		usleep(1000*20);
		event_last = 0xff;
		loop       = 0;

		if (exit_thread_log_thd == TRUE)
		{
			printf("pthread_exit threadfifoout log\n");
			pthread_exit(0);
		}
	}
}
/**
 ******************************************************************************
 ** \简  述  thread start
 **  注  意   
 ** \参  数  none
 ** \返回值  none
 ** \作  者
 ******************************************************************************/
void startlog(void)
{
	int stacksize = 6*1024*1024; 
	pthread_attr_t attr;

	fifoin = 0;
	fifooutp = 0;
	for(int i = 0;i < MAXLENGTH;i++)
		m_u8Buf[i].event = 0xff;
	int ret = pthread_attr_init(&attr); 
	if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0){
		char log[256] = {0};
		sprintf(log,"%s","statcksize set error");
		log_e("networkmonitor", log);
	}
	pthread_create(&thread_log_thd,&attr,threadfifoout,NULL);
	if((ret = pthread_attr_destroy(&attr)) != 0){
		char log[256] = {0};
		sprintf(log,"%s","thread attr destory error");
		log_e("networkmonitor", log);
	}
}

void report_log(u8 event,s8 *s_addr,s32 port)
{
	//on_NetEventReport_callback(event,s_addr,port);
	// 判断是否上报
	if(event<TYPESATTACK_NUM && !netEventReportSwitch[event])
		return;

	pthread_mutex_lock(&request_dpi_lock);
#if 1
	m_u8Buf[fifoin].event = event;
	m_u8Buf[fifoin].port  = port;
	memset(m_u8Buf[fifoin].s_addr,0,32);
	if(s_addr != NULL){
		strncpy(m_u8Buf[fifoin].s_addr,s_addr,32);
	}
	fifoin = (fifoin + 1)%MAXLENGTH;
#endif
	pthread_mutex_unlock(&request_dpi_lock);
	return;
}

void report_user_login_log(char *address)
{
	strncpy(s_login_address, address, sizeof(s_login_address) - 1);
	s_report_user_login_flag = 1;
}

// key对应着各个攻击的类别值 value对应攻击开关
void setNetEventReportSwitch(int index, int para)
{
	if(index<TYPESATTACK_NUM)
		netEventReportSwitch[index] = para;
} 

// 记录攻击数值，1全部记录记录, index单独记录
void value_log(int index, int value, int threshold)
{
	if(thresholdLog == 1 || thresholdLog == index)
	{
		char log[255]={0};
		sprintf(log,"AttackType:%d, Value:%d, Threshold:%d", index, value, threshold);
		log_i("networkmonitor Attack", log);
	}
}
void dpi_report_log_free(void)
{
	int ret = 0;

	if (thread_log_thd)
	{
		exit_thread_log_thd = TRUE;
		ret = pthread_join(thread_log_thd, NULL);
		thread_log_thd = 0;
		exit_thread_log_thd = FALSE;
		log_i("networkmonitor", "dpi_report_log_free pthread_join thread_log_thd\n");
	}
}
