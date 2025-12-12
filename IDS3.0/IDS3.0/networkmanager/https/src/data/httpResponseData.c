/*
*   Created by xuewenliang on 2020/1/9.
*/
#include <stdio.h>
#include <stdlib.h>
#include "data/httpResponseData.h"
#include "aes/AesManager.h"
#include "aes/KeyManager.h"
#include <unistd.h>
#include "spdloglib.h"
#include "cJSON.h"

/**
 ******************************************************************************
 ** \简  述  基本libcurl返回错误吗的定义
 **  注  意   
 ** \参  数 
 ** \时  间  20200701
 ** \作  者  
 ******************************************************************************/
#define ERROR_INVALID_TOKEN      -1001001
#define ERROR_WRONG_SESSION_KEY  -1000001
#define ERROR_WRONG_MANAGE_KEY   -1000002
/**
 ******************************************************************************
 ** \简  述  启动post请求
 **  注  意  返回指针不为NULL需要调用函数 freeResponseDataByDecyptMemory(parseResponseData_t *p) 释放内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
parseResponseData_t *startPost(char *url,char *header,int headerlength,char *body,int bodylength,void* _curl )
{
    parseResponseData_t *newdata=NULL;
    char spdlog[512] = {0};
    snprintf(spdlog, 256, "url:%s;header:%s,%d;body:%s,%d", url, header, headerlength, body, bodylength);
    log_v("IDS Post",spdlog);
    BaseResponse_t *response = startPostRequest(url,header,headerlength,body,bodylength,_curl);
    if(response == NULL){
        return NULL;
    }
    log_v("IDS Post_Ret",response->responseData);
    if(response->status_code != 0){//网络错误
        if((newdata = (parseResponseData_t *)malloc(sizeof(parseResponseData_t)) ) == NULL){
            freeRequestMemory(response);
            return NULL;
        }
        newdata->networkError = true;
        newdata->status_code = response->status_code;
        newdata->responseData = NULL;
        if((newdata->errorMsg = (char *)malloc(strlen(response->errorMsg)+1)) == NULL){
            free(newdata);
            freeRequestMemory(response);
            return NULL;
        }
        strncpy(newdata->errorMsg,response->errorMsg,strlen(response->errorMsg));
        freeRequestMemory(response);
        return newdata;
    }
    if(strstr(url,"register") != NULL){
        newdata = getResponseDataByDecypt(response,NULL);
    }
    else if(strstr(url,"session") != NULL){
        newdata = getResponseDataByDecypt(response,getManageKey(_curl));
         if (!newdata)
         {
             cJSON *cJSONArray = cJSON_Parse(response->responseData);
             if (cJSONArray)
             {
                 cJSON *cJSONCode  = cJSON_GetObjectItem(cJSONArray, "code");
                 checkCodeForKey(cJSONCode->valueint,_curl);
                 cJSON_Delete(cJSONArray);
             }
         }
    }
    else{
        newdata = getResponseDataByDecypt(response,getSessionKey(_curl));
    }
    if(newdata){
        checkCodeForKey(newdata->status_code,_curl);
    }
    freeRequestMemory(response);
    return newdata;
}
/**
 ******************************************************************************
 ** \简  述  启动get请求
 **  注  意   
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
parseResponseData_t *startGet(char *url, char *header,void* _curl)//
{
    parseResponseData_t *newdata = NULL;
    char spdlog[256] = {0};
    snprintf(spdlog, 256, "url:%s;header:%s", url, header);
    log_v("IDS Get",spdlog);
    BaseResponse_t *baseResponse = startGetRequest(url,header,_curl);
    if(baseResponse == NULL){
        return NULL;
    }
    log_v("IDS Get_Ret",baseResponse->responseData);
    if(baseResponse->status_code!=0){ //网络错误
        if((newdata = (parseResponseData_t *)malloc(sizeof(parseResponseData_t))) == NULL){
            freeRequestMemory(baseResponse);
            return NULL;
        }
        newdata->networkError = true;
        newdata->status_code = baseResponse->status_code;
        newdata->responseData = NULL;
        if((newdata->errorMsg = (char *)malloc(strlen(baseResponse->errorMsg)+1))==NULL){
            free(newdata);
            freeRequestMemory(baseResponse);
            return NULL;
        }
        strncpy(newdata->errorMsg,baseResponse->errorMsg,strlen(baseResponse->errorMsg));
        freeRequestMemory(baseResponse);
        return newdata;
    }
    newdata = getResponseDataByDecypt(baseResponse,getSessionKey(_curl));
    if(newdata){
        checkCodeForKey(newdata->status_code,_curl);
    }
    freeRequestMemory(baseResponse);
    return newdata;
}
/**
 ******************************************************************************
 ** \简  述  检查状态码，判断密钥是否过期或者异常
 **  注  意  
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
void checkCodeForKey(int code,void* _curl)
{
    char spdlog[128] = {0};
    snprintf(spdlog, 128, "code:%d", code);
    log_v("checkCodeForKey",spdlog);

    switch(code){
        case ERROR_INVALID_TOKEN://The auth token is invalid
            setSessionKeyToEmpty();
            getSessionKey(_curl);
        break;
        case ERROR_WRONG_SESSION_KEY://客户端会触发重新协商会话密钥的动作
            setSessionKeyToEmpty();
            getSessionKey(_curl);
        break;
        case ERROR_WRONG_MANAGE_KEY://客户端会触发重新协商注册密钥的动作
            setManageKeyToEmpty();
            getManageKey(_curl);
            log_v("checkCodeForKey","IDPS exit!");
            exit(0);

        break;
        default:
        break;
    }
}
