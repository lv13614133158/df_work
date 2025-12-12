
/*
*	Synchronize the server clock to get the current timestamp
*/
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include "clocker.h"

static long long oldtime = 0;
static long long time1,time2;
static int flag = 0;
static pthread_mutex_t clockerMutex = PTHREAD_MUTEX_INITIALIZER;

//获取系统时间，精确到ms
static long long get_local_time(void)
{
	struct timespec temp = {0};
	long long timestamp;

	/*此时间为系统启动时间，不受外界影响*/
	clock_gettime(CLOCK_MONOTONIC_RAW, &temp);
	timestamp = (long long)1000*temp.tv_sec + temp.tv_nsec/1000000;

	return timestamp;
}


/*  @brief Synchronous clock
*	@param time:timestamp
*	@return  No return value
*/
void sync_clock( long long time)
{
	oldtime = time;	
	time1 = get_local_time();
	flag = 1;
}
/*	
*	@brief Get the current timestamp
*	@param 
*	@return Return long integer timestamp
*
*/
long long get_current_time()
{
	long long ret;
	pthread_mutex_lock(&clockerMutex);
	time2 = get_local_time();
	if(flag != 1)
	{
		pthread_mutex_unlock(&clockerMutex);
		ret = time2;
		return ret;
	}	
	ret = time2-time1+oldtime; //ms
	pthread_mutex_unlock(&clockerMutex);
	return ret;	
}
