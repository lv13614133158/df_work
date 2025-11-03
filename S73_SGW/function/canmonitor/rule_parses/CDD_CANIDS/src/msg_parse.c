#include "msg_parse.h"
#include "communicate.h"
#include "string.h"

#define POLICY_HEADER 6

#define FLOW_ITEM_LEN 14
#define LIST_ITEM_LEN 3
#define LENGTH_ITEM_LEN 4
#define PERIOD_ITEM_LEN 7
#define SIGNAL_ITEM_LEN 47
#define DIAG_ITEM_LEN 6

#define PACKET_MAX 200

//下一包配置长度
static uint8 nextPolicyLen = POLICY_HEADER;

/* 策略类型： 
1、流量分析  2、白名单  3、周期  4、长度  5、信号分析  6、诊断 */
static uint8 policyType = 0;

//当前类型的策略总数
static uint8 policyTotal = 0;
//当前包的策略总数
static uint8 policyPacketCnt = 0;
//策略项索引
static uint8 policyIndex = 0;

uint8 getNextPolicyLen()
{
    return nextPolicyLen;
}

uint8 SK_ParseConfig(uint8 *data, uint8 len)
{
//    requestFlag = false;
    uint8 res = 0;
    uint8 tpDataLen = data[5] + 6;
    if (tpDataLen != len)
    {
        return res;
    }

    uint8 msglen = data[5] - 4;   //策略包长度
    uint8* msgData = data + 10;

    if (0xFF == nextPolicyLen)
    {
        return msglen;
    }

    if ( 6 == msglen )  //长度为6时可能是包头
    {
        uint8 strflow[4] = "flow";
        uint8 strlist[4] = "list";
        uint8 strlen[4] = "clen";
        uint8 strper[4] = "peri";
        uint8 strsig[4] = "sign";
        uint8 strdiag[4] = "diag";

        if (memcmp(msgData, strflow, 4) == 0)
        {
            policyType = 1;
            uint8 ruleSwitch = msgData[4];
            setFlowSwitch(ruleSwitch);
            policyTotal = msgData[5];
            if (policyTotal * FLOW_ITEM_LEN < PACKET_MAX)
            {
                nextPolicyLen = policyTotal * FLOW_ITEM_LEN;
            }
            else
            {
                uint8 cnt = PACKET_MAX / FLOW_ITEM_LEN;
                nextPolicyLen = cnt * FLOW_ITEM_LEN;
            }
            policyIndex = 0;
            return msglen;
        }
        else if (memcmp(msgData, strlist, 4) == 0)
        {
            policyType = 2;
            uint8 ruleSwitch = msgData[4];
            setListSwitch(ruleSwitch);
            policyTotal = msgData[5];
            if (policyTotal * LIST_ITEM_LEN < PACKET_MAX)
            {
                nextPolicyLen = policyTotal * LIST_ITEM_LEN;
            }
            else
            {
                uint8 cnt = PACKET_MAX / LIST_ITEM_LEN;
                nextPolicyLen = cnt * LIST_ITEM_LEN;
            }
            policyIndex = 0;
            return msglen;
        }
        else if (memcmp(msgData, strper, 4) == 0)
        {
            policyType = 3;
            uint8 ruleSwitch = msgData[4];
            setPeriodSwitch(ruleSwitch);
            policyTotal = msgData[5];
            if (policyTotal * PERIOD_ITEM_LEN < PACKET_MAX)
            {
                nextPolicyLen = policyTotal * PERIOD_ITEM_LEN;
            }
            else
            {
                uint8 cnt = PACKET_MAX / PERIOD_ITEM_LEN;
                nextPolicyLen = cnt * PERIOD_ITEM_LEN;
            }
            policyIndex = 0;
            return msglen;
        }
        else if (memcmp(msgData, strlen, 4) == 0)
        {
            policyType = 4;
            uint8 ruleSwitch = msgData[4];
            setLengthSwitch(ruleSwitch);
            policyTotal = msgData[5];
            if (policyTotal * LENGTH_ITEM_LEN < PACKET_MAX)
            {
                nextPolicyLen = policyTotal * LENGTH_ITEM_LEN;
            }
            else
            {
                uint8 cnt = PACKET_MAX / LENGTH_ITEM_LEN;
                nextPolicyLen = cnt * LENGTH_ITEM_LEN;
            }
            policyIndex = 0;
            return msglen;
        }
        else if (memcmp(msgData, strsig, 4) == 0)
        {
            policyType = 5;
            uint8 ruleSwitch = msgData[4];
            setAnalySwitch(ruleSwitch);
            policyTotal = msgData[5];
            if (policyTotal * SIGNAL_ITEM_LEN < PACKET_MAX)
            {
                nextPolicyLen = policyTotal * SIGNAL_ITEM_LEN;
            }
            else
            {
                uint8 cnt = PACKET_MAX / SIGNAL_ITEM_LEN;
                nextPolicyLen = cnt * SIGNAL_ITEM_LEN;
            }
            policyIndex = 0;
            return msglen;         
        }
        else if (memcmp(msgData, strdiag, 4) == 0)
        {
            policyType = 6;
            uint8 ruleSwitch = msgData[4];
            setDiagSwitch(ruleSwitch);
            policyTotal = msgData[5];
            if (policyTotal * DIAG_ITEM_LEN < PACKET_MAX)
            {
                nextPolicyLen = policyTotal * DIAG_ITEM_LEN;
            }
            else
            {
                uint8 cnt = PACKET_MAX / DIAG_ITEM_LEN;
                nextPolicyLen = cnt * DIAG_ITEM_LEN;
            }
            policyIndex = 0;
            return msglen;
        }
    }
    
    switch (policyType)
    {
        case 1:
            for (size_t i = 0; i < msglen / FLOW_ITEM_LEN; i++)
            {
                uint8 netID = msgData[i*FLOW_ITEM_LEN];
                uint32 period = 0;
                memcpy(&period, msgData + i*FLOW_ITEM_LEN + 1, sizeof(uint32));
                period = reverseBytes_u32(period);
                uint32 flowMax = 0;
                memcpy(&flowMax, msgData + i*FLOW_ITEM_LEN + 5, sizeof(uint32));
                flowMax = reverseBytes_u32(flowMax);
                uint32 flowMin = 0;
                memcpy(&flowMin, msgData + i*FLOW_ITEM_LEN + 9, sizeof(uint32));
                flowMin = reverseBytes_u32(flowMin);
                uint8 loadrateThreshold = msgData[i*FLOW_ITEM_LEN + 13];
                
                SK_InsertRule_Flow(policyIndex, netID, period, flowMax, flowMin, loadrateThreshold);
                policyIndex++;
            }
            if (policyIndex < policyTotal) // 还有未更新的策略
            {
                if ((policyTotal - policyIndex) * FLOW_ITEM_LEN < PACKET_MAX)
                {
                    nextPolicyLen = (policyTotal - policyIndex) * FLOW_ITEM_LEN;
                }
                else
                {
                    uint8 cnt = PACKET_MAX / FLOW_ITEM_LEN;
                    nextPolicyLen = cnt * FLOW_ITEM_LEN;
                }
            }
            else  //更新下一个类型策略
            {
                nextPolicyLen = 6;
            }
            break;
        case 2:
            for (size_t i = 0; i < msglen / LIST_ITEM_LEN; i++)
            {
                uint8 netID = msgData[i*LIST_ITEM_LEN];
                uint16 canID = 0;
                memcpy(&canID, msgData + i*LIST_ITEM_LEN + 1, sizeof(uint16));
                canID = reverseBytes_u16(canID);
                
                SK_InsertRule_List(policyIndex, netID, canID);
                policyIndex++;
            }
            if (policyIndex < policyTotal)
            {
                if ((policyTotal - policyIndex) * LIST_ITEM_LEN < PACKET_MAX)
                {
                    nextPolicyLen = (policyTotal - policyIndex) * LIST_ITEM_LEN;
                }
                else
                {
                    uint8 cnt = PACKET_MAX / LIST_ITEM_LEN;
                    nextPolicyLen = cnt * LIST_ITEM_LEN;
                }
            }
            else
            {
                nextPolicyLen = 6;
            }
            break;
        case 3:
            for (size_t i = 0; i < msglen / PERIOD_ITEM_LEN; i++)
            {
                uint8 netID = msgData[i*PERIOD_ITEM_LEN];
                uint16 canID = 0;
                memcpy(&canID, msgData + i*PERIOD_ITEM_LEN + 1, sizeof(uint16));
                canID = reverseBytes_u16(canID);
                uint16 period = 0;
                memcpy(&period, msgData + i*PERIOD_ITEM_LEN + 3, sizeof(uint16));
                period = reverseBytes_u16(period);
                uint16 offset = 0;
                memcpy(&offset, msgData + i*PERIOD_ITEM_LEN + 5, sizeof(uint16));
                offset = reverseBytes_u16(offset);
                
                SK_InsertRule_Period(policyIndex, netID, canID, period, offset);
                policyIndex++;
            }
            if (policyIndex < policyTotal)
            {
                if ((policyTotal - policyIndex) * PERIOD_ITEM_LEN < PACKET_MAX)
                {
                    nextPolicyLen = (policyTotal - policyIndex) * PERIOD_ITEM_LEN;
                }
                else
                {
                    uint8 cnt = PACKET_MAX / PERIOD_ITEM_LEN;
                    nextPolicyLen = cnt * PERIOD_ITEM_LEN;
                }
            }
            else
            {
                nextPolicyLen = 6;
            }
            break;
            
        case 4:
            for (size_t i = 0; i < msglen / LENGTH_ITEM_LEN; i++)
            {
                uint8 netID = msgData[i*LENGTH_ITEM_LEN];
                uint16 canID = 0;
                memcpy(&canID, msgData + i*LENGTH_ITEM_LEN + 1, sizeof(uint16));
                canID = reverseBytes_u16(canID);
                uint8 length = msgData[i*LENGTH_ITEM_LEN + 3];

                SK_InsertRule_Length(policyIndex, netID, canID, length);
                policyIndex++;
            }
            if (policyIndex < policyTotal)
            {
                if ((policyTotal - policyIndex) * LENGTH_ITEM_LEN < PACKET_MAX)
                {
                    nextPolicyLen = (policyTotal - policyIndex) * LENGTH_ITEM_LEN;
                }
                else
                {
                    uint8 cnt = PACKET_MAX / LENGTH_ITEM_LEN;
                    nextPolicyLen = cnt * LENGTH_ITEM_LEN;
                }
            }
            else
            {
                nextPolicyLen = 6;
            }
            break;

        case 5:
            for (size_t i = 0; i < msglen / SIGNAL_ITEM_LEN; i++)
            {
                uint8 netID = msgData[i*SIGNAL_ITEM_LEN];
                uint16 canID = 0;
                memcpy(&canID, msgData + i*SIGNAL_ITEM_LEN + 1, sizeof(uint16));
                canID = reverseBytes_u16(canID);
                uint8 startBit = msgData[i*SIGNAL_ITEM_LEN + 3];
                uint8 stopBit = msgData[i*SIGNAL_ITEM_LEN + 4];
                uint8 signalType = msgData[i*SIGNAL_ITEM_LEN + 5];
                uint8 paraLen = msgData[i*SIGNAL_ITEM_LEN + 6];
                uint32 para[10] = {0};
                memcpy(para, msgData + i*SIGNAL_ITEM_LEN + 7, sizeof(uint32)*10);
                SK_InsertRule_Signal(policyIndex, netID, canID, startBit, stopBit, signalType, paraLen, para);
                policyIndex++;
            }
            if (policyIndex < policyTotal)
            {
                if ((policyTotal - policyIndex) * SIGNAL_ITEM_LEN < PACKET_MAX)
                {
                    nextPolicyLen = (policyTotal - policyIndex) * SIGNAL_ITEM_LEN;
                }
                else
                {
                    uint8 cnt = PACKET_MAX / SIGNAL_ITEM_LEN;
                    nextPolicyLen = cnt * SIGNAL_ITEM_LEN;
                }
            }
            else
            {
                nextPolicyLen = 6;
            }
            break;
        case 6:
            for (size_t i = 0; i < msglen / DIAG_ITEM_LEN; i++)
            {
                uint8 netID = msgData[i*DIAG_ITEM_LEN];
                uint16 canID = 0;
                memcpy(&canID, msgData + i*DIAG_ITEM_LEN + 1, sizeof(uint16));
                canID = reverseBytes_u16(canID);
                uint8 sesSwitch = msgData[i*DIAG_ITEM_LEN + 3];
                uint8 resetSwitch = msgData[i*DIAG_ITEM_LEN + 4];
                uint8 accSwitch = msgData[i*DIAG_ITEM_LEN + 5];
                SK_InsertRule_Diag(policyIndex, netID, canID, sesSwitch, resetSwitch, accSwitch);
                policyIndex++;
            }
            if (policyIndex < policyTotal)
            {
                if ((policyTotal - policyIndex) * DIAG_ITEM_LEN < PACKET_MAX)
                {
                    nextPolicyLen = (policyTotal - policyIndex) * DIAG_ITEM_LEN;
                }
                else
                {
                    uint8 cnt = PACKET_MAX / DIAG_ITEM_LEN;
                    nextPolicyLen = cnt * DIAG_ITEM_LEN;
                }
            }
            else
            {
                nextPolicyLen = 0xFF;
            }
            break;
        default:
            // nextPolicyLen = 0;
            return 0;
            break;
    }

    res = msglen;
    return res;
}
