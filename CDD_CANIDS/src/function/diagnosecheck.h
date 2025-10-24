#ifndef _DIAGNOSECHECK_H_
#define _DIAGNOSECHECK_H_

#include "platformtypes.h"
#include "queue.h"

typedef struct _Diag_Elmt
{
    uint8  netID;
    uint32 canID;
    bool sessionSwitch;
    bool resetSwitch;
    bool accessSwitch;
    bool communiSwitch;
}Diag_Elmt;

bool SK_DiagCheck(Diag_Elmt* diagElmt, SK_Data_Stru data, uint32 elmtCnt);

bool SK_DiagInit(Diag_Elmt* diagElmt, uint32 index, uint8 netID, uint32 canID, bool sessionSwitch, bool resetSwitch, bool accessSwitch, bool communiSwitch);

#endif // _DIAGNOSECHECK_H_


