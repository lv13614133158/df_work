/*
 * @Author: your name
 * @Date: 2020-06-19 16:34:39
 * @LastEditTime: 2020-07-22 15:53:46
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/src/processmonitor/processInfoChecker.c
 */

#include "ProcessMonitor.h"
#include "ProcessInfoChecker.h"
#include "myList.h"
#include "cJSON.h"
#include <stdlib.h>
#include "util.h"
#include "myEqual.h"
#include "myHashCode.h"
#include "myHashMap.h"
#include "spdloglib.h"
#if MODULE_PROCESSMONITOR 

char *const EVENT_ON_WHITE_PROCESS_UNSTART_PARAM_KEY     = "whiteProcessUnstartList";
char *const EVENT_ON_WHITE_PROCESS_CLOSED_PARAM_KEY      = "whiteProcessClosedList";
char *const EVENT_ON_WHITE_PROCESS_RESTART_PARAM_KEY     = "whiteProcessRestartList";
char *const EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY     = "nonWhiteProcessRunList";
char *const EVENT_ON_NON_WHITE_PROCESS_RESTART_PARAM_KEY = "nonWhiteProcessRestartList";

static MyHashMap *lastProcessMap;
static MyHashMap *currentProcessMap;
static list *whiteProcessName;


void ProcessCheck_initProcessInfoChecker(MyHashMap **lastProcessMapTemp, MyHashMap **currentProcessMapTemp, list **whiteProcessNameTemp)
{
    lastProcessMap = (NULL == *lastProcessMapTemp ? createMyHashMap(myHashCodeString, myEqualString) : *lastProcessMapTemp);
    currentProcessMap = (NULL == *currentProcessMapTemp ? createMyHashMap(myHashCodeString, myEqualString) : *currentProcessMapTemp);
    whiteProcessName = (NULL == *whiteProcessNameTemp ? (list*)(malloc(sizeof(list))) : *whiteProcessNameTemp);
    *lastProcessMapTemp = lastProcessMap;
    *currentProcessMapTemp = currentProcessMap;
    *whiteProcessNameTemp = whiteProcessName;
}
/**
 * @description: pass parameter ret in order to conform to the rule who malloc who free(decoupling) 
 * @param {type} 
 * @return: 
 */
static char *getJsonString(const char *processName, long long pid, char *res)
{
    // char *ret = (char *)malloc(strlen(processName) + 25);
    char *ret = res;
    sprintf(ret, "{\"process_name\":\"%s\",\"pid\":\"%lld\"}", processName, pid);
    return ret;
}


static char *getJsonStringFromL(MyList *list, char *res)
{
    char *local = (char *)malloc(256);//max process info
    char *ret = res;
    if (NULL == list || list->count == 0)
    {
        memcpy(ret,"[]",strlen("[]"));
	//printf("61@@@@@@@@@@@@@@@ret:%s",ret);
        return ret;
    }
    for (int i = 0; i < list->count; i++)
    {
        memset(local,0,256);
        snprintf(local, 256,"%s,", (char *)myListGetDataAt(list, i));
//	printf("68@@@@@@@@@@@@@@@local：%s",local);
        free( (char *)myListGetDataAt(list, i));
        strcat(ret,local);
//	printf("72@@@@@@@@@@@@@@@ret：%s&&&local:%s",ret,local);
    }
    if (strlen(ret) > 0) // delete last comma
    {
        ret[strlen(ret)-1] = '\0';
    }
    free(local);
   
   // sprintf(ret, "[%s]", ret);
   char* temp1=(char*)malloc(strlen(ret)+20);
   memset(temp1,0,strlen(ret)+20);
   temp1[0]='[';
   char* temp2="]";
 //  sprintf(ret,"%s%s%s",temp1,ret,temp2);
    strcat(temp1,ret);
    strcat(temp1,temp2);
    free(ret);
    ret=temp1;
  // printf("77^^^^^^^^^^^^^^^^^^^ret:%s",ret);
    return ret;
}

MyHashMap *ProcessCheck_formatStringToProcessMap(char *processString)
{
    int n = 0;
    MyHashMap *processMap = createMyHashMap(myHashCodeString, myEqualString);
    cJSON *processMapJSON = cJSON_Parse(processString);
    if (!processMapJSON)
    {
        log_i("processmonitor","formatStringToProcessMap processString parse NULL");
        return processMap;
    }
    n = cJSON_GetArraySize(processMapJSON);
    cJSON *elem;
    cJSON *pid;
    cJSON *process_name;
    for (int i = 0; i < n; i++)
    {
        elem = cJSON_GetArrayItem(processMapJSON, i);
        pid = cJSON_GetObjectItem(elem, "pid");
        process_name = cJSON_GetObjectItem(elem, "process_name");
        char *sProcessName = process_name->valuestring;
        long long iPid = pid->valueint;
        if (NULL != sProcessName && strlen(sProcessName) > 0 && iPid > 0)
        {
            char* local = (char*)malloc(strlen(sProcessName) + 1);
            memset(local,0,strlen(sProcessName) + 1);
            memcpy(local,sProcessName,strlen(sProcessName));
            char* localpid = (char*)malloc(64);
            memset(localpid,0,64);
            snprintf(localpid,64,"%lld",iPid);
            myHashMapPutData(processMap, local, localpid);
        }
    }                   
    cJSON_Delete(processMapJSON);   
    return processMap;
}
/**
 * @description: pay attention to free myhashmap memory malloced within this method 
 * @param {type} 
 * @return: 
 */
MyHashMap *ProcessCheck_checkProcessForFirstTime()
{
    MyHashMap *resultMap = createMyHashMap(myHashCodeString, myEqualString);
    MyList *whiteProcessUnstartList = createMyList();
    MyList *nonWhiteProcessRunList = createMyList();
//    int sizeOfWhiteProcessName = list_size(whiteProcessName);
	list_elmt *cur_elmt = list_head(whiteProcessName);
	while(cur_elmt != NULL)
	{
		char *processName = cur_elmt->data;
        char *currentPid = (char *)myHashMapGetDataByKey(currentProcessMap, processName);
        if (NULL == currentPid)
        { //当前进程没有,则说明白名单进程未启动
            char *jsonStrRes = (char *)malloc(128);
            memset(jsonStrRes,0,128);
            myListInsertDataAtLast(whiteProcessUnstartList, getJsonString(processName, -1, jsonStrRes));
        }
		cur_elmt = cur_elmt->next;
	}
    MyHashMapEntryIterator *it = createMyHashMapEntryIterator(currentProcessMap);
    while (myHashMapEntryIteratorHasNext(it))
    {
        Entry *pp = myHashMapEntryIteratorNext(it);
        char *key = pp->key;    //processName
        char *value = pp->value; //pid
        int containFlag = 0;
        cur_elmt = list_head(whiteProcessName);
        while(cur_elmt != NULL)
        {
            char *oneWhiteProcessName = cur_elmt->data;
            if (strcmp(oneWhiteProcessName, key) == 0)
                containFlag = 1;
            cur_elmt = cur_elmt->next;
        }
        if (!containFlag) //当前进程不属于白名单，则说明非白名单进程运行
        {
            char *jsonStrRes1 = (char *)malloc(256);
            memset(jsonStrRes1,0,256);
            long long  tempvalue = atoll(value);
            myListInsertDataAtLast(nonWhiteProcessRunList, getJsonString(key, tempvalue, jsonStrRes1));
            // free(jsonStrRes);
        }
    }
    freeMyHashMapEntryIterator(it);
   // freeMyHashMap(currentProcessMap);

    int sizeOfWhiteProcessUnstartList = myListGetSize(whiteProcessUnstartList);
    int sizeOfNonWhiteProcessRunList = myListGetSize(nonWhiteProcessRunList);
    if (sizeOfWhiteProcessUnstartList > 0)
    {
        char *jsonStrRes2 = (char *)malloc(64 * sizeOfWhiteProcessUnstartList);
        memset(jsonStrRes2,0,64 * sizeOfWhiteProcessUnstartList);
        char * local = malloc(strlen(EVENT_ON_WHITE_PROCESS_UNSTART_PARAM_KEY) + 1);
        memset(local,0,strlen(EVENT_ON_WHITE_PROCESS_UNSTART_PARAM_KEY) + 1);
        memcpy(local,EVENT_ON_WHITE_PROCESS_UNSTART_PARAM_KEY,strlen(EVENT_ON_WHITE_PROCESS_UNSTART_PARAM_KEY));
        myHashMapPutData(resultMap, local, getJsonStringFromL(whiteProcessUnstartList, jsonStrRes2));
        //free(jsonStrRes);// if free here ,the json string returned will be null
    }
    if (sizeOfNonWhiteProcessRunList > 0)
    {
        char *jsonStrRes3 = (char *)malloc(64 * sizeOfNonWhiteProcessRunList);
        memset(jsonStrRes3,0,64 * sizeOfNonWhiteProcessRunList);
        char * local = malloc(strlen(EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY) + 1);
        memset(local,0,strlen(EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY) + 1);
        memcpy(local,EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY,strlen(EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY));
        myHashMapPutData(resultMap, local, getJsonStringFromL(nonWhiteProcessRunList, jsonStrRes3));
    }
    freeMyList(whiteProcessUnstartList);
    freeMyList(nonWhiteProcessRunList);
    return resultMap;
}

MyHashMap *ProcessCheck_checkProcessForAnotherTime()
{
    MyHashMap *resultMap = createMyHashMap(myHashCodeString, myEqualString);
    MyList *whiteProcessClosedList = createMyList();
    MyList *whiteProcessRestartList = createMyList();
    MyList *nonWhiteProcessRunList = createMyList();
    MyList *nonWhiteProcessRestartList = createMyList();
    // int sizeOfWhiteProcessName = list_size(whiteProcessName);
    list_elmt *cur_elmt = list_head(whiteProcessName);
	while(cur_elmt != NULL)
	{
		char *processName = cur_elmt->data;
        char *currentPid = (char *)myHashMapGetDataByKey(currentProcessMap, processName);
        if (NULL == currentPid)
        { //当前进程没有,则说明白名单进程未启动
             char *tempLastPorcessPid = (char *)myHashMapGetDataByKey(lastProcessMap, processName);
             if(NULL == tempLastPorcessPid)
             {
                 char *jsonStrRes = (char *)malloc(128);
                 memset(jsonStrRes,0,128);
                 myListInsertDataAtLast(whiteProcessClosedList, getJsonString(processName, -1, jsonStrRes));
             }
        }
        else
        {
            char *lastPid = (char *)myHashMapGetDataByKey(lastProcessMap, processName);
            if (lastPid != NULL)
            { //当前进程有，之前进程也有，但是pid不一样，则说明白名单进程重启
                long long lastPidlld = atoll(lastPid);
                long long currentPidlld = atoll(currentPid);
                if(currentPidlld != lastPidlld){
                     char *jsonStrRes1 = (char *)malloc(128);
                     memset(jsonStrRes1,0,128);
                     myListInsertDataAtLast(whiteProcessRestartList, getJsonString(processName, currentPidlld, jsonStrRes1));
                 }
            }
        }
		cur_elmt = cur_elmt->next;
	}
    MyHashMapEntryIterator *it = createMyHashMapEntryIterator(currentProcessMap);
    while (myHashMapEntryIteratorHasNext(it))
    {
        Entry *pp = myHashMapEntryIteratorNext(it);
        char *key = pp->key;    //processName
        char *value = pp->value; //pid
        int containFlag = 0;
        long long valuelld = atoll(value);
        cur_elmt = list_head(whiteProcessName);
        while(cur_elmt != NULL)
        {
            char *oneWhiteProcessName = cur_elmt->data;
            if (strcmp(oneWhiteProcessName, key) == 0)
                containFlag = 1;
            cur_elmt = cur_elmt->next;
        }

        if (!containFlag) //当前进程不属于白名单
        {   
            long long lastPidlld = -1;
            char *lastPid = (char *)myHashMapGetDataByKey(lastProcessMap, key);
            char *jsonStrRes2 = (char *)malloc(128);
            memset(jsonStrRes2,0,128);
            if (lastPid != NULL)
                 lastPidlld = atoll(lastPid);
 
            if (lastPid == NULL)
            { 
                //当前进程不属于白名单，并且之前进程没有此进程，则说明非白名单进程运行
                 myListInsertDataAtLast(nonWhiteProcessRunList, getJsonString(key, valuelld, jsonStrRes2));//test sucess 
            }else if(lastPidlld == valuelld)
            {
               //当前进程不属于白名单，并且之前进程和此进程pid相同，则说明非白名单进程一直在运行
                 myListInsertDataAtLast(nonWhiteProcessRunList, getJsonString(key, valuelld, jsonStrRes2));//test sucess
            }else
            {   //当前进程不属于白名单，并且之前进程和此进程pid不相同，则说明非白名单进程重启
                 myListInsertDataAtLast(nonWhiteProcessRestartList, getJsonString(key, valuelld, jsonStrRes2));//test sucess
            }
        }
    }
    freeMyHashMapEntryIterator(it);
   // freeMyHashMap(currentProcessMap);
    int sizeOfWhiteProcessClosedList = myListGetSize(whiteProcessClosedList);
    int sizeOfWhiteProcessRestartList = myListGetSize(whiteProcessRestartList);
    int sizeOfNonWhiteProcessRunList = myListGetSize(nonWhiteProcessRunList);
    int sizeOfNonWhiteProcessRestartList = myListGetSize(nonWhiteProcessRestartList);
    if (sizeOfWhiteProcessClosedList > 0)
    {
        char *jsonStrRes2 = (char *)malloc(64 * sizeOfWhiteProcessClosedList);
        memset(jsonStrRes2,0,64 * sizeOfWhiteProcessClosedList);
        char * local = malloc(strlen(EVENT_ON_WHITE_PROCESS_CLOSED_PARAM_KEY) + 1);
        memset(local,0,strlen(EVENT_ON_WHITE_PROCESS_CLOSED_PARAM_KEY) + 1);
        memcpy(local,EVENT_ON_WHITE_PROCESS_CLOSED_PARAM_KEY,strlen(EVENT_ON_WHITE_PROCESS_CLOSED_PARAM_KEY));
        myHashMapPutData(resultMap, local, getJsonStringFromL(whiteProcessClosedList, jsonStrRes2));
        //free(jsonStrRes);// if free here ,the json string returned will be null
    }
    if (sizeOfWhiteProcessRestartList > 0)
    {
        char *jsonStrRes3 = (char *)malloc(64 * sizeOfWhiteProcessRestartList);
        memset(jsonStrRes3,0,64 * sizeOfWhiteProcessRestartList);
        char * local = malloc(strlen(EVENT_ON_WHITE_PROCESS_RESTART_PARAM_KEY) + 1);
        memset(local,0,strlen(EVENT_ON_WHITE_PROCESS_RESTART_PARAM_KEY) + 1);
        memcpy(local,EVENT_ON_WHITE_PROCESS_RESTART_PARAM_KEY,strlen(EVENT_ON_WHITE_PROCESS_RESTART_PARAM_KEY));
        myHashMapPutData(resultMap, local, getJsonStringFromL(whiteProcessRestartList, jsonStrRes3));
    }
    if (sizeOfNonWhiteProcessRunList > 0)
    {
        char *jsonStrRes4 = (char *)malloc(64 * sizeOfNonWhiteProcessRunList);
        memset(jsonStrRes4,0,64 * sizeOfNonWhiteProcessRunList);
        char * local = malloc(strlen(EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY) + 1);
        memset(local,0,strlen(EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY) + 1);
        memcpy(local,EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY,strlen(EVENT_ON_NON_WHITE_PROCESS_RUN_PARAM_KEY));
        myHashMapPutData(resultMap, local, getJsonStringFromL(nonWhiteProcessRunList, jsonStrRes4));
        //free(jsonStrRes);// if free here ,the json string returned will be null
    }
    if (sizeOfNonWhiteProcessRestartList > 0)
    {
        char *jsonStrRes5 = (char *)malloc(64 * sizeOfNonWhiteProcessRestartList);
        memset(jsonStrRes5,0,64 * sizeOfNonWhiteProcessRestartList);
        char * local = malloc(strlen(EVENT_ON_NON_WHITE_PROCESS_RESTART_PARAM_KEY) + 1);
        memset(local,0,strlen(EVENT_ON_NON_WHITE_PROCESS_RESTART_PARAM_KEY) + 1);
        memcpy(local,EVENT_ON_NON_WHITE_PROCESS_RESTART_PARAM_KEY,strlen(EVENT_ON_NON_WHITE_PROCESS_RESTART_PARAM_KEY));

        myHashMapPutData(resultMap, local, getJsonStringFromL(nonWhiteProcessRestartList, jsonStrRes5));
    }
    freeMyList(whiteProcessClosedList);
    freeMyList(whiteProcessRestartList);
    freeMyList(nonWhiteProcessRunList);
    freeMyList(nonWhiteProcessRestartList);
    return resultMap;
}

#endif
