/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-23 04:54:42
 */ 
#include <string.h>
#include <stdlib.h>
#include "Sql_SysconfigMonitor.h"
#if 0
/**
 * @name:   const val
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
const char* SQL_CREATE_TABLE_SYSTEM_CONFIG = "CREATE TABLE IF NOT EXISTS system_config(id INTEGER PRIMARY KEY AUTOINCREMENT, data TEXT, create_time TIMESTAMP default (datetime('now', 'localtime')))";
const char* SQL_INSERT_SYSTEM_CONFIG = "INSERT INTO system_config(data) ";
const char* SQL_DELETE_SYSTEM_CONFIG = "DELETE FROM system_config ";
const char* SQL_SELECT_LATEST_SYSTEM_CONFIG = "SELECT * FROM system_config order by create_time DESC limit 1;";
/**
 * @name:   create
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void SysSql_CreateTable(void){
	sqliteMedthodobj.createTable(SQL_CREATE_TABLE_SYSTEM_CONFIG);
}
/**
 * @name:   insertSystemConfig
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool SysSql_insertSystemConfig(char* data){
	int l_tmplen = 0;
	if(data != NULL)
		l_tmplen = strlen(data);
	char fullsql[l_tmplen + strlen(SQL_INSERT_SYSTEM_CONFIG) + 32];
	memset(fullsql,0,l_tmplen + strlen(SQL_INSERT_SYSTEM_CONFIG) + 32);
	//snprintf(fullsql, 256, "%sVALUES('%s')",SQL_INSERT_SYSTEM_CONFIG,data);
	snprintf(fullsql, l_tmplen + strlen(SQL_INSERT_SYSTEM_CONFIG) + 32, "%sVALUES('%s')",SQL_INSERT_SYSTEM_CONFIG,data);
	return sqliteMedthodobj.add(fullsql);
}
/**
 * @name:   querySystemConfig
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
ConfigDBDataStruct* SysSql_querySystemConfig(void){
    ConfigDBDataStruct *retvalue = NULL;
	sqlite3_stmt *l_stmt = sqliteMedthodobj.query(SQL_SELECT_LATEST_SYSTEM_CONFIG);
	if(sqliteMedthodobj.next(l_stmt))
	{
		retvalue = (ConfigDBDataStruct* )malloc(sizeof(ConfigDBDataStruct));
		int index = sqliteMedthodobj.getColumnIndex(l_stmt, "id");
		retvalue->id = sqliteMedthodobj.getLong(l_stmt, index);

		index = sqliteMedthodobj.getColumnIndex(l_stmt, "data");
		const char* l_string = sqliteMedthodobj.getString(l_stmt,index);
		if(l_string != NULL){
			retvalue->data = (char* )malloc(strlen(l_string) + 2);
			memset(retvalue->data,0,strlen(l_string) + 2);
			memcpy(retvalue->data,l_string,strlen(l_string));
		}
		else{
			retvalue->data = (char *)malloc(SPACE_MALLOC);
			memset(retvalue->data, 0, SPACE_MALLOC);
		}
	} 
	sqliteMedthodobj.finalize(l_stmt);
	return retvalue;
}

#endif