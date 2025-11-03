#include <stdio.h>
#include "diagnosecheck.h"
#include "event.h"

#define ACCESS_TRIGGER_NUM   (3)    // 安全访问次数限制
#define ACCESS_TRIGGER_TIME   (30000)    // 安全访问时间限制,单位ms



// 寻找报文的ID是否存在配置文件
static int Find_DiagIndex(Diag_Elmt* diagElmt, uint8 netID, uint32 canID, uint32 elmtCnt)
{
    for(int i=0; i<elmtCnt; i++)
    {
        if(diagElmt[i].netID == netID && diagElmt[i].canID == canID)
            return i;
    }
    return -1;
}


//计数器，安全访问异常次数
static uint32 access_num = 0;
//计数器，安全访问异常后开始计数
static uint32 access_time = 0;
static Time_Stru tmp_Time = {0};
// 定时器，时间统计
static uint32 get_access_lookup(Time_Stru os_time, Time_Stru data_time)
{
    if (access_num > 0)
    {
        if (tmp_Time.sysTimeH != data_time.sysTimeH || tmp_Time.sysTimeL != data_time.sysTimeL) 
        {
            tmp_Time.sysTimeH = data_time.sysTimeH;
            tmp_Time.sysTimeL = data_time.sysTimeL;
            access_time++;
        }
        
        if(access_time*CLOCK_CYCLE > ACCESS_TRIGGER_TIME)
        {
            access_time = 0;
            access_num = 0;
            return true;
        }
    }
    return false;
}


/**
 * SK_DiagCheck - 检查诊断元素的状态并根据数据内容发送异常事件消息。
 * @diagElmt: 诊断元素数组指针，包含诊断相关的配置信息。
 * @data: 包含网络ID、CAN ID、时间戳及数据的结构体。
 * @elmtCnt: 诊断元素数组的大小。
 * 
 * 返回值:
 * - 如果输入数据的第一个字节不是0x02，则返回false。
 * - 如果检测到异常诊断情况（如会话、复位或安全访问），发送相应的事件消息，并返回true。
 * - 如果未检测到异常但索引有效，返回true。
 */
bool SK_DiagCheck(Diag_Elmt* diagElmt, SK_Data_Stru data, uint32 elmtCnt)
{
    // 获取当前诊断元素的索引
    int index = Find_DiagIndex(diagElmt, data.netID, data.canID, elmtCnt);

    Time_Stru OS_time = Get_OS_Time();
    
    get_access_lookup(OS_time, data.time);
    //if (index >= 0)
    if (1)
    {
        if(data.data[0] != 0x02)
        {
            //printf("data.data[0] =%x\n",data.data[1]);
            //负响应标准格式  [0x03] [0x7F] [SID] [NRC]
            if (data.data[1] == 0x7F)
            {
                
               printf("异常：检测到错误响应  数据长度 = 0x%0x 响应标识 = 0x%0x 请求ID = 0x%0x  NRC = 0x%0x\n",\
                        data.data[0],data.data[1], data.data[2],data.data[3]);
               return true;
            }
            
            return false;
        }
           

        // 处理会话诊断：当data = 0x02 10 xx 00 00 00 00 00时，检查是否发生会话切换异常
        if(1)
        //if (diagElmt->sessionSwitch)
        {
            if (0x10 == data.data[1] && data.data[2] > 0x03)
            {
                // 异常：发送会话切换事件消息
                printf("异常：发送会话切换事件消息  netID = 0x%0x canID = 0x%0x data = 0x%0x\n",data.netID,data.canID, data.data[2]);
                sendSessionAndResetEventMsg(data.netID, 1, data.canID, data.data[2]);
                return true;
            } 
        }
        // 处理复位诊断：当data = 0x02 11 xx 00 00 00 00 00时，检查是否发生复位异常
        if (1)       
       //if (diagElmt->resetSwitch)
        {
            if (0x11 == data.data[1])
            {
                // 异常：发送复位事件消息
                printf("异常：发送复位事件消息  netID = 0x%0x canID = 0x%0x data = 0x%0x\n",data.netID,data.canID, data.data[2]);
                sendSessionAndResetEventMsg(data.netID, 2, data.canID, data.data[2]);
                return true;
            }     
        }

        // 处理安全访问诊断：当data = 0x02 27 01 00 00 00 00 00时，检查是否超过允许的安全访问次数
       if (1)
       // if (diagElmt->accessSwitch)
        {
            if (0x27 == data.data[1] && 0x01 == data.data[2])
            {
                access_num++;
                if (access_num >= ACCESS_TRIGGER_NUM)
                {
                    // 异常：发送安全访问事件消息
                    printf("异常：超过允许的安全访问次数  netID = 0x%0x canID = 0x%0x access_num = %d\n",data.netID,data.canID, access_num);
                    sendSafetyAccessEventMsg(data.netID, data.canID, access_num);
                }
                return true;
            }
        }
        
    }
    
    return true;
}

bool SK_DiagInit(Diag_Elmt* diagElmt, uint32 index, uint8 netID, uint32 canID, bool sessionSwitch, bool resetSwitch, bool accessSwitch, bool communiSwitch)
{
    diagElmt[index].netID   = netID;
    diagElmt[index].canID  = canID;
    diagElmt[index].sessionSwitch = sessionSwitch;
    diagElmt[index].resetSwitch = resetSwitch;
    diagElmt[index].accessSwitch = accessSwitch;
    diagElmt[index].communiSwitch  = communiSwitch;
    
    return true;
}

