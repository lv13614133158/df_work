/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-24 14:44:46
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_networkmonitor.git/src/igmp_detection.c
 */ 
#include<netinet/ip.h>
#include<netinet/igmp.h>
#include<stdio.h>
#include<string.h>
#include <time.h>
#include <pthread.h>
#include  <semaphore.h>
#include <unistd.h>
#include "typedef.h"
#include "dpi_report.h"
#include "common_fun.h"
#include "igmp_detection.h"
#include "api_networkmonitor.h"

static pthread_mutex_t request_igmp_lock = PTHREAD_MUTEX_INITIALIZER;
static long igmpFloodThreshold = 512;//IGMP_FLOOD_THRESHOLD

volatile u32 igmp_pack_count=0;
/**
 * @name:   igmp_value_consumer
 * @Author: qihoo360
 * @msg:    loop 1S 
 * @param  
 * @return: 
 */
void igmp_value_consumer(void){
		if(igmp_pack_count>igmpFloodThreshold)
		{
			value_log(IGMP_FLOODING, igmp_pack_count, igmpFloodThreshold);
			report_log(IGMP_FLOODING,NONE_SRC_IDENTIFIER,NONE_PORT_IDENTIFIER);
		}
		get_date();
		igmp_pack_count=0;
}
/**
 * @name:   igmp_parser
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */ 
void igmp_parser()
{
	igmp_pack_count++;
}

// 设置IGMP阈值
void setIgmpFloodThreshold(long para)
{
	igmpFloodThreshold = para;
}



