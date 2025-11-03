#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ids_config.h"
#include "log.h"
#include "platformtypes.h"
#include "canmanage.h"
#include "idsFrame.h"
#include "can_udp_fun.h"
#include "can_parser.h"
static void *can_connect_task(void *arg)
{
	pthread_detach(pthread_self());
#if 0
    
        CAN_DATA_INFO_T outputData[128];
    
        while (1)
        {
            char buf[10240] = {0};
            int len = 0;
    
            static FILE *fp = NULL;
            if (!fp)
            {
                if ((fp = fopen("./can_data_temp", "r")) != NULL)
                {
                    len = fread(buf, 1, 1024, fp);
                }
            }
            else
            {
                len = fread(buf, 1, 1024, fp);
            }
    
            printf("recv len:%d\n", len);
    
            if (len <= 0)
            {
                sleep(1000);
            }
            sleep(1);

            int numData = can_parse_data(buf, len, outputData);
            if (numData < 0)
            {
                printf("解析失败：错误码 %d\n", numData);
            }
            else
            {
                printf("解析成功：共解析到 %d 条数据\n", numData);
              
                for (int i = 0; i < numData; i++) {
                    printf("时间戳：%u, CAN ID：%u, 方向：%u, 通道：%u, Payload：", outputData[i].time_stamps, outputData[i].canid,
                           outputData[i].direction, outputData[i].channel);
                    for (int j = 0; j < outputData[i].payload_length; j++) {
                        printf("%02X ", outputData[i].payload[j]);
                    }
                    printf("\n");
               
               
    
        }
#else
    can_udp_server();
#endif
}

void initCanConnect(char* rule)
{
	pthread_t pthread_can_connect;

    // 日志初始化
    log_init();
    log_debug(LOG_INFO, "[I]can IDS module start");
    
    // ids初始化
    can_init(0, rule);

	pthread_create(&pthread_can_connect, NULL, can_connect_task, NULL);
}
