/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	warp_hw.c
*/
#include <warp.h>
#include <warp_hw.h>
#include <warp_utility.h>
#include "wed_hw.h"
#include "wdma_hw.h"
#include <linux/proc_fs.h>
#include <linux/pci.h>

/*Local function*/

/*
*
*/
static inline int
wed_agt_dis_ck(struct wed_entry *wed, u32 addr, u32 busy_bit)
{
	u32 cnt = 0;
	u32 value;

	warp_io_read32(wed, addr, &value);

	while ((value & (1 << busy_bit)) &&
	       cnt < WED_POLL_MAX) {
		usleep_range(10000, 15000);
		warp_io_read32(wed, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): %x disable bit %d fail!!\n", __func__, addr,
			 busy_bit);
		return -1;
	}

	return 0;
}

/*
*
*/
static inline int
wdma_agt_dis_ck(struct wdma_entry *wdma, u32 addr, u32 busy_bit)
{
	u32 cnt = 0;
	u32 value;

	warp_io_read32(wdma, addr, &value);

	while ((value & (1 << busy_bit)) &&
	       cnt < WED_POLL_MAX) {
		usleep_range(10000, 15000);
		warp_io_read32(wdma, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): %x disable bit %d fail!!\n", __func__, addr,
			 busy_bit);
		return -1;
	}

	return 0;
}

/*
*
*/
static inline int
wifi_agt_dis_ck(struct wifi_entry *wifi, u32 addr, u32 busy_bit)
{
	u32 cnt = 0;
	u32 value;

	warp_io_read32(wifi, addr, &value);

	while ((value & (1 << busy_bit)) &&
	       cnt < WED_POLL_MAX) {
		usleep_range(10000, 15000);
		warp_io_read32(wifi, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): %x disable bit %d fail!!\n", __func__, addr,
			 busy_bit);
		return -1;
	}

	return 0;
}

/*
*
*/
static void
wed_dma_ctrl(struct wed_entry *wed, u8 txrx)
{
	u32 wed_cfg;
	u32 wed_wdma_cfg;
	u32 wed_wpdma_cfg;
	/*reset wed*/
	warp_io_read32(wed, WED_GLO_CFG, &wed_cfg);
	wed_cfg &= ~((1 << WED_GLO_CFG_FLD_TX_DMA_EN) | (1 <<
			WED_GLO_CFG_FLD_RX_DMA_EN));
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &wed_wpdma_cfg);
	wed_wpdma_cfg &= ~((1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN) |
			   (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN));
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~((1 << WED_WDMA_GLO_CFG_FLD_TX_DRV_EN) |
			  (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN));

	switch (txrx) {
	case WARP_DMA_TX: {
		warp_dbg(WARP_DBG_INF, "%s(): %s DMA TX.\n", __func__,
			 (txrx ? "ENABLE" : "DISABLE"));
		wed_cfg |= (1 << WED_GLO_CFG_FLD_TX_DMA_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);
		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	}
	break;

	case WARP_DMA_RX: {
		warp_dbg(WARP_DBG_INF, "%s(): %s DMA RX.\n", __func__,
			 (txrx ? "ENABLE" : "DISABLE"));
		wed_cfg |= (1 << WED_GLO_CFG_FLD_RX_DMA_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
	}
	break;

	case WARP_DMA_TXRX: {
		warp_dbg(WARP_DBG_INF, "%s(): %s DMA TXRX.\n", __func__,
			 (txrx ? "ENABLE" : "DISABLE"));
		wed_cfg |= ((1 << WED_GLO_CFG_FLD_TX_DMA_EN) | (1 <<
				WED_GLO_CFG_FLD_RX_DMA_EN));
		wed_wpdma_cfg |= ((1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN) |
				  (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN));
		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	}
	break;
	}

	warp_io_write32(wed, WED_GLO_CFG, wed_cfg);
#ifdef WED_HW_TX_SUPPORT
	warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, wed_wpdma_cfg);
}

/*
*
*/
#ifdef WED_HW_TX_SUPPORT
int
warp_tx_bm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	struct warp_ring *ring = ring_ctrl->ring;
	u32 value, i, ring_size_sum = 0;

	for (i = 0; i < ring_ctrl->ring_num; i++)
		ring_size_sum += ring[i].ring_lens;
	/*PAUSE BUF MGMT*/
	value = (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	/*should be set before WED_MOD_RST is invoked*/
	value |= ((res->pkt_num / WED_TOKEN_UNIT) << WED_TX_BM_CTRL_FLD_VLD_GRP_NUM);
	value |= ((ring_size_sum / 256) << WED_TX_BM_CTRL_FLD_RSV_GRP_NUM);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	/*TX BM_BASE*/
	value = res->des_buf.alloc_pa;
	warp_io_write32(wed, WED_TX_BM_BASE, value);
	/*TX token cfg */
	value = ((res->token_start) << WED_TX_BM_TKID_FLD_START_ID);
	value |= ((res->token_end) << WED_TX_BM_TKID_FLD_END_ID);
	warp_io_write32(wed, WED_TX_BM_TKID, value);
	/*TX packet len*/
	value = (res->pkt_len & WED_TX_BM_BLEN_FLD_BYTE_LEN_MASK);
	warp_io_write32(wed, WED_TX_BM_BLEN, value);
	/*dynamic adjust*/
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	value = (WED_TOKEN_LOW << WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM);
	value |= (WED_TOKEN_HIGH << WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM);
#else
	value = (1 << WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM);
	value |= (WED_TX_BM_DYN_TH_FLD_HI_GRP_MASK << WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM);
#endif /*WED_DYNAMIC_BW_SUPPORT*/
	warp_io_write32(wed, WED_TX_BM_DYN_TH, value);
	/*Reset Buf mgmt for ready to start*/
	value = 1 << WED_MOD_RST_FLD_TX_BM;
	WHNAT_RESET(wed, WED_MOD_RST, value);
	/*Enable agent for BM*/
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_BM_EN);
	value |= (1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);

	return 0;
}

/*
*
*/
void
warp_bfm_get_tx_freecnt_hw(struct wed_entry *wed, u32 *cnt)
{
	u32 value;
	u32 tcnt = 0;
	u32 fcnt = 0;
	u32 cr;
	u32 i;
	u32 j = 0;
	u32 k = 0;
	u32 grp = 0;
	char str[256] = "";
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;

	grp = buf_res->pkt_num / WED_TOKEN_STATUS_UNIT;
	tcnt = buf_res->pkt_num % WED_TOKEN_STATUS_UNIT;
	value = 1 << WED_DBG_CTRL_FLD_TX_BM_MEM_DBG;
	warp_io_write32(wed, WED_DBG_CTRL, value);

	for (cr = WED_BMF_VALID_TABLE_START; cr <= WED_BMF_VALID_TABLE_END; cr += 4) {
		if (j % 4 == 0)
			sprintf(str, "%08x:\t", cr);

		warp_io_read32(wed, cr, &value);

		if (k < grp) {
			for (i = 0; i < WED_TOKEN_STATUS_UNIT; i++) {
				if (value & (1 << i))
					fcnt++;
			}
		} else if (k == grp) {
			for (i = 0; i < tcnt; i++) {
				if (value & (1 << i))
					fcnt++;
			}
		}

		k++;
		sprintf(str + strlen(str), "%08x\t", value);

		if (j % 4 == 3) {
			warp_dbg(WARP_DBG_OFF, "%s\n", str);
			memset(str, 0, sizeof(str));
		}

		j++;
	}

	value = 0x14;
	warp_io_write32(wed, WED_DBG_CTRL, value);
	warp_io_read32(wed, WED_DBG_PRB1, &value);
	warp_dbg(WARP_DBG_OFF,
		 "Total Free Cnt(%d), CR: Free Cnt(%d), Usage Cnt(%d),Pkt Cnt(%d)\n",
		 fcnt,
		 ((value >> 16) & 0xffff),
		 (value & 0xffff),
		 buf_res->pkt_num);
	*cnt = ((value >> 16) & 0xffff);
	/*Disable debug*/
	warp_io_write32(wed, WED_DBG_CTRL, 0);
}

/*
*
*/
void
warp_hal_bfm_free(struct wed_entry *wed)
{
	u32 value;
	u32 cr = 0;
	u32 cnt = 0;
	u32 grp = 0;
	u32 i;
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;
	/*PAUSE BUF MGMT*/
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value |= (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	/*POLL status bit*/
	warp_io_read32(wed, WED_ST, &value);

	while ((value & (0xff << WED_ST_FLD_TX_ST)) && cnt < WED_POLL_MAX) {
		warp_io_read32(wed, WED_ST, &value);
		cnt++;
	}

	/*Set SRAM enable*/
	value = 1 << WED_DBG_CTRL_FLD_TX_BM_MEM_DBG;
	warp_io_write32(wed, WED_DBG_CTRL, value);
	grp = buf_res->pkt_num / WED_TOKEN_STATUS_UNIT;
	cnt = buf_res->pkt_num % WED_TOKEN_STATUS_UNIT;

	for (i = 0; i < grp; i++) {
		cr = WED_BMF_VALID_TABLE_START + 4 * i;
		warp_io_write32(wed, cr, 0xffffffff);
	}

	value = 0;
	cr += 4;

	for (i = 0; i < cnt; i++)
		value |= (1 << cnt);

	warp_io_write32(wed, cr, value);
	/*Disable SRAM Mapping*/
	warp_io_write32(wed, WED_DBG_CTRL, 0);
	/*Disable PAUSE*/
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
}
#endif /*WED_HW_TX_SUPPORT*/

#ifdef WED_DYNAMIC_TXBM_SUPPORT
/*
*
*/
void
warp_bfm_update_hw(struct wed_entry *wed, u8 reduce)
{
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	struct warp_ring *ring = ring_ctrl->ring;
	u32 value = 0;
	u32 vld_num = 0;
	u32 reduce_grp = 0;
	u32 cnt, i, ring_size_sum = 0;

	for (i = 0; i < ring_ctrl->ring_num; i++)
		ring_size_sum += ring[i].ring_lens;
	/*PAUSE BUF MGMT*/
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value |= (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	vld_num = (value & WED_TX_BM_CTRL_FLD_VLD_GRP_NUM_MASK);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	/*POLL status bit*/
	warp_io_read32(wed, WED_ST, &value);

	while ((value & (0xff << WED_ST_FLD_TX_ST)) && cnt < WED_POLL_MAX) {
		warp_io_read32(wed, WED_ST, &value);
		cnt++;
	}

	if (cnt == WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): can't poll tx buf status to clear!\n", __func__);
		goto end;
	}

	/*check token can reduce or not*/
	if (reduce) {
		reduce_grp = (res->pkt_num / WED_TOKEN_UNIT) - vld_num;
		vld_num -= reduce_grp;

		if (vld_num > FREE_CR_SIZE)
			warp_io_read32(wed, WED_TX_BM_VB_FREE_32_63, &value);
		else
			warp_io_read32(wed, WED_TX_BM_VB_FREE_0_31, &value);

		vld_num = (vld_num - 1) % FREE_CR_SIZE;

		if ((value & (1 << vld_num)) == 0) {
			warp_dbg(WARP_DBG_ERR,
				 "%s(): reduce too much packet buffer, buffer still inused!\n", __func__);
			goto end;
		}
	}

	/*update token*/
	value = (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	value |= ((res->pkt_num / WED_TOKEN_UNIT) << WED_TX_BM_CTRL_FLD_VLD_GRP_NUM);
	value |= ((ring_size_sum / 256) << WED_TX_BM_CTRL_FLD_RSV_GRP_NUM);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	warp_dbg(WARP_DBG_INF, "%s(): update packet buffer done!\n", __func__);
end:
	/*Disable PAUSE BUF MGMT*/
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
}
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/

/*
*
*/
static int
reset_wed_tx_dma(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_TX_SUPPORT
	u32 value;

	warp_io_read32(wed, WED_GLO_CFG, &value);
	value &= ~(1 << WED_GLO_CFG_FLD_TX_DMA_EN);
	warp_io_write32(wed, WED_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_GLO_CFG, WED_GLO_CFG_FLD_TX_DMA_BUSY) < 0)
		return -1;

	if (reset_type == WARP_RESET_IDX_ONLY) {
		value = (1 << WED_RST_IDX_FLD_DTX_IDX0);
		value |= (1 << WED_RST_IDX_FLD_DTX_IDX1);
		warp_io_write32(wed, WED_RST_IDX, value);
		warp_io_write32(wed, WED_RST_IDX, 0);
	} else {
		value = (1 << WED_MOD_RST_FLD_WED_TX_DMA);
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int
reset_wdma_rx(struct wdma_entry *wdma)
{
	u32 value;
	/*Stop Frame Engin WDMA*/
	warp_io_read32(wdma, WDMA_GLO_CFG, &value);
	value &= ~(1 << WDMA_GLO_CFG_RX_DMA_EN);
	warp_io_write32(wdma, WDMA_GLO_CFG, value);

	if (wdma_agt_dis_ck(wdma, WDMA_GLO_CFG, WDMA_GLO_CFG_RX_DMA_BUSY) < 0)
		return -1;

	/*Reset Frame Engine WDMA DRX/CRX index*/
	value = 1 << WDMA_RST_IDX_RST_DRX_IDX0;
	value |= 1 << WDMA_RST_IDX_RST_DRX_IDX1;
	warp_io_write32(wdma, WDMA_RST_IDX, value);
	warp_io_write32(wdma, WDMA_RST_IDX, 0);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_0, 0);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_1, 0);
	return 0;
}

/*
*
*/
static int reset_wed_rx_drv(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_HW_TX_SUPPORT
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
	u32 value;
	/*Stop Frame Engin WDMA*/
	warp_io_read32(wdma, WDMA_GLO_CFG, &value);
	value &= ~(1 << WDMA_GLO_CFG_RX_DMA_EN);
	warp_io_write32(wdma, WDMA_GLO_CFG, value);

	if (wdma_agt_dis_ck(wdma, WDMA_GLO_CFG, WDMA_GLO_CFG_RX_DMA_BUSY) < 0)
		return -1;

	/*Stop WED WDMA Rx Driver Engine*/
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &value);
	value |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_DISABLE_FSM_AUTO_IDLE);
	value &= ~(1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
#ifdef WED_WORK_AROUND_WDMA_RETURN_IDLE
	value |= (1 << WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE);
#endif /*WED_WORK_AROUND_WDMA_RETURN_IDLE*/
	warp_io_write32(wed, WED_WDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WDMA_GLO_CFG,
			   WED_WDMA_GLO_CFG_FLD_RX_DRV_BUSY) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv can't return idle state!\n", __func__);
#ifdef WED_WORK_AROUND_WDMA_RETURN_IDLE
		value &= ~(1 << WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE);
		warp_io_write32(wed, WED_WDMA_GLO_CFG, value);
#endif /*WED_WORK_AROUND_WDMA_RETURN_IDLE*/
		return -1;
	}

#ifdef WED_WORK_AROUND_WDMA_RETURN_IDLE
	value &= ~(1 << WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, value);
#endif /*WED_WORK_AROUND_WDMA_RETURN_IDLE*/
	/*Reset Frame Engine WDMA DRX/CRX index*/
	value = 1 << WDMA_RST_IDX_RST_DRX_IDX0;
	value |= 1 << WDMA_RST_IDX_RST_DRX_IDX1;
	warp_io_write32(wdma, WDMA_RST_IDX, value);
	warp_io_write32(wdma, WDMA_RST_IDX, 0);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_0, 0);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_1, 0);

	/*Reset WED WDMA RX Driver Engin DRV/CRX index only*/
	if (reset_type == WARP_RESET_IDX_ONLY) {
		value = (1 << WED_WDMA_RST_IDX_FLD_CRX_IDX0);
		value |= (1 << WED_WDMA_RST_IDX_FLD_CRX_IDX1);
		value |= (1 << WED_WDMA_RST_IDX_FLD_DRV_IDX0);
		value |= (1 << WED_WDMA_RST_IDX_FLD_DRV_IDX1);
		warp_io_write32(wed, WED_WDMA_RST_IDX, value);
		warp_io_write32(wed, WED_WDMA_RST_IDX, 0);
		warp_io_read32(wed, WED_WDMA_GLO_CFG, &value);
		value |= (1  << WED_WDMA_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG);
		warp_io_write32(wed, WED_WDMA_GLO_CFG, value);
		value &= ~(1 << WED_WDMA_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG);
		warp_io_write32(wed, WED_WDMA_GLO_CFG, value);
	} else {
		/*Reset WDMA Interrupt Agent*/
		value &= ~(1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
		warp_io_write32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WDMA_INT_AGT_BUSY) < 0)
			return -1;

		value = 1 << WED_MOD_RST_FLD_WDMA_INT_AGT;
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*Reset WED RX Driver Engin*/
		value = 1 << WED_MOD_RST_FLD_WDMA_RX_DRV;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_HW_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int reset_wed_tx_bm(struct wed_entry *wed)
{
#ifdef WED_HW_TX_SUPPORT
	u32 value;
	u32 cnt = 0;
	/*Tx Free Agent Reset*/
	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);

	if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WED_TX_FREE_AGT_BUSY) < 0) {
		warp_dbg(WARP_DBG_OFF, "%s(): tx free agent reset faild!\n", __func__);
		return -1;
	}

	warp_io_read32(wed, WED_TX_BM_INTF, &value);

	while (((value >> 16) & 0xffff) != 0x40 && cnt < WED_POLL_MAX) {
		warp_io_read32(wed, WED_TX_BM_INTF, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_OFF, "%s(): tx free agent fifo reset faild!\n", __func__);
		return -1;
	}

	value = 1 << WED_MOD_RST_FLD_TX_FREE_AGT;
	WHNAT_RESET(wed, WED_MOD_RST, value);
	/*Reset TX Buffer manager*/
	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_TX_BM_EN);
	warp_io_write32(wed, WED_CTRL, value);

	if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WED_TX_BM_BUSY) < 0) {
		warp_dbg(WARP_DBG_OFF, "%s(): tx bm reset faild!\n", __func__);
		return -1;
	}

	value = 1 << WED_MOD_RST_FLD_TX_BM;
	WHNAT_RESET(wed, WED_MOD_RST, value);
#endif /*WED_HW_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int
reset_wed_tx_drv(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_TX_SUPPORT
	u32 value;
	/*Disable TX driver*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &value);
	value &= ~(1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WPDMA_GLO_CFG,
			   WED_WPDMA_GLO_CFG_FLD_TX_DRV_BUSY) < 0)
		return -1;

	if (reset_type == WARP_RESET_IDX_ONLY) {
		/*Reset TX Ring only*/
		value = (1 << WED_WPDMA_RST_IDX_FLD_CTX_IDX0);
		value |= (1 << WED_WPDMA_RST_IDX_FLD_CTX_IDX1);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, value);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, 0);
	} else {
		/*Reset TX Interrupt agent*/
		warp_io_read32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		warp_io_write32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WPDMA_INT_AGT_BUSY) < 0)
			return -1;

		value = 1 << WED_MOD_RST_FLD_WPDMA_INT_AGT;
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*Reset Tx driver*/
		value = 1 << WED_MOD_RST_FLD_WPDMA_TX_DRV;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int
reset_tx_traffic(struct wed_entry *wed, u32 reset_type)
{
	int ret = 0;

	ret = reset_wed_tx_dma(wed, reset_type);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_tx_dma reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	ret = reset_wed_rx_drv(wed, reset_type);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_tx_drv reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	ret = reset_wed_tx_bm(wed);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_tx_bm reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	ret = reset_wed_tx_drv(wed, reset_type);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_tx_drv reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	return ret;
}

/*
*
*/
static int
reset_rx_traffic(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_RX_SUPPORT
	u32 value;
	/*disable WPDMA RX Driver Engine*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &value);
	value &= ~(1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WPDMA_GLO_CFG,
			   WED_WPDMA_GLO_CFG_FLD_RX_DRV_BUSY) < 0)
		return -1;

	warp_io_read32(wed, WED_GLO_CFG, &value);
	value &= ~(1 << WED_GLO_CFG_FLD_RX_DMA_EN);
	warp_io_write32(wed, WED_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_GLO_CFG, WED_GLO_CFG_FLD_RX_DMA_BUSY) < 0)
		return -1;

	if (reset_type == WARP_RESET_IDX_ONLY) {
		value = (1 << WED_WPDMA_RST_IDX_FLD_CRX_IDX1);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, value);
		value = (1 << WED_RST_IDX_FLD_DRX_IDX1);
		warp_io_write32(wed, WED_RST_IDX, value);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, 0);
		warp_io_write32(wed, WED_RST_IDX, 0);
		warp_io_write32(wed, WED_RX1_CTRL2, 0);
	} else {
		/*WPDMA  interrupt agent*/
		warp_io_read32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		warp_io_write32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WPDMA_INT_AGT_BUSY) < 0)
			return -1;

		value = (1 << WED_MOD_RST_FLD_WPDMA_INT_AGT);
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*WPDMA RX Driver Engin*/
		value = (1 << WED_MOD_RST_FLD_WPDMA_RX_DRV);
		WHNAT_RESET(wed, WED_MOD_RST, value);
		warp_io_write32(wed, WED_RX1_CTRL2, 0);
	}

#endif /*WED_RX_SUPPORT*/
	return 0;
}

/*
*
*/
static void
reset_all(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 value;
	/*Reset WDMA*/
	reset_wdma_rx(wdma);
	/*Reset WED*/
	value = 1 << WED_MOD_RST_FLD_WED;
	WHNAT_RESET(wed, WED_MOD_RST, value);
}

/*
*
*/
static void
restore_tx_traffic(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 value;
	/*WPDMA*/
#ifdef WED_TX_SUPPORT
	/*Enable TX Driver*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &value);
	value |= (1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, value);
#endif /*WED_TX_SUPPORT*/
	/*Enable TX interrupt agent*/
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);
#ifdef WED_HW_TX_SUPPORT
	/*TX BM*/
	/*Enable TX buffer mgnt*/
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_BM_EN);
	warp_io_write32(wed, WED_CTRL, value);
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	/*Enable TX free agent*/
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);
	/*WDMA*/
	/*Enable WDMA RX Driver*/
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &value);
	value |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, value);
	/*Enable WDMA RX DMA*/
	warp_io_read32(wdma, WDMA_GLO_CFG, &value);
	value |= (1 << WDMA_GLO_CFG_RX_DMA_EN);
	warp_io_write32(wdma, WDMA_GLO_CFG, value);
	/*Enable WDMA interrupt agent*/
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);
	/*Enable WED WDMA RX Engine*/
	warp_io_read32(wed, WED_GLO_CFG, &value);
	value |= (1 << WED_GLO_CFG_FLD_TX_DMA_EN);
	warp_io_write32(wed, WED_GLO_CFG, value);
#endif /*WED_HW_TX_SUPPORT*/
}

/*
*
*/
static void
restore_rx_traffic(struct wed_entry *wed)
{
#ifdef WED_RX_SUPPORT
	u32 value;
	/*Enable WED RX DMA*/
	warp_io_read32(wed, WED_GLO_CFG, &value);
	value |= (1 << WED_GLO_CFG_FLD_RX_DMA_EN);
	warp_io_write32(wed, WED_GLO_CFG, value);
	/*Enable WPDMA Rx Driver*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &value);
	value |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, value);
	/*Enable WPDAM RX interrupt agent*/
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);
#endif /*WED_RX_SUPPORT*/
}

/*
*
*/
static void
wed_ctr_intr_set(struct wed_entry *wed, enum wed_int_agent int_agent,
		 unsigned char enable)
{
	u32 value = 0;

	warp_io_read32(wed, WED_CTRL, &value);

	switch (int_agent) {
	case WPDMA_INT_AGENT:
		if (enable)
			value |= (1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		else
			value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		break;
#ifdef WED_HW_TX_SUPPORT
	case WDMA_INT_AGENT:
		if (enable)
			value |= (1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
		else
			value &= ~(1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
		break;
#endif /*WED_HW_TX_SUPPORT*/
	case ALL_INT_AGENT:
		if (enable) {
			value |= (1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
#ifdef WED_HW_TX_SUPPORT
			value |= (1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
#endif /*WED_HW_TX_SUPPORT*/
		} else {
			value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
#ifdef WED_HW_TX_SUPPORT
			value &= ~(1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
#endif /*WED_HW_TX_SUPPORT*/
		}
		break;
	}

	warp_io_write32(wed, WED_CTRL, value);

#ifdef WED_DELAY_INT_SUPPORT
	warp_io_write32(wed, WED_DLY_INT_CFG, WED_DLY_INT_VALUE);
#endif /*WED_DELAY_INT_SUPPORT*/
}

/*
*
*/
static u32
wifi_cr_get(struct warp_entry *entry, u32 cr)
{
	u32 i = 0;

	while (entry->mtbl[i].wifi_cr != 0) {
		if (entry->mtbl[i].warp_cr == cr)
			return entry->mtbl[i].wifi_cr;

		i++;
	}

	warp_dbg(WARP_DBG_ERR, "%s(): can't get wifi cr from warp cr %x\n", __func__,
		 cr);
	return 0;
}

/*
* assign tx descripton to hw cr
*/

/*
*
*/
void
warp_int_ctrl_hw(struct wed_entry *wed, struct wifi_entry *wifi,
	struct wdma_entry *wdma, u32 int_agent,
	u8 enable, u32 pcie_ints_offset, int idx)
{
	struct wifi_hw *hw = &wifi->hw;
	u32 value = 0;
	/*wed control cr set*/
	wed_ctr_intr_set(wed, int_agent, enable);

	/*pcie interrupt status trigger register*/
	warp_io_write32(wed, WED_PCIE_INTS_TRIG, pcie_ints_offset);
	/*WPDMA interrupt triger*/
	value = 0;

	if (enable) {
#ifdef WED_RX_SUPPORT
		value |= (1 << WED_WPDMA_INT_TRIG_FLD_RX_DONE_INT1);
#endif /*WED_RX_SUPPORT*/
#ifdef WED_TX_SUPPORT
		value |= (1 << WED_WPDMA_INT_TRIG_FLD_TX_DONE_INT0);
		value |= (1 << WED_WPDMA_INT_TRIG_FLD_TX_DONE_INT1);
#endif /*WED_TX_SUPPORT*/
	}

	warp_io_write32(wed, WED_WPDMA_INT_TRIG, value);
	warp_io_read32(wed, WED_WPDMA_INT_CTRL, &value);
	value |= (1 << WED_WPDMA_INT_CTRL_FLD_SUBRT_ADV);
	/*disable due to interrupr lost issue should not happen in ASIC*/
	/*value |= (1 << WED_WPDMA_INT_CTRL_FLD_MSK_ON_DLY);*/
	warp_io_write32(wed, WED_WPDMA_INT_CTRL, value);
#ifdef WED_HW_TX_SUPPORT
	{
		/*WED_WDMA Interrupt agent */
		value = 0;

		if (enable) {
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE0);
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE1);
		}

		warp_io_write32(wed, WED_WDMA_INT_TRIG, value);
		/*WED_WDMA_SRC SEL */
		warp_io_read32(wed, WED_WDMA_INT_CTRL, &value);
		/*setting for wdma_int0->wdma0, wdma_int1->wdma1*/
		value &= ~(WED_WDMA_INT_CTRL_FLD_POLL_SRC_SEL_MASK <<
			   WED_WDMA_INT_CTRL_FLD_POLL_SRC_SEL);
		warp_io_write32(wed, WED_WDMA_INT_CTRL, value);
		/*WDMA interrupt enable*/
		value = 0;

		if (enable) {
			value |= (1 << WDMA_INT_MSK_RX_DONE_INT1);
			value |= (1 << WDMA_INT_MSK_RX_DONE_INT0);
		}

		warp_io_write32(wdma, WDMA_INT_MSK, value);
		warp_io_write32(wdma, WDMA_INT_GRP2, value);
	}
#endif /*WED_HW_TX_SUPPORT*/

	if (enable && hw->p_int_mask) {
		warp_pdma_mask_set_hw(wed, *hw->p_int_mask);
		warp_io_write32(wifi, hw->int_mask, *hw->p_int_mask);
	} else {
		warp_pdma_mask_set_hw(wed, 0);
		warp_io_write32(wifi, hw->int_mask, 0);
	}
}

/*
*
*/
void
warp_eint_init_hw(struct wed_entry *wed)
{
	u32 value = 0;
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	value |= (1 << WED_EX_INT_STA_FLD_TX_FBUF_HTH);
	value |= (1 << WED_EX_INT_STA_FLD_TX_FBUF_LTH);
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/
	value |= (1 << WED_EX_INT_STA_FLD_TF_LEN_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TF_TKID_WO_PYLD);
#ifdef WED_DYNAMIC_RXBM_SUPPORT
	value |= (1 << WED_EX_INT_STA_FLD_RX_FBUF_HTH);
	value |= (1 << WED_EX_INT_STA_FLD_RX_FBUF_LTH);
#endif /*WED_DYNAMIC_RXBM_SUPPORT*/
	value |= (1 << WED_EX_INT_STA_FLD_TX_DMA_W_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TX_DMA_R_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_INTI_WDMA_ENABLE);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_COHERENT);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_W_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_R_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TF_TKID_TITO_INVLD);
	wed->ext_int_mask = value;
}

/*
*
*/
void
warp_eint_ctrl_hw(struct wed_entry *wed, u8 enable)
{
	u32 value = 0;

	if (enable)
		value = wed->ext_int_mask;

	warp_io_write32(wed, WED_EX_INT_MSK, value);
	warp_io_read32(wed, WED_EX_INT_MSK, &value);
}

/*
*
*/
void
warp_eint_get_stat_hw(struct wed_entry *wed, u32 *state, u32 *err_state)
{
	/*read stat*/
	warp_io_read32(wed, WED_EX_INT_STA, state);
	/*write 1 clear*/
	warp_io_write32(wed, WED_EX_INT_STA, *state);

	*err_state = 0;
}

/*
*
*/
void
warp_dma_ctrl_hw(struct wed_entry *wed, u8 txrx)
{
	wed_dma_ctrl(wed, txrx);
}

/*
*
*/
void
warp_write_hw_extra(u32 addr, u32 *val)
{
#ifdef WED_DELAY_INT_SUPPORT
	if (addr == WED_INT_MSK) {
		*val &= ~((1 << WED_INT_MSK_FLD_TX_DONE_INT0) | (1 <<
				WED_INT_MSK_FLD_TX_DONE_INT1));
		*val &= ~(1 << WED_INT_MSK_FLD_RX_DONE_INT1);
		*val |= (1 << WED_INT_MSK_FLD_TX_DLY_INT);
		*val |= (1 << WED_INT_MSK_FLD_RX_DLY_INT);
	}
#endif /*WED_DELAY_INT_SUPPORT*/

}

/*
*
*/
void
warp_wdma_ring_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
#ifdef WED_HW_TX_SUPPORT
	struct wdma_rx_ring_ctrl *ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;
	struct warp_ring *ring;
	u32 offset;
	int i;

	/*set PDMA & WED_WPDMA Ring, wifi driver will configure WDMA ring by warp_hal_tx_ring_ctrl */
	/*tx ring*/
	for (i = 0; i < ring_ctrl->ring_num; i++) {
		offset = i * WDMA_RING_OFFSET;
		ring = &ring_ctrl->ring[i];
		warp_dbg(WARP_DBG_INF, "%s(): configure ring %d setting\n", __func__, i);
		warp_dbg(WARP_DBG_INF, "%s(): wed:%p,wdma:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
			 wed, wdma,
			 ring->hw_desc_base, (unsigned long)ring->cell[0].pkt_pa,
			 ring->hw_cnt_addr, ring_ctrl->ring_len,
			 ring->hw_cidx_addr, 0);
		/* WDMA */
		warp_io_write32(wdma, ring->hw_desc_base, ring->cell[0].alloc_pa);
		warp_io_write32(wdma, ring->hw_cnt_addr, ring_ctrl->ring_len);
		warp_io_write32(wdma, ring->hw_cidx_addr, 0);
		/* WED_WDMA */
		warp_io_write32(wed, WED_WDMA_RX0_BASE + offset, ring->cell[0].alloc_pa);
		warp_io_write32(wed, WED_WDMA_RX0_CNT + offset, ring_ctrl->ring_len);
	}

#endif /*WED_HW_TX_SUPPORT*/
}

#ifdef WED_TX_SUPPORT
/*
*
*/
int
warp_tx_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct wifi_hw *hw = &wifi->hw;
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	struct warp_ring *ring;
	u32 offset = (hw->tx[0].base - WED_WPDMA_TX0_CTRL0);
	int i;

	/*set PDMA & WED_WPDMA Ring, wifi driver will configure WDMA ring by warp_hal_tx_ring_ctrl */
	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		warp_dbg(WARP_DBG_INF, "%s(): configure ring %d setting\n", __func__, i);
		warp_dbg(WARP_DBG_INF, "%s(): wed:%p wifi:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
			 wed, wifi,
			 ring->hw_desc_base, (unsigned long)ring->cell[0].alloc_pa,
			 ring->hw_cnt_addr, ring->ring_lens,
			 ring->hw_cidx_addr, 0);
		/*WPDMA*/
		warp_io_write32(wifi, ring->hw_desc_base, ring->cell[0].alloc_pa);
		warp_io_write32(wifi, ring->hw_cnt_addr, ring->ring_lens);
		warp_io_write32(wifi, ring->hw_cidx_addr, 0);
		/*WED_WPDMA*/
		warp_io_write32(wed, (ring->hw_desc_base - offset), ring->cell[0].alloc_pa);
		warp_io_write32(wed, (ring->hw_cnt_addr - offset), ring->ring_lens);
		warp_io_write32(wed, (ring->hw_cidx_addr - offset), 0);
	}
	return 0;
}
#endif /*WED_TX_SUPPORT*/

#ifdef WED_RX_SUPPORT
/*
*
*/
int
warp_rx_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value;
	struct wifi_hw *hw = &wifi->hw;

	/*Rx Ring base */
	warp_io_read32(wed, WED_RX1_CTRL0, &value);
	warp_io_write32(wed, WED_WPDMA_RX1_CTRL0, value);
	warp_io_write32(wifi, hw->event.base, value);
	/*Rx CNT*/
	warp_io_read32(wed, WED_RX1_CTRL1, &value);
	warp_io_write32(wed, WED_WPDMA_RX1_CTRL1, value);
	warp_io_write32(wifi, hw->event.cnt, value);
	/*cpu idx*/
	warp_io_read32(wed, WED_RX1_CTRL2, &value);
	warp_io_write32(wed, WED_WPDMA_RX1_CTRL2, value);
	warp_io_write32(wifi, hw->event.cidx, value);
	return 0;
}
#endif /*WED_RX_SUPPORT*/

/*
*
*/
void
warp_trace_set_hw(struct warp_cputracer *tracer)
{
	u32 value = 0;
	/*enable cpu tracer*/
	warp_io_write32(tracer, CPU_TRACER_WP_ADDR, tracer->trace_addr);
	warp_io_write32(tracer, CPU_TRACER_WP_ADDR, tracer->trace_mask);

	if (tracer->trace_en) {
		value = (1 << CPU_TRACER_CON_BUS_DBG_EN) |
			(1 << CPU_TRACER_CON_WP_EN)		 |
			(1 << CPU_TRACER_CON_IRQ_WP_EN);
	}

	warp_io_write32(tracer, CPU_TRACER_CFG, value);
}

/*
*
*/
int
warp_reset_hw(struct wed_entry *wed, u32 reset_type)
{
	int ret = 0;
	struct warp_entry *warp = wed->warp;
	struct wdma_entry *wdma = &warp->wdma;

	switch (reset_type) {
	case WARP_RESET_IDX_ONLY:
	case WARP_RESET_IDX_MODULE:
		ret = reset_tx_traffic(wed, reset_type);

		if (ret < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): wed_tx reset fail,ret=%d\n", __func__, ret);
			return ret;
		}

		ret = reset_rx_traffic(wed, reset_type);

		if (ret < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): wed_rx reset fail,ret=%d\n", __func__, ret);
			return ret;
		}

		break;

	case WARP_RESET_INTERFACE:
		reset_all(wed, wdma);
		break;
	}

	return ret;
}

int
warp_stop_hw(struct wed_entry *wed, struct wdma_entry *wdma, u8 stop_type)
{
	return 0;
}


/*
*
*/
void
warp_restore_hw(struct wed_entry *wed, struct wdma_entry *wdma,  u8 restore_type)
{
	restore_rx_traffic(wed);
	restore_tx_traffic(wed, wdma);
}

/*
*
*/
void
warp_wifi_set_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = wifi->hw.wpdma_base;
	warp_io_write32(wed, WED_WPDMA_CFG_BASE, value);
}

int
warp_bus_msi_set(struct warp_entry *warp, struct warp_bus *bus, u8 enable)
{
	return 0;
}

/*
*
*/
void
warp_mtable_build_hw(struct warp_entry *warp)
{
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct warp_wifi_cr_map *m = &warp->mtbl[0];
#define MTBL_ADD(_wifi_cr, _warp_cr)\
	do {\
		m->warp_cr = _warp_cr,\
		m->wifi_cr = _wifi_cr;\
		m++;\
	} while(0)

	/*insert hw*/
	MTBL_ADD(hw->int_sta,    WED_INT_STA);
	MTBL_ADD(hw->int_mask,   WED_INT_MSK);
#ifdef WED_RX_SUPPORT
	MTBL_ADD(hw->event.base, WED_RX1_CTRL0);
	MTBL_ADD(hw->event.cnt,  WED_RX1_CTRL1);
	MTBL_ADD(hw->event.cidx, WED_RX1_CTRL2);
	MTBL_ADD(hw->event.didx, WED_RX1_CTRL3);
#endif /*RX*/
#ifdef WED_TX_SUPPORT
	MTBL_ADD(hw->tx[0].base, WED_TX0_CTRL0);
	MTBL_ADD(hw->tx[0].cnt,  WED_TX0_CTRL1);
	MTBL_ADD(hw->tx[0].cidx, WED_TX0_CTRL2);
	MTBL_ADD(hw->tx[0].didx, WED_TX0_CTRL3);
	MTBL_ADD(hw->tx[1].base, WED_TX1_CTRL0);
	MTBL_ADD(hw->tx[1].cnt,  WED_TX1_CTRL1);
	MTBL_ADD(hw->tx[1].cidx, WED_TX1_CTRL2);
	MTBL_ADD(hw->tx[1].didx, WED_TX1_CTRL3);
#endif
	MTBL_ADD(0, 0);

#undef MTBL_ADD
}

/*
*
*/
void
warp_pdma_mask_set_hw(struct wed_entry *wed, u32 int_mask)
{
	warp_io_write32(wed, WED_WPDMA_INT_MSK, int_mask);
}

/*
*
*/
void
warp_ser_trigger_hw(struct wifi_entry *wifi)
{
	struct wifi_hw *hw = &wifi->hw;

	warp_io_write32(wifi, hw->int_ser, hw->int_ser_value);
}

void
warp_wed_ver(struct wed_entry *wed)
{
	wed->ver = 1;
	wed->sub_ver = 0;
	wed->branch = 0;
	wed->eco = 0;
}

/*
*
*/
void
warp_wed_init_hwcap(int *hw_cap)
{
	*hw_cap = 0;
}

/*
*
*/
void
warp_ser_update_hw(struct wed_entry *wed, struct wed_ser_state *state)
{
	/*WED_TX_DMA*/
	warp_io_read32(wed, WED_ST, &state->tx_dma_stat);
	state->tx_dma_stat = (state->tx_dma_stat >> WED_ST_FLD_TX_ST) & 0xff;
	warp_io_read32(wed, WED_TX0_MIB, &state->tx0_mib);
	warp_io_read32(wed, WED_TX1_MIB, &state->tx1_mib);
	warp_io_read32(wed, WED_TX0_CTRL2, &state->tx0_cidx);
	warp_io_read32(wed, WED_TX1_CTRL2, &state->tx1_cidx);
	warp_io_read32(wed, WED_TX0_CTRL3, &state->tx0_didx);
	warp_io_read32(wed, WED_TX1_CTRL3, &state->tx1_didx);
	/*WED_WDMA*/
	warp_io_read32(wed, WED_WDMA_ST, &state->wdma_stat);
	state->wdma_stat = state->wdma_stat & 0xff;
	warp_io_read32(wed, WED_WDMA_RX0_MIB, &state->wdma_rx0_mib);
	warp_io_read32(wed, WED_WDMA_RX1_MIB, &state->wdma_rx1_mib);
	warp_io_read32(wed, WED_WDMA_RX0_RECYCLE_MIB, &state->wdma_rx0_recycle_mib);
	warp_io_read32(wed, WED_WDMA_RX1_RECYCLE_MIB, &state->wdma_rx1_recycle_mib);
	/*WED_WPDMA*/
	warp_io_read32(wed, WED_WPDMA_ST, &state->wpdma_stat);
	state->wpdma_stat = (state->wpdma_stat >> WED_WPDMA_ST_FLD_TX_DRV_ST) & 0xff;
	warp_io_read32(wed, WED_WPDMA_TX0_MIB, &state->wpdma_tx0_mib);
	warp_io_read32(wed, WED_WPDMA_TX1_MIB, &state->wpdma_tx1_mib);
	/*WED_BM*/
	warp_io_read32(wed, WED_TX_BM_STS, &state->bm_tx_stat);
	state->bm_tx_stat = state->bm_tx_stat & 0xffff;
	warp_io_read32(wed, WED_TX_FREE_TO_TX_BM_TKID_MIB, &state->txfree_to_bm_mib);
	warp_io_read32(wed, WED_TX_BM_TO_WDMA_RX_DRV_TKID_MIB,
		       &state->txbm_to_wdma_mib);
	return;
}

/*
*
*/
void
warp_dbginfo_dump_cfg_hw(struct warp_entry *warp)
{
	struct wed_entry *wed = &warp->wed;
	struct wifi_entry *wifi = &warp->wifi;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_hw *hw = &wifi->hw;

	warp_dbg(WARP_DBG_OFF, "==========WED basic info:==========\n");
	dump_addr_value(wed, "WED_REV", WED_REV);
	dump_addr_value(wed, "WED_CTRL", WED_CTRL);
	dump_addr_value(wed, "WED_CTRL2", WED_CTRL2);
	dump_addr_value(wed, "WED_EX_INT_STA", WED_EX_INT_STA);
	dump_addr_value(wed, "WED_EX_INT_MSK", WED_EX_INT_MSK);
	dump_addr_value(wed, "WED_ST", WED_ST);
	dump_addr_value(wed, "WED_GLO_CFG", WED_GLO_CFG);
	dump_addr_value(wed, "WED_INT_STA", WED_INT_STA);
	dump_addr_value(wed, "WED_INT_MSK", WED_INT_MSK);
	dump_addr_value(wed, "WED_AXI_CTRL", WED_AXI_CTRL);
	warp_dbg(WARP_DBG_OFF, "==========WED TX buf info:==========\n");
	dump_addr_value(wed, "WED_BM_ST", WED_BM_ST);
	dump_addr_value(wed, "WED_TX_BM_BASE", WED_TX_BM_BASE);
	dump_addr_value(wed, "WED_TX_BM_CTRL", WED_TX_BM_CTRL);
	dump_addr_value(wed, "WED_TX_BM_TKID", WED_TX_BM_TKID);
	dump_addr_value(wed, "WED_TX_BM_STS", WED_TX_BM_STS);
	dump_addr_value(wed, "WED_TX_BM_DYN_TH", WED_TX_BM_DYN_TH);
	dump_addr_value(wed, "WED_TX_BM_INTF", WED_TX_BM_INTF);
	dump_addr_value(wed, "WED_TX_BM_RECYC", WED_TX_BM_RECYC);
	dump_addr_value(wed, "WED_TX_FREE_TO_TX_BM_TKID_MIB",
			WED_TX_FREE_TO_TX_BM_TKID_MIB);
	dump_addr_value(wed, "WED_TX_BM_TO_WDMA_RX_DRV_TKID_MIB",
			WED_TX_BM_TO_WDMA_RX_DRV_TKID_MIB);
	warp_dbg(WARP_DBG_OFF, "==========WED PCI Host Control:==========\n");
	dump_addr_value(wed, "WED_PCIE_CFG_BASE", WED_PCIE_CFG_BASE);
	dump_addr_value(wed, "WED_PCIE_OFST", WED_PCIE_OFST);
	dump_addr_value(wed, "WED_PCIE_INTS_TRIG", WED_PCIE_INTS_TRIG);
	dump_addr_value(wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL);
	warp_dbg(WARP_DBG_OFF, "==========WED_WPDMA basic info:==========\n");
	dump_addr_value(wed, "WED_WPDMA_ST", WED_WPDMA_ST);
	dump_addr_value(wed, "WED_WPDMA_INT_STA_REC", WED_WPDMA_INT_STA_REC);
	dump_addr_value(wed, "WED_WPDMA_GLO_CFG", WED_WPDMA_GLO_CFG);
	dump_addr_value(wed, "WED_WPDMA_CFG_BASE", WED_WPDMA_CFG_BASE);
	dump_addr_value(wed, "WED_WPDMA_OFST0", WED_WPDMA_OFST0);
	dump_addr_value(wed, "WED_WPDMA_OFST1", WED_WPDMA_OFST1);
	dump_addr_value(wed, "WED_WPDAM_CTRL", WED_WPDAM_CTRL);
	warp_dbg(WARP_DBG_OFF, "==========WED_WDMA basic info:==========\n");
	dump_addr_value(wed, "WED_WDMA_ST", WED_WDMA_ST);
	dump_addr_value(wed, "WED_WDMA_INFO", WED_WDMA_INFO);
	dump_addr_value(wed, "WED_WDMA_GLO_CFG", WED_WDMA_GLO_CFG);
	dump_addr_value(wed, "WED_WDMA_RST_IDX", WED_WDMA_RST_IDX);
	dump_addr_value(wed, "WED_WDMA_LOAD_DRV_IDX", WED_WDMA_LOAD_DRV_IDX);
	dump_addr_value(wed, "WED_WDMA_LOAD_CRX_IDX", WED_WDMA_LOAD_CRX_IDX);
	dump_addr_value(wed, "WED_WDMA_SPR", WED_WDMA_SPR);
	dump_addr_value(wed, "WED_WDMA_INT_STA_REC", WED_WDMA_INT_STA_REC);
	dump_addr_value(wed, "WED_WDMA_INT_TRIG", WED_WDMA_INT_TRIG);
	dump_addr_value(wed, "WED_WDMA_INT_CTRL", WED_WDMA_INT_CTRL);
	dump_addr_value(wed, "WED_WDMA_INT_CLR", WED_WDMA_INT_CLR);
	dump_addr_value(wed, "WED_WDMA_CFG_BASE", WED_WDMA_CFG_BASE);
	dump_addr_value(wed, "WED_WDMA_OFST0", WED_WDMA_OFST0);
	dump_addr_value(wed, "WED_WDMA_OFST1", WED_WDMA_OFST1);
	/*other part setting*/
	warp_dbg(WARP_DBG_OFF, "==========WDMA basic info:==========\n");
	dump_addr_value(wdma, "WDMA_GLO_CFG", WDMA_GLO_CFG);
	dump_addr_value(wdma, "WDMA_INT_MSK", WDMA_INT_MSK);
	dump_addr_value(wdma, "WDMA_INT_STA", WDMA_INT_STA_REC);
	dump_addr_value(wdma, "WDMA_INFO", WDMA_INFO);
	dump_addr_value(wdma, "WDMA_DELAY_INT_CFG", WDMA_DELAY_INT_CFG);
	dump_addr_value(wdma, "WDMA_FREEQ_THRES", WDMA_FREEQ_THRES);
	dump_addr_value(wdma, "WDMA_INT_STS_GRP0", WDMA_INT_STS_GRP0);
	dump_addr_value(wdma, "WDMA_INT_STS_GRP1", WDMA_INT_STS_GRP1);
	dump_addr_value(wdma, "WDMA_INT_STS_GRP2", WDMA_INT_STS_GRP2);
	dump_addr_value(wdma, "WDMA_INT_GRP1", WDMA_INT_GRP1);
	dump_addr_value(wdma, "WDMA_INT_GRP2", WDMA_INT_GRP2);
	dump_addr_value(wdma, "WDMA_SCH_Q01_CFG", WDMA_SCH_Q01_CFG);
	dump_addr_value(wdma, "WDMA_SCH_Q23_CFG", WDMA_SCH_Q23_CFG);
	warp_dbg(WARP_DBG_OFF, "==========WPDMA basic info:==========\n");
	dump_addr_value(wifi, "WPDMA_GLO_CFG", hw->tx_dma_glo_cfg);
	dump_addr_value(wifi, "WPDMA_INT_MSK", hw->int_mask);
	dump_addr_value(wifi, "WPDMA_INT_STA", hw->int_sta);
}

/*
*
*/
void
warp_procinfo_dump_cfg_hw(struct warp_entry *warp, struct seq_file *seq)
{
	warp_dbginfo_dump_cfg_hw(warp);
}

/*
*
*/
void
warp_dbginfo_dump_txinfo_hw(struct warp_entry *warp)
{
	struct wed_entry *wed = &warp->wed;
	struct wifi_entry *wifi = &warp->wifi;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_hw *hw = &wifi->hw;

	warp_dbg(WARP_DBG_OFF, "==========WED TX ring info:==========\n");
	dump_addr_value(wed, "WED_TX0_MIB", WED_TX0_MIB);
	dump_addr_value(wed, "WED_TX1_MIB", WED_TX1_MIB);
	dump_addr_value(wed, "WED_TX0_BASE", WED_TX0_CTRL0);
	dump_addr_value(wed, "WED_TX0_CNT", WED_TX0_CTRL1);
	dump_addr_value(wed, "WED_TX0_CIDX", WED_TX0_CTRL2);
	dump_addr_value(wed, "WED_TX0_DIDX", WED_TX0_CTRL3);
	dump_addr_value(wed, "WED_TX1_BASE", WED_TX1_CTRL0);
	dump_addr_value(wed, "WED_TX1_CNT", WED_TX1_CTRL1);
	dump_addr_value(wed, "WED_TX1_CIDX", WED_TX1_CTRL2);
	dump_addr_value(wed, "WED_TX1_DIDX", WED_TX1_CTRL3);
	/*WPDMA status from WED*/
	warp_dbg(WARP_DBG_OFF, "==========WED WPDMA TX ring info:==========\n");
	dump_addr_value(wed, "WED_WPDMA_TX0_MIB", WED_WPDMA_TX0_MIB);
	dump_addr_value(wed, "WED_WPDMA_TX0_BASE", WED_WPDMA_TX0_CTRL0);
	dump_addr_value(wed, "WED_WPDMA_TX0_CNT", WED_WPDMA_TX0_CTRL1);
	dump_addr_value(wed, "WED_WPDMA_TX0_CIDX", WED_WPDMA_TX0_CTRL2);
	dump_addr_value(wed, "WED_WPDMA_TX0_DIDX", WED_WPDMA_TX0_CTRL3);
	dump_addr_value(wed, "WED_WPDMA_TX0_COHERENT_MIB", WED_WPDMA_TX0_COHERENT_MIB);
	dump_addr_value(wed, "WED_WPDMA_TX1_MIB", WED_WPDMA_TX1_MIB);
	dump_addr_value(wed, "WED_WPDMA_TX1_BASE", WED_WPDMA_TX1_CTRL0);
	dump_addr_value(wed, "WED_WPDMA_TX1_CNT", WED_WPDMA_TX1_CTRL1);
	dump_addr_value(wed, "WED_WPDMA_TX1_CIDX", WED_WPDMA_TX1_CTRL2);
	dump_addr_value(wed, "WED_WPDMA_TX1_DIDX", WED_WPDMA_TX1_CTRL3);
	dump_addr_value(wed, "WED_WPDMA_TX1_COHERENT_MIB", WED_WPDMA_TX1_COHERENT_MIB);
	/*WPDMA*/
	warp_dbg(WARP_DBG_OFF, "==========WPDMA TX ring info:==========\n");
	dump_addr_value(wifi, "WPDMA_TX0_BASE", hw->tx[0].base);
	dump_addr_value(wifi, "WPDMA_TX0_CNT", hw->tx[0].cnt);
	dump_addr_value(wifi, "WPDMA_TX0_CRX_IDX", hw->tx[0].cidx);
	dump_addr_value(wifi, "WPDMA_TX0_DRX_IDX", hw->tx[0].didx);
	dump_addr_value(wifi, "WPDMA_TX1_BASE", hw->tx[1].base);
	dump_addr_value(wifi, "WPDMA_TX1_CNT", hw->tx[1].cnt);
	dump_addr_value(wifi, "WPDMA_TX1_CRX_IDX", hw->tx[1].cidx);
	dump_addr_value(wifi, "WPDMA_TX1_DRX_IDX", hw->tx[1].didx);
	/*WDMA status from WED*/
	warp_dbg(WARP_DBG_OFF, "==========WED WDMA RX ring info:==========\n");
	dump_addr_value(wed, "WED_WDMA_RX0_MIB", WED_WDMA_RX0_MIB);
	dump_addr_value(wed, "WED_WDMA_RX0_BASE", WED_WDMA_RX0_BASE);
	dump_addr_value(wed, "WED_WDMA_RX0_CNT", WED_WDMA_RX0_CNT);
	dump_addr_value(wed, "WED_WDMA_RX0_CRX_IDX", WED_WDMA_RX0_CRX_IDX);
	dump_addr_value(wed, "WED_WDMA_RX0_DRX_IDX", WED_WDMA_RX0_DRX_IDX);
	dump_addr_value(wed, "WED_WDMA_RX0_THRES_CFG", WED_WDMA_RX0_THRES_CFG);
	dump_addr_value(wed, "WED_WDMA_RX0_RECYCLE_MIB", WED_WDMA_RX0_RECYCLE_MIB);
	dump_addr_value(wed, "WED_WDMA_RX0_PROCESSED_MIB", WED_WDMA_RX0_PROCESSED_MIB);
	dump_addr_value(wed, "WED_WDMA_RX1_MIB", WED_WDMA_RX1_MIB);
	dump_addr_value(wed, "WED_WDMA_RX1_BASE", WED_WDMA_RX1_BASE);
	dump_addr_value(wed, "WED_WDMA_RX1_CNT", WED_WDMA_RX1_CNT);
	dump_addr_value(wed, "WED_WDMA_RX1_CRX_IDX", WED_WDMA_RX1_CRX_IDX);
	dump_addr_value(wed, "WED_WDMA_RX1_DRX_IDX", WED_WDMA_RX1_DRX_IDX);
	dump_addr_value(wed, "WED_WDMA_RX1_THRES_CFG", WED_WDMA_RX1_THRES_CFG);
	dump_addr_value(wed, "WED_WDMA_RX1_RECYCLE_MIB", WED_WDMA_RX1_RECYCLE_MIB);
	dump_addr_value(wed, "WED_WDMA_RX1_PROCESSED_MIB", WED_WDMA_RX1_PROCESSED_MIB);
	/*WDMA*/
	warp_dbg(WARP_DBG_OFF, "==========WED WDMA RX ring info:==========\n");
	dump_addr_value(wdma, "WDMA_RX_BASE_PTR_0", WDMA_RX_BASE_PTR_0);
	dump_addr_value(wdma, "WDMA_RX_MAX_CNT_0", WDMA_RX_MAX_CNT_0);
	dump_addr_value(wdma, "WDMA_RX_CRX_IDX_0", WDMA_RX_CRX_IDX_0);
	dump_addr_value(wdma, "WDMA_RX_DRX_IDX_0", WDMA_RX_DRX_IDX_0);
	dump_addr_value(wdma, "WDMA_RX_BASE_PTR_1", WDMA_RX_BASE_PTR_1);
	dump_addr_value(wdma, "WDMA_RX_MAX_CNT_1", WDMA_RX_MAX_CNT_1);
	dump_addr_value(wdma, "WDMA_RX_CRX_IDX_1", WDMA_RX_CRX_IDX_1);
	dump_addr_value(wdma, "WDMA_RX_DRX_IDX_1", WDMA_RX_DRX_IDX_1);
}

void
warp_procinfo_dump_txinfo_hw(struct warp_entry *warp, struct seq_file *output)
{
	warp_dbginfo_dump_txinfo_hw(warp);
}

/*
*
*/
void
warp_dbginfo_dump_rxinfo_hw(struct warp_entry *warp)
{
	struct wed_entry *wed = &warp->wed;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw  = &wifi->hw;

	warp_dbg(WARP_DBG_OFF, "==========WED RX ring info:==========\n");
	dump_addr_value(wed, "WED_RX1_MIB", WED_RX1_MIB);
	dump_addr_value(wed, "WED_RX1_BASE", WED_RX1_CTRL0);
	dump_addr_value(wed, "WED_RX1_CNT", WED_RX1_CTRL1);
	dump_addr_value(wed, "WED_RX1_CIDX", WED_RX1_CTRL2);
	dump_addr_value(wed, "WED_RX1_DIDX", WED_RX1_CTRL3);
	/*WPDMA status from WED*/
	warp_dbg(WARP_DBG_OFF, "==========WPDMA RX ring info:==========\n");
	dump_addr_value(wed, "WED_WPDMA_RX1_MIB", WED_WPDMA_RX1_MIB);
	dump_addr_value(wed, "WED_WPDMA_RX1_BASE", WED_WPDMA_RX1_CTRL0);
	dump_addr_value(wed, "WED_WPDMA_RX1_CNT", WED_WPDMA_RX1_CTRL1);
	dump_addr_value(wed, "WED_WPDMA_RX1_CIDX", WED_WPDMA_RX1_CTRL2);
	dump_addr_value(wed, "WED_WPDMA_RX1_DIDX", WED_WPDMA_RX1_CTRL3);
	dump_addr_value(wed, "WED_WPDMA_RX1_COHERENT_MIB", WED_WPDMA_RX1_COHERENT_MIB);
	dump_addr_value(wed, "WED_WPDMA_RX1_EXTC_FREE_TKID_MIB",
			WED_WPDMA_RX_EXTC_FREE_TKID_MIB);
	/*WPDMA*/
	warp_dbg(WARP_DBG_OFF, "==========WPDMA RX ring info:==========\n");
	dump_addr_value(wifi, "WPDMA_RX1_BASE", hw->event.base);
	dump_addr_value(wifi, "WPDMA_RX1_CNT", hw->event.cnt);
	dump_addr_value(wifi, "WPDMA_RX1_CRX_IDX", hw->event.cidx);
	dump_addr_value(wifi, "WPDMA_RX1_DRX_IDX", hw->event.didx);
}

void warp_procinfo_dump_rxinfo_hw(struct warp_entry *warp, struct seq_file *output)
{
	warp_dbginfo_dump_rxinfo_hw(warp);
}

/*
*
*/
void
warp_dbginfo_dump_wed_hw(struct wed_entry *wed)
{
	warp_dbg(WARP_DBG_OFF, "==========WED DEBUG INFO:==========\n");
	dump_addr_value(wed, "WED_IRQ_MON", WED_IRQ_MON);
	dump_addr_value(wed, "WED_TX_BM_VB_FREE_0_31", WED_TX_BM_VB_FREE_0_31);
	dump_addr_value(wed, "WED_TX_BM_VB_FREE_32_63", WED_TX_BM_VB_FREE_32_63);
	dump_addr_value(wed, "WED_TX_BM_VB_USED_0_31", WED_TX_BM_VB_USED_0_31);
	dump_addr_value(wed, "WED_TX_BM_VB_USED_32_63", WED_TX_BM_VB_USED_32_63);
	dump_addr_value(wed, "WED_WDMA_INT_MON", WED_WDMA_INT_MON);
	dump_addr_value(wed, "WED_WPDMA_INT_CLR", WED_WPDMA_INT_CLR);
	dump_addr_value(wed, "WED_WPDMA_INT_CTRL", WED_WPDMA_INT_CTRL);
	dump_addr_value(wed, "WED_WPDMA_INT_MSK", WED_WPDMA_INT_MSK);
	dump_addr_value(wed, "WED_WPDMA_INT_MON", WED_WPDMA_INT_MON);
	dump_addr_value(wed, "WED_WPDMA_SPR", WED_WPDMA_SPR);
	dump_addr_value(wed, "WED_PCIE_INTM_REC", WED_PCIE_INTM_REC);
	dump_addr_value(wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL);
	dump_addr_value(wed, "WED_TXD_DW0", WED_TXD_DW0);
	dump_addr_value(wed, "WED_TXD_DW1", WED_TXD_DW1);
	dump_addr_value(wed, "WED_TXD_DW2", WED_TXD_DW2);
	dump_addr_value(wed, "WED_TXD_DW3", WED_TXD_DW3);
	dump_addr_value(wed, "WED_TXD_DW4", WED_TXD_DW4);
	dump_addr_value(wed, "WED_TXD_DW5", WED_TXD_DW5);
	dump_addr_value(wed, "WED_TXD_DW6", WED_TXD_DW6);
	dump_addr_value(wed, "WED_TXD_DW7", WED_TXD_DW7);
	dump_addr_value(wed, "WED_TXP_DW0", WED_TXP_DW0);
	dump_addr_value(wed, "WED_TXP_DW1", WED_TXP_DW1);
	dump_addr_value(wed, "WED_DBG_CTRL", WED_DBG_CTRL);
	dump_addr_value(wed, "WED_DBG_PRB0", WED_DBG_PRB0);
	dump_addr_value(wed, "WED_DBG_PRB1", WED_DBG_PRB1);
	dump_addr_value(wed, "WED_DBG_PRB2", WED_DBG_PRB2);
	dump_addr_value(wed, "WED_DBG_PRB3", WED_DBG_PRB3);
	dump_addr_value(wed, "WED_TX_COHERENT_MIB", WED_TX_COHERENT_MIB);
	dump_addr_value(wed, "WED_TXP_DW0", WED_TXP_DW0);
}

/*
*
*/
void
warp_tx_ring_get_hw(struct warp_entry *warp, struct warp_ring *ring, u8 idx)
{
	u32 offset = idx * WED_RING_OFFSET;

	ring->hw_desc_base = wifi_cr_get(warp, WED_TX0_CTRL0 + offset);
	ring->hw_cnt_addr  = wifi_cr_get(warp, WED_TX0_CTRL1 + offset);
	ring->hw_cidx_addr = wifi_cr_get(warp, WED_TX0_CTRL2 + offset);
	ring->hw_didx_addr = wifi_cr_get(warp, WED_TX0_CTRL3 + offset);
}

/*
*
*/
void
warp_wdma_rx_ring_get_hw(struct warp_ring *ring, u8 idx)
{
	u32 offset = idx * WDMA_RING_OFFSET;

	ring->hw_desc_base = WDMA_RX_BASE_PTR_0 + offset;
	ring->hw_cnt_addr  = WDMA_RX_MAX_CNT_0 + offset;
	ring->hw_cidx_addr = WDMA_RX_CRX_IDX_0 + offset;
	ring->hw_didx_addr = WDMA_RX_DRX_IDX_0 + offset;
}

void
warp_wed_tx_ring_get_idx(struct wed_entry *wed, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * WED_RING_OFFSET;

	warp_io_read32(wed, WED_TX0_CTRL0 + offset, base);
	warp_io_read32(wed, WED_TX0_CTRL1 + offset, tcnt);
	warp_io_read32(wed, WED_TX0_CTRL2 + offset, cidx);
	warp_io_read32(wed, WED_TX0_CTRL3 + offset, didx);
}

void
warp_wed_rx_ring_get_idx(struct wed_entry *wed, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * WED_RING_OFFSET;

	warp_io_read32(wed, WED_RX0_CTRL0 + offset, base);
	warp_io_read32(wed, WED_RX0_CTRL1 + offset, tcnt);
	warp_io_read32(wed, WED_RX0_CTRL2 + offset, cidx);
	warp_io_read32(wed, WED_RX0_CTRL3 + offset, didx);
}

void
warp_wdma_tx_ring_get_idx(struct wdma_entry *wdma, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	;
}
void
warp_wdma_rx_ring_get_idx(struct wdma_entry *wdma, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * WDMA_RING_OFFSET;

	warp_io_read32(wdma, WDMA_RX_BASE_PTR_0 + offset, base);
	warp_io_read32(wdma, WDMA_RX_MAX_CNT_0 + offset, tcnt);
	warp_io_read32(wdma, WDMA_RX_CRX_IDX_0 + offset, cidx);
	warp_io_read32(wdma, WDMA_RX_DRX_IDX_0 + offset, didx);
}

/*
*
*/
static inline void
eint_disable(struct wed_entry *wed, u32 isr)
{
	wed->ext_int_mask &= ~(1 << isr);
}

/*
*
*/
static inline void
eint_enable(struct wed_entry *wed, u32 isr)
{
	wed->ext_int_mask |= (1 << isr);
	warp_eint_ctrl_hw(wed, true);
}

/*
*
*/
void
warp_eint_work_hw(struct wed_entry *wed, u32 status, u32 err_status)
{
#ifdef WED_DYNAMIC_RXBM_SUPPORT

	if (status & (1 << WED_EX_INT_STA_FLD_RX_FBUF_HTH))
		warp_dbg(WARP_DBG_ERR, "%s(): rx rbuf high threshold!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_FBUF_LTH))
		warp_dbg(WARP_DBG_ERR, "%s(): rx rbuf low threshold!\n", __func__);

#endif	/* WED_DYNAMIC_RXBM_SUPPORT */

	if (status & (1 << WED_EX_INT_STA_FLD_TF_LEN_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): tx free notify len error!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TF_TKID_WO_PYLD))
		warp_dbg(WARP_DBG_ERR, "%s(): tx free token has no packet to point!\n",
			 __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TX_FBUF_HTH)) {
#ifdef WED_DYNAMIC_TXBM_SUPPORT
		eint_disable(wed, WED_EX_INT_STA_FLD_TX_FBUF_HTH);
		if ((WED_PKT_NUM_GET(wed) - WED_TOKEN_EXPEND_SIZE) >= WED_TOKEN_CNT_GET(wed)) {
			warp_dbg(WARP_DBG_INF, "%s(): tx buf high threshold!\n", __func__);
			tasklet_hi_schedule(&wed->tbuf_free_task);
		}
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/
	}

	if (status & (1 << WED_EX_INT_STA_FLD_TX_FBUF_LTH)) {
#ifdef WED_DYNAMIC_TXBM_SUPPORT
		eint_disable(wed, WED_EX_INT_STA_FLD_TX_FBUF_LTH);
		if ((WED_PKT_NUM_GET(wed) + WED_TOKEN_EXPEND_SIZE) <= WED_TOKEN_NUM_GET(wed)) {
			warp_dbg(WARP_DBG_INF, "%s(): tx buf low threshold!\n", __func__);
			tasklet_hi_schedule(&wed->tbuf_alloc_task);
		}

#endif /*WED_DYNAMIC_TXBM_SUPPORT*/
	}

	if (status & (1 << WED_EX_INT_STA_FLD_TX_DMA_W_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): tx dma write resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TX_DMA_R_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): tx dma read resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_INTI_WDMA_ENABLE))
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv inti wdma enable!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_COHERENT))
		warp_dbg(WARP_DBG_LOU, "%s(): rx drv coherent!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_W_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv write resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_R_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv read resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TF_TKID_TITO_INVLD))
		warp_dbg(WARP_DBG_ERR, "%s(): tx free token id is invaild!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_BM_DMAD_COHERENT))
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv buffer mgmt dmad coherent!\n", __func__);
}

#ifdef WARP_CPU_TRACER
/*
*
*/
void
warp_bus_cputracer_work_hw(struct warp_cputracer *tracer)
{
	u32 value;
	u32 sta;

	warp_io_read32(tracer, CPU_TRACER_CFG, &sta);
	value = sta | (1 << CPU_TRACER_CON_IRQ_CLR);

	if (sta & (1 << CPU_TRACER_CON_IRQ_WP_STA))
		warp_dbg(WARP_DBG_OFF, "[tracker] watch address: 0x%x was touched\n",
			 tracer->trace_addr);

	if (sta & (1 << CPU_TRACER_CON_IRQ_AR_STA))
		warp_dbg(WARP_DBG_OFF, "[tracker] read time out trigger\n");

	if (sta & (1 << CPU_TRACER_CON_IRQ_AW_STA))
		warp_dbg(WARP_DBG_OFF, "[tracker] write time out trigger\n");

	warp_io_write32(tracer, CPU_TRACER_CFG, value);
}
#endif /*WARP_CPU_TRACER*/

int
warp_fwdl_notify_set(struct wed_entry *wed, u32 value)
{
	return 0;
}

int
warp_fwdl_notify_get(struct wed_entry *wed, u32 *value)
{
	return 0;
}

void
warp_conf_hwcap(struct wed_entry *wed)
{
	warp_wed_ver(wed);

	wed->hw_cap = 0;
	if (wed->ver >= 2) {
		wed->hw_cap |= WED_HW_CAP_32K_TXBUF;
#ifdef WED_RX_D_SUPPORT
		wed->hw_cap |= WED_HW_CAP_RX_OFFLOAD;
		wed->hw_cap |= WED_HW_CAP_RX_WOCPU;
#endif
		if (wed->sub_ver)
			wed->hw_cap |= WED_HW_CAP_TXD_PADDING;
	}
}

void
warp_dma_ctrl_hw_after_fwdl(struct wed_entry *wed, u8 txrx)
{

}

void
warp_wed_512_support_hw(struct wed_entry *wed, u8 *enable)
{

}
