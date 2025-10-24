#ifndef __ENCYPTREQUESTWRAPPER_H
#define __ENCYPTREQUESTWRAPPER_H

#include "DispatchManager.h"
#ifdef __cplusplus
extern "C"{
#endif
typedef struct EncyptRequestWrapper{
    int dataLen;
    char *url;
    char *headers;
    char *data;
}EncyptRequestWrapper_t;

EncyptRequestWrapper_t *EncyptRequestWrapper(requestWrapper_t *requestWrapper,void* _curl);
#ifdef __cplusplus
}
#endif
#endif //__ENCYPTREQUESTWRAPPER_H