#ifndef __MSG_PARSE_H__
#define __MSG_PARSE_H__

#include "platformtypes.h"
#include "msg_config.h"
#include "rule.h"


//请求下一包策略的长度
uint8 getNextPolicyLen();

//解析配置信息,返回当前包序号
uint8 SK_ParseConfig(uint8 *data, uint8 len);

#endif //__MSG_PARSE_H__


