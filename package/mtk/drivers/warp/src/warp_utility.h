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
	warp_utility.h
*/

#ifndef _WARP_UTILITY_H_
#define _WARP_UTILITY_H_

#include <linux/list.h>
#include <linux/file.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/idr.h>
#include <warp_cfg.h>
#ifdef CONFIG_MULDF_HELPER
#include "warp_muldf.h"
#endif /* CONFIG_MULDF_HELPER */

#define MAX_NAME_SIZE 64

#define DEBUGFS_MAX_BUF_SIZE 2048
#define DEBUGFS_MIN_BUF_SIZE 1024

/*warp status code*/

#define WARP_INVALID_LENGTH_STATUS (-2)
#define WARP_NULL_POINTER_STATUS (-3)
#define WARP_INVALID_PARA_STATUS (-4)
#define WARP_NOT_HANDLE_STATUS (-5)
#define WARP_FAIL_STATUS (-1)
#define WARP_OK_STATUS (0)
#define WARP_ALREADY_DONE_STATUS (1)

enum {
	WARP_DBG_START,
	WARP_DBG_OFF = WARP_DBG_START,
	WARP_DBG_ERR,
	WARP_DBG_WAR,
	WARP_DBG_INF,
	WARP_DBG_LOU,
	WARP_DBG_ALL,
	WARP_DBG_END
};

/*platform register Read/Write*/
#define warp_io_write32(_A, _R, _V)								\
	writel(_V, (void *)((_A)->base_addr + (_R)))

#define warp_io_read32(_A, _R, _pV)								\
	(*(_pV) = readl((void *)((_A)->base_addr + (_R))))

#ifdef WED_RX_HW_RRO_2_0
#define fwdl_io_write32(_A, _R, _V)								\
	writel(_V, (void *)((_A)->boot_setting.base_addr + (_R)))

#define fwdl_io_read32(_A, _R, _pV)								\
	(*(_pV) = readl((void *)((_A)->boot_setting.base_addr + (_R))))
#endif


/*debug macro*/
#ifdef CONFIG_WARP_DBG_SUPPORT
#ifdef CONFIG_MULDF_HELPER
#define warp_dbg(lev, fmt, args...) \
	do {	\
		if (lev <= warp_log_get()) {	\
			pr_info(fmt, ## args);	\
			muldf_print(MTK_MSG_INFO, MULDF_MODULE_NAME_WARP, MULDF_CAT_MTWF_WARP,\
				NULL, NULL, NULL, 0, fmt, ## args);	\
		}	\
	} while (0)
#else /* CONFIG_MULDF_HELPER */
#define warp_dbg(lev, fmt, args...) \
	do {	if (lev <= warp_log_get())	pr_info(fmt, ## args); } while(0)
#endif /* CONFIG_MULDF_HELPER */
#else
#define warp_dbg(lev, fmt, args...)
#endif

#define WHNAT_RESET(_entry, _addr, _value) \
do {\
	u32 cnt = 0;\
	warp_io_write32(_entry, _addr, _value);\
	while (_value != 0 && cnt < WED_POLL_MAX) {\
		warp_io_read32(_entry, _addr, &_value);\
		cnt++;\
	} \
	if (cnt >= WED_POLL_MAX) {\
		warp_dbg(WARP_DBG_ERR,\
			"%s(): Reset addr = 0x%x, value = 0x%x, cnt = %d fail!\n",\
			__func__, _addr, _value, cnt);\
	} \
} while (0)

#define dump_addr_value(_entry, _name, _addr)\
do {\
	u32 value;\
	warp_io_read32(_entry, _addr, &value);\
	warp_dbg(WARP_DBG_OFF, "%s\t: 0x%x\n", _name, value);\
} while (0)

#define dump_io(_output, _entry, _name, _addr)\
do {\
	u32 value;\
	warp_io_read32(_entry, _addr, &value);\
	if (_output != NULL)\
		seq_printf(_output, "%s\t: 0x%x\n", _name, value);\
} while (0)

#define dump_io_diff(_output, _entry, _name, _addr, _addr2)\
do {\
	u32 value;\
	u32 value2;\
	warp_io_read32(_entry, _addr, &value);\
	warp_io_read32(_entry, _addr2, &value2);\
	if (_output != NULL)\
		seq_printf(_output, "%s\t: 0x%x\n", _name, value - value2);\
} while (0)

#define dump_string(_output, fmt, args...)\
do {\
	if (_output != NULL)\
		seq_printf(_output, fmt, ## args);\
} while (0)

struct warp_dma_buf {
	unsigned long alloc_size;
	void *alloc_va;
	struct list_head list;
	dma_addr_t alloc_pa;
};

struct warp_dma_cb {
	unsigned long alloc_size;
	void *alloc_va;
	dma_addr_t alloc_pa;
	struct sk_buff *pkt;
	void *pkt_va;
	dma_addr_t pkt_pa;
	unsigned int pkt_size;
	struct warp_dma_cb *next;
	struct list_head list;
};

#define WED_TX_RING_SIZE 4096
struct warp_ring {
	struct warp_dma_cb cell[WED_TX_RING_SIZE];
	struct warp_dma_buf desc;
	u32 hw_desc_base;
	u32 hw_cidx_addr;
	u32 hw_didx_addr;
	u32 hw_cnt_addr;
	u32 ring_lens;
};

/* TODO: merge warp_ring and warp_tx_ring */
struct warp_tx_ring {
	bool recycled;
	struct warp_dma_cb *cell;
	u32 hw_desc_base;
	u32 hw_cidx_addr;
	u32 hw_didx_addr;
	u32 hw_cnt_addr;
	u32 ring_lens;
	struct warp_dma_cb *head;
};

#define WED_BUS_RING_SIZE 256
struct warp_bus_ring {
	u32 hw_desc_base;
	u32 hw_cidx_addr;
	u32 hw_didx_addr;
	u32 hw_cnt_addr;
	u32 ring_lens;
	struct warp_dma_cb cell[WED_BUS_RING_SIZE];
	struct warp_dma_buf desc;
};


struct warp_rx_ring {
	bool recycled;
	bool spare;
	struct warp_dma_cb *cell;
	void *hw_desc_base_va;
	u32 hw_desc_base;
	u32 hw_cidx_addr;
	u32 hw_didx_addr;
	u32 hw_cnt_addr;
	u32 ring_lens;
	dma_addr_t hw_desc_base_pa;
	dma_addr_t cb_alloc_pa;
	struct warp_dma_cb *head;
};

struct profiling_statistic_t {
	u32 bound;
	u32 record;
};

char warp_log_get(void);
void warp_log_set(char log);

void warp_dump_dmabuf(struct warp_dma_buf *buf);
void warp_dump_dmacb(struct warp_dma_cb *cb);
void warp_dump_mem(void);
int warp_dma_buf_alloc(struct platform_device *dev,
		       struct warp_dma_buf *dma_buf, u32 size);
void warp_dma_buf_free(struct platform_device *dev,
		       struct warp_dma_buf *dma_buf);

void warp_dump_raw(char *str, u8 *va, u32 size);
long warp_str_tol(const char *str, char **endptr, int base);
struct sk_buff *alloc_dma_tx_pkt(struct platform_device *pdev, u32 len, void **vaddr, dma_addr_t *paddr);
int free_dma_tx_pkt(struct platform_device *pdev, struct sk_buff *pkt, dma_addr_t pa, u32 len);
bool wifi_dbdc_support(void *warp);

int warp_os_alloc_mem(
	unsigned char **mem,
	unsigned long size,
	u32 gfp);
void warp_os_free_mem(void *mem);
void pkt_alloc_mem(unsigned long size, unsigned char *mem, void *pCaller);
void pkt_free_mem(void *mem);



#endif /*_WARP_UTILITY_H_*/
