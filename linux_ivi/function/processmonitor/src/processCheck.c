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
#include <pwd.h>
#include "util.h"
#include "websocketmanager.h"
#include "processCheck.h"
#include "processInfo.h"

#define PROCESS_CHANGE_MODE 1
#define PROCESS_OPEN_MODE   2
#define PROCESS_CLOSE_MODE  3
#define PROCESS_ZOMBIE      4

static processNode_t pHead2 = {.next=NULL}, oldHead2 = {.next = NULL};
static list *whiteProcessName = NULL;

void processCheckInit(list *listName)
{
    whiteProcessName = listName;
}



static cJSON* processCompose(int pid,int ppid,int uid,int gid,char *path,int action)
{
	//白名单过滤
#if 0
	if(judgeWhiteList(whiteProcessName, path)) {
        return;
    }
#endif
	cJSON *cjson_data = cJSON_CreateObject();
	long long timestamp = clockobj.get_current_time();
	cJSON_AddNumberToObject(cjson_data,"pid",pid);
	cJSON_AddNumberToObject(cjson_data,"ppid",ppid);
	cJSON_AddNumberToObject(cjson_data,"uid",uid);
	cJSON_AddNumberToObject(cjson_data,"gid",gid);
	cJSON_AddStringToObject(cjson_data,"path",path);
	cJSON_AddStringToObject(cjson_data,"hashcode","");
	if(access(path,F_OK) == 0)  //文件且存在
	{
		struct stat buf;
		unsigned char md5[16] = {0};
		char md5String[34]={0};
		stat(path,&buf);
		Compute_file_md5(path,md5String);

		if (getpwuid(buf.st_uid) == NULL)
		{
			cJSON_AddStringToObject(cjson_data,"mode","");
		}
		else
		{
			cJSON_AddStringToObject(cjson_data,"mode",(getpwuid(buf.st_uid))->pw_name);
		}
		cJSON_AddStringToObject(cjson_data,"md5",md5String);
	}
	else
	{
		cJSON_AddStringToObject(cjson_data,"mode","");
		cJSON_AddStringToObject(cjson_data,"md5","");
	}
	cJSON_AddNumberToObject(cjson_data,"action",action);
	cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);

	return cjson_data;
}

static void uploadProcessList(char *s)
{	
    websocketMangerMethodobj.sendEventData("0010103000122",s,"EVENT_TYPE_PROCESS_CHANGED");
}

static void uploadProcessEvent(int pid,int ppid,int uid,int gid,char *path,int action)
{
	cJSON *cjson_data = processCompose(pid, ppid, uid, gid, path, action);
	char *s = cJSON_PrintUnformatted(cjson_data);
    websocketMangerMethodobj.sendEventData("0010103000122",s,"EVENT_TYPE_PROCESS_CHANGED");
	if(s)
		free(s);
	if(cjson_data)
		cJSON_Delete(cjson_data);
}

// 获取所有 进程+pid
char *get_process_information()
{
	processNode_t *i=NULL,*h=NULL;
	char *s = NULL;
	getProcessInfo(&pHead2);

	cJSON *root = NULL;
	root = cJSON_CreateArray();
	if(root == NULL)
		return NULL;
	for(i = pHead2.next;i != NULL; i = i -> next)
	{
        cJSON *pobj = NULL;
		cJSON_AddItemToArray(root,pobj = cJSON_CreateObject());
		cJSON_AddNumberToObject(pobj,"process_id",i->pid);
		cJSON_AddStringToObject(pobj,"process_name",i->path);
	}

	s = cJSON_PrintUnformatted(root);
	if(root)
		cJSON_Delete(root);
	destory_process_list(&pHead2);
	return s;
}

static char* processFilter(char *list)
{
    cJSON *root = cJSON_Parse(list), *cjson_data = cJSON_CreateObject();
    if(root == NULL || cjson_data == NULL)
        return NULL;
    int count = cJSON_GetArraySize(root);
    cJSON *elem, *process_id, *process_name;
    for (int i = 0; i < count; i++)
    {
        elem = cJSON_GetArrayItem(root, i);
        process_id = cJSON_GetObjectItem(elem, "process_id");
        process_name = cJSON_GetObjectItem(elem, "process_name");
        char *sProcessName = process_name->valuestring;
        long long iPid = process_id->valueint;
        if (NULL != sProcessName && strlen(sProcessName) > 0 && iPid > 0)
        {
            if(judgeWhiteList(whiteProcessName, sProcessName)) {
                continue; 
            }
            char localpid[64] = {0};
            snprintf(localpid,64,"%lld",iPid);

            cJSON *process_elem = NULL; 
            cJSON_AddItemToArray(cjson_data,process_elem = cJSON_CreateObject());
		    cJSON_AddStringToObject(process_elem,"pid",localpid);
		    cJSON_AddStringToObject(process_elem,"process_name",sProcessName);
        }
    }  
    char* s = cJSON_PrintUnformatted(cjson_data);
	if(cjson_data)
		cJSON_Delete(cjson_data);
    if(root)
		cJSON_Delete(root);  
    return s;
}

// 云端白名单
static void processCloudCompare(processNode_t pHead2, list* whiteProcessName)
{
    processNode_t *i=NULL;
	char *s = NULL;
	cJSON *root = cJSON_CreateObject();
	cJSON *cjson_elem = cJSON_CreateArray();
	cJSON *cjson_data = cJSON_CreateObject();
	if(!root || !cjson_data || !cjson_elem)
	{
		return;
	}

	for(i = pHead2.next;i != NULL; i = i -> next)
	{
		char *sProcessName = i->path;
        long long iPid  = i->pid;
        if (NULL != sProcessName && strlen(sProcessName) > 0 && iPid > 0)
        {
            if(judgeWhiteList(whiteProcessName, sProcessName)) {
                continue; //过滤掉白名单内容
            }
            char localpid[64] = {0};
            snprintf(localpid,64,"%lld",iPid);

            cJSON *process_elem = NULL; 
            cJSON_AddItemToArray(cjson_elem,process_elem = cJSON_CreateObject());
		    cJSON_AddStringToObject(process_elem,"pid",localpid);
		    cJSON_AddStringToObject(process_elem,"process_name",sProcessName);
        }	
	}
	cJSON_AddItemToObject(cjson_data, "onNonWhiteProcessRun", cjson_elem);
    cJSON_AddItemToObject(root, "postEventData", cjson_data);

	s = cJSON_PrintUnformatted(root);
	uploadProcessList(s);

	if(s)
		free(s);
	if(root)
		cJSON_Delete(root);
	//destory_process_list(&pHead2);
}

static void processLocalCompare(processNode_t pHead2, list* whiteProcessName)
{
	processNode_t *temp_p1 = pHead2.next;
	while(temp_p1)  //查找改变的进程和新进程
	{
		if(!judgeWhiteList(whiteProcessName, temp_p1->path)) 
		{
			uploadProcessEvent(temp_p1->pid,temp_p1->ppid,temp_p1->uid,temp_p1->gid,temp_p1->path,PROCESS_OPEN_MODE);
    	}
		temp_p1 = temp_p1->next;
	}
	//destory_process_list(&pHead2);
}

// 上次进程oldHead2和本次检测pHead2进行比较
static void processCompare(void)
{
	int findflag;

	// step 1
	getProcessInfo(&pHead2);

	// step 2
	if(oldHead2.next == NULL) //初次检测进程
	{
		oldHead2 = pHead2;
        pHead2.next=NULL;
	}
	else
	{	
		processNode_t *temp_p1 = pHead2.next;
		processNode_t *temp_p2 = oldHead2.next;
		while(temp_p1)  // 查找改变的进程和新进程
		{
			while(temp_p2)
			{
				if(strcmp(temp_p1->path,temp_p2->path) == 0)
				{	
					findflag = 1;
					if((temp_p1->pid != temp_p2->pid) || (temp_p1->ppid != temp_p2->ppid) || (temp_p1->uid != temp_p2->uid) || (temp_p1->gid != temp_p2->gid))
					{
						uploadProcessEvent(temp_p1->pid,temp_p1->ppid,temp_p1->uid,temp_p1->gid,temp_p1->path,PROCESS_CHANGE_MODE);
						// printf("change path:%s pid:%d ppid:%d uid:%d gid:%d mode:%d\n\n",temp_p1->path,temp_p1->pid,temp_p1->ppid,temp_p1->uid,temp_p1->gid,temp_p1->mode);
					}
					break;
				}
				temp_p2 = temp_p2->next;
			}
			if(!findflag)
			{
				uploadProcessEvent(temp_p1->pid,temp_p1->ppid,temp_p1->uid,temp_p1->gid,temp_p1->path,PROCESS_OPEN_MODE);
				// printf("new process:path:%s pid:%d ppid:%d uid:%d gid:%d mode:%d\n\n",temp_p1->path,temp_p1->pid,temp_p1->ppid,temp_p1->uid,temp_p1->gid,temp_p1->mode);
			}
			temp_p2 = oldHead2.next;
			temp_p1 = temp_p1->next;
			findflag = 0;
		}
		temp_p1 = pHead2.next;
		temp_p2 = oldHead2.next;
		while(temp_p2)	// 查找关闭的进程
		{
			while(temp_p1)
			{
				if(strcmp(temp_p2->path,temp_p1->path) == 0)
				{	
					findflag = 1;
					break;
				}
				temp_p1 = temp_p1->next;
			}
			if(!findflag){
				uploadProcessEvent(temp_p2->pid,temp_p2->ppid,temp_p2->uid,temp_p2->gid,temp_p2->path,PROCESS_CLOSE_MODE);
				// printf("closed process:path:%s pid:%d ppid:%d uid:%d gid:%d mode:%d\n\n",temp_p2->path,temp_p2->pid,temp_p2->ppid,temp_p2->uid,temp_p2->gid,temp_p2->mode);
			}
			temp_p1 = pHead2.next;
			temp_p2 = temp_p2->next;
			findflag = 0;
		}

		// 3
		destory_process_list(&oldHead2);
		oldHead2 = pHead2;
		pHead2.next=NULL;
	}
}

static void processCompareEx(void)
{
	int findflag, eventCnt = 0;
	getProcessInfo(&pHead2);
	processNode_t *temp_p1 = pHead2.next;
	processNode_t *temp_p2 = oldHead2.next;

	// 非初次检测进程
	if(oldHead2.next != NULL) 
	{
		cJSON *root = NULL;
		root   = cJSON_CreateArray();
		if(root == NULL)
		{
			return;
		}

		// 查找改变的进程和新进程
		while(temp_p1)  
		{
			while(temp_p2)
			{
				if(strcmp(temp_p1->path,temp_p2->path) == 0)
				{	
					findflag = 1;
					if((temp_p1->pid != temp_p2->pid) || (temp_p1->ppid != temp_p2->ppid) || (temp_p1->uid != temp_p2->uid) || (temp_p1->gid != temp_p2->gid))
					{
						cJSON *pobj = processCompose(temp_p1->pid,temp_p1->ppid,temp_p1->uid,temp_p1->gid,temp_p1->path,PROCESS_CHANGE_MODE);
						cJSON_AddItemToArray(root,pobj);
						eventCnt++;
					}
					break;
				}
				temp_p2 = temp_p2->next;
			}
			if(!findflag)
			{
				cJSON *pobj = processCompose(temp_p1->pid,temp_p1->ppid,temp_p1->uid,temp_p1->gid,temp_p1->path,PROCESS_OPEN_MODE);
				cJSON_AddItemToArray(root,pobj);
				eventCnt++;
			}
			temp_p2 = oldHead2.next;
			temp_p1 = temp_p1->next;
			findflag = 0;
		}

		// 查找关闭的进程
		temp_p1 = pHead2.next;
		temp_p2 = oldHead2.next;
		while(temp_p2)	
		{
			while(temp_p1)
			{
				if(strcmp(temp_p2->path,temp_p1->path) == 0)
				{	
					findflag = 1;
					break;
				}
				temp_p1 = temp_p1->next;
			}
			if(!findflag){
				cJSON *pobj = processCompose(temp_p2->pid,temp_p2->ppid,temp_p2->uid,temp_p2->gid,temp_p2->path,PROCESS_CLOSE_MODE);
				cJSON_AddItemToArray(root,pobj);
				eventCnt++;
			}
			temp_p1 = pHead2.next;
			temp_p2 = temp_p2->next;
			findflag = 0;
		}

		if(eventCnt > 0)
		{
			char* s = cJSON_PrintUnformatted(root);
			if(s)
			{
				uploadProcessList(s);
				free(s);
			}
		}

		if(root)
		{
			cJSON_Delete(root);
		}
		destory_process_list(&oldHead2);
	}
	oldHead2 = pHead2;
	pHead2.next=NULL;		
}

void checkProcessLocal()
{
    getProcessInfo(&pHead2);
    processLocalCompare(pHead2, whiteProcessName);
	destory_process_list(&pHead2);
}

void checkProcessCloud()
{
	getProcessInfo(&pHead2);
	//processCloudCompare(pHead2, whiteProcessName);
	processLocalCompare(pHead2, whiteProcessName);
	destory_process_list(&pHead2);
}

void checkProcessChange()
{
    processCompare();
}

// 暂时未使用
void checkProcessList()
{
	processCompareEx();
}

void checkZombieProcess()
{
//Z+     33292   33293 [jingzhi] <defunct>

    FILE *fp = NULL;
    char buffer[1024] = {0};
    int ret = 0;
    int i = 0, j = 0;

    fp = popen("ps -A -ostat,ppid,pid,comm | grep -e '^[Zz]'", "r");
    if (NULL == fp)
    {
        printf("popen\" ps -A -ostat,ppid,pid,comm | grep -e '^[Zz]' \" error\n");
        return ;
    }

    ret = fread(buffer, 1, sizeof(buffer), fp);
    if (ret > 0)
    {

        char ostat_name[24] = {0};
        char space[24] = {0};
        char ppid[24] = {0};
        char pid[24] = {0};
        char path_temp[64] = {0};
        char path[64] = {0};

        sscanf(buffer, "%[^ ]%[ ]%[^ ]%[ ]%[^ ]%[ ]%[^ ]", ostat_name, space, ppid, space, pid, space, path_temp);

        for (i = 0; i < sizeof(path_temp); i++)
        {
            if (path_temp[i] != '[' && path_temp[i] != ']')
            {
                path[j++] = path_temp[i];
            }
        }
        cJSON *cjson_data = processCompose(atoi(pid), atoi(ppid), 0, 0, path, PROCESS_ZOMBIE);
        char *s = cJSON_PrintUnformatted(cjson_data);
        websocketMangerMethodobj.sendEventData("0010103000122",s,"EVENT_TYPE_PROCESS_CHANGED");
        if(s)
            free(s);
        if(cjson_data)
            cJSON_Delete(cjson_data);
    }
    pclose(fp);

    return;
}

