#ifndef IDPSLOG_H
#define IDPSLOG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mysqlite.h"
#include "spdloglib.h"


void idpslog_init(int file_enb, const char * filepath,int max_file_size,
                 int max_files,int  db_enb,const char *  dbpath,
                 int  dbaesflag, int max_db_size,int max_db);


void idpslog(int type,const char *tag,const char *msg);

#ifdef __cplusplus
}
#endif
#endif