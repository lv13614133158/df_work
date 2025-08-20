
#ifndef __MAIN_NETWORKMONITOR_
#define __MAIN_NETWORKMONITOR_
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#include "api_networkmonitor.h"
#if MODULE_NETWORKMONITOR

typedef struct _NetWorkMonitorMethod{
    void (*newNetworkMonitor)(char*, char*, bool, char*, char*, bool, int, bool, int);
    void (*startNetWorkMonitor)(int);
    void (*stopNetworkMonitor)(void);
    void (*freeNetWorkMonitor)(void);
    void (*getTrafficUsageInfo)(char*, int);
    void (*setSnifferFilePath)(char*);
    void (*startSniffer)(void);
    void (*stopSniffer)(void);
    void (*updateNetAttackEvent)(char*, bool);
    void (*updateNetFlowEvent)(int, bool);
    void (*updateIpWhiteList)(char*);
    void (*updatePortWhiteList)(char*);
    void (*updateDNSWhiteList)(char*);
}NetWorkMonitorMethod;
extern NetWorkMonitorMethod NetWorkMonitorMethodObj;

#endif
#ifdef __cplusplus
}
#endif
#endif