/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#include "rt_config.h"
#include "bss_mngr.h"
#include "action.h"

extern struct bss_mngr_ops bmgr_ops;

NDIS_STATUS bss_mngr_event_bcn_update(
	IN PNET_DEV pNetDev
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"wdev(%d)\n", wdev->wdev_idx);

	/* re-sync beacon */
	UpdateBeaconHandler(ad, wdev, BCN_REASON(BCN_UPDATE_BMGR));

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_event_bss_attr_update(
	IN PNET_DEV pNetDev,
	IN struct bmgr_update_info *update_info,
	IN bool update_bcn
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"===> %s\n", RtmpOsGetNetDevName(pNetDev));

	wlan_config_set_t2lm_nego_support(wdev, update_info->mld_attr.mld_caps.t2l_nego_supp);

	/* EML Capabilities */
	if (update_info->mld_attr.eml_caps.emlsr_supp)
		wlan_config_set_emlsr_mr(wdev, EMLSR);
	else if (update_info->mld_attr.eml_caps.emlmr_supp)
		wlan_config_set_emlsr_mr(wdev, EMLMR);
	else
		wlan_config_set_emlsr_mr(wdev, EML_DISABLE);

	wlan_config_set_trans_to(wdev, update_info->mld_attr.eml_caps.trans_to);
	wlan_config_set_eml_omn_en(wdev, update_info->mld_attr.eml_caps.eml_omn_en);

	if (update_bcn) {
		/* Update beacons for new IEs */
		UpdateBeaconHandler(ad, wdev, BCN_REASON(BCN_UPDATE_BMGR));
	}

	return NDIS_STATUS_SUCCESS;
}

u8 *bss_mngr_event_ie_fragment(
	u8 *ie_buf, u16 *buf_len)
{
	u8 *pos = NULL;

	pos = fragment_information_element(ie_buf, buf_len);
	return pos;
}

#ifdef DOT11_EHT_BE
NDIS_STATUS bss_mngr_event_add_mld(
	IN PNET_DEV pNetDev,
	IN u8 mld_grp,
	IN u8 *mld_addr,
	IN u8 mld_type
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	mld_type = ap_mld_type_trans(mld_type);

	if (HcAcquireMldRsc(ad, mld_type, mld_grp, mld_addr) != HC_STATUS_OK) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: acquire hw rsc failed (type:%d, grp:%d, addr:%pM)\n",
			mld_type, mld_grp, mld_addr);

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_event_del_mld(
	IN PNET_DEV pNetDev,
	IN u8 mld_grp
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (HcReleaseMldRsc(ad, mld_grp) != HC_STATUS_OK) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: release hw rsc failed (grp:%d)\n", mld_grp);

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_event_mld_add_link(
	IN PNET_DEV pNetDev,
	IN u8 mld_grp
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (HcAcquireMldLinkRsc(ad, wdev, mld_grp) != HC_STATUS_OK) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: acquire hw rsc failed (if:%s, grp:%d)\n",
			RtmpOsGetNetDevName(pNetDev), mld_grp);

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_event_mld_del_link(
	IN PNET_DEV pNetDev
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (HcReleaseMldLinkRsc(ad, wdev) != HC_STATUS_OK) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: release hw rsc failed (if:%s)\n",
			RtmpOsGetNetDevName(pNetDev));

		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_peer_integrity_check(struct _MAC_TABLE_ENTRY *entry)
{
	struct _MAC_TABLE_ENTRY *link_entry = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct mld_entry_t mld_entry_info;
	struct eht_link_t *link_info = NULL;
	u8 i = 0;
	u8 bss_idx = 0;
	int ret = 0;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"caller=%pS\n", OS_TRACE);

	if (!entry) {
		ret = -EINVAL;
		goto err;
	}

	mt_rcu_read_lock();
	if (entry->mlo.mlo_en)
		mld_entry = rcu_dereference(entry->mld_entry);
	if (!mld_entry) {
		mt_rcu_read_unlock();
		ret = -EEXIST;
		goto err;
	}

	os_move_mem(&mld_entry_info, mld_entry, sizeof(struct mld_entry_t));
	mt_rcu_read_unlock();

	for (i = 0; i < MLD_LINK_MAX; i++) {
		link_entry = (struct _MAC_TABLE_ENTRY *)mld_entry_info.link_entry[i];
		if (!link_entry)
			continue;
		bss_idx = link_entry->wdev->bss_info_argument.ucBssIndex;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"StaRec(link_id=%d,pEntry=%p,wcid=%d,bss_idx=%d)\n",
			i, link_entry, link_entry->wcid, bss_idx);

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			" mlo: mlo_en=%d,is_setup_link_entry=%d\n",
			link_entry->mlo.mlo_en,
			link_entry->mlo.is_setup_link_entry);

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			" mld_info: vld=%d,nsep=%d,emlmr=0x%x,emlsr=0x%x,eml_cap=0x%x,mld_sta_idx=%d\n",
			mld_entry_info.valid,
			mld_entry_info.nsep,
			mld_entry_info.emlmr,
			mld_entry_info.emlsr,
			mld_entry_info.eml_cap,
			mld_entry_info.mld_sta_idx);

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			" mld_info: addr=%pM,setup_link_wcid=%d,link_num=%d,str[0x%x,0x%x,0x%x]\n",
			mld_entry_info.addr,
			mld_entry_info.setup_link_wcid,
			mld_entry_info.link_num,
			mld_entry_info.str_map[0], mld_entry_info.str_map[1], mld_entry_info.str_map[2]);

		link_info = &link_entry->mlo.link_info;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			" link_info: link_id=%d,tid_dl=0x%x,tid_ul=0x%x\n",
			link_info->link_id,
			link_info->tid_map_dl,
			link_info->tid_map_ul);

		if (i != link_info->link_id)
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
				" link_id(%d,%d) error\n", i, link_info->link_id);
	}

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

int bss_mngr_check_last_peer(
	IN struct mld_sta_add *add_sta,
	IN struct _MAC_TABLE_ENTRY *link_entry
)
{
	struct mld_link_add *add_link = NULL;
	u8 total_link_cnt = add_sta->total_link_cnt;
	u8 total_active_cnt = 0;
	u8 i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		if (i >= BSS_MNGR_MAX_BAND_NUM)
			continue;
		add_link = &add_sta->add_link[i];
		if (add_link->active && add_link->priv_ptr)
			total_active_cnt++;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
		"total_link_cnt=%d, total_active_cnt=%d\n",
		total_link_cnt, total_active_cnt);

	if (total_link_cnt == total_active_cnt) {
		if (link_entry)
			bss_mngr_peer_integrity_check(link_entry);
	}

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_fill_peer_mlo_info(
	IN struct mld_sta_add *add_sta,
	OUT struct eht_mlo_t *mlo
)
{
	struct mld_link_add *add_link = NULL;
	struct eht_link_t *link_info = NULL;
	u8 link_id = 0;
	int ret = 0;

	if (!add_sta || !mlo) {
		ret = -EINVAL;
		goto err;
	}

	if (add_sta->mld_sta_idx == MLD_STA_NONE) {
		ret = -EPERM;
		goto err;
	}

	link_id = add_sta->link_id;
	if (link_id >= MLD_LINK_MAX) {
		ret = -ELNRNG;
		goto err;
	}

	add_link = &add_sta->add_link[link_id];
	if (!add_link->active) {
		ret = -ENOENT;
		goto err;
	}

	mlo->mlo_en = 1;
	mlo->is_setup_link_entry = add_link->is_setup_link;

	/* link_info: per link info and this link info */
	link_info = &mlo->link_info;
	link_info->link_id = add_sta->link_id;
	link_info->tid_map_dl = 0xff; /*default mapping*/
	link_info->tid_map_ul = 0xff; /*default mapping*/
	link_info->link_sts = tid_2_link_sts(link_info->tid_map_dl, link_info->tid_map_ul);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
		"ret=%d\n", ret);
	return ret;
}

NDIS_STATUS bss_mngr_event_add_sta(
	IN PNET_DEV pNetDev,
	IN struct mld_sta_add *add_sta
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct mld_link_add *add_link;
	MAC_TABLE_ENTRY *pEntry = NULL;

	add_link = &add_sta->add_link[add_sta->link_id];

	/* add station */
	if (add_link->active) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"wdev(%d), mld_sta_idx(%d), aid(%d), MldAddr %pM\n",
				wdev->wdev_idx, add_sta->mld_sta_idx, add_sta->aid, add_sta->mld_addr);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tL%d: %pM, isSetup(%d), TID(0x%x), STR(0x%x), LinkCnt=%d\n",
				 add_sta->link_id, add_link->sta_addr, add_link->is_setup_link,
				 add_link->tid_map, add_link->str_map, add_sta->total_link_cnt);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tEMLCap(0x%x)\n",
				 add_sta->eml_caps);

		/* if FULL, return */
		if (ad->MacTab->Size >= GET_MAX_UCAST_NUM(ad))
			return NDIS_STATUS_FAILURE;

		pEntry = MacTableLookup(ad, add_link->sta_addr);
		if (pEntry == NULL) {
			/* allocate one MAC entry */
			pEntry = MacTableInsertEntry(ad,
				add_link->sta_addr,
				wdev, ENTRY_CLIENT,
				OPMODE_AP,
				TRUE,
				add_sta->mld_sta_idx,
				add_sta->mld_addr);
			if (pEntry) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_NOTICE,
					"pEntry(%p): wcid(%d) mac=%pM, mld(sta_idx=%d,addr=%pM) acquired!!\n",
					pEntry, pEntry->wcid, pEntry->Addr, add_sta->mld_sta_idx, add_sta->mld_addr);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"pEntry: acquire failed!!\n");
				return NDIS_STATUS_FAILURE;
			}
		} else {
			/* cover non-setup link mlo-capability changed */
			if (!IS_ENTRY_MLO(pEntry)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Fail:pEntry=%p wcid=%d midx=%d, occupied by non-mlo peer\n",
					pEntry, pEntry->wcid, add_sta->mld_sta_idx);
				return NDIS_STATUS_FAILURE;
			}

			/* same link addr, different mld addr */
			if (!MAC_ADDR_EQUAL(pEntry->mlo.mld_addr, add_sta->mld_addr)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Fail:pEntry=%p wcid=%d midx=%d, maddr=%pM,%pM\n",
					pEntry, pEntry->wcid, add_sta->mld_sta_idx,
					pEntry->mlo.mld_addr, add_sta->mld_addr);
				return NDIS_STATUS_FAILURE;
			}

			/* same link addr, different link_id */
			if (pEntry->mlo.link_info.link_id != add_sta->link_id) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Fail:pEntry=%p wcid=%d midx=%d, lkid=%d,%d\n",
					pEntry, pEntry->wcid, add_sta->mld_sta_idx,
					pEntry->mlo.link_info.link_id, add_sta->link_id);
				return NDIS_STATUS_FAILURE;
			}

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_NOTICE,
				"pEntry(%p): wcid(%d) mac=%pM, mld(sta_idx=%d,addr=%pM) allocated!!\n",
				pEntry, pEntry->wcid, pEntry->Addr, add_sta->mld_sta_idx, add_sta->mld_addr);
		}

		if (pEntry) {
			struct mld_entry_t *mld_entry = NULL, *new_mld_entry = NULL;
			struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
			struct hlist_head *head = NULL;
			u8 hash_idx = MAC_ADDR_HASH_INDEX(add_sta->mld_addr);

			add_link->priv_ptr = (void *)pEntry;

			bss_mngr_fill_peer_mlo_info(add_sta, &pEntry->mlo);

			spin_lock_bh(&mld_entry_ctrl->mld_entry_lock);
			mld_entry = get_mld_entry_by_mac(add_sta->mld_addr);
			if (mld_entry) {
				new_mld_entry = create_mld_entry();
				if (new_mld_entry) {
					update_mld_entry(mld_entry, new_mld_entry, pEntry, add_sta);
					hlist_replace_rcu(&mld_entry->hlist, &new_mld_entry->hlist);
					update_link_mld_entry(new_mld_entry);
					mt_call_rcu(&mld_entry->rcu, free_mld_entry);
				} else {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
						" create mld_entry Fail!\n");
					spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);
					return NDIS_STATUS_FAILURE;
				}
			} else {
				mld_entry = create_mld_entry();
				if (mld_entry) {
					update_mld_entry(NULL, mld_entry, pEntry, add_sta);
					head = &mld_entry_ctrl->mld_hash[hash_idx];
					hlist_add_head_rcu(&mld_entry->hlist, head);
					update_link_mld_entry(mld_entry);
				} else {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
						" create mld_entry Fail!\n");
					spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);
					return NDIS_STATUS_FAILURE;
				}
			}
			spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);

			bss_mngr_check_last_peer(add_sta, pEntry);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_event_conn_act(
	IN PNET_DEV pNetDev,
	IN struct mld_link_add *add_link
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	MAC_TABLE_ENTRY *pEntry = NULL;

	/* do conn action */
	if (add_link->active) {
		pEntry = add_link->priv_ptr;
		if (pEntry) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"update link%d (%pM) starec\n",
				pEntry->mlo.link_info.link_id, pEntry->Addr);
			wdev->wdev_ops->conn_act(wdev, pEntry);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_event_del_sta(
	IN struct mld_link_del *del_link
)
{
	struct _RTMP_ADAPTER *ad;
	MAC_TABLE_ENTRY *pEntry = del_link->priv_ptr;

	if (pEntry) {
		ad = pEntry->pAd;
		if (ad) {
			_MacTableDeleteEntry(ad, pEntry->wcid, pEntry->Addr);
			/* init mlo_en */
			pEntry->mlo.mlo_en = 0;
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"ad is NULL. (%pM)\n", del_link->sta_addr);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"entry (%pM) is NULL\n", del_link->sta_addr);
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_event_disconn_mld_sta(
	IN struct mld_link_del *del_link
)
{
	struct _RTMP_ADAPTER *ad;
	MAC_TABLE_ENTRY *pEntry = del_link->priv_ptr;

	if (pEntry) {
		ad = pEntry->pAd;
		if (ad) {
			MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"Disassoc STA("MACSTR")\n", MAC2STR(pEntry->Addr));

			APMlmeKickOutSta(ad, pEntry->Addr, pEntry->wcid, REASON_DISASSOC_INACTIVE);

			return NDIS_STATUS_SUCCESS;
		}
	}

	return NDIS_STATUS_FAILURE;
}

NDIS_STATUS bss_mngr_event_disconn_act(
	IN struct mld_link_del *del_link
)
{
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY *pEntry = del_link->priv_ptr;

	if (pEntry) {
		wdev = pEntry->wdev;
		if (wdev)
			wdev->wdev_ops->disconn_act(wdev, pEntry);
	}

	return NDIS_STATUS_SUCCESS;
}

void bss_mngr_event_query_probe_rsp_frame(
	IN PNET_DEV pNetDev,
	OUT struct ie *rsp_frm
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct buffer_wrapper rsp_buf = {0};

	eht_mlo_query_link_probe_rsp_frame(wdev, &rsp_buf);
	rsp_frm->buf = rsp_buf.buf;
	rsp_frm->len = rsp_buf.buf_len;

}

void bss_mngr_event_form_ml_probe_rsp_sta_pf(
	IN PNET_DEV pNetDev,
	IN struct ie *repting_rsp_frm,
	IN u8 repting_link_id
)
{
	NDIS_STATUS status;

	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct buffer_wrapper repting_rsp_buf = {0}, repted_rsp_buf = {0};

	/* get the probe response frame of reported link */
	eht_mlo_query_link_probe_rsp_frame(wdev, &repted_rsp_buf);

	/* There are two probe response frames now, perform post-process & inheritance rule */
	if (repted_rsp_buf.buf_len == 0) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: fail to query probe response frame!\n");
		goto err;
	} else {
		WRAP_BUFFER(repting_rsp_buf, repting_rsp_frm->buf, repting_rsp_frm->len);
		status = eht_ap_mlo_postprocess_reported_link_probe_rsp(
					&repted_rsp_buf,
					&repting_rsp_buf);
		if (status != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
				"Error: fail to form STA Profile\n");
			goto err;
		}
	}

	/* deliver STA Profile to BSS Manager (reported link) */
	if (repted_rsp_buf.buf_len < BSS_MNGR_MAX_STA_PROFILE_LEN)
		bss_mngr_mld_add_sta_profile(
			wdev, BMGR_MAX_MLD_STA_CNT,
			repting_link_id, BMGR_MLD_PROBE_RSP_COMPLETE_PROFILE,
			repted_rsp_buf.buf, repted_rsp_buf.buf_len);
	else
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
			"Error!!!: wdev[%d] After postprocessing, size of STA Profile > %d, drop it\n",
			BSS_MNGR_MAX_STA_PROFILE_LEN, wdev->wdev_idx);

err:
	if (repted_rsp_buf.buf)
		MlmeFreeMemory(repted_rsp_buf.buf);

}

void bss_mngr_event_ba_ori_setup(
	IN PNET_DEV pNetDev,
	IN struct mld_ba_info *ba_info
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MAC_TABLE_ENTRY *pEntry = ba_info->priv_ptr;

	if (pEntry)
		ba_ori_session_start(pAd, pEntry->wcid, ba_info->TID);

}

u8 bss_mngr_event_ba_resrc_ori_add(
	IN PNET_DEV pNetDev,
	IN struct mld_ba_info *ba_info
)
{
	int ret = FALSE;
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MAC_TABLE_ENTRY *pEntry = ba_info->priv_ptr;

	if (pEntry)
		ret = ba_resrc_ori_add(pAd, pEntry->wcid, ba_info->TID, ba_info->BAWinSize, ba_info->amsdu_en, ba_info->TimeOut);

	return ret;
}

u8 bss_mngr_event_ba_resrc_rec_add(
	IN PNET_DEV pNetDev,
	IN struct mld_ba_info *ba_info
)
{
	int ret = FALSE;
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MAC_TABLE_ENTRY *pEntry = ba_info->priv_ptr;

	if (pEntry)
		ret = ba_resrc_rec_add(pAd, pEntry->wcid, ba_info->TID, ba_info->TimeOut, ba_info->Seq, ba_info->BAWinSize);

	return ret;
}

void bss_mngr_event_ba_resrc_ori_del(
	IN PNET_DEV pNetDev,
	IN struct mld_ba_info *ba_info
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MAC_TABLE_ENTRY *pEntry = ba_info->priv_ptr;

	if (pEntry) {
		ba_resrc_ori_del(pAd, pEntry->wcid, ba_info->TID);

		if (!ba_info->bPassive)
			ba_send_delba(pAd, pEntry->wcid, ba_info->TID);
	}

}

void bss_mngr_event_ba_resrc_rec_del(
	IN PNET_DEV pNetDev,
	IN struct mld_ba_info *ba_info
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MAC_TABLE_ENTRY *pEntry = ba_info->priv_ptr;

	if (pEntry)
		ba_resrc_rec_del(pAd, pEntry->wcid, ba_info->TID);
}

void bss_mngr_event_ba_add_to_asic(
	IN PNET_DEV pNetDev,
	IN struct mld_ba_info *ba_info
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MAC_TABLE_ENTRY *pEntry = ba_info->priv_ptr;

	if (pEntry)
		RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, ba_info->TID,
			ba_info->Seq, ba_info->BAWinSize, ba_info->type, ba_info->amsdu_en);
}

void bss_mngr_event_ba_del_from_asic(
	IN PNET_DEV pNetDev,
	IN struct mld_ba_info *ba_info
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	MAC_TABLE_ENTRY *pEntry = ba_info->priv_ptr;

	if (pEntry)
		RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, pEntry->wcid, ba_info->TID, ba_info->type, ba_info->amsdu_en);
}

void bss_mngr_event_twt_action_frm_forward(
	IN struct _MLME_QUEUE_ELEM *elem,
	IN struct _MAC_TABLE_ENTRY *peer_entry
)
{
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *ad = NULL;
	struct MLO_TWT_PRIV_T *mlo_twt_priv = NULL;
	u8 *ie_ptr = NULL;
	struct mlo_itwt_ie *mlo_twt_ie = NULL;
	u8 linkid_bitmap_present = 0;

	if (!elem || !peer_entry)
		return;

	wdev = peer_entry->wdev;
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"action link=%s\n", wdev->if_dev->name);

	/* breakdown twt action frame if link_id_bitmap_present=1 or
	   consider it as legacy itwt request format */
	os_alloc_mem(ad, (UCHAR **)&mlo_twt_priv, sizeof(struct MLO_TWT_PRIV_T));

	if (!mlo_twt_priv) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"mlo_twt_priv alloc mem fail!!!\n");
		return;
	}

	os_zero_mem(mlo_twt_priv, sizeof(struct MLO_TWT_PRIV_T));

	/* get all_linkid_bitmap */
	ie_ptr = (u8 *)(elem->Msg + LENGTH_802_11 + 3);
	mlo_twt_ie = (struct mlo_itwt_ie *)ie_ptr;
	linkid_bitmap_present = (u8)GET_TWT_CTRL_LINKID_BITMAP_PRESENT(mlo_twt_ie->control);

	/* assign next state */
	mlo_twt_priv->state = linkid_bitmap_present ?
		STATE_BREAKDOWN_ACTION_FRAME : STATE_NO_LINKID_BITMAP_PRESENT;

	/* forward action frame to action link MLME */
	REPORT_MGMT_FRAME_TO_MLME(ad, peer_entry->wcid, &elem->Msg, elem->MsgLen, 0, 0, 0, 0,
		0, 0, OPMODE_AP, wdev, 0, mlo_twt_priv, 0);
}

u8 bss_mngr_event_twt_action_frm_breadown(
	IN PNET_DEV da_net_dev,
	IN PNET_DEV sa_net_dev,
	IN u8 da_linkid,
	IN u8 sa_linkid,
	IN u16 mld_sta_idx,
	IN u8 action_linkid,
	IN u16 linkid_bitmap,
	IN u16 all_linkid_bitmap,
	IN struct _MLME_QUEUE_ELEM *elem,
	IN struct mlo_itwt_ie *twt_ie,
	IN struct _MAC_TABLE_ENTRY *peer_entry
)
{
	NDIS_STATUS status = 0;
	u8 breakdown = 0;
	struct MLO_TWT_PRIV_T *mlo_twt_priv = NULL;
	struct frame_itwt_setup *in_action_frame = (struct frame_itwt_setup *)&elem->Msg;
	struct frame_itwt_setup out_action_frame = {0};

	if (!da_net_dev || !sa_net_dev || !elem || !twt_ie || !peer_entry)
		return breakdown;

	/* adujst A1=RA=DA, A2=TA=SA, A3=BSSID with link related information */
	ActHeaderInit(NULL, &out_action_frame.hdr, peer_entry->bssid, peer_entry->Addr, peer_entry->bssid);

	/* S1G action: twt setup frame action hdr */
	out_action_frame.category = in_action_frame->category;
	out_action_frame.s1g_action = in_action_frame->s1g_action;
	out_action_frame.token = in_action_frame->token;

	/* remove mlo itwt linkid_bitmap 2B and let legacy twt MLME can handle it directly */
	os_move_mem(&out_action_frame.twt_ie, twt_ie, sizeof(struct itwt_ie_t));
	out_action_frame.twt_ie.len = sizeof(struct itwt_ie_t) - 2;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"A1=%pM,A2=%pM,A3=%pM,len=%d,wcid=%d, and be forwarded to\n",
		out_action_frame.hdr.Addr1,
		out_action_frame.hdr.Addr2,
		out_action_frame.hdr.Addr3,
		out_action_frame.twt_ie.len,
		peer_entry->wcid);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"da/sa=%s,%s, da/sa=%d,%d, mld_sta_idx=%d, action_linkid=%d, linkid_bitmap=0x%x, all_linkid_bitmap=0x%x\n",
		da_net_dev->name, sa_net_dev->name,
		da_linkid, sa_linkid,
		mld_sta_idx,
		action_linkid,
		linkid_bitmap,
		all_linkid_bitmap);
	/* mlo itwt ctrl parameters */
	status = os_alloc_mem(NULL, (UCHAR **)&mlo_twt_priv, sizeof(struct MLO_TWT_PRIV_T));
	if (status == NDIS_STATUS_SUCCESS) {
		u16 wcid = peer_entry->wcid;
		struct wifi_dev *wdev = peer_entry->wdev;
		struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

		/* assign next state */
		os_zero_mem(mlo_twt_priv, sizeof(struct MLO_TWT_PRIV_T));
		mlo_twt_priv->da_net_dev = da_net_dev;
		mlo_twt_priv->sa_net_dev = sa_net_dev;
		mlo_twt_priv->da_linkid = da_linkid;
		mlo_twt_priv->sa_linkid = sa_linkid;
		mlo_twt_priv->mld_sta_idx = mld_sta_idx;
		mlo_twt_priv->action_linkid = action_linkid;
		mlo_twt_priv->linkid_bitmap = linkid_bitmap;
		mlo_twt_priv->all_linkid_bitmap = all_linkid_bitmap;
		mlo_twt_priv->state = STATE_ACTION_FRAME_HANDLE;

		/* forward N of "1->N" action frame to MLME */
		/*dump_hex_content((UINT8 *)&out_action_frame, sizeof(out_action_frame));*/
		REPORT_MGMT_FRAME_TO_MLME(ad, wcid, &out_action_frame, sizeof(out_action_frame), 0, 0, 0, 0,
			0, 0, OPMODE_AP, wdev, 0, mlo_twt_priv, 0);
		breakdown = 1;
	} else
		breakdown = 0;

	return breakdown;
}

void bss_mngr_event_twt_action_frm_agg(
	IN PNET_DEV pNetDev,
	IN u8 *out_buffer,
	IN u16 len,
	IN struct _MAC_TABLE_ENTRY *peer_entry,
	IN u16 mld_sta_idx,
	IN u8 action_linkid,
	IN u16 all_linkid_bitmap
)
{
	/* pNetDev = action link */
	if (peer_entry) {
		struct MLO_TWT_PRIV_T *mlo_twt_priv = NULL;
		struct wifi_dev *wdev = peer_entry->wdev;
		struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		u16 wcid = peer_entry->wcid;

		if (os_alloc_mem(NULL, (UCHAR **)&mlo_twt_priv, sizeof(struct MLO_TWT_PRIV_T))
			== NDIS_STATUS_SUCCESS) {
			os_zero_mem(mlo_twt_priv, sizeof(struct MLO_TWT_PRIV_T));
			/* da/sa net_dev, da/sa linkid, linkid_bitmap are not needed in tx phase */
			mlo_twt_priv->mld_sta_idx = mld_sta_idx;
			mlo_twt_priv->action_linkid = action_linkid;
			mlo_twt_priv->all_linkid_bitmap = all_linkid_bitmap;
			mlo_twt_priv->state = STATE_TX_AGGREGATE_ACTION_FRAME;
			/* forward 1 of "N->1" to action link */
			REPORT_MGMT_FRAME_TO_MLME(ad, wcid, out_buffer, len, 0, 0, 0, 0,
				0, 0, OPMODE_AP, wdev, 0, mlo_twt_priv, 0);
		} else
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"fail. os_alloc_mem\n");
	} else
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. peer_entry=NULL\n");
}


void bss_mngr_event_mld_twt_wait_ack_inform(
	IN PNET_DEV pNetDev,
	IN struct _MAC_TABLE_ENTRY *peer_entry,
	IN u8 wait_ack_sts
)
{
	if (pNetDev && peer_entry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"IF=%s,wdev_idx=%d,wcid=%d,wait_ack_sts=%d\n",
			pNetDev->name, peer_entry->wdev->wdev_idx,
			peer_entry->wcid, wait_ack_sts);
		twt_wait_ack_inform(peer_entry->wdev, peer_entry->wcid, wait_ack_sts);
	}
}

#ifdef MWDS
int bss_mbgr_event_mld_mwds_enable(
	IN PNET_DEV pNetDev,
	u8 enable
)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad;
	u8 if_idx = 0;

	if (!wdev)
		return FALSE;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (!ad)
		return FALSE;

	if_idx = wdev->func_idx;
	if (enable)
		return MWDSEnable(ad, if_idx, TRUE, FALSE);
	else
		return MWDSDisable(ad, if_idx, TRUE, FALSE);
}
#endif

void bss_mngr_event_mld_critical_update_bcn(
	IN PNET_DEV pNetDev,
	IN struct mld_crit_upd_info *info,
	IN u8 reason)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"[%s] bitmap: 0x%x, bypass_bmap: 0x%x, reason=0x%x\n",
		wdev->if_dev->name, info->bitmap, info->bypass_bcn_seq_bmap, reason);

	wdev->bcn_buf.cu_info.crit_upd_bitmap = info->bitmap;
	if (ad->CommonCfg.wifi_cert)
		wdev->bcn_buf.cu_info.bypass_bcn_seq_bmap = info->bypass_bcn_seq_bmap;
	wdev->bcn_buf.cu_info.crit_upd_flag = TRUE;

	/* Set Critical update Flag (high bit[6]) + Original reason (Low bits[5:0]) */
	UpdateBeaconHandler(ad, wdev, (BCN_UPDATE_REASON)BCN_SET_CRIT_UPD(reason));
}

u8 bss_mngr_event_mld_critical_update_op_lock(
	IN PNET_DEV pNetDev,
	IN u8 lock,
	IN u8 mld_id)
{
	u8 status = 1;
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_DEBUG,
		"[%s] lock: 0x%x\n", wdev->if_dev->name, lock);

	if (lock == BMGR_BPCC_OP_GET_LOCK)
		status = bcn_bpcc_op_lock_by_bssmngr(ad, wdev) ? 1 : 0;
	else if (lock == BMGR_BPCC_OP_RELEASE_LOCK)
		bcn_bpcc_op_unlock_by_bssmngr(ad, wdev);
	else if (lock == BMGR_BPCC_OP_RESET_LOCK_TIMER)
		bcn_bpcc_op_lock_set_timer_by_bssmngr(ad, wdev);
	else if (lock == BMGR_BPCC_OP_MLD_ID_LOG_ON)
		bcn_bpcc_mld_id_log_sync(ad, wdev, mld_id, TRUE);
	else if (lock == BMGR_BPCC_OP_MLD_ID_LOG_OFF)
		bcn_bpcc_mld_id_log_sync(ad, wdev, mld_id, FALSE);

	return status;
}

void bss_mngr_event_mld_eml_op_update(
	IN PNET_DEV pNetDev,
	IN struct mld_eml_op *eml_op)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);
	struct _MAC_TABLE_ENTRY *pEntry = eml_op->priv_ptr;
	struct mld_entry_t mld_entry = {0};
	struct _STA_REC_CTRL_T sta_rec_ctrl = {0};

	if (arch_ops->archUpdateEMLOp) {
		u8 *pOutBuffer = NULL;
		u32 eml_control = 0, eml_ctrl_len = 1;
		ULONG FrameLen;
		struct eht_prot_action_frame eml_op_act;

		if (!pEntry)
			return;

		/* update values */
		if (pEntry->mlo.mlo_en) {
			mld_entry.emlsr = eml_op->eml_link_bmap;
			mld_entry_fill_fields(pEntry->mlo.mld_addr, &mld_entry,
				MLD_ENTRY_FIELD_FLAGS_EMLSR);
		}

		pEntry->EmlsrBitmap = eml_op->eml_link_bmap;

		/* update StaRec */
		sta_rec_ctrl.WlanIdx = pEntry->wcid;
		arch_ops->archUpdateEMLOp(ad, &sta_rec_ctrl);

		/* send response */
		if (eml_op->eml_omn_en) {
			if (MlmeAllocateMemory(ad, &pOutBuffer) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Error: allocate memory failed\n");
				return;
			}

			ActHeaderInit(ad, &(eml_op_act.Hdr), pEntry->Addr, wdev->if_addr, wdev->bssid);
			eml_op_act.category = CATEGORY_PROTECTED_EHT;
			eml_op_act.prot_eht_action = EHT_PROT_ACT_EML_OP_NOTIF;

			if (pOutBuffer) {
				SET_DOT11BE_EML_CTRL_EMLSR_MODE(eml_control, eml_op->emlsr_en);
				SET_DOT11BE_EML_CTRL_EMLMR_MODE(eml_control, eml_op->emlmr_en);
				if (eml_op->emlsr_en || eml_op->emlmr_en) {
					SET_DOT11BE_EML_CTRL_EMLSR_LINK_BMAP(eml_control,
						cpu_to_le16(eml_op->eml_link_bmap));

					eml_ctrl_len += sizeof(eml_op->eml_link_bmap);
				}

				MakeOutgoingFrame(pOutBuffer, &FrameLen,
								  sizeof(struct eht_prot_action_frame), &eml_op_act,
								  sizeof(eml_op->dialog_token), &eml_op->dialog_token,
								  eml_ctrl_len, &eml_control,
								  END_OF_ARGS);

				MiniportMMRequest(ad, QID_AC_BE, pOutBuffer, FrameLen, NULL);

				MlmeFreeMemory(pOutBuffer);

				MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"Response EML OMN w/ token(%d) en(%d), bmap(0x%x) len(%d)\n",
					eml_op->dialog_token, eml_op->emlsr_en, eml_op->eml_link_bmap, eml_ctrl_len);
			}
		} else {
			MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"Do not Response EML OMN\n");
		}
	} else {
		MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
			"UpdateEMLOp not supported\n");
	}
}

void bss_mngr_event_tx_link_recomm(
	IN PNET_DEV pNetDev,
	IN u16 reason_code,
	IN u8 *buf,
	IN u16 buf_len)
{
	struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	u8 *pOutBuffer = NULL;
	ULONG FrameLen;
	struct eht_prot_action_frame link_recomm_act;
	PHEADER_802_11 pHdr80211;

	if (!buf || !buf_len) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"(Error) empty buffer %d\n", buf_len);
		return;
	}

	if (MlmeAllocateMemory(ad, &pOutBuffer) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"(Error) allocate memory failed\n");
		return;
	}

	if (pOutBuffer) {
		pHdr80211 = &(link_recomm_act.Hdr);
		NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
		pHdr80211->FC.Type = FC_TYPE_MGMT;
		pHdr80211->FC.SubType = SUBTYPE_ACTION_NO_ACK;
		COPY_MAC_ADDR(pHdr80211->Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(pHdr80211->Addr2, wdev->if_addr);
		COPY_MAC_ADDR(pHdr80211->Addr3, wdev->bssid);

		link_recomm_act.category = CATEGORY_PROTECTED_EHT;
		link_recomm_act.prot_eht_action = EHT_PROT_ACT_LINK_RECOMM;
		reason_code = cpu_to_le16(reason_code);

		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(struct eht_prot_action_frame), &link_recomm_act,
						  sizeof(reason_code), &reason_code,
						  buf_len, buf,
						  END_OF_ARGS);

		hex_dump_always("LR Frame", pOutBuffer, FrameLen);

		MiniportMMRequest(ad, QID_AC_BE, pOutBuffer, FrameLen, NULL);

		MlmeFreeMemory(pOutBuffer);

		MTWF_DBG(ad, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"(%s) TX Link Recomm w/ reason(%d) buf_len(%d)\n",
			RtmpOsGetNetDevName(pNetDev), cpu_to_le16(reason_code), buf_len);
	}
}

/**
 * @ML reconfiguration: update mld sta new setup link
 *
 * @param *mld_addr mld addr of non-ap mld
 * @param available_link_id_bmap available link id bitmap
 */
int bss_mngr_reconfig_sta_setup_link_set(
	IN u8 *mld_addr,
	IN u16 available_link_id_bmap
)
{
	int ret = 0;
	u8 link_id = 0;
	struct mld_entry_t *mld_entry = NULL;
	struct mld_entry_t temp_mld_entry = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
		 "mld_sta=%pM, available_link_id_bmap=0x%x\n", mld_addr, available_link_id_bmap);

	if (MAC_ADDR_EQUAL(mld_addr, ZERO_MAC_ADDR)) {
		ret = -EINVAL;
		goto err;
	}

	if (!available_link_id_bmap) {
		ret = -EINVAL;
		goto err;
	}

	/*assign new setup link by link_id*/
	link_id = ffs(available_link_id_bmap) - 1;

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(mld_addr);
	if (!mld_entry) {
		ret = -EINVAL;
		mt_rcu_read_unlock();
		goto err;
	}
	pEntry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[link_id];
	mt_rcu_read_unlock();

	if (!pEntry) {
		ret = -EINVAL;
		goto err;
	}
	pEntry->mlo.is_setup_link_entry = 1;

	wdev = pEntry->wdev;
	if (!wdev) {
		ret = -EINVAL;
		goto err;
	}

	temp_mld_entry.setup_link_wcid = pEntry->wcid;
	temp_mld_entry.setup_link_id = link_id;

	/*logan driver*/
	mld_entry_fill_fields(mld_addr, &temp_mld_entry,
		MLD_ENTRY_FIELD_FLAGS_SETUP_LINK_WCID |
		MLD_ENTRY_FIELD_FLAGS_SETUP_LINK_ID);

	/*hwifi driver*/
	HcChangeSetupLink(wdev->sys_handle, pEntry->wcid);
	/*IAPP*/
	IAPP_L2_Update_Frame_Send(wdev->sys_handle, mld_addr, pEntry->wdev->wdev_idx);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
		 "change new setup link to wcid=%d\n", pEntry->wcid);

	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}


/**
 * @ML reconfiguration: find available setup link
 *
 * @param *mld_addr non-AP mld addr
 * @param rm_link_id_bitmap link id bitmap of remove link
 * @param *available_link_id_bitmap availavle link id bitmap
 * @param is_rm_link_on_setup_link check if rm link=setup link
 * @param **lastt_setup_link_entry last setup link
 */
int bss_mngr_reconfig_available_link_find(
	IN u8 *mld_addr,
	IN u16 rm_link_id_bitmap,
	OUT u16 *available_link_id_bitmap,
	OUT u8 *is_rm_link_on_setup_link,
	OUT struct _MAC_TABLE_ENTRY **last_setup_link_entry
)
{
	int ret = 0;
	u8 link_id = 0;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct eht_mlo_t *mlo_info = NULL;
	struct eht_link_t *link_info = NULL;
	struct mld_entry_t *mld_entry = NULL;

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(mld_addr);
	if (!mld_entry) {
		mt_rcu_read_unlock();
		ret = -EEXIST;
		goto err;
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[link_id];
		if (!IS_ENTRY_MLO(entry))
			continue;

		mlo_info = &entry->mlo;
		link_info = &entry->mlo.link_info;
		if (mlo_info->is_setup_link_entry)
			*last_setup_link_entry = entry;
		if (rm_link_id_bitmap & BIT(link_id)) {
			link_info->link_sts = LINK_STS_REMOVE;
			if (mlo_info->is_setup_link_entry) {
				mlo_info->is_setup_link_entry = 0;
				*is_rm_link_on_setup_link = 1;
			}
		}
		if (link_info->link_sts != LINK_STS_REMOVE)
			*available_link_id_bitmap |= BIT(link_id);
	}

	mt_rcu_read_unlock();
	return ret;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
		 "err=%d\n", ret);
	return ret;
}

/**
 * @ML reconfiguration: handle peer mld sta including signle link mld sta case
 *
 * @param *mld_addr mld addr of non-ap mld
 * @param available_link_id_bitmap available link id bitmap
 */
int bss_mngr_event_mld_reconfig(
	IN u8 *mld_addr,
	IN u16 rm_link_id_bitmap
)
{
	int ret = 0;
	u8 is_rm_link_on_setup_link = 0;
	u16 available_link_id_bitmap = 0;
	struct _MAC_TABLE_ENTRY *last_setup_link_entry = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
		 "mld_sta=%pM\n", mld_addr);

	ret = bss_mngr_reconfig_available_link_find(mld_addr,
		rm_link_id_bitmap,
		&available_link_id_bitmap,
		&is_rm_link_on_setup_link,
		&last_setup_link_entry);

	if (ret) {
		ret = -ENXIO;
		goto err;
	}

	if (!last_setup_link_entry) {
		ret = -ENXIO;
		goto err;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
		 "rm_link_id=0x%.2x, ava_link_id=0x%.2x, rm_link_on_setup_link=%d, last_setup_link=%pM\n",
		 rm_link_id_bitmap,
		 available_link_id_bitmap,
		 is_rm_link_on_setup_link,
		 last_setup_link_entry->Addr);

	if (is_rm_link_on_setup_link) {
		if (available_link_id_bitmap) {
			/*select a new setup link*/
			bss_mngr_reconfig_sta_setup_link_set(mld_addr, available_link_id_bitmap);
		} else {
			/*disassoc non-AP MLD*/
			cntl_disconnect_request(last_setup_link_entry->wdev,
				CNTL_DEAUTH,
				last_setup_link_entry->Addr,
				REASON_NO_LONGER_VALID);
		}
	}

	return ret;
err:
	return ret;
}

#endif

struct bss_mngr_event_ops bmgr_event_ops = {
	.bmgr_event_bcn_update = bss_mngr_event_bcn_update,
	.bmgr_event_bss_attr_update = bss_mngr_event_bss_attr_update,
	.bmgr_event_ie_fragment = bss_mngr_event_ie_fragment,
#ifdef DOT11_EHT_BE
	.bmgr_event_add_mld = bss_mngr_event_add_mld,
	.bmgr_event_del_mld = bss_mngr_event_del_mld,
	.bmgr_event_mld_add_link = bss_mngr_event_mld_add_link,
	.bmgr_event_mld_del_link = bss_mngr_event_mld_del_link,
	.bmgr_event_mld_add_sta = bss_mngr_event_add_sta,
	.bmgr_event_mld_conn_act = bss_mngr_event_conn_act,
	.bmgr_event_mld_del_sta = bss_mngr_event_del_sta,
	.bmgr_event_mld_disconn_mld_sta = bss_mngr_event_disconn_mld_sta,
	.bmgr_event_mld_disconn_act = bss_mngr_event_disconn_act,
	.bmgr_event_mld_query_probe_rsp_frame = bss_mngr_event_query_probe_rsp_frame,
	.bmgr_event_mld_form_ml_probe_rsp_sta_pf = bss_mngr_event_form_ml_probe_rsp_sta_pf,
	.bmgr_event_mld_ba_ori_setup = bss_mngr_event_ba_ori_setup,
	.bmgr_event_mld_ba_resrc_ori_add = bss_mngr_event_ba_resrc_ori_add,
	.bmgr_event_mld_ba_resrc_rec_add = bss_mngr_event_ba_resrc_rec_add,
	.bmgr_event_mld_ba_resrc_ori_del = bss_mngr_event_ba_resrc_ori_del,
	.bmgr_event_mld_ba_resrc_rec_del = bss_mngr_event_ba_resrc_rec_del,
	.bmgr_event_mld_ba_add_to_asic = bss_mngr_event_ba_add_to_asic,
	.bmgr_event_mld_ba_del_from_asic = bss_mngr_event_ba_del_from_asic,
	.bmgr_event_mld_twt_action_frm_forward = bss_mngr_event_twt_action_frm_forward,
	.bmgr_event_mld_twt_action_frm_breadown = bss_mngr_event_twt_action_frm_breadown,
	.bmgr_event_mld_twt_action_frm_agg = bss_mngr_event_twt_action_frm_agg,
	.bmgr_event_mld_twt_wait_ack_inform = bss_mngr_event_mld_twt_wait_ack_inform,
#ifdef MWDS
	.bmgr_event_mld_mwds_enable = bss_mbgr_event_mld_mwds_enable,
#endif
	.bmgr_event_mld_critical_update_bcn = bss_mngr_event_mld_critical_update_bcn,
	.bmgr_event_mld_critical_update_op_lock = bss_mngr_event_mld_critical_update_op_lock,
	.bmgr_event_mld_eml_op_update = bss_mngr_event_mld_eml_op_update,
	.bmgr_event_mld_tx_link_recomm = bss_mngr_event_tx_link_recomm,
	.bmgr_event_mld_reconfig = bss_mngr_event_mld_reconfig,
#endif
};

static UCHAR get_opclass_for_rnr(struct _RTMP_ADAPTER *pAd, IN struct wifi_dev *wdev)
{
	UCHAR op_class = get_regulatory_class(pAd,
		wdev->channel, wdev->PhyMode, wdev);

	/* From Cert7 test plan v1.0 4.20:			*/
	/* NOTE: Operating Class 137 is not indicated in RNR,	*/
	/* for back-compatibility with Wi-Fi 6E STAs.			*/
	/* Operating Class subfield set to 131, 132, 133 or 134 */
	/* when operating Class is 137 on 6G band.				*/
	if (op_class == 137)
		op_class = 134;

	return op_class;
}

NDIS_STATUS bss_mngr_fill_bss_info(
	IN struct wifi_dev		*wdev,
	OUT struct bmgr_reg_info	*reg_info
)
{
	struct _RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
#endif
	PSECURITY_CONFIG pSecConfig = &wdev->SecConfig;
	u8 band_idx = 0;
	ULONG sys_time;

	u8 dot11v_mbssid_idx = 0;
	if (wdev) {
#ifdef CONFIG_AP_SUPPORT
		if (wdev->wdev_type == WDEV_TYPE_AP)
			pMbss = wdev->func_dev;
#endif
	} else {
		return NDIS_STATUS_FAILURE;
	}
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	if (ad == NULL)
		return NDIS_STATUS_FAILURE;
	band_idx = HcGetBandByWdev(wdev);
	reg_info->t_bss = NULL;
#if defined(CONFIG_AP_SUPPORT) && defined(DOT11V_MBSSID_SUPPORT)
	if (pMbss) {
		struct mbss_11v_ctrl *bss_11v_ctrl = &pMbss->mbss_11v;
		struct _BSS_STRUCT *t_bss = pMbss;
		dot11v_mbssid_idx = mbss_11v_group_add(ad, wdev);

		if (IS_BSSID_11V_TRANSMITTED(bss_11v_ctrl))
			reg_info->dev_type = DEV_TYPE_AP_11VT;
		else if (IS_BSSID_11V_NON_TRANS(bss_11v_ctrl)) {
			reg_info->dev_type = DEV_TYPE_AP_11VNT;
			t_bss = bss_11v_ctrl->mbss_11v_t_bss;
		} else if (IS_BSSID_11V_CO_HOSTED(bss_11v_ctrl))
			reg_info->dev_type = DEV_TYPE_AP_COH;
		else {
			reg_info->dev_type = DEV_TYPE_AP_LEGACY;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"dev_type UNDEFINED\n");
		}

		reg_info->max_bssid_indicator =
			ad->ApCfg.dot11v_max_indicator[bss_11v_ctrl->mbss_11v_grp_idx];

		reg_info->t_bss = t_bss;
	} else
#endif
		reg_info->dev_type = DEV_TYPE_AP_LEGACY;

	reg_info->phy_mode				= wdev->PhyMode;
	reg_info->hw_bss_index			= wdev->bss_info_argument.ucBssIndex;
#ifdef CONFIG_AP_SUPPORT
#ifdef CONFIG_6G_SUPPORT
	reg_info->rnr_6g_rule			= wlan_config_get_rnr_in_probe_rsp(wdev);
#endif /* CONFIG_6G_SUPPORT */
	reg_info->single_rnr			= ad->CommonCfg.single_rnr;
	reg_info->bcn_interval			= ad->CommonCfg.BeaconPeriod;
#endif /* CONFIG_AP_SUPPORT */
	reg_info->channel				= wdev->channel;

#ifdef CONFIG_AP_SUPPORT
	if (pMbss) {
		reg_info->dot11v_mbssid_idx	= dot11v_mbssid_idx;
		reg_info->dtim_period		= pMbss->DtimPeriod;
		reg_info->cap_info			= cpu2le16(pMbss->CapabilityInfo);

		/* ssid bssid */
		reg_info->ssid_len			= pMbss->SsidLen;
		reg_info->is_hide_ssid		= pMbss->bHideSsid;
		NdisMoveMemory(reg_info->ssid, pMbss->Ssid, (MAX_LEN_OF_SSID + 1));

#ifdef DOT11_EHT_BE
		reg_info->mld_grp			= pMbss->mld_grp_idx;
		reg_info->tid_map			= pMbss->tid_mapping;
		reg_info->dev_cfg_idx		= get_dev_config_idx(ad);
		reg_info->mld_addr_by_cfg	= pMbss->mld_addr_by_cfg;
		if (reg_info->mld_addr_by_cfg)
			COPY_MAC_ADDR(reg_info->pf_mld_addr, pMbss->pf_mld_addr);
#endif
	}
#endif

	reg_info->op_class = get_opclass_for_rnr(ad, wdev);

	NdisMoveMemory(reg_info->bssid, wdev->bssid, MAC_ADDR_LEN);

	/* security */
	reg_info->auth_mode			= GET_SEC_AKM(pSecConfig);
	reg_info->PairwiseCipher	= GET_PAIRWISE_CIPHER(pSecConfig);
	reg_info->GroupCipher		= GET_GROUP_CIPHER(pSecConfig);

	/* device register time info */
	NdisGetSystemUpTime(&sys_time);
	reg_info->reg_time = jiffies_to_usecs(sys_time);
	HW_GET_TSF(wdev, reg_info->reg_hw_tsf);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"phymode=%d, reg_time=%u, hw_tsf=%lu\n", reg_info->phy_mode, reg_info->reg_time,
		(UINT64)reg_info->reg_hw_tsf[0] + ((UINT64)reg_info->reg_hw_tsf[1] << 32));

	/* event handler (bmgr->drv) */
	reg_info->event_ops			= &bmgr_event_ops;

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_init(void)
{
	return bmgr_ops.bmgr_init();
}

NDIS_STATUS bss_mngr_deinit(void)
{
	return bmgr_ops.bmgr_deinit();
}

NDIS_STATUS bss_mngr_show_info(u8 type)
{
	return bmgr_ops.bmgr_show_info(type);
}

NDIS_STATUS bss_mngr_dev_reg(
	IN struct wifi_dev	*wdev
)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bmgr_reg_info dev_reg_info;

	NdisZeroMemory(&dev_reg_info, sizeof(dev_reg_info));

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"wdev(%d)\n", wdev->wdev_idx);
			bss_mngr_fill_bss_info(wdev, &dev_reg_info);
			ret = bmgr_ops.bmgr_bss_reg(wdev->if_dev, &dev_reg_info);
		}
	}

	return ret;
}

NDIS_STATUS bss_mngr_dev_dereg(
	IN struct wifi_dev	*wdev
)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"wdev(%d)\n", wdev->wdev_idx);
#ifdef DOT11V_MBSSID_SUPPORT
			mbss_11v_group_remove((struct _RTMP_ADAPTER *)wdev->sys_handle, wdev);
#endif /* DOT11V_MBSSID_SUPPORT */
			ret = bmgr_ops.bmgr_bss_dereg(wdev->if_dev);
		}
	}

	return ret;
}

NDIS_STATUS bss_mngr_dev_update(struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bmgr_reg_info dev_reg_info;

	NdisZeroMemory(&dev_reg_info, sizeof(dev_reg_info));

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP &&
			WDEV_BSS_STATE(wdev) >= BSS_READY) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"wdev(%d)\n", wdev->wdev_idx);
			bss_mngr_fill_bss_info(wdev, &dev_reg_info);
			ret = bmgr_ops.bmgr_bss_update(wdev->if_dev, &dev_reg_info);
		}
	}

	return ret;

}

NDIS_STATUS bss_mngr_ie_update(struct wifi_dev *wdev)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			return bmgr_ops.bmgr_ie_update(wdev->if_dev);
		}
	}
	return NDIS_STATUS_FAILURE;
}

void bss_mngr_sync_bcn_update(struct wifi_dev *wdev)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"wdev(%d)\n", wdev->wdev_idx);

			bmgr_ops.bmgr_sync_bcn_update(wdev->if_dev);
		}
	}
}
struct _RTMP_ADAPTER *bss_mngr_mld_get_pad_by_band(struct wifi_dev *wdev, UCHAR band_idx)
{
	struct _RTMP_ADAPTER *pad = NULL;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			pad = (struct _RTMP_ADAPTER *)bmgr_ops.bmgr_mld_get_pad_by_band_idx(band_idx);
		}
	}
	return pad;
}

#ifdef DOT11V_MBSSID_SUPPORT
u16 bss_mngr_query_mbssid_ie(struct wifi_dev *wdev, struct mbss_query_info *mbss_info)
{
	int ret = NDIS_STATUS_FAILURE;
	u16 f_len = 0;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"wdev(%d), is_probe(%d)\n",
					wdev->wdev_idx, mbss_info->is_probe_rsp);
			ret = bmgr_ops.bmgr_query_mbss_info(wdev->if_dev, mbss_info);
		}
	}

	if (ret != NDIS_STATUS_SUCCESS)
		return 0;

	f_len = mbss_info->f_len;
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
			"\tie len = %d, f_buf = %p\n", f_len, mbss_info->f_buf);

	hex_dump_with_cat_and_lvl("MBSS_IE:", mbss_info->f_buf, f_len,
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

	return f_len;
}
#endif

u16 bss_mngr_query_rnr_ie(struct wifi_dev *wdev, u8 *f_buf)
{
	int ret = NDIS_STATUS_FAILURE;
	u16 f_len;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"wdev(%d)\n", wdev->wdev_idx);
			ret = bmgr_ops.bmgr_query_rnr_info(wdev->if_dev, f_buf, &f_len);
		}
	}

	if (ret != NDIS_STATUS_SUCCESS)
		return 0;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
			"\tie len = %d, f_buf = %p\n", f_len, f_buf);

	hex_dump_with_cat_and_lvl("RNR_IE:", f_buf, f_len,
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

	return f_len;
}

bool bss_mngr_query_is_repted_ssid(struct wifi_dev *wdev, u32 short_ssid, u8 *ssid, u8 ssid_len)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"wdev(%d)\n", wdev->wdev_idx);
			return bmgr_ops.bmgr_query_is_repted_ssid(wdev->if_dev, short_ssid, ssid, ssid_len);
		}
	}

	return FALSE;
}

#ifdef CONFIG_6G_SUPPORT
NDIS_STATUS bss_mngr_update_oob_6g_disc(struct wifi_dev *wdev, u8 rnr_6g_rule)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"wdev(%d)\n", wdev->wdev_idx);
			ret = bmgr_ops.bmgr_update_oob_6g_disc(wdev->if_dev, rnr_6g_rule);
		}
	}

	return ret;
}
#endif /* CONFIG_6G_SUPPORT */

struct wifi_dev *bss_mngr_query_wdev_by_band(u8 band)
{
	struct query_band_netdev query_info = {0};

	/* BAND_24G / BAND_5G / BAND_6G */
	query_info.band = band;
	bmgr_ops.bmgr_query_netdev_by_band(&query_info);

	if (query_info.pNetDev)
		return RtmpOsGetNetDevWdev(query_info.pNetDev);

	return NULL;
}

#ifdef DOT11_EHT_BE
NDIS_STATUS bss_mngr_fill_mld_attr(
	IN struct eht_mld_param *mld_param,
	OUT struct bmgr_mld_attr *mld_attr
)
{

	if (!mld_param || !mld_attr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Invalid input param(%p), mld_attr(%p)\n",
			mld_param, mld_attr);
		return NDIS_STATUS_INVALID_DATA;
	}

	/* Medium Synchronization Delay */
	mld_attr->med_sync.en = mld_param->msd_en;
	mld_attr->med_sync.dur = mld_param->msd_dur;
	mld_attr->med_sync.ofdm_ed_thr = mld_param->msd_ofdm_ed_thr;
	mld_attr->med_sync.max_txop = mld_param->msd_max_txop;

	/* EML Capabilities */
	switch (mld_param->eml_mode) {
	case EML_DISABLE:
		mld_attr->eml_caps.emlsr_supp = FALSE;
		mld_attr->eml_caps.emlmr_supp = FALSE;
		mld_attr->eml_caps.en = FALSE;
		break;

	case EMLSR:
		mld_attr->eml_caps.emlsr_supp = TRUE;
		mld_attr->eml_caps.emlmr_supp = FALSE;
		mld_attr->eml_caps.en = TRUE;
		break;

	case EMLMR:
		mld_attr->eml_caps.emlsr_supp = FALSE;
		mld_attr->eml_caps.emlmr_supp = TRUE;
		mld_attr->eml_caps.en = TRUE;
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"eml_mode %d is invalid\n", mld_param->eml_mode);
		mld_attr->eml_caps.emlsr_supp = FALSE;
		mld_attr->eml_caps.emlmr_supp = FALSE;
		mld_attr->eml_caps.en = FALSE;
	}

	mld_attr->eml_caps.trans_to = mld_param->eml_trans_to;
	mld_attr->eml_caps.eml_omn_en = mld_param->eml_omn;

	/* MLD Capabilities */
	mld_attr->mld_caps.en = TRUE;
	mld_attr->mld_caps.t2l_nego_supp = mld_param->t2lm_nego_supp;

	return NDIS_STATUS_SUCCESS;
}

/**
 * Core API that create struct bmgr_mlo_dev in bssmngr.
 *
 * @mld_grp: MLD group index for which the MLD is to be created (unique among all MLD)
 * @mld_addr: MLD MAC address (unique among all MLD)
 * @mld_param: MLD attributes of the MLD to be created
 * @mld_type: MLD type, single or multi.
 */
NDIS_STATUS bss_mngr_mld_group_create(
	struct wifi_dev *wdev,
	u8 *mld_grp,
	u8 *mld_addr,
	struct eht_mld_param *mld_param,
	u8 mld_type
)
{
	int ret;
	struct bmgr_mld_attr mld_attr = {0};

	bss_mngr_fill_mld_attr(mld_param, &mld_attr);
	ret = bmgr_ops.bmgr_mld_group_create(wdev->if_dev, mld_grp, mld_addr, &mld_attr, mld_type);

	return ret;
}

/**
 * API that delete struct bmgr_mlo_dev in bssmngr.
 *
 * @mld_grp: MLD group index for which the MLD is to be deleted (unique among all MLD)
 */
NDIS_STATUS bss_mngr_mld_group_destroy(
	struct wifi_dev *wdev,
	u8 mld_grp)
{
	int ret;

	ret = bmgr_ops.bmgr_mld_group_destroy(wdev->if_dev, mld_grp);

	return ret;
}

/**
 * API that set/update the attributes of AP MLD (struct bmgr_mlo_dev in bssmngr).
 *
 * @mld_grp: MLD group index for which the MLD is to be set (unique among all MLD)
 */
NDIS_STATUS bss_mngr_mld_group_attr_set(
	u8 mld_grp,
	struct eht_mld_op_ctrl *op_ctrl)
{
	int ret;
	struct bmgr_mld_attr mld_attr = {0};

	bss_mngr_fill_mld_attr(&op_ctrl->mld_param, &mld_attr);
	ret = bmgr_ops.bmgr_mld_group_attr_set(mld_grp,
		op_ctrl->pres_bmap, &mld_attr, op_ctrl->cfg_disconn);

	return ret;
}

/**
 * API that add BSS/Link to the MLD (identified by mld_grp) in bssmngr.
 *
 * @wdev: BSS that is going to join MLD.
 * @mld_grp: MLD group index of MLD to which the BSS requests to join
 */
NDIS_STATUS bss_mngr_mld_group_add_link(struct wifi_dev *wdev, u8 mld_grp)
{
	int ret = NDIS_STATUS_FAILURE;

	if (mld_grp) {
		if (wdev) {
			if (wdev->wdev_type == WDEV_TYPE_AP)
				ret = bmgr_ops.bmgr_mld_group_add_link(wdev->if_dev, mld_grp);
		}
	} else
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"mld_grp(%d) shall be non-zero\n", mld_grp);

	return ret;
}

/**
 * API that delete BSS/Link in the MLD in bssmngr.
 *
 * @wdev: BSS that is going to leave MLD.
 */
NDIS_STATUS bss_mngr_mld_group_del_link(struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			ret = bmgr_ops.bmgr_mld_group_del_link(wdev->if_dev);
	}

	return ret;
}

u16 bss_mngr_query_mld_info(struct wifi_dev *wdev, struct query_mld_info *mld_query)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"wdev(%d)\n", wdev->wdev_idx);
		ret = bmgr_ops.bmgr_query_mld_info(wdev->if_dev, mld_query);
	} else {
		/* queries that do not need dev */
		switch (mld_query->query_type) {
		case BMGR_QUERY_MLD:
		case BMGR_QUERY_BSSID_ML_INFO:
		case BMGR_QUERY_MLD_CONN:
			ret = bmgr_ops.bmgr_query_mld_info(NULL, mld_query);
			break;
		default:
			break;
		}
	}

	if ((mld_query->query_type == BMGR_QUERY_ML_IE_BCN) ||
		(mld_query->query_type == BMGR_QUERY_ML_IE_PROBE_RSP) ||
		(mld_query->query_type == BMGR_QUERY_ML_IE_AUTH_RSP) ||
		(mld_query->query_type == BMGR_QUERY_ML_IE_ASSOC_RSP)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"\tIE: Type = %d, ie len = %d, ret=%d\n",
				mld_query->query_type,
				mld_query->ie.f_len,
				ret);

		hex_dump_with_cat_and_lvl("ML_IE:", mld_query->ie.f_buf, mld_query->ie.f_len,
			DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

		return mld_query->ie.f_len;
	}

	return ret;
}

u16 bss_mngr_query_tid_to_link_ie(struct wifi_dev *wdev, u8 *f_buf)
{
	int ret = NDIS_STATUS_FAILURE;
	u16 f_len;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"wdev(%d)\n", wdev->wdev_idx);
			ret = bmgr_ops.bmgr_query_tid_to_link_info(wdev->if_dev, f_buf, &f_len);
		}
	}

	if (ret != NDIS_STATUS_SUCCESS)
		return 0;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
			"\tie len = %d, f_buf = %p\n", f_len, f_buf);

	hex_dump_with_cat_and_lvl("T2L_IE:", f_buf, f_len,
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

	return f_len;
}

u16 bss_mngr_query_multi_link_traffic_ie(struct wifi_dev *wdev, u8 *f_buf_hdr, u8 *f_buf)
{
	int ret = NDIS_STATUS_FAILURE;
	u16 f_len;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			ret = bmgr_ops.bmgr_query_multi_link_traffic_ie(wdev->if_dev, f_buf, &f_len);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"wdev(%s),len=%d,ret=%d\n", wdev->if_dev->name, f_len, ret);
		}
	}

	if (ret != NDIS_STATUS_SUCCESS)
		return 0;

	wdev->bcn_buf.mlt_ie_offset = f_buf - f_buf_hdr;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
		"\tmlt len=%d, mlt_ie_offset=%d\n", f_len, wdev->bcn_buf.mlt_ie_offset);

	hex_dump_with_cat_and_lvl("MLT_IE:", f_buf, f_len,
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);

	return f_len;
}

NDIS_STATUS bss_mngr_mld_reconfig_ie_op(struct mld_reconfig_ie_op *ie_op)
{
	if (ie_op)
		return bmgr_ops.bmgr_mld_reconfig_ie_op(ie_op);

	return NDIS_STATUS_FAILURE;
}

int bss_mngr_mld_reconfig_ie_build(u8 mld_grp_idx, struct mld_reconfig_ie_build_t *build)
{
	int ret;
	struct mld_reconfig_ie_op ie_op = {0};

	ie_op.op = BMGR_RECONFIG_IE_OP_BUILD;
	ie_op.mld_grp_idx = mld_grp_idx;
	ie_op.reconf_build = build;

	ret = bss_mngr_mld_reconfig_ie_op(&ie_op);

	return ret;
}

int bss_mngr_mld_reconfig_ie_clean(u8 mld_grp_idx)
{
	int ret;
	struct mld_reconfig_ie_op ie_op = {0};

	ie_op.op = BMGR_RECONFIG_IE_OP_CLEAN;
	ie_op.mld_grp_idx = mld_grp_idx;

	ret = bss_mngr_mld_reconfig_ie_op(&ie_op);

	return ret;
}

int bss_mngr_mld_reconfig_ie_clean_link(u8 mld_grp_idx, struct mld_reconfig_ie_clean_t *clean)
{
	int ret;
	struct mld_reconfig_ie_op ie_op = {0};

	ie_op.op = BMGR_RECONFIG_IE_OP_CLEAN_LINK;
	ie_op.mld_grp_idx = mld_grp_idx;
	ie_op.reconf_clean = clean;

	ret = bss_mngr_mld_reconfig_ie_op(&ie_op);

	return ret;
}

u16 bss_mngr_mld_reconfig_ie_query(u8 mld_grp_idx, struct mld_reconfig_ie_query_t *query)
{
	int ret;
	struct mld_reconfig_ie_op ie_op = {0};

	ie_op.op = BMGR_RECONFIG_IE_OP_QUERY;
	ie_op.mld_grp_idx = mld_grp_idx;
	ie_op.reconf_query = query;

	ret = bss_mngr_mld_reconfig_ie_op(&ie_op);

	return (ret == NDIS_STATUS_SUCCESS) ? query->f_len : 0;
}

struct wifi_dev *bss_mngr_query_group_wdev_by_link(u8 group_id, u8 link_id)
{
	struct query_group_link_netdev query_info = {0};

	query_info.group_id = group_id;
	query_info.link_id = link_id;
	query_info.pNetDev = NULL;
	bmgr_ops.bmgr_mld_query_netdev_by_link(&query_info);

	if (query_info.pNetDev)
		return RtmpOsGetNetDevWdev(query_info.pNetDev);
	else
		return NULL;
}

bool bss_mngr_is_wdev_in_mlo_group(struct wifi_dev *wdev)
{

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

		return bmgr_ops.bmgr_query_is_mld_wdev(wdev->if_dev);
	}

	return FALSE;
}

bool bss_mngr_is_in_mlo_csa(struct wifi_dev *wdev)
{

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP)
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

		return bmgr_ops.bmgr_query_is_in_mlo_csa(wdev->if_dev);
	}

	return FALSE;
}

u16 bss_mngr_mld_bss_linkup(
	struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			ret = bmgr_ops.bmgr_mld_bss_linkup(wdev->if_dev);
		}
	}

	return ret;
}

u16 bss_mngr_mld_bss_linkdown(
	struct wifi_dev *wdev)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			ret = bmgr_ops.bmgr_mld_bss_linkdown(wdev->if_dev);
		}
	}

	return ret;
}

u16 bss_mngr_mld_bmc_sn_qry(
	struct wifi_dev *wdev,
	u8 *mld,
	u8 *agent_bss,
	u16 *sn,
	struct wifi_dev **wdev_list)
{
	int ret = NDIS_STATUS_FAILURE;
	u8 i = 0;
	PNET_DEV if_dev_list[BSS_MNGR_MAX_BAND_NUM] = {NULL};

	if (wdev && wdev->wdev_type == WDEV_TYPE_AP) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"wdev(%d)\n", wdev->wdev_idx);

		ret = bmgr_ops.bmgr_mld_bmc_sn_qry(wdev->if_dev, mld, agent_bss, sn, if_dev_list);
		if (ret == NDIS_STATUS_SUCCESS) {
			if (*agent_bss) {
				for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
					wdev_list[i] = NULL;
					if (!if_dev_list[i])
						continue;
					wdev_list[i] =  RTMP_OS_NETDEV_GET_WDEV(if_dev_list[i]);
				}
			}
		}
	}

	return ret;
}

u16 bss_mngr_mld_conn_req(struct wifi_dev *wdev, struct mld_conn_req *mld_conn)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			ret = bmgr_ops.bmgr_mld_conn_req(wdev->if_dev, mld_conn);
		}
	}

	return ret;
}

u16 bss_mngr_mld_conn_act(struct wifi_dev *wdev, u16 mld_sta_idx)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			ret = bmgr_ops.bmgr_mld_conn_act(wdev->if_dev, mld_sta_idx);
		}
	}

	return ret;
}

u16 bss_mngr_mld_disconn_op(struct wifi_dev *wdev, u16 mld_sta_idx, u8 op)
{
	int ret = NDIS_STATUS_FAILURE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d), op(%d)\n", wdev->wdev_idx, op);

			ret = bmgr_ops.bmgr_mld_disconn_op(wdev->if_dev, mld_sta_idx, op);
		}
	}

	return ret;
}

int bss_mngr_query_mld_basic(struct query_mld_basic *mld_basic)
{
	int ret;
	struct query_mld_info mld_query = {0};

	mld_query.query_type	= BMGR_QUERY_MLD;
	mld_query.mld_basic		= mld_basic;

	ret = bss_mngr_query_mld_info(NULL, &mld_query);

	if (ret == NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"Addr(%pM) grp(%d) type(%d) link_cnt(%d)\n",
				mld_basic->addr, mld_basic->mld_grp_idx,
				mld_basic->mld_type, mld_basic->link_cnt);
	}
	return ret;
}

int bss_mngr_query_mld_ap_basic(struct wifi_dev *wdev, struct query_mld_ap_basic *basic)
{
	int ret = NDIS_STATUS_FAILURE;
	struct query_mld_info mld_query = {0};

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			mld_query.query_type	= BMGR_QUERY_ML_AP_BASIC;
			mld_query.basic			= basic;

			ret = bss_mngr_query_mld_info(wdev, &mld_query);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"Basic link_cnt = %d, link_id = %d, %pM\n",
					basic->link_cnt, basic->link_id, basic->addr);
		}
	}
	return ret;
}

int bss_mngr_query_mld_by_bssid(struct query_mld_ap_basic *basic)
{
	int ret;
	struct query_mld_info mld_query = {0};

	mld_query.query_type = BMGR_QUERY_BSSID_ML_INFO;
	mld_query.basic = basic;

	ret = bss_mngr_query_mld_info(NULL, &mld_query);

	if (ret == NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"Basic link_cnt = %d, link_id = %d, %pM\n",
			basic->link_cnt, basic->link_id, basic->addr);
	}
	return ret;
}

u16 bss_mngr_query_mld_sta_aid(struct wifi_dev *wdev, u16 mld_sta_idx)
{
	int ret;
	struct query_mld_info mld_query = {0};
	struct query_mld_sta sta;
	u16 mld_aid = INVALID_AID;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			sta.mld_sta_idx         = mld_sta_idx;

			mld_query.query_type	= BMGR_QUERY_ML_STA;
			mld_query.sta			= &sta;

			ret = bss_mngr_query_mld_info(wdev, &mld_query);

			if (ret == NDIS_STATUS_SUCCESS) {
				mld_aid = sta.aid;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"mld_sta_idx = %d, aid = %d, addr %pM\n",
						sta.mld_sta_idx, sta.aid, sta.mld_addr);
			}
		}
	}

	return mld_aid;
}

int bss_mngr_query_mld_conn(struct query_mld_conn *mld_conn)
{
	int ret;
	struct query_mld_info mld_query = {0};

	mld_query.query_type = BMGR_QUERY_MLD_CONN;
	mld_query.mld_conn = mld_conn;

	ret = bss_mngr_query_mld_info(NULL, &mld_query);

	if (ret == NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"MLD %d, Num of associated MLD STA = %d\n",
			mld_conn->mld_group_idx, mld_conn->mld_sta_num);
	}

	return ret;
}

void bss_mngr_mld_add_sta_profile(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 link_id,
	u8 frame_mld_type,
	u8 *buf,
	u16 buf_len
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			bmgr_ops.bmgr_mld_add_sta_profile(wdev->if_dev,
				mld_sta_idx, link_id, frame_mld_type, buf, buf_len);
		}
	}
}

bool bss_mngr_mld_remove_sta_profile(
	struct wifi_dev *wdev,
	u8 frame_mld_type,
	u8 *elmt_id_arr,
	u16 id_arr_len
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			return bmgr_ops.bmgr_mld_remove_sta_profile(wdev->if_dev,
				frame_mld_type, elmt_id_arr, id_arr_len);
		}
	}

	return FALSE;
}

void bss_mngr_mld_sync_ml_probe_rsp(struct wifi_dev *wdev)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);

			bmgr_ops.bmgr_mld_sync_ml_probe_rsp_per_sta_profile(wdev->if_dev);
		}
	}
}

void bss_mngr_mld_ba_ori_setup(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 TID,
	u16 TimeOut
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			bmgr_ops.bmgr_mld_ba_ori_setup(wdev->if_dev, mld_sta_idx, TID, TimeOut);
		}
	}
}

u8 bss_mngr_mld_ba_resrc_ori_add(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 TID,
	u16 ori_ba_wsize,
	u8 amsdu_en,
	u16 TimeOut
)
{
	int ret = FALSE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			ret = bmgr_ops.bmgr_mld_ba_resrc_ori_add(wdev->if_dev, mld_sta_idx, TID, ori_ba_wsize, amsdu_en, TimeOut);
		}
	}

	return ret;
}

u8 bss_mngr_mld_ba_resrc_rec_add(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 TID,
	u16 TimeOut,
	u16 StartSeq,
	u16 rec_ba_wsize
)
{
	int ret = FALSE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			ret = bmgr_ops.bmgr_mld_ba_resrc_rec_add(wdev->if_dev, mld_sta_idx, TID, TimeOut, StartSeq, rec_ba_wsize);
		}
	}

	return ret;
}

void bss_mngr_mld_ba_resrc_ori_del(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 TID,
	u8 bPassive
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			bmgr_ops.bmgr_mld_ba_resrc_ori_del(wdev->if_dev, mld_sta_idx, TID, bPassive);
		}
	}
}

void bss_mngr_mld_ba_resrc_rec_del(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 TID,
	u8 bPassive
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			bmgr_ops.bmgr_mld_ba_resrc_rec_del(wdev->if_dev, mld_sta_idx, TID, bPassive);
		}
	}
}

void bss_mngr_mld_ba_add_to_asic(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 TID,
	u16 Seq,
	u16 BAWinSize,
	int type,
	u8 amsdu_en
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			bmgr_ops.bmgr_mld_ba_add_to_asic(wdev->if_dev, mld_sta_idx, TID, Seq, BAWinSize, type, amsdu_en);
		}
	}
}

void bss_mngr_mld_ba_del_from_asic(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 TID,
	int type,
	u8 amsdu_en
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			bmgr_ops.bmgr_mld_ba_del_from_asic(wdev->if_dev, mld_sta_idx, TID, type, amsdu_en);
		}
	}
}

void bss_mngr_mld_twt_action_frm_forward(
	struct _RTMP_ADAPTER *ad,
	struct _MLME_QUEUE_ELEM *elem
)
{
	struct _MAC_TABLE_ENTRY *peer_entry = NULL;
	struct wifi_dev *wdev = NULL;

	peer_entry = entry_get(ad, elem->Wcid);
	wdev = peer_entry->wdev;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"master link=%s\n", wdev->if_dev->name);
			bmgr_ops.bmgr_mld_forward_action_to_link(wdev->if_dev, elem);
		}
	}
}

u8 bss_mngr_mld_twt_action_frm_breadown(
	struct _RTMP_ADAPTER *ad,
	struct _MLME_QUEUE_ELEM *elem
)
{
	u8 breakdown = 0;
	u16 mld_sta_idx = 0;
	struct _MAC_TABLE_ENTRY *peer_entry = NULL;
	struct wifi_dev *wdev = NULL;
	struct mld_entry_t *mld_entry;
	int mlo_entry_exist = FALSE;

	peer_entry = entry_get(ad, elem->Wcid);
	wdev = peer_entry->wdev;

	if (peer_entry->mlo.mlo_en) {
		mt_rcu_read_lock();
		mld_entry = get_mld_entry_by_mac(peer_entry->mlo.mld_addr);
		if (mld_entry) {
			mlo_entry_exist = TRUE;
			mld_sta_idx = mld_entry->mld_sta_idx;
		}
		mt_rcu_read_unlock();

		if (mlo_entry_exist && wdev) {
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"IF=%s,wdev_idx=%d\n", wdev->if_dev->name, wdev->wdev_idx);
				breakdown = bmgr_ops.bmgr_mld_breakdown_action_frame(wdev->if_dev, mld_sta_idx, elem);
			}
		}
	}

	MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"breakdown=%d\n", breakdown);

	return breakdown;
}

void bss_mngr_mld_twt_action_frm_agg(
	struct wifi_dev *wdev,
	struct frame_itwt_setup *itwt_setup_frame,
	struct MLO_TWT_PRIV_T *priv
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"IF=%s,wdev_idx=%d\n", wdev->if_dev->name, wdev->wdev_idx);
			bmgr_ops.bmgr_mld_aggregate_action_frame(wdev->if_dev, itwt_setup_frame, priv);
		}
	}
}

void bss_mngr_mld_twt_wait_ack_inform(
	struct wifi_dev *wdev,
	u16 mld_sta_idx,
	u8 action_linkid,
	u16 all_linkid_bitmap,
	u8 wait_ack_sts
)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"IF=%s,wdev_idx=%d,mld_sta_idx=%d,action_linkid=%d,all_linkid_bitmap=0x%x,wait_ack_sts=%d\n",
				wdev->if_dev->name, wdev->wdev_idx, mld_sta_idx,
				action_linkid, all_linkid_bitmap, wait_ack_sts);
			bmgr_ops.bmgr_mld_twt_wait_ack_inform(wdev->if_dev,
				mld_sta_idx, action_linkid, all_linkid_bitmap, wait_ack_sts);
		}
	}
}

#ifdef MWDS
u8 bss_mngr_mld_mwds_enable(struct wifi_dev *wdev, u8 enable)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"IF=%s,wdev_idx=%d\n",
				wdev->if_dev->name, wdev->wdev_idx);
			return bmgr_ops.bmgr_mld_mwds_cap_sync(wdev->if_dev, enable);
		}
	}

	return FALSE;
}
#endif

/* bcn_reason is original reason w/o Critical Update high bit[6] set */
void bss_mngr_mld_critical_update_trigger(struct wifi_dev *wdev, u8 upd_scope, u8 bcn_reason)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"(caller:%pS), wdev_idx:%d , bcn_reason=%x\n", OS_TRACE, wdev->wdev_idx, bcn_reason);
			bmgr_ops.bmgr_mld_critical_update_trigger(wdev->if_dev, upd_scope, bcn_reason);
		}
	}
}

BOOLEAN bss_mngr_mld_critical_update_op_lock(struct wifi_dev *wdev, u8 lock_flag)
{
	BOOLEAN status = FALSE;

	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_INFO,
				"wdev_idx:%d lock_flag:%d,(caller:%pS\n", wdev->wdev_idx, lock_flag, OS_TRACE);
			status = bmgr_ops.bmgr_mld_critical_update_op_lock(wdev->if_dev, lock_flag);
		}
	}
	return status;
}


void bss_mngr_mld_eml_op_update(struct wifi_dev *wdev, struct mld_eml_op *eml_op)
{
	if (wdev) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"wdev(%d)\n", wdev->wdev_idx);
			bmgr_ops.bmgr_mld_eml_op_update(wdev->if_dev, eml_op);
		}
	}
}

void bss_mngr_mld_tx_link_recomm(u8 mld_grp_idx, u16 tx_link_bmap, struct mld_link_recomm *link_recomm)
{
	if (BMGR_VALID_MLD_GRP_IDX(mld_grp_idx) && tx_link_bmap && link_recomm)
		bmgr_ops.bmgr_mld_tx_link_recomm(mld_grp_idx, tx_link_bmap, link_recomm);
}

void bss_mngr_mld_reconfig_peer_mld(u8 mld_grp_idx, u16 rm_link_id_bitmap)
{
	if (BMGR_VALID_MLD_GRP_IDX(mld_grp_idx))
		bmgr_ops.bmgr_mld_reconfig_peer_mld(mld_grp_idx, rm_link_id_bitmap);
}

#endif
