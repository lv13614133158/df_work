/**
 * 文件名: queue.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 配置规则文件操作
 */
#include "rule.h"
#include <stdio.h>
#include <stdlib.h>
#include "flowcheck.h"
#include "lengthcheck.h"
#include "listcheck.h"
#include "periodcheck.h"
#include "signalanaly.h"
#include "ids_config.h"
#include "ctimer.h"
#include "event.h"
#include "cJSON.h"

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
   uint32 flowCheckCnt;
   uint32 listCheckCnt;
   uint32 lenCheckCnt;
   uint32 prdCheckCnt;
   uint32 signAnalyCnt;
   bool   configFlag;
}SK_Config_Stru;
static SK_Config_Stru configObj = {0};

/**
 * 配置规则直接在这里填充
*/
// 1、流量表
static Flow_Elmt flowList[1] = {
    //ch  prd  flow(3)  load(2)   
    0, 3000, 10000, 10, ZERO_INIT5
};
// 2、白名单表
static List_Elmt whiteList[26] = {
    //ch id
	0,	0xFE,//254
	0,	0x364,//868
	0,	0x3E2,//994
	0,	0xA2,//162
	0,	0x33C,//828
	0,	0x30C,//780
	0,	0x47E,//1150
	0,	0x425,//1061
	0,	0x304,//772
	0,	0x23A,//570
	0,	0x303,//771
	0,	0x33D,//829
	0,	0x322,//802
	0,	0x328,//808
	0,	0x36A,//874
	0,	0x419,//1049
	0,	0x37C,//892
	0,	0x60D,//1549
	0,	0x467,//1127
	0,	0x133,//307
	0,	0x11D,//285
	0,	0x1C8,//456
	0,	0x30A,//778
	0,	0x30B,//779
	0,	0x3F0,//1008
	0,	0x16//22
};
// 3、长度表
static Len_Elmt lengthList[3] = {
    //ch id len state  
	0,	0x33C , 8, 0,
	0,	0x322 , 8, 0,
	0,	0x30C , 8, 0
};
// 4、周期表
static Prd_Elmt periodList[3] = {
    //ch id period(2) state(2) time(3)
	0,	0x33C, 100, 50, ZERO_INIT6,
	0,	0x322, 100, 10, ZERO_INIT6,
	0,	0x30C, 100, 10, ZERO_INIT6
};

// 5、信号分析表
static Signal_Elmt msgList[7] = {
    {0, 0x322, "environmentTemp", 0, 7,  SIG_TYPE_THR, 3, {0x00, 0xB4, 3}},
    {0, 0x322, "1111", 0, 7,  SIG_TYPE_CHA, 4, {0, 0xA, 0, 0}},
    {0, 0x322, "1111", 8, 9,  SIG_TYPE_THR, 3, {0x00, 0x2, 3}},
    {0, 0x322, "1111", 8, 9,  SIG_TYPE_ELM, 3, {0, 1, 2}},
    {0, 0x322, "1111", 0, 1,  SIG_TYPE_STA, 3, {0, 1, 2}},//实际参考0x30C
    {0, 0x322, "1111", 0, 1,  SIG_TYPE_TRA, 3, {0, 1, 2}},//测试,未完善
    {0, 0x322, "1111", 0, 1,  SIG_TYPE_PRE, 3, {0, 1, 2}}//测试,未完善
    // {0, 0x127, 0, 4,  SIG_TYPE_CHA, 4, {10, 100, 0, 0}},
    // {0, 0x245, 0, 4,  SIG_TYPE_ELM, 7, {1, 3, 5, 6, 7, 8, 10}},
    // {0, 0x22C, 0, 4,  SIG_TYPE_TRA, 3, {10, 100, 5}},
    // {0, 0x1F4, 0, 4,  SIG_TYPE_STA, 4, {3, 15, 8, 11}},
    // {0, 0x32C, 0, 4,  SIG_TYPE_PRE, 3, {15, 3, 10}},
    // {0, 0x1F3, 0, 3,  SIG_TYPE_SIM, 3, {1, 3, 6}},
    // {0, 0x129, 0, 3,  SIG_TYPE_DIS, 3, {5, 3, 7}},
};

// 填充长度安全检查
static uint32 CheckIndexRange(uint32 index, uint32 maxIndex, const char* title)
{
    if(index >= maxIndex)
    {
        Debug_Print(0, "Config of %s Fill out of range!\n", title);
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
			SK_FlowInit(configObj.flowCheck, i, flowList[i].netID, flowList[i].period, flowList[i].flowMax, flowList[i].flowMin);
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
			SK_SignalInit(configObj.signAnaly, i, msgList[i].netID, msgList[i].canID, msgList[i].signal_name, msgList[i].startBit, msgList[i].stopBit, 
				msgList[i].dataType, msgList[i].rule.comPara, msgList[i].ruleLen);
			configObj.signAnalyCnt++;
		}
    }
    return configObj.signAnalyCnt;
}

void can_rule_parse(char* config)
{
	cJSON *root = NULL;
    int netID = 0; 
    int canID = 0;
    int can_id_white_list_element_find_flag = 0;

    int flow_period = 0;
    int flowMax = 0;
    int flowMin = 0;
    int can_flow_element_find_flag = 0;

    int length = 0;
    int can_length_element_find_flag = 0;

    int period_period = 0;
    int period_offset = 0;
    int can_period_element_find_flag = 0;

    uint64 comPara[SMG_COMPARA_MAX_SIZE] = {0};
    int comPara_cnt = 0;
    int ruleLen = 0;
    char* signal_name =NULL;
    int startBit = 0;
    int stopBit = 0;
    int dataType =0;
    int can_signal_element_find_flag = 0;

    root = cJSON_Parse(config);
	if(!root)
	{
	    return;
    }

    cJSON* j_can_flow_list = cJSON_GetObjectItem(root,"can_flow_list");
    if (j_can_flow_list)
    {
    	int size = cJSON_GetArraySize(j_can_flow_list);
    	for (int i=0; i< size; i++)
        {
    		cJSON* child = cJSON_GetArrayItem(j_can_flow_list, i);
    		if (!child)
            {
    			printf("Parse Attacklist element  error\n");
    			break;
    		}

            can_flow_element_find_flag = 0;
            int child_size = cJSON_GetArraySize(child);
            for (int i=0; i< child_size; i++)
            {
                cJSON* child_child = cJSON_GetArrayItem(child, i);
                if (!child_child)
                {
                    printf("Parse Attacklist element  error\n");
                    break;
                }
                else
                {
                    //printf("child_child->valueint:%d\n", child_child->valueint);
                    if (0 == i)
                    {
                        netID = child_child->valueint;
                    }
                    else if (1 == i)
                    {
                        flow_period = child_child->valueint;
                    }
                    else if (2 == i)
                    {
                        flowMax = child_child->valueint;
                    }
                    else if (3 == i)
                    {
                        flowMin = child_child->valueint;
                        can_flow_element_find_flag = 1;
                    }
                }
            }

            if (can_flow_element_find_flag)
            {
                if (CheckIndexRange(configObj.flowCheckCnt, SK_FLOWANALY_NUM, "flow"))
                {
                    SK_FlowInit(configObj.flowCheck, configObj.flowCheckCnt, netID, flow_period, flowMax, flowMin);
                    configObj.flowCheckCnt++;
                }
            }
    	}
    }

    cJSON* j_can_id_white_list = cJSON_GetObjectItem(root,"can_id_white_list");
    if (j_can_id_white_list)
    {
    	int size = cJSON_GetArraySize(j_can_id_white_list);
    	for (int i=0; i< size; i++)
        {
    		cJSON* child = cJSON_GetArrayItem(j_can_id_white_list, i);
    		if (!child)
            {
    			printf("Parse Attacklist element  error\n");
    			break;
    		}

            can_id_white_list_element_find_flag = 0;
            int child_size = cJSON_GetArraySize(child);
            for (int i=0; i< child_size; i++)
            {
                cJSON* child_child = cJSON_GetArrayItem(child, i);
                if (!child_child)
                {
                    printf("Parse Attacklist element  error\n");
                    break;
                }
                else
                {
                    //printf("child_child->valueint:%d\n", child_child->valueint);
                    if (0 == i)
                    {
                        netID = child_child->valueint;
                    }
                    else if (1 == i)
                    {
                        canID = child_child->valueint;
                        can_id_white_list_element_find_flag = 1;
                    }
                }
            }

            if (can_id_white_list_element_find_flag)
            {
                if (CheckIndexRange(configObj.listCheckCnt, SK_WHITELIST_NUM, "list"))
                {
                    SK_ListInit(configObj.listCheck, configObj.listCheckCnt, netID, canID);
                    configObj.listCheckCnt++;
                }
            }
    	}
    }

    cJSON* j_can_length_list = cJSON_GetObjectItem(root,"can_length_list");
    if (j_can_length_list)
    {
    	int size = cJSON_GetArraySize(j_can_length_list);
    	for (int i=0; i< size; i++)
        {
    		cJSON* child = cJSON_GetArrayItem(j_can_length_list, i);
    		if (!child)
            {
    			printf("Parse Attacklist element  error\n");
    			break;
    		}

            can_length_element_find_flag = 0;
            int child_size = cJSON_GetArraySize(child);
            for (int i=0; i< child_size; i++)
            {
                cJSON* child_child = cJSON_GetArrayItem(child, i);
                if (!child_child)
                {
                    printf("Parse Attacklist element  error\n");
                    break;
                }
                else
                {
                    //printf("child_child->valueint:%d\n", child_child->valueint);
                    if (0 == i)
                    {
                        netID = child_child->valueint;
                    }
                    else if (1 == i)
                    {
                        canID = child_child->valueint;
                    }
                    else if (2 == i)
                    {
                        length = child_child->valueint;
                        can_length_element_find_flag = 1;
                    }
                }
            }

            if (can_length_element_find_flag)
            {
                if (CheckIndexRange(configObj.listCheckCnt, SK_LENCHECK_NUM, "len"))
                {
                    SK_LenInit(configObj.lenCHeck, configObj.lenCheckCnt, netID, canID, length);
                    configObj.lenCheckCnt++;
                }
            }
    	}
    }

    cJSON* j_can_period_list = cJSON_GetObjectItem(root,"can_period_list");
    if (j_can_period_list)
    {
    	int size = cJSON_GetArraySize(j_can_period_list);
    	for (int i=0; i< size; i++)
        {
    		cJSON* child = cJSON_GetArrayItem(j_can_period_list, i);
    		if (!child)
            {
    			printf("Parse Attacklist element  error\n");
    			break;
    		}

            can_period_element_find_flag = 0;
            int child_size = cJSON_GetArraySize(child);
            for (int i=0; i< child_size; i++)
            {
                cJSON* child_child = cJSON_GetArrayItem(child, i);
                if (!child_child)
                {
                    printf("Parse Attacklist element  error\n");
                    break;
                }
                else
                {
                    //printf("child_child->valueint:%d\n", child_child->valueint);
                    if (0 == i)
                    {
                        netID = child_child->valueint;
                    }
                    else if (1 == i)
                    {
                        canID = child_child->valueint;
                    }
                    else if (2 == i)
                    {
                        period_period = child_child->valueint;
                    }
                    else if (3 == i)
                    {
                        period_offset = child_child->valueint;
                        can_period_element_find_flag = 1;
                    }
                }
            }

            if (can_period_element_find_flag)
            {
                if (CheckIndexRange(configObj.prdCheckCnt, SK_PRDCHECK_NUM, "period"))
                {
                    SK_PrdInit(configObj.prdCheck, configObj.prdCheckCnt, netID, canID, period_period, period_offset);
                    configObj.prdCheckCnt++;
                }

            }
    	}
    }

    cJSON* j_can_signal_list = cJSON_GetObjectItem(root,"can_signal_analy_list");
    if (j_can_signal_list)
    {
    	int size = cJSON_GetArraySize(j_can_signal_list);
    	for (int i=0; i< size; i++)
        {
    		cJSON* child = cJSON_GetArrayItem(j_can_signal_list, i);
    		if (!child)
            {
    			printf("Parse Attacklist element  error\n");
    			break;
    		}

            can_signal_element_find_flag = 0;
            comPara_cnt = 0;
            int child_size = cJSON_GetArraySize(child);
            for (int i=0; i< child_size; i++)
            {
                cJSON* child_child = cJSON_GetArrayItem(child, i);
                if (!child_child)
                {
                    printf("Parse Attacklist element  error\n");
                    break;
                }
                else
                {
                    //printf("child_child->valueint:%d\n", child_child->valueint);
                    if (0 == i)
                    {
                        netID = child_child->valueint;
                    }
                    else if (1 == i)
                    {
                        canID = child_child->valueint;
                    }
                    else if (2 == i)
                    {
                        signal_name = child_child->valuestring;
                        //printf("signal_name:%s\n", signal_name);
                        can_signal_element_find_flag = 1;
                    }
                    else if (3 == i)
                    {
                        startBit = child_child->valueint;
                    }
                    else if (4 == i)
                    {
                        stopBit = child_child->valueint;
                    }
                    else if (5 == i)
                    {
                        dataType = child_child->valueint;
                    }
                    else if (6 == i)
                    {
                        ruleLen = child_child->valueint;
                        if (0 == ruleLen)
                        {
                            can_signal_element_find_flag = 1;
                        }
                    }
                    else if (i > 6)
                    {
                        comPara[comPara_cnt++] = child_child->valueint;
                        if (i >= ruleLen + 6)
                        {
                            can_signal_element_find_flag = 1;
                        }
                    }

                }
            }

            if (can_signal_element_find_flag)
            {
                if(CheckIndexRange(configObj.signAnalyCnt, SK_SIGNALMAX_NUM, "smg"))
                {
                    SK_SignalInit(configObj.signAnaly, configObj.signAnalyCnt, netID, canID, signal_name, startBit, stopBit, 
                        dataType, comPara, ruleLen);
                    configObj.signAnalyCnt++;
                }


            }
    	}
    }

	cJSON_Delete(root);
}

// 规则初始化
void SK_RuleInit(char *rule)
{
    SK_RuleClear();

    can_rule_parse(rule);
    //Init_FlowCheck_Config();
    //Init_ListCheck_Config();
    //Init_LenCheck_Config();
    //Init_PrdCheck_Config();
    //Init_SignalAnaly_Config();
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
	return true;
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
