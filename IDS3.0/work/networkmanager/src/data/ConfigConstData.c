#include <stdio.h>
#include <string.h>
#include "data/ConfigConstData.h"
#include "ThreadPool.h"

struct ConfigConstData{
    char equipmentType[64];
    char channelId[64];
    /**
     * 唯一设备识别码
     * */
    char udid[128];
    char baseUrl[256];
    int consumeTPSize;
    int producerTPSize;
};

threadpool_t *consumerPtr = NULL,*producerPtr = NULL;
static struct ConfigConstData configData={{0},{0},{0},{0},1,1};
/**
 * @decla:低功耗状态标志位 false：正常状态 true：低功耗状态
 */ 
static bool lowerpowerstatus = false;
/**
 * @decla:查询当前状态
 */ 
bool getlowerpower(void){
    return lowerpowerstatus;
}
/**
 * @decla:设置低功耗状态
 */ 
void setlowerpower(bool status){
    lowerpowerstatus = status;
}
/**
 * @decla:setEquipmentType
 */ 
void setEquipmentType(const char *equipmentType)
{
    if(equipmentType != NULL)
        memcpy(configData.equipmentType,equipmentType,(strlen(equipmentType) > sizeof(configData.equipmentType))?(sizeof(configData.equipmentType)):(strlen(equipmentType))); 
}
 
/**
 * @decla:setChannelId
 */ 
void setChannelId(const char *channelId)
{
    if(channelId != NULL)
        memcpy(configData.channelId,channelId,(strlen(channelId) > sizeof(configData.channelId))?(sizeof(configData.channelId)):(strlen(channelId)));
}
 
/**
 * @decla:setBaseUrl
 */ 
void setBaseUrl(const char *baseUrl)
{
    if(baseUrl != NULL)
        memcpy(configData.baseUrl,baseUrl,(strlen(baseUrl) > sizeof(configData.baseUrl))?(sizeof(configData.baseUrl)):(strlen(baseUrl)));
}
  
/**
 * @decla:set uuid
 */ 
void setUdid(const char *udid)
{
    if(udid != NULL)
        memcpy(configData.udid,udid,(strlen(udid) > sizeof(configData.udid))?(sizeof(configData.udid)):(strlen(udid)));
}
/**
 * @decla:start thread pool
 * @reason:configData is a static param
 */ 
void starthread(){
    producerPtr = threadpool_create(MINTHREADNUMBER,configData.producerTPSize,QUEUEMAXSIZE);
	//consumerPtr = threadpool_create(MINTHREADNUMBER,configData.consumeTPSize,QUEUEMAXSIZE);
}
/**
 * @decla:stop thread pool
 * @reason:configData is a static param
 */ 
void stopthread(){
    threadpool_destroy(producerPtr);
	//threadpool_destroy(consumerPtr);
}

char *getChannelId()
{
	return configData.channelId;
}

char *getEquipmentType()
{
	return configData.equipmentType;
}

int getConsumeTPSize()
{
	return configData.consumeTPSize;
}

void setConsumeTPSize(int _consumeTPSize)
{
	configData.consumeTPSize = _consumeTPSize;
	return;
}

int getProducerTPSize()
{
	return configData.producerTPSize;
}

void setProducerTPSize(int _producerTPSize)
{
	configData.producerTPSize = _producerTPSize;
}

char *getUdid()
{
	return configData.udid;
}

char *getBaseUrl()
{
	return configData.baseUrl;
}
