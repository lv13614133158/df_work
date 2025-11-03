/*
*	auth:xuewenlinag 2019/9/26
*	brief:linux端系统配置模块
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include "debug.h"
#include "systemconfig.h"
#include "cJSON.h"
#include "get_imei.h"


#define READ_DATA_SIZE 1024
#define ARRAYSIZE 256

static struct hardword_info obj={{0},{0},{0},{0},{0},0,{0},0};
IpNode_t ip_head={{0},{0},NULL};

static void store_password(PassNode_t *h,char *addr);
static int append_pass_list(PassNode_t *h,char *str1,char *str2);
static void destory_pass_list(PassNode_t *h);

static void get_hostinfo();
static void get_meminfo();
static void get_cpuinfo();
static void init_hardinfo();
static void get_local_ip();



static int append_node_to_list(char *name,char *ip)
{
	IpNode_t *newNode,*h = &ip_head;

	newNode = malloc(sizeof(IpNode_t));
	if(newNode == NULL)
	{
		perror("malloc");
		return -1;
	}
	strncpy(newNode->name,name,30);
	strncpy(newNode->ip,ip,30);
	newNode->next = NULL;
	//尾部插入节点
	IpNode_t **l;
	l = &h->next;
	while((*l)!=NULL)
		l = &(*l)->next;
	*l = newNode;
	return 0;
}

static void destory_ip_list()
{
	IpNode_t *i,*temp,*h = &ip_head;
	i = h->next;
	while(i!=NULL)
	{
		temp = i->next;
		free(i);
		i = temp;
	}
	h->next = NULL;
	
}

/* 
*   @brief:get userAnd passwd 
*	@param:<result> JSON struct,Receive returned data 
*	@param:<length> result  length;
*	@return：success return 0，otherwish return negative
*/
int get_valid_user_and_passwd(char **result)
{
    FILE *fp=NULL;
	char buffer[ARRAYSIZE]={0};	
	char *s=NULL;
	char *temp = NULL;
    int userCount,num=0;
	PassNode_t pass_head = {.next = NULL};
	PassNode_t *i=NULL;
    PassNode_t *h = &pass_head;
	cJSON *root=NULL;
	fp = fopen("/etc/shadow","r");
	if(fp == NULL)
	{
		DERROR("fopen /etc/shadow error  %s\n",strerror(errno));
		return -1;;
	}		
	while(fgets(buffer,sizeof(buffer),fp)!=NULL)
	{
		if(strlen(buffer)>40)//初步筛选可能的有效用户
		{
             store_password(h,buffer);
		}
        memset(buffer,0,ARRAYSIZE);
	}
	fclose(fp);
    //获取信息并转为json结构

    userCount = h->count;
	cJSON *pobj[userCount];
    root = cJSON_CreateArray();
	if(root == NULL)
	{
		destory_pass_list(h);
		return -1;
	}
    for(i = h->next;i !=NULL;i=i->next)
    {
		cJSON_AddItemToArray(root,pobj[num]=cJSON_CreateObject());
		cJSON_AddStringToObject(pobj[num],"user",i->name);
		cJSON_AddStringToObject(pobj[num],"passwd",i->password);
		num++;
    }
	
    s = cJSON_PrintUnformatted(root);
	if(s == NULL)
	{
		cJSON_Delete(root);
		destory_pass_list(h);
		return -1;
	}
	temp = (char *)malloc(strlen(s)+1);
	if(temp == NULL)
	{
		cJSON_Delete(root);
		free(s);
		destory_pass_list(h);
		return -1;
	}
	memset(temp,0,strlen(s)+1);
	memcpy(temp,s,strlen(s));
	*result = temp;
	cJSON_Delete(root);
	free(s);
    destory_pass_list(h);
	return 0;
}

static void store_password(PassNode_t *h,char *addr)
{
	int i = 0;
	char *p[2],*temp=NULL,*name,*password;
	char *buf = addr;
	
	while((p[i]= strtok_r(buf,":",&temp))!=NULL)
	{
		i++;
		buf = NULL;
		if(i>=2)
			break;
	}
	name=p[0];
	password=p[1];
    if(strlen(password) > 4)
    {
        append_pass_list(h,name,password);
    }
}

static int append_pass_list(PassNode_t *h,char *str1,char *str2)
{
	PassNode_t *newNode;

	newNode = malloc(sizeof(PassNode_t));
	if(newNode == NULL)
	{
		perror("malloc");
		return -1;
	}

	newNode->count = 0;
	memset(newNode->name,0,32);
	memset(newNode->password,0,128);
	strncpy(newNode->name,str1,sizeof(newNode->name) - 1);
	strncpy(newNode->password,str2,sizeof(newNode->password) - 1);
	newNode->next = NULL;
	//尾部插入节点
	PassNode_t **l;
	l = &h->next;
	while((*l)!=NULL)
		l = &(*l)->next;
	*l = newNode;
	h->count +=1;
	return 0;
}

static void destory_pass_list(PassNode_t *h)
{
	PassNode_t *i,*temp;
	i = h->next;
	while(i!=NULL)
	{
		temp = i->next;
		free(i);
		i = temp;
	}
	h->count = 0;
	h->next = NULL;
}

//get hardware info

static void get_local_ip()
{
	int ret = 0;
	struct ifaddrs *addr = NULL;
	struct ifaddrs *temp_addr = NULL;
	ret = getifaddrs(&addr);
	if (ret == 0) {
		temp_addr = addr;
		while(temp_addr != NULL) 
		{	
			if(temp_addr->ifa_addr == 0)
			{
				temp_addr = temp_addr->ifa_next;
				continue;
			}
			if(temp_addr->ifa_addr->sa_family == AF_INET) 
			{			 
				char *tmp = inet_ntoa((struct in_addr){.s_addr=((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr.s_addr});				
				append_node_to_list(temp_addr->ifa_name,tmp);			 
			}
			temp_addr = temp_addr->ifa_next;
		}
	}
	freeifaddrs(addr);
}

static void get_hostinfo()
{
	struct utsname uts;
	if(uname(&uts)<0)
	{
		DERROR("not gain host infomation\n");
		return ;
	}	
	strncpy(obj.sys_name,uts.sysname,20);
	strncpy(obj.machine,uts.machine,20);
	strncpy(obj.host_name,uts.nodename,40);
	strncpy(obj.version,uts.version,50);
	strncpy(obj.release,uts.release,50);
}

//get RAM size
static void get_meminfo()
{
	struct sysinfo si;
	sysinfo(&si);
	obj.ram_size = si.totalram;
}

//get cpu info 
static void get_cpuinfo()
{
	FILE *fp;
	char buf[1024];
	int cpu_num;
	char *p,*after = NULL;
	fp = fopen("/proc/cpuinfo","rb");
	if(fp == NULL)
	{
		DPRINTF("fopen failed\n");
		return;
	}
	while(fgets(buf,sizeof(buf),fp) != NULL)
	{
		if((p = strstr(buf,"model name"))!= NULL)
		{
			strtok_r(p,":",&after);
			break;
		}
	}
	fclose(fp);	
	if(after != NULL)
		strncpy(obj.cpu_model_name,after,50);
	else
		strncpy(obj.cpu_model_name,"no find",50);

	cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	obj.core_num = cpu_num;

}

static void init_hardinfo()
{
	get_hostinfo();
	get_meminfo();
	get_cpuinfo();
	get_local_ip();

}


int get_hard_info(char **outbuf)
{
	cJSON *root=NULL,*item = NULL,*ipArray=NULL,*ipnode = NULL;
	IpNode_t *i,*h = &ip_head;
	char *temp = NULL;
	char buf[100]={0};
	int ret;
	char *network = NULL;
	init_hardinfo();
	//网卡信息封装层json字符串传递，避免上层解析出错
	ipArray = cJSON_CreateArray();
	if(ipArray == NULL)
		return -1;
	cJSON_AddItemToArray(ipArray,ipnode = cJSON_CreateObject());
	for(i = h->next;i!=NULL;i=i->next)
	{
		cJSON_AddStringToObject(ipnode,i->name,i->ip);	
	}
	network = cJSON_PrintUnformatted(ipArray);

	ret = get_device_sn(buf,sizeof(buf));
	if(ret < 0)
		memset(buf,0,sizeof(buf));
	root=cJSON_CreateObject();
	// root = cJSON_CreateArray();
	if(root == NULL)
		return -1;
	// cJSON_AddItemToArray(root,item = cJSON_CreateObject());
	cJSON_AddNumberToObject(root,"core_num",obj.core_num); 
	cJSON_AddStringToObject(root,"cpu_model_name",obj.cpu_model_name);
	cJSON_AddStringToObject(root,"host_name",obj.host_name);
	cJSON_AddStringToObject(root,"ip_addr",network);

	cJSON_AddStringToObject(root,"machine",obj.machine);
	cJSON_AddNumberToObject(root,"mem_total",obj.ram_size);
	cJSON_AddStringToObject(root,"release",obj.release);
	cJSON_AddStringToObject(root,"os_name",obj.sys_name);  
	cJSON_AddStringToObject(root,"os_version",obj.version); 

    // cJSON_AddStringToObject(item,"imei","tbox_aaa");
	// cJSON_AddStringToObject(item,"imei",buf);
	// cJSON_AddNumberToObject(item,"client_version_code",1); 
	// cJSON_AddStringToObject(item,"client_version_name","1.0.0"); 

	destory_ip_list();
	char *s = cJSON_PrintUnformatted(root);
	if(network)
	{
		free(network);
	}
	if(ipArray)
		cJSON_Delete(ipArray);
	if(root)
	{
		cJSON_Delete(root);
	}
	if(s == NULL)
	{
		return -1;
	}

	temp = (char *)malloc(strlen(s)+1);
	if(temp == NULL)
	{
		free(s);
		return -1;
	}
	memset(temp,0,strlen(s)+1);
	memcpy(temp,s,strlen(s));
	*outbuf = temp;
	free(s);	
	return 0;
}
