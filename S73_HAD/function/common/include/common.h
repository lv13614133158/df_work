/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-08-02 21:41:49
 */ 
#ifndef __COMMON_H
#define __COMMON_H
#ifdef __cplusplus
extern "C" {
#endif
#include "mysqlite.h"
#include "CommonTimer.h"
#include "clocker.h"
#include "MyCrypto.h"
#include "myHmac.h"
/**
 * @name:    if no data
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
#define SPACE_MALLOC         48
/**
 * @name:   clock related method
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
typedef struct _clockset{
    void (*sync_clock)(long long);
    long long (*get_current_time)();
}clockset;
extern clockset clockobj;
/**
 * @name:   timer obj function
 * @Author: qihoo360
 * @msg: 
 * @param    
 * @return: 
 */
typedef struct _timerStruct{
   timerIdps *(*newtime)(void*);
   void (*starttime)(timerIdps*);
   void (*stoptimer)(timerIdps*);
   void (*freetimer)(timerIdps*);
   void (*setInterval)(int,timerIdps*);
}timerStruct;
extern timerStruct  timerObj;
/**
 * @name:   cryptoobj
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
typedef struct __crypto{
    int (*setWorkDirectory)(const char*);
    bool (*storeKey)(int,const char*,char*);
    bool (*deleteKey)(char*);
    cypher_t*(*encryption)(cypher_t*,char*);
    cypher_t*(*decryption)(cypher_t*,char*);
}crypto;
extern crypto cryptoobj;
/**
 * @name:   hmacobj
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
typedef struct _hmac{
    void (*hmac_md5)(unsigned char*, int,unsigned char*, int, unsigned char*);
    int  (*Compute_string_md5)(const char*, unsigned int, char*);
    int (*Compute_file_md5)(const char *, char *);  
}hmac;
extern hmac hmacobj;
/**
 * @name:   
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
typedef struct _sqliteMedthod{
    void (*initDataBase)(const char *);  
    bool (*createTable)(const char*);
    bool (*add)(const char*);
    bool (*del)(const char*);
    bool (*update)(const char*);
    sqlite3_stmt *(*query)(const char*);
    void (*finalize)(sqlite3_stmt*);
    int (*getColumnCount)(sqlite3_stmt*);
    int (*getColumnIndex)(sqlite3_stmt*,const char*);
    int (*getCount)(sqlite3_stmt*);
    int (*getInt)(sqlite3_stmt*,int);
    const char* (*getString)(sqlite3_stmt*,int);
    long long (*getLong)(sqlite3_stmt*,int);
    bool (*next)(sqlite3_stmt*);
    bool (*reset)(sqlite3_stmt*);
    bool (*sqlitenolockpro)(const char *);
    bool (*sqliteLock)();
    void (*sqliteUnLock)();
    sqlite3_stmt *(*queryNoLock)(const char *);
    void (*finalizeNoLock)(sqlite3_stmt *);
}sqliteMedthod;
extern sqliteMedthod sqliteMedthodobj;
#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif
#endif
