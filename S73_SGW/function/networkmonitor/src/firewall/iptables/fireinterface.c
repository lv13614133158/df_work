/*
 * Author: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 * Based on the ipchains code by Paul Russell and Michael Neuling
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 * 		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 * 		    Marc Boucher <marc+nf@mbsi.ca>
 * 		    James Morris <jmorris@intercode.com.au>
 * 		    Harald Welte <laforge@gnumonks.org>
 * 		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 *	iptables -- IP firewall administration for kernels with
 *	firewall table (aimed for the 2.3 kernels)
 *
 *	See the accompanying manual page iptables(8) for information
 *	about proper usage of this program.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <iptables.h>
#include <stdbool.h>
#include <pthread.h>
#include "cJSON.h"
#include "firewallload.h"
#include <ctype.h>
#include "util.h"

extern void loadstring_init();
extern void standardload_init();
extern void contrackload_init();
extern void DNATload_init();
extern void icmpload_init();
extern void tcpload_init();
extern void udpload_init();
extern void pkttypeload_init();
/*Maximum number of input characters       example:firewall -L -n  is 3*/
#define MAXCMDLEN           							50 
/*pthread mutex  lock*/
pthread_mutex_t mutexmodifyfire = PTHREAD_MUTEX_INITIALIZER; 
extern list list_netinfo;//pid related infomation  define in pid_detection.c
/**save cmd*/
arg_t* argv[MAXCMDLEN];
/*********rules for list******/
list list_firerules;
struct firerule *pfirerule = NULL;
enum Chainlist {
    DROP,// – 防火墙丢弃包
	ACCEPT
};
 /*
* rules related
*/
enum headrules {
    ZERORULES,// 0
	JUSTHAVEHEAD,
	HAVERULES
};
 enum colour {
    black,// 0
	white,
	DNAT,
	SNAT
};

 /**
  * argc:cmd count  argv:save string
  * decla:Because there are too many commands, we use character matching, which is more easy to understand and basically consistent with iptable
  * modify:20200104 frre()delete
  */
 static int stringprocess(int argc, char *argv[])
 {
     int ret;
     char *table = "filter";
     struct xtc_handle *handle = NULL;
  
     ret = do_command4(argc, argv, &table, &handle, false);
     if (ret) {
         pthread_mutex_lock(&mutexmodifyfire);
         ret = iptc_commit(handle);
         iptc_free(handle);
         pthread_mutex_unlock(&mutexmodifyfire);
     }
 }

 /*
* delete head space
*/
char *ltrim(char *str)
{
	if (str == NULL || *str == '\0'){
		return str;
	}
	int len = 0;
	char *p = str;
	while (*p != '\0' && isspace(*p)){
		++p;
		++len;
	}
	memmove(str, p, strlen(str) - len + 1);
	return str;
}
/**
 * declaration: to check iptables exist or not
 */
 bool iptablescheck(void)
 {
	 char iptablesversion[32];
	 FILE *pIptables = NULL;
	 pIptables = popen("iptables --version","r");
	 if(pIptables == NULL){
		 perror("open fd error");
		 return false;
	 }
	 fgets(iptablesversion,sizeof("iptables v1.6.7"),pIptables);
	 if(!memcmp(iptablesversion,"iptables v1.6.7",sizeof("iptables"))){
		 pclose(pIptables);
		 return true;
	 }
	pclose(pIptables);
	return false;
 }
 /*
* split string function
*/
void splitinputstring(char* strinput)
{
	uint8_t argc = 0;
	char *token  = NULL;
	char str[512];
	strcpy(str,strinput);//reason:strinput may function("string"),so we copy 
   	token = strtok(str," ");
    while( token != NULL ) {
	  argv[ argc ] = (arg_t* )malloc(strlen( token ) +2);
	  strcpy(argv[ argc ] ->data,token);
	  argc = (argc + 1)%MAXCMDLEN;
      token = strtok(NULL, " ");
    }
	stringprocess(argc,(char**)argv);
	for(int loop = 0; loop < argc; loop ++)
		free(argv[loop]);
}
 /*
* split cmd response
*/
void queryrulessplit(FILE *ptr){
	struct firerule *pfirerule = NULL;
	char buffer[512],chain[32],policy[32],rulescount = ZERORULES;
	rewind(ptr);
	while(!feof(ptr))
	{
		memset(buffer,0,sizeof(buffer));
		if(NULL != fgets(buffer, 512, ptr)){
			if((strlen(buffer) <= 5))
				continue;
			if((NULL != strstr(buffer,"num   pkts bytes target     prot opt in"))){
				rulescount = JUSTHAVEHEAD;
				continue;
			}
			if((NULL != strstr(buffer,"Chain"))&&(NULL != strstr(buffer,"policy"))){
				if(rulescount == JUSTHAVEHEAD){
					pfirerule = (struct firerule *)malloc(sizeof(struct firerule));		
					memset(pfirerule,0,sizeof(struct firerule));
					memcpy(pfirerule->Chain,chain,strlen(chain));
					memcpy(pfirerule->policy,policy,strlen(policy));
					pfirerule->num = 0;
					list_ins_next(&list_firerules,NULL,pfirerule);
				}
				rulescount == ZERORULES;
				memset(chain,0,sizeof(chain));
				memset(policy,0,sizeof(policy));
				sscanf(buffer,"Chain %[A-Z] (policy %[A-z][^ ]",chain,policy);
				rulescount = 0;
				continue;
			}
			rulescount = HAVERULES;
			pfirerule = (struct firerule *)malloc(sizeof(struct firerule));		
			memset(pfirerule,0,sizeof(struct firerule));
			memcpy(pfirerule->Chain,chain,strlen(chain));
			memcpy(pfirerule->policy,policy,strlen(policy));
			int matchnumber = sscanf(buffer,"%d %*[0-9A-Z] %*[0-9A-Z] %32[A-Z] %[a-z] %s %s %s %s %s %128[^\n]",&(pfirerule->num),pfirerule->target,pfirerule->prot,pfirerule->opt,pfirerule->devicein,pfirerule->deviceout,pfirerule->source,pfirerule->destination,pfirerule->expand);
			list_ins_next(&list_firerules,NULL,pfirerule);
		}
	}
	if(rulescount == JUSTHAVEHEAD){
		pfirerule = (struct firerule *)malloc(sizeof(struct firerule));		
		memset(pfirerule,0,sizeof(struct firerule));
		memcpy(pfirerule->Chain,chain,strlen(chain));
		memcpy(pfirerule->policy,policy,strlen(policy));
		pfirerule->num = 0;
		list_ins_next(&list_firerules,NULL,pfirerule);
	}
}
//init modul  just once
void init_source(){
	int ret = 0;
	static int runstate = ZERORULES;
	if(runstate == ZERORULES)
		runstate = HAVERULES;
	else 
		return;
	iptables_globals.program_name = "iptables";
	ret = xtables_init_all(&iptables_globals, NFPROTO_IPV4);
	if (ret < 0) {
		printf("xtables_init_all wrong\n");
	}
	standardload_init();
	loadstring_init();
	contrackload_init();
	DNATload_init();
	icmpload_init();
	tcpload_init();
	udpload_init();
	pkttypeload_init();
	 
}

 /**
  * decla:process  input iptables string   API
  * date :20200103  mod
  */
void inputprocessstring(char* processstring)
{
	FILE* pIptables = NULL;
 
	if(true == iptablescheck()){
		 pIptables = popen(processstring,"r");
		 if(pIptables == NULL){
			 perror("open fd error");
			 return;
		 }
		 pclose(pIptables);
	 }
	 else{
		 splitinputstring(processstring);
	 }
}
 /**
  * decla:process  internet control   API  MOD
  * tatus:true-->process can connect internet  status:false-->process can not connect internet
  * warning:this function just for android
  * 		linux or QT envirment,please do not use this function!!!!
  * date :20200103
  */
 bool  controlprocessbyandroid(bool status,char* processname,int pidnumber){
	FILE* pPopenFile = NULL,*pCatFile = NULL;
	char  buffer[256],username[128],UID[128];
	if(pidnumber > 0 )
		sprintf(buffer,"ps -aux|awk '{print $1,$2}'|grep \" %d\"|awk '{print $1}'",pidnumber); 
	else if((processname != NULL)&&(processname != ""))
		sprintf(buffer,"ps -aux|awk '{print $1,$11}'|grep \" %s\"|awk '{print $1}'",processname); 
	else
		return false;
	pPopenFile = popen(buffer,"r");
	if(pPopenFile == NULL){
		perror("open fd error");
		return false;
	}
	memset(username,0,sizeof(username));
	if(fgets(username,128,pPopenFile) != NULL){
		pclose(pPopenFile);
		memset(UID,0,sizeof(UID));
		memcpy(UID,username,(strlen(username)>1)?(strlen(username)-1):(1));
		sprintf(buffer,"cat /etc/passwd|grep \"%s\"|awk -F ':' '{print $3}'",UID);
		pCatFile = popen(buffer,"r");
		fgets(UID,128,pCatFile);//UID_name
		if(pCatFile != NULL)
			pclose(pCatFile);
	}else{
		pclose(pPopenFile);
		return false;
	}
	if(strlen(UID) == 0)
		return false;
	memset(username,0,sizeof(username));
	memcpy(username,UID,(strlen(UID)>1)?(strlen(UID)-1):(1));
	sprintf(buffer,"iptables -A OUTPUT -m owner --uid-owner %s -j ",username);
	strcat(buffer,(status)?("ACCEPT"):("DROP"));
	inputprocessstring(buffer);
	return true;
 }
 /**
  * decla:dns related   API MOD
  * date :20191231
  */
 void dnsrulesadd(char *chain,char *dns,char status,char *ethdevice,char action){
	 FILE* pIptables = NULL;
	 char processstring[128],chainpool[64];
	 memset(chainpool,0,sizeof(chainpool));
	 if((chain == NULL)||(chain == ""))
		memcpy(chainpool,"INPUT",sizeof("INPUT"));
	else
	   memcpy(chainpool,chain,strlen(chain));
	 if(action)
	 	memcpy(processstring,"iptables -A ",sizeof("iptables -A "));
	 else
	 	memcpy(processstring,"iptables -D ",sizeof("iptables -D "));
	strcat(processstring,chainpool);
	 if(ethdevice != NULL){
		if((NULL != strstr(chainpool,"INPUT"))||(NULL != strstr(chainpool,"FORWARD"))||(NULL != strstr(chainpool,"PREROUTING")))
			strcat(processstring," -i "); 
		else
			strcat(processstring," -o "); 
		strcat(processstring,ethdevice); 
	 }
	 strcat(processstring," -m string --string \"");
	 strcat(processstring,dns);
	 strcat(processstring,"\" --algo bm --to 65535 -j ");
	 strcat(processstring,(status == DROP)?("DROP"):("ACCEPT"));
	 inputprocessstring(processstring);
 }
 /**
  * decla:destion address convert   API
  * date:20191231
  * param: beforeip(after):XXX.XXX.XXX.XXX or XXX.XXX.XXX.XXX/mask
  */
 void destionaddressconvert(char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice,char action){
	 FILE* pIptables = NULL;
	 char  fromstring[128],portstr[16];
	 memset(fromstring,0,sizeof(fromstring));
	 if(action)
	 	memcpy(fromstring,"iptables -t nat -A PREROUTING ",sizeof("iptables -t nat -A PREROUTING "));
	 else
	 	memcpy(fromstring,"iptables -t nat -D PREROUTING ",sizeof("iptables -t nat -D PREROUTING "));
	 if((ethdevice != NULL)&&(ethdevice != "")){
		 strcat(fromstring,"-i ");
		 strcat(fromstring,ethdevice);
		 strcat(fromstring," ");
	 }
	 
	 if((protocol != NULL)&&(protocol != "")&&(memcmp(protocol,"all",sizeof("all")) != 0)){
		 strcat(fromstring,"-p ");
		 strcat(fromstring,protocol);
		 strcat(fromstring," ");
	 }
	 if((beforeip != NULL)&&(beforeip != "")){
		strcat(fromstring,"-d ");
	 	strcat(fromstring,beforeip);
	 	strcat(fromstring," ");
	 }
	 if(beforport != 0){
		 strcat(fromstring,"--dport ");
		 sprintf(portstr,"%d",beforport);
		 strcat(fromstring,portstr);
		 strcat(fromstring," ");
	 }
	 strcat(fromstring,"-j DNAT --to-destination ");
	 if((afterip != NULL)&&(afterip != "")){
		 strcat(fromstring,afterip);
	 }
	 if(afterport != 0){
		 strcat(fromstring,":");
		 sprintf(portstr,"%d",afterport);
		 strcat(fromstring,portstr);
	 }
 	 //printf("string is %s\n",fromstring);
	 inputprocessstring(fromstring);
	  
 }
/*
* query current rules   API
* firewall -L -n --line-number
* size: data size     data:copy space
* table:if no set ,chain:filter
*/
char* queryallrules(char *chain)    
{
	uint8_t argc = 0,loop = 0;
	char *token  = NULL,*s = NULL;
	FILE *piptables = NULL;
	char str[256];
	list_init(&list_firerules,free);//malloc rules
	strcpy(str,"iptables -vL -n --line-number");
	if((chain != NULL)&&(chain != "")){
		strcat(str," -t ");
		strcat(str,chain);
	}
	 
	if(true == iptablescheck()){
		piptables = popen(str,"r");
		queryrulessplit(piptables); 
		if(piptables != NULL)
			pclose(piptables);
	}
	else{
		token = strtok(str," ");
		while( token != NULL ) {
			argv[ argc ] = (arg_t* )malloc(strlen( token ) +2);
			strcpy(argv[ argc ] ->data,token);
			argc = (argc + 1)%MAXCMDLEN;
			token = strtok(NULL, " ");
		}
		stringprocess(argc,(char**)argv);	
		for(int loop = 0; loop < argc; loop ++)
			free(argv[loop]);		
	}
	cJSON *queryCjson,*pobj[list_size(&list_firerules)];
	queryCjson = cJSON_CreateArray();
	list_elmt *cur_elmt = list_head(&list_firerules);
	while(cur_elmt != NULL)
	{
		struct firerule *e = cur_elmt->data;
		cJSON_AddItemToArray(queryCjson,pobj[loop] = cJSON_CreateObject());
		cJSON_AddStringToObject(pobj[loop],"chain",e->Chain);
		cJSON_AddStringToObject(pobj[loop],"policy",e->policy);
		cJSON_AddNumberToObject(pobj[loop],"number",e->num);
		cJSON_AddStringToObject(pobj[loop],"target",e->target);
		cJSON_AddStringToObject(pobj[loop],"in",e->devicein);
		cJSON_AddStringToObject(pobj[loop],"out",e->deviceout);
		cJSON_AddStringToObject(pobj[loop],"prot",e->prot);
		cJSON_AddStringToObject(pobj[loop],"opt",e->opt);
		cJSON_AddStringToObject(pobj[loop],"source",e->source);
		cJSON_AddStringToObject(pobj[loop],"destination",e->destination);
		cJSON_AddStringToObject(pobj[loop],"expand",e->expand);
		loop ++;
		cur_elmt = cur_elmt->next;
	}
	s = cJSON_PrintUnformatted(queryCjson);
	if(queryCjson)
		cJSON_Delete(queryCjson);
	list_destroy(&list_firerules);//free list
	return s;
}

/**
 * delete point rules       API
 * chain:INPUT /OUTPUT /FORWARD//all     and so on
 * num  :delete rules number 
 * if num equal zero,then clear all rules in this chain
 * if chain is  NULL or "all",then clear all rules  
 * table:nat /filter,if table is NULL,then table is filter  
 */
void deleterules(char* chain,int num,char* table)
{
	char catstring[256],number[16];
	FILE *piptables = NULL;
	sprintf(number," %d",num);
	if((num == 0)||(chain == NULL)||(chain == "all")||(chain == "")){
		memcpy(catstring,"iptables -F ",sizeof("iptables -F "));
		if((chain != NULL)&&(chain != "all"))
			strcat(catstring,chain);
		if((table != NULL)&&(table != "")){
			strcat(catstring," -t ");
			strcat(catstring,table);
		}
	}
	else{
		memcpy(catstring,"iptables -D ",sizeof("iptables -D "));
		strcat(catstring,chain);
		strcat(catstring,number);
		if((table != NULL)&&(table != "")){
			strcat(catstring," -t ");
			strcat(catstring,table);
		}
	}
	 
	if(true == iptablescheck()){
		piptables = popen(catstring,"r");
		if(piptables != NULL)
			pclose(piptables);
	}
	else{
		splitinputstring(catstring);
	}
}
/**
 *example:iptables -P INPUT DROP   API
 *chain  :INPUT/OUTPUT/FORWARD  
 *status : 
 *date   :20191231
 */
void chainstatusset(char* chain,int status)
{
	char catstr[128];
	FILE *piptables = NULL;
	memcpy(catstr,"iptables -P ",sizeof("iptables -P "));
	strcat(catstr,chain);
	strcat(catstr,(status == DROP)?(" DROP"):(" ACCEPT"));
	 
	if(true == iptablescheck()){
		piptables = popen(catstr,"r");
		if(piptables != NULL)
			pclose(piptables);
	}
	else{
		splitinputstring(catstr);
	}
}
/**
 * spt:source port(if spt equal 0,we will not set)  dpt:dest port (if dpt equal 0,we will not set)   
 * protocol:tcp/udp/icmp/all/esp/ah(choose one )  status:TRUE->pass FALSE->forbid
 * srcip:source ip(XXX.XXX.XXX.XXX or xxx.xxx.xxx.xxx/mask   if you do not want to set ,then NULL) 
 * dstip:dest ip(XXX.XXX.XXX.XXX or xxx.xxx.xxx.xxx/mask     if you do not want to set ,then NULL ) 
 * chain:INPUT OUTPUT   FORWARD  PREROUTING  (choose one)
 * example:firewall -A INPUT -p tcp --dport 80 --sport 80 -j ACCEPT
 * status :0:no pass   !0:pass   
 */
void filterportpassornot(char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,char* chain,char status,char action)
{
	char catstring[256],sport[24],dport[24];
	FILE *piptables = NULL;
	sprintf(sport," --sport %d",spt);
	sprintf(dport," --dport %d",dpt);
	if(action)
		sprintf(catstring,"iptables -A %s -p %s",chain,protocol);
	else
		sprintf(catstring,"iptables -D %s -p %s",chain,protocol);
	if(ethdevice != NULL){
		if((NULL != strstr(chain,"INPUT"))||(NULL != strstr(chain,"FORWARD"))||(NULL != strstr(chain,"PREROUTING")))
			strcat(catstring," -i ");
		else
			strcat(catstring," -o ");
		strcat(catstring,ethdevice);
	}
	if(srcip != NULL){
		strcat(catstring," -s ");
		strcat(catstring,srcip);
		strcat(catstring," ");
	}
	if(dstip != NULL){
		strcat(catstring," -d ");
		strcat(catstring,dstip);
		strcat(catstring," ");
	}
	if(spt)strcat(catstring,sport);
	if(dpt)strcat(catstring,dport);
	strcat(catstring," -j ");
	strcat(catstring,(status != 0)?("ACCEPT"):("DROP"));
   
	if(true == iptablescheck()){
		piptables = popen(catstring,"r");
		if(piptables != NULL)
			pclose(piptables);
	}
	else{
		splitinputstring(catstring);
	}
}
/**
 * declaration：check appoint list kernel rules and list rules
 * listchain: list_ip_blacklist  list_ip_whitelist or other ethdevice list  
 * srcip(dstip):XXX.XXX.XXX.XXX/MASK
 * colour: balck or white(must match listchain)
 */
bool check_iprules_list(list *listchain,char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,char colour){
	list_elmt *cur_elmt = list_head(listchain);
	while(cur_elmt != NULL)
	{
		struct iplist *ipinfo = cur_elmt->data;
		if((memcmp(protocol,ipinfo->prot,strlen(ipinfo->prot)) == 0)&&((memcmp(srcip,ipinfo->source,strlen(ipinfo->source)) == 0))&&\
		(memcmp(dstip,ipinfo->destination,strlen(ipinfo->destination)) == 0)&&(ipinfo->dstport == dpt)\
		&&(ipinfo->srcport == spt)&&(memcmp(ethdevice,ipinfo->eth,strlen(ipinfo->eth)) == 0)){
			return true;
		}
		cur_elmt = cur_elmt->next;
	}
	return false;
}
/**
 * declaration:add white ip list(add INPUT chain and OUTPUT chain)
 * listchain:list_ip_whitelist
 * date     :20200114
 */
void addwhiteinode(char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,list *listchain){
	char stringpool[128] ={0},chain[64] = {0},ethchannel[32] = {0};
	char srcipinput[32] = {0},dstipinput[32] = {0};
	if(check_iprules_list(listchain,ethdevice,spt,dpt,srcip,dstip,protocol,white))
		return;
	memcpy(dstipinput,srcip,strlen(srcip));
	memcpy(srcipinput,dstip,strlen(dstip));
	memcpy(chain,"INPUTWHITE",strlen("INPUTWHITE"));
	strcat(chain,ethdevice);
	memcpy(ethchannel," -i ",sizeof(" -i "));
	for(char loop = 0;loop < 2;loop ++){
		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -D  ",strlen("iptables -D "));
		strcat(stringpool,chain);
		strcat(stringpool,ethchannel);
		strcat(stringpool,ethdevice);
		strcat(stringpool," -j DROP");
		inputprocessstring(stringpool);	

		filterportpassornot(ethdevice,spt,dpt,srcipinput,dstipinput,protocol,chain,true,true);//input output

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -A ",strlen("iptables -A "));
		strcat(stringpool,chain);
		strcat(stringpool,ethchannel);
		strcat(stringpool,ethdevice);
		strcat(stringpool," -j DROP");
		inputprocessstring(stringpool);

		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTWHITE",strlen("OUTPUTWHITE"));
		strcat(chain,ethdevice);
		memset(srcipinput,0,sizeof(srcipinput));
		memset(dstipinput,0,sizeof(dstipinput));
		memcpy(srcipinput,srcip,strlen(srcip));
		memcpy(dstipinput,dstip,strlen(dstip));
		memset(ethchannel,0,sizeof(ethchannel));
		memcpy(ethchannel," -o ",sizeof(" -o "));
	}
	struct iplist *ipinfo = (struct iplist *)malloc(sizeof(struct iplist));	
	memset(ipinfo,0,sizeof(struct iplist));
	memcpy(ipinfo->target,"ACCEPT",strlen("ACCEPT"));
	memcpy(ipinfo->Chain,chain,strlen(chain));
	memcpy(ipinfo->prot,protocol,strlen(protocol));
	memcpy(ipinfo->source,srcip,strlen(srcip));
	memcpy(ipinfo->destination,dstip,strlen(dstip));
	memcpy(ipinfo->eth,ethdevice,strlen(ethdevice));
	ipinfo->dstport = dpt;
	ipinfo->srcport = spt;
	list_ins_next(listchain,NULL,ipinfo);
}

/**
 * declaration：check point rule exist 

 * listchain: list_dns_whitelist  list_dns_blacklist or other ethdevice list  
 * colour:black white(must match listchain)
 */
bool check_dnsrules_list(char *dns,char *ethdevice,list *listchain){
	list_elmt *cur_elmt = list_head(listchain);
	while(cur_elmt != NULL)
	{
		struct stringmatch *stringget = cur_elmt->data;
		if((memcmp(dns,stringget->dnsstring,strlen(stringget->dnsstring)) == 0)&&((memcmp(ethdevice,stringget->eth,strlen(stringget->eth)) == 0))){
			return true;
		}
		cur_elmt = cur_elmt->next;
	}
	return false;
}
/**
 * declaration:add dns inode  ip (add INPUT chain and OUTPUT chain)
 * listchain:list_dns_whitelist
 */
void addwhitednsinode(char *dns,char *ethdevice,list *listchain){
	char stringpool[128],chain[64] = {0},ethchannel[32] = {0};
	if(check_dnsrules_list(dns,ethdevice,listchain))
		return;
	memcpy(chain,"INPUTWHITE",strlen("INPUTWHITE"));
	strcat(chain,ethdevice);
	memcpy(ethchannel," -i ",sizeof(" -i "));
	for(char loop = 0;loop < 1/*2*/;loop ++){//20210527
		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -D  ",strlen("iptables -D "));
		strcat(stringpool,chain);
		strcat(stringpool,ethchannel);
		strcat(stringpool,ethdevice);
		strcat(stringpool," -j DROP");
		inputprocessstring(stringpool);	

		dnsrulesadd(chain,dns,true,ethdevice,true);
		
		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -A ",strlen("iptables -A "));
		strcat(stringpool,chain);
		strcat(stringpool,ethchannel);
		strcat(stringpool,ethdevice);
		strcat(stringpool," -j DROP");
		inputprocessstring(stringpool);

		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTWHITE",strlen("OUTPUTWHITE"));
		strcat(chain,ethdevice);
		memset(ethchannel,0,sizeof(ethchannel));
		memcpy(ethchannel," -o ",sizeof(" -o "));
	}

	struct stringmatch *stringget = (struct stringmatch *)malloc(sizeof(struct stringmatch));
	memset(stringget,0,sizeof(struct stringmatch));
	memcpy(stringget->algostring,"bm",strlen("bm"));
	memcpy(stringget->target,"ACCEPT",strlen("ACCEPT"));
	memcpy(stringget->destip,"0.0.0.0/0",strlen("0.0.0.0/0"));
	memcpy(stringget->dnsstring,dns,strlen(dns));
	memcpy(stringget->srcip,"0.0.0.0/0",strlen("0.0.0.0/0"));
	memcpy(stringget->protocol,"all",strlen("all"));
	memcpy(stringget->chain,chain,strlen(chain));
	memcpy(stringget->fromstring,"0",strlen("0"));
	memcpy(stringget->tostring,"65535",strlen("65535"));
	memcpy(stringget->eth,ethdevice,strlen(ethdevice));//record device name for multi device
	list_ins_next(listchain,NULL,stringget);
}
/**
 * declaration:add black dns list
 * listchain:list_dns_blacklist
 */ 
void adddnsblacklist(char *dns,char *ethdevice,list *listchain){
	char chain[64]={0};
	if(check_dnsrules_list(dns,ethdevice,listchain))
		return;
	memcpy(chain,"INPUTBLACK",strlen("INPUTBLACK"));
	strcat(chain,ethdevice);
	for(char loop =0;loop < 1/*2*/ ;loop ++){//20210527 OUTPUT dns drop may cause can not dedect rules
		dnsrulesadd(chain,dns,false,ethdevice,true);
		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTBLACK",strlen("OUTPUTBLACK"));
		strcat(chain,ethdevice);
	}
	struct stringmatch *stringget = (struct stringmatch *)malloc(sizeof(struct stringmatch));
	memset(stringget,0,sizeof(struct stringmatch));
	memcpy(stringget->algostring,"bm",strlen("bm"));
	memcpy(stringget->target,"ACCEPT",strlen("ACCEPT"));
	memcpy(stringget->destip,"0.0.0.0/0",strlen("0.0.0.0/0"));
	memcpy(stringget->dnsstring,dns,strlen(dns));
	memcpy(stringget->srcip,"0.0.0.0/0",strlen("0.0.0.0/0"));
	memcpy(stringget->protocol,"all",strlen("all"));
	memcpy(stringget->chain,chain,strlen(chain));
	memcpy(stringget->fromstring,"0",strlen("0"));
	memcpy(stringget->tostring,"65535",strlen("65535"));
	memcpy(stringget->eth,ethdevice,strlen(ethdevice));//record device name for multi device
	list_ins_next(listchain,NULL,stringget);
}

/**
 * declaration：del appoint dns list rules and kernel rules
 * listchain: list_dns_blacklist or other ethdevice list  
 * colour:black white(must match listchain)
 */
bool deldnslist(char *dns,char *ethdevice,char colour,list *listchain){
	void *data;
	char chain[64] = {0};
	list_elmt *old_elmt = NULL;
	list_elmt *cur_elmt = list_head(listchain);
	
	while(cur_elmt != NULL)
	{
		struct stringmatch *stringget = cur_elmt->data;
		if((memcmp(dns,stringget->dnsstring,strlen(stringget->dnsstring)) == 0)&&((memcmp(ethdevice,stringget->eth,strlen(stringget->eth)) == 0))){
			if(colour == black){
				memcpy(chain,"INPUTBLACK",strlen("INPUTBLACK"));
				strcat(chain,ethdevice);
				dnsrulesadd(chain,dns,false,ethdevice,false);
				/*
				memset(chain,0,sizeof(chain));
				memcpy(chain,"OUTPUTBLACK",strlen("OUTPUTBLACK"));
				strcat(chain,ethdevice);
				dnsrulesadd(chain,dns,false,ethdevice,false);*/
			}
			else{
				memcpy(chain,"INPUTWHITE",strlen("INPUTWHITE"));
				strcat(chain,ethdevice);
				dnsrulesadd(chain,dns,true,ethdevice,false);
				/*memset(chain,0,sizeof(chain));
				memcpy(chain,"OUTPUTWHITE",strlen("OUTPUTWHITE"));
				strcat(chain,ethdevice);
				dnsrulesadd(chain,dns,true,ethdevice,false);*/
			}
			list_delindex(listchain,old_elmt);	
			return true;
		}
		old_elmt = cur_elmt;
		cur_elmt = cur_elmt->next;
	}
	return false;
}
/**
 * declaration：ip black list add
 * listchain: list_ip_blacklist or other ethdevice list  
 * dstip(srcip):XXX.XXX.XXX.XXX/MASK
 */
void addipblacklistrule(char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol,list *listchain){
	char chain[64] = {0};
	char srcipinput[32] ={0},dstipinput[32] = {0};
	if(check_iprules_list(listchain,ethdevice,spt,dpt,srcip,dstip,protocol,black))
		return;
	memcpy(dstipinput,srcip,strlen(srcip));
	memcpy(srcipinput,dstip,strlen(dstip));
	memset(chain,0,sizeof(chain));
	memcpy(chain,"INPUTBLACK",strlen("INPUTBLACK"));
	strcat(chain,ethdevice);
	for(char loop =0;loop <2 ;loop ++){
		filterportpassornot(ethdevice,spt,dpt,srcipinput,dstipinput,protocol,chain,false,true);
		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTBLACK",strlen("OUTPUTBLACK"));
		strcat(chain,ethdevice);
		memset(dstipinput,0,strlen(dstipinput));
		memset(srcipinput,0,strlen(srcipinput));
		memcpy(srcipinput,srcip,strlen(srcip));
		memcpy(dstipinput,dstip,strlen(dstip));
	}
	struct iplist *ipinfo = (struct iplist *)malloc(sizeof(struct iplist));	
	memset(ipinfo,0,sizeof(struct iplist));
	memcpy(ipinfo->target,"ACCEPT",strlen("ACCEPT"));
	memcpy(ipinfo->Chain,"INPUT",strlen("INPUT"));
	memcpy(ipinfo->prot,protocol,strlen(protocol));
	memcpy(ipinfo->source,srcip,strlen(srcip));
	memcpy(ipinfo->destination,dstip,strlen(dstip));
	memcpy(ipinfo->eth,ethdevice,strlen(ethdevice));
	ipinfo->dstport = dpt;
	ipinfo->srcport = spt;
	list_ins_next(listchain,NULL,ipinfo);
}

/**
 * declaration：del appoint list kernel rules and list rules
 * listchain: list_ip_blacklist  list_ip_whitelist or other ethdevice list  
 * beforip(afterip):XXX.XXX.XXX.XXX/MASK
 * colour: balck or white(must match listchain)
 */
bool deliplist(list *listchain,char colour,char* ethdevice,unsigned int spt,unsigned int dpt,char* srcip,char* dstip,char* protocol){
	void *data;
	char chain[64] = {0};
	list_elmt *old_elmt = NULL;
	list_elmt *cur_elmt = list_head(listchain);
	
	while(cur_elmt != NULL)
	{
		struct iplist *ipinfo = cur_elmt->data;
		if((memcmp(protocol,ipinfo->prot,strlen(ipinfo->prot)) == 0)&&((memcmp(srcip,ipinfo->source,strlen(ipinfo->source)) == 0)&&\
		(memcmp(dstip,ipinfo->destination,strlen(ipinfo->destination)) == 0)&&(ipinfo->dstport == dpt)\
		&&(ipinfo->srcport == spt)&&(memcmp(ethdevice,ipinfo->eth,strlen(ipinfo->eth)) == 0))){
			if(colour == black){
				memset(chain,0,sizeof(chain));
				memcpy(chain,"INPUTBLACK",strlen("INPUTBLACK"));
				strcat(chain,ethdevice);
				filterportpassornot(ethdevice,spt,dpt,dstip,srcip,protocol,chain,false,false);
				memset(chain,0,sizeof(chain));
				memcpy(chain,"OUTPUTBLACK",strlen("OUTPUTBLACK"));
				strcat(chain,ethdevice);
				filterportpassornot(ethdevice,spt,dpt,srcip,dstip,protocol,chain,false,false);
			}
			else{
				memset(chain,0,sizeof(chain));
				memcpy(chain,"INPUTWHITE",strlen("INPUTWHITE"));
				strcat(chain,ethdevice);
				filterportpassornot(ethdevice,spt,dpt,dstip,srcip,protocol,chain,true,false);
				memset(chain,0,sizeof(chain));
				memcpy(chain,"OUTPUTWHITE",strlen("OUTPUTWHITE"));
				strcat(chain,ethdevice);
				filterportpassornot(ethdevice,spt,dpt,srcip,dstip,protocol,chain,true,false);
			}
			list_delindex(listchain,old_elmt);	
			return true;
		}
		old_elmt = cur_elmt;
		cur_elmt = cur_elmt->next;
	}
	return false;
}
/**
 * declaration：dnat convert
 * listchain: list_dnat_blacklist or other ethdevice list  
 * beforip(afterip):XXX.XXX.XXX.XXX/MASK
 */
void adddnatlist(list *listchain,char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice){

	destionaddressconvert(protocol,beforeip,beforport,afterip,afterport,ethdevice,true);
	struct dnatlist *dnatinfo = (struct dnatlist *)malloc(sizeof(struct dnatlist));	
	memset(dnatinfo,0,sizeof(struct dnatlist));
	memcpy(dnatinfo->prot,protocol,strlen(protocol));
	memcpy(dnatinfo->source,beforeip,strlen(beforeip));
	memcpy(dnatinfo->destination,afterip,strlen(afterip));
	memcpy(dnatinfo->eth,ethdevice,strlen(ethdevice));
	dnatinfo->dstport = afterport;
	dnatinfo->srcport = beforport;
	list_ins_next(listchain,NULL,dnatinfo);
} 
/**
 * declaration：check list rules(dnat)
 * listchain: list_dnat_blacklist or other ethdevice list  
 * beforip(afterip):XXX.XXX.XXX.XXX/MASK
 */
bool check_dnatrules_black_list(list *listchain,char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice){
	list_elmt *cur_elmt = list_head(listchain);
	while(cur_elmt != NULL)
	{
		struct dnatlist *dnatinfo = cur_elmt->data;
		if((memcmp(protocol,dnatinfo->prot,strlen(dnatinfo->prot)) == 0)&&((memcmp(beforeip,dnatinfo->source,strlen(dnatinfo->source)) == 0)&&\
		(memcmp(afterip,dnatinfo->destination,strlen(dnatinfo->destination)) == 0)&&(dnatinfo->dstport == afterport)\
		&&(dnatinfo->srcport == beforport)&&(memcmp(ethdevice,dnatinfo->eth,strlen(dnatinfo->eth)) == 0))){
			return true;
		}
		cur_elmt = cur_elmt->next;
	}
	return false;
}
/**
 * declaration：del appoint list kernel rules and list rules(dnat related) 
 * listchain: list_dnat_blacklist or other ethdevice list  
 * beforip(afterip):XXX.XXX.XXX.XXX/MASK
 */
bool deldnatlist(list *listchain,char* protocol,char* beforeip,int beforport,char* afterip,int afterport,char* ethdevice){
	void *data;
	list_elmt *cur_elmt = list_head(listchain);
	list_elmt *old_elmt = NULL;
	while(cur_elmt != NULL)
	{
		struct dnatlist *dnatinfo = cur_elmt->data;
		if((memcmp(protocol,dnatinfo->prot,strlen(dnatinfo->prot)) == 0)&&((memcmp(beforeip,dnatinfo->source,strlen(dnatinfo->source)) == 0)&&\
		(memcmp(afterip,dnatinfo->destination,strlen(dnatinfo->destination)) == 0)&&(dnatinfo->dstport == afterport)\
		&&(dnatinfo->srcport == beforport)&&(memcmp(ethdevice,dnatinfo->eth,strlen(dnatinfo->eth)) == 0))){
			destionaddressconvert(dnatinfo->prot,dnatinfo->source,dnatinfo->srcport,dnatinfo->destination,dnatinfo->dstport,dnatinfo->eth,false);
			list_delindex(listchain,old_elmt);	
			return true;
		}
		old_elmt = cur_elmt;
		cur_elmt = cur_elmt->next;
	}
	return false;
}
/**
 * declaration：operation all blackip list
 * listchain: list_ip_blacklist or other ethdevice list  
 * action   :false :delete(clear all list netfilter rules but still store in list)   true:insert(reloadinit function :rules restore)
 */
void allipblacklist(list* listchain,bool action){
	list_elmt *cur_elmt = list_head(listchain);
	list_elmt *old_elmt = NULL;
	char chain[64];
	while(cur_elmt != NULL)
	{
		struct iplist *ipinfo = cur_elmt->data;
		memset(chain,0,sizeof(chain));
		memcpy(chain,"INPUTBLACK",strlen("INPUTBLACK"));
		strcat(chain,ipinfo->eth);
		filterportpassornot(ipinfo->eth,ipinfo->srcport,ipinfo->dstport,ipinfo->destination,ipinfo->source,ipinfo->prot,chain,false,action);
		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTBLACK",strlen("OUTPUTBLACK"));
		strcat(chain,ipinfo->eth);
		filterportpassornot(ipinfo->eth,ipinfo->srcport,ipinfo->dstport,ipinfo->source,ipinfo->destination,ipinfo->prot,chain,false,action);
		cur_elmt = cur_elmt->next;
	}
}
/**
 * declaration：operation all blackdns list
 * listchain: list_dns_blacklist or other ethdevice list  
 * action   :false :delete(clear all list netfilter rules but still store in list)   true:insert(reloadinit function :rules restore)
 */
void allblackdnslist(list* listchain,bool action){
	list_elmt *cur_elmt = list_head(listchain);
	char chain[64];
	while(cur_elmt != NULL)
	{
		struct stringmatch *stringget = cur_elmt->data;
		memset(chain,0,sizeof(chain));
		memcpy(chain,"INPUTBLACK",strlen("INPUTBLACK"));
		strcat(chain,stringget->eth);
		dnsrulesadd(chain,stringget->dnsstring,false,stringget->eth,action);
		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTBLACK",strlen("OUTPUTBLACK"));
		strcat(chain,stringget->eth);
		dnsrulesadd(chain,stringget->dnsstring,false,stringget->eth,action);
		cur_elmt = cur_elmt->next;
	}
}
/**
 * declaration：operation all whiteip list
 * listchain: list_ip_whitelist or other ethdevice list  
 * action   :false :delete(clear all list netfilter rules but still store in list)   true:insert(reloadinit function :rules restore)
 */
void allwhiteiplist(list* listchain,bool action){
	list_elmt *cur_elmt = list_head(listchain);
	char chain[64];
	while(cur_elmt != NULL)
	{
		struct iplist *ipinfo = cur_elmt->data;
		memset(chain,0,sizeof(chain));
		memcpy(chain,"INPUTWHITE",strlen("INPUTWHITE"));
		strcat(chain,ipinfo->eth);
		filterportpassornot(ipinfo->eth,ipinfo->srcport,ipinfo->dstport,ipinfo->destination,ipinfo->source,ipinfo->prot,chain,true,action);
		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTWHITE",strlen("OUTPUTWHITE"));
		strcat(chain,ipinfo->eth);
		filterportpassornot(ipinfo->eth,ipinfo->srcport,ipinfo->dstport,ipinfo->source,ipinfo->destination,ipinfo->prot,chain,true,action);
		cur_elmt = cur_elmt->next;
	}
}
/**
 * declaration：operation all whitedns lis
 * listchain: list_dns_whitelist or other ethdevice list  
 * action   :false :delete(clear all list netfilter rules but still store in list)   true:insert(reloadinit function :rules restore)
 */
void allwhitednslist(list* listchain,bool action){
	list_elmt *cur_elmt = list_head(listchain);
	char chain[64];
	while(cur_elmt != NULL)
	{
		struct stringmatch *stringget = cur_elmt->data;
		memset(chain,0,sizeof(chain));
		memcpy(chain,"INPUTWHITE",strlen("INPUTWHITE"));
		strcat(chain,stringget->eth);
		dnsrulesadd(chain,stringget->dnsstring,true,stringget->eth,action);
		memset(chain,0,sizeof(chain));
		memcpy(chain,"OUTPUTWHITE",strlen("OUTPUTWHITE"));
		strcat(chain,stringget->eth);
		dnsrulesadd(chain,stringget->dnsstring,true,stringget->eth,action);
		cur_elmt = cur_elmt->next;
	}
}
/**
 * declaration：operation all dnatlist
 * listchain: list_dnat_blacklist or other ethdevice list  
 * action   :false :delete(clear all list netfilter rules but still store in list)   true:insert(reloadinit function :rules restore)
 */
void alldnatlist(list* listchain,bool action){
	list_elmt *cur_elmt = list_head(listchain);
	while(cur_elmt != NULL)
	{
		struct dnatlist *dnatinfo = cur_elmt->data;
		destionaddressconvert(dnatinfo->prot,dnatinfo->source,dnatinfo->srcport,dnatinfo->destination,dnatinfo->dstport,dnatinfo->eth,action);
		cur_elmt = cur_elmt->next;
	}
}
/**
 * create list  and add chain to input /output
 * clear chain
 */ 
void createlist(char* ethwhite,char* ethblack){
	char stringpool[128];
	init_source();
	memset(stringpool,0,sizeof(stringpool));
	if(ethblack != NULL){
		memcpy(stringpool,"iptables -N INPUTBLACK",strlen("iptables -N INPUTBLACK"));
		strcat(stringpool,ethblack);
		//splitinputstring(stringpool);
		inputprocessstring(stringpool);

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -D INPUT -i ",strlen("iptables -D INPUT -i "));
		strcat(stringpool,ethblack);
		strcat(stringpool," -j INPUTBLACK");
		strcat(stringpool,ethblack);
		inputprocessstring(stringpool); 
		//splitinputstring(stringpool);

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -I INPUT 1 -i ",strlen("iptables -I INPUT 1 -i "));
		strcat(stringpool,ethblack);
		strcat(stringpool," -j INPUTBLACK");
		strcat(stringpool,ethblack);
		inputprocessstring(stringpool);//add
		 
		/////////////////////////
		//clear list
		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -F INPUTBLACK",strlen("iptables -F INPUTBLACK"));
		strcat(stringpool,ethblack);
		inputprocessstring(stringpool);

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -N OUTPUTBLACK",strlen("iptables -N OUTPUTBLACK"));
		strcat(stringpool,ethblack);
		inputprocessstring(stringpool);//create

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -D OUTPUT -o ",strlen("iptables -D OUTPUT -o "));
		strcat(stringpool,ethblack);
		strcat(stringpool," -j OUTPUTBLACK");
		strcat(stringpool,ethblack);
		inputprocessstring(stringpool);//delete 

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -I OUTPUT 1 -o ",strlen("iptables -I OUTPUT 1 -o "));
		strcat(stringpool,ethblack);
		strcat(stringpool," -j OUTPUTBLACK");
		strcat(stringpool,ethblack);
		inputprocessstring(stringpool);//add

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -F OUTPUTBLACK",strlen("iptables -F OUTPUTBLACK"));
		strcat(stringpool,ethblack);
		inputprocessstring(stringpool);

	}
	if(ethwhite != NULL){
		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -N INPUTWHITE",strlen("iptables -N INPUTWHITE"));
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);//create

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -D INPUT -i ",strlen("iptables -D INPUT -i "));
		strcat(stringpool,ethwhite);
		strcat(stringpool," -j INPUTWHITE");
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);//delete 

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -I INPUT 1 -i ",strlen("iptables -I INPUT 1 -i "));
		strcat(stringpool,ethwhite);
		strcat(stringpool," -j INPUTWHITE");
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);//add

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -F INPUTWHITE",strlen("iptables -F INPUTWHITE"));
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -A INPUTWHITE",strlen("iptables -A INPUTWHITE"));
		strcat(stringpool,ethwhite);
		strcat(stringpool," -i ");
		strcat(stringpool,ethwhite);
		strcat(stringpool," -j DROP");
		inputprocessstring(stringpool);

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -N OUTPUTWHITE",strlen("iptables -N OUTPUTWHITE"));
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);//create

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -D OUTPUT -o ",strlen("iptables -D OUTPUT -o "));
		strcat(stringpool,ethwhite);
		strcat(stringpool," -j OUTPUTWHITE");
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);//delete 

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -I OUTPUT 1 -o ",strlen("iptables -I OUTPUT 1 -o "));
		strcat(stringpool,ethwhite);
		strcat(stringpool," -j OUTPUTWHITE");
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);//add

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -F OUTPUTWHITE",strlen("iptables -F OUTPUTWHITE"));
		strcat(stringpool,ethwhite);
		inputprocessstring(stringpool);

		memset(stringpool,0,sizeof(stringpool));
		memcpy(stringpool,"iptables -A OUTPUTWHITE",strlen("iptables -A OUTPUTWHITE"));
		strcat(stringpool,ethwhite);
		strcat(stringpool," -o ");
		strcat(stringpool,ethwhite);
		strcat(stringpool," -j DROP");
		inputprocessstring(stringpool);
	}
}



