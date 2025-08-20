// libsysinfo_c.h
#ifndef LIBSYSINFO_C_H
#define LIBSYSINFO_C_H

#ifdef __cplusplus
extern "C" {
#endif

void libsysinfo_Init();
void libsysinfo_Deinit();
const char* GetVinDataIdentifier(int *len);  // C接口声明
const char* GetEcuSerialNumber(int *len);
const char* GetProductSerialNumber(int *len);              
const char* GetSystemSupplierIdentifier(int *len);         
const char* GetSystemSupplierECUHardwareNumber(int *len);  
const char* GetSystemSupplierECUSoftwareNumber(int *len);  
const char* GetOEMHardwareVersion(int *len);               
const char* GetOEMSoftwareVersion(int *len);             

#ifdef __cplusplus
}
#endif
#endif