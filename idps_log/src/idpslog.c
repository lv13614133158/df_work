#include <idpslog.h>


int  fileflag = 0;
int  dbflag = 0;
int  aesflag = 0;

// 添加替换函数
char* replace_quotes(const char* input) {
    if (!input) return NULL;
    
    int len = strlen(input);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    
    for (int i = 0; i <= len; i++) {
        if (input[i] == '"') {
            result[i] = '\'';
        } else {
            result[i] = input[i];
        }
    }
    
    return result;
}


/**
 * @brief 初始化IDPS日志系统
 * 
 * 该函数用于初始化IDPS（入侵检测防护系统）的日志记录功能，
 * 支持文件日志和数据库日志两种存储方式的配置
 * 
 * @param file_enb 文件日志启用标志，非0表示启用文件日志，0表示禁用
 * @param filepath 文件日志存储路径，当fileflag启用时有效
 * @param max_file_size 单个日志文件的最大大小（单位：字节）
 * @param max_files 最大日志文件数量，用于日志轮转控制
 * @param db_enb 数据库日志启用标志，非0表示启用数据库日志，0表示禁用
 * @param dbpath 数据库文件路径，当dbflag启用时有效
 * @param dbaesflag 数据库加密标志，非0表示启用AES加密，0表示不加密
 * @param max_db_size 数据库文件最大大小限制 （暂时使用默认）
 * @param max_db 数据库保存数据id最大大小限制 
 * 
 * @return 无
 */
void idpslog_init(int file_enb, const char * filepath,int max_file_size,
                 int max_files,int  db_enb,const char *  dbpath,
                 int  dbaesflag, int max_db_size,int max_db)
{
    int rt;
    if(file_enb)
    {
        set_console_logger(true);     // 客户端打印
        set_file_write_logger(true);  // 写入文件
        set_file_logger(filepath, max_file_size, max_files);  //设置日志文件存储路径、大小、滚动个数
        set_level(0);
        fileflag=1;
    }

    if(db_enb)
    {
        //数据库日志
        initSqliteDB(dbpath,max_db_size,max_db);
        dbflag=1;
    }
    if(dbaesflag)
    {
        rt = aes_init(AES_KEY,AES_IV);
        if(rt != 0 )
        {
            printf("aes_init error error = %d \n",rt);
        }
        aesflag=1;
    }

  

}

/**
 * 记录日志信息到文件和数据库
 * @param type 日志类型：0-verbose, 1-debug, 2-info, 3-warn, 4-error, 5-assert
 * @param tag 日志标签，用于标识日志来源
 * @param msg 日志消息内容
 */
void idpslog(int type,const char *tag,const char *msg)
{
    char * mgs_data;
    char * aes_data;
    int msg_len; 
    char *sql;
    msg_len=strlen(msg);
    mgs_data = (char*)malloc(msg_len+1);
    memcpy(mgs_data,msg,msg_len);

    if(fileflag)
    {
        //文件日志
        if(type == 0){
            log_v(tag,mgs_data);
        }else if(type==1){
            log_d(tag,mgs_data);
        }else if(type==2){
            log_i(tag,mgs_data);
        }else if(type==3){
            log_w(tag,mgs_data);
        }else if(type==4){
            log_e(tag,mgs_data);
        }else if(type==5){
            log_a(tag,mgs_data);
        }
           
    }
    if(aesflag)
    {
        
         aes_data = aes_char_write(mgs_data);
         msg_len=strlen(aes_data);
         if (mgs_data)
         {
            free(mgs_data);
         }
         mgs_data=aes_data;
    }

    if(dbflag)
    {
        //数据库日志

        sql=replace_quotes(mgs_data);
        if(sql == NULL)
        {  
            return;
        }
        insertLog(type,sql);
        if(sql == NULL )
        {
            free(sql);
        }

    }

    if (mgs_data)
    {
        free(mgs_data);
    }

}
