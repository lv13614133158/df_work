/**
 * 文件名: canmanage.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: can驱动模块
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "log.h"
#include "canmanage.h"

// can通道描述符
#define MAX_NUM_CAN  (2)
static int fd_can[MAX_NUM_CAN];
static can_fun_callback pcan_output;


// 打开can 设置波特率
int can_open(char *dev, int Baudrate)
{
    int sockfd = -1;
    struct sockaddr_can addr = {0};
    struct ifreq ifr = {0};

    log_debug(LOG_INFO, "(CAN):%s can open baudrate:%d\n", dev, Baudrate);
    // 设置波特率
    //system("sudo ip link set can0 type can bitrate 500000");
    //system("sudo ifconfig can0 up");
    char cmdline[63] = {0};
    snprintf(cmdline, 63, "sudo ip link set %s type can bitrate %d", dev, Baudrate);
    system(cmdline);
    snprintf(cmdline, 63, "sudo ifconfig %s up", dev);
    system(cmdline);

    //1.Create socket
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0) {
        log_debug(LOG_ERR,"(CAN):socket PF_CAN failed");
        return -1;
    }

    //2.Specify can0 device
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        log_debug(LOG_ERR,"(CAN):ioctl failed");
        return -1;
    }

    //3.Bind the socket to can0
    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        log_debug(LOG_ERR,"(CAN):bind failed");
        return -1;
    }

    // 4.Define receive rules
    // 设置过滤规律，如只接收ID 为0x123和0x89A 的报文帧
    // 这里默认全部接收
    if(0){
        struct can_filter rfilter[2];
        rfilter[0].can_id = 0x123;
        rfilter[0].can_mask = CAN_SFF_MASK;
        rfilter[1].can_id = 0x89A;
        rfilter[1].can_mask = CAN_SFF_MASK;
        setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    }
    return sockfd;
}

// can 关闭
void can_close(int fd)
{
    close(fd);
    log_debug(LOG_INFO, "(CAN):can close\n");
}

// can数据读取
int can_read(int fd, TCAN* data)
{
    struct can_frame frame = {0};

    if( !data ){
        return -1;
    }

    if (0 > read(fd, &frame, sizeof(struct can_frame))) {
        log_debug(LOG_ERR, "(CAN):read error\n");
        return -1;
    }

    if (frame.can_id & CAN_ERR_FLAG) {
        log_debug(LOG_ERR, "(CAN):Error frame!\n");//错误帧
        return -1;
    }
    else if (frame.can_id & CAN_RTR_FLAG) {
        log_debug(LOG_ERR, "(CAN):remote request\n");//远程帧
        return -1;
    }
    else if (frame.can_id & CAN_EFF_FLAG) //扩展帧
    {
        printf("扩展帧 <0x%08x> ", frame.can_id & CAN_EFF_MASK);
        data->FID = frame.can_id & CAN_EFF_MASK;
    }
    else //标准帧
    {
        //printf("标准帧 <0x%03x> ", frame.can_id & CAN_SFF_MASK);
        data->FID = frame.can_id & CAN_SFF_MASK;
    }

    data->FDLC = frame.can_dlc;
    for (int i = 0; i < frame.can_dlc; i++)
        data->FData[i] = frame.data[i];

    for(int i=0; i<MAX_NUM_CAN; i++)
        if(fd == fd_can[i]){
            data->FIdxChn = i;
        }
    
    return 1;
}

// can 数据写入
int can_write(int fd, TCAN data)
{
    struct can_frame frame = {0};

    for (int i = 0; i < data.FDLC; i++)
        frame.data[i] = data.FData[i];
    frame.can_dlc = data.FDLC; //一次发送 len 个字节数据
    frame.can_id  = data.FID;  //帧 ID 为 0x123,标准帧

    int ret = write(fd, &frame, sizeof(frame)); //发送数据
    if(sizeof(frame) != ret) { //如果 ret 不等于帧长度，就说明发送失败
        log_debug(LOG_ERR, "(CAN):write error");
        return -1;
    }
    return 1;
}

#include <pthread.h>
// 0:准备, 1:运行中, 2:退出中
static int can_run[MAX_NUM_CAN] = {0}; 
static int can_idx[MAX_NUM_CAN] = {0}; 
static pthread_t thread_can_thd[MAX_NUM_CAN];

static void *read_thread(void *args){
    int index = *(int*)args;
	pthread_detach(pthread_self());

	while(can_run[index] == 1)
    {
        TCAN data ={0};
        if(fd_can[index]>0 && pcan_output && (can_read(fd_can[index], &data)==1))
        {
            pcan_output(data);
        }  
	}
    can_run[index] = 0;
}

// 线程调度管理
void manager_thread(int action)
{
    for(int i=0; i<MAX_NUM_CAN; i++)
    {
        can_idx[i] = i;
        while(can_run[i] == 2)//防止上次退出未完成
        {
            usleep(10000);
            printf("%d can eixt...\n",can_idx[i]);
        }

        if(action)
        {
            if(can_run[i] == 0){
                can_run[i] = 1;
                pthread_create(&thread_can_thd[i], 0, read_thread, &can_idx[i]);
            }
        }
        else
        {   
            if(can_run[i] == 1)
                can_run[i] = 2;
            while(can_run[i]) // 等待退出完成
            {
                usleep(10000);
                printf("%d can eixt...\n", can_idx[i]);
            }
        }
    }
}

// 设置回调函数
int can_set_callback(can_fun_callback fp)
{
    pcan_output = fp;
    return 1;
}

// 启动can读取模块
int can_start()
{
    int ret = 0;
    fd_can[0] = can_open("can0", 500000);
    fd_can[1] = can_open("can1", 500000);
    if(fd_can[0] < 0 && fd_can[1] < 0)
    {
        can_stop();
        log_debug(LOG_ERR, "(CAN):can open \n");
        ret = -1;
    }
    else
    {
        manager_thread(1);
    }
    return ret;
}

// 停止can读取模块
int can_stop()
{
    int ret = 0;
    for(int i=0; i<MAX_NUM_CAN; i++)
    {
        can_close(fd_can[i]);
        fd_can[i] = 0;
    }
    manager_thread(0);
    return ret;
}

// 查询can模块状态
int* can_get_status()
{
    return can_run;
}

#if 0

// 数据接收回调，can驱动被动调用
#ifndef USED_MCU_TYPE  
#include "canmanage.h"
TCAN can_data = {0};
void can_callback(TCAN data)
{
    SK_CANIDS_MsgReceiveIndi(data.FIdxChn, data.FID, data.FData, data.FDLC, 0);
}
#endif

int main1(int argc, char *argv[])
{
    // 日志初始化
    log_init();
    log_debug(LOG_INFO, "[I]can IDS module start");
#ifndef USED_MCU_TYPE 
    // can回调
    can_set_callback(can_callback);
    // can驱动开始
    can_start();
#endif
    // ids初始化
    can_init(0, NULL);

    signal(SIGINT, cleanHandler);
    for(;;){
        sleep(10);
    }
}

#endif
