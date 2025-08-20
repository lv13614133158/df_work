/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-23 03:53:59
 */ 

#ifndef __SYSCONFIGMONITOR__
#define __SYSCONFIGMONITOR__
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#include "mysqlite.h"


#if MODULE_CONFIGMONITOR
/**
 * @name:   ConfigDBData
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
typedef struct _ConfigDBData
{
    /* data */
    char* data;
    long createTime;
    long id;
}ConfigDBData;

typedef struct _SysConfigMonitor
{
    void (*startConfigMonitor)(void);
    void (*initConfigMonitor)(void);
    int  (*getValidUserInfo)(char**);
    char* (*getTerminalInfo)(char**);
    void  (*stopConfigMonitor)();
    void (*uploadHardInfo)(char*);
    void (*runoneshot)(char*);
}SysConfigMonitor;
extern SysConfigMonitor SysConfigMonitorObj;

#endif 
#ifdef __cplusplus
}
#endif
#endif