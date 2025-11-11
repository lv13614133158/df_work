#ifndef LIBLOG_H
#define LIBLOG_H

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

typedef struct log_data
{
    char source[128]; 
    int level;
    LOG_TYPE log_type;
    char log_tag[128];
    int data_len;
    char log_date[1024];
} LOG_DATA;
// 回调函数类型定义
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
 * 写入日志数据
 * @param data 日志数据
 * @param len 数据长度
 * @return 写入的字节数，失败返回-1
 */
int ids_log_write(LOG_DATA *data);

/**
 * 清理写入端资源
 */
void ids_log_cleanup(void);

/**
 * 清理读取端资源
 */
void ids_log_reader_cleanup(void);

/**
 * 检查是否有活动的写入端
 * @return 1 有活动写入端，0 无活动写入端
 */
int ids_log_has_writer(void);

#ifdef __cplusplus
}
#endif

#endif /* LIBLOG_H */