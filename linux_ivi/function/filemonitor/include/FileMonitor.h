/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-12 03:30:32
 */ 

#ifndef __FILEMONITOR_H
#define __FILEMONITOR_H
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#if MODULE_FILEMONITOR
/**
 * @name:   WatchPointMap
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
typedef struct _WatchPointMap{
    int   mask;
    char* path;
}WatchPointMap;

typedef struct _FileMonitor{
    void (*initFileMonitor)(void);
    void (*startFileMonitor)(void);
    void (*stopFileMonitor)(void);
    void (*freeFileMonitor)(void);
    void (*setBackupDirectoryThreshold)(const int);
    void (*setIsolateDirectoryThreshold)(int);
    void (*setFileWorkDirectory)(char *);
    void (*updateWatchPoint)(char *);
    void (*changeMode)(const char*, int);
    void (*changeOwner)(const char*, const char*);
    void (*restoreFile)(int, char*);
    void (*backupFile)(char*);
    void (*isolateFile)(char*);
    void (*copyFile)(char*,char*);
    void (*moveFile)(char*,char*);
    void (*removeFile)(const char*);
    bool (*compareFingerprint)(char*,char*);
    int (*getFingerprint)(char*,char **);
}FileMonitor;
extern FileMonitor FileMonitorObj;

#endif 
#ifdef __cplusplus
}
#endif
#endif