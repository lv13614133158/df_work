/*
 * @Author: your name
 * @Date: 2021-07-29 04:33:26
 * @LastEditTime: 2021-08-08 22:13:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/websocket/websocketclient.h
 */
#ifndef __WEBSOCKETLOOP_H_
#define __WEBSOCKETLOOP_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdbool.h>
#include "util.h"

typedef struct _wbsmap_elmt 
{
    long long key;
    bool used;
    int  len;
    void *data;
    bool send_only_once;
    bool ready_retry_send;
}map_elmt;

// 可以容纳map对个数
#define WBSMAP_MAX  (1000)

// 添加map对
int wbsAddMap(long long key, char *data, bool send_only_once);
// 删除map对
int wbsDelMap(long long key);
// 获取map队个数
int wbsGetSize();
// 按pos查找map
int wbsGetMap(int index, map_elmt *map_buff);
// map内容读取到list中
void wbsGetList(list *listTx);
// 清除现有map内容
void wbsClearMap();
// 打印map内容
void wbsPrintMap();
//设置map的重传标志位
int wbsSetRetrySend(void);

#ifdef __cplusplus
}
#endif
#endif