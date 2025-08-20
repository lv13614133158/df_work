#ifndef __CCSMEC_NVRAM_API_H__
#define __CCSMEC_NVRAM_API_H__

#include <string>
#include <iostream>
#include <string.h>

typedef int (*cmNotifyFun)(int notifyId, const char *payload, const unsigned int len);

int cm_nvramInit(cmNotifyFun callback);

int cm_nvramConnectStatus(void);

int cm_nvramWrite(const char* group,  const char* key, int flag, int tid, const char* val, const unsigned int len);

int cm_nvramRead(const char* group,  const char* key, int flag, int tid, const char* val, const unsigned int len);

bool cm_nvramCompareTid(const std::string& group, const std::string& key,int flag,int tid,int mtid); 

bool cm_nvramAddSubscribe(const std::string& group, const std::string& key,int flag,int tid);

#endif


