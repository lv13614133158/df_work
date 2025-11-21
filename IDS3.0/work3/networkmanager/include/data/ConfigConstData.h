/*
*   Created by xuewenliang on 2020/1/2.
*/
#ifndef NATIVEDEMO_CONFIGCONSTDATA_H
#define NATIVEDEMO_CONFIGCONSTDATA_H
#ifdef __cplusplus
extern "C"{
#endif
#include <stdbool.h>

bool getlowerpower(void);
void setlowerpower(bool status);
void setChannelId(const char *channelId);
void setBaseUrl(const char *baseUrl);
void setEquipmentType(const char *equipmentType); 
void setUdid(const char *udid);
void starthread();
void stopthread();

char *getChannelId();
char *getEquipmentType();
int getConsumeTPSize();
void setConsumeTPSize(int _consumeTPSize);
int getProducerTPSize();
void setProducerTPSize(int _producerTPSize);
char *getUdid();
char *getBaseUrl();

#ifdef __cplusplus
}
#endif
#endif //NATIVEDEMO_CONFIGCONSTDATA_H
