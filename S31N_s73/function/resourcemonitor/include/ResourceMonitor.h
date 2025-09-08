/*
 * @Author: your name
 * @Date: 2020-06-09 02:57:33
 * @LastEditTime: 2020-07-02 14:04:55
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/resourcemonitor/ResourceMonitor.h
 */

#ifndef __ResourceMonitor__H_
#define __ResourceMonitor__H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "util.h"
#include "common.h"
#include "idps_main.h"
#if MODULE_RESOURSEMONITOR

    /**
 * @name:   _ResourceCpuLoadavgData
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
typedef struct _ResourceCpuLoadavgData
{
    float loadavg1;
    float loadavg5;
    float loadavg15;
    char *loadProcessRate;
    long pid;
} ResourceCpuLoadavgData, *pResourceCpuLoadavgData; 

typedef struct _ResourceMonitor
{
    void (*initResourceMonitor)(int);
    void (*startResourceMonitor)(void);
    void (*stopResourceMonitor)(void);
    void (*freeResourceMonitor)(void);
    void (*uploadResourceSnapshot)(void);
    void (*uploadCPULoadInfo)(void);
    void (*uploadRAMUsageSize)(void);
    void (*uploadROMUsageSize)(void);
    pResourceCpuLoadavgData (*getLoadavgInfo)(void);
    void (*setCPUWorkParameter)(int, int, int);
    void (*setRAMWorkParameter)(int, int);
    void (*setROMWorkParameter)(int, int);
    void (*startCheckStatusMonitor)(int);
    void (*stopCheckStatusMonitor)(int);
    void (*freeCheckStatusMonitor)(int);     
} ResourceMonitor;
extern ResourceMonitor ResourceMonitorObj;

#endif
#ifdef __cplusplus
}
#endif

#endif