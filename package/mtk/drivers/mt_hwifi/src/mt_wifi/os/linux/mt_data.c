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
/****************************************************************************
 ****************************************************************************

    Module Name:
	mt_data.c

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include "rt_config.h"

/* We assume the s1 is a sting, s2 is a memory space with 6 bytes.
 * and content of s1 will be changed.
 */
BOOLEAN rtstrmactohex(RTMP_STRING *s1, RTMP_STRING *s2)
{
	int i = 0;
	RTMP_STRING *ptokS = s1, *ptokE;

	if (strlen(s1) != MAC_ADDR_STR_LEN)
		return FALSE;

	while ((*ptokS) != '\0') {
		ptokE = strchr(ptokS, ':');

		if (ptokE != NULL)
			*ptokE++ = '\0';

		if ((strlen(ptokS) != 2) || (!isxdigit(*ptokS)) || (!isxdigit(*(ptokS + 1))))
			break; /* fail*/

		AtoH(ptokS, (PUCHAR)&s2[i++], 1);
		ptokS = ptokE;

		if (ptokS == NULL)
			break;

		if (i == 6)
			break; /* parsing finished*/
	}

	return (i == 6 ? TRUE : FALSE);
}


#define ASC_LOWER(_x)	((((_x) >= 0x41) && ((_x) <= 0x5a)) ? (_x) + 0x20 : (_x))
/* we assume the s1 and s2 both are strings.*/
BOOLEAN rtstrcasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	RTMP_STRING *p1 = s1, *p2 = s2;
	CHAR c1, c2;

	if (strlen(s1) != strlen(s2))
		return FALSE;

	while (*p1 != '\0') {
		c1 = ASC_LOWER(*p1);
		c2 = ASC_LOWER(*p2);

		if (c1 != c2)
			return FALSE;

		p1++;
		p2++;
	}

	return TRUE;
}


/* we assume the s1 (buffer) and s2 (key) both are strings.*/
RTMP_STRING *rtstrstruncasecmp(RTMP_STRING *s1, RTMP_STRING *s2)
{
	INT l1, l2, i;
	char temp1, temp2;

	l2 = strlen(s2);

	if (!l2)
		return (char *) s1;

	l1 = strlen(s1);

	while (l1 >= l2) {
		l1--;

		for (i = 0; i < l2; i++) {
			temp1 = *(s1 + i);
			temp2 = *(s2 + i);

			if (('a' <= temp1) && (temp1 <= 'z'))
				temp1 = 'A' + (temp1 - 'a');

			if (('a' <= temp2) && (temp2 <= 'z'))
				temp2 = 'A' + (temp2 - 'a');

			if (temp1 != temp2)
				break;
		}

		if (i == l2)
			return (char *) s1;

		s1++;
	}

	return NULL; /* not found*/
}


/**
 * strstr - Find the first substring in a %NUL terminated string
 * @s1: The string to be searched
 * @s2: The string to search for
 */
RTMP_STRING *rtstrstr(const RTMP_STRING *s1, const RTMP_STRING *s2)
{
	INT l1, l2;

	l2 = strlen(s2);

	if (!l2)
		return (RTMP_STRING *)s1;

	l1 = strlen(s1);

	while (l1 >= l2) {
		l1--;

		if (!memcmp(s1, s2, l2))
			return (RTMP_STRING *)s1;

		s1++;
	}

	return NULL;
}

/**
 * rstrtok - Split a string into tokens
 * @s: The string to be searched
 * @ct: The characters to search for
 * * WARNING: strtok is deprecated, use strsep instead.
 * * However strsep is not compatible with old architecture.
 */
RTMP_STRING *__rstrtok;
RTMP_STRING *rstrtok(RTMP_STRING *s, const RTMP_STRING *ct)
{
	RTMP_STRING *sbegin, *send;

	sbegin  = s ? s : __rstrtok;

	if (!sbegin)
		return NULL;

	sbegin += strspn(sbegin, ct);

	if (*sbegin == '\0') {
		__rstrtok = NULL;
		return NULL;
	}

	send = strpbrk(sbegin, ct);

	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;
	return sbegin;
}

/**
 * delimitcnt - return the count of a given delimiter in a given string.
 * @s: The string to be searched.
 * @ct: The delimiter to search for.
 * Notice : We suppose the delimiter is a single-char string(for example : ";").
 */
INT delimitcnt(RTMP_STRING *s, RTMP_STRING *ct)
{
	INT count = 0;
	/* point to the beginning of the line */
	RTMP_STRING *token = s;

	for (;; ) {
		token = strpbrk(token, ct); /* search for delimiters */

		if (token == NULL) {
			/* advanced to the terminating null character */
			break;
		}

		/* skip the delimiter */
		++token;
		/*
		 * Print the found text: use len with %.*s to specify field width.
		 */
		/* accumulate delimiter count */
		++count;
	}

	return count;
}

/*
  * converts the Internet host address from the standard numbers-and-dots notation
  * into binary data.
  * returns nonzero if the address is valid, zero if not.
  */
int rtinet_aton(const RTMP_STRING *cp, unsigned int *addr)
{
	unsigned int	val;
	int	base, n;
	RTMP_STRING c;
	unsigned int    parts[4] = {0};
	unsigned int    *pp = parts;

	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 *	0x=hex, 0=octal, other=decimal.
		 */
		val = 0;
		base = 10;

		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}

		while ((c = *cp) != '\0') {
			if (isdigit((unsigned char) c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}

			if (base == 16 && isxdigit((unsigned char) c)) {
				val = (val << 4) +
					  (c + 10 - (islower((unsigned char) c) ? 'a' : 'A'));
				cp++;
				continue;
			}

			break;
		}

		if (*cp == '.') {
			/*
			 * Internet format: a.b.c.d a.b.c   (with c treated as 16-bits)
			 * a.b     (with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return 0;

			*pp++ = val, cp++;
		} else
			break;
	}

	/*
	 * Check for trailing junk.
	 */
	while (*cp)
		if (!isspace((unsigned char) *cp++))
			return 0;

	/*
	 * Concoct the address according to the number of parts specified.
	 */
	n = pp - parts + 1;

	switch (n) {
	case 1:         /* a -- 32 bits */
		break;

	case 2:         /* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return 0;

		val |= parts[0] << 24;
		break;

	case 3:         /* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return 0;

		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:         /* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return 0;

		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}

	*addr = OS_HTONL(val);
	return 1;
}

#ifdef SNIFFER_RADIOTAP_SUPPORT
void announce_802_11_radiotap_packet(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, struct _RX_BLK *rx_blk)
{
	struct sk_buff *pOSPkt;
	UCHAR wdev_idx = BSS0;

	radiotap_fill_field(pPacket, &rx_blk->radiotap_info);
	pOSPkt = RTPKT_TO_OSPKT(pPacket);
	skb_reset_mac_header(pOSPkt);
	pOSPkt->protocol = htons(ETH_P_802_2);

	pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);
	pOSPkt->pkt_type = PACKET_OTHERHOST;
	pOSPkt->ip_summed = CHECKSUM_NONE;

	if (pOSPkt->dev == NULL)
		dev_kfree_skb(pPacket);
	else
		netif_rx(pOSPkt);

}
#endif


void announce_802_3_packet(
	IN VOID *pAdSrc,
	IN PNDIS_PACKET pPacket,
	IN UCHAR OpMode)
{
	RTMP_ADAPTER *pAd = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
	VOID *napi = NULL;

	pAd =  (RTMP_ADAPTER *)pAdSrc;
	napi = pAd->tr_ctl.napi;
	ASSERT(pAd);
	ASSERT(pPacket);

	if (pAd == NULL || pPacket == NULL)
		return;
	MEM_DBG_PKT_FREE_INC(pPacket);

#ifdef CONFIG_WIFI_PREFETCH_RXDATA
	prefetch(GET_OS_PKT_DATAPTR(pPacket));
#endif /* CONFIG_WIFI_PREFETCH_RXDATA */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT

		if (RTMP_MATPktRxNeedConvert(pAd, RtmpOsPktNetDevGet(pRxPkt)))
			RTMP_MATEngineRxHandle(pAd, pRxPkt, 0);
#endif /* MAT_SUPPORT */
	}

#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef MAT_SUPPORT

		if (RTMP_MATPktRxNeedConvert(pAd, RtmpOsPktNetDevGet(pRxPkt)) &&
			(pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE))
			RTMP_MATEngineRxHandle(pAd, pRxPkt, 0);

#endif /* MAT_SUPPORT */
	}

#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	/* Push up the protocol stack */
#ifdef IKANOS_VX_1X0
	{
		IKANOS_DataFrameRx(pAd, pRxPkt);
		return;
	}
#endif /* IKANOS_VX_1X0 */
#ifdef INF_PPA_SUPPORT
	{
		if (ppa_hook_directpath_send_fn && (pAd->PPAEnable == TRUE)) {
			INT retVal, ret = 0;
			UINT ppa_flags = 0;

			retVal = process_nbns_packet(pAd, pRxPkt);

			if (retVal > 0) {
				ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pRxPkt, pRxPkt->len, ppa_flags);

				if (ret == 0) {
					pRxPkt = NULL;
					return;
				}

				RtmpOsPktRcvHandle(pRxPkt, napi);
			} else if (retVal == 0) {
				RtmpOsPktProtocolAssign(pRxPkt);
				RtmpOsPktRcvHandle(pRxPkt, napi);
			} else {
				dev_kfree_skb_any(pRxPkt);
				MEM_DBG_PKT_FREE_INC(pAd);
			}
		} else {
			RtmpOsPktProtocolAssign(pRxPkt);
			RtmpOsPktRcvHandle(pRxPkt, napi);
		}

		return;
	}
#endif /* INF_PPA_SUPPORT */
	{
#ifdef CONFIG_RT2880_BRIDGING_ONLY
		PACKET_CB_ASSIGN(pRxPkt, 22) = 0xa8;
#endif
#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)
		if (ra_classifier_hook_rx != NULL)
			ra_classifier_hook_rx(pRxPkt, classifier_cur_cycle);
#endif /* CONFIG_RA_CLASSIFIER */

#ifdef DYNAMIC_VLAN_SUPPORT
	{
		USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);

		if (VALID_UCAST_ENTRY_WCID(pAd, Wcid)) {
			MAC_TABLE_ENTRY *pMacEntry = entry_get(pAd, Wcid);

			if (pMacEntry->vlan_id) {
				UCHAR VLAN_Size = LENGTH_802_1Q;
				UCHAR *data_p;
				UINT16 TCI;
				UCHAR   TPID[] = {0x81, 0x00}; /* VLAN related */
				UINT16 TypeLen;

				data_p = GET_OS_PKT_DATAPTR(pRxPkt);
				TypeLen = (data_p[12] << 8) | data_p[13];
				if (TypeLen != ETH_TYPE_EAPOL) {
					struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pPacket);

					memmove((GET_OS_PKT_DATAPTR(pRxPkt)) - 4, (GET_OS_PKT_DATAPTR(pRxPkt)), 12);
					data_p = skb_push(pRxPkt, VLAN_Size);
					/* make up TCI field keeping vlan priority to 0 currently*/

					TCI = (pMacEntry->vlan_id & 0x0fff) | ((0 & 0x7) << 13);

#ifndef CFG_BIG_ENDIAN
					TCI = SWAP16(TCI);
#endif /* CFG_BIG_ENDIAN */
					*(UINT16 *) (data_p + LENGTH_802_3_NO_TYPE) = *(UINT16 *) TPID;
					*(UINT16 *) (data_p + LENGTH_802_3_NO_TYPE + 2) = TCI;
					MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
						"%s: add vlan tag with vlan_id=%d, send pkt to %s\n",
						__func__, pMacEntry->vlan_id, pOSPkt->dev->name);
				}

			}
		}
	}
#endif

#ifdef CONFIG_FAST_NAT_SUPPORT
		if (ra_sw_nat_hook_rx) {
#ifdef WHNAT_SUPPORT
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
			BOOLEAN whnat_rx_en = PD_GET_WHNAT_ENABLE(pAd->physical_dev) &&
							(cap->tkn_info.feature & TOKEN_RX);
#else
			BOOLEAN whnat_rx_en = FALSE;
#endif
#ifdef PKTLOSS_CHK
			if (pAd->pktloss_chk.enable)
				pAd->pktloss_chk.pktloss_chk_handler(pAd, GET_OS_PKT_DATAPTR(pRxPkt), MAT_ETHER_HDR_LEN, 3, FALSE);
#endif
			RtmpOsPktProtocolAssign(pRxPkt);

			if (!whnat_rx_en) {
				RtmpOsPktNatMagicTag(pRxPkt);
				if (ra_sw_nat_hook_rx(pRxPkt))
					RtmpOsPktRcvHandle(pRxPkt, napi);
			} else {
				if (RTMP_GET_PACKET_TYPE(pRxPkt)
						== RX_PPE_VALID)
					ra_sw_nat_hook_rx(pRxPkt);
				RtmpOsPktRcvHandle(pRxPkt, napi);
			}

			return;
		}
#endif /* CONFIG_FAST_NAT_SUPPORT */
	}
#ifdef TRACELOG_TCP_PKT

	if (RTMPIsTcpAckPkt(pRxPkt))
		pAd->u4TcpRxAckCnt++;

#endif
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckUpdateDataCnx(pAd, pRxPkt);
#endif
#ifdef PKTLOSS_CHK
	if (pAd->pktloss_chk.enable)
		pAd->pktloss_chk.pktloss_chk_handler(pAd, GET_OS_PKT_DATAPTR(pRxPkt), MAT_ETHER_HDR_LEN, 4, FALSE);
#endif
	RtmpOsPktProtocolAssign(pRxPkt);

	RtmpOsPktRcvHandle(pRxPkt, napi);
}


#ifdef SNIFFER_SUPPORT
INT Monitor_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(dev_p));

	if (VIRTUAL_IF_INIT(pAd, dev_p) != 0)
		return -1;

	if (VIRTUAL_IF_UP(pAd, dev_p) != 0)
		return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(dev_p);
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_OPEN, 0, dev_p, 0);
	/* Monitor_Open(pAd,dev_p); */
	return 0;
}

INT Monitor_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(dev_p));

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_CLOSE, 0, dev_p, 0);
	/* Monitor_Close(pAd,dev_p); */
	VIRTUAL_IF_DOWN(pAd, dev_p);

	VIRTUAL_IF_DEINIT(pAd, dev_p);

	RT_MOD_HNAT_DEREG(dev_p);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}


VOID RT28xx_Monitor_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;
	/* init operation functions */
	NdisZeroMemory(&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = Monitor_VirtualIF_Open;
	netDevOpHook.stop = Monitor_VirtualIF_Close;
	netDevOpHook.xmit = rt28xx_send_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO, "!!!!####!!!!!!\n");
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_INIT,	0, &netDevOpHook, 0);
}
VOID RT28xx_Monitor_Remove(VOID *pAd)
{
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_REMOVE, 0, NULL, 0);
}

void STA_MonPktSend(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk)
{
	PNET_DEV pNetDev;
	PNDIS_PACKET pRxPacket;
	UCHAR *dot11_fc_field;
	USHORT DataSize;
	CHAR MaxRssi, RSSI1;
	UINT32 timestamp = 0;
	CHAR RssiForRadiotap = 0;
	UCHAR L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM, AMPDU, STBC;
	UCHAR BssMonitorFlag11n, Channel, CentralChannel = 0;
	UCHAR *pData, *pDevName;
	UCHAR sniffer_type = pAd->sniffer_ctl.sniffer_type;
	UCHAR sideband_index = 0;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UINT32 UP_value = 0;
#ifdef SNIFFER_MT7615
	UINT32 value = 0;
	UCHAR gid = 0;
#endif
	ASSERT(pRxBlk->pRxPacket);

	if (pRxBlk->DataSize < 10) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				 "Size is too small! (%d)\n", pRxBlk->DataSize);
		goto err_free_sk_buff;
	}

	if (sniffer_type == RADIOTAP_TYPE) {
		if (pRxBlk->DataSize + sizeof(struct mtk_radiotap_header) > pAd->monitor_ctrl.FilterSize) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					 "Size is too large! (%ld)\n",
					  pRxBlk->DataSize + sizeof(struct mtk_radiotap_header));
			goto err_free_sk_buff;
		}
	}

	if (sniffer_type == PRISM_TYPE) {
		if (pRxBlk->DataSize + sizeof(wlan_ng_prism2_header) > RX_BUFFER_AGGRESIZE) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					 "Size is too large! (%ld)\n",
					  pRxBlk->DataSize + sizeof(wlan_ng_prism2_header));
			goto err_free_sk_buff;
		}
	}

	MaxRssi = RTMPMaxRssi(pAd,
						  ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0),
						  ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1),
						  ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2));

	if (sniffer_type == RADIOTAP_TYPE) {
		RssiForRadiotap = RTMPMaxRssi(pAd,
									  ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0),
									  ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1),
									  ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_2));
	}

#ifdef SNIFFER_MT7615
	pNetDev = get_netdev_from_bssid(pAd, BSS0);
#else
	pNetDev = pAd->monitor_ctrl.wdev.if_dev;  /* send packet to mon0 */
#endif
	pRxPacket = pRxBlk->pRxPacket;
	dot11_fc_field = pRxBlk->FC;
	pData = pRxBlk->pData;
	DataSize = pRxBlk->DataSize;
	L2PAD = pRxBlk->pRxInfo->L2PAD;
	PHYMODE = pRxBlk->rx_rate.field.MODE;
	BW = pRxBlk->rx_rate.field.BW;
	ShortGI = pRxBlk->rx_rate.field.ShortGI;
	MCS = pRxBlk->rx_rate.field.MCS;
	LDPC = pRxBlk->rx_rate.field.ldpc;

	if (IS_HIF_TYPE(pAd, HIF_RLT))
		LDPC_EX_SYM = pRxBlk->ldpc_ex_sym;
	else
		LDPC_EX_SYM = 0;

	AMPDU = pRxBlk->pRxInfo->AMPDU;
	STBC = pRxBlk->rx_rate.field.STBC;
	RSSI1 = pRxBlk->rx_signal.raw_rssi[1];
	/* if(pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[12] != 0) */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		timestamp = pRxBlk->TimeStamp;

#endif
	BssMonitorFlag11n = 0;
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	BssMonitorFlag11n = (pAd->StaCfg[0].BssMonitorFlag & MONITOR_FLAG_11N_SNIFFER);
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	pDevName = (UCHAR *)RtmpOsGetNetDevName(pAd->net_dev);
	Channel = pAd->ApCfg.MBSSID[0].wdev.channel;

	if (BW == BW_20)
		CentralChannel = Channel;
	else if (BW == BW_40)
		CentralChannel = wlan_operate_get_cen_ch_1(wdev);

#ifdef DOT11_VHT_AC
#ifdef SNIFFER_MT7615
	else if (BW == BW_80 || BW == BW_160)
#else
	else if (BW == BW_80)
#endif /* SNIFFER_MT7615 */
		CentralChannel = wlan_operate_get_cen_ch_1(wdev);

#endif /* DOT11_VHT_AC */
#ifdef DOT11_VHT_AC

	if (BW == BW_80)
		sideband_index = vht_prim_ch_idx(CentralChannel, Channel, RF_BW_80);

#ifdef SNIFFER_MT7615
	else if (BW == BW_160)
		sideband_index = vht_prim_ch_idx(CentralChannel, Channel, RF_BW_160);

#endif /* SNIFFER_MT7615 */
#endif /* DOT11_VHT_AC */

	if (sniffer_type == RADIOTAP_TYPE) {
		UP_value = 0;
		send_radiotap_monitor_packets(
			pNetDev, pRxBlk->AmsduState, pRxBlk->rmac_info, pRxPacket,
			(void *)dot11_fc_field, pData, DataSize,
			L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM,
			AMPDU, STBC, RSSI1, pDevName, Channel, CentralChannel,
			sideband_index, RssiForRadiotap, timestamp, UP_value);
	}
	if (sniffer_type == PRISM_TYPE) {
		send_prism_monitor_packets(
			pNetDev, pRxPacket, (void *)dot11_fc_field, pData, DataSize,
			L2PAD, PHYMODE, BW, ShortGI, MCS, AMPDU, STBC, RSSI1,
			BssMonitorFlag11n, pDevName, Channel, CentralChannel,
			MaxRssi);
	}

	return;
err_free_sk_buff:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
}
#endif /* SNIFFER_SUPPORT */
#ifdef ACTION_MONITOR_SUPPORT

/*click_wifi_extra header is used for forwarding
action/probe request from driver to click OS*/
#define CLICK_WIFI_SCHEDULE_SIZE 2
#define CLICK_WIFI_MAX_ANTENNA 8
#define CLICK_NOISE_OFFSET 95

#define CLICK_WIFI_EXTRA_MAGIC          0x74

#define CLICK_WIFI_EXTRA_NSS_1          0x01
#define CLICK_WIFI_EXTRA_NSS_2          0x02
#define CLICK_WIFI_EXTRA_NSS_3          0x03
#define CLICK_WIFI_EXTRA_NSS_4          0x04
#define CLICK_WIFI_EXTRA_NSS_5          0x05
#define CLICK_WIFI_EXTRA_NSS_6          0x06
#define CLICK_WIFI_EXTRA_NSS_7          0x07
#define CLICK_WIFI_EXTRA_NSS_8          0x08

#define CLICK_WIFI_EXTRA_BW_20          0x00
#define CLICK_WIFI_EXTRA_BW_40          0x10
#define CLICK_WIFI_EXTRA_BW_80          0x20
#define CLICK_WIFI_EXTRA_BW_160         0x30

#define CLICK_WIFI_EXTRA_SGI            0x00
#define CLICK_WIFI_EXTRA_LGI            0x40


struct click_wifi_extra {
	UCHAR magic; /*should be 0x74*/
	struct {
		UCHAR fgTx:1;
		UCHAR fgRx_4ss:1;
		UCHAR fgRx_he:1;
		UCHAR fgRx_err:1;
		UCHAR fgRx_more:1;
		UCHAR fgAuto_rate:1;
		UCHAR fgEncrypt:1;
		UCHAR fgLog:1;
	} ucFlags;
	UCHAR channel;
	UCHAR keyidx; /*ignore*/
	UCHAR rssi;
	UCHAR silence; /*silence = noise_floor + CLICK_NOISE_OFFSET, silence >= 0*/
	UCHAR power; /*power = rssi - silence*/
	UCHAR retries; /*ignore*/
	UCHAR max_tries[CLICK_WIFI_SCHEDULE_SIZE]; /*ignore*/
	UCHAR rates[CLICK_WIFI_SCHEDULE_SIZE];

	/* this is for capwap sniffer mode to get phy info*/
	UCHAR phymode;
	UCHAR unused[3];
	UCHAR rssi_per_antenna[CLICK_WIFI_MAX_ANTENNA];
	UCHAR band_id;
};


int rt28xx_send_action_packets(struct sk_buff *skb, struct net_device *ndev)
{
	struct wifi_dev *wdev;
	int status = 0;
	RTMP_ADAPTER *pAd;
	HEADER_802_11 *pHead;

	os_zero_mem((PUCHAR)&skb->cb[CB_OFF], CB_LEN);
	if (!(RTMP_OS_NETDEV_STATE_RUNNING(ndev))) {
		RELEASE_NDIS_PACKET(NULL, (PNDIS_PACKET)skb, NDIS_STATUS_FAILURE);
		return 0;
	}
	wdev = RTMP_OS_NETDEV_GET_WDEV(skb->dev);
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_MGMT, DBG_LVL_ERROR, "wdev is NULL!\n");
		RELEASE_NDIS_PACKET(NULL, (PNDIS_PACKET)skb, NDIS_STATUS_FAILURE);
		ASSERT(wdev);
		return status;
	}
	if (!wdev->sys_handle) {
		ASSERT(wdev->sys_handle);
		return 0;
	}
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	if (pAd->bActionMonOn) {
		if (skb->len > 24) {
			pHead = (HEADER_802_11 *)skb->data;
			if ((pHead->FC.Type == FC_TYPE_MGMT) && ((pHead->FC.SubType == SUBTYPE_ACTION) ||
				(pHead->FC.SubType == SUBTYPE_ACTION_NO_ACK)))
				status = MiniportMMRequest(pAd, 0, skb->data, skb->len, NULL);
		}
	}
	RELEASE_NDIS_PACKET(NULL, (PNDIS_PACKET)skb, NDIS_STATUS_SUCCESS);
	return status;
}
INT Action_Mon_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_MISC, DBG_LVL_INFO, "===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(dev_p));
	RT_MOD_INC_USE_COUNT();
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_ACTION_MON_OPEN, 0, dev_p, 0);
	return 0;
}
INT Action_Mon_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_MISC, DBG_LVL_INFO, "===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(dev_p));
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_ACTION_MON_CLOSE, 0, dev_p, 0);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}
VOID RT28xx_Action_Mon_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;

	NdisZeroMemory(&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = Action_Mon_VirtualIF_Open;
	netDevOpHook.stop = Action_Mon_VirtualIF_Close;
	netDevOpHook.xmit = rt28xx_send_action_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	MTWF_DBG(pAd, DBG_CAT_CFG, DBG_SUBCAT_MISC, DBG_LVL_INFO, "!!!!####!!!!!!\n");
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_ACTION_MON_INIT,	0, &netDevOpHook, 0);
}
VOID RT28xx_Action_Mon_Remove(VOID *pAd)
{
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_ACTION_MON_REMOVE, 0, NULL, 0);
}
BOOLEAN ActionPktSend(RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk, BOOLEAN isProbe)
{
	BOOLEAN bProcessed = FALSE;
	struct wifi_dev *pWdev = wdev_search_by_address(pAd, pRxBlk->Addr1);

	PNET_DEV net_dev = pAd->act_mon_wdev.if_dev;
	UCHAR *pData = NULL;
	struct click_wifi_extra *click_extra_hdr;
	UINT16  extra_header_len =  sizeof(struct click_wifi_extra);
	UINT8 ant_idx, nss;
	INT8 rssi = 0, cur_rssi;
	INT8 noise;
	UINT8 silence;
	UINT i;
	UCHAR status;



	if ((pWdev && (pWdev->wdev_type != WDEV_TYPE_STA)) || isProbe == TRUE) {
		void *napi = pAd->tr_ctl.napi;
		PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
		struct sk_buff *skb = RTPKT_TO_OSPKT(pRxPacket);


		if (skb_headroom(skb) < extra_header_len) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_ERROR,
			"%s: headroom space(%d) not enough for click wifi header(%d)...(ERROR)\n",
			__func__, skb_headroom(skb), extra_header_len);
			return bProcessed;
		}

		skb_reset_tail_pointer(skb);
		skb_trim(skb, 0);
		skb_put(skb, pRxBlk->DataSize);

		/*append click os wifi header*/
		skb_push(skb, extra_header_len);
		pData = (UCHAR *)(skb->data);
		memset(pData, 0, extra_header_len);

		click_extra_hdr = (struct click_wifi_extra *)pData;

		/*fill parameters in extra header*/
		click_extra_hdr->magic = CLICK_WIFI_EXTRA_MAGIC;

		/*==Flags==*/
		/*bit0*/
		click_extra_hdr->ucFlags.fgTx = 0;

		/*bit1*/
		nss = pAd->Antenna.field.RxPath;
		if (nss == 4)
			click_extra_hdr->ucFlags.fgRx_4ss = 1;
		else
			click_extra_hdr->ucFlags.fgRx_4ss = 0;

		/*bit2*/
		switch (pRxBlk->rx_mode) {
		case MODE_HE:
		case MODE_EHT:
		case MODE_HE_SU:
		case MODE_HE_24G:
		case MODE_HE_EXT_SU:
		case MODE_HE_TRIG:
		case MODE_HE_MU:
		case MODE_EHT_ER_SU:
		case MODE_EHT_TB:
		case MODE_EHT_MU:
			click_extra_hdr->ucFlags.fgRx_he = 1;
			break;
		default:
			click_extra_hdr->ucFlags.fgRx_he = 0;
			break;
		}

		/*bit3, 0:no CRC error, 1:CRC error*/
		click_extra_hdr->ucFlags.fgRx_err = 0;

		/*bit4*/
		click_extra_hdr->ucFlags.fgRx_more = 0;

		/*bit5, fgAuto_rate is handled in click os. Fill in 0 here.*/
		click_extra_hdr->ucFlags.fgAuto_rate = 0;

		/*bit6*/
		click_extra_hdr->ucFlags.fgEncrypt = 0;

		/*bit7*/
		click_extra_hdr->ucFlags.fgLog = 0;

		/*==channel==*/
		click_extra_hdr->channel = pRxBlk->channel_freq;

		/*==keyidx== Ignored*/
		click_extra_hdr->keyidx = 0;

		/*==rssi==*/
		nss = pAd->Antenna.field.RxPath;

		for (ant_idx = 0; ant_idx < nss; ant_idx++) {
			cur_rssi = ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), ant_idx);
			rssi += cur_rssi;
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
			"%s: rssi_%d=%d, rssi_sum=%d\n",
			__func__, ant_idx, cur_rssi, rssi);
		}
		/*get average rssi*/
		rssi = rssi / nss;

		click_extra_hdr->rssi = rssi;

		/*==silence==*/
		/*get noise floor*/
		noise = pAd->noise_floor;

		if (noise + CLICK_NOISE_OFFSET < 0)
			silence = 0;
		else
			silence = noise + CLICK_NOISE_OFFSET;

		click_extra_hdr->silence = silence;

		/*==power==*/
		/*power = rssi - silence*/
		click_extra_hdr->power = rssi - silence;

		/*==retries==*/
		/* Tx retrys, optional or can be initialized to 0*/
		click_extra_hdr->retries = 0;

		/*==max_tries==*/
		/* Tx Max retrys, optional or can be initialized to 0 */
		for (i = 0; i < CLICK_WIFI_SCHEDULE_SIZE; i++)
			click_extra_hdr->max_tries[i] = 0;

		/*==rate==*/
		/*Todo, need to migrate radiotap code design*/
		/*legacy mode*/

		/*non-legacy(ht/vht/he/be) mode*/
		click_extra_hdr->rates[0] = 0;
		click_extra_hdr->rates[1] = 0;

		/*==phymode==*/
		click_extra_hdr->phymode = pRxBlk->rx_mode;

		/*==unused==*/
		for (i = 0; i < 3; i++)
			click_extra_hdr->unused[i] = 0;

		/*==rssi_per_ant==*/
		for (i = 0; i < CLICK_WIFI_MAX_ANTENNA; i++) {
			if (i < nss)
				click_extra_hdr->rssi_per_antenna[i] =
					ConvertToRssi(pAd, (struct raw_rssi_info *)(&pRxBlk->rx_signal.raw_rssi[0]), i);
			else
				click_extra_hdr->rssi_per_antenna[i] = 0;
		}

		/*==band==*/
			click_extra_hdr->band_id = pRxBlk->band;


		/*update stat counter*/
		net_dev->stats.rx_bytes += skb->len;
		net_dev->stats.rx_packets++;

/*Debug Dump*/
		/*check the extra_header content.*/
		//hex_dump_with_lvl("skb->data:", skb->data, 25, DBG_LVL_ERROR);
		//hex_dump_with_lvl("skb->data + extra_wifi_len:",
		//skb->data + extra_header_len, 20, DBG_LVL_ERROR);
/*End of Debug Dump*/

		skb->dev = pAd->act_mon_wdev.if_dev;

		skb_reset_mac_header(skb);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->pkt_type = PACKET_OTHERHOST;
		skb->protocol = htons(ETH_P_802_2);

		if (napi && in_serving_softirq())
			status = napi_gro_receive((struct napi_struct *)napi, skb);
		else
			status = netif_rx(skb);

		MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_INFO,
			"%s:rx status = %d, mon dev name:%s\n",	__func__, status, skb->dev->name);

		bProcessed = TRUE;
	} else {
		if (pWdev)
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN,
				"%s:Invalid wdev, rx dev name=%s, type=%d\n", __func__, pWdev->if_dev->name, pWdev->wdev_type);
		else
			MTWF_DBG(pAd, DBG_CAT_MLME, CATMLME_WTBL, DBG_LVL_WARN,
				"%s:wdev not found, RA="MACSTR"\n", __func__, MAC2STR(pRxBlk->Addr1));
	}
	return bProcessed;
}
#endif

VOID RTMPFreeAdapter(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	struct _PCI_HIF_T *hif;
	int i;
	struct physical_device *ph_dev = pAd->physical_dev;

	MlmeQueueDestroy(&pAd->Mlme);
	hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	os_cookie = (POS_COOKIE)pAd->OS_Cookie;
#ifdef RLM_CAL_CACHE_SUPPORT
	rlmCalCacheDeinit(&pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */
#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT

	/* only if in MULTIPLE_CARD the eebuf be allocated not static */
	if (pAd->eebuf  /*&& (pAd->eebuf != pAd->chipCap.EEPROM_DEFAULT_BIN)*/) {
		os_free_mem(pAd->eebuf);
		pAd->eebuf = NULL;
	}

#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT

	if (pAd->ApCfg.bMACRepeaterEn == TRUE)
		AsicSetReptFuncEnable(pAd, FALSE, hc_get_hw_band_idx(pAd));

	NdisFreeSpinLock(&pAd->ApCfg.CliLinkMapLock);
	NdisFreeSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	NdisFreeSpinLock(&pAd->ApCfg.btwt_ie_lock);
#endif
#endif
#endif
	NdisFreeSpinLock(&pAd->BssInfoIdxBitMapLock);
	NdisFreeSpinLock(&pAd->WdevListLock);

#ifdef UAPSD_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock);
#endif /* UAPSD_SUPPORT */
	for (i = 0; i < CFG_WIFI_RAM_BAND_NUM; i++)
		NdisFreeSpinLock(&pAd->mpdu_blk_pool[i].lock);
#ifdef GREENAP_SUPPORT
	NdisFreeSpinLock(&pAd->ApCfg.greenap.lock);
#endif /* GREENAP_SUPPORT */
#if defined(MTK_HOSTAPD_SUPPORT) && defined(CONFIG_MAP_SUPPORT) && defined(MAP_R3)
	NdisFreeSpinLock(&pAd->gas_frame_list_lock);
#endif /* MTK_HOSTAPD_SUPPORT && CONFIG_MAP_SUPPORT && MAP_R3 */

	if (pAd->iw_stats) {
		os_free_mem(pAd->iw_stats);
		pAd->iw_stats = NULL;
	}

	if (pAd->stats) {
		os_free_mem(pAd->stats);
		pAd->stats = NULL;
	}
#ifdef WIFI_DIAG
	diag_ctrl_free(pAd);
#endif
#ifdef MT_MAC
#ifdef CONFIG_AP_SUPPORT

	if ((IS_HIF_TYPE(pAd, HIF_MT)) && (pAd->OpMode == OPMODE_AP)) {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		ASSERT(pMbss);

		if (pMbss)
			bcn_buf_deinit(pAd, &pMbss->wdev);
		else {
			MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR,
				"func_dev is NULL!\n");
			return;
		}
	}

#endif
#endif
	RTMP_OS_FREE_TIMER(pAd);
	RTMP_OS_FREE_LOCK(pAd);
	RTMP_OS_FREE_TASKLET(pAd);
	RTMP_OS_FREE_TASK(pAd);
	RTMP_OS_FREE_SEM(pAd);
	RTMP_OS_FREE_ATOMIC(pAd);
	/*remove chipcap*/
	hif_core_ops_unregister(pAd->hdev_ctrl, pAd->infType);
	hdev_ctrl_exit(pAd);
	physical_device_remove_mac_adapter(ph_dev->hw_dev, pAd);
	RtmpOsVfree(pAd);

	if (os_cookie)
		os_free_mem(os_cookie);
}


#ifdef ETH_CONVERT_SUPPORT
static inline BOOLEAN rtmp_clone_mac_snoop(
	RTMP_ADAPTER *pAd,
	UCHAR *pkt,
	RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	ETH_CONVERT_STRUCT *eth_convert;
	UCHAR *pData = GET_OS_PKT_DATAPTR(pkt);
	BOOLEAN valid = TRUE;

	eth_convert = &pAd->EthConvert;

	/* Don't move this checking into wdev_tx_pkts(), because the net_device is OS-depeneded. */
	if ((eth_convert->ECMode & ETH_CONVERT_MODE_CLONE)
		&& (!eth_convert->CloneMacVaild)
		&& (eth_convert->macAutoLearn)
		&& (!(pData[6] & 0x1))) {
		VOID *pNetDev = Func(pAd->net_dev, pData);

		if (!pNetDev) {
			os_move_mem(&eth_convert->EthCloneMac[0], &pData[6], MAC_ADDR_LEN);
			eth_convert->CloneMacVaild = TRUE;
		}
	}

	/* Drop pkt since we are in pure clone mode and the src is not the cloned mac address. */
	if ((eth_convert->ECMode == ETH_CONVERT_MODE_CLONE)
		&& (os_equal_mem(pAd->CurrentAddress, &pData[6], MAC_ADDR_LEN) == FALSE))
		valid = FALSE;

	return valid;
}
#endif /* ETH_CONVERT_SUPPORT */


int RTMPSendPackets(
	IN NDIS_HANDLE dev_hnd,
	IN PPNDIS_PACKET pkt_list,
	IN UINT pkt_cnt,
	IN UINT32 pkt_total_len,
	IN RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	struct wifi_dev *wdev = (struct wifi_dev *)dev_hnd;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket = pkt_list[0];
	struct tr_counter *tr_cnt = NULL;

	if (!wdev->sys_handle) {
		ASSERT(wdev->sys_handle);
		return 0;
	}

	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	INC_COUNTER64(pAd->WlanCounters.TransmitCountFrmOs);

#ifdef IXIA_C50_MODE
	if (IS_EXPECTED_LENGTH(pAd, pkt_total_len)) {
		pAd->tx_cnt.txpktdetect++;
		pAd->tx_cnt.tx_pkt_from_os++;

		if (pAd->tx_cnt.tx_pkt_len != pkt_total_len)
			pAd->tx_cnt.tx_pkt_len = pkt_total_len;
	}
#endif

	if (!pPacket)
		return 0;

	/* drop zero-length packet to avoid tx hang */
	if (pkt_total_len <= 14) {
		tr_cnt = &pAd->tr_ctl.tr_cnt;
		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pPacket), pkt_total_len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		tr_cnt->pkt_len_invalid++;
		return 0;
	}

#ifdef VERIFICATION_MODE
	if (pAd->veri_ctrl.verify_mode_on == VERIFY_ON) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}
#endif

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT

	if (wdev->wdev_type == WDEV_TYPE_STA
		|| wdev->wdev_type == WDEV_TYPE_GC) {
		if (rtmp_clone_mac_snoop(pAd, pPacket, Func) == FALSE) {
			/* Drop pkt since we are in pure clone mode
			 * and the src is not the cloned mac address.
			 */
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}
	}

#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	skb_orphan(pPacket);
	return wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) & pPacket, 1, wdev);
}

#ifdef CONFIG_STA_SUPPORT
/*
 * ========================================================================
 * Routine Description:
 *	Driver pre-Ioctl for STA.
 *
 * Arguments:
 *	pAdSrc			    - WLAN control block pointer
 *	pParm				- the IOCTL parameters
 *
 * Return Value:
 *	NDIS_STATUS_SUCCESS	- IOCTL OK
 *	Otherwise			- IOCTL fail
 *
 * ========================================================================
 */
INT RTMP_STA_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pParm)
{
	RT_CMD_IOCTL_INTERFACE_CONFIG *pConfig = (RT_CMD_IOCTL_INTERFACE_CONFIG *)pParm;
	POS_COOKIE pObj;
	USHORT index;
	INT	Status = NDIS_STATUS_SUCCESS;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* determine this ioctl command is coming from which interface. */
	if (pConfig->priv_flags == INT_MAIN) {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MSTA_ID;
	} else if (pConfig->priv_flags == INT_MSTA) {
		pObj->ioctl_if_type = INT_MSTA;

		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) { /* sample */
			for (index = 0; index < MAX_MULTI_STA; index++) {
				if (pAd->StaCfg[index].wdev.if_dev == pConfig->net_dev) {
					pObj->ioctl_if = index;
					break;
				}
			}

			/* Interface not found! */
			if (index == MAX_MULTI_STA)
				return -ENETDOWN;
		} else  /* ioctl command from I/F(ra0) */
			pObj->ioctl_if = MAIN_MSTA_ID;
	} else if (pConfig->priv_flags == INT_APCLI) {
		pObj->ioctl_if_type = INT_APCLI;
		pObj->ioctl_if = MAIN_MSTA_ID;
	} else
		return -EOPNOTSUPP;

	pConfig->interface_idx = pObj->ioctl_if;
	return Status;
}

#endif

#ifdef CONFIG_AP_SUPPORT
/*
 * ========================================================================
 * Routine Description:
 *	Driver pre-Ioctl for AP.
 *
 * Arguments:
 *	pAdSrc			- WLAN control block pointer
 *	pCB				- the IOCTL parameters
 *
 * Return Value:
 *	NDIS_STATUS_SUCCESS	- IOCTL OK
 *	Otherwise			- IOCTL fail
 * ========================================================================
 */
INT RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pCB)
{
	RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pCB;
	POS_COOKIE pObj;
	USHORT index;
	INT	Status = NDIS_STATUS_SUCCESS;
	RTMP_STRING *buffer = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if ((pConfig->priv_flags == INT_MAIN) && !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		if (pConfig->pCmdData == NULL)
			return Status;

		if (RtPrivIoctlSetVal() == pConfig->CmdId_RTPRIV_IOCTL_SET) {
			os_alloc_mem(pAd, (UCHAR **)&buffer, pConfig->cmd_data_len + 1);

			if (buffer == NULL)
				return -ENOMEM;

			if (copy_from_user(buffer, pConfig->pCmdData, pConfig->cmd_data_len))
				Status = -EFAULT;
			/* Play safe -
			 * take care of a situation in which user-space didn't NULL terminate
			 */
			buffer[pConfig->cmd_data_len] = 0;
			if (TRUE
#ifdef CONFIG_APSTA_MIXED_SUPPORT
				&& ((pConfig->cmd_data_len <= (strlen("OpMode") + 1))
					&& (strstr(buffer, "OpMode") == NULL))
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef SINGLE_SKU
				&& (strstr(buffer, "ModuleTxpower") == NULL)
#endif /* SINGLE_SKU */
			) {
				Status = -ENETDOWN;
			}

			os_free_mem(buffer);
			if (Status != NDIS_STATUS_SUCCESS)
				return Status;
		} else
			return -ENETDOWN;
	}

	pObj->pSecConfig = NULL;

	/* determine this ioctl command is coming from which interface. */
	if (pConfig->priv_flags == INT_MAIN) {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
		pObj->pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;
	} else if (pConfig->priv_flags == INT_MBSSID) {
		pObj->ioctl_if_type = INT_MBSSID;

		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) { /* sample */
			for (index = 1; index < pAd->ApCfg.BssidNum; index++) {
				if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pConfig->net_dev) {
					pObj->ioctl_if = index;
					pObj->pSecConfig = &pAd->ApCfg.MBSSID[index].wdev.SecConfig;
					break;
				}
#ifdef CONFIG_VLAN_GTK_SUPPORT
				else if (CFG80211_MatchVlandev(&pAd->ApCfg.MBSSID[index].wdev, pConfig->net_dev)) {
					pObj->ioctl_if = index;
					pObj->pSecConfig = &pAd->ApCfg.MBSSID[index].wdev.SecConfig;
					break;
				}
#endif
			}

			/* Interface not found! */
			if (index == pAd->ApCfg.BssidNum)
				return -ENETDOWN;
		} else  /* ioctl command from I/F(ra0) */
			pObj->ioctl_if = MAIN_MBSSID;

		MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
	}

#ifdef WDS_SUPPORT
	else if (pConfig->priv_flags == INT_WDS) {
		pObj->ioctl_if_type = INT_WDS;

		for (index = 0; index < MAX_WDS_ENTRY; index++) {
			if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pConfig->net_dev) {
				pObj->ioctl_if = index;
				pObj->pSecConfig = &pAd->WdsTab.WdsEntry[index].wdev.SecConfig;
				break;
			}

			if (index == MAX_WDS_ENTRY) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"can not find wds I/F\n");
				return -ENETDOWN;
			}
		}
	}

#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
	else if (pConfig->priv_flags == INT_APCLI) {
		pObj->ioctl_if_type = INT_APCLI;

		for (index = 0; index < MAX_APCLI_NUM; index++) {
			if (pAd->StaCfg[index].wdev.if_dev == pConfig->net_dev) {
				pObj->ioctl_if = index;
				pObj->pSecConfig = &pAd->StaCfg[index].wdev.SecConfig;
				break;
			}

			if (index == MAX_APCLI_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
					"can not find Apcli I/F\n");
				return -ENETDOWN;
			}
		}

		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}

#endif /* APCLI_SUPPORT */
	else
		return -EOPNOTSUPP;

	pConfig->apidx = pObj->ioctl_if;
	return Status;
}


VOID AP_E2PROM_IOCTL_PostCtrl(
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	RTMP_STRING *msg)
{
	UCHAR *p = NULL;
	UINT32 offset = 0, len = 0;

	wrq->u.data.length = strlen(msg);

	if (wrq->u.data.length >= 4095) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"length more than iwpriv buffer size 4096");
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			", change from length:%d to 4095\n  print from driver directly\n\n",
			wrq->u.data.length);
		p = msg;
		len = wrq->u.data.length;

		while (len) {
			MTWF_PRINT("%c", p[offset]);
			len--;
			offset++;
		}
		wrq->u.data.length = 4095;
	} else if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_INFO,
			"%s: copy_to_user() fail, length:%d\n",
			__func__, wrq->u.data.length);
}


VOID IAPP_L2_UpdatePostCtrl(RTMP_ADAPTER *pAd, UINT8 *mac_p, INT wdev_idx)
{
}
#endif /* CONFIG_AP_SUPPORT */

void RTMPSetPacketDestType(PNDIS_PACKET pPacket)
{
	struct sk_buff *skb = RTPKT_TO_OSPKT(pPacket);
	struct net_device *net_dev = skb->dev;
	struct ethhdr *eth = (struct ethhdr *)skb->data;

	if (unlikely(eth->h_dest[0] & 0x1)) {
		if (ether_addr_equal(eth->h_dest, net_dev->broadcast))
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	}
}

void RTMPSetPacketProtocol(
	PNDIS_PACKET pPacket,
	UCHAR *pHeader,
	UINT HeaderLen)
{
	struct sk_buff *skb = RTPKT_TO_OSPKT(pPacket);
	struct ethhdr *eth = (struct ethhdr *)pHeader;

	if (!skb || !eth || HeaderLen < ETH_HLEN)
		return;

	skb->protocol = eth->h_proto;
}

