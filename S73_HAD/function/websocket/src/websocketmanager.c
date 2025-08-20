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
	// 需添加读取本地缓存
	/*
	*/
}

// 网络重新连接模块
#include "Base_networkmanager.h"
#include "aes/KeyManager.h"
#include "ConfigParse.h"

/*-----------------------------------------enum defination---------------------------------*/
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

static bool Websocket_is_init = false;
static configSet* configSetObj;
static configData* configObj;
static int s_net_connect_fail_cnt = 0;

#define FIRST_STAGE_RETRY_INTERVAL_TIME (60) //1 minute
#define FIRST_STAGE_RETRY_ALL_TIME (20*FIRST_STAGE_RETRY_INTERVAL_TIME) //20 minute
#define SECOND_STAGE_RETRY_INTERVAL_TIME (300) //5 minute
#define SECOND_STAGE_RETRY_ALL_TIME ((20*SECOND_STAGE_RETRY_INTERVAL_TIME)+FIRST_STAGE_RETRY_ALL_TIME) //100 minute
#define THIRD_STAGE_RETRY_INTERVAL_TIME (1800) //30 minute


// 获取密钥
static bool get_key(void)
{
	char *_sManagekey = NULL, *_sSessionkey = NULL, *_sSn = NULL, *_sToken = NULL;

	_sManagekey  = networkMangerMethodobj.getManagerKey();
	_sSessionkey = networkMangerMethodobj.getSessionKey();
	_sSn    	 = networkMangerMethodobj.getSn();
	_sToken      = networkMangerMethodobj.getToken();

	if (_sManagekey && _sSessionkey && _sSn && _sToken)
	{
		return true;
	}
	else
	{
		return false;
	}

	return false;
}

/* 1.get policy config.
 * 2.get the secret key.
 * 3.If the communication is abnormal, obtain the certificate again and establish a network connection.
 */
static void *net_connect_task(void *arg)
{
	pthread_detach(pthread_self());

	NET_CONNECT_STEP_E net_connect_step = NET_CONNECT_STEP__WAIT_INIT;
	unsigned int net_connect_exception_cnt = 0;
	bool ready_to_reconnect_wbs = false;
	int ret = -1;

	while (1)
	{
		sleep(1);
		switch (net_connect_step)
		{
			case NET_CONNECT_STEP__WAIT_INIT:
			{
				/*disable reinitWebsocketConnect flag*/
				wbsClient_set_reinit(false);

				/*get certificate*/
				initCert();

				/*clear key*/
				setSessionKeyToEmpty();
				net_connect_step = NET_CONNECT_STEP__GET_KEY_ING;
				break;
			}
			case NET_CONNECT_STEP__GET_KEY_ING:
			{
				if (get_key() == true) //get key succ
				{
					/*if get key success, go to get cloud config.*/

					ret = getCloudConfig(configSetObj);  //获取探针配置
					if (ret == 1 && s_net_connect_fail_cnt >=2)
					{
						exit(0);
					}

					if (false == Websocket_is_init)
					{
						websocketMangerMethodobj.initWebsocket(&configObj->websocketInfoObj);
						websocketMangerMethodobj.startWebsocket();
						Websocket_is_init = true;
					}
					s_net_connect_fail_cnt = 0;
					net_connect_step = NET_CONNECT_STEP__GET_KEY_SUCCESS;
				}
				/*if get key fail,enter retry.*/
				else
				{
					s_net_connect_fail_cnt++;
					net_connect_step = NET_CONNECT_STEP__NET_EXCEPTION_FIRST_STAGE;
				}
				break;
			}
			case NET_CONNECT_STEP__GET_KEY_SUCCESS:
			{
				/*update sessionkey,managekey,sn,token in wbs*/
				wbsSetKeyInfo();
				if (ready_to_reconnect_wbs == true)
				{
					/*enable reinitWebsocketConnect flag*/
					wbsClient_set_reinit(true);
				}
				net_connect_step = NET_CONNECT_STEP__WBS_CONNECT_CHECK;
				break;
			}
			case NET_CONNECT_STEP__WBS_CONNECT_CHECK:
			{
				/*During the program running, the loop to detect the network connection.*/
				if (wbs_client_connect_success() == true)
				{
					net_connect_exception_cnt = 0;
					if (ready_to_reconnect_wbs == true)
					{
						ready_to_reconnect_wbs = false;
						wbsClient_set_reinit(false);
					}
				}
				/*if get wbs connect fail,enter retry.*/
				else
				{
					ready_to_reconnect_wbs = true;
					net_connect_step = NET_CONNECT_STEP__NET_EXCEPTION_FIRST_STAGE;
				}
				break;
			}
			case NET_CONNECT_STEP__NET_EXCEPTION_FIRST_STAGE:
			/*Retry mechanism: Phase 1.*/
			{
				/*if get wbs connect success,end retry.*/
				if (wbs_client_connect_success() == true)
				{
					net_connect_step = NET_CONNECT_STEP__WBS_CONNECT_CHECK;
					break;
				}

				net_connect_exception_cnt++;
				if (net_connect_exception_cnt > FIRST_STAGE_RETRY_ALL_TIME)
				{
					net_connect_step = NET_CONNECT_STEP__NET_EXCEPTION_SECOND_STAGE;
				}
				else if (0 == net_connect_exception_cnt % FIRST_STAGE_RETRY_INTERVAL_TIME)
				{
					net_connect_step = NET_CONNECT_STEP__WAIT_INIT;
				}
				break;
			}
			case NET_CONNECT_STEP__NET_EXCEPTION_SECOND_STAGE:
			/*Retry mechanism: Phase 2.*/
			{
				/*if get wbs connect success,end retry.*/
				if (wbs_client_connect_success() == true)
				{
					net_connect_step = NET_CONNECT_STEP__WBS_CONNECT_CHECK;
					break;
				}

				net_connect_exception_cnt++;
				if (net_connect_exception_cnt > SECOND_STAGE_RETRY_ALL_TIME)
				{
					net_connect_step = NET_CONNECT_STEP__NET_EXCEPTION_THIRD_STAGE;
				}
				else if (0 == net_connect_exception_cnt % SECOND_STAGE_RETRY_INTERVAL_TIME)
				{
					net_connect_step = NET_CONNECT_STEP__WAIT_INIT;
				}
				break;
			}
			case NET_CONNECT_STEP__NET_EXCEPTION_THIRD_STAGE:
			/*Retry mechanism: Phase 3.*/
			{
				/*if get wbs connect success,end retry.*/
				if (wbs_client_connect_success() == true)
				{
					net_connect_step = NET_CONNECT_STEP__WBS_CONNECT_CHECK;
					break;
				}

				net_connect_exception_cnt++;
				if (0 == net_connect_exception_cnt % THIRD_STAGE_RETRY_INTERVAL_TIME)
				{
					net_connect_step = NET_CONNECT_STEP__WAIT_INIT;
				}
				break;
			}
			default:
				break;
		}
	}
}

static void initNetConnect(void* pconfigObj, void* pconfigSetObj)
{
	pthread_t pthread_net_connect;
	configObj = (configData*)pconfigObj;
	configSetObj = (configSet*)pconfigSetObj;
	initFlagNetworkConnection();
	if (networkFunctionEnabled())
	{
		pthread_create(&pthread_net_connect, NULL, net_connect_task, NULL);
	}
	else
	{
		log_v("idps main", "Enter no network mode actively");
		getCloudConfig(configSetObj);	//获取探针配置
		return;
	}

	/*Wait for the initial configuration in the thread(net_connect_task).*/
	while (1)
	{
		sleep(1);
		if (s_net_connect_fail_cnt >= 2)
		{
			log_v("idps main", "Enter no network mode passively");
			getCloudConfig(configSetObj);	//获取探针配置
			break;
		}

		if (true == Websocket_is_init)
		{
			log_v("idps main", "Websocket_is_init ok");
			break;
		}
	}
}


websocketMangerMethod  websocketMangerMethodobj = {
	initWebsocket,
	startWebsocket,
	stopWebsocket,
	sendEventData,
	sendInfoData,
	initNetConnect
};
