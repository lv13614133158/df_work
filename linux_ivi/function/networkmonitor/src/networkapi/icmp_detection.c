#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<stdio.h>
#include<string.h>
#include <time.h>
#include <pthread.h>
#include  <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "typedef.h"
#include "dpi_report.h"
#include "icmp_detection.h"
#include "data_dispatcher.h"
#include "api_networkmonitor.h"
#include "spdloglib.h"

/**
 * @name:   变量声明与定义
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
extern s8 local_net_ip[32];

#define	MAX_IP_SRC_ADDR_SIZE			(16)
struct icmp_body{
	u32 port_number;
	u32 echo_count;
	u32 echoreply_count;
	u16 src_addr_count;
	u32 s_addr[MAX_IP_SRC_ADDR_SIZE];
};
static long icmpFloodThreshold	= 400; // ICMP_FLOOD_THRESHOLD 
static long deathPingSizeThreshold = 65502;// DEATH_PING_SIZE_THRESHOLD
static long largePingSizeThreshold = 1500; // LARGE_PING_SIZE_THRESHOLD

struct icmp_body _icmp_body;
static pthread_mutex_t    request_icmp_lock = PTHREAD_MUTEX_INITIALIZER;
static list list_icmp_packet;
boolean icmp_smurf_attack = FALSE;
/**
 * @name:   ins_src_address
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */
static void ins_src_address(u32 addr)
{
	if(_icmp_body.src_addr_count == MAX_IP_SRC_ADDR_SIZE)
		return;
		
	for(u16 i=0;i<_icmp_body.src_addr_count;i++)
	{
		if(_icmp_body.s_addr[i]==addr)
		{
			return;
		}
	}	
	_icmp_body.s_addr[_icmp_body.src_addr_count++] = addr;
}
/**
 * @name:   icmp_value_con
 * @Author: qihoo360
 * @msg:    loop 1S
 * @param  
 * @return: 
 */
void icmp_scan_init(void){
	pthread_mutex_lock(&request_icmp_lock);
	memset(&_icmp_body,0,sizeof(struct icmp_body));
	pthread_mutex_unlock(&request_icmp_lock);
}
/**
 * @name:   icmp_value_consumer
 * @Author: qihoo360
 * @msg:    loop 1S
 * @param  
 * @return: 
 */
void icmp_value_consumer(void){
	pthread_mutex_lock(&request_icmp_lock);
	if(_icmp_body.echo_count > icmpFloodThreshold)
	{
		u8 src_bytes[20];
		memset(src_bytes,0,sizeof(src_bytes));
		u8 *tmp = inet_ntoa((struct in_addr){.s_addr=_icmp_body.s_addr[0]});
		memcpy(src_bytes,tmp,strlen(tmp));
		value_log(ICMP_ECHO_FLOODING, _icmp_body.echo_count, icmpFloodThreshold);	
		report_log(ICMP_ECHO_FLOODING,tmp,_icmp_body.port_number);
	}

	if(_icmp_body.src_addr_count==MAX_IP_SRC_ADDR_SIZE)
	{
		u8 src_bytes[20];
		memset(src_bytes,0,sizeof(src_bytes));
		u8 *tmp = inet_ntoa((struct in_addr){.s_addr=_icmp_body.s_addr[0]});
		memcpy(src_bytes,tmp,strlen(tmp));	
		report_log(ICMP_FORGE_SRC_ATTACK,tmp,_icmp_body.port_number);
	}

	if(icmp_smurf_attack == TRUE)
	{
		u8 src_bytes[20];
		memset(src_bytes,0,sizeof(src_bytes));
		u8 *tmp = inet_ntoa((struct in_addr){.s_addr=_icmp_body.s_addr[0]});
		memcpy(src_bytes,tmp,strlen(tmp));
		report_log(ICMP_SMURF_ATTACK,tmp,_icmp_body.port_number);
		icmp_smurf_attack = FALSE;
	}		
	pthread_mutex_unlock(&request_icmp_lock);
	icmp_scan_init();
}
/**
 * @name:   icmp_parser
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */
static u8 type_status = 0xFF;
void icmp_parser(struct iphdr *ip,struct icmphdr* icmp,u32 pack_length)
{
	u16 ip_fragoff = ntohs(ip->frag_off);
	u16 ip_pack_length =  ntohs(ip->tot_len);

	u16 frag_off = ip_fragoff & 0x1FFF;

	u16 is_not_zero = (ip_fragoff&0x8000==1);
	if(is_not_zero)
	{
		char log[256] = {0};
		sprintf(log,"%s","stack ip bit error");
		log_i("networkmonitor", log);
	}
	u16 has_more_frag = ip_fragoff&0x2000;
	u16 is_no_frag = ip_fragoff&0x4000;
	u16 data_length = ip_pack_length - sizeof(struct iphdr);
	if(frag_off == 0)//first frament
	{
		data_length -= sizeof(struct icmphdr);
		switch(icmp->type)
		{
			case ICMP_ECHO:
				type_status = ICMP_ECHO;
				_icmp_body.echo_count++;
				ins_src_address(ip->saddr);
				if((0 == memcmp(local_net_ip, inet_ntoa((struct in_addr){.s_addr=ip->saddr}), strlen(local_net_ip) + 1)))
				{
					icmp_smurf_attack=TRUE;
				}
				break;
			case ICMP_ECHOREPLY:
				type_status = ICMP_ECHOREPLY;
				_icmp_body.echoreply_count++;
			break;
			default:
				type_status = icmp->type;
				break;
		}
	}
	
	if((type_status == ICMP_ECHO)&&(has_more_frag==0))	//is also last fragment
	{
		u32 frag_off_bytes = frag_off*8;
		u32 tot_len = frag_off_bytes+(u32)data_length;
		
		if(tot_len>deathPingSizeThreshold)
		{
			u8 src_bytes[20];
			memset(src_bytes,0,sizeof(src_bytes));
			u8 *tmp = inet_ntoa((struct in_addr){.s_addr=ip->saddr});
			memcpy(src_bytes,tmp,strlen(tmp));	
			value_log(ICMP_DEATH_PING, tot_len, deathPingSizeThreshold);
			report_log(ICMP_DEATH_PING,tmp,NONE_PORT_IDENTIFIER);
		}
		else if(tot_len>largePingSizeThreshold)
		{	
			u8 src_bytes[20];
			memset(src_bytes,0,sizeof(src_bytes));
			u8 *tmp = inet_ntoa((struct in_addr){.s_addr=ip->saddr});
			memcpy(src_bytes,tmp,strlen(tmp));	
			value_log(ICMP_LARGE_PING, tot_len, largePingSizeThreshold);
			report_log(ICMP_LARGE_PING,tmp,NONE_PORT_IDENTIFIER);
		} 
		else
		{
			u8 src_bytes[20];
			memset(src_bytes,0,sizeof(src_bytes));
			u8 *tmp = inet_ntoa((struct in_addr){.s_addr=ip->saddr});
			memcpy(src_bytes,tmp,strlen(tmp));
			//简单ping 不锁定
			report_log(ICMP_TERMINAL_EXIST_DETECT,tmp,NONE_PORT_IDENTIFIER);
		}
	}	
}

// 设置icmp阈值
void setIcmpFloodThreshold(long para)
{
	icmpFloodThreshold = para;
}
void setDeathPingSizeThreshold(long para)
{
	deathPingSizeThreshold = para;
}
void setLargePingSizeThreshold(long para)
{
	largePingSizeThreshold = para;
}
