#ifndef _DfsdkAPIC_HPP_
#define _DfsdkAPIC_HPP_
#include <stdbool.h> 
struct SdkGnssInfo
{
    double latitude ; //weidu
    double longitude ; //jingdu
    double altitude ; //haiba
    bool valid ;
};
int dfsdkapi_init(const char *clientname);
int dfsdkapi_setGnssNotify(void (*callback)(struct SdkGnssInfo gnss));

#endif