
#ifndef CURLHTTPCLIENT_CJSONUTILS_H
#define CURLHTTPCLIENT_CJSONUTILS_H
#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>

char *getinfofromCJsonstr(const char* response,char* getstr);
char* convertFromJson(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOServiceMonitor(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTONetworkMonitor(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTONetFlowNodel(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTODeviceInfo(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOTBoxHardware(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOAvgModel(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOAvgAndMemory(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOAppInfoList(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOUsageStatsList(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOHookAPIWhiteList(const char* sn, const char* data,uint32_t *length);
char* convertFromJsonTOHookAPIBlackList(const char* sn, const char* data,uint32_t *length);
char* changeToGetResponseData(parseResponseData_t *httpResponseData,int* _length);
#ifdef __cplusplus
}
#endif
#endif //CURLHTTPCLIENT_CJSONUTILS_H
