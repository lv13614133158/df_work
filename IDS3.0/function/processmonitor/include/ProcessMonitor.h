/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-30 19:09:49
 */ 
#ifndef __MAIN_PROCESSMONITOR_
#define __MAIN_PROCESSMONITOR_
#ifdef __cplusplus
extern "C" {
#endif
#include "idps_main.h"
#if MODULE_PROCESSMONITOR
typedef struct _ProcessMonitor{
  void (*initProcessMonitor)(int);
  void (*startProcessMonitor)(void);
  void (*stopProcessMonitor)(void);
  void (*updateProcessWhiteList)(char*);
  void (*setProcessWhiteList)(char*);
  void (*killProcess)(int);
}ProcessMonitor;
extern ProcessMonitor ProcessMonitorObj;

#endif
#ifdef __cplusplus
}
#endif
#endif
