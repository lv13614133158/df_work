#ifndef __SIGNALANALY_H__
#define __SIGNALANALY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "platformtypes.h"
#include "queue.h"

#define SMG_COMPARA_MAX_SIZE  (10)

/**
 * 信号检测类型
*/ 
typedef enum _MsgType {
    SIG_TYPE_THR = 0,
    SIG_TYPE_CHA,
    SIG_TYPE_ELM,
    SIG_TYPE_TRA,
    SIG_TYPE_STA,
    SIG_TYPE_PRE,
    SIG_TYPE_SIM,
    SIG_TYPE_DIS
}MsgType;


/*
* signType: (1)信号阈值分析  (2)信号变化率分析  (3)信号枚举   (4)信号跟踪
*           (5)信号状态      (6)预置条件分析    (7)同信号分析 (8)异信号分析
* dataStart： 数据起始位置 dataEnd： 数据长度
*/
// 1阈值 
typedef struct _Value_Config{
    uint64 valueRange_Min;   
    uint64 valueRange_Max;  
    uint64 valueState; 
} Value_Config;

// 2变化率
typedef struct _Rate_Config{
    uint64 valueRange_Min;   
    uint64 valueRange_Max;
    uint64 valueLast;  
    uint64 valueRun;   
    //Time_Stru  time;
} Rate_Config;

// 3枚举, 必须在枚举范围内
typedef struct _Enum_Config{   
    uint64 enumRange[10];   
} Enum_Config;

// 4跟踪计数，按照等步进在状态内
typedef struct _Step_Config{
    uint64 valueRange_Min;   
    uint64 valueRange_Max;  
    uint64 valueSetp;
    uint64 valueLast;  
    uint64 valueRun;  
} Step_Config;

// 5状态识别，按照非等步进在状态内/6预置条件，当某种条件必须不在状态范围内
typedef struct _Stat_Config{    
    uint64 statRange[4]; 
    uint64 valueLast;  
    uint64 valueRun;   
} Stat_Config;

// 7通信号/8异信号
typedef struct _Smg_Config{    
    uint64 msgSimDis[4]; 
} Smg_Config;

/* *
* 配置文件涉及时间都为ms为单位
* 5、信号分析
* */
typedef struct _Signal_Elmt{
    uint8  netID;
    uint32 canID;
    uint8  signal_name[64];
    uint8  startBit;
    uint8  stopBit; 
    uint8  dataType;
    uint8  ruleLen;
    union{ 
        uint64 comPara[SMG_COMPARA_MAX_SIZE];  // 最大的空间,用于参数赋值 
        Value_Config thre;
        Rate_Config  rate;
        Enum_Config  enuv;
        Step_Config  step;
        Stat_Config  stat;
        Stat_Config  prec;
        Smg_Config   same;
        Smg_Config   diff;
    }rule;
}Signal_Elmt;


// Set signal status
bool SK_SmgSataus_Init(bool car, bool mode);
// Signal Analysis
bool SK_SignalAnaly(Signal_Elmt* signalElmt, SK_Data_Stru data, uint32 msgSwitch, uint32 elmtCnt);

// Specific classification
bool SK_Signal_Threshold( Signal_Elmt* smgElmt, SK_Data_Stru smgData);
bool SK_Signal_ChangeRate(Signal_Elmt* smgElmt, SK_Data_Stru smgData);
bool SK_Signal_Enumerate( Signal_Elmt* smgElmt, SK_Data_Stru smgData);
bool SK_Signal_TrackeCnt( Signal_Elmt* smgElmt, SK_Data_Stru smgData);
bool SK_Signal_StatIdent( Signal_Elmt* smgElmt, SK_Data_Stru smgData);
bool SK_Signal_PreCondit( Signal_Elmt* smgElmt, SK_Data_Stru smgData);
bool SK_Signal_Similarities(Signal_Elmt* smgElmt, SK_Data_Stru smgData);
bool SK_Signal_Differences( Signal_Elmt* smgElmt, SK_Data_Stru smgData);

// Config init
bool SK_SignalInit(Signal_Elmt* signAnaly, uint32 index, uint8 netID, uint32 canID, uint8* signal_name, uint8 startBit, uint8 stopBit,
    uint8 type, uint64* para, uint8 paraLen);

#ifdef __cplusplus
}
#endif

#endif