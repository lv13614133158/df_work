#ifndef __ARP_DETECTION_H__
#define __ARP_DETECTION_H__

void arp_parser_proc(void);
void arp_parser(struct arphdr *arpinput);

#endif
