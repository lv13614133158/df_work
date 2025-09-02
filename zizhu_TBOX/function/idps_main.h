/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-05-07 22:31:46
 */ 
#ifndef __IDPS_MAIN_
#define __IDPS_MAIN_
#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>
#include "spdloglib.h"

/**
 * @name:   module choose
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */

#define MODULE_NETWORKMANAGER     1
#define MODULE_NETWORKMONITOR     1
#define MODULE_PROCESSMONITOR     1
#define MODULE_RESOURSEMONITOR    1
#define MODULE_CONFIGMONITOR      1
#define MODULE_FILEMONITOR        1
#define MODULE_WEBSOCKETRPC       1
#define MODULE_NETFIREWALL        1
#define IDPS_FULL_FUNCTION_       0
#define MODULE_FUNCMONITOR		  0
#define MODULE_MQTTMANAGER		  0

typedef struct _commonModule{
	char dataBaseDir[256];
	char databaseName[32];
	char dataspdlog[64];
	char certsPath[128];
}comModule_t;

typedef struct _networkManagerModule{
	int heartbeat;
	int threadPoolNumber;
	int readDbLoop;
	int manageKeyStore;
	char server[128];
	char snPath[128];
	char mqttPem[128];
	char imeiNumber[128];
	char channelId[32];
	char equipmentType[32];
	char watchNicDevice[32];
}networkManagerModult_t;

typedef struct _websocketInfoModult{
   char url[128];
   char channelId[128];
   char equmentType[128];
   char sn[128];
   int  version;
   int  sslShutdown;
   int  port;
   int  intervalread;
   int  intervalheart;
   int  intervalkey;
   char listPath[128];
   char certsPath[128];
}websocketInfoModult_t;
/**
 * @description: module config info
 * @param 
 * @return {*}
 */
typedef struct configData{
	comModule_t  		   commonModuleObj;
	networkManagerModult_t networkManagerObj;
	websocketInfoModult_t  websocketInfoObj;
}configData;

#ifdef __cplusplus
}
#endif
#endif
