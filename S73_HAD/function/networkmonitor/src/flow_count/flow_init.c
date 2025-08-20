/*
 * @Author: idps members
 * @Date: 2021-01-18 14:45:58
 * @LastEditTime: 2021-05-14 09:36:28
 * @LastEditors: Please set LastEditors
 * @Description: init pcap handle(you may not use in some instance)
 * @FilePath: \idps_networkmonitor\src\flow_count\flow_calc.c
 */
//user h file
#include "ether.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include "ethertype.h"
#include <sys/types.h>
#include "token.h"
#include "ppp.h"
#include "extract.h"
#include "llc.h"
#include "flow_init.h"
#include <ifaddrs.h>  
#include <arpa/inet.h> 
#include <time.h>
#include "ip.h"
#include "cJSON.h"
#include "api_networkmonitor.h"
#include <string.h>
#include <unistd.h>
#include "spdloglib.h"
#include "common.h"

#ifdef DLT_LINUX_SLL
	#include "sll.h"
#endif
unsigned int flowinterval = 60;
/**
 * @description:  global info
 * @param      :  
 * @return     :  
 */
interface_instance instance_eth0 = {.list_flow_packet = NULL};
interfaceinfo      instance_info = {.count = 0};//current deviceinfo
static long long   continuedtime = 0;
static pthread_t   flow_loop;

#ifndef THREAD_MODULE 
	pcap_handler    processhandler;
#endif

/* Only need ethernet (plus optional 4 byte VLAN) and IP headers (48) + first 2 bytes of tcp/udp header */
#ifndef CAPTURE_LENGTH
	#define CAPTURE_LENGTH 72
#endif
/**
 * @description:  port node
 * @param      :  to record list port
 * @return     :  void
 */
typedef struct{
	long long total_sent;
	long long total_recv;
	struct in_addr  instance_ip;//设备destaddress
	struct in_addr  device_ip;//设备deviceaddress
	unsigned short int interval;//多长时间没有数据，超过60S没有数据则清除这条规则
} portnode;
/**
 * @description:  由于模块数目较少，故采用栈空间
 * @param      :  
 * @return     :  
 */
#define MOST_DEVICE 56
struct {
	struct in_addr     instance_module[MOST_DEVICE];
	int                validcount;
}moduledevice = {.validcount = 0};
/**
 * @description: 
 * @param      :direction:0:incoming 1:leaving 
 * @return     :void
 * @decla      :
 */
#define ip_addr_netcmp(addr1, addr2, mask) ((addr1 & mask) == (addr2 & mask))
/**
 * @description:  ip_addr_match
 * @param      :  
 * @return     :  
 */
int ip_addr_match(struct in_addr addr,interface_instance* instance) {
    return addr.s_addr == instance->if_ip_addr.s_addr;
}
/*
 * This function identifies the IP address and ethernet address for the requested
 * interface
 *
 * This function returns -1 on catastrophic failure, or a bitwise OR of the
 * following values:
 *
 * 1 - Was able to get the ethernet address
 * 2 - Was able to get the IP address
 *
 * This function should return 3 if all information was found
 */

int get_addrs_ioctl(char *interface, char if_hw_addr[], struct in_addr *if_ip_addr)
{
  int s;
  struct ifreq ifr = {};
  int got_hw_addr = 0;
  int got_ip_addr = 0;

  /* -- */

  s = socket(PF_INET, SOCK_DGRAM, 0); /* any sort of IP socket will do */

  if (s == -1) {
    perror("socket");
    return -1;
  }

  fprintf(stderr,"interface: %s\n", interface);

  memset(if_hw_addr, 0, 6);
  strncpy(ifr.ifr_name, interface, IFNAMSIZ);

#ifdef SIOCGIFHWADDR
  if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
    fprintf(stderr, "Error getting hardware address for interface: %s\n", interface); 
    perror("ioctl(SIOCGIFHWADDR)");
  }
  else {
    memcpy(if_hw_addr, ifr.ifr_hwaddr.sa_data, 6);
    got_hw_addr = 1;
  }
#else
#if defined __FreeBSD__ || defined __OpenBSD__ || defined __APPLE__
  {
    int sysctlparam[6] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0};
    size_t needed = 0;
    char *buf = NULL;
    struct if_msghdr *msghdr = NULL;
    sysctlparam[5] = if_nametoindex(interface);
    if (sysctlparam[5] == 0) {
      fprintf(stderr, "Error getting hardware address for interface: %s\n", interface);
      goto ENDHWADDR;
    }
    if (sysctl(sysctlparam, 6, NULL, &needed, NULL, 0) < 0) {
      fprintf(stderr, "Error getting hardware address for interface: %s\n", interface);
      goto ENDHWADDR;
    }
    if ((buf = malloc(needed)) == NULL) {
      fprintf(stderr, "Error getting hardware address for interface: %s\n", interface);
      goto ENDHWADDR;
    }
    if (sysctl(sysctlparam, 6, buf, &needed, NULL, 0) < 0) {
      fprintf(stderr, "Error getting hardware address for interface: %s\n", interface);
      free(buf);
      goto ENDHWADDR;
    }
    msghdr = (struct if_msghdr *) buf;
    memcpy(if_hw_addr, LLADDR((struct sockaddr_dl *)(buf + sizeof(struct if_msghdr) - sizeof(struct if_data) + sizeof(struct if_data))), 6);
    free(buf);
    got_hw_addr = 1;

  ENDHWADDR:
    1; /* compiler whines if there is a label at the end of a block...*/
  }
#else
  fprintf(stderr, "Cannot obtain hardware address on this platform\n");
#endif
#endif
  
  /* Get the IP address of the interface */
#ifdef SIOCGIFADDR
  (*(struct sockaddr_in *) &ifr.ifr_addr).sin_family = AF_INET;
  if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
    fprintf(stderr, "Unable to get IP address for interface: %s\n", interface); 
    perror("ioctl(SIOCGIFADDR)");
  }
  else {
    memcpy(if_ip_addr, &((*(struct sockaddr_in *) &ifr.ifr_addr).sin_addr), sizeof(struct in_addr));
    got_ip_addr = 2;
  }
#else
  fprintf(stderr, "Cannot obtain IP address on this platform\n");
#endif
  
  close(s);

  return got_hw_addr + got_ip_addr;
}
/**
 * @description:match device
 * @param      :direction:0:incoming 1:leaving 
 * @return     :void
 * @decla      :
 */
bool module_filter(unsigned int __ipmatch)
{
	bool state = true;
	for(int loop = 0;loop < moduledevice.validcount;loop ++){
		if(__ipmatch == moduledevice.instance_module[loop].s_addr){
			state = false;
			break;
		}
	}
	return state;
}
/**
 * @description:确定是否在相同设备的子网络
 * @param      :direction:0:incoming 1:leaving 
 * @return     :void
 * @decla      :
 */
static bool lan_ckeck(struct ip* iptr,int direction){
	unsigned int l_matchip = 0;
	unsigned int l_deviceip = 0;
	if(direction == 0){
		l_matchip = iptr->ip_src.s_addr;
		l_deviceip = iptr->ip_dst.s_addr;
	}
	else{
		l_matchip = iptr->ip_dst.s_addr;
		l_deviceip = iptr->ip_src.s_addr;
	}
	if(module_filter(l_deviceip))
	  return true;
	for(int i = 0;i < instance_info.count ;i ++){
		if(ip_addr_netcmp(l_matchip,instance_info.netinfo.device_net[i].s_addr,instance_info.netinfo.netmask[i].s_addr)){
			return true;	
		}
	}
	return false;	
}
/**
 * @description:通过上一步的函数匹配，确定是带统计模块 
 * @param      :direction:0:incoming 1:leaving 
 * @return     :void
 * @decla      :filter_flowModule
 */
void filter_flowModule(struct ip* iptr,int direction,interface_instance* instance){
	unsigned int l_matchip = 0,l_matchipdevice = 0;
	int len = ntohs(iptr->ip_len);	
	pthread_mutex_lock(&(instance->mtx));
	if(direction == 0){//incoming
		l_matchip = iptr->ip_src.s_addr;
		l_matchipdevice = iptr->ip_dst.s_addr;
		instance->total_recv += len;
	}
	else{
		l_matchip = iptr->ip_dst.s_addr;
		l_matchipdevice = iptr->ip_src.s_addr;
		instance->total_sent += len;
	} 

    list_elmt *cur_elmt_Info = list_head(instance->list_flow_packet);
	while(cur_elmt_Info != NULL){
		portnode* firstinfo = cur_elmt_Info->data;
		if(((firstinfo->instance_ip.s_addr == l_matchip)&&(l_matchipdevice == firstinfo->device_ip.s_addr))){		
			if(direction == 0){
				firstinfo->total_recv += len;
			}
			else{
				firstinfo->total_sent += len;
			}
			pthread_mutex_unlock(&(instance->mtx));
			return;
		}
		cur_elmt_Info = cur_elmt_Info->next;
	}
	
	portnode* listnode = (portnode*)malloc(sizeof(portnode));
	memset(listnode,0,sizeof(portnode));
	listnode->instance_ip.s_addr = l_matchip;
	listnode->device_ip.s_addr = l_matchipdevice;
	
	if(direction == 0)
		listnode->total_recv = len;
	else
		listnode->total_sent = len;
	
	list_ins_next(instance->list_flow_packet, NULL, listnode);
	pthread_mutex_unlock(&(instance->mtx));
}

/**
 * @description:  
 * @param      :
 * @return     :
 * @notify     :具体上传数据格式需要后续修改确定 
 * @format     :基本数据格式如下所示：[{"netdevice":"eth0","totalrecv":2349,
 * "totalsend":8979,"info_flow":[{"netaddress":"192.168.12.3","totalrecv":9080,
 * "totalsend":8979},{"netaddress":"192.168.12.4","totalrecv":9080,"totalsend":8979}]]
 */ 
extern char* getnettxrx();

void sendstring(interface_instance* instance){
	cJSON *root = NULL;
	cJSON *Cjson_ip_data = NULL;
	cJSON *Cjson_ipinfo  = NULL;
	cJSON *Cjson_dst_ip  = NULL;
	cJSON *Cjson_info    = NULL;
	if((root = cJSON_CreateObject()) == NULL)
	{
		char log[256] = {0};
		sprintf(log,"%s","cJSON_CreateArray() oops");
		log_e("networkmonitor", log);
		return;
	}
  
	if(instance->list_flow_packet == NULL){
		cJSON_Delete(root);
		return;
	}

	// 为了效率暂时没循环本地ip,默认只监测一个
	Cjson_ip_data = cJSON_CreateArray();
	{
		Cjson_ipinfo = cJSON_CreateObject();
		Cjson_dst_ip = cJSON_CreateArray();
		struct in_addr local_ip;
		long long local_total_recv = 0, local_total_sent = 0;
		char l_buffer[IFNAMSIZ] = {0};

		pthread_mutex_lock(&(instance->mtx));
		list_elmt *cur_elmt_Info = list_head(instance->list_flow_packet);
		//printf("TX RX info is %s\n",s45);
		while(cur_elmt_Info != NULL){
			portnode* firstinfo = cur_elmt_Info->data;
			local_ip = firstinfo->device_ip;
			local_total_recv += firstinfo->total_recv;
			local_total_sent += firstinfo->total_sent;

			Cjson_info = cJSON_CreateObject();
			memset(l_buffer,0,IFNAMSIZ);
			sprintf(l_buffer,"%s",inet_ntoa(firstinfo->instance_ip));
			cJSON_AddStringToObject(Cjson_info,"remote_ip",l_buffer);//deviceaddress
			cJSON_AddNumberToObject(Cjson_info,"received_bytes",firstinfo->total_recv);
			cJSON_AddNumberToObject(Cjson_info,"send_bytes",firstinfo->total_sent);	
			cJSON_AddItemToArray(Cjson_dst_ip,Cjson_info);
			cur_elmt_Info = cur_elmt_Info->next;
		}
		pthread_mutex_unlock(&(instance->mtx));

		memset(l_buffer,0,IFNAMSIZ);
		sprintf(l_buffer,"%s",inet_ntoa(local_ip));
		cJSON_AddItemToObject(Cjson_ipinfo,  "ip_data",Cjson_dst_ip);
		cJSON_AddStringToObject(Cjson_ipinfo,"iface_ip",l_buffer);//deviceaddress
		cJSON_AddNumberToObject(Cjson_ipinfo,"total_received_bytes",local_total_recv);
		cJSON_AddNumberToObject(Cjson_ipinfo,"total_send_bytes",local_total_sent);
		cJSON_AddItemToArray(Cjson_ip_data, Cjson_ipinfo);
	}
	cJSON_AddItemToObject(root,  "iface_data", Cjson_ip_data);
	cJSON_AddNumberToObject(root,"start_time",continuedtime);
	continuedtime = clockobj.get_current_time();
	cJSON_AddNumberToObject(root,"end_time",continuedtime);
	
	char* s = cJSON_PrintUnformatted(root);
	if(root)
		cJSON_Delete(root);
    on_FlowDataReport_callback(s);
	free(s);
}

/**
 * @description:handle_ip_packet
 * @param      :process data function
 * @return     :void
 * @notify     :
 */ 
static void handle_ip_packet(struct ip* iptr, int hw_dir,unsigned char* args,interface_instance* instance)
{
    int direction = 0; 
	if(hw_dir == 1) 
		direction = 1;
	else if(hw_dir == 0) 
		direction = 0;
	else if(instance->have_ip_addr && ip_addr_match(iptr->ip_src,instance)) {
		direction = 1;
	}
	else if(instance->have_ip_addr && ip_addr_match(iptr->ip_dst,instance)) {
		direction = 0;
	}
	/*
		* Cannot determine direction from hardware or IP levels.  Therefore 
		* assume that it was a packet between two other machines, assign
		* source and dest arbitrarily (by numerical value) and account as 
		* incoming.
		*/
	else if(iptr->ip_src.s_addr < iptr->ip_dst.s_addr) 
		direction = 0;
	else{
		direction = 0;
	}
 	filter_flowModule(iptr,direction,instance);
}
/**
 * @description:find eth instance in callback
 * @param      :
 * @return     :
 */ 
#define MATCH_INSTANCE() 	interface_instance *instance = &instance_eth0;
//interface_instance *instance = NULL;\
//if(memcmp(args,instance_eth0.interface,strlen(args)) == 0){\
//instance = &instance_eth0;}\
//else{\
//printf("oops cannot find netdevice name args %s interface %s\n",args,instance_eth0.interface);}
/**
 * @description:handle_llc_packet
 * @param      :process data function
 * @return     :void
 */ 
static void handle_llc_packet(const struct llc* llc, int dir,unsigned char* args,interface_instance* instance) {
    struct ip* ip = (struct ip*)((void*)llc + sizeof(struct llc));
	/* Taken from tcpdump/print-llc.c */
    if(llc->ssap == LLCSAP_SNAP && llc->dsap == LLCSAP_SNAP
       && llc->llcui == LLC_UI) {
        u_int32_t orgcode;
        register u_short et;
        orgcode = EXTRACT_24BITS(&llc->llc_orgcode[0]);
        et = EXTRACT_16BITS(&llc->llc_ethertype[0]);
        switch(orgcode) {
          case OUI_ENCAP_ETHER:
          case OUI_CISCO_90:
            handle_ip_packet(ip, dir,args,instance);
            break;
          case OUI_APPLETALK:
            if(et == ETHERTYPE_ATALK) {
              handle_ip_packet(ip, dir,args,instance);
            }
            break;
          default:;
            /* Not a lot we can do */
        }
    }
}
/**
 * @description:handle_raw_packet
 * @param      :callback function
 * @return     :void
 */ 
static void handle_raw_packet(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet)
{
	MATCH_INSTANCE()
    handle_ip_packet((struct ip*)packet, -1,args,instance);
}
/**
 * @description:handle_tokenring_packet
 * @param      :callback function
 * @return     :void
 */ 
static void handle_tokenring_packet(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet)
{
    struct token_header *trp;
    int dir = -1;
    trp = (struct token_header *)packet;

    if(IS_SOURCE_ROUTED(trp)) {
      packet += RIF_LENGTH(trp);
    }
    packet += TOKEN_HDRLEN;
	MATCH_INSTANCE()
    if(memcmp(trp->token_shost, instance->if_hw_addr, 6) == 0 ) {
      /* packet leaving this i/f */
      dir = 1;
    } 
    else if(memcmp(trp->token_dhost, instance->if_hw_addr, 6) == 0 || memcmp("\xFF\xFF\xFF\xFF\xFF\xFF", trp->token_dhost, 6) == 0) {
      /* packet entering this i/f */
      dir = 0;
    }
    /* Only know how to deal with LLC encapsulated packets */
    if(FRAME_TYPE(trp) == TOKEN_FC_LLC) {
      handle_llc_packet((struct llc*)packet, dir,args,instance);
    }
}
/**
 * @description:handle_tokenring_packet
 * @param      :callback function
 * @return     :void
 */ 
static void handle_ppp_packet(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet)
{
	register u_int length = pkthdr->len;
	register u_int caplen = pkthdr->caplen;
	u_int proto;

	if (caplen < 2) 
        return;
	MATCH_INSTANCE()
	if(packet[0] == PPP_ADDRESS) {
		if (caplen < 4) 
            return;

		packet += 2;
		length -= 2;

		proto = EXTRACT_16BITS(packet);
		packet += 2;
		length -= 2;

        if(proto == PPP_IP || proto == ETHERTYPE_IP) {
            handle_ip_packet((struct ip*)packet, -1,args,instance);
        }
    }
}
/**
 * @description:handle_cooked_packet
 * @param      :callback function
 * @return     :void
 */ 
#ifdef DLT_LINUX_SLL
static void handle_cooked_packet(unsigned char *args, const struct pcap_pkthdr * thdr, const unsigned char * packet)
{
    struct sll_header *sptr;
    int dir = -1;
    sptr = (struct sll_header *) packet;

    switch (ntohs(sptr->sll_pkttype))
    {
    case LINUX_SLL_HOST:
        /*entering this interface*/
	dir = 0;
	break;
    case LINUX_SLL_OUTGOING:
	/*leaving this interface */
	dir=1;
	break;
    }
	MATCH_INSTANCE()
    handle_ip_packet((struct ip*)(packet+SLL_HDR_LEN), dir,args,instance);
}
#endif /* DLT_LINUX_SLL */

/**
 * @description:handle_eth_packet
 * @param      :callback function
 * @return     :void
 */
static void handle_eth_packet(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet)
{
    struct ether_header *eptr;
    int ether_type;
    const unsigned char *payload;
    eptr = (struct ether_header*)packet;
    ether_type = ntohs(eptr->ether_type);
    payload = packet + sizeof(struct ether_header);

    if(ether_type == ETHERTYPE_8021Q) {
	struct vlan_8021q_header* vptr;
	vptr = (struct vlan_8021q_header*)payload;
	ether_type = ntohs(vptr->ether_type);
        payload += sizeof(struct vlan_8021q_header);
    }
	MATCH_INSTANCE()

	if(ether_type == ETHERTYPE_IP) {
        struct ip* iptr;
        int dir = -1;
        /*
         * Is a direction implied by the MAC addresses?
         */
        if(instance->have_hw_addr && memcmp(eptr->ether_shost, instance->if_hw_addr, 6) == 0 ) {
            /* packet leaving this i/f */
            dir = 1;
        }
        else if(instance->have_hw_addr && memcmp(eptr->ether_dhost, instance->if_hw_addr, 6) == 0 ) {
	    /* packet entering this i/f */
	   	 dir = 0;
		}
		else if (memcmp("\xFF\xFF\xFF\xFF\xFF\xFF", eptr->ether_dhost, 6) == 0) {
	  /* broadcast packet, count as incoming */
            dir = 0;
        }
		else{
			
		}
        iptr = (struct ip*)(payload); /* alignment? */
		
        handle_ip_packet(iptr, dir,args,instance);
    }
}
/**
 * @description:查找当前设备的所有内网子网段
 * @param      : 
 * @return     :void
 * @decla      :
 */
int find_netdevice(void){
	struct sockaddr_in *sin = NULL;
	struct ifaddrs *ifa = NULL, *ifList = NULL;
	if (getifaddrs(&ifList) < 0)
	{
		char log[256] = {0};
		sprintf(log,"%s","getifaddrs(&ifList) < 0 error");
		log_i("networkmonitor", log);
		return -1;
	}
	
	for (ifa = ifList; ifa != NULL; ifa = ifa->ifa_next)
	{
		if(ifa->ifa_addr == NULL)
			continue;
         if(ifa->ifa_addr->sa_family == AF_INET)
         {
			 memcpy(instance_info.netinfo.interface[instance_info.count],ifa->ifa_name,strlen(ifa->ifa_name));
			 instance_info.netinfo.device_net[instance_info.count].s_addr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
             sin = (struct sockaddr_in *)ifa->ifa_addr; 
			 instance_info.netinfo.netmask[instance_info.count].s_addr = ((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr.s_addr;
             sin = (struct sockaddr_in *)ifa->ifa_netmask;
			 instance_info.count ++;
			 if(instance_info.count >= DEVICE_ETH){
				 freeifaddrs(ifList);
				 return 1;
			 }
		 }
     }
     freeifaddrs(ifList);
     return 0;
}
/**
 * @description: get netmask 
 * @param      :interface:eth0 or wlan0 
 * @return     :int
 */
int get_netmask(char* _input_interface,unsigned int*  _out_mask){
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror("socket");
		return -1;		
	}
	strncpy(ifr.ifr_name, _input_interface, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	if (ioctl(sock, SIOCGIFNETMASK, &ifr) < 0)
	{
		perror("ioctl");
		return -1;
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));		
	*_out_mask = sin.sin_addr.s_addr;
	close(sock);

	return 0;
}
/**
 * @description:packet_inithandle 
 * @param      :interface:eth0 or wlan0 
 * @return     :void
 * @notify     :单独抓取数据的时候采用线程模式，外部接口的时候采用基本初始化模式
 */
void *packet_inithandle(void *__instance){
	int dlt = 0,result = 0;
	char errbuf[128] = {0};
	interface_instance *instance = __instance;
	
	pthread_mutex_init(&(instance->mtx),NULL);	
	instance->total_recv = 0;
	instance->total_sent = 0;
	instance->initstate = false;
	instance->list_flow_packet = (list*)malloc(sizeof(list));
	// list_destroy(instance->list_flow_packet);
	memset(instance->list_flow_packet,0,sizeof(list));
	list_init(instance->list_flow_packet,free);
	continuedtime = clockobj.get_current_time();
	 
	//step2:MAC and IP and mask
#ifdef HAVE_DLPI
    result = get_addrs_dlpi(instance->interface, instance->if_hw_addr, &(instance->if_ip_addr));
#else
    result = get_addrs_ioctl(instance->interface, instance->if_hw_addr, &(instance->if_ip_addr));
#endif
    if (result < 0) {
      fprintf(stderr, "get_addrs_ioctl(%s): %s\n", instance->interface, instance->if_hw_addr);
	  return NULL;
    }

    instance->have_hw_addr = result & 1;
    instance->have_ip_addr = result & 2;

	get_netmask(instance->interface,&(instance->netmask.s_addr));
	instance->pd = pcap_open_live(instance->interface, CAPTURE_LENGTH, 1,100, errbuf);
	if(instance->pd == NULL) { 
		fprintf(stderr, "pcap_open_live(%s): %s\n", instance->interface, errbuf); 
		return NULL;
	}
	dlt = pcap_datalink(instance->pd);
	instance->initstate = true;
	if(dlt == DLT_EN10MB) {
#ifdef THREAD_MODULE 
		pcap_loop(instance->pd,-1,(pcap_handler)handle_eth_packet,instance->interface);
#else
		processhandler = handle_eth_packet;
#endif
    }
    else if(dlt == DLT_RAW || dlt == DLT_NULL) {
#ifdef THREAD_MODULE 
		pcap_loop(instance->pd,-1,(pcap_handler)
		handle_raw_packet,instance->interface);
#else
	    processhandler = handle_raw_packet;
#endif
	} 
    else if(dlt == DLT_IEEE802) {
#ifdef THREAD_MODULE
		pcap_loop(instance->pd,-1,(pcap_handler)handle_tokenring_packet,instance->interface);
#else
		processhandler = handle_tokenring_packet;
#endif
	}
    else if(dlt == DLT_PPP) {
#ifdef THREAD_MODULE
		pcap_loop(instance->pd,-1,(pcap_handler)handle_tokenring_packet,instance->interface);
#else
        processhandler = handle_tokenring_packet;
#endif
	}
	/* 
 * SLL support not available in older libpcaps
 */
#ifdef DLT_LINUX_SLL
    else if(dlt == DLT_LINUX_SLL) {
#ifdef THREAD_MODULE
	  pcap_loop(instance->pd,-1,(pcap_handler)handle_cooked_packet,instance->interface);
#else
      processhandler = handle_cooked_packet;
#endif
	}
#endif
    else {
        fprintf(stderr, "Unsupported datalink type: %d\n"
                "Please email pdw@ex-parrot.com, quoting the datalink type and what you were\n"
                "trying to do at the time\n.", dlt);
    }
#ifndef THREAD_MODULE
	if(instance->pd != NULL)
		pcap_close(instance->pd);
#endif
}
/**
 * @description: 定时上传线程 
 * @param      : 
 * @return     :void
 * @notify     :
 */
void *uploadflowinfo(void* arg){
	sleep(5);
	for(;;){
		sleep(flowinterval);
		sendstring(&instance_eth0);
		pthread_mutex_lock(&(instance_eth0.mtx));
		instance_eth0.total_recv = 0;
		instance_eth0.total_sent = 0;
		if(instance_eth0.list_flow_packet != NULL){
			list_destroy(instance_eth0.list_flow_packet);
			list_init(instance_eth0.list_flow_packet,free);
		}
		pthread_mutex_unlock(&(instance_eth0.mtx));
	}
}
/**
 * @description:module appoint
 * @param      : 
 * @return     :void
 * @decla      :
 */
void addmoduledevice(char* _ip){
	moduledevice.instance_module[moduledevice.validcount].s_addr = inet_addr(_ip);
	moduledevice.validcount = (moduledevice.validcount + 1)%MOST_DEVICE;
}
/**
 * @description: 流量启动API 
 * @param      : 
 * @return     :void
 * @notify     :
 */
void flowmoduleinit(char *watchNicDevice){
	int ret = 0,stacksize = 2000*1024; 
	pthread_attr_t attr;

	memset(instance_eth0.interface, 0, sizeof(instance_eth0.interface));
	strncpy(instance_eth0.interface, watchNicDevice, sizeof(instance_eth0.interface) - 1);
	find_netdevice();
	ret = pthread_attr_init(&attr); 
	if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0){
		char log[256] = {0};
		sprintf(log,"oops file %s line %d\n",__FILE__,__LINE__);
		log_i("networkmonitor", log);
	}
	pthread_create(&(instance_eth0.ip_dispatcher_thd),&attr,packet_inithandle,&instance_eth0);
	if((ret = pthread_attr_destroy(&attr)) != 0){
		char log[256] = {0};
		sprintf(log,"%s","thread attr destory error");
		log_i("networkmonitor", log);
	}
	
	ret = pthread_attr_init(&attr); 
	if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0){
		char log[256] = {0};
		sprintf(log,"oops file %s line %d\n",__FILE__,__LINE__);
		log_i("networkmonitor", log);
	}
	pthread_create(&(flow_loop),&attr,uploadflowinfo,NULL);
	if((ret = pthread_attr_destroy(&attr)) != 0){
		char log[256] = {0};
		sprintf(log,"%s","thread attr destory error");
		log_i("networkmonitor", log);
	}
     
	while((instance_eth0.initstate == false))
		usleep(1000);	
}
/*
* function:setflowinterval
* input   :如参数名所示 
* output  :void 
* decla   :
*/
void setflowinterval(int interval){
	flowinterval = interval;
}
/**
 * @description: 流量由外部提供，其基本外调API 
 * @param      : 
 * @return     :voidmake
 * @notify     :
 */
#ifndef THREAD_MODULE
void api_data(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet){
	processhandler(args,pkthdr,packet);
}
#endif
 


