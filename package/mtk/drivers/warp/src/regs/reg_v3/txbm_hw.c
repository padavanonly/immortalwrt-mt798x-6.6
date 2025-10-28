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

 txbm_hw.c (txbm hal for warp2.0)

*/
#include <warp.h>
#include <warp_hw.h>
#include <warp_utility.h>
#include <mcu/warp_fwdl.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include "mcu/warp_woif.h"
#include <warp_bm.h>

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
warp_txm_tkid_init_hw(struct wed_entry *wed)
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
	warp_txm_tkid_init_hw(wed);
	/*Reset Buf mgmt for ready to start*/
	value = 1 << WED_MOD_RST_FLD_TX_BM;
	WHNAT_RESET(wed, WED_MOD_RST, value);

	/* change to legacy TX BM */
	warp_io_read32(wed, WED_TX_BM_CTRL, &value);
	warp |= (1 << WED_TX_BM_CTRL_FLD_LEGACY_EN);
	warp_io_write32(wed, WED_TX_BM_CTRL, value);

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

