#ifndef LIBLOG_H
#define LIBLOG_H

#ifdef __cplusplus
extern "C" {
#endif

// 回调函数类型定义
typedef void (*log_message_callback)(const char* message, int len);

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
int ids_log_write(const char *data, int len);

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