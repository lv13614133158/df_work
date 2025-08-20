/*
 * @Author: idps menbers
 * @Date: 2021-01-19 14:52:38
 * @LastEditTime: 2021-05-02 15:32:16
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \idps_networkmonitor\includes\flow_count\flow_init.h
 */
#ifndef __FLOW_INIT__
#define __FLOW_INIT__
#include <stdio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <pcap.h>
#include <stdbool.h>
#include <pthread.h>
#include "util.h"

#ifndef IFNAMSIZ
	#define IFNAMSIZ 32
#endif
#define THREAD_MODULE
/**
 * @description: instance pcap  
 * @param 	   :
 * @return     :
 */
//#define THREAD_MODULE 
typedef struct{
    long long 		total_sent;
    long long 		total_recv;
	char            interface[IFNAMSIZ];
	int             have_hw_addr;
	unsigned char   if_hw_addr[6];  
	int             have_ip_addr;
	struct in_addr  if_ip_addr;
	struct in_addr  netmask;
	pcap_t* 		pd; 
	pthread_mutex_t mtx;
	list* 			list_flow_packet;
//#ifdef THREAD_MODULE
	pthread_t 		ip_dispatcher_thd;
//#endif
	bool            initstate;
}interface_instance;
/**
 * @description: same device netinfo  
 * @param 	   :
 * @return     :
 */
#ifndef DEVICE_ETH  
	#define DEVICE_ETH  40
#endif
typedef struct{
	 struct __info{
		struct in_addr device_net[DEVICE_ETH];
		struct in_addr netmask[DEVICE_ETH];
		char           interface[DEVICE_ETH][IFNAMSIZ];
	} netinfo;
	char count;
} interfaceinfo;
//标记当前流量上传的时间间隔
extern unsigned int flowinterval;
/**外部回调函数**/
// 初始统计网卡
void flowmoduleinit(char *watchNicDevice);
//添加待添加的网络IP分段情况
void addmoduledevice(char* _ip);
// 设置统计上报时间间隔
void setflowinterval(int interval);

#ifndef THREAD_MODULE
void api_data(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet);
#endif
#endif