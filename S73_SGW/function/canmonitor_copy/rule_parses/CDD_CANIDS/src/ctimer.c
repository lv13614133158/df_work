/**
 * 文件名: ctime.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 定时器计时
 * 版权:
 */
#include "ids_config.h"
#include "ctimer.h"

/**
 * OS 为我们系统基准时钟，不要轻易修改
 * Count 为各个子模块标记时间
 * 避免因为系统兼容问题，这里时钟用的二个32位；1ms的滴答时钟，一个32位够跑49.7天，二个完全够用
 * CLOCK_CNT_MAX 时钟计时32位数最大值，可以根据需求更改
*/


#define  CLOCK_CNT_MAX    SK_NUM32_MAX


// OS系统时间
static Time_Stru OS_time = {
    .sysTimeH = 0,
    .sysTimeL = 0,
    .scale = CLOCK_CYCLE
};

// OS系统时间 计数
Time_Stru Cnt_OS_Time()
{
    if(CLOCK_CNT_MAX == OS_time.sysTimeL)
    {
        OS_time.sysTimeL = 0;
        if(CLOCK_CNT_MAX == OS_time.sysTimeH){
            OS_time.sysTimeH = 0;
        }
        else{
            OS_time.sysTimeH++;
        }
    }
    else{
        OS_time.sysTimeL += OS_time.scale; 
    }
    return OS_time;
}

// 设置OS系统时间为一个特定值
Time_Stru Set_OS_Time(Time_Stru time)
{
    OS_time.sysTimeH = time.sysTimeH;
    OS_time.sysTimeL = time.sysTimeL;
    OS_time.scale    = time.scale;
    return OS_time;
}

// 得到当前OS系统时间
Time_Stru Get_OS_Time()
{
    return OS_time;
}

// 配置表里统计信号的各个计数Count时间
// 初始化计数时间参数为0
void Init_Count(Time_Stru* count)
{
    count->sysTimeH = 0;
    count->sysTimeL = 0;
    count->scale = CLOCK_CYCLE;
}

// 设置计数时间为当前can ids系统时间
void Set_Count(Time_Stru* count, Time_Stru time)
{
    count->sysTimeH = time.sysTimeH;
    count->sysTimeL = time.sysTimeL;
    count->scale    = time.scale;
}

// 同步计数时间为OS时间
void Set_Count_OS(Time_Stru* count)
{
    Set_Count(count, OS_time);
}

// 计算二个计数时间相差次数
uint32  Get_RelCount(Time_Stru count, Time_Stru time)
{
    uint32 ret = CLOCK_CNT_MAX;
    if( ((time.sysTimeH == (count.sysTimeH + 1))&&(time.sysTimeL <  count.sysTimeL)) ||
        ((time.sysTimeH ==  count.sysTimeH)     &&(time.sysTimeL >= count.sysTimeL)) )
    {
        uint32 tmep = (time.sysTimeH - count.sysTimeH)*CLOCK_CNT_MAX + time.sysTimeL - count.sysTimeL;
        ret = tmep;
    }
    else if( (time.sysTimeH < count.sysTimeH ) || ((time.sysTimeH == count.sysTimeH)&&(time.sysTimeL < count.sysTimeL)) )
    {
        ret = 0;
    }
    return ret;
}

// 计算二个计数时间相差时间
uint32  Get_RelCtime(Time_Stru count, Time_Stru time)
{
    uint32 ret = Get_RelCount(count, time);
    return ret;
}



#include <unistd.h>
// 时间延迟ms
void Delay_MS(int cnt)
{
    int num = cnt>1000 ? 1000:cnt;
    num = num *1000;
    usleep(num);
}

// 时间延迟s
void Delay_S(int cnt)
{
    sleep(cnt);
}