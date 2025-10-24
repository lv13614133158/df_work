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
#define LOADDISPLAY_TIME     (30000)   // 负载统计周期,单位ms
#define BAUDRATE_CAN         (500000)  // 波特率
#define STANDARD_FRAME       (111)     // 标准帧bit长度
#define EXTENDED_FRAME       (64)     // 扩展帧

static uint32 g_flowCheckTime = LOADDISPLAY_TIME;


// X通道流量是否在阈值范围分析
static bool SK_FlowAnaly(Flow_Elmt* flowEmlt, uint32 i)
{
    bool ret = true;
    if(flowEmlt[i].flowCnt > flowEmlt[i].flowMax)
    {
        ret = false;
    }
    else if(flowEmlt[i].flowCnt < flowEmlt[i].flowMin)
    {
        ret = false;
    }
    else{
        //Event_Print(EVENT_LEVEL_PASS, SK_FLOW_PASS_EVENT,  flowEmlt[i].netID, SK_NUM32_MAX, "The flow Pass!");
    }
    return ret;  
}

// 总线负载doc分析
static bool SK_DosAnaly(Flow_Elmt* flowEmlt, uint32 i)
{
    
    if(flowEmlt[i].loadrate > flowEmlt[i].loadrateThreshold)
    {
        if(flowEmlt[i].dosFalg == 0)
        {
            flowEmlt[i].dosFalg = 1;
        }
    }
    else
    {
        if(flowEmlt[i].dosFalg == 1)
        {
            flowEmlt[i].dosFalg = 0;
           // Event_Print(EVENT_LEVEL_OUT, SK_DOS_STOP_EVENT, flowEmlt[i].netID, SK_NUM32_MAX, "DOS attack stop");
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
    if(loss_count_time*CLOCK_CYCLE > g_flowCheckTime)
    {
        loss_count_time = 0;
        return true;
    }
    return false;
}

// 总线负载统计展示
bool SK_LoadDisplay(Flow_Elmt* flowElmt, uint32 elmtCnt, uint32 flowSwitch)
{
    if( !Get_LookUp() )
        return false;

    //char infostr[128] = {0}, len = 0;
    for(int i=0; i<elmtCnt; i++)
    {
        flowElmt[i].loadrate = (float)flowElmt[i].flowCnt*STANDARD_FRAME*100/ (g_flowCheckTime/1000)/ BAUDRATE_CAN;

        if(flowSwitch & 0x01){
            // SK_DosAnaly(&flowElmt[i], i);
            if(flowElmt[i].loadrate > flowElmt[i].loadrateThreshold)
            {
                if(flowElmt[i].dosFalg == 0)
                {
                    flowElmt[i].dosFalg = 1;
                    sendFlowCheckEventMsg(flowElmt[i].netID, flowElmt[i].loadrate, flowElmt[i].loadrateThreshold, flowElmt[i].flowCnt, flowElmt[i].flowMin, flowElmt[i].flowMax);
                }
            }
            else
            {
                if(flowElmt[i].dosFalg == 1)
                {
                    flowElmt[i].dosFalg = 0;
                }
            }
        }
        if(flowSwitch & 0x02){
            // SK_FlowAnaly(&flowElmt[i], i);
            if(flowElmt[i].flowCnt > flowElmt[i].flowMax)
            {
                sendFlowCheckEventMsg(flowElmt[i].netID, flowElmt[i].loadrate, flowElmt[i].loadrateThreshold, flowElmt[i].flowCnt, flowElmt[i].flowMin, flowElmt[i].flowMax);
            }
            else if(flowElmt[i].flowCnt < flowElmt[i].flowMin)
            {
                sendFlowCheckEventMsg(flowElmt[i].netID, flowElmt[i].loadrate, flowElmt[i].loadrateThreshold, flowElmt[i].flowCnt, flowElmt[i].flowMin, flowElmt[i].flowMax);
            }
        }
        if(flowSwitch & 0x04){
            sendFlowCheckEventMsg(flowElmt[i].netID, flowElmt[i].loadrate, flowElmt[i].loadrateThreshold, flowElmt[i].flowCnt, flowElmt[i].flowMin, flowElmt[i].flowMax);
        }
        flowElmt[i].flowCnt = 0;
    }

    if(flowSwitch & 0x01){
        //Event_Print(EVENT_LEVEL_OUT, SK_LOADRATE_EVENT, (uint8)SK_NUM32_MAX, SK_NUM32_MAX, infostr);
    }
    return true;
}

// 配置结构初始化，单独使用注意防止超过最大值
bool SK_FlowInit(Flow_Elmt* flowElmt, uint32 index, uint8 netID, uint32 period, uint32 flowMax, uint32 flowMin, float loadrateThreshold)
{
    flowElmt[index].netID   = netID;
    flowElmt[index].period  = period;
    flowElmt[index].flowMax = flowMax;
    flowElmt[index].flowMin = flowMin;
    flowElmt[index].flowCnt = 0;
    flowElmt[index].loadrate  = 0;
    flowElmt[index].loadrateThreshold  = loadrateThreshold;
    flowElmt[index].dosFalg   = 0;
    return true;
}

bool SK_SetFlowPeriod(uint32 flowPreiod)
{
    g_flowCheckTime = flowPreiod;
    return true;
}