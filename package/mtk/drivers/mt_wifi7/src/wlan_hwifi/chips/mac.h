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


#ifndef __MAC_H__
#define __MAC_H__

#include <linux/types.h>

#define SEC_CIPHER_NONE 0
#define IS_CIPHER_NONE(_Cipher) (((_Cipher) & (1 << SEC_CIPHER_NONE)) > 0)


#define MT_TXD_SIZE (8 * 4)
#define MT_TX_RETRY_UNLIMIT	0x1f
#define MT_TX_SHORT_RETRY	0x07
#define MT_TX_LONG_RETRY	0x0f

#define MT_TX_PSE_HDR_LEN	8
#define MT_TX_ETH_HDR_LEN	14
#define MT_TX_ETH_TYPE_LEN	2

/*
 *Bit[1]: padding mode selection
 * 1'b0: pad the dummy bytes in the tail of header
 * 1'b1: pad the dummy bytes in the head of header
 *Bit[0]: padding 2 byte length
*/
#define MT_HDR_PAD_TAIL_NO_PAD	0
#define MT_HDR_PAD_TAIL_PAD		1
#define MT_HDR_PAD_HEAD_NO_PAD	2
#define MT_HDR_PAD_HEAD_PAD		3

enum txd_len_page {
	MT_TXD_LEN_1_PAGE,
	MT_TXD_LEN_2_PAGE,
	MT_TXD_LEN_3_PAGE,
	MT_TXD_LEN_4_PAGE,
};

enum tx_pkt_type {
	MT_TX_TYPE_CT,
	MT_TX_TYPE_SF,
	MT_TX_TYPE_CMD,
	MT_TX_TYPE_FW,
};

enum tx_header_format {
	MT_HDR_FORMAT_802_3,
	MT_HDR_FORMAT_CMD,
	MT_HDR_FORMAT_802_11,
	MT_HDR_FORMAT_802_11_EXT,
};

enum tx_pkt_queue_idx {
	MT_LMAC_AC00,
	MT_LMAC_AC01,
	MT_LMAC_AC02,
	MT_LMAC_AC03,
	MT_LMAC_ALTX0 = 0x10,
	MT_LMAC_BMC0 = 0x10,
	MT_LMAC_BCN0 = 0x12,
};

enum tx_port_idx {
	MT_TX_PORT_IDX_LMAC,
	MT_TX_PORT_IDX_MCU
};

enum tx_mcu_port_q_idx {
	MT_TX_MCU_PORT_RX_Q0 = 0x20,
	MT_TX_MCU_PORT_RX_Q1,
	MT_TX_MCU_PORT_RX_Q2,
	MT_TX_MCU_PORT_RX_Q3,
	MT_TX_MCU_PORT_RX_FWDL = 0x3e
};

#define MT_TXP_MAX_BUF_NUM		6
#define HIF_TXP_BUF_PTR_MASK	0xffffffff
#define HIF_TXP_BUF_PTR_SHIFT	0
#define HIF_TXP_BUF_LEN_MASK	0x00000fff
#define HIF_TXP_BUF_LEN_SHIFT	0
#define HIF_TXP_BUF_PTR_H_MASK	0x0000f000
#define HIF_TXP_BUF_PTR_H_SHIFT	12
#define DMA_ADDR_H_SHIFT		32

struct hif_txp {
	__le16 flags;
	__le16 token;
	u8 bss_idx;
	__le16 rept_wds_wcid;
	u8 nbuf;
	__le32 buf[MT_TXP_MAX_BUF_NUM];
	__le16 len[MT_TXP_MAX_BUF_NUM];
} __packed __aligned(4);

#define MAC_TXP_SIZE (8 * 4)
#define HIF_TXP_SIZE (11 * 4)
#define HIF_TXP_V2_SIZE (24 * 4)
#define HIF_TXP_V3_SIZE (23 * 4)

#define MT_CT_INFO_APPLY_TXD		BIT(0)
#define MT_CT_INFO_COPY_HOST_TXD_ALL	BIT(1)
#define MT_CT_INFO_MGMT_FRAME		BIT(2)
#define MT_CT_INFO_NONE_CIPHER_FRAME	BIT(3)
#define MT_CT_INFO_HSR2_TX		BIT(4)
#define MT_CT_INFO_PTK_NO_ACK BIT(5)
#define MT_CT_INFO_PKT_FR_HOST	BIT(7)

enum {
	MSDU_FORMAT,
	FINAL_AMSDU_FORMAT,
	MIDDLE_AMSDU_FORMAT,
	FIRST_AMSDU_FORMAT,
};

enum rx_pkt_type {
	PKT_TYPE_TXS,
	PKT_TYPE_TXRXV,
	PKT_TYPE_NORMAL,
	PKT_TYPE_RX_DUP_RFB,
	PKT_TYPE_RX_TMR,
	PKT_TYPE_RETRIEVE,
	PKT_TYPE_TXRX_NOTIFY,
	PKT_TYPE_RX_EVENT,
	PKT_TYPE_RX_ICS = 0xC,
	PKT_TYPE_RX_SDO_EVENT = 0xD,
	PKT_TYPE_TXRX_NOTIFY_V0 = 0x18,
	PKT_TYPE_MAX_VAL = 0x19,
};

/* TODO: included from wlan_driver */
#define FC_TYPE_MGMT 0
#define FC_TYPE_CNTL 1
#define FC_TYPE_DATA 2
#define FC_TYPE_RSVED 3

#define RXD_SW_PKT_TYPE_MASK	GENMASK(31, 16)
#define RXD_SW_PKT_TYPE_BITMAP	0x380F
#define RXD_SW_PKT_TYPE_FRAME	0x3801

#endif
