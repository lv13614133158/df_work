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
        //Debug_Print(level, "(%s):event_type:%s, event_ID:0x%x:%s\n", title, type, id, data);
    }
    else if(SK_NUM32_MAX != canID){
       // Debug_Print(level, "(%s):event_type:%s, event_ID:0x%x: CAN_CH:0x%x, CAN_ID:0x%x: {%s}\n", title, type, id, netID, canID, data);
    }
    else{
       // Debug_Print(level, "(%s):event_type:%s, event_ID:0x%x: CAN_CH:0x%x  {%s}\n", title, type, id, netID, data);
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

