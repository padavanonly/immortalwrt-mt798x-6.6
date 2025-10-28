/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2018, Mediatek Technology, Inc.
 *
 * All rights reserved. Mediatek's source code is an unpublished work and the
 * use of a copyright notice doeas not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    mld_link_mgr.h

    Abstract:
    MLD LINK Manager

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __MLD_LINK_MGR_H__
#define __MLD_LINK_MGR_H__

#ifdef DOT11_EHT_BE
#include "rt_config.h"


/* mld_grp_idx of AP and ApCli shall be disjoint,
 * so ApCli uses (AP's mld_grp_idx + 1)
 */
#define	STA_MLD_GROUP_IDX	(BMGR_MAX_MLD_GRP_CNT)

/* specific station index for peer ap mld device, it must differ with AP mode,
 * so it is defined to be (BMGR_MAX_MLD_STA_CNT + 1)
 */
#define PEER_AP_MLD_IDX	(BMGR_MAX_MLD_STA_CNT + 1)

/* peer mld single link entry */
struct peer_mld_single_link {
	uint8_t active;
	uint8_t reconfig_rm;
	uint8_t is_peer_need_clear;
	uint8_t link_addr[MAC_ADDR_LEN];
	uint8_t sta_ctrl;
	uint16_t bcn_interval;
	uint8_t dtim_count;
	uint8_t dtim_period;
	uint16_t nstr_bmap;
	u8 link_id;
	void *priv_ptr;
};

struct mld_link_entry {
	uint8_t used;
	uint8_t is_setup_link;
	struct wifi_dev *wdev;
	struct ie mld;		/* ml ie w/ link info */
	struct ie mld_cmm;	/* ml ie w/o link info */
};

/* mld peer entry, include multiple mld link */
struct peer_mld_entry {
	uint8_t valid;
	uint16_t idx;
	uint16_t aid;
	uint8_t emlsr;
	uint8_t mld_addr[MAC_ADDR_LEN];
	uint8_t BAOriTID;
	uint8_t BARecTID;
	struct peer_mld_single_link single_link[MLD_LINK_MAX];
	struct mld_link_entry *set_up_link;
	struct nt2lm_contract_t nt2lm_contract;
};

struct mld_dev {
	struct mld_link_entry *master_link;
	uint8_t mld_grp_idx;
	uint8_t used_mld_grp_ids[MAX_MLD_GROUP_NUM];
	uint8_t used_mld_grp_idx;
	uint8_t valid_link_num;
	uint8_t mld_addr[MAC_ADDR_LEN];
	struct mld_link_entry mld_own_links[MLD_LINK_MAX];
	struct peer_mld_entry peer_mld;
	struct ie probe_req_ml_ie;
	struct ie assoc_req_ml_ie;
};

enum ENUM_ML_APC_BCN_RX_UPDATE_NUM {
	ML_APC_BCN_RX_TIME = 0,
	ML_APC_BCN_RX_ENQ_TIME = 1,
	ML_APC_BCN_RX_TIME_MAX_NUM
};


int sta_mld_link_mgr_reg_dev(struct wifi_dev *wdev);
void sta_mld_link_mgr_dereg_dev(struct wifi_dev *wdev);
struct wifi_dev *get_sta_mld_setup_wdev(struct mld_dev *mld);
struct mld_link_entry *get_sta_mld_link_by_idx(struct mld_dev *mld, uint8_t link_idx);
int sta_update_mld_group_id(u8 group_id);
int init_sta_mld_link_mgr(void);
void uninit_sta_mld_link_mgr(void);
int sta_mld_conn_req(struct wifi_dev *wdev, struct mld_conn_req *mld_conn);
int sta_mld_disconn_req(struct wifi_dev *wdev);
uint16_t sta_mld_build_multi_link_ie(uint8_t *pos,
	struct wifi_dev *wdev, uint16_t type);
int sta_mld_link_up(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry);
int sta_mld_link_down(struct wifi_dev *wdev);
int sta_mld_connect_act(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry);
int sta_mld_disconnect_act(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry);
int sta_mld_link_sync(struct wifi_dev *wdev_src, MAC_TABLE_ENTRY *pEntry_src);
int sta_mld_update_rx_bcn_time(struct wifi_dev *pwdev, uint8_t rx_bcn_type);
int sta_mld_ba_ori_setup(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, u8 TID, u16 TimeOut);
int sta_mld_ba_resrc_ori_add(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, u8 TID, u16 ori_ba_wsize, u8 amsdu_en, u16 TimeOut);
int sta_mld_ba_resrc_rec_add(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, u8 TID, u16 TimeOut, u16 StartSeq, u16 rec_ba_wsize);
int sta_mld_ba_resrc_ori_del(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, u8 TID, u8 bPassive);
int sta_mld_ba_resrc_rec_del(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, u8 TID, u8 bPassive);
int sta_mld_ba_add_to_asic(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, u8 TID, u16 Seq, u16 BAWinSize, int type, u8 amsdu_en);
int sta_mld_ba_del_from_asic(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, u8 TID, int type, u8 amsdu_en);
int sta_mld_ra_init(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry);
bool sta_mld_set_up_wdev_check(struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry);
VOID sta_sync_mld_per_link_cap(struct _MAC_TABLE_ENTRY *pEntry, struct ml_ie_info *ml_info, struct common_ies *comm_ies);
VOID sta_sync_mld_per_link_mlo_csa(struct _MAC_TABLE_ENTRY *pEntry, struct ml_ie_info *ml_info);
VOID sta_sync_mlo_csa_channel(RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
INT sta_sync_mlo_comm_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, struct common_ies *cmm_ies, PEDCA_PARM pEdcaParm);
VOID mld_parse_per_link_cap(RTMP_ADAPTER *pAd, BCN_IE_LIST *ie_list,
	struct multi_link_info *ml_info);
int fill_peer_mlo_info(struct peer_mld_entry *peer, uint8_t link_num, uint8_t is_setup_link, uint16_t setup_wcid, OUT struct eht_mlo_t *mlo);
int sta_mld_insert_link(struct mld_dev *mld, struct peer_mld_entry *peer, struct ml_ie_info *ml_info, int own_idx, int
peer_idx, uint16_t *setup_wcid, struct mld_link_entry *set_up_link, uint8_t *link_num);
struct mld_link_entry *get_sta_mld_link_by_wdev(struct mld_dev *mld, struct wifi_dev *wdev);
struct rnr_ml_param *get_unused_rnr_ml_param_entry(struct rnr_ml_param *rnr_ml_params, int rnr_len);
struct rnr_ml_param *search_rnr_ml_param_entry(struct rnr_ml_param *rnr_ml_params, int rnr_len, struct rnr_ml_param *ml_param);
void sta_mld_link_change_handle(struct _RTMP_ADAPTER *pAd,
		struct wifi_dev *wdev, u16 wcid, BCN_IE_LIST *bcn_ie_list);
#ifdef RT_CFG80211_SUPPORT
int mtk_cfg80211_reply_apcli_mld(struct wiphy *wiphy);
#endif
#endif
#endif /*__MLD_LINK_MGR_H__*/
