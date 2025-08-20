#include <stdio.h>
#include <string.h>
#include "idpsReadInfo.h"
	
int main()
{
	tbox_mcu_info_t tbox_mcu_info;
	memset(&tbox_mcu_info, 0, sizeof(tbox_mcu_info));

	tbox_info_get_mcu_info(&tbox_mcu_info);

	printf("vin:%s, sn:%s, supplierInfo:%s, hardwareVersion:%s, softwareVersion:%s\n",
			tbox_mcu_info.vin, tbox_mcu_info.sn, tbox_mcu_info.supplierInfo, tbox_mcu_info.hardwareVersion, tbox_mcu_info.softwareVersion);
	return 0;
}