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


#ifndef __BMAC_H__
#define __BMAC_H__

#include "wf_tx_descriptor_besra.h"
#include "wf_rx_descriptor_besra.h"
#include "tx_free_done_event_besra.h"
#include "wf_ds_agg_txs.h"
#include "wf_ds_txs_mpdu.h"

#define MAX_ANTENNA_NUM 4
#define RCPI_TO_RSSI(RCPI) ((RCPI == 255) ? (-127) : (RCPI - 220)/2)

#define RMAC_INFO_BASE_SIZE 8
#define RMAC_INFO_GRP_1_SIZE 4
#define RMAC_INFO_GRP_2_SIZE 4
#define RMAC_INFO_GRP_3_SIZE 4
#define RMAC_INFO_GRP_4_SIZE 4
#define RMAC_INFO_GRP_5_SIZE 24

#define BMAC_RXD_HO_MASK (0x7 << 13)
#define BMAC_RXD_HO_SHIFT 13
#define MODE_HE_SU	8

#define BMAC_TXS_TR_DELAY_UNIT	32

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

/* MAC TXD definition */
/* DW2 (MAC TXD V5) */
#define WF_TX_DESCRIPTOR_V5_BF_TYPE_DW 2
#define WF_TX_DESCRIPTOR_V5_BF_TYPE_ADDR 8
#define WF_TX_DESCRIPTOR_V5_BF_TYPE_MASK 0x000001c0
#define WF_TX_DESCRIPTOR_V5_BF_TYPE_SHIFT 6
#define WF_TX_DESCRIPTOR_V5_OM_MAP_DW 2
#define WF_TX_DESCRIPTOR_V5_OM_MAP_ADDR 8
#define WF_TX_DESCRIPTOR_V5_OM_MAP_MASK 0x00000200
#define WF_TX_DESCRIPTOR_V5_OM_MAP_SHIFT 9

/* DW6 (MAC TXD V5) */
#define WF_TX_DESCRIPTOR_V5_MSDU_COUNT_DW 6
#define WF_TX_DESCRIPTOR_V5_MSDU_COUNT_ADDR 24
#define WF_TX_DESCRIPTOR_V5_MSDU_COUNT_MASK 0x0000FC00
#define WF_TX_DESCRIPTOR_V5_MSDU_COUNT_SHIFT 10

/* DW6 (MAC TXD V6) */
#define WF_TX_DESCRIPTOR_V6_RTS_DIS_DW                                         6
#define WF_TX_DESCRIPTOR_V6_RTS_DIS_ADDR                                       24
#define WF_TX_DESCRIPTOR_V6_RTS_DIS_MASK                                       0x00000010 //  4- 4
#define WF_TX_DESCRIPTOR_V6_RTS_DIS_SHIFT                                      4
#define WF_TX_DESCRIPTOR_V6_FIX_PRI_DW                                         6
#define WF_TX_DESCRIPTOR_V6_FIX_PRI_ADDR                                       24
#define WF_TX_DESCRIPTOR_V6_FIX_PRI_MASK                                       0x00000020 //  5- 5
#define WF_TX_DESCRIPTOR_V6_FIX_PRI_SHIFT                                      5
#define WF_TX_DESCRIPTOR_V6_OTF_CAL_DW                                         6
#define WF_TX_DESCRIPTOR_V6_OTF_CAL_ADDR                                       24
#define WF_TX_DESCRIPTOR_V6_OTF_CAL_MASK                                       0x00000040 //  6- 6
#define WF_TX_DESCRIPTOR_V6_OTF_CAL_SHIFT                                      6
#define WF_TX_DESCRIPTOR_V6_NO_DROP_DW                                         6
#define WF_TX_DESCRIPTOR_V6_NO_DROP_ADDR                                       24
#define WF_TX_DESCRIPTOR_V6_NO_DROP_MASK                                       0x00000080 //  7- 7
#define WF_TX_DESCRIPTOR_V6_NO_DROP_SHIFT                                      7
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_IDX_DW                            6
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_IDX_ADDR                          24
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_IDX_MASK                          0x00001f00u // 12- 8
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_IDX_SHIFT                         8
#define WF_TX_DESCRIPTOR_V6_TID_ADDBA_DW                                       6
#define WF_TX_DESCRIPTOR_V6_TID_ADDBA_ADDR                                     24
#define WF_TX_DESCRIPTOR_V6_TID_ADDBA_MASK                                     0x00000700u // 10- 8
#define WF_TX_DESCRIPTOR_V6_TID_ADDBA_SHIFT                                    8
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_EN_DW                             6
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_EN_ADDR                           24
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_EN_MASK                           0x0000e000u // 15-13
#define WF_TX_DESCRIPTOR_V6_TIMESTAMP_OFFSET_EN_SHIFT                          13

/* DW7 (MAC TXD V5) */
#define WF_TX_DESCRIPTOR_V5_RTS_DW 7
#define WF_TX_DESCRIPTOR_V5_RTS_ADDR 28
#define WF_TX_DESCRIPTOR_V5_RTS_MASK 0x00002000
#define WF_TX_DESCRIPTOR_V5_RTS_SHIFT 13

/* DW8 - DW9 */
#define MAC_TXP_TOKEN_ID0_SHIFT 0
#define MAC_TXP_TOKEN_ID0_MASK GENMASK(14, 0)
#define MAC_TXP_TOKEN_ID0_VALID_SHIFT 15
#define MAC_TXP_TOKEN_ID0_VALID_MASK BIT(15)
#define MAC_TXP_TOKEN_ID1_SHIFT 16
#define MAC_TXP_TOKEN_ID1_MASK GENMASK(30, 16)
#define MAC_TXP_TOKEN_ID1_VALID_SHIFT 31
#define MAC_TXP_TOKEN_ID1_VALID_MASK BIT(31)
#define MAC_TXP_TID_ADDBA_SHIFT 12
#define MAC_TXP_TID_ADDBA_MASK GENMASK(14, 12)

/* DW10 - DW15 */
#define MAC_TXP_BUF_PTR0_L_SHIFT 0
#define MAC_TXP_BUF_PTR0_L_MASK GENMASK(31, 0)
#define MAC_TXP_BUF_LEN0_SHIFT 0
#define MAC_TXP_BUF_LEN0_MASK GENMASK(11, 0)
#define MAC_TXP_BUF_PTR0_H_SHIFT 12
#define MAC_TXP_BUF_PTR0_H_MASK GENMASK(13, 12)
#define MAC_TXP_BUF_SRC0_SHIFT 14
#define MAC_TXP_BUF_SRC0_MASK BIT(14)
#define MAC_TXP_BUF_ML0_SHIFT 15
#define MAC_TXP_BUF_ML0_MASK BIT(15)
#define MAC_TXP_BUF_LEN1_SHIFT 16
#define MAC_TXP_BUF_LEN1_MASK GENMASK(27, 16)
#define MAC_TXP_BUF_PTR1_H_SHIFT 28
#define MAC_TXP_BUF_PTR1_H_MASK GENMASK(29, 28)
#define MAC_TXP_BUF_SRC1_SHIFT 30
#define MAC_TXP_BUF_SRC1_MASK BIT(30)
#define MAC_TXP_BUF_ML1_SHIFT 31
#define MAC_TXP_BUF_ML1_MASK BIT(31)
#define MAC_TXP_BUF_PTR1_L_SHIFT 0
#define MAC_TXP_BUF_PTR1_L_MASK GENMASK(31, 0)

/* PAO definition */
/* DW0 */
#define HIF_TXD_VERSION_SHIFT 19
#define HIF_TXD_VERSION_MASK GENMASK(22, 19)

/* DW8 */
#define HIF_TXP_V2_0_PRIORITY_SHIFT 0
#define HIF_TXP_V2_0_PRIORITY_MASK BIT(0)
#define HIF_TXP_STRICT_PRIOR_SHIFT 0
#define HIF_TXP_STRICT_PRIOR_MASK BIT(0)
#define HIF_TXP_FIXED_RATE_SHIFT 1
#define HIF_TXP_FIXED_RATE_MASK BIT(1)
#define HIF_TXP_TCP_SHIFT 2
#define HIF_TXP_TCP_MASK BIT(2)
#define HIF_TXP_NON_CIPHER_SHIFT 3
#define HIF_TXP_NON_CIPHER_MASK BIT(3)
#define HIF_TXP_VLAN_SHIFT 4
#define HIF_TXP_VLAN_MASK BIT(4)
#define HIF_TXP_BC_MC_FLAG_SHIFT 5
#define HIF_TXP_BC_MC_FLAG_MASK GENMASK(6, 5)
#define HIF_TXP_FR_HOST_SHIFT 7
#define HIF_TXP_FR_HOST_MASK BIT(7)
#define HIF_TXP_ETYPE_SHIFT 8
#define HIF_TXP_ETYPE_MASK BIT(8)
#define HIF_TXP_AMSDU_SHIFT 9
#define HIF_TXP_AMSDU_MASK BIT(9)
#define HIF_TXP_MC_CLONE_SHIFT 10
#define HIF_TXP_MC_CLONE_MASK BIT(10)
#define HIF_TXP_OVERRIDE_SHIFT 11
#define HIF_TXP_OVERRIDE_MASK BIT(11)
#define HIF_TXP_PRIORITY_SHIFT 12
#define HIF_TXP_PRIORITY_MASK BIT(12)
#define HIF_TXP_TOKEN_ID_SHIFT 16
#define HIF_TXP_TOKEN_ID_MASK GENMASK(31, 16)

/* DW9 */
#define HIF_TXP_BSS_IDX_SHIFT 0
#define HIF_TXP_BSS_IDX_MASK GENMASK(7, 0)
#define HIF_TXP_V1_WLAN_IDX_SHIFT 8
#define HIF_TXP_V1_WLAN_IDX_MASK GENMASK(23, 8)
#define HIF_TXP_V1_BUF_NUM_SHIFT 24
#define HIF_TXP_V1_BUF_NUM_MASK GENMASK(31, 24)
#define HIF_TXP_USER_PRIORITY_SHIFT 8
#define HIF_TXP_USER_PRIORITY_MASK GENMASK(15, 8)
#define HIF_TXP_BUF_NUM_SHIFT 16
#define HIF_TXP_BUF_NUM_MASK GENMASK(20, 16)
#define HIF_TXP_MSDU_CNT_SHIFT 21
#define HIF_TXP_MSDU_CNT_MASK GENMASK(25, 21)
#define HIF_TXP_SRC_SHIFT 26
#define HIF_TXP_SRC_MASK GENMASK(27, 26)

/* DW10 */
#define HIF_TXP_ETH_TYPE_SHIFT 0
#define HIF_TXP_ETH_TYPE_MASK GENMASK(15, 0)
#define HIF_TXP_WLAN_IDX_SHIFT 16
#define HIF_TXP_WLAN_IDX_MASK GENMASK(27, 16)

/* DW11 */
#define HIF_TXP_PPE_INFO_SHIFT 0
#define HIF_TXP_PPE_INFO_MASK GENMASK(31, 0)

/* DW10 - DW18 (HIF TXD V1) */
/* DW12 - DW31 (HIF TXD V2) */
/* DW13 - DW30 (HIF TXD V3) */
#define HIF_TXP_BUF_PTR0_L_SHIFT 0
#define HIF_TXP_BUF_PTR0_L_MASK GENMASK(31, 0)
#define HIF_TXP_BUF_LEN0_SHIFT 0
#define HIF_TXP_BUF_LEN0_MASK GENMASK(11, 0)
#define HIF_TXP_BUF_PTR0_H_SHIFT 12
#define HIF_TXP_BUF_PTR0_H_MASK GENMASK(15, 12)
#define HIF_TXP_BUF_LEN1_SHIFT 16
#define HIF_TXP_BUF_LEN1_MASK GENMASK(27, 16)
#define HIF_TXP_BUF_PTR1_H_SHIFT 28
#define HIF_TXP_BUF_PTR1_H_MASK GENMASK(31, 28)
#define HIF_TXP_BUF_PTR1_L_SHIFT 0
#define HIF_TXP_BUF_PTR1_L_MASK GENMASK(31, 0)

/* DW31 (HIF TXD V2) */
/* DW12 (HIF TXD V3) */
#define HIF_TXP_ML_SHIFT 16
#define HIF_TXP_ML_MASK GENMASK(28, 16)
#define HIF_TXP_V3_ML_SHIFT 0
#define HIF_TXP_V3_ML_MASK GENMASK(11, 0)

/* first 6G channel number from RXD */
#define CHANNEL_6G_BASE			181

/* SDO EVENT */
/* Common */
#define SDO_EVENT_COMMON_DW_NUM 2
#define WF_SDO_EVENT_RX_BYTE_COUNT_MASK 0x0000ffff // 15- 0
#define WF_SDO_EVENT_RX_BYTE_COUNT_SHIFT 0
#define WF_SDO_EVENT_EVENT_COUNT_MASK 0x7f00000 // 26-20
#define WF_SDO_EVENT_EVENT_COUNT_SHIFT 20

/* Event-Specific */
enum {
	SDO_BA_TRIG_EVENT_ID = 0x0,
	SDO_CACHE_OVERRIDE_EVENT_ID = 0x1,
};

#define WF_SDO_EVENT_DW_LEN_MASK 0xf8000000 // 31-27
#define WF_SDO_EVENT_DW_LEN_SHIFT 27
#define WF_SDO_EVENT_EVENT_ID_MASK 0x7e00000 // 26-21
#define WF_SDO_EVENT_EVENT_ID_SHIFT 21
#define WF_SDO_EVENT_TID_MASK 0x1c000 // 16-14
#define WF_SDO_EVENT_TID_SHIFT 14
#define WF_SDO_EVENT_MLD_ID_MASK 0x3fff // 13- 0
#define WF_SDO_EVENT_MLD_ID_SHIFT 0

struct bmac_tx_free_v0 {
	__le32 dw0;
	__le32 dw1;
	__le32 token[];
} __packed __aligned(4);

struct bmac_tx_free_v4 {
	__le32 dw0;
	__le32 dw1;
	__le32 info;
	__le32 token[];
} __packed __aligned(4);

struct bmac_tx_free_v7 {
	__le32 dw0;
	__le32 dw1;
	__le32 dw2;
	__le32 dw3;
	__le32 token[];
} __packed __aligned(4);

struct bmac_txs {
	__le32 dw0;
	__le32 dw1;
	__le32 dw2;
	__le32 dw3;
	__le32 info[];
} __packed __aligned(4);

struct bmac_rxd_g_0 {
	__le32 rxd_0;
	__le32 rxd_1;
	__le32 rxd_2;
	__le32 rxd_3;
	__le32 rxd_4;
	__le32 rxd_5;
	__le32 rxd_6;
	__le32 rxd_7;
} __packed __aligned(4);

struct bmac_rxd_g_1 {
	__le32 rxd_12;
	__le32 rxd_13;
	__le32 rxd_14;
	__le32 rxd_15;
} __packed __aligned(4);

struct bmac_rxd_g_2 {
	__le32 rxd_16;
	__le32 rxd_17;
	__le32 rxd_18;
	__le32 rxd_19;
} __packed __aligned(4);

struct bmac_rxd_g_3 {
	__le32 rxd_20;
	__le32 rxd_21;
	__le32 rxd_22;
	__le32 rxd_23;
} __packed __aligned(4);

struct bmac_rxd_g_4 {
	__le32 rxd_8;
	__le32 rxd_9;
	__le32 rxd_10;
	__le32 rxd_11;
} __packed __aligned(4);

struct bmac_rxd_g_5 {
	__le32 rxd_24;
	__le32 rxd_25;
	__le32 rxd_26;
	__le32 rxd_27;
	__le32 rxd_28;
	__le32 rxd_29;
	__le32 rxd_30;
	__le32 rxd_31;
	__le32 rxd_32;
	__le32 rxd_33;
	__le32 rxd_34;
	__le32 rxd_35;
	__le32 rxd_36;
	__le32 rxd_37;
	__le32 rxd_38;
	__le32 rxd_39;
	__le32 rxd_40;
	__le32 rxd_41;
	__le32 rxd_42;
	__le32 rxd_43;
	__le32 rxd_44;
	__le32 rxd_45;
	__le32 rxd_46;
	__le32 rxd_47;
} __packed __aligned(4);

int bmac_write_mac_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
int bmac_write_hif_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
int bmac_write_hif_txd_v2(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
int bmac_write_hif_txd_v3(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info);

int bmac_write_mac_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info);
int bmac_write_mac_txp_pao(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info);

int bmac_write_hif_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info);
int bmac_write_hif_txp_v2(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info);
int bmac_write_hif_txp_v3(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info);

int bmac_write_txd(struct mtk_hw_dev *dev, struct mtk_hw_sta *sta,
	struct mtk_tk_entry *tk_entry, void *tx_pkt_info);
int bmac_write_txp(struct mtk_hw_dev *dev, struct mtk_tk_entry *tk_entry,
				void *tx_pkt_info);

void
bmac_tx_free_notify(struct mtk_bus_trans *trans,
			struct sk_buff *skb);
void
bmac_rx_data(struct mtk_bus_trans *trans,
			struct sk_buff *skb);
void
bmac_rx_ics(struct mtk_bus_trans *trans,
			struct sk_buff *skb);
void
bmac_rx_txs(struct mtk_bus_trans *trans,
			struct sk_buff *skb);
void
bmac_rx_sdo_event(struct mtk_bus_trans *trans,
			struct sk_buff *skb);

void bmac_ops_init(struct mtk_hw_dev *dev);
void bmac_skb_unmap(struct mtk_hw_dev *dev, __le32 *buf, u8 buf_cnt);
void bmac_dump_txd(__le32 *txd);
void bmac_dump_mac_txp(__le32 *txp);
void bmac_dump_hif_txp(__le32 *txp);
void bmac_dump_hif_txp_v2(__le32 *txp);
void bmac_dump_hif_txp_v3(__le32 *txp);
void bmac_dump_rxd(__le32 *rxd);

#endif
