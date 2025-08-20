#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <mntent.h>
#include <string.h>
#include "resourceStatus.h"
#include "debug.h"


#define INTERVAL_TIME 1
struct memInfo{
        unsigned int  dwMemTotal;
        unsigned int  dwMemFree;
        unsigned int  dwMemAvailable;
        unsigned int  dwBuffers;
        unsigned int  dwCached;
    };

typedef struct listNode{
	char name[64];
	struct listNode *next;
}listNode_t;


//初始化带头结点的单项链表
static void initList(listNode_t **head)
{
	*head = (listNode_t *)malloc(sizeof(listNode_t));
	(*head)->next = NULL;
}

//判断str是否存在，存在返回true，不存在返回false
static bool compareString(listNode_t *head, const char *str)
{
	listNode_t *node=NULL;
	if(head == NULL)
		return false;
	node = head->next;
	while(node !=NULL)
	{
		if(strcmp(str,node->name) == 0)
		{
			return true;
		}
		node = node->next;
	}
	return false;
}


//头插
static void insertHead(listNode_t *head, const char *str)
{
	listNode_t *node = (listNode_t *)malloc(sizeof(listNode_t));
	memset(node->name,0,sizeof(node->name));
	memcpy(node->name,str,sizeof(node->name));
	node -> next = head->next;
	head -> next = node;
}

//删除
static void deleteNode(listNode_t **head)
{
	listNode_t *p=NULL,*temp=NULL;

	if (!head)
	{
		return;
	}
	if (*head == NULL)
	{
		return;
	}

	p = (*head)->next;
	while(p)
	{
		temp = p;
		p = p ->next;
		(*head) -> next = p;
		free(temp);
	}
	free(*head);
	*head = NULL;
}

/*  _internal use
*   @brief Float number to integer, rounded
*/
static int floatToInt(float value)
{
    int temp,integer,remainder;
    temp = (int)(value*10);
    integer = temp / 10;
    remainder = temp % 10;
    if(remainder>=5)
        integer+=1;
    return integer;
}

/*
*	@brief Get ram total size
*	@return availed RAM size ,unit : Kb
*
*/
long long getRAMTotalSize()
{
	FILE* fpMemInfo = fopen("/proc/meminfo", "r");
	if (NULL == fpMemInfo)
	{
		DERROR("fopen /proc/meminfo failed!:%s\n",strerror(errno));
		return -1;
	}    
	long long TotalSize;
	long long value;
	char name[1024];
	char line[1024];
	int nFiledNumber = 2;
    struct memInfo tMemInfo;
	memset(&tMemInfo,0,sizeof(struct memInfo));
	while (fgets(line, sizeof(line) - 1, fpMemInfo))
	{
		if (sscanf(line, "%s%lld", name, &value) != nFiledNumber)
		{
			continue;
		}
		if (0 == strcmp(name, "MemTotal:"))
		{
			break;
		}
	}
	//TotalSize = value << 10;
	TotalSize = value;
    fclose(fpMemInfo);
	return TotalSize;
}


/*
*	@brief Get ram available size
*	@return availed RAM size ,unit : kb
*
*/
long long getRAMFreeSize()
{
	FILE* fpMemInfo = fopen("/proc/meminfo", "r");
	if (NULL == fpMemInfo)
	{
		DERROR("fopen /proc/meminfo failed!:%s\n",strerror(errno));
		return -11;
	}    
	int i = 0;
	long long freeSize;
	long long temp;
	unsigned int value;
	char name[1024];
	char line[1024];
	int nFiledNumber = 2;
	int nMemberNumber = 5;
    struct memInfo tMemInfo;
	memset(&tMemInfo,0,sizeof(struct memInfo));
	while (fgets(line, sizeof(line) - 1, fpMemInfo))
	{
		if (sscanf(line, "%s%u", name, &value) != nFiledNumber)
		{
			continue;
		}
		if (0 == strcmp(name, "MemFree:"))
		{
			++i;
			tMemInfo.dwMemFree = value;
		}
		else if (0 == strcmp(name, "Buffers:"))
		{
			++i;
			tMemInfo.dwBuffers = value;
		}
		else if (0 == strcmp(name, "Cached:"))
		{
			++i;
			tMemInfo.dwCached = value;
		}
		if (i == nMemberNumber)
		{
			break;
		}
	}
	temp = tMemInfo.dwMemFree+tMemInfo.dwBuffers+tMemInfo.dwCached;
	freeSize = temp;
    fclose(fpMemInfo);
	return freeSize;
}

/*
*   @brief Get ram utilization
*   @param void
*   @return The return value is the percentage of utilization,int type
*/
int getRAMUsage()
{
    FILE* fpMemInfo = fopen("/proc/meminfo", "r");
	if (NULL == fpMemInfo)
	{
		DERROR("fopen /proc/meminfo failed!:%s\n",strerror(errno));
		return 1;
	}    
	int i = 0;
	unsigned int value;
	char name[1024];
	char line[1024];
	int nFiledNumber = 2;
	int nMemberNumber = 5;
	int memUseRate;
    float temp;
    struct memInfo tMemInfo;
	memset(&tMemInfo,0,sizeof(struct memInfo));
	while (fgets(line, sizeof(line) - 1, fpMemInfo))
	{
		if (sscanf(line, "%s%u", name, &value) != nFiledNumber)
		{
			continue;
		}
		if (0 == strcmp(name, "MemTotal:"))
		{
			++i;
			tMemInfo.dwMemTotal = value;
		}
		else if (0 == strcmp(name, "MemFree:"))
		{
			++i;
			tMemInfo.dwMemFree = value;
		}
        else if (0 == strcmp(name,"MemAvailable:"))
        {
            ++i;
            tMemInfo.dwMemAvailable = value;
        }

		else if (0 == strcmp(name, "Buffers:"))
		{
			++i;
			tMemInfo.dwBuffers = value;
		}
		else if (0 == strcmp(name, "Cached:"))
		{
			++i;
			tMemInfo.dwCached = value;
		}
		if (i == nMemberNumber)
		{
			break;
		}
	}
    //(memavailed+free)/total*100;
	temp = (((float)(tMemInfo.dwMemTotal - tMemInfo.dwMemFree - tMemInfo.dwBuffers - tMemInfo.dwCached)/((float)tMemInfo.dwMemTotal)) * 100);
	if (temp < 0)
	{
		temp = 0;
	}
    memUseRate = floatToInt(temp);
	fclose(fpMemInfo);
	return memUseRate;
}


/*
*   @brief cpu usage
*   
*/
int getCPUUsage()
{
	char cpu[5],buf[128]={0};
	FILE *fp;
	float usage;
    int ret;
	long int user,nice1,sys,idle,iowait,irq,softirq;
	long int all1,all2,idle1,idle2;
					   
	fp = fopen("/proc/stat","r");
	if(fp == NULL)
	{
		DERROR("fopen file /proc/stat error\n");
		return -1;
	}

	char* result = fgets(buf,sizeof(buf),fp);
	sscanf(buf,"%s%ld%ld%ld%ld%ld%ld%ld",cpu,&user,&nice1,&sys,&idle,&iowait,&irq,&softirq);

	all1 = user + nice1 + sys + idle + iowait + irq + softirq;
	idle1 = idle;
	//将指针回到文件首部，准备第二次读取数据
	rewind(fp);
	
	sleep(INTERVAL_TIME);
	memset(buf,0,sizeof(buf));
	cpu[0]= '\0';
	user = nice1 = sys = idle = iowait = irq = softirq = 0;

	result = fgets(buf,sizeof(buf),fp);
	sscanf(buf,"%s%ld%ld%ld%ld%ld%ld%ld",cpu,&user,&nice1,&sys,&idle,&iowait,&irq,&softirq);
	
	all2 = user + nice1 + sys + idle + iowait + irq + softirq;
	idle2 = idle;

	usage = (float)(all2 - all1 - (idle2 - idle1))/(all2 - all1)*100;//cpu利用率
    if(usage < 0)
        usage = 0;
    ret = floatToInt(usage);
	fclose(fp);
	
	return ret;
}


/*
*	function:getROMTotalSize
*	input   :void
*	output  :long long 
*	decla	:获取ROM总大小,单位b
*/
long long getROMTotalSize()
{
	long long totoalSize= 0;
	FILE* mount_table;
	struct mntent *mount_entry;
	struct statfs s;
	unsigned long blocks_used;
	unsigned blocks_percent_used;
	const char *disp_units_hdr = NULL;
	listNode_t *head = NULL;

	mount_table = NULL;
	mount_table = setmntent("/etc/mtab", "r");
	if (!mount_table)
	{
		fprintf(stderr, "\n");
		return -1;
	}
	while (1) {
		bool ret;
		const char *device;
		const char *mount_point;
		if (mount_table) {
			mount_entry = getmntent(mount_table);
			if (!mount_entry) {
				endmntent(mount_table);
				break;
			}
		} 
		else
			continue;
		device = mount_entry->mnt_fsname;
		mount_point = mount_entry->mnt_dir;
		//fprintf(stderr, "mount info: device=%s mountpoint=%s\n", device, mount_point);
		if (statfs(mount_point, &s) != 0) 
		{
			fprintf(stderr, "statfs failed!\n");	
			continue;
		}
		if ((s.f_blocks > 0) || !mount_table ) 
		{
			/* GNU coreutils 6.10 skips certain mounts, try to be compatible.  */
			if ((strcmp(device, "rootfs") == 0) ||(strcmp(device,"tmpfs") == 0))
				continue;
			if(head == NULL)
			{
				initList(&head);
			}
			ret = compareString(head, device);
			if(ret == true)
				continue;
			insertHead(head, device);
			totoalSize += s.f_blocks * (unsigned long long )s.f_bsize;
		}
	}

	deleteNode(&head);
	return totoalSize;
}
/*
*	@brief Get rom free space
*	@return return ROM space size ，unit b
*/
long long getROMFreeSize()
{
	long long freeSize= 0;
	FILE* mount_table;
	struct mntent *mount_entry;
	struct statfs s;
	unsigned long blocks_used;
	unsigned blocks_percent_used;
	const char *disp_units_hdr = NULL;
	listNode_t *head = NULL;

	mount_table = NULL;
	mount_table = setmntent("/etc/mtab", "r");
	if (!mount_table)
	{
		fprintf(stderr, "set mount entry error\n");
		return -1;
	}
	while (1) {
		bool ret;
		const char *device;
		const char *mount_point;
		if (mount_table) {
			mount_entry = getmntent(mount_table);
			if (!mount_entry) {
				endmntent(mount_table);
				break;
			}
		} 
		else
			continue;
		device = mount_entry->mnt_fsname;
		mount_point = mount_entry->mnt_dir;
		//fprintf(stderr, "mount info: device=%s mountpoint=%s\n", device, mount_point);
		if (statfs(mount_point, &s) != 0) 
		{
			fprintf(stderr, "statfs failed!\n");	
			continue;
		}
		if ((s.f_blocks > 0) || !mount_table ) 
		{
			/* GNU coreutils 6.10 skips certain mounts, try to be compatible.  */
			if ((strcmp(device, "rootfs") == 0) ||(strcmp(device,"tmpfs") == 0))
				continue;

			if(head == NULL)
			{
				initList(&head);
			}
			ret = compareString(head, device);
			if(ret == true)
				continue;
			insertHead(head, device);
			freeSize += s.f_bavail * (unsigned long long )s.f_bsize;
		}
	}

	deleteNode(&head);
	return freeSize;
}

/*
*	@brief get rom usage
*	@return Returns the percentage of utilization
*
*/
int getROMUsage()
{
	long long freeSize = 0;
	long long totalSize = 0;
	float ROMusage=0;
	int romUsage = 0;
	FILE* mount_table;
	struct mntent *mount_entry;
	struct statfs s;
	unsigned long blocks_used;
	unsigned blocks_percent_used;
	const char *disp_units_hdr = NULL;
	listNode_t *head = NULL;

	mount_table = NULL;
	mount_table = setmntent("/etc/mtab", "r");
	if (!mount_table)
	{
		fprintf(stderr, "set mount entry error\n");
		return -1;
	}
	while (1) {
		bool ret;
		const char *device;
		const char *mount_point;
		if (mount_table) {
			mount_entry = getmntent(mount_table);
			if (!mount_entry) {
				endmntent(mount_table);
				break;
			}
		} 
		else
			continue;
		device = mount_entry->mnt_fsname;
		mount_point = mount_entry->mnt_dir;
		//fprintf(stderr, "mount info: device=%s mountpoint=%s\n", device, mount_point);
		if (statfs(mount_point, &s) != 0) 
		{
			fprintf(stderr, "statfs failed!\n");	
			continue;
		}
		if ((s.f_blocks > 0) || !mount_table ) 
		{
			// blocks_used = s.f_blocks - s.f_bfree;
			// blocks_percent_used = 0;
			// if (blocks_used + s.f_bavail) 
			// {
			// 	blocks_percent_used = (blocks_used * 100ULL
			// 			+ (blocks_used + s.f_bavail)/2
			// 			) / (blocks_used + s.f_bavail);
			// }
			/* GNU coreutils 6.10 skips certain mounts, try to be compatible.  */
			if ((strcmp(device, "rootfs") == 0) ||(strcmp(device,"tmpfs") == 0))
				continue;
			

			if(head == NULL)
			{
				initList(&head);
			}
			ret = compareString(head, device);
			if(ret == true)
				continue;
			insertHead(head, device);
			totalSize +=  s.f_blocks * (unsigned long long )s.f_bsize;
			freeSize += s.f_bavail * (unsigned long long )s.f_bsize;

			// if (printf("\n%-20s" + 1, device) > 20)
			// 	    printf("\n%-20s", "");
			// char s1[20];
			// char s2[20];
			// char s3[20];
			// strcpy(s1, kscale(s.f_blocks, s.f_bsize));
			// strcpy(s2, kscale(s.f_blocks - s.f_bfree, s.f_bsize));
			// strcpy(s3, kscale(s.f_bavail, s.f_bsize));
			// printf(" %9s %9s %9s %3u%% %s\n",
			// 		s1,
			// 		s2,
			// 		s3,
			// 		blocks_percent_used, mount_point);
		}
	}
	deleteNode(&head);
	ROMusage = (float)(totalSize-freeSize)/totalSize *100;
	romUsage = floatToInt(ROMusage);

	return romUsage;
}


//get cpu core number
int getCpuCoreNum()
{
	int num;
	num = sysconf(_SC_NPROCESSORS_ONLN);
	return num;

}
