/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-06-16 22:14:32
 */ 


#include "https_networkManger.h"
#include "spdloglib.h"
int networkFunctionEnabled()
{
    return 1;
}


static void KeyLoop(int atcion)
{
	while(atcion){
		char *_sManagekey = NULL, *_sSessionkey = NULL, *_sSn = NULL, *_sToken = NULL;
		_sManagekey  = networkMangerMethodobj.getManagerKey();
		_sSessionkey = networkMangerMethodobj.getSessionKey();
		_sSn    	 = networkMangerMethodobj.getSn();
		_sToken      = networkMangerMethodobj.getToken();
		if(_sManagekey){
			printf("idps Key    idps_Key successful");
			printf("idps _sManagekey   = %s\n", _sManagekey);
			printf("idps _sSessionkey   = %s\n",_sSessionkey);
			printf("idps _sSn   = %s\n", _sSn);
			printf("idps _sToken   = %s\n", _sToken);
			return;
		}
		sleep(5);
	}
}

int main(){

    双向认证   证书方式  
    networkMangerMethodobj.newNetworkManager("/home/nvidia/df/df_work/CA/s73a2/","./idps/cache"); //mqtt.pem路径和managekey及sn的存储路径
    networkMangerMethodobj.setDeviceConfig("LQH02501170001","T00001","t-box"); //imei (udid)
    networkMangerMethodobj.setManageKeyStore(1); //设置key_iv的模式
    networkMangerMethodobj.setServerConfig("https://vsocidps-uat.dfiov.com.cn");  //设置baseurl

    char* response = (char* )malloc(GETREQUEST_DATALEN);
    int length = 0;
    memset(response, 0, GETREQUEST_DATALEN);
    networkMangerMethodobj.getUrlRequestData("/api/v1.2/policy/config", 0, &response, &length, GETREQUEST_DATALEN);
    printf("response %s\n",response);
    sleep(5);
    KeyLoop(1);
    for(;;){
        sleep(100);
    }
    
    

    return 0;
}

