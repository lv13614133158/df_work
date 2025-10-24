#ifndef __PROCESSINFO_H
#define __PROCESSINFO_H
#include "util.h"

typedef struct processNode{
    int pid;
	int ppid;
	int uid;
	int gid;
	char path[256];   //path + name
    struct processNode *next;
}processNode_t;

bool judgeWhiteList(list* list, char* name);
void append_process_list(processNode_t *h, int pid, int ppid, int uid, int gid, char *path);
int  search_process_list(processNode_t *h,char *path);
void destory_process_list(processNode_t *h);
void getProcessInfo(processNode_t *h);

#endif
