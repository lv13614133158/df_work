/*
 * @Author: your name
 * @Date: 2021-05-21 11:01:48
 * @LastEditTime: 2021-05-21 13:00:47
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \idps_networkmanager\idps_networkmanager\idps_networkmanager\src\includes\util\postHeaderUtil.h
 */

#ifndef __POSTHEADERUTIL_H
#define __POSTHEADERUTIL_H
#ifdef __cplusplus
extern "C"{
#endif
char *getHeaders(const char *encyptData, int encyptDataLen,void* _curl);
char *getHeadersForHardware(const char *encyptData, int encyptDataLen,void* _curl);
char *getFileHeaders(void* _curl); 
char *getManageKeyHeaders();
char *getSessionKeyHeaders(const char *encyptData, int encyptDataLen,void* _curl);
char *getGetRequestHeaders(void* _curl);
char *getManageKeyBoys();
char *getSessionKeyBoys(void* _curl);
#ifdef __cplusplus
}
#endif
#endif //__POSTHEADERUTIL_H