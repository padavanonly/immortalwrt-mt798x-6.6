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
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/


#ifdef MULTI_INF_SUPPORT

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>

/* Index 0 for Card_1, Index 1 for Card_2 */
VOID *adapt_list[MAX_NUM_OF_INF] = {NULL, NULL, NULL};

VOID *multi_inf_get_pAd_by_Band(VOID *pAd, UINT BandIdx)
{
	int idx;
	VOID *pRetAd = NULL;

	for (idx = 0; idx < MAX_NUM_OF_INF; idx++) {
		if (adapt_list[idx] != NULL) {
			if (hc_get_hw_band_idx(adapt_list[idx]) == BandIdx) {
				pRetAd = adapt_list[idx];
				break;
			}
		}
	}

	return pRetAd;
}

int multi_inf_active_cnt(void)
{
	int active_cnt = 0; /* use number 0 as default */
	int idx;

	for (idx = 0; idx < MAX_NUM_OF_INF; idx++) {
		if (adapt_list[idx] != NULL) {
			if (VIRTUAL_IF_NUM(adapt_list[idx]) != 0)
				active_cnt++;
		}
	}
	if (active_cnt == 0)
		MTWF_DBG(NULL, DBG_CAT_INIT, CATCFG_DBGLOG, DBG_LVL_ERROR,
			 "failed to find nonempty adapt_list!\n");
	return active_cnt;
}

int multi_inf_get_idx(VOID *pAd)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)pAd;

	return ad->dev_idx;
}
EXPORT_SYMBOL(multi_inf_get_idx);

/* Driver module load/unload function */
static int __init wifi_drv_init_module(void)
{
	int status = 0;

	os_module_init();

	return status;
}


static void __exit wifi_drv_cleanup_module(void)
{
	os_module_exit();
}

module_init(wifi_drv_init_module);
module_exit(wifi_drv_cleanup_module);
#endif /* MULTI_INF_SUPPORT */

