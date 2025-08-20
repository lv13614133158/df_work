#ifndef _IDPS_READ_INFO_H_
#define _IDPS_READ_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TboxMcuDeviceInfo
{
    char vin[24];
    char sn[128];
	char supplierInfo[128];
	char hardwareVersion[32];
	char softwareVersion[32];
}tbox_mcu_info_t;

typedef void(*tbox_info_get_gps_callback)(double latitude, double longitude);

int tbox_info_get_mcu_info(tbox_mcu_info_t *p_tbox_mcu_info, tbox_info_get_gps_callback get_gps_callback);

#ifdef __cplusplus
}
#endif
#endif