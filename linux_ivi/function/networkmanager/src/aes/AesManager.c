

#include "AesManager.h"
#include "aes/KeyManager.h"
#include "BaseResponseData.pb-c.h"
#include "util/Tool.h"
#include "log_util.h"
#include "spdloglib.h"
/**
 ******************************************************************************
 ** \简  述  用会话密钥加密数据
 **  注  意  使用完毕需要对返回的空间进行释放
 ** \参  数  sourceData:待加密数据 
 ** \返回值   密文，失败返回NULL
 ** \作  者  
 ******************************************************************************/
cypher_t *encyptDataByAesAndSessionKey(const char *sourceData,int length,void* _curl) {
    char key[17]={0};
    char iv[17]={0};
    char *pdata=NULL;
    if(sourceData==NULL)
        return NULL;
    pdata = getSessionKey(_curl); 
    if(pdata == NULL)
        return NULL;
    memcpy(key,pdata,16);
    key[16] = '\0';
    memcpy(iv,pdata+16,16);
    iv[16] = '\0';
    int len = (length == 0)?(strlen(sourceData)):(length);
    cypher_t *plain = (cypher_t *) malloc(sizeof(int) + len + 2);
    plain->len_data = len;
    memset(plain->data,0,plain->len_data + 2);
    memcpy(plain->data, sourceData, plain->len_data);

    cypher_t *cypher = aes_cbc_encrypt(plain, key, iv);
    free(plain);
    if (cypher) 
        return cypher;
    return NULL;
}
/**
 ******************************************************************************
 ** \简  述  用管理者密钥加密数据
 **  注  意  使用完毕需要对返回的空间进行释放
 ** \参  数  sourceData:待加密数据 
 ** \返回值   密文，失败返回NULL
 ** \作  者  
 ******************************************************************************/
cypher_t *encyptDataByAesAndManageKey(const char *sourceData,void* _curl ) 
{
    char key[17]={0};
    char iv[17]={0};
    char *pdata=NULL;
    if(sourceData==NULL)
        return NULL;
    pdata = getManageKey(_curl);
    if(pdata == NULL)
        return NULL;
    memcpy(key,pdata,16);
    key[16] = '\0';	
    memcpy(iv,pdata+16,16);
    iv[16] = '\0';
    //加密
    cypher_t *plain = (cypher_t *) malloc(sizeof(int) + strlen(sourceData) + 2);
    plain->len_data = strlen(sourceData);
    memset(plain->data,0,plain->len_data + 2);
    memcpy(plain->data, sourceData, plain->len_data);
    cypher_t *cypher = aes_cbc_encrypt(plain, key, iv);
    free(plain);
    if (cypher) 
        return cypher;
    return NULL;
}

/**
 ******************************************************************************
 ** \简  述  解析网络请求的响应数据.
 **  注  意  使用完毕需要调用函数 freeResponseDataByDecyptMemory(parseResponseData_t *p) 释放内存
 ** \参  数  sourceData:待加密数据 
 ** \返回值   密文，失败返回NULL
 ** \作  者  may leak
 ******************************************************************************/
parseResponseData_t *getResponseDataByDecypt(BaseResponse_t *response,  char *fullKey)//( char *encyptData,  char *fullKey,  char *responseHeader)
{
    char iv[KEY_IV_LENGTH+1]  = {0};
    char key[KEY_IV_LENGTH+1] = {0};
    parseResponseData_t *plainResponse = NULL;
    plainResponse = (parseResponseData_t *)malloc(sizeof(parseResponseData_t));
    if(plainResponse == NULL){
        log_e("networkmanager", "Decypt data molloc err");
        return NULL;
    }
        
    plainResponse->networkError = false;
    plainResponse->status_code  = response->status_code;
    plainResponse->errorMsg     = NULL;
    if(fullKey == NULL) //注册
    {
        plainResponse->responseData = (char *)malloc(response->responseDatalength+1);
        if(plainResponse->responseData == NULL)
        {
            free(plainResponse);
            return NULL;
        }
        memset(plainResponse->responseData,0,response->responseDatalength+1);
        memcpy(plainResponse->responseData,response->responseData,response->responseDatalength); 
    }
    else
    {
        char*  encyptData = response->responseData;
        memcpy(key,fullKey,KEY_IV_LENGTH);
        key[KEY_IV_LENGTH]='\0';
        memcpy(iv,fullKey+16,KEY_IV_LENGTH);
        iv[KEY_IV_LENGTH]='\0';
        cypher_t* plain = (cypher_t*)malloc(sizeof(cypher_t) + response->responseDatalength);
        plain->len_data = response->responseDatalength;
        memcpy(plain->data, encyptData, plain->len_data);  
        cypher_t* cypher = aes_cbc_decrypt(plain,key,iv);
        if(!cypher)
        {
            free(plain);
            free(plainResponse);
            log_e("networkmanager", "aes_cbc_decrypt err");
            return NULL;
        }
        log_v("[IDS decrypt]:\n",cypher->data);
        plainResponse->responseData = (char *)malloc(cypher->len_data+1);
        if(plainResponse->responseData == NULL)
        {
            free(plain);
            free(cypher);
            free(plainResponse);
            return NULL;
        }
        memset(plainResponse->responseData,0,cypher->len_data+1);
        strncpy(plainResponse->responseData,cypher->data,cypher->len_data);
        free(plain);
        free(cypher); 
    }
    return plainResponse;
}

void freeResponseDataByDecyptMemory(parseResponseData_t *p)
{
    if(p == NULL)
        return;
    if(p->errorMsg)
        free(p->errorMsg);
    if(p->responseData)
        free(p->responseData);
    free(p);
}
