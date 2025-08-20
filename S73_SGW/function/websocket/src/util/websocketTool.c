#include <limits.h>
#include "websocketTool.h"
#include "base64.h"
#include "common.h"
#include "myHmac.h"

static pthread_rwlock_t rwlockPosition;

static float positionInfo(float * _ilat,float* _ilongi,int _set)
{
	static float _latitude  = 0;
	static float _longitude = 0;
	float _ltemp = 0;
	switch (_set){
		case 0:
			pthread_rwlock_wrlock(&rwlockPosition);
			_latitude = *_ilat;
			_longitude = *_ilongi;
			pthread_rwlock_unlock(&rwlockPosition);
		break;
		case 1:
			pthread_rwlock_rdlock(&rwlockPosition);
			_ltemp = _longitude;
			pthread_rwlock_unlock(&rwlockPosition);
		break;
		case 2:
			pthread_rwlock_rdlock(&rwlockPosition);
			_ltemp = _latitude;
			pthread_rwlock_unlock(&rwlockPosition);
		break;
		default:
		break;
	}
	return _ltemp;
}
void wbsClient_initPosition(){
	pthread_rwlock_init(&rwlockPosition,NULL);
}  

void wbsClient_endPosition(){
	pthread_rwlock_destroy(&rwlockPosition);
}  

void wbsClient_setPosition(float _lat,float _longi){
	positionInfo(&_lat,&_longi,0);
}  
/**
 * @description: getPositionLat
 * @param {*}
 * @return {*}
 */
float wbsClient_getPositionLat(void){
	return positionInfo(NULL,NULL,2);;
}
/**
 * @description: getPositionLong
 * @param {*}
 * @return {*}
 */
float wbsClient_getPositionLong(void){
	return positionInfo(NULL,NULL,1);;
}

long long wbsClient_getTimestampMs(void){
	long long _ltimestamp = 0;
	_ltimestamp = clockobj.get_current_time();
	return _ltimestamp;
}

long long wbsClient_getSeqnumber(void){
	static long long _lseqnumber = 1;
	if(_lseqnumber == LLONG_MAX) 
		_lseqnumber = 1;
	return _lseqnumber++;
}

char* wbsClient_getSeqnumberEx(char *out){
	static bool initStatus = true;
	static long long _lseqnumber = 1;
	if( initStatus )
	{
		initStatus = false;
		_lseqnumber = clockobj.get_current_time();
	}
	if(out)
		sprintf(out, "%lld", _lseqnumber);
		
	if(++_lseqnumber == LLONG_MAX) 
		_lseqnumber = 1;
	
	return out;
}

char* wbsClient_base64_sha1(char* _iinput){
	unsigned char sha1[20] = {0};
	get_string_sha1((const char *)_iinput,strlen(_iinput), sha1);
	size_t data_length_hmac;
    char *base64_hmac = (char *)base64_encode((unsigned char *)sha1,20,&data_length_hmac);
	return base64_hmac;
}

// str必须大于26字节
// long long 大于16~17位（double）精度不准,int最大10位。
char* numConvertStr(long long num, char* str)
{
	if(str)
	{
		sprintf(str, "%lld", num);
	}

	return str;
}

long long strConvertnum(char* str)
{
	long long num = 0;
	if(str)
	{
		sscanf(str,"%lld",&num);
	}
	return num;
}