#include <sys/types.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "DataRecord.h"

static int mk_dir_exist(char *path)
{
    struct stat stat_buf; //useless, but ...
	if(stat(path, &stat_buf) == -1) {
		/*create dir*/
		mkdir(path, 0750);
        return 0;
	}
    return 1;
}

int writeVersion(char *path, char *version)
{
    int  ret =-1;
    char verPath[255] = {0};
    char readVer[255] ={0};
    if(!path || !version)
        return ret; 

    memcpy(verPath, path, strlen(path)<255 ? strlen(path):255);
    char *p = strrchr(verPath,'/');
    *p = 0; //去掉尾部
    mk_dir_exist(verPath);

    FILE* fp = fopen(path, "a+");
    if(fp)
    {
        size_t result = fread(readVer, 255, 1, fp);
        if(strcmp(readVer, version))
        {
            fclose(fp);
            fp = NULL;
            fp = fopen(path, "w+");
            if(fp){
                ret = fwrite(version, strlen(version), 1, fp);
                fflush(fp);
                fclose(fp);
            }   
        }
        else
        {
           fclose(fp);
           ret = 1; 
        } 
    }
    return ret;
}
