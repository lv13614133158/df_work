 #ifndef __API_NETWORKCLIENT_H_
 #define __API_NETWORKCLIENT_H_
 #ifdef __cplusplus
extern "C"
{
#endif 
#include <stdio.h>
/*
 * function:getSessionKey
 * input   :void
 * output  :char*
 * decla   :
 * modify  :20210906
 */
char* getSessionKeyApi(void);
/*
 * function:getManageKeyApi
 * input   :void
 * output  :char*
 * decla   :
 * modify  :20210906
 */
char* getManageKeyApi(void);
/*
 * function:UploadFileSetTime
 * input   :long
 * output  :void
 * decla   :
 */
void UploadFileSetTime(long _stamp);
/*
 * function:UploadFileSetTime
 * input   :long
 * output  :void
 * decla   :
 */
long UploadFileGetTime(void);
 /*
+ * function:进入低功耗模式
+ * input   :void
+ * output  :void
+ * decla   :20201111
+ */
void lowerpower_enter(void);
 /*
+ * function:退出低功耗模式
+ * input   :void
+ * output  :void
+ * decla   :20201111
+ */
void lowerpower_exit(void);
 /*
+ * function:查询当前的低功耗状态
+ * input   :void
+ * output  :void
+ * decla   :为了与上层兼容 1：低功耗状态下   0：非低功耗状态下
+ */
unsigned char lowerpower_states(void);
/*
 * function: getRequestData
 * input   :（1）_url:如"/api/v1/file_monitor_list"
            （2）_responsedata:待传数据指针
            （3）_length      :返回数据长度
             (5) _responsedatalen:response data区域大小
 * output  :void
 * decla   :(0)和getRequestData功能一致
 */
void getRequestDatanodate(char* _url,char** _responsedata,int* _length,int _responsedatalen);
 /*
 * function:initALLByMaster
 * input   :idpsdb:/usrdata/idps.db
 * output  :dir   :/usrdata
 * decla   :(0)基本的初始化
 *          (1)工作目录的设置
 *          (2)数据库的基本创建（仅仅指调用Open()API）
 */
void initALLByMaster(char* certsPath,char* dir);
/*
 * function:initTPSize
 * input   :_consumerTPSize:sonsumer size _producerTPSize:producersize
 * output  :void
 * decla   :(0)基本线程池参数的设置
 *          (1)消费者线程池，目前没有启用，目前是预留接口，设置1即可，不会对当前存在影响
 *          (2)生产者线程池，设置大于0的数值，目前建议设置1即可，数值越大内存占用越大
 */
void initTPSize(int _consumerTPSize,int _producerTPSize);
/*
 * function:setServerConfig
 * input   :_baseUrl:url
 * output  :void
 * decla   :(0)如function所示，就是设置baseurl,这里设置的是http://qa.service.car.360.cn
 *          (1)对指针变量，SO API不会做任何的操作,如是statck space,请注意空间的释放
 */
void setServerConfig(char* _baseUrl);
/*
 * function:setDeviceConfig
 * input   :_jsonstring:json string
 * output  :void
 * decla   :(0) 
 *          (1)设置uuid
 *          (2)设置channelId
 *          (3)设置equipmentType
 */
void setDeviceConfig(char* sn,char* channel_id,char* equipment_type);
/*
 * function:setManageKeyStore
 * input   :_mode:模式
 * output  :void
 * decla   :(0)设施key_iv的模式（目前设置1即可）
 */
void setManageKeyStore(int  _mode);
/*
 * function:postGatherData
 * input   :（1）_url： 基本url类型，如"/api/v2/service_monitor"
            （2）_data: 发送的数据串，如"[{\"dns\":\"dnstest\",\"ips\":[\"192.168.21.2\"]}]"
            （3）_priority:数据串的优先级，目前未使用，统一写1即可
 * output  :void
 * decla   :(0)发送数据，由于该function会线程池里压数据，所以无返回值
 *          (1)对指针变量，SO API不会做任何的操作
 */
void postGatherData(char* _url,char* _data,int _priority);
/*
 * function:postEventData
 * input   :（1）_policy_id:策略ID
            （2）_data:待发送的数据
            （3）_ticket_id:ticket_id
            （4）_priority:优先级

 * output  :void
 * decla   :(0)发送数据，由于该function会线程池里压数据，所以无返回值
 *          (1)对指针变量，SO API不会做任何的操作
 */
void postEventData(const char* _policy_id,char* _data,char* _ticket_id,int _priority);
/*
 * function:postEventDataWithAttachment(目前未使用)
 * input   :（1）_policy_id:策略ID
            （2）_data:待发送的数据
            （3）_ticket_id:ticket_id
            （4）_priority:优先级
            （5）_path    :路径（如"/usrdata/1.txt"）

 * output  :void
 * decla   :(0)带附属文件的发送
 *          
 */
void postEventDataWithAttachment (const char *_policy_id, const char *_data, const char *_ticket_id, const char *_path, int _priority);
/*
 * function:start
 * input   :void
 * output  :void
 * decla   :(0)启动生产者线程池，必须要启动，负责一些数据无法发送   
 */
void start(void);
/*
 * function:postHeartbeatData
 * input   :（1）_url:j
 * output  :void
 * decla   :(0)主要目的是发送心跳数据帧，属于非同步发送（即往线程池里填充的方式）（目前未使用，使用时需要测试）   
 */
void postHeartbeatData(char* _url);
/*
 * function:stop
 * input   :void
 * output  :void
 * decla   :(0)做必要的线程池资源释放
 */
void stop(void);
/*
 * function:startReRequestFromDB
 * input   :void
 * output  :void
 * decla   :(0)一些数据因为网络或者其他原因未发送成功保存到数据库，
 * 此function的目的是读数据库进行数据重新发送，返回true时表明启动
 * 成功，返回false表明当前网络忙，稍后再试。
 */
void startReRequestFromDB(void);
/*
 * function:registernet
 * input   :void
 * output  :void
 * decla   :(0)基本注册，主要包括获取sn，获取managekey操作
 */
int registernet(void);
/*
 * function:getSn
 * input   :void
 * output  :void
 * decla   :(0)获取当前SN
 */
char* getSn(void);
char* getToken(void);
/*
 * function:getRequestData
 * input   :（1）_url:如"/api/v1/file_monitor_list"
            （2）_updateTime:如1584952302124
            （3）待传数据指针
            （4）返回数据长度
             (5) _responsedatalen:response data区域大小
 * output  :void
 * decla   :(0)获取数据
 * modify  :20200609
 */
void getRequestData(char* _url,long long _updateTime,char** _responsedata,int* _length,int _responsedatalen);
/*
 * function:postHeartbeatDataSync
  * input   :（1）_url:基本url
             （2）_responseData:待传数据指针
             （3）inputlength  :返回数据长度
              (5) _responsedatalen:response data区域大小
 * output  :void
 * decla   :(0)发送心跳数据，直接发送
 */
void postHeartbeatDataSync(char* _url,char** _responseData,int *inputlength,int _responsedatalen);
#ifdef __cplusplus
}
#endif
 #endif 
