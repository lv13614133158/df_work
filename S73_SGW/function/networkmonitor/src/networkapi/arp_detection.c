#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include "typedef.h"
#include "udp_detection.h"
#include "dpi_report.h"
#include "arp_detection.h"
#include "data_dispatcher.h"
/**
 * @name:   变量声明与定义
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
extern u8 local_net_ip_hex[4];
extern u8 local_net_mac_hex[6];
static long arpFloodThreshold = 512;//ARP_FLOOD_THRESHOLD
static long arpAttackThreshold = 6; //ARP_ATTARK_THRESHOLD

u32 arp_pack_count = 0;
u32 arp_pack_countarp1 = 0;
u32 arp_pack_countarp2 = 0;

/**
 * @name:   arp_parser_proc
 * @Author: qihoo360
 * @msg:    流程处理函数（供外部使用）
 * @param    
 * @return: 
 */
void arp_parser_proc(void){
	if(arp_pack_count>arpFloodThreshold)
	{
		value_log(ARP_ATTACK_0, arp_pack_count, arpFloodThreshold);

		char net_info[128] = {0};
		snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%ld", arp_pack_count, arpFloodThreshold);
		report_log(ARP_ATTACK_0,NONE_SRC_IDENTIFIER,NONE_PORT_IDENTIFIER, net_info);
	}
	if(arp_pack_countarp1 >= arpAttackThreshold){
		value_log(ARP_ATTACK_1, arp_pack_countarp1, arpAttackThreshold);

		char net_info[128] = {0};
		snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%ld", arp_pack_countarp1, arpAttackThreshold);
		report_log(ARP_ATTACK_1,NONE_SRC_IDENTIFIER,NONE_PORT_IDENTIFIER, net_info);
	}
	if(arp_pack_countarp2 >= arpAttackThreshold){
		value_log(ARP_ATTACK_1, arp_pack_countarp2, arpAttackThreshold);

		char net_info[128] = {0};
		snprintf(net_info, sizeof(net_info), "Value:%d, Threshold:%ld", arp_pack_countarp2, arpAttackThreshold);
		report_log(ARP_ATTACK_2,NONE_SRC_IDENTIFIER,NONE_PORT_IDENTIFIER, net_info);
	}
	arp_pack_count = 0;
	arp_pack_countarp2 = 0;
	arp_pack_countarp1 = 0;
}
/**
 * @name:   arp_parser
 * @Author: qihoo360
 * @msg:    arp数据解析函数
 * @param  
 * @return: 
 */
void arp_parser(struct arphdr *arpinput)
{
	struct arphdr_local
	{
		unsigned short int ar_hrd;		/* Format of hardware address.  */
		unsigned short int ar_pro;		/* Format of protocol address.  */
		unsigned char ar_hln;		/* Length of hardware address.  */
		unsigned char ar_pln;		/* Length of protocol address.  */
		unsigned short int ar_op;		/* ARP opcode (command).  */
		
		/* Ethernet looks like this : This bit is variable sized
			however...  */
		unsigned char __ar_sha[ETH_ALEN];	/* Sender hardware address.  */
		unsigned char __ar_sip[4];		/* Sender IP address.  */
		unsigned char __ar_tha[ETH_ALEN];	/* Target hardware address.  */
		unsigned char __ar_tip[4];		/* Target IP address.  */
	};
	struct arphdr_local *arp = (struct arphdr_local *)arpinput;
	arp_pack_count++;
	switch (arp->ar_pro)
	{
		case ARPOP_InREQUEST:/*ARPOP_InREQUEST:8*/	
		{
			if(ntohs(arp->ar_op) == ARPOP_REPLY)
			{
				if(memcmp(local_net_ip_hex,arp->__ar_sip,sizeof(local_net_ip_hex))==0)
				{
					if(memcmp(local_net_mac_hex,arp->__ar_sha,sizeof(local_net_mac_hex))!=0)
					{
						arp_pack_countarp1 ++;
					}
				}
				if(memcmp(local_net_ip_hex,arp->__ar_tip,sizeof(local_net_ip_hex))==0)
				{
					if(memcmp(local_net_mac_hex,arp->__ar_tha,sizeof(local_net_mac_hex)) !=0)
					{
						arp_pack_countarp2 ++;
					}
				}
			}		
		}			 
	}
}

// 设置arp阈值
void setArpFloodThreshold(long para)
{
	arpFloodThreshold = para;
}

void setArpAttackThreshold(long para)
{
	arpAttackThreshold = para;
}






















