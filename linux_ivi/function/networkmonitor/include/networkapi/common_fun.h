#ifndef __COMMON_FUN_H__
#define __COMMON_FUN_H__
#include <stdlib.h>
#include "typedef.h"

// 3、common供底层调用，公共区
// 获取时间戳
long get_timestamp();

struct tm *get_date();

#endif