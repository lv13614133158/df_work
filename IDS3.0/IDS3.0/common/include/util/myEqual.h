/*
 * @Author: your name
 * @Date: 2020-06-23 11:27:53
 * @LastEditTime: 2020-06-24 08:18:46
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /idps_frame/includes/common/util/myEqual.h
 */ 
/*************************
*** File myEqual.h
**************************/
#ifndef MYEQUAL_H_INCLUDED
#define MYEQUAL_H_INCLUDED
 
//默认的相等的方法
int myEqualDefault(void * a, void *b);
 
//int类型相等的方法
int myEqualInt(void * a, void *b);
 
//char类型相等的方法
int myEqualChar(void * a, void *b);
 
//string类型相等的方法
int myEqualString(void * a, void *b);
 
#endif // MYEQUAL_H_INCLUDED