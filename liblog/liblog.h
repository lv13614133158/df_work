#ifndef LIBIDSLIG_H
#define LIBIDSLIG_H

typedef enum 
{
    LOG_DEFAULT = 0,   // 默认
    LOG_VEHICLE = 1,   // 车辆
    LOG_NETWORK = 2,   // 网络
    LOG_DIAGNOSTIC = 3,// 诊断
    LOG_OTA = 4,       // OTA
    LOG_V2X = 5        // V2X
} LOG_TYPE;

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

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 写入日志数据到日志文件
 * 
 * 该函数负责将指定的日志数据写入到日志文件中，用于记录系统运行状态、
 * 错误信息或其他需要持久化的日志内容。
 * 
 * @param data 指向LOG_DATA结构体的指针，包含要写入的日志数据信息
 * 
 * @return int 返回操作结果状态码
 *         - 成功时返回0
 *         - 失败时返回相应的错误码
 */

int ids_log_write(LOG_DATA *data);



#ifdef __cplusplus
}
#endif

#endif /* LIBIDSLIG_H */