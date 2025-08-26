/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2017, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#ifndef __MT_BMAC_H__
#define __MT_BMAC_H__

#define BMAC_RMAC_INFO_BASE_SIZE 32
#define BMAC_RMAC_INFO_GRP_1_SIZE 16
#define BMAC_RMAC_INFO_GRP_2_SIZE 16
#define BMAC_RMAC_INFO_GRP_3_SIZE 16
#define BMAC_RMAC_INFO_GRP_4_SIZE 16
#define BMAC_RMAC_INFO_GRP_5_SIZE 96

/*
 *Bit[1]: padding mode selection
 * 1'b0: pad the dummy bytes in the tail of header
 * 1'b1: pad the dummy bytes in the head of header
 *Bit[0]: padding 2 byte length
*/
#define BMAC_TXD_HDR_PAD_MODE_TAIL	0
#define BMAC_TXD_HDR_PAD_MODE_HEAD	1

/*
 * 2'b00: non-802.11 frame (Ethernet / 802.3)
 * 2'b01: CMD frame
 * 2'b10: 802.11 frame (normal mode)
 * 2'b11: 802.11 frame (enhancement mode)
 */
enum {
	BMAC_HF_NON_802_11_FRAME,
	BMAC_HF_CMD_FRAME,
	BMAC_HF_802_11_FRAME,
	BMAC_HF_802_11_EN_FRAME,
};

/*
 * 0 : not a bf control packet
 * 1 : NDP
 * 2 : NDP announcement frame (11n) or NDPA (11ac)
 * 3 : Sounding PPDU
 */
enum {
	BMAC_BF_NDP = 1,
	BMAC_BF_NDPA = 2,
	BMAC_BF_SD = 3,
};

/*
 * 2'h0: No fragment packet
 * 2'h1: the 1st fragment packet
 * 2'h2: the middle fragment packet
 * 2'h3: the last fragment packet
 */
enum {
	BMAC_NO_FRAG,
	BMAC_FIRST_FRAG,
	BMAC_MIDDLE_FRAG,
	BMAC_LAST_FRAG,
};

/*
 * tid_mgmt_type - when TXD.type=MGMT
 * TM/ADD_BA
 * b'0: Timing Measurement
 * b'1: ADD BA frame
 */
/*
 * TXD.type = DATA
 * | bit | description
 * | 3-0 | TID
 * TXD.type = MGMT
 * | bit | description
 * | 3	 | bip
 * | 2-0 | mgmt type
 * 0: normal management
 * 1: Timing Measurement
 * 2: ADD BA frame
 */
enum {
	BMAC_MGMT_NORMAL = 0x00,
	BMAC_MGMT_TM = 0x01,
	BMAC_MGMT_ADD_BA = 0x02,
	BMAC_MGMT_BIP = 0x08
};

/* GROUP VLD
 * Bit[0] indicates GROUP1 (DW10~DW13)
 * Bit[1] indicates GROUP2 (DW14~DW15)
 * Bit[2] indicates GROUP3 (DW16~DW17)
 * Bit[3] indicates GROUP4 (DW6~DW9)
 * Bit[4] indicates GROUP5 (DW18~DW33)
 */
enum {
	BMAC_GROUP_VLD_1 = 0x01,
	BMAC_GROUP_VLD_2 = 0x02,
	BMAC_GROUP_VLD_3 = 0x04,
	BMAC_GROUP_VLD_4 = 0x08,
	BMAC_GROUP_VLD_5 = 0x10,
};

/*
 * 2'b00: 1 page (16 DW)
 * 2'b01: 2 page (32 DW)
 * 2'b10: 3 page (48 DW)
 * 2'b11: 4 page (64 DW)
 */
#define BMAC_TXDLEN_PAGE_SIZE 64
enum {
	BMAC_TXDLEN_1_PAGE,
	BMAC_TXDLEN_2_PAGE,
	BMAC_TXDLEN_3_PAGE,
	BMAC_TXDLEN_4_PAGE,
};

enum bmac_pkt_ft_hif {
	BMAC_FT_HIF_CTD = 0, /* Cut-through */
	BMAC_FT_HIF_SF,      /* Store & forward  */
	BMAC_FT_HIF_CMD,     /* Command frame to N9/CR4 */
	BMAC_FT_HIF_PP,      /* PP redirect mode (driver to PSE to MCU's N9) */
};
enum bmac_pkt_ft_mcu {
	BMAC_FT_MCU_CTD = 0, /* N9 Cut-through */
	BMAC_FT_MCU_SF,      /* N9 Store & forward  */
	BMAC_FT_MCU_RSV,     /* Reserved */
	BMAC_FT_MCU_FW,      /* PP redirect mode (MCU's N9 to PSE to driver) */
};

struct GNU_PACKED bmac_txd {
	UINT32 txd_0;
	UINT32 txd_1;
	UINT32 txd_2;
	UINT32 txd_3;
	UINT32 txd_4;
	UINT32 txd_5;
	UINT32 txd_6;
	UINT32 txd_7;
};

struct GNU_PACKED bmac_rxd_grp_0 {
	UINT32 rxd_0;
	UINT32 rxd_1;
	UINT32 rxd_2;
	UINT32 rxd_3;
	UINT32 rxd_4;
	UINT32 rxd_5;
	UINT32 rxd_6;
	UINT32 rxd_7;
};

struct GNU_PACKED bmac_rxd_grp_1 {
	UINT32 rxd_12;
	UINT32 rxd_13;
	UINT32 rxd_14;
	UINT32 rxd_15;
};

struct GNU_PACKED bmac_rxd_grp_2 {
	UINT32 rxd_16;
	UINT32 rxd_17;
	UINT32 rxd_18;
	UINT32 rxd_19;
};

struct GNU_PACKED bmac_rxd_grp_3 {
	UINT32 rxd_20;
	UINT32 rxd_21;
	UINT32 rxd_22;
	UINT32 rxd_23;
};

struct GNU_PACKED bmac_rxd_grp_4 {
	UINT32 rxd_8;
	UINT32 rxd_9;
	UINT32 rxd_10;
	UINT32 rxd_11;
};

struct GNU_PACKED bmac_rxd_grp_5 {
	UINT32 rxd_24;
	UINT32 rxd_25;
	UINT32 rxd_26;
	UINT32 rxd_27;
	UINT32 rxd_28;
	UINT32 rxd_29;
	UINT32 rxd_30;
	UINT32 rxd_31;
	UINT32 rxd_32;
	UINT32 rxd_33;
	UINT32 rxd_34;
	UINT32 rxd_35;
	UINT32 rxd_36;
	UINT32 rxd_37;
	UINT32 rxd_38;
	UINT32 rxd_39;
	UINT32 rxd_40;
	UINT32 rxd_41;
	UINT32 rxd_42;
	UINT32 rxd_43;
	UINT32 rxd_44;
	UINT32 rxd_45;
	UINT32 rxd_46;
	UINT32 rxd_47;
};

struct GNU_PACKED bmac_txs_header {
	UINT32 txs_h_0;
	UINT32 txs_h_1;
	UINT32 txs_h_2;
	UINT32 txs_h_3;
};

struct bmac_empty_queue_info {
	PCHAR QueueName;
	UINT32 Portid;
	UINT32 Queueid;
	UINT32 tgid;
};

#define BMAC_TXS_FRAME_DW 12

VOID mtb_dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info);
VOID mtb_dump_rmac_info_for_ICVERR(
	RTMP_ADAPTER *pAd, UCHAR *rmac_info);
VOID mtb_dump_rxinfo(RTMP_ADAPTER *pAd, UCHAR *rxinfo);
VOID mtb_write_tmac_info_fixed_rate(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR *tmac_info,
	MAC_TX_INFO *info,
	union _HTTRANSMIT_SETTING *transmit);

#ifdef TX_POWER_CONTROL_SUPPORT_V2
VOID mtb_txpower_boost_v2(struct _RTMP_ADAPTER *pAd, UCHAR ucBandIdx);
VOID mtb_txpower_boost_ctrl_v2(
	struct _RTMP_ADAPTER *pAd, CHAR cPwrUpCat, PUCHAR pcPwrUpValue);
BOOLEAN mtb_txpower_boost_info_v2(struct _RTMP_ADAPTER *pAd, enum POWER_BOOST_TABLE_CATEGORY_V2 ePowerBoostRateType);
VOID mtb_txpower_boost_profile_v2(struct _RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *pBuffer);
#endif /* TX_POWER_CONTROL_SUPPORT_V2 */

#endif /* __MT_BMAC_H__ */
