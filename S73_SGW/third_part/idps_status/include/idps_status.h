#ifndef _IDPS_STATUS_H_
#define _IDPS_STATUS_H_

#include <time.h>

/* 错误码 */
#define    IDPS_SOFTWARE_ONGOING        0x05000000
#define    IDPS_SOFTWARE_NORUNNING      0x05000001
#define    IDPS_WORKFLOW_SUCCESS        0x05010000
#define    IDPS_WORKFLOW_NOVIN          0x05010001
#define    IDPS_WORKFLOW_NOSN           0x05010002
#define    IDPS_WORKFLOW_NONETCOMMU     0x05010003
#define    IDPS_WORKFLOW_NOCERT         0x05010004
#define    IDPS_PROGRAM_OK              0x05020000
#define    IDPS_PROGRAM_NOTOK           0x05020001
#define    IDPS_PROGRAM_OOM             0x05020002

/**
 * \brief 状态集合结构体
 */
typedef struct
{
    unsigned int    idps_software_status;  /**< 软件状态 */
    unsigned int    idps_workflow_status;  /**< 业务状态 */
    unsigned int    idps_program_status;   /**< 程序状态 */
    unsigned int    idps_version;          /**< 版本号 */
} idpsStatusInfo_t;

typedef struct
{
    idpsStatusInfo_t idps_status;
    time_t idps_status_last_tv_sec;
} idpsStatusTransfer_t;

/**
 * \brief 状态通知，回调函数模板
 */
typedef void(*fun_callback)(idpsStatusInfo_t statusinfo);

/**
 * \brief 注册回调函数
 * \note  状态改变后，通过回调函数通知使用者
 * \param fp[in]                输入回调函数指针。
 * \return                      0:注册成功，-1:注册失败。
 */
int init_idps_status(fun_callback fp);

/**
 * \brief 销毁回调函数
 */
void deinit_idps_status();

/**
 * \brief 获取IDPS状态信息
 * \param statusInfo[out]       导出IDPS状态信息结构体。
 * \return                      0:获取成功，-1:获取失败。 
 */
int get_idps_status(idpsStatusInfo_t *statusInfo);

/**
 * \brief 获取idps_status版本号
 * \return                      libidps_status版本号
 */
const char *get_lib_idps_status_version();

#endif