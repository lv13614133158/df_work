#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <dirent.h>

#include "isolationFile.h"
#include "debug.h"
#include "fileOption.h"
#include "myHmac.h"

#define ISOPATHSIZE 255
#define BACKUPPATHSIZE 255
#define PATH_MAXSIZE 272
#define MAX_LISTENERLEN 5  //隔离目录或者备份目录超过阈值时，最大回调个数

static spaceListener mlistens[MAX_LISTENERLEN]={0};
static char isolationPATH[ISOPATHSIZE+1];
static char backupDirectory[BACKUPPATHSIZE+1];
static long ISODIRSIZETHRESHOLD; //隔离目录的阈值
static long BACKUPDIRTHRESHOLD; //备份目录的阈值

static int setWorkDirFlag=0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static long get_file_size(char *dirname);
static int is_dir_exist(const char *dir_path);
static int is_file_exist(const char *file_path);
static void _array_to_string(const unsigned char *array,const int len,char *string);


//设置工作目录，在该目录下创建两个子目录，用于文件隔离目录和文件备份目录
int setFileWorkDir(const char *workDir)
{
    int ret;
    struct stat src_stat;
    char isodir[] = "isoDir/";
    char backupdir[] = "backupDir/";
    if(workDir == NULL)
    {
        DERROR("The parameter <workDir> cannot be NULL\n ");
        return -1;
    }
    ret = is_dir_exist(workDir);
    if(ret < 0)
    {
        DERROR("Directory <%s> does not exist\n",workDir);
        return -2;
    }
    memset(isolationPATH,0,ISOPATHSIZE+1);
    memset(backupDirectory,0,BACKUPPATHSIZE+1);
    memcpy(isolationPATH,workDir,strlen(workDir));
    if(isolationPATH[strlen(workDir)-1]!='/')
        isolationPATH[strlen(workDir)]='/';
    memcpy(backupDirectory,workDir,strlen(workDir));
    if(backupDirectory[strlen(workDir)-1]!='/')
        backupDirectory[strlen(workDir)]='/';
    strncat(isolationPATH,isodir,sizeof(isolationPATH) -1);
    strncat(backupDirectory,backupdir,sizeof(backupDirectory) -1);
    if( stat(workDir,&src_stat))
    {
        DERROR("stat: %s\n",strerror(errno));
        return -3;
    }
    ret = is_dir_exist(isolationPATH);
    if(ret < 0)
    {
        if( -1 == mkdir(isolationPATH,src_stat.st_mode))
        {
            DERROR("mkdir: %s\n",strerror(errno));
            return  -4;
        }
    }
    ret = is_dir_exist(backupDirectory);
    if(ret < 0)
    {
        if( -1 == mkdir(backupDirectory,src_stat.st_mode))
        {
            DERROR("mkdir: %s\n",strerror(errno));
            return  -4;
        }  
    }
    setWorkDirFlag = 1;
    return 0;

}

/*
*   @brief 配置备份路径
*   @param threshold 目录最大容纳的大小(MB)
*
*   @return :NULL
*/
void setBackupDirectory(const int threshold)
{
    BACKUPDIRTHRESHOLD =threshold*1024*1024*0.85; //B
}


/*
*   @brief 配置文件隔离目录存储阈值
*   @param threshold 目录最大容纳大小（MB）
*
*   @return 无返回值
*/
void setOfflimitDirectory(int threshold)
{
    ISODIRSIZETHRESHOLD = threshold*1024*1024*0.85;
}
/*
*   @brief 备份文件
*   @param filepath 文件路径，绝对路径
*   @return null
*/
void backupFile(char *filePath)
{
    static struct stat my_stat;
    char destPath[PATH_MAXSIZE]={0};
    int backupContentSize;
    int ret,len;
    char name[33]={0};

    if(filePath == NULL)
        return ;
    if(setWorkDirFlag!=1)
    {
        DERROR("please Please call the setFileWorkDir function\n");
        return ;
    }
    if(-1==lstat(filePath,&my_stat))
    {
        return;
    }
    else if(S_ISDIR(my_stat.st_mode))
    {
        DERROR("don't backup DIR\n");
        return;
    }

    ret = Compute_string_md5(filePath,strlen(filePath),name);
    if(ret != 0)
        return;

    memcpy(destPath,backupDirectory,strlen(backupDirectory));
    if((strlen(backupDirectory)+strlen(name) < PATH_MAXSIZE))
    {
        len = strlen(name);
    }
    else
    {
        len = PATH_MAXSIZE-strlen(backupDirectory);
    }
    strncat(destPath,name,len);
    cmd_cp(filePath,destPath);

    backupContentSize = get_file_size(backupDirectory);
    if(backupContentSize > BACKUPDIRTHRESHOLD)
    {
        int i;
        for(i = 0;i<MAX_LISTENERLEN;i++)
        {
            if(mlistens[i]!=0)
                mlistens[i](2);//2表示备份目录超出阈值
        }
        // Java_Level_Method_insufficient_SpaceEvent_Callback(2);
    }
}

/*
*   @brief 隔离文件
*   @param filePath 文件名为绝对路径
*
*   @return 无返回值
*/
void IsolateFile(const char *filePath)
{
    char destPath[PATH_MAXSIZE]={0};
    int isolationContentSize;
    int ret,len;
    char name[33]={0};
    static struct stat my_stat;
    if(filePath == NULL)
        return ;
    if(is_dir_exist(filePath)== 0)
    {
        DERROR("don`t isolation directory [%s]\n",filePath);
    }

    if(setWorkDirFlag!=1)
    {
        DERROR("please Please call the setFileWorkDir function\n");
        return ;
    }
    if(is_file_exist(filePath)!=0)
    {
        DERROR("file [%s] not exist\n",filePath);
        return;
    }
    if(-1==lstat(filePath,&my_stat))
        return;
    else if(S_ISDIR(my_stat.st_mode))
    {
        DERROR("don't Isolation DIR\n");
        return;
    }

    ret = Compute_string_md5(filePath,(unsigned int)strlen(filePath),name);
    if(ret != 0)
        return;

    memcpy(destPath,isolationPATH,strlen(isolationPATH));
    if((strlen(isolationPATH)+strlen(name) < PATH_MAXSIZE))
    {
        len = strlen(name);
    }
    else
    {
        len = PATH_MAXSIZE-strlen(isolationPATH);
    }
    strncat(destPath,name,len);
    cmd_mv(filePath,destPath);

    isolationContentSize = get_file_size(isolationPATH);
    if(isolationContentSize > ISODIRSIZETHRESHOLD)
    {
        int i;
        for(i = 0;i<MAX_LISTENERLEN;i++)
        {
            if(mlistens[i]!=0)
                mlistens[i](1);//1表示备份目录超出阈值
        }
        // Java_Level_Method_insufficient_SpaceEvent_Callback(1);
    }
}

/*
*   @brief 恢复文件到原来的位置
*   @param <mode> mode ==1 恢复隔离的文件，mode == 2 恢复备份的文件
*   @param filePath 文件名，文件名为隔离前文件的绝对路径
*
*   @return 无返回值
*/
void restoreFile(int mode,const char *filePath)
{
    if((mode !=1)&&(mode != 2))
    {
        DERROR("please set restore mode <1 | 2>\n");
        return ;
    }
    if(filePath == NULL)
        return ;
    if(setWorkDirFlag!=1)
    {
        DERROR("please Please call the setFileWorkDir function\n");
        return ;
    }
    int ret,len;  
    char srcbuf[PATH_MAXSIZE]={0};
    char name[33]={0};

    ret = Compute_string_md5(filePath,strlen(filePath),name);
    if(ret != 0)
        return;
 
    if(mode == 1) //恢复隔离文件
    {
        memcpy(srcbuf,isolationPATH,strlen(isolationPATH));

        if((strlen(isolationPATH)+strlen(name) < PATH_MAXSIZE))
        {
            len = strlen(name);
        }
        else{
            len = PATH_MAXSIZE-strlen(isolationPATH);
        }
        strncat(srcbuf,name,len);
    }else if(mode == 2)//恢复备份文件
    {
        memcpy(srcbuf,backupDirectory,strlen(backupDirectory));
        if((strlen(backupDirectory)+strlen(name) < PATH_MAXSIZE))
        {
            len = strlen(name);
        }
        else{
            len = PATH_MAXSIZE-strlen(backupDirectory);
        }
        strncat(srcbuf,name,len);
    }  
    ret = is_file_exist(srcbuf);
    if(ret != 0)
    {
        DERROR("file [%s] not isolation or backup\n",filePath);
        return;
    }
    //2.移动到原路径
    cmd_mv(srcbuf,filePath);
}

/*
*   @brief 判断文件是否存在
*   @param dir_path 目录名称，包含路径
*
*   @return 存在返回0，不存在返回-1
*/
static int is_dir_exist(const char *dir_path)
{
    DIR *fp = NULL;
    if(dir_path == NULL)
        return -1;
    if((fp =opendir(dir_path)) == NULL)
        return -1;
    if(fp)
        closedir(fp);    
    return 0;
}

/*
*   @brief 判断文件是否存在
*   @param file_path 文件名称，包含路径
*
*   @return 文件路径为空或不存在则返回-1，存在返回0
*/
static int is_file_exist(const char *file_path)
{
    if(file_path == NULL)
        return -1;
    if(access(file_path,F_OK) == 0)
        return 0;
    return -1;
}

/*
* @brief 统计指定目录下所有文件的大小
* @param  <dirname>: 要统计的文件夹路径
* @return  计算的所有文件的大小,单位B
*/
long get_file_size(char *dirname)
{
    DIR *dir;
    struct dirent *ptr;
    long total_size = 0;
    char path[PATH_MAXSIZE] = {0};

    dir = opendir(dirname);
    if(dir == NULL)
    {
        DERROR("open dir: %s failed\n", dirname);
        return -1;
    }
    while((ptr=readdir(dir)) != NULL)
    {
        memset(path,0,PATH_MAXSIZE);
        snprintf(path, (size_t)PATH_MAXSIZE, "%s/%s", dirname,ptr->d_name);
        struct stat buf;
        if(lstat(path, &buf) < 0)
        {
            DERROR("lstat %s error.\n", path);
        }
        if(strcmp(ptr->d_name,".") == 0)
        {
            total_size += buf.st_size;
            continue;
        }
        if(strcmp(ptr->d_name,"..") == 0)
        {
            continue;
        }
        if(ptr->d_type == DT_DIR)
        {
            total_size += get_file_size(path);
            memset(path, 0, sizeof(path));
        }else
        {
            total_size += buf.st_size;
        }
    }
    closedir(dir);
    return total_size;
}

/*
*   @brief Register internal space listener,
*   @param callBack function
*   @return success return id number,failed return -1;
*/
int registerInsufficientSpaceEventListener(spaceListener obj)
{
    int i;
    for(i = 0;i < MAX_LISTENERLEN;i++)
    {
        if(mlistens[i]==0)
        {
            pthread_mutex_lock(&lock);
            if(mlistens[i]==0)
            {
                mlistens[i]= obj;
                pthread_mutex_unlock(&lock);
                return i;
            }
            pthread_mutex_unlock(&lock);
        }
    }
    return -1;

}

void unRegisterInsufficientSpaceEventListener(int id)
{
    pthread_mutex_lock(&lock);
	mlistens[id] = 0;
	pthread_mutex_unlock(&lock);
}

//Convert an array to a string
static void _array_to_string(const unsigned char *array,const int len,char *string)
{
    int i;
    for(i = 0; i < len; i++)
	{
		snprintf(string + i*2, 2+1, "%02x", array[i]);
	}

}