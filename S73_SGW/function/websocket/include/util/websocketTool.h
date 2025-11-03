#ifndef  __WEBSOCKETTOOL_H_
#define  __WEBSOCKETTOOL_H_
#ifdef __cplusplus
extern "C"
{
#endif
void  wbsClient_initPosition();
void  wbsClient_endPosition();
void  wbsClient_setPosition(float _lat,float _longi);
float wbsClient_getPositionLat(void);
float wbsClient_getPositionLong(void);

long long wbsClient_getSeqnumber(void);
long long wbsClient_getTimestampMs(void);
char* wbsClient_getSeqnumberEx(char *out);

char* wbsClient_base64_sha1(char* _iinput);

// str必须大于26字节
char* numConvertStr(long long num, char* str);
long long strConvertnum(char* str);

#ifdef __cplusplus
}
#endif 
#endif