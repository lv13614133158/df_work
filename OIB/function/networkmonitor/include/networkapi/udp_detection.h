#ifndef __UDP_DETECTION_H__
#define __UDP_DETECTION_H__

void udp_scanner_init();
void udpport_value_consumer(void);
void udp_parser(u32 src_addr,u32 dest_addr,struct udphdr* udp,u32 pack_length);
void dns_parser(char* src_addr,char* dest_addr,struct udphdr* udp,int action);

#endif


