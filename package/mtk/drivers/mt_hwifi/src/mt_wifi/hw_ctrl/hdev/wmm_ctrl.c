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

*/

#include "rt_config.h"
#include "hdev/hdev.h"

/*WMM control*/


/*
  *
 */
static UINT32 wmm_ctrl_get_num(struct hdev_ctrl *ctrl)
{
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(ctrl);
	struct physical_device *ph_dev = ad->physical_dev;
	struct _RTMP_CHIP_CAP *cap = &ph_dev->chip_cap;

	return cap->qos.WmmHwNum;
}

/*Export Functions*/


/*
 *
*/
struct wmm_entry *wmm_ctrl_get_entry_by_idx(struct hdev_ctrl *ctrl, UINT32 idx)
{
	return &ctrl->HwResourceCfg.wmm_ctrl.entries[idx];
}

/*
 *
*/
INT32 wmm_ctrl_init(struct hdev_ctrl *ctrl, struct wmm_ctrl *wctrl)
{
	INT32 num = wmm_ctrl_get_num(ctrl);
	struct wmm_entry *entries = NULL;
	struct _EDCA_PARM *edca;
	INT32 i = 0;

	os_alloc_mem(NULL, (UCHAR **)&entries, sizeof(struct wmm_entry) * num);

	if (entries == NULL)
		return -1;

	os_zero_mem(entries, sizeof(struct wmm_entry) * num);
	wctrl->entries = entries;
	wctrl->num = num;

	for (i = 0; i < num; i++) {
		edca = &entries[i].edca;
		edca->bValid = FALSE;
		entries[i].wmm_set = i;
		entries[i].ref_cnt = 0;
		entries[i].dbdc_idx = 0;
		entries[i].tx_mode = HOBJ_TX_MODE_TXD;
	}

	return 0;
}


/*
 *
*/
INT32 wmm_ctrl_exit(struct wmm_ctrl *ctrl)
{
	if (ctrl->entries) {
		os_free_mem(ctrl->entries);
		ctrl->entries = NULL;
	}

	return 0;
}


/*
 *
*/
VOID wmm_ctrl_show_entry(struct wmm_ctrl *ctrl)
{
	INT i;
	struct _EDCA_PARM *edca = NULL;
	struct wmm_entry *entry;

	for (i = 0; i < ctrl->num; i++) {
		entry = &ctrl->entries[i];
		edca = &entry->edca;

		if (edca->bValid) {
			MTWF_PRINT("\tEdcaIdx: %d,BandIdx: %d, RfCnt: %d, TXMODE: %d\n",
				entry->wmm_set, entry->dbdc_idx, entry->ref_cnt, entry->tx_mode);
			MTWF_PRINT("\tAifs: %d/%d/%d/%d\n",
					 edca->Aifsn[0], edca->Aifsn[1], edca->Aifsn[2], edca->Aifsn[3]);
			MTWF_PRINT("\tTxop: %d/%d/%d/%d\n",
					 edca->Txop[0], edca->Txop[1], edca->Txop[2], edca->Txop[3]);
			MTWF_PRINT("\tCwmin: %d/%d/%d/%d\n",
					 edca->Cwmin[0], edca->Cwmin[1], edca->Cwmin[2], edca->Cwmin[3]);
			MTWF_PRINT("\tCwmax: %d/%d/%d/%d\n",
					 edca->Cwmax[0], edca->Cwmax[1], edca->Cwmax[2], edca->Cwmax[3]);
		}
	}
}

