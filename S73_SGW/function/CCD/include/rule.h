#ifndef __RULE_H__
#define __RULE_H__

#include "platformtypes.h"
#include "queue.h"

// extern object
void SK_RuleInit();
void SK_RuleClear();
bool SK_IsRuleInit();

// rule switch
void setFlowSwitch(uint8 flowSwitch);
uint8 getFlowSwitch();
void setListSwitch(uint8 listSwitch);
uint8 getListSwitch();
void setLengthSwitch(uint8 lengthSwitch);
uint8 getLengthSwitch();
void setPeriodSwitch(uint8 periodSwitch);
uint8 getPeriodSwitch();
void setAnalySwitch(uint8 analySwitch);
uint8 getAnalySwitch();
void setDiagSwitch(uint8 diagSwitch);
uint8 getDiagSwitch();

// update rule
bool SK_StaticRule_Clear();
bool SK_InsertRule_Flow(uint32 num, uint8 netID, uint32 period, uint32 flowMax, uint32 flowMin, uint32 loadrateThreshold);
bool SK_InsertRule_List(uint32 num, uint8 netID, uint32 canID);
bool SK_InsertRule_Length(uint32 num, uint8 netID, uint32 canID, uint8 length);
bool SK_InsertRule_Period(uint32 num, uint8 netID, uint32 canID, uint32 period, uint32 offset);
bool SK_InsertRule_Signal(uint32 num, uint8 netID, uint32 canID, uint8 startBit, uint8 stopBit, uint8 dataType, uint8 ruleLen, uint32* comPara);
bool SK_InsertRule_Diag(uint32 num, uint8 netID, uint32 canID, uint8 sesSwitch, uint8 resetSwitch, uint8 accSwitch);


// Analy
bool SK_Rule_FlowCheck(SK_Data_Stru data);
bool SK_Rule_LoadDisplay(uint32 flowSwitch);
bool SK_Rule_LengthCheck(SK_Data_Stru data);
bool SK_Rule_ListCheck(SK_Data_Stru data);
bool SK_Rule_PeriodCheck(SK_Data_Stru data);
bool SK_Rule_PeriodLossCheck();
bool SK_Rule_SignalAnaly(SK_Data_Stru data, uint32 msgSwitch);
bool SK_Rule_DiagCheck(SK_Data_Stru data);


#endif
