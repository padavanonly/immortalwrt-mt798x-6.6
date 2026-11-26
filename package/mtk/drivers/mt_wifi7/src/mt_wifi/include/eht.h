/*
 * Copyright (c) [2021], MediaTek Inc. All rights reserved.
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

#ifndef _EHT_H_
#define _EHT_H_

#include "dot11be_eht.h"
#include "mlme.h"
#include "tr_ba.h"

#ifdef DOT11_EHT_BE
#include "bss_mngr.h"

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#define IS_EHT_STA(mode)\
	((mode) & (EHT_24G_SUPPORT | EHT_5G_SUPPORT | EHT_6G_SUPPORT))
#define IS_EHT_2G_STA(mode)\
	(((mode) & EHT_24G_SUPPORT) == EHT_24G_SUPPORT)
#define IS_EHT_5G_STA(mode)\
	(((mode) & EHT_5G_SUPPORT) == EHT_5G_SUPPORT)
#define IS_EHT_6G_STA(mode)\
	(((mode) & EHT_6G_SUPPORT) == EHT_6G_SUPPORT)

#define WRAP_BUFFER(_wrapper, _buf_head, _buf_len)\
	do {\
		_wrapper.buf = _buf_head;\
		_wrapper.buf_len = _buf_len;\
	} while (0)

#define NUM_SWAP(X, Y) do {X ^= Y; Y ^= X; X ^= Y; } while (0)


/*******************************************************************************
 *                         D A T A   T Y P E S
 *******************************************************************************
 */
#define EHT_MAX_SUPPORT_STREAM DOT11BE_MAX_STREAM
struct common_ies;
struct multi_link_info;

/**
 * struct mld_entry -
 * 1. This structure is used by MLO case and per link (mac table) in MLO case
 *	share the same mld_entry.
 * 2. mld_entry maintains MLD level features, mac tbale entry maintains LINK
 *	level features
 *
 * @addr: mld mac address
 * @mld_sta_idx: provide by bssmngr and used between bssmngr/driver/hwifi to
 *	identify peer stas which are in the same mld.
 * @nsep: nsep
 * @emlmr: emlmr
 * @emlsr: emlsr
 * @eml_cap: eml cap
 * @str_map: str link pair
 * @setup_link_wcid: setup link wcid
 * @setup_link_id: link id of setup link, the link id here is the same as mlo
 *	protocol's
 * @link_num: link number of this mld
 * @link_entry: struct _MAC_TABLE_ENTRY of each link
 * @valid: if this mld_entry is ready to be used
 * @next: pointer to next struct mld_entry and used in mld hash table
 *
 */
struct mld_entry_t {
	struct list_head list;
	/* mld */
	uint8_t addr[MAC_ADDR_LEN];
	uint16_t mld_sta_idx;
	uint8_t nsep;
	uint16_t emlmr;
	uint16_t emlsr;
	uint16_t eml_cap;
	uint16_t str_map[MLD_LINK_MAX];
	/* overall links info of mld */
	uint16_t setup_link_wcid;
	uint8_t setup_link_id;
	uint8_t link_num;
	void *link_entry[MLD_LINK_MAX];
	/* others */
	uint8_t valid;
	struct hlist_node hlist;
	struct rcu_head rcu;
	struct BA_INFO ba_info;
};

/**
 * struct eht_link_t - eht link related information
 *
 * @link_id: link id
 * @tid_bitmap_dl: dl tid bitmap of this link, BIT0: TID0,  BIT1: TID1....
 * @tid_bitmap_ul: ul tid bitmap of this link, BIT0: TID0,  BIT1: TID1....
 */
struct eht_link_t {
	uint8_t link_id;
	uint8_t tid_map_dl;
	uint8_t tid_map_ul;
	uint8_t link_sts;
};

/**
 * struct eht_mlo_t - used by eht link for link and mld information
 *
 * @mlo_en: this link is one of MLD or not
 * @is_setup_link_entry: is this link entry a setup link
 * @mld_addr: mld mac addr
 * @link_info: eht link info of this link
 */
struct eht_mlo_t {
	uint8_t mlo_en;
	uint8_t is_setup_link_entry;
	uint8_t mld_addr[MAC_ADDR_LEN];
	struct eht_link_t link_info;
};

struct eht_ch_layout {
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};

/* Protected EHT Action frame */
struct eht_prot_action_frame {
	HEADER_802_11   Hdr;
	UCHAR category;
	UCHAR prot_eht_action;
	UCHAR data[0];
};

enum eht_mld_eml_mode {
	EML_DISABLE = 0,
	EMLSR,
	EMLMR,
	MAX_EML_MODE,
};

enum mld_reconfig_mode {
	MLD_RECONFIG_MODE_INSTANT = 0,	/* delete link instantly */
	MLD_RECONFIG_MODE_COUNTDOWN,	/* delete link follow spec countdown procedure */
};

enum {
	EHT_BW_20,
	EHT_BW_2040,
	EHT_BW_80,
	EHT_BW_160,
	EHT_BW_320,
	EHT_BW_MAX
};

enum eht_bf_caps {
	EHT_SU_BFER = 1,
	EHT_SU_BFEE = 1 << 1,
	EHT_MU_BFER = 1 << 2,
	EHT_BFEE_NG16_SU_FEEDBACK = 1 << 3,
	EHT_BFEE_NG16_MU_FEEDBACK = 1 << 4,
	EHT_BFEE_CODEBOOK_4_2_SU_FEEDBACK = 1 << 5,
	EHT_BFEE_CODEBOOK_7_5_SU_FEEDBACK = 1 << 6,
	EHT_TRIGED_SU_BF_FEEDBACK = 1 << 7,
	EHT_TRIGED_MU_BF_PATIAL_BW_FEEDBACK = 1 << 8,
	EHT_TRIGED_CQI_FEEDBACK = 1 << 9,
	EHT_NON_TRIGED_CQI_FEEDBACK = 1 << 10,

};

enum eht_gi_caps {
	EHT_NDP_4X_EHT_LTF_3DOT2US_GI = 1,
	EHT_MU_PPDU_4X_EHT_LTF_DOT8US_GI = 1 << 1,
};

enum eht_mcs15_mru_support {
	EHT_MCS15_MRU_106_or_52_w_26_tone = 1,
	EHT_MCS15_MRU_484_w_242_tone_80M = 1 << 1,
	EHT_MCS15_MRU_996_to_242_tone_160M = 1 << 2,
	EHT_MCS15_MRU_3x996_tone_320M = 1 << 3,
};
enum eht_phy_caps {
	EHT_320M_6G = 1,
	EHT_242_TONE_RU_WT_20M = 1 << 1,
	EHT_PARTIAL_BW_UL_MI_MIMO  = 1 << 2,
	EHT_PARTIAL_BW_DL_MU_MIMO = 1 << 3,
	EHT_PSR_BASED_SR = 1 << 4,
	EHT_POWER_BOOST_FACTOR = 1 << 5,
	EHT_TX_1024QAM_4096QAM_LE_242_TONE_RU = 1 << 6,
	EHT_RX_1024QAM_4096QAM_LE_242_TONE_RU = 1 << 7,
	EHT_PPE_THRLD_PRESENT = 1 << 8,
	EHT_EXTRA_LTF_SUPPORT = 1 << 9,
	EHT_DUP_6G = 1 << 10,
	EHT_20M_RX_NDP_W_WIDER_BW = 1 << 11,
	EHT_NON_OFMDA_UL_MU_MIMO_LE_EQ_80M = 1 << 12,
	EHT_NON_OFMDA_UL_MU_MIMO_160M = 1 << 13,
	EHT_NON_OFMDA_UL_MU_MIMO_320M = 1 << 14,
	EHT_MU_BF_LE_EQ_80M = 1 << 15,
	EHT_MU_BF_160M = 1 << 16,
	EHT_MU_BF_320M = 1 << 17,
};

struct eht_bf_info {
	enum eht_bf_caps bf_cap;
	uint8_t bfee_ss_le_eq_bw80;
	uint8_t bfee_ss_bw160;
	uint8_t bfee_ss_bw320;
	uint8_t snd_dim_le_eq_bw80;
	uint8_t snd_dim_bw160;
	uint8_t snd_dim_bw320;
	uint8_t bfee_max_nc;
	uint8_t max_eht_ltf_num;

};

/* For AP handle multi-link discovery & setup */
struct eht_assoc_req_priv {
	BOOLEAN setup_link_success;		/* in */
	ULONG buf_len;					/* out */
	uint8_t *buf;					/* out */
	USHORT non_setup_link_status;	/* out */
};

struct buffer_wrapper {
	ULONG buf_len;
	UINT8 *buf;
};

struct MEM_NODE {
	UINT16 id;
	UINT8 *addr;
	struct MEM_NODE *next;
};

/* For AP MLD feature, Link Recommendation */
struct eht_period_link_recomm_t {
	u8 cnt;
	u16 num_mlme_period;	/* transmit LR frames every number of MLME periodical event */
	u16 tx_link_bmap;	/* links that want to tx LR frames*/
	u8 tx_times;		/* total number of LR frame to be tx */
	u8 recom_mld_grp;
	u16 recom_link_bmap;
	/* u8 recom_mld_grp[BMGR_MAX_MLD_GROUP_NUM]; */
	/* u16 recom_link_bmap[BMGR_MAX_MLD_GROUP_NUM]; */
};

/* for presence bitmap of MLD operations params during using MLD grouping API */
enum eht_mld_op_param_pres {
	MLD_OP_PARAM_PRES_GROUP_IDX = BIT(0),
	MLD_OP_PARAM_PRES_ADDR = BIT(1),
	MLD_OP_PARAM_PRES_EML_MODE = BIT(2),
	MLD_OP_PARAM_PRES_EML_TRANS_TO = BIT(3),
	MLD_OP_PARAM_PRES_EML_OMN = BIT(4),
	MLD_OP_PARAM_PRES_T2LM_NEGO = BIT(5),
	MLD_OP_PARAM_PRES_RECONFIG_TO = BIT(6),
};

/** struct eht_mld_param - parameters of MLD
 *
 * This struct used to maintain the parameters of MLD to BSS/MLD Manager.
 *
 * @param msd_en Indicate whether the Medium Sync Delay Information subfield appears in ML IE
 * @param msd_dur MSD subfield in MSD Info subfield
 * @param msd_ofdm_ed_thr MSD OFDM ED Threshold subfield in MSD Info subfield
 * @param msd_max_txop MSD Max Num of TXOPs subfield in MSD Info subfield
 * @param eml_mode Indicate whether MLD supports eml mode and what mode it supports
 * @param eml_trans_to Indicate EML Transition Timeout subfield in ML IE
 * @param eml_omn Indicate whether AP MLD replies EML OMN frame from peer
 * @param t2lm_nego_supp Indicate support for TTLM negotiation
 */
struct eht_mld_param {
	/* Medium Synchronization Delay */
	u8 msd_en;
	u8 msd_dur;
	u8 msd_ofdm_ed_thr;
	u8 msd_max_txop;

	/* EML Capabilities */
	u8 eml_mode; /* eht_mld_eml_mode */
	u8 eml_trans_to;
	u8 eml_omn;

	/* MLD Capabilities */
	u8 t2lm_nego_supp;
};

/** struct eht_mld_op_ctrl - control of MLD operation
 *
 * This struct used to control flow/maintain parameters for MLD operation.
 * MLD Operations include MLD Create/Destroy/Add Link/Delete Link/Set Attr.
 *
 * @param mld_grp MLD index used in BSS/MLD Manager
 * @param mld_addr MAC address of corresponding MLD
 * @param reconfig_to Indicate how many seconds to countdown in ML Reconfig. (Delete link)
 * @param cfg_disconn Indicate whether disconnects peer in corresponding MLD op (MLD Set Attr)
 * @param pres_bmap Presence bitmap of fields in `mld_param` field (MLD Set Attr)
 * @param mld_param MLD Parameters/Attributes (MLD Create, MLD Set Attr)
 */
struct eht_mld_op_ctrl {
	/* general */
	u8 mld_grp;
	u8 mld_addr[MAC_ADDR_LEN];

	/* control of MLD Set Attr */
	u8 cfg_disconn;
	u32 pres_bmap; /* eht_mld_op_param_pres */

	/* AP ML Reconfiguration */
	u16 reconfig_to;

	struct eht_mld_param mld_param;
};


/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

uint32_t eht_add_beacon_ies(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint32_t f_len);

uint32_t eht_add_probe_rsp_ies(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint32_t f_len,
	struct common_ies *cmm_ies);

uint32_t eht_add_auth_req_ies(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint32_t f_len);

uint32_t eht_add_auth_rsp_ies(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint32_t f_len,
	struct common_ies *cmm_ies);

uint32_t eht_add_assoc_rsp_ies(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint32_t f_len,
	struct common_ies *cmm_ies);

uint32_t eht_add_assoc_req_ml_ies(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint32_t f_len);

uint32_t eht_add_assoc_req_cap_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf,
	UINT32 f_len);

uint8_t *eht_build_multi_link_ie(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint8_t *ml_ie,
	struct frag_ie_info *ml_frag_info,
	uint16_t ml_type,
	uint16_t mld_sta_idx);

uint8_t *eht_build_reconfig_multi_link_ie(
	struct wifi_dev *wdev,
	uint8_t *f_buf,
	uint16_t f_len,
	uint16_t ml_type);

void eht_mlo_query_link_probe_rsp_frame(
	struct wifi_dev *wdev,
	struct buffer_wrapper *rsp_buf);

int eht_get_ies(
	uint8_t *ie_head,
	struct common_ies *cmm_ies,
	struct frag_ie_info *frag_info);

uint32_t eht_starec_feature_decision(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *entry,
	uint64_t *feature);

int eht_calculate_ch_band(struct common_ies *cmm_ies, UCHAR *ch_band);

int eht_calculate_channel(struct common_ies *cmm_ies, UCHAR *channel);

uint16_t eht_build_multi_link_conn_req(
	struct wifi_dev *wdev,
	uint8_t subtype,
	uint8_t *ml_ie,
	struct frag_ie_info *ml_frag_info,
	uint8_t *t2l_ie,
	uint8_t *link_addr,
	uint8_t *mld_addr,
	u8 ch_band);

void eht_update_peer_caps(
	struct _MAC_TABLE_ENTRY *peer,
	struct common_ies *cmm_ies);

void eht_store_basic_multi_link_info(
	UCHAR * basic_multi_link_ie,
	struct common_ies *cmm_ies);

UINT32 eht_add_probe_req_ies(
	struct wifi_dev *wdev,
	UINT8 *f_buf);

void eht_parse_multi_link_ie(
	struct wifi_dev *wdev,
	uint8_t *ml_ie,
	struct frag_ie_info *ml_frag_info,
	struct ml_ie_info *ml_info);

UCHAR rf_bw_2_eht_bw(UCHAR rf_bw);

BOOLEAN eht320_channel_group(
	struct _RTMP_ADAPTER *pAd,
	UCHAR channel,
	struct wifi_dev *wdev);

VOID eht_mode_adjust(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *peer,
	struct common_ies *cmm_ies);

struct eht_ch_layout *get_eht_ch_array(
	UINT8 bw,
	UCHAR ch_band,
	UINT *arr_size);

UCHAR eht_cent_ch_freq
	(struct wifi_dev *wdev,
	UCHAR prim_ch,
	UCHAR eht_bw,
	UCHAR ch_band);

void eht_update_peer_dscb_info(
	struct wifi_dev *wdev,
	struct _MAC_TABLE_ENTRY *peer,
	struct common_ies *cmm_ies);

UINT8 *eht_build_cap_ie(
	struct wifi_dev *wdev,
	UINT8 *f_buf);

UINT8 ap_mld_type_trans(enum bmgr_mld_type mld_type);
INT eht_ap_mld_fill_mld_param_from_selected_link(struct wifi_dev *wdev, struct eht_mld_param *mld_param);
INT eht_ap_mld_create(struct wifi_dev *wdev, u8 *mld_group_idx, u8 *mld_addr, struct eht_mld_param *mld_param, u8 *mld_type);
INT eht_ap_mld_destroy(struct wifi_dev *wdev, u8 mld_group_idx);
INT eht_ap_mld_destroy_by_mld_addr(struct wifi_dev *wdev, u8 *mld_addr);
INT eht_ap_mld_attr_set(struct wifi_dev *wdev, u8 mld_group_idx, struct eht_mld_op_ctrl *op_ctrl);
INT eht_ap_mld_attr_set_by_mld_addr(struct wifi_dev *wdev, u8 *mld_addr, struct eht_mld_op_ctrl *op_ctrl);
INT eht_ap_mld_add_link(struct wifi_dev *wdev, u8 mld_group_idx);
INT eht_ap_mld_add_link_by_mld_addr(struct wifi_dev *wdev, u8 *mld_addr);
INT eht_ap_mld_del_link(struct wifi_dev *wdev);
INT eht_ap_add_to_single_link_mld(struct wifi_dev *wdev);
INT eht_ap_mld_link_transfer(struct wifi_dev *wdev, u8 mld_group_idx);
BOOLEAN eht_ap_mld_available_address_search(struct wifi_dev *wdev, u8 mld_group_idx, u8 *mld_addr, u8 mld_type);
INT eht_ap_mld_trig_link_reconfiguration(struct wifi_dev *wdev, u16 reconfig_to);
INT eht_ap_mld_exec_link_reconfiguration(struct reconfig_tmr_to_t *reconfig_to_info);
INT eht_ap_mld_periodic_link_recomm(struct _RTMP_ADAPTER *pAd);

/**
 * @brief Wrapper function to fill default MLD attribute to struct eht_mld_param
 *
 * This function fills the default MLD attibutes to the eht_mld_param structure.
 * The mld_param will be used to create MLD later.
 *
 * @param mld_param Save the parameters(attributes) of an MLD.
 */
static inline INT eht_ap_mld_fill_default_mld_param(struct eht_mld_param *mld_param)
{
	NdisZeroMemory(mld_param, sizeof(struct eht_mld_param));
	mld_param->eml_mode = EMLSR;
	mld_param->t2lm_nego_supp = 1;

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS eht_ap_mlo_postprocess_reported_link_probe_rsp(
	struct buffer_wrapper *repted_rsp_buf,
	struct buffer_wrapper *repting_rsp_buf);

VOID eht_ap_mlo_peer_non_setup_links_assoc_req(
	struct _MLME_QUEUE_ELEM *Elem,
	struct _MAC_TABLE_ENTRY *pEntry,
	UINT8 setup_link_Sst,
	USHORT setup_link_status_code,
	struct buffer_wrapper *setup_link_rsp,
	struct common_ies *cmm_ies,
	BOOLEAN isReassoc);

NDIS_STATUS eht_mlo_sta_profile_inheritance_rule(
	struct buffer_wrapper *repting_frame_ie,
	struct buffer_wrapper *repted_frame_ie,
	struct buffer_wrapper *result);

void eht_remove_ie_by_id_list(
	struct buffer_wrapper *source,
	struct buffer_wrapper *target,
	struct buffer_wrapper *id_list,
	struct buffer_wrapper *id_ext_list);

UCHAR eht_bw_2_rf_bw(UCHAR eht_bw);
#else /* DOT11_EHT_BE */
#define eht_get_ies(_a, _b, _c)
#define eht_update_peer_caps(_a, _b)
#endif /* !DOT11_EHT_BE */

#endif /*_EHT_H_*/
