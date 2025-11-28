#ifndef LIBLOG_H
#define LIBLOG_H
#include <stdint.h> 
#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
    LOG_DEFAULT = 0,   // 默认
    LOG_VEHICLE = 1,   // 车辆
    LOG_NETWORK = 2,   // 网络
    LOG_DIAGNOSTIC = 3,// 诊断
    LOG_OTA = 4,       // OTA
    LOG_V2X = 5        // V2X
} LOG_TYPE;
// log_time[14] 数组中各位置表示的时间部分：

// 位置	含义
// [0]-[3]	年份 (YYYY)
// [4]-[5]	月份 (MM)
// [6]-[7]	日期 (DD)
// [8]-[9]	小时 (HH)
// [10]-[11]	分钟 (MM)
// [12]-[13]	秒 (SS)
typedef struct 
{

    char source[128];       //来源
    int level;              //日志等级
    uint8_t log_time[14];   //日志时间
    LOG_TYPE log_type;      //日志类型
    char log_tag[128];      //日志标签
    int data_len;           //日志数据长度
    char log_date[1024];    //日志数据
} LOG_DATA;

typedef void (*log_message_callback)(LOG_DATA *  message);

/**
 * 初始化日志系统（写入端）
 * @return 0 成功，-1 失败
 */
int ids_log_init(void);

/**
 * 初始化日志系统（读取端）
 * @param callback 接收消息时调用的回调函数
 * @param enable_auto_cleanup 是否启用自动清理
 * @return 0 成功，-1 失败
 */
int ids_log_reader_init(log_message_callback callback, int enable_auto_cleanup);
/**
 * 清理读端资源（读取端）
 * @param callback 
 * @param enable_auto_cleanup 
 * @return 
 */
void ids_log_reader_cleanup(void);

/**
 * 写入日志数据
 * @param data 日志数据
 * @param len 数据长度
 * @return 写入的字节数，失败返回-1
 */
int ids_log_write(LOG_DATA *data);


#ifdef __cplusplus
}
#endif

#endif /* LIBLOG_H */