#ifndef __FUNSING_H__
#define __FUNSING_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "ctimer.h"
#include "platformtypes.h"

// Fused data structure
typedef struct _Fusing_Stru
{
    uint32 fusing_cnt;
    uint32 fusing_state;
    uint32 fusing_count_time;
    uint32 fusing_delay_time;
    uint32 fusing_num;
    Time_Stru timeCnt;
}Fusing_Stru; 

// Fusing initialization, limiting time fusing_Time unit in ms, limited number of fusing_Num
Fusing_Stru SK_FusingInit(uint32 count_time, uint32 delay_time, uint32 fusing_num);
// Obtain fusing status
uint32 SK_Getfusing(Fusing_Stru* f);


#ifdef __cplusplus
}
#endif

#endif
