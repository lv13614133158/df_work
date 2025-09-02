#include <stdio.h>
#include "common.h"
#include "runtimemanager.h"


static long long timestamp_now  = 0;
static long long timestamp_init = 0;
static long long timestamp_total = 0;
static bool runflag = false;


// 获取当前时间
long long getLocalTime()
{
    long long _ltimestamp = 0;
	//_ltimestamp = clockobj.get_current_time();
    _ltimestamp = getOSTime();
    return _ltimestamp;
}

void initRunTime()
{
    timestamp_init = getLocalTime();
    runflag = true;
}

void stopRunTime()
{
    // 更新保存最后的时间
    getRunTime();
    runflag = false;
}

// 校正IDPS运行时间
void setRunTime(long long timeValue)
{
    timestamp_total = timeValue;
    timestamp_init  = getLocalTime();
}

// 获取IDPS运行时间
long long getRunTime()
{
    long long timestamp_now = getLocalTime();
    if(runflag)
    {
        timestamp_total += (timestamp_now - timestamp_init);
        timestamp_init = timestamp_now;
    }
    return timestamp_total;
}

// 获取OS启动时间
long long getOSTime()
{
    long long _ltime = 0;
    char szline[1024] = {0};
    FILE* fp = fopen("/proc/uptime", "r");//打开系统文件查看
	if(!fp)
	{
		perror("fopen /proc/uptime");
		return -1;
	}
	// 获取运行时间和空闲时间
	char* result = fgets(szline, sizeof(szline), fp);
    fclose(fp);
    // printf("/proc/uptime:%s\n",  szline);
    _ltime = atoi(szline);
    return _ltime;
}

//#define RUNTIME_TEST 1
// 模块测试
#include <unistd.h>
int delay_runtime_s(int keep)
{
    static int cnt = 0;
    sleep(keep);
    cnt += keep;
    printf("sleep:+%d, runtime:%lld, Forecasttime:%d\n", keep, getRunTime(), cnt);
    return cnt;
}

#ifdef RUNTIME_TEST
int main()
#else
int runtime_test()
#endif
{
    int timeout = 0;
    printf("OS up time:%lld\n",getOSTime());
    initRunTime();
    // 测试断续时间
    for(int i=0, j=0; i<3; i++)
    {
        delay_runtime_s(5);
        delay_runtime_s(10);
        delay_runtime_s(2);
        delay_runtime_s(1);
        delay_runtime_s(3);
        delay_runtime_s(4);
        stopRunTime();
        printf("stop 20s\n");
        sleep(20);
        initRunTime();
        delay_runtime_s(7);
        delay_runtime_s(1);
	}

    timeout = delay_runtime_s(1);
    // 测试更改系统时间
    for(int i=0; i<1000; i++){
        sleep(1);
        printf("Uptime:%d, Forecasttime:%d\n",getRunTime(), ++timeout); 
    }
}
