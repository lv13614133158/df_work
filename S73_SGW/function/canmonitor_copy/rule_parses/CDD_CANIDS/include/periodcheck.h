#ifndef __PERIODCHECK_H__
#define __PERIODCHECK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "platformtypes.h"
#include "queue.h"

/* *
* 配置文件涉及时间都为ms为单位
* 4、周期检测配置
* */
typedef struct _Prd_Elmt{
    uint8  netID;
    uint32 canID;
    uint32 period;
    uint32 offset;
    sint32 stateFalg;
    uint32 continCnt;
    Time_Stru timeCnt;
    double data_time;
}Prd_Elmt;

// Period message check
bool SK_PeriodCheck(Prd_Elmt* periodElmt, SK_Data_Stru data, uint32 elmtCnt); 
// Period message loss check
bool SK_PeriodLossCheck(Prd_Elmt* periodElmt, uint32 elmtCnt); 
// Period configuration Table Initialization
bool SK_PrdInit(Prd_Elmt* periodElmt, uint32 index, uint8 netID, uint32 canID, uint32 period, uint32 offset);

#ifdef __cplusplus
}
#endif

#endif