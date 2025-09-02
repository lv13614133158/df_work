/*
 * @Author: your name
 * @Date: 2021-08-05 06:05:14
 * @LastEditTime: 2021-08-09 06:24:21
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/websocket/websocketmanager.h
 */
#ifndef __WEBSOCKETMANAGER_H
#define __WEBSOCKETMANAGER_H
#ifdef __cplusplus
extern "C" {
#endif
#include "idps_main.h"

typedef struct _websocketMangerMethod{
   void (*initWebsocket)(websocketInfoModult_t*);
   void (*startWebsocket)(void);
   void (*stopWebsocket)(void);
   void (*sendEventData)(char*,char*,char*);
   void (*sendInfoData)(char*);
   void (*reinitWebsocketConnect)(bool);
}websocketMangerMethod;
extern websocketMangerMethod  websocketMangerMethodobj;

#ifdef __cplusplus
}
#endif
#endif