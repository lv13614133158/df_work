/*
 * @Author: your name
 * @Date: 2021-08-06 02:32:00
 * @LastEditTime: 2021-08-08 21:35:58
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/websocket/websocketrpc.h
 */
#ifndef __WEBSOCKETRPCC_H
#define __WEBSOCKETRPCC_H
#include "util.h"

/**
 * @description: param info
 * @param {*}
 * @return {*}
 */
typedef struct _rpctypedes{
	char* m_type;//space par
	int   m_typeid;
	char* m_name;//space par
	int   m_nameid;
    char* m_content;
	//default par
	/***
	 * @prief
	 * 
	 * ***/
}rpctypedes;
/**
 * @description:normal struct
 * @param 
 * @return {*void}
 */
typedef struct _functionlist{
	char  m_functiondes[64];
	int   m_functionid;
    int   m_paranumber;
	int   m_funcptr;//map position function
	list *m_ptr;//functions param
}functionlist;
/**
 * @description:normal struct
 * @param 
 * @return {*void}
 */
typedef  struct _packdesinfo{
	char  			m_functionowner[64];
	int   			m_functionpackid;
	list*           m_ptr;
}packdesinfo;

char wbsRpcProcess(char* _idata);

#endif 