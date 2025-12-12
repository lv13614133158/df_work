#ifndef __DISPATCHMANAGER_H
#define __DISPATCHMANAGER_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C"{
#endif
typedef struct EventWrapper{
    char *policy_id;
    char *data;
    char *ticket_id;
    char *error_msg;
    char *attachment_id;
    char *path;
}eventWrapper_t;

typedef struct RequestWrapper{
    bool withAttachment;
    int index;
    long long id;
    char *url;
    char *headers;
    char *data;
    int datalen;
    eventWrapper_t *eventWrapper;

}requestWrapper_t;


//static void dispatch();
void consumeContinuously();
void produceGatherDataRequest(const char *url, const char *data, int priority,void* _curl);
void produceEventDataRequest(const char *policy_id, const char *data, const char *ticket_id, int priority,void* _curl);
void produceEventDataRequestWithAttachment(const char *policy_id, const char *data, const char *ticket_id, const char *path, int priority);
void produceEventDataRequestAfterUploadAttachment(const char *policy_id, const char *data, const char *ticket_id, const char* attachment_id, const char* error_msg, int priority,void* _url);

void produceHeartbeatRequest(const char *url,void* _curl);
void Restart_UpdateTable(void);
void stopDispatchManager();
void startDispatchManager();
void produceAllRequestFromDb(void *nullarg,void* curl);
void consumeRequest(void *pdata,void* curl);
#ifdef __cplusplus
}
#endif
#endif //__DISPATCHMANAGER_H
