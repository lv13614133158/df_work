// 进程获取
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include "cJSON.h"
#include "common.h"
#include "fileOption.h"
#include <pwd.h>
#include "util.h"
#include "spdloglib.h"
#include "processInfo.h"


#define PATH_PROC	   		"/proc"
#define PATH_CMDLINE		"cmdline"
#define PATH_STATUS			"status"
#define PATH_PROC_CMDLINE    PATH_PROC "/%s/" PATH_CMDLINE
#define PATH_PROC_STATUS     PATH_PROC "/%s/" PATH_STATUS


//拦截白名单里内容不上报
bool judgeWhiteList(list* list, char *name)
{
	list_elmt *element = list->head;
	while(element)
	{
		if(strcmp(element->data, name) == 0){
			return true;
		}
		element = element->next;
	}
	return false;
}

void append_process_list(processNode_t *h, int pid, int ppid, int uid, int gid, char *path)
{
	// printf("name:%s pid:%d ppid:%d uid:%d gid:%d\n\n",path,pid,ppid,uid,gid);
	processNode_t **temp,*newNode;
	newNode = malloc(sizeof(processNode_t));
	if(newNode == NULL)
	{
		//DERROR("malloc error\n");
		return; 
	}

	newNode->pid = pid;
	newNode->ppid = ppid;
	newNode->uid = uid;
	newNode->gid = gid;
	memset(newNode->path,0,256);
	strncpy(newNode->path,path,255);
	newNode->next = NULL;

	temp = &h->next;
	while(*temp!=NULL)
		temp = &(*temp)->next;
	*temp = newNode;
	h->pid ++;  //表示当前进程数量
}

int  search_process_list(processNode_t *h,char *path)
{
	processNode_t *i;
	i = h->next;
	while(i != NULL)
	{
		if(strcmp(i->path,path) == 0)
			return 1;
		i = i->next;
 	}
	return 0;
}

void destory_process_list(processNode_t *h)
{
	processNode_t *i,*temp;
	i = h->next;
	while(i != NULL)
	{
		temp = i -> next;
		free(i);
		i = temp;
 	}
	h->next = NULL;
	h->pid = 0;
}

// 检测的进程放入pHead2
void getProcessInfo(processNode_t *h)
{
	char line[2048];
	FILE *fp = NULL;
	int fd,cmdllen;
	char cmdlbuf[512],statusbuf[512],finbuf[256];
	const char *cs = NULL;
	char *cmdlp = NULL,*processName = NULL,*p1 = NULL,*p2 = NULL,*p3 = NULL,*p4 = NULL;
	DIR *dirproc=NULL;
	struct dirent *direproc,*direfd;
	cmdlbuf[sizeof(cmdlbuf)-1]='\0';

	int pid,ppid,uid,gid,mode;
	int pidflag,ppidflag,uidflag,gidflag;

	if (!(dirproc=opendir(PATH_PROC))) goto fail;
	while (direproc=readdir(dirproc)) 
	{
		pidflag = 0;
		ppidflag = 0;
		uidflag = 0;
		gidflag = 0;
		for (cs=direproc->d_name;*cs;cs++)        
			if (!isdigit(*cs))  
				break;
		if (*cs) 
			continue;
		snprintf(line,sizeof(line),PATH_PROC_CMDLINE,direproc->d_name);
		fd = open(line, O_RDONLY);
		if (fd < 0) 
			continue;
		cmdllen = read(fd, cmdlbuf, sizeof(cmdlbuf) - 1);
		if (close(fd)) 
			continue;
		if ((cmdllen == -1) || (cmdllen == 0))  
			continue;
		if (cmdllen < sizeof(cmdlbuf) - 1) 
			cmdlbuf[cmdllen]='\0';
		if ((cmdlp = strchr(cmdlbuf, '/'))) 
			processName = cmdlp;
		else
			processName = cmdlbuf;
		if(search_process_list(h,processName))
			continue;
		else
		{
			snprintf(line,sizeof(line),PATH_PROC_STATUS,direproc->d_name);
			fp = fopen(line, "r");
			if(fp == NULL) continue;
			while(fgets(statusbuf,sizeof(statusbuf),fp)!=NULL)
			{
				p1 = strtok(statusbuf,":");
				p2 = strtok(NULL,":");
				if(strcmp("Pid",p1) == 0)
				{
					pidflag = 1;
					pid = atoi(p2);	
				}
				if(strcmp("PPid",p1) == 0)
				{
                    ppidflag = 1;
					ppid = atoi(p2);
				}
				if(strcmp("Uid",p1) == 0)
				{
                	uidflag = 1;
                	p3 = strtok(p2+1," ");
            		uid = atoi(p3);					
				}
				if(strcmp("Gid",p1) == 0)
				{
                    gidflag = 1;
                    p3 = strtok(p2+1," ");
                	gid = atoi(p3);
				}
				if(pidflag && ppidflag && uidflag && gidflag)
				{
					append_process_list(h, pid,ppid,uid,gid,processName);
					break;
				}
			}
				fclose(fp);
			}
	}
	if(dirproc) 
		closedir(dirproc);
	return;
fail:
	printf("some error happen \n");
    log_e("processmonitor","open proc error");
}

