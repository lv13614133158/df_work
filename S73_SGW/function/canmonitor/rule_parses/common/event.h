#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "platformtypes.h"
#include "log.h"

#define SK_LOADRATE_EVENT             (0x8000)
#define SK_DOS_START_EVENT            (0x8501)
#define SK_DOS_STOP_EVENT             (0x8502)
#define SK_FLOW_PASS_EVENT            (0x8100)
#define SK_FLOW_MAX_EVENT             (0x8101)
#define SK_FLOW_MIN_EVENT             (0x8102)
#define SK_WHITELIST_EVENT            (0x8201)
#define SK_LEN_PASS_EVENT             (0x8300)
#define SK_LEN_MAX_EVENT              (0x8301)
#define SK_LEN_MIN_EVENT              (0x8302)
#define SK_PRD_PASS_EVENT             (0x8400)
#define SK_PRD_MAX_EVENT              (0x8401)
#define SK_PRD_MIN_EVENT              (0x8402)
#define SK_PRD_LOSS_EVENT             (0x8403)
#define SK_SMG_THRESHOLD_MAX_EVENT    (0x8601)
#define SK_SMG_THRESHOLD_MIN_EVENT    (0x8602)

typedef enum _SK_EVENT_LEVEL{
    EVENT_LEVEL_OUT,
    EVENT_LEVEL_PASS,
    EVENT_LEVEL_NOPASS
}SK_EVENT_LEVEL;

// Event reporting, output
void Event_Print(uint8 level, uint32 id, uint8 netID, uint32 canID, uint8* data);  
// Debug log 
#define Debug_Print log_debug
//void Debug_Print(int level, const char *msg, ...);  

int init_event_type(char* loadrate, char* whitelist, char* len, char* period, char* signal_threshold,
                 char* signal_change_rate, char* signal_enumerate, char* signal_stat, char* signal_tracke_cnt, char* signal_relate);

void loadrate_event_update(uint8 level, uint32 id, double time, uint8 netID, float loadrate, uint8* data);
void whitelist_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID);

void len_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, uint32 length,
                            int min_len, int max_len, int DLC_err_type, uint8* data);

void period_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, double period,
                            double min_period, double max_period, int period_err_type, uint8* data);

void signal_threshold_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, 
                                            int period_err_type, uint8* can_data, int can_data_len);

void signal_changeRate_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_rate,
                                            int period_err_type, uint8* can_data, int can_data_len);

void signal_enumerate_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_value,
                                            uint8* can_data, int can_data_len);

void signal_stat_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_value,
                                            int normal_signal_value, uint8* can_data, int can_data_len);

void signal_trackeCnt_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID, char* signal_name, int signal_value,
                                            int normal_signal_value, uint8* can_data, int can_data_len);

void signal_relate_event_update(uint8 level, uint32 id, double time, uint8 netID, uint32 canID,
                                            char* signal_name, int signal_value, uint8* can_data, int can_data_len,
                                            char* related_signal_name, int related_signal_value, uint8* related_can_data, int related_can_data_len);
#ifdef __cplusplus
}
#endif

#endif