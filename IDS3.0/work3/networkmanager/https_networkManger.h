#ifndef HTTPS_NETWORKMANAGER_H
#define HTTPS_NETWORKMANAGER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "signal.h"
#include "common.h"
#include "cJSON.h"
#include "Base_networkmanager.h"

#define GETREQUEST_DATALEN  (1024*5)

#define _IDPS_CA "root_cert.pem"
#define _IDPS_CRT "device_cert.pem"
#define _IDPS_KEY "device_key.pem"

int networkFunctionEnabled();

init 双向认证key  证书类型   

#endif