/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-08 02:58:11
 */ 
#include "cJSON.h"
#include "util.h"
#include <unistd.h>
#include <string.h>
#include "FileMonitor.h"
#include "Sql_FileMonitor.h"
#include "Base_networkmanager.h"
#include "myHmac.h"
#include "fileOption.h"
#include "inotifyAPI.h"
#include "isolationFile.h"
#include "websocketmanager.h"
#include "websocketclient.h"
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "common.h"
#include "base64.h"

#if MODULE_FILEMONITOR


static bool isInitial = false;
/**
 * @name:    val def
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
long long mConfig =  0;
list *mWatchPointMap = NULL;
/**
 * @name:   enum 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
enum ConfigItemOpcode{
    CONFIG_ITEM_OPCODE_REMOVE,
    CONFIG_ITEM_OPCODE_ADD,
    CONFIG_ITEM_OPCODE_UPDATE
};
/**
 * @name:   const val 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
const char *API_UPDATE_FILE_WATCH_MAP = "/api/v1/file_monitor_list";
/*************************instance function*****************************/

// 文件单独操作功能函数区
int getFingerprint(char* filePath,char **outbuf){
    unsigned char sha[20] = {0};
    unsigned char md5[16] = {0};
    char shaString[42]={0};
    char md5String[34]={0};
    int i;
    char *temp = NULL;

    cJSON *cJSONObj = cJSON_CreateObject();
    cJSON_AddStringToObject(cJSONObj,"path",filePath);

    Compute_file_sha1(filePath,shaString); 
    Compute_file_md5(filePath,md5String);
    
    cJSON_AddStringToObject(cJSONObj,"sha1",shaString);
    cJSON_AddStringToObject(cJSONObj,"md5",md5String);
    char *s = cJSON_PrintUnformatted(cJSONObj);
	if(cJSONObj)
		cJSON_Delete(cJSONObj);
    temp = (char *)malloc(strlen(s)+1);
    if(temp == NULL)
    {
        free(s);
        return -1;
    }
    memcpy(temp,s,strlen(s));
    *outbuf = temp;
    free(s);
    return 0;
}
/**
 * @name:   compareFingerprint
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
bool compareFingerprint_Base(char* filePath,char* desFingerprint){
    //compareFingerprint(srcFingerprint, desFingerprint);
    bool ret = false;
    char *outbuf = NULL;

    getFingerprint(filePath,&outbuf);
    if(strcmp(outbuf,desFingerprint) == 0)
    {
        ret = true;
    }
    free(outbuf);
    return ret;
}
/**
 * @name:   removeFile
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void removeFile_Base(const char* filePath){
    cmd_rm(filePath);
}
/**
 * @name: 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void moveFile_Base(char* src,char* des){
   cmd_mv(src, des);
}
/**
 * @name:   copyFile
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void copyFile(char* src,char* des){
    cmd_cp(src,des);
}
/**
 * @name: 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void isolateFile(char* filePath){
    IsolateFile(filePath);
}
/**
 * @name:   backupFile 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void backupFile_Base(char* filePath){
    backupFile(filePath);
}
/**
 * @name:   restoreFile
 * @Author: qihoo360
 * @msg: 
 * @param  mode:1  恢复隔离的文件，  2 恢复备份的文件
 *         filePath  待恢复文件的原始绝对路径
 * @return: 
 */
void restoreFile_Base(int mode, char* filePath){
    restoreFile(mode,filePath);
}
/**
 * @name:   changeOwner 
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void changeOwner(const char* userAndGroup, const char* filePath){
    cmd_chown(userAndGroup,filePath);
}
/**
 * @name:   changeMode
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void changeMode(const char* filePath, int mode){
    cmd_chmod(mode,filePath);
}
/**
 * @name:   ls
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void ls(char* dirPath) {
    //todo
}
/**
 * @name:   mount
 * @Author: qihoo360
 * @msg: 
 * @param  
 * @return: 
 */
void mount(char* para){
/**
 * todo
*/
}
/**
 * @name:   umount 
 * @Author: qihoo360
 * @msg: 
 * @param 
 * @return: 
 */
void unmount(char* para){
/**
 * todo
*/
}


// 链表区
static void updateLastWatchPointTimestamp(long long timestamp){
    mConfig = timestamp;
}

static inline void initlist(list* _inputlist){
	list_destroy(_inputlist);
	list_init(_inputlist,free);
}

static void freeliststack(list* _inputlist){
    list_destroy(_inputlist);
    free(_inputlist);
}
 
static void addWatchPointfornative(list* _list){
    list_elmt *cur = _list->head;
    while(cur != NULL)
    {
        WatchPointMap *body =	(WatchPointMap*)cur->data;
        addWatchPoint(body->path,body->mask);
        cur = cur->next;
    }
}

static bool InserNodeFileMonitor(list* _list,char* path,int mask,int status){
    list_elmt *old_elmt = NULL;
    list_elmt *head = _list->head;
	list_elmt *cur = head;

    if(status == CONFIG_ITEM_OPCODE_REMOVE)
    {
        while(cur !=NULL)
        {
            WatchPointMap *body =	(WatchPointMap*)cur->data;
            if((strcmp(path,body->path) == 0)&&(mask == body->mask))
            { 
                free(body->path);
                list_delindex(_list,old_elmt);
                return true;
            } 
            old_elmt = cur;
	     	cur = cur->next;              
        }
        return false;
    }
    else if(status == CONFIG_ITEM_OPCODE_ADD)
    {
        WatchPointMap* node = (WatchPointMap *)malloc(sizeof(WatchPointMap));		
        memset(node,0,sizeof(WatchPointMap));
        node->mask = mask;
        node->path = (char*)malloc(strlen(path) + 2);
        memset(node->path,0,strlen(path) + 2);
        memcpy(node->path,path,strlen(path));
        list_ins_next(_list,NULL,node);	
        return true;
    }
    else if(status == CONFIG_ITEM_OPCODE_UPDATE)
    {
        while(cur !=NULL)
        {
            WatchPointMap *body = (WatchPointMap*)cur->data;
            if((strcmp(path,body->path) == 0))
            { 
                body->mask = mask;
                return true;
            } 
            old_elmt = cur;
	     	cur = cur->next;              
        }
    }
    return false;
}

static bool addWatchPoint_Base(char* path, int mask,list* _list){
    if(InserNodeFileMonitor(_list,path,mask,CONFIG_ITEM_OPCODE_ADD)){
        return true;
    }   
    return false;
}

static bool removeWatchPoint_Base(char* path,int mask,list* _list){
    if(InserNodeFileMonitor(_list,path,mask,CONFIG_ITEM_OPCODE_REMOVE)){
        return true; 
    }
    return false;
}

bool updateWatchPoint_Base(char* path, int mask,list* _list) {
    if(InserNodeFileMonitor(_list,path,mask,CONFIG_ITEM_OPCODE_UPDATE)){
        return true;
    }
    return false;
}

static int fileIntegrityCheck(char* filePath, char *old_md5)
{
    char sha1String[42]={0};
    char md5String[34]={0};
    struct stat buf;
    DIR *dirp = NULL;
    char mode[11] = {0};
    long long timestamp = 0;
    cJSON *cjson_data = NULL;
    char *s = NULL;

    if ((dirp = opendir(filePath)) != NULL)  //目录且存在
    {
        closedir(dirp);
        stat(filePath,&buf);
    }
    else
    {
        timestamp = clockobj.get_current_time();
        cjson_data = cJSON_CreateObject();

        cJSON_AddStringToObject(cjson_data,"path",filePath);
        cJSON_AddStringToObject(cjson_data,"event_source","");
        cJSON_AddNumberToObject(cjson_data,"action",2);
        if(access(filePath,F_OK) == 0)  //文件且存在
        {
            Compute_file_md5(filePath,md5String);
            if (strncmp(md5String, old_md5, sizeof(md5String)))
            {
                Compute_file_sha1(filePath,sha1String);

                stat(filePath,&buf);
                mode_to_letters(buf.st_mode,mode);
                cJSON_AddStringToObject(cjson_data,"md5",md5String);
                cJSON_AddStringToObject(cjson_data,"old_md5",old_md5);
                cJSON_AddStringToObject(cjson_data,"sha1",sha1String);
                cJSON_AddStringToObject(cjson_data,"sha256","");
                cJSON_AddNumberToObject(cjson_data,"size",buf.st_size);
                cJSON_AddNumberToObject(cjson_data,"uid",buf.st_uid);
                cJSON_AddNumberToObject(cjson_data,"gid",buf.st_gid);
                cJSON_AddStringToObject(cjson_data,"mode",mode);

                cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
                s = cJSON_PrintUnformatted(cjson_data);
                websocketMangerMethodobj.sendEventData(EVENT_TYPE_FILE_CHANGED, s);
            }
        }
        else
        {
            cJSON_AddStringToObject(cjson_data,"md5","");
            cJSON_AddStringToObject(cjson_data,"old_md5",old_md5);
            cJSON_AddStringToObject(cjson_data,"sha1","");
            cJSON_AddStringToObject(cjson_data,"sha256","");
            cJSON_AddNumberToObject(cjson_data,"size",0);
            cJSON_AddNumberToObject(cjson_data,"uid",0);
            cJSON_AddNumberToObject(cjson_data,"gid",0);
            cJSON_AddStringToObject(cjson_data,"mode","");

            cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
            s = cJSON_PrintUnformatted(cjson_data);
            websocketMangerMethodobj.sendEventData(EVENT_TYPE_FILE_CHANGED, s);
        }

        if (cjson_data)
            cJSON_Delete(cjson_data);
        if (s)
            free(s);
    }

    return 0;
}

void setWatchPoint(char* _input)
{
    int isAdd = 1;
    if(_input != NULL)
    {             
        cJSON *cJSONList = cJSON_Parse(_input);
        int size =cJSON_GetArraySize(cJSONList);
        if (size == 0) {                         
            cJSON_Delete(cJSONList);
            addWatchPointfornative(mWatchPointMap);                       
            return;                     
        }

        for (int i = 0; i < size; i ++) 
        {
            cJSON *child = cJSON_GetArrayItem(cJSONList, i);
            char  *path = cJSON_GetObjectItem(child,"watch_path")->valuestring;
            int event = cJSON_GetObjectItem(child,"event_level")->valuedouble;
            char  *md5 = cJSON_GetObjectItem(child,"md5")->valuestring;

            fileIntegrityCheck(path, md5);
            int mask = 0;
            int updateFlag = 0;
    
            cJSON *maskObj = cJSON_GetObjectItem(child,"watch_event");
            if(maskObj != NULL)
            {
                if(maskObj->type == 8) //number
                {
                    mask = maskObj->valueint;
                }
                else if(maskObj->type == 16)
                {
                    mask = 0;
                    char *p = NULL;
                    p = maskObj->valuestring;
                    mask = atoi(p);
                }
            }

            if(isAdd)// Add
            {
                list_elmt *head = mWatchPointMap->head;
                list_elmt *cur = head;
                while(cur != NULL)//update
                {
                    WatchPointMap *body =	(WatchPointMap*)cur->data;
                    if((strcmp(path,body->path) == 0))
                    {
                        updateWatchPoint_Base(path,mask,mWatchPointMap);
                        updateFlag = 1;
                        break;
                    }
                    cur = cur->next;
                }
                if(updateFlag == 0)//add
                {
                    addWatchPoint_Base(path,mask,mWatchPointMap);
                }             
            }
            else// Delect
            {
                removeWatchPoint_Base(path,mask,mWatchPointMap);
            }
        }
        cJSON_Delete(cJSONList); 
        addWatchPointfornative(mWatchPointMap);
    }
}


// 函数回调区
static void saveFileEvent(char *eventSource,char* filePath, int mask){
    char spdlog[255] = {0};
    snprintf(spdlog, 255,"eventSource:%s,filePath:%s,mask0%x", eventSource, filePath, mask);
    log_d("filemonitor", spdlog);
}

static int onFileMonitorEvent(char* filePath, int mask,char *processInfo) {
    //define IN_IGNORED   0x00008000 /* File was ignored.
    if(mask == 32768)   //File was ignored
    {
        return 0;
    }
    saveFileEvent(processInfo,filePath, mask);

    char sha1String[42]={0};
    char md5String[34]={0};
    int i = 0;
    struct stat buf;
    DIR *dirp = NULL;
    char mode[11] = {0};
    unsigned char sha1[20] = {0},md5[16] = {0};
    long long timestamp = clockobj.get_current_time();
    cJSON *cjson_data = cJSON_CreateObject();
    cJSON_AddStringToObject(cjson_data,"path",filePath);
    cJSON_AddStringToObject(cjson_data,"event_source",processInfo);
    cJSON_AddNumberToObject(cjson_data,"action",mask);
    if((dirp = opendir(filePath)) != NULL)  //目录且存在
    {
        closedir(dirp);
        stat(filePath,&buf);
        mode_to_letters(buf.st_mode,mode);
        cJSON_AddNumberToObject(cjson_data,"size",buf.st_size);
        cJSON_AddNumberToObject(cjson_data,"uid",buf.st_uid);
        cJSON_AddNumberToObject(cjson_data,"gid",buf.st_gid);
        cJSON_AddStringToObject(cjson_data,"mode",mode);
        cJSON_AddStringToObject(cjson_data,"md5","");
        cJSON_AddStringToObject(cjson_data,"old_md5","");
        cJSON_AddStringToObject(cjson_data,"sha1","");
        cJSON_AddStringToObject(cjson_data,"sha256","");
    }
    else
    {   if(access(filePath,F_OK) == 0)  //文件且存在
        {
            Compute_file_md5(filePath,md5String);  
            Compute_file_sha1(filePath,sha1String); 
          
            stat(filePath,&buf);
            mode_to_letters(buf.st_mode,mode);
            cJSON_AddStringToObject(cjson_data,"md5",md5String);
            cJSON_AddStringToObject(cjson_data,"old_md5","");
            cJSON_AddStringToObject(cjson_data,"sha1",sha1String);
            cJSON_AddStringToObject(cjson_data,"sha256","");
            cJSON_AddNumberToObject(cjson_data,"size",buf.st_size);
            cJSON_AddNumberToObject(cjson_data,"uid",buf.st_uid);
            cJSON_AddNumberToObject(cjson_data,"gid",buf.st_gid);
            cJSON_AddStringToObject(cjson_data,"mode",mode);
        }
        else
        {
            cJSON_AddStringToObject(cjson_data,"md5","");\
            cJSON_AddStringToObject(cjson_data,"old_md5","");
            cJSON_AddStringToObject(cjson_data,"sha1","");
            cJSON_AddStringToObject(cjson_data,"sha256","");
            cJSON_AddNumberToObject(cjson_data,"size",0);
            cJSON_AddNumberToObject(cjson_data,"uid",0);
            cJSON_AddNumberToObject(cjson_data,"gid",0);
            cJSON_AddStringToObject(cjson_data,"mode","");
        }
    }
    cJSON_AddNumberToObject(cjson_data,"timestamp",timestamp);
    char *s = cJSON_PrintUnformatted(cjson_data);
    if(cjson_data)
        cJSON_Delete(cjson_data);
    websocketMangerMethodobj.sendEventData(EVENT_TYPE_FILE_CHANGED, s);

    if(s)
	    free(s);
    return 0;
}

static void onBackUpInsufficientSpaceEvent(void) {
/**
 * todo
*/
}

static void onIsolateInsufficientSpaceEvent(void) {
/**
 * todo
*/  
}

static void insufficientSpaceEventCallback(int flag)
{	
    if(flag == 1)
    {
        onIsolateInsufficientSpaceEvent();
    }
    else if(flag == 2)
    {
        onBackUpInsufficientSpaceEvent();
    }
}

// 模块设置区域
void updateWatchPoint(char *response){
    setWatchPoint(response);
    //free(response);
}

void initWatchPoint(void) {
    list *head = NULL;
	head = (list *)malloc(sizeof(list));
	if (head == NULL)
	{
		log_e("filemonitor","fatal error,Out Of Space");
		return;
	}	
	list_init(head,free);
    mWatchPointMap = head;
}

static void callinit(void){
    //注册文件监控回调
    registerFileMonitorListener(onFileMonitorEvent);
    //注册空间监控回调
    registerInsufficientSpaceEventListener(insufficientSpaceEventCallback);
}

void initFileMonitor(void){
    //注册回调接口，注册文件监听回调和空间溢出回调
    callinit();
    initWatchPoint();//init list
}

void startFileMonitor_Base(void){
    if(isInitial == true)
        return;
    startFileMonitor();
    isInitial = true;
}

void stopFileMonitor(void){
    //stopMonitor();
    removeAllWatchPoint();
}

void freeFileMonitor(void){
    freeliststack(mWatchPointMap);    
}

void setIsolateDirectoryThreshold(int threshold){
    setOfflimitDirectory(threshold);
}

void setBackupDirectoryThreshold(const int threshold){
    setBackupDirectory(threshold);
}

void setFileWorkDirectory(char *directory)
{
    if(directory == NULL)return;
    setFileWorkDir(directory);
}

FileMonitor FileMonitorObj = {
    initFileMonitor,
    startFileMonitor_Base,
    stopFileMonitor,
    freeFileMonitor,
    setBackupDirectoryThreshold,
    setIsolateDirectoryThreshold,
    setFileWorkDirectory,
    updateWatchPoint,
    changeMode,
    changeOwner,
    restoreFile_Base,
    backupFile_Base,
    isolateFile,
    copyFile,
    moveFile_Base,
    removeFile_Base,
    compareFingerprint_Base,
    getFingerprint,
};

#endif
