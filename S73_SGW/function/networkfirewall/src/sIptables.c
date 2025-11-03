#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "spdloglib.h"
#include "cJSON.h"
#include "util.h"
#include "spdloglib.h"
#include "sIptables.h"

char iptables_table[][16]={"filter","nat","mangle","raw"}; //表
char iptables_chain[][16]={"PREROUTING","INPUT","OUTPUT","FORWARD","POSTROUTING"};//规则链
char iptables_action[][16]={"ACCEPT","REJECT","DROP"};     //动作


static void _addIptable(char *body)
{
    char cmd[255] ={0};
    sprintf(cmd, "iptables %s", body);
    log_v("networkfirewall", cmd);
    system(cmd);
}

int setIptable(char* rules)
{
    if(rules != NULL)
    {             
        cJSON *cJSONList = cJSON_Parse(rules);
        int size =cJSON_GetArraySize(cJSONList);
        if (size == 0){                         
            cJSON_Delete(cJSONList);                     
            return 0;                     
        }
        for (int i = 0; i < size; i ++) {
            cJSON *child = cJSON_GetArrayItem(cJSONList, i);
            if(child){
                _addIptable(child->valuestring);
            }
        }
        cJSON_Delete(cJSONList);  
    }
    return 1;
}

int addIptableForlist(list *rules)
{
    char cmd[255] ={0};
    list_elmt *head = rules->head;
    
    if(head ==NULL)   
        return -1;
    list_elmt *cur = head;
    while(cur != NULL)
    {
        char *body = (char*)cur->data;
        sprintf(cmd, "iptables %s",body);
        log_e("networkfirewall",cmd);
        system(cmd);
        cur = cur->next;
    }

    return 0;
}

int addIptable(char* tables, char* chains, char* sip, char *action)
{
    char cmd[255] ={0};

    //sprintf(cmd, "iptables -t %s -A %s -s %s -d %s -j %s",iptables, chains, ip, des, action);
    // -t 表、-A(命令) 规则链、-s 源IP -j 触发动作
    sprintf(cmd, "iptables -t %s -A %s -s %s -j %s",tables, chains, sip, action);

    return system(cmd);
}


int delIptable(char* tables, char* chains, int line)
{
    char cmd[255] ={0};

    sprintf(cmd, "iptables -t %s -D %s %d",tables, chains, line);
    return system(cmd);
}

int viewIptable(char* tables)
{
    char cmd[255] ={0}, result_buf[255];
    
    if(tables)
        sprintf(cmd, "iptables -t %s -L %s -s %s -j %s",tables, tables, tables, tables);
    else
        sprintf(cmd, "iptables -L");

    FILE *fp = popen(cmd, "r");
    if( fp == NULL)
    {
        perror("popen执行失败！");
        return -1;
    }
    while(fgets(result_buf, sizeof(result_buf), fp) != NULL)
    {
        printf("%s\n", result_buf);
    }
    pclose(fp);
    
    return 0;
}