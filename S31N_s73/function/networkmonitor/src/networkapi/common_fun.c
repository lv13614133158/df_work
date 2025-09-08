#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "typedef.h"
#include "common_fun.h"

// 3、common供底层调用，公共区
static struct tm nowTime;
struct tm *get_date()
{
	struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
	localtime_r(&time.tv_sec, &nowTime);	
	return &nowTime;
}

long get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

//local check ip
bool checkattack(s8* ip){
	static s8 _local_buffer[3][32] = {0};
	static s8 _localloop = 0;
	if(ip != NULL){
		memset(_local_buffer[_localloop],0,32);
		memcpy(_local_buffer[_localloop],ip,strlen(ip));
		_localloop = (_localloop + 1)%3;
		if((strcmp(_local_buffer[0],_local_buffer[1]) == 0)&&(strcmp(_local_buffer[1],_local_buffer[2]) == 0)){
			return true;
		}
		else{
			return false;
		}
	}
	return false;
}