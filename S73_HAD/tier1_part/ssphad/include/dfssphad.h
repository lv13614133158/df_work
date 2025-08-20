/**
 * @file dfssphad.h
 * @brief SDK接口头文件
 * getCertState 获取设备证书状态接口，
 * getCertStateByDeviceId 通过零部件ID获取设备证书状态接口，
 * importRootCert 写入根设备证书，
 * importGeneralCert 写入通用设备证书及对应私钥接口，
 * requestCert 申请下发设备证书接口，
 * updateCert 申请更新设备证书接口，
 * verifyControlInstruction 控车指令验签，
 */

#ifndef DFSSPHAD
#define DFSSPHAD
#include "stddef.h"
#include <stdarg.h>

/* 日志级别定义 */
#define LOG_ERROR               0
#define LOG_WARN                1
#define LOG_INFO                2
#define LOG_DEBUG               3

typedef void (*LogCallback)(int level, const char* msg);

void set_log_callback(LogCallback cb);  // 回调函数
int getCertState(void);//完成
int getCertStateByDeviceId(char *deviceId, size_t deviceIdLen);
int importRootCert(void* cert, size_t certLen);
int importGeneralCert(void* cert, char *password, size_t certLen, size_t passwordLen);
int requestCert(char *deviceId, size_t deviceIdLen);
int updateCert(void);
// 读取证书接口
int DSec_ReadFile(const char * id_name, char * pp_DataOut, size_t vp_DataOutLen);

#ifdef DEBUG_ON
//测试删除证书,测试完成后删除
// int deleteCert(const char *id);
// int setCertStateTest(int state);
// int importDeviceCert(void* cert, size_t certLen);
// int importDeviceKey(void* cert, size_t certLen);
// int getRootCertTest(char *cert, size_t certLen);
#endif
#endif