/*
 * @Author: your name
 * @Date: 2020-06-23 11:26:07
 * @LastEditTime: 2020-06-23 11:27:11
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/src/common/util/myHashCode.C
 */ 
#include "myHashCode.h"
 
//默认的hashCode
long myHashCodeDefault(void * a)
{
    return (long) a;
}
 
//int类型hashCode
int myHashCodeInt(void * a)
{
    int * aa = (int *) a;
    return *aa;
}
 
//char类型的hashCode
int myHashCodeChar(void * a)
{
    char *aa = (char *) a;
    return *aa;
}
 
//string类型的hashCode
int myHashCodeString(void * a)
{
    int re = 0;
    char *aa = (char *) a;
    while (*aa)
    {
        re += HASHCODE_MULT * *aa;
        aa++;
    }
    return re;

}