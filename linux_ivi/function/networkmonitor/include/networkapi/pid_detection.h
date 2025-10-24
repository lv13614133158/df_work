#ifndef __PID_DETECTION_H__
#define	__PID_DETECTION_H__

void piddetection_scanner_init();
void pid_value_consumer(void);
void tcppid_callback(char *src_ip,int src_port,char* des_ip,int des_port,char* str_pid,char *processname,char maxlength);
void udppid_callback(char *src_ip,int src_port,char* des_ip,int des_port,char* str_pid,char *processname,char maxlength);
void append_netport_list(unsigned int portint, int whitelist);

#endif
