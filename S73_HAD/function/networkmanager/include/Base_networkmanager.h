/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-08 02:57:28
 */ 
 #ifndef __API_NETWORKMANAGER_H_
 #define __API_NETWORKMANAGER_H_
 #ifdef __cplusplus
extern "C"
{
#endif 
#include <stdio.h>
#include "common.h"
typedef struct _networkMangerMethod{
   void (*initTPSize)(int, int);
   void (*setServerConfig)(char* );
   void (*setDeviceConfig)(char* ,char*,char*);
   void (*setManageKeyStore)(int );
   void (*setHeartbeatInterval)(int);
   void (*newNetworkManager)(char *,char *);
   void (*freeNetWorkManager)(void);
   void (*startNetworkRequestManager)(void);
   void (*postGatherData)(char*, char* , int );
   void (*postEventDatawithpath)(const char*, char* , char* , char* , int );
   void (*postEventData)(const char* ,char* ,char* , int );
   void (*getUrlRequestData)(const char*, long long,char**,int*,int);
   void (*getUrlRequestDatanodate)(char*,char**,int*,int);
   void (*stopNetworkRequestManager)(void);
   char* (*getSn)(void);
   char* (*getToken)(void);
   char* (*getSessionKey)(void);
   char* (*getManagerKey)(void);
}networkMangerMethod;
extern networkMangerMethod  networkMangerMethodobj;
#ifdef __cplusplus
}
#endif
 #endif 
