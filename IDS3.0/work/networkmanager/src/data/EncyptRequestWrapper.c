#include "EncyptRequestWrapper.h"
#include "DispatchManager.h"
#include "AesManager.h"
#include "cryptogram.h"
#include "util/postHeaderUtil.h"
/**
 * 数据加密
 * 需要对返回的整个结构体进行释放  无需对单个变量进行释放
 */ 
EncyptRequestWrapper_t *EncyptRequestWrapper(requestWrapper_t *requestWrapper,void* _curl) {
    cypher_t* cypher = encyptDataByAesAndSessionKey( requestWrapper ->data,requestWrapper->datalen,_curl);
    if(cypher){
        EncyptRequestWrapper_t *encyptRequestWrapperdata =  (EncyptRequestWrapper_t *)malloc(sizeof(EncyptRequestWrapper_t));
        if(encyptRequestWrapperdata == NULL)
             return NULL;
        encyptRequestWrapperdata->url = (char *)malloc(strlen(requestWrapper->url) + 2);    
        memset(encyptRequestWrapperdata->url,0,strlen(requestWrapper->url) + 2);
        memcpy(encyptRequestWrapperdata->url,requestWrapper ->url,strlen(requestWrapper->url));
        //encyptRequestWrapperdata->url = requestWrapper ->url;
        encyptRequestWrapperdata->dataLen = cypher->len_data;
	    encyptRequestWrapperdata->data = (char*)malloc(cypher->len_data + 2);
	    memset(encyptRequestWrapperdata->data ,0,cypher->len_data + 2);
	    memcpy(encyptRequestWrapperdata->data,cypher->data,cypher->len_data);
        if(strstr(encyptRequestWrapperdata->url,"hardware_data") == NULL)
        {
            encyptRequestWrapperdata->headers = getHeaders((char*)cypher->data, cypher->len_data,_curl);//header 头
        }
        else
        {
            encyptRequestWrapperdata->headers = getHeadersForHardware((char*)cypher->data, cypher->len_data,_curl);//header 头
        }
        free(cypher);
        return encyptRequestWrapperdata;
    }
    return NULL;
}
