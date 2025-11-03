#ifndef _FIREL_INTERFACE_H
#define _FIREL_INTERFACE_H
#include <stdbool.h>
#include "util.h"
 enum colour {
    black,// 0
	white,
	DNAT,
	SNAT
};
void inputprocessstring(char* processstring);
bool  controlprocessbyandroid(bool status,char* processname,int pidnumber);
void dnsrulesadd(char *chain,char *dns,char status,char *ethdevice,char action);
void destionaddressconvert(char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice,char action);
char* queryallrules(char *chain);
void deleterules(char* chain,int num,char* table);
void chainstatusset(char* chain,int status);
void filterportpassornot(char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,char* chain,char status,char action);

void addwhiteinode(char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,list *listchain);
void addwhitednsinode(char *dns,char *ethdevice,list *listchain);
void adddnsblacklist(char *dns,char *ethdevice,list *listchain);
void addipblacklistrule(char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,list *listchain);
void adddnatlist(list *listchain,char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice);

bool check_dnsrules_list(char *dns,char *ethdevice,list *listchain);
bool deldnslist(char *dns,char *ethdevice,char colour,list *listchain);
bool check_iprules_list(list *listchain,char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,char colour);
bool deliplist(list *listchain,char colour,char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol);
bool check_dnatrules_black_list(list *listchain,char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice);
bool deldnatlist(list *listchain,char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice);

void allipblacklist(list* listchain,bool action);
void allblackdnslist(list* listchain,bool action);
void allwhiteiplist(list* listchain,bool action);
void allwhitednslist(list* listchain,bool action);
void alldnatlist(list* listchain,bool action);

void createlist(char* ethwhite,char* ethblack);
#endif /*_FIREL_INTERFACE_H*/
