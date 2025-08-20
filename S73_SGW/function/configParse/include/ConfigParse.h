
#ifndef __CONFIGPARSE_H__
#define __CONFIGPARSE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#include "cJSON.h"
#define CodeVersion  	"IDPS-Version-1.2.5"
#define Version			"1.2.5"
#define IDSVERSION 		"IDPS-Version-" Version

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
	int canSwitchFun;
	char* can;
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

typedef struct {
    char ID[256];           // 设备序列号
    char VIN[256];          // 车辆识别号码
    char CAR[256];          // 车型信息
    char SERVER[256];  		// https 地址
    char URL[256]; 			// websocket 地址
} cache_t;

static unsigned char *s_root_cert = NULL;
static unsigned char *s_dev_cert = NULL;
static unsigned char *s_dev_key = NULL;

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

int initTboxInfo(configData* configObj);
int initCert(void);
char* getTCUID();
char* getVIN();
char* getCAR();
char* getSIMU();
char* getManufacturer();
unsigned char *get_pki_client_cert(void);
unsigned char *get_pki_client_private_key(void);
unsigned char *get_pki_root_cert(void);

int networkFunctionEnabled(void);
void initFlagNetworkConnection(void);
int init_sync_clock(void);
int conf_rw_path_init();
int comparison_cache_data(cache_t *cache);
#ifdef __cplusplus
}
#endif
#endif