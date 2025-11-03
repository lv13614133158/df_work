#ifndef __LISTCHECK_H__
#define __LISTCHECK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "platformtypes.h"
#include "queue.h"
  
/* *
* 配置文件涉及时间都为ms为单位
* 2、白名单配置
* */
typedef struct _List_Elmt{
    uint8  netID;
    uint32 canID;
}List_Elmt;

// list message check
bool SK_ListCheck(List_Elmt* listElmt, SK_Data_Stru data, uint32 elmtCnt);
// list configuration Table Initialization
bool SK_ListInit(List_Elmt* listElmt, uint32 index, uint8 netID, uint32 canID);

#ifdef __cplusplus
}
#endif

#endif