#ifndef __INOTIFYAPI_H
#define __INOTIFYAPI_H

#include <sys/inotify.h>

struct monitorEvent{
    char path[512];
    int mask;
};

//path,mask,processInfo
typedef int (*listener)(char *,int ,char *);

/* the following are legal, implemented events that user-space can watch for */
//#define ACCESS       0x00000001  /* File was accessed */
#define MODIFY           0x00000002  /* File was modified */
#define OPEN             0x00000020  /* File was opened */
#define CREATE           0x00000100  /* Subfile was created */
#define CLOSE_WRITE      0x00000008  /* Writtable file was closed */
#define CLOSE_NOWRITE    0x00000010  /* Unwrittable file closed */
#define DELETE           0x00000200  /* Subfile was deleted */
#define DELETE_SELF      0x00000400  /* Self was deleted */

#define ISDIR            0x40000000  /* event occurred against dir */



//启动文件监控引擎
int startFileMonitor();
//增加文件、文件夹的文件监视事件，dirs 表示文件或目录集合，以 | 进行分割
//mask 表示监控的事件触发类型，
int addWatchPoint(const char *dirs, int mask);
//删除文件、文件夹的文件监视事件
void removeWatchPoint(char *dirs);
//删除所有的文件监视点
void removeAllWatchPoint();
//注册指定文件、文件夹的指定事件监听器，返回监视器id
int registerFileMonitorListener(listener obj);
//注销指定事件监听器
void unRegisterFileMonitorListener(int id);
//
void inotify_all_observer(struct inotify_event *event);


#endif