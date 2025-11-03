#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<stdio.h>
#include<string.h>
#include <time.h>
#include <pthread.h>
#include  <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "typedef.h"
#include "tcp_detection.h"
#include "dpi_report.h"
#include "common_fun.h"
#include "data_dispatcher.h"
#include "api_networkmonitor.h"
#define IP_SOURCE_MAX_SIZE			(8)  //pacp抓取数据每个端口最多可以对应ip源数目

struct tcp_body{
	u16 port_number;
	u16 count;
	struct {
		u16 syn_flag;
		u16 ack_flag;
		u16 rst_flag;
		u16 fin_flag;
		u16 urg_flag;
		u16 psh_flag;
	}flags;
	long time_stamp;
	struct {
		u32 src_ip[IP_SOURCE_MAX_SIZE];
		u16 src_ipcount[IP_SOURCE_MAX_SIZE];//add 20200509
		u16 valid_ip_conut;
	}ip_properties;
};
static pthread_mutex_t    request_tcp_lock = PTHREAD_MUTEX_INITIALIZER;
static long tcpScanConnectRetrytimesThreshold = 2; //
static long tcpDosRetryTimesThreshold = 192;  // dos攻击阈值
static long tcpPortsPerSecThreshold   = 10;   // 扫描计数阈值
static float tcpConnectOrScanWeight   = 0.75; //泛红攻击和扫描攻击占总包数比

// 以port为key，tcp_body结构体存储
static list list_tcp_packet;
#define  TCP_INIT_VALUE()\
	list_destroy(&list_tcp_packet);\
	list_init(&list_tcp_packet,free);

///just calc port number
struct tcp_portcalc{
	u32 ipaddr;
	u32 count;
} _tcp_portinfo[IP_SOURCE_MAX_SIZE] = {0};

#define  clearall(){\
	for(int clear = 0;clear < IP_SOURCE_MAX_SIZE;clear ++){\
		_tcp_portinfo[clear].ipaddr = 0;\
		_tcp_portinfo[clear].count = 0;\
	}\
}
//calc in period time,the most port ip 
void calcportnumber(struct tcp_body* node){
	int min = _tcp_portinfo[0].count,minindex = 0,valid = 0;
	for(int i = 0;i < node->ip_properties.valid_ip_conut;i ++){
		for(int j = 0;j < IP_SOURCE_MAX_SIZE;j ++){
			if(_tcp_portinfo[j].ipaddr == node->ip_properties.src_ip[i]){
				_tcp_portinfo[j].count ++;
				valid = 1;
			}
			if(min >= _tcp_portinfo[j].count){
				minindex = j;
				min = _tcp_portinfo[j].count;
			}
		}
		if(valid == 0){
			_tcp_portinfo[minindex].ipaddr = node->ip_properties.src_ip[i];
			_tcp_portinfo[minindex].count = 1;
		}
		valid = 0;
	}
}

u32 getmaxfreip(void){
	u32 _localip = _tcp_portinfo[0].ipaddr,_arrayindex = _tcp_portinfo[0].count;
	for(int i = 0;i < IP_SOURCE_MAX_SIZE;i ++){
		if(_tcp_portinfo[i].count > _arrayindex){
			_arrayindex = _tcp_portinfo[i].count;
			_localip = _tcp_portinfo[i].ipaddr;
		}
	}
	return _localip;
}
/**
 * @name:   tcpport_value_consumer
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */
void tcpport_value_consumer(void)
{
	static char l_loop = 0;
	u16 syn_flag_count = 0;
	u16 fin_flag_count = 0;
	u16 ack_flag_count = 0;
	u16 psh_flag_count = 0;
	u16 urg_flag_count = 0; 			
	u16 null_flag_count = 0;
	u16 threshold = 0;
	u16 temp = 0,i = 0;
	u32 ports_count = 0;
	u16 count_of_connect_scan = 0;

	if(l_loop ++ >= 2)
	{	
		clearall()
		l_loop = 0;
		pthread_mutex_lock(&request_tcp_lock);
		ports_count = list_size(&list_tcp_packet);
		list_elmt *cur_elmt = list_head(&list_tcp_packet);

		if(cur_elmt == NULL)
		{
			pthread_mutex_unlock(&request_tcp_lock);
			return;
		}
		
		// 泛洪攻击
		while(cur_elmt != NULL)
		{
			struct tcp_body *e = cur_elmt->data;
			/*	analysis and process dos attack below */
	 		calcportnumber(e);
			if(e->count > tcpDosRetryTimesThreshold)//may be dos attack  当前port来了多少次
			{
				threshold = (u16)(e->count*tcpConnectOrScanWeight);
			//	printf("e->count is threshold%d\n",threshold);
				{
					if((e->flags.ack_flag >= threshold) && (e->flags.fin_flag >= threshold))	//may be ack_syn flood
					{
						u8 src_bytes[20];
						memset(src_bytes,0,sizeof(src_bytes));
						u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
						memcpy(src_bytes,tmp,strlen(tmp));
						value_log(TCP_ACK_FIN_DOS, e->flags.ack_flag, threshold);

						char net_info[128] = {0};
						snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", e->flags.ack_flag, threshold);
						report_log(TCP_ACK_FIN_DOS,tmp,e->port_number, net_info);
					}
					if((e->flags.ack_flag >= threshold) && (e->flags.rst_flag >= threshold))	//may be ack_rst flood
					{
						u8 src_bytes[20];
						memset(src_bytes,0,sizeof(src_bytes));
						u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
						memcpy(src_bytes,tmp,strlen(tmp));
						value_log(TCP_ACK_RST_DOS, e->flags.ack_flag, threshold);

						char net_info[128] = {0};
						snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", e->flags.ack_flag, threshold);
						report_log(TCP_ACK_RST_DOS,tmp,e->port_number, net_info);
					}					
					if((e->flags.fin_flag >= threshold) && (e->flags.rst_flag >= threshold))	//may be fin_rst flood
					{
						u8 src_bytes[20];
						memset(src_bytes,0,sizeof(src_bytes));
						u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
						memcpy(src_bytes,tmp,strlen(tmp));
						value_log(TCP_FIN_RST_DOS, e->flags.fin_flag, threshold);

						char net_info[128] = {0};
						snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", e->flags.fin_flag, threshold);
						report_log(TCP_FIN_RST_DOS,tmp,e->port_number, net_info);
					}
					if((e->flags.ack_flag >= threshold) && (e->flags.psh_flag >= threshold))        //may be ack_psh flood 
					{
						u8 src_bytes[20];
						memset(src_bytes,0,sizeof(src_bytes));
						u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
						memcpy(src_bytes,tmp,strlen(tmp));
						value_log(TCP_ACK_PSH_FLOOD, e->flags.ack_flag, threshold);

						char net_info[128] = {0};
						snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", e->flags.ack_flag, threshold);
						report_log(TCP_ACK_PSH_FLOOD,tmp,e->port_number, net_info);
					}

					if(e->flags.syn_flag >= threshold)
					{									
						if(e->flags.fin_flag >= threshold)
						{
							u8 src_bytes[20];
							memset(src_bytes,0,sizeof(src_bytes));
							u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
							memcpy(src_bytes,tmp,strlen(tmp));
							value_log(TCP_FIN_SYN_DOS, e->flags.fin_flag, threshold);

							char net_info[128] = {0};
							snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", e->flags.fin_flag, threshold);
							report_log(TCP_FIN_SYN_DOS,tmp,e->port_number, net_info);//may be fin_syn flood
						}
						else if(e->flags.ack_flag >= threshold)
						{
							u8 src_bytes[20];
							memset(src_bytes,0,sizeof(src_bytes));
							u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
							memcpy(src_bytes,tmp,strlen(tmp));
							value_log(TCP_SYN_ACK_FLOOD, e->flags.ack_flag, threshold);

							char net_info[128] = {0};
							snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", e->flags.ack_flag, threshold);
							report_log(TCP_SYN_ACK_FLOOD,tmp,e->port_number, net_info);
						}
						else
						{
							u8 src_bytes[20];
							memset(src_bytes,0,sizeof(src_bytes));
							u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
							memcpy(src_bytes,tmp,strlen(tmp));
							value_log(TCP_SYN_FLOOD, e->flags.syn_flag, threshold);

							char net_info[128] = {0};
							snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", e->flags.syn_flag, threshold);
							report_log(TCP_SYN_FLOOD,tmp,e->port_number, net_info);
						}
					}
				}
			}
			
			// 扫描攻击
			if(ports_count > tcpPortsPerSecThreshold)	//may be scan accack			
			{
				if(e->count == 1)//20202224
				{
					if(e->flags.syn_flag ==1)//20202224
					{
						syn_flag_count++;
					}
					if(e->flags.fin_flag ==1)//20202224
					{
						fin_flag_count++;
					}
					if(e->flags.ack_flag ==1)//20202224
					{
						ack_flag_count++;
					}
					if(e->flags.psh_flag ==1)//20202224
					{
						psh_flag_count++;
					}
					if(e->flags.urg_flag ==1)//20202224
					{
						urg_flag_count++;
					}										
					if((e->flags.syn_flag|e->flags.ack_flag|e->flags.fin_flag|e->flags.rst_flag|e->flags.psh_flag|e->flags.urg_flag)==0)
					{						
						null_flag_count++;
					}
				}
				else if((e->flags.psh_flag==0)&&(e->flags.urg_flag==0)&&(e->flags.fin_flag==0)&&(e->flags.rst_flag==0)\
				&&(e->flags.ack_flag==0)&&(e->flags.syn_flag==e->count) && (e->count >= tcpScanConnectRetrytimesThreshold))	//scann more than twice times,may be tcp connect scan 
				{
					count_of_connect_scan++;
				}
			}
			cur_elmt = cur_elmt->next;
		}
 
		threshold = ports_count*tcpConnectOrScanWeight;
		#if 0
		cur_elmt = list_head(&list_tcp_packet);
		struct tcp_body *e = cur_elmt->data;
		s8 *ip_str = NONE_SRC_IDENTIFIER;
		temp = 0;
		for(i = 0;i < e->ip_properties.valid_ip_conut;i ++)
			temp = (e->ip_properties.src_ipcount[i] > temp)?(i):(temp);		
		u32 int_addr = e->ip_properties.src_ip[temp];//此处存在漏洞  攻击的同时  有正常IP连接 dst_port也一样  那么就会保存到第一个位置  发现问题的时候  传输的是第一个IP  就有可能不正确
		#endif
		u32 int_addr = getmaxfreip();
		s8 *ip_str = NONE_SRC_IDENTIFIER;

 		if(int_addr!=0)
		{
			ip_str = inet_ntoa((struct in_addr){.s_addr=int_addr});			
		}		
		char log[256] = {0};
		if(syn_flag_count>threshold)
		{	
			value_log(TCP_SYN_SCAN, syn_flag_count, threshold);

			char net_info[128] = {0};
			snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", syn_flag_count, threshold);
			report_log(TCP_SYN_SCAN,ip_str,NONE_PORT_IDENTIFIER, NULL);
		}
		if(fin_flag_count>threshold)
		{
			if(psh_flag_count>threshold && urg_flag_count>threshold){
				value_log(TCP_XMAS_SCAN, psh_flag_count, threshold);

				char net_info[128] = {0};
				snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", psh_flag_count, threshold);
				report_log(TCP_XMAS_SCAN,ip_str,NONE_PORT_IDENTIFIER, net_info);
			}
			else{
				value_log(TCP_FIN_SCAN, fin_flag_count, threshold);

				char net_info[128] = {0};
				snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", fin_flag_count, threshold);
				report_log(TCP_FIN_SCAN,ip_str,NONE_PORT_IDENTIFIER, net_info);
			}

		}
		if(ack_flag_count>threshold)  
		{
			value_log(TCP_ACK_SCAN, ack_flag_count, threshold);

			char net_info[128] = {0};
			snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", ack_flag_count, threshold);
			report_log(TCP_ACK_SCAN,ip_str,NONE_PORT_IDENTIFIER, net_info);
		}
		if(null_flag_count>threshold)  
		{
			value_log(TCP_NULL_SCAN, null_flag_count, threshold);

			char net_info[128] = {0};
			snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", null_flag_count, threshold);
			report_log(TCP_NULL_SCAN,ip_str,NONE_PORT_IDENTIFIER, net_info);
		}
		if(count_of_connect_scan > threshold)
		{
			value_log(TCP_CONNECT_SCAN, count_of_connect_scan, threshold);

			char net_info[128] = {0};
			snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%d", count_of_connect_scan, threshold);
			report_log(TCP_CONNECT_SCAN,ip_str,NONE_PORT_IDENTIFIER, net_info);
		}
		syn_flag_count = 0;
		fin_flag_count = 0;
		ack_flag_count = 0;
		psh_flag_count = 0;
		urg_flag_count = 0;
		null_flag_count = 0;
		threshold = 0;
		ports_count=0;
		count_of_connect_scan = 0;
		TCP_INIT_VALUE();	
		pthread_mutex_unlock(&request_tcp_lock);
	}
}
/**
 * @name:   tcp_scanner_init
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */
void tcp_scanner_init()
{
	pthread_mutex_lock(&request_tcp_lock);		
	TCP_INIT_VALUE();
	pthread_mutex_unlock(&request_tcp_lock);
}
/**
 * @name:   tcp_search_elmt
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */
static struct tcp_body *tcp_search_elmt(list *_list,u16 port)
{
	list_elmt *head = _list->head;
	if(head ==NULL)
		return NULL;
	list_elmt *cur = head;
	while(cur != NULL)
	{
		struct tcp_body *body =	(struct tcp_body *)cur->data;
		if(body->port_number == port)
		{
			return cur->data;
		}
		cur = cur->next;
	}
	return NULL;
}
/**
 * @name:   tcp_parser
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */
void tcp_parser(u32 src_addr,u32 dest_addr,struct tcphdr* tcp,u32 pack_length)
{
	u16 dst_port = ntohs(tcp->dest);
	u16 src_port = ntohs(tcp->source);
	u8 src_bytes[20];
	memset(src_bytes,0,sizeof(src_bytes));
	u8 *tmp = inet_ntoa((struct in_addr){.s_addr=src_addr});
	memcpy(src_bytes,tmp,strlen(tmp));
	if(src_port==0)
	{
		report_log(TCP_SRC_PORT_ZERO,tmp,dst_port, NULL);
		return;
	}
	if(src_addr == dest_addr)
	{
		report_log(TCP_LAND_ATTACK,tmp,dst_port, NULL);
		return;
	}
	if((tcp->fin==1)&&(tcp->syn==1))
	{
		report_log(TCP_FIN_SYN_STACK_ABNORMAL,tmp,dst_port, NULL);//7%
		//return;
	}
	// 在原先的tcp数据队列里找出对应的数据，以port号为key，并统计相同ip地址数量。
	long localtime = get_timestamp();
	pthread_mutex_lock(&request_tcp_lock);
	struct tcp_body *e = tcp_search_elmt(&list_tcp_packet,dst_port);
	if(e == NULL)
	{
		e = (struct tcp_body *)malloc(sizeof(struct tcp_body));		
		memset(e,0,sizeof(struct tcp_body));
		e->port_number = dst_port;
		list_ins_next(&list_tcp_packet,NULL,e);		
	}
	e->count++;
	e->time_stamp = localtime;
	//if source ip is greater than IP_SOURCE_MAX_SIZE,then it's abnormal,may be DOS attack
	if(e->ip_properties.valid_ip_conut < IP_SOURCE_MAX_SIZE)
	{
		boolean ip_found=FALSE;
		for(u16 ip_index=0;ip_index < e->ip_properties.valid_ip_conut;ip_index++)
		{
			if(e->ip_properties.src_ip[ip_index] == src_addr)
			{
				e->ip_properties.src_ipcount[ip_index] ++;//add
				ip_found = TRUE;
				break;
			}
		}
		if(ip_found == FALSE)
		{
			e->ip_properties.src_ip[e->ip_properties.valid_ip_conut] = src_addr;
			e->ip_properties.src_ipcount[e->ip_properties.valid_ip_conut] = 1;//add
			e->ip_properties.valid_ip_conut++;
		}
	}

	if(tcp->ack==1)
		e->flags.ack_flag++;
	if(tcp->rst==1)
		e->flags.rst_flag++;
	if(tcp->syn==1)
		e->flags.syn_flag++;
	if(tcp->psh==1)
		e->flags.psh_flag++;
	if(tcp->urg==1)
		e->flags.urg_flag++;
	if(tcp->fin==1)
		e->flags.fin_flag++;
	pthread_mutex_unlock(&request_tcp_lock);
}

// 设置TCP阈值
void setTcpScanConnectRetrytimesThreshold(long para)
{
	tcpScanConnectRetrytimesThreshold = para;
}
void setTcpDosRetryTimesThreshold(long para)
{
	tcpDosRetryTimesThreshold = para;
}
void setTcpPortsPerSecThreshold(long para)
{
	tcpPortsPerSecThreshold = para;
}
void setTcpConnectOrScanWeight(float para)
{
	tcpConnectOrScanWeight = para;
}


























