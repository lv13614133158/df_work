#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <clocker.h>
#include "base64.h"
#include "DnsModelList.pb-c.h"
#include "ServiceMonitorModelList.pb-c.h"
#include "NetworkMonitorModelList.pb-c.h"
#include "NetFlowModelList.pb-c.h"
#include "DeviceInfo.pb-c.h"
#include "AvgAndMemory.pb-c.h"
#include "AvgModelList.pb-c.h"
#include "AppInfoList.pb-c.h"
#include "UsageStatsList.pb-c.h"
#include "TBoxHardware.pb-c.h"
#include "funchook_protobuf.pb-c.h"
#include "AesManager.h"
#include "CJsonUtils.h"
#include "curlWrapper.h"
#include "common.h"

/**
 ******************************************************************************
 ** \简  述   从CJSON串中获取指定字段相关信息
 **  注  意   如果返回值为NULL,请不要free对应空间，否则必须free对应信息
 ** \参  数   response:source str getstr:待获取字段（file_id  sn  token  register_key  session_key  timestamp）
 ** \返回值   返回对应字段的信息
 ** \作  者  
 **  原函数   char* getFileIdFromUploadResponse(const char* response)
              char* getSN(char* responseData)
              char* getTokenKey(char* responseData)
              char* getManageKey(char* responseData)//register_key
              char* getSessionKey(char* responseData)//session_key）
              void getTimestampToInitClock(const char* responseData)//timestamp）
 ******************************************************************************/
char *getinfofromCJsonstr(const char* response,char* getstr)
{
    char* retstr = NULL;

    cJSON *cJSONArray = cJSON_Parse(response);
    cJSON *cJSONData  = cJSON_GetObjectItem(cJSONArray, "data");
    if (!cJSONArray) {
        return retstr;
    }
    cJSON *item = cJSON_GetObjectItem(cJSONData, getstr);
    if (!item) {
        cJSON_Delete(cJSONArray);
        return retstr;
    }

    if((strstr(getstr,"register_key") != NULL)||(strstr(getstr,"session_key") != NULL)){
		if (!item->valuestring) {
			cJSON_Delete(cJSONArray);
			return retstr;
		}

        size_t data_length;
        char* decode64_char = (char*)base64_decode((unsigned char*)item->valuestring, strlen(item->valuestring), &data_length);
	    retstr = (char*)malloc(strlen(decode64_char)+2);
        memset(retstr,0,strlen(decode64_char)+2);
        strcat(retstr,decode64_char);
        free(decode64_char);
    }
    else if(strstr(getstr,"timestamp") != NULL){
        long long time = item->valuedouble;

        clockobj.sync_clock(time);
        cJSON_Delete(cJSONArray);
       // long long newTime = get_current_time();
        return NULL;
    }
    else{
        retstr = (char* )malloc(strlen(item->valuestring) + 2);
        memset(retstr,0,strlen(item->valuestring) + 2);
        strcat(retstr,item->valuestring);
    }
    cJSON_Delete(cJSONArray);
    return retstr;
}
  

/**
 ******************************************************************************
 ** \简  述    获取response数据
 **  注  意    if !NULL please free
 ** \参  数     
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
char* changeToGetResponseData(parseResponseData_t *httpResponseData,int* _length)
{
    char *data = NULL;
    int len    =  0;
    if(httpResponseData->status_code!=0)
    {
        data = (char *)malloc(256);
        if(data == NULL)
            return NULL;
        memset(data,0,256);
        snprintf(data,255,"errorCode:%d\nerrorMsg:%s",httpResponseData->status_code,httpResponseData->errorMsg);
        len = 255;
    }else
    {
        len = strlen(httpResponseData->responseData);
        data = (char *)malloc(len+1);
        if(data == NULL)
            return NULL;
        memset(data,0,len+1);
        memcpy(data,httpResponseData->responseData,len);
    }
    *_length = len;
    return data;  
}

/**
 ******************************************************************************
 ** \简  述    根据key查找指定的字符串
 **  注  意    if !NULL please free
 ** \参  数     
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
char* getJsonString(cJSON *cJSONParent, const char* jsonKey){
    if (!cJSONParent) 
        return NULL;
    cJSON *item = cJSON_GetObjectItem(cJSONParent, jsonKey);
    if (!item) 
        return NULL;
    if (!item->valuestring) 
        return NULL;
    char* valueString = (char*)malloc(strlen(item->valuestring) + 2);
    memset(valueString,0,strlen(item->valuestring) + 2);
    snprintf(valueString,strlen(item->valuestring) + 1, "%s" , item->valuestring);
    return valueString;
}
 
/**
 ******************************************************************************
 ** \简  述    通用CJSON串解析以及获取CJSON获取大小 填充数据大小字段 填充sn字段 填充时间戳字段
 **  注  意    
 ** \参  数     
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
#define CJSONPARSE_GETSIZE(data,dnsModelList,sn,size,cJSONArray){\
    dnsModelList.sn = (char *)malloc(strlen(sn)+1);         \
    if(dnsModelList.sn==NULL)           \
        return NULL;                    \
    memset(dnsModelList.sn,0,strlen(sn)+1);\
    memcpy(dnsModelList.sn,sn,strlen(sn));\
    dnsModelList.timestamp = get_current_time();    \
    cJSONArray = cJSON_Parse(data);   \
    if (!cJSONArray) {                       \
        free(dnsModelList.sn);                  \
        return NULL;                     \
    }                                        \
    size =cJSON_GetArraySize(cJSONArray);\
    if (size == 0) {                         \
        cJSON_Delete(cJSONArray);            \
        free(dnsModelList.sn);              \
        return NULL;                     \
    }                                        \
    dnsModelList.n_data = size;       \
} 
/**
 ******************************************************************************
 ** \简  述    DNS-IP信息上传数据组包，经过一系列组包之后生成的是一个字符串
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    before:_Com__Qihoo360__Vehiclesafe__Model__DnsModelList  after: char*
 ** \作  者  
 
 ******************************************************************************/
char* convertFromJson(const char* sn, const char* data,uint32_t *length){

    int size = 0,ipsSize = 0;
    cJSON *cJSONArray = NULL;
    /******form list*******/
    Com__Qihoo360__Vehiclesafe__Model__DnsModelList dnsModelList = COM__QIHOO360__VEHICLESAFE__MODEL__DNS_MODEL_LIST__INIT;
    com__qihoo360__vehiclesafe__model__dns_model_list__init(&dnsModelList);
    CJSONPARSE_GETSIZE(data,dnsModelList,sn,size,cJSONArray);
    dnsModelList.data = (Com__Qihoo360__Vehiclesafe__Model__DnsModelList__DnsModel **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__DnsModelList__DnsModel *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        struct _Com__Qihoo360__Vehiclesafe__Model__DnsModelList__DnsModel *dnsModel = malloc(sizeof(struct _Com__Qihoo360__Vehiclesafe__Model__DnsModelList__DnsModel));
        com__qihoo360__vehiclesafe__model__dns_model_list__dns_model__init(dnsModel);
        dnsModel->dns = getJsonString(child, "dns");
        cJSON *ips = cJSON_GetObjectItem(child, "ips");
        ipsSize = cJSON_GetArraySize(ips);
        if (ipsSize > 0) {
            dnsModel->ips = (char **)malloc(sizeof(char *) *ipsSize);
            dnsModel->n_ips = ipsSize;
            for (int j = 0; j < ipsSize; j++) {
                cJSON *ip = cJSON_GetArrayItem(ips, j);
                if (ip && ip->valuestring) {
                    dnsModel->ips[j] = (char* )malloc(strlen(ip->valuestring) + 1);
                    snprintf(dnsModel->ips[j], strlen(ip->valuestring) + 1, "%s", ip->valuestring);
                }
            }
        }
        dnsModelList.data[i] = dnsModel;
    }
   
    /********from char********/
    size_t len = com__qihoo360__vehiclesafe__model__dns_model_list__get_packed_size(&dnsModelList);
    char *buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__dns_model_list__pack(&dnsModelList, buf);
    *length = len;
    /********free space********/
    for (int i = 0; i < size; i++){
        free(dnsModelList.data[i]->dns);
        if (ipsSize > 0) {
            for (int j = 0; j < ipsSize; j++) {
                if (dnsModelList.data[i]->ips[j] != NULL) {
                    free(dnsModelList.data[i]->ips[j]);
                }
            }
            free(dnsModelList.data[i]->ips);
        }
        free(dnsModelList.data[i]);
    }
    free(dnsModelList.data);
    free(dnsModelList.sn);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    服务监控信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    before:_Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList  after: char*
 ** \作  者  
 
 ******************************************************************************/
char* convertFromJsonTOServiceMonitor(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /********form list*********/
    Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList serviceMonitorModelList = COM__QIHOO360__VEHICLESAFE__MODEL__SERVICE_MONITOR_MODEL_LIST__INIT;
    CJSONPARSE_GETSIZE(data,serviceMonitorModelList,sn,size,cJSONArray);
    serviceMonitorModelList.data = (Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor *dnsModel = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor));
        com__qihoo360__vehiclesafe__model__service_monitor_model_list__service_parent_monitor__init(dnsModel);
        dnsModel->package_name = getJsonString(child,"package_name");
        cJSON *port_status_array = cJSON_GetObjectItem(child,"port_status");
        if (port_status_array) {
            int portSize = cJSON_GetArraySize(port_status_array);
            dnsModel->n_port_status = portSize;
            dnsModel->port_status = (Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor__ServiceMonitor **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor__ServiceMonitor *) *portSize);
            for (int j = 0; j < portSize; j++) {
                Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor__ServiceMonitor *serviceMonitor = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__ServiceMonitorModelList__ServiceParentMonitor__ServiceMonitor));
                com__qihoo360__vehiclesafe__model__service_monitor_model_list__service_parent_monitor__service_monitor__init(serviceMonitor);
                cJSON *childPort = cJSON_GetArrayItem(port_status_array, j);
                serviceMonitor->port = (cJSON_GetObjectItem(childPort,"port"))->valueint;

                cJSON *childStatus = cJSON_GetArrayItem(port_status_array, j);
                serviceMonitor->status = (cJSON_GetObjectItem(childStatus,"status"))->valueint;
                dnsModel->port_status[j] = serviceMonitor;
            }
        }
        serviceMonitorModelList.data[i] = dnsModel;
    }
    /******form char******/
    int len = com__qihoo360__vehiclesafe__model__service_monitor_model_list__get_packed_size(&serviceMonitorModelList);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__service_monitor_model_list__pack(&serviceMonitorModelList, buf);
    *length = len;
    /*******free space*******/
    for(int i = 0;i < size;i ++){
        cJSON *childfree = cJSON_GetArrayItem(cJSONArray, i);
        free(serviceMonitorModelList.data[i]->package_name);
        cJSON *port_status_arrayfree = cJSON_GetObjectItem(childfree,"port_status");
        if(port_status_arrayfree){
            int portSizefree = cJSON_GetArraySize(port_status_arrayfree);
            for(int j = 0; j < portSizefree; j++){
                free(serviceMonitorModelList.data[i]->port_status[j]);
            }
            free( serviceMonitorModelList.data[i]->port_status);
        }
        free(serviceMonitorModelList.data[i]);
    }
    free(serviceMonitorModelList.data);
    free(serviceMonitorModelList.sn);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    hook进程黑名单数据流化接口
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  20200903
 
 ******************************************************************************/
char* convertFromJsonTOHookAPIBlackList(const char* sn, const char* data,uint32_t *length)
{
    int size = 0;
    cJSON* cJSONArray = NULL;
    FuncapiblackMessage FuncapiblackMessageObj =  FUNCAPIBLACK_MESSAGE__INIT;
    //to from struct
    CJSONPARSE_GETSIZE(data,FuncapiblackMessageObj,sn,size,cJSONArray);
    FuncapiblackMessageObj.data = (FuncapiblackMessage__FuncData**)malloc(sizeof(FuncapiblackMessage__FuncData*)*size);
    for(int loop = 0;loop < size;loop ++){
        cJSON *child = cJSON_GetArrayItem(cJSONArray, loop);
        FuncapiblackMessage__FuncData *blacklistModel = malloc(sizeof(FuncapiblackMessage__FuncData));
        funcapiblack_message__func_data__init(blacklistModel);
        blacklistModel->processname = getJsonString(child,"processname");
        cJSON *ips_array = cJSON_GetObjectItem(child,"ips");
        if(ips_array){
            int ipSize = cJSON_GetArraySize(ips_array);
            blacklistModel->ips = (FuncapiblackMessage__FuncData__IpsData**)malloc(sizeof(FuncapiblackMessage__FuncData__IpsData*)*ipSize);
            blacklistModel->n_ips = ipSize;
            for(int loopstage1 = 0;loopstage1 < ipSize;loopstage1++){
                FuncapiblackMessage__FuncData__IpsData *ipmodel = malloc(sizeof(FuncapiblackMessage__FuncData__IpsData));
                funcapiblack_message__func_data__ips_data__init(ipmodel);
                cJSON *childIp = cJSON_GetArrayItem(ips_array, loopstage1);
                ipmodel->func_name = getJsonString(childIp,"func_name");
                ipmodel->pid       = cJSON_GetObjectItem(childIp,"pid")->valueint; 
                blacklistModel->ips[loopstage1] = ipmodel;
            }
        }
       FuncapiblackMessageObj.data[loop] = blacklistModel;
    }
    /*********from string*********/
    size_t len = funcapiblack_message__get_packed_size(&FuncapiblackMessageObj);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    funcapiblack_message__pack(&FuncapiblackMessageObj,buf);
    *length = len;
    //free space
     for(int i = 0;i < size ;i++){
        cJSON *childfree = cJSON_GetArrayItem(cJSONArray, i);
        cJSON *ips_arrayfree = cJSON_GetObjectItem(childfree,"ips");
        if(ips_arrayfree){
            int ipSizefree = cJSON_GetArraySize(ips_arrayfree);
            for(int j = 0;j < ipSizefree; j++){
                free(FuncapiblackMessageObj.data[i]->ips[j]->func_name);
                free(FuncapiblackMessageObj.data[i]->ips[j]);
            }
            free(FuncapiblackMessageObj.data[i]->ips);
        }
        free(FuncapiblackMessageObj.data[i]->processname);
        free(FuncapiblackMessageObj.data[i]);
    }
    free(FuncapiblackMessageObj.sn);
    free(FuncapiblackMessageObj.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    hook进程白名单数据流化接口
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  20200903
 
 ******************************************************************************/
char* convertFromJsonTOHookAPIWhiteList(const char* sn, const char* data,uint32_t *length)
{
    int size = 0;
    cJSON* cJSONArray = NULL;
    FuncapiwhiteMessage FuncapiwhiteMessageObj =  FUNCAPIWHITE_MESSAGE__INIT;
    //to from struct
    CJSONPARSE_GETSIZE(data,FuncapiwhiteMessageObj,sn,size,cJSONArray);
    FuncapiwhiteMessageObj.data = (FuncapiwhiteMessage__FuncData**)malloc(sizeof(FuncapiwhiteMessage__FuncData*)*size);
    for(int loop = 0;loop < size;loop ++){
        cJSON *child = cJSON_GetArrayItem(cJSONArray, loop);
        FuncapiwhiteMessage__FuncData *whitelistModel = malloc(sizeof(FuncapiwhiteMessage__FuncData));
        funcapiwhite_message__func_data__init(whitelistModel);
        whitelistModel->processname = getJsonString(child,"processname");
        cJSON *ips_array = cJSON_GetObjectItem(child,"ips");
        if(ips_array){
            int ipSize = cJSON_GetArraySize(ips_array);
            whitelistModel->ips = (FuncapiwhiteMessage__FuncData__IpsData**)malloc(sizeof(FuncapiwhiteMessage__FuncData__IpsData*)*ipSize);
            whitelistModel->n_ips = ipSize;
            for(int loopstage1 = 0;loopstage1 < ipSize;loopstage1++){
                FuncapiwhiteMessage__FuncData__IpsData *ipmodel = malloc(sizeof(FuncapiwhiteMessage__FuncData__IpsData));
                funcapiwhite_message__func_data__ips_data__init(ipmodel);
                cJSON *childIp = cJSON_GetArrayItem(ips_array, loopstage1);
                ipmodel->func_name = getJsonString(childIp,"func_name");
                ipmodel->pid       = cJSON_GetObjectItem(childIp,"pid")->valueint; 
                whitelistModel->ips[loopstage1] = ipmodel;
            }
        }
       FuncapiwhiteMessageObj.data[loop] = whitelistModel;
    }
    /*********from string*********/
    size_t len = funcapiwhite_message__get_packed_size(&FuncapiwhiteMessageObj);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    funcapiwhite_message__pack(&FuncapiwhiteMessageObj,buf);
    *length = len;
    //free space
     for(int i = 0;i < size ;i++){
        cJSON *childfree = cJSON_GetArrayItem(cJSONArray, i);
        cJSON *ips_arrayfree = cJSON_GetObjectItem(childfree,"ips");
        if(ips_arrayfree){
            int ipSizefree = cJSON_GetArraySize(ips_arrayfree);
            for(int j = 0;j < ipSizefree; j++){
                free(FuncapiwhiteMessageObj.data[i]->ips[j]->func_name);
                free(FuncapiwhiteMessageObj.data[i]->ips[j]);
            }
            free(FuncapiwhiteMessageObj.data[i]->ips);
        }
        free(FuncapiwhiteMessageObj.data[i]->processname);
        free(FuncapiwhiteMessageObj.data[i]);
    }
    free(FuncapiwhiteMessageObj.sn);
    free(FuncapiwhiteMessageObj.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    网络信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    before:_Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList  after: char*
 ** \作  者  
 
 ******************************************************************************/
char* convertFromJsonTONetworkMonitor(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList networkMonitorModelList = COM__QIHOO360__VEHICLESAFE__MODEL__NETWORK_MONITOR_MODEL_LIST__INIT;
    /*********form struct***********/
    CJSONPARSE_GETSIZE(data,networkMonitorModelList,sn,size,cJSONArray);
    networkMonitorModelList.data = (Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__NetworkMonitorModel **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__NetworkMonitorModel *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__NetworkMonitorModel *networkModel = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__NetworkMonitorModel));
        com__qihoo360__vehiclesafe__model__network_monitor_model_list__network_monitor_model__init(networkModel);
        networkModel->package_name = getJsonString(child,"package_name");
        cJSON *ips_array = cJSON_GetObjectItem(child,"ips");
        if (ips_array) {
            int ipSize = cJSON_GetArraySize(ips_array);
            networkModel->ips = (Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__IpModel **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__IpModel *) *ipSize);
            networkModel->n_ips = ipSize;
            for (int j = 0; j < ipSize; j++) {
                Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__IpModel *ipModel = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__NetworkMonitorModelList__IpModel));
                com__qihoo360__vehiclesafe__model__network_monitor_model_list__ip_model__init(ipModel);
                cJSON *childIp = cJSON_GetArrayItem(ips_array, j);
                ipModel->src_ip = getJsonString(childIp,"src_ip");
                ipModel->src_port = cJSON_GetObjectItem(childIp,"src_port")->valueint;
                ipModel->dest_ip = getJsonString(childIp,"dest_ip");
                ipModel->dest_port = cJSON_GetObjectItem(childIp,"dest_port")->valueint;
                ipModel->protocol = cJSON_GetObjectItem(childIp,"protocol")->valueint;
                ipModel->version = getJsonString(childIp,"version");
                ipModel->num = cJSON_GetObjectItem(childIp,"num")->valueint;
                networkModel->ips[j] = ipModel;
            }
        }
        networkMonitorModelList.data[i] = networkModel;
    }
    /*********form char***********/
    size_t len = com__qihoo360__vehiclesafe__model__network_monitor_model_list__get_packed_size(&networkMonitorModelList);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__network_monitor_model_list__pack(&networkMonitorModelList, buf);
    *length = len;
    /**********free space*****************/
    for(int i = 0;i < size ;i++){
        cJSON *childfree = cJSON_GetArrayItem(cJSONArray, i);
        cJSON *ips_arrayfree = cJSON_GetObjectItem(childfree,"ips");
        if(ips_arrayfree){
            int ipSizefree = cJSON_GetArraySize(ips_arrayfree);
            for(int j = 0;j < ipSizefree; j++){
                free(networkMonitorModelList.data[i]->ips[j]->src_ip);
                free(networkMonitorModelList.data[i]->ips[j]->dest_ip);
                free(networkMonitorModelList.data[i]->ips[j]->version);
                free(networkMonitorModelList.data[i]->ips[j]);
            }
            free(networkMonitorModelList.data[i]->ips);
        }
        free(networkMonitorModelList.data[i]->package_name);
        free(networkMonitorModelList.data[i]);
    }
    free(networkMonitorModelList.sn);
    free(networkMonitorModelList.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    网络流量上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    before:_Com__Qihoo360__Vehiclesafe__Model__NetFlowModelList  after: char*
 ** \作  者  
 
 ******************************************************************************/
char* convertFromJsonTONetFlowNodel(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /*****form struct*******/
    Com__Qihoo360__Vehiclesafe__Model__NetFlowModelList netFlowModelList = COM__QIHOO360__VEHICLESAFE__MODEL__NET_FLOW_MODEL_LIST__INIT;
    CJSONPARSE_GETSIZE(data,netFlowModelList,sn,size,cJSONArray);
    netFlowModelList.data = (Com__Qihoo360__Vehiclesafe__Model__NetFlowModelList__NetFlowModel **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__NetFlowModelList__NetFlowModel *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__NetFlowModelList__NetFlowModel *netFlowModel = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__NetFlowModelList__NetFlowModel));
        com__qihoo360__vehiclesafe__model__net_flow_model_list__net_flow_model__init(netFlowModel);
        netFlowModel->package_name = getJsonString(child,"package_name");
        netFlowModel->send_bytes = cJSON_GetObjectItem(child,"send_bytes")->valuedouble;
        netFlowModel->received_bytes = cJSON_GetObjectItem(child,"received_bytes")->valuedouble;
        netFlowModel->timestamp = cJSON_GetObjectItem(child,"timestamp")->valuedouble;
        netFlowModel->wifi_send_bytes = cJSON_GetObjectItem(child,"wifi_send_bytes")->valuedouble;
        netFlowModel->wifi_received_bytes = cJSON_GetObjectItem(child,"wifi_received_bytes")->valuedouble;
        netFlowModel->uid = cJSON_GetObjectItem(child,"uid")->valueint;
        netFlowModel->name = getJsonString(child,"name");
        netFlowModelList.data[i] = netFlowModel;
    }
    /*****form char******/
    size_t len = com__qihoo360__vehiclesafe__model__net_flow_model_list__get_packed_size(&netFlowModelList);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__net_flow_model_list__pack(&netFlowModelList, buf);
    *length = len;
    /*********free space**********/
    for(int j = 0;j< size;j ++){
        free(netFlowModelList.data[j]->package_name);
        free(netFlowModelList.data[j]->name);
        free(netFlowModelList.data[j]);
    }
    free(netFlowModelList.sn);
    free(netFlowModelList.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    设备信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
char *convertFromJsonTODeviceInfo(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /**********form struct*************/
    Com__Qihoo360__Vehiclesafe__Model__DeviceInfo deviceInfo = COM__QIHOO360__VEHICLESAFE__MODEL__DEVICE_INFO__INIT;
    CJSONPARSE_GETSIZE(data,deviceInfo,sn,size,cJSONArray);
    deviceInfo.data = (Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__DeviceData **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__DeviceData *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__DeviceData *deviceData = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__DeviceData));
        com__qihoo360__vehiclesafe__model__device_info__device_data__init(deviceData);
        struct _Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item1 *vehicleInfo = malloc(sizeof(struct _Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item1));
        com__qihoo360__vehiclesafe__model__device_info__item1__init(vehicleInfo);
        struct _Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item2 *vehicleMachineInfo = malloc(sizeof(struct _Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item2));
        com__qihoo360__vehiclesafe__model__device_info__item2__init(vehicleMachineInfo);
        cJSON *itemVehicleInfo = cJSON_GetObjectItem(child,"vehicle_info");
        vehicleInfo->model = getJsonString(itemVehicleInfo,"model");
        vehicleInfo->brand = getJsonString(itemVehicleInfo,"brand");
        vehicleInfo->vin = getJsonString(itemVehicleInfo,"vin");
        cJSON *itemVehicleMachineInfo = cJSON_GetObjectItem(child,"vehicle_machine_info");
        vehicleMachineInfo->series_number = getJsonString(itemVehicleMachineInfo,"sn");
        vehicleMachineInfo->board = getJsonString(itemVehicleMachineInfo,"board");
        vehicleMachineInfo->brand = getJsonString(itemVehicleMachineInfo,"brand");
        vehicleMachineInfo->manufacturer = getJsonString(itemVehicleMachineInfo,"manufacturer");
        vehicleMachineInfo->fingerprint = getJsonString(itemVehicleMachineInfo,"fingerprint");
        vehicleMachineInfo->device = getJsonString(itemVehicleMachineInfo,"device");
        vehicleMachineInfo->product = getJsonString(itemVehicleMachineInfo,"product");
        vehicleMachineInfo->model = getJsonString(itemVehicleMachineInfo,"model");
        vehicleMachineInfo->os_version = getJsonString(itemVehicleMachineInfo,"os_version");

        vehicleMachineInfo->rom_version = getJsonString(itemVehicleMachineInfo,"rom_version");
        vehicleMachineInfo->wifi_mac = getJsonString(itemVehicleMachineInfo,"wifi_mac");
        vehicleMachineInfo->bluetooth_mac = getJsonString(itemVehicleMachineInfo,"bluetooth_mac");
        vehicleMachineInfo->memory_size = cJSON_GetObjectItem(itemVehicleMachineInfo,"memory_size")->valuedouble;
        vehicleMachineInfo->storage_size = cJSON_GetObjectItem(itemVehicleMachineInfo,"storage_size")->valuedouble;
        vehicleMachineInfo->kernel_version = getJsonString(itemVehicleMachineInfo,"kernel_version");

        cJSON *network_card_infoVehicleMachineInfo = cJSON_GetObjectItem(itemVehicleMachineInfo,"network_card_info");
        int network_card_info_size =cJSON_GetArraySize(network_card_infoVehicleMachineInfo);
        if (network_card_info_size != 0) {
            vehicleMachineInfo->n_network_card_info = network_card_info_size;
            vehicleMachineInfo->network_card_info =
                    (Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item2__Item3 **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item2__Item3 *) *network_card_info_size);
            for (int k = 0; k < network_card_info_size; k++) {
                cJSON *network_card_info_child = cJSON_GetArrayItem(network_card_infoVehicleMachineInfo, k);
                struct _Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item2__Item3 *network_card_info = malloc(sizeof(struct _Com__Qihoo360__Vehiclesafe__Model__DeviceInfo__Item2__Item3));
                com__qihoo360__vehiclesafe__model__device_info__item2__item3__init(network_card_info);
                network_card_info->name = getJsonString(network_card_info_child,"name");
                network_card_info->display_name = getJsonString(network_card_info_child,"display_name");
                network_card_info->virtual_ = cJSON_GetObjectItem(network_card_info_child,"virtual")->valueint;
                network_card_info->hardware_address = getJsonString(network_card_info_child,"hardware_address");
                network_card_info->host_address = getJsonString(network_card_info_child,"host_address");
                network_card_info->host_name = getJsonString(network_card_info_child,"host_name");
                network_card_info->broadcast_address = getJsonString(network_card_info_child,"broadcast_address");
                network_card_info->mask_length = cJSON_GetObjectItem(network_card_info_child,"mask_length")->valueint;
                vehicleMachineInfo->network_card_info[k] = network_card_info;
            }
        }
        vehicleMachineInfo->security_patch = getJsonString(itemVehicleMachineInfo,"security_patch");
        vehicleMachineInfo->build_sdk = cJSON_GetObjectItem(itemVehicleMachineInfo,"build_sdk")->valueint;
        vehicleMachineInfo->sys_version = getJsonString(itemVehicleMachineInfo,"sys_version");
        vehicleMachineInfo->user_agent = getJsonString(itemVehicleMachineInfo,"user_agent");
        vehicleMachineInfo->build_id = getJsonString(itemVehicleMachineInfo,"build_id");
        vehicleMachineInfo->compile_type = getJsonString(itemVehicleMachineInfo,"compile_type");
        vehicleMachineInfo->version_name = getJsonString(itemVehicleMachineInfo,"version_name");
        vehicleMachineInfo->version_code = cJSON_GetObjectItem(itemVehicleMachineInfo,"version_code")->valueint;
        vehicleMachineInfo->hw_version = getJsonString(itemVehicleMachineInfo,"hw_version");
        deviceData->vehicle_info = vehicleInfo;
        deviceData->vehicle_machine_info = vehicleMachineInfo;
        deviceInfo.data[i] = deviceData;
    }
    /*********form char*************/
    size_t len = com__qihoo360__vehiclesafe__model__device_info__get_packed_size(&deviceInfo);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__device_info__pack(&deviceInfo, buf);
    *length = len;
    /***********free space**********/
    for(int j = 0;j < size ; j++){
        free(deviceInfo.data[j]->vehicle_info->model);
        free(deviceInfo.data[j]->vehicle_info->brand);
        free(deviceInfo.data[j]->vehicle_info->vin);
        free(deviceInfo.data[j]->vehicle_info);
        
        free(deviceInfo.data[j]->vehicle_machine_info->series_number);
        free(deviceInfo.data[j]->vehicle_machine_info->board);
        free(deviceInfo.data[j]->vehicle_machine_info->brand);
        free(deviceInfo.data[j]->vehicle_machine_info->manufacturer);
        free(deviceInfo.data[j]->vehicle_machine_info->fingerprint);
        free(deviceInfo.data[j]->vehicle_machine_info->device);
        free(deviceInfo.data[j]->vehicle_machine_info->product);
        free(deviceInfo.data[j]->vehicle_machine_info->model);
        free(deviceInfo.data[j]->vehicle_machine_info->os_version);
        free(deviceInfo.data[j]->vehicle_machine_info->rom_version);
        free(deviceInfo.data[j]->vehicle_machine_info->wifi_mac);
        free(deviceInfo.data[j]->vehicle_machine_info->bluetooth_mac);
        free(deviceInfo.data[j]->vehicle_machine_info->kernel_version);

        free(deviceInfo.data[j]->vehicle_machine_info->security_patch);
        free(deviceInfo.data[j]->vehicle_machine_info->sys_version);
        free(deviceInfo.data[j]->vehicle_machine_info->user_agent);
        free(deviceInfo.data[j]->vehicle_machine_info->build_id);
        free(deviceInfo.data[j]->vehicle_machine_info->compile_type);
        free(deviceInfo.data[j]->vehicle_machine_info->version_name);
        free(deviceInfo.data[j]->vehicle_machine_info->hw_version);

        for(int k = 0;k<deviceInfo.data[j]->vehicle_machine_info->n_network_card_info;k++)
        {
            free(deviceInfo.data[j]->vehicle_machine_info->network_card_info[k]->name);
            free(deviceInfo.data[j]->vehicle_machine_info->network_card_info[k]->display_name);
            free(deviceInfo.data[j]->vehicle_machine_info->network_card_info[k]->hardware_address);
            free(deviceInfo.data[j]->vehicle_machine_info->network_card_info[k]->host_address);
            free(deviceInfo.data[j]->vehicle_machine_info->network_card_info[k]->host_name);
            free(deviceInfo.data[j]->vehicle_machine_info->network_card_info[k]->broadcast_address);
            free(deviceInfo.data[j]->vehicle_machine_info->network_card_info[k]);
        }

        free(deviceInfo.data[j]->vehicle_machine_info->network_card_info);
        free(deviceInfo.data[j]->vehicle_machine_info);
        free(deviceInfo.data[j]);
    }
    free(deviceInfo.sn);
    free(deviceInfo.data);
    cJSON_Delete(cJSONArray);
    return buf ;
}
/**
 ******************************************************************************
 ** \简  述    硬件信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
char *convertFromJsonTOTBoxHardware(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /*********from struct*************/
    Com__Qihoo360__Vehiclesafe__Model__TBoxHardware tBoxHardware = COM__QIHOO360__VEHICLESAFE__MODEL__TBOX_HARDWARE__INIT;
    CJSONPARSE_GETSIZE(data,tBoxHardware,sn,size,cJSONArray);
    tBoxHardware.data = (Com__Qihoo360__Vehiclesafe__Model__TBoxHardware__TBoxInfo **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__TBoxHardware__TBoxInfo *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *itemVehicleInfo = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__TBoxHardware__TBoxInfo* tBoxInfo = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__TBoxHardware__TBoxInfo));
        com__qihoo360__vehiclesafe__model__tbox_hardware__tbox_info__init(tBoxInfo);
        tBoxInfo->sys_name = getJsonString(itemVehicleInfo,"sys_name");
        tBoxInfo->machine = getJsonString(itemVehicleInfo,"machine");
        tBoxInfo->hostname = getJsonString(itemVehicleInfo,"hostname");
        tBoxInfo->release= getJsonString(itemVehicleInfo,"release");
        tBoxInfo->ram_size = cJSON_GetObjectItem(itemVehicleInfo,"ram_size")->valuedouble;
        tBoxInfo->cpu_model_name = getJsonString(itemVehicleInfo,"cpu_model_name");
        tBoxInfo->cpu_num = cJSON_GetObjectItem(itemVehicleInfo,"cpu_num")->valueint;
        tBoxInfo->network_info = getJsonString(itemVehicleInfo,"network_info");
        tBoxInfo->imei = getJsonString(itemVehicleInfo,"imei");
        tBoxInfo->client_version_code = cJSON_GetObjectItem(itemVehicleInfo,"client_version_code")->valueint;
        tBoxInfo->client_version_name = getJsonString(itemVehicleInfo,"client_version_name");
        tBoxHardware.data[i] = tBoxInfo;
    }
    /**********form string*************/
    size_t len = com__qihoo360__vehiclesafe__model__tbox_hardware__get_packed_size(&tBoxHardware);
    char* buf = (uint8_t*)malloc(len + 8);
    memset(buf,0,len + 8);
    com__qihoo360__vehiclesafe__model__tbox_hardware__pack(&tBoxHardware, buf);
   *length = len;
    /***********free  space**********/
    for(int j = 0;j < size;j ++){
        free(tBoxHardware.data[j]->sys_name);
        free(tBoxHardware.data[j]->machine);
        free(tBoxHardware.data[j]->hostname);
        free(tBoxHardware.data[j]->release);
        free(tBoxHardware.data[j]->cpu_model_name);
        free(tBoxHardware.data[j]->network_info);
        free(tBoxHardware.data[j]->imei);
        free(tBoxHardware.data[j]->client_version_name);
        free(tBoxHardware.data[j]);
    }
    free(tBoxHardware.sn);
    free(tBoxHardware.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    平均负载信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
char *convertFromJsonTOAvgModel(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /**********from struct************/
    Com__Qihoo360__Vehiclesafe__Model__AvgModelList avgModelList = COM__QIHOO360__VEHICLESAFE__MODEL__AVG_MODEL_LIST__INIT;
    com__qihoo360__vehiclesafe__model__avg_model_list__init(&avgModelList);
     CJSONPARSE_GETSIZE(data,avgModelList,sn,size,cJSONArray);
    avgModelList.data = (Com__Qihoo360__Vehiclesafe__Model__AvgModelList__AvgModel **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__AvgModelList__AvgModel *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__AvgModelList__AvgModel *netFlowModel = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__AvgModelList__AvgModel));
        com__qihoo360__vehiclesafe__model__avg_model_list__avg_model__init(netFlowModel);
        netFlowModel->load_avg1 = cJSON_GetObjectItem(child,"load_avg1")->valuedouble;
        netFlowModel->load_avg5 = cJSON_GetObjectItem(child,"load_avg5")->valuedouble;
        netFlowModel->load_avg15 = cJSON_GetObjectItem(child,"load_avg15")->valuedouble;
        netFlowModel->load_process_rate = getJsonString(child,"load_process_rate");
        netFlowModel->pid = cJSON_GetObjectItem(child,"pid")->valuedouble;
        avgModelList.data[i] = netFlowModel;
    }
    /************from string****************/
    size_t len = com__qihoo360__vehiclesafe__model__avg_model_list__get_packed_size(&avgModelList);
    char* buf = (char*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__avg_model_list__pack(&avgModelList, buf);
    *length = len;
    /**************free space**********************/
    for(int j = 0;j< size;j++){
        free(avgModelList.data[j]->load_process_rate);
        free(avgModelList.data[j]);
    }
    free(avgModelList.sn);
    free(avgModelList.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    车机系统资源占用率信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
char  *convertFromJsonTOAvgAndMemory(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /********from struct**********/
    Com__Qihoo360__Vehiclesafe__Model__AvgAndMemory avgAndMemoryModelList = COM__QIHOO360__VEHICLESAFE__MODEL__AVG_AND_MEMORY__INIT;
    com__qihoo360__vehiclesafe__model__avg_and_memory__init(&avgAndMemoryModelList);
    CJSONPARSE_GETSIZE(data,avgAndMemoryModelList,sn,size,cJSONArray);
    avgAndMemoryModelList.data = (Com__Qihoo360__Vehiclesafe__Model__AvgAndMemory__AvgAndMemoryModel **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__AvgAndMemory__AvgAndMemoryModel *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__AvgAndMemory__AvgAndMemoryModel *netFlowModel = (Com__Qihoo360__Vehiclesafe__Model__AvgAndMemory__AvgAndMemoryModel*) malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__AvgAndMemory__AvgAndMemoryModel));
        com__qihoo360__vehiclesafe__model__avg_and_memory__avg_and_memory_model__init(netFlowModel);
        netFlowModel->load_avg1 = cJSON_GetObjectItem(child,"load_avg1")->valuedouble;
        netFlowModel->load_avg5 = cJSON_GetObjectItem(child,"load_avg5")->valuedouble;
        netFlowModel->load_avg15 = cJSON_GetObjectItem(child,"load_avg15")->valuedouble;
        netFlowModel->ram_rate = cJSON_GetObjectItem(child,"ram_rate")->valueint;
        netFlowModel->sd_rate = cJSON_GetObjectItem(child,"sd_rate")->valueint;
        netFlowModel->rom_rate = cJSON_GetObjectItem(child,"rom_rate")->valueint;
        netFlowModel->ram_size = cJSON_GetObjectItem(child,"ram_size")->valuedouble;
        netFlowModel->sd_size = cJSON_GetObjectItem(child,"sd_size")->valuedouble;
        netFlowModel->rom_size = cJSON_GetObjectItem(child,"rom_size")->valuedouble;
        avgAndMemoryModelList.data[i] = netFlowModel;
    }
    /**********from string************/
    size_t len = com__qihoo360__vehiclesafe__model__avg_and_memory__get_packed_size(&avgAndMemoryModelList);
    char* buf = (uint8_t*)malloc(len*2);
    memset(buf,0,len*2);
    com__qihoo360__vehiclesafe__model__avg_and_memory__pack(&avgAndMemoryModelList, buf);
    *length = len;
    /***********free space**********/
    for(int j = 0;j< size;j ++){
        free(avgAndMemoryModelList.data[j]);
    }
    free(avgAndMemoryModelList.sn);
    free(avgAndMemoryModelList.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    车机应用列表信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/
char *convertFromJsonTOAppInfoList(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /*******from struct********/
    Com__Qihoo360__Vehiclesafe__Model__AppInfoList appInfoList = COM__QIHOO360__VEHICLESAFE__MODEL__APP_INFO_LIST__INIT;
    CJSONPARSE_GETSIZE(data,appInfoList,sn,size,cJSONArray);
    appInfoList.data = (Com__Qihoo360__Vehiclesafe__Model__AppInfoList__AppInfo **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__AppInfoList__AppInfo *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__AppInfoList__AppInfo *appInfo = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__AppInfoList__AppInfo));
        com__qihoo360__vehiclesafe__model__app_info_list__app_info__init(appInfo);
        appInfo->package_name = getJsonString(child,"package_name");
        appInfo->name = getJsonString(child,"name");
        appInfo->version_code = cJSON_GetObjectItem(child,"version_code")->valueint;
        appInfo->version_name = getJsonString(child,"version_name");
        appInfo->is_system = cJSON_GetObjectItem(child,"is_system")->valueint;
        appInfo->timestamp = cJSON_GetObjectItem(child,"timestamp")->valuedouble;
        appInfo->is_power_boot = cJSON_GetObjectItem(child,"is_power_boot")->valueint;
        appInfo->status = getJsonString(child,"status");
        appInfo->path = getJsonString(child,"path");
        appInfo->hashcode = getJsonString(child,"hashcode");
        appInfo->permissions = getJsonString(child,"permissions");
        appInfo->md5 = getJsonString(child,"md5");
        appInfo->file_size = cJSON_GetObjectItem(child,"file_size")->valuedouble;
        appInfo->uid = cJSON_GetObjectItem(child,"uid")->valueint;
        appInfoList.data[i] = appInfo;
    }
    /********from string************/
    size_t len = com__qihoo360__vehiclesafe__model__app_info_list__get_packed_size(&appInfoList);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__app_info_list__pack(&appInfoList, buf);
    *length = len;
    /************space free*******************/
    for(int j = 0;j < size;j ++){
        free(appInfoList.data[j]->package_name);
        free(appInfoList.data[j]->name);
        free(appInfoList.data[j]->version_name);
        free(appInfoList.data[j]->status);
        free(appInfoList.data[j]->path);
        free(appInfoList.data[j]->hashcode);
        free(appInfoList.data[j]->permissions);
        free(appInfoList.data[j]->md5);
        free(appInfoList.data[j]);
    }
    free(appInfoList.sn);
    free(appInfoList.data);
    cJSON_Delete(cJSONArray);
    return buf;
}
/**
 ******************************************************************************
 ** \简  述    采集车机行为数据信息上传数据组包
 **  注  意    对应的数据使用完成以后的释放,此函数采用多层递归极易容易造成内存泄漏，请注意
 ** \参  数    sn:设备sn号（全局变量）  data:待组包数据
 ** \返回值    
 ** \作  者  
 
 ******************************************************************************/

char *convertFromJsonTOUsageStatsList(const char* sn, const char* data,uint32_t *length){
    int size = 0;
    cJSON *cJSONArray = NULL;
    /***********from struct***************/
    Com__Qihoo360__Vehiclesafe__Model__UsageStatsList usageStatsList = COM__QIHOO360__VEHICLESAFE__MODEL__USAGE_STATS_LIST__INIT;
    CJSONPARSE_GETSIZE(data,usageStatsList,sn,size,cJSONArray);
    usageStatsList.data = (Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStatsData **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStatsData *) *size);
    for (int i = 0; i < size; i++) {
        cJSON *child = cJSON_GetArrayItem(cJSONArray, i);
        Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStatsData *usageStatsData = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStatsData));
        com__qihoo360__vehiclesafe__model__usage_stats_list__usage_stats_data__init(usageStatsData);
        usageStatsData->uptime = cJSON_GetObjectItem(child,"uptime")->valuedouble;
        usageStatsData->runtime = cJSON_GetObjectItem(child,"runtime")->valuedouble;
        usageStatsData->begin_time = cJSON_GetObjectItem(child,"begin_time")->valuedouble;
        usageStatsData->end_time = cJSON_GetObjectItem(child,"end_time")->valuedouble;
        cJSON *apps_array = cJSON_GetObjectItem(child,"apps");
        if (apps_array) {
            int portSize = cJSON_GetArraySize(apps_array);
            usageStatsData->n_apps = portSize;
            usageStatsData->apps = (Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStats **)malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStats *) *portSize);
            for (int j = 0; j < portSize; j++) {
                Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStats *usageStats = malloc(sizeof(Com__Qihoo360__Vehiclesafe__Model__UsageStatsList__UsageStats));
                com__qihoo360__vehiclesafe__model__usage_stats_list__usage_stats__init(usageStats);
                cJSON *childApp = cJSON_GetArrayItem(apps_array, j);
                usageStats->package_name = getJsonString(childApp,"package_name");
                usageStats->launch_count = cJSON_GetObjectItem(childApp,"launch_count")->valueint;
                usageStats->total_time_in_foreground = cJSON_GetObjectItem(childApp,"total_time_in_foreground")->valuedouble;
                usageStats->last_time_used = cJSON_GetObjectItem(childApp,"last_time_used")->valuedouble;
                usageStatsData->apps[j] = usageStats;
            }
        }
        usageStatsList.data[i] = usageStatsData;
    }
    /************from string ****************/
    size_t len = com__qihoo360__vehiclesafe__model__usage_stats_list__get_packed_size(&usageStatsList);
    char* buf = (uint8_t*)malloc(len);
    memset(buf,0,len);
    com__qihoo360__vehiclesafe__model__usage_stats_list__pack(&usageStatsList, buf);
    *length = len;
    /**************free space****************/
    for(int j=0;j<size;j++ )
    {
        int portSize = usageStatsList.data[j]->n_apps;
        for(int k = 0;k<portSize;k++)
        {
            free(usageStatsList.data[j]->apps[k]->package_name);
            free(usageStatsList.data[j]->apps[k]);
        }
        free(usageStatsList.data[j]->apps);
        free(usageStatsList.data[j]);
    }
    free(usageStatsList.sn);
    free(usageStatsList.data);
    cJSON_Delete(cJSONArray);
    return buf;
}


