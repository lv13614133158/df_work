/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-08 23:35:14
 */
#include <sys/types.h>
#include <signal.h>
#include "myList.h"
#include "cJSON.h"
#include "idps_main.h"
#include "clocker.h"
#include "myEqual.h"
#include "myHashCode.h"
#include "myHashMap.h"
#include "Base_networkmanager.h"
#include "CommonTimer.h"
#include "websocketmanager.h"
#include "ProcessMonitor.h"
#include "processCheck.h" 
#include "ConfigParse.h"

#if MODULE_PROCESSMONITOR 
#define  PROCESS_FUN_SWITCH     2

static timerIdps *timerProcessMonitor = NULL;
static list *whiteProcessList = NULL;
static int mProcessCollectPeriod = 30;
static int processMonitorInitFlag = 0; 
static void ProcessCheck(void);


#define  createtimer() timerProcessMonitor = timerObj.newtime(ProcessCheck);


/**
 * @name:   ProcessCheck
 * @Author: qihoo360
 * @msg:    
 * @param 
 * @return: 
 */
static void ProcessCheck(void)
{
   switch (PROCESS_FUN_SWITCH)
   {
   case 2: // 云端白名单对比， 每次上传所有 pid+进程 
      checkProcessCloud();
      break;

   case 3: // 本地白名单对比， 每次上传单个 进程全部信息
      checkProcessLocal();
      break;

   case 4:  // 进程变化，  每次上传所有 进程全部信息
      checkProcessList();
      break;   
   
   default: //进程变化， 每次上传单个 进程全部信息
      checkProcessChange();
      break;
   }

   checkZombieProcess();
}

void initProcessWhiteList() 
{
   list *head = NULL;
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_e("processmonitor","fatal error,Out Of Space");
		return;
	}
	//list_destroy(head);
	list_init(head, free);
   whiteProcessList = head;
   processCheckInit(whiteProcessList);
}
/**
 * @name:   addProcessWhiteList
 * @Author: qihoo360
 * @msg:    
 * @param 
 * @return: 
 */
static int addProcessWhiteList(char *process)
{
   if(addliststring(whiteProcessList,process)){
      return true;
   }
   return false;
}
/**
 * @name:   removeProcessWhiteList
 * @Author: qihoo360
 * @msg:    
 * @param 
 * @return: 
 */
static int removeProcessWhiteList(char *process)
{
   if(delliststring(whiteProcessList,process)){
      return true;
   }
   return false;
}

void setProcessWhiteList(char *config)
{
   addProcessWhiteList(config);
}

char* getLocalProcessWhileList()
{
   char* localList = getProcessWhileList();
   if(!localList)
   {  
      localList = get_process_information();
      if(setProcessWhileList(localList) < 0)
      {
         log_e("processmonitor","write local WhileList error!");
      }
   }
   return localList;
}

void updateProcessWhiteList(char* whiteProcessListlocal)
{
	cJSON *cJSONList = NULL;
   if(PROCESS_FUN_SWITCH == 3)
   {
      char *list = getLocalProcessWhileList();
      cJSONList = cJSON_Parse(list);
      free(list);
   }
   else{
      cJSONList = cJSON_Parse(whiteProcessListlocal);
   }  
   if (!cJSONList)return;  

	int size =cJSON_GetArraySize(cJSONList);
   if (size == 0) {                         
      cJSON_Delete(cJSONList);    
      //free(whiteProcessListlocal);                   
      return;                     
   }
	for (int i = 0; i < size; i ++) 
   {
		cJSON *child = cJSON_GetArrayItem(cJSONList, i);
		char*  process_name = cJSON_GetObjectItem(child,"process_name")->valuestring;
		int    process_id   = cJSON_GetObjectItem(child,"process_id")->valueint;
      addProcessWhiteList(process_name);
	}
	cJSON_Delete(cJSONList); 
   //free(whiteProcessListlocal);
} 

void setProcessCollectPeriod(int period)
{
   if(period > 0){
      mProcessCollectPeriod = period;
   }  
   else{
      mProcessCollectPeriod = 30;
   } 
}

void initProcessMonitor(int period)
{
   if (!processMonitorInitFlag)
   {
      setProcessCollectPeriod(period);
      initProcessWhiteList();
      createtimer();
      processMonitorInitFlag = 1;
   }
}

void startProcessMonitor(void)
{
   if(timerProcessMonitor)
   {
      timerObj.setInterval(mProcessCollectPeriod, timerProcessMonitor);
      timerObj.starttime(timerProcessMonitor);
   }
}
/**
 * @name:   stopProcessMonitor
 * @Author: qihoo360
 * @msg:    
 * @param 
 * @return: 
 */
void stopProcessMonitor(void)
{
   if(timerProcessMonitor)
      timerObj.stoptimer(timerProcessMonitor);
}
/**
 * @name:   killProcess
 * @Author: qihoo360
 * @msg:    
 * @param   :pid  > 0
 * @return: 
 */
void killProcess(int pid)
{
   if(pid <=0)
      return;
   kill(pid, SIGKILL);
   printf("\nRPC success nice!\n\n");
}

ProcessMonitor ProcessMonitorObj={
   initProcessMonitor,
   startProcessMonitor,
   stopProcessMonitor,
   updateProcessWhiteList,
   setProcessWhiteList,
   killProcess
};
#endif
