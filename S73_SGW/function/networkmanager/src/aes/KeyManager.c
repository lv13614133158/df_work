/*
*   Created by xuewenliang on 2020/1/2.
*/
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "aes/KeyManager.h"
#include "util/Tool.h"
#include "data/dataToProtobufUtils.h"
#include "util/postHeaderUtil.h"
#include "data/httpResponseData.h"
#include "CJsonUtils.h"
#include "AesManager.h"
#include "util/log_util.h"
#include "ConfigConstData.h"
#include "spdloglib.h"
#include "ConfigParse.h"

#define GET_MANAGE_KEY_URL  "/api/v1/register"
#define GET_SESSION_KEY_URL "/api/v1/session"

static pthread_mutex_t gDBLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gDBLockManageKey = PTHREAD_MUTEX_INITIALIZER;

//模式默认为1
static keyManager_t keyManager={{0},{0},1,{0},{0},{0},{0}};
static void getManageKeyFromLocal();
static void getSNFromLocal();
static void getManageKeyFromNetWork(void* _curl);
static void getSessionKeyFromNetwork(void* _curl);

void initMode(int mode)
{
    keyManager.mode = mode;
    setMode(mode);
}
/*
*   brief:获取会话密钥
*   注意：第一次获取时，是耗时方法，因为需要初始化协商管理密钥和会话密钥
*   返回：返回会话密钥，失败返回NULL
*/
char *getSessionKey(void* _curl)
{
   if(keyManager.sessionKey[0]!='\0')
        return keyManager.sessionKey;
   if (networkFunctionEnabled())
   {
	   getSessionKeyFromNetwork(_curl);
	   if(keyManager.sessionKey[0]!='\0')
		   return keyManager.sessionKey;
   }
    return NULL;
}

char *gettoken(void* _curl)
{
    char *ret=NULL;
    ret = getSessionKey(_curl);
    if(ret == NULL)
        return NULL;
    return keyManager.token;
}

void setManageKeyToEmpty()
{
    delete_value(keyManager.indexKey);
    delete_value(keyManager.indexSN);
    memset(keyManager.manageKey,0,sizeof(keyManager.manageKey));
    memset(keyManager.indexKey, 0,sizeof(keyManager.indexKey));
    memset(keyManager.sn,0,sizeof(keyManager.sn));
    memset(keyManager.indexSN, 0,sizeof(keyManager.indexSN));

    setIndexKey(keyManager.indexKey);
    setIndexSN(keyManager.indexSN);
    setSessionKeyToEmpty();
}


void setSessionKeyToEmpty(void)
{
    memset(keyManager.sessionKey,0,sizeof(keyManager.sessionKey));
    memset(keyManager.token,0,sizeof(keyManager.token));
}

char *getManageKey(void* _curl)
{
    if(keyManager.manageKey[0]!='\0'){
        return keyManager.manageKey;
    }
    getManageKeyFromLocal();
    if(keyManager.manageKey[0]!='\0'){
        return keyManager.manageKey;
    }
	if (networkFunctionEnabled())
	{
	    getManageKeyFromNetWork(_curl);
	    if(keyManager.manageKey[0]!='\0'){
	        return keyManager.manageKey;
	    }
	}

    return NULL;
}

char *getSN(void* _curl)
{
    if(keyManager.sn[0]!='\0')
        return keyManager.sn;
    getSNFromLocal();
    if(keyManager.sn[0]!='\0')
        return keyManager.sn;
	if (networkFunctionEnabled())
	{
	    getManageKeyFromNetWork(_curl);
	    if(keyManager.sn[0]!='\0')
	        return keyManager.sn;
	}

    return NULL;
}
/**
 ******************************************************************************
 ** \简  述  模块相关的数据库初始化，从数据库中读取参数等
 **  注  意  
 ** \参  数
 ** \返回值
 ** \作  者  
 ******************************************************************************/
void   NetWorkManagerSql(void){
    char indexkeytemp[128] = {0};
    char indexSNtemp[128]  = {0};
    //createKeyInfoTable();
    getIndexKeyNetWork(indexkeytemp);
    if(strlen(indexkeytemp) > 0){
        memcpy(keyManager.indexKey,indexkeytemp,strlen(indexkeytemp));
    }
    getIndexSN(indexSNtemp);
    if(strlen(indexSNtemp) > 0){
        memcpy(keyManager.indexSN,indexSNtemp,strlen(indexSNtemp));
    }
    int modetemp = getMode();
    if(modetemp != -1)
    {
        keyManager.mode = modetemp;
    }
}

static void getManageKeyFromLocal()
{
    char tempManageKey[33]={0};
    if (keyManager.indexKey[0]=='\0'){
        return ;
    }
    int result =  get_value(keyManager.mode,keyManager.indexKey, tempManageKey);
    tempManageKey[32] = '\0';
    if (result != 0) {
        memset(keyManager.manageKey,0,sizeof(keyManager.manageKey));
        char spdlog[512] = {0};
        snprintf(spdlog,512,"get manage key from file fail: %d", result);
        log_e("networkmanager",spdlog);
        return ;
    }
    strncpy(keyManager.manageKey,tempManageKey,sizeof(keyManager.manageKey));
}

static void getSNFromLocal()
{
    if (keyManager.indexSN[0]=='\0') {
        return;
    }
    char tempSN[23] = {0};
    int result =  get_value(keyManager.mode,keyManager.indexSN, tempSN);
    tempSN[22] = '\0';
    if (result != 0) {
        memset(keyManager.sn,0,sizeof(keyManager.sn));
        char spdlog[512] = {0};
        snprintf(spdlog,512,"getSNFromLocal from file:%d", result);
        log_v("networkmanager",spdlog);
        return ;
    }
    strncpy(keyManager.sn,tempSN,sizeof(keyManager.sn) - 1);
}

static void getManageKeyFromNetWork(void* _curl)
{
    parseResponseData_t *response = NULL;
    if(pthread_mutex_lock(&gDBLockManageKey)!=0)
        return ;
    if(keyManager.manageKey[0]!='\0'&&keyManager.sn[0]!='\0')
    {
        pthread_mutex_unlock(&gDBLockManageKey);
        return;
    }
    
    uint16_t l_urllen = strlen(getBaseUrl()) + strlen(GET_MANAGE_KEY_URL);
    char cUrl[l_urllen + 2];
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,GET_MANAGE_KEY_URL);
    char *body=NULL;
    body = getManageKeyBoys();
    if(body==NULL)
    {
        pthread_mutex_unlock(&gDBLockManageKey);
        return;
    }
    char *header = NULL;
    header = getManageKeyHeaders();
    if(header == NULL)
    {
        pthread_mutex_unlock(&gDBLockManageKey);
        free(body);
        return;
    }
    response = startPost(cUrl,header,0,body,0,_curl);
    if(response == NULL)
        goto exit;
    if(response->status_code != 0)  //网络错误
    {  
        goto exit;
    }

    char *manageKey = NULL;
    manageKey = getinfofromCJsonstr(response->responseData,"register_key");
    if (manageKey == NULL)
        goto exit;
    printf("manangerkey from networkmanager is %s\n",manageKey);
    strncpy(keyManager.manageKey,manageKey,sizeof(keyManager.manageKey) - 1);
    char *tempIndexKey = set_value(keyManager.mode,manageKey);
    if(tempIndexKey)
    {
        strncpy(keyManager.indexKey,tempIndexKey,sizeof(keyManager.indexKey) - 1);
        setIndexKey(tempIndexKey);
        char spdlog[512] = {0};
        snprintf(spdlog,512,"save manage key %s",keyManager.indexKey);
        log_v("networkmanager",spdlog);

    	memset(spdlog,0,512); 
        snprintf(spdlog,512,"save manage key to db :%s", tempIndexKey);
        log_v("networkmanager",spdlog);

    	memset(spdlog,0,512); 
        snprintf(spdlog,512,"managekey:%s", manageKey);
        log_v("networkmanager",spdlog);        
    }
    char *sn = getinfofromCJsonstr(response->responseData,"sn");
    if(sn == NULL)
    {
        free(manageKey);
        goto exit;
    }
    strncpy(keyManager.sn,sn,sizeof(keyManager.sn) - 1);
    char *tempIndexSn = set_value(keyManager.mode,keyManager.sn);
    if(tempIndexSn)
    {
        strncpy(keyManager.indexSN,tempIndexSn,sizeof(keyManager.indexSN) - 1);
        setIndexSN(keyManager.indexSN);  
        char spdlog[512] = {0};
        snprintf(spdlog,512,"save indexSN to db :%s", tempIndexSn);
        log_v("networkmanager",spdlog);

        memset(spdlog,0,512);
        snprintf(spdlog,512,"sn:%s", sn);
        log_v("networkmanager",spdlog);
    }
        
    free(manageKey);
    free(sn);
exit:
    free(body);
    free(header);
    if(response)
        freeResponseDataByDecyptMemory(response);
    pthread_mutex_unlock(&gDBLockManageKey);
}

static void getSessionKeyFromNetwork(void *_curl)
{
    parseResponseData_t *response = NULL;
    if(pthread_mutex_lock(&gDBLock)!=0)
        return;
    if(getManageKey(_curl)==NULL)
    {
        pthread_mutex_unlock(&gDBLock);
        return;
    }
    if (keyManager.sessionKey[0]!='\0'&& keyManager.token[0]!='\0') 
    {
        pthread_mutex_unlock(&gDBLock);
        return;
    }
 
    uint16_t l_urllen = strlen(getBaseUrl()) + strlen(GET_SESSION_KEY_URL);
    char cUrl[l_urllen + 2];
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,GET_SESSION_KEY_URL);

    char *body=NULL;
    body = getSessionKeyBoys(_curl);
    if(body==NULL)
    {
        pthread_mutex_unlock(&gDBLock);
        return;
    }
    cypher_t* cypher = encyptDataByAesAndManageKey((char*)body,_curl);
    if (!cypher) {
        pthread_mutex_unlock(&gDBLock);
        free(body);
        log_e("networkmanager", "encypt error");
        return;
    }

    char *header = NULL;
    header = getSessionKeyHeaders(cypher->data,cypher->len_data,_curl);
    if(header == NULL)
    {
        pthread_mutex_unlock(&gDBLock);
        free(cypher);
        free(body);
        return;
    }
    char *sessionKey=NULL;
    char *token = NULL;
    if((response = startPost(cUrl,header,0,cypher->data,cypher->len_data,_curl)) == NULL){

        goto exit;
    }
    if(response->status_code != 0)
    {
        goto exit;
    }
    sessionKey = getinfofromCJsonstr(response->responseData,"session_key");
    if(sessionKey ==NULL)
    {
        goto exit;
    }
    token = getinfofromCJsonstr(response->responseData,"token");
    if(token == NULL)
    {
        free(sessionKey);
        goto exit;
    }
    log_v("session key manager:",sessionKey);
    strncpy(keyManager.sessionKey,sessionKey,sizeof(keyManager.sessionKey));
    strncpy(keyManager.token,token,sizeof(keyManager.token));
    getinfofromCJsonstr(response->responseData,"timestamp");//获取时间戳初始化时钟
    free(sessionKey);
    free(token);

exit:
    if(body)free(body);
    if(cypher)free(cypher);
    if(header)free(header);
    if(response)
        freeResponseDataByDecyptMemory(response);
    pthread_mutex_unlock(&gDBLock);
    return ;
}

