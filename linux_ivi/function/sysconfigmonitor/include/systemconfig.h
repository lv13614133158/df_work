#ifndef __SYSTEMCONFIG_H
#define __SYSTEMCONFIG_H
#include <stdbool.h>

typedef int  (*systemListener)(char *,char *,long long ,int);


typedef struct PassNode{
	char name[32];
	char password[128];
	int count;
	struct PassNode *next;
}PassNode_t;

struct hardword_info{
	char sys_name[20]; //"linux"
	char machine[20];	//"x86_64"
	char host_name[40];	 // "edan"
	char release[50];	//"4.15.0-55-generic"
	char version[50];  
	unsigned long long ram_size;
	char cpu_model_name[50]; //Intel(R) Core(TM) i7-4790 CPU @ 3.60GHz
	int core_num;		//CPU number
};

typedef struct IpNode{
	char name[30];
	char ip[30];
	struct IpNode *next;
}IpNode_t;

/******************************************************
 * 
 * @func:启动文件监控
 * @param:void
 * @return: 
 * @note:注册监听前调用
 * 
 ******************************************************/
int startMonitor();

/******************************************************
 * 
 * @func:停止文件监控
 * @param:void
 * @return: 
 * @note:
 * 
 ******************************************************/
void stopMonitor();

/*******************************************************
 * 
 * @func:获取有效用户信息
 * @param:传递一个空指针地址，用于接收字符串
 * @return:成功返回0,失败返回负数
 * @note:底层调用malloc分配空间，赋值给result，
 * 		调用完毕要对result指针进行free 
 * @example: <li> char *ptr = NULL                </li>
 * 			 <li> get_valid_user_and_passwd(&ptr) </li>
 * 			 <li> if(ptr)						  </li>
 * 			 <li>	  free(ptr)					  </li>
 * 
 ******************************************************/
int get_valid_user_and_passwd(char **result);

/*******************************************************
 * 	@func:获取硬件信息 
 *  @outbuf:outbuf,空指针地址，在底层malloc空间，接收字符串
 * 	@return： 成功返回0,失败返回-1
 *  @note:使用完毕后需要对outbuf指针进行释放
 *  @example:  <li> char *ptr = NULL    </li>
 * 			   <li> get_hard_info(&ptr) </li>
 * 			   <li> if(ptr)             </li>
 *    		   <li> 	free(ptr)       </li>
 ******************************************************/
int get_hard_info(char **outbuf); 
/**
 *  @func:注册回调函数，返回注册id
 *  @param:obj,回调函数
 */
/******************************************************
 * 
 * @func: 注册监听函数
 * @param:
 * @return:返回注册id，用于注销回调监控
 * @note: 
 * 
 *****************************************************/
int registerSystemConfigListener(systemListener obj);

/******************************************************
 * 
 * @func: 注销监听函数
 * @param:注册时返回的id
 * @return
 * @note: 
 * 
 *****************************************************/
void unregisterSystemConfigListener(int id);

#endif