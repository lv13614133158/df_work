#ifndef  __IDS_CONFIG_H__
#define  __IDS_CONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

// MCU还是CPU+linux系统
#define  USED_MCU_TYPE                

// Configure Table Size
#define  SK_FLOWANALY_NUM    (5)        
#define  SK_WHITELIST_NUM    (60)
#define  SK_LENCHECK_NUM     (50)
#define  SK_PRDCHECK_NUM     (50)
#define  SK_SIGNALMAX_NUM    (50)
#define  SK_STACKSIZE_NUM    (100)

#ifdef __cplusplus
}
#endif

#endif