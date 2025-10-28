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
/***************************************************************************
 ***************************************************************************

*/

#include "rt_config.h"
#ifdef TXRX_STAT_SUPPORT
#include "hdev/hdev_basic.h"
#endif

static inline BOOLEAN is_tcp_ctrl_frame(IN PNDIS_PACKET	pPacket)
{
	if (RTMP_GET_PACKET_TCP(pPacket)) {
		PUCHAR pSrcBuf;

		pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
		if (*(pSrcBuf + LENGTH_802_3 + IP_HDR_LEN + 13) & (BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7)))
			return TRUE;
	}

	return FALSE;
}

BOOLEAN is_force_link_mgmt_frame(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	PHEADER_802_11 pHeader_802_11;
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len, sub_type;
	UINT8 *ptr;

	pHeader_802_11 = (HEADER_802_11 *)(RTMP_GET_PKT_SRC_VA(pPacket) + tx_hw_hdr_len);

	if (!(pHeader_802_11->FC.Type == FC_TYPE_MGMT))
		return FALSE;

	sub_type = pHeader_802_11->FC.SubType;
	ptr = (UINT8 *)pHeader_802_11;
	if (sub_type == SUBTYPE_ACTION_NO_ACK) {
		ptr += sizeof(HEADER_802_11);
		if ((*ptr == CATEGORY_PROTECTED_EHT)
			&& (*(ptr+1) == EHT_PROT_ACT_LINK_RECOMM))
			return TRUE;
	}

	return FALSE;
}

static BOOLEAN isLLDP(
	IN PUCHAR pMacAddr)
{
	if (((pMacAddr[0] == 0x01)
		&& (pMacAddr[1] == 0x80)
		&& (pMacAddr[2] == 0xc2)
		&& (pMacAddr[3] == 0x00)
		&& (pMacAddr[4] == 0x00)
		&& ((pMacAddr[5] == 0x0e) || (pMacAddr[5] == 0x03) || (pMacAddr[5] == 0x00)))
		)
		return TRUE;

	return FALSE;
}

static inline int ap_tx_packet_list_free(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, INT free_status)
{
	int free_num = 0;
	struct _QUEUE_ENTRY *q_entry = NULL;

	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);

	while (q_entry) {
		RELEASE_NDIS_PACKET(pAd, ((void *)q_entry), free_status);
		free_num++;
		q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	}

	return free_num;
}

static VOID ap_tx_drop_update(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _TX_BLK *txblk)
{
	struct tr_counter *tr_cnt = &ad->tr_ctl.tr_cnt;

#ifdef STATS_COUNT_SUPPORT
	BSS_STRUCT *mbss = txblk->pMbss;

	if (mbss != NULL) {
#ifdef TXRX_STAT_SUPPORT
		struct hdev_ctrl *ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
		INC_COUNTER64(mbss->stat_bss.TxPacketDroppedCount);
		INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxPacketDroppedCount);
#endif
		mbss->TxDropCount++;
	}
#ifdef APCLI_SUPPORT
	else {
		if (txblk->pApCliEntry != NULL)
			txblk->pApCliEntry->StaStatistic.TxDropCount++;
	}
#endif
#endif /* STATS_COUNT_SUPPORT */

	tr_cnt->fill_tx_blk_fail_drop++;

}

static VOID ap_tx_ok_update(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _TX_BLK *txblk)
{
#ifdef STATS_COUNT_SUPPORT
	MAC_TABLE_ENTRY *entry = txblk->pMacEntry;
#ifdef TXRX_STAT_SUPPORT
	UCHAR  pUserPriority, QIdx;
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)ad->hdev_ctrl;
#endif
	BSS_STRUCT * mbss = (txblk->pMbss == NULL && txblk->wdev) ?
		&ad->ApCfg.MBSSID[txblk->wdev->wdev_idx] : txblk->pMbss;

	if (!entry) {
#ifdef TXRX_STAT_SUPPORT
		if (txblk->wdev) {
			if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(mbss->stat_bss.TxMulticastDataPacket);
				INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxMulticastDataPacket);
			} else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(mbss->stat_bss.TxBroadcastDataPacket);
				INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxBroadcastDataPacket);
			}
		}
#endif /* TXRX_STAT_SUPPORT */
		return ;
	}

	if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader)) {
		if (mbss) {
			mbss->mcPktsTx++;
#ifdef MAP_R2
#ifdef MAP_R6
			if (entry && entry->mlo.mlo_en && IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad))
				mbss->mlo_mcBytesTx += txblk->SrcBufLen;
			else
#endif /* MAP_R6 */
				mbss->mcBytesTx += txblk->SrcBufLen;
#endif /* MAP_R2 */
		}
#ifdef TR181_SUPPORT
		INC_COUNTER64(ad->WlanCounters.mcPktsTx);
		ad->WlanCounters.mcBytesTx.QuadPart += txblk->SrcBufLen;
#endif /* TR181_SUPPORT */
	} else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader)) {
		if (mbss) {
			mbss->bcPktsTx++;
#ifdef MAP_R2
#ifdef MAP_R6
			if (entry->mlo.mlo_en && IS_MAP_ENABLE(ad) && IS_MAP_R2_ENABLE(ad))
				mbss->mlo_bcBytesTx += txblk->SrcBufLen;
			else
#endif /* MAP_R6 */
				mbss->bcBytesTx += txblk->SrcBufLen;
#endif /* MAP_R2 */
		}
#ifdef TR181_SUPPORT
		INC_COUNTER64(ad->WlanCounters.bcPktsTx);
		ad->WlanCounters.bcBytesTx.QuadPart += txblk->SrcBufLen;
#endif /* TR181_SUPPORT */
	}

#ifdef WHNAT_SUPPORT
	/*if WHNAT enable, query from CR4 and then update it*/
	if (PD_GET_WHNAT_ENABLE(ad->physical_dev) && (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD)))
		return;
#endif /*WHNAT_SUPPORT*/

	/* calculate Tx count and ByteCount per BSS */
	{

#ifdef TXRX_STAT_SUPPORT
		if (mbss) {
			if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(mbss->stat_bss.TxMulticastDataPacket);
				INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxMulticastDataPacket);
			} else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader)) {
				INC_COUNTER64(mbss->stat_bss.TxBroadcastDataPacket);
				INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxBroadcastDataPacket);
			} else {
				INC_COUNTER64(mbss->stat_bss.TxUnicastDataPacket);
				INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxUnicastDataPacket);
			}
		}
#endif

		if (entry->Sst == SST_ASSOC)
			ad->TxTotalByteCnt += txblk->SrcBufLen;
	}
#ifdef TXRX_STAT_SUPPORT
	RTMPGetUserPriority(ad, txblk->pPacket, wdev, &pUserPriority, &QIdx);
	if ((entry && (IS_ENTRY_CLIENT(entry) || IS_ENTRY_PEER_AP(entry))) && (entry->Sst == SST_ASSOC) &&
		QIdx < ARRAY_SIZE(entry->TxDataPacketCountPerAC)) {
		/*increase unicast packet count per station*/
		INC_COUNTER64(entry->TxDataPacketCount);
		INC_COUNTER64(entry->TxDataPacketCountPerAC[QIdx]);
		if (entry->pMbss) {
			mbss = MBSS_GET(entry->pMbss);

			INC_COUNTER64(mbss->stat_bss.TxDataPacketCount);
			INC_COUNTER64(mbss->stat_bss.TxDataPacketCountPerAC[QIdx]);
			mbss->stat_bss.TxDataPacketByte.QuadPart += txblk->SrcBufLen;
			mbss->stat_bss.TxDataPayloadByte.QuadPart += (txblk->SrcBufLen - 14);
			mbss->stat_bss.LastPktStaWcid = txblk->Wcid;
		}
		INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxDataPacketCount);
		INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxDataPacketCountPerAC[QIdx]);
		entry->TxDataPacketByte.QuadPart += txblk->SrcBufLen;
		ctrl->rdev.pRadioCtrl->TxDataPacketByte.QuadPart += txblk->SrcBufLen;
	} else
		if (entry && (IS_ENTRY_MCAST(entry))) {
			/*increase mcast packet count per mbss*/
		}
#endif

#ifdef WDS_SUPPORT

	if (IS_ENTRY_WDS(entry) && (entry->func_tb_idx < MAX_WDS_ENTRY)) {
		INC_COUNTER64(ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedByteCount += txblk->SrcBufLen;
	}

#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */
}


INT ap_fp_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UCHAR *pkt_va;
	UINT pkt_len;
	MAC_TABLE_ENTRY *entry = NULL;
	UINT16 wcid = WCID_INVALID;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *pTmpEntry = NULL;
	UCHAR frag_nums;
#ifdef MAP_TS_TRAFFIC_SUPPORT
	MAC_TABLE_ENTRY *peer_entry = NULL;
#endif
#ifdef HIGH_PRIO_QUEUE_SUPPORT
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);
#endif
	pkt_va = RTMP_GET_PKT_SRC_VA(pkt);
	pkt_len = RTMP_GET_PKT_LEN(pkt);

	if ((!pkt_va) || (pkt_len <= 14))
		return NDIS_STATUS_FAILURE;

	if (MAC_ADDR_IS_GROUP(pkt_va)) {
#ifdef CONFIG_VLAN_GTK_SUPPORT
		struct sk_buff *skb = RTPKT_TO_OSPKT(pkt);
		INT16 vlan_id;
		struct vlan_gtk_info *vg_info;
#endif
		if (wdev->PortSecured != WPA_802_1X_PORT_SECURED)
			return NDIS_STATUS_FAILURE;
		wcid = wdev->tr_tb_idx;
#ifdef CONFIG_VLAN_GTK_SUPPORT
		vlan_id = CFG80211_IsVlanPkt(pkt);
		vg_info = CFG80211_GetVlanInfoByVlanid(wdev, vlan_id);
		if (vlan_id > 0 && vg_info) {
		/* substitute wcid with vlan bmc_wcid */
			wcid = vg_info->vlan_tr_tb_idx;
			/* remove vlan tag and set procotol */
			memmove(skb->data + VLAN_HLEN, skb->data, MAC_ADDR_LEN*2);
			RtmpOsSkbPullRcsum(skb, LENGTH_802_1Q);
			RtmpOsSkbResetMacHeader(skb);
			RtmpOsSkbResetNetworkHeader(skb);
			RtmpOsSkbResetTransportHeader(skb);
			RtmpOsSkbResetMacLen(skb);
			skb->protocol = skb->data[12] + (skb->data[13] << 8);
			RTMP_SET_PACKET_VLANGTK(pkt, vlan_id);
			RTMP_SET_PACKET_VLAN(pkt, FALSE);
			RTMP_SET_PACKET_PROTOCOL(pkt, ntohs(skb->protocol));
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_DEBUG,
				"%s() tx bmc pkt, proto=0x%x, bmc_wcid=%d vlan_id=%d\n",
				__func__, ntohs(skb->protocol), wcid, vlan_id);
		}
#endif
	} else {
		entry = MacTableLookupForTx(pAd, pkt_va, wdev);

		if (entry && (entry->Sst == SST_ASSOC)) {
#ifdef WH_EVENT_NOTIFIER
			if (IS_ENTRY_CLIENT(entry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(entry)
#endif /* A4_CONN */
			)
				entry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */

#if defined(RT_CFG80211_SUPPORT) || defined(DYNAMIC_VLAN_SUPPORT)
			{
				UCHAR *pSrcBuf;
				UINT16 TypeLen;

				pSrcBuf = GET_OS_PKT_DATAPTR(pkt);
				TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];

#ifdef DYNAMIC_VLAN_SUPPORT
				pSrcBuf += LENGTH_802_3;
				if (TypeLen == ETH_TYPE_VLAN && entry->vlan_id) {
					USHORT vlan_id = *(USHORT *)pSrcBuf;

					vlan_id = cpu2be16(vlan_id);
					vlan_id = vlan_id & 0x0FFF; /* 12 bit */
					if (vlan_id != entry->vlan_id)
						return NDIS_STATUS_FAILURE;
					pSrcBuf -= LENGTH_802_3;
					memmove(pSrcBuf + 4, pSrcBuf, 12);
					skb_pull(pkt, 4);
				}
#endif
#ifdef RT_CFG80211_SUPPORT

				if (TypeLen == ETH_TYPE_EAPOL) {
					struct _STA_TR_ENTRY *tr_entry = NULL;

					if (IS_WCID_VALID(pAd, entry->wcid))
						tr_entry = tr_entry_get(pAd, entry->wcid);

					if (!tr_entry)
						MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_ERROR,
							"tr_entry is NULL!\n");

					if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
						RTMP_SET_PACKET_CLEAR_EAP_FRAME(pkt,0);
					else
						RTMP_SET_PACKET_CLEAR_EAP_FRAME(pkt,1);

					if (pAd->FragFrame.wcid == entry->wcid) {
						MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_INFO,
							"\nClear Wcid = %d FragBuffer !!!!!\n", entry->wcid);
						RESET_FRAGFRAME(pAd->FragFrame);
					}
				}
#endif
			}
#endif

			wcid = entry->wcid;
		}

#ifdef A4_CONN
		if ((entry == NULL) ||
			(entry && (entry->roaming_entry == TRUE))
#ifdef AIR_MONITOR
			|| (entry && IS_ENTRY_MONITOR(entry))
#endif
		){
			UINT16 main_wcid;

			/* If we check an ethernet source move to this device, we should remove it. */
			if (!RTMP_GET_PACKET_A4_FWDDATA(pkt))
				a4_proxy_delete(pAd, wdev->func_idx, (pkt_va + MAC_ADDR_LEN));
			if (a4_proxy_lookup(pAd, wdev->func_idx, pkt_va, FALSE, FALSE, &main_wcid))
				wcid = main_wcid;
		}
#endif /* A4_CONN */
#ifdef CLIENT_WDS
		if (entry == NULL) {
			PUCHAR pEntryAddr = CliWds_ProxyLookup(pAd, pkt_va);

			if (pEntryAddr != NULL) {
				entry = MacTableLookup(pAd, pEntryAddr);
				if (entry && (entry->Sst == SST_ASSOC))
					wcid = (UCHAR)entry->wcid;
			}
		}
#endif /* CLIENT_WDS */

	}
	if (!IS_WCID_VALID(pAd, wcid))
		return NDIS_STATUS_FAILURE;

	tr_entry = tr_entry_get(pAd, wcid);
	if (!IS_VALID_ENTRY(tr_entry))
		return NDIS_STATUS_FAILURE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
#ifdef CONFIG_AP_SUPPORT
#ifdef CFG80211_SUPPORT

	/* CFG_TODO: POS NO GOOD */
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		RTMP_SET_PACKET_OPMODE(pkt, OPMODE_AP);

#endif /* CFG80211_SUPPORT */
#endif
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);
	/*  ethertype check is not offload to mcu for fragment frame*/
	if ((frag_nums > 1)
		|| RTMP_TEST_FLAG(pAd->physical_dev, PH_DEV_CHECK_ETH_TYPE)
#ifdef PER_PKT_CTRL_FOR_CTMR
		|| pAd->PerPktCtrlEnable
#endif
#ifdef IGMP_SNOOPING_NON_OFFLOAD
		|| (wdev->IgmpSnoopEnable && IS_MULTICAST_MAC_ADDR(GET_OS_PKT_DATAPTR(pkt)))
#endif
#ifdef SW_CONNECT_SUPPORT
		|| (IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry))
#endif /* SW_CONNECT_SUPPORT */
#ifdef HIGH_PRIO_QUEUE_SUPPORT
		|| (qm_parm->max_highpri_que_num > 0)
#endif
	) {
		if (!RTMPCheckEtherType(pAd, pkt, tr_entry, wdev))
			return NDIS_STATUS_FAILURE;
	}

#ifndef RT_CFG80211_SUPPORT
	if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
		if (!((IS_AKM_WPA_CAPABILITY_Entry(wdev) || (entry && entry->bWscCapable)
#ifdef DOT1X_SUPPORT
			   || (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */
#ifdef HOSTAPD_11R_SUPPORT
			   || (IS_AKM_PSK_Entry(wdev))
#endif
			  ) && ((RTMP_GET_PACKET_EAPOL(pkt) || RTMP_GET_PACKET_WAI(pkt))))
		   ) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_INFO,
				"Drop PKT before 4-Way Handshake done! wcid = %d.\n", wcid);
				return NDIS_STATUS_FAILURE;
		}
	}
#endif

	/* if sta rec isn't valid, don't allow pkt tx */
	pTmpEntry = entry_get(pAd, wcid);
	if (!pTmpEntry) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_INFO,
			"Drop PKT due to no StaRec! wcid = %d.\n",  wcid);
		return NDIS_STATUS_FAILURE;
	} else if (!pTmpEntry->sta_rec_valid) {
		struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

		if (tr_entry->delay_queue.Number < SQ_ENQ_DELAYQ_MAX) {
			qm_ops->enq_delayq_pkt(pAd, wdev, tr_entry, pkt);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
				"Requeue PKT until StaRec ready! wcid = %d.\n",  wcid);
			return NDIS_STATUS_PKT_REQUEUE;
		}

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			"Drop PKT due to delay queue full! wcid = %d.\n", wcid);
		return NDIS_STATUS_FAILURE;
	}

#ifdef MAP_TS_TRAFFIC_SUPPORT
	if (pAd->bTSEnable) {
		peer_entry = entry_get(pAd, wcid);

		if (!map_ts_tx_process(pAd, wdev, pkt, peer_entry))
			return NDIS_STATUS_FAILURE;
	}
#endif

#ifdef SW_CONNECT_SUPPORT
	tr_entry->tx_fp_allow_cnt++;
#endif /* SW_CONNECT_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

INT ap_tx_pkt_allowed(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt)
{
	UCHAR *pkt_va;
	UINT pkt_len;
	MAC_TABLE_ENTRY *entry = NULL;
	UINT16 wcid = WCID_INVALID;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *pTmpEntry = NULL;
	UCHAR frag_nums;
#ifdef MAP_TS_TRAFFIC_SUPPORT
	MAC_TABLE_ENTRY *peer_entry = NULL;
#endif

	pkt_va = RTMP_GET_PKT_SRC_VA(pkt);
	pkt_len = RTMP_GET_PKT_LEN(pkt);

	if ((!pkt_va) || (pkt_len <= 14))
		return NDIS_STATUS_FAILURE;

	if (MAC_ADDR_IS_GROUP(pkt_va)) {
		if (wdev->PortSecured != WPA_802_1X_PORT_SECURED)
			return NDIS_STATUS_FAILURE;

		wcid = wdev->tr_tb_idx;
	} else {
		entry = MacTableLookupForTx(pAd, pkt_va, wdev);

		if (entry && (entry->Sst == SST_ASSOC)) {
#ifdef WH_EVENT_NOTIFIER
			if (IS_ENTRY_CLIENT(entry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(entry)
#endif /* A4_CONN */
			)
				entry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */

#ifdef RADIUS_MAC_AUTH_SUPPORT
			if (wdev->radius_mac_auth_enable) {
				if (!entry->bAllowTraffic)
					return NDIS_STATUS_FAILURE;
			}
#endif

#ifdef DYNAMIC_VLAN_SUPPORT
		{
			UCHAR *pSrcBuf;
			UINT16 TypeLen;

			if (entry->vlan_id) {
				pSrcBuf = GET_OS_PKT_DATAPTR (pkt);
				TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
				pSrcBuf += LENGTH_802_3;
				if (TypeLen == ETH_TYPE_VLAN) {
					USHORT vlan_id = *(USHORT *)pSrcBuf;

					vlan_id = cpu2be16(vlan_id);
					vlan_id = vlan_id & 0x0FFF; /* 12 bit */
					if (vlan_id != entry->vlan_id)
						return NDIS_STATUS_FAILURE;
					pSrcBuf -= LENGTH_802_3;
					memmove(pSrcBuf + 4, pSrcBuf, 12);
					skb_pull(pkt, 4);
				}
			}
		}
#endif

			wcid = entry->wcid;
		}
#ifdef A4_CONN
		if ((entry == NULL)
#ifdef AIR_MONITOR
			|| (entry && IS_ENTRY_MONITOR(entry))
#endif
		) {
			UINT16 main_wcid;

			/* If we check an ethernet source move to this device, we should remove it. */
			if (!RTMP_GET_PACKET_A4_FWDDATA(pkt))
				a4_proxy_delete(pAd, wdev->func_idx, (pkt_va + MAC_ADDR_LEN));
			if (a4_proxy_lookup(pAd, wdev->func_idx, pkt_va, FALSE, FALSE, &main_wcid))
				wcid = main_wcid;
		}
#endif /* A4_CONN */
	}

	if (!IS_WCID_VALID(pAd, wcid))
		return NDIS_STATUS_FAILURE;

	tr_entry = tr_entry_get(pAd, wcid);

	if (!IS_VALID_ENTRY(tr_entry))
		return NDIS_STATUS_FAILURE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
#ifdef CONFIG_HOTSPOT

	/* Drop broadcast/multicast packet if disable dgaf */
	if (IS_ENTRY_CLIENT(tr_entry)) {
		BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

		if ((wcid == wdev->bss_info_argument.bmc_wlan_idx) &&
			(pMbss->HotSpotCtrl.HotSpotEnable || pMbss->HotSpotCtrl.bASANEnable) &&
			pMbss->HotSpotCtrl.DGAFDisable) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				"Drop broadcast/multicast packet when dgaf disable\n");
			return NDIS_STATUS_FAILURE;
		}
	}

#endif
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

	if (!RTMPCheckEtherType(pAd, pkt, tr_entry, wdev))
		return NDIS_STATUS_FAILURE;

	if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
		if (!((IS_AKM_WPA_CAPABILITY_Entry(wdev)
#ifdef DOT1X_SUPPORT
			   || (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */

			  ) && ((RTMP_GET_PACKET_EAPOL(pkt) ||
					 RTMP_GET_PACKET_WAI(pkt))))
		) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_SEC_TX, DBG_LVL_INFO,
				"Drop PKT before 4-Way Handshake done! wcid = %d.\n", wcid);
			return NDIS_STATUS_FAILURE;
		}
	}

	/* if sta rec isn't valid, don't allow pkt tx */
	pTmpEntry = entry_get(pAd, wcid);
	if (!pTmpEntry) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			"Drop PKT due to no StaRec! wcid = %d.\n", wcid);
		return NDIS_STATUS_FAILURE;
	} else if (!pTmpEntry->sta_rec_valid) {
		struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

		if (tr_entry->delay_queue.Number < SQ_ENQ_DELAYQ_MAX) {
			qm_ops->enq_delayq_pkt(pAd, wdev, tr_entry, pkt);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
				"Requeue PKT until StaRec ready! wcid = %d.\n",  wcid);
			return NDIS_STATUS_PKT_REQUEUE;
		}

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO,
			"Drop PKT due to delay queue full! wcid = %d.\n", wcid);
		return NDIS_STATUS_FAILURE;
	}

#ifdef CFG80211_SUPPORT

	/* CFG_TODO: POS NO GOOD */
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		RTMP_SET_PACKET_OPMODE(pkt, OPMODE_AP);

#endif /* CFG80211_SUPPORT */

#ifdef MAP_TS_TRAFFIC_SUPPORT
	if (pAd->bTSEnable) {
		peer_entry = entry_get(pAd, tr_entry->wcid);

		if (!map_ts_tx_process(pAd, wdev, pkt, peer_entry))
			return NDIS_STATUS_FAILURE;
	}
#endif

	return NDIS_STATUS_SUCCESS;
}

UINT16 ap_mlme_search_wcid(RTMP_ADAPTER *pAd, UCHAR *addr1, UCHAR *addr2, PNDIS_PACKET pkt, struct wifi_dev *wdev)
{
	MAC_TABLE_ENTRY *mac_entry;

	if (wdev->tr_tb_idx == WCID_INVALID) {
		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO, "wdev not initailized\n");
		return 0;
	}


	if ((wdev->wdev_type == WDEV_TYPE_AP) && MAC_ADDR_IS_GROUP(addr1)) {
		mac_entry = entry_get(pAd, wdev->tr_tb_idx);
	} else {
		mac_entry = MacTableLookup2(pAd, addr1, wdev);
	}

#ifdef MAC_REPEATER_SUPPORT
	if ((mac_entry != NULL) && (IS_ENTRY_PEER_AP(mac_entry) || IS_ENTRY_REPEATER(mac_entry))) {

		REPEATER_CLIENT_ENTRY *rept_entry = lookup_rept_entry(pAd, addr2);

		if (rept_entry) { /*repeater case*/
			if ((rept_entry->CliEnable == TRUE) && (rept_entry->CliValid == TRUE))
				mac_entry = rept_entry->pMacEntry;
		} else { /*apcli case*/
			UINT16 apcli_wcid = 0;

			if (mac_entry->wdev && (mac_entry->wdev->func_idx < pAd->ApCfg.ApCliNum))
				apcli_wcid = pAd->StaCfg[mac_entry->wdev->func_idx].MacTabWCID;
			else   /* use default apcli0 */
				apcli_wcid = pAd->StaCfg[0].MacTabWCID;

			if ((apcli_wcid >= 0) && IS_WCID_VALID(pAd, apcli_wcid))
				mac_entry = entry_get(pAd, apcli_wcid);
		}
	}

#endif
	if (mac_entry)
		return mac_entry->wcid;
	else
		return 0;
}

INT ap_send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue)
{
	HEADER_802_11 *pHeader_802_11;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR *pSrcBufVA;
	INT ret;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	struct _MAC_TABLE_ENTRY *pEntry;
#ifdef ZERO_PKT_LOSS_SUPPORT
	struct DOT11_H *pDot11h = (struct DOT11_H *)&pAd->Dot11_H;
#endif

	RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
	RTMP_SET_PACKET_MGMT_PKT(pkt, 1);
	pSrcBufVA = RTMP_GET_PKT_SRC_VA(pkt);

	if (pSrcBufVA == NULL) {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pHeader_802_11 = (HEADER_802_11 *)(pSrcBufVA + tx_hw_hdr_len);
	RTMP_SET_PACKET_WCID(pkt, ap_mlme_search_wcid(pAd, pHeader_802_11->Addr1, pHeader_802_11->Addr2, pkt, wdev));

	if (in_altx_filter_list(pHeader_802_11)
		&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT)
		) {
		if (!(RTMP_GET_PACKET_TXTYPE(pkt) == TX_ATE_FRAME))
			RTMP_SET_PACKET_TYPE(pkt, TX_ALTX);
	}

#ifdef ZERO_PKT_LOSS_SUPPORT
	if (pAd->Zero_Loss_Enable &&
		pDot11h->ChnlSwitchState == ASIC_CHANNEL_SWITCH_COMMAND_ISSUED &&
		(pHeader_802_11->FC.Type == FC_TYPE_DATA &&
		pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL)) {
		RTMP_SET_PACKET_TYPE(pkt, TX_ALTX);
	}
#endif
#ifdef FORCE_NULL_TX
	if (((q_idx & NULL_USE_ALTX_FLAG) == NULL_USE_ALTX_FLAG)
	&& (pHeader_802_11->FC.Type == FC_TYPE_DATA)
	&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL)
	|| (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	) {
		MTWF_DBG(pAd, DBG_CAT_AP, DBG_CAT_PS, DBG_LVL_ERROR,
			"%s: FORCE NULL Frame type=%d, sub_type=%d\n",
			__func__,
			pHeader_802_11->FC.Type,
			pHeader_802_11->FC.SubType);
		RTMP_SET_PACKET_TYPE(pkt, TX_ALTX);
		q_idx &= (~NULL_USE_ALTX_FLAG);
	}
#endif

	if  (!is_data_queue) {
	} else {
#ifdef UAPSD_SUPPORT
		{
			UAPSD_MR_QOS_NULL_HANDLE(pAd, pHeader_802_11, pkt);
		}

#endif /* UAPSD_SUPPORT */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pkt, 1);
	}

	pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);

	if ((!pEntry || !pEntry->sta_rec_valid) && (RTMP_GET_PACKET_TYPE(pkt) != TX_ALTX)
		&& !valid_broadcast_mgmt_in_acq(pHeader_802_11)) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"pkt from non-connected(or not ready) sta, type=%d, sub_type=%d to ACQ, drop\n",
			pHeader_802_11->FC.Type,
			pHeader_802_11->FC.SubType);
		tr_cnt->tx_invalid_mgmt_drop++;
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (PD_GET_ASSOC_RECORD_EN(pAd->physical_dev)
		&& pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		UINT16 payload_size =  GET_OS_PKT_LEN(pkt) - tx_hw_hdr_len;

		if (payload_size < MAX_MGMT_PKT_LEN) {
			if (pHeader_802_11->FC.SubType == SUBTYPE_ASSOC_REQ) {
				pAd->last_assoc_req_len = payload_size;
				NdisZeroMemory(pAd->last_assoc_req, MAX_MGMT_PKT_LEN);
				memcpy(pAd->last_assoc_req,
					   pHeader_802_11,
					   pAd->last_assoc_req_len);
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						"received assoc req FC.SubType=0x%x, size=0x%x\n",
						pHeader_802_11->FC.SubType,
						pAd->last_assoc_req_len);
			} else if (pHeader_802_11->FC.SubType == SUBTYPE_ASSOC_RSP) {
				pAd->last_assoc_resp_len = payload_size;
				NdisZeroMemory(pAd->last_assoc_resp, MAX_MGMT_PKT_LEN);
				memcpy(pAd->last_assoc_resp,
					   pHeader_802_11,
					   pAd->last_assoc_resp_len);
				MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO,
						"received assoc resp FC.SubType=0x%x, size=0x%x\n",
						pHeader_802_11->FC.SubType,
						pAd->last_assoc_resp_len);
			}
		}
	}

	ret = qm_ops->enq_mgmtq_pkt(pAd, wdev, pkt);

	return ret;
}

static INT ap_ps_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STA_TR_ENTRY *tr_entry,
					PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct qm_ctl *qm_ctl = PD_GET_QM_CTL(pAd->physical_dev);
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	UINT16 occupy_cnt = (tr_entry->token_cnt + tr_entry->enqCount);

	if (occupy_cnt >= SQ_ENQ_PS_MAX) {
		if ((tr_entry->ps_queue.Number < SQ_ENQ_PSQ_MAX) &&
		    (qm_ctl->total_psq_cnt < SQ_ENQ_PSQ_TOTAL_MAX)) {
			if (qm_ops->enq_psq_pkt) {
				qm_ops->enq_psq_pkt(pAd, wdev, tr_entry, pkt);
				qm_ctl->total_psq_cnt++;
			} else {
				MTWF_DBG(pAd, DBG_CAT_PS, CATPS_CFG, DBG_LVL_ERROR,
					"no enq_psq_pkt handler\n");
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
			}
		} else {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
		}
	} else {
		if (tr_entry->ps_queue.Number != 0) {
			NDIS_PACKET *ps_pkt = NULL;
			UINT16 quota = (SQ_ENQ_PS_MAX - occupy_cnt);

			do {
				ps_pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

				if (ps_pkt) {
					quota--;
					qm_ctl->total_psq_cnt--;
					qm_ops->enq_dataq_pkt(pAd, wdev, ps_pkt, q_idx);
				}
			} while (ps_pkt && (quota > 0));

			if (quota > 0) {
				qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
			} else {
				qm_ops->enq_psq_pkt(pAd, wdev, tr_entry, pkt);
				qm_ctl->total_psq_cnt++;
			}

		} else {
			qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

INT ap_delayq_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STA_TR_ENTRY *tr_entry)
{
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	NDIS_PACKET *pkt = NULL;

	if (qm_ops->deq_delayq_pkt == NULL)
		return NDIS_STATUS_SUCCESS;

	pkt = qm_ops->deq_delayq_pkt(pAd, tr_entry);
	while (pkt) {
		send_data_pkt_exec(pAd, wdev, pkt);
		pkt = qm_ops->deq_delayq_pkt(pAd, tr_entry);
	}

	return NDIS_STATUS_SUCCESS;
}

INT ap_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UINT16 wcid = RTMP_GET_PACKET_WCID(pkt);
#ifdef IGMP_SNOOP_SUPPORT
	INT InIgmpGroup = IGMP_NONE;
	MULTICAST_FILTER_TABLE_ENTRY *pGroupEntry = NULL;
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#endif /* IGMP_SNOOP_SUPPORT */
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR user_prio = 0;
	UCHAR q_idx;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	UCHAR *pkt_va;

	pkt_va = RTMP_GET_PKT_SRC_VA(pkt);
	tr_entry = tr_entry_get(pAd, wcid);
	user_prio = RTMP_GET_PACKET_UP(pkt);
	q_idx = RTMP_GET_PACKET_QUEIDX(pkt);

	if (tr_entry->EntryType != ENTRY_CAT_MCAST)
		wdev_ops->detect_wmm_traffic(pAd, wdev, user_prio, FLG_IS_OUTPUT);
	else {
#ifdef IGMP_SNOOP_SUPPORT
		if (wdev->IgmpSnoopEnable) {
			if (IgmpPktInfoQuery(pAd, pkt_va, pkt, wdev,
						&InIgmpGroup, &pGroupEntry) != NDIS_STATUS_SUCCESS) {
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
				return NDIS_STATUS_FAILURE;
			}

			/* if it's a mcast packet in igmp gourp. ucast clone it for all members in the gourp. */
			if ((InIgmpGroup == IGMP_IN_GROUP)
				 && pGroupEntry
				 && (IgmpMemberCnt(&pGroupEntry->MemberList) > 0)) {
				NDIS_STATUS PktCloneResult = IgmpPktClone(pAd, wdev, pkt, InIgmpGroup, pGroupEntry,
								q_idx, user_prio, GET_OS_PKT_NETDEV(pkt));
#ifdef IGMP_TVM_SUPPORT
				if (PktCloneResult != NDIS_STATUS_MORE_PROCESSING_REQUIRED)
#endif /* IGMP_TVM_SUPPORT */
				{
					if (PktCloneResult != NDIS_STATUS_SUCCESS)
						tr_cnt->igmp_clone_fail_drop++;
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
					return PktCloneResult;
				}
			}

			RTMP_SET_PACKET_TXTYPE(pkt, TX_BMC_FRAME);
		} else
#endif /* IGMP_SNOOP_SUPPORT */
			RTMP_SET_PACKET_TXTYPE(pkt, TX_BMC_FRAME);
	}

	RTMP_SET_PACKET_UP(pkt, user_prio);

	OS_SEM_LOCK(&tr_entry->ps_sync_lock);

	if (tr_entry->ps_state == PWR_ACTIVE)
		qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
	else
		ap_ps_handle(pAd, wdev, tr_entry, pkt, q_idx);

	OS_SEM_UNLOCK(&tr_entry->ps_sync_lock);

	ba_ori_session_start(pAd, tr_entry->wcid, user_prio);
	return NDIS_STATUS_SUCCESS;
}

/*
	--------------------------------------------------------
	FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
		Find the WPA key, either Group or Pairwise Key
		LEAP + TKIP also use WPA key.
	--------------------------------------------------------
	Decide WEP bit and cipher suite to be used.
	Same cipher suite should be used for whole fragment burst
	In Cisco CCX 2.0 Leap Authentication
		WepStatus is Ndis802_11WEPEnabled but the key will use PairwiseKey
		Instead of the SharedKey, SharedKey Length may be Zero.
*/
VOID ap_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	SET_CIPHER_NONE(pTxBlk->CipherAlg);
	pTxBlk->KeyIdx = 0;

	/* TODO:Eddy, Confirm MESH/Apcli.WAPI */
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)
		) {
		pTxBlk->pKey =  NULL;
	} else if (pTxBlk->TxFrameType == TX_BMC_FRAME) {
		if (!IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) {
			/* Change pTxBlk->CipherAlg non zero for TxD ref */
			pTxBlk->CipherAlg = wdev->SecConfig.GroupCipher;
			pTxBlk->KeyIdx =  wdev->SecConfig.GroupKeyId;
			if (IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) {
				pTxBlk->pKey = (PCIPHER_KEY)&(wdev->SecConfig.WepKey[pTxBlk->KeyIdx]);
			} else {
				/* TBD : S/W no support bcast now */
				pTxBlk->pKey = NULL;
				/* pTxBlk->pKey = wdev->SecConfig.GTK; */
			}
		}
	} else if (pMacEntry) {
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT)) {
			/* S/W case only support partial encryp type */
			TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
			pTxBlk->KeyIdx =  pMacEntry->SecConfig.PairwiseKeyId;
			if (IS_CIPHER_WEP40(pMacEntry->SecConfig.PairwiseCipher)) {
				pTxBlk->pKey = (PCIPHER_KEY)&(pMacEntry->SecConfig.WepKey[pTxBlk->KeyIdx]);
				SET_CIPHER_WEP40(pTxBlk->CipherAlg);
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
			} else if (IS_CIPHER_WEP104(pMacEntry->SecConfig.PairwiseCipher)) {
				pTxBlk->pKey = (PCIPHER_KEY)&(pMacEntry->SecConfig.WepKey[pTxBlk->KeyIdx]);
				SET_CIPHER_WEP104(pTxBlk->CipherAlg);
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
			} else {
				/* CIPHER_AES , CIPHER_TKIP and others TBD */
				pTxBlk->pKey = &pMacEntry->SecConfig.SwPairwiseKey;
				pTxBlk->CipherAlg = pMacEntry->SecConfig.SwPairwiseKey.CipherAlg;

				if (IS_CIPHER_CCMP128(pTxBlk->CipherAlg))
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
				else if (IS_CIPHER_TKIP(pTxBlk->CipherAlg))
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
			}
		} else
#endif /* defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT) */
		{
			pTxBlk->pKey = NULL;
			/* Non S/W encrypt cases */
			if (!IS_CIPHER_NONE(pMacEntry->SecConfig.PairwiseCipher)) {
				/* Change pTxBlk->CipherAlg non zero for TxD ref */
				pTxBlk->CipherAlg = pMacEntry->SecConfig.PairwiseCipher;
				pTxBlk->KeyIdx =  pMacEntry->SecConfig.PairwiseKeyId;
				if (IS_CIPHER_WEP(pMacEntry->SecConfig.PairwiseCipher))
					pTxBlk->pKey = (PCIPHER_KEY)&(pMacEntry->SecConfig.WepKey[pTxBlk->KeyIdx]);
			}
		}
	}

}

static inline VOID ap_build_cache_802_11_header(
	IN RTMP_ADAPTER *pAd,
	IN struct _TX_BLK *pTxBlk,
	IN UCHAR *pHeader)
{
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *pHeader80211;
	MAC_TABLE_ENTRY *pMacEntry;
	struct seq_ctrl_t *seq_ctrl = NULL;

	pHeader80211 = (PHEADER_802_11)pHeader;
	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;
	seq_ctrl = &tr_entry->seq_ctrl;
	/*
		Update the cached 802.11 HEADER
	*/
	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	/* More Bit */
	pHeader80211->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
	/* Sequence */
	if (pTxBlk->UserPriority < NUM_OF_TID) {
		pHeader80211->Sequence = seq_ctrl->TxSeq[pTxBlk->UserPriority];
		seq_ctrl->TxSeq[pTxBlk->UserPriority] = (seq_ctrl->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
	}
	/* SA */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
		if (FALSE
#ifdef WDS_SUPPORT
			|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
			|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
		   ) {
			/* The addr3 of WDS packet is Destination Mac address and Addr4 is the Source Mac address. */
			COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		} else
#endif /* WDS_SUPPORT || CLIENT_WDS */
#ifdef A4_CONN
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bA4Frame)) {
		pHeader80211->FC.ToDs = 1;
		pHeader80211->FC.FrDs = 1;
		if (pTxBlk->pMacEntry) {
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(pHeader80211->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);
			} else
#endif /* APCLI_SUPPORT */
			if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(pHeader80211->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pAd->CurrentAddress); /* from AP1 */
			}
			COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		} else
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"pTxBlk->pMacEntry == NULL!\n");
	} else
#endif /* A4_CONN */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) {
				/* The addr3 of Ap-client packet is Destination Mac address. */
				COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
			} else
#endif /* APCLI_SUPPORT */
			{	/* The addr3 of normal packet send from DS is Src Mac address. */
				COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			}
}

static inline VOID ap_build_802_11_header(RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	struct wifi_dev *wdev = pTxBlk->wdev;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;
	struct seq_ctrl_t *seq_ctrl = NULL;
	/*
		MAKE A COMMON 802.11 HEADER
	*/
	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	pTxBlk->HeaderBuf = (u8 *)pTxBlk->HeaderBuffer;
	pTxBlk->wifi_hdr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];
	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	NdisZeroMemory(wifi_hdr, sizeof(HEADER_802_11));
	wifi_hdr->FC.FrDs = 1;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

	/* TODO: shiang-usw, for BCAST/MCAST, original it's sequence assigned by "pAd->Sequence", how about now? */
	if (tr_entry) {
		seq_ctrl = &tr_entry->seq_ctrl;
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM) && (pTxBlk->UserPriority < NUM_OF_TID)) {
			wifi_hdr->Sequence = seq_ctrl->TxSeq[pTxBlk->UserPriority];
			seq_ctrl->TxSeq[pTxBlk->UserPriority] = (seq_ctrl->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
		} else {
			wifi_hdr->Sequence = seq_ctrl->NonQosDataSeq;
			seq_ctrl->NonQosDataSeq = (seq_ctrl->NonQosDataSeq + 1) & MAXSEQ;
		}
	} else {
		wifi_hdr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ; /* next sequence */
	}

	wifi_hdr->Frag = 0;
	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
#ifdef A4_CONN
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bA4Frame)) {
		wifi_hdr->FC.ToDs = 1;
		wifi_hdr->FC.FrDs = 1;
		if (pTxBlk->pMacEntry) {
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);
			} else
#endif /* APCLI_SUPPORT */
			if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry)) {
				COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress); /* from AP1 */
			}
			COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(wifi_hdr->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		} else
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"pTxBlk->pMacEntry == NULL!\n");
	} else
#endif /* A4_CONN*/

#ifdef APCLI_SUPPORT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket)) {
			wifi_hdr->FC.ToDs = 1;
			wifi_hdr->FC.FrDs = 0;
			COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid));	/* to AP2 */
#ifdef MAC_REPEATER_SUPPORT

			if (pTxBlk->pMacEntry && IS_REPT_LINK_UP(pTxBlk->pMacEntry->pReptCli))
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pMacEntry->pReptCli->CurrentAddress);
			else
#endif /* MAC_REPEATER_SUPPORT */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);		/* from AP1 */

			COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
		} else
#endif /* APCLI_SUPPORT */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
			if (FALSE
#ifdef WDS_SUPPORT
				|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
				|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
			   ) {
				wifi_hdr->FC.ToDs = 1;

				if (pTxBlk->pMacEntry == NULL) {
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
						"pTxBlk->pMacEntry == NULL!\n");
				} else {
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);				/* to AP2 */
				}
				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);						/* from AP1 */
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
				COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
				pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
				pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
			} else
#endif /* WDS_SUPPORT || CLIENT_WDS */
			{
				/* TODO: how about "MoreData" bit? AP need to set this bit especially for PS-POLL response */
#if defined(IGMP_SNOOP_SUPPORT)
				if (pTxBlk->tr_entry->EntryType != ENTRY_CAT_MCAST) {
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr); /* DA */
				} else
#endif /* defined(IGMP_SNOOP_SUPPORT) */
				{
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
				}

				if ((wdev->func_idx >= 0) && (wdev->func_idx < MAX_BEACON_NUM))
					COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid);		/* BSSID */
				else
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
						"wdev->func_idx is out of valid range!\n");
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);			/* SA */
			}

	if (!IS_CIPHER_NONE(pTxBlk->CipherAlg))
		wifi_hdr->FC.Wep = 1;

	pTxBlk->dot11_type = wifi_hdr->FC.Type;
	pTxBlk->dot11_subtype = wifi_hdr->FC.SubType;

#ifdef SW_CONNECT_SUPPORT
	if (TX_BLK_TEST_FLAG2(pTxBlk, fTX_bSnVld))
		pTxBlk->sn = wifi_hdr->Sequence;
#endif /* SW_CONNECT_SUPPORT */
}

BOOLEAN ap_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	pPacket = pTxBlk->pPacket;
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->pMbss = NULL;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_BMC_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);


	if (pTxBlk->tr_entry->EntryType == ENTRY_CAT_MCAST) {
		pMacEntry = entry_get(pAd, wdev->tr_tb_idx);
		pTxBlk->pMacEntry = NULL;
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);
		{
#ifdef MCAST_RATE_SPECIFIC
			PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);

			if (((*pDA & 0x01) == 0x01) && (*pDA != 0xff)) {
#ifdef MCAST_VENDOR10_CUSTOM_FEATURE
				pTxBlk->pTransmit = (wdev->channel > 14) ?
					(&wdev->rate.MCastPhyMode_5G) :
					(&wdev->rate.MCastPhyMode);
#else
				pTxBlk->pTransmit = &wdev->rate.mcastphymode;
#endif
			} else
#endif /* MCAST_RATE_SPECIFIC */
			{
				pTxBlk->pTransmit = &pMacEntry->HTPhyMode;

				if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G) {
					pTxBlk->pTransmit->field.MODE = MODE_OFDM;
					pTxBlk->pTransmit->field.MCS = MCS_RATE_6;
				}
			}
		}
		/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, (fTX_bAckRequired | fTX_bAllowFrag | fTX_bWMM));

		if (RTMP_GET_PACKET_MOREDATA(pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
	} else {
		if (IS_WCID_VALID(pAd, pTxBlk->Wcid)) {
			pTxBlk->pMacEntry = entry_get(pAd, pTxBlk->Wcid);
			pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;
			pMacEntry = pTxBlk->pMacEntry;
		}

		if (!pMacEntry) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				"Err!! pMacEntry is NULL!!\n");
		} else {
			pTxBlk->pMbss = MBSS_GET(pMacEntry->pMbss);
		}

#ifdef MULTI_WMM_SUPPORT

		if (IS_ENTRY_PEER_AP(pMacEntry))
			pTxBlk->QueIdx = EDCA_WMM1_AC0_PIPE;

#endif /* MULTI_WMM_SUPPORT */
		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
#ifdef MULTI_WMM_SUPPORT

		if (pTxBlk->QueIdx >= EDCA_WMM1_AC0_PIPE) {
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		} else
#endif /* MULTI_WMM_SUPPORT */
		{
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		}

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WDS_SUPPORT
			if (pMacEntry && IS_ENTRY_WDS(pMacEntry))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWDSEntry);
			else
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
				if (pMacEntry && IS_ENTRY_CLIWDS(pMacEntry)) {
					PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
					PUCHAR pSA = GET_OS_PKT_DATAPTR(pPacket) + MAC_ADDR_LEN;
				UCHAR idx = pMacEntry->func_tb_idx;

				if (((idx < MAX_MBSSID_NUM(pAd))
					 && !MAC_ADDR_EQUAL(pSA, pAd->ApCfg.MBSSID[idx].wdev.bssid))
						|| !MAC_ADDR_EQUAL(pDA, pMacEntry->Addr)
					   )
						TX_BLK_SET_FLAG(pTxBlk, fTX_bClientWDSFrame);
				} else
#endif /* CLIENT_WDS */
					if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
#ifdef A4_CONN
				if (IS_ENTRY_A4(pMacEntry)
				&& !RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
#endif /* A4_CONN */
					} else
						return FALSE;

			/* If both of peer and us support WMM, enable it.*/
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)
				&& CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME) {
			if (((RTMP_GET_PACKET_LOWRATE(pPacket))
#ifdef UAPSD_SUPPORT
				  && (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */
				 ) ||
#ifdef SW_CONNECT_SUPPORT
				(IS_SW_MAIN_STA(pTxBlk->tr_entry) || IS_SW_STA(pTxBlk->tr_entry)) ||
#endif /* SW_CONNECT_SUPPORT */
				 ((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			   ) {
				/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate. */
				pTxBlk->pTransmit = &(wdev->rate.MlmeTransmit);
#ifdef SW_CONNECT_SUPPORT
				if (RTMP_GET_PACKET_LOWRATE(pPacket)) {
					pTxBlk->pTransmit = &(wdev->rate.MlmeTransmit);
				} else if (pAd->dummy_obj.bFixedRateSet &&
					(IS_SW_MAIN_STA(pTxBlk->tr_entry) || IS_SW_STA(pTxBlk->tr_entry))) {
					pTxBlk->fr_tbl_idx = pAd->dummy_obj.fr_tbl_idx;
					pTxBlk->fr_bw = pAd->dummy_obj.fr_bw;
				}
#endif /* SW_CONNECT_SUPPORT */
				TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (!pTxBlk->pMacEntry) {
					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
						"Err!! pTxBlk->pMacEntry is NULL!!\n");
				} else if (IS_HT_STA(pTxBlk->pMacEntry) &&
						 (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET))) {
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
				}
			}

			if (!pMacEntry)
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
					"Err!! pMacEntry is NULL!!\n");

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);

#ifdef UAPSD_SUPPORT

			if (RTMP_GET_PACKET_EOSP(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);

#endif /* UAPSD_SUPPORT */
		} else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);

		if (!pMacEntry)
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_DEBUG,
				"Err!! pMacEntry is NULL!!\n");
		else
			pMacEntry->DebugTxCount++;

#ifdef IGMP_SNOOP_SUPPORT
		if (RTMP_GET_PACKET_MCAST_CLONE(pPacket)) {
			TX_BLK_SET_FLAG(pTxBlk, fTX_MCAST_CLONE);
#ifdef IGMP_SNOOPING_NON_OFFLOAD
			TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);
#endif
		}
#endif
#ifdef PER_PKT_CTRL_FOR_CTMR
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
		{
			BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

			if (pMbss && pMbss->HotSpotCtrl.HotSpotEnable
					&& pMbss->WNMCtrl.ProxyARPEnable) {
				if (pPacket && (RTMP_GET_PACKET_ARP(pPacket) || RTMP_GET_PACKET_DHCP(pPacket)))
					RTMP_SET_PACKET_HS2_TX(pPacket, 1);
				else
					RTMP_SET_PACKET_HS2_TX(pPacket, 0);
			}
#ifdef CONFIG_PROXY_ARP
			else if (pMbss && pMbss->WNMCtrl.ProxyARPEnable) {
				if (pPacket && (RTMP_GET_PACKET_ARP(pPacket)))
					RTMP_SET_PACKET_HS2_TX(pPacket, 1);
				else
					RTMP_SET_PACKET_HS2_TX(pPacket, 0);
			}
#endif
		}
#endif /* HOSTAPD_HS_R2_SUPPORT */
#ifdef CONFIG_FAST_NAT_SUPPORT
				if (pMacEntry && (pMacEntry->ForceSwPath & FORCE_SW_PATH_TX))
					TX_BLK_SET_FLAG2(pTxBlk, fTX_SW_PATH);
#endif
#endif
#ifdef SW_CONNECT_SUPPORT
		/* correct to H/W WCID before fill TxD */
		if (IS_SW_MAIN_STA(pTxBlk->tr_entry) || IS_SW_STA(pTxBlk->tr_entry)) {
			/* only Unicast entry will bet set as S/W */
			TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);
			TX_BLK_SET_FLAG2(pTxBlk, fTX_bSnVld);
			TX_BLK_SET_FLAG2(pTxBlk, fTX_bSwSTA_WithOutWTBL);
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_HDR_TRANS);
			if (IS_SW_STA(pTxBlk->tr_entry))
				pTxBlk->Wcid = pTxBlk->tr_entry->HwWcid;
		}
#endif /* SW_CONNECT_SUPPORT */
	}

	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;
	ops->find_cipher_algorithm(pAd, wdev, pTxBlk);

#ifdef PER_PKT_CTRL_FOR_CTMR
	if (RTMP_GET_PACKET_EAPOL(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceLink);

	if (is_tcp_ctrl_frame(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_HIGH_PRIO);

	if (pTxBlk->ApplyTid) {
		if (pTxBlk->Tid > 7)
			pTxBlk->Tid = 7;

		pTxBlk->UserPriority = pTxBlk->Tid;
		pTxBlk->QueIdx = WMM_UP2AC_MAP[pTxBlk->UserPriority];
	}

	if (pTxBlk->ApplyFixRate) {
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);
		pTxBlk->fr_tbl_idx = pTxBlk->FrIdx;
		pTxBlk->fr_bw = pTxBlk->bw;
	} else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_ForceRate);
#endif

	return TRUE;
}

BOOLEAN ap_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk)
{
	PNDIS_PACKET pPacket;
	struct _BSS_INFO_ARGUMENT_T *bssinfo;
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
	BSS_STRUCT * pMbss = NULL;
#endif /* HOSTAPD_HS_R2_SUPPORT */
#if defined(DOT11_EHT_BE) || defined(CONFIG_FAST_NAT_SUPPORT)
	struct _MAC_TABLE_ENTRY *MacEntry = pTxBlk->pMacEntry;
#endif
#ifdef CONFIG_6G_SUPPORT
	UCHAR iob_mode = wlan_operate_get_unsolicit_tx_mode(wdev);
#endif /* CONFIG_6G_SUPPORT */
	pPacket = pTxBlk->pPacket;
	pTxBlk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(pPacket);
	pTxBlk->SrcBufLen = RTMP_GET_PKT_LEN(pPacket);
	bssinfo = &wdev->bss_info_argument;

	if (RTMP_GET_PACKET_MGMT_PKT(pPacket)) {

		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

#ifdef DOT11_EHT_BE
		if (MacEntry && MacEntry->mlo.mlo_en)
			pTxBlk->fr_tbl_idx = bssinfo->mloFixedRateIdx;
		else
#endif /* DOT11_EHT_BE */
		{
			if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
				(wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G)) {
				if (pAd->CommonCfg.bSeOff != TRUE)
					pTxBlk->fr_tbl_idx = FR_CCK_SPE0x18_11M;
				else
					pTxBlk->fr_tbl_idx = FR_CCK_11M;
			} else {
				if (pAd->CommonCfg.bSeOff != TRUE)
					pTxBlk->fr_tbl_idx = FR_OFDM_SPE0x18_6M;
				else
					pTxBlk->fr_tbl_idx = FR_OFDM_6M;
			}
			pTxBlk->fr_bw = BW_20;
		}

#ifdef MLR_SUPPORT
		if (MacEntry && MacEntry->MlrCurState == MLR_STATE_START)
			pTxBlk->fr_tbl_idx = FR_MLR_SPE0x18_MCS0;
#endif /* MLR_SUPPORT */

		/*Fixed or auto BW*/
		if (pAd->CommonCfg.wifi_cert)
			pTxBlk->fr_bw |= 8; /*Fixed BW*/
		else {
#ifdef CONFIG_6G_SUPPORT
			if (iob_mode == UNSOLICIT_TXMODE_NON_HT_DUP
#ifdef CONFIG_6G_AFC_SUPPORT
				&& (!is_afc_in_run_state(pAd) || pAd->CommonCfg.AfcSpBwDup)
#endif /*CONFIG_6G_AFC_SUPPORT*/
				)
				pTxBlk->fr_bw = 0; /*Auto BW*/
			else
#endif /* CONFIG_6G_SUPPORT */
				pTxBlk->fr_bw |= 8; /*Fixed BW*/
		}

		if (is_force_link_mgmt_frame(pAd, pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_ForceLink);
	}

	/*error handling:if 5G band use cck 1m fixed rate*/
	if ((wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G) &&
		(pTxBlk->fr_tbl_idx == FR_CCK_1M || pTxBlk->fr_tbl_idx == FR_CCK_11M)) {
		if (pAd->CommonCfg.bSeOff != TRUE)
			pTxBlk->fr_tbl_idx = FR_OFDM_SPE0x18_6M;
		else
			pTxBlk->fr_tbl_idx = FR_OFDM_6M;
	}

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	/* To make sure eapol frames go to setup link */
	if (RTMP_GET_PACKET_EAPOL(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceLink);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_BMC_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	} else {
		struct wifi_dev_ops *ops = wdev->wdev_ops;

		/* only MAC TXD mode if not HW TX HDR_TRANS */
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);
		ops->find_cipher_algorithm(pAd, wdev, pTxBlk);

		/* If both of peer and us support WMM, enable it.*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)
			&& pTxBlk->pMacEntry && CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
			TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);

		if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		else
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	}


	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (is_tcp_ctrl_frame(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_HIGH_PRIO);
	/*get MBSS for tx counter usage*/
	if ((pTxBlk->TxFrameType != TX_BMC_FRAME) && (pTxBlk->pMacEntry))
		pTxBlk->pMbss = MBSS_GET(pTxBlk->pMacEntry->pMbss);
	if (RTMP_TEST_FLAG(pAd->physical_dev, PH_DEV_CHECK_ETH_TYPE))
		pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);

#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
	pMbss = (BSS_STRUCT *)wdev->func_dev;
	if (pMbss) {
		if (pMbss->HotSpotCtrl.HotSpotEnable || pMbss->WNMCtrl.ProxyARPEnable) {
			if (pPacket && (RTMP_GET_PACKET_DHCP(pPacket) || RTMP_GET_PACKET_ARP(pPacket) || RTMP_GET_PACKET_IPv6(pPacket)))
				RTMP_SET_PACKET_HS2_TX(pPacket, 1);
			else
				RTMP_SET_PACKET_HS2_TX(pPacket, 0);
		}
	}
#endif /* HOSTAPD_HS_R2_SUPPORT */
#ifdef CONFIG_FAST_NAT_SUPPORT
	if (MacEntry && (MacEntry->ForceSwPath & FORCE_SW_PATH_TX))
		TX_BLK_SET_FLAG2(pTxBlk, fTX_SW_PATH);
#endif
	return TRUE;
}

#ifdef TXRX_STAT_SUPPORT
VOID ap_tx_update_ctrl_mgmt_cnts(RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk)
{
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UCHAR *pData = GET_OS_PKT_DATAPTR(tx_blk->pPacket);
	HEADER_802_11 *pHeader_802_11 = (HEADER_802_11 *)(pData + tx_hw_hdr_len);

	if (tx_blk->TxFrameType == TX_MLME_MGMTQ_FRAME ||
			tx_blk->TxFrameType == TX_MLME_DATAQ_FRAME) {
		pAd->WlanCounters.TxManagementBytes.QuadPart += tx_blk->SrcBufLen;
		INC_COUNTER64(pAd->WlanCounters.TxManagementCnt);
	}

	if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_QOS_NULL:
			INC_COUNTER64(pAd->WlanCounters.TxQosNullCnt);
			break;

		case SUBTYPE_DATA_NULL:
			INC_COUNTER64(pAd->WlanCounters.TxDataNullCnt);
			break;
		}
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_BLOCK_ACK_REQ:
			INC_COUNTER64(pAd->WlanCounters.TxBarCnt);
			break;

		case SUBTYPE_BLOCK_ACK:
			INC_COUNTER64(pAd->WlanCounters.TxBaCnt);
			break;

		case SUBTYPE_PS_POLL:
			INC_COUNTER64(pAd->WlanCounters.TxPsPollCnt);
			break;
		}
	} else if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_ASSOC_REQ:
			INC_COUNTER64(pAd->WlanCounters.TxAssocReqCnt);
			break;

		case SUBTYPE_ASSOC_RSP:
			INC_COUNTER64(pAd->WlanCounters.TxAssocRspCnt);
			break;

		case SUBTYPE_REASSOC_REQ:
			INC_COUNTER64(pAd->WlanCounters.TxReassocReqCnt);
			break;

		case SUBTYPE_REASSOC_RSP:
			INC_COUNTER64(pAd->WlanCounters.TxReassocRspCnt);
			break;

		case SUBTYPE_PROBE_REQ:
			INC_COUNTER64(pAd->WlanCounters.TxProbeReqCnt);
			break;

		case SUBTYPE_PROBE_RSP:
			INC_COUNTER64(pAd->WlanCounters.TxProbeRspCnt);
			break;

		case SUBTYPE_DISASSOC:
			INC_COUNTER64(pAd->WlanCounters.TxDisassocCnt);
			break;

		case SUBTYPE_AUTH:
			INC_COUNTER64(pAd->WlanCounters.TxAuthCnt);
			break;

		case SUBTYPE_DEAUTH:
			INC_COUNTER64(pAd->WlanCounters.TxDeauthCnt);
			break;

		case SUBTYPE_ACTION:
		case SUBTYPE_ACTION_NO_ACK:
			INC_COUNTER64(pAd->WlanCounters.TxActionCnt);
			break;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_CNT, DBG_LVL_DEBUG, "<--(%d) invalid type\n", __LINE__);
	}
}
#endif /* TXRX_STAT_SUPPORT */

INT ap_mgmt_emlsr_update(RTMP_ADAPTER *pAd, MAC_TX_INFO *info, struct _TX_BLK *tx_blk)
{
#ifdef DOT11_EHT_BE
	struct _RTMP_CHIP_CAP *cap = NULL;
	UINT8 tx_hw_hdr_len = 0;
	struct wifi_dev *wdev = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	HEADER_802_11 *pHeader_802_11 = NULL;

	if (!pAd || !info || !tx_blk)
		return NDIS_STATUS_FAILURE;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	tx_hw_hdr_len = cap->tx_hw_hdr_len;

	pMacEntry = tx_blk->pMacEntry;
	if (!pMacEntry)
		return NDIS_STATUS_FAILURE;

	wdev = pMacEntry->wdev;
	if (!wdev)
		return NDIS_STATUS_FAILURE;

	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

	if (IS_ENTRY_MLO(pMacEntry) && !IS_BM_MAC_ADDR(pHeader_802_11->Addr1)) {
		u16 emlsr = 0;
		struct mld_entry_t *mld_entry = NULL;

		mt_rcu_read_lock();
		mld_entry = rcu_dereference(pMacEntry->mld_entry);
		if (mld_entry && mld_entry->valid)
			emlsr = mld_entry->emlsr;
		mt_rcu_read_unlock();

		if (emlsr) {
			TX_BLK_CLEAR_FLAG(tx_blk, fTX_ForceRate);
			info->q_idx = asic_get_hwq_from_ac(pAd, HcGetWmmIdx(pAd, wdev), QID_AC_BE);
		}
	}
#endif /* DOT11_EHT_BE */

	return NDIS_STATUS_SUCCESS;
}

INT ap_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk)
{
	UCHAR *tmac_info;
	union _HTTRANSMIT_SETTING *transmit, tmp_transmit;
	UCHAR MlmeRate;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PHEADER_802_11 pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry = tx_blk->pMacEntry;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR PID, tx_rate;
	UINT16 wcid, hw_wcid;
	UCHAR prot = 0;
	UCHAR apidx = 0;
	MAC_TX_INFO mac_info;
	struct DOT11_H *pDot11h = NULL;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
#ifdef	CONFIG_RCSA_SUPPORT
	UINT8 BandIdx = 0;
#ifdef MT_DFS_SUPPORT
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif
#endif
#ifdef DPP_SUPPORT
	UINT16 orig_sn;
#endif /* DPP_SUPPORT */
	int ret;
	struct _STA_TR_ENTRY *tr_entry;

	if (wdev == NULL || wdev->pDot11_H == NULL) {
		tr_cnt->tx_invalid_wdev++;
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pDot11h = wdev->pDot11_H;

	ap_fill_offload_tx_blk(pAd, wdev, tx_blk);
	tmac_info = tx_blk->pSrcBufHeader;
	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	/*
		copy to local var first to prevernt the dev->rate.MlmeTransmit is change this moment
	*/
	NdisMoveMemory(&tmp_transmit, &wdev->rate.MlmeTransmit, sizeof(union _HTTRANSMIT_SETTING));
	transmit = &tmp_transmit;
	/* Verify Mlme rate for a / g bands.*/
	if ((wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G) &&
		(MlmeRate < RATE_6)) {
		MlmeRate = RATE_6;
		transmit->field.MCS = MCS_RATE_6;
		transmit->field.MODE = MODE_OFDM;
	}

	pHeader_802_11->FC.MoreData = RTMP_GET_PACKET_MOREDATA(tx_blk->pPacket);
	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) { /* must be PS-POLL*/
		bAckRequired = FALSE;
#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			pHeader_802_11->Duration = 100;
		}

#endif /* VHT_TXBF_SUPPORT */
	} else { /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
		if (pHeader_802_11->Addr1[0] & 0x01) { /* MULTICAST, BROADCAST*/
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		} else {
#ifdef SOFT_SOUNDING

			if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
				&& pMacEntry && (pMacEntry->snd_reqired == TRUE)) {
				bAckRequired = FALSE;
				pHeader_802_11->Duration = 0;
			} else
#endif /* SOFT_SOUNDING */
			{
				bAckRequired = TRUE;
#ifdef ZERO_PKT_LOSS_SUPPORT
				if (pAd->Zero_Loss_Enable && pDot11h->ChnlSwitchState == ASIC_CHANNEL_SWITCH_COMMAND_ISSUED &&
					(pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL))  {
					TX_BLK_SET_FLAG(tx_blk, fTX_bNoRetry);
				}
#endif
				pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);

				if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
					bInsertTimestamp = TRUE;
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Response*/
#ifdef SPECIFIC_TX_POWER_SUPPORT
					/* Find which MBSSID to be send this probeRsp */
					UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

					if (!(apidx >= pAd->ApCfg.BssidNum) &&
						 (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
						 (transmit->field.MODE == MODE_CCK) &&
						 (transmit->field.MCS == RATE_1))
						TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;

#endif /* SPECIFIC_TX_POWER_SUPPORT */
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Request*/
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH) &&
						   (pMacEntry == NULL)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Deauth */
				}
			}
		}
	}

#ifdef DPP_SUPPORT
	orig_sn = pHeader_802_11->Sequence;
#endif /* DPP_SUPPORT */
	pHeader_802_11->Sequence = pAd->Sequence++;

	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/*
		Before radar detection done, mgmt frame can not be sent but probe req
		Because we need to use probe req to trigger driver to send probe req in passive scan
	*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pHeader_802_11->FC.SubType != SUBTYPE_ACTION)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->ChannelMode != CHAN_NORMAL_MODE)) {
		tr_cnt->carrier_detect_drop++;
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	/*
		fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one physical buffer, and the whole frame size equals
		to the first scatter buffer size

		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement

		management frame doesn't need encryption.
		so use WCID_NO_MATCHED no matter u are sending to specific wcid or not
	*/
	PID = PID_MGMT;
#ifdef DOT11W_PMF_SUPPORT
	PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &prot);
#endif /* DOT11W_PMF_SUPPORT */

	if (pMacEntry) {
		hw_wcid = wcid = pMacEntry->wcid;
#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_STA(tx_blk->tr_entry))
			hw_wcid = tx_blk->tr_entry->HwWcid;
#endif /* SW_CONNECT_SUPPORT */
	} else {
		hw_wcid = wcid = 0;
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO, "pMacEntry is null !!\n");
	}

	tx_rate = (UCHAR)transmit->field.MCS;
	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	if (prot)
		mac_info.prot = prot;

	if (prot == 2 || prot == 3)
		mac_info.bss_idx = apidx;

	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;
	mac_info.Ack = bAckRequired;
	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = hw_wcid;
	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.PsmBySw = 0;
	mac_info.txpwr_offset = 0;

	/* check if the pkt is Tmr frame. */
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if (wdev->eap.eap_mgmrate_en >= (1 << MGMT_TYPE)) {

			if (wdev->eap.eap_mgmrate_en & (1 << MGMT_TYPE))
				transmit = &wdev->eap.mgmphymode[MGMT_TYPE];

			switch (pHeader_802_11->FC.SubType) {
			case SUBTYPE_PROBE_REQ:
			case SUBTYPE_PROBE_RSP:
				if (wdev->eap.eap_mgmrate_en & (1 << PROBE_TYPE))
					transmit = &wdev->eap.mgmphymode[PROBE_TYPE];
				break;
			case SUBTYPE_AUTH:
			case SUBTYPE_DEAUTH:
				if (wdev->eap.eap_mgmrate_en & (1 << AUTH_TYPE))
					transmit = &wdev->eap.mgmphymode[AUTH_TYPE];
				break;
			case SUBTYPE_ASSOC_REQ:
			case SUBTYPE_ASSOC_RSP:
			case SUBTYPE_REASSOC_REQ:
			case SUBTYPE_REASSOC_RSP:
			case SUBTYPE_DISASSOC:
				if (wdev->eap.eap_mgmrate_en & (1 << ASSOC_TYPE))
					transmit = &wdev->eap.mgmphymode[ASSOC_TYPE];
				break;
			case SUBTYPE_ACTION:
			case SUBTYPE_ACTION_NO_ACK:
				if (wdev->eap.eap_mgmrate_en & (1 << ACTION_TYPE))
					transmit = &wdev->eap.mgmphymode[ACTION_TYPE];
				break;
			default:
				break;
			}
			tx_rate = (UCHAR)transmit->field.MCS;

			tx_blk->fr_tbl_idx = transmit_to_fr_idx(transmit->word);
#ifdef DOT11_EHT_BE
			if (pMacEntry && pMacEntry->mlo.mlo_en) {
				if (transmit->field.MODE == MODE_CCK)
					tx_blk->fr_tbl_idx = wdev->bss_info_argument.mloFixedRateIdx;
			}
#endif /* DOT11_EHT_BE */

		}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */

#ifdef DPP_SUPPORT
		/* Check whether it is a DPP frame */
		if (!mac_info.BM && (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) &&
			((tx_blk->SrcBufLen - tx_hw_hdr_len) > sizeof(HEADER_802_11))) {
			GAS_FRAME *GASFrame = (GAS_FRAME *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
			UCHAR *pkt_buf = (tx_blk->pSrcBufHeader + tx_hw_hdr_len + sizeof(HEADER_802_11));
			/* Public action frame with WFA DPP */
			if ((pkt_buf[0] == CATEGORY_PUBLIC) &&
					(pkt_buf[1] == ACTION_WIFI_DIRECT) && /*vendor specific*/
					(memcmp(&pkt_buf[2], DPP_OUI, OUI_LEN) == 0) &&
					(pkt_buf[2 + OUI_LEN] == WFA_DPP_SUBTYPE) &&
					(!MAC_ADDR_IS_GROUP(pHeader_802_11->Addr1))) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DPP, DBG_LVL_INFO,
					"sending out a dpp frame on interface:%s da:%02x:%02x:%02x:%02x:%02x:%02x\n",
					wdev->if_dev->name, PRINT_MAC(pHeader_802_11->Addr1));
				PID = PID_MGMT_DPP_FRAME;
				mac_info.seq_no = orig_sn;
			}
			/* Gas DPP frame */
			if (((GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
				NdisEqualMemory(&GASFrame->u.GAS_INIT_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
				GASFrame->u.GAS_INIT_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE)
				||
				((GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Length_Index] > GAS_WFA_DPP_Min_Length) &&
				NdisEqualMemory(&GASFrame->u.GAS_CB_RSP.Variable[GAS_OUI_Index], DPP_OUI, OUI_LEN) &&
				GASFrame->u.GAS_CB_RSP.Variable[GAS_WFA_DPP_Subtype_Index] == WFA_DPP_SUBTYPE)) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_DPP, DBG_LVL_INFO,
					"sending out a dpp frame on interface %s\n",
					wdev->if_dev->name);
				PID = PID_MGMT_DPP_FRAME;
				mac_info.seq_no = orig_sn;
				transmit = &tmp_transmit;
				transmit->field.MODE = MODE_OFDM;
				transmit->field.BW = BW_20;
				transmit->field.MCS = MCS_RATE_6;
				tx_rate = transmit->field.MCS;
			}
		}
#endif /* DPP_SUPPORT */
		mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_DATA_NULL:
			mac_info.hdr_len = 24;
			tx_rate = (UCHAR)transmit->field.MCS;
			/* use AC queue to get tx success status */
			//RTMP_SET_PACKET_TYPE(tx_blk->pPacket, TX_ALTX);
			break;

		case SUBTYPE_QOS_NULL:
			mac_info.hdr_len = 26;
			tx_rate = (UCHAR)transmit->field.MCS;
			/* use AC queue to get tx success status */
			//RTMP_SET_PACKET_TYPE(tx_blk->pPacket, TX_ALTX);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
					 "FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			hex_dump("DataFrame", (char *)pHeader_802_11, 24);
			break;
		}

		if (pMacEntry && IS_WCID_VALID(pAd, wcid)) {
			tr_entry = tr_entry_get(pAd, wcid);
			if (tr_entry->PsDeQWaitCnt)
				PID = PID_PS_DATA;
		}

		mac_info.WCID = hw_wcid;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_PS_POLL:
			mac_info.hdr_len = sizeof(PSPOLL_FRAME);
			tx_rate = (UCHAR)transmit->field.MCS;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
				pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			break;
		}
	} else {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"FIXME!!!Unexpected frame send to MgmtRing, need to assign the length!\n");
	}

	mac_info.PID = PID;
	mac_info.TID = 0;
	mac_info.TxRate = tx_rate;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	mac_info.wmm_set = HcGetWmmIdx(pAd, wdev);
	mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));

	mac_info.OmacIdx = wdev->OmacIdx;

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_DEBUG,
			 "%d, WMMSET=%d,QId=%d\n",
			  __LINE__, mac_info.wmm_set, mac_info.q_idx);

	/* For wifi7 4.58 ROM initator , it need to send non-ofdm qos null pkt  */
	if (wdev->bMgmtQAutoRate) {
		if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)
		&& (pHeader_802_11->FC.Order))
			if (pMacEntry != NULL) {
				TX_BLK_CLEAR_FLAG(tx_blk, fTX_ForceRate);
				mac_info.IsAutoRate = TRUE;
			}
	}

#ifdef APCLI_SUPPORT
	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))) {
		if ((pMacEntry != NULL) && (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry))) {
			/* CURRENT_BW_TX_CNT/CURRENT_BW_FAIL_CNT only count for aute rate */
			mac_info.IsAutoRate = TRUE;

			{
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
				PSTA_ADMIN_CONFIG sta_cfg = GetStaCfgByWdev(pAd, pMacEntry->wdev);

				if (sta_cfg && twtPlannerIsRunning(pAd, sta_cfg))
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
					mac_info.Ack = 0;
			}
		}
	}
	if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
		UCHAR Category = (UCHAR) (pHeader_802_11->Octet[(pHeader_802_11->FC.Order ? 4 : 0)]);

		if (Category == CATEGORY_VSP) {
			TX_BLK_SET_FLAG(tx_blk, fTX_ForceLink);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO, "VSP/WNM/RM Force link, wcid = %d !!\n", tx_blk->Wcid);
		}
	}

#ifdef	CONFIG_RCSA_SUPPORT
	if ((pDot11h->ChannelMode == CHAN_SWITCHING_MODE) && (pDfsParam->bRCSAEn == TRUE)
		&& (pHeader_802_11->FC.Type == FC_TYPE_MGMT) && (pHeader_802_11->FC.SubType == SUBTYPE_ACTION)) {
		BandIdx = HcGetBandByWdev(wdev);
		RTMP_SET_PACKET_TYPE(tx_blk->pPacket, TX_ALTX);
		mac_info.q_idx = TxQ_IDX_ALTX0;

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO, "Set q_idx to %d\n",
					 mac_info.q_idx);

		mac_info.Type = FC_TYPE_MGMT;
		mac_info.SubType = SUBTYPE_ACTION;
	}
#endif
#endif /* APCLI_SUPPORT */
#ifdef SOFT_SOUNDING

	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
		&& pMacEntry && (pMacEntry->snd_reqired == TRUE)) {
		tx_rate = (UCHAR)pMacEntry->snd_rate.field.MCS;
		NdisMoveMemory(&tmp_transmit, &pMacEntry->snd_rate, sizeof(union _HTTRANSMIT_SETTING));
		mac_info.Txopmode = IFS_PIFS;
		pMacEntry->snd_reqired = FALSE;
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_SOUND, DBG_LVL_INFO,
			"Kick Sounding to "MACSTR", dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
			MAC2STR(pMacEntry->Addr),
			get_phymode_str(transmit->field.MODE),
			get_bw_str(transmit->field.BW, BW_FROM_OID),
			(transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf));
	} else
#endif /* SOFT_SOUNDING */
	{
		mac_info.Txopmode = IFS_BACKOFF;
	}

	/* if we are going to send out FTM action. enable CR to report TMR report.*/
	if ((pAd->pTmrCtrlStruct != NULL) && (pAd->pTmrCtrlStruct->TmrEnable != TMR_DISABLE)) {
		if (mac_info.IsTmr == TRUE) {
			pAd->pTmrCtrlStruct->TmrState = SEND_OUT;
		}
	}

#ifdef OCE_SUPPORT
	if (IS_OCE_ENABLE(wdev) && (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_24G) &&
		pHeader_802_11->FC.Type == FC_TYPE_MGMT && pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) {
		transmit->field.MODE = (transmit->field.MODE >= MODE_OFDM) ? transmit->field.MODE : MODE_OFDM;
	}
#endif /* OCE_SUPPORT */
	if (wdev) {
		if (wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G) {
			if (transmit->field.MODE == MODE_CCK) {
				/*
				    something wrong with rate->MlmeTransmit
					correct with OFDM mode
				*/
				transmit->field.MODE = MODE_OFDM;
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					"@@@@ FIXME!!frame(Type=%x, SubType=%x) use the CCK RATE but wdev support A band only, mac_info.Length=%lu, mac_info.wmm_set=%d, mac_info.q_idx=%d, mac_info.OmacIdx=%d\n",
						 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType, mac_info.Length, mac_info.wmm_set, mac_info.q_idx, mac_info.OmacIdx);
			}
		}
	}

#ifdef CONFIG_RCSA_SUPPORT
	/* Before sending RCSA using ALTx0 first flush it then enable as there might be pending pkts */
	if ((pDot11h->ChannelMode == CHAN_SWITCHING_MODE)
		&& (pDfsParam->bRCSAEn == TRUE)
		&& (pDfsParam->ChSwMode == 1)) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO,
			"Enable ALTX for BandIdx:%d for RCSA Tx\n", BandIdx);
		mtRddControl(pAd, RDD_ALTX_CTRL, BandIdx, 0, 1);
		pAd->CommonCfg.DfsParameter.fCheckRcsaTxDone = TRUE;
	}
#endif
#ifdef TXRX_STAT_SUPPORT
	{
		ap_tx_update_ctrl_mgmt_cnts(pAd, tx_blk);

		if (!(scan_in_run_state(pAd, wdev)) && pMacEntry && IS_ENTRY_CLIENT(pMacEntry) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
			struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
			INC_COUNTER64(pMacEntry->TxMgmtPacketCount);
			INC_COUNTER64(MBSS_GET(pMacEntry->pMbss)->stat_bss.TxMgmtPacketCount);
			INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxMgmtPacketCount);
			ctrl->rdev.pRadioCtrl->TxMgmtPacketByte.QuadPart += tx_blk->SrcBufLen;
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO, " On Channel ::  Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
		if ((scan_in_run_state(pAd, wdev)) && (pAd->ScanCtrl.ScanType == SCAN_ACTIVE) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)
				&& wdev && (wdev->wdev_idx >= 0) && (wdev->wdev_idx < MAX_BEACON_NUM)) {
			BSS_STRUCT *mbss = NULL;
			mbss = &pAd->ApCfg.MBSSID[wdev->wdev_idx];
			INC_COUNTER64(mbss->stat_bss.TxMgmtOffChPktCount);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO, " Off Channel :: Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
	}
#endif
#ifdef MGMT_TXPWR_CTRL
		if ((wdev->bPwrCtrlEn == TRUE) && (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) &&
			(pHeader_802_11->FC.Type == FC_TYPE_MGMT) && (wdev->tr_tb_idx != WCID_INVALID)) {
			mac_info.WCID = wdev->tr_tb_idx;
			mac_info.IsAutoRate = TRUE;
		}
#endif

	if ((RTMP_GET_PACKET_TYPE(tx_blk->pPacket) != TX_ALTX) && (mac_info.WCID == 0)) {
		enum PACKET_TYPE pkt_type_old = RTMP_GET_PACKET_TYPE(tx_blk->pPacket);
		UCHAR q_idx_old = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));

		RTMP_SET_PACKET_TYPE(tx_blk->pPacket, TX_ALTX);
		mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
		tr_cnt->pkt_invalid_wcid++;

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
			"pkt_t(%d),type(%d),sub_type(%d),q_idx(%d,%d),cnt(%d)\n",
			pkt_type_old, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType,
			q_idx_old, mac_info.q_idx, tr_cnt->pkt_invalid_wcid);

		if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
			PFRAME_ACTION_HDR Frame = (PFRAME_ACTION_HDR)pHeader_802_11;
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"cat(%d),act(%d)\n",
				Frame->Category, Frame->Action);
		}
	}

	/* For Tx mgmt enhancement feature, set PID if txs required*/
	if ((RTMP_GET_PACKET_REQ_TXS(tx_blk->pPacket))) {
		tx_blk->TxS2Host = TRUE;
		tx_blk->Pid = RTMP_GET_PACKET_PID(tx_blk->pPacket);
		mac_info.PID = RTMP_GET_PACKET_PID(tx_blk->pPacket);
	}

	asic_mgmt_emlsr_update(pAd, &mac_info, tx_blk);

#ifdef MGMT_TXPWR_CTRL
	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		if (transmit->field.MODE == MODE_CCK && transmit->field.MCS  <= MCS_SHORTP_RATE_11) {
			if (transmit->field.MCS >= MCS_SHORTP_RATE_1)
				mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset_cck[transmit->field.MCS - MCS_SHORTP_RATE_1];
			else
				mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset_cck[transmit->field.MCS];
		}

		if (transmit->field.MODE == MODE_OFDM && transmit->field.MCS <= MCS_RATE_54)
			mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset_ofdm[transmit->field.MCS];
	}
#endif

	ret = asic_mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
	if (ret != NDIS_STATUS_SUCCESS)
		tr_cnt->tx_hwifi_err_drop += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);

	return ret;
}

INT ap_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk)
{
	UCHAR *tmac_info, *frm_buf;
	UINT frm_len;
#ifdef CFG_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PHEADER_802_11 pHeader_802_11;
	PFRAME_BAR pBar = NULL;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR MlmeRate, tx_rate;
	UINT16 wcid, hw_wcid;
	MAC_TABLE_ENTRY *pMacEntry = tx_blk->pMacEntry;
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	union _HTTRANSMIT_SETTING *transmit, tmp_transmit, TransmitSetting;
	MAC_TX_INFO mac_info;
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	struct dev_rate_info *rate;
	struct DOT11_H *pDot11h = NULL;
	int ret;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

	if (wdev == NULL || wdev->pDot11_H == NULL) {
		tr_cnt->tx_invalid_wdev++;
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pDot11h = wdev->pDot11_H;

	ap_fill_offload_tx_blk(pAd, wdev, tx_blk);

	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

	rate = &wdev->rate;
	frm_buf = tx_blk->pSrcBufHeader;
	frm_len = tx_blk->SrcBufLen;
	tmac_info = tx_blk->pSrcBufHeader;

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;


	/*
		copy to local var first to prevernt the dev->rate.MlmeTransmit is change this moment
	*/
	NdisMoveMemory(&tmp_transmit, &wdev->rate.MlmeTransmit, sizeof(union _HTTRANSMIT_SETTING));
	transmit = &tmp_transmit;

	/* Verify Mlme rate for a/g bands.*/
	if ((wlan_config_get_ch_band(wdev) != CMD_CH_BAND_24G)
		&& (MlmeRate < RATE_6)) { /* 11A band*/
		MlmeRate = RATE_6;
		transmit->field.MCS = MCS_RATE_6;
		transmit->field.MODE = MODE_OFDM;
	}

	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/
	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */
	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		if (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ) {
			pBar = (PFRAME_BAR)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
			bAckRequired = TRUE;
		} else
			bAckRequired = FALSE;

#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			pHeader_802_11->Duration =
				RTMPCalcDuration(pAd, MlmeRate, (tx_blk->SrcBufLen - TXINFO_SIZE - cap->TXWISize - TSO_SIZE));
		}
#endif /* VHT_TXBF_SUPPORT*/
	} else { /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
		if (pHeader_802_11->Addr1[0] & 0x01) { /* MULTICAST, BROADCAST */
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		} else {
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);

			if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) {
				bInsertTimestamp = TRUE;
				bAckRequired = FALSE;
#ifdef SPECIFIC_TX_POWER_SUPPORT
				/* Find which MBSSID to be send this probeRsp */
				UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

				if (!(apidx >= pAd->ApCfg.BssidNum) &&
					(pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
					(transmit->field.MODE == MODE_CCK) &&
					(transmit->field.field.MCS == RATE_1))
					TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;

#endif /* SPECIFIC_TX_POWER_SUPPORT */
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;

	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/* Before radar detection done, mgmt frame can not be sent but probe req*/
	/* Because we need to use probe req to trigger driver to send probe req in passive scan*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pHeader_802_11->FC.SubType != SUBTYPE_ACTION)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->ChannelMode != CHAN_NORMAL_MODE)) {
		MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "MlmeHardTransmit --> radar detect not in normal mode !!!\n");
		tr_cnt->carrier_detect_drop++;
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (pMacEntry) {
		hw_wcid = wcid = pMacEntry->wcid;
#ifdef SW_CONNECT_SUPPORT
		if (IS_SW_STA(tx_blk->tr_entry))
			hw_wcid = tx_blk->tr_entry->HwWcid;
#endif /* SW_CONNECT_SUPPORT */
	} else {
		hw_wcid = wcid = 0;
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, "pMacEntry is null !!\n");
	}

	tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;
	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.Ack = bAckRequired;
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = hw_wcid;
	mac_info.TID = 0;
	mac_info.wmm_set = HcGetWmmIdx(pAd, wdev);
	mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
	mac_info.txpwr_offset = 0;
	mac_info.OmacIdx = wdev->OmacIdx;
	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;

		mac_info.PID = PID_MGMT;

		if (IS_ASIC_CAP(pAd, fASIC_CAP_ADDBA_HW_SSN)) {
			if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
				PFRAME_ACTION_HDR act_hdr = (PFRAME_ACTION_HDR)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

				if (act_hdr->Category == CATEGORY_BA) {
					PVOID ba_frame = (PVOID)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
					PFRAME_ADDBA_REQ addba_req;
					PFRAME_ADDBA_RSP addba_rsp;
					PFRAME_DELBA_REQ delba;
					PBA_PARM addba_parm;
					PDELBA_PARM delba_parm;
					USHORT ba_parm;
					BOOLEAN isHandled = TRUE;

					switch (act_hdr->Action) {
					case ADDBA_REQ:
						addba_req = (PFRAME_ADDBA_REQ)ba_frame;
						NdisMoveMemory(&ba_parm, &addba_req->BaParm,
							sizeof(BA_PARM));
						ba_parm = le2cpu16(ba_parm);
						addba_parm = (PBA_PARM)&ba_parm;
						mac_info.TID = addba_parm->TID;
						mac_info.addba = TRUE;
						break;
					case ADDBA_RESP:
						addba_rsp = (PFRAME_ADDBA_RSP)ba_frame;
						NdisMoveMemory(&ba_parm, &addba_rsp->BaParm,
							sizeof(BA_PARM));
						ba_parm = le2cpu16(ba_parm);
						addba_parm = (PBA_PARM)&ba_parm;
						mac_info.TID = addba_parm->TID;
						break;
					case DELBA:
						delba = (PFRAME_DELBA_REQ)ba_frame;
						NdisMoveMemory(&ba_parm, &delba->DelbaParm,
							sizeof(DELBA_PARM));
						ba_parm = le2cpu16(ba_parm);
						delba_parm = (PDELBA_PARM)&ba_parm;
						mac_info.TID = delba_parm->TID;
						break;
					default:
						isHandled = FALSE;
					}

					if (isHandled && mac_info.TID < ARRAY_SIZE(WMM_UP2AC_MAP))
						mac_info.q_idx = WMM_UP2AC_MAP[mac_info.TID];
				}
			}
		}

		if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
			UCHAR Category = (UCHAR) (pHeader_802_11->Octet[(pHeader_802_11->FC.Order ? 4 : 0)]);

			if (Category == CATEGORY_VSP) {
				TX_BLK_SET_FLAG(tx_blk, fTX_ForceLink);
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_INFO, "VSP Force link, wcid = %d !!\n", tx_blk->Wcid);
			}
		}

#ifdef DOT11W_PMF_SUPPORT
		PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &mac_info.prot);
#endif /* DOT11W_PMF_SUPPORT */
		mac_info.txpwr_offset = wdev->mgmt_txd_txpwr_offset;

#ifdef CONFIG_RA_PHY_RATE_SUPPORT
		if (wdev->eap.eap_mgmrate_en >= (1 << MGMT_TYPE)) {

			if (wdev->eap.eap_mgmrate_en & (1 << MGMT_TYPE))
				transmit = &wdev->eap.mgmphymode[MGMT_TYPE];

			switch (pHeader_802_11->FC.SubType) {
			case SUBTYPE_PROBE_REQ:
			case SUBTYPE_PROBE_RSP:
				if (wdev->eap.eap_mgmrate_en & (1 << PROBE_TYPE))
					transmit = &wdev->eap.mgmphymode[PROBE_TYPE];
				break;
			case SUBTYPE_AUTH:
			case SUBTYPE_DEAUTH:
				if (wdev->eap.eap_mgmrate_en & (1 << AUTH_TYPE))
					transmit = &wdev->eap.mgmphymode[AUTH_TYPE];
				break;
			case SUBTYPE_ASSOC_REQ:
			case SUBTYPE_ASSOC_RSP:
			case SUBTYPE_REASSOC_REQ:
			case SUBTYPE_REASSOC_RSP:
			case SUBTYPE_DISASSOC:
				if (wdev->eap.eap_mgmrate_en & (1 << ASSOC_TYPE))
					transmit = &wdev->eap.mgmphymode[ASSOC_TYPE];
				break;
			case SUBTYPE_ACTION:
			case SUBTYPE_ACTION_NO_ACK:
				if (wdev->eap.eap_mgmrate_en & (1 << ACTION_TYPE))
					transmit = &wdev->eap.mgmphymode[ACTION_TYPE];
				break;
			default:
				break;
			}
			tx_rate = (UCHAR)transmit->field.MCS;

			tx_blk->fr_tbl_idx = transmit_to_fr_idx(transmit->word);
#ifdef DOT11_EHT_BE
			if (pMacEntry && pMacEntry->mlo.mlo_en) {
				if (transmit->field.MODE == MODE_CCK)
					tx_blk->fr_tbl_idx = wdev->bss_info_argument.mloFixedRateIdx;
			}
#endif /* DOT11_EHT_BE */

		}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_DATA_NULL:
			mac_info.hdr_len = 24;
			tx_rate = (UCHAR)transmit->field.MCS;
			break;

		case SUBTYPE_QOS_NULL:
			mac_info.hdr_len = 26;
			tx_rate = (UCHAR)transmit->field.MCS;
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			hex_dump("DataFrame", frm_buf, frm_len);
			break;
		}

		mac_info.WCID = hw_wcid;

		if (pMacEntry && IS_WCID_VALID(pAd, wcid)) {
			struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, wcid);

			if (tr_entry->PsDeQWaitCnt)
				mac_info.PID = PID_PS_DATA;
			else
				mac_info.PID = PID_MGMT;
		} else
			mac_info.PID = PID_MGMT;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_BLOCK_ACK_REQ:
			mac_info.PID = PID_CTL_BAR;
			mac_info.hdr_len = 16;
			mac_info.SpeEn = 0;
			mac_info.TID = pBar->BarControl.TID;
			if (pBar->BarControl.TID < ARRAY_SIZE(WMM_UP2AC_MAP))
				mac_info.q_idx = WMM_UP2AC_MAP[pBar->BarControl.TID];
			if (((wlan_config_get_ch_band(wdev) == CMD_CH_BAND_6G) ||
				(wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G))
#ifdef GN_MIXMODE_SUPPORT
				|| (pAd->CommonCfg.GNMixMode
				    && (WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
					|| WMODE_EQUAL(wdev->PhyMode, WMODE_G)
					|| WMODE_EQUAL(wdev->PhyMode, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G |
					WMODE_BE_24G))))
#endif /*GN_MIXMODE_SUPPORT*/
			) {
				/* 5G */
				TransmitSetting.field.MODE = MODE_OFDM;
			} else {
				/* 2.4G */
				TransmitSetting.field.MODE = MODE_CCK;
			}

			TransmitSetting.field.BW = BW_20;
			TransmitSetting.field.STBC = 0;
			TransmitSetting.field.ShortGI = 0;
			TransmitSetting.field.MCS = 0;
			TransmitSetting.field.ldpc = 0;
			transmit = &TransmitSetting;
#ifdef CONFIG_RA_PHY_RATE_SUPPORT
			if (wdev->eap.eap_mgmrate_en & (1 << MGMT_TYPE)) {
				transmit = &wdev->eap.mgmphymode[MGMT_TYPE];
				tx_blk->fr_tbl_idx = transmit_to_fr_idx(transmit->word);
	#ifdef DOT11_EHT_BE
				if (pMacEntry && pMacEntry->mlo.mlo_en) {
					if (transmit->field.MODE == MODE_CCK)
						tx_blk->fr_tbl_idx = wdev->bss_info_argument.mloFixedRateIdx;
				}
	#endif /* DOT11_EHT_BE */
			}
#endif /* CONFIG_RA_PHY_RATE_SUPPORT */
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 " FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					  pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType);
			hex_dump("Control Frame", frm_buf, frm_len);
			break;
		}
	}

	mac_info.TxRate = tx_rate;
	mac_info.Txopmode = IFS_BACKOFF;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
#ifdef APCLI_SUPPORT

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))) {
		if ((pMacEntry != NULL) && (IS_ENTRY_PEER_AP(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry))) {
			/* CURRENT_BW_TX_CNT/CURRENT_BW_FAIL_CNT only count for aute rate */
			mac_info.IsAutoRate = TRUE;
		}
	}

#endif /* APCLI_SUPPORT */

	if (wdev) {
		if (WMODE_CAP_5G(wdev->PhyMode)) {
			if (transmit->field.MODE == MODE_CCK) {
				/*
				    something wrong with rate->MlmeTransmit
					correct with OFDM mode
				*/
				transmit->field.MODE = MODE_OFDM;
				MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, "@@@@ FIXME!!frame(Type=%d, SubType=%d) use the CCK RATE but wdev support A band only, mac_info.Length=%lu, mac_info.wmm_set=%d, mac_info.q_idx=%d, mac_info.OmacIdx=%d\n",
						 pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType, mac_info.Length, mac_info.wmm_set, mac_info.q_idx, mac_info.OmacIdx);
			}
		}
	}
#ifdef TXRX_STAT_SUPPORT
	{
		ap_tx_update_ctrl_mgmt_cnts(pAd, tx_blk);

		if (!(scan_in_run_state(pAd, wdev)) && pMacEntry && IS_ENTRY_CLIENT(pMacEntry) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
			struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
			INC_COUNTER64(pMacEntry->TxMgmtPacketCount);
			INC_COUNTER64(MBSS_GET(pMacEntry->pMbss)->stat_bss.TxMgmtPacketCount);
			INC_COUNTER64(ctrl->rdev.pRadioCtrl->TxMgmtPacketCount);
			ctrl->rdev.pRadioCtrl->TxMgmtPacketByte.QuadPart += tx_blk->SrcBufLen;
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, " On Channel ::  Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
		if ((scan_in_run_state(pAd, wdev)) && (pAd->ScanCtrl.ScanType == SCAN_ACTIVE) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)
				&& wdev && (wdev->wdev_idx >= 0) && (wdev->wdev_idx < MAX_BEACON_NUM)) {
			BSS_STRUCT *mbss = NULL;
			mbss = &pAd->ApCfg.MBSSID[wdev->wdev_idx];
			INC_COUNTER64(mbss->stat_bss.TxMgmtOffChPktCount);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, " Off Channel ::	Tx Mgmt Subtype : %d\n", pHeader_802_11->FC.SubType);
		}
	}
#endif
#ifdef MGMT_TXPWR_CTRL
		if ((wdev->bPwrCtrlEn == TRUE) && (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) &&
			(pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
			mac_info.WCID = wdev->tr_tb_idx;
			mac_info.IsAutoRate = TRUE;
		}
#endif

	if ((RTMP_GET_PACKET_TYPE(tx_blk->pPacket) != TX_ALTX) && (mac_info.WCID == 0)) {
		enum PACKET_TYPE pkt_type_old = RTMP_GET_PACKET_TYPE(tx_blk->pPacket);
		UCHAR q_idx_old = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));

		RTMP_SET_PACKET_TYPE(tx_blk->pPacket, TX_ALTX);
		mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
		tr_cnt->pkt_invalid_wcid++;

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
			"pkt_t(%d),type(%d),sub_type(%d),q_idx(%d,%d),cnt(%d)\n",
			pkt_type_old, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType,
			q_idx_old, mac_info.q_idx, tr_cnt->pkt_invalid_wcid);

		if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION) {
			PFRAME_ACTION_HDR Frame = (PFRAME_ACTION_HDR)pHeader_802_11;
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				"cat(%d),act(%d)\n",
				Frame->Category, Frame->Action);
		}
	}

	asic_mgmt_emlsr_update(pAd, &mac_info, tx_blk);
	ret = asic_mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
	if (ret != NDIS_STATUS_SUCCESS)
		tr_cnt->tx_hwifi_err_drop += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);

	return ret;
}

INT ap_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk)
{
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	struct _QUEUE_ENTRY *q_entry = tx_blk->TxPacketList.Head;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

	if (!fill_tx_blk(pAd, wdev, tx_blk)) {
		ap_tx_drop_update(pAd, wdev, tx_blk);
		ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS)) {
		wdev_ops->ieee_802_3_data_tx(pAd, wdev, tx_blk);
	} else {
		do {
			tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
			wdev_ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);
			q_entry = QUEUE_GET_NEXT_ENTRY(q_entry);
		} while (q_entry);
	}

	ap_tx_ok_update(pAd, wdev, tx_blk);
	ret = asic_hw_tx(pAd, tx_blk);

	if (ret != NDIS_STATUS_SUCCESS) {
		tr_cnt->tx_hwifi_err_drop += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

VOID ap_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	BOOLEAN bVLANPkt;
#ifdef TXBF_SUPPORT
	struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
#endif

	ap_build_802_11_header(pAd, pTxBlk);
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif /* SOFT_ENCRYPT || SW_CONNECT_SUPPORT */
	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;
	/* skip vlan tag */
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	/* record these MCAST_TX frames for group key rekey */
	if (pTxBlk->TxFrameType == TX_BMC_FRAME) {
		INT idx;
#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
#ifndef RT_CFG80211_SUPPORT
			if (pAd->ApCfg.MBSSID[idx].WPAREKEY.ReKeyMethod == PKT_REKEY)
				pAd->ApCfg.MBSSID[idx].REKEYCOUNTER += (pTxBlk->SrcBufLen);
#endif /* RT_CFG80211_SUPPORT */
		}
	}

#ifdef MT_MAC
	else {
		/* Unicast */
		if (pTxBlk->tr_entry && pTxBlk->tr_entry->PsDeQWaitCnt)
			pTxBlk->Pid = PID_PS_DATA;
	}

#endif /* MT_MAC */
	pHeaderBufPtr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;
	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		struct wifi_dev *wdev_wmm = NULL;
		UCHAR ack_policy = 0;

		if (pTxBlk->QueIdx < WMM_NUM_OF_AC)
			ack_policy = pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx];

		wdev_wmm = pTxBlk->wdev;
		if (wdev_wmm) {
			if (pTxBlk->QueIdx < WMM_NUM_OF_AC)
				ack_policy = wlan_config_get_ack_policy(wdev_wmm, pTxBlk->QueIdx);
		}
		/* build QOS Control bytes */
		if (wdev_wmm || (pTxBlk->QueIdx < WMM_NUM_OF_AC))
			*pHeaderBufPtr = ((pTxBlk->UserPriority & 0x0F) | (ack_policy << 5));
		else
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR,
				"ack_policy is not set!\n");

#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		   ) {
			/*
				we can not use bMoreData bit to get EOSP bit because
				maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}

#endif /* UAPSD_SUPPORT */
		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->wifi_hdr_len += 2;


#ifdef TXBF_SUPPORT
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_SOUND, DBG_LVL_DEBUG, "tx_bf: %d\n",
							 cap->FlgHwTxBfCap);
#endif /* TXBF_SUPPORT */
	}

	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
	else
#endif /* SOFT_ENCRYPT || SW_CONNECT_SUPPORT */
	{
		/*
			Insert LLC-SNAP encapsulation - 8 octets
			if original Ethernet frame contains no LLC/SNAP,
			then an extra LLC/SNAP encap is required
		*/
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
										   pTxBlk->pExtraLlcSnapEncap);

		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr,
						   pTxBlk->pSrcBufHeader + 12 + vlan_size,
						   2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}
}

NDIS_STATUS ap_group_frame_sn(
	RTMP_ADAPTER * pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt,
	UINT8 *mld,
	UINT8 *agent_bss,
	struct wifi_dev **wdev_list,
	PNDIS_PACKET *pkt_list)
{
	NDIS_STATUS sts = NDIS_STATUS_SUCCESS;
#ifdef DOT11_EHT_BE
	UINT16 sn = 0;
#endif /* DOT11_EHT_BE */

#ifdef DOT11_EHT_BE
	if (!pAd || !wdev || !pkt_list) {
		sts = NDIS_STATUS_INVALID_DATA;
		goto err;
	}

	if (!mld || !agent_bss || !wdev_list) {
		sts = NDIS_STATUS_INVALID_DATA;
		goto err;
	}

	sts = bss_mngr_mld_bmc_sn_qry(wdev, mld, agent_bss, &sn, wdev_list);
	if (sts)
		goto err;

	if (*mld && *agent_bss) {
		UINT8 i = 0;
		struct wifi_dev *target_wdev = NULL;
		PNDIS_PACKET target_pkt = NULL;

		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			target_wdev = wdev_list[i];
			if (target_wdev == NULL)
				continue;
			target_pkt = (target_wdev == wdev) ? pkt :
				DuplicatePacket(target_wdev->if_dev, pkt);
			if (target_pkt) {
				RTMP_SET_PACKET_SN(target_pkt, sn);
				RTMP_SET_PACKET_SN_VLD(target_pkt, 1);
				RTMP_SET_PACKET_WDEV(target_pkt, target_wdev->wdev_idx);
				pkt_list[i] = target_pkt;
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BMC_SN, DBG_LVL_DEBUG,
		"wdev(%s), mld=%d, agent_bss=%d, sn=%d\n",
		wdev->if_dev->name, *mld, *agent_bss, sn);
	return sts;
err:
#endif /* DOT11_EHT_BE */
	return sts;
}

VOID ap_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *pTxBlk)
{
	pTxBlk->MpduHeaderLen = 0;
	pTxBlk->HdrPadLen = 0;
	pTxBlk->wifi_hdr_len = 0;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	pTxBlk->dot11_type = FC_TYPE_DATA;
	pTxBlk->dot11_subtype = TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM) ?
		SUBTYPE_QDATA : SUBTYPE_DATA;
}

INT ap_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

	if (!fill_tx_blk(pAd, wdev, tx_blk)) {
		ap_tx_drop_update(pAd, wdev, tx_blk);
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS))
		wdev_ops->ieee_802_3_data_tx(pAd, wdev, tx_blk);
	else {
		struct _QUEUE_ENTRY *pPktEntry;
		NDIS_STATUS status;
		PNDIS_PACKET pkt;

		wdev_ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);

		status = RTMPAllocateNdisPacket(pAd, &pkt,
			tx_blk->wifi_hdr, tx_blk->MpduHeaderLen + tx_blk->HdrPadLen,
			tx_blk->pSrcBufData, tx_blk->SrcBufLen);

		if (status != NDIS_STATUS_SUCCESS) {
			ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
			return status;
		}

		NdisCopyMemory(GET_OS_PKT_CB(pkt),  GET_OS_PKT_CB(tx_blk->pPacket),
			sizeof(GET_OS_PKT_CB(tx_blk->pPacket)));

		RTMP_SET_PACKET_WDEV(pkt, RTMP_GET_PACKET_WDEV(tx_blk->pPacket));
		RTMP_SET_PACKET_WCID(pkt, tx_blk->Wcid);

		pPktEntry = RemoveHeadQueue(&tx_blk->TxPacketList);
		RELEASE_NDIS_PACKET(pAd, ((void *)pPktEntry), NDIS_STATUS_PKT_REQUEUE);

		InsertTailQueue(&tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));
		tx_blk->pPacket = pkt;
		tx_blk->pSrcBufData = GET_OS_PKT_DATAPTR(pkt);
		tx_blk->SrcBufLen = GET_OS_PKT_LEN(pkt);
		tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
	}

	ap_tx_ok_update(pAd, wdev, tx_blk);

	ret = asic_hw_tx(pAd, tx_blk);

	if (ret != NDIS_STATUS_SUCCESS) {
		tr_cnt->tx_hwifi_err_drop += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
		return ret;
	}

#ifdef IXIA_C50_MODE
	if (IS_EXPECTED_LENGTH(pAd, GET_OS_PKT_LEN(tx_blk->pPacket)))
		pAd->tx_cnt.tx_pkt_to_hw[TX_LEGACY][smp_processor_id()]++;
#endif

	return NDIS_STATUS_SUCCESS;

}

INT ap_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk)
{
	HEADER_802_11 *wifi_hdr;
#ifdef SOFT_ENCRYPT
	UCHAR *tmp_ptr = NULL;
	UINT32 buf_offset = 0;
#endif /* SOFT_ENCRYPT */
	union _HTTRANSMIT_SETTING *pTransmit;
	UCHAR fragNum = 0;
	USHORT EncryptionOverhead = 0;
	UINT32 FreeMpduSize, SrcRemainingBytes;
	USHORT AckDuration;
	UINT NextMpduSize;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

#ifdef SW_CONNECT_SUPPORT
	if (IS_SW_MAIN_STA(tx_blk->tr_entry) || IS_SW_STA(tx_blk->tr_entry)) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_FRAG_TX, DBG_LVL_NOTICE,
				"drop !!! S/W STA not support frag !!! wdev->wdev_idx=%d\n", wdev->wdev_idx);
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}
#endif /* SW_CONNECT_SUPPORT */

	if (!fill_tx_blk(pAd, wdev, tx_blk)) {
		ap_tx_drop_update(pAd, wdev, tx_blk);
		ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_FRAG_TX, DBG_LVL_INFO,
			"<--(%d): ##########Fail#########\n", __LINE__);
		return NDIS_STATUS_FAILURE;
	}

#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, tx_blk) == FALSE) {
			ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
	}

#endif /* SOFT_ENCRYPT */

	if (IS_CIPHER_TKIP(tx_blk->CipherAlg)) {
		tx_blk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, tx_blk->pPacket);

		if (tx_blk->pPacket == NULL)
			return NDIS_STATUS_FAILURE;

		tx_blk->pSrcBufHeader = RTMP_GET_PKT_SRC_VA(tx_blk->pPacket);
		tx_blk->SrcBufLen = RTMP_GET_PKT_LEN(tx_blk->pPacket);
	}

	ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);

	/*  1. If TKIP is used and fragmentation is required. Driver has to
		   append TKIP MIC at tail of the scatter buffer
		2. When TXWI->FRAG is set as 1 in TKIP mode,
		   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC */
	/*  TKIP appends the computed MIC to the MSDU data prior to fragmentation into MPDUs. */
	if (IS_CIPHER_TKIP(tx_blk->CipherAlg)) {
		RTMPCalculateMICValue(pAd, tx_blk->pPacket, tx_blk->pExtraLlcSnapEncap, tx_blk->pKey->Key, &tx_blk->pKey->Key[LEN_TK], wdev->func_idx);
		/*
			NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
				to correct lenght, refer to tx_blk->SrcBufLen for the packet length in following progress.
		*/
		NdisMoveMemory(tx_blk->pSrcBufData + tx_blk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		tx_blk->SrcBufLen += 8;
		tx_blk->TotalFrameLen += 8;
	}
	ap_tx_ok_update(pAd, wdev, tx_blk);

	/*
		calcuate the overhead bytes that encryption algorithm may add. This
		affects the calculate of "duration" field
	*/
	if ((tx_blk->CipherAlg == CIPHER_WEP64) || (tx_blk->CipherAlg == CIPHER_WEP128) || (tx_blk->CipherAlg == CIPHER_WEP152))
		EncryptionOverhead = 8; /* WEP: IV[4] + ICV[4]; */
	else if (tx_blk->CipherAlg == CIPHER_TKIP)
		EncryptionOverhead = 12; /* TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength */
	else if (tx_blk->CipherAlg == CIPHER_AES)
		EncryptionOverhead = 16;	/* AES: IV[4] + EIV[4] + MIC[8] */

	else
		EncryptionOverhead = 0;

	pTransmit = tx_blk->pTransmit;
	/* Decide the TX rate */
	/* TODO: Add new rate index for frag tx */
	if (pTransmit->field.MODE == MODE_CCK)
		tx_blk->fr_tbl_idx = FR_CCK_1M;
	else if (pTransmit->field.MODE == MODE_OFDM)
		tx_blk->fr_tbl_idx = FR_OFDM_6M;
	else
		tx_blk->fr_tbl_idx = FR_OFDM_6M;

	/* decide how much time an ACK/CTS frame will consume in the air */
	if (tx_blk->fr_tbl_idx == FR_CCK_1M)
		AckDuration = RTMPCalcDuration(pAd, RATE_1, 14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6, 14);

#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
		/* store the outgoing frame for calculating MIC per fragmented frame */
		os_alloc_mem(pAd, (PUCHAR *)&tmp_ptr, tx_blk->SrcBufLen);

		if (tmp_ptr == NULL) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_FRAG_TX, DBG_LVL_ERROR,
				"no memory for MIC calculation!\n");
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}

		NdisMoveMemory(tmp_ptr, tx_blk->pSrcBufData, tx_blk->SrcBufLen);
	}

#endif /* SOFT_ENCRYPT */
	/* Init the total payload length of this frame. */
	SrcRemainingBytes = tx_blk->SrcBufLen;
	tx_blk->TotalFragNum = 0xff;
	wifi_hdr = (HEADER_802_11 *)tx_blk->wifi_hdr;

	do {
		INT dot11_meta_hdr_len, tx_hw_hdr_len;
		PNDIS_PACKET pkt;
		NDIS_STATUS status;
		UCHAR *dot11_hdr;
		struct _TX_BLK new_tx_blk, *p_new_tx_blk = &new_tx_blk;
		struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);

		FreeMpduSize = wlan_operate_get_frag_thld(wdev);
		FreeMpduSize -= LENGTH_CRC;
		FreeMpduSize -= tx_blk->MpduHeaderLen;

		if (SrcRemainingBytes <= FreeMpduSize) {
			/* This is the last or only fragment */
			tx_blk->SrcBufLen = SrcRemainingBytes;
			wifi_hdr->FC.MoreFrag = 0;
			wifi_hdr->Duration = pAd->CommonCfg.Dsifs + AckDuration;
			/* Indicate the lower layer that this's the last fragment. */
			tx_blk->TotalFragNum = fragNum;
			tx_blk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_NO : TX_FRAG_ID_LAST);
		} else {
			/* more fragment is required */
			tx_blk->SrcBufLen = FreeMpduSize;
			NextMpduSize = min(((UINT)SrcRemainingBytes - tx_blk->SrcBufLen),
							   ((UINT)wlan_operate_get_frag_thld(wdev)));
			wifi_hdr->FC.MoreFrag = 1;
			wifi_hdr->Duration = (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) +
						RTMPCalcDuration(pAd, tx_blk->TxRate,
							NextMpduSize + EncryptionOverhead);
			tx_blk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_FIRST : TX_FRAG_ID_MIDDLE);
		}

		SrcRemainingBytes -= tx_blk->SrcBufLen;

#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
			UCHAR ext_offset = 0;

			NdisMoveMemory(tx_blk->pSrcBufData, tmp_ptr + buf_offset, tx_blk->SrcBufLen);
			buf_offset += tx_blk->SrcBufLen;
			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd,
									 tx_blk->CipherAlg,
									 (UCHAR *)wifi_hdr,
									 tx_blk->pSrcBufData,
									 tx_blk->SrcBufLen,
									 tx_blk->KeyIdx,
									 tx_blk->pKey,
									 &ext_offset);
			tx_blk->SrcBufLen += ext_offset;
			tx_blk->TotalFrameLen += ext_offset;
		}

#endif /* SOFT_ENCRYPT */

		NdisCopyMemory(p_new_tx_blk, tx_blk, sizeof(*tx_blk));
		InitializeQueueHeader(&p_new_tx_blk->TxPacketList);
		dot11_meta_hdr_len = tx_blk->MpduHeaderLen + tx_blk->HdrPadLen;
		tx_hw_hdr_len = cap->tx_hw_hdr_len;
		dot11_hdr = &tx_blk->HeaderBuf[tx_hw_hdr_len];
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_FRAG_TX, DBG_LVL_INFO,
			"DataFrm, MpduHdrL=%d,WFHdrL=%d,HdrPadL=%d, NeedCopyHdrLen=%d\n",
			tx_blk->MpduHeaderLen, tx_blk->wifi_hdr_len, tx_blk->HdrPadLen,
			dot11_meta_hdr_len);
		/*
		 * original packet only 802.3 payload,
		 * so create a new packet including 802.11 header for cut-through transfer
		 *
		 */
		status = RTMPAllocateNdisPacket(pAd, &pkt,
			dot11_hdr, dot11_meta_hdr_len,
			tx_blk->pSrcBufData, tx_blk->SrcBufLen);

		if (status != NDIS_STATUS_SUCCESS) {
			ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
			return status;
		}

		NdisCopyMemory(GET_OS_PKT_CB(pkt),  GET_OS_PKT_CB(tx_blk->pPacket),
			sizeof(GET_OS_PKT_CB(tx_blk->pPacket)));

		RTMP_SET_PACKET_WDEV(pkt, RTMP_GET_PACKET_WDEV(tx_blk->pPacket));
		RTMP_SET_PACKET_WCID(pkt, RTMP_GET_PACKET_WCID(tx_blk->pPacket));

		InsertTailQueue(&p_new_tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));
		p_new_tx_blk->pPacket = pkt;
		p_new_tx_blk->pMacEntry = entry_get(pAd, tx_blk->Wcid);
		p_new_tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		p_new_tx_blk->tr_entry = tx_blk->tr_entry;
		p_new_tx_blk->Wcid = tx_blk->Wcid;
		p_new_tx_blk->TxFrameType = tx_blk->TxFrameType;
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_FRAG_TX, DBG_LVL_INFO,
			"Wcid(%d), TxFrameType(%d), TxPacketList.Number = %d, wifi_hdr_len = %d\n",
			p_new_tx_blk->Wcid,
			p_new_tx_blk->TxFrameType,
			p_new_tx_blk->TxPacketList.Number,
			p_new_tx_blk->wifi_hdr_len);
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_FRAG_TX, DBG_LVL_INFO,
			"DataFrm, FragIdx=%d\n",
			tx_blk->FragIdx);
		p_new_tx_blk->pSrcBufData = GET_OS_PKT_DATAPTR(pkt);
		p_new_tx_blk->SrcBufLen = GET_OS_PKT_LEN(pkt);
		TX_BLK_SET_FLAG(p_new_tx_blk, fTX_CT_WithTxD);
		TX_BLK_SET_FLAG(p_new_tx_blk, fTX_ForceRate);
#ifdef MT7990
		chip_do_extra_action(pAd, wdev, p_new_tx_blk->pMacEntry->Addr,
			CHIP_EXTRA_ACTION_SW_PN, (UCHAR *)p_new_tx_blk, NULL);
#endif /* MT7990 */
		ret = asic_hw_tx(pAd, p_new_tx_blk);

		if (ret != NDIS_STATUS_SUCCESS) {
#ifdef SOFT_ENCRYPT
			if (tmp_ptr != NULL)
				os_free_mem(tmp_ptr);
#endif /* SOFT_ENCRYPT */
			tr_cnt->tx_hwifi_err_drop += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
			ap_tx_packet_list_free(pAd, p_new_tx_blk, NDIS_STATUS_FAILURE);
			return ret;
		}

#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_bSwEncrypt)) {
				if ((tx_blk->CipherAlg == CIPHER_WEP64) || (tx_blk->CipherAlg == CIPHER_WEP128)) {
					inc_iv_byte(tx_blk->pKey->TxTsc, LEN_WEP_TSC, 1);
					/* Construct and insert 4-bytes WEP IV header to MPDU header */
					RTMPConstructWEPIVHdr(tx_blk->KeyIdx, tx_blk->pKey->TxTsc,
										  pHeaderBufPtr - (LEN_WEP_IV_HDR));
				} else if (tx_blk->CipherAlg == CIPHER_TKIP)
					;
				else if (tx_blk->CipherAlg == CIPHER_AES) {
					inc_iv_byte(tx_blk->pKey->TxTsc, LEN_WPA_TSC, 1);
					/* Construct and insert 8-bytes CCMP header to MPDU header */
					RTMPConstructCCMPHdr(tx_blk->KeyIdx, tx_blk->pKey->TxTsc,
										 pHeaderBufPtr - (LEN_CCMP_HDR));
				}
		} else
#endif /* SOFT_ENCRYPT */
		{
			/* Update the frame number, remaining size of the NDIS packet payload. */
			if (fragNum == 0 && tx_blk->pExtraLlcSnapEncap)
				tx_blk->MpduHeaderLen -= LENGTH_802_1_H;	/* space for 802.11 header. */
		}

		fragNum++;

#ifdef TXRX_STAT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.TxFragmentationCount);
#endif /* TXRX_STAT_SUPPORT */

		/* SrcRemainingBytes -= tx_blk->SrcBufLen; */
		tx_blk->pSrcBufData += tx_blk->SrcBufLen;
		wifi_hdr->Frag++;	/* increase Frag # */
	} while (SrcRemainingBytes > 0);

	ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
#ifdef SOFT_ENCRYPT

	if (tmp_ptr != NULL)
		os_free_mem(tmp_ptr);

#endif /* SOFT_ENCRYPT */
	return NDIS_STATUS_SUCCESS;
}

INT ap_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk)
{
	struct wifi_dev_ops *ops = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct DOT11_H *pDot11h = NULL;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;
	UINT16 wcid = 0;
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY * tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */

#ifdef CONFIG_6G_AFC_SUPPORT
	if (!afc_check_tx_enable(pAd, wdev)) {
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}
#endif /*CONFIG_6G_AFC_SUPPORT*/

	if (!wdev) {
		tr_cnt->tx_invalid_wdev += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	ops = wdev->wdev_ops;
	pDot11h = wdev->pDot11_H;

	if (pDot11h == NULL) {
		tr_cnt->tx_invalid_wdev += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (pDot11h->ChannelMode != CHAN_NORMAL_MODE) {
		struct _RTMP_CHIP_CAP *cap = PD_GET_CHIP_CAP_PTR(pAd->physical_dev);
		UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
		UCHAR *pData = GET_OS_PKT_DATAPTR(tx_blk->pPacket);
		HEADER_802_11 *pHeader_802_11 = (HEADER_802_11 *)(pData + tx_hw_hdr_len);
		UINT8 channel_tx_allow = FALSE;

#ifdef DOT11_EHT_BE
		if (pDot11h->ChannelMode == CHAN_SILENCE_MODE) {
			/* During 5G CAC period */
			pEntry = tx_blk->pMacEntry;
			if (IS_ENTRY_MLO(pEntry) &&
				((tx_blk->TxFrameType == TX_LEGACY_FRAME) ||
				(tx_blk->TxFrameType == TX_AMSDU_FRAME) ||
				(tx_blk->TxFrameType == TX_BMC_FRAME))) {
				/* Data MLO case */
				struct mld_entry_t *mld_entry;

				mt_rcu_read_lock();
				mld_entry = rcu_dereference(pEntry->mld_entry);
				if (mld_entry) {
					u8 link_id = 0;
					MAC_TABLE_ENTRY *entry_ptr = NULL;

					for (link_id = 0; link_id < MLD_LINK_MAX; link_id++) {
						entry_ptr = mld_entry->link_entry[link_id];

						if (!entry_ptr || wdev == entry_ptr->wdev)
							continue;

						if (!entry_ptr->wdev || !entry_ptr->wdev->pDot11_H)
							continue;

						pDot11h = entry_ptr->wdev->pDot11_H;
						if (pDot11h->ChannelMode == CHAN_NORMAL_MODE) {
							channel_tx_allow = TRUE;
							tr_cnt->silence_in_mlo_allow++;
							break;
						}
					}
				}
				mt_rcu_read_unlock();
			}
		}
#endif

		if (tx_blk->TxFrameType == TX_MLME_MGMTQ_FRAME &&
			pHeader_802_11->FC.SubType == SUBTYPE_ACTION
			&& pHeader_802_11->Octet[0] == CATEGORY_SPECTRUM && pHeader_802_11->Octet[1] == SPEC_CHANNEL_SWITCH)
			channel_tx_allow = TRUE;

#ifdef ZERO_PKT_LOSS_SUPPORT
		/*drop frame only if zero pkt loss not enabled*/
		if (!(pAd->Zero_Loss_Enable)) {
#endif /*ZERO_PKT_LOSS_SUPPORT*/
			if (channel_tx_allow == FALSE) {
				tr_cnt->carrier_detect_drop += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_CNT, DBG_LVL_DEBUG, "<--(%d)\n", __LINE__);
				return NDIS_STATUS_FAILURE;
			}
#ifdef ZERO_PKT_LOSS_SUPPORT
		}
#endif /*ZERO_PKT_LOSS_SUPPORT*/
	}
#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
	if (IS_RRM_QUIET(wdev)) {
		tr_cnt->tx_rrm_quiet_drop += ap_tx_packet_list_free(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

	switch (tx_blk->TxFrameType) {
	case TX_LEGACY_FRAME:
		ret = ops->legacy_tx(pAd, wdev, tx_blk);
		break;

	case TX_BMC_FRAME:
		ret = ops->legacy_tx(pAd, wdev, tx_blk);
		break;

	case TX_AMSDU_FRAME:
		ret = ops->amsdu_tx(pAd, wdev, tx_blk);
		break;

	case TX_FRAG_FRAME:
		ret = ops->frag_tx(pAd, wdev, tx_blk);
		break;

	case TX_MLME_MGMTQ_FRAME:
		ret = ops->mlme_mgmtq_tx(pAd, wdev, tx_blk);
		break;

	case TX_MLME_DATAQ_FRAME:
		ret = ops->mlme_dataq_tx(pAd, wdev, tx_blk);
		break;

#ifdef VERIFICATION_MODE
	case TX_VERIFY_FRAME:
		ret = ops->verify_tx(pAd, wdev, tx_blk);
		break;
#endif

	default:
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_CNT, DBG_LVL_ERROR,
			"Send a pacekt was not classified!!\n");
		tr_cnt->tx_unknow_type_drop += ap_tx_packet_list_free(pAd, tx_blk, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (ret != NDIS_STATUS_SUCCESS)
		goto end;

	/* wcid of tx_blk should be checked already when packets dequeued */
	wcid = tx_blk->Wcid;

	/*check for valid pEntry before accessing, to avoid null kernel panic*/
	if (IS_WCID_VALID(pAd, wcid)) {
		pEntry = &pAd->MacTab->Content[wcid];
#ifdef SW_CONNECT_SUPPORT
		if (IS_ENTRY_CLIENT(pEntry)) {
			tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
			tr_entry->tx_handle_cnt++;
		}
#endif /* SW_CONNECT_SUPPORT */

#ifdef ANTENNA_DIVERSITY_SUPPORT
		pEntry->ant_div_tx_bytes += tx_blk->SrcBufLen;
#endif
	}


end:
	return ret;
}

/*
    ==========================================================================
    Description:
	Some STA/AP
    Note:
	This action should never trigger AUTH state transition, therefore we
	separate it from AUTH state machine, and make it as a standalone service
    ==========================================================================
 */
VOID ap_cls2_err_action(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk)
{
	HEADER_802_11 Hdr;
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Reason = REASON_CLS2ERR;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR apidx;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = entry_get(pAd, pRxBlk->wcid);

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		/*ApLogEvent(pAd, pAddr, EVENT_DISASSOCIATED); */
#ifdef WIFI_DIAG
		diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_CLS2ERR);
#endif
		mac_entry_delete(pAd, pEntry, TRUE);
	} else {
		apidx = get_apidx_by_addr(pAd, pRxBlk->Addr1);

		if (apidx >= pAd->ApCfg.BssidNum) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO, "AUTH - Class 2 error but not my bssid "MACSTR"\n", MAC2STR(pRxBlk->Addr1));
			return;
		}
	}

	/* send out DEAUTH frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_AUTH, DBG_LVL_INFO,
			 "AUTH - Class 2 error, Send DEAUTH frame to "MACSTR"\n",
			  MAC2STR(pRxBlk->Addr2));
	MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pRxBlk->Addr2,
					 pRxBlk->Addr1,
					 pRxBlk->Addr1);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &Hdr,
					  2, &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
}


/*
    ==========================================================================
    Description:
	right part of IEEE 802.11/1999 page 374
    Note:
	This event should never cause ASSOC state machine perform state
	transition, and has no relationship with CNTL machine. So we separate
	this routine as a service outside of ASSOC state transition table.
    ==========================================================================
 */
VOID ap_cls3_err_action(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk)
{
	HEADER_802_11         DisassocHdr;
	PUCHAR                pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	USHORT                Reason = REASON_CLS3ERR;
	MAC_TABLE_ENTRY       *pEntry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = entry_get(pAd, pRxBlk->wcid);

	if (pEntry) {
#ifdef WIFI_DIAG
		if (IS_ENTRY_CLIENT(pEntry))
			diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif
		/*ApLogEvent(pAd, pAddr, EVENT_DISASSOCIATED); */
		mac_entry_delete(pAd, pEntry, TRUE);
	}

	/* 2. send out a DISASSOC request frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_ASSOC, DBG_LVL_INFO, "ASSOC - Class 3 Error, Send DISASSOC frame to "MACSTR"\n",
			 MAC2STR(pRxBlk->Addr2));
	MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pRxBlk->Addr2,
					 pRxBlk->Addr1,
					 pRxBlk->Addr1);
	MakeOutgoingFrame(pOutBuffer,            &FrameLen,
					  sizeof(HEADER_802_11), &DisassocHdr,
					  2,                     &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen, NULL);
	MlmeFreeMemory(pOutBuffer);
}

/*
  ========================================================================
  Description:
	This routine checks if a received frame causes class 2 or class 3
	error, and perform error action (DEAUTH or DISASSOC) accordingly
  ========================================================================
*/
BOOLEAN ap_chk_cl2_cl3_err(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry;

	/* software MAC table might be smaller than ASIC on-chip total size. */
	/* If no mathed wcid index in ASIC on chip, do we need more check???  need to check again. 06-06-2006 */
	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {

		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

		if (pEntry)
			return FALSE;

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
				 "Rx a frame from "MACSTR" with WCID(%d) > %d\n",
				 MAC2STR(pRxBlk->Addr2),
				 pRxBlk->wcid, GET_MAX_UCAST_NUM(pAd));
		ap_cls2_err_action(pAd, pRxBlk);
		return TRUE;
	}

	pEntry = entry_get(pAd, pRxBlk->wcid);
	if (pEntry->Sst == SST_ASSOC)
		/* okay to receive this DATA frame */
		return FALSE;
	else if (pEntry->Sst == SST_AUTH) {
		ap_cls3_err_action(pAd, pRxBlk);
		return TRUE;
	}
	ap_cls2_err_action(pAd, pRxBlk);
	return TRUE;
}

BOOLEAN ap_check_valid_frame(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN isVaild = FALSE;

	do {
		if (FC->ToDs == 0)
			break;

#ifdef IDS_SUPPORT

		if ((FC->FrDs == 0) && (pRxBlk->wcid == WCID_NO_MATCHED(pAd))) { /* not in RX WCID MAC table */
			if (++pAd->ApCfg.RcvdMaliciousDataCount > pAd->ApCfg.DataFloodThreshold)
				break;
		}

#endif /* IDS_SUPPORT */

		/* check if Class2 or 3 error */
		if ((FC->FrDs == 0) && (ap_chk_cl2_cl3_err(pAd, pRxBlk)))
			break;

		if (pAd->ApCfg.BANClass3Data == TRUE)
			break;

		isVaild = TRUE;
	} while (0);

	return isVaild;
}

#ifdef TXRX_STAT_SUPPORT
static VOID ap_rx_update_replay_attack_cnt(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk)
{
	INT i;
	struct _SECURITY_CONFIG *pSecConfig = &pAd->StaCfg[0].wdev.SecConfig;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		/* Conflict SSID detection */
		if (NdisEqualMemory(pRxBlk->Addr2, pAd->ApCfg.MBSSID[i].wdev.bssid, MAC_ADDR_LEN)) {
			if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher))
				INC_COUNTER64(pAd->WlanCounters.RxTkipReplayCount);
			else if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher))
				INC_COUNTER64(pAd->WlanCounters.RxCcmpReplayCount);
			break;
		}
	}
}
#endif /* TXRX_STAT_SUPPORT */

INT ap_rx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *pRxBlk)
{
#ifdef A4_CONN
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT)
	struct _RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
#endif
#endif
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _STA_TR_ENTRY *tr_entry;
	INT hdr_len = 0;

	pEntry = PACInquiry(pAd, pRxBlk->wcid);
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(A4_CONN)

	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
#ifdef CLIENT_WDS

		if (pEntry) {
			/* The CLIENT WDS must be a associated STA */
			if (IS_ENTRY_CLIWDS(pEntry))
				;
			else if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
				SET_ENTRY_CLIWDS(pEntry);
			else
				return FALSE;

			CliWds_ProxyTabUpdate(pAd, pEntry->wcid, pRxBlk->Addr4);
		}

#endif /* CLIENT_WDS */

#ifdef A4_CONN
		if (!pEntry)
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

		if (pEntry && (IS_ENTRY_A4(pEntry)
#ifdef MWDS
			|| ((pFmeCtrl->SubType & 0x08) && RTMPEqualMemory(EAPOL, pRxBlk->pData+LENGTH_802_11_WITH_ADDR4+LENGTH_WMMQOS_H+6, 2))
#endif
		) ) {
			MAC_TABLE_ENTRY *pMovedEntry = NULL;
			UINT16 ProtoType = 0;
			UINT32 ARPSenderIP = 0;
			UCHAR *Pos = (pRxBlk->pData + 12);
			BOOLEAN bTAMatchSA = MAC_ADDR_EQUAL(pEntry->Addr, pRxBlk->Addr4);

#ifdef DOT11_EHT_BE
			if (pEntry->mlo.mlo_en && !bTAMatchSA)
				bTAMatchSA = mld_entry_match_to_source_addr(pEntry, pRxBlk->Addr4);
#endif
#ifdef MWDS
			pEntry->MWDSInfo.Addr4PktNum++;
#endif
			ProtoType = OS_NTOHS(*((UINT16 *)Pos));
			if (ProtoType == 0x0806) /* ETH_P_ARP */
				NdisCopyMemory(&ARPSenderIP, (Pos + 16), 4);

			/*
			   It means this source entry has moved to another one and hidden behind it.
			   So delete this source entry!
			*/
			if (!bTAMatchSA) { /* TA isn't same with SA case*/
				pMovedEntry = MacTableLookup(pAd, pRxBlk->Addr4);
				if (pMovedEntry
#ifdef AIR_MONITOR
					&& !IS_ENTRY_MONITOR(pMovedEntry)
#endif /* AIR_MONITOR */
					&& IS_ENTRY_CLIENT(pMovedEntry)
					&& !IS_ENTRY_A4(pMovedEntry)
			) {
#ifdef WH_EVENT_NOTIFIER
				{
					EventHdlr pEventHdlrHook = NULL;

					pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
					if (pEventHdlrHook && pMovedEntry->wdev)
						pEventHdlrHook(pAd, pMovedEntry->wdev, pMovedEntry->Addr, pMovedEntry->wdev->channel);
				}
#endif /* WH_EVENT_NOTIFIER */

#ifdef CONFIG_MAP_SUPPORT
					if (!IS_MAP_ENABLE(pAd) || (IS_MAP_ENABLE(pAd) && pMovedEntry->MAP_Enqueue_single == 0)) {
						pMovedEntry->MAP_Enqueue_single = 1;
#endif
#ifdef MAP_R2
					if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd))
						wapp_handle_sta_disassoc(pAd, pMovedEntry->wcid, REASON_DEAUTH_STA_LEAVING);
#endif
					entry_del_hw_ppe_entry(pAd, pMovedEntry->Addr);

					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_DEBUG,
						 "AP found a entry(%02X:%02X:%02X:%02X:%02X:%02X) moved to another dev! Delete it from MAC table.\n",
							PRINT_MAC(pMovedEntry->Addr));
#ifdef DOT11_EHT_BE
					if (pMovedEntry->mlo.mlo_en)
						mld_update_roaming_entry(pAd, pMovedEntry, pEntry);
					else
						pMovedEntry->roaming_entry = TRUE;
#else
					pMovedEntry->roaming_entry = TRUE;
#endif
					mac_entry_delete(pAd, pMovedEntry, TRUE);
#ifdef CONFIG_MAP_SUPPORT
				}
#endif
			}
		}
			a4_proxy_update(pAd, pEntry->func_tb_idx, pEntry->wcid, pRxBlk->Addr4, ARPSenderIP);
		} else {
			if (pEntry != NULL) {
#if defined(CONFIG_BS_SUPPORT) || defined(CONFIG_MAP_SUPPORT)
				if (!IS_MAP_ENABLE(pAd)
#if defined(CONFIG_MAP_SUPPORT)
					|| ((pEntry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)) == 0)
#endif
				)
#endif
#ifdef MWDS
				if (!pEntry->bSupportMWDS)
#endif
				{
#if defined(MBSS_AS_WDS_AP_SUPPORT) || defined(APCLI_AS_WDS_STA_SUPPORT)
					if (!(pEntry->wdev && pEntry->wdev->wds_enable))
#endif
						pEntry = NULL;
				}
			}
		}
#endif /* A4_CONN */

#ifdef WDS_SUPPORT

		if (!pEntry) {
			struct wifi_dev *main_bss_wdev = NULL;

			main_bss_wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
			/*
				The WDS frame only can go here when in auto learning mode and
				this is the first trigger frame from peer

				So we check if this is un-registered WDS entry by call function
					"FindWdsEntry()"
			*/
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG,
				 "[band%d]RA:"MACSTR", TA:"MACSTR"\n",
				  pRxBlk->band, MAC2STR(pRxBlk->Addr1), MAC2STR(pRxBlk->Addr2));
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG,
				 "\tA3:"MACSTR", A4:"MACSTR"\n",
				  MAC2STR(pRxBlk->Addr3), MAC2STR(pRxBlk->Addr4));

			if (MAC_ADDR_EQUAL(pRxBlk->Addr1, main_bss_wdev->if_addr))
				pEntry = FindWdsEntry(pAd, pRxBlk);
			else {
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_INFO,
					 "[band%d]WDS for RA "MACSTR" is not enabled!\n", pRxBlk->band,
					  MAC2STR(pRxBlk->Addr1));
				return FALSE;
			}

			/* have no valid wds entry exist, then discard the incoming packet.*/
			if (!(pEntry && WDS_IF_UP_CHECK(pAd, pRxBlk->band, pEntry->func_tb_idx))) {
				if (!pEntry)
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG,
						 "[band%d]WDS dropped due to entry for "MACSTR" not found!\n",
						 pRxBlk->band, MAC2STR(pRxBlk->Addr2));
				else
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG,
						 "[band%d]WDS dropped due to Entry[%d] not enabled!\n",
						 pRxBlk->band, pEntry->func_tb_idx);
				return FALSE;
			}

			/*receive corresponding WDS packet, disable TX lock state (fix WDS jam issue) */
			if (pEntry && (pEntry->LockEntryTx == TRUE)) {
				tr_entry = tr_entry_get(pAd, pEntry->wcid);
				MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_INFO, "[band%d]Receive WDS packet, disable TX lock state!\n", pRxBlk->band);
				pEntry->ContinueTxFailCnt = 0;
				pEntry->LockEntryTx = FALSE;
				tr_entry->ContinueTxFailCnt = 0;
				tr_entry->LockEntryTx = FALSE;
			}
		} else if (!IS_ENTRY_WDS(pEntry)) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_INFO, "[band%d]Receive 4-addr packet, but not from a WDS entry!\n", pRxBlk->band);
			/*return FALSE;*/
		}

#endif /* WDS_SUPPORT */

#ifndef WDS_SUPPORT
		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_INFO, "[band%d]WDS packet dropped due to entry not valid!\n", pRxBlk->band);
			return FALSE;
		}
#endif /* WDS_SUPPORT */

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
		if (IS_ENTRY_WDS(pEntry)) {
			RT_802_11_WDS_ENTRY *pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

			pWdsEntry->WdsCounter.ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			INC_COUNTER64(pWdsEntry->WdsCounter.ReceivedFragmentCount);

			if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
				INC_COUNTER64(pWdsEntry->WdsCounter.MulticastReceivedFrameCount);
		}
#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */
		RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		hdr_len = LENGTH_802_11_WITH_ADDR4;
		return hdr_len;

	}

#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

	if (!pEntry) {
#ifdef IDS_SUPPORT

		if ((pFmeCtrl->FrDs == 0) && (pRxBlk->wcid == WCID_NO_MATCHED(pAd))) /* not in RX WCID MAC table */
			pAd->ApCfg.RcvdMaliciousDataCount++;

#endif /* IDS_SUPPORT */
		return FALSE;
	}

	if (!((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
#ifdef IDS_SUPPORT

		/*
			Replay attack detection,
			drop it if detect a spoofed data frame from a rogue AP
		*/
		if (pFmeCtrl->FrDs == 1)
			RTMPReplayAttackDetection(pAd, pRxBlk->Addr2, pRxBlk);

#endif /* IDS_SUPPORT */
#ifdef TXRX_STAT_SUPPORT
		if (pFmeCtrl->FrDs == 1)
			ap_rx_update_replay_attack_cnt(pAd, pRxBlk);
#endif /* TXRX_STAT_SUPPORT */
		return FALSE;
	}

#ifdef A4_CONN
	if (((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
#ifdef MWDS
		if (pEntry && GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
			pEntry->MWDSInfo.Addr3PktNum++;
			if ((pFmeCtrl->SubType == SUBTYPE_DATA_NULL) || (pFmeCtrl->SubType == SUBTYPE_QOS_NULL))
				pEntry->MWDSInfo.NullPktNum++;
			if (pRxInfo->Mcast || pRxInfo->Bcast)
				pEntry->MWDSInfo.bcPktNum++;
		}
#endif
		if ((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL)) {
#ifdef MWDS
			if (pEntry && GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
				return FALSE;
			}
#endif
#ifdef CONFIG_MAP_SUPPORT
			/* do not receive 3-address broadcast/multicast packet, */
			/* because the broadcast/multicast packet woulld be send using 4-address, */
			/* 1905 message is an exception, need to receive 3-address 1905 multicast, */
			/* because some vendor send only one 3-address 1905 multicast packet */
			/* 1905 daemon would filter and drop duplicate packet */
			if (GET_ENTRY_A4(pEntry) == A4_TYPE_MAP &&
				(pRxInfo->Mcast || pRxInfo->Bcast) &&
				(memcmp(pRxBlk->Addr1, multicast_mac_1905, MAC_ADDR_LEN) != 0))
				return FALSE;
#endif
		}
	}
#endif /* A4_CONN */

	/* check if Class2 or 3 error */
	if (ap_chk_cl2_cl3_err(pAd, pRxBlk))
#ifdef TXRX_STAT_SUPPORT
	{
		pAd->tr_ctl.tr_cnt.rx_cl2cl3_err_cnt++;
		return FALSE;
	}
#else
		return FALSE;
#endif /* TXRX_STAT_SUPPORT */

	if (pAd->ApCfg.BANClass3Data == TRUE)
		return FALSE;

#ifdef STATS_COUNT_SUPPORT

	/* Increase received counter per BSS will add in UNI_EVENT_ALL_STA_TRX_MSDU_COUNT
	   and UNI_EVENT_ALL_STA_TXRX_ADM_STAT event by MacTableMaintenance, so remove here*/


	if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
		INC_COUNTER64(pAd->WlanCounters.MulticastReceivedFrameCount);

#endif /* STATS_COUNT_SUPPORT */

#ifdef WH_EVENT_NOTIFIER
	if (pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
		&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
		&& ((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL))
	)
		pEntry->rx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */

#ifdef RADIUS_MAC_AUTH_SUPPORT
	if (pEntry->wdev->radius_mac_auth_enable) {
		if (!pEntry->bAllowTraffic)
			return FALSE;
	}
#endif

	hdr_len = LENGTH_802_11;
	RX_BLK_SET_FLAG(pRxBlk, fRX_STA);
	ASSERT(pEntry->wcid == pRxBlk->wcid);
	return hdr_len;
}

INT ap_rx_pkt_foward(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	MAC_TABLE_ENTRY *pSrcEntry = NULL;
	BOOLEAN to_os, to_air;
	UCHAR *pHeader802_3;
	PNDIS_PACKET pForwardPacket;
	BSS_STRUCT *pMbss = NULL;
	struct wifi_dev *dst_wdev = NULL;
	UINT16 wcid;
	PHEADER_802_3 pHDR = NULL;

	WLAN_HOOK_CALL(WLAN_HOOK_RX_DRIVER_FORWARD, pAd, pPacket);

	if (!VALID_MBSS(pAd, wdev->func_idx)) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_ERROR,
			"Invalid func_idx(%d), type(%d)!\n",
			wdev->func_idx, wdev->wdev_type);
		return FALSE;
	}

	/* TODO: shiang-usw, remove pMbss structure here to make it more generic! */
	if ((wdev->func_idx >= 0) && (wdev->func_idx < MAX_BEACON_NUM))
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	pHeader802_3 = GET_OS_PKT_DATAPTR(pPacket);
	pHDR = (PHEADER_802_3)pHeader802_3;
	/* by default, announce this pkt to upper layer (bridge) and not to air */
	to_os = TRUE;
	to_air = FALSE;

	if (pMbss && (pHeader802_3[0] & 0x01)) {
		if (pMbss->StaCount > 1
#ifdef DOT11_EHT_BE
			|| bss_mngr_is_wdev_in_mlo_group(wdev)
#endif /* DOT11_EHT_BE */
			) {
			/* forward the M/Bcast packet back to air if connected STA > 1 */
			to_air = TRUE;
		}

		/* LLDP pkt handled by os */
		if (isLLDP(pHeader802_3)) {
			to_air = FALSE;
		}
	} else {
		/* if destinated STA is a associated wireless STA */
		pEntry = MacTableLookupForTx(pAd, pHeader802_3, NULL);

		if (pEntry && pEntry->Sst == SST_ASSOC && pEntry->wdev) {
			dst_wdev = pEntry->wdev;

			if (wdev == dst_wdev) {
				/*
					STAs in same SSID, default send to air and not to os,
					but not to air if match following case:
						a). pMbss->IsolateInterStaTraffic == TRUE
				*/
				to_air = FALSE;
				to_os = TRUE;

				if (pMbss && (pMbss->IsolateInterStaTraffic == 1)
#ifdef HOSTAPD_HS_R2_SUPPORT
					&& !(pMbss->HotSpotCtrl.HotSpotEnable
						&& pMbss->WNMCtrl.ProxyARPEnable)
#endif /* HOSTAPD_HS_R2_SUPPORT */
				   )
					to_air = FALSE;
			} else {
				/*
					STAs in different SSID, default send to os and not to air
					but not to os if match any of following cases:
						a). destination VLAN ID != source VLAN ID
						b). pAd->ApCfg.IsolateInterStaTrafficBTNBSSID
				*/
				to_os = TRUE;
				to_air = FALSE;

				if (pAd->ApCfg.IsolateInterStaTrafficBTNBSSID == 1)
					to_os = FALSE;
			}
#ifdef WH_EVENT_NOTIFIER
			if (to_air && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
			)
				pEntry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */
				/* Check Source STA is in PortSecured then do FWD packet */
				pSrcEntry = MacTableLookupForTx(pAd, pHDR->SAAddr2, wdev);
				if (pSrcEntry) {
					STA_TR_ENTRY *tr_entry = NULL;

					if (IS_WCID_VALID(pAd, pSrcEntry->tr_tb_idx))
						tr_entry = tr_entry_get(pAd, pSrcEntry->tr_tb_idx);

					if (tr_entry && (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
						MTWF_DBG(pAd, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_ERROR,
							"Not PortSecured Pkt FWD to STAs from wcid(%d)"
							"to wcid(%d)!\n", pSrcEntry->wcid, pEntry->wcid);
						to_os = FALSE;
						to_air = FALSE;
					}
				}
			}
#ifdef A4_CONN
		else if ((((!pEntry)
#ifdef AIR_MONITOR
			|| (pEntry && IS_ENTRY_MONITOR(pEntry))
#endif
		 ) &&a4_proxy_lookup(pAd, wdev->func_idx, pHeader802_3, FALSE, TRUE, (UINT16 *)&wcid))) {
			if (IS_WCID_VALID(pAd, wcid))
				pEntry = entry_get(pAd, wcid);

			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				UINT16 tem_wcid;

				to_os = FALSE;
				to_air = TRUE;
				dst_wdev = pEntry->wdev;

				tem_wcid = RTMP_GET_PACKET_WCID(pPacket);
				if (tem_wcid == wcid)
					to_air = FALSE;
			}
		}
#endif /* A4_CONN */

#ifdef MBSS_AS_WDS_AP_SUPPORT
#ifdef CLIENT_WDS
		else if ((!pEntry)
#ifdef AIR_MONITOR
			|| (pEntry && IS_ENTRY_MONITOR(pEntry))
#endif
			) {
			PUCHAR pEntryAddr = CliWds_ProxyLookup(pAd, pHeader802_3);
			if ((pEntryAddr != NULL)
				&& (!MAC_ADDR_EQUAL(pEntryAddr, pHeader802_3 + 6))) {
				pEntry = MacTableLookupForTx(pAd, pEntryAddr, NULL);
				if (pEntry && (pEntry->Sst == SST_ASSOC) && pEntry->wdev && (!pEntry->wdev->bVLAN_Tag)) {
					to_os = FALSE;
					to_air = TRUE;
					dst_wdev = pEntry->wdev;
					if ((wdev == dst_wdev) && pMbss && (pMbss->IsolateInterStaTraffic == 1)) {
					/*
						STAs in same SSID, default send to air and not to os,
						but not to air if match following case:
						a). pMbss->IsolateInterStaTraffic == TRUE
					*/
						to_air = FALSE;
						to_os = FALSE;
					} else if (wdev == dst_wdev) {
					/*
						STAs in same SSID, default send to air and not to os
					*/
						to_air = TRUE;
						to_os = FALSE;
					} else {
					/*
						STAs in different SSID, default send to os and not to air
						but not to os if match any of following cases:
						a). destination VLAN ID != source VLAN ID
						b). pAd->ApCfg.IsolateInterStaTrafficBTNBSSID
					*/
						to_os = TRUE;
						to_air = FALSE;
					if (pAd->ApCfg.IsolateInterStaTrafficBTNBSSID == 1)
						to_os = FALSE;
				}
			}
		}
	}
#endif /* CLIENT_WDS */
#endif

	}

	if (to_air) {
#ifdef MAP_TS_TRAFFIC_SUPPORT
		if (pAd->bTSEnable)
			pForwardPacket = CopyPacket(wdev->if_dev, pPacket);
		else
#endif
		pForwardPacket = DuplicatePacket(wdev->if_dev, pPacket);

		if (pForwardPacket == NULL)
			return to_os;

#if defined(RT_CFG80211_SUPPORT)
		/*when forward pkt, need to clear skb CB buffer, nor some CB flag will be set*/
		os_zero_mem((PUCHAR)&(RTPKT_TO_OSPKT(pForwardPacket)->cb[CB_OFF]), CB_LEN);
#endif

		/* 1.1 apidx != 0, then we need set packet mbssid attribute. */
		if (pEntry) {
			wcid = pEntry->wcid;
			RTMP_SET_PACKET_WDEV(pForwardPacket, dst_wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		} else { /* send bc/mc frame back to the same bss */
			wcid = wdev->tr_tb_idx;
			RTMP_SET_PACKET_WDEV(pForwardPacket, wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		}

		RTMP_SET_PACKET_MOREDATA(pForwardPacket, FALSE);

#ifdef REDUCE_TCP_ACK_SUPPORT
		ReduceAckUpdateDataCnx(pAd, pForwardPacket);

		if (ReduceTcpAck(pAd, pForwardPacket) == FALSE)
#endif
		{
#ifndef A4_CONN
			send_data_pkt(pAd, wdev, pForwardPacket);
#else
			RTMP_SET_PACKET_A4_FWDDATA(pForwardPacket, TRUE);

			/* send bc/mc frame back to the same bss */
			if (pHeader802_3[0] & 0x01)
				a4_send_clone_pkt(pAd, wdev->func_idx, pForwardPacket, pHeader802_3 + MAC_ADDR_LEN);

			send_data_pkt(pAd, wdev, pForwardPacket);	/* rakesh: recheck */
#endif /* A4_CONN */
		}
	}

	return to_os;
}

INT ap_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	struct _RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	UCHAR wdev_idx = BSS0;
	BOOLEAN bFragment = FALSE;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	struct wifi_dev_ops *ops = wdev->wdev_ops;
#ifdef TXRX_STAT_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif
	struct _STA_TR_ENTRY *tr_entry;

	wdev_idx = wdev->wdev_idx;
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_3D, DBG_LVL_DEBUG,
			"wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, "
			"fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			pEntry->wcid, wdev->wdev_idx,
			pRxBlk->Flags,
			RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
			RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
			RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
			pFmeCtrl->Type, pFmeCtrl->SubType,
			pFmeCtrl->FrDs, pFmeCtrl->ToDs);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if (IS_SUPPORT_V10_DFS(pAd) && (((FRAME_CONTROL *)pRxBlk->FC)->Type == FC_TYPE_DATA))
		pEntry->LastRxTimeCount = 0;
#endif

	pEntry->NoDataIdleCount = 0;
	if (IS_WCID_VALID(pAd, pEntry->wcid)) {
		tr_entry = tr_entry_get(pAd, pEntry->wcid);
		tr_entry->NoDataIdleCount = 0;
	}
	if (!IS_MT7990(pAd) &&
		!IS_MT7992(pAd) &&
		!IS_MT7993(pAd)) {
		pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
#ifdef CONFIG_MAP_SUPPORT
		if (IS_MAP_ENABLE(pAd))
			pEntry->RxBytesMAP += pRxBlk->MPDUtotalByteCnt;
#endif
		pEntry->OneSecRxBytes += pRxBlk->MPDUtotalByteCnt;
#ifdef ANTENNA_DIVERSITY_SUPPORT
		pEntry->ant_div_rx_bytes += pRxBlk->MPDUtotalByteCnt;
#endif
		INC_COUNTER64(pEntry->RxPackets);
#ifdef RX_COUNT_DETECT
		pEntry->one_sec_rx_pkts++;
#endif /* RX_COUNT_DETECT */
	}

#ifndef WIFI_UNIFIED_COMMAND
	pAd->WlanCounters.RxTotByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
	pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;

#ifdef TXRX_STAT_SUPPORT
#ifdef ANDLINK_FEATURE_SUPPORT
	if ((pEntry != NULL) && (pEntry->Sst == SST_ASSOC) &&
			(IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)))
#else
	if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry))
#endif
	{
		struct _BSS_STRUCT *mbss = MBSS_GET(pEntry->pMbss);
		INC_COUNTER64(pEntry->RxUnicastPktCount);
		pEntry->RxUnicastByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
		INC_COUNTER64(pEntry->RxDataPacketCount);
		INC_COUNTER64(ctrl->rdev.pRadioCtrl->RxDataPacketCount);
		pEntry->RxDataPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
		if (mbss) {
			INC_COUNTER64(mbss->stat_bss.RxUnicastDataPacket);
			INC_COUNTER64(mbss->stat_bss.RxDataPacketCount);
			mbss->stat_bss.RxDataPayloadByte.QuadPart += (pRxBlk->MPDUtotalByteCnt - 14);
			mbss->stat_bss.RxDataPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
		}
		ctrl->rdev.pRadioCtrl->RxDataPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
			INC_COUNTER64(pEntry->RxDataPacketCountPerAC[WMM_UP2AC_MAP[pRxBlk->UserPriority]]);
			INC_COUNTER64(ctrl->rdev.pRadioCtrl->RxDataPacketCountPerAC[WMM_UP2AC_MAP[pRxBlk->UserPriority]]);
			if (mbss) {
				INC_COUNTER64(
					mbss->stat_bss.RxDataPacketCountPerAC[
						WMM_UP2AC_MAP[pRxBlk->UserPriority]]);
			}
		}
		{
			int ant_idx;
			for (ant_idx = 0; ant_idx < 4; ant_idx++) {
				pEntry->LastDataPktRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), ant_idx);
				ctrl->rdev.pRadioCtrl->LastDataPktRssi[ant_idx] = pEntry->LastDataPktRssi[ant_idx];
			}
		}
	}

#endif /* TXRX_STAT_SUPPORT */
#if defined(CUSTOMER_RSG_FEATURE) || defined(CUSTOMER_DCC_FEATURE) || defined(MAP_R2)
	if (pFmeCtrl->FrDs == 0 && pRxInfo->U2M && wdev->wdev_idx < pAd->ApCfg.BssidNum) {
#if defined(CUSTOMER_DCC_FEATURE) || defined(MAP_R2)
		UCHAR *pDA = pRxBlk->Addr3;
		BSS_STRUCT *pMbss = NULL;

		if ((wdev->wdev_idx >= 0) && (wdev->wdev_idx < MAX_BEACON_NUM))
			pMbss = &pAd->ApCfg.MBSSID[wdev->wdev_idx];

		if (!pMbss)
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_3D, DBG_LVL_ERROR,
				"pMbss is NULL!\n");

		if (((*pDA) & 0x1) == 0x01) {
			if (IS_BROADCAST_MAC_ADDR(pDA)) {
				if(pMbss) {
					pMbss->bcPktsRx++;
#ifdef MAP_R6
				if (pEntry->mlo.mlo_en)
					pMbss->mlo_bcBytesRx += pRxBlk->MPDUtotalByteCnt;
				else
					pMbss->bcBytesRx += pRxBlk->MPDUtotalByteCnt;
#else
				pMbss->bcBytesRx += pRxBlk->MPDUtotalByteCnt;
#endif
				}
#ifdef TR181_SUPPORT
				pAd->WlanCounters.bcPktsRx.QuadPart++;
				pAd->WlanCounters.bcBytesRx.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
			} else {
				if(pMbss) {
					pMbss->mcPktsRx++;
#ifdef MAP_R6
				if (pEntry->mlo.mlo_en)
					pMbss->mlo_bcBytesRx += pRxBlk->MPDUtotalByteCnt;
				else
					pMbss->bcBytesRx += pRxBlk->MPDUtotalByteCnt;
#else
				pMbss->bcBytesRx += pRxBlk->MPDUtotalByteCnt;
#endif
				}
#ifdef TR181_SUPPORT
				pAd->WlanCounters.mcPktsRx.QuadPart++;
				pAd->WlanCounters.mcBytesRx.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
			}
		}
#ifdef CUSTOMER_DCC_FEATURE
		pEntry->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
		pEntry->RxCount++;

		if (pRxBlk->rx_signal.raw_snr[0])
			Update_Snr_Sample(pAd, pEntry, &pRxBlk->rx_signal);
#endif
#endif
	}
#endif


	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x08) {

		if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid) ||
		    pRxBlk->TID >= NUM_OF_TID || !pRxInfo->U2M) {
			pRxInfo->BA = 0;
		} else {
			MAC_TABLE_ENTRY *pEntry = entry_get(pAd, pRxBlk->wcid);
			USHORT Idx = pEntry->ba_info.RecWcidArray[pRxBlk->TID];

#ifdef DOT11_EHT_BE
			if (IS_ENTRY_MLO(pEntry)) {
				struct mld_entry_t *mld_entry;

				mt_rcu_read_lock();
				mld_entry = rcu_dereference(pEntry->mld_entry);
				if (!mld_entry) {
					mt_rcu_read_unlock();
					return FALSE;
				}
				Idx = mld_entry->ba_info.RecWcidArray[pRxBlk->TID];
				mt_rcu_read_unlock();
			}
#endif /* DOT11_EHT_BE */

			pRxInfo->BA = Idx ? 1 : 0;
		}

		if (pRxBlk->AmsduState)
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);

		if (pRxInfo->BA)
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}

	/*check if duplicate frame, ignore it and then drop*/
	if (rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_3D, DBG_LVL_INFO, "duplicate frame drop it!\n");
		return FALSE;
	}

	if (rx_chk_amsdu_invalid_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_3D, DBG_LVL_INFO,
			"invalid amsdu frame drop it!\n");
		return FALSE;
	}
	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x04) { /* bit 2 : no DATA */
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_3D, DBG_LVL_DEBUG, "Null/QosNull frame!\n");
#ifdef TXRX_STAT_SUPPORT
		if (pEntry && pFmeCtrl->Type == FC_TYPE_DATA) {
			if (pFmeCtrl->SubType == SUBTYPE_DATA_NULL)
				INC_COUNTER64(pAd->WlanCounters.RxDataNullCnt);
			else if (pFmeCtrl->SubType == SUBTYPE_QOS_NULL)
				INC_COUNTER64(pAd->WlanCounters.RxQosNullCnt);
		}
#endif /* TXRX_STAT_SUPPORT */

		return FALSE;
	}

	if ((pRxBlk->FN == 0) && (pFmeCtrl->MoreFrag != 0)) {
		bFragment = TRUE;
		de_fragment_data_pkt(pAd, pRxBlk);
	}

	if (pRxInfo->U2M)
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#ifdef IGMP_SNOOP_SUPPORT
	if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
		&& (wdev->IgmpSnoopEnable)
		&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3)) {
		PUCHAR pDA = pRxBlk->Addr3;
		PUCHAR pSA = pRxBlk->Addr2;
		PUCHAR pData = pRxBlk->pData + 12;
		PUCHAR pDataEnd = pRxBlk->pData + pRxBlk->DataSize;
		UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));
#ifdef MAP_TS_TRAFFIC_SUPPORT
		/*when TS enable, for igmp need skip vlan header before handle*/
		if (pAd->bTSEnable) {
			if (RTMPEqualMemory(pData, TPID, 2)) {
				pData += LENGTH_802_1Q;
				protoType = OS_NTOHS(*((UINT16 *)(pData)));
			}
		}
#endif
		if (protoType == ETH_P_IP)
			IGMPSnooping(pAd, pDA, pSA, pData, pDataEnd, pEntry, pRxBlk->wcid);
		else if (protoType == ETH_P_IPV6)
			MLDSnooping(pAd, pDA, pSA,  pData, pDataEnd, pEntry, pRxBlk->wcid);
	}

#ifdef IGMP_TVM_SUPPORT
		/* Convert Unicast Rx packet from AP to Multicast */
		if (IS_ENTRY_APCLI(pEntry)
			&& !IS_BM_MAC_ADDR(pRxBlk->Addr1)
			&& IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			ConvertUnicastMacToMulticast(pAd, wdev, pRxBlk);
		}
#endif /* IGMP_TVM_SUPPORT */

#endif /* IGMP_SNOOP_SUPPORT */

	if (0
#ifdef QOS_R1
		|| IS_QOS_ENABLE(pAd)
#endif
#ifdef MAP_R2
		|| IS_MAP_R2_ENABLE(pAd)
#endif
#ifdef MAP_R3
		|| IS_MAP_R3_ENABLE(pAd)
#endif
	) {
		if (pRxBlk->pRxPacket) {
			if (pRxBlk->UserPriority < 8) {
				RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, pRxBlk->UserPriority);
				RTMP_SET_PACKET_UP_CB33(pRxBlk->pRxPacket, pRxBlk->UserPriority);
			} else {
				RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, 0);
				RTMP_SET_PACKET_UP_CB33(pRxBlk->pRxPacket, 0);
			}
		}
	}

	if (pRxBlk->pRxPacket) {
		RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);
		rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, pEntry->wdev);
	}

	ops->detect_wmm_traffic(pAd, wdev, pRxBlk->UserPriority, FLAG_IS_INPUT);

	return TRUE;
}

INT ap_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	struct _RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN bFragment = FALSE;
	UCHAR wdev_idx = BSS0;
	UCHAR UserPriority = 0;
	INT hdr_len = LENGTH_802_11;
	COUNTER_RALINK *pCounter = &pAd->RalinkCounters;
	UCHAR *pData;
	BOOLEAN drop_err = TRUE;
#if defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT)
	PCIPHER_KEY pPairwiseKey = NULL;
#ifdef SW_CONNECT_SUPPORT
#ifdef CONFIG_LINUX_CRYPTO
	struct crypto_aead *tfm = NULL;
#endif /* CONFIG_LINUX_CRYPTO */
#endif /* SW_CONNECT_SUPPORT */
	UINT8 KeyId = 0;
	NDIS_STATUS status;
#endif /* defined(SOFT_ENCRYPT) || defined(SW_CONNECT_SUPPORT) */
	struct wifi_dev_ops *ops = wdev->wdev_ops;
	STA_TR_ENTRY *tr_entry  = NULL;
	struct tr_counter *tr_cnt = &pAd->tr_ctl.tr_cnt;

	wdev_idx = wdev->wdev_idx;
	MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_11D, DBG_LVL_INFO,
			 "wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			 pEntry->wcid, wdev->wdev_idx,
			 pRxBlk->Flags,
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
			 pFmeCtrl->Type, pFmeCtrl->SubType,
			 pFmeCtrl->FrDs, pFmeCtrl->ToDs);
	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
	/* must be here, before no DATA check */
	pData = pRxBlk->FC;

	pEntry->NoDataIdleCount = 0;
	if (IS_WCID_VALID(pAd, pEntry->wcid)) {
		tr_entry = tr_entry_get(pAd, pEntry->wcid);
		tr_entry->NoDataIdleCount = 0;
	}

	/*
		update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
	*/
#if	defined(A4_CONN) || defined(APCLI_AS_WDS_STA_SUPPORT) || defined(MBSS_AS_WDS_AP_SUPPORT) || defined(WDS_SUPPORT)
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	pData = pRxBlk->FC;
	/* 1. skip 802.11 HEADER */
	if (pRxBlk->DataSize > hdr_len) {
		pData += hdr_len;
		pRxBlk->DataSize -= hdr_len;
	} else {
#ifdef TXRX_STAT_SUPPORT
		if (pFmeCtrl->SubType & 0x04)
			INC_COUNTER64(pAd->WlanCounters.RxDataNullCnt);
		else
			tr_cnt->rx_invalid_small_pkt_drop++;
#endif /* TXRX_STAT_SUPPORT */
		return FALSE;
	}

	/* 2. QOS */
	if (pFmeCtrl->SubType & 0x08) {
		UserPriority = *(pData) & 0x0f;

		if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid) ||
		    pRxBlk->TID >= NUM_OF_TID || !pRxInfo->U2M) {
			pRxInfo->BA = 0;
		} else {
			MAC_TABLE_ENTRY *pEntry = entry_get(pAd, pRxBlk->wcid);
			USHORT Idx = pEntry->ba_info.RecWcidArray[pRxBlk->TID];

#ifdef DOT11_EHT_BE
			if (IS_ENTRY_MLO(pEntry)) {
				struct mld_entry_t *mld_entry;

				mt_rcu_read_lock();
				mld_entry = rcu_dereference(pEntry->mld_entry);
				if (!mld_entry) {
					mt_rcu_read_unlock();
					return FALSE;
				}
				Idx = mld_entry->ba_info.RecWcidArray[pRxBlk->TID];
				mt_rcu_read_unlock();
			}
#endif /* DOT11_EHT_BE */

			pRxInfo->BA = Idx ? 1 : 0;
		}

		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pData) & 0x80) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
			pCounter->RxAMSDUCount.u.LowPart++;
		}

		if (pRxInfo->BA) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
			/* incremented by the number of MPDUs */
			/* received in the A-MPDU when an A-MPDU is received. */
			pCounter->MPDUInReceivedAMPDUCount.u.LowPart++;
		}

		/* skip QOS contorl field */
		RX_BLK_SET_FLAG(pRxBlk, fRX_QOS);
		if (pRxBlk->DataSize > 2) {
			pData += 2;
			pRxBlk->DataSize -= 2;
		} else {
#ifdef TXRX_STAT_SUPPORT
			if (pFmeCtrl->SubType & 0x04)
				INC_COUNTER64(pAd->WlanCounters.RxQosNullCnt);
			else
				tr_cnt->rx_invalid_small_pkt_drop++;
#endif /* TXRX_STAT_SUPPORT */
			return FALSE;
		}
	}

	pRxBlk->UserPriority = UserPriority;

	if (0
#ifdef QOS_R1
		|| IS_QOS_ENABLE(pAd)
#endif
#ifdef MAP_R2
		|| IS_MAP_R2_ENABLE(pAd)
#endif
#ifdef MAP_R3
		|| IS_MAP_R3_ENABLE(pAd)
#endif
	) {
		if (pRxBlk->pRxPacket) {
			if  (UserPriority < 8) {
				RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, UserPriority);
				RTMP_SET_PACKET_UP_CB33(pRxBlk->pRxPacket, UserPriority);
			} else {
				RTMP_SET_PACKET_UP(pRxBlk->pRxPacket, 0);
				RTMP_SET_PACKET_UP_CB33(pRxBlk->pRxPacket, 0);
			}
		}
	}

	/*check if duplicate frame, ignore it and then drop*/
	if (rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_11D, DBG_LVL_INFO,
			" duplicate frame drop it!\n");
		return FALSE;
	}

	if (rx_chk_amsdu_invalid_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_11D, DBG_LVL_INFO,
			"invalid amsdu frame drop it!\n");
		return FALSE;
	}

	/* 3. Order bit: A-Ralink or HTC+ */
	if (pFmeCtrl->Order) {
		/* skip HTC control field */
		if (pRxBlk->DataSize > 4) {
			pData += 4;
			pRxBlk->DataSize -= 4;
		} else {
#ifdef TXRX_STAT_SUPPORT
			if (pFmeCtrl->SubType & 0x04)
				INC_COUNTER64(pAd->WlanCounters.RxQosNullCnt);
			else
				tr_cnt->rx_invalid_small_pkt_drop++;
#endif /* TXRX_STAT_SUPPORT */
			return FALSE;
		}
	}

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (pFmeCtrl->SubType & 0x04) { /* bit 2 : no DATA */
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_802_11D, DBG_LVL_DEBUG, "Null/QosNull frame!\n");
		drop_err = FALSE;
#ifdef TXRX_STAT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.RxDataNullCnt);
#endif /* TXRX_STAT_SUPPORT */
		return FALSE;
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD) {
		/* just move pData pointer because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		if (pRxBlk->DataSize > 2)
			pData += 2;
		else {
			tr_cnt->rx_invalid_small_pkt_drop++;
			return FALSE;
		}
	}

	if (pRxBlk->AmsduState) {
		struct _RXD_BASE_STRUCT *rx_base = NULL;

		rx_base = (struct _RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		/* skip 2 bytes HW padding */
		if (pRxBlk->DataSize > 2) {
			pData += 2;
			pRxBlk->DataSize -= 2;
		} else {
			tr_cnt->rx_invalid_small_pkt_drop++;
			return FALSE;
		}

		if (pRxBlk->DataSize > LENGTH_802_3) {
			pData += LENGTH_802_3;
			pRxBlk->DataSize -= LENGTH_802_3;
		} else {
			tr_cnt->rx_invalid_small_pkt_drop++;
			return FALSE;
		}
	}

	pRxBlk->pData = pData;
#if defined(SOFT_ENCRYPT) ||  defined(SW_CONNECT_SUPPORT)
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1)
	   and it's passed to driver with "Decrypted" marked as 0 in RxInfo.
	*/
	if (
#ifdef SW_CONNECT_SUPPORT
		(pEntry && tr_entry && IS_SW_STA(tr_entry)) ||
#endif /* SW_CONNECT_SUPPORT */
		!IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((pFmeCtrl->Wep == 1) && (pRxInfo->Decrypted == 0)) {
#if defined(SW_CONNECT_SUPPORT)
#ifdef SW_CONNECT_SUPPORT
#ifdef CONFIG_LINUX_CRYPTO
			tfm = pEntry->SecConfig.tfm;
#endif /* CONFIG_LINUX_CRYPTO */
#endif /* SW_CONNECT_SUPPORT */
			if (IS_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher)) {
				pPairwiseKey = &pEntry->SecConfig.SwPairwiseKey;
			} else if (IS_CIPHER_TKIP(pEntry->SecConfig.PairwiseCipher)) {
				pPairwiseKey = &pEntry->SecConfig.SwPairwiseKey;
			} else if (IS_CIPHER_WEP40(pEntry->SecConfig.PairwiseCipher)) {
				if (pRxBlk->key_idx < SEC_KEY_NUM)
					KeyId = pRxBlk->key_idx;
				pPairwiseKey = (PCIPHER_KEY)&pEntry->SecConfig.WepKey[KeyId];
				/* non-bitwise */
				pPairwiseKey->CipherAlg = CIPHER_WEP64;
			} else if (IS_CIPHER_WEP104(pEntry->SecConfig.PairwiseCipher)) {
				if (pRxBlk->key_idx < SEC_KEY_NUM)
					KeyId = pRxBlk->key_idx;
				pPairwiseKey = (PCIPHER_KEY)&pEntry->SecConfig.WepKey[KeyId];
				/* non-bitwise */
				pPairwiseKey->CipherAlg = CIPHER_WEP128;
			} else {
				ASSERT(0);
				return FALSE;
			}
#endif /* SW_CONNECT_SUPPORT */

#ifdef HDR_TRANS_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
				status = RTMPSoftDecryptionAction(pAd,
												  pRxBlk->FC,
												  UserPriority,
												  pPairwiseKey,
#ifdef CONFIG_STA_SUPPORT
												  wdev_idx,
#endif /* CONFIG_STA_SUPPORT */
												  pRxBlk->pTransData + 14,
												  &(pRxBlk->TransDataSize));
			} else
#endif /* HDR_TRANS_SUPPORT */
			{
#ifdef SW_CONNECT_SUPPORT
#ifdef CONFIG_LINUX_CRYPTO
				if (tfm && pPairwiseKey && IS_CIPHER_CCMP128(pPairwiseKey->CipherAlg)) {
					status = ccmp_decrypt(tfm, pRxBlk, pPairwiseKey->RxTsc, pRxBlk->FC);

					if (status == NDIS_STATUS_SUCCESS) {
						/* skip CCMP hdr */
						if (pRxBlk->DataSize > (LEN_CCMP_HDR+LEN_CCMP_MIC)) {
							pRxBlk->pData = pData + LEN_CCMP_HDR;
							/* skip CCMP hdr & MIC LEN */
							pRxBlk->DataSize = pRxBlk->DataSize - LEN_CCMP_HDR - LEN_CCMP_MIC;
						} else {
							tr_cnt->rx_invalid_small_pkt_drop++;
							return FALSE;
						}
					}

				} else
#endif /* CONFIG_LINUX_CRYPTO */
#endif /* SW_CONNECT_SUPPORT */
				{
					status = RTMPSoftDecryptionAction(pAd,
													  pRxBlk->FC,
													  UserPriority,
													  pPairwiseKey,
#ifdef CONFIG_STA_SUPPORT
													  wdev_idx,
#endif /* CONFIG_STA_SUPPORT */
													  pRxBlk->pData,
													  &(pRxBlk->DataSize));
				}
			}

			if (status != NDIS_STATUS_SUCCESS) {
#ifdef SW_CONNECT_SUPPORT
				if (tr_entry && IS_SW_STA(tr_entry))
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_SEC_RX, DBG_LVL_ERROR, "wcid =%d,decryp fail !!\n", pEntry->wcid);
#endif /* SW_CONNECT_SUPPORT */
				/* Fix pkt Double Free */
				return FALSE;
			} else {
#ifdef SW_CONNECT_SUPPORT
				if (tr_entry && IS_SW_STA(tr_entry)) {
					MTWF_DBG(pAd, DBG_CAT_RX, CATRX_SEC_RX, DBG_LVL_INFO, "wcid =%d, S/W decryp OK !!, pRxBlk->DataSize=%u\n", pEntry->wcid, pRxBlk->DataSize);
					RX_BLK_CLEAR_FLAG(pRxBlk, fRX_CM);
				}
#endif /* SW_CONNECT_SUPPORT */
			}

			/* Record the Decrypted bit as 1 */
			pRxInfo->Decrypted = 1;
		}
	}

#endif /* SOFT_ENCRYPT */

#ifdef SW_CONNECT_SUPPORT
	if (tr_entry)
		tr_entry->rx_handle_cnt++;
#endif /* SW_CONNECT_SUPPORT */

	if (pRxInfo->U2M) {
#ifdef SW_CONNECT_SUPPORT
		if (tr_entry)
			tr_entry->rx_u2m_cnt++;
#endif /* SW_CONNECT_SUPPORT */

		pAd->ApCfg.NumOfAvgRssiSample++;
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);
#ifdef TXBF_SUPPORT

		if (pRxBlk->rx_rate.field.ShortGI)
			pEntry->OneSecRxSGICount++;
		else
			pEntry->OneSecRxLGICount++;

#endif /* TXBF_SUPPORT */
#ifdef DBG_DIAGNOSE

		if ((pAd->DiagStruct.inited) && (pAd->DiagStruct.ArrayCurIdx >= 0) && (pAd->DiagStruct.ArrayCurIdx < DIAGNOSE_TIME)) {
			struct dbg_diag_info *diag_info;
			diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
			diag_info->RxDataCnt++;
		}

#endif /* DBG_DIAGNOSE */
	}

	wdev->LastSNR0 = (UCHAR)(pRxBlk->rx_signal.raw_snr[0]);
	wdev->LastSNR1 = (UCHAR)(pRxBlk->rx_signal.raw_snr[1]);
	pEntry->freqOffset = (CHAR)(pRxBlk->rx_signal.freq_offset);
	pEntry->freqOffsetValid = TRUE;

	if ((pRxBlk->FN != 0) || (pFmeCtrl->MoreFrag != 0)) {
		bFragment = TRUE;
		de_fragment_data_pkt(pAd, pRxBlk);
		pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	}

	if (pRxBlk->pRxPacket) {
		/*
			process complete frame which encrypted by TKIP,
			Minus MIC length and calculate the MIC value
		*/
		if (bFragment && (pFmeCtrl->Wep) && IS_CIPHER_TKIP_Entry(pEntry)) {
			if (pRxBlk->DataSize > 8)
				pRxBlk->DataSize -= 8;
			else {
				tr_cnt->rx_invalid_small_pkt_drop++;
				return FALSE;
			}

			if (rtmp_chk_tkip_mic(pAd, pEntry, pRxBlk) == FALSE)
				return TRUE;
		}

		if (!IS_MT7990(pAd) &&
			!IS_MT7992(pAd) &&
			!IS_MT7993(pAd)) {
			pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
#ifdef CONFIG_MAP_SUPPORT
			if (IS_MAP_ENABLE(pAd))
				pEntry->RxBytesMAP += pRxBlk->MPDUtotalByteCnt;
#endif
			INC_COUNTER64(pEntry->RxPackets);
#ifdef RX_COUNT_DETECT
			pEntry->one_sec_rx_pkts++;
#endif /* RX_COUNT_DETECT */
		}
#ifndef WIFI_UNIFIED_COMMAND
		pAd->WlanCounters.RxTotByteCount.QuadPart += pRxBlk->MPDUtotalByteCnt;
#endif
		pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;


	RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);

#ifdef IGMP_SNOOP_SUPPORT

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
			&& (wdev->IgmpSnoopEnable)
			&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3)) {
			PUCHAR pDA = pRxBlk->Addr3;
			PUCHAR pSA = pRxBlk->Addr2;
			PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
			PUCHAR pDataEnd = pRxBlk->pData + pRxBlk->DataSize;
			UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

			if (protoType == ETH_P_IP)
				IGMPSnooping(pAd, pDA, pSA, pData, pDataEnd, pEntry, pRxBlk->wcid);
			else if (protoType == ETH_P_IPV6)
				MLDSnooping(pAd, pDA, pSA,  pData, pDataEnd, pEntry, pRxBlk->wcid);
		}

#endif /* IGMP_SNOOP_SUPPORT */

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
		else
			rx_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
	}

	ops->detect_wmm_traffic(pAd, wdev, pRxBlk->UserPriority, FLAG_IS_INPUT);

	return TRUE;
}

BOOLEAN ap_rx_mgmt_drop(
	struct _RTMP_ADAPTER *pAd, FRAME_CONTROL *FC)
{
	if (!FC) {
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_WARN,
			"Band[%d]: FC is NULL\n", hc_get_hw_band_idx(pAd));
		return TRUE;
	}
	switch (pAd->ApCfg.reject_mgmt_rx) {
	case MGMT_RX_ACCEPT_BEACON_PROBERSP:
		if ((FC->SubType != SUBTYPE_BEACON)
			&& (FC->SubType != SUBTYPE_PROBE_RSP)) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO,
				"Band[%d]: Reject mgmt. frame(=%d) here!\n",
				hc_get_hw_band_idx(pAd),
				FC->SubType);
			return TRUE;
		}
		break;
	case MGMT_RX_REJECT_ALL:
		return TRUE;
	case MGMT_RX_ACCEPT_ALL:
	default:
		break;
	}
	return FALSE;
}

BOOLEAN ap_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	struct _RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	INT op_mode = OPMODE_AP;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	struct wifi_dev_ops *ops = NULL;

#ifdef APCLI_SUPPORT
#ifdef WIFI_IAP_BCN_STAT_FEATURE
	UINT i = 0;
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
#endif/*APCLI_SUPPORT*/

#ifdef IDS_SUPPORT

	/*
		Check if a rogue AP impersonats our mgmt frame to spoof clients
		drop it if it's a spoofed frame
	*/
	if (RTMPSpoofedMgmtDetection(pAd, pRxBlk))
		return FALSE;

	/* update sta statistics for traffic flooding detection later */
	RTMPUpdateStaMgmtCounter(pAd, FC->SubType);
#endif /* IDS_SUPPORT */

	if (!pRxInfo->U2M) {
		if ((FC->SubType != SUBTYPE_BEACON) && (FC->SubType != SUBTYPE_PROBE_REQ)) {
			BOOLEAN bDrop = TRUE;
			/* For PMF TEST Plan 5.4.3.1 & 5.4.3.2 */
#ifdef APCLI_SUPPORT

			if ((pEntry) && IS_ENTRY_PEER_AP(pEntry) &&
				((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH) || (FC->SubType == SUBTYPE_ACTION)))
				bDrop = FALSE;

#endif /* APCLI_SUPPORT */

#if defined(WAPP_SUPPORT)

			if (IsPublicActionFrame(pAd, (VOID *)FC))
				bDrop = FALSE;

#endif /* defined(WAPP_SUPPORT) */
#ifdef CONFIG_6G_SUPPORT
			if (FC->SubType == SUBTYPE_PROBE_RSP)
				bDrop = FALSE;
#endif
			if (bDrop == TRUE)
				return FALSE;
		}
#ifdef APCLI_SUPPORT
#ifdef WIFI_IAP_BCN_STAT_FEATURE
		for (i = 0; i < MAX_MULTI_STA; i++) {
			if (!pAd->StaCfg[i].wdev.DevInfo.WdevActive)
				continue;

			if ((FC->SubType == SUBTYPE_BEACON)
				&& (MAC_ADDR_EQUAL(pAd->StaCfg[i].Bssid, pRxBlk->Addr2))) {
					pAd->StaCfg[i].rx_beacon++;
			}
		}
#endif/*APCLI_SUPPORT*/
#endif/*WIFI_IAP_BCN_STAT_FEATURE*/
	}

	/* Software decrypt WEP data during shared WEP negotiation */
	if ((FC->SubType == SUBTYPE_AUTH) &&
		(FC->Wep == 1) && (pRxInfo->Decrypted == 0)) {
		UCHAR *pMgmt = (PUCHAR)FC;
		UINT16 mgmt_len = pRxBlk->MPDUtotalByteCnt;
		UCHAR DefaultKeyId;

		if (!pEntry) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
				"ERROR: SW decrypt WEP data fails - the Entry is empty.\n");
			return FALSE;
		}

		/* Skip 802.11 header */
		pMgmt += LENGTH_802_11;
		mgmt_len -= LENGTH_802_11;
#ifdef CONFIG_AP_SUPPORT
		if (pEntry->func_tb_idx < MAX_BEACON_NUM)
			DefaultKeyId = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PairwiseKeyId;
#endif /*  CONFIG_AP_SUPPORT */

		/* handle WEP decryption */
		if ((pEntry->func_tb_idx < MAX_BEACON_NUM)
			&& (DefaultKeyId < SEC_KEY_NUM)
			&& (RTMPSoftDecryptWEP(
				&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.WepKey[DefaultKeyId],
				pMgmt, &mgmt_len) == FALSE)) {
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				diag_conn_error(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_AUTH_FAIL, REASON_DECRYPTION_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry) && (pEntry->func_tb_idx < MAX_BEACON_NUM))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_MIC_FAILURE);
#endif

			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_ERROR,
				"ERROR: SW decrypt WEP data fails.\n");
			return FALSE;
		}

#ifdef CFG_BIG_ENDIAN
		/* swap 16 bit fields - Auth Alg No. field */
		*(USHORT *)pMgmt = SWAP16(*(USHORT *)pMgmt);
		/* swap 16 bit fields - Auth Seq No. field */
		*(USHORT *)(pMgmt + 2) = SWAP16(*(USHORT *)(pMgmt + 2));
		/* swap 16 bit fields - Status Code field */
		*(USHORT *)(pMgmt + 4) = SWAP16(*(USHORT *)(pMgmt + 4));
#endif /* CFG_BIG_ENDIAN */
		MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO,
			"Decrypt AUTH seq#3 successfully\n");
		/* Update the total length */
		pRxBlk->DataSize -= (LEN_WEP_IV_HDR + LEN_ICV);
	}

	if (pEntry) {
		/* Update rcpi value to pEntry */
		pEntry->rcpi0 = pRxBlk->rcpi[0];
		pEntry->rcpi1 = pRxBlk->rcpi[1];
		pEntry->rcpi2 = pRxBlk->rcpi[2];
		pEntry->rcpi3 = pRxBlk->rcpi[3];
		MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
			"Entry rcpi0=%x\n", pEntry->rcpi0);
		MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
			"Entry rcpi1=%x\n", pEntry->rcpi1);
		MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
			"Entry rcpi2=%x\n", pEntry->rcpi2);
		MTWF_DBG(NULL, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_DEBUG,
			"Entry rcpi3=%x\n", pEntry->rcpi3);

		if ((op_mode == OPMODE_AP) && IS_ENTRY_CLIENT(pEntry)) {
			UCHAR pAddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

			if (!MAC_ADDR_EQUAL(pRxBlk->Addr1, pAddr))
				RtmpPsIndicate(pAd, pRxBlk->Addr2, pRxBlk->wcid, FC->PwrMgmt);
		}

		/*
		 * 20190613 - accroding to IEEE802.11-2016
		 * To change power management modes a STA shall inform the AP by completing a successful frame
		 * exchange (as described in Annex G) that is initiated by the STA. This frame exchange shall include a
		 * Management frame, Extension frame or Data frame from the STA, and an Ack or a BlockAck frame from
		 * the AP.
		 */
	}
#ifdef TXRX_STAT_SUPPORT
	if (FC->SubType == SUBTYPE_AUTH) {
		int wdev_idx = 0;
		struct hdev_ctrl *ctrl_rx = (struct hdev_ctrl *)pAd->hdev_ctrl;

		for (wdev_idx = 0; wdev_idx < WDEV_NUM_MAX; wdev_idx++) {
			if ((pAd->wdev_list[wdev_idx] != NULL) && (MAC_ADDR_EQUAL(pAd->wdev_list[wdev_idx]->bssid, pRxBlk->Addr3))) {
				INC_COUNTER64(ctrl_rx->rdev.pRadioCtrl->RxMgmtPacketCount);
				ctrl_rx->rdev.pRadioCtrl->RxMgmtPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
				break;
			}
		}
	}

	if (pEntry && !(scan_in_run_state(pAd, pEntry->wdev))) {
		int ant_idx;
		struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
		union _HTTRANSMIT_SETTING last_mgmt_rx_rate;
		ULONG MgmtRate;
		if ((pEntry != NULL) && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC || FC->SubType == SUBTYPE_ASSOC_REQ)) {
			INC_COUNTER64(MBSS_GET(pEntry->pMbss)->stat_bss.RxMgmtPacketCount);
			INC_COUNTER64(pEntry->RxMgmtPacketCount);
			INC_COUNTER64(ctrl->rdev.pRadioCtrl->RxMgmtPacketCount);
			ctrl->rdev.pRadioCtrl->RxMgmtPacketByte.QuadPart += pRxBlk->MPDUtotalByteCnt;
			last_mgmt_rx_rate = pRxBlk->rx_rate;
			getRate(last_mgmt_rx_rate, &MgmtRate);
			pEntry->RxLastMgmtPktRate = MgmtRate;
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO, "Rx-Pkt Src Address : "MACSTR"\n", MAC2STR(pRxBlk->Addr2));
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO, "Rx Mgmt Subtype : %d\n", FC->SubType);
			for (ant_idx = 0; ant_idx < 4; ant_idx++)
				pEntry->LastMgmtPktRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), ant_idx);
		}
	} else if (pEntry == NULL)
		INC_COUNTER64(pAd->WlanCounters.RxNot2MMgmtCnt);
#endif


#ifdef CONVERTER_MODE_SWITCH_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		if (((FC->Type == FC_TYPE_MGMT) && ((FC->SubType == SUBTYPE_ASSOC_REQ) ||
			(FC->SubType == SUBTYPE_REASSOC_REQ) || (FC->SubType == SUBTYPE_PROBE_REQ))) &&
			(pAd->ApCfg.MBSSID[0].APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT)) {
			MTWF_DBG(pAd, DBG_CAT_RX, CATRX_MGMT, DBG_LVL_INFO, "Ignore mgmt packet\n");
			return TRUE;
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */


	/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
	{
		struct wifi_dev *recv_wdev = NULL;
		unsigned char *buffer = NULL;
		unsigned char *buf = NULL;
		int i;
		struct sk_buff *skb = (struct sk_buff *)(pRxBlk->pRxPacket);
		struct skb_shared_info *shinfo = skb_shinfo(skb);

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			if (pAd->wdev_list[i]) {
				recv_wdev = pAd->wdev_list[i];
				break;
			}
		}
		if (pEntry && pEntry->wdev && !IS_ENTRY_NONE(pEntry)
			&& (HcGetBandByWdev(pEntry->wdev) == pRxBlk->band))
			recv_wdev = pEntry->wdev;

#ifdef CONFIG_6G_SUPPORT
		if (!pRxInfo->U2M && (FC->SubType == SUBTYPE_PROBE_RSP) && !WMODE_CAP(recv_wdev->PhyMode, WMODE_AX_6G))
			return FALSE;
#endif
		if (!recv_wdev)
			return FALSE;
		if (shinfo->nr_frags > 0) {
			os_alloc_mem(NULL, (PUCHAR *)&buffer, skb->len);
			if (buffer) {
				buf = skb_header_pointer(skb, 0, skb->len, buffer);
				if (buf)
					FC = (FRAME_CONTROL *)buf;
			}
		}

		/* DOT11V_MBSSID_SUPPORT */
		if (recv_wdev) {
			ops = recv_wdev->wdev_ops;
			if (ops && (ops->ieee_802_11_rx_bcn_dup) && PD_GET_11V_BCN_DUP(pAd->physical_dev)) {
				if ((FC->Type == FC_TYPE_MGMT)
					&& (FC->SubType == SUBTYPE_BEACON)) {
					pRxBlk->FC = (u8 *)FC;
					ops->ieee_802_11_rx_bcn_dup(pAd, recv_wdev, pRxBlk);
				}
			}
		}

		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
			FC,
			pRxBlk->DataSize,
			pRxBlk->rx_signal.raw_rssi[0],
			pRxBlk->rx_signal.raw_rssi[1],
			pRxBlk->rx_signal.raw_rssi[2],
			pRxBlk->rx_signal.raw_rssi[3],
			min(pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1]),
			pRxBlk->channel_freq,
			op_mode,
			recv_wdev,
			pRxBlk->rx_rate.field.MODE,
			0,
			pRxBlk->raw_channel);

		if (buffer)
			os_free_mem(buffer);
	}
	return TRUE;
}


struct wifi_dev_ops ap_wdev_ops = {
	.tx_pkt_allowed = ap_tx_pkt_allowed,
	.fp_tx_pkt_allowed = ap_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.legacy_tx = ap_legacy_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.fill_non_offload_tx_blk = ap_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = ap_fill_offload_tx_blk,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
#ifdef VERIFICATION_MODE
	.verify_tx = verify_pkt_tx,
#endif
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = ap_rx_pkt_allowed,
	.rx_pkt_foward = ap_rx_pkt_foward,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
#ifdef DOT11V_MBSSID_SUPPORT
#ifdef APCLI_CFG80211_SUPPORT
	.ieee_802_11_rx_bcn_dup = ap_ieee_802_11_rx_bcn_dup,
#endif /* APCLI_CFG80211_SUPPORT */
#endif /* DOT11V_MBSSID_SUPPORT */
	.find_cipher_algorithm = ap_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
	.media_state_connected = media_state_connected,
	.ioctl = rt28xx_ap_ioctl,
	.open = ap_inf_open,
	.close = ap_inf_close,
	.linkup = ap_link_up,
	.linkdown = ap_link_down,
	.conn_act = ap_conn_act,
	.disconn_act = wifi_sys_disconn_act,
};
