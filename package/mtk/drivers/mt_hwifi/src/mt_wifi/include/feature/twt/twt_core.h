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
    twt_core.h

    Abstract:
    Support twt mlme

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/

#ifndef _TWT_CORE_H_
#define _TWT_CORE_H_

#include "rtmp.h"

#ifdef MT_MAC

struct TWT_CTRL_T {
	struct _DL_LIST twt_link[SCH_LINK_NUM]; /* twt sch-link/usch-link */
	NDIS_SPIN_LOCK twt_rec_lock;            /* twt agrt resource lock */
	UINT8 token;
};

struct BTWT_BUF_STRUCT {
	BOOLEAN btwt_element_exist;
	BOOLEAN support_btwt_id_0;
	UINT8 btwt_element_num;
	UINT8 reserved;
	UINT16 btwt_bcn_offset;
	UINT16 btwt_probe_rsp_offset;
	UINT32 schedule_sp_start_tsf[TWT_HW_BTWT_MAX_NUM];
	UCHAR btwt_element[sizeof(struct btwt_ie)];
};

struct twt_link_node {
	/* twt link node control */
	struct _DL_LIST list;
	/* twt entry */
	/* occupied status, reserve for ctrl acquire/release */
	UINT8	state;
	UINT16	agrt_tbl_idx;
	UINT8	own_mac_idx;
	UINT8	flow_id;
	/* peer wcid, grp id, btwt_id */
	UINT16	peer_id_grp_id;
	UINT8	agrt_sp_duration;
	UINT8	bss_idx;
	/* indicate tsf from requester (REQUEST/SUGGEST/DEMAND)*/
	UINT8   tsf_type;
	/* used by scheduler */
	UINT64	schedule_sp_start_tsf;
	/* get abssoluate tsf based on schedule_sp_start_tsf & current_tsf */
	UINT64	schedule_sp_start_tsf_abs;
	/* tsf from requester */
	UINT64	agrt_sp_start_tsf;
	/* tsf from twt info frame */
	UINT64	agrt_sp_info_tsf;
	UINT16	agrt_sp_wake_intvl_mantissa;
	UINT8	agrt_sp_wake_intvl_exponent;
	UINT8	channel;
	UINT8	is_role_ap;
	/* ..protect/announce/trigger */
	UINT8	agrt_para_bitmap;
	/* individual/group/btwt, reserve for ctrl acquire/release */
	UINT8	type;
	BOOLEAN suspend;
	/* grp case */
	UINT8	grp_grade;
	UINT8	grp_member_cnt;
	UINT16	sta_list[TWT_HW_BTWT_MAX_MEMBER_CNT];
	/* b.TWT */
	UINT8	band;
	UINT8	twt_info_frame_dis;
	UINT8	wake_dur_unit;
	UINT8	btwt_recommendation;
	UINT8	persistence;
	UINT8	twt_setup_cmd;
	/* btwt ie is present in BCN/ProbeRsp */
	UINT8	present;
#ifdef DOT11_EHT_BE
	/* rTWT */
	BOOLEAN rtwt_create;
	BOOLEAN need_bcn_update;
	UINT8 rtwt_traffic_info_present;
	UINT8 rtwt_schedule_info;
	UINT8 traffic_info_ctrl;
	UINT8 tid_dl_bitmap;
	UINT8 tid_ul_bitmap;
#endif /* DOT11_EHT_BE */
};

struct twt_ctrl_btwt {
	UINT8 band;
	UINT8 btwt_id;
	UINT8 agrt_sp_duration;
	UINT16 agrt_sp_wake_intvl_mantissa;
	UINT8 agrt_sp_wake_intvl_exponent;
	UINT8 agrt_para_bitmap;
	UINT8 twt_setup_cmd;
	UINT8 persistence;
	UINT8 twt_info_frame_dis;
	UINT8 wake_dur_unit;
	UINT8 btwt_recommendation;
#ifdef DOT11_EHT_BE
	UINT8 rtwt_traffic_info_present;
	UINT8 rtwt_schedule_info;
	UINT8 traffic_info_ctrl;
	UINT8 tid_dl_bitmap;
	UINT8 tid_ul_bitmap;
#endif /* DOT11_EHT_BE */
};

/* hardware control interface */
struct TWT_AGRT_PARA_T {
	/* DW0 */
	UINT8		agrt_tbl_idx;
	UINT8		agrt_ctrl_flag;
	UINT8		own_mac_idx;
	/* It is set to 0xff when peerGrpId is a group ID */
	UINT8		flow_id;
	/* DW1 */
	/* Specify the peer ID (MSB=0) or group ID (MSB=1)  (lsb 10 bits=StaIdx, MSB bit=groupId) */
	UINT16		peer_id_grp_id;
	/* Same as SPEC definition. 8 bits, in unit of 256 us */
	UINT8		agrt_sp_duration;
	/* So that we know which BSS TSF should be used for this AGRT */
	UINT8		bss_idx;
	/* DW2, DW3, DW4 */
	UINT32		agrt_sp_start_tsf_low;
	UINT32		agrt_sp_start_tsf_high;
	UINT16		agrt_sp_wake_intvl_mantissa;
	UINT8		agrt_sp_wake_intvl_exponent;
	UINT8		is_role_ap;
	/* DW5 */
	/* For Bitmap definition,please refer to TWT_AGRT_PARA_BITMAP_IS_TRIGGER and etc */
	UINT8		agrt_para_bitmap;
	UINT8		persistence;
	UINT16		ntbtt_before_reject;

	/* Following field is valid ONLY when peer_id_grp_id is a group ID */
	/* DW6 */
	UINT8		grp_member_cnt;
	UINT8		agrt_tbl_idx_h;
#ifdef DOT11_EHT_BE
	/* Restricted TWT Traffic Info */
	UINT8		tid_dl_bitmap;
	UINT8		tid_ul_bitmap;
#else
	UINT16		reserved_d;
#endif /* DOT11_EHT_BE */
	/* DW7 ... */
	UINT16		sta_list[TWT_HW_BTWT_MAX_MEMBER_CNT];
};

struct TWT_AGRT_MGMT_T {
	UINT16 wcid;
	UINT16 res_id;
	UINT8 sts;
	UINT8 reserved[3];
};

struct MGMT_FRAME_OFFLOAD_T {
	UINT16 wcid;
	UINT8 bssinfo_idx;
	UINT8 token;
	/* mlo */
	UINT8 mlo_en;
	UINT8 action_linkid;
	UINT16 all_linkid_bitmap;
	UINT16 mld_sta_idx;
	UINT8 reserved[2];
	/* action frame body */
	UINT8 *buf;
	UINT32 len;
};

/* cmd option ctrl */
enum {
	OPT_SETUP_CMD_ACCEPT = 1 << 0,
	OPT_ITWT_REQ = 1 << 1,
	OPT_BTWT_REQ = 1 << 2,
	OPT_ITWT_TRAEDOWN_BY_AP = 1 << 3,
	OPT_BTWT_TRAEDOWN = 1 << 4,
};

/* teardown request reason */
enum ITWT_TEARDOWN_REASON {
	ITWT_TEARDOWN_BY_STA = 0,
	ITWT_TEARDOWN_BY_AP = 1,
	ITWT_TEARDOWN_BY_LINKDOWN = 2
};

struct TWT_TX_CMD_OPT_T {
	UINT16 opt;
	struct twt_link_node *twt_node;
	struct TWT_AGRT_PARA_T agrt_para;
};

struct TWT_TX_CMD_CTRL_T {
	UINT8 token; /* offload action frame FW token */
	UINT8 cmd_num;
	struct TWT_TX_CMD_OPT_T cmd[TWT_CMD_MAX_NUM];
};

#ifdef DOT11_EHT_BE
struct RTWT_INFO_T {
	BOOLEAN rtwt_valid;
	UINT32 btwt_id;
	UINT8 traffic_info_ctrl;
	UINT8 tid_dl_bitmap;
	UINT8 tid_ul_bitmap;
};
#endif /* DOT11_EHT_BE */

struct TWT_PEER_CTRL_T {
	UINT8 twt_flow_id_bitmap;
	UINT32 twt_btwt_id_bitmap;
	UINT32 twt_interval_max;
	RALINK_TIMER_STRUCT act_wait_ack_timer;
	struct TWT_TX_CMD_CTRL_T twt_cmd;
	/* MLO iTWT */
	UINT16 mld_sta_idx;
	UINT8 action_linkid;
	UINT16 all_linkid_bitmap;
#ifdef DOT11_EHT_BE
	/* rTWT */
	struct RTWT_INFO_T rtwt_info[TWT_BTWT_ID_NUM];
#endif /* DOT11_EHT_BE */
};

enum {
	FIRST_MLO_ITWT_IE_REQ = 1 << 0,
	BUILD_IE = 1 << 1,
	AGG_IE = 1 << 2,
	TX_ACTION = 1 << 3,
};

enum {
	STATE_FORWARD_ACTION_TO_LINK = 0,
	STATE_NO_LINKID_BITMAP_PRESENT = 1,
	STATE_BREAKDOWN_ACTION_FRAME = 2,
	STATE_ACTION_FRAME_HANDLE = 3,
	STATE_AGGREGATE_ACTION_FRAME = 4,
	STATE_TX_AGGREGATE_ACTION_FRAME = 5,
};

struct MLO_TWT_PRIV_T {
	/* da/sa is within AP links */
	PNET_DEV da_net_dev;
	PNET_DEV sa_net_dev;
	UINT8 da_linkid;
	UINT8 sa_linkid;
	/* MLO iTWT */
	UINT16 mld_sta_idx;
	UINT8 action_linkid;
	UINT16 linkid_bitmap;
	UINT16 all_linkid_bitmap;
	/* state */
	UINT16 state;
};

struct MLO_ITWT_ELEMENT_T {
	struct list_head list;
	struct frame_itwt_setup itwt_setup_frame;
	struct MLO_TWT_PRIV_T priv;
};

struct TWT_TX_ACTION_REQ_ARG_T {
	struct wifi_dev *wdev;
	UINT16 u2WlanIdx;
	UINT8 ucBssIndex;
	UINT8 ucToken;
	UINT8 ucMloEn;
	UINT16 u2MldStaIndx;
	UINT8 ucActionLinkid;
	UINT16 u2AllLinkidBitmap;
};

VOID twt_ctrl_init(struct _RTMP_ADAPTER *ad);
VOID twt_ctrl_exit(struct _RTMP_ADAPTER *ad);
INT twt_ctrl_acquire_twt_node(struct wifi_dev *wdev, struct twt_link_node **twt_node, UINT8 type);
VOID twt_ctrl_release_twt_node(struct twt_link_node *twt_node);
INT twt_ctrl_btwt_dump(struct wifi_dev *wdev, UINT32 *btwt_id_bitmap);
INT twt_ctrl_itwt_dump(struct wifi_dev *wdev);


#define TWT_ROLE_STA		0
#define TWT_ROLE_AP			1
#define TWT_ROLE_APCLI		2

#define TWT_TSF_ALIGNMENT_EN		1

#define IS_STA_WITH_TWT_IE(_ie_list) \
	(((_ie_list->twt_ie.elem_id == IE_TWT) && (_ie_list->twt_ie.len > 0)) ? TRUE : FALSE)

#define SET_PEER_ITWT_FID(_entry, _fid)      ((_entry)->twt_ctrl.twt_flow_id_bitmap |= (1 << (_fid)))
#define CLR_PEER_ITWT_FID(_entry, _fid)      ((_entry)->twt_ctrl.twt_flow_id_bitmap &= ~(1 << (_fid)))
#define GET_PEER_ITWT_FID_BITMAP(_entry)     ((_entry)->twt_ctrl.twt_flow_id_bitmap)

#ifdef APCLI_SUPPORT
#define TWT_AGRT_PARA_BITMAP_TRIGGER_OFFSET		0
#define TWT_AGRT_PARA_BITMAP_ANNCE_OFFSET		1
#define TWT_AGRT_PARA_BITMAP_PROTECT_OFFSET		2
#endif /* APCLI_SUPPORT */

enum TWT_WAIT_ACK_STS_T {
	TWT_WAIT_ACK_OK = 0,
	TWT_WAIT_ACK_TO = 1,
	TWT_WAIT_ACK_NG = 2,
	TWT_WAIT_ACK_END,
};

/* TWT tear down reqest for mlme */
struct mlme_twt_tear_down_req_struct {
	struct wifi_dev *wdev;
	UINT8 nego_type;
	UINT16   wcid;
	UCHAR   peer_addr[MAC_ADDR_LEN];
	UCHAR   twt_flow_id;
	/* bTWT*/
	UINT8 btwt_id;
	BOOLEAN teardown_all_twt;
};

/* TWT join btwt id=0 */
struct mlme_twt_join_btwt_req_struct {
	struct wifi_dev *wdev;
	UINT16   wcid;
	UCHAR   btwt_id;
};

struct mlme_twt_wait_ack_struct {
	UINT16	wcid;
	UINT8	wait_ack_sts;
};

VOID twt_agrt_cmd_set(
	OUT struct TWT_AGRT_PARA_T *agrt_para,
	IN struct twt_link_node *curr_twt_node,
	IN UINT8 agrt_ctrl_flag,
	IN UINT8 cmd_tsf_type);

VOID mlme_twt_wait_ack_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

/* TWT action frame state machine management (for peer STA role) */
VOID peer_twt_setup_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

VOID peer_twt_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

VOID twt_teardown_itwt(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 twt_flow_id);

BOOLEAN twt_get_twt_info_frame_support(
	struct wifi_dev *wdev);

VOID peer_twt_info_frame_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

VOID twt_get_resume_event(
	IN struct wifi_dev *wdev,
	IN struct twt_resume_info *resume_info);

VOID twt_wait_ack_inform(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid,
	IN UINT8 wait_ack_sts);

VOID mlme_twt_resume_info_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

/* TWT action frame trigger (for AP role) */
VOID mlme_twt_teradown_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

/* Peer STA link down twt management */
VOID twt_resource_release_at_link_down(
	IN struct wifi_dev *wdev,
	IN UINT16 wcid);

/* TWT resource dump */
VOID twt_dump_resource(
	IN struct wifi_dev *wdev);

/* TWT get current tsf */
VOID twt_get_current_tsf(
	IN struct wifi_dev *wdev,
	OUT PUINT32 current_tsf);

/* bTWT */
VOID mlme_twt_handle_btwt_join_action(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

VOID twt_peer_join_btwt_id_0(
	struct wifi_dev *wdev,
	VOID *enrty);

VOID twt_tardown_btwt(
	struct wifi_dev *wdev,
	UINT8 *peer_addr,
	UINT8 nego_type,
	UINT8 btwt_id,
	BOOLEAN teardown_all_twt);

VOID twt_build_btwt_ie(
	struct wifi_dev *wdev,
	UINT32 ie_type);

VOID twt_update_btwt_twt(
	struct wifi_dev *wdev,
	UINT32 *current_tsf);

VOID twt_update_btwt_action_frame_tsf(
	struct wifi_dev *wdev,
	UINT64 schedule_sp_start_tsf,
	struct btwt_para_set_fix *btwt_para_fix);

INT twt_acquire_btwt_node(
	struct wifi_dev *wdev,
	struct twt_ctrl_btwt *btwt_ctrl_para);

VOID twt_release_btwt_node(
	struct wifi_dev *wdev,
	UINT8 btwt_id);

VOID twt_release_btwt_resource(
	struct wifi_dev *wdev);

/* remove btwt_ie & hw resource */
VOID twt_remove_btwt_resouce(
	struct wifi_dev *wdev,
	UINT8 btwt_id);

UINT8 twt_get_btwt_element_num(
	struct wifi_dev *wdev);

BOOLEAN twt_check_btwt_member(
	UINT16 *sta_list,
	UINT16 wcid);

BOOLEAN twt_add_btwt_member(
	IN struct twt_link_node *curr_twt_node,
	IN UINT16 wcid);

VOID twt_remove_btwt_member(
	IN struct twt_link_node *curr_twt_node,
	IN UINT16 wcid);

VOID twt_handle_peer_join_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	UINT8 twt_cmd_idx,
	struct btwt_para_set *btwt_para_in,
	struct btwt_para_set *btwt_para_out);

#ifdef DOT11_EHT_BE
VOID rtwt_handle_peer_join_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	UINT8 control_in,
	UINT8 twt_cmd_idx,
	struct btwt_para_set *btwt_para_in,
	struct btwt_para_set *btwt_para_out);
#endif /* DOT11_EHT_BE */

VOID twt_handle_peer_leave_btwt_id(
	struct wifi_dev *wdev,
	UINT16 wcid,
	UINT16 btwt_id);

VOID twt_dump_btwt_elem(
	struct wifi_dev *wdev,
	UINT32 *btwt_id_bitmap);

BOOLEAN twt_is_sp_duration_tolerance(
	IN struct _RTMP_ADAPTER *ad,
	IN struct itwt_ie_t *twt_ie_in);

BOOLEAN twt_is_link_node_valid(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct twt_link_node *twt_node,
	IN BOOLEAN need_lock);

#ifdef APCLI_SUPPORT
/* TWT action frame trigger (for AP role) */
VOID twtMlmeSetupAction(
	IN struct _RTMP_ADAPTER *ad,
	IN struct _MLME_QUEUE_ELEM *elem);

VOID twtReqFsmSteps(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN enum ENUM_TWT_REQUESTER_STATE_T eNextState,
	IN UINT8 ucTWTFlowId,
	IN void *pParam);

VOID twtReqFsmRunEventRxSetup(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId);

VOID twtReqFsmRunEventRxTeardown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId);

VOID twtParseTWTElement(
	struct itwt_ie_t *prTWTIE,
	struct twt_params_t *prTWTParams);

UINT8 twtGetRxSetupFlowId(
	struct itwt_ie_t *prTWTIE);

UINT32 twtPlannerAddAgrtTbl(
	IN struct _RTMP_ADAPTER *ad,
	IN struct wifi_dev *wdev,
	IN VOID *pEntry,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucFlowId);

VOID twtReqFsmSendEvent(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucTWTFlowId,
	IN UINT8 eMsgId);

VOID twtTxDoneCheckSetupFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pkt);

VOID twtPlannerDbgPrintVal(
	IN struct _RTMP_ADAPTER *ad,
	IN struct twt_params_t *prTWTParams);

UINT32 twtPlannerDrvAgrtInsert(
	IN struct twt_planner_t *prTWTPlanner,
	IN UINT8 ucBssIdx,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 ucIdx);

UINT32 twtPlannerDrvAgrtAdd(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ucFlowId,
	IN struct twt_params_t *prTWTParams,
	IN UINT8 *pucIdx);

UINT32 twtPlannerGetCurrentTSF(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN struct twt_get_tsf_context_t *prGetTsfCtxt,
	IN UINT32 u4SetBufferLen);

UINT32 twtGetTxTeardownFlowId(
	IN struct frame_itwt_teardown *pframe_tear_down);

BOOLEAN twtPlannerIsRunning(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _STA_ADMIN_CONFIG *prStaCfg);

VOID twtPlannerDrvAgrtDelAll(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _STA_ADMIN_CONFIG *prStaCfg);
#endif /* APCLI_SUPPORT */
#endif /* MT_MAC */

#endif /* _TWT_CORE_H_ */
