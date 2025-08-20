#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "websocketclient.h"
#include "websocketnormal.h"
#include "SysConfigMonitor.h"
#include "websocketConfig.h"
#include "websocketLoop.h"
#include "websocketTool.h"

int statusProcessNormal(int _istatus,char* _iaction,long _iseqnumber){
	int _lstatus = _istatus;
    long long _lseqnumber = _iseqnumber;
	switch(_lstatus)
	{
		case websocketsucess:
			if(strcmp(_iaction,"connack") == 0)
			{
				char *info = NULL;
				char *new_info = SysConfigMonitorObj.getTerminalInfo(&info);
				SysConfigMonitorObj.runoneshot(new_info);
				if(info)free(info);
				lstagerun = websocketstage2;
				printf("login success\n");
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
			if(strcmp(_iaction,"statusack") == 0){
				wbsDelMap(_lseqnumber);
				printf("[删除状态码]：seqnumber：%lld\n\n",_lseqnumber);
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

