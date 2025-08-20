
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <errno.h>
#include "debug.h"
#include "fileOption.h"
#include "cJSON.h"

typedef enum
{
    NOT_EXIST=1,
    DIRECTORY,
    FILES
}FILE_ENUM;

static FILE_ENUM check_file(char *var, struct stat *st);    //检查文件类型

static  int cp_file(char *src_var, char *dest_var, struct stat *st);     //copy文件

static int cp_dir(char *src, char *dest);       //copy目录

//ls
void dostat(char *filename,char *out);
void show_file_info(char *filename,struct stat *info_p,char *out);
char * uid_to_name(uid_t uid);
char *gid_to_name(gid_t gid);


static int is_dir_exist(const char *dir_path)
{
    DIR *dirp;
    if(dir_path == NULL)
        return -1;
    dirp = opendir(dir_path);
    if(dirp == NULL)
    {
        return -1;
    }
	closedir(dirp);
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

FILE_ENUM check_file(char *var, struct stat *st)
{
    if( stat(var, st) )    //if stat function error(renturn nonzero)
    {
        if( ENOENT == errno)    //No such file or directory
        {
            return NOT_EXIST;
        }
        else
        {
            DERROR("stat: %s\n",strerror(errno));
            return(EXIT_FAILURE);
        }
    }

    else    // stat() ok, no error
    {
        //check file attr(dir or file)
        if( S_ISDIR(st->st_mode ))  
            return DIRECTORY;
        else if( S_ISREG(st->st_mode) )
            return FILES;
        else
        {
            DERROR("file type error\n");
            return(EXIT_FAILURE);
        }
    }
}


int cp_file(char *src_var, char *dest_var, struct stat *st)
{
    FILE *src = NULL;
    FILE *dest = NULL;

    if( S_ISREG(st->st_mode) )  //if dest is file
    {
        //1. open src and dest file
        if( NULL == (src = fopen(src_var, "r")) )
        {
            DERROR("fopen: %s\n",strerror(errno));
            return-1;
        }

        if( NULL == (dest = fopen(dest_var, "w+")) )
        {
            DERROR("fopen: %s\n",strerror(errno));
            return-1;
        }

        //2. copy the context from src to dest
        char buf[1024];
        int num;

        while(1)
        {
            // if at the end of file or an error occured
            if( 1024 != (num = fread(buf, 1,1024, src)))
            {
                if( !feof(src))
                {
                    DERROR("fread: %s\n",strerror(errno));
                    fclose(src);
                    fclose(dest); 
                    return -3;
                }

                else
                {
                    fwrite(buf, 1, num, dest);                
                    break;
                }
            }
            fwrite(buf, 1, 1024, dest);

        }

        //3. close src file
        fclose(src);
        fclose(dest);   
        return 0;
    }

    if( S_ISDIR(st->st_mode) )
    {
        FILE *dest = NULL;
        char buf[256]={0};
        char *name=NULL;

        //make the relative path to absolute path
        strncpy(buf, dest_var, sizeof(buf) - 1);
        buf[strlen(buf)]='/';
        name = strrchr(src_var,'/');
        strncat(buf, name+1,256-strlen(dest_var));
        //if dest file doesn't exist, creat it first
        if( NULL == (dest = fopen(buf, "w+")) )
        {
            DERROR("fopen:  %s  error:%s\n",buf,strerror(errno));
            return-1;
        }
        fclose(dest);
        struct stat new_dest_stat;
        if( stat(buf, &new_dest_stat))
        {
            DERROR("stat: %s\n",strerror(errno));
            return -4;
        }
        cp_file(src_var, buf, &new_dest_stat);
    }
    return 0;
}

//----------------------------------------------
//if src file is a dir
int cp_dir(char *src, char *dest)
{
    DIR *dirp = NULL;
    struct stat src_stat;
    struct dirent *entp = NULL;
    char src_buf[272] = {0};
    char dest_buf[272] = {0};

    //1. open the dir
    if( NULL == (dirp = opendir(src)) )
    {
        DERROR("opendir: %s\n",strerror(errno));
        return -1;
    }
 
    //2. read the dir
    while( NULL != (entp = readdir(dirp)))      //read the dir context
    {
        if( 0 == (strcmp(entp->d_name,"..")) || 0 == (strcmp(entp->d_name, ".")))
        {
            continue;
        }
        memset(src_buf,0,sizeof(src_buf));
        memset(dest_buf,0,sizeof(dest_buf));
        snprintf(src_buf, sizeof(src_buf),"%s/%s", src, entp->d_name);
        snprintf(dest_buf, sizeof(dest_buf),"%s/%s", dest, entp->d_name);

        if( stat(src_buf,&src_stat) )
        {
            DERROR("stat: %s\n",strerror(errno));
            return -2;
        }

        if( S_ISREG(src_stat.st_mode) )
        {
            cp_file(src_buf, dest_buf, &src_stat);
        }

        else if( S_ISDIR(src_stat.st_mode) )
        {
            if( -1 == mkdir(dest_buf, src_stat.st_mode) )
            {
                DERROR("mkdir: %s\n",strerror(errno));
                return -3;
            }

            cp_dir(src_buf, dest_buf);  //if subdir, recursive call itself
        }
    }

    return 0;
}

/*
*   @brief Copy files or directories
*   @param <sourcePath> source file/dir path
*          <destPath> dest file/file path
*   @return success return 0, otherwish return  negative
*/
int cmd_cp(char *sourcePath,char *destPath)
{
    //check the source file is a file or a directory
    struct stat src_stat;
    FILE *fp=NULL;
    //destination file check
    struct stat dest_stat;

    
    if(sourcePath == NULL || destPath == NULL)
    {
        return -1;
    }
    if(is_file_exist(sourcePath)!=0 && is_dir_exist(sourcePath)!=0)
    {
        //sourcePath not exist
        DERROR(" source file [%s] not exit\n",sourcePath);
        return -1;
    }

    //If source is a file
    if( FILES == check_file(sourcePath, &src_stat) )
    {
        FILE_ENUM dest_enum;
        dest_enum = check_file( destPath, &dest_stat );
       
        switch(dest_enum)
        {
            case NOT_EXIST:           
                fp = fopen(destPath,"w+");
                if(fp == NULL)
                {
                    DERROR("fopen error:%s\n",strerror(errno));
                    break;
                }
                fclose(fp);

                check_file( destPath, &dest_stat );
                cp_file(sourcePath, destPath, &dest_stat);

                break;

            case DIRECTORY:
//                printf("source:%s  destPath:%s\n",sourcePath,destPath);
                cp_file(sourcePath, destPath, &dest_stat);
                break;

            case FILES:
                cp_file(sourcePath,destPath, &dest_stat);
                break;
            default:
                DERROR("file type error\n");
        }
    }

    //If source file is a directory
    else if( DIRECTORY == check_file(sourcePath, &dest_stat) )
    {
        char destbuf[256]={0}; //存放新的目标路径
        char *name=NULL;       //要拷贝的目录名
        FILE_ENUM dest_enum;
        struct stat src_stat;
        dest_enum = check_file( destPath, &dest_stat );
        switch(dest_enum)
        {
            case NOT_EXIST:
                if( stat(sourcePath,&src_stat) )
                {
                    DERROR("stat: %s\n",strerror(errno));
                    return -2;
                }
                if( -1 == mkdir(destPath, src_stat.st_mode) )
                {
                    DERROR("mkdir: %s\n",strerror(errno));
                    return -3;
                }

                cp_dir(sourcePath,destPath); 
                break;

            case DIRECTORY:
                if(sourcePath[strlen(sourcePath)-1]=='/')
                    sourcePath[strlen(sourcePath)-1]='\0';
                name = strrchr(sourcePath,'/');
                strncpy(destbuf,destPath,sizeof(destbuf) - 1);
                if(destPath[strlen(destPath)-1]!='/')
                    destbuf[strlen(destPath)]='/';
                strncat(destbuf,name+1,255-strlen(destPath));

                if( stat(sourcePath,&src_stat) )
                {
                    DERROR("stat: %s\n",strerror(errno));
                    return -2;
                }
                if(0==is_dir_exist(destbuf))//目标已存在则删除，拷贝新文件
                {
                    cmd_rm(destbuf);
                }
                if( -1 == mkdir(destbuf, src_stat.st_mode) )
                {
                    DERROR("mkdir: %s\n",strerror(errno));
                    return -3;
                }

                cp_dir(sourcePath,destbuf); 
                break;

            case FILES:
                DERROR("copy a directory to a file\n");
                break;

            default:
                DERROR("file type error");
                break;
        }
    }

    return 0;
}
//get ls return length
int get_ls_return_length(char *dirname)
{
    DIR *p_dir;
	struct dirent *p_dirent;
	
	char dir_path[256]= {0};
	static struct stat my_stat;
    char temp[256]={0};
    int length=0;

	//防止溢出
	if(strlen(dirname)> 256 || dirname == NULL)
	{
		return -2;
	}
	//处理为绝对路径
	if(strcmp(dirname,".")==0)
	{
		getcwd(dir_path,256);
	}
	else
	{
		strncpy(dir_path,dirname,sizeof(dir_path) - 1);
	}
	
	if(-1==lstat(dir_path,&my_stat))
    {
        return -1;
    }
    else if(S_ISREG(my_stat.st_mode))
    {
        dostat(dir_path,temp);//需要绝对路径
        length = strlen(temp);
    }
    else if(S_ISDIR(my_stat.st_mode))
    {
		if(dir_path[strlen(dir_path)-1]!='/')
		{
			dir_path[strlen(dir_path)]='/';
		}
		if((p_dir=opendir(dir_path)) == NULL)
		{
			DERROR("opendir %s error\n",dirname);
			return -1;
		}
        char *path = malloc(sizeof(char)*256);
        if(path == NULL)
        {
            return -1;
        }
		while((p_dirent = readdir(p_dir)))
		{
			if(strcmp(p_dirent->d_name,".")==0 || strcmp(p_dirent->d_name,"..")==0)
				continue;
			memset(path,0,256);
			memcpy(path,dir_path,strlen(dir_path));
			if((strlen(p_dirent->d_name)+strlen(dir_path))<256)
			{   
                memset(temp,0,128);
				strncat(path,p_dirent->d_name,256-strlen(dir_path));
				dostat(path,temp);//需要绝对路径
                length+=strlen(temp);
			}
		}
		closedir(p_dir);
        free(path);
	}
	return length;
}
/*
*   @brief :Similar to the ls -l command
*   @param:<dirname> Absolute path to the directory 
*           <out> Receive return value，JSON structure
*   @return: Successfully returns 0, failure returns negative
*/
int cmd_ls(char *dirname,char *out)
{
	DIR *p_dir;
	struct dirent *p_dirent;
	
	char dir_path[256]= {0};
	static struct stat my_stat;
    char temp[128]={0};
    int length=0;

	//防止溢出
	if(strlen(dirname)> 256 || dirname == NULL)
	{
		return -2;
	}

	//处理为绝对路径
	if(strcmp(dirname,".")==0)
	{
		getcwd(dir_path,256);
	}
	else
	{
		strncpy(dir_path,dirname,sizeof(dir_path) - 1);
	}
	
	if(-1==lstat(dir_path,&my_stat))
    {
        return -1;
    }
    else if(S_ISREG(my_stat.st_mode))
    {
        dostat(dir_path,out);//需要绝对路径

    }
    else if(S_ISDIR(my_stat.st_mode))
    {
		if(dir_path[strlen(dir_path)-1]!='/')
		{
			dir_path[strlen(dir_path)]='/';
		}
		if((p_dir=opendir(dir_path)) == NULL)
		{
			DERROR("opendir %s error\n",dirname);
			return -1;
		}
        char *path = malloc(sizeof(char)*256);
        if(path == NULL)
        {
            closedir(p_dir);
            return -1;
        }
		while((p_dirent = readdir(p_dir)))
		{
			if(strcmp(p_dirent->d_name,".")==0 || strcmp(p_dirent->d_name,"..")==0)
				continue;
			memset(path,0,256);
			memcpy(path,dir_path,strlen(dir_path));
			if((strlen(p_dirent->d_name)+strlen(dir_path))<256)
			{   
                memset(temp,0,128);
				strncat(path,p_dirent->d_name,256-strlen(dir_path));
				dostat(path,temp);//需要绝对路径
                memcpy(out+length,temp,strlen(temp));
                length+=strlen(temp);
			}
		}
		closedir(p_dir);
        free(path);
	}
	return 0;
}

void dostat(char *filename,char *out)
{
	struct stat info;
	if(stat(filename,&info) == -1)
	{
		DERROR("filename:%s\n",strerror(errno));
	}
	else
	{
		show_file_info(filename,&info,out);
	}
}

void show_file_info(char *filename,struct stat *info_p,char *out)
{
	char *uid_to_name(), *ctime(), *gid_to_name(), *filemode();
	void mode_to_letters();
	char modestr[11]={0};
	char buf[128]={0};
	
	mode_to_letters(info_p->st_mode,modestr);	
	snprintf(buf,128,"%s %4d %-8s %-8s %8ld %s\n",modestr,(int)info_p->st_nlink,uid_to_name(info_p->st_uid),gid_to_name(info_p->st_gid),(long) info_p->st_size,filename);
    memcpy(out,buf,strlen(buf));
}

void mode_to_letters(int mode,char str[])
{
    char *ptr = "----------";
	strncpy(str,ptr,strlen(ptr));
	if(S_ISDIR(mode))
	{
		str[0] = 'd';
	}
	if(S_ISCHR(mode))
	{
		str[0] = 'c';
	}
	if(S_ISBLK(mode))
	{
		str[0] = 'b';
	}

	if((mode & S_IRUSR))
	{
		str[1]= 'r';
	}
	if((mode & S_IWUSR))
	{
		str[2]= 'w';
	}
	if((mode & S_IXUSR))
	{
		str[3]= 'x';
	}
	if((mode & S_IRGRP))
	{
		str[4]= 'r';
	}
	if((mode & S_IWGRP))
	{
		str[5]= 'w';
	}
	if((mode & S_IXGRP))
	{
		str[6]= 'x';
	}
	
	if((mode & S_IROTH))
	{
		str[7]= 'r';
	}
	if((mode & S_IWOTH))
	{
		str[8]= 'w';
	}
	if((mode & S_IXOTH))
	{
		str[9]= 'x';
	}
}

char * uid_to_name(uid_t uid)
{
	struct passwd *pw_ptr;
	static char errstr[10]={0};
	if((pw_ptr = getpwuid(uid))== NULL)
	{
		snprintf(errstr,10,"%d error",uid);
		return errstr;
	}
	else
		return pw_ptr->pw_name;
}

char *gid_to_name(gid_t gid)
{
	struct group *grp_ptr;
	static char errstr[10]={0};
	if((grp_ptr = getgrgid(gid))== NULL)
	{
		snprintf(errstr,10,"%d error",gid);
		return errstr;
	}
	else
		return grp_ptr->gr_name;
}


/*
*	@brief:Move the file/folder, change the file name, and overwrite if the newpath file already exists without warning
*   @param <oldpath> <newpath> absolute path
*	@return successfully returns 0,failure returns negative
*/
int cmd_mv(const char *oldpath,const char *newpath)
{
	int ret;
    struct stat new_stat;
    char destbuf[256]={0};
    char srcbuf[256]={0}; 
    char *name = NULL;
    memcpy(destbuf,newpath,strlen(newpath));
    memcpy(srcbuf,oldpath,strlen(oldpath));
    if(oldpath == NULL || newpath == NULL)
    {
        return -1;
    }

    if(access(destbuf,F_OK)!=0)//newpath 不存在，认为是将要修改的文件名
    {
        rename(srcbuf,destbuf); //移动文件并重命名
        return 0;
    }
    //如果存在则进行判断，如果newpath是文件，则直接覆盖，如果是目录，则在newpath目录下创建新的目录名
    if(stat(destbuf,&new_stat))
    {
        DERROR("stat: %s\n",strerror(errno));
        return -2;
    }

    if( S_ISREG(new_stat.st_mode))
    {
        rename(srcbuf,destbuf); //移动文件并重命名
        return 0;
    }
    if(srcbuf[strlen(srcbuf)-1]=='/')
        srcbuf[strlen(srcbuf)-1]='\0';
    if(destbuf[strlen(destbuf)-1]!='/')
        destbuf[strlen(destbuf)]='/';
    name = strrchr(srcbuf,'/');
    strncat(destbuf,name+1,255-strlen(newpath));
    if(0==is_dir_exist(destbuf))//目标已存在则删除，拷贝新文件
    {
        cmd_rm(destbuf);
    }
	ret = rename(srcbuf,destbuf);
	if(ret < 0)
	  DERROR("rename:%s\n",strerror(errno));
	return 0;
}

/*
*   @brief delete file or direction
*   @param <path> file or direction path
*   @return success return 0, otherwish return negative
*/
int cmd_rm(const char *path)
{
	char cur_dir[] = ".";
	char up_dir[] = "..";
	char dir_name[272]={0};
	DIR *dirp;
	struct dirent *dp;
	struct stat dir_stat;

    if(path == NULL)
    {
        return -1;
    }
	if(0 != access(path,F_OK))
	{
		DERROR("<%s> not exit\n",path);
		return 0;
	}

	//get file property failed ,return error
	if( stat(path,&dir_stat) < 0)
	{
		DERROR("get dir stat error:%s\n",strerror(errno));
		return -1;
	}

	if(S_ISREG(dir_stat.st_mode)) //Ordinary file
		remove(path);
	else if(S_ISDIR(dir_stat.st_mode))//Directory file, recursively delete directory contents
	{
		dirp = opendir(path);
		while((dp = readdir(dirp))!= NULL)
		{
			//ignore . and ..
			if((strcmp(cur_dir,dp->d_name) == 0) || (strcmp(up_dir,dp->d_name) == 0))
				continue;
			snprintf(dir_name,sizeof(dir_name),"%s/%s",path,dp->d_name);
			cmd_rm(dir_name);  //recursion call
		}
		closedir(dirp);
		rmdir(path);
	}
	else{
		DERROR("unknow file type! :%s",strerror(errno));
	}
	return 0;
}

/*
*   @brief:Modify user and group permissions for a file ;This function requires root privileges to execute
*   @param <userAndGroup> example <root:root> or <root> or <:root>
*           <filePath> file/dir path
*   @return: success return 0,otherwish return negative
*/
int cmd_chown(const char *userAndGroup,const char *filePath)
{
    //reset optind to avoid unexpected crash 
    optind = 1;
    if((userAndGroup== NULL) || (filePath == NULL))
    {
        return -1;
    }

    // Copy userAndGroup to 'user' so we can truncate it at the period
    // if a group id specified.
    char user[32];
    char *group = NULL;
    strncpy(user, userAndGroup, sizeof(user));
    if ((group = strchr(user, ':')) != NULL) {
        *group++ = '\0';
    } else if ((group = strchr(user, '.')) != NULL) {
        *group++ = '\0';
    }

    // Lookup uid (and gid if specified)
    struct passwd *pw;
    struct group *grp = NULL;
    uid_t uid=-1;   //passing -1 to chown preserves current user
    gid_t gid = -1; // passing -1 to chown preserves current group
    if(user[0]!='\0')
    {
        pw = getpwnam(user);
        if (pw != NULL) {
            uid = pw->pw_uid;
        } else {
            char* endptr;
            uid = (int) strtoul(user, &endptr, 0);
            if (endptr == user) {  // no conversion
            DERROR("No such user '%s'\n", user);
            return -2;
            }
        }
    }
    if (group != NULL) {
        grp = getgrnam(group);
        if (grp != NULL) {
            gid = grp->gr_gid;
        } else {
            char* endptr;
            gid = (int) strtoul(group, &endptr, 0);
            if (endptr == group) {  // no conversion
                DERROR("No such group '%s'\n", group);
                return -3;
            }
        }
    }

    if (chown(filePath, uid, gid) < 0) {
        DERROR("Unable to chown %s: %s\n", filePath, strerror(errno));
        return -4;
    }
    return 0;
}

/*
*   @brief Modify file permission interface
*   @param <mode> permission example "777","742"...
*          <filePath> Modify the path of the file
*   @return success return 0,otherwish return negative
*/
int cmd_chmod(int mode, const char *filePath) 
{

    int mode_u;   //所有者user的权限
    int mode_g;   //所属组group的权限
    int mode_o;   //其他用户other的权限

    if(filePath == NULL)
    {
        return -1;
    }
    if(mode>777||mode<0)
    {
        DPRINTF("number mode error!,Mode range is 0-777\n");
        return -1;
    }
    mode_u=mode/100;
    mode_g=mode/10%10;
    mode_o=mode%10;
    mode=(mode_u*8*8)+(mode_g*8)+mode_o;   //八进制转换

    //更改失败返回-1
    if(chmod(filePath,mode)==-1)
    {
        DERROR("chmod error :%s\n",strerror(errno));
        return -1;
    } 
    return 0;
}