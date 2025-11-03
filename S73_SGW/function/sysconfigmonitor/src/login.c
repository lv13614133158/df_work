/*
 *who命令是从/var/run/utmp文件中读取信息，该文件是一个struct utmp的结构体
 * 故按照指定格式进行读取即可
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <utmp.h>
#include <sys/inotify.h> 
#include <string.h> 
#include "systemconfig.h"
#include "debug.h"
#include "inotifytools.h"
#include "common.h"

//#define   UTMP_FILE   /var/run/utmp
//#define		PATH	"./login.txt"
#define NAMELEN 32
#define ADDRLEN 256
#define TERMINALLEN 32

#define MAX_SYSTEMLISTENER 5

#define LOGIN_SUCC 7
#define LOGIN_FAIL 8

typedef struct logNode{
	int type;
	char name[NAMELEN];
	char addr[ADDRLEN];
	char terminal[TERMINALLEN];
	long long time;
	struct logNode *next;
}LogNode_t;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int login_check(LogNode_t *h);
//链表操作
static void login_append_list(LogNode_t *h,const struct utmp *utinfo);
static void login_destory_list(LogNode_t *h);
static int  init_loginfo();
static int  updata_oldlog();
static int logical_judgment_user_log();

static int watch_inotify_events(const int fd);
static bool monitorFlag = false;
static int InotifyFd;  
static int wd;

systemListener nlisten[MAX_SYSTEMLISTENER]={0};

/**
 * 	@func:注册用户登录监听回调函数
 *  @param: obj 回调函数
 * 
 */
int registerSystemConfigListener(systemListener obj)
{
	//loginCallback = obj;
	int i;
	for(i = 0;i < MAX_SYSTEMLISTENER;i++)
	{
		if(nlisten[i] == obj)
		{
			return -1;
		}
		if(nlisten[i] == 0)
		{
			pthread_mutex_lock(&lock);
			if(nlisten[i] == 0)
			{
				nlisten[i] = obj;
				pthread_mutex_unlock(&lock);
				return i;
			}
			pthread_mutex_unlock(&lock);	
		}
	}
	return -1;


}

/**
 * 	@func:注销文件监听回调函数
 * 
 */
void unregisterSystemConfigListener(int id)
{
	pthread_mutex_lock(&lock);
	nlisten[id] = 0;
	pthread_mutex_unlock(&lock);
}


/*
 * 描述：检测用户登陆的逻辑代码，当文件监控到有用户登陆时触发，上报登陆用户信息
 *
 * 参数：无
 *
 */

LogNode_t logHead = {.time = 0,.next = NULL};
LogNode_t oldHead = {.time = 0,.next = NULL};

/*
 *
 *	每次有用户登陆信号时将登陆信息读取到此链表，与原来的进行比较，查看
 *
 * 	新增加的用户信息
 */
static int  init_loginfo()
{
	int err;
	if(logHead.next != NULL)
	{
		login_destory_list(&logHead);
	}
	err = login_check(&logHead);
	return err;
}

/*
 *	将登陆信息备份到链表，方便比对，每次更新先清空链表然后再重新读取
 */
static int  updata_oldlog()
{
	int err;
	if(oldHead.next != NULL)
	{
		login_destory_list(&oldHead);
	}
	err = login_check(&oldHead);
	return err;
	
}

//读取/run/var/utmp 文件到h指定的头链表中
static int login_check(LogNode_t *h)
{
	struct utmp current_record;
	int utmpfd;
	int reclen = sizeof(current_record);

	utmpfd = open(UTMP_FILE,O_RDONLY);
	if(utmpfd == -1)
	{
		DERROR("open UTMP_FIL error:%s",strerror(errno));
		return -1;
	}

	while(read(utmpfd,&current_record,reclen) == reclen)
	{
		login_append_list(h,&current_record);
	}
	close(utmpfd);
	return 0;
}
/*
 * 描述：将utmp文件中的登陆信息按照需求追加到链表，
 *
 * 参数：struct utmp 类型的指针，每个结构体为一个登陆用户信息
 */

static void login_append_list(LogNode_t *h,const struct utmp *utinfo)
{
	LogNode_t **temp,*newNode;

	if((utinfo->ut_type != USER_PROCESS)&&(utinfo->ut_type != LOGIN_PROCESS)) //只考虑用户登陆的方式
		return ;
	newNode = malloc(sizeof(LogNode_t));
	if(newNode == NULL)
	{
		DERROR("malloc error\n");
		return ;
	}
	memset(newNode,0,sizeof(LogNode_t));
	newNode->type = utinfo->ut_type;
	strncpy(newNode->name,utinfo->ut_user,NAMELEN);  //用户名
	strncpy(newNode->addr,utinfo->ut_host,ADDRLEN);	//登陆IP
	strncpy(newNode->terminal,utinfo->ut_line,TERMINALLEN);//登陆终端
	newNode->time = (long long)utinfo->ut_tv.tv_sec*1000 + (long long)utinfo->ut_tv.tv_usec/1000;	//登陆时间
	
	//一定要使新节点指向空,
	newNode->next = NULL;

	temp = &h->next;
	while(*temp!=NULL)
		temp = &(*temp)->next;
	*temp = newNode;
	h->time++;//表示当前登录的用户数量
}

//judgment login user add to msgqueue
static int logical_judgment_user_log()
{
	int i;
	LogNode_t  *temp,*oldtemp;
	init_loginfo();
	char addr[ADDRLEN]={0};
	if(oldHead.time > logHead.time)
	{
		updata_oldlog();
		return 0;
	}


	if(oldHead.next == NULL)//第一次登陆状态
	{
		for(temp = logHead.next;temp!= NULL;temp = temp ->next)
		{
			memset(addr,0,ADDRLEN);
			if((temp->addr) && (strlen(temp->addr) != 0))
			{
				char *p1 = strrchr(temp->addr,':');
				int len1 = p1 ? strlen(p1):0;
				int len = strlen(temp->addr) - len1;
				memcpy(addr,temp->addr,len);
			}
			else
				snprintf(addr,ADDRLEN,"%s",temp->terminal);
			for(i = 0;i < MAX_SYSTEMLISTENER;i++)
			{
				if(nlisten[i] !=0)
				{		
					nlisten[i](temp->name,addr,temp->time,LOGIN_SUCC);
				}
			}
			//loginCallback(temp->name,addr,temp->time,temp->type);
			// login_event_cb(temp->name,addr,temp->time,temp->type);
		}
	}
	else //有多个登陆用户或终端
	{
		for(temp = logHead.next;temp!= NULL;temp = temp ->next)
		{
			int flag = 0;
			for(oldtemp = oldHead.next;oldtemp != NULL;oldtemp = oldtemp->next)
			{
				if((oldtemp->time == temp->time) && strcmp(oldtemp->terminal,temp->terminal)== 0)
				{
					flag = 1;//标志着此登陆用户已经被记录
					break;
				}
			}
			if(flag != 1)//新增用户
			{
				memset(addr,0,ADDRLEN);
				if((temp->addr) && (strlen(temp->addr) != 0))
				{
					char *p1 = strrchr(temp->addr,':');
					int len1 = p1 ? strlen(p1):0;
					int len = strlen(temp->addr) - len1;
					memcpy(addr,temp->addr,len);
				}
				else
					snprintf(addr,ADDRLEN,"%s",temp->terminal);
				for(i = 0;i < MAX_SYSTEMLISTENER;i++)
				{
					if(nlisten[i] !=0)
					{		
						nlisten[i](temp->name,addr,temp->time,temp->type);
					}
				}
				//loginCallback(temp->name,addr,temp->time,temp->type);
				// login_event_cb(temp->name,addr,temp->time,temp->type);
				break;
			}
		}
	}
	updata_oldlog();
	return 0;
}


/*
 * 描述： 销毁链表
 *
 * 参数： 无
 *
 */

static void login_destory_list(LogNode_t *h)
{
	LogNode_t *i,*p;
	
	i = h->next;
	while(i!= NULL)
	{
		p = i->next;
		free(i);
		i = p;
	}
	h->next = NULL;
	h->time = 0;
}

/****************login fail monitor**********************************************************/

static bool monitorLoginFailFlag = false;
static int InotifyFdLoginFail;
static int wdLoginFail;

static int logical_judgment_user_log_login_fail()
{
//asdfasd  ssh:notty    192.168.109.1    Sun Feb 19 17:57 - 17:57  (00:00)

    FILE *fp = NULL;
    char buffer[1024] = {0};
    int ret = 0;
    int i = 0, j = 0;

    fp = popen("lastb -n 1", "r");
    if (NULL == fp)
    {
        printf("popen\" lastb -n 1 \" error\n");

        return -1;
    }

    ret = fread(buffer, 1, sizeof(buffer), fp);
    if (ret > 0)
    {
        char user_name[24] = {0};
        char space[24] = {0};
        char terminal[24] = {0};
        char login_address[24] = {0};

        sscanf(buffer, "%[^ ]%[ ]%[^ ]%[ ]%[^ ]", user_name, space, terminal, space, login_address);

        for (i = 0;i < MAX_SYSTEMLISTENER;i++)
        {
            if(nlisten[i] !=0)
            {
                nlisten[i](user_name,login_address,clockobj.get_current_time(),LOGIN_FAIL);
            }
        }
    }
    pclose(fp);

    return 0;
}

static int watch_inotify_events_login_fail(int fd)  
{  
	char event_buf[512];  
	int ret;  
	int event_pos = 0;  
	int event_size = 0;  
	struct inotify_event *event;  
		
	/*读事件是否发生，没有发生就会阻塞*/  
	ret = read(fd, event_buf, sizeof(event_buf));  
		
	/*如果read的返回值，小于inotify_event大小出现错误*/  
	if(ret < (int)sizeof(struct inotify_event))  
	{  
		DPRINTF("counld not get event!\n");  
		return -1;  
	}  
		
	/*因为read的返回值存在一个或者多个inotify_event对象，需要一个一个取出来处理*/  
	while( ret >= (int)sizeof(struct inotify_event) )  
	{  
		event = (struct inotify_event*)(event_buf + event_pos);  
		if(event -> mask & IN_MODIFY)
		{
			logical_judgment_user_log_login_fail();
		}
		/*event_size就是一个事件的真正大小*/  
		event_size = sizeof(struct inotify_event) + event->len;  
		ret -= event_size;  
		event_pos += event_size;  
	}  		
	return 0;  
}  

void *pthreadFuncLoginFail(void *arg)
{
	int fd = InotifyFdLoginFail;
	int ret;
	pthread_detach(pthread_self());
	/*处理事件*/  
	while(monitorLoginFailFlag)
	{
		ret = watch_inotify_events_login_fail(fd); 
		if(ret < 0)
		{
			DERROR("watch_inotify_events_login_fail error\n");
			break;
		}
	}
}

int startLoginFailMonitor()
{  	 
	pthread_t tid;
	if(monitorLoginFailFlag)
	{
		return 0;
	}
	monitorLoginFailFlag = true;
	/*inotify初始化*/  
	InotifyFdLoginFail = inotify_init();  
	if( InotifyFdLoginFail == -1)  
	{  
		DERROR("inotify_init error!\n");  
		return -1;  
	}	
	/*添加watch对象*/  
	wdLoginFail = inotify_add_watch(InotifyFdLoginFail, "/var/log/btmp", IN_MODIFY); 
	if(wdLoginFail < 0)
	{
		DERROR("inotify_add_watch error\n");
		return -1;
	}  
	pthread_create(&tid,NULL,(void *)pthreadFuncLoginFail,NULL);				
	return 0;  
} 
/**************************************************************************/

static int watch_inotify_events(int fd)  
{  
	char event_buf[512];  
	int ret;  
	int event_pos = 0;  
	int event_size = 0;  
	struct inotify_event *event;  
		
	/*读事件是否发生，没有发生就会阻塞*/  
	ret = read(fd, event_buf, sizeof(event_buf));  
		
	/*如果read的返回值，小于inotify_event大小出现错误*/  
	if(ret < (int)sizeof(struct inotify_event))  
	{  
		DPRINTF("counld not get event!\n");  
		return -1;  
	}  
		
	/*因为read的返回值存在一个或者多个inotify_event对象，需要一个一个取出来处理*/  
	while( ret >= (int)sizeof(struct inotify_event) )  
	{  
		event = (struct inotify_event*)(event_buf + event_pos);  
		if(event -> mask & IN_MODIFY)
		{
			logical_judgment_user_log();
		}
		/*event_size就是一个事件的真正大小*/  
		event_size = sizeof(struct inotify_event) + event->len;  
		ret -= event_size;  
		event_pos += event_size;  
	}  		
	return 0;  
}  
      
void *pthreadFunc(void *arg)
{
	int fd = InotifyFd;
	int ret;
	pthread_detach(pthread_self());
	/*处理事件*/  
	while(monitorFlag)
	{
		ret = watch_inotify_events(fd); 
		if(ret < 0)
		{
			DERROR("watch_inotify_events error\n");
			break;
		}
	}
}

/**
 * 	@func:开启用户登录监控
 *  @param:NULL
 * 	@return :success return 0,faile return -1
 *  @note:
 */
int startMonitor()
{  	 
	pthread_t tid;
	if(monitorFlag)
	{
		return 0;
	}
	monitorFlag = true;
	/*inotify初始化*/  
	InotifyFd = inotify_init();  
	if( InotifyFd == -1)  
	{  
		DERROR("inotify_init error!\n");  
		return -1;  
	}	
	/*添加watch对象*/  
	wd = inotify_add_watch(InotifyFd, UTMP_FILE, IN_MODIFY); 
	if(wd < 0)
	{
		DERROR("inotify_add_watch error\n");
		return -1;
	}  
	pthread_create(&tid,NULL,(void *)pthreadFunc,NULL);				
    startLoginFailMonitor();
	return 0;  
} 


/******************************************************
 * 
 * @func:停止文件监控
 * @param:void
 * @return: 
 * @note:
 * 
 ******************************************************/
void stopMonitor()
{
	monitorFlag = false;
	inotify_rm_watch(InotifyFd,wd);
	close(InotifyFd);
}
