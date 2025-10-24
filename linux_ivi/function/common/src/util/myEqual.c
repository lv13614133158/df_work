/*
 * @Author: your name
 * @Date: 2020-06-23 11:28:54
 * @LastEditTime: 2020-06-23 11:29:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/src/common/util/myEqual.c
 */ 

#include "myEqual.h"
#include <string.h>
 
//默认的相等的方法
int myEqualDefault(void * a, void *b)
{
    return a == b;
}
 
//int类型相等的方法
int myEqualInt(void * a, void *b)
{
    int *aa = (int*) a;
    int *bb = (int *) b;
    return *aa == *bb;
}
 
//char类型相等的方法
int myEqualChar(void * a, void *b)
{
    char *aa = (char *) a;
    char *bb = (char *) b;
    return *aa = *bb;
}
 
//string类型相等的方法
int myEqualString(void * a, void *b)
{
    char *aa = (char *) a;
    char *bb = (char *) b;
    return strcmp(aa, bb)==0;
}