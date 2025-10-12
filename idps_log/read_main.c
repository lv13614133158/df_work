#include "mysqlite.h"
#include "sqlite3.h"
#include <stdio.h>
#include <idpslog.h>
#include <unistd.h>
#include "openAes.h"
#include <string.h>
#include <stdlib.h>

char * key = "wq23456789abcde1";
char * iv = "wq23456789abcde1";
char * plaintext = NULL;
char * ciphertext = NULL;
int rt = 1;
int ase_len;


// 回调函数用于处理查询结果
static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    printf("Row data:  %d\n",argc);
    for(i = 0; i<argc; i++){
        if(i == 3) {
            // 检查argv[i]是否为NULL
            if (argv[i] == NULL) {
                printf("%s = NULL\n", azColName[i]);
                break;
            }
            
            // 分配内存并检查是否成功
            char *buff = (char *)malloc(strlen(argv[i])+1);
            if (buff == NULL) {
                printf("Memory allocation failed\n");
                break;
            }
            
            // 安全地复制字符串
            strcpy(buff, argv[i]);
            ase_len = strlen(buff);
            
            // 解密数据
            plaintext = aes_char_read(buff, ase_len);
            printf("%s = %s \n", azColName[i], buff);
            printf("\n");
            
            if (plaintext != NULL) {
                printf("%s = %s \n", azColName[i], plaintext);
            } else {
                printf("%s = (decryption failed)\n", azColName[i]);
            }
            free(plaintext);
            free(buff);
            break;
        }
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
int main()
{
    rt=aes_init(key,iv);
    if(rt != 0 )
    {
        printf("aes_init error error = %d \n",rt);
    }
   // initSqliteDB("./idps.db", 0, 30);
    initSqliteDB( "./idpslog.db", 5*1024*1024, 1000);
    //createTable("create table if not exists test(id int primary key,name varchar(20))");
    // char * data = "123456789QWERTYUIOPASDFGHJKLZXCVBNM";

    // ciphertext = aes_char_write(data);

    // insertLog(0,ciphertext);
    sql_select("select * from IDPS_LOG", callback, NULL);
    return 0;
}