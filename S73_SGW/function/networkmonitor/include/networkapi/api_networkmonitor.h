#ifndef __API_NETWORKMONITOR_
#define __API_NETWORKMONITOR_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include "typedef.h"
#include "util.h"
#include "firewallload.h"
#define   WHITEDEVICE   NULL
#define   BLACKDEVICE   "eth0" 
#define   DNATDEVICE    NULL


#define		LINUX_COOKED_CAPTURE 						0
#define 	BLACK_LIST_CHECK_OPTION						1
/**
 * decla :基本回调函数的声明 
 * notify:请注意参数类型的一致性
 */
typedef struct _invokefunction{
	void (*onFlowDataReport)(char* data);
	void (*onNetEventReport)(int event_id,char *src_ip,int src_port, s8 *net_info);
	void (*onPortOpenEvent)(unsigned int port, char* uid);
	void (*onDnsInquireEvent)(char* dns);
	void (*onDnsResponseEvent)(char* dns, char* ip_list);
	void (*onIpConnectEvent)(int ip_version,char *src_ip,int src_port,char *dst_ip,int dst_port,int protocol);
	void (*onTcpConnectEvent)(char* srcIp, int srcPort,char* desIp, int desPort);
	void (*onUdpConnectEvent)(char* srcIp, int srcPort,char* desIp, int desPort);
	void (*onUserLoginEvent)(char* loginAddress);
}invokefunction;

/*
 * function:IDS IPS IDPS模式切换
 * input   :mode:  0:IDS 1:IPS 2:IDPS
 * output  :void 
 * decla   :
 */
#define IDS    0
#define IPS    1
/*
 * function:IDS IPS IDPS模式切换
 * input   :mode:  0:IDS 1:IPS 
 * output  :void 
 * decla   :
 */
void change_IDPS_Mode(int mode);

// 1、底层函数暴露给上层调用
char *get_monitor_mac(void);

// 2、供上层调用函数 接口区
 /*
 * function:start_loadinit
 * input   :void
 * output  :void
 * decla   :(0)本地链表的初始化
 *          (1)常用回调函数入栈操作
 *             @1：onIpDataAvailable
 *             @2：onNetEventReport
 *             @3：onBlackIpConnectEvent
 *             @4：onBlackDnsConnectEvent
 *             @5：onwhiteIpConnectEvent
 *             @6：onwhiteDnsConnectEvent
 */
 void start_loadinit(void* _struct);
 /*
 * function:StartMonitor
 * input   :void
 * output  :void
 * decla   :(0)开始网络pcap抓取数据
 */
void StartMonitor(char *watchNicDevice);

// pacp抓包启动
void monitor_start(s8* if_name);

// pacp抓包停止
void monitor_stop(void);
 /*
 * function:StopMonitor
 * input   :void
 * output  :void
 * decla   :(0)停止网络监控
 */
void StopMonitor(void);
 /*
 * function:SetSnifferFilePath
 * input   :void
 * output  :void
 * decla   :(0)设置pcap文件的保存位置
 */
void SetSnifferFilePath(char* path);
 /*
 * function:StartSniffer
 * input   :void
 * output  :void
 * decla   :(0)pcap文件数据开始保存
 */
void StartSniffer(void);
 /*
 * function:StopSniffer
 * input   :void
 * output  :void
 * decla   :(0)pcap文件数据保存停止
 */
void StopSniffer(void);
 /*
 * function:GetIMEI
 * input   :imeistring:待保存数据的数据指针 imeistring_maxsize:该指针的的最大容量,防止数组溢出 
 * output  :void 
 * decla   :获取imei号
 */
void GetIMEI(char* imeistring,int imeistring_maxsize);
 /*
 * function:NetworkInterfaceInfo
 * input   :outstring:待保存数据的数据指针 out_string_maxlen:该指针的的最大容量,防止数组溢出 
 * output  :void 
 * decla   :获取当前网卡信息
 */
void NetworkInterfaceInfo(char* outstring,int out_string_maxlen);
 /*
 * function:getTrafficUsageInfo
 * input   :outstring:待保存数据的数据指针 out_string_maxlen:该指针的的最大容量,防止数组溢出 
 * output  :void 
 * decla   :获取当前网卡的TX　RX信息
 */
void getTrafficUsageInfo(char* outstring,int out_string_maxlen);

// 3、本层函数给底层调用
/****************回调函数*********************/
 /*
 * function:on_IpDataRec_callback
 * input   :如参数名所示 
 * output  :void 
 * decla   :
 */
void on_FlowDataReport_callback(char* data);

void on_NetEventReport_callback(int event_id,char *src_ip,int src_port, s8 *net_info);

void on_IpConnectEvent_callback(int ip_version,char* srcIp, int srcPort,char* desIp, int desPort, int protocol);

void on_TcpConnectEvent_callback(char* srcIp, int srcPort,char* desIp, int desPort);

void on_UdpConnectEvent_callback(char* srcIp, int srcPort,char* desIp, int desPort);

void on_onDnsInquireEvent_callback(char* dns);

void on_onDnsResponseEvent_callback(char* dns, char* ip_list);

void on_onPortOpenEvent_callback(unsigned int port, char* uid);

void on_onUserLoginEvent_callback(char *address);

// 阈值设置，包含底层函数接口，底层函数给本层使用
// arp参数设置
void setArpFloodThreshold(long para);
void setArpAttackThreshold(long para);
void setIcmpFloodThreshold(long para);
// icmp参数设置
void setDeathPingSizeThreshold(long para);
void setLargePingSizeThreshold(long para);
// igmp参数设置
void setIgmpFloodThreshold(long para);
// impl参数设置
void setTcpConnectAttachThreshold (long para);
// tcp参数设置
void setTcpScanConnectRetrytimesThreshold(long para);
void setTcpDosRetryTimesThreshold(long para);
void setTcpPortsPerSecThreshold(long para);
void setTcpConnectOrScanWeight(float para);
// udp参数设置
void setUdpDosRetrytimesThreshold(long para);
void setudpPortsPerSecThreshold(long para);
void setfraggleAttemptPerSecThreshold(long para);
// 设置上报开关参数
void setNetEventReportSwitch(int index, int para);

// 各个网络阈值设置分析和阈值记录，对比关键字进行设置
void switchNetAttackThreshold(char* key, int value);

// 各个攻击种类开关设置
void setNetAttackSwitch(int index, int para);

#ifdef __cplusplus
}
#endif
#endif
