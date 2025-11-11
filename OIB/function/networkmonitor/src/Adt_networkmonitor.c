/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-03 03:37:44
 */ 
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "util.h"
#include "Adt_networkmonitor.h"
#if MODULE_NETWORKMONITOR
/**
 * @name:   Copytostack
 * @Author: qihoo360
 * @msg: 
 * @param :
 * @return: 
 */
#define Copytostack(l_string,_spacelen,_output){\
	char* ptr = NULL;\
	int l_len = strlen(l_string) + 1;\
	if(_spacelen < l_len){\
		if((ptr = realloc(_output, l_len)) == NULL)\
			return;\
		bzero(ptr,l_len);\
		_output = ptr;\
	}\
	memcpy(_output,l_string,l_len);\
}
/**
 * @name:   NetworkConfigtoString
 * @Author: qihoo360
 * @msg:    the string format: space may need modify
 * @param   _input:NetworkConfig struct _output:string __spacelen:max length _output
 * @return: void
 */
static void NetworkConfigtoString(NetworkConfig *_input,char* _output,int _spacelen){
	int  l_len = (strlen(_input->mobileIfaces) + strlen(_input->wifiIface) + 2) + 100;
	char l_string[l_len];
	memset(l_string,0,l_len);
	sprintf(l_string,"[NetworkConfig: %d,%s,%s,%lld,%lld]",\
	_input->period,_input->mobileIfaces,_input->wifiIface,_input->lastIpTimestamp,_input->lastDnsTimeStamp);
	Copytostack(l_string,_spacelen,_output);
}
/**
 * @name:   NetworkDnsDatatoString
 * @Author: qihoo360
 * @msg: 
 * @param   _input:NetworkConfig struct _output:string __spacelen:max length _output 
 * @return: 
 */
static void NetworkDnsDatatoString(NetworkDnsData *_input,char* _output,int _spacelen){
	int  l_len = (strlen(_input->processName) + strlen(_input->dns) + strlen(_input->destIp)+ 2) + 100;
	char l_string[l_len];
	memset(l_string,0,l_len);
	sprintf(l_string,"[NetworkDnsData: %lld,%s,%s,%s]",\
	_input->timestamp,_input->processName,_input->dns,_input->destIp);
	Copytostack(l_string,_spacelen,_output);
}
/**
 * @name:   NetworkDnsUploadDatatoString
 * @Author: qihoo360
 * @msg: 
 * @param _input:NetworkConfig struct _output:string __spacelen:max length _output
 * @return: 
 */
static void NetworkDnsUploadDatatoString(NetworkDnsUploadData *_input,char* _output,int _spacelen){
	int  l_len = (strlen(_input->processName) + strlen(_input->dns) + strlen(_input->destIp)+ 2) + 100;
	char l_string[l_len];
	memset(l_string,0,l_len);
	sprintf(l_string,"[NetworkDnsData: %lld,%d,%s,%s,%s]",\
	_input->timestamp,_input->num,_input->processName,_input->dns,_input->destIp);
	Copytostack(l_string,_spacelen,_output);
}
/**
 * @name:   addBytes
 * @Author: qihoo360
 * @msg:    NetworkFlowData  related function
 * @param {type} 
 * @return: 
 */ 
static void addBytes(long rbs,long tbx,list_elmt *Node){
	IfaceFlow *l_interface = Node->data;
	if(strlen(l_interface->iface) == 0)
		return;
	l_interface->last_rx_bytes += rbs;
	l_interface->last_tx_bytes += tbx;
}
/**
 * @name: addIfaceFlow
 * @Author: qihoo360
 * @msg:    NetworkFlowData  related function
 * @param   
 * @return: 
 */
static void addIfaceFlow(char* iface,long rxbytes,long txbytes,list *listhead){
	IfaceFlow *l_interfaceflow = NULL;
	if((l_interfaceflow = malloc(sizeof(IfaceFlow))) == NULL)
		return;
	l_interfaceflow->rx_bytes = rxbytes;
	l_interfaceflow->tx_bytes = txbytes;
	l_interfaceflow->iface = (char*)malloc(strlen(iface) + 2);
	memset(l_interfaceflow->iface,0,(strlen(iface) + 2));
	strcat(l_interfaceflow->iface,iface);
	list_ins_next(listhead,NULL,l_interfaceflow);	
}
 /**
  * @name:   clearAllIfaceFlow
  * @Author: qihoo360
  * @msg:    NetworkFlowData  related function
  * @param   
  * @return: 
  */
 static void clearAllIfaceFlow(list *interfacelist){
	list_elmt *cur_elmt = list_head(interfacelist);//free space
	while(cur_elmt != NULL)
	{
		IfaceFlow *l_interfaceflow = cur_elmt->data;
		if(l_interfaceflow->iface != NULL)
			free(l_interfaceflow->iface);
		cur_elmt = cur_elmt->next;
	}
	list_destroy(interfacelist); 
 }
 /**
  * @name:   initclearAllIfaceFlowlist
  * @Author: qihoo360
  * @msg:    NetworkFlowData  related function
  * @param {type} 
  * @return: 
  */
 static void initclearAllIfaceFlowlist(list *_list){
	destroy(_list);
	list_init(_list,free);
 }
 /**
  * @name:   NetworkFlowDatatoString
  * @Author: qihoo360
  * @msg:    NetworkFlowData  related function
  * @param 
  * @return: 
  */
static void NetworkFlowDatatoString(NetworkFlowData *_input,char* _output,int _spacelen){
	char l_tmpstring[248] = {0};
	int list_sizecount = list_size(&(_input->head));
	char l_string[248*list_sizecount];
	memset(l_string,0,248*list_sizecount);
	strcat(l_string,"[NetworkFlowData: \n");
	list_elmt *cur_elmt = list_head(&(_input->head));
	while(cur_elmt != NULL)
	{
		IfaceFlow *l_interfaceflow = cur_elmt->data;
		//to process interface string 
		memset(l_tmpstring,0,248);
		if(strlen(l_interfaceflow->iface) > 124){
			memset(l_interfaceflow->iface,0,strlen(l_interfaceflow->iface));
			memcpy(l_interfaceflow->iface,"interface name length > 124,please check devices",strlen("interface name length > 124,please check devices"));
		}
		snprintf(l_tmpstring,248,"%d,%s,%ld,%ld\n",_input->uid,l_interfaceflow->iface,l_interfaceflow->rx_bytes,l_interfaceflow->tx_bytes);
		strcat(l_string,l_tmpstring);
		cur_elmt = cur_elmt->next;
	}
	Copytostack(l_string,_spacelen,_output);
}

/**
 * @name:   NetworkIpDatatoString
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
static void NetworkIpDatatoString(NetworkIpData* _input,char* _output,int _spacelen){
	int  l_len = (strlen(_input->destIp) + strlen(_input->processName) + strlen(_input->srcIp)+ 2) + 100;
	char l_string[l_len];
	memset(l_string,0,l_len);
	sprintf(l_string,"[NetworkIpData:%lld,%s,%d,%s,%d,%s,%d,%d]",_input->timestamp,_input->processName,\
	_input->version,_input->srcIp,_input->srcPort,_input->destIp,_input->destPort,_input->protocol);
	Copytostack(l_string,_spacelen,_output);
}
/**
 * @name:   NetworkIpUploadDatatoString
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
static void NetworkIpUploadDatatoString(NetworkIpUploadData* _input,char* _output,int _spacelen){
	int  l_len = (strlen(_input->destIp) + strlen(_input->processName) + strlen(_input->srcIp)+ 2) + 100;
	char l_string[l_len];
	memset(l_string,0,l_len);
	sprintf(l_string,"[NetworkIpData:%lld,%d,%s,%d,%s,%d,%s,%d,%d]",_input->timestamp,_input->num,\
	_input->processName,_input->version,_input->srcIp,_input->srcPort,_input->destIp,_input->destPort,_input->protocol);
	Copytostack(l_string,_spacelen,_output);

}
/**
 * @name:   NetworkMonitorConfigtoString
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
static void NetworkMonitorConfigtoString(NetworkMonitorConfig* _input,char* _output,int _spacelen){
	int  l_len = (strlen(_input->mobileIfaces) + strlen(_input->wifiIface) + 2) + 100;
	char l_string[l_len];
	memset(l_string,0,l_len);
	sprintf(l_string,"[NetworkConfig:%d,%s,%s,%lld,%lld,%lld,%lld,%lld,%lld]",_input->period,_input->mobileIfaces,\
	_input->wifiIface,_input->lastIpTimestamp,_input->lastDnsTimeStamp,_input->lastUpdateBlackIpTimestamp,_input->lastUpdateBlackDnsTimestamp,_input->lastUpdateBlackProcessTimestamp,_input->lastUpdateBlackRedirectIpTimestamp);
	Copytostack(l_string,_spacelen,_output);
}
/******************************************************************************************************/
/**
 * @name:   freeobjectNetworkMonitorConfig
 * @Author: qihoo360
 * @msg: 
 * @param
 * @return: 
 */
void NetworkAdt_freeobjectNetworkMonitorConfig(NetworkMonitorConfig* _input){
	if(_input->wifiIface != NULL)
		free(_input->wifiIface);
	if(_input->mobileIfaces != NULL)
		free(_input->mobileIfaces);
	free(_input);
}
/**
 * @name:   newobjectNetworkMonitorConfig
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
NetworkMonitorConfig* NetworkAdt_newobjectNetworkMonitorConfig(void){
	NetworkMonitorConfig* g_NetworkMonitorConfig = (NetworkMonitorConfig*)malloc(sizeof(NetworkMonitorConfig));
	memset(g_NetworkMonitorConfig,0,sizeof(NetworkMonitorConfig));
	g_NetworkMonitorConfig->NetworkMonitorConfigtoString = NetworkMonitorConfigtoString;
	return g_NetworkMonitorConfig;
}
/**
 * @name:   newobjectNetworkIpData
 * @Author: qihoo360
 * @msg:    ret value need free
 * @param 
 * @return: 
 */
NetworkIpData* NetworkAdt_newobjectNetworkIpData(void){
	NetworkIpData* g_NetworkIpData = (NetworkIpData*)malloc(sizeof(NetworkIpData));
	memset(g_NetworkIpData,0,sizeof(NetworkIpData));
	g_NetworkIpData->NetworkIpDatatoString = NetworkIpDatatoString;
	return g_NetworkIpData;
}
/**
 * @name: freeobjectNetworkIpData
 * @Author: qihoo360
 * @msg:    free space
 * @param 
 * @return: 
 */
void NetworkAdt_freeobjectNetworkIpData(NetworkIpData* _input){
	if(_input->destIp != NULL)
		free(_input->destIp);
	if(_input->processName != NULL)
		free(_input->processName);
	if(_input->srcIp != NULL)
		free(_input->srcIp);
	free(_input);
}
/**
 * @name:   newobjectNetworkIpUploadData
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
NetworkIpUploadData* NetworkAdt_newobjectNetworkIpUploadData(void){
	NetworkIpUploadData* g_NetworkIpUploadData = (NetworkIpUploadData*)malloc(sizeof(NetworkIpUploadData));
	memset(g_NetworkIpUploadData,0,sizeof(NetworkIpUploadData));
	g_NetworkIpUploadData->NetworkIpUploadDatatoString = NetworkIpUploadDatatoString;
	return g_NetworkIpUploadData;

}
/**
 * @name:   freeobjectNetworkIpUploadData 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void NetworkAdt_freeobjectNetworkIpUploadData(NetworkIpUploadData* _input){
	if(_input->destIp != NULL)
		free(_input->destIp);
	if(_input->processName != NULL)
		free(_input->processName);
	if(_input->srcIp != NULL)
		free(_input->srcIp);
	free(_input);
}
/**
 * @name:   freeobjectNetworkConfig
 * @Author: qihoo360
 * @msg:    void
 * @param    
 * @return: 
 */ 
void NetworkAdt_freeobjectNetworkConfig(NetworkConfig* _input){
	if(_input->mobileIfaces != NULL )
		free(_input->mobileIfaces);
	if(_input->wifiIface != NULL)
		free(_input->wifiIface);
	free(_input);
}
/**
 * @name:   newobjectNetworkConfig
 * @Author: qihoo360
 * @msg:    return value must free 
 * 			some char* val need malloc
 * @param   void 
 * @return: 
 */
NetworkConfig* NetworkAdt_newobjectNetworkConfig(void){
	NetworkConfig* g_NetworkConfig = (NetworkConfig*)malloc(sizeof(NetworkConfig));
	memset(g_NetworkConfig,0,sizeof(NetworkConfig));
	g_NetworkConfig->NetworkConfigtoString = NetworkConfigtoString;
	return g_NetworkConfig;
}
 
/**
 * @name:   newobjectNetworkDnsData
 * @Author: qihoo360
  * @msg:   return value must free 
 * 			some char* val need malloc
 * @param   void
 * @return: 
 */
NetworkDnsData* NetworkAdt_newobjectNetworkDnsData(void){
	NetworkDnsData* g_NetworkDnsData = (NetworkDnsData*)malloc(sizeof(NetworkDnsData));
	memset(g_NetworkDnsData,0,sizeof(NetworkDnsData));
	g_NetworkDnsData->NetworkDnsDatatoString = NetworkDnsDatatoString;
	return g_NetworkDnsData;
}
/**
 * @name:   freeobjectNetworkDnsData
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void NetworkAdt_freeobjectNetworkDnsData(NetworkDnsData* _input){
	if(_input->destIp != NULL)
		free(_input->destIp);
	if(_input->dns != NULL)
		free(_input->dns);
	if(_input->processName != NULL)
		free(_input->processName);
	free(_input);
}
/**
 * @name:   newobjectNetworkDnsUploadData
 * @Author: qihoo360
  * @msg:   return value must free 
 * 			some char* val need malloc
 * @param   retvalue must free 
 * @return: 
 */
NetworkDnsUploadData* NetworkAdt_newobjectNetworkDnsUploadData(void){
	NetworkDnsUploadData *l_obj = (NetworkDnsUploadData*)malloc(sizeof(NetworkDnsUploadData));
	memset(l_obj,0,sizeof(NetworkDnsUploadData));
	l_obj->NetworkDnsUploadDatatoString = NetworkDnsUploadDatatoString;
	return l_obj;
}
/**
 * @name:   freeobjectNetworkDnsUploadData
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void NetworkAdt_freeobjectNetworkDnsUploadData(NetworkDnsUploadData* _input){
	if(_input->destIp != NULL)
		free(_input->destIp);
	if(_input->dns != NULL)
		free(_input->dns);
	if(_input->processName != NULL)
		free(_input->processName);
	free(_input);
}
/**
 * @name:   newobjectNetworkFlowData
 * @Author: qihoo360
 * @msg:   return value must free 
 * 		   some char* val need malloc
 * @param 
 * @return: 
 */
NetworkFlowData* NetworkAdt_newobjectNetworkFlowData(void){
	NetworkFlowData *l_obj = (NetworkFlowData*)malloc(sizeof(NetworkFlowData));
	memset(l_obj,0,sizeof(NetworkFlowData));
	l_obj->addBytes = addBytes;
	l_obj->head.size = 0;
	l_obj->head.head = NULL;
	l_obj->head.tail = NULL;
	l_obj->NetworkFlowDatatoString = NetworkFlowDatatoString;
	l_obj->listpro.addIfaceFlow = addIfaceFlow;
	l_obj->listpro.clearAllIfaceFlow = clearAllIfaceFlow;
	l_obj->listpro.initclearAllIfaceFlowlist = initclearAllIfaceFlowlist;
	((l_obj->listpro.initclearAllIfaceFlowlist))(&(l_obj->head));
	return l_obj;
}
/**
 * @name:   freeobjectNetworkFlowData
 * @Author: qihoo360
 * @msg: 
 * @param   
 * @return: 
 */
void NetworkAdt_freeobjectNetworkFlowData(NetworkFlowData* _input){
	(*(_input->listpro.clearAllIfaceFlow))(&(_input->head));
	free(_input);
}

#endif
