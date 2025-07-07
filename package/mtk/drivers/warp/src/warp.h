/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	warp.h
*/

#ifndef _WARP_H
#define _WARP_H

#include "wdma.h"
#include "wed.h"
#include "warp_wifi.h"
#include "warp_bus.h"
#include "warp_utility.h"
#include "mcu/warp_wo.h"
#include "mcu/warp_woctrl.h"
#include "mcu/warp_cmd.h"
#include "mcu/warp_msg.h"
#include "mcu/warp_fwdl.h"
#ifdef CONFIG_WARP_CCIF_SUPPORT
#include "mcu/warp_ccif.h"
#endif

#define WARP_VERSION_CODE		0x0300
#define WARP_VERSION(VER, SUB_VER)	(((VER) << 8) + (SUB_VER))

struct warp_cfg {
	char hw_tx_en; /*hw tx wed enable*/
};

struct warp_wifi_cr_map {
	u32 wifi_cr;
	u32 warp_cr;
};

#define MAX_MAP_TABLE_SIZE 32
struct warp_entry {
	u8 idx;
	u8 slot_id;
	struct wifi_entry wifi;
	struct platform_driver pdriver;
	struct wdma_entry wdma;
	struct wed_entry wed;
	struct platform_device *pdev;
	struct warp_cfg cfg;
	struct warp_wifi_cr_map mtbl[MAX_MAP_TABLE_SIZE];
	struct warp_msg_ctrl msg_ctrl[MAX_MODULE_ID];
	struct woif_entry woif;
	void *proc;
	void *proc_stat;
	void *proc_cr;
	void *proc_cfg;
	void *proc_tx;
	void *proc_rx;
	void *proc_ctrl;
	void *proc_wo;
	void *proc_wo_ee_dump;
	void *proc_amsdu_fifo;
	void *proc_amsdu_eng0;
	void *proc_amsdu_eng1;
	void *proc_amsdu_eng2;
	void *proc_amsdu_eng3;
	void *proc_amsdu_eng4;
	void *proc_amsdu_eng5;
	void *proc_amsdu_eng6;
	void *proc_amsdu_eng7;
	void *proc_amsdu_eng8;
	void *proc_qmem;
	void *proc_qmem_head;
	void *proc_qmem_tail;
	void *proc_qmem_pre;
	void *proc_hiftxd;
	void *proc_rtqm_igrs0;
	void *proc_rtqm_igrs1;
	void *proc_rtqm_igrs2;
	void *proc_rtqm_igrs3;
	void *proc_rtqm_enq;
	void *proc_rtqm_deq;
	void *proc_rro;
};

struct warp_ctrl {
	u32 warp_num;
	struct sw_conf_t *sw_conf;
	struct warp_entry *entry;
	struct warp_bus bus;
	u32 warp_driver_idx;
	void *proc;
	void *proc_trace;
	void *proc_sw_conf;
	u8 warp_ref;
	spinlock_t warp_lock;
};

#define to_warp_entry(_wifi) container_of(_wifi, struct warp_entry, wifi)

enum {
	WARP_DMA_DISABLE,
	WARP_DMA_TX,
	WARP_DMA_RX,
	WARP_DMA_TXRX,
	WARP_DMA_TX_DISABLE,
	WARP_DMA_RX_DISABLE,
};

enum {
	WARP_WED1 = 0,
	WARP_WED2 = 1,
	WARP_WED3 = 2,
	WARP_WED_NUM
};

enum {
	WARP_DYBM_EN,
	WARP_TX_WDMA_RING_DEP,
	WARP_RX_WDMA_RING_DEP,
	WARP_DYBM_BUDGE,
	WARP_DYBM_BUDGE_LMT,
	WARP_DYBM_BUDGE_REFILL_MARK,
	WARP_DYBM_L_THRD,
	WARP_DYBM_H_THRD,
	WARP_DYBM_VLD_GRP,
	WARP_DYBM_MAX_GRP,
	WARP_DYBM_RECYCLE_POSTPONED,
	WARP_SW_CONF_MAX,
};

struct warp_entry *
warp_entry_get_by_idx(u8 wed_idx);

u8
warp_get_ref(void);

/*export functions for wifi client to call when events happened.*/

void *
warp_page_hash_get(void *priv_data, dma_addr_t pa);

/*
* wifi client call when tx packet is happening for add this tx session to hw path
*/
int
warp_wlan_tx(void *priv_data, u8 *tx_info);

void
warp_wlan_rx(void *priv_data, u8 *rx_info);

void
warp_512_support_handler(void *priv_data, u8 *enable);

void
warp_set_pcie_after_swap_irq(void *priv_data, u8 *enable);

/*
*
*/
u8
warp_get_wed_idx(void *priv_data);

/*
*
*/
int
warp_proxy_read(void *priv_data, u32 addr, u32 *value);

/*
*
*/
int
warp_proxy_write(void *priv_data, u32 addr, u32 value);

/*
*
*/
struct wifi_hw *
warp_alloc_client(u32 chip_id, u8 bus_type, u8 wed_id, u32 slot_id, void *dev);

/*
*
*/
int
warp_register_client(struct wifi_hw *hw, struct wifi_ops *ops);

/*
*
*/
void
warp_client_remove(void *priv_data);

/*
*
*/
int
warp_ring_init(void *priv_data);

/*
*
*/
void
warp_ring_exit(void *priv_data);

/*
*
*/
void
warp_ser_handler(void *priv_data, u32 ser_status);

/*
* for warp 3.0
*/
int
warp_reset_handler(void *priv_data, u32 ser_status);

/*
*
*/
void
warp_dma_handler(void *priv_data, int dma_ctrl);

/*
 *
 */
void
warp_dma_handler_after_fwdl(void *priv_data, int dma_ctrl);

/*
*
*/
void
warp_isr_handler(void *priv_data);

/*
*
*/
void
warp_suspend_handler(void *priv_data);

/*
*
*/
void
warp_resume_handler(void *priv_data);

/*
*
*/
void
warp_hb_chk_handler(void *priv_data, u8 *stop);

/*
*
*/
void
wed_wo_ops_register(struct wo_ops *ops);


int
warp_rro_write_dma_idx(void *priv_data, u32 val);

int
warp_set_pao_sta_info(void *priv_data, u16 wcid,
	u8 max_amsdu_nums, u32 max_amsdu_len, int remove_vlan, int hdrt_mode);

int
warp_set_pn_check(void *priv_data, u32 se_id, bool enble_wed_pn_chk);

int
warp_msg_send_cmd_handler(u8 wed_idx, struct warp_msg_cmd *msg_cmd);

bool
warp_mcu_wo_support(void);

int
warp_get_wdma_rx_ring_dep(void *priv_data);

#endif /*_WARP_H*/
