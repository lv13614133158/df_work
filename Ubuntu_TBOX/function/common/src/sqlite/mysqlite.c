
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "mysqlite.h"
#include "sqlite3.h"
#include "spdloglib.h"

pthread_mutex_t sqliteMutex = PTHREAD_MUTEX_INITIALIZER;

static void free_errmsg(char *errmsg);
static sqlite3 *openDb(const char *dbpath);
static void closeDb(sqlite3* db);
static sqlite3 *_db=NULL;
static char _dataPath[256]={0};

void initDataBase(const char *dataPath)
{
    int ret;
    memset(_dataPath,0,sizeof(_dataPath));
    strncpy(_dataPath,dataPath,strlen(dataPath));
    ret = pthread_mutex_lock(&sqliteMutex);
    if(ret)
    {
        return ;
    }

    if(sqlite3_open(_dataPath,&_db)!=SQLITE_OK)
    {
        char spdlog[256] = {0};
        snprintf(spdlog,256,"%s %s %d %s\n",__FILE__,__func__,__LINE__,sqlite3_errmsg(_db));
        //printf("%s\n",spdlog);
        log_e("SqliteModule",spdlog);
    }
//    sqlite3_close(_db);
    pthread_mutex_unlock(&sqliteMutex);
}

/*************************************************************
 *  @func:增、删、改数据库
 *  @param sqlString 
 *  @return 
 *  @note 执行成功后不会关闭数据库，只有每次查询后调用finalize()方法
 *        才会关闭，原因：
 *          1、减小开关频率
 *          2、networkManager需要add 和query结合使用不能关闭
 * 
 ************************************************************/
bool excSqlNoCallBack(const char *sqlString)
{
    char *errMsg=NULL;
    int count = 5;
    do
    {
        int rv;
        int ret;
        ret = pthread_mutex_lock(&sqliteMutex);
        if(ret)
        {
            break;
        }
        if(_db == NULL)
        {
            if(sqlite3_open(_dataPath,&_db)!=SQLITE_OK)
            {
                sqlite3_close(_db);
                pthread_mutex_unlock(&sqliteMutex);
                count--;
                continue; 
            }
        }        
        rv = sqlite3_exec(_db,sqlString,0,0,&errMsg);
        if(rv != SQLITE_OK)
        {
            char spdlog[1024] = {0};
            snprintf(spdlog,1024,"%s %s %d %d %s\n",__FILE__,__func__,__LINE__,rv,errMsg);
            log_e("SqliteModule",spdlog);
            //printf("%s\n",spdlog);
            memset(spdlog,0,1024);
            snprintf(spdlog,1024,"sqlString:%s\n",sqlString);
            //printf("%s\n",spdlog);
            log_e("SqliteModule",spdlog);
            if(errMsg){
                free_errmsg(errMsg);
                errMsg=NULL;
            }
            sqlite3_close(_db);
            pthread_mutex_unlock(&sqliteMutex);
            count--;
            continue;
        }
        pthread_mutex_unlock(&sqliteMutex);
        return true;
    } while (count);
    return false; 
}


bool createTable(const char *sqlString)
{
    // sqlite3_busy_timeout(db,100);
    return excSqlNoCallBack(sqlString);
}

bool add(const char *sqlString)
{
    return excSqlNoCallBack(sqlString);
}

bool del(const char *sqlString)
{
    return excSqlNoCallBack(sqlString);
}

bool update(const char *sqlString)
{
    return excSqlNoCallBack(sqlString);
}

/*************************************************************
 *  @func:数据库查询
 *  @param sqlString 查询指令
 *  @return 失败返回NULL，成功返回 sqlite3_stmt指针
 *  @note 返回不为NULL，必须调用finalize()方法释放返回句柄
 * 
 ************************************************************/
sqlite3_stmt *query(const char *sqlString)
{   
    //加锁，防止增、删、改完毕后关闭数据库
    int ret;
    sqlite3_stmt *stmt = NULL;
    ret = pthread_mutex_lock(&sqliteMutex);
    if(ret)
    {
        return NULL;
    }
    if(_db == NULL)
    {
        if(sqlite3_open(_dataPath,&_db)!=SQLITE_OK)
        {
            sqlite3_close(_db);
            pthread_mutex_unlock(&sqliteMutex);
            return NULL; 
        }
    }  
    ret = sqlite3_prepare_v2(_db,sqlString,-1,&stmt,0);
    if(ret != SQLITE_OK)
    {
        char spdlog[256] = {0};
        snprintf(spdlog,256,"%s %s %d %s\n",__FILE__,__func__,__LINE__,sqlite3_errmsg(_db));
        // printf("%s\n",spdlog);
        log_e("SqliteModule",spdlog);
        sqlite3_close(_db);
        pthread_mutex_unlock(&sqliteMutex);
        return NULL;
    }
    return stmt;
}

/*************************************************************
 *  @func:释放数据库查询句柄，关闭数据库，释放线程锁
 *  @param stmt：查询返回的句柄
 *  @return NULL
 *  @note 查询完毕后必须调用该方法
 * 
 ************************************************************/
void finalize(sqlite3_stmt *stmt)
{     
    if(stmt !=NULL) 
        sqlite3_finalize(stmt);
    sqlite3_close(_db);
    _db = NULL;
    pthread_mutex_unlock(&sqliteMutex); 
}

/*
*   function:release malloc memory when error have
*
*/
static void free_errmsg(char *errmsg)
{
    if(errmsg != NULL)
        sqlite3_free(errmsg);
}

int getColumnCount(sqlite3_stmt *stmt)
{
    if(stmt == NULL)
        return -1;
    return sqlite3_column_count(stmt);
}

int getColumnIndex(sqlite3_stmt *stmt,const char* columName)
{
    if(stmt == NULL || columName == NULL)
        return -1;
    int column_count,i; 
    column_count = sqlite3_column_count(stmt);
    for(i = 0; i < column_count; i++)
    {
        const char *temp = sqlite3_column_name(stmt,i);
        if(strcmp(temp,columName) == 0)
            return i;
    }
}

int getCount(sqlite3_stmt *stmt)
{
    if(stmt == NULL)
        return -1;
    return sqlite3_data_count(stmt);
}

int getInt(sqlite3_stmt *stmt,int index)
{
    if(stmt && SQLITE_INTEGER == sqlite3_column_type(stmt,index))
    {
        return sqlite3_column_int(stmt,index);
    }
    return -1;
}

const char* getString(sqlite3_stmt *stmt,int index)
{
    if(stmt && SQLITE_TEXT == sqlite3_column_type(stmt , index)){
        const  char* value = (const char *)sqlite3_column_text(stmt,index);
        return value;
    }
    return NULL;
}
long long getLong(sqlite3_stmt *stmt,int index)
{
   if(stmt && SQLITE_INTEGER == sqlite3_column_type(stmt , index)){
        long long int value = sqlite3_column_int64(stmt,index);
        return value;
    }
    return -1;
}

bool next(sqlite3_stmt *stmt)
{
    if(stmt)
    {
        return sqlite3_step(stmt) == SQLITE_ROW;
    }
    return false;
}

bool reset(sqlite3_stmt *stmt)
{
    if(stmt){
        return sqlite3_reset(stmt) == SQLITE_OK;
    }
    return false;
}

bool sqlitenolockpro(const char *sqlString)
{
    char *errMsg = NULL;
    int rv;
    int count = 5;
    while(count)
    {
        if(_db == NULL)
        {
            if(sqlite3_open(_dataPath,&_db)!=SQLITE_OK)
            {
                sqlite3_close(_db);
                count--;
                continue; 
            }
        }  
        rv = sqlite3_exec(_db,sqlString,0,0,&errMsg);
        if(rv != SQLITE_OK)
        {
            char spdlog[512] = {0};
            snprintf(spdlog,512,"%s %s %d %s\n",__FILE__,__func__,__LINE__,errMsg);
            // printf("%s\n",spdlog);
            log_e("SqliteModule",spdlog);
            memset(spdlog,0,512);
            snprintf(spdlog,512,"sqlString:%s\n",sqlString);
            // printf("%s\n",spdlog);
            log_e("SqliteModule",spdlog);
            if(errMsg)
                free_errmsg(errMsg);
            count--;
            continue;
        }
        return true;
    }
    return false;
}

sqlite3_stmt *queryNoLock(const char *sqlString)
{
    int ret;
    sqlite3_stmt *stmt = NULL;
    if(_db == NULL)
    {
        if(sqlite3_open(_dataPath,&_db)!=SQLITE_OK)
        {
            sqlite3_close(_db);
            return NULL; 
        }
    }  
    ret = sqlite3_prepare_v2(_db,sqlString,-1,&stmt,0);
    if(ret != SQLITE_OK)
    {
        char spdlog[256] = {0};
        snprintf(spdlog,256,"%s %s %d %s\n",__FILE__,__func__,__LINE__,sqlite3_errmsg(_db));
        // printf("%s\n",spdlog);
        log_e("SqliteModule",spdlog);
        sqlite3_close(_db);
        return NULL;
    }
    return stmt;
}

void finalizeNoLock(sqlite3_stmt *stmt)
{
    if(stmt !=NULL) 
        sqlite3_finalize(stmt);
    sqlite3_close(_db);
    _db = NULL;
}

bool sqliteLock()
{
    int  ret;
    ret = pthread_mutex_lock(&sqliteMutex);
    if(ret)
    {
        return false;
    }
    return true;
}

void sqliteUnLock()
{
    pthread_mutex_unlock(&sqliteMutex);
}