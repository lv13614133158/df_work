#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CHECK_INTERVAL 5  
#define TARGET_PROCESS "IDPS"  
#define IDPS_PATH "/dfdata/output/bin/IDPS &" 
#define SCRIPT_NAME "idps_start" 

int main() {
    char command[256];
    char buffer[16];
    int process_count;
    FILE *fp;

    printf("Process monitor for %s started...\n", TARGET_PROCESS);

    while(1) {
        // 构造检查进程是否存在的命令，排除自身和grep进程
        snprintf(command, sizeof(command), 
                 "ps | grep %s | grep -v grep |grep -v bash| grep -v %s | wc -l", 
                 TARGET_PROCESS, SCRIPT_NAME);

        // 执行命令并读取结果
        fp = popen(command, "r");
        if (fp == NULL) {
             perror("popen failed");
             sleep(CHECK_INTERVAL);
             continue;
        }

        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            process_count = atoi(buffer);
        } else {
            process_count = 0;
        }
        
        pclose(fp);

  
        if (process_count == 0) {
            printf("%s START\n", TARGET_PROCESS);
            
            system(IDPS_PATH);
            
            sleep(CHECK_INTERVAL);
        }
        sleep(CHECK_INTERVAL);
    }

    return 0;
}