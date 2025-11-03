/* * --------------------------------------------------------------------
 * |                                                                  |
 * |     _         _    _ _______ ____         _____ ____  __  __     |
 * |    (_)   /\  | |  | |__   __/ __ \       / ____/ __ \|  \/  |    |
 * |     _   /  \ | |  | |  | | | |  | |     | |   | |  | | \  / |    |
 * |    | | / /\ \| |  | |  | | | |  | |     | |   | |  | | |\/| |    |
 * |    | |/ ____ \ |__| |  | | | |__| |  _  | |___| |__| | |  | |    |
 * |    |_/_/    \_\____/   |_|  \____/  (_)  \_____\____/|_|  |_|    |
 * |                                                                  |
 * --------------------------------------------------------------------
 *
 * Copyright @ 2022 iAuto (Shanghai) Co., Ltd.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted except as agreed by
 * iAuto (Shanghai) Co., Ltd.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */



#ifndef IDPS_PROXY_H
#define IDPS_PROXY_H
namespace nutshell
{

int getVin(std::string& vin);
int getSerialNumber(std::string& seq);
int getSupplierInfo(std::string& info);
int getHardwareVersion(std::string& version);
int getSoftwareVersion(std::string& version);

}
#endif