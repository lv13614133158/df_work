/*
 * @Author: your name
 * @Date: 2020-06-23 11:20:08
 * @LastEditTime: 2020-06-23 11:20:22
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/common/util/myHashCode.h
 */ 
/*************************
*** File myHashCode.h
**************************/
#ifndef MYHASHCODE_H_INCLUDED
#define MYHASHCODE_H_INCLUDED
 
#include <string.h>
 
#define HASHCODE_MULT 31
 
//默认的hashCode
long myHashCodeDefault(void * a);
 
//int类型hashCode
int myHashCodeInt(void * a);
 
//char类型的hashCode
int myHashCodeChar(void * a);
 
//string类型的hashCode
int myHashCodeString(void * a);
 
#endif // MYHASHCODE_H_INCLUDED