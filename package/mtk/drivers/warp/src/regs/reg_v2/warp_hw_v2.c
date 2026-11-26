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
#include <mcu/warp_fwdl.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include "mcu/warp_woif.h"
#include "warp_bm.h"

#define WDMA_MIN_RING_LEN (0x6)
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
#ifdef WED_RX_D_SUPPORT
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw = &wifi->hw;
	struct wed_rx_ring_ctrl *ring_ctrl = &wed->res_ctrl.rx_ctrl.ring_ctrl;
	struct warp_rx_ring *ring;
	int i, j;
	u32 d_cfg;
#endif
	/*reset wed*/
	warp_io_read32(wed, WED_GLO_CFG, &wed_cfg);
	wed_cfg &= ~((1 << WED_GLO_CFG_FLD_TX_DMA_EN) | (1 <<
			WED_GLO_CFG_FLD_RX_DMA_EN));
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &wed_wpdma_cfg);
	wed_wpdma_cfg &= ~((1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN) |
			   (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN) |
			   (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_PKT_PROC) |
			   (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_CRX_SYNC));

	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~(1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
#ifdef WED_RX_D_SUPPORT
	warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &d_cfg);
	d_cfg &= ~(1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
#endif

#ifdef WED_HW_RX_SUPPORT
	wed_wdma_cfg &= ~(1 << WED_WDMA_GLO_CFG_FLD_TX_DDONE_CHK);
#endif

	switch (txrx) {
	case WARP_DMA_TX:
		warp_dbg(WARP_DBG_INF, "%s(): %s DMA TX.\n", __func__,
			 (txrx ? "ENABLE" : "DISABLE"));
		wed_cfg |= (1 << WED_GLO_CFG_FLD_TX_DMA_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);

		/*new txbm, not required to keep token id*/
		wed_wpdma_cfg &= ~(1 << WED_WPDMA_GLO_CFG_FLD_TX_TKID_KEEP);

		wed_wpdma_cfg &= ~(1 << WED_WPDMA_GLO_CFG_FLD_TX_DMAD_DW3_PREV);

		if (IS_WED_HW_CAP(wed, WED_HW_CAP_TXD_PADDING)) {
			u32 wed_wpdma_ctrl = 0;
			/* force HIF_TXD padding due to SDO limitation */
			warp_io_read32(wed, WED_WPDMA_CTRL, &wed_wpdma_ctrl);
			warp_io_write32(wed, WED_WPDMA_CTRL, (wed_wpdma_ctrl | (0x1 << WED_WPDMA_CTRL_FLD_SDL1_FIXED)));
		}

		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
		warp_io_write32(wed, WED_GLO_CFG, wed_cfg);
#ifdef WED_HW_TX_SUPPORT
		warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
		warp_io_write32(wed, WED_WPDMA_GLO_CFG, wed_wpdma_cfg);

#ifdef WED_RX_D_SUPPORT
		warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, d_cfg);
#endif
		break;

	case WARP_DMA_RX:
		warp_dbg(WARP_DBG_INF, "%s(): %s DMA RX.\n", __func__,
			 (txrx ? "ENABLE" : "DISABLE"));
		wed_cfg |= (1 << WED_GLO_CFG_FLD_RX_DMA_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_PKT_PROC);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_CRX_SYNC);

#ifdef WED_RX_D_SUPPORT
		warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &d_cfg);
		d_cfg |= (0x2 << WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_RXEN_SEL);
		d_cfg |= ((hw->max_rxd_size >> 2) << WED_WPDMA_RX_D_GLO_CFG_FLD_RXD_READ_LEN);
		d_cfg |= (1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
#endif

#ifdef WED_HW_RX_SUPPORT
		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_TX_DDONE_CHK);
#endif

		warp_io_write32(wed, WED_GLO_CFG, wed_cfg);
#ifdef WED_HW_TX_SUPPORT
		warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
		warp_io_write32(wed, WED_WPDMA_GLO_CFG, wed_wpdma_cfg);

#ifdef WED_RX_D_SUPPORT
		warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, d_cfg);
#endif
		break;

	case WARP_DMA_TXRX:
		warp_dbg(WARP_DBG_INF, "%s(): %s DMA TXRX.\n", __func__,
			 (txrx ? "ENABLE" : "DISABLE"));
		wed_cfg |= ((1 << WED_GLO_CFG_FLD_TX_DMA_EN) | (1 <<
				WED_GLO_CFG_FLD_RX_DMA_EN));
		wed_wpdma_cfg |= ((1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN) |
				  (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN));

		/*new txbm, not required to keep token id*/
		wed_wpdma_cfg &= ~(1 << WED_WPDMA_GLO_CFG_FLD_TX_TKID_KEEP);

		wed_wpdma_cfg &= ~(1 << WED_WPDMA_GLO_CFG_FLD_TX_DMAD_DW3_PREV);

		if (IS_WED_HW_CAP(wed, WED_HW_CAP_TXD_PADDING)) {
			u32 wed_wpdma_ctrl = 0;
			/* force HIF_TXD padding due to SDO limitation */
			warp_io_read32(wed, WED_WPDMA_CTRL, &wed_wpdma_ctrl);
			warp_io_write32(wed, WED_WPDMA_CTRL, (wed_wpdma_ctrl | (0x1 << WED_WPDMA_CTRL_FLD_SDL1_FIXED)));
		}

		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_TX_DRV_EN);
		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_PKT_PROC);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_RING0_CRX_SYNC);
#ifdef WED_RX_D_SUPPORT

		warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &d_cfg);
		d_cfg |= (0x2 << WED_WPDMA_RX_D_GLO_CFG_FLD_INIT_PHASE_RXEN_SEL);
		d_cfg |= ((hw->max_rxd_size >> 2) << WED_WPDMA_RX_D_GLO_CFG_FLD_RXD_READ_LEN);
		d_cfg |= (1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
#endif

#ifdef WED_HW_RX_SUPPORT
		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_TX_DDONE_CHK);
#endif
		warp_io_write32(wed, WED_GLO_CFG, wed_cfg);

#ifdef WED_HW_TX_SUPPORT
		warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
		warp_io_write32(wed, WED_WPDMA_GLO_CFG, wed_wpdma_cfg);

#ifdef WED_RX_D_SUPPORT
		warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, d_cfg);

#if 1
		for (i = 0; i < ring_ctrl->ring_num; i++) {
			j = 0;
			while(j < 3) {
				mdelay(10);
				ring = &ring_ctrl->ring[i];
				warp_io_read32(wifi, ring->hw_cidx_addr, &d_cfg);
				if (d_cfg == (ring->ring_lens - 1))
					break;
				else
					j++;
			}
		}

		if (j < 3) {
			warp_io_read32(wifi, hw->rx_dma_glo_cfg, &d_cfg);
			d_cfg |= (0x1 << 2);
			warp_io_write32(wifi, hw->rx_dma_glo_cfg, d_cfg);
		} else {
			warp_dbg(WARP_DBG_ERR,
				 "%s(): wfdma rx data dma enable fail\n", __func__);
		}
#endif
#endif
		break;
	default:
		warp_dbg(WARP_DBG_INF,
			 "%s(): Unknown DMA control (%d).\n", __func__, txrx);
		break;
	}
}


#ifdef WED_HW_TX_SUPPORT
/*
*
*/
static void
warp_bfm_txbm_init_hw(struct wed_entry *wed)
{
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	u32 value = 0;

	/*PAUSE BUF MGMT*/
	value = (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	/*should be set before WED_MOD_RST is invoked*/
	value |= (res->bm_vld_grp << WED_TX_BM_CTRL_FLD_VLD_GRP_NUM);
	value |= (res->bm_rsv_grp << WED_TX_BM_CTRL_FLD_RSV_GRP_NUM);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	/*TX BM_BASE*/
	value = res->des_buf.alloc_pa;
	warp_io_write32(wed, WED_TX_BM_BASE, value);
	/*TX packet len*/
	value = (res->pkt_len & WED_TX_BM_BLEN_FLD_BYTE_LEN_MASK);
	warp_io_write32(wed, WED_TX_BM_BLEN, value);
	/*dynamic BM adjust*/
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
		value = (wed->sw_conf->txbm.buf_low << WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM);
		value |= (wed->sw_conf->txbm.buf_high << WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM);
	} else
#endif /* WED_DYNAMIC_TXBW_SUPPORT */
	{
		value = (0 << WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM);
		value |= (WED_TX_BM_DYN_TH_FLD_HI_GRP_MASK << WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM);
	}

	warp_io_write32(wed, WED_TX_BM_DYN_TH, value);
}

/*
*
*/
static void
warp_bfm_tkid_init_hw(struct wed_entry *wed)
{
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	u32 value = 0;

	/*PASUE TKID BUF MGMT*/
	value = (1 << WED_TX_TKID_CTRL_FLD_PAUSE);
	/*should be set before WED_MOD_RST is invoked*/
	value |= (res->tkn_rsv_grp << WED_TX_TKID_CTRL_FLD_RSV_GRP_NUM);
	value |= (res->tkn_vld_grp << WED_TX_TKID_CTRL_FLD_VLD_GRP_NUM);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);
	/*TX token cfg */
	value = ((res->token_start) << WED_TX_TKID_TKID_FLD_START_ID);
	value |= ((res->token_end) << WED_TX_TKID_TKID_FLD_END_ID);
	warp_io_write32(wed, WED_TX_TKID_TKID, value);
	/*dynamic TKID table adjust*/
#ifdef WED_DYNAMIC_TXBM_SUPPORT
	if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
		/*
		value = (wed->sw_conf->txbm.buf_low << WED_TX_TKID_DYN_TH_FLD_LOW_GRP_NUM);
		turn off tkid extend temporarily
		 */
		value = (0 << WED_TX_TKID_DYN_TH_FLD_LOW_GRP_NUM);
		/*
		value |= ((wed->sw_conf->txbm.buf_high << WED_TX_TKID_DYN_TH_FLD_HI_GRP_NUM);
		turn off txbm shrink temporarily
		 */
		value |= (WED_TX_TKID_DYN_TH_FLD_HI_GRP_MASK << WED_TX_TKID_DYN_TH_FLD_HI_GRP_NUM);
	} else
#endif /* WED_DYNAMIC_BW_SUPPORT */
	{
		value = (0 << WED_TX_TKID_DYN_TH_FLD_LOW_GRP_NUM);
		value |= (WED_TX_TKID_DYN_TH_FLD_HI_GRP_MASK << WED_TX_TKID_DYN_TH_FLD_HI_GRP_NUM);
	}

	warp_io_write32(wed, WED_TX_TKID_DYN_TH, value);
}


int
warp_tx_bm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = 0;

	/*initial txbm setting*/
	warp_bfm_txbm_init_hw(wed);
	/*initial tkid setting*/
	warp_bfm_tkid_init_hw(wed);
	/*Reset Buf mgmt for ready to start*/
	value = 1 << WED_MOD_RST_FLD_TX_BM;
	WHNAT_RESET(wed, WED_MOD_RST, value);
	/*Enable agent for BM*/
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_BM_EN);
	value |= (1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);
	/*txbm*/
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	/*tkid*/
	warp_io_read32(wed, WED_TX_TKID_CTRL, &value);
	value &= ~(1 << WED_TX_TKID_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);

	return 0;
}

/*
*
*/
void
warp_bfm_get_tx_freecnt_hw(struct wed_entry *wed, u32 *cnt)
{
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;

	u32 value = 0;
	u32 fcnt = 0;
	u32 ucnt = 0;

#define TOKEN_PER_PAGE 128

	warp_io_read32(wed, WED_TX_TKID_VB_FREE_0_31, &value);
	warp_dbg(WARP_DBG_INF, "WED_TX_TKID_VB_FREE_0_31:(0x%x)\n", value);
	fcnt = (hweight32(value));

	warp_io_read32(wed, WED_TX_TKID_VB_FREE_32_63, &value);
	warp_dbg(WARP_DBG_INF, "WED_TX_TKID_VB_FREE_32_63:(0x%x)\n", value);
	fcnt += (hweight32(value));

	warp_io_read32(wed, WED_TX_TKID_VB_USED_0_31, &value);
	warp_dbg(WARP_DBG_INF, "WED_TX_TKID_VB_USED_0_31:(0x%x)\n", value);
	ucnt = hweight32(value);

	warp_io_read32(wed, WED_TX_TKID_VB_USED_32_63, &value);
	warp_dbg(WARP_DBG_INF, "WED_TX_TKID_VB_USED_32_63:(0x%x)\n", value);
	ucnt += hweight32(value);

	warp_dbg(WARP_DBG_OFF,
		 "One page has 128 tokens. WED always take one page(one token).\n");
	warp_dbg(WARP_DBG_OFF,
		 "Once one of token in a page is used, this page become to non_free page\n");
	warp_dbg(WARP_DBG_OFF,
		 "CR: Free Page(%d), Usage Page(%d), Total page: 64\n", fcnt, ucnt);
	warp_dbg(WARP_DBG_OFF,
		 "CR: Free Cnt(%d),CR: Usage Cnt(%d),Token Cnt(%d)\n",
		 fcnt*TOKEN_PER_PAGE, ucnt*TOKEN_PER_PAGE, buf_res->token_num);
#if 0
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;
	u32 value;
	u32 fcnt = 0;
	u32 cr;
	u32 i;
	u32 cur_cnt = 0;

	value = 1 << WED_DBG_CTRL_FLD_TX_TKID_BM_MEM_DBG;
	warp_io_write32(wed, WED_DBG_CTRL, value);

#define BMF_VALID_TABLE_START (0x00000800)
#define BMF_VALID_TABLE_END (0x00000BFF)

	/*tkid table*/
	for (cr = WED_BMF_VALID_TABLE_START; cr <= WED_BMF_VALID_TABLE_END; cr += 4) {
		warp_io_read32(wed, cr, &value);
		fcnt += hweight32(value);
	}
	/*internal debug probe*/
	value = 0x14;
	warp_io_write32(wed, WED_DBG_CTRL, value);
	warp_io_read32(wed, WED_DBG_PRB1, &value);
	warp_dbg(WARP_DBG_OFF,
		 "Free Token Cnt(%d), CR: Free Cnt(%d), Usage Cnt(%d),Token Cnt(%d)\n",
		 fcnt,
		 ((value >> 16) & 0xffff),
		 (value & 0xffff),
		 buf_res->token_num);
	*cnt = ((value >> 16) & 0xffff);
	/*bm table*/
#define TKF_VALID_TABLE_START (0x00000C00)
#define TKF_VALID_TABLE_END (0x00000FFF)
	fcnt = 0;
	cur_cnt = 0;
	for (i = 0 ; i < 4; i++) {
		value = (1 << WED_DBG_CTRL_FLD_TX_TKID_BM_MEM_DBG) |
				(i << WED_DBG_CTRL_FLD_TX_BM_MEM_PGSEL);
		warp_io_write32(wed, WED_DBG_CTRL, value);
		for (cr = TKF_VALID_TABLE_START ; cr <= TKF_VALID_TABLE_END; cr += 4) {
			warp_io_read32(wed, cr, &value);
			fcnt += hweight32(value);
			cur_cnt += 32;
		}
		if (cur_cnt >= buf_res->pkt_num)
			break;
	}
	*cnt = fcnt;
	warp_dbg(WARP_DBG_OFF,
		 "Free SKB Cnt(%d), Usage Cnt(%d),Pkt Cnt(%d)\n",
		 fcnt,
		 buf_res->pkt_num - fcnt,
		 buf_res->pkt_num);
	/*Disable debug*/
	warp_io_write32(wed, WED_DBG_CTRL, 0);
#endif
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
	value = 1 << WED_DBG_CTRL_FLD_TX_TKID_BM_MEM_DBG;
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
	u32 value = 0;
	u32 cnt = 0;
	/*PAUSE BUF MGMT*/
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value |= (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	/*POLL status bit*/
	warp_io_read32(wed, WED_ST, &value);

	while ((value & (0xf << WED_ST_FLD_TX_ST)) && cnt < WED_POLL_MAX) {
		warp_io_read32(wed, WED_ST, &value);
		cnt++;
	}

	if (cnt == WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): can't poll tx buf status to clear!\n", __func__);
		goto end;
	} else
		warp_dbg(WARP_DBG_INF, "%s(): polling %d time(s) to idle!\n", __func__, cnt);

	/*update token*/
	value = (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	value |= (res->bm_rsv_grp << WED_TX_BM_CTRL_FLD_RSV_GRP_NUM);
	value |= (res->bm_vld_grp << WED_TX_BM_CTRL_FLD_VLD_GRP_NUM);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
	warp_dbg(WARP_DBG_INF, "%s(): update packet buffer done!\n", __func__);

end:
	/*Disable PAUSE BUF MGMT*/
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);
}


/*
*
*/
void
warp_btkn_update_hw(struct wed_entry *wed, u8 reduce)
{
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	u32 value = 0;
	u32 cnt = 0;
	/*PAUSE BUF MGMT*/
	warp_io_read32(wed, WED_TX_TKID_CTRL, &value);
	value |= (1 << WED_TX_TKID_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);
	/*POLL status bit*/
	warp_io_read32(wed, WED_ST, &value);

	while ((value & (0xf << WED_ST_FLD_TX_ST)) && cnt < WED_POLL_MAX) {
		warp_io_read32(wed, WED_ST, &value);
		cnt++;
	}

	if (cnt == WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): can't poll tx buf status to clear!\n", __func__);
		goto end;
	} else
		warp_dbg(WARP_DBG_INF, "%s(): polling %d time(s) to idle!\n", __func__, cnt);

	/*update token*/
	value = (1 << WED_TX_TKID_CTRL_FLD_PAUSE);
	value |= (res->tkn_rsv_grp << WED_TX_TKID_CTRL_FLD_RSV_GRP_NUM);
	value |= (res->tkn_vld_grp << WED_TX_TKID_CTRL_FLD_VLD_GRP_NUM);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);
	warp_dbg(WARP_DBG_INF, "%s(): update token done!\n", __func__);

end:
	/*Disable PAUSE BUF MGMT*/
	warp_io_read32(wed, WED_TX_TKID_CTRL, &value);
	value &= ~(1 << WED_TX_TKID_CTRL_FLD_PAUSE);
	warp_io_write32(wed, WED_TX_TKID_CTRL, value);
}

/*
 *
 */
u32
warp_get_recycle_grp_idx(struct wed_entry *wed)
{
	u32 value;

	warp_io_read32(wed, WED_TX_BM_RECYC, &value);
	value &= 0xff;

	return value;
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
	int ret = 0;

	warp_io_read32(wed, WED_GLO_CFG, &value);
	value &= ~(1 << WED_GLO_CFG_FLD_TX_DMA_EN);
	warp_io_write32(wed, WED_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_GLO_CFG, WED_GLO_CFG_FLD_TX_DMA_BUSY) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_tx_dma is busy\n", __func__);
		ret = -1;
	}
	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
		value = (1 << WED_RST_IDX_FLD_DTX_IDX0);
		value |= (1 << WED_RST_IDX_FLD_DTX_IDX1);
		warp_io_write32(wed, WED_RST_IDX, value);
		warp_io_write32(wed, WED_RST_IDX, 0);
	} else {
		value = (1 << WED_MOD_RST_FLD_WED_TX_DMA);
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif
	return ret;
}



#ifdef WED_HW_TX_SUPPORT
/*
*
*/
static int
reset_wdma_rx(struct wdma_entry *wdma)
{
	u32 value;
	int ret = 0;
	/* Stop Frame Engin WDMA */
	warp_io_read32(wdma, WDMA_GLO_CFG, &value);
	value &= ~(1 << WDMA_GLO_CFG_FLD_RX_DMA_EN);
	warp_io_write32(wdma, WDMA_GLO_CFG, value);

	if (wdma_agt_dis_ck(wdma, WDMA_GLO_CFG, WDMA_GLO_CFG_FLD_RX_DMA_BUSY) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wdma_rx is busy\n", __func__);
		ret = -1;
	}

	/* Reset Frame Engine WDMA DRX/CRX index */
	warp_io_read32(wdma, WDMA_RST_IDX, &value);
	value = 1 << WDMA_RST_IDX_FLD_RST_DRX_IDX0;
	value |= 1 << WDMA_RST_IDX_FLD_RST_DRX_IDX1;
	warp_io_write32(wdma, WDMA_RST_IDX, value);
	warp_io_read32(wdma, WDMA_RST_IDX, &value);
	value &= ~(1 << WDMA_RST_IDX_FLD_RST_DRX_IDX0);
	value &= ~(1 << WDMA_RST_IDX_FLD_RST_DRX_IDX1);
	warp_io_write32(wdma, WDMA_RST_IDX, value);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_0, 0);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_1, 0);
	return ret;
}
#endif

#ifdef WED_HW_RX_SUPPORT
static int
reset_wdma_tx(struct wdma_entry *wdma)
{
	u32 value;
	int ret = 0;
	/* Stop Frame Engin WDMA */
	warp_io_read32(wdma, WDMA_GLO_CFG, &value);
	value &= ~(1 << WDMA_GLO_CFG_FLD_TX_DMA_EN);
	warp_io_write32(wdma, WDMA_GLO_CFG, value);

	if (wdma_agt_dis_ck(wdma, WDMA_GLO_CFG, WDMA_GLO_CFG_FLD_TX_DMA_BUSY) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wdma_tx is busy\n", __func__);
		ret = -1;
	}
	/* Reset Frame Engine WDMA DTX/CTX index*/
	warp_io_read32(wdma, WDMA_RST_IDX, &value);
	value = 1 << WDMA_RST_IDX_FLD_RST_DTX_IDX0;
	value |= 1 << WDMA_RST_IDX_FLD_RST_DTX_IDX1;
	warp_io_write32(wdma, WDMA_RST_IDX, value);
	warp_io_read32(wdma, WDMA_RST_IDX, &value);
	value &= ~(1 << WDMA_RST_IDX_FLD_RST_DTX_IDX0);
	value &= ~(1 << WDMA_RST_IDX_FLD_RST_DTX_IDX1);
	warp_io_write32(wdma, WDMA_RST_IDX, value);
	warp_io_write32(wdma, WDMA_TX_CTX_IDX_0, 0);
	warp_io_write32(wdma, WDMA_TX_CTX_IDX_1, 0);
	return ret;
}


/*
*
*/
static void
wdma_dma_ctrl(struct wdma_entry *wdma, u8 txrx)
{
	u32 wdma_cfg = 0;

	warp_io_read32(wdma, WDMA_GLO_CFG, &wdma_cfg);
	warp_dbg(WARP_DBG_OFF, "%s(): WDMA_GLO_CFG=%x, txrx = %d\n",  __func__, wdma_cfg, txrx);

	if (txrx) {
		/*reset wdma*/
		wdma_cfg |= (1 << WDMA_GLO_CFG_FLD_RX_INFO1_PRESERVE);
		wdma_cfg |= (1 << WDMA_GLO_CFG_FLD_RX_INFO2_PRESERVE);

#ifdef WED_HW_RX_SUPPORT
		if ((txrx == WARP_DMA_RX) || (txrx == WARP_DMA_TXRX))
			wdma_cfg |= (1 << WDMA_GLO_CFG_FLD_TX_DMA_EN);
#endif
	} else {
		wdma_cfg &= ~(1 << WDMA_GLO_CFG_FLD_RX_INFO1_PRESERVE);
		wdma_cfg &= ~(1 << WDMA_GLO_CFG_FLD_RX_INFO2_PRESERVE);

#ifdef WED_HW_RX_SUPPORT
		wdma_cfg &= ~(1 << WDMA_GLO_CFG_FLD_TX_DMA_EN);
#endif
	}

	warp_io_write32(wdma, WDMA_GLO_CFG, wdma_cfg);
}
#endif

#if defined(CONFIG_WARP_HW_DDR_PROF)
static int configure_mptr_dram_stat(struct wed_entry *wed, u8 direction)
{
	u32 pmtr_tgt = 0, pmtr_tgt_st = 0, pmtr_ctrl = 0;

	/* Configure PMTR for read DRAM state */
	warp_io_read32(wed, WED_PMTR_TGT, &pmtr_tgt);
	if (direction == WED_DRAM_PROF_TX) {
		pmtr_tgt &= (0x1f << WED_PMTR_TGT_FLD_MST0);	/* clear first*/
		pmtr_tgt |= (0x8 << WED_PMTR_TGT_FLD_MST0); 	/* set 0x8 */
	} else {
		pmtr_tgt &= (0x1f << WED_PMTR_TGT_FLD_MST1);	/* clear first*/
		pmtr_tgt |= (0x5 << WED_PMTR_TGT_FLD_MST1); 	/* set 0x5 */
	}
	warp_io_write32(wed, WED_PMTR_TGT, pmtr_tgt);

	warp_io_read32(wed, WED_PMTR_TGT_ST, &pmtr_tgt_st);
	if (direction == WED_DRAM_PROF_TX) {
		pmtr_tgt_st &= (0xff << WED_PMTR_TGT_ST_FLD_MST0);	/* clear first*/
		pmtr_tgt_st |= (0x6 << WED_PMTR_TGT_ST_FLD_MST0);	/* set 0x6 */
	} else {
		pmtr_tgt_st &= (0xff << WED_PMTR_TGT_ST_FLD_MST1);	/* clear first*/
		pmtr_tgt_st |= (0x6 << WED_PMTR_TGT_ST_FLD_MST1);	/* set 0x6 */
	}
	warp_io_write32(wed, WED_PMTR_TGT_ST, pmtr_tgt_st);
	/* Specify PMTR count unit */
	warp_io_read32(wed, WED_PMTR_CTRL, &pmtr_ctrl);
	pmtr_ctrl &= ~(0xf << WED_PMTR_CTRL_FLD_MAX_DIV);	/* clear */
	pmtr_ctrl &= ~(0xf << WED_PMTR_CTRL_FLD_ACC_DIV);	/* clear first */
	/* for ASIC */
	pmtr_ctrl |= (0x2 << WED_PMTR_CTRL_FLD_ACC_DIV);	/* set 0x2 */
	/* for FPGA remian ACC_DIV = 0*/
	warp_io_write32(wed, WED_PMTR_CTRL, pmtr_ctrl);

	return 0;
}

static int clear_pmtr_counter(struct wed_entry *wed)
{
	/* Clear PMTR counter */
	warp_io_write32(wed, WED_PMTR_LTCY_MAX0_1, 0);
	warp_io_write32(wed, WED_PMTR_LTCY_MAX2_3, 0);
	warp_io_write32(wed, WED_PMTR_LTCY_ACC0, 0);
	warp_io_write32(wed, WED_PMTR_LTCY_ACC1, 0);
	warp_io_write32(wed, WED_PMTR_LTCY_ACC2, 0);
	warp_io_write32(wed, WED_PMTR_LTCY_ACC3, 0);

	return 0;
}

#if defined(WED_HW_TX_SUPPORT)
static int restart_wed_tx_prof(struct wed_entry *wed)
{
	u32 wed_wdma_cfg = 0, pmtr_ctrl = 0;

	/* Disable WED_WDMA_RX_DRV */
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~(1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
	/* Disable PMTR */
	warp_io_read32(wed, WED_PMTR_CTRL, &pmtr_ctrl);
	pmtr_ctrl &= ~(1 << WED_PMTR_CTRL_FLD_EN);
	warp_io_write32(wed, WED_PMTR_CTRL, pmtr_ctrl);

	configure_mptr_dram_stat(wed, WED_DRAM_PROF_TX);

	clear_pmtr_counter(wed);

	/* Clear WED_WDMA_RX_DRV packet counter */
	warp_io_write32(wed, WED_WDMA_RX0_PROCESSED_MIB, 0);
	warp_io_write32(wed, WED_WDMA_RX1_PROCESSED_MIB, 0);
	/* Enable WED_WDMA_RX_DRV */
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
	/* Enable PMTR */
	warp_io_read32(wed, WED_PMTR_CTRL, &pmtr_ctrl);
	pmtr_ctrl |= (1 << WED_PMTR_CTRL_FLD_EN);
	warp_io_write32(wed, WED_PMTR_CTRL, pmtr_ctrl);

	return 0;
}
#endif	/* WED_HW_TX_SUPPORT */

#if defined(WED_HW_RX_SUPPORT)
static int restart_wed_rx_prof(struct wed_entry *wed)
{
	u32 wed_wpdma_cfg = 0, pmtr_ctrl = 0;

	/* Disable WED_WPDMA_RX_D_DRV */
	warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &wed_wpdma_cfg);
	wed_wpdma_cfg &= ~(1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, wed_wpdma_cfg);
	/* Disable PMTR */
	warp_io_read32(wed, WED_PMTR_CTRL, &pmtr_ctrl);
	pmtr_ctrl &= ~(1 << WED_PMTR_CTRL_FLD_EN);
	warp_io_write32(wed, WED_PMTR_CTRL, pmtr_ctrl);

	configure_mptr_dram_stat(wed, WED_DRAM_PROF_RX);

	clear_pmtr_counter(wed);

	/* Clear WED_WPDMA_RX_D_DRV packet counter */
	warp_io_write32(wed, WED_WPDMA_RX_D_RX0_PROCESSED_MIB, 0);
	warp_io_write32(wed, WED_WPDMA_RX_D_RX1_PROCESSED_MIB, 0);
	/* Enable WED_WPDMA_RX_D_DRV */
	warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &wed_wpdma_cfg);
	wed_wpdma_cfg |= (1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, wed_wpdma_cfg);
	/* Enable PMTR */
	warp_io_read32(wed, WED_PMTR_CTRL, &pmtr_ctrl);
	pmtr_ctrl |= (1 << WED_PMTR_CTRL_FLD_EN);
	warp_io_write32(wed, WED_PMTR_CTRL, pmtr_ctrl);

	return 0;
}
#endif /* WED_HW_RX_SUPPORT */

static int stop_wed_ddr_prof(struct wed_entry *wed)
{
	u32 wed_wdma_cfg = 0, pmtr_ctrl = 0, wed_wpdma_cfg = 0;

	/* Disable WED_WPDMA_RX_D_DRV */
	warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~(1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, wed_wdma_cfg);
	/* Disable WED_WDMA_RX_DRV */
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wpdma_cfg);
	wed_wpdma_cfg &= ~(1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wpdma_cfg);
	/* Disable PMTR */
	warp_io_read32(wed, WED_PMTR_CTRL, &pmtr_ctrl);
	pmtr_ctrl &= ~(1 << WED_PMTR_CTRL_FLD_EN);
	warp_io_write32(wed, WED_PMTR_CTRL, pmtr_ctrl);

	clear_pmtr_counter(wed);

	/* Clear WED_WPDMA_RX_D_DRV packet counter */
	warp_io_write32(wed, WED_WPDMA_RX_D_RX0_PROCESSED_MIB, 0);
	warp_io_write32(wed, WED_WPDMA_RX_D_RX1_PROCESSED_MIB, 0);
	/* Enable WED_WDMA_RX_DRV */
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
	/* Enable WED_WPDMA_RX_D_DRV */
	warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &wed_wpdma_cfg);
	wed_wpdma_cfg |= (1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, wed_wpdma_cfg);

	return 0;
}


int wed_dram_prof(struct wed_entry *wed, u8 direction)
{
	int ret = -1;
#if defined(WED_HW_TX_SUPPORT)
	if (direction == WED_DRAM_PROF_TX)
		ret = restart_wed_tx_prof(wed);
#endif

#if defined(WED_HW_RX_SUPPORT)
	if (direction == WED_DRAM_PROF_RX)
		ret = restart_wed_rx_prof(wed);
#endif	/* WED_HW_RX_SUPPORT */

	if (direction == WED_DRAM_PROF_OFF)
		ret = stop_wed_ddr_prof(wed);

	if (ret)
		warp_dbg(WARP_DBG_ERR, "%s(): not supported!\n", __func__);

	return ret;
}
#endif	/* CONFIG_WARP_HW_DDR_PROF && CONFIG_WARP_HW_DDR_PROF */


/*
*
*/
static int reset_wed_rx_drv(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_HW_TX_SUPPORT
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
	u32 value;
	int ret = 0;

	/*Stop Frame Engin WDMA*/
	warp_io_read32(wdma, WDMA_GLO_CFG, &value);
	value &= ~(1 << WDMA_GLO_CFG_FLD_RX_DMA_EN);
	warp_io_write32(wdma, WDMA_GLO_CFG, value);

	if (wdma_agt_dis_ck(wdma, WDMA_GLO_CFG, WDMA_GLO_CFG_FLD_RX_DMA_BUSY) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_drv is busy\n", __func__);
		ret = -1;
	}
	/*Stop WED WDMA Rx Driver Engine*/
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &value);
	value |= (1 << WED_WDMA_GLO_CFG_FLD_RXDRV_DISABLED_FSM_AUTO_IDLE);
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
		ret = -1;
	}
#ifdef WED_WORK_AROUND_WDMA_RETURN_IDLE
	value &= ~(1 << WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, value);
#endif /*WED_WORK_AROUND_WDMA_RETURN_IDLE*/
	/*Reset Frame Engine WDMA DRX/CRX index*/
	value = 1 << WDMA_RST_IDX_FLD_RST_DRX_IDX0;
	value |= 1 << WDMA_RST_IDX_FLD_RST_DRX_IDX1;
	warp_io_write32(wdma, WDMA_RST_IDX, value);
	warp_io_write32(wdma, WDMA_RST_IDX, 0);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_0, 0);
	warp_io_write32(wdma, WDMA_RX_CRX_IDX_1, 0);

	/*Reset WED WDMA RX Driver Engin DRV/CRX index only*/
	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
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
		warp_io_read32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
		warp_io_write32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WDMA_INT_AGT_BUSY) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s():WED_CTRL_FLD_WDMA_INT_AGT_BUSY is busy!\n", __func__);
			ret = -1;
		}
		value = 1 << WED_MOD_RST_FLD_WDMA_INT_AGT;
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*Reset WED RX Driver Engin*/
		value = 1 << WED_MOD_RST_FLD_WDMA_RX_DRV;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_HW_TX_SUPPORT*/
	return ret;
}



/*
*
*/
static int reset_wed_tx_bm(struct wed_entry *wed)
{
#ifdef WED_HW_TX_SUPPORT
	u32 value;
	u32 cnt = 0;
	int ret = 0;
	/*Tx Free Agent Reset*/
	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	warp_io_write32(wed, WED_CTRL, value);

	if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WED_TX_FREE_AGT_BUSY) < 0) {
		warp_dbg(WARP_DBG_OFF, "%s(): tx free agent reset is busy!!\n", __func__);
		ret = -1;
	}

	warp_io_read32(wed, WED_TX_TKID_INTF, &value);

	while (((value >> 16) & 0x3ff) != 0x200 && cnt < WED_POLL_MAX) {
		warp_io_read32(wed, WED_TX_TKID_INTF, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_OFF, "%s(): tx free agent fifo reset faild!\n", __func__);
		ret = -1;
	}

	value = 1 << WED_MOD_RST_FLD_TX_FREE_AGT;
	WHNAT_RESET(wed, WED_MOD_RST, value);
	/*Reset TX Buffer manager*/
	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_TX_BM_EN);
	warp_io_write32(wed, WED_CTRL, value);

	if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WED_TX_BM_BUSY) < 0) {
		warp_dbg(WARP_DBG_OFF, "%s(): tx bm reset is busy!!\n", __func__);
		ret = -1;
	}

	value = 1 << WED_MOD_RST_FLD_TX_BM;
	WHNAT_RESET(wed, WED_MOD_RST, value);
#endif /*WED_HW_TX_SUPPORT*/
	return ret;
}



/*
*
*/
static int
reset_wed_tx_drv(struct wed_entry *wed, u32 reset_type)
{
#ifdef WED_TX_SUPPORT
	u32 value = 0;
	int ret = 0;
	/*Disable TX driver*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &value);
	value &= ~(1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WPDMA_GLO_CFG,
			   WED_WPDMA_GLO_CFG_FLD_TX_DRV_BUSY) < 0) {
		warp_dbg(WARP_DBG_OFF, "%s(): reset_wed_tx_drv is busy, force reset!!\n", __func__);
		ret = -1;
	}
	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
		/*Reset TX Ring only*/
		value = (1 << WED_WPDMA_RST_IDX_FLD_CTX_IDX0);
		value |= (1 << WED_WPDMA_RST_IDX_FLD_CTX_IDX1);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, value);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, 0);
	} else {
		warp_io_read32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		warp_io_write32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WPDMA_INT_AGT_BUSY) < 0) {
			warp_dbg(WARP_DBG_OFF, "%s(): WED_CTRL_FLD_WPDMA_INT_AGT_BUSY is busy!!\n", __func__);
			ret = -1;
		}
		value = 1 << WED_MOD_RST_FLD_WPDMA_INT_AGT;
		WHNAT_RESET(wed, WED_MOD_RST, value);

		value = 1 << WED_MOD_RST_FLD_WPDMA_TX_DRV;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_TX_SUPPORT*/
	return ret;
}


/*
*
*/
static int
reset_tx_traffic(struct wed_entry *wed, u32 reset_type)
{
	int ret = 0;
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_moudle_busy_cnt *busy_cnt = &ser_ctrl->ser_busy_cnt;

	/*host tx dma*/
	ret = reset_wed_tx_dma(wed, reset_type);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_tx_dma is busy do module reset\n", __func__);
		busy_cnt->reset_wdma_tx++;
	}

	/*wdma rx dma and rx driver*/
	ret = reset_wed_rx_drv(wed, reset_type);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_wdma_rx_drv is busy do module reset\n", __func__);
		busy_cnt->reset_wed_wdma_rx_drv++;
	}

	/*wed txbm*/
	ret = reset_wed_tx_bm(wed);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_tx_bm is busy\n", __func__);
		busy_cnt->reset_wed_tx_bm++;
	}

	/*wpdma tx driver*/
	ret = reset_wed_tx_drv(wed, reset_type);
	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): wed_tx_drv is busy do module reset\n", __func__);
		busy_cnt->reset_wed_wpdma_tx_drv++;
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
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_moudle_busy_cnt *busy_cnt = &ser_ctrl->ser_busy_cnt;

	u32 value = 0;
	int ret = 0;
	/*disable WPDMA RX Driver Engine*/
	warp_io_read32(wed, WED_WPDMA_GLO_CFG, &value);
	value &= ~(1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WPDMA_GLO_CFG,
			   WED_WPDMA_GLO_CFG_FLD_RX_DRV_BUSY) < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_rx_traffic is busy, force reset\n", __func__);
		ret = -1;
	}
	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
		/* Reset WPDMA side index */
		warp_io_read32(wed, WED_WPDMA_RST_IDX, &value);
		value = (1 << WED_WPDMA_RST_IDX_FLD_CRX_IDX1);
		value |= (1 << WED_WPDMA_RST_IDX_FLD_CRX_IDX0);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, value);
		warp_io_read32(wed, WED_WPDMA_RST_IDX, &value);
		value &= ~(1 << WED_WPDMA_RST_IDX_FLD_CRX_IDX1);
		value &= ~(1 << WED_WPDMA_RST_IDX_FLD_CRX_IDX0);
		warp_io_write32(wed, WED_WPDMA_RST_IDX, value);

		/* Reset mirror to host CPU side index */
		warp_io_read32(wed, WED_RST_IDX, &value);
		value = (1 << WED_RST_IDX_FLD_WPDMA_DRX_IDX0);
		value |= (1 << WED_RST_IDX_FLD_WPDMA_DRX_IDX1);
		warp_io_write32(wed, WED_RST_IDX, value);
		warp_io_read32(wed, WED_RST_IDX, &value);
		value &= ~(1 << WED_RST_IDX_FLD_WPDMA_DRX_IDX0);
		value &= ~(1 << WED_RST_IDX_FLD_WPDMA_DRX_IDX1);
		warp_io_write32(wed, WED_RST_IDX, value);
		/* WED Tx free done ring1 - it should not be set.
		warp_io_write32(wed, WED_RX1_CTRL2, 0);
		*/
	} else {
		/*WPDMA  interrupt agent*/
		warp_io_read32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		warp_io_write32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WPDMA_INT_AGT_BUSY) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): WARP_RESET_IDX_MODULE: reset_rx_traffic is busy\n", __func__);
			ret = -1;
		}
		value = (1 << WED_MOD_RST_FLD_WPDMA_INT_AGT);
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*WPDMA RX Driver Engin*/
		value = (1 << WED_MOD_RST_FLD_WPDMA_RX_DRV);
		WHNAT_RESET(wed, WED_MOD_RST, value);
		warp_io_write32(wed, WED_RX1_CTRL2, 0);
	}
	if (ret < 0)
		busy_cnt->reset_wed_rx_drv++;
#endif /*WED_RX_SUPPORT*/
	return ret;
}


#ifdef WED_RX_D_SUPPORT
static int reset_wed_rx_d_drv(struct wed_entry *wed, u32 reset_type)
{
	int cnt = 0;
	u32 value = 0;
	int ret = 0;

	warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &value);
	value &= ~(1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_EN);
	warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, value);

	do {
		udelay(100);
		warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &value);

		if (!(value & (1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RX_DRV_BUSY)))
			break;
	} while (cnt++ < WED_POLL_MAX);

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_d_drv is busy, force reset\n", __func__);
		ret = -1;
	}
	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
		warp_io_read32(wed, WED_WPDMA_RX_D_RST_IDX, &value);
		value |= (1 << WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX0);
		value |= (1 << WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX1);
		value |= (1 << WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX0);
		value |= (1 << WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX1);
		warp_io_write32(wed, WED_WPDMA_RX_D_RST_IDX, value);
		warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &value);
		value |= (1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG);
		warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, value);
		warp_io_read32(wed, WED_WPDMA_RX_D_GLO_CFG, &value);
		value |= (1 << WED_WPDMA_RX_D_GLO_CFG_FLD_FSM_RETURN_IDLE);
		warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, value);
		value &= ~(1 << WED_WPDMA_RX_D_GLO_CFG_FLD_FSM_RETURN_IDLE);
		value &= ~(1 << WED_WPDMA_RX_D_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG);
		warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG, value);
		warp_io_write32(wed, WED_WPDMA_RX_D_RST_IDX, 0);
	} else {
		warp_io_read32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		warp_io_write32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WPDMA_INT_AGT_BUSY) < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): WED_CTRL_FLD_WPDMA_INT_AGT_BUSY is busy\n", __func__);
			ret = -1;
		}
		value = 1 << WED_MOD_RST_FLD_WPDMA_INT_AGT;
		WHNAT_RESET(wed, WED_MOD_RST, value);

		value = 1 << WED_MOD_RST_FLD_WPDMA_RX_D_DRV;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

	return ret;
}

static int reset_wed_rx_rro_qm(struct wed_entry *wed, u32 reset_type)
{
	int cnt = 0;
	u32 value = 0;
	int ret = 0;

	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_RX_RRO_QM_EN);
	warp_io_write32(wed, WED_CTRL, value);

	do {
		udelay(100);
		warp_io_read32(wed, WED_CTRL, &value);

		if (!(value & (1 << WED_CTRL_FLD_WED_RX_RRO_QM_BUSY)))
			break;
	} while (cnt++ < WED_POLL_MAX);

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_rro_qm is busy, force reset\n", __func__);
		ret = -1;
	}
	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
		warp_io_read32(wed, WED_RROQM_RST_IDX, &value);
		value |= (1 << WED_RROQM_RST_IDX_FLD_FDBK);
		value |= (1 << WED_RROQM_RST_IDX_FLD_MIOD);
		warp_io_write32(wed, WED_RROQM_RST_IDX, value);
		warp_io_write32(wed, WED_RROQM_RST_IDX, 0);
	} else {
		value = 1 << WED_MOD_RST_FLD_RX_RRO_QM;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

	return ret;
}

static int reset_wed_rx_route_qm(struct wed_entry *wed, u32 reset_type)
{
	int cnt = 0;
	u32 value;
	int ret = 0;

	/* wait for there is no traffic from ppe/rro */
	do {
		udelay(100);
		warp_io_read32(wed, WED_CTRL, &value);

		if (!(value & (1 << WED_CTRL_FLD_WED_RX_ROUTE_QM_BUSY)))
			break;
	} while (cnt++ < WED_POLL_MAX);

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_route_qm is busy, force reset\n", __func__);
		ret = -1;
	}
	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_RX_ROUTE_QM_EN);
	warp_io_write32(wed, WED_CTRL, value);

	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
		warp_io_read32(wed, WED_RTQM_GLO_CFG, &value);
		value |= (1 << WED_RTQM_GLO_CFG_FLD_Q_RST);
		warp_io_write32(wed, WED_RTQM_GLO_CFG, value);

		warp_io_read32(wed, WED_RTQM_GLO_CFG, &value);
		value &= ~(1 << WED_RTQM_GLO_CFG_FLD_Q_RST);
		warp_io_write32(wed, WED_RTQM_GLO_CFG, value);
	} else {
		value = 1 << WED_MOD_RST_FLD_RX_ROUTE_QM;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

	return ret;
}


static int reset_wed_rx_dma(struct wed_entry *wed, u32 reset_type)
{
	int cnt = 0;
	u32 value;
	int ret = 0;

	do {
		udelay(100);
		warp_io_read32(wed, WED_GLO_CFG, &value);

		if (!(value & WED_GLO_CFG_FLD_RX_DMA_BUSY))
			break;
	} while (cnt++ < WED_POLL_MAX);

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_dma is busy, force reset\n", __func__);
		ret = -1;
	}
	warp_io_read32(wed, WED_GLO_CFG, &value);
	value &= ~(1 << WED_GLO_CFG_FLD_RX_DMA_EN);
	warp_io_write32(wed, WED_GLO_CFG, value);

	if (reset_type == WARP_RESET_IDX_ONLY && ret == 0) {
		warp_io_read32(wed, WED_RST_IDX, &value);
		value |= (1 << WED_RST_IDX_FLD_DRX_IDX0);
		value |= (1 << WED_RST_IDX_FLD_DRX_IDX1);
		warp_io_write32(wed, WED_RST_IDX, value);
		warp_io_read32(wed, WED_RST_IDX, &value);
		value &= ~(1 << WED_RST_IDX_FLD_DRX_IDX0);
		value &= ~(1 << WED_RST_IDX_FLD_DRX_IDX1);
		warp_io_write32(wed, WED_RST_IDX, value);
	} else {
		value = 1 << WED_MOD_RST_FLD_WED_RX_DMA;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}
	return ret;
}



static int reset_wed_rx_bm(struct wed_entry *wed, u32 reset_type)
{
	int cnt = 0;
	u32 value;
	int ret = 0;

	warp_io_read32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_RX_BM_EN);
	warp_io_write32(wed, WED_CTRL, value);

	do {
		udelay(100);
		warp_io_read32(wed, WED_CTRL, &value);

		if (!(value & (1 << WED_CTRL_FLD_WED_RX_BM_BUSY)))
			break;
	} while (cnt++ < WED_POLL_MAX);

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_bm is busy\n", __func__);
		ret = -1;
	}
	warp_io_read32(wed, WED_MOD_RST, &value);
	value |= (1 << WED_MOD_RST_FLD_RX_BM);
	warp_io_write32(wed, WED_MOD_RST, value);
	warp_io_read32(wed, WED_MOD_RST, &value);
	value &= ~(1 << WED_MOD_RST_FLD_RX_BM);
	warp_io_write32(wed, WED_MOD_RST, value);

	return ret;
}


static int reset_wdma_tx_drv(struct wed_entry *wed, u32 reset_type)
{
	u32 value = 0;
	int cnt = 0;
	int ret = 0;

	warp_io_read32(wed, WED_WDMA_GLO_CFG, &value);
	value &= ~(1 << WED_WDMA_GLO_CFG_FLD_TX_DRV_EN);
	warp_io_write32(wed, WED_WDMA_GLO_CFG, value);

	do {
		udelay(100);
		warp_io_read32(wed, WED_CTRL, &value);

		if (!(value & (1 << WED_CTRL_FLD_WDMA_INT_AGT_BUSY)))
			break;
	} while (cnt++ < WED_POLL_MAX);

	if (cnt >= WED_POLL_MAX) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wdma_tx_drv busy check fail,FE reset?\n", __func__);
		ret = -1;
	}
	warp_io_read32(wed, WED_MOD_RST, &value);
	value |= (1 << WED_MOD_RST_FLD_WDMA_TX_DRV);
	warp_io_write32(wed, WED_MOD_RST, value);
	warp_io_read32(wed, WED_MOD_RST, &value);
	value &= ~(1 << WED_MOD_RST_FLD_WDMA_TX_DRV);
	warp_io_write32(wed, WED_MOD_RST, value);

	return ret;
}


static int
reset_rx_d_traffic(struct wed_entry *wed, u32 reset_type)
{
	struct warp_entry *warp = wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_moudle_busy_cnt *busy_cnt = &ser_ctrl->ser_busy_cnt;

	int ret = 0;

	/* WOCPU Enter SER */
#ifdef WED_RX_HW_RRO_2_0
	warp_woctrl_enter_state((struct warp_entry *)wed->warp, WO_STATE_SER_RESET);
	warp_dbg(WARP_DBG_ERR, "%s(): WOCPU Enter SER\n", __func__);
#endif

	ret = reset_wed_rx_d_drv(wed, reset_type);

	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_d_drv busy check fail,ret=%d\n", __func__, ret);
		busy_cnt->reset_wed_wpdma_rx_d_drv++;
	}

	ret = reset_wed_rx_rro_qm(wed, reset_type);

	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_rro_qm busy check fail,ret=%d\n", __func__, ret);
		busy_cnt->reset_wed_rx_rro_qm++;
	}

	ret = reset_wed_rx_route_qm(wed, reset_type);

	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_route_qm busy check fail,ret=%d\n", __func__, ret);
		busy_cnt->reset_wed_rx_route_qm++;
	}

	ret = reset_wdma_tx(wdma);

	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wdma_tx busy check fail,ret=%d\n", __func__, ret);
		busy_cnt->reset_wdma_tx++;
	}

	ret = reset_wdma_tx_drv(wed, reset_type);

	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_wdma_tx_drv busy check fail,ret=%d\n", __func__, ret);
		busy_cnt->reset_wdma_tx_drv++;
	}

	ret = reset_wed_rx_dma(wed, reset_type);

	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_dma fail busy check fail,ret=%d\n", __func__, ret);
		busy_cnt->reset_wed_rx_dma++;
	}

	ret = reset_wed_rx_bm(wed, reset_type);

	if (ret < 0) {
		warp_dbg(WARP_DBG_ERR, "%s(): reset_wed_rx_bm fail busy check fail,ret=%d\n", __func__, ret);
		busy_cnt->reset_wed_rx_bm++;
	}

	/* WOCPU Enter SER */
#ifdef WED_RX_HW_RRO_2_0
	warp_woctrl_exit_state((struct warp_entry *)wed->warp, WO_STATE_SER_RESET);
	warp_dbg(WARP_DBG_ERR, "%s(): WOCPU Exit SER\n", __func__);
#endif

	return ret;
}

#endif

/*
*
*/
static void
reset_interface(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 value = 0;
	u32 wo_heartbeat = 0;
	u32 check_wo_heart_cnt = 0;
	struct warp_entry *warp = wed->warp;

#ifdef WED_HW_TX_SUPPORT
	/* Reset WDMA RX */
	reset_wdma_rx(wdma);
#endif
	/* Reset WED */
	value = 1 << WED_MOD_RST_FLD_WED;
	WHNAT_RESET(wed, WED_MOD_RST, value);

#ifdef WED_RX_HW_RRO_2_0
	if (warp->woif.wo_ctrl.cur_state == WO_STATE_WF_RESET) {
		/* wo reset by cr instead of get node from dts since get reset node protection */
		warp_woctrl_enter_state((struct warp_entry *)wed->warp, WO_STATE_DISABLE);
		warp_woctrl_enter_state((struct warp_entry *)wed->warp, WO_STATE_HALT);

		/* check heart_beat, if heart_beat is not 0 polling to 0.*/
		while(check_wo_heart_cnt < 3) {
			mdelay(10);
			warp_fwdl_get_wo_heartbeat(&warp->woif.fwdl_ctrl, &wo_heartbeat, warp->idx);

			if (wo_heartbeat == 0)
				break;
			else
				check_wo_heart_cnt++;
		}

		if (check_wo_heart_cnt < 3) {
			warp_wo_reset(warp->idx);
			/* clear fwdl parameter for wo fwdl next time */
			warp_fwdl_clear(warp);
		} else {
			warp_dbg(WARP_DBG_ERR,
				 "%s(): wo_heartbeat != 0, wo reset fail.\n", __func__);
		}
	} else {
		/* Disable WOCPU */
		warp_woctrl_enter_state((struct warp_entry *)wed->warp, WO_STATE_DISABLE);
		/* WO enters Gating further for power saving  */
		warp_woctrl_enter_state((struct warp_entry *)wed->warp, WO_STATE_GATING);
	}
#endif
#ifdef WED_HW_RX_SUPPORT
	/* Reset WDMA TX */
	reset_wdma_tx(wdma);
#endif
}

#if 0
static void
reset_rx_all(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 value;
	/*Reset WDMA*/
	reset_wdma_tx(wdma);
	/*Reset WED*/
	value = 1 << WED_MOD_RST_FLD_WED;
	WHNAT_RESET(wed, WED_MOD_RST, value);
}
#endif

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
	value |= (1 << WDMA_GLO_CFG_FLD_RX_DMA_EN);
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
*
*/
static void
wed_wpdma_inter_tx_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	u32 value = 0;

#ifdef WED_TX_SUPPORT
	/*TX0*/
	value |= (1 << WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_EN0);
	value |= (1 << WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_CLR0);
	value |= ((hw->wfdma_tx_done_trig0_bit & 0x1f) <<
		  WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_TRIG0);
	/*TX1*/
	value |= (1 << WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_EN1);
	value |= (1 << WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_CLR1);
	value |= ((hw->wfdma_tx_done_trig1_bit & 0x1f) <<
		  WED_WPDMA_INT_CTRL_TX_FLD_TX_DONE_TRIG1);
	warp_io_write32(wed, WED_WPDMA_INT_CTRL_TX, value);
#endif /*WED_TX_SUPPORT*/
#ifdef WED_RX_SUPPORT
	/*free notify*/
	value = 0;
	value |= (1 << WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_EN0);
	value |= (1 << WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_CLR0);
	value |= ((hw->wfdma_tx_done_free_notify_trig_bit & 0x1f) <<
		  WED_WPDMA_INT_CTRL_TX_FREE_FLD_TX_FREE_DONE_TRIG0);
	warp_io_write32(wed, WED_WPDMA_INT_CTRL_TX_FREE, value);
#endif /*WED_RX_SUPPORT*/
}

/*
*
*/
static void
wed_wpdma_inter_rx_init(struct wed_entry *wed, struct wifi_hw *hw)
{
#ifdef WED_RX_D_SUPPORT
	u32 value = 0;

	/*RX0*/
	value |= (1 << WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_EN0);
	value |= (1 << WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_CLR0);
	value |= ((hw->wfdma_rx_done_trig0_bit & 0x1f) <<
		  WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_TRIG0);
	/*RX1*/
	value |= (1 << WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_EN1);
	value |= (1 << WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_CLR1);
	value |= ((hw->wfdma_rx_done_trig1_bit & 0x1f) <<
		  WED_WPDMA_INT_CTRL_RX_FLD_RX_DONE_TRIG1);
	warp_io_write32(wed, WED_WPDMA_INT_CTRL_RX, value);
#endif /*WED_RX_D_SUPPORT*/
}

/*
*
*/
#define WED_WDMA_INT_TRIG_FLD_TX_DONE0  0
#define WED_WDMA_INT_TRIG_FLD_TX_DONE1  1
#define WED_WDMA_INT_TRIG_FLD_RX_DONE1	17
#define WED_WDMA_INT_TRIG_FLD_RX_DONE0	16

void
warp_int_ctrl_hw(struct wed_entry *wed, struct wifi_entry *wifi,
	struct wdma_entry *wdma, u32 int_agent,
	u8 enable, u32 pcie_ints_offset, int idx)
{
	struct wifi_hw *hw = &wifi->hw;
	u32 value = 0;

	/*wed control cr set*/
	wed_ctr_intr_set(wed, int_agent, enable);

	/*initail tx interrupt trigger*/
	wed_wpdma_inter_tx_init(wed, hw);
	/*initail rx interrupt trigger*/
	wed_wpdma_inter_rx_init(wed, hw);

#ifdef WED_HW_TX_SUPPORT
	{
		/*WED_WDMA Interrupt agent */
		value = 0;

		if (enable) {
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE0);
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE1);
		}

		warp_io_write32(wed, WED_WDMA_INT_TRIG, value);

		warp_io_read32(wed, WED_WDMA_INT_CLR, &value);

		if (enable) {
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE0);
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE1);
		}

		warp_io_write32(wed, WED_WDMA_INT_CLR, value);

		warp_wdma_int_sel(wed, idx);

		/*WDMA interrupt enable*/
		value = 0;

		if (enable) {
			value |= (1 << WDMA_INT_MASK_FLD_RX_DONE_INT1);
			value |= (1 << WDMA_INT_MASK_FLD_RX_DONE_INT0);
		}

		warp_io_write32(wdma, WDMA_INT_MASK, value);
		warp_io_write32(wdma, WDMA_INT_GRP2, value);
	}
#endif /*WED_HW_TX_SUPPORT*/

#ifdef WED_HW_RX_SUPPORT
	/*WED_WDMA Interrupt agent */
	warp_io_read32(wed, WED_WDMA_INT_TRIG , &value);

	if (enable) {
		value |= (1 << WED_WDMA_INT_TRIG_FLD_TX_DONE0);
		value |= (1 << WED_WDMA_INT_TRIG_FLD_TX_DONE1);
	}

	warp_io_write32(wed, WED_WDMA_INT_TRIG, value);

	warp_io_read32(wed, WED_WDMA_INT_CLR, &value);

	if (enable) {
		value |= (1 << WED_WDMA_INT_TRIG_FLD_TX_DONE0);
		value |= (1 << WED_WDMA_INT_TRIG_FLD_TX_DONE1);
	}

	warp_io_write32(wed, WED_WDMA_INT_CLR, value);

	/*WDMA interrupt enable*/
	warp_io_read32(wdma, WDMA_INT_MASK, &value);

	if (enable) {
		value |= (1 << WDMA_INT_MASK_FLD_TX_DONE_INT0);
		value |= (1 << WDMA_INT_MASK_FLD_TX_DONE_INT1);
	}

	warp_io_write32(wdma, WDMA_INT_MASK, value);

	warp_io_read32(wdma, WDMA_INT_GRP2, &value);

	if (enable) {
		value |= (1 << WDMA_INT_MASK_FLD_TX_DONE_INT0);
		value |= (1 << WDMA_INT_MASK_FLD_TX_DONE_INT1);
	}

	warp_io_write32(wdma, WDMA_INT_GRP2, value);
#endif

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
	if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
		value |= (1 << WED_EX_INT_STA_FLD_TX_BM_HTH);
		value |= (1 << WED_EX_INT_STA_FLD_TX_BM_LTH);
		value |= (1 << WED_EX_INT_STA_FLD_TX_TKID_HTH);
		value |= (1 << WED_EX_INT_STA_FLD_TX_TKID_LTH);
	}
#endif /*WED_DYNAMIC_TXBM_SUPPORT*/
	value |= (1 << WED_EX_INT_STA_FLD_TF_LEN_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TF_TKID_WO_PYLD);
#ifdef WED_DYNAMIC_RXBM_SUPPORT
	value |= (0x1 << WED_EX_INT_STA_FLD_RX_BM_H_BUF);
	value |= (0x1 << WED_EX_INT_STA_FLD_RX_BM_L_BUF);
	value |= (1 << WED_EX_INT_STA_FLD_RX_BM_DMAD_RD_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_RX_BM_FREE_AT_EMPTY);
#endif /*WED_DYNAMIC_RXBM_SUPPORT*/
	value |= (1 << WED_EX_INT_STA_FLD_TX_DMA_W_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TX_DMA_R_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_INTI_WDMA_ENABLED);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_COHERENT);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_W_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_R_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TF_TKID_FIFO_INVLD);

	wed->ext_int_mask = value;
#ifdef WED_RX_D_SUPPORT
	wed->ext_int_mask1 |= (1 << WED_EX_INT_STA_FLD_MID_RDY);
	wed->ext_int_mask2 |= (1 << WED_EX_INT_STA_FLD_MID_RDY);
	wed->ext_int_mask3 |= (1 << WED_EX_INT_STA_FLD_MID_RDY);
#endif
}

/*
*
*/
void
warp_eint_get_stat_hw(struct wed_entry *wed, u32 *state, u32 *err_state)
{
#ifdef WED_RX_D_SUPPORT
	u8 mode = WED_HWRRO_MODE;
#endif

	/*read stat*/
	warp_io_read32(wed, WED_EX_INT_STA, state);

#ifdef WED_RX_D_SUPPORT
	if (mode == WED_HWRRO_MODE_WOCPU)
		*state &= ~(1 << WED_EX_INT_STA_FLD_MID_RDY);
#endif

	/*write 1 clear*/
	warp_io_write32(wed, WED_EX_INT_STA, *state);

	*state &= wed->ext_int_mask;

	*err_state = 0;
}

/*
*
*/
void
warp_eint_clr_dybm_stat_hw(struct wed_entry *wed)
{
	u32 state = 0, mask = 0;;

	/*read stat*/
	warp_io_read32(wed, WED_EX_INT_STA, &state);

#if defined(WED_DYNAMIC_TXBM_SUPPORT)
	if (state & (0x1 << WED_EX_INT_STA_FLD_TX_BM_HTH))
		mask |= (0x1 << WED_EX_INT_STA_FLD_TX_BM_HTH);

	if (state & (0x1 << WED_EX_INT_STA_FLD_TX_BM_LTH))
		mask |= (0x1 << WED_EX_INT_STA_FLD_TX_BM_LTH);
#endif	/* WED_DYNAMIC_TXBM_SUPPORT */

#if defined(WED_DYNAMIC_RXBM_SUPPORT)
	if (state & (0x1 << WED_EX_INT_STA_FLD_RX_BM_H_BUF))
		mask |= (0x1 << WED_EX_INT_STA_FLD_RX_BM_H_BUF);

	if (state & (0x1 << WED_EX_INT_STA_FLD_RX_BM_L_BUF))
		mask |= (0x1 << WED_EX_INT_STA_FLD_RX_BM_L_BUF);
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */

	if (mask) {
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): INT STA:0x%04x, clear as:0x%04x\n",
				 __func__, state, mask);
#endif
		/*write 1 clear*/
		warp_io_write32(wed, WED_EX_INT_STA, mask);
	}
}


/*
*
*/
void
warp_dma_ctrl_hw(struct wed_entry *wed, u8 txrx)
{
#ifdef WED_HW_RX_SUPPORT
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
#endif

	wed_dma_ctrl(wed, txrx);
#ifdef WED_HW_RX_SUPPORT
	wdma_dma_ctrl(wdma, txrx);
#endif
}

void
warp_wed_ver(struct wed_entry *wed)
{
	u32 version = 0;

	warp_io_read32(wed, WED_REV_ID, &version);
	wed->ver = (version >> WED_REV_ID_FLD_MAJOR);
	wed->sub_ver = ((version >> WED_REV_ID_FLD_MINOR) & 0xfff);
	wed->branch = (version >> WED_REV_ID_FLD_BRANCH & 0xff);
	wed->eco = (version >> WED_REV_ID_FLD_ECO & 0xf);
}

/*
*
*/
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

/*
*
*/
void
warp_wed_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
	u32 wed_wdma_cfg;
	/*cfg wdma recycle*/
	warp_io_read32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~(WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE_MASK <<
			  WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE);
	wed_wdma_cfg &= ~((1 << WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN) |
			  (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE) |
			  (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE));
	/*disable auto idle*/
	wed_wdma_cfg &= ~(1 << WED_WDMA_GLO_CFG_FLD_RXDRV_DISABLED_FSM_AUTO_IDLE);
	/*Set to 16 DWORD for 64bytes*/
	wed_wdma_cfg |= (0x2 << WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE);
	/*enable skip state for fix dma busy issue*/
	wed_wdma_cfg |= ((1 << WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN) |
			 (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE));
#ifdef WED_HW_TX_SUPPORT
	warp_io_write32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
}

/*
*
*/
void
warp_wed_512_support_hw(struct wed_entry *wed, u8 *enable)
{
	u32 value;

	if (*enable) {
		warp_io_read32(wed, WED_TXDP_CTRL, &value);
		value |= (1 << 9);
		warp_io_write32(wed, WED_TXDP_CTRL, value);

		warp_io_read32(wed, WED_TXP_DW1, &value);
		value &= (0x0000ffff);
		value |= (0x0103 << 16);
		warp_io_write32(wed, WED_TXP_DW1, value);
	} else {
		warp_io_read32(wed, WED_TXP_DW1, &value);
		value &= (0x0000ffff);
		value |= (0x0100 << 16);
		warp_io_write32(wed, WED_TXP_DW1, value);

		warp_io_read32(wed, WED_TXDP_CTRL, &value);
		value &= ~(1 << 9);
		warp_io_write32(wed, WED_TXDP_CTRL, value);
	}
}

/*
*
*/
void
warp_write_hw_extra(u32 addr, u32 *val)
{
#ifdef WED_DELAY_INT_SUPPORT
	if (addr == WED_INT_MSK) {
		*val &= ~((1 << WED_INT_MSK_FLD_EN_4 /*WED_INT_MSK_FLD_TX_DONE_INT0*/) | (1 <<
				WED_INT_MSK_FLD_EN_5 /*WED_INT_MSK_FLD_TX_DONE_INT1*/));
		*val &= ~(1 << WED_INT_MSK_FLD_EN_1 /*WED_INT_MSK_FLD_RX_DONE_INT1*/);
		*val |= (1 << WED_INT_MSK_FLD_EN_23 /*WED_INT_MSK_FLD_TX_DLY_INT*/);
		*val |= (1 << WED_INT_MSK_FLD_EN_22 /*WED_INT_MSK_FLD_RX_DLY_INT*/);
	}
#endif /*WED_DELAY_INT_SUPPORT*/

}

/*
*
*/
void
warp_wdma_init_hw(struct wed_entry *wed, struct wdma_entry *wdma, int idx)
{
	u32 value = 0;

	/* WDMA base physical address */
	warp_io_write32(wed, WED_WDMA_CFG_BASE, wdma->base_phy_addr);

	warp_io_read32(wed, WED_CTRL, &value);
	if (wdma->ver == 0x2)
		value |= (1 << WED_CTRL_FLD_ETH_DMAD_FMT);
	warp_io_write32(wed, WED_CTRL, value);

	value = 0;
	/* OFST0_GLO_CFG offset */
	value |= ((WDMA_GLO_CFG - WDMA_TX_BASE_PTR_0) <<  WED_WDMA_OFST0_FLD_GLO_CFG);
	/* OFST0_FLD_INTS offset */
	value |= ((WDMA_INT_STATUS - WDMA_TX_BASE_PTR_0) <<  WED_WDMA_OFST0_FLD_INTS);
	warp_io_write32(wed, WED_WDMA_OFST0, value);

	value = 0;
	/* OFST1_RX0_CTRL offset */
	value |= ((WDMA_RX_BASE_PTR_0 - WDMA_TX_BASE_PTR_0) <<  WED_WDMA_OFST1_FLD_RX0_CTRL);
	/* OFST1_TX0_CTRL offset */
	value |= ((WDMA_TX_BASE_PTR_0 - WDMA_TX_BASE_PTR_0) <<  WED_WDMA_OFST1_FLD_TX0_CTRL);
	warp_io_write32(wed, WED_WDMA_OFST1, value);
}

/*
*
*/
void
warp_wdma_ring_init_hw(struct wed_entry *wed, struct wdma_entry *wdma)
{
#ifdef WED_HW_TX_SUPPORT
	struct wdma_rx_ring_ctrl *rx_ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;
	struct warp_ring *rx_ring;
#endif

#ifdef WED_HW_RX_SUPPORT
	struct wdma_tx_ring_ctrl *tx_ring_ctrl = &wdma->res_ctrl.tx_ctrl.tx_ring_ctrl;
	struct warp_ring *tx_ring;
#endif

	u32 offset = 0;
	int i = 0;

#ifdef WED_HW_TX_SUPPORT
	/* WDMA 2 RX rings shall both enbled due to H/W limitation.
	   For scenario that single ring required, it is only pratical to set unused ring as minimum length */
	for (i = 0; i < rx_ring_ctrl->ring_num; i++) {
		offset = i * WDMA_RING_OFFSET;
		rx_ring = &rx_ring_ctrl->ring[i];
		warp_dbg(WARP_DBG_INF, "%s(): configure ring %d setting\n", __func__, i);
		warp_dbg(WARP_DBG_INF, "%s(): wed:%p,wdma:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
			 wed, wdma,
			 rx_ring->hw_desc_base, (unsigned long)rx_ring->cell[0].pkt_pa,
			 rx_ring->hw_cnt_addr, rx_ring_ctrl->ring_len,
			 rx_ring->hw_cidx_addr, 0);
		/*WDMA*/
		warp_io_write32(wdma, rx_ring->hw_desc_base, rx_ring->cell[0].alloc_pa);
#ifdef WED_WDMA_DUAL_RX_RING
		if ((wifi_dbdc_support(wed->warp) == false) && (i == 1))	/* dbdc_mode is false, disable wdma ring1 by set to minimum length */
		/* according to coda, ring len - 2 should more than recycle threshold(minimum, 3) */
			warp_io_write32(wdma, rx_ring->hw_cnt_addr, WDMA_MIN_RING_LEN);
		else
#endif	/* WED_WDMA_DUAL_RX_RING */
			warp_io_write32(wdma, rx_ring->hw_cnt_addr, rx_ring_ctrl->ring_len);
		warp_io_write32(wdma, rx_ring->hw_cidx_addr, 0);
		/*WED_WDMA*/
		warp_io_write32(wed, WED_WDMA_RX0_BASE + offset, rx_ring->cell[0].alloc_pa);
#ifdef WED_WDMA_DUAL_RX_RING
		if ((wifi_dbdc_support(wed->warp) == false) && (i == 1))	/* dbdc_mode is false, disable wdma ring1 by set to minimum length */
		/* according to coda, ring len - 2 should more than recycle threshold(minimum, 3) */
			warp_io_write32(wed, WED_WDMA_RX0_CNT + offset, WDMA_MIN_RING_LEN);
		else
#endif	/* WED_WDMA_DUAL_RX_RING */
			warp_io_write32(wed, WED_WDMA_RX0_CNT + offset, rx_ring_ctrl->ring_len);
	}
#endif /*WED_HW_TX_SUPPORT*/

#ifdef WED_HW_RX_SUPPORT
	for (i = 0; i < tx_ring_ctrl->ring_num; i++) {
		offset = i * WDMA_RING_OFFSET;
		tx_ring = &tx_ring_ctrl->ring[i];
		warp_dbg(WARP_DBG_INF, "%s(): configure ring %d setting\n", __func__, i);
		warp_dbg(WARP_DBG_INF, "%s(): wed:%p,wdma:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
			 wed, wdma,
			 tx_ring->hw_desc_base, (unsigned long)tx_ring->cell[0].pkt_pa,
			 tx_ring->hw_cnt_addr, tx_ring_ctrl->ring_len,
			 tx_ring->hw_cidx_addr, 0);
		/*WDMA*/
		warp_io_write32(wdma, tx_ring->hw_desc_base, tx_ring->cell[0].alloc_pa);
		warp_io_write32(wdma, tx_ring->hw_cnt_addr, tx_ring_ctrl->ring_len);
		warp_io_write32(wdma, tx_ring->hw_cidx_addr, 0);
		warp_io_write32(wdma, tx_ring->hw_didx_addr, 0);

		if (i == 0) {
			/*WED_WDMA*/
			warp_io_write32(wed, WED_WDMA_TX0_BASE + offset, tx_ring->cell[0].alloc_pa);
			warp_io_write32(wed, WED_WDMA_TX0_CNT + offset, tx_ring_ctrl->ring_len);
			warp_io_write32(wed, WED_WDMA_TX0_CTX_IDX + offset, 0);
			warp_io_write32(wed, WED_WDMA_TX0_DTX_IDX + offset, 0);
		}
	}
#endif
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
		warp_dbg(WARP_DBG_OFF, "%s(): configure ring %d setting\n", __func__, i);
		warp_dbg(WARP_DBG_OFF, "%s(): wed:%p wifi:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
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
	warp_io_read32(wed, WED_RX0_CTRL0, &value);
	warp_io_write32(wed, WED_WPDMA_RX0_CTRL0, value);
	warp_io_write32(wifi, hw->event.base, value);
	/*Rx CNT*/
	warp_io_read32(wed, WED_RX0_CTRL1, &value);
	warp_io_write32(wed, WED_WPDMA_RX0_CTRL1, value);
	warp_io_write32(wifi, hw->event.cnt, value);
	/*cpu idx*/
	warp_io_read32(wed, WED_RX0_CTRL2, &value);
	warp_io_write32(wed, WED_WPDMA_RX0_CTRL2, value);
	warp_io_write32(wifi, hw->event.cidx, value);
	return 0;
}
#endif

#ifdef WED_RX_D_SUPPORT
int
warp_rx_data_ring_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct wifi_hw *hw = &wifi->hw;
	struct wed_rx_ring_ctrl *ring_ctrl = &wed->res_ctrl.rx_ctrl.ring_ctrl;
	struct warp_rx_ring *ring;
	u32 offset = (hw->rx[0].base - WED_WPDMA_RX_D_RX0_BASE);
	int i;
	u32 value = 0;

	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		warp_dbg(WARP_DBG_INF,
			 "%s(): configure rx ring %d setting\n", __func__, i);
		warp_dbg(WARP_DBG_INF,
			 "%s(): wed:%p wifi:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
			 wed, wifi,
			 ring->hw_desc_base, (unsigned long)ring->cell[0].alloc_pa,
			 ring->hw_cnt_addr, ring->ring_lens,
			 ring->hw_cidx_addr, 0);

		/* WFDMA */
		warp_io_write32(wifi, ring->hw_desc_base, ring->cell[0].alloc_pa);
		warp_io_write32(wifi, ring->hw_cnt_addr, ring->ring_lens);

		/* WED_WFDMA */
		warp_io_write32(wed, (ring->hw_desc_base - offset),
				ring->cell[0].alloc_pa);
		warp_io_write32(wed, (ring->hw_cnt_addr - offset),
				ring->ring_lens);
	}

	/* Reset Index of Ring */
	value = (1 << WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX0);
	value |= (1 << WED_WPDMA_RX_D_RST_IDX_FLD_CRX_IDX1);
	value |= (1 << WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX0);
	value |= (1 << WED_WPDMA_RX_D_RST_IDX_FLD_DRV_IDX1);
	warp_io_write32(wed, WED_WPDMA_RX_D_RST_IDX, value);
	warp_io_write32(wed, WED_WPDMA_RX_D_RST_IDX, 0);

	return 0;
}

int
warp_rx_dybm_mod_thrd (
	struct wed_entry *wed,
	u32 operation,
	u32 quota)
{
	u32 value = 0, buf_l_thrd = 0, buf_h_thrd = 0;
	struct dybm_conf_t *sw_conf = &wed->sw_conf->rxbm;
	struct wed_rx_ctrl *rx_ctrl = &wed->res_ctrl.rx_ctrl;

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_TH, &value);

	buf_l_thrd = (value & 0xffff);
	buf_h_thrd = ((value & 0xffff0000) >> WED_RX_BM_DYN_ALLOC_TH_H);

	switch (operation) {
		case THRD_INC_ALL:
			if (buf_l_thrd == 0) {
				if (rx_ctrl->budget_head_idx >= sw_conf->recycle_postponed)
					buf_l_thrd = ((sw_conf->buf_low + ((rx_ctrl->budget_head_idx-sw_conf->recycle_postponed) * sw_conf->alt_quota)*128) & 0xffff);
				else
					buf_l_thrd = (sw_conf->buf_low & 0xffff);
			} else
				buf_l_thrd = ((buf_l_thrd + quota) & 0xffff);
			value = buf_l_thrd;
			value |= (((buf_h_thrd + quota) & 0xffff) << WED_RX_BM_DYN_ALLOC_TH_H);
#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF, "%s(): Set buffer Low threshold=%d (%d)!\n",
					 __func__, (value & 0xffff), rx_ctrl->budget_head_idx);
#endif	/* WARP_DVT */

			break;

		case THRD_DEC_ALL:
			if (buf_l_thrd && (buf_l_thrd - quota) >= sw_conf->buf_low)	{/* only decrease threshold once valid */
				value = ((buf_l_thrd - quota) & 0xffff);
			} else {
				value = 0;	/* once buffer low threshold decrease to initial value, set 0 to disable detection */
#ifdef WARP_DVT
				warp_dbg(WARP_DBG_OFF, "%s(): Hit buffer low threshold minimum(%d), clear!\n",
						 __func__, sw_conf->buf_high*128+1);
#endif	/* WARP_DVT */
			}
			value |= (((buf_h_thrd - quota) & 0xffff) << WED_RX_BM_DYN_ALLOC_TH_H);

			break;

		case THRD_INC_H:
			value &= 0xffff;
			value |= (((buf_h_thrd + quota) & 0xffff) << WED_RX_BM_DYN_ALLOC_TH_H);

			break;

		case THRD_INC_L:
			value &= 0xffff0000;
			if (buf_l_thrd == 0) {
				if (rx_ctrl->budget_head_idx >= sw_conf->recycle_postponed)
					buf_l_thrd = ((sw_conf->buf_low + ((rx_ctrl->budget_head_idx-sw_conf->recycle_postponed) * sw_conf->alt_quota)*128) & 0xffff);
				else
					buf_l_thrd = (sw_conf->buf_low & 0xffff);
			} else
				buf_l_thrd = ((buf_l_thrd + quota) & 0xffff);
			value |= buf_l_thrd;
#ifdef WARP_DVT
			warp_dbg(WARP_DBG_OFF, "%s(): Set buffer Low threshold=%d (%d)!\n",
					 __func__, (value & 0xffff), rx_ctrl->budget_head_idx);
#endif	/* WARP_DVT */
			break;

		case THRD_DEC_H:
			value &= 0xffff;
			value |= (((buf_h_thrd - quota) & 0xffff) << WED_RX_BM_DYN_ALLOC_TH_H);

			break;

		case THRD_DEC_L:
			value &= 0xffff0000;
			if (buf_l_thrd && (buf_l_thrd - quota) >= sw_conf->buf_low)
				value |= ((buf_l_thrd - quota) & 0xffff);

			break;

		default:
			break;
	}
#ifdef WARP_DVT
	warp_dbg(WARP_DBG_OFF, "%s(): Change buffer threshold as: high=0x%04x, low=0x%04x! caller:%pS\n",
			 __func__, (value >> WED_RX_BM_DYN_ALLOC_TH_H), (value & 0xffff), __builtin_return_address(0));
#endif
	warp_io_write32(wed, WED_RX_BM_DYN_ALLOC_TH, value);

	warp_eint_clr_dybm_stat_hw(wed);

	return 0;
}

int
warp_rx_bm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u32 value = 0;
	struct wifi_hw *hw = &wifi->hw;
	u32 pkt_num = wed->res_ctrl.rx_ctrl.res.pkt_num;
	struct warp_dma_cb *cb = &wed->res_ctrl.rx_ctrl.res.ring->cell[0];

	warp_io_read32(wed, WED_RX_BM_RX_DMAD, &value);
	value |= (hw->rx_pkt_size << WED_RX_BM_RX_DMAD_FLD_SDL0);
	warp_io_write32(wed, WED_RX_BM_RX_DMAD, value);

	value = (cb->alloc_pa << WED_RX_BM_BASE_FLD_PTR);
	warp_io_write32(wed, WED_RX_BM_BASE, value);

	value = 0;
	value |= (pkt_num << WED_RX_BM_INIT_PTR_FLD_SW_TAIL_IDX);
	value |= (1 << WED_RX_BM_INIT_PTR_FLD_INIT_SW_TAIL_IDX);
	warp_io_write32(wed, WED_RX_BM_INIT_PTR, value);

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_TH, &value);
#ifdef WED_DYNAMIC_RXBM_SUPPORT
	if (IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
		value = 0;	/* disable shrink initially */
		value |= (((wed->sw_conf->rxbm.buf_high*128-32) & 0xffff) << WED_RX_BM_DYN_ALLOC_TH_H);
	} else
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */
	{
		value = 0;
		value |= (0xffff << WED_RX_BM_DYN_ALLOC_TH_H);
	}
	warp_io_write32(wed, WED_RX_BM_DYN_ALLOC_TH, value);

	/* Enable RX_BM to fetch DMAD */
	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_RX_BM_EN);
	warp_io_write32(wed, WED_CTRL, value);

	return 0;
}

int
warp_rx_rro_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	u8 mode = WED_HWRRO_MODE;
	u32 value;
	struct wed_rro_ctrl *rro_ctrl = &wed->res_ctrl.rx_ctrl.rro_ctrl;

	if (mode == WED_HWRRO_MODE_BYPASS) {
		warp_io_read32(wed, WED_RROQM_GLO_CFG, &value);
		value |= (1 << WED_RROQM_GLO_CFG_FLD_RROQM_BYPASS);
		warp_io_write32(wed, WED_RROQM_GLO_CFG, value);

		warp_io_read32(wed, WED_RTQM_DBG_CFG, &value);
		value |= (1 << WED_RTQM_DBG_CFG_FLD_R2HQ_FORCE);
		warp_io_write32(wed, WED_RTQM_DBG_CFG, value);

		warp_io_read32(wed, WED_RTQM_GLO_CFG , &value);
		value |= (1 << WED_RTQM_GLO_CFG_FLD_DBG_BYPASS);
		warp_io_write32(wed, WED_RTQM_GLO_CFG, value);
	} else {
		warp_io_read32(wed, WED_RROQM_GLO_CFG, &value);
		value &= ~(1 << WED_RROQM_GLO_CFG_FLD_RROQM_BYPASS);
		warp_io_write32(wed, WED_RROQM_GLO_CFG, value);

		/* mid/mod basic setting */
		value = 0;
		value |= ((rro_ctrl->mid_size >> 2) << WED_RROQM_MIOD_CFG_FLD_MID_DW);
		value |= ((rro_ctrl->mod_size >> 2) << WED_RROQM_MIOD_CFG_FLD_MOD_DW);
		value |= ((rro_ctrl->miod_entry_size >> 2) <<
			  WED_RROQM_MIOD_CFG_FLD_MIOD_ENTRY_DW);
		warp_io_write32(wed, WED_RROQM_MIOD_CFG, value);

		/* Specify the MID/MOD Ring */
		warp_io_read32(wed, WED_RROQM_MIOD_CTRL0, &value);
		value |= (rro_ctrl->miod_desc_base_pa << WED_RROQM_MIOD_CTRL0_FLD_BASE_PTR);
		warp_io_write32(wed, WED_RROQM_MIOD_CTRL0, value);

		warp_io_read32(wed, WED_RROQM_MIOD_CTRL1, &value);
		value |= (rro_ctrl->miod_cnt << WED_RROQM_MIOD_CTRL1_FLD_MAX_CNT);
		warp_io_write32(wed, WED_RROQM_MIOD_CTRL1, value);

		/* Specify the Feedback Command Ring */
		warp_io_read32(wed, WED_RROQM_FDBK_CTRL0, &value);
		value |= (rro_ctrl->fdbk_desc_base_pa << WED_RROQM_FDBK_CTRL0_FLD_BASE_PTR);
		warp_io_write32(wed, WED_RROQM_FDBK_CTRL0, value);

		warp_io_read32(wed, WED_RROQM_FDBK_CTRL1, &value);
		value |= (rro_ctrl->fdbk_cnt << WED_RROQM_FDBK_CTRL1_FLD_MAX_CNT);
		warp_io_write32(wed, WED_RROQM_FDBK_CTRL1, value);

		warp_io_write32(wed, WED_RROQM_FDBK_CTRL2, 0);

		/* Specify the RRO Queue */
		warp_io_read32(wed, WED_RROQ_BASE_L, &value);
		value |= (rro_ctrl->rro_que_base_pa << WED_RROQ_BASE_L_FLD_PTR);
		warp_io_write32(wed, WED_RROQ_BASE_L, value);

		/* Reset the Index of Rings */
		warp_io_read32(wed, WED_RROQM_RST_IDX, &value);
		value |= (1 << WED_RROQM_RST_IDX_FLD_FDBK);
		value |= (1 << WED_RROQM_RST_IDX_FLD_MIOD);
		warp_io_write32(wed, WED_RROQM_RST_IDX, value);

		warp_io_write32(wed, WED_RROQM_RST_IDX, 0);

		warp_io_write32(wed, WED_RROQM_MIOD_CTRL2, rro_ctrl->miod_cnt - 1);
	}

	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_RX_RRO_QM_EN);
	warp_io_write32(wed, WED_CTRL, value);
	return 0;
}

bool
is_warp_rx_dybm_add_op(struct wed_entry *wed)
{
	bool ret = false;
	u32 value = 0;

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);

	if (value & (0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ)) {
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_ERR, "%s(): Add operation!!!!\n", __func__);
#endif
		ret = true;
	}

	return ret;
}

bool
warp_rx_dybm_addsub_acked(struct wed_entry *wed)
{
	bool drained = false;
	u32 value = 0;

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);

	if (value & (0x1 << WED_RX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK)) {
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_ERR, "%s(): Cache drained!!!!\n", __func__);
#endif
		drained = true;
	} else
		drained = false;

	return drained;
}

int
warp_rx_dybm_w_cache(struct wed_entry *wed, u32 dma_pa)
{
	u32 value = 0;
	struct wed_rx_bm_res *extra_res = &wed->res_ctrl.rx_ctrl.extra;

	warp_io_read32(wed, WED_RX_BM_ADD_BASE, &value);
	value = 0;
	value = dma_pa;
	warp_io_write32(wed, WED_RX_BM_ADD_BASE, value);

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);
	value &= ~(0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ);
	value &= ~(0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ);

	value |= (0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ);
	value &= ~0xffff;
	value |= extra_res->ring_len;
	warp_io_write32(wed, WED_RX_BM_DYN_ALLOC_CFG, value);

	return 0;
}

int
warp_rx_dybm_r_fifo(
	struct wed_entry *wed,
	struct warp_dma_buf *desc,
	u32 stop_idx)
{
	int ret = 0;
#ifdef WARP_DVT
	struct warp_bm_rxdmad *rxdmad = NULL;
#endif	/* WARP_DVT */
	struct wed_rx_bm_res *res = &wed->res_ctrl.rx_ctrl.res;
	struct wed_rx_bm_res *extra_res = &wed->res_ctrl.rx_ctrl.extra;

	dma_sync_single_for_cpu(&wed->pdev->dev,
							(dma_addr_t)desc->alloc_pa,
							res->rxd_len*extra_res->ring_len, DMA_FROM_DEVICE);

	if ((0xffff - extra_res->ring_len) >= stop_idx) {
		dma_sync_single_for_cpu(&wed->pdev->dev,
								(dma_addr_t)res->desc->alloc_pa + stop_idx*res->rxd_len,
								res->rxd_len*extra_res->ring_len, DMA_FROM_DEVICE);
		memcpy((u8 *)desc->alloc_va,
				(u8 *)res->desc->alloc_va + stop_idx*res->rxd_len,
				res->rxd_len*extra_res->ring_len);
	} else {
		u32 break_cnt = 0xffff - stop_idx + 1;

		dma_sync_single_for_cpu(&wed->pdev->dev,
								(dma_addr_t)res->desc->alloc_pa + stop_idx*res->rxd_len,
								res->rxd_len*break_cnt, DMA_FROM_DEVICE);
		dma_sync_single_for_cpu(&wed->pdev->dev,
								(dma_addr_t)res->desc->alloc_pa,
								res->rxd_len*(extra_res->ring_len-break_cnt), DMA_FROM_DEVICE);

		memcpy((u8 *)desc->alloc_va,
				(u8 *)res->desc->alloc_va+stop_idx*res->rxd_len,
				res->rxd_len*break_cnt);
		memcpy((u8 *)desc->alloc_va+break_cnt*res->rxd_len,
				(u8 *)res->desc->alloc_va,
				res->rxd_len*(extra_res->ring_len-break_cnt));
#ifdef WARP_DVT
		warp_dbg(WARP_DBG_OFF, "%s(): fetch %d DMAD before FIFO end, and %d DMADs from FIFO start!(%d)\n",
				__func__, break_cnt, (extra_res->ring_len-break_cnt), stop_idx);
#endif	/* WARP_DVT */
	}

#ifdef WARP_DVT
	rxdmad = (struct warp_bm_rxdmad *)desc->alloc_va;
	warp_dbg(WARP_DBG_OFF, "%s(): recycle token start:%d!\n", __func__,
			 (rxdmad->token >> TOKEN_ID_SHIFT));

	rxdmad = (struct warp_bm_rxdmad *)desc->alloc_va + (extra_res->ring_len - 1);
	warp_dbg(WARP_DBG_OFF, "%s(): recycle token end:%d!\n", __func__,
			 (rxdmad->token >> TOKEN_ID_SHIFT));
#endif /* WARP_DVT */

	return ret;
}


u32
warp_rx_dybm_fifo_jump(struct wed_entry *wed, u32 ring_cnt, u32 *stop_idx)
{
	u32 value, max_wait=100, jump_idx = 0;
	struct wed_rx_ctrl *rx_ctrl = NULL;
	struct wed_rx_bm_res *extra_res = NULL;
#ifdef WARP_DVT
	unsigned long now = 0, after = 0;
#endif	/* WARP_DVT */

	rx_ctrl = &wed->res_ctrl.rx_ctrl;
	extra_res = &rx_ctrl->extra;

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);
	value &= ~(0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_ADD_BUF_REQ);
	value &= ~(0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ);

	value |= (0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_BUF_REQ);
	value &= ~0xffff;
	value |= extra_res->ring_len*ring_cnt;
	warp_io_write32(wed, WED_RX_BM_DYN_ALLOC_CFG, value);

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);
	while ((value & (0x1 << WED_RX_BM_DYN_ALLOC_CFG_HW_SUB_PAUSE)) == 0 && max_wait--)
		warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);

	if (max_wait == 0)
		warp_dbg(WARP_DBG_ERR,
				 "%s(): Wait pause flag for 100 times polling!!!\n", __func__);
#ifdef WARP_DVT
	else
		warp_dbg(WARP_DBG_OFF,
				 "%s(): Wait pause flag for %d times of polling!!!\n", __func__, 100-max_wait);
#endif	/* WARP_DVT */

	warp_io_read32(wed, WED_RX_BM_PTR, &value);
	value &= (0xffff << WED_RX_BM_PTR_FLD_HEAD_IDX);
	value >>= WED_RX_BM_PTR_FLD_HEAD_IDX;
	*stop_idx = value;
#ifdef WARP_DVT
	warp_dbg(WARP_DBG_OFF,
			 "%s(): Recycle head_idx:0x%04x from DMAD FIFO\n", __func__, value);
#endif /* WARP_DVT */
	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);
	value |= (0x1 << WED_RX_BM_DYN_ALLOC_CFG_SW_SUB_RDY);
	warp_io_write32(wed, WED_RX_BM_DYN_ALLOC_CFG, value);

	warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);
	max_wait = 100;
	while (((value & (0x1 << WED_RX_BM_DYN_ALLOC_CFG_HW_ADDSUB_ACK)) == 0) && max_wait--)
		warp_io_read32(wed, WED_RX_BM_DYN_ALLOC_CFG, &value);

	if (max_wait == 0)
		warp_dbg(WARP_DBG_ERR,
				 "%s(): Wait ACK flag for 100 times polling!!!\n", __func__);

	warp_io_read32(wed, WED_RX_BM_PTR, &value);
	value &= (0xffff << WED_RX_BM_PTR_FLD_HEAD_IDX);
	value >>= WED_RX_BM_PTR_FLD_HEAD_IDX;
	jump_idx = value;
#ifdef WARP_DVT
	warp_dbg(WARP_DBG_OFF,
			 "%s(): Recycle done, head_idx moved to 0x%04x\n", __func__, value);
#endif	/* WARP_DVT */

	return (jump_idx > *stop_idx) ? (jump_idx - *stop_idx) : (0x10000 - *stop_idx + jump_idx);
}


int
warp_rx_route_qm_init_hw(struct wed_entry *wed, struct wifi_entry *wifi)
{
	struct warp_entry *warp = (struct warp_entry *)wed->warp;
	struct wdma_entry *wdma = &warp->wdma;
	u32 value;

	/* Reset RX_ROUTE_QM */
	value = (1 << WED_MOD_RST_FLD_RX_ROUTE_QM);
	warp_io_write32(wed, WED_MOD_RST, value);

	do {
		udelay(100);
		warp_io_read32(wed, WED_MOD_RST, &value);

		if (!(value & WED_MOD_RST_FLD_RX_ROUTE_QM))
			break;
	} while (1);

	/* Configure RX_ROUTE_QM */
	warp_io_read32(wed, WED_RTQM_GLO_CFG, &value);
	value &= ~(1 << WED_RTQM_GLO_CFG_FLD_Q_RST);
	value &= ~(0xF << WED_RTQM_GLO_CFG_FLD_TXDMAD_FPORT);
	value |= (wdma->wdma_tx_port << WED_RTQM_GLO_CFG_FLD_TXDMAD_FPORT);
	warp_io_write32(wed, WED_RTQM_GLO_CFG, value);

	/* Enable RX_ROUTE_QM */
	warp_io_read32(wed, WED_RTQM_GLO_CFG, &value);
	value &= ~(1 << WED_RTQM_GLO_CFG_FLD_DBG_BYPASS);
	warp_io_write32(wed, WED_RTQM_GLO_CFG, value);

	warp_io_read32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_RX_ROUTE_QM_EN);
	warp_io_write32(wed, WED_CTRL, value);
	return 0;
}

u32 warp_rxbm_left_query(struct wed_entry *wed)
{
	u32 left = 0;
	u32 value = 0, head_idx = 0, tail_idx = 0;
	struct wed_rx_ctrl *rx_ctrl = &wed->res_ctrl.rx_ctrl;

	warp_io_read32(wed, WED_RX_BM_PTR, &value);
	head_idx = (value & (0xffff << WED_RX_BM_PTR_FLD_HEAD_IDX));
	head_idx >>= WED_RX_BM_PTR_FLD_HEAD_IDX;

	tail_idx = (value & 0xffff);

	if (tail_idx != head_idx) {
		left = (tail_idx > head_idx) ?
					(tail_idx - head_idx + 1) :
					(rx_ctrl->res.ring_len - head_idx + tail_idx);
	} else
		left = 0;

#ifdef WARP_DVT
	warp_dbg(WARP_DBG_OFF, "%s(): head:0x%04x tail:0x%04x\n", __func__, head_idx, tail_idx);
#endif	/* WARP_DVT */

	return left;
}
#endif

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
		/* TX data offload path reset */
		ret = reset_tx_traffic(wed, reset_type);

		if (ret < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): wed_tx reset check fail,ret=%d\n", __func__, ret);
			warp_dbg(WARP_DBG_ERR, "%s(): check ser_busy_cnt: echo 18 > wed\n", __func__, ret);
		}
		/* TX free done reset - rx path */
		ret = reset_rx_traffic(wed, reset_type);

		if (ret < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): wed_rx reset check fail,ret=%d\n", __func__, ret);
			warp_dbg(WARP_DBG_ERR, "%s(): check ser_busy_cnt: echo 18 > wed\n", __func__, ret);
		}

#ifdef WED_RX_D_SUPPORT
		/* Rx data offload path reset */
		ret = reset_rx_d_traffic(wed, reset_type);

		if (ret < 0) {
			warp_dbg(WARP_DBG_ERR, "%s(): wed_rx_data check fail,ret=%d\n", __func__, ret);
			warp_dbg(WARP_DBG_ERR, "%s(): check ser_busy_cnt: echo 18 > wed\n", __func__, ret);
		}
#endif

		break;

	case WARP_RESET_INTERFACE:
		{
			if (wed->sub_ver > 0)
				warp->woif.wo_ctrl.cur_state = WO_STATE_WF_RESET;

			reset_interface(wed, wdma);
		}
		break;
	}

	return ret;
}


/*
*
*/
void
warp_restore_hw(struct wed_entry *wed, struct wdma_entry *wdma, u8 restore_type)
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
	struct wifi_hw *hw = &wifi->hw;

	warp_io_write32(wed, WED_WPDMA_CFG_ADDR_INTS,
			(u32)wifi->hw.base_phy_addr + hw->int_sta);
	warp_io_write32(wed, WED_WPDMA_CFG_ADDR_INTM,
			(u32)wifi->hw.base_phy_addr + hw->int_mask);
	warp_io_write32(wed, WED_WPDMA_CFG_ADDR_TX,
			(u32)wifi->hw.base_phy_addr + hw->tx[0].base);
	warp_io_write32(wed, WED_WPDMA_CFG_ADDR_TX_FREE,
			(u32)wifi->hw.base_phy_addr + hw->event.base);

#ifdef WED_RX_D_SUPPORT
	/* WPDMA_RX_D_DRV WPDMA mapping */
	warp_io_write32(wed, WED_WPDMA_RX_D_GLO_CFG_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx_dma_glo_cfg);
	warp_io_write32(wed, WED_WPDMA_RX_D_RING_CFG_ADDR,
			(u32)wifi->hw.base_phy_addr + hw->rx[0].base);
#endif
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
	MTBL_ADD(hw->event.base, WED_RX0_CTRL0);
	MTBL_ADD(hw->event.cnt,  WED_RX0_CTRL1);
	MTBL_ADD(hw->event.cidx, WED_RX0_CTRL2);
	MTBL_ADD(hw->event.didx, WED_RX0_CTRL3);
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
#ifdef WED_RX_D_SUPPORT
	MTBL_ADD(hw->rx[0].base, WED_RX_BASE_PTR_0);
	MTBL_ADD(hw->rx[0].cnt,  WED_RX_MAX_CNT_0);
	MTBL_ADD(hw->rx[0].cidx, WED_RX_CRX_IDX_0);
	MTBL_ADD(hw->rx[0].didx, WED_RX_DRX_IDX_0);
	MTBL_ADD(hw->rx[1].base, WED_RX_BASE_PTR_1);
	MTBL_ADD(hw->rx[1].cnt,  WED_RX_MAX_CNT_1);
	MTBL_ADD(hw->rx[1].cidx, WED_RX_CRX_IDX_1);
	MTBL_ADD(hw->rx[1].didx, WED_RX_DRX_IDX_1);
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
	warp_io_read32(wed, WED_TX_FREE_TO_TX_TKID_TKID_MIB, &state->txfree_to_bm_mib);
	warp_io_read32(wed, WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB,
		       &state->txbm_to_wdma_mib);
	return;
}

/*
*
*/
void
warp_procinfo_dump_cfg_hw(struct warp_entry *warp, struct seq_file *seq)
{
	struct wed_entry *wed = &warp->wed;
	struct wifi_entry *wifi = &warp->wifi;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_hw *hw = &wifi->hw;
	u32 bm_size = 0;

	dump_string(seq, "==========WED basic info:==========\n");
	dump_io(seq, wed, "WED_REV", WED_REV_ID);
	dump_io(seq, wed, "WED_CTRL", WED_CTRL);
	dump_io(seq, wed, "WED_CTRL2", WED_CTRL2);
	dump_io(seq, wed, "WED_EX_INT_STA", WED_EX_INT_STA);
	dump_io(seq, wed, "WED_EX_INT_MSK", WED_EX_INT_MSK);
	dump_io(seq, wed, "WED_ST", WED_ST);
	dump_io(seq, wed, "WED_GLO_CFG", WED_GLO_CFG);
	dump_io(seq, wed, "WED_INT_STA", WED_INT_STA);
	dump_io(seq, wed, "WED_INT_MSK", WED_INT_MSK);
	dump_io(seq, wed, "WED_AXI_CTRL", WED_AXI_CTRL);
	dump_string(seq, "==========WED TX buf info:==========\n");
	dump_io(seq, wed, "WED_BM_ST", WED_BM_ST);
	dump_io(seq, wed, "WED_TX_BM_BASE", WED_TX_BM_BASE);
	warp_io_read32(wed, WED_TX_BM_CTRL, &bm_size);
	dump_string(seq, "WED_TX_BM_CTRL\t:%x\n", bm_size);
	dump_string(seq, "(BM size\t:%d packets)\n", (bm_size & WED_TX_BM_CTRL_FLD_VLD_GRP_NUM_MASK)*wed->res_ctrl.tx_ctrl.res.bm_grp_sz);
	dump_io(seq, wed, "WED_TX_BM_STS", WED_TX_BM_STS);
	dump_io(seq, wed, "WED_TX_BM_DYN_TH", WED_TX_BM_DYN_TH);
	dump_io(seq, wed, "WED_TX_BM_RECYC", WED_TX_BM_RECYC);
	dump_io(seq, wed, "WED_TX_TKID_BASE", WED_TX_TKID_BASE);
	dump_io(seq, wed, "WED_TX_TKID_CTRL", WED_TX_TKID_CTRL);
	dump_io(seq, wed, "WED_TX_TKID_TKID", WED_TX_TKID_TKID);
	dump_io(seq, wed, "WED_TX_TKID_DYN_TH", WED_TX_TKID_DYN_TH);
	dump_io(seq, wed, "WED_TX_TKID_INTF", WED_TX_TKID_INTF);
	dump_io(seq, wed, "WED_TX_TKID_RECYC", WED_TX_TKID_RECYC);
	dump_io(seq, wed, "WED_TX_FREE_TO_TX_BM_TKID_MIB",
			WED_TX_FREE_TO_TX_TKID_TKID_MIB);
	dump_io(seq, wed, "WED_TX_BM_TO_WDMA_RX_DRV_TKID_MIB",
			WED_TX_BM_TO_WDMA_RX_DRV_SKBID_MIB);
	dump_io(seq, wed, "WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB",
			WED_TX_TKID_TO_TX_BM_FREE_SKBID_MIB);
	dump_string(seq, "==========WED RX BM info:==========\n");
	dump_io(seq, wed, "WED_RX_BM_TO_WPDMA_RX_D_DRV_TKID_MIB",
			WED_RX_BM_TO_WPDMA_RX_D_DRV_TKID_MIB);
	dump_io(seq, wed, "WED_RX_BM_RX_DMAD", WED_RX_BM_RX_DMAD);
	dump_io(seq, wed, "WED_RX_BM_BASE", WED_RX_BM_BASE);
	dump_io(seq, wed, "WED_RX_BM_INIT_PTR", WED_RX_BM_INIT_PTR);
	dump_io(seq, wed, "WED_RX_BM_PTR", WED_RX_BM_PTR);
	dump_io(seq, wed, "WED_RX_BM_BLEN", WED_RX_BM_BLEN);
	dump_io(seq, wed, "WED_RX_BM_STS", WED_RX_BM_STS);
	dump_io(seq, wed, "WED_RX_BM_INTF2", WED_RX_BM_INTF2);
	dump_io(seq, wed, "WED_RX_BM_INTF", WED_RX_BM_INTF);
	dump_io(seq, wed, "WED_RX_BM_ERR_STS", WED_RX_BM_ERR_STS);
	dump_string(seq, "==========WED RRO QM:==========\n");
	dump_io(seq, wed, "WED_RROQM_GLO_CFG", WED_RROQM_GLO_CFG);
#if 0 // CODA_REMOVED_1025_TODO
	dump_addr_value(wed, "WED_RROQM_INT_STS", WED_RROQM_INT_STS);
	dump_addr_value(wed, "WED_RROQM_INT_MSK", WED_RROQM_INT_MSK);
#endif //CODA_REMOVED_1025_TODO
	dump_io(seq, wed, "WED_RROQM_MIOD_CTRL0", WED_RROQM_MIOD_CTRL0);
	dump_io(seq, wed, "WED_RROQM_MIOD_CTRL1", WED_RROQM_MIOD_CTRL1);
	dump_io(seq, wed, "WED_RROQM_MIOD_CTRL2", WED_RROQM_MIOD_CTRL2);
	dump_io(seq, wed, "WED_RROQM_MIOD_CTRL3", WED_RROQM_MIOD_CTRL3);
	dump_io(seq, wed, "WED_RROQM_FDBK_CTRL0", WED_RROQM_FDBK_CTRL0);
	dump_io(seq, wed, "WED_RROQM_FDBK_CTRL1", WED_RROQM_FDBK_CTRL1);
	dump_io(seq, wed, "WED_RROQM_FDBK_CTRL2", WED_RROQM_FDBK_CTRL2);
	dump_io(seq, wed, "WED_RROQM_FDBK_CTRL3", WED_RROQM_FDBK_CTRL3);
	dump_io(seq, wed, "WED_RROQ_BASE_L", WED_RROQ_BASE_L);
	dump_io(seq, wed, "WED_RROQ_BASE_H", WED_RROQ_BASE_H);
	dump_io(seq, wed, "WED_RROQM_MIOD_CFG", WED_RROQM_MIOD_CFG);
	dump_string(seq,  "==========WED PCI Host Control:==========\n");
	dump_io(seq, wed, "WED_PCIE_CFG_ADDR_INTS", WED_PCIE_CFG_ADDR_INTS);
	dump_io(seq, wed, "WED_PCIE_CFG_ADDR_INTM", WED_PCIE_CFG_ADDR_INTM);
	dump_io(seq, wed, "WED_PCIE_INTS_TRIG", WED_PCIE_INTS_TRIG);
	dump_io(seq, wed, "WED_PCIE_INTS_REC", WED_PCIE_INTS_REC);
	dump_io(seq, wed, "WED_PCIE_INTM_REC", WED_PCIE_INTM_REC);
	dump_io(seq, wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL);
	dump_string(seq,  "==========WED_WPDMA basic info:==========\n");
	dump_io(seq, wed, "WED_WPDMA_ST", WED_WPDMA_ST);
	dump_io(seq, wed, "WED_WPDMA_INT_STA_REC", WED_WPDMA_INT_STA_REC);
	dump_io(seq, wed, "WED_WPDMA_GLO_CFG", WED_WPDMA_GLO_CFG);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_INTS", WED_WPDMA_CFG_ADDR_INTS);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_INTM", WED_WPDMA_CFG_ADDR_INTM);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_TX", WED_WPDMA_CFG_ADDR_TX);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_TX_FREE", WED_WPDMA_CFG_ADDR_TX_FREE);
	dump_io(seq, wed, "WED_WPDMA_CFG_ADDR_RX", WED_WPDMA_CFG_ADDR_RX);
	dump_io(seq, wed, "WED_WPDAM_CTRL", WED_WPDMA_CTRL);
	dump_io(seq, wed, "WED_WPDMA_RX_D_GLO_CFG_ADDR",
			WED_WPDMA_RX_D_GLO_CFG_ADDR);
	dump_io(seq, wed, "WED_WPDMA_RX_D_RING_CFG_ADDR",
			WED_WPDMA_RX_D_RING_CFG_ADDR);
	dump_string(seq, "==========WED_WDMA basic info:==========\n");
	dump_io(seq, wed, "WED_WDMA_ST", WED_WDMA_ST);
	dump_io(seq, wed, "WED_WDMA_INFO", WED_WDMA_INFO);
	dump_io(seq, wed, "WED_WDMA_GLO_CFG", WED_WDMA_GLO_CFG);
	dump_io(seq, wed, "WED_WDMA_RST_IDX", WED_WDMA_RST_IDX);
	dump_io(seq, wed, "WED_WDMA_LOAD_DRV_IDX", WED_WDMA_LOAD_DRV_IDX);
	dump_io(seq, wed, "WED_WDMA_LOAD_CRX_IDX", WED_WDMA_LOAD_CRX_IDX);
	dump_io(seq, wed, "WED_WDMA_SPR", WED_WDMA_SPR);
	dump_io(seq, wed, "WED_WDMA_INT_STA_REC", WED_WDMA_INT_STA_REC);
	dump_io(seq, wed, "WED_WDMA_INT_TRIG", WED_WDMA_INT_TRIG);
	dump_io(seq, wed, "WED_WDMA_INT_CTRL", WED_WDMA_INT_CTRL);
	dump_io(seq, wed, "WED_WDMA_INT_CLR", WED_WDMA_INT_CLR);
	dump_io(seq, wed, "WED_WDMA_CFG_BASE", WED_WDMA_CFG_BASE);
	dump_io(seq, wed, "WED_WDMA_OFST0", WED_WDMA_OFST0);
	dump_io(seq, wed, "WED_WDMA_OFST1", WED_WDMA_OFST1);
	/*other part setting*/
	dump_string(seq,  "==========WDMA basic info:==========\n");
	dump_io(seq, wdma, "WDMA_GLO_CFG", WDMA_GLO_CFG);
	dump_io(seq, wdma, "WDMA_INT_MSK", WDMA_INT_MASK);
	dump_io(seq, wdma, "WDMA_INT_STA", WDMA_INT_STATUS);
	dump_io(seq, wdma, "WDMA_INFO", WDMA_INFO);
	dump_io(seq, wdma, "WDMA_DELAY_INT_CFG", WDMA_DELAY_INT_CFG);
	dump_io(seq, wdma, "WDMA_FREEQ_THRES", WDMA_FREEQ_THRES);
	dump_io(seq, wdma, "WDMA_INT_STS_GRP0", WDMA_INT_STS_GRP0);
	dump_io(seq, wdma, "WDMA_INT_STS_GRP1", WDMA_INT_STS_GRP1);
	dump_io(seq, wdma, "WDMA_INT_STS_GRP2", WDMA_INT_STS_GRP2);
	dump_io(seq, wdma, "WDMA_INT_GRP1", WDMA_INT_GRP1);
	dump_io(seq, wdma, "WDMA_INT_GRP2", WDMA_INT_GRP2);
	dump_io(seq, wdma, "WDMA_SCH_Q01_CFG", WDMA_SCH_Q01_CFG);
	dump_io(seq, wdma, "WDMA_SCH_Q23_CFG", WDMA_SCH_Q23_CFG);
	dump_string(seq,  "==========WPDMA basic info:==========\n");
	dump_io(seq, wifi, "WPDMA_TX_GLO_CFG", hw->tx_dma_glo_cfg);
	dump_io(seq, wifi, "WPDMA_RX_GLO_CFG", hw->rx_dma_glo_cfg);
	dump_io(seq, wifi, "WPDMA_INT_MSK", hw->int_mask);
	dump_io(seq, wifi, "WPDMA_INT_STA", hw->int_sta);
}

/*
*
*/
void
warp_procinfo_dump_txinfo_hw(struct warp_entry *warp, struct seq_file *output)
{
	struct wed_entry *wed = &warp->wed;
	struct wifi_entry *wifi = &warp->wifi;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_hw *hw = &wifi->hw;
#if defined(CONFIG_WARP_HW_DDR_PROF)
	u32 max_ltcy = 0, acc = 0, rx_cnt[2] = {0}, max_div = 0;
#endif	/* CONFIG_WARP_HW_DDR_PROF */
	u32 cidx = 0, didx = 0, qcnt = 0, tcnt = 0;

	dump_string(output, "==========WED TX ring info:==========\n");
	dump_io(output, wed, "WED_TX0_MIB", WED_TX0_MIB);
	dump_io(output, wed, "WED_TX0_BASE", WED_TX0_CTRL0);
	warp_io_read32(wed, WED_TX0_CTRL1, &tcnt);
	warp_io_read32(wed, WED_TX0_CTRL2, &cidx);
	warp_io_read32(wed, WED_TX0_CTRL3, &didx);
	seq_printf(output, "%s\t:%x\n", "WED_TX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_TX0_CIDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_TX0_DIDX", didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_TX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_TX1_MIB", WED_TX1_MIB);
	dump_io(output, wed, "WED_TX1_BASE", WED_TX1_CTRL0);
	warp_io_read32(wed, WED_TX1_CTRL1, &tcnt);
	warp_io_read32(wed, WED_TX1_CTRL2, &cidx);
	warp_io_read32(wed, WED_TX1_CTRL3, &didx);
	seq_printf(output, "%s\t:%x\n", "WED_TX1_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_TX1_CIDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_TX1_DIDX", didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_TX1 Qcnt", qcnt);
	/*WPDMA status from WED*/
	dump_string(output, "==========WED WPDMA TX ring info:==========\n");
	dump_io(output, wed, "WED_WPDMA_TX0_MIB", WED_WPDMA_TX0_MIB);
	dump_io(output, wed, "WED_WPDMA_TX0_BASE", WED_WPDMA_TX0_CTRL0);
	warp_io_read32(wed, WED_WPDMA_TX0_CTRL1, &tcnt);
	warp_io_read32(wed, WED_WPDMA_TX0_CTRL2, &cidx);
	warp_io_read32(wed, WED_WPDMA_TX0_CTRL3, &didx);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x(%x)\n", "WED_WPDMA_TX0_CIDX", (cidx & 0xfff), ((cidx & 0xfff0000) >> 16));
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX0_DIDX", didx);
	qcnt = ((cidx & 0xfff) >= didx) ? ((cidx & 0xfff) - didx) : ((cidx & 0xfff) - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_WPDMA_TX1_MIB", WED_WPDMA_TX1_MIB);
	dump_io(output, wed, "WED_WPDMA_TX1_BASE", WED_WPDMA_TX1_CTRL0);
	warp_io_read32(wed, WED_WPDMA_TX1_CTRL1, &tcnt);
	warp_io_read32(wed, WED_WPDMA_TX1_CTRL2, &cidx);
	warp_io_read32(wed, WED_WPDMA_TX1_CTRL3, &didx);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX1_CNT", tcnt);
	seq_printf(output, "%s\t:%x(%x)\n", "WED_WPDMA_TX1_CIDX", (cidx & 0xfff), ((cidx & 0xfff0000) >> 16));
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX1_DIDX", didx);
	qcnt = ((cidx & 0xfff) >= didx) ? ((cidx & 0xfff) - didx) : ((cidx & 0xfff) - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX1 Qcnt", qcnt);
	dump_io(output, wed, "WED_WPDMA_TX_COHERENT_MIB", WED_WPDMA_TX_COHERENT_MIB);
	/*WPDMA*/
	dump_string(output, "==========WPDMA TX ring info:==========\n");
	dump_io(output, wifi, "WPDMA_TX0_BASE", hw->tx[0].base);
	warp_io_read32(wifi, hw->tx[0].cnt, &tcnt);
	warp_io_read32(wifi, hw->tx[0].cidx, &cidx);
	warp_io_read32(wifi, hw->tx[0].didx, &didx);
	seq_printf(output, "%s\t:%x\n", "WPDMA_TX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WPDMA_TX0_CRX_IDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WPDMA_TX0_DRX_IDX", didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX0 Qcnt", qcnt);
	dump_io(output, wifi, "WPDMA_TX1_BASE", hw->tx[1].base);
	warp_io_read32(wifi, hw->tx[1].cnt, &tcnt);
	warp_io_read32(wifi, hw->tx[1].cidx, &cidx);
	warp_io_read32(wifi, hw->tx[1].didx, &didx);
	seq_printf(output, "%s\t:%x\n", "WPDMA_TX1_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WPDMA_TX1_CRX_IDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WPDMA_TX1_DRX_IDX", didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_TX1 Qcnt", qcnt);
	/* WPDMA TX Free Done Event Ring */
	dump_string(output, "==========WPDMA RX ring info(free done):==========\n");
	dump_io(output, wifi, "WPDMA_RX0_BASE", hw->event.base);
	warp_io_read32(wifi, hw->event.cnt, &tcnt);
	warp_io_read32(wifi, hw->event.cidx, &cidx);
	warp_io_read32(wifi, hw->event.didx, &didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WPDMA_RX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WPDMA_RX0_CRX_IDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WPDMA_RX0_DRX_IDX", didx);
	seq_printf(output, "%s\t:%x\n", "WPDMA_RX0 Qcnt", qcnt);
	/*WPDMA status from WED*/
	dump_string(output, "==========WED WPDMA RX ring info(free done):==========\n");
	dump_io(output, wed, "WED_WPDMA_RX0_MIB", WED_WPDMA_RX0_MIB);
	dump_io(output, wed, "WED_WPDMA_RX0_BASE", WED_WPDMA_RX0_CTRL0);
	warp_io_read32(wed, WED_WPDMA_RX0_CTRL1, &tcnt);
	warp_io_read32(wed, WED_WPDMA_RX0_CTRL2, &cidx);
	warp_io_read32(wed, WED_WPDMA_RX0_CTRL3, &didx);
	qcnt = (didx > (cidx & 0xfff)) ? (didx - 1 - (cidx & 0xfff)) : (didx - 1 - (cidx & 0xfff) + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x(%x)\n", "WED_WPDMA_RX0_CIDX", (cidx & 0xfff), ((cidx & 0xfff0000) >> 16));
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX0_DIDX", didx);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_WPDMA_RX1_MIB", WED_WPDMA_RX1_MIB);
	dump_io(output, wed, "WED_WPDMA_RX1_BASE", WED_WPDMA_RX1_CTRL0);
	warp_io_read32(wed, WED_WPDMA_RX1_CTRL1, &tcnt);
	warp_io_read32(wed, WED_WPDMA_RX1_CTRL2, &cidx);
	warp_io_read32(wed, WED_WPDMA_RX1_CTRL3, &didx);
	qcnt = (didx > (cidx & 0xfff)) ? (didx - 1 - (cidx & 0xfff)) : (didx - 1 - (cidx & 0xfff) + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX1_CNT", tcnt);
	seq_printf(output, "%s\t:%x(%x)\n", "WED_WPDMA_RX1_CIDX", (cidx & 0xfff), ((cidx & 0xfff0000) >> 16));
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX1_DIDX", didx);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX1 Qcnt", qcnt);
	dump_io(output, wed, "WED_WPDMA_RX_COHERENT_MIB", WED_WPDMA_RX_COHERENT_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_EXTC_FREE_TKID_MIB",
			WED_WPDMA_RX_EXTC_FREE_TKID_MIB);
	dump_string(output, "==========WED RX ring info(free done):==========\n");
	dump_io(output, wed, "WED_RX0_MIB", WED_RX0_MIB);
	dump_io(output, wed, "WED_RX0_BASE", WED_RX0_CTRL0);
	warp_io_read32(wed, WED_RX0_CTRL1, &tcnt);
	warp_io_read32(wed, WED_RX0_CTRL2, &cidx);
	warp_io_read32(wed, WED_RX0_CTRL3, &didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX0_CIDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_RX0_DIDX", didx);
	seq_printf(output, "%s\t:%x\n", "WED_RX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_RX1_MIB", WED_RX1_MIB);
	dump_io(output, wed, "WED_RX1_BASE", WED_RX1_CTRL0);
	warp_io_read32(wed, WED_RX1_CTRL1, &tcnt);
	warp_io_read32(wed, WED_RX1_CTRL2, &cidx);
	warp_io_read32(wed, WED_RX1_CTRL3, &didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX1_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX1_CIDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_RX1_DIDX", didx);
	seq_printf(output, "%s\t:%x\n", "WED_RX1 Qcnt", qcnt);
	/*WDMA status from WED*/
	dump_string(output, "==========WED WDMA RX ring info:==========\n");
	dump_io(output, wed, "WED_WDMA_RX0_MIB", WED_WDMA_RX0_MIB);
	dump_io(output, wed, "WED_WDMA_RX0_BASE", WED_WDMA_RX0_BASE);
	warp_io_read32(wed, WED_WDMA_RX0_CNT, &tcnt);
	warp_io_read32(wed, WED_WDMA_RX0_CRX_IDX, &cidx);
	warp_io_read32(wed, WED_WDMA_RX0_DRX_IDX, &didx);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX0_CRX_IDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX0_DRX_IDX", didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_WDMA_RX0_THRES_CFG", WED_WDMA_RX0_THRES_CFG);
	dump_io(output, wed, "WED_WDMA_RX0_RECYCLE_MIB", WED_WDMA_RX0_RECYCLE_MIB);
	dump_io(output, wed, "WED_WDMA_RX0_PROCESSED_MIB", WED_WDMA_RX0_PROCESSED_MIB);
	dump_io(output, wed, "WED_WDMA_RX1_MIB", WED_WDMA_RX1_MIB);
	dump_io(output, wed, "WED_WDMA_RX1_BASE", WED_WDMA_RX1_BASE);
	warp_io_read32(wed, WED_WDMA_RX1_CNT, &tcnt);
	warp_io_read32(wed, WED_WDMA_RX1_CRX_IDX, &cidx);
	warp_io_read32(wed, WED_WDMA_RX1_DRX_IDX, &didx);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX1_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX1_CRX_IDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX1_DRX_IDX", didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_RX1 Qcnt", qcnt);
	dump_io(output, wed, "WED_WDMA_RX1_THRES_CFG", WED_WDMA_RX1_THRES_CFG);
	dump_io(output, wed, "WED_WDMA_RX1_RECYCLE_MIB", WED_WDMA_RX1_RECYCLE_MIB);
	dump_io(output, wed, "WED_WDMA_RX1_PROCESSED_MIB", WED_WDMA_RX1_PROCESSED_MIB);
	/*WDMA*/
	dump_string(output, "==========WDMA RX ring info:==========\n");
	dump_io(output, wdma, "WDMA_RX_BASE_PTR_0", WDMA_RX_BASE_PTR_0);
	warp_io_read32(wdma, WDMA_RX_MAX_CNT_0, &tcnt);
	warp_io_read32(wdma, WDMA_RX_CRX_IDX_0, &cidx);
	warp_io_read32(wdma, WDMA_RX_DRX_IDX_0, &didx);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_MAX_CNT_0", tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_CRX_IDX_0", cidx);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_DRX_IDX_0", didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_0 Qcnt", qcnt);
	dump_io(output, wdma, "WDMA_RX_BASE_PTR_1", WDMA_RX_BASE_PTR_1);
	warp_io_read32(wdma, WDMA_RX_MAX_CNT_1, &tcnt);
	warp_io_read32(wdma, WDMA_RX_CRX_IDX_1, &cidx);
	warp_io_read32(wdma, WDMA_RX_DRX_IDX_1, &didx);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_MAX_CNT_1", tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_CRX_IDX_1", cidx);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_DRX_IDX_1", didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_RX_1 Qcnt", qcnt);
#if defined(CONFIG_WARP_HW_DDR_PROF)
	dump_string(output, "==========WDMA DRAM Latency info:==========\n");
	dump_io(output, wed, "WED_PMTR_CTRL", WED_PMTR_CTRL);
	dump_io(output, wed, "WED_PMTR_TGT", WED_PMTR_TGT);
	dump_io(output, wed, "WED_PMTR_TGT_ST", WED_PMTR_TGT_ST);
	dump_io(output, wed, "WED_PMTR_LTCY_MAX0_1", WED_PMTR_LTCY_MAX0_1);
	dump_io(output, wed, "WED_PMTR_LTCY_ACC0", WED_PMTR_LTCY_ACC0);
	warp_io_read32(wed, WED_PMTR_LTCY_MAX0_1, &max_ltcy);
	max_ltcy &= 0xffff; //WED_PMTR_LTCY_MAX0_1_FLD_MST0_CNT
	warp_io_read32(wed, WED_PMTR_CTRL, &max_div);
	if (max_div & (0x1 << WED_PMTR_CTRL_FLD_EN)) {
		max_div &= (0xf << WED_PMTR_CTRL_FLD_MAX_DIV);
		warp_io_read32(wed, WED_PMTR_LTCY_ACC0, &acc);
		warp_io_read32(wed, WED_WDMA_RX0_PROCESSED_MIB, &rx_cnt[0]);
		warp_io_read32(wed, WED_WDMA_RX1_PROCESSED_MIB, &rx_cnt[1]);
		seq_printf(output, "%s\t:%d ticks\n", "MAX DDR Latency", max_ltcy);
		if ((rx_cnt[0]+rx_cnt[1]))
			seq_printf(output, "%s\t:%d ticks\n", "AVG DDR Latency",
						(acc*(1 << max_div))/(rx_cnt[0]+rx_cnt[1]));
		else
			seq_printf(output, "Invalid total packet count(%d)!", (rx_cnt[0]+rx_cnt[1]));
	} else
		seq_printf(output, "DDR access latency profiling is not enabled!\n");
#endif	/* CONFIG_WARP_HW_DDR_PROF */
}

/*
*
*/
void
 warp_procinfo_dump_rxinfo_hw(struct warp_entry *warp, struct seq_file *output)
 {
	struct wed_entry *wed = &warp->wed;
	struct wdma_entry *wdma = &warp->wdma;
	struct wifi_entry *wifi = &warp->wifi;
	struct wifi_hw *hw  = &wifi->hw;
	struct wed_rro_ctrl *rro_ctrl = &wed->res_ctrl.rx_ctrl.rro_ctrl;
#if defined(CONFIG_WARP_HW_DDR_PROF)
	u32 max_ltcy = 0, acc = 0, rx_cnt[2] = {0}, max_div = 0;
#endif	/* CONFIG_WARP_HW_DDR_PROF */
	u32 cidx = 0, didx = 0, qcnt = 0, tcnt = 0, value = 0;

	dump_string(output, "==========WED RX INT info:==========\n");
	dump_io(output, wed, "WED_PCIE_INT_CTRL", WED_PCIE_INT_CTRL);
	dump_io(output, wed, "WED_PCIE_INTS_REC", WED_PCIE_INTS_REC);
	dump_io(output, wed, "WED_WPDMA_INT_STA_REC", WED_WPDMA_INT_STA_REC);
	dump_io(output, wed, "WED_WPDMA_INT_MON", WED_WPDMA_INT_MON);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL", WED_WPDMA_INT_CTRL);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL_TX", WED_WPDMA_INT_CTRL_TX);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL_RX", WED_WPDMA_INT_CTRL_RX);
	dump_io(output, wed, "WED_WPDMA_INT_CTRL_TX_FREE", WED_WPDMA_INT_CTRL_TX_FREE);
	dump_io(output, wed, "WED_WPDMA_ST", WED_WPDMA_ST);
	dump_io(output, wed, "WED_WPDMA_D_ST", WED_WPDMA_D_ST);
	dump_io(output, wed, "WED_WPDMA_RX_D_GLO_CFG", WED_WPDMA_RX_D_GLO_CFG);
	/* WED RX Data Ring */
	dump_string(output, "==========WED RX ring info:==========\n");
	dump_io(output, wed, "WED_RX_BASE_PTR_0", WED_RX_BASE_PTR_0);
	warp_io_read32(wed, WED_RX_MAX_CNT_0, &tcnt);
	warp_io_read32(wed, WED_RX_CRX_IDX_0, &cidx);
	warp_io_read32(wed, WED_RX_DRX_IDX_0, &didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX_MAX_CNT_0", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX_CRX_IDX_0", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_RX_DRX_IDX_0", didx);
	seq_printf(output, "%s\t:%x\n", "WED_RX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_RX_BASE_PTR_1", WED_RX_BASE_PTR_1);
	warp_io_read32(wed, WED_RX_MAX_CNT_1, &tcnt);
	warp_io_read32(wed, WED_RX_CRX_IDX_1, &cidx);
	warp_io_read32(wed, WED_RX_DRX_IDX_1, &didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX_MAX_CNT_1", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RX_CRX_IDX_1", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_RX_DRX_IDX_1", didx);
	seq_printf(output, "%s\t:%x\n", "WED_RX1 Qcnt", qcnt);
	/* WED_WPDMA RX Data Ring */
	dump_string(output, "==========WED_WPDMA RX ring info:==========\n");
	dump_io(output, wed, "WED_WPDMA_RX_D_RX0_BASE", WED_WPDMA_RX_D_RX0_BASE);
	warp_io_read32(wed, WED_WPDMA_RX_D_RX0_CNT, &tcnt);
	warp_io_read32(wed, WED_WPDMA_RX_D_RX0_CRX_IDX, &cidx);
	warp_io_read32(wed, WED_WPDMA_RX_D_RX0_DRX_IDX, &didx);
	qcnt = (didx > (cidx & 0xfff)) ? (didx - 1 - (cidx & 0xfff)) : (didx - 1 - (cidx & 0xfff) + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX_D_RX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x(%x)\n", "WED_WPDMA_RX_D_RX0_CRX_IDX", (cidx & 0xfff), ((cidx & 0xfff0000) >> 16));
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX_D_RX0_DRX_IDX", didx);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX_D_RX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX1_BASE", WED_WPDMA_RX_D_RX1_BASE);
	warp_io_read32(wed, WED_WPDMA_RX_D_RX1_CNT, &tcnt);
	warp_io_read32(wed, WED_WPDMA_RX_D_RX1_CRX_IDX, &cidx);
	warp_io_read32(wed, WED_WPDMA_RX_D_RX1_DRX_IDX, &didx);
	qcnt = (didx > (cidx & 0xfff)) ? (didx - 1 - (cidx & 0xfff)) : (didx - 1 - (cidx & 0xfff) + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX_D_RX1_CNT", tcnt);
	seq_printf(output, "%s\t:%x(%x)\n", "WED_WPDMA_RX_D_RX1_CRX_IDX", (cidx & 0xfff), ((cidx & 0xfff0000) >> 16));
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX_D_RX1_DRX_IDX", didx);
	seq_printf(output, "%s\t:%x\n", "WED_WPDMA_RX_D_RX1 Qcnt", qcnt);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX0_MIB", WED_WPDMA_RX_D_RX0_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX1_MIB", WED_WPDMA_RX_D_RX1_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX0_RECYCLE_MIB",
			WED_WPDMA_RX_D_RX0_RECYCLE_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX1_RECYCLE_MIB",
			WED_WPDMA_RX_D_RX1_RECYCLE_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX0_PROCESSED_MIB",
			WED_WPDMA_RX_D_RX0_PROCESSED_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX1_PROCESSED_MIB",
			WED_WPDMA_RX_D_RX1_PROCESSED_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_D_RX_COHERENT_MIB",
			WED_WPDMA_RX_D_RX_COHERENT_MIB);
	dump_io(output, wed, "WED_WPDMA_RX_D_ERR_STS", WED_WPDMA_RX_D_ERR_STS);
	/* WPDMA RX Data Ring */
	dump_string(output, "==========WPDMA RX ring info:==========\n");
	dump_io(output, wifi, "WIFI_RX_DATA_RING0_BASE", hw->rx[0].base);
	warp_io_read32(wifi, hw->rx[0].cnt, &tcnt);
	warp_io_read32(wifi, hw->rx[0].cidx, &cidx);
	warp_io_read32(wifi, hw->rx[0].didx, &didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING0_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING0_CIDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING0_DIDX", didx);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING0 Qcnt", qcnt);
	dump_io(output, wifi, "WIFI_RX_DATA_RING1_BASE", hw->rx[1].base);
	warp_io_read32(wifi, hw->rx[1].cnt, &tcnt);
	warp_io_read32(wifi, hw->rx[1].cidx, &cidx);
	warp_io_read32(wifi, hw->rx[1].didx, &didx);
	qcnt = (didx > cidx) ? (didx - 1 - cidx) : (didx - 1 - cidx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING1_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING1_CIDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING1_DIDX", didx);
	seq_printf(output, "%s\t:%x\n", "WIFI_RX_DATA_RING1 Qcnt", qcnt);
	/*WDMA status from WED*/
	dump_string(output, "==========WED WDMA TX ring info:==========\n");
	dump_io(output, wed, "WED_WDMA_TX0_BASE", WED_WDMA_TX0_BASE);
	warp_io_read32(wed, WED_WDMA_TX0_CNT, &tcnt);
	warp_io_read32(wed, WED_WDMA_TX0_CTX_IDX, &cidx);
	warp_io_read32(wed, WED_WDMA_TX0_DTX_IDX, &didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_TX0_CNT", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_TX0_CTX_IDX", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_TX0_DTX_IDX", didx);
	seq_printf(output, "%s\t:%x\n", "WED_WDMA_TX0 Qcnt", qcnt);
	dump_io(output, wed, "WED_WDMA_TX0_MIB", WED_WDMA_TX0_MIB);
	/*WDMA*/
	dump_string(output, "==========WDMA TX ring info:==========\n");
	dump_io(output, wdma, "WDMA_TX_BASE_PTR_0", WDMA_TX_BASE_PTR_0);
	warp_io_read32(wdma, WDMA_TX_MAX_CNT_0, &tcnt);
	warp_io_read32(wdma, WDMA_TX_CTX_IDX_0, &cidx);
	warp_io_read32(wdma, WDMA_TX_DTX_IDX_0, &didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX_MAX_CNT_0", tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX_CTX_IDX_0", cidx);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX_DTX_IDX_0", didx);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX0 Qcnt", qcnt);
	dump_io(output, wdma, "WDMA_TX_BASE_PTR_1", WDMA_TX_BASE_PTR_1);
	warp_io_read32(wdma, WDMA_TX_MAX_CNT_1, &tcnt);
	warp_io_read32(wdma, WDMA_TX_CTX_IDX_1, &cidx);
	warp_io_read32(wdma, WDMA_TX_DTX_IDX_1, &didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX_MAX_CNT_1", tcnt);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX_CTX_IDX_1", cidx);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX_DTX_IDX_1", didx);
	seq_printf(output, "%s\t:%x\n", "WDMA_TX1 Qcnt", qcnt);
	/* MID/MOD/Feedback Cmd/RRI Queue */
	dump_string(output, "==========MID/MOD/feedback cmd/RRO queue info:==========\n");
#ifdef WED_RX_D_SUPPORT
	dump_string(output, "RX Data Mode = %d\n", WED_HWRRO_MODE);
#endif
	dump_string(output, "MID/MOD base addr = 0x%llx\n",
		 rro_ctrl->miod_desc_base_pa);
	dump_string(output, "Feedback Cmd base addr = 0x%llx\n",
		 rro_ctrl->fdbk_desc_base_pa);
	dump_string(output, "RRO queue base addr = 0x%llx\n",
		 rro_ctrl->rro_que_base_pa);

	dump_io(output, wed, "WED_RROQM_MIOD_CTRL0(base)", WED_RROQM_MIOD_CTRL0);
	warp_io_read32(wed, WED_RROQM_MIOD_CTRL1, &tcnt);
	warp_io_read32(wed, WED_RROQM_MIOD_CTRL2, &cidx);
	warp_io_read32(wed, WED_RROQM_MIOD_CTRL3, &didx);
	qcnt = (cidx >= didx) ? (cidx - didx) : (cidx - didx + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RROQM_MIOD_CTRL1(max_cnt)", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RROQM_MIOD_CTRL2(cpu_idx)", cidx);
	seq_printf(output, "%s\t:%x\n", "WED_RROQM_MIOD_CTRL3(dma_idx)", didx);
	seq_printf(output, "%s\t:%x\n", "WED_RROQM_MIOD Qcnt", qcnt);
	dump_io(output, wed, "WED_RROQM_MID_MIB", WED_RROQM_MID_MIB);
	dump_io(output, wed, "WED_RROQM_MOD_MIB", WED_RROQM_MOD_MIB);
	dump_io(output, wed, "WED_RROQM_MOD_COH_MIB", WED_RROQM_MOD_COH_MIB);
	dump_io(output, wed, "WED_RROQM_FDBK_CTRL0(base)", WED_RROQM_FDBK_CTRL0);
	warp_io_read32(wed, WED_RROQM_FDBK_CTRL1, &tcnt);
	warp_io_read32(wed, WED_RROQM_FDBK_CTRL2, &cidx);
	warp_io_read32(wed, WED_RROQM_FDBK_CTRL3, &didx);
	qcnt = (cidx >= (didx & 0xffff)) ? (cidx - (didx & 0xffff)) : (cidx - (didx & 0xffff) + tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RROQM_FDBK_CTRL1(max_cnt)", tcnt);
	seq_printf(output, "%s\t:%x\n", "WED_RROQM_FDBK_CTRL2(cpu_idx)", cidx);
	seq_printf(output, "%s\t:%x(%x)\n", "WED_RROQM_FDBK_CTRL3(dma_idx)", (didx & 0xffff), (didx >> 16));
	seq_printf(output, "%s\t:%x\n", "WED_RROQM_FDBK Qcnt", qcnt);
	dump_io(output, wed, "WED_RROQM_FDBK_MIB", WED_RROQM_FDBK_MIB);
	dump_io(output, wed, "WED_RROQM_FDBK_COH_MIB", WED_RROQM_FDBK_COH_MIB);
	dump_io(output, wed, "WED_RROQM_FDBK_IND_MIB", WED_RROQM_FDBK_IND_MIB);
	dump_io(output, wed, "WED_RROQM_FDBK_ENQ_MIB", WED_RROQM_FDBK_ENQ_MIB);
	dump_io(output, wed, "WED_RROQM_FDBK_ANC_MIB", WED_RROQM_FDBK_ANC_MIB);
	dump_io(output, wed, "WED_RROQM_FDBK_ANC2H_MIB", WED_RROQM_FDBK_ANC2H_MIB);

	/* WED Route QM RRO */
	dump_string(output, "==========WED Route QM RRO info:==========\n");
	dump_io(output, wed, "WED_RTQM_R2H_MIB_0", WED_RTQM_R2H_MIB_0);
	dump_io(output, wed, "WED_RTQM_R2H_MIB_1", WED_RTQM_R2H_MIB_1);
	dump_io(output, wed, "WED_RTQM_R2Q_MIB_0", WED_RTQM_R2Q_MIB_0);
	dump_io(output, wed, "WED_RTQM_R2Q_MIB_1", WED_RTQM_R2Q_MIB_1);
	dump_io(output, wed, "WED_RTQM_Q2N_MIB", WED_RTQM_Q2N_MIB);
	dump_io(output, wed, "WED_RTQM_Q2H_MIB_0", WED_RTQM_Q2H_MIB_0);
	dump_io(output, wed, "WED_RTQM_Q2H_MIB_1", WED_RTQM_Q2H_MIB_1);
	dump_io(output, wed, "WED_RTQM_Q2B_MIB", WED_RTQM_Q2B_MIB);
	dump_io(output, wed, "WED_RTQM_PFDBK_MIB", WED_RTQM_PFDBK_MIB);
	dump_io_diff(output, wed, "Rx_Route_QM_Qcnt ", WED_RTQM_Q2N_MIB, WED_RTQM_PFDBK_MIB);
	dump_string(output, "===== WED RXBM debug info =====\n");
	warp_io_read32(wed, WED_RX_BM_PTR, &value);
	seq_printf(output, "WED_RX_BM_PTR\t:0x%04x\n", value);
	seq_printf(output, "\t(head:0x%04x tail:0x%04x)\n", (value >> 16), (value & 0xffff));
#ifdef WED_DYNAMIC_RXBM_SUPPORT
	if (IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
		dump_string(output, "===== WED Dynamic RXBM debug info =====\n");
		dump_io(output, wed, "WED_RX_BM_DYN_ALLOC_CFG", WED_RX_BM_DYN_ALLOC_CFG);
		dump_io(output, wed, "WED_RX_BM_DYN_ALLOC_TH", WED_RX_BM_DYN_ALLOC_TH);
		warp_io_read32(wed, WED_RX_BM_ADD_PTR, &value);
		seq_printf(output, "WED_RX_BM_ADD_PTR\t:0x%04x\n", value);
		seq_printf(output, "\t(head2:0x%04x remain:0x%04x)\n", (value >> 16), (value & 0xffff));
		dump_io(output, wed, "WED_RX_BM_TOTAL_DMAD_IDX", WED_RX_BM_TOTAL_DMAD_IDX);
	}
#endif	/* WED_DYNAMIC_RXBM_SUPPORT */

#if defined(CONFIG_WARP_HW_DDR_PROF)
	dump_string(output, "==========WPDMA DRAM Latency info:==========\n");
	dump_io(output, wed, "WED_PMTR_CTRL", WED_PMTR_CTRL);
	dump_io(output, wed, "WED_PMTR_TGT", WED_PMTR_TGT);
	dump_io(output, wed, "WED_PMTR_TGT_ST", WED_PMTR_TGT_ST);
	dump_io(output, wed, "WED_PMTR_LTCY_MAX0_1", WED_PMTR_LTCY_MAX0_1);
	dump_io(output, wed, "WED_PMTR_LTCY_ACC1", WED_PMTR_LTCY_ACC1);
	warp_io_read32(wed, WED_PMTR_LTCY_MAX0_1, &max_ltcy);
	max_ltcy &= ((0xffff) << WED_PMTR_LTCY_MAX0_1_FLD_MST1_CNT); //WED_PMTR_LTCY_MAX0_1_FLD_MST1_CNT
	max_ltcy >>= WED_PMTR_LTCY_MAX0_1_FLD_MST1_CNT;
	warp_io_read32(wed, WED_PMTR_CTRL, &max_div);
	if (max_div & (0x1 << WED_PMTR_CTRL_FLD_EN)) {
		max_div &= (0xf << WED_PMTR_CTRL_FLD_MAX_DIV);
		warp_io_read32(wed, WED_PMTR_LTCY_ACC1, &acc);
		warp_io_read32(wed, WED_WPDMA_RX_D_RX0_PROCESSED_MIB, &rx_cnt[0]);
		warp_io_read32(wed, WED_WPDMA_RX_D_RX1_PROCESSED_MIB, &rx_cnt[1]);
		seq_printf(output, "%s\t:%d ticks\n", "MAX DDR Latency", max_ltcy);
		if ((rx_cnt[0]+rx_cnt[1]))
			seq_printf(output, "%s\t:%d ticks\n", "AVG DDR Latency",
						(acc*(1 << max_div))/(rx_cnt[0]+rx_cnt[1]));
		else
			seq_printf(output, "Invalid total packet count(%d)!", (rx_cnt[0]+rx_cnt[1]));
	} else
		seq_printf(output, "DDR access latency profiling is not enabled!\n");
#endif	/* CONFIG_WARP_HW_DDR_PROF */
}

/*
*
*/
void
warp_dbginfo_dump_wed_hw(struct wed_entry *wed)
{
	warp_dbg(WARP_DBG_OFF, "==========WED DEBUG INFO:==========\n");
	dump_addr_value(wed, "WED_IRQ_MON", WED_IRQ_MON);
	dump_addr_value(wed, "WED_TX_BM_MON_CTRL", WED_TX_BM_MON_CTRL);
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
	dump_addr_value(wed, "WED_PCIE_CFG_ADDR_INTS", WED_PCIE_CFG_ADDR_INTS);
	dump_addr_value(wed, "WED_PCIE_CFG_ADDR_INTM", WED_PCIE_CFG_ADDR_INTM);
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
warp_wdma_tx_ring_get_hw(struct warp_ring *ring, u8 idx)
{
	u32 offset = idx * WDMA_RING_OFFSET;

	ring->hw_desc_base = WDMA_TX_BASE_PTR_0 + offset;
	ring->hw_cnt_addr  = WDMA_TX_MAX_CNT_0 + offset;
	ring->hw_cidx_addr = WDMA_TX_CTX_IDX_0 + offset;
	ring->hw_didx_addr = WDMA_TX_DTX_IDX_0 + offset;
}


void
warp_rx_data_ring_get_hw(struct warp_entry *warp, struct warp_rx_ring *ring,
			 u8 idx)
{
	u32 offset = idx * WED_RING_OFFSET;

	ring->hw_desc_base = wifi_cr_get(warp, WED_RX_BASE_PTR_0 + offset);
	ring->hw_cnt_addr = wifi_cr_get(warp, WED_RX_MAX_CNT_0 + offset);
	ring->hw_cidx_addr = wifi_cr_get(warp, WED_RX_CRX_IDX_0 + offset);
	ring->hw_didx_addr = wifi_cr_get(warp, WED_RX_DRX_IDX_0 + offset);
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

	warp_io_read32(wed, WED_RX_BASE_PTR_0 + offset, base);
	warp_io_read32(wed, WED_RX_MAX_CNT_0 + offset, tcnt);
	warp_io_read32(wed, WED_RX_CRX_IDX_0 + offset, cidx);
	warp_io_read32(wed, WED_RX_DRX_IDX_0 + offset, didx);
}

void
warp_wdma_tx_ring_get_idx(struct wdma_entry *wdma, u32 idx,
	u32 *base, u32 *tcnt, u32 *cidx, u32 *didx)
{
	u32 offset = idx * WDMA_RING_OFFSET;

	warp_io_read32(wdma, WDMA_TX_BASE_PTR_0 + offset, base);
	warp_io_read32(wdma, WDMA_TX_MAX_CNT_0 + offset, tcnt);
	warp_io_read32(wdma, WDMA_TX_CTX_IDX_0 + offset, cidx);
	warp_io_read32(wdma, WDMA_TX_DTX_IDX_0 + offset, didx);
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

int
warp_wed_get_info(struct wed_entry *wed, struct warp_info *info)
{
	return -EOPNOTSUPP;
}

void warp_woif_bus_init_hw(struct woif_bus *bus)
{
	struct warp_bus_ring *ring;

	/*set PDMA & WED_WPDMA Ring, wifi driver will configure WDMA ring by warp_hal_tx_ring_ctrl */
	/*tx ring*/
	ring = bus_to_tx_ring(bus);
	warp_dbg(WARP_DBG_OFF, "%s(): bus:%p, txring: %x, pa: %pad, %pad, %x, %x, %x\n", __func__,
		bus,
		ring->hw_desc_base, &ring->cell[0].alloc_pa, &ring->desc.alloc_pa,
		ring->hw_cnt_addr,
		ring->hw_cidx_addr,
		ring->hw_didx_addr);

	warp_io_write32((struct warp_io *) bus->hw, ring->hw_desc_base, ring->cell[0].alloc_pa);
	warp_io_write32((struct warp_io *) bus->hw, ring->hw_cnt_addr, bus_to_tx_ring_len(bus));
	warp_io_write32((struct warp_io *) bus->hw, ring->hw_cidx_addr, 0);

	/*rx ring*/
	ring = bus_to_rx_ring(bus);
	warp_dbg(WARP_DBG_OFF, "%s(): bus:%p, rxring: %x, pa: %pad, %pad, %x, %x, %x\n", __func__,
		bus,
		ring->hw_desc_base, &ring->cell[0].alloc_pa, &ring->desc.alloc_pa,
		ring->hw_cnt_addr,
		ring->hw_cidx_addr,
		ring->hw_didx_addr);

	warp_io_write32((struct warp_io *) bus->hw, ring->hw_desc_base, ring->cell[0].alloc_pa);
	warp_io_write32((struct warp_io *) bus->hw, ring->hw_cnt_addr, bus_to_rx_ring_len(bus));
	warp_io_write32((struct warp_io *) bus->hw, ring->hw_cidx_addr, 0);
}


u32 warp_woif_bus_get_tx_res(struct woif_bus *bus)
{
	u32 cidx, didx;
	struct warp_bus_ring *tx_ring = bus_to_tx_ring(bus);
	struct wo_ring_ctrl *tx_ctrl = &bus->tx_ring;

	warp_io_read32((struct warp_io *) bus->hw, tx_ring->hw_cidx_addr, &cidx);
	warp_io_read32((struct warp_io *) bus->hw, tx_ring->hw_didx_addr, &didx);

	cidx = (cidx + 1) % tx_ctrl->ring_len;
	if (cidx == didx)
		return -1;

	return cidx;
}


void warp_woif_bus_get_rx_res(struct woif_bus *bus, u32 *didx, u32 *cidx)
{
	struct warp_bus_ring *rx_ring = bus_to_rx_ring(bus);

	warp_io_read32((struct warp_io *) bus->hw, rx_ring->hw_didx_addr, didx);
	warp_io_read32((struct warp_io *) bus->hw, rx_ring->hw_cidx_addr, cidx);
}


void warp_woif_bus_set_rx_res(struct woif_bus *bus, u32 cidx)
{
	struct warp_bus_ring *rx_ring = bus_to_rx_ring(bus);

	warp_io_write32((struct warp_io *) bus->hw, rx_ring->hw_cidx_addr, cidx);
}



void warp_woif_bus_kickout(struct woif_bus *bus, int cpu_idx)
{
	struct warp_bus_ring *tx_ring  = bus_to_tx_ring(bus);

	warp_io_write32((struct warp_io *) bus->hw, tx_ring->hw_cidx_addr, cpu_idx);

}


int
warp_dummy_cr_set(struct wed_entry *wed, u8 index, u32 value)
{
	if (index > 7) /* WED_SCR0 ~ WED_SCR7)*/
		return -1;

	warp_io_write32(wed, WED_SCR0 + 4 * index, value);

	return 0;
}

int
warp_dummy_cr_get(struct wed_entry *wed, u8 index, u32 *value)
{
	if (index > 7) /* WED_SCR0 ~ WED_SCR7)*/
		return -1;

	warp_io_read32(wed, WED_SCR0 + 4 * index, value);

	return 0;
}

#if defined(WED_DYNAMIC_TXBM_SUPPORT) || defined(WED_DYNAMIC_RXBM_SUPPORT)
/*
*
*/
void
dybm_eint_ctrl(struct wed_entry *wed, bool enable, u8 type)
{
	u32 mask = 0;

	switch(type) {
		case WARP_DYBM_EINT_BM_H:
			mask = (0x1 << WED_EX_INT_STA_FLD_TX_BM_HTH);
			break;
		case WARP_DYBM_EINT_BM_L:
			mask = (0x1 << WED_EX_INT_STA_FLD_TX_BM_LTH);
			break;
		case WARP_DYBM_EINT_RXBM_H:
			mask = (0x1 << WED_EX_INT_STA_FLD_RX_BM_H_BUF);
			break;
		case WARP_DYBM_EINT_RXBM_L:
			mask = (0x1 << WED_EX_INT_STA_FLD_RX_BM_L_BUF);
			break;
		case WARP_DYBM_EINT_TKID_H:
			mask = (0x1 << WED_EX_INT_STA_FLD_TX_TKID_HTH);
			break;
		case WARP_DYBM_EINT_RXBM_HL:
			mask = ((0x1 << WED_EX_INT_STA_FLD_RX_BM_H_BUF) | (0x1 << WED_EX_INT_STA_FLD_RX_BM_L_BUF));
			break;
		case WARP_DYBM_EINT_TKID_L:
			mask = (0x1 << WED_EX_INT_STA_FLD_TX_TKID_LTH);
			break;
		default:
			mask = ((0x1 << WED_EX_INT_STA_FLD_TX_BM_LTH) | (0x1 << WED_EX_INT_STA_FLD_TX_BM_HTH));
			mask |= ((0x1 << WED_EX_INT_STA_FLD_TX_TKID_LTH) | (0x1 << WED_EX_INT_STA_FLD_TX_TKID_HTH));
	}

	if (enable) {
		wed->ext_int_mask |= mask;
	} else {
		wed->ext_int_mask &= ~mask;
	}
	warp_eint_ctrl_hw(wed, true);
}
#endif	/* WED_DYNAMIC_TXBM_SUPPORT || WED_DYNAMIC_RXBM_SUPPORT */
/*
*
*/
void
warp_eint_work_hw(struct wed_entry *wed, u32 status, u32 err_status)
{
#ifdef WED_RX_SUPPORT

	if (status & (1 << WED_EX_INT_STA_FLD_RX_BM_FREE_AT_EMPTY))
		warp_dbg(WARP_DBG_ERR, "%s(): rx bm free at empty!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_BM_DMAD_RD_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): rx bm dmad rd err!\n", __func__);

#endif

	if (status & (1 << WED_EX_INT_STA_FLD_TF_LEN_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): tx free notify len error!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TF_TKID_WO_PYLD))
		warp_dbg(WARP_DBG_ERR, "%s(): tx free token has no packet to point!\n",
			 __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TX_BM_HTH)) {
#ifdef WED_DYNAMIC_TXBM_SUPPORT
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
			warp_dbg(WARP_DBG_INF, "%s(): tx buf high threshold!\n", __func__);
			dybm_eint_ctrl(wed, false, WARP_DYBM_EINT_BM_H);
			dybm_dl_int_disp(wed, WARP_DYBM_EINT_BM_H);
		} else
#endif /* WED_DYNAMIC_TXBM_SUPPORT */
			warp_dbg(WARP_DBG_ERR, "%s(): DYTXBM high threshold INT w/o enabled!\n", __func__);
	}

	if (status & (1 << WED_EX_INT_STA_FLD_TX_BM_LTH)) {
#ifdef WED_DYNAMIC_TXBM_SUPPORT
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
			warp_dbg(WARP_DBG_INF, "%s(): tx buf low threshold!\n", __func__);
			dybm_eint_ctrl(wed, false, WARP_DYBM_EINT_BM_L);
			dybm_dl_int_disp(wed, WARP_DYBM_EINT_BM_L);
		} else
#endif /* WED_DYNAMIC_TXBM_SUPPORT */
			warp_dbg(WARP_DBG_ERR, "%s(): DYTXBM low threshold INT w/o enabled!\n", __func__);
	}

	if (status & (1 << WED_EX_INT_STA_FLD_RX_BM_H_BUF)) {
#ifdef WED_DYNAMIC_RXBM_SUPPORT
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
			warp_dbg(WARP_DBG_INF, "%s(): rx buf high threshold!\n", __func__);
//			if ((WED_PKT_GRPNUM_GET(wed) - wed->sw_conf->rxbm.alt_quota) >= res->bm_rsv_grp) {
				dybm_eint_ctrl(wed, false, WARP_DYBM_EINT_RXBM_HL);
				/* intend to reverse action handler, due to H/W definition reversed */
				dybm_ul_int_disp(wed, WARP_DYBM_EINT_RXBM_L);
//			} else {
//				warp_dbg(WARP_DBG_ERR, "%s(): DYBM high threshold INT but BM will be less then reserved size. dismissed!\n", __func__);
//			}
		} else
#endif /* WED_DYNAMIC_RXBM_SUPPORT */
			warp_dbg(WARP_DBG_ERR, "%s(): DYRXBM high threshold INT w/o enabled!\n", __func__);
	}

	if (status & (1 << WED_EX_INT_STA_FLD_RX_BM_L_BUF)) {
#ifdef WED_DYNAMIC_RXBM_SUPPORT
		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_RXBM)) {
//			if ((WED_PKT_GRPNUM_GET(wed) + wed->sw_conf->rxbm.alt_quota) <= res->bm_max_grp) {
				warp_dbg(WARP_DBG_INF, "%s(): rx buf low threshold!\n", __func__);
				dybm_eint_ctrl(wed, false, WARP_DYBM_EINT_RXBM_HL);
				/* intend to reverse action handler, due to H/W definition reversed */
				dybm_ul_int_disp(wed, WARP_DYBM_EINT_RXBM_H);
//			} else {
//				warp_dbg(WARP_DBG_ERR, "%s(): DYBM high threshold INT but BM will exceed capability. dismissed!\n", __func__);
//			}
		} else
#endif /* WED_DYNAMIC_TXBM_SUPPORT */
			warp_dbg(WARP_DBG_ERR, "%s(): DYRXBM low threshold INT w/o enabled!\n", __func__);
	}

#if 0
	if (status & (1 << WED_EX_INT_STA_FLD_TX_TKID_HTH)) {
#ifdef WED_DYNAMIC_TXBM_SUPPORT
		struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;

		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
			warp_dbg(WARP_DBG_OFF, "%s(): token high threshold!\n", __func__);
			if ((WED_TOKEN_GRPNUM_GET(wed) - 1) >= res->tkn_rsv_grp) {
				//free token id
			} else {
				//free token id and tbuf
			}
		} else
#endif /* WED_DYNAMIC_TXBM_SUPPORT */
			warp_dbg(WARP_DBG_ERR, "%s(): DYTKID high threshold INT w/o enabled!\n", __func__);
	}

	if (status & (1 << WED_EX_INT_STA_FLD_TX_TKID_LTH)) {
#ifdef WED_DYNAMIC_TXBM_SUPPORT
		struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;

		if(IS_WED_HW_CAP(wed, WED_HW_CAP_DYN_TXBM)) {
			warp_dbg(WARP_DBG_INF, "%s(): token low threshold!\n", __func__);
			if (res->tkn_vld_grp <= res->tkn_max_grp) {
				warp_dbg(WARP_DBG_OFF, "%s(): token low threshold!\n", __func__);
				dybm_eint_ctrl(wed, false, WARP_DYBM_EINT_TKID_L);
				tasklet_hi_schedule(&wed->tkn_alloc_task);
			} else {
				//ignore, tkid exceed limit
			}
		} else
#endif /* WED_DYNAMIC_TXBM_SUPPORT */
			warp_dbg(WARP_DBG_ERR, "%s(): DYTKID low threshold INT w/o enabled!\n", __func__);
	}
#endif

	if (status & (1 << WED_EX_INT_STA_FLD_TX_DMA_W_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): tx dma write resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TX_DMA_R_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): tx dma read resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_INTI_WDMA_ENABLED))
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv inti wdma enable!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_COHERENT))
		warp_dbg(WARP_DBG_LOU, "%s(): rx drv coherent!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_W_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv write resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_RX_DRV_R_RESP_ERR))
		warp_dbg(WARP_DBG_ERR, "%s(): rx drv read resp err!\n", __func__);

	if (status & (1 << WED_EX_INT_STA_FLD_TF_TKID_FIFO_INVLD))
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
