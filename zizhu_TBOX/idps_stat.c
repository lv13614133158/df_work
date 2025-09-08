#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CHECK_INTERVAL 5  // 检查间隔(秒)
#define TARGET_PROCESS "IDPS"  // 目标进程名
#define SCRIPT_NAME "process_monitor"  // 当前脚本名称

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

        // 如果没有找到目标进程，则启动它
        if (process_count == 0) {
            printf("%s START\n", TARGET_PROCESS);
            // 启动进程（假设IDPS在当前目录下）
            system("./IDPS &");
            // 启动后等待一段时间
            sleep(CHECK_INTERVAL);
        }

        // 等待下一个检查周期
        sleep(CHECK_INTERVAL);
    }

    return 0;
}