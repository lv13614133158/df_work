#include "libidslog.h"
#include <stdio.h>
#include <unistd.h>

void on_message_received(LOG_DATA *message)
{
    printf("Received message:\n");
    printf("  Source: %s \n", message->source);
    printf("  Level: %d\n", message->level);
    printf("  Type: %d\n", message->log_type);
    printf("  Tag: %s \n", message->log_tag);
    printf("  Date: %s\n", message->log_date);
    printf("  Data len: %d\n", message->data_len);
    printf("------------------------\n");
}

int main()
{
    if (ids_log_reader_init(on_message_received, 1) != 0) {
        fprintf(stderr, "Failed to initialize log reader\n");
        return -1;
    }
    
    printf("Log reader started, waiting for messages...\n");
    
    // 运行60秒等待消息
    for (;;) {
        sleep(1);
    }
    
    ids_log_reader_cleanup();
    return 0;
}