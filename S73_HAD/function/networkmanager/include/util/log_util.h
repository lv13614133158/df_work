//
// Created by wangchunlong1 on 2019/8/14.
//

#ifndef CURLHTTPCLIENT_LOG_UTIL_H
#define CURLHTTPCLIENT_LOG_UTIL_H

#define TAG "BaseUploadNetwork"

#ifdef ANDROID
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG,__VA_ARGS__)
#endif

#ifndef ANDROID
#include <stdio.h>
#define LOGV(fmt, ...) printf(fmt,  ##__VA_ARGS__);\
	printf("\n")

#define LOGD(fmt, ...) printf(fmt,  ##__VA_ARGS__);\
	printf("\n")

#define LOGI(fmt, ...) printf(fmt,  ##__VA_ARGS__);\
	printf("\n")

#define LOGW(fmt, ...) printf(fmt,  ##__VA_ARGS__);\
	printf("\n")

#define LOGE(fmt, ...) printf(fmt,  ##__VA_ARGS__);\
	printf("\n")

#endif

#endif //CURLHTTPCLIENT_LOG_UTIL_H
