/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-06-16 22:14:32
 */ 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "signal.h"
#include "util.h"
#include "idps_main.h"
#include "common.h"
#include "cJSON.h"
#include "Base_networkmanager.h"
#include "Networkmonitor.h"
#include "FileMonitor.h"
#include "SysConfigMonitor.h"
#include "ResourceMonitor.h"
#include "ProcessMonitor.h"
#include "Networkfirewall.h"
#include "ConfigParse.h"
#include "websocketmanager.h"
#include "runtimemanager.h"
#include "aes/KeyManager.h"
#include "websocketclient.h"
#include "websocketConfig.h"
#include "idps_share_data.h"
#include "canMonitor.h"
#include "idpslog.h"
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
static configSet configSetObj;
static configData configObj;
static int s_net_connect_fail_cnt = 0;

#define FIRST_STAGE_RETRY_INTERVAL_TIME (60) //1 minute
#define FIRST_STAGE_RETRY_ALL_TIME (20*FIRST_STAGE_RETRY_INTERVAL_TIME) //20 minute
#define SECOND_STAGE_RETRY_INTERVAL_TIME (300) //5 minute
#define SECOND_STAGE_RETRY_ALL_TIME ((20*SECOND_STAGE_RETRY_INTERVAL_TIME)+FIRST_STAGE_RETRY_ALL_TIME) //100 minute
#define THIRD_STAGE_RETRY_INTERVAL_TIME (1800) //30 minute




/**
 * @func 信号清理函数
 **/
static void cleanHandler();
static void GetIDSVersion(char* _versioninfo);
static void SetIDSVersion();

// Udid区分我们设备，imei是区分通信模块
// 获取不对则使用配置表中udid
static int GetImei(char *udid, configData configObj)
{
	if(udid){
		strcpy(udid, getTCUID());
		if(strlen(udid)<2){
			memcpy(udid, configObj.networkManagerObj.imeiNumber, strlen(configObj.networkManagerObj.imeiNumber));
		}
	}

	if(getVIN() && udid){
		char spdlog[256] = {0};
		snprintf(spdlog, 256, "Udid:%s, Vin:%s", udid, getVIN());
		idpslog(0,"idps mian", spdlog);
	}
	else{
		idpslog(4,"idps mian", "udid or vin is null!");
	}

	return 0;
}




// 获取密钥测试
static void KeyLoop(int atcion)
{
	while(atcion){
		char *_sManagekey = NULL, *_sSessionkey = NULL, *_sSn = NULL, *_sToken = NULL;
		_sManagekey  = networkMangerMethodobj.getManagerKey();
		_sSessionkey = networkMangerMethodobj.getSessionKey();
		_sSn    	 = networkMangerMethodobj.getSn();
		_sToken      = networkMangerMethodobj.getToken();
		if(_sManagekey){
			idpslog(0,"idps Key", "idps_Key successful");
			idpslog(0,"idps Managekey", _sManagekey);
			idpslog(0,"idps Sessionkey",_sSessionkey);
			idpslog(0,"idps Sn", _sSn);
			idpslog(0,"idps Token", _sToken);
			return;
		}
		sleep(5);
	}
}

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
	idpsStatusInfo_t statusInfo;

	while (1)
	{
		sleep(1);
		switch (net_connect_step)
		{
			case NET_CONNECT_STEP__WAIT_INIT:
			{
				/*disable reinitWebsocketConnect flag*/
				websocketMangerMethodobj.reinitWebsocketConnect(false);

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

					ret = getCloudConfig(&configSetObj);  //获取探针配置
					if (ret == 1 && s_net_connect_fail_cnt >=2)
					{
						exit(0);
					}

					if (false == Websocket_is_init)
					{
						websocketMangerMethodobj.initWebsocket(&configObj.websocketInfoObj);
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
					websocketMangerMethodobj.reinitWebsocketConnect(true);
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
						websocketMangerMethodobj.reinitWebsocketConnect(false);
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

		shm_get_idps_status(&statusInfo);
		if (net_connect_step == NET_CONNECT_STEP__WBS_CONNECT_CHECK)
		{
				statusInfo.idps_workflow_status = IDPS_WORKFLOW_SUCCESS;
				shm_set_idps_status(statusInfo);
		}
		else
		{
				statusInfo.idps_workflow_status = IDPS_WORKFLOW_NONETCOMMU;
				shm_set_idps_status(statusInfo);
		}
	}
}

static void initNetConnect(void)
{
	pthread_t pthread_net_connect;
	if (networkFunctionEnabled())
	{
		pthread_create(&pthread_net_connect, NULL, net_connect_task, NULL);
	}
	else
	{
		idpslog(0,"idps main", "Enter no network mode actively");
		getCloudConfig(&configSetObj);	//获取探针配置
		return;
	}

	/*Wait for the initial configuration in the thread(net_connect_task).*/
	while (1)
	{
		sleep(1);
		if (s_net_connect_fail_cnt >= 2)
		{
			idpslog(0,"idps main", "Enter no network mode passively");
			getCloudConfig(&configSetObj);	//获取探针配置
			break;
		}

		if (true == Websocket_is_init)
		{
			idpslog(0,"idps main", "Websocket_is_init ok");
			break;
		}
	}
}


int main(){
	//step 0:获取基础配置
	char spdlog[256] = {0};
	int ret = 0;

	idpsStatusInfo_t statusInfo;

	conf_rw_path_init();
	SetIDSVersion();
	getLocalConfig(&configObj, 0);  //获取common配置

	idpslog_init(configObj.commonModuleObj.file_enb,
				 configObj.commonModuleObj.dataspdlog,
				 5*1024*1024, 
				 2, 
				 configObj.commonModuleObj.db_enb,
				 configObj.commonModuleObj.databaseName,
				 1,//加密使能
				 5*1024*1024,
				 1000);

	idps_share_data_area_init();
	statusInfo.idps_software_status = IDPS_SOFTWARE_ONGOING;
	statusInfo.idps_workflow_status = IDPS_WORKFLOW_SUCCESS;
	statusInfo.idps_program_status = IDPS_PROGRAM_OK;
	statusInfo.idps_version = 0x01020001;
	shm_set_idps_status(statusInfo);

	ret = initTboxInfo(&configObj);                 //初始化tbox的硬件信息
	if (ret !=0)
	{
		goto main_sleep;
	}
	//InitSql(configObj);
	char imei[72] = {0};
	GetImei(imei, configObj);
	init_sync_clock();
	initFlagNetworkConnection();

    //step 2:network manger init
#if MODULE_NETWORKMANAGER
    networkMangerMethodobj.newNetworkManager(configObj.networkManagerObj.mqttPem,configObj.networkManagerObj.snPath); //mqtt.pem路径和managekey及sn的存储路径
    networkMangerMethodobj.setDeviceConfig(imei,configObj.networkManagerObj.channelId,configObj.networkManagerObj.equipmentType); //imei (udid)
    networkMangerMethodobj.setManageKeyStore(configObj.networkManagerObj.manageKeyStore); //设置key_iv的模式
    networkMangerMethodobj.setServerConfig(configObj.networkManagerObj.server);  //设置baseurl
	initNetConnect();
#endif

#if MODULE_NETFIREWALL
	if(configSetObj.firewallMonitorObj.switchFun)
	{
		NetWorkFirewallMethodObj.startIptables(configSetObj.firewallMonitorObj.rules);
	}
#endif	

#if MODULE_NETWORKMONITOR
    if(configSetObj.networkMonitorObj.switchFun ||
       configSetObj.networkTrafficObj.switchFun ||
       configSetObj.networkInvasionObj.switchFun)
	{ 
		NetWorkMonitorMethodObj.newNetworkMonitor(
			configSetObj.networkInvasionObj.interface,
			configObj.networkManagerObj.watchNicDevice,
			configSetObj.networkInvasionObj.switchFun,
			configSetObj.networkInvasionObj.attackList,
			configSetObj.networkInvasionObj.attackThreshold,
			configSetObj.networkTrafficObj.switchFun,
			configSetObj.networkTrafficObj.collectPeriod,
			configSetObj.networkMonitorObj.switchFun,
			configSetObj.networkMonitorObj.collectPeriod);
		NetWorkMonitorMethodObj.updateIpWhiteList(configSetObj.networkMonitorObj.IpWhiteList);
		NetWorkMonitorMethodObj.updatePortWhiteList(configSetObj.networkMonitorObj.PortWhiteList);
		NetWorkMonitorMethodObj.updateDNSWhiteList(configSetObj.networkMonitorObj.DNSWhiteList);
		NetWorkMonitorMethodObj.startNetWorkMonitor(10); //寻找网卡周期
	}

	if (configSetObj.networkMonitorObj.canSwitchFun == 1)
	{
		initCanConnect(configSetObj.networkMonitorObj.can);
	}
#endif  

#if MODULE_FILEMONITOR
	if(configSetObj.fileMonitorObj.switchFun)
	{
		FileMonitorObj.initFileMonitor();//0.设置回调函数，
		FileMonitorObj.setFileWorkDirectory("/usr/local/ids");//1.设置隔离和备份目录
		FileMonitorObj.setIsolateDirectoryThreshold(10);//2.设置隔离和备份的阈值
		FileMonitorObj.setBackupDirectoryThreshold(10);
		FileMonitorObj.startFileMonitor();//3.启动文件监控
		FileMonitorObj.updateWatchPoint(configSetObj.fileMonitorObj.watchPoints);//4.更新文件监控点
	}
#endif

#if MODULE_CONFIGMONITOR
	if(configSetObj.shellMonitorObj.switchFun)
	{
		char* validuserinfo = NULL;
		SysConfigMonitorObj.initConfigMonitor();
		SysConfigMonitorObj.startConfigMonitor();
		SysConfigMonitorObj.getValidUserInfo(&validuserinfo);
		snprintf(spdlog, 256, "valid user info :%s", validuserinfo);
		idpslog(0,"idps main",spdlog);
		memset(spdlog,0,256);
		if(validuserinfo)
			free(validuserinfo);
	}	
#endif 

#if MODULE_PROCESSMONITOR
	if(configSetObj.processMonitorObj.switchFun)
	{
		ProcessMonitorObj.initProcessMonitor(configSetObj.processMonitorObj.collectPeriod);
		ProcessMonitorObj.updateProcessWhiteList(configSetObj.processMonitorObj.whiteList);
		ProcessMonitorObj.startProcessMonitor();
	}
#endif

#if MODULE_RESOURSEMONITOR
	if(configSetObj.resourceMonitorObj.switchFun)
	{
		ResourceMonitorObj.initResourceMonitor(configSetObj.resourceMonitorObj.collectPeriod);
		ResourceMonitorObj.setCPUWorkParameter(configSetObj.resourceMonitorObj.cpu_collectPeriod, configSetObj.resourceMonitorObj.cpu_overloadThreshold, configSetObj.resourceMonitorObj.cpu_usageRateOverloadThreshold);
		ResourceMonitorObj.setRAMWorkParameter(configSetObj.resourceMonitorObj.ram_collectPeriod, configSetObj.resourceMonitorObj.ram_overloadThreshold);
		ResourceMonitorObj.setROMWorkParameter(configSetObj.resourceMonitorObj.rom_collectPeriod, configSetObj.resourceMonitorObj.rom_overloadThreshold);
		ResourceMonitorObj.startResourceMonitor();
		//ResourceMonitorObj.uploadResourceSnapshot();  //资源快照
	}
#endif

main_sleep:
    //thats a loop
    signal(SIGINT, cleanHandler);
    for(;;){
        sleep(100);
    }
    
    return 0;
}

static void cleanHandler()
{
	idpslog(0,"idps mian","capture signnal >> exec cleanHandler");
	exit(0);
#if MODULE_NETWORKMANAGER
    networkMangerMethodobj.freeNetWorkManager();
#endif
#if MODULE_NETWORKMONITOR
    NetWorkMonitorMethodObj.stopNetworkMonitor();
    NetWorkMonitorMethodObj.freeNetWorkMonitor();
#endif   
#if MODULE_FILEMONITOR
    FileMonitorObj.freeFileMonitor();
    FileMonitorObj.stopFileMonitor();
#endif
#if MODULE_RESOURSEMONITOR
    ResourceMonitorObj.freeResourceMonitor();
#endif
#if MODULE_PROCESSMONITOR
#endif
#if MODULE_CONFIGMONITOR
    SysConfigMonitorObj.stopConfigMonitor();
#endif
#if MODULE_WEBSOCKETRPC
	websocketMangerMethodobj.stopWebsocket();
#endif
    exit(0);
}

static void GetIDSVersion(char* _versioninfo){
	char* idpsVersion = IDSVERSION;
	printf("%s\n", idpsVersion);
	if(_versioninfo){
		memcpy(_versioninfo, idpsVersion, strlen(idpsVersion));
	}
}

static void SetIDSVersion(){
	recordVesion(IDSVERSION);
}
