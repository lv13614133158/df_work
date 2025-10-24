/*
 * @Author: your name
 * @Date: 2021-07-29 04:33:26
 * @LastEditTime: 2021-08-08 22:13:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/websocket/websocketclient.h
 */
#ifndef __WEBSOCKETCLIENT_H_
#define __WEBSOCKETCLIENT_H_
#include <stdbool.h>
#include "util.h"
/**
 * @description: global stage 
 * @param {*}
 * @return {*}
 */
enum _stagerun{
	websocketstage0,//before login
	websocketstage2 //login  sucess
};

/**
 * @description:enum message 
 * @param {*}
 * @return {*}
 */
enum _websocketstatus{
	websocketsucess = 0,         //成功
	websocketfail,               //失败
	websocketsndup,              //SN重复
	websocketchannelunsupport,	 //组件通道号不支持
	websocketequimentunsupport,  //组件类型不支持
	websockrtkeyinlegal,         //秘钥非法请重新注册
	websocketversionerror        //版本号不一致
};


void wbsClient_localStopWbClient(void);
int  wbsClient_localWebSocketclient();
void wbsClient_sendEventData(char* _postid,char* _data,char* _event);
void wbsClient_sendInfoData(char* _data);
void wbsClient_sendHeartBeat(void);
void wbsClient_sendRpcAck(long long lseqnumber);
void wbsClient_sendRpcResp(char* _data);
void wbsClient_readLoopData(void);
bool wbs_client_connect_success(void);
int wbsClient_set_reinit(bool reinit);


extern int lstagerun;
#endif
