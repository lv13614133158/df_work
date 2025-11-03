/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-08-02 21:41:27
 */ 
#include "mysqlite.h"
#include "common.h"
 /**
  * @name:   clockset clockobj
  * @Author: qihoo360
  * @msg: 
  * @param 
  * @return: 
  */
 clockset clockobj = {
   sync_clock,
   get_current_time,
 };
 /**
 * @name:   timerStruct  timerObj
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
timerStruct  timerObj = {
   newtime,
   starttime,
   stoptimer,
   freetimer,
   setInterval,
};
 /**
  * @name:   crypto cryptoobj
  * @Author: qihoo360
  * @msg: 
  * @param 
  * @return: 
  */
 crypto cryptoobj = {
    setWorkDirectory,
    storeKey,
    deleteKey,
    encryption,
    decryption,
 };
 /**
  * @name:    hmac hmacobj
  * @Author: qihoo360
  * @msg: 
  * @param 
  * @return: 
  */
 hmac hmacobj = {
   hmac_md5,
   Compute_string_md5,
   Compute_file_md5,  
 };

 /**
  * @name:   sqliteMedthod sqliteMedthodobj
  * @Author: qihoo360
  * @msg: 
  * @param 
  * @return: 
  */
 sqliteMedthod sqliteMedthodobj = {
   initDataBase,
   createTable, 
   add,
   del,
   update,
   query,
   finalize,
   getColumnCount,
   getColumnIndex,
   getCount,
   getInt,
   getString,
   getLong,
   next,
   reset,
   sqlitenolockpro, 
   sqliteLock,
   sqliteUnLock,
   queryNoLock,
   finalizeNoLock,
 };
 