/*
 * @Author: 
 * @Date: 2021-08-02 05:41:46
 * @LastEditTime: 2021-08-09 06:33:52
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/src/websocket/websocketStart.c
 */

#include "cJSON.h"
#include "common.h"
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include "spdloglib.h"
#include <string.h>
#include <unistd.h>
#include "websocketclient.h"
#include "websocketmanager.h"
#include "websocketConfig.h"
#include "websocketrpc.h"

timerIdps *timerWebsocketMonitor      = NULL;
timerIdps *timerWebsocketHeartMonitor = NULL;
static int timeIntervalRead;
static int timeIntervalHeart;
static int timeIntervalKey;

void testRpc()
{
	cJSON * root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"action","rpc");
	cJSON_AddNumberToObject(root,"seq_number",1655435617354021002);
	cJSON_AddNumberToObject(root,"timestamp",1655435617354);
	cJSON_AddStringToObject(root,"hmac","VXjZTUPMN2s7Qe9p749HjigZL/A=");
	cJSON_AddStringToObject(root,"body","{\"function\":{\"args\":[{\"type\":\"int\",\"type_id\":1,\"value\":1},{\"type\":\"string\",\"type_id\":2,\"value\":\"b\"}],\"id\":0,\"module_id\":0,\"module_name\":\"SampleFunc\",\"name\":\"SampleFunc\",\"return\":{\"type\":\"Boolean\",\"type_id\":3},\"version\":1},\"rpc_id\":\"redisChannel:T1000TB165457199757245:1655435618328\",\"sn\":\"T1000TB165457199757245\"}");
	char *s = cJSON_PrintUnformatted(root);
    wbsRpcProcess(s);
}

void testEvent()
{
    cJSON *cjson_data = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_data,"user_name","ljk");
	cJSON_AddStringToObject(cjson_data,"login_address","192.168.1.110");
	cJSON_AddNumberToObject(cjson_data,"login_type",1);
	cJSON_AddNumberToObject(cjson_data,"timestamp",1655435617354);
    char *s = cJSON_PrintUnformatted(cjson_data);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(EVENT_TYPE_USER_LOGIN, s);
	if(s) free(s);
}

void sendEventData(char* _postid,char* _data)
{
	wbsClient_sendEventData( _postid, _data);
}

void sendInfoData(char* _data)
{
	wbsClient_sendInfoData(_data);
}

static void initConfigSet(websocketInfoModult_t *pWebsocketInfoObj)
{
	if(!pWebsocketInfoObj)
	{
		log_e("idps_websocket","websocketconfigobj is NULL");
		return;
	}
	wbsSetConfig(pWebsocketInfoObj->url, pWebsocketInfoObj->channelId, pWebsocketInfoObj->equmentType, 
	  pWebsocketInfoObj->sn, pWebsocketInfoObj->version, pWebsocketInfoObj->sslShutdown, pWebsocketInfoObj->port, 
	  pWebsocketInfoObj->certsPath);
	timeIntervalRead = pWebsocketInfoObj->intervalread;
	timeIntervalHeart= pWebsocketInfoObj->intervalheart;
	timeIntervalKey  = pWebsocketInfoObj->intervalkey;
}

// 为了不断维护连接所以wbsClient_localWebSocketclient是阻塞式，需要单独开线程
// 理论上需要放入getkey线程里，这样行
static void *threadConnectWebocket(void *args)
{
	pthread_detach(pthread_self());
	wbsClient_localWebSocketclient();
}

static void *threadGetKeyInfo(void *args)
{
	pthread_detach(pthread_self());
	static int threadTime = 0;
	while(wbsSetKeyInfo() == false)
	{
		if(threadTime < 60)
		{
			sleep(10);
			threadTime++;
		}
		else
		{
			sleep(300);
		}
	}
}

void initWebsocket(websocketInfoModult_t* configObj)
{
	initConfigSet(configObj);
	wbsSetKeyInfo();
	pthread_t pthreadgetkey,pthreadwebsocket;
	pthread_create(&pthreadgetkey,   NULL,threadGetKeyInfo,     NULL);
	pthread_create(&pthreadwebsocket,NULL,threadConnectWebocket,NULL);
	sleep(1);
}

void stopWebsocket(void)
{
    wbsClient_localStopWbClient();
	if(timerWebsocketMonitor)
		timerObj.stoptimer(timerWebsocketMonitor);
	if(timerWebsocketHeartMonitor)
		timerObj.stoptimer(timerWebsocketHeartMonitor);
}

void startWebsocket()
{
	wbsClient_sendHeartBeat();
	timerWebsocketHeartMonitor = timerObj.newtime(wbsClient_sendHeartBeat);
	timerObj.setInterval(timeIntervalHeart,timerWebsocketHeartMonitor);
	timerObj.starttime(timerWebsocketHeartMonitor);

	timerWebsocketMonitor = timerObj.newtime(wbsClient_readLoopData);
	timerObj.setInterval(timeIntervalRead,timerWebsocketMonitor);
	timerObj.starttime(timerWebsocketMonitor);
	wbsClient_sendIDPSErr();
	// 需添加读取本地缓存
	/*
	*/
}

void reinitWebsocketConnect(bool reinit)
{
	wbsClient_set_reinit(reinit);
}

websocketMangerMethod  websocketMangerMethodobj = {
	initWebsocket,
	startWebsocket,
	stopWebsocket,
	sendEventData,
	sendInfoData,
	reinitWebsocketConnect
};