#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ids_config.h"
#include "log.h"
#include "platformtypes.h"
#include "idsFrame.h"
#include <stdio.h>
#include "rule.h"
#include "ctimer.h"
#include "event.h"
#include "cJSON.h"

typedef struct _IDS_Stru{
    bool flowSwitch;
    bool listSwitch;
    bool priodSwitch;
    bool lengthSwitch;
    unsigned int signaSwitch;
}IDS_Stru;

// 各个模块启动状态
static  char startFalg    = 0;   // 启动标志
static  bool flowSwitch   = 0;   // 流量检测开关/负载检测开关/doc监测开关
static  bool listSwitch   = 0;   // 白名单检测开关 
static  bool lengthSwitch = 0;   // DLC开关
static  bool priodSwitch  = 0;   // 周期检测开关
static  int  signaSwitch  = 0;   //0x100+0x20;   // 信号分析开关，8位代表8个功能


// can数据接收
uint8 SK_CANIDS_MsgReceiveIndi(uint8 netID, uint32 canID, uint8* data, uint32 len, double time)
{
    int ret = -1;
    if(startFalg)
    {
        ret = SK_Can_PushQueue(netID, canID, data, len, time);
    }   
    ret = (ret>=0);
    return ret;
}

// 时间循环，周期调用can功能函数
uint8 SK_CANIDS_5ms_Mainfunction()
{
    if(startFalg)
    {
        SK_Data_Stru canData = {0};
        // 数据出栈
        while(SK_Can_PopQueue(&canData) >= 0)
        {
            // 流量数据统计
            if(flowSwitch)
            {
                SK_Rule_FlowCheck(canData);
            }

            // 流量监测
            if(flowSwitch)
            {
                SK_Rule_LoadDisplay(flowSwitch);
            }

            //白名单检测
            if(listSwitch)
            {
                if(SK_Rule_ListCheck(canData) == false)
                    continue;
            }
            // 长度监测
            if(lengthSwitch)
            {
                if(SK_Rule_LengthCheck(canData) == false)
                    continue;
            }
            // 周期监测
            if(priodSwitch)
            {
                SK_Rule_PeriodCheck(canData);
            }
            // 信号分析监测
            if(signaSwitch)
            {
                SK_Rule_SignalAnaly(canData, signaSwitch);
            }
        }

        // 周期丢失监测
        if(priodSwitch)
        {
            SK_Rule_PeriodLossCheck();
        }
    }
   //Debug_Print(0, "[I]timeH: %d, L: %d", OS_time.sysTimeH, OS_time.sysTimeL);
}

// 框架初始化 
uint8  SK_CANIDS_Init(IDS_Stru canIDS, char *rule)
{
    Debug_Print(1, "[I]IDS init");
    flowSwitch   = canIDS.flowSwitch;
    listSwitch   = canIDS.listSwitch;
    priodSwitch  = canIDS.priodSwitch;
    lengthSwitch = canIDS.lengthSwitch;
    signaSwitch  = canIDS.signaSwitch;

	cJSON* root = NULL;
    cJSON* j_tmp_switch = NULL;

    root = cJSON_Parse(rule);
    if (root)
    {
        j_tmp_switch = cJSON_GetObjectItem(root, "can_flow_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_flow_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            flowSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_id_white_list_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_id_white_list_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            listSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_priod_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_priod_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            priodSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_length_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_length_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            lengthSwitch = j_tmp_switch->valueint;
        }

        j_tmp_switch = cJSON_GetObjectItem(root, "can_signal_switch");
        if (cJSON_IsNumber(j_tmp_switch))
        {
            printf("can_signal_switch is Number:%d!!!!!!\n", j_tmp_switch->valueint);
            signaSwitch = j_tmp_switch->valueint;
        }

        cJSON* j_loadrate_event_type = cJSON_GetObjectItem(root, "loadrate_event_type");
        cJSON* j_whitelist_event_type= cJSON_GetObjectItem(root, "whitelist_event_type");
        cJSON* j_len_event_type = cJSON_GetObjectItem(root, "len_event_type");
        cJSON* j_period_event_type = cJSON_GetObjectItem(root, "period_event_type");
        cJSON* j_signal_threshold_event_type = cJSON_GetObjectItem(root, "signal_threshold_event_type");
        cJSON* j_signal_change_rate_event_type = cJSON_GetObjectItem(root, "signal_change_rate_event_type");
        cJSON* j_signal_enumerate_event_type = cJSON_GetObjectItem(root, "signal_enumerate_event_type");
        cJSON* j_signal_stat_event_type = cJSON_GetObjectItem(root, "signal_stat_event_type");
        cJSON* j_signal_tracke_cnt_event_type = cJSON_GetObjectItem(root, "signal_tracke_cnt_event_type");
        cJSON* j_signal_relate_event_type = cJSON_GetObjectItem(root, "signal_relate_event_type");

        if (cJSON_IsString(j_loadrate_event_type) &&
            cJSON_IsString(j_whitelist_event_type) &&
            cJSON_IsString(j_len_event_type) &&
            cJSON_IsString(j_period_event_type) &&
            cJSON_IsString(j_signal_threshold_event_type) &&
            cJSON_IsString(j_signal_change_rate_event_type) &&
            cJSON_IsString(j_signal_enumerate_event_type) &&
            cJSON_IsString(j_signal_stat_event_type) &&
            cJSON_IsString(j_signal_tracke_cnt_event_type) &&
            cJSON_IsString(j_signal_relate_event_type))
        {
            init_event_type(j_loadrate_event_type->valuestring, j_whitelist_event_type->valuestring,j_len_event_type->valuestring,
                            j_period_event_type->valuestring, j_signal_threshold_event_type->valuestring, j_signal_change_rate_event_type->valuestring,
                            j_signal_enumerate_event_type->valuestring, j_signal_stat_event_type->valuestring, j_signal_tracke_cnt_event_type->valuestring,
                            j_signal_relate_event_type->valuestring);
        }
        cJSON_Delete(root);
    }

    SK_RuleInit(rule);
    SK_Can_InitQueue();
    return true;
}

// 删除
uint8  SK_CANIDS_DeInit(IDS_Stru canIDS)
{
    Debug_Print(1, "[I]IDS delect");
    SK_RuleClear();
    int SK_Can_InitQueue();
    return true;
}

// 启动
uint8  SK_CANIDS_Start(IDS_Stru canIDS)
{
    if(SK_IsRuleInit())
    {
        Debug_Print(1, "[I]IDS start");
        startFalg = true;
    } 
    else
    {
        Debug_Print(1, "[I]IDS already start");
    }
    return startFalg;
}

// 停止
uint8  SK_CANIDS_Stop(IDS_Stru canIDS)
{
    Debug_Print(1, "[I]IDS stop");
    startFalg = false;
    return startFalg;
}

int can_device_pub_dat(unsigned char netID, unsigned int canID, unsigned char* data, unsigned int len, double time)
{

	SK_CANIDS_MsgReceiveIndi(netID, canID, data, len, time);

	return 0;
}

// 控制发送周期
#define COUNT_TIME_PRT(cnt, prt) (cnt%prt==(prt-1))

// test function, 程序自己模拟can数据
void funMsg(int type)
{
    static int cnt_time = 0;
    
    // 流量检测，复制检测，doc攻击检测
    if(type & 0x01)
    {
        uint8 data[10]={1,2,3,4,5,6,7,8,9,0};
        if(COUNT_TIME_PRT(cnt_time, 10))  // 周期信号，10为周期
        {
            SK_CANIDS_MsgReceiveIndi(0x0, 0xC0, data, 8, 0);
        }
    }

    // 白名单检测，熔断机制
    if(type & 0x02)
    {
        uint8 data[10]={1,2,3,4,5,6,7,8,9,0};
        if(COUNT_TIME_PRT(cnt_time, 10)){
            SK_CANIDS_MsgReceiveIndi(0x0, 0xCE, data, 8, 0);
        }
    }

    // 长度检测
    // 长度检测目前是状态变化才变，是否需要更改
    if(type & 0x04)
    {
        uint8 data[10]={1,2,3,4,5,6,7,8,9,0};
        int len = 0;
        if(COUNT_TIME_PRT(cnt_time, 20)){
            len = rand()%9; //长度状态改变
            SK_CANIDS_MsgReceiveIndi(0x0, 0x22A, data, len, 0);
        }
    }

    // 周期检测，信号丢失
    // 目前是变化状态才上报，还有周期包丢失是否需要去掉，是否需要修改
    if(type & 0x08)
    {
        static int cnt = 0;
        static int t1 = 20, t2 =170;
        uint8 data[10]={1,2,3,4,5,6,7,8,9,0};

        if(cnt % 2000 == 1999)
        {
            int tmep = t1;
            t1 = t2;
            t2 = tmep;
        }
        
        if(cnt < 6000)// 模拟过长、过短、丢失
        {
            if(COUNT_TIME_PRT(cnt_time, t1))
                SK_CANIDS_MsgReceiveIndi(0x0, 0x32A, data, 8, 0);
            if(COUNT_TIME_PRT(cnt_time, t2))
                SK_CANIDS_MsgReceiveIndi(0x0, 0x1F5, data, 8, 0);
        }
        else if(cnt > 10000)
        {
            cnt =0;
        }
        cnt ++;
    }

    // 信号分析阈值
    if(type & 0x10)
    {
        //uint8 data[10]={0, 0, 0, 1, 0};
        uint8 data[10]={0, 0, 0, 10, 0};
        if(COUNT_TIME_PRT(cnt_time, 100))
        {
            SK_CANIDS_MsgReceiveIndi(0x0, 0x244, data, 8, 0);
            //data[0] = 1;
            //SK_CANIDS_MsgReceiveIndi(0x0, 0x244, data, 8);
        }
    }

    // 信号分析变化率
    if(type & 0x20)
    {
        static unsigned char speed = 0;
        uint8 data[10]={0, 0, 0, 1, 0};
        if(COUNT_TIME_PRT(cnt_time, 1000))
        {
            speed +=1;
            data[1] = speed;
            SK_CANIDS_MsgReceiveIndi(0x0, 0x127, data, 8, 0);
        }
    }

    // 信号分析枚举
    if(type & 0x40)
    {
        uint8 data[10]={0,0,0,10};
        if(COUNT_TIME_PRT(cnt_time, 100))
        {
            data[3] = 11;
            SK_CANIDS_MsgReceiveIndi(0x0, 0x245, data, 8, 0);
        }
    }

    // 信号跟踪
    if(type & 0x80)
    {
        static unsigned char speed = 0;
        uint8 data[10]={0,0,0,10};
        if(COUNT_TIME_PRT(cnt_time, 100))
        {
            speed += 5;
            if(speed == 95)
                speed = 0;
            data[3] += speed;
            
            SK_CANIDS_MsgReceiveIndi(0x0, 0x22C, data, 8, 0);
        }
    }

    // 状态识别
    if(type & 0x100)
    {
        static unsigned char speed = 0;
        uint8 data[10]={0, 0, 0, 10};
        uint8 poll[4]={3, 15, 8, 11};
        if(COUNT_TIME_PRT(cnt_time, 100))
        {
            if(speed++ >= 3){
                speed = 0;
            }   
            data[3] = poll[speed];
            //printf("---%d, %d\n", speed, data[3]);
            SK_CANIDS_MsgReceiveIndi(0x0, 0x1F4, data, 8, 0);
        }
    }
    
    // 预置条件
    if(type & 0x200)
    {
        uint8 data[10]={0, 0, 0, 10};
        if(COUNT_TIME_PRT(cnt_time, 100))
        {
            //data[3] = 14;
            SK_CANIDS_MsgReceiveIndi(0x0, 0x32C, data, 8, 0);
        }
    }

    // 同信号
    if(type & 0x400)
    {
        
        if(COUNT_TIME_PRT(cnt_time, 100))
        {
            uint8 data[10]={1, 3, 6};
            SK_CANIDS_MsgReceiveIndi(0x0, 0x1F3, data, 8, 0);
        }
    }

    // 异信号
    if(type & 0x800)
    {
        if(COUNT_TIME_PRT(cnt_time, 100))
        {
           uint8 data[10]={6, 8, 9};
           SK_CANIDS_MsgReceiveIndi(0x0, 0x129, data, 8, 0);
        }
    }
    cnt_time++;
}

// 时钟计数
pthread_t thread_tid;//线程ID
void *thread_time_work(void * arg)
{
    //pthread_detach(pthread_self());
	printf("Time thread running\n");
    while(1)
    {
        /*
        funMsg(0x01*flowSwitch +
               0x02*listSwitch +
               0x04*lengthSwitch +
               0x08*priodSwitch +
               0x10*signaSwitch);
               */
        // 1ms
        usleep(1000);
        SK_CANIDS_5ms_Mainfunction();
        pthread_testcancel();
    }
	pthread_exit("thanks for you cup time!\n");
}

// can 初始化, 线程创建
int can_init(int argc, char *rule)
{
    IDS_Stru canIDS = { 0 };
    
    // 框架启动
    SK_CANIDS_Init(canIDS, rule);
    SK_CANIDS_Start(canIDS);
	int ret = pthread_create(&thread_tid, NULL, thread_time_work, 0);//创建线程
	if(ret != 0){
		perror("create thread error!");
		exit(-1);
	}

    //等待子线程结束
    void *thread_rel;
	//pthread_join(thread_tid, &thread_rel);
    return 0;
}

// 退出
void cleanHandler()
{
    IDS_Stru canIDS;
    // can驱动停止
#ifndef USED_MCU_TYPE  
    can_stop();
#endif
    pthread_cancel(thread_tid);
    SK_CANIDS_Stop(canIDS);
    SK_CANIDS_DeInit(canIDS);
    log_debug(LOG_INFO, "[I]can IDS module stop");
}
