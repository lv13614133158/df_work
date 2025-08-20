#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include "idps_share_data.h"

static int isShareDataInit = 0;
static idpsStatusTransfer_t *shareDataArea = NULL;

int idps_share_data_area_init(void)
{
    int shmid;
    int keyFile = -1;
    key_t key;
    struct shmid_ds shmds;
    struct timespec tv;

    keyFile = open("/tmp/idpsStatusShareData.key", O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if (keyFile > 0)
    {
        close(keyFile);
    }
    else
    {
        isShareDataInit = 0;
        return -1;
    }

    if ((key = ftok("/tmp/idpsStatusShareData.key", 100)) < 0)
    {
        perror("creat key err");
        isShareDataInit = 0;
        return -1;
    }

    if ((shmid = shmget(key, sizeof(idpsStatusTransfer_t), 0666 | IPC_CREAT)) < 0)
    {
        perror("shmget error\n");
        isShareDataInit = 1;
        return -1;
    }

    shareDataArea = shmat(shmid, (const void *)0, 0);

    if (shareDataArea)
    {
        if (shmctl(shmid, IPC_STAT, &shmds) < 0)
        {
            fprintf(stderr, "shmctl get shared descriptor failure\n");
            isShareDataInit = 0;
            return -1;
        }
        else
        {
            /*Init data area*/
            if (shmds.shm_nattch <= 1)
            {
                memset(shareDataArea, 0, sizeof(idpsStatusTransfer_t));
                shareDataArea->idps_status.idps_software_status = IDPS_SOFTWARE_NORUNNING;
                shareDataArea->idps_status.idps_workflow_status = IDPS_WORKFLOW_NOVIN;
                shareDataArea->idps_status.idps_program_status = IDPS_PROGRAM_NOTOK;
                clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
                shareDataArea->idps_status_last_tv_sec = tv.tv_sec;
            }
        }
    }
    else
    {
        isShareDataInit = 0;
        return -1;
    }

    isShareDataInit = 1;

    return 0;
}

int shm_set_idps_status(idpsStatusInfo_t statusInfo)
{
    struct timespec tv;

    if (isShareDataInit != 1 || !shareDataArea)
    {
        printf("init share data area failed or not initialized.\n");
        return -1;
    }

    shareDataArea->idps_status = statusInfo;

    clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
    shareDataArea->idps_status_last_tv_sec = tv.tv_sec;

    return 0;
}

int shm_get_idps_status(idpsStatusInfo_t *statusInfo)
{
    if (isShareDataInit != 1 || !shareDataArea)
    {
        printf("init share data area failed or not initialized.\n");
        return -1;
    }

    if (!statusInfo)
    {
        printf("parameter is null.\n");
        return -1;
    }


    memcpy(statusInfo, &shareDataArea->idps_status, sizeof(idpsStatusInfo_t));

    return 0;
}
