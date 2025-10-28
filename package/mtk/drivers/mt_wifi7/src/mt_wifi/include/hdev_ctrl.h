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
	hdev_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HDEV_CTRL_H__
#define __HDEV_CTRL_H__

struct _RTMP_ADAPTER;
struct _EDCA_PARM;
struct _QLOAD_CTRL;
struct _AUTO_CH_CTRL;
struct wifi_dev;
struct _OMAC_BSS_CTRL;
struct _REPEATER_CLIENT_ENTRY;
struct radio_res;
struct freq_oper;

struct wmm_entry {
	UCHAR dbdc_idx;
	UCHAR ref_cnt;
	UCHAR tx_mode;
	UCHAR wmm_set;
	struct _EDCA_PARM edca;
};

enum {
	HC_STATUS_OK,
	HC_STATUS_FAIL
};

struct mtk_hdev_ops {
	int (*start)(struct _RTMP_ADAPTER *ad);
	int (*stop)(struct _RTMP_ADAPTER *ad);
	int (*add_interface)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
	int (*remove_interface)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
	int (*add_mld)(struct _RTMP_ADAPTER *ad, u32 mld_type, u32 mld_group_idx, u8 *mld_addr);
	int (*add_bmc_sta)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, BOOLEAN isVlan);
	int (*remove_bmc_sta)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u16 sta_idx);
	int (*remove_mld)(struct _RTMP_ADAPTER *ad, u32 mld_group_idx);
	int (*mld_add_link)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u32 mld_group_idx);
	int (*mld_remove_link)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
	bool (*radio_state)(struct wifi_dev *wdev);
	int (*radio_info)(struct wifi_dev *wdev, struct freq_oper *oper);
	bool (*set_chan)(struct wifi_dev *wdev, struct radio_res *res);
	int (*mcu_tx)(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
	int (*set_hw_ops)(struct _RTMP_ADAPTER *ad);
	int (*clear_hw_ops)(struct _RTMP_ADAPTER *ad);
	u16 (*add_sta)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, BOOLEAN is_A4, BOOLEAN is_apcli, uint16_t mld_sta_idx);
	u16 (*remove_sta)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, u16 sta_idx);
	u8 (*get_radio_state)(struct wifi_dev *wdev);
	u8 (*get_link)(struct wifi_dev *wdev);
	int (*set_link)(struct wifi_dev *wdev, uint32_t band_idx);
	int (*get_bssidx)(struct wifi_dev *wdev);
	struct mtk_mac_dev * (*get_mac_dev)(struct _RTMP_ADAPTER *ad);
	struct mtk_mac_dev * (*get_mac_dev_by_bss)(struct _RTMP_ADAPTER *ad, u8 bss_idx);
	void (*set_radio_state)(struct wifi_dev *wdev, PHY_STATUS state);
	void (*set_radio_state_by_channel)(struct _RTMP_ADAPTER *ad, u8 channel, PHY_STATUS state);
	u32 (*get_wmm_idx)(struct wifi_dev *wdev);
	u32 (*get_omac)(struct wifi_dev *wdev);
	int (*get_bss_mld)(struct wifi_dev *wdev, struct bss_mld *mld);
	int (*get_peer_mld)(struct _RTMP_ADAPTER *ad, u16 wcid, struct peer_mld *mld);
	u8 (*get_hw_band_idx)(struct _RTMP_ADAPTER *ad);
	int (*update_wdev)(struct wifi_dev *wdev);
	bool (*init_rro_addr_elem_by_seid)(struct _RTMP_ADAPTER *ad, u16 seid);
	int (*set_wed_rmvl_per_sta)(struct _RTMP_ADAPTER *ad, u16 wcid, u8 remove_vlan);
	int (*set_wed_pn_check)(struct _RTMP_ADAPTER *ad, u16 wcid, u16 se_id, u8 enable);
	u32 (*change_setup_link_sta)(struct _RTMP_ADAPTER *ad, uint16_t wcid);
};

INT32 hw_start(struct _RTMP_ADAPTER *ad);
INT32 hw_stop(struct _RTMP_ADAPTER *ad);
int hc_get_bssidx(struct wifi_dev *wdev);
struct mtk_mac_dev *hc_get_mac_dev(struct _RTMP_ADAPTER *ad);
struct mtk_mac_dev *hc_get_mac_dev_by_bssidx(struct _RTMP_ADAPTER *ad, u8 bss_idx);
bool hc_init_rro_addr_elem_by_seid(PRTMP_ADAPTER ad, u16 seid);
int hc_set_wed_rmvl_per_sta(PRTMP_ADAPTER ad, u16 wcid, u8 remove_vlan);
int hc_set_wed_pn_check(PRTMP_ADAPTER ad, u16 wcid, u16 se_id, u8 enable);

#ifdef HWIFI_SUPPORT
VOID hc_set_hdev_ops(void *hdev_ctrl, struct mtk_hdev_ops *ops);
#endif

INT32 hc_radio_init(struct _RTMP_ADAPTER *pAd, UCHAR RfIC, UCHAR DbdcMode);
INT32 hc_radio_exit(struct _RTMP_ADAPTER *pAd, UCHAR dbdc_mode);
INT32 HcAcquireRadioForWdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef DOT11_EHT_BE
INT32 HcAcquireMldRsc(RTMP_ADAPTER *pAd, u32 mld_type, u32 mld_group_idx, u8 *mld_addr);
INT32 HcReleaseMldRsc(RTMP_ADAPTER *pAd, u32 mld_group_idx);
INT32 HcAcquireMldLinkRsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, u32 mld_group_idx);
INT32 HcReleaseMldLinkRsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /* DOT11_EHT_BE */
BOOLEAN prepare_bpcc_csa(RTMP_ADAPTER *pAd);
BOOLEAN HcUpdateCsaCnt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR HcGetBandByWdev(struct wifi_dev *wdev);
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS CurStat);
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *pAd, UCHAR Channel, PHY_STATUS CurStat);
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd);
BOOLEAN IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev);
BOOLEAN IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
#ifdef GREENAP_SUPPORT
VOID HcSetGreenAPActive(struct _RTMP_ADAPTER *pAd, BOOLEAN bGreenAPActive);
BOOLEAN IsHcGreenAPActive(struct _RTMP_ADAPTER *pAd);
#endif /* GREENAP_SUPPORT */
BOOLEAN HcIsRadioAcq(struct wifi_dev *wdev);


/*Wtable Ctrl*/
UINT16 HcAcquireGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN isVlan);
UINT16 HcAcquireGroupKeyWcid2(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcReleaseGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT16 wcid);

UINT16 HcAcquireUcastWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN is_A4, BOOLEAN is_apcli, UINT16 mld_sta_idx);
UINT16 HcReleaseUcastWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT16 wcid);
VOID HcWtblRecDump(struct _RTMP_ADAPTER *pAd);
UINT16 HcChangeSetupLink(RTMP_ADAPTER *pAd, UINT16 wcid);

VOID HcShowChCtrlInfo(struct _RTMP_ADAPTER *pAd);
#ifdef GREENAP_SUPPORT
VOID HcShowGreenAPInfo(RTMP_ADAPTER *pAd);
#endif /* GREENAP_SUPPORT */

INT32 hdev_ctrl_init(struct _RTMP_ADAPTER *pAd, INT type);
VOID hdev_ctrl_exit(struct _RTMP_ADAPTER *pAd);
VOID hdev_resource_init(void *hdev_ctrl);

void hc_set_txcmd_mode(VOID *ctrl, UCHAR txcmd_mode);
uint8_t hc_get_txcmd_mode(VOID *ctrl);
int hdev_resource_exit(struct hdev_ctrl *ctrl);

/*
WMM
*/
BOOLEAN HcAcquiredEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _EDCA_PARM *pEdca);
VOID HcReleaseEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#define HcGetWmmIdx(_pAd, _pwdev) ((_pwdev)->WmmIdx)

/*
* OmacCtrl
*/
#define HcGetOmacIdx(_pAd, _pwdev) ((_pwdev)->OmacIdx)

/*
* Bss mld
*/
int HcGetBssMld(struct wifi_dev *wdev, struct bss_mld *mld);
/*
* Peer mld
*/
int hc_get_peer_mld(RTMP_ADAPTER *ad, u16 wcid, struct peer_mld *mld);

/*
* Band index
*/
#define hc_get_hw_band_idx(_pAd) (((RTMP_ADAPTER *)_pAd)->band_idx)

uint8_t hc_update_wdev(struct wifi_dev *wdev);


USHORT HcGetRadioPhyMode(struct _RTMP_ADAPTER *pAd);
USHORT HcGetRadioPhyModeByBandIdx(RTMP_ADAPTER *pAd);
UCHAR HcGetRadioChannel(struct _RTMP_ADAPTER *pAd);
BOOLEAN  HcIsRfSupport(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);
BOOLEAN  HcIsRfRun(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);

UCHAR HcGetBw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

UCHAR HcGetRadioRfIC(struct _RTMP_ADAPTER *pAd);

struct _QLOAD_CTRL *HcGetQloadCtrlByRf(struct _RTMP_ADAPTER *pAd, UINT32 RfIC);
VOID *hc_get_qload_by_wdev(struct wifi_dev *wdev);
struct _QLOAD_CTRL *HcGetQloadCtrl(struct _RTMP_ADAPTER *pAd);
struct _AUTO_CH_CTRL *HcGetAutoChCtrl(struct _RTMP_ADAPTER *pAd);
UINT32 HcGetMgmtQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type);
UINT32 HcGetBcnQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR HcGetBandByChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
VOID HcCrossChannelCheck(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel);

INT32 HcUpdateMSDUTxAllowByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);
INT32 HcSuspendMSDUTxByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);

/*radio resource mgmt*/
BOOLEAN hc_radio_res_request(struct wifi_dev *wdev, struct radio_res *res);
UCHAR hc_reset_radio(struct _RTMP_ADAPTER *ad);
VOID hc_set_rrm_init(struct wifi_dev *wdev);
INT hc_radio_query(struct _RTMP_ADAPTER *ad, struct freq_oper *oper);
INT hc_radio_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper);

VOID *hc_get_hdev_ctrl(struct wifi_dev *wdev);

CHANNEL_CTRL *hc_get_channel_ctrl(void *hdev_ctrl);
UCHAR hc_init_ChCtrl(RTMP_ADAPTER *pAd);
UCHAR hc_init_ACSChCtrl(RTMP_ADAPTER *pAd);
UCHAR hc_set_ChCtrl(CHANNEL_CTRL *ChCtrl, RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR ChIdx2);
UCHAR hc_set_ChCtrlFlags_CAP(CHANNEL_CTRL *ChCtrl, UINT ChannelListFlag, UCHAR ChIdx);
UCHAR hc_set_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat);
UCHAR hc_check_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat);

#ifdef DOT11_HE_AX
struct pe_control *hc_get_pe_ctrl(struct wifi_dev *wdev);
#endif

/*chipcap & chipop related*/
struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl);
struct _RTMP_CHIP_OP *hc_get_chip_ops(void *hdev_ctrl);
void hc_register_chip_ops(void *hdev_ctrl, struct _RTMP_CHIP_OP *ops);
struct _RTMP_ARCH_OP *hc_get_arch_ops(void *hdev_ctrl);
struct mt_io_ops *hc_get_io_ops(void *hdev_ctrl);
struct _RTMP_CHIP_DBG *hc_get_chip_dbg(void *hdev_ctrl);
UINT32 hc_get_hif_type(void *hdev_ctrl);
VOID hc_clear_asic_cap(void *hdev_ctrl, UINT32 caps);
UCHAR hc_get_cur_rfic(struct wifi_dev *wdev);
VOID hc_set_asic_cap(void *hdev_ctrl, UINT32 caps);
UINT32 hc_get_asic_cap(void *hdev_ctrl);
UINT8 hc_get_chip_bcn_max_num(struct _RTMP_ADAPTER *ad);
void hc_set_chip_wtbl_num(void *hdev_ctrl, UINT16 max, UINT16 ucast);
UINT16 hc_get_chip_wtbl_no_matched_idx(void *hdev_ctrl);

/* MACRO */
#define hc_get_chip_wtbl_max_num(_pAd) (_pAd->physical_dev->chip_cap.wtbl_max_entries)
#define hc_get_chip_ucast_max_num(_pAd) (_pAd->physical_dev->chip_cap.wtbl_ucast_entries)

#ifdef SW_CONNECT_SUPPORT
/* API */
UINT16 hc_get_chip_sw_ucast_max_num(void *hdev_ctrl);
UINT16 hc_get_chip_sw_bcast_max_num(void *hdev_ctrl);
void hc_set_chip_sw_ucast_range_num(void *hdev_ctrl, UINT16 low, UINT16 high);
void hc_get_chip_sw_ucast_range_num(void *hdev_ctrl, UINT16 *low, UINT16 *high);
void hc_set_chip_sw_bcast_range_num(void *hdev_ctrl, UINT16 low, UINT16 high);
void hc_get_chip_sw_bcast_range_num(void *hdev_ctrl, UINT16 *low, UINT16 *high);
void hc_get_chip_hw_ucast_range_num(void *hdev_ctrl, uint16_t *low, uint16_t *high);
void hc_get_chip_hw_bcast_range_num(void *hdev_ctrl, uint16_t *low, uint16_t *high);
void hc_set_chip_hw_sw_wtbl_num(void *hdev_ctrl, UINT16 hw, UINT16 sw);
void hc_set_chip_hw_sw_ucast_num(void *hdev_ctrl, UINT16 hw, UINT16 sw);
/* get bcast sw_wcid from hw_wcid , 1: 1 mapping  */
uint16_t hc_get_chip_sw_bcast_wcid_by_hw(void *hdev_ctrl, uint16_t hw_wcid);
BOOLEAN hc_is_sw_wcid(struct _RTMP_ADAPTER *pAd, UINT16 wcid);
BOOLEAN hc_is_sw_sta_enable(struct _RTMP_ADAPTER *pAd);
struct mtk_mac_sta *hc_is_dummy_sta_exist(struct _RTMP_ADAPTER *pAd, u8 band_idx);
BOOLEAN hc_add_dummy_sta(struct _RTMP_ADAPTER *pAd, struct mtk_mac_sta *mac_sta);
BOOLEAN hc_del_dummy_sta(struct _RTMP_ADAPTER *pAd, struct mtk_mac_sta *mac_sta);
#endif /* SW_CONNECT_SUPPORT */
UINT16 hc_get_chip_max_ba_win_sz(void *hdev_ctrl);
BOOLEAN hc_get_chip_wapi_sup(void *hdev_ctrl);
UINT32 hc_get_chip_tx_token_nums(void *hdev_ctrl);
UINT32 hc_get_chip_sw_tx_token_nums(void *hdev_ctrl);
UINT32 hc_get_chip_mac_rxd_size(void *hdev_ctrl);
VOID *hc_get_hif_ctrl(void *hdev_ctrl);
VOID *hc_get_os_cookie(void *hdev_ctrl);
VOID *hc_get_mcu_ctrl(void *hdev_ctrl);
void hc_show_edca_info(void *hdev_ctrl);
VOID hc_set_phy_cap(void *hdev_ctrl, enum PHY_CAP caps);
UINT32 hc_get_phy_cap(void *hdev_ctrl);
VOID hc_set_mac_cap(void *hdev_ctrl, enum MAC_CAP caps);
UINT32 hc_get_mac_cap(void *hdev_ctrl);
void *hc_get_hdev_privdata(void *hdev_ctrl);
#ifdef CUT_THROUGH
VOID *hc_get_ct_cb(void *hdev_ctrl);
VOID hc_set_ct_cb(void *hdev_ctrl, void *ct_cb);
#endif /*CUT_THROUGH*/
VOID *hc_get_hif_ops(void *hdev_ctrl);

#define RADIO_IN_ABAND(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_5GHZ || hc_get_cur_rfic(_wdev) == RFIC_6GHZ)
#define RADIO_IN_2G(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_24GHZ)
#define RADIO_IN_5G(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_5GHZ)
#define RADIO_IN_6G(_wdev) (hc_get_cur_rfic(_wdev) == RFIC_6GHZ)

#endif /*__HDEV_CTRL_H__*/
