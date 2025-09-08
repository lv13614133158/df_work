/*
*   Created by xuewenliang on 2020/1/2.
*/

#ifndef __TOOL_H
#define __TOOL_H
#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>
#include <stdbool.h>
#define STR_LEN 8//定义随机输出的字符串长度。
#define KEY_IV_LENGTH 16
 
void GenerateStr(char* str); 
char *getHmacFromData(char *key, uint8_t* data, int dataLen);
char *getUrlWithUpdateTime(char *url,const long long updateTime,char *sn);
#ifdef __cplusplus
}
#endif
#endif