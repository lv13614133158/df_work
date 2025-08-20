#include <string>
#include <iostream>
#include <string.h>
#include "idpsProxy.h"
#include "idpsReadInfo.h"

#ifdef __cplusplus
extern "C" {
#endif


int sgw_info_get_mcu_info(sgw_mcu_info_t *p_sgw_mcu_info)
{
	int ret = -1;
	std::string vin;
	std::string seq;
	std::string info;
	std::string hardwareVersion;
	std::string softwareVersion;

	if (!p_sgw_mcu_info)
	{
		return -1;
	}

	ret = nutshell::getVin(vin);
	ret = nutshell::getSerialNumber(seq);
	ret = nutshell::getSupplierInfo(info);
	ret = nutshell::getHardwareVersion(hardwareVersion);
	ret = nutshell::getSoftwareVersion(softwareVersion);
	
	strncpy(p_sgw_mcu_info->vin, vin.c_str(), sizeof(p_sgw_mcu_info->vin) - 1);
	strncpy(p_sgw_mcu_info->sn, seq.c_str(), sizeof(p_sgw_mcu_info->sn)  - 1);
	strncpy(p_sgw_mcu_info->supplierInfo, info.c_str(), sizeof(p_sgw_mcu_info->supplierInfo) - 1);
	strncpy(p_sgw_mcu_info->hardwareVersion, hardwareVersion.c_str(), sizeof(p_sgw_mcu_info->hardwareVersion) - 1);
	strncpy(p_sgw_mcu_info->softwareVersion, softwareVersion.c_str(), sizeof(p_sgw_mcu_info->softwareVersion) - 1);
}

#ifdef __cplusplus
}
#endif