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
    dot11_base.h

    Abstract:
	Defined IE/frame structures of 802.11 base line

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
*/


#ifndef _DOT11_BASE_H_
#define _DOT11_BASE_H_

#include "rtmp_type.h"
#include "dot11_eid.h"



/* value domain of 802.11 header FC.Tyte, which is b3..b2 of the 1st-byte of MAC header */
#define FC_TYPE_MGMT	0
#define FC_TYPE_CNTL	1
#define FC_TYPE_DATA	2
#define FC_TYPE_RSVED	3

/* value domain of 802.11 MGMT frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_ASSOC_REQ           0
#define SUBTYPE_ASSOC_RSP           1
#define SUBTYPE_REASSOC_REQ         2
#define SUBTYPE_REASSOC_RSP         3
#define SUBTYPE_PROBE_REQ           4
#define SUBTYPE_PROBE_RSP           5
#define SUBTYPE_TIMING_ADV			6
#define SUBTYPE_BEACON              8
#define SUBTYPE_ATIM                9
#define SUBTYPE_DISASSOC            10
#define SUBTYPE_AUTH                11
#define SUBTYPE_DEAUTH              12
#define SUBTYPE_ACTION              13
#define SUBTYPE_ACTION_NO_ACK		14

/* value domain of 802.11 CNTL frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_TRIGGER				2
#define SUBTYPE_VHT_NDPA			5
#define SUBTYPE_WRAPPER				7
#define SUBTYPE_BLOCK_ACK_REQ       8
#define SUBTYPE_BLOCK_ACK           9
#define SUBTYPE_PS_POLL             10
#define SUBTYPE_RTS                 11
#define SUBTYPE_CTS                 12
#define SUBTYPE_ACK                 13
#define SUBTYPE_CFEND               14
#define SUBTYPE_CFEND_CFACK         15
/* TODO: shiang-MT7615, remove this because SUBTYPE_XXX only used for SPEC defined!! */
#define SUBTYPE_ALL					16

/* value domain of 802.11 DATA frame's FC.subtype, which is b7..4 of the 1st-byte of MAC header */
#define SUBTYPE_DATA                0
#define SUBTYPE_DATA_CFACK          1
#define SUBTYPE_DATA_CFPOLL         2
#define SUBTYPE_DATA_CFACK_CFPOLL   3
#define SUBTYPE_DATA_NULL           4
#define SUBTYPE_CFACK               5
#define SUBTYPE_CFPOLL              6
#define SUBTYPE_CFACK_CFPOLL        7
#define SUBTYPE_QDATA               8
#define SUBTYPE_QDATA_CFACK         9
#define SUBTYPE_QDATA_CFPOLL        10
#define SUBTYPE_QDATA_CFACK_CFPOLL  11
#define SUBTYPE_QOS_NULL            12
#define SUBTYPE_QOS_CFACK           13
#define SUBTYPE_QOS_CFPOLL          14
#define SUBTYPE_QOS_CFACK_CFPOLL    15

/* 2-byte Frame control field */
typedef struct GNU_PACKED {
#ifdef CFG_BIG_ENDIAN
	UINT16 Order:1;		/* Strict order expected */
	UINT16 Wep:1;		/* Wep data */
	UINT16 MoreData:1;	/* More data bit */
	UINT16 PwrMgmt:1;	/* Power management bit */
	UINT16 Retry:1;		/* Retry status bit */
	UINT16 MoreFrag:1;	/* More fragment bit */
	UINT16 FrDs:1;		/* From DS indication */
	UINT16 ToDs:1;		/* To DS indication */
	UINT16 SubType:4;	/* MSDU subtype */
	UINT16 Type:2;		/* MSDU type */
	UINT16 Ver:2;		/* Protocol version */
#else
	UINT16 Ver:2;		/* Protocol version */
	UINT16 Type:2;		/* MSDU type, refer to FC_TYPE_XX */
	UINT16 SubType:4;	/* MSDU subtype, refer to  SUBTYPE_XXX */
	UINT16 ToDs:1;		/* To DS indication */
	UINT16 FrDs:1;		/* From DS indication */
	UINT16 MoreFrag:1;	/* More fragment bit */
	UINT16 Retry:1;		/* Retry status bit */
	UINT16 PwrMgmt:1;	/* Power management bit */
	UINT16 MoreData:1;	/* More data bit */
	UINT16 Wep:1;		/* Wep data */
	UINT16 Order:1;		/* Strict order expected */
#endif	/* !CFG_BIG_ENDIAN */
} FRAME_CONTROL, *PFRAME_CONTROL;

#ifdef HWIFI_SUPPORT
#define IEEE80211_FCTL_VERS			0x0003
#define IEEE80211_FCTL_FTYPE		0x000c
#define IEEE80211_FCTL_STYPE		0x00f0
#define IEEE80211_FCTL_TODS			0x0100
#define IEEE80211_FCTL_FROMDS		0x0200
#define IEEE80211_FCTL_MOREFRAGS	0x0400
#define IEEE80211_FCTL_RETRY		0x0800
#define IEEE80211_FCTL_PM			0x1000
#define IEEE80211_FCTL_MOREDATA		0x2000
#define IEEE80211_FCTL_PROTECTED	0x4000
#define IEEE80211_FCTL_ORDER		0x8000
#define IEEE80211_FCTL_CTL_EXT		0x0f00
#endif /* HWIFI_SUPPORT */

typedef struct GNU_PACKED _HEADER_802_11 {
	FRAME_CONTROL   FC;
	UINT16          Duration;
	UCHAR           Addr1[6];
	UCHAR           Addr2[6];
	UCHAR		Addr3[6];
#ifdef CFG_BIG_ENDIAN
	UINT16		Sequence:12;
	UINT16		Frag:4;
#else
	UINT16		Frag:4;
	UINT16		Sequence:12;
#endif /* !CFG_BIG_ENDIAN */
	UCHAR		Octet[0];
} HEADER_802_11, *PHEADER_802_11;


typedef struct GNU_PACKED _HEADER_PS_POLL {
	FRAME_CONTROL FC;
	UINT16 AID;
	UCHAR Addr1[6];
	UCHAR Addr2[6];
} HEADER_PS_POLL;

/* 7.3.1.3 Beacon Interval field */
#define BEACON_INTERVAL_FIELD_LEN                   2
/* 7.3.1.4 Capability Information field */
#define CAP_INFO_FIELD_LEN                          2
/* 7.3.1.10 Timestamp field */
#define TIMESTAMP_FIELD_LEN                         8


/* Reduced Neighbor Report (RNR) */

/*
    just a default assume value,
    if Bcn length has so many IEs, shall enlarge the value.
*/
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
#define MAX_IE_V2_LENGTH                               1400
#endif
#define MAX_IE_LENGTH                               1400


#define SLOT_TIME_24G_LONG		20  /* 802.11b (DS), 802.11g (ERP), 802.11n */
#define SLOT_TIME_24G_SHORT	9	/* 802.11g, 802.11n */
#define SLOT_TIME_5G			9	/* 802.11a, 802.11n, 802.11ac */

#define SIFS_TIME_24G		10
#define SIFS_TIME_5G		16

#define RIFS_TIME		2	/* 802.11n */
#define EIFS_TIME		60	/* refine to 60 from 360 us, 2018.05.09 */

enum af {
	/* ht */
	AMPDU_LEN_8K,
	AMPDU_LEN_16K,
	AMPDU_LEN_32K,
	AMPDU_LEN_64K,
	/* vht */
	AMPDU_LEN_128K,
	AMPDU_LEN_256K,
	AMPDU_LEN_512K,
	AMPDU_LEN_1024K
};

enum amsdu_length {
	AMSDU_3839_OCTETS,
	AMSDU_7935_OCTETS
};

/* ht/vht/he */
enum rx_stbc {
	/* ht */
	RX_STBC_NOT_SUPPORT,
	RX_STBC_1SS,
	RX_STBC_2SS,
	RX_STBC_3SS,
	/* vht */
	RX_STBC_4SS
};

/*
	max_tx_pwr_cnt:
		0: Local Maximum Transmit Power For 20 MHz.
		1: Local Maximum Transmit Power For 20, 40MHz
		2: Local Maximum Transmit Power For 20, 40, 80MHz
		3: Local Maximum Transmit Power For 20, 40, 80, 160/80+80MHz
		4~7: rsv

	max_tx_pwr_interpretation:
		0: EIRP (11ac~)
		1: EIRP PSD (11ax~)
		2: Regulatory client EIRP (11ax~)
		3: Regulatory client EIRP PSD (11ax~)
		4~7: rsv
*/
#define TX_PWR_INTERPRET_EIRP                           0
#define TX_PWR_INTERPRET_EIRP_PSD                       1
#define TX_PWR_INTERPRET_REG_CLIENT_EIRP                2
#define TX_PWR_INTERPRET_REG_CLIENT_EIRP_PSD            3
#define TX_PWR_INTERPRET_ADDITIONAL_REG_CLIENT_EIRP     4
#define TX_PWR_INTERPRET_ADDITIONAL_REG_CLIENT_EIRP_PSD 5


struct GNU_PACKED _tx_pwr_info {
#ifdef CFG_BIG_ENDIAN
	u8 max_tx_pwr_category:2;
	u8 max_tx_pwr_interpretation:3;
	u8 max_tx_pwr_cnt:3;
#else
	u8 max_tx_pwr_cnt:3;
	u8 max_tx_pwr_interpretation:3;
	u8 max_tx_pwr_category:2;
#endif
};

/*
	Transmit Power Envelope element (TPE)

	max_txpwr: Maximum Transmit Power
		-> Define the maximum transmit power limit of the tx bandwidth defined
			by the VHT Transmit Power Envelop element. The Maximum Transmit
			Power field is a 8 bit 2's complement signed integer in the range of
			-64 dBm to 63.5 dBm with a 0.5 dB step.

	NOTE: The following two subfields may repeated as needed.
		center_freq_1: Channel Center Frequency Segment
		ch_seg_width: Segment Channel Width
*/
#define MAX_TX_POWER_COUNT 17
struct GNU_PACKED _tx_pwr_env_ie {
	struct _tx_pwr_info tx_pwr_info;
	u8 max_tx_pwr[MAX_TX_POWER_COUNT];
};

/*
	Bandwidth Indication element

	BW_IND_PARA(one byte): Bit 1 indicates dscb present or not, others are reserved.
	BW_IND_INFO(three or five byte): like EHT Operation element.


	NOTE:
	Based on 802.11be D3.0 description:
	This element is present for an EHT STA when channel switching or extended channel switching
	to an EHT BSS operating channel width wider than 160 MHz
	or to an EHT BSS operating channel width including at least one punctured 20 MHz subchannel.
	Otherwise, the Bandwidth Indication element is not present.
*/
#define DOT11BE_BW_IND_PARAM_DSCB_PRESENT_OFFSET 1
struct GNU_PACKED _bw_ind_ie {
	u8 bw_ind_param;
	u8 control;
	u8 ccfs0;
	u8 ccfs1;
	u16 dscb;
};

#endif /* _DOT11_BASE_H_ */

