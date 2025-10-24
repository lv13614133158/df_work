/**
 * 文件名: fusing.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 熔断功能，特定时间长内事件次数大于特定值，限制一段时间上报。
 */
#include <stdio.h>
#include "ctimer.h"
#include "fusing.h"
#include "event.h"

/**
 * fusing_num :熔断触发事件-次数
 * fusing_count_time :熔断触发统计-时间
 * fusing_delay_time :熔断等待时间
 * 熔断状态返回1，非熔断状态返回0
*/

// 熔断函数初始化
Fusing_Stru SK_FusingInit(uint32 count_time, uint32 delay_time, uint32 fusing_num)
{
    Fusing_Stru f;
    f.fusing_cnt   = 0;
    f.fusing_state = 0;
    f.fusing_num   = fusing_num;
    f.fusing_count_time  = count_time;
    f.fusing_delay_time  = delay_time;
    f.timeCnt.sysTimeH = 0;
    f.timeCnt.sysTimeL = 0;
    f.timeCnt.scale = 0;
    return f;
}

// 熔断状态获取
uint32 SK_Getfusing(Fusing_Stru* f)
{
    if(f->timeCnt.sysTimeH==0 &&  f->timeCnt.sysTimeL==0)
        f->timeCnt = Get_OS_Time();

    // 熔断期时间等待
    if(f->fusing_state)
    {
        Time_Stru curTime = Get_OS_Time();
        uint32 time = Get_RelCtime(f->timeCnt, curTime);
        if(time > f->fusing_delay_time)
        {
            //Debug_Print(0, "No Fusing, %d\n", f->fusing_cnt);
            f->fusing_state = 0;
            f->timeCnt = curTime;
        }
        return f->fusing_state;
    }

    // 非熔断统计
    if(f->fusing_num < f->fusing_cnt++)
    {
        Time_Stru curTime = Get_OS_Time();
        uint32 time = Get_RelCtime(f->timeCnt, curTime);
        if(time < f->fusing_count_time)
        {
            //Debug_Print("0, Fusing, %d\n", f->fusing_cnt);
            f->fusing_state = 1;
        }
        f->timeCnt = curTime;
        f->fusing_cnt = 0;
    }

    return f->fusing_state;
}