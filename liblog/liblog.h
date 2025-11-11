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

    char source[128]; 
    int level;
    LOG_TYPE log_type;
    char log_tag[128];
    int data_len;
    char log_date[1024];
} LOG_DATA;

#ifdef __cplusplus
extern "C" {
#endif


int ids_log_write(LOG_DATA *data);


#ifdef __cplusplus
}
#endif

#endif /* LIBIDSLIG_H */