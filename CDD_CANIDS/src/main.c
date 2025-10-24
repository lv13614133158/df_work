#include <stdio.h> 
#include <string.h>  
#include <time.h>   
#include <stdlib.h>
#include <CANIDS_Interface.h>
#include <pthread.h>
#include <stdint.h>

void* can_thread_func(void* arg) {
    while (1)
    {
        SK_CANIDS_5ms_MainRunnable();
        sleep(5);
    }


    
    return NULL;
}


void * send_thread_func(void * arg)
{
    int Controller=0;
    uint ID=0x123;


    uint8_t DataPtr[5][8]={{0x02,0x10,0x04,00,00,00,00,00},\
                            {0x02,0x11,0x01,00,00,00,00,00},\
                            {0x02,0x27,0x01,00,00,00,00,00},\
                            {0x03,0x7f,0x27,0x12,00,00,00,00},\
                            {0x01,0x11,0x11,0x11,00,00,00,00}};
    uint8_t DataLength = 8;

    SK_CANIDS_MsgReceiveIndi(Controller, ID, (uint8_t*)DataPtr[0], DataLength);
    SK_CANIDS_MsgReceiveIndi(Controller, ID, (uint8_t*)DataPtr[1], DataLength);
    for(int i = 0; i < 3; i++)
    {
    SK_CANIDS_MsgReceiveIndi(Controller, ID, (uint8_t*)DataPtr[2], DataLength);
    }
    SK_CANIDS_MsgReceiveIndi(Controller, ID, (uint8_t*)DataPtr[3], DataLength);
    for (size_t i = 0; i < 20; i++)
    {
        SK_CANIDS_MsgReceiveIndi(Controller, ID, (uint8_t*)DataPtr[4], DataLength);
    }
    
    
}

void main()
{
    SK_CANIDS_Init();
    SK_CANIDS_Start();
    pthread_t can_thread;
    pthread_t send_thread;
 

    if (pthread_create(&can_thread, NULL, can_thread_func, NULL) != 0) {
        perror("Failed to create detection thread");
        return;
    }

    if (pthread_create(&send_thread, NULL, send_thread_func, NULL) != 0) {
        perror("Failed to create detection thread");
        return;
    }
    pthread_join(can_thread, NULL);
    pthread_join(send_thread, NULL);
    SK_CANIDS_DeInit();
}