#ifndef LIBIDSLIG_H
#define LIBIDSLIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化日志系统（写入端）
 * @return 0 成功，-1 失败
 */
int ids_log_init(void);

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

#ifdef __cplusplus
}
#endif

#endif /* LIBIDSLIG_H */