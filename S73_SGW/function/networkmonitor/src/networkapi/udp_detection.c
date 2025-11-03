#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "typedef.h"
#include "udp_detection.h"
#include "dpi_report.h"
#include "common_fun.h"
#include "data_dispatcher.h"
#include "api_networkmonitor.h"

#define SWAP16BIT(num) ((num>>8)&0xFF + ((num&0xFF)<<8)) // 16位高低位交换
#define DNS_DATA_MAX_SIZE  (0x40)
#define IP_DATA_MAX_SIZE   (0x40*5)
	
extern s8 local_net_ip[32];

#define IP_SOURCE_MAX_SIZE			(8)

struct udp_body{
	u16 port_number;
	u16 count;
	long time_stamp;
	struct {
		u32 src_ip[IP_SOURCE_MAX_SIZE];
		u16 valid_ip_conut;
	}ip_properties;
};
static pthread_mutex_t request_udp_lock = PTHREAD_MUTEX_INITIALIZER;
static long udpDosRetrytimesThreshold     = 75;
static long udpPortsPerSecThreshold		  = 16;
static long fraggleAttemptPerSecThreshold = 16;

static list list_udp_packet;
#define UDP_INIT_VALUE()\
	list_destroy(&list_udp_packet);\
	list_init(&list_udp_packet,free);
static list *whiteDNS = NULL;

static struct udp_body *udp_search_elmt(list *_list,u16 port)
{
	list_elmt *head = _list->head;
	if(head ==NULL)
		return NULL;

	list_elmt *cur = head;
	while(cur != NULL)
	{
		struct udp_body *body =	(struct udp_body *)cur->data;
		if(body->port_number == port)
		{
			return cur->data;
		}
		cur = cur->next;
	}
	return NULL;
}

static boolean src_port_zero_flag = FALSE;
static u32 fraggle_attack_count = 0;
/**
 * @name:   udp_scanner_init
 * @Author: qihoo360
 * @msg:    loop 1S 
 * @param  
 * @return: 
 */
void udp_scanner_init(void){
	pthread_mutex_lock(&request_udp_lock);
	UDP_INIT_VALUE();
	fraggle_attack_count = 0;
	pthread_mutex_unlock(&request_udp_lock);
}
/**
 * @name:   udpport_value_con
 * @Author: qihoo360
 * @msg:    loop 1S 
 * @param  
 * @return: 
 */
void udpport_value_consumer(void){
	u32 ports_count = 0;
	ports_count = list_size(&list_udp_packet);
	list_elmt *cur_elmt = list_head(&list_udp_packet);
	if(cur_elmt == NULL)
		return;
	
	s8 *ip_str = NONE_SRC_IDENTIFIER;
	struct udp_body *e = cur_elmt->data;
	u32 int_addr = e->ip_properties.src_ip[0];		
	if(int_addr!=0)
	{
		ip_str = inet_ntoa((struct in_addr){.s_addr=int_addr});			
	}		
	
	if(ports_count > udpPortsPerSecThreshold)
	{
		value_log(UDP_PORT_SCAN, ports_count, udpPortsPerSecThreshold);

		char net_info[128] = {0};
		snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%ld", ports_count, udpPortsPerSecThreshold);
		report_log(UDP_PORT_SCAN,ip_str,NONE_PORT_IDENTIFIER, net_info);
	}
	if(fraggle_attack_count>fraggleAttemptPerSecThreshold)
	{
		value_log(FRAGGLE_ATTACK, fraggle_attack_count, fraggleAttemptPerSecThreshold);

		char net_info[128] = {0};
		snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%ld", fraggle_attack_count, fraggleAttemptPerSecThreshold);
		report_log(FRAGGLE_ATTACK,ip_str,NONE_PORT_IDENTIFIER, net_info);
	}
	if(src_port_zero_flag==TRUE)
	{
		report_log(UDP_SRC_PORT_ZERO,ip_str,NONE_PORT_IDENTIFIER, NULL);
	}
	pthread_mutex_lock(&request_udp_lock);
	while(cur_elmt != NULL)
	{
		struct udp_body *e = cur_elmt->data;
		u8 src_bytes[20];
		memset(src_bytes,0,sizeof(src_bytes));
		u8 *tmp = inet_ntoa((struct in_addr){.s_addr=e->ip_properties.src_ip[0]});
		 
		memcpy(src_bytes,tmp,strlen(tmp));
		//	analysis and process dos attack below 
		if(e->count > udpDosRetrytimesThreshold)		//may be dos attack
		{
			value_log(UDP_PORT_FLOOD, e->count, udpDosRetrytimesThreshold);

			char net_info[128] = {0};
			snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%ld", e->count, udpDosRetrytimesThreshold);
			report_log(UDP_PORT_FLOOD,tmp,NONE_PORT_IDENTIFIER, net_info);
		}
		cur_elmt = cur_elmt->next;
	}
	UDP_INIT_VALUE();
	src_port_zero_flag = FALSE;
	fraggle_attack_count = 0;	
	pthread_mutex_unlock(&request_udp_lock);
}
struct dns_hdr{
	u16 transaction_id;
	u16 flags;
	u16 questions;
	u16 answer_rrs;
	u16 auth_rrs;
	u16 additional_rrs;
	u8  payload[];
};
/**
 * @name:   only check one dns
 * @Author: qihoo360
 * @msg:     
 * @param  
 * @return: 
 */
static void extract_name(struct dns_hdr *dnshdr)
{
	#define 	MAX_DNS_DATA_SIZE 	0x20
	static u8 dns_data_tmp_buff[MAX_DNS_DATA_SIZE];
	memset(dns_data_tmp_buff,0,MAX_DNS_DATA_SIZE);
	u16 offset0=0;
	u16 offset1=0;
	u16 qus = ntohs(dnshdr->questions);

	if(qus>0)
	{
		u8 len = 0;
		while((len=dnshdr->payload[offset1++]) != 0)
		{
			strncpy((u8 *)(dns_data_tmp_buff+offset0),&dnshdr->payload[offset1],len);
			offset0+=len;
			offset1+=len;
			*(u8 *)(dns_data_tmp_buff + offset0++)='.';

			if(offset0>MAX_DNS_DATA_SIZE){
				return;
			}
		}
		*(u8 *)(dns_data_tmp_buff + offset0-1)=0;

		if(strstr(dns_data_tmp_buff,"www.")) 
		{
			on_onDnsInquireEvent_callback(dns_data_tmp_buff);//申请
		}
		else if(dns_data_tmp_buff[0]>='0' && dns_data_tmp_buff[0]<='9')
		{
			// ip反转 1.2.3.4->4.3.2.1
			unsigned char ip_data_tmp_buff[16]={0}, ip_pos[4] ={0}, ip_len=0;
			int i=0, j=0;
			for(i=0, j=0; i<16 && j<4; i++)
			{
				if(dns_data_tmp_buff[i]=='.')
				{
					ip_pos[j++] = i;
				}
			}
			if (j != 4)
			{
				return;
			}
			ip_len += 0;
			memcpy(ip_data_tmp_buff+ip_len, dns_data_tmp_buff+ip_pos[2]+1, ip_pos[3]-ip_pos[2]);
			ip_len += ip_pos[3]-ip_pos[2];
			memcpy(ip_data_tmp_buff+ip_len, dns_data_tmp_buff+ip_pos[1]+1, ip_pos[2]-ip_pos[1]);
			ip_len += ip_pos[2]-ip_pos[1];
			memcpy(ip_data_tmp_buff+ip_len, dns_data_tmp_buff+ip_pos[0]+1, ip_pos[1]-ip_pos[0]);
			ip_len += ip_pos[1]-ip_pos[0];
			memcpy(ip_data_tmp_buff+ip_len, dns_data_tmp_buff, ip_pos[0]+1);
			on_onDnsResponseEvent_callback("",ip_data_tmp_buff);//响应
		}
	}
}

void udp_parser(u32 src_addr,u32 dest_addr,struct udphdr* udp,u32 pack_length)
{
	u16 src_port = ntohs(udp->source);
	u16 dst_port = ntohs(udp->dest);
	
	if(src_port==0)
	{
		src_port_zero_flag = TRUE;
		return;
	}
	if((dst_port==7)||(dst_port==19))
	{
		fraggle_attack_count++;
	}
	long localtime = get_timestamp();
	pthread_mutex_lock(&request_udp_lock);

	struct udp_body *e = udp_search_elmt(&list_udp_packet,dst_port);
	if(e == NULL)
	{
		e = (struct udp_body *)malloc(sizeof(struct udp_body));		
		memset(e,0,sizeof(struct udp_body));
		e->port_number = dst_port;
		list_ins_next(&list_udp_packet,NULL,e);		
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
				ip_found = TRUE;
				break;
			}
		}
		if(ip_found == FALSE)
		{
			e->ip_properties.src_ip[e->ip_properties.valid_ip_conut] = src_addr;
			e->ip_properties.valid_ip_conut++;
		}
	}
	pthread_mutex_unlock(&request_udp_lock);
}

// 设置UDP阈值
void setUdpDosRetrytimesThreshold(long para)
{	
	udpDosRetrytimesThreshold = para;
}
void setudpPortsPerSecThreshold(long para)
{	
	udpPortsPerSecThreshold = para;
}
void setfraggleAttemptPerSecThreshold(long para)
{	
	fraggleAttemptPerSecThreshold = para;
}

int dns_name_len(unsigned char* input)
{
	int len =0;
	while( *input )
	{
		if(*input == 0xc0)
			return len+2;
		len++;
		input++;
	}
	return len+1;
}

// 1、输入字符0结束,输出字符串长度slen防止内存越界。
// 2、域名字符串 3'www'5'baidu'3'com'  IP字符串3'150'2'38'3'181'3'220'，IP字符串是反了
// 3、遇见c0xx代表重复拼接前面的，xx代表从dns数据偏移地址
// 4、返回值等于输出字符串长度,询问没有拼接，返回值也等于输入字符串长度
int dnsConver_name(unsigned char* input, unsigned char* output, int slen, unsigned char* origin_addr)
{
	int len =0, out_len =0, iplen =0, ipcnt =0;
	unsigned char* pi=input+1, *po=output;
	
	if(*input == 0xc0)
	{
		if(origin_addr)
			return dnsConver_name(origin_addr+*(input+1), po, slen, origin_addr);
		else
			goto EXIT;
	}

	while( *pi )
	{	
		if(*pi == 0xc0) //字符串拼接，只有回答区有拼接
		{
			if(origin_addr && len<slen)
			{
				*po++ ='.'; 
				len++;
				len +=dnsConver_name(origin_addr+*(pi+1), po, slen-(po-output), origin_addr);
				return len;
			}
			else
			{
				goto EXIT;
			}
		}
		if(len < slen)
		{
			if(*pi < 32) //控制字符，为了防止复杂出错,这里就默认每段不大于于32个字符
			{
				*po++ = '.';
				len++;
			}
			else
			{
				*po++ = *pi;
				len++;
			}
		}
		pi++;
	}

EXIT:
	*po =' '; //去掉最后0,为了拼接，  
	len++;
	return len;
}

// 转换vip6, out长度大于30,
int dnsConver_vip6(unsigned char* input, unsigned char* output)
{
	int len =0;
	unsigned char *pi =input, *po =output;
	for(int i=0; i<16; i++, pi++)
	{
		unsigned char temp =*pi, tbit[2] ={0};
		tbit[0] = (temp/16)%16;
		tbit[1] = temp%16;

		for(int j=0; j<2; j++)
		{
			if(tbit[j]>=10)
				*po++ = tbit[j]-10+'a';
			else
				*po++ = tbit[j]+'0';
			len++;
		}
		if(i!=15 && i%2)
		{
			*po++ = ':';
			len++;
		}
	}	
	*po++ =' ';
	len++;

	return len;
}

// 转换vip4
int dnsConver_vip4(unsigned char* input, unsigned char* output)
{
	int len =0;
	unsigned char *pi = input, *po =output;
	for(int i=0; i<4; i++, pi++)
	{
		unsigned char temp =*pi, tbit[3] ={0};
		tbit[0] = (temp/100)%100;
		tbit[1] = (temp/10)%10;
		tbit[2] = temp%10;

		for(int j=0; j<3; j++)
		{
			int hset = 0; //高位非全0
			if(tbit[j]!=0 )
			{
				*po++ = tbit[j]+'0';
				hset = 1;
				len++;
			}
			else if(hset)
			{
				*po++ = tbit[j]+'0';
				len++;
			}
		}

		if(i != 3){
			*po++ = '.';
			len++;
		}
		
	}	
	*po++ =' ';
	len++;
	return len;
}

// 字符串 3'www'5'baidu'3'com'  
// 字符串3'150'2'38'3'181'3'220'，依此翻过来
int dnsConver_Filp(unsigned char* input, unsigned char* output, int out_len, int option)
{
	int index =0, len =0, isNum = 0;
	unsigned char ip_len[5] ={0}, ip_content[5][20] ={0}; // ip/dns分段和长度
	unsigned char *pi=input, *po=output;
	while(len<out_len && index<5 && pi[len]>0 && pi[len]<32)
	{
		// 长度内容解析
		ip_len[index] = pi[len];
		memcpy(ip_content[index], pi+len+1, ip_len[index]);
		len += (ip_len[index++]+1);

		// 非数字判断
		for(int i=0; i<ip_len[index-1]; i++)
		{
			//printf("%c\n", ip_content[index-1][i]);
			if(ip_content[index-1][i]<'0' || ip_content[index-1][i]>'9')
				isNum = 1;
		}

		if(!isNum && index==4)
			break;
	}

	if(isNum)
	{
		for(int i=0, pos=0; i<index; i++)
		{
			memcpy(po+pos, ip_content[i], ip_len[i]);
			pos +=ip_len[i];
			po[pos++] = '.';
		}
	}
	else
	{
		for(int i=0, pos=0; i<4; i++)
		{
			memcpy(po+pos, ip_content[3-i], ip_len[3-i]);
			pos +=ip_len[3-i];
			po[pos++] = '.';
		}
	}
	po[len-1] = 0;

	return len;
}

void dns_print(struct dns_hdr *dnshdr, int len)
{
	printf(" DNS ID:%x, Flags:%x\n \
	 questions:%4d, answer_rrs:%4d, auth_rrs:%4d, additional_rrs:%4d\n \
	 content:%s, len:%d\n",
		dnshdr->transaction_id, dnshdr->flags,
		SWAP16BIT(dnshdr->questions), SWAP16BIT(dnshdr->answer_rrs),
		SWAP16BIT(dnshdr->auth_rrs), SWAP16BIT(dnshdr->additional_rrs),
		dnshdr->payload,len);
	for(int i=0; i<len; i++){
		if((len-i)%16 == 0)printf("\n");
		printf("%2x ",dnshdr->payload[i]);
	}printf("\n");
}

void dns_parser(char* src_addr,char* dest_addr,struct udphdr* udp, int action)
{
	u16 dst_port = ntohs(udp->dest);
	u16 src_port = ntohs(udp->source);
	
	if(dst_port == 53 || src_port == 53)
	{
		struct dns_hdr *dnshdr = (struct dns_hdr *)((u8 *)udp + sizeof(struct udphdr));
		int payload_total_len = SWAP16BIT(udp->len)-8-12;
		char dns_data_tmp_buff[DNS_DATA_MAX_SIZE], ip_data_tmp_buff[IP_DATA_MAX_SIZE];
		int dns_data_tmp_buff_len = 0;

		memset(dns_data_tmp_buff, 0, DNS_DATA_MAX_SIZE);
		memset(ip_data_tmp_buff,  0, IP_DATA_MAX_SIZE);	
		//dns_print(dnshdr, payload_total_len);

		// 询问区
		int payload_len_cnt = dns_name_len(dnshdr->payload)+4;
		dnsConver_name(dnshdr->payload, dns_data_tmp_buff, DNS_DATA_MAX_SIZE, NULL);

		dns_data_tmp_buff[DNS_DATA_MAX_SIZE - 1] = '\0';
		dns_data_tmp_buff_len = strlen(dns_data_tmp_buff);
		if (dns_data_tmp_buff[dns_data_tmp_buff_len - 1] == ' ')
		{
			dns_data_tmp_buff[dns_data_tmp_buff_len - 1] = '\0';
		}

		if (strstr(dns_data_tmp_buff, "in-addr.arpa"))
		{
			return;
		}

		/*Whitelists are used for filtering*/
		if (whiteDNS)
		{
			list_elmt *element = whiteDNS->head;
			while (element)
			{
				if(strcmp(element->data, dns_data_tmp_buff) == 0)
				{
					return;
				}
				element = element->next;
			}
		}

		if(action)
		{
			//printf("询问:%s,len:%d\n\n\n",dns_data_tmp_buff, payload_len_cnt);
			on_onDnsInquireEvent_callback(dns_data_tmp_buff);//申请
		}
		else
		{
			// 回答区
			int ip_tmp_len_cnt  = 0;
			while( (payload_total_len-payload_len_cnt) > 10 )
			{
				//type、length
				int type   = dnshdr->payload[payload_len_cnt+3]; 
				int length = dnshdr->payload[payload_len_cnt+10]*16 + dnshdr->payload[payload_len_cnt+11];
				int ip_index = 0;

				if(length+payload_len_cnt>payload_total_len  || ip_tmp_len_cnt+30>IP_DATA_MAX_SIZE)
				{
					break;
				}
		
				payload_len_cnt += 12;
				switch (type)
				{
				case 1: /* ipv4 */
					ip_index = dnsConver_vip4(dnshdr->payload+payload_len_cnt, ip_data_tmp_buff+ip_tmp_len_cnt);
					ip_tmp_len_cnt += ip_index;
					break;
				case 5: /* check name */
					ip_index = dnsConver_name(dnshdr->payload+payload_len_cnt, ip_data_tmp_buff+ip_tmp_len_cnt, IP_DATA_MAX_SIZE-ip_tmp_len_cnt, dnshdr->payload-12);
					ip_tmp_len_cnt += ip_index;
					break;
				case 28: /* ipv6 */ 
					ip_index = dnsConver_vip6(dnshdr->payload+payload_len_cnt, ip_data_tmp_buff+ip_tmp_len_cnt);
					ip_tmp_len_cnt += ip_index;
					break;
				case 6: 
					ip_index = dnsConver_name(dnshdr->payload+payload_len_cnt, ip_data_tmp_buff+ip_tmp_len_cnt, IP_DATA_MAX_SIZE-ip_tmp_len_cnt, dnshdr->payload-12);
					ip_tmp_len_cnt += ip_index;
					break;
				default:
					printf("type:%d 暂时不支持的DNS解析类型\n", type);
					break;
				}
				payload_len_cnt += length;
			}
			if( ip_data_tmp_buff[0] <= 32)
				memcpy(ip_data_tmp_buff, "Not processed", strlen("Not processed\n"));
			//printf("回复:%s\n%s,len:%d\n\n\n",dns_data_tmp_buff,ip_data_tmp_buff, payload_len_cnt);
			on_onDnsResponseEvent_callback(dns_data_tmp_buff, ip_data_tmp_buff);//响应
		}
	}
}
void DNSWhiteCheckInit(list *listName)
{
	whiteDNS = listName;
}
