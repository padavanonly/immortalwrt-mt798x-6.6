/*
* Copyright (c) [2020] MediaTek Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __MT7992_DBG_H
#define __MT7992_DBG_H

enum {
	WTBL_GENERIC,
	WTBL_RX,
	WTBL_HT,
	WTBL_VHT,
	WTBL_PEER_PS,		/* not used */
	WTBL_TX_PS,
	WTBL_HDR_TRANS,
	WTBL_SEC_KEY,
	WTBL_BA,
	WTBL_RDG,		/* obsoleted */
	WTBL_PROTECT,		/* not used */
	WTBL_CLEAR,		/* not used */
	WTBL_BF,
	WTBL_SMPS,
	WTBL_RAW_DATA,		/* debug only */
	WTBL_PN,
	WTBL_SPE,
	WTBL_MAX_NUM
};

typedef enum _ENUM_UMAC_PORT_T {
	ENUM_UMAC_HIF_PORT_0         = 0,
	ENUM_UMAC_CPU_PORT_1         = 1,
	ENUM_UMAC_LMAC_PORT_2        = 2,
	ENUM_PLE_CTRL_PSE_PORT_3     = 3,
	ENUM_UMAC_PSE_PLE_PORT_TOTAL_NUM = 4
} ENUM_UMAC_PORT_T, *P_ENUM_UMAC_PORT_T;

/* N9 MCU QUEUE LIST */
typedef enum _ENUM_UMAC_CPU_P_QUEUE_T {
	ENUM_UMAC_CTX_Q_0 = 0,
	ENUM_UMAC_CTX_Q_1 = 1,
	ENUM_UMAC_CTX_Q_2 = 2,
	ENUM_UMAC_CTX_Q_3 = 3,
	ENUM_UMAC_CRX     = 0,
	ENUM_UMAC_CIF_QUEUE_TOTAL_NUM = 4
} ENUM_UMAC_CPU_P_QUEUE_T, *P_ENUM_UMAC_CPU_P_QUEUE_T;


/* LMAC PLE TX QUEUE LIST */
typedef enum _ENUM_UMAC_LMAC_PLE_TX_P_QUEUE_T {
	ENUM_UMAC_LMAC_PLE_TX_Q_00           = 0x00,
	ENUM_UMAC_LMAC_PLE_TX_Q_01           = 0x01,
	ENUM_UMAC_LMAC_PLE_TX_Q_02           = 0x02,
	ENUM_UMAC_LMAC_PLE_TX_Q_03           = 0x03,

	ENUM_UMAC_LMAC_PLE_TX_Q_10           = 0x04,
	ENUM_UMAC_LMAC_PLE_TX_Q_11           = 0x05,
	ENUM_UMAC_LMAC_PLE_TX_Q_12           = 0x06,
	ENUM_UMAC_LMAC_PLE_TX_Q_13           = 0x07,

	ENUM_UMAC_LMAC_PLE_TX_Q_20           = 0x08,
	ENUM_UMAC_LMAC_PLE_TX_Q_21           = 0x09,
	ENUM_UMAC_LMAC_PLE_TX_Q_22           = 0x0a,
	ENUM_UMAC_LMAC_PLE_TX_Q_23           = 0x0b,

	ENUM_UMAC_LMAC_PLE_TX_Q_30           = 0x0c,
	ENUM_UMAC_LMAC_PLE_TX_Q_31           = 0x0d,
	ENUM_UMAC_LMAC_PLE_TX_Q_32           = 0x0e,
	ENUM_UMAC_LMAC_PLE_TX_Q_33           = 0x0f,

	ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0      = 0x10,
	ENUM_UMAC_LMAC_PLE_TX_Q_BMC_0       = 0x11,
	ENUM_UMAC_LMAC_PLE_TX_Q_BNC_0       = 0x12,
	ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0      = 0x13,

	ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1      = 0x14,
	ENUM_UMAC_LMAC_PLE_TX_Q_BMC_1       = 0x15,
	ENUM_UMAC_LMAC_PLE_TX_Q_BNC_1       = 0x16,
	ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1      = 0x17,
	ENUM_UMAC_LMAC_PLE_TX_Q_NAF         = 0x18,
	ENUM_UMAC_LMAC_PLE_TX_Q_NBCN        = 0x19,
	ENUM_UMAC_LMAC_PLE_TX_Q_RELEASE     = 0x1f, /* DE suggests not to use 0x1f, it's only for hw free queue */
	ENUM_UMAC_LMAC_QUEUE_TOTAL_NUM      = 24,

} ENUM_UMAC_LMAC_TX_P_QUEUE_T, *P_ENUM_UMAC_LMAC_TX_P_QUEUE_T;

/* LMAC PLE For PSE Control P3 */
typedef enum _ENUM_UMAC_PLE_CTRL_P3_QUEUE_T {
	ENUM_UMAC_PLE_CTRL_P3_Q_0X1E            = 0x1e,
	ENUM_UMAC_PLE_CTRL_P3_Q_0X1F            = 0x1f,
	ENUM_UMAC_PLE_CTRL_P3_TOTAL_NUM         = 2
} ENUM_UMAC_PLE_CTRL_P3_QUEUE_T, *P_ENUM_UMAC_PLE_CTRL_P3_QUEUE_T;


struct empty_q_info {
	char *name;
	u32 p_id;
	u32 q_id;
};

struct rxd_grp_0 {
	u32 rxd_0;
	u32 rxd_1;
	u32 rxd_2;
	u32 rxd_3;
	u32 rxd_4;
	u32 rxd_5;
};

/* RXD DW0 */
#define RXD_RX_BYTE_COUNT_MASK 0xffff
#define RXD_RX_BYTE_COUNT_SHIFT 0
#define RXD_ETH_TYPE_OFFSET_MASK (0x7f << 16)
#define RXD_ETH_TYPE_OFFSET_SHIFT 16
#define RXD_IP (1 << 23)
#define RXD_UT (1 << 24)
#define RXD_DUP_MODE_MASK (0x3 << 25)
#define RXD_DUP_MODE_SHIFT 25

enum {
	RXD_PT_TXS,
	RXD_PT_VEC,
	RXD_PT_RX,
	RXD_PT_D_RX,
	RXD_PT_TMR,
	RXD_PT_RETRIEVE,
	RXD_PT_MSDU_ID_RPT,
	RXD_PT_SW_CMD,
	RXD_PT_SPL,
	RXD_PT_TXCMD_RPT,
	RXD_PT_RX_RPT,
};
#define RXD_PKT_TYPE_MASK (0x1f << 27)
#define RXD_PKT_TYPE_SHIFT 27

/* RXD DW1 */
#define RXD_WLAN_IDX_MASK (0x3ff)
#define RXD_WLAN_IDX_SHIFT 0
#define RXD_GROUP_VLD_MASK (0x1f << 11)
#define RXD_GROUP_VLD_SHIFT 11
#define RXD_GROUP1_VLD (1 << 11)
#define RXD_GROUP2_VLD (1 << 12)
#define RXD_GROUP3_VLD (1 << 13)
#define RXD_GROUP4_VLD (1 << 14)
#define RXD_GROUP5_VLD (1 << 15)
#define RXD_SEC_MODE_MASK (0x1f << 16)
#define RXD_SEC_MODE_SHIFT 16
#define RXD_KID_MASK (0x3 << 21)
#define RXD_KID_SHIFT 21
#define RXD_CM (1 << 23)
#define RXD_CLM (1 << 24)
#define RXD_ICV_ERR (1 << 25)
#define RXD_TKIPMIC_ERR (1 << 26)
#define RXD_FCS_ERR (1 << 27)
#define RXD_BN (1 << 28)
#define RXD_SPP_EN (1 << 29)
#define RXD_ADD_OM (1 << 30)
#define RXD_SEC_DONE (1 < 31)

/* RXD DW2 */
#define RXD_BSSID_MASK (0x3f)
#define RXD_BSSID_SHIFT 0
#define RXD_BF_CQI (1 << 7)
#define RXD_MAC_HDR_LEN_MASK (0x1f << 8)
#define RXD_MAC_HDR_LEN_SHIFT 8
#define RXD_H (1 << 13)
#define RXD_HO_MASK (0x3 << 14)
#define RXD_HO_SHIFT 14
#define RXD_TID_MASK (0xf << 16)
#define RXD_TID_SHIFT 16
#define RXD_MU_BAR (1 << 21)
#define RXD_SWBIT (1 << 22)
#define RXD_DAF (1 << 23)
#define RXD_EL (1 << 24)
#define RXD_HTF (1 << 25)
#define RXD_INTF (1 << 26)
#define RXD_FRAG (1 << 27)
#define RXD_NULL (1 << 28)
#define RXD_NDATA (1 << 29)
#define RXD_NAMP (1 << 30)
#define RXD_BF_RPT (1 << 31)

/* RXD DW3 */
#define RXD_RXV_SN_MASK 0xff
#define RXD_RXV_SN_SHIFT 0
#define RXD_CF_MASK (0xff << 8)
#define RXD_CF_SHIFT 8
#define RXD_A1_TYPE_MASK (0x3 << 16)
#define RXD_A1_TYPE_SHIFT 16
#define RXD_HTC (1 << 18)
#define RXD_TCL (1 << 19)
#define RXD_BBM (1 << 20)
#define RXD_BU (1 << 21)
#define RXD_AMS (1 << 22)
#define RXD_MESH (1 << 23)
#define RXD_MHCP (1 << 24)
#define RXD_NO_INFO_WB (1 << 25)
#define RXD_DIS_RHTR (1 << 26)
#define RXD_PSS (1 << 27)
#define RXD_MORE (1 << 28)
#define RXD_UWAT (1 << 29)
#define RXD_RX_DROP (1 << 30)
#define RXD_VLAN2ETH (1 << 31)

/* RXD DW4 */
#define RXD_PF_MASK (0x3)
#define RXD_PF_SHIFT 0
#define RXD_DP (1 << 9)
#define RXD_CLS (1 << 10)
#define RXD_OFLD_MASK (0x3 << 11)
#define RXD_OFLD_SHIFT 11
#define RXD_MGC (1 << 13)
#define RXD_WOL_MASK (0x1f << 14)
#define RXD_WOL_SHIFT 14
#define RXD_CLS_BITMAP_MASK (0x3ff << 19)
#define RXD_CLS_BITMAP_SHIFT 19
#define RXD_PF_MODE (1 << 29)
#define RXD_PF_STS_MASK (0x3 << 30)
#define RXD_PF_STS_SHIFT 30

/* RXD DW6 */
#define RXD_FC_MASK (0xffff)
#define RXD_FC_SHIFT 0
#define RXD_TA_1_MASK (0xffff << 16)
#define RXD_TA_1_SHIFT 16

/* RXD DW7 */
#define RXD_TA_2_MASK (0xffffffff)
#define RXD_TA_2_SHIFT 0

/* RXD DW8 */
#define RXD_FN_MASK (0xf)
#define RXD_FN_SHIFT 0
#define RXD_SN_MASK (0xfff << 4)
#define RXD_SN_SHIFT 4
#define RXD_AID_MASK (0xffff)
#define RXD_AID_SHIFT 0
#define RXD_QOS_CTL_MASK (0xffff << 16)
#define RXD_QOS_CTL_SHIFT 16

/* RXD DW9 */
#define RXD_HT_CTL_MASK (0xffffffff)
#define RXD_HT_CTL_SHIFT 0

/* RXD DW10 */
#define RXD_PN_0_31_MASK (0xffffffff)
#define RXD_PN_0_31_SHIFT 0

/* RXD DW11 */
#define RXD_PN_32_47_MASK ((0xffff))
#define RXD_PN_32_47_SHIFT 0


/* TMAC_TXD_1.hdr_format */
#define TMI_HDR_FT_NON_80211	0x0
#define TMI_HDR_FT_CMD		0x1
#define TMI_HDR_FT_NOR_80211	0x2
#define TMI_HDR_FT_ENH_80211	0x3

#define TXD_Q_IDX_MASK (0x7f << 25)
#define TXD_Q_IDX_SHIFT 25

#define TXD_PKT_FT_MASK (0x3 << 23)
#define TXD_PKT_FT_SHIFT 23

#define TXD_TX_BYTE_COUNT_MASK 0xffff
#define TXD_TX_BYTE_COUNT_SHIFT 0
#define TXD_ETH_TYPE_OFFSET_MASK (0x7f << 16)
#define TXD_ETH_TYPE_OFFSET_SHIFT 16

#define TXD_WLAN_IDX_MASK 0x3ff
#define TXD_WLAN_IDX_SHIFT 0

#define TXD_VTA (1 << 10)
#define TXD_HDR_LEN_MASK (0x1f << 11)
#define TXD_HDR_LEN_SHIFT 11

#define TXD_EOSP (1 << 12)
#define TXD_AMS (1 << 13)

#define TXD_MRD (1 << 11)
#define TXD_RMVL (1 << 13)
#define TXD_VLAN (1 << 14)
#define TXD_ETYP (1 << 15)

enum {
	HF_802_3_FRAME,
	HF_CMD_FRAME,
	HF_802_11_FRAME,
	HF_802_11_EN_FRAME,
};
#define TXD_HF_MASK (0x3 << 16)
#define TXD_HF_SHIFT 16

#define TXD_HDR_PAD_MASK (0x3 << 18)
#define TXD_HDR_PAD_SHIFT 18
#define TXD_TID_MASK (0x7 << 20)
#define TXD_TID_SHIFT 20
#define TXD_AMSDU (1 << 23)

#define TXD_OM_MASK (0x3f << 24)
#define TXD_OM_SHIFT 24
#define TXD_TGID (1 << 30)
#define TXD_FT (1 << 31)

/* TXD DW2 */
#define TXD_SUBTYPE_MASK (0xf)
#define TXD_SUBTYPE_SHIFT 0
#define TXD_TYPE_MASK (0x3 << 4)
#define TXD_TYPE_SHIFT 4
#define TXD_NDP (1 << 6)
#define TXD_NDPA (1 << 7)
#define TXD_SD (1 << 8)
#define TXD_RTS (1 << 9)
#define TXD_BM (1 << 10)
#define TXD_B (1 << 11)
#define TXD_DU (1 << 12)
#define TXD_HE (1 << 13)

enum {
	NO_FRAG,
	FIRST_FRAG,
	MIDDLE_FRAG,
	LAST_FRAG,
};
#define TXD_FRAG_MASK (0x3 << 14)
#define TXD_FRAG_SHIFT 14
#define TXD_REMAIN_TIME_MASK (0x7f << 16)	/* in unit of 64TU, MSB bit is reserved for HW ans SW should set to 0 */
#define TXD_REMAIN_TIME_SHIFT 16
#define TXD_PWR_OFFESET_MASK (0x3f << 24)
#define TXD_PWR_OFFESET_SHIFT 24
#define TXD_FRM (1 << 30)
#define TXD_FR (1 << 31)

/* TXD DW3 */
#define TXD_NA (1 << 0)
#define TXD_PF (1 << 1)
#define TXD_EMRD (1 << 2)
#define TXD_EEOSP (1 << 3)
#define TXD_DAS (1 << 4)
#define TXD_TM (1 << 5)
#define TXD_TX_CNT_MASK (0x1f << 6)
#define TXD_TX_CNT_SHIFT 6
#define TXD_REMAIN_TX_CNT_MASK (0x1f << 11)
#define TXD_REMAIN_TX_CNT_SHIFT 11
#define TXD_SN_MASK (0xfff << 16)
#define TXD_SN_SHIFT 16
#define TXD_BA_DIS (1 << 28)
#define TXD_PM (1 << 29)
#define TXD_PN_VLD (1 << 30)
#define TXD_SN_VLD (1 << 31)

/* TXD DW4 */
#define TXD_PN1_MASK (0xffffffff)
#define TXD_PN1_SHIFT 0

/* TXD DW5 */
#define TXD_PID_MASK (0xff)
#define TXD_PID_SHIFT 0
#define TXD_TXSFM (1 << 8)
#define TXD_TXSFM_SHIFT 8
#define TXD_TXS2M (1 << 9)
#define TXD_TXS2M_SHIFT 9
#define TXD_TXS2H (1 << 10)
#define TXD_TXS2H_SHIFT 10

#define TXD_ADD_BA (1 << 14)
#define TXD_MD (1 << 15)
#define TXD_PN2_MASK (0xffff << 16)
#define TXD_PN2_SHIFT 16

/* TXD DW6 */
#define TXD_BW_MASK (0x7)
#define TXD_BW_SHIFT 0
#define TXD_DYN_BW (1 << 3)
#define TXD_ANT_ID_MASK (0xf << 4)
#define TXD_ANT_ID_SHIFT 4
#define TXD_SPE_IDX_SEL (1 << 10)
#define TXD_SPE_IDX_SEL_SHIFT 10
/* MT7915 move from DW5[11][13:12][14] */
#define TXD_LDPC (1 << 11)
#define TXD_HELTF_TYPE_MASK (0x3 << 12)
#define TXD_HELTF_TYPE_SHIFT 12
#define TXD_GI_MASK (0x3 << 14)
#define TXD_GI_SHIFT 14
#define TXD_FR_RATE_MASK (0x3fff << 16)
#define TXD_FR_RATE_SHIFT 16
#define TXD_TX_RATE_BIT_SUEXTTONE 5	/* TX format Doc. is incorrect */
#define TXD_TX_RATE_BIT_DCM 4		/* TX format Doc. is incorrect */
#define TXD_TX_RATE_BIT_STBC 13
#define TXD_TX_RATE_BIT_NSS 10
#define TXD_TX_RATE_MASK_NSS 0x7
#define TXD_TX_RATE_BIT_MODE 6
#define TXD_TX_RATE_MASK_MODE 0xf


#define TXD_TXEBF (1 << 30)
#define TXD_TXIBF (1 << 31)

/* TXD DW7 */
#define TXD_SW_TX_TIME_MASK (0x3ff)
#define TXD_SW_TX_TIME_SHIFT 0
#define TXD_TAT (0x3ff)
#define TXD_TAT_SHIFT 0
#define TXD_HW_AMSDU_CAP (1 << 10)
#define TXD_SPE_IDX_MASK (0x1f << 11)
#define TXD_SPE_IDX_SHIFT 11
#define TXD_PSE_FID_MASK (0xfff << 16)
#define TXD_PSE_FID_SHIFT 16
#define TXD_PP_SUBTYPE_MASK (0xf << 16)
#define TXD_PP_SUBTYPE_SHIFT 16
#define TXD_PP_TYPE_MASK (0x3 << 20)
#define TXD_PP_TYPE_SHIFT 20
#define TXD_CTXD_CNT_MASK (0x7 << 23)
#define TXD_CTXD_CNT_SHIFT 23
#define TXD_CTXD (1 << 26)
#define TXD_IP_CHKSUM (1 << 28)
#define TXD_UT (1 << 29)

#define TXDLEN_PAGE_SIZE 64
enum {
	TXDLEN_1_PAGE,
	TXDLEN_2_PAGE,
	TXDLEN_3_PAGE,
	TXDLEN_4_PAGE,
};
#define TXD_TXD_LEN_MASK (0x3 << 30)
#define TXD_TXD_LEN_SHIFT 30

/* TXD DW8 */
#define TXD_MSDU_ID0_MASK (0x7fff)
#define TXD_MSDU_ID0_SHIFT 0
#define TXD_MSDU_VLD0 (1 << 15)
#define TXD_MSDU_ID1_MASK (0x7fff << 16)
#define TXD_MSDU_ID1_SHIFT 16
#define TXD_MSDU_VLD1 (1 << 31)

/* TXD DW9 */
#define TXD_MSDU_ID2_MASK (0x7fff)
#define TXD_MSDU_ID2_SHIFT 0
#define TXD_MSDU_VLD2 (1 << 15)
#define TXD_MSDU_ID3_MASK (0x7fff << 16)
#define TXD_MSDU_ID3_SHIFT 16
#define TXD_SSN (0xfff << 16)
#define TXD_SSN_SHIFT 16
#define TXD_MSDU_VLD3 (1 << 31)
/* TXD DW10 */
#define TXD_TXP_ADDR0_MASK (0xffffffff)
#define TXD_TXP_ADDR0_SHIFT 0

/* TXD DW11 */
#define TXD_TXP_LEN0_MASK (0x3fff)
#define TXD_TXP_LEN0_SHIFT 0
#define TXD_ML0 (1 << 14)
#define TXD_AL0 (1 << 15)
#define TXD_TXP_LEN1_MASK (0X3fff << 16)
#define TXD_TXP_LEN1_SHIFT 16
#define TXD_ML1 (1 << 30)
#define TXD_AL1 (1 << 31)

/* TXD DW12 */
#define TXD_TXP_ADDR1_MASK (0xffffffff)
#define TXD_TXP_ADDR1_SHIFT 0

/* TXD DW13 */
#define TXD_TXP_ADDR2_MASK (0xffffffff)
#define TXD_TXP_ADDR2_SHIFT 0

/* TXD DW14 */
#define TXD_TXP_LEN2_MASK (0x3fff)
#define TXD_TXP_LEN2_SHIFT 0
#define TXD_ML2 (1 << 14)
#define TXD_AL2 (1 << 15)
#define TXD_TXP_LEN3_MASK (0X3fff << 16)
#define TXD_TXP_LEN3_SHIFT 16
#define TXD_ML3 (1 << 30)
#define TXD_AL3 (1 << 31)

/* TXD DW15 */
#define TXD_TXP_ADDR3_MASK (0xffffffff)
#define TXD_TXP_ADDR3_SHIFT 0

/* TXD DW16 */
#define TXD_TXP_ADDR4_MASK (0xffffffff)
#define TXD_TXP_ADDR4_SHIFT 0

/* TXD DW17 */
#define TXD_TXP_LEN4_MASK (0x3fff)
#define TXD_TXP_LEN4_SHIFT 0
#define TXD_ML4 (1 << 14)
#define TXD_AL4 (1 << 15)
#define TXD_TXP_LEN5_MASK (0X3fff << 16)
#define TXD_TXP_LEN5_SHIFT 16
#define TXD_ML5 (1 << 30)
#define TXD_AL5 (1 << 31)

/* TXD DW18 */
#define TXD_TXP_ADDR5_MASK (0xffffffff)
#define TXD_TXP_ADDR5_SHIFT 0

/* TXD DW19 */
#define TXD_TXP_ADDR6_MASK (0xffffffff)
#define TXD_TXP_ADDR6_SHIFT 0

/* TXD DW20 */
#define TXD_TXP_LEN6_MASK (0x3fff)
#define TXD_TXP_LEN6_SHIFT 0
#define TXD_ML6 (1 << 14)
#define TXD_AL6 (1 << 15)
#define TXD_TXP_LEN7_MASK (0X3fff << 16)
#define TXD_TXP_LEN7_SHIFT 16
#define TXD_ML7 (1 << 30)
#define TXD_AL7 (1 << 31)

/* TXD DW21 */
#define TXD_TXP_ADDR7_MASK (0xffffffff)
#define TXD_TXP_ADDR7_SHIFT 0

/* TXD DW22 */
#define TXD_TXP_ADDR8_MASK (0xffffffff)
#define TXD_TXP_ADDR8_SHIFT 0

/* TXD DW23 */
#define TXD_TXP_LEN8_MASK (0x3fff)
#define TXD_TXP_LEN8_SHIFT 0
#define TXD_ML8 (1 << 14)
#define TXD_AL8 (1 << 15)
#define TXD_TXP_LEN9_MASK (0X3fff << 16)
#define TXD_TXP_LEN9_SHIFT 16
#define TXD_ML9 (1 << 30)
#define TXD_AL9 (1 << 31)

/* TXD DW24 */
#define TXD_TXP_ADDR9_MASK (0xffffffff)
#define TXD_TXP_ADDR9_SHIFT 0

/* TXD DW25 */
#define TXD_TXP_ADDR10_MASK (0xffffffff)
#define TXD_TXP_ADDR10_SHIFT 0

/* TXD DW26 */
#define TXD_TXP_LEN10_MASK (0x3fff)
#define TXD_TXP_LEN10_SHIFT 0
#define TXD_ML10 (1 << 14)
#define TXD_AL10 (1 << 15)
#define TXD_TXP_LEN11_MASK (0X3fff << 16)
#define TXD_TXP_LEN11_SHIFT 16
#define TXD_ML11 (1 << 30)
#define TXD_AL11 (1 << 31)

/* TXD DW27 */
#define TXD_TXP_ADDR11_MASK (0xffffffff)
#define TXD_TXP_ADDR11_SHIFT 0

/* TXD DW28 */
#define TXD_TXP_ADDR12_MASK (0xffffffff)
#define TXD_TXP_ADDR12_SHIFT 0

/* TXD DW29 */
#define TXD_TXP_LEN12_MASK (0x3fff)
#define TXD_TXP_LEN12_SHIFT 0
#define TXD_ML12 (1 << 14)
#define TXD_AL12 (1 << 15)
#define TXD_TXP_LEN13_MASK (0X3fff << 16)
#define TXD_TXP_LEN13_SHIFT 16
#define TXD_ML13 (1 << 30)
#define TXD_AL13 (1 << 31)

/* TXD DW30 */
#define TXD_TXP_ADDR13_MASK (0xffffffff)
#define TXD_TXP_ADDR13_SHIFT 0

/***** WTBL(LMAC) *****/
/* WTBL Group - Peer Information */
/* DW 0 */
#define WTBL_MUAR_IDX_MASK              GENMASK(21, 16)
#define WTBL_MUAR_IDX_OFFSET            16
#define WTBL_RCA1                       BIT(22)
#define WTBL_KID_MASK                   GENMASK(24, 23)
#define WTBL_KID_OFFSET                 23
#define WTBL_RCID                       BIT(25)
#define WTBL_FROM_DS                    BIT(26)
#define WTBL_TO_DS                      BIT(27)
#define WTBL_RV                         BIT(28)
#define WTBL_RCA2                       BIT(29)
#define WTBL_WPI_FLAG                   BIT(30)


/* WTBL Group - TxRx Capability/Information */
/* DW 2 */
#define WTBL_AID12_MASK                 GENMASK(11, 0)
#define WTBL_AID12_OFFSET               0
#define WTBL_GID_SU                     BIT(12)
#define WTBL_SPP_EN                     BIT(13)
#define WTBL_WPI_EVEN                   BIT(14)
#define WTBL_AAD_OM                     BIT(15)
#define WTBL_CIPHER_SUITE_MASK          GENMASK(20, 16)
#define WTBL_CIPHER_SUITE_OFFSET        16
#define WTBL_CIPHER_SUITE_IGTK_MASK     GENMASK(22, 21)
#define WTBL_CIPHER_SUITE_IGTK_OFFSET   21
#define WTBL_SW                         BIT(24)
#define WTBL_UL                         BIT(25)
#define WTBL_TX_POWER_SAVE_STATUS       BIT(26)
#define WTBL_QOS                        BIT(27)
#define WTBL_HT_RAW                         BIT(28)
#define WTBL_VHT_RAW                        BIT(29)
#define WTBL_HE_RAW                         BIT(30)
#define WTBL_MESH_RAW                       BIT(31)

/* DW 3 */
#define WTBL_WMM_Q_MASK                 GENMASK(1, 0)
#define WTBL_WMM_Q_OFFSET               0
#define WTBL_RXD_DUP_MODE_MASK          GENMASK(3, 2)
#define WTBL_RXD_DUP_MODE_OFFSET        2
#define WTBL_VLAN2ETH                   BIT(4)
#define WTBL_BEAM_CHG                   BIT(5)
#define WTBL_DIS_BA256                  BIT(6)
#define WTBL_PFMU_IDX_MASK              GENMASK(15, 8)
#define WTBL_PFMU_IDX_OFFSET            8
#define WTBL_ULPF_IDX_MASK              GENMASK(23, 16)
#define WTBL_ULPF_IDX_OFFSET            16
#define WTBL_RIBF                       BIT(24)
#define WTBL_ULPF                       BIT(25)
#define WTBL_IGN_FBK                    BIT(26)
#define WTBL_TBF                        BIT(29)
#define WTBL_TBF_VHT                    BIT(30)
#define WTBL_TBF_HE                     BIT(31)

/* DW 4 */
#define WTBL_ANT_ID_MASK                GENMASK(23, 0)
#define WTBL_ANT_ID_STS0_MASK           GENMASK(2, 0)
#define WTBL_ANT_ID_STS0_OFFSET         0
#define WTBL_ANT_ID_STS1_MASK           GENMASK(5, 3)
#define WTBL_ANT_ID_STS1_OFFSET         3
#define WTBL_ANT_ID_STS2_MASK           GENMASK(8, 6)
#define WTBL_ANT_ID_STS2_OFFSET         6
#define WTBL_ANT_ID_STS3_MASK           GENMASK(11, 9)
#define WTBL_ANT_ID_STS3_OFFSET         9
#define WTBL_ANT_ID_STS4_MASK           GENMASK(14, 12)
#define WTBL_ANT_ID_STS4_OFFSET         12
#define WTBL_ANT_ID_STS5_MASK           GENMASK(17, 15)
#define WTBL_ANT_ID_STS5_OFFSET         15
#define WTBL_ANT_ID_STS6_MASK           GENMASK(20, 18)
#define WTBL_ANT_ID_STS6_OFFSET         18
#define WTBL_ANT_ID_STS7_MASK           GENMASK(23, 21)
#define WTBL_ANT_ID_STS7_OFFSET         21
#define WTBL_CASCAD                     BIT(24)
#define WTBL_LDPC_HT                    BIT(25)
#define WTBL_LDPC_HT_OFFSET             25
#define WTBL_LDPC_VHT                   BIT(26)
#define WTBL_LDPC_VHT_OFFSET            26
#define WTBL_LDPC_HE                    BIT(27)
#define WTBL_LDPC_HE_OFFSET             27
#define WTBL_DIS_RHTR                   BIT(28)
#define WTBL_ALL_ACK                    BIT(29)
#define WTBL_DROP                       BIT(30)
#define WTBL_ACK_EN                     BIT(31)

/* DW 5 */
#define WTBL_AF_MASK                    GENMASK(2, 0)
#define WTBL_AF_OFFSET                  0
#define WTBL_AF_HE_MASK                 GENMASK(4, 3)
#define WTBL_AF_HE_OFFSET               3
#define WTBL_RTS                        BIT(5)
#define WTBL_SMPS_RAW                       BIT(6)
#define WTBL_DYN_BW                     BIT(7)
#define WTBL_MMSS_MASK                  GENMASK(10, 8)
#define WTBL_MMSS_OFFSET                8
#define WTBL_USR                        BIT(11)
#define WTBL_SR_RATE_MASK               GENMASK(14, 12)
#define WTBL_SR_RATE_OFFSET             12
#define WTBL_SR_ABORT                   BIT(15)
#define WTBL_TX_POWER_OFFSET_MASK       GENMASK(21, 16)
#define WTBL_TX_POWER_OFFSET_OFFSET     16
#define WTBL_MPDU_SIZE_MASK             GENMASK(23, 22)
#define WTBL_MPDU_SIZE_OFFSET           22
#define WTBL_PE_MASK                    GENMASK(25, 24)
#define WTBL_PE_OFFSET                  24
#define WTBL_DOPPL                      BIT(26)
#define WTBL_TXOP_PS_CAP                BIT(27)
#define WTBL_DONOT_UPDATE_I_PSM         BIT(28)
#define WTBL_I_PSM                      BIT(29)
#define WTBL_PSM                        BIT(30)
#define WTBL_SKIP_TX                    BIT(31)

/* DW 6 */
#define WTBL_BA_WIN_SIZE_TID_MASK       GENMASK(3, 0)
#define WTBL_BA_WIN_SIZE_TID_LEN        4
#define WTBL_BA_WIN_SIZE_TID_MASK_ALL   GENMASK(31, 0)

/* DW 7 */
#define WTBL_CBRN_MASK                  GENMASK(2, 0)
#define WTBL_CBRN_OFFSET                0
#define WTBL_DBNSS_EN                   BIT(3)
#define WTBL_BAF_EN                     BIT(4)
#define WTBL_RDGBA                      BIT(5)
#define WTBL_RDG_RAW                        BIT(6)
#define WTBL_SPE_IDX_MASK               GENMASK(11, 7)
#define WTBL_SPE_IDX_OFFSET             7
#define WTBL_G2                         BIT(12)
#define WTBL_G2_OFFSET                  12
#define WTBL_G4                         BIT(13)
#define WTBL_G4_OFFSET                  13
#define WTBL_G8                         BIT(14)
#define WTBL_G8_OFFSET                  14
#define WTBL_G16                        BIT(15)
#define WTBL_G16_OFFSET                 15
#define WTBL_G2_LTF_MASK                GENMASK(17, 16)
#define WTBL_G2_LTF_OFFSET              16
#define WTBL_G4_LTF_MASK                GENMASK(19, 18)
#define WTBL_G4_LTF_OFFSET              18
#define WTBL_G8_LTF_MASK                GENMASK(21, 20)
#define WTBL_G8_LTF_OFFSET              20
#define WTBL_G16_LTF_MASK               GENMASK(23, 22)
#define WTBL_G16_LTF_OFFSET             22
#define WTBL_G2_HE_MASK                 GENMASK(25, 24)
#define WTBL_G2_HE_OFFSET               24
#define WTBL_G4_HE_MASK                 GENMASK(27, 26)
#define WTBL_G4_HE_OFFSET               26
#define WTBL_G8_HE_MASK                 GENMASK(29, 28)
#define WTBL_G8_HE_OFFSET               28
#define WTBL_G16_HE_MASK                GENMASK(31, 30)
#define WTBL_G16_HE_OFFSET              30

/* WTBL Group - Auto Rate */
/* DW 8 */
#define WTBL_FAIL_CNT_AC0_MASK          GENMASK(4, 0)
#define WTBL_FAIL_CNT_AC0_OFFSET        0
#define WTBL_FAIL_CNT_AC1_MASK          GENMASK(9, 5)
#define WTBL_FAIL_CNT_AC1_OFFSET        5
#define WTBL_FAIL_CNT_AC2_MASK          GENMASK(14, 10)
#define WTBL_FAIL_CNT_AC2_OFFSET        10
#define WTBL_FAIL_CNT_AC3_MASK          GENMASK(19, 15)
#define WTBL_FAIL_CNT_AC3_OFFSET        15
#define WTBL_PARTIAL_AID_MASK           GENMASK(28, 20)
#define WTBL_PARTIAL_AID_OFFSET         20
#define WTBL_CHK_PER                    BIT(31)

/* DW 9 */
#define WTBL_RX_AVG_MPDU_SIZE_MASK      GENMASK(13, 0)
#define WTBL_RX_AVG_MPDU_SIZE_OFFSET    0
#define WTBL_PRITX_SW_MODE              BIT(16)
#define WTBL_PRITX_PLR                  BIT(17)
#define WTBL_PRITX_DCM                  BIT(18)
#define WTBL_PRITX_ER160                BIT(19)
#define WTBL_PRITX_ERSU                 BIT(20)
#define WTBL_FCAP_20_TO_160_MHZ         GENMASK(22, 21)
#define WTBL_FCAP_20_TO_160_MHZ_OFFSET  21
#define WTBL_MPDU_FAIL_CNT_MASK         GENMASK(25, 23)
#define WTBL_MPDU_FAIL_CNT_OFFSET       23
#define WTBL_MPDU_OK_CNT_MASK           GENMASK(28, 26)
#define WTBL_MPDU_OK_CNT_OFFSET         26
#define WTBL_RATE_IDX_MASK              GENMASK(31, 29)
#define WTBL_RATE_IDX_OFFSET            29

/* DW 10*/
#define WTBL_RATE1_MASK                 GENMASK(13, 0)
#define WTBL_RATE1_OFFSET               0
#define WTBL_RATE2_MASK                 GENMASK(29, 16)
#define WTBL_RATE2_OFFSET               16

/* DW 11 */
#define WTBL_RATE3_MASK                 GENMASK(13, 0)
#define WTBL_RATE3_OFFSET               0
#define WTBL_RATE4_MASK                 GENMASK(29, 16)
#define WTBL_RATE4_OFFSET               16

/* DW 12 */
#define WTBL_RATE5_MASK                 GENMASK(13, 0)
#define WTBL_RATE5_OFFSET               0
#define WTBL_RATE6_MASK                 GENMASK(29, 16)
#define WTBL_RATE6_OFFSET               16

/* DW 13 */
#define WTBL_RATE7_MASK                 GENMASK(13, 0)
#define WTBL_RATE7_OFFSET               0
#define WTBL_RATE8_MASK                 GENMASK(29, 16)
#define WTBL_RATE8_OFFSET               16


/* WTBL Group - Rate Counter */
/* DW 14 */
#define WTBL_RATE1_TX_CNT_MASK          GENMASK(15, 0)
#define WTBL_RATE1_TX_CNT_OFFSET        0
#define WTBL_RATE1_FAIL_CNT_MASK        GENMASK(31, 16)
#define WTBL_RATE1_FAIL_CNT_OFFSET      16

/* DW 15 */
#define WTBL_RATE2_OK_CNT_MASK          GENMASK(15, 0)
#define WTBL_RATE2_OK_CNT_OFFSET        0
#define WTBL_RATE3_OK_CNT_MASK          GENMASK(31, 16)
#define WTBL_RATE3_OK_CNT_OFFSET        16

/* DW 16 */
#define WTBL_CURRENT_BW_TX_CNT_MASK     GENMASK(15, 0)
#define WTBL_CURRENT_BW_TX_CNT_OFFSET   0
#define WTBL_CURRENT_BW_FAIL_CNT_MASK   GENMASK(31, 16)
#define WTBL_CURRENT_BW_FAIL_CNT_OFFSET 16

/* DW 17 */
#define WTBL_OTHER_BW_TX_CNT_MASK       GENMASK(15, 0)
#define WTBL_OTHER_BW_TX_CNT_OFFSET     0
#define WTBL_OTHER_BW_FAIL_CNT_MASK     GENMASK(31, 16)
#define WTBL_OTHER_BW_FAIL_CNT_OFFSET   16

/* WTBL Group - PPDU Counter */
/* DW 18 */
#define WTBL_RTS_OK_CNT_MASK            GENMASK(15, 0)
#define WTBL_RTS_OK_CNT_OFFSET          0
#define WTBL_RTS_FAIL_CNT_MASK          GENMASK(31, 16)
#define WTBL_RTS_FAIL_CNT_OFFSET        16

/* DW 19 */
#define WTBL_DATA_RETRY_CNT_MASK        GENMASK(15, 0)
#define WTBL_DATA_RETRY_CNT_OFFSET      0
#define WTBL_MGNT_RETRY_CNT_MASK        GENMASK(31, 16)
#define WTBL_MGNT_RETRY_CNT_OFFSET      16


/* WTBL Group - Rx Statistics Counter */
/* DW 28 */
#define WTBL_OM_INFO_MASK               GENMASK(11, 0)
#define WTBL_OM_INFO_OFFSET             0
#define WTBL_OM_RXD_DUP_MODE            BIT(12)

/* DW 29 */
#define WTBL_USER_RSSI_MASK             GENMASK(8, 0)
#define WTBL_USER_RSSI_OFFSET           0
#define WTBL_USER_SNR_MASK              GENMASK(14, 9)
#define WTBL_USER_SNR_OFFSET            9
#define WTBL_RAPID_REACTION_RATE_MASK   GENMASK(26, 16)
#define WTBL_RAPID_REACTION_RATE_OFFSET 16
#define WTBL_HT_AMSDU                   BIT(30)
#define WTBL_AMSDU_CROSS_LG             BIT(31)

/* DW 30 */
#define WTBL_RESP_RCPI0_MASK            GENMASK(7, 0)
#define WTBL_RESP_RCPI0_OFFSET          0
#define WTBL_RESP_RCPI1_MASK            GENMASK(15, 8)
#define WTBL_RESP_RCPI1_OFFSET          8
#define WTBL_RESP_RCPI2_MASK            GENMASK(23, 16)
#define WTBL_RESP_RCPI2_OFFSET          16
#define WTBL_RESP_RCPI3_MASK            GENMASK(31, 24)
#define WTBL_RESP_RCPI3_OFFSET          24

/* DW 31 */
#define WTBL_RESP_RCPI4_MASK            GENMASK(7, 0)
#define WTBL_RESP_RCPI4_OFFSET          0
#define WTBL_RESP_RCPI5_MASK            GENMASK(15, 8)
#define WTBL_RESP_RCPI5_OFFSET          8
#define WTBL_RESP_RCPI6_MASK            GENMASK(23, 16)
#define WTBL_RESP_RCPI6_OFFSET          16
#define WTBL_RESP_RCPI7_MASK            GENMASK(31, 24)
#define WTBL_RESP_RCPI7_OFFSET          24

/* DW 32 */
#define WTBL_SNR_RX0_MASK               GENMASK(5, 0)
#define WTBL_SNR_RX0_OFFSET             0
#define WTBL_SNR_RX1_MASK               GENMASK(11, 6)
#define WTBL_SNR_RX1_OFFSET             6
#define WTBL_SNR_RX2_MASK               GENMASK(17, 12)
#define WTBL_SNR_RX2_OFFSET             12
#define WTBL_SNR_RX3_MASK               GENMASK(23, 18)
#define WTBL_SNR_RX3_OFFSET             18

/* DW 33 */
#define WTBL_SNR_RX4_MASK               GENMASK(5, 0)
#define WTBL_SNR_RX4_OFFSET             0
#define WTBL_SNR_RX5_MASK               GENMASK(11, 6)
#define WTBL_SNR_RX5_OFFSET             6
#define WTBL_SNR_RX6_MASK               GENMASK(17, 12)
#define WTBL_SNR_RX6_OFFSET             12
#define WTBL_SNR_RX7_MASK               GENMASK(23, 18)
#define WTBL_SNR_RX7_OFFSET             18


/***** WTBL(UMAC) *****/
/* WTBL Group - Packet Number */
/* DW 0 */
#define WTBL_PN0_MASK                   GENMASK(7, 0)
#define WTBL_PN0_OFFSET                 0
#define WTBL_PN1_MASK                   GENMASK(15, 8)
#define WTBL_PN1_OFFSET                 8
#define WTBL_PN2_MASK                   GENMASK(23, 16)
#define WTBL_PN2_OFFSET                 16
#define WTBL_PN3_MASK                   GENMASK(31, 24)
#define WTBL_PN3_OFFSET                 24

/* DW 1 */
#define WTBL_PN4_MASK                   GENMASK(7, 0)
#define WTBL_PN4_OFFSET                 0
#define WTBL_PN5_MASK                   GENMASK(15, 8)
#define WTBL_PN5_OFFSET                 8

/* WTBL Group - Serial Number */
/* DW 1 */
#define WTBL_COM_SN_MASK                GENMASK(27, 16)
#define WTBL_COM_SN_OFFSET              16

/* DW 2 */
#define WTBL_TID0_AC0_SN_MASK           GENMASK(11, 0)
#define WTBL_TID0_AC0_SN_OFFSET         0
#define WTBL_TID1_AC1_SN_MASK           GENMASK(23, 12)
#define WTBL_TID1_AC1_SN_OFFSET         12
#define WTBL_TID2_AC2_SN_0_7_MASK       GENMASK(31, 24)
#define WTBL_TID2_AC2_SN_0_7_OFFSET     24

/* DW 3 */
#define WTBL_TID2_AC2_SN_8_11_MASK      GENMASK(3, 0)
#define WTBL_TID2_AC2_SN_8_11_OFFSET    0
#define WTBL_TID3_AC3_SN_MASK           GENMASK(15, 4)
#define WTBL_TID3_AC3_SN_OFFSET         4
#define WTBL_TID4_SN_MASK               GENMASK(27, 16)
#define WTBL_TID4_SN_OFFSET             16
#define WTBL_TID5_SN_0_3_MASK           GENMASK(31, 28)
#define WTBL_TID5_SN_0_3_OFFSET         28

/* DW 4 */
#define WTBL_TID5_SN_4_11_MASK          GENMASK(7, 0)
#define WTBL_TID5_SN_4_11_OFFSET        0
#define WTBL_TID6_SN_MASK               GENMASK(19, 8)
#define WTBL_TID6_SN_OFFSET             8
#define WTBL_TID7_SN_MASK               GENMASK(31, 20)
#define WTBL_TID7_SN_OFFSET             20

/* UWTBL DW 5 */
#define WTBL_KEY_LINK_DW_KEY_LOC0_MASK        GENMASK(10, 0)
#define WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET      0
#define WTBL_KEY_LINK_DW_KEY_LOC1_MASK        GENMASK(26, 16)
#define WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET      16
#define WTBL_QOS_MASK                         BIT(27)
#define WTBL_QOS_OFFSET                       27
#define WTBL_HT_VHT_HE_MASK                   BIT(28)
#define WTBL_HT_VHT_HE_OFFSET                 28

/* UWTBL DW 6 */
#define WTBL_AMSDU_LEN_MASK                    GENMASK(5, 0)
#define WTBL_AMSDU_LEN_OFFSET                  0
#define WTBL_AMSDU_NUM_MASK                    GENMASK(8, 6)
#define WTBL_AMSDU_NUM_OFFSET                  6
#define WTBL_AMSDU_EN_MASK                     BIT(9)
#define WTBL_AMSDU_EN_OFFSET                   9


/***** WTBL(LMAC) DW Offset *****/
/* LMAC WTBL Group - Peer Unique Information */
#define WTBL_GROUP_PEER_INFO_DW_0               0
#define WTBL_GROUP_PEER_INFO_DW_1               1

/* WTBL Group - TxRx Capability/Information */
#define WTBL_GROUP_TRX_CAP_DW_2                 2
#define WTBL_GROUP_TRX_CAP_DW_3                 3
#define WTBL_GROUP_TRX_CAP_DW_4                 4
#define WTBL_GROUP_TRX_CAP_DW_5                 5
#define WTBL_GROUP_TRX_CAP_DW_6                 6
#define WTBL_GROUP_TRX_CAP_DW_7                 7
#define WTBL_GROUP_TRX_CAP_DW_8                 8
#define WTBL_GROUP_TRX_CAP_DW_9                 9

/* WTBL Group - Auto Rate Table*/
#define WTBL_GROUP_AUTO_RATE_1_2                10
#define WTBL_GROUP_AUTO_RATE_3_4                11
#define WTBL_GROUP_AUTO_RATE_5_6                12
#define WTBL_GROUP_AUTO_RATE_7_8                13

/* WTBL Group - Tx Counter */
#define WTBL_GROUP_TX_CNT_LINE_1                14
#define WTBL_GROUP_TX_CNT_LINE_2                15
#define WTBL_GROUP_TX_CNT_LINE_3                16
#define WTBL_GROUP_TX_CNT_LINE_4                17
#define WTBL_GROUP_TX_CNT_LINE_5                18
#define WTBL_GROUP_TX_CNT_LINE_6                19

/* WTBL Group - Admission Control Counter */
#define WTBL_GROUP_ADM_CNT_LINE_1               20
#define WTBL_GROUP_ADM_CNT_LINE_2               21
#define WTBL_GROUP_ADM_CNT_LINE_3               22
#define WTBL_GROUP_ADM_CNT_LINE_4               23
#define WTBL_GROUP_ADM_CNT_LINE_5               24
#define WTBL_GROUP_ADM_CNT_LINE_6               25
#define WTBL_GROUP_ADM_CNT_LINE_7               26
#define WTBL_GROUP_ADM_CNT_LINE_8               27

/* WTBL Group - Rx Statistics Counter */
#define WTBL_GROUP_RX_STAT_CNT_LINE_1           28
#define WTBL_GROUP_RX_STAT_CNT_LINE_2           29
#define WTBL_GROUP_RX_STAT_CNT_LINE_3           30
#define WTBL_GROUP_RX_STAT_CNT_LINE_4           31
#define WTBL_GROUP_RX_STAT_CNT_LINE_5           32
#define WTBL_GROUP_RX_STAT_CNT_LINE_6           33


/***** WTBL(UMAC) DW Offset *****/
/* UWTBL Group - Serial Number */
#define UWTBL_PN_DW_0                           0
#define UWTBL_PN_SN_DW_1                        1
#define UWTBL_SN_DW_2                           2
#define UWTBL_SN_DW_3                           3
#define UWTBL_SN_DW_4                           4

/* UWTBL Group - Key Link */
#define UWTBL_KEY_LINK_DW                       5

/* UWTBL Group - HW AMSDU */
#define UWTBL_HW_AMSDU_DW                       6


#define INVALID_KEY_ENTRY                      WTBL_KEY_LINK_DW_KEY_LOC0_MASK
#define ONE_KEY_ENTRY_LEN_IN_DW                8

/***** Key Table operation command value(WF_UWTBL_TOP_KTCR_OPERATION_ADDR) *****/
#define UWTBL_TOP_KTCR_OPERATION_DELETE     0
#define UWTBL_TOP_KTCR_OPERATION_ALLOCATE   1
#define UWTBL_TOP_KTCR_OPERATION_SEARCH     2

#define WTBL_SR_RATE_E1_MASK        GENMASK(13, 12)
#define WTBL_SR_RATE_E1_OFFSET      12
#define WTBL_BEAM_CHG_E1            BIT(14)


#define WIFI_LWTBL_BASE                 WF_WTBLON_TOP_BASE
#define WIFI_UWTBL_BASE                 WF_UWTBL_TOP_BASE

#define NO_SHIFT_DEFINE 0xFFFFFFFF

/* This address is not generated by CODA and might be different by project */
#define WIFI_WTBL_BASE                  0x38000


#define LWTBL_CONFIG(_wlanIdx)
#define UWTBL_CONFIG(_wlanIdx)
#define KEYTBL_CONFIG(_key_loc)

#define LWTBL_IDX2BASE(_wlanIdx, _DW) \
	(WIFI_WTBL_BASE | ((_wlanIdx & 0x7F) << 8) | (_DW & 0x3F) << 2)


#define UWTBL_IDX2BASE(_wlanIdx, _DW) \
	(WIFI_UWTBL_BASE | 0x2000 | ((_wlanIdx & 0x7F) << 6) | (_DW & 0xF) << 2)

#define KEYTBL_IDX2BASE(_key_loc, _DW) \
	(WIFI_UWTBL_BASE | 0x2000 | ((_key_loc & 0x7F) << 6) | (_DW & 0xF) << 2)

typedef enum _ENUM_WTBL_TYPE_T
{
    WTBL_TYPE_LMAC = 0,     /** WTBL in LMAC */
    WTBL_TYPE_UMAC = 1,     /** WTBL in UMAC */
    WTBL_TYPE_KEY = 2,      /** Key Table */
    MAX_NUM_WTBL_TYPE
} ENUM_WTBL_TYPE_T;


#endif
