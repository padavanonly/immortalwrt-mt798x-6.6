#ifdef IGMP_SNOOP_SUPPORT

#include "rt_config.h"
#include "ipv6.h"
#include "igmp/igmp_snoop.h"

#ifdef IGMP_TVM_SUPPORT
extern UCHAR IGMP_TVM_OUI[];
#endif /* IGMP_TVM_SUPPORT */

UINT16 IPv6MulticastFilterExclued[] = {
	IPV6_NEXT_HEADER_ICMPV6,	/* ICMPv6. */
	IPV6_NEXT_HEADER_PIM,		/* PIM. */
};
#define IPV6_MULTICAST_FILTER_EXCLUED_SIZE  \
	(sizeof(IPv6MulticastFilterExclued) / sizeof(UINT16))

PMULTICAST_FILTER_TABLE m2u_pMulticastFilterTable;

static inline void initFreeEntryList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList)
{
	int i;

	for (i = 0; i < FREE_MEMBER_POOL_SIZE; i++)
		insertTailList(pList, (RT_LIST_ENTRY *)&(pMulticastFilterTable->freeMemberPool[i]));

	return;
}

static inline PMEMBER_ENTRY AllocaGrpMemberEntry(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable)
{
	PMEMBER_ENTRY pMemberEntry;
	OS_SEM_LOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
	pMemberEntry = (PMEMBER_ENTRY)removeHeadList(&pMulticastFilterTable->freeEntryList);
	OS_SEM_UNLOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
	return (PMEMBER_ENTRY)pMemberEntry;
}

static inline VOID FreeGrpMemberEntry(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PMEMBER_ENTRY pEntry)
{
	OS_SEM_LOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
	insertTailList(&pMulticastFilterTable->freeEntryList, (RT_LIST_ENTRY *)pEntry);
	OS_SEM_UNLOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
}

static VOID IGMPTableDisplay(
	IN PRTMP_ADAPTER pAd);

static BOOLEAN isIgmpMacAddr(
	IN PUCHAR pMacAddr);

static VOID InsertIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr,
	IN MulticastFilterEntryType type);

static VOID DeleteIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr);

static VOID DeleteIgmpMemberList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList);

#ifdef A4_CONN
/* Whether member is present on MWDS link */
BOOLEAN isMemberOnMWDSLink(VOID *prMemberEntry);
#endif

/*
    ==========================================================================
    Description:
	This routine init the entire IGMP table.
    ==========================================================================
 */
VOID MulticastFilterTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable)
{
#ifdef IGMP_TVM_SUPPORT
	UCHAR i = 0;
#endif /* IGMP_TVM_SUPPORT*/

	if (m2u_pMulticastFilterTable) {
		*ppMulticastFilterTable = m2u_pMulticastFilterTable;
		(*ppMulticastFilterTable)->ref_cnt++;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
				"%s(): m2u_pMulticastFilterTable(%p) has been inited, ref_cnt %d\n",
				__func__, m2u_pMulticastFilterTable, m2u_pMulticastFilterTable->ref_cnt);
		return;
	}

	/* Initialize MAC table and allocate spin lock */
	os_alloc_mem(NULL, (UCHAR **)ppMulticastFilterTable, sizeof(MULTICAST_FILTER_TABLE));

	if (*ppMulticastFilterTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "unable to alloc memory for Multicase filter table, size=%lu\n",
				 (ULONG)sizeof(MULTICAST_FILTER_TABLE));
		return;
	}

	NdisZeroMemory(*ppMulticastFilterTable, sizeof(MULTICAST_FILTER_TABLE));
	NdisAllocateSpinLock(pAd, &((*ppMulticastFilterTable)->MulticastFilterTabLock));
	NdisAllocateSpinLock(pAd, &((*ppMulticastFilterTable)->FreeMemberPoolTabLock));
	initList(&((*ppMulticastFilterTable)->freeEntryList));
	initFreeEntryList(*ppMulticastFilterTable, &((*ppMulticastFilterTable)->freeEntryList));

	m2u_pMulticastFilterTable = *ppMulticastFilterTable;
	(*ppMulticastFilterTable)->ref_cnt++;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"%s(): init m2u_pMulticastFilterTable(%p), ref_cnt %d\n",
			__func__, m2u_pMulticastFilterTable, m2u_pMulticastFilterTable->ref_cnt);

#ifdef IGMP_TVM_SUPPORT
	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
		(*ppMulticastFilterTable)->Content[i].AgeOutTime = IGMPMAC_TB_ENTRY_AGEOUT_TIME;
	}
#endif /* IGMP_TVM_SUPPORT */
	return;
}

/*
    ==========================================================================
    Description:
	This routine reset the entire IGMP table.
    ==========================================================================
 */
VOID MultiCastFilterTableReset(RTMP_ADAPTER *pAd,
							   IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable)
{

#ifndef IGMP_SNOOPING_NON_OFFLOAD
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		return;
	}
#endif

	if (*ppMulticastFilterTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Multicase filter table is not ready.\n");
		return;
	}

	if (--(*ppMulticastFilterTable)->ref_cnt) {
		ppMulticastFilterTable = NULL;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
				"%s(): m2u_pMulticastFilterTable(%p), ref_cnt %d\n",
				__func__, m2u_pMulticastFilterTable, m2u_pMulticastFilterTable->ref_cnt);
		return;
	}

	NdisFreeSpinLock(&((*ppMulticastFilterTable)->FreeMemberPoolTabLock));
	NdisFreeSpinLock(&((*ppMulticastFilterTable)->MulticastFilterTabLock));
	os_free_mem(*ppMulticastFilterTable);
	*ppMulticastFilterTable = NULL;

	m2u_pMulticastFilterTable = NULL;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"%s(): m2u_pMulticastFilterTable(%p)\n", __func__, m2u_pMulticastFilterTable);
}

/*
    ==========================================================================
    Description:
	Display all entrys in IGMP table
    ==========================================================================
 */
static VOID IGMPTableDisplay(
	IN PRTMP_ADAPTER pAd)
{
	int i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;
#ifdef IGMP_SNOOPING_DENY_LIST
	MULTICAST_DENY_LIST_FILTER_TABLE * deny_list_tbl = pAd->pMcastDLTable;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR ifIndex = pObj->ioctl_if;
	PNET_DEV pDev = (ifIndex == MAIN_MBSSID) ?
			(pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);
#endif

	if (pMulticastFilterTable == NULL) {
		MTWF_PRINT("Multicase filter table is not ready.\n");
		return;
	}
	OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	/* if FULL, return */
	if (pMulticastFilterTable->Size == 0) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Table empty.\n");
	} else {
	/* allocate one MAC entry */
		for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
			/* pick up the first available vacancy */
			if (pMulticastFilterTable->Content[i].Valid == TRUE) {
				PMEMBER_ENTRY pMemberEntry = NULL;

				pEntry = &pMulticastFilterTable->Content[i];
				MTWF_PRINT("IF(%s) entry#%d, type=%s, GId=("IPV6STR") memCnt=%d\n",
					RTMP_OS_NETDEV_GET_DEVNAME(pEntry->net_dev), i,
					(pEntry->type == 0 ? "static" : "dynamic"),
					PRINT_IPV6(pEntry->Addr), IgmpMemberCnt(&pEntry->MemberList));
				pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

				while (pMemberEntry) {
					MTWF_PRINT("member mac=("MACSTR")\n", MAC2STR(pMemberEntry->Addr));
					pMemberEntry = pMemberEntry->pNext;
				}
			}
		}
	}
	OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

#ifdef IGMP_SNOOPING_DENY_LIST
	if (deny_list_tbl == NULL)
		return;

	MTWF_PRINT("\nIGMP Snooping deny list table:\n");
	OS_SEM_LOCK(&deny_list_tbl->MulticastDenyTabLock);
	for (i = 0; i < IGMP_DENY_TABLE_SIZE_MAX; i++) {
		if (deny_list_tbl->entry[i].valid) {
			MTWF_PRINT("IF(%s) entry #%d, GrpId: "IPV6STR"/%d\n",
						RTMP_OS_NETDEV_GET_DEVNAME(pDev), i,
						PRINT_IPV6(deny_list_tbl->entry[i].addr),
						((deny_list_tbl->entry[i].addr[0]) ? (deny_list_tbl->entry[i].prefix) : (deny_list_tbl->entry[i].prefix - 96)));
		}
	}
	OS_SEM_UNLOCK(&deny_list_tbl->MulticastDenyTabLock);
#endif
	return;
}

/*
    ==========================================================================
    Description:
	Add and new entry into MAC table
    ==========================================================================
 */
BOOLEAN MulticastFilterTableInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN UINT8 type,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT16 wcid)
{
	UCHAR HashIdx;
	int i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL, *pCurrEntry, *pPrevEntry;
	PMEMBER_ENTRY pMemberEntry;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
#ifdef IGMP_TVM_SUPPORT
	UINT32 AgeOutTime = IGMPMAC_TB_ENTRY_AGEOUT_TIME;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
#endif /* IGMP_TVM_SUPPORT */

	if (pMulticastFilterTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Multicase filter table is not ready.\n");
		return FALSE;
	}

	OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	/* if FULL, return */
	if (pMulticastFilterTable->Size >= MAX_LEN_OF_MULTICAST_FILTER_TABLE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "Multicase filter table full. max-entries = %d\n",
				 MAX_LEN_OF_MULTICAST_FILTER_TABLE);
		OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
		return FALSE;
	}

	if (pMulticastFilterTable->size_cfg.enable &&
		(pMulticastFilterTable->size_cfg.used_num[band_idx] >= pMulticastFilterTable->size_cfg.cfg_num[band_idx])) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_NOTICE,
				"Multicase filter table full for this band, IF(%s). used num = %d, cfg num = %d\n",
				 RTMP_OS_NETDEV_GET_DEVNAME(dev), pMulticastFilterTable->size_cfg.used_num[band_idx],
				 pMulticastFilterTable->size_cfg.cfg_num[band_idx]);
		OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
		return FALSE;
	}
	OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

#ifdef IGMP_TVM_SUPPORT
		pMacEntry = entry_get(pAd, wcid);
		if (pMacEntry && pMacEntry->wdev) {
			AgeOutTime = pMacEntry->wdev->u4AgeOutTime;
		}
#endif /* IGMP_TVM_SUPPORT */

	/* check the rule is in table already or not. */
	pEntry = MulticastFilterTableLookup(pMulticastFilterTable, pGrpId, dev);
	if (pEntry) {
		/* doesn't indicate member mac address. */
		if (pMemberAddr == NULL)
			return FALSE;

		pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

		while (pMemberEntry) {
			if (MAC_ADDR_EQUAL(pMemberAddr, pMemberEntry->Addr)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "already in Members list.\n");
				return FALSE;
			}

			pMemberEntry = pMemberEntry->pNext;
		}
	}

	OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	do {
		ULONG Now;
		PRTMP_ADAPTER entry_pAd;
		UINT8 entry_band_idx;

		/* the multicast entry already exist but doesn't include the member yet. */
		if (pEntry != NULL && pMemberAddr != NULL) {
#ifdef IGMP_TVM_SUPPORT
			pEntry->AgeOutTime = AgeOutTime;
#endif /* IGMP_TVM_SUPPORT */
			InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr, type);
			break;
		}

		/* allocate one MAC entry */
		for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
			/* pick up the first available vacancy */
			pEntry = &pMulticastFilterTable->Content[i];
			NdisGetSystemUpTime(&Now);

			if ((pEntry->Valid == TRUE) && (pEntry->type == MCAT_FILTER_DYNAMIC)
				&& RTMP_TIME_AFTER(Now, pEntry->lastTime +
#ifdef IGMP_TVM_SUPPORT
				pEntry->AgeOutTime
#else
				IGMPMAC_TB_ENTRY_AGEOUT_TIME
#endif /* IGMP_TVM_SUPPORT */
				)) {
				PMULTICAST_FILTER_TABLE_ENTRY pHashEntry;
				HashIdx = MULTICAST_IPV6_ADDR_HASH_INDEX(pEntry->Addr);
				pHashEntry = pMulticastFilterTable->Hash[HashIdx];

				if ((pEntry->net_dev == pHashEntry->net_dev)
					&& IPV6_ADDR_EQUAL(pEntry->Addr, pHashEntry->Addr)) {
					pMulticastFilterTable->Hash[HashIdx] = pHashEntry->pNext;
					pMulticastFilterTable->Size--;
					GET_PAD_FROM_NET_DEV(entry_pAd, pEntry->net_dev);
					entry_band_idx = hc_get_hw_band_idx(entry_pAd);
					pMulticastFilterTable->size_cfg.used_num[entry_band_idx]--;
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
							"MCastFilterTableDeleteEntry 1 - Total= %d\n", pMulticastFilterTable->Size);
				} else {
					while (pHashEntry->pNext) {
						pPrevEntry = pHashEntry;
						pHashEntry = pHashEntry->pNext;

						if ((pEntry->net_dev == pHashEntry->net_dev)
							&& IPV6_ADDR_EQUAL(pEntry->Addr, pHashEntry->Addr)) {
							pPrevEntry->pNext = pHashEntry->pNext;
							pMulticastFilterTable->Size--;
							GET_PAD_FROM_NET_DEV(entry_pAd, pEntry->net_dev);
							entry_band_idx = hc_get_hw_band_idx(entry_pAd);
							pMulticastFilterTable->size_cfg.used_num[entry_band_idx]--;
							MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
									"MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size);
							break;
						}
					}
				}

				pEntry->Valid = FALSE;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
			}

			if (pEntry->Valid == FALSE) {
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pEntry->Valid = TRUE;
				COPY_IPV6_ADDR(pEntry->Addr, pGrpId);
				pEntry->net_dev = dev;
				NdisGetSystemUpTime(&Now);
				pEntry->lastTime = Now;
#ifdef IGMP_TVM_SUPPORT
				pEntry->AgeOutTime = AgeOutTime;
#endif /* IGMP_TVM_SUPPORT */
				pEntry->type = (MulticastFilterEntryType)(((UINT8)type) & GROUP_ENTRY_TYPE_BITMASK); /* remove member detail*/
				initList(&pEntry->MemberList);

				if (pMemberAddr != NULL)
					InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr, type);

				pMulticastFilterTable->Size++;
				GET_PAD_FROM_NET_DEV(entry_pAd, pEntry->net_dev);
				entry_band_idx = hc_get_hw_band_idx(entry_pAd);
				pMulticastFilterTable->size_cfg.used_num[entry_band_idx]++;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"MulticastFilterTableInsertEntry -IF(%s) allocate entry #%d, Total= %d\n",
						RTMP_OS_NETDEV_GET_DEVNAME(dev), i, pMulticastFilterTable->Size);
				break;
			}
		}

		/* add this MAC entry into HASH table */
		if (pEntry) {
			HashIdx = MULTICAST_IPV6_ADDR_HASH_INDEX(pGrpId);

			if (pMulticastFilterTable->Hash[HashIdx] == NULL)
				pMulticastFilterTable->Hash[HashIdx] = pEntry;
			else {
				pCurrEntry = pMulticastFilterTable->Hash[HashIdx];

				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;

				pCurrEntry->pNext = pEntry;
			}
		}
	} while (FALSE);

	OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Delete a specified client from MAC table
    ==========================================================================
 */
BOOLEAN MulticastFilterTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT16 wcid)
{
	USHORT HashIdx;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry, *pPrevEntry;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if (pMulticastFilterTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Multicase filter table is not ready.\n");
		return FALSE;
	}

	OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	do {
		PRTMP_ADAPTER entry_pAd;
		UINT8 entry_band_idx;

		HashIdx = MULTICAST_IPV6_ADDR_HASH_INDEX(pGrpId);
		pPrevEntry = pEntry = pMulticastFilterTable->Hash[HashIdx];

		while (pEntry && pEntry->Valid) {
			if ((pEntry->net_dev ==  dev)
				&& IPV6_ADDR_EQUAL(pEntry->Addr, pGrpId))
				break;
			else {
				pPrevEntry = pEntry;
				pEntry = pEntry->pNext;
			}
		}

		/* check the rule is in table already or not. */
		if (pEntry && (pMemberAddr != NULL)) {
			DeleteIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);

			if (IgmpMemberCnt(&pEntry->MemberList) > 0)
				break;
		}

		if (pEntry) {
			if (pEntry == pMulticastFilterTable->Hash[HashIdx]) {
				pMulticastFilterTable->Hash[HashIdx] = pEntry->pNext;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
				GET_PAD_FROM_NET_DEV(entry_pAd, pEntry->net_dev);
				entry_band_idx = hc_get_hw_band_idx(entry_pAd);
				pMulticastFilterTable->size_cfg.used_num[entry_band_idx]--;
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pMulticastFilterTable->Size--;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"MCastFilterTableDeleteEntry 1 - Total= %d\n", pMulticastFilterTable->Size);
			} else {
				pPrevEntry->pNext = pEntry->pNext;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
				GET_PAD_FROM_NET_DEV(entry_pAd, pEntry->net_dev);
				entry_band_idx = hc_get_hw_band_idx(entry_pAd);
				pMulticastFilterTable->size_cfg.used_num[entry_band_idx]--;
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pMulticastFilterTable->Size--;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size);
			}
		} else
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"the Group doesn't exist.\n");
	} while (FALSE);

	OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	return TRUE;
}

#ifdef IGMP_SNOOPING_DENY_LIST
BOOLEAN MulticastDenyListUpdate(struct _RTMP_ADAPTER *pAd,
								UINT8 BssIdx,
								UINT8 entry_cnt,
								UINT8 add_to_list,
								UINT8 *pAddr,
								UINT8 *Prefix_list)
{
	MULTICAST_DENY_LIST_FILTER_TABLE *deny_list_tbl = pAd->pMcastDLTable;
	UINT8 entry_num = 0, tbl_idx = 0;

	OS_SEM_LOCK(&deny_list_tbl->MulticastDenyTabLock);
	if (entry_cnt == 0) {
		NdisZeroMemory(deny_list_tbl, sizeof(MULTICAST_DENY_LIST_FILTER_TABLE));
	}

	if (add_to_list) {
		if ((deny_list_tbl->valid_entry_count + entry_cnt) > IGMP_DENY_TABLE_SIZE_MAX) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"Deny list table full\n");
			OS_SEM_UNLOCK(&deny_list_tbl->MulticastDenyTabLock);
			return NDIS_STATUS_FAILURE;
		}

		for (tbl_idx = 0; tbl_idx < IGMP_DENY_TABLE_SIZE_MAX; tbl_idx++) {
			if (deny_list_tbl->entry[tbl_idx].valid == DENY_ENTRY_INVALID) {
				NdisCopyMemory(deny_list_tbl->entry[tbl_idx].addr,
								pAddr + entry_num * IPV6_ADDR_LEN * sizeof(UCHAR),
								IPV6_ADDR_LEN);
				deny_list_tbl->entry[tbl_idx].valid = DENY_ENTRY_VALID;
				deny_list_tbl->entry[tbl_idx].prefix = Prefix_list[entry_num];
				deny_list_tbl->valid_entry_count++;

				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"List index: %d, addr: "IPV6STR"\n", tbl_idx,
						PRINT_IPV6(deny_list_tbl->entry[tbl_idx].addr));

				entry_num++;
				if (entry_num >= entry_cnt)
					break;
			}
		}
	} else {
		for (entry_num = 0; entry_num < entry_cnt; entry_num++) {
			for (tbl_idx = 0; tbl_idx < IGMP_DENY_TABLE_SIZE_MAX; tbl_idx++) {
				if (deny_list_tbl->entry[tbl_idx].valid == DENY_ENTRY_VALID
					&& NdisEqualMemory(deny_list_tbl->entry[tbl_idx].addr,
										pAddr + entry_num * IPV6_ADDR_LEN * sizeof(UCHAR),
										IPV6_ADDR_LEN)) {
					NdisZeroMemory(&deny_list_tbl->entry[tbl_idx],
									sizeof(MULTICAST_DENY_LIST_ENTRY));
					deny_list_tbl->valid_entry_count--;
				}
			}
		}
	}
	OS_SEM_UNLOCK(&deny_list_tbl->MulticastDenyTabLock);
	return NDIS_STATUS_SUCCESS;
}

static BOOLEAN isbInclude(UCHAR maskBits, UCHAR *curIp, UCHAR *denyIp, int len)
{
	int i;
	UCHAR mask[IPV6_ADDR_LEN] = {0};

	for (i = 0; i < len; i++) {
		if (maskBits >= 8) {
			mask[i] = 0xFF;
			maskBits -= 8;
			if ((curIp[i] & mask[i]) != (denyIp[i] & mask[i]))
				return FALSE;
		} else if (maskBits > 0) {
			mask[i] = 0xFF << (8 - maskBits);
			maskBits = 0;
			if ((curIp[i] & mask[i]) != (denyIp[i] & mask[i]))
				return FALSE;
		} else {
			mask[i] = 0;
			if ((curIp[i] & mask[i]) != (denyIp[i] & mask[i]))
				return FALSE;
		}
	}
	return TRUE;
}



BOOLEAN isIgmpSnoopDeny(struct _RTMP_ADAPTER *pAd, PUCHAR pDstMacAddr, PUCHAR pIpHeader)
{
	UINT8 tbl_idx = 0;
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));
	MULTICAST_DENY_LIST_FILTER_TABLE *deny_list_table = pAd->pMcastDLTable;
	MULTICAST_DENY_LIST_ENTRY *deny_list_entry = NULL;
	UCHAR GroupIpv6Addr[IPV6_ADDR_LEN] = {0};
	PUCHAR pGroupIpv6Addr = (PUCHAR)GroupIpv6Addr;

	if (deny_list_table == NULL)
		return FALSE;

	OS_SEM_LOCK(&deny_list_table->MulticastDenyTabLock);
	if (deny_list_table->valid_entry_count == 0) {
		OS_SEM_UNLOCK(&deny_list_table->MulticastDenyTabLock);
		return FALSE;
	}

	if (IpProtocol == ETH_P_IP) {
		GroupIpv6Addr[12] = *(pIpHeader + 18);
		GroupIpv6Addr[13] = *(pIpHeader + 19);
		GroupIpv6Addr[14] = *(pIpHeader + 20);
		GroupIpv6Addr[15] = *(pIpHeader + 21);
	} else if (IpProtocol == ETH_P_IPV6)
		pGroupIpv6Addr = pIpHeader + 26;

	for (tbl_idx = 0; tbl_idx < IGMP_DENY_TABLE_SIZE_MAX; tbl_idx++) {
		deny_list_entry = &deny_list_table->entry[tbl_idx];
		if (deny_list_entry->valid) {
			if (isbInclude(deny_list_entry->prefix, pGroupIpv6Addr, deny_list_entry->addr, IPV6_ADDR_LEN)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"Deny list addr match: "IPV6STR"\n",
						PRINT_IPV6(deny_list_entry->addr));
				OS_SEM_UNLOCK(&deny_list_table->MulticastDenyTabLock);
				return TRUE;
			}
		}
	}
	OS_SEM_UNLOCK(&deny_list_table->MulticastDenyTabLock);
	return FALSE;
}
#endif

#ifdef IGMP_SNOOPING_NON_OFFLOAD
BOOLEAN isIgmpMldFloodingPkt(IN PRTMP_ADAPTER pAd, IN PUCHAR pSrcBufVA)
{
	BOOLEAN bInclude = FALSE;
	UCHAR idx = 0, MaskIdx = 0, Mask = 0;
	PUCHAR pDstMacAddr = pSrcBufVA;
	PUCHAR pIpHeader = pSrcBufVA + 12;
	UINT16 protoType = ntohs(*((UINT16 *)(pIpHeader)));
	PMULTICAST_WHITE_LIST_FILTER_TABLE pMcastWLTable = pAd->pMcastWLTable;

	do {
		if (pMcastWLTable->EntryNum == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"Flooding CIDR list is empty\n");
			break;
		}

		if (!isIgmpMacAddr(pDstMacAddr)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"Pkt is not IGMP MAC addr\n");
			break;
		}

		/* IPv4 only */
		if (protoType != ETH_P_IP)
			break;

		for (idx = 0; idx < MULTICAST_WHITE_LIST_SIZE_MAX; idx++) {
			PMULTICAST_WHITE_LIST_ENTRY pEntryTab = &pMcastWLTable->EntryTab[idx];

			if (pEntryTab->bValid) {
				for (MaskIdx = 0; MaskIdx < IPV4_ADDR_LEN; MaskIdx++) {
					Mask = pEntryTab->PrefixMask.Byte[3 - MaskIdx];
					if (!Mask)
						continue;

					bInclude = TRUE;
					if ((pEntryTab->Addr[MaskIdx + 2] & Mask)
						!= (pDstMacAddr[MaskIdx + 2] & Mask)) {
						bInclude = FALSE;
						break;
					}
				}
				if (bInclude == TRUE)
					break;
			}
		}
	} while (FALSE);

	return bInclude;
}
#endif

/*
    ==========================================================================
    Description:
	Look up the MAC address in the IGMP table. Return NULL if not found.
    Return:
	pEntry - pointer to the MAC entry; NULL is not found
    ==========================================================================
*/
PMULTICAST_FILTER_TABLE_ENTRY MulticastFilterTableLookup(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PUCHAR pAddr,
	IN PNET_DEV dev)
{
	ULONG HashIdx, Now;
	PMULTICAST_FILTER_TABLE_ENTRY pEntry = NULL, pPrev = NULL;
	RTMP_ADAPTER *pAd = NULL;
	UINT8 band_idx;

	GET_PAD_FROM_NET_DEV(pAd, dev);
	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "pAd is NULL.\n");
		return NULL;
	}

	if (pMulticastFilterTable == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Multicase filter table is not ready.\n");
		return NULL;
	}

	OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	HashIdx = MULTICAST_IPV6_ADDR_HASH_INDEX(pAddr);
	pEntry = pPrev = pMulticastFilterTable->Hash[HashIdx];

	while (pEntry && pEntry->Valid) {
		if ((pEntry->net_dev ==  dev)
			&& IPV6_ADDR_EQUAL(pEntry->Addr, pAddr)) {
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			break;
		} else {
			NdisGetSystemUpTime(&Now);

			if ((pEntry->Valid == TRUE) && (pEntry->type == MCAT_FILTER_DYNAMIC)
				&& RTMP_TIME_AFTER (Now, pEntry->lastTime +
#ifdef IGMP_TVM_SUPPORT
				pEntry->AgeOutTime
#else
				IGMPMAC_TB_ENTRY_AGEOUT_TIME
#endif /* IGMP_TVM_SUPPORT */
				)) {
				/* Remove the aged entry */
				if (pEntry == pMulticastFilterTable->Hash[HashIdx]) {
					pMulticastFilterTable->Hash[HashIdx] = pEntry->pNext;
					pPrev = pMulticastFilterTable->Hash[HashIdx];
					DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
					GET_PAD_FROM_NET_DEV(pAd, pEntry->net_dev);
					band_idx = hc_get_hw_band_idx(pAd);
					pMulticastFilterTable->size_cfg.used_num[band_idx]--;
					NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
					pMulticastFilterTable->Size--;
					pEntry = pPrev;
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
							"MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size);
				} else {
					pPrev->pNext = pEntry->pNext;
					DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
					GET_PAD_FROM_NET_DEV(pAd, pEntry->net_dev);
					band_idx = hc_get_hw_band_idx(pAd);
					pMulticastFilterTable->size_cfg.used_num[band_idx]--;
					NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
					pMulticastFilterTable->Size--;
					pEntry = pPrev->pNext;
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
							"MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size);
				}
			} else {
				pPrev = pEntry;
				pEntry = pEntry->pNext;
			}
		}
	}

	OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	return pEntry;
}

#ifdef IGMP_TVM_SUPPORT
INT IgmpSnEnableTVMode(IN RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 IsTVModeEnable, UINT8 TVModeType)
{
	INT Result = FALSE;
#ifdef APCLI_SUPPORT
	struct wifi_dev *ApcliWdev = NULL;
#endif /* APCLI_SUPPORT */

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"Before: Enable = %u, IsTVModeEnable = %u/%u, TVModeType = %u/%u\n",
			wdev->IgmpSnoopEnable, wdev->IsTVModeEnable, IsTVModeEnable, wdev->TVModeType, TVModeType);

	do {
		if (wdev->IgmpSnoopEnable == FALSE) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "Igmp Disabled\n");
			IsTVModeEnable = IGMP_TVM_SWITCH_DISABLE;
			TVModeType = IGMP_TVM_MODE_DISABLE;
		}

		if (IsTVModeEnable > IGMP_TVM_SWITCH_ENABLE) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Invalid Value: Please input, %u to DISABLE or %u to ENABLE IGMP Enhanced mode\n",
					IGMP_TVM_SWITCH_DISABLE, IGMP_TVM_SWITCH_ENABLE);
			break;
		}

		if (IS_IGMP_TVM_MODE_EN(IsTVModeEnable)) {

			if (TVModeType > IGMP_TVM_MODE_AUTO) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
						"Invalid Value: Please input, %u to DISABLE or %u to ENABLE or %u to AUTO mode\n",
						IGMP_TVM_MODE_DISABLE, IGMP_TVM_MODE_ENABLE, IGMP_TVM_MODE_AUTO);
				break;
			}

			wdev->IsTVModeEnable = TRUE;

			/* Add few exceptional list which is hardcoded, for the first time */
			if ((wdev->TVModeType == IGMP_TVM_MODE_DISABLE) &&
				(TVModeType != IGMP_TVM_MODE_DISABLE)) {
				Set_IgmpSn_BlackList_Proc(pAd, (RTMP_STRING *)"1-224.0.0.1/24-1");
				Set_IgmpSn_BlackList_Proc(pAd, (RTMP_STRING *)"1-239.255.255.250/0-1");
				Set_IgmpSn_BlackList_Proc(pAd, (RTMP_STRING *)"1-ff02:0:0:0:0:0:0:fb/0-1");
			}

			wdev->TVModeType = TVModeType;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"wdev[%d],OMAC[%d], TVMode : %s\n", wdev->wdev_idx,
					wdev->DevInfo.OwnMacIdx,
					((TVModeType == IGMP_TVM_MODE_AUTO) ? "AUTO" :
					((TVModeType == IGMP_TVM_MODE_ENABLE) ? "ENABLE" :
					((TVModeType == IGMP_TVM_MODE_DISABLE) ? "DISABLE" : "Invalid Value, Not Changed")))
					);

			Result = TRUE;
		} else {
			wdev->IsTVModeEnable = FALSE;
			wdev->TVModeType = IGMP_TVM_MODE_DISABLE;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"wdev[%d],OMAC[%d]-%s\n", wdev->wdev_idx, wdev->DevInfo.OwnMacIdx,
					IsTVModeEnable == TRUE ? "Enable IGMP Snooping":"Disable IGMP Snooping");
		}

		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));

#ifdef APCLI_SUPPORT
		if ((wdev->func_idx < MAX_APCLI_NUM) &&
			pAd->StaCfg[wdev->func_idx].ApcliInfStat.ApCliInit) {
		ApcliWdev = &pAd->StaCfg[wdev->func_idx].wdev;
		if (ApcliWdev)
			ApcliWdev->IsTVModeEnable = wdev->IsTVModeEnable;
		}
#endif /* APCLI_SUPPORT */
	} while (FALSE);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"Enable = %u, IsTVModeEnable = %u, TVModeType = %u\n",
			wdev->IgmpSnoopEnable, wdev->IsTVModeEnable, wdev->TVModeType);

	return Result;
}

VOID ConvertUnicastMacToMulticast(IN RTMP_ADAPTER *pAd, IN struct wifi_dev *wdev, IN struct _RX_BLK * pRxBlk)
{
	UCHAR *Header802_3 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
	UINT16 protoType = OS_NTOHS(*((UINT16 *)(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket) + 12)));
	PUCHAR pGroupIpAddr;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;

	if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "TV Mode not enabled\n");
		return;
	}

	if (protoType == ETH_P_IP) {
		UINT32 GroupIpAddr;

		pGroupIpAddr = Header802_3 + LENGTH_802_3 + 16;
		GroupIpAddr = OS_NTOHL(*((UINT32 *)pGroupIpAddr));

		if (IS_MULTICAST_IP(GroupIpAddr)) {
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			COPY_MAC_ADDR(Header802_3, GroupMacAddr);
		}
	} else if (protoType == ETH_P_IPV6) {
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(Header802_3 + LENGTH_802_3);

		pGroupIpAddr = pIpv6Hdr->dstAddr.ipv6_addr;

		if (IS_MULTICAST_IPV6_ADDR(pIpv6Hdr->dstAddr)) {
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
			COPY_MAC_ADDR(Header802_3, GroupMacAddr);
		}

	}
}

VOID MakeTVMIE(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN OUT PUCHAR pOutBuffer,
	IN OUT PULONG pFrameLen)
{
	if (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
		struct _nec_tvm_ie tvm_ie = {0};
		ULONG TVIeLen = 0;

		tvm_ie.eid = IE_VENDOR_SPECIFIC;
		tvm_ie.len = IGMP_TVM_IE_LENGTH;
		if (ARRAY_SIZE(IGMP_TVM_OUI) >= 4)
			NdisMoveMemory(tvm_ie.oui_oitype, IGMP_TVM_OUI, 4);
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"Length of IGMP_TVM_OUI is invalid\n");
			return;
		}
		tvm_ie.version1 = IGMP_TVM_IE_VERSION_1;
		tvm_ie.version2 = IGMP_TVM_IE_VERSION_2;
		tvm_ie.data.field.rsvd = 0;

		if (wdev->wdev_type == WDEV_TYPE_AP) {
			if ((wdev->TVModeType == IGMP_TVM_MODE_AUTO)
				|| (wdev->TVModeType == IGMP_TVM_MODE_DISABLE))
				tvm_ie.data.field.TVMode = IGMP_TVM_IE_MODE_AUTO;
			else if (wdev->TVModeType == IGMP_TVM_MODE_ENABLE)
				tvm_ie.data.field.TVMode = IGMP_TVM_IE_MODE_ENABLE;
		} else if (wdev->wdev_type == WDEV_TYPE_STA) {
			tvm_ie.data.field.TVMode = IGMP_TVM_IE_MODE_AUTO;
		}

		MakeOutgoingFrame(pOutBuffer + *pFrameLen,	&TVIeLen,
							IGMP_TVM_IE_LENGTH+2,	(PUCHAR)&tvm_ie,
							END_OF_ARGS);

		*pFrameLen += TVIeLen;
	}
}

INT Set_IgmpSn_BlackList_Proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	UCHAR *pOperationType = NULL; /* 1=Add or 0=Del */
	UCHAR *pIP = NULL; /* IPv4 or IPv6 */
	UCHAR *pPrefix = NULL; /* IPv4 or IPv6 */
	UCHAR Prefix = 0;
	UCHAR *pIsStatic = NULL; /* Static or Dynamic */
	UCHAR IsStatic = FALSE;
	UCHAR *value = 0;
	PMULTICAST_BLACK_LIST_FILTER_TABLE pMcastBLTable = NULL;
	MULTICAST_BLACK_LIST_ENTRY ThisMcastEntry = {0};
	BOOLEAN bPrintCmdUsages = FALSE;
	INT Result = FALSE;
	BOOLEAN bAdd = FALSE; /* bAdd == FALSE means Delete operation */
	UCHAR i = 0;
	RTMP_STRING IPString[100] = {'\0'};
	RTMP_STRING *pIPString = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (wdev)
		pMcastBLTable = &wdev->McastBLTable;

	do {
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"wdev not initialized\n");
			break;
		}

		if (wdev->IgmpSnoopEnable == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"IGMP Snooping is disabled\n");
			break;
		}

		if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"TV Mode not enabled\n");
			break;
		}

		if (pMcastBLTable == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Mcast Black List Not init, skip this operation\n");
			break;
		}

		pIPString = IPString;
		if (strlen(arg) < ARRAY_SIZE(pIPString))
			NdisMoveMemory(pIPString, arg, strlen(arg));
		else
			NdisMoveMemory(pIPString, arg, ARRAY_SIZE(pIPString));

		pIPString[strlen(arg)] = '\0';

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				"arg = [%s]\n", pIPString);

		/* Parse, if user supplied, Operation/IP/Prefix-Len */
		pOperationType = strsep((char **)&pIPString, "-");
		if (pIPString && (*pIPString != '\0')) {
			pIP = strsep((char **)&pIPString, "/");
			if (pIPString && (*pIPString != '\0')) {
				pPrefix = strsep((char **)&pIPString, "-");
				if (pPrefix) {
					Prefix = (UCHAR)os_str_tol(pPrefix, NULL, 10);
					if (pIPString && (*pIPString != '\0')) {
						pIsStatic = (PUCHAR)pIPString;
						IsStatic = ((UCHAR)os_str_tol(pIsStatic, NULL, 10) == 0) ? 0 : 1;
					}
				}
			}
		}

		/* Check if incorrect or no operation entered */
		if ((pOperationType == NULL) || ((*pOperationType != '0') && (*pOperationType != '1'))) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Incorrect Operation Type entered, skip this operation\n");
			bPrintCmdUsages = TRUE;
			break;
		}

		bAdd = (BOOLEAN)(((UCHAR)os_str_tol(pOperationType, NULL, 10) == 0) ? FALSE : TRUE);

		/* Check if add operation but IP not entered */
		if ((bAdd == TRUE) && (pIP == NULL)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Mcast IP address not entered, skip this operation\n");
			bPrintCmdUsages = TRUE;
			break;
		}

		if ((bAdd == TRUE) && (pMcastBLTable->EntryNum >= MULTICAST_BLACK_LIST_SIZE_MAX)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Mcast List Already full, skip this operation\n");
			bPrintCmdUsages = TRUE;
			break;
		}

		if ((bAdd == FALSE) && (pIP == NULL)) {
			PMULTICAST_BLACK_LIST_ENTRY pEntryTab = NULL;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Delete full list\n");
			for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
				pEntryTab = &pMcastBLTable->EntryTab[i];
				if ((pEntryTab->bValid == TRUE) && (pEntryTab->bStatic == FALSE)) {
					NdisZeroMemory((PUCHAR)pEntryTab,
							sizeof(MULTICAST_BLACK_LIST_ENTRY));
					pEntryTab->bValid = FALSE;
					pMcastBLTable->EntryNum -= 1;
				}
			}
			Result = TRUE;
			break;
		}


		/* IPv4 address */
		if ((strlen(pIP) >= 9) && (strlen(pIP) <= 15)) {
			for (i = 0, value = rstrtok(pIP, "."); value && i < IPV4_ADDR_LEN;
					value = rstrtok(NULL, "."), i++) {
				UCHAR ii = 0;
				if (strlen(value) > 3) {
					bPrintCmdUsages = TRUE;
					break;
				}
				for (ii = 0; ii < strlen(value); ii++) {
					if (!isdigit(*(value + ii))) {
						bPrintCmdUsages = TRUE;
						break;
					}
				}
				if (bPrintCmdUsages == TRUE)
					break;

				ThisMcastEntry.IPData.IPv4[i] = (UCHAR)os_str_tol(value, NULL, 10);
			}
			/* Check if any invalid multicast address specified */
			if ((bPrintCmdUsages == TRUE) ||
				(ThisMcastEntry.IPData.IPv4[0] < 224) ||
				(ThisMcastEntry.IPData.IPv4[0] > 239)) {
				bPrintCmdUsages = TRUE;
				break;
			}
			ThisMcastEntry.bValid = TRUE;
			ThisMcastEntry.EntryIPType = IP_V4;
			if (Prefix == 0)
				Prefix = 32; /* Use all 32 bits (IPv4 has 32 bits) to mask, in case user spcified 0 Prefix length */

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "Blacklist %sING: IPv4 addr = %d.%d.%d.%d%s%s\n",
					 ((bAdd == TRUE) ? "ADD":"DELETE"),
					 ThisMcastEntry.IPData.IPv4[0], ThisMcastEntry.IPData.IPv4[1], ThisMcastEntry.IPData.IPv4[2],
					 ThisMcastEntry.IPData.IPv4[3],
					 (PUCHAR)((pPrefix) ? (PUCHAR)"/" : (PUCHAR)""), (PUCHAR)((pPrefix) ? pPrefix : (PUCHAR)""));
		} else if ((strlen(pIP) >= 15) && (strlen(pIP) <= 39)) { /* IPv6 address */
			for (i = 0, value = rstrtok(pIP, ":"); value && (i * 2) < IPV6_ADDR_LEN;
					value = rstrtok(NULL, ":"), i++) {
				UCHAR ii = 0;
				UCHAR pIPVal[4] = {'0'};

				NdisFillMemory(pIPVal, 4, '0');

				if (strlen(value) > sizeof(UINT32)) {
					bPrintCmdUsages = TRUE;
					break;
				}
				for (ii = 0; ii < strlen(value); ii++) {
					if (!isxdigit(*(value + ii))) {
						bPrintCmdUsages = TRUE;
						break;
					}
				}
				if (bPrintCmdUsages == TRUE)
					break;
				NdisMoveMemory((PUCHAR)&pIPVal[sizeof(UINT32)-strlen(value)],
								(PUCHAR)value, min((UINT)strlen(value),
								(UINT)sizeof(UINT32)));
				AtoH(pIPVal, &ThisMcastEntry.IPData.IPv6[i*2], 4);
			}
			/* Check if any invalid multicast address specified */
			if ((bPrintCmdUsages == TRUE) ||
				(ThisMcastEntry.IPData.IPv6[0] != 0xFF)) {
				bPrintCmdUsages = TRUE;
				break;
			}
			ThisMcastEntry.bValid = TRUE;
			ThisMcastEntry.EntryIPType = IP_V6;
			if (Prefix == 0)
				Prefix = 128; /* Use all 128 bits (IPv6 has  128 bits) to mask, in case user spcified 0 Prefix length */

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Blacklist %sING: IPv6 addr = "
					"%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X%s%s\n",
					 ((bAdd == TRUE) ? "ADD":"DELETE"),
					 ThisMcastEntry.IPData.IPv6[0], ThisMcastEntry.IPData.IPv6[1], ThisMcastEntry.IPData.IPv6[2],
					 ThisMcastEntry.IPData.IPv6[3], ThisMcastEntry.IPData.IPv6[4], ThisMcastEntry.IPData.IPv6[5],
					 ThisMcastEntry.IPData.IPv6[6], ThisMcastEntry.IPData.IPv6[7], ThisMcastEntry.IPData.IPv6[8],
					 ThisMcastEntry.IPData.IPv6[9], ThisMcastEntry.IPData.IPv6[10], ThisMcastEntry.IPData.IPv6[11],
					 ThisMcastEntry.IPData.IPv6[12], ThisMcastEntry.IPData.IPv6[13], ThisMcastEntry.IPData.IPv6[14],
					 ThisMcastEntry.IPData.IPv6[15],
					 (PUCHAR)((pPrefix) ? (PUCHAR)"/" : (PUCHAR)""), (PUCHAR)((pPrefix) ? pPrefix : (PUCHAR)""));

		}

		if (ThisMcastEntry.bValid == TRUE) {
			/* Store prefix length */
			ThisMcastEntry.PrefixLen = Prefix;
			ThisMcastEntry.bStatic = (BOOLEAN)((IsStatic == 0) ? FALSE : TRUE);
			if (bAdd == TRUE) {
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = NULL;
				/* FIrst check if there is any existing entry */
				for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
					pEntryTab = &pMcastBLTable->EntryTab[i];
					if ((pEntryTab->bValid == TRUE) &&
						(((ThisMcastEntry.EntryIPType == IP_V4) && (pEntryTab->EntryIPType == IP_V4) &&
						NdisEqualMemory(pEntryTab->IPData.IPv4, ThisMcastEntry.IPData.IPv4, IPV4_ADDR_LEN))
						|| ((ThisMcastEntry.EntryIPType == IP_V6) && (pEntryTab->EntryIPType == IP_V6) &&
						NdisEqualMemory(pEntryTab->IPData.IPv6, ThisMcastEntry.IPData.IPv6, IPV6_ADDR_LEN))) &&
						(pEntryTab->PrefixLen == ThisMcastEntry.PrefixLen)) {
						Result = TRUE;
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
								"This IP address already present in Igmp Snooping Exemption table\n\n");
						break;
					}
				}
				/* IP entry already present, no need to add again, so exit */
				if (Result == TRUE)
					break;

				/* Find the empty entry in black list entry to add new entry */
				for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
					UCHAR Index = 0;
					UCHAR Bits = 0;
					UINT32 Mask = 0;
					UCHAR GroupIpv6Addr[IPV6_ADDR_LEN];

					RTMPZeroMemory(GroupIpv6Addr, IPV6_ADDR_LEN);
					if (pMcastBLTable->EntryTab[i].bValid == FALSE) {
						NdisZeroMemory(&pMcastBLTable->EntryTab[i],
										sizeof(MULTICAST_BLACK_LIST_ENTRY));
						NdisMoveMemory(&pMcastBLTable->EntryTab[i],
										&ThisMcastEntry,
										sizeof(MULTICAST_BLACK_LIST_ENTRY));
						pMcastBLTable->EntryNum += 1;

						if (pMcastBLTable->EntryTab[i].EntryIPType == IP_V4) {
							RTMPZeroMemory(GroupIpv6Addr, IPV6_ADDR_LEN);
							CVT_IPV4_IPV6(GroupIpv6Addr, ThisMcastEntry.IPData.IPv4);
						} else {
							COPY_IPV6_ADDR(GroupIpv6Addr, ThisMcastEntry.IPData.IPv6);
						}
						AsicMcastEntryDelete(pAd,
											GroupIpv6Addr,
											wdev->bss_info_argument.ucBssIndex,
											NULL,
											wdev->if_dev,
											0);

						/* Prepare Mask of bytes from Prefix Length to be matched with IP address of entery and packets received */
						Index = 0;
						do {
							/* here 32 = 32 bits in a DWord */
							Bits = ((Prefix%32)?(Prefix%32):32);
							if (Bits == 32)
								Mask = ((UINT32)~0);
							else
								Mask = (UINT32)~(((UINT32)~0)<<(UINT32)Bits);
							pMcastBLTable->EntryTab[i].PrefixMask.DWord[Index] = Mask;
							Prefix = Prefix - ((Prefix%32)?(Prefix%32):32);
							Index += 1;
						} while (Prefix > 0);
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
								"This IP address added in Igmp Snooping Exemption table\n\n");
						break;
					}
				}
			} else {
				/* Find the this entry in black list entry to delete it from list */
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = NULL;
				Result = TRUE;
				for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
					pEntryTab = &pMcastBLTable->EntryTab[i];
					if ((pEntryTab->bValid == TRUE) && (pEntryTab->bStatic == FALSE) &&
						(((ThisMcastEntry.EntryIPType == IP_V4) && (pEntryTab->EntryIPType == IP_V4) &&
						NdisEqualMemory(pEntryTab->IPData.IPv4, ThisMcastEntry.IPData.IPv4, IPV4_ADDR_LEN))
						|| ((ThisMcastEntry.EntryIPType == IP_V6) && (pEntryTab->EntryIPType == IP_V6) &&
						NdisEqualMemory(pEntryTab->IPData.IPv6, ThisMcastEntry.IPData.IPv6, IPV6_ADDR_LEN))) &&
						(pEntryTab->PrefixLen == ThisMcastEntry.PrefixLen)) {
						NdisZeroMemory((PUCHAR)pEntryTab,
								sizeof(MULTICAST_BLACK_LIST_ENTRY));
						pEntryTab->bValid = FALSE;
						pMcastBLTable->EntryNum -= 1;
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
								"This IP address deleted from Igmp Snooping Exemption table\n\n");
						break;
					}
				}
			}
			Result = TRUE;
		}
	} while (FALSE);

	if (bPrintCmdUsages == TRUE) {
		/* CR4 FW already has 224.0.0.x and FF02:0:0:0:0:0:0:FB exempted */
		MTWF_PRINT("\nCommand usages:\n"
				"	iwpriv ra0 set IgmpSnExemptIP=<Operation>-<IP Addr>/<PrefixLength>\n"
				"		<Operation>     :	1 for ADD and 0 for DELETE\n"
				"		<IP Addr>       :	IPv4 or IPv6 address to exempt from being snooped\n"
				"		<PrefixLength>  :	Prefix Length, Value between 1 to 32 or 128 for IPv4 or IPv6\n"
				"\n"
				"		IPv4 address format :\n"
				"			Example = 225.0.0.0\n"
				"		IPv6 address format = No short notation.Only expanded format is accepted, with or without Prefix\n"
				"			Example = FF:0:0:0:0:0:0:0 or FF00:0000:0000:0000:0000:0000:0000:0000\n"
				"\n"
				"	Example : iwpriv ra0 set IgmpSnExemptIP=1-FF01:0:0:0:0:0:0:1\n"
				"		or  : iwpriv ra0 set IgmpSnExemptIP=1-FF01:0:0:0:0:0:0:1/60\n"
				"		or  : iwpriv ra0 set IgmpSnExemptIP=0-225.1.2.3/16\n"
				"		or  : iwpriv ra0 set IgmpSnExemptIP=0-225.1.2.3\n\n");

	}

	return Result;
}

INT Show_IgmpSn_BlackList_Proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	PMULTICAST_BLACK_LIST_FILTER_TABLE pMcastBLTable = NULL;
	UCHAR idx = 0;
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	UCHAR Count = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (wdev)
		pMcastBLTable = &wdev->McastBLTable;

	do {
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"wdev not initialized\n");
			break;
		}

		if (wdev->IgmpSnoopEnable == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"IGMP Snooping is disabled\n");
			break;
		}

		if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"TV Mode not enabled\n");
			break;
		}

		if (pMcastBLTable->EntryNum == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"IGMP/MLD snooping exemption list Empty\n");
			break;
		}

		Count = 0;
		/* First dump IPv4 address */
		for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
			if ((pMcastBLTable->EntryTab[idx].bValid) &&
				(pMcastBLTable->EntryTab[idx].EntryIPType == IP_V4)) {
				Count += 1;
				MTWF_PRINT("IPv4 addr to Exempt snooping:[%u] = %d.%d.%d.%d", Count,
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[0],
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[1],
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[2],
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[3]);
				if (pMcastBLTable->EntryTab[idx].PrefixLen < 32) {
					MTWF_PRINT("/%u : %s\n", pMcastBLTable->EntryTab[idx].PrefixLen,
							((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC"));
				} else {
					MTWF_PRINT(" : %s\n", ((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC"));
				}
			}
		}

		Count = 0;
		/* Second dump IPv6 address */
		for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
			if ((pMcastBLTable->EntryTab[idx].bValid) &&
				(pMcastBLTable->EntryTab[idx].EntryIPType == IP_V6)) {
				Count += 1;
				MTWF_PRINT("IPv6 addr to Exempt snooping:[%u] = "
					"%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X", Count,
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[0],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[1],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[2],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[3],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[4],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[5],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[6],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[7],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[8],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[9],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[10],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[11],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[12],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[13],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[14],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[15]);
				if (pMcastBLTable->EntryTab[idx].PrefixLen < 128) {
					MTWF_PRINT("/%u : %s\n", pMcastBLTable->EntryTab[idx].PrefixLen,
							((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC"));
				} else {
					MTWF_PRINT(" : %s\n", ((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC"));
				}
			}
		}

	} while (FALSE);

	return TRUE;
}

BOOLEAN isIgmpMldExemptPkt(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PUCHAR pGroupIpAddr,
	IN UINT16 ProtoType)
{
	BOOLEAN bExempt = FALSE;
	PMULTICAST_BLACK_LIST_FILTER_TABLE pMcastBLTable = NULL;
	UCHAR idx = 0;
	UCHAR MaskIdx = 0;
	UCHAR Mask = 0;

	do {
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,"Invalid wdev pointer\n");
			break;
		}

		if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,"TV Mode not enabled\n");
			break;
		}

		pMcastBLTable = &wdev->McastBLTable;
		if (pMcastBLTable->EntryNum == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,"Exemption list Empty\n");
			break;
		}

		if (ProtoType == ETH_P_IP) {
			for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = &pMcastBLTable->EntryTab[idx];
				if (pEntryTab->bValid && (pEntryTab->EntryIPType == IP_V4)) {
					for (MaskIdx = 0; MaskIdx < IPV4_ADDR_LEN; MaskIdx++) {
						Mask = pEntryTab->PrefixMask.Byte[MaskIdx];
						if (Mask > 0) {
							bExempt = TRUE;
							if ((pEntryTab->IPData.IPv4[MaskIdx] & Mask) != (pGroupIpAddr[MaskIdx] & Mask)) {
								bExempt = FALSE;
								break;
							}
						}
					}
					if (bExempt == TRUE) {
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
							"Exempt from snooping: IPv4 addr (%d.%d.%d.%d)\n",
								 pEntryTab->IPData.IPv4[0],
								 pEntryTab->IPData.IPv4[1],
								 pEntryTab->IPData.IPv4[2],
								 pEntryTab->IPData.IPv4[3]);
					}

				}
			}
		} else if (ProtoType == ETH_P_IPV6) {
			for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = &pMcastBLTable->EntryTab[idx];
				if (pEntryTab->bValid && (pEntryTab->EntryIPType == IP_V6)) {

					for (MaskIdx = 0; MaskIdx < IPV6_ADDR_LEN; MaskIdx++) {
						Mask = pEntryTab->PrefixMask.Byte[MaskIdx];
						if (Mask > 0) {
							bExempt = TRUE;
							if ((pEntryTab->IPData.IPv4[MaskIdx] & Mask) != (pGroupIpAddr[MaskIdx] & Mask)) {
								bExempt = FALSE;
								break;
							}
						}
					}
					if (bExempt == TRUE) {
						MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
								"Exempt from snooping: IPv6 addr "
								"(%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X)\n",
									 pEntryTab->IPData.IPv6[0],
									 pEntryTab->IPData.IPv6[1],
									 pEntryTab->IPData.IPv6[2],
									 pEntryTab->IPData.IPv6[3],
									 pEntryTab->IPData.IPv6[4],
									 pEntryTab->IPData.IPv6[5],
									 pEntryTab->IPData.IPv6[6],
									 pEntryTab->IPData.IPv6[7],
									 pEntryTab->IPData.IPv6[8],
									 pEntryTab->IPData.IPv6[9],
									 pEntryTab->IPData.IPv6[10],
									 pEntryTab->IPData.IPv6[11],
									 pEntryTab->IPData.IPv6[12],
									 pEntryTab->IPData.IPv6[13],
									 pEntryTab->IPData.IPv6[14],
									 pEntryTab->IPData.IPv6[15]);
					}
				}
			}
		}
	} while (FALSE);

	return bExempt;
}


INT Set_IgmpSn_AgeOut_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;
	UINT32 AgeOut = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	AgeOut = (UINT32) simple_strtol(arg, 0, 10);

	if (wdev) {
		wdev->u4AgeOutTime = AgeOut;
		if ((wdev->IgmpSnoopEnable) && (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable))) {
			AsicMcastConfigAgeOut(pAd, AgeOut, wdev->DevInfo.OwnMacIdx);
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "IgmpSnooping or TV Mode is disabled\n");
		}
	}
	return TRUE;
}

INT Show_IgmpSn_McastTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	if (wdev) {
		if ((wdev->IgmpSnoopEnable) && (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)))
			AsicMcastGetMcastTable(pAd, wdev->DevInfo.OwnMacIdx, wdev);
		else
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "IgmpSnooping or TV Mode is disabled\n");
		}

	return TRUE;
}

BOOLEAN IgmpSnoopingGetMulticastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, P_IGMP_MULTICAST_TABLE pEvtMcastTable)
{
	P_IGMP_MULTICAST_TABLE_MEMBER pEvtMcastMember = NULL, pDrvMcastMember = NULL;
	P_IGMP_MULTICAST_TABLE_ENTRY pEvtMcastTableEntry = NULL, pDrvMcastTableEntry = NULL;
	UINT_32 GroupIdx = 0, MemberIdx = 0;
	struct wifi_dev *wdev = NULL;
	BOOLEAN bNeedToShowTable = FALSE;

	do {

		if (ucOwnMacIdx >= HW_BSSID_MAX) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "Invalid MacIndex = %u\n", ucOwnMacIdx);
			return bNeedToShowTable;
		}

		wdev = wdev_search_by_omac_idx(pAd, ucOwnMacIdx);
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "Invalid wdev\n");
			return bNeedToShowTable;
		}

		if ((wdev->IgmpTableSize == 0) || (wdev->pIgmpMcastTable == NULL)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "IgmpMcastTable was not allocated\n");
			return bNeedToShowTable;
		}

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "Event sequence = %u\n", pEvtMcastTable->EvtSeqNum);

		/* This allocation will happen for the first time, and for rest event attempt, only entries will be added */
		if (pEvtMcastTable->EvtSeqNum == 1) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "First Event\n");
			NdisZeroMemory((UCHAR *)wdev->pIgmpMcastTable, wdev->IgmpTableSize);
			wdev->pIgmpMcastTable->pNxtFreeGroupLocation = &wdev->pIgmpMcastTable->IgmpMcastTableEntry[0];
			wdev->pIgmpMcastTable->TotalGroup = pEvtMcastTable->TotalGroup;
			wdev->pIgmpMcastTable->TotalSize = pEvtMcastTable->TotalSize;
		}

		pDrvMcastTableEntry = wdev->pIgmpMcastTable->pNxtFreeGroupLocation;

		GroupIdx = 0;
		pEvtMcastTableEntry = &pEvtMcastTable->IgmpMcastTableEntry[GroupIdx];

		for (;
			GroupIdx < min((UINT_32)pEvtMcastTable->NumOfGroup, (UINT_32)MAX_LEN_OF_MULTICAST_FILTER_TABLE);
			GroupIdx++) {


			/* Check if the size do not exceed and overwrite the next buffer which is outside the size of this table */
			if ((((ULONG)pDrvMcastTableEntry - (ULONG)wdev->pIgmpMcastTable)
				+ pDrvMcastTableEntry->ThisGroupSize) > (ULONG)wdev->IgmpTableSize) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
						"Potential buffer overflow, skip write operation to prevent buffer overwrite\n");
				break;
			}

			/* hex_dump_with_lvl("Group", (UCHAR*)pEvtMcastTableEntry, 200, DBG_LVL_OFF); */

			NdisMoveMemory(pDrvMcastTableEntry, pEvtMcastTableEntry, sizeof(IGMP_MULTICAST_TABLE_ENTRY));

			for (MemberIdx = 0;
				MemberIdx < min((UINT_32)pEvtMcastTableEntry->NumOfMember, (UINT_32)FREE_MEMBER_POOL_SIZE);
				MemberIdx++) {
				pDrvMcastMember = &pDrvMcastTableEntry->IgmpMcastMember[MemberIdx];
				pEvtMcastMember = &pEvtMcastTableEntry->IgmpMcastMember[MemberIdx];

				COPY_MAC_ADDR(pDrvMcastMember->Addr, pEvtMcastMember->Addr);
				pDrvMcastMember->TVMode = pEvtMcastMember->TVMode;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"Member[%u] = "MACSTR"\n", MemberIdx, MAC2STR(pDrvMcastMember->Addr));
			}

			wdev->pIgmpMcastTable->NumOfGroup += 1;

			pDrvMcastTableEntry =
				(P_IGMP_MULTICAST_TABLE_ENTRY)
				((PUCHAR)(pDrvMcastTableEntry) + pDrvMcastTableEntry->ThisGroupSize);
			pEvtMcastTableEntry =
				(P_IGMP_MULTICAST_TABLE_ENTRY)
					((PUCHAR)(pEvtMcastTableEntry) + pEvtMcastTableEntry->ThisGroupSize);

		}

		wdev->pIgmpMcastTable->ThisTableSize += pEvtMcastTable->ThisTableSize;
		/* Reinitialize the next free location, which points to the end of last member in last group */
		wdev->pIgmpMcastTable->pNxtFreeGroupLocation = pDrvMcastTableEntry;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
				"Local No of Group = %u, No Of Group = %u, Total Group = %u, ThisTableSize = %u\n",
				wdev->pIgmpMcastTable->NumOfGroup, pEvtMcastTable->NumOfGroup, pEvtMcastTable->TotalGroup,
				pEvtMcastTable->ThisTableSize);
	} while (FALSE);

	return bNeedToShowTable;
}


VOID IgmpSnoopingShowMulticastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	P_IGMP_MULTICAST_TABLE_MEMBER pDrvMcastMember = NULL;
	P_IGMP_MULTICAST_TABLE_ENTRY pDrvMcastTableEntry = NULL;
	UINT_32 GroupIdx = 0, MemberIdx = 0;

	do {
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "Invalid wdev\n");
			return;
		}

		if ((wdev->IgmpTableSize == 0) || (wdev->pIgmpMcastTable == NULL)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "IgmpMcastTable was not allocated\n");
			return;
		}

		if (wdev->pIgmpMcastTable->NumOfGroup == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "IgmpMcastTable empty\n");
			return;
		}

		/* hex_dump_with_lvl("Local Group", (UCHAR*)&wdev->pIgmpMcastTable->IgmpMcastTableEntry[0], 200, DBG_LVL_OFF); */

		MTWF_PRINT("*************************************************"
								"*************************************************\n");
		MTWF_PRINT("	S.No.		   GROUP ID                "
								"MEMBER 			   "
								"TVM				"
								"AGEOUT(Sec)\n");
		GroupIdx = 0;
		pDrvMcastTableEntry = &wdev->pIgmpMcastTable->IgmpMcastTableEntry[GroupIdx];
		for (;
			GroupIdx < min((UINT_32)wdev->pIgmpMcastTable->NumOfGroup, (UINT_32)MAX_LEN_OF_MULTICAST_FILTER_TABLE);
			GroupIdx++) {
			MTWF_PRINT("	%-2u	      "MACSTR"				  "
								"				 "
								"				 "
								"    %5u\n",
								(GroupIdx+1), MAC2STR(pDrvMcastTableEntry->GroupAddr),
													pDrvMcastTableEntry->AgeOut);
			for (MemberIdx = 0;
				MemberIdx < min((UINT_32)pDrvMcastTableEntry->NumOfMember, (UINT_32)FREE_MEMBER_POOL_SIZE);
				MemberIdx++) {
				pDrvMcastMember = &pDrvMcastTableEntry->IgmpMcastMember[MemberIdx];
				MTWF_PRINT("	%3u.%-2u                            "
					MACSTR"		 "
					"%s\n",
					(GroupIdx+1), (MemberIdx+1),
					MAC2STR(pDrvMcastMember->Addr),
					((pDrvMcastMember->TVMode == 0) ? "AUTO":((pDrvMcastMember->TVMode == 1) ? "ENABLE":"NO TVM IE")));
			}
			/* The next group table will start from the end of last member of last group */
			pDrvMcastTableEntry =
				(P_IGMP_MULTICAST_TABLE_ENTRY)
				((PUCHAR)(pDrvMcastTableEntry) + pDrvMcastTableEntry->ThisGroupSize);
		}
		MTWF_PRINT("-------------------------------------------------"
								"-------------------------------------------------\n");
		NdisZeroMemory((UCHAR *)wdev->pIgmpMcastTable, wdev->IgmpTableSize);
	} while (FALSE);
}

BOOLEAN MulticastFilterConfigAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 ucOwnMacIdx)
{
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;
	UINT i = 0;

	if (pMulticastFilterTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Multicase filter table is not ready.\n");
		return FALSE;
	}

	OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
		pMulticastFilterTable->Content[i].AgeOutTime = (AgeOutTime * OS_HZ);
	}

	OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	return TRUE;
}

BOOLEAN MulticastFilterInitMcastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN bActive)
{
	if (bActive == TRUE) {
		wdev->IgmpTableSize = sizeof(IGMP_MULTICAST_TABLE)
			+ ((sizeof(IGMP_MULTICAST_TABLE_ENTRY) * MAX_LEN_OF_MULTICAST_FILTER_TABLE)
			- sizeof(IGMP_MULTICAST_TABLE_ENTRY))
			+ ((sizeof(IGMP_MULTICAST_TABLE_MEMBER) * FREE_MEMBER_POOL_SIZE)
			- sizeof(IGMP_MULTICAST_TABLE_MEMBER));
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				"Allocate IGMP Multicast Memory size = %u\n", wdev->IgmpTableSize));

		wdev->pIgmpMcastTable =
			(P_IGMP_MULTICAST_TABLE)kmalloc(wdev->IgmpTableSize, 0);
		if (wdev->pIgmpMcastTable == NULL) {
			wdev->IgmpTableSize = 0;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Failed to allocate IGMP Multicast Memory\n");
		} else {
			NdisZeroMemory((UCHAR *)wdev->pIgmpMcastTable, wdev->IgmpTableSize);
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				"Deallocate IGMP Multicast Memory\n");
		kfree(wdev->pIgmpMcastTable);
		wdev->pIgmpMcastTable = NULL;
		wdev->IgmpTableSize = 0;
	}

	return TRUE;
}

BOOLEAN MulticastFilterGetMcastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev)
{
	UINT_32 GroupIdx = 0, MemberIdx = 0;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	do {
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Invalid wdev\n");
			return FALSE;
		}

		/* if empty, return */
		if (pMulticastFilterTable->Size == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, "Table empty.\n");
			return FALSE;
		}

		MTWF_PRINT("*************************************************"
								"*************************************************\n");
		MTWF_PRINT("	S.No.		   GROUP ID                "
								"MEMBER 			   "
								"TVM				"
								"AGEOUT(Sec)\n");
		/* allocate one MAC entry */
		OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

		for (GroupIdx = 0; GroupIdx < MAX_LEN_OF_MULTICAST_FILTER_TABLE; GroupIdx++) {
			/* pick up the valid entry */
			if (pMulticastFilterTable->Content[GroupIdx].Valid == TRUE) {
				PMEMBER_ENTRY pMemberEntry = NULL;
				pEntry = &pMulticastFilterTable->Content[GroupIdx];

				MTWF_PRINT("	%-2u		  "MACSTR" 			  "
							"				 "
							"				 "
							"	 %5u\n",
							(GroupIdx+1), MAC2STR(pEntry->Addr),
							(pEntry->AgeOutTime / OS_HZ));

				pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

				MemberIdx = 0;

				while (pMemberEntry) {
					MTWF_PRINT("	%3u.%-2u							"
						MACSTR"		 "
						"%s\n",
						(GroupIdx+1), (MemberIdx+1),
						MAC2STR(pMemberEntry->Addr),
						((pMemberEntry->TVMode == 0) ? "AUTO":((pMemberEntry->TVMode == 1) ? "ENABLE":"NO TVM IE")));

					pMemberEntry = pMemberEntry->pNext;
					MemberIdx += 1;
				}
			}
		}
		OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

		MTWF_PRINT("-------------------------------------------------"
								"-------------------------------------------------\n");

	} while (FALSE);

	return TRUE;
}

#endif /* IGMP_TVM_SUPPORT */

VOID IGMPSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN PUCHAR pDataEnd,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT16 Wcid
)
{
	INT i;
	INT IpHeaderLen;
	UCHAR GroupType;
	UINT16 numOfGroup;
	UCHAR IgmpVerType;
	PUCHAR pIgmpHeader;
	PUCHAR pGroup;
	UCHAR AuxDataLen;
	UINT16 numOfSources;
	PUCHAR pGroupIpAddr;
	UCHAR GroupIpv6Addr[IPV6_ADDR_LEN];
	UINT8 Type = MCAT_FILTER_DYNAMIC;
	struct wifi_dev *wdev = pEntry->wdev;
#ifdef IGMP_TVM_SUPPORT
	UCHAR TVModeType = 0;
#endif /* IGMP_TVM_SUPPORT */

	if (isIgmpPkt(pDstMacAddr, pIpHeader)) {
		IpHeaderLen = (*(pIpHeader + 2) & 0x0f) * 4;
		pIgmpHeader = pIpHeader + 2 + IpHeaderLen;
		if (pIgmpHeader >= pDataEnd)
			return;

		IgmpVerType = (UCHAR)(*(pIgmpHeader));
		RTMPZeroMemory(GroupIpv6Addr, IPV6_ADDR_LEN);
#ifdef A4_CONN
						if (pEntry && (wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))
							Type |= MCAT_FILTER_MWDS_CLI;	/* set info about message on MWDS link*/
#endif
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "IGMP type=%0x\n", IgmpVerType);

#ifdef IGMP_TVM_SUPPORT
		pEntry = entry_get(pAd, Wcid);

		if (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			if (pEntry->TVMode == IGMP_TVM_IE_MODE_AUTO)
				TVModeType = MCAT_FILTER_TVM_AUTO;
			else if (pEntry->TVMode == IGMP_TVM_IE_MODE_ENABLE)
				TVModeType = MCAT_FILTER_TVM_ENABLE;
		}
#endif /* IGMP_TVM_SUPPORT */

		switch (IgmpVerType) {
		case IGMP_V1_MEMBERSHIP_REPORT: /* IGMP version 1 membership report. */
		case IGMP_V2_MEMBERSHIP_REPORT: /* IGMP version 2 membership report. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
			if ((pGroupIpAddr + 3) >= pDataEnd)
				return;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"EntryInsert IGMP Group=%pI4\n", pGroupIpAddr);
#ifdef IGMP_TVM_SUPPORT
			if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IP) == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"Exempt Snooping IGMP Group\n");
				break;
			}
#endif /* IGMP_TVM_SUPPORT */
			CVT_IPV4_IPV6(GroupIpv6Addr, pGroupIpAddr);
#if defined(A4_CONN) && defined(IGMP_TX_QUERY_HOLD)
			if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry)))
#endif
				AsicMcastEntryInsert(pAd, GroupIpv6Addr, wdev->bss_info_argument.ucBssIndex,
				(Type
#ifdef IGMP_TVM_SUPPORT
				|TVModeType
#endif /* IGMP_TVM_SUPPORT */
				), pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case IGMP_LEAVE_GROUP: /* IGMP version 1 and version 2 leave group. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
			if ((pGroupIpAddr + 3) >= pDataEnd)
				return;

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"EntryDelete IGMP Group=%pI4\n", pGroupIpAddr);
			CVT_IPV4_IPV6(GroupIpv6Addr, pGroupIpAddr);
#ifdef A4_CONN
			if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))) /* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#endif
				AsicMcastEntryDelete(pAd, GroupIpv6Addr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case IGMP_V3_MEMBERSHIP_REPORT: /* IGMP version 3 membership report. */
			if ((pIgmpHeader + 7) >= pDataEnd)
				return;

			numOfGroup = ntohs(*((UINT16 *)(pIgmpHeader + 6)));
			pGroup = (PUCHAR)(pIgmpHeader + 8);

			if (numOfGroup > MAX_NUM_OF_GRP) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"numOfGroup %d is wrong according to IGMPv3 membership report message\n",
						numOfGroup);
				break;
			}

			for (i = 0; i < numOfGroup; i++) {
				if ((pGroup + 7) >= pDataEnd)
					return;
				GroupType = (UCHAR)(*pGroup);
				AuxDataLen = (UCHAR)(*(pGroup + 1));
				numOfSources = ntohs(*((UINT16 *)(pGroup + 2)));
				pGroupIpAddr = (PUCHAR)(pGroup + 4);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"IGMPv3 Type=%d, ADL=%d, numOfSource=%d\n", GroupType, AuxDataLen, numOfSources);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"IGMP Group=%pI4\n", pGroupIpAddr);
				CVT_IPV4_IPV6(GroupIpv6Addr, pGroupIpAddr);

				do {
					if ((GroupType == MODE_IS_EXCLUDE)
						|| (GroupType == CHANGE_TO_EXCLUDE_MODE)
						|| (GroupType == ALLOW_NEW_SOURCES)) {
#ifdef IGMP_TVM_SUPPORT
						if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IP) == TRUE) {
							MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
									"Exempt Snooping IGMP Group");
							break;
						}
#endif /* IGMP_TVM_SUPPORT */
						AsicMcastEntryInsert(pAd, GroupIpv6Addr, wdev->bss_info_argument.ucBssIndex,
							(Type
#ifdef IGMP_TVM_SUPPORT
							|TVModeType
#endif /* IGMP_TVM_SUPPORT */
							), pSrcMacAddr, wdev->if_dev, Wcid);
						break;
					}

					if ((GroupType == CHANGE_TO_INCLUDE_MODE)
						|| (GroupType == MODE_IS_INCLUDE)
						|| (GroupType == BLOCK_OLD_SOURCES)) {
						if (numOfSources == 0) {

/* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#ifdef A4_CONN

						if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry)))
#endif
							AsicMcastEntryDelete(pAd, GroupIpv6Addr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
						} else {
#ifdef IGMP_TVM_SUPPORT
							if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IP) == TRUE) {
								MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
										"Exempt Snooping IGMP Group\n");
								break;
							}
#endif /* IGMP_TVM_SUPPORT */
							AsicMcastEntryInsert(pAd, GroupIpv6Addr, wdev->bss_info_argument.ucBssIndex,
								(Type
#ifdef IGMP_TVM_SUPPORT
								|TVModeType
#endif /* IGMP_TVM_SUPPORT */
								), pSrcMacAddr, wdev->if_dev, Wcid);
						}
						break;
					}
				} while (FALSE);

				pGroup += (8 + (numOfSources * 4) + AuxDataLen);
			}

			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"unknow IGMP Type=%d\n", IgmpVerType);
			break;
		}
	}

	return;
}

#ifdef A4_CONN
/* Indicate if Specific Pkt is an IGMP query message*/
BOOLEAN isIGMPquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	INT IpHeaderLen;
	UCHAR IgmpVerType;
	PUCHAR pIgmpHeader;
	BOOLEAN isIGMPquery = FALSE;

	if (isIgmpPkt(pDstMacAddr, pIpHeader)) {
		IpHeaderLen = (*(pIpHeader + 2) & 0x0f) * 4;
		pIgmpHeader = pIpHeader + 2 + IpHeaderLen;
		IgmpVerType = (UCHAR)(*(pIgmpHeader));

		switch (IgmpVerType) {
		case IGMP_MEMBERSHIP_QUERY:
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"isIGMPquery-> IGMP Type=0x%x IGMP_MEMBERSHIP_QUERY\n", IgmpVerType);
				isIGMPquery = TRUE;
				break;
		}
	}

	return isIGMPquery;
}
#endif

static BOOLEAN isIgmpMacAddr(
	IN PUCHAR pMacAddr)
{
	if ((pMacAddr[0] == 0x01)
		&& (pMacAddr[1] == 0x00)
		&& (pMacAddr[2] == 0x5e))
		return TRUE;

	return FALSE;
}

BOOLEAN isIgmpPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));
	UCHAR IgmpProtocol;

	if (!isIgmpMacAddr(pDstMacAddr))
		return FALSE;

	if (IpProtocol == ETH_P_IP) {
		IgmpProtocol  = (UCHAR)*(pIpHeader + 11);

		if (IgmpProtocol == IGMP_PROTOCOL_DESCRIPTOR)
			return TRUE;
	}

	return FALSE;
}

static VOID InsertIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr,
	IN MulticastFilterEntryType type)
{
	PMEMBER_ENTRY pMemberEntry;

	if (pList == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "membert list doesn't exist.\n");
		return;
	}

	if (pMemberAddr == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "invalid member.\n");
		return;
	}

	pMemberEntry = (PMEMBER_ENTRY)AllocaGrpMemberEntry(pMulticastFilterTable);
	if (pMemberEntry != NULL) {
		NdisZeroMemory(pMemberEntry, sizeof(MEMBER_ENTRY));
		COPY_MAC_ADDR(pMemberEntry->Addr, pMemberAddr);
#ifdef IGMP_TVM_SUPPORT
		if (type & MCAT_FILTER_TVM_ENABLE)
			pMemberEntry->TVMode = IGMP_TVM_IE_MODE_ENABLE;
		else if (type & MCAT_FILTER_TVM_AUTO)
			pMemberEntry->TVMode = IGMP_TVM_IE_MODE_AUTO;
		else
			pMemberEntry->TVMode = IGMP_TVM_IE_MODE_DISABLE;
#endif /* IGMP_TVM_SUPPORT */
#ifdef A4_CONN
		/* Extract detail regarding presence on MWDS link*/
		if (type & MCAT_FILTER_MWDS_CLI) {
			pMemberEntry->onMWDSLink = TRUE;
		} else {
			pMemberEntry->onMWDSLink = FALSE;
		}
#endif
		insertTailList(pList, (RT_LIST_ENTRY *)pMemberEntry);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
				"Member Mac="MACSTR"\n", MAC2STR(pMemberEntry->Addr));
	}

	return;
}

static VOID DeleteIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr)
{
	PMEMBER_ENTRY pCurEntry;

	if (pList == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "membert list doesn't exist.\n");
		return;
	}

	if (pList->pHead == NULL)
		return;

	if (pMemberAddr == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "invalid member.\n");
		return;
	}

	pCurEntry = (PMEMBER_ENTRY)pList->pHead;

	while (pCurEntry) {
		PMEMBER_ENTRY pCurEntryNext = pCurEntry->pNext;

		if (MAC_ADDR_EQUAL(pMemberAddr, pCurEntry->Addr)) {
			delEntryList(pList, (RT_LIST_ENTRY *)pCurEntry);
			FreeGrpMemberEntry(pMulticastFilterTable, pCurEntry);
			break;
		}

		pCurEntry = pCurEntryNext;
	}

	return;
}

static VOID DeleteIgmpMemberList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList)
{
	PMEMBER_ENTRY pCurEntry, pPrvEntry;

	if (pList == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "membert list doesn't exist.\n");
		return;
	}

	if (pList->pHead == NULL)
		return;

	pPrvEntry = pCurEntry = (PMEMBER_ENTRY)pList->pHead;

	while (pCurEntry) {
		delEntryList(pList, (RT_LIST_ENTRY *)pCurEntry);
		pPrvEntry = pCurEntry;
		pCurEntry = pCurEntry->pNext;
		FreeGrpMemberEntry(pMulticastFilterTable, pPrvEntry);
	}

	initList(pList);
}


UCHAR IgmpMemberCnt(
	IN PLIST_HEADER pList)
{
	if (pList == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "membert list doesn't exist.\n");
		return 0;
	}

	return getListSize(pList);
}

VOID IgmpGroupDelMembers(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMemberAddr,
	IN struct wifi_dev *wdev,
	UINT16 Wcid)
{
	INT i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;
	BOOLEAN valid;
	MulticastFilterEntryType type;

#ifndef IGMP_SNOOPING_NON_OFFLOAD
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		return;
#endif

	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
		/* pick up the first available vacancy */
		OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
		pEntry = &pMulticastFilterTable->Content[i];
		valid = pEntry->Valid;
		type = pEntry->type;
		OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

		if (valid == TRUE) {
			if (pMemberAddr != NULL) {
				OS_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
				DeleteIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);
				OS_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
			}

			if ((type == MCAT_FILTER_DYNAMIC)
				&& (IgmpMemberCnt(&pEntry->MemberList) == 0))
				AsicMcastEntryDelete(pAd, pEntry->Addr, wdev->bss_info_argument.ucBssIndex, pMemberAddr, wdev->if_dev, Wcid);
		}
	}
}



#if defined(RT_CFG80211_SUPPORT)
INT Set_IgmpSn_Enable(RTMP_ADAPTER *pAd, UCHAR enable)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;
	INT ifIndex = 0;

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid mbss id(%d)\n", ifIndex);
			return -EINVAL;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		if ((ifIndex < 0) || (ifIndex >= MAX_MULTI_STA)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid apcli id(%d)\n", ifIndex);
			return -EINVAL;
		}
		wdev = &pAd->StaCfg[ifIndex].wdev;
	}
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"ioctl_if_type:%d error.\n", pObj->ioctl_if_type);
		return -EINVAL;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "wdev is NULL\n");
		return -EINVAL;
	}

	wdev->IgmpSnoopEnable = (BOOLEAN)(enable == 0 ? FALSE : TRUE);

#ifndef IGMP_SNOOPING_NON_OFFLOAD
	/* For kite, it will send cmd to fw */
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		CmdMcastCloneEnable(pAd, enable, wdev->DevInfo.BandIdx, wdev->DevInfo.OwnMacIdx);
#endif

	MTWF_PRINT("%s\n", wdev->IgmpSnoopEnable ?
			"multicast snooping has enabled" : "multicast snooping has disabled");

	return 0;
}

int cfg80211_get_igmp_snoop_status(RTMP_ADAPTER *pAd, struct wiphy *wiphy)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;
	INT ifIndex = 0;
	BOOLEAN igmp_snoop_enable;
	struct sk_buff *skb;

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid mbss id(%d)\n", ifIndex);
			return -EINVAL;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		if ((ifIndex < 0) || (ifIndex >= MAX_MULTI_STA)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid apcli id(%d)\n", ifIndex);
			return -EINVAL;
		}
		wdev = &pAd->StaCfg[ifIndex].wdev;
	}
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"ioctl_if_type:%d error.\n", pObj->ioctl_if_type);
		return -EINVAL;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "wdev is NULL\n");
		return -EINVAL;
	}

	igmp_snoop_enable = wdev->IgmpSnoopEnable;

	MTWF_PRINT("%s\n", wdev->IgmpSnoopEnable ?
			"multicast snooping is enabled" : "multicast snooping is disabled");

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 2);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_IGMP, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		return -EINVAL;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE, igmp_snoop_enable)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_IGMP, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	if (mt_cfg80211_vendor_cmd_reply(skb)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATPROTO_IGMP, DBG_LVL_ERROR,
			"reply msg failed\n");
		return -EINVAL;
	}

	return 0;
}


INT Set_IgmpSn_Allow_Non_Memb_Enable(RTMP_ADAPTER *pAd, UCHAR enable)
{

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"%s\n", enable ? "enable unknown flooding":"disable unknown flooding");

#ifdef IGMP_SNOOPING_NON_OFFLOAD
	pAd->mcsnoop_policy = enable ? UNKNOWN_FLOODING : UNKNOWN_DROP;
	MTWF_PRINT("%s\n", pAd->mcsnoop_policy ?
				"unknown flooding has enabled" : "unknown drop has disabled");
#else
	/* For kite */
#endif

	return 0;
}

static BOOLEAN ipv6_pton(const char *thisChar, UCHAR *dst)
{
	CHAR val = 0;
	INT32 cnt = 0, colon_pos = -1;
	UINT32 digit = 0;
	UCHAR tmp_ipv6_addr[IPV6_ADDR_LEN] = {0};

	if (!thisChar || *thisChar == '\0') {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"Formal parameter is NULL\n");
		return FALSE;
	}

	/* Leading :: requires special handlings */
	if (*thisChar == ':' && *(thisChar + 1) == ':') {
		if (*(thisChar + 2) == '\0') {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"Input IPv6 address only have two colons\n");
			return FALSE;
		}
		thisChar += 2;
		colon_pos = cnt;
	}

	while (*thisChar != '\0') {
		val = (CHAR)BtoH(*thisChar);

		if (val >= 0) {
			digit <<= 4;
			digit |= (UCHAR)val;

			if (digit > 0xffff) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"Input IPv6 address has more than two bytes between colons\n");
				return FALSE;
			}
		} else if (val == -1) {
			if (*thisChar == ':') {
				if (cnt >= IPV6_ADDR_LEN) {
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
							"The length of input IPv6 address is greater than 16 bytes\n");
					return FALSE;
				}

				tmp_ipv6_addr[cnt++] = (UCHAR)(digit >> 8) & 0xff;
				tmp_ipv6_addr[cnt++] = (UCHAR)digit & 0xff;
				digit = 0;

				if (*(thisChar + 1) == ':') {
					if (colon_pos > 0) {
						MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
								"Input IPv6 address has two ::\n");
						return FALSE;
					}

					colon_pos = cnt;
					thisChar++;
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"Input IPv6 address was wrong\n");
				return FALSE;
			}
		}
		thisChar++;
	}

	if (digit != 0) {
		if (cnt >= IPV6_ADDR_LEN) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"Input IPv6 has more than 16 bytes\n");
			return FALSE;
		}

		tmp_ipv6_addr[cnt++] = (UCHAR)(digit >> 8) & 0xff;
		tmp_ipv6_addr[cnt++] = (UCHAR)digit & 0xff;
		digit = 0;
	}

	if (colon_pos >= 0) {
		memmove(tmp_ipv6_addr + IPV6_ADDR_LEN - (cnt - colon_pos),
				tmp_ipv6_addr + colon_pos,
				cnt - colon_pos);
		memset(tmp_ipv6_addr + colon_pos, 0, IPV6_ADDR_LEN - cnt);
		cnt = IPV6_ADDR_LEN;
	}

	if (cnt != IPV6_ADDR_LEN)
		return FALSE;
	memcpy(dst, tmp_ipv6_addr, IPV6_ADDR_LEN);

	return TRUE;
}

static BOOLEAN chk_ipv4_addr_vld(const RTMP_STRING *dotted_val)
{
	if (strlen(dotted_val) > 0 && strlen(dotted_val) <= 3) {
		int ii;

		for (ii = 0; ii < strlen(dotted_val); ii++) {
			if (!isxdigit(*(dotted_val + ii)))
				return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}

INT Set_IgmpSn_AddEntry(RTMP_ADAPTER *pAd, UCHAR *group_entry)
{
	INT i = 0, memberCnt = 0;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[IPV4_ADDR_LEN] = {0};
	UCHAR Addr[MAC_ADDR_LEN];
	BOOLEAN grpid_in_mac = FALSE;
	UCHAR grpid_mac_addr[MAC_ADDR_LEN] = {0};
	UCHAR grpid_ip_addr[32][IPV6_ADDR_LEN] = {0};
	UCHAR GroupId[IPV6_ADDR_LEN];
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	UCHAR mwds_type = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&group_entry, "-")) != NULL) {
		if (bGroupId) {
			if (strstr(thisChar, ".") && BtoH(*((UCHAR *)thisChar)) != 0xf) {
				for (i = 0, value = rstrtok(thisChar, "."); value; value = rstrtok(NULL, ".")) {
					if (chk_ipv4_addr_vld(value) == FALSE)
						return -EINVAL;

					if (i >= IPV4_ADDR_LEN)
						return -EINVAL;

					IpAddr[i] = (UCHAR)os_str_tol(value, NULL, 10);

					i++;
				}

				if (i != 4)
					return -EINVAL;  /*Invalid */

				RTMPZeroMemory(GroupId, IPV6_ADDR_LEN);
				CVT_IPV4_IPV6(GroupId, IpAddr);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "group ip (%pI4)\n", IpAddr);

				bGroupId = 0;
			} else if (strstr(thisChar, ":") && BtoH(*((UCHAR *)thisChar)) == 0xf) {
				RTMPZeroMemory(GroupId, IPV6_ADDR_LEN);
				if (ipv6_pton((CHAR *)thisChar, GroupId) == FALSE) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
							"IPv6 parse fail\n");
				}
				bGroupId = 0;
			} else if (strstr(thisChar, ":") && strlen(thisChar) == 17) {
				/* support add multicast group through MAC address */
				grpid_in_mac = TRUE;

				for (i = 0, value = rstrtok(thisChar, ":"); value; value = rstrtok(NULL, ":")) {
					if ((strlen(value) != 2) || (!isxdigit((unsigned char)value[0])) || (!isxdigit((unsigned char)value[1])))
						return -EINVAL;

					if (i >= MAC_ADDR_LEN)
						return -EINVAL;

					AtoH(value, &grpid_mac_addr[i++], 1);
				}

				if (i != 6)
					return -EINVAL;

				ConvertMulticastMAC2IP(grpid_mac_addr, ETH_P_IP, grpid_ip_addr);

				bGroupId = 0;
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"Input IP address invalid\n");
				return -EINVAL;
			}

		} else {
			/* refuse the Member if it's not a MAC address. */
			if (strlen(thisChar) != 17)
				continue;

			for (i = 0, value = rstrtok(thisChar, ":"); value; value = rstrtok(NULL, ":")) {
				if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
					return -EINVAL;  /*Invalid */

				if (i >= MAC_ADDR_LEN)
					return -EINVAL;

				AtoH(value, &Addr[i++], 1);
			}

			if (i != 6)
				return -EINVAL;  /*Invalid */

			pEntry = MacTableLookup(pAd, Addr);
			memberCnt++;

#ifdef A4_CONN
			/* set info about message on MWDS link*/
			if ((memberCnt > 0) && pEntry && (pAd->ApCfg.MBSSID[ifIndex].wdev.wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))
				mwds_type |= MCAT_FILTER_MWDS_CLI;
#endif

			if ((memberCnt > 0) && !IS_MULTICAST_MAC_ADDR(Addr)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "("MACSTR")\n", MAC2STR(Addr));
				if (pEntry && grpid_in_mac == FALSE)
					AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC | mwds_type, Addr, pDev, pEntry->wcid);
				else if (!pEntry && grpid_in_mac == FALSE)
					AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, Addr, pDev, 0);
				else if (pEntry && grpid_in_mac == TRUE) {
					for (i = 0; i < 32; i++)
						AsicMcastEntryInsert(pAd, &grpid_ip_addr[i][0], pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
											MCAT_FILTER_STATIC | mwds_type, Addr, pDev, pEntry->wcid);
				}
			}
		}
	}

	if (memberCnt == 0 && grpid_in_mac == FALSE) {
		AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC | mwds_type, NULL, pDev, 0);
	} else if (memberCnt == 0 && grpid_in_mac == TRUE) {
		AsicMcastEntryInsert(pAd, &grpid_ip_addr[i][0], pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
							MCAT_FILTER_STATIC | mwds_type, NULL, pDev, 0);
	}

	return 0;

}

INT Set_IgmpSn_DelEntry(RTMP_ADAPTER *pAd, UCHAR *group_entry)
{
	INT i, memberCnt = 0;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[IPV4_ADDR_LEN] = {0};
	BOOLEAN grpid_in_mac = FALSE;
	UCHAR grpid_mac_addr[MAC_ADDR_LEN] = {0};
	UCHAR grpid_ip_addr[32][IPV6_ADDR_LEN] = {0};
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR GroupId[IPV6_ADDR_LEN];
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	MAC_TABLE_ENTRY *pEntry = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&group_entry, "-")) != NULL) {
		if (bGroupId) {
			if (strstr(thisChar, ".") && BtoH(*((UCHAR *)thisChar)) != 0xf) {
				for (i = 0, value = rstrtok(thisChar, "."); value; value = rstrtok(NULL, ".")) {
					if (chk_ipv4_addr_vld(value) == FALSE)
						return -EINVAL;

					if (i >= IPV4_ADDR_LEN)
						return -EINVAL;

					IpAddr[i] = (UCHAR)os_str_tol(value, NULL, 10);

					i++;
				}

				if (i != 4)
					return -EINVAL;  /*Invalid */

				RTMPZeroMemory(GroupId, IPV6_ADDR_LEN);
				CVT_IPV4_IPV6(GroupId, IpAddr);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"group ip (%d.%d.%d.%d)\n",
						IpAddr[0], IpAddr[1], IpAddr[2], IpAddr[3]);

				bGroupId = 0;
			} else if (strstr(thisChar, ":") && BtoH(*((UCHAR *)thisChar)) == 0xf) {
				RTMPZeroMemory(GroupId, IPV6_ADDR_LEN);
				if (ipv6_pton((CHAR *)thisChar, GroupId) == FALSE)
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
							"IPv6 parse fail\n");

				bGroupId = 0;
			} else if (strstr(thisChar, ":") && strlen(thisChar) == 17) {
				/* support add multicast group through MAC address */
				grpid_in_mac = TRUE;

				for (i = 0, value = rstrtok(thisChar, ":"); value; value = rstrtok(NULL, ":")) {
					if ((strlen(value) != 2) || (!isxdigit((unsigned char)value[0])) || (!isxdigit((unsigned char)value[1])))
						return -EINVAL;

					if (i >= MAC_ADDR_LEN)
						return -EINVAL;

					AtoH(value, &grpid_mac_addr[i++], 1);
				}

				if (i != 6)
					return -EINVAL;

				ConvertMulticastMAC2IP(grpid_mac_addr, ETH_P_IP, grpid_ip_addr);

				bGroupId = 0;
			} else {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"Input IP address invalid\n");
				return -EINVAL;
			}

		} else {
			/* refuse the Member if it's not a MAC address. */
			if ((bGroupId == 0) && (strlen(thisChar) != 17))
				continue;

			/*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			for (i = 0, value = rstrtok(thisChar, ":"); value; value = rstrtok(NULL, ":")) {
				if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
					return -EINVAL;  /*Invalid */

				if (i >= MAC_ADDR_LEN)
					return -EINVAL;

				AtoH(value, &Addr[i++], 1);
			}

			if (i != 6)
				return -EINVAL;  /*Invalid */

			pEntry = MacTableLookup(pAd, Addr);
			memberCnt++;

			if (memberCnt > 0) {
				if (pEntry && grpid_in_mac == FALSE)
					AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
								Addr, pDev, pEntry->wcid);
				else if (!pEntry && grpid_in_mac == FALSE)
					AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
								Addr, pDev, 0);
				else if (pEntry && grpid_in_mac == TRUE) {
					for (i = 0; i < 32; i++)
						AsicMcastEntryDelete(pAd, &grpid_ip_addr[i][0], pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
											Addr, pDev, pEntry->wcid);
				}
			}
		}
	}

	if (memberCnt == 0 && grpid_in_mac == FALSE)
		AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, NULL, pDev, 0);
	else if (memberCnt == 0 && grpid_in_mac == TRUE) {
		for (i = 0; i < 32; i++)
			AsicMcastEntryDelete(pAd, &grpid_ip_addr[i][0], pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
								NULL, pDev, 0);
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "("MACSTR")\n", MAC2STR(Addr));
	return 0;

}


#ifdef IGMP_SNOOPING_DENY_LIST
INT Set_IgmpSn_Deny(RTMP_ADAPTER *pAd, UCHAR *deny_entry)
{
	UINT i = 0, ii = 0;
	UINT8 entry_cnt = 0, add_to_list = 0;
	RTMP_STRING *value = NULL;
	RTMP_STRING *this_char = NULL;
	RTMP_STRING *this_char_temp = NULL;
	UCHAR ip_addr[IPV4_ADDR_LEN] = {0};
	UCHAR ip6_addr[IPV6_ADDR_LEN] = {0};
	UCHAR deny_list[IGMP_DENY_TABLE_SIZE_MAX][IPV6_ADDR_LEN] = {0};
	RTMP_STRING str_temp[64] = {'\0'};
	RTMP_STRING *pstr_temp = NULL;
	UCHAR Prefix = 0;
	UCHAR Prefix_list[IGMP_DENY_TABLE_SIZE_MAX] = {0};
	UCHAR *pIP = NULL;
	UCHAR *pPrefix = NULL;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	PNET_DEV pDev;

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ?
			(pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"Deny list mwctl called\n");

	while ((this_char_temp = strsep((char **)&deny_entry, ";")) != NULL) {
		Prefix = 0;
		pstr_temp = str_temp;
		if (strlen(this_char_temp) <= ARRAY_SIZE(str_temp))
			NdisMoveMemory(pstr_temp, this_char_temp, strlen(this_char_temp));
		else
			return FALSE;

		pstr_temp[strlen(this_char_temp)] = '\0';

		pIP = strsep((char **)&pstr_temp, "/");
		if (pstr_temp && (*pstr_temp != '\0')) {
			pPrefix = strsep((char **)&pstr_temp, ";");
			if (pPrefix)
				Prefix = (UCHAR)os_str_tol(pPrefix, NULL, 10);
		}

		this_char = strsep((char **)&this_char_temp, "/");
		if (strstr(this_char, ".") && BtoH(*((UCHAR *)this_char)) != 0xf) {
			for (i = 0, value = rstrtok(this_char, "."); value && (i < ARRAY_SIZE(ip_addr)); value = rstrtok(NULL, ".")) {
				if (!(strlen(value) > 0 && strlen(value) <= 3))
					return FALSE;

				for (ii = 0; ii < strlen(value); ii++) {
					if (!isxdigit(*(value + ii)))
						return FALSE;
				}

				ip_addr[i] = (UCHAR)os_str_tol(value, NULL, 10);
				i++;
			}

			/* use IgmpSnoopDeny = 0. to delete all */
			if (i == 1)
				break;

			if (i != IPV4_ADDR_LEN)
				return FALSE;

			RTMPZeroMemory(ip6_addr, IPV6_ADDR_LEN);
			CVT_IPV4_IPV6(ip6_addr, ip_addr);
			if ((Prefix == 0) || (Prefix > 32))
				Prefix = 128;
			else
				Prefix += 96;
		} else if (strstr(this_char, ":") && BtoH(*((UCHAR *)this_char)) == 0xf) {
			RTMPZeroMemory(ip6_addr, IPV6_ADDR_LEN);
			if (ipv6_pton((CHAR *)this_char, ip6_addr) == FALSE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
							"IPv6 parse fail\n");
				return -EINVAL;
			}
			if ((Prefix == 0) || (Prefix > 128))
				Prefix = 128;
		} else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"Input IP address invalid\n");
			return -EINVAL;
		}

		NdisMoveMemory(&deny_list[entry_cnt], ip6_addr, IPV6_ADDR_LEN);
		Prefix_list[entry_cnt] = Prefix;
		entry_cnt++;
	}

	if (entry_cnt > 0)
		add_to_list = 1;
	AsicMcastEntryDenyList(pAd, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
		entry_cnt, add_to_list, (UINT8 *)deny_list, (UINT8 *)Prefix_list);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"IF(%s), entry_cnt: %d, add_to_list: %d\n",
			RTMP_OS_NETDEV_GET_DEVNAME(pDev), entry_cnt, add_to_list);

	return 0;

}
#endif
#endif

INT Set_IgmpSn_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;
	UINT Enable;
#ifdef IGMP_TVM_SUPPORT
	UINT32 TVMode = IGMP_TVM_MODE_DISABLE;
	RTMP_STRING *pTVMode = NULL;
	UINT32 IsTVModeEnable = IGMP_TVM_SWITCH_DISABLE;
	RTMP_STRING *pIgmpEnhancedEn = NULL;
#endif /* IGMP_TVM_SUPPORT */
	INT ifIndex = 0;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		if ((ifIndex < 0) || (ifIndex >= MAX_MULTI_STA)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid apcli id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->StaCfg[ifIndex].wdev;
	}
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"ioctl_if_type:%d error.\n", pObj->ioctl_if_type);
		return FALSE;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "wdev is NULL\n");
		return FALSE;
	}
	Enable = (UINT) simple_strtol(arg, 0, 10);

	wdev->IgmpSnoopEnable = (BOOLEAN)(Enable == 0 ? FALSE : TRUE);
#ifdef IGMP_SNOOPING_NON_OFFLOAD
	if (wdev->IgmpSnoopEnable) {
	    MTWF_PRINT("IGMP snooping has enabled.\n");
	} else {
	    MTWF_PRINT("IGMP snooping has disabled.\n");
	}
#endif
#ifdef IGMP_TVM_SUPPORT

	if (wdev->IgmpSnoopEnable) {
		/* Just remove First Enable Parameter from arg */
		if (strsep(&arg, "-")) {
			pIgmpEnhancedEn = strsep(&arg, "-");
			if (pIgmpEnhancedEn) {
				IsTVModeEnable = os_str_toul(pIgmpEnhancedEn, 0, 10);

				pTVMode = strsep(&arg, "-");
				if (pTVMode)
					TVMode = os_str_toul(pTVMode, 0, 10);
			}
		}
	}
	if (IgmpSnEnableTVMode(pAd, wdev, IsTVModeEnable, TVMode)) {
		Enable = TVMode;
	}


	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"IgmpSnoopEnable = %u, IsTVModeEnable = %u, TVModeType = %u\n",
			wdev->IgmpSnoopEnable, wdev->IsTVModeEnable, wdev->TVModeType);
#else
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"wdev[%d],OMAC[%d]-%s\n", wdev->wdev_idx, wdev->DevInfo.OwnMacIdx,
			Enable == TRUE ? "Enable IGMP Snooping":"Disable IGMP Snooping");
#endif /* IGMP_TVM_SUPPORT */

#ifndef IGMP_SNOOPING_NON_OFFLOAD
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		CmdMcastCloneEnable(pAd, Enable, wdev->DevInfo.BandIdx, wdev->DevInfo.OwnMacIdx);
#endif

	return TRUE;
}

INT Set_IgmpSn_Allow_Non_Memb_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Enable;

	Enable = (UINT) simple_strtol(arg, 0, 10);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"%s\n", Enable == TRUE ? "Enable non-member igmp":"Disable non-member igmp");

#ifdef IGMP_SNOOPING_NON_OFFLOAD
	pAd->mcsnoop_policy = Enable ? UNKNOWN_FLOODING : UNKNOWN_DROP;
#else
	CmdMcastAllowNonMemberEnable(pAd, IGMPSN_G_POLICY, Enable);
#endif

	return TRUE;
}

INT Set_IgmpSn_Allow_Mwds_Clone_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT Enable;

	Enable = (UINT)os_str_tol(arg, NULL, 10);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
		"%s\n", Enable == TRUE ? "Enable igmp mwds clone":"Disable igmp mwds clone");

#ifdef IGMP_SNOOPING_NON_OFFLOAD
	pAd->mcsnoop_mwds_clone_policy = Enable;
#else
	CmdMcastAllowMwdsCloneEnable(pAd, IGMPSN_MWDS_CLONE_POLICY, Enable);
#endif

	return TRUE;
}


INT Set_IgmpSn_AddEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i = 0, memberCnt = 0;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[4] = { 0 };
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR GroupId[IPV6_ADDR_LEN];
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	UCHAR mwds_type = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		/* refuse the Member if it's not a MAC address. */
		if ((bGroupId == 0) && (strlen(thisChar) != 17))
			continue;

		if (strlen(thisChar) == 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			for (i = 0, value = rstrtok(thisChar, ":"); value && i < MAC_ADDR_LEN;
					value = rstrtok(NULL, ":")) {
				if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
					return FALSE;  /*Invalid */

				AtoH(value, &Addr[i++], 1);
			}

			if (i != 6)
				return FALSE;  /*Invalid */
		} else {
			for (i = 0, value = rstrtok(thisChar, "."); value && i < 4;
					value = rstrtok(NULL, ".")) {
				if ((strlen(value) > 0) && (strlen(value) <= 3)) {
					int ii;

					for (ii = 0; ii < strlen(value); ii++)
						if (!isxdigit(*(value + ii)))
							return FALSE;
				} else
					return FALSE;  /*Invalid */

				IpAddr[i] = (UCHAR)os_str_tol(value, NULL, 10);
				i++;
			}

			if (i != 4)
				return FALSE;  /*Invalid */
		}

		if (bGroupId == 1) {
			RTMPZeroMemory(GroupId, IPV6_ADDR_LEN);
			CVT_IPV4_IPV6(GroupId, IpAddr);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "group ip (%pI4)\n", IpAddr);
		}
		if (bGroupId == 0) {
			pEntry = MacTableLookup(pAd, Addr);
			memberCnt++;
		}
#ifdef A4_CONN
		if ((memberCnt > 0) && pEntry && (pAd->ApCfg.MBSSID[ifIndex].wdev.wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry)) {
			mwds_type |= MCAT_FILTER_MWDS_CLI;	/* set info about message on MWDS link*/
		}
#endif

		/* Group-Id must be a MCAST address. */
		if ((memberCnt > 0) && !IS_MULTICAST_MAC_ADDR(Addr)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "("MACSTR")\n", MAC2STR(Addr));
			if (pEntry)
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC | mwds_type, Addr, pDev, pEntry->wcid);
			else
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, Addr, pDev, 0);
		}

		bGroupId = 0;
	}
	if ((memberCnt == 0)) {
		AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC | mwds_type, NULL, pDev, 0);
	}

	return TRUE;
}

INT Set_IgmpSn_DelEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i, memberCnt = 0;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[4] = { 0 };
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR GroupId[IPV6_ADDR_LEN];
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	MAC_TABLE_ENTRY *pEntry = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		/* refuse the Member if it's not a MAC address. */
		if ((bGroupId == 0) && (strlen(thisChar) != 17))
			continue;

		if (strlen(thisChar) == 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			for (i = 0, value = rstrtok(thisChar, ":"); value && i < MAC_ADDR_LEN; value = rstrtok(NULL, ":")) {
				if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
					return FALSE;  /*Invalid */

				AtoH(value, &Addr[i++], 1);
			}

			if (i != 6)
				return FALSE;  /*Invalid */
		} else {
			for (i = 0, value = rstrtok(thisChar, "."); value && i < 4; value = rstrtok(NULL, ".")) {
				if ((strlen(value) > 0) && (strlen(value) <= 3)) {
					int ii;

					for (ii = 0; ii < strlen(value); ii++)
						if (!isxdigit(*(value + ii)))
							return FALSE;
				} else
					return FALSE;  /*Invalid */

				IpAddr[i] = (UCHAR)os_str_tol(value, NULL, 10);
				i++;
			}

			if (i != 4)
				return FALSE;  /*Invalid */
		}

		pEntry = MacTableLookup(pAd, Addr);

		if (bGroupId == 1) {
			RTMPZeroMemory(GroupId, IPV6_ADDR_LEN);
			CVT_IPV4_IPV6(GroupId, IpAddr);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "group ip (%d.%d.%d.%d)\n",
					 IpAddr[0], IpAddr[1], IpAddr[2], IpAddr[3]);
		} else {
			pEntry = MacTableLookup(pAd, Addr);
			memberCnt++;
		}

		if (memberCnt > 0) {
			if (pEntry)
				AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
							Addr, pDev, pEntry->wcid);
			else
				AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
							Addr, pDev, 0);
		}

		bGroupId = 0;
	}

	if (memberCnt == 0)
		AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, NULL, pDev, 0);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "("MACSTR")\n", MAC2STR(Addr));
	return TRUE;
}

INT Set_IgmpSn_CfgPerBandSize_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;
	INT ifIndex = 0;
	UINT total = 0;
	UINT8 i = 0, bCfg = 1, Enable;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR perBandEnable = 0;
	UINT perBandSize[3] = {0};

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	}
#ifdef APCLI_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		if ((ifIndex < 0) || (ifIndex >= MAX_MULTI_STA)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
					"invalid apcli id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->StaCfg[ifIndex].wdev;
	}
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"ioctl_if_type:%d error.\n", pObj->ioctl_if_type);
		return FALSE;
	}

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "wdev is NULL\n");
		return FALSE;
	}

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		if (bCfg == 1) {
			if (strlen(thisChar) != 1)
				return FALSE;
			if (kstrtou8(thisChar, 10, &Enable))
				return FALSE;
			perBandEnable = Enable;
			bCfg = 0;
		} else if (Enable == 1) {
			for (i = 0, value = rstrtok(thisChar, ":"); value && i < MAX_BAND_NUM; value = rstrtok(NULL, ":"), i++) {
				if (strlen(value) > 0 && strlen(value) <= 3) {
					if (kstrtou32(value, 10, &perBandSize[i]))
						return FALSE;
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_NOTICE,
							"config band%d m2u table size = %d\n", i, perBandSize[i]);
					total += perBandSize[i];
				}
			}

			if (total > MAX_LEN_OF_MULTICAST_FILTER_TABLE) {
				perBandEnable = 0;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"config num exceeds MAX_LEN_OF_MULTICAST_FILTER_TABLE\n");
				return FALSE;
			}
		}
	}

#ifdef IGMP_SNOOPING_OFFLOAD
	CmdMcastPerBandSize(pAd, perBandEnable, perBandSize[0], perBandSize[1], perBandSize[2]);
#else
	if (!pAd->pMulticastFilterTable) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
			"pAd->pMulticastFilterTable is NULL\n");
		return FALSE;
	}
	pAd->pMulticastFilterTable->size_cfg.enable = perBandEnable;
	for (i = 0; i < MAX_BAND_NUM; i++)
		pAd->pMulticastFilterTable->size_cfg.cfg_num[i] = perBandSize[i];
#endif
	return TRUE;
}

#ifdef IGMP_SNOOPING_DENY_LIST
VOID MulticastDLTableInit(PRTMP_ADAPTER pAd,
						P_MULTICAST_DENY_LIST_FILTER_TABLE * ppMulticastDLTable)
{
	os_alloc_mem(NULL, (UCHAR **)ppMulticastDLTable,
				sizeof(MULTICAST_DENY_LIST_FILTER_TABLE));

	if (*ppMulticastDLTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"unable to alloc memory for multicast deny list table\n");
		return;
	}

	NdisZeroMemory(*ppMulticastDLTable, sizeof(MULTICAST_DENY_LIST_FILTER_TABLE));
	return;
}

VOID MulticastDLTableReset(PRTMP_ADAPTER pAd,
						P_MULTICAST_DENY_LIST_FILTER_TABLE * ppMulticastDLTable)
{
	if (*ppMulticastDLTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
				"multicast deny list table is not ready\n");
		return;
	}

	os_free_mem(*ppMulticastDLTable);
	return;
}


INT Set_IgmpSn_Deny_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i = 0, ii = 0;
	UINT8 entry_cnt = 0, add_to_list = 0;
	RTMP_STRING *value = NULL;
	RTMP_STRING *this_char = NULL;
	UCHAR ip_addr[IPV4_ADDR_LEN] = {0};
	UCHAR deny_list[IGMP_DENY_TABLE_SIZE_MAX][IPV4_ADDR_LEN] = {0};
	POS_COOKIE pObj;
	UCHAR ifIndex;
	PNET_DEV pDev;
	UCHAR Prefix_list[IGMP_DENY_TABLE_SIZE_MAX] = {0};

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ?
			(pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"Deny list ioctl called\n");

	while((this_char = strsep((char **)&arg, ";")) != NULL) {
		for (i = 0, value = rstrtok(this_char, "."); value && i < IPV4_ADDR_LEN; value = rstrtok(NULL, ".")) {
			if (!(strlen(value) > 0 && strlen(value) <= 3))
				return FALSE;
			for (ii = 0; ii < strlen(value); ii++) {
				if (!isxdigit(*(value + ii)))
					return FALSE;
			}

			ip_addr[i] = (UCHAR)os_str_tol(value, NULL, 10);
			i++;
		}

		/* use IgmpSnoopDeny = 0 to delete all */
		if (i == 1)
			break;

		if (i != IPV4_ADDR_LEN)
			return FALSE;

		NdisMoveMemory(&deny_list[entry_cnt], ip_addr, IPV4_ADDR_LEN);
		entry_cnt++;
	}

	if (entry_cnt > 0)
		add_to_list = 1;
	AsicMcastEntryDenyList(pAd, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex,
		entry_cnt, add_to_list, (UINT8 *)deny_list, (UINT8 *)Prefix_list);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			"IF(%s), entry_cnt: %d, add_to_list: %d\n",
			RTMP_OS_NETDEV_GET_DEVNAME(pDev), entry_cnt, add_to_list);

	return TRUE;
}
#endif

INT Set_IgmpSn_TabDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("=================	Dump IgmpInfo Table  =================\n");

#ifndef IGMP_SNOOPING_NON_OFFLOAD
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		RTMP_STRING IgmpIn[4] = {"1-8"};
		show_tpinfo_proc(pAd, IgmpIn);
	}
	else
#endif
		IGMPTableDisplay(pAd);
	return TRUE;
}

VOID MulticastWLTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_WHITE_LIST_FILTER_TABLE * ppMulticastWLTable)
{
	/* Initialize MAC table and allocate spin lock */
	os_alloc_mem(NULL, (UCHAR **)ppMulticastWLTable, sizeof(MULTICAST_WHITE_LIST_FILTER_TABLE));

	if (*ppMulticastWLTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "unable to alloc memory for Multicase White list table, size=%lu\n",
				 (ULONG)sizeof(MULTICAST_WHITE_LIST_FILTER_TABLE));
		return;
	}

	NdisZeroMemory(*ppMulticastWLTable, sizeof(MULTICAST_WHITE_LIST_FILTER_TABLE));
	NdisAllocateSpinLock(pAd, &((*ppMulticastWLTable)->MulticastWLTabLock));
	return;
}

VOID MultiCastWLTableReset(RTMP_ADAPTER *pAd,
							   IN PMULTICAST_WHITE_LIST_FILTER_TABLE * ppMulticastWLTable)
{
	if (*ppMulticastWLTable == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, "Multicase White list table is not ready.\n");
		return;
	}

	NdisFreeSpinLock(&((*ppMulticastWLTable)->MulticastWLTabLock));
	os_free_mem(*ppMulticastWLTable);
	*ppMulticastWLTable = NULL;
}

INT Set_Igmp_Flooding_CIDR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING IPString[25] = {'\0'};
	RTMP_STRING *pIPString = NULL;
	UCHAR *pOperationType = NULL;
	UCHAR *pIP = NULL;
	UCHAR *pPrefix = NULL;
	UCHAR Prefix = 0;
	BOOLEAN bPrintCmdUsages = FALSE;
	BOOLEAN bAdd = FALSE;
	UCHAR i = 0;
	UCHAR *value = 0;
	INT Result = FALSE;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;
	PMULTICAST_WHITE_LIST_FILTER_TABLE pMcastWLTable = pAd->pMcastWLTable;
	MULTICAST_WHITE_LIST_ENTRY ThisMcastEntry = {0};


	do{
		if (pMcastWLTable == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					"Mcast White List Not init, skip this operation\n");
			break;
		}

		if (strlen(arg) <= ARRAY_SIZE(IPString)) {
			pIPString = IPString;
			NdisMoveMemory(pIPString, arg, strlen(arg));
			pIPString[strlen(arg)] = '\0';

			pOperationType = strsep((char **)&pIPString, "-");
			if (pIPString && (*pIPString != '\0')) {
				pIP = strsep((char **)&pIPString, "/");
				if (pIPString && (*pIPString != '\0')) {
					pPrefix = strsep((char **)&pIPString, "-");
					if (pPrefix) {
						Prefix = (UCHAR)os_str_tol(pPrefix, NULL, 10);
					}
				}
			}
		}

		/* Check if incorrect or no operation entered */
		if ((pOperationType == NULL) || ((*pOperationType != '0') && (*pOperationType != '1'))) {
			MTWF_PRINT("%s() Incorrect Operation Type entered, skip this operation\n", __func__);
			bPrintCmdUsages = TRUE;
			break;
		}

		bAdd = (BOOLEAN)(((UCHAR)os_str_tol(pOperationType, NULL, 10) == 0) ? FALSE : TRUE);

		/* Check if add operation but IP not entered */
		if ((bAdd == TRUE) && (pIP == NULL)) {
			MTWF_PRINT("%s() Mcast IP address not entered, skip this operation\n", __func__);
			bPrintCmdUsages = TRUE;
			break;
		}

		OS_SEM_LOCK(&pMcastWLTable->MulticastWLTabLock);
		if ((bAdd == TRUE) && (pMcastWLTable->EntryNum >= MULTICAST_WHITE_LIST_SIZE_MAX)) {
			MTWF_PRINT("%s() Mcast List Already full, skip this operation\n", __func__);
			bPrintCmdUsages = TRUE;
			OS_SEM_UNLOCK(&pMcastWLTable->MulticastWLTabLock);
			break;
		}
		OS_SEM_UNLOCK(&pMcastWLTable->MulticastWLTabLock);

		if ((bAdd == FALSE) && (pIP == NULL)) {
			PMULTICAST_WHITE_LIST_ENTRY pEntryTab = NULL;
			OS_SEM_LOCK(&pMcastWLTable->MulticastWLTabLock);
			MTWF_PRINT("%s() Delete full list\n", __func__);
			for (i = 0; i < MULTICAST_WHITE_LIST_SIZE_MAX; i++) {
				pEntryTab = &pMcastWLTable->EntryTab[i];
				if (pEntryTab->bValid == TRUE) {
					NdisZeroMemory((PUCHAR)pEntryTab, sizeof(MULTICAST_WHITE_LIST_ENTRY));
					pEntryTab->bValid = FALSE;
					pMcastWLTable->EntryNum -= 1;
				}
			}
			Result = TRUE;
#ifndef IGMP_SNOOPING_NON_OFFLOAD
			CmdMcastFloodingCIDR(pAd, pMcastWLTable->EntryTab[0].EntryIPType, FALSE, pMcastWLTable->EntryTab[0].Addr , pMcastWLTable->EntryTab[0].PrefixMask.DWord);
#endif
			OS_SEM_UNLOCK(&pMcastWLTable->MulticastWLTabLock);
			break;
		}

		if ((strlen(pIP) >= 9) && (strlen(pIP) <= 15)) {
			for (i = 0, value = rstrtok(pIP, "."); value && i < IPV4_ADDR_LEN; value = rstrtok(NULL, "."), i++) {
				UCHAR ii = 0;
				if (strlen(value) > 3) {
					bPrintCmdUsages = TRUE;
					break;
				}
				for (ii = 0; ii < strlen(value); ii++) {
					if (!isdigit(*(value + ii))) {
						bPrintCmdUsages = TRUE;
						break;
					}
				}
				if (bPrintCmdUsages == TRUE)
					break;

				ThisMcastEntry.IPData.IPv4[i] = (UCHAR)os_str_tol(value, NULL, 10);
			}
			/* Check if any invalid multicast address specified */
			if ((bPrintCmdUsages == TRUE) ||
				(ThisMcastEntry.IPData.IPv4[0] < 224) ||
				(ThisMcastEntry.IPData.IPv4[0] > 239)) {
				bPrintCmdUsages = TRUE;
				break;
			}
			ThisMcastEntry.bValid = TRUE;
			ThisMcastEntry.EntryIPType = IP_V4;
			if (Prefix == 0)
				Prefix = 32;
		}

		if (ThisMcastEntry.bValid == TRUE) {
			ThisMcastEntry.PrefixLen = Prefix;
			if (bAdd == TRUE) {
				PMULTICAST_WHITE_LIST_ENTRY pEntryTab = NULL;
				OS_SEM_LOCK(&pMcastWLTable->MulticastWLTabLock);
				/* First check if there is any existing entry */
				for (i = 0; i < MULTICAST_WHITE_LIST_SIZE_MAX; i++) {
					pEntryTab = &pMcastWLTable->EntryTab[i];
					if ((pEntryTab->bValid == TRUE) &&
						(((ThisMcastEntry.EntryIPType == IP_V4) && (pEntryTab->EntryIPType == IP_V4) &&
						NdisEqualMemory(pEntryTab->IPData.IPv4, ThisMcastEntry.IPData.IPv4, IPV4_ADDR_LEN))) &&
						(pEntryTab->PrefixLen == ThisMcastEntry.PrefixLen)) {
						Result = TRUE;
						MTWF_PRINT("This IP address already present in Igmp Flooding CIDR table\n\n");
						break;
					}
				}
				OS_SEM_UNLOCK(&pMcastWLTable->MulticastWLTabLock);
				/* IP entry already present, no need to add again, so exit */
				if (Result == TRUE)
					break;

				OS_SEM_LOCK(&pMcastWLTable->MulticastWLTabLock);
				/* Find the empty entry in white list entry to add new entry */
				for (i = 0; i < MULTICAST_WHITE_LIST_SIZE_MAX; i++) {
					UCHAR Index = 0;
					UCHAR Bits = 0;
					UINT32 Mask = 0;
					if (pMcastWLTable->EntryTab[i].bValid == FALSE) {
						NdisZeroMemory(&pMcastWLTable->EntryTab[i],
										sizeof(MULTICAST_WHITE_LIST_ENTRY));
						NdisMoveMemory(&pMcastWLTable->EntryTab[i],
										&ThisMcastEntry,
										sizeof(MULTICAST_WHITE_LIST_ENTRY));
						pMcastWLTable->EntryNum += 1;

						if (pMcastWLTable->EntryTab[i].EntryIPType == IP_V4)
							ConvertMulticastIP2MAC(ThisMcastEntry.IPData.IPv4, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
						else
							ConvertMulticastIP2MAC(ThisMcastEntry.IPData.IPv6, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
						COPY_MAC_ADDR(pMcastWLTable->EntryTab[i].Addr, GroupMacAddr);

						/* Prepare Mask of bytes from Prefix Length to be matched with IP address of entery and packets received */
						Index = 0;
						do {
							/* here 32 = 32 bits in a DWord */
							Bits = ((Prefix%32)?(Prefix%32):32);
							if (Bits == 32)
								Mask = ((UINT32)~0);
							else
								Mask = ( ((UINT32)~0) << (UINT32)(32-Bits) );

							pMcastWLTable->EntryTab[i].PrefixMask.DWord[Index] = Mask;
							Prefix = Prefix - ((Prefix%32)?(Prefix%32):32);
							Index += 1;
						} while (Prefix > 0);
						MTWF_PRINT("White list %sING: IPv4 addr = %d.%d.%d.%d%s%s\n", ((bAdd == TRUE) ? "ADD":"DELETE"),
							ThisMcastEntry.IPData.IPv4[0], ThisMcastEntry.IPData.IPv4[1], ThisMcastEntry.IPData.IPv4[2],ThisMcastEntry.IPData.IPv4[3],
							(PUCHAR)((pPrefix) ? (PUCHAR)"/" : (PUCHAR)""), (PUCHAR)((pPrefix) ? pPrefix : (PUCHAR)""));
						MTWF_PRINT("This IP address added in Igmp Flooding CIDR table\n\n");
#ifndef IGMP_SNOOPING_NON_OFFLOAD
						if (pMcastWLTable->EntryTab[i].EntryIPType == IP_V4){
							CmdMcastFloodingCIDR(pAd, pMcastWLTable->EntryTab[i].EntryIPType, TRUE, pMcastWLTable->EntryTab[i].Addr, pMcastWLTable->EntryTab[i].PrefixMask.DWord);
						}
#endif
						break;
					}
				}
				OS_SEM_UNLOCK(&pMcastWLTable->MulticastWLTabLock);
			} else {
				/* Find the this entry in white list entry to delete it from list */
				PMULTICAST_WHITE_LIST_ENTRY pEntryTab = NULL;
				Result = TRUE;
				OS_SEM_LOCK(&pMcastWLTable->MulticastWLTabLock);
				for (i = 0; i < MULTICAST_WHITE_LIST_SIZE_MAX; i++) {
					UCHAR Index = 0;
					UCHAR Bits = 0;
					UINT32 Mask = 0;
					pEntryTab = &pMcastWLTable->EntryTab[i];
					if ((pEntryTab->bValid == TRUE) &&
						(((ThisMcastEntry.EntryIPType == IP_V4) && (pEntryTab->EntryIPType == IP_V4) &&
						NdisEqualMemory(pEntryTab->IPData.IPv4, ThisMcastEntry.IPData.IPv4, IPV4_ADDR_LEN))) &&
						(pEntryTab->PrefixLen == ThisMcastEntry.PrefixLen)) {
						NdisZeroMemory((PUCHAR)pEntryTab,
								sizeof(MULTICAST_WHITE_LIST_ENTRY));
						pEntryTab->bValid = FALSE;
						pMcastWLTable->EntryNum -= 1;

						if (pMcastWLTable->EntryTab[i].EntryIPType == IP_V4)
							ConvertMulticastIP2MAC(ThisMcastEntry.IPData.IPv4, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
						else
							ConvertMulticastIP2MAC(ThisMcastEntry.IPData.IPv6, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
						COPY_MAC_ADDR(pMcastWLTable->EntryTab[i].Addr, GroupMacAddr);
						/* Prepare Mask of bytes from Prefix Length to be matched with IP address of entery and packets received */
						Index = 0;
						do {
							/* here 32 = 32 bits in a DWord */
							Bits = ((Prefix%32)?(Prefix%32):32);
							if (Bits == 32)
								Mask = ((UINT32)~0);
							else
								Mask = (((UINT32)~0) << (UINT32)(32-Bits));
							pMcastWLTable->EntryTab[i].PrefixMask.DWord[Index] = Mask;
							Prefix = Prefix - ((Prefix%32)?(Prefix%32):32);
							Index += 1;
						} while (Prefix > 0);
						MTWF_PRINT("White list %sING: IPv4 addr = %d.%d.%d.%d%s%s\n", ((bAdd == TRUE) ? "ADD":"DELETE"),
							ThisMcastEntry.IPData.IPv4[0], ThisMcastEntry.IPData.IPv4[1], ThisMcastEntry.IPData.IPv4[2],ThisMcastEntry.IPData.IPv4[3],
							(PUCHAR)((pPrefix) ? (PUCHAR)"/" : (PUCHAR)""), (PUCHAR)((pPrefix) ? pPrefix : (PUCHAR)""));
						MTWF_PRINT("This IP address deleted from Flooding CIDR table\n\n");
#ifndef IGMP_SNOOPING_NON_OFFLOAD
						if (pMcastWLTable->EntryTab[i].EntryIPType == IP_V4){
							CmdMcastFloodingCIDR(pAd, pMcastWLTable->EntryTab[i].EntryIPType, FALSE, pMcastWLTable->EntryTab[i].Addr, pMcastWLTable->EntryTab[i].PrefixMask.DWord);
						}
						break;
#endif
					}
				}
				OS_SEM_UNLOCK(&pMcastWLTable->MulticastWLTabLock);
			}
			Result = TRUE;
		}else{
			MTWF_PRINT("Invalid command!\n");
			bPrintCmdUsages = TRUE;
			break;
		}
	}while(FALSE);

	if (bPrintCmdUsages == TRUE) {
		MTWF_PRINT("\nCommand usages:\n"
			"	iwpriv ra0 set IgmpFloodingCIDR=<Operation>-<IP Addr>/<PrefixLength>\n"
			"		<Operation>     :	1 for ADD and 0 for DELETE\n"
			"		<IP Addr>       :	IPv4 address which is allowed to receive pkts\n"
			"		<PrefixLength>  :	Prefix Length, Value between 1 to 32 for IPv4\n"
			"\n"
			"		IPv4 address format :\n"
			"			Example = 224.0.0.1/24\n"
			"\n"
			"	Example : iwpriv ra0 set IgmpFloodingCIDR=1-224.0.0.1/24\n"
			"		or  : iwpriv ra0 set IgmpFloodingCIDR=0-224.0.0.1/24\n");

		}
#if defined(RT_CFG80211_SUPPORT) && defined(IGMP_SNOOPING_NON_OFFLOAD)
	if (Result == TRUE)
		return 0;
	else
		return -EINVAL;
#else
	return Result;
#endif
}

INT Set_Igmp_Show_Flooding_CIDR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT Result = TRUE;
	UCHAR idx = 0;
	PMULTICAST_WHITE_LIST_FILTER_TABLE pFloodingCIDRFilterTable = pAd->pMcastWLTable;

	MTWF_PRINT("=================Dump Driver Table=================\n\n");

	if (pFloodingCIDRFilterTable == NULL) {
		MTWF_PRINT("%s() Mcast White List Not init, skip this operation\n", __func__);
		return Result;
	}

	OS_SEM_LOCK(&pFloodingCIDRFilterTable->MulticastWLTabLock);
	if (pFloodingCIDRFilterTable->EntryNum == 0) {
		OS_SEM_UNLOCK(&pFloodingCIDRFilterTable->MulticastWLTabLock);
		MTWF_PRINT("Table empty.\n");
		return Result;
	}


	for (idx = 0; idx < MULTICAST_WHITE_LIST_SIZE_MAX; idx++) {
		if (pFloodingCIDRFilterTable->EntryTab[idx].bValid) {
			MTWF_PRINT("entry #%d, GrpId="MACSTR"/%d\n\n",
				idx,  MAC2STR(pFloodingCIDRFilterTable->EntryTab[idx].Addr),pFloodingCIDRFilterTable->EntryTab[idx].PrefixLen);
		}
	}
	OS_SEM_UNLOCK(&pFloodingCIDRFilterTable->MulticastWLTabLock);

	return Result;
}

INT Set_Igmp_Show_FW_Flooding_CIDR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING FWIn[4] = {"1-8"};
	MTWF_PRINT("=================  Dump FW Table  =================\n\n");
	show_tpinfo_proc(pAd, FWIn);
	return TRUE;
}

#ifdef A4_CONN
/* Indicate Whether specified member is present on MWDS link */
BOOLEAN isMemberOnMWDSLink(VOID *prMemberEntry)
{
	PMEMBER_ENTRY pMemberEntry = (PMEMBER_ENTRY) prMemberEntry;

	if (pMemberEntry->onMWDSLink != FALSE)
		return TRUE;
	else
		return FALSE;

}
#endif

#ifdef IGMP_SNOOPING_NON_OFFLOAD
NDIS_STATUS igmp_snoop_non_offload(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	INT InIgmpGroup = IGMP_NONE;
	MULTICAST_FILTER_TABLE_ENTRY *pGroupEntry = NULL;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	UCHAR user_prio = RTMP_GET_PACKET_UP(pkt);
	UCHAR *pkt_va = RTMP_GET_PKT_SRC_VA(pkt);
	UCHAR q_idx = RTMP_GET_PACKET_QUEIDX(pkt);

	/* For Panther's IGMP snooping implemented in driver. */
	if (!wdev->IgmpSnoopEnable)
		return NDIS_STATUS_MORE_PROCESSING_REQUIRED;

	if (IgmpPktInfoQuery(pAd, pkt_va, pkt, wdev,
			&InIgmpGroup, &pGroupEntry) != NDIS_STATUS_SUCCESS &&
		pAd->mcsnoop_policy == UNKNOWN_DROP) {
		if (isIgmpMldFloodingPkt(pAd, pkt_va))
			goto NotDrop;

		tr_cnt->igmp_unknown_drop++;
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

NotDrop:
	/* if it's a mcast packet in igmp group. ucast clone it for all members in the group. */
	if ((InIgmpGroup == IGMP_IN_GROUP)
		&& pGroupEntry
		&& (IgmpMemberCnt(&pGroupEntry->MemberList) > 0)) {
		NDIS_STATUS PktCloneResult;

		RTMP_SET_PACKET_SN_VLD(pkt, 0);
		PktCloneResult = IgmpPktClone(pAd, wdev, pkt, InIgmpGroup, pGroupEntry,
			q_idx, user_prio, GET_OS_PKT_NETDEV(pkt));
#ifdef IGMP_TVM_SUPPORT
		if (PktCloneResult != NDIS_STATUS_MORE_PROCESSING_REQUIRED)
#endif
		{
			if (PktCloneResult != NDIS_STATUS_SUCCESS)
				tr_cnt->igmp_clone_fail_drop++;
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			return PktCloneResult;
		}
	}
	/* In order to use sta->txq[0] */
	RTMP_SET_PACKET_QUEIDX(pkt, 0);
	return NDIS_STATUS_MORE_PROCESSING_REQUIRED;
}
#endif /* IGMP_SNOOPING_NON_OFFLOAD */

/*
 * If Packet is IGMP or MLD type multicast packet, send packet OUT
 * Else check whether multicast destination address matches any group-id
 * in the multicast filter table, if no match, drop the packet, else have two case
 * If the member-list of the matching entry is empty and AP just forwards packet to all stations
 * Else if AP will do the MC-to-UC conversation base one memberships
 */
NDIS_STATUS IgmpPktInfoQuery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSrcBufVA,
	IN PNDIS_PACKET pPacket,
	IN struct wifi_dev *wdev,
	OUT INT *pInIgmpGroup,
	OUT PMULTICAST_FILTER_TABLE_ENTRY *ppGroupEntry)
{
	PUCHAR pIpHeader = pSrcBufVA + 12;
	PUCHAR pDstIpAddr = pSrcBufVA + 30;
	UINT16 protoType;
	UCHAR GroupIpv6Addr[IPV6_ADDR_LEN];
	PUCHAR pGroupIpv6Addr = GroupIpv6Addr;
	RTMPZeroMemory(GroupIpv6Addr, IPV6_ADDR_LEN);
	protoType = ntohs(*((UINT16 *)(pIpHeader)));

	if (IS_MULTICAST_MAC_ADDR(pSrcBufVA)) {
		BOOLEAN IgmpMldPkt = FALSE;

		if (protoType == ETH_P_IPV6) {
			IgmpMldPkt = IPv6MulticastFilterExcluded(pPacket, pIpHeader);
			pDstIpAddr = pSrcBufVA + 38;
			pGroupIpv6Addr = pDstIpAddr;
		} else {
			IgmpMldPkt = isIgmpPkt(pSrcBufVA, pIpHeader);
			CVT_IPV4_IPV6(GroupIpv6Addr, pDstIpAddr);
		}

#ifdef IGMP_SNOOPING_DENY_LIST
		if (isIgmpSnoopDeny(pAd, pSrcBufVA, pIpHeader)) {
			*ppGroupEntry = MulticastFilterTableLookup(pAd->pMulticastFilterTable,
														pGroupIpv6Addr, wdev->if_dev);
			*pInIgmpGroup = IGMP_NONE;
			return NDIS_STATUS_SUCCESS;
		}
#endif

		*ppGroupEntry = MulticastFilterTableLookup(pAd->pMulticastFilterTable, pGroupIpv6Addr, wdev->if_dev);
		if (IgmpMldPkt) {
			*ppGroupEntry = NULL;
			*pInIgmpGroup = IGMP_PKT;
		} else if (*ppGroupEntry == NULL) {
#ifdef IGMP_TVM_SUPPORT
			/* This code is for following case: */
			/* CASE: [When the group has been formed first, and then the IgmpSnooping is enabled] */
			/* Earlier it was seen, that for the above case, after Group formation but IgmpSnooping OFF */
			/* Packet used to go as Mcast packet, but after enabling IgmpSnooping, */
			/* we used to drop all the Mcast packet beloning to this group */
			*pInIgmpGroup = IGMP_NONE;
			return NDIS_STATUS_SUCCESS;
#else
			return NDIS_STATUS_FAILURE;
#endif /* IGMP_TVM_SUPPORT */
		} else
			*pInIgmpGroup = IGMP_IN_GROUP;
	} else if (IS_BROADCAST_MAC_ADDR(pSrcBufVA)) {
		CVT_IPV4_IPV6(GroupIpv6Addr, pDstIpAddr);
		*ppGroupEntry = MulticastFilterTableLookup(pAd->pMulticastFilterTable, pGroupIpv6Addr, wdev->if_dev);
		if (*ppGroupEntry != NULL)
			*pInIgmpGroup = IGMP_IN_GROUP;
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS IgmpPktClone(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket,
	INT IgmpPktInGroup,
	PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry,
	UCHAR QueIdx,
	UINT8 UserPriority,
	PNET_DEV pNetDev)
{
#ifdef IGMP_TVM_SUPPORT
	NDIS_STATUS nStatus = NDIS_STATUS_SUCCESS;
#endif /* IGMP_TVM_SUPPORT*/
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	PNDIS_PACKET pSkbClone = NULL;
	PMEMBER_ENTRY pMemberEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	USHORT Aid;
	SST Sst = SST_ASSOC;
	UCHAR PsMode = PWR_ACTIVE;
	UCHAR Rate;
	BOOLEAN bContinue;
	PUCHAR pMemberAddr = NULL;
	bContinue = FALSE;

	if ((IgmpPktInGroup == IGMP_IN_GROUP) && (pGroupEntry == NULL))
		return NDIS_STATUS_FAILURE;

	if (IgmpPktInGroup == IGMP_IN_GROUP) {
		pMemberEntry = (PMEMBER_ENTRY)pGroupEntry->MemberList.pHead;

		if (pMemberEntry) {
			pMemberAddr = pMemberEntry->Addr;
			pMacEntry = APSsPsInquiry(pAd, wdev, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
			bContinue = TRUE;
		}
	} else
		return NDIS_STATUS_FAILURE;

	/* check all members of the IGMP group. */
	while (bContinue == TRUE) {
#ifdef IGMP_TVM_SUPPORT
		/* If TV Mode is enabled in AP, then we need to send unicast packet to all connected STA's */
		if (wdev->IsTVModeEnable &&
			((wdev->TVModeType == IGMP_TVM_MODE_DISABLE) ||
			((wdev->TVModeType == IGMP_TVM_MODE_AUTO) &&
			(pMemberEntry->TVMode == IGMP_TVM_IE_MODE_DISABLE)))) {

			nStatus = NDIS_STATUS_MORE_PROCESSING_REQUIRED;

			pMemberEntry = pMemberEntry->pNext;

			if (pMemberEntry != NULL) {
				pMemberAddr = pMemberEntry->Addr;
				pMacEntry = APSsPsInquiry(pAd, wdev, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
				continue;
			} else
				break;
		}
#endif /* IGMP_TVM_SUPPORT */
		if (pMacEntry)
			tr_entry = tr_entry_get(pAd, pMacEntry->wcid);
		if (pMacEntry && tr_entry && (Sst == SST_ASSOC) &&
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) &&
			pMacEntry->wdev == wdev) {
#ifdef A4_CONN
			/*For MWDS member clone use DA from MPDU*/
			if (isMemberOnMWDSLink(pMemberEntry))
				pSkbClone = NULL;
			else
#endif
			{
				OS_PKT_CLONE(pAd, pPacket, pSkbClone, MEM_ALLOC_FLAG);
			}
			if (pSkbClone) {
				RTMP_SET_PACKET_WCID(pSkbClone, pMacEntry->wcid);
				RTMP_SET_PACKET_MCAST_CLONE(pSkbClone, 1);
				RTMP_SET_PACKET_UP(pSkbClone, UserPriority);
#ifdef IGMP_SNOOP_SUPPORT
				pMacEntry->M2U_TxPackets++;
				pMacEntry->M2U_TxBytes += RTMP_GET_PKT_LEN(pPacket);
#endif
				qm_ops->enq_dataq_pkt(pAd, wdev, pSkbClone, QueIdx);

			} else {
				pMemberEntry = pMemberEntry->pNext;

				if (pMemberEntry != NULL) {
					pMemberAddr = pMemberEntry->Addr;
					pMacEntry = APSsPsInquiry(pAd, wdev, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
					bContinue = TRUE;
				} else {
					bContinue = FALSE;
				}
				continue;
			}

			ba_ori_session_start(pAd, tr_entry->wcid, UserPriority);
		}

		pMemberEntry = pMemberEntry->pNext;

		if (pMemberEntry != NULL) {
			pMemberAddr = pMemberEntry->Addr;
			pMacEntry = APSsPsInquiry(pAd, wdev, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
			bContinue = TRUE;
		} else {
			bContinue = FALSE;
		}
	}

#ifdef IGMP_TVM_SUPPORT
	return nStatus;
#else
	return NDIS_STATUS_SUCCESS;
#endif /* IGMP_TVM_SUPPORT */
}

static inline BOOLEAN isMldMacAddr(
	IN PUCHAR pMacAddr)
{
	return ((pMacAddr[0] == 0x33) && (pMacAddr[1] == 0x33)) ? TRUE : FALSE;
}

static inline BOOLEAN IsSupportedMldMsg(
	IN UINT8 MsgType)
{
	BOOLEAN result = FALSE;

	switch (MsgType) {
	case MLD_V1_LISTENER_REPORT:
	case MLD_V1_LISTENER_DONE:
	case MLD_V2_LISTERNER_REPORT:
	case MLD_LISTENER_QUERY:
		result = TRUE;
		break;

	default:
		result = FALSE;
		break;
	}

	return result;
}

BOOLEAN isMldPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader,
	IN PUCHAR pDataEnd,
	OUT UINT8 *pProtoType,
	OUT PUCHAR *pMldHeader)
{
	BOOLEAN result = FALSE;
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));

	if (!isMldMacAddr(pDstMacAddr))
		return FALSE;

	if (IpProtocol != ETH_P_IPV6)
		return FALSE;

	/* skip protocol (2 Bytes). */
	pIpHeader += 2;

	do {
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(pIpHeader);
		UINT8 nextProtocol = pIpv6Hdr->nextHdr;
		UINT32 offset = IPV6_HDR_LEN;

		while (nextProtocol != IPV6_NEXT_HEADER_ICMPV6) {
			if ((pIpHeader + offset + 1) >= pDataEnd)
				return FALSE;
			if (IPv6ExtHdrHandle((RT_IPV6_EXT_HDR *)(pIpHeader + offset), &nextProtocol, &offset) == FALSE)
				break;
		}

		if (nextProtocol == IPV6_NEXT_HEADER_ICMPV6) {
			PRT_ICMPV6_HDR pICMPv6Hdr = (PRT_ICMPV6_HDR)(pIpHeader + offset);

			if ((pIpHeader + offset + sizeof(PRT_ICMPV6_HDR) - 1) >= pDataEnd)
				return FALSE;
			if (IsSupportedMldMsg(pICMPv6Hdr->type) == TRUE) {
				if (pProtoType != NULL)
					*pProtoType = pICMPv6Hdr->type;

				if (pMldHeader != NULL)
					*pMldHeader = (PUCHAR)pICMPv6Hdr;

				result = TRUE;
			}
		}
	} while (FALSE);

	return result;
}

BOOLEAN IPv6MulticastFilterExcluded(
	IN PNDIS_PACKET pPacket,
	IN PUCHAR pIpHeader)
{
	BOOLEAN result = FALSE;
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));
	INT idx;
	UINT8 nextProtocol;
	UCHAR *pkt_data = GET_OS_PKT_DATAPTR(pPacket);
	INT32 pkt_len = (INT32)GET_OS_PKT_LEN(pPacket);
	INT32 remain_len;

	if (!IS_IPV6_MULTICAST_MAC_ADDR(pkt_data))
		return FALSE;

	if (IpProtocol != ETH_P_IPV6)
		return FALSE;

	/* skip protocol (2 Bytes). */
	pIpHeader += 2;

	if (pkt_len < LENGTH_802_3_IPV6) /* LENGTH_802_3 + IPV6_HDR_LEN */
		return FALSE;
	do {
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(pIpHeader);
		INT32 offset = IPV6_HDR_LEN;
		nextProtocol = pIpv6Hdr->nextHdr;

		if (!pIpv6Hdr->payload_len)
			return FALSE;

		remain_len = pkt_len - LENGTH_802_3_IPV6;
		while ((remain_len >= 2) && (nextProtocol == IPV6_NEXT_HEADER_HOP_BY_HOP)) {
			if (IPv6ExtHdrHandle((RT_IPV6_EXT_HDR *)(pIpHeader + offset), &nextProtocol, &offset) == FALSE)
				break;
			remain_len -= (offset+LENGTH_802_3);
		}
	} while (FALSE);

	for (idx = 0; idx < IPV6_MULTICAST_FILTER_EXCLUED_SIZE; idx++) {
		if (nextProtocol == IPv6MulticastFilterExclued[idx]) {
			result = TRUE;
			break;
		}
	}

	return result;
}

/*  MLD v1 messages have the following format:
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Type      |     Code      |          Checksum             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Maximum Response Delay    |          Reserved             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	+                                                               +
	|                                                               |
	+                       Multicast Address                       +
	|                                                               |
	+                                                               +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/*	Version 3 Membership Report Message
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|  Type = 143   |    Reserved   |           Checksum            |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Reserved            |  Number of Group Records (M)  |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [1]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [2]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                               .                               |
	.                               .                               .
	|                               .                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [M]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


	where each Group Record has the following internal format:
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|  Record Type  |  Aux Data Len |     Number of Sources (N)     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Multicast Address                       *
    |                                                               |
    *                                                               *
    |                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [1]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-                                                             -+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [2]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-                                                             -+
    .                               .                               .
    .                               .                               .
    .                               .                               .
    +-                                                             -+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [N]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    .                                                               .
    .                         Auxiliary Data                        .
    .                                                               .
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

VOID MLDSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN PUCHAR pDataEnd,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT16 Wcid)
{
	INT i;
	UCHAR GroupType;
	UINT16 numOfGroup;
	PUCHAR pGroup;
	UCHAR AuxDataLen;
	UINT16 numOfSources;
	PUCHAR pGroupIpAddr;
	UINT8 MldType;
	PUCHAR pMldHeader;
	UINT8 Type = MCAT_FILTER_DYNAMIC;
	struct wifi_dev *wdev = pEntry->wdev;
#ifdef IGMP_TVM_SUPPORT
	UCHAR TVModeType = 0;
#endif /* IGMP_TVM_SUPPORT */

	if (isMldPkt(pDstMacAddr, pIpHeader, pDataEnd, &MldType, &pMldHeader) == TRUE) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "MLD type=%0x\n", MldType);
#ifdef A4_CONN
	if (pEntry && ((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry)))
		Type |= MCAT_FILTER_MWDS_CLI;	/* set info about message on MWDS link*/
#endif

#ifdef IGMP_TVM_SUPPORT
	pEntry = entry_get(pAd, Wcid);

	if (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
		if (pEntry->TVMode == IGMP_TVM_IE_MODE_AUTO)
			TVModeType = MCAT_FILTER_TVM_AUTO;
		else if (pEntry->TVMode == IGMP_TVM_IE_MODE_ENABLE)
			TVModeType = MCAT_FILTER_TVM_ENABLE;
	}
#endif /* IGMP_TVM_SUPPORT */
		switch (MldType) {
		case MLD_V1_LISTENER_REPORT:
			/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
			pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"EntryInsert MLD Group=%pI6\n", pGroupIpAddr);
#ifdef IGMP_TVM_SUPPORT
			if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IPV6) == TRUE) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"Exempt Snooping IGMP Group\n");
				break;
			}
#endif /* IGMP_TVM_SUPPORT */
			AsicMcastEntryInsert(pAd, pGroupIpAddr, wdev->bss_info_argument.ucBssIndex,
				(Type
#ifdef IGMP_TVM_SUPPORT
				|TVModeType
#endif /* IGMP_TVM_SUPPORT */
				), pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case MLD_V1_LISTENER_DONE:
			/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
			pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
					"Listener MLD Group=%pI6\n", pGroupIpAddr);
#ifdef A4_CONN
			if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))) /* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#endif
			AsicMcastEntryDelete(pAd, pGroupIpAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case MLD_V2_LISTERNER_REPORT: /* IGMP version 3 membership report. */
			numOfGroup = ntohs(*((UINT16 *)(pMldHeader + 6)));
			pGroup = (PUCHAR)(pMldHeader + 8);

			if (numOfGroup > MAX_NUM_OF_GRP) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR,
						"numOfGroup %d is wrong according to MLDv2 membership report message\n",
						numOfGroup);
				break;
			}

			for (i = 0; i < numOfGroup; i++) {
				GroupType = (UCHAR)(*pGroup);
				AuxDataLen = (UCHAR)(*(pGroup + 1));
				numOfSources = ntohs(*((UINT16 *)(pGroup + 2)));
				pGroupIpAddr = (PUCHAR)(pGroup + 4);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "MLDv2 Type=%d, ADL=%d, numOfSource=%d\n",
						 GroupType, AuxDataLen, numOfSources);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
						"MLD Group=%pI6\n", pGroupIpAddr);

				do {
					if ((GroupType == MODE_IS_EXCLUDE)
						|| (GroupType == CHANGE_TO_EXCLUDE_MODE)
						|| (GroupType == ALLOW_NEW_SOURCES)) {
#ifdef IGMP_TVM_SUPPORT
						if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IPV6) == TRUE) {
							MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
									"Exempt Snooping MLD Group\n");
							break;
						}
#endif /* IGMP_TVM_SUPPORT */
						AsicMcastEntryInsert(pAd, pGroupIpAddr, wdev->bss_info_argument.ucBssIndex,
							(Type
#ifdef IGMP_TVM_SUPPORT
							|TVModeType
#endif /* IGMP_TVM_SUPPORT */
							), pSrcMacAddr, wdev->if_dev, Wcid);
						break;
					}

					if ((GroupType == CHANGE_TO_INCLUDE_MODE)
						|| (GroupType == MODE_IS_INCLUDE)
						|| (GroupType == BLOCK_OLD_SOURCES)) {
						if (numOfSources == 0) {
#ifdef A4_CONN
							if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))) /* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#endif
								AsicMcastEntryDelete(pAd,
											pGroupIpAddr,
											wdev->bss_info_argument.ucBssIndex,
											pSrcMacAddr,
											wdev->if_dev,
											Wcid);
						} else {
#ifdef IGMP_TVM_SUPPORT
							if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IPV6) == TRUE) {
								MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
										"Exempt Snooping MLD Group\n");
								break;
							}
#endif /* IGMP_TVM_SUPPORT */
							AsicMcastEntryInsert(pAd, pGroupIpAddr, wdev->bss_info_argument.ucBssIndex,
								(Type
#ifdef IGMP_TVM_SUPPORT
								|TVModeType
#endif /* IGMP_TVM_SUPPORT */
								), pSrcMacAddr, wdev->if_dev, Wcid);
						}
						break;
					}
				} while (FALSE);

				/* skip 4 Bytes (Record Type, Aux Data Len, Number of Sources) + a IPv6 address. */
				pGroup += (4 + IPV6_ADDR_LEN + (numOfSources * 16) + AuxDataLen);
			}

			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "unknow MLD Type=%d\n", MldType);
			break;
		}
	}

	return;
}

#ifdef A4_CONN
/* Indicate if Specific Pkt is an MLD query message*/
BOOLEAN isMLDquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader,
	IN PUCHAR pDataEnd)
{
	UINT8 MldType = 0;
	PUCHAR pMldHeader;
	BOOLEAN isMLDquery = FALSE;

	if (isMldPkt(pDstMacAddr, pIpHeader, pDataEnd, &MldType, &pMldHeader) == TRUE) {
		switch (MldType) {
		case MLD_LISTENER_QUERY:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "isMLDquery-> MLD type=0x%x MLD_LISTENER_QUERY\n", MldType);
			isMLDquery = TRUE;
			break;

		}
	}

	return isMLDquery;
}

UCHAR	TYPEIPV4[] = {0x08, 0x00};

/* Send an IGMP query message on particular AP interface*/
void send_igmpv3_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry)
{
	UCHAR ALL_HOST_ADDR[MAC_ADDR_LEN] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01}; /* as per IGMP spec */
	UCHAR Header802_3[14] = {0};
	UCHAR CustomPayload[36];

	UCHAR IpHdr[24] = {0x46, 0x00, /* version(4bit), hdr lenght(4bit), tos*/
							   0x00, 0x24, /* total ip datagram length*/
							   0x00, 0x01, /* identification (random)*/
							   0x00, 0x00, /* flag & fragmentation*/
							   0x01, 0x02, /* TTL, Protocol type (as per igmp spec)*/
							   0x44, 0xD2, /* hdr checksum (considered 0 for calculation and computed manually for this msg)*/
							   0x00, 0x00, /* Source IP (0.0.0.0)*/
							   0x00, 0x00, /* Source IP*/
							   0xE0, 0x00, /* Dest IP (224.0.0.1 - All Host addr as per IGMP spec)*/
							   0x00, 0x01, /* Dest IP*/
							   0x94, 0x04, /* Router Alert (as per IPv4 Router alert spec & IGMP spec)*/
							   0x00, 0x00}; /* Router Alert*/

	UCHAR               IgmpGenQuery[12] = {0x11, 0x0A, /* type(Mmbrship Query), Max Rsp Code (10 i.e 1 sec)*/
						0xEE, 0xF5, /* chksum (considered 0 for calculation and computed manually for this msg)*/
						0x00, 0x00, /* Grp addrss (general query)*/
						0x00, 0x00, /* Grp addr*/
						0x00, 0x00, /* rsv, S, QRC	QQIC (other routers to use defaults)*/
						0x00, 0x00}; /* No of Sources	(general query)*/

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
		"---->\n");

	NdisZeroMemory(Header802_3, sizeof(UCHAR)*14);

	MAKE_802_3_HEADER(Header802_3, ALL_HOST_ADDR, &pMacEntry->wdev->if_addr[0], TYPEIPV4);

	/* Using fixed payload due to checksum calculation required using one's complement*/
	NdisCopyMemory(&CustomPayload[0], IpHdr, 24);
	NdisCopyMemory(&CustomPayload[24], IgmpGenQuery, 12);

	/* Copy frame to Tx ring*/

	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 36, FALSE);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "<-----\n");
}

UCHAR	TYPEIPV6[] = {0x86, 0xDD};

/* Send a MLD query message on particular AP interface*/
void send_mldv2_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry)
{
	UCHAR ALL_HOST_ADDR[MAC_ADDR_LEN] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01}; /*as per MLD spec*/
	UCHAR Header802_3[14] = {0};
	UCHAR CustomPayload[76];
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pMacEntry->wdev->func_idx];

	UCHAR Ipv6Hdr[40] = {0x60, 0x00, /* version(4bit), Tclass(8bit), Flow label (4bit+)*/
					0x00, 0x00, /* Flow Label (16 bit)*/
					0x00, 0x24, /* Payload length	(Ipv6 Router alert + MLD Gen Query)*/
					0x00, 0x01, /*Next hdr (Hop y Hop Options for Router Alert), Hop Limit*/
									0x00, 0x00, /* ipv6 src addr	(Ipv6 link-local addr)		<-------------- to update from preformed address*/
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0xFF, 0x02, /* ipv6 dst addr	(All node Ipv6 Multcast addr, as per MLD spec)*/
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x01};

	UCHAR Ipv6RouterAlert[8] = {0x3A, 0x00, /* NxtHdr (ICMPv6-MLD), Hdr Ext len*/
									   0x05, 0x02, /* Option Type - Router Alert, Length*/
									   0x00, 0x00, /* Value - MLD*/
									   0x01, 0x00}; /* Padding - Pad2*/

	UCHAR MldGenQuery[28] = {0x82, 0x00, /* type(MLD Mmbrship Query), Code*/
						0x00, 0x00, /* chksum						<------- to update from precomputed checksum for each mbss*/
						0x03, 0xE8, /* max rsp code (1000 ms i.e. 1 second)*/
						0x00, 0x00, /* rsvd*/
						0x00, 0x00, /* ipv6 grp addr (general query)*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00, /* rsv, S, QRC	QQIC	(other routers to use defaults)*/
						0x00, 0x00}; /* No of Sources	(general query)*/

	/* Get the Link Local Src Addr for this interface*/
	NdisCopyMemory(&Ipv6Hdr[8], &pMbss->ipv6LinkLocalSrcAddr[0], 16);

	/* Get Checksum*/
	MldGenQuery[2] = (pMbss->MldQryChkSum >> 8);
	MldGenQuery[3] = (pMbss->MldQryChkSum & 0xff);

	/* Form the pkt to be sent*/
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
		"---->\n");

	NdisZeroMemory(Header802_3, sizeof(UCHAR)*14);

	MAKE_802_3_HEADER(Header802_3, ALL_HOST_ADDR, &pMacEntry->wdev->if_addr[0], TYPEIPV6);

	/* Using fixed payload due to checksum calculation required using one's complement*/
	NdisZeroMemory(CustomPayload, 76);
	NdisCopyMemory(&CustomPayload[0], Ipv6Hdr, 40);
	NdisCopyMemory(&CustomPayload[40], Ipv6RouterAlert, 8);
	NdisCopyMemory(&CustomPayload[48], MldGenQuery, 28);

	/* Copy frame to Tx ring*/

	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 76, FALSE);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, "<-----\n");
}

/* For specifed MBSS, compute & store IPv6 format checksum for MLD query message to be sent on that interface*/
void calc_mldv2_gen_query_chksum(
	IN	PRTMP_ADAPTER	pAd,
	IN  BSS_STRUCT *pMbss)
{
	UCHAR CustomPayload[68];
	UINT32 sum = 0, operand = 0, exCarry = 0;
	UINT16 chksum = 0, dataLen = 0;
	UINT8 ctr = 0;

	UCHAR ipv6LinkLocalSrcAddr[16] = {0xFE, 0x80, /*ipv6 src addr	(Ipv6 link-local addr)		<------------------to form*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0xFF,
						0xFE, 0x00,
						0x00, 0x00};

	UCHAR Ipv6PsuedoHdr[40] = { 0x00, 0x00, /* ipv6 src addr (Ipv6 link-local addr)		<------------------to form*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0xFF, 0x02, /* ipv6 dst addr (All node Ipv6 Multcast addr as per spec)*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x01,
						0x00, 0x00, /* Next Hdr (MLD) Pkt Len*/
						0x00, 0x1C, /* Next Hdr (MLD) Pkt Len*/
						0x00, 0x00, /* Zero*/
						0x00, 0x3A}; /* Zero, Next hdr (ICMPv6 - MLD)*/

	UCHAR               MldGenQuery[28] = {0x82, 0x00, /* type(MLD Mmbrship Query), Code*/
						0x00, 0x00, /* chksum		<--------- calculate*/
						0x03, 0xE8, /* max rsp code (1000 ms i.e. 1 second)*/
						0x00, 0x00, /* rsvd*/
						0x00, 0x00, /* ipv6 grp addr (general query)*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00, /* rsv, S, QRC	QQIC	(other routers to use defaults)*/
						0x00, 0x00}; /* No of Sources	(general query)*/

	/* Form the Link Local Src Addr for this interface in EUI-64 format, as per spec*/
	ipv6LinkLocalSrcAddr[8] = pMbss->wdev.if_addr[0];
	ipv6LinkLocalSrcAddr[9] = pMbss->wdev.if_addr[1];
	ipv6LinkLocalSrcAddr[10] = pMbss->wdev.if_addr[2];
	ipv6LinkLocalSrcAddr[13] = pMbss->wdev.if_addr[3];
	ipv6LinkLocalSrcAddr[14] = pMbss->wdev.if_addr[4];
	ipv6LinkLocalSrcAddr[15] = pMbss->wdev.if_addr[5];

	ipv6LinkLocalSrcAddr[8] = ipv6LinkLocalSrcAddr[8] ^ 0x02; /* togle universal/local bit*/

	NdisCopyMemory(&pMbss->ipv6LinkLocalSrcAddr[0], &ipv6LinkLocalSrcAddr[0], 16);
	NdisCopyMemory(&Ipv6PsuedoHdr[0], &ipv6LinkLocalSrcAddr[0], 16);

	/* Rakesh: A print is required here to avoid crash during reboot. Please don't remove !!!*/
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
		"-->");

	/* Calculate Checksum*/
	NdisZeroMemory(CustomPayload, 68);
	NdisCopyMemory(&CustomPayload[0], Ipv6PsuedoHdr, 40);
	NdisCopyMemory(&CustomPayload[40], MldGenQuery, 28);
	dataLen = 68; /* total size of Pseudo Hdr & MLD*/

	/* Note: current logic assumes even data len, as per IP checksum format*/
	sum = 0;
	operand = 0;
	for (ctr = 0; ctr < dataLen; ctr += 2) {
		operand = (UINT32)(CustomPayload[ctr] << 8);
		operand |= (UINT32)(CustomPayload[ctr+1]);

		sum += operand;
	}
	exCarry = sum >> 16;
	sum = sum & 0x0000ffff;

	while (exCarry != 0) {
		sum += exCarry;
		exCarry = sum >> 16;
		sum = sum & 0x0000ffff;
	}
	chksum = (UINT16)sum;
	chksum = chksum ^ 0xFFFF;
	if (chksum == 0)
		chksum = 0xFFFF;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
		"ChkSum Done: chksum: %04x\n", chksum);

	NdisCopyMemory(&pMbss->MldQryChkSum, &chksum, 2);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
		"<-----\n");
}
#endif


#endif /* IGMP_SNOOP_SUPPORT */
