/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-08-09 06:23:50
 */ 
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include "cJSON.h"
#include <unistd.h>
#include "common.h"
#include "Networkmonitor.h"
#include "api_networkclient.h"
#include "Base_networkmanager.h"
#include "websocketmanager.h"
#include "flow_init.h"
#include "data_dispatcher.h"
#include "pid_detection.h"

#if MODULE_NETWORKMONITOR
/**
 * @name:   function
 * @Author: qihoo360
 * @msg: 
 * @param   
 * @return: 
 */
static timerIdps *timernetworkmonitor = NULL; 
static char s_watchNicDevice[32] = {0};
static bool startupFlowFalg = 0;
static list *mIpWhiteList  = NULL;
static list *mDNSWhiteList  = NULL;

/**
 * @name:   const string list
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
typedef struct _Pair{
    int key;
    char url[48];
}Pairmonitor;    

/**
 * @name:    base enum  
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
enum{
	TCP_CONNECT_SCAN = 1,
	TCP_SYN_SCAN,
	TCP_FIN_SCAN,
	TCP_ACK_SCAN,
	TCP_NULL_SCAN,	
	TCP_XMAS_SCAN,	
	TCP_SRC_PORT_ZERO = 8,
	TCP_PORT_SYN_FLOOD,
	TCP_ACK_FIN_DOS,
	TCP_ACK_RST_DOS,
	TCP_FIN_SYN_DOS,
	TCP_FIN_RST_DOS,
	TCP_ACK_PSH_FLOOD,	//14	
	TCP_SYN_FLOOD = 20,
	TCP_SYN_ACK_FLOOD,
	TCP_LAND_ATTACK = 30,
	TCP_FIN_SYN_STACK_ABNORMAL,
	TCP_CONNECT_ATTACK,
	UDP_SRC_PORT_ZERO = 40,
	UDP_PORT_SCAN,
	UDP_PORT_FLOOD,
	FRAGGLE_ATTACK,
	ICMP_DEATH_PING = 50,  		
	ICMP_LARGE_PING, 
	ICMP_ECHO_FLOODING,  	
	ICMP_FLOOD,
	ICMP_SMURF_ATTACK,
	ICMP_IGMP_FLOOD,		
	ICMP_FORGE_SRC_ATTACK,   
	ICMP_TERMINAL_EXIST_DETECT,	   
	IGMP_FLOODING = 60,		
	ARP_ATTACK_0 = 70,			
	ARP_ATTACK_1,
	ARP_ATTACK_2,
	IP_PACK_WITH_OPTION = 80,
	IP_PACK_WITH_TIMESTAMP,
	IP_PACK_WITH_RECORD_TRACE,
	IP_PACK_WITH_EOL,			
	IP_PACK_WITH_SATID
};
/**
 * @name:   warning struct
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
#define  NETWORKMONITORPAIR     (39)
Pairmonitor networkmonitorpair[NETWORKMONITORPAIR] = 
{
	{TCP_CONNECT_SCAN,"TCP_CONNECT_SCAN"},
	{TCP_SYN_SCAN,"TCP_SYN_SCAN"},
	{TCP_FIN_SCAN,"TCP_FIN_SCAN"},
	{TCP_ACK_SCAN,"TCP_ACK_SCAN"},
	{TCP_NULL_SCAN,"TCP_NULL_SCAN"},	
	{TCP_XMAS_SCAN,"TCP_XMAS_SCAN"},	
	{TCP_SRC_PORT_ZERO,"TCP_SRC_PORT_ZERO"},
	{TCP_PORT_SYN_FLOOD,"TCP_PORT_SYN_FLOOD"},
	{TCP_ACK_FIN_DOS,"TCP_ACK_FIN_DOS"},
	{TCP_ACK_RST_DOS,"TCP_ACK_RST_DOS"},
	{TCP_FIN_SYN_DOS,"TCP_FIN_SYN_DOS"},
	{TCP_FIN_RST_DOS,"TCP_FIN_RST_DOS"},
	{TCP_ACK_PSH_FLOOD,"TCP_ACK_PSH_FLOOD"},		
	{TCP_SYN_FLOOD,"TCP_SYN_FLOOD"},
	{TCP_SYN_ACK_FLOOD,"TCP_SYN_ACK_FLOOD"},
	{TCP_LAND_ATTACK,"TCP_LAND_ATTACK"},
	{TCP_FIN_SYN_STACK_ABNORMAL,"TCP_FIN_SYN_STACK_ABNORMAL"},
	{TCP_CONNECT_ATTACK,"TCP_CONNECT_ATTACK"},
	{UDP_SRC_PORT_ZERO,"UDP_SRC_PORT_ZERO"},
	{UDP_PORT_SCAN,"UDP_PORT_SCAN"},
	{UDP_PORT_FLOOD,"UDP_PORT_FLOOD"},
	{FRAGGLE_ATTACK,"FRAGGLE_ATTACK"},
	{ICMP_DEATH_PING,"ICMP_DEATH_PING"},  		
	{ICMP_LARGE_PING,"ICMP_LARGE_PING"}, 
	{ICMP_ECHO_FLOODING,"ICMP_ECHO_FLOODING"},  	
	{ICMP_FLOOD,"ICMP_FLOOD"},
	{ICMP_SMURF_ATTACK,"ICMP_SMURF_ATTACK"},
	{ICMP_IGMP_FLOOD,"ICMP_IGMP_FLOOD"},		
	{ICMP_FORGE_SRC_ATTACK,"ICMP_FORGE_SRC_ATTACK"},   
	{ICMP_TERMINAL_EXIST_DETECT,"ICMP_TERMINAL_EXIST_DETECT"},	   
	{IGMP_FLOODING,"IGMP_FLOODING"},		
	{ARP_ATTACK_0,"ARP_ATTACK_0"},			
	{ARP_ATTACK_1,"ARP_ATTACK_1"},
	{ARP_ATTACK_2,"ARP_ATTACK_2"},
	{IP_PACK_WITH_OPTION,"IP_PACK_WITH_OPTION"},
	{IP_PACK_WITH_TIMESTAMP,"IP_PACK_WITH_TIMESTAMP"},
	{IP_PACK_WITH_RECORD_TRACE,"IP_PACK_WITH_RECORD_TRACE"},
	{IP_PACK_WITH_EOL,"IP_PACK_WITH_EOL"},			
	{IP_PACK_WITH_SATID,"IP_PACK_WITH_SATID"},
};

/**
 * @name:   findkeymap
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
static void findkeymap(int key,char* retvalue){
	for(int i= 0;i < NETWORKMONITORPAIR;i ++)
		if(networkmonitorpair[i].key == key){
			memcpy(retvalue,networkmonitorpair[i].url,strlen(networkmonitorpair[i].url));
			return;
		}
	memcpy(retvalue,"monitor no event\n",strlen("monitor no event\n"));
}

// 查询网卡存在
static int watchNicDeviceIsExist(char *watchNicDevice)
{
	int ret = 0;
	struct ifaddrs *addr = NULL;
	struct ifaddrs *temp_addr = NULL;
	int nic_exist = 0;

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
				if(strcmp(temp_addr->ifa_name, watchNicDevice)  == 0) 
				{
					nic_exist = 1;
				}
			}
			temp_addr = temp_addr->ifa_next;
		}
	}
	freeifaddrs(addr);

	return nic_exist;
}

static void timerarrived(void){
	char log_buff[64] = {0};

	snprintf(log_buff, sizeof(log_buff), "go to find watch nic device:%s", s_watchNicDevice);
	log_i("networkmonitor", log_buff);
	if(watchNicDeviceIsExist(s_watchNicDevice))
	{
		log_i("networkmonitor","watch nic device is exist");
		StartMonitor(s_watchNicDevice);
		if(startupFlowFalg)
		{
			flowmoduleinit(s_watchNicDevice);
		}

		if(timernetworkmonitor)
		{
			timerObj.freetimer(timernetworkmonitor);
			timernetworkmonitor = NULL;
		}
	}
}

static void createtimer(void){
	if(!timernetworkmonitor)
		timernetworkmonitor = timerObj.newtime(timerarrived);
}


static void freeliststack(list* _inputlist){
	list_destroy(_inputlist);
	list_init(_inputlist,free);
	free(_inputlist);
}

static bool addIpWhiteList(char* ip) {
	uint8_t length = 0;
	uint8_t code[256] = {0};
	if(addliststring(mIpWhiteList,ip)){
		return true;
	}
	return false;
}

static bool removeIpWhiteList(char* ip) {
	uint8_t length = 0;
	uint8_t code[256] = {0};

	if(delliststring(mIpWhiteList,ip)){
	  return true;
	}
	return false;
}

void updateIpWhiteList(char* whiteIpList)
{
	cJSON *cJSONList = NULL;
	int size = 0;

	if (!whiteIpList)
	{
		return;
	}

	cJSONList = cJSON_Parse(whiteIpList);
	if (!cJSONList)
	{
		return;
	}

	size =cJSON_GetArraySize(cJSONList);
	if (size == 0)
	{
		cJSON_Delete(cJSONList);
		return;
	}
	for (int i = 0; i < size; i ++) 
	{
		cJSON *child = cJSON_GetArrayItem(cJSONList, i);
		char*  ip_str = cJSON_GetObjectItem(child,"ip")->valuestring;
		addIpWhiteList(ip_str);
	}
	cJSON_Delete(cJSONList);
}

static bool addDNSWhiteList(char* ip) {
	uint8_t length = 0;
	uint8_t code[256] = {0};
	if(addliststring(mDNSWhiteList,ip)){
		return true;
	}
	return false;
}

static bool removeDNSWhiteList(char* dns) {
	uint8_t length = 0;
	uint8_t code[256] = {0};

	if(delliststring(mDNSWhiteList,dns)){
	  return true;
	}
	return false;
}

void updateDNSWhiteList(char* whiteDNSList)
{
	cJSON *cJSONList = NULL;
	int size = 0;

	if (!whiteDNSList)
	{
		return;
	}

	cJSONList = cJSON_Parse(whiteDNSList);
	if (!cJSONList)
	{
		return;
	}

	size =cJSON_GetArraySize(cJSONList);
	if (size == 0)
	{
		cJSON_Delete(cJSONList);
		return;
	}
	for (int i = 0; i < size; i ++) 
	{
		cJSON *child = cJSON_GetArrayItem(cJSONList, i);
		char*  dns_str = cJSON_GetObjectItem(child,"name")->valuestring;
		addDNSWhiteList(dns_str);
	}
	cJSON_Delete(cJSONList);
}


static void initWhiteList(void)
{
	list *head_ip = NULL;
	list *head_dns = NULL;

	head_ip = (list *)malloc(sizeof(list));
	if (head_ip == NULL){  
		log_e("networkmonitor","fatal error,Out Of Space head_ip");
		return;
	}
	list_init(head_ip,free);

	mIpWhiteList = head_ip;
	ipWhiteCheckInit(mIpWhiteList);

	head_dns = (list *)malloc(sizeof(list));
	if (head_dns == NULL){  
		log_e("networkmonitor","fatal error,Out Of Space head_dns");
		return;
	}
	list_init(head_dns,free);

	mDNSWhiteList = head_dns;
	DNSWhiteCheckInit(mDNSWhiteList);

}

void updatePortWhiteList(char* portIpList)
{
	cJSON *cJSONList = NULL;
	int size = 0;

	if (!portIpList)
	{
		log_e("networkmonitor","portIpList is null!");
		return;
	}

	cJSONList = cJSON_Parse(portIpList);
	if (!cJSONList)
	{
		return;
	}

	size =cJSON_GetArraySize(cJSONList);
	if (size == 0)
	{
		cJSON_Delete(cJSONList);
		return;
	}
	for (int i = 0; i < size; i ++) 
	{
		cJSON *child = cJSON_GetArrayItem(cJSONList, i);
		int port = cJSON_GetObjectItem(child,"port")->valueint;
		append_netport_list(port, 1);
	}
	cJSON_Delete(cJSONList);
}

// 附加功能
void getIMEI_Base(char* _input,int maxlength) {
	return GetIMEI(_input,maxlength);
}

// 打印数据 on 1:打印开启，0，打印关闭
void printNetEventformatted(char *title, char *content, int on)
{
	if(content && title && on)
	{
		cJSON* js_content    = cJSON_Parse(content);
		char*  print_content = cJSON_Print(js_content);
		printf("%s:%s\n",title, print_content);
		free(print_content);
		cJSON_Delete(js_content);
	}
}

// 1、网络流量数据上报
__attribute__((unused)) static void onFlowDataReport(char* data) {
	printNetEventformatted("[Flow Event]",data, false);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_FLOW, data);
}

// 2、网络攻击上报
__attribute__((unused)) static void onNetEventReport(int eventId, char* srcIp, int localPort, s8 *net_info) {
	char l_event[48] = {0};
	long long timestamp = clockobj.get_current_time();
	findkeymap(eventId,l_event);
	cJSON *cjson_data=NULL;
    cjson_data = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_data,"event",l_event);
	cJSON_AddStringToObject(cjson_data,"srcIp",srcIp);
	cJSON_AddNumberToObject(cjson_data,"localPort",localPort);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddStringToObject(cjson_data,"netInfo",net_info);
    char *s = cJSON_PrintUnformatted(cjson_data);
	//printNetEventformatted("[Attack event]",s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_ATTACK, s);
	if(s)free(s);
}

// 3、DNS查询上报
__attribute__((unused)) static void onDnsInquireEvent(char* dns)
{
	long long timestamp = clockobj.get_current_time();
	cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_data,"dns",dns);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
    char *s = cJSON_PrintUnformatted(cjson_data);
	//printNetEventformatted("[DNS event]",s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_DNS_INQUIRE, s);
	if(s)free(s);
}

// 4、DNS响应上报
__attribute__((unused)) static void onDnsResponseEvent(char* dns, char* ip_list) {
	long long timestamp = clockobj.get_current_time();
	cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_data,"dns",dns);
	cJSON_AddStringToObject(cjson_data,"ip_list",ip_list);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
    char *s = cJSON_PrintUnformatted(cjson_data);
	//printNetEventformatted("[DNS event]",s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_DNS_RESPONSE, s);
	if(s)free(s);
}

// 5、端口打开上报
__attribute__((unused)) static void onPortopenEvent(unsigned int port, char* uid)
{
	long long timestamp = clockobj.get_current_time();
	cJSON *cjson_data = cJSON_CreateObject();
	int inuid = atoi(uid);
	cJSON_AddNumberToObject(cjson_data,"uid",inuid);
	cJSON_AddNumberToObject(cjson_data,"pid",0);
	cJSON_AddStringToObject(cjson_data,"path","");
	cJSON_AddNumberToObject(cjson_data,"port",port);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
    char *s = cJSON_PrintUnformatted(cjson_data);
	//printNetEventformatted("[PORT event]",s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_PORT_OPEN, s);
	if(s)free(s);
}

// 6、Ip连接上报
__attribute__((unused)) static void onIpConnectEvent(int ip_version,char* srcIp, int srcPort,char* desIp, int desPort, int protocol)
{
	long long timestamp = clockobj.get_current_time();
	cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddNumberToObject(cjson_data,"uid",0);
	cJSON_AddNumberToObject(cjson_data,"pid",0);
	cJSON_AddStringToObject(cjson_data,"path","");
	cJSON_AddNumberToObject(cjson_data,"version",ip_version);
	cJSON_AddStringToObject(cjson_data,"local_ip",srcIp);
	cJSON_AddNumberToObject(cjson_data,"local_port",srcPort);
	cJSON_AddStringToObject(cjson_data,"remote_ip",desIp);
	cJSON_AddNumberToObject(cjson_data,"remote_port",desPort);
	cJSON_AddNumberToObject(cjson_data,"protocol",protocol);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
    char *s = cJSON_PrintUnformatted(cjson_data);
	//printNetEventformatted("[IP event]",s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_IP_CONNECT, s);
	if(s)free(s);
}

// 7、Tcp连接上报
__attribute__((unused)) static void onTcpConnectEvent(char* srcIp, int srcPort,char* desIp, int desPort)
{
	long long timestamp = clockobj.get_current_time();
	cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddNumberToObject(cjson_data,"uid",0);
	cJSON_AddNumberToObject(cjson_data,"pid",0);
	cJSON_AddStringToObject(cjson_data,"path","");
	cJSON_AddStringToObject(cjson_data,"local_ip",srcIp);
	cJSON_AddNumberToObject(cjson_data,"local_port",srcPort);
	cJSON_AddStringToObject(cjson_data,"remote_ip",desIp);
	cJSON_AddNumberToObject(cjson_data,"remote_port",desPort);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
    char *s = cJSON_PrintUnformatted(cjson_data);
	//printNetEventformatted("[TCP event]",s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_TCP_CONNECT, s);
	if(s)free(s);
}

// 8、UDP连接上报
__attribute__((unused)) static void onUdpConnectEvent(char* srcIp, int srcPort,char* desIp, int desPort)
{
	long long timestamp = clockobj.get_current_time();
	cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddNumberToObject(cjson_data,"uid",0);
	cJSON_AddNumberToObject(cjson_data,"pid",0);
	cJSON_AddStringToObject(cjson_data,"path","");
	cJSON_AddStringToObject(cjson_data,"local_ip",srcIp);
	cJSON_AddNumberToObject(cjson_data,"local_port",srcPort);
	cJSON_AddStringToObject(cjson_data,"remote_ip",desIp);
	cJSON_AddNumberToObject(cjson_data,"remote_port",desPort);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
    char *s = cJSON_PrintUnformatted(cjson_data);
	//printNetEventformatted("[UDP event]",s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_NETWORK_UDP_CONNECT, s);
	if(s)free(s);
}

//8、用户登陆失败事件上报
__attribute__((unused)) static void onUserLoginEvent(char* loginAddress)
{
    cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_data,"user_name","ftp");
	cJSON_AddStringToObject(cjson_data,"login_address",loginAddress);
	cJSON_AddNumberToObject(cjson_data,"login_type",8);
	cJSON_AddNumberToObject(cjson_data,"timestamp",clockobj.get_current_time());
    char *s = cJSON_PrintUnformatted(cjson_data);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_USER_LOGIN, s);
	if(s)free(s);
}

// 解析攻击种类参数
void updateNetAttackEvent(char* config, bool on)
{
	if(config && on)
	{
		cJSON *root = cJSON_Parse(config);
		if(root)
		{
			int size = cJSON_GetArraySize(root);
			for(int i=0; i< size; i++){
				cJSON* child = cJSON_GetArrayItem(root, i);
				if(!child) {
					log_e("networkmonitor","Parse Attacklist element  error");
					break;
				}
					
				cJSON* js_type   = cJSON_GetObjectItem(child, "type");
				cJSON* js_switch = cJSON_GetObjectItem(child, "switch");
				if(!js_type || !js_switch){
					log_e("networkmonitor","Parse Attacklist element type error");
					break;
				}
					
				for(int j=0; j<NETWORKMONITORPAIR; j++)
				{
					if(strcmp(networkmonitorpair[j].url, js_type->valuestring) == 0){
						setNetAttackSwitch(networkmonitorpair[j].key, js_switch->valueint);
						break;
					}
				}
			}
			cJSON_Delete(root);
		}
		else{
			log_e("networkmonitor","Parse Attacklist parameter list error");
		}
	}
}

// 各个网络攻击阈值设置
void setNetAttackThreshold(char *attackThreshold)
{
	cJSON *attackThreshold_json = NULL;
	cJSON * tmp_json = NULL;
	int value = 0;
	int i =0;
	char attackkey[][24]= {"ARPFLOOD","ARPATTACK", "ICMPLOOD", "ICMPDEATH", "ICMPLARGE", "IGMPFLOOD",
							"IMPLATTCK", "TCPSCANRET", "TCPDOSRET", "TCPPORTS", "TCPCONNECT", "UDPDOSRET",
							"UDPPORTS", "UDPFRAGGLE"};

	if (!attackThreshold)
	{
		return;
	}

	attackThreshold_json = cJSON_Parse(attackThreshold);
	if (attackThreshold_json)
	{
		for(i = 0; i < sizeof(attackkey)/sizeof(attackkey[0]); i++)
		{
			tmp_json = cJSON_GetObjectItem(attackThreshold_json, attackkey[i]);
			if (tmp_json)
			{
				if (cJSON_IsNumber(tmp_json))
				{
					value = tmp_json->valueint;
					switchNetAttackThreshold(attackkey[i], value);
				}
			}
		}

		cJSON_Delete(attackThreshold_json);
	}
	else
	{
		log_e("networkmonitor","Parse Attack Threshold parameter list error");
	}
}

// 解析更新流量配置
void updateNetFlowEvent(int interval, bool on)
{
	startupFlowFalg = on;
	setflowinterval(interval);
}

// 设置回调，开启数据上报
static inline void LoadNetWorkMonitor(bool connectSwitch)
{
 	invokefunction callinsert; //回调
	memset(&callinsert,0,sizeof(invokefunction));
	//callback function  register
	callinsert.onFlowDataReport 	  = onFlowDataReport;
	callinsert.onNetEventReport       = onNetEventReport;
	if (connectSwitch)
	{
		callinsert.onIpConnectEvent       = onIpConnectEvent;
		callinsert.onTcpConnectEvent      = onTcpConnectEvent;
		callinsert.onUdpConnectEvent      = onUdpConnectEvent;
		callinsert.onDnsInquireEvent      = onDnsInquireEvent;
		callinsert.onDnsResponseEvent     = onDnsResponseEvent;
		callinsert.onPortOpenEvent        = onPortopenEvent;
	}
	callinsert.onUserLoginEvent       = onUserLoginEvent;
	start_loadinit(&callinsert);
}

// 获取网卡流量数据
void getTrafficUsageInfo_Base(char* _input,int maxlength){
    return getTrafficUsageInfo(_input,maxlength);
}

// 网卡流量数据记录
void setSnifferFilePath(char* filePath){
	if(filePath == NULL)
		log_i("networkmonitor","incorrect parameter");
	SetSnifferFilePath(filePath);
}

void startSniffer_Base(void)
{
	StartSniffer();
}

void stopSniffer_Base(void)
{
	StopSniffer();
}

// 建立监测
void newNetworkMonitor(char *watchNicDevicePolicy, char *watchNicDeviceBase, bool attackSwitch, char* attackList, char* attackThreshold,
							bool flowSwitch, int flowInterval, bool connectSwitch, int connectInterval)
{
	if (strlen(watchNicDevicePolicy) <= 0)
	{
		strncpy(s_watchNicDevice, watchNicDeviceBase, sizeof(s_watchNicDevice) - 1);
	}
	else
	{
		strncpy(s_watchNicDevice, watchNicDevicePolicy, sizeof(s_watchNicDevice) - 1);
	}

	LoadNetWorkMonitor(connectSwitch);
	updateNetConnectReportInterval(connectInterval);
	initWhiteList();
	createtimer();
	setNetAttackThreshold(attackThreshold);
	updateNetAttackEvent(attackList, attackSwitch);
	updateNetFlowEvent(flowInterval, flowSwitch);	
}

// 启动数据监测监测
void startNetWorkMonitor(int _interval){
	if(timernetworkmonitor)
	{
	 	timerObj.setInterval(_interval,timernetworkmonitor);
	 	timerObj.starttime(timernetworkmonitor);
	}	
}

// 停止监测
void stopNetworkMonitor(void){
	 if(timernetworkmonitor)
	 	timerObj.stoptimer(timernetworkmonitor);
}

void freeNetWorkMonitor(void){
	StopMonitor();
	if (mIpWhiteList)
	{
		freeliststack(mIpWhiteList);
		mIpWhiteList = NULL;
	}

	if (mDNSWhiteList)
	{
		freeliststack(mDNSWhiteList);
		mDNSWhiteList = NULL;
	}

	if (timernetworkmonitor)
	{
		timerObj.freetimer(timernetworkmonitor);
		timernetworkmonitor = NULL;
	}
}

NetWorkMonitorMethod NetWorkMonitorMethodObj = {
    newNetworkMonitor,
	startNetWorkMonitor,
	stopNetworkMonitor,
	freeNetWorkMonitor,
    getTrafficUsageInfo_Base,
	setSnifferFilePath,
	startSniffer_Base,
	stopSniffer_Base,
	updateNetAttackEvent,
	updateNetFlowEvent,
	updateIpWhiteList,
	updatePortWhiteList,
	updateDNSWhiteList,
};
#endif
