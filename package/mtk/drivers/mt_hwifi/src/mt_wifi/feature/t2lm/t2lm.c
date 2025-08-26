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
    t2lm

    Abstract:
    t2lm

    Who             When            What
    --------------  ----------      --------------------------------------------

*/

#include "rt_config.h"

#ifndef offsetof
#define offsetof(TYPE, MEMBER)  ((size_t)&((TYPE *)0)->MEMBER)
#endif

extern struct bss_manager bss_mngr;

#ifdef DOT11_EHT_BE
static struct bmgr_entry *get_bss_entry_by_netdev(
	IN PNET_DEV pNetDev
)
{
	struct bmgr_entry *entry = NULL;
	int i;

	if (!pNetDev)
		return NULL;

	for (i = 0; BMGR_VALID_BSS_IDX(i); i++) {
		entry = bss_mngr.entry[i];

		if (entry && (entry->pwdev == RtmpOsGetNetDevWdev(pNetDev)))
			return entry;
	}

	return NULL;
}

/**
 * @find ap mld by mld_addr
 *
 * @param *mld_addr ap mld addr
 * @param **mld ap mld
 */
int find_ap_mld_by_mld_addr(
	IN u8 *mld_addr,
	OUT struct bmgr_mlo_dev **mld
)
{
	int ret = 0;
	struct bmgr_mlo_dev *_mld = NULL;
	struct query_mld_basic mld_basic = {0};
	u8 mld_grp = 0;

	COPY_MAC_ADDR(mld_basic.addr, mld_addr);
	if (bss_mngr_query_mld_basic(&mld_basic)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"MLD Lookup failed: MLD MAC (%pM)\n", mld_addr);
		ret = -EINVAL;
		goto err;
	}

	if (!BMGR_VALID_MLD_GRP_IDX(mld_basic.mld_grp_idx)) {
		ret = -EINVAL;
		goto err;
	}
	mld_grp = mld_basic.mld_grp_idx;

	_mld = GET_MLD_BY_GRP_IDX(mld_grp);
	if (!BMGR_VALID_MLO_DEV(_mld)) {
		ret = -EINVAL;
		goto err;
	}
	*mld = _mld;

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/* at2lm contract resource mgmt */

/**
 * @reset ap mld at2lm all contracts
 *
 * @param *mld ap mld
 */
void a2tlm_contract_reset(
	IN struct bmgr_mlo_dev *mld
)
{
	u8 i, link_id;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;
	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		at2lm_crt_ctl->contract[i].id = i;
		at2lm_crt_ctl->contract[i].state = AT2LM_STE_NONE;
		at2lm_crt_ctl->contract[i].sts = AT2LM_STS_FREE;
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++)
			at2lm_crt_ctl->contract[i].tid_map[link_id] = 0xff;
	}

	mutex_init(&at2lm_crt_ctl->lock);
}

/**
 * @allocate ap mld at2lm contract
 *
 * @param *mld ap mld
 * @param **at2lm_contract return at2lm contract
 */
int a2tlm_contract_alloc(
	IN struct bmgr_mlo_dev *mld,
	OUT struct at2lm_contract_t **at2lm_contract
)
{
	int ret = -ENOMEM;
	u8 i = 0;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;

	mutex_lock(&at2lm_crt_ctl->lock);
	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		if (at2lm_crt_ctl->contract[i].sts == AT2LM_STS_FREE) {
			at2lm_crt_ctl->contract[i].sts = AT2LM_STS_OCCUPY;
			at2lm_crt_ctl->contract[i].state = AT2LM_STE_NONE;
			*at2lm_contract = &at2lm_crt_ctl->contract[i];
			ret = 0;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
				"alloc contract id=%d\n", i);
			break;
		}
	}
	mutex_unlock(&at2lm_crt_ctl->lock);

	return ret;
}

/**
 * @get ap mld at2lm contract by id
 *
 * @param *mld ap mld
 * @param id contract id
 * @param **at2lm_contract return at2lm contract
 */
int a2tlm_contract_get(
	IN struct bmgr_mlo_dev *mld,
	IN u8 id,
	OUT struct at2lm_contract_t **at2lm_contract
)
{
	int ret = -ENOMEM;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;

	mutex_lock(&at2lm_crt_ctl->lock);
	if (at2lm_crt_ctl->contract[id].sts == AT2LM_STS_OCCUPY) {
		*at2lm_contract = &at2lm_crt_ctl->contract[id];
		ret = 0;
	}
	mutex_unlock(&at2lm_crt_ctl->lock);

	return ret;
}

/**
 * @release ap mld at2lm contract by id
 *
 * @param *mld ap mld
 * @param id contract id
 */
int a2tlm_contract_release(
	IN struct bmgr_mlo_dev *mld,
	IN u8 id
)
{
	int ret = -ENOMEM;
	u8 i = 0;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;

	if (!mld) {
		ret = -EINVAL;
		goto err;
	}

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;

	/* mld contract id release */
	mutex_lock(&at2lm_crt_ctl->lock);
	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		if (at2lm_crt_ctl->contract[i].id == id &&
			at2lm_crt_ctl->contract[i].sts == AT2LM_STS_OCCUPY) {
			at2lm_crt_ctl->contract[i].sts = AT2LM_STS_FREE;
			ret = 0;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
				"release contract id=%d\n", id);
			break;
		}
	}
	mutex_unlock(&at2lm_crt_ctl->lock);

	return ret;
err:
	return ret;
}

/* at2lm ie buffer mgmt */

/**
 * @allocate per BSS at2lm ie buffer
 *
 * @param *entry bmgr_entry
 */
int at2lm_ie_buf_alloc(
	IN struct bmgr_entry *entry
)
{
	int ret = 0;
	u8 i = 0;
	u8 *at2lm = NULL;

	if (!entry) {
		ret = -EINVAL;
		goto err;
	}

	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		if (!entry->at2lm_ctrl[i].at2lm.buf) {
			os_alloc_mem(NULL, (u8 **)&at2lm, BSS_MNGR_MAX_T2LM_IE_LEN);
			if (!at2lm) {
				ret = -ENOMEM;
				goto err;
			}
			NdisZeroMemory(at2lm, BSS_MNGR_MAX_T2LM_IE_LEN);
			entry->at2lm_ctrl[i].at2lm.buf = at2lm;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
				"wdev(%s) allocated (at2lm[%d]:%p)\n",
				entry->pNetDev->name, i, entry->at2lm_ctrl[i].at2lm.buf);
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
				"wdev(%s)already allocated (at2lm[%d]:%p)\n",
				entry->pNetDev->name, i, entry->at2lm_ctrl[i].at2lm.buf);
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"allocated t2lm[%d]:error\n", i);
	return ret;
}

/**
 * @free per BSS at2lm ie buffer
 *
 * @param *entry bmgr_entry
 */
int at2lm_ie_buf_free(
	IN struct bmgr_entry *entry
)
{
	int ret = 0;
	u8 i = 0;
	u8 *at2lm = NULL;

	if (!entry) {
		ret = -EINVAL;
		goto err;
	}

	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		at2lm = entry->at2lm_ctrl[i].at2lm.buf;
		if (at2lm)
			os_free_mem(at2lm);
	}

	return ret;
err:
	return ret;
}

/* t2lm ie */

/**
 * @update at2lm_contract tid_map with tid_map=0x00,
 *  remaining links with tid_map=0xff
 *
 * @param *mld ap mld
 * @param dis_link_id_bitmap disable link_id in bitmap
 * @param *tid_map return all links tid_map
 */
int at2lm_contract_dis_link_tid_map_update(
	IN struct bmgr_mlo_dev *mld,
	IN u16 dis_link_id_bitmap,
	OUT u8 *tid_map
)
{
	int ret = 0;
	u8 link_id = 0;
	u8 bss_idx = 0;
	struct bmgr_entry *entry = NULL;

	if (!mld) {
		ret = -EINVAL;
		goto err;
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		bss_idx = mld->bss_idx_mld[link_id];
		entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
		if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
			if (dis_link_id_bitmap & BIT(link_id))
				tid_map[link_id] = 0;
			else
				tid_map[link_id] = 0xff;
		} else
			tid_map[link_id] = 0;
	}

	return ret;
err:
	return ret;
}

/**
 * @update at2lm_contract tid_map with disable link tid_map=0xff,
 *  remaining links tid_map=0xff
 *
 * @param *mld ap mld
 * @param *entry bmgr_entry
 * @param *tid_map return all links tid_map
 */
int at2lm_contract_end_dis_link_tid_map_update(
	IN struct bmgr_mlo_dev *mld,
	OUT u8 *tid_map
)
{
	int ret = 0;
	u8 i = 0;
	u8 bss_idx = 0;
	struct bmgr_entry *entry = NULL;

	if (!mld) {
		ret = -EINVAL;
		goto err;
	}

	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
		bss_idx = mld->bss_idx_mld[i];
		entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
		if (BMGR_VALID_MLO_BSS_ENTRY(entry))
			tid_map[i] = 0xff;
		else
			tid_map[i] = 0;
	}

	return ret;
err:
	return ret;
}

/**
 * @based on t2lm_ctrl to build t2lm ie
 *
 * @param *wdev which BSS
 * @param *t2lm_ctrl parameters to build t2lm ie
 * @param mst mapping switch time
 * @param ed expected duration for contract state=ED
 * @param t2lm tid_map for disable link and remaining links
 * @param at2lm t2lm ie buffer of BSS
 */
void build_t2lm_ie(
	IN struct wifi_dev *wdev,
	IN struct t2lm_ctrl_t *t2lm_ctrl,
	IN u16 mst,	/*unit: TU*/
	IN u32 ed,	/*unit: TU*/
	IN u8 *t2lm,
	IN u8 present,
	OUT struct ie *at2lm
)
{
	struct _EID_STRUCT *eid = (struct _EID_STRUCT *)at2lm->buf;
	u8 link_id = 0, tid = 0, _t2lm = 0;
	u16 link_map_of_tid[TID_MAX] = {0};
	u8 octet_sz = 0;
	u8 *pos = at2lm->buf;
	u8 *f_buf = at2lm->buf;
	u8 ie_len = 0;

	if (!present) {
		eid->Len = 0;
		at2lm->len = 0;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
			"wdev(%s), t2lm_ie len=%d, buf=%p, len=%d\n",
			wdev->if_dev->name, eid->Len, at2lm->buf, at2lm->len);
		return;
	}

	if (eid) {
		eid->Eid = IE_WLAN_EXTENSION;
		eid->Octet[0] = EID_EXT_EHT_TID2LNK_MAP;
		pos += sizeof(struct _EID_STRUCT);

		NdisMoveMemory(pos, (u8 *)t2lm_ctrl, sizeof(struct t2lm_ctrl_t));
		pos += sizeof(struct t2lm_ctrl_t);

		if (t2lm_ctrl->mst_present) {
			NdisMoveMemory(pos, (u8 *)&mst, sizeof(mst));
			pos += sizeof(mst);
		}

		if (t2lm_ctrl->ed_present) {
			NdisMoveMemory(pos, (u8 *)&ed, 3);
			pos += 3;
		}

		if (t2lm_ctrl->def_link_map == 0) {
			octet_sz = t2lm_ctrl->link_map_sz == 1 ? 1 : 2;
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				_t2lm = t2lm[link_id];
				for (tid = 0; tid < 8; tid++)
					link_map_of_tid[tid] |= (((_t2lm & (1 << tid)) >> tid) << link_id);
			}

			for (tid = 0; tid < 8; tid++) {
				NdisMoveMemory(pos, &link_map_of_tid[tid], octet_sz);
				pos += octet_sz;
			}
		}

		/* eid_length */
		ie_len = pos - (u8 *)eid;
		if (ie_len != 0)
			eid->Len = (ie_len - 2);

		at2lm->len = pos - f_buf;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
			"wdev(%s), t2lm_ie len=%d, buf=%p, len=%d\n",
			wdev->if_dev->name, eid->Len, at2lm->buf, at2lm->len);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"error: eid=NULL\n");
	}
}

/**
 * @dump sta mld each link tid_map
 *
 * @param *mld_sta bmgr_mld_sta
 */
int mld_sta_tid_map_dump(
	IN struct bmgr_mld_sta *mld_sta
)
{
	int ret = 0;
	struct bmgr_mld_link *mld_link = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct eht_link_t *link_info = NULL;
	u8 link_id = 0, setup_link_id = 0;
	u16 mld_sta_idx = 0, setup_link_wcid = 0, pri = 0, sec = 0;
	struct peer_mld peer_mld_hw_info = {0};
	struct nt2lm_contract_t *nt2lm_contract;

	if (!mld_sta || !mld_sta->valid) {
		ret = -EINVAL;
		goto err;
	}
	nt2lm_contract = &mld_sta->nt2lm_contract;

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		if (!mld_link->active)
			continue;
		pEntry = mld_link->priv_ptr;
		if (pEntry) {
			struct mld_entry_t *mld_entry;

			mt_rcu_read_lock();
			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (mld_entry) {
				mld_sta_idx = mld_entry->mld_sta_idx;
				setup_link_id = mld_entry->setup_link_id;
				setup_link_wcid = mld_entry->setup_link_wcid;
			}
			mt_rcu_read_unlock();

			if (!hc_get_peer_mld(pEntry->wdev->sys_handle,
				pEntry->wcid, &peer_mld_hw_info)) {
				pri = peer_mld_hw_info.mld_primary_idx;
				sec = peer_mld_hw_info.mld_secondary_idx;
			}

			break;
		}
	}

	MTWF_PRINT("\t  - sta_mld=%pM, idx=%d, setup(link_id=%d,wcid=%d), pri=%d, sec=%d\n",
		mld_sta->mld_addr, mld_sta_idx, setup_link_id, setup_link_wcid, pri, sec);
	MTWF_PRINT("\t\tnt2lm_contract_info: in_nt2lm=%d\n", nt2lm_contract->in_nego_tid);
	MTWF_PRINT("\t\t          ");
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		pEntry = mld_link->priv_ptr;
		if (!mld_link->active || !pEntry) {
			MTWF_PRINT("L%02d_DL= N/A, ", link_id);
		} else {
			MTWF_PRINT("L%02d_DL=0x%.2x, ",
				 link_id, nt2lm_contract->tid_map_dl[link_id]);
		}
	}

	MTWF_PRINT("\n");

	MTWF_PRINT("\t\t          ");
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		pEntry = mld_link->priv_ptr;
		if (!mld_link->active || !pEntry) {
			MTWF_PRINT("L%02d_UL= N/A, ", link_id);
		} else {
			MTWF_PRINT("L%02d_UL=0x%.2x, ",
				 link_id, nt2lm_contract->tid_map_ul[link_id]);
		}
	}

	MTWF_PRINT("\n");
	MTWF_PRINT("\t\ttid_map_info:\n");
	MTWF_PRINT("\t\t          ");
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		pEntry = mld_link->priv_ptr;
		if (!mld_link->active || !pEntry) {
			MTWF_PRINT("L%02d_ST= N/A, ", link_id);
		} else {
			link_info = &pEntry->mlo.link_info;
			MTWF_PRINT("L%02d_ST= %s, ",
				 link_id,
				 link_info->link_sts == 0 ? "DIS" :
				 link_info->link_sts == 1 ? "ENA" :
				 "RMV");
		}
	}

	MTWF_PRINT("\n");

	MTWF_PRINT("\t\t          ");

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		pEntry = mld_link->priv_ptr;
		if (!mld_link->active || !pEntry) {
			MTWF_PRINT("L%02d_DL= N/A, ", link_id);
		} else {
			link_info = &pEntry->mlo.link_info;
			MTWF_PRINT("L%02d_DL=0x%.2x, ",
				 link_id, link_info->tid_map_dl);
		}
	}

	MTWF_PRINT("\n");

	MTWF_PRINT("\t\t          ");
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		pEntry = mld_link->priv_ptr;
		if (!mld_link->active || !pEntry) {
			MTWF_PRINT("L%02d_UL= N/A, ", link_id);
		} else {
			link_info = &pEntry->mlo.link_info;
			MTWF_PRINT("L%02d_UL=0x%.2x, ",
				 link_id, link_info->tid_map_ul);
		}
	}

	MTWF_PRINT("\n");

	return ret;
err:
	return ret;
}

/**
 * @dump sta mld nt2lm info
 *
 * @param wifi_dev *wdev
 */
int sta_mld_t2lm_show(IN struct wifi_dev *wdev)
{
	int ret = 0;
	u16 link_id = 0;
	struct mld_dev *mld = NULL;
	struct nt2lm_contract_t *nt2lm_contract = NULL;

	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	if (wdev->wdev_type != WDEV_TYPE_STA) {
		ret = -EPERM;
		goto err;
	}

	mld = wdev->mld_dev;
	if (mld != &mld_device) {
		ret = -EINVAL;
		goto err;
	}
	if (!mld->peer_mld.valid) {
		ret = -EINVAL;
		goto err;
	}

	nt2lm_contract = &mld->peer_mld.nt2lm_contract;
	MTWF_PRINT("\t\tnt2lm_contract_info: in_nt2lm=%d, dir=%d\n", nt2lm_contract->in_nego_tid,
		nt2lm_contract->dir);
	MTWF_PRINT("\t\t          ");
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		MTWF_PRINT("L%02d_DL=0x%.2x, ",
				link_id, nt2lm_contract->tid_map_dl[link_id]);
	}
	MTWF_PRINT("\n");
	MTWF_PRINT("\t\t          ");
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		MTWF_PRINT("L%02d_UL=0x%.2x, ",
				link_id, nt2lm_contract->tid_map_ul[link_id]);
	}
	MTWF_PRINT("\n");

err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @show all peer mld sta tid_map
 *
 * @param wifi_dev *wdev
 */
int mld_sta_tid_map_show(
	IN struct wifi_dev *wdev
)
{
	int ret = 0;
	struct bmgr_mld_sta *mld_sta = NULL;
	u16 i = 0;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
			mld_sta = GET_MLD_STA_BY_IDX(i);
			if (!mld_sta || !mld_sta->valid)
				continue;
			mld_sta_tid_map_dump(mld_sta);
		}
	} else if (wdev->wdev_type == WDEV_TYPE_STA)
		sta_mld_t2lm_show(wdev);
	else
		ret = -1;

	return ret;
}


/**
 * @show all ap mld tid_map
 *
 * @param void
 */
int mld_ap_tid_map_show(
	void
)
{
	int ret = 0;
	u16 i, j, k, link_id;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mld_sta *mld_sta = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;

	for (i = 0; i < BMGR_MAX_MLD_GRP_CNT; i++) {
		mld = GET_MLD_BY_GRP_IDX(i);
		if (BMGR_VALID_MLO_DEV(mld)) {
			at2lm_crt_ctl = &mld->at2lm_crt_ctl;
			MTWF_PRINT("- ap_mld[%02d]=%pM, link_cnt=%d\n",
				mld->mld_grp, mld->mld_addr, mld->mld_link_cnt);
			MTWF_PRINT("\t\tat2lm_contract_info:\n");
			mutex_lock(&at2lm_crt_ctl->lock);
			for (j = 0; j < BSS_MNGR_MAX_AT2LM_SET_NUM; j++) {
				MTWF_PRINT("\t\tat2lm[%d]: ", j);
				for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
					entry = GET_BSS_ENTRY_BY_IDX(mld->bss_idx_mld[link_id]);
					if (BMGR_VALID_MLO_BSS_ENTRY(entry))
						MTWF_PRINT("L%02d_BI=0x%.2x, ",
							link_id, at2lm_crt_ctl->contract[j].tid_map[link_id]);
					else
						MTWF_PRINT("L%02d_BI= N/A, ", link_id);
				}
				MTWF_PRINT("state=%s\n",
					((at2lm_crt_ctl->contract[j].state == AT2LM_STE_NONE) ? "NONE" :
					((at2lm_crt_ctl->contract[j].state == AT2LM_STE_MST) ? "MST" : "ED")));
			}
			mutex_unlock(&at2lm_crt_ctl->lock);

			MTWF_PRINT("\t\ttid_map_info:\n");
			MTWF_PRINT("\t\t          ");
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				entry = GET_BSS_ENTRY_BY_IDX(mld->bss_idx_mld[link_id]);
				if (BMGR_VALID_MLO_BSS_ENTRY(entry))
					MTWF_PRINT("L%02d_BI=0x%.2x, ",
						link_id, entry->tid_map);
				else
					MTWF_PRINT("L%02d_BI= N/A, ", link_id);
			}
			MTWF_PRINT("\n");

			for (k = 0; BMGR_VALID_MLD_STA(k); k++) {
				mld_sta = GET_MLD_STA_BY_IDX(k);
				if (!mld_sta || !mld_sta->valid)
					continue;
				if (mld_sta->mld_grp_idx != mld->mld_grp)
					continue;
				mld_sta_tid_map_dump(mld_sta);
			}
		}
	}

	return ret;
}

/**
 * @at2lm contract sanity check for mld
 *
 * @param *mld bmgr_mlo_dev
 * @param dis_link_id_bitmap bitmap of disable link id
 * @param *pass pass or fail
 */
int at2lm_mld_sanity_check(
	IN struct bmgr_mlo_dev *mld,
	IN u16 dis_link_id_bitmap,
	IN u8 *pass
)
{
	int ret = 0;
	u8 link_id = 0;
	u8 bss_idx = 0;
	u8 valid_bss_num = 0;
	u8 dis_bss_num = 0;
	struct bmgr_entry *entry = NULL;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		bss_idx = mld->bss_idx_mld[link_id];
		entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
		if (BMGR_VALID_MLO_BSS_ENTRY(entry))
			valid_bss_num++;
		if (dis_link_id_bitmap & BIT(link_id)) {
			dis_bss_num++;
			if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
					"link_id=%d is invalid\n", link_id);
				goto err;
			}
		}
	}

	if (valid_bss_num == dis_bss_num) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"disable all link_id is invalid\n");
		ret = -EINVAL;
		goto err;
	}

	*pass = 1;
	return ret;
err:
	*pass = 0;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @check if at2lm contract result in invalid tid_bitamp
 *
 * @param *mld disable bmgr_mlo_dev
 * @param dis_link_id_bitmap bitmap of disable link id
 * @param pass return pass or fail
 */
int at2lm_peer_mld_sanity_check(
	IN struct bmgr_mlo_dev *mld,
	IN u16 dis_link_id_bitmap,
	IN u8 *pass
)
{
	int ret = 0;
	struct bmgr_mld_sta *mld_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct eht_link_t *link_info = NULL;
	u16 i = 0;
	u8 link_id = 0;
	u8 tid_map = 0; /*BI*/
	u8 tid_map_dl = 0;
	u8 tid_map_ul = 0;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	*pass = 1;
	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		mld_sta = GET_MLD_STA_BY_IDX(i);
		if (!mld_sta || !mld_sta->valid)
			continue;
		if (mld_sta->mld_grp_idx != mld->mld_grp)
			continue;
		tid_map = 0;
		tid_map_dl = 0;
		tid_map_ul = 0;
		/*check if remaining links tid_map ok*/
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			mld_link = &mld_sta->mld_link[link_id];
			if (!mld_link->active)
				continue;
			pEntry = mld_link->priv_ptr;
			if (!pEntry)
				continue;
			if (dis_link_id_bitmap & BIT(link_id))
				continue;
			link_info = &pEntry->mlo.link_info;
			if (link_info->link_sts == LINK_STS_REMOVE)
				continue;
			tid_map_dl |= link_info->tid_map_dl;
			tid_map_ul |= link_info->tid_map_ul;
		}

		tid_map = tid_map_dl & tid_map_ul;
		if (tid_map != 0xff) {
			*pass = 0;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				 "dis_link_id_bitmap=0x%.4x\n", dis_link_id_bitmap);
			mld_sta_tid_map_dump(mld_sta);
			break;
		}
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @check if entry is disabled link or not
 *
 * @param *mld bmgr_mlo_dev
 * @param dis_link_id_bitmap bitmap of disable link id
 * @param repted_entry reported entry
 * @param *is_dis_link is disable link or not
 */
int at2lm_is_disable_entry(
	IN struct bmgr_mlo_dev *mld,
	IN u16 dis_link_id_bitmap,
	IN struct bmgr_entry *repted_entry,
	OUT u8 *is_dis_link
)
{
	int ret = 0;
	u8 link_id = 0;
	u8 bss_idx = 0;
	struct bmgr_entry *entry = NULL;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	*is_dis_link = 0;
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		if (dis_link_id_bitmap & BIT(link_id)) {
			bss_idx = mld->bss_idx_mld[link_id];
			entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(entry) &&
				entry == repted_entry) {
				*is_dis_link = 1;
				break;
			}
		}
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @get current tsf of wdev
 *
 * @param *wdev which BSS
 */
UINT64 at2lm_curr_tsf_get(
	IN struct wifi_dev *wdev
)
{
	UINT32 current_tsf[2] = {0};

	/* TODO, AT2LM: modify this cmd with fw_idx instead of hw_idx */
	if (HW_GET_TSF(wdev, current_tsf) == NDIS_STATUS_SUCCESS)
		return ((UINT64)current_tsf[0] + ((UINT64)current_tsf[1] << 32));
	else
		return 0;
}

/**
 * @get expected duration
 *
 * @param *wdev which BSS
 * @param tsf mst tsf or e tsf
 */
u32 at2lm_ed_get(
	IN struct wifi_dev *wdev,
	IN UINT64 tsf
)
{
	if (tsf > at2lm_curr_tsf_get(wdev))
		return (u32)(((tsf - at2lm_curr_tsf_get(wdev)) >> 10) & 0x00ffffff);
	else
		return 0;
}

/**
 * @apply mld contract[id] to link at2lm_ctrl[id] and build at2lm ie with the
 *  same buffer used by BCN/ProbeRsp/AssocRsp
 *
 * @param *at2lm_contract mld level at2lm contract
 */
void at2lm_ie_build(
	IN struct at2lm_contract_t *at2lm_contract
)
{
	struct bmgr_entry *dis_entry = NULL;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct at2lm_ctrl_t *at2lm_ctrl = NULL;
	struct t2lm_ctrl_t t2lm_ctrl = {0};
	UINT64 tsf_offset = 0;
	u16 mst = 0;
	u32 ed = 0;
	u8 id = 0;
	u8 contract_state = 0;
	u8 i = 0;
	u8 bss_idx = 0;
	u8 is_dis_entry = 0;
	u16 dis_link_id_bitmap = 0;

	dis_entry = at2lm_contract->dis_entry;
	dis_link_id_bitmap = at2lm_contract->dis_link_id_bitmap;
	id = at2lm_contract->id;
	contract_state = at2lm_contract->state;
	mld = dis_entry->mld_ptr;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"contract state=%d\n", contract_state);

	/* < T0 */
	if (contract_state == AT2LM_STE_NONE) {
		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			bss_idx = mld->bss_idx_mld[i];
			entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
				at2lm_ctrl = &entry->at2lm_ctrl[id];
				at2lm_ctrl->present = 0;
			}
		}
	/* T0 <= T < T1 */
	} else if (contract_state == AT2LM_STE_MST) {
		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			bss_idx = mld->bss_idx_mld[i];
			entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
				at2lm_ctrl = &entry->at2lm_ctrl[id];
				at2lm_ctrl->present = 1;
				t2lm_ctrl.dir = T2LM_BI;
				t2lm_ctrl.def_link_map = 0;
				t2lm_ctrl.mst_present = 1;
				t2lm_ctrl.ed_present = 1;
				t2lm_ctrl.link_map_sz = (BSS_MNGR_MAX_BAND_NUM > 8) ? 0 : 1;
				t2lm_ctrl.link_map_ind = 0xff;
				at2lm_is_disable_entry(mld,
					dis_link_id_bitmap,
					entry,
					&is_dis_entry);
				if (is_dis_entry) {
					at2lm_ctrl->mst_tsf = at2lm_contract->mst_tsf;
					at2lm_ctrl->ed_tsf = at2lm_contract->ed_tsf;
				} else {
					/*unit:2u*/
					tsf_offset = entry->tsf_offset[dis_entry->link_id] * 2;
					at2lm_ctrl->mst_tsf = at2lm_contract->mst_tsf + tsf_offset;
					at2lm_ctrl->ed_tsf = at2lm_contract->ed_tsf + tsf_offset;
				}

				mst = (u16)((at2lm_ctrl->mst_tsf >> 10) & 0xffff);
				ed = (at2lm_contract->mst_dur >> 10) & 0x00ffffff;

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
					"wdev(%s, %d), dis=%d, present=%d, mst_tsf=%lld, ed_tsf=%lld, mst=%d TU, ed=%d TU, state=%d\n",
					entry->pNetDev->name,
					entry->entry_info.hw_bss_index,
					is_dis_entry,
					at2lm_ctrl->present,
					at2lm_ctrl->mst_tsf,
					at2lm_ctrl->ed_tsf,
					mst, ed,
					contract_state);

				build_t2lm_ie(
					entry->pwdev,
					&t2lm_ctrl,
					mst,
					ed,
					at2lm_contract->tid_map,
					at2lm_ctrl->present,
					&at2lm_ctrl->at2lm);

				hex_dump_with_cat_and_lvl("T2LM_IE:",
					at2lm_ctrl->at2lm.buf, at2lm_ctrl->at2lm.len,
					DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO);
			}
		}
	/* T1 <= T < T2 */
	} else if (contract_state == AT2LM_STE_ED) {
		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			bss_idx = mld->bss_idx_mld[i];
			entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
				at2lm_ctrl = &entry->at2lm_ctrl[id];
				t2lm_ctrl.dir = T2LM_BI;
				t2lm_ctrl.def_link_map = 0;
				t2lm_ctrl.mst_present = 0;
				t2lm_ctrl.ed_present = 1;
				t2lm_ctrl.link_map_sz = (BSS_MNGR_MAX_BAND_NUM > 8) ? 0 : 1;
				t2lm_ctrl.link_map_ind = 0xff;
				at2lm_is_disable_entry(mld,
					dis_link_id_bitmap,
					entry,
					&is_dis_entry);
				if (is_dis_entry) {
					at2lm_ctrl->present = 0;
					at2lm_ctrl->mst_tsf = at2lm_contract->mst_tsf;
					at2lm_ctrl->ed_tsf = at2lm_contract->ed_tsf;
				} else {
					at2lm_ctrl->present = 1;
					/*unit:2u*/
					tsf_offset = entry->tsf_offset[dis_entry->link_id] * 2;
					at2lm_ctrl->mst_tsf = at2lm_contract->mst_tsf + tsf_offset;
					at2lm_ctrl->ed_tsf = at2lm_contract->ed_tsf + tsf_offset;
				}

				mst = (u16)((at2lm_ctrl->mst_tsf >> 10) & 0xffff);
				ed = (at2lm_contract->e_dur >> 10) & 0x00ffffff;

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
					"wdev(%s, %d), dis=%d, present=%d, mst_tsf=%lld, ed_tsf=%lld, mst=%d TU, ed=%d TU, state=%d\n",
					entry->pNetDev->name,
					entry->entry_info.hw_bss_index,
					is_dis_entry,
					at2lm_ctrl->present,
					at2lm_ctrl->mst_tsf,
					at2lm_ctrl->ed_tsf,
					mst, ed,
					contract_state);

				build_t2lm_ie(
					entry->pwdev,
					&t2lm_ctrl,
					mst,
					ed,
					at2lm_contract->tid_map,
					at2lm_ctrl->present,
					&at2lm_ctrl->at2lm);

				hex_dump_with_cat_and_lvl("T2LM_IE:",
					at2lm_ctrl->at2lm.buf, at2lm_ctrl->at2lm.len,
					DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO);
			}
		}
	}
}

/**
 * @generate T2LM IE and queried by BCN/ProbeRSP/AssocRsp
 *  BCN: ED is counted down by WM
 *  ProbeRsp: ED is counted down by Driver
 *  AssocRsp: ED is counted down by Driver
 *
 * @param *wdev which BSS
 * @param *f_buf_hdr BCN header address
 * @param *f_buf_last BCN last address
 * @param frame_type BCN=0, ProbeRsp=1, AssocRsp=2
 */
u16 at2lm_ie_query(
	IN struct wifi_dev *wdev,
	IN u8 *f_buf_hdr,
	IN u8 *f_buf_last,
	IN u8 frame_type
)
{
	PNET_DEV pNetDev = NULL;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	struct at2lm_ctrl_t *at2lm_ctrl = NULL;
	struct at2lm_ie_info_t *at2lm_info = NULL;
	struct ie *at2lm = NULL;
	u8 *pos = f_buf_last;
	u32 ed = 0;
	u8 i = 0;
	u8 id = 0;
	u16 len = 0;
	int ret = 0;

	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	if (wdev->wdev_type != WDEV_TYPE_AP)
		return 0;

	pNetDev = wdev->if_dev;
	if (!pNetDev) {
		ret = -EINVAL;
		goto err;
	}

	entry = get_bss_entry_by_netdev(pNetDev);
	if (!entry) {
		ret = -EINVAL;
		goto err;
	}

	if (!BMGR_VALID_BSS_ENTRY(entry)) {
		ret = -EINVAL;
		goto err;
	}

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_BSS_ENTRY(entry) || !BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}
	at2lm_crt_ctl = &mld->at2lm_crt_ctl;

	switch (frame_type) {
	case AT2LM_BCN:
		at2lm_info = &wdev->bcn_buf.at2lm_info;
		at2lm_info->vld_id = 0;
		for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
			at2lm_contract = &at2lm_crt_ctl->contract[i];
			at2lm_info->ie_offset[i] = 0;
			if (at2lm_contract->sts == AT2LM_STS_FREE)
				continue;
			if (at2lm_contract->state == AT2LM_STE_NONE)
				continue;
			id = at2lm_contract->id;
			at2lm_ctrl = &entry->at2lm_ctrl[id];
			at2lm = &at2lm_ctrl->at2lm;
			if (at2lm->buf && at2lm_ctrl->present) {
				/* update ED: MST won't be changed, keep ED and updated by WM */
				if (at2lm_contract->state == AT2LM_STE_MST) {
					ed = (at2lm_contract->e_dur >> 10) & 0x00ffffff;
					NdisMoveMemory(at2lm->buf + OFFSET_ED_WHEN_STE_MST, (u8 *)&ed, 3);
				}
				/* update ED: No MST, keep ED and updated by WM */
				if (at2lm_contract->state == AT2LM_STE_ED) {
					ed = (at2lm_contract->e_dur >> 10) & 0x00ffffff;
					NdisMoveMemory(at2lm->buf + OFFSET_ED_WHEN_STE_ED, (u8 *)&ed, 3);
				}

				NdisMoveMemory(pos, at2lm->buf, at2lm->len);
				at2lm_info->vld_id |= (1 << i);
				at2lm_info->ie_offset[id] = (u16)(pos - f_buf_hdr);
				pos += at2lm->len;
				len += at2lm->len;
			}
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_DEBUG,
				 "wdev(%s): frm=%d, id=%d, present=%d, len=%d, offset=%d\n",
				 entry->pNetDev->name,
				 AT2LM_BCN,
				 id, at2lm_ctrl->present,
				 at2lm->len,
				 at2lm_info->ie_offset[id]);
		}
		break;
	case AT2LM_PROBE_RSP:
		for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
			at2lm_contract = &at2lm_crt_ctl->contract[i];
			if (at2lm_contract->sts == AT2LM_STS_FREE)
				continue;
			if (at2lm_contract->state == AT2LM_STE_NONE)
				continue;
			id = at2lm_contract->id;
			at2lm_ctrl = &entry->at2lm_ctrl[id];
			at2lm = &at2lm_ctrl->at2lm;
			if (at2lm->buf && at2lm_ctrl->present) {
				/* update ED: MST won't be changed, just update ED */
				if (at2lm_contract->state == AT2LM_STE_MST) {
					ed = (at2lm_contract->e_dur >> 10) & 0x00ffffff;
					NdisMoveMemory(at2lm->buf + OFFSET_ED_WHEN_STE_MST, (u8 *)&ed, 3);
				}
				/* update ED: No MST, just update ED */
				if (at2lm_contract->state == AT2LM_STE_ED) {
					ed = at2lm_ed_get(wdev, at2lm_ctrl->ed_tsf);
					NdisMoveMemory(at2lm->buf + OFFSET_ED_WHEN_STE_ED, (u8 *)&ed, 3);
				}

				NdisMoveMemory(pos, at2lm->buf, at2lm->len);
				pos += at2lm->len;
				len += at2lm->len;
			}
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_DEBUG,
				 "wdev(%s): frm=%d, id=%d, present=%d, len=%d, ste=%d, ed=%d\n",
				 entry->pNetDev->name,
				 AT2LM_PROBE_RSP,
				 id, at2lm_ctrl->present,
				 at2lm->len,
				 at2lm_contract->state,
				 ed);
		}
		break;
	case AT2LM_ASSOC:
		for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
			at2lm_contract = &at2lm_crt_ctl->contract[i];
			if (at2lm_contract->sts == AT2LM_STS_FREE)
				continue;
			if (at2lm_contract->state != AT2LM_STE_ED)
				continue;
			id = at2lm_contract->id;
			at2lm_ctrl = &entry->at2lm_ctrl[id];
			at2lm = &at2lm_ctrl->at2lm;
			if (at2lm->buf && at2lm_ctrl->present) {
				/* update ED: No MST, just update ED */
				ed = at2lm_ed_get(wdev, at2lm_ctrl->ed_tsf);
				NdisMoveMemory(at2lm->buf + OFFSET_ED_WHEN_STE_ED, (u8 *)&ed, 3);

				NdisMoveMemory(pos, at2lm->buf, at2lm->len);
				pos += at2lm->len;
				len += at2lm->len;
			}
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
				 "wdev(%s): frm=%d, id=%d, present=%d, len=%d, ste=%d, ed=%d\n",
				 entry->pNetDev->name,
				 AT2LM_ASSOC,
				 id, at2lm_ctrl->present,
				 at2lm->len,
				 at2lm_contract->state,
				 ed);
		}
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "wdev(%s) with unknown frm=%d\n", entry->pNetDev->name, frame_type);
		ret = -EINVAL;
		goto err;

		break;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_DEBUG,
		 "wdev(%s): frm=%d, total_len=%d\n",
		 entry->pNetDev->name,
		 frame_type,
		 len);

	return len;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_DEBUG,
		 "err=%d\n", ret);

	return 0;
}

/**
 * @convert link status with tid_map
 *
 * @param tid_map_dl tid map dl
 * @param tid_map_ul tid map ul
 */
u8 tid_2_link_sts(
	IN u8 tid_map_dl,
	IN u8 tid_map_ul
)
{
	return (tid_map_dl & tid_map_ul) ? LINK_STS_ENABLE : LINK_STS_DISABLE;
}

/**
 * @convert driver mld_grp to wm fw_mld_id
 *
 * @param mld_grp mld grp
 */
u8 mld_grp_2_fw_mld_id(
	IN u8 mld_grp
)
{
	return 0xff;
}

/**
 * @convert wm fw_mld_id to driver mld_grp
 *
 * @param fw_mld_id wm fw mld id
 */
u8 fw_mld_id_2_mld_grp(
	IN u8 fw_mld_id
)
{
	return 0xff;
}

/**
 * @update peer mld sta tid_map such as assoc phase
 *
 * @param *mld_sta bmgr_mld_sta
 * @param *tid_map_dl tid_map_dl
 * @param *tid_map_ul tid_map_ul
 */
int at2lm_peer_tid_map_update(
	IN struct bmgr_mld_sta *mld_sta,
	IN u8 *tid_map_dl,
	IN u8 *tid_map_ul
)
{
	int ret = 0;
	struct bmgr_mld_link *mld_link = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct eht_link_t *link_info = NULL;
	u8 link_id = 0;

	if (!mld_sta || !mld_sta->valid) {
		ret = -EINVAL;
		goto err;
	}

	/* peer link tid_map update and set h/w by UniCmdStaRecEhtBasic */
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		if (!mld_link->active)
			continue;
		pEntry = mld_link->priv_ptr;
		if (!pEntry)
			continue;
		link_info = &pEntry->mlo.link_info;
		link_info->tid_map_dl = tid_map_dl[link_id];
		link_info->tid_map_ul = tid_map_ul[link_id];
		link_info->link_sts = tid_2_link_sts(link_info->tid_map_dl, link_info->tid_map_ul);
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @update AT2LM result to STA tid_map when assoc
 *
 * @param *mld ap mld
 * @param *mld_sta peer sta mld
 */
int at2lm_update_peer_mld_t2lm_at_assoc(
	IN struct bmgr_mlo_dev *mld,
	IN struct bmgr_mld_sta *mld_sta
)
{
	int ret = 0;
	struct at2lm_contract_t *at2lm_contract = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	u8 i = 0;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	if (!mld_sta || !mld_sta->valid) {
		ret = -EINVAL;
		goto err;
	}

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;

	mutex_lock(&at2lm_crt_ctl->lock);
	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		at2lm_contract = &at2lm_crt_ctl->contract[i];
		if (at2lm_contract->sts != AT2LM_STS_OCCUPY)
			continue;
		if (at2lm_contract->state != AT2LM_STE_ED)
			continue;
		if (at2lm_contract->to_type != AT2LM_TO_TYPE_MST_TSF)
			continue;
		at2lm_peer_tid_map_update(mld_sta,
			at2lm_contract->tid_map,
			at2lm_contract->tid_map);
		break;
	}
	mutex_unlock(&at2lm_crt_ctl->lock);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @update AT2LM result to STA tid_map when timeout
 *
 * @param *mld_addr mld addr
 * @param mld_grp mld group
 * @param id AT2LM contract id in this AP mld
 * @param tid_map tid to be set
 */
int at2lm_update_peer_mld_t2lm_at_expiry(
	IN u8 *mld_addr,
	IN u8 mld_grp,
	IN u8 id
)
{
	int ret = 0;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_sta *mld_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct eht_link_t *link_info = NULL;
	u8 link_id = 0;
	u16 i = 0;

	mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	if (!NdisEqualMemory(mld_addr, mld->mld_addr, MAC_ADDR_LEN)) {
		ret = -EINVAL;
		goto err;
	}

	ret = a2tlm_contract_get(mld, id, &at2lm_contract);
	if (ret) {
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		mld_sta = GET_MLD_STA_BY_IDX(i);
		if (!mld_sta || !mld_sta->valid)
			continue;
		if (mld_sta->mld_grp_idx != mld_grp)
			continue;
		nt2lm_contract = &mld_sta->nt2lm_contract;
		if (at2lm_contract->to_type == AT2LM_TO_TYPE_E_TSF &&
			nt2lm_contract->in_nego_tid) {
			ret = nt2lm_peer_tid_map_update(at2lm_contract->dis_entry->pwdev,
				mld, mld_sta);
			if (ret) {
				ret = -EINVAL;
				goto err;
			}
		} else {
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				mld_link = &mld_sta->mld_link[link_id];
				if (!mld_link->active)
					continue;
				pEntry = mld_link->priv_ptr;
				if (!pEntry)
					continue;
				link_info = &pEntry->mlo.link_info;
				link_info->tid_map_dl = at2lm_contract->tid_map[link_id];
				link_info->tid_map_ul = at2lm_contract->tid_map[link_id];
				link_info->link_sts = tid_2_link_sts(link_info->tid_map_dl, link_info->tid_map_ul);
			}
		}
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @update AT2LM result to BSS tid_map
 *
 * @param *mld_addr mld addr
 * @param mld_grp mld group
 * @param id AT2LM contract id in this AP mld
 */
int at2lm_update_mld_t2lm(
	IN u8 *mld_addr,
	IN u8 mld_grp,
	IN u8 id
)
{
	int ret = 0;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_entry *entry = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	u8 link_id = 0;
	u8 bss_idx = 0;

	mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	if (!NdisEqualMemory(mld_addr, mld->mld_addr, MAC_ADDR_LEN)) {
		ret = -EINVAL;
		goto err;
	}

	ret = a2tlm_contract_get(mld, id, &at2lm_contract);
	if (ret) {
		ret = -ENOMEM;
		goto err;
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		bss_idx = mld->bss_idx_mld[link_id];
		entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
		if (!BMGR_VALID_MLO_BSS_ENTRY(entry))
			continue;
		entry->tid_map = at2lm_contract->tid_map[link_id];
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @get the reported BSS link status for rnr in reporting BSS
 *
 * @param *repted_entry reported BSS
 * @param *is_dis_link link status
 */
int at2lm_rnr_dli_get(
	IN struct bmgr_entry *repted_entry,
	IN u8 *is_dis_link
)
{
	int ret = 0;
	struct bmgr_mlo_dev *mld = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	u16 i = 0;
	u8 _is_dis_link = 0;

	if (!BMGR_VALID_MLO_BSS_ENTRY(repted_entry)) {
		ret = -EINVAL;
		goto err;
	}

	mld = repted_entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}
	at2lm_crt_ctl = &mld->at2lm_crt_ctl;

	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		at2lm_contract = &at2lm_crt_ctl->contract[i];
		if (at2lm_contract->sts != AT2LM_STS_OCCUPY)
			continue;
		if (at2lm_contract->state != AT2LM_STE_ED)
			continue;
		if (at2lm_contract->to_type != AT2LM_TO_TYPE_MST_TSF)
			continue;
		if (!at2lm_is_disable_entry(mld,
			at2lm_contract->dis_link_id_bitmap,
			repted_entry,
			&_is_dis_link))
			*is_dis_link = _is_dis_link;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_DEBUG,
		 "wdev(%s): is_dis_link=%d\n", repted_entry->pNetDev->name, *is_dis_link);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_DEBUG,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @get bss info index of disable link_ld
 *
 * @param *mld struct bmgr_mlo_dev
 * @param dis_link_id_bitmap disable link_id in bitmap
 * @param *dis_link_id_bss_info_idx bss_info_idx of dis_link_id
 */
int at2lm_dis_link_id_to_bss_info_idx(
	IN struct bmgr_mlo_dev *mld,
	IN u16 dis_link_id_bitmap,
	OUT u8 *dis_link_id_bss_info_idx
)
{
	int ret = 0;
	struct bmgr_entry *entry = NULL;
	u8 link_id = 0;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		if ((dis_link_id_bitmap & BIT(link_id)) == 0)
			continue;
		entry = GET_BSS_ENTRY_BY_IDX(mld->bss_idx_mld[link_id]);
		if (!BMGR_VALID_MLO_BSS_ENTRY(entry))
			continue;
		dis_link_id_bss_info_idx[link_id] = entry->entry_info.hw_bss_index;
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/* at2lm contract execution */

/**
 * @at2lm contract request
 *
 * @param *mld bmgr_mlo_dev
 * @param dis_link_id disable link id
 *        1. TSF reference
 *        2. one of dis_link_id_bitmap
 * @param dis_link_id_bitmap which link_id to be disabled
 * @param mst_dur mapping switch time duration (uint: us)
 * @param e_dur expected duration for contract state=ED (uint: us)
 */
int _at2lm_req(
	IN struct bmgr_mlo_dev *mld,
	IN u8 dis_link_id,
	IN u16 dis_link_id_bitmap,
	IN u32 mst_dur,		/*unit: us*/
	IN u32 e_dur		/*unit: us*/
)
{
	int ret = 0;
	u8 pass = 1;
	u8 bss_idx = 0;
	u8 link_id = 0;
	struct bmgr_entry *entry = NULL;
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	struct at2lm_res_req_t req = {0};
	struct at2lm_res_rsp_t rsp = {0};

	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	bss_idx = mld->bss_idx_mld[dis_link_id];
	entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
	if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		ret = -EINVAL;
		goto err;
	}

	wdev = entry->pwdev;
	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!pAd) {
		ret = -EINVAL;
		goto err;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"mld=%pM, dis_link_id(tsf,%s)=%d, dis_link_id_bitmap=0x%x\n",
		mld->mld_addr, wdev->if_dev->name,
		dis_link_id, dis_link_id_bitmap);

	if (at2lm_mld_sanity_check(mld, dis_link_id_bitmap, &pass)) {
		ret = -EINVAL;
		goto err;
	}

	if (!pass) {
		ret = -EINVAL;
		goto err;
	}

	if (at2lm_peer_mld_sanity_check(mld, dis_link_id_bitmap, &pass)) {
		ret = -EINVAL;
		goto err;
	}

	if (!pass) {
		ret = -EINVAL;
		goto err;
	}

	/* allocate at2lm contract */
	ret = a2tlm_contract_alloc(mld, &at2lm_contract);
	if (ret) {
		ret = -ENOMEM;
		goto err;
	}

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;
	mutex_lock(&at2lm_crt_ctl->lock);
	at2lm_contract->state = AT2LM_STE_MST;
	at2lm_contract->to_type = AT2LM_TO_TYPE_NONE;
	at2lm_contract->dis_entry = entry;
	at2lm_contract->mst_dur = mst_dur;
	at2lm_contract->e_dur = e_dur;
	at2lm_contract->dis_link_id_bitmap = dis_link_id_bitmap;

	/* alloate h/w resource */
	NdisMoveMemory(req.mld_addr, mld->mld_addr, MAC_ADDR_LEN);
	/* TODO, AT2LM: this mld_grp is fw side mld_grp NOT driver side mld_grp */
	req.fw_mld_idx = mld_grp_2_fw_mld_id(mld->mld_grp);
	req.flag = 0;
	req.at2lm_id = at2lm_contract->id;
	req.bss_info_idx = entry->entry_info.hw_bss_index; /*fw_idx*/
	req.en_mst_timer = 1;
	req.en_ed_timer = 1;
	req.mst_timer_adv_time = 50;  /* advance time */
	req.e_timer_adv_time = 0;
	req.mst_dur = mst_dur;
	req.e_dur = e_dur;
	req.dis_link_id_bitmap = at2lm_contract->dis_link_id_bitmap;
	at2lm_dis_link_id_to_bss_info_idx(mld,
		at2lm_contract->dis_link_id_bitmap,
		req.dis_link_id_bss_info_idx);
	mutex_unlock(&at2lm_crt_ctl->lock);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"req: mld_addr=%pM, mld_idx=%d, a_id=%d, fw_idx=%d\n",
		req.mld_addr, req.fw_mld_idx, req.at2lm_id, req.bss_info_idx);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"req: en_mst_timer=%d, en_ed_timer=%d, mst_adv=%d, e_adv=%d, mst_dur=%d, e_dur=%d\n",
		req.en_mst_timer, req.en_ed_timer,
		req.mst_timer_adv_time, req.e_timer_adv_time,
		req.mst_dur, req.e_dur);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"req: dis_link_id_bitmap=0x%.4x\n", req.dis_link_id_bitmap);
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
			"req: dis_link_id_bss_info_idx[%d]=%d\n",
			link_id,
			req.dis_link_id_bss_info_idx[link_id]);
	}

	ret = HW_GET_AT2LM_RES(wdev, &req, &rsp);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"rsp: ret=%d, sts=%d, mld_addr=%pM, fw_mld_idx=%d, a_id=%d\n",
		ret, rsp.sts, rsp.mld_addr, rsp.fw_mld_idx, rsp.at2lm_id);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"rsp: mst_tsf=%lld, ed_tsf=%lld\n",
		rsp.mst_tsf, rsp.ed_tsf);

	if (ret) {
		ret = -ENOMEM;
		goto err;
	}

	if (!(rsp.sts == 0 &&
		rsp.at2lm_id == req.at2lm_id &&
		NdisEqualMemory(rsp.mld_addr, req.mld_addr, MAC_ADDR_LEN))) {
		ret = -EBUSY;
		goto err;
	}

	at2lm_contract->mst_tsf = rsp.mst_tsf;
	at2lm_contract->ed_tsf = rsp.ed_tsf;
	at2lm_contract_dis_link_tid_map_update(mld, dis_link_id_bitmap, at2lm_contract->tid_map);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"contract[%d]: state=%d, sts=%d, dis_entry=%s\n",
		at2lm_contract->id, at2lm_contract->state,
		at2lm_contract->sts, entry->pNetDev->name);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"contract[%d]: mst_dur=%d us, e_dur=%d us, mst_tsf=%lld, ed_tsf=%lld\n",
		at2lm_contract->id,
		at2lm_contract->mst_dur, at2lm_contract->e_dur,
		at2lm_contract->mst_tsf, at2lm_contract->ed_tsf);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		"contract[%d]: tid_map[lk_id=0]=0x%x, tid_map[lk_id=1]=0x%x, tid_map[lk_id=2]=0x%x\n",
		at2lm_contract->id,
		at2lm_contract->tid_map[0],
		at2lm_contract->tid_map[1],
		at2lm_contract->tid_map[2]);

	/* build state=MST IE */
	if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_AT2LM) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"Error: bcn_bpcc_op_lock fail\n");
		ret = -EBUSY;
		goto err;
	}
	at2lm_ie_build(at2lm_contract);

	/* force BCN update*/
	UpdateBeaconHandler_BPCC((struct _RTMP_ADAPTER *)wdev->sys_handle,
		wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_AT2LM, TRUE);

	/* critical update: at2lm ie is not critical ie */

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	if (at2lm_contract && at2lm_contract->sts == AT2LM_STS_OCCUPY)
		a2tlm_contract_release(mld, at2lm_contract->id);

	return ret;
}

/**
 * @expiry for mst tsf
 *
 * @param pNetDev which BSS
 * @param *mld_addr mld_ addr
 * @param mld_grp driver viewpoint mld group id
 * @param id mld at2lm contract id
 * @param to_type timeout type
 */
int at2lm_msttsf_expiry(
	IN PNET_DEV pNetDev,
	IN u8 *mld_addr,
	IN u8 mld_grp,
	IN u8 id,
	IN u8 to_type
)
{
	int ret = 0;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	struct wifi_dev *wdev = NULL;

	/* pNetDev = disable link */
	if (!pNetDev) {
		ret = -EINVAL;
		goto err;
	}

	entry = get_bss_entry_by_netdev(pNetDev);
	if (!entry) {
		ret = -EINVAL;
		goto err;
	}

	wdev = entry->pwdev;
	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
		 "dis_entry(%s), mld_addr=%pM, mld_grp=%d, id=%d, to_type=%d\n",
		 wdev->if_dev->name,
		 mld_addr,
		 mld_grp,
		 id,
		 to_type);

	if (!BMGR_VALID_BSS_ENTRY(entry)) {
		ret = -EINVAL;
		goto err;
	}

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_BSS_ENTRY(entry) || !BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	ret = a2tlm_contract_get(mld, id, &at2lm_contract);
	if (ret) {
		ret = -ENOMEM;
		goto err;
	}

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;
	mutex_lock(&at2lm_crt_ctl->lock);
	/* build state=ED IE */
	if (at2lm_contract->state == AT2LM_STE_MST &&
		to_type == AT2LM_TO_TYPE_MST_TSF) {
		at2lm_contract->to_type = AT2LM_TO_TYPE_MST_TSF;
		at2lm_contract->state = AT2LM_STE_ED;
		at2lm_ie_build(at2lm_contract);
	} else {
		mutex_unlock(&at2lm_crt_ctl->lock);
		ret = -EINVAL;
		goto err;
	}
	mutex_unlock(&at2lm_crt_ctl->lock);

	/* self AP MLD */
	at2lm_update_mld_t2lm(mld_addr, mld_grp, id);
	/* peer MLD STA */
	at2lm_update_peer_mld_t2lm_at_expiry(mld_addr, mld_grp, id);

	wdev->bcn_buf.stop_tx |= STOP_BCN_TX_T2LM;

	/* remaining links: rnr */
	bss_mngr_ie_update(entry->pwdev);
	bss_mngr_sync_bcn_update(entry->pwdev);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @expiry for expected tsf
 *
 * @param pNetDev which BSS
 * @param *mld_addr mld addr
 * @param mld_grp driver viewpoint mld group id
 * @param id mld at2lm contract id
 * @param to_type timeout type
 */
int at2lm_etsf_expiry(
	IN PNET_DEV pNetDev,
	IN u8 *mld_addr,
	IN u8 mld_grp,
	IN u8 id,
	IN u8 to_type
)
{
	int ret = 0;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	struct wifi_dev *wdev = NULL;

	/* pNetDev = disable link */
	if (!pNetDev) {
		ret = -EINVAL;
		goto err;
	}

	entry = get_bss_entry_by_netdev(pNetDev);
	if (!entry) {
		ret = -EINVAL;
		goto err;
	}

	wdev = entry->pwdev;
	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
		 "dis_entry(%s), mld_addr=%pM, mld_grp=%d, id=%d, to_type=%d\n",
		 wdev->if_dev->name,
		 mld_addr,
		 mld_grp,
		 id,
		 to_type);

	if (!BMGR_VALID_BSS_ENTRY(entry)) {
		ret = -EINVAL;
		goto err;
	}

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_BSS_ENTRY(entry) || !BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	ret = a2tlm_contract_get(mld, id, &at2lm_contract);
	if (ret) {
		ret = -ENOMEM;
		goto err;
	}

	at2lm_crt_ctl = &mld->at2lm_crt_ctl;
	mutex_lock(&at2lm_crt_ctl->lock);
	/* update at2lm contract */
	if (at2lm_contract->state == AT2LM_STE_ED &&
		to_type == AT2LM_TO_TYPE_E_TSF) {
		at2lm_contract->to_type = to_type;
		at2lm_contract->state = AT2LM_STE_NONE;
		at2lm_ie_build(at2lm_contract);
	} else {
		mutex_unlock(&at2lm_crt_ctl->lock);
		ret = -EINVAL;
		goto err;
	}
	mutex_unlock(&at2lm_crt_ctl->lock);

	at2lm_contract_end_dis_link_tid_map_update(mld, at2lm_contract->tid_map);
	/* self AP MLD */
	at2lm_update_mld_t2lm(mld_addr, mld_grp, id);
	/* peer MLD STA */
	at2lm_update_peer_mld_t2lm_at_expiry(mld_addr, mld_grp, id);

	wdev->bcn_buf.stop_tx &= ~STOP_BCN_TX_T2LM;

	/* remaining links: rnr */
	bss_mngr_ie_update(entry->pwdev);
	bss_mngr_sync_bcn_update(entry->pwdev);

	/* release at2lm contract */
	a2tlm_contract_release(mld, at2lm_contract->id);
	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	/* release at2lm contract */
	if (at2lm_contract)
		a2tlm_contract_release(mld, at2lm_contract->id);
	return ret;
}

/**
 * @at2lm contract request for wifi7 certi
 *
 * @param *wdev disable BSS
 * @param mst_dur mapping switch time duration (uint: ms)
 * @param e_dur expected duration for contract state=ED (uint: ms)
 */
int at2lm_req_certi(
	IN struct wifi_dev *wdev,
	IN u32 mst_dur,
	IN u32 e_dur
)
{
	int ret = 0;
	PNET_DEV pNetDev = NULL;
	struct bmgr_entry *entry = NULL;
	struct bmgr_entry *temp_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	u8 link_id = 0;
	u8 mld_bss_idx = 0;
	u16 dis_linkid_bitmap = 0;

	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	pNetDev = wdev->if_dev;
	if (!pNetDev) {
		ret = -EINVAL;
		goto err;
	}

	entry = get_bss_entry_by_netdev(pNetDev);
	if (!entry) {
		ret = -EINVAL;
		goto err;
	}

	if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		ret = -EINVAL;
		goto err;
	}

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	/* get dis_linkid_bitmap */
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_bss_idx = mld->bss_idx_mld[link_id];
		if (BMGR_VALID_BSS_IDX(mld_bss_idx)) {
			temp_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(temp_entry) && temp_entry == entry)
				dis_linkid_bitmap |= (1 << link_id);
		}
	}

	ret = _at2lm_req(mld, entry->link_id, dis_linkid_bitmap, mst_dur * 1000, e_dur * 1000);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @at2lm contract request for normal operation or EM R6
 *
 * @param mld_addr ap mld addr
 * @param dis_linkid_bitmap disabled link_id bitmap and 1st disabled link
 *        in dis_linkid_bitmap will be the tsf timing reference
 * @param mst_dur mapping switch time duration (uint: ms)
 * @param e_dur expected duration for contract state=ED (uint: ms)
 */
int at2lm_req(
	IN u8 *mld_addr,
	IN u16 dis_linkid_bitmap,
	IN u32 mst_dur,
	IN u32 e_dur
)
{
	int ret = 0;
	struct bmgr_mlo_dev *mld = NULL;
	u8 dis_link_id = 0;
	u8 link_id = 0;
	u8 mld_bss_idx = 0;
	u16 max_dis_linkid_bitmap = BITS(0, BSS_MNGR_MAX_BAND_NUM - 1);

	ret = find_ap_mld_by_mld_addr(mld_addr, &mld);
	if (ret) {
		ret = -EINVAL;
		goto err;
	}

	/* check dis_linkid_bitmap integrity */
	if (dis_linkid_bitmap == 0) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "dis_linkid_bitmap=0, fail\n");
		ret = -EINVAL;
		goto err;
	}

	if (dis_linkid_bitmap > max_dis_linkid_bitmap) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "dis_linkid_bitmap=0x%04x > support_linkid_bitmap=0x%04x, fail\n",
			 dis_linkid_bitmap, max_dis_linkid_bitmap);
		ret = -EINVAL;
		goto err;
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		if (dis_linkid_bitmap & (1 << link_id)) {
			mld_bss_idx = mld->bss_idx_mld[link_id];
			if (!BMGR_VALID_BSS_IDX(mld_bss_idx)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_WARN,
					 "mld=%pM, dis_linkid=%d of dis_linkid_bitmap=0x%04x is not valid\n",
					 mld_addr, link_id, dis_linkid_bitmap);
				ret = -EINVAL;
				goto err;
			}
		}
	}

	dis_link_id = ffs(dis_linkid_bitmap) - 1;

	ret = _at2lm_req(mld, dis_link_id, dis_linkid_bitmap, mst_dur * 1000, e_dur * 1000);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @at2lm tsf expire and call related tsf function such as
 *        at2lm_msttsf_expiry or at2lm_etsf_expiry
 *
 * @param *mld_addr ap mld addr
 * @param fw_mld_id fw mld id
 * @param id at2lm contract id
 * @param to_type timeout type
 */
int at2lm_tsf_expiry(
	IN u8 *mld_addr,
	IN u8 fw_mld_id,
	IN u8 id,
	IN u8 to_type
)
{
	int ret = 0;
	struct wifi_dev *wdev = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	u8 mld_grp = 0;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		 "mld_addr=%pM, fw_mld_id=%d, id=%d, to_type=%d\n",
		 mld_addr,
		 fw_mld_id,
		 id,
		 to_type);

	/* TODO, AT2LM: mld cmd fw_mld_idx is not ready yet
	 * use mld_addr to find correct mld_grp and
	 *
	 * below code will be removed when fw_mld_id->mld_grp ready
	 */
	ret = find_ap_mld_by_mld_addr(mld_addr, &mld);
	if (ret) {
		ret = -EINVAL;
		goto err;
	}
	mld_grp = mld->mld_grp;

	ret = a2tlm_contract_get(mld, id, &at2lm_contract);
	if (ret) {
		ret = -ENOMEM;
		goto err;
	}

	wdev = at2lm_contract->dis_entry->pwdev;
	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	switch (to_type) {
	case AT2LM_TO_TYPE_MST_TSF:
		ret = at2lm_msttsf_expiry(wdev->if_dev, mld_addr, mld_grp, id, to_type);
		break;
	case AT2LM_TO_TYPE_E_TSF:
		ret = at2lm_etsf_expiry(wdev->if_dev, mld_addr, mld_grp, id, to_type);
		break;
	default:
		break;
	}

	if (ret) {
		ret = -EPERM;
		goto err;
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @find mld sta by wcid
 *
 * @param ad PRTMP_ADAPTER
 * @param wcid wcid
 * @param **bmgr_sta bmgr_mld_sta
 */
int find_mld_sta_by_wcid(
	IN PRTMP_ADAPTER ad,
	IN u16 wcid,
	OUT struct bmgr_mld_sta **mld_sta
)
{
	int ret = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct bmgr_mld_sta *_mld_sta = NULL;
	u16 mld_sta_idx = 0;

	pEntry = entry_get(ad, wcid);
	mt_rcu_read_lock();
	if (pEntry->mlo.mlo_en)
		mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
	if (!mld_entry) {
		ret = -EINVAL;
		mt_rcu_read_unlock();
		goto err;
	}
	mld_sta_idx = mld_entry->mld_sta_idx;
	mt_rcu_read_unlock();

	if (!BMGR_VALID_MLD_STA(mld_sta_idx)) {
		ret = -EINVAL;
		goto err;
	}

	_mld_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
	if (!_mld_sta || !_mld_sta->valid) {
		ret = -EINVAL;
		goto err;
	}

	*mld_sta = _mld_sta;

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}


/**
 * @convert t2m ie link mapping to driver tid_map
 *
 * @param *t2lm_ie t2lm ie
 * @param *tid_map tid map of each link
 * @param *dir direction
 */
int nt2lm_t2lm_ie_link_map_to_tid_map(
	IN u8 *t2lm_ie,
	OUT u8 *tid_map,
	OUT u8 *dir
)
{
	int ret = 0;
	PEID_STRUCT eid = (PEID_STRUCT)(t2lm_ie);
	struct t2lm_ctrl_t t2lm_ctrl = {0};
	u16 link_map = 0;
	u8 tid, link_id, mld_link_max;

	if (eid->Eid != IE_WLAN_EXTENSION ||
		eid->Octet[0] != EID_EXT_EHT_TID2LNK_MAP) {
		ret = -EINVAL;
		goto err;
	}

	NdisMoveMemory(&t2lm_ctrl, &eid->Octet[1], sizeof(struct t2lm_ctrl_t));
	*dir = t2lm_ctrl.dir;
	for (link_id = 0; link_id < MLD_LINK_MAX; link_id++)
		tid_map[link_id] = 0;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		 "t2lm_ctrl=0x%x\n", *((u16 *)&t2lm_ctrl));

	for (tid = 0 ; tid < TID_MAX; tid++) {
		if (t2lm_ctrl.link_map_sz == 0) { /*2 octets*/
			mld_link_max = MLD_LINK_MAX;
			NdisMoveMemory(&link_map, &eid->Octet[3 + tid * 2], 2);
			link_map = cpu_to_le16(link_map);
		} else { /*1 octet*/
			mld_link_max = MLD_LINK_MAX / 2;
			NdisMoveMemory(&link_map, &eid->Octet[3 + tid * 1], 1);
		}
		for (link_id = 0; link_id < mld_link_max; link_id++)
			tid_map[link_id] |= ((link_map & (1 << link_id)) >> link_id) << tid;
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @get contract dl/ul tid_map from tid_map request and existed tid_map
 *
 * @param link_id link id
 * @param dir direction
 * @param tid_map tid_map request
 * @param *pEntry mac table entry
 * @param *tid_map_dl contract dl tid_map
 * @param *tid_map_ul contract ul tid_map
 */
void nt2lm_contract_dl_ul_tid_map_get(
	IN u8 link_id,
	IN u8 dir,
	IN u8 *tid_map,
	IN MAC_TABLE_ENTRY *pEntry,
	OUT u8 *tid_map_dl,
	OUT u8 *tid_map_ul
)
{
	if (dir == T2LM_BI) {
		tid_map_dl[link_id] = tid_map[link_id];
		tid_map_ul[link_id] = tid_map[link_id];
	} else if (dir == T2LM_DL) {
		tid_map_dl[link_id] = tid_map[link_id];
		tid_map_ul[link_id] = pEntry->mlo.link_info.tid_map_ul;
	} else { /* T2LM_UL */
		tid_map_dl[link_id] = pEntry->mlo.link_info.tid_map_dl;
		tid_map_ul[link_id] = tid_map[link_id];
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		 "link_id=%d, dir=%d, tid_map_dl[%d]=0x%.2x, tid_map_ul[%d]=0x%.2x\n",
		 link_id, dir,
		 link_id, tid_map_dl[link_id],
		 link_id, tid_map_ul[link_id]);
}

/**
 * @peer nt2lm send/receive request frame sanity check
 *
 * @param *ad PRTMP_ADAPTER
 * @param *wdev wdev
 * @param wcid wcid
 * @param *nt2lm_contract nt2lm contract parameters
 */
int nt2lm_request_sanity_check(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid,
	IN struct nt2lm_contract_t *nt2lm_contract
)
{
	int ret = 0;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mld_sta *mld_sta = NULL;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	u8 i = 0, link_id = 0;
	u8 t2l_nego_supp = 0;
	u8 ap_mld_t2l_nego_supp = 0;
	struct bmgr_mld_link *mld_link = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	u8 temp_tid_map = 0;
	u8 temp_tid_map_dl = 0;
	u8 temp_tid_map_ul = 0;
	u8 *tid_map_dl = NULL;
	u8 *tid_map_ul = NULL;
	u8 *tid_map = NULL;
	u8 dir = 0;

	if (!wdev || !ad || !nt2lm_contract) {
		ret = -EINVAL;
		goto err;
	}

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		if (find_mld_sta_by_wcid(ad, wcid, &mld_sta)) {
			ret = -EINVAL;
			goto err;
		}

		if (!mld_sta || !mld_sta->valid) {
			ret = -EINVAL;
			goto err;
		}

		tid_map_dl = nt2lm_contract->tid_map_dl;
		tid_map_ul = nt2lm_contract->tid_map_ul;
		tid_map = nt2lm_contract->tid_map;
		dir = nt2lm_contract->dir;

		entry = get_bss_entry_by_netdev(wdev->if_dev);
		if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
			ret = -EINVAL;
			goto err;
		}

		mld = entry->mld_ptr;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			ret = -EINVAL;
			goto err;
		}
		at2lm_crt_ctl = &mld->at2lm_crt_ctl;

		/* check nego t2lm capability */
		t2l_nego_supp = GET_DOT11BE_ML_BASIC_CMM_MLD_CAP_TID2LNK_NEGO_SUP(mld_sta->mld_caps);
		ap_mld_t2l_nego_supp = mld->attr.mld_caps.t2l_nego_supp;
		if (ap_mld_t2l_nego_supp != t2l_nego_supp) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				 "t2l_nego_supp=%d,%d\n",
				 ap_mld_t2l_nego_supp,
				 t2l_nego_supp);
			ret = -EINVAL;
			goto err;
		}

		mutex_lock(&at2lm_crt_ctl->lock);
		/* AP MLD in AT2LM
		 *
		 * TID mapping request is subset of AT2LM
		 * tid_map(BSS) = tid_map(BSS) | tid_map(STA)
		 */
		for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
			at2lm_contract = &at2lm_crt_ctl->contract[i];
			if (at2lm_contract->sts != AT2LM_STS_OCCUPY)
				continue;
			if (at2lm_contract->state != AT2LM_STE_ED)
				continue;
			if (at2lm_contract->to_type != AT2LM_TO_TYPE_MST_TSF)
				continue;
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				mld_link = &mld_sta->mld_link[link_id];
				if (!mld_link->active)
					continue;
				pEntry = mld_link->priv_ptr;
				if (!pEntry)
					continue;
				temp_tid_map_dl = at2lm_contract->tid_map[link_id];
				temp_tid_map_ul = at2lm_contract->tid_map[link_id];
				nt2lm_contract_dl_ul_tid_map_get(link_id,
					dir,
					tid_map,
					pEntry,
					tid_map_dl,
					tid_map_ul);
				if ((tid_map_dl[link_id] | temp_tid_map_dl) != temp_tid_map_dl) {
					mutex_unlock(&at2lm_crt_ctl->lock);
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
						 "subset tid_map error: tid_map_dl[%d]=0x%.2x, at2lm[%d].tid_map[%d]=0x%.2x\n",
						 link_id, tid_map_dl[link_id], i, link_id, at2lm_contract->tid_map[link_id]);
					ret = -EINVAL;
					goto err;
				}
				if ((tid_map_ul[link_id] | temp_tid_map_ul) != temp_tid_map_ul) {
					mutex_unlock(&at2lm_crt_ctl->lock);
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
						 "subset tid_map error: tid_map_ul[%d]=0x%.2x, at2lm[%d].tid_map[%d]=0x%.2x\n",
						 link_id, tid_map_ul[link_id], i, link_id, at2lm_contract->tid_map[link_id]);
					ret = -EINVAL;
					goto err;
				}
			}
		}
		mutex_unlock(&at2lm_crt_ctl->lock);

		/* Each TID is mapped */
		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			mld_link = &mld_sta->mld_link[link_id];
			if (!mld_link->active)
				continue;
			pEntry = mld_link->priv_ptr;
			if (!pEntry)
				continue;
			nt2lm_contract->link_id_bitmap |= (1 << link_id);
			nt2lm_contract->link_id_to_wcid[link_id] = pEntry->wcid;
			nt2lm_contract_dl_ul_tid_map_get(link_id,
				dir,
				tid_map,
				pEntry,
				tid_map_dl,
				tid_map_ul);
			temp_tid_map_dl |= tid_map_dl[link_id];
			temp_tid_map_ul |= tid_map_ul[link_id];
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
				 "link_id_to_wcid[%d]=%d\n",
				 link_id, nt2lm_contract->link_id_to_wcid[link_id]);
		}
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct mld_dev *mld = wdev->mld_dev;

		if (!mld || (mld != &mld_device)) {
			ret =  -1;
			goto err;
		}
		if (!mld->peer_mld.valid) {
			ret = -1;
			goto err;
		}

		tid_map_dl = nt2lm_contract->tid_map_dl;
		tid_map_ul = nt2lm_contract->tid_map_ul;
		tid_map = nt2lm_contract->tid_map;
		dir = nt2lm_contract->dir;

		for (i = 0; i < MLD_LINK_MAX; i++) {
			struct peer_mld_single_link *sta_mld_link;

			sta_mld_link = &mld->peer_mld.single_link[i];
			if (!sta_mld_link->active)
				continue;
			pEntry = sta_mld_link->priv_ptr;
			if (!pEntry)
				continue;
			link_id = sta_mld_link->link_id;
			nt2lm_contract->link_id_bitmap |= (1 << link_id);
			nt2lm_contract->link_id_to_wcid[link_id] = pEntry->wcid;
			nt2lm_contract_dl_ul_tid_map_get(link_id,
				dir,
				tid_map,
				pEntry,
				tid_map_dl,
				tid_map_ul);
			temp_tid_map_dl |= tid_map_dl[link_id];
			temp_tid_map_ul |= tid_map_ul[link_id];
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
				 "apcli link_id_to_wcid[%d]=%d\n",
				 link_id, nt2lm_contract->link_id_to_wcid[link_id]);
		}
	} else {
		ret = -EINVAL;
		goto err;
	}

	temp_tid_map = temp_tid_map_dl & temp_tid_map_ul;
	if (temp_tid_map != 0xff) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"not all TIDs are mapped error: tid_map=0x%.2x\n", temp_tid_map);
		ret = -EINVAL;
		goto err;
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;

}

/**
 * @peer mld nt2lm  rx request frame sanity check
 *
 * @param *ad PRTMP_ADAPTER
 * @param *wdev wdev
 * @param wcid wcid
 * @param *eht_prot_act struct eht_prot_action_frame
 * @param *dialog_token dialog token
 */
int nt2lm_peer_mld_tid_sanity_check(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid,
	IN struct eht_prot_action_frame *eht_prot_act,
	OUT u8 *dialog_token
)
{
	int ret = 0;
	u8 dir = 0;
	struct nt2lm_contract_t *nt2lm_contract = NULL;

	if (!wdev || !eht_prot_act || !dialog_token) {
		ret = -EINVAL;
		goto err;
	}

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		struct bmgr_mld_sta *mld_sta = NULL;

		find_mld_sta_by_wcid(ad, wcid, &mld_sta);
		if (!mld_sta || !mld_sta->valid) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"INVALID MLD_STA\n");
			ret = -EINVAL;
			goto err;
		}
		nt2lm_contract = &mld_sta->nt2lm_contract;
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct mld_dev *mld = wdev->mld_dev;

		if (mld != &mld_device) {
			ret =  -1;
			goto err;
		}
		if (!mld->peer_mld.valid) {
			ret = -1;
			goto err;
		}
		nt2lm_contract = &mld->peer_mld.nt2lm_contract;
	} else {
		ret = -EINVAL;
		goto err;
	}

	if (!nt2lm_contract) {
		ret = -EINVAL;
		goto err;
	}

	/* get t2lm ie link map and assign to nt2lm_contract with tid map */
	nt2lm_t2lm_ie_link_map_to_tid_map(&eht_prot_act->data[1],
		nt2lm_contract->tid_map,
		&dir);
	nt2lm_contract->dir = dir;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		 "dir=%d, tid_map=0x%.2x,0x%.2x,0x%.2x\n",
		 dir,
		 nt2lm_contract->tid_map[0],
		 nt2lm_contract->tid_map[1],
		 nt2lm_contract->tid_map[2]);

	if (dir != T2LM_BI) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
			 "dir!=T2LM_BI dir=%d\n", dir);
		ret = -EINVAL;
		goto err;
	}

	*dialog_token = eht_prot_act->data[0];

	ret = nt2lm_request_sanity_check(ad,
		wdev,
		wcid,
		nt2lm_contract);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @apply contract tid_map_dl[]/tid_map_ul[]
 *
 * @param *wdev wifi_dev
 * @param *mld bmgr_mlo_dev
 * @param *mld_sta bmgr_mld_sta
 */
int nt2lm_peer_tid_map_update(
	IN struct wifi_dev *wdev,
	IN struct bmgr_mlo_dev *mld,
	IN struct bmgr_mld_sta *mld_sta
)
{
	int ret = 0;
	struct nt2lm_req_t req = {0};
	struct bmgr_mld_link *mld_link = NULL;
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct eht_link_t *link_info = NULL;
	u8 i = 0;
	u8 link_id = 0;

	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	if (!mld) {
		ret = -EINVAL;
		goto err;
	}

	if (!mld_sta || !mld_sta->valid) {
		ret = -EINVAL;
		goto err;
	}

	/* h/w tid_map update */
	nt2lm_contract = &mld_sta->nt2lm_contract;
	COPY_MAC_ADDR(req.self_mld_addr, mld->mld_addr);
	COPY_MAC_ADDR(req.peer_mld_add, mld_sta->mld_addr);
	req.mld_idx = 0xff;
	req.link_id_bitmap = nt2lm_contract->link_id_bitmap;
	for (i = 0; i < MLD_LINK_MAX; i++) {
		req.link_id_to_wcid[i] = nt2lm_contract->link_id_to_wcid[i];
		req.tid_map_dl[i] = nt2lm_contract->tid_map_dl[i];
		req.tid_map_ul[i] = nt2lm_contract->tid_map_ul[i];
	}
	ret = HW_SET_NT2LM(wdev, &req);
	if (ret) {
		ret = -EINVAL;
		goto err;
	}

	/* peer link tid_map update */
	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		mld_link = &mld_sta->mld_link[link_id];
		if (!mld_link->active)
			continue;
		pEntry = mld_link->priv_ptr;
		if (!pEntry)
			continue;
		link_info = &pEntry->mlo.link_info;
		link_info->tid_map_dl = nt2lm_contract->tid_map_dl[link_id];
		link_info->tid_map_ul = nt2lm_contract->tid_map_ul[link_id];
		link_info->link_sts = tid_2_link_sts(link_info->tid_map_dl, link_info->tid_map_ul);
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

/**
 * @handle peer mld sta tid_map after peer's request
 *
 * @param *wdev wifi_dev
 * @param *mld bmgr_mlo_dev
 * @param *mld_sta bmgr_mld_sta
 */
int nt2lm_peer_tid_map_update_after_request(
	IN struct wifi_dev *wdev,
	IN struct bmgr_mlo_dev *mld,
	IN struct bmgr_mld_sta *mld_sta
)
{
	int ret = 0;

	if (!wdev || !mld || !mld_sta) {
		ret = -EINVAL;
		goto err;
	}

	ret = nt2lm_peer_tid_map_update(wdev, mld, mld_sta);
	if (ret) {
		ret = -EINVAL;
		goto err;
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @execute nt2lm request callback function with TXS
 *
 * @param *arg callback function argument
 * @param *txs_info struct txs_info_t
 */
int nt2lm_peer_tid_map_update_callback_after_request(
	void *arg,
	struct txs_info_t *txs_info
)
{
	int ret = 0;
	struct nt2lm_req_arg_t *callback_arg = NULL;

	if (!arg || !txs_info) {
		ret = -EINVAL;
		goto err1;
	}

	callback_arg = arg;

	if (txs_info->txs_sts == TXS_STS_OK &&
		callback_arg->status_code == MLME_SUCCESS) {
		struct nt2lm_contract_t *nt2lm_contract = NULL;
		struct wifi_dev *wdev = callback_arg->wdev;

		if (!wdev) {
			ret = -EINVAL;
			goto err1;
		}

		if (wdev->wdev_type == WDEV_TYPE_AP) {
			/* upadte t2lm tid_map */
			ret = nt2lm_peer_tid_map_update_after_request(callback_arg->wdev,
				callback_arg->mld,
				callback_arg->mld_sta);
			if (ret) {
				ret = -EBUSY;
				goto err2;
			}

			nt2lm_contract = &callback_arg->mld_sta->nt2lm_contract;
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			struct mld_dev *mld = wdev->mld_dev;

			if (mld != &mld_device) {
				ret = -EINVAL;
				goto err1;
			}
			if (!mld->peer_mld.valid) {
				ret = -EINVAL;
				goto err1;
			}

			ret = nt2lm_sta_mld_tid_map_update(wdev);
			if (ret) {
				ret = -EBUSY;
				goto err2;
			}

			nt2lm_contract = &mld->peer_mld.nt2lm_contract;
		} else {
			ret = -EINVAL;
			goto err1;
		}
		nt2lm_contract->in_nego_tid = 1;
	} else if (txs_info->txs_sts == TXS_STS_NG) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "TXS_STS_NG\n");
		ret = -EINVAL;
		goto err2;
	} else if (txs_info->txs_sts == TXS_STS_TO) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "TXS_STS_TO\n");
		ret = -EINVAL;
		goto err2;
	}

	return ret;

err1:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
err2:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d, txs_sts=%d\n", ret, txs_info->txs_sts);
	return ret;
}

/**
 * @handle peer mld sta tid_map after teardown by peer mld sta or
 *         ap mld
 *
 * @param *wdev wifi_dev
 * @param *mld bmgr_mlo_dev
 * @param *mld_sta bmgr_mld_sta
 */
int nt2lm_peer_tid_map_update_after_teardown(
	IN struct wifi_dev *wdev,
	IN struct bmgr_mlo_dev *mld,
	IN struct bmgr_mld_sta *mld_sta
)
{
	int ret = 0;
	struct at2lm_contract_ctrl_t *at2lm_crt_ctl = NULL;
	struct at2lm_contract_t *at2lm_contract = NULL;
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	u8 *tid_map_dl = NULL;
	u8 *tid_map_ul = NULL;
	u8 i = 0;
	u8 link_id = 0;
	u8 tid_map = 0;
	u8 in_at2lm = 0;

	if (!wdev || !mld || !mld_sta) {
		ret = -EINVAL;
		goto err;
	}

	/* in at2lm: restore to at2lm tid_map */
	at2lm_crt_ctl = &mld->at2lm_crt_ctl;
	nt2lm_contract = &mld_sta->nt2lm_contract;
	tid_map_dl = nt2lm_contract->tid_map_dl;
	tid_map_ul = nt2lm_contract->tid_map_ul;
	mutex_lock(&at2lm_crt_ctl->lock);
	for (i = 0; i < BSS_MNGR_MAX_AT2LM_SET_NUM; i++) {
		at2lm_contract = &at2lm_crt_ctl->contract[i];
		if (at2lm_contract->sts == AT2LM_STS_OCCUPY &&
			at2lm_contract->state == AT2LM_STE_ED) {
			in_at2lm = 1;
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				tid_map = at2lm_contract->tid_map[link_id];
				tid_map_dl[link_id] = tid_map;
				tid_map_ul[link_id] = tid_map;
			}
			break;
		}
	}
	mutex_unlock(&at2lm_crt_ctl->lock);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		 "in_at2lm=%d\n", in_at2lm);

	/* not in at2lm: restore to default tid_map */
	if (!in_at2lm) {
		for (i = 0; i < MLD_LINK_MAX; i++) {
			tid_map_dl[i] = 0xff;
			tid_map_ul[i] = 0xff;
		}
	}

	ret = nt2lm_peer_tid_map_update(wdev, mld, mld_sta);
	if (ret) {
		ret = -EINVAL;
		goto err;
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @apply nt2lm sta_mld update tid_map
 *
 * @param *wdev wifi_dev
 */
int nt2lm_sta_mld_tid_map_update(
	IN struct wifi_dev *wdev
)
{
	int ret = 0;
	UINT8 i;
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	struct nt2lm_req_t req = {0};
	struct _MAC_TABLE_ENTRY *tmp_entry = NULL;
	struct eht_link_t *link_info = NULL;
	struct peer_mld_single_link *sta_link = NULL;
	struct mld_dev *mld = NULL;

	mld = wdev->mld_dev;
	if (mld != &mld_device) {
		ret =  -1;
		goto err;
	}
	if (!mld->peer_mld.valid) {
		ret = -1;
		goto err;
	}

	nt2lm_contract = &mld->peer_mld.nt2lm_contract;
	COPY_MAC_ADDR(req.self_mld_addr, mld->mld_addr);
	COPY_MAC_ADDR(req.peer_mld_add, mld->peer_mld.mld_addr);
	req.mld_idx = 0xff;
	req.link_id_bitmap = nt2lm_contract->link_id_bitmap;
	for (i = 0; i < MLD_LINK_MAX; i++) {
		req.link_id_to_wcid[i] = nt2lm_contract->link_id_to_wcid[i];
		req.tid_map_dl[i] = nt2lm_contract->tid_map_dl[i];
		req.tid_map_ul[i] = nt2lm_contract->tid_map_ul[i];
	}
	ret = HW_SET_NT2LM(wdev, &req);
	if (ret) {
		ret = -EBUSY;
		goto err;
	}

	/* peer link tid_map update */
	for (i = 0; i < MLD_LINK_MAX; i++) {
		sta_link = &mld->peer_mld.single_link[i];
		if (!sta_link->active)
			continue;
		tmp_entry = sta_link->priv_ptr;
		if (!tmp_entry)
			continue;

		link_info = &tmp_entry->mlo.link_info;
		link_info->tid_map_dl = nt2lm_contract->tid_map_dl[sta_link->link_id];
		link_info->tid_map_ul = nt2lm_contract->tid_map_ul[sta_link->link_id];
		link_info->link_sts = tid_2_link_sts(link_info->tid_map_dl, link_info->tid_map_ul);
	}
	return ret;

err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @execute nt2lm teardown callback function with TXS
 *
 * @param *arg callback function argument
 * @param *txs_info struct txs_info_t
 */
int nt2lm_peer_tid_map_update_callback_after_teardown(
	void *arg,
	struct txs_info_t *txs_info
)
{
	int ret = 0;
	struct nt2lm_teardown_arg_t *callback_arg = NULL;

	if (!arg || !txs_info) {
		ret = -EINVAL;
		goto err1;
	}

	callback_arg = arg;

	if (txs_info->txs_sts == TXS_STS_OK) {
		struct nt2lm_contract_t *nt2lm_contract = NULL;

		/* upadte t2lm tid_map */
		ret = nt2lm_peer_tid_map_update_after_teardown(callback_arg->wdev,
			callback_arg->mld,
			callback_arg->mld_sta);
		if (ret) {
			ret = -EBUSY;
			goto err2;
		}

		nt2lm_contract = &callback_arg->mld_sta->nt2lm_contract;
		nt2lm_contract->in_nego_tid = 0;
	} else if (txs_info->txs_sts == TXS_STS_NG) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "TXS_STS_NG\n");
		ret = -EINVAL;
		goto err2;
	} else if (txs_info->txs_sts == TXS_STS_TO) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "TXS_STS_TO\n");
		ret = -EINVAL;
		goto err2;
	}

	return ret;

err1:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
err2:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d, txs_sts=%d\n", ret, txs_info->txs_sts);
	return ret;
}

/**
 * @execute sta mld nt2lm teardown callback function with TXS
 *
 * @param *arg callback function argument
 * @param *txs_info struct txs_info_t
 */
int nt2lm_sta_mld_tid_map_update_callback_after_teardown(
	void *arg,
	struct txs_info_t *txs_info
)
{
	int ret = 0;
	struct nt2lm_teardown_arg_t *callback_arg = NULL;
	struct wifi_dev *wdev = NULL;
	struct mld_dev *mld = NULL;
	int i;

	if (!arg || !txs_info) {
		ret = -EINVAL;
		goto err1;
	}

	callback_arg = arg;
	wdev = callback_arg->wdev;
	if (!wdev) {
		ret = -EINVAL;
		goto err1;
	}
	mld = wdev->mld_dev;
	if (mld != &mld_device) {
		ret = -EINVAL;
		goto err1;
	}
	if (!mld->peer_mld.valid) {
		ret = -EINVAL;
		goto err1;
	}

	if (txs_info->txs_sts == TXS_STS_OK) {
		struct nt2lm_contract_t *nt2lm_contract = &mld->peer_mld.nt2lm_contract;
		/* todo: check at2lm status*/

		/*set nt2lm to default mapping*/
		for (i = 0; i < MLD_LINK_MAX; i++) {
			nt2lm_contract->tid_map_dl[i] = 0xff;
			nt2lm_contract->tid_map_ul[i] = 0xff;
		}
		if (nt2lm_sta_mld_tid_map_update(wdev)) {
			ret = -EINVAL;
			goto err1;
		}
		/* upadte t2lm tid_map */
		nt2lm_contract->in_nego_tid = 0;
	} else if (txs_info->txs_sts == TXS_STS_NG) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "TXS_STS_NG\n");
		ret = -EINVAL;
		goto err2;
	} else if (txs_info->txs_sts == TXS_STS_TO) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			 "TXS_STS_TO\n");
		ret = -EINVAL;
		goto err2;
	}

	return ret;

err1:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
err2:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d, txs_sts=%d\n", ret, txs_info->txs_sts);
	return ret;
}

/**
 * @used to handle peer mld's nt2lm request action frame:
 *  EHT_PROT_ACT_T2LM_REQUEST
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param wcid which link receive this action frame
 * @param eht_prot_act eht_prot_action_frame
 */
int nt2lm_peer_t2lm_request_action(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid,
	IN struct eht_prot_action_frame *eht_prot_act
)
{
	int ret = 0;
	struct eht_prot_action_frame n2tlm_rsp = {0};
	struct _MAC_TABLE_ENTRY *peer_entry = NULL;
	u16 status_code = MLME_SUCCESS;
	u8 dialog_token = 0;
	u8 *out_buffer = NULL;
	ULONG frame_len;
	struct txs_callback_info_t *callback = NULL;
	struct nt2lm_req_arg_t *callback_arg = NULL;

	if (nt2lm_peer_mld_tid_sanity_check(ad, wdev, wcid,
		eht_prot_act, &dialog_token))
		status_code = MLME_DENIED_TID_TO_LINK_MAPPING;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		struct bmgr_mlo_dev *mld = NULL;
		struct bmgr_mld_sta *mld_sta = NULL;
		u8 mld_grp = 0;

		if (find_mld_sta_by_wcid(ad, wcid, &mld_sta)) {
			ret = -EINVAL;
			goto err;
		}

		if (!mld_sta || !mld_sta->valid) {
			ret = -EINVAL;
			goto err;
		}

		mld_grp = mld_sta->mld_grp_idx;
		mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			ret = -EINVAL;
			goto err;
		}

#ifdef RT_CFG80211_SUPPORT
		/* TODO, NT2LM: inform BTM to cancel disassoc */
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
				"%s calling event to stop disassoc imminent timer\n", __func__);

		nl80211_send_event_stop_disassoc_timer(ad->pCfg80211_CB->pCfg80211_Wdev->wiphy,
							 ad->pCfg80211_CB->pCfg80211_Wdev,
							 eht_prot_act,
							 sizeof(struct eht_prot_action_frame));
#endif /* RT_CFG80211_SUPPORT */

		if (TxsInitCallbackInfo(ad,
			&callback,
			nt2lm_peer_tid_map_update_callback_after_request,
			(VOID **)&callback_arg,
			sizeof(struct nt2lm_req_arg_t))
			== NDIS_STATUS_SUCCESS) {
			callback_arg->wdev = wdev;
			callback_arg->mld = mld;
			callback_arg->mld_sta = mld_sta;
			callback_arg->status_code = status_code;
		} else {
			ret = -EINVAL;
			goto err;
		}
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		if (TxsInitCallbackInfo(ad,
			&callback,
			nt2lm_peer_tid_map_update_callback_after_request,
			(VOID **)&callback_arg,
			sizeof(struct nt2lm_req_arg_t))
			== NDIS_STATUS_SUCCESS) {
			callback_arg->wdev = wdev;
			callback_arg->status_code = status_code;
		} else {
			ret = -EINVAL;
			goto err;
		}

	} else {
		ret = -EINVAL;
		goto err;
	}
	if (MlmeAllocateMemory(ad, &out_buffer) != NDIS_STATUS_SUCCESS) {
		TxsFreeCallbackInfo(callback);
		ret = -EINVAL;
		goto err;
	}
	/* send response */
	peer_entry = &ad->MacTab->Content[wcid];
	ActHeaderInit(ad, &(n2tlm_rsp.Hdr), peer_entry->Addr, wdev->if_addr, wdev->bssid);
	n2tlm_rsp.category = CATEGORY_PROTECTED_EHT;
	n2tlm_rsp.prot_eht_action = EHT_PROT_ACT_T2LM_RESPONSE;
	status_code = cpu_to_le16(status_code);

	MakeOutgoingFrame(out_buffer, &frame_len,
		sizeof(struct eht_prot_action_frame), &n2tlm_rsp,
		1, &dialog_token,
		2, &status_code,
		END_OF_ARGS);
	if (MiniportMMRequest(ad, QID_AC_BE, out_buffer, frame_len, callback)
		!= NDIS_STATUS_SUCCESS) {
		ret = -EBUSY;
		TxsFreeCallbackInfo(callback);
		MlmeFreeMemory(out_buffer);
		goto err;
	}

	hex_dump_with_cat_and_lvl("NT2LM_RSP:",
		out_buffer, frame_len,
		DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO);

	MlmeFreeMemory(out_buffer);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @called by net link cmd to trigger nt2lm request
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param wicd
 * @param *eht_prot_act NT2LM response frame
 * @param *status status in repsponse frame
 */
int nt2lm_peer_t2lm_rsp_sanity_check(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid,
	IN struct eht_prot_action_frame *eht_prot_act,
	OUT u8 *status
)
{
	int ret = 0;
	//u8 token; /*todo*/

	if (!eht_prot_act)
		return -1;
	*status = eht_prot_act->data[1];
	return ret;
}
#ifdef WAPP_SUPPORT
/**
 * @use in response wapp, after trigger NT2LM request
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param *cmd used to build response information
 */
int wapp_report_t2lm_response_status(
	IN PRTMP_ADAPTER ad, IN struct wifi_dev *wdev,
	IN struct nl_80211_t2lm_cmd *t2lm_cmd)
{
	int ret = 0;
	char *buf = NULL;
	UINT16 buflen = 0;
	struct wapp_event *event = NULL;

	if (!ad || !wdev || !t2lm_cmd) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_QOS_MAP, DBG_LVL_ERROR,
			"invalid pointer: ad:%p, wdev:%p, t2lm_cmd:%p\n", ad, wdev, t2lm_cmd);
		return -1;
	}

	buflen = offsetof(struct wapp_event, data) + sizeof(struct nl_80211_t2lm_cmd);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	if (!buf) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_QOS_MAP, DBG_LVL_ERROR,
			"alloc memory failed.\n");
		return -1;
	}
	NdisZeroMemory(buf, buflen);
	event = (struct wapp_event *)buf;
	event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event->event_id = WAPP_T2LM_RESPONSE_REPORT;
	NdisCopyMemory(&event->data, t2lm_cmd, sizeof(*t2lm_cmd));

	wapp_send_wapp_qry_rsp(ad, event);

	os_free_mem(buf);

	/* TODO, NT2LM: peer t2ln response action, no need in WFA 4.49/5.49 */
	return ret;
}
#endif
/**
 * @used to handle peer mld's nt2lm response action frame:
 *  EHT_PROT_ACT_T2LM_RESPONSE
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param wcid which link receive this action frame
 * @param eht_prot_act eht_prot_action_frame
 */
int nt2lm_peer_t2lm_response_action(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid,
	IN struct eht_prot_action_frame *eht_prot_act
)
{
	int ret = 0;
	u8 status = 0;
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	struct nt2lm_req_t req = {0};

	if (nt2lm_peer_t2lm_rsp_sanity_check(ad, wdev, wcid, eht_prot_act, &status)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"t2lm rsp sanity check fail\n");
		return -1;
	}

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		struct bmgr_mld_sta *mld_sta = NULL;
		struct bmgr_mlo_dev *mld = NULL;

		find_mld_sta_by_wcid(ad, wcid, &mld_sta);
		if (!mld_sta || !mld_sta->valid) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"INVALID MLD_STA\n");
			return -EINVAL;
		}
		mld = BMGR_VALID_MLD_GRP_IDX(mld_sta->mld_grp_idx) ? GET_MLD_BY_GRP_IDX(mld_sta->mld_grp_idx) : NULL;
		if (!BMGR_VALID_MLO_DEV(mld))
			return -1;

		COPY_MAC_ADDR(req.self_mld_addr, mld->mld_addr);
		COPY_MAC_ADDR(req.peer_mld_add, mld_sta->mld_addr);
		nt2lm_contract = &mld_sta->nt2lm_contract;
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct mld_dev *mld = wdev->mld_dev;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
			"MLD_STA(wdev=%s) get nt2lm_rsp\n",
			wdev->if_dev->name);

		if (mld != &mld_device)
			return -1;
		if (!mld->peer_mld.valid)
			return -1;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
			"MLD_STA(wdev=%s) peer_mld_add=%pM, self_mld_addr=%pM\n",
			wdev->if_dev->name, mld->peer_mld.mld_addr, mld->mld_addr);

		nt2lm_contract = &mld->peer_mld.nt2lm_contract;
		COPY_MAC_ADDR(req.self_mld_addr, mld->mld_addr);
		COPY_MAC_ADDR(req.peer_mld_add, mld->peer_mld.mld_addr);
	}
	if (nt2lm_contract == NULL)
		return -1;

	if (status == MLME_SUCCESS) {
		nt2lm_contract->in_nego_tid = 1;
		req.mld_idx = 0xff;
		req.link_id_bitmap = nt2lm_contract->link_id_bitmap;
		NdisMoveMemory(req.link_id_to_wcid, nt2lm_contract->link_id_to_wcid, MLD_LINK_MAX);
		NdisMoveMemory(req.tid_map_dl, nt2lm_contract->tid_map_dl, MLD_LINK_MAX);
		NdisMoveMemory(req.tid_map_ul, nt2lm_contract->tid_map_ul, MLD_LINK_MAX);
		HW_SET_NT2LM(wdev, &req);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"status=%d not handle\n", status);
		ret = -1;
	}

	/*sync to wapp*/
#ifdef WAPP_SUPPORT
	if (nt2lm_contract->request_type == REQ_FROM_WAPP) {
		struct nl_80211_t2lm_cmd t2lm_cmd = {0};

		t2lm_cmd.tid_bit_map = nt2lm_contract->tid_bit_map;
		NdisMoveMemory(t2lm_cmd.tid_to_link_bitmap, nt2lm_contract->link_map, TID_MAX);
		t2lm_cmd.dir = nt2lm_contract->dir;
		NdisMoveMemory(t2lm_cmd.ra_addr, wdev->bssid, MAC_ADDR_LEN);
		t2lm_cmd.status_code = status;
		wapp_report_t2lm_response_status(ad, wdev, &t2lm_cmd);
	}
#endif

	return ret;
}

/**
 * @used to handle peer mld's nt2lm teardown action frame:
 *  EHT_PROT_ACT_T2LM_TEARDOWN
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param wcid which link receive this action frame
 * @param eht_prot_act eht_prot_action_frame
 */
int nt2lm_peer_t2lm_teardown_action(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid,
	IN struct eht_prot_action_frame *eht_prot_act
)
{
	int ret = 0;
	struct nt2lm_contract_t *nt2lm_contract = NULL;

	/* peer t2lm teardown request action, no need in WFA 4.49/5.49 */
	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		u8 mld_grp = 0;
		struct bmgr_mlo_dev *mld = NULL;
		struct bmgr_mld_sta *mld_sta = NULL;

		if (find_mld_sta_by_wcid(ad, wcid, &mld_sta)) {
			ret = -EINVAL;
			goto err;
		}

		if (!mld_sta || !mld_sta->valid) {
			ret = -EINVAL;
			goto err;
		}

		mld_grp = mld_sta->mld_grp_idx;
		nt2lm_contract = &mld_sta->nt2lm_contract;
		mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			ret = -EINVAL;
			goto err;
		}

		/* upadte t2lm tid_map */
		ret = nt2lm_peer_tid_map_update_after_teardown(wdev, mld, mld_sta);
		if (ret) {
			ret = -EBUSY;
			goto err;
		}
		nt2lm_contract->in_nego_tid = 0;
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct mld_dev *mld = NULL;
		int i;

		mld = wdev->mld_dev;
		if (mld != &mld_device) {
			ret = -EINVAL;
			goto err;
		}
		if (!mld->peer_mld.valid) {
			ret = -EINVAL;
			goto err;
		}

		nt2lm_contract = &mld->peer_mld.nt2lm_contract;
		/* todo: check at2lm status*/

		/*set nt2lm to default mapping*/
		for (i = 0; i < MLD_LINK_MAX; i++) {
			nt2lm_contract->tid_map_dl[i] = 0xff;
			nt2lm_contract->tid_map_ul[i] = 0xff;
		}
		if (nt2lm_sta_mld_tid_map_update(wdev)) {
			ret = -EINVAL;
			goto err;
		}
		/* upadte t2lm tid_map */
		nt2lm_contract->in_nego_tid = 0;
	} else {
		ret = -EINVAL;
		goto err;
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @mld initiate nt2lm contract request to peer mld
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param wcid which link to send out request frame
 */
int nt2lm_t2lm_request(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid
)
{
	int ret = 0;
	u8 *out_buffer = NULL;
	struct _MAC_TABLE_ENTRY *peer_entry = NULL;
	struct eht_prot_action_frame n2tlm_req = {0};
	struct t2lm_ctrl_t t2lm_ctrl = {0};
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	ULONG frame_len;
	u8 dialog_token = 0;
	UCHAR eid = IE_WLAN_EXTENSION;
	UCHAR length = 0;
	UCHAR eid_ext = EID_EXT_EHT_TID2LNK_MAP;

	peer_entry = entry_get(ad, wcid);
	if (!IS_VALID_ENTRY(peer_entry) || !wdev) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"error: null pointer\n");
		return -EINVAL;
	}

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		struct bmgr_mld_sta *mld_sta = NULL;

		find_mld_sta_by_wcid(ad, wcid, &mld_sta);
		if (!mld_sta || !mld_sta->valid) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"INVALID MLD_STA\n");
			ret = -EINVAL;
			goto err;
		}
		nt2lm_contract = &mld_sta->nt2lm_contract;
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct mld_dev *mld = wdev->mld_dev;

		if (mld != &mld_device) {
			ret =  -1;
			goto err;
		}
		if (!mld->peer_mld.valid) {
			ret = -1;
			goto err;
		}
		nt2lm_contract = &mld->peer_mld.nt2lm_contract;
	} else {
		ret = -EINVAL;
		goto err;
	}

	if (nt2lm_request_sanity_check(ad,
		wdev,
		wcid,
		nt2lm_contract)) {
		ret = -EINVAL;
		goto err;
	}

	if (MlmeAllocateMemory(ad, &out_buffer) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		"alloc fail\n");
		return -EINVAL;
	}
	ActHeaderInit(ad, &(n2tlm_req.Hdr), peer_entry->Addr, wdev->if_addr, wdev->bssid);
	n2tlm_req.category = CATEGORY_PROTECTED_EHT;
	n2tlm_req.prot_eht_action = EHT_PROT_ACT_T2LM_REQUEST;

	t2lm_ctrl.link_map_ind = nt2lm_contract->tid_bit_map;
	t2lm_ctrl.dir = nt2lm_contract->dir;
	length = 3 + sizeof(struct t2lm_ctrl_t) + sizeof(u16) * TID_MAX;
	MakeOutgoingFrame(out_buffer, &frame_len,
		sizeof(struct eht_prot_action_frame), &n2tlm_req,
		1, &dialog_token,
		1, &eid,
		1, &length,
		1, &eid_ext,
		sizeof(struct t2lm_ctrl_t), &t2lm_ctrl,
		(sizeof(u16) * TID_MAX), nt2lm_contract->link_map,
		END_OF_ARGS);
	hex_dump_with_cat_and_lvl("NT2LM_REQ:",
		out_buffer, frame_len,
		DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO);
	MiniportMMRequest(ad, QID_AC_BE, out_buffer, frame_len, NULL);
	MlmeFreeMemory(out_buffer);
	return ret;

err:
	if (out_buffer)
		MlmeFreeMemory(out_buffer);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @mld initiate nt2lm contract teardown to peer mld
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param wcid which link to send out teardown frame
 */
int nt2lm_t2lm_teardown(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid
)
{
	int ret = 0;
	struct eht_prot_action_frame n2tlm_teardown = {0};
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mld_sta *mld_sta = NULL;
	struct _MAC_TABLE_ENTRY *peer_entry = NULL;
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	struct txs_callback_info_t *callback = NULL;
	struct nt2lm_teardown_arg_t *callback_arg = NULL;
	u8 *out_buffer = NULL;
	ULONG frame_len;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
		 "wcid=%d\n", wcid);

	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}
	ad = wdev->sys_handle;
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		if (find_mld_sta_by_wcid(ad, wcid, &mld_sta)) {
			ret = -EINVAL;
			goto err;
		}

		if (!mld_sta || !mld_sta->valid) {
			ret = -EINVAL;
			goto err;
		}

		nt2lm_contract = &mld_sta->nt2lm_contract;
		if (!nt2lm_contract->in_nego_tid) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"NOT in_nego_tid\n");
			ret = -EINVAL;
			goto err;
		}

		entry = get_bss_entry_by_netdev(wdev->if_dev);
		if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
			ret = -EINVAL;
			goto err;
		}

		mld = entry->mld_ptr;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			ret = -EINVAL;
			goto err;
		}
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct mld_dev *mld = wdev->mld_dev;

		if (mld != &mld_device) {
			ret = -EINVAL;
			goto err;
		}
		if (!mld->peer_mld.valid) {
			ret = -EINVAL;
			goto err;
		}
		nt2lm_contract = &mld->peer_mld.nt2lm_contract;
		if (!nt2lm_contract->in_nego_tid) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
				"NOT in_nego_tid\n");
			ret = -EINVAL;
			goto err;
		}
	} else {
		ret = -EINVAL;
		goto err;
	}

	/* TODO, NT2LM: find a available link to tx teardown frame */
	peer_entry = &ad->MacTab->Content[wcid];
	if (!(IS_VALID_ENTRY(peer_entry) && (IS_ENTRY_CLIENT(peer_entry) ||
		IS_ENTRY_PEER_AP(peer_entry)))) {
		ret = -EINVAL;
		goto err;
	}

	/* send response */
	if (MlmeAllocateMemory(ad, &out_buffer) != NDIS_STATUS_SUCCESS) {
		ret = -ENOMEM;
		goto err;
	}

	ActHeaderInit(ad, &(n2tlm_teardown.Hdr), peer_entry->Addr, wdev->if_addr, wdev->bssid);
	n2tlm_teardown.category = CATEGORY_PROTECTED_EHT;
	n2tlm_teardown.prot_eht_action = EHT_PROT_ACT_T2LM_TEARDOWN;

	/* only category + action */
	MakeOutgoingFrame(out_buffer, &frame_len,
		sizeof(struct eht_prot_action_frame), &n2tlm_teardown,
		END_OF_ARGS);
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		if (TxsInitCallbackInfo(ad,
		&callback,
		nt2lm_peer_tid_map_update_callback_after_teardown,
		(VOID **)&callback_arg,
		sizeof(struct nt2lm_teardown_arg_t))
		== NDIS_STATUS_SUCCESS) {
			callback_arg->wdev = wdev;
			callback_arg->mld = mld;
			callback_arg->mld_sta = mld_sta;
		} else {
			ret = -EINVAL;
			MlmeFreeMemory(out_buffer);
			goto err;
		}
	} else {
		if (TxsInitCallbackInfo(ad,
		&callback,
		nt2lm_sta_mld_tid_map_update_callback_after_teardown,
		(VOID **)&callback_arg,
		sizeof(struct nt2lm_teardown_arg_t))
		== NDIS_STATUS_SUCCESS) {
			callback_arg->wdev = wdev;
		} else {
			ret = -EINVAL;
			MlmeFreeMemory(out_buffer);
			goto err;
		}
	}

	if (MiniportMMRequest(ad, QID_AC_BE, out_buffer, frame_len, callback)
		!= NDIS_STATUS_SUCCESS) {
		ret = -EBUSY;
		TxsFreeCallbackInfo(callback);
		MlmeFreeMemory(out_buffer);
		goto err;
	}

	hex_dump_with_cat_and_lvl("NT2LM_TEARDOWN:",
		out_buffer, frame_len,
		DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO);

	MlmeFreeMemory(out_buffer);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @used to handle peer mld nt2lm action frame
 *
 * @param ad PRTMP_ADAPTER
 * @param *Elem MLME_QUEUE_ELEM
 */
void nt2lm_peer_t2lm_req_action(
	IN PRTMP_ADAPTER ad,
	IN MLME_QUEUE_ELEM * Elem
)
{
	struct eht_prot_action_frame *eht_prot_act = (struct eht_prot_action_frame *)Elem->Msg;
	u8 action = eht_prot_act->prot_eht_action;
	struct wifi_dev *wdev = Elem->wdev;
	u16 wcid = Elem->Wcid;

	switch (action) {
	case EHT_PROT_ACT_T2LM_REQUEST:
		nt2lm_peer_t2lm_request_action(ad, wdev, wcid, eht_prot_act);
		break;
	case EHT_PROT_ACT_T2LM_RESPONSE:
		nt2lm_peer_t2lm_response_action(ad, wdev, wcid, eht_prot_act);
		break;
	case EHT_PROT_ACT_T2LM_TEARDOWN:
		nt2lm_peer_t2lm_teardown_action(ad, wdev, wcid, eht_prot_act);
		break;
	default:
		break;
	}
}

/**
 * @nt2lm_tid_map_to_link_map: translate link_map to tid_map
 *
 * @param *nt2lm_contract per mld nt2lm param
 */
int nt2lm_trans_link_map_to_tid_map(
	IN struct nt2lm_contract_t *nt2lm_contract
)
{
	u8 tid, link_id;
	int ret = 0;
	u16 tid_map;
	u8 *link_to_tid_map;

	if (!nt2lm_contract)
		return -1;

	link_to_tid_map = nt2lm_contract->tid_map;
	NdisZeroMemory(link_to_tid_map, MLD_LINK_MAX);
	for (tid = 0; tid < TID_MAX; tid++) {
		tid_map = nt2lm_contract->link_map[tid];
		for (link_id = 0; link_id < MLD_LINK_MAX; link_id++)
			link_to_tid_map[link_id] |= ((tid_map & (1 << link_id)) >> link_id) << tid;
	}

	return ret;
}

#ifdef RT_CFG80211_SUPPORT
/**
 * @called by net link cmd to trigger nt2lm request
 *
 * @param ad PRTMP_ADAPTER
 * @param *wdev wifi_dev
 * @param *cmd used to build request frame
 */
int mtk_cfg80211_nt2lm_request(IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN struct nl_80211_t2lm_cmd *cmd)
{
	MAC_TABLE_ENTRY *entry = NULL;
	struct nt2lm_contract_t *nt2lm_contract = NULL;
	int ret = 0;

	if (!cmd) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"null cmd pointer\n");
		return -1;
	}
	entry = MacTableLookup(ad, cmd->ra_addr);
	if (!IS_VALID_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"err: entry is not valid\n");
		return -1;
	}
	if (!entry->mlo.mlo_en) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"err: entry is not mld\n");
		return -1;
	}
	if (wdev->wdev_type == WDEV_TYPE_AP) {
		struct bmgr_mld_sta *mld_sta = NULL;

		find_mld_sta_by_wcid(ad, entry->wcid, &mld_sta);
		if (!mld_sta || !mld_sta->valid) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"INVALID MLD_STA\n");
			return -1;
		}
		nt2lm_contract = &mld_sta->nt2lm_contract;
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		struct mld_dev *mld = wdev->mld_dev;

		if (mld != &mld_device) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"INVALID mld_device\n");
			return -1;
		}
		if (!mld->peer_mld.valid) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"INVALID mld_device\n");
			return -1;
		}
		nt2lm_contract = &mld->peer_mld.nt2lm_contract;
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"INVALID wdev_type\n");
		return -1;
	}
	nt2lm_contract->dir = cmd->dir;
	nt2lm_contract->tid_bit_map = cmd->tid_bit_map;
	nt2lm_contract->request_type = REQ_FROM_WAPP;
	NdisMoveMemory(nt2lm_contract->link_map,
		cmd->tid_to_link_bitmap, TID_MAX * sizeof(u16));

	nt2lm_trans_link_map_to_tid_map(nt2lm_contract);
	nt2lm_t2lm_request(ad, wdev, entry->wcid);
	return ret;
}
#endif
#endif /* DOT11_EHT_BE */
