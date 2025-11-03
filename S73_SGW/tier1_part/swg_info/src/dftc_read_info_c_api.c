#include <stdio.h>
#include <string.h>
#include "idpsReadInfo.h"
	
int main()
{
	sgw_mcu_info_t sgw_mcu_info;
	memset(&sgw_mcu_info, 0, sizeof(sgw_mcu_info));

	sgw_info_get_mcu_info(&sgw_mcu_info);

	printf("vin:%s, sn:%s, supplierInfo:%s, hardwareVersion:%s, softwareVersion:%s\n",
			sgw_mcu_info.vin, sgw_mcu_info.sn, sgw_mcu_info.supplierInfo, sgw_mcu_info.hardwareVersion, sgw_mcu_info.softwareVersion);
	return 0;
}