/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-07 22:16:49
 */ 

#ifndef __MAIN_FILEMONITOR___
#define __MAIN_FILEMONITOR__
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#include "mysqlite.h"
#if MODULE_FILEMONITOR
/**
 * @name:   filemonitor format
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
typedef struct _filestruct{
	int mask;
	char* path;
}filestruct;

bool FileSql_insertFileEvent(char *eventSource,char* path, int mask);
list* FileSql_queryWatchPoint(void);
bool FileSql_updateWatchPointsql(char* path, int mask);
bool FileSql_deleteWatchPoint(char* path, int mask);
bool FileSql_insertWatchPoint(char* path, int mask);
long FileSql_queryConfigsql(void);
bool FileSql_updateLastUpdateWatchPointTimestamp(long long timestamp);
bool FileSql_insertFileMonitorConfig(long long lastUpdateWatchPointMapTimestamp);
void FileSql_CreateTable(void);

#endif 
#ifdef __cplusplus
}
#endif
#endif