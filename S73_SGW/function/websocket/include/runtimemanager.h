/*
 * @Author: your name
 * @Date: 2023-01-09 010:33:26
 * @LastEditTime: 2021-08-08 22:13:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: 
 */
#ifndef __RUNTIMEMANAGER_H_
#define __RUNTIMEMANAGER_H_

// int:0x7fffffff/60/60/24/365 = 68年
// 使用int也完全够用，这里兼容统一使用long long
// 先initRunTime才可以继续记时，stopRunTime每次休眠需要调用，恢复后initRunTime继续
// setRunTime自定义记时时间，用于特殊情况修正记时时间
void initRunTime();
void stopRunTime();
void setRunTime(long long timeValue);
long long getRunTime();
long long getOSTime();

#endif
