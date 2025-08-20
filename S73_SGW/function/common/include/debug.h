#ifndef __DEBUG_H
#define __DEBUG_H
#include <errno.h>
#include <string.h>

#define __PRINTLOG__  
#ifdef __PRINTLOG__  
#define DPRINTF(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)  
#else  
#define DPRINTF(format,...)  
#endif

#define __ERRORLOG__
#ifdef __ERRORLOG__  
#define DERROR(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)  
#else  
#define DERROR(format,...)  
#endif

#endif
