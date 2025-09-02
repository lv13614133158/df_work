#include "DfsdkAPIC.h"
#include <unistd.h>
#include <stdio.h>


void gnss_callback(struct SdkGnssInfo gnss)
{
    printf("location update:latitude(%f),longitude(%f),altitude(%f)\n", gnss.latitude, gnss.longitude, gnss.altitude);
}

int main(int argc, char *argv[])
{

    if (dfsdkapi_init("Dfsdk"))
    {
        printf("init client fail\n");
        return -1;
    }
    
    dfsdkapi_setGnssNotify(gnss_callback);
    while(1)
    {
        sleep(1);
        printf("sleep(1)\n");
    }
    return 0 ;
}