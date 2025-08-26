
#ifndef __RLM_CAL_CACHE_H__
#define __RLM_CAL_CACHE_H__

#include "rtmp_type.h"

#define SCN_NUM 6

#define RX_SWAGC_LNA_NUM 2
#define RX_FDIQ_LPF_GAIN_NUM 3
#define RX_FDIQ_TABLE_SIZE 6

#define CHANNEL2G4_GROUP  0
#define CHANNEL5G_GROUP_1 1
#define CHANNEL5G_GROUP_2 2
#define CHANNEL5G_GROUP_3 3
#define CHANNEL5G_GROUP_4 4
#define CHANNEL5G_GROUP_5 5
#define CHANNEL5G_GROUP_6 6
#define CHANNEL5G_GROUP_7 7
#define CHANNEL5G_GROUP_8 8

#define CHANNEL_GROUP_NUM 9

#define TXLPF_PER_GROUP_DATA_SIZE    (SCN_NUM * sizeof(UINT32))
#define TXIQ_PER_GROUP_DATA_SIZE     (SCN_NUM * 6 * sizeof(UINT32))
#define TXDC_PER_GROUP_DATA_SIZE     (SCN_NUM * 6 * sizeof(UINT32))
#define RXFI_PER_GROUP_DATA_SIZE     (SCN_NUM * 4 * sizeof(UINT32))
#define RXFD_PER_GROUP_DATA_SIZE     (((SCN_NUM * RX_SWAGC_LNA_NUM) +  \
                                     (SCN_NUM * RX_FDIQ_LPF_GAIN_NUM * \
                                      RX_FDIQ_TABLE_SIZE * 3)) * sizeof(UINT32))

#define PRECAL_FLASH_MODE 1


#define RLM_CAL_CACHE_TXLPF_CAL_INFO(_C) (((P_RLM_CAL_CACHE)(_C))->txLpfCalInfo)
#define RLM_CAL_CACHE_TXIQ_CAL_INFO(_C) (((P_RLM_CAL_CACHE)(_C))->txIqCalInfo)
#define RLM_CAL_CACHE_TXDC_CAL_INFO(_C) (((P_RLM_CAL_CACHE)(_C))->txDcCalInfo)
#define RLM_CAL_CACHE_RXFI_CAL_INFO(_C) (((P_RLM_CAL_CACHE)(_C))->rxFiCalInfo)
#define RLM_CAL_CACHE_RXFD_CAL_INFO(_C, _I) (((P_RLM_CAL_CACHE)(_C))->rxFdCalInfo[(_I)])

//#define BIT(n) ((UINT32) 1 << (n))
#define TXPLF_READY BIT(0)
#define TXIQ_READY BIT(1)
#define TXPDC_READY BIT(2)
#define RXFI_READY BIT(3)
#define RXFD_GROUP_READY(_I) BIT((_I)+4)

#define RLM_CAL_CACHE_TXLPF_CAL_DONE(_C) \
    ((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) |= TXPLF_READY)
#define RLM_CAL_CACHE_TXIQ_CAL_DONE(_C) \
    ((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) |= TXIQ_READY)
#define RLM_CAL_CACHE_TXDC_CAL_DONE(_C) \
    ((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) |= TXPDC_READY)
#define RLM_CAL_CACHE_RXFI_CAL_DONE(_C) \
    ((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) |= RXFI_READY)
#define RLM_CAL_CACHE_RXFD_CAL_DONE(_C, _I) \
    ((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) |= RXFD_GROUP_READY(_I))

#define RLM_CAL_CACHE_IS_TXLPF_CAL_DONE(_C) \
    (((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) & TXPLF_READY) == TXPLF_READY)
#define RLM_CAL_CACHE_IS_TXIQ_CAL_DONE(_C) \
    (((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) & TXIQ_READY) == TXIQ_READY)
#define RLM_CAL_CACHE_IS_TXDC_CAL_DONE(_C) \
    (((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) & TXPDC_READY) == TXPDC_READY)
#define RLM_CAL_CACHE_IS_RXFI_CAL_DONE(_C) \
    (((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) & RXFI_READY) == RXFI_READY)
#define RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, _I) \
    (((((P_RLM_CAL_CACHE)(_C))->rlmCalStatus) & RXFD_GROUP_READY(_I)) == RXFD_GROUP_READY(_I))

#define RLM_CAL_CACHE_IS_DONE(_C) \
    RLM_CAL_CACHE_IS_RXFI_CAL_DONE(_C) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 0) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 1) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 2) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 3) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 4) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 5) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 6) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 7) & \
    RLM_CAL_CACHE_IS_RXFD_CAL_DONE(_C, 8)

#define RLM_PRECAL_TXLPF_TO_FLASH_CHECK(_C) \
    ((((P_TXLPF_CAL_INFO_T)(_C))->ucDataToFromFlash) == PRECAL_FLASH_MODE)
#define RLM_PRECAL_TXIQ_TO_FLASH_CHECK(_C) \
    ((((P_TXIQ_CAL_INFO_T)(_C))->ucDataToFromFlash) == PRECAL_FLASH_MODE)
#define RLM_PRECAL_TXDC_TO_FLASH_CHECK(_C) \
    ((((P_TXDC_CAL_INFO_T)(_C))->ucDataToFromFlash) == PRECAL_FLASH_MODE)
#define RLM_PRECAL_RXFI_TO_FLASH_CHECK(_C) \
    ((((P_RXFI_CAL_INFO_T)(_C))->ucDataToFromFlash) == PRECAL_FLASH_MODE)
#define RLM_PRECAL_RXFD_TO_FLASH_CHECK(_C) \
    ((((P_RXFD_CAL_INFO_T)(_C))->ucDataToFromFlash) == PRECAL_FLASH_MODE)

typedef struct _TXLPF_CAL_INFO_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucReserved[3];
    UINT32 au4Data[CHANNEL_GROUP_NUM*SCN_NUM];
} TXLPF_CAL_INFO_T, *P_TXLPF_CAL_INFO_T;

#ifdef PRE_CAL_MT7915_SUPPORT
typedef struct _PRE_CAL_INFO_MT7915_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucSecBw80;
    UINT8  ucReserved[3];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_MT7915_T, *P_PRE_CAL_INFO_MT7915_T;
#endif /* PRE_CAL_MT7915_SUPPORT */

#ifdef PRE_CAL_MT7986_SUPPORT
typedef struct _PRE_CAL_INFO_MT7986_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucReserved[4];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_MT7986_T, *P_PRE_CAL_INFO_MT7986_T;
#endif /* PRE_CAL_MT7986_SUPPORT */

#ifdef PRE_CAL_MT7916_SUPPORT
typedef struct _PRE_CAL_INFO_MT7916_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucReserved[4];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_MT7916_T, *P_PRE_CAL_INFO_MT7916_T;
#endif /* PRE_CAL_MT7916_SUPPORT */

#ifdef PRE_CAL_MT7981_SUPPORT
typedef struct _PRE_CAL_INFO_MT7981_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucReserved[4];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_MT7981_T, *P_PRE_CAL_INFO_MT7981_T;
#endif /* PRE_CAL_MT7981_SUPPORT */

#ifdef PRE_CAL_BELLWETHER_SUPPORT
typedef struct _PRE_CAL_INFO_BELLWETHER_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucSecBw80;
    UINT8  ucReserved[3];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_BELLWETHER_T, *P_PRE_CAL_INFO_BELLWETHER_T;
#endif /* PRE_CAL_BELLWETHER_SUPPORT */

#ifdef PRE_CAL_MT7990_SUPPORT
typedef struct _PRE_CAL_INFO_MT7990_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucSecBw80;
    UINT8  ucReserved[3];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_MT7990_T, *P_PRE_CAL_INFO_MT7990_T;
#endif /* PRE_CAL_MT7990_SUPPORT */

#ifdef PRE_CAL_MT7992_SUPPORT
typedef struct _PRE_CAL_INFO_MT7992_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucSecBw80;
    UINT8  ucReserved[3];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_MT7992_T, *P_PRE_CAL_INFO_MT7992_T;
#endif /* PRE_CAL_MT7992_SUPPORT */

#ifdef PRE_CAL_MT7993_SUPPORT
typedef struct _PRE_CAL_INFO_MT7993_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    INT8   cPreCalTemp;
    UINT8  ucAction;
    UINT8  ucBand;
    UINT8  ucIndex;
    UINT8  ucSecBw80;
    UINT8  ucReserved[3];
    UINT32 u4Length;
    UINT32 au4Data[0];
} PRE_CAL_INFO_MT7993_T, *P_PRE_CAL_INFO_MT7993_T;
#endif /* PRE_CAL_MT7993_SUPPORT */

typedef struct _TXIQ_CAL_INFO_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    UINT32 au4Data[CHANNEL_GROUP_NUM*SCN_NUM*6];
} TXIQ_CAL_INFO_T, *P_TXIQ_CAL_INFO_T;

typedef struct _TXDC_CAL_INFO_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    UINT32 au4Data[CHANNEL_GROUP_NUM*SCN_NUM*6];
} TXDC_CAL_INFO_T, *P_TXDC_CAL_INFO_T;

typedef struct _RXFI_CAL_INFO_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    UINT32 au4Data[CHANNEL_GROUP_NUM*SCN_NUM*4];
} RXFI_CAL_INFO_T, *P_RXFI_CAL_INFO_T;

typedef struct _RXFD_CAL_INFO_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    UINT32 u4ChGroupId;
    UINT32 au4Data[
        (SCN_NUM*RX_SWAGC_LNA_NUM)
        + (SCN_NUM*RX_FDIQ_LPF_GAIN_NUM*RX_FDIQ_TABLE_SIZE*3)];
} RXFD_CAL_INFO_T, *P_RXFD_CAL_INFO_T;

typedef struct _RXFD_CAL_CACHE_T
{
    UINT8  ucDataToFromFlash;
    UINT8  ucDataValid;
    UINT16 u2BitMap;
    UINT32 u4ChGroupId;
    UINT32 au4Data[
        (SCN_NUM*RX_SWAGC_LNA_NUM)
        + (SCN_NUM*RX_FDIQ_LPF_GAIN_NUM* RX_FDIQ_TABLE_SIZE*3)];
} RXFD_CAL_CACHE_T, *P_RXFD_CAL_CACHE_T;

typedef struct _RLM_POR_CAL_INFO_T
{
    UINT8 ucRlmPorCal;
    UINT8 aucReserved[3];
} RLM_POR_CAL_INFO_T, *P_RLM_POR_CAL_INFO_T;

typedef struct _RLM_CAL_CACHE {
    UINT32 rlmCalStatus;
    TXLPF_CAL_INFO_T txLpfCalInfo;
    TXIQ_CAL_INFO_T txIqCalInfo;
    TXDC_CAL_INFO_T txDcCalInfo;
    RXFI_CAL_INFO_T rxFiCalInfo;
    RXFD_CAL_CACHE_T rxFdCalInfo[CHANNEL_GROUP_NUM];
} RLM_CAL_CACHE, *P_RLM_CAL_CACHE;

INT rlmCalCacheDone(VOID *rlmCalCache);
INT rlmCalCacheInit(VOID *pAd, VOID **ppRlmCalCache);
INT rlmCalCacheDeinit(VOID **ppRlmCalCache);
INT RlmCalCacheTxLpfInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length);
INT RlmCalCacheTxIqInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length);
INT RlmCalCacheTxDcInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length);
INT RlmCalCacheRxFiInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length);
INT RlmCalCacheRxFdInfo(VOID *rlmCalCtrl, UINT8 *Data, UINT32 Length);
VOID rlmCalCacheStatus(VOID *rlmCalCache);
VOID rlmCalCacheDump(VOID *rlmCalCache);

INT TxLpfCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr);
INT TxIqCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr);
INT TxDcCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr);
INT RxFiCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr);
INT RxFdCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr, UINT32 chGroup);
INT RlmPorCalInfoAlloc(VOID *pAd, VOID *rlmCalCache, VOID **pptr);

#ifndef WIFI_UNIFIED_COMMAND
#ifdef PRE_CAL_MT7915_SUPPORT
INT GroupPreCalInfoAlloc_7915(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_7915(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_MT7915_SUPPORT */

#ifdef PRE_CAL_MT7986_SUPPORT
INT GroupPreCalInfoAlloc_7986(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_7986(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_MT7986_SUPPORT */

#ifdef PRE_CAL_MT7916_SUPPORT
INT GroupPreCalInfoAlloc_7916(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_7916(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_MT7916_SUPPORT */

#ifdef PRE_CAL_MT7981_SUPPORT
INT GroupPreCalInfoAlloc_7981(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_7981(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_MT7981_SUPPORT */

#ifdef PRE_CAL_BELLWETHER_SUPPORT
INT GroupPreCalInfoAlloc_bellwether(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_bellwether(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_BELLWETHER_SUPPORT */

#ifdef PRE_CAL_MT7990_SUPPORT
INT GroupPreCalInfoAlloc_7990(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_7990(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, INT32 eeprom_offset);
#endif /* PRE_CAL_MT7990_SUPPORT */

#ifdef PRE_CAL_MT7992_SUPPORT
INT GroupPreCalInfoAlloc_7992(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_7992(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_MT7992_SUPPORT */

#ifdef PRE_CAL_MT7993_SUPPORT
INT GroupPreCalInfoAlloc_7993(struct _RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length);
INT DpdFlatnessCalInfoAlloc_7993(RTMP_ADAPTER *pAd, VOID **pptr, UINT16 idx, UINT32 length, BOOLEAN bSecBw80);
#endif /* PRE_CAL_MT7993_SUPPORT */
#endif /* WIFI_UNIFIED_COMMAND */

#endif /* __RLM_CAL_CACHE_H__ */

