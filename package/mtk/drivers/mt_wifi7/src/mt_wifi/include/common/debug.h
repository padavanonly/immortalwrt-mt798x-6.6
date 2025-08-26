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
/****************************************************************************
	****************************************************************************

    Module Name:
	debug.h

    Abstract:
	All function prototypes and macro are provided from debug message.

    Revision History:
    Who             When            What
    ---------    ----------    ----------------------------------------------
    Name           Date              Modification logs
    UnifyLOGTF   2014.07.11     Initial version

***************************************************************************/

#ifndef __DEBUG_H__
#define __DEBUG_H__


/* */
/*  Debug information verbosity: lower values indicate higher urgency */
/* */

/* Debug Level */
#define DBG_LVL_OFF		0
#define DBG_LVL_ERROR	1 /* ERROR level in RFC5424 */
#define DBG_LVL_WARN	2 /* WARN level in RFC5424 */
#define DBG_LVL_NOTICE	3 /* NOTICE level in RFC5424 */
#define DBG_LVL_INFO	4 /* INFO level in RFC5424 */
#define DBG_LVL_DEBUG	5 /* DEBUG level in RFC5424 */
#define DBG_LVL_MAX		DBG_LVL_DEBUG
#if !defined(EVENT_TRACING)
/* Debug Category */
/* if change the definition of below category or update new category, please update cat_str and sub_cat_str in cmm_info.c */
#define DBG_CAT_MISC     0 /* misc */
#define DBG_CAT_INIT     1 /* initialization/shutdown */
#define DBG_CAT_HW       2 /* MAC/BBP/RF/Chip */
#define DBG_CAT_FW       3 /* FW related command, response, CR that FW care about */
#define DBG_CAT_HIF      4 /* Host interface: usb/pcie/rbus */
#define DBG_CAT_FPGA     5 /* FPGA Chip verify, DVT */
#define DBG_CAT_TEST     6 /* ATE, QA, UT, FPGA?, TDT, SLT, WHQL, and other TEST */
#define DBG_CAT_RA       7 /* Rate Adaption/Throughput related */
#define DBG_CAT_AP       8 /* AP, MBSS, WDS */
#define DBG_CAT_CLIENT   9 /* STA, ApClient, AdHoc, Mesh */
#define DBG_CAT_TX       10 /* Tx data path */
#define DBG_CAT_RX       11 /* Rx data path */
#define DBG_CAT_CFG      12 /* ioctl/oid/profile/cfg80211/Registry */
#define DBG_CAT_MLME     13 /* 802.11 fundamental connection flow, auth, assoc, disconnect, etc */
#define DBG_CAT_PROTO    14 /* protocol, ex. TDLS */
#define DBG_CAT_SEC      15 /* security/key/WPS/WAPI/PMF/11i related*/
#define DBG_CAT_PS       16 /* power saving/UAPSD */
#define DBG_CAT_POWER    17 /* power Setting, Single Sku, Temperature comp, etc */
#define DBG_CAT_COEX     18 /* BT, BT WiFi Coex, LTE, TVWS*/
#define DBG_CAT_P2P      19 /* P2P, Miracast */
#define DBG_CAT_TOKEN	 20
#define DBG_CAT_CMW      21 /* CMW Link Test related */
#define DBG_CAT_BF       22 /* BF */
#define DBG_CAT_CHN      23 /* Channel related; Channel/ACS/DFS/Scan/SCS/CSI */
#define DBG_CAT_MLO      24 /* Multi-Link Operation */
#define DBG_CAT_CCN34    25 /* CCN34 related */
#define DBG_CAT_INTF     26
#define DBG_CAT_ANDLINK  27 /* ANDLINK related */
#define DBG_CAT_CFG80211 28
#define DBG_CAT_MLR	 29
#define DBG_CAT_CCN67     30 /* CCN67 related */
#define DBG_CAT_RSV2     31 /* reserved index for code development */
#define DBG_CAT_MAX      31
#define DBG_CAT_ALL      DBG_CAT_MISC
#define DBG_CAT_EN_ALL_MASK 0xFFFFFFFFu
#endif

/* Debug SubCategory */
/* if change the definition of below subcategory or update new subcategory, please update cat_str and sub_cat_str in cmm_info.c */

#define DBG_SUBCAT_ALL	DBG_SUBCAT_MISC

#define DBG_SUBCAT_EN_ALL_MASK	0xFFFFFFFFu
#define DBG_SUBCAT_DIS_ALL_MASK	0x00000000u

#define DBG_SUBCAT_MISC		0x00000001u /* misc for all category */

/* SUb-Category of DBG_CAT_INIT */
#define CATINIT_INTF		0x00000002u /* Interface related */
#define CATINIT_TASK		0x00000004u /* Task thread init related */
#define CATINIT_TRCTRL		0x00000008u /* Interface related */
#define CATINIT_MLME		0x00000010u /* Mlme task related */
#define CATINIT_EEPROM		0x00000020u /* Eeprom init related */

/* SUb-Category of DBG_CAT_TEST */
#define CATTEST_RFEATURE    0x00000002u
#define CATTEST_PRECFG      0x00000004u
#define CATTEST_ATE         0x00000008u
#define CATTEST_BF          0x00000010u
#define CATTEST_MDVT        0x00000020u
#define CATTEST_SDO         0x00000040u
#define CATTEST_PROFILING   0x00000080u
#define CATTEST_RVR         0x00000100u
#define CATTEST_SANITY      0x00000200u
#define CATTEST_DIAG        0x00000400u
#define CATTEST_OOM         0x00000800u

/* SUb-Category of DBG_CAT_RA */
#define CATRA_CFG		0x00000001u
#define CATRA_MLR               0x00000004u /* MTK Long Range */

/* Sub-Category of  DBG_CAT_HW */
#define CATHW_SA		0x00000002u /* debug flag for smart antenna */
#define CATHW_SER		0x00000004u /* debug flag for SER */
#define CATHW_PHY		0x00000008u /* debug flag for PHY related */
#define CATHW_MAC		0x00000010u /* debug flag for MAC related */
#define CATHW_RXV		0x00000020u /* debug flag for RXV related */
#define CATHW_RXPHY		0x00000040u /* debug flag for RXPHY related */
#define CATHW_CMD_CTRL		0x00000080u /* debut flag for hw cmd queu handle */
#define CATHW_HDEV_CTRL		0x00000100u /* debut flag for hdev ctrl handle */
#define CATHW_EEPROM		0x00000200u /* debug flag for eeprom read/write */
#define CATHW_GREENAP		0x00000400u /* debug flag for green ap */

/* Sub-Category of  DBG_CAT_FW */
#define CATFW_DEVINFO		0x00000002u /* fwcmd of devinfo */
#define CATFW_BSSINFO		0x00000004u /* fwcmd of bssinfo */
#define CATFW_STAREC		0x00000008u /* fwcmd of starec */
#define CATFW_MLD		0x00000010u
#define CATFW_EHT		0x00000020u
#define CATFW_FR_TABLE		0x00000040u
#define CATFW_COUNTER_INFO	0x00000080u
#define CATFW_11V_MBSS		0x00000100u
#define CATFW_EFUSE		0x00000200u /* EFUSE, E2PROM, EEFlash */
#define CATFW_DBGINFO		0x00000400u
#define CATFW_SR		0x00000800u
#define CATFW_PHY		0x00001000u
#define CATFW_EAP		0x00002000u /* Enterprise AP */
#define CATFW_SDO		0x00004000u /* SDO config */

/* Sub-Category of  DBG_CAT_HIF */
#define CATHIF_PCI		0x00000002u
#define CATHIF_USB		0x00000004u
#define CATHIF_HWIFI		0x00000008u
#define CATHIF_IO		0x00000010u

/* Sub-Category of  DBG_CAT_AP */
#define CATAP_MBSS		0x00000002u
#define CATAP_WDS		0x00000004u
#define CATAP_BCN		0x00000008u
#define CATAP_DISC		0x00000010u /* AP 6G discovery */
#define CATAP_KPLIVE		0x00000020u
#define CATAP_11V_MBSS		0x00000040u
#define CATAP_TR181		0x00000080u
#define CATAP_DISCON		0x00000100u /* STA disconnect from AP */
#define CATAP_ACL		0x00000200u /* access control list */
#define CATAP_COSR		0x00000400u /* access control list */
#define CATAP_MNT		0x00000800u /* air monitor */
#define CATAP_WAPP		0x00001000u /* Wifi APP, MAP, MBO */
#define CATAP_STEERING		0x00002000u /* band steering */
#define CATAP_IDS		0x00004000u
#define CATAP_QBSS		0x00008000u
#define CATAP_SEC		0x00010000u
#define CATAP_BMC_SN		0x00020000u /* BMC SN */
#define CATAP_VOW		0x00040000u /* debug flag for VOW */
#define CATAP_RECONF_FLOW		0x00080000u
#define CATAP_BPCC		0x00100000u

/* Sub-Category of  DBG_CAT_CLIENT */
#define CATCLIENT_ADHOC		0x00000002u
#define CATCLIENT_APCLI		0x00000004u
#define CATCLIENT_MESH		0x00000008u
#define CATCLIENT_LP		0x00000010u
#define CATCLIENT_ROAMING	0x00000020u

/* Sub-Category of  DBG_CAT_TX */
#define CATTX_TMAC		0x00000002u	/* debug flag for tmac info dump */
#define CATTX_TXOP		0x00000004u	/* debug flag for txop info dump */
#define CATTX_FRAG_TX		0x00000008u	/* debug flag for frag tx info dump */
#define CATTX_VLAN		0x00000010u	/* debug flag for vlan dump */
#define CATTX_MGMT		0x00000020u	/* debug flag for mgmt dump */
#define CATTX_DATA		0x00000040u	/* debug flag for data dump */
#define CATTX_SEC_TX		0x00000080u	/* debug flag for sec tx dump */
#define CATTX_WMM		0x00000100u	/* debug flag for wmm dump */
#define CATTX_DABS_QOS		0x00000200u	/* debug flag for qabs qos dump */
#define CATTX_4ADDR		0x00000400u	/* debug flag for 4-address handle dump */
#define CATTX_VOW		0x00000800u     /* debug flag for vow dump */
#define CATTX_CNT		0x00001000u	/* debug flag for counter info dump */
#define CATTX_RED		0x00002000u	/* debug flag for red dump */
#define CATTX_PREAMBLE	0x00004000u	/* debug flag for preamble dump */
#define CATTX_POWER		0x00008000u	/* debug flag for power */

/* Sub-Category of  DBG_CAT_RX */
#define CATRX_FRAG_RX		0x00000002u	/* debug flag for frag rx info dump */
#define CATRX_SEC_RX		0x00000004u	/* debug flag for sec rx info dump */
#define CATRX_AMSDU		0x00000008u	/* debug flag for amsdu info dump */
#define CATRX_AMPDU		0x00000010u	/* debug flag for ampdu rx info dump */
#define CATRX_802_11D		0x00000020u	/* debug flag for 802.11 data info dump */
#define CATRX_802_3D		0x00000040u	/* debug flag for 802.3 data info dump */
#define CATRX_MGMT		0x00000080u	/* debug flag for mgmt frame info dump */
#define CATRX_CTRL		0x00000100u	/* debug flag for contl frame info dump */
#define CATRX_RXINFO		0x00000200u	/* debug flag for rx check info dump */
#define CATRX_CNT		0x00000400u	/* debug flag for counter info dump */
#define CATRX_VOW		0x00000800u	/* debug flag for vow dump */

/* Sub-Category of  DBG_CAT_CFG */
#define CATCFG_TX		0x00000002u	/* debug flag for cfg tx info dump */
#define CATCFG_INTF		0x00000004u	/* debug flag for cfg interface info dump */
#define CATCFG_CMD		0x00000008u	/* debug flag for cmd info dump */
#define CATCFG_DBGLOG		0x00000010u	/* debug flag for debug log setting */
#define CATCFG_PROFILE		0x00000020u	/* debug flag for profile setting */
#define CATCFG_SR		0x00000040u	/* debug flag for sr cmd setting */
#define CATCFG_PP		0x00000080u	/* debug flag for pp cmd setting */
#define CATCFG_VENDOR	0x00000100u	/* debug flag for vendor setting */


/* Sub-Category of  DBG_CAT_MLME */
#define CATMLME_WTBL		0x00000002u	/* debug flag for wtbl */
#define CATMLME_TXOP		0x00000004u	/* debug flag for txop information */
#define CATMLME_AIFS		0x00000008u	/* debug flag for aifs information */
#define CATMLME_RX_DROP		0x00000010u	/* debug flag for rx drop information */
#define CATMLME_IE_INFO		0x00000020u	/* debug flag for some basic IE information */
#define CATMLME_AUTH		0x00000040u	/* debug flag for auth in MLME */
#define CATMLME_ASSOC		0x00000080u	/* debug flag for assoc in MLME */
#define CATMLME_CNTL		0x00000100u	/* debug flag for cntl in MLME */
#define CATMLME_SYNC		0x00000200u	/* debug flag for sync in MLME */
#define CATMLME_STAT		0x00000400u	/* debug flag for statistics in MLME */
#define CATMLME_ROUTINE		0x00000800u	/* debug flag for MlmePeriodicExec, MlmeHandle */
#define CATMLME_DOT1X		0x00001000u	/* debug flag for DOT1X, EAP, EAPOL related */
#define CATMLME_BSSENTRY	0x00002000u	/* debug flag for bss entry */

/*  Sub-Category of DBG_CAT_TOKEN */
#define TOKEN_INFO		0x00000002u
#define TOKEN_PROFILE		0x00000004u
#define TOKEN_TRACE		0x00000008u

/* Sub-Category of  DBG_CAT_PROTO */
#define CATPROTO_ACM            0x00000002u
#define CATPROTO_BA             0x00000004u /* 802.11e, Block ACK */
#define CATPROTO_TDLS           0x00000008u
#define CATPROTO_WNM            0x00000010u /* 802.11v */
#define CATPROTO_IGMP           0x00000020u
#define CATPROTO_MAT            0x00000040u
#define CATPROTO_RRM            0x00000080u /* 802.11k */
#define CATPROTO_DFS            0x00000100u /* 802.11h */
#define CATPROTO_FT             0x00000200u /* 802.11r */
#define CATPROTO_SCAN           0x00000400u
#define CATPROTO_FTM            0x00000800u
#define CATPROTO_OCE            0x00001000u
#define CATPROTO_TWT            0x00002000u
#define CATPROTO_COLOR          0x00004000u
#define CATPROTO_EHT            0x00008000u
#define CATPROTO_ACTION         0x00010000u
#define CATPROTO_HS_R2          0x00020000u
#define CATPROTO_HT             0x00040000u
#define CATPROTO_VHT            0x00080000u
#define CATPROTO_HE             0x00100000u
#define CATPROTO_DPP            0x00200000u
#define CATPROTO_AFC            0x00400000u
#define CATPROTO_MBO            0x00800000u
#define CATPROTO_MAP            0x01000000u
#define CATPROTO_QOS_MAP        0x02000000u
#define CATPROTO_BND_STRG       0x04000000u
#define CATPROTO_WHC            0x08000000u
#define CATPROTO_BTM            0x10000000u /* 802.11v, BSS Transition Management */

/* Sub-Category of  DBG_CAT_SEC */
#define CATSEC_KEY      0x00000002u
#define CATSEC_WPS      0x00000004u /* WPS, WSC, NFC */
#define CATSEC_WAPI     0x00000008u
#define CATSEC_PMF      0x00000010u /* 802.11w, PMF, MFP */
#define CATSEC_SAE      0x00000020u
#define CATSEC_SUITEB   0x00000040u
#define CATSEC_OWE      0x00000080u
#define CATSEC_ECC      0x00000100u
#define CATSEC_BCNPROT  0x00000200u
#define CATSEC_OCV      0x00000400u
#define CATSEC_MLO      0x00000800u
#define CATSEC_ALG      0x00001000u /* algorithm */
#define CATSEC_PMK      0x00002000u
#define CATSEC_WEP      0x00004000u
#define CATSEC_WPA      0x00008000u
#define CATSEC_AES      0x00010000u
#define CATSEC_TKIP     0x00020000u
#define CATSEC_FIPS     0x00040000u /* FIPS 140-3 */
#define CATSEC_RSNO     0x00080000u /* RSN override */

/* Sub-Category of  DBG_CAT_PS */
#define CATPS_UAPSD		0x00000002u
#define CATPS_CFG		0x00000004u
#define CATPS_LP		0x00000008u /* ps handle for low power */
#define CATPS_GREENAP		0x00000010u /* green ap */

/* Sub-Category of  DBG_CAT_POWER */
#define CATPOWER_SKU		0x00000002u

/* Sub-Category of  DBG_CAT_COEX */
#define CATCOEX_MD		0x00000002u

/* Sub-Category of  DBG_CAT_CMW */
#define CATCMW_LINK		0x00000002u

/* Sub-Category of  DBG_CAT_BF */
#define CATBF_IWCMD		0x00000002u
#define CATBF_ASSOC		0x00000004u
#define CATBF_SOUND		0x00000008u
#define CATBF_CFG		0x00000010u
#define CATBF_EVENT_INFO	0x00000020u

/* Sub-Category of  DBG_CAT_CHN */
#define CATCHN_ACS	0x00000002u
#define CATCHN_DFS	0x00000004u
#define CATCHN_SCAN	0x00000008u
#define CATCHN_UNSAFE	0x00000010u
#define CATCHN_CHN	0x00000020u
#define CATCHN_SCS	0x00000040u
#define CATCHN_CSI	0x00000080u


/* Sub-Category of  DBG_CAT_MLO */
#define CATMLO_BMGR     0x00000002u
#define CATMLO_CONN     0x00000004u
#define CATMLO_AUTH     0x00000008u
#define CATMLO_ASSOC    0x00000010u
#define CATMLO_ENTRY    0x00000020u
#define CATMLO_WDS      0x00000040u
#define CATMLO_CFG      0x00000080u
#define CATMLO_BTM      0x00000100u
#define CATMLO_T2LM     0x00000200u
#define CATMLO_RECONFIG	0x00000400u
#define CATMLO_BPCC     0x00000800u

/* Sub-Category of  DBG_CAT_CCN34 */
#define CATCCN34_RX     0x00000002u
#define CATCCN34_TX     0x00000004u
#define CATCCN34_CFG    0x00000008u

/* Sub-Category of  DBG_CAT_CCN67 */
#define CATCCN67_RX     0x00000002u
#define CATCCN67_TX     0x00000004u
#define CATCCN67_CFG    0x00000008u
#define CATCCN67_DSAE   0x00000010u

/* SUb-Category of DBG_CAT_INTF */
#define CATINTF_UP      0x00000002u	/* interface init, up */
#define CATINTF_DOWN    0x00000004u	/* interface down, close */
#define CATINTF_UPDATE  0x00000008u	/* interface update */
#define CATINTF_CTRL    0x00000010u	/* interface control */

/* Sub-Category of  DBG_CAT_ANDLINK */
#define CATANDLINK_CFG  0x00000002u
#define CATANDKINK_TX   0x00000004u

/* Sub-Category of  DBG_CAT_CFG80211 */
#define CATCFG80211_AP		0x00000002u
#define CATCFG80211_STA		0x00000004u
#define CATCFG80211_CMM		0x00000008u
#define CATCFG80211_INTF	0x00000010u

/* Sub-Category of  DBG_CAT_MLR */
#define CATMLR_CFG		0x00000002u
#define CATMLR_TX		0x00000004u
#define CATMLR_FRAG		0x00000008u
#define CATMLR_EVENT		0x00000010u

/***********************************************************************************
 *	Debugging and printing related definitions and prototypes
 ***********************************************************************************/
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define PRINT_IPV4(addr) \
	addr[0], addr[1], addr[2], addr[3]

#define PRINT_IPV6(addr) \
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], \
	addr[8], addr[9], addr[10], addr[11], addr[12], addr[13], addr[14], addr[15]

#define IPV4STR "%d.%d.%d.%d"

#define IPV6STR \
	"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"

#ifdef MASK_PARTIAL_MACADDR
#define MACSTR "%02x:**:**:%02x:%02x:%02x"
#define MAC2STR(addr) (addr)[0], (addr)[3], (addr)[4], (addr)[5]
#else
#define MACSTR "%pM"
#define MAC2STR(addr) addr
#endif

extern int			DebugLevel;
extern UINT32		DebugCategory;
extern UINT32		DebugSubCategory[DBG_LVL_MAX + 1][32];

#ifdef DBG
#ifdef FW_LOG_DUMP
#define HOST_DBG(pAd, ...) host_dbg_prt(pAd, ##__VA_ARGS__)
#define HOST_HEXDUMP(pAd, str, va, size) host_dbg_hexdump(pAd, str, va, size)
#else
#define HOST_DBG(pAd, ...)
#define HOST_HEXDUMP(pAd, str, va, size)
#endif

#ifdef DBG_ENHANCE
#define MTWF_DBG(pAd, Category, SubCategory, Level, ...)	\
	do {	\
		if (((0x1 << Category) & DebugCategory)	\
			&& (SubCategory & DebugSubCategory[Level][Category]))	\
			mtwf_dbg_prt(pAd,Category,Level,__func__,__LINE__,##__VA_ARGS__);\
	} while (0)
#else
#define MTWF_DBG(pAd, Category, SubCategory, Level, ...)	\
	do {	\
		if (((0x1 << Category) & DebugCategory)		\
			&& (SubCategory & DebugSubCategory[Level][Category]))	\
			MTWF_PRINT(__VA_ARGS__);	\
	} while (0)
#endif

/* Printing log without prefix, NP: No prefix */
#define MTWF_DBG_NP(Category, SubCategory, Level, ...)	\
	do {	\
		if (((0x1 << Category) & DebugCategory)		\
			&& (SubCategory & DebugSubCategory[Level][Category]))	\
			MTWF_PRINT(__VA_ARGS__);	\
	} while (0)

#else
#define MTWF_DBG(pAd, Category, SubCategory, Level, ...)
#define MTWF_DBG_NP(Category, SubCategory, Level, ...)
#define HOST_DBG(pAd, ...)
#define HOST_HEXDUMP(pAd, str, va, size)
#endif

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void hex_dump_with_lvl(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen, int dbglvl);
void hex_dump_with_cat_and_lvl(char *str, UCHAR *pSrcBufVA, UINT SrcBufLen, INT dbgcat, INT dbg_sub_cat, INT dbglvl);
void hex_dump_always(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
void hex_dump32(char *str,	UINT32 *pu4StartAddr,	UINT32 u4Length);


enum {
	HOST_HELP,
	HOST_DBG_INFO,
	TX_FREE_NOTIFY_HOST_INFO,
	WFDMA_INFO,
	COUNTER_INFO,
	RX_PPS,
};

enum {
	WACPU_HELP,
	WACPU_DBG_INFO,
	MSDU_DROP_INFO,
	AC_TAIL_DROP_INFO,
	BSS_TABLE_INFO,
	STAREC_INFO,
	TX_FREE_NOTIFY_WACPU_INFO,
	CTXD_INFO,
	IGMP_INFO,
	IGMP_WHITE_LIST_INFO,
	SDO_INFO,
};

enum {
	WOCPU_HELP,
	WOCPU_DBG_INFO,
	WOCPU_DEV_INFO,
	WOCPU_BSS_INFO,
	WOCPU_STA_REC,
	WOCPU_BA_INFO,
	WOCPU_FBCMD_Q_INFO,
	WOCPU_RX_STAT
};

#endif /* __DEBUG_H__ */

