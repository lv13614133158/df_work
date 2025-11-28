#include "libidslog.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{

    LOG_DATA data;
    strncpy(data.source, "idslog", sizeof(data.source) - 1);
    data.source[sizeof(data.source) - 1] = '\0';
    data.level = 1; 
    data.log_type = LOG_DEFAULT; //默认0
    strncpy(data.log_tag, "tag", sizeof(data.log_tag) - 1);
    data.log_tag[sizeof(data.log_tag) - 1] = '\0';
    data.data_len = 5;//数据长度
    strncpy(data.log_date, "log_e", sizeof(data.log_tag) - 1);//数据内容
   
   
    int ret = ids_log_write(&data);
    printf("write ret %d \n", ret);

return 0;
}