/*
 * @Descripttion: 
 * @version: V0.0
 * @Author: qihoo360
 * @Date: 1969-12-31 19:00:00
 * @LastEditors: 
 * @LastEditTime: 2020-06-02 23:39:15
 */ 
#ifndef __CRYPTO_H
#define __CRYPTO_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cryptogram.h"
/*
*   密钥存储格式
*   flag：判断密钥是否失效，注册和删除时改变此标志
*        true：有效
*        flase：失效，该结构体可以被覆盖
*   mode：存储密钥的模式
*       1：存文件   2：存白盒  3:存SE
*   data：待存储字符串的hash值字符串
*   token：密钥存se时有效，为注册密钥返回的token值
*   data：key&iv字符串，如果mode==1，则data存储的是key&iv的字符串
*         如果mode==3，则data只存储iv向量，加密时将token和iv向量
*          一起传入api
*/

typedef struct cryptoNode{
    bool flag;
    int mode;
    unsigned char index[33];
    int token;
    unsigned char data[255];
}cryptoNode_t;

int  setWorkDirectory(const char *dir);

bool storeKey(int mode,const char *data,char *outbuf);

bool deleteKey(char *index);

cypher_t *encryption(cypher_t *data_in,char *index);

cypher_t *decryption(cypher_t *data_in,char *index);


//测试接口
int readFile();
#endif