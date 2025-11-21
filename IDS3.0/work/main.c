#include "idshttps.h"

int main(){


  idsnetworkManagerModult_t idsnetworkManagerModule = {
    .snPath = "",                                            // SN路径
    .url = "https://vsocidps-uat.dfiov.com.cn",              // URL
    .vin = "LQH02501170001",                                 // vin
    .channelId = "T00001",                                   // 渠道ID
    .equipmentType = "t-box",                                // 设备类型
    .caPath = "/home/nvidia/df/df_work/CA/s73a2/",           // 网络接口设备名
};
  
  idshttps_init(&idsnetworkManagerModule);
    return 0;
}

