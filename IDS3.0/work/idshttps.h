#ifndef __HTTPS_H_
#define __HTTPS_H_
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include "cryptogram.h"
#include "curlWrapper.h"
#include "ThreadPool.h"
#include "util/postHeaderUtil.h"
#include "cJSON.h"
#include "base64.h"

#define _IDPS_CA "root_cert.pem"
#define _IDPS_CRT "device_cert.pem"
#define _IDPS_KEY "device_key.pem"

#define GET_MONITOR_CONFIG  "/api/v1.2/policy/config"
#define GET_MANAGE_KEY_URL  "/api/v1/register"
#define GET_SESSION_KEY_URL "/api/v1/session"
#define STR_LEN 8//定义随机输出的字符串长度。
#define UPDATE_TIME_SUFFIX "&update_time="
#define UPDATE_SN_SUFFIX   "?sn="
#define KEY_IV_LENGTH 16
typedef struct _idsnetworkManagerModule{
	char snPath[128];
	char url[128];
	char vin[128];
	char channelId[32];
	char equipmentType[32];
    char caPath[128];
    char *manageKey;
    char *sN;
	char *session_key;
	char *token;
}idsnetworkManagerModult_t;

static idsnetworkManagerModult_t idsnetworkManagerModule;


//离线在线模式 1：在线 0：离线
int networkFunctionEnabled(void);


int idshttps_init(idsnetworkManagerModult_t *_idsnetworkManagerModule);

char * getcaPath();

 #endif 
