#ifndef CANIDS_WORKING_H
#define CANIDS_WORKING_H

#include "platformtypes.h"

/**
 *  Can working configuration structure
 */
typedef struct _IDS_Stru{
    bool flowSwitch;
    bool listSwitch;
    bool priodSwitch;
    bool lengthSwitch;
    unsigned int signaSwitch;
    bool diagSwitch;
}IDS_Stru;

void SK_CANIDS_TPMsgReceive(uint8* data, uint16 dataLen);

uint8 SK_CANIDS_MsgReceive_Test(uint8 netID, uint32 canID, uint8* data, uint32 len);
void SK_CANIDS_MsgGet_Test(uint8* data, uint32* len);

// Framework
uint8  SK_CANIDS_Init_Ex(IDS_Stru canIDS, uint8 (*pSendMsg)(uint8* data, uint16 len) );
uint8  SK_CANIDS_DeInit_Ex();
uint8  SK_CANIDS_Start();
uint8  SK_CANIDS_Stop();
uint8  SK_CANIDS_Version(IDS_Stru canIDS);

// Can data reception
uint8  SK_CANIDS_MsgReceiveIndi(uint8 netID, uint32 canID, uint8* data, uint32 len);
// can event send and receive
uint8  SK_CANIDS_EventSendReqEx(uint8 *data, uint32 len);
uint8  SK_CANIDS_EventReceiveReqEx(uint8 *data, uint32 len);

// Timer, call clock once+5
uint8  SK_CANIDS_5ms_MainfunctionEx();

// Storage data, drop disk
ssize_t  SK_CANIDS_MemWrite(int addr, const void* buf, size_t count);
ssize_t  SK_CANIDS_MemRead(int addr, void* buf, size_t count);



#endif //CANIDS_WORKING_H