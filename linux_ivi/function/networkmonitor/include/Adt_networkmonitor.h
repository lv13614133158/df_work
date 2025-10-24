/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 2020-05-28 03:20:25
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-05-09 08:47:06
 */ 
#ifndef __ADT_NETWORKMONITOR_
#define __ADT_NETWORKMONITOR_
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"//for list
#include "idps_main.h"

#if MODULE_NETWORKMONITOR
/**
 * @name:   DNAT format
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
typedef struct _dnatstruct{
	char dstip[32];
	char redirectIP[32];
}dnatstruct;
/**
 * @name:   NetworkConfig
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
typedef struct _NetworkConfig {
    int period;
    char* mobileIfaces;
    char* wifiIface;
    long long lastIpTimestamp;
    long long lastDnsTimeStamp;
	void (*NetworkConfigtoString)(struct _NetworkConfig *,char*,int);
}NetworkConfig;
/**
 * @name:   NetworkDnsData
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
typedef struct _NetworkDnsData {
    long long timestamp;
    char* processName;
    char* dns;
    char* destIp;
	void (*NetworkDnsDatatoString)(struct _NetworkDnsData *,char*,int);
}NetworkDnsData;
 /**
  * @name:  NetworkDnsUploadData
  * @Author: qihoo360
  * @msg: 
  * @param  
  * @return: 
  */
 typedef struct _NetworkDnsUploadData {
    long long timestamp;
    int num;
    char* processName;
    char* dns;
    char* destIp;
	void (*NetworkDnsUploadDatatoString)(struct _NetworkDnsUploadData *,char*,int);
}NetworkDnsUploadData;
/**
 * @name:   IfaceFlow
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
typedef struct _IfaceFlow{
	char *iface;
	long rx_bytes;
	long tx_bytes;
	long last_rx_bytes;
	long last_tx_bytes;
}IfaceFlow;
 /**
  * @name:   NetworkFlowData
  * @Author: qihoo360
  * @msg: 
  * @param {type} 
  * @return: 
  */
 typedef struct _NetworkFlowData{
	int   uid;
	list  head;
    void (*addBytes)(long,long,list_elmt *Node);
	struct{
		void (*addIfaceFlow)(char*,long,long,list*);
		void (*clearAllIfaceFlow)(list*);
		void (*initclearAllIfaceFlowlist)(list*);
	}listpro;
	void (*NetworkFlowDatatoString)(struct _NetworkFlowData*,char*,int);
 }NetworkFlowData;
/**
 * @name:   NetworkIpData
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */ 
typedef struct _NetworkIpData{
    long long timestamp;
    char* processName;
    int version;
    char* srcIp;
    int srcPort;
    char* destIp;
    int destPort;
    int protocol;
	void (*NetworkIpDatatoString)(struct _NetworkIpData*,char*,int);
}NetworkIpData;
/**
 * @name: NetworkIpUploadData
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
typedef struct _NetworkIpUploadData {
    long long timestamp;
    int num;
    char* processName;
    int version;
    char* srcIp;
    int srcPort;
    char* destIp;
    int destPort;
    int protocol;
	void (*NetworkIpUploadDatatoString)(struct _NetworkIpUploadData*,char*,int);
}NetworkIpUploadData;
/**
 * @name:   NetworkMonitorConfig 
 * @Author: qihoo360
 * @msg: 
 * @param    
 * @return: 
 */
typedef struct _NetworkMonitorConfig {
    int idpsmode;
    int period;
    char* mobileIfaces;
    char* wifiIface;
    /******space val******/
    long long lastidpsmodeTimestamp;
    long long lastIpTimestamp;
    long long lastDnsTimeStamp;
    long long lastUpdateBlackIpTimestamp;
    long long lastUpdateBlackPortTimestamp;
    long long lastUpdateBlackDnsTimestamp;
    long long lastUpdatefileTimestamp;
    long long lastUpdateBlackProcessTimestamp;
    long long lastUpdateBlackRedirectIpTimestamp;
	void (*NetworkMonitorConfigtoString)(struct _NetworkMonitorConfig*,char*,int);
}NetworkMonitorConfig;

void NetworkAdt_freeobjectNetworkMonitorConfig(NetworkMonitorConfig* _input);
NetworkMonitorConfig* NetworkAdt_newobjectNetworkMonitorConfig(void);
void NetworkAdt_freeobjectNetworkIpUploadData(NetworkIpUploadData* _input);
NetworkIpUploadData* NetworkAdt_newobjectNetworkIpUploadData(void);
NetworkIpData* NetworkAdt_newobjectNetworkIpData(void);
void NetworkAdt_freeobjectNetworkIpData(NetworkIpData* _input);
NetworkConfig* NetworkAdt_newobjectNetworkConfig(void);
void NetworkAdt_freeobjectNetworkConfig(NetworkConfig* _input);
void NetworkAdt_freeobjectNetworkDnsData(NetworkDnsData* _input);
NetworkDnsData* NetworkAdt_newobjectNetworkDnsData(void);
NetworkDnsUploadData* NetworkAdt_newobjectNetworkDnsUploadData(void);
void NetworkAdt_freeobjectNetworkDnsUploadData(NetworkDnsUploadData* _input);
NetworkFlowData* NetworkAdt_newobjectNetworkFlowData(void);
void NetworkAdt_freeobjectNetworkFlowData(NetworkFlowData* _input);

/**
 * @name:   Pair struct
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */

#endif 
#ifdef __cplusplus
}
#endif
#endif
