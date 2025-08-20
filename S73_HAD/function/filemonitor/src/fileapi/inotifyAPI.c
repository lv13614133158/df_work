#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <dirent.h>
#include "inotifytools.h"
#include "inotifyAPI.h"
#include "debug.h"
#include "lsof.h"

#define MAX_LISTENER 10

listener mlisten[MAX_LISTENER]={0};
static pthread_t tid;
static int flag = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int is_dir_exist(const char *dir_path);

void *inotify_read_event(void *arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
	struct inotify_event *event;
	while(1)
	{
		pthread_testcancel();
		event = inotifytools_next_event(-1);
		pthread_testcancel();
		inotify_all_observer(event);
	}
}
int startFileMonitor()
{
	if(flag)
		return 0;
	int ret;
	ret = inotifytools_initialize();
	if(ret!=1)
		return -1;
	flag = 1;//标志已经初始化
	ret = pthread_create(&tid,NULL,inotify_read_event,NULL);
	return 0;
}
/**
*	添加文件监控点
*	使用该函数前必须先调用startFileMonitor()函数
*	@param dirs:需要监控的目录和文件，多个目录可以用'|'分割
*	@param mask:监控的mask值，参考inotifyAPI.h头文件
*	@return  成功返回1,失败返回-1
*	
*	@note:	同一个目录相同mask值多次添加监控，只触发一次
*			同一个目录不同mask值多次添加监控，最后一次mask为最终监控值		
*
*/
int  addWatchPoint(const char *dirs, int mask)
{
	DPRINTF("path:%s  mask:0x%x\n",dirs,mask);
	int ret;
	char *buf=NULL;
	if(dirs == NULL)
	{
		return -1;
	}
	buf = malloc(strlen(dirs)+1);
	if(buf == NULL)
		return -1;
	memset(buf,0,strlen(dirs)+1);
	
    char delims[] = "|";
    char *result = NULL;
	char *pSave = NULL;
	memcpy(buf,dirs,strlen(dirs));
	result = strtok_r(buf, delims, &pSave);

    while( result != NULL ) {
		ret = inotifytools_watch_recursively(result, mask);
		if(ret !=1)
		{
			free(buf);
			return -1;
		}
       result = strtok_r(NULL, delims, &pSave);
   }  
   free(buf);
   return 0;    
}

void removeWatchPoint(char *dirs)
{
	char delims[] = "|";
    char *result = NULL;
    result = strtok( dirs, delims);
    while( result != NULL ) {
		inotifytools_remove_watch_by_filename(result);
        result = strtok( NULL, delims );
   }        
}

void removeAllWatchPoint()
{
	flag = 0;
	inotifytools_cleanup();
	pthread_cancel(tid);
    pthread_join(tid,NULL);
}


/**
*注册监听回调函数
* @param obs 回调函数指针
* @return 返回注册id，通过id注销函数监听
* @note 同一个函数指针多次注册，会进行过滤，只有一次注册成功
*
*/
int registerFileMonitorListener(listener obs)
{
	int i;
	for(i = 0;i < MAX_LISTENER;i++)
	{
		if(mlisten[i] == obs)
		{
			return -1;
		}
		if(mlisten[i] == 0)
		{
			pthread_mutex_lock(&lock);
			if(mlisten[i] == 0)
			{
				mlisten[i] = obs;
				pthread_mutex_unlock(&lock);
				return i;
			}
			pthread_mutex_unlock(&lock);	
		}
	}
	return -1;
}
void unRegisterFileMonitorListener(int id)
{
	pthread_mutex_lock(&lock);
	mlisten[id] = 0;
	pthread_mutex_unlock(&lock);
}


void inotify_all_observer(struct inotify_event *event)
{
	int i;
	char *path=NULL;
	char *processData=NULL;
	struct monitorEvent obj={{0},0};
	path = inotifytools_filename_from_wd(event->wd);
	strncpy(obj.path,path,sizeof(obj.path) - 1);
	//判断，如果是文件则不追加文件名
	if(is_dir_exist(obj.path)== 0)
		strncat(obj.path,event->name,512-strlen(path));
	//strcpy(obj.mask,inotifytools_event_to_str( event->mask));
	obj.mask = event->mask;

	//如果时创建目录则将新目录注册到监控列表中
	if(is_dir_exist(obj.path)==0 && (obj.mask & IN_CREATE))
	{
		inotifytools_watch_recursively(obj.path,IN_CREATE | IN_DELETE | IN_MODIFY);
	}
	 processData = gather_proc_info(obj.path);
	// //调用java的回调方法
	// Java_Level_Method_notify_callback(obj.path,obj.mask,processData);
#if 1
	//调用c层的检测者，暂时不用
	for(i = 0;i < MAX_LISTENER;i++)
	{
		if(mlisten[i] !=0)
		{		
			mlisten[i](obj.path,obj.mask,processData);
		}
	}
#endif 
	 free_proc_info_memory(processData);

}
static int is_dir_exist(const char *dir_path)
{
	DIR *dirp;
    if(dir_path == NULL)
        return -1;
    if((dirp = opendir(dir_path)) == NULL)
        return -1;
	closedir(dirp);
    return 0;
}

