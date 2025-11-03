#ifndef __FLOWCHECK_H__
#define __FLOWCHECK_H__

#include "platformtypes.h"
#include "queue.h"

/*
* 配置文件涉及时间都为ms为单位
* 1、流量配置，load配置，doc配置
*/
typedef struct _Flow_Elmt{
    uint8  netID;
    uint32 period;      // 统计周期，目前使用宏定义
    uint32 flowMax;     // 统计阈值最大值
    uint32 flowMin;     // 统计阈值最小值
    uint32 flowCnt;     // 统计当前值
    float  loadrate;    // load负载率
    float  loadrateThreshold;  // load负载率阈值
    sint32 dosFalg;     // dos当前状态标志
}Flow_Elmt, *pFlow_Elmt;

// Flow message check
bool SK_FlowCheck(Flow_Elmt* flowElmt, SK_Data_Stru data, uint32 elmtCnt);
// Flow statistics display
bool SK_LoadDisplay(Flow_Elmt* flowElmt, uint32 elmtCnt, uint32 flowSwitch);
// Flow configuration Table Initialization
bool SK_FlowInit(Flow_Elmt* flowElmt, uint32 index, uint8 netID, uint32 period, uint32 flowMax, uint32 flowMin, float loadrateThreshold);

bool SK_SetFlowPeriod(uint32 flowPreiod);

#endif
