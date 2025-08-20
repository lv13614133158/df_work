/**
 * 文件名: signalanaly.c
 * 作者: ljk
 * 创建时间: 2023-08-03
 * 文件描述: 信号分析处理
 */
#include <stdlib.h>
#include <stdio.h>
#include "signalanaly.h"
#include "event.h"
#include "log.h"
#include "fusing.h"
#include <string.h>

#define THRESHOLD_FUSING_TIME  (10*1000)  // 熔断时间
#define THRESHOLD_FUSING_NUM   (100)      // 熔断次数上限
// 熔断
static Fusing_Stru thresholdFusing = {
    .fusing_num = THRESHOLD_FUSING_NUM, 
    .fusing_count_time = THRESHOLD_FUSING_TIME,
    .fusing_delay_time = THRESHOLD_FUSING_TIME
};


// 整byte字节数字组合，低位在前
static uint64 ReConver(uint8* num, uint8 len)
{
    uint64 ret = 0;
    if(num)
    {
        int l = SK_MIN(len,8);
        for(int i=0; i<l; i++){
            ret *= 256;
            ret += num[l-1-i];
        }
    }
    return ret;
} 

// 整byte字节数字组合,高位在前
static uint64 Conver(uint8* num, uint8 len)
{
    uint64 ret = 0;
    if(num)
    {
        for(int i=0; i<SK_MIN(len,8); i++){
            ret *= 256;
            ret += num[i];
        }
    }
    return ret;
}

// 按bit位转换 目前仅支持8个字节64位
// can 接收字节7在高位，字节0在低位，接收的字节内低位组成数字在低，高位在高
static uint64 combineBytes64(uint8* num, uint8 startBit, uint8 stopBit)
{
    uint64 ret = 0;
    if(num && (stopBit>=startBit))
    {
        for(int i=stopBit; i>=startBit; i--){
            ret = (ret << 1) + (num[i/8]>>(i%8) & 0x01);
        }
    }
    return ret;
}

// 寻找报文的值是否存在rule配置文件
static int findPosIndex(Signal_Elmt smgElmt, uint64 value)
{
    for(int i=0; i<smgElmt.ruleLen; i++){
        //Debug_Print(0, "%d:%d\n", smgElmt.rule.comPara[i], value);
        if(smgElmt.rule.comPara[i] == value){
            return i;
        }
    }
    return -1;
}


// 1信号阈值分析
// valueRange 0： 正常 1：低于阈值 2：高于阈值
bool SK_Signal_Threshold(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    char slog[255] = {0};
    bool ret = true;
    uint64 temp = combineBytes64(smgData.data, smgElmt->startBit, smgElmt->stopBit);
    //Debug_Print(0, "[Sign TH debug]: %ld", temp);

    if(temp > smgElmt->rule.thre.valueRange_Max)
    {
        if(smgElmt->rule.thre.valueState == 2 || SK_Getfusing(&thresholdFusing) )
            return false;
        sprintf(slog, "The signal value exceeds the maximum threshold!, Normal range value:[%llu~%llu] The current value:%llu, value data start byte:%d byte length:%d", 
            smgElmt->rule.thre.valueRange_Min, smgElmt->rule.thre.valueRange_Max, (uint64)temp, smgElmt->startBit, smgElmt->stopBit);
        //Event_Print(EVENT_LEVEL_NOPASS, SK_SMG_THRESHOLD_MAX_EVENT, smgData.netID, smgData.canID, slog);
        signal_threshold_event_update(EVENT_LEVEL_NOPASS, SK_SMG_THRESHOLD_MAX_EVENT, smgData.data_time, smgData.netID, smgData.canID,
                                      smgElmt->signal_name, 1, smgData.data, smgData.len);
        smgElmt->rule.thre.valueState = 2;
        ret = false;
    }
    else if(temp < smgElmt->rule.thre.valueRange_Min)
    {
        if( smgElmt->rule.thre.valueState == 1 || SK_Getfusing(&thresholdFusing) )
            return false;
        sprintf(slog, "The signal value exceeds the minimum threshold!, Normal range value:[%llu~%llu] The current value:%llu, value data start byte:%d byte length:%d", 
            smgElmt->rule.thre.valueRange_Min, smgElmt->rule.thre.valueRange_Max, temp, smgElmt->startBit, smgElmt->stopBit);
        //Event_Print(EVENT_LEVEL_NOPASS, SK_SMG_THRESHOLD_MIN_EVENT, smgData.netID, smgData.canID, slog);
        signal_threshold_event_update(EVENT_LEVEL_NOPASS, SK_SMG_THRESHOLD_MAX_EVENT, smgData.data_time, smgData.netID, smgData.canID,
                                      smgElmt->signal_name, 0, smgData.data, smgData.len);
        smgElmt->rule.thre.valueState = 1;
        ret = false;
    }
    else
    {
        smgElmt->rule.thre.valueState = 0;
        //Event_Print(EVENT_LEVEL_PASS,  0x8600, data.netID, data.canID, "The signal threshold value Pass!");
    }
    
    return true;
}

// 2信号变化率
bool SK_Signal_ChangeRate(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    bool ret = true;
    uint64 temp = combineBytes64(smgData.data, smgElmt->startBit, smgElmt->stopBit);

    if(smgElmt->rule.rate.valueRun)
    {
        //uint32 time = Get_RelCtime(smgElmt->rule.rate.time, smgData.time);
        //double rate = abs((temp - smgElmt->rule.rate.valueLast))/SK_MAX(time, 1); //防止溢出
        //Debug_Print(0, "[Sign CH debug]: %lld, %lld, time%d, rate%f, [%lld~%lld]\n", temp, smgElmt->rule.rate.valueLast, time, rate, smgElmt->rule.rate.valueRange_Min, smgElmt->rule.rate.valueRange_Max);

        uint64 rate = abs(temp - smgElmt->rule.rate.valueLast);
        //printf("rate:%llu, temp:%llu, valueLast:%llu\n", rate, temp, smgElmt->rule.rate.valueLast);
        if(rate > smgElmt->rule.rate.valueRange_Max)
        {
            //Event_Print(EVENT_LEVEL_NOPASS, 0x8701, smgData.netID, smgData.canID, "The signal change exceeds the maximum rate!\n");
            signal_changeRate_event_update(EVENT_LEVEL_NOPASS, 0x8701, smgData.data_time ,smgData.netID, smgData.canID, smgElmt->signal_name,
                                           rate, 1, smgData.data, smgData.len);
            ret = false;
        }
        else if(rate < smgElmt->rule.rate.valueRange_Min)
        {
            //Event_Print(EVENT_LEVEL_NOPASS, 0x8702, smgData.netID, smgData.canID, "The signal change exceeds the minimum rate!\n");
            signal_changeRate_event_update(EVENT_LEVEL_NOPASS, 0x8702, smgData.data_time ,smgData.netID, smgData.canID, smgElmt->signal_name,
                                           rate, 0, smgData.data, smgData.len);
            ret = false;
        }
        else
        {
            //Event_Print(EVENT_LEVEL_PASS,  0x8700, smgData.netID, smgData.canID, "Pass!");
        }
        
        //Set_Count(&smgElmt->rule.rate.time, smgData.time);
        smgElmt->rule.rate.valueLast = temp;
    }
    else
    {
        // 计时开始
        //Set_Count(&smgElmt->rule.rate.time, smgData.time);
        smgElmt->rule.rate.valueLast = temp;
        smgElmt->rule.rate.valueRun  = true;
    }
    return true;
}

// 3信号枚举
bool SK_Signal_Enumerate(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    uint64 temp = combineBytes64(smgData.data, smgElmt->startBit, smgElmt->stopBit);
    //printf("temp:%llu\n", temp);
    int pos = findPosIndex(*smgElmt, temp);

    if( pos < 0 ){
        //Event_Print(EVENT_LEVEL_NOPASS, 0x8801, smgData.netID, smgData.canID, "The signal undefined enumeration!");
        signal_enumerate_event_update(EVENT_LEVEL_NOPASS, 0x8801, smgData.data_time, smgData.netID, smgData.canID, smgElmt->signal_name,
                                       temp, smgData.data, smgData.len);
        return false;
    }

    return true;
}

// 4信号跟踪计数
bool SK_Signal_TrackeCnt(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    bool ret = false;
    //uint64 temp = Conver((smgData.data+smgElmt->startBit), smgElmt->stopBit);
    uint64 temp = combineBytes64(smgData.data, smgElmt->startBit, smgElmt->stopBit);

    if( (temp - smgElmt->rule.step.valueRange_Min) % smgElmt->rule.step.valueSetp ){
        //Event_Print(EVENT_LEVEL_NOPASS, 0x8902, smgData.netID, smgData.canID, "The signal state not in state range!");
        signal_trackeCnt_event_update(EVENT_LEVEL_NOPASS, 0x8902, smgData.data_time, smgData.netID, smgData.canID, smgElmt->signal_name,
                                    temp, 0, smgData.data, smgData.len);
        return ret;
    }
    return ret;

    if(smgElmt->rule.step.valueRun)
    {
        if((smgElmt->rule.step.valueLast + smgElmt->rule.step.valueSetp) == temp){
            Event_Print(EVENT_LEVEL_PASS, 0x8900, smgData.netID, smgData.canID, "Pass!");
        }
        else if( ((smgElmt->rule.step.valueLast + smgElmt->rule.step.valueSetp) > smgElmt->rule.step.valueRange_Max) && (smgElmt->rule.step.valueRange_Min == temp) ){
            Event_Print(EVENT_LEVEL_PASS, 0x8900, smgData.netID, smgData.canID, "Pass!");
        }
        else{
            Event_Print(EVENT_LEVEL_NOPASS, 0x8901, smgData.netID, smgData.canID, "The signal state abnormal!");
            ret = false;
        }
        //Debug_Print(0, "%d:%d\n",smgElmt->rule.step.valueLast,temp);
        smgElmt->rule.step.valueLast = temp;
    }
    else{
        smgElmt->rule.step.valueRun = true;
        smgElmt->rule.step.valueLast = temp;
    }
        
    return ret;
}

// 5信号状态识别
bool SK_Signal_StatIdent(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    bool ret = false;
    //uint64 temp = Conver((smgData.data+smgElmt->startBit), smgElmt->stopBit);
    uint64 temp = combineBytes64(smgData.data, smgElmt->startBit, smgElmt->stopBit);
    //printf("temp:%d\n", temp);
    int pos = findPosIndex(*smgElmt, temp);

    if( pos < 0 ){
        //Event_Print(EVENT_LEVEL_NOPASS, 0x8C02, smgData.netID, smgData.canID, "The signal state not in state range!");
        signal_stat_event_update(EVENT_LEVEL_NOPASS, 0x8C02, smgData.data_time, smgData.netID, smgData.canID, smgElmt->signal_name,
                                    temp, 0, smgData.data, smgData.len);
        return ret;
    }
    return ret;

    if(smgElmt->rule.stat.valueRun)
    {
        if( (pos==smgElmt->rule.stat.valueLast+1) ||
        (smgElmt->rule.stat.valueLast == (smgElmt->ruleLen-1) && pos==0)){
            ret = true;
        }
        if(!ret){
            Event_Print(EVENT_LEVEL_NOPASS, 0x8C01, smgData.netID, smgData.canID, "The signal state abnormal!");
        }
        else{
            Event_Print(EVENT_LEVEL_PASS,  0x8C00, smgData.netID, smgData.canID, "Pass!");
        }
        //Debug_Print(0, "%d:%d\n\n\n", smgElmt->rule.stat.valueLast, pos);
        smgElmt->rule.stat.valueLast = pos;
    }
    else{
        smgElmt->rule.stat.valueRun = true;
        smgElmt->rule.stat.valueLast = pos;
    }
        
    return ret;
}

// 6汽车启动标志,预置条件
bool SK_SmgSataus_Init(bool car, bool mode) 
{
    static bool carRunFlag = true;

    if(mode){
        carRunFlag = car;
    }
    return carRunFlag;
}

// 信号预置条件，目前手动改
bool SK_Signal_PreCondit(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    /*关联监控*/
    uint64 temp1 = combineBytes64(smgData.data, smgElmt->startBit, smgElmt->stopBit);
    int pos1 = findPosIndex(*smgElmt, temp1);
    if(pos1 >= 0 && SK_SmgSataus_Init(0, 0))
    {
        Event_Print(EVENT_LEVEL_NOPASS, 0x8D01, smgData.netID, smgData.canID, "The signal pre status abnormal!");
        signal_relate_event_update(EVENT_LEVEL_NOPASS, 0x8D01, smgData.data_time, smgData.netID, smgData.canID,
                                    smgElmt->signal_name, temp1, smgData.data, smgData.len,
                                     smgElmt->signal_name, temp1, smgData.data, smgData.len);
        return false;
    }
    else
    {
        //Event_Print(EVENT_LEVEL_PASS,  0x8D00, smgData.netID, smgData.canID, "Pass!");
    }
    
    return true;
    /*关联监控*/

    uint64 temp = Conver((smgData.data+smgElmt->startBit), smgElmt->stopBit);
    int pos = findPosIndex(*smgElmt, temp);
    if(pos >= 0 && SK_SmgSataus_Init(0, 0))
    {
        Event_Print(EVENT_LEVEL_NOPASS, 0x8D01, smgData.netID, smgData.canID, "The signal pre status abnormal!");
        return false;
    }
    else
    {
        Event_Print(EVENT_LEVEL_PASS,  0x8D00, smgData.netID, smgData.canID, "Pass!");
    }
    
    return true;
}

// 7同信号
bool SK_Signal_Similarities(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    uint32 simCnt = 0;
    for(int i=0; i< smgElmt->ruleLen; i++)
    {
        if(smgData.data[smgElmt->startBit+i] == smgElmt->rule.same.msgSimDis[i]){
            simCnt++;
        }
    }
    if(simCnt != smgElmt->ruleLen)
    {
        Event_Print(EVENT_LEVEL_NOPASS, 0x8A01, smgData.netID, smgData.canID, "The synchronization signal abnormal!");
        return false;
    }
    else
    {
        Event_Print(EVENT_LEVEL_PASS,  0x8A00, smgData.netID, smgData.canID, "Pass!");
    }
    return true;
}

// 8异信号
bool SK_Signal_Differences(Signal_Elmt* smgElmt, SK_Data_Stru smgData)
{
    uint32 disCnt = 0;
    for(int i=0; i< smgElmt->ruleLen; i++)
    {
        if(smgData.data[smgElmt->startBit+i] != smgElmt->rule.diff.msgSimDis[i]){
            disCnt++;
        }
    }
    if(disCnt != smgElmt->ruleLen)
    {
        Event_Print(EVENT_LEVEL_NOPASS, 0x8B01, smgData.netID, smgData.canID, "The asynchronous signal abnormal!");
        return false;
    }
    else
    {
        Event_Print(EVENT_LEVEL_PASS,  0x8B00, smgData.netID, smgData.canID, "Pass!");
    }
    return true;
}

// 分类调用检测
static void SK_SortSignIndex(uint8 type, Signal_Elmt* smgElmt, SK_Data_Stru data)
{
    switch (type)
    {
    case SIG_TYPE_THR:
        SK_Signal_Threshold(smgElmt, data);
        break;
    
    case SIG_TYPE_CHA:
        SK_Signal_ChangeRate(smgElmt, data);
        break;

    case SIG_TYPE_ELM:
        SK_Signal_Enumerate(smgElmt, data);
        break;

    case SIG_TYPE_TRA:
        SK_Signal_TrackeCnt(smgElmt, data);
        break;

    case SIG_TYPE_STA:
        SK_Signal_StatIdent(smgElmt, data);
        break;

    case SIG_TYPE_PRE:
        SK_Signal_PreCondit(smgElmt, data);
        break;

    case SIG_TYPE_SIM:
        SK_Signal_Similarities(smgElmt, data);
        break;

    case SIG_TYPE_DIS:
        SK_Signal_Differences(smgElmt, data);
        break;

    default:
        /* code */
        break;
    }
}

// 信号分析 msgSwitch 8位8种信号分析开关
bool SK_SignalAnaly(Signal_Elmt* signalElmt, SK_Data_Stru data, uint32 msgSwitch, uint32 elmtCnt)
{
    if(!msgSwitch) {
        return false;
    }

    for(int i=0; i<elmtCnt; i++)
    {
        if(signalElmt[i].netID==data.netID && signalElmt[i].canID==data.canID)
        {
            uint8 type = signalElmt[i].dataType-SIG_TYPE_THR;
            if(SK_CHECKBIT(msgSwitch, type)){
                SK_SortSignIndex(type, &signalElmt[i], data);
            }
        }
    }
    return true;
}


// 信号分析配置参数填充
static void SignalCom_Init(Signal_Elmt* signAnaly, uint32 index, uint8 netID, uint32 canID, uint8 startBit, uint8 stopBit)
{
    signAnaly[index].netID = netID;
    signAnaly[index].canID = canID;
    signAnaly[index].startBit = startBit;
    signAnaly[index].stopBit  = stopBit;
}

// 信号分析配置参数填充
static void SignalDef_Init(Signal_Elmt* signAnaly, uint32 index, uint8 type, uint64* para, uint8 paraLen, uint8* signal_name)
{
    signAnaly[index].dataType = type;
    signAnaly[index].ruleLen  = paraLen;
    strncpy(signAnaly[index].signal_name, signal_name, sizeof(signAnaly[index].signal_name) - 1);
    for(int i=0; i<SMG_COMPARA_MAX_SIZE; i++)
    {
        if(i< paraLen){
            signAnaly[index].rule.comPara[i] = para[i];
        }
        else{
             signAnaly[index].rule.comPara[i] = 0;
        }
    }
   
    if(type == SIG_TYPE_CHA)
    {
        //Init_Count(&signAnaly[index].rule.rate.time);
    }
}

bool SK_SignalInit(Signal_Elmt* signAnaly, uint32 index, uint8 netID, uint32 canID, uint8* signal_name, uint8 startBit, uint8 stopBit,
    uint8 type, uint64* para, uint8 paraLen)
{
	SignalCom_Init(signAnaly, index, netID, canID, startBit, stopBit);
	SignalDef_Init(signAnaly, index, type,  para,  paraLen, signal_name);
	return true;
}