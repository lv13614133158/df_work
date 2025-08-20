#ifndef __MYSQLITE_H
#define __MYSQLITE_H
#include <stdbool.h>
#include "sqlite3.h"

/************************************************************
 *  @func:初始化数据库，设置数据库文件路径，以及初始化文件最大值及数据库表最大值
 *  @param filePath:数据库文件路径
 *  @param maxSize:数据库文件最大值，默认0，当为0时不设置最大值
 *  @param maxRows:数据库表最大行数，默认0，当为0时不设置最大值
 *  @return bool
 *  @note：数据库操作前必须调用该方法
 ************************************************************/
bool initSqliteDB(const char* filePath, int maxSize, int maxRows);

/************************************************************
 *  @func:向表中插入日志数据
 *  @param logType:日志类型（level）
 *  @param logText:日志内容
 *  @return bool
 *  @note：数据库操作前必须调用该方法
 ************************************************************/
bool insertLog(int logType, const char* logText);


//xuewenliang  create  2020/5/25
//***********  modify  2020/7/1
void initDataBase(const char *dataPath);

static bool createTable(const char *sqlString);

bool add(const char *sqlString);

static bool del(const char *sqlString);

static bool update(const char *sqlString);

bool sql_select(const char *sqlString, int (*callback)(void*,int,char**,char**), void *data);

/*************************************************************
 *  @func:数据库查询
 *  @param sqlString 查询指令
 *  @return 失败返回NULL，成功返回 sqlite3_stmt指针
 *  @note 返回不为NULL，必须调用finalize()方法释放返回句柄
 * 
 ************************************************************/
sqlite3_stmt *query(const char *sqlString);

/*************************************************************
 *  @func:释放数据库查询句柄，关闭数据库，释放线程锁
 *  @param stmt：查询返回的句柄
 *  @return NULL
 *  @note 查询完毕后必须调用该方法
 * 
 ************************************************************/
void finalize(sqlite3_stmt *stmt);

/*************************************************************
 *  @func:有或没有结果总会返回列数
 *  @param stmt：句柄
 *  @return NULL
 *  @note 
 * 
 ************************************************************/
int getColumnCount(sqlite3_stmt *stmt);

/*************************************************************
 *  @func:返回当前执行的语句stmt指向的当前行可用值数，没有则返回0
 *  @param stmt：句柄
 *  @return NULL
 *  @note 
 * 
 ************************************************************/
int getCount(sqlite3_stmt *stmt);

int getColumnIndex(sqlite3_stmt *stmt,const char* columName);
int getInt(sqlite3_stmt *stmt,int index);
const char* getString(sqlite3_stmt *stmt,int index);
long long getLong(sqlite3_stmt *stmt,int index);

/*************************************************************
 *  @func:指向下一行
 *  @param stmt：句柄
 *  @return NULL
 *  @note 
 * 
 ************************************************************/
bool next(sqlite3_stmt *stmt);
bool reset(sqlite3_stmt *stmt);



/*************************************************************
 *  @func:附加接口，主要用于networkManager模块，用于解决查询中执行增
 *        删、改功能
 *  @param sqlString:sql语句
 *  @return bool
 *  @note :该模块在networkmanager模块调用，其他模块调用正常接口,
 *          该接口去掉了线程锁，仅在query（）方法中调用
 * 
 ************************************************************/
bool sqlitenolockpro(const char *sqlString);

/*************************************************************
 *  @func:数据库查询 内部不加锁，可以和sqlitenolockpro()配合使用
 *  @param sqlString 查询指令
 *  @return 失败返回NULL，成功返回 sqlite3_stmt指针
 *  @note 返回不为NULL，必须调用finalizeNoLock()方法释放返回句柄
 * 
 ************************************************************/
sqlite3_stmt *queryNoLock(const char *sqlString);

/*************************************************************
 *  @func:释放数据库查询句柄，关闭数据库,
 *  @param stmt：查询返回的句柄
 *  @return NULL
 *  @note queryNoLock()查询完毕后必须调用该方法
 * 
 ************************************************************/
void finalizeNoLock(sqlite3_stmt *stmt);

/*************************************************************
 *  @func:获取数据库线程锁,module层方法调用
 *  @param sqlString 
 *  @return 
 *  @note 主要给monitor模块使用，配合sqlitenolockpro()和
 *        queryNoLock()方法使用，保证这两个方法为原子操作，
 * 
 ************************************************************/
bool sqliteLock();

/*************************************************************
 *  @func:释放数据库线程锁,module层方法调用
 *  @param sqlString 
 *  @return 
 *  @note 和sqliteLock()方法匹配，调用sqliteLock()后必须调用该方法
 *          否则造成死锁
 * 
 ************************************************************/
void sqliteUnLock();

#endif