
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <idpslog.h>


int main()
{
    char  buff[256];
  	snprintf(buff, 256, "valid user info :%s", "1");
		//idpslog(0,"idps main",spdlog);
    idpslog_init(1,"./log.db",1000,2,
                  1,"./idpslog.db",
                    1,1000,2000);
    while (1)
    {
         idpslog(1,"main",buff);
         sleep(1);
    }
    
   
    return 0;
}