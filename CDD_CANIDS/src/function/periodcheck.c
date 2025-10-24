/**
 * @Descripttion:  period of the packet ID meets the design
 * @version: V1.0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-20 22:14:32
 */ 
#include <stdio.h>
#include "periodcheck.h"
#include "event.h"
#include "log.h"

#define REPEAT_TIME       (5)      // 滤波大小
#define LOSS_CHECK_TIME   (500)    // 丢失多久监测一次,单位ms


// stateFalg 0:第一次接收到报文， 1:上次报文正常周期 2:上次报文周期过短 3上次报文周期过长 4上次报文丢失
// 策略1：无滤波直接显示
// 缺点：数据跳会跳，会报很多事件
static bool SK_PeriodAnaly(Prd_Elmt* prdElmt, SK_Data_Stru data)
{
    bool ret = true;
    uint32 min  = prdElmt->period - prdElmt->offset;
    uint32 max  = prdElmt->period + prdElmt->offset;
    uint32 time = Get_RelCtime(prdElmt->timeCnt, data.time);
    //Debug_Print(0, "[Prd debug]: divTime-%ld,%d~%d, state %d\n", time, max, min, prdElmt->stateFalg);

    if(prdElmt->stateFalg == 0)
    {
        prdElmt->stateFalg = 1;
    }
    else if(time > max)
    {
        if(prdElmt->stateFalg != 3){
            Event_Print(EVENT_LEVEL_NOPASS, SK_PRD_MAX_EVENT, data.netID, data.canID, "The msg exceeds maximum period!");
            prdElmt->stateFalg = 3;
        }
        ret = false;
    }
    else if(time < min)
    {
        if(prdElmt->stateFalg != 2){
            Event_Print(EVENT_LEVEL_NOPASS, SK_PRD_MIN_EVENT, data.netID, data.canID, "The msg exceeds minimum period!");
            prdElmt->stateFalg = 2;
        }
        ret = false;
    }
    else if(prdElmt->stateFalg != 1)
    {
        Event_Print(EVENT_LEVEL_PASS, SK_PRD_PASS_EVENT, data.netID, data.canID, "The msg period Pass!");
        prdElmt->stateFalg = 1;
    }
    Set_Count(&prdElmt->timeCnt, data.time);
    
    return ret;
}

// 策略2：当改变一次状态，重新积累再次报新的事件，
// 缺点：因为数据会跳，所以会经常报同一个事件
// continCnt 某种事件连续了几次
static bool SK_PeriodAnalyEx(Prd_Elmt* prdElmt, SK_Data_Stru data)
{
    bool ret = true;
   
    uint32 min  = prdElmt->period - prdElmt->offset;
    uint32 max  = prdElmt->period + prdElmt->offset;
    uint32 time = Get_RelCtime(prdElmt->timeCnt, data.time);
    //Debug_Print(0, "[Prd debug]: divTime-%d,%d~%d, state %d, cnt %d\n", time, max, min, prdElmt->stateFalg, prdElmt->continCnt);

    if(prdElmt->continCnt < REPEAT_TIME)
    {
        prdElmt->continCnt++;
    }

    if(prdElmt->stateFalg == 0)
    {
        prdElmt->stateFalg = 1;
        prdElmt->continCnt  = 0;
    }
    else if(time > max)
    {
        if(prdElmt->stateFalg != 3){
            prdElmt->stateFalg = 3;
            prdElmt->continCnt = 0;
        }
        if(prdElmt->continCnt == REPEAT_TIME-1){
            Event_Print(EVENT_LEVEL_NOPASS, SK_PRD_MAX_EVENT, data.netID, data.canID, "The msg exceeds maximum period!");
        }
        ret = false;
    }
    else if(time < min)
    {
        if(prdElmt->stateFalg != 2){
            prdElmt->stateFalg = 2;
            prdElmt->continCnt = 0;
        }
        if(prdElmt->continCnt == REPEAT_TIME-1)
        {
            Event_Print(EVENT_LEVEL_NOPASS, SK_PRD_MIN_EVENT, data.netID, data.canID, "The msg exceeds minimum period!");
        }
        ret = false;
    }
    else
    {
        if(prdElmt->stateFalg != 1){
            prdElmt->stateFalg = 1;
            prdElmt->continCnt = 0;
        }
        if(prdElmt->continCnt == REPEAT_TIME-1)
        {
            Event_Print(EVENT_LEVEL_PASS, SK_PRD_PASS_EVENT, data.netID, data.canID, "The msg period Pass!");
        } 
    }
    Set_Count(&prdElmt->timeCnt, data.time);
    
    return ret;
}

// 策略3:改变状态后连续几次是同一事件再上报，
// 缺点，停止后再发同一事件，不会再报,不知道是否重新开始
/**
 * SK_PeriodAnalyEx2 - 周期分析函数。
 * @param prdElmt: 指向周期元素结构体的指针，包含周期、偏移量、状态标志等信息。
 * @param data: 输入数据结构，包含网络ID、CAN ID和时间戳。
 * @return 返回布尔值，true表示周期正常，false表示异常。
 *
 * 该函数通过以下步骤进行周期分析：
 * 1. 计算最小允许周期和最大允许周期。
 * 2. 根据当前时间与上次接收的时间差，判断是否超出范围。
 * 3. 更新连续计数器和状态标志。
 * 4. 记录日志并发送事件消息（如果适用）。
 */
static bool SK_PeriodAnalyEx2(Prd_Elmt* prdElmt, SK_Data_Stru data)
{
    char slog[255] = {0};  // 用于构建日志消息的缓冲区
    bool ret = true;       // 返回值，true表示周期正常，false表示异常
    uint32 min  = prdElmt->period - prdElmt->offset;  // 计算最小允许周期
    uint32 max  = prdElmt->period + prdElmt->offset;  // 计算最大允许周期
    uint32 time = Get_RelCtime(prdElmt->timeCnt, data.time);  // 获取距离上次接收的时间间隔

    // 初始化或丢失状态处理
    if(prdElmt->stateFalg == 0)
    {
        // 初始化状态标志和计数器
        prdElmt->stateFalg = 1;
        prdElmt->continCnt  = 0;
    }
    // 周期过长处理
    else if(time > max)
    {
        // 当连续计数达到阈值时，更新状态并发送事件
        if(GET_VALUE8(prdElmt->continCnt,16) == REPEAT_TIME-1){
            prdElmt->stateFalg = 3;
            sprintf(slog, "The msg exceeds maximum period!, Normal period:%u~%u", min, max);
            sendPeriodCheckEventMsg(data.netID, data.canID, time, prdElmt->period, prdElmt->offset);
        }

        // 更新周期过长的连续计数器
        if(GET_VALUE8(prdElmt->continCnt,16) < REPEAT_TIME)
        {
            prdElmt->continCnt += (1<<16);
        }
        else
        {
            prdElmt->continCnt = SET_VALUE8(prdElmt->continCnt,16);
        }

        ret = false;
    }
    // 周期过短处理
    else if(time < min)
    {
        // 当连续计数达到阈值时，更新状态并发送事件
        if(GET_VALUE8(prdElmt->continCnt,8) == REPEAT_TIME-1)
        {
            prdElmt->stateFalg = 2;
            sprintf(slog, "The msg exceeds minimum period!, Normal period:%u~%u", min, max);
            sendPeriodCheckEventMsg(data.netID, data.canID, time, prdElmt->period, prdElmt->offset);
        }

        // 更新周期过短的连续计数器
        if(GET_VALUE8(prdElmt->continCnt,8) < REPEAT_TIME)
        {
            prdElmt->continCnt += (1<<8);
        }
        else
        {
            prdElmt->continCnt = SET_VALUE8(prdElmt->continCnt,8);
        }

        ret = false;
    }
    // 正常周期处理
    else
    {
        // 当连续正常计数达到阈值时，确认周期正常
        if(((prdElmt->continCnt)&0xFF) == REPEAT_TIME-1)
        {
            prdElmt->stateFalg = 1;
            sprintf(slog, "The msg period Pass!, Normal period:%u~%u", min, max);
        } 

        // 更新正常周期的连续计数器
        if(((prdElmt->continCnt)&0xFF) < REPEAT_TIME)
        {
            prdElmt->continCnt += (1);
        }
        else
        {
            prdElmt->continCnt = prdElmt->continCnt & 0xFF;
        }
    }

    // 更新本次接收的时间戳
    Set_Count(&prdElmt->timeCnt, data.time);
    
    return ret;
}

// 寻找报文的ID是否存在配置文件
static int Find_PrdIndex(Prd_Elmt* periodElmt, uint8 netID, uint32 canID, uint32 elmtCnt)
{
    for(int i=0; i<elmtCnt; i++)
    {
        if(periodElmt[i].netID == netID && periodElmt[i].canID == canID)
            return i;
    }
    return -1;
}

// 定时器，时间统计
static uint32 Get_LookUp()
{
    static uint32 loss_count_time = 0;
    loss_count_time++;
    if(loss_count_time*CLOCK_CYCLE > LOSS_CHECK_TIME)
    {
        loss_count_time = 0;
        return true;
    }
    return false;
}

// 周期丢失包监测
bool SK_PeriodLossCheck(Prd_Elmt* periodElmt, uint32 elmtCnt)
{
    if( !Get_LookUp() )
        return true;
    
    char slog[255] = {0};
    Time_Stru OS_time = Get_OS_Time();

    for(int i=0; i<elmtCnt; i++)
    {
        if(periodElmt[i].stateFalg)
        {
            uint32 time = Get_RelCtime(periodElmt[i].timeCnt, OS_time);
            if(time > periodElmt[i].period *10)
            {
                periodElmt[i].stateFalg = 0;
                periodElmt[i].continCnt = 0;
                //sprintf(slog, "The msg loss!, Normal period:%u~%u", periodElmt[i].period - periodElmt[i].offset, periodElmt[i].period + periodElmt[i].offset);
                //Event_Print(EVENT_LEVEL_NOPASS, SK_PRD_LOSS_EVENT, periodElmt[i].netID, periodElmt[i].canID, slog);
            }
        }
    }

    return true;
}

// 周期监测
bool SK_PeriodCheck(Prd_Elmt* periodElmt, SK_Data_Stru data, uint32 elmtCnt) 
{
    sint32 index = Find_PrdIndex(periodElmt, data.netID, data.canID, elmtCnt);
    if(index >= 0){   
        SK_PeriodAnalyEx2(&periodElmt[index], data);
    }
    
    return true;
}

// 配置结构初始化，单独使用注意防止超过最大值
bool SK_PrdInit(Prd_Elmt* periodElmt, uint32 index, uint8 netID, uint32 canID, uint32 period, uint32 offset)
{
    periodElmt[index].netID  = netID;
    periodElmt[index].canID  = canID;
    periodElmt[index].period = period;
    periodElmt[index].offset = offset;
    periodElmt[index].stateFalg = 0;
    periodElmt[index].continCnt = 0;
    Init_Count(&periodElmt[index].timeCnt);
    return true;
}
