#ifndef HTTPS_NETWORKMANAGER_H
#define HTTPS_NETWORKMANAGER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "signal.h"
#include "common.h"
#include "cJSON.h"
#include "Base_networkmanager.h"
#include "stdbool.h"

#define GETREQUEST_DATALEN  (1024*5)
#define GET_MONITOR_CONFIG  "/api/v1.2/policy/config"
#define _IDPS_CA "root_cert.pem"
#define _IDPS_CRT "device_cert.pem"
#define _IDPS_KEY "device_key.pem"
#define IDS_VERSION  "3.0.0"

#define FIRST_STAGE_RETRY_INTERVAL_TIME (60) //1 minute
#define FIRST_STAGE_RETRY_ALL_TIME (20*FIRST_STAGE_RETRY_INTERVAL_TIME) //20 minute
#define SECOND_STAGE_RETRY_INTERVAL_TIME (300) //5 minute
#define SECOND_STAGE_RETRY_ALL_TIME ((20*SECOND_STAGE_RETRY_INTERVAL_TIME)+FIRST_STAGE_RETRY_ALL_TIME) //100 minute
#define THIRD_STAGE_RETRY_INTERVAL_TIME (1800) //30 minute

typedef struct {
    //配置参数
    char model[128];
    char brand[128];
    char idps_version[128];
    char manufacturer[128];
    char simu_sys_version[128];
    char mac_addr[128];
    //系统参数
    short core_num;
    char cpu_model_name[128];
    char host_name[128];
    char ip_addr[128];
    char machine[128];
    int mem_total;
    char release[128];
    char os_name[128];
    char os_version[128];
    
}network_config_t;

typedef struct _networkManager{
    bool networkFunction;
    char vin[128];
    char sn[128];
    char channelId[32];
    char equmentType[32];
    char certsPath[128];
    bool cert_eable;
    char *ca_mem;
    int  ca_mem_len;
    char *cert_mem;
    int  cert_mem_len;
    char *key_mem;
    int  key_mem_len;
    char work_path[128];
    //https
    char https_url[128];
    //websocket
    char web_url[128];
    int port;
    bool ssl_chose;
    int intervalheart;
	int	intervalkey;
	int intervalread;
    network_config_t config;
}networkManager_t;


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
// typedef struct configData{
// 	networkManagerModult_t networkManagerObj;
// 	websocketInfoModult_t  websocketInfoObj;
// }configData;
typedef enum
{
    NET_CONNECT_STEP__WAIT_INIT = 0,
    NET_CONNECT_STEP__GET_KEY_ING = 1,
    NET_CONNECT_STEP__GET_KEY_SUCCESS = 2,
    NET_CONNECT_STEP__WBS_CONNECT_CHECK = 3,
    NET_CONNECT_STEP__NET_EXCEPTION_FIRST_STAGE = 4,
    NET_CONNECT_STEP__NET_EXCEPTION_SECOND_STAGE = 5,
    NET_CONNECT_STEP__NET_EXCEPTION_THIRD_STAGE = 6,
    NET_CONNECT_STEP__FINISH,
} NET_CONNECT_STEP_E;
typedef void (*websocket_callback_t)(void* data);
static bool Websocket_is_init = false;
static websocketInfoModult_t websocket_config;
extern networkManager_t networkManagerObj;
extern websocket_callback_t networkManager_callback;
// static configSet configSetObj;
static int s_net_connect_fail_cnt = 0;





int networkFunctionEnabled();
int networkManager_init(networkManager_t *data);
int networkManager_send(char *data);
void register_networkManager_callback(void* callback);
#endif