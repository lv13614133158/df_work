
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "debug.h"
#include "cJSON.h"


#define MALLOC_S size_t
#define MALLOC_P char
#define PROCFS "/proc"
#define MAXPATHLEN 1024

static int make_proc_path(char *pp, int pl, char **np, int *nl, char *sf);
static int nm2id(char *nm,int *id,int *idl);
static int checkProcessOpenFile(char *pidpath,int n,char *fileName);
static int getlinksrc(char *ln, char *src, int srcl);
static int read_id_stat(char *p,int id,char **cmd);
static MALLOC_S alloc_cbf(MALLOC_S len, char **cbf, MALLOC_S cbfa);


char  *gather_proc_info(char *fileName)
{
    char *pidpath = (char *)NULL;
	char *path = (char *)NULL;
    int pathl = 0;
    MALLOC_S pidpathl = 0;
	MALLOC_S pidx = 0;
    DIR *ps = (DIR *)NULL;
    struct dirent *dp;
    int pid,n,ret;
	char *cmd;
	char *s = NULL;
	cJSON *root,*obj;
	root = cJSON_CreateArray();
	if(root == NULL)
		return NULL;
	
/*
 * Do one-time setup.
 */
	if (!pidpath) {
	    pidx = strlen(PROCFS) + 1;
	    pidpathl = pidx + 64 + 1;	/* 64 is growth room */
	    if (!(pidpath = (char *)malloc(pidpathl))) {
			DERROR(" can't allocate %d bytes for %s\n", (int)pidpathl, PROCFS);
			goto exit;
	    }
	    (void) snprintf(pidpath, pidpathl, "%s/", PROCFS);
	}
/*
 * Read /proc, looking for PID directories.  Open each one and
 * gather its process and file information.
 */
   if(!ps)
   {
       if(!(ps = opendir(PROCFS)))//open failed
       {
        //    DERROR("can't open %s\n",PROCFS);
           goto exit;
       }
   }else
   {
       (void)rewinddir(ps);
   }
   while((dp = readdir(ps)))
   {
        if(nm2id(dp->d_name,&pid,&n))
            continue;     
        /*
        * Build path to PID's directory.
        */
        if((pidx + n + 1 + 1) > pidpathl)
        {
            pidpathl = pidx + n + 1 + 1 + 64;
            if(!(pidpath = (char *)realloc((MALLOC_P *)pidpath,pidpathl)))
            {
                DERROR("can't allocate %d bytes for %s/%s\n",(int)pidpathl,PROCFS,dp->d_name);
				closedir(ps);
				goto exit;
                //return;
            }
        }
        (void) snprintf(pidpath + pidx,pidpathl - pidx,"%s/",dp->d_name);
        n +=(pidx +1);  //记录/proc/<pid>长度
		//查找进程打开的文件，找到则返回1，没找到返回0
		ret = checkProcessOpenFile(pidpath,n,fileName);

		//如果上个api返回1，则查找该进程的进程名，读取/proc/<pid>/stat文件
		if(ret != 1)
			continue;
		(void) make_proc_path(pidpath,n,&path,&pathl,"stat"); //proc/<pid>/stat
		ret = read_id_stat(path,pid,&cmd);
		if(ret < 0 )
			continue;
		DPRINTF("%s  %d  %s\n",cmd,pid,fileName);
		cJSON_AddItemToArray(root,obj = cJSON_CreateObject());
		cJSON_AddNumberToObject(obj,"pid",pid);
		cJSON_AddStringToObject(obj,"process_name",cmd);

    }
	s = cJSON_PrintUnformatted(root);
	if(root)
		cJSON_Delete(root);
	closedir(ps);
exit:
	if(pidpath!= NULL)
	{
		free(pidpath);
		pidpath = NULL;
	}

	if(path != NULL)
	{
		free(path);
		path = NULL;
	}
	if(s != NULL)
	{
		return s;
	}
	return NULL;

}

/*
 * make_proc_path() - make a path in a /proc directory
 *
 * entry:
 *	pp = pointer to /proc prefix
 *	lp = length of prefix
 *	np = pointer to malloc'd buffer to receive new file's path
 *	nl = length of new file path buffer
 *	sf = new path's suffix
 *
 * return: length of new path
 *	np = updated with new path
 *	nl = updated with new path length
 */

int make_proc_path(char *pp, int pl, char **np, int *nl, char *sf)
{
	char *cp;
	MALLOC_S rl, sl;

	sl = strlen(sf);
	if ((rl = pl + sl + 1) > *nl) {
	    if ((cp = *np))
		cp = (char *)realloc((MALLOC_P *)cp, rl);
	    else
		cp = (char *)malloc(rl);
	    if (!cp) {
			DERROR("can't allocate %d bytes\n",(int)rl);
			return -1;
	    }
	    *nl = rl;
	    *np = cp;
	}
	(void) snprintf(*np, *nl, "%s", pp);
	(void) snprintf(*np + pl, *nl - pl, "%s", sf);
	return(rl - 1);
}

/*
*   nm2id() - convert a name to an integer ID
*   arg:
*       nm:pointer to name
*       id:pointer to ID receiver
*       idl:pointer to ID length receiver 
*/

static int nm2id(char *nm,int *id,int *idl)
{
    register int tid, tidl;
    for(*id = *idl = tid = tidl = 0; *nm; nm++)
    {
	    if (!isdigit((unsigned char)*nm))
        {
            return (1);
        }
        tid = tid * 10 + (int )(*nm - '0');
        tidl++;
    }
    *id = tid;
    *idl = tidl;
    return 0;
}


/*
*	检查进程打开的文件并比对
*	pidpath:pointer to ID's path
*	pointer to ID's path length
*	fileName: compare fileName
* 
*   return :  1 == fileName exist
*   return :  0 == fileName not exist
*/
static int checkProcessOpenFile(char *pidpath,int n,char *fileName)
{
	char *dpath = (char *)NULL;
	int dpathl = 0;
	char *path = (char *)NULL;
	int pathl = 0;
	DIR *fdp = NULL;
	struct dirent *fp;
	char pbuf[MAXPATHLEN + 1];
	int i,fd,length,flag = 0;

	if((i = make_proc_path(pidpath,n,&dpath,&dpathl,"fd/"))<3)
	{
		goto exit1;
	}
	dpath[i-1]='\0';
// printf("pidpath:%s  dpath:%s\n",pidpath,dpath);
	if(!(fdp = opendir(dpath)))
	{
		// DERROR("opendir error <%s> \n",dpath);
		goto exit1;
	}
	dpath[i-1]='/';
	while((fp = readdir(fdp)))
	{
		if(nm2id(fp->d_name,&fd,&n))
			continue;
		(void)make_proc_path(dpath,i,&path,&pathl,fp->d_name);
		memset(pbuf,0,sizeof(pbuf));
		if((length = getlinksrc(path,pbuf,sizeof(pbuf)))< 1)
		{
			continue;
		}

		if(strncmp(pbuf,fileName,length)== 0)//文件名不相同
		{
			flag = 1;
			break;
		}
	}
		closedir(fdp);
exit1:
		if(path !=NULL)
		{
			free(path);
			path = NULL;
		}

		if(dpath!=NULL)
		{
			free(dpath);
			dpath = NULL;
		}
		return flag;
}

/*
 * getlinksrc() - get the source path name for the /proc/<PID>/fd/<FD> link
 * 	char *ln;			link path 
	char *src;			link source path return address 
	int srcl;			length of src[] 
 */

static int getlinksrc(char *ln, char *src, int srcl)
{
	int ll;
	if ((ll = readlink(ln, src, srcl - 1)) < 1 ||  ll >= srcl)
	    return(-1);
	src[ll] = '\0';
	if (*src == '/')
	    return(ll);
	else 
		return -1;
/*
	//socket file
	if ((cp = strchr(src, ':'))) {
	    *cp = '\0';
	    ll = strlen(src);
	    if (rest)
		*rest = cp + 1;
	}
	return(ll);
*/
}

/*
*	get process command
*	p:path to status file
*	id:PID
*	cmd:malloc'd command name
*	return:
*		    -1 == ID is unavailable
 *          0 == ID OK
*/

static int read_id_stat(char *p,int id,char **cmd)
{
	FILE *fs;
	char *cp,*cp1;
	char buf[MAXPATHLEN];
	MALLOC_S cbfa = 0;
	char *cbf = (char *)NULL;
	int pc,ch,cx,es;
/*
 * Open the stat file path, and read the file's first line.
 */
	if(!(fs = fopen(p,"r")))
	{
		return -1;
	}
	if(!(cp = fgets(buf,sizeof(buf),fs)))
	{
read_id_stat_exit:
		(void) fclose(fs);
		return -1;
	}

/*
 * Skip to the first field, and make sure it is a matching ID.
 */
	cp1 = cp;
	while(*cp && (*cp != ' ') && (*cp != '\t'))
		cp++;
	if(*cp)
		*cp = '\0';
	if(atoi(cp1) != id)
		goto read_id_stat_exit;

/*
 * The second field should contain the command, enclosed in parentheses.
 * If it also has embedded '\n' characters, replace them with '?' characters,
 * accumulating command characters until a closing parentheses appears.
 *
 */
	for (++cp; *cp && (*cp == ' '); cp++)
		;
	if (!cp || (*cp != '('))
	    goto read_id_stat_exit;
	cp++;
	pc = 1;			/* start the parenthesis balance count at 1 */

	/*
 * Enter the command characters safely.  Supply them from the initial read
 * of the stat file line, a '\n' if the initial read didn't yield a ')'
 * command closure, or by reading the rest of the command a character at
 * a time from the stat file.  Count embedded '(' characters and balance
 * them with embedded ')' characters.  The opening '(' starts the balance
 * count at one.
 */
	for (cx = es = 0;;) {
	    if (!es)
		ch = *cp++;
	    else {
		if ((ch = fgetc(fs)) == EOF)
		    goto read_id_stat_exit;
	    }
	    if (ch == '(')		/* a '(' advances the balance count */
		pc++;
	    if (ch == ')') {
	    
	    /*
	     * Balance parentheses when a closure is encountered.  When
	     * they are balanced, this is the end of the command.
	     */
		pc--;
		if (!pc)
		    break;
	    }
	    if ((cx + 2) > cbfa)
		cbfa = alloc_cbf((cx + 2), &cbf, cbfa);
	    cbf[cx] = ch;
	    cx++;
	    cbf[cx] = '\0';
	    if (!es && !*cp)
			es = 1;		/* Switch to fgetc() when a '\0' appears. */
	}
	*cmd = cbf;
	fclose(fs);
	return 0;
}

/*
 * alloc_cbf() -- allocate a command buffer
 * 
 * 	MALLOC_S len;				required length 
	char **cbf;				    current buffer 
	MALLOC_S cbfa;				current buffer allocation 
 */

static MALLOC_S alloc_cbf(MALLOC_S len, char **cbf, MALLOC_S cbfa)
{
	if (*cbf)
	    *cbf = (char *)realloc((MALLOC_P *)*cbf, len);
	else
	    *cbf = (char *)malloc(len);
	if (!*cbf) {
		DERROR("can't allocate cocmmand %d bytes\n",(int)len);
		return 1;
	}
	return(len);
}


//Free up the space allocated to get the process information
void free_proc_info_memory(char *p)
{
	if(p!=NULL)
	{
		free(p);
	}
}