#include "websocketTool.h"
#include "websocketUtils.h"
#include "websocketConfig.h"
#include "runtimemanager.h"
#include "cJSON.h"
#include <string.h>

/* 错误码 */
#define    IDPS_SOFTWARE_ONGOING        0x05000000
#define    IDPS_SOFTWARE_NORUNNING      0x05000001
#define    IDPS_WORKFLOW_SUCCESS        0x05010000
#define    IDPS_WORKFLOW_NOVIN          0x05010001
#define    IDPS_WORKFLOW_NOSN           0x05010002
#define    IDPS_WORKFLOW_NONETCOMMU     0x05010003
#define    IDPS_WORKFLOW_NOCERT         0x05010004
#define    IDPS_PROGRAM_OK              0x05020000
#define    IDPS_PROGRAM_NOTOK           0x05020001
#define    IDPS_PROGRAM_OOM             0x05020002

enum webProcessType wbsDispatchData(char* _iflowdata){
	//decode json
	enum webProcessType _ltype = OTHERS; 
	if(!_iflowdata)return _ltype;
	cJSON* _lcJSONArray = NULL;
        _lcJSONArray = cJSON_Parse(_iflowdata);
	if(!_lcJSONArray)return _ltype;
	cJSON* _lactionjson = cJSON_GetObjectItem(_lcJSONArray,"action");
	if(_lactionjson == NULL){
		_ltype = OTHERS;
		cJSON_Delete(_lcJSONArray);
		return _ltype;
	}
	char* _laction = _lactionjson->valuestring;
	if((strncmp(_laction,"connack",strlen("connack"))) == 0){
		_ltype = NORMAL;
	}
	else if(strncmp(_laction,"heartack",strlen("heartack")) == 0){
		_ltype = NORMAL;
	}
	else if(strncmp(_laction,"eventack",strlen("eventack")) == 0){
		_ltype = NORMAL;
	}
	else if(strncmp(_laction,"statusack",strlen("statusack")) == 0){
		_ltype = NORMAL;
	}

	if((strncmp(_laction,"infoack",strlen("infoack"))) == 0){
		_ltype = NORMAL;
	}
	else if(strncmp(_laction,"rpc",strlen("rpc")) == 0){
		_ltype = RPC;
	}
	else if(strncmp(_laction,"rpcrespack",strlen("rpcrespack")) == 0){
		_ltype = RPC;
	}
	
	cJSON_Delete(_lcJSONArray);
	return _ltype;
}

char* wbsGetformEvent(char* data, char* postid, long long* plSeqNumber)
{
	cJSON* CjsonMain,* CjsonSecond,* CjsonThird = NULL;
	long long timestamp  = wbsClient_getTimestampMs();
	long long lseqnumber = wbsClient_getSeqnumber();
	unsigned char str_lseqnumber[26] = {0};
	numConvertStr(lseqnumber, str_lseqnumber);

	CjsonThird = cJSON_Parse(data);
	CjsonMain  = cJSON_CreateObject();
	if(CjsonMain == NULL)return NULL;

	cJSON_AddStringToObject(CjsonMain,"action","event");
	cJSON_AddStringToObject(CjsonMain,"seq_number",str_lseqnumber);
	cJSON_AddNumberToObject(CjsonMain,"timestamp",timestamp);

    CjsonSecond = cJSON_CreateObject();
	cJSON_AddStringToObject(CjsonSecond,"sn",wbsGetSnnumber());
	cJSON_AddStringToObject(CjsonSecond,"type",postid);
	cJSON_AddNumberToObject(CjsonSecond,"latitude", wbsClient_getPositionLat());
	cJSON_AddNumberToObject(CjsonSecond,"longitude",wbsClient_getPositionLong());
	cJSON_AddStringToObject(CjsonSecond,"version","1.5.0");
	cJSON_AddItemToObject(CjsonSecond,"data",CjsonThird);

	char* boy = cJSON_PrintUnformatted(CjsonSecond);
	if(CjsonSecond)cJSON_Delete(CjsonSecond);
	cJSON_AddStringToObject(CjsonMain,"body",boy);
	char* base64_bodystring = wbsClient_base64_sha1(boy);
	cJSON_AddStringToObject(CjsonMain,"hmac",base64_bodystring);
	cJSON_AddNumberToObject(CjsonMain,"len",strlen(boy));
	if(base64_bodystring)free(base64_bodystring);
	if(boy)free(boy);

	if(plSeqNumber)*plSeqNumber = lseqnumber;
	char* eventData = cJSON_PrintUnformatted(CjsonMain);
	if(CjsonMain)cJSON_Delete(CjsonMain);

	return eventData;	
}

char* wbsGetformInfo(char* data, long long* plSeqNumber)
{
	cJSON* CjsonMain = NULL;
	long long timestamp  = wbsClient_getTimestampMs();
	long long lseqnumber = wbsClient_getSeqnumber();
	unsigned char str_lseqnumber[26] = {0};
	numConvertStr(lseqnumber, str_lseqnumber);

	CjsonMain = cJSON_CreateObject();
	if(CjsonMain == NULL)return NULL;
    
	cJSON_AddStringToObject(CjsonMain,"action","info");
	cJSON_AddStringToObject(CjsonMain,"seq_number",str_lseqnumber);
	cJSON_AddNumberToObject(CjsonMain,"timestamp",timestamp);
	cJSON_AddStringToObject(CjsonMain,"body",data);

	char* base64_bodystring = wbsClient_base64_sha1(data);
	cJSON_AddStringToObject(CjsonMain,"hmac",base64_bodystring);
	if(base64_bodystring)free(base64_bodystring);

	if(plSeqNumber)*plSeqNumber = lseqnumber;
	char* infoData = cJSON_PrintUnformatted(CjsonMain);
	if(CjsonMain)cJSON_Delete(CjsonMain);

	return infoData;
}

char* wbsGetHeartBeat(long long* plSeqNumber)
{
	cJSON* CjsonMain = NULL;
	long long timestamp = wbsClient_getTimestampMs();
	long long lseqnumber = wbsClient_getSeqnumber();
	unsigned char str_lseqnumber[26] = {0};
	numConvertStr(lseqnumber, str_lseqnumber);
	long long lruntime_OS   = getOSTime();
	long long lruntime_IDPS = getRunTime();

	CjsonMain = cJSON_CreateObject();
	if(CjsonMain == NULL)return NULL;

	cJSON_AddStringToObject(CjsonMain,"action","heart");
	cJSON_AddStringToObject(CjsonMain,"seq_number",str_lseqnumber);
	cJSON_AddNumberToObject(CjsonMain,"timestamp",timestamp);
	cJSON_AddNumberToObject(CjsonMain,"runtime_OS",lruntime_OS);
	cJSON_AddNumberToObject(CjsonMain,"runtime_IDPS",lruntime_IDPS);

	if(plSeqNumber)
		*plSeqNumber = lseqnumber;
	char* heartData = cJSON_PrintUnformatted(CjsonMain);
	if(CjsonMain)
		cJSON_Delete(CjsonMain);

	return heartData;
}

char* wbsGetRpcAck(long long lSeqNumber)
{
	cJSON* CjsonMain = NULL;
	long long timestamp = wbsClient_getTimestampMs();
	unsigned char str_lseqnumber[26] = {0};
	numConvertStr(lSeqNumber, str_lseqnumber);

	CjsonMain = cJSON_CreateObject();
	if(CjsonMain == NULL)return NULL;
	cJSON_AddStringToObject(CjsonMain,"action","rpcack");
	cJSON_AddStringToObject(CjsonMain,"seq_number",str_lseqnumber);
	cJSON_AddNumberToObject(CjsonMain,"timestamp",timestamp);
	
	char* rpcAck = cJSON_PrintUnformatted(CjsonMain);
	if(CjsonMain)
		cJSON_Delete(CjsonMain);

	return rpcAck;
}

char* wbsGetRpcResp(char* data, long long* plSeqNumber)
{
	cJSON* CjsonMain = NULL;
	long long timestamp  = wbsClient_getTimestampMs();
	long long lseqnumber = wbsClient_getSeqnumber();
	unsigned char str_lseqnumber[26] = {0};
	numConvertStr(lseqnumber, str_lseqnumber);

	CjsonMain = cJSON_CreateObject();
	if(CjsonMain == NULL)return NULL;
    
	cJSON_AddStringToObject(CjsonMain,"action","rpcresp");
	cJSON_AddStringToObject(CjsonMain,"seq_number",str_lseqnumber);
	cJSON_AddNumberToObject(CjsonMain,"timestamp",timestamp);
	cJSON_AddStringToObject(CjsonMain,"body",data);

	char* base64_bodystring = wbsClient_base64_sha1(data);
	cJSON_AddStringToObject(CjsonMain,"hmac",base64_bodystring);
	if(base64_bodystring)free(base64_bodystring);

	if(plSeqNumber)*plSeqNumber = lseqnumber;
	char* rpcResp = cJSON_PrintUnformatted(CjsonMain);
	if(CjsonMain)
		cJSON_Delete(CjsonMain);

	return rpcResp;
}
char* wbsGetformIDPSErr(long long* plSeqNumber)
{
	cJSON* CjsonMain,* CjsonSecond,* CjsonThird = NULL;
	long long timestamp  = wbsClient_getTimestampMs();
	long long lseqnumber = wbsClient_getSeqnumber();
	unsigned char str_lseqnumber[26] = {0};
	numConvertStr(lseqnumber, str_lseqnumber);

	CjsonMain  = cJSON_CreateObject();
	if(CjsonMain == NULL)return NULL;

	cJSON_AddStringToObject(CjsonMain,"action","status");
	cJSON_AddStringToObject(CjsonMain,"seq_number",str_lseqnumber);
	cJSON_AddNumberToObject(CjsonMain,"timestamp",timestamp);

	CjsonSecond = cJSON_CreateObject();
	cJSON_AddNumberToObject(CjsonSecond,"software",IDPS_SOFTWARE_ONGOING);
	cJSON_AddNumberToObject(CjsonSecond,"workflow", IDPS_WORKFLOW_SUCCESS);
	cJSON_AddNumberToObject(CjsonSecond,"program",IDPS_PROGRAM_OK);

	char* boy = cJSON_PrintUnformatted(CjsonSecond);
	if(CjsonSecond)cJSON_Delete(CjsonSecond);
	cJSON_AddStringToObject(CjsonMain,"body",boy);
	char* base64_bodystring = wbsClient_base64_sha1(boy);
	cJSON_AddStringToObject(CjsonMain,"hmac",base64_bodystring);
	cJSON_AddNumberToObject(CjsonMain,"len",strlen(boy));
	if(base64_bodystring)free(base64_bodystring);
	if(boy)free(boy);

	if(plSeqNumber)*plSeqNumber = lseqnumber;
	char* eventData = cJSON_PrintUnformatted(CjsonMain);
	if(CjsonMain)cJSON_Delete(CjsonMain);

	return eventData;
}
