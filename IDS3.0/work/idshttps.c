/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-06-16 22:14:32
 */ 
#include <stdio.h>
#include <time.h>
#include "idshttps.h"

handle *http_handle;

static void idsgetgetManageKeyandSn();
static void getsession_keyandtoken();


int networkFunctionEnabled(void)
{
    return 1;

}
int idshttps_init(idsnetworkManagerModult_t *_idsnetworkManagerModule)
{
    idsnetworkManagerModule= *_idsnetworkManagerModule;
    idsgetgetManageKeyandSn();
    getsession_keyandtoken();
    printf("idshttps_init \n manageKey %s \n sN  %s\n session_key %s\n token  %s\n",\
    idsnetworkManagerModule.manageKey,idsnetworkManagerModule.sN,\
    idsnetworkManagerModule.session_key,idsnetworkManagerModule.token);
    getheartbeat();
}


static void idsgetgetManageKeyandSn()
{
    char cUrl[128];
    memset(cUrl,0,128);

    strncpy(cUrl,idsnetworkManagerModule.url,128);
    strcat(cUrl,GET_MANAGE_KEY_URL);
    char *body = NULL;
    char buff[256] = {0};
    snprintf(buff,256,"{\"channel_id\":\"%s\",\"udid\":\"%s\",\"equipment_type\":\"%s\",\"os_type\":\"linux\"}", \
        idsnetworkManagerModule.channelId, \ 
        idsnetworkManagerModule.vin,        \ 
        idsnetworkManagerModule.equipmentType);
    size_t body_len = strlen(buff) + 1;
    // 动态分配内存
    body = (char*)malloc(body_len);
    if (body == NULL) {
        return;
    }
    strcpy(body,buff);
   
    if(body==NULL)
    {
        return;
    }
    char *header = NULL;
    header = getManageKeyHeaders();
    if(header == NULL)
    {
        return;
    }
    BaseResponse_t *response=startPostRequest(cUrl,header,0,body,0,&http_handle);
    cJSON *json_obj = cJSON_Parse(response->responseData);
    if (json_obj == NULL) {
        return ;
    }
    cJSON *data = cJSON_GetObjectItem(json_obj,"data");    
    cJSON *register_key_item = cJSON_GetObjectItem(data,"register_key");

    if (register_key_item != NULL && cJSON_IsString(register_key_item)) {
   
        size_t data_length;
        char* decode64_char = (char*)base64_decode((unsigned char*)register_key_item->valuestring,\
        strlen(register_key_item->valuestring), &data_length);
        idsnetworkManagerModule.manageKey = decode64_char;

    } else {
        printf("Register key not found or not a string\n");
    }
    cJSON *sn_item = cJSON_GetObjectItem(data,"sn");
    if (sn_item != NULL && cJSON_IsString(sn_item)) {
        char * sn= (char *)malloc(strlen(sn_item->valuestring) + 1);
        if (sn == NULL) {
            return ;
        }
        strcpy(sn, sn_item->valuestring);
        idsnetworkManagerModule.sN = sn;
    } else {
        printf("SN not found or not a string\n");
    }
    cJSON_Delete(json_obj);
    return;
}

static void getsession_keyandtoken()
{
    char cUrl[128];
    BaseResponse_t * response = NULL;
    
    // 安全地构建URL
    memset(cUrl, 0, sizeof(cUrl));
    strncpy(cUrl, idsnetworkManagerModule.url, sizeof(cUrl) - strlen(GET_SESSION_KEY_URL) - 1);
    strncat(cUrl, GET_SESSION_KEY_URL, sizeof(cUrl) - strlen(cUrl) - 1);
    
    char *body = getSessionKeyBoys(&http_handle);
    if(body == NULL) {
        return;
    }
    // 加密准备
    char iv[KEY_IV_LENGTH+1]  = {0};
    char key[KEY_IV_LENGTH+1] = {0};
    char *manageKey = idsnetworkManagerModule.manageKey;
    if(manageKey == NULL) {
        return;
    }
    // 验证manageKey长度
    if(strlen(manageKey) < 32) {
        printf("Error: manageKey too short\n");
        return;
    }
    
    memcpy(key, manageKey, 16);
    key[16] = '\0';    
    memcpy(iv, manageKey + 16, 16);
    iv[16] = '\0';
    
    size_t body_len = strlen(body);
    // 更安全的内存分配
    cypher_t *plain = (cypher_t *) malloc(sizeof(int) + body_len + 16);
    if (!plain) {
        return;
    }
    plain->len_data = body_len;
    memset(plain->data, 0, body_len + 16);
    if (body_len > 0) {
        memcpy(plain->data, body, body_len);
    }
    cypher_t *cypher = aes_cbc_encrypt(plain, key, iv);
    free(plain);
    
    if (!cypher) {
        return;
    }
    // 签名处理
    unsigned char hmac[32] = {0}; 

    hmac_md5((unsigned char *) key, 16,  \
             (unsigned char *) cypher->data, cypher->len_data, hmac);
    size_t data_length_hmac = 0;
    char *base64_hmac = (char *) base64_encode((unsigned char *) hmac, 16, &data_length_hmac);
    if (!base64_hmac) {
        free(cypher);
        return;
    }
    // 计算并分配header内存
    size_t channelIdLen = strlen(idsnetworkManagerModule.channelId);
    size_t sNLen = strlen(idsnetworkManagerModule.sN);
    size_t hmacLen = strlen(base64_hmac);
    
    // 更精确的长度计算
    int l_headerlen = 128 + channelIdLen + sNLen + hmacLen;
    char *header = (char*)malloc(l_headerlen);
    if(header == NULL) {
        free(base64_hmac);
        free(cypher);
        return;
    }
    
    memset(header, 0, l_headerlen);
    int written = snprintf(header, l_headerlen-1, 
                          "{\"X-Channel-Id\":\"%s\",\"X-Crypt\":\"1\",\"X-Proto-Buf\":\"0\","
                          "\"X-Sn\":\"%s\",\"X-HMAC\":\"%s\",\"Authorization\":\"\"}",
                          idsnetworkManagerModule.channelId, 
                          idsnetworkManagerModule.sN, 
                          base64_hmac);

    free(base64_hmac);
    
    if(written >= l_headerlen-1) {
        printf("Warning: header buffer may be too small\n");
    }
    response = startPostRequest(cUrl, header, 0, cypher->data, cypher->len_data, &http_handle);
    free(header);
    free(cypher);
    
    if(response == NULL) {
        return;
    }
    if (!response->responseData) {
        free(response);
        return;
    }

    size_t resp_data_len =response->responseDatalength;
    // 使用更保守的内存分配策略
    plain = (cypher_t*)calloc(1, sizeof(cypher_t) + resp_data_len + 16);
    if (!plain) {

        printf("Error: memory allocation failed\n");
        free(response);
        return;
    }

    plain->len_data = resp_data_len;
    if (resp_data_len > 0) {
        memcpy(plain->data, response->responseData, resp_data_len);
    }
    plain->data[resp_data_len] = '\0';

    cypher = aes_cbc_decrypt(plain, key, iv);
    free(plain);
    
    if (!cypher) {
        printf("Error: decryption failed\n");
        free(response);
        return;
    }
    cJSON *json_obj = cJSON_Parse(cypher->data);
    if (json_obj == NULL) {
        return ;
    }
    cJSON *data = cJSON_GetObjectItem(json_obj,"data");    
    cJSON *session_key_item = cJSON_GetObjectItem(data,"session_key");

    if (session_key_item != NULL && cJSON_IsString(session_key_item)) {
   
        char * session_key= (char *)malloc(strlen(session_key_item->valuestring) + 1);
        if (session_key == NULL) {
            return ;
        }
        strcpy(session_key, session_key_item->valuestring);
        idsnetworkManagerModule.session_key = session_key;

    } else {
        printf("Register session_key not found or not a string\n");
    }
    cJSON *token_item = cJSON_GetObjectItem(data,"token");
    if (token_item != NULL && cJSON_IsString(token_item)) {
        char * token= (char *)malloc(strlen(token_item->valuestring) + 1);
        if (token == NULL) {
            return ;
        }
        strcpy(token, token_item->valuestring);
        idsnetworkManagerModule.token = token;
    } else {
        printf("token not found or not a string\n");
    }
    cJSON_Delete(json_obj);
    
    free(cypher);
    free(response);
}
char * getcaPath()
{
    return idsnetworkManagerModule.caPath;
}

void getheartbeat()
{
  
    char _url[128];
    memset(_url,0,128);
    strcpy(_url,idsnetworkManagerModule.url);
    _url[sizeof(_url) - 1] = '\0'; 
    strcat(_url,GET_MONITOR_CONFIG);
    int l_urllen = strlen(_url);
    char *URL = malloc(l_urllen + 64);
    memset(URL,0,l_urllen + 64);
    snprintf(URL,l_urllen + 64,"%s%s%s%s%lld",_url,UPDATE_SN_SUFFIX,\
        idsnetworkManagerModule.sN,UPDATE_TIME_SUFFIX,0);

    char *header = (char *)malloc(256 + strlen(idsnetworkManagerModule.channelId)  \
    + strlen(idsnetworkManagerModule.sN) + strlen(idsnetworkManagerModule.token));
    if(header == NULL)
        return NULL;
    sprintf(header, "{\"X-Channel-ID\":\"%s\",\"X-Crypt\":\"2\",\"X-Proto-Buf\":\"0\","
                    "\"X-Sn\":\"%s\",\"Authorization\":\"%s\"}",
            "T00001",idsnetworkManagerModule.sN,idsnetworkManagerModule.token);
    BaseResponse_t *response = startGetRequest(URL,header,&http_handle);
    printf("33222222222233 response->errorMsg:%s\n",response->responseData);
    if(response == NULL) {
        return;
    }
    if (!response->responseData) {
        free(response);
        return;
    }

    char iv[KEY_IV_LENGTH+1]  = {0};
    char key[KEY_IV_LENGTH+1] = {0};
    cypher_t *plain=NULL;
    cypher_t *cypher=NULL;
    char *session_key = idsnetworkManagerModule.session_key;
    printf("session_key:%s\n",session_key);
    if(session_key == NULL) {
        return;
    }
    memcpy(key, session_key, 16);
    key[16] = '\0';    
    memcpy(iv, session_key + 16, 16);
    iv[16] = '\0';
    size_t resp_data_len =response->responseDatalength;
    // 使用更保守的内存分配策略
    plain = (cypher_t*)calloc(1, sizeof(cypher_t) + resp_data_len + 16);
    if (!plain) {

        printf("Error: memory allocation failed\n");
        free(response);
        return;
    }

    plain->len_data = resp_data_len;
    if (resp_data_len > 0) {
        memcpy(plain->data, response->responseData, resp_data_len);
    }
    plain->data[resp_data_len] = '\0';

    cypher = aes_cbc_decrypt(plain, key, iv);
    printf("11%s\n",cypher->data);

    if (!cypher) {
        printf("Error: decryption failed\n");
        free(response);
        return;
    }


    free(header);
    free(URL);
    free(response);
    free(cypher);
    free(plain);
    return;
}