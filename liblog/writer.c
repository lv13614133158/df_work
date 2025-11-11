#include "libidslog.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
        if(argc != 2)
        {
            return -1;
        }
        int i = atoi(argv[1]);
    while(1)
    {   
      
        LOG_DATA data;
        
        strncpy(data.source, "test", sizeof(data.source) - 1);
        data.source[sizeof(data.source) - 1] = '\0';
        data.level = i;
        data.log_type = 0;
        
        strncpy(data.log_tag, "tag_example", sizeof(data.log_tag) - 1);
        data.log_tag[sizeof(data.log_tag) - 1] = '\0';
        
        data.data_len = 5; 
        strncpy(data.log_date, "tag_example", sizeof(data.log_tag) - 1);
        int ret = ids_log_write(&data);
        printf("write ret %d    id   %d\n", ret,i);
        int sleep_time = rand() % 10 + 1;
        sleep(sleep_time);

    }

    

    return 0;
}