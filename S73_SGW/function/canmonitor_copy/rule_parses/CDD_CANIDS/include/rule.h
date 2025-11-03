#ifndef __RULE_H__
#define __RULE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "platformtypes.h"
#include "queue.h"

// extern object
void SK_RuleInit(char *rule);
void SK_RuleClear();
bool SK_IsRuleInit();

// Analy
bool SK_Rule_FlowCheck(SK_Data_Stru data);
bool SK_Rule_LoadDisplay(uint32 flowSwitch);
bool SK_Rule_LengthCheck(SK_Data_Stru data);
bool SK_Rule_ListCheck(SK_Data_Stru data);
bool SK_Rule_PeriodCheck(SK_Data_Stru data);
bool SK_Rule_PeriodLossCheck();
bool SK_Rule_SignalAnaly(SK_Data_Stru data, uint32 msgSwitch);

#ifdef __cplusplus
}
#endif

#endif
