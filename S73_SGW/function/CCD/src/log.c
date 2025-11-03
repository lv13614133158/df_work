/**
 * 文件名: log.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 日志写入功能
 */

#include <stdarg.h>
#include "log.h"
#include "ids_config.h"
#include "communicate.h"

#define LOG_PATH_SIZE    (255)       
#define LOG_BUF_SIZE     (255)           // 每次写入最大数据量 byte
#define LOG_FILE_SIZE    (5*1024*1000)     // 每个日志文件最大大小 KB
int log_init() 
{
    return 1;
}

void log_debug(int level, const char *msg, ...)  
{  
    va_list ap;    
    char message[LOG_BUF_SIZE] = {0};  
    int  nMessageLen = 0;  
   
    va_start(ap, msg);  
    nMessageLen = vsnprintf(message, LOG_BUF_SIZE, msg, ap);  
    va_end(ap);  
    //printf("[log]:%s\n", message);
    sendCommLogMsg(SK_COMM_SUBFUNCTION_LOG_DEBUG, message, nMessageLen, 0);
}  

void log_event(int level, const char *msg, ...)  
{  
    va_list ap;    
    char message[LOG_BUF_SIZE] = {0};  
    int  nMessageLen = 0;  
   
    va_start(ap, msg);  
    nMessageLen = vsnprintf(message, LOG_BUF_SIZE, msg, ap);  
    va_end(ap); 

    sendCommLogMsg(SK_COMM_SUBFUNCTION_LOG_DEBUG, message, nMessageLen, 1);
}
