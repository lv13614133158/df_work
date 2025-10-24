#include "CANIDS_Interface.h"
//#include "Rte_CDD_CANIDS.h"
//#include "platformtypes.h"
#include "CANIDS_Working.h"

#define MAX_REQUEST_LENGTH 255
static uint8 g_receiveFlag = 0;

uint8 SK_CANIDS_TpSendMsg(uint8* data, uint16 len)
{
   // return Rte_Send_IDS_Req_IVI_DEIDS_Req_IVI(data, len);
}

void SK_CANIDS_Init(void)
{
    /* Initialize */
    IDS_Stru initCanStru;
    initCanStru.flowSwitch = 0;
    initCanStru.lengthSwitch = 0;
    initCanStru.listSwitch = 0;
    initCanStru.priodSwitch = 0;
    initCanStru.signaSwitch = 0;
    initCanStru.diagSwitch = 0;
    SK_CANIDS_Init_Ex(initCanStru, SK_CANIDS_TpSendMsg);
    //SK_CANIDS_Start();
}

void SK_CANIDS_DeInit(void)
{
    /* DeInitialize */
    IDS_Stru canIDS;
    SK_CANIDS_Stop(canIDS);
    SK_CANIDS_DeInit_Ex(canIDS);
}

//5ms周期处理函数
void SK_CANIDS_5ms_MainRunnable(void)
{
    SK_CANIDS_5ms_MainfunctionEx();
}

//CAN TP接收函数
// void SK_CANIDS_EventReceiveRunnable(void)
// {
//     uint8 request_data[MAX_REQUEST_LENGTH]; 
//     uint16 received_length;
//     if (RTE_E_OK == Rte_Receive_IDS_Req_BCM_DEIDS_Req_BCM(request_data, &received_length))
//     {
//         /* code */
//         SK_CANIDS_TPMsgReceive(request_data, received_length);
//     }
//     else
//     {
//         g_receiveFlag = 10;
//     }
// }
