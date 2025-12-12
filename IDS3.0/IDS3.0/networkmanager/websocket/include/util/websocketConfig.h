#ifndef  __WEBSOCKETCONFIG_H_
#define  __WEBSOCKETCONFIG_H_
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _configWebsocket{
	char url[128];
	char equment[128];
	char channelId[128];
	char sn[128];
	char certsPath[128];
    int  sslShutdown;
    int  port;
	int  version;
}configWebsocket; 
extern configWebsocket websocketTable;
// 配置设置
int  wbsSetConfig(char* _iurl,char* _ichannelid,char* _iequmenttype,char* _isn, int _iversion,int _ssl,int _port, char* _path);
#define wbsGetUrl()      websocketTable.url
#define wbsGetEqument()  websocketTable.equment
#define wbsGetChannelId()websocketTable.channelId
#define wbsGetSn()       websocketTable.sn
#define wbsGetSsl()      websocketTable.sslShutdown
#define wbsGetPort()     websocketTable.port
#define wbsGetPath()     websocketTable.certsPath
// Key设置
char* wbsGetSnnumber(void);
char* wbsGetTokennumber(void);
bool  wbsSetKeyInfo();
bool  wbsSetKeyStatus();

#ifdef __cplusplus
}
#endif 
#endif