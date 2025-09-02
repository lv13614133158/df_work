/*
 * @Author: your name
 * @Date: 2020-06-09 01:19:42
 * @LastEditTime: 2020-07-07 07:07:45
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/src/resourcemonitor/resourcemonitor.c
 */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "ResourceMonitor.h"
#include "resourceStatus.h"
#include "cJSON.h"
#include "Base_networkmanager.h"
#include "CommonTimer.h"
#include "websocketmanager.h"
#include "common.h"

#if MODULE_RESOURSEMONITOR
static char *API_UPLOAD_LOADAVG           = "/api/v2/upload_load_avg";
static char *API_UPLOAD_RESOURCE_SNAPSHOT = "/api/v2/upload_avg_and_memory";
#define MAX(a,b)  (((a)>(b))?(a):(b))
typedef enum _WorkState {
    WORK_STATE_NORMAL,
    WORK_STATE_OVERLOAD
}WorkState;

static int isInitial = 0;
// 注意这里 Period为小于等于0代表不检测，Threshold小于等于0代表每个检测周期都上报
static int mCPULoadavgCollectPeriod;
static int mCPUOverLoadThreshold;
static int mCPUUsageRateThreshold;
static int mRAMUsageCollectPeriod;
static int mRAMOverSizeThreshold;
static int mROMUsageCollectPeriod;
static int mROMOverSizeThreshold;
static int mAllLoadCollectPeriod;
static WorkState mCPUWorkState = WORK_STATE_NORMAL;
static WorkState mRAMWorkState = WORK_STATE_NORMAL;
static WorkState mROMWorkState = WORK_STATE_NORMAL;
timerIdps *timerALLResourceMonitor = NULL;
timerIdps *timerCPUResourceMonitor = NULL;
timerIdps *timerRAMResourceMonitor = NULL;
timerIdps *timerROMResourceMonitor = NULL;
typedef void (*pTimeArrivedMethod)(void);

// 上报事件
static void checkCPUStatus(void);
static void checkRAMStatus(void);
static void checkROMStatus(void);
pResourceCpuLoadavgData getLoadavgInfo(void);
int getSDcardFreeSize();
int getSDcardUsage();
int getSDcardUsedSize();


/**
 *  @func:获取资源负载信息
 *  @param:void
 *  @return :string 
 *  @note  调用后需要释放资源,example
 *      <li> char *p = getLoadavgInfo();     </li>
 *      <li> ....                            </li>
 *      <li> if(p)                           </li>
 *      <li> { 
 *      <li>     free(p->loadProcessRate);    </li>
 *      <li>     free(p);                     </li>
 *      <li> }                                </li>
 */
pResourceCpuLoadavgData getLoadavgInfo(void)
{
    char *result = (char *)malloc(1024);
    memset(result, 0, 1024);
    FILE *fp;
    char *items[5] = {0};
    int i = 0;
    char *p = NULL;
    if(result == NULL)
        return NULL;

    fp = fopen("/proc/loadavg", "r");
    if (fp == NULL)
    {
        log_v("resoucemonitor","fopen file /proc/loadavg error");
        if (result)
        {
            free(result);
            result = NULL;
        }
        return NULL;
    }
    char* result_s = fgets(result, 1024, fp);
    if (strlen(result) == 0)
    {
        free(result);
        fclose(fp);
        return NULL;
    }
    
    fclose(fp);
    p = strtok(result, " ");
    items[i] = p;
    while (++i < 5)
    {
        p = strtok(NULL, " ");
        if (p == NULL || strlen(p) == 0)
        {
            if (result)
            {
                free(result);
                result = NULL;
            }
            return NULL;
        }
        items[i] = p;
    }
    
    pResourceCpuLoadavgData pResourceCpuLoadavgDataObj = (pResourceCpuLoadavgData)malloc(sizeof(ResourceCpuLoadavgData));
    pResourceCpuLoadavgDataObj->loadavg1 = atof(items[0]);
    pResourceCpuLoadavgDataObj->loadavg5 = atof(items[1]);
    pResourceCpuLoadavgDataObj->loadavg15 = atof(items[2]);

    pResourceCpuLoadavgDataObj->loadProcessRate = (char *)malloc(strlen(items[3])+1);
    if(pResourceCpuLoadavgDataObj->loadProcessRate == NULL)
    {
        if(result)
        {
            free(result);
            result = NULL;
        }
        return NULL;
    }
    memset(pResourceCpuLoadavgDataObj->loadProcessRate,0,strlen(items[3])+1);
    memcpy(pResourceCpuLoadavgDataObj->loadProcessRate,items[3],strlen(items[3]));
    pResourceCpuLoadavgDataObj->pid = atoll(items[4]);
    if (result)
    {
        free(result);
        result = NULL;
    }
    return pResourceCpuLoadavgDataObj;
}

static void onCPULoadinfoEvent(pResourceCpuLoadavgData loadavgData, int cpuUsageRate, int cpuCoreNum)
{
    char *s = NULL;
    cJSON *snapshot = cJSON_CreateObject();
    char loadavg1[32]={0};
    char loadavg5[32]={0};
    char loadavg15[32]={0};
    snprintf(loadavg1, sizeof(loadavg1)-1, "%.2f",loadavgData->loadavg1);
    snprintf(loadavg5, sizeof(loadavg5)-1, "%.2f",loadavgData->loadavg5);
    snprintf(loadavg15,sizeof(loadavg15)-1,"%.2f",loadavgData->loadavg15);
    cJSON_AddStringToObject(snapshot,"load_avg1",loadavg1);
    cJSON_AddStringToObject(snapshot,"load_avg5",loadavg5);
    cJSON_AddStringToObject(snapshot,"load_avg15",loadavg15);
    cJSON_AddNumberToObject(snapshot, "cpu_num", cpuCoreNum);
    cJSON_AddNumberToObject(snapshot, "cpu_load",loadavgData->loadavg1 / cpuCoreNum);
    cJSON_AddNumberToObject(snapshot, "cpu_usage",cpuUsageRate);
    s = cJSON_PrintUnformatted(snapshot);
    websocketMangerMethodobj.sendEventData("0010106000222",s,"EVENT_TYPE_RESOURCE_USAGE");
    if(snapshot)
        cJSON_Delete(snapshot);
    if(s)
        free(s); 
}

static void onRAMUsagesizeEvent(long long ramUsageSize, long long ramUsageRate)
{
    char *s = NULL;
    cJSON *snapshot = cJSON_CreateObject();
    cJSON_AddNumberToObject(snapshot, "ram_size",ramUsageSize);
    cJSON_AddNumberToObject(snapshot, "ram_rate",ramUsageRate);
    s = cJSON_PrintUnformatted(snapshot);
    websocketMangerMethodobj.sendEventData("0010106000322",s,"EVENT_TYPE_RESOURCE_USAGE");
    if(snapshot)
        cJSON_Delete(snapshot);
    if(s)
        free(s);   
}

static void onROMUsagesizeEvent(long long romUsageSize, long long romUsageRate)
{
    char *s = NULL;
    cJSON *snapshot = cJSON_CreateObject();
    cJSON_AddNumberToObject(snapshot, "rom_size",romUsageSize);
    cJSON_AddNumberToObject(snapshot, "rom_rate",romUsageRate);
    s = cJSON_PrintUnformatted(snapshot);
    websocketMangerMethodobj.sendEventData("0010106000422",s,"EVENT_TYPE_RESOURCE_USAGE");
    if(snapshot)
        cJSON_Delete(snapshot);
    if(s)
        free(s); 
}

// websocket资源快照上传
void uploadResourceSnapshot(void)
{
    char *s = NULL;
    pResourceCpuLoadavgData loadavgData = getLoadavgInfo();
    int cpuCoreNum = getCpuCoreNum();
    long long timestamp = clockobj.get_current_time();
    if(loadavgData == NULL)
        return;
      
    cJSON *snapshot = cJSON_CreateObject();
    char loadavg1[32]={0};
    char loadavg5[32]={0};
    char loadavg15[32]={0};
    snprintf(loadavg1, sizeof(loadavg1)-1, "%.2f",loadavgData->loadavg1);
    snprintf(loadavg5, sizeof(loadavg5)-1, "%.2f",loadavgData->loadavg5);
    snprintf(loadavg15,sizeof(loadavg15)-1,"%.2f",loadavgData->loadavg15);
    cJSON_AddStringToObject(snapshot,"load_avg1",loadavg1);
    cJSON_AddStringToObject(snapshot,"load_avg5",loadavg5);
    cJSON_AddStringToObject(snapshot,"load_avg15",loadavg15);
    cJSON_AddNumberToObject(snapshot, "cpu_num",cpuCoreNum);
    cJSON_AddNumberToObject(snapshot, "cpu_load",loadavgData->loadavg1 / cpuCoreNum);
    cJSON_AddNumberToObject(snapshot, "cpu_usage",getCPUUsage());
    long long ramUsageSize = getRAMTotalSize() - getRAMFreeSize();
    cJSON_AddNumberToObject(snapshot, "ram_size",ramUsageSize < 0 ? 0 : ramUsageSize);
    cJSON_AddNumberToObject(snapshot, "ram_rate",getRAMUsage());
    long long romUsageSize = getROMTotalSize() - getROMFreeSize();
    cJSON_AddNumberToObject(snapshot, "rom_size",romUsageSize < 0 ? 0 : romUsageSize);
    cJSON_AddNumberToObject(snapshot, "rom_rate",getROMUsage());
    cJSON_AddNumberToObject(snapshot, "timestamp",timestamp);
    s = cJSON_PrintUnformatted(snapshot);
    websocketMangerMethodobj.sendEventData("0010106000122",s,"EVENT_TYPE_RESOURCE_USAGE");
    if (loadavgData)
    {
        free(loadavgData->loadProcessRate);
        free(loadavgData);
        loadavgData = NULL;
    }
    if(snapshot)
        cJSON_Delete(snapshot);
    if(s)
        free(s);  
}

// cpu资源上传
void uploadCPULoadInfo(void)
{
    int cpu_usage = 0;
    int cpuCoreNum = getCpuCoreNum();

    pResourceCpuLoadavgData loadavgData = getLoadavgInfo();
    if(loadavgData == NULL)
        return;

    cpu_usage = getCPUUsage();

    onCPULoadinfoEvent(loadavgData, cpu_usage, cpuCoreNum);
    if (loadavgData)
    {
        free(loadavgData->loadProcessRate);
        free(loadavgData);
        loadavgData = NULL;
    }
}

// ram资源上传
void uploadRAMUsageSize(void)
{
    long long ramUsageSize = getRAMTotalSize() - getRAMFreeSize();
    long long ramUsageRate = getRAMUsage();
    ramUsageSize = MAX(ramUsageSize, 0);
    onRAMUsagesizeEvent(ramUsageSize, ramUsageRate);
}

// rom资源上传
void uploadROMUsageSize(void)
{
    long long romUsageSize = getROMTotalSize() - getROMFreeSize();
    long long romUsageRate = getROMUsage();
    romUsageSize = MAX(romUsageSize, 0);
    onROMUsagesizeEvent(romUsageSize, romUsageRate); 
}

// 定时器检测式上传部分
static void checkCPUStatus(void)
{
    int cpu_usage = 0;
    int cpuCoreNum = getCpuCoreNum();
    pResourceCpuLoadavgData loadavgData = getLoadavgInfo();
    if(loadavgData == NULL)
        return;

    cpu_usage = getCPUUsage();
    if (((loadavgData->loadavg1 / cpuCoreNum) > mCPUOverLoadThreshold) || cpu_usage > mCPUUsageRateThreshold)
        onCPULoadinfoEvent(loadavgData, cpu_usage, cpuCoreNum);
    if (loadavgData)
    {
        free(loadavgData->loadProcessRate);
        free(loadavgData);
        loadavgData = NULL;
    }    
}

static void checkRAMStatus(void)
{
    long long ramUsageSize = getRAMTotalSize() - getRAMFreeSize();
    long long ramUsageRate = getRAMUsage();
    ramUsageSize = MAX(ramUsageSize, 0);
    if(ramUsageRate > mRAMOverSizeThreshold)
        onRAMUsagesizeEvent(ramUsageSize, ramUsageRate);
}

static void checkROMStatus(void)
{
    long long romUsageSize = getROMTotalSize() - getROMFreeSize();
    long long romUsageRate = getROMUsage();
    romUsageSize = MAX(romUsageSize, 0);
    if(romUsageRate > mROMOverSizeThreshold)
        onROMUsagesizeEvent(romUsageSize, romUsageRate); 
}

void setCPUWorkParameter(int cpuPeriod, int cpuThreshold, int cpuUsageRateThreshold)
{
    mCPULoadavgCollectPeriod = MAX(cpuPeriod,0);
    mCPUOverLoadThreshold    = cpuThreshold;
    mCPUUsageRateThreshold = cpuUsageRateThreshold;
}

void setRAMWorkParameter(int ramPeriod, int ramThreshold)
{
    if (ramPeriod <= 0 || ramThreshold <= 0)
        log_e("resourcemonitor module","threshold incorrect parameter");
    mRAMUsageCollectPeriod = MAX(ramPeriod,0);
    mRAMOverSizeThreshold  = ramThreshold;
}

void setROMWorkParameter(int romPeriod, int romThreshold)
{
    if (romPeriod <= 0 || romThreshold <= 0)
        log_e("resourcemonitor module","threshold incorrect parameter");
    mROMUsageCollectPeriod = MAX(romPeriod,0);
    mROMOverSizeThreshold  = romThreshold;
}

void startCheckStatusMonitor(int index)
{
    switch (index)
    {
    case 0:
        if(timerCPUResourceMonitor || !mCPULoadavgCollectPeriod) 
            break;
        timerCPUResourceMonitor = timerObj.newtime(checkCPUStatus);
        timerObj.setInterval(mCPULoadavgCollectPeriod, timerCPUResourceMonitor);
        timerObj.starttime(timerCPUResourceMonitor);
        break;
    case 1:
        if(timerRAMResourceMonitor || !mRAMUsageCollectPeriod) 
            break;
        timerRAMResourceMonitor = timerObj.newtime(checkRAMStatus);
        timerObj.setInterval(mRAMUsageCollectPeriod, timerRAMResourceMonitor);
        timerObj.starttime(timerRAMResourceMonitor);
        break;
    case 2:
     if(timerROMResourceMonitor || !mROMUsageCollectPeriod) 
            break;
        timerROMResourceMonitor = timerObj.newtime(checkROMStatus);
        timerObj.setInterval(mROMUsageCollectPeriod, timerROMResourceMonitor);
        timerObj.starttime(timerROMResourceMonitor);
        break;
    }
}

void stopCheckStatusMonitor(int index) 
{
    switch (index)
    {
    case 0:
        if(!timerCPUResourceMonitor) break;
        timerObj.stoptimer(timerCPUResourceMonitor);
        mCPUWorkState = WORK_STATE_NORMAL;
        break;
    case 1:
         if(!timerRAMResourceMonitor) break;
        timerObj.stoptimer(timerRAMResourceMonitor);
        mRAMWorkState = WORK_STATE_NORMAL;
        break;
    case 2:
         if(!timerROMResourceMonitor) break;
        timerObj.stoptimer(timerROMResourceMonitor);
        mROMWorkState = WORK_STATE_NORMAL;
        break;
    }
}

void freeCheckStatusMonitor(int index)
{
    switch (index)
    {
    case 0:
        if(!timerCPUResourceMonitor) break;
        timerObj.freetimer(timerCPUResourceMonitor);
        timerCPUResourceMonitor = NULL;
        mCPUWorkState = WORK_STATE_NORMAL;
        break;
    case 1:
        if(!timerRAMResourceMonitor) break;
        timerObj.freetimer(timerRAMResourceMonitor);
        timerRAMResourceMonitor = NULL;
        mRAMWorkState = WORK_STATE_NORMAL;
        break;
    case 2:
        if(!timerROMResourceMonitor) break;
        timerObj.freetimer(timerROMResourceMonitor);
        timerROMResourceMonitor = NULL;
        mROMWorkState = WORK_STATE_NORMAL;
        break;
    }
}

int getSDcardUsage()
{
    return 0;
}

int getSDcardFreeSize()
{
    return 0;
}

int getSDcardUsedSize()
{
    return 0;
}

/*
*	function:getCPUUsage_Base
*	input   :void
*	output  :int
*	decla	:获取cpu利用率的百分比，返回int类型，如使用率是45.6%，则返回46
*/
int getCPUUsage_Base()
{
    return getCPUUsage();
}

/*
*	function:getRAMUsage_Base
*	input   :void
*	output  :int
*	decla	:获取RAM利用率的百分比，返回int类型，如使用率是45%，则返回45
*/
int getRAMUsage_Base()
{
    return getRAMUsage();
}

/*
*	function:getRAMFreeSize_Base
*	input   :void
*	output  :long long 
*	decla	:获取可用RAM大小，单位MB
*/
long long  getRAMFreeSize_Base()
{
    return getRAMFreeSize();
}

/*
*	function:getRAMTotalSize_Base
*	input   :void
*	output  :long long
*	decla	:获取RAM总大小，单位b
*/
long long  getRAMTotalSize_Base()
{
    return getRAMTotalSize();
}

/*
*	function:getROMUsage_Base
*	input   :void
*	output  :int
*	decla	:获取ROM利用率的百分比，返回int类型，如使用率是45.6%，则返回46
*/
int getROMUsage_Base()
{
    return getROMUsage();
}

/*
*	function:getROMFreeSize_Base
*	input   :void
*	output  :long long 
*	decla	:获取ROM可用空间大小，单位MB
*/
long long  getROMFreeSize_Base()
{
    return getROMFreeSize();
}

/*
*	function:getROMTotalSize_Base
*	input   :void
*	output  :long long 
*	decla	:获取ROM总大小,单位b
*/
long long  getROMTotalSize_Base()
{
    return getROMTotalSize();
}

/**
 * @func:初始化资源监控配置，设置默认cpu监控阈值，cpu监控回调时间,cpu状态标志 
 * @param: void
 * @return :void
 * @note: 启动cpu监控前调用
 **/
void initResourceMonitor(int Period)
{
    mAllLoadCollectPeriod = Period;
    mCPUWorkState = WORK_STATE_NORMAL;
    mRAMWorkState = WORK_STATE_NORMAL;
    mROMWorkState = WORK_STATE_NORMAL;
}

void startResourceMonitor(void)
{
    if(isInitial == 1)return;
    startCheckStatusMonitor(0);
    startCheckStatusMonitor(1);
    startCheckStatusMonitor(2);

    if(mAllLoadCollectPeriod >0)
    {
        timerALLResourceMonitor = timerObj.newtime(uploadResourceSnapshot);
        timerObj.setInterval(mAllLoadCollectPeriod, timerALLResourceMonitor);
        timerObj.starttime(timerALLResourceMonitor);
    }
    else
    {
       log_e("resourcemonitor module","threshold incorrect parameter");
    }

    isInitial = 1;
}

void stopResourceMonitor(void)
{
    stopCheckStatusMonitor(0);
    stopCheckStatusMonitor(1);
    stopCheckStatusMonitor(2);
}

void freeResourceMonitor(void)
{
    freeCheckStatusMonitor(0);
    freeCheckStatusMonitor(1);
    freeCheckStatusMonitor(2);
}

ResourceMonitor ResourceMonitorObj = {
    initResourceMonitor,
    startResourceMonitor,
    stopResourceMonitor,
    freeResourceMonitor,
    uploadResourceSnapshot,
    uploadCPULoadInfo,
    uploadRAMUsageSize,
    uploadROMUsageSize,
    getLoadavgInfo,
    setCPUWorkParameter,
    setRAMWorkParameter,
    setROMWorkParameter,
    startCheckStatusMonitor,
    stopCheckStatusMonitor,
    freeCheckStatusMonitor,
};

#endif