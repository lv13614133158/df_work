/**
 * 文件名: queue.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 数据队列操作
 */
#include "queue.h"
#include "ids_config.h"
//#include "Os.h"


// 这里为了减少复杂度，将队列直接静态定义到这里，
// 如需要通用化，只需改为结构体指针操作。
static SK_Can_Queue canQueueObj = {0};

// Security check write queue
static int _SK_Write_Can_Queue(uint32 index ,uint8 netID, uint32 canID, uint8 *data, uint8 len)
{
    if(index >= 0 && index < STACKSIZE_NUM){
        canQueueObj.data[index].time  = Get_OS_Time();
        canQueueObj.data[index].netID = netID;
        canQueueObj.data[index].canID = canID;
        canQueueObj.data[index].len   = SK_MIN(len, 64);
        for(int i=0; i<canQueueObj.data[index].len; i++){
            canQueueObj.data[index].data[i] = data[i];
        }
        return true;
    }
    return false;
}

// Security check read queue
static int _SK_Read_Can_Queue(uint32 index, SK_Data_Stru *data)
{
    SK_Data_Stru *canData = data;
    if(index >= 0 && index < STACKSIZE_NUM && canData)
    {
        canData->netID = canQueueObj.data[index].netID;
        canData->canID = canQueueObj.data[index].canID;
        canData->len   = canQueueObj.data[index].len;
        canData->time  = canQueueObj.data[index].time;
        for(int i=0; i<SK_MIN(canData->len, 8); i++){
            canData->data[i] = canQueueObj.data[index].data[i];
        }
        return true;
    }
    return false;
}

// init queue
int SK_Can_InitQueue()
{
    canQueueObj.pushPos   = 0;
    canQueueObj.popPos    = 0;
    canQueueObj.overwrite = 0;
    return 0;
}

static int gSpinlock = 0;

// push queue
int SK_Can_PushQueue(uint8 netID, uint32 canID, uint8 *data, uint8 len)
{
    int index = -1;
    // GetSpinlock(CANIDS_Spinlock);
    if (gSpinlock)
    {
        return index;
    }
    gSpinlock = 1;

    if(canQueueObj.overwrite)
    {
        if(canQueueObj.pushPos >= canQueueObj.popPos){
            // ReleaseSpinlock(CANIDS_Spinlock);
            gSpinlock = 0;
            return index;
        }
    }
    else
    {
        if(canQueueObj.pushPos==STACKSIZE_NUM)
        {
            if(canQueueObj.popPos==0)
            {
                // ReleaseSpinlock(CANIDS_Spinlock);
                gSpinlock = 0;
                return index;
            }
            else{
                canQueueObj.overwrite = 1;
                canQueueObj.pushPos   = 0;
            }         
        }
    }
    index = canQueueObj.pushPos;
    _SK_Write_Can_Queue(index, netID, canID, data, len);
    canQueueObj.pushPos++;
    // ReleaseSpinlock(CANIDS_Spinlock);
    gSpinlock = 0;
    return index;
}

// pop queue
int SK_Can_PopQueue(SK_Data_Stru *data)
{
    int index = -1;
    // GetSpinlock(CANIDS_Spinlock);
    if (gSpinlock)
    {
        return index;
    }
    gSpinlock = 1;

    if(canQueueObj.overwrite)
    {
        if(canQueueObj.popPos == STACKSIZE_NUM){
            canQueueObj.overwrite = 0;
            canQueueObj.popPos   = 0;
        }
    }
    else
    {
        if(canQueueObj.popPos >= canQueueObj.pushPos){
            // ReleaseSpinlock(CANIDS_Spinlock);
            gSpinlock = 0;
            return index;
        }
    }
    index = canQueueObj.popPos;
    _SK_Read_Can_Queue(index, data);
    canQueueObj.popPos++;
    // ReleaseSpinlock(CANIDS_Spinlock);
    gSpinlock = 0;
    return index;
}
