/******************************************************************************
 *
 * Copyright (c) 2011 TP-LINK Technologies CO.,LTD.
 * All rights reserved.
 *
 * FILE NAME  :   wlan_debug.h
 * VERSION    :   1.0
 * DESCRIPTION:   Minimal debug/trace/assert driver definitions.
 *
 * AUTHOR     :   zhengxinggu <zhengxinggu@tp-link.net>
 * CREATE DATE:   12/28/2011
 *
 * HISTORY    :
 * 01   12/28/2011  zhengxinggu     Create.
 * 02   05/30/2014  yangyang        Modify it for ralink driver.
 *
 ******************************************************************************/
#ifndef _WLAN_DEBUG_H_
#define _WLAN_DEBUG_H_

typedef struct umac_dbglvl_map
{
	INT8 *lvlName;
	UINT32 lvlFlag;
}UMACDBGLVLMAP_T;

#ifdef TPWD_DEBUG

#define WL_DBG_NONE				0x00000000
#define WL_DBG_INFO				0x00000001
#define WL_DBG_ERROR			0x00000002
#define WL_DBG_WARN				0x00000004
#define WL_DBG_TRACE			0x00000008
#define WL_DBG_ASSOC			0x00000010
#define WL_DBG_PWR				0x00000020
#define WL_DBG_WPA				0x00000040
#define WL_DBG_WPS				0x00000080
#define WL_DBG_BR				0x00000100
#define WL_DBG_MCAST			0x00000200
#define WL_DBG_AMPDU			0x00000400
#define WL_DBG_MFG				0x00000800
#define WL_DBG_INT				0x00001000
#define WL_DBG_TX99				0x00002000
#define WL_DBG_RATE				0x00004000
#define WL_DBG_A34DETECT		0x00008000
#define WL_DBG_TURBO			0x00010000
#define WL_DBG_TMP				0x80000000

#define WL_DBG_DEF				0xFFFFFFFF//(WL_DBG_NONE)

#define TPWD_PRINT(flg, fmt, args...)										\
	do																		\
	{																		\
		if (g_tpwdDebug & flg)												\
		{																	\
			my_printf(fmt, ## args);	\
		}																	\
	}																		\
	while (0)

#define __MAC__			"%02X-%02X-%02X-%02X-%02X-%02X : "
#define __IPV4__		"%u.%u.%u.%u"

#define IPV4_PRINT(ip)	0xff&*(ip), 0xff&*(ip+1), 0xff&*(ip+2), 0xff&*(ip+3)
#define MAC_PRINT(mac)	0xff&*(mac), 0xff&*(mac+1), 0xff&*(mac+2), 0xff&*(mac+3), 0xff&*(mac+4), 0xff&*(mac+5)

#define DBG_INFO(fmt, args...)				TPWD_PRINT(WL_DBG_INFO, fmt, ## args)
#define DBG_ERR(fmt, args...)				TPWD_PRINT(WL_DBG_ERROR, fmt, ## args)
#define DBG_WARN(fmt, args...)				TPWD_PRINT(WL_DBG_WARN, fmt, ## args)
#define DBG_TRACE(fmt, args...)				TPWD_PRINT(WL_DBG_TRACE, fmt, ## args)
#define DBG_ASSOC(fmt, args...)				TPWD_PRINT(WL_DBG_ASSOC, fmt, ## args)
#define DBG_PWR(fmt, args...)				TPWD_PRINT(WL_DBG_PWR, fmt, ## args)
#define DBG_WPA(fmt, args...)				TPWD_PRINT(WL_DBG_WPA, fmt, ## args)
#define DBG_WPS(fmt, args...)				TPWD_PRINT(WL_DBG_WPS, fmt, ## args)
#define DBG_BR(fmt, args...)				TPWD_PRINT(WL_DBG_BR, fmt, ## args)
#define DBG_MCAST(fmt, args...)				TPWD_PRINT(WL_DBG_MCAST, fmt, ## args)
#define DBG_AMPDU(fmt, args...)				TPWD_PRINT(WL_DBG_AMPDU, fmt, ## args)
#define DBG_MFG(fmt, args...)				TPWD_PRINT(WL_DBG_MFG, fmt, ## args)
#define DBG_INT(fmt, args...)				TPWD_PRINT(WL_DBG_INT, fmt, ## args)
#define DBG_TX99(fmt, args...)				TPWD_PRINT(WL_DBG_TX99, fmt, ## args)
#define DBG_RATE(fmt, args...)				TPWD_PRINT(WL_DBG_RATE, fmt, ## args)
#define DBG_A34DETECT(fmt, args...)			TPWD_PRINT(WL_DBG_A34DETECT, fmt, ## args)
#define DBG_TURBO(fmt, args...)				TPWD_PRINT(WL_DBG_TURBO, fmt, ## args);
#define DBG_TMP(fmt, args...)				TPWD_PRINT(WL_DBG_TMP, fmt, ## args)

#ifdef ASSERT
#undef ASSERT
#define ASSERT(x)							OS_ASSERT(x)

#else
#define ASSERT(x)							OS_ASSERT(x)
#endif

extern UINT32 g_tpwdDebug;	/* golbal debug level mask */

#else /* TPWD_DEBUG */

#define WL_DBG_DEF							0

#define DBG_INFO(fmt, args...)
#define DBG_ERR(fmt, args...)
#define DBG_WARN(fmt, args...)
#define DBG_TRACE(fmt, args...)
#define DBG_ASSOC(fmt, args...)
#define DBG_PWR(fmt, args...)
#define DBG_WPA(fmt, args...)
#define DBG_WPS(fmt, args...)
#define DBG_BREXT(fmt, args...)
#define DBG_MCAST(fmt, args...)
#define DBG_AMPDU(fmt, args...)
#define DBG_MFG(fmt, args...)
#define DBG_TX99(fmt, args...)
#define DBG_RATE(fmt, args...)
#define DBG_A34DETECT(fmt, args...)
#define DBG_TMP(fmt, args...)

#ifdef ASSERT
#undef ASSERT
#define ASSERT(x)
#endif

#endif /* TPWD_DEBUG */

#endif /* _WLAN_DEBUG_H_ */
