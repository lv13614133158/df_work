//
// Created by wangchunlong1 on 2019/9/10.
//

#ifndef CURLHTTPCLIENT_KEYMANAGER_H
#define CURLHTTPCLIENT_KEYMANAGER_H
#ifdef __cplusplus
extern "C"{
#endif

typedef struct KeyManager{
    char indexKey[128];
    char indexSN[128];
    int  mode;
    char manageKey[128];
    char sn[128];
    char sessionKey[128];
    char token[512];
}keyManager_t;

void initMode(int mode);
char *getSessionKey(void* _curl);
char *gettoken(void* _curl);

void setManageKeyToEmpty();
void setSessionKeyToEmpty();

char *getSessionKey(void* _curl);
char *getManageKey(void* _curl);

char *getSN(void* _curl);
void NetWorkManagerSql(void);
#ifdef __cplusplus
}
#endif
#endif //CURLHTTPCLIENT_KEYMANAGER_H


