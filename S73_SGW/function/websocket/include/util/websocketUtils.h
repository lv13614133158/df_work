#ifndef  __WEBSOCKETUTIL_H_
#define  __WEBSOCKETUTIL_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "util.h"

enum webProcessType{
    RPC = 1,
    NORMAL,
    OTHERS
};

enum webProcessType wbsDispatchData(char *_iflowdata);
char *wbsGetformEvent(char *data, char *postid, long long *plSeqNumber);
char *wbsGetformInfo(char *data, long long *plSeqNumber);
char *wbsGetHeartBeat(long long *plSeqNumber);
char *wbsGetRpcAck(long long lSeqNumber);
char *wbsGetRpcResp(char *data, long long* plSeqNumber);
char* wbsGetformIDPSErr(long long* plSeqNumber);

#ifdef __cplusplus
}
#endif 
#endif