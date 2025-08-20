#include "ConfigParse.h"
#include "DataRecord.h"
#include <sys/types.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include "Base_networkmanager.h"
#include "idpsReadInfo.h"
#include "secCrypto.h"
#include "idpslog.h"

// 配置文件目录位置设置
#define POS_4  1
#ifdef POS_1
	#define ROOT_PATH  "./"
#elif POS_2
	#define ROOT_PATH  "/usr/local/idps"
#elif POS_3
	#define ROOT_PATH_RW  "/data/idps"
	#define ROOT_PATH_OR  "/vendor/etc/idps"
#elif POS_4
	#define ROOT_PATH_RW  "../conf"
	#define ROOT_PATH_OR  "../conf"
#else
	#define ROOT_PATH  "/mnt/sdcard/idps"
#endif
#define DEVICE_INFO_PATH      ROOT_PATH_OR "/config/device_info.conf"
#define BASE_CONFIG_PATH   ROOT_PATH_OR "/config/base_config.json"
#define POLICY_CONFIG_PATH ROOT_PATH_RW "/config/policy_config.json"
#define PROCESS_WLIST_PATH ROOT_PATH_RW "/config/process_list.json"
#define POLICY_CONFIG_MD5  ROOT_PATH_RW "/config/save_conf.json"
#define BASE_VERSION_PATH  ROOT_PATH_RW "/version/version.ver"
#define BASE_CACHE_PATH  ROOT_PATH_RW "/cache"

#define GETREQUEST_DATALEN  (1024*5)
const char* GET_MONITOR_CONFIG = "/api/v1.2/policy/config";
tboxInfo_t tboxInfo_obj;




// buf为空时读取文件长度，
static int readLocalJson(char* path, char* buf, int len)
{
	int ret = -1;
	int fd  = open(path,O_RDONLY);
	if(fd < 0){
		idpslog(4,"ConfigParse","open file failed");
		return ret;
	}

	if(buf) //读取数据
	{
		ret = read(fd, buf, len);

		if(ret == -1)
			idpslog(4,"ConfigParse","read file failed");
	}
	else  //读取文件长度
	{
		struct stat stat_buf;   // 获取本地配置文件大小
    	memset(&stat_buf, 0, sizeof(struct stat));  
    	fstat(fd, &stat_buf);
		ret = stat_buf.st_size;
	}
	close(fd);
	return ret;
}

// 写入Json文件
static int writeLocalJson(char* path, char* buf, int len)
{
	int ret = -1;
	FILE* fp = fopen(path, "w");
	if(fp)
	{
		cJSON* cJSONBuf = cJSON_Parse(buf);
		char * writeBuf = cJSON_PrintUnformatted(cJSONBuf);
		if(writeBuf){
			ret = fwrite(writeBuf, strlen(writeBuf), 1, fp);
			fflush(fp);
			idpslog(0,"ConfigParse",writeBuf);
			free(writeBuf);

			if(ret == -1)
				idpslog(4,"ConfigParse","write file failed");
		}
		cJSON_Delete(cJSONBuf);
		fclose(fp);
		return ret;
	}
	
	idpslog(4,"ConfigParse","open file failed");
	return -1;	
}

// 保证Cjson数据安全封装
cJSON* getJsonObjectItem_s(cJSON* cJSONBuf, char *name)
{
	char spdlog[128] ={0};
	if(!cJSONBuf && !name)
	{
		idpslog(4,"ConfigParse","getJsonObjectItem_s parameter Input error");
		return NULL;
	}

	cJSON* child = cJSON_GetObjectItem(cJSONBuf, name);
	if(!child)
	{
		snprintf(spdlog, sizeof(spdlog), "getJsonObjectItem_s NO have %s ObjectItem error", name);
		idpslog(4,"ConfigParse",spdlog);
	}

	return child;
}

// 安全获取Cjson数值
double getJsonItemValue_s(cJSON* cJSONBuf, char *name)
{
	char spdlog[128] ={0};
	
	cJSON *child = getJsonObjectItem_s(cJSONBuf, name);
	if(child)
	{
		if( child->type==8)
		{
			return child->valuedouble;
		}
		else{
			snprintf(spdlog, sizeof(spdlog), "getJsonItemValue_s %s type NO is Int is %x", name, child->type);
			idpslog(4,"ConfigParse",spdlog);
		}
	}

	return -1;
}

// 并无开辟空间,安全获取Cjson字符串
char* getJsonItemString_t(cJSON* cJSONBuf, char* name)
{
	char spdlog[128] ={0};

	cJSON* child = getJsonObjectItem_s(cJSONBuf, name);
	if(child)
	{
		if(child->type==16)
		{
			return child->valuestring;
		}
		else{
			snprintf(spdlog, sizeof(spdlog), "getJsonItemString_t %s type NO is string is %x", name, child->type);
			idpslog(4,"ConfigParse",spdlog);
		}
	}
	
	return NULL;
}

// 安全获取Cjson字符串并复制
void getJsonItemString_s(cJSON* cJSONBuf, char* name, char* valuestr)
{
	char spdlog[128] ={0};

	char* cstring = getJsonItemString_t(cJSONBuf, name);
	if(cstring)
	{
		if(valuestr)
		{
			memcpy(valuestr, cstring, strlen(cstring));
		}
		else {
			snprintf(spdlog, sizeof(spdlog), "getJsonItemString_s %s parameter Input error", name);
			idpslog(4,"ConfigParse",spdlog);
		}
	}
	else{
		snprintf(spdlog, sizeof(spdlog), "No have string:%s", name);
		idpslog(4,"ConfigParse",spdlog);
	}
}

// 解析数组公共头
static void _parseGeneralConfig(cJSON* child, cJSON* object,void* obj, int mode)
{
	int* p = (int*)(obj);
	p[0] = getJsonItemValue_s(child,"switch");
	p[1] = getJsonItemValue_s(child,"policy_type_id");
	p[2] = getJsonItemValue_s(child,"terminal_type_id");
	 
	if(mode == 0) 
		p[3]=getJsonItemValue_s(object,"event_level");
}

// 1、解析时间戳，版本
long long parseCloudTime(char* str, configSet* configSetObj)
{
	long long ret = 0;
	cJSON* root = cJSON_Parse(str);
	cJSON* js_data = cJSON_GetObjectItem(root, "data");
	cJSON* js_update_time = cJSON_GetObjectItem(js_data, "update_time");
	if(js_update_time)
	{
		char log[255] ={0};
		ret = js_update_time->valuedouble;
		if(configSetObj)
			configSetObj->updateTime = ret;

		sprintf(log, "update_time:%lld", ret);
		idpslog(0,"ConfigParse",log);
	}
	cJSON_Delete(root);
	return ret;
}

// 2、解析json格式的探针配置
int parseCloudConfig(char* str,  configSet* configSetObj)
{
	cJSON* root = cJSON_Parse(str);
	cJSON* js_data = getJsonObjectItem_s(root,    "data");
	cJSON* js_list = getJsonObjectItem_s(js_data, "list");

	int size = cJSON_GetArraySize(js_list);
	if (size == 0)
	{                         
		cJSON_Delete(root); 
		idpslog(4,"ConfigParse","not get valid rules\n");                   
		return -1;                     
	}

	for (int i = 0; i < size; i++) 
	{
		cJSON* child = cJSON_GetArrayItem(js_list, i);
		char * print_data = cJSON_PrintUnformatted(child);
		if(print_data){
			idpslog(1,"ConfigParse:",print_data);
			free(print_data);
		}

		cJSON* object = getJsonObjectItem_s(child,"config");
		char*  name   = getJsonItemString_t(child,"name");
		if(!strcmp(name,"FileMonitor")) {
			_parseGeneralConfig(child, object, &configSetObj->fileMonitorObj, 0);
			cJSON* js_watch_points = getJsonObjectItem_s(object,"watch_points");
			configSetObj->fileMonitorObj.watchPoints = cJSON_PrintUnformatted(js_watch_points);	
		}
		else if(!strcmp(name,"NetworkMonitor")){
			_parseGeneralConfig(child, object, &configSetObj->networkMonitorObj, 0);
			configSetObj->networkMonitorObj.collectPeriod = getJsonItemValue_s(object,"collect_period");
			cJSON* js_ip_white_list = getJsonObjectItem_s(object,"ip_white_list");
			configSetObj->networkMonitorObj.IpWhiteList = cJSON_PrintUnformatted(js_ip_white_list);
			cJSON* js_port_white_list = getJsonObjectItem_s(object,"port_white_list");
			configSetObj->networkMonitorObj.PortWhiteList = cJSON_PrintUnformatted(js_port_white_list);
			cJSON* js_dns_white_list = getJsonObjectItem_s(object,"dns_white_list");
			configSetObj->networkMonitorObj.DNSWhiteList = cJSON_PrintUnformatted(js_dns_white_list);
			configSetObj->networkMonitorObj.canSwitchFun = getJsonItemValue_s(object,"canSwitchFun");
			cJSON* can = getJsonObjectItem_s(object,"can");
			configSetObj->networkMonitorObj.can = cJSON_PrintUnformatted(can);
		}
		else if(!strcmp(name,"ProcessMonitor")){
			_parseGeneralConfig(child, object, &configSetObj->processMonitorObj, 0);
			configSetObj->processMonitorObj.collectPeriod = getJsonItemValue_s(object,"collect_period");
			cJSON* js_white_list = getJsonObjectItem_s(object,"white_list");
			configSetObj->processMonitorObj.whiteList = cJSON_PrintUnformatted(js_white_list);		
		}
		else if(!strcmp(name,"ShellLoginMonitor")){
			_parseGeneralConfig(child, object, &configSetObj->shellMonitorObj, 0);
		}
		else if(!strcmp(name,"ResourceMonitor")){
			_parseGeneralConfig(object, object, &configSetObj->resourceMonitorObj, 0);
			configSetObj->resourceMonitorObj.collectPeriod         = getJsonItemValue_s(object, "collect_period");
			cJSON* js_cpu = getJsonObjectItem_s(object,"cpu");
			configSetObj->resourceMonitorObj.cpu_collectPeriod     = getJsonItemValue_s(js_cpu,"collect_period");
			configSetObj->resourceMonitorObj.cpu_overloadThreshold = getJsonItemValue_s(js_cpu,"overload_threshold");
			configSetObj->resourceMonitorObj.cpu_usageRateOverloadThreshold = getJsonItemValue_s(js_cpu,"usage_rate_overload_threshold");
			configSetObj->resourceMonitorObj.cpu_eventLevel        = getJsonItemValue_s(js_cpu,"event_level");
			cJSON* js_ram = getJsonObjectItem_s(object,"ram");
			configSetObj->resourceMonitorObj.ram_collectPeriod     = getJsonItemValue_s(js_ram,"collect_period");
			configSetObj->resourceMonitorObj.ram_overloadThreshold = getJsonItemValue_s(js_ram,"overload_threshold");
			configSetObj->resourceMonitorObj.ram_eventLevel        = getJsonItemValue_s(js_ram,"event_level");
			cJSON* js_rom = getJsonObjectItem_s(object,"rom");
			configSetObj->resourceMonitorObj.rom_collectPeriod     = getJsonItemValue_s(js_rom,"collect_period");
			configSetObj->resourceMonitorObj.rom_overloadThreshold = getJsonItemValue_s(js_rom,"overload_threshold");
			configSetObj->resourceMonitorObj.rom_eventLevel        = getJsonItemValue_s(js_rom,"event_level");
		}
		else if(!strcmp(name,"NetworkTrafficMonitor")){
			_parseGeneralConfig(child, object, &configSetObj->networkTrafficObj, 0);
			configSetObj->networkTrafficObj.collectPeriod = getJsonItemValue_s(object,"collect_period");
		}
		else if(!strcmp(name,"NetworkInvasionMonitor")){
			_parseGeneralConfig(child, object, &configSetObj->networkInvasionObj, 2);
			getJsonItemString_s(object, "interface", configSetObj->networkInvasionObj.interface);
			cJSON* js_attack_list = getJsonObjectItem_s(object,"list");
			configSetObj->networkInvasionObj.attackList = cJSON_PrintUnformatted(js_attack_list);
			cJSON* js_attack_threshold = getJsonObjectItem_s(object,"threshold");
			configSetObj->networkInvasionObj.attackThreshold = cJSON_PrintUnformatted(js_attack_threshold);
		}
		else if(!strcmp(name,"FirewallRegulation")){
			_parseGeneralConfig(child, object, &configSetObj->firewallMonitorObj, 2);
			cJSON* js_rules = getJsonObjectItem_s(object,"rules");
			configSetObj->firewallMonitorObj.rules = cJSON_PrintUnformatted(js_rules);	
		}
		else if(!strcmp(name,"Log")){
			_parseGeneralConfig(child, object, &configSetObj->logMonitorObj, 1);
			getJsonItemString_s(object,"storage_path",configSetObj->logMonitorObj.storage_path);
		}
	}

	cJSON_Delete(root); 
	return 0;
}

static int getPolicyConfigMd5(char *md5, int md5_len)
{
    int ret = -1;
    char *malloc_policy_buff = NULL;
    int len = 0;
    cJSON *file_obj= NULL;

    len = readLocalJson(POLICY_CONFIG_MD5, NULL, 0);
    if (len <= 0)
    {
        idpslog(2,"ConfigParse","getPolicyConfigMd5 readLocalJson faild");
        return -1;
    }

    malloc_policy_buff = (char *)malloc(len);
    if (!malloc_policy_buff)
    {
        idpslog(4,"ConfigParse","getPolicyConfigMd5 malloc error");
        return -1;
    }

    ret = readLocalJson(POLICY_CONFIG_MD5, malloc_policy_buff, len);
    if (ret <= 0)
    {
        idpslog(4,"ConfigParse","getPolicyConfigMd5 readLocalJson faild");
        free(malloc_policy_buff);
        return -1;
    }

    file_obj = cJSON_Parse(malloc_policy_buff);
    if (file_obj)
    {
        char *policy_config_md5 = cJSON_GetObjectItem(file_obj,"policy_config_md5")->valuestring;
        strncpy(md5, policy_config_md5, md5_len - 1);
    }

    free(malloc_policy_buff);
    cJSON_Delete(file_obj);

    return 0;
}

static int setPolicyConfigMd5(char *md5)
{
    cJSON *cJSONObj = NULL;
    int ret = -1;
    FILE *fp = NULL;
    char *writeBuf = NULL;

    if (!md5)
    {
        idpslog(4,"ConfigParse","setPolicyConfigMd5 param is null");
        return -1;
    }

    cJSONObj = cJSON_CreateObject();
    if (!cJSONObj)
    {
        idpslog(4,"ConfigParse","setPolicyConfigMd5 cJSON_CreateObject err");
        return -1;
    }

    cJSON_AddStringToObject(cJSONObj, "policy_config_md5", md5);

    fp = fopen(POLICY_CONFIG_MD5, "w");
    if (fp)
    {
        writeBuf = cJSON_Print(cJSONObj);
        if (writeBuf)
        {
            ret = fwrite(writeBuf, strlen(writeBuf), 1, fp);
            free(writeBuf);
        }
        cJSON_Delete(cJSONObj);
        fflush(fp);
        fclose(fp);
        return ret;
    }
    cJSON_Delete(cJSONObj);

    idpslog(4,"ConfigParse","setPolicyConfigMd5 open file failed");

    return -1;
}


/*return: -1. err; 0,use local conf;1,use cloud conf*/
int selectParseConfig(char* responses, configSet* configSetObj)
{
	int ret = -1;
    if(strstr(responses,"error") || strlen(responses) < 64) //未从平台拉取到配置,或者拉取为空
    {
        char* reasult = NULL;
		int len = readLocalJson(POLICY_CONFIG_PATH, NULL, 0);
		if(len < 0)
			return -1;
	
		reasult = (char*)malloc(len);  
		if(reasult == NULL){
			idpslog(4,"ConfigParse","policy config malloc error");
			return -1;
		}

		readLocalJson(POLICY_CONFIG_PATH, reasult, len);
		ret = parseCloudConfig(reasult, configSetObj); //使用上次保存的配置
		free(reasult);

		if(ret == -1){
			return -1;
		}

		return 0;
    }
    else //平台拉取到配置
    {
		ret  = parseCloudConfig(responses,configSetObj);
		if(ret == -1)
			return -1;
		//存到本地配置文件
		writeLocalJson(POLICY_CONFIG_PATH, responses, strlen(responses));

		return 1;
    }

	return 0;
}

/*return: -1. err; 0,use local conf; 1,use cloud conf*/
int getCloudConfig(configSet* configSetObj)
{
  	int ret = -1;
	int length = 0; 
	long long updateTime = 0;
	char* response = (char* )malloc(GETREQUEST_DATALEN);
    char policy_md5[48]={0};
    char save_md5_buff[48] = {0};

    if(response == NULL) {
		char log[255]={0};
        sprintf(log,"%s  %s  %d  malloc error\n",__FILE__,__func__,__LINE__);
		idpslog(4,"ConfigParse",log);
        return ret;
    }
	memset(configSetObj, 0, sizeof(configSet));
    memset(response, 0, GETREQUEST_DATALEN);

	//从本地读取时间戳
	if((length = readLocalJson(POLICY_CONFIG_PATH, NULL, 0)) > 0)
	{
		char* reasult = NULL;
		reasult = (char*)malloc(length);
		if(reasult){
			memset(reasult,0,length);
			readLocalJson(POLICY_CONFIG_PATH, reasult, length);
			updateTime = parseCloudTime(reasult, NULL);
			free(reasult);
			length = 0;
		}
	}

    /*md5 check*/
#if 0
    getPolicyConfigMd5(save_md5_buff, sizeof(save_md5_buff));
    Compute_file_md5(POLICY_CONFIG_PATH, policy_md5);
    if (strncmp(policy_md5, save_md5_buff, sizeof(policy_md5)))
    {
        /*repull the configuration*/
        updateTime = 0;
        idpslog("ConfigParse","md5 is diff!");
    }
#endif

    //从云端获取文件监控目录
	networkMangerMethodobj.getUrlRequestData(GET_MONITOR_CONFIG, updateTime, &response, &length, GETREQUEST_DATALEN);
    ret = selectParseConfig(response, configSetObj);  
	free(response);
    if(ret == -1){
	  idpslog(4,"ConfigParse","Config pasre err!");
	  return ret;
	}

    /*save md5*/
#if 0
    memset(policy_md5, 0, sizeof(policy_md5));
    Compute_file_md5(POLICY_CONFIG_PATH, policy_md5);
    setPolicyConfigMd5(policy_md5);
#endif

    return ret;
}

static void parseCommonConfig(cJSON* root, configData* configObj)
{
	//1.commonModule
	cJSON* js_common = cJSON_GetObjectItem(root,"commonModule");
	if(js_common){
		cJSON* dataBaseDir = cJSON_GetObjectItem(js_common,"dataBaseDir");
		if(dataBaseDir)strncpy(configObj->commonModuleObj.dataBaseDir,
			dataBaseDir->valuestring,sizeof(configObj->commonModuleObj.dataBaseDir));

		cJSON* db_enb = cJSON_GetObjectItem(js_common,"db_enb");
		if(db_enb)
			configObj->commonModuleObj.db_enb = db_enb->valueint;	

		cJSON* databaseName = cJSON_GetObjectItem(js_common,"databaseName");
		if(databaseName)strncpy(configObj->commonModuleObj.databaseName,databaseName->valuestring,
			sizeof(configObj->commonModuleObj.databaseName));
			
		cJSON* file_enb = cJSON_GetObjectItem(js_common,"file_enb");
		if(file_enb)
			configObj->commonModuleObj.file_enb = file_enb->valueint;	
			
		cJSON* dataspdlog = cJSON_GetObjectItem(js_common,"dataspdlog");
		if(dataspdlog)strncpy(configObj->commonModuleObj.dataspdlog,dataspdlog->valuestring,
			sizeof(configObj->commonModuleObj.dataspdlog));

		cJSON* certspath = cJSON_GetObjectItem(js_common,"certsPath");
		if(certspath)strncpy(configObj->commonModuleObj.certsPath, certspath->valuestring,
			sizeof(configObj->commonModuleObj.certsPath));
	}

	//2.networkManagerModule
	cJSON* js_manager = cJSON_GetObjectItem(root,"networkManager");
	if(js_manager){
		cJSON* heartBeat = cJSON_GetObjectItem(js_manager,"heartBeat");
		if(heartBeat)
			configObj->networkManagerObj.heartbeat = heartBeat->valueint;
		cJSON* threadPoolNumber = cJSON_GetObjectItem(js_manager,"threadPoolNumber");
		if(threadPoolNumber)
			configObj->networkManagerObj.threadPoolNumber = threadPoolNumber->valueint;
		cJSON* readDbLoop = cJSON_GetObjectItem(js_manager,"readDbLoop");
		if(readDbLoop)
			configObj->networkManagerObj.readDbLoop = readDbLoop->valueint;
		cJSON* manageKeyStore = cJSON_GetObjectItem(js_manager,"manageKeyStore");
		if(manageKeyStore)
			configObj->networkManagerObj.manageKeyStore = manageKeyStore->valueint;		
		cJSON* server = cJSON_GetObjectItem(js_manager,"server");
		if(server)
			strncpy(configObj->networkManagerObj.server,server->valuestring,
		sizeof(configObj->networkManagerObj.server));	

		cJSON* snPath = cJSON_GetObjectItem(js_manager,"snPath");
		if(snPath)
			strncpy(configObj->networkManagerObj.snPath,snPath->valuestring,
		sizeof(configObj->networkManagerObj.snPath));

		cJSON* imeiNumber = cJSON_GetObjectItem(js_manager,"imeiNumber");
		if(imeiNumber)
			strncpy(configObj->networkManagerObj.imeiNumber,imeiNumber->valuestring,
		sizeof(configObj->networkManagerObj.imeiNumber));

		cJSON* channelId = cJSON_GetObjectItem(js_manager,"channelId");
		if(channelId)
			strncpy(configObj->networkManagerObj.channelId,channelId->valuestring,
		sizeof(configObj->networkManagerObj.channelId));

		cJSON* equipmentType = cJSON_GetObjectItem(js_manager,"equipmentType");
		if(equipmentType)
			strncpy(configObj->networkManagerObj.equipmentType,equipmentType->valuestring,
		sizeof(configObj->networkManagerObj.equipmentType));

		cJSON* watchNicDevice = cJSON_GetObjectItem(js_manager,"watchNicDevice");
		if(watchNicDevice)
			strncpy(configObj->networkManagerObj.watchNicDevice,watchNicDevice->valuestring,
		sizeof(configObj->networkManagerObj.watchNicDevice));

		if(configObj->commonModuleObj.certsPath)
			strncpy(configObj->networkManagerObj.mqttPem, configObj->commonModuleObj.certsPath,
		 sizeof(configObj->networkManagerObj.mqttPem));
	}

	//3.websocketconfig
	cJSON* js_websocket = cJSON_GetObjectItem(root,"websocketModule");
	if(js_websocket){
		cJSON* websocketchannelid = cJSON_GetObjectItem(js_websocket,"channelId");
		if(websocketchannelid)
			strncpy(configObj->websocketInfoObj.channelId,websocketchannelid->valuestring,sizeof(configObj->websocketInfoObj.channelId));
		
		cJSON* websocketequipmenttype = cJSON_GetObjectItem(js_websocket,"equipmentType");
		if(websocketequipmenttype)
			strncpy(configObj->websocketInfoObj.equmentType,websocketequipmenttype->valuestring,sizeof(configObj->websocketInfoObj.equmentType));

		cJSON* websocketurl = cJSON_GetObjectItem(js_websocket,"url");
		if(websocketurl)
			strncpy(configObj->websocketInfoObj.url,websocketurl->valuestring,sizeof(configObj->websocketInfoObj.url));
		
		cJSON* websocketpath = cJSON_GetObjectItem(js_websocket,"rpcconfigpath");
		if(websocketpath)
			strncpy(configObj->websocketInfoObj.listPath,websocketpath->valuestring,sizeof(configObj->websocketInfoObj.listPath));

		cJSON* websocketsn = cJSON_GetObjectItem(js_websocket,"defaultsn");
		if(websocketsn)
			strncpy(configObj->websocketInfoObj.sn,websocketsn->valuestring,sizeof(configObj->websocketInfoObj.sn));
		
		configObj->websocketInfoObj.version       = cJSON_GetObjectItem(js_websocket,"version")->valueint;
		configObj->websocketInfoObj.sslShutdown   = cJSON_GetObjectItem(js_websocket,"sslshutdown")->valueint;
		configObj->websocketInfoObj.port          = cJSON_GetObjectItem(js_websocket,"port")->valueint;	
		configObj->websocketInfoObj.intervalheart = cJSON_GetObjectItem(js_websocket,"intervalheart")->valueint;
		configObj->websocketInfoObj.intervalread  = cJSON_GetObjectItem(js_websocket,"intervalsql")->valueint;	
		configObj->websocketInfoObj.intervalkey   = cJSON_GetObjectItem(js_websocket,"intervalkey")->valueint;	

		if(configObj->commonModuleObj.certsPath)
			strncpy(configObj->websocketInfoObj.certsPath, configObj->commonModuleObj.certsPath, sizeof(configObj->websocketInfoObj.certsPath)); 
	}
}

static void parseDefaultConfig(cJSON* root, configData* configObj)
{
	return;
}

int getLocalConfig(configData* configObj, int mode)
{
	char buf[2048] = {0};
	memset(configObj, 0,sizeof(configData));
	if(readLocalJson(BASE_CONFIG_PATH,buf,sizeof(buf)) == -1)
		return -1;

	cJSON* root = cJSON_Parse(buf);
	if(!root)
	{
		idpslog(4,"ConfigParse","ConfigParse get root faild");
		return -1;
	}

	if(mode == 0)
		parseCommonConfig(root, configObj);
	else if(mode == 1)
		parseDefaultConfig(root, configObj);

	if(root)
		cJSON_Delete(root);
	return 0;
}

char *getProcessWhileList(void)
{
    char* reasult = NULL;
    int   length = 0;
	if((length = readLocalJson(PROCESS_WLIST_PATH, NULL, 0)) > 0)
	{
		reasult = (char*)malloc(length);
		if(reasult)
		{
			readLocalJson(PROCESS_WLIST_PATH, reasult, length);
		}
	}
   
    return reasult;
}

int setProcessWhileList(char *white_process)
{
    int ret = -1;
    if (white_process)
    {
        FILE *fp = fopen(PROCESS_WLIST_PATH, "w");
		if (fp)
		{
			ret = fwrite(white_process, strlen(white_process), 1, fp);
			fflush(fp);
		}
    }
	
    return ret;
}

/*return: 1,data is String; 0,data is not String*/
int isString(const char *data)
{
	int i = 0;

	while (data[i] != '\0')
	{
		if (!((data[i] >= 'a' && data[i] <= 'z') || (data[i] >= 'A' && data[i] <= 'Z') || (data[i] >= '0' && data[i] <= '9')))
		{
			return 0;
		}
		i++;
	}

	return 1;
}

/*return: 1,str is AllZero; 0,str is not AllZero*/
int isAllZero(const char *str)
{
	while (*str != '\0')
	{
		if (*str != '0')
		{
			return 0;
		}
		str++;
	}

	return 1;
}

// 自定义获取第三方厂家外部信息，
static int getOutInfoAPI(char* VIN, char* SN)
{
	int ret = 0;
	// 自定义获取tbox内部信息函数


	return ret;
}

/**
 *  获取 SN、VIN、MODEL、SYS_version信息
 *  保存信息到内存
 *  use_default_info代表获取信息状态
 *	b0位: SN b1位: VIN，b2位: vehicle_model，b3位: simu_info
**/
int initTboxInfo(configData* configObj)
{
	char spdlog[512] = {0};
	tboxInfo_t tbox_info_local = {0};
	int use_default_info[5] = {0};
	char buf[512]={0};
	sgw_mcu_info_t sgw_mcu_info;
	int get_mcu_info_fail_cnt = 0;

	// 特定接口 getOutInfoAPI，返回值是use_default_info
	
	while (1)
	{
		memset(&sgw_mcu_info, 0, sizeof(sgw_mcu_info));
		//sgw_info_get_mcu_info(&sgw_mcu_info);
		strncpy(sgw_mcu_info.vin, "VIN12345678900001",17);
		sgw_mcu_info.vin[17]='\0';
		strncpy(sgw_mcu_info.sn,"SN123456789123456781", 20);
		sgw_mcu_info.sn[20]='\0';
		memset(spdlog, 0 ,sizeof(spdlog));
		snprintf(spdlog, sizeof(spdlog),
				"vin:%s, sn:%s, supplierInfo:%s, hardwareVersion:%s, softwareVersion:%s\n",
				sgw_mcu_info.vin, sgw_mcu_info.sn, sgw_mcu_info.supplierInfo, sgw_mcu_info.hardwareVersion, sgw_mcu_info.softwareVersion);
		idpslog(1,"ConfigParse", spdlog);

		/*判断是否为字符串*/
		if (isString(sgw_mcu_info.vin) && isString(sgw_mcu_info.sn))
		{
			/*判断字符串长度*/
			if (strlen(sgw_mcu_info.vin) == 0 || strlen(sgw_mcu_info.sn) == 0)
			{
				idpslog(2,"ConfigParse", "device_num or diag_vin len is 0");
			}
			else
			{
				/*判断字符串是否全部由0组成*/
				if (isAllZero(sgw_mcu_info.vin) || isAllZero(sgw_mcu_info.sn))
				{
					idpslog(2,"ConfigParse", "device_num or diag_vin is all zero");
				}
				else
				{
					break;
				}
			}
		}

		sleep(5);
		get_mcu_info_fail_cnt++;
		/*get fail 1min, return*/
		if (get_mcu_info_fail_cnt > 12)
		{
			return -1;
		}
	}

	/*get TBOX info from local configuration files*/
	if(readLocalJson(DEVICE_INFO_PATH, buf, sizeof(buf)) == -1){
		goto exit;
	}
	cJSON* root = cJSON_Parse(buf);
	if(!root)      //解析json失败
	{
		idpslog(1,"ConfigParse","tboxinfo file format error");
		goto exit;
	}

	cJSON* device_info = cJSON_GetObjectItem(root,"device_info");
	if(device_info)
	{
		cJSON* SN = cJSON_GetObjectItem(device_info,"SN");
		if((use_default_info[0]==0) && SN && *(SN->valuestring)){
			strncpy(tbox_info_local.ID,SN->valuestring,sizeof(tbox_info_local.ID) - 1);
			use_default_info[0] = 1;
		}
			
		cJSON* VIN = cJSON_GetObjectItem(device_info,"VIN");
		if((use_default_info[1]==0) && VIN && *(VIN->valuestring)){
			strncpy(tbox_info_local.VIN,VIN->valuestring,sizeof(tbox_info_local.VIN) - 1);
			use_default_info[1] = 1;
		}

		cJSON* vehicle_model = cJSON_GetObjectItem(device_info,"vehicle_model");
		if((use_default_info[2]==0) && vehicle_model && *(vehicle_model->valuestring)){
			strncpy(tbox_info_local.CAR,vehicle_model->valuestring,sizeof(tbox_info_local.CAR) - 1);
			use_default_info[2] = 1;
		}
	}

	cJSON* simu_info = cJSON_GetObjectItem(root,"simu_info");
	if(simu_info)
	{
		cJSON* sys_version = cJSON_GetObjectItem(simu_info,"sys_version");
		if((use_default_info[3]==0) && sys_version && *(sys_version->valuestring)){
			strncpy(tbox_info_local.SYS_VERSION,sys_version->valuestring,sizeof(tbox_info_local.SYS_VERSION) - 1);
			use_default_info[3] = 1;
		}
	}

exit:	
	if(use_default_info[0] == 0)
	{
		strncpy(tbox_info_local.ID, "TEST_LINUX_IDPS_SN",sizeof(tbox_info_local.ID) - 1);
	}
	if(use_default_info[1] == 0)
	{
		strncpy(tbox_info_local.VIN,"TEST_LINUX_IDPS_VIN",sizeof(tbox_info_local.VIN) - 1);
	}
	if(use_default_info[2] == 0)
	{
		strncpy(tbox_info_local.CAR,"TEST_LINUX_IDPS_MODEL",sizeof(tbox_info_local.CAR) - 1);
	}
	if(use_default_info[3] == 0)
	{
		strncpy(tbox_info_local.SYS_VERSION,"TEST_LINUX_SYS_VERSION",sizeof(tbox_info_local.SYS_VERSION) - 1);
	}

	if (root)
	{
		cJSON_Delete(root);
	}

	strncpy(tboxInfo_obj.ID, sgw_mcu_info.sn, sizeof(tboxInfo_obj.ID) - 1);
	strncpy(tboxInfo_obj.VIN, sgw_mcu_info.vin, sizeof(tboxInfo_obj.VIN) - 1);
	strncpy(tboxInfo_obj.CAR, tbox_info_local.CAR, sizeof(tboxInfo_obj.CAR) - 1);
	strncpy(tboxInfo_obj.MANUFACTURER, sgw_mcu_info.supplierInfo, sizeof(tboxInfo_obj.MANUFACTURER) - 1);

	memset(spdlog, 0 ,sizeof(spdlog));

	sprintf(spdlog, "ID:%s, VIN:%s, CAR:%s, SIMU:%s, MANUFACTURER:%s\n", tboxInfo_obj.ID, tboxInfo_obj.VIN, tboxInfo_obj.CAR, tboxInfo_obj.SYS_VERSION, tboxInfo_obj.MANUFACTURER);
	idpslog(1,"ConfigParse", spdlog);

	cache_t data;
	char cmd[256];
	int ret;
	memset(&data,0,sizeof(data));
	strncpy(data.ID,tboxInfo_obj.ID,sizeof(tboxInfo_obj.ID));
	strncpy(data.VIN,tboxInfo_obj.VIN,sizeof(tboxInfo_obj.VIN));
	strncpy(data.CAR,tboxInfo_obj.CAR,sizeof(tboxInfo_obj.CAR));
	strncpy(data.SERVER,configObj->networkManagerObj.server,sizeof(configObj->networkManagerObj.server));
	strncpy(data.URL,configObj->websocketInfoObj.url,sizeof(configObj->websocketInfoObj.url));
	ret=comparison_cache_data(&data);
	if(ret==1)
	{
		sprintf(cmd,"rm -rf %s",configObj->commonModuleObj.dataBaseDir);
		system(cmd);
	}
	return 0;
}

int initCert(void)
{
	uint32_t ret = 0;
	uint32_t data_size = 0;
	uint8_t *root_cert_buff = NULL;
	uint8_t *dev_cert_buff = NULL;
	uint8_t *dev_key_buff = NULL;

	ret = secCryptoInit();
	if (ret != 0)
	{
		idpslog(4,"ConfigParse", "secCryptoInit fail!");
	}

	/*root cert*/
	ret = secCryptoDataGetsize(SecCrypto_DataID_SGW_ROOT_CERT, &data_size);
	if (ret != 0)
	{
		idpslog(4,"ConfigParse", "secCryptoDataGetsize SecCrypto_DataID_SGW_ROOT_CERT fail!");
	}
	if (data_size > 0)
	{
		root_cert_buff = (uint8_t *)malloc(data_size + 1);
		if (root_cert_buff)
		{
			ret = secCryptoDataGet(SecCrypto_DataID_SGW_ROOT_CERT, root_cert_buff, data_size);
			if (ret != 0)
			{
				idpslog(4,"ConfigParse", "secCryptoDataGet SecCrypto_DataID_SGW_ROOT_CERT fail!");
			}
			else
			{
				if (s_root_cert)
				{
					free(s_root_cert);
					s_root_cert = NULL;
				}
				s_root_cert = (uint8_t *)malloc(data_size + 1);
				if (s_root_cert)
				{
					memset(s_root_cert, 0, data_size + 1);
					memcpy(s_root_cert, root_cert_buff, data_size);
				}
			}
			free(root_cert_buff);
			root_cert_buff = NULL;
		}
	}
	else
	{
		idpslog(4,"ConfigParse", "root cert data_size err!");
	}

	/*dev cert*/
	data_size = 0;
	ret = secCryptoDataGetsize(SecCrypto_DataID_SGW_DEV_CERT, &data_size);
	if (ret != 0)
	{
		idpslog(4,"ConfigParse", "secCryptoDataGetsize SecCrypto_DataID_SGW_DEV_CERT fail!");
	}
	if (data_size > 0)
	{
		dev_cert_buff = (uint8_t *)malloc(data_size + 1);
		if (dev_cert_buff)
		{
			ret = secCryptoDataGet(SecCrypto_DataID_SGW_DEV_CERT, dev_cert_buff, data_size);
			if (ret != 0)
			{
				idpslog(4,"ConfigParse", "secCryptoDataGet SecCrypto_DataID_SGW_DEV_CERT fail!");
			}
			else
			{
				if (s_dev_cert)
				{
					free(s_dev_cert);
					s_dev_cert = NULL;
				}
				s_dev_cert = (uint8_t *)malloc(data_size + 1);
				if (s_dev_cert)
				{
					memset(s_dev_cert, 0, data_size + 1);
					memcpy(s_dev_cert, dev_cert_buff, data_size);
				}
			}
			free(dev_cert_buff);
			dev_cert_buff = NULL;
		}
	}
	else
	{
		idpslog(4,"ConfigParse", "dev certificate data_size err!");
	}

	/*dev key*/
	data_size = 0;
	ret = secCryptoDataGetsize(SecCrypto_DataID_SGW_DEV_KEY, &data_size);
	if (ret != 0)
	{
		idpslog(4,"ConfigParse", "secCryptoDataGetsize SecCrypto_DataID_SGW_DEV_KEY fail!");
	}
	if (data_size > 0)
	{
		dev_key_buff = (uint8_t *)malloc(data_size + 1);
		if (dev_key_buff)
		{
			secCryptoDataGet(SecCrypto_DataID_SGW_DEV_KEY, dev_key_buff, data_size);
			if (ret != 0)
			{
				idpslog(4,"ConfigParse", "secCryptoDataGet SecCrypto_DataID_SGW_DEV_KEY fail!");
			}
			else
			{
				if (s_dev_key)
				{
					free(s_dev_key);
					s_dev_key = NULL;
				}
				s_dev_key = (uint8_t *)malloc(data_size + 1);
				if (s_dev_key)
				{
					memset(s_dev_key, 0, data_size + 1);
					memcpy(s_dev_key, dev_key_buff, data_size);
				}
			}
			free(dev_key_buff);
			dev_key_buff = NULL;
		}
	}
	else
	{
		idpslog(4,"ConfigParse", "dev key data_size err!");
	}

	ret = secCryptoDeinit();
	if (ret != 0)
	{
		idpslog(4,"ConfigParse", "secCryptoDeinit fail!");
	}
}

char* getTCUID()
{
	return tboxInfo_obj.ID;
}

char* getVIN()
{
	return tboxInfo_obj.VIN;
}

char* getCAR()
{
	return tboxInfo_obj.CAR;
}

// 获取靶场版本号
char* getSIMU()
{
	return tboxInfo_obj.SYS_VERSION;
}

char* getManufacturer()
{
	return tboxInfo_obj.MANUFACTURER;
}

// 落盘IDPS版本
int recordVesion(char *version)
{
	return writeVersion(BASE_VERSION_PATH, version);
}

unsigned char *get_pki_client_cert(void)
{
	return s_dev_cert;
}

unsigned char *get_pki_client_private_key(void)
{
	return s_dev_key;
}

unsigned char *get_pki_root_cert(void)
{
	return s_root_cert;
}

// 离线在线模式 1：在线 0：离线
static int s_network_connection_enabled = 0;

int networkFunctionEnabled(void)
{
	return s_network_connection_enabled;
}

// 读取离在线模式
void initFlagNetworkConnection(void)
{
	char str[8] = {0};
	s_network_connection_enabled = 1;
	int fd = open("./operate_mode", O_RDONLY);
	if (fd >= 0)
	{
		read(fd, str, sizeof(str) - 1);
		close(fd);
		
		if (atoi(str) != 0)
		{
			s_network_connection_enabled = 0;
		}
	}

	char log[255]={0};
	sprintf(log,"s_network_connection_enabled:%d\n",s_network_connection_enabled);
	idpslog(1,"ConfigParse",log);
}

/*使用本地时间初始化IDPS上传到平台的时间，防止无网状态下时间未同步，导致事件中timestamp错误的问题*/
int init_sync_clock(void)
{
    struct timeval temp;
    long long timestamp;

    gettimeofday(&temp,NULL);
    timestamp = (long long)1000*temp.tv_sec + temp.tv_usec/1000;

    clockobj.sync_clock(timestamp);

    return 0;
}

int conf_rw_path_init()
{
    DIR *dir = NULL;
    char system_buff[128] = {0};

    dir = opendir(ROOT_PATH_RW"/config");
    if (dir)
    {
        //printf("目录存在\n");
        closedir(dir);
    }
    else
    {
        snprintf(system_buff, sizeof(system_buff), "mkdir -p %s", ROOT_PATH_RW"/config");
        system(system_buff);
    }

    if (access(POLICY_CONFIG_PATH, F_OK) == 0)
    {
        //printf("文件存在\n");
    }
    else
    {
        memset(system_buff, 0, sizeof(system_buff));
        snprintf(system_buff, sizeof(system_buff), "cp %s %s", ROOT_PATH_OR"/config/policy_config.json", POLICY_CONFIG_PATH);
        system(system_buff);
    }

    return 0;
}


int comparison_cache_data(cache_t *data)
{
	
	cache_t cache;
    int fd = open(BASE_CACHE_PATH, O_RDWR | O_CREAT, 0644);
    if (fd >= 0) {
        ssize_t bytesRead = read(fd, &cache, sizeof(cache_t));
        
        if (bytesRead != sizeof(cache_t)) {
            memcpy(&cache, data, sizeof(cache_t));
            lseek(fd, 0, SEEK_SET); 
            ftruncate(fd, 0);        
            write(fd, &cache, sizeof(cache_t)); 
        } else {
            // 文件存在且完整，打印数据
            // printf("lvqinghe ID:%s\n", cache.ID);
            // printf("lvqinghe VIN:%s\n", cache.VIN);
            // printf("lvqinghe CAR:%s\n", cache.CAR);
            // printf("lvqinghe SERVER:%s\n", cache.SERVER);
            // printf("lvqinghe data URL:%s\n", cache.URL);
            // printf("lvqinghe data ID:%s\n", data->ID);
            // printf("lvqinghe data VIN:%s\n", data->VIN);
            // printf("lvqinghe data CAR:%s\n", data->CAR);
            // printf("lvqinghe data SERVER:%s\n", data->SERVER);
            // printf("lvqinghe data URL:%s\n", data->URL);
			int ret = strcmp(cache.ID,data->ID);
			ret |= strcmp(cache.VIN,data->VIN);
			ret |= strcmp(cache.CAR,data->CAR);
			ret |= strcmp(cache.SERVER,data->SERVER);
			ret |= strcmp(cache.URL,data->URL);
			if(ret != 0)
			{
				memcpy(&cache,data,sizeof(cache_t));
				lseek(fd, 0, SEEK_SET); 
            	ftruncate(fd, 0);       
           		write(fd, &cache, sizeof(cache_t)); 
				close(fd);
				return 1;
			}
        }
        close(fd);
    } else {
        return -1;
    }
    
    return 0;
}