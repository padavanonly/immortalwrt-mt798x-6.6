
#ifdef CONFIG_MAP_SUPPORT

#include "rt_config.h"
#include "map.h"
#include "cfg80211/cfg80211.h"
#ifdef MAP_R2
#include <linux/if_vlan.h>
#endif
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif

UCHAR MAP_OUI[3] = {0x50, 0x6F, 0x9A};
UCHAR MAP_OUI_TYPE[1] = {0x1B};
UCHAR MAP_EXT_ATTRI[1] = {0x06};
UCHAR MAP_ATTRI_LEN[1] = {1};

UCHAR multicast_mac_1905[MAC_ADDR_LEN] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x13};
#define MAP_EXT_ATTRIBUTE 0x06
#ifdef MAP_R2
UCHAR MAP_PROFILE_ATTRI = 0x07;
UCHAR MAP_PROFILE_LEN = 1;
UCHAR MAP_TRAFFIC_SEPARATION_ATTRI = 0x08;
UCHAR MAP_TRAFFIC_SEPARATION_LEN = 2;
#define MAP_PROFILE_ATTRIBUTE 0x07
#define MAP_TRAFFIC_SEPARATION_ATTRIBUTE 0x08
#endif
#ifdef FT_R1KH_KEEP
#define RADIO_ON_SET	1
#define RADIO_ON_RESET	2
#endif /* FT_R1KH_KEEP */
#ifdef QOS_R1
extern void qos_set_dscp2up_mapping(PRTMP_ADAPTER pAd, BSS_STRUCT * pMbss);
#endif

static UCHAR MAP_CheckDevRole(
	PRTMP_ADAPTER pAd,
	UCHAR wdev_type
)
{
	UCHAR res = 0;

	switch (wdev_type) {
	case WDEV_TYPE_AP:
		res = BIT(MAP_ROLE_FRONTHAUL_BSS); /* BH_BSS will be set by map cmd */
	break;

	/* case WDEV_TYPE_APCLI: */
	case WDEV_TYPE_STA:
		res = BIT(MAP_ROLE_BACKHAUL_STA);
	break;

	default:
		res = 0;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
			"AP Role not set (Fixed me)\n");
	}

	return res;
}

INT MAP_InsertMapWscAttr(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf
)
{
	UCHAR MapVendorExt[10] = {0};
	UCHAR va[2] = {0x10, 0x49};
	UCHAR vl[2] = {0x00, 0x06};
	UCHAR vi[3] = {0x00, 0x37, 0x2A};

	/*WPS Vendor Extension */
	NdisMoveMemory(MapVendorExt, va, 2);
	NdisMoveMemory(MapVendorExt + 2, vl, 2);
	NdisMoveMemory(MapVendorExt + 4, vi, 3);
	NdisMoveMemory(MapVendorExt + 7, MAP_EXT_ATTRI, 1);
	NdisMoveMemory(MapVendorExt + 8, MAP_ATTRI_LEN, 1);
	NdisMoveMemory(MapVendorExt + 9, &wdev->MAPCfg.DevOwnRole, 1);

	NdisMoveMemory(pFrameBuf, MapVendorExt, sizeof(MapVendorExt));

	return sizeof(MapVendorExt);
}

VOID MAP_InsertMapCapIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen
)
{
	ULONG TmpLen = 0, total_len = 0;
	UCHAR IEType = IE_VENDOR_SPECIFIC;
	UCHAR IELen = 7;
	UCHAR MAP_EXT_ATTRI_LEN = 1;
	UCHAR MAP_EXT_ATTRI_VAL = wdev->MAPCfg.DevOwnRole;
#ifdef MAP_R2
	UCHAR MAP_EXT_PROFILE_VAL;
	UINT16 vid = 0;

	if (IS_MAP_R2_ENABLE(pAd)) {
		IELen += 3;
		if (wdev->wdev_type == WDEV_TYPE_AP && wdev->MAPCfg.vid_num > 0
#ifdef MAP_R4
		&& (wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS))
			&& (wdev->MAPCfg.primary_vid != INVALID_VLAN_ID)
#endif /* MAP_R4 */
		)
			IELen += 4;
	}
#endif
	MakeOutgoingFrame(pFrameBuf, &TmpLen,
						1, &IEType,
						1, &IELen,
						3, MAP_OUI,
						1, MAP_OUI_TYPE,
						1, MAP_EXT_ATTRI,
						1, &MAP_EXT_ATTRI_LEN,
						1, &MAP_EXT_ATTRI_VAL,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TmpLen;
	total_len += TmpLen;
#ifdef MAP_R2
	if (IS_MAP_R2_ENABLE(pAd)
#ifdef MAP_R3
		|| IS_MAP_R3_ENABLE(pAd)
#endif
	) {
		MAP_EXT_PROFILE_VAL = 0x02;
#ifdef MAP_R3
		/* modify the map profile val to 3 for MAP_R3 */
		if (IS_MAP_R3_ENABLE(pAd))
			MAP_EXT_PROFILE_VAL = 0x03;
#endif /* MAP_R3 */
		MakeOutgoingFrame(pFrameBuf + total_len, &TmpLen,
						1, &MAP_PROFILE_ATTRI,
						1, &MAP_PROFILE_LEN,
						1, &MAP_EXT_PROFILE_VAL,
						END_OF_ARGS);

		*pFrameLen = *pFrameLen + TmpLen;
		total_len += TmpLen;

		/*only add default 802.1q setting in assoc response*/
		if (wdev->wdev_type == WDEV_TYPE_AP && wdev->MAPCfg.vid_num > 0
			&& wdev->MAPCfg.primary_vid != INVALID_VLAN_ID) {
			vid = cpu2le16(wdev->MAPCfg.primary_vid);
			MakeOutgoingFrame(pFrameBuf + total_len, &TmpLen,
							1, &MAP_TRAFFIC_SEPARATION_ATTRI,
							1, &MAP_TRAFFIC_SEPARATION_LEN,
							2, &vid,
							END_OF_ARGS);

			*pFrameLen = *pFrameLen + TmpLen;
		}

	}
#endif
}

VOID MakeMAPIe(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	ULONG *pFrameLen,
	UCHAR *pFrame)
{
	ULONG FrameLen = *pFrameLen;

	if (IS_MAP_ENABLE(pAd) && wdev->MAPCfg.vendor_ie_len) {
		ULONG MAPIeTmpLen = 0;

		MakeOutgoingFrame(pFrame + FrameLen, &MAPIeTmpLen,
						wdev->MAPCfg.vendor_ie_len, wdev->MAPCfg.vendor_ie_buf,
						END_OF_ARGS);
		FrameLen += MAPIeTmpLen;

	}
	*pFrameLen = FrameLen;
}

UINT32 map_rc_get_band_idx_by_chan(PRTMP_ADAPTER pad, UCHAR channel)
{
	return hc_get_hw_band_idx(pad);
}

/* return map attribute*/
BOOLEAN map_check_cap_ie(
	IN PEID_STRUCT   eid,
	OUT  unsigned char *cap
#ifdef MAP_R2
	, OUT UCHAR *profile,
	OUT UINT16 *vid
#endif
)
{
	BOOLEAN Ret = FALSE;
	INT32 len;
	PEID_STRUCT map_eid;
#ifdef MAP_R2
	*profile = 0;
	*vid = INVALID_VLAN_ID;
#endif
	if (NdisEqualMemory(eid->Octet, MAP_OUI, sizeof(MAP_OUI)) && (eid->Len >= 7)) {
		if (NdisEqualMemory((UCHAR *)&eid->Octet[3], MAP_OUI_TYPE, sizeof(MAP_OUI_TYPE))) {
			len = eid->Len - 4;
			map_eid = (PEID_STRUCT)&eid->Octet[4];
			Ret = TRUE;
			while ((len > 2) && ((map_eid->Len + 2) <= len)) {
				switch (map_eid->Eid) {
				case MAP_EXT_ATTRIBUTE:
					if (map_eid->Len != MAP_ATTRI_LEN[0]) {
						Ret = FALSE;
						break;
					}
					*cap = map_eid->Octet[0];
					break;
#ifdef MAP_R2
				case MAP_PROFILE_ATTRIBUTE:
					if (map_eid->Len != MAP_PROFILE_LEN) {
						Ret = FALSE;
						break;
					}
					*profile = map_eid->Octet[0];
					break;
				case MAP_TRAFFIC_SEPARATION_ATTRIBUTE:
					if (map_eid->Len != MAP_TRAFFIC_SEPARATION_LEN) {
						Ret = FALSE;
						break;
					}
					os_move_mem(vid, map_eid->Octet, MAP_TRAFFIC_SEPARATION_LEN);
					*vid = le2cpu16(*vid);
					break;
#endif
				default:
					break;
				}
				len -= (INT32)(2 + map_eid->Len);
				map_eid = (PEID_STRUCT)((UCHAR *)map_eid + 2 + map_eid->Len);
			}
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
					"[MAP] STA Attri = %02x\n", *cap);
#ifdef MAP_R2
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG,
					"[MAP] Profile = %02x, vid=%d\n", *profile, *vid);
#endif
			if (Ret == FALSE)
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"[MAP] !!!!!Invalid MAP IE\n");
		}
	}

	return Ret;
}

UCHAR getNonOpChnNum(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR op_class
)
{
	UCHAR i = 0, j = 0;
	UCHAR nonOpChnNum = 0, opChnNum = 0;
	UCHAR *opChList = get_channelset_by_reg_class(pAd, op_class, wdev->PhyMode);
	UCHAR opChListLen = get_channel_set_num(opChList);

	for (i = 0; i < opChListLen; i++) {
		for (j = -0; j < pAd->ChannelListNum; j++) {
			if (opChList[i] == pAd->ChannelList[j].Channel) {
				opChnNum++;
				break;
			}
		}
	}
	nonOpChnNum = opChListLen - opChnNum;

	return nonOpChnNum;
}

UCHAR getAutoChannelSkipListNum(
		IN PRTMP_ADAPTER pAd,
		IN struct wifi_dev *wdev)
{
	return pAd->ApCfg.AutoChannelSkipListNum;
}

VOID setNonOpChnList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PCHAR nonOpChnList,
	IN UCHAR op_class,
	IN UCHAR nonOpChnNum
)
{
	UCHAR i = 0, j = 0, k = 0;
	BOOLEAN found = false;
	UCHAR *opChList = get_channelset_by_reg_class(pAd, op_class, wdev->PhyMode);
	UCHAR opChListLen = get_channel_set_num(opChList);

	if (nonOpChnNum > 0) {
		for (i = 0; i < opChListLen; i++) {
			for (j = -0; j < pAd->ChannelListNum; j++) {
				if (opChList[i] == pAd->ChannelList[j].Channel)
					found = true;
			}

			if (found == false) {
				nonOpChnList[k] = opChList[i];
				k++;
			} else
				found = false;
		}
	} else
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"No Non Op Channel\n");


}


VOID setAutoChannelSkipList(
		IN PRTMP_ADAPTER pAd,
		IN struct wifi_dev *wdev,
		IN wdev_chn_info * chn_list)
{

	UCHAR i = 0;
	UCHAR AutoChannelSkipListNum = getAutoChannelSkipListNum(pAd, wdev);

	for (i = 0; i < AutoChannelSkipListNum; i++)
		chn_list->AutoChannelSkipList[i] = pAd->ApCfg.AutoChannelSkipList[i];

	chn_list->AutoChannelSkipListNum = i;
}

int map_make_vend_ie(IN PRTMP_ADAPTER pAd, IN UCHAR ApIdx)
{
	struct vendor_map_element *ie = NULL;
	char *buf;
	int ie_len = 0;

	ie_len = sizeof(struct vendor_map_element);

	os_alloc_mem(NULL, (UCHAR **)&buf, sizeof(struct vendor_map_element));
	if (!buf) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"memory is not available\n");
		return -1;
	}
	NdisZeroMemory(buf, ie_len);
	ie = (struct vendor_map_element *)buf;

	ie->eid = VEND_IE_TYPE;
	ie->length = ie_len - 2;
	NdisCopyMemory(ie->oui, MTK_OUI, OUI_LEN);
	ie->mtk_ie_element[0] = 0;
	ie->mtk_ie_element[1] = 1;
	ie->type = 0;
	ie->subtype = 0;
	ie->root_distance = 0;
	ie->controller_connectivity = 0;
	ie->uplink_rate = 0;
	NdisZeroMemory(ie->_2g_bssid, ETH_ALEN);
	NdisZeroMemory(ie->_5g_bssid, ETH_ALEN);
	NdisZeroMemory(ie->uplink_bssid, ETH_ALEN);
	wapp_set_ap_ie(pAd, buf, ie_len, ApIdx);

	os_free_mem(buf);

	return 0;
}


VOID MAP_Init(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR wdev_type
)
{
	wdev->MAPCfg.DevOwnRole = MAP_CheckDevRole(pAd, wdev_type);
	wdev->MAPCfg.bUnAssocStaLinkMetricRptOpBss = TRUE;/*by default*/
	wdev->MAPCfg.bUnAssocStaLinkMetricRptNonOpBss = FALSE;/*by default*/
#ifdef MAP_R2
	wdev->MAPCfg.primary_vid = INVALID_VLAN_ID;
	wdev->MAPCfg.primary_pcp = 0x08;
	wdev->MAPCfg.vid_num = 0;
	wdev->MAPCfg.fh_vid = INVALID_VLAN_ID;
	NdisZeroMemory(wdev->MAPCfg.vids, sizeof(wdev->MAPCfg.vids));
	NdisZeroMemory(wdev->MAPCfg.bitmap_trans_vlan, sizeof(wdev->MAPCfg.bitmap_trans_vlan));
#endif
	pAd->ApCfg.SteerPolicy.steer_policy = 0;
	pAd->ApCfg.SteerPolicy.cu_thr = 0;
	pAd->ApCfg.SteerPolicy.rcpi_thr = 0;
	NdisZeroMemory(wdev->MAPCfg.vendor_ie_buf, VENDOR_SPECIFIC_LEN);
	wdev->MAPCfg.vendor_ie_len = 0;
	NdisZeroMemory(&(wdev->MAPCfg.scan_bh_ssids), sizeof(struct scan_BH_ssids));
#ifdef MAP_R2
	/*// TODO: Raghav: enable per client Tx/Rx airtime calculation*/
#endif

}

#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
extern struct bss_manager bss_mngr;

static struct bmgr_entry *get_bss_entry_by_netdev(
		IN PNET_DEV pNetDev)
{
	struct bmgr_entry *entry = NULL;
	int i;

	for (i = 0; BMGR_VALID_BSS_IDX(i); i++) {
		entry = bss_mngr.entry[i];
		if (entry && (entry->pNetDev == pNetDev))
			return entry;
	}

	return NULL;
}
#endif

INT map_send_bh_sta_wps_done_event(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY mac_entry,
	IN BOOLEAN is_ap)
{
	struct wifi_dev *wdev;
	struct wapp_event event;
	BOOLEAN send_event = FALSE;
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
	struct bmgr_entry *entry = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *link_entry = NULL;
	int j = 0, i = 0;
#ifdef MAP_R6
	int mld_mac_updated = 0;
#endif /*MAP_R6*/
#endif
	if (mac_entry) {

#ifdef APCLI_SUPPORT
		PSTA_ADMIN_CONFIG apcli_entry;
		struct wapp_bhsta_info *bsta_info = &event.data.bhsta_info;
#ifdef MAP_R6
	memset(bsta_info, 0, sizeof(wapp_bhsta_info));
#endif
#endif

		if (is_ap) {
			if (IS_MAP_ENABLE(pAd) && (mac_entry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
				send_event = TRUE;
		}
#ifdef APCLI_SUPPORT
		else {
			apcli_entry = GetStaCfgByWdev(pAd, mac_entry->wdev);
			if (IS_MAP_ENABLE(pAd) &&
				(mac_entry->DevPeerRole &
					(BIT(MAP_ROLE_FRONTHAUL_BSS) | BIT(MAP_ROLE_BACKHAUL_BSS)))) {
				COPY_MAC_ADDR(bsta_info->connected_bssid, apcli_entry->wdev.bssid);
				COPY_MAC_ADDR(bsta_info->mac_addr, apcli_entry->wdev.if_addr);
				bsta_info->peer_map_enable = 1;
				send_event = TRUE;
#ifdef MAP_R6
				if (apcli_entry->wdev.mld_dev) {
					memset(bsta_info->sta_mld_addr, 0, MAC_ADDR_LEN);
					memcpy(bsta_info->sta_mld_addr, apcli_entry->wdev.mld_dev->mld_addr, MAC_ADDR_LEN);
				}
				bsta_info->ssid_len = apcli_entry->SsidLen;
				memset(bsta_info->ssid, 0, MAX_LEN_OF_SSID);
				memcpy(bsta_info->ssid, apcli_entry->Ssid,  bsta_info->ssid_len);
#endif
			} else {
				bsta_info->peer_map_enable = 0;
				send_event = TRUE;
			}
		}
#endif
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
			wdev = mac_entry->wdev;
			entry = get_bss_entry_by_netdev(wdev->if_dev);
			if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
				if (!BMGR_VALID_MLO_DEV(entry->mld_ptr)) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"Entry not affiliated to MLO GRP.\n");
					if (send_event == TRUE && is_ap == 0)
						bsta_info->setup_link_num = 1;
					goto out;
				}
			}

			mt_rcu_read_lock();
			mld_entry = get_mld_entry_by_mac(mac_entry->mlo.mld_addr);

			if (!mld_entry) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"(ERROR):mld_entry=NULL\n");
				if (send_event == TRUE && is_ap == 0)
					bsta_info->setup_link_num = 1;
				goto out;
			}

			if (MAC_ADDR_EQUAL(mac_entry->mlo.mld_addr, ZERO_MAC_ADDR)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"(ERROR):peer_mld_addr=zero\n");
				if (send_event == TRUE && is_ap == 0)
					bsta_info->setup_link_num = 1;
				goto out;
			}

			if (mld_entry->link_num == 0 ||
					mld_entry->link_num > BSS_MNGR_MAX_BAND_NUM) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"(ERROR):link_num=%d\n", mld_entry->link_num);
				if (send_event == TRUE && is_ap == 0)
					bsta_info->setup_link_num = 1;
				goto out;
			}

#ifdef MAP_R6
			bsta_info->sta.mld_mac_valid  = 1;
			bsta_info->sta.bss_mld_mac_valid  = 1;
			/* Need to add Proper handling when supported */
			bsta_info->sta.str_en_dis  = 1;
			bsta_info->sta.nstr_en_dis  = 0;
			bsta_info->sta.emlsr_en_dis  = 1;
			bsta_info->sta.emlmr_en_dis  = 0;
#endif
			for (i = 0; i < MLD_LINK_MAX; i++) {
				link_entry = (struct _MAC_TABLE_ENTRY *)mld_entry->link_entry[i];
				if (!link_entry)
					continue;

				if (IS_MAP_ENABLE(pAd) && link_entry->wdev) {
#ifdef MAP_R6
					/* Fetch MLD MAC Address*/
					if (mld_mac_updated == 0 && link_entry->wdev->mld_dev) {
						mld_mac_updated = 1;
						COPY_MAC_ADDR(bsta_info->sta.mld_mac, link_entry->wdev->mld_dev->mld_addr);
						MTWF_DBG(pAd,
							DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
							"DEbug:"MACSTR"\n", MAC2STR(link_entry->wdev->mld_dev->mld_addr));
					}
#endif

					j++;
					bsta_info->setup_link_num = j;
					bsta_info->linkinfo[j - 1].mlo_enable = link_entry->mlo.mlo_en;
					bsta_info->linkinfo[j - 1].is_setup_link_entry = link_entry->mlo.is_setup_link_entry;
					COPY_MAC_ADDR(bsta_info->linkinfo[j-1].connected_bssid, link_entry->wdev->bssid);
					COPY_MAC_ADDR(bsta_info->linkinfo[j-1].mac_addr, link_entry->wdev->if_addr);
#ifdef MAP_R6
					if (MAC_ADDR_EQUAL(bsta_info->linkinfo[j-1].mac_addr, bsta_info->sta.mld_mac)) {
						MTWF_DBG(pAd,
							DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
							"NAKUL DEbug: MLD MAC ADDRESS EQUAL\n");
						COPY_MAC_ADDR(bsta_info->sta.bss_mld_mac, mac_entry->mlo.mld_addr);
					}
#endif

					if (WMODE_CAP_6G(link_entry->wdev->PhyMode))
						bsta_info->linkinfo[j-1].phymode = 4;
					else if (WMODE_CAP_5G(link_entry->wdev->PhyMode))
						bsta_info->linkinfo[j-1].phymode = 2;
					else if (WMODE_CAP_2G(link_entry->wdev->PhyMode))
						bsta_info->linkinfo[j-1].phymode = 1;
					else
						bsta_info->linkinfo[j-1].phymode = 255;
					bsta_info->linkinfo[j-1].channel = link_entry->wdev->channel;
#ifdef MAP_R6
					bsta_info->linkinfo[j-1].ifindex =  RtmpOsGetNetIfIndex(link_entry->wdev->if_dev);
#endif
					send_event = TRUE;
				}
			}
out:
		mt_rcu_read_unlock();
#endif
		if (mac_entry->wdev->bh_ready_sent) {
			send_event = FALSE;
		}

		if (send_event) {
			wdev = mac_entry->wdev;
			event.event_id = MAP_BH_STA_WPS_DONE;
			event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
			wapp_send_wapp_qry_rsp(pAd, &event);
		}
	}

	return 0;
}

void wapp_send_rssi_steer_event(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	char rssi_thrd)
{
	struct wifi_dev *wdev;
	wdev_steer_sta *str_sta;
	struct wapp_event event;

	/* send event to daemon */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_DEBUG, "Trigger Rssi steering!\n");
	pEntry->isTriggerSteering = TRUE;

	wdev = pEntry->wdev;
	event.event_id = MAP_TRIGGER_RSSI_STEER;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	str_sta = &event.data.str_sta;
	COPY_MAC_ADDR(str_sta->mac_addr, pEntry->Addr);
	wapp_send_wapp_qry_rsp(pAd, &event);
}

VOID map_rssi_status_check(
	IN PRTMP_ADAPTER pAd)
{
	UINT16 i = 0;
	char rssi_thrd = 0;

	if (pAd->ApCfg.SteerPolicy.steer_policy == AGENT_INIT_RSSI_STEER_ALLOW ||
		pAd->ApCfg.SteerPolicy.steer_policy == AGENT_INIT_RSSI_STEER_MANDATE) {
		rssi_thrd = (pAd->ApCfg.SteerPolicy.rcpi_thr >> 1) - 110;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);

#ifdef SW_CONNECT_SUPPORT
			/* TBD : RSSI  of SW STA RSSI not support by query WM */
			if (hc_is_sw_wcid(pAd, i))
				continue;
#endif /* SW_CONNECT_SUPPORT */

			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				if (pEntry->RssiSample.AvgRssi[0] < rssi_thrd) {
					pEntry->cur_rssi_status = BELOW_THRESHOLD;

					/*
					*	If sta's rssi is within RCPI tollenant boundary,
					*	ignore this rssi detection to avoid sending event
					*	to wapp constantly
					*/
					if (pEntry->isTriggerSteering == TRUE &&
						pEntry->pre_rssi_status == ABOVE_THRESHOLD &&
						pEntry->cur_rssi_status == BELOW_THRESHOLD &&
						abs(pEntry->RssiSample.AvgRssi[0] - rssi_thrd) <= RCPI_TOLLENACE)
						return;

					wapp_send_rssi_steer_event(pAd, pEntry, rssi_thrd);
				} else
					pEntry->cur_rssi_status = ABOVE_THRESHOLD;

				if (pEntry->pre_rssi_status == ABOVE_THRESHOLD &&
					pEntry->cur_rssi_status == BELOW_THRESHOLD &&
					abs(pEntry->RssiSample.AvgRssi[0] - rssi_thrd) > RCPI_TOLLENACE)
						pEntry->isTriggerSteering = FALSE;

				pEntry->pre_rssi_status = pEntry->cur_rssi_status;
			}
		}
	}
}

#ifdef MAP_R2
UINT32 is_vid_configed(UINT16 vid, UINT32 vids[])
{
	return vids[vid / 32] & BIT(vid % 32);
}
#endif

#ifdef MAP_TS_TRAFFIC_SUPPORT

#define PKT_TYPE_1905 0x893a
#define ETH_TYPE_SVLAN  0X88A8

BOOLEAN get_vlanid_from_pkt(PNDIS_PACKET pkt, UINT16 *pvlanid)
{
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);
	struct vlan_ethhdr *veth = (struct vlan_ethhdr *)skb->data;

	if (veth->h_vlan_proto != OS_HTONS(ETH_P_8021Q) &&
		veth->h_vlan_proto != OS_HTONS(ETH_TYPE_SVLAN))
		return FALSE;

	*pvlanid = (OS_NTOHS(veth->h_vlan_TCI) & 0x0FFF);
	return TRUE;
}

static inline VOID remove_vlan_tag(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UCHAR *pSrcBuf;
	UINT16 VLAN_LEN = 4;
	UCHAR extra_field_offset = 2 * ETH_ALEN;
	struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);

	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
	ASSERT(pSrcBuf);
	memmove(GET_OS_PKT_DATAPTR(pkt) + VLAN_LEN,
		GET_OS_PKT_DATAPTR(pkt), extra_field_offset);
	RtmpOsSkbPullRcsum(RTPKT_TO_OSPKT(pkt), 4);
	RtmpOsSkbResetMacHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetNetworkHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetTransportHeader(RTPKT_TO_OSPKT(pkt));
	RtmpOsSkbResetMacLen(RTPKT_TO_OSPKT(pkt));
	skb->vlan_tci = 0;
}

BOOLEAN add_vlan_tag(void *packet, UINT16 vlan_id, UCHAR vlan_pcp)
{
	struct sk_buff *skb = (struct sk_buff *)packet;
	UINT16 vlan_tci = 0;

	vlan_tci |= 0x0fff & vlan_id;
	vlan_tci |= vlan_pcp << 13;

	skb = vlan_insert_tag(skb, htons(ETH_P_8021Q), vlan_tci);
	if (skb) {
		skb->protocol = htons(ETH_P_8021Q);
		skb->vlan_tci = htons(vlan_tci);
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOLEAN is_ts_configed(struct wifi_dev *wdev)
{
	if (wdev->MAPCfg.primary_vid != INVALID_VLAN_ID ||
		wdev->MAPCfg.vid_num != 0)
		return TRUE;

	return FALSE;
}

static inline UINT16 map_get_dev_vid(struct wifi_dev *wdev)
{
	if (!wdev)
		return 0;
	if (IS_VALID_VID(wdev->MAPCfg.fh_vid))
		return wdev->MAPCfg.fh_vid;
	else if (IS_VALID_VID(wdev->MAPCfg.primary_vid))
		return wdev->MAPCfg.primary_vid;
	else
		return 0;
}

BOOLEAN map_ts_tx_process(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					PNDIS_PACKET pkt, struct _MAC_TABLE_ENTRY *peer_entry)
{
	UINT16 pkt_vid = 0, conf_vid = 0;
	BOOLEAN vlan_tagged = FALSE;
	UCHAR *pSrcBuf = NULL;
	UINT16 pkt_type = 0;

	if (!IS_MAP_R2_ENABLE(pAd))
		goto suc;
	conf_vid = map_get_dev_vid(wdev);
	vlan_tagged = get_vlanid_from_pkt(pkt, &pkt_vid);
	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);

	pkt_type = (pSrcBuf[12] << 8) | pSrcBuf[13];
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"%s() on %s, DevPeerRole=%02x, profile=%02x, vlan_tagged:%d, DA:"MACSTR" SA:"MACSTR", pkt_type0x%x pkt_vid(%d) conf_vid(%d)\n",
		__func__, wdev->if_dev->name, peer_entry->DevPeerRole, peer_entry->profile, vlan_tagged,
		MAC2STR(pSrcBuf), MAC2STR(pSrcBuf+6), pkt_type, pkt_vid, conf_vid);

	/*pass through all vlan tagged packet with transparent vlan id*/
	if (vlan_tagged && is_vid_configed(pkt_vid, wdev->MAPCfg.bitmap_trans_vlan))
		goto suc;

	if (!is_ts_configed(wdev))
		goto suc;

	if (vlan_tagged) {
		if (peer_entry->DevPeerRole == 0) {
			/*normal sta*/
			if (pkt_vid == conf_vid) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"map_ts_tx_process %s remove the matching vid=%d for station\n",
					wdev->if_dev->name, pkt_vid);
				remove_vlan_tag(pAd, pkt);
			} else
				goto fail;
		} else if (peer_entry->profile < 0x02) {
			if (pkt_vid != wdev->MAPCfg.primary_vid) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"map_ts_tx_process %s drop pkts with vid(%d) not equal to primary vlan(%d)\n",
					wdev->if_dev->name, pkt_vid, wdev->MAPCfg.primary_vid);
				goto fail;
			}
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"map_ts_tx_process %s remove tag for r1 vid=%d\n",
					wdev->if_dev->name, pkt_vid);
			/*map r1 device*/
			remove_vlan_tag(pAd, pkt);
		} else if (peer_entry->profile >= 0x02) {
			/*	map r2 device check whether the vid is included
			 *	in the recent received ts policy-TBD
			 */
		}
	} else {
		if (peer_entry->profile >= 0x02) {
			/*	should we assume that if primary vlan id has been
			 *	configured, we should add the primary vid to these
			 *	packets without any vlan tags, including the 1905 and
			 *	EAPOL message???
			 */
			/* tag all packets sending on R2/R3 Backhual */
			if (IS_VALID_VID(wdev->MAPCfg.primary_vid)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
						"map_ts_tx_process %s add tag for r2 vid=%d to untaged ptk\n",
						wdev->if_dev->name, wdev->MAPCfg.primary_vid);
				/*add primary vlan id for 1905 message*/
				if (!add_vlan_tag(pkt, wdev->MAPCfg.primary_vid, wdev->MAPCfg.primary_pcp))
					goto fail;
			}
		}
	}
suc:
	return TRUE;
fail:
	return FALSE;
}

BOOLEAN map_ts_rx_process(RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
					PNDIS_PACKET pkt, struct _MAC_TABLE_ENTRY *peer_entry)
{

	UINT16 pkt_vid = 0;
	BOOLEAN vlan_tagged = FALSE;
	UCHAR *pSrcBuf;
	UINT16 pkt_type = 0;

	if (!IS_MAP_R2_ENABLE(pAd))
		goto suc;

	vlan_tagged = get_vlanid_from_pkt(pkt, &pkt_vid);
	pSrcBuf = GET_OS_PKT_DATAPTR(pkt);

	pkt_type = (pSrcBuf[12] << 8) | pSrcBuf[13];
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"%s() on %s, "MACSTR" DevPeerRole=%02x, profile=%02x\n",
		__func__, wdev->if_dev->name, MAC2STR(peer_entry->Addr),
		peer_entry->DevPeerRole, peer_entry->profile);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"vlan_tagged:%d, DA:"MACSTR", SA:"MACSTR", pkt_type0x%x pkt_vid(%d) conf_vid(%d)\n",
		vlan_tagged, MAC2STR(pSrcBuf), MAC2STR(pSrcBuf+6), pkt_type, pkt_vid,
		map_get_dev_vid(wdev));
	if (vlan_tagged && is_vid_configed(pkt_vid, wdev->MAPCfg.bitmap_trans_vlan))
		goto suc;

	if (!is_ts_configed(wdev))
		goto suc;

	if (vlan_tagged) {
		if (peer_entry->DevPeerRole == 0 ||
			peer_entry->profile < 0x02) {
			/*	normal sta drop the packet with vlan which is send from
			 *	normal station or map r1 device
			 */
			if (wdev->MAPCfg.fh_vid != INVALID_VLAN_ID) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"%s drop ptk vid=%d,entry=%p,DevPeerRole=%02x, profile=%02x\n",
					wdev->if_dev->name, pkt_vid,
					peer_entry, peer_entry->DevPeerRole,
					peer_entry->profile);
				goto fail;
			}
		} else if (peer_entry->profile >= 0x02) {
			/*	map r2 device check whether the vid is included
			 *	in the recent received ts policy
			*/
			/*check if it is vlan eapol pkts*/
			if (RTMPEqualMemory(EAPOL, pSrcBuf + LENGTH_802_3_NO_TYPE + LENGTH_802_1Q, 2) &&
				wdev->MAPCfg.primary_vid == pkt_vid) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_NOTICE,
					"%s receive vlan eapol, remove tag\n", wdev->if_dev->name);
				remove_vlan_tag(pAd, pkt);
				goto suc;
			}
			if (is_ts_configed(wdev) &&
				!is_vid_configed(pkt_vid, wdev->MAPCfg.vids) &&
				wdev->MAPCfg.primary_vid != pkt_vid) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"map_ts_rx_process %s drop ptk with vid =%d that is not in ts policy\n",
					wdev->if_dev->name, pkt_vid);
				goto fail;
			}
		}
	} else {
		if (peer_entry->DevPeerRole == 0) {
			/*do not add tag for eapol for nomal sta*/
			if (RTMPEqualMemory(EAPOL, pSrcBuf + LENGTH_802_3_NO_TYPE, 2)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_NOTICE,
					"%s do not add tag for eapol from normal sta\n",
					wdev->if_dev->name);
				goto suc;
			}
			/*if this bss has been configured, add the corresponding vid*/
			if (IS_VALID_VID(wdev->MAPCfg.fh_vid) ||
				IS_VALID_VID(wdev->MAPCfg.primary_vid)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"map_ts_rx_process %s add for station vid=%d\n",
					wdev->if_dev->name, wdev->MAPCfg.fh_vid);
				if (!add_vlan_tag(pkt, IS_VALID_VID(wdev->MAPCfg.fh_vid) ?
					wdev->MAPCfg.fh_vid : wdev->MAPCfg.primary_vid, wdev->MAPCfg.primary_pcp))
					goto fail;
			}
		} else if (peer_entry->profile < 0x02) {
			if (IS_VALID_VID(wdev->MAPCfg.primary_vid)) {
				/*if received a packet from map r1 device, add a vlan tag*/
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
						"map_ts_rx_process %s add for R1 with primary vid=%d\n",
						wdev->if_dev->name,
						wdev->MAPCfg.primary_vid);
				if (!add_vlan_tag(pkt, wdev->MAPCfg.primary_vid, wdev->MAPCfg.primary_pcp))
					goto fail;
			}
		} else {
			/*	if received a packet without vlan from map r2 device,
			 *	waht should do??
			 */
		}
	}

suc:
	return TRUE;
fail:
	return FALSE;
}
#endif

#ifdef A4_CONN
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
VOID map_a4_mlo_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PSTA_ADMIN_CONFIG pApCliEntry,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
	UCHAR link = 0, link_index = 0;
	struct _MAC_TABLE_ENTRY *entry_ptr;
	struct _RTMP_ADAPTER *ad = NULL;
	struct mld_entry_t *mld_entry = NULL;
	struct _MAC_TABLE_ENTRY *tmp_entry_ptr[MLD_LINK_MAX];
	struct mld_dev *mld = NULL;

	if (is_ap) {
		if (IS_MAP_ENABLE(adapter) &&
			(pEntry->wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			(pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA))) {

				/* mlo case */
				mt_rcu_read_lock();
				mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);

				if (!mld_entry) {
					MTWF_DBG(adapter, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"ERROR, mld_entry = NULL\n");
					mt_rcu_read_unlock();
					return;
				}

				for (link_index = 0; link_index < MLD_LINK_MAX; link_index++)
					tmp_entry_ptr[link_index] = mld_entry->link_entry[link_index];

				mt_rcu_read_unlock();

				do {
					entry_ptr = tmp_entry_ptr[link++];
					if (!entry_ptr)
						continue;
					ad = entry_ptr->pAd;
					if (!ad)
						continue;

					a4_ap_peer_enable(ad, entry_ptr, A4_TYPE_MAP);
				} while (link < MLD_LINK_MAX);
		}
	}
#ifdef APCLI_SUPPORT
	else {
		if (IS_MAP_ENABLE(adapter) &&
			(pEntry->DevPeerRole & (BIT(MAP_ROLE_BACKHAUL_BSS)))) {
			if (pEntry->mlo.mlo_en) {
				struct wifi_dev *wdev = &pApCliEntry->wdev;
				PSTA_ADMIN_CONFIG pStaCfg;

				mld = wdev->mld_dev;
				if (mld == NULL)
					return;

				for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
					if (mld->peer_mld.single_link[link].active && mld->mld_own_links[link].used) {
						entry_ptr = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[link].priv_ptr;
						if (!entry_ptr)
							continue;
						ad = entry_ptr->pAd;
						if (!ad)
							continue;

						pStaCfg = GetStaCfgByWdev(ad, mld->mld_own_links[link].wdev);
						a4_apcli_peer_enable(ad, pStaCfg, entry_ptr, A4_TYPE_MAP);
					}
				}
			}
		}
	}
#endif
}
#endif
BOOLEAN map_a4_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
#ifdef APCLI_SUPPORT
	PSTA_ADMIN_CONFIG apcli_entry;
#endif

	if (is_ap) {
		if (IS_MAP_ENABLE(adapter) &&
			(entry->wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
			(entry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
			return a4_ap_peer_enable(adapter, entry, A4_TYPE_MAP);
	}
#ifdef APCLI_SUPPORT
	else {
		apcli_entry = GetStaCfgByWdev(adapter, entry->wdev);
		if (IS_MAP_ENABLE(adapter) &&
			(entry->DevPeerRole & (BIT(MAP_ROLE_BACKHAUL_BSS)))) {
			return a4_apcli_peer_enable(adapter,
										apcli_entry,
										entry,
										A4_TYPE_MAP);
		}
	}
#endif

	return FALSE;
}
#if (defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE))
VOID map_a4_mlo_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PSTA_ADMIN_CONFIG pApCliEntry,
	IN PMAC_TABLE_ENTRY entry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
	if (is_ap)
		a4_ap_peer_disable(adapter, entry, A4_TYPE_MAP);
#ifdef APCLI_SUPPORT
	else {
		UCHAR i;
		struct _MAC_TABLE_ENTRY *entry_ptr;
		struct wifi_dev *wdev = &pApCliEntry->wdev;
		struct _RTMP_ADAPTER *ad;
		PSTA_ADMIN_CONFIG pStaCfg;
		struct mld_dev *mld = wdev->mld_dev;

		if (mld == NULL)
			return;

		for (i = 0; i < MLD_LINK_MAX; i++) {
			entry_ptr = (MAC_TABLE_ENTRY *)mld->peer_mld.single_link[i].priv_ptr;
			if (!entry_ptr)
				continue;

			ad = entry_ptr->pAd;
			if (!ad)
				continue;

			pStaCfg = GetStaCfgByWdev(ad, mld->mld_own_links[i].wdev);
			if (!pStaCfg)
				continue;

			a4_apcli_peer_disable(ad, pStaCfg, entry_ptr, A4_TYPE_MAP);
		}
	}
#else

#endif
}
#endif
BOOLEAN map_a4_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
	if (is_ap)
		return a4_ap_peer_disable(adapter, entry, A4_TYPE_MAP);
#ifdef APCLI_SUPPORT
	else
		return a4_apcli_peer_disable(adapter, GetStaCfgByWdev(adapter, entry->wdev), entry, A4_TYPE_MAP);
#else
	return FALSE;
#endif
}


BOOLEAN map_a4_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
)
{
	return a4_interface_init(adapter, if_index, is_ap, A4_TYPE_MAP);
}


BOOLEAN map_a4_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
)
{
	return a4_interface_deinit(adapter, if_index, is_ap, A4_TYPE_MAP);
}
BOOLEAN MapNotRequestedChannel(struct wifi_dev *wdev, unsigned char channel)
{
	int i = 0;

	if (wdev->MAPCfg.scan_bh_ssids.scan_channel_count == 0)
		return FALSE;
	for (i = 0; i < wdev->MAPCfg.scan_bh_ssids.scan_channel_count; i++) {
		if (channel == wdev->MAPCfg.scan_bh_ssids.scan_channel_list[i])
			return FALSE;
	}
	return TRUE;
}
#endif

/* Blacklist for BS2.0 */
#ifdef MAP_BL_SUPPORT
BOOLEAN map_is_entry_bl(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR apidx)
{
	PLIST_HEADER pBlackList = &pAd->ApCfg.MBSSID[apidx].BlackList;
	RT_LIST_ENTRY *pListEntry = pBlackList->pHead;
	PBS_BLACKLIST_ENTRY	pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;

	while (pBlEntry != NULL) {
		if (NdisEqualMemory(pBlEntry->addr, pAddr, MAC_ADDR_LEN))
			return TRUE;

		pListEntry = pListEntry->pNext;
		pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
	}

	return FALSE;
}

PBS_BLACKLIST_ENTRY	map_find_bl_entry(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr)
{
	PBS_BLACKLIST_ENTRY	pBlEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = pBlackList->pHead;
	pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;

	while (pBlEntry != NULL) {
		if (NdisEqualMemory(pBlEntry->addr, pMacAddr, MAC_ADDR_LEN))
			return pBlEntry;

		pListEntry = pListEntry->pNext;
		pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
	}

	return NULL;
}

VOID map_blacklist_add(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr)
{
	PBS_BLACKLIST_ENTRY pBlEntry = NULL;

	pBlEntry = map_find_bl_entry(pBlackList, pMacAddr);

	if (pBlEntry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"entry already presnet\n");
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "New entry add\n");
		os_alloc_mem(NULL, (UCHAR **)&pBlEntry, sizeof(BS_BLACKLIST_ENTRY));
		if (pBlEntry) {
			NdisZeroMemory(pBlEntry, sizeof(BS_BLACKLIST_ENTRY));
			NdisMoveMemory(pBlEntry->addr, pMacAddr, MAC_ADDR_LEN);
			insertTailList(pBlackList, (RT_LIST_ENTRY *)pBlEntry);
		}
		ASSERT(pBlEntry != NULL);
	}
}

VOID map_blacklist_del(
	IN  PLIST_HEADER pBlackList,
	IN  PUCHAR pMacAddr)
{
	RT_LIST_ENTRY *pListEntry = NULL;

	pListEntry = (RT_LIST_ENTRY *)map_find_bl_entry(pBlackList, pMacAddr);

	if (pListEntry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"pMacAddr = "MACSTR"\n", MAC2STR(pMacAddr));
		delEntryList(pBlackList, pListEntry);
		os_free_mem(pListEntry);
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"Entry not present in list ["MACSTR"]\n", MAC2STR(pMacAddr));
	}
}

VOID map_blacklist_show(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR apidx)
{
	BSS_STRUCT *pBss = &pAd->ApCfg.MBSSID[apidx];
	PLIST_HEADER pBlackList = &pBss->BlackList;
	PBS_BLACKLIST_ENTRY	pBlEntry = NULL;
	RT_LIST_ENTRY *pListEntry = NULL;

	if (pBlackList->size != 0) {
		OS_SEM_LOCK(&pBss->BlackListLock);
		pListEntry = pBlackList->pHead;
		pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
		while (pBlEntry != NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"STA :: "MACSTR"\n", MAC2STR(pBlEntry->addr));
			pListEntry = pListEntry->pNext;
			pBlEntry = (PBS_BLACKLIST_ENTRY)pListEntry;
		}
		OS_SEM_UNLOCK(&pBss->BlackListLock);
	}
}
#endif /*  MAP_BL_SUPPORT */

#ifdef MTK_MLO_MAP_SUPPORT
USHORT mtk_map_parse_assoc_rsnie_pmkid_match(
	IN RTMP_ADAPTER * pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN IE_LISTS * ie_list)
{
	INT cacheidx;
	UCHAR *peer_mac = pEntry->Addr;
	UCHAR is_mlo_connect = FALSE;
	UCHAR *own_mac = pEntry->wdev->bssid;
	struct _MAC_TABLE_ENTRY *entry_ptr;
	UCHAR link_num = 0;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
#endif /* DOT11_EHT_BE */

#ifdef DOT11_EHT_BE
	if (pEntry->mlo.mlo_en) {
		is_mlo_connect = TRUE;
		own_mac = pEntry->wdev->bss_info_argument.mld_info.mld_addr;
		peer_mac = pEntry->mlo.mld_addr;
		if (MAC_ADDR_EQUAL(peer_mac, ZERO_MAC_ADDR)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"peer_mac invalid\n");
			return MLME_UNSPECIFY_FAIL;
		}

		/* If AKM for connection is DPP then extract all sta
		 * links mac address and compare with PMKID entry.
		 * if nothing is matched then give INVALID_PMKID
		 * error
		 */
		if (IS_AKM_DPP(pEntry->SecConfig.AKMMap)) {
			is_mlo_connect = FALSE;

			mt_rcu_read_lock();
			mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);

			if (!mld_entry) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"ERROR, mld_entry = NULL\n");
				mt_rcu_read_unlock();
				return MLME_UNSPECIFY_FAIL;
			}
			for (link_num = 0; link_num < MLD_LINK_MAX ; link_num++) {
				entry_ptr = mld_entry->link_entry[link_num];

				if (!entry_ptr)
					continue;

				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"%s - ASSOC REQ MLO LINK MAC link:%u "MACSTR"\n",
					__func__, link_num, MAC2STR(entry_ptr->Addr));

				peer_mac = entry_ptr->Addr;

				if (is_rsne_pmkid_cache_match(ie_list->RSN_IE,
							ie_list->RSNIE_Len,
							PD_GET_PMKID_PTR(pAd->physical_dev),
							own_mac,
							peer_mac,
							is_mlo_connect,
							&cacheidx)) {
					if (cacheidx != INVALID_PMKID_IDX) {
						MTWF_DBG(pAd,
							DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
							"ASSOC - CacheIdx = %d link_num:%u\n",
							cacheidx, link_num);
						break;
					}
								}
			}
			mt_rcu_read_unlock();
			if (link_num == MLD_LINK_MAX) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"PMKSA cache not match\n");
				return MLME_INVALID_PMKID;
			}
		}
	}
#endif

	if ((IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap)
		|| (IS_AKM_DPP(pEntry->SecConfig.AKMMap)
#ifdef DOT11_EHT_BE
		&& !(pEntry->mlo.mlo_en)
#endif /* DOT11_EHT_BE */
		))
		&& is_rsne_pmkid_cache_match(ie_list->RSN_IE,
				ie_list->RSNIE_Len,
				PD_GET_PMKID_PTR(pAd->physical_dev),
				own_mac,
				peer_mac,
				is_mlo_connect,
				&cacheidx)
		&& (cacheidx == INVALID_PMKID_IDX)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"PMKSA cache not match\n");
		return MLME_INVALID_PMKID;
	}
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "PMKID match\n");
	return MLME_SUCCESS;
}
#endif /* MTK_MLO_MAP_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
int mtk_nl80211_easymesh_set_ssid(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN apcliEn;
	PSTA_ADMIN_CONFIG pApCliEntry;
#endif /* CONFIG_STA_SUPPORT */
	struct DOT11_H *pDot11h = NULL;
	BSS_STRUCT *pMbss = NULL;

	if (cmd_len > MAX_LEN_OF_SSID) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"%s parse set ssid failed - [ERROR]data_len large than %d.\n",
			__func__, MAX_LEN_OF_SSID);
		return -EINVAL;
	}

#ifdef APCLI_CFG80211_SUPPORT
	if (wl_dev->iftype == NL80211_IFTYPE_STATION) {
		ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);
		if (ifIndex >= MAX_MULTI_STA || ifIndex == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				 "error station index\n");
			return -EINVAL;
		}
		pApCliEntry = &pAd->StaCfg[ifIndex];
		wdev = &pApCliEntry->wdev;
		/* bring apcli interface down first */
		apcliEn = pApCliEntry->ApcliInfStat.Enable;

		if (apcliEn == TRUE) {
			pApCliEntry->ApcliInfStat.Enable = FALSE;
			ApCliIfDown(pAd);
		}

		pApCliEntry->ApcliInfStat.bPeerExist = FALSE;
		NdisZeroMemory(pApCliEntry->CfgSsid, MAX_LEN_OF_SSID);
		memcpy(pApCliEntry->CfgSsid, cmd, cmd_len);
		pApCliEntry->CfgSsidLen = cmd_len;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"I/F(apcli%d) OID_APCLI_SSID::(Len=%d,Ssid=%s)\n",
			ifIndex, pApCliEntry->CfgSsidLen,
			pApCliEntry->CfgSsid);
		pApCliEntry->ApcliInfStat.Enable = apcliEn;
		return 0;
	}
#endif /* APCLI_CFG80211_SUPPORT */

	if (wl_dev->iftype == NL80211_IFTYPE_AP) {
		ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
		if (ifIndex == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"ifIndex = WDEV_NOT_FOUND\n");
			return -EINVAL;
		}
		if (ifIndex >= MAX_MBSSID_NUM(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"ifIndex(%d) large than MAX_MBSSID_NUM(%d)\n",
				ifIndex, MAX_MBSSID_NUM(pAd));
			return -EINVAL;
		}
		pMbss = &pAd->ApCfg.MBSSID[ifIndex];
		wdev = &pMbss->wdev;
		NdisZeroMemory(pMbss->Ssid, MAX_LEN_OF_SSID);
		memcpy(pMbss->Ssid, cmd, cmd_len);
		pMbss->SsidLen = cmd_len;

		if (wdev == NULL)
			return -EINVAL;

		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			return -EINVAL;

		OPSTATUS_CLEAR_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
		ap_send_broadcast_deauth(pAd, wdev);
		if (IS_SECURITY(&wdev->SecConfig))
			pMbss->CapabilityInfo |= 0x0010;
		else
			pMbss->CapabilityInfo &= ~(0x0010);
		APSecInit(pAd, wdev);
		restart_ap(&pMbss->wdev);
		OPSTATUS_SET_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
		if ((pAd->CommonCfg.need_fallback == 1) &&
			(wlan_config_get_ch_band(&pMbss->wdev) == CMD_CH_BAND_24G)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"Need fallback to 20 MHz: pMbss->wdev.channel:%d\n", pMbss->wdev.channel);
			wlan_operate_set_ht_bw(&pMbss->wdev, HT_BW_20, EXTCHA_NONE);
		}
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"I/F(ra%d) Set_SSID::(Len=%d,Ssid=%s)\n",
			ifIndex, pMbss->SsidLen, pMbss->Ssid);
#ifdef QOS_R1
		qos_set_dscp2up_mapping(pAd, pMbss);
#endif /*QOS_R1*/
	}

	return 0;
}

static RTMP_STRING *BAND_STR[] = {"Invalid", "2.4G", "5G", "2.4G/5G"};
#ifdef MBSS_SUPPORT
static BOOLEAN wmode_valid(RTMP_ADAPTER *pAd, enum WIFI_MODE wmode)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((WMODE_CAP_6G(wmode) && (!PHY_CAP_6G(cap->phy_caps))) ||
		(WMODE_CAP_5G(wmode) && (!PHY_CAP_5G(cap->phy_caps))) ||
	    (WMODE_CAP_2G(wmode) && (!PHY_CAP_2G(cap->phy_caps))) ||
	    (WMODE_CAP_N(wmode) && RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
	   )
		return FALSE;
	else
		return TRUE;
}

INT nl80211_set_mbss_wirless_mode(RTMP_ADAPTER *pAd, UCHAR cfg_mode)
{
	USHORT wmode;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	wmode = cfgmode_2_wmode(cfg_mode);

	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			 "Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
			 cfg_mode, wmode,
			  BAND_STR[pChipCap->phy_caps & 0x3]);
		return FALSE;
	}

	if (WMODE_CAP_5G(wmode) && WMODE_CAP_2G(wmode)) {
		if (pAd->CommonCfg.dbdc_mode == 1) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"AP cannot support 2.4G/5G band mxied mode!\n");
			return FALSE;
		}
	}

	pAd->CommonCfg.cfg_wmode = wmode;
	return TRUE;
}
#endif /* MBSS_SUPPORT */


INT nl80211_set_wirless_mode(RTMP_ADAPTER *pAd, UCHAR cfg_mode, struct wifi_dev *wdev)
{
	USHORT wmode;
	UCHAR *mode_str;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode(cfg_mode);

	if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			 "Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
			 cfg_mode, wmode,
			  BAND_STR[pChipCap->phy_caps & 0x3]);
		return FALSE;
	}

	if (wmode_band_equal(wdev->PhyMode, wmode) == TRUE)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"wmode_band_equal(): Band Equal!\n");
	else
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"wmode_band_equal(): Band Not Equal!\n");

	wdev->PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	mode_str = wmode_2_str(wmode);

	if (mode_str) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"Set WMODE=%s(0x%x)\n", mode_str, wmode);
		os_free_mem(mode_str);
	}

	return TRUE;
}


int mtk_nl80211_easymesh_set_wireless_mode(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UCHAR cfg_mode = *(UCHAR *)(cmd);
	USHORT wmode = cfgmode_2_wmode(cfg_mode);
	INT success;
#ifdef CONFIG_AP_SUPPORT
	UINT32 i = 0;
	struct wifi_dev *TmpWdev = NULL;
#endif
	CHANNEL_CTRL * pChCtrl;
	INT ifIndex;
#ifdef MT_DFS_SUPPORT
	UCHAR BandIdx = hc_get_hw_band_idx(pAd);
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (BandIdx >= RDD_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR, "Invalid BandIdx: %d!!\n",
					BandIdx);
		return -EINVAL;
	}
#endif
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "(cfg_mode=%d)\n", cfg_mode);

	if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR, "Invalid wmode!!\n");
		return -EINVAL;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "(wl_dev->iftype=%d)\n",
			wl_dev->iftype);

#ifdef APCLI_CFG80211_SUPPORT
	if (wl_dev->iftype == NL80211_IFTYPE_STATION) {
		UINT8 ba_en = 1;
		PSTA_ADMIN_CONFIG pStaCfg;
		SCAN_INFO *ScanInfo = NULL;
		BSS_TABLE *ScanTab = NULL;

		ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);
		if (ifIndex >= MAX_MULTI_STA || ifIndex == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"error station index\n");
			return -EINVAL;
		}

		pStaCfg = &pAd->StaCfg[ifIndex];
		wdev = &pAd->StaCfg[ifIndex].wdev;
		wdev->PhyMode = wmode;
		ScanInfo = &wdev->ScanInfo;
		ScanTab = get_scan_tab_by_wdev(pAd, wdev);
		success = nl80211_set_wirless_mode(pAd, cfg_mode, wdev);

		if (!success) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"set wireless mode failed!!\n");
			return -EINVAL;
		}

		hc_update_wdev(wdev);
		/* Change channel state to NONE */
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef MT_DFS_SUPPORT
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
#endif
		BuildChannelList(pAd, wdev);
		RTMPUpdateRateInfo(wmode, &wdev->rate);
		RTMPSetPhyMode(pAd, wdev, wmode);
		BssTableInit(ScanTab);
		ScanInfo->LastScanTime = 0;
#ifdef DOT11_N_SUPPORT
		ba_en = (WMODE_CAP_N(wmode)) ? 1 : 0;
		wlan_config_set_ba_enable(wdev, ba_en);
#endif /* DOT11_N_SUPPORT */

		/* Set AdhocMode rates*/
		if (pStaCfg->BssType == BSS_ADHOC) {
			MlmeUpdateTxRates(pAd, FALSE, 0);
			UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IF_STATE_CHG));
		}
	}
#endif /* APCLI_CFG80211_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	if (wl_dev->iftype == NL80211_IFTYPE_AP) {
		ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
		if (ifIndex == WDEV_NOT_FOUND) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"ifIndex = WDEV_NOT_FOUND\n");
			return -EINVAL;
		}
		if (ifIndex >= MAX_MBSSID_NUM(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"ifIndex(%d) large than MAX_MBSSID_NUM(%d)\n",
				ifIndex, MAX_MBSSID_NUM(pAd));
			return -EINVAL;
		}

		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		if (wdev == NULL) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"wdev is NULL\n");
			return -EINVAL;
		}

		wdev->PhyMode = wmode;

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"%s::(wdev->PhyMode=%d, wmode=%d)\n", __func__, wdev->PhyMode, wmode);
#ifdef MBSS_SUPPORT
		success = nl80211_set_mbss_wirless_mode(pAd, cfg_mode);

		if (!success) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"set mbss wireless mode failed!!\n");
			return -EINVAL;
		}

		for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
			TmpWdev = &pAd->ApCfg.MBSSID[i].wdev;

			/*update WmmCapable*/
			if (!wmode_band_equal(TmpWdev->PhyMode, wmode))
				continue;

			TmpWdev->bWmmCapable = pAd->ApCfg.MBSSID[i].bWmmCapableOrg;
		}

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "(BSS%d=%d)\n",
					ifIndex, wdev->PhyMode);
#else
		success = nl80211_set_wirless_mode(pAd, cfg_mode, wdev);

		if (!success) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"set wireless mode failed!!\n");
			return -EINVAL;
		}

#endif /*MBSS_SUPPORT*/
		hc_update_wdev(wdev);
		/* Change channel state to NONE */
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
#ifdef EXT_BUILD_CHANNEL_LIST
		BuildChannelListEx(pAd, wdev);
#else
#ifdef MT_DFS_SUPPORT
		pDfsParam->NeedSetNewChList = DFS_SET_NEWCH_ENABLED;
#endif
		BuildChannelList(pAd, wdev);
#endif
		RTMPUpdateRateInfo(wmode, &wdev->rate);
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		rtmpeapupdaterateinfo(wmode, &wdev->rate, &wdev->eap);
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
		RTMPSetPhyMode(pAd, wdev, wmode);
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
	}
#endif /* CONFIG_AP_SUPPORT */

	return 0;
}

int mtk_nl80211_easymesh_set_channel(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UCHAR Channel = *(UCHAR *)(cmd);
	INT ifIndex;
#ifdef TR181_SUPPORT
	UCHAR old_channel = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
#endif /*TR181_SUPPORT*/
	INT32 success = FALSE;
	INT ret = 0;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

#ifdef TR181_SUPPORT
	old_channel = wdev->channel;
#endif

#ifdef WIFI_MD_COEX_SUPPORT
	if (!IsChannelSafe(pAd, Channel)) {
		if (IsPwrChannelSafe(pAd, Channel))
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_WARN,
				"caller:%pS. The channel %d is power backoff channel\n",
				OS_TRACE, Channel);
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"caller:%pS. The channel %d is in unsafe channel list!!\n",
				OS_TRACE, Channel);
			return -EINVAL;
		}
	}
#endif

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DfsDedicatedExamineSetNewCh(pAd, wdev, Channel);
	DedicatedZeroWaitStop(pAd, TRUE);
#endif

	/*To do set channel, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"TakeChannelOpCharge fail for SET channel!!\n");
		return -EINVAL;
	}

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (pAd->CommonCfg.DfsParameter.BW160ZeroWaitSupport == TRUE)
		pAd->CommonCfg.DfsParameter.BW160ZeroWaitState = DFS_SET_CHANNEL;
#endif

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"Channel(%d), Cert(%d), Quick(%d)\n",
			Channel, pAd->CommonCfg.wifi_cert, wdev->quick_ch_change);

	pAd->ApCfg.iwpriv_event_flag = TRUE;
	RTMP_OS_REINIT_COMPLETION(&pAd->ApCfg.set_ch_aync_done);
#ifdef TR181_SUPPORT
	success = rtmp_set_channel(pAd, wdev, Channel);

	if (success && (old_channel != Channel)) {
		if (ctrl) {
			ctrl->rdev.pRadioCtrl->ManualChannelChangeCount++;
			ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"success = %d	Manual:%d Total:%d\n",
				success, ctrl->rdev.pRadioCtrl->ManualChannelChangeCount,
				ctrl->rdev.pRadioCtrl->TotalChannelChangeCount);
		}
	}
#else
#ifdef DFS_CAC_R2
	if (IS_MAP_ENABLE(pAd) || IS_MAP_TURNKEY_ENABLE(pAd)) {
		success = rtmp_set_channel(pAd, wdev, Channel);
		if (success == FALSE)
			wapp_send_cac_stop(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), wdev->channel, FALSE);
		/*return success; after set channel finished,then return iwpriv.*/
	} else
#endif
		success = rtmp_set_channel(pAd, wdev, Channel);
#endif
	if (pAd->ApCfg.set_ch_async_flag == TRUE) {
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((80*100*OS_HZ)/1000));/*Wait 8s.*/
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"wait channel setting success.\n");
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"wait channel setting timeout.\n");
			pAd->ApCfg.set_ch_async_flag = FALSE;
		}
	}
	pAd->ApCfg.iwpriv_event_flag = FALSE;

	/*if channel setting is DONE, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);

	return success ? 0 : -EFAULT;
}

int mtk_nl80211_easymesh_set_htbw(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UCHAR Bandidx = 0;
	UCHAR HtBw = *(UCHAR *)(cmd);
	INT ifIndex;
	UCHAR i = 0;
	struct wifi_dev *tdev;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	Bandidx = HcGetBandByWdev(wdev);

	if ((HtBw != BW_40) && (HtBw != BW_20))
		return -EINVAL;  /*Invalid argument */

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev))) {
			if ((tdev->wdev_type == WDEV_TYPE_AP) &&
				(bcn_bpcc_op_lock(pAd, tdev, TRUE, BCN_BPCC_HEOP) == FALSE))
				MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);

			if (HtBw == BW_40) {
				wlan_config_set_ht_bw(tdev, BW_40);
				wlan_operate_set_ht_bw(tdev, HT_BW_40, wlan_operate_get_ext_cha(tdev));
			} else {
				wlan_config_set_ht_bw(tdev, BW_20);
				wlan_operate_set_ht_bw(tdev, HT_BW_20, EXTCHA_NONE);
			}
			SetCommonHtVht(pAd, tdev);
			if (tdev->wdev_type == WDEV_TYPE_AP)
				UpdateBeaconHandler_BPCC(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG),
								BCN_BPCC_HEOP, TRUE);
		}
	}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Update Beacon to Reflect BW Changes */
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd))
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
#endif /* BW_VENDOR10_CUSTOM_FEATURE */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "(HtBw=%d)\n",
			wlan_config_get_ht_bw(wdev));

	return 0;
}
#ifdef CONFIG_MAP_SUPPORT
int mtk_nl80211_easymesh_set_ehtbw(
	IN RTMP_ADAPTER * pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UCHAR Bandidx = 0;
	UCHAR eht_bw = *(UCHAR *)(cmd);
	INT ifIndex;
	UCHAR i = 0;
	struct wifi_dev *tdev;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	Bandidx = HcGetBandByWdev(wdev);

	if (eht_bw <= EHT_BW_320) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"case 1 (eht_bw=%d)\n", eht_bw);
	} else {
		eht_bw = EHT_BW_2040;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"case 2 (eht_bw=%d)\n", eht_bw);
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR, "(eht_bw=%d)\n", eht_bw);

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev))) {
			if ((tdev->wdev_type == WDEV_TYPE_AP) &&
				(bcn_bpcc_op_lock(pAd, tdev, TRUE, BCN_BPCC_HEOP) == FALSE))
				MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);

			wlan_config_set_eht_bw(tdev, eht_bw);
			if (WMODE_CAP_BE_6G(tdev->PhyMode)) {
				wlan_operate_set_eht_bw(tdev, eht_bw);
			}
			SetCommonHtVht(pAd, tdev);
			if (tdev->wdev_type == WDEV_TYPE_AP)
				UpdateBeaconHandler_BPCC(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG),
										BCN_BPCC_HEOP, TRUE);
		}
	}

#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Update Beacon to Reflect BW Changes */
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd))
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
#endif /* BW_VENDOR10_CUSTOM_FEATURE */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "(EHT_BW=%d)\n", eht_bw);

	return 0;
}
#endif

INT mtk_nl80211_set_mld_group_create(
	IN RTMP_ADAPTER * pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN struct mld_group *req_mld)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	struct eht_mld_param mld_param = {0};

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	eht_ap_mld_fill_default_mld_param(&mld_param);
	eht_ap_mld_create(wdev, &req_mld->mld_group_idx, req_mld->mld_addr, &mld_param, &req_mld->mld_type);
	return 0;
}

#ifdef MAP_R6
int mtk_nl80211_set_mld_link_reconf(
	IN RTMP_ADAPTER * pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN struct mld_reconf *req_mld)
{
	struct wifi_dev *wdev;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	eht_ap_mld_trig_link_reconfiguration(wdev, req_mld->reconfig_to);

	return 0;
}

#endif /* MAP_R6 */

int mtk_nl80211_easymesh_set_mld_group_delete(
	IN RTMP_ADAPTER * pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	UCHAR mld_group_idx = *(UCHAR *)(cmd);

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	eht_ap_mld_destroy(wdev, mld_group_idx);
	return 0;
}

int mtk_nl80211_easymesh_set_mld_del_link(
	IN RTMP_ADAPTER * pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;

#ifdef CONFIG_MAP_SUPPORT
	UINT8 orig_reject_mgmt_rx;
#endif


	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		/*ignore mgmt during reconfiguration*/
		/*disconnect all sta,for 1st draft,will remove later*/
		MacTableResetWdev(pAd, wdev);
		orig_reject_mgmt_rx = pAd->ApCfg.reject_mgmt_rx;
		pAd->ApCfg.reject_mgmt_rx = MGMT_RX_REJECT_ALL;
	}
#endif

	eht_ap_mld_del_link(wdev);

#ifdef CONFIG_MAP_SUPPORT
	/* restore setting*/
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		pAd->ApCfg.reject_mgmt_rx = orig_reject_mgmt_rx;
#endif

	return 0;
}

int mtk_nl80211_easymesh_set_mld_link_add(
	IN RTMP_ADAPTER * pAd,
	IN struct wireless_dev *wl_dev,
	IN struct mld_add_link *req_mld)
{
	struct wifi_dev *wdev;
	UCHAR mld_group_idx = req_mld->mld_id;
#ifdef MAP_R6
	u16 is_reconfig;
#endif /* MAP_R6 */
#ifdef CONFIG_MAP_SUPPORT
	UINT8 orig_reject_mgmt_rx;
#endif


#ifdef MAP_R6
	is_reconfig = req_mld->is_reconfig;
#endif /* MAP_R6 */

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
#ifdef MAP_R6
	if (!is_reconfig) {
#endif
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_TURNKEY_ENABLE(pAd)) {
			/*ignore mgmt during reconfiguration*/
			/*disconnect all sta,for 1st draft,will remove later*/
			MacTableResetWdev(pAd, wdev);
			orig_reject_mgmt_rx = pAd->ApCfg.reject_mgmt_rx;
			pAd->ApCfg.reject_mgmt_rx = MGMT_RX_REJECT_ALL;
		}
#endif
#ifdef MAP_R6
	}
#endif /* MAP_R6 */

	eht_ap_mld_add_link(wdev, mld_group_idx);

#ifdef MAP_R6
	if (!is_reconfig) {
#endif
#ifdef CONFIG_MAP_SUPPORT
		/* restore setting*/
		if (IS_MAP_TURNKEY_ENABLE(pAd))
			pAd->ApCfg.reject_mgmt_rx = orig_reject_mgmt_rx;
#endif
#ifdef MAP_R6
	}
#endif /* MAP_R6 */

	return 0;
}

int mtk_nl80211_easymesh_set_mld_link_transfer(
	IN RTMP_ADAPTER * pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	UCHAR mld_group_idx = *(UCHAR *)(cmd);

#ifdef CONFIG_MAP_SUPPORT
	UINT8 orig_reject_mgmt_rx;
#endif

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		/*ignore mgmt during reconfiguration*/
		/*disconnect all sta,for 1st draft,will remove later*/
		MacTableResetWdev(pAd, wdev);
		orig_reject_mgmt_rx = pAd->ApCfg.reject_mgmt_rx;
		pAd->ApCfg.reject_mgmt_rx = MGMT_RX_REJECT_ALL;
	}
#endif

	eht_ap_mld_link_transfer(wdev, mld_group_idx);

#ifdef CONFIG_MAP_SUPPORT
	/* restore setting*/
	if (IS_MAP_TURNKEY_ENABLE(pAd))
		pAd->ApCfg.reject_mgmt_rx = orig_reject_mgmt_rx;
#endif
	return 0;
}

int mtk_nl80211_easymesh_set_vhtbw(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UCHAR Bandidx = 0;
	UCHAR vht_cw = *(UCHAR *)(cmd);
	UCHAR vht_bw;
	INT ifIndex;
	UCHAR i = 0;
	struct wifi_dev *tdev;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	Bandidx = HcGetBandByWdev(wdev);

	if (vht_cw <= VHT_BW_8080) {
		vht_bw = vht_cw;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"case 1 (vht_bw=%d)\n", vht_bw);
	} else {
		vht_bw = VHT_BW_2040;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"case 2 (vht_bw=%d)\n", vht_bw);
	}

#ifdef DFS_ADJ_BW_ZERO_WAIT
	if (IS_ADJ_BW_ZERO_WAIT(pAd->CommonCfg.DfsParameter.BW160ZeroWaitState) == TRUE)
		pAd->CommonCfg.DfsParameter.band_bw = vht_bw;
#endif

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR, "(vht_bw=%d)\n", vht_bw);

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		tdev = pAd->wdev_list[i];
		if (tdev && (Bandidx == HcGetBandByWdev(tdev))) {
			if ((tdev->wdev_type == WDEV_TYPE_AP) &&
				(bcn_bpcc_op_lock(pAd, tdev, TRUE, BCN_BPCC_HEOP) == FALSE))
				MTWF_PRINT("%s(%d): bcn_bpcc_op_lock fail!\n", __func__, __LINE__);

			wlan_config_set_vht_bw(tdev, vht_bw);
			if (WMODE_CAP_AC(tdev->PhyMode)) {
				wlan_operate_set_vht_bw(tdev, vht_bw);
			}

			SetCommonHtVht(pAd, tdev);
			if (tdev->wdev_type == WDEV_TYPE_AP)
				UpdateBeaconHandler_BPCC(pAd, tdev, BCN_REASON(BCN_UPDATE_IE_CHG),
								BCN_BPCC_HEOP, TRUE);
		}
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "(vht_bw=%d)aaa\n", vht_bw);

#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Update Beacon to Reflect BW Changes */
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd))
		UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG));
#endif /* BW_VENDOR10_CUSTOM_FEATURE */
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "(VHT_BW=%d)\n", vht_bw);

	return 0;
}

int mtk_nl80211_easymesh_set_hidden_ssid(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	BOOLEAN bHideSsid;
	INT ifIndex;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	if (ifIndex < 0) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find ifIndex .\n");
		return -EINVAL;
	}
	bHideSsid = *(BOOLEAN *)(cmd);

	if (bHideSsid == 1)
		bHideSsid = TRUE;
	else if (bHideSsid == 0)
		bHideSsid = FALSE;
	else
		return FALSE;  /*Invalid argument */

	if (pAd->ApCfg.MBSSID[ifIndex].bHideSsid != bHideSsid)
		pAd->ApCfg.MBSSID[ifIndex].bHideSsid = bHideSsid;

#ifdef WSC_V2_SUPPORT
	if (pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl.WscV2Info.bEnableWpsV2)
		WscOnOff(pAd, ifIndex, pAd->ApCfg.MBSSID[ifIndex].bHideSsid);

#endif /* WSC_V2_SUPPORT */
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"IF(ra%d) (HideSSID=%d)\n", ifIndex,
		pAd->ApCfg.MBSSID[ifIndex].bHideSsid);

	return 0;
}

int mtk_nl80211_easymesh_set_auth_mode(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	SetWdevAuthMode(&wdev->SecConfig, (RTMP_STRING *)cmd);
	return 0;
}

int mtk_nl80211_easymesh_set_encrypt_type(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	SetWdevEncrypMode(&wdev->SecConfig, (RTMP_STRING *)cmd);
	return 0;
}

int mtk_nl80211_easymesh_set_default_key_id(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;
	UCHAR KeyIdx = *(UCHAR *)(cmd);

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	if (ifIndex < 0) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't ifIndex .\n");
		return -EINVAL;
	}

	if ((KeyIdx >= 1) && (KeyIdx <= 4))
		wdev->SecConfig.PairwiseKeyId = (UCHAR) (KeyIdx - 1);
	else
		return -EINVAL;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "==> DefaultKeyId=%d\n",
			 wdev->SecConfig.PairwiseKeyId);
	return 0;
}

int mtk_nl80211_easymesh_set_key1(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;
	INT retVal = FALSE;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	retVal = ParseWebKey(&wdev->SecConfig, (RTMP_STRING *)cmd, 0, 0);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "KeyID=0, key=%s\n",
			(RTMP_STRING *)cmd);

	if (retVal)
		return 0;
	else
		return -EINVAL;
}

int mtk_nl80211_easymesh_set_wpa_psk(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;
	INT apcli_ifIndex;
	BSS_STRUCT *pMbss;
	INT i;
	RTMP_STRING *wpa_psk = (RTMP_STRING *)cmd;
	struct _NDIS_AP_802_11_PMKID *pmkid_cache;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	apcli_ifIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);

	if (ifIndex < 0 || ifIndex >= MAX_BEACON_NUM
		|| ifIndex >= pAd->ApCfg.BssidNum) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"invalid ifIndex\n");
		return -EINVAL;
	}
	if (apcli_ifIndex < 0 || apcli_ifIndex >= MAX_MULTI_STA
		|| apcli_ifIndex >= pAd->MSTANum) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"invalid apcli_ifIndex\n");
		return -EINVAL;
	}

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];

	if (strlen(wpa_psk) < 65) {
		if (strlen(wpa_psk) != strlen(wdev->SecConfig.PSK)
			|| !RTMPEqualMemory(wpa_psk, wdev->SecConfig.PSK, strlen(wpa_psk))
			) {
			pmkid_cache = PD_GET_PMKID_PTR(pAd->physical_dev);
			OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
			for (i = 0; i < MAX_PMKID_COUNT; i++) {
				if ((pmkid_cache->BSSIDInfo[i].Valid == TRUE)
					&& (MAC_ADDR_EQUAL(&pmkid_cache->BSSIDInfo[i].own_mac, pMbss->wdev.bssid))) {
					pmkid_cache->BSSIDInfo[i].Valid = FALSE;
					MTWF_PRINT("%s():Modify PSK and clear PMKID (idx %d)from (mbssidx %d)\n", __func__, i, pMbss->mbss_idx);
				}
			}
			OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
		}
#ifdef CONFIG_STA_SUPPORT
#ifdef APCLI_SUPPORT
		if (wdev->wdev_type == INT_APCLI) {
			BOOLEAN is_psk_same = 0;
			UCHAR i = 0;

			for (i = 0; i < (LEN_PSK + 1); i++) {
				if (wdev->SecConfig.PSK[i] != wpa_psk[i]) {
					is_psk_same = 0;
					break;
				}

				if (wdev->SecConfig.PSK[i] == '\0') {
					is_psk_same = 1;
					break;
				}
			}

			if (!is_psk_same) {
				/*PSK has changed we need to clear store apcli
				* pmk cache for AKM's that use PSK
				*/
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
						"Delete pmk cache on password change\n");
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
				sta_delete_psk_pmkid_cache_all(pAd, apcli_ifIndex);
#endif
			}
		}
#endif /* APCLI_SUPPORT */
#endif
			os_move_mem(wdev->SecConfig.PSK, wpa_psk, strlen(wpa_psk));
			wdev->SecConfig.PSK[strlen(wpa_psk)] = '\0';
		} else
			wdev->SecConfig.PSK[0] = '\0';

		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "PSK = %s\n",
				wdev->SecConfig.PSK);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			WSC_CTRL *pWscControl = NULL;

			if ((wdev->wdev_type == INT_MAIN || wdev->wdev_type == INT_MBSSID)) {
				UCHAR apidx = ifIndex;

				pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
			}
#ifdef APCLI_SUPPORT
			else if (wdev->wdev_type == INT_APCLI) {
				UCHAR apcli_idx = apcli_ifIndex;

				pWscControl = &pAd->StaCfg[apcli_idx].wdev.WscControl;
			}
#endif /* APCLI_SUPPORT */

			if (pWscControl) {
				NdisZeroMemory(pWscControl->WpaPsk, 64);
				pWscControl->WpaPskLen = 0;
				pWscControl->WpaPskLen = strlen(wpa_psk);
				NdisMoveMemory(pWscControl->WpaPsk, wpa_psk, pWscControl->WpaPskLen);
			}
		}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		return 0;
}

int mtk_nl80211_easymesh_set_radio_on(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	UCHAR radio = *(UCHAR *)(cmd);
	//INT ifIndex;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
#endif

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
		if (!pMbss) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				 "pMbss is NULL\n");
			return -EINVAL;
		}
	}
#endif

	if (!wdev->if_up_down_state) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			 "==>(%s) but IF is done, ignore!!! (wdev_idx %d)\n",
			 radio ? "ON" : "OFF", wdev->wdev_idx);
		return 0;
	}

	if (radio == !IsHcRadioCurStatOffByWdev(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			 "==>(%s) equal to current state, ignore!!! (wdev_idx %d)\n",
			 radio ? "ON" : "OFF", wdev->wdev_idx);
		return 0;
	}

	if (radio) {
		MlmeRadioOn(pAd, wdev);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "==>(ON)\n");
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
#ifdef FT_R1KH_KEEP
			/*
			 * Keep the R1KH table when Radio On is done twice for MBO-4.2.6(E)
			 * case to meet the R1KH miss case.
			 */
			pAd->ApCfg.FtTab.RadioOn++;

			if (pAd->ApCfg.FtTab.RadioOn <= RADIO_ON_SET)
				pAd->ApCfg.FtTab.FT_RadioOff = FALSE;

			if (pAd->ApCfg.FtTab.RadioOn == RADIO_ON_RESET)
				pAd->ApCfg.FtTab.RadioOn = FALSE;
#endif /* FT_R1KH_KEEP */
		}
#endif
		wdev->radio_off_req = FALSE;
	} else {
		wdev->radio_off_req = TRUE;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef FT_R1KH_KEEP
			pAd->ApCfg.FtTab.FT_RadioOff = TRUE;
#endif /* FT_R1KH_KEEP */
			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		}
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MSTAStop(pAd, wdev);
		}
#endif
#ifndef HWIFI_SUPPORT
		if (!hwctrl_cmd_q_empty(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"(OFF),cmd q not empty!\n");
		}
#endif /* HWIFI_SUPPORT */

		MlmeRadioOff(pAd, wdev);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "==>(OFF)\n");
	}

	return 0;
}

extern INT Set_DisConnectAllSta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
int mtk_nl80211_easymesh_set_disconnect_sta(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wifi_dev *wdev;
	UCHAR macAddr[MAC_ADDR_LEN];
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR *Addr = (UCHAR *)cmd;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	if (cmd_len != MAC_ADDR_LEN) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"MAC_ADDR Length error .\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_NOTICE,
				"aaa: %02x:%02x:%02x:%02x:%02x:%02x .\n", PRINT_MAC(Addr));

	COPY_MAC_ADDR(macAddr, Addr);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_NOTICE,
				"%02x:%02x:%02x:%02x:%02x:%02x .\n", PRINT_MAC(macAddr));

	if (NdisEqualMemory(&macAddr[0], &BROADCAST_ADDR[0], MAC_ADDR_LEN)) {
		Set_DisConnectAllSta_Proc(pAd, "2");
		return 0;
	}

	pEntry = MacTableLookup(pAd, macAddr);

	if (pEntry) {
#ifdef MAP_R2
		if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
			pEntry->DisconnectReason = REASON_DISASSOC_STA_LEAVING;
#endif
		MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
		/*		MacTableDeleteEntry(pAd, pEntry->wcid, Addr); */
	}

	return 0;
}

int mtk_nl80211_easymesh_set_ts_bh_primary_vid(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
#ifdef MAP_R2
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UINT16 vid = *(UINT16 *)(cmd);
	INT ifIndex;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	wdev->MAPCfg.primary_vid = vid;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"%s default vid=%d\n", wdev->if_dev->name, vid);
#endif /* MAP_R2 */
	return 0;
}

int mtk_nl80211_easymesh_set_ts_bh_primary_pcp(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
#ifdef MAP_R2
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UCHAR pcp = *(UCHAR *)(cmd);
	INT ifIndex;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	wdev->MAPCfg.primary_pcp = pcp;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"%s default pcp=%d\n", wdev->if_dev->name, pcp);
#endif /* MAP_R2 */
	return 0;
}

int mtk_nl80211_easymesh_set_ts_bh_vid(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
#ifdef MAP_R2
	UINT16 vid;
	UINT32 index = 0, offset = 0;
	RTMP_STRING *p = NULL;
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;
	RTMP_STRING *vid_str = (RTMP_STRING *)cmd;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	RTMPZeroMemory(wdev->MAPCfg.vids, sizeof(wdev->MAPCfg.vids));
	wdev->MAPCfg.vid_num = 0;

	while (1) {
		p = strsep(&vid_str, ",");
		if (!p || *p == '\0')
			break;
		vid = (UINT16) os_str_tol(p, 0, 10);
		if (vid >= INVALID_VLAN_ID || vid == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"%s invalid vid=%d\n", wdev->if_dev->name, vid);
			continue;
		}

		index = vid / (sizeof(UINT32) * 8);
		offset = vid % (sizeof(UINT32) * 8);

		if (!(wdev->MAPCfg.vids[index] & BIT(offset))) {
			wdev->MAPCfg.vids[index] |= BIT(offset);
			wdev->MAPCfg.vid_num++;
		}
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"%s bh vid vlan id=%d\n", wdev->if_dev->name, vid);
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"%s total vid_num=%d\n", wdev->if_dev->name, wdev->MAPCfg.vid_num);
#endif /* MAP_R2 */
	return 0;
}

int mtk_nl80211_easymesh_set_ts_fh_vid(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
#ifdef MAP_R2
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UINT16 vid = *(UINT16 *)(cmd);
	INT ifIndex;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	wdev->MAPCfg.fh_vid = vid;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"%s fh vid=%d\n", wdev->if_dev->name, vid);
#endif /* MAP_R2 */
	return 0;
}

int mtk_nl80211_easymesh_set_transparent_vid(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
#ifdef MAP_R2
	UINT16 vid;
	UINT32 index = 0, offset = 0;
	RTMP_STRING *p = NULL;

	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;
	RTMP_STRING *vid_str = (RTMP_STRING *)cmd;

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	RTMPZeroMemory(wdev->MAPCfg.bitmap_trans_vlan, sizeof(wdev->MAPCfg.bitmap_trans_vlan));

	while (1) {
		p = strsep(&vid_str, ",");
		if (!p || *p == '\0')
			break;
		vid = (UINT16) os_str_tol(p, 0, 10);
		if (vid >= INVALID_VLAN_ID || vid == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"%s invalid vid=%d\n", wdev->if_dev->name, vid);
			continue;
		}

		index = vid / (sizeof(UINT32) * 8);
		offset = vid % (sizeof(UINT32) * 8);

		wdev->MAPCfg.bitmap_trans_vlan[index] |= BIT(offset);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"%s transparent vlan id=%d\n",  wdev->if_dev->name, vid);
	}
#endif /* MAP_R2 */
	return 0;
}

#ifdef DOT11K_RRM_SUPPORT
int mtk_nl80211_easymesh_set_bcn_req(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	INT Loop;
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	INT ifIndex;
	UINT16 wcid = WCID_INVALID;
	UINT ArgIdx;
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */
	RTMP_STRING *thisChar;
	RRM_MLME_BCN_REQ_INFO BcnReq;
	RTMP_STRING *bcn_req_str = (RTMP_STRING *)cmd;
	UINT8 default_request_ie[MAX_NUM_OF_REQ_IE] = {
							0,		/* SSID */
							1, /* Support Rate*/

							50, /* Extended Support Rate*/

							45, /* HT IE*/
							61, /* HT ADD*/

							127, /* Ext Cap*/

							191, /* VHT 1*/
							192, /* VHT 2*/
							195, /* VHT 3*/

							48, /* RSN IE */
							70, /* RRM Capabilities. */
							54, /* Mobility Domain. */
							221
						};	/* Vendor Specific. */

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					 "vid_str: %s.\n", bcn_req_str);

	ArgIdx = 0;
	NdisZeroMemory(&BcnReq, sizeof(RRM_MLME_BCN_REQ_INFO));
	BcnReq.LastBcnRptInd = 0; /* set default LBRI to 0 */
	BcnReq.report_detail = 1; /*set default report detail to 1*/
	/*set default request ie*/
	BcnReq.request_ie_num = MAX_NUM_OF_REQ_IE;
	NdisCopyMemory(BcnReq.request_ie, default_request_ie, MAX_NUM_OF_REQ_IE);

	while ((thisChar = strsep((char **)&bcn_req_str, "!")) != NULL) {
		switch (ArgIdx) {
		case 0: /* Aid or Peer's MAC Addr */
			if (strlen(thisChar) == 17) {
				UCHAR mac_addr[MAC_ADDR_LEN];
				CHAR *token;
				MAC_TABLE_ENTRY *pEntry = NULL;
				INT i = 0;

				token = rstrtok(thisChar, ":");

				while (token != NULL) {
					AtoH(token, (char *) &mac_addr[i], 1);
					i++;
					if (i >= MAC_ADDR_LEN)
						break;
					token = rstrtok(NULL, ":");
			}

				pEntry = MacTableLookup(pAd, mac_addr);
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						 "%02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(mac_addr));

				if (pEntry != NULL)
					wcid = pEntry->wcid;
			} else {
				entrytb_aid_search_t aid_map;

				os_zero_mem(&aid_map, sizeof(entrytb_aid_search_t));
				if (kstrtou16(thisChar, 16, &aid_map.aid_search)) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						"aid_search transform failed\n");
					return 0;
				}

				if (entrytb_traversal(pAd, traversal_func_find_entry_by_aid, (void *)&aid_map) == TRUE) {
					if (aid_map.entry != NULL)
						wcid = aid_map.entry->wcid;
				}
			}

			if (!IS_WCID_VALID(pAd, wcid)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"unknown sta of wcid(%d)\n", wcid);
				return 0;
			}

#ifdef SW_CONNECT_SUPPORT
			if (IS_WCID_VALID(pAd, wcid)) {
				tr_entry = tr_entry_get(pAd, wcid);
				if (IS_SW_STA(tr_entry)) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"wcid(%d) is SW STA SKIP!!\n", wcid);
					return 0;
				}
			}
#endif /* SW_CONNECT_SUPPORT */

			break;

		case 1: /* Meausre Duration. */
			BcnReq.MeasureDuration = (UINT8) os_str_tol(thisChar, 0, 10);
			break;

		case 2: /* Regulator Class */
			BcnReq.RegulatoryClass = (UINT8) os_str_tol(thisChar, 0, 10);
			break;

		case 3: /* BSSID */
			if (strlen(thisChar) != 17) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
						 "invalid value BSSID.\n");
				return 0;
			}

			/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
			if (strlen(thisChar) == 17) {
				RTMP_STRING *value;

				for (Loop = 0, value = rstrtok(thisChar, ":"); value; value = rstrtok(NULL, ":")) {
					if ((strlen(value) != 2) || (!isxdigit((unsigned char)value[0])) || (!isxdigit((unsigned char)value[1])))
						return -EINVAL;  /*Invalid */

					AtoH(value, &BcnReq.Bssid[Loop++], 1);
				}

				if (Loop != 6)
					return 0;
			}

			break;

		case 4: /* SSID */
			BcnReq.pSsid = (PUINT8)thisChar;
			BcnReq.SsidLen = strlen(thisChar);
			break;

		case 5: /* measure channel */
			BcnReq.MeasureCh = (UINT8) os_str_tol(thisChar, 0, 10);
			break;

		case 6: /* measure mode. */
			BcnReq.MeasureMode = (UINT8) os_str_tol(thisChar, 0, 10);
			break;

		case 7: { /* regulatory class. */
			RTMP_STRING *RegClassString;
			UINT8 RegClassIdx;

			RegClassIdx = 0;

			while ((RegClassString = strsep((char **)&thisChar, "+")) != NULL) {
				BcnReq.ChRepRegulatoryClass[RegClassIdx] =
					(UINT8) os_str_tol(RegClassString, 0, 10);
				RegClassIdx++;

				if (RegClassIdx >= MAX_NUM_OF_REGULATORY_CLASS)
					break;
			}
		}
		break;

		case 8: { /* Channel Report  List. */
			RTMP_STRING *ChIdString;
			UINT8 ChId;

			ChId = 0;

			while ((ChIdString = strsep((char **)&thisChar, "#")) != NULL) {
				BcnReq.ChRepList[ChId] =
					(UINT8) os_str_toul(ChIdString, 0, 10);
				ChId++;

				if (ChId >= MAX_NUM_OF_CHS)
					break;
			}
		}
		break;
		case 9: /* report detail*/
			BcnReq.report_detail = (UINT8) os_str_tol(thisChar, 0, 10);
			break;
		case 10: { /* request ie */
			RTMP_STRING *req_ie_str;

			BcnReq.request_ie_num = 0;
			while ((req_ie_str = strsep((char **)&thisChar, "#")) != NULL) {
				if (req_ie_str[0] == '\0')
					break;

				BcnReq.request_ie[BcnReq.request_ie_num] =
					(UINT8) os_str_tol(req_ie_str, 0, 10);
				BcnReq.request_ie_num++;
			}

			if (BcnReq.request_ie_num == 0) {
				/*if request ie list is empty, add SSID ie to it*/
				BcnReq.request_ie_num = 1;
				BcnReq.request_ie[0] = 0;
			}
		}
		break;
		}

		ArgIdx++;
	}

	if (ArgIdx < 7 || ArgIdx > 11) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				 "invalid args (%d).\n", ArgIdx);
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				 "eg: iwpriv ra0 set BcnReq=<Aid>-<Duration>-<RegulatoryClass>-<BSSID>-<SSID>-<MeasureCh>-<MeasureMode>-<ChRegClass>-<ChReptList>\n");
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				 "eg: iwpriv ra0 set BcnReq=1!50!12!FF:FF:FF:FF:FF:FF!WiFi1!255!1!32+1!1#6#36#48\n");
		return 0;
	}

	BcnReq.BcnReqCapFlag.field.ReportCondition = TRUE;

	/* Add MeasureMode Sanity check of peer STA */
	if (VALID_UCAST_ENTRY_WCID(pAd, wcid)) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, wcid);

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"pEntry is	NULL!\n");
			return -EINVAL;
		}

		switch (BcnReq.MeasureMode) {
		case RRM_BCN_REQ_MODE_PASSIVE:
			if (!IS_RRM_BEACON_PASSIVE_MEASURE(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"invalid Measure Mode. %d, Peer STA Support(PASS:%d, ACT:%d, TAB:%d)!\n",
					BcnReq.MeasureMode, IS_RRM_BEACON_PASSIVE_MEASURE(pEntry), IS_RRM_BEACON_ACTIVE_MEASURE(pEntry), IS_RRM_BEACON_TABLE_MEASURE(pEntry));
					/* return TRUE; */
			}
			break;

		case RRM_BCN_REQ_MODE_ACTIVE:
			if (!IS_RRM_BEACON_ACTIVE_MEASURE(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"invalid Measure Mode. %d, Peer STA Support(PASS:%d, ACT:%d, TAB:%d)!\n",
					BcnReq.MeasureMode, IS_RRM_BEACON_PASSIVE_MEASURE(pEntry), IS_RRM_BEACON_ACTIVE_MEASURE(pEntry), IS_RRM_BEACON_TABLE_MEASURE(pEntry));
				/* return TRUE; */
			}
			break;

		case RRM_BCN_REQ_MODE_BCNTAB:
			if (!IS_RRM_BEACON_TABLE_MEASURE(pEntry)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"invalid Measure Mode. %d, Peer STA Support(PASS:%d, ACT:%d, TAB:%d)!\n",
					BcnReq.MeasureMode, IS_RRM_BEACON_PASSIVE_MEASURE(pEntry), IS_RRM_BEACON_ACTIVE_MEASURE(pEntry), IS_RRM_BEACON_TABLE_MEASURE(pEntry));
				/* return TRUE; */
			}
			break;

		default:
			{
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"invalid Measure Mode. %d\n", BcnReq.MeasureMode);
				/* return TRUE; */
			}
				break;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"invalid wcid\n");
		return -EINVAL;

	}

	BcnReq.BcnReqCapFlag.field.ReportCondition = TRUE;
	if (BcnReq.MeasureCh == 255)
		BcnReq.BcnReqCapFlag.field.ChannelRep = TRUE;
	else
		BcnReq.BcnReqCapFlag.field.ChannelRep = FALSE;

	BcnReq.RandInt = 0;
	RRM_EnqueueBcnReq(pAd, wcid, ifIndex, &BcnReq);
	return 0;
}
#endif

int mtk_nl80211_easymesh_set_HtBssCoex(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	UCHAR bBssCoexEnable = 0;
	struct wifi_dev *wdev;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	bBssCoexEnable = *(UCHAR *)(cmd);

	pAd->CommonCfg.bBssCoexEnable = ((bBssCoexEnable == 1) ? TRUE : FALSE);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"Set bBssCoexEnable=%d!\n", pAd->CommonCfg.bBssCoexEnable);

#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd) && pAd->CommonCfg.bBssCoexEnable) {
		/* Disable BSS Coex Enable Fields */
		pAd->CommonCfg.bBssCoexEnable = FALSE;
	}
#endif

	if ((pAd->CommonCfg.bBssCoexEnable == FALSE)
		&& pAd->CommonCfg.bRcvBSSWidthTriggerEvents) {
		/* switch back 20/40 */
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"Set bBssCoexEnable: Switch back 20/40.\n");
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

		if ((HcIsRfSupport(pAd, RFIC_24GHZ)) && (wlan_config_get_ht_bw(wdev) == BW_40))
			wlan_operate_set_ht_bw(wdev, HT_BW_40, wlan_config_get_ext_cha(wdev));
	}

	return 0;
}

int mtk_nl80211_easymesh_set_PMFMFPC(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	UCHAR PmfMfpcEnable = *(UCHAR *)(cmd);
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		UCHAR ApIndex = 0;

		ApIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);

		if (ApIndex >= pAd->ApCfg.BssidNum || ApIndex >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"ifIndex is invalid value\n");
			return -EINVAL;
		}

		if (PmfMfpcEnable)
			pAd->ApCfg.MBSSID[ApIndex].wdev.SecConfig.PmfCfg.Desired_MFPC = TRUE;
		else
			pAd->ApCfg.MBSSID[ApIndex].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"[PMF] :: apidx=%d, Desired MFPC=%d\n", ApIndex,
				pAd->ApCfg.MBSSID[ApIndex].wdev.SecConfig.PmfCfg.Desired_MFPC);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		UCHAR StaIndex = 0;

		StaIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);

		if (StaIndex >= pAd->MSTANum || StaIndex >= MAX_MULTI_STA) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"StaIndex is invalid value\n");
			return -EINVAL;
		}

		if (PmfMfpcEnable)
			pAd->StaCfg[StaIndex].wdev.SecConfig.PmfCfg.Desired_MFPC = TRUE;
		else
			pAd->StaCfg[StaIndex].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"[PMF]: staidx=%d, Desired MFPC=%d\n", StaIndex,
				pAd->StaCfg[StaIndex].wdev.SecConfig.PmfCfg.Desired_MFPC);
	}
#endif /* CONFIG_STA_SUPPORT */

	return 0;
}

int mtk_nl80211_easymesh_set_AutoRoaming(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	UCHAR AutoRoamingEnable = *(UCHAR *)(cmd);
	struct wifi_dev *wdev;
	struct net_device *pNetDev = wl_dev->netdev;
	UCHAR StaIndex = 0;
	PSTA_ADMIN_CONFIG pStaCfg;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	StaIndex = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);
	if (StaIndex >= pAd->MSTANum || StaIndex >= MAX_MULTI_STA) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"StaIndex is invalid value\n");
		return -EINVAL;
	}

	pStaCfg = &pAd->StaCfg[StaIndex];

	if (AutoRoamingEnable)
		pStaCfg->bAutoRoaming = TRUE;
	else
		pStaCfg->bAutoRoaming = FALSE;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"bAutoRoaming=%d\n", pStaCfg->bAutoRoaming);
	return 0;
}

#ifdef QOS_R2
int mtk_nl80211_easymesh_set_DSCPPolicyEnable(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	UCHAR DSCPPolicyEnable = *(UCHAR *)(cmd);
	struct wifi_dev *wdev;
	BSS_STRUCT *pmbss = NULL;
	UCHAR IfIdx = 0;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	IfIdx = wdev->func_idx;

	if (IfIdx < MAX_BEACON_NUM) {
		pmbss = &pAd->ApCfg.MBSSID[IfIdx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"only support on bss interface.\n");
		return -EINVAL;
	}

	if (!pmbss) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"no mbss was found, please check command.\n");
		return -EINVAL;
	}

	if (pmbss->bDSCPPolicyEnable == DSCPPolicyEnable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"DSCP Policy is already %s on %s\n",
			DSCPPolicyEnable ? "enabled":"disabled", pmbss->wdev.if_dev->name);
		return 0;
	}

	if (DSCPPolicyEnable)
		pmbss->bDSCPPolicyEnable = 1;
	else
		pmbss->bDSCPPolicyEnable = 0;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"DSCP Policy is %s on %s\n",
		pmbss->bDSCPPolicyEnable ? "enabled" : "disabled", pmbss->wdev.if_dev->name);

	return 0;
}

int mtk_nl80211_easymesh_set_QosMapCapaEnable(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	UCHAR QosMapCapaEnable = *(UCHAR *)(cmd);
	struct wifi_dev *wdev;
	BSS_STRUCT *pmbss = NULL;
	UCHAR IfIdx = 0;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	IfIdx = wdev->func_idx;

	if (IfIdx < MAX_BEACON_NUM) {
		pmbss = &pAd->ApCfg.MBSSID[IfIdx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"only support on bss interface.\n");
		return -EINVAL;
	}

	if (!pmbss) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"no mbss was found, please check command.\n");
		return -EINVAL;
	}

	if (pmbss->QosMapSupport == QosMapCapaEnable) {
		/* No need to do anything, current and previos values are same */
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"DSCP Policy is already %s on %s\n",
			QosMapCapaEnable ? "enabled":"disabled", pmbss->wdev.if_dev->name);
		return 0;
	}

	if (QosMapCapaEnable)
		pmbss->QosMapSupport = 1;
	else
		pmbss->QosMapSupport = 0;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
		"QoS Map Capability is %s on %s\n",
		pmbss->QosMapSupport ? "enabled" : "disabled", pmbss->wdev.if_dev->name);

	return 0;
}
#endif

int mtk_nl80211_easymesh_set_psk(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
#ifdef WSC_AP_SUPPORT
	struct net_device *pNetDev = wl_dev->netdev;
#endif /* WSC_AP_SUPPORT */
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
#ifdef MAP_R4
	BSS_STRUCT * pMbss = &pAd->ApCfg.MBSSID[pObj->ioctl_if];
	INT i;
#endif /* MAP_R4 */

	if (pSecConfig == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"pSecConfig == NULL\n");
		return -EINVAL;
	}

	if (cmd_len > 0 && cmd_len < 65) {
		memcpy(pSecConfig->PSK, cmd, cmd_len);
		pSecConfig->PSK[cmd_len] = '\0';
	} else
		pSecConfig->PSK[0] = '\0';

#ifdef MAP_R4
	if (!IS_AKM_DPP(pSecConfig->AKMMap)) {
		struct _NDIS_AP_802_11_PMKID *pmkid_cache;

		pmkid_cache = PD_GET_PMKID_PTR(pAd->physical_dev);
		OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
		for (i = 0; i < MAX_PMKID_COUNT; i++) {
			if ((pmkid_cache->BSSIDInfo[i].Valid == TRUE)
					&& (MAC_ADDR_EQUAL(&pmkid_cache->BSSIDInfo[i].own_mac, pMbss->wdev.bssid))) {
				pmkid_cache->BSSIDInfo[i].Valid = FALSE;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_WARN,
					"Modify PSK and clear PMKID (idx %d)from (mbssidx %d)\n",
					i, pMbss->mbss_idx);
			}
		}
		OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
	}
#endif /* MAP_R4 */

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "PSK = %s\n",
		pSecConfig->PSK);
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		WSC_CTRL *pWscControl = NULL;

		if (wl_dev->iftype == NL80211_IFTYPE_AP) {
			CHAR apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);

			if (apidx == WDEV_NOT_FOUND) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"apidx = WDEV_NOT_FOUND\n");
				return -EINVAL;
			}

			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		}
#ifdef APCLI_SUPPORT
		else if (wl_dev->iftype == NL80211_IFTYPE_STATION) {
			CHAR apcli_idx = CFG80211_FindStaIdxByNetDevice(pAd, pNetDev);

			if (apcli_idx == WDEV_NOT_FOUND) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"apcli_idx = WDEV_NOT_FOUND\n");
				return -EINVAL;
			}

			pWscControl = &pAd->StaCfg[apcli_idx].wdev.WscControl;
		}

#endif /* APCLI_SUPPORT */

		if (pWscControl) {
			NdisZeroMemory(pWscControl->WpaPsk, 64);
			pWscControl->WpaPskLen = 0;
			pWscControl->WpaPskLen = cmd_len;
			if (pWscControl->WpaPskLen <= 64)
				memcpy(pWscControl->WpaPsk, cmd, cmd_len);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "PSK = %s\n",
				pWscControl->WpaPsk);
		}
	}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef MAP_R4
#ifdef CONFIG_STA_SUPPORT
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		UCHAR sta_idx = pObj->ioctl_if;

		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_WARN,
				"%s():Delete pmk cache on password change\n",
				__func__);

		if (!IS_AKM_DPP(pSecConfig->AKMMap))
			sta_delete_psk_pmkid_cache_all(pAd, sta_idx);
	}
#endif /* APCLI_SUPPORT */
#endif
#endif /* MAP_R4 */

	return 0;
}

#ifdef DPP_SUPPORT
int mtk_nl80211_easymesh_set_pmk(
	IN RTMP_ADAPTER *pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct net_device *pNetDev = wl_dev->netdev;
	struct wifi_dev *wdev;
	struct pmk_req *pmk_data;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	INT ifIndex;
#if defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE)
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
#endif /* MTK_MLO_MAP_SUPPORT && DOT11_EHT_BE */

	if (!pAd->bDppEnable) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"DPP Disabled please enable it\n");
		return -EINVAL;
	}

	ifIndex = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetDev);
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	pSecConfig = &wdev->SecConfig;

	if (!IS_AKM_DPP(pSecConfig->AKMMap)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"DPP not supported, first set configuration\n");
		return -EINVAL;
	}

	pmk_data = (struct pmk_req *)cmd;
	NdisMoveMemory(pSecConfig->PMK, pmk_data->pmk, LEN_PMK);

	/* add one entry in pmk cache */
	if (wl_dev->iftype == NL80211_IFTYPE_AP) {
		struct _NDIS_AP_802_11_PMKID *pmkid_cache;

		pmkid_cache = PD_GET_PMKID_PTR(pAd->physical_dev);
		OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
		RTMPAddPMKIDCache(pmkid_cache,
				pmk_data->authenticator_addr,
				pmk_data->supplicant_addr,
				pmk_data->pmkid,
				pmk_data->pmk,
				FALSE,
				FALSE,
				pmk_data->pmk_len,
#ifndef RT_CFG80211_SUPPORT
				pAd->ApCfg.MBSSID[ifIndex].PMKCachePeriod
#else
				0
#endif
);
		OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
#if defined(MTK_MLO_MAP_SUPPORT) && defined(DOT11_EHT_BE)
		/* Adding one more PMKID entry with MLD
		 * MAC address for DPP MLO connection
		 */
		if ((entry != NULL) && BMGR_VALID_MLO_BSS_ENTRY(entry)
				&& !NdisEqualMemory(entry->mld_ptr->mld_addr, pmk_data->authenticator_addr, MAC_ADDR_LEN)) {
			NdisZeroMemory(pmk_data->authenticator_addr, MAC_ADDR_LEN);
			NdisMoveMemory(pmk_data->authenticator_addr,
					entry->mld_ptr->mld_addr, MAC_ADDR_LEN);
			OS_SEM_LOCK(&pmkid_cache->pmkid_lock);
			RTMPAddPMKIDCache(pmkid_cache,
					pmk_data->authenticator_addr,
					pmk_data->supplicant_addr,
					pmk_data->pmkid,
					pmk_data->pmk,
					FALSE,
					FALSE,
					pmk_data->pmk_len,
#ifndef RT_CFG80211_SUPPORT
					pAd->ApCfg.MBSSID[ifIndex].PMKCachePeriod
#else
					0
#endif
);
			OS_SEM_UNLOCK(&pmkid_cache->pmkid_lock);
		}
#endif /* MTK_MLO_MAP_SUPPORT && DOT11_EHT_BE */
	} else if (wl_dev->iftype == NL80211_IFTYPE_STATION) {
		UINT32 sec_akm = 0;

		SET_AKM_DPP(sec_akm);
		sta_add_pmkid_cache(pAd, pmk_data->supplicant_addr, pmk_data->pmkid,
			pmk_data->pmk, pmk_data->pmk_len, wdev->func_idx, wdev, sec_akm, pmk_data->ssid, pmk_data->ssidlen);

	}
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO, "<---");
	hex_dump("PMK", pSecConfig->PMK, pmk_data->pmk_len);

	return 0;
}

INT mtk_nl80211_send_dpp_frame(
	IN RTMP_ADAPTER *pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	struct wapp_event *event;
	struct wapp_dpp_action_frame *req_data = NULL;
	struct dpp_frame_list *dpp_frame;
	struct wifi_dev *wdev;
	UINT32 frm_id;
	unsigned int data_len;
	int ret;

	if (!pAd->bDppEnable) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"DPP Disabled please enable in driver\n");
		return -EINVAL;
	}

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	if (cmd_len < 4) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]invalid nla length %d .\n", cmd_len);
		return -EINVAL;
	}
	memcpy(&frm_id, cmd, 4);

	if (DlListEmpty(&wdev->dpp_frame_event_list)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"no frame latched on this wdev\n");
		return -EINVAL;
	}

	DlListForEach(dpp_frame, &wdev->dpp_frame_event_list, struct dpp_frame_list, List) {
		if (dpp_frame->dpp_frame_event->data.frame.wapp_dpp_frame_id_no == frm_id) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"Found Matching Frame ID\n");
			break;
		}
	}
	if (dpp_frame == NULL) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"DPP Action frame of ID %d not found in Buffer\n", frm_id);
		return -EINVAL;
	}

	event = (struct wapp_event *)dpp_frame->dpp_frame_event;
	req_data = (struct wapp_dpp_action_frame *)&(event->data.frame);

	data_len = sizeof(*req_data) + req_data->frm_len;
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd, wiphy, MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_DPP_FRAME,
					req_data, data_len);
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	DlListDel(&dpp_frame->List);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
			"list size is now=%d\n", DlListLen(&wdev->dpp_frame_event_list));
	os_free_mem(dpp_frame->dpp_frame_event);
	os_free_mem(dpp_frame);
	return ret;
}
#endif /* DPP_SUPPORT */

INT mtk_nl80211_get_assoc_req_frame(
	IN RTMP_ADAPTER *pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
	UCHAR *pStaMacAddr = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	int ret;

	os_alloc_mem(NULL, &pStaMacAddr, MAC_ADDR_LEN);
	if (!pStaMacAddr) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [MEM ERROR] malloc pStaMacAddr fail!!!\n");
		return -ENOMEM;
	}

	if (cmd_len > ASSOC_REQ_LEN) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]check cmd_len fail.\n");
		os_free_mem(pStaMacAddr);
		return -EINVAL;
	}

	if (cmd_len < MAC_ADDR_LEN) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]invalid nla length %d .\n", cmd_len);
		os_free_mem(pStaMacAddr);
		return -EINVAL;
	}

	memcpy(pStaMacAddr, cmd, MAC_ADDR_LEN);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
		"GET::(ASSOC REQ STA - "MACSTR")\n", MAC2STR(pStaMacAddr));

	pEntry = MacTableLookup(pAd, pStaMacAddr);
	if (pEntry) {
		ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd,
						wiphy,
						MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME,
						pEntry->assoc_req_frame,
						cmd_len);
	} else {
		ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd,
						wiphy,
						MTK_NL80211_VENDOR_ATTR_EASYMESH_GET_ASSOC_REQ_FRAME,
						pEntry->assoc_req_frame,
						0);
	}
	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	os_free_mem(pStaMacAddr);

	return ret;
}

int mtk_nl80211_easymesh_sr_enable(
	IN RTMP_ADAPTER * pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd)
{
#ifdef CFG_SUPPORT_FALCON_SR
	UINT_8 Enable;
	UCHAR band_idx = 0;
	struct wifi_dev *wdev;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	band_idx = HcGetBandByWdev(wdev);
	Enable = *(UCHAR *)(cmd);
	SrMeshSrEnable(pAd, band_idx, Enable);
#endif /* CFG_SUPPORT_FALCON_SR */
	return 0;
}

int mtk_nl80211_easymesh_set_bh_sr_bitmap(
	IN RTMP_ADAPTER * pAd,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd)
{
#ifdef CFG_SUPPORT_FALCON_SR
	UCHAR band_idx = 0;
	struct wifi_dev *wdev;
	struct SR_SRG_BITMAP_T *bh_sr_bitmap;

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}

	bh_sr_bitmap = (struct SR_SRG_BITMAP_T *)cmd;
	band_idx = HcGetBandByWdev(wdev);
	SrBHMeshSrgBitmap(pAd, band_idx, (UINT_8 *)bh_sr_bitmap);
#endif /* CFG_SUPPORT_FALCON_SR */
	return 0;
}


INT mtk_nl80211_get_spt_reuse_req(
	IN RTMP_ADAPTER * pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN const void *cmd,
	IN int cmd_len)
{
#ifdef CFG_SUPPORT_FALCON_SR
	struct wapp_mesh_sr_info srg_info;
	struct _BSS_INFO_ARGUMENT_T *bssinfo;
	struct bss_color_ctrl *bss_color;
	struct wifi_dev *wdev;
	UCHAR band_idx = 0;
	int ret;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"GET:: SPT REUSE REQ\n");

	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
		return -EINVAL;
	}
	if (cmd_len != sizeof(srg_info)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]check cmd_len fail.\n");
		return -EINVAL;
	}

	NdisZeroMemory(&srg_info, sizeof(srg_info));
	if (wdev) {
		band_idx = HcGetBandByWdev(wdev);
		bssinfo = &wdev->bss_info_argument;
		bss_color = &bssinfo->bss_color;
		srg_info.bss_color = bss_color->color;
		SrMeshGetSrMode(pAd, band_idx, (PUINT_8)&srg_info.sr_mode);
		SrMeshGetSrgBitmap(pAd, band_idx, (PUINT_8)&srg_info.bm_info);
	}
	ret = mtk_cfg80211_vndr_cmd_reply_msg(pAd,
			wiphy,
			MTK_NL80211_VENDOR_ATTR_GET_SPT_REUSE_REQ,
			&srg_info,
			cmd_len);

	if (ret != 0)
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"mtk_cfg80211_vndr_cmd_reply_msg fail!\n");

	return ret;
#else /* CFG_SUPPORT_FALCON_SR */
	return 0;
#endif /* !CFG_SUPPORT_FALCON_SR */
}

INT mtk_nl80211_set_map_channel(
	IN RTMP_ADAPTER *pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN struct map_ch *wapp_map_ch)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR if_idx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, if_idx, pObj->ioctl_if_type);
	INT32 success = FALSE;  /*FALSE = 0*/
	UINT32 i;
	INT ret = 0;
#ifdef TR181_SUPPORT
	UCHAR old_channel;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
#endif
	UCHAR Channel = 0;
#ifdef MAP_R2
	UCHAR cac_req;
	UCHAR dev_role;
#endif

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"wdev == NULL! if_type %d, if_idx = %d\n",
				pObj->ioctl_if_type,
				if_idx);
		return FALSE;
	}

	for (i = 0; i < MAX_BEACON_NUM; i++)
		pAd->ApCfg.MBSSID[i].wdev.cac_not_required = FALSE;

	Channel = wapp_map_ch->ch_num;
#ifdef MAP_R2
	cac_req = wapp_map_ch->cac_req;
	if (cac_req != SET_CH_ARG_NOT_REQ) {
#ifdef MT_DFS_SUPPORT
		if (cac_req == 0 && pAd->CommonCfg.DfsParameter.bDfsEnable) {
			for (i = 0; i < MAX_BEACON_NUM; i++) {
				if (pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
					pAd->ApCfg.MBSSID[i].wdev.cac_not_required = TRUE;
			}
		}
#endif
	}
	dev_role = wapp_map_ch->map_dev_role;
	if (dev_role != SET_CH_ARG_NOT_REQ) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
				"dev_role %d\n", dev_role);
		for (i = 0; i < MAX_BEACON_NUM; i++) {
			if (pAd->ApCfg.MBSSID[i].wdev.channel == wdev->channel)
				pAd->ApCfg.MBSSID[i].wdev.dev_role = dev_role;
		}
	}
#endif /* MAP_R2 */

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR, "\n");

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DfsDedicatedExamineSetNewCh(pAd, wdev, Channel);
	DedicatedZeroWaitStop(pAd, TRUE);
#endif

	/*To do set channel, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN, TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
				"TakeChannelOpCharge fail for SET channel!!\n");
		return FALSE;
	}
	pAd->ApCfg.iwpriv_event_flag = TRUE;
	RTMP_OS_REINIT_COMPLETION(&pAd->ApCfg.set_ch_aync_done);

#ifdef TR181_SUPPORT
	success = rtmp_set_channel(pAd, wdev, Channel);

	old_channel = wdev->channel;
	if (success && (old_channel != Channel)) {
		if (ctrl) {
			ctrl->rdev.pRadioCtrl->ManualChannelChangeCount++;
			ctrl->rdev.pRadioCtrl->TotalChannelChangeCount++;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"success = %d   Manual:%d Total:%d\n",
					success, ctrl->rdev.pRadioCtrl->ManualChannelChangeCount,
					ctrl->rdev.pRadioCtrl->TotalChannelChangeCount);
		}
	}
#else
	success = rtmp_set_channel(pAd, wdev, Channel);
#endif

	if (pAd->ApCfg.set_ch_async_flag == TRUE) {
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.set_ch_aync_done, ((80*100*OS_HZ)/1000));/*Wait 8s.*/
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_INFO,
					"wait channel setting success.\n");
		else {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"wait channel setting timeout.\n");
			pAd->ApCfg.set_ch_async_flag = FALSE;
		}
	}
	pAd->ApCfg.iwpriv_event_flag = FALSE;

	/*if channel setting is DONE, release ChannelOpCharge here*/
	ReleaseChannelOpCharge(pAd, wdev, CH_OP_OWNER_SET_CHN);

	return success;
}

INT mtk_nl80211_set_punc_bitmap(
	IN RTMP_ADAPTER * pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN u16 punctured_bitmap)
{
#ifdef CFG_SUPPORT_FALCON_PP
	char buf[20] = {0};
	int ret = 0;
	int ret_pp_cap = 0;
	UCHAR band_idx = 0;

	band_idx = hc_get_hw_band_idx(pAd);
	ret = snprintf(buf, sizeof(buf), "%d-0-2-%2x-1-0", band_idx, punctured_bitmap);
	if (os_snprintf_error(sizeof(buf), ret)) {
		MTWF_PRINT("%s: snprintf error\n", __func__);
		return FALSE;
	}
	ret_pp_cap = set_pp_cap_ctrl(pAd, buf);
	if (!ret_pp_cap) {
		MTWF_PRINT("%s failed - [ERROR]set_pp_cap_ctrl fail\n", __func__);
		return -EINVAL;
	}
#endif /* CFG_SUPPORT_FALCON_PP */
	return TRUE;
}

INT mtk_nl80211_set_map_enable(
	IN RTMP_ADAPTER *pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN u8 map_mode)
{
	UCHAR ifIndex = 0;
#ifdef CONFIG_STA_SUPPORT
	STA_ADMIN_CONFIG * pApCliEntry = NULL;
	PULONG pCurrState;
#endif /* CONFIG_STA_SUPPORT */
#ifdef WSC_INCLUDED
	PWSC_CTRL pWscControl = NULL;
#endif /* WSC_INCLUDED */
	struct wifi_dev *wdev = NULL;
	int i = 0;

	for (i = BSS0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		if (wdev != NULL)
			NdisZeroMemory(&(wdev->MAPCfg.scan_bh_ssids), sizeof(struct scan_BH_ssids));
	}

	for (i = BSS0; i < MAX_APCLI_NUM; i++) {
		STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[i];

		if (apcli_entry) {
			wdev = &apcli_entry->wdev;
			if (wdev != NULL)
				NdisZeroMemory(&(wdev->MAPCfg.scan_bh_ssids), sizeof(struct scan_BH_ssids));
		}
	}

	if (pAd->MAPMode == map_mode) {
		/* No need to do anything, current and previos values are same */
		MTWF_PRINT("%s current MAP MODE is %d\n", __func__, map_mode);
		return TRUE;
	}

	/* In case of enable we don't need to do anything
	* since upper layer should configure interface roles */
	pAd->MAPMode = map_mode;

	if (pAd->MAPMode != MAP_DISABLED) {
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_FW] &= ~(CATFW_STAREC);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_HIF] &= ~(CATHIF_HWIFI);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_MLME] &= ~(CATMLME_WTBL | CATMLME_AUTH | CATMLME_ASSOC);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_SEC] &= ~(CATSEC_KEY);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_CFG80211] &= ~(CATCFG80211_AP);
	} else {
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_FW] |= (CATFW_STAREC);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_HIF] |= (CATHIF_HWIFI);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_MLME] |= (CATMLME_WTBL | CATMLME_AUTH | CATMLME_ASSOC);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_SEC] |= (CATSEC_KEY);
		DebugSubCategory[DBG_LVL_NOTICE][DBG_CAT_CFG80211] |= (CATCFG80211_AP);
	}

	if (IS_MAP_TURNKEY_ENABLE(pAd)) {
		for (i = BSS0; i < MAX_APCLI_NUM; i++) {
			STA_ADMIN_CONFIG *apcli_entry = &pAd->StaCfg[i];

			if (apcli_entry) {
				wdev = &apcli_entry->wdev;
				if (wdev != NULL)
					/* TODO: Need to evaluate this for
					* hosapd-supplicant code */
					SetApCliEnableByWdev(pAd, wdev, FALSE);
			}
		}
	}

	if (!IS_MAP_ENABLE(pAd)) {
		/* Reset roles for For AP interfaces */
		for (ifIndex = 0; (ifIndex < MAX_MBSSID_NUM(pAd)); ifIndex++) {
			pAd->ApCfg.MBSSID[ifIndex].wdev.MAPCfg.DevOwnRole = BIT(MAP_ROLE_FRONTHAUL_BSS);
			APMlmeKickOutAllSta(pAd, ifIndex, REASON_DEAUTH_STA_LEAVING);
#ifdef WSC_INCLUDED
			pWscControl = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl;
#endif /* WSC_INCLUDED */

			reset_mtk_map_vendor_ie(pAd, &pAd->ApCfg.MBSSID[ifIndex].wdev);
#ifdef WSC_INCLUDED
			if (pWscControl->bWscTrigger == TRUE)
				WscStop(pAd, FALSE, pWscControl);
#endif /* WSC_INCLUDED */
		}

#ifdef CONFIG_STA_SUPPORT
		/* Reset Roles for CLI interfaces */
		for (ifIndex = 0; (ifIndex < MAX_APCLI_NUM); ifIndex++) {
			pApCliEntry = &pAd->StaCfg[ifIndex];
			pCurrState = &pAd->StaCfg[ifIndex].wdev.cntl_machine.CurrState;
			if (!pApCliEntry->ApcliInfStat.Valid)
				continue;
#ifdef WSC_INCLUDED
			pWscControl = &pAd->StaCfg[ifIndex].wdev.WscControl;
#endif /* WSC_INCLUDED */

			LinkDown(pAd, FALSE, &pApCliEntry->wdev, NULL);

			/* set the apcli interface be invalid.*/
			pApCliEntry->ApcliInfStat.Valid = FALSE;
			/* clear MlmeAux.Ssid and Bssid.*/
			NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
			pApCliEntry->MlmeAux.SsidLen = 0;
			NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pApCliEntry->MlmeAux.Rssi = 0;
			*pCurrState = CNTL_IDLE;
#ifdef WSC_INCLUDED
			if (pWscControl->bWscTrigger == TRUE)
				WscStop(pAd, TRUE, pWscControl);
#endif /* WSC_INCLUDED */
		}
#endif /* CONFIG_STA_SUPPORT */
		MTWF_PRINT("%s: disabled MAP", __func__);
	}
	return TRUE;
}


#ifdef MAP_VENDOR_SUPPORT

#define IS_40M_BW(__opclass) \
	((__opclass == 116) || (__opclass == 119) || (__opclass == 122) || (__opclass == 126) \
	|| (__opclass == 117) || (__opclass == 120) || (__opclass == 123) || (__opclass == 127))
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];

UCHAR fill_ch_prio_list(IN PRTMP_ADAPTER pAd, UCHAR ch, UCHAR num_of_ch, UCHAR prio)
{
	int i = 0;
	UCHAR j = num_of_ch;
	CHANNEL_CTRL *pChCtrl = NULL;

	if (ch == 0) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"%s Invalid channel 0 found return\n", __func__);
		return 0;
	}

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	while (j) {
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (pChCtrl->ChList[i].Channel == ch) {
				pChCtrl->ChList[i].Priority = prio;
				/* if channel priority set via NL then mark it as valid */
				if (prio != 0 && prio < 0x0f)
					pChCtrl->ch_cfg.chan_prio_valid = TRUE;
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
						"%s priority:%d\n", __func__, pChCtrl->ChList[i].Priority);
			}
		}

		j = j - 1;
	}

	return 0;
}

UCHAR fill_nop_ch_list(IN PRTMP_ADAPTER pAd, UCHAR ch, UCHAR num_of_ch, UCHAR prio, UCHAR bw)
{
	int i = 0;
	UCHAR j = num_of_ch;
	CHANNEL_CTRL *pChCtrl = NULL;
	int ret = 0;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	while (j) {
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (pChCtrl->ChList[i].Channel == ch) {
				ret = RadarChannelCheck(pAd, ch);
				if (!ret) {
					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"%s skip not radar channel:%d\n", __func__, ch);
					continue;
				}

				if (prio == 0) {
					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
						"%s set nop ch:%d, bw:%d\n",  __func__, ch, bw);
					/* mark NOP; */
					 mtk_nl80211_easymesh_set_nop_state_by_bw(pAd, ch, bw, 1);
				} else if (RadarChannelCheck(pAd, ch)) {
					/* clear NOP; */
					MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
						"%s clear nop ch:%d\n",  __func__, ch);
				/* mtk_nl80211_easymesh_set_nop_state_by_bw(pAd, ch, bw, 0); */
				}
				MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
						"%s nop ch:%d\n",  __func__, ch);
			}
		}

		j = j - 1;
	}

	return 0;
}

INT mtk_nl80211_set_map_vendor_nop_info(
	IN RTMP_ADAPTER *pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN struct cont_nop_info *nop)
{
	struct wifi_dev *wdev;
	int num_of_op_class = 0;
	int i = 0, j = 0;
	int ch_num = 0;
	int radio_bw = 0;
#if defined(DOT11_HE_AX) || defined(DOT11_EHT_BE)
	PCH_DESC pChDesc;
#endif /* DOT11_HE_AX */
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
			return -EINVAL;
	}

	radio_bw = wlan_operate_get_eht_bw(wdev);
	num_of_op_class = nop->num_of_op_class;
	if (num_of_op_class >= MAX_MAP_VENDOR_LIMIT) {
		num_of_op_class = 0;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
		"DRIVER [%s] opclass overflow condition hit:%d\n",  __func__, num_of_op_class);
	}

	for (i = 0; i < num_of_op_class; i++) {
		ch_num = nop->channel_nop[i].num_of_ch;
		if (ch_num >= MAX_NUM_OF_CHANNELS) {
			ch_num = 0;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"DRIVER [%s] channel overflow condition hit:%d\n",  __func__, ch_num);
		}
		for (j = 0; j < ch_num; j++) {
			if (WMODE_CAP_5G(wdev->PhyMode)) {
				if ((nop->channel_nop[i].op_class > 81
					&& nop->channel_nop[i].op_class < 84)
					|| (nop->channel_nop[i].op_class >= 115 &&
					nop->channel_nop[i].op_class <= 130)) {
					if (nop->channel_nop[i].op_class <= 127) {
						if (IS_40M_BW(nop->channel_nop[i].op_class)) {
							/* 40M BW */

#if defined(DOT11_HE_AX) || defined(DOT11_EHT_BE)
							pChDesc = Country_Region_ChDesc_5GHZ[pAd->CommonCfg.CountryRegionForABand].pChDesc;
#endif /* DOT11_HE_AX */
							while (pChDesc->FirstChannel) {
							if ((nop->channel_nop[i].ch_list[j] >= pChDesc->FirstChannel) &&
								(nop->channel_nop[i].ch_list[j] < (pChDesc->FirstChannel + (4*pChDesc->NumOfCh)))) {
								fill_nop_ch_list(pAd, nop->channel_nop[i].ch_list[j], ch_num, nop->channel_nop[i].nop_state[j], radio_bw);
							}
							pChDesc++;
							}
						} else {
							/* 20M BW */
							fill_nop_ch_list(pAd, nop->channel_nop[i].ch_list[j], 1, nop->channel_nop[i].nop_state[j], radio_bw);
						}
					} else {
#if defined(DOT11_HE_AX) || defined(DOT11_EHT_BE)
						/* Code for Greater than 80M BW */
						pChDesc = Country_Region_ChDesc_5GHZ[pAd->CommonCfg.CountryRegionForABand].pChDesc;
#endif /* DOT11_HE_AX */
						while (pChDesc->FirstChannel) {
						if ((nop->channel_nop[i].ch_list[j] >= pChDesc->FirstChannel) &&
							(nop->channel_nop[i].ch_list[j] < (pChDesc->FirstChannel + (4*pChDesc->NumOfCh)))) {
							if (nop->channel_nop[i].op_class == 128)
								fill_nop_ch_list(pAd, nop->channel_nop[i].ch_list[j], ch_num, nop->channel_nop[i].nop_state[j], radio_bw);
							else if (nop->channel_nop[i].op_class == 129)
								fill_nop_ch_list(pAd, nop->channel_nop[i].ch_list[j], ch_num, nop->channel_nop[i].nop_state[j], radio_bw);
							else
								MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
									"Invalid 80/160M opclass for %d.\n", nop->channel_nop[i].op_class);
						}
						pChDesc++;
						}
					}
				}
			}
		}
	}

	BuildChannelList(pAd, wdev);
	return 0;
}

INT mtk_nl80211_set_map_vendor_ch_info(
	IN RTMP_ADAPTER *pAd,
	IN struct wiphy *wiphy,
	IN struct wireless_dev *wl_dev,
	IN struct cont_ch_info *ch_info)
{
	struct wifi_dev *wdev;
	int num_of_op_class = 0, i = 0, j = 0;
	int ch_num = 0;
#if defined(DOT11_HE_AX) || defined(DOT11_EHT_BE)
	PCH_DESC pChDesc;
#endif /* DOT11_HE_AX */
	wdev = mtk_cfg80211_vndr_get_wdev_by_wireless_dev(pAd, wl_dev);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"failed - [ERROR]can't find wdev by idx .\n");
			return -EINVAL;
	}

	num_of_op_class = ch_info->num_of_op_class;
	if (num_of_op_class >= MAX_MAP_VENDOR_OP_CLASS) {
		num_of_op_class = 0;
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
		"DRIVER [%s] opclass overflow condition hit:%d\n",  __func__, num_of_op_class);
	}

	for (i = 0; i < num_of_op_class; i++) {
		if (ch_info->channel_pref[i].op_class == 0) {
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"[%s] Invalid Zero opclass\n",  __func__);
			continue;
		}

		ch_num = ch_info->channel_pref[i].num_of_ch;
		if (ch_num >= MAX_MAP_VENDOR_LIMIT) {
			ch_num = 0;
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
			"DRIVER [%s] channel overflow condition hit:%d\n",  __func__, ch_num);
		}

		for (j = 0; j < ch_num; j++) {
			if (IS_40M_BW(ch_info->channel_pref[i].op_class) && (ch_info->is_40M_bw_disable == 1)
				&& ch_info->channel_pref[i].ch_pref[j] != 0) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MAP, DBG_LVL_ERROR,
					"[%s] 40M BW Disable at cont for %d\n",  __func__, ch_info->channel_pref[i].op_class);
					continue;
			}

			if (WMODE_CAP_5G(wdev->PhyMode)) {
				if ((ch_info->channel_pref[i].op_class > 81
					&& ch_info->channel_pref[i].op_class < 84)
					|| (ch_info->channel_pref[i].op_class >= 115 &&
					ch_info->channel_pref[i].op_class <= 130)) {
					if (ch_info->channel_pref[i].op_class <= 127) {
						if (IS_40M_BW(ch_info->channel_pref[i].op_class)) {
							/* 40M BW */
							pChDesc = Country_Region_ChDesc_5GHZ[pAd->CommonCfg.CountryRegionForABand].pChDesc;
							while(pChDesc->FirstChannel) {
							if ((ch_info->channel_pref[i].ch_list[j] >= pChDesc->FirstChannel) &&
								(ch_info->channel_pref[i].ch_list[j] < (pChDesc->FirstChannel + (4*pChDesc->NumOfCh)))) {
								fill_ch_prio_list(pAd, ch_info->channel_pref[i].ch_list[j], ch_num, ch_info->channel_pref[i].ch_pref[j]);
							}
								pChDesc++;
							}
						} else {
							/* 20M BW */
							fill_ch_prio_list(pAd, ch_info->channel_pref[i].ch_list[j], 1, ch_info->channel_pref[i].ch_pref[j]);
						}
					} else {
						/* Code for Greater than 80M BW */
						pChDesc = Country_Region_ChDesc_5GHZ[pAd->CommonCfg.CountryRegionForABand].pChDesc;
						while(pChDesc->FirstChannel) {
						if ((ch_info->channel_pref[i].ch_list[j] >= pChDesc->FirstChannel) &&
							(ch_info->channel_pref[i].ch_list[j] < (pChDesc->FirstChannel + (4*pChDesc->NumOfCh)))) {
							fill_ch_prio_list(pAd, ch_info->channel_pref[i].ch_list[j], ch_num, ch_info->channel_pref[i].ch_pref[j]);
						}
							pChDesc++;
						}
					}
				}
			}
		}
	}

	BuildChannelList(pAd, wdev);
	return 0;
}
#endif /* MAP_VENDOR_SUPPORT */

#endif /* RT_CFG80211_SUPPORT */
#endif /* CONFIG_MAP_SUPPORT */
