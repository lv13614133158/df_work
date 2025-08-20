#ifndef _IDPS_SHARE_DATA_H_
#define _IDPS_SHARE_DATA_H_

#include "idps_status.h"

int idps_share_data_area_init(void);
int shm_set_idps_status(idpsStatusInfo_t statusInfo);
int shm_get_idps_status(idpsStatusInfo_t *statusInfo);

#endif
