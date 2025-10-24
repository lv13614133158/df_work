/*
 * @Author: your name
 * @Date: 2021-05-21 11:01:48
 * @LastEditTime: 2021-05-21 13:26:00
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \idps_networkmanager\idps_networkmanager\idps_networkmanager\src\includes\data\curlWrapper.h
 */
//
// Created by xuewenliang on 2020/1/8.
//
#ifndef __CURLWRAPPER_H
#define __CURLWRAPPER_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C"{
#endif
extern char mqttpath[128];
typedef struct BaseResponse{
    bool networkError;
    char *responseData;
    char *responseHeader;
    char *errorMsg;
    int status_code;
    int errorMsglength;
    int responseHeaderlength;
    int responseDatalength;
}BaseResponse_t;
typedef struct parseResponseData{
    bool networkError;
    int status_code;
    char *responseData;
    char *errorMsg;
}parseResponseData_t;

void curl_init();
BaseResponse_t *startPostfile(char* header,const char * remotepath, const char * localpath,const char* filename);
BaseResponse_t *startGetRequest(char *url, char *header,void* __curl);
BaseResponse_t *startPostRequest(char *url, char *header,int lengthheader,char *requestBody,int bodylength,void* __curl);
void freeRequestMemory(BaseResponse_t *p);
#ifdef __cplusplus
}
#endif
#endif //__CURLWRAPPER_H
