/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-06 07:12:03
 */ 
#ifndef __SQL_PROCESSMONITOR___
#define __SQL_PROCESSMONITOR___
#ifdef __cplusplus
extern "C" {
#endif
#include "idps_main.h"
#include "util.h"
#include "common.h"
#include "ProcessMonitor.h"
#if MODULE_PROCESSMONITOR
typedef struct _ProcessMonitorConfig{
    int period;
    long long lastUpdateProcessTimestamp;
}ProcessMonitorConfig,*pProcessMonitorConfig;

void ProcessSql_createTable(void);
bool ProcessSql_insertConfig(int period, long long lastUpdateProcessTimestamp);
bool ProcessSql_updatePeriodConfig(int period);
bool ProcessSql_updateLastUpdateTimestamp(char* timestamp);
bool ProcessSql_insertWhiteList(char *processName);
bool ProcessSql_deleteWhiteList(char *processName);
pProcessMonitorConfig ProcessSql_queryConfig(void);
list *ProcessSql_queryProcessNameList(void);


#endif
#ifdef __cplusplus
}
#endif
#endif