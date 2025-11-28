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

void my_callback(void* data) {
    printf("Callback received data: %s\n", (char*)data);
}

int main(){
	networkManager_t data_https = {
		.networkFunction =	1,
		.vin = "LQH02501170003001",
		.sn = "SN012345678912345",
		.channelId = "T00001",
		.equmentType = "t-box",
		.certsPath = "/home/nvidia/df/df_work/CA/s73a2/",
		.cert_eable = 0,
		.cert_mem = NULL,
		.cert_mem_len = 0,
		.key_mem = NULL,
		.key_mem_len = 0,
		.ca_mem = NULL,
		.ca_mem_len = 0,
		.ssl_chose = 1,
		.https_url = "https://vsocidps-uat.dfiov.com.cn",
		.work_path = "./idps/cache",
		.web_url = "vsocidps-uat.dfiov.com.cn",
		.port = 443,
		.intervalheart = 60,
		.intervalkey = 60,
		.intervalread = 60,
		.config = {
            .model = "S73",            
            .brand = "东风",             
            .idps_version = "3.0.0",
            .manufacturer = "xxx",
            .simu_sys_version = "30.0.0",
            .mac_addr = "888.8888.888.88",
            // 系统参数
            .core_num = 8,
            .cpu_model_name = "x86",
            .host_name = "nvidia",
            .ip_addr = "192.168.10.10",
            .machine = "x86_64",
            .mem_total = 1024,
            .release = "ubunutu 16.04.5 LTS",
            .os_name = "linux",
            .os_version = "3.0.0x"
		},
	};
	register_networkManager_callback(my_callback);

 	networkManager_init(&data_https);
    for(;;){
        sleep(10);
		//networkManager_send("{\"data\":\"hello\",\"extended_data\":\"这是一个扩展的数据字段，用于增加消息的大小。在实际应用中，我们可能需要发送大量的数据，这就要求我们的网络传输能够处理大容量的信息。通过增加数据量，我们可以测试网络连接的稳定性和传输效率。这个扩展的数据包含了一些中文文本，用来模拟真实应用中可能传输的业务数据。在车联网系统中，车辆可能会发送大量的传感器数据、位置信息、诊断信息等，这些数据的大小可能会远远超过原始的简单hello消息。为了确保系统能够处理这些大数据量的传输，我们需要进行相应的测试。此外，大数据量的传输也对网络带宽和延迟提出了更高的要求。通过这种方式，我们可以验证系统在各种负载条件下的性能表现。在实际部署中，优化数据传输效率对于提升用户体验和降低运营成本都是非常重要的。这个测试数据还将帮助我们识别潜在的性能瓶颈，并为系统优化提供依据。随着车联网技术的发展，数据传输的需求会越来越大，因此确保系统能够处理大容量数据传输变得至关重要。\",\"timestamp\":\"2023-06-16T22:14:32Z\",\"device_info\":{\"vin\":\"LQH025011700030001\",\"sn\":\"SN012345678912345\",\"channelId\":\"T00001\",\"equmentType\":\"t-box\",\"config\":{\"model\":\"S73\",\"brand\":\"东风\",\"idps_version\":\"3.0.0\",\"manufacturer\":\"xxx\",\"simu_sys_version\":\"30.0.0\",\"mac_addr\":\"888.8888.888.88\",\"core_num\":8,\"cpu_model_name\":\"x86\",\"host_name\":\"nvidia\",\"ip_addr\":\"192.168.10.10\",\"machine\":\"x86_64\",\"mem_total\":1024,\"release\":\"ubunutu 16.04.5 LTS\",\"os_name\":\"linux\",\"os_version\":\"3.0.0x\"}},\"additional_info\":{\"purpose\":\"测试大数据量传输\",\"expected_size\":\"约1500字节\",\"test_type\":\"网络性能测试\",\"description\":\"此消息用于测试网络管理器处理大容量数据传输的能力。通过发送包含大量信息的JSON消息，我们可以验证网络连接的稳定性和数据传输的可靠性。这对于车联网应用尤其重要，因为车辆可能需要传输大量的传感器数据、位置信息和诊断信息。确保系统能够高效处理这些大数据量的传输对于提供良好的用户体验和实现可靠的车联网服务至关重要。\",\"test_parameters\":{\"compression_enabled\":true,\"encoding\":\"gzip\",\"connection_type\":\"HTTPS\",\"security_level\":\"high\"},\"performance_metrics\":{\"expected_latency\":\"<100ms\",\"expected_throughput\":\">1Mbps\",\"reliability_target\":\">99.9%\"}},\"sequence_number\":123456789,\"message_type\":\"test_data\",\"version\":\"1.0\",\"checksum\":\"ABCDEF1234567890\"}}");
    }
    sleep(5);

    

    return 0;
}

