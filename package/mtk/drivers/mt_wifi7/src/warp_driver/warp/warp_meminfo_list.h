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

	Module Name:
    meminfo_list.h
*/
#ifndef __MEMINFO_LIST_H__
#define __MEMINFO_LIST_H__

#include <linux/spinlock.h>
#include <linux/gfp.h>
#include <linux/skbuff.h>

#include "warp_link_list.h"
#include "warp_utility.h"

#define POOL_ENTRY_NUMBER 2000
#define MEM_HASH_SIZE	1024

#define WarpFreeSpinLock(lock)				\
	do {} while (0)

#define RTPKT_TO_OSPKT(_p)		((struct sk_buff *)(_p))

#define GET_OS_PKT_DATAPTR(_pkt) \
		(RTPKT_TO_OSPKT(_pkt)->data)

typedef struct _MEM_INFO_LIST_ENTRY {
		DL_LIST mList;
		DL_LIST cList;
		u32 MemSize;
		u32 cnt;
		void *pMemAddr;
		void *pCaller;
} MEM_INFO_LIST_ENTRY;
/*
typedef struct _CALLER_INFO_LIST_ENTRY {
	DL_LIST mList;
	int cnt;
	unsigned int size;
	void *pCaller;
} CALLER_INFO_LIST_ENTRY;
*/
typedef struct _MEM_INFO_LIST_POOL {
	MEM_INFO_LIST_ENTRY Entry[POOL_ENTRY_NUMBER];
	DL_LIST List;
} MEM_INFO_LIST_POOL;

typedef struct _FREE_LIST_POOL {
	MEM_INFO_LIST_ENTRY head;
	MEM_INFO_LIST_POOL Poolhead;
	u32 EntryNumber;
	spinlock_t Lock;
	u32 MLlistCnt;
} FREE_LIST_POOL;

typedef struct _MEM_INFO_LIST {
	MEM_INFO_LIST_ENTRY pHead[MEM_HASH_SIZE];
	MEM_INFO_LIST_ENTRY pHeadCal[MEM_HASH_SIZE];
	spinlock_t Lock;
	u32 EntryNumber;
	u32 CurAlcSize;
	u32 MaxAlcSize;
	FREE_LIST_POOL *pFreeEntrylist;
} MEM_INFO_LIST;

enum {
	SHOW_PCALLER_INFO,
	SHOW_ALL_PKT_INFO,
	SHOW_PCALLER_PKT_INFO,
	DUMP_PCALLER_PKT,
	RELEASE_PCALLER_PKT,
	SHOW_PKT_INFO,
	MAX_TYPE,
};

enum {
	MEMINFO,
	PKTMEMINFO,
};

#define MAX_POS_IDX 14

static FREE_LIST_POOL FreeEntrylist;

static inline MEM_INFO_LIST_ENTRY *GetEntryFromFreeList(MEM_INFO_LIST *MIList)
{
	MEM_INFO_LIST_ENTRY *pEntry = 0;
	MEM_INFO_LIST_ENTRY *pheadEntry = 0;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;
	unsigned long IrqFlags = 0;
	u32 i;

	spin_lock_irqsave((spinlock_t *)&pFreeEntrylist->Lock, IrqFlags);

	if (DlListEmpty(&pFreeEntrylist->head.mList)) {
		MEM_INFO_LIST_POOL *Pool = 0;
		MEM_INFO_LIST_POOL *pFreepool = 0;

		Pool = kmalloc(sizeof(MEM_INFO_LIST_POOL), GFP_ATOMIC);
		if (Pool == NULL) {
			warp_dbg(WARP_DBG_ERR, "%s: alloc fail, please check memory\n", __func__);
		} else {
			pFreepool = &pFreeEntrylist->Poolhead;
			DlListAdd(&pFreepool->List, &Pool->List);
			pheadEntry = &pFreeEntrylist->head;
		}
		for (i = 0; i < POOL_ENTRY_NUMBER; i++) {
			pEntry = &Pool->Entry[i];
			DlListAdd(&pheadEntry->mList, &pEntry->mList);
		}

		pFreeEntrylist->EntryNumber += POOL_ENTRY_NUMBER;
	}

	pheadEntry = &pFreeEntrylist->head;
	pEntry = DlListFirst(&pheadEntry->mList, MEM_INFO_LIST_ENTRY, mList);
	if (pEntry != NULL)
		DlListDel(&pEntry->mList);

	if (pEntry != 0)
		pFreeEntrylist->EntryNumber -= 1;

	spin_unlock_irqrestore((spinlock_t *)&pFreeEntrylist->Lock, (unsigned long)IrqFlags);
	return pEntry;
}

static inline u32 HashF(void *pMemAddr)
{
	signed long addr = (signed long)pMemAddr;
	u32 a = addr & 0xF;
	u32 b = (addr & 0xF0) >> 4;
	u32 c = (addr & 0xF00) >> 8;
	u32 d = (addr & 0xF000) >> 12;
	u32 e = (addr & 0xF0000) >> 16;
	u32 f = (addr & 0xF00000) >> 20;
	u32 g = (addr & 0xF000000) >> 24;
	u32 h = (addr & 0xF0000000) >> 28;

	return (a + b * 3 + c * 5 + d * 7 + e * 11 + f * 13 + g * 17 + h * 19) % MEM_HASH_SIZE;
}

static inline void PoolInit(void)
{
	if (FreeEntrylist.MLlistCnt == 0) {
		spin_lock_init((spinlock_t *)&FreeEntrylist.Lock);
		DlListInit(&FreeEntrylist.Poolhead.List);
		DlListInit(&FreeEntrylist.head.mList);
		DlListInit(&FreeEntrylist.head.cList);
	}

	FreeEntrylist.MLlistCnt++;
}

static inline void PoolUnInit(void)
{
	FreeEntrylist.MLlistCnt--;

	if (FreeEntrylist.MLlistCnt == 0) {
		MEM_INFO_LIST_POOL *pEntry = 0;

		while (!DlListEmpty(&FreeEntrylist.Poolhead.List)) {
			pEntry = DlListFirst(&FreeEntrylist.Poolhead.List, MEM_INFO_LIST_POOL, List);
			DlListDel(&pEntry->List);
			kfree(pEntry);
		}
	}
}

static inline void MIListInit(MEM_INFO_LIST *MIList)
{
	u32 i;

	for (i = 0; i < MEM_HASH_SIZE; i++) {
		DlListInit(&MIList->pHead[i].mList);
		DlListInit(&MIList->pHeadCal[i].cList);
		MIList->pHeadCal[i].cnt = 0;
	}
	spin_lock_init((spinlock_t *)&MIList->Lock);
	MIList->EntryNumber = 0;
	MIList->CurAlcSize = 0;
	MIList->MaxAlcSize = 0;
	PoolInit();

	if (DlListEmpty(&FreeEntrylist.Poolhead.List)) {
		MEM_INFO_LIST_POOL *Pool = 0;
		MEM_INFO_LIST_POOL *pFreepool = 0;
		MEM_INFO_LIST_ENTRY *pEntry = 0;
		MEM_INFO_LIST_ENTRY *newEntry = 0;

		Pool = kmalloc(sizeof(MEM_INFO_LIST_POOL), 0);
		if (Pool == NULL) {
			warp_dbg(WARP_DBG_ERR, "%s: alloc fail, please check memory\n", __func__);
		} else {
			pFreepool = &FreeEntrylist.Poolhead;
			DlListAdd(&pFreepool->List, &Pool->List);
			pEntry = &FreeEntrylist.head;
		}
		for (i = 0; i < POOL_ENTRY_NUMBER; i++) {
			newEntry = &Pool->Entry[i];
			DlListAdd(&pEntry->mList, &newEntry->mList);
		}

		FreeEntrylist.EntryNumber += POOL_ENTRY_NUMBER;
	}

	MIList->pFreeEntrylist = &FreeEntrylist;
}
static inline void MIListExit(MEM_INFO_LIST *MIList)
{
	u32 i = 0;
	unsigned long IrqFlags = 0;

	spin_lock_irqsave((spinlock_t *)&MIList->Lock, IrqFlags);

	for (i = 0; i < MEM_HASH_SIZE; i++)
		DlListInit(&MIList->pHead[i].mList);

	spin_unlock_irqrestore((spinlock_t *)&MIList->Lock, (unsigned long)IrqFlags);
	WarpFreeSpinLock(&MIList->Lock);
	MIList->EntryNumber = 0;
	PoolUnInit();
}

static inline void MIListAddHead(MEM_INFO_LIST *MIList, u32 Size, void *pMemAddr, void *pCaller)
{
	u32 Index = HashF(pMemAddr);
	u32 CalIndex = HashF(pCaller);
	unsigned long IrqFlags = 0;
	MEM_INFO_LIST_ENTRY *pEntry = 0;
	MEM_INFO_LIST_ENTRY *pCalEntry = 0;
	MEM_INFO_LIST_ENTRY *pheadEntry = 0;
	MEM_INFO_LIST_ENTRY *pheadCalEntry = 0;
	u32 bFind = 0;

	spin_lock_irqsave((spinlock_t *)&MIList->Lock, IrqFlags);
	pEntry = GetEntryFromFreeList(MIList);

	if (!pEntry) {
		spin_unlock_irqrestore((spinlock_t *)&MIList->Lock, (unsigned long)IrqFlags);
		warp_dbg(WARP_DBG_ERR, "%s: pEntry is null!!!\n", __func__);
		return;
	}

	pEntry->MemSize = Size;
	pEntry->pMemAddr = pMemAddr;
	pEntry->pCaller = pCaller;
	pheadEntry = &MIList->pHead[Index];
	pheadCalEntry = &MIList->pHeadCal[CalIndex];
	DlListAdd(&pheadEntry->mList, &pEntry->mList);
	MIList->EntryNumber++;
	MIList->CurAlcSize += pEntry->MemSize;
	pEntry->cnt = 0;

	DlListInit(&pEntry->cList);
	DlListForEach(pCalEntry, &pheadCalEntry->cList, MEM_INFO_LIST_ENTRY, cList) {
		if (pCalEntry->pCaller == pCaller) {
			pCalEntry->cnt++;
			pCalEntry->MemSize += Size;
			bFind = 1;
			DlListAdd(&pCalEntry->cList, &pEntry->cList);
			break;
		}
	}

	if (!bFind) {
		pEntry->cnt = 1;
		DlListAdd(&pheadCalEntry->cList, &pEntry->cList);
	}

	if (MIList->CurAlcSize > MIList->MaxAlcSize)
		MIList->MaxAlcSize = MIList->CurAlcSize;

	spin_unlock_irqrestore((spinlock_t *)&MIList->Lock, (unsigned long)IrqFlags);

}

static inline MEM_INFO_LIST_ENTRY *MIListRemove(MEM_INFO_LIST *MIList, void *pMemAddr)
{
	u32 Index = HashF(pMemAddr);
	unsigned long IrqFlags = 0;
	unsigned long IrqFlags2 = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	MEM_INFO_LIST_ENTRY *pTmpEntry = NULL;
	MEM_INFO_LIST_ENTRY *pheadEntry = NULL;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;
	int find = 0;

	spin_lock_irqsave((spinlock_t *)&MIList->Lock, IrqFlags);
	DlListForEachSafe(pEntry, pTmpEntry, &MIList->pHead[Index].mList,
						MEM_INFO_LIST_ENTRY, mList) {
		if (pEntry->pMemAddr == pMemAddr) {
			u32 CalIndex = HashF(pEntry->pCaller);
			MEM_INFO_LIST_ENTRY *pheadCalEntry = 0;
			MEM_INFO_LIST_ENTRY *pCalEntry = 0;

			find = 1;
			DlListDel(&pEntry->mList);
			MIList->EntryNumber--;

			spin_lock_irqsave((spinlock_t *)&pFreeEntrylist->Lock, IrqFlags2);
			pheadCalEntry = &MIList->pHeadCal[CalIndex];

			DlListForEach(pCalEntry, &pheadCalEntry->cList,
				MEM_INFO_LIST_ENTRY, cList) {
				if (pCalEntry->pCaller == pEntry->pCaller) {
					MEM_INFO_LIST_ENTRY *pCalEntryNext = 0;

					pCalEntryNext = DlListFirst(&pCalEntry->cList,
							MEM_INFO_LIST_ENTRY, cList);
					if (pCalEntry == pEntry &&
							pEntry->cnt > 1 &&
							pCalEntryNext) {
						MIList->CurAlcSize -= pCalEntryNext->MemSize;
						pCalEntryNext->cnt = pCalEntry->cnt - 1;
						pCalEntryNext->MemSize =
							pEntry->MemSize - pCalEntryNext->MemSize;
					} else {
						MIList->CurAlcSize -= pEntry->MemSize;
						pCalEntry->cnt--;
						pCalEntry->MemSize -= pEntry->MemSize;
					}
					DlListDel(&pEntry->cList);
					break;
				}
			}
			pheadEntry = &pFreeEntrylist->head;
			DlListAddTail(&pheadEntry->mList, &pEntry->mList);
			pFreeEntrylist->EntryNumber += 1;
			spin_unlock_irqrestore((spinlock_t *)&pFreeEntrylist->Lock, (unsigned long)IrqFlags2);

			break;
		}
	}
	spin_unlock_irqrestore((spinlock_t *)&MIList->Lock, (unsigned long)IrqFlags);

	if (find)
		return pEntry;
	else
		return NULL;

}
static inline void ShowMIList(MEM_INFO_LIST *MIList)
{
	u32 i = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;

	for (i = 0; i < MEM_HASH_SIZE; i++) {

		DlListForEach(pEntry, &MIList->pHeadCal[i].cList, MEM_INFO_LIST_ENTRY, cList)
			if (pEntry->cnt)
				warp_dbg(WARP_DBG_OFF, "cnt=%u  \t,size=%u   \t\t:%pS\n",
						pEntry->cnt, pEntry->MemSize, pEntry->pCaller);
	}

	warp_dbg(WARP_DBG_OFF, "the number of allocated memory = %u\n", MIList->EntryNumber);
	warp_dbg(WARP_DBG_OFF, "the number of free pool entry = %u\n", MIList->pFreeEntrylist->EntryNumber);
}
void MemInfoListInital(void);
u32 ShowMemAllocInfo(void);
u32 ShowPktAllocInfo(void);
void MemInfoListExit(void);
int warp_os_alloc_mem(unsigned char **mem, unsigned long size, u32 gfp);
void pkt_alloc_mem(unsigned long size, unsigned char *mem, void *pCaller);
void warp_os_free_mem(void *mem);


#endif

