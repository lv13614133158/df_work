/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-05-09 09:51:23
 */ 

#ifndef __SQL_NETWORKMONITOR_
#define __SQL_NETWORKMONITOR_
#ifdef __cplusplus
extern "C" {
#endif
#include "util.h"
#include <stdbool.h>
#include "idps_main.h"
#include "Adt_networkmonitor.h"
#include <stdint.h>
#if MODULE_NETWORKMONITOR

void NetworkSql_createTable(void);
list *NetworkSql_queryIpBlackList(void);
list *NetworkSql_queryPortBlackList(void);
list *NetworkSql_queryDnsBlackList(void);
list *NetworkSql_queryProcessBlackList(void);
list *NetworkSql_queryDirectMap(void);
NetworkMonitorConfig *NetworkSql_queryConfig(void);
bool NetworkSql_insertConfig(int idpsmode,
				  int period,
				  char *mobileIfaces,
				  char *wifiIface,
				  long long lastidpsTimestamp,
				  long long lastIpTimestamp,
				  long long lastDnsTimestamp,
				  long long lastUpdateBlackIpTimestamp,
				  long long lastUpdateBlackDnsTimestamp,
				  long long lastUpdateBlackProcessTimestamp,
				  long long lastUpdateBlackRedirectIpTimestamp,
				  long long lastUpdateBlackPortTimestamp,
				  long long lastfileTimestamp);
bool NetworkSql_updateMobileIfacesConfig(char *_input);
bool NetworkSql_updateWifiIfaceConfig(char *wifiIface);
bool NetworkSql_updatePeriodConfig(int period);
bool NetworkSql_updateLastIpTimestamp(long long timestamp);
bool NetworkSql_updateLastidpsmodeTimestamp(long long timestamp);
bool NetworkSql_updateLastUpdateIpBlackListTimestamp(long long timestamp);
bool NetworkSql_updateLastUpdatePortBlackListTimestamp(long long timestamp);
bool NetworkSql_updateLastUpdateDnsBlackListTimestamp(long long timestamp);
bool NetworkSql_updateLastUpdateProcessBlackListTimestamp(long long timestamp);
bool NetworkSql_updateLastUpdateRedirectIpMapTimestamp(long long timestamp);
bool NetworkSql_updateLastDnsTimestamp(long long timestamp);
list *NetworkSql_queryIpUploadDataGte(long long timestamp);
list *NetworkSql_queryIpDataGte(long long timestamp);
list *NetworkSql_queryDnsDataGte(long long timestamp);
list *NetworkSql_queryDnsUploadDataGte(long long timestamp);
bool NetworkSql_insertBlackDnsConnectEvent(char *processName, char *dns, char *destIp);
bool NetworkSql_insertBlackProcessConnectEvent(char *processName, int uid);
bool NetworkSql_insertIpConnectData(char *processName, int version, char *srcIp, int srcPort, char *destIp, int destPort, int protocol);
bool NetworkSql_insertDnsConnectData(char *processName, char *dns, char *destIp);
bool NetworkSql_insertBlackIpConnectEvent(char *processName, int version, char *srcIp, int srcPort, char *destIp, int destPort, int protocol);
bool NetworkSql_deleteDnsBlackList(uint8_t *dns,uint8_t code);
bool NetworkSql_insertDnsBlackList(uint8_t *dns,uint8_t len);
bool NetworkSql_insertProcessBlackList(uint8_t *process,uint8_t length);
bool NetworkSql_insertDirectMap(char *destIp, char *redirectIp);
bool NetworkSql_deleteDirectMap(char *destIp, char *redirectIp);
bool NetworkSql_insertIpBlackList(uint8_t *ip,uint8_t length);
bool NetworkSql_insertPortBlackList(uint8_t *ip,uint8_t length);
bool NetworkSql_deleteIpBlackList(uint8_t *ip,uint8_t length);
bool NetworkSql_deletePortBlackList(uint8_t *ip,uint8_t length);
bool NetworkSql_deleteProcessBlackList(uint8_t *process,uint8_t length);
bool NetworkSql_updateDirectMap(char *destIp, char *redirectIp);
bool NetworkSql_updateIDPSConfig(int period);
int NetworkSql_encrypt(const char *org_pass,char *new_pass);
int NetworkSql_decrypt(const char *new_pass,char *org_pass);
bool NetworkSql_deletefileList(uint8_t *ip,uint8_t length);
bool NetworkSql_insertfileList(uint8_t *file,uint8_t length);
list *NetworkSql_queryfileList(void);
bool NetworkSql_updateLastfileTimestamp(long long timestamp);

#endif 
#ifdef __cplusplus
}
#endif
#endif