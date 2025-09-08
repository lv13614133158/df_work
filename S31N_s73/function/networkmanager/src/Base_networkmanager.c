

/*
 * @Descripttion:  networkmanager method
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-11-11 03:03:04
 */ 
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "api_networkclient.h"
#include "Base_networkmanager.h"
/**lower power related**/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "spdloglib.h"
#define UNIX_DOMAIN "@qihooidps"
#define MAX_LENGTH  32
/**
 * @name:   val def
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
static timerIdps *heartbeatManager = NULL;
static timerIdps *reRequestManager = NULL;
 /**
  * @name:   function struct decla
  * @Author: qihoo360
  * @msg: 
  * @param 
  * @return: 
  */
 typedef struct _networkclientStruct{
    void (*getRequestDatanodate)(char*,char**,int*,int);
    void (*initALLByMaster)(char*,char*);
    void (*initTPSize)(int,int);
    void (*setServerConfig)(char*);
    void (*setDeviceConfig)(char*,char*,char*);
    void (*setManageKeyStore)(int);
    void (*postGatherData)(char*,char*,int);
    void (*postEventData)(const char*,char*,char*,int);
    void (*postEventDataWithAttachment)(const char*, const char*, const char*, const char*, int);
    void (*start)(void);
    void (*postHeartbeatData)(char*);
    void (*stop)(void);
    void (*startReRequestFromDB)(void);
    int (*registernet)(void);
    char* (*getSn)(void);
    char* (*getToken)(void);
    void (*getRequestData)(char*,long long,char**,int*,int);
    void (*postHeartbeatDataSync)(char*,char**,int*,int);
 }networkclientStruct;
/**
 * @name:   init networkmanager  define
 * @Author: qihoo360
 * @msg: 
 * @param   
 * @return: 
 */
networkclientStruct  networkclient  = {
    getRequestDatanodate,
    initALLByMaster,
    initTPSize,
    setServerConfig,
    setDeviceConfig,
    setManageKeyStore,
    postGatherData,
    postEventData,
    postEventDataWithAttachment,
    start,
    postHeartbeatData,
    stop,
    startReRequestFromDB,
    registernet,
    getSn,
    getToken,
    getRequestData,
    postHeartbeatDataSync,
};
/**
 * 初始化线程池
 *
 * @param consumerTPSize the consumer tp size
 * @param producerTPSize the producer tp size
 *
 */
void initTPSize_Base(int consumerTPSize, int producerTPSize){
    networkclient.initTPSize(consumerTPSize, producerTPSize);
}
/**
 * Sets server config.
 *
 * @param baseUrl the base url
 */
void setServerConfig_Base(char* baseUrl){
    if(strlen(baseUrl) < 4)
        log_i("networkmanager","base url error");
    networkclient.setServerConfig(baseUrl);
}
/**
 *
 * @param jsonConfig         
 * sn: 设备序列号
 * channel_id: 设备渠道号
 * equipment_type: 设备类型（"vehicle" 车机， "tbox" tbox）
 */
void setDeviceConfig_Base(char* sn,char* channel_id,char* equment_id){
    networkclient.setDeviceConfig(sn,channel_id,equment_id);
}
/**
 * Sets manage key store.
 *
 * @param mode the mode
 */
void setManageKeyStore_Base(int mode){
    networkclient.setManageKeyStore(mode);
}
/**
 * @name:   setHeartbeatInterval
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void setHeartbeatInterval_Base(int interval){
     timerObj.setInterval(interval,heartbeatManager);
}
/**
 * @name:   take timestamp 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
static inline long taketimestamp(char* _response){
    long long timestamp = 0;
    char temp[128] = {0};
    sscanf(_response,"%s%*[^:]:%lld",temp,&timestamp); 
    return timestamp;
}
/**
 * @name:   heartbeat
 * @Author: qihoo360
 * @msg:    
 * @param   
 * @return: 
 */
static void heartbeat_Base(void){
    char *l_response = NULL;
    int  l_retlength      =  0;
    l_response = (char*)malloc(256);
    memset(l_response,0,256);
    networkclient.postHeartbeatDataSync(NULL,&l_response,&l_retlength,256);
    long long retvalue = taketimestamp(l_response);
    if (retvalue > 0)
        clockobj.sync_clock(retvalue *1000);
    free(l_response);
}

/**
 * @name:   startReRequestFromDB
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
static void startReRequestFromDB_Base(void){
    networkclient.startReRequestFromDB();
}
/**
  * @name:   处理来自客户端的请求数据
  * @Author: qihoo360
  * @msg: 
  * @param 
  * @return: 20201111 add for lower power
  */
static void ReadClientData(int* _currenthandle,int _epoll)
{
    int     L_16RecvSize = 0;
	unsigned char  l_u8buffer[MAX_LENGTH] = {0};
    L_16RecvSize = read( *_currenthandle, &l_u8buffer[0], MAX_LENGTH);
    switch(L_16RecvSize)
    {
    case -1:
    case 0:
        epoll_ctl(_epoll,EPOLL_CTL_DEL,*_currenthandle,(void*)0);
        close(*_currenthandle);
        *_currenthandle = -1;
        return;
    default:
        break;
    }
    if(memcmp(l_u8buffer,"up",strlen("up")) == 0){
      lowerpower_exit();
    }
    else if(memcmp(l_u8buffer,"down",strlen("down")) == 0){
      lowerpower_enter();
    }
    else{
        if(lowerpower_states() == 1)
            lowerpower_exit();
        else
            lowerpower_enter();
    }
}
/**
  * @name:   sendnolock
  * @Author: qihoo360
  * @msg:    设置非阻塞模式
  * @param 
  * @return: 20201111 add for lower power
  */
static int setnoblock(int sock) 
{
	int opts;
    opts = fcntl(sock,F_GETFL);
    if (opts < 0 ){
        perror("setnoblock error");
        return 1;
    }
    opts  =  opts | O_NONBLOCK;
    if(fcntl(sock,  F_SETFL, opts ) < 0){
    	perror("setnoblock");
    	return 1;
    }
    return 0;
}
/**
  * @name:   低功耗处理线程
  * @Author: qihoo360
  * @msg:    
  * @param 
  * @return: 20201111 add for lower power
  */
static void *threadrun_socket(void *args)
{
  struct epoll_event ev, events[5];
  int connect_fd = -1,epollfd = -1,client_LP = -1;
  int ret = 0,l_intCurrentEvent = -1,connfd = -1;
  char snd_buf[MAX_LENGTH] = {0};
  int i = 0,trycnt = 0,len = 0;
  struct sockaddr_un srv_addr;
  struct sockaddr_un clt_addr;
  connect_fd=socket(PF_UNIX,SOCK_STREAM,0);
  if(connect_fd<0)
    perror("cannot create communication socket for lp server");
  srv_addr.sun_family = AF_UNIX; 
  strcpy(srv_addr.sun_path, UNIX_DOMAIN); 
  srv_addr.sun_path[0]=0;
  int len_bind = strlen(UNIX_DOMAIN) + offsetof(struct sockaddr_un, sun_path); 
  bind(connect_fd, (struct sockaddr*)&srv_addr, len_bind);
  ev.events = EPOLLIN;//no edge trigger
  ev.data.fd = connect_fd;
  epollfd = epoll_create(5);
  if(epollfd < 0)
    perror("create epoll error");
  if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connect_fd, &ev)<0)
	perror("epoll_ctl add error");
  ret = listen(connect_fd,1);
  if(ret == -1)
    perror("listen error");
  pthread_detach(pthread_self());
  for(;;){
		l_intCurrentEvent = epoll_wait(epollfd,events,5,-1);//block for event
		if(l_intCurrentEvent == -1){
			perror("epoll_wait  return fail");
        }
		else
        {
			for(i =0;i<l_intCurrentEvent;i++)
			{
                int l_fd = events[i].data.fd;
				if(l_fd == connect_fd)
				{
					if(events[i].events&EPOLLIN)
					{
                        len=sizeof(clt_addr); 
						if((connfd = accept(connect_fd,(struct sockaddr*)&clt_addr,&len))>0)
						{
                            client_LP = connfd;//如有多个连接 则只读取最新的连接
							ev.data.fd = connfd;
							ev.events = EPOLLIN | EPOLLET ;
							setnoblock(connfd);
							if(-1 == epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev))
							{
								perror("epoll_ctl:conn_sock\n");
							}
						}
					}
				}
				else if((l_fd == 0x00)||(-1 == l_fd))
				{
					perror("epoll_wait error or time out!!!\n");
				}
				else
				{
					if(events[i].events&EPOLLIN)//read data
					{
						ReadClientData(&client_LP,epollfd);
					}
				}
			}
        }
  }
  close(connect_fd);
}
/**
 * @name:   创建SOCKET线程
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 20201110 add for lower power
 */
static pthread_t _threadsocket;
static void createthreadsocket(void){
    int stacksize = 1*1024*1024;//1M
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr); 
	if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0)
		log_i("networkmonitor","statcksize set error");
	pthread_create(&_threadsocket,&attr,threadrun_socket,(void*)0);
	if((ret = pthread_attr_destroy(&attr)) != 0)
		log_i("networkmonitor","thread attr destory error");  
}
/**
 * @name:   关闭SOCKET线程
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 20201110 add for lower power
 */
static void shutdownsocket(void){
    pthread_cancel(_threadsocket); 
}
/**
 * @name:   NewNetworkManager
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void newNetworkManager_Base(char *pemPath,char *workDir)
{
    networkclient.initALLByMaster(pemPath,workDir);//create idps,set workdir
    //heartbeatManager = timerObj.newtime(heartbeat_Base);
    //reRequestManager = timerObj.newtime(startReRequestFromDB_Base);
    //timerObj.setInterval(60,reRequestManager);
    //createthreadsocket();//add 20201111 for lower power
}
/**
 * @name:   FreeNetWorkManager
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void freeNetWorkManager_Base(void){
    timerObj.freetimer(heartbeatManager);
    timerObj.freetimer(reRequestManager);
    shutdownsocket();//add 20201111 for lowerpower
}
/**
 * @name:   
 * @Author: qihoo360
 * @msg: 
 * @param   
 * @return: 
 */
void startNetworkRequestManager_Base(void){
    networkclient.start();
    // timerObj.starttime(heartbeatManager);
    // timerObj.starttime(reRequestManager);
}
/**
 * Post gather data boolean.,
 * 非原子操作
 *
 * @param url  网络请求的url
 * @param body 网络请求的数据体，数组形式的json字符串，比如如下形式body体
 *   [{
 *      "ips": [
 *      "10.18.115.141:80"
 *       ],
 *       "package_name": "com.android.wallpaperbackup"
 *    }]
 * @return 成功 true, 失败 false
 */
void postGatherData_Base(char* url, char* body, int policyPriority){
    networkclient.postGatherData(url, body, policyPriority);
}
/**
 * @name:   postEventData_Base
 * @Author: qihoo360
 * @msg: 
 * @param   body is json string
 * @return: 
 */
void postEventDatawithpath(const char* policyId, char* body, char* path, char* ticketId, int policyPriority){
    if (path == NULL) {
        networkclient.postEventData(policyId, body, ticketId, policyPriority);
    }
    else
    {
       networkclient.postEventDataWithAttachment(policyId ,body, ticketId,path, policyPriority);
    }
}
/**
 * 提交包含附件的事件型数据,原子操作
 *
 * url    网络请求的url//写死
 * policy_status 策略状态通过原子链结果判断
 * @param policy_id     策略id
 * @param data          网络请求的数据体，形式同postGatherData的body
 * @return 成功 true, 失败 false
 */
void postEventData_Base(const char* policy_id,char* data,char* ticket_id, int priority){
    networkclient.postEventData(policy_id, data, ticket_id, priority);
}
/**
 * Gets policy data.
 * 非原子操作，策略管理器，初始化时主动触发
 *
 * @param url        the url
 */
void getUrlRequestData_Base(const char* url, long long updateTime,char** _response,int* _length,int _responselen){
    networkclient.getRequestData((char*)url,updateTime,_response,_length,_responselen);
}

/**
 * Gets policy data.
 * 非原子操作，策略管理器，初始化时主动触发
 *
 * @param url        the url
 */
void getUrlRequestDatanodate(char* url,char** _response,int* _length,int _responselen){
    networkclient.getRequestDatanodate(url,_response,_length,_responselen);
}
/**
 * @name:   stopNetworkRequestManager 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void stopNetworkRequestManager_Base(void){
    timerObj.stoptimer(heartbeatManager);
    timerObj.stoptimer(reRequestManager);
}
/**
 * @name:   getSn 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
char* getSn_Base(void){
    return networkclient.getSn();
}

char* getToken_Base(void){
    return networkclient.getToken();
}
 
/**
 * @name:   outer method
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
networkMangerMethod  networkMangerMethodobj = {
   initTPSize_Base,
   setServerConfig_Base,
   setDeviceConfig_Base,
   setManageKeyStore_Base,
   setHeartbeatInterval_Base,
   newNetworkManager_Base,
   freeNetWorkManager_Base,
   startNetworkRequestManager_Base,
   postGatherData_Base,
   postEventDatawithpath,
   postEventData_Base,
   getUrlRequestData_Base,
   getUrlRequestDatanodate,
   stopNetworkRequestManager_Base,
   getSn_Base,
   getToken_Base,
   getSessionKeyApi,
   getManageKeyApi,
};

