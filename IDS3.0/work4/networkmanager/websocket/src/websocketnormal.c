#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "websocketclient.h"
#include "websocketnormal.h"
#include "websocketConfig.h"
#include "websocketLoop.h"
#include "websocketTool.h"
#include "websocketmanager.h"


int get_hard(char **outbuf)
{
	cJSON *root=NULL,*item = NULL,*ipArray=NULL,*ipnode = NULL;
	// IpNode_t *i,*h = &ip_head;
	char *temp = NULL;
	char buf[100]={0};
	int ret;
	char *network = NULL;
	//init_hardinfo();
	//网卡信息封装层json字符串传递，避免上层解析出错
	// ipArray = cJSON_CreateArray();
	// if(ipArray == NULL)
	// 	return -1;
	// cJSON_AddItemToArray(ipArray,ipnode = cJSON_CreateObject());
	// for(i = h->next;i!=NULL;i=i->next)
	// {
	// 	cJSON_AddStringToObject(ipnode,i->name,i->ip);	
	// }
	// network = cJSON_PrintUnformatted(ipArray);

	// ret = get_device_sn(buf,sizeof(buf));
	// if(ret < 0)
	// 	memset(buf,0,sizeof(buf));

	root=cJSON_CreateObject();
	// root = cJSON_CreateArray();
	if(root == NULL)
		return -1;
	// cJSON_AddItemToArray(root,item = cJSON_CreateObject());
	// cJSON_AddNumberToObject(root,"core_num",obj.core_num); 
	// cJSON_AddStringToObject(root,"cpu_model_name",obj.cpu_model_name);
	// cJSON_AddStringToObject(root,"host_name",obj.host_name);
	// cJSON_AddStringToObject(root,"ip_addr",network);

	// cJSON_AddStringToObject(root,"machine",obj.machine);
	// cJSON_AddNumberToObject(root,"mem_total",obj.ram_size);
	// cJSON_AddStringToObject(root,"release",obj.release);
	// cJSON_AddStringToObject(root,"os_name",obj.sys_name);  
	// cJSON_AddStringToObject(root,"os_version",obj.version); 

	cJSON_AddNumberToObject(root,"core_num",networkManagerObj.config.core_num); //"CPU核心数"
	cJSON_AddStringToObject(root,"cpu_model_name",networkManagerObj.config.cpu_model_name);//CPU型号
	cJSON_AddStringToObject(root,"host_name",networkManagerObj.config.host_name);//主机名
	cJSON_AddStringToObject(root,"ip_addr",networkManagerObj.config.ip_addr);//IP地址
	cJSON_AddStringToObject(root,"machine",networkManagerObj.config.machine);//机器类型
	cJSON_AddNumberToObject(root,"mem_total",networkManagerObj.config.mem_total);//"总内存"
	cJSON_AddStringToObject(root,"release",networkManagerObj.config.release);//系统版本
	cJSON_AddStringToObject(root,"os_name",networkManagerObj.config.os_name);  //系统名称
	cJSON_AddStringToObject(root,"os_version",networkManagerObj.config.os_version); //系统版
	//destory_ip_list();
	char *s = cJSON_PrintUnformatted(root);
	if(network)
	{
		free(network);
	}
	if(ipArray)
		cJSON_Delete(ipArray);
	if(root)
	{
		cJSON_Delete(root);
	}
	if(s == NULL)
	{
		return -1;
	}

	temp = (char *)malloc(strlen(s)+1);
	if(temp == NULL)
	{
		free(s);
		return -1;
	}
	memset(temp,0,strlen(s)+1);
	memcpy(temp,s,strlen(s));
	*outbuf = temp;
	free(s);	
	return 0;
}

char* getTerminalInfomation(char** _output)
{
	cJSON *array = NULL,*root = NULL,*cjson_data1=NULL,*cjson_data2=NULL;
	char mac_addr[100] = {0};
	get_hard(_output);
	cjson_data2 = cJSON_Parse((char *)(*_output));
    root = cJSON_CreateObject();
    cjson_data1 = cJSON_CreateObject();
	// cJSON_AddStringToObject(root,"sn",networkMangerMethodobj.getSn());
	// cJSON_AddStringToObject(cjson_data1,"vin",getVIN());
	// cJSON_AddStringToObject(cjson_data1,"model",getCAR());
	// cJSON_AddStringToObject(cjson_data1,"brand","东风");
	// cJSON_AddStringToObject(cjson_data2,"sn",getTCUID());        
	// cJSON_AddStringToObject(cjson_data2,"idps_version",IDS_VERSION);
	// cJSON_AddStringToObject(cjson_data2,"manufacturer",getManufacturer());
	// cJSON_AddStringToObject(cjson_data2,"simu_sys_version", getSIMU());
	// char *addr =  get_monitor_mac();
	// memcpy(local_net_mac_hex,addr,6);
	// sprintf(mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",local_net_mac_hex[0],local_net_mac_hex[1],local_net_mac_hex[2],local_net_mac_hex[3],local_net_mac_hex[4],local_net_mac_hex[5]);
	// cJSON_AddStringToObject(cjson_data2,"mac_addr",mac_addr);
	// cJSON_AddItemToObject(root, "vehicle_info", cjson_data1);
	// cJSON_AddItemToObject(root, "terminal_info", cjson_data2);

	cJSON_AddStringToObject(root,"sn",networkMangerMethodobj.getSn());
	cJSON_AddStringToObject(cjson_data1,"vin",networkManagerObj.vin);
	cJSON_AddStringToObject(cjson_data1,"model",networkManagerObj.config.model);
	cJSON_AddStringToObject(cjson_data1,"brand",networkManagerObj.config.brand);
	cJSON_AddStringToObject(cjson_data2,"sn",networkManagerObj.sn);        
	cJSON_AddStringToObject(cjson_data2,"idps_version",networkManagerObj.config.idps_version);
	cJSON_AddStringToObject(cjson_data2,"manufacturer",networkManagerObj.config.manufacturer);
	cJSON_AddStringToObject(cjson_data2,"simu_sys_version", networkManagerObj.config.simu_sys_version);
	cJSON_AddStringToObject(cjson_data2,"mac_addr",networkManagerObj.config.mac_addr);
	cJSON_AddItemToObject(root, "vehicle_info", cjson_data1);
	cJSON_AddItemToObject(root, "terminal_info", cjson_data2);


    char *s = cJSON_PrintUnformatted(root);
	char *a = cJSON_Print(root);
	//printf("hardinfo:%s\n\n",a);
	if(a)free(a);
    if(root)
        cJSON_Delete(root);
	return s;
}

int statusProcessNormal(int _istatus,char* _iaction,long _iseqnumber){
	int _lstatus = _istatus;
    long long _lseqnumber = _iseqnumber;
	switch(_lstatus)
	{
		case websocketsucess:
			if(strcmp(_iaction,"connack") == 0)
			{
				char *info = NULL;
				char *new_info = getTerminalInfomation(&info);
				websocketMangerMethodobj.sendInfoData(new_info);
				if(info)free(info);
				lstagerun = websocketstage2;
			}
			if(strcmp(_iaction,"infoack") == 0){
				lstagerun = websocketstage2;
				wbsDelMap(_lseqnumber);
				printf("[删除信息]：seqnumber：%lld\n\n",_lseqnumber);
			}
			if(strcmp(_iaction,"heartack") == 0){
				printf("[删除心跳]：seqnumber：%lld\n\n",_lseqnumber);
			}
			if(strcmp(_iaction,"eventack") == 0){
				wbsDelMap(_lseqnumber);
				printf("[删除事件]：seqnumber：%lld\n\n",_lseqnumber);
			}
			break;
		case websocketfail:
			printf("[Normal失败]：seqnumber：%lld\n\n",_lseqnumber);
			break;
		case websocketsndup:
			break;
		case websocketchannelunsupport://session again
			break;
		case websocketequimentunsupport:
			break;
		case websockrtkeyinlegal:
			break;
		case websocketversionerror:
			break;
		default:
			break;
	}
	return _lstatus;
}

char wbsNormalProcess(char* _idata)
{
	int status = 0;
	if(!_idata)return 0;
    cJSON* root = cJSON_Parse(_idata);
	char* _laction = cJSON_GetObjectItem(root,"action")->valuestring;
	if(strcmp(_laction,"heartack") == 0)
	{
		long long _ltimestamp = cJSON_GetObjectItem(root,"timestamp")->valuedouble;
		//clockobj.sync_clock(_ltimestamp);  //同步时间
	}
	if(strcmp(_laction,"infoack") == 0)
	{
		cJSON *js_status = cJSON_GetObjectItem(root,"status");
		if(js_status)
			status = js_status->valueint;
	}
	if(strcmp(_laction,"eventack") == 0)
	{
		cJSON *js_status = cJSON_GetObjectItem(root,"status");
		if(js_status)
			status = js_status->valueint;
	}

	char* _cseqnumber = cJSON_GetObjectItem(root,"seq_number")->valuestring;
	long long _lseqnumber = strConvertnum(_cseqnumber);
	statusProcessNormal(status,_laction,_lseqnumber);
	if(root)
		cJSON_Delete(root);
	return 0;
}

