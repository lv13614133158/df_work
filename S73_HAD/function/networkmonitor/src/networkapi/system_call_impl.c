/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-24 16:29:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_networkmonitor.git/src/system_call_impl.c
 */ 
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "typedef.h"
#include "dpi_report.h"
#include "system_call_impl.h"
#include "spdloglib.h"

//#define TCP_CONNECT_ATTACK_THRESHOLD   64
static FILE *_file = NULL;
static u8 readBuf[4];
static long tcpConnectAttachThreshold = 64;

void setTcpConnectAttachThreshold (long para)
{
    tcpConnectAttachThreshold = para;
}
/**
 * @name:   system_call_implthread
 * @Author: qihoo360
 * @msg:    loop 1S 
 * @param  
 * @return: 
 */
void system_call_implthread(void){
	if (!_file)
	{
		return;
	}
	memset(readBuf,0,sizeof(readBuf));
	 /*we can use popen*/
	int result = system("netstat -an | grep ESTABLISHED | wc -l > /tmp/estab");
	fflush(_file);
	fseek(_file,0,SEEK_SET); 
	s32 res = fread (readBuf, 1, sizeof(readBuf),_file);
	if(res>1)
	{
		res--;
		s32 c = 0;
		/*This function is similar to 'atoi(readBuf)'*/
		for(u8 i=0;i<res;i++)
		{
			c *= 10;
			readBuf[i]-='0';
			c += readBuf[i];
		}
		if(c>tcpConnectAttachThreshold)
		{
			value_log(TCP_CONNECT_ATTACK, c, tcpConnectAttachThreshold);

			char net_info[128] = {0};
			snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%ld", c, tcpConnectAttachThreshold);
			report_log(TCP_CONNECT_ATTACK,NONE_SRC_IDENTIFIER,NONE_PORT_IDENTIFIER, net_info);
		}
	}
} 
/**
 * @name:   system_call_initpro
 * @Author: qihoo360
 * @msg:    loop 1S 
 * @param  
 * @return: 
 */
void system_call_init(void){
	if (_file)
	{
		fclose(_file);
		_file = NULL;
	}

	_file = fopen("/tmp/estab","w+");
	if(_file == NULL)
	{
		char log[256] = {0};
		sprintf(log,"%s","open failed");
		log_i("networkmonitor", log);
		return;
	}
	
}
 
void system_call_free(void){
	if (_file)
	{
		fclose(_file);
		_file = NULL;
	}
}
