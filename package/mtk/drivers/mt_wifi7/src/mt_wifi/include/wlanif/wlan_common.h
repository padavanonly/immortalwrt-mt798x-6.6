/******************************************************************************
 *
 * Copyright (c) 2011 TP-LINK Technologies CO.,LTD.
 * All rights reserved.
 *
 * FILE NAME  :   wlan_common.h
 * VERSION    :   1.0
 * DESCRIPTION:   NOTE: Please do not add any functional prototype in here and 
 *				  also do not include any other header files.
 *				  Only constants, enum, and structure definitions.
 *
 * AUTHOR     :   zhengxinggu <zhengxinggu@tp-link.net>
 * CREATE DATE:   10/17/2011
 *
 * HISTORY    :
 * 01   10/17/2011  zhengxinggu     Create.
 * 02   05/30/2014  yangyang        Modify it for ralink drivers.
 ******************************************************************************/
#ifndef _WLAN_COMMON_H_
#define _WLAN_COMMON_H_

#define SIZE_1_BYTE					1
#define SIZE_2_BYTES				2
#define SIZE_4_BYTES				4
#define SIZE_6_BYTES				6
#define SIZE_8_BYTES				8
#define SIZE_16_BYTES				16
#define SIZE_20_BYTES				20
#define SIZE_32_BYTES				32
#define SIZE_64_BYTES				64
#define SIZE_80_BYTES				80
#define SIZE_128_BYTES				128
#define SIZE_192_BYTES				192
#define SIZE_512_BYTES				512
#define SIZE_1024_BYTES				1024

#define MAX_SSID_LEN				32
#define MAX_AP_LIST					8		/* candidate ap list cnt for sta */
#define MAX_PSK_LEN					64
#define MAX_RADIUS_LEN				64
#define MAX_WEP_NKID				4		/* number of key ids */
#define MAX_MPT_CNT					6

#define MAC_ADDR_LEN				6
#define IPV4_ADDR_LEN				4		/* IPV4 address length */
#define WLAN_OUI_LEN				3		/* IEEE80211 OUI length */
#define MAX_PROTO_STRING_LENGTH		32		/* maximum length of network protocol */

#define HW_LED_SCHEME				0		/* hardware control led. */
#define SIX_LED_SCHEME				1		/* There is no wlan led. */
#define EIGHT_LED_SCHEME			2		/* Wlan led is diff with QSS led. */

#define	ARR_LENGTH(_a)    		(sizeof (_a) / sizeof (_a[0]))

enum
{
	DETECT_NOT_LINK = 0,
	DETECT_DETECTING,
	DETECT_DETECTED,
	DETECT_NOT_NECESSARY    /* no need to detect, conf is explicitly set. added by xiejingfen    g 30Jan13 */
};

/* os defined unit */
enum
{
	/* 2G for dual band driver  */
	UNIT_2G_MAIN = 0,
	/* UNIT_IPTV, */
	UNIT_2G_GUESTNETWORK,
	UNIT_2G_APC,
	UNIT_2G_MAX = UNIT_2G_APC,

	/* 5G for dual band driver  */
	UNIT_5G_MAIN = 0,
	/* UNIT_IPTV, */
	UNIT_5G_APC,
	UNIT_5G_MAX = UNIT_5G_APC,

	/* 2G only for single band driver  */
	UNIT_MAIN = UNIT_2G_MAIN,
	/* UNIT_IPTV, */
	UNIT_GUESTNETWORK = UNIT_2G_GUESTNETWORK,
	UNIT_APC = UNIT_2G_APC,
	UNIT_MAX = UNIT_2G_MAX,
};

enum
{
	DOMAIN_FCC = 0,
	DOMAIN_IC,
	DOMAIN_ETSI,
	DOMAIN_SPAIN,
	DOMAIN_FRANCE,
	DOMAIN_MKK,
	DOMAIN_ISRAEL,
	DOMAIN_MKK1,
	DOMAIN_MKK2,
	DOMAIN_MKK3,
	DOMAIN_MAX
};

/* MAC ACL support */
enum
{
	ACL_POLICY_OPEN		= 0,			/* open, don't check ACL's */
	ACL_POLICY_DENY		= 1,			/* deny traffic from MAC */
	ACL_POLICY_ALLOW	= 2				/* allow traffic from MAC */
};

#define MIN(a, b)					((a) < (b) ? (a) : (b))
#define MAX(a, b)					((a) > (b) ? (a) : (b))

#define SETBIT(a,i)					((a)[(i) >> 3] |= 1 << ((i) & (NBBY - 1)))
#define CLRBIT(a,i)					((a)[(i) >> 3] &= ~(1 << ((i) & (NBBY - 1))))
#define ISSET(a,i)					((a)[(i) >> 3] & (1 << ((i) & (NBBY - 1))))
#define ISCLR(a,i)					(((a)[(i) >> 3] & (1 << ((i) & (NBBY - 1)))) == 0)


#define SWAP64(x) ((UINT64)(											\
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000000000ffULL) << 56) |	\
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000000000ff00ULL) << 40) |	\
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000000000ff0000ULL) << 24) |	\
    (UINT64)(((UINT64)(x) & (UINT64) 0x00000000ff000000ULL) <<  8) |	\
    (UINT64)(((UINT64)(x) & (UINT64) 0x000000ff00000000ULL) >>  8) |	\
    (UINT64)(((UINT64)(x) & (UINT64) 0x0000ff0000000000ULL) >> 24) |	\
    (UINT64)(((UINT64)(x) & (UINT64) 0x00ff000000000000ULL) >> 40) |	\
    (UINT64)(((UINT64)(x) & (UINT64) 0xff00000000000000ULL) >> 56) ))

#define SWAP32(x) ((UINT32)(							\
	(((UINT32)(x) & (UINT32)0x000000ffUL) << 24) |		\
	(((UINT32)(x) & (UINT32)0x0000ff00UL) <<  8) |		\
	(((UINT32)(x) & (UINT32)0x00ff0000UL) >>  8) |		\
	(((UINT32)(x) & (UINT32)0xff000000UL) >> 24)))

#define SWAP16(x) ((UINT16)(							\
	(((UINT16)(x) & (UINT16)0x00ffU) << 8) |			\
	(((UINT16)(x) & (UINT16)0xff00U) >> 8)))


#define _BIG_ENDIAN			1234
#define _LITTLE_ENDIAN		4321

#if defined(MIPSEB) || defined(__MIPSEB__)
#undef _BYTE_ORDER
#define	_BYTE_ORDER			_BIG_ENDIAN
#elif defined(MIPSEL) || defined(__MIPSEL__)
#undef _BYTE_ORDER
#define	_BYTE_ORDER			_LITTLE_ENDIAN
#elif defined(ARMEB) || defined(__ARMEB_)
#undef _BYTE_ORDER
#define	_BYTE_ORDER			_BIG_ENDIAN
#elif defined(ARMEL) || defined(__ARMEL__)
#undef _BYTE_ORDER
#define	_BYTE_ORDER			_LITTLE_ENDIAN
#else
#warning "One of MIPSEL or MIPSEB or ARMEL or ARMEB must be defined"
#endif

#define TU_TO_TSF(_tsf) ((_tsf)<<10)
#define TSF_TO_TU(_h,_l) \
    ((((UINT32)(_h)) << 22) | (((UINT32)(_l)) >> 10))

/*
 * Macros for number representation conversion.
 */
#if	_BYTE_ORDER == _BIG_ENDIAN
#define CPU_TO_BE64(x)		(x)
#define CPU_TO_BE32(x)		(x)
#define CPU_TO_BE16(x)		(x)
#define BE64_TO_CPU(x)		(x)
#define BE32_TO_CPU(x)		(x)
#define BE16_TO_CPU(x)		(x)

#define CPU_TO_LE64(x)		SWAP64(x)
#define CPU_TO_LE32(x)		SWAP32(x)
#define CPU_TO_LE16(x)		SWAP16(x)
#define LE64_TO_CPU(x)		SWAP64(x)
#define LE32_TO_CPU(x)		SWAP32(x)
#define LE16_TO_CPU(x)		SWAP16(x)

#elif _BYTE_ORDER == _LITTLE_ENDIAN
#define CPU_TO_BE64(x)		SWAP64(x)
#define CPU_TO_BE32(x)		SWAP32(x)
#define CPU_TO_BE16(x)		SWAP16(x)
#define BE64_TO_CPU(x)		SWAP64(x)
#define BE32_TO_CPU(x)		SWAP32(x)
#define BE16_TO_CPU(x)		SWAP16(x)

#define CPU_TO_LE32(x)		(x)
#define CPU_TO_LE16(x)		(x)
#define LE32_TO_CPU(x)		(x)
#define LE16_TO_CPU(x)		(x)

#else
#error "_BYTE_ORDER Unknown"
#endif /* _BYTE_ORDER ==_BIG_ENDIAN */

#define cpu_to_le32 CPU_TO_LE32
#define cpu_to_le16 CPU_TO_LE16
#define be32_to_cpu BE32_TO_CPU
#define be16_to_cpu BE16_TO_CPU

/* unalligned little endian access */     
#define LE16_READ(p)		((UINT16)((((UINT8 *)(p))[0]) | (((UINT8 *)(p))[1] <<  8)))

#define LE16_WRITE(a, val)			\
	do										\
	{										\
		(a)[1] = ((UINT16) (val)) >> 8;	\
		(a)[0] = ((UINT16) (val)) & 0xff;	\
	}										\
	while (0)

#define LE32_READ(p)					\
    ((UINT32)							\
    ((((UINT8 *)(p))[0]) |				\
    (((UINT8 *)(p))[1] <<  8) |			\
    (((UINT8 *)(p))[2] << 16) |			\
    (((UINT8 *)(p))[3] << 24)))

#define BE32_READ(a) ((((UINT32) (a)[0]) << 24) | (((UINT32) (a)[1]) << 16) | \
			 			(((UINT32) (a)[2]) << 8) | ((UINT32) (a)[3]))

#define BE32_WRITE(a, val)				\
	do												\
	{												\
			(a)[0] = (UINT8) (((UINT32) (val)) >> 24);	\
			(a)[1] = (UINT8) (((UINT32) (val)) >> 16);	\
			(a)[2] = (UINT8) (((UINT32) (val)) >> 8);	\
			(a)[3] = (UINT8) (((UINT32) (val)) & 0xff);	\
	}												\
	while (0)

#define BE64_WRITE(a, val)				\
	do												\
	{												\
			(a)[0] = (UINT8) (((UINT64) (val)) >> 56);	\
			(a)[1] = (UINT8) (((UINT64) (val)) >> 48);	\
			(a)[2] = (UINT8) (((UINT64) (val)) >> 40);	\
			(a)[3] = (UINT8) (((UINT64) (val)) >> 32);	\
			(a)[4] = (UINT8) (((UINT64) (val)) >> 24);	\
			(a)[5] = (UINT8) (((UINT64) (val)) >> 16);	\
			(a)[6] = (UINT8) (((UINT64) (val)) >> 8); \
			(a)[7] = (UINT8) (((UINT64) (val)) & 0xff);	\
	}												\
	while (0)

#endif /* _WLAN_COMMON_H_ */
