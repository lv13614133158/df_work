#include "liblog.h"
#include <stdio.h>
#include <unistd.h>

int main()
{
    if (ids_log_init() != 0) {
        fprintf(stderr, "Failed to initialize log writer\n");
        return -1;
    }
   for (int i = 0;i<10;i++)
   {
    sleep(1);
    
    int ret =ids_log_write("Another message1111", 19);
    printf("write ret %d\n", ret);
   }
    

    
    ids_log_cleanup();
    return 0;
}