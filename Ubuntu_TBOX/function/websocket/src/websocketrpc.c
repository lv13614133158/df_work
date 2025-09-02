#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "stdio.h"
#include "cJSON.h"
#include <stdbool.h>
#include <string.h>
/***rpc related header***/
#include "websocketrpc.h"
#include "websocketclient.h"
#include "Networkmonitor.h"
#include "ProcessMonitor.h"
#include "SysConfigMonitor.h"
#include "ResourceMonitor.h"
#include "FileMonitor.h"
#include "common.h"
#include "websocketConfig.h"
#include "websocketTool.h"

// Demo函数
bool SampleFunc(int argA, char *argB)
{
	bool ret = 1;
	if(argB)
		printf("call SampleFunc %d, %s\n",argA, argB);
	else
	{
		printf("call SampleFunc %d, argB is NULL\n",argA);
	}
	
	return ret;
}

bool test1(int argA, float argB, float argC, double argD, long argE, bool argF, char *argG)
{
	bool ret = 1;
	if(argG)
		printf("call test1 %d, %f, %f, %f, %ld, %d, %s\n",
			argA, argB, argC, argD, argE, argF, *argG);
	else
	{
		printf("call test1 %d, %f, %f, %f, %ld, %d, argG is NULL\n",
			argA, argB, argC, argD, argE, argF);
	}
	return ret;
}

void test3(void)
{
	bool ret = 1;
	printf("call test3\n");
}

void test4(void)
{
	bool ret = 1;
	printf("call test4\n");
}



cJSON* handleResultProcess(char *name, cJSON *arg, cJSON *ret)
{
	cJSON *result = cJSON_CreateObject();
	char *a = cJSON_PrintUnformatted(arg);
	printf("[rpc:arg]:%s\n", a);free(a);
	char *b = cJSON_PrintUnformatted(ret);
	printf("[rpc:retrun]:%s\n", b);free(b);

	if(strcmp(name,"SampleFunc") == 0)
	{
		cJSON_AddStringToObject(result,"action","{\"type\": \"Boolean\",\"type_id\": 3,\"value\": true}");
	}
	else if(strcmp(name,"test1") == 0)
	{
		cJSON_AddStringToObject(result,"action","{\"type\": \"Boolean\",\"type_id\": 3,\"value\": false}");
	}
	else if(strcmp(name,"test3") == 0)
	{
		test3();
		cJSON_AddStringToObject(result,"action","{\"type\": \"Boolean\",\"type_id\": 3}");	
	}
	else if(strcmp(name,"test4") == 0)
	{
		test4();
		cJSON_AddStringToObject(result,"action","{\"type\": \"Boolean\",\"type_id\": 3}");
	}
	else 
	{
		printf("[rpc:retrun]:Unknown function\n");
	}
	return result;
}

cJSON* parseFunctionProcess(cJSON *inFuns)
{
	cJSON *outFuns = cJSON_CreateObject();
	if(inFuns)
	{	
		int  id       	  = cJSON_GetObjectItem(inFuns,"id")->valueint; //valuedouble
		int  version      = cJSON_GetObjectItem(inFuns,"version")->valueint;
		int  module_id    = cJSON_GetObjectItem(inFuns,"module_id")->valueint;
		char *module_name = cJSON_GetObjectItem(inFuns,"module_name")->valuestring;
		char *name        = cJSON_GetObjectItem(inFuns,"name")->valuestring;

		cJSON *js_arg     = cJSON_GetObjectItem(inFuns,"args");
		cJSON *js_return  = cJSON_GetObjectItem(inFuns,"return");
		cJSON *retfuns     = handleResultProcess(name, js_arg, js_return);

		cJSON_AddNumberToObject(outFuns,"id",id);
		cJSON_AddNumberToObject(outFuns,"version",version);
		cJSON_AddNumberToObject(outFuns,"module_id",module_id);
		cJSON_AddStringToObject(outFuns,"module_name",module_name);
		cJSON_AddStringToObject(outFuns,"name",name);
		cJSON_AddNumberToObject(outFuns,"status",retfuns->child==NULL?1:0);
		cJSON_AddItemToObject(outFuns,"result",retfuns);
	}
	else
	{
		printf("Rpc function is null\n");
	}
	
	return outFuns;
}

cJSON *parseBodyProcess(cJSON *inBodys)
{
	cJSON* outBodys  = cJSON_CreateObject();
	if(inBodys)
	{	
		char *sn       = cJSON_GetObjectItem(inBodys,"sn")->valuestring;;
		char *rpc_id   = cJSON_GetObjectItem(inBodys,"rpc_id")->valuestring;
		cJSON *js_funs = cJSON_GetObjectItem(inBodys,"function");
		cJSON *retfuns = parseFunctionProcess(js_funs);

		cJSON_AddStringToObject(outBodys,"sn",sn);
		cJSON_AddStringToObject(outBodys,"rpc_id",rpc_id);
		cJSON_AddItemToObject(outBodys,"result",retfuns);
	}
	else
	{
		printf("Rpc body is null\n");
	}

	return outBodys;
}

char wbsRpcProcess(char* _idata){
	if(!_idata) return 0;

    cJSON* root = cJSON_Parse(_idata);
	char* _laction = cJSON_GetObjectItem(root,"action")->valuestring;
	char* _cseqnumber = cJSON_GetObjectItem(root,"seq_number")->valuestring;
	long long _lseqnumber = strConvertnum(_cseqnumber);
	long long _ltimestamp = cJSON_GetObjectItem(root,"timestamp")->valuedouble;
	if(strcmp(_laction,"rpcrespack") == 0){
		//wbsDelMap(_lseqnumber);
		printf("[删除rpc]：seqnumber：%lld\n",_lseqnumber);
	}
	else if(strcmp(_laction,"rpc") == 0)
	{
		wbsClient_sendRpcAck(_lseqnumber);
		char* _bodystring = cJSON_GetObjectItem(root,"body")->valuestring;
		char* _inputHmac  = cJSON_GetObjectItem(root,"hmac")->valuestring;
		char* _outPutHmac = wbsClient_base64_sha1(_bodystring);
		if((!_outPutHmac)||(!_inputHmac)||(strcmp(_outPutHmac,_inputHmac) != 0)) //校验失败
		{
			log_e("idps_websocket", "[rpc Hmac失败]");
		}
		else
		{               
			cJSON *js_body = cJSON_Parse(_bodystring);
			cJSON *retBody = parseBodyProcess(js_body);
			char *s = cJSON_PrintUnformatted(retBody);
			wbsClient_sendRpcResp(s);
			if(s) free(s);
			if(js_body) cJSON_Delete(js_body);
			if(retBody) cJSON_Delete(retBody);
		}
		if(_outPutHmac)free(_outPutHmac);
	}
	if(root)
		cJSON_Delete(root);

	return 0;
}





