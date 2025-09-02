#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <paths.h>
#include <pwd.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>
#include  <semaphore.h>
#include "pid_detection.h"
#include "api_networkmonitor.h"

#define  PORT_SCAN_TIME  (4)

static list list_tcpconnectinfo;
static list list_udpconnectinfo;
static list list_rawconnectinfo;
#define  TCP_INIT_VALUE()\
	list_destroy(&list_tcpconnectinfo);\
	list_init(&list_tcpconnectinfo,free);
#define  UDP_INIT_VALUE()\
	list_destroy(&list_udpconnectinfo);\
	list_init(&list_udpconnectinfo,free);
#define  RAW_INIT_VALUE()\
	list_destroy(&list_rawconnectinfo);\
	list_init(&list_rawconnectinfo,free);
static list list_portopenedinfo;
static pthread_mutex_t netport_lock = PTHREAD_MUTEX_INITIALIZER;
static int netport_list_init = 0;

typedef struct _netPortNode{
	unsigned int port;
	char status;
	int whitelist;
}netPortNode, pNetPortNode;

static bool search_netport_list(unsigned int element)
{
	pthread_mutex_lock(&netport_lock);
	list_elmt *head = list_portopenedinfo.head;
	while(head)
	{
		netPortNode* p =head->data;
		if(p->port == element){
			p->status = 1;
			pthread_mutex_unlock(&netport_lock);
			return false;
		}
		head = head->next;
	}
	pthread_mutex_unlock(&netport_lock);
	return true;
}

void append_netport_list(unsigned int portint, int whitelist)
{
	pthread_mutex_lock(&netport_lock);
	netPortNode* p = (netPortNode *)malloc(sizeof(netPortNode));
	p->port = portint;
	p->status = 1;
	p->whitelist = whitelist;
	list_ins_next(&list_portopenedinfo,NULL,p);
	pthread_mutex_unlock(&netport_lock);
}	

static void change_netport_list()
{
	pthread_mutex_lock(&netport_lock);
	list_elmt *head = list_portopenedinfo.head, *last=NULL;
	while(head)
	{
		netPortNode* p = head->data;

		if (!(p->whitelist))
		{
			p->status--;
			if(p->status<0) //释放已经关闭的端口
			{
				if(head == list_portopenedinfo.head)
				{
					list_portopenedinfo.head = head->next;
					last = head->next;
					free(head->data);
					free(head);
					list_portopenedinfo.size--;
					head = last;
					continue;
				}
				last->next = head->next;
				free(head->data);
				free(head);
				list_portopenedinfo.size--;
				head = last->next;
			}
			else
			{
				last = head;
				head = head->next;
			}
		}
		else
		{
			last = head;
			head = head->next;
		}
	}
	pthread_mutex_unlock(&netport_lock);
}

/******test define QDebug_Deep have more info**********/
//#define  QDebug
//#define  QDebug_Deep
/* pathnames of the procfs files used by NET.   we use tcp tcp6 udp udp6 raw raw6 */
#define _PATH_PROCNET_TCP		"/proc/net/tcp"
#define _PATH_PROCNET_TCP6		"/proc/net/tcp6"
#define _PATH_PROCNET_UDP		"/proc/net/udp"
#define _PATH_PROCNET_UDP6		"/proc/net/udp6"
#define _PATH_PROCNET_RAW		"/proc/net/raw"
#define _PATH_PROCNET_RAW6		"/proc/net/raw6"
#define PROGNAME_WIDTH          40 	
#define PRG_HASH_SIZE           211
#define RAW 					0x02
#define TCP 					0x01
#define UDP 					0x00
#ifndef LINE_MAX
#define LINE_MAX 				4096
#endif
#if !defined(s6_addr32) && defined(in6a_words)
#define s6_addr32 in6a_words	/* libinet6			*/
#endif
/******dir info**********/
FILE *procinfo;
FILE *procinfotcp;
FILE *procinfoudp;
FILE *procinforaw;

#define INFO_GUTS1(file,name,proc,src_ip,des_ip,src_port,des_port,type)\
	list_elmt *cur_elmt = list_head(&file);\
	while(cur_elmt != NULL){\
		char* e = cur_elmt->data;\
		if((retbuffer = (proc)(lnr++, e,src_ip,des_ip,src_port,des_port,type)) != NULL)\
			return retbuffer;	\
		cur_elmt = cur_elmt->next;\
	}

#if HAVE_AFINET6
#define INFO_GUTS2(file,proc,src_ip,des_ip,type)				\
	  lnr = 0;						\
	  procinfo = fopen((file), "r");				\
	  if (procinfo != NULL) {				\
		do {						\
		  if (fgets(buffer, sizeof(buffer), procinfo))	\
			if((retbuffer = (proc)(lnr++, buffer,src_ip,des_ip,type)!= NULL)	{			\
				fclose(procinfo);return retbuffer;}\
		} while (!feof(procinfo));				\
		fclose(procinfo);					\
	  }
#else
#define INFO_GUTS2(file,proc,src_ip,des_ip,type)
#endif
	
#define INFO_GUTS3					\
	 return NULL;
	
#define INFO_GUTS6(file,file6,name,proc,src_ip,des_ip,src_port,des_port,type)		\
	 char buffer[8192]; 				\
	 char*retbuffer = NULL;				\
	 int rc = 0;						\
	 int lnr = 0;						\
	 INFO_GUTS1(file,name,proc,src_ip,des_ip,src_port,des_port,type)				\
	 INFO_GUTS2(file6,proc,src_ip,des_ip,type)				\
	 INFO_GUTS3
#define INFO_GUTS(file,name,proc)			\
	 char buffer[8192]; 				\
	 int rc = 0;						\
	 int lnr = 0;						\
	 INFO_GUTS1(file,name,proc) 			\
	 INFO_GUTS3
static struct prg_node {
	struct prg_node *next;
	int inode;
	char name[PROGNAME_WIDTH];
} *prg_hash[PRG_HASH_SIZE];
	

/*
* decla: find inode related
*/		
#define PRG_HASHIT(x) ((x) % PRG_HASH_SIZE)	
#define PRG_SOCKET_PFX    "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
#define PRG_SOCKET_PFX2   "[0000]:"
#define PRG_SOCKET_PFX2l  (strlen(PRG_SOCKET_PFX2))
 	
/*
* decla: for inode inset list
*/	
#define PATH_PROC	   		"/proc"
#define PATH_FD_SUFF		"fd"
#define PATH_FD_SUFFl       strlen(PATH_FD_SUFF)
#define PATH_PROC_X_FD      PATH_PROC "/%s/" PATH_FD_SUFF
#define PATH_CMDLINE		"cmdline"
#define PATH_CMDLINEl       strlen(PATH_CMDLINE)
/**
* inode:VFS node name:process name(format:PID/processname)
*/
static void prg_cache_add(int inode, char *name)
{
	unsigned hi = PRG_HASHIT(inode);
	struct prg_node **pnp,*pn;
	 
	for (pnp=prg_hash+hi;(pn=*pnp);pnp=&pn->next) {
	if (pn->inode==inode) {
		/* Some warning should be appropriate here
		   as we got multiple processes for one i-node */
#ifdef QDebug_Deep
		printf("insert inode fail %d %s\n",inode,name);
#endif
		return;
	}
	}
	if (!(*pnp=malloc(sizeof(**pnp)))) 
	return;
	pn=*pnp;
	pn->next=NULL;
	pn->inode=inode;
	if (strlen(name)>sizeof(pn->name)-1) 
	name[sizeof(pn->name)-1]='\0';
	strcpy(pn->name,name);
#ifdef QDebug_Deep
	printf("insert inode success %d %s\n",inode,name);
#endif
}
/*
*function: find inode ->process name
*/
static char *prg_cache_get(int inode)
{
	unsigned hi=PRG_HASHIT(inode);
	struct prg_node *pn;

	for (pn=prg_hash[hi];pn;pn=pn->next)
	if (pn->inode==inode) return(pn->name);
	return NULL;
}
/**
*function:clear list
*/
static void prg_cache_clear(void)
{
	struct prg_node **pnp,*pn;

	for (pnp=prg_hash;pnp<prg_hash+PRG_HASH_SIZE;pnp++)
		while ((pn=*pnp)) {
		*pnp=pn->next;
		free(pn);
		}
}
/**
*function:get inode type 1
*/
static void extract_type_1_socket_inode(const char lname[], long * inode_p) {

	/* If lname is of the form "socket:[12345]", extract the "12345"
	   as *inode_p.  Otherwise, return -1 as *inode_p.
	   */

	if (strlen(lname) < PRG_SOCKET_PFXl+3) *inode_p = -1;
	else if (memcmp(lname, PRG_SOCKET_PFX, PRG_SOCKET_PFXl)) *inode_p = -1;
	else if (lname[strlen(lname)-1] != ']') *inode_p = -1;
	else {
		char inode_str[strlen(lname + 1)];	/* e.g. "12345" */
		const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
		char *serr;

		strncpy(inode_str, lname+PRG_SOCKET_PFXl, inode_str_len);
		inode_str[inode_str_len] = '\0';
		*inode_p = strtol(inode_str,&serr,0);
		if (!serr || *serr || *inode_p < 0 || *inode_p >= INT_MAX) 
			*inode_p = -1;
	}
}


/**
*function:get inode  type2
*/
static void extract_type_2_socket_inode(const char lname[], long * inode_p) {

	/* If lname is of the form "[0000]:12345", extract the "12345"
	   as *inode_p.  Otherwise, return -1 as *inode_p.
	   */

	if (strlen(lname) < PRG_SOCKET_PFX2l+1) *inode_p = -1;
	else if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l)) *inode_p = -1;
	else {
		char *serr;

		*inode_p=strtol(lname + PRG_SOCKET_PFX2l,&serr,0);
		if (!serr || *serr || *inode_p < 0 || *inode_p >= INT_MAX) 
			*inode_p = -1;
	}
}
/**
*function:read /proc/7878(PID)/fd/inode(link VFS node)
*         read /proc/7878(PID)/cmdline(process name)
*         insert list (inode ,pid/processname)
*/
static void prg_cache_load(void)
{
	char line[LINE_MAX],eacces=0;
	int procfdlen,fd,cmdllen,lnamelen;
	char lname[30],cmdlbuf[512],finbuf[PROGNAME_WIDTH];
	long inode = 0;
	const char *cs = NULL,*cmdlp = NULL;
	DIR *dirproc=NULL,*dirfd=NULL;
	struct dirent *direproc,*direfd;

	cmdlbuf[sizeof(cmdlbuf)-1]='\0';
	if (!(dirproc=opendir(PATH_PROC))) goto fail;
	while (errno=0,direproc=readdir(dirproc)) {
		for (cs=direproc->d_name;*cs;cs++)
			if (!isdigit(*cs))  
			break;
		if (*cs) //make sure if dir have name 
			continue;
		procfdlen=snprintf(line,sizeof(line),PATH_PROC_X_FD,direproc->d_name);// proc/hello/fd direproc->d_name is pid
		if (procfdlen<=0 || procfdlen>=sizeof(line)-5) 
			continue;
		errno=0;
		dirfd=opendir(line);
		if (! dirfd) {
			if (errno==EACCES) 
			eacces=1;
			continue;
		}
		line[procfdlen] = '/';
		cmdlp = NULL;
		while ((direfd = readdir(dirfd))) {//read subfile
			if((strcmp(".",direfd->d_name) == 0)||(strcmp("..",direfd->d_name) == 0)||(strcmp("...",direfd->d_name) == 0))
				continue;		
			if (procfdlen+1+strlen(direfd->d_name)+1>sizeof(line)) 
				continue;
			memcpy(line + procfdlen - PATH_FD_SUFFl, PATH_FD_SUFF "/",
			PATH_FD_SUFFl+1);
			strcpy(line + procfdlen + 1, direfd->d_name);
			lnamelen=readlink(line,lname,sizeof(lname)-1);
			lname[lnamelen] = '\0';  /*make it a null-terminated string*/
			extract_type_1_socket_inode(lname, &inode);//find inode  if return !-1  save fd
			if (inode < 0) extract_type_2_socket_inode(lname, &inode);//
			if (inode < 0) continue;
			if (!cmdlp) {
			if (procfdlen - PATH_FD_SUFFl + PATH_CMDLINEl >= 
				sizeof(line) - 5) 
				continue;
			strcpy(line + procfdlen-PATH_FD_SUFFl, PATH_CMDLINE);
			fd = open(line, O_RDONLY);//open cmdline
			if (fd < 0) 
				continue;
			cmdllen = read(fd, cmdlbuf, sizeof(cmdlbuf) - 1);
			if (close(fd)) 
				continue;
			if (cmdllen == -1) 
				continue;
			if (cmdllen < sizeof(cmdlbuf) - 1) 
				cmdlbuf[cmdllen]='\0';
			if ((cmdlp = strrchr(cmdlbuf, '/'))) 
				cmdlp++;
			else 
				cmdlp = cmdlbuf;//get process name
			}
			snprintf(finbuf, sizeof(finbuf), "%s/%s", direproc->d_name, cmdlp);
			prg_cache_add(inode, finbuf);
		}
		closedir(dirfd); 
		dirfd = NULL;
	}
	if (dirproc) 
	closedir(dirproc);
	if (dirfd) 
	closedir(dirfd);
	if (!eacces) 
	return;
	fail:
		printf("No info could be read for    but you should be root. \n");
}
/**
*decla:UDP a port can only be used once
*      Only one process can be detected in UDP communication of this machine
*/
static bool local_ipmatch(int src_ip_t,int src_port_t,int dst_ip_t,int des_port_t,int localport)//(src_ip,src_port,des_ip,des_port,local_port)
{
	char ip[128],buffer[32];
    int fd, intrface, retn = 0;
    struct ifreq buf[INET_ADDRSTRLEN];
    struct ifconf ifc;
	if((src_port_t != localport)&&(des_port_t != localport))
		return false;
	char src_ip[64] = {0},dst_ip[64] = {0};
	char *tmp = inet_ntoa((struct in_addr){.s_addr=src_ip_t});
	memcpy(src_ip,tmp,strnlen(tmp,sizeof(src_ip)));
	snprintf(buffer, sizeof(buffer), "%d",src_port_t);
	strcat(src_ip, ":");
	strcat(src_ip, buffer);
	 
	tmp = inet_ntoa((struct in_addr){.s_addr=dst_ip_t});
	memcpy(dst_ip,tmp,strnlen(tmp,sizeof(dst_ip)));
	snprintf(buffer, sizeof(buffer), "%d",des_port_t);
	strcat(dst_ip, ":");
	strcat(dst_ip, buffer);
	 
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0){
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)){
	        intrface = ifc.ifc_len/sizeof(struct ifreq);
	        while (intrface-- > 0){
	            if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface]))){
					strcpy(ip,(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr)));
					snprintf(buffer, sizeof(buffer), "%d",localport);
					strcat(ip, ":");
					strcat(ip, buffer);
					if((strcmp(ip,dst_ip) == 0)||(strcmp(ip,src_ip) == 0)){
						return true;
					}

	            }
	        }
        }
		close(fd);
		return false;
    }
	return false;
}


/*
*function:process tcp line data
*src_ip  :XXX.XXX.XXX.XXX:port
*/
/*
*function:process tcp line data
*src_ip  :XXX.XXX.XXX.XXX:port
*/
static char* tcpudpraw_do_one(int lnr, const char *line,const int src_ip,const int des_ip,int src_port,int des_port,char type)
{
	unsigned long rxq, txq, time_len, retr, inode;
	char*         Temp = NULL;
	struct in_addr   temp_aton_st,temp1_aton_st;
	int num, local_port, rem_port, d, state, uid, timer_run, timeout;
	char rem_addr[128],rem_addrtoh[128],local_addr[128], local_addrtoh[128],timers[64], buffer[1024], more[512];
#if HAVE_AFINET6
#else
	struct sockaddr_in localaddr, remaddr;
#endif
	 
	if (lnr == 0)
		return NULL;	 
    char localiptemp[9] = {'\n'},remiptemp[9] = {'\n'};
    char localport[5] = {'\n'},remoteport[5] = {'\n'};
	for(int loop = 0;loop < 128;loop ++){
		if(line[loop] == ':'){
			memcpy(localiptemp,&line[ loop + 2],8);
			memcpy(localport,&line[ loop + 11],4);
			memcpy(remiptemp,&line[ loop + 16],8);
			memcpy(remoteport,&line[ loop + 25],4);
			break;
		}
	}	
 
	unsigned int localipint = strtol(localiptemp,NULL,16);
	unsigned int localportint = strtol(localport,NULL,16);
	unsigned int remoteipint = strtol(remiptemp,NULL,16);
	unsigned int remoteportint = strtol(remoteport,NULL,16);

	if (strlen(local_addr) > 8) {
#if HAVE_AFINET6
#endif
	} else { 
	}
#ifdef QDebug
	printf("src_ip %s    local_addrtoh %s  des_ip%s  rem_addrtoh %s inode %d\n",
		   src_ip,local_addrtoh,des_ip,rem_addrtoh,inode);
#endif
	if(TCP == type){
		if((((src_ip == localipint)&&(src_port == localportint))&&((des_ip == remoteipint)&&(des_port == remoteportint)  ))\
			||(((src_ip == remoteipint)&&(src_port == remoteportint) )&&((des_ip == localipint)&&(des_port == localportint)))){
					num = sscanf(line,
			"%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
				&d, local_addr, &local_port, rem_addr, &rem_port, &state,
				&txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode, more);
			 prg_cache_clear();
		     prg_cache_load();
			  return prg_cache_get(inode);
			}
		else
			return NULL;
	}else{//udp
		if(((src_ip == localipint)&&(src_port == localportint))||((des_ip == localipint)&&(des_port == localportint))){
			num = sscanf(line,
		"%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
			&d, local_addr, &local_port, rem_addr, &rem_port, &state,
			&txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode, more);
			prg_cache_clear();
		    prg_cache_load();
			return prg_cache_get(inode);
		}
		else{
			if(local_ipmatch(src_ip,src_port,des_ip,des_port,localportint)){
			//if(local_ipmatch(src_ip,des_ip,local_port)) {
				num = sscanf(line,
			"%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %512s\n",
				&d, local_addr, &local_port, rem_addr, &rem_port, &state,
				&txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode, more);
				printf("inode is %d\n",inode);
				prg_cache_clear();
		        prg_cache_load();
				return prg_cache_get(inode);
			}
			else
				return NULL;
		}
			 
	}
}
pthread_mutex_t    request_pid_lock = PTHREAD_MUTEX_INITIALIZER;
/*
*function:get pid/processname
*input:       src_ip(XXX.XXX.XXX.XXX:port)
*/
static char* tcp_info(int src_ip,int des_ip,int srcport,int desport,char type)
{
	INFO_GUTS6(list_tcpconnectinfo, _PATH_PROCNET_TCP6, "AF INET (tcp)",
		   tcpudpraw_do_one,src_ip,des_ip,srcport,desport,type);//tcp 0
}
/*
*function:get pid/processname
*input:       src_ip(XXX.XXX.XXX.XXX:port)
*/
static char* udp_info(int src_ip,int des_ip,int srcport,int desport,char type)
{
	INFO_GUTS6(list_udpconnectinfo, _PATH_PROCNET_UDP6, "AF INET (udp)",
		   tcpudpraw_do_one,src_ip,des_ip,srcport,desport,type);//udp 1
}
/*
*function:get pid/processname
*input:       src_ip(XXX.XXX.XXX.XXX:port)
*/
static char* raw_info(int src_ip,int des_ip,int srcport,int desport,char type)
{
	INFO_GUTS6(list_rawconnectinfo, _PATH_PROCNET_RAW6, "AF INET (raw)",
		   tcpudpraw_do_one,src_ip,des_ip,srcport,desport,type);//raw 2
}

/*snprintf约占6%CPU**/
/*#define tcpudpcallback(type)													\
	char stringsrc[40] = {0},stringdst[40] = {0};											\
	char*name = NULL, *Temp = NULL,*filepid = NULL,*filename = NULL,templen = 0;\
	if(maxlength < PROGNAME_WIDTH)												\
		return;																	\
	pthread_mutex_lock(&request_pid_lock);										\
	if((name = ((type == TCP)?(tcp_info(src_ip,des_ip,src_port,des_port,TCP)):(udp_info(src_ip,des_ip,src_port,des_port,UDP)))) == NULL)\
		name = raw_info(src_ip,des_ip,src_port,des_port,type);		\
	pthread_mutex_unlock(&request_pid_lock);			\					
	if(name != NULL){								\
		if((filepid = strtok(name,"/")) == NULL){	\
			return;								\
		}											\
		if((Temp = strtok(NULL,"/")) == NULL){		\
			return;								\
		}else{										\
			filename = strtok(Temp,":");			\
		}											\
		memcpy(processname,filename,strlen(filename)+1);\
		memcpy(str_pid,filepid,strlen(filepid)+1);  \
	}
*/
/*
*function:
*input:   src_ip:待检测数据包的源Ip地址（xxx.xxx.xxx.xxx）         src_port:待检测数据包的源port(如 50000)
		  des_ip:待检测数据包的目的Ip地址（xxx.xxx.xxx.xxx）         des_port:待检测数据包的目的port(如 50000)
		  str_pid:该数据包属于的进程PID processname:该数据包属于的进程名
		  maxlength:src_pid与processname缓冲空间的最小空间（str_pid[100] processname[10] 则应该填入10 
		  空间至少要大于PROGNAME_WIDTH ）
		  如果文件名大于40字节则会出现截断
*/
void tcppid_callback\
(char *src_ip,int src_port,char* des_ip,int des_port,char* str_pid,char *processname,char maxlength)
{
	//tcpudpcallback(TCP);

	char stringsrc[40] = {0},stringdst[40] = {0};											\
	char*name = NULL, *Temp = NULL,*filepid = NULL,*filename = NULL,templen = 0;\
	if(maxlength < PROGNAME_WIDTH)												\
		return;																	\
	pthread_mutex_lock(&request_pid_lock);										\
	if((name = ((TCP == TCP)?(tcp_info((long)src_ip,(long)des_ip,src_port,des_port,TCP)):(udp_info((long)src_ip,(long)des_ip,src_port,des_port,UDP)))) == NULL)\
		name = raw_info((long)src_ip,(long)des_ip,src_port,des_port,TCP);		\
	pthread_mutex_unlock(&request_pid_lock);
	if(name != NULL){
		if((filepid = strtok(name,"/")) == NULL){	\
			return;								\
		}											\
		if((Temp = strtok(NULL,"/")) == NULL){		\
			return;								\
		}else{										\
			filename = strtok(Temp,":");			\
		}											\
		memcpy(processname,filename,strlen(filename)+1);\
		memcpy(str_pid,filepid,strlen(filepid)+1);  \
	}

}
/*
*function:
*input:   src_ip:待检测数据包的源Ip地址（xxx.xxx.xxx.xxx）         src_port:待检测数据包的源port(如 50000)
		  des_ip:待检测数据包的目的Ip地址（xxx.xxx.xxx.xxx）         des_port:待检测数据包的目的port(如 50000)
		  str_pid:该数据包属于的进程PID processname:该数据包属于的进程名
		  maxlength:src_pid与processname缓冲空间的最小空间（str_pid[100] processname[10] 则应该填入10 
		  空间至少要大于PROGNAME_WIDTH ）
		  如果文件名大于40字节则会出现截断
*/

void udppid_callback\
(char *src_ip,int src_port,char* des_ip,int des_port,char* str_pid,char *processname,char maxlength)
{
	//tcpudpcallback(UDP);

	char stringsrc[40] = {0},stringdst[40] = {0};											\
	char*name = NULL, *Temp = NULL,*filepid = NULL,*filename = NULL,templen = 0;\
	if(maxlength < PROGNAME_WIDTH)												\
		return;																	\
	pthread_mutex_lock(&request_pid_lock);										\
	if((name = ((UDP == TCP)?(tcp_info((long)src_ip,(long)des_ip,src_port,des_port,TCP)):(udp_info((long)src_ip,(long)des_ip,src_port,des_port,UDP)))) == NULL)\
		name = raw_info((long)src_ip,(long)des_ip,src_port,des_port,UDP);		\
	pthread_mutex_unlock(&request_pid_lock);
	if(name != NULL){
		if((filepid = strtok(name,"/")) == NULL){	\
			return;								\
		}											\
		if((Temp = strtok(NULL,"/")) == NULL){		\
			return;								\
		}else{										\
			filename = strtok(Temp,":");			\
		}											\
		memcpy(processname,filename,strlen(filename)+1);\
		memcpy(str_pid,filepid,strlen(filepid)+1);  \
	}

}

//update file  read file time longer
void updatefile(){ 
	char buff[512] = {0};
	char* p = NULL;
	int len  = 0;
	FILE *in = NULL;
	TCP_INIT_VALUE();
	UDP_INIT_VALUE();
	RAW_INIT_VALUE();
	memset(buff,0,sizeof(buff));
	if((in = fopen(_PATH_PROCNET_TCP,"r")) == NULL )
		return ;
	do {						
		if (fgets(buff, sizeof(buff), in)){
			p = (char*)malloc(sizeof(buff));	
			memcpy(p,buff,sizeof(buff));
			list_ins_next(&list_tcpconnectinfo,NULL,p);
			memset(buff,0,sizeof(buff));
		}	
	} while (!feof(in));				
	fclose(in);
	memset(buff,0,sizeof(buff));
	in = NULL;
	if((in = fopen(_PATH_PROCNET_UDP,"r")) == NULL)
		return;
	do {						
		if (fgets(buff, sizeof(buff), in)){
			p = (char*)malloc(sizeof(buff));	
			memcpy(p,buff,sizeof(buff));
			list_ins_next(&list_udpconnectinfo,NULL,p);
			memset(buff,0,sizeof(buff));
		}	
	} while (!feof(in));
	fclose(in);
	memset(buff,0,sizeof(buff));
	in = NULL;
	if((in = fopen(_PATH_PROCNET_RAW,"r")) == NULL)
		return;
	do {						
		if (fgets(buff, sizeof(buff), in)){
			p = (char*)malloc(sizeof(buff));	
			memcpy(p,buff,sizeof(buff));
			list_ins_next(&list_rawconnectinfo,NULL,p);
			memset(buff,0,sizeof(buff));
		}	
	} while (!feof(in));
	fclose(in);
}

//端口解析部分
static void portParse(char* path, int protocol, int type)
{
	char line[512] = {0};
	FILE *in = NULL;

	if((in = fopen(path,"r")) == NULL){
		return ;
	}
	do {						
		if (fgets(line, sizeof(line), in))
		{   //printf("%s",line);
			char localip[33]  = {'\n'},remoteip[33]  = {'\n'};
			char localport[5] = {'\n'},remoteport[5] = {'\n'};
			char status[3]   = {'\n'};
			char uidchar[5]   = {'\n'};
			for(int loop = 0;loop < 128;loop ++)
			{
				if(line[loop] == ':'){
					if(protocol == 4){
						memcpy(localip,   &line[loop + 2], 8);
						memcpy(localport, &line[loop + 11],4);
						memcpy(remoteip,  &line[loop + 16],8);
						memcpy(remoteport,&line[loop + 25],4);
						memcpy(status,    &line[loop + 30],2);
						memcpy(uidchar,	  &line[loop + 73],4);
					}
					else{
						memcpy(localip,   &line[loop + 2],    32);
						memcpy(localport, &line[loop + 11+24],4);
						memcpy(remoteip,  &line[loop + 16+24],32);
						memcpy(remoteport,&line[loop + 25+48],4);
						memcpy(status,&line[loop + 25+48+5],2);
						memcpy(uidchar,	  &line[loop + 73+48],4);
					}
					break;
				}
			}	
			unsigned int localipint    = strtol(localip,   NULL,16);
			unsigned int localportint  = strtol(localport, NULL,16);
			unsigned int remoteipint   = strtol(remoteip,  NULL,16);
			unsigned int remoteportint = strtol(remoteport,NULL,16);
			unsigned int statusint = strtol(status,NULL,16);
			unsigned int uidint = strtol(uidchar,NULL,10);

			/**Only the listening port is left*/
			if ((TCP == type && statusint != 10) ||
				(UDP == type && statusint != 7) ||
				(RAW == type && statusint != 7))
			{
				continue;
			}

			if(!localportint)
				continue;
			if(search_netport_list(localportint)){
				append_netport_list(localportint, 0);
				if (netport_list_init)
				{
					on_onPortOpenEvent_callback(localportint, uidchar);
				}
			}
		}	
	} while (!feof(in));				
	fclose(in);
}

void updateport(){ 
	//PORT_INIT_VALUE();
	change_netport_list();
	portParse(_PATH_PROCNET_TCP, 4, TCP);
	portParse(_PATH_PROCNET_TCP6,6, TCP);
	//portParse(_PATH_PROCNET_UDP, 4, UDP);
	//portParse(_PATH_PROCNET_UDP6,6, UDP);
	//portParse(_PATH_PROCNET_RAW, 4, RAW);
	//portParse(_PATH_PROCNET_RAW6,6, RAW);
	netport_list_init = 1;
}
/**
 * @name:   piddetection_scanner_init
 * @Author: qihoo360
 * @msg:    loop 4S 
 * @param  
 * @return: 
 */
void piddetection_scanner_init(void){
	// pthread_mutex_lock(&request_pid_lock);
	// hashtableinit();
	// pthread_mutex_unlock(&request_pid_lock);
}
/**
 * @name:   pid_value_consumer
 * @Author: qihoo360
 * @msg:    loop 4S 
 * @param  
 * @return: 
 */
void pid_value_consumer(void){
	static int l_loop = 0;
	if(l_loop ++ >= PORT_SCAN_TIME)
	{
		//pthread_mutex_lock(&request_pid_lock);
		//hashtableinit();
		updateport();
		//pthread_mutex_unlock(&request_pid_lock);
		l_loop = 0;
	}
}
 


