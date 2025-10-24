#ifndef __EVENT_H__
#define __EVENT_H__

#include "platformtypes.h"
#include "log.h"
#include "communicate.h"

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
#define Send_Event log_event


#endif