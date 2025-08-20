#include "api_networkmonitor.h"
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "dpi_report.h"
#include "spdloglib.h"
#include "fireinterface.h"
#include "data_dispatcher.h"


 /*
 * decla   :(0)本地回调函数的定义
 */
invokefunction  callbackfunction;
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
 void start_loadinit(void* _struct){
	// 回调函数构造
	memcpy(&callbackfunction,(invokefunction*)_struct,sizeof(invokefunction));
	// 启动网络攻击结果上报线程
	startlog();
	//添加待添加的网络IP分段情况
	//addmoduledevice("192.168.169.114");
 }
 /*
 * function:StartMonitor
 * input   :void
 * output  :void
 * decla   :(0)开始网络pcap抓取数据
 */
void StartMonitor(char *watchNicDevice){
	// 开始网络pcap抓取数据,和数据分析分类
	monitor_start(watchNicDevice);
}
 /*
 * function:StopMonitor
 * input   :void
 * output  :void
 * decla   :(0)停止网络监控
 */
void StopMonitor(void){
	monitor_stop();
	dpi_report_log_free();
}
// pacp start
void monitor_start(s8 *if_name)
{
	data_dispatcher_init(if_name);
}
// pacp stops
void monitor_stop()
{
	stop_pcap();
}

 /*
 * function:SetSnifferFilePath
 * input   :void
 * output  :void
 * decla   :(0)设置pcap文件的保存位置
 */
void SetSnifferFilePath(char* path){
	if(path != NULL)
		set_store_path(path);
}
 /*
 * function:StartSniffer
 * input   :void
 * output  :void
 * decla   :(0)pcap文件数据开始保存
 */
void StartSniffer(void){
	sniffer_start();
}
 /*
 * function:StopSniffer
 * input   :void
 * output  :void
 * decla   :(0)pcap文件数据保存停止
 */
void StopSniffer(void){
	sniffer_stop();
}
 /*
 * function:GetIMEI
 * input   :imeistring:待保存数据的数据指针 imeistring_maxsize:该指针的的最大容量,防止数组溢出 
 * output  :void 
 * decla   :获取imei号
 */
void GetIMEI(char* imeistring,int imeistring_maxsize){
	char imei_array[64]={0};
	//get_device_sn(imei_array,sizeof(imei_array));
	char buff[128]={0},buf[128] = {0};
    FILE* fp;
	fp=popen("tail /etc/config/system.ini -n 1","r"); 
	fread(buff,1,127,fp);//将fp的数据流读到buff中   
	sscanf(buff,"serial=%s",buf);
	pclose(fp); 
	strncpy(imeistring,buf,imeistring_maxsize);
}
 /*
 * function:NetworkInterfaceInfo
 * input   :outstring:待保存数据的数据指针 out_string_maxlen:该指针的的最大容量,防止数组溢出 
 * output  :void 
 * decla   :获取当前网卡信息
 */
void NetworkInterfaceInfo(char* outstring,int out_string_maxlen){
	char *s = getnetinfoforpcap();
	strncpy(outstring,s,out_string_maxlen);
	free(s);
}
 /*
 * function:getTrafficUsageInfo
 * input   :outstring:待保存数据的数据指针 out_string_maxlen:该指针的的最大容量,防止数组溢出 
 * output  :void 
 * decla   :获取当前网卡的TX　RX信息
 */
void getTrafficUsageInfo(char* outstring,int out_string_maxlen){
	char *s = getnettxrx();
	strncpy(outstring,s,out_string_maxlen);
	free(s);
}

// 回调函数，底层调用，网络流量上报
void on_FlowDataReport_callback(char* data){
	if(data != NULL && callbackfunction.onFlowDataReport){
		callbackfunction.onFlowDataReport(data);
	}
}
// 回调函数，底层调用，网络攻击上报
void on_NetEventReport_callback(int event_id,char *src_ip,int src_port, s8 *net_info)
{
	if(callbackfunction.onNetEventReport != NULL){
		callbackfunction.onNetEventReport(event_id,src_ip,src_port, net_info);
	}
}
// 回调函数，底层调用，IP连接上报
void on_IpConnectEvent_callback(int ip_version,char* srcIp, int srcPort,char* desIp, int desPort, int protocol)
{
	if(callbackfunction.onIpConnectEvent){
		callbackfunction.onIpConnectEvent(ip_version, srcIp, srcPort, desIp, desPort, protocol);
	}
}
// 回调函数，底层调用，TCP连接上报
void on_TcpConnectEvent_callback(char* srcIp, int srcPort,char* desIp, int desPort)
{
	if(callbackfunction.onTcpConnectEvent){
		callbackfunction.onTcpConnectEvent(srcIp, srcPort, desIp, desPort);
	}
}
// 回调函数，底层调用，UDP连接上报
void on_UdpConnectEvent_callback(char* srcIp, int srcPort,char* desIp, int desPort)
{
	if(callbackfunction.onUdpConnectEvent){
		callbackfunction.onUdpConnectEvent(srcIp, srcPort, desIp, desPort);
	}
}
// 回调函数，底层调用，DNS查询上报
void on_onDnsInquireEvent_callback(char* dns)
{
	if(callbackfunction.onDnsInquireEvent){
		callbackfunction.onDnsInquireEvent(dns);
	}
}
// 回调函数，底层调用，DNS响应上报
void on_onDnsResponseEvent_callback(char* dns, char* ip_list)
{
	if(callbackfunction.onDnsResponseEvent){
		callbackfunction.onDnsResponseEvent(dns, ip_list);
	}
}
// 回调函数，底层调用，端口开启上报
void on_onPortOpenEvent_callback(unsigned int port, char* uid)
{
	if(callbackfunction.onPortOpenEvent){
		callbackfunction.onPortOpenEvent(port, uid);
	}
}

void on_onUserLoginEvent_callback(char *address)
{
	if(callbackfunction.onUserLoginEvent){
		callbackfunction.onUserLoginEvent(address);
	}
}
 /*
 * function:change_IDPS_Mode
 * input   :如参数名所示 
 * output  :void 
 * decla   :
 */
void change_IDPS_Mode(int mode){
	modeIDPS = mode;
}

// 各个网络阈值设置分析和阈值记录，对比关键字进行设置
void switchNetAttackThreshold(char* key, int value)
{
	printf("networkmonitor Attack match key:%s %d\n",key, value);
	if (value <= 0)
	{
		return;
	}

	if (strncmp(key, "ARPFLOOD", strlen("ARPFLOOD")) == 0)
	{
		setArpFloodThreshold(value);
	}
	else if (strncmp(key, "ARPATTACK", strlen("ARPATTACK")) == 0)
	{
		setArpAttackThreshold(value);
	}
	else if (strncmp(key, "ICMPLOOD", strlen("ICMPLOOD")) == 0)
	{
		setIcmpFloodThreshold(value);
	}
	else if (strncmp(key, "ICMPDEATH", strlen("ICMPDEATH")) == 0)
	{
		setDeathPingSizeThreshold(value);
	}
	else if (strncmp(key, "ICMPLARGE", strlen("ICMPLARGE")) == 0)
	{
		setLargePingSizeThreshold(value);
	}
	else if (strncmp(key, "IGMPFLOOD", strlen("IGMPFLOOD")) == 0)
	{
		setIgmpFloodThreshold(value);
	}
	else if (strncmp(key, "IMPLATTCK", strlen("IMPLATTCK")) == 0)
	{
		setTcpConnectAttachThreshold(value);
	}
	else if (strncmp(key, "TCPSCANRET", strlen("TCPSCANRET")) == 0)
	{
		setTcpScanConnectRetrytimesThreshold(value);
	}
	else if (strncmp(key, "TCPDOSRET", strlen("TCPDOSRET")) == 0)
	{
		setTcpDosRetryTimesThreshold(value);
	}
	else if (strncmp(key, "TCPPORTS", strlen("TCPPORTS")) == 0)
	{
		setTcpPortsPerSecThreshold(value);
	}
	else if (strncmp(key, "TCPCONNECT", strlen("TCPCONNECT")) == 0)
	{
		setTcpConnectOrScanWeight((float)value/100);
	}
	else if (strncmp(key, "UDPDOSRET", strlen("UDPDOSRET")) == 0)
	{
		setUdpDosRetrytimesThreshold(value);
	}
	else if (strncmp(key, "UDPPORTS", strlen("UDPPORTS")) == 0)
	{
		setudpPortsPerSecThreshold(value);
	}
	else if (strncmp(key, "UDPFRAGGLE", strlen("UDPFRAGGLE")) == 0)
	{
		setfraggleAttemptPerSecThreshold(value);
	}
	// 是否记录攻击值
	else if (strncmp(key, "THRESHOLDLOG", strlen("THRESHOLDLOG")) == 0)
	{
		thresholdLog = value;
	}
}

// 转小写转大写
static void strlwr(char *str)
{
	char* p = str;
	while(*p!='\0')
	{
		if(*p>='a'&&*p<='z'){
			*p=*p-32;
		}
		p++;
	}
}

// 各个网络攻击阈值设置
void setNetAttackThreshold1(){
	char key[16] = {0}; 
	int  value = 0;
	char data[100];

	/*
	char s[1000]={0};
	NetworkInterfaceInfo(s, 1000);
	printf("%s\n", s);*/

	FILE* fp = fopen("./addconfigure", "r");
	if(fp)
	{
		log_i("networkmonitor Attack", "addconfigure read...");
		while(!feof(fp))
		{
			fscanf(fp,"%s",data);
			log_i("networkmonitor Attack", data);
			value = atoi(data);
			if(value == 0)
			{
				strlwr(data);
				strncpy(key,data,16);
			}
			else{
				switchNetAttackThreshold(key, value);
				value = 0;
			}
		}
		fclose(fp);
	}
}

// 各个攻击种类开关设置
void setNetAttackSwitch(int index, int para){
	setNetEventReportSwitch(index, para);
}