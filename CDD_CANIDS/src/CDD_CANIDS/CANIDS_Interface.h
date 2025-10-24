#ifndef CANIDS_INTERFACE_H
#define CANIDS_INTERFACE_H

//#include "Compiler.h"
//#include "Std_Types.h"
#include "Can_GeneralTypes.h"
#include "Can_30_Core_Cfg.h"


//初始化
extern void SK_CANIDS_Init(void);

//反初始化
extern void SK_CANIDS_DeInit(void);

//5ms周期处理函数
extern void SK_CANIDS_5ms_MainRunnable(void);

//CAN TP接收函数
extern void SK_CANIDS_EventReceiveRunnable(void);

#endif //CANIDS_INTERFACE_H