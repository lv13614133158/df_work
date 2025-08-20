#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

//设置日志记录等级,大于此登记可以被记录
#define LOG_LEVEL   0 
#define LOG_ERR     4
#define LOG_EVENT   3
#define LOG_INFO    2
#define LOG_DEBUG   1

void log_debug(int level, const char *msg, ...);
int  log_init();

#ifdef __cplusplus
}
#endif

#endif