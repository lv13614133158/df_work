#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "curl/curl.h"
#include "cJSON.h"
#include "curlWrapper.h"
#include <pthread.h>
#include "ThreadPool.h"
#include "data/ConfigConstData.h"
#include "spdloglib.h"
#include "ConfigParse.h"

/**
 ******************************************************************************
 ** \简  述  线程保护
 **  注  意   
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
char mqttpath[128] = {0};
static char caPath[128] = {0}, caInfo[128] = {0};
static pthread_mutex_t gHandleProtect     = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gHandleProtectfile = PTHREAD_MUTEX_INITIALIZER;
/**
 ******************************************************************************
 ** \简  述  基本数据结构
 **  注  意   
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
struct MemoryStruct {
  char *memory;
  size_t size;
};
/**
 ******************************************************************************
 ** \简  述  post请求的body回调函数，
 **  注  意  参数为固定格式
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
static size_t writeFunctionCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL)return 0;
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

static size_t headerFunctionCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL)return 0;
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

static void setCurlCApath(char *mqttpath)
{
    memset(caPath, 0, strlen(caPath));
    memset(caInfo, 0, strlen(caInfo));
    memcpy(caPath, mqttpath, strlen(mqttpath));
    memcpy(caInfo, mqttpath, strlen(mqttpath));
    strcat(caPath, "ca.pem");
    strcat(caInfo, "ca.pem");
}

static void setCurlComopt(CURL *curlget)
{
    curl_easy_setopt(curlget, CURLOPT_SSL_VERIFYPEER, 1L); //验证证书
    curl_easy_setopt(curlget, CURLOPT_SSL_VERIFYHOST, 2L); //检查证书中的公用名是否存在，并且是否与提供的主机名匹配
    //curl_easy_setopt(curlget, CURLOPT_CAPATH,"/etc/ssl/certs/");
    setCurlCApath(mqttpath);
    //curl_easy_setopt(curlget, CURLOPT_CAPATH, caPath); //指定证书路径
    //curl_easy_setopt(curlget, CURLOPT_CAINFO, caInfo); //指定证书信息
    curl_easy_setopt(curlget, CURLOPT_VERBOSE, 1L);         //curl日志打印，0关闭，1打开

    /*Method 1. load the certificate in memory*/
    char *client_cert_buff = NULL;
    int client_cert_len = 0;
    char *client_private_key_buff = NULL;
    int client_private_key_len = 0;
    char *root_cert_buff = NULL;
    int root_cert_len = 0;
    struct curl_blob blob;

    if (get_pki_root_cert())
    {
        root_cert_len = strlen(get_pki_root_cert());
        root_cert_buff = malloc(root_cert_len);
        if (root_cert_buff)
        {
            memcpy(root_cert_buff, get_pki_root_cert(), root_cert_len);
        }
    }

    if (get_pki_client_cert())
    {
        client_cert_len = strlen(get_pki_client_cert());
        client_cert_buff = malloc(client_cert_len);
        if (client_cert_buff)
        {
            memcpy(client_cert_buff, get_pki_client_cert(), client_cert_len);
        }
    }

    if (get_pki_client_private_key())
    {
        client_private_key_len = strlen(get_pki_client_private_key());
        client_private_key_buff = malloc(client_private_key_len);
        if (client_private_key_buff)
        {
            memcpy(client_private_key_buff, get_pki_client_private_key(), client_private_key_len);
        }
    }

    if (root_cert_buff)
    {
        blob.data = root_cert_buff;
        blob.len = root_cert_len;
        blob.flags = CURL_BLOB_COPY;
        curl_easy_setopt(curlget, CURLOPT_CAINFO_BLOB, &blob);
        free(root_cert_buff);
        root_cert_buff = NULL;
    }

    if (client_cert_buff)
    {
        curl_easy_setopt(curlget, CURLOPT_SSLCERTTYPE, "PEM");
        blob.data = client_cert_buff;
        blob.len = client_cert_len;
        blob.flags = CURL_BLOB_COPY;
        curl_easy_setopt(curlget, CURLOPT_SSLCERT_BLOB, &blob);
        free(client_cert_buff);
        client_cert_buff = NULL;
    }

    if (client_private_key_buff)
    {
        curl_easy_setopt(curlget, CURLOPT_SSLKEYTYPE, "PEM");
        blob.data = client_private_key_buff;
        blob.len = client_private_key_len;
        blob.flags = CURL_BLOB_COPY;
        curl_easy_setopt(curlget, CURLOPT_SSLKEY_BLOB, &blob);
        free(client_private_key_buff);
        client_private_key_buff = NULL;
    }
    /*Method 1. end*/

    /*Method 2. load the certificate as a file*/
#if 0
    /* cert is stored PEM coded in file... */
    /* since PEM is default, we needn't set it for PEM */
    curl_easy_setopt(curlget, CURLOPT_SSLCERTTYPE, "PEM");

    /* set the cert for client authentication */
    curl_easy_setopt(curlget, CURLOPT_SSLCERT, get_pki_client_cert_file_path());

    //curl_easy_setopt(curlget, CURLOPT_KEYPASSWD, "password");

    /* if we use a key stored in a crypto engine,
    *          we must set the key type to "ENG" */
    curl_easy_setopt(curlget, CURLOPT_SSLKEYTYPE, "PEM");

    /* set the private key (file or ID in engine) */
    curl_easy_setopt(curlget, CURLOPT_SSLKEY, get_pki_client_private_key_file_path());

    /* set the file with the certs vaildating the server */
    curl_easy_setopt(curlget, CURLOPT_CAINFO, get_pki_root_cert_file_path());
#endif
    /*Method 2. end*/
}

static void setGetRequestopt(CURL *curlget)
{
    curl_easy_setopt(curlget, CURLOPT_DNS_CACHE_TIMEOUT, 20L);
    curl_easy_setopt(curlget, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curlget, CURLOPT_FORBID_REUSE, 1);    //默认是7200S
    curl_easy_setopt(curlget, CURLOPT_MAXCONNECTS, 2);     //默认是5
    curl_easy_setopt(curlget, CURLOPT_NOSIGNAL, 1);
}

static void setPostRequestopt(CURL *curlget)
{
    curl_easy_setopt(curlget, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curlget, CURLOPT_TIMEOUT, 10);         //接收数据时超时设置，如果10秒内数据未接收完，直接退出
    curl_easy_setopt(curlget, CURLOPT_CONNECTTIMEOUT, 10); //连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
    curl_easy_setopt(curlget, CURLOPT_FORBID_REUSE, 0); //默认是7200S
    curl_easy_setopt(curlget, CURLOPT_MAXCONNECTS, 20); //默认是5
    curl_easy_setopt(curlget, CURLOPT_NOSIGNAL, 1);
}

/**
 ******************************************************************************
 ** \简  述  启动get请求，返回一个相应的结构体指针，
 **  注  意  使用完毕需要调用释放内存API  freeRequestMemory(BaseResponse_t *p)
 ** \参  数  url:请求的完整的url; header:JSON字符串类型，需要解析
 ** \返回值  成功返回一个BaseResponse_t 类型的结构体指针，失败返回NULL
 ** \注  意   
 ** \作  者  
 ******************************************************************************/
BaseResponse_t *startGetRequest(char *url, char *header,void* __curl)
{
    CURL *curlget = NULL;
    BaseResponse_t *response = NULL;
    CURLcode res;
    char errbuf[CURL_ERROR_SIZE]={0};  //256
    char buf[1024]; //临时存放header字段
    struct MemoryStruct chunkwirte;
    struct MemoryStruct chunkheader;
    chunkwirte.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunkwirte.size = 0;            /* no data at this point */	
    memset(chunkwirte.memory,0,1);

    chunkheader.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunkheader.size = 0;            /* no data at this point */
    memset(chunkheader.memory,0,1);	
  
    curlget = curl_easy_init();
    if(!curlget)
    {
        free(chunkheader.memory);
        free(chunkwirte.memory);
        return NULL;
    }
   
    setGetRequestopt(curlget);
    setCurlComopt(curlget);
    curl_easy_setopt(curlget, CURLOPT_ERRORBUFFER, errbuf);//请求返回值不是CURLE_OK,则将错误码的信息存放到errbuf中
    curl_easy_setopt(curlget, CURLOPT_WRITEFUNCTION, writeFunctionCallback);
    curl_easy_setopt(curlget, CURLOPT_WRITEDATA,  (void *)&chunkwirte);
    curl_easy_setopt(curlget, CURLOPT_HEADERFUNCTION, headerFunctionCallback);
    curl_easy_setopt(curlget, CURLOPT_HEADERDATA,  (void *)&chunkheader);
    curl_easy_setopt(curlget, CURLOPT_URL,url);
    curl_easy_setopt(curlget, CURLOPT_CUSTOMREQUEST, "GET");

    struct curl_slist *list = NULL;
    cJSON *root = cJSON_Parse(header);
    if(root == NULL)
    {
        free(chunkheader.memory);
        free(chunkwirte.memory);
        curl_easy_cleanup(curlget);
        return NULL;
    }
    cJSON * c = root->child;
    while (c)
    {
        memset(buf,0,sizeof(buf));
        snprintf(buf,sizeof(buf),"%s:%s",c->string,c->valuestring);
        list = curl_slist_append(list,buf);
        c = c->next;
    }
    curl_easy_setopt(curlget, CURLOPT_HTTPHEADER,list);//set header
    errbuf[0]='\0';
    res = curl_easy_perform(curlget);
    if(res!=CURLE_OK)
        goto exit;
   
    response = (BaseResponse_t *)malloc(sizeof(BaseResponse_t));
    if(response == NULL)
        goto exit;
    response->status_code = res;
    response->networkError = false; //上层调用
    response->responseHeader = (char *)malloc(chunkheader.size);
    if(response->responseHeader == NULL)
    {
        free(response);
        goto exit;
    }
    response->responseHeaderlength = chunkheader.size;
    memset(response->responseHeader,0,chunkheader.size);
    memcpy(response->responseHeader,chunkheader.memory,chunkheader.size);
    if(res != CURLE_OK) 
    {
        size_t len = strlen(errbuf);
        if(len<=0)
            sprintf(errbuf, "%s", curl_easy_strerror(res));
        response->responseData = NULL;
        response->errorMsg =(char *)malloc(strlen(errbuf)+1);
        if(response->errorMsg == NULL)
        {
            free(response->responseHeader);
            free(response);
            goto exit;
        }
        memset(response->errorMsg,0,strlen(errbuf)+1);
        memcpy(response->errorMsg,errbuf,strlen(errbuf));
    }
    else
    {
        response->errorMsg = NULL;
        response->responseData = (char *)malloc(chunkwirte.size);
        if(response->responseData ==NULL)
        {
            free(response->responseHeader);
            free(response);
        }
        memset(response->responseData,0,chunkwirte.size);
	    response->responseDatalength = chunkwirte.size;
        memcpy(response->responseData,chunkwirte.memory,chunkwirte.size);       
    }
    free(chunkwirte.memory);
    free(chunkheader.memory);
    cJSON_Delete(root);
    curl_slist_free_all(list);
    curl_easy_cleanup(curlget);
    //printf("get message %s\n",response->responseData);
    return response;

exit:
    if(chunkwirte.memory)
        free(chunkwirte.memory);
    if(chunkheader.memory)
        free(chunkheader.memory);
    cJSON_Delete(root);
    curl_slist_free_all(list);
    curl_easy_cleanup(curlget);
    // printf("get rescode is %d  \n",res);
    char spdlog[64] = {0};
    snprintf(spdlog,64,"get rescode is %d ",res);
    log_v("networkmanager",spdlog);
    printf("%s\n",spdlog);
    return NULL;
}
/**
 ******************************************************************************
 ** \简  述  启动post请求，返回一个相应的结构体指针，
 **  注  意  使用完毕需要调用释放内存API  freeRequestMemory(BaseResponse_t *p)
 ** \参  数  url:请求的完整的url; header:JSON字符串类型，需要解析； requestBody:序列化并加密后的二进制数据
 ** \返回值  成功返回一个BaseResponse_t 类型的结构体指针，失败返回NULL
 ** \作  者  
 ******************************************************************************/
BaseResponse_t *startPostRequest(char *url, char *header,int lengthheader,char *requestBody,int bodylength,void* __curl)
{ 
    BaseResponse_t *response;
    CURLcode res;
    char errbuf[CURL_ERROR_SIZE]={0};  //256
    char buf[1024] = {0};              //临时存放header字段
    struct MemoryStruct chunkwirte;
    struct MemoryStruct chunkheader;
    chunkwirte.memory = malloc(1);    /* will be grown as needed by the realloc above */ 
    chunkwirte.size = 0;              /* no data at this point */	
    handle* _curl = __curl;
    if(_curl->handlethread == 1)
        pthread_mutex_lock(&gHandleProtect);
    
    chunkheader.memory = malloc(1);   /* will be grown as needed by the realloc above */ 
    chunkheader.size = 0;             /* no data at this point */
    if(_curl->curlpostvalid == 0){
        _curl->curlpost = curl_easy_init();
        if(!_curl->curlpost)
        {
          free(chunkheader.memory);
          free(chunkwirte.memory);
          if(_curl->handlethread == 1)
            pthread_mutex_unlock(&gHandleProtect);
	      return NULL;
        }
        _curl->curlpostvalid = 1;//valid
        setPostRequestopt(_curl->curlpost);
        setCurlComopt(_curl->curlpost);
    }
    curl_easy_setopt(_curl->curlpost, CURLOPT_WRITEFUNCTION, writeFunctionCallback);
    curl_easy_setopt(_curl->curlpost, CURLOPT_WRITEDATA,  (void *)&chunkwirte);
    curl_easy_setopt(_curl->curlpost, CURLOPT_HEADERFUNCTION, headerFunctionCallback);
    curl_easy_setopt(_curl->curlpost, CURLOPT_HEADERDATA,  (void *)&chunkheader);
    curl_easy_setopt(_curl->curlpost, CURLOPT_ERRORBUFFER, errbuf);//请求返回值不是CURLE_OK,则将错误码的信息存放到errbuf中
    curl_easy_setopt(_curl->curlpost, CURLOPT_URL,url);
    curl_easy_setopt(_curl->curlpost, CURLOPT_CUSTOMREQUEST, "POST"); 

    struct curl_slist *list = NULL;
    cJSON *root = cJSON_Parse(header);
    if(root == NULL)
    {
        free(chunkheader.memory);
        free(chunkwirte.memory);
        curl_easy_cleanup(_curl->curlpost);
        _curl->curlpost = NULL;
        _curl->curlpostvalid = 0;//invalid
        if(_curl->handlethread == 1)
            pthread_mutex_unlock(&gHandleProtect);
        return NULL;
    }
    cJSON * c = root->child;
    while (c)
    {
        memset(buf,0,sizeof(buf));
        snprintf(buf,sizeof(buf),"%s:%s",c->string,c->valuestring);
        list = curl_slist_append(list,buf);
        c = c->next;
    }
    curl_easy_setopt(_curl->curlpost,CURLOPT_HTTPHEADER,list);//set header
    if(bodylength == 0)
        curl_easy_setopt(_curl->curlpost, CURLOPT_POSTFIELDSIZE, strlen(requestBody)); //set body
    else
        curl_easy_setopt(_curl->curlpost, CURLOPT_POSTFIELDSIZE, bodylength);
    curl_easy_setopt(_curl->curlpost, CURLOPT_POSTFIELDS, requestBody);
    errbuf[0]='\0';
    res = curl_easy_perform(_curl->curlpost);
    if(res!=CURLE_OK){ 
        goto exit;
    }
   
    response = (BaseResponse_t *)malloc(sizeof(BaseResponse_t));
    if(response == NULL)
        goto exit;
    response->status_code = res;
    response->networkError = false;  //上层调用
    response->responseHeader = (char *)malloc(chunkheader.size + 1);
    response->responseHeaderlength = chunkheader.size;
    if(response->responseHeader == NULL)
    {
        free(response);
        goto exit;
    }
    memset(response->responseHeader,0,chunkheader.size);
    memcpy(response->responseHeader,chunkheader.memory,chunkheader.size);
    if(res != CURLE_OK) 
    {
        size_t len = strlen(errbuf);
        if(len<=0)
            sprintf(errbuf, "%s", curl_easy_strerror(res));
        response->responseData = NULL;
        response->errorMsg =(char *)malloc(strlen(errbuf)+1);
        if(response->errorMsg == NULL)
        {
            free(response->responseHeader);
            free(response);
            goto exit;
        }
        memset(response->errorMsg,0,strlen(errbuf)+1);
        memcpy(response->errorMsg,errbuf,strlen(errbuf));
    }
    else
    {
        response->errorMsg = NULL;
        response->responseData = (char *)malloc(chunkwirte.size);
        response->responseDatalength = chunkwirte.size;
        if(response->responseData ==NULL)
        {
            free(response->responseHeader);
            free(response);
	        goto exit;
        }
        memset(response->responseData,0,chunkwirte.size);
        memcpy(response->responseData,chunkwirte.memory,chunkwirte.size);   	    
    }
    free(chunkwirte.memory);
    free(chunkheader.memory);
    cJSON_Delete(root);
    curl_slist_free_all(list);
    if(_curl->handlethread == 1)
        pthread_mutex_unlock(&gHandleProtect);
    return response;
exit:
    if(chunkwirte.memory)
        free(chunkwirte.memory);
    if(chunkheader.memory)
        free(chunkheader.memory);
    cJSON_Delete(root);
    curl_slist_free_all(list);
    if(_curl->curlpostvalid == 1){
        if(_curl->curlpost != NULL)curl_easy_cleanup(_curl->curlpost);
        _curl->curlpost = NULL;
        _curl->curlpostvalid = 0;
    }
    if(_curl->handlethread == 1)
        pthread_mutex_unlock(&gHandleProtect);

    char spdlog[64] = {0};
    snprintf(spdlog,64,"get rescode is %d ",res);
    log_v("networkmanager",spdlog);
    printf("%s\n",spdlog);
    return NULL;
}
/**
 * @decla:free memory space
 * @datemodify:20200120
 */ 
void freeRequestMemory(BaseResponse_t *p)
{
    if(p != NULL){
        if(p->errorMsg != NULL)
		free(p->errorMsg);
        if(p->responseData != NULL)
		free(p->responseData);
        if(p->responseHeader != NULL)
		free(p->responseHeader);
        free(p);
    }
}
/**
 ******************************************************************************
 ** \简  述  发送文件专用接口
 **  注  意  返回指针不为NULL需要调用函数 freeResponseDataByDecyptMemory(parseResponseData_t *p) 释放内存空间
 ** \参  数 
 ** \返回值
 ** \作  者  
 ******************************************************************************/
BaseResponse_t *startPostfile(char* header,const char* remotepath, const char* localpath,const char* filename)
{
    char errbuf[CURL_ERROR_SIZE]={0};  //256
    char buf[1024] = {0};              //临时存放header字段
    CURLcode retcode = CURLE_GOT_NOTHING;
    BaseResponse_t*  response = NULL;
    struct stat file_info;
    struct MemoryStruct chunkwirte;
    struct MemoryStruct chunkheader;
    static CURL *curlhandle = NULL;
    chunkwirte.memory = malloc(1);  /* will be grown as needed by the realloc above */
    memset(chunkwirte.memory,0,1); 
    chunkwirte.size = 0;             /* no data at this point */	    
    chunkheader.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunkheader.size = 0;
    memset(chunkheader.memory,0,1);

    pthread_mutex_lock(&gHandleProtectfile);
    if(curlhandle == NULL){
        curlhandle = curl_easy_init();
    }
    curl_mime*     mime  = NULL;
    curl_mimepart* filed = NULL;
    mime  = curl_mime_init(curlhandle);
    filed = curl_mime_addpart(mime);
    curl_mime_name(filed,"sendfile");
    curl_mime_filedata(filed,localpath);

    filed = curl_mime_addpart(mime);
    curl_mime_name(filed,"file");
    curl_mime_filename(filed,filename);

    filed = curl_mime_addpart(mime);
    curl_mime_name(filed,"submit");
    curl_mime_data(filed,"send",CURL_ZERO_TERMINATED);
    //设置http 头部处理函数
    struct curl_slist *list = NULL;
    cJSON *root = cJSON_Parse(header);
    if(root == NULL)
    {
        if(chunkheader.memory != NULL){
            free(chunkheader.memory);
        }
        if(chunkwirte.memory != NULL){
            free(chunkwirte.memory);
        }
        if(curlhandle == NULL){
            curl_easy_cleanup(curlhandle);
        }
        curlhandle = NULL;
        curl_mime_free(mime);
        char spdlog[512] = {0};
        snprintf(spdlog,512,"upload file:%s header fail!!!", localpath);
        log_i("networkmanager",spdlog);
        pthread_mutex_unlock(&gHandleProtectfile);
        return NULL;
    }
    cJSON * cc = root->child;
    while (cc)
    {
        memset(buf,0,sizeof(buf));
        snprintf(buf,sizeof(buf),"%s:%s",cc->string,cc->valuestring);
        list = curl_slist_append(list,buf);
        cc = cc->next;
    }
    list = curl_slist_append(list, "Connection:Keep-Alive");
    list = curl_slist_append(list, "Charset:UTF-8");
    list = curl_slist_append(list, "Expect:");
    curl_easy_setopt(curlhandle, CURLOPT_HTTPHEADER,list);//set header
    curl_easy_setopt(curlhandle, CURLOPT_MIMEPOST,mime);
    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);

    // curl_easy_setopt(curlhandle, CURLOPT_CAINFO,mqttpath);//指定证书信息
    setCurlCApath(mqttpath);
    curl_easy_setopt(curlhandle, CURLOPT_CAPATH,caPath);
    curl_easy_setopt(curlhandle, CURLOPT_CAINFO,caInfo);

    curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, 0L); 
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, writeFunctionCallback);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA,  (void *)&chunkwirte);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION,headerFunctionCallback);
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA,  (void *)&chunkheader);
    curl_easy_setopt(curlhandle, CURLOPT_ERRORBUFFER, errbuf);//请求返回值不是CURLE_OK,则将错误码的信息存放到errbuf中
 
    retcode = curl_easy_perform(curlhandle);
    response = (BaseResponse_t *)malloc(sizeof(BaseResponse_t));
    memset(response,0,sizeof(BaseResponse_t));
    if(response == NULL){
        curl_mime_free(mime);
        if(chunkheader.memory != NULL)free(chunkheader.memory);
        if(chunkwirte.memory != NULL)free(chunkwirte.memory);
        if(root != NULL)cJSON_Delete(root);
        if(list != NULL)curl_slist_free_all(list);
        pthread_mutex_unlock(&gHandleProtectfile);
        return NULL;
    }

    response->status_code  = retcode;
    response->networkError = false;  //上层调用
    if (retcode == CURLE_OK){
        response->responseHeader = (char *)malloc(chunkheader.size + 1);
        response->responseHeaderlength = chunkheader.size;
        if(chunkheader.size > 1){
            memset(response->responseHeader,0,chunkheader.size);
            memcpy(response->responseHeader,chunkheader.memory,chunkheader.size);
        }
        response->responseData = (char *)malloc(chunkwirte.size + 1);
        response->responseDatalength = chunkwirte.size;
        if(chunkwirte.size > 0){
            memset(response->responseData,0,chunkwirte.size);
            memcpy(response->responseData,chunkwirte.memory,chunkwirte.size); 
        }
        char spdlog[512] = {0};
        snprintf(spdlog,512,"upload file:%s sucess,header is %s body is: %s!!!", localpath,chunkheader.memory,chunkwirte.memory);
        log_v("networkmanager",spdlog);
        curl_mime_free(mime);
        if(chunkheader.memory != NULL){
            free(chunkheader.memory);
        }
        if(chunkwirte.memory != NULL){
            free(chunkwirte.memory);
        }
        if(root != NULL){
            cJSON_Delete(root);
        }
        if(list != NULL){
            curl_slist_free_all(list);
        }
        pthread_mutex_unlock(&gHandleProtectfile);
	    return response;
    }
    else {
	    curl_mime_free(mime);
        log_i("networkmanager","upload file error\n"); 
        size_t len = strlen(errbuf);
        if(len > 0){
            sprintf(errbuf, "%s", curl_easy_strerror(retcode));
        }
        response->responseData = NULL;
        response->errorMsg =(char *)malloc(strlen(errbuf)+1);
        memset(response->errorMsg,0,strlen(errbuf)+1);
        memcpy(response->errorMsg,errbuf,strlen(errbuf));

        if(chunkheader.memory != NULL)free(chunkheader.memory);
        if(chunkwirte.memory != NULL)free(chunkwirte.memory);
        if(root != NULL)cJSON_Delete(root);
        if(list != NULL)curl_slist_free_all(list);
        if(curlhandle != NULL){
            curl_easy_cleanup(curlhandle);
            curlhandle = NULL;
        }
        pthread_mutex_unlock(&gHandleProtectfile);
		return response;
	}
}
/**
 *  ******************************************************************************
 *   ** \简  述  多线程的基本初始化，防止出现异常
 *   ** \作  者  
 *  ******************************************************************************/
void curl_init(){
	//curl_global_init(CURL_GLOBAL_ALL);
	curl_global_init(CURL_GLOBAL_SSL);
}