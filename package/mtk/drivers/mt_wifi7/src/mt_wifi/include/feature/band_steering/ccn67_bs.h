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
	band_steering_def.h
*/

#ifndef _BS_H_
#define __BS_H__

#ifdef CCN67_BS_SUPPORT

INT BS_Init(PRTMP_ADAPTER pAd);
INT BS_Release(PRTMP_ADAPTER pAd);
INT BS_TableInit(PRTMP_ADAPTER pAd);
INT BS_TableRelease(struct bs_cli_table *table);
struct bs_cli_entry *
BS_TableLookup(struct bs_cli_table *table, PUCHAR pAddr);
struct bs_entry_list *
FindBsEntryInList(PLIST_HEADER pBsList, PUCHAR pMacAddr);
VOID AddBlackListStaInList(PRTMP_ADAPTER pAd, PUCHAR pMacAddr);
VOID DelBlackListStaFromList(struct bs_cli_table *table, struct bs_cli_entry *cli_entry);

VOID AddBsEntryInList(PLIST_HEADER pBsList, PUCHAR pMacAddr);
VOID DelBsEntryInList(PLIST_HEADER pBsList, PUCHAR pMacAddr);
VOID ClearBsEntryList(PLIST_HEADER pBsList);
VOID Bs_TableMaintain(PRTMP_ADAPTER pAd);


BOOLEAN BS_CheckConnectionReq(
	PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev,
	PUCHAR pSrcAddr,
	MLME_QUEUE_ELEM *Elem,
	PEER_PROBE_REQ_PARAM *ProbeReqParam);

#define IS_VALID_MAC(addr) \
	((addr[0])|(addr[1])|(addr[2])|(addr[3])|(addr[4])|(addr[5]))

#endif /* CCN67_BS_SUPPORT */
#endif /* _BS_H_ */

