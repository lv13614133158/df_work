#ifndef _IDPS_READ_INFO_H_
#define _IDPS_READ_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SgwMcuDeviceInfo
{
    char vin[24];
    char sn[128];
	char supplierInfo[128];
	char hardwareVersion[32];
	char softwareVersion[32];
}sgw_mcu_info_t;

int sgw_info_get_mcu_info(sgw_mcu_info_t *p_sgw_mcu_info);

#ifdef __cplusplus
}
#endif
#endif