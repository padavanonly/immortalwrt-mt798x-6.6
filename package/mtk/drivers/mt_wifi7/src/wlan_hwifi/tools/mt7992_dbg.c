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

#include <linux/kernel.h>
#include <linux/module.h>
#include "mt7915_coda/bn0_wf_mib_top.h"
#include "mt7915_coda/bn1_wf_mib_top.h"
#include "mt7915_coda/wf_wtblon_top.h"
#include "mt7915_coda/wf_pse_top.h"
#include "mt7915_coda/wf_ple_top.h"
#include "mt7915_coda/bn0_wf_agg_top.h"
#include "mt7915_coda/bn1_wf_agg_top.h"
#include "mt7915_coda/wf_uwtbl_top.h"
#include "mt7992_coda/mt7992_dbg.h"
#include "mt7992_coda/wf_rro_top.h"
#include "core.h"
#include "hw_ops.h"
#include "main.h"
#include <bus/mtk_rro.h>
#include "chips/mt7992/wf_wfdma_host_dma0.h"
#include "chips/mt7992/wf_wfdma_host_dma0_pcie1.h"
#include "chips/mt7992/mt7992.h"

#ifdef CONFIG_HWIFI_DBG_ISR
#include <bus/mtk_pci_dma.h>
#endif

static char *rmac_info_type_str[] = {
	"TXS",
	"RXV",
	"RxNormal",
	"DupRFB",
	"TMR",
	"Unknown",
};

static char *rxd_pkt_type_str(int pkt_type)
{
	if (pkt_type >= 0 && pkt_type <= 0x04)
		return rmac_info_type_str[pkt_type];
	else
		return rmac_info_type_str[5];
}

static struct mtk_hw_dev *
seqfile_to_dev(struct seq_file *s)
{
	void *trans = dev_get_drvdata(s->private);
	struct mtk_hw_dev *dev = to_hw_dev(trans);

	return dev;
}

static void
hex_dump(unsigned char *src, size_t len)
{
	unsigned char *p;
	int i;

	p = src;
	pr_info("%s: %p, len = %zu\n", "dump:", src, len);
	for (i = 0 ; i < len; i++) {
		if (i % 16 == 0)
			pr_info("0x%04x : ", i);
		pr_info("%02x ", ((unsigned char)p[i]));
		if (i % 16 == 15)
			pr_info("\n");
	}
	pr_info("\n");
}

static void
mt7992_dump_rxd_info(u32 *rmac_info)
{
	struct rxd_grp_0 *rxd_grp0 = (struct rxd_grp_0 *)(rmac_info);

	hex_dump((u8 *)rxd_grp0, sizeof(*rxd_grp0));
	pr_info("RxData_BASE:\n");

	/* DW0 */
	pr_info("\tPktType = %d(%s), ",
			((rxd_grp0->rxd_0 & RXD_PKT_TYPE_MASK) >> RXD_PKT_TYPE_SHIFT),
			rxd_pkt_type_str((rxd_grp0->rxd_0 & RXD_PKT_TYPE_MASK) >> RXD_PKT_TYPE_SHIFT));

	pr_info("RxByteCnt = %d\n",
			((rxd_grp0->rxd_0 & RXD_RX_BYTE_COUNT_MASK) >> RXD_RX_BYTE_COUNT_SHIFT));
	pr_info("\tEtherTypeOffset = %d(WORD), ",
			((rxd_grp0->rxd_0 & RXD_ETH_TYPE_OFFSET_MASK) >> RXD_ETH_TYPE_OFFSET_SHIFT));
	pr_info("IP/UT = %d/%d, ",
			(rxd_grp0->rxd_0 & RXD_IP) ? 1 : 0,
			(rxd_grp0->rxd_0 & RXD_UT) ? 1 : 0);

	/* DW1 */
	pr_info("SEC_DONE = %d\n",
			(rxd_grp0->rxd_1 & RXD_SEC_DONE) ? 1 : 0);
	pr_info("\tWlanIndex = %d, ",
			((rxd_grp0->rxd_1 & RXD_WLAN_IDX_MASK) >> RXD_WLAN_IDX_SHIFT));
	pr_info("GroupValid = 0x%x, ",
			((rxd_grp0->rxd_1 & RXD_GROUP_VLD_MASK) >> RXD_GROUP_VLD_SHIFT));
	pr_info("BN = %d, ",
				(rxd_grp0->rxd_1 & RXD_BN) ? 1 : 0);
	pr_info("SPP_EN = %d\n",
			(rxd_grp0->rxd_1 & RXD_SPP_EN) ? 1 : 0);
	pr_info("\tSEC Mode = %d, ",
			((rxd_grp0->rxd_1 & RXD_SEC_MODE_MASK) >> RXD_SEC_MODE_SHIFT));
	pr_info("KeyID = %d, ",
			((rxd_grp0->rxd_1 & RXD_KID_MASK) >> RXD_KID_SHIFT));
	pr_info("CM = %d, ",
			(rxd_grp0->rxd_1 & RXD_CM) ? 1 : 0);
	pr_info("CLM = %d,",
			(rxd_grp0->rxd_1 & RXD_CLM) ? 1 : 0);
	pr_info("ADD_OM = %d\n",
			(rxd_grp0->rxd_1 & RXD_ADD_OM) ? 1 : 0);
	pr_info("\tICV Err(I) = %d, ",
			(rxd_grp0->rxd_1 & RXD_ICV_ERR) ? 1 : 0);
	pr_info("TKIP MIC Err(T) = %d, ",
			(rxd_grp0->rxd_1 & RXD_TKIPMIC_ERR) ? 1 : 0);
	pr_info("FCE Error(FC) = %d\n",
			(rxd_grp0->rxd_1 & RXD_FCS_ERR) ? 1 : 0);

	/* DW2 */
	pr_info("\tBSSID = %d, ",
			((rxd_grp0->rxd_2 & RXD_BSSID_MASK) >> RXD_BSSID_SHIFT));
	pr_info("BF_CQI = %d, ",
			(rxd_grp0->rxd_2 & RXD_BF_CQI) ? 1 : 0);
	pr_info("HdearLength(MAC) = %d\n",
			((rxd_grp0->rxd_2 & RXD_MAC_HDR_LEN_MASK) >> RXD_MAC_HDR_LEN_SHIFT));
	pr_info("\tHeaerTrans(H) = %d, ",
			(rxd_grp0->rxd_2 & RXD_H) ? 1 : 0);
	pr_info("HeaerOffset(HO) = %d, ",
			((rxd_grp0->rxd_2 & RXD_HO_MASK) >> RXD_HO_SHIFT));
	pr_info("TID = %d\n",
			((rxd_grp0->rxd_2 & RXD_TID_MASK) >> RXD_TID_SHIFT));
	pr_info("\tMU_BAR = %d, ",
			(rxd_grp0->rxd_2 & RXD_MU_BAR) ? 1 : 0);
	pr_info("SWBIT = %d, ",
			(rxd_grp0->rxd_2 & RXD_SWBIT) ? 1 : 0);
	pr_info("DeAMSDU Fail(DAF) = %d\n",
			(rxd_grp0->rxd_2 & RXD_DAF) ? 1 : 0);
	pr_info("\tExceedMax Rx Length(EL) = %d, ",
			(rxd_grp0->rxd_2 & RXD_EL) ? 1 : 0);
	pr_info("HdrTransFail(HTF) = %d\n",
			(rxd_grp0->rxd_2 & RXD_HTF) ? 1 : 0);
	pr_info("\tInterested Frame(INTF) = %d, ",
			(rxd_grp0->rxd_2 & RXD_INTF) ? 1 : 0);
	pr_info("Fragment Frame(FRAG) = %d\n",
			(rxd_grp0->rxd_2 & RXD_FRAG) ? 1 : 0);
	pr_info("\tNull Frame(NULL) = %d, ",
			(rxd_grp0->rxd_2 & RXD_NULL) ? 1 : 0);
	pr_info("Non Data Frame(NDATA) = %d\n",
			(rxd_grp0->rxd_2 & RXD_NDATA) ? 1 : 0);
	pr_info("\tNon-AMPDU Subframe(NASF) = %d, ",
			(rxd_grp0->rxd_2 & RXD_NAMP) ? 1 : 0);

	pr_info("BF_RPT = %d\n",
			(rxd_grp0->rxd_2 & RXD_BF_RPT) ? 1 : 0);

	/* DW3 */
	pr_info("\tRX Vector Sequence No = %d, ",
			((rxd_grp0->rxd_3 & RXD_RXV_SN_MASK) >> RXD_RXV_SN_SHIFT));
	pr_info("Channel Frequency = %d\n",
			((rxd_grp0->rxd_3 & RXD_CF_MASK) >> RXD_CF_SHIFT));
	pr_info("\tHTC/UC2ME/MC/BC = %d/%d/%d/%d\n",
			(rxd_grp0->rxd_3 & RXD_HTC) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x1) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x2) ? 1 : 0,
			(((rxd_grp0->rxd_3 & RXD_A1_TYPE_MASK) >> RXD_A1_TYPE_SHIFT) == 0x3) ? 1 : 0);
	pr_info("\tTCL = %d, ",
			(rxd_grp0->rxd_3 & RXD_TCL) ? 1 : 0);
	pr_info("BBM = %d, ",
			(rxd_grp0->rxd_3 & RXD_BBM) ? 1 : 0);
	pr_info("BU = %d, ",
			(rxd_grp0->rxd_3 & RXD_BU) ? 1 : 0);
	pr_info("AMS = %d, ",
			(rxd_grp0->rxd_3 & RXD_AMS) ? 1 : 0);
	pr_info("MESH = %d\n",
			(rxd_grp0->rxd_3 & RXD_MESH) ? 1 : 0);
	pr_info("\tMHCP = %d, ",
			(rxd_grp0->rxd_3 & RXD_MHCP) ? 1 : 0);
	pr_info("NO_INFO_WB = %d,",
			(rxd_grp0->rxd_3 & RXD_NO_INFO_WB) ? 1 : 0);
	pr_info("DIS_RHTR = %d, ",
			(rxd_grp0->rxd_3 & RXD_DIS_RHTR) ? 1 : 0);
	pr_info("PSS = %d\n",
			(rxd_grp0->rxd_3 & RXD_PSS) ? 1 : 0);
	pr_info("\tMORE = %d, ",
			(rxd_grp0->rxd_3 & RXD_MORE) ? 1 : 0);
	pr_info("UWAT = %d, ",
			(rxd_grp0->rxd_3 & RXD_UWAT) ? 1 : 0);
	pr_info("RX_DROP = %d, ",
			(rxd_grp0->rxd_3 & RXD_RX_DROP) ? 1 : 0);
	pr_info("VLAN2ETH = %d\n",
			(rxd_grp0->rxd_3 & RXD_VLAN2ETH) ? 1 : 0);

	/* DW4 */
	pr_info("\tPF = %d, ",
			((rxd_grp0->rxd_4 & RXD_PF_MASK) >> RXD_PF_SHIFT));
	pr_info("DP = %d, ",
			(rxd_grp0->rxd_4 & RXD_DP) ? 1 : 0);
	pr_info("CLS = %d, ",
			(rxd_grp0->rxd_4 & RXD_CLS) ? 1 : 0);
	pr_info("OFLD = %d, ",
			((rxd_grp0->rxd_4 & RXD_OFLD_MASK) >> RXD_OFLD_SHIFT));
	pr_info("MGC = %d\n",
			(rxd_grp0->rxd_4 & RXD_MGC) ? 1 : 0);
	pr_info("\tWOL = %d, ",
			((rxd_grp0->rxd_4 & RXD_WOL_MASK) >> RXD_WOL_SHIFT));
	pr_info("CLS_BITMAP = %d, ",
			((rxd_grp0->rxd_4 & RXD_CLS_BITMAP_MASK) >> RXD_CLS_BITMAP_SHIFT));
	pr_info("PF_MODE = %d, ",
			(rxd_grp0->rxd_4 & RXD_PF_MODE) ? 1 : 0);
	pr_info("PF_STS = %d\n",
			((rxd_grp0->rxd_4 & RXD_PF_STS_MASK) >> RXD_PF_STS_SHIFT));
}

static char *pkt_ft_str[] = {"cut_through", "store_forward", "cmd", "PDA_FW_Download"};
static char *hdr_fmt_str[] = {
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};

static void
mt7992_dump_txd_info(struct mtk_hw_dev *dev, u32 *txd_info)
{
	bool fixrate, vta;
	u32 txd = le32_to_cpu(txd_info[0]);

	pr_info("TMAC_TXD Fields:\n");
	pr_info("\tTMAC_TXD_0:\n");

	/* DW0 */
	/* TX Byte Count [15:0]  */
	pr_info("\t\tTxByteCnt = %d\n", ((txd & TXD_TX_BYTE_COUNT_MASK) >> TXD_TX_BYTE_COUNT_SHIFT));

	/* PKT_FT: Packet Format [24:23] */
	pr_info("\t\tpkt_ft = %d(%s)\n", ((txd & TXD_PKT_FT_MASK) >> TXD_PKT_FT_SHIFT),
				pkt_ft_str[((txd & TXD_PKT_FT_MASK) >> TXD_PKT_FT_SHIFT)]);

	/* Q_IDX [31:25]  */
	pr_info("\t\tQueID =0x%x\n", ((txd & TXD_Q_IDX_MASK) >> TXD_Q_IDX_SHIFT));

	pr_info("\tTMAC_TXD_1:\n");
	txd = le32_to_cpu(txd_info[1]);
	/* DW1 */
	/* WLAN Indec [9:0] */
	pr_info("\t\tWlan Index = %d\n", ((txd & TXD_WLAN_IDX_MASK) >> TXD_WLAN_IDX_SHIFT));

	/* HF: Header Format [17:16] */
	pr_info("\t\tHdrFmt = %d(%s)\n",
		((txd & TXD_HF_MASK) >> TXD_HF_SHIFT),
		hdr_fmt_str[((txd & TXD_HF_MASK) >> TXD_HF_SHIFT)]);


	switch ((txd & TXD_HF_MASK) >> TXD_HF_SHIFT) {
	case TMI_HDR_FT_NON_80211:

		/* MRD [11], EOSP [12], RMVL [13], VLAN [14], ETYPE [15] */
		pr_info("\t\t\tMRD = %d, EOSP = %d, RMVL = %d, VLAN = %d, ETYP = %d\n",
				(txd & TXD_MRD) ? 1 : 0,
				(txd & TXD_EOSP) ? 1 : 0,
				(txd & TXD_RMVL) ? 1 : 0,
				(txd & TXD_VLAN) ? 1 : 0,
				(txd & TXD_ETYP) ? 1 : 0);
		break;
	case TMI_HDR_FT_NOR_80211:
		/* HEADER_LENGTH [15:11] */
		pr_info("\t\t\tHeader Len = %d(WORD)\n",
				((txd & TXD_HDR_LEN_MASK) >> TXD_HDR_LEN_SHIFT));
		break;

	case TMI_HDR_FT_ENH_80211:
		/* EOSP [12], AMS [13]  */
		pr_info("\t\t\tEOSP = %d, AMS = %d\n",
				(txd & TXD_EOSP) ? 1 : 0,
				(txd & TXD_AMS) ? 1 : 0);
		break;
	}

	vta = txd & TXD_VTA;
	/* Header Padding [19:18] */
	pr_info("\t\tHdrPad = %d\n", ((txd & TXD_HDR_PAD_MASK) >> TXD_HDR_PAD_SHIFT));
	/* TID [22:20] */
	pr_info("\t\tTID = %d\n", (txd & TXD_TID_MASK) >> TXD_TID_SHIFT);
	/* UtxB/AMSDU_C/AMSDU [23] */
	pr_info("\t\tamsdu = %d\n", ((txd & TXD_AMSDU) ? 1 : 0));
	/* OM [29:24] */
	pr_info("\t\town_mac = %d\n", ((txd & TXD_OM_MASK) >> TXD_OM_SHIFT));
	/* TGID [30] */
	pr_info("\t\tTGID = %d\n", ((txd & TXD_TGID) ? 1 : 0));
	/* FT [31] */
	pr_info("\t\tTxDFormatType = %d\n", (txd & TXD_FT) ? 1 : 0);

	pr_info("\tTMAC_TXD_2:\n");
	txd = le32_to_cpu(txd_info[2]);
	/* DW2 */
	/* Subtype [3:0] */
	pr_info("\t\tsub_type = %d\n", ((txd & TXD_SUBTYPE_MASK) >> TXD_SUBTYPE_SHIFT));
	/* Type[5:4] */
	pr_info("\t\tfrm_type = %d\n", ((txd & TXD_TYPE_MASK) >> TXD_TYPE_SHIFT));
	/* NDP [6] */
	pr_info("\t\tNDP = %d\n", (txd & TXD_NDP) ? 1 : 0);
	/* NDPA [7] */
	pr_info("\t\tNDPA = %d\n", (txd & TXD_NDPA) ? 1 : 0);
	/* SD [8] */
	pr_info("\t\tSounding = %d\n", (txd & TXD_SD) ? 1 : 0);
	/* RTS [9] */
	pr_info("\t\tRTS = %d\n", (txd & TXD_RTS) ? 1 : 0);
	/* BM [10] */
	pr_info("\t\tbc_mc_pkt = %d\n", (txd & TXD_BM) ? 1 : 0);
	/* B [11]  */
	pr_info("\t\tBIP = %d\n", (txd & TXD_B) ? 1 : 0);
	/* DU [12] */
	pr_info("\t\tDuration = %d\n", (txd & TXD_DU) ? 1 : 0);
	/* HE [13] */
	pr_info("\t\tHE(HTC Exist) = %d\n", (txd & TXD_HE) ? 1 : 0);
	/* FRAG [15:14] */
	pr_info("\t\tFRAG = %d\n", (txd & TXD_FRAG_MASK) >> TXD_FRAG_SHIFT);

	/* Remaining Life Time [23:16]*/
	pr_info("\t\tReamingLife/MaxTx time = %d (unit: 64TU)\n",
	((txd & TXD_REMAIN_TIME_MASK) >> TXD_REMAIN_TIME_SHIFT));

	/* Power Offset [29:24] */
	pr_info("\t\tpwr_offset = %d\n",
		((txd & TXD_PWR_OFFESET_MASK) >> TXD_PWR_OFFESET_SHIFT));
	/* FRM [30] */
	pr_info("\t\tfix rate mode = %d\n", (txd & TXD_FRM) ? 1 : 0);
	/* FR[31] */
	pr_info("\t\tfix rate = %d\n", (txd & TXD_FR) ? 1 : 0);
	fixrate = txd & TXD_FR;
	pr_info("\tTMAC_TXD_3:\n");
	txd = le32_to_cpu(txd_info[3]);
	/* DW3 */
	/* NA [0] */
	pr_info("\t\tNoAck = %d\n", (txd & TXD_NA) ? 1 : 0);
	/* PF [1] */
	pr_info("\t\tPF = %d\n", (txd & TXD_PF) ? 1 : 0);
	/* EMRD [2] */
	pr_info("\t\tEMRD = %d\n", (txd & TXD_EMRD) ? 1 : 0);
	/* EEOSP [3] */
	pr_info("\t\tEEOSP = %d\n", (txd & TXD_EEOSP) ? 1 : 0);
	/* DAS [4] */
	pr_info("\t\tda_select = %d\n", (txd & TXD_DAS) ? 1 : 0);
	/* TM [5] */
	pr_info("\t\ttm = %d\n", (txd & TXD_TM) ? 1 : 0);
	/* TX Count [10:6] */
	pr_info("\t\ttx_cnt = %d\n", (txd & TXD_TX_CNT_MASK) >> TXD_TX_CNT_SHIFT);
	/* Remaining TX Count [15:11] */
	pr_info("\t\tremain_tx_cnt = %d\n",
	((txd & TXD_REMAIN_TX_CNT_MASK) >> TXD_REMAIN_TX_CNT_SHIFT));
	/* SN [27:16] */
	pr_info("\t\tsn = %d\n", (txd & TXD_SN_MASK) >> TXD_SN_SHIFT);
	/* BA_DIS [28] */
	pr_info("\t\tba dis = %d\n", (txd & TXD_BA_DIS) ? 1 : 0);
	/* Power Management [29] */
	pr_info("\t\tpwr_mgmt = 0x%x\n", (txd & TXD_PM) ? 1 : 0);
	/* PN_VLD [30] */
	pr_info("\t\tpn_vld = %d\n", (txd & TXD_PN_VLD) ? 1 : 0);
	/* SN_VLD [31] */
	pr_info("\t\tsn_vld = %d\n", (txd & TXD_SN_VLD) ? 1 : 0);

	/* DW4 */
	pr_info("\tTMAC_TXD_4:\n");
	txd = le32_to_cpu(txd_info[4]);
	/* PN_LOW [31:0] */
	pr_info("\t\tpn_low = 0x%x\n", (txd & TXD_PN1_MASK) >> TXD_PN1_SHIFT);
	/* DW5 */
	pr_info("\tTMAC_TXD_5:\n");
	txd = le32_to_cpu(txd_info[5]);
	/* PID [7:0] */
	pr_info("\t\tpid = %d\n", (txd & TXD_PID_MASK) >> TXD_PID_SHIFT);
	/* TXSFM [8] */
	pr_info("\t\ttx_status_fmt = %d\n", (txd & TXD_TXSFM) ? 1 : 0);
	/* TXS2M [9] */
	pr_info("\t\ttx_status_2_mcu = %d\n", (txd & TXD_TXS2M) ? 1 : 0);
	/* TXS2H [10] */
	pr_info("\t\ttx_status_2_host = %d\n", (txd & TXD_TXS2H) ? 1 : 0);
	/* ADD_BA [14] */
	pr_info("\t\tADD_BA = %d\n", (txd & TXD_ADD_BA) ? 1 : 0);
	/* MD [15] */
	pr_info("\t\tMD = %d\n", (txd & TXD_MD) ? 1 : 0);

	/* PN_HIGH [31:16]  */
	pr_info("\t\tpn_high = 0x%x\n", ((txd & TXD_PN2_MASK) >> TXD_PN2_SHIFT));
	/* DW6 */
	pr_info("\tTMAC_TXD_6:\n");
	txd = le32_to_cpu(txd_info[6]);
	if (fixrate) {
		/* Fixed BandWidth mode [2:0] */
		pr_info("\t\tbw = %d\n", ((txd & TXD_BW_MASK) >> TXD_BW_SHIFT));
		/* DYN_BW [3] */
		pr_info("\t\tdyn_bw = %d\n", (txd & TXD_DYN_BW) ? 1 : 0);
		/* ANT_ID [7:4] */
		pr_info("\t\tant_id = %d\n", ((txd & TXD_ANT_ID_MASK) >> TXD_ANT_ID_SHIFT));
		/* SPE_IDX_SEL [10] */
		pr_info("\t\tspe_idx_sel = %d\n", (txd & TXD_SPE_IDX_SEL) ? 1 : 0);
		/* LDPC [11] */
		pr_info("\t\tldpc = %d\n", (txd & TXD_LDPC) ? 1 : 0);
		/* HELTF Type[13:12] */
		pr_info("\t\tHELTF Type = %d\n", ((txd & TXD_HELTF_TYPE_MASK) >> TXD_HELTF_TYPE_SHIFT));

		/* GI Type [15:14] */
		pr_info("\t\tGI = %d\n", ((txd & TXD_GI_MASK) >> TXD_GI_SHIFT));
		/* Rate to be Fixed [29:16] */
		pr_info("\t\ttx_rate = 0x%x\n", ((txd & TXD_FR_RATE_MASK) >> TXD_FR_RATE_SHIFT));
	}

	/* TXEBF [30] */
	pr_info("\t\ttxebf = %d\n", (txd & TXD_TXEBF)  ? 1 : 0);
	/* TXIBF [31] */
	 pr_info("\t\ttxibf = %d\n", (txd & TXD_TXIBF) ? 1 : 0);
	/* DW7 */
	pr_info("\tTMAC_TXD_7:\n");
	txd = le32_to_cpu(txd_info[7]);
	if (!vta) {
		/* SW Tx Time [9:0] */
		pr_info("\t\tsw_tx_time = %d\n", ((txd & TXD_SW_TX_TIME_MASK) >> TXD_SW_TX_TIME_SHIFT));
	} else {
		/* TXD Arrival Time [9:0] */
		pr_info("\t\tat = %d\n", ((txd & TXD_TAT) >> TXD_TAT_SHIFT));
	}

	/* HW_AMSDU_CAP [10] */
	pr_info("\t\thw amsdu cap = %d\n", (txd & TXD_HW_AMSDU_CAP) ? 1 : 0);

	/* SPE_IDX [15:11] */
	if (fixrate)
		pr_info("\t\tspe_idx = 0x%x\n", ((txd & TXD_SPE_IDX_MASK) >> TXD_SPE_IDX_SHIFT));

	/* PSE_FID [27:16] */
	pr_info("\t\tpse_fid = 0x%x\n", ((txd & TXD_PSE_FID_MASK) >> TXD_PSE_FID_SHIFT));
	/* Subtype [19:16] */
	pr_info("\t\tpp_sub_type=%d\n", ((txd & TXD_PP_SUBTYPE_MASK) >> TXD_PP_SUBTYPE_SHIFT));
	/* Type [21:20] */
	pr_info("\t\tpp_type=%d\n", ((txd & TXD_PP_TYPE_MASK) >> TXD_PP_TYPE_SHIFT));
	/* CTXD_CNT [25:23] */
	pr_info("\t\tctxd cnt=0x%x\n", ((txd & TXD_CTXD_CNT_MASK) >> TXD_CTXD_CNT_SHIFT));
	/* CTXD [26] */
	pr_info("\t\tctxd = %d\n", (txd & TXD_CTXD) ? 1 : 0);
	/* I [28]  */
	pr_info("\t\ti = %d\n", (txd & TXD_IP_CHKSUM) ? 1 : 0);
	/* UT [29] */
	pr_info("\t\tUT = %d\n", (txd & TXD_UT) ? 1 : 0);
	/* TXDLEN [31:30] */
	pr_info("\t\t txd len= %d\n", ((txd & TXD_TXD_LEN_MASK) >> TXD_TXD_LEN_SHIFT));
}

static int
mt7992_dump_mib_info(struct seq_file *s, void *data)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;
#define bss_nums 1
	u32 mac_val0, mac_val, mac_val1, idx, band_idx = 0, band_offset = 0;
	u32 msdr6, msdr7, msdr8, msdr9, msdr10, msdr16, msdr17, msdr18, msdr19, msdr20, msdr21;
	u32 mbxsdr[bss_nums][4];
	u32 mbtcr[16], mbtbcr[16], mbrcr[16], mbrbcr[16];
	u32 btcr[bss_nums], btbcr[bss_nums], brcr[bss_nums], brbcr[bss_nums], btdcr[bss_nums], brdcr[bss_nums];
	u32 mu_cnt[5];
	u32 ampdu_cnt[3];
	unsigned long per;

	band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
	seq_printf(s, "Band %d MIB Status\n", band_idx);
	seq_puts(s, "===============================\n");
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SCR0_ADDR + band_offset);
	seq_printf(s, "MIB Status Control=0x%x\n", mac_val);
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0PBSCR_ADDR + band_offset);
	seq_printf(s, "MIB Per-BSS Status Control=0x%x\n", mac_val);

	msdr6 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR6_ADDR + band_offset);
	msdr7 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR7_ADDR + band_offset);
	msdr8 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR8_ADDR + band_offset);
	msdr9 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR9_ADDR + band_offset);
	msdr10 = bus_read(trans,  BN0_WF_MIB_TOP_M0SDR10_ADDR + band_offset);
	msdr16 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR16_ADDR + band_offset);
	msdr17 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR17_ADDR + band_offset);
	msdr18 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR18_ADDR + band_offset);
	msdr19 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR19_ADDR + band_offset);
	msdr20 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR20_ADDR + band_offset);
	msdr21 = bus_read(trans, BN0_WF_MIB_TOP_M0SDR21_ADDR + band_offset);
	ampdu_cnt[0] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR12_ADDR + band_offset);
	ampdu_cnt[1] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR14_ADDR + band_offset);
	ampdu_cnt[2] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR15_ADDR + band_offset);
	ampdu_cnt[1] &= BN0_WF_MIB_TOP_M0SDR14_AMPDU_MPDU_COUNT_MASK;
	ampdu_cnt[2] &= BN0_WF_MIB_TOP_M0SDR15_AMPDU_ACKED_COUNT_MASK;

	seq_puts(s, "===Phy/Timing Related Counters===\n");
	seq_printf(s, "\tChannelIdleCnt=0x%x\n", msdr6 & BN0_WF_MIB_TOP_M0SDR6_CHANNEL_IDLE_COUNT_MASK);
	seq_printf(s, "\tCCA_NAV_Tx_Time=0x%x\n", msdr9 & BN0_WF_MIB_TOP_M0SDR9_CCA_NAV_TX_TIME_MASK);
	seq_printf(s, "\tRx_MDRDY_CNT=0x%x\n", msdr10 & BN0_WF_MIB_TOP_M0SDR10_RX_MDRDY_COUNT_MASK);
	seq_printf(s, "\tCCK_MDRDY_TIME=0x%x, OFDM_MDRDY_TIME=0x%x, OFDM_GREEN_MDRDY_TIME=0x%x\n",
			 msdr19 & BN0_WF_MIB_TOP_M0SDR19_CCK_MDRDY_TIME_MASK,
			 msdr20 & BN0_WF_MIB_TOP_M0SDR20_OFDM_LG_MIXED_VHT_MDRDY_TIME_MASK,
			 msdr21 & BN0_WF_MIB_TOP_M0SDR21_OFDM_GREEN_MDRDY_TIME_MASK);
	seq_printf(s, "\tPrim CCA Time=0x%x\n", msdr16 & BN0_WF_MIB_TOP_M0SDR16_P_CCA_TIME_MASK);
	seq_printf(s, "\tSec CCA Time=0x%x\n", msdr17 & BN0_WF_MIB_TOP_M0SDR17_S_CCA_TIME_MASK);
	seq_printf(s, "\tPrim ED Time=0x%x\n", msdr18 & BN0_WF_MIB_TOP_M0SDR18_P_ED_TIME_MASK);

	seq_puts(s, "===Tx Related Counters(Generic)===\n");
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SDR0_ADDR + band_offset);
	seq_printf(s, "\tBeaconTxCnt=0x%x\n", mac_val);

	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0DR0_ADDR + band_offset);
	seq_printf(s, "\tTx 20MHz Cnt=0x%x\n", mac_val & BN0_WF_MIB_TOP_M0DR0_TX_20MHZ_CNT_MASK);
	seq_printf(s, "\tTx 40MHz Cnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR0_TX_40MHZ_CNT_SHFT);
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0DR1_ADDR + band_offset);
	seq_printf(s, "\tTx 80MHz Cnt=0x%x\n", mac_val & BN0_WF_MIB_TOP_M0DR1_TX_80MHZ_CNT_MASK);
	seq_printf(s, "\tTx 160MHz Cnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR1_TX_160MHZ_CNT_SHFT);
	seq_printf(s, "\tAMPDU Cnt=0x%x\n", ampdu_cnt[0]);
	seq_printf(s, "\tAMPDU MPDU Cnt=0x%x\n", ampdu_cnt[1]);
	seq_printf(s, "\tAMPDU MPDU Ack Cnt=0x%x\n", ampdu_cnt[2]);
	per = (ampdu_cnt[2] == 0 ? 0 : 1000 * (ampdu_cnt[1] - ampdu_cnt[2]) / ampdu_cnt[1]);
	seq_printf(s, "\tAMPDU MPDU PER=%ld.%1ld%%\n", per / 10, per % 10);

	seq_puts(s, "===MU Related Counters===\n");
	mu_cnt[0] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR34_ADDR + band_offset);
	mu_cnt[1] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR8_ADDR + band_offset);
	mu_cnt[2] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR9_ADDR + band_offset);
	mu_cnt[3] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR10_ADDR + band_offset);
	mu_cnt[4] = bus_read(trans, BN0_WF_MIB_TOP_M0SDR11_ADDR + band_offset);

	seq_printf(s, "\tMUBF_TX_COUNT=0x%x\n", mu_cnt[0] & BN0_WF_MIB_TOP_M0SDR34_MUBF_TX_COUNT_MASK);
	seq_printf(s, "\tMU_TX_MPDU_COUNT(Ok+Fail)=0x%x\n", mu_cnt[1]);
	seq_printf(s, "\tMU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[2]);
	seq_printf(s, "\tMU_TO_SU_PPDU_COUNT=0x%x\n", mu_cnt[3] & BN0_WF_MIB_TOP_M0DR10_MU_FAIL_PPDU_CNT_MASK);
	seq_printf(s, "\tSU_TX_OK_MPDU_COUNT=0x%x\n", mu_cnt[4]);

	seq_puts(s, "===Rx Related Counters(Generic)===\n");
	seq_printf(s, "\tVector Mismacth Cnt=0x%x\n", msdr7 & BN0_WF_MIB_TOP_M0SDR7_VEC_MISS_COUNT_MASK);
	seq_printf(s, "\tDelimiter Fail Cnt=0x%x\n", msdr8 & BN0_WF_MIB_TOP_M0SDR8_DELIMITER_FAIL_COUNT_MASK);

	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SDR3_ADDR + band_offset);
	seq_printf(s, "\tRxFCSErrCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR3_RX_FCS_ERROR_COUNT_MASK));
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SDR4_ADDR + band_offset);
	seq_printf(s, "\tRxFifoFullCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR4_RX_FIFO_FULL_COUNT_MASK));
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SDR11_ADDR + band_offset);
	seq_printf(s, "\tRxLenMismatch=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR11_RX_LEN_MISMATCH_MASK));
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SDR5_ADDR + band_offset);
	seq_printf(s, "\tRxMPDUCnt=0x%x\n", (mac_val & BN0_WF_MIB_TOP_M0SDR5_RX_MPDU_COUNT_MASK));
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SDR22_ADDR + band_offset);
	seq_printf(s, "\tRx AMPDU Cnt=0x%x\n", mac_val);
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0SDR23_ADDR + band_offset);
	seq_printf(s, "\tRx Total ByteCnt=0x%x\n", mac_val);

	band_offset = WF_WTBLON_TOP_B1BTCRn_ADDR - WF_WTBLON_TOP_B0BTCRn_ADDR;
	seq_puts(s, "===Per-BSS Related Tx/Rx Counters===\n");
	seq_puts(s, "BSS Idx   TxCnt/DataCnt  TxByteCnt  RxCnt/DataCnt  RxByteCnt\n");

	for (idx = 0; idx < bss_nums; idx++) {
		btcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0BTCRn_ADDR + band_offset + idx * 4);
		btbcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0BTBCRn_ADDR + band_offset + idx * 4);
		brcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0BRCRn_ADDR + band_offset + idx * 4);
		brbcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0BRBCRn_ADDR + band_offset + idx * 4);
		btdcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0BTDCRn_ADDR + band_offset + idx * 4);
		brdcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0BRDCRn_ADDR + band_offset + idx * 4);
	}

	for (idx = 0; idx < bss_nums; idx++) {
		seq_printf(s, "%d\t 0x%x/0x%x\t 0x%x \t 0x%x/0x%x \t 0x%x\n",
					idx, btcr[idx], btdcr[idx], btbcr[idx],
					brcr[idx], brdcr[idx], brbcr[idx]);
	}

	band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
	seq_puts(s, "===Per-MBSS Related MIB Counters===\n");
	seq_puts(s, "BSS Idx   RTSTx/RetryCnt  BAMissCnt  AckFailCnt  FrmRetry1/2/3Cnt\n");

	for (idx = 0; idx < bss_nums; idx++) {
		mbxsdr[idx][0] = bus_read(trans, BN0_WF_MIB_TOP_M0B0SDR0_ADDR + band_offset + idx * 0x10);
		mbxsdr[idx][1] = bus_read(trans, BN0_WF_MIB_TOP_M0B0SDR1_ADDR + band_offset + idx * 0x10);
		mbxsdr[idx][2] = bus_read(trans, BN0_WF_MIB_TOP_M0B0SDR2_ADDR + band_offset + idx * 0x10);
		mbxsdr[idx][3] = bus_read(trans, BN0_WF_MIB_TOP_M0B0SDR3_ADDR + band_offset + idx * 0x10);
	}

	for (idx = 0; idx < bss_nums; idx++) {
		seq_printf(s, "%d:\t0x%x/0x%x  0x%x \t 0x%x \t  0x%x/0x%x/0x%x\n",
			idx, (mbxsdr[idx][0] & BN0_WF_MIB_TOP_M0B0SDR0_RTSTXCOUNT_MASK),
			(mbxsdr[idx][0] & BN0_WF_MIB_TOP_M0B0SDR0_RTSRETRYCOUNT_MASK) >> BN0_WF_MIB_TOP_M0B0SDR0_RTSRETRYCOUNT_SHFT,
			(mbxsdr[idx][1] & BN0_WF_MIB_TOP_M0B0SDR1_BAMISSCOUNT_MASK),
			(mbxsdr[idx][1] & BN0_WF_MIB_TOP_M0B0SDR1_ACKFAILCOUNT_MASK) >> BN0_WF_MIB_TOP_M0B0SDR1_ACKFAILCOUNT_SHFT,
			(mbxsdr[idx][2] & BN0_WF_MIB_TOP_M0B0SDR2_FRAMERETRYCOUNT_MASK),
			(mbxsdr[idx][2] & BN0_WF_MIB_TOP_M0B0SDR2_FRAMERETRY2COUNT_MASK) >> BN0_WF_MIB_TOP_M0B0SDR2_FRAMERETRY2COUNT_SHFT,
			(mbxsdr[idx][3] & BN0_WF_MIB_TOP_M0B0SDR3_FRAMERETRY3COUNT_MASK));
	}

	seq_puts(s, "===Dummy delimiter insertion result===\n");
	mac_val0 = bus_read(trans, BN0_WF_MIB_TOP_M0DR12_ADDR + band_offset);
	mac_val = bus_read(trans, BN0_WF_MIB_TOP_M0DR6_ADDR + band_offset);
	mac_val1 = bus_read(trans, BN0_WF_MIB_TOP_M0DR7_ADDR + band_offset);
	seq_printf(s, "Range0 = %d\t Range1 = %d\t Range2 = %d\t Range3 = %d\t Range4 = %d\n",
		(mac_val0 & BN0_WF_MIB_TOP_M0DR12_TX_DDLMT_RNG0_CNT_MASK),
		(mac_val & BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG1_CNT_MASK),
		(mac_val & BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG2_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR6_TX_DDLMT_RNG2_CNT_SHFT,
		(mac_val1 & BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG3_CNT_MASK),
		(mac_val1 & BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG4_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR7_TX_DDLMT_RNG4_CNT_SHFT);

	band_offset = WF_WTBLON_TOP_B1BTCRn_ADDR - WF_WTBLON_TOP_B0BTCRn_ADDR;
	seq_puts(s, "===Per-MBSS Related Tx/Rx Counters===\n");
	seq_puts(s, "MBSSIdx   TxCnt  TxByteCnt  RxCnt  RxByteCnt\n");

	for (idx = 0; idx < 16; idx++) {
		mbtcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0MBTCRn_ADDR + band_offset + idx * 4);
		mbtbcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0MBTBCRn_ADDR + band_offset + idx * 4);
		mbrcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0MBRCRn_ADDR + band_offset + idx * 4);
		mbrbcr[idx] = bus_read(trans, WF_WTBLON_TOP_B0MBRBCRn_ADDR + band_offset + idx * 4);
	}

	for (idx = 0; idx < 16; idx++) {
		seq_printf(s, "%d\t 0x%x\t 0x%x \t 0x%x \t 0x%x\n",
			idx, mbtcr[idx], mbtbcr[idx], mbrcr[idx], mbrbcr[idx]);
	}
	return 0;
}

static struct empty_q_info pse_queue_empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 4~7 not defined */
	{"HIF Q0", ENUM_UMAC_HIF_PORT_0,    0}, /* Q8 */
	{"HIF Q1", ENUM_UMAC_HIF_PORT_0,    1},
	{"HIF Q2", ENUM_UMAC_HIF_PORT_0,    2},
	{"HIF Q3", ENUM_UMAC_HIF_PORT_0,    3},
	{"HIF Q4", ENUM_UMAC_HIF_PORT_0,    4},
	{"HIF Q5", ENUM_UMAC_HIF_PORT_0,    5},
	{NULL, 0, 0}, {NULL, 0, 0},  /* 14~15 not defined */
	{"LMAC Q",  ENUM_UMAC_LMAC_PORT_2,    0},
	{"MDP TX Q", ENUM_UMAC_LMAC_PORT_2, 1},
	{"MDP RX Q", ENUM_UMAC_LMAC_PORT_2, 2},
	{"SEC TX Q", ENUM_UMAC_LMAC_PORT_2, 3},
	{"SEC RX Q", ENUM_UMAC_LMAC_PORT_2, 4},
	{"SFD_PARK Q", ENUM_UMAC_LMAC_PORT_2, 5},
	{"MDP_TXIOC Q", ENUM_UMAC_LMAC_PORT_2, 6},
	{"MDP_RXIOC Q", ENUM_UMAC_LMAC_PORT_2, 7},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 24~30 not defined */
	{"RLS Q",  ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1F}
};

static int
mt7992_show_pse_info(struct seq_file *s, void *data)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;

	u32 pse_buf_ctrl, pg_sz, pg_num;
	u32 pse_stat, pg_flow_ctrl[22] = {0};
	u32 fpg_cnt, ffa_cnt, fpg_head, fpg_tail;
	u32 max_q, min_q, rsv_pg, used_pg;
	int i;

	pse_buf_ctrl = bus_read(trans, WF_PSE_TOP_PBUF_CTRL_ADDR);
	pse_stat = bus_read(trans, WF_PSE_TOP_QUEUE_EMPTY_ADDR);
	pg_flow_ctrl[0] = bus_read(trans, WF_PSE_TOP_FREEPG_CNT_ADDR);
	pg_flow_ctrl[1] = bus_read(trans, WF_PSE_TOP_FREEPG_HEAD_TAIL_ADDR);
	pg_flow_ctrl[2] = bus_read(trans, WF_PSE_TOP_PG_HIF0_GROUP_ADDR);
	pg_flow_ctrl[3] = bus_read(trans, WF_PSE_TOP_HIF0_PG_INFO_ADDR);
	pg_flow_ctrl[4] = bus_read(trans, WF_PSE_TOP_PG_HIF1_GROUP_ADDR);
	pg_flow_ctrl[5] = bus_read(trans, WF_PSE_TOP_HIF1_PG_INFO_ADDR);
	pg_flow_ctrl[6] = bus_read(trans, WF_PSE_TOP_PG_CPU_GROUP_ADDR);
	pg_flow_ctrl[7] = bus_read(trans, WF_PSE_TOP_CPU_PG_INFO_ADDR);
	pg_flow_ctrl[8] = bus_read(trans, WF_PSE_TOP_PG_LMAC0_GROUP_ADDR);
	pg_flow_ctrl[9] = bus_read(trans, WF_PSE_TOP_LMAC0_PG_INFO_ADDR);
	pg_flow_ctrl[10] = bus_read(trans, WF_PSE_TOP_PG_LMAC1_GROUP_ADDR);
	pg_flow_ctrl[11] = bus_read(trans, WF_PSE_TOP_LMAC1_PG_INFO_ADDR);
	pg_flow_ctrl[12] = bus_read(trans, WF_PSE_TOP_PG_LMAC2_GROUP_ADDR);
	pg_flow_ctrl[13] = bus_read(trans, WF_PSE_TOP_LMAC2_PG_INFO_ADDR);
	pg_flow_ctrl[14] = bus_read(trans, WF_PSE_TOP_PG_PLE_GROUP_ADDR);
	pg_flow_ctrl[15] = bus_read(trans, WF_PSE_TOP_PLE_PG_INFO_ADDR);
	pg_flow_ctrl[16] = bus_read(trans, WF_PSE_TOP_PG_LMAC3_GROUP_ADDR);
	pg_flow_ctrl[17] = bus_read(trans, WF_PSE_TOP_LMAC3_PG_INFO_ADDR);
	pg_flow_ctrl[18] = bus_read(trans, WF_PSE_TOP_PG_MDP_GROUP_ADDR);
	pg_flow_ctrl[19] = bus_read(trans, WF_PSE_TOP_MDP_PG_INFO_ADDR);
	pg_flow_ctrl[20] = bus_read(trans, WF_PSE_TOP_PG_PLE1_GROUP_ADDR);
	pg_flow_ctrl[21] = bus_read(trans, WF_PSE_TOP_PLE1_PG_INFO_ADDR);
	/* Configuration Info */
	seq_puts(s, "PSE Configuration Info:\n");
	seq_printf(s, "\tPacket Buffer Control(0x82068014): 0x%08x\n", pse_buf_ctrl);
	pg_sz = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PSE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	seq_printf(s, "\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 256 : 128));
	seq_printf(s, "\t\tPage Offset=%d(in unit of 64KB)\n",
			 (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PSE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (pse_buf_ctrl & WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PSE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	seq_printf(s, "\t\tTotal page numbers=%d pages\n", pg_num);
	/* Page Flow Control */
	seq_puts(s, "PSE Page Flow Control:\n");
	seq_printf(s, "\tFree page counter(0x82068100): 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	seq_printf(s, "\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PSE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PSE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	seq_printf(s, "\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	seq_printf(s, "\tFree page head and tail(0x82068104): 0x%08x\n", pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PSE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	seq_printf(s, "\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);
	seq_printf(s, "\tReserved page counter of HIF0 group(0x82068110): 0x%08x\n", pg_flow_ctrl[2]);
	seq_printf(s, "\tHIF0 group page status(0x82068114): 0x%08x\n", pg_flow_ctrl[3]);
	min_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[2] & WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF0_GROUP_HIF0_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of HIF0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[3] & WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_MASK) >> WF_PSE_TOP_HIF0_PG_INFO_HIF0_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of HIF0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	seq_printf(s, "\tReserved page counter of HIF1 group(0x82068118): 0x%08x\n", pg_flow_ctrl[4]);
	seq_printf(s, "\tHIF1 group page status(0x8206811c): 0x%08x\n", pg_flow_ctrl[5]);
	min_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[4] & WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_HIF1_GROUP_HIF1_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of HIF1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[5] & WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_MASK) >> WF_PSE_TOP_HIF1_PG_INFO_HIF1_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of HIF1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	seq_printf(s, "\tReserved page counter of CPU group(0x82068150): 0x%08x\n", pg_flow_ctrl[6]);
	seq_printf(s, "\tCPU group page status(0x82068154): 0x%08x\n", pg_flow_ctrl[7]);
	min_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[6] & WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[7] & WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PSE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	seq_printf(s, "\tReserved page counter of LMAC0 group(0x82068170): 0x%08x\n", pg_flow_ctrl[8]);
	seq_printf(s, "\tLMAC0 group page status(0x82068174): 0x%08x\n", pg_flow_ctrl[9]);
	min_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[8] & WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC0_GROUP_LMAC0_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of LMAC0 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[9] & WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC0_PG_INFO_LMAC0_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of LMAC0 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	seq_printf(s, "\tReserved page counter of LMAC1 group(0x82068178): 0x%08x\n", pg_flow_ctrl[10]);
	seq_printf(s, "\tLMAC1 group page status(0x8206817c): 0x%08x\n", pg_flow_ctrl[11]);
	min_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[10] & WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC1_GROUP_LMAC1_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of LMAC1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[11] & WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC1_PG_INFO_LMAC1_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of LMAC1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	seq_printf(s, "\tReserved page counter of LMAC2 group(0x82068180): 0x%08x\n", pg_flow_ctrl[11]);
	seq_printf(s, "\tLMAC2 group page status(0x82068184): 0x%08x\n", pg_flow_ctrl[12]);
	min_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[12] & WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC2_GROUP_LMAC2_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of LMAC2 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[13] & WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC2_PG_INFO_LMAC2_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of LMAC2 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	seq_printf(s, "\tReserved page counter of LMAC3 group(0x82068188): 0x%08x\n", pg_flow_ctrl[16]);
	seq_printf(s, "\tLMAC3 group page status(0x8206818c): 0x%08x\n", pg_flow_ctrl[17]);
	min_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[16] & WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_LMAC3_GROUP_LMAC3_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of LMAC3 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[17] & WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_MASK) >> WF_PSE_TOP_LMAC3_PG_INFO_LMAC3_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of LMAC3 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	seq_printf(s, "\tReserved page counter of PLE group(0x82068160): 0x%08x\n", pg_flow_ctrl[14]);
	seq_printf(s, "\tPLE group page status(0x82068164): 0x%08x\n", pg_flow_ctrl[15]);
	min_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[14] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of PLE group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[15] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of PLE group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	seq_printf(s, "\tReserved page counter of PLE1 group(0x82068168): 0x%08x\n", pg_flow_ctrl[14]);
	seq_printf(s, "\tPLE1 group page status(0x8206816c): 0x%08x\n", pg_flow_ctrl[15]);
	min_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[20] & WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_PLE_GROUP_PLE_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of PLE1 group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[21] & WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_MASK) >> WF_PSE_TOP_PLE_PG_INFO_PLE_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of PLE1 group=0x%03x/0x%03x\n", used_pg, rsv_pg);

	seq_printf(s, "\tReserved page counter of MDP group(0x82068198): 0x%08x\n", pg_flow_ctrl[18]);
	seq_printf(s, "\tMDP group page status(0x8206819c): 0x%08x\n", pg_flow_ctrl[19]);
	min_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MIN_QUOTA_SHFT;
	max_q = (pg_flow_ctrl[18] & WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_MASK) >> WF_PSE_TOP_PG_MDP_GROUP_MDP_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of MDP group=0x%03x/0x%03x\n", max_q, min_q);
	rsv_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_RSV_CNT_SHFT;
	used_pg = (pg_flow_ctrl[19] & WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_MASK) >> WF_PSE_TOP_MDP_PG_INFO_MDP_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of MDP group=0x%03x/0x%03x\n", used_pg, rsv_pg);
	/* Queue Empty Status */
	seq_puts(s, "PSE Queue Empty Status:\n");
	seq_printf(s, "\tQUEUE_EMPTY(0x820680b0): 0x%08x\n", pse_stat);
	seq_printf(s, "\t\tCPU Q0/1/2/3 empty=%d/%d/%d/%d\n",
			  (pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q0_EMPTY_SHFT,
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q1_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q2_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_CPU_Q3_EMPTY_SHFT));
	seq_printf(s, "\t\tHIF Q0/1/2/3/4/5 empty=%d/%d/%d/%d/%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_0_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_0_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_1_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_1_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_2_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_2_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_3_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_3_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_4_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_4_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_HIF_5_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_HIF_5_EMPTY_SHFT));
	seq_printf(s, "\t\tLMAC TX Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_LMAC_TX_QUEUE_EMPTY_SHFT));
	seq_printf(s, "\t\tMDP TX Q/RX Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RX_QUEUE_EMPTY_SHFT));
	seq_printf(s, "\t\tSEC TX Q/RX Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_TX_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SEC_RX_QUEUE_EMPTY_SHFT));
	seq_printf(s, "\t\tSFD PARK Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_SFD_PARK_QUEUE_EMPTY_SHFT));
	seq_printf(s, "\t\tMDP TXIOC Q/RXIOC Q empty=%d/%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_TXIOC_QUEUE_EMPTY_SHFT),
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_MDP_RXIOC_QUEUE_EMPTY_SHFT));
	seq_printf(s, "\t\tRLS Q empty=%d\n",
			  ((pse_stat & WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_MASK) >> WF_PSE_TOP_QUEUE_EMPTY_RLS_Q_EMTPY_SHFT));
	seq_puts(s, "Nonempty Q info:\n");

	for (i = 0; i < 31; i++) {
		if (((pse_stat & (0x1 << i)) >> i) == 0) {
			u32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (pse_queue_empty_info[i].name != NULL) {
				seq_printf(s, "\t%s: ", pse_queue_empty_info[i].name);
				fl_que_ctrl[0] |= WF_PSE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (pse_queue_empty_info[i].p_id << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (pse_queue_empty_info[i].q_id << WF_PSE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			fl_que_ctrl[0] |= (0x1 << 31);
			bus_write(trans, WF_PSE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			fl_que_ctrl[1] = bus_read(trans, WF_PSE_TOP_FL_QUE_CTRL_2_ADDR);
			fl_que_ctrl[2] = bus_read(trans, WF_PSE_TOP_FL_QUE_CTRL_3_ADDR);
			hfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >> WF_PSE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			seq_printf(s, "tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
		}
	}
	return 0;
}

static void
chip_get_ple_acq_stat(struct mtk_bus_trans *trans, u32 *ple_stat)
{
	ple_stat[0] = bus_read(trans, WF_PLE_TOP_QUEUE_EMPTY_ADDR);
	ple_stat[1] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR);
	ple_stat[2] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY1_ADDR);
	ple_stat[3] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY2_ADDR);
	ple_stat[4] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY3_ADDR);
	ple_stat[5] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY4_ADDR);
	ple_stat[6] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY5_ADDR);
	ple_stat[7] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY6_ADDR);
	ple_stat[8] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY7_ADDR);
	ple_stat[9] = bus_read(trans, WF_PLE_TOP_AC0_QUEUE_EMPTY8_ADDR);

	ple_stat[10] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR);
	ple_stat[11] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY1_ADDR);
	ple_stat[12] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY2_ADDR);
	ple_stat[13] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY3_ADDR);
	ple_stat[14] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY4_ADDR);
	ple_stat[15] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY5_ADDR);
	ple_stat[16] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY6_ADDR);
	ple_stat[17] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY7_ADDR);
	ple_stat[18] = bus_read(trans, WF_PLE_TOP_AC1_QUEUE_EMPTY8_ADDR);

	ple_stat[19] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR);
	ple_stat[20] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY1_ADDR);
	ple_stat[21] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY2_ADDR);
	ple_stat[22] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY3_ADDR);
	ple_stat[23] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY4_ADDR);
	ple_stat[24] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY5_ADDR);
	ple_stat[25] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY6_ADDR);
	ple_stat[26] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY7_ADDR);
	ple_stat[27] = bus_read(trans, WF_PLE_TOP_AC2_QUEUE_EMPTY8_ADDR);

	ple_stat[28] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR);
	ple_stat[29] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY1_ADDR);
	ple_stat[30] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY2_ADDR);
	ple_stat[31] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY3_ADDR);
	ple_stat[32] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY4_ADDR);
	ple_stat[33] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY5_ADDR);
	ple_stat[34] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY6_ADDR);
	ple_stat[35] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY7_ADDR);
	ple_stat[36] = bus_read(trans, WF_PLE_TOP_AC3_QUEUE_EMPTY8_ADDR);
}

static void
chip_get_ple_txcmd_stat(struct mtk_bus_trans *trans, u32 *ple_txcmd_stat)
{
	*ple_txcmd_stat = bus_read(trans, WF_PLE_TOP_NATIVE_TXCMD_QUEUE_EMPTY_ADDR);
}

static void
chip_get_dis_sta_map(struct mtk_bus_trans *trans, u32 *dis_sta_map)
{
	dis_sta_map[0] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP0_ADDR);
	dis_sta_map[1] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP1_ADDR);
	dis_sta_map[2] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP2_ADDR);
	dis_sta_map[3] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP3_ADDR);
	dis_sta_map[4] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP4_ADDR);
	dis_sta_map[5] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP5_ADDR);
	dis_sta_map[6] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP6_ADDR);
	dis_sta_map[7] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP7_ADDR);
	dis_sta_map[8] = bus_read(trans, WF_PLE_TOP_DIS_STA_MAP8_ADDR);
}

static void
chip_get_sta_pause(struct mtk_bus_trans *trans, u32 *sta_pause)
{
	sta_pause[0] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE0_ADDR);
	sta_pause[1] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE1_ADDR);
	sta_pause[2] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE2_ADDR);
	sta_pause[3] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE3_ADDR);
	sta_pause[4] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE4_ADDR);
	sta_pause[5] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE5_ADDR);
	sta_pause[6] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE6_ADDR);
	sta_pause[7] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE7_ADDR);
	sta_pause[8] = bus_read(trans, WF_PLE_TOP_STATION_PAUSE8_ADDR);
}


#define CR_NUM_OF_AC 9
#define ALL_CR_NUM_OF_ALL_AC (CR_NUM_OF_AC * 4)

static char *sta_ctrl_reg[] = {"ENABLE", "DISABLE", "PAUSE"};

static u32
chip_show_sta_acq_info(struct mtk_bus_trans *trans, u32 *ple_stat,
				   u32 *sta_pause, u32 *dis_sta_map, struct seq_file *s)
{
	int i, j;
	u32 total_nonempty_cnt = 0;

	for (j = 0; j < ALL_CR_NUM_OF_ALL_AC; j++) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				u32 hfid, tfid, pktcnt, ac_num = j / CR_NUM_OF_AC, ctrl = 0;
				u32 sta_num = i + (j % CR_NUM_OF_AC) * 32, fl_que_ctrl[3] = {0};
				u32 wmmidx = 0;

				seq_printf(s, "\tSTA%d AC%d: ", sta_num, ac_num);

				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ENUM_UMAC_LMAC_PORT_2 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ac_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				fl_que_ctrl[0] |= (sta_num << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
				bus_write(trans, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
				fl_que_ctrl[1] = bus_read(trans, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR);
				fl_que_ctrl[2] = bus_read(trans, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR);
				hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
				pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
					WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				seq_printf(s, "tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x",
						  tfid, hfid, pktcnt);

				if (((sta_pause[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % CR_NUM_OF_AC] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				seq_printf(s, " ctrl = %s", sta_ctrl_reg[ctrl]);
				seq_printf(s, " (wmmidx=%d)\n", wmmidx);

				total_nonempty_cnt++;
			}
		}
	}

	return total_nonempty_cnt;
}


static struct empty_q_info ple_txcmd_queue_empty_info[] = {
	{"AC00Q", ENUM_UMAC_LMAC_PORT_2, 0x40},
	{"AC01Q", ENUM_UMAC_LMAC_PORT_2, 0x41},
	{"AC02Q", ENUM_UMAC_LMAC_PORT_2, 0x42},
	{"AC03Q", ENUM_UMAC_LMAC_PORT_2, 0x43},
	{"AC10Q", ENUM_UMAC_LMAC_PORT_2, 0x44},
	{"AC11Q", ENUM_UMAC_LMAC_PORT_2, 0x45},
	{"AC12Q", ENUM_UMAC_LMAC_PORT_2, 0x46},
	{"AC13Q", ENUM_UMAC_LMAC_PORT_2, 0x47},
	{"AC20Q", ENUM_UMAC_LMAC_PORT_2, 0x48},
	{"AC21Q", ENUM_UMAC_LMAC_PORT_2, 0x49},
	{"AC22Q", ENUM_UMAC_LMAC_PORT_2, 0x4a},
	{"AC23Q", ENUM_UMAC_LMAC_PORT_2, 0x4b},
	{"AC30Q", ENUM_UMAC_LMAC_PORT_2, 0x4c},
	{"AC31Q", ENUM_UMAC_LMAC_PORT_2, 0x4d},
	{"AC32Q", ENUM_UMAC_LMAC_PORT_2, 0x4e},
	{"AC33Q", ENUM_UMAC_LMAC_PORT_2, 0x4f},
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2, 0x50},
	{"TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x51},
	{"TWT TSF-TF Q0", ENUM_UMAC_LMAC_PORT_2, 0x52},
	{"TWT DL Q0", ENUM_UMAC_LMAC_PORT_2, 0x53},
	{"TWT UL Q0", ENUM_UMAC_LMAC_PORT_2, 0x54},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2, 0x55},
	{"TF Q1", ENUM_UMAC_LMAC_PORT_2, 0x56},
	{"TWT TSF-TF Q1", ENUM_UMAC_LMAC_PORT_2, 0x57},
	{"TWT DL Q1", ENUM_UMAC_LMAC_PORT_2, 0x58},
	{"TWT UL Q1", ENUM_UMAC_LMAC_PORT_2, 0x59},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
};

static void
chip_show_txcmdq_info(struct mtk_bus_trans *trans, u32 ple_txcmd_stat, struct seq_file *s)
{
	int i;

	seq_puts(s, "Nonempty TXCMD Q info:\n");
	for (i = 0; i < 32 ; i++) {
		if (((ple_txcmd_stat & (0x1 << i)) >> i) == 0) {
			u32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_txcmd_queue_empty_info[i].name != NULL) {
				seq_printf(s, "\t%s: ", ple_txcmd_queue_empty_info[i].name);
				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ple_txcmd_queue_empty_info[i].p_id <<
							WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ple_txcmd_queue_empty_info[i].q_id <<
							WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			bus_write(trans, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			fl_que_ctrl[1] = bus_read(trans, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR);
			fl_que_ctrl[2] = bus_read(trans, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR);
			hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			seq_printf(s, "tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
		}
	}
}

static struct empty_q_info ple_queue_empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 4~7 not defined */
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0}, /* Q16 */
	{"BMC Q0",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BMC_0},
	{"BCN Q0",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BNC_0},
	{"PSMP Q0", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1},
	{"BMC Q1",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BMC_1},
	{"BCN Q1",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BNC_1},
	{"PSMP Q1", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1},
	{"NAF Q",   ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_NAF},
	{"NBCN Q",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_NBCN},
	{NULL, 0, 0}, {NULL, 0, 0}, /* 18, 19 not defined */
	{"FIXFID Q", ENUM_UMAC_LMAC_PORT_2, 0x1a},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, /* 21~29 not defined */
	{"RLS Q",   ENUM_PLE_CTRL_PSE_PORT_3, 0x7e},
	{"RLS2 Q",  ENUM_PLE_CTRL_PSE_PORT_3, 0x7f}
};

static void dump_ple_txd(struct mtk_hw_dev *dev, int fid)
{
#define MEM_SIZE 64
	int i = 0;
	u8 data[MEM_SIZE] = {0};
	u32 addr = 0;

	if (fid >= 0xFFF)
		return;

	addr = 0xa << 28 | fid << 16; /* TXD addr: 0x{a}{fid}{0000}*/
	mtk_hw_get_mem(dev, addr, &data[0]);

	for (i = 0; i < MEM_SIZE; i = i + 4)
		pr_info("DW%02d: 0x%02x%02x%02x%02x\n", i / 4, data[i + 3], data[i + 2],
				 data[i + 1], data[i]);

	mt7992_dump_txd_info(dev, (u32 *)&data[0]);
}

#ifdef CONFIG_HWIFI_DBG_ISR
static char *legacy_ring_bit_info[MT7992_MAX_BUS_NUM][MT7992_LEGACY_MAX_BIT_NUM] = {
	/* bus_idx 0: pcie 0 */
	[0] = {
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_16_SHFT] =
			"T16:FWDL",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_17_SHFT] =
			"T17:Cmd(H2WM)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_18_SHFT] =
			"T18:TXD0(H2WA)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_19_SHFT] =
			"T19:TXD1(H2WA)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_20_SHFT] =
			"T20:Cmd(H2WA)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_0_SHFT] =
			"R0:Event(WM2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_1_SHFT] =
			"R1:Event(WA2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_2_SHFT] =
			"R2:TxDone0(WA2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_3_SHFT] =
			"R3:TxDone1(WA2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_4_SHFT] =
			"R4:Data0(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_5_SHFT] =
			"R5:Data1(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_6_SHFT] =
			"R6:BUF1(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_7_SHFT] =
			"R7:TxDone1(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_8_SHFT] =
			"R8:BUF0(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_9_SHFT] =
			"R9:TxDone0(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_10_SHFT] =
			"R10:MSDU_PG0(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_11_SHFT] =
			"R11:MSDU_PG1(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_done_int_sts_12_SHFT] =
			"R12:MSDU_PG2(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_wf_rro_irq_sts_SHFT] =
			"IND:IND_CMD(MAC2H)",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_0_SHFT] =
			"T0",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_1_SHFT] =
			"T1",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_2_SHFT] =
			"T2",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_3_SHFT] =
			"T3",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_4_SHFT] =
			"T4",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_5_SHFT] =
			"T5",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_6_SHFT] =
			"T6",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_rx_coherent_int_sts_SHFT] =
			"rx_coherent_int_sts",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_coherent_int_sts_SHFT] =
			"tx_coherent_int_sts",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_subsys_int_sts_SHFT] =
			"subsys_int_sts",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_mcu2host_sw_int_sts_SHFT] =
			"mcu2host_sw_int_sts",
		[WF_WFDMA_HOST_DMA0_HOST_INT_STA_tx_done_int_sts_15_SHFT] =
			"T15",
		[24] =
			"wpdma2host_err_int_sts"
	},
	/* bus_idx 1: pcie 1 */
	[1] = {
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_21_SHFT] =
			"T21:TXD2(H2WA)",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_22_SHFT] =
			"T22:TXD?(H2WA)",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_3_SHFT] =
			"R3:TxDone1(WA2H)",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_4_SHFT] =
			"R4",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_5_SHFT] =
			"R5:Data1(MAC2H)",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_7_SHFT] =
			"R7:TxDone1(MAC2H)",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_7_SHFT] =
			"T7",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_8_SHFT] =
			"T8",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_9_SHFT] =
			"T9",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_10_SHFT] =
			"T10",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_11_SHFT] =
			"T11",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_12_SHFT] =
			"T12",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_13_SHFT] =
			"T13",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_14_SHFT] =
			"T14",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_8_SHFT] =
			"R8",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_9_SHFT] =
			"R9",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_10_SHFT] =
			"R10",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_11_SHFT] =
			"R11",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_12_SHFT] =
			"R12",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_done_int_sts_19_SHFT] =
			"T19",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_coherent_int_sts_SHFT] =
			"rx_coherent_int_sts",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_tx_coherent_int_sts_SHFT] =
			"tx_coherent_int_sts",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_wpdma2host_err_int_sts_SHFT] =
			"wpdma2host_err_int_sts",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_subsys_int_sts_SHFT] =
			"subsys_int_sts",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_mcu2host_sw_int_sts_SHFT] =
			"mcu2host_sw_int_sts",
		[WF_WFDMA_HOST_DMA0_PCIE1_HOST_INT_STA_PCIE1_rx_done_int_sts_13_SHFT] =
			"R13"
	}
};

static char *rss_ring_bit_info[MT7992_MAX_BUS_NUM][MT7992_RSS_MAX_BIT_NUM] = {
	[0] = {
		[WF_WFDMA_HOST_DMA0_RSS_AP_INT_STA_rss_ap_int_sts_0_SHFT] =
			"rss_ap_int_sts_0",
		[WF_WFDMA_HOST_DMA0_RSS_AP_INT_STA_rss_ap_int_sts_1_SHFT] =
			"rss_ap_int_sts_1",
		[WF_WFDMA_HOST_DMA0_RSS_AP_INT_STA_rss_ap_int_sts_2_SHFT] =
			"rss_ap_int_sts_2",
		[WF_WFDMA_HOST_DMA0_RSS_AP_INT_STA_rss_ap_int_sts_3_SHFT] =
			"rss_ap_int_sts_3"
	}
};

static int
mt7992_show_isr_info(struct mtk_bus_trans *trans, u8 bus_id)
{
	struct pci_trans *ptrans = to_pci_trans(trans);
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	u32 len = 0;
	int ret;
	int i;
	char buf[MAX_BUF_LEN] = {0};
	char *bit_ring;

	pr_info("\n=============bus %u (pcie %u)=============\n", bus_id, bus_id);

	switch (dev->irq_type) {
	case PCI_IRQ_LEGACY:
		pr_info("IRQ type: INTx\n");
		break;
	case PCI_IRQ_MSI:
		pr_info("IRQ type: MSI\n");
		break;
	default:
		pr_info("IRQ type: other\n");
		break;
	}

	for (i = 0; i < dev->vec_num; i++) {
		ret = snprintf(buf + len, sizeof(buf) - len, "%d ", ptrans->vec_data[i].irq);

		if (ret < 0) {
			dev_err(dev->dev, "%s(): snprintf error\n", __func__);
			return -EIO;
		}

		if (ret > (sizeof(buf) - len - 1)) {
			dev_err(dev->dev, "%s(): MAX_BUF_LEN size error: %zu\n", __func__,
				ret - (sizeof(buf) - len - 1));
			return -EOVERFLOW;
		}

		len = strlen(buf);
	}
	pr_info("IRQ number: %s\n", buf);

	pr_info("interrupt status:\n");
	pr_info("  Legacy: 0x%08x\n",
		bus_read(trans, ptrans->int_mask_map[LEGACY_IDX].int_sts_addr));

	if (dev->rss_enable)
		pr_info("  RSS: 0x%08x\n",
			bus_read(trans, ptrans->int_mask_map[RSS_IDX].int_sts_addr));

	pr_info("interrupt mask:\n");
	pr_info("  Legacy: 0x%08x\n",
		bus_read(trans, ptrans->int_mask_map[LEGACY_IDX].int_ena_addr));

	if (dev->rss_enable)
		pr_info("  RSS: 0x%08x\n",
			bus_read(trans, ptrans->int_mask_map[RSS_IDX].int_ena_addr));

	pr_info("%25s%15s\n", "name", "counter");
	pr_info("========================================\n");
	pr_info("%25s%15d\n\n", "total IRQ received",
		atomic_read(&trans->bus_dbg.irq_cnt));
	atomic_set(&trans->bus_dbg.irq_cnt, 0);

	if (!trans->bus_dbg.legacy_ring_irq_cnt) {
		dev_err(dev->dev, "%s(): null pointer: legacy_ring_irq_cnt\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < MT7992_LEGACY_MAX_BIT_NUM; i++) {
		bit_ring = legacy_ring_bit_info[bus_id][i];

		if (!bit_ring)
			continue;

		pr_info("%25s%15d\n", bit_ring,
			atomic_read(&trans->bus_dbg.legacy_ring_irq_cnt[i]));
		atomic_set(&trans->bus_dbg.legacy_ring_irq_cnt[i], 0);
	}

	if (trans->bus_dbg.rss_ring_irq_cnt) {
		for (i = 0; i < MT7992_RSS_MAX_BIT_NUM; i++) {
			bit_ring = rss_ring_bit_info[bus_id][i];

			if (!bit_ring)
				continue;

			pr_info("%25s%15d\n", bit_ring,
				atomic_read(&trans->bus_dbg.rss_ring_irq_cnt[i]));
			atomic_set(&trans->bus_dbg.rss_ring_irq_cnt[i], 0);
		}
	}

	return 0;
}
#endif

static int
mt7992_show_ple_info(struct seq_file *s, void *data)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;
	u32 ple_buf_ctrl, pg_sz, pg_num;
	u32 ple_stat[ALL_CR_NUM_OF_ALL_AC + 1] = {0}, pg_flow_ctrl[CR_NUM_OF_AC] = {0};
	u32 ple_native_txcmd_stat;
	u32 ple_txcmd_stat;
	u32 sta_pause[CR_NUM_OF_AC] = {0}, dis_sta_map[CR_NUM_OF_AC] = {0};
	u32 fpg_cnt, ffa_cnt, fpg_head, fpg_tail, hif_max_q, hif_min_q;
	u32 rpg_hif, upg_hif, cpu_max_q, cpu_min_q, rpg_cpu, upg_cpu;
	int i, j;

	ple_buf_ctrl = bus_read(trans, WF_PLE_TOP_PBUF_CTRL_ADDR);
	chip_get_ple_acq_stat(trans, ple_stat);
	ple_txcmd_stat = bus_read(trans, WF_PLE_TOP_TXCMD_QUEUE_EMPTY_ADDR);
	chip_get_ple_txcmd_stat(trans, &ple_native_txcmd_stat);
	pg_flow_ctrl[0] = bus_read(trans, WF_PLE_TOP_FREEPG_CNT_ADDR);
	pg_flow_ctrl[1] = bus_read(trans, WF_PLE_TOP_FREEPG_HEAD_TAIL_ADDR);
	pg_flow_ctrl[2] = bus_read(trans, WF_PLE_TOP_PG_HIF_GROUP_ADDR);
	pg_flow_ctrl[3] = bus_read(trans, WF_PLE_TOP_HIF_PG_INFO_ADDR);
	pg_flow_ctrl[4] = bus_read(trans, WF_PLE_TOP_PG_CPU_GROUP_ADDR);
	pg_flow_ctrl[5] = bus_read(trans, WF_PLE_TOP_CPU_PG_INFO_ADDR);
	pg_flow_ctrl[6] = bus_read(trans, WF_PLE_TOP_PG_HIF_TXCMD_GROUP_ADDR);
	pg_flow_ctrl[7] = bus_read(trans, WF_PLE_TOP_HIF_TXCMD_PG_INFO_ADDR);
	chip_get_dis_sta_map(trans, dis_sta_map);
	chip_get_sta_pause(trans, sta_pause);

	/* Configuration Info */
	seq_puts(s, "PLE Configuration Info:\n");
	seq_printf(s, "\tPacket Buffer Control(0x82060014): 0x%08x\n", ple_buf_ctrl);
	pg_sz = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >> WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	seq_printf(s, "\t\tPage Size=%d(%d bytes per page)\n", pg_sz, (pg_sz == 1 ? 128 : 64));
	seq_printf(s, "\t\tPage Offset=%d(in unit of 2KB)\n",
			 (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >> WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >> WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	seq_printf(s, "\t\tTotal Page=%d pages\n", pg_num);
	/* Page Flow Control */
	seq_puts(s, "PLE Page Flow Control:\n");
	seq_printf(s, "\tFree page counter(0x820c0100): 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	seq_printf(s, "\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FFA_CNT_MASK) >> WF_PLE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	seq_printf(s, "\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	seq_printf(s, "\tFree page head and tail(0x820c0104): 0x%08x\n", pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] & WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >> WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	seq_printf(s, "\t\tThe tail/head page of free page list=0x%03x/0x%03x\n", fpg_tail, fpg_head);
	seq_printf(s, "\tReserved page counter of HIF group(0x820c0110): 0x%08x\n", pg_flow_ctrl[2]);
	seq_printf(s, "\tHIF group page status(0x820c0114): 0x%08x\n", pg_flow_ctrl[3]);
	hif_min_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_SHFT;
	hif_max_q = (pg_flow_ctrl[2] & WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n", hif_max_q, hif_min_q);
	rpg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	upg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n", upg_hif, rpg_hif);

	seq_printf(s, "\tReserved page counter of HIF_TXCMD group(0x820c0118): 0x%08x\n", pg_flow_ctrl[6]);
	seq_printf(s, "\tHIF_TXCMD group page status(0x820c011c): 0x%08x\n", pg_flow_ctrl[7]);
	cpu_min_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[6] & WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of HIF_TXCMD group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[7] & WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_MASK) >> WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of HIF_TXCMD group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	seq_printf(s, "\tReserved page counter of CPU group(0x820c0150): 0x%08x\n", pg_flow_ctrl[4]);
	seq_printf(s, "\tCPU group page status(0x820c0154): 0x%08x\n", pg_flow_ctrl[5]);
	cpu_min_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[4] & WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >> WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	seq_printf(s, "\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n", cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >> WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	seq_printf(s, "\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n", upg_cpu, rpg_cpu);

	if ((ple_stat[0] & WF_PLE_TOP_QUEUE_EMPTY_ALL_AC_EMPTY_MASK) == 0) {
		for (j = 0; j < ALL_CR_NUM_OF_ALL_AC; j++) {
			if (j % CR_NUM_OF_AC == 0)
				seq_printf(s, "\n\tNonempty AC%d Q of STA#: ", j / CR_NUM_OF_AC);

			for (i = 0; i < 32; i++) {
				if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0)
					seq_printf(s, "%d ", i + (j % CR_NUM_OF_AC) * 32);
			}
		}
		seq_puts(s, "\n");
	}

	seq_printf(s, "non-native/native txcmd queue empty = %d/%d\n", ple_txcmd_stat, ple_native_txcmd_stat);

	seq_puts(s, "Nonempty Q info:\n");

	for (i = 0; i < 32; i++) {
		if (((ple_stat[0] & (0x1 << i)) >> i) == 0) {
			u32 hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_queue_empty_info[i].name != NULL) {
				seq_printf(s, "\t%s: ", ple_queue_empty_info[i].name);
				fl_que_ctrl[0] |= WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |= (ple_queue_empty_info[i].p_id << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |= (ple_queue_empty_info[i].q_id << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			if (ple_queue_empty_info[i].q_id >= ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0 &&
				ple_queue_empty_info[i].q_id <= ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0)
				/* band0 set TGID 0, bit31 = 0 */
				bus_write(trans, WF_PLE_TOP_FL_QUE_CTRL_1_ADDR, 0x0);
			else if (ple_queue_empty_info[i].q_id >= ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1 &&
				ple_queue_empty_info[i].q_id <= ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1)
				/* band1 set TGID 1, bit31 = 1 */
				bus_write(trans, WF_PLE_TOP_FL_QUE_CTRL_1_ADDR, 0x80000000);

			bus_write(trans, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR, fl_que_ctrl[0]);
			fl_que_ctrl[1] = bus_read(trans, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR);
			fl_que_ctrl[2] = bus_read(trans, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR);
			hfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] & WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt = (fl_que_ctrl[2] & WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >> WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			seq_printf(s, "tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
					  tfid, hfid, pktcnt);
			if (pktcnt > 0)
				dump_ple_txd(to_hw_dev(trans), 0);
		}
	}

	chip_show_sta_acq_info(trans, ple_stat, sta_pause, dis_sta_map, s);
	chip_show_txcmdq_info(trans, ple_native_txcmd_stat, s);
	return 0;
}

static int
mt7992_dump_agg_info(struct seq_file *s, void *data)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;
	u32 value, idx, agg_rang_sel[15], ampdu_cnt[11], band_offset = 0, total_ampdu = 0;
	u8 band_idx = 0;

	for (band_idx = 0; band_idx < 2; band_idx++) {
		band_offset = (BN1_WF_AGG_TOP_BASE - BN0_WF_AGG_TOP_BASE) * band_idx;

		seq_printf(s, "Band %d AGG Status\n", band_idx);
		seq_puts(s, "===============================\n");
		value = bus_read(trans, BN0_WF_AGG_TOP_AALCR0_ADDR + band_offset);
		seq_printf(s, "AC00 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC00_AGG_LIMIT_SHFT);
		seq_printf(s, "AC01 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC01_AGG_LIMIT_SHFT);
		seq_printf(s, "AC02 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR0_AC02_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC02_AGG_LIMIT_SHFT);
		seq_printf(s, "AC03 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR0_AC03_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR0_AC03_AGG_LIMIT_SHFT);
		value = bus_read(trans, BN0_WF_AGG_TOP_AALCR1_ADDR + band_offset);
		seq_printf(s, "AC10 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC10_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC10_AGG_LIMIT_SHFT);
		seq_printf(s, "AC11 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC11_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC11_AGG_LIMIT_SHFT);
		seq_printf(s, "AC12 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR1_AC12_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC12_AGG_LIMIT_SHFT);
		seq_printf(s, "AC13 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR1_AC13_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR1_AC13_AGG_LIMIT_SHFT);
		value = bus_read(trans, BN0_WF_AGG_TOP_AALCR2_ADDR + band_offset);
		seq_printf(s, "AC20 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC20_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC20_AGG_LIMIT_SHFT);
		seq_printf(s, "AC21 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC21_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC21_AGG_LIMIT_SHFT);
		seq_printf(s, "AC22 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR2_AC22_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC22_AGG_LIMIT_SHFT);
		seq_printf(s, "AC23 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR2_AC23_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR2_AC23_AGG_LIMIT_SHFT);
		value = bus_read(trans, BN0_WF_AGG_TOP_AALCR3_ADDR + band_offset);
		seq_printf(s, "AC30 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC30_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC30_AGG_LIMIT_SHFT);
		seq_printf(s, "AC31 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC31_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC31_AGG_LIMIT_SHFT);
		seq_printf(s, "AC32 Agg limit = %d\t", (value & BN0_WF_AGG_TOP_AALCR3_AC32_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC32_AGG_LIMIT_SHFT);
		seq_printf(s, "AC33 Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR3_AC33_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR3_AC33_AGG_LIMIT_SHFT);
		value = bus_read(trans, BN0_WF_AGG_TOP_AALCR4_ADDR + band_offset);
		seq_printf(s, "ALTX Agg limit = %d\n", (value & BN0_WF_AGG_TOP_AALCR4_ALTX0_AGG_LIMIT_MASK) >>  BN0_WF_AGG_TOP_AALCR4_ALTX0_AGG_LIMIT_SHFT);

		value = bus_read(trans, BN0_WF_AGG_TOP_AWSCR0_ADDR + band_offset);
		seq_printf(s, "Winsize0 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE0_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE0_SHFT);
		seq_printf(s, "Winsize1 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE1_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE1_SHFT);
		seq_printf(s, "Winsize2 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE2_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE2_SHFT);
		seq_printf(s, "Winsize3 limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR0_WINSIZE3_MASK) >> BN0_WF_AGG_TOP_AWSCR0_WINSIZE3_SHFT);
		value = bus_read(trans, BN0_WF_AGG_TOP_AWSCR1_ADDR + band_offset);
		seq_printf(s, "Winsize4 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE4_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE4_SHFT);
		seq_printf(s, "Winsize5 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE5_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE5_SHFT);
		seq_printf(s, "Winsize6 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE6_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE6_SHFT);
		seq_printf(s, "Winsize7 limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR1_WINSIZE7_MASK) >> BN0_WF_AGG_TOP_AWSCR1_WINSIZE7_SHFT);
		value = bus_read(trans, BN0_WF_AGG_TOP_AWSCR2_ADDR + band_offset);
		seq_printf(s, "Winsize8 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZE8_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZE8_SHFT);
		seq_printf(s, "Winsize9 limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZE9_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZE9_SHFT);
		seq_printf(s, "WinsizeA limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZEA_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZEA_SHFT);
		seq_printf(s, "WinsizeB limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR2_WINSIZEB_MASK) >> BN0_WF_AGG_TOP_AWSCR2_WINSIZEB_SHFT);
		value = bus_read(trans, BN0_WF_AGG_TOP_AWSCR3_ADDR + band_offset);
		seq_printf(s, "WinsizeC limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZEC_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZEC_SHFT);
		seq_printf(s, "WinsizeD limit = %d\t", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZED_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZED_SHFT);
		seq_printf(s, "WinsizeE limit = %d\n", (value & BN0_WF_AGG_TOP_AWSCR3_WINSIZEE_MASK) >> BN0_WF_AGG_TOP_AWSCR3_WINSIZEE_SHFT);

		band_offset = (BN1_WF_MIB_TOP_BASE - BN0_WF_MIB_TOP_BASE) * band_idx;
		seq_puts(s, "===AMPDU Related Counters===\n");
		value = bus_read(trans, BN0_WF_MIB_TOP_M0ARNG0_ADDR + band_offset);
		agg_rang_sel[0] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_0_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_0_SHFT;
		agg_rang_sel[1] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_1_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_1_SHFT;
		agg_rang_sel[2] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_2_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_2_SHFT;
		agg_rang_sel[3] = (value & BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_3_MASK) >> BN0_WF_MIB_TOP_M0ARNG0_AGG_RANG_SEL_3_SHFT;
		value = bus_read(trans, BN0_WF_MIB_TOP_M0ARNG1_ADDR + band_offset);
		agg_rang_sel[4] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_4_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_4_SHFT;
		agg_rang_sel[5] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_5_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_5_SHFT;
		agg_rang_sel[6] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_6_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_6_SHFT;
		agg_rang_sel[7] = (value & BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_7_MASK) >> BN0_WF_MIB_TOP_M0ARNG1_AGG_RANG_SEL_7_SHFT;
		value = bus_read(trans, BN0_WF_MIB_TOP_M0ARNG2_ADDR + band_offset);
		agg_rang_sel[8] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_8_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_8_SHFT;
		agg_rang_sel[9] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_9_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_9_SHFT;
		agg_rang_sel[10] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_10_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_10_SHFT;
		agg_rang_sel[11] = (value & BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_11_MASK) >> BN0_WF_MIB_TOP_M0ARNG2_AGG_RANG_SEL_11_SHFT;
		value = bus_read(trans, BN0_WF_MIB_TOP_M0ARNG3_ADDR + band_offset);
		agg_rang_sel[12] = (value & BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_12_MASK) >> BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_12_SHFT;
		agg_rang_sel[13] = (value & BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_13_MASK) >> BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_13_SHFT;
		agg_rang_sel[14] = (value & BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_14_MASK) >> BN0_WF_MIB_TOP_M0ARNG3_AGG_RANG_SEL_14_SHFT;

		/* Need to add 1 after read from AGG_RANG_SEL CR */
		for (idx = 0; idx < 15; idx++)
			agg_rang_sel[idx]++;

		ampdu_cnt[3] = bus_read(trans, BN0_WF_MIB_TOP_M0DR2_ADDR + band_offset);
		ampdu_cnt[4] = bus_read(trans, BN0_WF_MIB_TOP_M0DR3_ADDR + band_offset);
		ampdu_cnt[5] = bus_read(trans, BN0_WF_MIB_TOP_M0DR4_ADDR + band_offset);
		ampdu_cnt[6] = bus_read(trans, BN0_WF_MIB_TOP_M0DR5_ADDR + band_offset);
		ampdu_cnt[7] = bus_read(trans, BN0_WF_MIB_TOP_M0DR13_ADDR + band_offset);
		ampdu_cnt[8] = bus_read(trans, BN0_WF_MIB_TOP_M0DR14_ADDR + band_offset);
		ampdu_cnt[9] = bus_read(trans, BN0_WF_MIB_TOP_M0DR15_ADDR + band_offset);
		ampdu_cnt[10] = bus_read(trans, BN0_WF_MIB_TOP_M0DR16_ADDR + band_offset);

		seq_printf(s,
				"\tTx Agg Range: \t%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d \t%d~%d\n",
				 agg_rang_sel[0],
				 agg_rang_sel[0] + 1, agg_rang_sel[1],
				 agg_rang_sel[1] + 1, agg_rang_sel[2],
				 agg_rang_sel[2] + 1, agg_rang_sel[3],
				 agg_rang_sel[3] + 1, agg_rang_sel[4],
				 agg_rang_sel[4] + 1, agg_rang_sel[5],
				 agg_rang_sel[5] + 1, agg_rang_sel[6],
				 agg_rang_sel[6] + 1, agg_rang_sel[7]);

#define BIT_0_to_15_MASK 0x0000FFFF
#define BIT_15_to_31_MASK 0xFFFF0000
#define SHFIT_16_BIT 16

		for (idx = 3; idx < 11; idx++)
			total_ampdu = total_ampdu + (ampdu_cnt[idx] & BIT_0_to_15_MASK) + ((ampdu_cnt[idx] & BIT_15_to_31_MASK) >> SHFIT_16_BIT);

		seq_printf(s,
				"\t\t\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				 (ampdu_cnt[3]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK,
				 (ampdu_cnt[3] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT,
				 (ampdu_cnt[4]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK,
				 (ampdu_cnt[4] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT,
				 (ampdu_cnt[5]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK,
				 (ampdu_cnt[5] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT,
				 (ampdu_cnt[6]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK,
				 (ampdu_cnt[6] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT);

		if (total_ampdu != 0) {
			seq_printf(s,
					"\t\t\t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%)\n",
					 ((ampdu_cnt[3]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[3] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[4]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[4] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[5]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[5] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[6]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[6] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT) * 100 / total_ampdu);
			}

		seq_printf(s,
				"\t\t\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~%d\t%d~256\n",
				 agg_rang_sel[7] + 1, agg_rang_sel[8],
				 agg_rang_sel[8] + 1, agg_rang_sel[9],
				 agg_rang_sel[9] + 1, agg_rang_sel[10],
				 agg_rang_sel[10] + 1, agg_rang_sel[11],
				 agg_rang_sel[11] + 1, agg_rang_sel[12],
				 agg_rang_sel[12] + 1, agg_rang_sel[13],
				 agg_rang_sel[13] + 1, agg_rang_sel[14],
				 agg_rang_sel[14] + 1);

		seq_printf(s,
				"\t\t\t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x \t0x%x\n",
				 (ampdu_cnt[7]) & BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE8_CNT_MASK,
				 (ampdu_cnt[7] & BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE9_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR13_TRX_AGG_RANGE9_CNT_SHFT,
				 (ampdu_cnt[8]) & BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE10_CNT_MASK,
				 (ampdu_cnt[8] & BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE11_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR14_TRX_AGG_RANGE11_CNT_SHFT,
				 (ampdu_cnt[9]) & BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE12_CNT_MASK,
				 (ampdu_cnt[9] & BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE13_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR15_TRX_AGG_RANGE13_CNT_SHFT,
				 (ampdu_cnt[10]) & BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE14_CNT_MASK,
				 (ampdu_cnt[10] & BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE15_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR16_TRX_AGG_RANGE15_CNT_SHFT);

		if (total_ampdu != 0) {
			seq_printf(s,
					"\t\t\t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%) \t(%d%%)\n",
					 ((ampdu_cnt[7]) & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE0_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[7] & BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR2_TRX_AGG_RANGE1_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[8]) & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE2_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[8] & BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR3_TRX_AGG_RANGE3_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[9]) & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE4_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[9] & BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR4_TRX_AGG_RANGE5_CNT_SHFT) * 100 / total_ampdu,
					 ((ampdu_cnt[10]) & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE6_CNT_MASK) * 100 / total_ampdu,
					 ((ampdu_cnt[10] & BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_MASK) >> BN0_WF_MIB_TOP_M0DR5_TRX_AGG_RANGE7_CNT_SHFT) * 100 / total_ampdu);
			}
	}
	return 0;
}

static u32
halWtblReadRaw(
	struct mtk_bus_trans *trans,
	u16  u2EntryIdx,
	ENUM_WTBL_TYPE_T  eType,
	u16  u2StartDW,
	u16  u2LenInDW,
	u8 *pBuffer
)
{
	u32 *dest_cpy = (u32 *)pBuffer;
	u32 sizeInDW = u2LenInDW;
	u32 u4SrcAddr = 0;

	if (pBuffer == NULL)
		return 0xFF;

	if (eType == WTBL_TYPE_LMAC) {
		LWTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = LWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_UMAC) {
		UWTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = UWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_KEY) {
		KEYTBL_CONFIG(u2EntryIdx);
		u4SrcAddr = KEYTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else {
		/* TODO: */
	}

	while (sizeInDW--) {
		*dest_cpy++ = bus_read(trans, u4SrcAddr);
		u4SrcAddr += 4;
	}

	return 0;
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW0[] = {
	{"MUAR_IDX",    WTBL_MUAR_IDX_MASK, WTBL_MUAR_IDX_OFFSET,	false},
	{"RCA1",        WTBL_RCA1,          NO_SHIFT_DEFINE,	false},
	{"KID",         WTBL_KID_MASK,      WTBL_KID_OFFSET,	false},
	{"RCID",        WTBL_RCID,          NO_SHIFT_DEFINE,	false},
	{"FROM_DS",     WTBL_FROM_DS,       NO_SHIFT_DEFINE,	true},
	{"TO_DS",       WTBL_TO_DS,         NO_SHIFT_DEFINE,	false},
	{"RV",          WTBL_RV,            NO_SHIFT_DEFINE,	false},
	{"RCA2",        WTBL_RCA2,          NO_SHIFT_DEFINE,	false},
	{"WPI_FLAG",    WTBL_WPI_FLAG,      NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void
parse_fmac_lwtbl_DW0_1(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	seq_printf(s,
			"\n\tAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
			  lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]);

	/* LMAC WTBL DW 0 */
	seq_puts(s, "\nLWTBL DW 0/1\n\t");
	addr = (u32 *) &(lwtbl[WTBL_GROUP_PEER_INFO_DW_0*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW0[i].name) {

		if (WTBL_LMAC_DW0[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW0[i].name,
					 (dw_value & WTBL_LMAC_DW0[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW0[i].name,
					  (dw_value & WTBL_LMAC_DW0[i].mask) >> WTBL_LMAC_DW0[i].shift);

		if (WTBL_LMAC_DW0[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW2[] = {
	{"AID12",               WTBL_AID12_MASK,                WTBL_AID12_OFFSET,	false},
	{"SU",                  WTBL_GID_SU,                    NO_SHIFT_DEFINE,	false},
	{"SPP_EN",              WTBL_SPP_EN,                    NO_SHIFT_DEFINE,	false},
	{"WPI_EVEN",            WTBL_WPI_EVEN,                  NO_SHIFT_DEFINE,	true},
	{"CIPHER",              WTBL_CIPHER_SUITE_MASK,         WTBL_CIPHER_SUITE_OFFSET,		false},
	{"CIPHER_IGTK",         WTBL_CIPHER_SUITE_IGTK_MASK,    WTBL_CIPHER_SUITE_IGTK_OFFSET,	false},
	{"AAD_OM",              WTBL_AAD_OM,                    NO_SHIFT_DEFINE,	true},
	{"SW",                  WTBL_SW,                        NO_SHIFT_DEFINE,	false},
	{"UL",                  WTBL_UL,                        NO_SHIFT_DEFINE,	false},
	{"TX_POWER_SAVE",       WTBL_TX_POWER_SAVE_STATUS,      NO_SHIFT_DEFINE,	true},
	{"QOS",                 WTBL_QOS,                       NO_SHIFT_DEFINE,	false},
	{"HT",                  WTBL_HT_RAW,                        NO_SHIFT_DEFINE,	false},
	{"VHT",                 WTBL_VHT_RAW,                       NO_SHIFT_DEFINE,	false},
	{"HE",                  WTBL_HE_RAW,                        NO_SHIFT_DEFINE,	false},
	{"MESH",                WTBL_MESH_RAW,                      NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void
parse_fmac_lwtbl_DW2(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 2 */
	seq_puts(s, "\nLWTBL DW 2\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW2[i].name) {

		if (WTBL_LMAC_DW2[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW2[i].name,
					 (dw_value & WTBL_LMAC_DW2[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW2[i].name,
					  (dw_value & WTBL_LMAC_DW2[i].mask) >> WTBL_LMAC_DW2[i].shift);
		if (WTBL_LMAC_DW2[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW3[] = {
	{"WMM_Q",           WTBL_WMM_Q_MASK,        WTBL_WMM_Q_OFFSET,	false},
	{"RXD_DUP_MODE",    WTBL_RXD_DUP_MODE_MASK, WTBL_RXD_DUP_MODE_OFFSET, true},
	{"VLAN2ETH",        WTBL_VLAN2ETH,          NO_SHIFT_DEFINE,	false},
	{"BEAM_CHG",        WTBL_BEAM_CHG,          NO_SHIFT_DEFINE,	false},
	{"DIS_BA256",       WTBL_DIS_BA256,         NO_SHIFT_DEFINE,	true},
	{"PFMU_IDX",        WTBL_PFMU_IDX_MASK,     WTBL_PFMU_IDX_OFFSET,	false},
	{"ULPF_IDX",        WTBL_ULPF_IDX_MASK,     WTBL_ULPF_IDX_OFFSET,	false},
	{"RIBF",            WTBL_RIBF,              NO_SHIFT_DEFINE,	false},
	{"ULPF",            WTBL_ULPF,              NO_SHIFT_DEFINE,	true},
	{"IGN_FBK",         WTBL_IGN_FBK,           NO_SHIFT_DEFINE,	false},
	{"TBF",             WTBL_TBF,               NO_SHIFT_DEFINE,	false},
	{"TBF_VHT",         WTBL_TBF_VHT,           NO_SHIFT_DEFINE,	false},
	{"TBF_HE",          WTBL_TBF_HE,            NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void
parse_fmac_lwtbl_DW3(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 3 */
	seq_puts(s, "\nLWTBL DW 3\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_3*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW3[i].name) {

		if (WTBL_LMAC_DW3[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW3[i].name,
					 (dw_value & WTBL_LMAC_DW3[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW3[i].name,
					  (dw_value & WTBL_LMAC_DW3[i].mask) >> WTBL_LMAC_DW3[i].shift);
		if (WTBL_LMAC_DW3[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW4[] = {
	{"ANT_ID_STS0",     WTBL_ANT_ID_STS0_MASK,      WTBL_ANT_ID_STS0_OFFSET,	false},
	{"STS1",            WTBL_ANT_ID_STS1_MASK,      WTBL_ANT_ID_STS1_OFFSET,	false},
	{"STS2",            WTBL_ANT_ID_STS2_MASK,      WTBL_ANT_ID_STS2_OFFSET,	false},
	{"STS3",            WTBL_ANT_ID_STS3_MASK,      WTBL_ANT_ID_STS3_OFFSET,	true},
	{"ANT_ID_STS4",     WTBL_ANT_ID_STS4_MASK,      WTBL_ANT_ID_STS4_OFFSET,	false},
	{"STS5",            WTBL_ANT_ID_STS5_MASK,      WTBL_ANT_ID_STS5_OFFSET,	false},
	{"STS6",            WTBL_ANT_ID_STS6_MASK,      WTBL_ANT_ID_STS6_OFFSET,	false},
	{"STS7",            WTBL_ANT_ID_STS7_MASK,      WTBL_ANT_ID_STS7_OFFSET,	true},
	{"CASCAD",          WTBL_CASCAD,                NO_SHIFT_DEFINE,	false},
	{"LDPC_HT",         WTBL_LDPC_HT,               NO_SHIFT_DEFINE,	false},
	{"LDPC_VHT",        WTBL_LDPC_VHT,              NO_SHIFT_DEFINE,	false},
	{"LDPC_HE",         WTBL_LDPC_HE,               NO_SHIFT_DEFINE,	true},
	{"DIS_RHTR",        WTBL_DIS_RHTR,              NO_SHIFT_DEFINE,	false},
	{"ALL_ACK",         WTBL_ALL_ACK,               NO_SHIFT_DEFINE,	false},
	{"DROP",            WTBL_DROP,                  NO_SHIFT_DEFINE,	false},
	{"ACK_EN",          WTBL_ACK_EN,                NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void
parse_fmac_lwtbl_DW4(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 4 */
	seq_puts(s, "\nLWTBL DW 4\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW4[i].name) {
		if (WTBL_LMAC_DW4[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW4[i].name,
					 (dw_value & WTBL_LMAC_DW4[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW4[i].name,
					  (dw_value & WTBL_LMAC_DW4[i].mask) >> WTBL_LMAC_DW4[i].shift);

		if (WTBL_LMAC_DW4[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW5[] = {
	{"AF",                  WTBL_AF_MASK,               WTBL_AF_OFFSET,		false},
	{"AF_HE",               WTBL_AF_HE_MASK,            WTBL_AF_HE_OFFSET,	false},
	{"RTS",                 WTBL_RTS,                   NO_SHIFT_DEFINE,	false},
	{"SMPS",                WTBL_SMPS,                  NO_SHIFT_DEFINE,	false},
	{"DYN_BW",              WTBL_DYN_BW,                NO_SHIFT_DEFINE,	true},
	{"MMSS",                WTBL_MMSS_MASK,             WTBL_MMSS_OFFSET,	false},
	{"USR",                 WTBL_USR,                   NO_SHIFT_DEFINE,	false},
	{"SR_RATE",             WTBL_SR_RATE_MASK,          WTBL_SR_RATE_OFFSET,	false},
	{"SR_ABORT",            WTBL_SR_ABORT,              NO_SHIFT_DEFINE,	true},
	{"TX_POWER_OFFSET",     WTBL_TX_POWER_OFFSET_MASK,  WTBL_TX_POWER_OFFSET_OFFSET,	false},
	{"WTBL_MPDU_SIZE",      WTBL_MPDU_SIZE_MASK,        WTBL_MPDU_SIZE_OFFSET,	true},
	{"PE",                  WTBL_PE_MASK,               WTBL_PE_OFFSET,		false},
	{"DOPPL",               WTBL_DOPPL,                 NO_SHIFT_DEFINE,	false},
	{"TXOP_PS_CAP",         WTBL_TXOP_PS_CAP,           NO_SHIFT_DEFINE,	false},
	{"DONOT_UPDATE_I_PSM",  WTBL_DONOT_UPDATE_I_PSM,    NO_SHIFT_DEFINE,	true},
	{"I_PSM",               WTBL_I_PSM,                 NO_SHIFT_DEFINE,	false},
	{"PSM",                 WTBL_PSM,                   NO_SHIFT_DEFINE,	false},
	{"SKIP_TX",             WTBL_SKIP_TX,               NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void
parse_fmac_lwtbl_DW5(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 5 */
	seq_puts(s, "\nLWTBL DW 5\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_5*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW5[i].name) {
		if (WTBL_LMAC_DW5[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW5[i].name,
					 (dw_value & WTBL_LMAC_DW5[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW5[i].name,
					  (dw_value & WTBL_LMAC_DW5[i].mask) >> WTBL_LMAC_DW5[i].shift);
		if (WTBL_LMAC_DW5[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static void
parse_fmac_lwtbl_DW6(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	int i = 0;

	/* LMAC WTBL DW 6 */
	seq_puts(s, "\nLWTBL DW 6\n");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_6*4]);
	dw_value = *addr;

	seq_puts(s, "\tTID 0/1/2/3/4/5/6/7 BA_WIN_SIZE:");

	for (i = 0; i < 8; i++)
		if (i != 7)
			seq_printf(s, "%lu/", ((dw_value & GENMASK(i*4+3, i*4)) >> i*4));
		else
			seq_printf(s, "%lu\n", ((dw_value & GENMASK(i*4+3, i*4)) >> i*4));
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW7[] = {
	{"CBRN",        WTBL_CBRN_MASK,     WTBL_CBRN_OFFSET,	false},
	{"DBNSS_EN",    WTBL_DBNSS_EN,      NO_SHIFT_DEFINE,	false},
	{"BAF_EN",      WTBL_BAF_EN,        NO_SHIFT_DEFINE,	false},
	{"RDGBA",       WTBL_RDGBA,         NO_SHIFT_DEFINE,	true},
	{"RDG",         WTBL_RDG_RAW,           NO_SHIFT_DEFINE,	false},
	{"SPE_IDX",     WTBL_SPE_IDX_MASK,  WTBL_SPE_IDX_OFFSET,	false},
	{"G2",          WTBL_G2,            NO_SHIFT_DEFINE,	false},
	{"G4",          WTBL_G4,            NO_SHIFT_DEFINE,	false},
	{"G8",          WTBL_G8,            NO_SHIFT_DEFINE,	false},
	{"G16",         WTBL_G16,           NO_SHIFT_DEFINE,	true},
	{"G2_LTF",      WTBL_G2_LTF_MASK,   WTBL_G2_LTF_OFFSET,	false},
	{"G4_LTF",      WTBL_G4_LTF_MASK,   WTBL_G4_LTF_OFFSET,	false},
	{"G8_LTF",      WTBL_G8_LTF_MASK,   WTBL_G8_LTF_OFFSET,	false},
	{"G16_LTF",     WTBL_G16_LTF_MASK,  WTBL_G16_LTF_OFFSET,	true},
	{"G2_HE",       WTBL_G2_HE_MASK,    WTBL_G2_HE_OFFSET,	false},
	{"G4_HE",       WTBL_G4_HE_MASK,    WTBL_G4_HE_OFFSET,	false},
	{"G8_HE",       WTBL_G8_HE_MASK,    WTBL_G8_HE_OFFSET,	false},
	{"G16_HE",      WTBL_G16_HE_MASK,   WTBL_G16_HE_OFFSET,	true},
	{NULL,}
};

static void
parse_fmac_lwtbl_DW7(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 7 */
	seq_puts(s, "\nLWTBL DW 7\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_7*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW7[i].name) {
		if (WTBL_LMAC_DW7[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW7[i].name,
					 (dw_value & WTBL_LMAC_DW7[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW7[i].name,
					  (dw_value & WTBL_LMAC_DW7[i].mask) >> WTBL_LMAC_DW7[i].shift);
		if (WTBL_LMAC_DW7[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW8[] = {
	{"FAIL_CNT_AC0",    WTBL_FAIL_CNT_AC0_MASK, WTBL_FAIL_CNT_AC0_OFFSET,	false},
	{"AC1",    WTBL_FAIL_CNT_AC1_MASK, WTBL_FAIL_CNT_AC1_OFFSET,	false},
	{"AC2",    WTBL_FAIL_CNT_AC2_MASK, WTBL_FAIL_CNT_AC2_OFFSET,	false},
	{"AC3",    WTBL_FAIL_CNT_AC3_MASK, WTBL_FAIL_CNT_AC3_OFFSET,	true},
	{"PARTIAL_AID",     WTBL_PARTIAL_AID_MASK,  WTBL_PARTIAL_AID_OFFSET,	false},
	{"CHK_PER",         WTBL_CHK_PER,           NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void
parse_fmac_lwtbl_DW8(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 8 */
	seq_puts(s, "\nLWTBL DW 8\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW8[i].name) {
		if (WTBL_LMAC_DW8[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW8[i].name,
					 (dw_value & WTBL_LMAC_DW8[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW8[i].name,
					  (dw_value & WTBL_LMAC_DW8[i].mask) >> WTBL_LMAC_DW8[i].shift);
		if (WTBL_LMAC_DW8[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW9[] = {
	{"RX_AVG_MPDU",     WTBL_RX_AVG_MPDU_SIZE_MASK, WTBL_RX_AVG_MPDU_SIZE_OFFSET,	false},
	{"PRITX_SW_MODE",   WTBL_PRITX_SW_MODE,         NO_SHIFT_DEFINE,	false},
	{"PRITX_PLR",       WTBL_PRITX_PLR,             NO_SHIFT_DEFINE,	true},
	{"PRITX_DCM",       WTBL_PRITX_DCM,             NO_SHIFT_DEFINE,	false},
	{"PRITX_ER160",     WTBL_PRITX_ER160,           NO_SHIFT_DEFINE,	false},
	{"PRITX_ERSU",      WTBL_PRITX_ERSU,            NO_SHIFT_DEFINE,	true},
/*     {"FCAP(0:20 1:~40)",    WTBL_FCAP_20_TO_160_MHZ,    WTBL_FCAP_20_TO_160_MHZ_OFFSET}, */
	{"MPDU_FAIL_CNT",   WTBL_MPDU_FAIL_CNT_MASK,    WTBL_MPDU_FAIL_CNT_OFFSET,	false},
	{"MPDU_OK_CNT",     WTBL_MPDU_OK_CNT_MASK,      WTBL_MPDU_OK_CNT_OFFSET,	false},
	{"RATE_IDX",        WTBL_RATE_IDX_MASK,         WTBL_RATE_IDX_OFFSET,	true},
	{NULL,}
};

char *fcap_name[] = {"20MHz", "20/40MHz", "20/40/80MHz", "20/40/80/160/80+80MHz"};

static void
parse_fmac_lwtbl_DW9(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 8 */
	seq_puts(s, "\nLWTBL DW 9\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_9*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW9[i].name) {
		if (WTBL_LMAC_DW9[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW9[i].name,
					 (dw_value & WTBL_LMAC_DW9[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW9[i].name,
					  (dw_value & WTBL_LMAC_DW9[i].mask) >> WTBL_LMAC_DW9[i].shift);
		if (WTBL_LMAC_DW9[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}

	/* FCAP parser */
	seq_printf(s, "FCAP:%s\n", fcap_name[(dw_value & WTBL_FCAP_20_TO_160_MHZ) >> WTBL_FCAP_20_TO_160_MHZ_OFFSET]);
}

#define HW_TX_RATE_TO_MODE(_x)			(((_x) & GENMASK(9, 6)) >> 6)
#define HW_TX_RATE_TO_MCS(_x, _mode)		((_x) & (0x3f))
#define HW_TX_RATE_TO_NSS(_x)			(((_x) & GENMASK(12, 10)) >> 10)
#define HW_TX_RATE_TO_STBC(_x)			(((_x) & BIT(13)) >> 13)

#define MAX_TX_MODE 12
static char *HW_TX_MODE_STR[] = {"CCK", "OFDM", "HT-Mix", "HT-GF", "VHT", "N/A", "N/A", "N/A",
							"HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU", "N/A"};
static char *HW_TX_RATE_CCK_STR[] = {"1M", "2Mlong", "5.5Mlong", "11Mlong", "N/A", "2Mshort", "5.5Mshort", "11Mshort", "N/A"};
static char *HW_TX_RATE_OFDM_STR[] = {"6M", "9M", "12M", "18M", "24M", "36M", "48M", "54M", "N/A"};

static char *hw_rate_ofdm_str(u16 ofdm_idx)
{
	switch (ofdm_idx) {
	case 11: /* 6M */
		return HW_TX_RATE_OFDM_STR[0];

	case 15: /* 9M */
		return HW_TX_RATE_OFDM_STR[1];

	case 10: /* 12M */
		return HW_TX_RATE_OFDM_STR[2];

	case 14: /* 18M */
		return HW_TX_RATE_OFDM_STR[3];

	case 9: /* 24M */
		return HW_TX_RATE_OFDM_STR[4];

	case 13: /* 36M */
		return HW_TX_RATE_OFDM_STR[5];

	case 8: /* 48M */
		return HW_TX_RATE_OFDM_STR[6];

	case 12: /* 54M */
		return HW_TX_RATE_OFDM_STR[7];

	default:
		return HW_TX_RATE_OFDM_STR[8];
	}
}

static char *hw_rate_str(u8 mode, u16 rate_idx)
{
	if (mode == 0)
		return rate_idx < 8 ? HW_TX_RATE_CCK_STR[rate_idx] : HW_TX_RATE_CCK_STR[8];
	else if (mode == 1)
		return hw_rate_ofdm_str(rate_idx);
	else
		return "MCS";
}

static void parse_rate(struct seq_file *s, u16 rate_idx, u16 txrate)
{
	u16 txmode, mcs, nss, stbc;

	txmode = HW_TX_RATE_TO_MODE(txrate);
	mcs = HW_TX_RATE_TO_MCS(txrate, txmode);
	nss = HW_TX_RATE_TO_NSS(txrate);
	stbc = HW_TX_RATE_TO_STBC(txrate);

	seq_printf(s, "\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
			  rate_idx + 1, txrate,
			  txmode, (txmode < MAX_TX_MODE ? HW_TX_MODE_STR[txmode] : HW_TX_MODE_STR[MAX_TX_MODE]),
			  mcs, hw_rate_str(txmode, mcs), nss, stbc);
}


static struct {
	char *name;
	u32 mask;
	u32 shift;
} WTBL_LMAC_DW10[] = {
	{"RATE1",       WTBL_RATE1_MASK,        WTBL_RATE1_OFFSET},
	{"RATE2",       WTBL_RATE2_MASK,        WTBL_RATE2_OFFSET},
	{NULL,}
};

static void parse_fmac_lwtbl_DW10(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 10 */
	seq_puts(s, "\nLWTBL DW 10\n");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_1_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW10[i].name) {
		parse_rate(s, i, (dw_value & WTBL_LMAC_DW10[i].mask) >> WTBL_LMAC_DW10[i].shift);
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
} WTBL_LMAC_DW11[] = {
	{"RATE3",       WTBL_RATE3_MASK,        WTBL_RATE3_OFFSET},
	{"RATE4",       WTBL_RATE4_MASK,        WTBL_RATE4_OFFSET},
	{NULL,}
};

static void parse_fmac_lwtbl_DW11(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 11 */
	seq_puts(s, "\nLWTBL DW 11\n");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_3_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW11[i].name) {
		parse_rate(s, i+2, (dw_value & WTBL_LMAC_DW11[i].mask) >> WTBL_LMAC_DW11[i].shift);
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
} WTBL_LMAC_DW12[] = {
	{"RATE5",       WTBL_RATE5_MASK,        WTBL_RATE5_OFFSET},
	{"RATE6",       WTBL_RATE6_MASK,        WTBL_RATE6_OFFSET},
	{NULL,}
};

static void parse_fmac_lwtbl_DW12(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 12 */
	seq_puts(s, "\nLWTBL DW 12\n");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_5_6*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW12[i].name) {
		parse_rate(s, i+4, (dw_value & WTBL_LMAC_DW12[i].mask) >> WTBL_LMAC_DW12[i].shift);
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
} WTBL_LMAC_DW13[] = {
	{"RATE7",       WTBL_RATE7_MASK,        WTBL_RATE7_OFFSET},
	{"RATE8",       WTBL_RATE8_MASK,        WTBL_RATE8_OFFSET},
	{NULL,}
};

static void parse_fmac_lwtbl_DW13(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 13 */
	seq_puts(s, "\nLWTBL DW 13\n");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_AUTO_RATE_7_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW13[i].name) {
		parse_rate(s, i+6, (dw_value & WTBL_LMAC_DW13[i].mask) >> WTBL_LMAC_DW13[i].shift);
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW3_E1[] = {
	{"WMM_Q",           WTBL_WMM_Q_MASK,        WTBL_WMM_Q_OFFSET,	false},
	{"RXD_DUP_MODE",    WTBL_RXD_DUP_MODE_MASK, WTBL_RXD_DUP_MODE_OFFSET, true},
	{"VLAN2ETH",        WTBL_VLAN2ETH,          NO_SHIFT_DEFINE,	false},
	{"PFMU_IDX",        WTBL_PFMU_IDX_MASK,     WTBL_PFMU_IDX_OFFSET,	false},
	{"RIBF",            WTBL_RIBF,              NO_SHIFT_DEFINE,	true},
	{"TBF",             WTBL_TBF,               NO_SHIFT_DEFINE,	false},
	{"TBF_VHT",         WTBL_TBF_VHT,           NO_SHIFT_DEFINE,	false},
	{"TBF_HE",          WTBL_TBF_HE,            NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW3_E1(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 3 */
	seq_puts(s, "\nLWTBL DW 3\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_3*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW3_E1[i].name) {

		if (WTBL_LMAC_DW3_E1[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW3_E1[i].name,
					 (dw_value & WTBL_LMAC_DW3_E1[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW3_E1[i].name,
					  (dw_value & WTBL_LMAC_DW3_E1[i].mask) >> WTBL_LMAC_DW3_E1[i].shift);
		if (WTBL_LMAC_DW3_E1[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW5_E1[] = {
	{"AF",                  WTBL_AF_MASK,               WTBL_AF_OFFSET,		false},
	{"AF_HE",               WTBL_AF_HE_MASK,            WTBL_AF_HE_OFFSET,	false},
	{"RTS",                 WTBL_RTS,                   NO_SHIFT_DEFINE,	false},
	{"SMPS",                WTBL_SMPS_RAW,                  NO_SHIFT_DEFINE,	false},
	{"DYN_BW",              WTBL_DYN_BW,                NO_SHIFT_DEFINE,	false},
	{"MMSS",                WTBL_MMSS_MASK,             WTBL_MMSS_OFFSET,	true},
	{"USR",                 WTBL_USR,                   NO_SHIFT_DEFINE,	false},
	{"SR_RATE",             WTBL_SR_RATE_E1_MASK,       WTBL_SR_RATE_E1_OFFSET,	false},
	{"BEAM_CHG",            WTBL_BEAM_CHG_E1,           NO_SHIFT_DEFINE,	false},
	{"SR_ABORT",            WTBL_SR_ABORT,              NO_SHIFT_DEFINE,	true},
	{"TX_POWER_OFFSET",     WTBL_TX_POWER_OFFSET_MASK,  WTBL_TX_POWER_OFFSET_OFFSET,	false},
	{"WTBL_MPDU_SIZE",      WTBL_MPDU_SIZE_MASK,        WTBL_MPDU_SIZE_OFFSET,	false},
	{"PE",                  WTBL_PE_MASK,               WTBL_PE_OFFSET,		true},
	{"DOPPL",               WTBL_DOPPL,                 NO_SHIFT_DEFINE,	false},
	{"TXOP_PS_CAP",         WTBL_TXOP_PS_CAP,           NO_SHIFT_DEFINE,	false},
	{"DONOT_UPDATE_I_PSM",  WTBL_DONOT_UPDATE_I_PSM,    NO_SHIFT_DEFINE,	true},
	{"I_PSM",               WTBL_I_PSM,                 NO_SHIFT_DEFINE,	false},
	{"PSM",                 WTBL_PSM,                   NO_SHIFT_DEFINE,	false},
	{"SKIP_TX",             WTBL_SKIP_TX,               NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW5_E1(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 5 */
	seq_puts(s, "\nLWTBL DW 5\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_5*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW5_E1[i].name) {
		if (WTBL_LMAC_DW5_E1[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW5_E1[i].name,
					 (dw_value & WTBL_LMAC_DW5_E1[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW5_E1[i].name,
					  (dw_value & WTBL_LMAC_DW5_E1[i].mask) >> WTBL_LMAC_DW5_E1[i].shift);
		if (WTBL_LMAC_DW5_E1[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW28_E1[] = {
	{"USER_RSSI",                   WTBL_USER_RSSI_MASK,            WTBL_USER_RSSI_OFFSET,	false},
	{"USER_SNR",                    WTBL_USER_SNR_MASK,             WTBL_USER_SNR_OFFSET,	false},
	{"RAPID_REACTION_RATE",         WTBL_RAPID_REACTION_RATE_MASK,  WTBL_RAPID_REACTION_RATE_OFFSET,	true},
	{"HT_AMSDU(Read Only)",         WTBL_HT_AMSDU,                  NO_SHIFT_DEFINE,	false},
	{"AMSDU_CROSS_LG(Read Only)",   WTBL_AMSDU_CROSS_LG,            NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW28_E1(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 28 */
	seq_puts(s, "\nLWTBL DW 28\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_1*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW28_E1[i].name) {
		if (WTBL_LMAC_DW28_E1[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW28_E1[i].name,
					 (dw_value & WTBL_LMAC_DW28_E1[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW28_E1[i].name,
					  (dw_value & WTBL_LMAC_DW28_E1[i].mask) >> WTBL_LMAC_DW28_E1[i].shift);

		if (WTBL_LMAC_DW28_E1[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW29_E1[] = {
	{"RCPI 0",	WTBL_RESP_RCPI0_MASK,		WTBL_RESP_RCPI0_OFFSET,	false},
	{"RCPI 1",	WTBL_RESP_RCPI1_MASK,		WTBL_RESP_RCPI1_OFFSET,	false},
	{"RCPI 2",	WTBL_RESP_RCPI2_MASK,		WTBL_RESP_RCPI2_OFFSET,	false},
	{"RCPI 3",	WTBL_RESP_RCPI3_MASK,		WTBL_RESP_RCPI3_OFFSET,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW29_E1(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 29 */
	seq_puts(s, "\nLWTBL DW 29\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_2*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW29_E1[i].name) {
		if (WTBL_LMAC_DW29_E1[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW29_E1[i].name,
					 (dw_value & WTBL_LMAC_DW29_E1[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW29_E1[i].name,
					  (dw_value & WTBL_LMAC_DW29_E1[i].mask) >> WTBL_LMAC_DW29_E1[i].shift);

		if (WTBL_LMAC_DW29_E1[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW30_E1[] = {
	{"SNR 0",	WTBL_SNR_RX0_MASK,		WTBL_SNR_RX0_OFFSET,	false},
	{"SNR 1",	WTBL_SNR_RX1_MASK,		WTBL_SNR_RX1_OFFSET,	false},
	{"SNR 2",	WTBL_SNR_RX2_MASK,		WTBL_SNR_RX2_OFFSET,	false},
	{"SNR 3",	WTBL_SNR_RX3_MASK,		WTBL_SNR_RX3_OFFSET,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW30_E1(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 30 */
	seq_puts(s, "\nLWTBL DW 30\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_3*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW30_E1[i].name) {
		if (WTBL_LMAC_DW30_E1[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW30_E1[i].name,
					 (dw_value & WTBL_LMAC_DW30_E1[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW30_E1[i].name,
					  (dw_value & WTBL_LMAC_DW30_E1[i].mask) >> WTBL_LMAC_DW30_E1[i].shift);
		if (WTBL_LMAC_DW30_E1[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW28[] = {
	{"OM_INFO",                     WTBL_OM_INFO_MASK,              WTBL_OM_INFO_OFFSET,	false},
	{"OM_RXD_DUP_MODE",             WTBL_OM_RXD_DUP_MODE,           NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW28(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 28 */
	seq_puts(s, "\nLWTBL DW 28\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_1*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW28[i].name) {
		if (WTBL_LMAC_DW28[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW28[i].name,
					 (dw_value & WTBL_LMAC_DW28[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW28[i].name,
					  (dw_value & WTBL_LMAC_DW28[i].mask) >> WTBL_LMAC_DW28[i].shift);

		if (WTBL_LMAC_DW28[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW29[] = {
	{"USER_RSSI",                   WTBL_USER_RSSI_MASK,            WTBL_USER_RSSI_OFFSET,	false},
	{"USER_SNR",                    WTBL_USER_SNR_MASK,             WTBL_USER_SNR_OFFSET,	false},
	{"RAPID_REACTION_RATE",         WTBL_RAPID_REACTION_RATE_MASK,  WTBL_RAPID_REACTION_RATE_OFFSET,	true},
	{"HT_AMSDU(Read Only)",         WTBL_HT_AMSDU,                  NO_SHIFT_DEFINE,	false},
	{"AMSDU_CROSS_LG(Read Only)",   WTBL_AMSDU_CROSS_LG,            NO_SHIFT_DEFINE,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW29(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 29 */
	seq_puts(s, "\nLWTBL DW 29\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW29[i].name) {
		if (WTBL_LMAC_DW29[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW29[i].name,
					 (dw_value & WTBL_LMAC_DW29[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW29[i].name,
					  (dw_value & WTBL_LMAC_DW29[i].mask) >> WTBL_LMAC_DW29[i].shift);

		if (WTBL_LMAC_DW29[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_LMAC_DW30[] = {
	{"RCPI 0",	WTBL_RESP_RCPI0_MASK,		WTBL_RESP_RCPI0_OFFSET,	false},
	{"RCPI 1",	WTBL_RESP_RCPI1_MASK,		WTBL_RESP_RCPI1_OFFSET,	false},
	{"RCPI 2",	WTBL_RESP_RCPI2_MASK,		WTBL_RESP_RCPI2_OFFSET,	false},
	{"RCPI 3",	WTBL_RESP_RCPI3_MASK,		WTBL_RESP_RCPI3_OFFSET,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW30(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 30 */
	seq_puts(s, "\nLWTBL DW 30\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_3*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW30[i].name) {
		if (WTBL_LMAC_DW30[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW30[i].name,
					 (dw_value & WTBL_LMAC_DW30[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW30[i].name,
					  (dw_value & WTBL_LMAC_DW30[i].mask) >> WTBL_LMAC_DW30[i].shift);

		if (WTBL_LMAC_DW30[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	char new_line;
} WTBL_LMAC_DW31[] = {
	{"RCPI 4",	WTBL_RESP_RCPI4_MASK,		WTBL_RESP_RCPI4_OFFSET,	false},
	{"RCPI 5",	WTBL_RESP_RCPI5_MASK,		WTBL_RESP_RCPI5_OFFSET,	false},
	{"RCPI 6",	WTBL_RESP_RCPI6_MASK,		WTBL_RESP_RCPI6_OFFSET,	false},
	{"RCPI 7",	WTBL_RESP_RCPI4_MASK,		WTBL_RESP_RCPI7_OFFSET,	true},
	{NULL,}
};

static void parse_fmac_lwtbl_DW31(struct seq_file *s, u8 *lwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* LMAC WTBL DW 31 */
	seq_puts(s, "\nLWTBL DW 31\n\t");
	addr = (u32 *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_4*4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW31[i].name) {
		if (WTBL_LMAC_DW31[i].shift == NO_SHIFT_DEFINE)
			seq_printf(s, "%s:%d", WTBL_LMAC_DW31[i].name,
					 (dw_value & WTBL_LMAC_DW31[i].mask) ? 1 : 0);
		else
			seq_printf(s, "%s:%u", WTBL_LMAC_DW31[i].name,
					  (dw_value & WTBL_LMAC_DW31[i].mask) >> WTBL_LMAC_DW31[i].shift);
		if (WTBL_LMAC_DW31[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static void parse_fmac_lwtbl_rx_stats(struct seq_file *s, u8 *lwtbl)
{
	if (0) {
		parse_fmac_lwtbl_DW28_E1(s, lwtbl);
		parse_fmac_lwtbl_DW29_E1(s, lwtbl);
		parse_fmac_lwtbl_DW30_E1(s, lwtbl);
	} else {
		parse_fmac_lwtbl_DW28(s, lwtbl);
		parse_fmac_lwtbl_DW29(s, lwtbl);
		parse_fmac_lwtbl_DW30(s, lwtbl);
		parse_fmac_lwtbl_DW31(s, lwtbl);
	}
}

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_UMAC_DW0[] = {
	{"PN0",		WTBL_PN0_MASK,		WTBL_PN0_OFFSET,	false},
	{"PN1",		WTBL_PN1_MASK,		WTBL_PN1_OFFSET,	false},
	{"PN2",		WTBL_PN2_MASK,		WTBL_PN2_OFFSET,	true},
	{"PN3",		WTBL_PN3_MASK,		WTBL_PN3_OFFSET,	false},
	{NULL,}
};

static struct {
	char *name;
	u32 mask;
	u32 shift;
	u8 new_line;
} WTBL_UMAC_DW1[] = {
	{"PN4",     WTBL_PN4_MASK,      WTBL_PN4_OFFSET,	false},
	{"PN5",     WTBL_PN5_MASK,      WTBL_PN5_OFFSET,	true},
	{NULL,}
};

static void parse_fmac_uwtbl_pn(struct seq_file *s, u8 *uwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u16 i = 0;

	/* UMAC WTBL DW 0 */
	seq_puts(s, "\nUWTBL PN\n\t");

	addr = (u32 *)&(uwtbl[UWTBL_PN_DW_0*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW0[i].name) {
		seq_printf(s, "%s:%u", WTBL_UMAC_DW0[i].name,
				  (dw_value & WTBL_UMAC_DW0[i].mask) >> WTBL_UMAC_DW0[i].shift);
		if (WTBL_UMAC_DW0[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}

	i = 0;
	addr = (u32 *)&(uwtbl[UWTBL_PN_SN_DW_1*4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW1[i].name) {
		seq_printf(s, "%s:%u", WTBL_UMAC_DW1[i].name,
				  (dw_value & WTBL_UMAC_DW1[i].mask) >> WTBL_UMAC_DW1[i].shift);
		if (WTBL_UMAC_DW1[i].new_line)
			seq_puts(s, "\n\t");
		else
			seq_puts(s, "/ ");
		i++;
	}
}

static void parse_fmac_uwtbl_sn(struct seq_file *s, u8 *uwtbl)
{
	u32 *addr = 0;
	u32 u2SN = 0;

	/* UMAC WTBL DW SN part */
	seq_puts(s, "\nUWTBL SN\n");

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID0_AC0_SN_MASK) >> WTBL_TID0_AC0_SN_OFFSET;
	seq_printf(s, "\t%s:%u\n", "TID0_AC0_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID1_AC1_SN_MASK) >> WTBL_TID1_AC1_SN_OFFSET;
	seq_printf(s, "\t%s:%u\n", "TID1_AC1_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_2*4]);
	u2SN = ((*addr) & WTBL_TID2_AC2_SN_0_7_MASK) >> WTBL_TID2_AC2_SN_0_7_OFFSET;
	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN |= (((*addr) & WTBL_TID2_AC2_SN_8_11_MASK) >> WTBL_TID2_AC2_SN_8_11_OFFSET) << 8;
	seq_printf(s, "\t%s:%u\n", "TID2_AC2_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID3_AC3_SN_MASK) >> WTBL_TID3_AC3_SN_OFFSET;
	seq_printf(s, "\t%s:%u\n", "TID3_AC3_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID4_SN_MASK) >> WTBL_TID4_SN_OFFSET;
	seq_printf(s, "\t%s:%u\n", "TID4_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_3*4]);
	u2SN = ((*addr) & WTBL_TID5_SN_0_3_MASK) >> WTBL_TID5_SN_0_3_OFFSET;
	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN |= (((*addr) & WTBL_TID5_SN_4_11_MASK) >> WTBL_TID5_SN_4_11_OFFSET) << 4;
	seq_printf(s, "\t%s:%u\n", "TID5_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN = ((*addr) & WTBL_TID6_SN_MASK) >> WTBL_TID6_SN_OFFSET;
	seq_printf(s, "\t%s:%u\n", "TID6_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_SN_DW_4*4]);
	u2SN = ((*addr) & WTBL_TID7_SN_MASK) >> WTBL_TID7_SN_OFFSET;
	seq_printf(s, "\t%s:%u\n", "TID6_SN", u2SN);

	addr = (u32 *)&(uwtbl[UWTBL_PN_SN_DW_1*4]);
	u2SN = ((*addr) & WTBL_COM_SN_MASK) >> WTBL_COM_SN_OFFSET;
	seq_printf(s, "\t%s:%u\n", "COM_SN", u2SN);
}

static void dump_key_table(struct seq_file *s, u16 keyloc0, u16 keyloc1)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;
	u8 keytbl[ONE_KEY_ENTRY_LEN_IN_DW*4] = {0};
	u16 x;

	seq_printf(s, "\n\t%s:%d\n", "keyloc0", keyloc0);
	if (keyloc0 != (WTBL_KEY_LINK_DW_KEY_LOC0_MASK >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET)) {

		/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
		halWtblReadRaw(trans, keyloc0, WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		seq_printf(s, "KEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
									WF_UWTBL_TOP_WDUCR_ADDR,
									bus_read(trans, WF_UWTBL_TOP_WDUCR_ADDR),
									KEYTBL_IDX2BASE(keyloc0, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			seq_printf(s, "DW%02d: %02x %02x %02x %02x\n",
										x,
										keytbl[x * 4 + 3],
										keytbl[x * 4 + 2],
										keytbl[x * 4 + 1],
										keytbl[x * 4]);
		}
	}

	seq_printf(s, "\t%s:%d\n", "keyloc1", keyloc1);
	if (keyloc1 != (WTBL_KEY_LINK_DW_KEY_LOC1_MASK >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET)) {
		/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
		halWtblReadRaw(trans, keyloc1, WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		seq_printf(s, "KEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
									WF_UWTBL_TOP_WDUCR_ADDR,
									bus_read(trans, WF_UWTBL_TOP_WDUCR_ADDR),
									KEYTBL_IDX2BASE(keyloc1, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			seq_printf(s, "DW%02d: %02x %02x %02x %02x\n",
										x,
										keytbl[x * 4 + 3],
										keytbl[x * 4 + 2],
										keytbl[x * 4 + 1],
										keytbl[x * 4]);
		}
	}
}

static void parse_fmac_uwtbl_others(struct seq_file *s, u8 *uwtbl)
{
	u32 *addr = 0;
	u32 dw_value = 0;
	u32 amsdu_len = 0;

	/* UMAC WTBL DW 0 */
	seq_puts(s, "\nUWTBL others\n");

	addr = (u32 *)&(uwtbl[UWTBL_KEY_LINK_DW*4]);
	dw_value = *addr;

	seq_printf(s, "\t%s:%lu/%lu\n", "Key Loc 1/2",
					(dw_value & WTBL_KEY_LINK_DW_KEY_LOC0_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET,
					(dw_value & WTBL_KEY_LINK_DW_KEY_LOC1_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET
					);

	seq_printf(s, "\t%s:%d\n", "UWTBL_QOS",
					(dw_value & WTBL_QOS_MASK) ? 1 : 0
					);

	seq_printf(s, "\t%s:%d\n", "UWTBL_HT_VHT_HE",
					(dw_value & WTBL_HT_VHT_HE_MASK) ? 1 : 0
					);

	addr = (u32 *)&(uwtbl[UWTBL_HW_AMSDU_DW*4]);
	dw_value = *addr;

	seq_printf(s, "\t%s:%d\n", "HW AMSDU Enable",
					(dw_value & WTBL_AMSDU_EN_MASK) ? 1 : 0
					);

	amsdu_len = (dw_value & WTBL_AMSDU_LEN_MASK) >> WTBL_AMSDU_LEN_OFFSET;
	if (amsdu_len == 0)
		seq_printf(s, "\t%s:invalid (WTBL value=0x%x)\n", "HW AMSDU Len",
						amsdu_len
						);
	else if (amsdu_len == 1)
		seq_printf(s, "\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
						1,
						255,
						amsdu_len
						);
	else
		seq_printf(s, "\t%s:%d~%d (WTBL value=0x%x)\n", "HW AMSDU Len",
						256 * (amsdu_len - 1),
						256 * (amsdu_len - 1) + 255,
						amsdu_len
						);

	seq_printf(s, "\t%s:%lu (WTBL value=0x%lx)\n", "HW AMSDU Num",
					((dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET) + 1,
					(dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET
					);

	/* Parse KEY link */
	addr = (u32 *)&(uwtbl[UWTBL_KEY_LINK_DW*4]);
	dw_value = *addr;
	dump_key_table(s,
			(dw_value & WTBL_KEY_LINK_DW_KEY_LOC0_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET,
			(dw_value & WTBL_KEY_LINK_DW_KEY_LOC1_MASK) >> WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET
			);
}


static void
dump_fmac_wtbl_info(struct seq_file *s, u8 *lwtbl, u8 *uwtbl)
{
	parse_fmac_lwtbl_DW0_1(s, lwtbl);
	parse_fmac_lwtbl_DW2(s, lwtbl);
	if (0)
		parse_fmac_lwtbl_DW3_E1(s, lwtbl);
	else
		parse_fmac_lwtbl_DW3(s, lwtbl);
	parse_fmac_lwtbl_DW4(s, lwtbl);
	if (0)
		parse_fmac_lwtbl_DW5_E1(s, lwtbl);
	else
		parse_fmac_lwtbl_DW5(s, lwtbl);
	parse_fmac_lwtbl_DW6(s, lwtbl);
	parse_fmac_lwtbl_DW7(s, lwtbl);
	parse_fmac_lwtbl_DW8(s, lwtbl);
	parse_fmac_lwtbl_DW9(s, lwtbl);
	parse_fmac_lwtbl_DW10(s, lwtbl);
	parse_fmac_lwtbl_DW11(s, lwtbl);
	parse_fmac_lwtbl_DW12(s, lwtbl);
	parse_fmac_lwtbl_DW13(s, lwtbl);
	parse_fmac_lwtbl_rx_stats(s, lwtbl);

	parse_fmac_uwtbl_pn(s, uwtbl);
	parse_fmac_uwtbl_sn(s, uwtbl);
	parse_fmac_uwtbl_others(s, uwtbl);
}

static int
mt7992_dump_wtbl_info(struct seq_file *s, void *data)
{
#define LWTBL_LEN_IN_DW 32
#define UWTBL_LEN_IN_DW 8
#define LWTBL_LEN_IN_DW_E1 31
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;
	u8 lwtbl[LWTBL_LEN_IN_DW*4] = {0};
	u8 uwtbl[UWTBL_LEN_IN_DW*4] = {0};
	u8 real_lwtbl_size = 0;
	int x;
	u16 wtbl_idx = 0;

	if (0)
		real_lwtbl_size = LWTBL_LEN_IN_DW_E1;
	else
		real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(trans, wtbl_idx, WTBL_TYPE_LMAC, 0, real_lwtbl_size, lwtbl);
	seq_printf(s, "Dump WTBL info of WLAN_IDX:%d\n", wtbl_idx);
	seq_printf(s, "LMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_WTBLON_TOP_WDUCR_ADDR,
								bus_read(trans, WF_WTBLON_TOP_WDUCR_ADDR),
								LWTBL_IDX2BASE(wtbl_idx, 0));
	for (x = 0; x < real_lwtbl_size; x++) {
		seq_printf(s, "DW%02d: %02x %02x %02x %02x\n",
									x,
									lwtbl[x * 4 + 3],
									lwtbl[x * 4 + 2],
									lwtbl[x * 4 + 1],
									lwtbl[x * 4]);
	}

	/* Don't swap below two lines, halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR */
	halWtblReadRaw(trans, wtbl_idx, WTBL_TYPE_UMAC, 0, UWTBL_LEN_IN_DW, uwtbl);
	seq_printf(s, "UMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
								WF_UWTBL_TOP_WDUCR_ADDR,
								bus_read(trans, WF_UWTBL_TOP_WDUCR_ADDR),
								UWTBL_IDX2BASE(wtbl_idx, 0));
	for (x = 0; x < UWTBL_LEN_IN_DW; x++) {
		seq_printf(s, "DW%02d: %02x %02x %02x %02x\n",
									x,
									uwtbl[x * 4 + 3],
									uwtbl[x * 4 + 2],
									uwtbl[x * 4 + 1],
									uwtbl[x * 4]);
	}

	dump_fmac_wtbl_info(s, lwtbl, uwtbl);
	return 0;
}

static int
mtk_eeprom_debugfs(struct seq_file *s, void *data)
{
#ifdef HW_DBG
	void *trans = dev_get_drvdata(s->private);
	struct mtk_hw_dev *dev = to_hw_dev(trans);
	u8 *buf = dev->hw_ctrl.eeprom.data;
	u16 idx_round = round_down(dev->hw_ctrl.eeprom.size, 16);
	int i, j;

	for (j = 0 ; j < idx_round; j += 16) {
		seq_printf(s, "%08x  ", j);
		for (i = 0; i < 16; i++) {
			if (i < 15)
				seq_printf(s, "%02x ", buf[j + i]);
			else
				seq_printf(s, "%02x\n", buf[j + i]);
		}
	}
#endif
	return 0;
}

static int
mt7992_dbg_rro(struct seq_file *s, void *data)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;
	u32 reg, reg2;
	u16 i;

	seq_puts(s, "RRO debug Info:\n");

	for (i = 0x0; i <= 0x2f; i++) {
		reg2 = 0x100 | i;
		bus_write(trans, 0xa124, reg2);
		reg = bus_read(trans, 0xa128);
		seq_printf(s, "write 0x%3x to cr 0xa124, 0xa128 = %x\n", reg2, reg);
	}
	reg = bus_read(trans, WF_RRO_TOP_RRO_BUSY_STATUS_ADDR);
	seq_printf(s, "rro busy int status = %x\n", reg);

	for (reg = 0xda000;  reg < 0xda300; reg += 0x20) {
		seq_printf(s, "0x%x: %8x %8x %8x %8x %8x %8x %8x %8x\n", reg,
			bus_read(trans, reg), bus_read(trans, reg + 4),
			bus_read(trans, reg + 8), bus_read(trans, reg + 12),
			bus_read(trans, reg + 16), bus_read(trans, reg + 20),
			bus_read(trans, reg + 24), bus_read(trans, reg + 28));
	}

	for (reg = 0xda600;  reg < 0xda800; reg += 0x20) {
		seq_printf(s, "0x%x: %8x %8x %8x %8x %8x %8x %8x %8x\n", reg,
			bus_read(trans, reg), bus_read(trans, reg + 4),
			bus_read(trans, reg + 8), bus_read(trans, reg + 12),
			bus_read(trans, reg + 16), bus_read(trans, reg + 20),
			bus_read(trans, reg + 24), bus_read(trans, reg + 28));
	}

	for (i = 0x0; i <= 0xff; i++) {
		reg2 = 0x80060000 | i;
		bus_write(trans, 0xa0e0, reg2);
		reg = bus_read(trans, 0xa0f0);
		if (reg != i+1)
			seq_printf(s, "write 0x%3x to cr 0xa0e0, 0xa0f0 = %x, a0f4 = %x, a0f8 = %x, a0fc = %x\n",
				reg2, reg,
				bus_read(trans, 0xa0f4),
				bus_read(trans, 0xa0f8),
				bus_read(trans, 0xa0fc));
	}

	for (i = 0x0; i <= 0xfff; i++) {
		reg2 = 0x80020000 | i;
		bus_write(trans, 0xa0e0, reg2);
		reg = bus_read(trans, 0xa0f0);
		if (reg != i+1)
			seq_printf(s, "write 0x%3x to cr 0xa0e0, 0xa0f0 = %x\n", reg2, reg);
	}

	for (i = 0x0; i <= 0x13ff; i++) {
		reg2 = 0x80030000 | i;
		bus_write(trans, 0xa0e0, reg2);
		reg = bus_read(trans, 0xa0f0);
		seq_printf(s, "write 0x%3x to cr 0xa0e0, 0xa0f0 = %x\n", reg2, reg);
	}

	for (i = 0x0; i <= 0x3ff; i++) {
		reg2 = 0x80040000 | i;
		bus_write(trans, 0xa0e0, reg2);
		reg = bus_read(trans, 0xa0f0);
		seq_printf(s, "write 0x%3x to cr 0xa0e0, 0xa0f0 = %x, 0xa0f4 = %x\n",
			reg2, reg, bus_read(trans, 0xa0f4));
	}

	return 0;
}


static int
mt7992_show_rro_mib(struct seq_file *s, void *data)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *trans = dev->bus_trans;
	u32 reg[12];

	seq_puts(s, "RRO mib Info:\n");

	reg[0] = bus_read(trans, WF_RRO_TOP_STATISTIC_0_ADDR);
	reg[1] = bus_read(trans, WF_RRO_TOP_STATISTIC_1_ADDR);
	reg[2] = bus_read(trans, WF_RRO_TOP_STATISTIC_2_ADDR);
	reg[3] = bus_read(trans, WF_RRO_TOP_STATISTIC_3_ADDR);
	reg[4] = bus_read(trans, WF_RRO_TOP_STATISTIC_4_ADDR);
	reg[5] = bus_read(trans, WF_RRO_TOP_STATISTIC_5_ADDR);
	reg[6] = bus_read(trans, WF_RRO_TOP_STATISTIC_6_ADDR);
	reg[7] = bus_read(trans, WF_RRO_TOP_STATISTIC_7_ADDR);
	reg[8] = bus_read(trans, WF_RRO_TOP_STATISTIC_8_ADDR);
	reg[9] = bus_read(trans, WF_RRO_TOP_STATISTIC_9_ADDR);
	reg[10] = bus_read(trans, WF_RRO_TOP_STATISTIC_10_ADDR);
	reg[11] = bus_read(trans, WF_RRO_TOP_STATISTIC_11_ADDR);

	seq_printf(s, "STEP_ONE/WITHIN/SURPASS = %x/%x/%x\n", reg[0], reg[3], reg[4]);
	seq_printf(s, "REPEAT/OLDPKT/BAR = %x/%x/%x\n", reg[1], reg[2], reg[5]);
	seq_printf(s, "SURPASS with big gap = %x\n", reg[6]);
	seq_printf(s, "DISCONNECT/INVALID = %x/%x\n", reg[7], reg[8]);
	seq_printf(s, "TO(Step one)/TO(flush all) = %x/%x\n", reg[9], reg[10]);
	seq_printf(s, "buf ran out = %x\n", reg[11]);

	return 0;
}


u32 seid;

static ssize_t
rro_se_tbl_ctl_read_debugfs(struct file *file,
							char __user *ubuf,
							size_t count, loff_t *ppos)
{
	char out_buf[256] = {0};
	int len = 0;
	struct mtk_hw_dev *dev = file->private_data;
	struct mtk_bus_trans *trans = dev->bus_trans;
	u32 reg = 0;
	u32 addr1, addr2;
	u32 value[2];
	struct ba_session_tbl *tbl;

	reg = WF_RRO_TOP_DBG_RD_CTRL_DBG_RD_EXEC_MASK + 0x200;
	reg += (seid >> 1);
	bus_write(trans, WF_RRO_TOP_DBG_RD_CTRL_ADDR, reg);

	if (seid & 0x1) {
		addr1 = WF_RRO_TOP_DBG_RDAT_DW2_ADDR;
		addr2 = WF_RRO_TOP_DBG_RDAT_DW3_ADDR;
	} else {
		addr1 = WF_RRO_TOP_DBG_RDAT_DW0_ADDR;
		addr2 = WF_RRO_TOP_DBG_RDAT_DW1_ADDR;
	}
	value[0] = bus_read(trans, addr1);
	value[1] = bus_read(trans, addr2);
	tbl = (struct ba_session_tbl *) &value[0];
	len += scnprintf(out_buf + len, sizeof(out_buf) - len,
				"seid %d:\nba session table DW0:%08x DW2:%08x\n",
				seid,
				value[0],
				value[1]);

	len += scnprintf(out_buf + len, sizeof(out_buf) - len,
				"ack_sn = 0x%x, last_in_sn = 0x%x, sat/bn/bc/bd/cn = %d/%d/%d/%d/%d\n",
				tbl->ack_sn,
				tbl->last_in_sn,
				tbl->sat,
				tbl->bn,
				tbl->bc,
				tbl->bd,
				tbl->cn);

	len += scnprintf(out_buf + len, sizeof(out_buf) - len,
				"within_cnt = %d, to_sel = %d, last_in_rxtime = %d\n",
				tbl->within_cnt,
				tbl->to_sel,
				tbl->last_in_rxtime);

	return simple_read_from_buffer(ubuf, count, ppos, out_buf, len);
}


static ssize_t
rro_se_tbl_ctl_write_debugfs(struct file *file,
							const char __user *ubuf,
							size_t count, loff_t *ppos)
{
	char buf[256] = {0};
	int ret, rc;
	u32 in_seid;
	u32 reg = 0;
	struct mtk_hw_dev *dev = file->private_data;
	struct mtk_bus_trans *trans = dev->bus_trans;
	u32 addr1, addr2;
	u32 value[2];
	struct ba_session_tbl *tbl;

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		goto done;
	}

	buf[rc] = '\0';

	ret = sscanf(buf, "%u", &in_seid);

	if (ret != 1) {
		ret = -EINVAL;
		goto done;
	}

	if (in_seid >= MAX_HW_RRO_SESSION_CNT) {
		ret = -EINVAL;
		goto done;
	} else {
		seid = in_seid;
	}

	reg = WF_RRO_TOP_DBG_RD_CTRL_DBG_RD_EXEC_MASK + 0x200;
	reg += (seid >> 1);
	bus_write(trans, WF_RRO_TOP_DBG_RD_CTRL_ADDR, reg);
	if (seid & 0x1) {
		addr1 = WF_RRO_TOP_DBG_RDAT_DW2_ADDR;
		addr2 = WF_RRO_TOP_DBG_RDAT_DW3_ADDR;
	} else {
		addr1 = WF_RRO_TOP_DBG_RDAT_DW0_ADDR;
		addr2 = WF_RRO_TOP_DBG_RDAT_DW1_ADDR;
	}

	value[0] = bus_read(trans, addr1);
	value[1] = bus_read(trans, addr2);

	tbl = (struct ba_session_tbl *) &value[0];

	dev_info(dev->dev,
				"seid %d:\nba session table DW0:%08x DW2:%08x\n",
				seid,
				value[0],
				value[1]);

	dev_info(dev->dev,
				"ack_sn = 0x%x, last_in_sn = 0x%x, sat/bn/bc/bd/cn = %d/%d/%d/%d/%d\n",
				tbl->ack_sn,
				tbl->last_in_sn,
				tbl->sat,
				tbl->bn,
				tbl->bc,
				tbl->bd,
				tbl->cn);

	dev_info(dev->dev,
				"within_cnt = %d, to_sel = %d, last_in_rxtime = %d\n",
				tbl->within_cnt,
				tbl->to_sel,
				tbl->last_in_rxtime);

	ret = count;
done:
	return ret;
}

static const struct file_operations fops_rro_se_tbl_ctl = {
	.read = rro_se_tbl_ctl_read_debugfs,
	.write = rro_se_tbl_ctl_write_debugfs,
	.open = simple_open,
};

static ssize_t
addr_elem_read_debugfs(struct file *file,
							char __user *ubuf,
							size_t count, loff_t *ppos)
{
	char out_buf[256] = {0};
	int len = 0;

	len += scnprintf(out_buf + len, sizeof(out_buf) - len,
				"This file is for input.\n");
	return simple_read_from_buffer(ubuf, count, ppos, out_buf, len);
}

static ssize_t
addr_elem_write_debugfs(struct file *file,
							const char __user *ubuf,
							size_t count, loff_t *ppos)
{
	char buf[256] = {0};
	int ret, rc;
	u16 in_seid, in_start_sn, in_size = 1;
	struct mtk_hw_dev *dev = file->private_data;
	struct mtk_bus_trans *bus_trans = dev->bus_trans;
	struct pci_trans *pci_trans = to_pci_trans(bus_trans);
	struct hw_rro_cfg *rro_cfg = pci_trans->rro_cfg;

	rc = simple_write_to_buffer(buf, sizeof(buf) - 1, ppos, ubuf, count);

	if (rc < 0) {
		ret = rc;
		goto done;
	}

	buf[rc] = '\0';

	ret = sscanf(buf, "%hu %hu %hu", &in_seid, &in_start_sn, &in_size);

	if (ret < 2 || ret > 3) {
		ret = -EINVAL;
		goto done;
	}

	if (in_seid > MAX_HW_RRO_SESSION_CNT || in_size > rro_cfg->win_sz) {
		ret = -EINVAL;
		goto done;
	}

	bus_trans->ae_info->seid = in_seid;
	bus_trans->ae_info->start_sn = in_start_sn;
	bus_trans->ae_info->size = in_size;
	ret = count;
done:
	return ret;
}

static const struct file_operations fops_addr_elem = {
	.read = addr_elem_read_debugfs,
	.write = addr_elem_write_debugfs,
	.open = simple_open,
};

static int
mt7992_dump_addr_elem(struct seq_file *s, void *data)
{
	struct mtk_hw_dev *dev = seqfile_to_dev(s);
	struct mtk_bus_trans *bus_trans = dev->bus_trans;
	struct pci_trans *pci_trans = to_pci_trans(bus_trans);
	struct addr_elem *elem;
	struct hw_rro_cfg *rro_cfg = pci_trans->rro_cfg;
	u16 seid = bus_trans->ae_info->seid;
	u16 start_sn = bus_trans->ae_info->start_sn;
	u16 size = bus_trans->ae_info->size;
	u16 i, index;

	if (size == 0) {
		seq_puts(s, "please execute \"echo [seid] [ssn] [size] > addr_elem_input\" first.\n");
		return 0;
	}
	seq_puts(s, "-----------------------------------\n");
	seq_printf(s, "| ADDR_ARRAY_PA      : %pad |\n", &rro_cfg->addr_elem_alloc_pa[0]);
	seq_printf(s, "| Session ID         : %10u |\n", seid);
	seq_printf(s, "| Start Serial Number: %10u |\n", start_sn);
	seq_puts(s, "------------------------------------------------------------------------------\n");
	seq_puts(s, "| sn | id |phy address|     raw data     |sig|oor|s-cnt|pkt-addr-h|pkt-addr-l|\n");
	seq_puts(s, "------------------------------------------------------------------------------\n");
	for (i = 0; i < size; i++) {
		elem = get_addr_elem(pci_trans, rro_cfg, bus_trans->ae_info->seid,
		bus_trans->ae_info->start_sn + i);
		index = (start_sn + i) % rro_cfg->win_sz;
		seq_printf(s, "|%4u|%4u| 0x%08llx| %08x %08x|%3d| %1d | %4d|    0x%1x   |0x%08x|\n",
				 start_sn + i,
				 index, rro_cfg->addr_elem_alloc_pa[0] +
				 ((u64)seid * rro_cfg->win_sz + index) * sizeof(struct addr_elem),
				 *(((u32 *)elem)), *(((u32 *)elem) + 1),
				 le32_to_cpu(elem->signature), le32_to_cpu(elem->out_of_range),
				 le32_to_cpu(elem->seg_cnt),
				 le32_to_cpu(elem->head_pkt_addr_info_h),
				 le32_to_cpu(elem->head_pkt_addr_info_l));
	}
	seq_puts(s, "------------------------------------------------------------------------------\n");
	return 0;
}

#ifdef HW_DBG
static int
mt7992_reg_set(void *data, u64 val)
{
	struct mtk_hw_dev *dev = data;

	bus_write(dev->bus_trans, dev->dbg_ops->debugfs_reg, val);
	return 0;
}

static int
mt7992_reg_get(void *data, u64 *val)
{
	struct mtk_hw_dev *dev = data;

	*val = bus_read(dev->bus_trans, dev->dbg_ops->debugfs_reg);
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_regval, mt7992_reg_get, mt7992_reg_set,
			 "0x%08llx\n");
#endif

static void
mt7992_dbgfs_init(struct mtk_hw_dev *dev)
{
	struct dentry *dir = dev->sys_idx.dir;

	if (!dir)
		return;
	/*per chip support*/
	mt_debugfs_create_devm_seqfile(dev->dev, "mib", dir, mt7992_dump_mib_info);
	mt_debugfs_create_devm_seqfile(dev->dev, "pse", dir, mt7992_show_pse_info);
	mt_debugfs_create_devm_seqfile(dev->dev, "ple", dir, mt7992_show_ple_info);
	mt_debugfs_create_devm_seqfile(dev->dev, "agg", dir, mt7992_dump_agg_info);
	mt_debugfs_create_devm_seqfile(dev->dev, "wtbl", dir, mt7992_dump_wtbl_info);
	mt_debugfs_create_devm_seqfile(dev->dev, "efuse", dir, mtk_eeprom_debugfs);
	mt_debugfs_create_devm_seqfile(dev->dev, "rro_dbg", dir, mt7992_dbg_rro);
	mt_debugfs_create_devm_seqfile(dev->dev, "rro_mib", dir, mt7992_show_rro_mib);
	mt_debugfs_create_devm_seqfile(dev->dev, "addr_elem_output", dir, mt7992_dump_addr_elem);
	mt_debugfs_create_file("rro_se_tbl", 0644,
						dev->sys_idx.dir,
						dev,
						&fops_rro_se_tbl_ctl);
	mt_debugfs_create_file("addr_elem_input", 0644,
						dev->sys_idx.dir,
						dev,
						&fops_addr_elem);
#ifdef HW_DBG
	debugfs_create_u32("regidx", 0600, dir, &dev->dbg_ops->debugfs_reg);
	mt_debugfs_create_file_unsafe("regval", 0600, dir, dev,
				   &fops_regval);
#endif
}

static void
mt7992_dbgfs_exit(struct mtk_hw_dev *dev)
{

}

static int
mt7992_dbgfs_alloc(struct mtk_hw_dev *dev)
{
	struct mtk_bus_trans *trans = dev->bus_trans;

	trans->ae_info = kzalloc(sizeof(struct addr_elem_info), GFP_KERNEL);

	if (!(trans->ae_info))
		return -ENOMEM;

	return 0;
}

static void
mt7992_dbgfs_free(struct mtk_hw_dev *dev)
{
	struct mtk_bus_trans *trans = dev->bus_trans;

	kfree(trans->ae_info);
}

struct mtk_dbg_ops mt7992_dbg_ops = {
	.rxd_info = mt7992_dump_rxd_info,
	.txd_info = mt7992_dump_txd_info,
	.dbgfs_init = mt7992_dbgfs_init,
	.dbgfs_exit = mt7992_dbgfs_exit,
	.dbgfs_alloc = mt7992_dbgfs_alloc,
	.dbgfs_free = mt7992_dbgfs_free,
#ifdef CONFIG_HWIFI_DBG_ISR
	.show_isr_info = mt7992_show_isr_info,
#endif
};

static int __init mtk_chip_7992_dbg_init(void)
{
	mtk_wsys_dev_debugfs_plugin("mt7992", &mt7992_dbg_ops);
	return 0;
}

static void __exit mtk_chip_7992_dbg_exit(void)
{
	mtk_wsys_dev_debugfs_unplug("mt7992");
}


module_init(mtk_chip_7992_dbg_init);
module_exit(mtk_chip_7992_dbg_exit);


MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Mediatek Hardware Wi-Fi Module");
