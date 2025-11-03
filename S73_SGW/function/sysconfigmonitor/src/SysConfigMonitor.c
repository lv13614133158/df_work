/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-27 22:19:24
 */ 
#include "cJSON.h"
#include "SysConfigMonitor.h"
#include "systemconfig.h"
#include "Base_networkmanager.h"
#include "websocketmanager.h"
#include "ConfigParse.h"
#include "api_networkmonitor.h"

static char local_net_mac_hex[6];

#if MODULE_CONFIGMONITOR
/**
 * @name:   val define
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
// char* UPLOAD_TBOX_URL = "/api/v1/tboxes";
char* UPLOAD_TBOX_URL = "/api/vsoc/v1/tboxes/hardware_data";
 
/**
 * @name:   stopConfigMonitor
 * @Author: qihoo360
 * @msg:    thats a null function,you can expand it
 * @param  
 * @return: 
 */
void stopConfigMonitor() {
	stopMonitor();
}
/**
 * @name:   //获取有效用户， 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
int getValidUserInfo(char** _output)
{
	return get_valid_user_and_passwd(_output);
}
/**
 * @name:   getTerminalInfomation
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
char* getTerminalInfomation(char** _output)
{
	cJSON *array = NULL,*root = NULL,*cjson_data1=NULL,*cjson_data2=NULL;
	char mac_addr[100] = {0};
	get_hard_info(_output);
	cjson_data2 = cJSON_Parse((char *)(*_output));
    root = cJSON_CreateObject();
    cjson_data1 = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"sn",networkMangerMethodobj.getSn());
	cJSON_AddStringToObject(cjson_data1,"vin",getVIN());
	cJSON_AddStringToObject(cjson_data1,"model",getCAR());
	cJSON_AddStringToObject(cjson_data1,"brand","东风");
	cJSON_AddStringToObject(cjson_data2,"sn",getTCUID());        
	cJSON_AddStringToObject(cjson_data2,"idps_version",Version);
	cJSON_AddStringToObject(cjson_data2,"manufacturer", getManufacturer());
	cJSON_AddStringToObject(cjson_data2,"simu_sys_version", getSIMU());
	char *addr =  get_monitor_mac();
	memcpy(local_net_mac_hex,addr,6);
	sprintf(mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",local_net_mac_hex[0],local_net_mac_hex[1],local_net_mac_hex[2],local_net_mac_hex[3],local_net_mac_hex[4],local_net_mac_hex[5]);
	cJSON_AddStringToObject(cjson_data2,"mac_addr",mac_addr);
	cJSON_AddItemToObject(root, "vehicle_info", cjson_data1);
	cJSON_AddItemToObject(root, "terminal_info", cjson_data2);
    char *s = cJSON_PrintUnformatted(root);
	char *a = cJSON_Print(root);
	printf("hardinfo:%s\n\n",a);
	if(a)free(a);
    if(root)
        cJSON_Delete(root);
	return s;
}


void uploadHardInfo(char* systemConfigInfoinput){
	if(!systemConfigInfoinput)return;
	websocketMangerMethodobj.sendInfoData(systemConfigInfoinput);
}

static void insertSystemConfig(char* data)
{

}

static char* querySystemConfig()
{
	return NULL;
}

/**
 * @name:   isSystemConfigChanged
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
static inline bool isSystemConfigChanged(char* currentSystemConfigInfo){
	if (currentSystemConfigInfo == NULL) {
		return false;
	}
	char* oldinfo =	NULL;
	if(strcmp(oldinfo,currentSystemConfigInfo) == 0){
		free(oldinfo);
		return false;
	}
	else{
		free(oldinfo);
		return true;
	}
}
/**
 * @name:  thread runing
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void runoneshot(char* systemConfigInfo){
#if 0 //硬件信息变化才上报
	if(false == isSystemConfigChanged(systemConfigInfo))
	{
		if(systemConfigInfo)
			free(systemConfigInfo);
		return;
	}
	else{
	{
		uploadHardInfo(systemConfigInfo);
		insertSystemConfig(systemConfigInfo);
		if(systemConfigInfo)
			free(systemConfigInfo);
	}
#else
	uploadHardInfo(systemConfigInfo);
	if(systemConfigInfo)
		free(systemConfigInfo);
#endif	
}
/**
 * @name:   onLoginEvent
 * @Author: qihoo360
 * @msg:    its a callback
 * @param  
 * @return: 
 */
__attribute__((unused)) int onLoginEvent(char* userName,char* loginAddress, long long time, int loginType) 
{
    cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_data,"user_name",userName);
	cJSON_AddStringToObject(cjson_data,"login_address",loginAddress);
	cJSON_AddNumberToObject(cjson_data,"login_type",loginType);
	cJSON_AddNumberToObject(cjson_data,"timestamp",time);
    char *s = cJSON_PrintUnformatted(cjson_data);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_USER_LOGIN, s);
	if(s)free(s);
	
	return 0;
}
/**
 * @name:   callbackInit 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
static inline void callbackInit(void){
	registerSystemConfigListener(onLoginEvent);
}
/**
 * @name:   initConfigMonitor
 * @Author: qihoo360
 * @msg:    callback init and sqltable  create
 * @param  
 * @return: 
 */
void initConfigMonitor(void){
	callbackInit();
}
/**
 * @name:   startConfigMonitor
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void startConfigMonitor(void){
	startMonitor();
}

SysConfigMonitor SysConfigMonitorObj = {
	startConfigMonitor,
	initConfigMonitor,
	getValidUserInfo,
	getTerminalInfomation,
    stopConfigMonitor,
    uploadHardInfo,
    runoneshot,
};
 
#endif