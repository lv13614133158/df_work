
#include "DispatchManager.h"
#include "Tool.h"
#include "mysqlite.h"
#include "data/dataToProtobufUtils.h"
#include "ThreadPool.h"
#include "ConfigConstData.h"
#include "data/EncyptRequestWrapper.h"
#include "AesManager.h"
#include "FileWrapper.h"
#include "httpResponseData.h"
#include "mysqlite.h"
#include "base64.h"
#include "CJsonUtils.h"
#include <stdlib.h>
#include "util/postHeaderUtil.h"

#define EVENT_REQUEST_URL      "/api/v1/incidents"
#define HEARTBEAT_REQUEST_URL  "/api/v1/heart_beat" 
//free space requestWrapper_t
#define freerequestWrapper_t(data){\
        if(data != NULL){\
	    if(data->data != NULL)free(data->data);\
	    if(data->headers != NULL)free(data->headers);\
	    if(data->url != NULL)free(data->url);\
	    if(data->eventWrapper != NULL){\
		             if(data->eventWrapper->attachment_id != NULL)free(data->eventWrapper->attachment_id);\
		             if(data->eventWrapper->data != NULL)free(data->eventWrapper->data);\
		             if(data->eventWrapper->error_msg != NULL)free(data->eventWrapper->error_msg);\
		             if(data->eventWrapper->path != NULL)free(data->eventWrapper->path);\
		             if(data->eventWrapper->policy_id != NULL)free(data->eventWrapper->policy_id);\
		             if(data->eventWrapper->ticket_id != NULL)free(data->eventWrapper->ticket_id);\
		             free(data->eventWrapper);\
		        }\
	    free(data);}\
}
#define freeEncyptRequestWrapper_t(encyptRequestWrapperdata){\
if(encyptRequestWrapperdata != NULL ){\
if(encyptRequestWrapperdata->data != NULL)free(encyptRequestWrapperdata->data);\
if(encyptRequestWrapperdata->headers != NULL)free(encyptRequestWrapperdata->headers);\
if(encyptRequestWrapperdata->url != NULL)free(encyptRequestWrapperdata->url);\
free(encyptRequestWrapperdata); \
}}

//free local space(blind produceAllRequestFromDb function) 
#define freeproduceAllRequestFromDbsql(event_policy_id,event_data,event_path,event_ticket_id){\
    if(event_policy_id != NULL)free(event_policy_id);\
    if(event_data      != NULL)free(event_data);\
    if(event_path      != NULL)free(event_path);\
    if(event_ticket_id != NULL)free(event_ticket_id);\
}
/**
 ******************************************************************************
 ** \简  述   消息进入发送队列，status是1（既正要发送）；未成功进入发送队列是0；在正常进入发送队列的
              但是发送失败的，状态更新为0；此处存在的问题是，突然断电等问题时，由于status1为正在发送，
              那么readsqlite，错误认为时在正常发送队列里的数据；此function的作用是，每次重新启动的时候，更新
              原有sqlite table，将1变成0
 **  注  意   此函数IDPS启动只能调用一次
 ** \参  数   void
 ** \返回值   void
 ** \时 间    2020年7月3日
 ******************************************************************************/
void Restart_UpdateTable(void){
    int columnCount = -1;
    char sql[128] = {0};
    long long id       =   0;
    sqlite3_stmt *stmt = NULL;
    if ((stmt = query("SELECT * FROM base_upload_queue WHERE status IS NULL OR status == 1")) == NULL){
       // sqliteUnLock();
        return;
    }
    while(next(stmt))
    {
        id = getLong(stmt,getColumnIndex(stmt,"id"));
        memset(sql,0,128);
        sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, id);
        sqlitenolockpro(sql); 
    }
    finalize(stmt);
}
/**
 ******************************************************************************
 ** \简  述   定期发送发送失败的事件
 **  注  意   根据IDPS的设置间隔调用
 ** \参  数   void*（无意义）
 ** \返回值   void
 ** \时 间    2020年7月3日
 ******************************************************************************/
void produceAllRequestFromDb(void *nullarg,void *curl){
	int columnCount = -1;
    char sql[128] = {0};
    sqlite3_stmt *stmt = NULL;
    if ((stmt = query("SELECT * FROM base_upload_queue WHERE status IS NULL OR status != 1")) == NULL){
        sqliteUnLock();
        return;
    }
    while(next(stmt))
    {
        requestWrapper_t *data = (requestWrapper_t *)malloc(sizeof(requestWrapper_t));
        if(data == NULL) 
            goto exit;
        memset(data,0,sizeof(requestWrapper_t));
        data->id = getLong(stmt,getColumnIndex(stmt,"id"));
        data->index = getInt(stmt,getColumnIndex(stmt,"priority"));
        const char* url = getString(stmt,getColumnIndex(stmt,"url"));
        if(url == NULL){
            free(data);
            continue;
        }
        if((data->url = malloc(strlen(url)+1)) == NULL) {
            free(data);
            goto exit;
        }
        memset(data->url,0,strlen(url)+1);
        sprintf(data->url,"%s",url);
        if (getInt(stmt,getColumnIndex(stmt,"with_attachment")) <= 0) {
            const char* datasql = getString(stmt,getColumnIndex(stmt,"data"));
            int dataLen = getInt(stmt,getColumnIndex(stmt,"data_len"));
            size_t data_length;
            char* decode64_data = (char*)base64_decode((unsigned char*)datasql, dataLen, &data_length);
            if((data->data = malloc(data_length+2)) == NULL)
            {
                free(data->url);
                free(decode64_data);
                free(data);
                goto exit;
            }
            memset(data->data,0,data_length+2);
	        memcpy(data->data,decode64_data,data_length);
	        data->datalen = data_length;
            free(decode64_data);
            data->eventWrapper=NULL;
            data->withAttachment=false;
        }else{
            const char* event_policy_id = getString(stmt,getColumnIndex(stmt,"event_policy_id"));
            const char* event_data = getString(stmt,getColumnIndex(stmt,"event_data"));
            const char* event_path = getString(stmt,getColumnIndex(stmt,"event_path"));
            const char* event_ticket_id = getString(stmt,getColumnIndex(stmt,"event_ticket_data"));
            data->withAttachment=true;
            if((data->eventWrapper = (eventWrapper_t*)malloc(sizeof(eventWrapper_t))) == NULL){
                free(data->url);
                free(data);
                goto exit;
            }
            memset(data->eventWrapper,0,sizeof(eventWrapper_t));
            if((data->eventWrapper->policy_id = (char*)malloc(strlen(event_policy_id))) == NULL){
                free(data->eventWrapper);
                free(data->url);
                free(data);
                goto exit;
            }
            sprintf(data->eventWrapper->policy_id,"%s",event_policy_id);
            if((data->eventWrapper->data = (char*)malloc(strlen(event_data))) == NULL){
                free(data->eventWrapper->policy_id);
                free(data->eventWrapper);
                free(data->url);
                free(data);
                goto exit;
            }
            sprintf(data->eventWrapper->data,"%s",event_data);
            if((data->eventWrapper->path = (char*)malloc(strlen(event_path))) == NULL){
                free(data->eventWrapper->data);
                free(data->eventWrapper->policy_id);
                free(data->eventWrapper);
                free(data->url);
                free(data);
                goto exit;
            }
            sprintf(data->eventWrapper->path,"%s",event_path);
            if((data->eventWrapper->ticket_id = (char*)malloc(strlen((event_ticket_id ? event_ticket_id : "")))) == NULL){
                free(data->eventWrapper->path);
                free(data->eventWrapper->data);
                free(data->eventWrapper->policy_id);
                free(data->eventWrapper);
                free(data->url);
                free(data);
                goto exit;
            }
            sprintf(data->eventWrapper->ticket_id,"%s",(event_ticket_id ? event_ticket_id : ""));
        }
        data->headers=NULL;
       if(true == threadpool_add(producerPtr,consumeRequest,(void *)data)){
           memset(sql,0,128);
           sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 1, data->id);
           sqlitenolockpro(sql);
        }
        else{//insert fail
            freerequestWrapper_t(data);
        }
    }
exit:
    finalize(stmt);
}
/**
 * date:20200119 17:18
 */ 
void produceEventDataRequestAfterUploadAttachment(const char *policy_id, const char *datacontent, const char *ticket_id, const char* attachment_id, const char* error_msg, int priority,void* _curl){
    int datalen = 0;
    requestWrapper_t *data = (requestWrapper_t *)malloc(sizeof(requestWrapper_t));
    if(data == NULL)return ;
    memset(data,0,sizeof(requestWrapper_t));

    uint16_t l_urllen = strlen(getBaseUrl()) + strlen(EVENT_REQUEST_URL);
    char cUrl[l_urllen + 2];
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,EVENT_REQUEST_URL);

    data->url = malloc(strlen(cUrl)+1);
    if(data->url == NULL)
    {
        free(data);
        return;
    }
    memset(data->url,0,strlen(cUrl)+1);
    strncpy(data->url,cUrl,strlen(cUrl));

    char* bodyString = getAlarmEventProtobufFromGaterData(policy_id, datacontent, ticket_id, error_msg, attachment_id,&datalen,_curl);
    if (bodyString == NULL) {
        free(data->url);
        free(data);
        return;
    }
    data->data = malloc(datalen+1);
    if(data->data == NULL)
    {
        free(bodyString);
        free(data->url);
        free(data);
        return;
    }
    memset(data->data,0,datalen+1);
    strncpy(data->data,bodyString,datalen);
    free(bodyString);
    data->index = priority;
    data->eventWrapper=NULL;
    data->headers=NULL;
    data->datalen = datalen;
    data->withAttachment=false;
    long long id = 0;
    if(id > 0)data->id = id;
    if(false == threadpool_add(producerPtr,consumeRequest,(void *)data)){
        char sql[128] = {0};
        sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, data->id);
        update(sql);
        freerequestWrapper_t(data);
    }
}
void produceGatherDataRequest(const char *url, const char *body, int priority,void* _curl)
{
    char *bodyString = NULL;
    uint32_t retlen = 0;
    uint16_t l_urllen = strlen(getBaseUrl()) + strlen(url);
    char cUrl[l_urllen + 2];
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,url);
    if(strstr(url, "hardware_data") == NULL)
    {
        bodyString = getProtobufFromGaterData(cUrl, body,&retlen,_curl);
    }
    else
    {
        bodyString = (char*)malloc(strlen(body) + 1);
        memset(bodyString,0,strlen(body) + 1);
        memcpy(bodyString,body,strlen(body) + 1);
    }
    if(bodyString == NULL)
        return;
    requestWrapper_t *data = (requestWrapper_t *)malloc(sizeof(requestWrapper_t));/////
    if(data == NULL)
    {
        free(bodyString);
        return ;
    }
    memset(data,0,sizeof(requestWrapper_t));
    data->index = priority;
    data->url = malloc(strlen(cUrl)+1);/////
    if(data->url == NULL)
    {
        free(data);
        free(bodyString);
        return;
    }
    memset(data->url,0,strlen(cUrl)+1);
    strncpy(data->url,cUrl,strlen(cUrl));
    retlen = (retlen == 0)?(strlen(bodyString)):(retlen);    
    data->data = (char*)malloc(retlen + 1);/////
    if(data->data == NULL)
    {
        free(bodyString);
        free(data->url);
        free(data);
        return;
    }
    memset(data->data,0,retlen + 1);
    memcpy(data->data,bodyString,retlen);
    data->datalen = retlen;
    free(bodyString);
    data->eventWrapper=NULL;
    data->headers=NULL;
    data->withAttachment=false;
    long long id = 0;//status 1
    if(id > 0)data->id = id;
    if(threadpool_add(producerPtr,consumeRequest,(void *)data) == false){//
        char sql[128] = {0};
        sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, data->id);
        update(sql);
        freerequestWrapper_t(data);
    }
}

void produceEventDataRequest(const char *policy_id, const char *datacontent, const char *ticket_id, int priority,void* _curl){
    int retlen = 0;
    uint16_t l_urllen = strlen(getBaseUrl()) + strlen(EVENT_REQUEST_URL);
    char cUrl[l_urllen + 2];
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,EVENT_REQUEST_URL);
    int8_t* bodyString = getAlarmEventProtobufFromGaterData(policy_id, datacontent, ticket_id, NULL, NULL,&retlen,_curl);
    if(bodyString == NULL){
        return;
    }
    if (strlen(bodyString) == 0) {
        free(bodyString);
        return;
    }
    requestWrapper_t *data = (requestWrapper_t *)malloc(sizeof(requestWrapper_t));
    if(data == NULL)
    {
        free(bodyString);
        return ;
    }
    memset(data,0,sizeof(requestWrapper_t));
    data->index = priority;
    data->url = malloc(strlen(cUrl)+1);
    if(data->url == NULL)
    {
        free(data);
        free(bodyString);
        return;
    }
    memset(data->url,0,strlen(cUrl)+1);
    strncpy(data->url,cUrl,strlen(cUrl));
   
    data->data = malloc(retlen+1);
    if(data->data == NULL)
    {
        free(bodyString);
        free(data->url);
        free(data);
        return;
    }
    memset(data->data,0,retlen+1);
    memcpy(data->data,bodyString,retlen);
    free(bodyString);
    data->datalen = retlen;
    data->eventWrapper=NULL;
    data->headers=NULL;
    data->withAttachment=false;
    long long id = 0;
    if(id > 0)data->id = id;
    if(false == threadpool_add(producerPtr,consumeRequest,(void *)data)){
        char sql[128] = {0};
        sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, data->id);
        update(sql);
        freerequestWrapper_t(data);
    }
}
void produceEventDataRequestWithAttachment(const char *policy_id, const char *datacontent, const char *ticket_id, const char *path, int priority){

    uint16_t l_urllen = strlen(getBaseUrl()) + strlen("/api/v1/incidents");
    char cUrl[l_urllen + 2];
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,"/api/v1/incidents");

    requestWrapper_t *data = (requestWrapper_t *)malloc(sizeof(requestWrapper_t));
    if(data == NULL)
        return ;
    memset(data,0,sizeof(requestWrapper_t));
    data->index = priority;
    data->url = malloc(strlen(cUrl)+1);
    if(data->url == NULL)
    {
        free(data);
        return;
    }
    memset(data->url,0,strlen(cUrl)+1);
    strncpy(data->url,cUrl,strlen(cUrl));

    data->withAttachment=true;
    data->eventWrapper = (eventWrapper_t*)malloc(sizeof(eventWrapper_t));
    if(data->eventWrapper == NULL){
        free(data->url);
        free(data);
        return;
    }
    memset(data->eventWrapper,0,sizeof(eventWrapper_t));
    data->eventWrapper->policy_id = (char*)malloc(strlen(policy_id) + 1);
    if(data->eventWrapper->policy_id == NULL){
        free(data->eventWrapper);
        free(data->url);
        free(data);
        return;
    }
    memset(data->eventWrapper->policy_id,0,strlen(policy_id)+1);
    strncpy(data->eventWrapper->policy_id,policy_id,strlen(policy_id));
    if(datacontent != NULL){
        data->eventWrapper->data = (char*)malloc(strlen(datacontent)+1);
        if(data->eventWrapper->data == NULL){
            free(data->eventWrapper->policy_id);
            free(data->eventWrapper);
            free(data->url);
            free(data);
            return;
        }
        memset(data->eventWrapper->data,0,strlen(datacontent)+1);
        strncpy(data->eventWrapper->data,datacontent,strlen(datacontent));
    }
    data->eventWrapper->path = (char*)malloc(strlen(path)+1);
    if(data->eventWrapper->path == NULL){
        free(data->eventWrapper->data);
        free(data->eventWrapper->policy_id);
        free(data->eventWrapper);
        free(data->url);
        free(data);
        return;
    }
    memset(data->eventWrapper->path,0,strlen(path)+1);
    strncpy(data->eventWrapper->path,path,strlen(path));
    data->eventWrapper->ticket_id = (char*)malloc(strlen((ticket_id ? ticket_id : ""))+1);
    if(data->eventWrapper->ticket_id == NULL){
        free(data->eventWrapper->path);
        free(data->eventWrapper->data);
        free(data->eventWrapper->policy_id);
        free(data->eventWrapper);
        free(data->url);
        free(data);
        return;
    }
    memset(data->eventWrapper->ticket_id,0,strlen((ticket_id ? ticket_id : ""))+1);
    strncpy(data->eventWrapper->ticket_id,(ticket_id ? ticket_id : ""),strlen((ticket_id ? ticket_id : "")));
    data->headers=NULL;
     
    long long id = 0;
    if(id > 0)data->id = id;
    
    if(false == threadpool_add(producerPtr,consumeRequest,(void *)data)){
         
        char sql[128] = {0};
        sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, data->id);
        update(sql);
        
        freerequestWrapper_t(data);
    } 
}

/**
 * date:20200119 17:21
 */ 
void produceHeartbeatRequest(const char *url,void* _curl){
    uint16_t l_urllen = strlen(getBaseUrl()) + strlen(((url != NULL)&&(strlen(url) > 0))?(url):(HEARTBEAT_REQUEST_URL));
    char cUrl[l_urllen + 2];
    char sql[128] = {0};
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,((url != NULL)&&(strlen(url) > 0))?(url):(HEARTBEAT_REQUEST_URL));
 
    char* bodyString = getProtobufForHeartbeat(_curl);
    if(bodyString == NULL)
        return;
    if (strlen(bodyString) == 0) {
        free(bodyString);
        return;
    }
    requestWrapper_t *data = (requestWrapper_t *)malloc(sizeof(requestWrapper_t));
    if(data == NULL)
    {
        free(bodyString);
        return ;
    }
    memset(data,0,sizeof(requestWrapper_t));
    data->index = 100;
    data->url = malloc(strlen(cUrl)+1);
    if(data->url == NULL)
    {
        free(data);
        free(bodyString);
        return;
    }
    memset(data->url,0,strlen(cUrl)+1);
    strncpy(data->url,cUrl,strlen(cUrl));

    data->data = malloc(strlen(bodyString)+1);
    if(data->data == NULL)
    {
        free(bodyString);
        free(data->url);
        free(data);
        return;
    }
    memset(data->data,0,strlen(bodyString)+1);
    memcpy(data->data,bodyString,strlen(bodyString));
    free(bodyString);
    data->eventWrapper=NULL;
    data->headers=NULL;
    data->withAttachment=false;
    long long id = 0;
    if(id > 0)data->id = id;
    if(false == threadpool_add(producerPtr,consumeRequest,(void *)data)){
        memset(sql,0,128);
        sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, data->id);
        update(sql);
        freerequestWrapper_t(data);
    }
}
void consumeToUploadFile(requestWrapper_t *datacontent,void* _curl){
    //header
    char* header = getFileHeaders(_curl);
    if(header == NULL)
        return;
    uint16_t l_urllen = strlen(getBaseUrl()) + strlen("/api/v1/file/upload");
    char cUrl[l_urllen + 2];
    memset(cUrl,0,l_urllen + 2);
    strncpy(cUrl,getBaseUrl(),strlen(getBaseUrl()));
    strcat(cUrl,"/api/v1/file/upload");
    char localpath[256] = {0},fn[128] = {0};
    char* p = NULL;
    memcpy(localpath,datacontent->eventWrapper->path,strlen(datacontent->eventWrapper->path));
    strcpy(fn,(p=strrchr(localpath,'/')) ? p+1 : localpath);
    BaseResponse_t *httpResponseData = startPostfile(header,cUrl,datacontent->eventWrapper->path,fn);
    free(header);
    if(httpResponseData->networkError){//上传网络错误
        char sql[256] = {0};
        snprintf(sql,127, "UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, datacontent->id);
        update(sql);
        freeRequestMemory(httpResponseData);
        return;
    }
    char sql[128] = {0};
    sprintf(sql,"DELETE FROM base_upload_queue WHERE id = %lld", datacontent->id);
    del(sql); 
    if( httpResponseData->status_code == 0) {//上传文件成功
        //char* attachment_id = getinfofromCJsonstr(httpResponseData->responseData,"file_id");   
        char attachment_id[128] = {0};
        sscanf(httpResponseData->responseData,"*{\"file_id\":\"%s\"}",attachment_id);
        if(attachment_id != NULL){
     	 if (strlen(attachment_id) == 0) {
            produceEventDataRequestAfterUploadAttachment( datacontent->eventWrapper->policy_id,datacontent->eventWrapper->data, datacontent->eventWrapper->ticket_id,attachment_id, "", datacontent->index,_curl);

         }else{
            produceEventDataRequestAfterUploadAttachment( datacontent->eventWrapper->policy_id,datacontent->eventWrapper->data, datacontent->eventWrapper->ticket_id,"", "file_id 为空", datacontent->index,_curl);
         }
	    }
        freeRequestMemory(httpResponseData);
        return;
    }
    produceEventDataRequestAfterUploadAttachment( datacontent->eventWrapper->policy_id,datacontent->eventWrapper->data, datacontent->eventWrapper->ticket_id,"",httpResponseData->errorMsg, datacontent->index,_curl);
    freeRequestMemory(httpResponseData);
}

void consumeRequest(void *pdata,void *curl)
{
    requestWrapper_t *data = (requestWrapper_t *)pdata;
    char sql[128] = {0};
    if(data->withAttachment==true) //有附加文件
    {
        consumeToUploadFile(data,curl);
    }
    else
    {
        EncyptRequestWrapper_t *encyptRequestWrapperdata = EncyptRequestWrapper(data,curl);
        if(encyptRequestWrapperdata == NULL){
            freerequestWrapper_t(data);
            return;
        }
        if ((NULL == encyptRequestWrapperdata->url)||(NULL == encyptRequestWrapperdata->data)||(NULL == encyptRequestWrapperdata->headers)){ 
            freeEncyptRequestWrapper_t(encyptRequestWrapperdata);
            freerequestWrapper_t(data);
            return;
        }
        parseResponseData_t *httpResponseData = startPost(encyptRequestWrapperdata->url,encyptRequestWrapperdata->headers,0,encyptRequestWrapperdata->data,encyptRequestWrapperdata->dataLen,curl);
        if(httpResponseData){
            if(httpResponseData->status_code == 0){//删除数据库
	    	    sprintf(sql,"DELETE FROM base_upload_queue WHERE id = %lld", data->id);
                del(sql);
            }
            else{//更新request的状态，后续定时到后再取出放到队列重新发起网络请求
                sprintf(sql,"UPDATE base_upload_queue SET status = %d WHERE id = %lld", 0, data->id);
                update(sql);
            }
            freeResponseDataByDecyptMemory(httpResponseData);
        }
        freeEncyptRequestWrapper_t(encyptRequestWrapperdata);
    }
    freerequestWrapper_t(data);
}
