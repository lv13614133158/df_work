#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "util/Tool.h"
#include "data/ConfigConstData.h"
#include "util/log_util.h"
#include "myHmac.h"
#include "base64.h"
 
#define STR_LEN 8//定义随机输出的字符串长度。
#define UPDATE_TIME_SUFFIX "&update_time="
#define UPDATE_SN_SUFFIX   "?sn="
#define KEY_IV_LENGTH 16
/**
 ******************************************************************************
 ** \简  述  生成随机字符串
 **  注  意  返回的是静态数组指针，不需要释放
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
void GenerateStr(char* str)
{
    int i = 0;
    srand(time(NULL));//通过时间函数设置随机数种子，使得每次运行结果随机。
    for(i = 0; i < STR_LEN; i ++)
    {
        switch(0)
        {
            case 0:
                str[i] = rand()%26 + 'a';
                break;
        }
    }
    str[STR_LEN] = '\0';
} 
/**
 ******************************************************************************
 ** \简  述  获取hmac值
 **  注  意  返回指针使用后需要释放内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char* getHmacFromData(char *key, uint8_t* data, int dataLen)
{
    unsigned char hmac[100] = {0};
    char *base64_hmac=NULL;
    hmac_md5((unsigned char*)key, strlen(key), data, dataLen, hmac);
    size_t data_length_hmac = 0;
    base64_hmac = (char*)base64_encode((unsigned char*)hmac, 16,  &data_length_hmac);
    // base64_hmac[data_length_hmac - 1] = '\0';
    return base64_hmac;
}
/**
 ******************************************************************************
 ** \简  述  获取带时间戳的url路径
 **  注  意  返回数组地址，需要释放
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char *getUrlWithUpdateTime(char *url,const long long updateTime,char *sn)
{
    uint16_t l_urllenbefore = strlen(getBaseUrl()) + strlen(url);
    char _url[l_urllenbefore + 20];
    memset(_url,0,l_urllenbefore + 20);
    memcpy(_url,getBaseUrl(),strlen(getBaseUrl()));
    strcat(_url,url);
 
    int l_urllen = strlen(_url);
    char *urlArr = malloc(l_urllen + 64);
    memset(urlArr,0,l_urllen + 64);
    snprintf(urlArr,l_urllen + 64,"%s%s%s%s%lld",_url,UPDATE_SN_SUFFIX,sn,UPDATE_TIME_SUFFIX,updateTime);
    return urlArr;
}

