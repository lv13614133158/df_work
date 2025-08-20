/**
 * 文件名: log.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 日志写入功能
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "log.h"
#include "ids_config.h"

#define LOG_PRINT_MODE      1        // 日志打印开关
#define LOG_WRITE_MODE      1        // 日志落盘开关
#define LOG_SAVEALLFILE     4        // 日志滚动个数
#define LOG_PATH_SIZE    (255)       
#define LOG_BUF_SIZE     (1024)           // 每次写入最大数据量 byte
#define LOG_FILE_SIZE    (5*1024*1000)     // 每个日志文件最大大小 KB
#define LOG_FILE_PATH    "/usr/local/360/canids/log"
#define LOG_FILE_NAME    LOG_FILE_PATH "/record"

#ifdef USED_MCU_TYPE
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
    printf("[log]:%s\n", message);   
}  
#else
pthread_mutex_t mutexlog = PTHREAD_MUTEX_INITIALIZER;

// 方便将框架赋值参数，而不是宏定义里
typedef struct _log_st  
{  
    char path[LOG_PATH_SIZE];  
    int fd;  
    int size;  
    int level;  
    int num;         
}log_st;
static log_st *logs = NULL;

static void log_checksize()  
{  
#define LOG_PATH_SIZE_ADD  (LOG_PATH_SIZE+10) 
    struct stat stat_buf;  
    char new_path[LOG_PATH_SIZE_ADD] = {0};  
    char bak_path[LOG_PATH_SIZE_ADD] = {0};  

    if(NULL == logs || '\0' == logs->path[0]) 
        return;  
        
    memset(&stat_buf, 0, sizeof(struct stat));  
    fstat(logs->fd, &stat_buf);  
    if(stat_buf.st_size > logs->size) 
    {  
        close(logs->fd);  
        if(logs->num) { 
            //snprintf(new_path, LOG_PATH_SIZE, "%s%d", logs->path, (int)time(NULL)); 
            snprintf(bak_path, LOG_PATH_SIZE_ADD, "%s.%c", logs->path, logs->num+'A');
            remove(bak_path);
            for(int i=0; i<logs->num; i++)
            {
                snprintf(bak_path, LOG_PATH_SIZE_ADD, "%s.%c", logs->path, 'A'+logs->num-i);  
                snprintf(new_path, LOG_PATH_SIZE_ADD, "%s.%c", logs->path, 'A'+logs->num-i-1);   
                rename(new_path, bak_path);
            } 
        }
        else {  
            snprintf(bak_path, LOG_PATH_SIZE_ADD, "%s.log.old", logs->path);  
            snprintf(new_path, LOG_PATH_SIZE_ADD, "%s.log", logs->path);  
            remove(bak_path);  
            rename(new_path, bak_path);
        }  
        //create a new file  
        logs->fd = open(new_path, O_RDWR|O_APPEND|O_CREAT|O_SYNC, S_IRUSR|S_IWUSR|S_IROTH);  
    }  
}

/*print msg*/
void log_debug(int level, const char *msg, ...)  
{  
    va_list ap;  
    time_t now;  
    char *pos;  
    char _n = '\n';  
    char message[LOG_BUF_SIZE] = {0};  
    int nMessageLen = 0;  
    int sz; 

    if(NULL == logs || level < logs->level) 
        return; 

    now = time(NULL); 
   	pos = ctime(&now);
    sz = strlen(pos); 
    pos[sz-1] =']'; 
    snprintf(message, LOG_BUF_SIZE, "[%s\n", pos);  
    for (pos = message; *pos; pos++);  
	sz = pos - message;  

    va_start(ap, msg);  
    nMessageLen = vsnprintf(pos, LOG_BUF_SIZE - sz, msg, ap);  
    va_end(ap);  
    if (nMessageLen <= 0) 
        return; 
    if (LOG_PRINT_MODE)
        printf("%s\n", message);   
    if (LOG_WRITE_MODE) 
    {   
        pthread_mutex_lock(&mutexlog);
        log_checksize();
        int ret = write(logs->fd, message, strlen(message));
        if (ret != strlen(message)) {
            fprintf(stderr, "Log write error");
        }
        ret = write(logs->fd, &_n, 1); 
        if (ret != 1) {
            fprintf(stderr, "Log write error");
        }
        fsync(logs->fd); 
        pthread_mutex_unlock(&mutexlog);
    }
}  

/*creat and init log file*/
static log_st *_log_init(char *path, int size, int level, int num)  
{  
	char new_path[LOG_PATH_SIZE] = {0};  

	if (NULL == path ||  level > LOG_ERR || logs != NULL) 
        return NULL; 

    log_st *logs = (log_st *)malloc(sizeof(log_st));  
    memset(logs, 0, sizeof(log_st));  
    if (LOG_WRITE_MODE) 
    {  
        //the num use to control file naming  
        logs->num = num;  
        if(num)  
            //snprintf(new_path, LOG_PATH_SIZE, "%s%d", path, (int)time(NULL));
            snprintf(new_path, LOG_PATH_SIZE, "%s.A", path);  
        else  
            snprintf(new_path, LOG_PATH_SIZE, "%s.log", path);  
        if(-1 == (logs->fd = open(new_path, O_RDWR|O_APPEND|O_CREAT|O_SYNC, S_IRUSR|S_IWUSR|S_IROTH))) {  
            free(logs);  
            logs = NULL;  
            return NULL;  
        }  
    }  
    strncpy(logs->path, path, LOG_PATH_SIZE);  
	logs->size = (size > 0 ? size:0);  
	logs->level = (level > 0 ? level:0);  

	return logs;  
}

int log_init() 
{
    struct stat stat_buf; //useless, but ...
	if(stat(LOG_FILE_PATH, &stat_buf) == -1) 
    {
		/*create irtouch log dir*/
		mkdir(LOG_FILE_PATH, 0750);
	}
    logs = _log_init(LOG_FILE_NAME, LOG_FILE_SIZE, LOG_LEVEL, LOG_SAVEALLFILE); 
    return 1;    
}

#endif