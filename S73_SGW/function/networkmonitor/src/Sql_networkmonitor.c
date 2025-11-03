/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-05-09 09:51:43
 */

#include "Sql_networkmonitor.h"
#include "stdbool.h"
#include "common.h"
#include <string.h>
#include <stdlib.h>
#if MODULE_NETWORKMONITOR

/**
 * @name:   sql create tring
 * @Author: qihoo360
 * @msg:    network related sql operation
 * @param 
 * @return: 
 */
const char *SQL_CREATE_TABLE_IP_BLACKLIST = "CREATE TABLE IF NOT EXISTS network_ip_blacklist(id INTEGER PRIMARY KEY AUTOINCREMENT, black_ip TEXT)";
const char *SQL_CREATE_TABLE_PORT_BLACKLIST = "CREATE TABLE IF NOT EXISTS network_port_blacklist(id INTEGER PRIMARY KEY AUTOINCREMENT, black_port TEXT)";
const char *SQL_CREATE_TABLE_FILELIST = "CREATE TABLE IF NOT EXISTS network_filelist(id INTEGER PRIMARY KEY AUTOINCREMENT, file TEXT)";
const char *SQL_CREATE_TABLE_DNS_BLACKLIST = "CREATE TABLE IF NOT EXISTS network_dns_blacklist(id INTEGER PRIMARY KEY AUTOINCREMENT, black_dns TEXT)";
const char *SQL_CREATE_TABLE_PROCESS_BLACKLIST = "CREATE TABLE IF NOT EXISTS network_process_blacklist(id INTEGER PRIMARY KEY AUTOINCREMENT, black_process TEXT)";
const char *SQL_CREATE_TABLE_BLACK_IP_CONNECT = "CREATE TABLE IF NOT EXISTS black_ip_connect(id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp long, process TEXT, version INTEGER, src_ip TEXT, src_port INTEGER, dest_ip TEXT, dest_port INTEGER, protocol INTEGER)";
const char *SQL_CREATE_TABLE_BLACK_DNS_CONNECT = "CREATE TABLE IF NOT EXISTS black_dns_connect(id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp long, process TEXT, dns TEXT, dest_ip TEXT)";
const char *SQL_CREATE_TABLE_BLACK_PROCESS_CONNECT = "CREATE TABLE IF NOT EXISTS black_process_connect(id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp long, process TEXT, uid INTEGER)";
const char *SQL_CREATE_TABLE_IP_DATA = "CREATE TABLE IF NOT EXISTS network_ip_connect(id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp long, process TEXT, version INTEGER, src_ip TEXT, src_port INTEGER, dest_ip TEXT, dest_port INTEGER, protocol INTEGER)";
const char *SQL_CREATE_TABLE_DNS_DATA = "CREATE TABLE IF NOT EXISTS network_dns_connect(id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp long, process TEXT, dns TEXT, dest_ip TEXT)";
const char *SQL_CREATE_TABLE_REDIRECT_MAP = "CREATE TABLE IF NOT EXISTS network_redirect_map(id INTEGER PRIMARY KEY AUTOINCREMENT, dest_ip TEXT, redirect_ip TEXT)";
const char *SQL_CREATE_TABLE_NETWORK_MONITOR_CONFIG = "CREATE TABLE IF NOT EXISTS network_monitor_config(id INTEGER PRIMARY KEY AUTOINCREMENT, idpsmode INTEGER, period INTEGER, mobile_ifaces TEXT, wifi_iface Text, last_idpsmode_timestamp long,last_ip_timestamp long, last_dns_timestamp long, last_update_black_ip_timestamp long, last_update_black_dns_timestamp long, last_update_black_process_timestamp long, last_update_redirect_ip_timestamp long, last_update_black_port_timestamp long, last_update_file_timestamp long)";

const char *SQL_INSERT_FILELIST = "INSERT INTO network_filelist(file) ";

const char *SQL_INSERT_IP_BLACKLIST = "INSERT INTO network_ip_blacklist(black_ip) ";
const char *SQL_INSERT_PORT_BLACKLIST = "INSERT INTO network_port_blacklist(black_port) ";
const char *SQL_DELETE_IP_BLACKLIST = "DELETE FROM network_ip_blacklist ";
const char *SQL_DELETE_PORT_BLACKLIST = "DELETE FROM network_port_blacklist ";
const char *SQL_SELECT_ALL_IP_BLACKLIST = "SELECT * FROM network_ip_blacklist";
const char *SQL_SELECT_ALL_PORT_BLACKLIST = "SELECT * FROM network_port_blacklist";
const char *SQL_SELECT_ALL_FILELIST = "SELECT * FROM network_filelist";

const char *SQL_INSERT_DNS_BLACKLIST = "INSERT INTO network_dns_blacklist(black_dns) ";
const char *SQL_DELETE_DNS_BLACKLIST = "DELETE FROM network_dns_blacklist ";
const char *SQL_SELECT_ALL_DNS_BLACKLIST = "SELECT * FROM network_dns_blacklist";
const char *SQL_DELETE_FILELIST = "DELETE FROM network_filelist ";

const char *SQL_INSERT_PROCESS_BLACKLIST = "INSERT INTO network_process_blacklist(black_process) ";
const char *SQL_DELETE_PROCESS_BLACKLIST = "DELETE FROM network_process_blacklist ";
const char *SQL_SELECT_ALL_PROCESS_BLACKLIST = "SELECT * FROM network_process_blacklist";

const char *SQL_INSERT_REDIRECT_MAP = "INSERT INTO network_redirect_map(dest_ip, redirect_ip) ";
const char *SQL_DELETE_REDIRECT_MAP = "DELETE FROM network_redirect_map ";
const char *SQL_UPDATE_REDIRECT_MAP = "UPDATE network_redirect_map ";
const char *SQL_SELECT_ALL_REDIRECT_MAP = "SELECT * FROM network_redirect_map";

const char *SQL_INSERT_BLACK_IP_CONNECT = "INSERT INTO black_ip_connect(timestamp, process, version, src_ip, src_port, dest_ip, dest_port, protocol) ";
const char *SQL_INSERT_BLACK_DNS_CONNECT = "INSERT INTO black_dns_connect(timestamp, process, dns, dest_ip) ";
const char *SQL_INSERT_BLACK_PROCESS_CONNECT = "INSERT INTO black_process_connect(timestamp, process, uid) ";
const char *SQL_INSERT_IP_CONNECT = "INSERT INTO network_ip_connect(timestamp, process, version, src_ip, src_port, dest_ip, dest_port, protocol) ";
const char *SQL_SELECT_IP_CONNECT = "SELECT * FROM network_ip_connect WHERE ";
const char *SQL_INSERT_DNS_CONNECT = "INSERT INTO network_dns_connect(timestamp, process, dns, dest_ip) ";
const char *SQL_SELECT_DNS_CONNECT = "SELECT * FROM network_dns_connect WHERE ";

const char *SQL_INSERT_CONFIG = "INSERT INTO network_monitor_config(idpsmode, period, mobile_ifaces, wifi_iface, last_idpsmode_timestamp,last_ip_timestamp, last_dns_timestamp, last_update_black_ip_timestamp, last_update_black_dns_timestamp, last_update_black_process_timestamp, last_update_redirect_ip_timestamp, last_update_black_port_timestamp, last_update_file_timestamp) ";
static const char *SQL_UPDATE_CONFIG = "UPDATE network_monitor_config ";
static const char *SQL_SELECT_ALL_CONFIG = "SELECT * FROM network_monitor_config";

const char *SQL_STATISTICS_SELECT_IP_CONNECT = "select count(*) as num, max(timestamp) as tt , process, version, src_ip, src_port, dest_ip, dest_port, protocol from network_ip_connect ";
const char *SQL_STATISTICS_GROUP_IP_CONNECT = " group by version, src_ip, src_port, dest_ip, dest_port, protocol order by tt ASC";

const char *SQL_STATISTICS_SELECT_DNS_CONNECT = "select count(*) as num, max(timestamp) as tt, process,  dns,  dest_ip from network_dns_connect ";
const char *SQL_STATISTICS_GROUP_DNS_CONNECT = " group by dns, dest_ip order by tt ASC";

/**
 * @name:   习惯把密码明文存在本地文件中，这个小程序可以把存的密码以密文形式保存
 * @Author: qihoo360
 * @msg:    
 * @param   
 * @return: 
 */
int chartoasc(char c);
int xor(int i);
char asctochar(int a);
int rand_num();
int NetworkSql_encrypt(const char *org_pass,char *new_pass);
int NetworkSql_decrypt(const char *new_pass,char *org_pass);
/**
 * @name:   将字符转换为ASCII值
 * @Author: qihoo360
 * @msg:    
 * @param   
 * @return: 
 */
 
int chartoasc(char c)
{
    int i= 0;
    i = c;
    return i;
}
/**
 * @name:   将ASCII进行异或运算，产生新的ASCII值
 * @Author: qihoo360
 * @msg:    
 * @param   
 * @return: 
 */
int xor(int i)
{
    int m = 27;
    int result = 0;
    if(59==i || 100==i)
    {
        return i;
    }
    result = i^m;
    return result;
}

/**
 * @name:   将ASCII值转换为字符
 * @Author: qihoo360
 * @msg:    
 * @param   
 * @return: 
 */
char asctochar(int a)
{
    char c;
    c = a;
    return c;
}

/**
 * @name:   输入原密码产生新的密码
 * @Author: qihoo360
 * @msg:    
 * @param   
 * @return: 
 */
int NetworkSql_encrypt(const char *org_pass,char *new_pass)
{
    char org_password[250];
    char new_password[250];
    int len = 0;
    int i = 0;
    int asc = 0 ;
    char ch = 0;
    int x = 0;

    bzero(org_password,sizeof(org_password));
    bzero(new_password,sizeof(new_password));
    strcpy(org_password, org_pass);
    len = strlen(org_password);
    for(i=0 ; i<len ; i++)
    {
        ch = org_password[i];
        asc = chartoasc(ch);
        x = xor(asc);
        new_password[i] = asctochar(x);
    }
    strcpy(new_pass,new_password);

    return 0;
}
/**
 * @name:   输入加密后的密码返回原密码
 * @Author: qihoo360
 * @msg:    
 * @param   
 * @return: 
 */
int NetworkSql_decrypt(const char *new_pass,char *org_pass)
{
    char new_password[250];
    char org_password[250];
    char ch;
    int a = -1;
    int len =0;
    int i=0;
    int x = -1;

    bzero(new_password,sizeof(new_password));
    bzero(org_password,sizeof(org_password));

    strcpy(new_password,new_pass);
    len = strlen(new_password);
    for(i=0;i<len;i++)
    {
        ch = new_password[i];
        a = chartoasc(ch);
        x = xor(a);
        org_password[i]=asctochar(x);
    }
    strcpy(org_pass,org_password);

    return 0;
}
/********endcode**********/
/**
 * @name:   createtable
 * @Author: qihoo360
 * @msg:    networkmonitor related
 * @param   
 * @return: 
 */
void NetworkSql_createTable(void)
{
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_BLACK_DNS_CONNECT);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_BLACK_IP_CONNECT);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_BLACK_PROCESS_CONNECT);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_DNS_BLACKLIST);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_IP_BLACKLIST);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_PORT_BLACKLIST);
    sqliteMedthodobj.createTable(SQL_CREATE_TABLE_FILELIST);

	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_PROCESS_BLACKLIST);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_IP_DATA);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_DNS_DATA);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_REDIRECT_MAP);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_NETWORK_MONITOR_CONFIG);
}
/**
 * @name:   insertConfig
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
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
				  long long lastfileTimestamp)
{
	char l_sql[512] = {0}, l_insert[823] = {0};
	snprintf(l_sql, 512, "VALUES(%d, %d, '%s', '%s', %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld)", idpsmode,period, mobileIfaces, wifiIface,lastidpsTimestamp,
			 lastIpTimestamp, lastDnsTimestamp, lastUpdateBlackIpTimestamp, lastUpdateBlackDnsTimestamp, lastUpdateBlackProcessTimestamp, lastUpdateBlackRedirectIpTimestamp,lastUpdateBlackPortTimestamp,lastfileTimestamp);
	memcpy(l_insert, SQL_INSERT_CONFIG, strlen(SQL_INSERT_CONFIG));
	strcat(l_insert, l_sql);
	return sqliteMedthodobj.add(l_insert);
}
/**
 * @name:   updatePeriodConfig
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updatePeriodConfig(int period)
{
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "SET period = %d WHERE id = 1", period);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   updateIDPSConfig
 * @Author: qihoo360
 * @msg:    20210509
 * @param 
 * @return: 
 */
bool NetworkSql_updateIDPSConfig(int period){
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "SET idpsmode = %d WHERE id = 1", period);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   updateMobileIfacesConfig
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updateMobileIfacesConfig(char *_input)
{
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "SET mobile_ifaces = '%s' WHERE id = 1", _input);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   updateWifiIfaceConfig 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updateWifiIfaceConfig(char *wifiIface)
{
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "SET wifi_iface = '%s' WHERE id = 1", wifiIface);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   updateLastIpTimestamp 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updateLastIpTimestamp(long long timestamp)
{
	char sql[256] = {0}, fullsql[256] = {0};
	snprintf(sql, 256, "SET last_ip_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   updateLastidpsmodeTimestamp 
 * @Author: qihoo360
 * @msg:    20210509
 * @param 
 * @return: 
 */
bool NetworkSql_updateLastidpsmodeTimestamp(long long timestamp)
{
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "SET last_idpsmode_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
  * @name:   updateLastUpdateIpBlackListTimestamp
  * @Author: qihoo360
  * @msg: 
  * @param  
  * @return: 
  */
bool NetworkSql_updateLastUpdateIpBlackListTimestamp(long long timestamp)
{
	char sql[256] = {0}, fullsql[256] = {0};
	snprintf(sql, 256, "SET last_update_black_ip_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
  * @name:   updateLastUpdateIpBlackListTimestamp
  * @Author: qihoo360
  * @msg: 
  * @param  
  * @return: 
  */
bool NetworkSql_updateLastUpdatePortBlackListTimestamp(long long timestamp)
{
	char sql[256] = {0}, fullsql[256] = {0};
	snprintf(sql, 256, "SET last_update_black_port_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
  * @name:   updateLastUpdateDnsBlackListTimestamp
  * @Author: qihoo360
  * @msg: 
  * @param  
  * @return: 
  */
bool NetworkSql_updateLastUpdateDnsBlackListTimestamp(long long timestamp)
{
	char sql[256] = {0}, fullsql[256] = {0};
	snprintf(sql, 256, "SET last_update_black_dns_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   updateLastUpdateProcessBlackListTimestamp 
 * @Author: qihoo360
 * @msg: 
 * @param   
 * @return: 
 */
bool NetworkSql_updateLastUpdateProcessBlackListTimestamp(long long timestamp)
{
	char sql[256] = {0}, fullsql[256] = {0};
	snprintf(sql, 256, "SET last_update_black_process_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:  updateLastUpdateRedirectIpMapTimestamp 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updateLastUpdateRedirectIpMapTimestamp(long long timestamp)
{
	char sql[256] = {0}, fullsql[256] = {0};
	snprintf(sql, 256, "SET last_update_redirect_ip_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:  insertIpBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool NetworkSql_insertIpBlackList(uint8_t *ip,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, length, "VALUES('%s' )", ip);
	strcat(sql,"VALUES('");
	strcat(sql,ip);
	strcat(sql,"' )");
	strcat(fullsql, SQL_INSERT_IP_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:  insertfileList 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool NetworkSql_insertfileList(uint8_t *file,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	strcat(sql,"VALUES('");
	strcat(sql,file);
	strcat(sql,"' )");
	strcat(fullsql, SQL_INSERT_FILELIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   deletefileList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_deletefileList(uint8_t *ip,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	strcat(sql,"where file = '");
	strcat(sql,ip);
	strcat(sql,"'");

	strcat(fullsql, SQL_DELETE_FILELIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:  insertPortBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool NetworkSql_insertPortBlackList(uint8_t *ip,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, length, "VALUES('%s' )", ip);
	strcat(sql,"VALUES('");
	strcat(sql,ip);
	strcat(sql,"' )");
	strcat(fullsql, SQL_INSERT_PORT_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   deleteIpBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_deleteIpBlackList(uint8_t *ip,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, length, "where black_ip = '%s'", ip);
	
	strcat(sql,"where black_ip = '");
	strcat(sql,ip);
	strcat(sql,"'");

	strcat(fullsql, SQL_DELETE_IP_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:   deletePortBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_deletePortBlackList(uint8_t *ip,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, length, "where black_port = '%s'", ip);
	
	strcat(sql,"where black_port = '");
	strcat(sql,ip);
	strcat(sql,"'");

	strcat(fullsql, SQL_DELETE_PORT_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:   queryIpBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *NetworkSql_queryIpBlackList(void)
{
	list *head = NULL;
	char org_pass[256] = {0};
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor","fatal error,Out Of Space\n");
		return NULL;
	}
	list_init(head,free);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_IP_BLACKLIST);
	while (sqliteMedthodobj.next(l_stmt))
	{
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "black_ip");
		const char* ret = sqliteMedthodobj.getString(l_stmt, index);

        memset(org_pass,0,256);
		int result = NetworkSql_decrypt(ret,org_pass);
		if(result){
            log_i("networkmonitor","decode error");
		}
		else{
			char *node = (char *)malloc(strlen(org_pass) + 2);
			memset(node, 0, strlen(org_pass) + 2);
			memcpy(node, org_pass, strlen(org_pass));
			list_ins_next(head, NULL, node);
		}	 
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   queryfileList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *NetworkSql_queryfileList(void)
{
	list *head = NULL;
	char org_pass[512] = {0};
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor","fatal error,Out Of Space\n");
		return NULL;
	}
	list_init(head,free);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_FILELIST);
	while (sqliteMedthodobj.next(l_stmt))
	{
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "file");
		const char* ret = sqliteMedthodobj.getString(l_stmt, index);

        memset(org_pass,0,512);
		int result = NetworkSql_decrypt(ret,org_pass);
		if(result){
            log_i("networkmonitor","decode error");
		}
		else{
			char *node = (char *)malloc(strlen(org_pass) + 2);
			memset(node, 0, strlen(org_pass) + 2);
			memcpy(node, org_pass, strlen(org_pass));
			list_ins_next(head, NULL, node);
		}	 
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   queryPortBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *NetworkSql_queryPortBlackList(void)
{
	list *head = NULL;
	char org_pass[256] = {0};
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor","fatal error,Out Of Space\n");
		return NULL;
	}
	list_init(head,free);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_PORT_BLACKLIST);
	while (sqliteMedthodobj.next(l_stmt))
	{
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "black_port");
		const char* ret = sqliteMedthodobj.getString(l_stmt, index);
        memset(org_pass,0,256);
		int result = NetworkSql_decrypt(ret,org_pass);
		if(result){
		   log_i("networkmonitor","decode error");
		}
		else{
			char *node = (char *)malloc(strlen(org_pass) + 2);
			memset(node, 0, strlen(org_pass) + 2);
			memcpy(node, org_pass, strlen(org_pass));
			list_ins_next(head, NULL, node);
		}
	}
	//if(list_size(head) == 0){
	//	free(head);
	//	head = NULL;
	//}
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   insertDnsBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_insertDnsBlackList(uint8_t *dns,uint8_t len)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, len, "VALUES('%s' )", dns);
	strcat(sql,"VALUES('");
	strcat(sql,dns);
	strcat(sql,"' )");
	strcat(fullsql, SQL_INSERT_DNS_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   deleteDnsBlackList
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_deleteDnsBlackList(uint8_t *dns,uint8_t code)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, code, "where black_dns = '%s'", dns);

	strcat(sql,"where black_dns = '");
	strcat(sql,dns);
	strcat(sql,"'");

	strcat(fullsql, SQL_DELETE_DNS_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:   queryDnsBlackList
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *NetworkSql_queryDnsBlackList(void)
{
	char org_pass[256] = {0};
	list *head = NULL;
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head,free);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_DNS_BLACKLIST);
	while (sqliteMedthodobj.next(l_stmt))
	{
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "black_dns");
		const char *ret = sqliteMedthodobj.getString(l_stmt, index);
        memset(org_pass,0,256);
		int result = NetworkSql_decrypt(ret,org_pass);
		if(result){
		   log_i("networkmonitor","decode error");
		}
		else{
			char *node = (char *)malloc(strlen(org_pass) + 2);
			memset(node, 0, strlen(org_pass) + 2);
			memcpy(node, org_pass, strlen(org_pass));
			list_ins_next(head, NULL, node);
		}
	}
	sqliteMedthodobj.finalize(l_stmt);
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	return head;
}
/**
 * @name:   insertProcessBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_insertProcessBlackList(uint8_t *process,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, length, "VALUES('%s' )", process);
	strcat(sql,"VALUES('");
	strcat(sql,process);
	strcat(sql,"' )");
	strcat(fullsql, SQL_INSERT_PROCESS_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   deleteProcessBlackList 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_deleteProcessBlackList(uint8_t *process,uint8_t length)
{
	char sql[256] = {0}, fullsql[256] = {0};
	//snprintf(sql, length, "where black_process = '%s'", process);
	strcat(sql,"where black_process = '");
	strcat(sql,process);
	strcat(sql,"'");

	strcat(fullsql, SQL_DELETE_PROCESS_BLACKLIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:   queryProcessBlackList
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *NetworkSql_queryProcessBlackList(void)
{
	char org_pass[256] = {0};
	list *head = NULL;
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{  
		
		log_i("networkmonitor","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head,free);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_PROCESS_BLACKLIST);
	while (sqliteMedthodobj.next(l_stmt))
	{
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "black_process");
		const char *ret = sqliteMedthodobj.getString(l_stmt, index);
        memset(org_pass,0,256);
		int result = NetworkSql_decrypt(ret,org_pass);
		if(result){
		   log_i("networkmonitor ","decode error");
		}
		else{
			char *node = (char *)malloc(strlen(org_pass) + 2);
			memset(node, 0, strlen(org_pass) + 2);
			memcpy(node, org_pass, strlen(org_pass));
			list_ins_next(head, NULL, node);
		}
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   insertDirectMap 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_insertDirectMap(char *destIp, char *redirectIp)
{
	char sql[128] = {0}, fullsql[256] = {0};
	//snprintf(sql, 64, "VALUES('%s','%s' )", destIp, redirectIp);

	strcat(sql,"VALUES('");
	strcat(sql,destIp);
	strcat(sql,"','");
	strcat(sql,redirectIp);
	strcat(sql,"' )");

	strcat(fullsql, SQL_INSERT_REDIRECT_MAP);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   deleteDirectMap
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_deleteDirectMap(char *destIp, char *redirectIp)
{
	char sql[64] = {0}, fullsql[256] = {0};
	//snprintf(sql, 64, "where dest_ip = '%s' and redirect_ip = '%s'", destIp, redirectIp);

	strcat(sql,"where dest_ip = '");
	strcat(sql,destIp);
	strcat(sql,"' and redirect_ip = '");
	strcat(sql,redirectIp);
	strcat(sql,"'");

	strcat(fullsql, SQL_DELETE_REDIRECT_MAP);
	strcat(fullsql, sql);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:   updateDirectMap
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updateDirectMap(char *destIp, char *redirectIp)
{
	char sql[128] = {0}, fullsql[256] = {0};
	//snprintf(sql, 128, "SET redirect_ip = '%s' where dest_ip = '%s'", destIp, redirectIp);

	strcat(sql,"SET redirect_ip = '");
	strcat(sql,destIp);
	strcat(sql,"' where dest_ip = '");
	strcat(sql,redirectIp);
	strcat(sql,"'");

	strcat(fullsql, SQL_UPDATE_REDIRECT_MAP);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   queryDirectMap 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *NetworkSql_queryDirectMap(void)
{
	char org_pass[256] = {0},org_passre[256] = {0};
	list *head = NULL;
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor ","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head,free);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_REDIRECT_MAP);
	while (sqliteMedthodobj.next(l_stmt))
	{
		int dest_ip_index = sqliteMedthodobj.getColumnIndex(l_stmt, "dest_ip");
		const char *dest_ip_ret = sqliteMedthodobj.getString(l_stmt, dest_ip_index);

		int redirect_ip_index = sqliteMedthodobj.getColumnIndex(l_stmt, "redirect_ip");
		const char *redirect_ip_ret = sqliteMedthodobj.getString(l_stmt, redirect_ip_index);

        memset(org_pass,0,256);
		memset(org_passre,0,256);
		int ret = NetworkSql_decrypt(dest_ip_ret,org_pass);
		if(ret){
           log_i("networkmonitor ","decode error");
		}
		else{
			ret = NetworkSql_decrypt(redirect_ip_ret,org_passre);
			if(ret){
				log_i("networkmonitor ","decode error");
			}
			else{
				dnatstruct *input = (dnatstruct *)malloc(sizeof(dnatstruct));
				if (input == NULL)
				{
					log_i("networkmonitor ","Out Of Space");
					free(head);
					return NULL;
				}
				memset(input, 0, sizeof(dnatstruct));
				memcpy(input->dstip, org_pass, strlen(org_pass));
				memcpy(input->redirectIP, org_passre, strlen(org_passre));
				list_ins_next(head, NULL, input);
			}
		}
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   insertBlackIpConnectEvent
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_insertBlackIpConnectEvent(char *processName, int version, char *srcIp, int srcPort, char *destIp, int destPort, int protocol)
{
	char sql[128] = {0}, fullsql[256] = {0};
	long long timestamp = clockobj.get_current_time();
	snprintf(sql, 128, "VALUES(%lld, '%s', %d, '%s', %d, '%s', %d, %d )", timestamp, processName, version, srcIp, srcPort, destIp, destPort, protocol);
	strcat(fullsql, SQL_INSERT_BLACK_IP_CONNECT);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   insertBlackDnsConnectEvent 
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
bool NetworkSql_insertBlackDnsConnectEvent(char *processName, char *dns, char *destIp)
{
	char sql[128] = {0}, fullsql[256] = {0};
	long long timestamp = clockobj.get_current_time();
	snprintf(sql, 128, "VALUES(%lld, '%s', '%s', '%s' )", timestamp, processName, dns, destIp);
	strcat(fullsql, SQL_INSERT_BLACK_DNS_CONNECT);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   insertBlackProcessConnectEvent 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_insertBlackProcessConnectEvent(char *processName, int uid)
{
	char sql[128] = {0}, fullsql[256] = {0};
	long long timestamp = clockobj.get_current_time();
	snprintf(sql, 128, "VALUES(%lld, '%s', %d )", timestamp, processName, uid);
	strcat(fullsql, SQL_INSERT_BLACK_PROCESS_CONNECT);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   insertIpConnectData
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_insertIpConnectData(char *processName, int version, char *srcIp, int srcPort, char *destIp, int destPort, int protocol)
{
	char sql[128] = {0}, fullsql[256] = {0};
	long long timestamp = clockobj.get_current_time();
	//printf("insertIpConnectData timestamp %lld\n",timestamp);//need to delete
	snprintf(sql, 128, "VALUES(%lld, '%s', %d, '%s', %d, '%s', %d, %d )", timestamp, processName, version, srcIp, srcPort, destIp, destPort, protocol);
	strcat(fullsql, SQL_INSERT_IP_CONNECT);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   insertDnsConnectData 
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
bool NetworkSql_insertDnsConnectData(char *processName, char *dns, char *destIp)
{
	char sql[128] = {0}, fullsql[256] = {0};
	long long timestamp = clockobj.get_current_time();
	snprintf(sql, 128, "VALUES(%lld, '%s', '%s', '%s' )", timestamp, processName, dns, destIp);
	strcat(fullsql, SQL_INSERT_DNS_CONNECT);
	strcat(fullsql, sql);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   queryIpDataGte 
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
list *NetworkSql_queryIpDataGte(long long timestamp)
{
	list *head = NULL;
	char l_sql[256] = {0};
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor ","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head,free);
	snprintf(l_sql, 256, "%s%s%lld", SQL_SELECT_IP_CONNECT, "timestamp >= ", timestamp);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(l_sql);
	while (sqliteMedthodobj.next(l_stmt))
	{
		NetworkIpData *input = (NetworkIpData *)malloc(sizeof(NetworkIpData));
		if (input == NULL)
		{
			log_i("networkmonitor ","fatal error,Out Of Space");
			free(head);
			return NULL;
		}
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "timestamp");
		input->timestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "process");
		const char *l_processname = sqliteMedthodobj.getString(l_stmt, index);
		input->processName = (char *)malloc(strlen(l_processname) + 2);
		memset(input->processName, 0, strlen(l_processname) + 2);
		memcpy(input->processName, l_processname, strlen(l_processname));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "src_ip");
		const char *l_src_ip = sqliteMedthodobj.getString(l_stmt, index);
		input->srcIp = (char *)malloc(strlen(l_src_ip) + 2);
		memset(input->srcIp, 0, strlen(l_src_ip) + 2);
		memcpy(input->srcIp, l_src_ip, strlen(l_src_ip));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "src_port");
		input->srcPort = sqliteMedthodobj.getInt(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dest_ip");
		const char *l_dest_ip = sqliteMedthodobj.getString(l_stmt, index);
		input->destIp = (char *)malloc(strlen(l_dest_ip) + 2);
		memset(input->destIp, 0, strlen(l_dest_ip) + 2);
		memcpy(input->destIp, l_dest_ip, strlen(l_dest_ip));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dest_port");
		input->destPort = sqliteMedthodobj.getInt(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "protocol");
		input->protocol = sqliteMedthodobj.getInt(l_stmt, index);
		list_ins_next(head, NULL, input);
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   queryIpUploadDataGte 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: its a list point 
 */
list *NetworkSql_queryIpUploadDataGte(long long timestamp)
{
	list *head = NULL;
	char l_sql[512] = {0};
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor ","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head,free);
	snprintf(l_sql, 512, "%sWhere timestamp >= %lld%s", SQL_STATISTICS_SELECT_IP_CONNECT,timestamp,SQL_STATISTICS_GROUP_IP_CONNECT);
	//printf("the string >>%s\n",l_sql);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(l_sql);
	while (sqliteMedthodobj.next(l_stmt))
	{
		NetworkIpUploadData *input = (NetworkIpUploadData *)malloc(sizeof(NetworkIpUploadData));
		if (input == NULL)
		{
			log_i("networkmonitor ","fatal error,Out Of Space");
			free(head);
			sqliteMedthodobj.finalize(l_stmt);
			return NULL;
		}
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "tt");
		input->timestamp = sqliteMedthodobj.getLong(l_stmt, index);
		//printf("desc timestamp %lld\n",input->timestamp);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "num");
		input->num = sqliteMedthodobj.getInt(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "process");
		const char *l_processname = sqliteMedthodobj.getString(l_stmt, index);
		input->processName = (char *)malloc(strlen(l_processname) + 2);
		memset(input->processName, 0, strlen(l_processname) + 2);
		memcpy(input->processName, l_processname, strlen(l_processname));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "version");
		input->version = sqliteMedthodobj.getInt(l_stmt,index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "src_ip");
		const char *l_src_ip = sqliteMedthodobj.getString(l_stmt, index);
		input->srcIp = (char *)malloc(strlen(l_src_ip) + 2);
		memset(input->srcIp, 0, strlen(l_src_ip) + 2);
		memcpy(input->srcIp, l_src_ip, strlen(l_src_ip));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "src_port");
		input->srcPort = sqliteMedthodobj.getInt(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dest_ip");
		const char *l_dest_ip = sqliteMedthodobj.getString(l_stmt, index);
		input->destIp = (char *)malloc(strlen(l_dest_ip) + 2);
		memset(input->destIp, 0, strlen(l_dest_ip) + 2);
		memcpy(input->destIp, l_dest_ip, strlen(l_dest_ip));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dest_port");
		input->destPort = sqliteMedthodobj.getInt(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "protocol");
		input->protocol = sqliteMedthodobj.getInt(l_stmt, index);
		list_ins_next(head, NULL, input);
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   queryDnsDataGte
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *NetworkSql_queryDnsDataGte(long long timestamp)
{
	list *head = NULL;
	char l_sql[256] = {0};
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor ","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head,free);
	snprintf(l_sql, 256, "%stimestamp >= %lld", SQL_SELECT_DNS_CONNECT,timestamp);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(l_sql);
	while (sqliteMedthodobj.next(l_stmt))
	{
		NetworkDnsData *input = (NetworkDnsData *)malloc(sizeof(NetworkDnsData));
		if (input == NULL)
		{
			log_i("networkmonitor ","fatal error,Out Of Space");
			free(head);
			return NULL;
		}
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "timestamp");
		input->timestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "process");
		const char *l_processname = sqliteMedthodobj.getString(l_stmt, index);
		input->processName = (char *)malloc(strlen(l_processname) + 2);
		memset(input->processName, 0, strlen(l_processname) + 2);
		memcpy(input->processName, l_processname, strlen(l_processname));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dns");
		const char *l_dns = sqliteMedthodobj.getString(l_stmt, index);
		input->dns = (char *)malloc(strlen(l_dns) + 2);
		memset(input->dns, 0, strlen(l_dns) + 2);
		memcpy(input->dns, l_dns, strlen(l_dns));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dest_ip");
		const char *l_dest_ip = sqliteMedthodobj.getString(l_stmt, index);
		input->destIp = (char *)malloc(strlen(l_dest_ip) + 2);
		memset(input->destIp, 0, strlen(l_dest_ip) + 2);
		memcpy(input->destIp, l_dest_ip, strlen(l_dest_ip));
		list_ins_next(head, NULL, input);
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   queryDnsUploadDataGte 
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
list *NetworkSql_queryDnsUploadDataGte(long long timestamp)
{
	list *head = NULL;
	char l_sql[256] = {0};
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("networkmonitor ","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head,free);
	snprintf(l_sql, 256, "%sWHERE timestamp >= %lld%s", SQL_STATISTICS_SELECT_DNS_CONNECT,timestamp,SQL_STATISTICS_GROUP_DNS_CONNECT);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(l_sql);
	while (sqliteMedthodobj.next(l_stmt))
	{
		NetworkDnsUploadData *input = (NetworkDnsUploadData *)malloc(sizeof(NetworkDnsUploadData));
		if (input == NULL)
		{
			log_i("networkmonitor ","fatal error,Out Of Space");
			free(head);
			return NULL;
		}
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "tt");
		input->timestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "num");
		input->num = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "process");
		const char *l_processname = sqliteMedthodobj.getString(l_stmt, index);
		input->processName = (char *)malloc(strlen(l_processname) + 2);
		memset(input->processName, 0, strlen(l_processname) + 2);
		memcpy(input->processName, l_processname, strlen(l_processname));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dns");
		const char *l_dns = sqliteMedthodobj.getString(l_stmt, index);
		input->dns = (char *)malloc(strlen(l_dns) + 2);
		memset(input->dns, 0, strlen(l_dns) + 2);
		memcpy(input->dns, l_dns, strlen(l_dns));

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "dest_ip");
		const char *l_dest_ip = sqliteMedthodobj.getString(l_stmt, index);
		input->destIp = (char *)malloc(strlen(l_dest_ip) + 2);
		memset(input->destIp, 0, strlen(l_dest_ip) + 2);
		memcpy(input->destIp, l_dest_ip, strlen(l_dest_ip));
		list_ins_next(head, NULL, input);
	}
	#if 0
	if(list_size(head) == 0){
		free(head);
		head = NULL;
	}
	#endif
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   queryConfig
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
NetworkMonitorConfig *NetworkSql_queryConfig(void)
{
 
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_CONFIG);
	NetworkMonitorConfig *l_NetworkConfig = NetworkAdt_newobjectNetworkMonitorConfig();
	if(sqliteMedthodobj.next(l_stmt)){
		int index = sqliteMedthodobj.getColumnIndex(l_stmt,"period");
		l_NetworkConfig->period = sqliteMedthodobj.getInt(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"idpsmode");
		l_NetworkConfig->idpsmode = sqliteMedthodobj.getInt(l_stmt, index);
        
		index = sqliteMedthodobj.getColumnIndex(l_stmt,"mobile_ifaces");
		const char* mobile_ifaces = sqliteMedthodobj.getString(l_stmt, index);

		if(mobile_ifaces != NULL){
			l_NetworkConfig->mobileIfaces = (char *)malloc(strlen(mobile_ifaces) + 2);
			memset(l_NetworkConfig->mobileIfaces, 0, strlen(mobile_ifaces) + 2);
			memcpy(l_NetworkConfig->mobileIfaces, mobile_ifaces, strlen(mobile_ifaces));
		}
		else{
			l_NetworkConfig->mobileIfaces = (char *)malloc(SPACE_MALLOC);
			memset(l_NetworkConfig->mobileIfaces, 0, SPACE_MALLOC);
		}
		index = sqliteMedthodobj.getColumnIndex(l_stmt,"wifi_ifaces");
		const char* wifi_ifaces = sqliteMedthodobj.getString(l_stmt, index);
		if(wifi_ifaces == NULL){
			l_NetworkConfig->wifiIface = (char *)malloc(SPACE_MALLOC);
			memset(l_NetworkConfig->wifiIface, 0, SPACE_MALLOC);
		}else{
			l_NetworkConfig->wifiIface = (char *)malloc(strlen(wifi_ifaces) + 2);
			memset(l_NetworkConfig->wifiIface, 0, strlen(wifi_ifaces) + 2);
			memcpy(l_NetworkConfig->wifiIface, wifi_ifaces, strlen(wifi_ifaces));
		}

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_idpsmode_timestamp");
		l_NetworkConfig->lastidpsmodeTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_ip_timestamp");
		l_NetworkConfig->lastIpTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_dns_timestamp");
		l_NetworkConfig->lastIpTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_update_black_ip_timestamp");
		l_NetworkConfig->lastUpdateBlackIpTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_update_black_port_timestamp");
		l_NetworkConfig->lastUpdateBlackPortTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_update_black_dns_timestamp");
		l_NetworkConfig->lastUpdateBlackDnsTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_update_black_process_timestamp");
		l_NetworkConfig->lastUpdateBlackProcessTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_update_redirect_ip_timestamp");
		l_NetworkConfig->lastUpdateBlackRedirectIpTimestamp = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt,"last_update_file_timestamp");
		l_NetworkConfig->lastUpdatefileTimestamp = sqliteMedthodobj.getLong(l_stmt, index);
		
		sqliteMedthodobj.finalize(l_stmt);
		return l_NetworkConfig;
	}
	else
	{
		sqliteMedthodobj.finalize(l_stmt);
		free(l_NetworkConfig);
		return NULL;
	}
 
}
/**
 * @name:   updateLastDnsTimestamp
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updateLastDnsTimestamp(long long timestamp)
{
	char sql[128] = {0}, fullsql[256] = {0};
	snprintf(sql, 128, "SET  last_dns_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   updateLastfileTimestamp
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool NetworkSql_updateLastfileTimestamp(long long timestamp)
{
	char sql[128] = {0}, fullsql[256] = {0};
	snprintf(sql, 128, "SET  last_update_file_timestamp = '%lld' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}

#endif
