#ifndef		__ICMP_DETECTION_H__
#define		__ICMP_DETECTION_H__
void icmp_scan_init(void);
void icmp_parser(struct iphdr *ip,struct icmphdr* icmp,u32 pack_length);
void icmp_value_consumer(void);

#endif




