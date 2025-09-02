/*
 * @Author: your name
 * @Date: 2020-06-19 16:37:25
 * @LastEditTime: 2020-07-07 00:26:32
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/processmonitor/ProcessInfoChecker.h
 */ 
//ProcessInfoChecker
#ifndef __MAIN_PROCESSINFOCHECKER_
#define __MAIN_PROCESSINFOCHECKER_
#ifdef __cplusplus
extern "C" {
#endif
#include "myHashMap.h"
#include "myList.h"
#include "util.h"
#if MODULE_PROCESSMONITOR

void ProcessCheck_initProcessInfoChecker(MyHashMap **lastProcessMapTemp, MyHashMap **currentProcessMapTemp, list **whiteProcessNameTemp);
MyHashMap *ProcessCheck_formatStringToProcessMap(char *processString);
MyHashMap *ProcessCheck_checkProcessForFirstTime();
MyHashMap *ProcessCheck_checkProcessForAnotherTime();

#endif
#ifdef __cplusplus
}
#endif
#endif