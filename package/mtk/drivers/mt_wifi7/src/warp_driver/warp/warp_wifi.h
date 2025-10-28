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
	warp_wifi.h
*/
#ifndef _WARP_WIFI_H_
#define _WARP_WIFI_H_

#include <linux/types.h>
#ifdef CONFIG_MULDF_HELPER
#include "warp_dbg.h"
#endif

#define WARP_VERSION_CODE		0x0301
#define WARP_VERSION(VER, SUB_VER)	(((VER) << 8) + (SUB_VER))

/*TX hook structure*/
struct wlan_tx_info {
	u8 *pkt;
	u32 bssidx;
	u32 wcid;
	u32 ringidx;
	u32 usr_info;
	u8 tid;
	u8 is_fixedrate;
	u8 is_prior;
	u8 is_sp;
	u8 hf;
	u8 amsdu_en;
};

struct wlan_rx_info {
	u8 *pkt;
	u16 ppe_entry;
	u8 csrn;
};

struct ring_ctrl {
	u32 base;
	u32 cnt;
	u32 cidx;
	u32 didx;
	u32 lens;
	u32 attr_mask;
	dma_addr_t cb_alloc_pa;
	bool attr_enable;
};

struct rro_ctrl {
	u16 max_win_sz;
	u32 ack_sn_cr;
	u32 particular_se_id;
	dma_addr_t particular_se_base;
	u8 se_group_num;
	dma_addr_t se_base[128];
};

enum {
	WIFI_HW_CAP_PAO,
	WIFI_HW_CAP_RRO,
	WIFI_HW_CAP_RRO_3_0 = WIFI_HW_CAP_RRO,
	WIFI_HW_CAP_RRO_3_1,
};

enum {
	WIFI_HIF_TXD_V0_0,
	WIFI_HIF_TXD_V0_1,
	WIFI_HIF_TXD_V1_0,
	WIFI_HIF_TXD_V2_0,
	WIFI_HIF_TXD_V2_1,
	WIFI_HIF_TXD_V2_2,
	WIFI_HIF_TXD_V3_0,
};

struct wifi_hw {
	void *priv;
	void *hif_dev;
	u8 non_coherent_dma_addr_size;
	u8 coherent_dma_addr_size;
	u32 *p_int_mask;
	u32 chip_id;
	u32 tx_ring_num;
	u32 tx_token_nums;
	u32 tx_buf_nums;
	u32 sw_tx_token_nums;
	u32 hw_rx_token_num;
	u32 rx_page_nums;
	unsigned long dma_offset;
	u32 int_sta;
	u32 int_mask;
	u32 tx_dma_glo_cfg;
	u32 ring_offset;
	u32 txd_size;
	u32 fbuf_size;
	u32 tx_pkt_size;
	u32 tx_ring_size;
	u32 rx_ring_size;
	u32 int_ser;
	u32 int_ser_value;
	struct ring_ctrl tx[2];
	struct ring_ctrl event;

	u32 rx_dma_glo_cfg;
	u32 rx_ring_num;
	u32 rx_data_ring_size;
	u32 rxd_size;
	u32 rx_pkt_size;
	u16 rx_page_size;
	u32 max_rxd_size;
	u32 sign_base_cr;
	u8 rx_wed_ring_num;
	u8 rx_rro_data_ring_num;
	u8 rx_rro_page_ring_num;
	u8 rx_rro_ind_cmd_ring_num;
	u8 rx_rro_3_1_ring_num;
	u8 tx_free_done_ver;
	struct ring_ctrl rx[2];
	struct ring_ctrl rx_wed[2];
	struct ring_ctrl rx_rro[2];
	struct ring_ctrl rx_rro_pg[3];
	struct ring_ctrl rx_rro_ind_cmd;
	struct ring_ctrl rx_rro_3_1;
	struct rro_ctrl rro_ctl;

	unsigned long wpdma_base;
	unsigned long base_phy_addr;
	unsigned long base_addr;

	u8 rx_wed_done_trig0_bit;
	u8 rx_wed_done_trig1_bit;
	u8 wfdma_tx_done_trig0_bit;
	u8 wfdma_tx_done_trig1_bit;
	u8 wfdma_tx_done_free_notify_trig_bit;
	u8 wfdma_rx_done_trig0_bit;
	u8 wfdma_rx_done_trig0_enable;
	u8 wfdma_rx_done_trig1_bit;
	u8 wfdma_rx_done_trig1_enable;
	u8 wfdma_rro_rx_done_trig0_bit;
	u8 wfdma_rro_rx_done_trig1_bit;
	u8 wfdma_rro_rx_pg_trig0_bit;
	u8 wfdma_rro_rx_pg_trig1_bit;
	u8 wfdma_rro_rx_pg_trig2_bit;
	u8 wfdma_rro_rx_3_1_trig_bit;

	u32 hif_type;
	u32 irq;
	u32 hw_cap;
	u32 hif_txd_ver;
	u32 max_amsdu_len;
	u32 max_wcid_nums;
	u32 wed_v1_compatible_en_addr;
	u32 wed_v1_compatible_en_msk;
	u32 wed_v1_compatible_tx0_addr;
	u32 wed_v1_compatible_tx0_msk;
	u32 wed_v1_compatible_tx0_id;
	u32 wed_v1_compatible_tx1_addr;
	u32 wed_v1_compatible_tx1_msk;
	u32 wed_v1_compatible_tx1_id;
	u32 wed_v1_compatible_rx1_addr;
	u32 wed_v1_compatible_rx1_msk;
	u32 wed_v1_compatible_rx1_id;

	u8 max_amsdu_nums;
	u8 mac_ver;
	u8 src;
	bool msi_enable;
	bool dbdc_mode;
	bool rm_vlan;
	bool hdtr_mode;
};

struct wifi_ops {
	void (*config_atc)(void *priv_data, bool enable);
	void (*swap_irq)(void *priv_data, u32 irq);
	void (*fbuf_init)(u8 *fbuf, u32 pkt_pa, u32 tkid);
	void (*txinfo_wrapper)(u8 *tx_info, struct wlan_tx_info *info);
	void (*txinfo_set_drop)(u8 *tx_info);
	bool (*hw_tx_allow)(u8 *tx_info);
	void (*tx_ring_info_dump)(void *priv_data, u8 ring_id, u32 idx);
	void (*warp_ver_notify)(void *priv_data, u8 ver, u8 warp_sub_ver, u8 warp_branch, int warp_hw_caps);
	u32 (*token_rx_dmad_init)(void *priv_data, void *pkt, unsigned long alloc_size,
				 void *alloc_va, dma_addr_t alloc_pa);
	int (*token_rx_dmad_lookup)(void *priv_data, u32 tkn_rx_id, void **pkt,
				    void **alloc_va, dma_addr_t *alloc_pa);
	void (*rxinfo_wrapper)(u8 *rx_info, struct wlan_rx_info *info);
	void (*do_wifi_reset)(void *priv_data);
	void (*update_wo_rxcnt)(void *priv_data, void *wo_rxcnt);
	void (*request_irq)(void *priv_data, u32 irq);
	void (*free_irq)(void *priv_data, u32 irq);
	void (*hb_check_notify)(void *priv_data);
	void (*fbuf_v1_init)(u8 *fbuf, dma_addr_t pkt_pa, u32 tkid, u8 src);
	void (*set_attach)(void *priv_data, bool enable);
};

enum {
	BUS_TYPE_PCIE,
	BUS_TYPE_AXI,
	BUS_TYPE_MAX
};

enum {
	WIFI_FREE_IRQ = 0,
	WIFI_REQUEST_IRQ = 1,
};


struct wifi_entry {
	unsigned long base_addr;
	struct wifi_hw hw;
	struct wifi_ops *ops;
};

/*default SER status*/
#define WIFI_ERR_RECOV_NONE				0x10
#define WIFI_ERR_RECOV_STOP_IDLE		0
#define WIFI_ERR_RECOV_STOP_PDMA0		1
#define WIFI_ERR_RECOV_RESET_PDMA0		2
#define WIFI_ERR_RECOV_STOP_IDLE_DONE		3
#define WIFI_ETH_ERR_STOP_WED_RX_TRAFFIC	4
#define WIFI_ETH_ERR_START_WED_RX_TRAFFIC	5


/*wifi related hal*/
void wifi_tx_ring_info_dump(struct wifi_entry *wifi, u8 ring_id, u32 idx);
void wifi_chip_atc_set(struct wifi_entry *wifi, bool enable);
void wifi_chip_probe(struct wifi_entry *wifi, u32 irq, u8 warp_ver, u8 warp_sub_ver,
		     u8 warp_branch, int warp_hw_caps);
void wifi_chip_remove(struct wifi_entry *wifi);
void wifi_chip_set_irq(struct wifi_entry *wifi, bool irq_set, u32 irq_number);
bool wifi_hw_tx_allow(struct wifi_entry *wifi, u8 *tx_info);
void wifi_chip_reset(struct wifi_entry *wifi);
void wifi_chip_update_wo_rxcnt(struct wifi_entry *wifi, void *wo_rxcnt);
void wifi_hb_check_notify(struct wifi_entry *wifi);

#define to_wifi_entry(_hw) container_of(_hw, struct wifi_entry, hw)

#endif /*_WARP_WIFI_H_*/
