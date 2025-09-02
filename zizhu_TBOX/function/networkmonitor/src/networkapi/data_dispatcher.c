#include <net/ethernet.h>
#include <stdio.h>
#include <string.h>
#include <pcap/pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <linux/igmp.h>
#include "api_networkmonitor.h"
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <pcap/pcap.h>
#include <stdbool.h>
#include "typedef.h"
#include "tcp_detection.h"
#include "udp_detection.h"
#include "icmp_detection.h"
#include "igmp_detection.h"
#include "arp_detection.h"
#include "system_call_impl.h"
#include "dpi_report.h"
#include "data_dispatcher.h"
#include "pid_detection.h"
#include "cJSON.h"
#include "spdloglib.h"

#define	IPV4_VERSION	(4)
#define	IPV6_VERSION	(6)
#define IF_INTERFACE_MAX_SIZE 			(65535)//28800000//65535
#define	IF_INTERFACE_NAME_MAX_SIZE 		(0x40)
s8 local_net_ip[32];
u8 local_net_ip_hex[4];
u8 local_net_mac_hex[6];
static pthread_t ip_data_dispatcher_thd = 0;
static pcap_t *pcap_t_handle = NULL;
static pcap_dumper_t *pcap_dumper = NULL;
static s8 *sniffer_path = NULL;
static pthread_mutex_t network_lock = PTHREAD_MUTEX_INITIALIZER;	
static boolean pcap_init_flag = FALSE;
static list *whiteIpName = NULL;
static pthread_t thread_parser_thd = 0;
static boolean exit_thread_parser_thd = FALSE;
static int s_net_connect_report_interval = 30;

typedef struct networkNode{
	unsigned int dstip;
	char ipFlag;
	char tcpFlag;
	char udpFlag;
	char time_left;
    struct networkNode *next;
}networkNode_t;

void ipWhiteCheckInit(list *listName)
{
    whiteIpName = listName;
}

void updateNetConnectReportInterval(int interval)
{
	if (interval <= 0) {
		interval = 30;
	}
	
	s_net_connect_report_interval = interval;
}

// 网络抓包数据保存文件里
boolean set_store_path(s8 *path)
{
	if(sniffer_path != NULL)
		free(sniffer_path);
	u16 len = strnlen(path,256)+1;
	sniffer_path = malloc(len);
	memset(sniffer_path,0,len);
	memcpy(sniffer_path,path,strnlen(path,256));	
}

void sniffer_start()
{
	pcap_dumper = pcap_dump_open(pcap_t_handle,sniffer_path);
}

void sniffer_stop()
{
	if(pcap_dumper)
		pcap_dump_close(pcap_dumper);
	
	pcap_dumper = NULL;
}

// ip字符串转换为十进制数值
static void transfer_to_dex() 
{
	u16 string_len = strnlen(local_net_ip,sizeof(local_net_ip));
	u8 hex_index = 0;
	for(u16 i=0;i<string_len;i++)
	{
		u8 tmp = local_net_ip[i];
		if(tmp =='.')
		{
			hex_index++;
			continue;
		}
		local_net_ip_hex[hex_index] *= 10;
		tmp -= '0';
		local_net_ip_hex[hex_index] += tmp;		
	}
}

// 1、获取固定网卡mac地址，存入本地
static void get_local_mac(char *if_name)
{
	struct ifreq m_ifreq;
	int sock = 0;
	int i = 0;

	sock = socket(AF_INET,SOCK_STREAM,0);
	strcpy(m_ifreq.ifr_name,if_name);
	
	ioctl(sock,SIOCGIFHWADDR,&m_ifreq);

	for(i = 0; i < 6; i++){
		local_net_mac_hex[i] = m_ifreq.ifr_hwaddr.sa_data[i];
	}

	close(sock);
}

char *get_monitor_mac(void)
{
	return (char *)local_net_mac_hex;
}

// 2、获取固定地址ip地址，存入本地
static void get_local_ip(char *if_name)
{
	int ret = 0;
	struct ifaddrs *addr = NULL;
	struct ifaddrs *temp_addr = NULL;
	ret = getifaddrs(&addr);
	if (ret == 0) {
		temp_addr = addr;
		while(temp_addr != NULL) 
		{	
			if(temp_addr->ifa_addr == 0)
			{
				temp_addr = temp_addr->ifa_next;
				continue;
			}
			if(temp_addr->ifa_addr->sa_family == AF_INET) 
			{
				if(strcmp(temp_addr->ifa_name, if_name)  == 0) 
				{
					s8 *tmp = inet_ntoa((struct in_addr){.s_addr=((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr.s_addr});
					memcpy(local_net_ip, tmp,strnlen(tmp,sizeof(local_net_ip)));
					printf("\nlocal ip = %s\n\n",local_net_ip);
					transfer_to_dex();
				}
			}
			temp_addr = temp_addr->ifa_next;
		}
	}
	freeifaddrs(addr);
}


// 3、获取网卡mtu，mtu为一个网卡数据包最大限制
int get_mtu(const char *nic) {
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, nic, IFNAMSIZ-1);   // eth0
    if (ioctl(fd, SIOCGIFMTU, &ifr)) {
		close(fd);
        return -1;
    }
    close(fd);
    return ifr.ifr_mtu;
}

// 4、获取网卡状态
char *net_detect(char* net_name)
{
        int skfd = 0;
        struct ifreq ifr;
        skfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(skfd < 0) {
                return NULL;
        }
        strcpy(ifr.ifr_name, net_name);
        if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 ) {
                close(skfd);
                return NULL;
        }
		close(skfd);
        if(ifr.ifr_flags & IFF_RUNNING) 
                return "UP";
        else 
                return "DOWN";
}

// 5、获取固定网卡IP地址，并返回
char* get_local_ipfornet(char *if_name)
{
	int ret = 0;
	char* tmp = NULL;
	struct ifaddrs *addr = NULL;
	struct ifaddrs *temp_addr = NULL;
	ret = getifaddrs(&addr);
	if (ret == 0) {
		temp_addr = addr;
		while(temp_addr != NULL) 
		{	
			if(temp_addr->ifa_addr == 0)
			{
				temp_addr = temp_addr->ifa_next;
				continue;
			}
			if(strcmp(temp_addr->ifa_name, if_name)  == 0) {
				if(temp_addr->ifa_addr->sa_family == AF_INET) 
				{
					tmp = inet_ntoa((struct in_addr){.s_addr=((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr.s_addr});
					break;
				}
			}
			temp_addr = temp_addr->ifa_next;
		}
	}
	freeifaddrs(addr);
	return tmp;
}

// 6、获取主机网卡的 ip mac 状态等
char* getnetinfoforpcap()
{
	pcap_if_t *alldev;
	pcap_if_t *p;
	char count = 0;
	char* s = NULL;
	cJSON *root,*pobj[32]; 
	char error[100] = {0};
	root = cJSON_CreateArray(); 
	if(root == NULL){
		char log[256] = {0};
		log_i("networkmonitor", "cJSON_CreateArray NULL");
		return NULL;}
	if(pcap_findalldevs(&alldev,error)==-1)
	{
		char log[256] = {0};
		sprintf(log,"pcap_findalldevs NULL %s",error);
		log_v("networkmonitor", log);
		return NULL;
	}
	for(p=alldev;p;p=p->next)
	{
		if(get_mtu(p->name) != -1){
			cJSON_AddItemToArray(root,pobj[count] = cJSON_CreateObject());
			cJSON_AddStringToObject(pobj[count],"eth_name",p->name);
			cJSON_AddStringToObject(pobj[count],"eth_ipaddr",(get_local_ipfornet(p->name) == NULL)?("invalid"):(get_local_ipfornet(p->name)));
			cJSON_AddStringToObject(pobj[count],"eth_status",net_detect(p->name));
			cJSON_AddNumberToObject(pobj[count],"eth_mtu",get_mtu(p->name));
			count ++;
		}
	}
	pcap_freealldevs(alldev);
	s = cJSON_PrintUnformatted(root);
	if(root)
		cJSON_Delete(root);
	return s;
}

// 7、获取网卡流量统计数据，并返回
char* getnettxrx(){
	struct net{
		char name[128];
		unsigned long long rx_bytes;
		unsigned long long tx_bytes;
	};
	enum if_item		
	{
		RX_BYTES = 0,
		RX_PACKETS,
		RX_ERRS,
		RX_DROP,
		RX_FIFO,
		RX_FRAME,
		RX_COMPRESSED,
		RX_MULTICAST,
		TX_BYTES,
		TX_PACKETS,
		TX_ERRS,
		TX_DROP,
		TX_FIFO,
		TX_COLLS,
		TX_CARRIER,
		TX_COMPRESSED,
		IF_ITEM_MAX
	};
	char* s = NULL;
	cJSON *root,*pobj[32]; 
	struct net Adapter[32]={0};
	int nCount=0;
	FILE* fp = fopen("/proc/net/dev", "r");//打开系统文件查看网卡接口
	if(!fp)
	{
		perror("fopen /proc/net/dev");
		return NULL;
	}
	char szline[1024] = {0};
	char* result = fgets(szline, sizeof(szline), fp);//跳过前面两行
	result = fgets(szline, sizeof(szline), fp);
	memset(szline, 0, sizeof(szline));
	root = cJSON_CreateArray(); 
	if(root == NULL){
		char log[256] = {0};
		sprintf(log,"%s""cJSON_CreateArray NULL");
		log_i("networkmonitor", log);
		return NULL;}
	//跳过前两行，后面一行代表一个网卡的信息，循环读取每个网卡的信息
	while(fgets(szline, sizeof(szline), fp) != NULL)
	{
		int pnamestart = 0;
		for(int i = 0;i < sizeof(szline);i++){
			if(szline[i] != ' '){
				pnamestart = i;
				break;
			}
		}
		int pnameend = 0;
		for(int i = 0;i < sizeof(szline);i++){
			if(szline[i] == ':'){
				pnameend = i;
				break;
			}
		} 
		unsigned long long data[32] = {0};
		sscanf(&szline[pnameend + 1], "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", 
			data + RX_BYTES,
			data + RX_PACKETS,
			data + RX_ERRS,
			data + RX_DROP,
			data + RX_FIFO,
			data + RX_FRAME,
			data + RX_COMPRESSED,
			data + RX_MULTICAST,
			
			data + TX_BYTES,
			data + TX_PACKETS,
			data + TX_ERRS,
			data + TX_DROP,
			data + TX_FIFO,
			data + TX_COLLS,
			data + TX_CARRIER,
			data + TX_COMPRESSED);
 
		Adapter[nCount].rx_bytes = data[RX_BYTES];//网卡接收流量
		Adapter[nCount].tx_bytes = data[TX_BYTES];//网卡输出流量
 		memcpy(Adapter[nCount].name,&szline[pnamestart],pnameend - pnamestart);

		cJSON_AddItemToArray(root,pobj[nCount] = cJSON_CreateObject());
		cJSON_AddStringToObject(pobj[nCount],"eth_name",Adapter[nCount].name);
		cJSON_AddNumberToObject(pobj[nCount],"RX",Adapter[nCount].rx_bytes);
		cJSON_AddNumberToObject(pobj[nCount],"TX",Adapter[nCount].tx_bytes);
		nCount++;//网卡数量相加	
		memset(szline, 0, sizeof(szline));//每循环一次，数据清0
	}
	fclose(fp);
	s = cJSON_PrintUnformatted(root);
	if(root)
		cJSON_Delete(root);
	return s;
}

static u32 itoaDecimal(u32 num,char*str)
{
	char index[]="0123456789";
	u8 unum = num;
	u8 i=0,j = 0;
	do{
		str[i++]=index[unum%10];
		unum/=10;
	}while(unum);
	char temp;
	for(j=0;j<=(i-1)/2;j++)
	{
		temp=str[j];
		str[j]=str[i-1-j];
		str[i-1-j]=temp;
	}
	return i;
}

// 数值转换为字符串ip
static void ipNtoA(s8* addr_bytes,uint32_t addr)
{
	u8  length = 0;
	//s8 *tmp = inet_ntoa((struct in_addr){.s_addr=ip->saddr});
	//memcpy(src_bytes,tmp,strnlen(tmp,sizeof(src_bytes)));
	char ipstr[4];
	u8 len = itoaDecimal((addr>>0)&0x000000ff,ipstr);
	memcpy(addr_bytes,ipstr,len);
	memcpy(&addr_bytes[len],".",1);
	length = (len + 1);
	len = itoaDecimal((addr>>8)&0x000000ff,ipstr);
	memcpy(&addr_bytes[length],ipstr,len);
	memcpy(&addr_bytes[length+len],".",1);
	length += (len + 1);
	len = itoaDecimal((addr>>16)&0x000000ff,ipstr);
	memcpy(&addr_bytes[length],ipstr,len);
	memcpy(&addr_bytes[length+len],".",1);
	length += (len + 1);
	len = itoaDecimal((addr>>24)&0x000000ff,ipstr);
	memcpy(&addr_bytes[length],ipstr,len);
}


#define  IP_PROTCOL_ICMP	1
#define  IP_PROTCOL_IGMP	2
#define  IP_PROTCOL_TCP		6
#define  IP_PROTCOL_UDP		17

struct linux_cooked_capture_header{
	u16 pack_type;
	u16 link_layer_addr_type;
	u16 link_layer_addr_length;
	u8 unused[8];
	u16 ether_type;
}__attribute__ ((__packed__));


#if	LINUX_COOKED_CAPTURE
	#define 	ETHERNET_FRAME_HEAD		linux_cooked_capture_header
#else
	#define 	ETHERNET_FRAME_HEAD		ether_header
#endif

#define		ETHERNET_HEADER		sizeof(struct ETHERNET_FRAME_HEAD)
#define		IP_HEADER			sizeof(struct iphdr)


// CRC20_key  CRC20算法根据接收数据的ip、port和协议类型，计算成key值
// 让后利用 桶型hash，装入数据
//1、CRC20_key的实现
#define POLY 0x01101 // CRC20生成多项式x^20+x^12+x^8+1即:01101 CRC32:04C11DB7L
static unsigned int crc_table[256] = {0};
unsigned int get_sum_poly(unsigned char data)
{
	unsigned int sum_poly = data;
	int j = 0;
	sum_poly <<= 24;
	for(j = 0; j < 8; j++)
	{
		int hi = sum_poly&0x80000000; // 取得reg的最高位
		sum_poly <<= 1;
		if(hi) sum_poly = sum_poly^POLY;
	}
	return sum_poly;
}

void create_crc_table(void)  //在使用CRC20_key函数应该先建立crc表
{
	int i;
	for(i = 0; i < 256; i++)
	{
		crc_table[i] = get_sum_poly(i&0xFF);
	}
}

unsigned int CRC20_key(unsigned char* data, int len)
{
	int i = 0;
	unsigned int reg = 0xFFFFFFFF;
	for(i = 0; i < len; i++)
	{
		reg = (reg<<8) ^ crc_table[(reg>>24)&0xFF ^ data[i]];
	}
	return reg;
}
#define  TCPHASHSIZE   255
static list list_ipdata_packet[TCPHASHSIZE];
void hashtableinit(void){
	for (size_t i = 0; i < TCPHASHSIZE; i++)
	{
		/* code */
		list_destroy(&list_ipdata_packet[i]);
		list_init(&list_ipdata_packet[i],free);
	}
}
extern pthread_mutex_t    request_pid_lock;
//2、说明：hash最好的方式为，建立单个链表，如有相同则进行二次查找匹配
bool Tuple_5CalcHash(unsigned int srcip,unsigned short sport,unsigned int dstip,unsigned short dport,unsigned char type){
	unsigned char source[14] = {0};
	source[0] = sport;
	source[1] = sport>>8;
	source[2] = dport;
	source[3] = dport>>8;

	source[4] = srcip>>24;
	source[5] = srcip>>16;
	source[6] = srcip>>8;
	source[7] = srcip;

	source[8] = dstip>>24;
	source[9] = dstip>>16;
	source[10] = dstip>>8;
	source[11] = dstip;

	source[12] = type;
	source[13] = type>>8;
	unsigned int key = CRC20_key(source,14);
	unsigned char index = key%TCPHASHSIZE;
	//printf("Tuple_5CalcHash key %d index %d\n",key,index);
	pthread_mutex_lock(&request_pid_lock);
	list_elmt *cur_elmt = list_head(&list_ipdata_packet[index]);
	while(cur_elmt != NULL){
		unsigned int *e = cur_elmt->data;
		if(*e == key){
			pthread_mutex_unlock(&request_pid_lock);
			return true;
		}
		cur_elmt = cur_elmt->next;
	}
	unsigned int *e = (unsigned int *)malloc(sizeof(unsigned int));		
	*e = key;
	list_ins_next(&list_ipdata_packet[index],NULL,e);	
	pthread_mutex_unlock(&request_pid_lock);
	return false;
}

// ip tcp udp三种协议
// status 0:未找到ip; 2找到ip; 1找到ip且上报过; 3错误;4在白名单内,无需上报
// pthreadNetworkStart 会将建立ip保存一段时间后清空
static networkNode_t pHeadNetList = {.next=NULL};
static void append_network_list(networkNode_t *h, unsigned int dstip)
{
	pthread_mutex_lock(&network_lock);
	networkNode_t **temp,*newNode;
	newNode = malloc(sizeof(networkNode_t));
	if(newNode == NULL)
	{
		pthread_mutex_unlock(&network_lock);
		return; 
	}
	newNode->ipFlag  = 0;
	newNode->tcpFlag = 0;
	newNode->udpFlag = 0;
	newNode->dstip = dstip;
	newNode->time_left = s_net_connect_report_interval;
	newNode->next = NULL;

	temp = &h->next;
	while(*temp!=NULL)
		temp = &(*temp)->next;
	*temp = newNode;
	pthread_mutex_unlock(&network_lock);
}

static int  search_network_list(networkNode_t *h, unsigned int dstip,char flag)
{
	s8 ip_bytes[20] = {0};

	/*Whitelists are used for filtering*/
	if (whiteIpName)
	{
		ipNtoA(ip_bytes, dstip);
		list_elmt *element = whiteIpName->head;
		while (element)
		{
			if(strcmp(element->data, ip_bytes) == 0)
			{
				return 4;
			}
			element = element->next;
		}
	}
	
	pthread_mutex_lock(&network_lock);
	networkNode_t *i;
	i = h->next;
	while(i != NULL)
	{
		if(i->dstip == dstip)  //找到ip
		{
			switch(flag)
			{
				case 0:
					if(i->ipFlag){
						pthread_mutex_unlock(&network_lock);
						return 1;
					}
					else{
						pthread_mutex_unlock(&network_lock);
						return 2;
					}
				case 1:
					if(i->tcpFlag){
						pthread_mutex_unlock(&network_lock);
						return 1;
					}
					else{
						pthread_mutex_unlock(&network_lock);
						return 2;
					}
				case 2:
					if(i->udpFlag){
						pthread_mutex_unlock(&network_lock);
						return 1;
					}
					else{
						pthread_mutex_unlock(&network_lock);
						return 2;
					}
				default:
					pthread_mutex_unlock(&network_lock);
					return 3;
			}
		}
		i = i->next;
 	}
	pthread_mutex_unlock(&network_lock);
	return 0;  //未找到ip，返回0
}

static void change_network_list_state(networkNode_t *h, unsigned int dstip, char state)
{
	pthread_mutex_lock(&network_lock);
	networkNode_t *i;
	i = h->next;
	while(i != NULL)
	{
		if(i->dstip == dstip)
		{
			switch (state)
			{
			case 0:
				i->ipFlag = 1;
				pthread_mutex_unlock(&network_lock);
				return;
			case 1:
				i->tcpFlag = 1;
				pthread_mutex_unlock(&network_lock);
				return;
			case 2:
				i->udpFlag = 1;
				pthread_mutex_unlock(&network_lock);
				return;
			default:
				pthread_mutex_unlock(&network_lock);
				return;
			}
		}
		i = i->next;
 	}
	pthread_mutex_unlock(&network_lock);
}

static void destory_network_list(networkNode_t *h)
{
	pthread_mutex_lock(&network_lock);
	networkNode_t *i,*temp;
	i = h->next;
	while(i != NULL)
	{
		temp = i -> next;
		free(i);
		i = temp;
 	}
	h->next = NULL;
	pthread_mutex_unlock(&network_lock);
}

// 刷新记录连接的ip
static void update_network_list_state(networkNode_t *h)
{
	networkNode_t *i,*tmp;
	pthread_mutex_lock(&network_lock);
	i = h;
	while(i->next != NULL)
	{
		i->next->time_left--;
		if(i->next->time_left == 0)
		{
			tmp = i->next;
			i->next = i->next->next;
			free(tmp);
			continue;
		}
		i = i->next;
	}
	pthread_mutex_unlock(&network_lock);
}

void call(u_char *argument,const struct pcap_pkthdr* pack,const u_char *content)
{	
	// printf("network callback\n\n");
	int ret = 0;
	struct ETHERNET_FRAME_HEAD *ethernet;

	struct iphdr   *ip;
	struct tcphdr  *tcp;
	struct udphdr  *udp;
	struct icmphdr *icmp;
	struct igmphdr *igmp;
	struct arphdr  *arp;

	char PID[0x10] = {0};
	char PIDname[0x80] = {0};
	s8 src_bytes[20] = {0};
	s8 dst_bytes[20] = {0};
		
	// 如果上面的文件存储打开，这里可以将捕获的数据content，写入文件里
	if(pcap_dumper != NULL)
	{
		pcap_dump((char *)pcap_dumper, pack, content);
	}
	ethernet = (struct ETHERNET_FRAME_HEAD *)content;

	if(ntohs(ethernet->ether_type) == ETHERTYPE_IP)
	{
		ip = (struct iphdr*)(content + ETHERNET_HEADER);	
		ipNtoA(src_bytes, ip->saddr);
		ipNtoA(dst_bytes, ip->daddr);

		//网络连接事件、数据发送分析
		switch(ip->protocol)
		{
			case IP_PROTCOL_TCP:
			{
				tcp = (struct tcphdr*)(content + ETHERNET_HEADER + IP_HEADER);

				//FTP port 
				if (tcp->source == htons(21)) {				
					int tcp_header_length = tcp->doff * 4;
					char *data = (char*)(content + ETHERNET_HEADER + IP_HEADER + tcp_header_length);
					
					// Check if the packet contains a FTP command
					if (strncmp(data, "530", 3) == 0) {
						report_user_login_log(dst_bytes);
					}
				}

				if(0 != memcmp(local_net_ip, src_bytes, strnlen(local_net_ip, sizeof(local_net_ip)) + 1))
					break;

				if (tcp->syn != 1)
					break;
				//ip 
				ret = search_network_list(&pHeadNetList,(unsigned int)ip->daddr,0); 
				if(ret == 0 || ret == 2) {
					on_IpConnectEvent_callback(IPV4_VERSION, src_bytes, 
												ntohs(tcp->source), dst_bytes, ntohs(tcp->dest), IP_PROTCOL_TCP);
					if(ret == 0) {
						append_network_list(&pHeadNetList, (unsigned int)ip->daddr);
					}
					
					change_network_list_state(&pHeadNetList, (unsigned int)ip->daddr, 0);
				}
				
				//tcp
				ret = search_network_list(&pHeadNetList, (unsigned int)ip->daddr,1); 
				if(ret == 2) {
					on_TcpConnectEvent_callback(src_bytes, ntohs(tcp->source), dst_bytes, ntohs(tcp->dest));
					change_network_list_state(&pHeadNetList, (unsigned int)ip->daddr, 1);
				}
				break;
			}
			case IP_PROTCOL_UDP:
			{
				udp = (struct udphdr*)(content + ETHERNET_HEADER+IP_HEADER);
					// dns
				if(0 != memcmp(local_net_ip, src_bytes, strnlen(local_net_ip,sizeof(local_net_ip)) + 1)) {
					dns_parser(src_bytes, dst_bytes,udp, 0);
					break;
				} else {
					dns_parser(src_bytes, dst_bytes,udp, 1);
				}
				
				//ip
				ret = search_network_list(&pHeadNetList, (unsigned int)ip->daddr,0); 
				if(ret == 0 || ret == 2) {
					on_IpConnectEvent_callback(IPV4_VERSION, src_bytes, ntohs(udp->source), dst_bytes, ntohs(udp->dest),IP_PROTCOL_UDP);
					if(ret == 0) {
						append_network_list(&pHeadNetList, (unsigned int)ip->daddr);
					}
					change_network_list_state(&pHeadNetList, (unsigned int)ip->daddr, 0);
				}
				
				//udp
				ret = search_network_list(&pHeadNetList, (unsigned int)ip->daddr, 2); 
				if(ret == 2) {
					on_UdpConnectEvent_callback(src_bytes, ntohs(udp->source), dst_bytes, ntohs(udp->dest));
					change_network_list_state(&pHeadNetList, (unsigned int)ip->daddr, 2);
				}
				break;
			}
			default:
			{
				if(0 != memcmp(local_net_ip, src_bytes, strnlen(local_net_ip, sizeof(local_net_ip)) + 1))
					break;
				//ip
				ret = search_network_list(&pHeadNetList, (unsigned int)ip->daddr, 0); 
				if(ret == 0 || ret == 2) {
					on_IpConnectEvent_callback(IPV4_VERSION, src_bytes, 0, dst_bytes, 0, ip->protocol);
					
					if(ret == 0) {
						append_network_list(&pHeadNetList, (unsigned int)ip->daddr);
					}
					change_network_list_state(&pHeadNetList, (unsigned int)ip->daddr, 0);
				}
				break;
			}
		}

		//网络攻击事件和DNS事件、接收数据和部分发送数据分析
		switch(ip->protocol)
		{
			case IP_PROTCOL_TCP:
			{
				tcp=(struct tcphdr*)(content + ETHERNET_HEADER + IP_HEADER);//skip ether and ip header
				if(0 != memcmp(local_net_ip, dst_bytes, strnlen(local_net_ip, sizeof(local_net_ip)) + 1))
					return;
				
				tcp_parser(ip->saddr, ip->daddr, tcp,pack->len);
				break;
			}
			case IP_PROTCOL_UDP:
			{
				udp=(struct udphdr*)(content + ETHERNET_HEADER + IP_HEADER);
				
				udp_parser(ip->saddr, ip->daddr, udp, pack->len);	 
				break;
			}
			case IP_PROTCOL_ICMP:
			{
				icmp=(struct icmphdr*)(content + ETHERNET_HEADER + IP_HEADER);
					
				icmp_parser(ip, icmp, pack->len); 
				break;
			}
			case IP_PROTCOL_IGMP:
			{
				igmp=(struct igmphdr*)(content + ETHERNET_HEADER + IP_HEADER);
				
				igmp_parser();
				break;
			}
			default:
				break;
		}
	} else if(ntohs (ethernet->ether_type) == ETHERTYPE_ARP) {
		arp = (struct arphdr *)(content + ETHERNET_HEADER);
		arp_parser(arp);
	}
	
	//	api_data(argument,pack,content);//flow count
	return;
}
/**
 * @name:   thread_parser
 * @Author: qihoo360
 * @msg:    loop 1S
 * @param   对收集的数据进行分析处理
 * @return: 
 */
static void *thread_parser(void *args){
	for(;;){
		sleep(1);
		udpport_value_consumer();
		tcpport_value_consumer();
		//system_call_implthread();
		pid_value_consumer();
		igmp_value_consumer();
		icmp_value_consumer();
		arp_parser_proc();
		update_network_list_state(&pHeadNetList);

		if (exit_thread_parser_thd == TRUE)
		{
			log_i("networkmonitor", "thread_parser pthread_exit\n");
			pthread_exit(0);
		}

	}
}
/**
 * @name:   thread_parser
 * @Author: qihoo360
 * @msg:    loop 1S
 * @param   对收集的数据进行分析处理
 * @return: 
 */
void create_parserthread(void) 
{
	int stacksize = 6*1024*1024; 
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr); 
	
	if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0) {
		char log[256] = {0};
		sprintf(log,"%s""statcksize set error");
		log_i("networkmonitor", log);
	}
	
	pthread_create(&thread_parser_thd, &attr, thread_parser,NULL);
	if((ret = pthread_attr_destroy(&attr)) != 0) {
		char log[256] = {0};
		sprintf(log,"%s","thread attr destory error");
		log_i("networkmonitor", log);
	}
}
/**
 * @name:   stop_pcap
 * @Author: qihoo360
 * @msg:     
 * @param    
 * @return: 
 */
void stop_pcap(void)
{
	int ret = 0;

	if (thread_parser_thd)
	{
		exit_thread_parser_thd = TRUE;
		ret = pthread_join(thread_parser_thd, NULL);
		thread_parser_thd = 0;
		exit_thread_parser_thd = FALSE;
		log_i("thread_parser", "stop_pcap pthread_join thread_parser_thd\n");
	}

	system_call_free();

	if (pcap_t_handle != NULL)
	{
		pcap_close(pcap_t_handle);
		pcap_t_handle = NULL;
		pcap_init_flag = FALSE;
	}

	if (ip_data_dispatcher_thd)
	{
		ret = pthread_join(ip_data_dispatcher_thd, NULL);
		ip_data_dispatcher_thd = 0;
		log_i("thread_parser", "stop_pcap pthread_join ip_data_dispatcher_thd\n");
	}

	sniffer_stop();
	destory_network_list(&pHeadNetList);

	return;
}
/*
*   read timeout:
	If, when capturing,  packets  are  delivered  as  soon  as  they
	arrive,  the  application capturing the packets will be woken up
	for each packet as it arrives, and might have  to  make  one  or
	more calls to the operating system to fetch each packet.

	If,  instead,  packets are not delivered as soon as they arrive,
	but are delivered after a short delay (called a "read timeout"),
	more  than  one packet can be accumulated before the packets are
	delivered, so that a single wakeup would be  done  for  multiple
	packets,  and  each  set  of  calls made to the operating system
	would supply multiple packets,  rather  than  a  single  packet.
	This reduces the per-packet CPU overhead if packets are arriving
	at a high rate, increasing the number of packets per second that
	can be captured
 *
*/
static void *dispatcher(void *args)
{
	pthread_t pthreadNetwork;
	pcap_if_t *p = NULL;
	char error[100];
	struct in_addr net_ip_addr;
	struct in_addr net_mask_addr;
	struct ether_header *ethernet;
	
	char *net_ip_string;
	char *net_mask_string;
	char *interface;
	u_int32_t net_ip;
	u_int32_t net_mask;
	
	struct pcap_pkthdr pack; 
	const u_char *content;
	static bool start_oneshot = false;
	interface = args;
	get_local_ip(interface);	
	if((pcap_t_handle=pcap_open_live(interface,IF_INTERFACE_MAX_SIZE,1,100,error))==NULL){
		char log[256] = {0};
		sprintf(log,"%s\n",error);
		log_v("networkmonitor", log);
		return NULL;
	}
	//	pcap_setnonblock(pcap_t_handle, 1, error);
	if(pcap_lookupnet(interface,&net_ip,&net_mask,error)==-1){
		char log[256] = {0};
		sprintf(log,"%s",error);
		log_v("networkmonitor", log);
		return NULL;
	}

	create_crc_table();//from table
	tcp_scanner_init();//tcp init
	udp_scanner_init();//udp init
	//system_call_init();//you can delete it
	piddetection_scanner_init();//pid init
	icmp_scan_init();//icmp init
	//igmp no init
	//arp  no init
	create_parserthread();//thread create

	get_local_mac(interface);
	net_ip_addr.s_addr=net_ip;
	net_ip_string  =inet_ntoa((struct in_addr){.s_addr=net_ip_addr.s_addr});
	net_mask_string=inet_ntoa((struct in_addr){.s_addr=net_mask});
	pcap_init_flag = TRUE;
	pcap_loop(pcap_t_handle,-1,call,interface);
	return NULL;
}

void data_dispatcher_init(s8 *interface_name)
{
	static s8 tmp[IF_INTERFACE_NAME_MAX_SIZE];
	if(interface_name == NULL)
		return;
	if(ip_data_dispatcher_thd != 0)
		return;
	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,interface_name,strnlen(interface_name,IF_INTERFACE_NAME_MAX_SIZE));
	int stacksize = 100*1024; 
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr); 
	if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0){
		char log[256] = {0};
		sprintf(log,"%s","statcksize set error");
		log_i("networkmonitor", log);
	}
	pthread_create(&ip_data_dispatcher_thd,&attr,dispatcher,tmp);
	if((ret = pthread_attr_destroy(&attr)) != 0){
		char log[256] = {0};
		sprintf(log,"%s","thread attr destory error");
		log_i("networkmonitor", log);
	}
	while(pcap_init_flag == FALSE)
		usleep(10000);
}

void list_network_card()
{
	pcap_t *handle;
	pcap_if_t *alldev;
	pcap_if_t *p;
	char error[100];
	char *interface;
	int i=0,num;
	if(pcap_findalldevs(&alldev,error)==-1)
	{
		char log[256] = {0};
		sprintf(log,"%s""find all devices is error");
		log_i("networkmonitor", log);
		return;
	}
	for(p=alldev;p;p=p->next)
	{
		//printf("%d:%s\n",++i,p->name);
		if(p->description)
		{
			char log[256] = {0};
			sprintf(log,"%s",p->description);
			log_v("networkmonitor", log);
		}
	}
	if(i==1)
		interface=p->name;
	else
	{
		printf("please input which interface you want to use\n");
		int result = scanf("%d",&num);
		if(num<1||num>i)
		{
			char log[256] = {0};
			sprintf(log,"%s","interface is unavillible");
			log_i("networkmonitor", log);
			return;
		}
		for(p=alldev,i=1;i<=num;p=p->next,i++)
			interface=p->name;
	}
}







