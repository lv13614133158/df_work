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
		log_v("idps mian", spdlog);
	}
	else{
		log_e("idps mian", "udid or vin is null!");
	}

	return 0;
}

// sqllite init
// static int InitSql(configData configObj)
// {
// 	// 密钥目录
//     char databasepath[256] = {0};
//     cryptoobj.setWorkDirectory(configObj.commonModuleObj.dataBaseDir);  
//     memcpy(databasepath, configObj.commonModuleObj.dataBaseDir, strlen(configObj.commonModuleObj.dataBaseDir));

// 	// 数据库的目录
//     strcat(databasepath, "/");
//     strcat(databasepath, configObj.commonModuleObj.databaseName);
//     sqliteMedthodobj.initDataBase(databasepath);  

// 	return 0;
// }

// spdlog init
static int InitLog(configData configObj)
{
	set_console_logger(true);     // 客户端打印
	set_file_write_logger(true);  // 写入文件
    set_file_logger(configObj.commonModuleObj.dataspdlog, 5*1024*1024, 2);  //设置日志文件存储路径、大小、滚动个数
	set_level(0);
	log_v("idps version","360 Autocare DR V1.2"); 

	return 0;
}

int main(){
	//step 0:获取基础配置
	char spdlog[256] = {0};
	configSet configSetObj;
	configData configObj;

	memset(&configSetObj, 0, sizeof(configSetObj));
	memset(&configObj, 0, sizeof(configObj));

	conf_rw_path_init();
	SetIDSVersion();
	getLocalConfig(&configObj, 0);  //获取common配置
	InitLog(configObj);
	initTboxInfo();                 //初始化tbox的硬件信息
	//InitSql(configObj);
	char imei[72] = {0};
	GetImei(imei, configObj);
	init_sync_clock();

    //step 2:network manger init
#if MODULE_NETWORKMANAGER
    networkMangerMethodobj.newNetworkManager(configObj.networkManagerObj.mqttPem,configObj.networkManagerObj.snPath); //mqtt.pem路径和managekey及sn的存储路径
    networkMangerMethodobj.setDeviceConfig(imei,configObj.networkManagerObj.channelId,configObj.networkManagerObj.equipmentType); //imei (udid)
    networkMangerMethodobj.setManageKeyStore(configObj.networkManagerObj.manageKeyStore); //设置key_iv的模式
    networkMangerMethodobj.setServerConfig(configObj.networkManagerObj.server);  //设置baseurl
	websocketMangerMethodobj.initNetConnect(&configObj, &configSetObj);  // https连接 wss开启
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
		log_v("idps main",spdlog);
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

 	//thats a loop
    signal(SIGINT, cleanHandler);
    for(;;){
        sleep(100);
    }
    
    return 0;
}

static void cleanHandler()
{
	log_v("idps mian","capture signnal >> exec cleanHandler");
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
