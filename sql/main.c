#include "mysqlite.h"
#include "sqlite3.h"
#include <stdio.h>
#include <idpslog.h>
#include <unistd.h>


// 回调函数用于处理查询结果
static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    printf("Row data:\n");
    for(i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
int main()
{

    initSqliteDB("./idps.db", 0, 30);
    //idpslog_init(1, "./", 5*1024*1024, 2, 1, "./idps.db", 1, 5*1024*1024, 5);
    //createTable("create table if not exists test(id int primary key,name varchar(20))");
    
    // add("insert into IDPS_LOG ('log_type', 'log_content')values(1,'hello')");
    sql_select("select * from IDPS_LOG", callback, NULL);
    return 0;
}