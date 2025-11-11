/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-08-09 06:23:50
 */ 
#include "Networkfirewall.h"
#include "sIptables.h"
#if MODULE_NETFIREWALL

void startIptables(char* rules){
   setIptable(rules);
}

NetWorkFirewallMethod NetWorkFirewallMethodObj ={
   startIptables
} ;
#endif
