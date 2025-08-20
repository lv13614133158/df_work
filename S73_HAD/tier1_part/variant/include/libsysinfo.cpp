#include <cstring>
#include "libsysinfo.hpp"
#include "libsysinfo.h"

VariantSysInfoClient client;
void libsysinfo_Init()
{
   client.Init();
}
void libsysinfo_Deinit(){
    client.Deinit();
}
const char* GetVinDataIdentifier(int *len)
{
    *len = client.GetVinDataIdentifier().length();
    char* charbuffer = new char[*len];
    memcpy(charbuffer, client.GetVinDataIdentifier().c_str(), *len);
    return charbuffer;
}
const char* GetEcuSerialNumber(int *len){
    *len = client.GetEcuSerialNumber().length();
    char* charbuffer = new char[*len];
    memcpy(charbuffer, client.GetEcuSerialNumber().c_str(), *len);
    return charbuffer;
}
const char* GetProductSerialNumber(int *len)
{
    *len = client.GetProductSerialNumber().length();
    char*   charbuffer = new char[*len];
    memcpy(charbuffer, client.GetProductSerialNumber().c_str(), *len);
    return charbuffer;
}
const char* GetSystemSupplierIdentifier(int *len){
    *len = client.GetSystemSupplierIdentifier().length();
    char*    charbuffer = new char[*len];
    memcpy(charbuffer, client.GetSystemSupplierIdentifier().c_str(), *len);
    return charbuffer;
}         
const char* GetSystemSupplierECUHardwareNumber(int *len){
    *len = client.GetSystemSupplierECUHardwareNumber().length();
    char*   charbuffer = new char[*len];
    memcpy(charbuffer, client.GetSystemSupplierECUHardwareNumber().c_str(), *len);
    return charbuffer;
} 
const char* GetSystemSupplierECUSoftwareNumber(int *len){
    *len = client.GetSystemSupplierECUSoftwareNumber().length();
    char*    charbuffer = new char[*len];
    memcpy(charbuffer, client.GetSystemSupplierECUSoftwareNumber().c_str(), *len);
    return charbuffer;
} 
const char* GetOEMHardwareVersion(int *len){
    *len = client.GetOEMHardwareVersion().length();
    char*    charbuffer = new char[*len];
    memcpy(charbuffer, client.GetOEMHardwareVersion().c_str(), *len);
    return charbuffer;
}               
const char* GetOEMSoftwareVersion(int *len)
{
    *len = client.GetOEMSoftwareVersion().length();
    char* charbuffer = new char[*len];
    memcpy(charbuffer, client.GetOEMSoftwareVersion().c_str(), *len);
    return charbuffer;
}   