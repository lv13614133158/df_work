#include "communicate.h"
#include "ctimer.h"

static uint8 (*pTpSendMsg)(uint8* data, uint16 len) = NULL;

uint32 reverseBytes_u32(uint32 x) {
    return ((x & 0xff000000) >> 24) |
           ((x & 0x00ff0000) >> 8)  |
           ((x & 0x0000ff00) << 8)  |
           ((x & 0x000000ff) << 24);
}

uint16 reverseBytes_u16(uint16 x) {
    return ((x & 0xff00) >> 8)  |
           ((x & 0x00ff) << 8);
}


static MsgArrayQueue msgQueue = 
{
    .data = {0},
    .front = 0,
    .rear = 0
};

uint8 initCommunicate(uint8 (*pSendMsg)(uint8* data, uint16 len) ) 
{
    pTpSendMsg = pSendMsg;
    initMsgQueue();
    return 1;
}

static void initMsgQueue()
{
    msgQueue.front = msgQueue.rear = 0;
    
}

uint8 pushMsgQueue(uint8 *data, uint8 len)
{
    if (len > SK_COMM_BUF_MAXSIZE)
    {
        return 0; // 返回0表示失败
    }
    
    if ((msgQueue.rear + 1) % SK_MSG_QUEUE_SIZE == msgQueue.front) { // 队列满
        return 0; // 返回0表示失败
    }
    memset(msgQueue.data[msgQueue.rear], 0, SK_COMM_BUF_MAXSIZE);
    memcpy(msgQueue.data[msgQueue.rear], data, len); // 复制字符串到队列
    msgQueue.data[msgQueue.rear][SK_MSG_QUEUE_SIZE - 1] = len;
    msgQueue.rear = (msgQueue.rear + 1) % SK_MSG_QUEUE_SIZE; // 更新队尾指针
    return 1; // 成功返回1
}

uint8 sendMsgFromQueue()
{
    if (msgQueue.front == msgQueue.rear) { // 队列空
        return 0;
    }
    if (Get_OS_Time().sysTimeL % 2000 != 0) // 每2000ms发送一次
    {
        return 0;
    }
    
    uint8 sendData[SK_COMM_BUF_MAXSIZE] = {0};
    memcpy(sendData, msgQueue.data[msgQueue.front], SK_COMM_BUF_MAXSIZE); // 复制并返回队首元素
    uint16 msgLen = msgQueue.data[msgQueue.front][SK_MSG_QUEUE_SIZE - 1];
    if (pTpSendMsg)
    {
        uint8 reg = pTpSendMsg(sendData, msgLen);
        if (0 == reg) //RTE_E_OK == 0
        {
            msgQueue.front = (msgQueue.front + 1) % SK_MSG_QUEUE_SIZE; // 更新队首指针
            return 1;
        }
    }
    
    return 0;    
}

uint16 sendCommInfoMsg(uint8 subfunction, uint8* info, uint8 infoLen)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;
    
    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_INFO;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    if(info == NULL || infoLen == 0)
    {
        return 0;
    }
    //报文长度
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    //报文内容
    memcpy(data + msgLen, info, infoLen);
    msgLen += infoLen;

    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendCommVSocMsg(uint8 subfunction, uint8* info, uint8 infoLen)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    if(info == NULL || infoLen == 0)
    {
        return 0;
    }
    //报文长度
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    //报文内容
    memcpy(data + msgLen, info, infoLen);
    msgLen += infoLen;
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

static uint16 heartNum = 0;
uint16 sendCommHeartMsg(uint8 subfunction)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_HEART;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    uint8 infoLen = 2;
    //报文长度
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    //报文内容
    if(SK_COMM_SUBFUNCTION_HEART_CYCLE == subfunction)
    {
        heartNum = reverseBytes_u16(heartNum);
        memcpy(data + msgLen, &heartNum, sizeof(heartNum));
        msgLen += sizeof(heartNum);
        if (heartNum >= 65535)
            heartNum = 0;
        else
            heartNum++;
    }
    
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}


uint16 sendCommLogMsg(uint8 subfunction, uint8* info, uint8 infoLen, uint8 saveflag)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);

    uint8 functionID = SK_COMM_FUNNCTION_LOG;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    if(info == NULL || infoLen == 0)
    {
        return 0;
    }
    uint8 newLen = infoLen + sizeof(saveflag);  //添加saveflag长度
    memcpy(data + msgLen, &newLen, sizeof(newLen));
    msgLen += sizeof(infoLen);
    memcpy(data + msgLen, &saveflag, sizeof(saveflag));
    msgLen += sizeof(saveflag);
    memcpy(data + msgLen, info, infoLen);
    msgLen += infoLen;

    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);
    
    pushMsgQueue(data, msgLen);
    return msgLen;
}


/////////////////////////////////////////////////
/////////////////////////////////////////////////

#define CAN_OPER_MODE 51   //0x33

uint16 sendFlowCheckEventMsg(uint8 netID, float loadRate, float loadRateThreshold, uint32 flowCnt, uint32 flowMin, uint32 flowMax)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 26;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | 负载率 | 负载率异常类型 | 负载率阈值 | 流量 | 流量异常类型 | 流量最小阈值 | 流量最大阈值 |
    | 1字节 | 2字节 | 1字节 | 4字节 | 1字节 | 4字节 | 4字节 | 1字节 | 4字节 | 4字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x0001;   //0001
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);

    // 负载率转化成整数
    uint32 intLoadRate = (uint32)(loadRate * 10000);
    intLoadRate = reverseBytes_u32(intLoadRate);
    memcpy(data + msgLen, &intLoadRate, sizeof(intLoadRate));
    msgLen += sizeof(intLoadRate);
    uint8 loadErrType = 0;
    if (loadRate > loadRateThreshold)
    {
        loadErrType = 1;
    }
    memcpy(data + msgLen, &loadErrType, sizeof(loadErrType));
    msgLen += sizeof(loadErrType);
    // 负载率阈值转化成整数
    uint32 intLoadRateThreshold = (uint32)(loadRateThreshold * 10000);
    intLoadRateThreshold = reverseBytes_u32(intLoadRateThreshold);
    memcpy(data + msgLen, &intLoadRateThreshold, sizeof(intLoadRateThreshold));
    msgLen += sizeof(intLoadRateThreshold);

    // 流量
    uint8 flowErrType = 0;
    if (flowCnt < flowMin)
    {
        flowErrType = 1;
    }
    else if (flowCnt > flowMax)
    {
        flowErrType = 2;
    }
    flowCnt = reverseBytes_u32(flowCnt);
    memcpy(data + msgLen, &flowCnt, sizeof(flowCnt));
    msgLen += sizeof(flowCnt);
    memcpy(data + msgLen, &flowErrType, sizeof(flowErrType));
    msgLen += sizeof(flowErrType);
    flowMin = reverseBytes_u32(flowMin);
    memcpy(data + msgLen, &flowMin, sizeof(flowMin));
    msgLen += sizeof(flowMin);
    flowMax = reverseBytes_u32(flowMax);
    memcpy(data + msgLen, &flowMax, sizeof(flowMax));
    msgLen += sizeof(flowMax); 
    

    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendListCheckEventMsg(uint8 netID, uint32 canID)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 8;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 
    | 1字节 | 2字节 | 1字节 | 4字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x0002;  //0002
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);
 
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendPeriodCheckEventMsg(uint8 netID, uint32 canID, uint32 period, uint32 periodThreshold, uint32 offset)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 20;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 当前周期 | 周期阈值 | 偏差值 | 
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 4字节 | 4字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x0003;  //0003
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    period = reverseBytes_u32(period);
    memcpy(data + msgLen, &period, sizeof(period));
    msgLen += sizeof(period);
    periodThreshold = reverseBytes_u32(periodThreshold);
    memcpy(data + msgLen, &periodThreshold, sizeof(periodThreshold));
    msgLen += sizeof(periodThreshold);
    offset = reverseBytes_u32(offset);
    memcpy(data + msgLen, &offset, sizeof(offset));
    msgLen += sizeof(offset); 
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendLengthCheckEventMsg(uint8 netID, uint32 canID, uint32 length, uint32 lengthThreshold)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 16;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 数据长度 | 长度阈值 |
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 4字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x0004;   //0004
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    length = reverseBytes_u32(length);
    memcpy(data + msgLen, &length, sizeof(length));
    msgLen += sizeof(length);
    lengthThreshold = reverseBytes_u32(lengthThreshold);
    memcpy(data + msgLen, &lengthThreshold, sizeof(lengthThreshold));
    msgLen += sizeof(lengthThreshold); 
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendSignalThresholdEventMsg(uint8 netID, uint32 canID, uint32 signalScale, uint32 signalValue, uint32 signalValueMin, uint32 signalValueMax, uint8* signalData)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 32;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 数据转换率 | 信号值 | 最小阈值 | 最大阈值 | 异常报文 | 
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 4字节 | 4字节 | 4字节 | 8字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x03E9;  //1001
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    signalScale = reverseBytes_u32(signalScale);
    memcpy(data + msgLen, &signalScale, sizeof(signalScale));
    msgLen += sizeof(signalScale);
    signalValue = reverseBytes_u32(signalValue);
    memcpy(data + msgLen, &signalValue, sizeof(signalValue));
    msgLen += sizeof(signalValue);
    signalValueMin = reverseBytes_u32(signalValueMin);
    memcpy(data + msgLen, &signalValueMin, sizeof(signalValueMin));
    msgLen += sizeof(signalValueMin);
    signalValueMax = reverseBytes_u32(signalValueMax);
    memcpy(data + msgLen, &signalValueMax, sizeof(signalValueMax));
    msgLen += sizeof(signalValueMax);
    memcpy(data + msgLen, signalData, 8);
    msgLen += 8; 
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendSignalEnumEventMsg(uint8 netID, uint32 canID, uint32 signalValue, uint8* signalData)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 20;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 枚举值 | 异常报文 | 
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 8字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x03EA;  //1002
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    signalValue = reverseBytes_u32(signalValue);
    memcpy(data + msgLen, &signalValue, sizeof(signalValue));
    msgLen += sizeof(signalValue);
    memcpy(data + msgLen, signalData, 8);
    msgLen += 8;
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendSignalChangeRateEventMsg(uint8 netID, uint32 canID, uint32 signalChangeRate, uint32 signalRateMin, uint32 signalRateMax, uint8* signalData)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 28;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 变化率 | 最小阈值 | 最大阈值 | 异常报文 | 
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 4字节 | 4字节 | 8字节 | 
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x03EB;  //1003
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    signalChangeRate = reverseBytes_u32(signalChangeRate);
    memcpy(data + msgLen, &signalChangeRate, sizeof(signalChangeRate));
    msgLen += sizeof(signalChangeRate);
    signalRateMin = reverseBytes_u32(signalRateMin);
    memcpy(data + msgLen, &signalRateMin, sizeof(signalRateMin));
    msgLen += sizeof(signalRateMin);
    signalRateMax = reverseBytes_u32(signalRateMax);
    memcpy(data + msgLen, &signalRateMax, sizeof(signalRateMax));
    msgLen += sizeof(signalRateMax);
    memcpy(data + msgLen, signalData, 8);
    msgLen += 8;
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}


uint16 sendSignalCountEventMsg(uint8 netID, uint32 canID, uint32 signalScale, uint32 signalValue, uint32 signalLastValue, uint32 signalStep, uint8* signalData)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 32;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 数据转换率 | 信号值 | 上一帧信号值 | 信号步长 | 异常报文 | 
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 4字节 | 4字节 | 4字节 | 8字节 | 
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x03EC;  //1004
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    signalScale = reverseBytes_u32(signalScale);
    memcpy(data + msgLen, &signalScale, sizeof(signalScale));
    msgLen += sizeof(signalScale);
    signalValue = reverseBytes_u32(signalValue);
    memcpy(data + msgLen, &signalValue, sizeof(signalValue));
    msgLen += sizeof(signalValue);
    signalLastValue = reverseBytes_u32(signalLastValue);
    memcpy(data + msgLen, &signalLastValue, sizeof(signalLastValue));
    msgLen += sizeof(signalLastValue);
    signalStep = reverseBytes_u32(signalStep);
    memcpy(data + msgLen, &signalStep, sizeof(signalStep));
    msgLen += sizeof(signalStep);
    memcpy(data + msgLen, signalData, 8);
    msgLen += 8;
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendSignalStateEventMsg(uint8 netID, uint32 canID, uint32 signalScale, uint32 signalValue, uint32 signalLastValue, uint32 signalChangeMin, uint32 signalChangeMax, uint8* signalData)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 36;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 数据转换率 | 信号值 | 上一帧信号值 | 变化下限 | 变化上限 | 异常报文 | 
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 4字节 | 4字节 | 4字节 | 4字节 | 8字节 | 
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x03ED;  //1005
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);
    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    signalScale = reverseBytes_u32(signalScale);
    memcpy(data + msgLen, &signalScale, sizeof(signalScale));
    msgLen += sizeof(signalScale);
    signalValue = reverseBytes_u32(signalValue);
    memcpy(data + msgLen, &signalValue, sizeof(signalValue));
    msgLen += sizeof(signalValue);
    signalLastValue = reverseBytes_u32(signalLastValue);
    memcpy(data + msgLen, &signalLastValue, sizeof(signalLastValue));
    msgLen += sizeof(signalLastValue);
    signalChangeMin = reverseBytes_u32(signalChangeMin);
    memcpy(data + msgLen, &signalChangeMin, sizeof(signalChangeMin));
    msgLen += sizeof(signalChangeMin);
    signalChangeMax = reverseBytes_u32(signalChangeMax);
    memcpy(data + msgLen, &signalChangeMax, sizeof(signalChangeMax));
    msgLen += sizeof(signalChangeMax);
    memcpy(data + msgLen, signalData, 8);
    msgLen += 8;
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendSignalRelevanceEventMsg(uint8 netID, uint32 canID_1, uint32 signalScale_1, uint32 signalValue_1, uint32 signalValueMin_1, uint32 signalValueMax_1, uint8* signalData_1, 
    uint32 canID_2, uint32 signalScale_2, uint32 signalValue_2, uint32 signalValueMin_2, uint32 signalValueMax_2, uint8* signalData_2)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 60;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | A ID | A转换率 | A值 | A最小值 | A最大值 | A报文 | B ID | B转换率 | B值 | B最小值 | B最大值 | B报文 | 
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 | 4字节 | 4字节 | 4字节 | 8字节 | 4字节 | 4字节 | 4字节 | 4字节 | 4字节 | 8字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);
    uint16 operNum = 0x03EE;  //1006
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);

    canID_1 = reverseBytes_u32(canID_1);
    memcpy(data + msgLen, &canID_1, sizeof(canID_1));
    msgLen += sizeof(canID_1);
    signalScale_1 = reverseBytes_u32(signalScale_1);
    memcpy(data + msgLen, &signalScale_1, sizeof(signalScale_1));
    msgLen += sizeof(signalScale_1);
    signalValue_1 = reverseBytes_u32(signalValue_1);
    memcpy(data + msgLen, &signalValue_1, sizeof(signalValue_1));
    msgLen += sizeof(signalValue_1);
    signalValueMin_1 = reverseBytes_u32(signalValueMin_1);
    memcpy(data + msgLen, &signalValueMin_1, sizeof(signalValueMin_1));
    msgLen += sizeof(signalValueMin_1);
    signalValueMax_1 = reverseBytes_u32(signalValueMax_1);
    memcpy(data + msgLen, &signalValueMax_1, sizeof(signalValueMax_1));
    msgLen += sizeof(signalValueMax_1);
    memcpy(data + msgLen, signalData_1, 8);
    msgLen += 8;

    canID_2 = reverseBytes_u32(canID_2);
    memcpy(data + msgLen, &canID_2, sizeof(canID_2));
    msgLen += sizeof(canID_2);
    signalScale_2 = reverseBytes_u32(signalScale_2);
    memcpy(data + msgLen, &signalScale_2, sizeof(signalScale_2));
    msgLen += sizeof(signalScale_2);
    signalValue_2 = reverseBytes_u32(signalValue_2);
    memcpy(data + msgLen, &signalValue_2, sizeof(signalValue_2));
    msgLen += sizeof(signalValue_2);
    signalValueMin_2 = reverseBytes_u32(signalValueMin_2);
    memcpy(data + msgLen, &signalValueMin_2, sizeof(signalValueMin_2));
    msgLen += sizeof(signalValueMin_2);
    signalValueMax_2 = reverseBytes_u32(signalValueMax_2);
    memcpy(data + msgLen, &signalValueMax_2, sizeof(signalValueMax_2));
    msgLen += sizeof(signalValueMax_2);
    memcpy(data + msgLen, signalData_2, 8);
    msgLen += 8;
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendSessionAndResetEventMsg(uint8 netID, uint8 diagType, uint32 canID, uint8 subType)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 9;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | SF |
    | 1字节 | 2字节 | 1字节 | 4字节 | 1字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);

    uint16 operNum = 0;
    if (1 == diagType)
    {
        operNum = 0x07D1;  //2001
    }
    else if (2 == diagType)
    {
        operNum = 0x07D2;  //2002
    }
    
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);

    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    memcpy(data + msgLen, &subType, sizeof(subType));
    msgLen += sizeof(subType);
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

uint16 sendSafetyAccessEventMsg(uint8 netID, uint32 canID, uint32 accessNum)
{
    uint8 data[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 msgLen = 0;

    //头
    uint16 header = SK_COMM_SENDHEAD;
    header = reverseBytes_u16(header);
    memcpy(data, &header, sizeof(header));
    msgLen += sizeof(header);
    //设备ID
    uint8 devices = SK_COMM_DEVICE_BCM;
    memcpy(data + msgLen, &devices, sizeof(devices));
    msgLen += sizeof(devices);
    //功能码
    uint8 functionID = SK_COMM_FUNNCTION_VSOC;
    memcpy(data + msgLen, &functionID, sizeof(functionID));
    msgLen += sizeof(functionID);
    //子功能
    uint8 subfunction = SK_COMM_SUBFUNCTION_VSOC_EVENT;
    memcpy(data + msgLen, &subfunction, sizeof(subfunction));
    msgLen += sizeof(subfunction);

    //报文长度
    uint8 infoLen = 12;
    memcpy(data + msgLen, &infoLen, sizeof(infoLen));
    msgLen += sizeof(infoLen);
    /* 报文内容
    | 业务模块 | 详细业务编号 | 通道ID | CAN ID | 次数 |
    | 1字节 | 2字节 | 1字节 | 4字节 | 4字节 |
    */
    uint8 operModel = CAN_OPER_MODE; 
    memcpy(data + msgLen, &operModel, sizeof(operModel));
    msgLen += sizeof(operModel);

    uint16 operNum = 0x07D3;  //2003
    operNum = reverseBytes_u16(operNum);
    memcpy(data + msgLen, &operNum, sizeof(operNum));
    msgLen += sizeof(operNum);
    
    memcpy(data + msgLen, &netID, sizeof(netID));
    msgLen += sizeof(netID);

    canID = reverseBytes_u32(canID);
    memcpy(data + msgLen, &canID, sizeof(canID));
    msgLen += sizeof(canID);

    accessNum = reverseBytes_u32(accessNum);
    memcpy(data + msgLen, &accessNum, sizeof(accessNum));
    msgLen += sizeof(accessNum);
 
    //校验码
    uint8 checkCode = 0x5A;
    memcpy(data + msgLen, &checkCode, sizeof(checkCode));
    msgLen += sizeof(checkCode);

    pushMsgQueue(data, msgLen);
    return msgLen;
}

