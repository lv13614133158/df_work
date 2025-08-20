#include <stdio.h>
#include "GlobalParameterDefinition.h"
#include <signal.h>
#include <unistd.h>
#include "nvRamApi.h"
#include "idpsReadInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

static tbox_info_get_gps_callback s_get_gps_callback_fun = NULL;

/// GPS信息
typedef struct gnss_data_tables {
	unsigned long long timestamp;
	double latitude;
	double longitude;
	double altitude;
	char   fixQuality;
	int  satellitesTracked;
}GNSSInfo;

int cmnvCallback(int notifyId,const char *payload , const unsigned int len)
{
	if(cm_nvramCompareTid(TBOX_GKEY_CHAR0_MODEMGNSSINFO, notifyId))
	{
		GNSSInfo* locInfo = (GNSSInfo*)(payload);	
		//printf("latitude:%f, longitude:%f, timestamp:%d, satellitesTracked:%d\r\n", locInfo->latitude, locInfo->longitude, locInfo->timestamp, locInfo->satellitesTracked);
		if (s_get_gps_callback_fun)
		{
			s_get_gps_callback_fun(locInfo->latitude, locInfo->longitude);
		}
	}

	return 0;
}

int tbox_info_get_mcu_info(tbox_mcu_info_t *p_tbox_mcu_info, tbox_info_get_gps_callback get_gps_callback)
{
	int  ret = -1;
	char vin[18] = {0};
	char tboxSn[20] = {0};
	char supplierCode[7] = {0};
	char hardwareVersionNumber[25] = {0};
	char softwareVersionNumber[25] = {0};
	static int cm_nvramInit_flag = 0;

	if (!p_tbox_mcu_info)
	{
		return -1;
	}

	if (!cm_nvramInit_flag)
	{
		cm_nvramInit_flag = 1;
		s_get_gps_callback_fun = get_gps_callback;
		cm_nvramInit(cmnvCallback);
		do
		{
			sleep(1);
			printf("cm_nvramConnectStatus wait...\n");
		}while(!cm_nvramConnectStatus());
		cm_nvramAddSubscribe(TBOX_GKEY_CHAR0_MODEMGNSSINFO);
	}

	ret = cm_nvramRead(TBOX_GKEY_CHAR17_VIN, vin, sizeof(vin));
	if (ret != 0)
	{
		memset(vin, 0, sizeof(vin));
		printf("read vin err!\n");
	}
	ret = cm_nvramRead(TBOX_GKEY_CHAR19_TBOXSNSERIALNUMBER, tboxSn, sizeof(tboxSn));
	if (ret != 0)
	{
		memset(tboxSn, 0, sizeof(tboxSn));
		printf("read tboxSn err!\n");
	}
	ret = cm_nvramRead(TBOX_GKEY_CHAR6_SUPPLIERCODE, supplierCode, sizeof(supplierCode));
	if (ret != 0)
	{
		memset(supplierCode, 0, sizeof(supplierCode));
		printf("read supplierCode err!\n");
	}
	ret = cm_nvramRead(TBOX_GKEY_CHAR24_SUPPLIERDEFINEDHARDWAREVERSIONNUMBER, hardwareVersionNumber, sizeof(hardwareVersionNumber));
	if (ret != 0)
	{
		memset(hardwareVersionNumber, 0, sizeof(hardwareVersionNumber));
		printf("read hardwareVersionNumber err!\n");
	}
	ret = cm_nvramRead(TBOX_GKEY_CHAR24_SUPPLIERDEFINEDSOFTWAREVERSIONNUMBER, softwareVersionNumber, sizeof(softwareVersionNumber));
	if (ret != 0)
	{
		memset(softwareVersionNumber, 0, sizeof(softwareVersionNumber));
		printf("read softwareVersionNumber err!\n");
	}

	vin[17] = '\0';
	tboxSn[19] = '\0';
	supplierCode[6] = '\0';
	hardwareVersionNumber[24] = '\0';
	softwareVersionNumber[24] = '\0';

	strncpy(p_tbox_mcu_info->vin, vin, sizeof(p_tbox_mcu_info->vin) - 1);
	strncpy(p_tbox_mcu_info->sn, tboxSn, sizeof(p_tbox_mcu_info->sn)  - 1);
	strncpy(p_tbox_mcu_info->supplierInfo, supplierCode, sizeof(p_tbox_mcu_info->supplierInfo) - 1);
	strncpy(p_tbox_mcu_info->hardwareVersion, hardwareVersionNumber, sizeof(p_tbox_mcu_info->hardwareVersion) - 1);
	strncpy(p_tbox_mcu_info->softwareVersion, softwareVersionNumber, sizeof(p_tbox_mcu_info->softwareVersion) - 1);

	return 0;
}

#ifdef __cplusplus
}
#endif