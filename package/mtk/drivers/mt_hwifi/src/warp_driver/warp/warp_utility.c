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

	Module Name: wifi_offload
	warp_basic.c
*/


#include "warp_utility.h"
#include "warp_meminfo_list.h"
#include "warp.h"


static char warp_log_level = WARP_DBG_ERR;
MEM_INFO_LIST MemInfoList;
MEM_INFO_LIST PktInfoList;

/*
*
*/
char
warp_log_get(void)
{
	return warp_log_level;
}

/*
*
*/
void
warp_log_set(char log)
{
	warp_log_level = log;
}

/*
*
*/
void
warp_dma_buf_free(struct platform_device *dev, struct warp_dma_buf *dma_buf)
{
	if (!dev || !dma_buf->alloc_va)
		return;
#ifdef WED_MEM_LEAK_DBG
{
		MEM_INFO_LIST_ENTRY *delEntry;

		delEntry = MIListRemove(&MemInfoList, dma_buf->alloc_va);
		if (delEntry == 0) {
			warp_dbg(WARP_DBG_ERR, "the memory has not been allocated\n");
			warp_dbg(WARP_DBG_ERR, "mem addr = %p, caller is %pS\n",
					dma_buf->alloc_va, __builtin_return_address(0));
		}
}
#endif	/* WED_MEM_LEAK_DBG */

	memset(dma_buf->alloc_va, 0, dma_buf->alloc_size);
	dma_free_coherent(&dev->dev, dma_buf->alloc_size, dma_buf->alloc_va,
			  dma_buf->alloc_pa);
	memset(dma_buf, 0, sizeof(struct warp_dma_buf));
}

/*
*
*/
int
warp_dma_buf_alloc(struct platform_device *dev,
		   struct warp_dma_buf *dma_buf, u32 size)
{
	dma_addr_t paddr = {0};
	void *vaddr = NULL;

	dma_buf->alloc_size = size;
	vaddr = dma_alloc_coherent(&dev->dev, size, &paddr, GFP_KERNEL);

	if (vaddr == NULL) {
		warp_dbg(WARP_DBG_ERR, "Failed to allocate a big buffer,size=%d\n", size);
		return -1;
	}
#ifdef WED_MEM_LEAK_DBG
		MIListAddHead(&MemInfoList, size, vaddr, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */


	dma_buf->alloc_pa = paddr;
	dma_buf->alloc_va = vaddr;
	/*zero init this memory block*/
	memset(dma_buf->alloc_va, 0, size);
	return 0;
}

/*
*
*/
void
warp_dump_dmabuf(struct warp_dma_buf *buf)
{
	warp_dbg(WARP_DBG_OFF, "AllocPA\t: %pad\n", &buf->alloc_pa);
	warp_dbg(WARP_DBG_OFF, "AllocVa\t: 0x%p\n", buf->alloc_va);
	warp_dbg(WARP_DBG_OFF, "Size\t: %lu\n", buf->alloc_size);

}

void
warp_dump_mem()
{
	u32 memalctotal = ShowMemAllocInfo();
	u32 pktalctotal = ShowPktAllocInfo();

	warp_dbg(WARP_DBG_OFF, "memalctotal:%d pktalctotal:%d\n", memalctotal, pktalctotal);
}

/*
*
*/
void
warp_dump_dmacb(struct warp_dma_cb *cb)
{
	warp_dbg(WARP_DBG_OFF, "AllocPA\t: %pad\n", &cb->alloc_pa);
	warp_dbg(WARP_DBG_OFF, "AllocVa\t: %p\n", cb->alloc_va);
	warp_dbg(WARP_DBG_OFF, "Size\t: %lu\n", cb->alloc_size);
	warp_dbg(WARP_DBG_OFF, "Pkt\t: %p\n", cb->pkt);
}

/*
*
*/
void
warp_dump_raw(char *str, unsigned char *va, unsigned int size)
{
	int ret = 0;
	u8 *pt;
	char buf[512] = "";
	u32 len = 0;
	int x;

	pt = va;
	warp_dbg(WARP_DBG_OFF, "%s: %p, len = %d\n", str, va, size);

	for (x = 0; x < size; x++) {
		if (x % 16 == 0) {
			ret = snprintf(buf + len, sizeof(buf) - len, "\n0x%04x : ", x);
			if (ret < 0)
				return;
			len = strlen(buf);
		}

		ret = snprintf(buf + len, sizeof(buf) - len, "%02x ", ((unsigned char)pt[x]));
		if (ret < 0)
			return;
		len = strlen(buf);
	}

	warp_dbg(WARP_DBG_OFF, "%s\n", buf);
}

long
warp_str_tol(const char *str, char **endptr, int base)
{
	return simple_strtol(str, endptr, base);
}

/*
*
*/
struct sk_buff *
alloc_dma_tx_pkt(
	struct platform_device *pdev,
	u32 len,
	void **vaddr,
	dma_addr_t *paddr)
{
	struct sk_buff *pkt;
#ifdef BB_SOC
	pkt = skbmgr_dev_alloc_skb4k();
#else
	pkt = dev_alloc_skb(len);
#endif

	if (pkt == NULL)
		warp_dbg(WARP_DBG_ERR, "%s(): can't allocate tx %d size packet\n", __func__,
			 len);

	if (pkt) {
		*vaddr = (void *) pkt->data;
		*paddr = dma_map_single(&pdev->dev, *vaddr, len, DMA_TO_DEVICE);
	} else {
		*vaddr = (void *) NULL;
		*paddr = (dma_addr_t) 0;
	}

	return pkt;
}

/*
*
*/
int
free_dma_tx_pkt(
	struct platform_device *pdev,
	struct sk_buff *pkt,
	dma_addr_t pa,
	u32 len)
{
	if (pkt == NULL)
		return -1;

	dma_unmap_single(&pdev->dev, pa, len, DMA_TO_DEVICE);
	dev_kfree_skb_any(pkt);
	return 0;
}

/*
 *
 */
bool
wifi_dbdc_support(void *warp)
{
	bool enable = false;

	if (warp) {
		struct warp_entry *entry = (struct warp_entry *)warp;

		enable = entry->wifi.hw.dbdc_mode;
	}

	return enable;
}

void MemInfoListInital(void)
{
	MIListInit(&MemInfoList);
	MIListInit(&PktInfoList);
}

u32 ShowMemAllocInfo(void)
{
#ifdef WED_MEM_LEAK_DBG
	warp_dbg(WARP_DBG_OFF,
			 "Show General Allocated Memory Info:\n");
	ShowMIList(&MemInfoList);
	warp_dbg(WARP_DBG_OFF,
			 "Maximum Allocated Memory Size: %u\n", MemInfoList.MaxAlcSize);
	warp_dbg(WARP_DBG_OFF,
			 "Current Allocated Memory Size: %u\n", MemInfoList.CurAlcSize);
#endif	/* WED_MEM_LEAK_DBG */
	return MemInfoList.EntryNumber;
}

u32 ShowPktAllocInfo(void)
{
#ifdef WED_MEM_LEAK_DBG
	warp_dbg(WARP_DBG_OFF,
			 "Show Packet Allocated Memory Info:\n");
	ShowMIList(&PktInfoList);
#endif	/* WED_MEM_LEAK_DBG */
	return PktInfoList.EntryNumber;
}

void MemInfoListExit(void)
{
	u32 memalctotal, pktalctotal;

	memalctotal = ShowMemAllocInfo();
	pktalctotal = ShowPktAllocInfo();

	if ((memalctotal != 0) || (pktalctotal != 0)) {
		warp_dbg(WARP_DBG_OFF,
				 "Error: Memory leak!!\n");
	}

	MIListExit(&MemInfoList);
	MIListExit(&PktInfoList);
}

int warp_os_alloc_mem(
	unsigned char **mem,
	unsigned long size,
	u32 gfp)
{
	*mem = kmalloc(size, gfp);
	if (*mem) {
#ifdef WED_MEM_LEAK_DBG
		MIListAddHead(&MemInfoList, size, *mem, __builtin_return_address(0));
#endif	/* WED_MEM_LEAK_DBG */
		return 0;
	} else
		return 1;
}


void pkt_alloc_mem(
	unsigned long size, unsigned char *mem,
	void *pCaller)
{
	MIListAddHead(&MemInfoList, size, mem, pCaller);
}

void warp_os_free_mem(void *mem)
{
#ifdef WED_MEM_LEAK_DBG
	MEM_INFO_LIST_ENTRY *delEntry;
	delEntry = MIListRemove(&MemInfoList, mem);
	if (delEntry == 0) {
		warp_dbg(WARP_DBG_ERR, "the memory has not been allocated\n");
		warp_dbg(WARP_DBG_ERR, "mem addr = %p, caller is %pS\n", mem, __builtin_return_address(0));
	}
#endif	/* WED_MEM_LEAK_DBG */
	//assert(mem);
	kfree(mem);
}

void pkt_free_mem(void *mem)
{
	MIListRemove(&MemInfoList, mem);
}

