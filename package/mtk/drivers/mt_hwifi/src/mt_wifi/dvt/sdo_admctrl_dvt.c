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
#include "rt_config.h"
#include "rtmp.h"
#ifdef WIFI_UNIFIED_COMMAND
#include "wlan_tr.h"
//#include "mcu/mt_cmd.h"
#include "cmm_fw_uni_cmd.h"
#include "cmm_fw_uni_event.h"
#include "framework_dvt.h"
#include "sdo_admctrl_dvt.h"
#include <net/ip.h>
#include <linux/wait.h>

#define ADM_DVT_TIMER_PERIOD	10
#define NUM_OF_WMM_AC           4

static u8  g_ucMappingUP[NUM_OF_WMM_AC] = {
	2 /* AC_BK */,   0 /* AC_BE */,   4 /* AC_VI */,   6 /* AC_VO */
};

u8 ac_to_dscp[NUM_OF_WMM_AC] = {0x10, 0x0, 0x20, 0x30};

unsigned char pattern[14 + 20] = {
	0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00,
	0x00, 0x0C, 0x43, 0x79, 0x90, 0xE1,
	0x08, 0x00,
	0x45, 0x00,
	0x05, 0x78, 0x00, 0x01,
	0x40, 0x00,
	0x40, 0x11, /* UDP */
	/* 0x40, 0x06,*/ /* TCP */
	0x00, 0x00,
	0x12, 0x34, 0x56, 0x78,
	0x78, 0x56, 0x34, 0x12
};

unsigned char tcp_header[20] = {
	0x12, 0x34, 0x56, 0x78,
	0x11, 0x22, 0x33, 0x44,
	0x55, 0x66, 0x77, 0x88,
	0x50, 0x00, /* ctrl*/
	0x99, 0xAA,
	0x00, 0x00, 0x00, 0x00
};

unsigned char dvt_mac_addr[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0x00, 0x00};
struct adm_dvt_var_t adm_dvt_var;
int adm_sta_packet_cnt[MAX_ADM_DVT_STA][4] = {0};
int adm_sta_drop_cnt[MAX_ADM_DVT_STA][4] = {0};
struct task_struct *adm_dvt_task;
RALINK_TIMER_STRUCT adm_dvt_timer;
int is_tcp;
unsigned char tcp_ctrl_flag;

DECLARE_TIMER_FUNCTION(adm_dvt_timeout);

void adm_ut_pseudo_sta_template_get(struct wifi_dev *wdev, struct dvt_seudo_sta *sta, u16 idx)
{
	os_zero_mem(sta, sizeof(*sta));
	os_move_mem(sta->addr, dvt_mac_addr, 6);
	sta->addr[4] = (UINT_8)(idx >> 8);
	sta->addr[5] = (UINT_8)(idx & 0x0FF);

	sta->cap_flag |= DVT_STA_WMM_CAP;
	sta->wdev = wdev;

	sta->phy_mode = (WMODE_G | WMODE_A | WMODE_GN | WMODE_AN | WMODE_AC | WMODE_AX_24G | WMODE_AX_5G |
				WMODE_BE_24G | WMODE_BE_5G | WMODE_BE_6G);

}

int adm_dvt_destroy_starec(u16 idx, u8 band_idx)
{
	struct _RTMP_ADAPTER *ad;
	struct adm_sta_t *adm_sta;

	ad = adm_dvt_var.ad[band_idx];
	adm_sta = &adm_dvt_var.adm_sta[idx];
	dvt_ut_seudo_sta_disconnect(ad, &adm_sta->pseudo_sta);

	return 1;
}

int adm_dvt_create_starec(u16 idx, u8 band_idx)
{
	struct _RTMP_ADAPTER *ad;
	struct wifi_dev *wdev;
	struct _STA_TR_ENTRY *tr_entry = NULL;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct adm_sta_t *adm_sta;

	ad = adm_dvt_var.ad[band_idx];
	wdev = wdev_search_by_idx(ad, 0);

	if (!wdev) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
					"wdev_search_by_idx fail\n");
		return -1;
	}

	adm_sta = &adm_dvt_var.adm_sta[idx];

	/*create a template pseudo sta*/
	adm_ut_pseudo_sta_template_get(wdev, &adm_sta->pseudo_sta, idx);
	if (dvt_ut_seudo_sta_connect(ad, &adm_sta->pseudo_sta) != DVT_STATUS_OK) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"dvt_ut_seudo_sta_connect fail\n");
		return -1;
	}

	pEntry = adm_sta->pseudo_sta.mac_entry;
	pEntry->StaIdleTimeout = 0x7FFFFFFF;
	pEntry->sta_force_keep = TRUE;
	adm_sta->band_idx = band_idx;
	adm_sta->pEntry = pEntry;
	adm_sta->wdev = wdev;
	adm_sta->wcid = pEntry->wcid;

	tr_entry = tr_entry_get(ad, pEntry->tr_tb_idx);
	tr_entry->wdev = wdev;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

	adm_dvt_var.pseudo_sta_cnt++;
	adm_dvt_var.pseudo_sta_cnt_per_band[band_idx]++;

	return 1;
}

int adm_dvt_set_rate(u16 idx, struct adm_dvt_phy_setting *phy_setting)
{
	struct _MAC_TABLE_ENTRY *pEntry;
	struct _RTMP_ADAPTER *ad;
	struct wifi_dev *wdev;
	STA_REC_CFG_T StaCfg;
	CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;
	struct adm_sta_t *adm_sta;
	INT32 ret;
	u8 band_idx;

	adm_sta = &adm_dvt_var.adm_sta[idx];

	band_idx = adm_sta->band_idx;
	ad = adm_dvt_var.ad[band_idx];

	wdev = adm_sta->wdev;
	pEntry = adm_sta->pEntry;

	StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
	StaCfg.ConnectionState = STATE_PORT_SECURE;
	StaCfg.ConnectionType = pEntry->ConnectionType;
	StaCfg.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
	StaCfg.u2WlanIdx = pEntry->wcid;
	StaCfg.pEntry = pEntry;

	pEntry->bAutoTxRateSwitch = FALSE;

	NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));

	rRaParam.FixedRateCfg.MODE = phy_setting->txmode;
	rRaParam.FixedRateCfg.BW = phy_setting->bw;
	rRaParam.FixedRateCfg.MCS = phy_setting->mcs;
	rRaParam.FixedRateCfg.STBC = 0;
	rRaParam.FixedRateCfg.ShortGI = 1;
	rRaParam.FixedRateCfg.he_ltf = 1;
	rRaParam.FixedRateCfg.ldpc = 0;
	rRaParam.FixedRateCfg.VhtNss = phy_setting->nss;

	rRaParam.u4Field = RA_PARAM_FIXED_RATE;
	RAParamUpdate(ad, pEntry, &rRaParam);

	StaCfg.u8EnableFeature = STA_REC_RA_FEATURE;
	ret = UniCmdStaRecUpdate(ad, &StaCfg);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"UniCmdStaRecUpdate STA_REC_RA_FEATURE fail(%d)\n", ret);
		return 0;
	}

	return 1;
}

int adm_dvt_set_packet(u16 idx, struct adm_dvt_packet_setting *packet_setting)
{
	struct _MAC_TABLE_ENTRY *pEntry = NULL;
	PNDIS_PACKET pkt;
	u16 wcid;
	u16 ba_winsize = 64;
	u8 tid_idx;
	u8 band_idx;
	u8 q_idx = 0;
	struct sk_buff *skb = NULL;
	u16 pktlen = DEFAULT_ADM_PKT_LEN;
	unsigned char *pData;
	struct adm_sta_t *adm_sta;
	struct _RTMP_ADAPTER *ad;

	adm_sta = &adm_dvt_var.adm_sta[idx];
	band_idx = adm_sta->band_idx;
	ad = adm_dvt_var.ad[band_idx];

	q_idx = packet_setting->qid;

	if (adm_sta->skb == NULL) {
		skb = dev_alloc_skb(MAX_ADM_PKT_LEN);

		if (skb == NULL) {
			MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"dev_alloc_skb fail\n");
			return 0;
		}

		wcid =  adm_sta->wcid;
		pktlen = packet_setting->pktlen;

		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
			"dev_alloc_skb - wcid:%u idx:%u band:%u,cnt:%u qid:%u(dscp=0x%x) pktlen=%u,ok\n"
			, wcid, idx, band_idx, packet_setting->pktcnt,
			q_idx, ac_to_dscp[q_idx % 4], pktlen);

		adm_sta->skb = skb;
		skb_put(skb, pktlen);
		pkt = (PNDIS_PACKET)skb;
		pData = GET_OS_PKT_DATAPTR(pkt);
		memset(pData, 0, pktlen);
		memcpy(pData, pattern, 14 + 20);
		if (is_tcp == 1) {
			memcpy(pData + 14 + 20, tcp_header, 20);
			pData[23] = 0x6;
			pData[47] = tcp_ctrl_flag;
		}
		pEntry = adm_sta->pEntry;
		COPY_MAC_ADDR(pData, pEntry->Addr);
		pData[14+1] = (ac_to_dscp[q_idx % 4] << 2);
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"%s:hdr[23]=0x%02x,hdr[47]=0x%02x\n",
				__func__, pData[23], pData[47]);

		ip_send_check((struct iphdr *)(pData + 14));
	}
	adm_sta->qid = q_idx;
	adm_sta->packet_cnt = packet_setting->pktcnt;
	adm_sta->pri_packet_start = packet_setting->pri_pkt_start;

	if (pEntry && adm_dvt_var.ple_pause) {
		unsigned int val[2];

		HW_IO_WRITE32(ad->hdev_ctrl, 0x820c88e0, (1 << (16 + band_idx)) | (1 << band_idx));

		HW_IO_WRITE32(ad->hdev_ctrl, 0x820c88e4,
			(0x50000000 | 1 << (12 + q_idx % 4) | (pEntry->wcid << 0)));
		HW_IO_READ32(ad->hdev_ctrl, 0x820c88e0, &val[0]);
		HW_IO_READ32(ad->hdev_ctrl, 0x820c88e4, &val[1]);
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR, "%s:val=%08x,%08X\n",
				__func__, val[0], val[1]);

	}

	adm_dvt_var.tot_dvt_pkt_cnt += packet_setting->pktcnt;

	ad->CommonCfg.AckPolicy[3] = BLOCK_ACK;/*NO_ACK;*/
	ad->CommonCfg.AckPolicy[2] = BLOCK_ACK;
	ad->CommonCfg.AckPolicy[1] = BLOCK_ACK;
	ad->CommonCfg.AckPolicy[0] = BLOCK_ACK;

	ba_winsize = packet_setting->ba_winsize;
	tid_idx = g_ucMappingUP[adm_sta->qid % 4];
	AsicUpdateBASession(ad, pEntry->wcid, tid_idx, 0, ba_winsize, TRUE, BA_SESSION_ORI, 1);

	/*
	if (adm_dvt_var.ple_pause)
		set_mec_ctrl(ad, "2-65535-0-0-0");
	*/

	/*
	if (tid_idx != 6)
		AsicUpdateBASession(ad, pEntry->wcid, 6, 0, ba_winsize,
			TRUE, BA_SESSION_ORI, 1);
	*/

	return 1;
}

int adm_dvt_update_starec(u16 idx, u8 band_idx)
{
	struct _STA_REC_CTRL_T sta_rec;
	STA_REC_CFG_T StaCfg;
	struct _RTMP_ADAPTER *ad;
	struct wifi_dev *wdev;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct _STA_TR_ENTRY *tr_entry;
	struct adm_sta_t *adm_sta;
	int ret;

	ad = adm_dvt_var.ad[band_idx];
	adm_sta = &adm_dvt_var.adm_sta[idx];
	wdev = adm_sta->wdev;
	pEntry = adm_sta->pEntry;

	tr_entry = tr_entry_get(ad, pEntry->tr_tb_idx);

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
			"wcid:%u,tr_tb_idx:%u\n", pEntry->wcid, pEntry->tr_tb_idx);

	os_zero_mem(&sta_rec, sizeof(struct _STA_REC_CTRL_T));
	os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

	fill_starec_he(wdev, pEntry, &sta_rec);

	sta_rec.ConnectionState = STATE_PORT_SECURE;
	sta_rec.ConnectionType = pEntry->ConnectionType;
	sta_rec.BssIndex = wdev->bss_info_argument.ucBssIndex;
	sta_rec.WlanIdx = pEntry->wcid;

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
			"wcid:%u,ConnectionState:%u,ConnectionType:%u\n", pEntry->wcid,
			sta_rec.ConnectionState, sta_rec.ConnectionType);

	memcpy(&StaCfg.he_sta, &sta_rec.he_sta, sizeof(struct he_sta_info));

	StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
	StaCfg.ConnectionState = STATE_PORT_SECURE;
	StaCfg.ConnectionType = pEntry->ConnectionType;
	StaCfg.ucBssIndex = wdev->bss_info_argument.ucBssIndex;
	StaCfg.u2WlanIdx = pEntry->wcid;
	StaCfg.pEntry = pEntry;
	StaCfg.u8EnableFeature = (1 << STA_REC_BASIC_STA_RECORD) |
		(1 << STA_REC_BASIC_HT_INFO) |
		(1 << STA_REC_BASIC_VHT_INFO) |
		(1 << UNI_CMD_STAREC_AP_PS) |
		(1 << UNI_CMD_STAREC_HE_BASIC) |
		(1 << UNI_CMD_STAREC_MURU)
		| ((uint64_t)1 << UNI_CMD_STAREC_EHT_MLD)
		| ((uint64_t)1 << UNI_CMD_STAREC_EHT_BASIC)
		| (1 << UNI_CMD_STAREC_HW_AMSDU)
		| (1 << UNI_CMD_STAREC_BA);
	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
			"wcid:%u,ConnectionState:%u,ConnectionType:%u, Feature=0x%llx\n",
			pEntry->wcid,
			sta_rec.ConnectionState, sta_rec.ConnectionType, StaCfg.u8EnableFeature);

/*
	StaCfg.u8EnableFeature = (1 << STA_REC_BASIC_STA_RECORD) |
		(1 << STA_REC_BASIC_HT_INFO) |
		(1 << STA_REC_BASIC_VHT_INFO) |
		(1 << UNI_CMD_STAREC_AP_PS) |
		(1 << STA_REC_AMSDU) | (1 << STA_REC_HW_AMSDU) |
		(1 << STA_REC_WTBL) |
		(1 << UNI_CMD_STAREC_HE_BASIC) |
		(1 << STA_REC_MURU);
*/

/*
	StaCfg.u8EnableFeature = (1 << STA_REC_BASIC_STA_RECORD) |
		(1 << STA_REC_BASIC_HT_INFO) |
		(1 << STA_REC_BASIC_VHT_INFO) |
		(1 << UNI_CMD_STAREC_HE_INFO) |
		(1 << UNI_CMD_STAREC_AP_PS) |
		(1 << STA_REC_AMSDU) | (1 << STA_REC_HW_AMSDU) |
		(1 << STA_REC_WTBL) |
		(1 << UNI_CMD_STAREC_HE_BASIC) |
		((uint64_t)1 << UNI_CMD_STAREC_EHT_MLD) |
		((uint64_t)1 << UNI_CMD_STAREC_EHT_BASIC) |
		(1 << STA_REC_MURU);
*/
/*
	StaCfg.u8EnableFeature = (1 << STA_REC_BASIC_STA_RECORD) |
		(1 << STA_REC_BASIC_HT_INFO) |
		(1 << STA_REC_BASIC_VHT_INFO) |
		(1 << UNI_CMD_STAREC_HE_INFO) |
		(1 << UNI_CMD_STAREC_AP_PS) |
		(1 << STA_REC_AMSDU) | (1 << STA_REC_HW_AMSDU) |
		(1 << STA_REC_WTBL) |
		(1 << UNI_CMD_STAREC_HE_BASIC) |
		(1 << STA_REC_MURU);
*/
	if (adm_dvt_var.ple_pause)
		StaCfg.u8EnableFeature &= ~((1 << STA_REC_AMSDU) | (1 << STA_REC_HW_AMSDU));
	ret = UniCmdStaRecUpdate(ad, &StaCfg);
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"UniCmdStaRecUpdate fail(%d)\n", ret);
		return 0;
	}

	/* ap_conn_act(wdev, pEntry);*/

	return 1;
}

INT set_adm_dvt_cmd(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 cmd = 3, rv, ret = 0;
	int i;
	UINT_16 tc_id = 0;
	UINT_8 band_idx = 0;
	UINT32 param[10] = {0};
	struct wifi_dev *wdev = NULL;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d-%d-%d", &cmd, &param[0], &param[1], &param[2], &param[3]);

		if (rv == 0)
			cmd = 3;

		switch (cmd) {
		case 11:
			for (i = 0; i < WDEV_NUM_MAX; i++) {
				wdev = pAd->wdev_list[i];

				if (wdev) {
					MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
						"wdev[%d] phymode:%x,bss:%u,hw_bssid:%u,idx:%u\n",
						i,
						wdev->PhyMode,
						wdev->BssIdx,
						wdev->hw_bssid_idx,
						wdev->wdev_idx);
				}
			}
			for (i = 0; i < 20; i++)
				MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
					"dbg[%u]=%u\n", i, adm_dvt_var.dbgcnt[i]);

			break;
		case 0:
			band_idx = param[0] % 3;
			if (band_idx == 0) {
				is_tcp = 0;
				tcp_ctrl_flag = 0;
				memset(&adm_dvt_var, 0, sizeof(struct adm_dvt_var_t));
				memset(adm_sta_packet_cnt, 0, sizeof(int)*MAX_ADM_DVT_STA*4);
				memset(adm_sta_drop_cnt, 0, sizeof(int)*MAX_ADM_DVT_STA*4);
				adm_dvt_task = NULL;
			}

			adm_dvt_var.ad[band_idx] = pAd;
			wdev = wdev_search_by_idx(pAd, 0);
			if (param[1] == 0)
				wlan_config_set_amsdu_en(wdev, FALSE);
			else
				wlan_config_set_amsdu_en(wdev, TRUE);
			wlan_config_set_ba_enable(wdev, TRUE);
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"band:%u,pAd=%p,wdev=%p\n", band_idx, pAd, wdev);

			break;
		case 1:
			tc_id = param[0];
			if (param[1] == 1)
				adm_dvt_var.ple_pause = 1;
			if (param[2] == 1)
				is_tcp = 1;
			else
				is_tcp = 0;
			if ((param[3] != 0) && is_tcp)
				tcp_ctrl_flag = param[3];
			else
				tcp_ctrl_flag = 0;

			adm_dvt_test_init(tc_id);
			break;
		case 2:
			adm_dvt_test_start();
			break;
		case 3:
			adm_dvt_var.collect_data = 1;
			break;
		case 4:
			adm_dvt_var.collect_data = 0;
			adm_dvt_ple_usage_dump(pAd, true);
			break;
		case 5:
			adm_dvt_timer_deinit(pAd);
			break;
		case 6:
			adm_dvt_test_deinit();
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"AdmCtrl DVT Cmd:%u[%s:%s]\n", cmd, "date", "time");
			break;
		}
		return FALSE;
	}
	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR, "ret=%u\n", ret);

	return TRUE;
}

static inline struct mtk_mac_dev *
pad_to_mac_dev(struct _RTMP_ADAPTER *ad)
{
	struct os_cookie *handle = ad->OS_Cookie;

	return handle->mac_dev;
}

int adm_dvt_thread(void *data)
{
	unsigned char q_idx = QID_AC_VO;
	struct _MAC_TABLE_ENTRY *pEntry;
	PNDIS_PACKET pkt;
	struct sk_buff *skb = NULL;
	u16 wcid = 1, idx = 0;
	u16 empty_idx = 0;
	u8 band_idx = 0;
	int ret;
	u32 cnt = 0, tot_dvt_pkt_cnt = 0;
	int pri_packet[10] = {0};
	int packet_cnt[10] = {0};
	unsigned char *pData;
	struct mtk_mac_dev *mac_dev = NULL;
	struct _RTMP_ADAPTER *ad;
	struct adm_dvt_var_t *adm_dvt = (struct adm_dvt_var_t *)data;
	struct adm_sta_t *adm_sta;

	empty_idx = 0;
	ad = adm_dvt->ad[0];

	tot_dvt_pkt_cnt = adm_dvt->tot_dvt_pkt_cnt;

	adm_dvt_var.collect_data = 1;

	for (idx = 0; idx < 10; idx++) {
		pri_packet[idx] = 0;
		adm_sta = &adm_dvt->adm_sta[idx];
		packet_cnt[idx] = adm_sta->packet_cnt;
	}

	do {
		adm_sta = &adm_dvt->adm_sta[idx];
		q_idx = adm_sta->qid;
		wcid = adm_sta->wcid;
		if (adm_sta->packet_cnt == 0) {
			empty_idx++;
			goto NEXT;
		}
		skb = skb_clone(adm_sta->skb, GFP_ATOMIC);
		if (skb == NULL) {
			MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"skb_clone fail\n");
			return 0;
		}
		pEntry = adm_sta->pEntry;

		pkt = (PNDIS_PACKET)skb;
		pData = GET_OS_PKT_DATAPTR(pkt);
		RTMP_SET_PACKET_WCID(pkt, wcid);
		RTMP_SET_PACKET_QUEIDX(pkt, q_idx);
		RTMP_SET_PACKET_UP(pkt, g_ucMappingUP[q_idx%4]);
		RTMP_SET_PACKET_HIGH_PRIO(pkt, FALSE);

		if ((idx < 10) && (adm_sta->pri_packet_start > 0)) {
			if ((!pri_packet[idx]) && ((packet_cnt[idx] - adm_sta->packet_cnt) >= adm_sta->pri_packet_start))
				pri_packet[idx] = 1;

			if (pri_packet[idx]) {
				adm_dvt_var.dbgcnt[1]++;
				RTMP_SET_PACKET_HIGH_PRIO(pkt, TRUE);
			}
		}

		skb->pkt_type =	PKT_TYPE_RED_DVT;
RETRY:

		band_idx = adm_sta->band_idx;
		ad = adm_dvt->ad[band_idx];
		mac_dev = pad_to_mac_dev(ad);

		ret = fp_adm_dvt_tx_pkt(ad, pkt, idx);

		if (ret < 0) {
			if (mac_dev->ops->tx_kick) {
				mac_dev->ops->tx_kick(&mac_dev->hw);
				cnt = 0;
			}
			adm_dvt->kthread_running = FALSE;
			wait_event_interruptible(adm_dvt->kthread_q,
				adm_dvt->kthread_running || kthread_should_stop());
			goto RETRY;
		}

		if (mac_dev->ops->tx_kick) {
			mac_dev->ops->tx_kick(&mac_dev->hw);
			cnt = 0;
			udelay(1000);
		}
		cnt++;
		adm_sta->packet_cnt--;
		tot_dvt_pkt_cnt--;
NEXT:
		idx = (idx + 1) % MAX_ADM_DVT_STA;
	} while ((tot_dvt_pkt_cnt > 0) && (adm_dvt->kthread_running));

	if ((mac_dev->ops->tx_kick) && adm_dvt->kthread_running)
		mac_dev->ops->tx_kick(&mac_dev->hw);

	adm_dvt->kthread_running = FALSE;

	/* adm_dvt_var.collect_data = 0; */

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR, "thread exit\n");

	adm_dvt_task = NULL;

	return 1;
}

int adm_dvt_tx_status(void *param)
{
	struct mtk_tx_status *tx_status = NULL;

	tx_status = (struct mtk_tx_status *)param;
	if (tx_status) {
		if (tx_status->stat != 0) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				 "wcid[%d] qid[%d] from band:%d, tx_status=%p\n",
				 tx_status->wcid, tx_status->qid, tx_status->band_idx, tx_status);
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_WARN,
				 "stat:0x%x, tx times:%d, air delay:%d, transmit delay:%d\n",
				 tx_status->stat, tx_status->cnt,
				 tx_status->air_latency, tx_status->mac_latency);
		}

		if (tx_status->wcid < MAX_ADM_DVT_STA) {
			if (tx_status->stat != 0) {
				adm_sta_drop_cnt[tx_status->wcid][(tx_status->qid % 4)]++;
				if (tx_status->stat == 1)
					adm_dvt_var.dbgcnt[14]++;
				if (tx_status->stat == 2)
					adm_dvt_var.dbgcnt[15]++;
				if (tx_status->stat == 3)
					adm_dvt_var.dbgcnt[16]++;
				if (tx_status->stat > 3)
					adm_dvt_var.dbgcnt[17]++;
			}
			adm_sta_packet_cnt[tx_status->wcid][(tx_status->qid % 4)]++;
		}
	}

	adm_dvt_var.dbgcnt[9]++;
	adm_dvt_var.txdone_pkt_cnt++;

	if (adm_dvt_var.curr_tc_id == 0)
		return 0;

	if (adm_dvt_task == NULL)
		return 0;

	if (adm_dvt_var.kthread_running == FALSE) {
		adm_dvt_var.kthread_running = TRUE;
		wake_up(&(adm_dvt_var.kthread_q));
	}

	return 0;
}

int fp_adm_dvt_tx_pkt(struct _RTMP_ADAPTER *ad, NDIS_PACKET *pkt, u16 idx)
{
	struct _TX_BLK *tx_blk;
	struct _TX_BLK blk;
	uint16_t wcid = 0;
	INT ret = 0;
	struct wifi_dev *wdev = NULL;
	struct _MAC_TABLE_ENTRY *pEntry;
	struct _STA_TR_ENTRY *tr_entry;
	struct wifi_dev_ops *wdev_ops;
	struct mtk_mac_bss *bss;
	struct mtk_mac_sta *sta;
	struct mtk_mac_txq *txq;
	struct mtk_mac_dev *mac_dev = pad_to_mac_dev(ad);
	struct adm_sta_t *adm_sta;

	NdisZeroMemory((UCHAR *)&blk, sizeof(struct _TX_BLK));
	tx_blk = &blk;

	adm_sta = &adm_dvt_var.adm_sta[idx];
	wcid = adm_sta->wcid;
	pEntry = adm_sta->pEntry;

	bss = NULL;

	tr_entry = tr_entry_get(ad, pEntry->tr_tb_idx);

	wdev = tr_entry->wdev;

	sta = tr_entry->mac_sta;
	if (!sta) {
		if (adm_dvt_var.dbgcnt[8] < 10) {
			MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
				"wcid=%u,tr_entry=%p,pEntry=%p,tr_tb_idx=%u\n",
				wcid, tr_entry, pEntry, pEntry->tr_tb_idx);
		}
		adm_dvt_var.dbgcnt[8]++;
		return -1;
	}


	tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
	if (tx_blk->QueIdx > 4)
		txq = sta->txq[0];
	else
		txq = sta->txq[tx_blk->QueIdx];


	if (!mac_dev->ops->tx_check_resource(&mac_dev->hw, NULL, txq))
		return -2;

	tx_blk->wdev = wdev;
	tx_blk->TotalFrameNum = 1;
	tx_blk->TotalFragNum = 1;
	tx_blk->tr_entry = tr_entry;
	tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
	tx_blk->pPacket = pkt;
	tx_blk->wmm_set = 0;

	InsertTailQueue(&tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));
	tx_blk->TxFrameType = tx_pkt_classification(ad, tx_blk->pPacket, tx_blk);
	if (tx_blk->TxFrameType == TX_AMSDU_FRAME)
		tx_blk->TxFrameType = TX_LEGACY_FRAME;
	tx_blk->Wcid = wcid;
	tx_blk->pMacEntry = entry_get(ad, wcid);

	tx_blk->dbdc_band = adm_sta->band_idx;

	wdev_ops = wdev->wdev_ops;

	if (GET_PKT_TYPE(pkt) == PKT_TYPE_RED_DVT) {
		adm_dvt_var.dbgcnt[idx]++;
		adm_dvt_var.dbgcnt[wcid+10]++;
		if (RTMP_GET_PACKET_HIGH_PRIO(pkt))
			TX_BLK_SET_FLAG(tx_blk, fTX_HIGH_PRIO);
		TX_BLK_SET_FLAG(tx_blk, fTX_bNoRetry);
		tx_blk->UserPriority = RTMP_GET_PACKET_UP(pkt);

	}
	ret = wdev_ops->tx_pkt_handle(ad, wdev, tx_blk);
	return ret;
}

VOID adm_dvt_ple_usage_dump(struct _RTMP_ADAPTER *ad, bool reset)
{
	char buf[256] = {0};
	u32 avg_usage = 0;
	int pos = 0, i;

	if (adm_dvt_var.timer_cnt == 0)
		adm_dvt_var.timer_cnt = 1;

	avg_usage = adm_dvt_var.ple_tot_usage/adm_dvt_var.timer_cnt;
	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
			"avg usage:%u (timer:%u)\n",
			avg_usage, adm_dvt_var.timer_cnt);

	for (i = 0; i < 32+8; i++) {
		pos += sprintf(buf + pos, "%u,", adm_dvt_var.ple_usage_level_cnt[i]);
		if ((i % 16) == 15)
			pos += sprintf(buf + pos, "\n");
	}

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR, "%s\n", buf);

	if (reset) {
		adm_dvt_var.timer_cnt = 0;
		adm_dvt_var.ple_tot_usage = 0;
		os_zero_mem(adm_dvt_var.ple_usage_level_cnt, sizeof(UINT_32)*20);
	}
}

VOID adm_dvt_timeout(PVOID SystemSpecific1, PVOID FunctionContext,
			PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)FunctionContext;
	UINT_32 u4Value = 0;
	UINT_32 usage_level = 0, usage_page = 0;

	if (!adm_dvt_var.collect_data)
		return;

	if (adm_dvt_var.txdone_pkt_cnt < adm_dvt_var.tot_dvt_pkt_cnt)
		adm_dvt_var.timeout_cnt++;

	if ((adm_dvt_var.timeout_cnt >= 30*60*100) || (adm_dvt_var.txdone_pkt_cnt >= adm_dvt_var.tot_dvt_pkt_cnt)) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
			"timeout:%u, txdone:%u\n",
			adm_dvt_var.timeout_cnt, adm_dvt_var.txdone_pkt_cnt);
		adm_dvt_var.collect_data = 0;
		adm_dvt_var.timeout_cnt = 0;

		adm_dvt_test_deinit();
		return;
	}

	HW_IO_READ32(ad->hdev_ctrl, 0x820c03a8, &u4Value);
	usage_page = u4Value >> 16;
	usage_level = usage_page / 256;
	adm_dvt_var.ple_usage_level_cnt[usage_level]++;
	adm_dvt_var.ple_tot_usage += usage_page;
	adm_dvt_var.timer_cnt++;
}

BUILD_TIMER_FUNCTION(adm_dvt_timeout);
int adm_dvt_timer_init(struct _RTMP_ADAPTER *ad)
{
	if (adm_dvt_timer.Valid)
		return 0;

	MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
		"AdmCtrl Flags:%08X\n", (UINT_32)ad->Flags);

	RTMPInitTimer(ad, &adm_dvt_timer, GET_TIMER_FUNCTION(adm_dvt_timeout),
			ad, TRUE);
	RTMPSetTimer(&adm_dvt_timer, ADM_DVT_TIMER_PERIOD);
	if (!adm_dvt_timer.Valid) {
		MTWF_DBG(ad, DBG_CAT_TEST, CATTEST_SDO, DBG_LVL_ERROR,
		"[QWERT]AdmCtrl Flags:%08X\n", (UINT_32)ad->Flags);
		return 0;
	}
	/* adm_dvt_var.collect_data = 1; */

	return 1;
}

int adm_dvt_timer_deinit(struct _RTMP_ADAPTER *ad)
{
	adm_dvt_var.collect_data = 0;

	adm_dvt_ple_usage_dump(ad, true);

	if (adm_dvt_timer.Valid) {
		BOOLEAN Cancelled = TRUE;

		RTMPReleaseTimer(&adm_dvt_timer, &Cancelled);
	}

	return 1;
}
#endif /* WIFI_UNIFIED_COMMAND */
