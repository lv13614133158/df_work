//
// Created by wangchunlong1 on 2019/9/29.
//

#ifndef CURLHTTPCLIENT_AESMANAGER_H
#define CURLHTTPCLIENT_AESMANAGER_H

#include "cryptogram.h"
#include "curlWrapper.h"
#ifdef __cplusplus
extern "C"{
#endif
#define SESSION_MANAGE_KEY_LENGTH 32

cypher_t *encyptDataByAesAndSessionKey(const char *sourceData,int length,void* _curl);
cypher_t *encyptDataByAesAndManageKey(const char *sourceData,void* _curl);

parseResponseData_t *getResponseDataByDecypt(BaseResponse_t *response,  char *fullKey);
void freeResponseDataByDecyptMemory(parseResponseData_t *p);
#ifdef __cplusplus
}
#endif
#endif //CURLHTTPCLIENT_AESMANAGER_H
