/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-08 22:13:44
 */ 

#ifndef __SQL_SYSCONFIGMONITOR__
#define __SQL_SYSCONFIGMONITOR__
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#include "mysqlite.h"
#if MODULE_CONFIGMONITOR
/**
 * @name:   ConfigDBDataStruct
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
typedef struct _ConfigDBDataStruct{
    char* data;
    long id;
}ConfigDBDataStruct;

void SysSql_CreateTable(void);
bool SysSql_insertSystemConfig(char* data);
ConfigDBDataStruct* SysSql_querySystemConfig(void);

#endif 
#ifdef __cplusplus
}
#endif
#endif