/**
 * @Descripttion: The can channel traffic analysis
 * @version: V1.0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-20 22:14:32
 */ 
#include <stdio.h>
#include <string.h>
#include "flowcheck.h"
#include "event.h"
#include "ctimer.h"

#define DOC_THRESHOLD        (20)      // 定义doc攻击的阈值
#define LOADDISPLAY_TIME     (3000)    // 负载统计周期,单位ms
#define BAUDRATE_CAN         (500000)  // 波特率
#define STANDARD_FRAME       (108)     // 标准帧字节长度
#define EXTENDED_FRAME       (120)     // 扩展帧



// X通道流量是否在阈值范围分析
static bool SK_FlowAnaly(Flow_Elmt* flowEmlt, uint32 i)
{
    bool ret = true;
    if(flowEmlt[i].flowCnt > flowEmlt[i].flowMax)
    {
        Event_Print(EVENT_LEVEL_NOPASS, SK_FLOW_MAX_EVENT, flowEmlt[i].netID, SK_NUM32_MAX, "The flow exceeds the maximum range!");
        ret = false;
    }
    else if(flowEmlt[i].flowCnt < flowEmlt[i].flowMin)
    {
        Event_Print(EVENT_LEVEL_NOPASS, SK_FLOW_MIN_EVENT, flowEmlt[i].netID, SK_NUM32_MAX, "The flow exceeds the minimum range!");
        ret = false;
    }
    else{
        Event_Print(EVENT_LEVEL_PASS, SK_FLOW_PASS_EVENT,  flowEmlt[i].netID, SK_NUM32_MAX, "The flow Pass!");
    }
    return ret;  
}

// 总线负载doc分析
static bool SK_DosAnaly(Flow_Elmt* flowEmlt, uint32 i)
{
    
    if(flowEmlt[i].loadrate > DOC_THRESHOLD)
    {
        if(flowEmlt[i].dosFalg == 0)
        {
            flowEmlt[i].dosFalg = 1;
            Event_Print(EVENT_LEVEL_OUT, SK_DOS_START_EVENT, flowEmlt[i].netID, SK_NUM32_MAX, "DOS attack start");
        }
    }
    else
    {
        if(flowEmlt[i].dosFalg == 1)
        {
            flowEmlt[i].dosFalg = 0;
            Event_Print(EVENT_LEVEL_OUT, SK_DOS_STOP_EVENT, flowEmlt[i].netID, SK_NUM32_MAX, "DOS attack stop");
        }
    }
    return true;
}

// 寻找报文的通道是否存在配置文件
static int Find_FlowIndex(Flow_Elmt* flowElmt, uint8 netID, uint32 elmtCnt)
{
    for(int i=0; i<elmtCnt; i++)
    {
        if(flowElmt[i].netID == netID )
            return i;
    }
    return -1;
}

// 流量监测包添加
bool SK_FlowCheck(Flow_Elmt* flowElmt, SK_Data_Stru data, uint32 elmtCnt)
{
    int index = Find_FlowIndex(flowElmt, data.netID, elmtCnt);
    if(index >= 0 && index < elmtCnt){
        if (flowElmt[index].flowCnt == 0)
        {
            flowElmt[index].data_time_begin = data.data_time;
        }
        flowElmt[index].data_time_last = data.data_time;
        flowElmt[index].flowCnt++;
        return true;
    }
    return false;
}

// 定时器，时间统计
static int Get_LookUp()
{
    static int loss_count_time = 0;
    loss_count_time++;
    if(loss_count_time*CLOCK_CYCLE > LOADDISPLAY_TIME)
    {
        loss_count_time = 0;
        return true;
    }
    return false;
}

// 总线负载统计展示
bool SK_LoadDisplay(Flow_Elmt* flowElmt, uint32 elmtCnt, uint32 flowSwitch)
{
    double time_diff = 0.0;

    //if( !Get_LookUp() )
    //    return false;

    char infostr[128] = {0};
    for(int i=0; i<elmtCnt; i++)
    {
        time_diff = flowElmt[i].data_time_last - flowElmt[i].data_time_begin;

        //printf("SK_LoadDisplay time_diff:%lf, period:%d\n", time_diff, flowElmt[i].period);
        if ((time_diff * 1000) < flowElmt[i].period)
        {
            continue;
        }

        //flowElmt[i].loadrate = (float)flowElmt[i].flowCnt*STANDARD_FRAME*100/ (LOADDISPLAY_TIME/1000)/ BAUDRATE_CAN;
        flowElmt[i].loadrate = (float)flowElmt[i].flowCnt*STANDARD_FRAME*100/ (time_diff)/ BAUDRATE_CAN;
        snprintf(infostr, sizeof(infostr), "\nCAN_CH:%d, Bus Load:%f, Frame Count:%d, Time:%dms", flowElmt[i].netID, flowElmt[i].loadrate, flowElmt[i].flowCnt, LOADDISPLAY_TIME);

        if(flowSwitch & 0x01){
            //Event_Print(EVENT_LEVEL_OUT, SK_LOADRATE_EVENT, (uint8)SK_NUM32_MAX, SK_NUM32_MAX, infostr);
            loadrate_event_update(EVENT_LEVEL_OUT, SK_LOADRATE_EVENT, flowElmt[i].data_time_last, flowElmt[i].netID, flowElmt[i].loadrate, infostr);
        }

        if(flowSwitch & 0x02){
            SK_DosAnaly(&flowElmt[i], i);
        }
        if(flowSwitch & 0x04){
            SK_FlowAnaly(&flowElmt[i], i);
        }
        flowElmt[i].flowCnt = 0;
        flowElmt[i].data_time_last = flowElmt[i].data_time_begin = 0.0;
    }

    return true;
}

// 配置结构初始化，单独使用注意防止超过最大值
bool SK_FlowInit(Flow_Elmt* flowElmt, uint32 index, uint8 netID, uint32 period, uint32 flowMax, uint32 flowMin)
{
    flowElmt[index].netID   = netID;
    flowElmt[index].period  = period;
    flowElmt[index].flowMax = flowMax;
    flowElmt[index].flowMin = flowMin;
    flowElmt[index].flowCnt = 0;
    flowElmt[index].loadrate  = 0;
    flowElmt[index].dosFalg   = 0;
    return true;
}