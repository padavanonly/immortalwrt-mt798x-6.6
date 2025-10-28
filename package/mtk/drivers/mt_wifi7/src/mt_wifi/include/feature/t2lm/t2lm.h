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
    t2lm

    Abstract:
    t2lm

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef _T2LM_H_
#define _T2LM_H_

#include "rt_config.h"

enum {
	AT2LM_STS_FREE,
	AT2LM_STS_OCCUPY,
};

enum {
	AT2LM_STE_NONE,
	AT2LM_STE_MST,
	AT2LM_STE_ED
};

enum {
	T2LM_DL = 0,
	T2LM_UL = 1,
	T2LM_BI = 2,
};

enum {
	AT2LM_BCN = 0,
	AT2LM_PROBE_RSP = 1,
	AT2LM_ASSOC = 2,
};

enum {
	AT2LM_TO_TYPE_NONE = 0,
	AT2LM_TO_TYPE_MST_TSF = 1,
	AT2LM_TO_TYPE_E_TSF = 2,
};

/* LINK STATUS */
enum {
	LINK_STS_DISABLE	= 0,	/* at least one tid mapping */
	LINK_STS_ENABLE		= 1,	/* no tid mapping */
	LINK_STS_REMOVE		= 2,	/* ml reconfiguration: remove */
};

/* STA TID DL/UL IDX */
enum {
	TID_DL			= 0,	/* DL IDX */
	TID_UL			= 1,	/* UL IDX */
	TID_DL_UL_MAX		= 2,	/* DL+UL MAX */
};

/*NT2LM request type*/
enum {
	REQ_FROM_IWPRIV = 0,	/*from iwpriv/mwctl cmd*/
	REQ_FROM_WAPP = 1,		/*from wapp*/
};

#define TID_MAX			8	/* TID0~TID7 */


#define OFFSET_ED_WHEN_STE_MST	7
#define OFFSET_ED_WHEN_STE_ED	5
extern struct mld_dev mld_device;

/* spec. definition */
struct t2lm_ctrl_t {
	u16 dir:2;
	u16 def_link_map:1;
	u16 mst_present:1;
	u16 ed_present:1;
	u16 link_map_sz:1;
	u16 rsvd:2;
	u16 link_map_ind:8;
};

/*struct nl_80211_t2lm_cmd is for communicating between driver/wapp,
 * when sending/received t2lm request/response frames.
 * @ta_addr:	inidicate mac address of sender.
 * @ra_addr:	inidicate mac address of receiver.
 * @dir:		indicate direction of t2lm mapping.
 * @tid_bit_map:	bit map for tid.
 * @tid_to_link_bitmap: bit map for link.
 * @status_code:	report status of response from driver, reserved in other cases.
 */
struct nl_80211_t2lm_cmd {
	u8 ta_addr[MAC_ADDR_LEN];
	u8 ra_addr[MAC_ADDR_LEN];
	u8 dir;
	u8 tid_bit_map;
	u16 tid_to_link_bitmap[TID_MAX];
	u16 status_code;
};

/* nt2lm request callback function argument
 *
 * @wdev: struct wifi_dev
 * @mld: struct bmgr_mlo_dev
 * @mld_sta: struct bmgr_mld_sta
 * @status_code status code from protocol
 */
struct nt2lm_req_arg_t {
	IN struct wifi_dev *wdev;
	IN struct bmgr_mlo_dev *mld;
	IN struct bmgr_mld_sta *mld_sta;
	IN u16 status_code;
};

/*struct nl_80211_at2lm_map is for setting at2lm mapping from wappd
 * @mld_addr:	inidicate mld address of this TLV applied.
 * @dis_link_map: inidicate disabled linkid bit map.
 * @mst_dur:	Mapping Switch Time.
 * @exp_dur:	Expected Duration.
 */
struct nl_80211_at2lm_map {
	u8 mld_addr[MAC_ADDR_LEN];
	u16 dis_link_map;
	u32 mst_dur;
	u32 exp_dur;
};

/*nt2lm teardown callback function argument
 *
 * @wdev: struct wifi_dev
 * @mld: struct bmgr_mlo_dev
 * @mld_sta: struct bmgr_mld_sta
 */
struct nt2lm_teardown_arg_t {
	IN struct wifi_dev *wdev;
	IN struct bmgr_mlo_dev *mld;
	IN struct bmgr_mld_sta *mld_sta;
};

/* convert driver mld_grp to wm fw_mld_id */
u8 mld_grp_2_fw_mld_id(
	IN u8 mld_grp
);

/* convert wm fw_mld_id to driver mld_grp */
u8 fw_mld_id_2_mld_grp(
	IN u8 fw_mld_id
);

/* at2lm contract resource mgmt */
void a2tlm_contract_reset(
	IN struct bmgr_mlo_dev *mld
);
int a2tlm_contract_alloc(
	IN struct bmgr_mlo_dev *mld,
	OUT struct at2lm_contract_t **at2lm_contract
);
int a2tlm_contrat_release(
	IN struct bmgr_mlo_dev *mld,
	IN u8 id
);

/* at2lm ie buffer mgmt */
int at2lm_ie_buf_alloc(
	IN struct bmgr_entry *entry
);
int at2lm_ie_buf_free(
	IN struct bmgr_entry *entry
);

/* t2lm ie */
int at2lm_contract_dis_link_tid_map_update(
	IN struct bmgr_mlo_dev *mld,
	IN u16 dis_link_id_bitmap,
	OUT u8 *tid_map
);
void build_t2lm_ie(
	IN struct wifi_dev *wdev,
	IN struct t2lm_ctrl_t *t2lm_ctrl,
	IN u16 mst,
	IN u32 ed,
	IN u8 *t2lm,
	IN u8 present,
	OUT struct ie *at2lm
);
int at2lm_sanity_check(
	IN PNET_DEV pNetDev,
	IN u8 *pass
);
void at2lm_ie_build(
	IN struct at2lm_contract_t *at2lm_contract
);
UINT64 at2lm_curr_tsf_get(
	struct wifi_dev *wdev
);
u16 at2lm_ie_query(
	IN struct wifi_dev *wdev,
	IN u8 *f_buf_hdr,
	IN u8 *f_buf_last,
	IN u8 frame_type
);
u8 tid_2_link_sts(
	IN u8 tid_map_dl,
	IN u8 tid_map_ul
);
int at2lm_update_peer_mld_t2lm_at_assoc(
	IN struct bmgr_mlo_dev *mld,
	IN struct bmgr_mld_sta *mld_sta
);
int at2lm_update_peer_t2lm(
	IN u8 *mld_addr,
	IN u8 mld_grp,
	IN u8 id
);
int at2lm_rnr_dli_get(
	IN struct bmgr_entry *repted_entry,
	IN u8 *is_dis_link
);

/* at2lm contract execution */
int at2lm_req_certi(
	IN struct wifi_dev *wdev,
	IN u32 mst_dur,
	IN u32 e_dur
);
int at2lm_req(
	IN u8 *mld_addr,
	IN u16 dis_linkid_bitmap,
	IN u32 mst_dur,
	IN u32 e_dur
);
int at2lm_tsf_expiry(
	IN u8 *mld_addr,
	IN u8 mld_grp,
	IN u8 id,
	IN u8 to_type
);
int find_mld_sta_by_wcid(
	IN PRTMP_ADAPTER ad,
	IN u16 wcid,
	OUT struct bmgr_mld_sta **mld_sta
);
int nt2lm_peer_tid_map_update(
	IN struct wifi_dev *wdev,
	IN struct bmgr_mlo_dev *mld,
	IN struct bmgr_mld_sta *mld_sta
);
int nt2lm_t2lm_request(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid
);
int nt2lm_t2lm_teardown(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid
);
void nt2lm_peer_t2lm_req_action(
	IN PRTMP_ADAPTER ad,
	IN MLME_QUEUE_ELEM * Elem
);
int nt2lm_request_sanity_check(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN u16 wcid,
	IN struct nt2lm_contract_t *nt2lm_contract
);

#ifdef RT_CFG80211_SUPPORT
int mtk_cfg80211_nt2lm_request(
	IN PRTMP_ADAPTER ad,
	IN struct wifi_dev *wdev,
	IN struct nl_80211_t2lm_cmd *cmd);
#endif

int find_ap_mld_by_mld_addr(
	IN u8 *mld_addr,
	OUT struct bmgr_mlo_dev **mld
);

int mld_sta_tid_map_show(IN struct wifi_dev *wdev);
int mld_ap_tid_map_show(void);
int nt2lm_sta_mld_tid_map_update(
	IN struct wifi_dev *wdev
);
#endif /*_T2LM_H_*/
