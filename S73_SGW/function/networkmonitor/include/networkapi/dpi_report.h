/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 2020-11-23 20:58:58
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-05-09 19:16:09
 */
#ifndef __DPI_REPORT_H__
#define __DPI_REPORT_H__

#define TCP_CONNECT_SCAN	1
#define TCP_SYN_SCAN		2
#define TCP_FIN_SCAN		3
#define TCP_ACK_SCAN		4
#define TCP_NULL_SCAN		5
#define TCP_XMAS_SCAN		6


#define TCP_SRC_PORT_ZERO	8

#define TCP_PORT_SYN_FLOOD	9

#define TCP_ACK_FIN_DOS		10
#define TCP_ACK_RST_DOS		11
#define TCP_FIN_SYN_DOS		12
#define TCP_FIN_RST_DOS		13
#define TCP_ACK_PSH_FLOOD		14


#define TCP_SYN_FLOOD		20
#define TCP_SYN_ACK_FLOOD	21

#define TCP_LAND_ATTACK		30
#define TCP_FIN_SYN_STACK_ABNORMAL		31
#define TCP_CONNECT_ATTACK	32

#define UDP_SRC_PORT_ZERO	40
#define UDP_PORT_SCAN		41
#define UDP_PORT_FLOOD		42
#define FRAGGLE_ATTACK		43

#define ICMP_DEATH_PING   		50
#define ICMP_LARGE_PING   		51

#define ICMP_ECHO_FLOODING   	52
#define ICMP_FLOOD				53
#define ICMP_SMURF_ATTACK		54
#define ICMP_IGMP_FLOOD			55
#define ICMP_FORGE_SRC_ATTACK   56
#define ICMP_TERMINAL_EXIST_DETECT	57
#define IGMP_FLOODING			60
#define ARP_ATTACK_0				70
#define ARP_ATTACK_1				71
#define ARP_ATTACK_2				72

#define IP_PACK_WITH_OPTION			80
#define IP_PACK_WITH_TIMESTAMP		81
#define IP_PACK_WITH_RECORD_TRACE	82
#define IP_PACK_WITH_EOL			83
#define IP_PACK_WITH_SATID			84

#define NONE_SRC_IDENTIFIER		(void *)0
#define NONE_PORT_IDENTIFIER		0


extern unsigned int modeIDPS;//标记当前的模式 IDS IDPS  IPS
extern unsigned int thresholdLog;//攻击值记录

void value_log(int index, int value, int threshold);
void report_log(u8 event,s8 *s_addr,s32 port, s8 *net_info);
void startlog(void);
void dpi_report_log_free(void);
void report_user_login_log(char *address);

#endif

