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
	ba_action.c
*/

#include "rt_config.h"
#include "action.h"

#define BA_ORI_INIT_SEQ		(seq_ctrl->TxSeq[TID]) /*1: initial sequence number of BA session*/
#define ORI_SESSION_MAX_RETRY	8
#define ORI_BA_SESSION_TIMEOUT	(2000)	/* ms */
#define REC_BA_SESSION_IDLE_TIMEOUT	(1000)	/* ms */
#define REORDERING_PACKET_TIMEOUT	((REORDERING_PACKET_TIMEOUT_IN_MS * OS_HZ)/1000)	/* system ticks -- 100 ms*/
#define MAX_REORDERING_PACKET_TIMEOUT	((MAX_REORDERING_PACKET_TIMEOUT_IN_MS * OS_HZ)/1000)	/* system ticks -- 100 ms*/
#define INVALID_RCV_SEQ (0xFFFF)

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
u16 free_buf_id_head;
u16 free_buf_id_tail = BUF_FREE_ID_SIZE - 1;
#define FLUSH_ALL_REORDERING_PACKET_TIMEOUT (REORDERING_PACKET_TIMEOUT<<1)
#define FLUSH_ALL_REORDERING_PACKET_TIMEOUT_IN_MS (250)
#define IDX_INC(idx, total_size) \
{\
	(idx)++; \
	if ((idx) >= (total_size)) \
		(idx) = 0;\
}
#endif

BUILD_TIMER_FUNCTION(ba_reorder_timeout_Exec);
DECLARE_TIMER_FUNCTION(ba_reorder_timeout_Exec);

static inline void update_last_in_sn(struct BA_REC_ENTRY *pBAEntry, u16 sn)
{
	pBAEntry->LastIndSeq = sn;
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
	pBAEntry->head_sn = (sn + 1) & MAXSEQ;
	pBAEntry->head_index = pBAEntry->head_sn % pBAEntry->BAWinSize;
#endif
}

static inline void ba_enqueue_head(struct reordering_list *list,
							struct reordering_mpdu *mpdu_blk)
{
	list->qlen++;
	mpdu_blk->next = list->next;
	list->next = mpdu_blk;

	if (!list->tail)
		list->tail = mpdu_blk;
}

static inline void ba_enqueue_tail(struct reordering_list *list,
							struct reordering_mpdu *mpdu_blk)
{
	list->qlen++;
	mpdu_blk->next = NULL;

	if (list->tail)
		list->tail->next = mpdu_blk;
	else
		list->next = mpdu_blk;

	list->tail = mpdu_blk;
}

static inline struct reordering_mpdu *ba_dequeue_head(struct reordering_list *list)
{
	struct reordering_mpdu *mpdu_blk = NULL;

	if (list && list->next) {
		list->qlen--;
		mpdu_blk = list->next;
		list->next = mpdu_blk->next;

		if (mpdu_blk == list->tail)
			list->tail = NULL;
	}

	return mpdu_blk;
}

static inline struct reordering_mpdu  *ba_reordering_mpdu_dequeue(struct reordering_list *list)
{
	struct reordering_mpdu *ret = (ba_dequeue_head(list));
	return ret;
}


inline struct reordering_mpdu *ba_reordering_mpdu_probe(struct reordering_list *list)
{
	struct reordering_mpdu *ret = NULL;

	if (list)
		ret = list->next;
	return ret;
}

VOID ba_resource_raw_dump(RTMP_ADAPTER *pAd)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_ORI_ENTRY *pOriBAEntry = NULL;
	struct BA_REC_ENTRY *pRecBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;
	int index = 1;

	NdisAcquireSpinLock(&ba_ctl->BATabLock);
	MTWF_PRINT("Originator=%ld, Recipient=%ld\n", ba_ctl->numAsOriginator, ba_ctl->numAsRecipient);
	NdisReleaseSpinLock(&ba_ctl->BATabLock);

	MTWF_PRINT("[Originator]\n");
	for (index = 1; index < MAX_LEN_OF_BA_ORI_TABLE; index++) {
		pOriBAEntry = &ba_ctl->BAOriEntry[index];
		pEntry = pOriBAEntry->pEntry;
		if (pOriBAEntry->ORI_BA_Status != Originator_NONE) {
			MTWF_PRINT("BA_idx[%d] Tid[%d] Win[%d] Status[%d] Valid[%d]\n",
				index, pOriBAEntry->TID, pOriBAEntry->BAWinSize,
				pOriBAEntry->ORI_BA_Status, IS_VALID_ENTRY(pEntry));
		}
	}

	MTWF_PRINT("[Recipient]\n");
	for (index = 1; index < MAX_LEN_OF_BA_REC_TABLE; index++) {
		pRecBAEntry = &ba_ctl->BARecEntry[index];
		NdisAcquireSpinLock(&pRecBAEntry->RxReRingLock);
		pEntry = pRecBAEntry->pEntry;
		if (pRecBAEntry->REC_BA_Status != Recipient_NONE) {
			MTWF_PRINT("BA_idx[%d] Tid[%d] Win[%d] Status[%d] Valid[%d]",
				index, pRecBAEntry->TID, pRecBAEntry->BAWinSize,
				pRecBAEntry->REC_BA_Status, IS_VALID_ENTRY(pEntry));

			MTWF_PRINT("Seid[%d] Act[%d] WaitWM[%d] Wcid[%d]\n",
				pRecBAEntry->Session_id, pRecBAEntry->Postpone_Action,
				pRecBAEntry->WaitWM, pEntry?pEntry->wcid:0);
		}
		NdisReleaseSpinLock(&pRecBAEntry->RxReRingLock);
	}
}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
void dump_ba_list(struct BA_REC_ENTRY *pBAEntry)
{
	struct reordering_mpdu **reorder_buf = pBAEntry->reorder_buf;
	struct reordering_mpdu *next_blk = NULL, *blk = NULL;
	int i = 0, amsdu_cnt = 1;
	u16 idx = pBAEntry->head_index;

	for (i = 0; i < pBAEntry->BAWinSize; i++) {
		if (reorder_buf[idx]) {
			do {
				if (next_blk) {
					blk = next_blk;
					amsdu_cnt++;
				} else {
					blk = reorder_buf[idx];
				}
				next_blk = blk->next;
			} while (next_blk);
			MTWF_PRINT("mpdu:SN = %d, AMSDU type = %d, cnt = %d\n", blk->Sequence, blk->bAMSDU, amsdu_cnt);
			amsdu_cnt = 1;
		}
		IDX_INC(idx, pBAEntry->BAWinSize);
	}
}
#else
void dump_ba_list(struct reordering_list *list)
{
	struct reordering_mpdu *mpdu_blk = NULL;

	if (list->next) {
		MTWF_PRINT("\n ba sn list:");
		mpdu_blk = list->next;

		while (mpdu_blk) {
			MTWF_PRINT("%x ", mpdu_blk->Sequence);
			mpdu_blk = mpdu_blk->next;
		}
	}

	MTWF_PRINT("\n\n");
}
#endif

VOID reclaim_ba_rec_session_by_seid(RTMP_ADAPTER *pAd, UINT_16 Seid, UINT_8 Tid)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_REC_ENTRY *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_INFO *ba_info;
	int index = 1;

	if (Seid == 0)
		return;

	for (index = 1; index < MAX_LEN_OF_BA_REC_TABLE; index++) {
		pBAEntry = &ba_ctl->BARecEntry[index];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if (pBAEntry->Session_id != Seid ||
			pBAEntry->Postpone_Action != DELBA_POSTPONE ||
			pBAEntry->pEntry == NULL) {
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			continue;
		}

		pEntry = pBAEntry->pEntry;
		ba_info = &pEntry->ba_info;

		if (!IS_VALID_ENTRY(pEntry) || pEntry->ba_info.RecWcidArray[Tid] == 0) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"Reclaim BA[%d] Seid[%d] Tid[%d] Status[%d] Valid[%d]\n",
				index, Seid, pBAEntry->TID, pBAEntry->REC_BA_Status, IS_VALID_ENTRY(pEntry));

			ba_info->RecWcidArray[pBAEntry->TID] = 0;
			ba_info->RxBitmap &= (~(1 << (pBAEntry->TID)));
			pBAEntry->REC_BA_Status = Recipient_NONE;
			pBAEntry->WaitWM = FALSE;
			pBAEntry->Session_id = 0;
			pBAEntry->RetryCnt = 0;
			NdisAcquireSpinLock(&ba_ctl->BATabLock);
			if (ba_ctl->numAsRecipient > 0)
				ba_ctl->numAsRecipient -= 1;
			NdisReleaseSpinLock(&ba_ctl->BATabLock);
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}
}

static VOID ba_free_ori_entry(RTMP_ADAPTER *pAd, struct BA_ORI_ENTRY *pBAEntry)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);

	if (pBAEntry->ORI_BA_Status == Originator_NONE)
		return;

	if (pBAEntry->ORI_BA_Status == Originator_Done) {
		NdisAcquireSpinLock(&ba_ctl->BATabLock);
		ba_ctl->numDoneOriginator -= 1;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"numAsOriginator= %ld\n",
			ba_ctl->numDoneOriginator);
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
	}

	NdisAcquireSpinLock(&ba_ctl->BATabLock);
	if (ba_ctl->numAsOriginator != 0)
		ba_ctl->numAsOriginator -= 1;
	else
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			": numAsOriginator = 0, ORI_BA_Status = %d\n",
			pBAEntry->ORI_BA_Status);
	NdisReleaseSpinLock(&ba_ctl->BATabLock);

	pBAEntry->ORI_BA_Status = Originator_NONE;
	pBAEntry->Token = 0;
}

static UINT announce_non_hw_damsdu_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, UCHAR OpMode)
{
	PUCHAR pData;
	USHORT DataSize;
	UINT nMSDU = 0;

	pData = (PUCHAR)GET_OS_PKT_DATAPTR(pPacket);
	DataSize = (USHORT)GET_OS_PKT_LEN(pPacket);
	nMSDU = deaggregate_amsdu_announce(pAd, pPacket, pData, DataSize, OpMode);
	return nMSDU;
}

static void announce_ba_reorder_pkt(RTMP_ADAPTER *pAd, struct reordering_mpdu *mpdu)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	PNDIS_PACKET pPacket;
	BOOLEAN opmode = pAd->OpMode;

	pPacket = mpdu->pPacket;

	if (mpdu->bAMSDU && tr_ctl->damsdu_type == RX_SW_AMSDU)
		announce_non_hw_damsdu_pkt(pAd, pPacket, mpdu->OpMode);
	else {
		/* pass this 802.3 packet to upper layer or forward this packet to WM directly */
		announce_or_forward_802_3_pkt(pAd, pPacket,
				wdev_search_by_idx(pAd, RTMP_GET_PACKET_WDEV(pPacket)), opmode);
	}
}

static void ba_mpdu_blk_free(struct ba_control *ba_ctl, struct reordering_mpdu *mpdu_blk)
{
	if (!mpdu_blk)
		return;
	NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[mpdu_blk->band].lock);
	ba_enqueue_head(&ba_ctl->mpdu_blk_pool[mpdu_blk->band].freelist, mpdu_blk);
	NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[mpdu_blk->band].lock);
}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
static inline void ba_indicate_reordering_mpdus_blk(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		struct BA_REC_ENTRY *pBAEntry,
		u16 idx)
{
	struct reordering_mpdu **reorder_buf = pBAEntry->reorder_buf;
	struct reordering_mpdu *blk = NULL;
	struct reordering_mpdu *next_blk = NULL;

	if (reorder_buf[idx]) {
		do {
			if (next_blk)
				blk = next_blk;
			else
				blk = reorder_buf[idx];

			next_blk = blk->next;

			if (pAd)
				announce_ba_reorder_pkt(pAd, blk);
			else
				RELEASE_NDIS_PACKET(
					NULL, blk->pPacket, NDIS_STATUS_FAILURE);
			ba_mpdu_blk_free(ba_ctl, blk);
			if (pBAEntry->stored_mpdu_num > 0)
				pBAEntry->stored_mpdu_num--;
		} while (next_blk);
		reorder_buf[idx] = NULL;
	}
}

static inline void __ba_refresh_reordering_mpdus(RTMP_ADAPTER *pAd, struct ba_control *ba_ctl, struct BA_REC_ENTRY *pBAEntry)
{
	u16 i = pBAEntry->head_index;
	u16 head_sn = pBAEntry->head_sn;
	u16 last_in_sn = INVALID_RCV_SEQ;

	while (pBAEntry->stored_mpdu_num > 0) {
		ba_indicate_reordering_mpdus_blk(pAd, ba_ctl, pBAEntry, i);
		head_sn = SEQ_INC(head_sn, 1, MAXSEQ);
		IDX_INC(i, pBAEntry->BAWinSize);

		/* Traverse one round */
		if (i == pBAEntry->head_index) {
			MTWF_PRINT("%s: stored_mpdu_num(%d), but reorder_buf is null\n",
				__func__, pBAEntry->stored_mpdu_num);
			break;
		}
	}

	last_in_sn = (head_sn == 0) ? MAXSEQ : (head_sn - 1);
	update_last_in_sn(pBAEntry, last_in_sn);
	if (pBAEntry->stored_mpdu_num > 0) {
		MTWF_PRINT("%s: stored_mpdu_num(%d) is not 0\n\n", __func__, pBAEntry->stored_mpdu_num);
		pBAEntry->stored_mpdu_num = 0;
	}
	pBAEntry->CurMpdu = NULL;
}
#else
static inline void __ba_refresh_reordering_mpdus(RTMP_ADAPTER *pAd,
	struct ba_control *ba_ctl, struct BA_REC_ENTRY *pBAEntry)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk;

	/* dequeue in-order frame from reodering list */
	while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list))) {
		announce_ba_reorder_pkt(pAd, mpdu_blk);

		while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
			announce_ba_reorder_pkt(pAd, msdu_blk);
			ba_mpdu_blk_free(ba_ctl, msdu_blk);
		}

		update_last_in_sn(pBAEntry, mpdu_blk->Sequence);
		ba_mpdu_blk_free(ba_ctl, mpdu_blk);
		/* update last indicated sequence */
	}

	ASSERT(pBAEntry->list.qlen == 0);
	pBAEntry->CurMpdu = NULL;
}
#endif

static void ba_refresh_reordering_mpdus(RTMP_ADAPTER *pAd,
	struct ba_control *ba_ctl, struct BA_REC_ENTRY *pBAEntry)
{
	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	__ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}


static VOID ba_ori_session_setup_timeout(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	struct BA_ORI_ENTRY *pBAEntry = (struct BA_ORI_ENTRY *)FunctionContext;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_INFO *ba_info;
	RTMP_ADAPTER *pAd;
	BOOLEAN sta_is_ps = FALSE;

	if (pBAEntry == NULL)
		return;

	if (pBAEntry->ORI_BA_Status == Originator_Done)
		return;

	pEntry = pBAEntry->pEntry;
	if (pEntry == NULL)
		return;

	if (pEntry->PsMode != PWR_ACTIVE)
		sta_is_ps = TRUE;

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();
	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;
		MAC_TABLE_ENTRY *setup_pEntry;
		MAC_TABLE_ENTRY *entry_ptr;
		int i = 0;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry)
			goto end;

		setup_pEntry = mld_entry_link_select(mld_entry);
		if (!IS_VALID_ENTRY(setup_pEntry))
			goto end;

		pBAEntry->pEntry = setup_pEntry;
		pEntry = setup_pEntry;
		ba_info = &mld_entry->ba_info;

		for (i = 0; i < MLD_LINK_MAX; i++) {
			entry_ptr = mld_entry->link_entry[i];
			if (!entry_ptr)
				continue;
			if (entry_ptr->PsMode == PWR_ACTIVE) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
					"MLO link %d (%pM) active!\n", i, entry_ptr->Addr);
				sta_is_ps = FALSE;
				break;
			}
		}
	}
#endif /* DOT11_EHT_BE */

	pAd = pEntry->pAd;
	if (pEntry->wdev == NULL) {
		/* Do not enque wdev if entry is NULL */
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"wdev is NULL\n");
		goto end;
	}

	if ((pBAEntry->ORI_BA_Status == Originator_WaitRes)
		&& (pBAEntry->Token < ORI_SESSION_MAX_RETRY)
		&& !(ba_info->PolicyNotSupBitmap & (1 << pBAEntry->TID)) && !sta_is_ps) {
		MLME_ADDBA_REQ_STRUCT AddbaReq;
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			PSTA_ADMIN_CONFIG pStaCfg = NULL;
			pStaCfg = GetStaCfgByWdev(pAd, pEntry->wdev);

			if (pStaCfg && INFRA_ON(pStaCfg) &&
				RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
				(STA_STATUS_TEST_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED))) {
				/* In scan progress and have no chance to send out, just re-schedule to another time period */
				RTMPSetTimer(&pBAEntry->ORIBATimer, ORI_BA_SESSION_TIMEOUT);
				goto end;
			}
		}

#endif /* CONFIG_STA_SUPPORT */
		NdisZeroMemory(&AddbaReq, sizeof(AddbaReq));
		COPY_MAC_ADDR(AddbaReq.pAddr, pEntry->Addr);
		AddbaReq.Wcid = pEntry->wcid;
		AddbaReq.TID = pBAEntry->TID;
		AddbaReq.BaBufSize = pBAEntry->BAWinSize;
		AddbaReq.TimeOutValue = pBAEntry->TimeOutValue;
		AddbaReq.Token = pBAEntry->Token;
		AddbaReq.amsdu_support = pBAEntry->amsdu_cap;

		MlmeEnqueueWithWdev(pAd, ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE,
			sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq, 0, pEntry->wdev, FALSE, NULL);
		RTMP_MLME_HANDLER(pAd);

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"Wcid=%d,TID=%d,Ori ADDBA Timeout(%d):Send again\n",
			pEntry->wcid, pBAEntry->TID, pBAEntry->Token);
		pBAEntry->Token++;
		RTMPSetTimer(&pBAEntry->ORIBATimer, ORI_BA_SESSION_TIMEOUT);
	} else {
		/* either not in the right state or exceed retry count */
		ba_resrc_ori_del(pAd, pEntry->wcid, pBAEntry->TID);
	}
end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
}
BUILD_TIMER_FUNCTION(ba_ori_session_setup_timeout);

#ifndef CONFIG_BA_REORDER_ARRAY_SUPPORT
static BOOLEAN ba_reordering_mpdu_insertsorted(struct reordering_list *list,
										struct reordering_mpdu *mpdu)
{
	struct reordering_mpdu **ppScan = &list->next;

	while (*ppScan != NULL) {
		if (SEQ_SMALLER((*ppScan)->Sequence, mpdu->Sequence, MAXSEQ))
			ppScan = &(*ppScan)->next;
		else if ((*ppScan)->Sequence == mpdu->Sequence) {
			/* give up this duplicated frame */
			return FALSE;
		} else {
			/* find position */
			break;
		}
	}

	if (*ppScan == NULL)
		list->tail = mpdu;

	mpdu->next = *ppScan;
	*ppScan = mpdu;
	list->qlen++;
	return TRUE;
}
#endif

static VOID ba_resource_dump_sn(struct ba_control *ba_ctl, struct BA_REC_ENTRY *pRecBAEntry)
{
	UINT j;
	struct ba_rec_debug *dbg;
	UINT k = pRecBAEntry->ba_rec_dbg_idx;

	if (!pRecBAEntry->ba_rec_dbg ||
	    !(pRecBAEntry->REC_BA_Status == Recipient_Established ||
	      pRecBAEntry->REC_BA_Status == Recipient_Initialization))
		return;

	for (j = k; j < BA_REC_DBG_SIZE; j++) {
		dbg = &pRecBAEntry->ba_rec_dbg[j];
		if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
			MTWF_PRINT("idx(%d), ta("MACSTR"), ra("MACSTR"), ",
				j, MAC2STR(dbg->ta), MAC2STR(dbg->ra));
			MTWF_PRINT("sn(%d), amsdu(%d), type(%d), last_in_seq%d\n",
				dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
		} else {
			MTWF_PRINT("idx(%d), sn(%d), amsdu(%d), type(%d), last_in_seq%d\n", j,
				dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
		}
	}

	for (j = 0; j < k; j++) {
		dbg = &pRecBAEntry->ba_rec_dbg[j];
		if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
			MTWF_PRINT("idx(%d), ta("MACSTR"), ra("MACSTR"), ",
				j, MAC2STR(dbg->ta), MAC2STR(dbg->ra));
			MTWF_PRINT("sn(%d), amsdu(%d), type(%d), last_in_seq%d\n",
				dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
		} else {
			MTWF_PRINT("idx(%d), sn(%d), amsdu(%d), type(%d), last_in_seq%d\n", j,
				dbg->sn, dbg->amsdu, dbg->type, dbg->last_in_seq);
		}
	}
}

VOID ba_resource_dump_all(RTMP_ADAPTER *pAd, ULONG second_idx)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	INT i, j;
	struct BA_ORI_ENTRY *pOriBAEntry;
	struct BA_REC_ENTRY *pRecBAEntry;
	struct BA_INFO *ba_info;
	RTMP_STRING tmpBuf[10];
	int ret;
	struct _STA_TR_ENTRY *tr_entry;
	struct seq_ctrl_t *seq_ctrl = NULL;
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
	char rro_type[] = "ARRAY";
#else
	struct reordering_mpdu *mpdu_blk = NULL, *msdu_blk = NULL;
	char rro_type[] = "LIST";
#endif

	MTWF_PRINT("%s: reorder algo is %s\n", __func__, rro_type);

	NdisAcquireSpinLock(&ba_ctl->BATabLock);
	MTWF_PRINT("Originator=%ld, Recipient=%ld\n", ba_ctl->numAsOriginator, ba_ctl->numAsRecipient);
	NdisReleaseSpinLock(&ba_ctl->BATabLock);

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		MAC_TABLE_ENTRY *pEntry = entry_get(pAd, i);

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;

		if (IS_ENTRY_PEER_AP(pEntry)) {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "ApCli");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (IS_ENTRY_REPEATER(pEntry)) {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "Repeater");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (IS_ENTRY_WDS(pEntry)) {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "WDS");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else {
			ret = snprintf(tmpBuf, sizeof(tmpBuf), "%s", "STA");
			if (os_snprintf_error(sizeof(tmpBuf), ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}

		ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();

		if (IS_ENTRY_MLO(pEntry) && pEntry->mlo.is_setup_link_entry) {
			struct mld_entry_t *mld_entry;
			struct mld_entry_ext_t *mld_entry_ext = NULL;

			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (!mld_entry) {
				mt_rcu_read_unlock();
				continue;
			}
			ba_info = &mld_entry->ba_info;
			mld_entry_ext = mld_entry_ext_get_by_idx(pAd, mld_entry->mld_sta_idx);
			if (mld_entry_ext)
				seq_ctrl = &mld_entry_ext->tr_entry.seq_ctrl;
		} else
#endif /* DOT11_EHT_BE */
		{
			tr_entry = tr_entry_get(pAd, pEntry->wcid);
			seq_ctrl = &tr_entry->seq_ctrl;
		}

		MTWF_PRINT("\n"MACSTR" (Aid=%d wcid=%d) (%s) -\n", MAC2STR(pEntry->Addr), pEntry->Aid, pEntry->wcid, tmpBuf);
		MTWF_PRINT("[Originator]\n");

		for (j = 0; j < NUM_OF_TID; j++) {
			UINT16 Idx = ba_info->OriWcidArray[j];

			if (Idx != 0) {
				pOriBAEntry = &ba_ctl->BAOriEntry[Idx];
				if (pOriBAEntry->ORI_BA_Status == Originator_Done)
					MTWF_PRINT(
					"mac="MACSTR",Idx=%d,TID=%d,BAWinSize=%d,SSN=%d,Seq=%d\n",
					MAC2STR(pEntry->Addr), Idx, j,
					pOriBAEntry->BAWinSize,
					pOriBAEntry->Sequence,
					seq_ctrl ? seq_ctrl->TxSeq[j] : 0);
			}
		}

		MTWF_PRINT("\n");
		MTWF_PRINT("[Recipient]\nSetTime=%d FreeMpduBls : ", atomic_read(&ba_ctl->SetFlushTimer));

		for (j = 0; j < CFG_WIFI_RAM_BAND_NUM; j++)
			MTWF_PRINT("band%d=%d. ", j, ba_ctl->mpdu_blk_pool[j].freelist.qlen);
		MTWF_PRINT("\n");

		for (j = 0; j < NUM_OF_TID; j++) {
			UINT16 Idx = ba_info->RecWcidArray[j];

			if (Idx != 0) {
				pRecBAEntry = &ba_ctl->BARecEntry[Idx];

				NdisAcquireSpinLock(&pRecBAEntry->RxReRingLock);
				if ((pRecBAEntry->REC_BA_Status == Recipient_Established) ||
						(pRecBAEntry->REC_BA_Status == Recipient_Initialization) ||
						(pRecBAEntry->REC_BA_Status == Recipient_Offload)) {
					MTWF_PRINT("State=%d, TID=%d, BAWinSize=%d, Se_id=%d, ",
						pRecBAEntry->REC_BA_Status, j, pRecBAEntry->BAWinSize, pRecBAEntry->Session_id);
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
					MTWF_PRINT("LastIndSeq=%d, ReorderingPkts=%d\n",
						pRecBAEntry->LastIndSeq, pRecBAEntry->stored_mpdu_num);
#else
					MTWF_PRINT("LastIndSeq=%d, ReorderingPkts=%d\n",
						pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen);
#endif
					MTWF_PRINT("\t\tDrop pkts: duplicated=%ld, old=%ld, ",
						pRecBAEntry->drop_dup_pkts,
						pRecBAEntry->drop_old_pkts);
					MTWF_PRINT("unknown_state=%ld. SN_Surpass_pkts=%ld\n",
						pRecBAEntry->drop_unknown_state_pkts,
						pRecBAEntry->ba_sn_large_win_end);
					MTWF_PRINT("\t\ticverr=%ld errflag=%ld pnchkfail=%ld ",
						pRecBAEntry->icv_err_cnt,
						pRecBAEntry->flag_err_cnt,
						pEntry->PNChkFailCnt);
					MTWF_PRINT("reset_rro=%ld DisPNChk=%ld",
						pRecBAEntry->RRO_RESET_CNT,
						(pEntry->DISABLE_PN_CHK & BIT(j)));
					MTWF_PRINT("CCMP_ICV_ERROR=%d HAS_ICV_ERROR=%d\n",
						pRecBAEntry->CCMP_ICV_ERROR,
						pRecBAEntry->HAS_ICV_ERROR);

					if (second_idx == SN_HISTORY)
						ba_resource_dump_sn(ba_ctl, pRecBAEntry);
				}
				NdisReleaseSpinLock(&pRecBAEntry->RxReRingLock);
			}
		}

		MTWF_PRINT("\n");
		MTWF_PRINT("[RX ReorderBuffer]\n");
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
		for (j = 0; j < NUM_OF_TID; j++) {
			UINT16 Idx = ba_info->RecWcidArray[j];

			if (Idx != 0) {
				pRecBAEntry = &ba_ctl->BARecEntry[Idx];
				dump_ba_list(pRecBAEntry);
			}
		}

#else
		for (j = 0; j < NUM_OF_TID; j++) {
			UINT16 Idx = ba_info->RecWcidArray[j];

			if (Idx != 0) {
				pRecBAEntry = &ba_ctl->BARecEntry[Idx];
				mpdu_blk = ba_reordering_mpdu_probe(&pRecBAEntry->list);

				if (mpdu_blk) {
					MTWF_PRINT("mpdu:SN = %d, AMSDU = %d\n", mpdu_blk->Sequence, mpdu_blk->bAMSDU);
					msdu_blk = ba_reordering_mpdu_probe(&mpdu_blk->AmsduList);

					if (msdu_blk) {
						MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);
						while (msdu_blk->next) {
							msdu_blk = msdu_blk->next;
							MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);
						}
					}

					while (mpdu_blk->next) {
						mpdu_blk = mpdu_blk->next;
						MTWF_PRINT("mpdu:SN = %d, AMSDU = %d\n", mpdu_blk->Sequence, mpdu_blk->bAMSDU);
						msdu_blk = ba_reordering_mpdu_probe(&mpdu_blk->AmsduList);

						if (msdu_blk) {
							MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);

							while (msdu_blk->next) {
								msdu_blk = msdu_blk->next;
								MTWF_PRINT("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence, msdu_blk->bAMSDU);
							}
						}
					}
				}
			}
		}
#endif
#ifdef DOT11_EHT_BE
		mt_rcu_read_unlock();
#endif
	}
	return;
}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
VOID ba_reordering_resource_dump_all(RTMP_ADAPTER *pAd)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_REC_ENTRY *pBAEntry;
	int i;
	UINT32 total_pkt_cnt = 0;

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &ba_ctl->BARecEntry[i];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		total_pkt_cnt += (pBAEntry->stored_mpdu_num);
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

	MTWF_PRINT("total %d msdu packt in ba list\n", total_pkt_cnt);
}

VOID ba_reodering_resource_dump(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	int i, j;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_REC_ENTRY *pBAEntry;
	struct BA_INFO *ba_info;
	UINT32 total_pkt_cnt = 0;

	if (!(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return;

	pEntry = &pAd->MacTab->Content[wcid];
	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return;
		}
		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	for (i = 0; i < NUM_OF_TID; i++) {

		j = ba_info->RecWcidArray[i];
		if (j == 0)
			continue;

		pBAEntry = &ba_ctl->BARecEntry[j];

		if (pBAEntry)
			total_pkt_cnt += (pBAEntry->stored_mpdu_num);
	}
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif

	MTWF_PRINT("total %d msdu packt in wcid (%d) ba list\n", total_pkt_cnt, wcid);
}

#else
VOID ba_reordering_resource_dump_all(RTMP_ADAPTER *pAd)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_REC_ENTRY *pBAEntry;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	int i;
	UINT32 total_pkt_cnt = 0;

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &ba_ctl->BARecEntry[i];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if (pBAEntry->list.next) {
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk) {
				if (mpdu_blk->AmsduList.next) {
					msdu_blk = mpdu_blk->AmsduList.next;
					while (msdu_blk) {
						msdu_blk = msdu_blk->next;
						total_pkt_cnt++;
					}
				}
				total_pkt_cnt++;
				mpdu_blk = mpdu_blk->next;
			}
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

	MTWF_PRINT("total %d msdu packt in ba list\n", total_pkt_cnt);
}

VOID ba_reodering_resource_dump(RTMP_ADAPTER *pAd, UINT16 wcid)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	int i, j;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_REC_ENTRY *pBAEntry;
	struct BA_INFO *ba_info;
	UINT32 total_pkt_cnt = 0;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;

	if (!(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return;

	pEntry = entry_get(pAd, wcid);
	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return;
		}
		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	for (i = 0; i < NUM_OF_TID; i++) {

		j = ba_info->RecWcidArray[i];
		if (j == 0)
			continue;

		pBAEntry = &ba_ctl->BARecEntry[j];

		if (pBAEntry->list.next) {
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk) {
				if (mpdu_blk->AmsduList.next) {
					msdu_blk = mpdu_blk->AmsduList.next;
					while (msdu_blk) {
						msdu_blk = msdu_blk->next;
						total_pkt_cnt++;
					}
				}
				total_pkt_cnt++;
				mpdu_blk = mpdu_blk->next;
			}
		}
	}

#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	MTWF_PRINT("total %d msdu packt in wcid (%d) ba list\n", total_pkt_cnt, wcid);
}
#endif

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
void ba_clean_reorder_buf(struct BA_REC_ENTRY *pBAEntry, struct ba_control *ba_ctl)
{
	u16 idx;

	for (idx = 0; idx < CHIP_BA_WINSIZE; idx++)
		ba_indicate_reordering_mpdus_blk(NULL, ba_ctl, pBAEntry, idx);

	if (pBAEntry->stored_mpdu_num) {
		MTWF_PRINT("%s: stored_mpdu_num(%d) != 0\n", __func__, pBAEntry->stored_mpdu_num);
		pBAEntry->stored_mpdu_num = 0;
	}
}
#endif

/* free all resource for reordering mechanism */
void ba_reordering_resource_release(struct physical_device *ph_dev)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ph_dev);
	struct BA_REC_ENTRY *pBAEntry;
	int i;
	BOOLEAN Cancelled;
#ifndef CONFIG_BA_REORDER_ARRAY_SUPPORT
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
#endif

	if (atomic_read(&ba_ctl->SetFlushTimer) != -1) {
		RTMPCancelTimer(&ba_ctl->FlushTimer, &Cancelled);
		RTMPReleaseTimer(&ba_ctl->FlushTimer, &Cancelled);
		atomic_set(&ba_ctl->SetFlushTimer, -1);
	}

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &ba_ctl->BARecEntry[i];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

		if (pBAEntry->REC_BA_Status != Recipient_NONE) {
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
			ba_clean_reorder_buf(pBAEntry, ba_ctl);
#else
			while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list))) {
				while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
					RELEASE_NDIS_PACKET(
						NULL, msdu_blk->pPacket, NDIS_STATUS_FAILURE);
					ba_mpdu_blk_free(ba_ctl, msdu_blk);
				}

				ASSERT(mpdu_blk->pPacket);
				RELEASE_NDIS_PACKET(
					NULL, mpdu_blk->pPacket, NDIS_STATUS_FAILURE);
				ba_mpdu_blk_free(ba_ctl, mpdu_blk);
			}
#endif
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

#ifndef CONFIG_BA_REORDER_ARRAY_SUPPORT
	ASSERT(pBAEntry->list.qlen == 0);
#endif
	/* II. free memory of reordering mpdu table */
	for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++) {
		NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
		if (ba_ctl->mpdu_blk_pool[i].mem != NULL)
			os_free_mem(ba_ctl->mpdu_blk_pool[i].mem);
		NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
	}
}

BOOLEAN ba_reordering_resource_init(struct physical_device *ph_dev, int num)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ph_dev);
	int i, j;
	PUCHAR mem;
	struct reordering_mpdu *mpdu_blk;
	struct reordering_list *freelist;

	for (i = 0; i < MAX_BAND_NUM; i++) {
		/* allocate spinlock */
		OS_NdisAllocateSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);

		NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
		/* initialize freelist */
		freelist = &ba_ctl->mpdu_blk_pool[i].freelist;
		freelist->next = NULL;
		freelist->qlen = 0;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"Allocate %d memory for BA reordering\n",
			(UINT32)(num * sizeof(struct reordering_mpdu)));
		/* allocate number of mpdu_blk memory */
		os_alloc_mem(NULL, (PUCHAR *)&mem, (num * sizeof(struct reordering_mpdu)));
		ba_ctl->mpdu_blk_pool[i].mem = mem;

		if (mem == NULL) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"Can't Allocate Memory for BA Reordering\n");
			NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
			return FALSE;
		}


		/* build mpdu_blk free list */
		for (j = 0; j < num; j++) {
			/* get mpdu_blk */
			mpdu_blk = (struct reordering_mpdu *)mem;
			/* initial mpdu_blk */
			NdisZeroMemory(mpdu_blk, sizeof(struct reordering_mpdu));
			/* next mpdu_blk */
			mem += sizeof(struct reordering_mpdu);
			/* insert mpdu_blk into freelist */
			ba_enqueue_head(freelist, mpdu_blk);
		}
		NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[i].lock);
	}

	return TRUE;
}

static struct reordering_mpdu *ba_mpdu_blk_alloc(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk)
{
	struct reordering_mpdu *mpdu_blk;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);

	NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[rx_blk->band].lock);
	mpdu_blk = ba_dequeue_head(&ba_ctl->mpdu_blk_pool[rx_blk->band].freelist);

	if (mpdu_blk) {
#ifndef CONFIG_BA_REORDER_ARRAY_SUPPORT
		NdisZeroMemory(mpdu_blk, sizeof(*mpdu_blk));
#endif
		mpdu_blk->band = rx_blk->band;
	}

	NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[rx_blk->band].lock);
	return mpdu_blk;
}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
static USHORT __ba_indicate_reordering_mpdus_in_order(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		struct BA_REC_ENTRY *pBAEntry,
		USHORT StartSeq)
{
	u16 LastIndSeq = INVALID_RCV_SEQ, i = 0;
	struct reordering_mpdu **reorder_buf = pBAEntry->reorder_buf;
	struct reordering_mpdu *blk = NULL;

	i = (((StartSeq + 1) & MAXSEQ) % pBAEntry->BAWinSize);

	if ((pBAEntry->stored_mpdu_num == 0) &&	(reorder_buf[i])) {
		blk = reorder_buf[i];
		MTWF_PRINT("%s: suspicious reorder_buf[%d]=%p, SSN:%d(%d), tid:%d, caller %pS\n",
				__func__, i, reorder_buf[i], StartSeq, blk->Sequence,
				pBAEntry->TID, __builtin_return_address(0));
		ba_indicate_reordering_mpdus_blk(pAd, ba_ctl, pBAEntry, i);
		goto err_out;
	}

	while (reorder_buf[i]) {
		/* pass this frame up */
		ba_indicate_reordering_mpdus_blk(pAd, ba_ctl, pBAEntry, i);
		StartSeq = SEQ_INC(StartSeq, 1, MAXSEQ);
		IDX_INC(i, pBAEntry->BAWinSize);
		LastIndSeq = StartSeq;
	}

err_out:
	return LastIndSeq;
}
static USHORT ba_indicate_reordering_mpdus_in_order(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		struct BA_REC_ENTRY *pBAEntry,
		USHORT StartSeq)
{
	USHORT last_in_seq;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	last_in_seq = __ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, StartSeq);
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	return last_in_seq;
}


static void __ba_indicate_reordering_mpdus_le_seq(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		struct BA_REC_ENTRY *pBAEntry,
		USHORT Sequence)
{
	u16 head_sn = pBAEntry->head_sn;
	u16 i = pBAEntry->head_index;

	while ((pBAEntry->stored_mpdu_num > 0) &&
		((head_sn == Sequence) || SEQ_SMALLER(head_sn, Sequence, MAXSEQ))) {
		//i = (head_sn % pBAEntry->BAWinSize);
		ba_indicate_reordering_mpdus_blk(pAd, ba_ctl, pBAEntry, i);
		head_sn = SEQ_INC(head_sn, 1, MAXSEQ);
		IDX_INC(i, pBAEntry->BAWinSize);
	}
}

static void ba_indicate_reordering_mpdus_le_seq(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		struct BA_REC_ENTRY *pBAEntry,
		USHORT Sequence)
{
	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	__ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, Sequence);
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}


#else
static USHORT ba_indicate_reordering_mpdus_in_order(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		struct BA_REC_ENTRY *pBAEntry,
		USHORT StartSeq)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	USHORT  LastIndSeq = INVALID_RCV_SEQ;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list))) {
		/* find in-order frame */
		if (!SEQ_STEPONE(mpdu_blk->Sequence, StartSeq, MAXSEQ))
			break;

		/* dequeue in-order frame from reodering list */
		mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
		/* pass this frame up */
		announce_ba_reorder_pkt(pAd, mpdu_blk);
		/* move to next sequence */
		StartSeq = mpdu_blk->Sequence;
		LastIndSeq = StartSeq;

		while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
			announce_ba_reorder_pkt(pAd, msdu_blk);
			ba_mpdu_blk_free(ba_ctl, msdu_blk);
		}

		/* free mpdu_blk */
		ba_mpdu_blk_free(ba_ctl, mpdu_blk);
	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	/* update last indicated sequence */
	return LastIndSeq;
}

static void ba_indicate_reordering_mpdus_le_seq(PRTMP_ADAPTER pAd,
		struct ba_control *ba_ctl,
		struct BA_REC_ENTRY *pBAEntry,
		USHORT Sequence)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk = NULL;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list))) {
		/* find in-order frame */
		if ((mpdu_blk->Sequence == Sequence) || SEQ_SMALLER(mpdu_blk->Sequence, Sequence, MAXSEQ)) {
			/* dequeue in-order frame from reodering list */
			mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
			/* pass this frame up */
			announce_ba_reorder_pkt(pAd, mpdu_blk);

			while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
				announce_ba_reorder_pkt(pAd, msdu_blk);
				ba_mpdu_blk_free(ba_ctl, msdu_blk);
			}

			/* free mpdu_blk */
			ba_mpdu_blk_free(ba_ctl, mpdu_blk);
		} else
			break;
	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}
#endif

#ifdef RRO_CHECK_RX
#ifdef RX_RPS_SUPPORT
void ba_timeout_flush_by_cpu(PRTMP_ADAPTER pAd)
{
	ULONG now;
	UINT32 idx0 = 0;
	UINT32 idx1 = 0;
	struct BA_REC_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	UINT32 cpu = smp_processor_id(), the_cpu = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisGetSystemUpTime(&now);

	for (idx0 = 0; idx0 < BA_TIMEOUT_BITMAP_LEN; idx0++) {
		idx1 = 0;
		while ((ba_ctl->ba_timeout_bitmap_per_cpu[cpu][idx0] != 0) && (idx1 < 32)) {
			if (ba_ctl->ba_timeout_bitmap_per_cpu[cpu][idx0] & 0x1) {
				pBAEntry = &ba_ctl->BARecEntry[(idx0 << 5) + idx1];
				the_cpu = cap->RxSwRpsCpuMap[((idx0) % cap->RxSwRpsNum)];

				if (cpu == the_cpu)
					ba_flush_reordering_timeout_mpdus(pAd, ba_ctl, pBAEntry, now);
			}

			ba_ctl->ba_timeout_bitmap_per_cpu[cpu][idx0] >>= 1;
			idx1++;
		}
	}

	ba_ctl->ba_timeout_check_per_cpu[cpu] = FALSE;
}

void ba_timeout_monitor_per_cpu(PRTMP_ADAPTER pAd)
{
	UINT32 idx = 0, cpu = 0;
	struct BA_REC_ENTRY *pBAEntry = NULL;
	ULONG now;
	BOOLEAN need_check[NR_CPUS] = {FALSE};
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);


	NdisGetSystemUpTime(&now);
	for (idx = 0; idx < MAX_LEN_OF_BA_REC_TABLE; idx++) {
		pBAEntry = &ba_ctl->BARecEntry[idx];
		cpu = cap->RxSwRpsCpuMap[((idx) % cap->RxSwRpsNum)];
		if (!ba_ctl->ba_timeout_check_per_cpu[cpu]) {
			if ((pBAEntry->REC_BA_Status == Recipient_Established)
					&& (pBAEntry->list.qlen > 0)) {
				if (RTMP_TIME_AFTER((unsigned long)now,
#ifdef IXIA_C50_MODE
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + pAd->ixia_ctl.BA_timeout)))
#else
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + REORDERING_PACKET_TIMEOUT)))
#endif
				{
					need_check[cpu] = TRUE;
					ba_ctl->ba_timeout_bitmap_per_cpu[cpu][(idx >> 5)] |= (1 << (idx % 32));
				}

			}
		}
	}

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (need_check[cpu])
			ba_ctl->ba_timeout_check_per_cpu[cpu] = need_check[cpu];

	}
}
#endif

void ba_timeout_flush(PRTMP_ADAPTER pAd)
{
	ULONG now;
	UINT32 idx0 = 0;
	UINT32 idx1 = 0;
	struct BA_REC_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
#ifdef RX_RPS_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#ifdef RX_RPS_SUPPORT
	if (cap->rx_qm_en)
		return ba_timeout_flush_by_cpu(pAd);
#endif
	NdisGetSystemUpTime(&now);

	for (idx0 = 0; idx0 < BA_TIMEOUT_BITMAP_LEN; idx0++) {
		idx1 = 0;
		while ((ba_ctl->ba_timeout_bitmap[idx0] != 0) && (idx1 < 32)) {
			if (ba_ctl->ba_timeout_bitmap[idx0] & 0x1) {
				pBAEntry = &ba_ctl->BARecEntry[(idx0 << 5) + idx1];
				ba_flush_reordering_timeout_mpdus(pAd, ba_ctl, pBAEntry, now);
			}

			ba_ctl->ba_timeout_bitmap[idx0] >>= 1;
			idx1++;
		}
	}

	ba_ctl->ba_timeout_check = FALSE;
}

void ba_timeout_monitor(PRTMP_ADAPTER pAd)
{
	UINT32 idx = 0;
	struct BA_REC_ENTRY *pBAEntry = NULL;
	ULONG now;
	BOOLEAN need_check = FALSE;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
#ifdef RX_RPS_SUPPORT
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#ifdef RX_RPS_SUPPORT
	if (cap->rx_qm_en)
		return ba_timeout_monitor_per_cpu(pAd);
#endif
	if (!ba_ctl->ba_timeout_check) {
		NdisGetSystemUpTime(&now);
		for (idx = 0; idx < MAX_LEN_OF_BA_REC_TABLE; idx++) {
			pBAEntry = &ba_ctl->BARecEntry[idx];
			if ((pBAEntry->REC_BA_Status == Recipient_Established)
					&& (pBAEntry->list.qlen > 0)) {

				if (RTMP_TIME_AFTER((unsigned long)now,
#ifdef IXIA_C50_MODE
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + pAd->ixia_ctl.BA_timeout)))
#else
				(unsigned long)(pBAEntry->LastIndSeqAtTimer + REORDERING_PACKET_TIMEOUT)))
#endif
				{
					need_check = TRUE;
					ba_ctl->ba_timeout_bitmap[(idx >> 5)] |= (1 << (idx % 32));
				}

			}
		}

		if (need_check) {
#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd)) {
				struct hdev_ctrl *hdev_ctrl = pAd->hdev_ctrl;
				struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(hdev_ctrl);
				struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;
				struct pci_schedule_task_ops *sched_ops = hif_chip->schedule_task_ops;

				if (IS_ASIC_CAP(pAd, fASIC_CAP_DLY_INT_LUMPED))
					sched_ops->schedule_rx_dly_done(&hif_chip->task_group);
				else
					sched_ops->schedule_rx_data_done(&hif_chip->task_group);
			}
#endif
			ba_ctl->ba_timeout_check = need_check;
		}
	}
}
#endif

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
void ba_flush_reordering_timeout_mpdus(PRTMP_ADAPTER pAd, struct BA_REC_ENTRY *pBAEntry)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct reordering_mpdu *mpdu_blk = NULL;
	struct reordering_mpdu **reorder_buf;
	INT sn, i;
	u16 idx;
	ULONG TimeOutTH = 0, TimeDelta = 0;
	struct sk_buff *skb = NULL;
#ifdef PROPRIETARY_DRIVER_SUPPORT
	struct timespec64 kts64 = {0};
	ktime_t kts;
#endif

	if ((pBAEntry == NULL) || (pBAEntry->stored_mpdu_num <= 0))
		return;

#ifdef IXIA_C50_MODE
	TimeOutTH = pAd->ixia_ctl.BA_timeout;
#else
	TimeOutTH = REORDERING_PACKET_TIMEOUT_IN_MS;
#endif

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	i = pBAEntry->BAWinSize;
	reorder_buf = pBAEntry->reorder_buf;
	idx = pBAEntry->head_index;
	while (i-- > 0) {
		mpdu_blk = reorder_buf[idx];
		if (!mpdu_blk) {
			IDX_INC(idx, pBAEntry->BAWinSize);
			continue;
		}
		skb = RTPKT_TO_OSPKT(mpdu_blk->pPacket);
#ifdef PROPRIETARY_DRIVER_SUPPORT
		ktime_get_real_ts64(&kts64);
		kts = timespec64_to_ktime(kts64);
		TimeDelta = ktime_to_ms(ktime_sub(kts, skb->tstamp));
#else
		TimeDelta = ktime_to_ms(net_timedelta(skb->tstamp));
#endif
		if (TimeDelta > TimeOutTH) {
			sn = SEQ_INC(mpdu_blk->Sequence, 1, MAXSEQ);
			__ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, sn);
			update_last_in_sn(pBAEntry, sn);
			sn = __ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, sn);
			if (sn != INVALID_RCV_SEQ)
				update_last_in_sn(pBAEntry, sn);
			idx = pBAEntry->head_index;
			pAd->tr_ctl.tr_cnt.ba_flush_one++;
#ifdef IXIA_C50_MODE
			pAd->rx_cnt.rx_flush_drop[pBAEntry->Wcid]++;
#endif
		} else
			break;

		if (pBAEntry->stored_mpdu_num == 0)
			break;
	}
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

	if (pBAEntry->stored_mpdu_num > 0 && atomic_read(&ba_ctl->SetFlushTimer) == 0) {
		RTMPSetTimer(&ba_ctl->FlushTimer, REORDERING_PACKET_TIMEOUT_IN_MS);
		atomic_set(&ba_ctl->SetFlushTimer, 1);
	}
}

#else
void ba_flush_reordering_timeout_mpdus(PRTMP_ADAPTER pAd, struct BA_REC_ENTRY *pBAEntry)
{
	struct reordering_mpdu *mpdu_blk = NULL;
	ULONG TimeOutTH = 0, TimeDelta = 0;
	struct sk_buff *skb = NULL;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
#ifdef PROPRIETARY_DRIVER_SUPPORT
	struct timespec64 kts64 = {0};
	ktime_t kts;
#endif

	if ((pBAEntry == NULL) || (pBAEntry->list.qlen <= 0))
		return;

#ifdef IXIA_C50_MODE
	TimeOutTH = pAd->ixia_ctl.BA_timeout;
#else
	TimeOutTH = REORDERING_PACKET_TIMEOUT_IN_MS;
#endif

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list))) {
		INT sn = mpdu_blk->Sequence;

		skb = RTPKT_TO_OSPKT(mpdu_blk->pPacket);
#ifdef PROPRIETARY_DRIVER_SUPPORT
		ktime_get_real_ts64(&kts64);
		kts = timespec64_to_ktime(kts64);
		TimeDelta = ktime_to_ms(ktime_sub(kts, skb->tstamp));
#else
		TimeDelta = ktime_to_ms(net_timedelta(skb->tstamp));
#endif
		if (TimeDelta > TimeOutTH) {
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, sn);
			update_last_in_sn(pBAEntry, sn);
			sn = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, sn);
			if (sn != INVALID_RCV_SEQ)
				update_last_in_sn(pBAEntry, sn);
			pAd->tr_ctl.tr_cnt.ba_flush_one++;
#ifdef IXIA_C50_MODE
			pAd->rx_cnt.rx_flush_drop[pBAEntry->pEntry->wcid]++;
#endif
			NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		} else
			break;
	}
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

	if (pBAEntry->list.qlen > 0 && atomic_read(&ba_ctl->SetFlushTimer) == 0) {
		RTMPSetTimer(&ba_ctl->FlushTimer, REORDERING_PACKET_TIMEOUT_IN_MS);
		atomic_set(&ba_ctl->SetFlushTimer, 1);
	}
}
#endif

VOID ba_reorder_timeout_Exec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct ba_control *ba_ctl = (struct ba_control *)FunctionContext;
	struct BA_REC_ENTRY *pBAEntry;
	MAC_TABLE_ENTRY *pEntry;
	RTMP_ADAPTER *pAd;
	INT i;

	atomic_set(&ba_ctl->SetFlushTimer, 0);
	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		if (ba_ctl->BARecEntry[i].REC_BA_Status >= Recipient_Established) {
			pBAEntry = &ba_ctl->BARecEntry[i];
			pEntry = pBAEntry->pEntry;
			if (!IS_VALID_ENTRY(pEntry)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
					"pEntry is INVALID\n");
				return;
			}

			pAd = pEntry->pAd;
			ba_flush_reordering_timeout_mpdus(pAd, pBAEntry);
		}
	}
}
static struct BA_ORI_ENTRY *ba_alloc_ori_entry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	struct BA_ORI_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);

	NdisAcquireSpinLock(&ba_ctl->BATabLock);

	if (ba_ctl->numAsOriginator >= (MAX_LEN_OF_BA_ORI_TABLE - 1)) {
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
		goto done;
	}

	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i = 1; i < MAX_LEN_OF_BA_ORI_TABLE; i++) {
		pBAEntry = &ba_ctl->BAOriEntry[i];
		if ((pBAEntry->ORI_BA_Status == Originator_NONE)) {
			ba_ctl->numAsOriginator++;
			pBAEntry->ORI_BA_Status = Originator_USED;
			*Idx = i;
			break;
		}
	}

	NdisReleaseSpinLock(&ba_ctl->BATabLock);

done:
	return pBAEntry;
}

/* 1). from wdev's setting or 2). from min(chip cap , phy mode) */
UINT16 ba_get_default_max_ba_wsize(
	struct wifi_dev *wdev, RTMP_ADAPTER *main_ad)
{
	UINT16 phy_mode = wdev->PhyMode;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (ad) { /* ref wdev's first & check phymode */
		struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(ad->hdev_ctrl);

		if (WMODE_CAP_BE(phy_mode) && PHY_CAP_BE(chip_cap->phy_caps))
			return min(hc_get_chip_max_ba_win_sz(ad->hdev_ctrl), (UINT16)BA_WIN_SZ_1024);
		else if (WMODE_CAP_AX(phy_mode) && PHY_CAP_AX(chip_cap->phy_caps))
			return min(hc_get_chip_max_ba_win_sz(ad->hdev_ctrl), (UINT16)BA_WIN_SZ_256);
		else
			return BA_WIN_SZ_64;
	} else if (main_ad) {
		/* wdev not init yet, force ref main wdev's ad, and use chip cap max */
		if (WMODE_CAP_BE(phy_mode))
			return min(hc_get_chip_max_ba_win_sz(main_ad->hdev_ctrl), (UINT16)BA_WIN_SZ_1024);
		else if (WMODE_CAP_AX(phy_mode))
			return min(hc_get_chip_max_ba_win_sz(main_ad->hdev_ctrl), (UINT16)BA_WIN_SZ_256);
		else
			return BA_WIN_SZ_64;
	} else
		return BA_WIN_SZ_64;
}

UINT16 cal_ori_ba_wsize(struct _MAC_TABLE_ENTRY *peer,
		UINT16 cfg_ori_wsize, UINT16 peer_rec_wsize)
{
	UINT16 ori_ba_wsize = 0;

	ori_ba_wsize = wlan_config_get_ba_tx_wsize(peer->wdev);

	/* peer recipient win size sanity check */
#ifdef DOT11_EHT_BE
	if (!IS_EHT_STA(peer->cap.modes)) {
		ori_ba_wsize = min(ori_ba_wsize,
				  (UINT16)MAX_HE_REORDERBUF);
		peer_rec_wsize = min(peer_rec_wsize,
				    (UINT16)MAX_HE_REORDERBUF);
	}
#endif
	if (!IS_HE_STA(peer->cap.modes)) {
		ori_ba_wsize = min(ori_ba_wsize,
				  (UINT16)MAX_HT_REORDERBUF);
		peer_rec_wsize = min(peer_rec_wsize,
				    (UINT16)MAX_HT_REORDERBUF);
	}

	/* if peer recipient win size is invalid, ignore it */
	if (peer_rec_wsize == 0)
		return ori_ba_wsize;

	/* intersection of ori and rec wsize if rec wsize is valid */
	ori_ba_wsize = min(peer_rec_wsize, ori_ba_wsize);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"addbarsp,(our)txba=%d,(peer)rxba=%d\n",
			ori_ba_wsize, peer_rec_wsize);

	return ori_ba_wsize;
}

static void recal_rec_ba_wsize(struct _MAC_TABLE_ENTRY *peer,
	UINT16 *rec_ba_wsize)
{
#ifdef DOT11_EHT_BE
	ULONG data_len;
	struct _RTMP_ADAPTER *pAd = peer->wdev->sys_handle;

	if (IS_EHT_STA(peer->cap.modes)) {
		/* MLO BA rec win size */
		chip_do_extra_action(pAd, NULL, peer->Addr,
			CHIP_EXTRA_ACTION_RX_BA_WSIZE_MLO, (UINT8 *)rec_ba_wsize, &data_len);
	}
#endif
}

UINT16 cal_rec_ba_wsize(struct _MAC_TABLE_ENTRY *peer,
		UINT16 cfg_rec_wsize, UINT16 peer_ori_wsize)
{
	struct wifi_dev *wdev = peer->wdev;
	UINT16 rec_ba_wsize;

	rec_ba_wsize = wlan_config_get_ba_rx_wsize(wdev);
	/* peer originator win size sanity check */
	if (peer_ori_wsize == 0)
		peer_ori_wsize = MAX_HT_REORDERBUF;
#ifdef DOT11_EHT_BE
	if (!IS_EHT_STA(peer->cap.modes)) {
		rec_ba_wsize = min(rec_ba_wsize,
				  (UINT16)MAX_HE_REORDERBUF);
		peer_ori_wsize = min(peer_ori_wsize,
				    (UINT16)MAX_HE_REORDERBUF);
	}
#endif
	if (!IS_HE_STA(peer->cap.modes)) {
		rec_ba_wsize = min(rec_ba_wsize,
				  (UINT16)MAX_HT_REORDERBUF);
		peer_ori_wsize = min(peer_ori_wsize,
				    (UINT16)MAX_HT_REORDERBUF);
	}

	/* intersection of ori and rec wsize */
	rec_ba_wsize = min(peer_ori_wsize, rec_ba_wsize);

	recal_rec_ba_wsize(peer, &rec_ba_wsize);

	/* SPEC define the buffer size of add ba resp should be at least 1 */
	if (rec_ba_wsize < 1)
		rec_ba_wsize = 1;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"[Peer is Originator]recv. add ba req, (peer:ori)tx_ba_wsize=%d, (our:rec)rx_ba_wsize=%d \n",
			peer_ori_wsize, rec_ba_wsize);

	return rec_ba_wsize;
}

static VOID ba_ori_session_setup(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR TID,
	USHORT TimeOut)
{
	struct BA_ORI_ENTRY *pBAEntry;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_INFO *ba_info;
	USHORT Idx;
	UCHAR tx_mode;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	UINT16 ori_ba_wsize = 0;
	UINT16 cfg_tx_ba_wsize = 0;
	ULONG DelayTime;
	UCHAR amsdu_en = 0;
	struct ppdu_caps *ppdu;
	struct wifi_dev *wdev;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "wcid = %d, tid = %d\n", wcid, TID);

	/* sanity check */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		/* Do nothing if monitor mode is on*/
		if (MONITOR_ON(pAd))
			return;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	if (TID >= NUM_OF_TID) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "Wrong TID %d!\n", TID);
		return;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = entry_get(pAd, wcid);
	if (!IS_VALID_ENTRY(pEntry)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			 "Invalid entry\n");
		return;
	}

	wdev = pEntry->wdev;
	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			 "Null wdev\n");
		return;
	}

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry)
			goto end;

		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	/* if this entry is limited to use legacy tx mode, it doesn't generate BA.  */
	tx_mode = RTMPStaFixedTxMode(pAd, pEntry);
	if (tx_mode == FIXED_TXMODE_CCK || tx_mode == FIXED_TXMODE_OFDM)
		goto end;

	/* parameter decision */
	cfg_tx_ba_wsize = wlan_config_get_ba_tx_wsize(wdev);
	ori_ba_wsize = cal_ori_ba_wsize(pEntry, cfg_tx_ba_wsize, 0);
	ppdu = (struct ppdu_caps *)wlan_config_get_ppdu_caps(wdev);
	amsdu_en = wlan_config_get_amsdu_en(wdev) && ppdu->tx_amsdu_support;

	/* resource management */
	if (!ba_resrc_ori_prep(pAd, wcid, TID, ori_ba_wsize, amsdu_en, TimeOut))
		goto end;

	/* Don't Support DELAY_BA */
	if (ba_info->PolicyNotSupBitmap & (1 << TID)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
			"STA%d, TID[%d], Err BA Policy\n", wcid, TID);
		goto end;
	}

	/* set timer to send add ba request */
	if (ba_info->DeclineBitmap & (1 << TID))
		DelayTime = 3000; /* request has been declined, try again after 3 secs*/
	else
		DelayTime = 10;

	Idx = ba_info->OriWcidArray[TID];
	pBAEntry = &ba_ctl->BAOriEntry[Idx];
	RTMPSetTimer(&pBAEntry->ORIBATimer, DelayTime);

end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
}

BOOLEAN ba_resrc_ori_prep(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 wcid, UCHAR TID, UINT16 ba_wsize, UCHAR amsdu_en, USHORT timeout)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_INFO *ba_info;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_ORI_ENTRY *pBAEntry;
	STA_TR_ENTRY *tr_entry;
	struct seq_ctrl_t *seq_ctrl = NULL;
	USHORT Idx;
	BOOLEAN Cancelled;
	BOOLEAN Status = TRUE;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return FALSE;

	pEntry = entry_get(pAd, wcid);
	tr_entry = tr_entry_get(pAd, pEntry->wcid);
	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;
		struct mld_entry_ext_t *mld_entry_ext = NULL;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			Status = FALSE;
			goto end;
		}
		ba_info = &mld_entry->ba_info;
		mld_entry_ext = mld_entry_ext_get_by_idx(pAd, mld_entry->mld_sta_idx);
		if (mld_entry_ext)
			seq_ctrl = &mld_entry_ext->tr_entry.seq_ctrl;
	} else
#endif /* DOT11_EHT_BE */
	{
		seq_ctrl = &tr_entry->seq_ctrl;
	}

	if (!seq_ctrl) {
		Status = FALSE;
		goto end;
	}

	Idx = ba_info->OriWcidArray[TID];
	if (Idx == 0)
		pBAEntry = ba_alloc_ori_entry(pAd, &Idx);
	else
		pBAEntry = &ba_ctl->BAOriEntry[Idx];

	if (!pBAEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"alloc BA session failed\n");
		Status = FALSE;
		goto end;
	}

	if (pBAEntry->ORI_BA_Status >= Originator_WaitRes) {
		if (ba_info->AutoTest)
			MTWF_PRINT(
				"<addba_info>: sta_mac="MACSTR", tid=%d, buffersize=%d\n",
				MAC2STR(pEntry->Addr), TID, ba_wsize);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"ori BA session already exist, status = %d\n",
				pBAEntry->ORI_BA_Status);
		Status = FALSE;
		goto end;
	}

	ba_info->OriWcidArray[TID] = Idx;
	pBAEntry->ORI_BA_Status = Originator_WaitRes;
	pBAEntry->BAWinSize = ba_wsize;
	pBAEntry->Sequence = BA_ORI_INIT_SEQ;
	pBAEntry->Token = 1;	/* (2008-01-21) Jan Lee recommends it - this token can't be 0*/
	pBAEntry->TID = TID;
	pBAEntry->TimeOutValue = timeout;
	pBAEntry->amsdu_cap = amsdu_en;
	pBAEntry->pEntry = pEntry;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"wcid=%d, tid=%d Idx=%d\n", wcid, TID, Idx);
	if (!(ba_info->TxBitmap & (1 << TID)))
		RTMPInitTimer(pAd, &pBAEntry->ORIBATimer,
				GET_TIMER_FUNCTION(ba_ori_session_setup_timeout),
				pBAEntry, FALSE);
	else
		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);

end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	return Status;
}

BOOLEAN ba_resrc_ori_add(
	IN RTMP_ADAPTER *pAd,
	IN UINT16 wcid, UCHAR TID, UINT16 ba_wsize, UCHAR amsdu_en, USHORT timeout)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	MAC_TABLE_ENTRY *pEntry = entry_get(pAd, wcid);
	struct BA_ORI_ENTRY  *pBAEntry;
	STA_TR_ENTRY *tr_entry;
	struct seq_ctrl_t *seq_ctrl = NULL;
	struct BA_INFO *ba_info = &pEntry->ba_info;
	USHORT Idx;
	BOOLEAN Cancelled;
	BOOLEAN Status = TRUE;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
	uint16_t mld_sta_idx;

	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_ext_t *mld_entry_ext = NULL;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			Status = FALSE;
			goto end;
		}
		mld_sta_idx = mld_entry->mld_sta_idx;
		ba_info = &mld_entry->ba_info;
		mld_entry_ext = mld_entry_ext_get_by_idx(pAd, mld_sta_idx);
		if (mld_entry_ext)
			seq_ctrl = &mld_entry_ext->tr_entry.seq_ctrl;
	} else
#endif /* DOT11_EHT_BE */
	{
		tr_entry = tr_entry_get(pAd, pEntry->wcid);
		seq_ctrl = &tr_entry->seq_ctrl;
	}

	if (!seq_ctrl) {
		Status = FALSE;
		goto end;
	}

	Idx = ba_info->OriWcidArray[TID];
	pBAEntry = &ba_ctl->BAOriEntry[Idx];
	if (Idx == 0) {
		Status = FALSE;
		goto end;
	}

	/* make sure originator is waiting for response, in case unsolicited add ba response is recieved. */
	if (pBAEntry->ORI_BA_Status == Originator_WaitRes) {
		pBAEntry->BAWinSize = ba_wsize;
		pBAEntry->TimeOutValue = timeout;
		pBAEntry->amsdu_cap = amsdu_en;
		pBAEntry->ORI_BA_Status = Originator_Done;
		NdisAcquireSpinLock(&ba_ctl->BATabLock);
		ba_ctl->numDoneOriginator++;
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
		/* reset sequence number */
		pBAEntry->Sequence = BA_ORI_INIT_SEQ;
		/* Set Bitmap flag.*/
		ba_info->TxBitmap |= (1 << TID);
		ba_info->DeclineBitmap &= ~(1 << TID);
		ba_info->PolicyNotSupBitmap &= ~(1 << TID);

		if (pBAEntry->amsdu_cap)
			ba_info->TxAmsduBitmap |= (1 << TID);
		else
			ba_info->TxAmsduBitmap &= ~(1 << TID);

		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);
		pBAEntry->ORIBATimer.TimerValue = 0;	/*pFrame->TimeOutValue;*/

#ifdef DOT11_EHT_BE
		if (mld_entry) {
			if (pEntry->wdev) {
				if (pEntry->wdev->wdev_type == WDEV_TYPE_AP)
					bss_mngr_mld_ba_add_to_asic(pEntry->wdev,
						mld_sta_idx, TID, pBAEntry->Sequence,
						pBAEntry->BAWinSize, BA_SESSION_ORI, pBAEntry->amsdu_cap);
#ifdef CONFIG_STA_SUPPORT
				else if (pEntry->wdev->wdev_type == WDEV_TYPE_STA)
					sta_mld_ba_add_to_asic(pEntry->wdev,
						pEntry, TID, pBAEntry->Sequence,
						pBAEntry->BAWinSize, BA_SESSION_ORI, pBAEntry->amsdu_cap);
#endif/*CONFIG_STA_SUPPORT*/
				}

		} else {
#endif
			RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, TID, pBAEntry->Sequence,
				pBAEntry->BAWinSize, BA_SESSION_ORI, pBAEntry->amsdu_cap);
#ifdef DOT11_EHT_BE
		}
#endif

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"Wcid=%d,TID=%d,Idx=%d,TXBAbitmap=%x,AMSDUCap=%d,WinSize=%d,TimeOut=%ld\n",
			 pEntry->wcid, TID, Idx, ba_info->TxBitmap, pBAEntry->amsdu_cap,
			 pBAEntry->BAWinSize, pBAEntry->ORIBATimer.TimerValue);

		if (pBAEntry->ORIBATimer.TimerValue)
			RTMPSetTimer(&pBAEntry->ORIBATimer, pBAEntry->ORIBATimer.TimerValue); /* in mSec */
	}

end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	return Status;
}

BOOLEAN ba_resrc_ori_del(
	RTMP_ADAPTER *pAd,
	UINT16 wcid, UCHAR tid)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct _MAC_TABLE_ENTRY *pEntry = entry_get(pAd, wcid);
	struct BA_INFO *ba_info = &pEntry->ba_info;
	struct BA_ORI_ENTRY *pBAEntry;
	UINT Idx;
	BOOLEAN Cancelled;
	BOOLEAN Status = TRUE;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
	uint16_t mld_sta_idx;

	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			Status = FALSE;
			goto end;
		}
		mld_sta_idx = mld_entry->mld_sta_idx;
		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	Idx = ba_info->OriWcidArray[tid];
	if (Idx == 0) {
		Status = FALSE;
		goto end;
	}

	pBAEntry = &ba_ctl->BAOriEntry[Idx];
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"Idx=%d, Wcid=%d, TID=%d, ORI_BA_Status=%d\n",
		Idx, wcid, tid, pBAEntry->ORI_BA_Status);


#ifdef DOT11_EHT_BE
	if (mld_entry) {
		if (pEntry->wdev) {
			if (pEntry->wdev->wdev_type == WDEV_TYPE_AP)
				bss_mngr_mld_ba_del_from_asic(pEntry->wdev,
					mld_sta_idx, tid, BA_SESSION_ORI, 0);
#ifdef CONFIG_STA_SUPPORT
			else if (pEntry->wdev->wdev_type == WDEV_TYPE_STA)
				sta_mld_ba_del_from_asic(pEntry->wdev,
					pEntry, tid, BA_SESSION_ORI, 0);
#endif/*CONFIG_STA_SUPPORT*/
		}
	} else {
#endif /* DOT11_EHT_BE */
		RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, wcid, tid, BA_SESSION_ORI, 0);
#ifdef DOT11_EHT_BE
	}
#endif

	RTMPReleaseTimer(&pBAEntry->ORIBATimer, &Cancelled);
	ba_free_ori_entry(pAd, pBAEntry);
	ba_info->OriWcidArray[tid] = 0;
	ba_info->TxBitmap &= (~(1 << (tid)));

end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	return Status;
}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
static struct ba_buf *allocate_ba_buf(struct ba_control *ba_ctl)
{
	u32 buf_id;
	struct ba_buf *buf;

	if (free_buf_id_head == free_buf_id_tail) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"%s: no free ba buf id(%u)\n", __func__, free_buf_id_head);
		return NULL;
	}

	buf_id = ba_ctl->free_ba_buf_id[free_buf_id_head];
	ba_ctl->free_ba_buf_id[free_buf_id_head] = BUF_FREE_ID_INVALID;
	IDX_INC(free_buf_id_head, BUF_FREE_ID_SIZE);
	buf = &ba_ctl->free_ba_buf[buf_id];
	os_zero_mem(buf, sizeof(struct ba_buf));
	buf->id = buf_id;

	return buf;
}

static void release_ba_buf(struct ba_control *ba_ctl, struct ba_buf *buf)
{
	u16 id = buf->id;

	ba_ctl->free_ba_buf_id[free_buf_id_tail] = id;
	IDX_INC(free_buf_id_tail, BUF_FREE_ID_SIZE);
}
#endif

static struct BA_REC_ENTRY *ba_alloc_rec_entry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	struct BA_REC_ENTRY *pBAEntry = NULL;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
	struct ba_buf *ba_buf = NULL;
#endif

	NdisAcquireSpinLock(&ba_ctl->BATabLock);

	if (ba_ctl->numAsRecipient >= (MAX_LEN_OF_BA_REC_TABLE - 1)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BA Recipeint Session (%ld) > %d\n",
				 ba_ctl->numAsRecipient, (MAX_LEN_OF_BA_REC_TABLE - 1));
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
		goto done;
	}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
	ba_buf = allocate_ba_buf(ba_ctl);
	if (ba_buf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"%s: allocate ba buf failed\n", __func__);
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
		goto done;
	}
#endif

	if (atomic_read(&ba_ctl->SetFlushTimer) == -1) {
		RTMPInitTimer(pAd, &ba_ctl->FlushTimer, GET_TIMER_FUNCTION(ba_reorder_timeout_Exec), ba_ctl, FALSE);
		atomic_set(&ba_ctl->SetFlushTimer, 0);
	}

	NdisReleaseSpinLock(&ba_ctl->BATabLock);

	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i = 1; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &ba_ctl->BARecEntry[i];
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if ((pBAEntry->REC_BA_Status == Recipient_NONE)) {
			/* get one */
			NdisAcquireSpinLock(&ba_ctl->BATabLock);
			ba_ctl->numAsRecipient++;
			NdisReleaseSpinLock(&ba_ctl->BATabLock);
			pBAEntry->REC_BA_Status = Recipient_USED;
			*Idx = i;
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
			pBAEntry->reorder_buf_id = ba_buf->id;
			pBAEntry->reorder_buf = &ba_buf->buf[0];
#endif
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			break;
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
	if (*Idx == MAX_LEN_OF_BA_REC_TABLE) {
		NdisAcquireSpinLock(&ba_ctl->BATabLock);
		release_ba_buf(ba_ctl, &ba_ctl->free_ba_buf[ba_buf->id]);
		NdisReleaseSpinLock(&ba_ctl->BATabLock);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"%s: get Idx failed, release ba buf\n", __func__);
	}
#endif

done:
	return pBAEntry;
}

BOOLEAN ba_resrc_rec_add(
	RTMP_ADAPTER *pAd,
	UINT16 wcid,
	UCHAR tid, USHORT timeout, USHORT ba_start_seq, UINT16 ba_buffer_size)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	MAC_TABLE_ENTRY *pEntry;
	struct BA_REC_ENTRY *pBAEntry;
	struct BA_INFO *ba_info;
	USHORT Idx;
	BOOLEAN Status = TRUE;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
	uint16_t mld_sta_idx = 0;
#endif

	pEntry = entry_get(pAd, wcid);
	if (!IS_VALID_ENTRY(pEntry)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"pEntry is INVALID\n");
		return FALSE;
	}

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			Status = FALSE;
			goto end;
		}
		mld_sta_idx = mld_entry->mld_sta_idx;
		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	Idx = ba_info->RecWcidArray[tid];
	if (Idx == 0)
		pBAEntry = ba_alloc_rec_entry(pAd, &Idx);
	else {
		pBAEntry = &ba_ctl->BARecEntry[Idx];
		if (pBAEntry->LastIndSeq == ((ba_start_seq - 1) & MAXSEQ) &&
				pBAEntry->REC_BA_Status == Recipient_Initialization) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"Previous BA is not finished yet, (Wcid/TID/LastIndSeq/SSN = %d/%d/%d/%d)\n",
				wcid, tid, pBAEntry->LastIndSeq, ba_start_seq);
			goto end;
		}

		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if (IS_BA_WAITING(pBAEntry)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"[WAITING] Receive ADDBA = %d/%d/%d/%d ACTN[%d->%d]\n",
				wcid, tid, pBAEntry->LastIndSeq, ba_start_seq,
				pBAEntry->Postpone_Action, ADDBA_POSTPONE);
			pBAEntry->BAWinSize = ba_buffer_size;
			pBAEntry->TimeOutValue = timeout;
			update_last_in_sn(pBAEntry, (ba_start_seq - 1) & MAXSEQ);
			pBAEntry->Postpone_Action = ADDBA_POSTPONE;
			pBAEntry->check_amsdu_miss = TRUE;
			pBAEntry->PreviousAmsduState = 0;
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			goto end;
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

		ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);

		/* BA entry exist should del_ba firstly */
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"DEL_BA (Wcid/TID/LastIndSeq/SSN/BA_Status = %d/%d/%d/%d/%d)\n",
					wcid, tid, pBAEntry->LastIndSeq, ba_start_seq, pBAEntry->REC_BA_Status);

		if (pBAEntry->REC_BA_Status == Recipient_Offload) {
			NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
			chip_do_extra_action(pAd, NULL, NULL,
					CHIP_EXTRA_ACTION_ADDBA_WAIT_ADD, (UINT8 *)pBAEntry, NULL);
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		}
#ifdef DOT11_EHT_BE
		if (mld_entry) {
			if (pEntry->wdev) {
				if (pEntry->wdev->wdev_type == WDEV_TYPE_AP)
					bss_mngr_mld_ba_del_from_asic(pEntry->wdev,
						mld_sta_idx, tid, BA_SESSION_RECP, 0);
#ifdef CONFIG_STA_SUPPORT
				else if (pEntry->wdev->wdev_type == WDEV_TYPE_STA)
					sta_mld_ba_del_from_asic(pEntry->wdev,
						pEntry, tid, BA_SESSION_RECP, 0);
#endif/*CONFIG_STA_SUPPORT*/
			}

		} else
#endif
		{
			RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, wcid, tid,
				BA_SESSION_RECP, 0);
		}

		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		if (pBAEntry->REC_BA_Status == Recipient_Offload) {
			if (!IS_CIPHER_NONE_OR_WEP_Entry(pEntry))
				hc_set_wed_pn_check(pAd, wcid, pBAEntry->Session_id, FALSE);
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}

	NdisAcquireSpinLock(&ba_ctl->BATabLock);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "(%ld): Idx = %d, BAWinSize = %d\n",
			 ba_ctl->numAsRecipient, Idx, ba_buffer_size);
	NdisReleaseSpinLock(&ba_ctl->BATabLock);

	/* Start fill in parameters.*/
	if (pBAEntry != NULL && pEntry->wdev != NULL) {
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

		/* WO RRO case, !fASIC_CAP_HW_RRO && fASIC_CAP_BA_OFFLOAD */
		if (!(cap->asic_caps & fASIC_CAP_HW_RRO) && (cap->asic_caps & fASIC_CAP_BA_OFFLOAD))
			pBAEntry->REC_BA_Status = Recipient_Offload;
		/* INCHIP RRO Special case - Waiting FW remove BA */
		else if (cap->hw_rro_en && (cap->asic_caps & fASIC_CAP_HW_RRO) && IS_BA_WAITING(pBAEntry))
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "Wait-RRO\n");
		/* INCHIP RRO case */
		else if (cap->hw_rro_en && (cap->asic_caps & fASIC_CAP_HW_RRO))
			pBAEntry->REC_BA_Status = Recipient_Initialization;
		/* SW RRO case */
		else
			pBAEntry->REC_BA_Status = Recipient_Initialization;

		pBAEntry->BAWinSize = ba_buffer_size;
		pBAEntry->TID = tid;
		pBAEntry->TimeOutValue = timeout;
		pBAEntry->check_amsdu_miss = TRUE;
		update_last_in_sn(pBAEntry, (ba_start_seq - 1) & MAXSEQ);
		pBAEntry->PreviousAmsduState = 0;
		pBAEntry->pEntry = pEntry;

		NdisAcquireSpinLock(&ba_ctl->BATabLock);

		chip_do_extra_action(pAd, NULL, NULL,
			CHIP_EXTRA_ACTION_ICV_ERR_INIT, (UINT8 *)pBAEntry, NULL);

		if ((ba_ctl->dbg_flag & SN_HISTORY) && (ba_ctl->numAsRecipient < 5)) {
			if (pBAEntry->ba_rec_dbg != NULL) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
					"Freeing previous Debug BA Rec %p\n",
					pBAEntry->ba_rec_dbg);
				os_free_mem(pBAEntry->ba_rec_dbg);
				pBAEntry->ba_rec_dbg = NULL;
			}
			os_alloc_mem(NULL, (UCHAR **)&pBAEntry->ba_rec_dbg, sizeof(struct ba_rec_debug) * BA_REC_DBG_SIZE);
			if (pBAEntry->ba_rec_dbg != NULL)
				os_zero_mem(pBAEntry->ba_rec_dbg, sizeof(struct ba_rec_debug) * BA_REC_DBG_SIZE);
			else
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA,
					DBG_LVL_ERROR, "alloc mem failed!\n");
		}
		NdisReleaseSpinLock(&ba_ctl->BATabLock);

		/* Set Bitmap flag.*/
		ba_info->RxBitmap |= (1 << tid);
		ba_info->RecWcidArray[tid] = Idx;

		if (IS_BA_WAITING(pBAEntry)) {
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			goto end;
		}

		chip_do_extra_action(pAd, NULL, NULL,
			CHIP_EXTRA_ACTION_ADDBA_WAIT_INIT, (UINT8 *)pBAEntry, NULL);

#ifdef DOT11_EHT_BE
		if (mld_entry) {
			if (pEntry->wdev) {
				if (pEntry->wdev->wdev_type == WDEV_TYPE_AP)
					bss_mngr_mld_ba_add_to_asic(pEntry->wdev,
						mld_sta_idx, pBAEntry->TID, pBAEntry->LastIndSeq,
						pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
#ifdef CONFIG_STA_SUPPORT
				else if (pEntry->wdev->wdev_type == WDEV_TYPE_STA)
					sta_mld_ba_add_to_asic(pEntry->wdev,
						pEntry, pBAEntry->TID, pBAEntry->LastIndSeq,
						pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
#endif/*CONFIG_STA_SUPPORT*/
				}
		} else
#endif
		{
			RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, pBAEntry->TID,
				pBAEntry->LastIndSeq, pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

		if (pBAEntry->REC_BA_Status == Recipient_Offload)
			ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "MACEntry[%d]RXBAbitmap = 0x%x. BARecWcidArray=%d\n",
			pEntry->wcid, ba_info->RxBitmap, ba_info->RecWcidArray[tid]);
	} else {
		Status = FALSE;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "Can't Accept ADDBA for "MACSTR" TID = %d\n",
				 MAC2STR(pEntry->Addr), tid);
	}
end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	return Status;
}

VOID ba_free_rec_entry(RTMP_ADAPTER *pAd, ULONG Idx)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_REC_ENTRY    *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_INFO *ba_info;

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_REC_TABLE))
		return;

	pBAEntry = &ba_ctl->BARecEntry[Idx];
	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	if (pBAEntry->REC_BA_Status != Recipient_NONE) {
		pEntry = pBAEntry->pEntry;
		if (!IS_VALID_ENTRY(pEntry)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"pEntry is INVALID\n");
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
			return;
		}

		ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();

		if (IS_ENTRY_MLO(pEntry)) {
			struct mld_entry_t *mld_entry;

			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (!mld_entry)
				goto end;

			ba_info = &mld_entry->ba_info;
		}
#endif /* DOT11_EHT_BE */

		ba_info->RecWcidArray[pBAEntry->TID] = 0;
		ba_info->RxBitmap &= (~(1 << (pBAEntry->TID)));
		pBAEntry->REC_BA_Status = Recipient_NONE;
		pBAEntry->WaitWM = FALSE;
		pBAEntry->Session_id = 0;
		pBAEntry->RetryCnt = 0;
		NdisAcquireSpinLock(&ba_ctl->BATabLock);
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
		release_ba_buf(ba_ctl, &ba_ctl->free_ba_buf[pBAEntry->reorder_buf_id]);
#endif
		if (ba_ctl->numAsRecipient > 0)
			ba_ctl->numAsRecipient -= 1;
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
				"Idx = %lu, REC_BA_Status = %d, Wcid(pEntry) = %d, Tid = %d\n",
				Idx, pBAEntry->REC_BA_Status, pEntry->wcid, pBAEntry->TID);
		}
		NdisReleaseSpinLock(&ba_ctl->BATabLock);

		if (ba_ctl->dbg_flag & SN_HISTORY) {
			if (pBAEntry->ba_rec_dbg) {
				os_free_mem(pBAEntry->ba_rec_dbg);
				pBAEntry->ba_rec_dbg = NULL;
			}
		 }
	}
end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}

BOOLEAN ba_resrc_rec_del(
	RTMP_ADAPTER *pAd,
	UINT16 wcid, UCHAR tid)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct _MAC_TABLE_ENTRY *pEntry = entry_get(pAd, wcid);
	struct BA_REC_ENTRY *pBAEntry;
	UINT Idx;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
	uint16_t mld_sta_idx;
#endif

	if (tid >= NUM_OF_TID)
		return FALSE;

	Idx = pEntry->ba_info.RecWcidArray[tid];

#ifdef DOT11_EHT_BE
	if (IS_ENTRY_MLO(pEntry)) {
		mt_rcu_read_lock();
		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return FALSE;
		}
		Idx = mld_entry->ba_info.RecWcidArray[tid];
		mld_sta_idx = mld_entry->mld_sta_idx;
		mt_rcu_read_unlock();
	}
#endif /* DOT11_EHT_BE */

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE))
		return FALSE;

	pBAEntry = &ba_ctl->BARecEntry[Idx];

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	if (IS_BA_WAITING(pBAEntry)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"[WAITING] Receive DELBA = %d/%d/%d ACTN[%d->%d]\n",
			pBAEntry->pEntry->wcid, pBAEntry->TID, pBAEntry->LastIndSeq,
			pBAEntry->Postpone_Action, DELBA_POSTPONE);
		pBAEntry->Postpone_Action = DELBA_POSTPONE;
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		return FALSE;
	}

	if (pBAEntry && pBAEntry->REC_BA_Status == Recipient_Offload) {
		chip_do_extra_action(pAd, NULL, NULL,
				CHIP_EXTRA_ACTION_ADDBA_WAIT_DELETE, (UINT8 *)pBAEntry, NULL);
	}
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

#ifdef DOT11_EHT_BE
	if (mld_entry) {
		if (pEntry->wdev) {
			if (pEntry->wdev->wdev_type == WDEV_TYPE_AP)
				bss_mngr_mld_ba_del_from_asic(pEntry->wdev,
					mld_sta_idx, tid, BA_SESSION_RECP, 0);
#ifdef CONFIG_STA_SUPPORT
			else if (pEntry->wdev->wdev_type == WDEV_TYPE_STA)
				sta_mld_ba_del_from_asic(pEntry->wdev,
					pEntry, tid, BA_SESSION_RECP, 0);
#endif/*CONFIG_STA_SUPPORT*/
		}

	} else {
#endif
		RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, wcid, tid,
			BA_SESSION_RECP, 0);
#ifdef DOT11_EHT_BE
	}
#endif

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	if (pBAEntry && pBAEntry->REC_BA_Status == Recipient_Offload) {
		if (!IS_CIPHER_NONE_OR_WEP_Entry(pEntry))
			hc_set_wed_pn_check(pAd, wcid, pBAEntry->Session_id, FALSE);
	}

	if (IS_BA_WAITING(pBAEntry)) {
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);
		return TRUE;
	}
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

	ba_free_rec_entry(pAd, Idx);
	/*
	 * report all mpdu in reordering buffer after ba_free_rec_entry
	 * to make sure the reordering buffer will be empty.
	 */
	ba_refresh_reordering_mpdus(pAd, ba_ctl, pBAEntry);

	return TRUE;
}

VOID ba_send_delba(
	struct _RTMP_ADAPTER *pAd,
	uint16_t Wcid,
	UCHAR TID)
{

	MLME_DELBA_REQ_STRUCT DelbaReq;
	MLME_QUEUE_ELEM *Elem;

	os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

	if (Elem != NULL) {
		NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
		NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
		COPY_MAC_ADDR(DelbaReq.Addr, entry_addr_get(pAd, Wcid));
		DelbaReq.Wcid = Wcid;
		DelbaReq.TID = TID;
		DelbaReq.Initiator = ORIGINATOR;
		Elem->MsgLen  = sizeof(DelbaReq);
		NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
		MlmeDELBAAction(pAd, Elem);
		os_free_mem(Elem);
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			":alloc memory failed!\n");
	}
}

VOID ba_ori_session_tear_down(
	INOUT RTMP_ADAPTER *pAd,
	IN UINT16 Wcid,
	IN UCHAR TID,
	IN BOOLEAN bPassive)
{
	MLME_DELBA_REQ_STRUCT	DelbaReq;
	MLME_QUEUE_ELEM *Elem;

	/* sanity check */
	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return;

	/* resource management */
	if (!ba_resrc_ori_del(pAd, Wcid, TID))
		return;

#ifdef WF_RESET_SUPPORT
	if (PD_GET_WF_RESET_IN_PROGRESS(pAd->physical_dev) == TRUE)
		return;
#endif

	/* send del ba */
	if (bPassive == FALSE) {
		os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

		if (Elem != NULL) {
			NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
			NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
			COPY_MAC_ADDR(DelbaReq.Addr, entry_addr_get(pAd, Wcid));
			DelbaReq.Wcid = Wcid;
			DelbaReq.TID = TID;
			DelbaReq.Initiator = ORIGINATOR;
			Elem->MsgLen  = sizeof(DelbaReq);
			NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
			MlmeDELBAAction(pAd, Elem);
			os_free_mem(Elem);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				":alloc memory failed!\n");
			return;
		}
	}
}

VOID ba_rec_session_tear_down(
	PRTMP_ADAPTER pAd,
	UINT16 Wcid,
	UCHAR TID,
	BOOLEAN bPassive)
{
	MLME_DELBA_REQ_STRUCT DelbaReq;
	MLME_QUEUE_ELEM *Elem;

	/* sanity check */
	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return;

/* resource management */
	if (!ba_resrc_rec_del(pAd, Wcid, TID))
		return;

#ifdef WF_RESET_SUPPORT
	if (PD_GET_WF_RESET_IN_PROGRESS(pAd->physical_dev) == TRUE)
		return;
#endif

	/* send del ba */
	if (bPassive == FALSE) {
		os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

		if (Elem != NULL) {
			NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
			NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
			COPY_MAC_ADDR(DelbaReq.Addr, entry_addr_get(pAd, Wcid));
			DelbaReq.Wcid = Wcid;
			DelbaReq.TID = TID;
			DelbaReq.Initiator = RECIPIENT;
			Elem->MsgLen  = sizeof(DelbaReq);
			NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
			MlmeDELBAAction(pAd, Elem);
			os_free_mem(Elem);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				":alloc memory failed!\n");
			return;
		}
	}
}


VOID ba_session_tear_down_all(RTMP_ADAPTER *pAd, UINT16 Wcid, BOOLEAN bPassive)
{
	int i;

	for (i = 0; i < NUM_OF_TID; i++) {
		ba_ori_session_tear_down(pAd, Wcid, i, bPassive);
		ba_rec_session_tear_down(pAd, Wcid, i, bPassive);
	}
}

void parse_addba_option_element(
	UINT8 *option_ie,
	UINT32 option_le_len,
	struct addba_option_info *addba_option_info)
{
	PEID_STRUCT eid;
	UINT32 length = 0;

	eid = (PEID_STRUCT) option_ie;
	while ((length + 2 + eid->Len) <= option_le_len) {
		switch (eid->Eid) {
		case IE_ADDBA_EXT:
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA,
				DBG_LVL_INFO,
				"- IE_ADDBA_EXT: 0x%x\n",
				eid->Octet[0]);
#ifdef DOT11_EHT_BE
			addba_option_info->addba_ext_bufsize =
				GET_ADDBA_EXT_BUF_SIZE(eid->Octet[0]);
#endif /* DOT11_EHT_BE */
			break;
		}

		length = length + 2 + eid->Len;
		eid = (PEID_STRUCT)((UCHAR *)eid + 2 + eid->Len);
	}
}

VOID peer_addba_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Status = MLME_UNSPECIFY_FAIL;
	PFRAME_ADDBA_REQ pAddreqFrame = (PFRAME_ADDBA_REQ)(&Elem->Msg[0]);
	MAC_TABLE_ENTRY *pEntry;
	MLME_QUEUE_ELEM *addba_resp_elem;
	UINT8 ba_decline = 0;
#ifdef DOT11W_PMF_SUPPORT
	STA_TR_ENTRY *tr_entry;
	PFRAME_802_11 pFrame = NULL;
#endif /* DOT11W_PMF_SUPPORT */
	struct ppdu_caps *ppdu = NULL;
	UINT16 rec_ba_wsize = 0;
	UINT16 cfg_rx_ba_wsize;
	BOOLEAN amsdu_supprot = 0;
	UINT8 *option_ie = NULL;
	UINT16 option_ie_total_len = 0;
	struct addba_option_info addba_option_info;
	UINT16 peer_wsize;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	UINT16 wcid = Elem->Wcid;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "==> (Wcid = %d)\n", wcid);

	/* sanity check */
	RETURN_IF_PAD_NULL(pAd);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = entry_get(pAd, wcid);

	if (pEntry && pEntry->wdev) {
		if (IS_ENTRY_CLIENT(pEntry)) {
			if (pEntry->Sst != SST_ASSOC) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "peer entry is not in association state\n");
				return;
			}

#ifdef DOT11W_PMF_SUPPORT
			tr_entry = tr_entry_get(pAd, wcid);

			if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
				(tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"PMF Connection IGNORE THIS PKT DUE TO NOT IN PORTSECURED\n");
				return;
			}

			pFrame = (PFRAME_802_11)Elem->Msg;

			if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
				(pFrame->Hdr.FC.Wep == 0)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
					"PMF CONNECTION BUT RECV WEP=0 ACTION, DROP FRAME\n");
				return;
			}

#endif /* DOT11W_PMF_SUPPORT */
		}
	} else
		return;

	if (!PeerAddBAReqActionSanity(pAd, Elem->Msg, Elem->MsgLen)) {
		Status = MLME_UNSPECIFY_FAIL;
		goto send_response;
	}

	if (ba_ctl->rx_ba_disable) {
		Status = MLME_REQUEST_DECLINED;
		goto send_response;
	}

	RTMPZeroMemory(&addba_option_info, sizeof(struct addba_option_info));
	if (Elem->MsgLen > sizeof(struct _FRAME_ADDBA_REQ)) {
		option_ie = &Elem->Msg[sizeof(struct _FRAME_ADDBA_REQ)];
		option_ie_total_len =
			Elem->MsgLen - sizeof(struct _FRAME_ADDBA_REQ);
		parse_addba_option_element(
			option_ie,
			option_ie_total_len,
			&addba_option_info);
	}

	ba_decline = wlan_config_get_ba_decline(pEntry->wdev);

#ifdef SW_CONNECT_SUPPORT
	/* S/W no AGG support */
	{
		STA_TR_ENTRY *tr_entry_temp = tr_entry_get(pAd, wcid);

		if (IS_SW_MAIN_STA(tr_entry_temp) || IS_SW_STA(tr_entry_temp)) {
			ba_decline = 1;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
						"S/W no AGG support!\n");
		}
	}
#endif /* SW_CONNECT_SUPPORT */

	if (ba_decline || !IS_HT_STA(pEntry)) {
		Status = MLME_REQUEST_DECLINED;
		goto send_response;
	}

	/* parameter decision */
	cfg_rx_ba_wsize = wlan_config_get_ba_rx_wsize(pEntry->wdev);
	peer_wsize = pAddreqFrame->BaParm.BufSize;
#ifdef DOT11_EHT_BE
	peer_wsize += (addba_option_info.addba_ext_bufsize*1024);
#endif /* DOT11_EHT_BE */
	rec_ba_wsize = cal_rec_ba_wsize(pEntry, cfg_rx_ba_wsize, peer_wsize);
	ppdu = wlan_config_get_ppdu_caps(pEntry->wdev);
	amsdu_supprot = (ppdu->rx_amsdu_in_ampdu_support & wlan_config_get_amsdu_en(pEntry->wdev)) ? pAddreqFrame->BaParm.AMSDUSupported : 0;

	/* resource management */
	if (ba_resrc_rec_add(pAd, wcid, pAddreqFrame->BaParm.TID,
		pAddreqFrame->TimeOutValue, pAddreqFrame->BaStartSeq.field.StartSeq, rec_ba_wsize))
		Status = MLME_SUCCESS;
	else
		Status = MLME_REQUEST_WITH_INVALID_PARAM;
send_response:
	/* send add ba response */
	os_alloc_mem(NULL, (UCHAR **)&addba_resp_elem, sizeof(MLME_QUEUE_ELEM));

	if (addba_resp_elem != NULL) {
		MLME_ADDBA_RESP_STRUCT	mlme_addba_resp;

		NdisZeroMemory(&mlme_addba_resp, sizeof(mlme_addba_resp));
		mlme_addba_resp.wcid = wcid;
		COPY_MAC_ADDR(mlme_addba_resp.addr, pAddreqFrame->Hdr.Addr2);

		mlme_addba_resp.status = Status;
		mlme_addba_resp.token = pAddreqFrame->Token;
		mlme_addba_resp.amsdu_support = amsdu_supprot;
		mlme_addba_resp.tid = pAddreqFrame->BaParm.TID;
		mlme_addba_resp.buf_size = rec_ba_wsize;
		mlme_addba_resp.timeout = 0;

#ifdef CONFIG_CPE_SUPPORT
#ifdef DOT11_EHT_BE
		mt_rcu_read_lock();
		if (IS_ENTRY_MLO(pEntry)) {
			struct mld_entry_t *mld_entry;

			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (mld_entry && mld_entry->link_num > 1 && !pAd->CommonCfg.wifi_cert)
				mlme_addba_resp.amsdu_support = 0;
		}
		mt_rcu_read_unlock();
#endif
#endif

		NdisZeroMemory(addba_resp_elem, sizeof(MLME_QUEUE_ELEM));
		addba_resp_elem->Wcid = Elem->Wcid;
		addba_resp_elem->wdev = Elem->wdev;
		addba_resp_elem->MsgLen  = sizeof(mlme_addba_resp);
		NdisMoveMemory(addba_resp_elem->Msg, &mlme_addba_resp, sizeof(mlme_addba_resp));

		mlme_send_addba_resp(pAd, addba_resp_elem);
		os_free_mem(addba_resp_elem);
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "alloc memory failed!\n");
		return;
	}
}

VOID peer_addba_rsp_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_ADDBA_RSP pFrame = NULL;
	MAC_TABLE_ENTRY *pEntry;
	struct BA_INFO *ba_info;
	struct ppdu_caps *ppdu;
	struct addba_option_info addba_option_info;
	UINT16 ori_ba_wsize = 0;
	UINT16 cfg_tx_ba_wsize = 0;
	UCHAR amsdu_en = 0;
	UINT16 peer_wsize = 0;
	UINT16 wcid = Elem->Wcid;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "==> Wcid(%d)\n", wcid);

	/* sanity check */
	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = entry_get(pAd, wcid);
	if (!pEntry || !pEntry->wdev)
		return;

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry)
			goto end;

		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	if (!PeerAddBARspActionSanity(pAd, Elem->Msg, Elem->MsgLen, wcid, &addba_option_info))
		goto end;

	pFrame = (PFRAME_ADDBA_RSP)(&Elem->Msg[0]);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "\t\t TID=%d Token=%d StatusCode = %d\n", pFrame->BaParm.TID, pFrame->Token, pFrame->StatusCode);

	switch (pFrame->StatusCode) {
	case MLME_SUCCESS:
		/* parameter decision */
		cfg_tx_ba_wsize = wlan_config_get_ba_tx_wsize(pEntry->wdev);
		peer_wsize = pFrame->BaParm.BufSize;
#ifdef DOT11_EHT_BE
		peer_wsize += (addba_option_info.addba_ext_bufsize*1024);
#endif /* DOT11_EHT_BE */
		ori_ba_wsize = cal_ori_ba_wsize(pEntry, cfg_tx_ba_wsize, peer_wsize);
		ppdu = (struct ppdu_caps *)wlan_config_get_ppdu_caps(pEntry->wdev);
		amsdu_en = wlan_config_get_amsdu_en(pEntry->wdev) && ppdu->tx_amsdu_support && pFrame->BaParm.AMSDUSupported;

		/* resource management */
		if (!ba_resrc_ori_add(pAd, wcid, pFrame->BaParm.TID, ori_ba_wsize, amsdu_en, pFrame->TimeOutValue)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"wcid=%d TID=%d Token=%d add ori resrc fail\n", wcid, pFrame->BaParm.TID, pFrame->Token);
			goto end;
		}
		/* send BAR after BA session is build up */
		SendRefreshBAR(pAd, pEntry, pFrame->BaParm.TID);
		if (ba_info->AutoTest)
			MTWF_PRINT(
				"<addba_info>: sta_mac="MACSTR", tid=%d, buffersize=%d\n",
				MAC2STR(pEntry->Addr),
				pFrame->BaParm.TID, ori_ba_wsize);
		break;
	case MLME_REQUEST_DECLINED:
		ba_info->DeclineBitmap |= 1 << pFrame->BaParm.TID;
		fallthrough;
		/* don't break, need to delete the session, too */
	default:
		/* delete the ori ba session passively */
		ba_ori_session_tear_down(pAd, wcid, pFrame->BaParm.TID, TRUE);
		break;
	}
end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
}

VOID peer_delba_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PFRAME_DELBA_REQ    pDelFrame = NULL;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, " ==>\n");

	/* sanity check */
	if (!PeerDelBAActionSanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen))
		return;

	pDelFrame = (PFRAME_DELBA_REQ)(&Elem->Msg[0]);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "Initiator = %d, Reason = %d\n", pDelFrame->DelbaParm.Initiator, pDelFrame->ReasonCode);

	if (pDelFrame->DelbaParm.Initiator == ORIGINATOR) {
		ba_rec_session_tear_down(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE);
	} else {
		ba_ori_session_tear_down(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE);
	}

}

static BOOLEAN amsdu_sanity(RTMP_ADAPTER *pAd, UINT16 CurSN,
	UINT8 cur_amsdu_state, struct BA_REC_ENTRY *pBAEntry)
{
	BOOLEAN PreviosAmsduMiss = FALSE;
	USHORT  LastIndSeq;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);

	if (CurSN != pBAEntry->PreviousSN) {
		if ((pBAEntry->PreviousAmsduState == FIRST_AMSDU_FORMAT) ||
				(pBAEntry->PreviousAmsduState == MIDDLE_AMSDU_FORMAT)) {
			PreviosAmsduMiss = TRUE;
		}
	} else {
		if (((pBAEntry->PreviousAmsduState == FIRST_AMSDU_FORMAT) ||
			(pBAEntry->PreviousAmsduState == MIDDLE_AMSDU_FORMAT)) &&
				(cur_amsdu_state == FIRST_AMSDU_FORMAT)) {
			PreviosAmsduMiss = TRUE;
		}


	}

	if (PreviosAmsduMiss) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_DEBUG, "PreviosAmsduMiss or only one MSDU in AMPDU");
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		pBAEntry->CurMpdu = NULL;
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

		switch (pBAEntry->PreviousReorderCase) {
		case STEP_ONE:
			update_last_in_sn(pBAEntry, pBAEntry->PreviousSN);
			LastIndSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, pBAEntry->LastIndSeq);

			if (LastIndSeq != INVALID_RCV_SEQ)
				update_last_in_sn(pBAEntry, LastIndSeq);

			break;
		case REPEAT:
		case OLDPKT:
		case WITHIN:
		case SURPASS:
			break;
		}
	}

	return PreviosAmsduMiss;
}

static inline VOID ba_inc_dbg_idx(UINT32 *idx, UINT16 ba_dbg_size)
{
	*idx = ((*idx + 1) % ba_dbg_size);
}

BOOLEAN bar_process(RTMP_ADAPTER *pAd, UINT16 Wcid, ULONG MsgLen, PFRAME_BA_REQ pMsg)
{
	PFRAME_BA_REQ pFrame = pMsg;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	MAC_TABLE_ENTRY *pEntry;
	struct BA_REC_ENTRY *pBAEntry;
	ULONG Idx;
	UCHAR TID;
	TID = (UCHAR)pFrame->BARControl.TID;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		": BAR-Wcid(%d), Tid (%d)\n", Wcid, TID);

	/*hex_dump("BAR", (PCHAR) pFrame, MsgLen);*/
	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"frame too large, size = %ld\n", MsgLen);
		return FALSE;
	} else if (MsgLen != sizeof(FRAME_BA_REQ)) {
		/*
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "BlockAck Request frame length size = %ld incorrect\n", MsgLen);
		*/
		return FALSE;
	}

	if ((VALID_UCAST_ENTRY_WCID(pAd, Wcid)) && (TID < 8)) {
		pEntry = entry_get(pAd, Wcid);
		Idx = pEntry->ba_info.RecWcidArray[TID];

#ifdef DOT11_EHT_BE
		if (IS_ENTRY_MLO(pEntry)) {
			struct mld_entry_t *mld_entry;

			mt_rcu_read_lock();
			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (!mld_entry) {
				mt_rcu_read_unlock();
				return FALSE;
			}
			Idx = mld_entry->ba_info.RecWcidArray[TID];
			mt_rcu_read_unlock();
		}
#endif /* DOT11_EHT_BE */

		if (Idx == 0)
			return FALSE;

		pBAEntry = &ba_ctl->BARecEntry[Idx];
	} else
		return FALSE;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "BAR(%d) : Tid (%d) - %04x:%04x\n",
		Wcid, TID, pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq);

	amsdu_sanity(pAd, pFrame->BAStartingSeq.field.StartSeq, FIRST_AMSDU_FORMAT, pBAEntry);
	tr_cnt->ba_rx_bar_cnt++;

	if ((ba_ctl->dbg_flag & SN_HISTORY) && (pBAEntry->ba_rec_dbg)) {
		if (ba_ctl->dbg_flag & SN_RECORD_BASIC) {
			struct ba_rec_debug *dbg = &pBAEntry->ba_rec_dbg[pBAEntry->ba_rec_dbg_idx];
			dbg->sn = pFrame->BAStartingSeq.field.StartSeq;
			dbg->amsdu = 0;
			dbg->type = BA_BAR;
			dbg->last_in_seq = pBAEntry->LastIndSeq;
			if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
				memcpy(dbg->ta, pMsg->Addr2, MAC_ADDR_LEN);
				memcpy(dbg->ra, pMsg->Addr1, MAC_ADDR_LEN);
			}
			ba_inc_dbg_idx(&pBAEntry->ba_rec_dbg_idx, BA_REC_DBG_SIZE);
		}
	}

	if (ba_ctl->dbg_flag & SN_DUMP_BAR)
		ba_resource_dump_all(pAd, SN_HISTORY);

	if (SEQ_SMALLER(pBAEntry->LastIndSeq, pFrame->BAStartingSeq.field.StartSeq, MAXSEQ)) {
		LONG TmpSeq, seq;
		/*MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, "BAR Seq = %x, LastIndSeq = %x\n", pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq);*/

		tr_cnt->bar_large_win_start++;
		seq = (pFrame->BAStartingSeq.field.StartSeq == 0) ? MAXSEQ : (pFrame->BAStartingSeq.field.StartSeq - 1);
		ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, seq);
		update_last_in_sn(pBAEntry, seq);

		TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, pBAEntry->LastIndSeq);

		if (TmpSeq != INVALID_RCV_SEQ) {
			update_last_in_sn(pBAEntry, TmpSeq);
		}
	}

	return TRUE;
}

void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	RTMP_ADAPTER *pAd,
	struct _RX_BLK *pRxBlk,
	UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPkt;
	UCHAR Header802_3[LENGTH_802_3];
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, "invalid wdev_idx(%d)\n", wdev_idx);
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];
	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
	ASSERT(pRxBlk->pRxPacket);

	if (pRxBlk->pRxPacket == NULL)
		return;

	pRxPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
	RTMP_OS_PKT_INIT(pRxBlk->pRxPacket,
					 get_netdev_from_bssid(pAd, wdev_idx),
					 pRxBlk->pData, pRxBlk->DataSize);

	/* copy 802.3 header, if necessary */
	if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU)) {
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;
		USHORT VLAN_VID = 0, VLAN_Priority = 0;
		/* TODO: shiang-usw, fix me!! */
#ifdef CONFIG_AP_SUPPORT

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA) || RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) {
			/* Check if need to insert VLAN tag to the received packet */
			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);

			if (VLAN_VID)
				VLAN_Size = LENGTH_802_1Q;
		}

#endif /* CONFIG_AP_SUPPORT */
		{
			data_p = OS_PKT_HEAD_BUF_EXTEND(pRxPkt, LENGTH_802_3 + VLAN_Size);
			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									 Header802_3, LENGTH_802_3,
									 data_p, TPID);
		}
	}
}

#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
static VOID ba_enqueue_reordering_packet(
	RTMP_ADAPTER *pAd,
	struct BA_REC_ENTRY *pBAEntry,
	struct _RX_BLK *pRxBlk,
	UCHAR wdev_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	struct reordering_mpdu *msdu_blk = NULL;
	UINT16 sn = pRxBlk->SN;
	struct reordering_list *list = NULL;
#ifdef PROPRIETARY_DRIVER_SUPPORT
	struct timespec64 kts64 = {0};
#endif

	msdu_blk = ba_mpdu_blk_alloc(pAd, pRxBlk);

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	if ((msdu_blk != NULL) &&
		(!RX_BLK_TEST_FLAG(pRxBlk, fRX_EAP))) {
		/* Write RxD buffer address & allocated buffer length */
		msdu_blk->Sequence = sn;
		msdu_blk->OpMode = pRxBlk->OpMode;
		msdu_blk->bAMSDU = RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU);
		msdu_blk->next = NULL;

		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			convert_reordering_packet_to_preAMSDU_or_802_3_packet(pAd, pRxBlk, wdev_idx);
		else {
			struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

			pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);

			SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		}

		/* it is necessary for reordering packet to record which BSS it come from */
		RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
		STATS_INC_RX_PACKETS(pAd, wdev_idx);
		msdu_blk->pPacket = pRxBlk->pRxPacket;

#ifdef PROPRIETARY_DRIVER_SUPPORT
		ktime_get_real_ts64(&kts64);
		RTPKT_TO_OSPKT(msdu_blk->pPacket)->tstamp = timespec64_to_ktime(kts64);
#else
		__net_timestamp(RTPKT_TO_OSPKT(msdu_blk->pPacket));
#endif

		if (!pBAEntry->CurMpdu) {
			struct reordering_mpdu **reorder_buf = pBAEntry->reorder_buf;
			u16 index = (sn % pBAEntry->BAWinSize);

			if (reorder_buf[index]) {
				tr_cnt->ba_err_dup2++;
				pBAEntry->drop_dup_pkts++;
#ifdef IXIA_C50_MODE
				if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(pRxBlk->pRxPacket)))
					pAd->rx_cnt.rx_dup_drop[pRxBlk->wcid]++;
#endif
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_SUCCESS);
				ba_mpdu_blk_free(ba_ctl, msdu_blk);
				pBAEntry->CurMpdu = NULL;
			} else {
				pBAEntry->CurMpdu = msdu_blk;
				reorder_buf[index] = msdu_blk;
				pBAEntry->stored_mpdu_num++;
			}
		} else {
			pBAEntry->CurMpdu->next = msdu_blk;
			pBAEntry->CurMpdu = msdu_blk;
			pBAEntry->stored_mpdu_num++;
		}

		if (atomic_read(&ba_ctl->SetFlushTimer) == 0) {
			RTMPSetTimer(&ba_ctl->FlushTimer, REORDERING_PACKET_TIMEOUT_IN_MS);
			atomic_inc(&ba_ctl->SetFlushTimer);
		}
	} else {
		if (msdu_blk)
			ba_mpdu_blk_free(ba_ctl, msdu_blk);
		else {
			NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[pRxBlk->band].lock);
			list = &ba_ctl->mpdu_blk_pool[pRxBlk->band].freelist;
			NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[pRxBlk->band].lock);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"!! band%u, wcid:%u, TID:%u, ba:%u, list:%d (used:%d/free:%d) freelist pool empty\n",
				pRxBlk->band, pRxBlk->wcid, pBAEntry->TID, pBAEntry->BAWinSize, !!(list->next),
				pBAEntry->stored_mpdu_num, list->qlen);
		}

		__ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, sn);
		update_last_in_sn(pBAEntry, sn);

		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		indicate_rx_pkt(pAd, pRxBlk, wdev_idx);
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

		sn = __ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, pBAEntry->LastIndSeq);

		if (sn != INVALID_RCV_SEQ)
			update_last_in_sn(pBAEntry, sn);

		pBAEntry->CurMpdu = NULL;

	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}

#else
static VOID ba_enqueue_reordering_packet(
	RTMP_ADAPTER *pAd,
	struct BA_REC_ENTRY *pBAEntry,
	struct _RX_BLK *pRxBlk,
	UCHAR wdev_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	struct reordering_mpdu *msdu_blk;
	UINT16 Sequence = pRxBlk->SN;
#ifdef PROPRIETARY_DRIVER_SUPPORT
	struct timespec64 kts64 = {0};
#endif

	msdu_blk = ba_mpdu_blk_alloc(pAd, pRxBlk);

	if ((msdu_blk != NULL) &&
		(!RX_BLK_TEST_FLAG(pRxBlk, fRX_EAP))) {
		/* Write RxD buffer address & allocated buffer length */
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		msdu_blk->Sequence = Sequence;
		msdu_blk->OpMode = pRxBlk->OpMode;
		msdu_blk->bAMSDU = RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU);

		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			convert_reordering_packet_to_preAMSDU_or_802_3_packet(pAd, pRxBlk, wdev_idx);
		else {
			struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

			pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);

			SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		}

		/* it is necessary for reordering packet to record
			which BSS it come from
		*/
		RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
		STATS_INC_RX_PACKETS(pAd, wdev_idx);
		msdu_blk->pPacket = pRxBlk->pRxPacket;

#ifdef PROPRIETARY_DRIVER_SUPPORT
		ktime_get_real_ts64(&kts64);
		RTPKT_TO_OSPKT(msdu_blk->pPacket)->tstamp = timespec64_to_ktime(kts64);
#else
		__net_timestamp(RTPKT_TO_OSPKT(msdu_blk->pPacket));
#endif
		if (!pBAEntry->CurMpdu) {
			if (ba_reordering_mpdu_insertsorted(&pBAEntry->list, msdu_blk) == FALSE) {
				tr_cnt->ba_err_dup2++;
				pBAEntry->drop_dup_pkts++;
#ifdef IXIA_C50_MODE
				if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(pRxBlk->pRxPacket)))
					pAd->rx_cnt.rx_dup_drop[pRxBlk->wcid]++;
#endif
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_SUCCESS);
				ba_mpdu_blk_free(ba_ctl, msdu_blk);
				pBAEntry->CurMpdu = NULL;
			} else
				pBAEntry->CurMpdu = msdu_blk;
		} else
			ba_enqueue_tail(&pBAEntry->CurMpdu->AmsduList, msdu_blk);

		if ((pBAEntry->list.qlen < 0) || (pBAEntry->list.qlen > pBAEntry->BAWinSize)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"(qlen:%d, BAWinSize:%d)\n",
				pBAEntry->list.qlen, pBAEntry->BAWinSize);
			dump_ba_list(&pBAEntry->list);
		} else if (atomic_read(&ba_ctl->SetFlushTimer) == 0) {
			RTMPSetTimer(&ba_ctl->FlushTimer, REORDERING_PACKET_TIMEOUT_IN_MS);
			atomic_inc(&ba_ctl->SetFlushTimer);
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	} else {
		if (msdu_blk)
			ba_mpdu_blk_free(ba_ctl, msdu_blk);
		else {
			NdisAcquireSpinLock(&ba_ctl->mpdu_blk_pool[pRxBlk->band].lock);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"!!! (used:%d/free:%d) Can't allocate reordering mpdu blk\n",
				pBAEntry->list.qlen, ba_ctl->mpdu_blk_pool[pRxBlk->band].freelist.qlen);
			NdisReleaseSpinLock(&ba_ctl->mpdu_blk_pool[pRxBlk->band].lock);
		}

		ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, pBAEntry, Sequence);
		update_last_in_sn(pBAEntry, Sequence);

		indicate_rx_pkt(pAd, pRxBlk, wdev_idx);

		Sequence = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, pBAEntry, pBAEntry->LastIndSeq);

		if (Sequence != INVALID_RCV_SEQ) {
			update_last_in_sn(pBAEntry, Sequence);
		}

		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		pBAEntry->CurMpdu = NULL;
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}
}
#endif

VOID ba_reorder(RTMP_ADAPTER *pAd, struct _RX_BLK *rx_blk, UCHAR wdev_idx)
{
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct tr_counter *tr_cnt = &tr_ctl->tr_cnt;
	MAC_TABLE_ENTRY *pEntry;
	UINT16 seq = rx_blk->SN;
	struct BA_REC_ENTRY *ba_entry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, rx_blk->wcid)) {
		UINT16 idx;

		pEntry = entry_get(pAd, rx_blk->wcid);
		idx = pEntry->ba_info.RecWcidArray[rx_blk->TID];

#ifdef DOT11_EHT_BE
		if (IS_ENTRY_MLO(pEntry)) {
			struct mld_entry_t *mld_entry;

			mt_rcu_read_lock();
			mld_entry = rcu_dereference(pEntry->mld_entry);
			if (!mld_entry) {
				mt_rcu_read_unlock();
				return;
			}
			idx = mld_entry->ba_info.RecWcidArray[rx_blk->TID];
			mt_rcu_read_unlock();
		}
#endif /* DOT11_EHT_BE */

		if (idx == 0) {
			/* recipient BA session had been torn down */
			indicate_rx_pkt(pAd, rx_blk, wdev_idx);
			tr_cnt->ba_err_tear_down++;
			return;
		}

		ba_entry = &ba_ctl->BARecEntry[idx];
	} else {
		tr_cnt->ba_err_wcid_invalid++;
		RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if ((ba_ctl->dbg_flag & SN_HISTORY) && (ba_entry->ba_rec_dbg)) {
		if (ba_ctl->dbg_flag & SN_RECORD_BASIC) {
			struct ba_rec_debug *dbg = &ba_entry->ba_rec_dbg[ba_entry->ba_rec_dbg_idx];
			dbg->sn = seq;
			dbg->amsdu = rx_blk->AmsduState;
			dbg->type = BA_DATA;
			dbg->last_in_seq = ba_entry->LastIndSeq;

			if (ba_ctl->dbg_flag & SN_RECORD_MAC) {
				memcpy(dbg->ta, rx_blk->Addr2, MAC_ADDR_LEN);
				memcpy(dbg->ra, rx_blk->Addr1, MAC_ADDR_LEN);
			}

			ba_inc_dbg_idx(&ba_entry->ba_rec_dbg_idx, BA_REC_DBG_SIZE);
		}
	}

	switch (ba_entry->REC_BA_Status) {
	case Recipient_NONE:
	case Recipient_USED:
	case Recipient_HandleRes:
		ba_refresh_reordering_mpdus(pAd, ba_ctl, ba_entry);
		indicate_rx_pkt(pAd, rx_blk, wdev_idx);
		return;

	case Recipient_Initialization:
		NdisAcquireSpinLock(&ba_entry->RxReRingLock);
		__ba_refresh_reordering_mpdus(pAd, ba_ctl, ba_entry);
		ba_entry->REC_BA_Status = Recipient_Established;
		NdisReleaseSpinLock(&ba_entry->RxReRingLock);
#ifndef CONFIG_BA_REORDER_ARRAY_SUPPORT
		ASSERT((ba_entry->list.qlen == 0) && (ba_entry->list.next == NULL));
#endif
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_DEBUG,
			"Reset Last Indicate Sequence(%d): amsdu state = %d\n",
			rx_blk->SN, rx_blk->AmsduState);
		/*
		 * For the first reordering pkt in the BA session, initialize LastIndSeq to (Sequence - 1)
		 * so that the ba_reorder_check will fall in the in-order SEQ_STEPONE case.
		 */
		update_last_in_sn(ba_entry, (seq - 1) & MAXSEQ);
		ba_entry->PreviousSN = ba_entry->LastIndSeq;
		ba_entry->PreviousAmsduState = 0;
	case Recipient_Established:
		break;
	case Recipient_Offload:
#ifdef OLDPKT_CHECK_CNT
		chip_do_extra_action(pAd, NULL, NULL,
			CHIP_EXTRA_ACTION_RX_RESTORE_OLDPKT, (UCHAR *)ba_entry, (ULONG *)rx_blk);
		if (rx_blk->pRxPacket)
#endif
			indicate_rx_pkt(pAd, rx_blk, wdev_idx);
		return;
	default:
		tr_cnt->ba_drop_unknown++;
		ba_entry->drop_unknown_state_pkts++;
		RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (ba_entry->check_amsdu_miss)
		amsdu_sanity(pAd, seq, rx_blk->AmsduState, ba_entry);

ba_reorder_check:
	/* I. Check if in order. */
	if (SEQ_STEPONE(seq, ba_entry->LastIndSeq, MAXSEQ)) {
		USHORT  LastIndSeq;

		if (ba_ctl->dbg_flag & SN_DUMP_STEPONE)
			ba_resource_dump_all(pAd, SN_HISTORY);

		indicate_rx_pkt(pAd, rx_blk, wdev_idx);

		if ((rx_blk->AmsduState == FINAL_AMSDU_FORMAT) || (rx_blk->AmsduState == MSDU_FORMAT)) {
			update_last_in_sn(ba_entry, seq);
			LastIndSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, ba_entry, ba_entry->LastIndSeq);

			if (LastIndSeq != INVALID_RCV_SEQ)
				update_last_in_sn(ba_entry, LastIndSeq);
		}

		ba_entry->PreviousReorderCase = STEP_ONE;
	}
	/* II. Drop Duplicated Packet*/
	else if (seq == ba_entry->LastIndSeq) {
		if (RX_BLK_TEST_FLAG(rx_blk, fRX_AMSDU) &&
			(tr_ctl->damsdu_type == RX_SW_AMSDU)) {
			indicate_rx_pkt(pAd, rx_blk, wdev_idx);
		} else {
			if (ba_ctl->dbg_flag & SN_DUMP_DUP)
				ba_resource_dump_all(pAd, SN_HISTORY);
			tr_cnt->ba_err_dup1++;
			ba_entry->drop_dup_pkts++;
#ifdef IXIA_C50_MODE
			if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(rx_blk->pRxPacket))) {
				pAd->rx_cnt.rx_dup_drop[rx_blk->wcid]++;
			}
#endif
			RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		}

		ba_entry->PreviousReorderCase = REPEAT;
	}
	/* III. Drop Old Received Packet*/
	else if (SEQ_SMALLER(seq, ba_entry->LastIndSeq, MAXSEQ)) {
		if (ba_ctl->dbg_flag & SN_DUMP_OLD)
			ba_resource_dump_all(pAd, SN_HISTORY);
		tr_cnt->ba_err_old++;
		ba_entry->drop_old_pkts++;
#ifdef IXIA_C50_MODE
		if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(rx_blk->pRxPacket))) {
			pAd->rx_cnt.rx_old_drop[rx_blk->wcid]++;
		}
#endif
		RELEASE_NDIS_PACKET(pAd, rx_blk->pRxPacket, NDIS_STATUS_FAILURE);
		ba_entry->PreviousReorderCase = OLDPKT;
	}
	/* IV. Receive Sequence within Window Size*/
	else if (SEQ_SMALLER(seq, (((ba_entry->LastIndSeq + ba_entry->BAWinSize + 1)) & MAXSEQ), MAXSEQ)) {
		if (ba_ctl->dbg_flag & SN_DUMP_WITHIN)
			ba_resource_dump_all(pAd, SN_HISTORY);
		ba_enqueue_reordering_packet(pAd, ba_entry, rx_blk, wdev_idx);
		ba_entry->PreviousReorderCase = WITHIN;
	}
	/* V. Receive seq surpasses Win(lastseq + nMSDU). So refresh all reorder buffer*/
	else {
		UINT16 WinStartSeq, TmpSeq;

		if (ba_ctl->dbg_flag & SN_DUMP_SURPASS)
			ba_resource_dump_all(pAd, SN_HISTORY);

		tr_cnt->ba_sn_large_win_end++;
#ifdef IXIA_C50_MODE
		pAd->rx_cnt.rx_surpass_drop[rx_blk->wcid]++;
#endif
		ba_entry->ba_sn_large_win_end++;

		WinStartSeq = SEQ_INC(SEQ_SUB(seq, ba_entry->BAWinSize, MAXSEQ), 1, MAXSEQ);
		ba_indicate_reordering_mpdus_le_seq(pAd, ba_ctl, ba_entry, (WinStartSeq - 1) & MAXSEQ);

		update_last_in_sn(ba_entry, (WinStartSeq - 1) & MAXSEQ);

		TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, ba_ctl, ba_entry, ba_entry->LastIndSeq);

		if (TmpSeq != INVALID_RCV_SEQ)
			update_last_in_sn(ba_entry, TmpSeq);

		ba_entry->PreviousReorderCase = SURPASS;

		goto ba_reorder_check;
	}

	ba_entry->PreviousAmsduState = rx_blk->AmsduState;
	ba_entry->PreviousSN = seq;

	if ((rx_blk->AmsduState == MSDU_FORMAT) || (rx_blk->AmsduState == FINAL_AMSDU_FORMAT)) {
		NdisAcquireSpinLock(&ba_entry->RxReRingLock);
		ba_entry->CurMpdu = NULL;
		NdisReleaseSpinLock(&ba_entry->RxReRingLock);
	}
}

VOID ba_refresh_bar(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_INFO *ba_info;
	struct BA_ORI_ENTRY *pBAEntry;
	USHORT idx;
	UCHAR i;

	if (pEntry == NULL)
		return;

	ba_info = &pEntry->ba_info;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;
		MAC_TABLE_ENTRY *setup_pEntry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry)
			goto end;

		setup_pEntry = mld_entry_link_select(mld_entry);
		if (!IS_VALID_ENTRY(setup_pEntry))
			goto end;

		pEntry = setup_pEntry;
		pAd = pEntry->pAd;
		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	for (i = 0; i < NUM_OF_TID; i++) {
		idx = ba_info->OriWcidArray[i];

		if (idx == 0)
			continue;

		pBAEntry = &ba_ctl->BAOriEntry[idx];
		pBAEntry->pEntry = pEntry;

		if (pBAEntry->ORI_BA_Status == Originator_Done)
			SendRefreshBAR(pAd, pEntry, pBAEntry->TID);
	}

end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif /* DOT11_EHT_BE */
}

VOID ba_refresh_bar_all(RTMP_ADAPTER *pAd)
{
	UINT16 wcid;
	PMAC_TABLE_ENTRY pEntry = NULL;

	for (wcid = 0; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		pEntry = entry_get(pAd, wcid);

		if (IS_ENTRY_NONE(pEntry))
			continue;

#ifdef SW_CONNECT_SUPPORT
		if (hc_is_sw_wcid(pAd, wcid))
			continue;
#endif /* SW_CONNECT_SUPPORT */

		/* send BAR */
		ba_refresh_bar(pAd, pEntry);
	}
}


static void ba_autotest_handle(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UINT8 UPriority)
{
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);
	struct BA_INFO *ba_info = &pEntry->ba_info;
	struct BA_ORI_ENTRY *pBAEntry;
	USHORT Idx;

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();

	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry)
			goto end;

		ba_info = &mld_entry->ba_info;
	}
#endif /* DOT11_EHT_BE */

	if ((ba_info->TxBitmap & (1 << UPriority)) != 0) {
		Idx = ba_info->OriWcidArray[UPriority];
		if (Idx == 0)
			goto end;

		pBAEntry = &ba_ctl->BAOriEntry[Idx];
		MTWF_PRINT(
			"<addba_info>: sta_mac="MACSTR", tid=%d, buffersize=%d\n",
			MAC2STR(pEntry->Addr), UPriority, pBAEntry->BAWinSize);
	}

end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
}

VOID ba_ori_session_start(RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 UPriority)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	struct BA_INFO *ba_info;
	struct wifi_dev *wdev;
	UINT8 ba_en;
	BOOLEAN sta_is_ps = FALSE;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = entry_get(pAd, wcid);
	tr_entry = tr_entry_get(pAd, wcid);

	if (!IS_VALID_ENTRY(pEntry)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			 "Invalid entry\n");
		return;
	}

	if (pEntry->PsMode != PWR_ACTIVE)
		sta_is_ps = TRUE;

	ba_info = &pEntry->ba_info;

#ifdef SW_CONNECT_SUPPORT
	/* S/W no AGG support */
	if (IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
				"S/W no AGG support!\n");
		return;
	}
#endif /* SW_CONNECT_SUPPORT */

#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();
	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;
		MAC_TABLE_ENTRY *setup_pEntry;
		MAC_TABLE_ENTRY *entry_ptr;
		int i = 0;

		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return;
		}

		setup_pEntry = mld_entry_link_select(mld_entry);
		if (!IS_VALID_ENTRY(setup_pEntry)) {
			mt_rcu_read_unlock();
			return;
		}

		wcid = setup_pEntry->wcid;
		pEntry = setup_pEntry;
		tr_entry = tr_entry_get(pAd, wcid);
		ba_info = &mld_entry->ba_info;

		for (i = 0; i < MLD_LINK_MAX; i++) {
			entry_ptr = mld_entry->link_entry[i];
			if (!entry_ptr)
				continue;
			if (entry_ptr->PsMode == PWR_ACTIVE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
					"MLO link %d (%pM) active!\n", i, entry_ptr->Addr);
				sta_is_ps = FALSE;
				break;
			}
		}
	}
#endif /* DOT11_EHT_BE */

	pAd = pEntry->pAd;
	wdev = pEntry->wdev;

	if (!pAd || !wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"%s is NULL\n", pAd ? "wdev" : "pAd");
		goto end;

	}

	if (sta_is_ps) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"STA is PS, skip this BA session start\n");
		goto end;
	}

	ba_en = wlan_config_get_ba_enable(wdev);

	if (ba_info->AutoTest)
		ba_autotest_handle(pAd, pEntry, UPriority);

	/* BA has already been setup */
	if ((ba_info->TxBitmap & (1 << UPriority)) != 0)
		goto end;

	if (!ba_en) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"HT_AutoBA = 0, disable BA\n");
		goto end;
	}

	/* TODO: shiang-usw, fix me for pEntry, we should replace this paramter as tr_entry! */
	if ((tr_entry && tr_entry->EntryType != ENTRY_CAT_MCAST && VALID_UCAST_ENTRY_WCID(pAd, wcid)) &&
		(IS_HT_STA(pEntry) || IS_HE_6G_STA(pEntry->cap.modes))) {

		if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))) {
			ba_ori_session_setup(pAd, wcid, UPriority, 0);
		}
	}
end:
#ifdef DOT11_EHT_BE
	mt_rcu_read_unlock();
#endif
}

VOID ba_ctl_init(struct physical_device *ph_dev)
{
	int i;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ph_dev);

	OS_NdisAllocateSpinLock(&ba_ctl->BATabLock);
	NdisAcquireSpinLock(&ba_ctl->BATabLock);
	ba_ctl->numAsOriginator = 0;
	ba_ctl->numAsRecipient = 0;
	ba_ctl->numDoneOriginator = 0;
	NdisReleaseSpinLock(&ba_ctl->BATabLock);
	ba_ctl->ba_timeout_check = FALSE;
	ba_ctl->dbg_flag |= SN_HISTORY;
	os_zero_mem((UCHAR *)&ba_ctl->ba_timeout_bitmap[0], sizeof(UINT32) * BA_TIMEOUT_BITMAP_LEN);
#ifdef RX_RPS_SUPPORT
	os_zero_mem((UCHAR *)&ba_ctl->ba_timeout_bitmap_per_cpu[0][0], sizeof(UINT32) * BA_TIMEOUT_BITMAP_LEN * NR_CPUS);
#endif
	atomic_set(&ba_ctl->SetFlushTimer, -1);
	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		OS_NdisAllocateSpinLock(&(ba_ctl->BARecEntry[i].RxReRingLock));
		NdisAcquireSpinLock(&(ba_ctl->BARecEntry[i].RxReRingLock));
		ba_ctl->BARecEntry[i].REC_BA_Status = Recipient_NONE;
		ba_ctl->BARecEntry[i].WaitWM = FALSE;
		ba_ctl->BARecEntry[i].Postpone_Action = 0;
		NdisReleaseSpinLock(&(ba_ctl->BARecEntry[i].RxReRingLock));
#ifdef CONFIG_BA_REORDER_ARRAY_SUPPORT
		ba_ctl->free_ba_buf_id[i] = i;
#endif
	}

	for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++)
		ba_ctl->BAOriEntry[i].ORI_BA_Status = Originator_NONE;

	/*  Allocate BA Reordering memory */
	if (ba_reordering_resource_init(ph_dev, MAX_REORDERING_MPDU_NUM) != TRUE) {
		ba_ctl->rx_ba_disable = TRUE;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"Allocate BA Reordering fail. We need to disable rx ba.\n");
	} else
		ba_ctl->rx_ba_disable = FALSE;
}

VOID ba_ctl_exit(struct physical_device *ph_dev)
{
	int i;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ph_dev);

	ba_reordering_resource_release(ph_dev);

	if (ba_ctl) {
		for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++)
			NdisFreeSpinLock(&ba_ctl->BARecEntry[i].RxReRingLock);

		NdisFreeSpinLock(&ba_ctl->BATabLock);
	}
}
