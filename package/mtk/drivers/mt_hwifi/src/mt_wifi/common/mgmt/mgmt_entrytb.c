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

#include <rt_config.h>
#ifdef VOW_SUPPORT
#include <ap_vow.h>
#endif /* VOW_SUPPORT */

static BOOLEAN aid_is_assigned(struct _aid_info *aid_info, UINT16 aid)
{
	/*this line shall not happen, allocation fail case is handled at the allocating stage.*/
	if (aid_info->aid_bitmap == NULL)
		return TRUE;

	if (aid_info->aid_bitmap[aid / 32] & (1 << (aid % 32)))
		return TRUE;

	return FALSE;
}

static void aid_set(struct _aid_info *aid_info, UINT16 aid)
{
	/*it shall not happen. check it before the function is called.*/
	if (aid_info->aid_bitmap == NULL)
		return;

	aid_info->aid_bitmap[aid / 32] |= (1 << (aid % 32));
}


static void aid_clear(struct _aid_info *aid_info, UINT16 aid)
{
	/*it shall not happen. check it before the function is called.*/
	if (aid_info->aid_bitmap == NULL)
		return;

	aid_info->aid_bitmap[aid / 32] &= ~(1 << (aid % 32));
}

static void aid_dump(struct _aid_info *aid_info)
{
	UINT16 i;

	/* it shall not happen, check it for sanity. */
	if (aid_info->aid_bitmap == NULL) {
		MTWF_PRINT("no aid_bitmap\n");
		return;
	}

	for (i = 0; i <= aid_info->max_aid; i++) {
		if (aid_info->aid_bitmap[i/32] > 0) {
			if (i % 32 == 0) {
				MTWF_PRINT("BIT:%d - BIT:%d:\t\t", i,
					(i+31) > INVALID_AID ? INVALID_AID-1 : (i+31));
			}
			MTWF_PRINT("%d", (aid_info->aid_bitmap[i/32] & (1 << (i%32))) >> (i%32));
			if (i % 32 == 31)
				MTWF_PRINT("\n");
		} else
			i += 32;
	}
	MTWF_PRINT("\n");
}

VOID TRTableEntryDump(RTMP_ADAPTER *pAd, INT tr_idx, const RTMP_STRING *caller, INT line)
{
	STA_TR_ENTRY *tr_entry;
	struct seq_ctrl_t *seq_ctrl = NULL;
	MAC_TABLE_ENTRY *pEntry;
	INT qidx;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

	ASSERT(IS_WCID_VALID(pAd, tr_idx));

	if (!IS_WCID_VALID(pAd, tr_idx))
		return;

	MTWF_PRINT("Dump TR_ENTRY called by function %s(%d)\n", caller, line);
	tr_entry = tr_entry_get(pAd, tr_idx);
	seq_ctrl = &tr_entry->seq_ctrl;
	pEntry = entry_get(pAd, tr_idx);

#ifdef DOT11_EHT_BE
	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_ext_t *mld_entry_ext = NULL;

		if (!mld_entry_ext_get(pEntry, &mld_entry_ext))
			seq_ctrl = &mld_entry_ext->tr_entry.seq_ctrl;
	}
#endif /* DOT11_EHT_BE */

	MTWF_PRINT("TR_ENTRY[%d]\n", tr_idx);
#ifdef SW_CONNECT_SUPPORT
	MTWF_PRINT("\tbSw=%d\n", IS_SW_STA(tr_entry));
	MTWF_PRINT("\tbSwMainSta=%d\n", IS_SW_MAIN_STA(tr_entry));
	MTWF_PRINT("\thw_wcid=%u\n", tr_entry->HwWcid);
#endif /* SW_CONNECT_SUPPORT */
	MTWF_PRINT("\tsta_rec_valid=%x\n", pEntry->sta_rec_valid);
	MTWF_PRINT("\tEntryType=%x\n", tr_entry->EntryType);
	MTWF_PRINT("\twdev=%p\n", tr_entry->wdev);
	MTWF_PRINT("\twcid=%d\n", tr_entry->wcid);
	MTWF_PRINT("\tfunc_tb_idx=%d\n", tr_entry->func_tb_idx);
	MTWF_PRINT("\tAddr="MACSTR"\n", MAC2STR(tr_entry->Addr));
	MTWF_PRINT("\tBSSID="MACSTR"\n", MAC2STR(tr_entry->bssid));
	MTWF_PRINT("\tFlags\n");
	MTWF_PRINT("\t\tbIAmBadAtheros=%d, isCached=%d, PortSecured=%d, PsMode=%d, LockEntryTx=%d\n",
			 tr_entry->bIAmBadAtheros, tr_entry->isCached, tr_entry->PortSecured, tr_entry->PsMode, tr_entry->LockEntryTx);
	MTWF_PRINT("\tTxRx Characters\n");
#ifdef SW_CONNECT_SUPPORT
	/* Tx debug Cnts */
	MTWF_PRINT("\t\ttx_fp_allow_cnt=%u\n", tr_entry->tx_fp_allow_cnt);
	MTWF_PRINT("\t\ttx_send_data_cnt=%u\n", tr_entry->tx_send_data_cnt);
	MTWF_PRINT("\t\ttx_deq_eap_cnt=%u\n", tr_entry->tx_deq_eap_cnt);
	MTWF_PRINT("\t\ttx_deq_arp_cnt=%u\n", tr_entry->tx_deq_arp_cnt);
	MTWF_PRINT("\t\ttx_deq_data_cnt=%u\n", tr_entry->tx_deq_data_cnt);
	MTWF_PRINT("\t\ttx_handle_cnt=%u\n", tr_entry->tx_handle_cnt);
	/* Rx debug Cnts */
	MTWF_PRINT("\t\trx_handle_cnt=%u\n", tr_entry->rx_handle_cnt);
	MTWF_PRINT("\t\trx_u2m_cnt=%u\n", tr_entry->rx_u2m_cnt);
	MTWF_PRINT("\t\trx_eap_cnt=%u\n", tr_entry->rx_eap_cnt);
#endif /* SW_CONNECT_SUPPORT */
	if (seq_ctrl) {
		MTWF_PRINT("\t\tNonQosDataSeq=%d\n", seq_ctrl->NonQosDataSeq);
		MTWF_PRINT("\t\tTxSeq[0]=%d, TxSeq[1]=%d, TxSeq[2]=%d, TxSeq[3]=%d\n",
				 seq_ctrl->TxSeq[0], seq_ctrl->TxSeq[1], seq_ctrl->TxSeq[2], seq_ctrl->TxSeq[3]);
#ifdef SW_CONNECT_SUPPORT
		MTWF_PRINT("\t\tRxSeq[0]=%d, RxSeq[1]=%d, RxSeq[2]=%d, RxSeq[3]=%d\n",
				 seq_ctrl->RxSeq[0], seq_ctrl->RxSeq[1], seq_ctrl->RxSeq[2], seq_ctrl->RxSeq[3]);
#endif /* SW_CONNECT_SUPPORT */
	}
	MTWF_PRINT("\tCurrTxRate=%x\n", tr_entry->CurrTxRate);
	MTWF_PRINT("\tQueuing Info\n");
	MTWF_PRINT("\t\tenq_cap=%d, deq_cap=%d\n", tr_entry->enq_cap, tr_entry->deq_cap);
	MTWF_PRINT("\t\tQueuedPkt: TxQ[0]=%u, TxQ[1]=%u, TxQ[2]=%u, TxQ[3]=%u, PSQ=%u, DELAYQ=%u\n",
			 tr_entry->tx_queue[0].Number, tr_entry->tx_queue[1].Number,
			 tr_entry->tx_queue[2].Number, tr_entry->tx_queue[3].Number,
			 tr_entry->ps_queue.Number, tr_entry->delay_queue.Number);
	MTWF_PRINT("\t\tdeq_cnt=%d, deq_bytes=%d\n", tr_entry->deq_cnt, tr_entry->deq_bytes);

	for (qidx = 0; qidx < 4; qidx++) {
		if (qm_ops->sta_dump_queue)
			qm_ops->sta_dump_queue(pAd, tr_entry->wcid, TX_DATA, qidx);

		if (cap->qm == GENERIC_QM)
			ge_tx_swq_dump(pAd, qidx);
	}
}

VOID TRTableResetEntry(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx;

	if (!IS_WCID_VALID(pAd, wcid))
		return;

	tr_entry = tr_entry_get(pAd, wcid);

	if (IS_ENTRY_NONE(tr_entry))
		return;

	tr_entry->enq_cap = FALSE;
	tr_entry->deq_cap = FALSE;

	SET_ENTRY_NONE(tr_entry);

#ifdef SW_CONNECT_SUPPORT
	tr_entry->bSw = FALSE;
	tr_entry->bSwMainSta = FALSE;
	tr_entry->HwWcid = WCID_INVALID;

	/* Reset Tx Counters */
	tr_entry->tx_fp_allow_cnt = 0;
	tr_entry->tx_send_data_cnt = 0;
	tr_entry->tx_deq_eap_cnt = 0;
	tr_entry->tx_deq_arp_cnt = 0;
	tr_entry->tx_deq_data_cnt = 0;
	tr_entry->tx_handle_cnt = 0;

	/* Reset Rx Counters */
	tr_entry->rx_handle_cnt = 0;
	tr_entry->rx_u2m_cnt = 0;
	tr_entry->rx_eap_cnt = 0;
#endif /* SW_CONNECT_SUPPORT */

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++)
		NdisFreeSpinLock(&tr_entry->txq_lock[qidx]);

	NdisFreeSpinLock(&tr_entry->ps_queue_lock);
	NdisFreeSpinLock(&tr_entry->ps_sync_lock);
	NdisFreeSpinLock(&tr_entry->delay_queue_lock);
	return;
}


VOID TRTableInsertEntry(RTMP_ADAPTER *pAd, UINT16 wcid, MAC_TABLE_ENTRY *pEntry)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx;

	if (IS_WCID_VALID(pAd, wcid)) {
		tr_entry = tr_entry_get(pAd, wcid);
		pEntry->tr_tb_idx = wcid;
		tr_entry->EntryType = pEntry->EntryType;
		tr_entry->wdev = pEntry->wdev;
		tr_entry->func_tb_idx = pEntry->func_tb_idx;
		tr_entry->wcid = pEntry->wcid;
		NdisMoveMemory(tr_entry->Addr, pEntry->Addr, MAC_ADDR_LEN);
		seq_ctrl_init(&tr_entry->seq_ctrl);

#ifdef MT_MAC
		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			if (tr_entry->wdev)
				tr_entry->OmacIdx = pEntry->wdev->OmacIdx;
			else
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "wdev == NULL\n");
		}
#endif /* MT_MAC */
		tr_entry->PsMode = PWR_ACTIVE;
		tr_entry->isCached = FALSE;
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		tr_entry->CurrTxRate = pEntry->CurrTxRate;

		for (qidx = 0; qidx < WMM_QUE_NUM; qidx++) {
			InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
			NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
		}

		InitializeQueueHeader(&tr_entry->ps_queue);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_sync_lock);
		/* Initial delay queue and its queue lock */
		InitializeQueueHeader(&tr_entry->delay_queue);
		NdisAllocateSpinLock(pAd, &tr_entry->delay_queue_lock);
		tr_entry->deq_cnt = 0;
		tr_entry->deq_bytes = 0;
		tr_entry->PsQIdleCount = 0;
		tr_entry->enq_cap = TRUE;
		tr_entry->deq_cap = TRUE;
		tr_entry->NoDataIdleCount = 0;
		tr_entry->ContinueTxFailCnt = 0;
		tr_entry->LockEntryTx = FALSE;
		tr_entry->TimeStamp_toTxRing = 0;
		tr_entry->PsDeQWaitCnt = 0;
		NdisMoveMemory(tr_entry->bssid, pEntry->wdev->bssid, MAC_ADDR_LEN);
	}
}


VOID TRTableInsertMcastEntry(RTMP_ADAPTER *pAd, UINT16 hw_wcid, struct wifi_dev *wdev)
{
	struct _STA_TR_ENTRY *tr_entry = NULL;
	INT qidx;
	UINT8 band = DBDC_BAND0;

	if (!IS_WCID_VALID(pAd, wdev->tr_tb_idx))
		return;

	tr_entry = tr_entry_get(pAd, wdev->tr_tb_idx);

	if (!tr_entry)
		return;

	if ((wdev->func_idx) < 0)
		return;

	tr_entry->EntryType = ENTRY_CAT_MCAST;
	tr_entry->wdev = wdev;
	tr_entry->func_tb_idx = wdev->func_idx;
	tr_entry->PsMode = PWR_ACTIVE;
	tr_entry->isCached = FALSE;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	tr_entry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	NdisMoveMemory(tr_entry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);
	tr_entry->wcid = hw_wcid;
	seq_ctrl_init(&tr_entry->seq_ctrl);

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++) {
		InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
		NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
	}

	InitializeQueueHeader(&tr_entry->ps_queue);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_sync_lock);
	tr_entry->deq_cnt = 0;
	tr_entry->deq_bytes = 0;
	tr_entry->PsQIdleCount = 0;
	tr_entry->enq_cap = TRUE;
	tr_entry->deq_cap = TRUE;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (tr_entry->wdev)
			tr_entry->OmacIdx = wdev->OmacIdx;
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "wdev == NULL\n");
	}

#endif /* MT_MAC */
	/*
	    Carter check,
	    if Mcast pkt will reference the bssid field for do something?
	    if so, need to check flow.
	*/
	NdisMoveMemory(tr_entry->bssid, wdev->bssid, MAC_ADDR_LEN);

	band = HcGetBandByWdev(wdev);

	if (!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)) {
#ifdef VOW_SUPPORT
		if (!pAd->vow_cfg.en_bw_ctrl)
#endif
			pAd->bss_group.group_idx[wdev->func_idx] = wdev->func_idx % pAd->max_bssgroup_num;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "band%u group_idx[%d]=%d\n",
		band, wdev->func_idx, pAd->bss_group.group_idx[wdev->func_idx]);
	}
}

VOID MgmtTableSetMcastEntry(RTMP_ADAPTER *pAd, UINT16 hw_wcid, struct wifi_dev *wdev)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_6G_SUPPORT
	UCHAR iob_mode;
#endif /* CONFIG_6G_SUPPORT */

	if (!IS_WCID_VALID_ONLY_HW(pAd, hw_wcid))
		return;

	pEntry = entry_get(pAd, wdev->tr_tb_idx);
	pEntry->pAd = pAd;
	NdisAcquireSpinLock(pAd->MacTabLock);
	pEntry->EntryType = ENTRY_CAT_MCAST;
	NdisReleaseSpinLock(pAd->MacTabLock);
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = hw_wcid;/*FIXME: why BCMC entry needs AID?*/
	pEntry->wcid = hw_wcid;
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	pEntry->Addr[0] = 0x01;/* FIXME: is the code segment necessary?
				* the mac address changes to BROADCAST addr in the below code segment right away.
				*/
	pEntry->HTPhyMode.field.MODE = MODE_OFDM;
	pEntry->HTPhyMode.field.MCS = 3;
	NdisMoveMemory(pEntry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);
	pEntry->wdev = wdev;
#ifdef CONFIG_6G_SUPPORT
	iob_mode = wlan_operate_get_unsolicit_tx_mode(wdev);
	if (WMODE_CAP_6G(wdev->PhyMode)
		&& (iob_mode == UNSOLICIT_TXMODE_NON_HT_DUP))
		pEntry->MaxHTPhyMode.field.BW = BW_80;
#endif /* CONFIG_6G_SUPPORT */
	if (hw_wcid)
		TRTableInsertMcastEntry(pAd, hw_wcid, wdev);
}


VOID MacTableSetEntryPhyCfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE) {
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	} else {
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}
}


VOID MacTableSetEntryRaCap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, struct _vendor_ie_cap *vendor_ie)
{
	ULONG ra_ie = vendor_ie->ra_cap;
	ULONG mtk_ie = vendor_ie->mtk_cap;
#ifdef DOT11_VHT_AC
	ULONG brcm_ie = vendor_ie->brcm_cap;
#endif /*DOT11_VHT_AC*/
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
			 "vendor_ie_cap for ra_cap=%08x, mtk_cap=%08x\n", (UINT32)ra_ie, (UINT32)mtk_ie);
	NdisCopyMemory(&ent->vendor_ie, vendor_ie, sizeof(struct _vendor_ie_cap));
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);

	/* TODO: need MTK CAP ? */

	/* Set cap flags */
	if (vendor_ie->is_rlt == TRUE) {
		CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);

#ifdef DOT11_VHT_AC

		if ((ra_ie & RALINK_256QAM_CAP)
			&& (pAd->CommonCfg.g_band_256_qam)) {
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
					 "RALINK_256QAM_CAP for 2.4G\n");
			ent->fgGband256QAMSupport = TRUE;
		}

#endif /*DOT11_VHT_AC*/
	}

#ifdef DOT11_VHT_AC
	else if ((mtk_ie & MEDIATEK_256QAM_CAP)
			 && (pAd->CommonCfg.g_band_256_qam)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
				 "MEDIATEK_256QAM_CAP for 2.4G\n");
		ent->fgGband256QAMSupport = TRUE;
	} else if ((brcm_ie & BROADCOM_256QAM_CAP)
			   && (pAd->CommonCfg.g_band_256_qam)) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
				 "BROADCOM_256QAM_CAP for 2.4G\n");
		ent->fgGband256QAMSupport = TRUE;
	}

#endif /*DOT11_VHT_AC*/
}

#ifdef DOT11_VHT_AC
VOID MacTableEntryCheck2GVHT(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef G_BAND_256QAM
	struct wifi_dev *wdev = NULL;

	if (!IS_VALID_ENTRY(pEntry) || pEntry->fgGband256QAMSupport)
		return;

	wdev = pEntry->wdev;

	if (wdev) {
		if ((pAd->CommonCfg.g_band_256_qam) &&
			(MCS_NSS_CAP(pAd)->g_band_256_qam) &&
			(WMODE_CAP(wdev->PhyMode, WMODE_GN)) &&
			WMODE_CAP_2G(wdev->PhyMode)) {
			pEntry->fgGband256QAMSupport = TRUE;

			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
				"Peer has 256QAM CAP support for 2.4G!\n");
		}
	}
#endif /* G_BAND_256QAM */
}
#endif /* DOT11_VHT_AC */

MAC_TABLE_ENTRY *MacTableLookupForTx(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL, *pSearchEntry = NULL;
	UINT16 seek_cnt = 0;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
#endif

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pSearchEntry = pAd->MacTab->Hash[HashIdx];

	while (pSearchEntry && !IS_ENTRY_NONE(pSearchEntry)) {
		if ((!wdev || pSearchEntry->wdev == wdev) && MAC_ADDR_EQUAL(pSearchEntry->Addr, pAddr)) {
			pEntry = pSearchEntry;
			break;
		}
		pSearchEntry = pSearchEntry->pNext;
		seek_cnt++;

		if (seek_cnt >= WTBL_MAX_NUM(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
					"!!ERROR!! ("MACSTR"), seek_cnt(%d)\n",
				MAC2STR(pAddr), seek_cnt);
			break;
		}
	}
#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();
	if (pEntry) {
		if (pEntry->mlo.mlo_en) {
			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (mld_entry && mld_entry->valid)
				pEntry = mld_entry_link_select(mld_entry);
		}
	} else {
		mld_entry = get_mld_entry_by_mac(pAddr);
		if (mld_entry && mld_entry->valid)
			pEntry = mld_entry_link_select(mld_entry);
	}
	mt_rcu_read_unlock();
#endif /* DOT11_EHT_BE */

	return pEntry;
}

/*
	==========================================================================
	Description:
		Look up the MAC address in the MAC table. Return NULL if not found.
	Return:
		pEntry - pointer to the MAC entry; NULL is not found
	==========================================================================
*/
MAC_TABLE_ENTRY *MacTableLookup(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL, *pSearchEntry = NULL;
	UINT16 seek_cnt = 0;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pSearchEntry = pAd->MacTab->Hash[HashIdx];

	while (pSearchEntry && !IS_ENTRY_NONE(pSearchEntry)) {
		if (MAC_ADDR_EQUAL(pSearchEntry->Addr, pAddr)) {
			pEntry = pSearchEntry;
			break;
		} else {
			pSearchEntry = pSearchEntry->pNext;
			seek_cnt++;
		}

		if (seek_cnt >= WTBL_MAX_NUM(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				"!!ERROR!! ("MACSTR"), seek_cnt(%d)\n",
				MAC2STR(pAddr), seek_cnt);
			break;
		}
	}

	return pEntry;
}

MAC_TABLE_ENTRY *MacTableLookup2(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL, *pSearchEntry = NULL;
	UINT16 seek_cnt = 0;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pSearchEntry = pAd->MacTab->Hash[HashIdx];

	if (wdev) {
		while (pSearchEntry && !IS_ENTRY_NONE(pSearchEntry)) {
			if (MAC_ADDR_EQUAL(pSearchEntry->Addr, pAddr) && (pSearchEntry->wdev == wdev)) {
				pEntry = pSearchEntry;
				break;
			}	else {
				pSearchEntry = pSearchEntry->pNext;
				seek_cnt++;
			}

			if (seek_cnt >= WTBL_MAX_NUM(pAd)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
					"!!ERROR!! ("MACSTR"), seek_cnt(%d)\n",
					MAC2STR(pAddr), seek_cnt);
				break;
			}
		}
	} else {
		while (pSearchEntry && !IS_ENTRY_NONE(pSearchEntry)) {
			if (MAC_ADDR_EQUAL(pSearchEntry->Addr, pAddr)) {
				pEntry = pSearchEntry;
				break;
			} else {
				pSearchEntry = pSearchEntry->pNext;
				seek_cnt++;
			}

			if (seek_cnt >= WTBL_MAX_NUM(pAd)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
					"!!ERROR!! ("MACSTR"), seek_cnt(%d)\n",
					MAC2STR(pAddr), seek_cnt);
				break;
			}
		}
	}

	return pEntry;
}

#ifdef CONFIG_STA_SUPPORT
static BOOLEAN is_ht_supported(
	struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *peer,
	struct _STA_ADMIN_CONFIG *sta_cfg, struct adhoc_info *adhocInfo,
	struct wifi_dev *wdev, struct common_ies *cmm_ies)
{
	BOOLEAN supported = FALSE;

	if (((!IS_CIPHER_WEP(peer->SecConfig.PairwiseCipher)) &&
		(!IS_CIPHER_TKIP(peer->SecConfig.PairwiseCipher))) ||
		(ad->CommonCfg.HT_DisallowTKIP == FALSE)) {
		if ((sta_cfg->BssType == BSS_INFRA) &&
			HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) && WMODE_CAP_N(wdev->PhyMode))
			supported = TRUE;

		if ((sta_cfg->BssType == BSS_ADHOC) &&
			(adhocInfo->bAdhocN == TRUE) &&
			HAS_HT_CAPS_EXIST(cmm_ies->ie_exists) && WMODE_CAP_N(wdev->PhyMode))
			supported = TRUE;
	}

	return supported;
}

BOOLEAN StaUpdateMacTableEntry(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN IE_LISTS * ie_list,
	IN USHORT cap_info)
{
	UCHAR MaxSupportedRate;
	BOOLEAN bSupportN = FALSE;
#ifdef TXBF_SUPPORT
	BOOLEAN supportsETxBf = FALSE;
#endif
	struct wifi_dev *wdev = NULL;
	STA_TR_ENTRY *tr_entry;
	PSTA_ADMIN_CONFIG pStaCfg = NULL;
	struct adhoc_info *adhocInfo = NULL;
	struct _RTMP_CHIP_CAP *cap;
	struct common_ies *cmm_ies = &ie_list->cmm_ies;
	struct _HT_CAPABILITY_IE *ht_cap = &cmm_ies->ht_cap;

	if (!pEntry)
		return FALSE;

	wdev = pEntry->wdev;
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	adhocInfo = &pStaCfg->adhocInfo;

	if (ADHOC_ON(pAd))
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

	if (WMODE_EQUAL(wdev->PhyMode, WMODE_G)
		&& (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
		return FALSE;

#ifdef DOT11_N_SUPPORT
	/* 11n only */
	if (WMODE_HT_ONLY(wdev->PhyMode) && !HAS_HT_CAPS_EXIST(cmm_ies->ie_exists))
		return FALSE;
#endif /* DOT11_N_SUPPORT */

	NdisAcquireSpinLock(pAd->MacTabLock);
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	NdisZeroMemory(pEntry->SecConfig.Handshake.ReplayCounter, sizeof(pEntry->SecConfig.Handshake.ReplayCounter));

	if ((MaxSupportedRate < RATE_FIRST_OFDM_RATE) ||
		WMODE_EQUAL(wdev->PhyMode, WMODE_B)) {
		pEntry->RateLen = 4;

		if (MaxSupportedRate >= RATE_FIRST_OFDM_RATE)
			MaxSupportedRate = RATE_11;
	} else
		pEntry->RateLen = 12;

	pEntry->MaxHTPhyMode.word = 0;
	pEntry->MinHTPhyMode.word = 0;
	pEntry->HTPhyMode.word = 0;
	pEntry->MaxSupportedRate = MaxSupportedRate;
	MacTableSetEntryPhyCfg(pAd, pEntry);
	pEntry->CapabilityInfo = cap_info;
	CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
#ifdef DOT11_N_SUPPORT
	NdisZeroMemory(&pEntry->HTCapability, sizeof(pEntry->HTCapability));

	bSupportN = is_ht_supported(pAd, pEntry, pStaCfg, adhocInfo, wdev, cmm_ies);
	if (bSupportN) {
		if (ADHOC_ON(pAd))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		ht_mode_adjust(pAd, pEntry, &cmm_ies->ht_cap);
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
		if (cap->FlgHwTxBfCap)
			supportsETxBf = mt_WrapClientSupportsETxBF(pAd, &ht_cap->TxBFCap);

#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(&wdev->DesiredHtPhyInfo.MCSSet[0], &ht_cap->MCSSet[0]);

		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
			set_ht_fixed_mcs(pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);

		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
		set_sta_ht_cap(pAd, pEntry, ht_cap);
		NdisMoveMemory(&pEntry->HTCapability, ht_cap, SIZE_HT_CAP_IE);
		assoc_ht_info_debugshow(pAd, pEntry, ht_cap);
#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(wdev->PhyMode) &&
			HAS_VHT_CAPS_EXIST(cmm_ies->ie_exists) &&
			HAS_VHT_OP_EXIST(cmm_ies->ie_exists)) {
			vht_mode_adjust(pAd, pEntry, &cmm_ies->vht_cap, &cmm_ies->vht_op,
			(cmm_ies->operating_mode_len == 0) ? NULL :  &cmm_ies->operating_mode, NULL);
			dot11_vht_mcs_to_internal_mcs(pAd, wdev,
					&cmm_ies->vht_cap, &pEntry->MaxHTPhyMode);
			set_vht_cap(pAd, pEntry, &cmm_ies->vht_cap);
			NdisMoveMemory(&pEntry->vht_cap_ie, &cmm_ies->vht_cap, sizeof(VHT_CAP_IE));
			assoc_vht_info_debugshow(pAd, pEntry,
					&cmm_ies->vht_cap, &cmm_ies->vht_op);
		}
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(wdev->PhyMode)
				&& HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
			update_peer_he_caps(pEntry, cmm_ies);
			update_peer_he_operation(pEntry, cmm_ies);
			he_mode_adjust(wdev, pEntry, NULL, TRUE);
		}
#endif /*DOT11_HE_AX*/
#ifdef DOT11_EHT_BE
		if (WMODE_CAP_BE(wdev->PhyMode)
				&& HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)) {
			eht_update_peer_caps(pEntry, cmm_ies);
			if (HAS_EHT_OP_EXIST(cmm_ies->ie_exists)) {
				/* TODO: update op capability if need */
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
					"op_parameters: 0x%x, control: 0x%0x, ccfs: %d/%d\n",
					cmm_ies->eht_op.op_parameters,
					cmm_ies->eht_op.op_info.control,
					cmm_ies->eht_op.op_info.ccfs0,
					cmm_ies->eht_op.op_info.ccfs1);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
					"dis_subch_bitmap: 0x%0x\n",
					cmm_ies->dis_subch_bitmap);
			}
		}
#endif /*DOT11_EHT_BE*/
	}
#ifdef DOT11_HE_AX
	else if (WMODE_CAP_AX_6G(wdev->PhyMode)
				&& HAS_HE_CAPS_EXIST(cmm_ies->ie_exists)) {
		update_peer_he_caps(pEntry, cmm_ies);
		update_peer_he_operation(pEntry, cmm_ies);
		he_mode_adjust(wdev, pEntry, NULL, FALSE);
	}
#endif /*DOT11_HE_AX*/
	else
		pAd->MacTab->fAnyStationIsLegacy = TRUE;

#ifdef DOT11_EHT_BE
	if (WMODE_CAP_BE(wdev->PhyMode) && HAS_EHT_CAPS_EXIST(cmm_ies->ie_exists)) {
		eht_update_peer_caps(pEntry, cmm_ies);
		eht_mode_adjust(wdev, pEntry, cmm_ies);
	}
#endif /*DOT11_EHT_BE*/

#endif /* DOT11_N_SUPPORT */
	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
	pEntry->CurrTxRate = pEntry->MaxSupportedRate;
#ifdef MFB_SUPPORT
	pEntry->lastLegalMfb = 0;
	pEntry->isMfbChanged = FALSE;
	pEntry->fLastChangeAccordingMfb = FALSE;
	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = 0; /* has to increment whenever a mrq is sent */
	pEntry->mrqCnt = 0;
	pEntry->pendingMfsi = 0;
	pEntry->toTxMfb = FALSE;
	pEntry->mfbToTx = 0;
	pEntry->mfb0 = 0;
	pEntry->mfb1 = 0;
#endif /* MFB_SUPPORT */
	pEntry->freqOffsetValid = FALSE;
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	mt_WrapTxBFInit(pAd, pEntry, ie_list, supportsETxBf);
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (wdev->bAutoTxRateSwitch == TRUE)
			pEntry->bAutoTxRateSwitch = TRUE;
		else {
			pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
			pEntry->HTPhyMode.field.MODE = wdev->HTPhyMode.field.MODE;
			pEntry->bAutoTxRateSwitch = FALSE;

			if (pEntry->HTPhyMode.field.MODE >= MODE_VHT) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
											  ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
			}

			/* If the legacy mode is set, overwrite the transmit setting of this entry. */
			RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
		}
	}

#endif /* MT_MAC */

	pEntry->Sst = SST_ASSOC;
	pEntry->AuthState = AS_AUTH_OPEN;
	/* pEntry->SecConfig.AKMMap = wdev->SecConfig.AKMMap; */
	/* pEntry->SecConfig.PairwiseCipher = wdev->SecConfig.PairwiseCipher; */
#ifdef HTC_DECRYPT_IOT

	if ((pEntry->HTC_ICVErrCnt)
		|| (pEntry->HTC_AAD_OM_Force)
		|| (pEntry->HTC_AAD_OM_CountDown)
		|| (pEntry->HTC_AAD_OM_Freeze)
	   ) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
			"(wcid=%u), HTC_ICVErrCnt(%u), HTC_AAD_OM_Freeze(%u)\n",
			pEntry->wcid, pEntry->HTC_ICVErrCnt, pEntry->HTC_AAD_OM_Force);
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
			", HTC_AAD_OM_CountDown(%u),  HTC_AAD_OM_Freeze(%u) is in Asso. stage!\n",
			pEntry->HTC_AAD_OM_CountDown, pEntry->HTC_AAD_OM_Freeze);
		/* Force clean. */
		pEntry->HTC_ICVErrCnt = 0;
		pEntry->HTC_AAD_OM_Force = 0;
		pEntry->HTC_AAD_OM_CountDown = 0;
		pEntry->HTC_AAD_OM_Freeze = 0;
		pEntry->HTC_AAD_OM_False_Trigger = 0;
		pEntry->HTC_AAD_OM_Valid_Trigger = 0;
	}

#endif /* HTC_DECRYPT_IOT */

	if (IS_AKM_OPEN(pEntry->SecConfig.AKMMap)
		|| IS_AKM_SHARED(pEntry->SecConfig.AKMMap)
		|| IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap)) {
		pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
		pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	} else {
		pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	}

	/* In WPA or 802.1x mode, the port is not secured. */
	if (IS_AKM_WPA_CAPABILITY(pEntry->SecConfig.AKMMap)
#ifdef DOT1X_SUPPORT
	|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
	)
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	else
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
		"wdev(name=%s,type=%d,PortSecured=%d/%d), (AID=%d, ssid=%s)\n",
		wdev->if_dev->name, wdev->wdev_type, tr_entry->PortSecured, wdev->PortSecured,
		pStaCfg->StaActive.Aid, pStaCfg->Ssid);

	NdisReleaseSpinLock(pAd->MacTabLock);

	return TRUE;
}
#endif /* CONFIG_STA_SUPPORT */


INT MacTableResetEntry(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN clean)
{
	BOOLEAN Cancelled;
	struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
	UINT8 i;
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
		"(caller:%pS)======\n", OS_TRACE);
#ifndef RT_CFG80211_SUPPORT
	RTMPCancelTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
#endif /*RT_CFG80211_SUPPORT*/
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
	OS_CLEAR_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
	RTMPCancelTimer(&pEntry->DABSRetryTimer, &Cancelled);
#endif
#endif
#ifdef DOT11W_PMF_SUPPORT
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
	pSecConfig->PmfCfg.SAQueryConfirmTimes = 0;
#endif /* DOT11W_PMF_SUPPORT */
	ba_session_tear_down_all(pAd, pEntry->wcid, FALSE);

#ifdef SW_CONNECT_SUPPORT
#ifdef CONFIG_LINUX_CRYPTO
	if (pSecConfig->tfm) {
		mt_aead_key_free(pSecConfig->tfm);
		pSecConfig->tfm = NULL;
	}
#endif /* CONFIG_LINUX_CRYPTO */
#endif /* SW_CONNECT_SUPPORT */

	NdisZeroMemory(&pEntry->EntryType, sizeof(MAC_TABLE_ENTRY)-sizeof(STA_TR_ENTRY));

	if (clean == TRUE) {
		pEntry->MaxSupportedRate = RATE_11;
		pEntry->CurrTxRate = RATE_11;

		/* init average rssi */
		for (i = 0; i < RX_STREAM_PATH_SINGLE_MODE; i++)
			pEntry->RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
		pEntry->RssiSample.Rssi_Updated = FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
		"(caller:%pS) End ======\n", OS_TRACE);
	return 0;
}

void vow_set_bss_group(PRTMP_ADAPTER pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef VOW_SUPPORT
	if ((!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG)) || pAd->vow_cfg.en_bw_ctrl) {
		if (VOW_IS_ENABLED(pAd)) {
			UINT32 orig_group_id = pAd->bss_group.group_idx[pEntry->func_tb_idx];

			if (pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG
				&& (pEntry->wdev->func_idx) >= 0) {
				UINT_8 new_group_id = pAd->max_bssgroup_num - pEntry->wdev->func_idx - 1;

				orig_group_id = pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx];

				pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx] = new_group_id;

				pAd->vow_bss_cfg[new_group_id].group_table_idx = pEntry->wdev->func_idx;
				RTMP_SET_STA_DWRR(pAd, pEntry);
				pAd->bss_group.bw_group_idx[pEntry->wdev->func_idx] = orig_group_id;
			} else {
				if (pAd->vow_cfg.en_bw_ctrl) {
					pAd->bss_group.group_idx[pEntry->func_tb_idx] =
						pAd->max_bssgroup_num - pEntry->func_tb_idx - 1;
				}

				RTMP_SET_STA_DWRR(pAd, pEntry);
				pAd->bss_group.group_idx[pEntry->func_tb_idx] = orig_group_id;
			}
		} else
			RTMP_SET_STA_DWRR(pAd, pEntry);
	}
#else
	if (!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG))
		RTMP_SET_STA_DWRR(pAd, pEntry);
#endif
}

MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UINT32 ent_type,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll,
	IN UINT16 mld_sta_idx,
	IN UCHAR *mld_addr)
{
	UCHAR HashIdx;

	BOOLEAN is_apcli = FALSE;
	UINT16 i;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	struct _STA_TR_ENTRY *tr_entry = NULL;
	/* ASIC_SEC_INFO Info = {0}; */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	struct _RTMP_CHIP_CAP *cap;
#ifdef CONFIG_AP_SUPPORT
	struct _BSS_STRUCT *mbss;
#endif /* CONFIG_AP_SUPPORT */

	if (pAd->MacTab->Size >= GET_MAX_UCAST_NUM(pAd)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
		"MacTab->Size full, Size=%d, Max=%d\n", pAd->MacTab->Size, GET_MAX_UCAST_NUM(pAd));
		return NULL;
	}
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	if((ent_type == ENTRY_INFRA) && (OpMode == OPMODE_STA))
		is_apcli = TRUE;
	i = HcAcquireUcastWcid(pAd, wdev, FALSE, is_apcli, mld_sta_idx);
	if (i == WCID_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
		"Entry full!\n");
		return NULL;
	}

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
		"(caller:%pS): wcid %d addr %pM EntryType:%d-%d =====\n",
		OS_TRACE, i, pAddr, ent_type, pAd->MacTab->Content[i].EntryType);

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(pAd->MacTabLock);

	pEntry = entry_get(pAd, i);
	tr_entry = tr_entry_get(pAd, i);
	/* pick up the first available vacancy*/
	if (IS_ENTRY_NONE(pEntry)) {
		MacTableResetEntry(pAd, pEntry, CleanAll);
		/* ENTRY PREEMPTION: initialize the entry */
		pEntry->wdev = wdev;
		pEntry->wcid = i;
		pEntry->func_tb_idx = wdev->func_idx;
		pEntry->bIAmBadAtheros = FALSE;
		pEntry->pAd = pAd;
		pEntry->CMTimerRunning = FALSE;
		pEntry->agg_err_flag = FALSE;
		pEntry->winsize_limit = 0xF;
#ifdef CONFIG_FAST_NAT_SUPPORT
		if (!PD_GET_WHNAT_ENABLE(pAd->physical_dev))
			pEntry->ForceSwPath = FORCE_SW_PATH_ALL;
#endif /* CONFIG_FAST_NAT_SUPPORT */
		COPY_MAC_ADDR(pEntry->Addr, pAddr);
#ifdef PRE_CFG_SUPPORT
		pEntry->fgPreCfgRunning = FALSE;
#endif /* PRE_CFG_SUPPORT */
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
		pEntry->del_reason = MTK_NL80211_VENDOR_DISC_INIT;
#endif
		if (OpMode != OPMODE_ATE) {
			struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
#ifndef RT_CFG80211_SUPPORT
			RTMPInitTimer(pAd, &pSecConfig->StartFor4WayTimer, GET_TIMER_FUNCTION(WPAStartFor4WayExec), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->StartFor2WayTimer, GET_TIMER_FUNCTION(WPAStartFor2WayExec), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->Handshake.MsgRetryTimer, GET_TIMER_FUNCTION(WPAHandshakeMsgRetryExec), pEntry, FALSE);
#endif /* RT_CFG80211_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
			RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
			pSecConfig->PmfCfg.SAQueryConfirmTimes = 0;
#endif /* DOT11W_PMF_SUPPORT */
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
			RTMPInitTimer(pAd, &pEntry->DABSRetryTimer, GET_TIMER_FUNCTION(RTMPDABSretry), pEntry, FALSE);
#endif
#endif
		}
		RTMP_OS_INIT_COMPLETION(&pEntry->WtblSetDone);
		pEntry->Sst = SST_NOT_AUTH;
		pEntry->AuthState = AS_NOT_AUTH;
		pEntry->Aid = entrytb_aid_aquire(pAd, wdev, ent_type, mld_sta_idx);
		if (pEntry->Aid == INVALID_AID) {
			MacTableResetEntry(pAd, pEntry, CleanAll);
			NdisReleaseSpinLock(pAd->MacTabLock);
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				"allocate AID fail!\n");
			HcReleaseUcastWcid(pAd, wdev, i);
			return NULL;
		}
		pEntry->CapabilityInfo = 0;
		pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
		pEntry->PsMode = PWR_ACTIVE;
		pEntry->NoDataIdleCount = 0;
		pEntry->ContinueTxFailCnt = 0;
#ifdef WDS_SUPPORT
		pEntry->LockEntryTx = FALSE;
#endif /* WDS_SUPPORT */
		pEntry->TimeStamp_toTxRing = 0;
		tr_entry->PsMode = PWR_ACTIVE;
		tr_entry->NoDataIdleCount = 0;
		tr_entry->ContinueTxFailCnt = 0;
		tr_entry->LockEntryTx = FALSE;
		tr_entry->TimeStamp_toTxRing = 0;
		tr_entry->PsDeQWaitCnt = 0;
		pEntry->SecConfig.pmkid = NULL;
		pEntry->SecConfig.pmk_cache = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		pEntry->bTxPktChk = FALSE;
		pEntry->TotalTxSuccessCnt = 0;
		pEntry->TxStatRspCnt = 0;
#endif
#ifdef IGMP_SNOOP_SUPPORT
		pEntry->M2U_TxPackets = 0;
		pEntry->M2U_TxBytes = 0;
#endif
#ifdef CONFIG_MAP_SUPPORT
		pEntry->MAP_Enqueue_single = 0;
#endif

		do {
#ifdef CONFIG_STA_SUPPORT
			pEntry->IsRekeyGTK = FALSE;
			pEntry->IsRekeyIGTK = FALSE;
			pEntry->IsRekeyBIGTK = FALSE;
			if (ent_type == ENTRY_INFRA) {
				SET_ENTRY_AP(pEntry);
				COPY_MAC_ADDR(pEntry->bssid, pAddr);

				pStaCfg->MacTabWCID = pEntry->wcid;

				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_AP);/*the peer type related to APCLI is AP role.*/
				pStaCfg->pAssociatedAPEntry = (PVOID)pEntry;

				if ((IS_AKM_OPEN(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap))) {
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				} else {
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
				}

				vow_set_bss_group(pAd, pEntry);
				break;
			}

			if (ent_type == ENTRY_AP || ent_type == ENTRY_ADHOC) {
				if  (ent_type == ENTRY_ADHOC)
					SET_ENTRY_ADHOC(pEntry);

				COPY_MAC_ADDR(pEntry->bssid, pAddr);
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			else if (ent_type == ENTRY_REPEATER) {
				PSTA_ADMIN_CONFIG main_sta = GetStaCfgByWdev(pAd, wdev);
				MAC_TABLE_ENTRY *pRootApEntry = &pAd->MacTab->Content[main_sta->MacTabWCID];

				SET_ENTRY_REPEATER(pEntry);
				COPY_MAC_ADDR(pEntry->bssid, pAddr);
				pEntry->SecConfig.AKMMap = pRootApEntry->SecConfig.AKMMap;
				pEntry->SecConfig.PairwiseCipher = pRootApEntry->SecConfig.PairwiseCipher;
				pEntry->SecConfig.GroupCipher = pRootApEntry->SecConfig.GroupCipher;
				pEntry->pReptCli = NULL;
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_AP);/*the peer type related to Rept is AP role.*/

				if ((IS_AKM_OPEN(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap))) {
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				} else {
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
				}

					/* apcli group idx assignments are from (VOW_MAX_GROUP_NUM-1) reversely
					 * e.g. entry[0] => group idx: VOW_MAX_GROUP_NUM - 1
					 *      entry[1] => group idx: VOW_MAX_GROUP_NUM - 2
					 */
				vow_set_bss_group(pAd, pEntry);

				MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
						 "Repeater Security wcid=%d, AKMMap=0x%x, PairwiseCipher=0x%x, GroupCipher=0x%x\n",
						  pEntry->wcid, pEntry->SecConfig.AKMMap,
						  pEntry->SecConfig.PairwiseCipher, pEntry->SecConfig.GroupCipher);
				break;
			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

			if (ent_type == ENTRY_WDS) {
				SET_ENTRY_WDS(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_WDS);
				COPY_MAC_ADDR(pEntry->bssid, pEntry->wdev->bssid);
				break;
			}

#endif /* WDS_SUPPORT */

			if (ent_type == ENTRY_CLIENT) {
				/* be a regular-entry*/
				if (pAd->ApCfg.EntryClientCount >= GET_MAX_UCAST_NUM(pAd)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
							 " The station number is over MaxUcastEntryNum = %d\n", GET_MAX_UCAST_NUM(pAd));
					aid_clear(&pAd->aid_info, pEntry->Aid);
					MacTableResetEntry(pAd, pEntry, CleanAll);
					NdisReleaseSpinLock(pAd->MacTabLock);
					HcReleaseUcastWcid(pAd, wdev, i);
					return NULL;
				}

				if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
					(pEntry->func_tb_idx < MAX_BEACON_NUM) &&
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum != 0) &&
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount >= pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR," The connection table is full in ra%d.\n", pEntry->func_tb_idx);
					aid_clear(&pAd->aid_info, pEntry->Aid);
					MacTableResetEntry(pAd, pEntry, CleanAll);
					NdisReleaseSpinLock(pAd->MacTabLock);
					HcReleaseUcastWcid(pAd, wdev, i);
					return NULL;
				}

				if ((pAd->ApCfg.BandMaxStaNum != 0) &&
					(pAd->ApCfg.perBandStaCount >= pAd->ApCfg.BandMaxStaNum)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
						"The connection table is full in band%d.\n", wlan_config_get_ch_band(wdev));
					aid_clear(&pAd->aid_info, pEntry->Aid);
					MacTableResetEntry(pAd, pEntry, CleanAll);
					NdisReleaseSpinLock(pAd->MacTabLock);
					HcReleaseUcastWcid(pAd, wdev, i);
					return NULL;
				}

				if (pEntry->func_tb_idx >= MAX_MBSSID_NUM(pAd)) {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
						" The func_tb_idx is over cap = %d\n",  MAX_MBSSID_NUM(pAd));
					aid_clear(&pAd->aid_info, pEntry->Aid);
					MacTableResetEntry(pAd, pEntry, CleanAll);
					NdisReleaseSpinLock(pAd->MacTabLock);
					HcReleaseUcastWcid(pAd, wdev, i);
					return NULL;
				}

				ASSERT((wdev == &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev));
				SET_ENTRY_CLIENT(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_STA);
				mbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
				pEntry->pMbss = mbss;
				COPY_MAC_ADDR(pEntry->bssid, wdev->bssid);

				if (IS_SECURITY_OPEN_WEP(&wdev->SecConfig)) {
					/* OPEN WEP */
					pEntry->SecConfig.AKMMap = wdev->SecConfig.AKMMap;
					pEntry->SecConfig.PairwiseCipher = wdev->SecConfig.PairwiseCipher;
					pEntry->SecConfig.PairwiseKeyId = wdev->SecConfig.PairwiseKeyId;
					pEntry->SecConfig.GroupCipher = wdev->SecConfig.GroupCipher;
					pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
					os_move_mem(pEntry->SecConfig.WepKey, wdev->SecConfig.WepKey, sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
					pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
				}

				if ((IS_AKM_OPEN(wdev->SecConfig.AKMMap))
					|| (IS_SECURITY_OPEN_WEP(&wdev->SecConfig))
					|| (IS_AKM_SHARED(wdev->SecConfig.AKMMap)))
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
				else
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;

				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				/* assign default idle timeout value to bss setting */
				pEntry->StaIdleTimeout = mbss->max_idle_period;
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount++;
				pAd->ApCfg.EntryClientCount++;
#ifdef CCN67_BS_SUPPORT
				pAd->MacTab->ClientCount++;
#endif
				if (pAd->ApCfg.BandMaxStaNum != 0)
					pAd->ApCfg.perBandStaCount++;
#ifdef VOW_SUPPORT

				/* vow_set_client(pAd, pEntry->func_tb_idx, pEntry->wcid); */
				if (VOW_IS_ENABLED(pAd)) {
					if (vow_watf_is_enabled(pAd))
						set_vow_watf_sta_dwrr(pAd, &pEntry->Addr[0], pEntry->wcid);
				}

				if ((pAd->vow_cfg.en_bw_ctrl) || vow_watf_is_enabled(pAd) ||
					(!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG))) {
					RTMP_SET_STA_DWRR(pAd, pEntry);
				}
#else
				if (!(pAd->vow_gen.VOW_FEATURE & VOW_FEATURE_BWCG))
					RTMP_SET_STA_DWRR(pAd, pEntry);
#endif /* VOW_SUPPORT */
#ifdef WH_EVENT_NOTIFIER
				pEntry->tx_state.CurrentState = WHC_STA_STATE_ACTIVE;
				pEntry->rx_state.CurrentState = WHC_STA_STATE_ACTIVE;
#endif /* WH_EVENT_NOTIFIER */
#ifdef ROAMING_ENHANCE_SUPPORT
#ifdef APCLI_SUPPORT
				pEntry->bRoamingRefreshDone = FALSE;
#endif /* APCLI_SUPPORT */
#endif /* ROAMING_ENHANCE_SUPPORT */
				break;
			}
#ifdef AIR_MONITOR
			else if (ent_type == ENTRY_MONITOR) {
				SET_ENTRY_MONITOR(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_STA);
				break;
			}

#endif /* AIR_MONITOR */
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_ATE)
			else if (ent_type == ENTRY_ATE) {
				SET_ENTRY_CLIENT(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_STA);
#if defined(CONFIG_AP_SUPPORT)
				pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
#endif
				break;
			}
#endif
		} while (FALSE);

		TRTableInsertEntry(pAd, pEntry->wcid, pEntry);

		if (get_starec_by_wcid(pAd, i))
			del_starec(pAd, tr_entry);

#ifdef UAPSD_SUPPORT

		/* Ralink WDS doesn't support any power saving.*/
		if (IS_ENTRY_CLIENT(pEntry)) {
			/* init U-APSD enhancement related parameters */
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
				"INIT UAPSD MR ENTRY");
			UAPSD_MR_ENTRY_INIT(pEntry);
		}

#endif /* UAPSD_SUPPORT */

		wdev->client_num++;
		pAd->MacTab->Size++;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WSC_AP_SUPPORT
			pEntry->bWscCapable = FALSE;
			pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
			"alloc entry #%d, Total= %d\n",
			i, pAd->MacTab->Size);
	} else {
		pEntry = NULL;
		tr_entry = NULL;
	}

	/* add this MAC entry into HASH table */
	if (pEntry) {
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);

		if (pAd->MacTab->Hash[HashIdx] == NULL)
			pAd->MacTab->Hash[HashIdx] = pEntry;
		else {
			pCurrEntry = pAd->MacTab->Hash[HashIdx];

			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;

			pCurrEntry->pNext = pEntry;
		}

#ifdef DOT11_EHT_BE
		if (mld_sta_idx != MLD_STA_NONE && mld_addr) {
			pEntry->mlo_join = TRUE;
		}
#endif /* DOT11_EHT_BE */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WSC_AP_SUPPORT

			if (IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
				MAC_ADDR_EQUAL(pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr))
				NdisZeroMemory(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr, MAC_ADDR_LEN);

#endif /* WSC_AP_SUPPORT */
#ifdef MTFWD
			if (IS_ENTRY_CLIENT(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
					"New Sta:"MACSTR"\n", MAC2STR(pEntry->Addr));
				RtmpOSWrielessEventSend(pEntry->wdev->if_dev,
							RT_WLAN_EVENT_CUSTOM,
							FWD_CMD_ADD_TX_SRC,
							NULL,
							(PUCHAR)pEntry->Addr,
							MAC_ADDR_LEN);
			}
#endif

		}
#endif /* CONFIG_AP_SUPPORT */
	}

	NdisReleaseSpinLock(pAd->MacTabLock);
	if (pEntry)
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
			"End for wcid %d =====\n", pEntry->wcid);

	/*update tx burst, must after unlock pAd->MacTabLock*/
	/* rtmp_tx_burst_set(pAd); */

	return pEntry;
}

INT32 MacTableDelEntryFromHash(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY  *pPrevEntry, *pProbeEntry;

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->Addr);
	pPrevEntry = NULL;
	pProbeEntry = pAd->MacTab->Hash[HashIdx];

	if (!pProbeEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
			 "pProbeEntry==NULL,pEntry->wcid=%d,MAC="MACSTR"\n",
			 pEntry->wcid, MAC2STR(pEntry->Addr));
	}

#ifdef DOT11_EHT_BE
	if (pEntry->mlo.mlo_en) {
		struct mld_entry_t *mld_entry = NULL, *new_mld_entry = NULL;
		struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
		u8 link_id = 0;

		spin_lock_bh(&mld_entry_ctrl->mld_entry_lock);
		/*how to get mld_addr*/
		mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);

		if (mld_entry) {
			if (mld_entry->link_num <= 1) {
				remove_link_mld_entry(pEntry);
				hlist_del_rcu(&mld_entry->hlist);
				mt_call_rcu(&mld_entry->rcu, free_mld_entry);
			} else {
				new_mld_entry = create_mld_entry();
				if (new_mld_entry) {
					os_move_mem(new_mld_entry, mld_entry, sizeof(struct mld_entry_t));
					link_id = pEntry->mlo.link_info.link_id;
					new_mld_entry->link_entry[link_id] = NULL;
					new_mld_entry->link_num--;

					remove_link_mld_entry(pEntry);
					update_link_mld_entry(new_mld_entry);
					hlist_replace_rcu(&mld_entry->hlist, &new_mld_entry->hlist);
					mt_call_rcu(&mld_entry->rcu, free_mld_entry);
				} else
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
						" create mld_entry Fail!\n");
			}
		}
		spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);
	}
#endif /* DOT11_EHT_BE */

	/* update Hash list*/
	while (pProbeEntry) {
		if (pProbeEntry == pEntry) {
			if (pPrevEntry == NULL)
				pAd->MacTab->Hash[HashIdx] = pEntry->pNext;
			else
				pPrevEntry->pNext = pEntry->pNext;

			break;
		}

		pPrevEntry = pProbeEntry;
		pProbeEntry = pProbeEntry->pNext;
	};

	if (!pProbeEntry) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
			"Failed to find pProbeEntry\n");
	}

	return TRUE;
}


/*
	==========================================================================
	Description:
		Delete a specified client from MAC table
	==========================================================================
 */
static VOID mac_entry_disconn_act(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG	pStaCfg = NULL;
#endif /* CONFIG_STA_SUPPORT */
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev = pEntry->wdev;

	if (!pEntry || IS_ENTRY_NONE(pEntry))
		return;

	if (!wdev)
		return;

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	twt_resource_release_at_link_down(wdev, pEntry->wcid);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	/* Set port secure to NOT_SECURED here to avoid race condition with ba_ori_session_start */
	tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
	tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

	ba_session_tear_down_all(pAd, pEntry->wcid, TRUE);
	/* RTMP_STA_ENTRY_MAC_RESET--> AsicDelWcidTab() should be integrated to below function*/
	/*in the future */
#ifdef CONFIG_STA_SUPPORT
	pStaCfg = GetStaCfgByWdev(pAd, wdev);
	if (pStaCfg && pStaCfg->pAssociatedAPEntry)
			if (pStaCfg->pAssociatedAPEntry == pEntry) {
				pStaCfg->pAssociatedAPEntry = NULL;
			}

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)

		if (!((pAd->WOW_Cfg.bEnable) &&
			  (pAd->WOW_Cfg.bWowIfDownSupport) &&
			  INFRA_ON(pStaCfg))) {
#endif /* WOW */
			if (wdev_do_disconn_act(pEntry->wdev, pEntry) != TRUE)
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
					" STA disconnection fail!\n");
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
		}

#endif /* WOW */

#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* peer link up needs pEntry type information to decide txop disable or not*/
		/* so invalid pEntry type later */
		if (wdev_do_disconn_act(pEntry->wdev, pEntry) != TRUE)
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				" AP disconnection fail!\n");
	}
#endif /* CONFIG_STA_SUPPORT */
}

uint8_t MacTableDeleteEntry(struct _RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _RTMP_ADAPTER *realpAd;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
	uint16_t mld_sta_idx;
#endif /* DOT11_EHT_BE */

	if (!pAd)
		return FALSE;

	/*check valid ucast wcid*/
	if (!wcid || !(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
		"(caller:%pS): wcid %d =====\n", OS_TRACE, wcid);

	if (MAC_ADDR_EQUAL(pAddr, ZERO_MAC_ADDR)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
		"Del Sta:"MACSTR"???, do nothing here\n", MAC2STR(pAddr));
		return FALSE;
	}

	pEntry = entry_get(pAd, wcid);
#ifdef CONFIG_MAP_SUPPORT
	pEntry->MAP_Enqueue_single = 0;
#endif

	if (pEntry->pAd)
		realpAd = (RTMP_ADAPTER *)pEntry->pAd;
	else
		realpAd = pAd;

	if (!MAC_ADDR_EQUAL(pEntry->Addr, pAddr)) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
			"This entry may be updated again. Stop to delete this entry.\n");
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
			"Del Sta:"MACSTR", Entry.Addr: "MACSTR"\n",
			MAC2STR(pAddr), MAC2STR(pEntry->Addr));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_EHT_BE
	if (!IS_ENTRY_NONE(pEntry)) {
		if (pEntry->mlo.mlo_en) {
			if (pEntry->wdev && pEntry->wdev->wdev_type == WDEV_TYPE_AP) {
				pEntry->mlo_join = FALSE;
				mt_rcu_read_lock();
				mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
				if (mld_entry)
					mld_sta_idx = mld_entry->mld_sta_idx;
				else {
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
						"mld_entry=NULL, fail\n");
					mt_rcu_read_unlock();
					return FALSE;
				}
				mt_rcu_read_unlock();
				/* Inform bss_mgr to delete all mld pEntries */
				bss_mngr_mld_disconn_op(
					pEntry->wdev,
					mld_sta_idx,
					MLD_DISC_OP_DEL_STA_N_ACT);

				return TRUE;
			} else if (pEntry->wdev && pEntry->wdev->wdev_type == WDEV_TYPE_STA) {
				sta_mld_disconn_req(pEntry->wdev);
				return TRUE;
			}
		}
	}
#endif /* DOT11_EHT_BE */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pEntry->IsRekeyGTK = FALSE;
	pEntry->IsRekeyIGTK = FALSE;
	pEntry->IsRekeyBIGTK = FALSE;
#endif /* CONFIG_STA_SUPPORT */

	return _MacTableDeleteEntry(realpAd, wcid, pAddr);
}

uint8_t _MacTableDeleteEntry(struct _RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN Cancelled;
	BOOLEAN	bDeleteEntry = FALSE;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *mbss = NULL;
#endif /*CONFIG_AP_SUPPORT*/
	struct wifi_dev *wdev = NULL;
	ADD_HT_INFO_IE *addht;
	UCHAR i;
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) || defined(WAPP_SUPPORT)
	UCHAR TmpAddrForIndicate[MAC_ADDR_LEN] = {0};
#endif
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
	BOOLEAN bIndicateSendEvent = FALSE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
	struct _RTMP_CHIP_CAP *cap;
#ifdef MTFWD
	PNET_DEV if_dev = NULL;
#endif /* MTFWD */
	if (!pAd)
		return FALSE;

	if (!wcid || !(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return FALSE;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pEntry = entry_get(pAd, wcid);
	tr_entry = tr_entry_get(pAd, wcid);

	pEntry->ht_ie_flag = FALSE;
	pEntry->vht_ie_flag = FALSE;
	pEntry->he_ie_flag = FALSE;
#ifdef PRE_CFG_SUPPORT
	if (pEntry->fgPreCfgRunning)
		return FALSE;
#endif /* PRE_CFG_SUPPORT */

	/*disconnect first*/
	mac_entry_disconn_act(pAd, pEntry);

	NdisAcquireSpinLock(pAd->MacTabLock);

	if (pEntry &&
	    !IS_ENTRY_NONE(pEntry) &&
	    /** #256STA, we would like to recycle the entry idx 0 to use.
	     *  the line should be removed once the the index 0 has been recycled done.
	     */
	    !IS_ENTRY_MCAST(pEntry)) {
#ifdef DABS_QOS
		delete_qos_param_tbl_by_wlan_idx(pAd, pEntry->wcid, pEntry->wdev);
#endif
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
		pEntry->dabs_cfg = FALSE;
		pEntry->dabs_trans_id = 0;
		OS_CLEAR_BIT(DABS_TIMER_RUNNING, &pEntry->DABSTimerFlag);
		RTMPCancelTimer(&pEntry->DABSRetryTimer, &Cancelled);
		RTMPReleaseTimer(&pEntry->DABSRetryTimer, &Cancelled);
#endif
#endif
#ifdef CONFIG_AP_SUPPORT
		mbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
#endif /*CONFIG_AP_SUPPORT*/
#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering) && IS_ENTRY_CLIENT(pEntry) && IS_VALID_MAC(pEntry->Addr))
			BndStrg_UpdateEntry(pAd, pEntry, NULL, FALSE);
#endif
#ifdef MWDS
		MWDSAPPeerDisable(pAd, pEntry);
#endif /* MWDS */
#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
		if (IS_MAP_ENABLE(pAd)) {
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
			if (pEntry->mlo.mlo_en)
				map_a4_mlo_peer_disable(pAd, NULL, pEntry, TRUE);
			else
#endif
				map_a4_peer_disable(pAd, pEntry, TRUE);
		}
#endif
		/*get wdev*/
		wdev = pEntry->wdev;

#ifdef MTFWD
		if (pEntry->wdev)
			if_dev = pEntry->wdev->if_dev;
#ifdef MAC_REPEATER_SUPPORT
		else if ((pEntry->wdev == NULL) && IS_ENTRY_REPEATER(pEntry))
			if_dev = repeater_get_apcli_ifdev(pAd, pEntry);
#endif /* MAC_REPEATER_SUPPORT */
		if (if_dev)
			RtmpOSWrielessEventSend(if_dev,
						RT_WLAN_EVENT_CUSTOM,
						FWD_CMD_DEL_TX_SRC,
						NULL,
						(PUCHAR)pEntry->Addr,
						MAC_ADDR_LEN);
#endif
		pEntry->agg_err_flag = FALSE;
		pEntry->winsize_limit = 0xF;
#ifdef PRE_CFG_SUPPORT
		pEntry->sta_force_keep = FALSE;
#endif /* PRE_CFG_SUPPORT */

		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr)) {

#if defined(CONFIG_AP_SUPPORT) && (defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP))

			if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WNMCtrl.ProxyARPEnable) {
				RemoveIPv4ProxyARPEntry(pAd, mbss, pEntry->Addr, 0);
				RemoveIPv6ProxyARPEntry(pAd, mbss, pEntry->Addr);
			}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
			pEntry->IsKeep = 0;
#endif /* CONFIG_HOTSPOT_R2 */
#endif
			bDeleteEntry = TRUE;
#ifdef CUSTOMISED_MGMT_TRANS_SUPPORT
			CFG80211_APStaDelSendVendorEvent(pAd, (VOID *)pEntry, NULL, 0);
#endif
#ifdef CONFIG_AP_SUPPORT
#ifdef CLIENT_WDS
			if (IS_ENTRY_CLIWDS(pEntry))
				CliWdsEnryFreeAid(pAd, pEntry->wcid);
#endif

			if (IS_ENTRY_CLIENT(pEntry)) {
#ifdef DOT1X_SUPPORT

				/* Notify 802.1x daemon to clear this sta info*/
				if (IS_AKM_1X_Entry(pEntry)
					|| IS_IEEE8021X_Entry(wdev)
#ifdef RADIUS_ACCOUNTING_SUPPORT
					|| IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
#ifdef OCE_FILS_SUPPORT
					|| IS_AKM_FILS_Entry(pEntry)
#endif /* OCE_FILS_SUPPORT */
				   )
					DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
				IgmpGroupDelMembers(pAd, (PUCHAR)pEntry->Addr, wdev, pEntry->wcid);
#endif /* IGMP_SNOOP_SUPPORT */
				if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount > 0)
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount--;
				if (pAd->ApCfg.EntryClientCount > 0)
					pAd->ApCfg.EntryClientCount--;
				if (pAd->ApCfg.perBandStaCount > 0)
					pAd->ApCfg.perBandStaCount--;
#ifdef CCN67_BS_SUPPORT
				if (pAd->MacTab->ClientCount > 0)
					pAd->MacTab->ClientCount--;
#endif
#ifdef MAC_REPEATER_SUPPORT
				if (pEntry->ProxySta) {
					RepeaterDisconnectRootAP(pAd,
								(REPEATER_CLIENT_ENTRY *) pEntry->ProxySta,
								APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA);
					pEntry->ProxySta = NULL;
				}
#endif /* MAC_REPEATER_SUPPORT */
			}

#endif /* CONFIG_AP_SUPPORT */
			MacTableDelEntryFromHash(pAd, pEntry);
#ifdef CONFIG_AP_SUPPORT
			APCleanupPsQueue(pAd, &tr_entry->ps_queue); /* return all NDIS packet in PSQ*/
#endif /* CONFIG_AP_SUPPORT */
			/* clean delay queue */
			APCleanupQueue(pAd, &tr_entry->delay_queue, &tr_entry->delay_queue_lock);
			TRTableResetEntry(pAd, wcid);
#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
			pEntry->pReptCli = NULL;
#endif
#endif /* UAPSD_SUPPORT */
			{
				struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
#ifndef RT_CFG80211_SUPPORT
				RTMPCancelTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
#endif /*RT_CFG80211_SUPPORT*/
#ifdef DOT11W_PMF_SUPPORT
				RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */
#ifdef SW_CONNECT_SUPPORT
#ifdef CONFIG_LINUX_CRYPTO
				if (pSecConfig->tfm) {
					mt_aead_key_free(pSecConfig->tfm);
					pSecConfig->tfm = NULL;
				}
#endif /* CONFIG_LINUX_CRYPTO */
#endif /* SW_CONNECT_SUPPORT */
			}
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT

			if (IS_ENTRY_CLIENT(pEntry)) {
				PWSC_CTRL	pWscControl = &mbss->wdev.WscControl;

				if (MAC_ADDR_EQUAL(pEntry->Addr, pWscControl->EntryAddr)) {
					/*
					Some WPS Client will send dis-assoc close to WSC_DONE.
					If AP misses WSC_DONE, WPS Client still sends dis-assoc to AP.
					Do not cancel timer if WscState is WSC_STATE_WAIT_DONE.
					*/
					if ((pWscControl->EapolTimerRunning == TRUE) &&
						(pWscControl->WscState != WSC_STATE_WAIT_DONE)) {
						RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
						pWscControl->EapolTimerRunning = FALSE;
						pWscControl->EapMsgRunning = FALSE;
						NdisZeroMemory(&(pWscControl->EntryAddr[0]), MAC_ADDR_LEN);
					}
				}

				pEntry->Receive_EapolStart_EapRspId = 0;
				pEntry->bWscCapable = FALSE;
#ifdef WH_EVENT_NOTIFIER
		pEntry->tx_state.CurrentState = WHC_STA_STATE_IDLE;
		pEntry->rx_state.CurrentState = WHC_STA_STATE_IDLE;
#endif /* WH_EVENT_NOTIFIER */
			}

#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) || defined(WAPP_SUPPORT)
			COPY_MAC_ADDR(TmpAddrForIndicate, pEntry->Addr);
#endif
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
			bIndicateSendEvent = TRUE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */

			/* NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY)); */
			NdisZeroMemory(pEntry->Addr, MAC_ADDR_LEN);

			pAd->MacTab->Size--;
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab->Size);
		} else {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN, "\n Impossible Wcid = %d !!!!!\n", wcid);
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
			bIndicateSendEvent = FALSE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
		}

#ifdef CONFIG_AP_SUPPORT
		ApUpdateCapabilityAndErpIe(pAd, mbss);
#endif /* CONFIG_AP_SUPPORT */
	}

	/*update tx burst, must after unlock pAd->MacTabLock*/
	/* rtmp_tx_burst_set(pAd); */

	if (bDeleteEntry) {
		/* Assign again to fix coverity issue (CID: 686248). */
		wdev = pEntry->wdev;

		if (pAd->FragFrame.wcid == wcid) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				"\n Clear Wcid = %d FragBuffer !!!!!\n", wcid);
			RESET_FRAGFRAME(pAd->FragFrame);
		}

		aid_clear(&pAd->aid_info, pEntry->Aid);
#ifdef CONFIG_AP_SUPPORT
		/*
		* move CFG80211_ApStaDelSendEvent here after the entry & hash are deleted ,
		* to prevent removing the same hash twice
		*/
#ifdef RT_CFG80211_SUPPORT


		if (bIndicateSendEvent && pEntry && !IS_ENTRY_NONE(pEntry) && IS_ENTRY_CLIENT(pEntry)) {
			if (wdev && RTMP_CFG80211_HOSTAPD_ON(pAd)
#ifdef DOT11_EHT_BE
				&& !wdev->do_not_send_sta_del_event_flag
#endif
				)
				CFG80211_ApStaDelSendEvent(pAd, TmpAddrForIndicate, wdev->if_dev);
		}

#endif /* RT_CFG80211_SUPPORT */

#ifdef MBO_SUPPORT
		/* mbo - indicate daemon to remve this sta */
		if (wdev && IS_MBO_ENABLE(wdev)) {
			MBO_EVENT_STA_DISASSOC evt_sta_disassoc;
			pEntry->is_mbo_bndstr_sta = 0;
			COPY_MAC_ADDR(evt_sta_disassoc.mac_addr, TmpAddrForIndicate);
			MboIndicateStaDisassocToDaemon(pAd, &evt_sta_disassoc, MBO_MSG_REMOVE_STA);
		}
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
#ifdef CONFIG_MAP_SUPPORT
		if (pAd->disconnect_all_sta != 0) {
/*optimizing events to daemon when all the stations are disconnected from the driver*/
			if (IS_ENTRY_CLIENT(pEntry) && wdev && wdev->if_dev && pAd->disconnect_all_sta == 1) {
				wapp_send_cli_all_leave_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), TmpAddrForIndicate, pEntry);
				pAd->disconnect_all_sta = 2;
			}
		} else {
#endif
			if (IS_ENTRY_CLIENT(pEntry) && wdev && wdev->if_dev)
				wapp_send_cli_leave_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), TmpAddrForIndicate, pEntry);
#ifdef CONFIG_MAP_SUPPORT
		}
#endif
#endif /* WAPP_SUPPORT */

		if (IS_ENTRY_CLIENT(pEntry)
			&& wdev
			&& wdev->wdev_type != WDEV_TYPE_ATE_AP
			&& wdev->wdev_type != WDEV_TYPE_ATE_STA)
			nonerp_sta_num(pEntry, PEER_LEAVE);
#endif /* CONFIG_AP_SUPPORT */
		if (wdev) {
			/* ASSERT(wdev->client_num); */
			if (wdev->client_num)
				wdev->client_num--;
		}

#ifdef R1KH_HARD_RETRY
		RTMP_OS_EXIT_COMPLETION(&pEntry->ack_r1kh);
#endif /* R1KH_HARD_RETRY */
		/* invalidate the entry */
#ifdef DOT11_EHT_BE
		NdisZeroMemory(pEntry->mlo.mld_addr, MAC_ADDR_LEN);
#endif /* DOT11_EHT_BE */
		SET_ENTRY_NONE(pEntry);
		pEntry->EntryDelState = ENTRY_STATE_NONE;
#ifdef GREENAP_SUPPORT
		/* Doing the greeap check after removing the entry */
		greenap_check_peer_connection_at_link_up_down(pAd, wdev);
#endif /* GREENAP_SUPPORT */
	}
#ifdef CONFIG_VLAN_GTK_SUPPORT
	pEntry->vlan_id = 0;
#endif
	NdisReleaseSpinLock(pAd->MacTabLock);

	/* release ucast wcid, avoid doing in spinlock section */
	if (bDeleteEntry)
		HcReleaseUcastWcid(pAd, pEntry->wdev, wcid);

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
		"Del Sta:"MACSTR", Wcid %d del=%d\n", MAC2STR(pAddr), wcid, bDeleteEntry);

	/*Reset operating mode when no Sta.*/
#ifdef DOT11_N_SUPPORT
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (!wdev)
			continue;

		if (!(wdev->if_dev) || (wdev->if_up_down_state == FALSE))
			continue;

		if (wdev->wdev_type != WDEV_TYPE_AP
			|| wdev->client_num)
			continue;

		addht = wlan_operate_get_addht(wdev);
		addht->AddHtInfo2.OperaionMode = 0;
	}
#endif /* DOT11_N_SUPPORT */

#ifdef A4_CONN
	pEntry->roaming_entry = FALSE;
#endif
	return TRUE;
}


/*
	==========================================================================
	Description:
		This routine reset the entire MAC table. All packets pending in
		the power-saving queues are freed here.
	==========================================================================
 */
VOID MacTableResetWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UINT16 i;
#ifdef CONFIG_AP_SUPPORT
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	struct _BSS_STRUCT *mbss;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "MacTableResetWdev\n");

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = entry_get(pAd, i);

		if (pMacEntry->wdev != wdev)
			continue;

		if (IS_ENTRY_CLIENT(pMacEntry)) {
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*	send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, " MlmeAllocateMemory fail  ..\n");
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN,
						"Send DeAuth (Reason=%d) to "MACSTR"\n", Reason, MAC2STR(pMacEntry->Addr));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 wdev->if_addr,
									 wdev->bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (wdev->wdev_type == WDEV_TYPE_AP) {
			mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
#ifdef WSC_AP_SUPPORT
			{
				BOOLEAN Cancelled;

				RTMPCancelTimer(&mbss->wdev.WscControl.EapolTimer, &Cancelled);
				mbss->wdev.WscControl.EapolTimerRunning = FALSE;
				NdisZeroMemory(mbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
				mbss->wdev.WscControl.EapMsgRunning = FALSE;
			}
#endif /* WSC_AP_SUPPORT */
			mbss->StaCount = 0;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}


VOID MacTableReset(RTMP_ADAPTER *pAd)
{
	UINT16 i;
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags = 0;
#endif /* RTMP_MAC_PCI */
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	UCHAR apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;
	/*
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];
	    MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	NdisZeroMemory(Hash, sizeof(Hash));
	NdisZeroMemory(Content, sizeof(Content));
	*/
	MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "MacTableReset\n");
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = entry_get(pAd, i);

		if (IS_ENTRY_CLIENT(pMacEntry)) {
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*  send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, " MlmeAllocateMemory fail  ..\n");
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
#ifdef WIFI_DIAG
					if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
						diag_conn_error(pAd, pMacEntry->func_tb_idx, pMacEntry->Addr,
							DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
					if (IS_ENTRY_CLIENT(pMacEntry))
						ApSendConnFailMsg(pAd,
							pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].Ssid,
							pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].SsidLen,
							pMacEntry->Addr,
							Reason);
#endif
#ifdef MAP_R2
					if (IS_ENTRY_CLIENT(pMacEntry) && IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
						wapp_handle_sta_disassoc(pAd, pMacEntry->wcid, Reason);
#endif
					MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN,
						"Send DeAuth (Reason=%d) to "MACSTR"\n", Reason, MAC2STR(pMacEntry->Addr));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.if_addr,
									 pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MAC_TABLE_ENTRY **Hash = NULL;
		MAC_TABLE_ENTRY *Content = NULL;

		os_alloc_mem(NULL, (UCHAR **)&Hash, sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		if (!Hash) {
			ASSERT(0);
			return;/*ALPS05330059*/
		}
		os_alloc_mem(NULL, (UCHAR **)&Content, sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		if (!Content) {
			ASSERT(0);
			os_free_mem(Hash);
			return;/*ALPS05330298*/
		}
		if (!IS_WCID_VALID(pAd, GET_MAX_UCAST_NUM(pAd))) {
			ASSERT(0);
			os_free_mem(Hash);
			os_free_mem(Content);
			return;
		}

		NdisZeroMemory(&Hash[0], sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisZeroMemory(&Content[0], sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));

		/* MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE]; */

		for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++) {
#ifdef WSC_AP_SUPPORT
			BOOLEAN Cancelled;

			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapolTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
			pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapMsgRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
			pAd->ApCfg.MBSSID[apidx].StaCount = 0;
		}

		os_zero_mem(&(pAd->bss_group), sizeof(struct bss_group_rec));
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_LOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), IrqFlags);
#endif /* RTMP_MAC_PCI */
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "McastPsQueue.Number %d...\n", pAd->MacTab->McastPsQueue.Number);

		if (pAd->MacTab->McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab->McastPsQueue);

		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "2McastPsQueue.Number %d...\n", pAd->MacTab->McastPsQueue.Number);
		/* ENTRY PREEMPTION: Zero Mac Table but entry's content */
		NdisCopyMemory(&Hash[0], pAd->MacTab->Hash, sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisCopyMemory(&Content[0], pAd->MacTab->Content, sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		NdisZeroMemory(pAd->MacTab, sizeof(MAC_TABLE));
		NdisCopyMemory(pAd->MacTab->Hash, &Hash[0], sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisCopyMemory(pAd->MacTab->Content, &Content[0], sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		os_free_mem(Hash);
		os_free_mem(Content);
		InitializeQueueHeader(&pAd->MacTab->McastPsQueue);
		/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_UNLOCK(&PD_GET_DEVICE_IRQ(pAd->physical_dev), IrqFlags);
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}

static VOID SetHtVhtForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef DOT11_N_SUPPORT
	SetCommonHT(pAd, wdev);
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode))
		SetCommonVHT(pAd, wdev);

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
}

INT	SetCommonHtVht(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (wdev) {
		if (wdev->DevInfo.WdevActive)
			SetHtVhtForWdev(pAd, wdev);
	} else
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR, "Can't update HT/VHT due to wdev is null!\n");

	return TRUE;
}

inline SCAN_CTRL *get_scan_ctrl_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	return &pAd->ScanCtrl;
}

inline PBSS_TABLE get_scan_tab_by_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	SCAN_CTRL *ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);

	return &ScanCtrl->ScanTab;
}

#ifdef CONFIG_STA_SUPPORT

inline PSTA_ADMIN_CONFIG GetStaCfgByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	if (pwdev && pwdev->func_dev) {
		if (pwdev->wdev_type == WDEV_TYPE_STA) {
			return (struct _STA_ADMIN_CONFIG *)pwdev->func_dev;
		}
#ifdef MAC_REPEATER_SUPPORT
		else if (pwdev->wdev_type == WDEV_TYPE_REPEATER) {
			REPEATER_CLIENT_ENTRY *rept_cli = (struct _REPEATER_CLIENT_ENTRY *)pwdev->func_dev;
			return (struct _STA_ADMIN_CONFIG *)rept_cli->main_wdev->func_dev;
		}
#endif /* MAC_REPEATER_SUPPORT */
	}
	return NULL;
}



MAC_TABLE_ENTRY *GetAssociatedAPByWdev(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	PSTA_ADMIN_CONFIG pStafCfg;

	if (pwdev == NULL)
		return NULL;

	pStafCfg = GetStaCfgByWdev(pAd, pwdev);

	if (pStafCfg == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN,
			"Avoid calling this function when wdev type is not client!\n");
		return NULL;
	}

	return (MAC_TABLE_ENTRY *)pStafCfg->pAssociatedAPEntry;
}

VOID sta_mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab->Hash[HashIdx];

	if (wdev) {
		while (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr) && (pEntry->wdev == wdev))
				break;
			else
				pEntry = pEntry->pNext;
		}
	} else {
		while (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
				break;
			else
				pEntry = pEntry->pNext;
		}
	}

	*entry = pEntry;
}
#endif

VOID mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab->Hash[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry)) {
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	*entry = pEntry;
}

/*
* Delete MacTableEntry and equeue to cmd thread
*/
VOID mac_entry_delete(
	struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry, u8 rm_now)
{
	UCHAR buf[32] = {0};

	NdisAcquireSpinLock(ad->MacTabLock);
	if (entry->EntryDelState == ENTRY_STATE_SYNC) {
		MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
		"entry in del state, no need to add job\n");
		NdisReleaseSpinLock(ad->MacTabLock);
		return;
	}
	*((UINT16 *)buf) = entry->wcid;
	memcpy(&buf[2], entry->Addr, 6);
	MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_NOTICE,
		"(caller:%pS): wcid %d ==("MACSTR") del State=%d\n", OS_TRACE,
		entry->wcid, MAC2STR(entry->Addr), entry->EntryDelState);
	physical_device_workq_add_job(
		ad, NULL, MAIN_WORKQ_DEL_MAC_ENTRY_TAG, buf, sizeof(buf), rm_now);
	entry->EntryDelState = ENTRY_STATE_SYNC;
	NdisReleaseSpinLock(ad->MacTabLock);
}

void entrytb_aid_bitmap_init(struct _aid_info *aid_info)
{
	UINT32 *aid_bitmap = NULL;

	UINT32 map_size = 0;

	/*allocate up to can contain MAX_VALID_AID*/
	map_size = ((INVALID_AID + 31) / 32) * sizeof(UINT32);
	os_alloc_mem(NULL, (UCHAR **)&aid_bitmap, map_size);
	if (aid_bitmap == NULL) {
		dump_stack();
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				 "Allocate memory size:%d for aid_bitmap failed!\n",
				  map_size);
		return;
	}

	aid_info->aid_bitmap = aid_bitmap;
	if (aid_info->aid_bitmap == NULL) {
		dump_stack();
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				 "Allocate memory size:%d for aid_info->aid_bitmap failed!\n",
				  map_size);
		return;
	}

	os_zero_mem(aid_bitmap, map_size);
	aid_info->max_aid = INVALID_AID - 1;
}

void entrytb_aid_bitmap_free(struct _aid_info *aid_info)
{
	UINT32 *aid_bitmap = aid_info->aid_bitmap;

	if (aid_bitmap)
		os_free_mem(aid_bitmap);

	aid_info->aid_bitmap = NULL;
}

void entrytb_aid_bitmap_reserve(
	struct _RTMP_ADAPTER *ad,
	struct _aid_info *aid_info)
{
	/*aid bitmap needs to consider the amounts of the non-transmitted bss of 11V mbss*/
#ifdef DOT11V_MBSSID_SUPPORT
	UINT32 i;
	BOOLEAN b11v_on = FALSE;

	/* check 11v on */
	for (i = 0; i < MAX_TX_BSS_CNT; i++) {
		if (ad->ApCfg.dot11v_BssidNum[i]) {
			b11v_on = TRUE;
			break;
		}
	}

	/* IEEE Std 802.11v-2011 (7.3.2.6 TIM):
	 * k is the number of actually supported non-transmitted BSSIDs.
	 * The bits 1 to k of the bitmap are used to indicate
	 * that one or more group addressed frames are buffered
	 * for each AP corresponding to a non-transmitted BSSID.
	 * The AIDs from 1 to k are not allocated to a STA.
	 * The AIDs from (k + 1) to (2n ¡V 1) are reserved and set to 0.
	 * * The AIDs from (k + 1) to (2n ¡V 1) are reserved and set to 0.
	 * The remaining AIDs are shared
	 * by the BSSs corresponding to the transmitted BSSID and all non-transmitted BSSIDs.
	 */
	/* Reserve max 11v bssids number. */
	if  (b11v_on == TRUE)
		aid_info->aid_allocate_from_idx = (1 << mbss_11v_bssid_num_to_max_indicator(ad->ApCfg.BssidNum));
	else
#endif
		aid_info->aid_allocate_from_idx = (1 << 0);
}

UINT16 _entrytb_aid_aquire(struct _aid_info *aid_info)
{
	UINT16 aid;

	for (aid = aid_info->aid_allocate_from_idx; aid <= aid_info->max_aid; aid++) {
		if (!aid_is_assigned(aid_info, aid)) {
			aid_set(aid_info, aid);
			MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
					"found non-occupied aid:%d, allocated from:%d\n",
					aid, aid_info->aid_allocate_from_idx);
			break;
		}
	}

	if (aid > aid_info->max_aid)
		aid = INVALID_AID;

	return aid;
}

uint16_t entrytb_aid_aquire(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, uint32_t ent_type, uint16_t mld_sta_idx)
{
	uint16_t aid = 0;

	if (ent_type == ENTRY_CLIENT) {
#ifdef DOT11_EHT_BE
		if (mld_sta_idx != MLD_STA_NONE)
			aid = bss_mngr_query_mld_sta_aid(wdev, mld_sta_idx);
		else
#endif /* DOT11_EHT_BE */
			aid = _entrytb_aid_aquire(&pAd->aid_info);
	} else
		aid = _entrytb_aid_aquire(&pAd->aid_info);

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
		"ent_type=%d,mld_sta_idx=%d,aid=%d\n",
		ent_type, mld_sta_idx, aid);

	return aid;
}

INT show_aid_info(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	struct _aid_info *aid_info = &ad->aid_info;
	UINT32 aid;

	if (arg && strlen(arg)) {
		aid = simple_strtol(arg, NULL, 10);
		if ((aid == 0) || (aid >= INVALID_AID)) {
			MTWF_DBG(ad, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
				"wrong AID input\n");
			return FALSE;
		}
	}

	aid_dump(aid_info);

	return TRUE;
}

UINT32 traversal_func_find_entry_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	entrytb_aid_search_t *aid_map = (entrytb_aid_search_t *)cookie;

	if (aid_map->aid_search == entry->Aid) {
		aid_map->entry = entry;
		result = TRUE;
	}

	return result;
}

UINT32 traversal_func_dump_entry_associated_to_bss(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)entry->pAd;
	entrytb_bss_idx_search_t *check_bss = (entrytb_bss_idx_search_t *)cookie;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	UCHAR	tmp_str[30];
	INT	temp_str_len = sizeof(tmp_str);
	int ret;
	UINT tmp_str_left = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#endif

	if (check_bss->need_print_field_name) {
		MTWF_PRINT("Dump the entries associated to BssIDX:%d\n", check_bss->bss_idx);
#ifdef CONFIG_HOTSPOT_R2
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd", "BW", "MCS", "SGI",
			"STBC", "Rate");
#else
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd(T/R)", "BW(T/R)", "MCS(T/R)", "SGI(T/R)",
			"STBC(T/R)", "Rate(T/R)");
#endif /* CONFIG_HOTSPOT_R2 */
		check_bss->need_print_field_name = 0;
	}

	if (check_bss->bss_idx == entry->func_tb_idx) {
		DataRate = 0;
		getRate(entry->HTPhyMode, &DataRate);
		MTWF_PRINT(MACSTR, MAC2STR(entry->Addr));
		MTWF_PRINT("%-7d", (int)entry->wcid);
		MTWF_PRINT("%-4d", (int)entry->func_tb_idx);
		MTWF_PRINT("%-4d", (int)entry->PsMode);
		ret = snprintf(tmp_str, temp_str_len, "%d/%d/%d/%d", entry->RssiSample.AvgRssi[0],
				entry->RssiSample.AvgRssi[1],
				entry->RssiSample.AvgRssi[2],
				entry->RssiSample.AvgRssi[3]);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-20s", tmp_str);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (cap->fgRateAdaptFWOffload == TRUE && (entry->bAutoTxRateSwitch == TRUE)) {
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
			UCHAR vht_nss;
			UCHAR vht_nss_r;
#endif
			UINT32 RawData;
			UINT32 RawData_r;
			UINT32 lastTxRate;
			UINT32 lastRxRate = entry->LastRxRate;

			if (entry->bAutoTxRateSwitch == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				union _HTTRANSMIT_SETTING LastTxRate;
				union _HTTRANSMIT_SETTING LastRxRate;

				os_zero_mem(&rTxStatResult, sizeof(rTxStatResult));
				MtCmdGetTxStatistic(ad,
						    GET_TX_STAT_ENTRY_TX_RATE,
						    0/*Don't Care*/,
						    entry->wcid,
						    &rTxStatResult);
				LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
				LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
				LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

				if (LastTxRate.field.MODE >= MODE_VHT)
					LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) +
								rTxStatResult.rEntryTxRate.MCS;
				else if (LastTxRate.field.MODE == MODE_OFDM)
					LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) &
								0x0000003F;
				else
					LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

				lastTxRate = (UINT32)(LastTxRate.word);
				LastRxRate.word = (USHORT)lastRxRate;
				RawData = lastTxRate;
				phy_mode = (RawData >> 13) & 0x7;
				rate = RawData & 0x3F;
				bw = (RawData >> 7) & 0x3;
				sgi = (RawData >> 9) & 0x1;
				stbc = ((RawData >> 10) & 0x1);
				/* ---- */
				RawData_r = lastRxRate;
				phy_mode_r = (RawData_r >> 13) & 0x7;
				rate_r = RawData_r & 0x3F;
				bw_r = (RawData_r >> 7) & 0x3;
				sgi_r = (RawData_r >> 9) & 0x1;
				stbc_r = ((RawData_r >> 10) & 0x1);
				ret = snprintf(tmp_str,
					 temp_str_len,
					 "%s/%s",
					 get_phymode_str(phy_mode),
					 get_phymode_str(phy_mode_r));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%s/%s",
					get_bw_str(bw, BW_FROM_TXRX_INFO), get_bw_str(bw_r, BW_FROM_TXRX_INFO));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
#ifdef DOT11_VHT_AC

				if (phy_mode >= MODE_VHT) {
					vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
					rate = rate & 0xF;
					ret = snprintf(tmp_str, temp_str_len, "%dS-M%d/", vht_nss, rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
				{
					ret = snprintf(tmp_str, temp_str_len, "%d/", rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}
#ifdef DOT11_VHT_AC

				if (phy_mode_r >= MODE_VHT) {
					vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
					rate_r = rate_r & 0xF;
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str), tmp_str_left,
						"%dS-M%d",
						vht_nss_r,
						rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
				if (phy_mode_r >= MODE_HTMIX) {
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}
				else
#endif
				if (phy_mode_r == MODE_OFDM) {
					if (rate_r == TMI_TX_RATE_OFDM_6M)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_OFDM_9M)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_OFDM_12M)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_OFDM_18M)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_OFDM_24M)
						LastRxRate.field.MCS = 4;
					else if (rate_r == TMI_TX_RATE_OFDM_36M)
						LastRxRate.field.MCS = 5;
					else if (rate_r == TMI_TX_RATE_OFDM_48M)
						LastRxRate.field.MCS = 6;
					else if (rate_r == TMI_TX_RATE_OFDM_54M)
						LastRxRate.field.MCS = 7;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else if (phy_mode_r == MODE_CCK) {
					if (rate_r == TMI_TX_RATE_CCK_1M_LP)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
						LastRxRate.field.MCS = 3;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}

				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d", sgi, sgi_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d",  stbc, stbc_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-10s", tmp_str);
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
			}
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		{
			MTWF_PRINT("%-12s", get_phymode_str(entry->HTPhyMode.field.MODE));
			MTWF_PRINT("%-9s", get_bw_str(entry->HTPhyMode.field.BW, BW_FROM_OID));
#ifdef DOT11_VHT_AC

			if (entry->HTPhyMode.field.MODE >= MODE_VHT) {
				ret = snprintf(tmp_str, temp_str_len, "%dS-M%d", ((entry->HTPhyMode.field.MCS >> 4) + 1),
						(entry->HTPhyMode.field.MCS & 0xf));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			} else
#endif /* DOT11_VHT_AC */
			{
				ret = snprintf(tmp_str, temp_str_len, "%d", entry->HTPhyMode.field.MCS);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			}

			MTWF_PRINT("%-12s", tmp_str);
			MTWF_PRINT("%-9d", entry->HTPhyMode.field.ShortGI);
			MTWF_PRINT("%-10d", entry->HTPhyMode.field.STBC);
		}

		ret = snprintf(tmp_str, temp_str_len, "%d/%d", (int)DataRate, (int)DataRate_r);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-10s", tmp_str);
		MTWF_PRINT("\n");
	}

	return result;
}

UINT32 traversal_func_dump_entry_psm_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)entry->pAd;
	UINT32 *aid = (UINT32 *)cookie;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ad->hdev_ctrl);
	ULONG now;

	NdisGetSystemUpTime(&now);

	if (*aid == entry->Aid) {
		MTWF_PRINT("dump PSM info for AID:%d\n", entry->Aid);
		MTWF_PRINT("\n%-19s\t%s\t%s\t%s\t%-9s\t%s\n",
			"MAC", "WCID", "BSS", "PSM", "NoRxData", "SLEEP TIME(msec)");

		MTWF_PRINT(MACSTR, MAC2STR(entry->Addr));
		MTWF_PRINT("\t%d", (int)entry->wcid);
		MTWF_PRINT("\t%d", (int)entry->func_tb_idx);
		MTWF_PRINT("\t%d", (int)entry->PsMode);
		MTWF_PRINT("\t%-9d", (int)entry->NoDataIdleCount);
		MTWF_PRINT("\t%d\n", entry->PsMode ? jiffies_to_msecs(now - entry->sleep_from) : 0);

		if (chip_dbg->show_ple_info_by_idx)
			chip_dbg->show_ple_info_by_idx(ad->hdev_ctrl, entry->wcid);

		result = TRUE;
	}

	return result;

}

UINT32 traversal_func_dump_entry_rate_by_aid(struct _MAC_TABLE_ENTRY *entry, void *cookie)
{
	UINT32 result = FALSE;
	UINT32 *aid = (UINT32 *)cookie;
	ULONG DataRate = 0;
	ULONG DataRate_r = 0;
	UCHAR	tmp_str[30];
	INT		temp_str_len = sizeof(tmp_str);
	int ret;
	UINT tmp_str_left = 0;

	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)entry->pAd;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
#endif

	if (*aid == entry->Aid) {
		MTWF_PRINT("dump rate info for AID:%d\n", entry->Aid);
#ifdef CONFIG_HOTSPOT_R2
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd", "BW", "MCS", "SGI",
			"STBC", "Rate");
#else
		MTWF_PRINT("\n%-19s%-7s%-4s%-4s%-20s%-12s%-9s%-12s%-9s%-10s%-10s\n",
			"MAC", "WCID", "BSS", "PSM",
			"RSSI0/1/2/3", "PhMd(T/R)", "BW(T/R)", "MCS(T/R)", "SGI(T/R)",
			"STBC(T/R)", "Rate(T/R)");
#endif /* CONFIG_HOTSPOT_R2 */

		DataRate = 0;
		getRate(entry->HTPhyMode, &DataRate);
		MTWF_PRINT(MACSTR, MAC2STR(entry->Addr));
		MTWF_PRINT("%-7d", (int)entry->wcid);
		MTWF_PRINT("%-4d", (int)entry->func_tb_idx);
		MTWF_PRINT("%-4d", (int)entry->PsMode);
		ret = snprintf(tmp_str, temp_str_len, "%d/%d/%d/%d", entry->RssiSample.AvgRssi[0],
				entry->RssiSample.AvgRssi[1],
				entry->RssiSample.AvgRssi[2],
				entry->RssiSample.AvgRssi[3]);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-20s", tmp_str);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		if (cap->fgRateAdaptFWOffload == TRUE && (entry->bAutoTxRateSwitch == TRUE)) {
			UCHAR phy_mode, rate, bw, sgi, stbc;
			UCHAR phy_mode_r, rate_r, bw_r, sgi_r, stbc_r;
#ifdef DOT11_VHT_AC
			UCHAR vht_nss;
			UCHAR vht_nss_r;
#endif
			UINT32 RawData;
			UINT32 RawData_r;
			UINT32 lastTxRate;
			UINT32 lastRxRate = entry->LastRxRate;

			if (entry->bAutoTxRateSwitch == TRUE) {
				EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
				union _HTTRANSMIT_SETTING LastTxRate;
				union _HTTRANSMIT_SETTING LastRxRate;

				os_zero_mem(&rTxStatResult, sizeof(rTxStatResult));
				MtCmdGetTxStatistic(ad,
						    GET_TX_STAT_ENTRY_TX_RATE,
						    0/*Don't Care*/,
						    entry->wcid,
						    &rTxStatResult);
				LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
				LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
				LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
				LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
				LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

				if (LastTxRate.field.MODE >= MODE_VHT)
					LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) +
								rTxStatResult.rEntryTxRate.MCS;
				else if (LastTxRate.field.MODE == MODE_OFDM)
					LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) &
								0x0000003F;
				else
					LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

				lastTxRate = (UINT32)(LastTxRate.word);
				LastRxRate.word = (USHORT)lastRxRate;
				RawData = lastTxRate;
				phy_mode = (RawData >> 13) & 0x7;
				rate = RawData & 0x3F;
				bw = (RawData >> 7) & 0x3;
				sgi = (RawData >> 9) & 0x1;
				stbc = ((RawData >> 10) & 0x1);
				/* ---- */
				RawData_r = lastRxRate;
				phy_mode_r = (RawData_r >> 13) & 0x7;
				rate_r = RawData_r & 0x3F;
				bw_r = (RawData_r >> 7) & 0x3;
				sgi_r = (RawData_r >> 9) & 0x1;
				stbc_r = ((RawData_r >> 10) & 0x1);
				ret = snprintf(tmp_str,
					 temp_str_len,
					 "%s/%s",
					 get_phymode_str(phy_mode),
					 get_phymode_str(phy_mode_r));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%s/%s", get_bw_str(bw, BW_FROM_TXRX_INFO), get_bw_str(bw_r, BW_FROM_TXRX_INFO));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
#ifdef DOT11_VHT_AC

				if (phy_mode >= MODE_VHT) {
					vht_nss = ((rate & (0x3 << 4)) >> 4) + 1;
					rate = rate & 0xF;
					ret = snprintf(tmp_str, temp_str_len, "%dS-M%d/", vht_nss, rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
				{
					ret = snprintf(tmp_str, temp_str_len, "%d/", rate);
					if (os_snprintf_error(temp_str_len, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}
#ifdef DOT11_VHT_AC

				if (phy_mode_r >= MODE_VHT) {
					vht_nss_r = ((rate_r & (0x3 << 4)) >> 4) + 1;
					rate_r = rate_r & 0xF;
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						tmp_str_left,
						"%dS-M%d",
						vht_nss_r,
						rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif /* DOT11_VHT_AC */
#if DOT11_N_SUPPORT
				if (phy_mode_r >= MODE_HTMIX) {
					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 rate_r);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else
#endif
				if (phy_mode_r == MODE_OFDM) {
					if (rate_r == TMI_TX_RATE_OFDM_6M)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_OFDM_9M)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_OFDM_12M)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_OFDM_18M)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_OFDM_24M)
						LastRxRate.field.MCS = 4;
					else if (rate_r == TMI_TX_RATE_OFDM_36M)
						LastRxRate.field.MCS = 5;
					else if (rate_r == TMI_TX_RATE_OFDM_48M)
						LastRxRate.field.MCS = 6;
					else if (rate_r == TMI_TX_RATE_OFDM_54M)
						LastRxRate.field.MCS = 7;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				} else if (phy_mode_r == MODE_CCK) {
					if (rate_r == TMI_TX_RATE_CCK_1M_LP)
						LastRxRate.field.MCS = 0;
					else if (rate_r == TMI_TX_RATE_CCK_2M_LP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_LP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_LP)
						LastRxRate.field.MCS = 3;
					else if (rate_r == TMI_TX_RATE_CCK_2M_SP)
						LastRxRate.field.MCS = 1;
					else if (rate_r == TMI_TX_RATE_CCK_5M_SP)
						LastRxRate.field.MCS = 2;
					else if (rate_r == TMI_TX_RATE_CCK_11M_SP)
						LastRxRate.field.MCS = 3;
					else
						LastRxRate.field.MCS = 0;

					tmp_str_left = temp_str_len - strlen(tmp_str);
					ret = snprintf(tmp_str + strlen(tmp_str),
						 tmp_str_left,
						 "%d",
						 LastRxRate.field.MCS);
					if (os_snprintf_error(tmp_str_left, ret)) {
						MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
						return FALSE;
					}
				}

				MTWF_PRINT("%-12s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d", sgi, sgi_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-9s", tmp_str);
				ret = snprintf(tmp_str, temp_str_len, "%d/%d",  stbc, stbc_r);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
				MTWF_PRINT("%-10s", tmp_str);
				getRate(LastTxRate, &DataRate);
				getRate(LastRxRate, &DataRate_r);
			}
		} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
		{
			MTWF_PRINT("%-12s", get_phymode_str(entry->HTPhyMode.field.MODE));
			MTWF_PRINT("%-9s", get_bw_str(entry->HTPhyMode.field.BW, BW_FROM_OID));
#ifdef DOT11_VHT_AC

			if (entry->HTPhyMode.field.MODE >= MODE_VHT) {
				ret = snprintf(tmp_str, temp_str_len, "%dS-M%d", ((entry->HTPhyMode.field.MCS >> 4) + 1),
						(entry->HTPhyMode.field.MCS & 0xf));
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			} else
#endif /* DOT11_VHT_AC */
			{
				ret = snprintf(tmp_str, temp_str_len, "%d", entry->HTPhyMode.field.MCS);
				if (os_snprintf_error(temp_str_len, ret)) {
					MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
					return FALSE;
				}
			}

			MTWF_PRINT("%-12s", tmp_str);
			MTWF_PRINT("%-9d", entry->HTPhyMode.field.ShortGI);
			MTWF_PRINT("%-10d", entry->HTPhyMode.field.STBC);
		}

		ret = snprintf(tmp_str, temp_str_len, "%d/%d", (int)DataRate, (int)DataRate_r);
		if (os_snprintf_error(temp_str_len, ret)) {
			MTWF_PRINT("[%d]os_snprintf fail!\n", __LINE__);
			return FALSE;
		}
		MTWF_PRINT("%-10s", tmp_str);
		MTWF_PRINT("\n");

		result = TRUE;
	}

	return result;
}

UINT32 entrytb_traversal(struct _RTMP_ADAPTER *ad, entrytb_traversal_func func, void *cookie)
{
	UINT16 i;
	UINT32 result = FALSE;

	MAC_TABLE_ENTRY *entry = NULL;

	for (i = 0; VALID_UCAST_ENTRY_WCID(ad, i); i++) {
		entry = entry_get(ad, i);

		if (!IS_ENTRY_CLIENT(entry))
			continue;

		result = func(entry, cookie);

		/**
		 * for some specific purpose traversal func,
		 * found the matched parameter and done the corresponding process.
		 * skip the remain entries here.
		 */
		if (result == TRUE)
			break;
	}

	return result;
}

void seq_ctrl_init(struct seq_ctrl_t *seq_ctrl)
{
	INT tid, up, cat_id;

	seq_ctrl->NonQosDataSeq = 0;

	for (tid = 0; tid < NUM_OF_TID; tid++) {
		seq_ctrl->TxSeq[tid] = 0;
#ifdef SW_CONNECT_SUPPORT
		seq_ctrl->RxSeq[tid] = 0;
#endif /* SW_CONNECT_SUPPORT */
	}

	for (up = 0; up < NUM_OF_UP; up++) {
		seq_ctrl->cacheSn[up] = -1;
		seq_ctrl->previous_sn[up] = -1;
		seq_ctrl->previous_amsdu_state[up] = MSDU_FORMAT;
	}

	for (cat_id = 0; cat_id < NUM_OF_MGMT_SN_CAT; cat_id++)
		seq_ctrl->cacheMgmtSn[cat_id] = -1;
}

#ifdef DOT11_EHT_BE
static int entry_sync_apps(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		NdisMoveMemory(dst_entry->bAPSDCapablePerAC, src_entry->bAPSDCapablePerAC, sizeof(src_entry->bAPSDCapablePerAC));
		NdisMoveMemory(dst_entry->bAPSDDeliverEnabledPerAC, src_entry->bAPSDDeliverEnabledPerAC, sizeof(src_entry->bAPSDDeliverEnabledPerAC));
		dst_entry->MaxSPLength = src_entry->MaxSPLength;
	}

	return TRUE;
}

static int entry_sync_wmm(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;
	struct wifi_dev *src_wdev;
	struct wifi_dev *dst_wdev;
	struct _RTMP_ADAPTER *pAd_src = NULL;
	struct _RTMP_ADAPTER *pAd_dst = NULL;
	PSTA_ADMIN_CONFIG src_pStaCfg = NULL;
	PSTA_ADMIN_CONFIG dst_pStaCfg = NULL;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		if (CLIENT_STATUS_TEST_FLAG(src_entry, fCLIENT_STATUS_WMM_CAPABLE))
			CLIENT_STATUS_SET_FLAG(dst_entry, fCLIENT_STATUS_WMM_CAPABLE);
		else
			CLIENT_STATUS_CLEAR_FLAG(dst_entry, fCLIENT_STATUS_WMM_CAPABLE);

		/* copy QOS related information for apcli*/
		src_wdev = src_entry->wdev;
		dst_wdev = dst_entry->wdev;
		if (src_wdev && dst_wdev && (src_wdev->wdev_type == WDEV_TYPE_STA) && (dst_wdev->wdev_type == WDEV_TYPE_STA)) {
			pAd_src = (struct _RTMP_ADAPTER *)(src_entry->wdev->sys_handle);
			pAd_dst = (struct _RTMP_ADAPTER *)(dst_entry->wdev->sys_handle);
			src_pStaCfg = GetStaCfgByWdev(pAd_src, src_wdev);
			dst_pStaCfg = GetStaCfgByWdev(pAd_dst, dst_wdev);
			NdisMoveMemory(&dst_pStaCfg->MlmeAux.APEdcaParm, &src_pStaCfg->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,wmm=%d),dst(wcid=%d,wmm=%d)\n",
			i,
			src_entry->wcid,
			CLIENT_STATUS_TEST_FLAG(src_entry, fCLIENT_STATUS_WMM_CAPABLE) ? 1 : 0,
			dst_entry->wcid,
			CLIENT_STATUS_TEST_FLAG(dst_entry, fCLIENT_STATUS_WMM_CAPABLE) ? 1 : 0);
	}

	return TRUE;
}

static int entry_sync_sst(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		dst_entry->Sst = src_entry->Sst;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,Sst=%d),dst(wcid=%d,Sst=%d)\n",
			i,
			src_entry->wcid, src_entry->Sst,
			dst_entry->wcid, dst_entry->Sst);
	}

	return TRUE;
}

static int entry_sync_af(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		dst_entry->MaxRAmpduFactor = src_entry->MaxRAmpduFactor;
#ifdef DOT11_HE_AX
		dst_entry->cap.ampdu.max_he_ampdu_len_exp = src_entry->cap.ampdu.max_he_ampdu_len_exp;
#endif /* DOT11_HE_AX */

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,af=%d,%d),dst(wcid=%d,af=%d,%d)\n",
			i,
			src_entry->wcid, src_entry->MaxRAmpduFactor, src_entry->cap.ampdu.max_he_ampdu_len_exp,
			dst_entry->wcid, dst_entry->MaxRAmpduFactor, dst_entry->cap.ampdu.max_he_ampdu_len_exp);
	}

	return TRUE;
}

static int entry_sync_ht(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		if (CLIENT_STATUS_TEST_FLAG(src_entry, fCLIENT_STATUS_HT_CAPABLE))
			CLIENT_STATUS_SET_FLAG(dst_entry, fCLIENT_STATUS_HT_CAPABLE);
		else
			CLIENT_STATUS_CLEAR_FLAG(dst_entry, fCLIENT_STATUS_HT_CAPABLE);

		dst_entry->MaxHTPhyMode.field.MODE = src_entry->MaxHTPhyMode.field.MODE;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,phy_mode=%d,ht=%d,cli_sts=0x%lx), dst(wcid=%d,phy_mode=%d,ht=%d,cli_sts=0x%lx)\n",
			i,
			src_entry->wcid, src_entry->MaxHTPhyMode.field.MODE,
			CLIENT_STATUS_TEST_FLAG(src_entry, fCLIENT_STATUS_HT_CAPABLE) ? 1 : 0,
			src_entry->ClientStatusFlags,
			dst_entry->wcid, dst_entry->MaxHTPhyMode.field.MODE,
			CLIENT_STATUS_TEST_FLAG(dst_entry, fCLIENT_STATUS_HT_CAPABLE) ? 1 : 0,
			src_entry->ClientStatusFlags);
	}

	return TRUE;
}

static int entry_sync_he(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		if (IS_HE_STA(src_entry->cap.modes)) {
			dst_entry->cap.modes &= ~(HE_24G_SUPPORT | HE_5G_SUPPORT);
			if (WMODE_CAP_AX_2G(dst_entry->wdev->PhyMode))
				dst_entry->cap.modes |= HE_24G_SUPPORT;
			if (WMODE_CAP_AX_5G(dst_entry->wdev->PhyMode))
				dst_entry->cap.modes |= HE_5G_SUPPORT;
		}
		dst_entry->cap.he_mac_cap = src_entry->cap.he_mac_cap;
		dst_entry->cap.he_phy_cap = src_entry->cap.he_phy_cap;
		os_move_mem(
			dst_entry->cap.rate.he80_rx_nss_mcs,
			src_entry->cap.rate.he80_rx_nss_mcs,
			sizeof(src_entry->cap.rate.he80_rx_nss_mcs));
		os_move_mem(
			dst_entry->cap.rate.he160_rx_nss_mcs,
			src_entry->cap.rate.he160_rx_nss_mcs,
			sizeof(src_entry->cap.rate.he160_rx_nss_mcs));
		os_move_mem(
			dst_entry->cap.rate.he8080_rx_nss_mcs,
			src_entry->cap.rate.he8080_rx_nss_mcs,
			sizeof(src_entry->cap.rate.he8080_rx_nss_mcs));

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,cap_mode=0x%x,cap=0x%x,0x%x), dst(wcid=%d,cap_mode=0x%x,cap=0x%x,0x%x)\n",
			i,
			src_entry->wcid, src_entry->cap.modes, src_entry->cap.he_mac_cap, src_entry->cap.he_phy_cap,
			dst_entry->wcid, dst_entry->cap.modes, dst_entry->cap.he_mac_cap, dst_entry->cap.he_phy_cap);
	}

	return TRUE;
}

static int entry_sync_eht(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	if (!IS_EHT_STA(src_entry->cap.modes))
		return FALSE;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		dst_entry->cap.modes &= ~(EHT_24G_SUPPORT | EHT_5G_SUPPORT | EHT_6G_SUPPORT);
		if (WMODE_CAP_BE_2G(dst_entry->wdev->PhyMode))
			dst_entry->cap.modes |= EHT_24G_SUPPORT;
		if (WMODE_CAP_BE_5G(dst_entry->wdev->PhyMode))
			dst_entry->cap.modes |= EHT_5G_SUPPORT;
		if (WMODE_CAP_BE_6G(dst_entry->wdev->PhyMode))
			dst_entry->cap.modes |= EHT_6G_SUPPORT;
		dst_entry->eht_mac_cap = src_entry->eht_mac_cap;
		dst_entry->eht_phy_cap = src_entry->eht_phy_cap;
		dst_entry->eht_phy_cap_ext = src_entry->eht_phy_cap_ext;
		if (dst_entry->MaxHTPhyMode.field.BW == BW_20)
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only = src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only;

		os_move_mem(
			dst_entry->cap.rate.rx_nss_mcs,
			src_entry->cap.rate.rx_nss_mcs,
			sizeof(src_entry->cap.rate.rx_nss_mcs));
		os_move_mem(
			dst_entry->cap.rate.tx_nss_mcs,
			src_entry->cap.rate.tx_nss_mcs,
			sizeof(src_entry->cap.rate.tx_nss_mcs));
		os_move_mem(
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss,
			sizeof(src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss));

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,<src> wcid=%d,cap_mode=0x%x,nss=(0x%x,0x%x,0x%x,0x%x;0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)\n",
			i,
			src_entry->wcid, src_entry->cap.modes,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs0_9_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs10_11_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs12_13_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs0_9_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs10_11_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs12_13_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs0_9_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs10_11_nss,
			src_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs12_13_nss);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,<dst> wcid=%d,cap_mode=0x%x,nss=(0x%x,0x%x,0x%x,0x%x;0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)\n",
			i,
			dst_entry->wcid, dst_entry->cap.modes,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs0_7_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs8_9_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs10_11_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss_20_only.max_tx_rx_mcs12_13_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs0_9_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs10_11_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[0].max_tx_rx_mcs12_13_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs0_9_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs10_11_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[1].max_tx_rx_mcs12_13_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs0_9_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs10_11_nss,
			dst_entry->eht_support_mcs_nss.eht_txrx_mcs_nss[2].max_tx_rx_mcs12_13_nss);
	}

	return TRUE;
}

static int entry_sync_vht(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	if (!IS_EHT_STA(src_entry->cap.modes))
		return FALSE;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		dst_entry->cap.modes &= (~VHT_SUPPORT);
		if (WMODE_CAP_AC(dst_entry->wdev->PhyMode)) {
			dst_entry->cap.modes |= VHT_SUPPORT;
			os_move_mem(
				&(dst_entry->vht_cap_ie.vht_cap),
				&(src_entry->vht_cap_ie.vht_cap),
				sizeof(src_entry->vht_cap_ie.vht_cap));
			os_move_mem(
				&(dst_entry->vht_cap_ie.mcs_set),
				&(src_entry->vht_cap_ie.mcs_set),
				sizeof(src_entry->vht_cap_ie.mcs_set));

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
				"i=%d,src(wcid=%d,cap_mode=0x%x);dst(wcid=%d,cap_mode=0x%x)\n",
				i, src_entry->wcid, src_entry->cap.modes, dst_entry->wcid,
				dst_entry->cap.modes);
		}
	}

	return TRUE;
}

static int entry_sync_sr(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	if (!IS_EHT_STA(src_entry->cap.modes))
		return FALSE;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		dst_entry->SRControl = src_entry->SRControl;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,SRControl=0x%x);dst(wcid=%d,SRControl=0x%x)\n",
			i, src_entry->wcid, src_entry->SRControl,
			dst_entry->wcid, dst_entry->SRControl);
	}

	return TRUE;
}

static int entry_sync_mu_edca(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	if (!IS_EHT_STA(src_entry->cap.modes))
		return FALSE;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		dst_entry->ucMUEdcaUpdateCnt = src_entry->ucMUEdcaUpdateCnt;
		os_move_mem(
			dst_entry->arMUEdcaParams,
			src_entry->arMUEdcaParams,
			sizeof(src_entry->arMUEdcaParams));
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,SRControl=0x%x);dst(wcid=%d,SRControl=0x%x)\n",
			i, src_entry->wcid, src_entry->SRControl,
			dst_entry->wcid, dst_entry->SRControl);
	}

	return TRUE;
}

static int entry_sync_amsdu(struct _MAC_TABLE_ENTRY *src_entry, struct mld_entry_t *mld_entry)
{
	struct _MAC_TABLE_ENTRY *dst_entry = NULL;
	uint8_t i = 0;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		dst_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (!dst_entry)
			continue;
		if (dst_entry->wcid == mld_entry->setup_link_wcid)
			continue;
		dst_entry->AMsduSize = src_entry->AMsduSize;
		dst_entry->amsdu_limit_len = src_entry->amsdu_limit_len;
		dst_entry->amsdu_limit_len_adjust = src_entry->amsdu_limit_len_adjust;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_INFO,
			"i=%d,src(wcid=%d,sz=%d,limit=%d,%d),dst(wcid=%d,sz=%d,limit=%d,%d)\n",
			i,
			src_entry->wcid, src_entry->AMsduSize, src_entry->amsdu_limit_len, src_entry->amsdu_limit_len_adjust,
			dst_entry->wcid, dst_entry->AMsduSize, dst_entry->amsdu_limit_len, dst_entry->amsdu_limit_len_adjust);
	}

	return TRUE;
}

static int _entrytb_mlo_sync(struct _MAC_TABLE_ENTRY *entry, uint8_t main_cat, uint8_t sub_cat)
{
	int sts = TRUE;
	struct mld_entry_t *mld_entry;
	struct mld_entry_t mld_entry_info;

	if (entry->mlo.mlo_en) {
		mt_rcu_read_lock();
		mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
				"mld_entry is NULL\n");
			return FALSE;
		}
		os_move_mem(&mld_entry_info, mld_entry, sizeof(struct mld_entry_t));
		mt_rcu_read_unlock();

		switch (main_cat) {
		case ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL:
			if (sub_cat == ENTRY_SYNC_SUB_APPS)
				sts = entry_sync_apps(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_QOS)
				sts = entry_sync_wmm(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_SST)
				sts = entry_sync_sst(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_AF)
				sts = entry_sync_af(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_HT_CAP)
				sts = entry_sync_ht(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_HE_CAP)
				sts = entry_sync_he(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_AMSDU)
				sts = entry_sync_amsdu(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_EHT_CAP)
				sts = entry_sync_eht(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_VHT_CAP)
				sts = entry_sync_vht(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_SR)
				sts = entry_sync_sr(entry, &mld_entry_info);
			if (sub_cat == ENTRY_SYNC_SUB_MU_EDCA)
				sts = entry_sync_mu_edca(entry, &mld_entry_info);
			break;
		case ENTRY_SYNC_MAIN_SETUPLINK_CONN_LINK_LEVEL:
			break;
		case ENTRY_SYNC_MAIN_SETUPLINK_NOT_CONN_MLD_LEVEL:
			break;
		case ENTRY_SYNC_MAIN_SETUPLINK_NOT_CONN_LINK_LEVEL:
			break;
		default:
			break;
		}
	}

	return sts;
}

void entrytb_mlo_sync(struct _MAC_TABLE_ENTRY *entry)
{
	if (entry) {
		_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_APPS);
		_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_QOS);
		_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_SST);
		_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_AF);
		_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_HT_CAP);
		_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_HE_CAP);
		_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_AMSDU);
		if ((entry->wdev) && (entry->wdev->wdev_type == WDEV_TYPE_STA)) {
			_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_EHT_CAP);
			_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_VHT_CAP);
			_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_SR);
			_entrytb_mlo_sync(entry, ENTRY_SYNC_MAIN_SETUPLINK_CONN_MLD_LEVEL, ENTRY_SYNC_SUB_MU_EDCA);
		}
	} else
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR, "entry is null!\n");
}

struct mld_entry_ctrl_t _mld_ety_ctrl;

struct mld_entry_ctrl_t *mld_entry_ctrl_get(void)
{
	return &_mld_ety_ctrl;
}

struct mld_entry_ext_t *mld_entry_ext_get_by_idx(
	struct _RTMP_ADAPTER *ad, UINT16 mld_sta_idx)
{
	if (mld_sta_idx > MLD_STA_MAX_NUM - 1) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			"INVALID mld_sta_idx=%d\n", mld_sta_idx);
		return NULL;
	} else
		return &ad->MacTab->mld_entry_ext[mld_sta_idx];
}

void mld_entry_ext_init(struct _RTMP_ADAPTER *ad, u16 mld_sta_idx)
{
	struct mld_entry_ext_t *mld_entry_ext = NULL;
	struct seq_ctrl_t *seq_ctrl = NULL;

	mld_entry_ext = mld_entry_ext_get_by_idx(ad, mld_sta_idx);
	if (mld_entry_ext) {
		os_zero_mem(mld_entry_ext, sizeof(*mld_entry_ext));

		seq_ctrl = &mld_entry_ext->tr_entry.seq_ctrl;
		seq_ctrl_init(seq_ctrl);
	}
}

int mld_entry_ext_get(struct _MAC_TABLE_ENTRY *entry, struct mld_entry_ext_t **mld_entry_ext)
{
	struct mld_entry_t *mld_entry = NULL;

	if (!entry)
		return NDIS_STATUS_FAILURE;

	mt_rcu_read_lock();
	mld_entry = rcu_dereference(entry->mld_entry);
	if (!mld_entry) {
		mt_rcu_read_unlock();
		return NDIS_STATUS_FAILURE;
	}
	*mld_entry_ext = mld_entry_ext_get_by_idx(entry->pAd, mld_entry->mld_sta_idx);
	mt_rcu_read_unlock();

	return  (*mld_entry_ext == NULL) ?
		NDIS_STATUS_FAILURE : NDIS_STATUS_SUCCESS;
}

void mld_entry_init(void)
{
	struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
	int i = 0;

	NdisAllocateSpinLock(NULL, &mld_entry_ctrl->mld_entry_lock);

	for (i = 0; i < HASH_TABLE_SIZE; i++)
		INIT_HLIST_HEAD(&mld_entry_ctrl->mld_hash[i]);
}

void mld_entry_exit(void)
{
	struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
	int i = 0;
	struct hlist_head *head = NULL;
	struct mld_entry_t *mld_entry = NULL;

	spin_lock_bh(&mld_entry_ctrl->mld_entry_lock);
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		head = &mld_entry_ctrl->mld_hash[i];
		hlist_for_each_entry_rcu(mld_entry, head, hlist) {
			hlist_del_rcu(&mld_entry->hlist);
			mt_call_rcu(&mld_entry->rcu, free_mld_entry);
		}
	}
	spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);
	NdisFreeSpinLock(&mld_entry_ctrl->mld_entry_lock);
}

struct mld_entry_t *get_mld_entry_by_mac(u8 *addr)
{
	struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
	struct mld_entry_t *mld_entry = NULL;
	struct hlist_head *head = NULL;
	u8 hash_idx = MAC_ADDR_HASH_INDEX(addr);

	head = &mld_entry_ctrl->mld_hash[hash_idx];

	hlist_for_each_entry_rcu(mld_entry, head, hlist) {
		if (MAC_ADDR_EQUAL(mld_entry->addr, addr))
			break;
	}

	return mld_entry;
}

struct mld_entry_t *create_mld_entry(void)
{
	struct mld_entry_t *mld_entry = NULL;

	os_alloc_mem(NULL, (u8 **)&mld_entry, sizeof(struct mld_entry_t));
	if (mld_entry)
		os_zero_mem(mld_entry, sizeof(struct mld_entry_t));

	return mld_entry;
}

void update_mld_entry(struct mld_entry_t *old_mld_entry, struct mld_entry_t *new_mld_entry, struct _MAC_TABLE_ENTRY *pEntry, struct mld_sta_add *add_sta)
{
	u8 link_id;
	struct mld_link_add *add_link, *tmp_add_link;
	u8 i = 0;
	struct _RTMP_ADAPTER *ad = pEntry->pAd;

	if (old_mld_entry)
		os_move_mem(new_mld_entry, old_mld_entry, sizeof(struct mld_entry_t));

	link_id = pEntry->mlo.link_info.link_id;
	if (link_id >= MLD_LINK_MAX)
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			"link_id >= MLD_LINK_MAX!\n");

	add_link = &add_sta->add_link[add_sta->link_id];
	COPY_MAC_ADDR(pEntry->mlo.mld_addr, add_sta->mld_addr);

	if (add_link->is_setup_link) {
		u16 mld_sta_idx = add_sta->mld_sta_idx;

		if (ad && BMGR_VALID_MLD_STA(mld_sta_idx)) {
			mld_entry_ext_init(ad, mld_sta_idx);
		}

		COPY_MAC_ADDR(new_mld_entry->addr, add_sta->mld_addr);
		new_mld_entry->mld_sta_idx = add_sta->mld_sta_idx;
		new_mld_entry->emlmr = add_sta->emlmr;
		new_mld_entry->emlsr = add_sta->emlsr;
		new_mld_entry->eml_cap = add_sta->eml_caps;
		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			tmp_add_link = &add_sta->add_link[i];
			new_mld_entry->str_map[i] = tmp_add_link->str_map;
		}
		new_mld_entry->setup_link_wcid = pEntry->wcid;
		new_mld_entry->setup_link_id = add_sta->link_id;

		for (i = 0; i < MLD_LINK_MAX; i++)
			new_mld_entry->link_entry[i] = NULL;
		new_mld_entry->link_num = 0;

		if (link_id < MLD_LINK_MAX) {
			new_mld_entry->link_entry[link_id] = pEntry;
			new_mld_entry->link_num = 1;
		}

		new_mld_entry->valid = 1;
	} else {
		if (link_id < MLD_LINK_MAX) {
			new_mld_entry->link_entry[link_id] = pEntry;
			new_mld_entry->link_num++;
		}
	}
}

void update_link_mld_entry(struct mld_entry_t *mld_entry)
{
	u8 i = 0;
	struct _MAC_TABLE_ENTRY *pEntry = NULL;

	if (!mld_entry)
		return;

	for (i = 0; i < MLD_LINK_MAX; i++) {
		pEntry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
		if (pEntry)
			rcu_assign_pointer(pEntry->mld_entry, mld_entry);
	}
}

void remove_link_mld_entry(struct _MAC_TABLE_ENTRY *pEntry)
{
	if (!pEntry)
		return;
	rcu_assign_pointer(pEntry->mld_entry, NULL);
}


void update_mld_entry_apcli(
	struct mld_entry_t *old_mld_entry,
	struct mld_entry_t *new_mld_entry,
	struct wifi_dev *wdev,
	uint8_t index,
	uint8_t link_num,
	uint8_t *flag_once,
	uint16_t setup_wcid)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct mld_dev *mld = wdev->mld_dev;
	struct mld_link_entry *set_up_link = NULL;
	struct peer_mld_entry *peer = NULL;

	set_up_link = get_sta_mld_link_by_wdev(mld, wdev);

	if (!set_up_link)
		return;

	peer = &mld->peer_mld;

	if (old_mld_entry)
		os_move_mem(new_mld_entry, old_mld_entry, sizeof(struct mld_entry_t));

	pEntry = (MAC_TABLE_ENTRY *)peer->single_link[index].priv_ptr;
	COPY_MAC_ADDR(pEntry->mlo.mld_addr, peer->mld_addr);

	if (!(*flag_once)) {
		struct _RTMP_ADAPTER *ad = pEntry->pAd;

		new_mld_entry->emlmr = 0;	/*to do*/
		new_mld_entry->emlsr = 0;	/*to do*/
		new_mld_entry->mld_sta_idx = peer->idx;
		COPY_MAC_ADDR(new_mld_entry->addr, peer->mld_addr);
		new_mld_entry->link_num = link_num;
		new_mld_entry->setup_link_wcid = setup_wcid;

		/* initialize mld_entry_ext */
		mld_entry_ext_init(ad, new_mld_entry->mld_sta_idx);

		*flag_once = 1;
	}

	new_mld_entry->str_map[index] = 0x07;/*copy from ap mlo*/
	if (peer->single_link[index].active && pEntry->mlo.link_info.link_id < MLD_LINK_MAX)
		new_mld_entry->link_entry[pEntry->mlo.link_info.link_id] = peer->single_link[index].priv_ptr;

	if (pEntry->wdev == set_up_link->wdev)
		new_mld_entry->setup_link_id = pEntry->mlo.link_info.link_id;
}

void free_mld_entry(struct rcu_head *head)
{
	struct mld_entry_t *mld_entry = container_of(head, struct mld_entry_t, rcu);

	kfree(mld_entry);
}

void mld_update_roaming_entry(IN RTMP_ADAPTER * pAd,
	struct _MAC_TABLE_ENTRY *moved_entry,
	struct _MAC_TABLE_ENTRY *a4_entry)
{
	struct _MAC_TABLE_ENTRY *entry_ptr;
	struct mld_entry_t *mld_entry = NULL;
	int i = 0;

	if (!moved_entry || !a4_entry)
		return;
	if (!moved_entry->mlo.mlo_en)
		return;

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(moved_entry->mlo.mld_addr);
	if (mld_entry) {
		for (i = 0; i < MLD_LINK_MAX; i++) {
			entry_ptr = mld_entry->link_entry[i];

			if (!entry_ptr)
				continue;
#ifdef A4_CONN
			entry_ptr->roaming_entry = TRUE;
			a4_proxy_update(pAd, a4_entry->func_tb_idx, a4_entry->wcid, entry_ptr->Addr, 0);
#endif
		}
	}
	mt_rcu_read_unlock();
}

int mld_entry_fill_fields(u8 *addr, struct mld_entry_t *mld_entry, UINT32 flags)
{
	struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
	struct mld_entry_t *old_mld_entry, *new_mld_entry;
	int ret = 0;

	if (!addr || !mld_entry)
		return -EINVAL;

	spin_lock_bh(&mld_entry_ctrl->mld_entry_lock);
	old_mld_entry = get_mld_entry_by_mac(addr);
	if (old_mld_entry) {
		new_mld_entry = create_mld_entry();
		if (new_mld_entry) {
			os_move_mem(new_mld_entry, old_mld_entry, sizeof(struct mld_entry_t));
			if (flags & MLD_ENTRY_FIELD_FLAGS_NSEP)
				new_mld_entry->nsep = mld_entry->nsep;
			if (flags & MLD_ENTRY_FIELD_FLAGS_EMLSR)
				new_mld_entry->emlsr = mld_entry->emlsr;
			if (flags & MLD_ENTRY_FIELD_FLAGS_SETUP_LINK_WCID)
				new_mld_entry->setup_link_wcid = mld_entry->setup_link_wcid;
			if (flags & MLD_ENTRY_FIELD_FLAGS_SETUP_LINK_ID)
				new_mld_entry->setup_link_id = mld_entry->setup_link_id;

			hlist_replace_rcu(&old_mld_entry->hlist, &new_mld_entry->hlist);
			update_link_mld_entry(new_mld_entry);
			mt_call_rcu(&old_mld_entry->rcu, free_mld_entry);
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
				" create mld_entry Fail!\n");
			ret = -ENOMEM;
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
			" Find mld_entry Fail!\n");
		ret = -EFAULT;
	}
	spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);

	return ret;
}

struct _MAC_TABLE_ENTRY *mld_entry_link_select(struct mld_entry_t *mld_entry)
{
	u8 setup_link_id = 0;
	struct _MAC_TABLE_ENTRY *link_entry = NULL;

	/*
	 * before ML configuration feature: select setup link
	 * with ML configuration feature: select any link
	 */
	if (mld_entry) {
		setup_link_id = mld_entry->setup_link_id;
		if (setup_link_id >= MLD_LINK_MAX) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
				"INVALID setup_link_id(%d)\n", setup_link_id);
			return link_entry;
		}
		link_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[setup_link_id];

		if (link_entry)
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_DEBUG,
				"mld_entry(%pM) select link_entry[setup_link_id=%d]=%pM\n",
				mld_entry->addr, setup_link_id, link_entry->Addr);
		else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_DEBUG,
				"mld_entry(%pM) fail to select link_entry[setup_link_id=%d]\n",
				mld_entry->addr, setup_link_id);
	}

	return link_entry;
}


/*
*** Fn: is_mld_link_entry_on_traffic
*** INPUT param: struct _MAC_TABLE_ENTRY *pEntry
*** OUTPUT param:
*** RET: Boolean:
***  -TRUE if any related mlo MacTable has traffic
***  -FALSE if neither of related mlo MacTable has traffic
***  (Do not check Input MacTable Entry)
*/
BOOLEAN is_mld_link_entry_on_traffic(struct _MAC_TABLE_ENTRY *pEntry)
{
	struct _MAC_TABLE_ENTRY *link_entry = NULL;
	u8 i = 0, link_cnt = 0;
	UINT32 sta_idle_timeout = 0;
	struct mld_entry_t *mld_entry = NULL;

	if (!pEntry || !(pEntry->mlo.mlo_en))
		return FALSE;

	mt_rcu_read_lock();
	mld_entry = rcu_dereference(pEntry->mld_entry);

	if (!mld_entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATSEC_MLO, DBG_LVL_ERROR,
		"[%d]: ERROR, mld_entry=NULL\n", __LINE__);
		mt_rcu_read_unlock();
		return FALSE;
	}

	do {
		link_entry = mld_entry->link_entry[i++];
		if ((!link_entry) || (link_entry == pEntry))
			continue;
		link_cnt++;

#if defined(DOT11_HE_AX) && defined(WIFI_TWT_SUPPORT)
		/* If TWT agreement is present for this STA,
		add maximum TWT wake up interval in sta idle timeout. */
		if (GET_PEER_ITWT_FID_BITMAP(link_entry)) {
			sta_idle_timeout = link_entry->StaIdleTimeout
								+ link_entry->twt_ctrl.twt_interval_max;
		} else
#endif
		{
			sta_idle_timeout = link_entry->StaIdleTimeout;
		}

		if (link_entry->NoDataIdleCount <= sta_idle_timeout)
			return TRUE;
	} while ((i < MLD_LINK_MAX) &&
			(link_cnt < mld_entry->link_num));
	mt_rcu_read_unlock();

	return FALSE;
}

BOOLEAN mld_entry_match_to_source_addr(struct _MAC_TABLE_ENTRY *pEntry, u8 *sa)
{
	struct mld_entry_t *mld_entry = NULL;
	MAC_TABLE_ENTRY *entry_ptr = NULL;
	UCHAR i;
	BOOLEAN match = FALSE;

	if (!pEntry || !sa)
		return match;
	if (!pEntry->mlo.mlo_en)
		return match;

	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
	if (mld_entry) {
		for (i = 0; i < MLD_LINK_MAX; i++) {
			entry_ptr = mld_entry->link_entry[i];
			if (!entry_ptr)
				continue;
			if (MAC_ADDR_EQUAL(entry_ptr->Addr, sa)) {
				match = TRUE;
				break;
			}
		}
	}
	mt_rcu_read_unlock();
	return match;
}

void MacTableAuthReqClearMloEntry(struct _RTMP_ADAPTER *pAd, UCHAR *link_addr, BOOLEAN is_mlo_auth)
{
	struct _MAC_TABLE_ENTRY *pEntry;

	/* sanity the peer's mlo-capability if peer existing */
	pEntry = MacTableLookup(pAd, link_addr);
	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		/* (mlo -> non-mlo/mlo) || (non-mlo -> mlo) */
		if (IS_ENTRY_MLO(pEntry) || is_mlo_auth) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ENTRY, DBG_LVL_ERROR,
				"clear peers, mac=%pM, wcid=%d, is_mlo=%d, request_mlo=%d\n",
				pEntry->Addr, pEntry->wcid, IS_ENTRY_MLO(pEntry), is_mlo_auth);
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}
	}
}


#ifdef EAP_STATS_SUPPORT
BOOLEAN check_mld_link_entry_per_times(struct _MAC_TABLE_ENTRY *pEntry)
{
	struct _MAC_TABLE_ENTRY *link_entry = NULL;
	u8 i = 0, link_cnt = 0;
	struct mld_entry_t *mld_entry = NULL;
	UINT8 per_err_times_total = 0;
	ULONG contd_fail_cnt_total = 0;

	if (!pEntry || !(pEntry->mlo.mlo_en))
		return FALSE;

	mt_rcu_read_lock();
	mld_entry = rcu_dereference(pEntry->mld_entry);

	if (!mld_entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATSEC_MLO, DBG_LVL_ERROR,
		"(%s)[%d]: ERROR, mld_entry=NULL\n", __func__, __LINE__);
		mt_rcu_read_unlock();
		return FALSE;
	}

	do {
		link_entry = mld_entry->link_entry[i++];
		if (!link_entry)
			continue;
		link_cnt++;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATSEC_MLO, DBG_LVL_DEBUG,
				"wcid %d, link_entry->txm.per_err_times %d, link_entry->txm.contd_fail_cnt %ld\n",
				link_entry->wcid, link_entry->txm.per_err_times, link_entry->txm.contd_fail_cnt);

		per_err_times_total += link_entry->txm.per_err_times;
		contd_fail_cnt_total += link_entry->txm.contd_fail_cnt;

	} while ((i < MLD_LINK_MAX) &&
			(link_cnt < mld_entry->link_num));
	mt_rcu_read_unlock();

	if (per_err_times_total >= txm_thres.per_err_total &&
		contd_fail_cnt_total >= txm_thres.tx_contd_fail_total)
		return TRUE;

	return FALSE;
}
#endif
#endif /* DOT11_EHT_BE */

void entry_del_hw_ppe_entry(struct _RTMP_ADAPTER *pad,
unsigned char *mac)
{
	if (!mac || !pad) {
		MTWF_DBG(NULL, DBG_CAT_MLME, CATMLME_WTBL,
			DBG_LVL_WARN, "null mac address\n");
		return;
	}
#ifdef WHNAT_SUPPORT
#ifdef CONFIG_FAST_NAT_SUPPORT
	if (PD_GET_WHNAT_ENABLE(pad->physical_dev) && ppe_del_entry_by_mac)
		ppe_del_entry_by_mac(mac);
#endif
#endif
#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_clean_entry_by_mac)
		ra_sw_nat_hook_clean_entry_by_mac(mac);
#endif
}
INLINE struct _MAC_TABLE_ENTRY *entry_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid)
{
	return &mac_ad->MacTab->Content[wcid];
}

INLINE UCHAR *entry_addr_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid)
{
	return &(mac_ad->MacTab->Content[wcid].Addr[0]);
}

INLINE struct wifi_dev *entry_wdev_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid)
{
	return mac_ad->MacTab->Content[wcid].wdev;
}

INLINE UCHAR entry_func_tb_idx_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid)
{
	return mac_ad->MacTab->Content[wcid].func_tb_idx;
}

INLINE struct _STA_TR_ENTRY *tr_entry_get(
	struct _RTMP_ADAPTER *mac_ad, UINT16 wcid)
{
	return &mac_ad->MacTab->Content[wcid].tr_entry;
}

void entry_table_init(struct physical_device *device)
{
	MTWF_PRINT("%s: PD_GET_HW_WTBL_MAX(device)=%u\n",
		__func__, PD_GET_HW_WTBL_MAX(device));

	device->mac_table.Content =
		vmalloc(sizeof(struct _MAC_TABLE_ENTRY) * PD_GET_HW_WTBL_MAX(device));
#ifdef DOT11_EHT_BE
	device->mac_table.mld_entry_ext =
		vmalloc(sizeof(struct mld_entry_ext_t) * MLD_STA_MAX_NUM);
#endif /*DOT11_EHT_BE*/
	device->RxRateResultPair =
		vmalloc(sizeof(struct _PHY_STATE_RX_RATE_PAIR) * PD_GET_HW_WTBL_MAX(device));

	ASSERT(device->mac_table.Content);
#ifdef DOT11_EHT_BE
	ASSERT(device->mac_table.mld_entry_ext);
#endif /*DOT11_EHT_BE*/
	ASSERT(device->RxRateResultPair);

	if (device->mac_table.Content)
		os_zero_mem(device->mac_table.Content,
			(sizeof(struct _MAC_TABLE_ENTRY) * PD_GET_HW_WTBL_MAX(device)));
#ifdef DOT11_EHT_BE
	if (device->mac_table.mld_entry_ext)
		os_zero_mem(device->mac_table.mld_entry_ext,
			(sizeof(struct mld_entry_ext_t) * MLD_STA_MAX_NUM));
#endif /*DOT11_EHT_BE*/
	if (device->RxRateResultPair)
		os_zero_mem(device->RxRateResultPair,
			(sizeof(struct _PHY_STATE_RX_RATE_PAIR) * PD_GET_HW_WTBL_MAX(device)));
}

void entry_table_deinit(struct physical_device *device)
{
	if (device->mac_table.Content)
		vfree(device->mac_table.Content);
#ifdef DOT11_EHT_BE
	if (device->mac_table.mld_entry_ext)
		vfree(device->mac_table.mld_entry_ext);
#endif /*DOT11_EHT_BE*/
	if (device->RxRateResultPair)
		vfree(device->RxRateResultPair);
}

void pause_bitmap_init(struct physical_device *device)
{
	UINT8 i;
	UINT8  band_num = PD_GET_BAND_NUM(device);
	UINT16 all_ac = PD_GET_HW_DRR_MAX_DW_ALL_AC(device);
	UINT16 per_ac = PD_GET_HW_DRR_MAX_DW_PER_AC(device);
	UINT16 twt = PD_GET_HW_DRR_MAX_DW_TWT(device);

	device->pause_bitmap.pause = NULL;
	device->pause_bitmap.twt_pause = NULL;

	/* 4 ACs */
	device->pause_bitmap.pause = vmalloc(band_num * sizeof(UINT32 *));
	if (!device->pause_bitmap.pause)
		goto err;
	for (i = 0; i < band_num; i++) {
		device->pause_bitmap.pause[i] = vmalloc(all_ac * sizeof(UINT32));
		if (!device->pause_bitmap.pause[i])
			goto err;
		os_zero_mem(device->pause_bitmap.pause[i],
			(all_ac * sizeof(UINT32)));
	}

	/* TWT */
	device->pause_bitmap.twt_pause = vmalloc(band_num * sizeof(UINT32 *));
	if (!device->pause_bitmap.twt_pause)
		goto err;
	for (i = 0; i < band_num; i++) {
		device->pause_bitmap.twt_pause[i] = vmalloc(twt * sizeof(UINT32));
		if (!device->pause_bitmap.twt_pause[i])
			goto err;
		os_zero_mem(device->pause_bitmap.twt_pause[i],
			(twt * sizeof(UINT32)));
	}

	MTWF_PRINT("%s: band_num=%u, all_ac=%u, per_ac=%u, twt=%u, pause=%p, twt_pause=%p\n",
		__func__, band_num, all_ac, per_ac, twt, device->pause_bitmap.pause, device->pause_bitmap.twt_pause);

	for (i = 0; i < band_num; i++) {
		MTWF_PRINT("pause[%u]=%p\n", i, device->pause_bitmap.pause[i]);
		MTWF_PRINT("twt_pause[%u]=%p\n", i, device->pause_bitmap.twt_pause[i]);
	}

	return;
err:

	MTWF_PRINT("!!!ERROR !!!%s: band_num=%u, all_ac=%u, per_ac=%u, twt=%u\n",
		__func__, band_num, all_ac, per_ac, twt);

	/* 4 ACs */
	for (i = 0; i < band_num; i++) {
		if (device->pause_bitmap.pause
				&& device->pause_bitmap.pause[i])
			vfree(device->pause_bitmap.pause[i]);
	}
	if (device->pause_bitmap.pause)
		vfree(device->pause_bitmap.pause);

	/* TWT */
	for (i = 0; i < band_num; i++) {
		if (device->pause_bitmap.twt_pause
				&& device->pause_bitmap.twt_pause[i])
			vfree(device->pause_bitmap.twt_pause[i]);
	}
	if (device->pause_bitmap.twt_pause)
		vfree(device->pause_bitmap.twt_pause);


}

void pause_bitmap_deinit(struct physical_device *device)
{
	UINT8 i;
	UINT8  band_num = PD_GET_BAND_NUM(device);

	/* 4 ACs */
	for (i = 0; i < band_num; i++) {
		if (device->pause_bitmap.pause[i])
			vfree(device->pause_bitmap.pause[i]);
	}
	if (device->pause_bitmap.pause)
		vfree(device->pause_bitmap.pause);

	/* TWT */
	for (i = 0; i < band_num; i++) {
		if (device->pause_bitmap.twt_pause[i])
			vfree(device->pause_bitmap.twt_pause[i]);
	}
	if (device->pause_bitmap.twt_pause)
		vfree(device->pause_bitmap.twt_pause);
}
