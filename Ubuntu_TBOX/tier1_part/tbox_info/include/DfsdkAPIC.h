#ifndef _DfsdkAPIC_HPP_
#define _DfsdkAPIC_HPP_
struct SdkGnssInfo
{
    double latitude ; //weidu
    double longitude ; //jingdu
    double altitude ; //haiba
};
int dfsdkapi_init(const char *clientname);
int dfsdkapi_setGnssNotify(void (*callback)(struct SdkGnssInfo gnss));

#endif