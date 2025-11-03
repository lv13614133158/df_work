#ifndef __QUEUE_H__
#define __QUEUE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "ids_config.h"
#include "platformtypes.h"
#include "ctimer.h"

// can data
typedef struct _Data_Stru{
    uint8  netID;
    uint32 canID;
    uint8  data[64];
    uint8  len;
    uint8  canFD;
    //Time_Stru  time;
    double data_time;
}SK_Data_Stru;

#define  STACKSIZE_NUM  SK_STACKSIZE_NUM 
// can data stack
typedef struct _Can_Queue{
    SK_Data_Stru data[STACKSIZE_NUM];
    uint8 pushPos;
    uint8 popPos;
    uint8 overwrite;
}SK_Can_Queue, *SK_Can_QueuePtr;

// init queue
int SK_Can_InitQueue();
// push queue
int SK_Can_PushQueue(uint8 netID, uint32 canID, uint8 *data, uint8 len, double time);
// pop queue
int SK_Can_PopQueue(SK_Data_Stru *data);


#ifdef __cplusplus
}
#endif

#endif