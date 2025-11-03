#include "CANIDS_Working.h"
#include <stdio.h>
#include "event.h"
#include "log.h"
#include "queue.h"
#include "rule.h"
#include "ctimer.h"
#include "communicate.h"
#include "msg_parse.h"
#include <string.h>
#include <time.h>  
// IDS 的版本号
#define IDS_VERSION     "CAN_IDS_V1.0.0_MCU"

#define TIME_WINDOW 60
#define REPLAY_THRESHOLD 10
#define CAN_TABLE_SIZE 30
//启动过程状态
static bool registerFlag = 0;   //注册标识
static bool sessionFlag = 0;    //协商标识

static  char startFalg    = 0;   // 启动标志


static uint32 policyTotalLen = 0; //当前策略总长度
static uint32 policyDataLen = 0; //当前接收到策略数据长度
static uint32 lastPolStart = 0; //上一条策略开始位置
static uint8 policyCurrentLen = 0; //当前请求策略长度

//是否接收到CAN数据
static uint16 receCanMsg = 0;



typedef struct {
    uint32_t can_id;        // canID
    uint8_t data[8];       //can data
    time_t first_seen;       // 第一次见到该连接的时间
    time_t last_seen;        // 最后一次见到该连接的时间
    int count;               // 相同序列号出现的次数
} can_table_info;



can_table_info can_table[CAN_TABLE_SIZE];
int can_table_size = 0;

// 回调版本号
static char* SK_Get_Version()
{
   // Debug_Print(1, "%s", IDS_VERSION);
    return IDS_VERSION;
}

// can数据接收
uint8 SK_CANIDS_MsgReceiveIndi(uint8 netID, uint32 canID, uint8* data, uint32 len)
{
    int ret = -1;
    if(startFalg)
    {
        if (0 == receCanMsg)
            receCanMsg = 1;

        ret = SK_Can_PushQueue(netID, canID, data, len);
    }   
    ret = (ret>=0);
    return ret;
}

// 假设新增的CAN重放攻击检测函数
int is_can_replay_attack(SK_Data_Stru *can_frame, time_t current_time) {
    int i;
    int found = -1;

    uint32_t can_id = can_frame->canID;
    uint8_t data[8];
    memcpy(data, can_frame->data, 8);
    for (i = 0; i < can_table_size; i++) {
        if (can_table[i].can_id == can_id && 
            memcmp(can_table[i].data, data, 8) == 0) {
            found = i;
            break;
        }
    }

    if (found != -1) {
        // 更新记录
        can_table[found].last_seen = current_time;
        can_table[found].count++;
        if (current_time - can_table[found].first_seen <= TIME_WINDOW) {
            if (can_table[found].count >= REPLAY_THRESHOLD) {
                // 发现重放攻击，重置计数器以便继续检测
                can_table[found].count = 1;
                can_table[found].first_seen = current_time;
                return 1;
            }
        } else {
            // 超出时间窗口，重置计数器和时间
            can_table[found].count = 1;
            can_table[found].first_seen = current_time;
        }
    } else {
        // 添加新记录
        if (can_table_size < CAN_TABLE_SIZE) {
            can_table[can_table_size].can_id = can_id;
            memcpy(can_table[can_table_size].data, data, 8);
            can_table[can_table_size].first_seen = current_time;
            can_table[can_table_size].last_seen = current_time;
            can_table[can_table_size].count = 1;
            can_table_size++;
        }else
        {
            printf("CANIDS: CAN table full\n");
        }
    }
    return 0;
}

void SK_CANIDS_ReplayAttack(SK_Data_Stru data)
{
     // 新增CAN重放攻击检测逻辑
  
    time_t current_time = time(NULL);
    if (is_can_replay_attack(&data, current_time)) {
        printf("异常：CAN重放攻击检测到！ canid =0x%0x , 0x%0x 0x%0x 0x%0x 0x%0x\n", \
            data.canID,data.data[0],data.data[1],data.data[2],data.data[3]);
        return; 
    }
}

void SK_CANIDS_TPMsgReceive(uint8* data, uint16 dataLen)
{
   // Debug_Print(0, "msg:%s", data);
    if (dataLen > SK_COMM_BUF_MAXSIZE)
    {
        return;
    }
    uint8 recData[SK_COMM_BUF_MAXSIZE] = {0};
    uint16 recDataLen = dataLen;
    memcpy(recData, data, dataLen);
    
    uint16 msgHead;
    memcpy(&msgHead, recData, sizeof(uint16) );
    msgHead = reverseBytes_u16(msgHead);



    //IVI异步发送应该为0x55AA
    if (msgHead == SK_COMM_SENDHEAD)
    {
        if (recData[2] != SK_COMM_DEVICE_IVI)
        {
            return;
        }

        if (recData[3] == SK_COMM_FUNNCTION_VSOC )
        {
            if (recData[4] == SK_COMM_SUBFUNCTION_VSOC_REGISTERACK)
            {
                uint8 ackMsg[6] = "regack";
                if (memcmp(recData+6, ackMsg, 6) == 0)
                {
                    uint16 ackres = 0;
                    memcpy(&ackres, recData+12, sizeof(uint16));
                    ackres = reverseBytes_u16(ackres);
                    if (0x0064 == ackres)
                    {
                        registerFlag = 1;
                    }
                    else
                    {
                        SK_CANIDS_Start();
                    }
                }            
            }
            else if (recData[4] == SK_COMM_SUBFUNCTION_VSOC_SESSIONACK)
            {
                uint8 ackMsg[6] = "sesack";
                if (memcmp(recData+6, ackMsg, 6) == 0)
                {
                    uint16 sesres = 0;
                    memcpy(&sesres, recData+12, sizeof(uint16));
                    sesres = reverseBytes_u16(sesres);
                    if (0x0064 == sesres)
                    {
                        sessionFlag = 1;
                        SK_StaticRule_Clear();
                    }
                    else
                    {
                        SK_CANIDS_Start();
                    }
                }
            } 
            else if (recData[4] == SK_COMM_SUBFUNCTION_VSOC_POLICYACK)
            {
                uint8 ackMsg[6] = "polack";
                if (memcmp(recData+6, ackMsg, 6) == 0)
                {
                    uint32 polLen = 0;
                    memcpy(&polLen, recData + 12, sizeof(uint32));
                    polLen = reverseBytes_u32(polLen);
                    policyTotalLen = polLen;
                    policyDataLen = 0;
                }
            }
        }
    }
    //IVI同步回复应为0xAA55
    else if (msgHead == SK_COMM_REQHEAD)
    {
        if (SK_COMM_DEVICE_IVI == data[2] && SK_COMM_FUNNCTION_VSOC == data[3] && SK_COMM_SUBFUNCTION_VSOC_REQRULE == data[4])
        {
            uint32 recPolStart = 0;
            memcpy(&recPolStart, data+6, sizeof(uint32));
            recPolStart = reverseBytes_u32(recPolStart);
            if (recPolStart < lastPolStart)
            {
                return;
            }
            
            if(SK_ParseConfig(data, recDataLen) == policyCurrentLen)
            {
                lastPolStart = policyDataLen + 1;
                policyDataLen = policyDataLen + policyCurrentLen;
                if (policyDataLen >= policyTotalLen)
                {
                    SK_CANIDS_Start();
                }
            }
        }       
    }
}


// 时间循环，周期调用can功能函数
uint8 SK_CANIDS_5ms_MainfunctionEx()
{
    Cnt_OS_Time(); //更新系统时间

    //当大于10秒时进入开始功能
    if(1 )
  // if(Get_OS_Time().sysTimeH > 0 || (Get_OS_Time().sysTimeH == 0 && Get_OS_Time().sysTimeL >= 10000) )
    {
        sendMsgFromQueue(); //发送消息队列
        if(startFalg)
        {
            if (Get_OS_Time().sysTimeL % 15000 == 0)  //15s发送一次心跳
            {
                sendCommHeartMsg(SK_COMM_SUBFUNCTION_HEART_CYCLE);
                if ( 0 == receCanMsg )
                {
                    char errorMsg[6] = "nodata";
                    sendCommLogMsg(SK_COMM_SUBFUNCTION_LOG_RUN, errorMsg, 6, 0);
                }
                
            }

            SK_Data_Stru canData = {0};
            // 数据出栈
            while(SK_Can_PopQueue(&canData) >= 0)
            {

                SK_CANIDS_ReplayAttack(canData);
           
                // 流量数据统计
                if(getFlowSwitch())
                {
                    SK_Rule_FlowCheck(canData);
                }
                //白名单检测
                if(getListSwitch())
                {
                    if(SK_Rule_ListCheck(canData) == false)
                    {
                        continue;
                    }
                }
                // 长度监测
                if(getLengthSwitch())
                {
                    if(SK_Rule_LengthCheck(canData) == false)
                        continue;
                }
                // 周期监测
                if(getPeriodSwitch()){
                    SK_Rule_PeriodCheck(canData);
                }
                // 信号分析监测
                if(getAnalySwitch()){
                    SK_Rule_SignalAnaly(canData, getAnalySwitch());
                }
                // 诊断监测
                if(1)
               // if(getDiagSwitch())
                {
                    SK_Rule_DiagCheck(canData);
                }
            }
            // 流量监测
            if(getFlowSwitch())
            {
                SK_Rule_LoadDisplay(getFlowSwitch());
            }

        }
        else
        {
            if (Get_OS_Time().sysTimeL % 2000 == 0) // 每2000ms发送一次
            {
                if (0 == registerFlag)
                {
                    uint8 message[8] = "register";
                    sendCommVSocMsg(SK_COMM_SUBFUNCTION_VSOC_REGISTER, message, 8);
                }
                if (1 == registerFlag && 0 == sessionFlag)
                {
                    uint8 message[7] = "session";
                    sendCommVSocMsg(SK_COMM_SUBFUNCTION_VSOC_SESSION, message, 7);
                }
                if (1 == registerFlag && 1 == sessionFlag)
                {
                    //请求获取策略长度
                    if (0 == policyTotalLen)
                    {
                        uint8 message[6] = "policy";
                        sendCommVSocMsg(SK_COMM_SUBFUNCTION_VSOC_POLICY, message, 6);
                    }
                    //分包请求策略
                    else 
                    {
                        if (policyDataLen < policyTotalLen)
                        {
                            uint8 message[5] = {0};
                            uint32 startNum = reverseBytes_u32(policyDataLen);
                            memcpy(message, &startNum, 4);

                            policyCurrentLen = getNextPolicyLen();
                            if (0xFF != policyCurrentLen)
                            {
                                message[4] = policyCurrentLen;
                                sendCommVSocMsg(SK_COMM_SUBFUNCTION_VSOC_REQRULE, message, 5);
                            }
                        }
                    }
                }
            }  
        }   
    }
    return true;
}


// 磁盘写入
ssize_t SK_CANIDS_MemWrite(int addr, const void* buf, size_t count)
{
    return 0;
}   

// 磁盘读取
ssize_t SK_CANIDS_MemRead(int addr,  void* buf,  size_t count )
{
    return 0;
}

// 事件消息发送
uint8 SK_CANIDS_EventSendReqEx(uint8 *data, uint32 len)
{
    return true;
}

// 事件消息接收
uint8 SK_CANIDS_EventReceiveReqEx(uint8 *data, uint32 len)
{
    return true;
}


// 框架初始化 
uint8 SK_CANIDS_Init_Ex(IDS_Stru canIDS, uint8 (*pSendMsg)(uint8* data, uint16 len) )
{
    //pSendMsgForTp = pSendMsg;
    initCommunicate(pSendMsg);
    //Debug_Print(1, "[I]IDS init");
    
    SK_RuleInit();
    SK_Can_InitQueue();
    receCanMsg = 0;
    return true;
}

// 删除
uint8 SK_CANIDS_DeInit_Ex()
{
    //Debug_Print(1, "[I]IDS delect");
    //SK_RuleClear();
    receCanMsg = 0;
    return true;
}

// 启动
uint8 SK_CANIDS_Start()
{
    SK_RuleInit();
    startFalg = true;

    return startFalg;
}

// 停止
uint8 SK_CANIDS_Stop()
{
   // Debug_Print(1, "[I]IDS stop");
    startFalg = false;
    return startFalg;
}

