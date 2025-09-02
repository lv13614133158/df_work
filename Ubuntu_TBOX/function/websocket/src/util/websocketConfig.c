#include <stdio.h>
#include "spdloglib.h"
#include "Base_networkmanager.h"
#include "websocketConfig.h"


#define KEY_SAVE_MAX  128
#define KEY_IV_LENGTH 16

typedef struct _keystore{
	char m_manageKey[KEY_SAVE_MAX];
	char m_sessionKey[KEY_SAVE_MAX];
	char m_sn[KEY_SAVE_MAX];
	char m_token[512];
}keystore;


static bool m_readyKeyFlag = false;
static keystore keystoreObj = {0};
configWebsocket websocketTable = {0};

int wbsSetConfig(char* _iurl,char* _ichannelid,char* _iequmenttype,char* _isn,int _iversion,int _ssl,int _port, char* _path)
{
	memset(&websocketTable,0,sizeof(configWebsocket));
	if(_ichannelid)strncpy(websocketTable.channelId,_ichannelid,128);
	if(_iequmenttype)strncpy(websocketTable.equment,_iequmenttype,128);
	if(_iurl)strncpy(websocketTable.url,_iurl,128);
	if(_isn)strncpy(websocketTable.sn,_isn,128);
	if(_path)strncpy(websocketTable.certsPath,_path,128);
	websocketTable.version     = _iversion;
	websocketTable.port        = _port;
	websocketTable.sslShutdown = _ssl;
}

char* wbsGetSnnumber(void){
	return keystoreObj.m_sn;
}

char* wbsGetTokennumber(void){
	return keystoreObj.m_token;
}

static void wbsSetManageKey(char* _iinput){
	memset(keystoreObj.m_manageKey,0,KEY_SAVE_MAX);
	if(_iinput){
		memcpy(keystoreObj.m_manageKey,_iinput,strlen(_iinput));
		log_v("idps_websocket","websocket managerkey get success");
	}
}

static void wbsSetSessionKey(char* _iinput){
	memset(keystoreObj.m_sessionKey,0,KEY_SAVE_MAX);
	if(_iinput){
		memcpy(keystoreObj.m_sessionKey,_iinput,strlen(_iinput));
		log_v("idps_websocket","websocket sessionkey get success");
	}
}

static void wbsSetSnnumber(char* _iinput){
	memset(keystoreObj.m_sn,0,128);
	if(_iinput){
		memcpy(keystoreObj.m_sn,_iinput,strlen(_iinput));
		log_v("idps_websocket","websocket getsnbumber success");
	}
}

static void wbsSetTokennumber(char* _iinput){
	memset(keystoreObj.m_token,0,512);
	if(_iinput){
		memcpy(keystoreObj.m_token,_iinput,strlen(_iinput));
		log_v("idps_websocket","websocket gettokenbumber success");
	}
}

// 获取sessionkey, managekey, sn, token信息赋值给变量
bool wbsSetKeyInfo(){
    char* _ltemp = NULL;
    bool _lsessionkey = false,_lmanagekey = false,_lsn = false,_ltoken = false; 
   
	if(((_ltemp = networkMangerMethodobj.getManagerKey()) != NULL)&&(strlen(_ltemp) > 2)){
		wbsSetManageKey(_ltemp);
		_lmanagekey = true;
		_ltemp = NULL;
	}
	if(((_ltemp = networkMangerMethodobj.getSessionKey()) != NULL)&&(strlen(_ltemp) > 2)){
		wbsSetSessionKey(_ltemp);			
		_lsessionkey = true;
		_ltemp = NULL;
	}
	if(((_ltemp = networkMangerMethodobj.getSn()) != NULL)&&(strlen(_ltemp) > 2)){
		wbsSetSnnumber(_ltemp);
		_lsn = true;
		_ltemp = NULL;
	}
	if(((_ltemp = networkMangerMethodobj.getToken()) != NULL)&&(strlen(_ltemp) > 2)){
		wbsSetTokennumber(_ltemp);
		_ltoken = true;
	}

	if(_lmanagekey && _lsessionkey && _lsn && _ltoken)
	{
		log_v("idps_websocket","managerkey && sessionkey && snnumber && tokennumber get success");
		m_readyKeyFlag = true;
	}else
	{
		log_v("idps_websocket","getKey failed");		
	}
	
	return m_readyKeyFlag;
}

bool wbsGetKeyStatus()
{ 
	return m_readyKeyFlag;
}

