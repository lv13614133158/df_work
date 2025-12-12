/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-06-16 22:14:32
 */ 


#include "networkManger.h"
#include "spdloglib.h"
#include "websocketclient.h"


networkManager_t  networkManagerObj;
websocket_callback_t networkManager_callback;
int networkFunctionEnabled()
{
    return networkManagerObj.networkFunction;
}

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

static int https_init(networkManager_t *data)
{
	char* response=NULL;
    networkMangerMethodobj.newNetworkManager(data->certsPath,data->work_path); //mqtt.pem路径和managekey及sn的存储路径
    networkMangerMethodobj.setDeviceConfig(data->vin,data->channelId,data->equmentType); //imei (udid)
    networkMangerMethodobj.setServerConfig(data->https_url);  //设置baseurl

    response = (char* )malloc(GETREQUEST_DATALEN);
    int length = 0;
    memset(response, 0, GETREQUEST_DATALEN);
    networkMangerMethodobj.getUrlRequestData(GET_MONITOR_CONFIG, 0, &response, &length, GETREQUEST_DATALEN);
    printf("response %s\n",response);
	if (response != NULL) {
        free(response);
        response = NULL;
    }
	return 0;
}

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

					ret = https_init(&networkManagerObj); //应该下拉配置  但是存在哪里
					if (ret == 1 && s_net_connect_fail_cnt >=2)
					{
						exit(0);
					}

					if (false == Websocket_is_init)
					{
						initWebsocket(&websocket_config);
						startWebsocket();
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
					reinitWebsocketConnect(true);
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
						reinitWebsocketConnect(false);
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
int websocket_init(websocketInfoModult_t *data)
{
	websocket_config=*data;
	pthread_t pthread_net_connect;
	pthread_create(&pthread_net_connect, NULL, net_connect_task, NULL);


}

int networkManager_init(networkManager_t *data)
{
	networkManagerObj= *data;
	https_init(&networkManagerObj);

	websocketInfoModult_t web_data;
	strncpy(web_data.url, networkManagerObj.web_url, sizeof(web_data.url) - 1);
	web_data.url[sizeof(web_data.url) - 1] = '\0';
	web_data.port = networkManagerObj.port;
	web_data.sslShutdown = networkManagerObj.ssl_chose;
	strncpy(web_data.sn, networkManagerObj.vin, sizeof(web_data.sn) - 1);
	web_data.sn[sizeof(web_data.sn) - 1] = '\0';
	strncpy(web_data.channelId, networkManagerObj.channelId, sizeof(web_data.channelId) - 1);
	web_data.channelId[sizeof(web_data.channelId) - 1] = '\0';
	strncpy(web_data.equmentType, networkManagerObj.equmentType, sizeof(web_data.equmentType) - 1);
	web_data.equmentType[sizeof(web_data.equmentType) - 1] = '\0';
	web_data.intervalheart = networkManagerObj.intervalheart;
	web_data.intervalkey = networkManagerObj.intervalkey;
	web_data.intervalread = networkManagerObj.intervalread;
	strncpy(web_data.listPath, networkManagerObj.work_path, sizeof(web_data.listPath) - 1);
	web_data.listPath[sizeof(web_data.listPath) - 1] = '\0';
	strncpy(web_data.certsPath, networkManagerObj.certsPath, sizeof(web_data.certsPath) - 1);
	web_data.certsPath[sizeof(web_data.certsPath) - 1] = '\0';
	//websocket_init(&web_data);

	return 0;
}

int networkManager_send(char *data)
{
	long long seqnumber =wbsClient_getSeqnumber();
	wbs_socketSend(seqnumber,data,0);
}
void register_networkManager_callback(void* callback) {
    networkManager_callback = callback;
}


