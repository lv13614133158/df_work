#ifndef __LENGTHCHECK_H__
#define __LENGTHCHECK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "platformtypes.h"
#include "queue.h"

/* *
* 配置文件涉及时间都为ms为单位
* 3、长度配置
* */
typedef struct _Len_Elmt{
    uint8  netID;
    uint32 canID;
    uint32 length;
    sint32 stateFalg;   // 统计当前状态标志
}Len_Elmt;

// Length message check
bool SK_LengthCheck(Len_Elmt* lenElmt, SK_Data_Stru data, uint32 elmtCnt);
// Length configuration Table Initialization
bool SK_LenInit(Len_Elmt* lenElmt,uint32 index, uint8 netID, uint32 canID, uint32 length);


#ifdef __cplusplus
}
#endif

#endif