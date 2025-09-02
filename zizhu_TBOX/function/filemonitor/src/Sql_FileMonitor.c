/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-21 23:55:26
 */ 
#include "util.h"
#include <string.h>
#include <unistd.h>
#include "Sql_FileMonitor.h"

#if MODULE_FILEMONITOR
/**
 * @name:   const val
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
const char* SQL_CREATE_TABLE_FILE_MONITOR_CONFIG = "CREATE TABLE IF NOT EXISTS file_monitor_config(id INTEGER PRIMARY KEY AUTOINCREMENT, last_update_watch_point_timestamp INTEGER)";
const char* SQL_CREATE_TABLE_WATCH_POINT = "CREATE TABLE IF NOT EXISTS file_watch_point(id INTEGER PRIMARY KEY AUTOINCREMENT, file_path TEXT ,  mask INTEGER)";
const char* SQL_CREATE_TABLE_FILE_EVENT = "CREATE TABLE IF NOT EXISTS file_event(id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp long ,file_path TEXT ,  mask INTEGER , event_source TEXT)";
const char* SQL_INSERT_FILE_MONITOR_CONFIG = "INSERT INTO file_monitor_config(last_update_watch_point_timestamp)";
const char* SQL_INSERT_WATCH_POINT = "INSERT INTO file_watch_point(file_path, mask) ";
const char* SQL_DELETE_WATCH_POINT = "DELETE FROM file_watch_point ";
const char* SQL_UPDATE_WATCH_POINT = "UPDATE file_watch_point ";
const char* SQL_SELECT_ALL_WATCH_POINT = "SELECT * FROM file_watch_point";
const char* SQL_INSERT_FILE_EVENT = "INSERT INTO file_event(timestamp, file_path, mask, event_source) ";
static const char* SQL_SELECT_ALL_CONFIG = "SELECT * FROM file_monitor_config";
static const char* SQL_UPDATE_CONFIG = "UPDATE file_monitor_config ";
/**
 * @name:   createTable
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void FileSql_CreateTable(void){
    sqliteMedthodobj.createTable(SQL_CREATE_TABLE_FILE_MONITOR_CONFIG);
    sqliteMedthodobj.createTable(SQL_CREATE_TABLE_WATCH_POINT);
    sqliteMedthodobj.createTable(SQL_CREATE_TABLE_FILE_EVENT);
}
/**
 * @name:   insertFileMonitorConfig   
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool FileSql_insertFileMonitorConfig(long long lastUpdateWatchPointMapTimestamp){
	char fullsql[256] = {0};
	snprintf(fullsql, 256, "%sVALUES('%lld' )",SQL_INSERT_FILE_MONITOR_CONFIG,lastUpdateWatchPointMapTimestamp);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   updateLastUpdateWatchPointTimestamp 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
bool FileSql_updateLastUpdateWatchPointTimestamp(long long timestamp){
    char fullsql[256] = {0};
	snprintf(fullsql, 256, "%sSET last_update_watch_point_timestamp = %lld WHERE id = 1",SQL_UPDATE_CONFIG, timestamp);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   queryConfig
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
long FileSql_queryConfigsql(void){
    long retvalue = 0;
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_CONFIG);

	if(sqliteMedthodobj.next(l_stmt))
	{
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "last_update_watch_point_timestamp");
		retvalue = sqliteMedthodobj.getInt(l_stmt, index);
	}
	sqliteMedthodobj.finalize(l_stmt);
	return retvalue;
}
/**
 * @name:   insertWatchPoint 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool FileSql_insertWatchPoint(char* path, int mask){
    char fullsql[256] = {0};
	snprintf(fullsql, 256, "%sVALUES('%s', %d)",SQL_INSERT_WATCH_POINT,path,mask);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name: 
 * @Author: qihoo360
 * @msg: 
 * @param {type} 
 * @return: 
 */
bool FileSql_deleteWatchPoint(char* path, int mask){
	char fullsql[256] = {0};
	snprintf(fullsql, 256, "%swhere file_path = '%s' and mask = %d",SQL_DELETE_WATCH_POINT,path,mask);
	return sqliteMedthodobj.del(fullsql);
}
/**
 * @name:   updateWatchPoint 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool FileSql_updateWatchPointsql(char* path, int mask){
    char fullsql[256] = {0};
	snprintf(fullsql, 256, "%s SET mask = %d where file_path = '%s'",SQL_UPDATE_WATCH_POINT, mask,path);
	return sqliteMedthodobj.update(fullsql);
}
/**
 * @name:   queryWatchPoint
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
list* FileSql_queryWatchPoint(void){
	list *head = NULL;
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_i("sql filemonitor","fatal error,Out Of Space");
		return NULL;
	}	
	list_init(head,free);
	//list_destroy(head);
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_ALL_WATCH_POINT);

	while (sqliteMedthodobj.next(l_stmt))
	{
		filestruct *node = (filestruct* )malloc(sizeof(filestruct));
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "file_path");
		const char* path = sqliteMedthodobj.getString(l_stmt, index);
		node->path = (char *)malloc(strlen(path) + 2);
		memset(node->path, 0, strlen(path) + 2);
		memcpy(node->path, path, strlen(path));
		index = sqliteMedthodobj.getColumnIndex(l_stmt, "mask");
		node->mask = sqliteMedthodobj.getInt(l_stmt, index);
		list_ins_next(head, NULL, node);
	}
	sqliteMedthodobj.finalize(l_stmt);
	return head;
}
/**
 * @name:   insertFileEvent 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool FileSql_insertFileEvent(char *eventSource,char* path, int mask){
    char fullsql[256] = {0};
    long long timestamp = clockobj.get_current_time();
	snprintf(fullsql, 256, "%sVALUES(%lld, '%s', %d,'%s' )",SQL_INSERT_FILE_EVENT,timestamp,path,mask,eventSource);
	return sqliteMedthodobj.add(fullsql);
}

#endif