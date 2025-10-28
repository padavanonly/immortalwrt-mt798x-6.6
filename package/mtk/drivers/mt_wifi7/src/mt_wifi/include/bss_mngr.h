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
    bss_mngr.h

    Abstract:
    BSS manager

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_BSS_MNGR_H__
#define __AP_BSS_MNGR_H__

#include "rt_config.h"

struct _MLME_QUEUE_ELEM;
struct MLO_TWT_PRIV_T;
struct eht_mld_param;
struct eht_mld_op_ctrl;

/* max link(band) count */
#define BSS_MNGR_MAX_BAND_NUM		3

/* max link_id that bmgr will assign */
#define BSS_MNGR_MAX_LINK_ID	(BSS_MNGR_MAX_BAND_NUM - 1)

/* information length include IE buffer */
#define BSS_MNGR_MAX_INFO_LEN		1024

/* information length for MBSSID IE buffer */
#define BSS_MNGR_MAX_MBSSID_IE_LEN	2048

/* STA Profile length in MLD IE */
#define BSS_MNGR_MAX_STA_PROFILE_LEN	512

/* Per-STA Profile length in MLD IE */
#define BSS_MNGR_MAX_PER_STA_PROFILE_LEN	576

/* aid reserved for mld station, 1024 ~ INVALID_AID */
#define START_MLD_STA_AID			1024

/* EID */
#define EID_REDUCED_NEIGHBOR_REPORT 201

/* sub-element of IE_NONTRANSMITTED_BSSID_CAP */
#define SUB_IE_NON_TRANS_PROFILE			0
#define SUB_IE_NON_TRANS_VENDOR_SPECIFIC	221

/* max econding of fields defined in spec */
/* Medium Synchronization OFDM ED Threshold subfield */
#define MLD_MAX_MSD_OFDM_ED_THR 10
/* Tansition Timeout subfield */
#define MLD_MAX_EML_TRANS_TO 10

/* per sta prfile max len */
#define MAX_PER_STA_PROFILE_LEN		200

/* per sta prf cap info size */
#define PER_STA_PROILE_CAP_INFO_SIZE	2

/* max mld group count maintained by bss manager */
#define BMGR_MAX_ML_MLD_CNT		16	/* MAX multi-link MLD */
#define BMGR_MAX_SL_MLD_CNT		48	/* MAX single-link MLD */
/* grp 0 means non-MLO/single-link MLO */
#define BMGR_MAX_MLD_GRP_CNT	(1 + BMGR_MAX_ML_MLD_CNT + BMGR_MAX_SL_MLD_CNT + 1)
#define BMGR_INVALID_MLD_GRP	(BMGR_MAX_MLD_GRP_CNT - 1)

/* max at2lm contract set */
#define BSS_MNGR_MAX_AT2LM_SET_NUM		1
/* max t2lm IE buffer sz*/
#define BSS_MNGR_MAX_T2LM_IE_LEN		26

/* max length of common info of Reconfiguration ML IE
 * including TLV and Multi-Link Contrl field
 */
#define MAX_RECONFIG_ML_IE_COMMON_LEN 24
/* max length of a Per-STA Profile of Reconfiguration ML IE */
#define MAX_RECONFIG_ML_IE_PER_LINK_INFO_LEN 24
/* max length of Reconfiguration ML IE in AP ML Reconfiguration */
#define MAX_RECONFIG_ML_IE_LEN \
	(MAX_RECONFIG_ML_IE_COMMON_LEN + \
	(BSS_MNGR_MAX_BAND_NUM * MAX_RECONFIG_ML_IE_PER_LINK_INFO_LEN))

/* BPCC lock operation */
enum {
	BMGR_BPCC_OP_RELEASE_LOCK     = 0,
	BMGR_BPCC_OP_GET_LOCK         = 1,
	BMGR_BPCC_OP_RESET_LOCK_TIMER = 2,
	BMGR_BPCC_OP_GET_LOCK_ALL     = 3,
	BMGR_BPCC_OP_MLD_ID_LOG_ON    = 4,
	BMGR_BPCC_OP_MLD_ID_LOG_OFF	  = 5,
};

#define BMGR_IS_ML_MLD_GRP_IDX(_idx) \
		((_idx <= BMGR_MAX_ML_MLD_CNT) && \
		(_idx != BMGR_INVALID_MLD_GRP) && \
		(_idx != 0))

/* dev_type */
enum {
	DEV_TYPE_AP_LEGACY	= 0,	/* AP: Legacy(Proprietary) BSSID */
	DEV_TYPE_AP_11VT	= 1,	/* AP: Transmitted BSSID */
	DEV_TYPE_AP_11VNT	= 2,	/* AP: Non-Transmitted BSSID */
	DEV_TYPE_AP_COH		= 3,	/* AP: Co-Hosted BSSID */
	DEV_TYPE_UNDEF		= 4,	/* undefined */
};

#define IS_DEV_TYPE_W_BCN(_type) \
	(((_type) == DEV_TYPE_AP_LEGACY) || \
	((_type) == DEV_TYPE_AP_11VT) || \
	((_type) == DEV_TYPE_AP_COH))
#define IS_DEV_TYPE_11V_T(_type) \
	((_type) == DEV_TYPE_AP_11VT)
#define IS_DEV_TYPE_11V_NT(_type) \
	((_type) == DEV_TYPE_AP_11VNT)
#define IS_DEV_TYPE_11V(_type) \
	(IS_DEV_TYPE_11V_T(_type) || IS_DEV_TYPE_11V_NT(_type))

/* MLD Type */
enum bmgr_mld_type {
	BMGR_MLD_TYPE_NONE,		/* non-MLD */
	BMGR_MLD_TYPE_SINGLE,	/* single-link MLD */
	BMGR_MLD_TYPE_MULTI,	/* multi-link MLD */
};

/*
 * Medium Synchronization Delay Information
 */
struct med_sync_info {
	u8 en;
	u8 dur;
	u8 ofdm_ed_thr;
	u8 max_txop;
};

/*
 * EML Capabilities
 */
struct eml_caps_info {
	u8 en;
	u8 emlsr_supp;
	u8 eml_padding_delay;
	u8 eml_trans_delay;
	u8 emlmr_supp;
	u8 trans_to;
	u8 eml_omn_en; /* set AP whether to response EML OMN frame from STA */
};

/*
 * MLD Capabilities
 */
struct mld_caps_info {
	u8 en;
	u8 srs_supp;
	u8 t2l_nego_supp;
	u8 freq_sep_str;
};

/** struct bmgr_mld_attr - Attributes of MLD
 *
 * This struct maintains the attr of MLD by BSS/MLD Manager
 *
 * @param med_sync Medium Synchonization Delay subfeild
 * @param eml_caps EML Capabilities subfield
 * @param mld_caps MLD Capabilities And Operations subfield
 */
struct bmgr_mld_attr {
	struct med_sync_info med_sync;
	struct eml_caps_info eml_caps;
	struct mld_caps_info mld_caps;
};

/** struct bmgr_reg_info - register information of BSS
 *
 * This struct used to bring the register information of BSS from Logan to BSS Manager
 *
 */
struct bmgr_reg_info {
	u8		dev_type;
	u8		dev_cfg_idx;		/* pAd->dev_idx */
	u8		dot11v_mbssid_idx;
	u8		hw_bss_index;		/* index sync to fw (per-NIC) */
	u8		max_bssid_indicator;
	u8		dtim_period;
	u8		bssid[MAC_ADDR_LEN];
	u16		phy_mode;
	u16		cap_info;
	u8		channel;
	u8		op_class;
	u8		ssid[MAX_LEN_OF_SSID+1];
	u8		ssid_len;
	bool	is_hide_ssid;
	u16		bcn_interval;
	u8		rnr_6g_rule;
	u8		single_rnr;	/*try to integret all rnr info*/

	/* security */
	u32		auth_mode;
	u32		PairwiseCipher;
	u32		GroupCipher;

	/* eht and mlo */
	u8		mld_grp;
	bool	mld_addr_by_cfg;
	u8		pf_mld_addr[MAC_ADDR_LEN];
	u8		tid_map;

	/* timestamp info */
	u32		reg_time;		/* device register time */
	u32		reg_hw_tsf[2];	/* register hw tsf */

	/* bss manager event handler (bmgr->drv) */
	struct bss_mngr_event_ops *event_ops;

	struct _BSS_STRUCT *t_bss;
};

/** struct bmgr_update_info - updated information of BSS
 *
 * This struct used to deliver the information of BSS from BSS/MLD Manager to Logan
 *
 */
struct bmgr_update_info {
	u8		ssid[MAX_LEN_OF_SSID+1];
	u8		ssid_len;

	/* eht and mlo */
	struct bmgr_mld_attr mld_attr;
};

/*
 * mbss ie query
 */
struct mbss_query_info {
	u8 is_probe_rsp;	/* 0: beacon, 1: probe.rsp */
	u32 query_bitmap;	/*for probe rsp, choose take which multip bssid ie*/
	u16 tim_ie_offset[MAX_BEACON_NUM];
	u16 mlt_ie_offset[MAX_BEACON_NUM];
	u16 cap_info_offset[MAX_BEACON_NUM];
	/* TODO: MBSS + Reconfig IE Offset */
	// u16 reconfig_tmr_offset[MAX_BEACON_NUM];
	u8 *ml_ie;			/* input:  request ie */
	u16 f_len;			/* output: ie length */
	u8 *f_buf;			/* output: ie buf prt */
};

struct ml_ie_link {
	u8		link_id;
	u8		ch_band;
	u8		channel;

	/* generic */
	bool	active;
	u16		sta_ctrl;
	u8		sta_info_len;
	u8		link_addr[MAC_ADDR_LEN];

	/* basic ml ie */
	u16		bcn_interval;
	u8		dtim_count;
	u8		dtim_period;
	u16		nstr_bmap;
	u8		sta_profile[BSS_MNGR_MAX_PER_STA_PROFILE_LEN];
	u16		sta_profile_len; /* for both sub-IE defragment and store STA Profile */

	/* reconfiguration ml ie */
	u8		reconfig_type;
};

struct ml_ie_info {
	u8		type;
	u16		pres_bmap;
	u8		mld_addr[MAC_ADDR_LEN];
	u8		link_id;
	u8		bss_chg_cnt;
	u16		md_sync_delay;
	u16		eml_caps;
	u8		emlsr_sup;
	u16		mld_caps;
	u16		ext_mld_caps;
	u8		cmm_info_len;
	u8		mld_id; /* 11be AP MLD ID */
	u8		rsvd[3];
	struct ml_ie_link link[BSS_MNGR_MAX_BAND_NUM];
};

/*
 * MLD query types
 */
enum {
	BMGR_QUERY_ML_AP_BASIC = 0,
	BMGR_QUERY_ML_IE_BCN,
	BMGR_QUERY_ML_IE_PROBE_REQ,
	BMGR_QUERY_ML_IE_PROBE_RSP,
	BMGR_QUERY_ML_IE_AUTH_REQ,
	BMGR_QUERY_ML_IE_AUTH_RSP,
	BMGR_QUERY_ML_IE_ASSOC_REQ,
	BMGR_QUERY_ML_IE_ASSOC_RSP,
	BMGR_QUERY_ML_STA,
	BMGR_QUERY_ML_PARSING_IE,
	BMGR_QUERY_MLD,
	BMGR_QUERY_BSSID_ML_INFO,
	BMGR_QUERY_MLD_CONN,
};

/*
 * MLD IE build types
 */
enum {
	BMGR_MLD_BCN_COMMON = 0,
	BMGR_MLD_PROBE_REQ,
	BMGR_MLD_PROBE_RSP_PARTIAL_PROFILE, /* TODO */
	BMGR_MLD_PROBE_RSP_COMPLETE_PROFILE,
	BMGR_MLD_AUTH_COMMON,
	BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE,
	BMGR_MLD_COMMON_LINK_INFO,
	BMGR_MLD_APPL_ELEM_STA_PROFILE, /* csa, ext_csa, max_chn_sw_time, quiet, quiet_chn */
};

/* BMGR_QUERY_MLD: query info of existed MLD */
struct query_mld_basic {
	u8 addr[MAC_ADDR_LEN];	/* input: MLD addr of queried MLD */
	u8 mld_grp_idx;			/* output: */
	u8 mld_type;			/* output: */
	u8 link_cnt;			/* output: #links in MLD */
};

/* BMGR_QUERY_ML_AP_BASIC / BMGR_QUERY_BSSID_ML_INFO */
struct query_mld_ap_basic {
	u8 mld_grp_idx;
	u8 link_cnt;
	u8 link_id;
	u8 addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN]; /* input for BMGR_QUERY_BSSID_ML_INFO only */
	struct wifi_dev *agent_bss_wdev;
};

struct csa_in_ml_ie {
	u16 csa_offset_in_ml_ie;	/* output: csa_ie offset in ml IE */
	u8 csa_bss_idx;		/* output: bss_idx of the csa in ml IE */
};

/* BMGR_QUERY_ML_IE */
struct query_ml_ie {
	u8 *ml_ie;			/* input:  request ie */
	struct frag_ie_info *ml_frag_info; /* input:  request fraged ie info */
	u16 mld_sta_idx;	/* input:  request mld sta */
	u16 f_len;			/* output: ie length */
	u8 *f_buf;			/* output: ie buf prt */
	struct csa_in_ml_ie csa_ie;		/* output: csa related in ml IE */
};

/* BMGR_QUERY_ML_STA */
struct query_mld_sta {
	u16	mld_sta_idx;			/* input:  mld_sta_idx */
	u16	aid;					/* output: aid */
	u8	mld_addr[MAC_ADDR_LEN];	/* output: sta mld addr */
};

/* BMGR_QUERY_ML_PARSING_IE */
struct query_mld_parse_ie {
	u8 *ml_ie;					/* input:  request ie */
	struct frag_ie_info *ml_frag_info; /* input:  request fraged ie info */
	struct ml_ie_info *ml_info;	/* output: ml_ie info */
};

/* BMGR_QUERY_MLD_CONN */
struct query_mld_conn {
	u8 mld_group_idx;	/* input: queried self MLD */
	u16 mld_sta_num;	/* output: number of MLD STA associated to queried MLD */
	u8 mld_sta_info[0];	/* output: struct query_mld_sta,
					info of MLD STA that associated with queried MLD */
};

struct query_mld_info {
	u8 query_type;						/* query type */

	union {
		struct query_mld_basic *mld_basic;	/* MLD */
		struct query_mld_ap_basic *basic;	/* MLD_AP_BASIC */
		struct query_ml_ie ie;			/* ML_IE */
		struct query_mld_sta *sta;		/* MLD_STA */
		struct query_mld_conn *mld_conn;	/* MLD_STA(s) that associated to MLD_AP */
		struct query_mld_parse_ie parse_ie;	/* MLD_PARSING_IE */
	};
};

/*
 * MLD connection request
 */
struct mld_conn_req {
	u8 req_type;		/* in:  requested from auth/assoc */
	u8 *ml_ie;			/* in:  requested ml ie */
	u8 *t2l_ie;			/* in:  requested tid-to-link ie */
	u16 mld_sta_idx;	/* out: mld sta index */
	u8 status;			/* out: status */
	u8 *link_addr;		/* in: link addr */
	u8 mld_addr[MAC_ADDR_LEN];
	struct frag_ie_info *ml_frag_info; /* in:  requested fragmented ml ie info */
	u8 ch_band;
};

/*
 * MLD EML Op Mode
 */
struct mld_eml_op {
	u16 mld_sta_idx;
	u8 eml_omn_en;		/* set AP whether to response EML OMN frame from STA */
	u8 emlsr_en;
	u8 emlmr_en;
	u8 dialog_token;
	u8 emlsr_param_upd_ctrl;
	u16 eml_link_bmap;
	void *priv_ptr;		/* pEntry */
};

/*
 * MLD Link Recommendation Frame
 */
struct mld_recomm_sta_info {
	bool valid;			/* sanity */
	u16 mld_sta_idx;
	u16 aid;
	u16 recomm_links;	/* bitmap for recommended link_id */
};

struct mld_link_recomm {
	u16 reason_code;
	u16 mld_sta_num; /* number of MLD STA to be recommended */
	u8 mld_sta_info[0]; /* struct mld_recomm_sta_info,
							info of recommended MLD STA */
};

/*
 * MLD Reconfiguration ML IE Operation Info
 */
struct mld_reconfig_ie_build_t {
	bool rm_links[BSS_MNGR_MAX_BAND_NUM];
	u16 tmr[BSS_MNGR_MAX_BAND_NUM]; /* number of seconds */
};

struct mld_reconfig_ie_clean_t {
	bool clean_links[BSS_MNGR_MAX_BAND_NUM]; /* input */
};

struct mld_reconfig_ie_query_t {
	u16 f_len;			/* output: ie length */
	u8 *f_buf;			/* output: ie buf prt */

	u8 fw_bss_idx[BSS_MNGR_MAX_BAND_NUM]; /* output: offset belongs to which bss */
	u16 tmr_offset[BSS_MNGR_MAX_BAND_NUM]; /* output: offset from IE head to Per-link Timer */
};

struct mld_reconfig_ie_op {
	u8 op;						/* enum _BMGR_RECONFIG_IE_OP */
	u8 mld_grp_idx;

	union {
		struct mld_reconfig_ie_build_t *reconf_build; /* BMGR_RECONFIG_IE_OP_BUILD */
		struct mld_reconfig_ie_clean_t *reconf_clean; /* BMGR_RECONFIG_IE_OP_CLEAN_LINK */
		struct mld_reconfig_ie_query_t *reconf_query; /* BMGR_RECONFIG_IE_OP_QUERY */
	};
};

/*
 * Query net_dev by band
 */
struct query_band_netdev {
	u8 band;						/* in: queried band */
	PNET_DEV pNetDev;				/* out: */
};

/*
 * Query netdev by group && link
 */
struct query_group_link_netdev {
	u8 group_id;						/* in: queried group */
	u8 link_id;						/* in: queried group link */
	PNET_DEV pNetDev;				/* out: */
};

/*
 * MLD disconnect op mode
 */
enum {
	MLD_DISC_OP_ACT_ONLY = 0,		/* MLD StaRec down */
	MLD_DISC_OP_DEL_STA_N_ACT = 1,	/* Delete MLD's MacTableEntries */
	MLD_DISC_OP_DEL_STA_LINK = 2,	/* Delete a MacTableEntry */
};

/*
 * Bss Parameter Critical Update (indicate BSS update scope)
 * BMGR_CRITICAL_UPDATE_SINGLE:
 *		update single BSS BPCC and set MLD Critical Update Flag
 * BMGR_CRITICAL_UPDATE_ALL:
 *		update all BSS's BPCC in the same band and set Critical Update Flag of each MLD
 * BMGR_CRITICAL_UPDATE_MLD_CUF:
 *		set Critical Update Flag of links in an MLD (w/o BPCC)
 */
enum {
	BMGR_CRITICAL_UPDATE_SINGLE = 0,
	BMGR_CRITICAL_UPDATE_ALL,
	BMGR_CRITICAL_UPDATE_MLD_CUF,
	BMGR_CRITICAL_UPDATE_MAX_NUM,
};

/*
 * Operation for Reconfiguration Multi-link IE function
 */
enum _BMGR_RECONFIG_IE_OP {
	BMGR_RECONFIG_IE_OP_BUILD,
	BMGR_RECONFIG_IE_OP_CLEAN,
	BMGR_RECONFIG_IE_OP_CLEAN_LINK,
	BMGR_RECONFIG_IE_OP_QUERY,
};

/*
 * Bss Info RFBand idx Update
 */
enum {
	CMD_BAND_2G4 = 1,
	CMD_BAND_5G,
	CMD_BAND_6G,
};

struct bss_mngr_ops {
	int (*bmgr_init)(void);
	int (*bmgr_deinit)(void);
	int (*bmgr_bss_reg)(PNET_DEV pNetDev, struct bmgr_reg_info *reg_info);
	int (*bmgr_bss_dereg)(PNET_DEV pNetDev);
	int (*bmgr_bss_update)(PNET_DEV pNetDev, struct bmgr_reg_info *reg_info);
	int (*bmgr_show_info)(u8 type);
	int (*bmgr_ie_update)(PNET_DEV pNetDev);
	int (*bmgr_sync_bcn_update)(PNET_DEV pNetDev);
	void *(*bmgr_mld_get_pad_by_band_idx)(u8 bss_idx);
#ifdef DOT11V_MBSSID_SUPPORT
	int (*bmgr_query_mbss_info)(PNET_DEV pNetDev, struct mbss_query_info *mbss_info);
#endif
	int (*bmgr_query_rnr_info)(PNET_DEV pNetDev, u8 *f_buf, u16 *f_len);
	bool (*bmgr_query_is_repted_ssid)(PNET_DEV pNetDev, u32 short_ssid, u8 *ssid, u8 ssid_len);
	bool (*bmgr_query_is_mld_wdev)(PNET_DEV pNetDev);
	bool (*bmgr_query_is_in_mlo_csa)(PNET_DEV pNetDev);
#ifdef CONFIG_6G_SUPPORT
	int (*bmgr_update_oob_6g_disc)(PNET_DEV pNetDev, u8 rnr_6g_rule);
#endif
	int (*bmgr_query_netdev_by_band)(struct query_band_netdev *query_info);
#ifdef DOT11_EHT_BE
	int (*bmgr_mld_group_create)(PNET_DEV pNetDev, u8 *mld_grp, u8 *mld_addr, struct bmgr_mld_attr *mld_attr, u8 mld_type);
	int (*bmgr_mld_group_destroy)(PNET_DEV pNetDev, u8 mld_grp);
	int (*bmgr_mld_group_attr_set)(u8 mld_grp, u32 attr_pres_bmap, struct bmgr_mld_attr *mld_attr, u8 cfg_disconn);
	int (*bmgr_mld_group_add_link)(PNET_DEV pNetDev, u8 mld_grp);
	int (*bmgr_mld_group_del_link)(PNET_DEV pNetDev);
	int (*bmgr_query_mld_info)(PNET_DEV pNetDev, struct query_mld_info *mld_info);
	int (*bmgr_query_tid_to_link_info)(PNET_DEV pNetDev, u8 *f_buf, u16 *f_len);
	int (*bmgr_query_multi_link_traffic_ie)(PNET_DEV pNetDev, u8 *f_buf, u16 *f_len);
	int (*bmgr_mld_reconfig_ie_op)(struct mld_reconfig_ie_op *ie_op);
	int (*bmgr_mld_query_netdev_by_link)(struct query_group_link_netdev *query_info);
	int (*bmgr_mld_conn_req)(PNET_DEV pNetDev, struct mld_conn_req *conn_req);
	int (*bmgr_mld_conn_act)(PNET_DEV pNetDev, u16 mld_sta_idx);
	int (*bmgr_mld_disconn_op)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 op);
	void (*bmgr_mld_add_sta_profile)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 link_id, u8 frame_mld_type, u8 *buf, u16 buf_len);
	bool (*bmgr_mld_remove_sta_profile)(PNET_DEV pNetDev, u8 frame_mld_type, u8 *elmt_id_arr, u16 id_arr_len);
	void (*bmgr_mld_sync_ml_probe_rsp_per_sta_profile)(PNET_DEV pNetDev);
	void (*bmgr_mld_ba_ori_setup)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 TID, u16 TimeOut);
	u8 (*bmgr_mld_ba_resrc_ori_add)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 TID, u16 ori_ba_wsize, u8 amsdu_en, u16 TimeOut);
	u8 (*bmgr_mld_ba_resrc_rec_add)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 TID, u16 TimeOut, u16 StartSeq, u16 rec_ba_wsize);
	void (*bmgr_mld_ba_resrc_ori_del)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 TID, u8 bPassive);
	void (*bmgr_mld_ba_resrc_rec_del)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 TID, u8 bPassive);
	void (*bmgr_mld_ba_add_to_asic)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 TID, u16 Seq, u16 BAWinSize, int type, u8 amsdu_en);
	void (*bmgr_mld_ba_del_from_asic)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 TID, int type, u8 amsdu_en);
	void (*bmgr_mld_forward_action_to_link)(PNET_DEV pNetDev, struct _MLME_QUEUE_ELEM *elem);
	u8 (*bmgr_mld_breakdown_action_frame)(PNET_DEV pNetDev, u16 mld_sta_idx, struct _MLME_QUEUE_ELEM *elem);
	void (*bmgr_mld_aggregate_action_frame)(PNET_DEV pNetDev, struct frame_itwt_setup *itwt_setup_frame, struct MLO_TWT_PRIV_T *priv);
	void (*bmgr_mld_twt_wait_ack_inform)(PNET_DEV pNetDev, u16 mld_sta_idx, u8 action_linkid, u16 all_linkid_bitmap, u8 wait_ack_sts);
#ifdef MWDS
	u8 (*bmgr_mld_mwds_cap_sync)(PNET_DEV pNetDev, u8 enable);
#endif
	void (*bmgr_mld_critical_update_trigger)(PNET_DEV pNetDev, u8 upd_scope, u8 bcn_reason);
	BOOLEAN (*bmgr_mld_critical_update_op_lock)(PNET_DEV pNetDev, u8 lock_flag);
	void (*bmgr_mld_eml_op_update)(PNET_DEV pNetDev, struct mld_eml_op *eml_op);
	int (*bmgr_mld_bss_linkup)(PNET_DEV pNetDev);
	int (*bmgr_mld_bss_linkdown)(PNET_DEV pNetDev);
	int (*bmgr_mld_bmc_sn_qry)(PNET_DEV pNetDev, u8 *mld, u8 *agent_bss, u16 *sn, PNET_DEV *if_dev_list);
	void (*bmgr_mld_tx_link_recomm)(u8 mld_grp_idx, u16 tx_link_bmap, struct mld_link_recomm *link_recomm);
	int (*bmgr_mld_reconfig_peer_mld)(u8 mld_grp_idx, u16 rm_link_id_bitmap);
#endif
};

struct ie {
	u8 *buf;
	u16 len;
};

struct mld_link_add {
	bool	active;
	bool	is_setup_link;
	u8		sta_addr[MAC_ADDR_LEN];

	u32		tid_map;
	u8		str_map;		/* supported link bitmap */
	u32		cap;			/* TBD */

	void	*priv_ptr;		/* pEntry */
};

struct mld_sta_add {
	/* mld sta idx, assigned by bmgr */
	u16	mld_sta_idx;

	/* mld aid, assigned by bmgr */
	u16	aid;

	/* link_id of bss to be created */
	u8	link_id;

	u8	mld_addr[MAC_ADDR_LEN];
	u16	eml_caps;	/* EML Capabilities subfield in Basic ML IE */
	u8	emlmr;
	u8	emlsr;
	u8	total_link_cnt;			/* setup link count */

	/* per-link info */
	struct mld_link_add	add_link[MLD_LINK_MAX];
};

struct mld_link_del {
	u8		sta_addr[MAC_ADDR_LEN];
	void	*priv_ptr;		/* pEntry */
};

struct mld_ba_info {
	u8	TID;
	u8	amsdu_en;
	u8	bPassive;
	u16	TimeOut;
	u16	BAWinSize;
	u16	Seq;
	int	type;
	void	*priv_ptr;
};

/* critical update */
struct mld_crit_upd_info {
	u32 bitmap;
	u32 bypass_bcn_seq_bmap;
};

struct bss_mngr_event_ops {
	int (*bmgr_event_bcn_update)(PNET_DEV pNetDev);
	int (*bmgr_event_bss_attr_update)(PNET_DEV pNetDev, struct bmgr_update_info *update_info, bool update_bcn);
	PUCHAR (*bmgr_event_ie_fragment)(u8 *ie_buf, u16 *buf_len);
#ifdef DOT11_EHT_BE
	int (*bmgr_event_add_mld)(PNET_DEV pNetDev, u8 mld_grp, u8 *mld_addr, u8 mld_type);
	int (*bmgr_event_del_mld)(PNET_DEV pNetDev, u8 mld_grp);
	int (*bmgr_event_mld_add_link)(PNET_DEV pNetDev, u8 mld_grp);
	int (*bmgr_event_mld_del_link)(PNET_DEV pNetDev);
	int (*bmgr_event_mld_add_sta)(PNET_DEV pNetDev, struct mld_sta_add *add_sta);
	int (*bmgr_event_mld_conn_act)(PNET_DEV pNetDev, struct mld_link_add *add_link);
	int (*bmgr_event_mld_del_sta)(struct mld_link_del *del_link);
	int (*bmgr_event_mld_disconn_mld_sta)(struct mld_link_del *del_link);
	int (*bmgr_event_mld_disconn_act)(struct mld_link_del *del_link);
	void (*bmgr_event_mld_query_probe_rsp_frame)(PNET_DEV pNetDev, struct ie *rsp_frm);
	void (*bmgr_event_mld_form_ml_probe_rsp_sta_pf)(PNET_DEV pNetDev, struct ie *repting_rsp_frm, u8 repting_link_id);
	void (*bmgr_event_mld_ba_ori_setup)(PNET_DEV pNetDev, struct mld_ba_info *ba_info);
	u8 (*bmgr_event_mld_ba_resrc_ori_add)(PNET_DEV pNetDev, struct mld_ba_info *ba_info);
	u8 (*bmgr_event_mld_ba_resrc_rec_add)(PNET_DEV pNetDev, struct mld_ba_info *ba_info);
	void (*bmgr_event_mld_ba_resrc_ori_del)(PNET_DEV pNetDev, struct mld_ba_info *ba_info);
	void (*bmgr_event_mld_ba_resrc_rec_del)(PNET_DEV pNetDev, struct mld_ba_info *ba_info);
	void (*bmgr_event_mld_ba_add_to_asic)(PNET_DEV pNetDev, struct mld_ba_info *ba_info);
	void (*bmgr_event_mld_ba_del_from_asic)(PNET_DEV pNetDev, struct mld_ba_info *ba_info);
	void (*bmgr_event_mld_twt_action_frm_forward)(struct _MLME_QUEUE_ELEM *elem, struct _MAC_TABLE_ENTRY *peer_entry);
	u8 (*bmgr_event_mld_twt_action_frm_breadown)(PNET_DEV da_net_dev, PNET_DEV sa_net_dev, u8 da_linkid, u8 sa_linkid,
		u16 mld_sta_idx, u8 action_linkid, u16 linkid_bitmap, u16 all_linkid_bitmap,
		struct _MLME_QUEUE_ELEM *elem, struct mlo_itwt_ie *twt_ie, struct _MAC_TABLE_ENTRY *peer_entry);
	void (*bmgr_event_mld_twt_action_frm_agg)(PNET_DEV pNetDev, u8 *out_buffer, u16 len, struct _MAC_TABLE_ENTRY *peer_entry,
		u16 mld_sta_idx, u8 action_linkid, u16 all_linkid_bitmap);
	void (*bmgr_event_mld_twt_wait_ack_inform)(PNET_DEV pNetDev, struct _MAC_TABLE_ENTRY *peer_entry, u8 wait_ack_sts);
#ifdef MWDS
	int (*bmgr_event_mld_mwds_enable)(PNET_DEV pNetDev, u8 enable);
#endif
	void (*bmgr_event_mld_critical_update_bcn)(PNET_DEV pNetDev, struct mld_crit_upd_info *info, u8 bcn_reason);
	u8 (*bmgr_event_mld_critical_update_op_lock)(PNET_DEV pNetDev, u8 lock, u8 mld_id);
	void (*bmgr_event_mld_eml_op_update)(PNET_DEV pNetDev, struct mld_eml_op *eml_op);
	void (*bmgr_event_mld_tx_link_recomm)(PNET_DEV pNetDev, u16 reason_code, u8 *buf, u16 buf_len);
	int (*bmgr_event_mld_reconfig)(u8 *mld_addr, u16 available_link_id_bitmap);
#endif
};

NDIS_STATUS bss_mngr_init(void);
NDIS_STATUS bss_mngr_deinit(void);
NDIS_STATUS bss_mngr_show_info(uint8_t type);
NDIS_STATUS bss_mngr_dev_reg(IN struct wifi_dev *wdev);
NDIS_STATUS bss_mngr_dev_dereg(IN struct wifi_dev *wdev);
NDIS_STATUS bss_mngr_dev_update(IN struct wifi_dev *wdev);
NDIS_STATUS bss_mngr_ie_update(IN struct wifi_dev *wdev);
void bss_mngr_sync_bcn_update(struct wifi_dev *wdev);
struct _RTMP_ADAPTER *bss_mngr_mld_get_pad_by_band(struct wifi_dev *wdev, UCHAR band_idx);

#ifdef DOT11V_MBSSID_SUPPORT
u16 bss_mngr_query_mbssid_ie(struct wifi_dev *wdev, struct mbss_query_info *mbss_query);
#endif
u16 bss_mngr_query_rnr_ie(struct wifi_dev *wdev, u8 *f_buf);
bool bss_mngr_query_is_repted_ssid(struct wifi_dev *wdev, u32 short_ssid, u8 *ssid, u8 ssid_len);
#ifdef CONFIG_6G_SUPPORT
NDIS_STATUS bss_mngr_update_oob_6g_disc(struct wifi_dev *wdev, u8 rnr_6g_rule);
#endif
struct wifi_dev *bss_mngr_query_wdev_by_band(u8 band);
#ifdef DOT11_EHT_BE
NDIS_STATUS bss_mngr_mld_group_create(struct wifi_dev *wdev, u8 *mld_grp, u8 *mld_addr, struct eht_mld_param *mld_param, u8 mld_type);
NDIS_STATUS bss_mngr_mld_group_destroy(struct wifi_dev *wdev, u8 mld_grp);
NDIS_STATUS bss_mngr_mld_group_attr_set(u8 mld_grp, struct eht_mld_op_ctrl *op_ctrl);
NDIS_STATUS bss_mngr_mld_group_add_link(struct wifi_dev *wdev, u8 mld_grp);
NDIS_STATUS bss_mngr_mld_group_del_link(struct wifi_dev *wdev);
int bss_mngr_peer_integrity_check(struct _MAC_TABLE_ENTRY *entry);
u16 bss_mngr_query_mld_info(struct wifi_dev *wdev, struct query_mld_info *mld_query);
u16 bss_mngr_query_tid_to_link_ie(struct wifi_dev *wdev, u8 *f_buf);
u16 bss_mngr_query_multi_link_traffic_ie(struct wifi_dev *wdev, u8 *f_buf_hdr, u8 *f_buf);
int bss_mngr_mld_reconfig_ie_build(u8 mld_grp_idx, struct mld_reconfig_ie_build_t *build);
int bss_mngr_mld_reconfig_ie_clean(u8 mld_grp_idx);
int bss_mngr_mld_reconfig_ie_clean_link(u8 mld_grp_idx, struct mld_reconfig_ie_clean_t *clean);
u16 bss_mngr_mld_reconfig_ie_query(u8 mld_grp_idx, struct mld_reconfig_ie_query_t *query);
struct wifi_dev *bss_mngr_query_group_wdev_by_link(u8 group_id, u8 link_id);
bool bss_mngr_is_wdev_in_mlo_group(struct wifi_dev *wdev);
bool bss_mngr_is_in_mlo_csa(struct wifi_dev *wdev);
u16 bss_mngr_mld_bss_linkup(struct wifi_dev *wdev);
u16 bss_mngr_mld_bss_linkdown(struct wifi_dev *wdev);
u16 bss_mngr_mld_bmc_sn_qry(struct wifi_dev *wdev, u8 *mld, u8 *agent_bss, u16 *sn, struct wifi_dev **wdev_list);
u16 bss_mngr_mld_conn_req(struct wifi_dev *wdev, struct mld_conn_req *mld_conn);
u16 bss_mngr_mld_conn_act(struct wifi_dev *wdev, u16 mld_sta_idx);
u16 bss_mngr_mld_disconn_op(struct wifi_dev *wdev, u16 mld_sta_idx, u8 op);
int bss_mngr_query_mld_basic(struct query_mld_basic *mld_basic);
int bss_mngr_query_mld_ap_basic(struct wifi_dev *wdev, struct query_mld_ap_basic *basic);
int bss_mngr_query_mld_by_bssid(struct query_mld_ap_basic *basic);
u16 bss_mngr_query_mld_sta_aid(struct wifi_dev *wdev, u16 mld_sta_idx);
int bss_mngr_query_mld_conn(struct query_mld_conn *mld_conn);
void bss_mngr_mld_add_sta_profile(struct wifi_dev *wdev, u16 mld_sta_idx, u8 link_id, u8 frame_mld_type, u8 *buf, u16 buf_len);
bool bss_mngr_mld_remove_sta_profile(
	struct wifi_dev *wdev,
	u8 frame_mld_type,
	u8 *elmt_id_arr,
	u16 id_arr_len
);
void bss_mngr_mld_sync_ml_probe_rsp(struct wifi_dev *wdev);
void bss_mngr_mld_ba_ori_setup(struct wifi_dev *wdev, u16 mld_sta_idx, u8 TID, u16 TimeOut);
u8 bss_mngr_mld_ba_resrc_ori_add(struct wifi_dev *wdev, u16 mld_sta_idx, u8 TID, u16 ori_ba_wsize, u8 amsdu_en, u16 TimeOut);
u8 bss_mngr_mld_ba_resrc_rec_add(struct wifi_dev *wdev, u16 mld_sta_idx, u8 TID, u16 TimeOut, u16 StartSeq, u16 rec_ba_wsize);
void bss_mngr_mld_ba_resrc_ori_del(struct wifi_dev *wdev, u16 mld_sta_idx, u8 TID, u8 bPassive);
void bss_mngr_mld_ba_resrc_rec_del(struct wifi_dev *wdev, u16 mld_sta_idx, u8 TID, u8 bPassive);
void bss_mngr_mld_ba_add_to_asic(struct wifi_dev *wdev, u16 mld_sta_idx, u8 TID, u16 Seq, u16 BAWinSize, int type, u8 amsdu_en);
void bss_mngr_mld_ba_del_from_asic(struct wifi_dev *wdev, u16 mld_sta_idx, u8 TID, int type, u8 amsdu_en);
void bss_mngr_mld_twt_action_frm_forward(struct _RTMP_ADAPTER *ad, struct _MLME_QUEUE_ELEM *elem);
u8 bss_mngr_mld_twt_action_frm_breadown(struct _RTMP_ADAPTER *ad, struct _MLME_QUEUE_ELEM *elem);
void bss_mngr_mld_twt_action_frm_agg(struct wifi_dev *wdev, struct frame_itwt_setup *itwt_setup_frame, struct MLO_TWT_PRIV_T *priv);
void bss_mngr_mld_twt_wait_ack_inform(struct wifi_dev *wdev, u16 mld_sta_idx, u8 action_linkid, u16 all_linkid_bitmap, u8 wait_ack_sts);
#ifdef MWDS
u8 bss_mngr_mld_mwds_enable(struct wifi_dev *wdev, u8 enable);
#endif
void bss_mngr_mld_critical_update_trigger(struct wifi_dev *wdev, u8 upd_scope, u8 bcn_reason);
BOOLEAN bss_mngr_mld_critical_update_op_lock(struct wifi_dev *wdev, u8 lock_flag);
void bss_mngr_mld_eml_op_update(struct wifi_dev *wdev, struct mld_eml_op *eml_op);
void bss_mngr_mld_tx_link_recomm(u8 mld_grp_idx, u16 tx_link_bmap, struct mld_link_recomm *link_recomm);
void bss_mngr_mld_reconfig_peer_mld(u8 mld_grp_idx, u16 rm_link_id_bitmap);
#endif


/************************ BSS Manager Internal ************************/

extern struct bss_mngr_event_ops bmgr_event_ops;

/* max bss count maintained by bss manager*/
#define BMGR_MAX_BSS_CNT		65
/* number of u32 BSS bitmaps for BMGR_MAX_BSS_CNT */
#define BMGR_U32_BSS_BITMAP_NUM ((BMGR_MAX_BSS_CNT + 31) / 32)
/* max mld station count */
#define BMGR_MAX_MLD_STA_CNT	256
#define BMGR_INVALID_BSS_IDX	(BMGR_MAX_BSS_CNT - 1)
/* */
#define TWT_ACT_FTM_AGG_TIME    5000 /* 500ms */

/* valid MLD group starts from 1 */
#define BMGR_VALID_MLD_GRP_IDX(_idx) \
	((_idx < BMGR_MAX_MLD_GRP_CNT) && \
	(_idx != BMGR_INVALID_MLD_GRP) && \
	(_idx != 0))

#define BMGR_VALID_BSS_IDX(_idx) \
	((_idx < BMGR_MAX_BSS_CNT) && \
	(_idx != BMGR_INVALID_BSS_IDX))

#define BMGR_VALID_MLD_STA(_idx) \
	(_idx < BMGR_MAX_MLD_STA_CNT)

#define BMGR_VALID_BSS_ENTRY(_entry) \
	(_entry && _entry->valid)

#define BMGR_VALID_MLO_BSS_ENTRY(_entry) \
	(BMGR_VALID_BSS_ENTRY(_entry) && _entry->mlo_en)

#define BMGR_VALID_MLO_DEV(_mld) \
	(_mld && _mld->valid)

/* BSS Bitmap bit-wise operation */
#define BMGR_U32_BITMAP_SET(_bitmap, _bit) (_bitmap[_bit / 32] |= (1 << (_bit % 32)))
#define BMGR_U32_BITMAP_CLEAR(_bitmap, _bit) (_bitmap[_bit / 32] &= ~(1 << (_bit % 32)))
#define BMGR_U32_BITMAP_IS_ASSIGNED(_bitmap, _bit) (_bitmap[_bit / 32] & (1 << (_bit % 32)))


#ifndef SET_CAP_BITS
#define SET_CAP_BITS(_cap, _capinfo, _value)\
{\
	_capinfo &= ~_cap##_MASK;\
	_capinfo |= ((_value << _cap##_SHIFT)\
			& _cap##_MASK);\
}
#endif

/* TBTT Information Header */
#define DOT11_RNR_TBTT_INFO_HDR_TYPE_SHIFT 0
#define DOT11_RNR_TBTT_INFO_HDR_TYPE_MASK 0x3
#define DOT11_RNR_TBTT_INFO_HDR_FLT_NAP_SHIFT 2
#define DOT11_RNR_TBTT_INFO_HDR_FLT_NAP_MASK (1 << 2)
#define DOT11_RNR_TBTT_INFO_HDR_CO_LOC_AP_SHIFT 3
#define DOT11_RNR_TBTT_INFO_HDR_CO_LOC_AP_MASK (1 << 3)
#define DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT_SHIFT 4
#define DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT_MASK (0xF << 4)
#define DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN_SHIFT 8
#define DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN_MASK (0xFF << 8)
#define MLD_PARAM_MLD_ID_MASK				BITS(0, 7)
#define MLD_PARAM_LINK_ID_MASK				BITS(8, 11)
#define MLD_PARAM_LINK_ID_SHIFT				8
#define MLD_PARAM_BSS_PARAM_CHANGE_COUNT_MASK		BITS(12, 19)
#define MLD_PARAM_BSS_PARAM_CHANGE_COUNT_SHIFT		12


#define SET_DOT11_RNR_TBTT_INFO_HDR_TYPE(_capinfo, _value) \
	SET_CAP_BITS(DOT11_RNR_TBTT_INFO_HDR_TYPE, _capinfo, _value)
#define SET_DOT11_RNR_TBTT_INFO_HDR_FLT_NAP(_capinfo, _value) \
	SET_CAP_BITS(DOT11_RNR_TBTT_INFO_HDR_FLT_NAP, _capinfo, _value)
#define SET_DOT11_RNR_TBTT_INFO_HDR_CO_LOC_AP(_capinfo, _value) \
	SET_CAP_BITS(DOT11_RNR_TBTT_INFO_HDR_CO_LOC_AP, _capinfo, _value)
#define SET_DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT(_capinfo, _value) \
	SET_CAP_BITS(DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT, _capinfo, _value)
#define SET_DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN(_capinfo, _value) \
	SET_CAP_BITS(DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_LEN, _capinfo, _value)

/* BSS Parameters */
#define TBTT_INFO_BSS_PARAM_OCT_RECOM BIT(0)
#define TBTT_INFO_BSS_PARAM_SAME_SSID BIT(1)
#define TBTT_INFO_BSS_PARAM_MULTI_BSSID BIT(2)
#define TBTT_INFO_BSS_PARAM_TX_BSSID BIT(3)
#define TBTT_INFO_BSS_PARAM_MBR_ESS_2G_5G_CO_LOC_AP BIT(4)
#define TBTT_INFO_BSS_PARAM_UNSOL_PROBE_RSP BIT(5)
#define TBTT_INFO_BSS_PARAM_CO_LOC_AP BIT(6)

/* Neighbor AP Information */
struct GNU_PACKED neighbor_ap_info {
	u16 tbtt_info_hdr;
	u8 op_class;
	u8 ch_num;
	u8 tbtt[0];
};

/*rnr tbtt mld_parameters*/
struct GNU_PACKED rnr_tbtt_mld_param {
#ifdef CFG_BIG_ENDIAN
	u16 rsv:2;/*Reserved*/
	u16 dis_link_ind:1;/*Disabled Link Indication*/
	u16 all_update_inc:1;/*All Updates Included*/
	u16 bss_ch_cnt:8;/*BSS Parameters Change Count*/
	u16 link_id:4;/*Link ID*/
	u16 ap_mld_id:8;/*AP MLD ID*/
#else
	u16 ap_mld_id:8;/*AP MLD ID*/
	u16 link_id:4;/*Link ID*/
	u16 bss_ch_cnt:8;/*BSS Parameters Change Count*/
	u16 all_update_inc:1;/*All Updates Included*/
	u16 dis_link_ind:1;/*Disabled Link Indication*/
	u16 rsv:2;/*Reserved*/
#endif
};

struct multi_bssid_sub_ie {
	u8 sub_eid;
	u8 len;
	u8 non_trans_profiles[];
};

struct multi_bssid_ie {
	u8 eid;
	u8 len;
	u8 dot11v_max_bssid_indicator;
	u8 sub_ies[];
};


#define GET_MLD_BY_GRP_IDX(_idx)	(bss_mngr.mld[_idx])

#define GET_BSS_ENTRY_BY_IDX(_idx)	(bss_mngr.entry[_idx])
#define BSS_MNGR_ENTRY_CNT			(bss_mngr.dev_cnt)

#define GET_MLD_STA_BY_IDX(_idx)	(&bss_mngr.mld_sta[_idx])


/* for probe rsp, we may not take all multip bss ie,
	so we store all multip bss ie info to do select.*/
struct mbssie_info {
	u8 mbssie_num;
	u8 mbssid_idx[MAX_BEACON_NUM-1];	/*store the mbssid_idx of the mbssie by sequence*/
	u16 mbssie_offset[MAX_BEACON_NUM-1];	/*store the single mbss ie pos in the buf*/
	u16 mbssie_len[MAX_BEACON_NUM-1];	/*store the single mbss ie len in the buf*/
};

struct mbss_info {
	struct ie	tx_probe;		/* mbss ie w/o ml ie */
	struct ie	tx_probe_cmm;	/* mbss ie w/ ml ie common part */
	struct mbssie_info tx_probe_mbssie;
	struct mbssie_info tx_probe_cmm_mbssie;
	struct ie	tx_bcn;			/* mbss ie w/ ml ie common part */
	u32			crit_upd_bitmap;	/* indicate updated entry (mbss) */
	u16			bcn_tim_offset[MAX_BEACON_NUM];
	u16			bcn_mlt_offset[MAX_BEACON_NUM]; /* multi link traffic ie */
	u16			bcn_cap_info_offset[MAX_BEACON_NUM];
};

struct rnr_nap_info {
	struct neighbor_ap_info	head[16];
	struct ie	tbtt[16]; /*at most 16 bss per band*/
	u8	tbtt_cnt;
	u32 tbtt_bitmap;
};

#define IS_TBTT_BITMAP_SET(_bitmap, _bit) (_bitmap & BIT(_bit))
#define TBTT_BITMAP_SET(_bitmap, _bit) (_bitmap |= BIT(_bit))
#define TBTT_BITMAP_CLEAR(_bitmap, _bit) (_bitmap &= ~BIT(_bit))

struct repting_rnr_info {
	/* maintain reporting-requirement change */
	u32			bss_idx_bitmap[BMGR_U32_BSS_BITMAP_NUM];

	u8			repting_cnt;
	/* from bss_idx_bitmap, keep which BSS are reported (for probe rsp.) */
	u8			repting_bss_idx[BMGR_MAX_BSS_CNT];
	struct ie	repting_rnr;
};

struct tid2lnk_ie_info {
	u16		ctrl_dir:2;
	u16		ctrl_default:1;
	u16		ctrl_map_pres:8;

	/* per-link's tid [7:0] mapping */
	u8		tid_map[BSS_MNGR_MAX_BAND_NUM];
};

struct at2lm_contract_t {
	u8		id;
	u8		state;		/*NONE, MST, ED*/
	u8		sts;		/*FREE, OCCUPY*/
	u8		to_type;	/*NONE, MST_TSF, E_TSF*/
	/*u8		fw_idx;*/
	struct bmgr_entry *dis_entry;	/*disable bss for tsf based */
	u32		mst_dur;	/*unit: us*/
	UINT64		mst_tsf;	/*time for start of disable link, DTIM alignment*/
	u32		e_dur;		/*unit: us*/
	UINT64		ed_tsf;		/*time for end of disable link*/
	u8		tid_map[MLD_LINK_MAX]; /*tid_map when T0/T1(mst_tsf)/T2(ed_tsf)*/
	u16		dis_link_id_bitmap; /*disable link_id in bitmap*/
};

struct at2lm_contract_ctrl_t {
	struct at2lm_contract_t contract[BSS_MNGR_MAX_AT2LM_SET_NUM];
	struct mutex lock;
};

struct at2lm_ctrl_t {
	UINT64		mst_tsf;
	UINT64		ed_tsf;
	struct ie	at2lm;
	u8		present;
	u16		offset;
};

#ifndef TID_MAX
#define TID_MAX 8
#endif
struct nt2lm_contract_t {
	u8		in_nego_tid;			/* in nego. tid */
	u16		link_id_bitmap;			/* lind lid bitamp */
	u16		link_id_to_wcid[MLD_LINK_MAX];	/* wcid of link id */
	u8		dir;				/* direction */
	u8		tid_bit_map;			/* link map presence indicator */
	u8		tid_map[MLD_LINK_MAX];		/* org. tid_map request */
	u16		link_map[TID_MAX];		/* link map of TID 0~7*/
	u8		tid_map_dl[MLD_LINK_MAX];	/* fianl result for fw cmd */
	u8		tid_map_ul[MLD_LINK_MAX];	/* final result for fw cmd */
	u8		request_type;			/*request side define and response handle*/
};

/* indicate the status of buffer for Spec Information Elements */
enum _BUF_STATUS {
	BUF_STAT_NULL = 0,	/* buffer not allocated */
	BUF_STAT_ALLOCED,	/* buffer allocated */
	BUF_STAT_IE_READY,	/* IE is built in buffer */
};

struct reconfig_ml_ie_t {
	u8 ie_status; /* enum _BUF_STATUS, TBD: Protect */
	struct ie common_info;
	struct ie link_info[BSS_MNGR_MAX_BAND_NUM];

	u8 fw_bss_idx[BSS_MNGR_MAX_BAND_NUM];
	/* offset from Per-STA Profile head to AP Removal Timer */
	u16 tmr_offset[BSS_MNGR_MAX_BAND_NUM];
};

/* mld entry, including multiple links */
struct bmgr_mlo_dev {
	u8	valid;

	/* MLD info */
	u8	mld_grp;
	u8	mld_type;
	u8	mld_addr[MAC_ADDR_LEN];
	u8	mld_ssid[MAX_LEN_OF_SSID + 1];
	u8	mld_ssid_len;
	u8	bss_idx_mld[BSS_MNGR_MAX_BAND_NUM]; /* bss index of mld group */
	u8	mld_link_cnt;
	u8	bmc_bss_idx;
	atomic_t	bmc_sn;
	/* advertised t2lm */
	struct at2lm_contract_ctrl_t at2lm_crt_ctl;
	/* Reconfiguration Multi-Link IE */
	struct reconfig_ml_ie_t reconf_ie;

	/* MLD Info */
	struct bmgr_mld_attr attr;
};

struct csa_info {
	u16	csa_offset_in_per_sta_pf[BSS_MNGR_MAX_BAND_NUM];
	u8 csa_bss_idx;
};

/* info per BSS entry */
struct bmgr_entry {
	PNET_DEV	pNetDev;
	struct wifi_dev *pwdev;
	u8			valid;

	/* register info */
	struct bmgr_reg_info	entry_info;		/* reg info */

	/* private info - BSS */
	u8		bss_idx;
	u8		bss_idx_tx;			/* tx-bss for nt-bss */
	u8		bss_chg_cnt;		/* bss param change count */

	/* private info - MLO */
	bool	mlo_en;				/* generic mlo enabled */
	u8		link_id;
	u8		dot11be_mld_id;
	u8		tid_map;			/* sanity checked */

	/* MLD group */
	struct bmgr_mlo_dev *mld_ptr;

	struct mbss_info	mbss;

	/*single RNR*/
	struct rnr_nap_info	rnr_nap[BSS_MNGR_MAX_BAND_NUM];	/*rnr info per band */
	/* RNR for OOB 6G */
	struct repting_rnr_info rnr_info;
	/* RNR for MLD */
	struct ie		rnr;
	struct ie		mld;		/* ml ie w/ link info */
	struct ie		mld_cmm;	/* ml ie w/o link info */
	struct ie		tid2lnk;

	/* Per-STA Profile for ML Probe Rsp. */
	struct ie		ml_probe_rsp_per_sta_pf[BSS_MNGR_MAX_BAND_NUM];
	/* (temp buffer) STA Profile for ML Probe Rsp */
	struct ie		ml_probe_rsp_sta_pf[BSS_MNGR_MAX_BAND_NUM];
	/* Per-STA Profile for Beacon, non-ML Probe Rsp */
	struct ie		bcn_appl_elem_per_sta_pf[BSS_MNGR_MAX_BAND_NUM];

	struct csa_info csa;
	INT64 tsf_offset[BSS_MNGR_MAX_BAND_NUM];

	/* advertised at2lm */
	struct at2lm_ctrl_t at2lm_ctrl[BSS_MNGR_MAX_AT2LM_SET_NUM];
};

/* mld link entry */
struct bmgr_mld_link {
	bool	active;	/* indicate success assoc */
	bool	requested; /* for assoc rsp, indicate MLD STA whether requests assoc this link */
	u8		link_addr[MAC_ADDR_LEN];
	u16		str_bmap;

	/* STA Profile for Assoc Rsp. */
	struct ie	assoc_rsp_sta_profile;

	void	*priv_ptr;
};

/* mld station entry, include multiple mld link */
struct bmgr_mld_sta {
	bool	valid;
	u16		idx;	/* non-AP MLD mld_sta_idx */
	u8		bss_idx_setup; /* bss_idx of setup link */
	u8		mld_grp_idx; /* AP MLD that non-AP MLD asscoiated */
	u16		aid;
	u8		emlsr_sup;
	u8		emlsr_en;
	u16		emlsr_link_bmap;
	u16		eml_caps; /* EML Capabilities subfield in Basic ML IE */
	u16		mld_caps; /* MLD Capabilities subfield in Basic ML IE */
	u8		mld_addr[MAC_ADDR_LEN];
	u8		BAOriTID;
	u8		BARecTID;
	struct nt2lm_contract_t nt2lm_contract;
	struct bmgr_mld_link mld_link[BSS_MNGR_MAX_BAND_NUM];
};

/* global manager for BSSs twt */
struct bss_mlo_twt {
	struct list_head breakdown_action_frame_list;
	NDIS_SPIN_LOCK	lock;
	RALINK_TIMER_STRUCT agg_action_frame_timer;
	u16 mld_sta_idx;
	struct work_struct agg_timeout_work;
};

/* global manager for BSSs */
struct bss_manager {
	u8			inited;
	u8			dev_cnt;
	u32			*bss_idx_bitmap;
	u32			*mld_grp_bitmap;
	struct mutex op_mutex;

	struct bmgr_mlo_dev *mld[BMGR_MAX_MLD_GRP_CNT];
	struct bmgr_entry *entry[BMGR_MAX_BSS_CNT];
	struct bmgr_mld_sta mld_sta[BMGR_MAX_MLD_STA_CNT];
	struct bss_mlo_twt mlo_twt;
	/* bss manager event handler (bmgr->drv) */
	struct bss_mngr_event_ops *event_ops;
};

#ifdef DOT11V_MBSSID_SUPPORT
NDIS_STATUS bss_mngr_con_ie_update_mbss(struct bmgr_entry *entry);
#endif
NDIS_STATUS bss_mngr_con_ie_update_rnr_for_6g(struct bmgr_entry *entry);
#ifdef DOT11_EHT_BE
NDIS_STATUS bss_mngr_con_ie_update_rnr_for_mld(struct bmgr_entry *entry);
NDIS_STATUS bss_mngr_con_build_rnr_for_mld(struct bmgr_entry *entry);
NDIS_STATUS bss_mngr_con_ie_update_mld(struct bmgr_entry *entry);
NDIS_STATUS bss_mngr_con_sync_mld_relation(struct bmgr_entry *entry);
NDIS_STATUS bss_mngr_con_ie_update_tid_to_link_map(struct bmgr_entry *entry);
NDIS_STATUS bss_mngr_con_del_mld_sta(struct bmgr_mld_sta *bmgr_sta, u8 op);
void bss_mngr_con_mld_disconn_all_mld_sta(u8 mld_grp);
int bss_mngr_con_mld_reconfig_peer_mld(u8 mld_grp_idx, u16 rm_link_id_bitmap);
#endif
NDIS_STATUS bss_mngr_con_ie_buf_alloc(struct bmgr_entry *entry);
NDIS_STATUS bss_mngr_con_ie_buf_free(struct bmgr_entry *entry);

#ifdef DOT11V_MBSSID_SUPPORT
u8 *build_multi_bssid_ie(struct bmgr_entry *entry, u8 *f_buf, bool is_probe, bool need_ml, u8 **tim_ie_ptr, u8 **mlt_ie_ptr, u8 **cap_info_ptr);
#endif
u8 *build_rnr_ie(struct bmgr_entry *repting_entry, struct bmgr_entry *repted_entry, u8 *f_buf, u8 mld_id);
#ifdef DOT11_EHT_BE
u8 *build_multi_link_per_sta_profile(struct bmgr_entry *entry, u8 *f_buf, u8 frame_mld_type, u16 mld_sta_idx, u8 repting_link_id);
u8 *build_multi_link_ie(struct bmgr_entry *entry, u8 *f_buf, u8 frame_mld_type, u16 mld_sta_idx);
u8 *build_multi_link_probe_rsp_complete_per_sta_pf(
	struct bmgr_entry *entry,
	struct ml_ie_info *ml_info,
	u8 *f_buf);
u8 *build_multi_link_appl_elem_per_sta_pf(
	struct bmgr_entry *repting_entry,
	u8 query_type,
	u8 *f_buf);
bool build_reconfig_multi_link_ie(struct bmgr_mlo_dev *mld, struct mld_reconfig_ie_build_t *reconfig_info);
u8 *build_multi_link_traffic_ie(struct bmgr_entry *entry, u8 *f_buf, u8 **mlt_ie_ptr);
u8 *frag_multi_link_per_sta_profile(u8 *per_sta_pf, u16 *pf_len);
u16 defrag_multi_link_per_sta_profile(u8 *per_sta_pf, u8 *ml_ie_end, u8 *pf_buf, u16 *pf_len);
int parse_multi_link_ie(u8 *ml_ie, struct frag_ie_info *frag_info, struct ml_ie_info *ml_info);
u8 *build_tid_to_link_map_ie(struct bmgr_entry *entry, u8 *f_buf);
u32 parse_tid_to_link_map_ie(u8 *t2l_ie, struct tid2lnk_ie_info *t2l_info);
u8 *build_aid_bitmap_ie(struct mld_link_recomm *link_recomm, u8 *f_buf);
u8 *build_link_recomm_multi_link_traffic_ie(struct mld_link_recomm *link_recomm, u8 *f_buf);
BOOLEAN ml_reconfig_ie_check(u8 *ml_ie);

#ifdef PRE_CFG_SUPPORT
u16 pre_cfg_mld_sta_add(
	IN PNET_DEV pNetDev,
	IN struct ml_ie_info *ml_info,
	IN UCHAR *setup_link_addr);

#ifdef APCLI_SUPPORT
u16 pre_cfg_mld_ap_add(
	IN struct wifi_dev *wdev,
	IN struct ml_ie_info *ml_info);
#endif /* APCLI_SUPPORT */
#endif /* PRE_CFG_SUPPORT */
#endif

int build_mlo_kde(
	IN uint8_t data_type,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 is_all_link,
	IN u8 *buf,
	OUT ULONG * offset);

int parse_all_mlo_link_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 **mlo_kde,
	IN u8 mlo_kde_num);

int parse_all_mlo_gtk_kde(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN u8 **mlo_kde,
	IN u8 mlo_kde_num);

int mlo_install_key(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN struct _ASIC_SEC_INFO *pInfo,
	IN BOOLEAN bAE,
	IN BOOLEAN is_install);

int mlo_install_group_key(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN BOOLEAN bAE,
	IN BOOLEAN is_install);

int mlo_update_port_secure(
	IN struct _MAC_TABLE_ENTRY *entry,
	IN struct _ASIC_SEC_INFO *asic_sec_info);

#endif /*__AP_BMG_H__*/

