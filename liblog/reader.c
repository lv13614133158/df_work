#include "libidslog.h"
#include <stdio.h>
#include <unistd.h>

void on_message_received(const char* message, int len)
{
    printf("Received [%d bytes]: %.*s \n", len, len, message);
}

int main()
{
    if (ids_log_reader_init(on_message_received, 1) != 0) {
        fprintf(stderr, "Failed to initialize log reader\n");
        return -1;
    }
    
    printf("Log reader started, waiting for messages...\n");
    
    // 运行10秒等待消息
    while (1) {
        sleep(1);
    }
    
    ids_log_reader_cleanup();
    return 0;
}