
#include "data/dataToProtobufUtils.h"
#include "protobuf/Session.pb-c.h"
#include "util/Tool.h"
#include "cryptogram.h"
#include "AesManager.h"
#include "base64.h"
#include "protobuf/SN.pb-c.h"
#include "data/ConfigConstData.h"
#include "protobuf/HeartBeatModel.pb-c.h"
#include "KeyManager.h"
#include "protobuf/DnsModelList.pb-c.h"
#include "CJsonUtils.h"
#include "data/FileWrapper.h"
#include "protobuf/FileModel.pb-c.h"
#include "protobuf/AlarmEventModelList.pb-c.h"
#include "clocker.h"
#include "util/log_util.h"
#include <stdint.h>
#include "spdloglib.h"

#define URL_UPLOAD_DNS      "upload_dns"
#define URL_SERVICE_MONITOR "service_monitor"
#define URL_NETWORK_MONITOR "network_monitor"
#define URL_NET_FLOW_MONITOR "net_flow_monitor"
#define URL_UPLOAD_HARDWARE_DATA "upload_hardware_data"
#define URL_UPLOAD_TBOX     "tboxes"
#define URL_UPLOAD_LOAD_AVG "upload_load_avg"
#define URL_UPLOAD_AVG_AND_MEMORY "upload_avg_and_memory"
#define URL_UPLOAD_APP_LIST "upload_app_list"
#define URL_UPLOAD_USAGE_STATS "upload_usage_stats"
#define URL_UPLOAD_HOOKAPI_BLACK "hookAPI_upload_black"
#define URL_UPLOAD_HOOKAPI_WHITE "hookAPI_upload_whiter"
#define DEVICE_TYPE_TBOX    "tbox"
/**
 ******************************************************************************
 ** \简  述  
 **  注  意  返回值需要释放，防止内存泄漏
 ** \参  数
 ** @datemodify:20200120  
 ******************************************************************************/
char *getProtobufFromGaterData(const char* url, const char* body,uint32_t *len,void* _curl)
{
    char *sn = getSN(_curl);
    if(sn == NULL)
	    return NULL;
    char * data = NULL;
    if (strstr(url, URL_UPLOAD_DNS)) 
        data = convertFromJson(sn, body,len);
    else if (strstr(url, URL_SERVICE_MONITOR)) 
        data = convertFromJsonTOServiceMonitor(sn, body,len);
    else if (strstr(url, URL_NETWORK_MONITOR)) 
        data = convertFromJsonTONetworkMonitor(sn, body,len);
    else if (strstr(url, URL_NET_FLOW_MONITOR))
        data = convertFromJsonTONetFlowNodel(sn, body,len);
    else if (strstr(url, URL_UPLOAD_HARDWARE_DATA)) 
        data = convertFromJsonTODeviceInfo(sn, body,len);
    else if (strstr(url, URL_UPLOAD_TBOX)) 
        data = convertFromJsonTOTBoxHardware(sn, body,len);
    else if (strstr(url, URL_UPLOAD_LOAD_AVG)) 
        data = convertFromJsonTOAvgModel(sn, body,len);
    else if (strstr(url, URL_UPLOAD_AVG_AND_MEMORY)) 
        data = convertFromJsonTOAvgAndMemory(sn, body,len);
    else if (strstr(url, URL_UPLOAD_APP_LIST)) 
        data = convertFromJsonTOAppInfoList(sn, body,len);
    else if (strstr(url, URL_UPLOAD_USAGE_STATS)) 
        data = convertFromJsonTOUsageStatsList(sn, body,len);
    else if(strstr(url,URL_UPLOAD_HOOKAPI_BLACK))
        data = convertFromJsonTOHookAPIBlackList(sn,body,len);
    else if(strstr(url,URL_UPLOAD_HOOKAPI_WHITE))
        data = convertFromJsonTOHookAPIWhiteList(sn,body,len);
    else 
        printf("please confirm your protocolbuf format!!!\n");
    return data;
}
/**
 * need to free
 */ 
char *getAlarmEventProtobufFromGaterData(const char *policy_id, const char *data, const char *ticket_id, const char *error_msg, const char* attachment_id,int* retlength,void* _curl)
{
    uint8_t *buf = NULL; 
    int8_t   *l_attachment_id  = NULL,*l_error_msgbuffer = NULL,*l_ticket_idbuffer = NULL;            
    size_t len = 0;                   
    Com__Qihoo360__Vehiclesafe__Model__AlarmEventModelList alarmEventModelList = COM__QIHOO360__VEHICLESAFE__MODEL__ALARM_EVENT_MODEL_LIST__INIT;
    char* local_sn = NULL;
    local_sn = getSN(_curl);
    if(local_sn == NULL)return NULL;
    uint16_t l_snlen = strlen(local_sn);
    int8_t   l_sn[l_snlen + 1];
    memset(l_sn,0,l_snlen + 1);
    strncpy(l_sn,getSN(_curl),l_snlen);
    alarmEventModelList.sn = l_sn;

    if(policy_id == NULL)return NULL;
    uint16_t l_policy_idlen = strlen(policy_id);
    int8_t   l_policy_id[l_policy_idlen + 1];
    memset(l_policy_id,0,l_policy_idlen + 1);
    memcpy(l_policy_id,policy_id,l_policy_idlen);
    alarmEventModelList.policy_id = l_policy_id;
 
    if(data == NULL)return NULL;
    uint16_t l_datalen = strlen(data);
    int8_t   databuffer[l_datalen + 1];
    memset(databuffer,0,l_datalen + 1);
    memcpy(databuffer,data,l_datalen);
    alarmEventModelList.data = databuffer;

    alarmEventModelList.timestamp = get_current_time();
    if(attachment_id != NULL && strlen(attachment_id)>0)
    {
        uint16_t l_attachment_idlen = strlen(attachment_id);
        l_attachment_id = (int8_t* )malloc(l_attachment_idlen + 1);
        memset(l_attachment_id,0,l_attachment_idlen + 1);
        memcpy(l_attachment_id,attachment_id,l_attachment_idlen);
        alarmEventModelList.attachment_id = l_attachment_id;
    }
    if(error_msg != NULL && strlen(error_msg)>0)
    {
        uint16_t l_error_msg = strlen(error_msg);
        l_error_msgbuffer = (int8_t*)malloc(l_error_msg + 1);
        memset(l_error_msgbuffer,0,l_error_msg + 1);
        memcpy(l_error_msgbuffer,error_msg,l_error_msg);
        alarmEventModelList.attachment_error_msg = l_error_msgbuffer;
    }
    if(ticket_id != NULL && strlen(ticket_id)>0)
    {
        uint16_t l_ticket_idlen = strlen(ticket_id);
        l_ticket_idbuffer = (int8_t* )malloc(l_ticket_idlen + 1);
        memset(l_ticket_idbuffer,0,l_ticket_idlen + 1);
        memcpy(l_ticket_idbuffer,ticket_id,l_ticket_idlen);
        alarmEventModelList.ticket_id = l_ticket_idbuffer;
    }
    len = com__qihoo360__vehiclesafe__model__alarm_event_model_list__get_packed_size(&alarmEventModelList);
    *retlength = len;
    buf = (char*)malloc(len + 2);
    if(buf == NULL){
        if(l_ticket_idbuffer != NULL)free(l_ticket_idbuffer);
        if(l_error_msgbuffer != NULL)free(l_error_msgbuffer);
        if(l_attachment_id != NULL)free(l_attachment_id);
        return NULL;
    }
    memset(buf,0,len + 2);
    com__qihoo360__vehiclesafe__model__alarm_event_model_list__pack(&alarmEventModelList, buf);
    if(l_ticket_idbuffer != NULL)free(l_ticket_idbuffer);
    if(l_error_msgbuffer != NULL)free(l_error_msgbuffer);
    if(l_attachment_id != NULL)free(l_attachment_id);
    return (char *)buf;
}
/**
 ******************************************************************************
 ** \简  述  
 **  注  意  返回值不为NULL需要释放，防止内存泄漏
 ** \参  数
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getFileModelProtobufFromFileData(const FileWrapper_t  *fileWrapper,void* _curl)
{
    uint8_t *buf = NULL;                      
    size_t len  = 0;                  
    Com__Qihoo360__Vehiclesafe__Model__FileModel fileModel = COM__QIHOO360__VEHICLESAFE__MODEL__FILE_MODEL__INIT;
 
    uint16_t l_snlen = strlen(getSN(_curl));
    int8_t   l_sn[l_snlen + 1];
    memset(l_sn,0,l_snlen + 1);
    strncpy(l_sn,getSN(_curl),l_snlen);
    fileModel.sn = l_sn;
    uint32_t l_filelen = fileWrapper->fileDataLen;
    //int8_t   l_filebuff[l_filelen + 1];

    int8_t   *l_filebuff = NULL;
    l_filebuff = malloc(l_filelen + 1);
    if(l_filebuff == NULL)
        printf("malloc file error");
    memset(l_filebuff,0,l_filelen + 1);
    strncpy(l_filebuff,fileWrapper->fileData,fileWrapper->fileDataLen);
    fileModel.file.data = l_filebuff;
    fileModel.file.len = fileWrapper->fileDataLen;

    len = com__qihoo360__vehiclesafe__model__file_model__get_packed_size(&fileModel);
    buf = (uint8_t*)malloc(len*2);
    if(buf == NULL)
        return NULL;
    memset(buf,0,len*2);
    com__qihoo360__vehiclesafe__model__file_model__pack(&fileModel, buf);
    free(l_filebuff);
    return (char *)buf;
}
/**
 ******************************************************************************
 ** \简  述  序列化注册需要的信息
 **  注  意  返回值需要释放，防止内存泄漏
 ** \参  数
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getProtobufForManageKey()
{
    Com__Qihoo360__Vehiclesafe__Model__SN model__sn = COM__QIHOO360__VEHICLESAFE__MODEL__SN__INIT;
    uint16_t l_udidlen = strlen(getUdid());
    uint16_t l_ptypelen = strlen(getEquipmentType());
    uint16_t l_pidlen = strlen(getChannelId());
    int8_t udid[l_udidlen + 1],ptype[l_ptypelen + 1],pid[l_pidlen + 1];
    memset(udid,0,l_udidlen+1);
    memset(ptype,0,l_ptypelen + 1);
    memset(pid,0,l_pidlen + 1);
    memcpy(udid,getUdid(),l_udidlen);
    udid[l_udidlen] = '\0';
    memcpy(ptype,getEquipmentType(),l_ptypelen);
    ptype[l_ptypelen] = '\0';
    memcpy(pid,getChannelId(),l_pidlen);
    pid[l_pidlen]='\0';

    model__sn.udid = udid;
    model__sn.equipment_type = ptype;
    model__sn.channel_id = pid;

    uint8_t *buf=NULL;                    
    size_t len;                  
    len = com__qihoo360__vehiclesafe__model__sn__get_packed_size(&model__sn);
    buf = (uint8_t*)malloc(len*2);
    memset(buf,0,len*2);
    com__qihoo360__vehiclesafe__model__sn__pack(&model__sn, buf);
    return (char *)buf;
}

/**
 ******************************************************************************
 ** \简  述  本地随机生成临时会话密钥字符串
 **  注  意  返回值需要释放，防止内存泄漏
 ** \参  数
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getProtobufForSessionKey(void* _curl)
{
    Com__Qihoo360__Vehiclesafe__Model__Session model__session = COM__QIHOO360__VEHICLESAFE__MODEL__SESSION__INIT;
    size_t data_length = 0;
    int8_t str[STR_LEN+1] = {0};
    GenerateStr(str);
    model__session.key = (char *)base64_encode((unsigned char *)str,STR_LEN,&data_length);
    // model__session.key[data_length-1]='\0';

    size_t len;                  
    len = com__qihoo360__vehiclesafe__model__session__get_packed_size(&model__session);
    uint8_t buf[len*2];
    memset(buf,0,len*2);

    com__qihoo360__vehiclesafe__model__session__pack(&model__session, buf);
    cypher_t* cypher = encyptDataByAesAndManageKey((char*)buf,_curl);
    free(model__session.key);
    if (!cypher) {
        log_e("networkmanager", "encypt error");
        return NULL;
    }
    char *data=NULL;
    data = malloc(cypher->len_data+1);
    memset(data,0,cypher->len_data+1);
    if(data == NULL)
    {
        free(cypher);
        return NULL;
    }
    strncpy(data,cypher->data,cypher->len_data);
    data[cypher->len_data]='\0';
    free(cypher);
    return data;
}

/**
 ******************************************************************************
 ** \简  述  从心跳中获取protobuf结果数据
 **  注  意  返回值需要释放，防止内存泄漏
 ** \参  数
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getProtobufForHeartbeat(void* _curl)
{
    Com__Qihoo360__Vehiclesafe__Model__HeartBeat heartBeat = COM__QIHOO360__VEHICLESAFE__MODEL__HEART_BEAT__INIT;
    int8_t *sn = getSN(_curl);
    if(sn == NULL)return NULL;
    uint16_t l_strlensn = strlen(sn);
    int8_t l_sn[l_strlensn + 1];
    memset(l_sn,0,l_strlensn + 1);
    strncpy(l_sn,sn,l_strlensn);
    heartBeat.sn = l_sn;
    uint8_t *buf = NULL;                     
    size_t len = 0;                   
    len = com__qihoo360__vehiclesafe__model__heart_beat__get_packed_size(&heartBeat);
    buf = (uint8_t*)malloc(len*2);
    memset(buf,0,len*2);
    com__qihoo360__vehiclesafe__model__heart_beat__pack(&heartBeat, buf);
    return (char *)buf;

}
