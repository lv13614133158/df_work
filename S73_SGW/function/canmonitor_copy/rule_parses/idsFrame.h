#ifndef  __CAN_IDS_FRAME_H__
#define  __CAN_IDS_FRAME_H__

// can 初始化, 线程创建
int can_init(int argc, char *rule);
int can_device_pub_dat(unsigned char netID, unsigned int canID, unsigned char* data, unsigned int len, double time);

#endif
