#ifndef		__DATA_DISPATCHER_H__
#define		__DATA_DISPATCHER_H__

#include "util.h"

void data_dispatcher_init(s8 *interface_name);
void  stop_pcap(void);
boolean set_store_path(s8 *);
void sniffer_start();
void sniffer_stop();
char* getnetinfoforpcap();
char* getnettxrx();
void ipWhiteCheckInit(list *listName);
void updateNetConnectReportInterval(int interval);
void DNSWhiteCheckInit(list *listName);

#endif
