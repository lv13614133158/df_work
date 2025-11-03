/**
 * 文件名: queue.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 配置规则文件操作
 */
#include "rule.h"
#include "flowcheck.h"
#include "lengthcheck.h"
#include "listcheck.h"
#include "periodcheck.h"
#include "signalanaly.h"
#include "ids_config.h"
#include "ctimer.h"
#include "event.h"
#include "diagnosecheck.h"
#include "string.h"


// 0填充
#define ZERO_INIT1  0
#define ZERO_INIT2  ZERO_INIT1, ZERO_INIT1
#define ZERO_INIT3  ZERO_INIT2, ZERO_INIT1
#define ZERO_INIT4  ZERO_INIT3, ZERO_INIT1
#define ZERO_INIT5  ZERO_INIT4, ZERO_INIT1
#define ZERO_INIT6  ZERO_INIT5, ZERO_INIT1
#define ZERO_INIT7  ZERO_INIT6, ZERO_INIT1
#define ZERO_INIT8  ZERO_INIT7, ZERO_INIT1
#define ZERO_INIT9  ZERO_INIT8, ZERO_INIT1
#define ZERO_INIT10 ZERO_INIT9, ZERO_INIT1


// 规则配置文件
typedef struct _SK_Config_Stru{
   Flow_Elmt    flowCheck[SK_FLOWANALY_NUM];
   List_Elmt    listCheck[SK_WHITELIST_NUM];
   Len_Elmt     lenCHeck[SK_LENCHECK_NUM];
   Prd_Elmt     prdCheck[SK_PRDCHECK_NUM];
   Signal_Elmt  signAnaly[SK_SIGNALMAX_NUM];
   Diag_Elmt   diagCheck[SK_DIAG_NUM];
   uint32 flowCheckCnt;
   uint32 listCheckCnt;
   uint32 lenCheckCnt;
   uint32 prdCheckCnt;
   uint32 signAnalyCnt;
   uint32 diagCheckCnt;
   bool   configFlag;
}SK_Config_Stru;
static SK_Config_Stru configObj = {0};

static  bool g_FlowSwitch   = 0;   // 流量检测开关/负载检测开关/doc监测开关
static  bool g_ListSwitch   = 0;   // 白名单检测开关 
static  bool g_LengthSwitch = 0;   // DLC开关
static  bool g_PriodSwitch  = 0;   // 周期检测开关
static  bool g_SignaSwitch  = 0;   // 信号分析开关，8位代表8个功能
static  bool g_DiagSwitch   = 0;

void setFlowSwitch(uint8 flowSwitch)
{
	g_FlowSwitch = flowSwitch;
}
uint8 getFlowSwitch()
{
	return g_FlowSwitch;
}
void setListSwitch(uint8 listSwitch)
{
	g_ListSwitch = listSwitch;
}
uint8 getListSwitch()
{
	return g_ListSwitch;
}
void setLengthSwitch(uint8 lengthSwitch)
{
	g_LengthSwitch = lengthSwitch;
}
uint8 getLengthSwitch()
{
	return g_LengthSwitch;
}
void setPeriodSwitch(uint8 periodSwitch)
{
	g_PriodSwitch = periodSwitch;
}
uint8 getPeriodSwitch()
{
	return g_PriodSwitch;
}
void setAnalySwitch(uint8 analySwitch)
{
	g_SignaSwitch = analySwitch;
}
uint8 getAnalySwitch()
{
	return g_SignaSwitch;
}
void setDiagSwitch(uint8 diagSwitch)
{
	g_DiagSwitch = diagSwitch;
}
uint8 getDiagSwitch()
{
	return g_DiagSwitch;
}

/**
 * 配置规则直接在这里填充
*/
// 1、流量表
static Flow_Elmt flowList[SK_FLOWANALY_NUM] = {
    //ch  prd  flow(3)  load(2)  dos
    0, 30000, 10000, 100, 0, 0.0, 0.8, 0,
	1, 30000, 10000, 100, 0, 0.0, 0.8, 0,
	2, 30000, 10000, 100, 0, 0.0, 0.8, 0,
	3, 30000, 10000, 100, 0, 0.0, 0.8, 0,
	4, 30000, 10000, 100, 0, 0.0, 0.8, 0,
	5, 30000, 10000, 100, 0, 0.0, 0.8, 0
};
// 2、白名单表
static List_Elmt whiteList[SK_WHITELIST_NUM] = {
    //ch id
	//info can 0x04
	4, 0x258,
	4, 0x242,
	4, 0x280,
	4, 0x282,
	4, 0x204,
	4, 0x288,
	4, 0x28B,
	4, 0x28F,
	4, 0x287,
	4, 0x281,
	4, 0x29A,
	4, 0x369,
	4, 0x291,
	4, 0x29E,
	4, 0x297,
	4, 0x299,
	4, 0x1A5,
	4, 0x28D,
	4, 0x28E,
	4, 0x2AB,
	4, 0x290,	
	4, 0x292,
	4, 0x294,
	4, 0x301,
	4, 0x29D,
	4, 0x29F,
	4, 0x8B,
	4, 0x372,
	4, 0x26C,
	4, 0x214,
	4, 0x285,
	4, 0x7C,
	4, 0x7D,
	4, 0x7E,
	4, 0x7F,
	4, 0x2F1,
	4, 0x6A0,
	4, 0x6A1,
	4, 0x6A8,
	4, 0x6A9,
	4, 0x450,
	4, 0x451,
	4, 0x452,
	4, 0x453,
	4, 0x456,
	4, 0x457,
	4, 0x464,
	4, 0x465,
	4, 0x5AF,
	4, 0x590,
	4, 0x5B8,
	4, 0x593,
	4, 0x5B4,
	//=====
	4, 0x216,	
	4, 0x284,
	4, 0x28C,

	//diag can 0x02
	2, 0x8D,
	2, 0x2A6,
	2, 0x2AE,
	2, 0x2A0,
	2, 0x2A2,
	2, 0x8A,
	2, 0x8E,
	2, 0x2A5,
	2, 0x2A7,
	2, 0x2A3,
	2, 0x2A8,
	2, 0x2AA,
	2, 0x462,
	2, 0x463,
	2, 0x58D,
	2, 0x750,
	2, 0x770,	
	//adas can 0x00
	0,	0x110,
	0,	0x260,
	0,	0x112,
	0,	0x1B2,
	0,	0x1B3,
	0,	0x1B4,
	0,	0x114,
	0,	0x262,
	0,	0x211,
	0,	0xD6,
	0,	0x11B,
	0,	0x11C,
	0,	0x11D,
	0,	0x11E,
	0,	0x11F,
	0,	0x25C,
	0,	0x25E,
	0,	0x430,
	0,	0x431,
	//====
	0,	0x599
};
// 3、长度表
static Len_Elmt lengthList[SK_LENCHECK_NUM] = {
    //ch id len state  
	//0,	0xCE , 8, 0,
	//info can 0x04
	4, 0x7C, 8, 0,
	4, 0x7D, 8, 0,
	4, 0x7E, 8, 0,
	4, 0x7F, 8, 0,
	4, 0x8B, 8, 0,
	4, 0x1A5, 8, 0,
	4, 0x214, 8, 0,
	4, 0x216, 8, 0,
	4, 0x242, 8, 0,
	4, 0x258, 8, 0,
	4, 0x26C, 8, 0,
	4, 0x280, 8, 0,
	4, 0x281, 8, 0,
	4, 0x282, 8, 0,
	4, 0x284, 8, 0,
	4, 0x285, 8, 0,
	4, 0x287, 8, 0,
	4, 0x288, 8, 0,
	4, 0x28B, 8, 0,
	4, 0x28C, 8, 0,
	4, 0x28D, 8, 0,
	4, 0x28E, 8, 0,
	4, 0x28F, 8, 0,
	4, 0x290, 8, 0,
	4, 0x291, 8, 0,
	4, 0x292, 8, 0,
	4, 0x294, 8, 0,
	4, 0x297, 8, 0,
	4, 0x299, 8, 0,
	4, 0x29A, 8, 0,
	4, 0x29D, 8, 0,
	4, 0x29E, 8, 0,
	4, 0x29F, 8, 0,
	4, 0x2AB, 8, 0,
	4, 0x2F1, 8, 0,
	4, 0x301, 8, 0,
	4, 0x372, 8, 0,
	4, 0x450, 8, 0,
	4, 0x451, 8, 0,
	4, 0x452, 8, 0,
	4, 0x453, 8, 0,
	4, 0x456, 8, 0,
	4, 0x457, 8, 0,
	4, 0x464, 8, 0,
	4, 0x465, 8, 0,
	4, 0x590, 8, 0,
	4, 0x593, 8, 0, 
	4, 0x5AF, 8, 0, 
	4, 0x5B4, 8, 0, 
	4, 0x5B8, 8, 0, 
	//diag can 0x02
	2, 0x8A, 8, 0,
	2, 0x8D, 8, 0,
	2, 0x8E, 8, 0,
	2, 0x2A0, 8, 0,
	2, 0x2A2, 8, 0,
	2, 0x2A3, 8, 0,
	2, 0x2A5, 8, 0,
	2, 0x2A6, 8, 0,
	2, 0x2A7, 8, 0,
	2, 0x2A8, 8, 0,
	2, 0x2AA, 8, 0,
	2, 0x2AE, 8, 0,
	2, 0x462, 8, 0,
	2, 0x463, 8, 0,
	2, 0x58D, 8, 0,
	//adas can 0x00
	0,	0xD6, 8, 0,
	0,	0x110, 8, 0,
	0,	0x112, 8, 0,
	0,	0x114, 8, 0,
	0,	0x11B, 8, 0,
	0,	0x11C, 8, 0,
	0,	0x11D, 8, 0,
	0,	0x11E, 8, 0,
	0,	0x11F, 8, 0,
	0,	0x1B2, 8, 0,
	0,	0x1B3, 8, 0,
	0,	0x1B4, 8, 0,
	0,	0x211, 8, 0,
	0,	0x25C, 8, 0,
	0,	0x25E, 8, 0,
	0,	0x260, 8, 0,
	0,	0x262, 8, 0,
	0,	0x430, 8, 0,
	0,	0x431, 8, 0,
	0,	0x599, 8, 0
};
// 4、周期表
static Prd_Elmt periodList[SK_PRDCHECK_NUM] = {
    //ch id period offset state continCnt time(3)
	//0,	0x22A, 100, 50, ZERO_INIT5,
	//info can 0x04
	4, 0x1A5, 50, 20, ZERO_INIT5,
	4, 0x214, 100, 40, ZERO_INIT5,
	4, 0x216, 100, 40, ZERO_INIT5,
	4, 0x242, 100, 40, ZERO_INIT5,
	4, 0x258, 100, 40, ZERO_INIT5,
	4, 0x26C, 100, 40, ZERO_INIT5,
	4, 0x280, 100, 40, ZERO_INIT5,
	4, 0x281, 100, 40, ZERO_INIT5,
	4, 0x282, 100, 40, ZERO_INIT5,
	4, 0x284, 100, 40, ZERO_INIT5,
	4, 0x285, 100, 40, ZERO_INIT5,
	4, 0x287, 100, 40, ZERO_INIT5,
	4, 0x288, 100, 40, ZERO_INIT5,
	4, 0x28B, 100, 40, ZERO_INIT5,
	4, 0x28C, 100, 40, ZERO_INIT5,
	4, 0x28D, 100, 40, ZERO_INIT5,
	4, 0x28E, 100, 40, ZERO_INIT5,
	4, 0x28F, 100, 40, ZERO_INIT5,
	4, 0x290, 100, 40, ZERO_INIT5,
	4, 0x291, 100, 40, ZERO_INIT5,
	4, 0x292, 100, 40, ZERO_INIT5,
	4, 0x294, 100, 40, ZERO_INIT5,
	4, 0x297, 100, 40, ZERO_INIT5,
	4, 0x299, 100, 40, ZERO_INIT5,
	4, 0x29A, 100, 40, ZERO_INIT5,
	4, 0x29D, 100, 40, ZERO_INIT5,
	4, 0x29E, 100, 40, ZERO_INIT5,
	4, 0x29F, 100, 40, ZERO_INIT5,
	4, 0x2AB, 100, 40, ZERO_INIT5,
	4, 0x2F1, 100, 40, ZERO_INIT5,
	4, 0x301, 1000, 100, ZERO_INIT5,
	4, 0x372, 100, 40, ZERO_INIT5,
	4, 0x450, 5000, 500, ZERO_INIT5,
	4, 0x451, 5000, 500, ZERO_INIT5,
	4, 0x451, 5000, 500, ZERO_INIT5,
	4, 0x452, 5000, 500, ZERO_INIT5,
	4, 0x453, 5000, 500, ZERO_INIT5,
	4, 0x456, 5000, 500, ZERO_INIT5,
	4, 0x457, 5000, 500, ZERO_INIT5,
	4, 0x464, 5000, 500, ZERO_INIT5,
	4, 0x465, 5000, 500, ZERO_INIT5,
	4, 0x590, 100, 40, ZERO_INIT5,
	4, 0x593, 100, 40, ZERO_INIT5,
	4, 0x5AF, 100, 40, ZERO_INIT5,
	4, 0x5B4, 100, 40, ZERO_INIT5,
	4, 0x5B8, 100, 40, ZERO_INIT5,
	//diag can 0x02
	2, 0x2A0, 100, 40, ZERO_INIT5,
	2, 0x2A2, 100, 40, ZERO_INIT5,
	2, 0x2A3, 100, 40, ZERO_INIT5,
	2, 0x2A5, 100, 40, ZERO_INIT5,
	2, 0x2A6, 100, 40, ZERO_INIT5,
	2, 0x2A7, 100, 40, ZERO_INIT5,
	2, 0x2A8, 100, 40, ZERO_INIT5,
	2, 0x2AA, 100, 40, ZERO_INIT5,
	2, 0x2AE, 100, 40, ZERO_INIT5,
	2, 0x462, 5000, 500, ZERO_INIT5,
	2, 0x463, 5000, 500, ZERO_INIT5,
	2, 0x58D, 100, 40, ZERO_INIT5,
	//adas can 0x00
	0,	0xD6, 10, 5, ZERO_INIT5,
	0,	0x110, 20, 10, ZERO_INIT5,
	0,	0x112, 20, 10, ZERO_INIT5,
	0,	0x114, 20, 10, ZERO_INIT5,
	0,	0x11B, 50, 25, ZERO_INIT5,
	0,	0x11C, 50, 25, ZERO_INIT5,
	0,	0x11D, 50, 25, ZERO_INIT5,
	0,	0x11E, 50, 25, ZERO_INIT5,
	0,	0x11F, 50, 25, ZERO_INIT5,
	0,	0x1B2, 20, 10, ZERO_INIT5,
	0,	0x1B3, 20, 10, ZERO_INIT5,
	0,	0x1B4, 20, 10, ZERO_INIT5,
	0,	0x211, 100, 40, ZERO_INIT5,
	0,	0x25C, 100, 40, ZERO_INIT5,
	0,	0x25E, 100, 40, ZERO_INIT5,
	0,	0x260, 100, 40, ZERO_INIT5,
	0,	0x262, 100, 40, ZERO_INIT5,
	0,	0x430, 5000, 500, ZERO_INIT5,
	0,	0x431, 5000, 500, ZERO_INIT5,
	0,	0x599, 100, 40, ZERO_INIT5
};

// 5、信号分析表
static Signal_Elmt msgList[SK_SIGNALMAX_NUM] = {
    {.netID = 4, .canID = 0x216, .startBit = 0, .stopBit = 2, .dataType = SIG_TYPE_THR, .ruleLen = 3, .rule.comPara = {10, 25, 0}},
    {.netID = 4, .canID = 0x2C6, .startBit = 0, .stopBit = 4, .dataType = SIG_TYPE_CHA, .ruleLen = 4, .rule.comPara = {10, 100, 0, 0}},
    {.netID = 4, .canID = 0x2C6, .startBit = 0, .stopBit = 4, .dataType = SIG_TYPE_ELM, .ruleLen = 7, .rule.comPara = {1, 3, 5, 6, 7, 8, 10}},
    {.netID = 4, .canID = 0x216, .startBit = 0, .stopBit = 4, .dataType = SIG_TYPE_TRA, .ruleLen = 3, .rule.comPara = {10, 100, 5}},
    {.netID = 4, .canID = 0x216, .startBit = 0, .stopBit = 4, .dataType = SIG_TYPE_STA, .ruleLen = 4, .rule.comPara = {3, 15, 8, 11}},
    {.netID = 4, .canID = 0x216, .startBit = 0, .stopBit = 4, .dataType = SIG_TYPE_PRE, .ruleLen = 3, .rule.comPara = {15, 3, 10}},
    {.netID = 4, .canID = 0x216, .startBit = 0, .stopBit = 3, .dataType = SIG_TYPE_SIM, .ruleLen = 3, .rule.comPara = {1, 3, 6}},
    {.netID = 4, .canID = 0x216, .startBit = 0, .stopBit = 3, .dataType = SIG_TYPE_DIS, .ruleLen = 3, .rule.comPara = {5, 3, 7}},
};

// 6、诊断表
static Diag_Elmt diagList[SK_DIAG_NUM] = {
    // ch id
    2, 0x58D, 0, 0, 0, 0,
	2, 0x2AE, 0, 0, 0, 0
};

// 填充长度安全检查
static uint32 CheckIndexRange(uint32 index, uint32 maxIndex, const char* title)
{
    if(index >= maxIndex)
    {
       // Debug_Print(0, "Config of %s Fill out of range!\n", title);
        return false;
    }
    return true;
}

// 流量配置参数填充
uint32 Init_FlowCheck_Config() 
{
    for(int i=0; i<sizeof(flowList)/sizeof(flowList[0]); i++)
    {
		if(CheckIndexRange(i, SK_FLOWANALY_NUM, "flow")){
			SK_FlowInit(configObj.flowCheck, i, flowList[i].netID, flowList[i].period, flowList[i].flowMax, flowList[i].flowMin, flowList[i].loadrateThreshold);
			configObj.flowCheckCnt++;
		}
    }   
    return configObj.flowCheckCnt;
}

// 白名单监测配置参数填充
uint32 Init_ListCheck_Config() 
{
    for(int i=0; i<sizeof(whiteList)/sizeof(whiteList[0]); i++)
    {
		if(CheckIndexRange(i, SK_WHITELIST_NUM, "list")){
			SK_ListInit(configObj.listCheck, i, whiteList[i].netID, whiteList[i].canID);
			configObj.listCheckCnt++;
		}
    }
    return configObj.listCheckCnt;
}

// 长度监测配置参数填充
uint32 Init_LenCheck_Config() 
{
    for(int i=0; i<sizeof(lengthList)/sizeof(lengthList[0]); i++)
    {
		if(CheckIndexRange(i, SK_LENCHECK_NUM, "len")){
			SK_LenInit(configObj.lenCHeck, i, lengthList[i].netID, lengthList[i].canID, lengthList[i].length);
			configObj.lenCheckCnt++;
		}
    }
    return configObj.lenCheckCnt;
}

// 周期监测配置参数填充
uint32 Init_PrdCheck_Config() 
{
    for(int i=0; i<sizeof(periodList)/sizeof(periodList[0]); i++)
    {
		if(CheckIndexRange(i, SK_PRDCHECK_NUM, "period")){
			SK_PrdInit(configObj.prdCheck, i, periodList[i].netID, periodList[i].canID, periodList[i].period, periodList[i].offset);
			configObj.prdCheckCnt++;
		}
    }
    return configObj.prdCheckCnt;
}

// 信号监测配置参数自适应调充数据
uint32 Init_SignalAnaly_Config() 
{
    for(int i=0; i<sizeof(msgList)/sizeof(Signal_Elmt); i++)
    {
		if(CheckIndexRange(i, SK_SIGNALMAX_NUM, "smg"))
		{
			SK_SignalInit(configObj.signAnaly, i, msgList[i].netID, msgList[i].canID, msgList[i].startBit, msgList[i].stopBit, 
				msgList[i].dataType, msgList[i].rule.comPara, msgList[i].ruleLen);
			configObj.signAnalyCnt++;
		}
    }
    return configObj.signAnalyCnt;
}

// 诊断监测配置参数自适应调充数据
uint32 Init_DiagCheck_Config() 
{
    for(int i=0; i<sizeof(diagList)/sizeof(Diag_Elmt); i++)
    {
		if(CheckIndexRange(i, SK_DIAG_NUM, "smg"))
		{
			SK_DiagInit(configObj.diagCheck, i, diagList[i].netID, diagList[i].canID, diagList[i].sessionSwitch, diagList[i].resetSwitch, 
				diagList[i].accessSwitch, diagList[i].communiSwitch);
			configObj.diagCheckCnt++;
		}
    }
    return configObj.diagCheckCnt;
}

// 规则初始化
void SK_RuleInit()
{
    SK_RuleClear();
    Init_FlowCheck_Config();
    Init_ListCheck_Config();
    Init_LenCheck_Config();
    Init_PrdCheck_Config();
    Init_SignalAnaly_Config();
	Init_DiagCheck_Config();
    configObj.configFlag = true;
}

// 规则清空
void SK_RuleClear()
{
    configObj.flowCheckCnt = 0;
    configObj.listCheckCnt = 0;
    configObj.lenCheckCnt  = 0;
    configObj.prdCheckCnt  = 0;
    configObj.signAnalyCnt = 0;
	configObj.diagCheckCnt = 0;
    configObj.configFlag = false;
}

// 查询规则是否初始化
bool SK_IsRuleInit()
{
    return configObj.configFlag;
}

// 流量分析
bool SK_Rule_FlowCheck(SK_Data_Stru data)
{
	bool ret = SK_FlowCheck((Flow_Elmt*)configObj.flowCheck, data, configObj.flowCheckCnt);
	return ret;
}

bool SK_Rule_LoadDisplay(uint32 flowSwitch)
{
	bool ret = SK_LoadDisplay((Flow_Elmt*)configObj.flowCheck, configObj.flowCheckCnt, flowSwitch);
	return ret;
}

// 长度检测
bool SK_Rule_LengthCheck(SK_Data_Stru data)
{
	bool ret = SK_LengthCheck((Len_Elmt*)configObj.lenCHeck, data, configObj.lenCheckCnt);
	return ret;
}

// 白名单检测
bool SK_Rule_ListCheck(SK_Data_Stru data)
{
	bool ret = SK_ListCheck((List_Elmt*)configObj.listCheck, data, configObj.listCheckCnt);
	return ret;
}

// 周期分析
bool SK_Rule_PeriodCheck(SK_Data_Stru data)
{
	SK_PeriodCheck((Prd_Elmt*)configObj.prdCheck, data, configObj.prdCheckCnt);
	return true;
}

bool SK_Rule_PeriodLossCheck()
{
	SK_PeriodLossCheck((Prd_Elmt*)configObj.prdCheck, configObj.prdCheckCnt);
	return true;
}

// 信号分析
bool SK_Rule_SignalAnaly(SK_Data_Stru data, uint32 msgSwitch)
{
	SK_SignalAnaly((Signal_Elmt*)configObj.signAnaly, data, msgSwitch, configObj.signAnalyCnt);
	return true;
}

bool SK_Rule_DiagCheck(SK_Data_Stru data)
{
	SK_DiagCheck((Diag_Elmt*)configObj.diagCheck, data, configObj.diagCheckCnt);
	return true;
}


bool SK_StaticRule_Clear()
{
	memset(flowList, 0, sizeof(flowList));
	memset(whiteList, 0, sizeof(whiteList));
	memset(lengthList, 0, sizeof(lengthList));
	memset(periodList, 0, sizeof(periodList));
	memset(msgList, 0, sizeof(msgList));
	memset(diagList, 0, sizeof(diagList));
	SK_RuleClear();
	return true;
}

bool SK_InsertRule_Flow(uint32 num, uint8 netID, uint32 period, uint32 flowMax, uint32 flowMin, uint32 loadrateThreshold)
{
	if (num >= SK_FLOWANALY_NUM)
	{
		return false;
	}
	float threshold = loadrateThreshold / 100;
	flowList[num].netID = netID;
	flowList[num].period = period;
	flowList[num].flowMax = flowMax;
	flowList[num].flowMin = flowMin;
	flowList[num].loadrateThreshold = threshold;
	flowList[num].loadrate = 0;
	flowList[num].dosFalg = 0;
	flowList[num].flowCnt = 0;
	
	return true;
}

bool SK_InsertRule_List(uint32 num, uint8 netID, uint32 canID)
{
	if (num >= SK_WHITELIST_NUM)
	{
		return false;
	}
	whiteList[num].netID = netID;
	whiteList[num].canID = canID;
	return true;
}

bool SK_InsertRule_Length(uint32 num, uint8 netID, uint32 canID, uint8 length)
{
	if (num >= SK_LENCHECK_NUM)
	{
		return false;
	}
	lengthList[num].netID = netID;
	lengthList[num].canID = canID;
	lengthList[num].length = length;
	lengthList[num].stateFalg = 0;
	return true;
}

bool SK_InsertRule_Period(uint32 num, uint8 netID, uint32 canID, uint32 period, uint32 offset)
{
	if (num >= SK_PRDCHECK_NUM)
	{
		return false;
	}
	periodList[num].netID = netID;
	periodList[num].canID = canID;
	periodList[num].period = period;
	periodList[num].offset = offset;
	periodList[num].stateFalg = 0;
	periodList[num].continCnt = 0;
	periodList[num].timeCnt.sysTimeH = 0;
	periodList[num].timeCnt.sysTimeL = 0;
	periodList[num].timeCnt.scale = 0;
	return true;
}

bool SK_InsertRule_Signal(uint32 num, uint8 netID, uint32 canID, uint8 startBit, uint8 stopBit, uint8 dataType, uint8 ruleLen, uint32* comPara)
{
	if (num >= SK_SIGNALMAX_NUM)
	{
		return false;
	}
	msgList[num].netID = netID;
	msgList[num].canID = canID;
	msgList[num].startBit = startBit;
	msgList[num].stopBit = stopBit;
	msgList[num].dataType = dataType;
	msgList[num].ruleLen = ruleLen;
	for (int i = 0; i < ruleLen; i++)
	{
		msgList[num].rule.comPara[i] = comPara[i];
	}
	return true;
}

bool SK_InsertRule_Diag(uint32 num, uint8 netID, uint32 canID, uint8 sesSwitch, uint8 resetSwitch, uint8 accSwitch)
{
	if (num >= SK_DIAG_NUM)
	{
		return false;
	}
	diagList[num].netID = netID;
	diagList[num].canID = canID;
	diagList[num].sessionSwitch = sesSwitch;
	diagList[num].resetSwitch = resetSwitch;
	diagList[num].accessSwitch = accSwitch;
	diagList[num].communiSwitch = 0;
	return true;
}
