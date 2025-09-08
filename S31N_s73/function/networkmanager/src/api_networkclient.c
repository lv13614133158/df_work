#include "api_networkclient.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include "data/dataToProtobufUtils.h"
#include "util/postHeaderUtil.h"
#include "data/httpResponseData.h"
#include "CJsonUtils.h"
#include "cJSON.h"
#include "aes/AesManager.h"
#include "Tool.h"
#include "ConfigConstData.h"
#include "DispatchManager.h"
#include "data/ConfigConstData.h"
#include "mysqlite.h"
#include "ThreadPool.h"
#include "curlWrapper.h"
#include <curl/curl.h>
#include "KeyManager.h" 

 /*
 * 说 明   :为了确保线程安全API的调用，主进程可以采用多线程模式调用，SO运行单例模式
 *          GET接口为了更好的跨线程，目前传入的_curl未使用，允许主线程多线程调用，增加响应的及时性
 * input   : 
 * output  :void
 * decla   :主句柄
 */
 static handle curlhandle = {.curlpost=NULL,.curlpostvalid = 0,.handlethread = 1};
/*
 * function:getSessionKey
 * input   :void
 * output  :char*
 * decla   :
 * modify  :20210906
 */
char* getSessionKeyApi(void){
    return getSessionKey(&curlhandle);
}
/*
 * function:getManageKeyApi
 * input   :void
 * output  :char*
 * decla   :
 * modify  :20210906
 */
char* getManageKeyApi(void){
    return getManageKey(&curlhandle);
}

 /*
 * function:进入低功耗模式
 * input   :void
 * output  :void
 * decla   :
 */
void lowerpower_enter(void){
    setlowerpower(true);
}
 /*
 * function:退出低功耗模式
 * input   :void
 * output  :void
 * decla   :
 */
void lowerpower_exit(void){
    setlowerpower(false);
}
 /*
 * function:查询当前的低功耗状态
 * input   :void
 * output  :void
 * decla   :为了与上层兼容 1：低功耗状态下   0：非低功耗状态下
 */
unsigned char lowerpower_states(void){
    return (getlowerpower() == true)?(1):(0);
}
 /*
 * function:initALLByMaster
 * input   :void
 * output  :void
 * decla   :(0)基本的初始化
 *          (1)工作目录的设置
 *          (2)idpsdb:mqtt目录，如/usrdata/mqtt.pem
 */
void initALLByMaster(char* certsPath,char* dir){
    strcpy(mqttpath,certsPath);
    set_work_directory(dir);
    NetWorkManagerSql();
    //Restart_UpdateTable();
}
/*
 * function:initTPSize
 * input   :_consumerTPSize:sonsumer size _producerTPSize:producersize
 * output  :void
 * decla   :(0)基本线程池参数的设置
 *          (1)消费者线程池，目前没有启用，目前是预留接口，设置1即可，不会对当前存在影响
 *          (2)生产者线程池，设置大于0的数值，目前建议设置1即可，数值越大内存占用越大
 */
void initTPSize(int _consumerTPSize,int _producerTPSize){
    setConsumeTPSize(_consumerTPSize);
    setProducerTPSize(_producerTPSize);
}
/*
 * function:setServerConfig
 * input   :_baseUrl:url
 * output  :void
 * decla   :(0)如function所示，就是设置baseurl,这里设置的是http://qa.service.car.360.cn
 *          (1)对指针变量，SO API不会做任何的操作,如是statck space,请注意空间的释放
 */
void setServerConfig(char* _baseUrl){
    setBaseUrl(_baseUrl);
}
/*
 * function:UploadFileSetTime
 * input   :long
 * output  :void
 * decla   :
 */
void UploadFileSetTime(long _stamp){
    //SetTimestamp_Upload(_stamp);
}
/*
 * function:UploadFileSetTime
 * input   :long
 * output  :void
 * decla   :
 */
long UploadFileGetTime(void){
    return 0;//GetTimestamp_Upload();
}
/*
 * function:setDeviceConfig
 * input   :_jsonstring:json string
 * output  :void
 * decla   :(0) 
 *          (1)设置uuid
 *          (2)设置channelId
 *          (3)设置equipmentType
 */
void  setDeviceConfig(char* sn,char* channel_id,char* equipment_type){
    if(sn != NULL)
        setUdid(sn);
    if(channel_id != NULL)
        setChannelId(channel_id);
    if(equipment_type != NULL)
        setEquipmentType(equipment_type);
}
/*
 * function:setManageKeyStore
 * input   :_mode:模式
 * output  :void
 * decla   :(0)设施key_iv的模式（目前设置1即可）
 */
void setManageKeyStore(int  _mode){
    initMode(_mode);
}
/*
 * function:postGatherData
 * input   :（1）_url： 基本url类型，如"/api/v2/service_monitor"
            （2）_data: 发送的数据串，如"[{\"dns\":\"dnstest\",\"ips\":[\"192.168.21.2\"]}]"
            （3）_priority:数据串的优先级，目前未使用，统一写1即可
 * output  :void
 * decla   :(0)发送数据，由于该function会线程池里压数据，所以无返回值
 *          (1)对指针变量，SO API不会做任何的操作
 */
void postGatherData(char* _url,char* _data,int _priority){
    produceGatherDataRequest(_url, _data, _priority,&curlhandle);
}
/*
 * function:postEventData
 * input   :（1）_policy_id:策略ID
            （2）_data:待发送的数据
            （3）_ticket_id:ticket_id
            （4）_priority:优先级

 * output  :void
 * decla   :(0)发送数据，由于该function会线程池里压数据，所以无返回值
 *          (1)对指针变量，SO API不会做任何的操作
 */
void postEventData(const char* _policy_id,char* _data,char* _ticket_id,int _priority){
    produceEventDataRequest( _policy_id, _data, _ticket_id, _priority,&curlhandle);
}
/*
 * function:postEventDataWithAttachment(目前未使用)
 * input   :（1）_policy_id:策略ID
            （2）_data:待发送的数据
            （3）_ticket_id:ticket_id
            （4）_priority:优先级
            （5）_path    :路径（如"/usrdata/1.txt"）

 * output  :void
 * decla   :(0)带附属文件的发送
 *          
 */
void postEventDataWithAttachment (const char *_policy_id, const char *_data, const char *_ticket_id, const char *_path, int _priority){
    //produceEventDataRequestWithAttachment( "79797979","testdata", "0", "/home/root/SKYGO/log/spdlog/haha.tar.gz", 1);
    produceEventDataRequestWithAttachment( _policy_id, _data, _ticket_id, _path, _priority);
}
/*
 * function:start
 * input   :void
 * output  :void
 * decla   :(0)启动生产者线程池，必须要启动，负责一些数据无法发送   
 */
void start(void){
    curl_init();
    //starthread();
}
/*
 * function:postHeartbeatData
 * input   :（1）_url:j
 * output  :void
 * decla   :(0)主要目的是发送心跳数据帧，属于非同步发送（即往线程池里填充的方式）（目前未使用，使用时需要测试）   
 */
void postHeartbeatData(char* _url){
     produceHeartbeatRequest(_url,&curlhandle);
}
/*
 * function:stop
 * input   :void
 * output  :void
 * decla   :(0)做必要的线程池资源释放
 */
void stop(void){
    stopthread();
}
/*
 * function:startReRequestFromDB
 * input   :void
 * output  :void
 * decla   :(0)一些数据因为网络或者其他原因未发送成功保存到数据库，
 * 此function的目的是读数据库进行数据重新发送，返回true时表明启动
 * 成功，返回false表明当前网络忙，稍后再试。
 */
void startReRequestFromDB(void){
    threadpool_add(producerPtr,produceAllRequestFromDb,(void *)NULL);      
}
/*
 * function:registernet
 * input   :void
 * output  :void
 * decla   :(0)基本注册，主要包括获取sn，获取managekey操作
 */
int registernet(void){
    if(getSN(&curlhandle) != NULL && getManageKey(&curlhandle)!=NULL)
        return true;
    return false;
}
/*
 * function:getSn
 * input   :void
 * output  :void
 * decla   :(0)获取当前SN
 */
char* getSn(void){
    char *sn = getSN(&curlhandle);
    return sn;
}

/*
 * function:getToken
 * input   :void
 * output  :void
 * decla   :(0)获取token
 */
char* getToken(void){
    char *token = gettoken(&curlhandle);
    return token;
}

/*
 * function:getRequestData
 * input   :（1）_url:如"/api/v1/file_monitor_list"
            （2）_updateTime:如1584952302124
            （3）待传数据指针
            （4）返回数据长度
             (5) _responsedatalen:response data区域大小
 * output  :void
 * decla   :(0)获取数据
 * modify  :20200708
 */
void getRequestData(char* _url,long long _updateTime,char** _responsedata,int* _length,int _responsedatalen){
    char* ptr = NULL;
    
    char *URL = getUrlWithUpdateTime((char *)_url, _updateTime, getSn());
    if(URL == NULL){
        *_length = 0;
         
        return;
    }
    char *header = getGetRequestHeaders(&curlhandle);
    if(header == NULL){
        *_length = 0;
        free(URL);
         
        return;
    }
    parseResponseData_t *httpResponseData = startGet(URL,header,&curlhandle);
    if(httpResponseData == NULL){
        *_length = 0;
        free(URL);
        free(header);
       
        return;
    }
    char *responseData = (char *)changeToGetResponseData(httpResponseData,_length);  
    freeResponseDataByDecyptMemory(httpResponseData);
    if(*_length > _responsedatalen){
        ptr = (char*)realloc(*_responsedata,*_length);
        if(ptr != NULL){
            memset(ptr,0,*_length);
            *_responsedata = ptr;
        }
    }
    memcpy(*_responsedata,responseData,*_length);
    free(responseData);
    free(URL);
    free(header);
}
/*
 * function: getRequestData
 * input   :（1）_url:如"/api/v1/file_monitor_list"
            （2）_responsedata:待传数据指针
            （3）_length      :返回数据长度
             (5) _responsedatalen:response data区域大小
 * output  :void
 * decla   :(0)和getRequestData功能一致
 * date    :20200708
 */
void getRequestDatanodate(char* _url,char** _responsedata,int* _length,int _responsedatalen){
    uint16_t l_urllen = strlen(getBaseUrl()) + strlen(_url);
    char cUrl[l_urllen + 2];
    char* ptr = NULL;
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,_url);
    char *header = getGetRequestHeaders(&curlhandle);
    parseResponseData_t *httpResponseData = startGet(cUrl,header,&curlhandle);
    char *responseData = changeToGetResponseData(httpResponseData,_length);  //返回值需要释放空间
    freeResponseDataByDecyptMemory(httpResponseData);
    if(*_length > _responsedatalen){
        ptr = (char*)realloc(*_responsedata,*_length);
        if(ptr != NULL){
            memset(ptr,0,*_length);
            *_responsedata = ptr;
        }
    }
    memcpy(*_responsedata,responseData,*_length);
    free(responseData);
    free(header);
}
/*
 * function:postHeartbeatDataSync
  * input   :（1）_url:基本url
             （2）_responseData:待传数据指针
             （3）inputlength  :返回数据长度
              (5) _responsedatalen:response data区域大小
 * output  :void
 * decla   :(0)发送心跳数据，直接发送
 */
void postHeartbeatDataSync(char* _url,char** _responseData,int *inputlength,int _responsedatalen){
    char *ptr = NULL;
    int l_urllen = 0;
    char *urlString = NULL;
    char *bodyString = NULL;
    char paddr[] = "/api/v3/heart_beat";

    if(_url != NULL)
    {
        if(_url && strlen(_url)>0){
            l_urllen = strlen(getBaseUrl()) + strlen(_url);
            urlString = (char*)malloc(l_urllen + 2);
            memset(urlString,0,l_urllen + 2);
            strncpy(urlString,getBaseUrl(),strlen(getBaseUrl()));
            strcat(urlString,_url);
        }
    }
    if(urlString == NULL)
    {
        l_urllen = strlen(getBaseUrl()) + strlen(paddr);
        urlString = (char*)malloc(l_urllen + 2);
        memset(urlString,0,l_urllen + 2);
        strncpy(urlString,getBaseUrl(),strlen(getBaseUrl()));
        strcat(urlString,paddr);
    }
    bodyString = getProtobufForHeartbeat(&curlhandle);
    cypher_t *cypher = encyptDataByAesAndSessionKey(bodyString,0,&curlhandle);
    if(cypher == NULL)
    {
        free(bodyString);
        free(urlString);
        return;
    }
    char *header = getHeaders((const char *)cypher->data,cypher->len_data,&curlhandle);
    if(header == NULL)
    {
        free(bodyString);
        free(urlString);
        free(cypher);
        return;
    }
    parseResponseData_t *httpResponseData = startPost(urlString,header,0,(char *)cypher->data,cypher->len_data,&curlhandle);
    if(httpResponseData == NULL)
    {
        free(bodyString);
        free(urlString);
        free(cypher);
        free(header);
        return;
    }
    char *responseData = changeToGetResponseData(httpResponseData,inputlength);
    free(bodyString);
    free(urlString);
    free(cypher);
    free(header);
    freeResponseDataByDecyptMemory(httpResponseData);
    if(*inputlength > _responsedatalen){
        ptr = (char*)realloc(*_responseData,*inputlength);
        if(ptr != NULL){
            memset(ptr,0,*inputlength);
            *_responseData = ptr;
        }
    }
    memcpy(*_responseData,responseData,*inputlength);
    free(responseData);
}
