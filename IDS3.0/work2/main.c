/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-06-16 22:14:32
 */ 
#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include "https.h"
#include "idswebsocket.h"

  // 新增函数：一次性获取四个关键信息

    

int main(){
    setHttpsconf("https://vsocidps-uat.dfiov.com.cn","LQH02501170001","T00001","t-box"); 
    Https_Path_init("/home/nvidia/df/df_work/CA/s73a2/","./idps/cache","https://vsocidps-uat.dfiov.com.cn");
    
    char* response = (char* )malloc(GETREQUEST_DATALEN_LEN);
    int response_len=0;
    getRequestData(GET_MONITOR_CONFIG,0,&response,&response_len,GETREQUEST_DATALEN_LEN);
    char* manageKey = NULL;
    char* sessionKey = NULL;
    char* sn = NULL;
    char* token = NULL;
    ids_keystore_t keystore;
    manageKey=getManageKeyApi();
    sessionKey=getSessionKeyApi();
    sn=getSn();
    token=getToken();
    memcpy(keystore.m_manageKey,manageKey,128);
    memcpy(keystore.m_sessionKey,sessionKey,128);
    memcpy(keystore.m_sn,sn,128);
    memcpy(keystore.m_token,token,512);

    ids_websocketInfoModult_t  websocketInfoModult;
    strcpy(websocketInfoModult.url, "vsocidps-uat.dfiov.com.cn");
    strcpy(websocketInfoModult.channelId, "T00001");
    strcpy(websocketInfoModult.equmentType, "t-box");
    strcpy(websocketInfoModult.sn, "LQH02501170001");
    websocketInfoModult.version = 1;
    websocketInfoModult.sslShutdown = 1;
  	websocketInfoModult.caShutdown = 0;
    websocketInfoModult.port = 443;
    websocketInfoModult.intervalread = 1000;
    websocketInfoModult.intervalheart = 5000;
    websocketInfoModult.intervalkey = 200;
    strcpy(websocketInfoModult.listPath, "./idps/cache");
    strcpy(websocketInfoModult.ca_certs_Path, "/home/nvidia/df/df_work/CA/s73a2/root_cert.pem");    
    strcpy(websocketInfoModult.client_certs_Path, "/home/nvidia/df/df_work/CA/s73a2/device_cert.pem"); 
    strcpy(websocketInfoModult.client_key_Path, "/home/nvidia/df/df_work/CA/s73a2/device_key.pem"); 
    websocketclient_init(&websocketInfoModult,&keystore);
    // 等待连接建立
    sleep(5);
    // 修正后的数据格式
    char *data = "{\"action\":\"connack\",\"seq_number\":\"1763454826016001\",\"timestamp\":1763454826016}";
    
    for(;;){
        int ret = SendStrData(data);
        if(ret == 0){
            printf("SendStrData success  data = [%s]\n",data);
        }else{
            printf("SendStrData fail\n");
        }
        sleep(2);
    }
    
    return 0;
}