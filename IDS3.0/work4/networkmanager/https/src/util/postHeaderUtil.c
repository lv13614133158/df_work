#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util/postHeaderUtil.h"
#include "aes/KeyManager.h"
#include "util/Tool.h"
#include "data/ConfigConstData.h"
#include "util/log_util.h"
#include "myHmac.h"
#include "base64.h"
#include "AesManager.h"
/**
 ******************************************************************************
 ** \简  述  获取头部信息
 **  注  意  如果返回值不为NULL，需要释放指针指向的内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getHeaders(const char *encyptData, int encyptDataLen,void* _curl) 
{
    char *sessionKey=NULL,*base64_hmac=NULL;
    char *header=NULL;
    sessionKey =  getSessionKey(_curl);
    char key[17] = {0};
    if(!sessionKey)return NULL;
 
    memcpy(key,sessionKey,KEY_IV_LENGTH);
    key[KEY_IV_LENGTH]='\0';

    base64_hmac = getHmacFromData(key,(uint8_t*)encyptData, encyptDataLen);
    int l_malloclen = 96 + strlen(getChannelId())+strlen(getSN(_curl))+strlen(base64_hmac)+strlen(gettoken(_curl));
    header = (char *)malloc(l_malloclen);
    if(header == NULL)
    {
        free(base64_hmac);
        return NULL;
    }
    sprintf(header, "{\"X-Channel-ID\":\"%s\",\"X-Crypt\":\"2\",\"X-Proto-Buf\":\"0\","
                        "\"X-Sn\":\"%s\",\"X-HMAC\":\"%s\",\"Authorization\":\"%s\"}",
                getChannelId(),
                getSN(_curl),
                base64_hmac,
                gettoken(_curl));

    free(base64_hmac);
    return header;
}

char *getHeadersForHardware(const char *encyptData, int encyptDataLen,void* _curl) 
{
    char *sessionKey=NULL,*base64_hmac=NULL;
    char *header=NULL;
    sessionKey =  getSessionKey(_curl);
    char key[17] = {0};
    if(!sessionKey)return NULL;
 
    memcpy(key,sessionKey,KEY_IV_LENGTH);
    key[KEY_IV_LENGTH]='\0';

    base64_hmac = getHmacFromData(key,(uint8_t*)encyptData, encyptDataLen);
    int l_malloclen = 96 + strlen(getChannelId())+strlen(getSN(_curl))+strlen(base64_hmac)+strlen(gettoken(_curl));
    header = (char *)malloc(l_malloclen);
    if(header == NULL)
    {
        free(base64_hmac);
        return NULL;
    }
    sprintf(header, "{\"X-Channel-ID\":\"%s\",\"X-Crypt\":\"2\",\"X-Proto-Buf\":\"0\","
                        "\"X-Sn\":\"%s\",\"X-HMAC\":\"%s\",\"Authorization\":\"%s\"}",
                getChannelId(),
                getSN(_curl),
                base64_hmac,
                gettoken(_curl));

    free(base64_hmac);
    return header;
}

/**
 ******************************************************************************
 ** \简  述  获取头部信息的file
 **  注  意  
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getFileHeaders(void* _curl) 
{
    char *header = NULL;
    header = malloc(256);
    if(header == NULL)
        return NULL;
    memset(header,0,256);
    snprintf(header,256, "{\"X-Proto-Buf\":\"0\",\"X-Channel-ID\":\"%s\",\"X-Sn\":\"%s\",\"Authorization\":\"%s\"}",
                getChannelId(),
                getSN(_curl),
                gettoken(_curl));
    return header;
}
/**
 ******************************************************************************
 ** \简  述  获取管理者密钥头部信息
 **  注  意  如果返回值不为NULL，需要释放指针指向的内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getManageKeyHeaders() 
{
    int l_malloclen = 64;
    char *header = (char *)malloc(l_malloclen);
    if(header == NULL)
        return NULL;
    sprintf(header, "{\"X-Proto-Buf\":\"0\"}");
    return header;
}
/**
 ******************************************************************************
 ** \简  述  获取管理者密钥内容信息
 **  注  意  如果返回值不为NULL，需要释放指针指向的内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getManageKeyBoys() 
{
    int l_malloclen = 96 + strlen(getChannelId()) + strlen(getUdid()) + strlen(getEquipmentType());
    char *bodys = (char *)malloc(l_malloclen);
    if(bodys == NULL)
        return NULL;
    sprintf(bodys, "{\"channel_id\":\"%s\",\"udid\":\"%s\",\"equipment_type\":\"%s\",\"os_type\":\"linux\"}",
            getChannelId(),getUdid(),getEquipmentType());
    return bodys;
}
/**
 ******************************************************************************
 ** \简  述  获取会话密钥头部信息
 **  注  意  如果返回值不为NULL，需要释放指针指向的内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getSessionKeyHeaders(const char *encyptData, int encyptDataLen,void* _curl) 
{
    //hmac-md5
    unsigned char hmac[100] = {0};
    char *manageKey = getManageKey(_curl);
    char key[17] = {0};
    if(!manageKey)return NULL;
    memcpy(key,manageKey,KEY_IV_LENGTH);
    key[KEY_IV_LENGTH]='\0';

    hmac_md5((unsigned char *) key, strlen(key), (unsigned char *) encyptData, encyptDataLen,hmac);
    size_t data_length_hmac;
    char *base64_hmac = (char *) base64_encode((unsigned char *) hmac, 16, &data_length_hmac);
    // base64_hmac[data_length_hmac - 1] = '\0';
    int l_headerlen = 96 + strlen(getChannelId()) + strlen(getSN(_curl)) + strlen(base64_hmac);
    char *header = malloc(l_headerlen);
    memset(header,0,l_headerlen);
    if(header == NULL)
    {
        free(base64_hmac);
        return NULL;
    }

    sprintf(header, "{\"X-Channel-Id\":\"%s\",\"X-Crypt\":\"1\",\"X-Proto-Buf\":\"0\","
                    "\"X-Sn\":\"%s\",\"X-HMAC\":\"%s\",\"Authorization\":\"\"}",
            getChannelId(),getSN(_curl),base64_hmac);
    free(base64_hmac);
    return header;
}
/**
 ******************************************************************************
 ** \简  述  获取会话密钥内容信息
 **  注  意  如果返回值不为NULL，需要释放指针指向的内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getSessionKeyBoys(void* _curl) 
{
    int l_malloclen = 64;
    char str[STR_LEN+1] = {0};
    GenerateStr(str);
    size_t data_length = 0;
    char *key = (char *)base64_encode((unsigned char *)str,STR_LEN,&data_length);
    char *bodys = (char *)malloc(l_malloclen);
    if(bodys == NULL)
        return NULL;
    sprintf(bodys, "{\"key\":\"%s\"}", key);
    free(key);
    return bodys;
}

/**
 ******************************************************************************
 ** \简  述  获取get请求头部信息
 **  注  意  如果返回值不为NULL，需要释放指针指向的内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getGetRequestHeaders(void* _curl) 
{
    char* tempsn = getSN(_curl);
    char* tempToken = gettoken(_curl);
    if((tempsn == NULL)||(tempToken == NULL))
    {
        return NULL;
    }

    char *header = (char *)malloc(256 + strlen(getChannelId()) + strlen(getSN(_curl)) + strlen(gettoken(_curl)));
    if(header == NULL)
        return NULL;
    sprintf(header, "{\"X-Channel-ID\":\"%s\",\"X-Crypt\":\"2\",\"X-Proto-Buf\":\"0\","
                    "\"X-Sn\":\"%s\",\"Authorization\":\"%s\"}",
            getChannelId(),getSN(_curl),gettoken(_curl));
    return header;
}
