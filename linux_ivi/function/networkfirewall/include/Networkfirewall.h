
#ifndef __NETWORKFIREWALL_
#define __NETWORKFIREWALL_
#ifdef __cplusplus
extern "C" {
#endif
#include "idps_main.h"
#if MODULE_NETFIREWALL

typedef struct _NetWorkFirewallMethod{
    void (*startIptables)(char*);
}NetWorkFirewallMethod;
extern NetWorkFirewallMethod NetWorkFirewallMethodObj;

#endif 
#ifdef __cplusplus
}
#endif
#endif