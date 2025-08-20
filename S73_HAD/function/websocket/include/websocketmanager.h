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
   void (*sendEventData)(char*,char*);
   void (*sendInfoData)(char*);
   void (*initNetConnect)(void*,void*);
}websocketMangerMethod;
extern websocketMangerMethod  websocketMangerMethodobj;

#define TERMINAL_TYPE_MARK  "24" //11:ivi;22:tbox;23:sgw;24;had;

#define EVENT_TYPE_FILE_CHANGED             "00101010001"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_FLOW             "00101080001"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_ATTACK           "00101090001"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_DNS_INQUIRE      "00101020004"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_DNS_RESPONSE     "00101020005"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_PORT_OPEN        "00101020006"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_IP_CONNECT       "00101020001"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_TCP_CONNECT      "00101020002"TERMINAL_TYPE_MARK
#define EVENT_TYPE_NETWORK_UDP_CONNECT      "00101020003"TERMINAL_TYPE_MARK
#define EVENT_TYPE_USER_LOGIN               "00101050001"TERMINAL_TYPE_MARK
#define EVENT_TYPE_PROCESS_CHANGED          "00101030001"TERMINAL_TYPE_MARK
#define EVENT_TYPE_CPU_RESOURCE_USAGE       "00101060002"TERMINAL_TYPE_MARK
#define EVENT_TYPE_RAM_RESOURCE_USAGE       "00101060003"TERMINAL_TYPE_MARK
#define EVENT_TYPE_ROM_RESOURCE_USAGE       "00101060004"TERMINAL_TYPE_MARK
#define EVENT_TYPE_SNAPSHOT_RESOURCE_USAGE  "00101060001"TERMINAL_TYPE_MARK

#ifdef __cplusplus
}
#endif
#endif
