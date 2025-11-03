#ifndef __CAN_MANAGE_H__
#define __CAN_MANAGE_H__

typedef struct _TCAN{
    unsigned int   FID;
    unsigned char  FIdxChn;  // 通道
    unsigned char  FDLC;;    
    unsigned char  FData[64];
} TCAN, *PCAN;

// can 数据写入
int can_write(int fd, TCAN data);
// can数据读取
int can_read(int fd, TCAN* data);

// 设置回调函数
typedef void(*can_fun_callback)(TCAN data);
int can_set_callback(can_fun_callback fp);
// 启动can读取模块
int can_start();
// 停止can读取模块
int can_stop();
// 查询can模块状态
int* can_get_status();

#endif