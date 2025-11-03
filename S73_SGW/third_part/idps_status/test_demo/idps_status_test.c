#include <stdio.h>
#include <unistd.h>
#include "idps_status.h"

int main()
{
    idpsStatusInfo_t statusInfo;
    int ret = -1;

    ret = init_idps_status(NULL);
    if (ret < 0)
    {
        printf("init idps status err!\n");
        return ret;
    }
    while (1)
    {
        ret = get_idps_status(&statusInfo);
        if (ret < 0)
        {
            printf("get idps status err!\n");
        }
        else
        {
            printf("software:%x,workflow:%x,program:%x,version:%x\n",
                statusInfo.idps_software_status,
                statusInfo.idps_workflow_status,
                statusInfo.idps_program_status,
                statusInfo.idps_version);
        }

        sleep(1);
    }
    deinit_idps_status();

    return 0;
}
