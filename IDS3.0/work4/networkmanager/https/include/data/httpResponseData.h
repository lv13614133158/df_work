
#ifndef __HTTPRESPONSEDATA_H
#define __HTTPRESPONSEDATA_H

#include "data/curlWrapper.h"
#include "aes/AesManager.h"
#ifdef __cplusplus
extern "C"{
#endif
parseResponseData_t *startPost(char *url,char *header,int headerlength,char *body,int bodylength,void* _curl);
parseResponseData_t *startGet(char *url, char *header,void* _curl);
void checkCodeForKey(int code,void* _curl);
#ifdef __cplusplus
}
#endif
#endif