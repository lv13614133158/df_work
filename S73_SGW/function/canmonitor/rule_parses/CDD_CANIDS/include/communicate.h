#ifndef __COMMUNICATE_H__
#define __COMMUNICATE_H__

#include "platformtypes.h"
#include "msg_config.h"

//通信报文最大长度
#define SK_COMM_BUF_MAXSIZE 255
//队列最大长度
#define SK_MSG_QUEUE_SIZE 200

typedef struct {
    uint8 data[SK_MSG_QUEUE_SIZE][SK_COMM_BUF_MAXSIZE];
    uint8 front, rear;
} MsgArrayQueue;

uint8 initCommunicate(uint8 (*pSendMsg)(uint8* data, uint16 len) );


// init queue
static void initMsgQueue();
// push queue
uint8 pushMsgQueue(uint8 *data, uint8 len);
//
uint8 sendMsgFromQueue();

//字节序转换函数
uint32 reverseBytes_u32(uint32 x);
uint16 reverseBytes_u16(uint16 x);

/*
发送信息同步报文
 * @param subfunction 子功能码 0x01-0x07 
 * @param info 信息内容
 * @param infoLen 信息长度
 * @param data 返回报文数据
 * @param len 返回报文长度(最大长度248)
*/
uint16 sendCommInfoMsg(uint8 subfunction, uint8* info, uint8 infoLen);


/*
发送VSOC通信报文 (注册、协商和拉取配置)
 * @param subfunction 子功能代码 0x01-0x07、0xA1
 * @param info 信息内容
 * @param infoLen 信息长度
*/
uint16 sendCommVSocMsg(uint8 subfunction, uint8* info, uint8 infoLen);

/*
发送心跳报文
 * @param subfunction 子功能代码 0x01
*/
uint16 sendCommHeartMsg(uint8 subfunction);


/*
发送日志报文
 * @param subfunction 子功能码 0x01-0x02
 * @param info 信息内容
 * @param infoLen 信息长度
 * @param saveflag 保存标志，用于指示是否需要保存该日志信息
*/
uint16 sendCommLogMsg(uint8 subfunction, uint8* info, uint8 infoLen, uint8 saveflag);


/////////////////////////////////////////////////
/////////////////////////////////////////////////

/*
发送流量统计报文
 * @param netID 通道
 * @param loadRate 当前的负载率
 * @param loadRateThreshold 负载率的阈值
 * @param flowCnt 统计到的流量数量
 * @param flowMin 流量的最小值
 * @param flowMax 流量的最大值
*/
uint16 sendFlowCheckEventMsg(uint8 netID, float loadRate, float loadRateThreshold, uint32 flowCnt, uint32 flowMin, uint32 flowMax);

/*
发送白名单报文
 * @param netID 通道
 * @param canID CAN总线ID
*/
uint16 sendListCheckEventMsg(uint8 netID, uint32 canID);

/*
发送周期丢失报文
 * @param netID 通道
 * @param canID CAN总线ID
 * @param period 异常的周期时间
 * @param periodThreshold 正常的周期时间
 * @param offset 允许的偏移时间
*/
uint16 sendPeriodCheckEventMsg(uint8 netID, uint32 canID, uint32 period, uint32 periodThreshold, uint32 offset);

/*
发送长度异常报文
 * @param netID 通道
 * @param canID CAN总线ID
 * @param length CAN帧的数据长度
 * @param lengthThreshold 长度阈值
*/
uint16 sendLengthCheckEventMsg(uint8 netID, uint32 canID, uint32 length, uint32 lengthThreshold);

/*
发送信号分析阈值异常报文
 * @param netID 通道
 * @param canID CAN总线ID
 * @param signalScale 信号转化率
 * @param signalValue 信号当前值
 * @param signalValueMin 信号最小值
 * @param signalValueMax 信号最大值
 * @param signalData 异常报文
*/
uint16 sendSignalThresholdEventMsg(uint8 netID, uint32 canID, uint32 signalScale, uint32 signalValue, uint32 signalValueMin, uint32 signalValueMax, uint8* signalData);

/*
发送信号分析枚举异常报文
 * @param netID 通道
 * @param canID CAN总线ID
 * @param signalValue 信号当前值
 * @param signalData 异常报文
*/
uint16 sendSignalEnumEventMsg(uint8 netID, uint32 canID, uint32 signalValue, uint8* signalData);

/*
发送信号分析变化率异常报文
 * @param netID 通道
 * @param canID CAN总线ID
 * @param signalChangeRate 信号异常变化率
 * @param signalRateMin 信号变化率最小值
 * @param signalRateMax 信号变化率最大值
 * @param signalData 异常报文
*/
uint16 sendSignalChangeRateEventMsg(uint8 netID, uint32 canID, uint32 signalChangeRate, uint32 signalRateMin, uint32 signalRateMax, uint8* signalData);

/*
发送信号分析信号计数跟踪异常报文
 * @param netID 通道
 * @param canID CAN总线ID
 * @param signalScale 信号转化率
 * @param signalValue 当前信号值
 * @param signalLastValue 信号上一次的值
 * @param signalStep 信号变化步长阈值
 * @param signalData 异常报文
*/
uint16 sendSignalCountEventMsg(uint8 netID, uint32 canID, uint32 signalScale, uint32 signalValue, uint32 signalLastValue, uint32 signalStep, uint8* signalData);


/*
发送信号分析信号状态跟踪异常报文
 * @param netID 通道
 * @param canID CAN总线ID
 * @param signalScale 信号转化率
 * @param signalValue 当前信号值
 * @param signalLastValue 信号上一次的值
 * @param signalChangeMin 信号变化最小值
 * @param signalChangeMax 信号变化最大值
 * @param signalData 异常报文
*/
uint16 sendSignalStateEventMsg(uint8 netID, uint32 canID, uint32 signalScale, uint32 signalValue, uint32 signalLastValue, uint32 signalChangeMin, uint32 signalChangeMax, uint8* signalData);


/*
发送信号分析信号关联性异常报文
 * @param netID 通道
 * @param canID_1 第一个信号的CAN ID
 * @param signalScale_1 第一个信号的转化率
 * @param signalValue_1 第一个信号的当前值
 * @param signalValueMin_1 第一个信号的最小值
 * @param signalValueMax_1 第一个信号的最大值
 * @param signalData_1 第一个信号报文
 * @param canID_2 第二个信号的CAN ID
 * @param signalScale_2 第二个信号的转化率
 * @param signalValue_2 第二个信号的当前值
 * @param signalValueMin_2 第二个信号的最小值
 * @param signalValueMax_2 第二个信号的最大值
 * @param signalData_2 第二个信号报文
*/
uint16 sendSignalRelevanceEventMsg(uint8 netID, uint32 canID_1, uint32 signalScale_1, uint32 signalValue_1, uint32 signalValueMin_1, uint32 signalValueMax_1, uint8* signalData_1, 
    uint32 canID_2, uint32 signalScale_2, uint32 signalValue_2, uint32 signalValueMin_2, uint32 signalValueMax_2, uint8* signalData_2);


/*
发送诊断会话和ECU复位监测异常
 * @param netID 通道
 * @param diagType 诊断类型 01:诊断会话 02:ECU复位
 * @param canID CAN总线ID
 * @param subType 会话/复位类型
*/
uint16 sendSessionAndResetEventMsg(uint8 netID, uint8 diagType, uint32 canID, uint8 subType);

/*
发送安全访问监测异常
 * @param netID 通道
 * @param canID CAN总线ID
 * @param accessNum 访问次数
*/
uint16 sendSafetyAccessEventMsg(uint8 netID, uint32 canID, uint32 accessNum);



#endif //__COMMUNICATE_H__


