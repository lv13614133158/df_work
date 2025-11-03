/**
 * @Descripttion:  Length of the packet ID meets the design
 * @version: V1.0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2022-09-20 22:14:32
 */ 
#include <stdio.h>
#include "lengthcheck.h"
#include "event.h"



// 寻找报文的ID是否存在配置文件
static int Find_LenIndex(Len_Elmt* lenElmt, uint8 netID, uint32 canID, uint32 elmtCnt)
{
    for(int i=0; i<elmtCnt; i++)
    {
        if(lenElmt[i].netID == netID && lenElmt[i].canID == canID)
            return i;
    }
    return -1;
}

// 长度检查
// stateFalg 0:初始状态, 1:长度等于正常，2:长度大于正常 3:长度小于正常
bool SK_LengthCheck(Len_Elmt* lenElmt, SK_Data_Stru data, uint32 elmtCnt)
{
    char slog[255] = {0};
    int index = Find_LenIndex(lenElmt, data.netID, data.canID, elmtCnt);

    if(index >= 0)
    {
        if(lenElmt[index].length < data.len)
        {
            if(lenElmt[index].stateFalg != 3){
                lenElmt[index].stateFalg = 3;
                sprintf(slog, "The length is longer!, Normal length:%d < The current length:%d", lenElmt[index].length, data.len);
                //Event_Print(EVENT_LEVEL_NOPASS, SK_LEN_MAX_EVENT, data.netID, data.canID, slog); 
                len_event_update(EVENT_LEVEL_NOPASS, SK_LEN_MAX_EVENT, data.data_time, data.netID, data.canID,  data.len,
                                    lenElmt[index].length, lenElmt[index].length, 0, slog);
            }
            return false;
        }
        else if(lenElmt[index].length > data.len)
        {
            if(lenElmt[index].stateFalg != 2)
            {
                lenElmt[index].stateFalg = 2;
                sprintf(slog, "The length is shorter!, Normal length:%d > The current length:%d", lenElmt[index].length, data.len);
                //Event_Print(EVENT_LEVEL_NOPASS, SK_LEN_MIN_EVENT, data.netID, data.canID, slog); 
                len_event_update(EVENT_LEVEL_NOPASS, SK_LEN_MAX_EVENT, data.data_time, data.netID, data.canID,  data.len, 
                                    lenElmt[index].length, lenElmt[index].length, 1, slog);
            }
            return false;
        }
        else
        {
            if(lenElmt[index].stateFalg != 1){
                lenElmt[index].stateFalg = 1;
                Event_Print(EVENT_LEVEL_PASS,  SK_LEN_PASS_EVENT, data.netID, data.canID, "The length Pass!");
            }
        }
    }

    return true;
}

// 配置结构初始化，单独使用注意防止超过最大值
bool SK_LenInit(Len_Elmt* lenElmt,uint32 index, uint8 netID, uint32 canID, uint32 length)
{
    lenElmt[index].netID  = netID;
    lenElmt[index].canID  = canID;
    lenElmt[index].length = length;
    lenElmt[index].stateFalg = 0;
    return true;
}