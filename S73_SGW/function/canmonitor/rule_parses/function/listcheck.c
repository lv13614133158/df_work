/**
 * @Descripttion: Analyze whitelist packets
 * @version: V1.0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-20 22:14:32
 */ 
#include <stdio.h>
#include "listcheck.h"
#include "event.h"
#include "fusing.h"

#define LIST_FUSING_TIME    (10*1000)       // 熔断时间,单位ms
#define LIST_FUSING_NUM     (100)           // 熔断条数上限

// list熔断
static Fusing_Stru listFusing = {
    .fusing_num = LIST_FUSING_NUM, 
    .fusing_count_time = LIST_FUSING_TIME,
    .fusing_delay_time = LIST_FUSING_TIME
};

// 寻找报文的ID是否存在配置文件
static int32_t Find_ListIndex(List_Elmt* listElmt, uint8 netID, uint32 canID, uint32 elmtCnt)
{
    for(int i=0; i<elmtCnt; i++)
    {
        if(listElmt[i].netID == netID && listElmt[i].canID == canID)
            return i;
    }
    return -1;
}

// 白名单检查
bool SK_ListCheck(List_Elmt* listElmt, SK_Data_Stru data, uint32 elmtCnt)
{
    int32_t index = Find_ListIndex(listElmt, data.netID, data.canID, elmtCnt);
    bool   isFind = index >= 0? true:false;

    if( !isFind )
    {
        if( SK_Getfusing(&listFusing) )
            return isFind;
        //Event_Print(EVENT_LEVEL_NOPASS, SK_WHITELIST_EVENT, data.netID, data.canID, "Non white list ID!");
        whitelist_event_update(EVENT_LEVEL_NOPASS, SK_WHITELIST_EVENT, data.data_time, data.netID, data.canID);
        return isFind;
    }
    return isFind;
}

// 配置结构初始化，单独使用注意防止超过最大值
bool SK_ListInit(List_Elmt* listElmt, uint32 index, uint8 netID, uint32 canID)
{
    listElmt[index].netID = netID;
    listElmt[index].canID = canID;
    return true;
}
