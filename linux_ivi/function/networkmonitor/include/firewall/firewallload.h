#ifndef _FIRELOAD_USER_H
#define _FIRELOAD_USER_H
#include "util.h"
/**
 * mark down ,to process some process,later you can delete it */
#define IPTABLES_VERSION "1.6.1"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define  errorexit        -1
typedef struct _arg_t{
	unsigned char data[0];
}arg_t;
struct firerule{
	/***just for head***/
	char Chain[32];//chain name
	char policy[32];//save as Chainlist
	/*******just for rules**********/
	int  num;
	char devicein[18];
	char deviceout[18];

	char target[32];
	char prot[8];
	char opt[16];
	char source[32];
	char destination[32];
	char expand[128];
};
struct stringmatch{
	char target[32];
	char fromstring[16];
	char dnsstring[64];
	char algostring[8];
	char tostring[16];
	char protocol[16];
	char destip[32];
	char srcip[32];
	char chain[32];
	char eth[32];
};
struct iplist{
	char target[32];
	char Chain[32];
	char prot[8];
	char source[32];
	char destination[32];
	int dstport;
	int srcport;
	char eth[32];
};
struct dnatlist{
	char prot[8];
	char source[32];
	char destination[32];
	int dstport;
	int srcport;
	char eth[32];
};
 
extern list list_firerules;;
extern struct firerule *pfirerule;
#endif
