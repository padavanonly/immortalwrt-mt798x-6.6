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
	bs.c
*/

#ifdef CCN67_BS_SUPPORT
#include "rt_config.h"

#ifndef APMT2_PEER_PROBE_REQ
#define APMT2_PEER_PROBE_REQ		0
#endif
#ifndef APMT2_PEER_DISASSOC_REQ
#define APMT2_PEER_DISASSOC_REQ     1
#endif
#ifndef APMT2_PEER_ASSOC_REQ
#define APMT2_PEER_ASSOC_REQ        2
#endif
#ifndef APMT2_PEER_AUTH_REQ
#define APMT2_PEER_AUTH_REQ			3
#endif

INT BS_TableInit(PRTMP_ADAPTER pAd)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	struct bs_cli_table *init_table = NULL;

	init_table = &pAd->ApCfg.BSTable;
	NdisZeroMemory(init_table, sizeof(struct bs_cli_table));
	OS_NdisAllocateSpinLock(&init_table->Lock);
	init_table->priv = (VOID *) pAd;
	init_table->Band = WMODE_CAP_5G(pMbss->wdev.PhyMode)?BAND_5G : BAND_24G;
	init_table->Channel = pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.channel;
	init_table->bInitialized = TRUE;

	return BS_SUCCESS;
}

INT BS_TableRelease(struct bs_cli_table *table)
{
	INT ret_val = BS_SUCCESS;

	if (table->bInitialized == FALSE)
		return BS_NOT_INITIALIZED;

	OS_NdisFreeSpinLock(&table->Lock);

	table->bInitialized = FALSE;

	if (ret_val != BS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_CCN67, CATCCN67_BS, DBG_LVL_ERROR,
					"error code = %d!\n", ret_val);
	}
	return ret_val;
}

struct bs_cli_entry *
BS_InsertEntry(
	struct bs_cli_table *table,
	PUCHAR pSrcAddr,
	MLME_QUEUE_ELEM *Elem,
	PEER_PROBE_REQ_PARAM *ProbeReqParam)
{
	INT i;
	UCHAR HashIdx;
	struct bs_cli_entry *entry = NULL, *this_entry = NULL;

	if (table->Size >= BS_MAX_TABLE_SIZE) {
		MTWF_DBG(NULL, DBG_CAT_CCN67, CATCCN67_BS, DBG_LVL_WARN, "Table is full!\n");
		return NULL;
	}

	NdisAcquireSpinLock(&table->Lock);

	for (i = 0; i < BS_MAX_TABLE_SIZE; i++) {
		entry = &table->Entry[i];

		/* pick up the first available vacancy*/
		if (entry->bValid == FALSE)	{
			NdisZeroMemory(entry, sizeof(struct bs_cli_entry));
			/* Fill Entry */
			NdisGetSystemUpTime(&entry->entryTime);
			entry->withhold_time = entry->entryTime;
			COPY_MAC_ADDR(entry->Addr, pSrcAddr);
			if (ProbeReqParam) {
				if (ProbeReqParam->SsidLen > 0) {
					NdisCopyMemory(entry->Ssid, ProbeReqParam->Ssid, ProbeReqParam->SsidLen);
					entry->SsidLen = ProbeReqParam->SsidLen;
				}
			}
			if (Elem)
				entry->Rssi = Elem->rssi_info.raw_rssi[0];

			entry->BS_Sta_State = BS_STA_INIT;
			entry->Auth_Count = 0;
			entry->Probe_Count = 0;
			entry->bValid = TRUE;
			break;
		}

		entry = NULL;
	}

	if (entry) {
		/* add this MAC entry into HASH table */
		HashIdx = MAC_ADDR_HASH_INDEX(pSrcAddr);

		if (table->Hash[HashIdx] == NULL)
			table->Hash[HashIdx] = entry;

		else {
			this_entry = table->Hash[HashIdx];

			while (this_entry->pNext != NULL)
				this_entry = this_entry->pNext;

			this_entry->pNext = entry;
		}
		table->Size++;
	}

	NdisReleaseSpinLock(&table->Lock);
	return entry;
}

INT BS_DeleteEntry(struct bs_cli_table *table, PUCHAR pAddr, UINT32 Index)
{
	USHORT HashIdx;
	struct bs_cli_entry *entry, *pre_entry, *this_entry;
	INT ret_val = BS_SUCCESS;

	NdisAcquireSpinLock(&table->Lock);

	if (Index >= BS_MAX_TABLE_SIZE) {
		if (pAddr == NULL) {
			NdisReleaseSpinLock(&table->Lock);
			return BS_INVALID_ARG;
		}

		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		entry = table->Hash[HashIdx];

		while (entry) {
			if (MAC_ADDR_EQUAL(pAddr, entry->Addr)) {
				/* this is the entry we're looking for */
				break;
			}
			entry = entry->pNext;
		}

		if (entry == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CCN67, CATCCN67_BS, DBG_LVL_DEBUG,
				"Index=%u, "MACSTR", Entry not found.\n",
				Index, MAC2STR(pAddr));
			NdisReleaseSpinLock(&table->Lock);
			return BS_INVALID_ARG;
		}
	} else {
		entry = &table->Entry[Index];
		HashIdx = MAC_ADDR_HASH_INDEX(entry->Addr);
	}

	if (entry->bValid) {
		pre_entry = NULL;
		this_entry = table->Hash[HashIdx];
		ASSERT(this_entry);

		if (this_entry != NULL) {
			/* update Hash list*/
			do {
				if (this_entry == entry) {
					if (pre_entry == NULL)
						table->Hash[HashIdx] = entry->pNext;
					else
						pre_entry->pNext = entry->pNext;

					break;
				}

				pre_entry = this_entry;
				this_entry = this_entry->pNext;
			} while (this_entry);
		}
		/* not found !!!*/
		ASSERT(this_entry != NULL);

		NdisZeroMemory(entry->Addr, MAC_ADDR_LEN);
		entry->pNext = NULL;
		entry->bValid = FALSE;
		table->Size--;
	}
	NdisReleaseSpinLock(&table->Lock);
	return ret_val;
}

VOID AddBlackListStaInList(PRTMP_ADAPTER pAd, PUCHAR pMacAddr)
{
	struct bs_cli_table *table = &pAd->ApCfg.BSTable;
	struct bs_cli_entry *entry = NULL;

	entry = BS_TableLookup(table, pMacAddr);

	if (entry == NULL)
		entry = BS_InsertEntry(table, pMacAddr, NULL, NULL);

	if (entry != NULL) {
		NdisAcquireSpinLock(&table->Lock);
		/*Bs client entry found mark it as black listed */
		entry->isBlackListed = TRUE;
		entry->withhold_count = 3;
		entry->withhold_age = 1;
		NdisGetSystemUpTime(&entry->entryTime);
		entry->withhold_time = entry->entryTime;
		NdisReleaseSpinLock(&table->Lock);
	}
}
VOID DelBlackListStaFromList(struct bs_cli_table *table, struct bs_cli_entry *cli_entry)
{
	USHORT HashIdx;
	struct bs_cli_entry *pre_entry = NULL;
	struct bs_cli_entry *this_entry;

	HashIdx = MAC_ADDR_HASH_INDEX(cli_entry->Addr);

	if (cli_entry->bValid) {
		this_entry = table->Hash[HashIdx];
		if (this_entry != NULL) {
			/* update Hash list*/
			do {
				if (this_entry == cli_entry) {
					if (pre_entry == NULL)
						table->Hash[HashIdx] = cli_entry->pNext;
					else
						pre_entry->pNext = cli_entry->pNext;

					break;
				}

				pre_entry = this_entry;
				this_entry = this_entry->pNext;
			} while (this_entry);
		}
	}
	NdisZeroMemory(cli_entry, sizeof(struct bs_cli_entry));
	table->Size--;
}


struct bs_cli_entry *
BS_TableLookup(struct bs_cli_table *table, PUCHAR pAddr)
{
	ULONG HashIdx;
	struct bs_cli_entry *entry = NULL, *pSearchEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	entry = table->Hash[HashIdx];

	NdisAcquireSpinLock(&table->Lock);
	while (entry && entry->bValid) {
		if (MAC_ADDR_EQUAL(entry->Addr, pAddr)) {
			pSearchEntry = entry;
			break;
		}
		entry = entry->pNext;
	}

	NdisReleaseSpinLock(&table->Lock);
	return pSearchEntry;
}

BOOLEAN CheckWithholdLimit(PRTMP_ADAPTER	pAd, BSS_STRUCT *mbss, struct bs_cli_entry *entry, UINT8 frameType)
{
	if (frameType == APMT2_PEER_PROBE_REQ) {
		entry->probe_withhold++;
		if (entry->probe_withhold >= PD_GET_BB_WITHOLD_LIMIT(pAd->physical_dev))
			return TRUE;
		mbss->is_probe_withheld++;
		pAd->WlanCounters.ProbeWithHeldCount++;
	} else if (frameType == APMT2_PEER_AUTH_REQ) {
		entry->auth_withhold++;
		if (entry->auth_withhold >= PD_GET_BB_WITHOLD_LIMIT(pAd->physical_dev))
			return TRUE;
		mbss->is_auth_witheld++;
		pAd->WlanCounters.AuthWithHeldCount++;
	}
	return FALSE;
}
BOOLEAN CheckClientOnOtherBand(PRTMP_ADAPTER pAd, struct bs_cli_entry *entry, UINT16 rssi_threshold)
{
	BOOLEAN needToWait = TRUE;
	BOOLEAN ssidFound = FALSE;
	UINT8 apidx = 0;
	struct bs_cli_entry *other_entry = BS_TableLookup(&pAd->ApCfg.BSTable, entry->Addr);

	if (entry->SsidLen > 0) {
		for (apidx = 0; ((apidx < pAd->ApCfg.BssidNum) && VALID_MBSS(pAd, apidx)); apidx++) {
			BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[apidx];

			if ((entry->SsidLen == mbss->SsidLen) &&
				(!NdisCmpMemory(entry->Ssid, mbss->Ssid, entry->SsidLen))) {
				ssidFound =  TRUE;
				break;
			}
		}
	}
	if (!ssidFound)
		return FALSE;

	if (other_entry != NULL) {
		/*need to put same ssid check */
		if (other_entry->Rssi < rssi_threshold)
			needToWait = FALSE;
		else
			needToWait = TRUE;
	} else {
		MTWF_DBG(NULL, DBG_CAT_CCN67, CATCCN67_BS, DBG_LVL_DEBUG,
				"Same entry not detected on other band yet. Need to wait\n");
	}
	return needToWait;
}
BOOLEAN BS_CheckConnectionReq(
	PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev,
	PUCHAR pSrcAddr,
	MLME_QUEUE_ELEM *Elem,
	PEER_PROBE_REQ_PARAM *ProbeReqParam)
{
	UINT8 FrameType = Elem->MsgType;
	struct bs_cli_table *table = &pAd->ApCfg.BSTable;
	struct bs_cli_entry *entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT8 meshEnabled = PD_GET_MESH_ENABLED(pAd->physical_dev);
	UINT32 staThreshold = PD_GET_BB_STA_THRESHOLD(pAd->physical_dev);
	BSS_STRUCT *mbss = wdev->func_dev;

	if (!wdev)
		return TRUE;

	if ((wdev->func_idx < 0) || (wdev->func_idx >= MAX_BEACON_NUM))
		return TRUE;

	if (!table->bEnabled)
		return TRUE;

	entry = BS_TableLookup(table, pSrcAddr);
	if (entry && entry->isBlackListed)
		return FALSE;

	pEntry = MacTableLookup(pAd, pSrcAddr);
	if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
		return TRUE;

	if (mbss->iv_bb_enabled && meshEnabled && (pAd->MacTab->ClientCount >= staThreshold)) {
		/*need to put withhold logic for probe or auth */
		UINT32 clientLimitOnBand = 0;
		UINT32 staLimitOnOtherBand = 0;
		UINT8 band_idx = hc_get_hw_band_idx(pAd);
		PRTMP_ADAPTER	pAd_other;
		UINT8 other_band_idx;

		if (entry == NULL)
			entry = BS_InsertEntry(table, pSrcAddr, Elem, ProbeReqParam);

		if (PD_GET_BB_BAND_THRESHOLD(pAd->physical_dev, band_idx)) {
			clientLimitOnBand = (pAd->MacTab->ClientCount + 1) * (PD_GET_BB_BAND_THRESHOLD(pAd->physical_dev, band_idx))/100;
			staLimitOnOtherBand = (pAd->MacTab->ClientCount + 1) - clientLimitOnBand;
		}

		if (pAd->ApCfg.EntryClientCount >= clientLimitOnBand) {
			/*band limit exhausted need to check for rssi*/
			/* same client found on another band and */
			/* if rssi on this band > than threshold on aonther band*/
			/* allow client on this band otherwise not */
			if (band_idx == BAND0)
				other_band_idx = BAND1;
			else
				other_band_idx = BAND0;

			pAd_other = physical_device_get_mac_adapter_by_band(pAd->physical_dev, other_band_idx);
			if (pAd_other) {
				if (pAd_other->ApCfg.EntryClientCount < staLimitOnOtherBand) {
					BOOLEAN needToWait;

					needToWait = CheckClientOnOtherBand(pAd_other, entry, mbss->iv_bb_rssi_thresh);

					if (needToWait)
						return CheckWithholdLimit(pAd, mbss, entry, FrameType);
					else
						return TRUE;
				} else {
					MTWF_DBG(NULL, DBG_CAT_CCN67, CATCCN67_BS, DBG_LVL_DEBUG,
							"Station Limit on other band not available\n");
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_CCN67, CATCCN67_BS, DBG_LVL_DEBUG,
					"other band not configured need to wait\n");
				return CheckWithholdLimit(pAd, mbss, entry, FrameType);
			}
		}
	}
	return TRUE;
}

BOOLEAN BlackListStaTimeUpdate(struct bs_cli_entry *cli_entry, ULONG curTime)
{
	BOOLEAN entryExpired = FALSE;

	if (cli_entry->isBlackListed) {
		if (RTMP_TIME_AFTER(curTime, cli_entry->withhold_time + (cli_entry->withhold_age * OS_HZ / 1000))) {
			cli_entry->withhold_count++;
			if (cli_entry->withhold_count == cli_entry->withhold_limit)
				entryExpired = TRUE;
		}
	}
	return entryExpired;
}
VOID Bs_TableMaintain(
	IN PRTMP_ADAPTER pAd)
{
	struct bs_cli_table *table = &pAd->ApCfg.BSTable;
	struct bs_cli_entry *cli_entry = NULL;
	ULONG Now;
	UINT32 count = 0, idx;

	if (!PD_GET_MESH_ENABLED(pAd->physical_dev))
		return;
	NdisGetSystemUpTime(&Now);

	NdisAcquireSpinLock(&table->Lock);
	for (idx = 0; idx < BS_MAX_TABLE_SIZE; idx++) {
		cli_entry = (struct bs_cli_entry *)(&table[idx].Entry);
		if (count >= table->Size)
			break;
		if (cli_entry->bValid) {
			count++;
			if (BlackListStaTimeUpdate(cli_entry, Now))
				DelBlackListStaFromList(table, cli_entry);
		}
	}
	NdisReleaseSpinLock(&table->Lock);
}

#endif /* CCN67_BS_SUPPORT */

