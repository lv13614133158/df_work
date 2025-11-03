
#ifndef __TCP_DETECTION_H__
#define __TCP_DETECTION_H__
void tcpport_value_consumer(void);
void tcp_parser(u32 src_addr,u32 dest_addr,struct tcphdr* tcp,u32 pack_length);
void tcp_scanner_init();

#endif

	

