#ifndef __CTIMER_H__
#define __CTIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "platformtypes.h"

// 定时器计时系统
#define  CLOCK_CYCLE      1 // 时钟周期单位1ms

// Time structure
typedef struct _TimeStru{
    uint32 sysTimeH;         
    uint32 sysTimeL;
    uint32 scale;
}Time_Stru;

// Set system time
Time_Stru Cnt_OS_Time();
Time_Stru Set_OS_Time(Time_Stru time);
Time_Stru Get_OS_Time();

// Set counting time
// This time requires the user to create a time object and count it
void Init_Count(Time_Stru* count);
void Set_Count(Time_Stru* count, Time_Stru time);
void Set_Count_OS(Time_Stru* count);
uint32 Get_RelCount(Time_Stru count, Time_Stru time);
uint32 Get_RelCtime(Time_Stru count, Time_Stru time);

// time delay
void Delay_S(int cnt);
void Delay_MS(int cnt);

#ifdef __cplusplus
}
#endif

#endif

