#include "CANIDS_Interface.h"
//#include "Rte_CDD_CANIDS.h"
//#include "platformtypes.h"
#include "CANIDS_Working.h"

#define MAX_REQUEST_LENGTH 255
static uint8 g_receiveFlag = 0;

uint8 SK_CANIDS_TpSendMsg(uint8* data, uint16 len)
{
    //间隔2000ms触发一次
   // return Rte_Send_IDS_Req_IVI_DEIDS_Req_IVI(data, len);
}

void SK_CANIDS_Init(IDS_Stru canIDS, char *rule)
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

    cJSON* root = NULL;
    cJSON* j_tmp_switch = NULL;

    root = cJSON_Parse(rule);
    if (root)
    {
        j_tmp_switch = cJSON_GetObjectItem(root, "can_flow_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_flow_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            initCanStru.flowSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_id_white_list_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_id_white_list_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            initCanStru.listSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_priod_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_priod_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
             initCanStru.priodSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_length_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_length_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            initCanStru.lengthSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_signal_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_signal_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            initCanStru.signaSwitch = j_tmp_switch->valueint;
        }

        cJSON* j_loadrate_event_type = cJSON_GetObjectItem(root, "loadrate_event_type");
        cJSON* j_whitelist_event_type= cJSON_GetObjectItem(root, "whitelist_event_type");
        cJSON* j_len_event_type = cJSON_GetObjectItem(root, "len_event_type");
        cJSON* j_period_event_type = cJSON_GetObjectItem(root, "period_event_type");
        cJSON* j_signal_threshold_event_type = cJSON_GetObjectItem(root, "signal_threshold_event_type");
        cJSON* j_signal_change_rate_event_type = cJSON_GetObjectItem(root, "signal_change_rate_event_type");
        cJSON* j_signal_enumerate_event_type = cJSON_GetObjectItem(root, "signal_enumerate_event_type");
        cJSON* j_signal_stat_event_type = cJSON_GetObjectItem(root, "signal_stat_event_type");
        cJSON* j_signal_tracke_cnt_event_type = cJSON_GetObjectItem(root, "signal_tracke_cnt_event_type");
        cJSON* j_signal_relate_event_type = cJSON_GetObjectItem(root, "signal_relate_event_type");

        if (cJSON_IsString(j_loadrate_event_type) &&
            cJSON_IsString(j_whitelist_event_type) &&
            cJSON_IsString(j_len_event_type) &&
            cJSON_IsString(j_period_event_type) &&
            cJSON_IsString(j_signal_threshold_event_type) &&
            cJSON_IsString(j_signal_change_rate_event_type) &&
            cJSON_IsString(j_signal_enumerate_event_type) &&
            cJSON_IsString(j_signal_stat_event_type) &&
            cJSON_IsString(j_signal_tracke_cnt_event_type) &&
            cJSON_IsString(j_signal_relate_event_type))
        {
            init_event_type(j_loadrate_event_type->valuestring, j_whitelist_event_type->valuestring,j_len_event_type->valuestring,
                            j_period_event_type->valuestring, j_signal_threshold_event_type->valuestring, j_signal_change_rate_event_type->valuestring,
                            j_signal_enumerate_event_type->valuestring, j_signal_stat_event_type->valuestring, j_signal_tracke_cnt_event_type->valuestring,
                            j_signal_relate_event_type->valuestring);
        }
        cJSON_Delete(root);
    }


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
