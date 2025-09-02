
#ifndef __CONFIGPARSE_H__
#define __CONFIGPARSE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#include "cJSON.h"
#include <sys/time.h>
#include <dirent.h>

// ... existing code ...
// 配置文件目录位置设置
#define POS_3  1
#ifdef POS_1
	#define ROOT_PATH  "./"
#elif POS_2
	#define ROOT_PATH  "/usr/local/idps"
#elif POS_3
	#define ROOT_PATH_OR  "/oemapp/etc/idps"
	#define ROOT_PATH_RW  "/oemdata/idps"
#elif POS_4
	#define ROOT_PATH_OR  "../conf"
	#define ROOT_PATH_RW  "/oemdata/idps"
#else
	#define ROOT_PATH  "/mnt/sdcard/idps"
#endif

#define DEVICE_INFO_PATH      ROOT_PATH_OR "/config/device_info.conf"
#define BASE_CONFIG_PATH   ROOT_PATH_OR "/config/base_config.json"
#define DEFAULT_POLICY_CONFIG_PATH ROOT_PATH_OR "/config/policy_config.json"	//初始化的配置文件
#define POLICY_CONFIG_PATH  ROOT_PATH_RW "/config/policy_config_cloud.json"			//云端获取的配置文件
#define PROCESS_WLIST_PATH ROOT_PATH_RW "/config/process_list.json"
#define POLICY_CONFIG_MD5  ROOT_PATH_RW "/config/save_conf.json"
#define BASE_VERSION_PATH  ROOT_PATH_RW "/version/version.ver"

#define GETREQUEST_DATALEN  (1024*5)

static unsigned char *s_root_cert = NULL;
static unsigned char *s_dev_cert = NULL;
static unsigned char *s_dev_key = NULL;

#define PKI_CERT_PATH      ROOT_PATH_RW "/data123"
#define PKI_CLIENT_CERT_NAME        PKI_CERT_PATH "/d2e64f06c8855b171ed9c0d952c1"
#define PKI_CLIENT_PRIVATE_KEY_NAME PKI_CERT_PATH "/d2e64f06c8855b171ed9c0d952c2"
#define PKI_ROOT_CERT_NAME          PKI_CERT_PATH "/d2e64f06c8855b171ed9c0d952c3"
#define IDS_CODE_VERSION "1.2.7"
#define IDS_VERSION "1.0.5"
#define INTERFACE_MAXSIZE     (128)    //网卡名长度
#define GENERALSET 	bool switchFun;\
					int  policyType;\
					int  terminalType;\
					int  eventLevel;
// Set是以功能划分
typedef struct _fileMonitorSet{
	GENERALSET
	char* watchPoints;
}fileMonitorSet;

typedef struct _networkMonitorSet{
	GENERALSET
	int  collectPeriod;
	char* IpWhiteList;
	char* PortWhiteList;
	char* DNSWhiteList;
}networkMonitorSet;

typedef struct _processMonitorSet{
	GENERALSET
	int  collectPeriod;
	char* whiteList;
}processMonitorSet;

typedef struct _shellMonitorSet{
	GENERALSET
}shellMonitorSet;

typedef struct _resourceMonitorSet{
	GENERALSET
	int cpu_collectPeriod;
	int cpu_overloadThreshold;
	int cpu_usageRateOverloadThreshold;
	int cpu_eventLevel;
	int ram_collectPeriod;
	int ram_overloadThreshold;
	int ram_eventLevel;
	int rom_collectPeriod;
	int rom_overloadThreshold;
	int rom_eventLevel;
	int collectPeriod;
}resourceMonitorSet;

typedef struct _networkTrafficSet{
	GENERALSET
	int  collectPeriod;
}networkTrafficSet;

typedef struct _networkInvasionSet{
	GENERALSET       
	char interface[INTERFACE_MAXSIZE];
	char* attackList;  //23个
	char* attackThreshold;
}networkInvasionSet;

typedef struct _firewallMonitorSet{
	GENERALSET       //无 eventLevel
	char* rules;
}firewallMonitorSet;

//9、logMonitor
typedef struct _logMonitorSet{
	GENERALSET      //无 eventLevel 为 level
	char storage_path[255];
	char node[256];
	char addr[256];
	char protocol[256];
	char token[256];
	char key[256];
	char uploadForm[256];  
}logMonitorSet;

typedef struct _moduleSet{
	long long updateTime;// 毫秒时间戳，充当对比版本号
	fileMonitorSet     fileMonitorObj; 
	shellMonitorSet    shellMonitorObj;
	networkMonitorSet  networkMonitorObj;
	processMonitorSet  processMonitorObj;
	resourceMonitorSet resourceMonitorObj;
	networkTrafficSet  networkTrafficObj;
	networkInvasionSet networkInvasionObj;
	firewallMonitorSet firewallMonitorObj;
	logMonitorSet      logMonitorObj;
}configSet,* pconfigSet;

typedef struct tboxInfo{
	char ID[50];
	char VIN[50];
	char CAR[50];
	char SYS_VERSION[50];
	char MANUFACTURER[50];
}tboxInfo_t;
extern tboxInfo_t tboxInfo_obj;

typedef struct tbox_gps_info{
	double latitude;
	double longitude;
}tbox_gps_info_t;

//获取本地mode :0 公共配置 1 :rpc置默认配置
int getLocalConfig(configData* configObj, int mode);
// 云端拉取rpc配置
int getCloudConfig(configSet* configSetObj);
// 字符rpc解析配置
int parseCloudConfig(char* str, configSet* configSetbj);
// rpc分析取云端缓存还是云端拉下来的
int selectParseConfig(char* responses, configSet* configSetObj);
// 添加版本信息
int recordVesion(char* version);

char *getProcessWhileList(void);
int   setProcessWhileList(char *white_process);
int conf_rw_path_init();
int initTboxInfo();
int initCert(void);
char* getTCUID();
char* getVIN();
char* getCAR();
char* getSIMU();
unsigned char *get_pki_client_cert(void);
unsigned char *get_pki_client_private_key(void);
unsigned char *get_pki_root_cert(void);
char* getManufacturer();

int networkFunctionEnabled(void);
void initFlagNetworkConnection(void);
int init_sync_clock(void);
int isAllZero(const char *str);

#ifdef __cplusplus
}
#endif
#endif
