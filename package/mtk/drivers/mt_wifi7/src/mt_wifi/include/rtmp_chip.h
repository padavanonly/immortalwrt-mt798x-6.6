/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	rtmp_chip.h

	Abstract:
	Ralink Wireless Chip related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef	__RTMP_CHIP_H__
#define	__RTMP_CHIP_H__

#include "rtmp_type.h"
#include "eeprom.h"
#include "tx_power.h"
#include "hif/hif.h"
#include "mac.h"
#include "mcu/mcu.h"
#if defined(MT7986) || defined(MT7916) || defined(MT7981) || defined(BELLWETHER) || \
	defined(MT7990) || defined(MT7992) || defined(MT7993)
#include "chip/common.h"
#endif






#ifdef MT7990
#include "chip/mt7990.h"
#endif

#ifdef MT7992
#include "chip/mt7992.h"
#endif

#ifdef MT7993
#include "chip/mt7993.h"
#endif

#include "hw_ctrl/hw_init.h"


/* P18 Firmware Version */
#define P18_FW_VER_E1		0x00		/* P18 FW Ver E1	0x8A00, use E1 RAM IMAGE */
#define IS_P18_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == P18_FW_VER_E1)
/* 0x6632 is for FPGA release v10 */
#define IS_P18(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00006632)
/* End of P18 Firmware Version */

/* MT7915 Firmware Version */
#define MT7915_FW_VER_E1		0x00		/* MT7915 FW Ver E1	0x8A00 */
#define MT7915_FW_VER_E2		0x01		/* MT7915 FW Ver E2	0x8A01 */
#define IS_MT7915_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7915_FW_VER_E1)
#define IS_MT7915_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7915_FW_VER_E2)
#define MT7915E1 0x0000
#define MT7915E2 0x0010
#define IS_MT7915(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007915)
#define IS_MT7915_6E(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007915) \
							&& ((_pAd)->Wifi6gCap == 1))
/* End of MT7915 Firmware Version */

/* MT7916 Firmware Version */
#define MT7916_FW_VER_E1		0x00		/* MT7916 FW Ver E1	0x8A00 */
#define IS_MT7916_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7916_FW_VER_E1)
#define IS_MT7916(_pAd)		((((_pAd)->ChipID & 0x0000ffff) == 0x00007906))
/* End of MT7916 Firmware Version */

/* MT7981 Firmware Version */
#define MT7981_FW_VER_E1		0x00		/* MT7981 FW Ver E1	0x8A00 */
#define IS_MT7981_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7981_FW_VER_E1)
#define IS_MT7981(_pAd)		((((_pAd)->ChipID & 0x0000ffff) == 0x00007981) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00000790))
/* End of MT7981 Firmware Version */

/* MT7986 Firmware Version */
#define MT7986_FW_VER_E1		0x00		/* MT7986 FW Ver E1	0x8A00 */
#define IS_MT7986_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7986_FW_VER_E1)
#define IS_MT7986(_pAd)		((((_pAd)->ChipID & 0x0000ffff) == 0x00007986) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00000789))
/* End of MT7986 Firmware Version */

/* BELLWETHER Firmware Version */
#define BELLWETHER_FW_VER_E2	0x01
#define BELLWETHER_FW_VER_E1	0x00
#define IS_BELLWETHER_FW_VER_E1(_pAd) \
	(((_pAd)->FWVersion & 0x000000ff) == BELLWETHER_FW_VER_E1)
#define IS_BELLWETHER_FW_VER_E2(_pAd) \
	(((_pAd)->FWVersion & 0x000000ff) == BELLWETHER_FW_VER_E2)
#define BELLWETHERE1 0x00
#define IS_BELLWETHER(_pAd)	(((_pAd)->ChipID & 0x0000ffff) == 0x00007902)
/* End of BELLWETHER Version */

/* MT7990 Firmware Version */
#define MT7990_FW_VER_E1		0x00		/* MT7990 FW Ver E1	0x8A00 */
#define MT7990_FW_VER_E2		0x01		/* MT7990 FW Ver E2	0x8A01 */
#define IS_MT7990_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7990_FW_VER_E1)
#define IS_MT7990_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7990_FW_VER_E2)
#define MT7990E1 0x0000
#define MT7990E2 0x0010
#define IS_MT7990(_pAd)	(((_pAd)->ChipID & 0x0000ffff) == 0x00007990)
/* End of MT7990 Firmware Version */

/* MT7992 Firmware Version */
#define MT7992_FW_VER_E1		0x00
#define MT7992_FW_VER_E2		0x01
#define IS_MT7992_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7992_FW_VER_E1)
#define IS_MT7992_FW_VER_E2(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7992_FW_VER_E2)
#define MT7992E1 0x0000
#define IS_MT7992(_pAd)	(((_pAd)->ChipID & 0x0000ffff) == 0x00007992)
/* End of MT7992 Firmware Version */

/* MT7993 Firmware Version */
#define MT7993_FW_VER_E1		0x00
#define IS_MT7993_FW_VER_E1(_pAd)		(((_pAd)->FWVersion & 0xff) == MT7993_FW_VER_E1)
#define MT7993E1 0x0000
#define IS_MT7993(_pAd)	(((_pAd)->ChipID & 0x0000ffff) == 0x00007993)
/* End of MT7993 Firmware Version */

#define IS_SUPPORT_ATE_TX_BY_TIME(_pAd) \
	(IS_P18(_pAd) || \
	 IS_MT7915(_pAd) || IS_MT7986(_pAd) || IS_MT7916(_pAd) || \
	 IS_MT7981(_pAd) || IS_BELLWETHER(_pAd) || IS_MT7990(_pAd) || \
	 IS_MT7992(_pAd) || IS_MT7993(_pAd))

/* RT3592BC8 (WiFi + BT) */
#define RT_REV_LT(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) < (_rev)))

#define RT_REV_GTE(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) >= (_rev)))

#define MT_REV_LT(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) < (_rev)))

#define MT_REV_GTE(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) >= (_rev)))

#define MT_REV_ET(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) == (_rev)))

#define MTK_REV_LT(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) < (_rev)))

#define MTK_REV_GTE(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) >= (_rev)))

#define MTK_REV_ET(_pAd, _chip, _rev)\
	(IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) == (_rev)))


struct _RSSI_SAMPLE;
union _EXT_CMD_EFUSE_BUFFER_MODE_T;


#endif /* __RTMP_CHIP_H__ */

