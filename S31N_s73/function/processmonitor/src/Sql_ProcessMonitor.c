/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-06 07:13:40
 */
#include "Sql_ProcessMonitor.h"
#include "ProcessMonitor.h"
#include "stdbool.h"
#include "common.h"
#include <stdlib.h>
#include "util.h"

#if MODULE_PROCESSMONITOR

/**
 * @name:   const val
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */

static const char *SQL_CREATE_TABLE_PROCESS_WHITE_LIST = "CREATE TABLE IF NOT EXISTS process_white_list(id INTEGER PRIMARY KEY AUTOINCREMENT, process_name TEXT)";
static const char *SQL_CREATE_TABLE_PROCESS_MONITOR_CONFIG = "CREATE TABLE IF NOT EXISTS process_monitor_config(id INTEGER PRIMARY KEY AUTOINCREMENT, period INTEGER, last_update_process_timestamp long)";

static const char *SQL_INSERT_PROCESS_WHITE_LIST = "INSERT INTO process_white_list(process_name) ";
static const char *SQL_DELETE_PROCESS_WHITE_LIST = "DELETE FROM process_white_list ";
static const char *SQL_SELECT_ALL_PROCESS_WHITE_LIST = "SELECT * FROM process_white_list ";

static const char *SQL_INSERT_CONFIG = "INSERT INTO process_monitor_config(period, last_update_process_timestamp) ";
static const char *SQL_UPDATE_CONFIG = "UPDATE process_monitor_config ";
static const char *SQL_SELECT_ALL_CONFIG = "SELECT * FROM process_monitor_config ";

/**
 * @name:   createTable
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void ProcessSql_createTable(void)
{
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_PROCESS_WHITE_LIST);
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_PROCESS_MONITOR_CONFIG);
}

bool ProcessSql_insertConfig(int period, long long lastUpdateProcessTimestamp)
{
	char l_sql[256] = {0}, l_insert[512] = {0};
	snprintf(l_sql, 256, "VALUES(%d,%lld)", period, lastUpdateProcessTimestamp);
	memcpy(l_insert, SQL_INSERT_CONFIG, strlen(SQL_INSERT_CONFIG));
	strcat(l_insert, l_sql);
	return sqliteMedthodobj.add(l_insert);
}

bool ProcessSql_updatePeriodConfig(int period)
{
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "SET period = %d WHERE id = 1", period);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}

bool ProcessSql_updateLastUpdateTimestamp(char* timestamp)
{
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "SET last_update_process_timestamp = '%s' WHERE id = 1", timestamp);
	strcat(fullsql, SQL_UPDATE_CONFIG);
	strcat(fullsql, sql);
	return sqliteMedthodobj.update(fullsql);
}

bool ProcessSql_insertWhiteList(char *processName)
{
	char l_sql[256] = {0}, l_insert[512] = {0};
	snprintf(l_sql, 256, "VALUES('%s' )", processName);
	memcpy(l_insert, SQL_INSERT_PROCESS_WHITE_LIST, strlen(SQL_INSERT_PROCESS_WHITE_LIST));
	strcat(l_insert, l_sql);
	return sqliteMedthodobj.add(l_insert);
}

bool ProcessSql_deleteWhiteList(char *processName)
{
	char sql[64] = {0}, fullsql[256] = {0};
	snprintf(sql, 64, "where process_name = '%s'", processName);
	strcat(fullsql, SQL_DELETE_PROCESS_WHITE_LIST);
	strcat(fullsql, sql);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:   queryConfig
 * @Author: qihoo360
 * @msg:    no matter sqlite have data or not
 * @param  
 * @return: 
 */
pProcessMonitorConfig ProcessSql_queryConfig(void)
{
	int column = 0;
	pProcessMonitorConfig pConfig = (pProcessMonitorConfig)malloc(sizeof(ProcessMonitorConfig));
	memset(pConfig, 0, sizeof(ProcessMonitorConfig));
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_CONFIG);
	if ((column = sqliteMedthodobj.getCount(l_stmt)) > 0)
	{
		if (sqliteMedthodobj.next(l_stmt))
		{
			int index = sqliteMedthodobj.getColumnIndex(l_stmt, "period");
			pConfig->period = sqliteMedthodobj.getInt(l_stmt, index);
			index = sqliteMedthodobj.getColumnIndex(l_stmt, "last_update_process_timestamp");
			pConfig->lastUpdateProcessTimestamp = sqliteMedthodobj.getLong(l_stmt, index);
			sqliteMedthodobj.finalize(l_stmt);
			return pConfig;
		}
	}
	sqliteMedthodobj.finalize(l_stmt);
	free(pConfig);
	return NULL;
}
/**
 * @name:   
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list *ProcessSql_queryProcessNameList(void)
{
	list *head = NULL;
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("processmonitor","fatal error,Out Of Space");
		return NULL;
	}
	//list_destroy(head);
	list_init(head, free);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_PROCESS_WHITE_LIST);
	//sqliteMedthodobj.finalize(l_stmt);

	while (sqliteMedthodobj.next(l_stmt))
	{
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "process_name");
		const char *strProcessName = sqliteMedthodobj.getString(l_stmt, index);

		char *strProcessNameCopy = (char *)malloc(strlen(strProcessName) + 2); //consider of \r\n according to caoyihua
		memset(strProcessNameCopy, 0, strlen(strProcessName) + 2);
		memcpy(strProcessNameCopy, strProcessName, strlen(strProcessName));
		list_ins_next(head, NULL, strProcessNameCopy);
	}
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}

#endif
