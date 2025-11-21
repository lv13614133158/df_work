#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>

// 生成 WebSocket Key 的辅助函数
char* generate_websocket_key() {
    static char key[25];
    srand(time(NULL));
    for(int i = 0; i < 22; i++) {
        int val = rand() % 64;
        if(val < 26) {
            key[i] = 'A' + val;
        } else if(val < 52) {
            key[i] = 'a' + (val - 26);
        } else if(val < 62) {
            key[i] = '0' + (val - 52);
        } else if(val == 62) {
            key[i] = '+';
        } else {
            key[i] = '/';
        }
    }
    key[22] = '=';
    key[23] = '=';
    key[24] = '\0';
    return key;
}

// 回调函数处理接收到的数据
struct responseData {
    char *data;
    size_t size;
};

size_t WriteCallback(void *contents, size_t size, size_t nmemb, struct responseData *response) {
    size_t realsize = size * nmemb;
    char *ptr = realloc(response->data, response->size + realsize + 1);
    
    if(ptr == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = 0;
    
    return realsize;
}

int main(void) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct responseData response = {0};
    
    // 初始化 curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        // 设置 WebSocket 连接 URL (使用 HTTPS 版本)
        curl_easy_setopt(curl, CURLOPT_URL, "https://echo.websocket.org");
        
        // 设置 HTTP 升级头
        headers = curl_slist_append(headers, "Connection: Upgrade");
        headers = curl_slist_append(headers, "Upgrade: websocket");
        headers = curl_slist_append(headers, "Sec-WebSocket-Version: 13");
        
        // 生成并设置 WebSocket Key
        char ws_key_header[100];
        snprintf(ws_key_header, sizeof(ws_key_header), "Sec-WebSocket-Key: %s", generate_websocket_key());
        headers = curl_slist_append(headers, ws_key_header);
        
        // 添加 Host 头部
        headers = curl_slist_append(headers, "Host: echo.websocket.org");
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // 设置 SSL 选项
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        
        // 设置其他选项
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
        
        // 设置跟随重定向
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        // 执行请求
        printf("Connecting to WebSocket server...\n");
        res = curl_easy_perform(curl);
        
        // 检查结果
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Response:\n%s\n", response.data);
            
            // 检查是否成功升级到 WebSocket
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            printf("HTTP Response Code: %ld\n", response_code);
        }
        
        // 清理
        curl_slist_free_all(headers);
        if(response.data) {
            free(response.data);
        }
    }
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    return 0;
}