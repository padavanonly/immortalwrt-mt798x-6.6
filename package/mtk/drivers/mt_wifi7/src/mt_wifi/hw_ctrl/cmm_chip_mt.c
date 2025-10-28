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
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"

#define BSSID_WCID_TO_REMOVE 1 /* Pat:TODO */

VOID MTHifPolling(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx)
{
#ifdef RTMP_MAC_PCI
	UINT32 Loop, RxPending = 0;
	PNDIS_PACKET pRxPacket = NULL;
	struct _RX_BLK RxBlk, *pRxBlk;
	BOOLEAN bReschedule = FALSE;
	EVENT_EXT_CMD_RESULT_T	rResult = {0};

	for (Loop = 0; Loop < 10; Loop++) {
		while (1) {
			pRxBlk = &RxBlk;
			/* pRxPacket = asic_get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, 0); */

			if ((RxPending == 0) && (bReschedule == FALSE))
				break;
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);

			msleep(20);
		}
	}

	for (Loop = 0; Loop < 10; Loop++) {
		AsicExtWifiHifCtrl(pAd, ucDbdcIdx, HIF_CTRL_ID_HIF_USB_TX_RX_IDLE, &rResult);

		if (rResult.u4Status == 0)
			break;

		while (1) {
			pRxBlk = &RxBlk;
			/* pRxPacket = asic_get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, 0); */

			if ((RxPending == 0) && (bReschedule == FALSE))
				break;
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
		}

		if (Loop == 1) {
			/* Above scenario should pass at 1st time or assert */
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				"Failed to poll RX path empry.\n");
			ASSERT(0);
		}
	}

#endif /* RTMP_MAC_PCI */
}

VOID MTRadioOn(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	/*  Send radio on command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, HcGetBandByChannel(pAd, wdev->channel), WIFI_RADIO_ON);

	/* Send Led on command */
	HcSetRadioCurStatByChannel(pAd, wdev->channel, PHY_INUSE);
}

VOID MTRadioOff(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	BOOLEAN tx_rx_empty = FALSE;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"(wdev): ch(%d), band(%d), tx_rx_empty(%d)\n", wdev->channel,
		HcGetBandByChannel(pAd, wdev->channel), tx_rx_empty);

	/* Set Radio off flag */
	HcSetRadioCurStatByChannel(pAd, wdev->channel, PHY_RADIOOFF);

	/* Send radio off command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, HcGetBandByChannel(pAd, wdev->channel), WIFI_RADIO_OFF);
}

#ifdef RTMP_MAC_PCI
VOID MTMlmeLpExit(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif

	MCU_CTRL_INIT(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	/*  Send radio on command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, hc_get_hw_band_idx(pAd), WIFI_RADIO_ON);
	/* Send Led on command */
	/* Enable RX */
	/* Offlaod below task to AsicExtPmStateCtrl() */
	/* AsicSetMacTxRx(pAd, ASIC_MAC_RX, TRUE); */
	HcSetAllSupportedBandsRadioOn(pAd);
	/*  Resume sending TX packet */
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
}

VOID MTMlmeLpEnter(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, wdev);
#endif
	/*  Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_STA_SUPPORT
	/* Clear PMKID cache.*/
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pStaCfg->SavedPMKNum = 0;
		RTMPZeroMemory(pStaCfg->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));

	/* Link down first if any association exists*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd)) {
				cntl_disconnect_request(wdev, CNTL_DISASSOC, pStaCfg->Bssid, REASON_DISASSOC_STA_LEAVING);
				RtmpusecDelay(1000);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);

		/* Link down first if any association exists*/
		if (INFRA_ON(pStaCfg) || ADHOC_ON(pAd))
			LinkDown(pAd, FALSE, wdev, NULL);

		RtmpusecDelay(10000);
		/*==========================================*/
		/* Clean up old bss table*/
#ifndef ANDROID_SUPPORT
		/* because abdroid will get scan table when interface down, so we not clean scan table */
		BssTableInit(ScanTab);
#endif /* ANDROID_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			if (pMbss->wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pMbss->wdev.if_dev);
		}
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif /* CONFIG_AP_SUPPORT */
	/*  Disable RX */
	/* Below function is offloaded to AsicExtPmStateCtrl() */
	/* AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE); */
	/* Set Radio off flag*/
	HcSetAllSupportedBandsRadioOff(pAd);
	/* Delay for CR access */
	msleep(1000);
	/*  Send Led off command */
	/*  Send radio off command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, hc_get_hw_band_idx(pAd), WIFI_RADIO_OFF);
	/*  Polling TX/RX path until packets empty */
	MTHifPolling(pAd, HcGetBandByWdev(wdev));
}

#endif /* RTMP_MAC_PCI */

int flip_mac_addr_bit(UCHAR *mac_addr, UCHAR a, CHAR b)
{
	int i = 0;

	for (i = 0; i < 5; i++) {
		UCHAR bit_a = ((a >> (7 - i)) & 0x1);
		UCHAR bit_b = ((b >> (7 - i)) & 0x1);

		if (bit_a == bit_b) {
			mac_addr[0] ^= (0x1 << (7 - i));
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
				"mac_addr conflict, find same bit i = %d addr[0]=%x\n", i, mac_addr[0]);
			return 0;
		}
	}
	/*a and b don't have same bit (bit0 ~ bit4)*/
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
		"mac_addr conflict, not find same bit\n");
	return -1;
}


VOID avoid_mac_addr_conflict(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	u8 band_idx = hc_get_hw_band_idx(pAd);
	struct physical_device *ph_dev = pAd->physical_dev;
	UCHAR *ra0_addr, *ra1_addr, *ra2_addr;

	/*note:follow this rule to avoid apcli and mbss mac address conflict
	** ra0: 00:11:22:33:44:55
	**rai0: 02:11:22:33:44:55
	**rax0: 04:11:22:33:44:55
	**step1: mac[0] |= 0xf8 (Set the last three bits to 0)
	**step2: After step 1, whether the address conflicts
	** if have address conflict, flip the bit of mac[0]
	*/

	ra0_addr = (UCHAR *)ph_dev->ra_mac_table[BAND0];
	ra1_addr = (UCHAR *)ph_dev->ra_mac_table[BAND1];
	ra2_addr = (UCHAR *)ph_dev->ra_mac_table[BAND2];
	ra0_addr[0] &= 0xf8;
	ra1_addr[0] &= 0xf8;
	ra2_addr[0] &= 0xf8;

	if (!NdisCmpMemory(ra0_addr, ra1_addr, MAC_ADDR_LEN) &&
		!NdisCmpMemory(ra1_addr, ra2_addr, MAC_ADDR_LEN)) {
		if (band_idx == BAND1)
			ra1_addr[0] ^= (0x1 << 7);
		if (band_idx == BAND2)
			ra2_addr[0] ^= (0x1 << 6);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"mac address conflict, change the bit: band idx= %d\n", band_idx);
	} else if (!NdisCmpMemory(ra0_addr, ra1_addr, MAC_ADDR_LEN) && band_idx == BAND1) {
		if (flip_mac_addr_bit(ra1_addr, ra1_addr[0], ra2_addr[0]) < 0)
			ra1_addr[0] ^= (0x1 << 7);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"mac_addr conflict, change the byte as[%x]: band idx= %d\n",
			ra1_addr[0], band_idx);
	} else if (!NdisCmpMemory(ra0_addr, ra2_addr, MAC_ADDR_LEN) && band_idx == BAND2) {
		if (flip_mac_addr_bit(ra2_addr, ra2_addr[0], ra1_addr[0]) < 0)
			ra2_addr[0] ^= (0x1 << 6);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"mac_addr conflict, change the byte as[%x]: band idx= %d\n",
			ra2_addr[0], band_idx);
	} else if (!NdisCmpMemory(ra1_addr, ra2_addr, MAC_ADDR_LEN) && band_idx == BAND2) {
		if (flip_mac_addr_bit(ra2_addr, ra2_addr[0], ra0_addr[0]) < 0)
			ra2_addr[0] ^= (0x1 << 6);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"mac_addr conflict, change the byte as[%x]: band idx= %d\n",
			ra2_addr[0], band_idx);
	}
	if (band_idx == BAND1) {
		wdev->if_addr[0] &= 0x07;
		wdev->if_addr[0] |= ra1_addr[0];
	}

	if (band_idx == BAND2) {
		wdev->if_addr[0] &= 0x07;
		wdev->if_addr[0] |= ra2_addr[0];
	}
}

#ifdef CONFIG_AP_SUPPORT

/*
 * NOTE: 2015-April-2.
 * this function is for MT7637/MT7615 and afterward chips
 */
VOID MtAsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR zeroMac[6] = {0};
	UCHAR MacMask = 0;
	INT idx = wdev->func_idx;
	UCHAR *if_addr = (UCHAR *)wdev->if_addr;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

#ifdef MAC_ADDR_ADJACENT_CHK
	if (PD_GET_MAC_ADDR_ADJ(pAd->physical_dev)) {
		if ((band_idx == BAND1 || band_idx == BAND2) &&
			(pAd->bLocalAdminMAC == FALSE)) {
			COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
			if_addr[0] &= 0xe7;
			if_addr[0] |= 0x2;
			if_addr[0] |= (band_idx << 3);
			COPY_MAC_ADDR(pAd->CurrentAddress, if_addr);
		}
	}
#endif /* MAC_ADDR_ADJACENT_CHK */

	if (pAd->ApCfg.BssidNum <= 2)
		MacMask = 0xef;
	else if (pAd->ApCfg.BssidNum <= 4)
		MacMask = 0xcf;
	else if (pAd->ApCfg.BssidNum <= 8)
		MacMask = 0x8f;
	else if (pAd->ApCfg.BssidNum <= 16)
		MacMask = 0x0f;
	else
		MacMask = 0x0f;
	if (idx > 0) {
		if (NdisEqualMemory(zeroMac, pAd->ExtendMBssAddr[idx - 1], MAC_ADDR_LEN)) {
			COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
			/*avoid mac addr conflict*/
			avoid_mac_addr_conflict(pAd, wdev);

#ifdef MAC_ADDR_ADJACENT_CHK
			if (PD_GET_MAC_ADDR_ADJ(pAd->physical_dev)) {
				if ((band_idx == BAND1 || band_idx == BAND2) &&
					(pAd->bLocalAdminMAC == FALSE)) {
					if_addr[0] &= 0xe7;
					if_addr[0] |= 0x2;
					if_addr[0] |= (band_idx << 3);
				}
			}
#endif /* MAC_ADDR_ADJACENT_CHK */

			if_addr[0] |= 0x2;
			/* default choose bit[31:28], if there is no assigned mac from profile. */
			if_addr[3] = if_addr[3] & MacMask;/* clear high 4 bits, */
			if_addr[3] = (if_addr[3] | ((idx % 16) << 4));

			/* reverse bit[24] if BssidNum >= 16 */
			if (idx >= 16)
				if_addr[3] ^= 0x1;

			if (NdisEqualMemory(if_addr, pAd->CurrentAddress, MAC_ADDR_LEN))
				if_addr[3] = if_addr[3] & MacMask;

			COPY_MAC_ADDR(pAd->ExtendMBssAddr[idx - 1], if_addr);
		} else
			COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[idx - 1]);
	} else
		COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_NOTICE,
			 "band_idx=%d, mbss_idx=%d, if_addr ="MACSTR"\n", band_idx, idx, MAC2STR(if_addr));
}
#endif /*CONFIG_AP_SUPPORT*/

#ifdef CONFIG_APSTA_MIXED_SUPPORT
VOID MtAsicSetApcliWdevIfAddr(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR zeroMac[6] = {0};
	INT idx = wdev->func_idx;
	UCHAR *if_addr = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
	if_addr = (UCHAR *)wdev->if_addr;

#ifdef MT_MAC
	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
#endif /* MT_MAC */

		if (cap->MBSSIDMode >= MBSSID_MODE1) {
			if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0)) {
				UCHAR MacMask = 0;

				if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
					MacMask = 0xFE;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
					MacMask = 0xFC;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
					MacMask = 0xF8;


				/*
					Refer to HW definition -
						Bit1 of MAC address Byte0 is local administration bit
						and should be set to 1 in extended multiple BSSIDs'
						Bit3~ of MAC address Byte0 is extended multiple BSSID index.
				*/
				if (cap->MBSSIDMode == MBSSID_MODE1) {
					/*
						Refer to HW definition -
							Bit1 of MAC address Byte0 is local administration bit
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					*/
#ifdef ENHANCE_NEW_MBSSID_MODE
		a			wdev->if_addr[0] &= (MacMask << 2);
#endif /* ENHANCE_NEW_MBSSID_MODE */
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
				}

#ifdef ENHANCE_NEW_MBSSID_MODE
				else {
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += ((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
				}

#endif /* ENHANCE_NEW_MBSSID_MODE */
			}
		} else
			wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;

#ifdef MT_MAC
	} else {
		UCHAR MacByte = 0;
		UINT32 Value = 0;

		HW_IO_READ32(pAd->hdev_ctrl, LPON_BTEIR, &Value);
		MacByte = Value >> 29;
		if ((idx >= MAX_MULTI_STA) || (idx < 0)) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				"error: interface idx out of range\n");
			return;
		}

		if (NdisEqualMemory(zeroMac, &pAd->ApcliAddr[idx][0], MAC_ADDR_LEN)) {

			/*void MAC address aviod*/
			avoid_mac_addr_conflict(pAd, wdev);

			/*
			 * mac addr not assigned
			 * Flip bit[idx+2] for apcli(idx)
			 * Ex, BSS0 if_addr[0] = 0x4
			 *     -> apcli(0)=0x2, bit[1]=1 | bit[2]=0x1 -> 0x0
			 *     -> apcli(1)=0xe, bit[1]=1 | bit[3]=0x0 -> 0x1
			 */
			if_addr[0] |= 0x2;
			if_addr[0] ^= (0x1 << (2 + idx));

			switch (MacByte) {
			case 0x2: /* choose bit[31:28]*/
				wdev->if_addr[3] = wdev->if_addr[3] & 0x0f;
				break;
			}
			/* multiple inf down-up will change address because of "if_addr[0] ^= (0x1 << (2 + idx))" code */
			/* In case of DBDC mode (AX1800) it result into both apcli inf will have same MAC addr*/
			/* Store the first time derived address to ApcliAddr so that for next apcli down up same is used */
			COPY_MAC_ADDR(&pAd->ApcliAddr[idx][0], if_addr);
		} else {
			/* mac addr assigned */
			COPY_MAC_ADDR(if_addr, &pAd->ApcliAddr[idx][0]);
		}
	}
#endif /* MT_MAC */

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
		"%s apcli_idx = %d, if_addr = %x %x %x %x %x %x (MBSSIDMode:%d)\n",
		__FILE__, idx, PRINT_MAC(if_addr), cap->MBSSIDMode);
}
#endif

VOID MtAsicSetWdevIfAddr(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode)
{
	if (opmode == OPMODE_AP) {
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11V_MBSSID_SUPPORT
		if (IS_BSSID_11V_ENABLED(pAd) || pAd->CommonCfg.wifi_cert)
			mbss_11v_set_if_addr_gen3(pAd, wdev);
		else
#endif /* DOT11V_MBSSID_SUPPORT */
			MtAsicSetMbssWdevIfAddrGen2(pAd, wdev);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	} else if (opmode == OPMODE_STA) {
		MtAsicSetApcliWdevIfAddr(pAd, wdev);
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_ERROR,
				 "wdev(%d) non-supported opmode(%d)\n", wdev->wdev_idx, opmode);
	}
}

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID mt_asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (!get_pcie_aspm_dym_ctrl_cap(pAd)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO, "return since cap=0\n");
		return;
	}

	if (ops->pcie_aspm_dym_ctrl)
		ops->pcie_aspm_dym_ctrl(pAd, ucDbdcIdx, fgL1Enable, fgL0sEnable);
	else
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"For this chip, no specified dynamic aspm ctrl function!\n");
}
#endif /* #ifdef PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID mt_asic_twt_agrt_update(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (!wlan_config_get_asic_twt_caps(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "return, twt h/w cap=0\n");
		return;
	}

	if (ops->twt_agrt_update)
		ops->twt_agrt_update(ad, agrt_para);
	else
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "twt_agrt_update=NULL\n");

#ifdef TWT_AGRT_ALLOC_SIM
	/* in real case: fw will free twt agrt resource when del twt session */
	/* in sim case: driver need to free twt agrt resource */
	if (agrt_para->agrt_ctrl_flag == TWT_AGRT_CTRL_DELETE) {
		struct TWT_AGRT_PARA_T agrt_para_agrt_free = {0};

		agrt_para_agrt_free.agrt_ctrl_flag = TWT_AGRT_CTRL_AGRT_FREE;
		agrt_para_agrt_free.agrt_tbl_idx = agrt_para->agrt_tbl_idx;
		agrt_para_agrt_free.agrt_tbl_idx_h = agrt_para->agrt_tbl_idx_h;
		agrt_para_agrt_free.peer_id_grp_id = agrt_para->peer_id_grp_id;
		mt_asic_twt_agrt_free(wdev, &agrt_para_agrt_free);
	}
#endif /* TWT_AGRT_ALLOC_SIM */
}

VOID mt_asic_twt_agrt_update_by_cmd_task(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (!wlan_config_get_asic_twt_caps(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "return, twt h/w cap=0\n");
		return;
	}

	rtmp_twt_agrt_update_by_cmd_task(ad, agrt_para);
}


static VOID mt_asic_twt_agrt_mgmt(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para, struct TWT_AGRT_MGMT_T *agrt_mgmt)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (!wlan_config_get_asic_twt_caps(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "return, twt h/w cap=0\n");
		return;
	}

	if (ops->twt_agrt_mgmt)
		ops->twt_agrt_mgmt(ad, agrt_para, agrt_mgmt);
	else
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR, "twt_agrt_update=NULL\n");
}

VOID mt_asic_twt_agrt_alloc(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para, struct TWT_AGRT_MGMT_T *agrt_mgmt)
{
	mt_asic_twt_agrt_mgmt(wdev, agrt_para, agrt_mgmt);
}

VOID mt_asic_twt_agrt_free(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para)
{
	mt_asic_twt_agrt_mgmt(wdev, agrt_para, NULL);
}

VOID mt_asic_twt_mgmt_frame_offload(
	struct wifi_dev *wdev,
	UINT8 bssinfo_idx,
	UINT16 wcid,
	UINT8 token,
	UINT8 mlo_en,
	UINT16 mld_sta_idx,
	UINT8 action_linkid,
	UINT16 all_linkid_bitmap,
	UINT8 *buf,
	UINT32 len)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (!wlan_config_get_asic_twt_caps(wdev)) {
		MTWF_DBG(ad, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"return, twt h/w cap=0\n");
		return;
	}

	if (ops->mgmt_frame_offload)
		ops->mgmt_frame_offload(ad,
			bssinfo_idx,
			wcid,
			token,
			mlo_en,
			mld_sta_idx,
			action_linkid,
			all_linkid_bitmap,
			buf,
			len);
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
