/**
 * 文件名: event.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 事件输出
 */
#include <stdio.h>
#include "event.h"
#include "ids_config.h"
#include "log.h"
#include "ctimer.h"
#include "fusing.h"

#include <stdlib.h>
#include "cJSON.h"
#include "websocketmanager.h"

/**
 * 事件类型输出，以及分类
 * 负载率:     0x8000,
 * DOS攻击:    0x8501, 攻击停止: 0x8502
 * 流量通过:   0x8100, 过高：0x8101, 过低：0x8102
 * 白名单通过：      不通过：0x8201
 * 长度通过：  0x8300, 过长：0x8301, 过短：0x8302
 * 周期通过：  0x8400, 过长：0x8401, 过短：0x8402, 缺失：0x8403
 * 信号阈值:           过高：0x8601, 过低：0x8602
 * 
 * 
 * 
 * 这里分为(1)事件上报Event_Print和(2)日志打印Event_Print二部分
 * 其中所有的调试接口必须统一使用Event_Print里的接口，
 * 由Event_Print决定调试日志选择，方便跨平台调试
*/


// 事件简称 类型
static char eventType[][32] ={
    "BUS_LOAD",
    "FLOW_CHECK",
    "WHITE_LIST",
    "DLC_CHECK",
    "PERIOD_CHECK",
    "DOS_ATTACK",
    "MSG_THRESHOLD",
    "MSG_CHANGERATE",
    "NO_KNOWN"
};

// 熔断限制
#define EVENT_FUSING_TIME       (10*1000)  // 熔断时间,单位ms
#define EVENT_FUSING_NUM        (1000)     // 熔断条数上限
static Fusing_Stru eventFusing = {
    .fusing_num = EVENT_FUSING_NUM, 
    .fusing_count_time = EVENT_FUSING_TIME,
    .fusing_delay_time = EVENT_FUSING_TIME
};


// 将事件id转为事件字符串
static const char* Get_TypeStr(uint32 id)
{
    int index = (id>>8) & 0x0F;
    int index_max = sizeof(eventType)/sizeof(eventType[0]);
    index = (index >= index_max) ?(index_max-1) : index;
    return eventType[index];
}

// 日志打印
static void Print_Log(uint8 level, char* title, uint32 id, uint8 netID, uint32 canID, uint8* data)
{
    if(SK_Getfusing(&eventFusing)){
        return ;
    }

    const char *type = Get_TypeStr(id);
    if(netID == 0xFF){
        Debug_Print(level, "(%s):event_type:%s, event_ID:0x%x:%s\n", title, type, id, data);
    }
    else if(SK_NUM32_MAX != canID){
        Debug_Print(level, "(%s):event_type:%s, event_ID:0x%x: CAN_CH:0x%x, CAN_ID:0x%x: {%s}\n", title, type, id, netID, canID, data);
    }
    else{
        Debug_Print(level, "(%s):event_type:%s, event_ID:0x%x: CAN_CH:0x%x  {%s}\n", title, type, id, netID, data);
    }
}

#if 0
// 调试日志
void Debug_Print(int level, const char *msg, ...)  
{  
    va_list ap;    
    char message[1024] = {0};  
    int  nMessageLen = 0;  
   
    va_start(ap, msg);  
    nMessageLen = vsnprintf(message, 1024, msg, ap);  
    va_end(ap);  
    printf("[log]:%s\n", message);
}  
#endif

// 事件上报，输出
void Event_Print(uint8 level, uint32 id, uint8 netID, uint32 canID, uint8* data)
{
    switch (level)
    {
    case EVENT_LEVEL_OUT:
        Print_Log(0, "360 CANIDS event",      id, netID, canID, data);
        break;

    case EVENT_LEVEL_PASS:
        Print_Log(0, "360 CANIDS pass_event", id, netID, canID, data);
        break;

    case EVENT_LEVEL_NOPASS:
        Print_Log(0, "360 CANIDS post_event", id, netID, canID, data);
        break;
    
    default:
        break;
    }
}

char loadrate_event_type[64] = "0010102000622";
char whitelist_event_type[64] = "0010102000622";
char len_event_type[64] = "0010102000622";
char period_event_type[64] = "0010102000622";
char signal_threshold_event_type[64] = "0010102000622";
char signal_change_rate_event_type[64] = "0010102000622";
char signal_enumerate_event_type[64] = "0010102000622";
char signal_stat_event_type[64] = "0010102000622";
char signal_tracke_cnt_event_type[64] = "0010102000622";
char signal_relate_event_type[64] = "0010102000622";

int init_event_type(char* loadrate, char* whitelist, char* len, char* period, char* signal_threshold,
                 char* signal_change_rate, char* signal_enumerate, char* signal_stat, char* signal_tracke_cnt, char* signal_relate)
{
    memset(loadrate_event_type, 0, sizeof(loadrate_event_type));
    memset(whitelist_event_type, 0, sizeof(whitelist_event_type));
    memset(len_event_type, 0, sizeof(len_event_type));
    memset(period_event_type, 0, sizeof(period_event_type));
    memset(signal_threshold_event_type, 0, sizeof(signal_threshold_event_type));
    memset(signal_change_rate_event_type, 0, sizeof(signal_change_rate_event_type));
    memset(signal_enumerate_event_type, 0, sizeof(signal_enumerate_event_type));
    memset(signal_stat_event_type, 0, sizeof(signal_stat_event_type));
    memset(signal_tracke_cnt_event_type, 0, sizeof(signal_tracke_cnt_event_type));
    memset(signal_tracke_cnt_event_type, 0, sizeof(signal_tracke_cnt_event_type));

    //printf("1:%s,2:%s,3:%s,4:%s,5:%s,6:%s,7:%s,8:%s,9:%s,10:%s\n",loadrate,  whitelist, len,  period, signal_threshold,
    //             signal_change_rate, signal_enumerate, signal_stat, signal_tracke_cnt, signal_relate);
    strncpy(loadrate_event_type, loadrate, sizeof(loadrate_event_type) - 1);
    strncpy(whitelist_event_type, whitelist, sizeof(whitelist_event_type) - 1);
    strncpy(len_event_type, len, sizeof(len_event_type) - 1);
    strncpy(period_event_type, period, sizeof(period_event_type) - 1);
    strncpy(signal_threshold_event_type, signal_threshold, sizeof(signal_threshold_event_type) - 1);
    strncpy(signal_change_rate_event_type, signal_change_rate, sizeof(signal_change_rate_event_type) - 1);
    strncpy(signal_enumerate_event_type, signal_enumerate, sizeof(signal_enumerate_event_type) - 1);
    strncpy(signal_stat_event_type, signal_stat, sizeof(signal_stat_event_type) - 1);
    strncpy(signal_tracke_cnt_event_type, signal_tracke_cnt, sizeof(signal_tracke_cnt_event_type) - 1);
    strncpy(signal_relate_event_type, signal_relate, sizeof(signal_relate_event_type) - 1);

    return 0;
}

void loadrate_event_update(uint8 level, uint32 id, double time, uint8 netID, float loadrate, uint8* data)
{
    //printf("loadrate_event_update,data:%s\n", data);

	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
	cJSON_AddNumberToObject(cjson_data, "load_rate", loadrate);

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    log_i("can", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(loadrate_event_type, s);
	if(s)free(s);

    return;
}

void whitelist_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(whitelist_event_type, s);
	if(s)free(s);

    return;

}

void len_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, uint32 length, 
                            int min_len, int max_len, int DLC_err_type, uint8* data)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};
    int normal_DLC[2] = {min_len, max_len};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
    cJSON_AddNumberToObject(cjson_data, "DLC", length);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddNumberToObject(cjson_data, "DLC_err_type", DLC_err_type);
    cJSON_AddItemToObject(cjson_data,  "normal_DLC", cJSON_CreateIntArray(normal_DLC, 2));

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(len_event_type, s);
	if(s)free(s);
}

void period_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, double period, 
                            double min_period, double max_period, int period_err_type, uint8* data)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};
    double normal_period[2] = {min_period, max_period};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
    cJSON_AddNumberToObject(cjson_data, "period", period);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddNumberToObject(cjson_data, "period_err_type", period_err_type);
    cJSON_AddItemToObject(cjson_data,  "normal_period", cJSON_CreateDoubleArray(normal_period, 2));

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(period_event_type, s);
	if(s)free(s);
}

void signal_threshold_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, 
                                            int period_err_type, uint8* can_data, int can_data_len)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddStringToObject(cjson_data, "can_signal_name", signal_name);
    cJSON_AddNumberToObject(cjson_data, "period_err_type", period_err_type);

    int* i_can_data = malloc(can_data_len * sizeof(int));
    for (int i = 0; i < can_data_len; i++)
    {
        i_can_data[i] = can_data[i];
    }

    cJSON_AddItemToObject(cjson_data,  "can_message", cJSON_CreateIntArray(i_can_data, can_data_len));
    free(i_can_data);

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(signal_threshold_event_type, s);
	if(s)free(s);
}

void signal_changeRate_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_rate,
                                            int period_err_type, uint8* can_data, int can_data_len)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddStringToObject(cjson_data, "can_signal_name", signal_name);
    cJSON_AddNumberToObject(cjson_data, "signal_rate", signal_rate);
    cJSON_AddNumberToObject(cjson_data, "period_err_type", period_err_type);

    int* i_can_data = malloc(can_data_len * sizeof(int));
    for (int i = 0; i < can_data_len; i++)
    {
        i_can_data[i] = can_data[i];
    }

    cJSON_AddItemToObject(cjson_data,  "can_message", cJSON_CreateIntArray(i_can_data, can_data_len));
    free(i_can_data);

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(signal_change_rate_event_type, s);
	if(s)free(s);
}

void signal_enumerate_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_value,
                                            uint8* can_data, int can_data_len)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddStringToObject(cjson_data, "can_signal_name", signal_name);
    cJSON_AddNumberToObject(cjson_data, "signal_value", signal_value);

    int* i_can_data = malloc(can_data_len * sizeof(int));
    for (int i = 0; i < can_data_len; i++)
    {
        i_can_data[i] = can_data[i];
    }

    cJSON_AddItemToObject(cjson_data,  "can_message", cJSON_CreateIntArray(i_can_data, can_data_len));
    free(i_can_data);

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(signal_enumerate_event_type, s);
	if(s)free(s);
}

void signal_stat_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_value,
                                            int normal_signal_value, uint8* can_data, int can_data_len)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddStringToObject(cjson_data, "can_signal_name", signal_name);
    cJSON_AddNumberToObject(cjson_data, "signal_value", signal_value);

    int* i_can_data = malloc(can_data_len * sizeof(int));
    for (int i = 0; i < can_data_len; i++)
    {
        i_can_data[i] = can_data[i];
    }

    cJSON_AddItemToObject(cjson_data,  "can_message", cJSON_CreateIntArray(i_can_data, can_data_len));
    free(i_can_data);

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(signal_stat_event_type, s);
	if(s)free(s);
}

void signal_trackeCnt_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_value,
                                            int normal_signal_value, uint8* can_data, int can_data_len)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddStringToObject(cjson_data, "can_signal_name", signal_name);
    cJSON_AddNumberToObject(cjson_data, "signal_value", signal_value);

    int* i_can_data = malloc(can_data_len * sizeof(int));
    for (int i = 0; i < can_data_len; i++)
    {
        i_can_data[i] = can_data[i];
    }

    cJSON_AddItemToObject(cjson_data,  "can_message", cJSON_CreateIntArray(i_can_data, can_data_len));
    free(i_can_data);

    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(signal_tracke_cnt_event_type, s);
	if(s)free(s);
}

void signal_relate_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID,
                                            char* signal_name, int signal_value, uint8* can_data, int can_data_len,
                                            char* related_signal_name, int related_signal_value, uint8* related_can_data, int related_can_data_len)
{
	long long timestamp = time * 1000;
	cJSON *cjson_data = cJSON_CreateObject();
    char can_channel_buff[8] = {0};

    snprintf(can_channel_buff, sizeof(can_channel_buff), "%d", netID);

	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
	cJSON_AddNumberToObject(cjson_data, "can_id", canID);
	cJSON_AddStringToObject(cjson_data, "can_channel", can_channel_buff);
    cJSON_AddStringToObject(cjson_data, "can_signal_name", signal_name);
    cJSON_AddNumberToObject(cjson_data, "signal_value", signal_value);

    int* i_can_data = NULL;
    i_can_data = malloc(can_data_len * sizeof(int));
    for (int i = 0; i < can_data_len; i++)
    {
        i_can_data[i] = can_data[i];
    }

    cJSON_AddItemToObject(cjson_data,  "can_message", cJSON_CreateIntArray(i_can_data, can_data_len));
    free(i_can_data);

    cJSON_AddStringToObject(cjson_data, "related_can_signal_name", related_signal_name);
    cJSON_AddNumberToObject(cjson_data, "related_signal_value", related_signal_value);
    i_can_data = malloc(related_can_data_len * sizeof(int));
    for (int i = 0; i < related_can_data_len; i++)
    {
        i_can_data[i] = related_can_data[i];
    }

    cJSON_AddItemToObject(cjson_data,  "related_can_message", cJSON_CreateIntArray(i_can_data, related_can_data_len));
    free(i_can_data);


    char *s = cJSON_PrintUnformatted(cjson_data);
    printf("s:%s\n", s);
    if(cjson_data)
        cJSON_Delete(cjson_data);
	websocketMangerMethodobj.sendEventData(signal_relate_event_type, s);
	if(s)free(s);
}

