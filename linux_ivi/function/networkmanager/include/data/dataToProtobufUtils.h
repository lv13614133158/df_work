#ifndef __DATATOPROTOBUFUTILS_H
#define __DATATOPROTOBUFUTILS_H
#include <stdint.h>
#include "data/FileWrapper.h"
#ifdef __cplusplus
extern "C"{
#endif
char *getProtobufFromGaterData(const char* url, const char* body,uint32_t *len,void* _curl);
char *getAlarmEventProtobufFromGaterData(const char *policy_id, const char *data, const char *ticket_id, const char *error_msg, const char* attachment_id,int* retlength,void* _curl);
char *getFileModelProtobufFromFileData(const FileWrapper_t *fileWrapper,void* _curl);

char *getProtobufForManageKey();
char *getProtobufForSessionKey(void* _curl);
char *getProtobufForHeartbeat(void* _curl);
#ifdef __cplusplus
}
#endif

#endif //__DATATOPROTOBUFUTILS_H
