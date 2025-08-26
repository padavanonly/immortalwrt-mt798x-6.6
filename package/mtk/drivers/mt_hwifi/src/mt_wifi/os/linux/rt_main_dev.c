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
 ***************************************************************************

    Module Name:
    rt_main_dev.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/ethtool.h>
/*---------------------------------------------------------------------*/
/* Private Variables Used                                              */
/*---------------------------------------------------------------------*/

RTMP_STRING *mac = "";		/* default 00:00:00:00:00:00 */
RTMP_STRING *mode = "";		/* supported mode: normal/ate/monitor;  default: normal */
RTMP_STRING *hostname = "";	/* default CMPC */

#if (KERNEL_VERSION(2, 6, 12) >= LINUX_VERSION_CODE)
MODULE_PARM(mac, "s");
MODULE_PARM(mode, "s");
#else
module_param(mac, charp, 0);
module_param(mode, charp, 0);
#endif
MODULE_PARM_DESC(mac, "rt_wifi: wireless mac addr");
MODULE_PARM_DESC(mode, "rt_wifi: wireless operation mode");

#if !defined(PROPRIETARY_DRIVER_SUPPORT) || defined(CONFIG_DBG_OOM)
MODULE_LICENSE("GPL");
#else
MODULE_LICENSE("Proprietary");
#endif

/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        */
/*---------------------------------------------------------------------*/

/* public function prototype */
int mt_wifi_close(VOID *net_dev);
int mt_wifi_open(VOID *net_dev);
int virtual_if_up_handler(VOID *dev);
int virtual_if_down_handler(VOID *dev);

/* private function prototype */
INT rt28xx_send_packets(IN struct sk_buff *skb_p, IN struct net_device *net_dev);

struct net_device_stats *RT28xx_get_ether_stats(struct net_device *net_dev);

#if ((KERNEL_VERSION(2, 4, 23) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(4, 20, 0) > LINUX_VERSION_CODE))
static int rt_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct iwreq req;

	NdisZeroMemory(&req, sizeof(req));
	rt28xx_ioctl(dev, (struct ifreq *)&req, SIOCGIWRATE);
	cmd->speed = req.u.bitrate.value/1000000; /* The speed is Mbit/s */

	return 0;
}

static const struct ethtool_ops rt_ethtool_ops = {
	.get_settings = rt_get_settings,
};
#endif

/*
 * ========================================================================
 * Routine Description:
 *    Close raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *    0					Open OK
 *	otherwise			Open Fail
 *
 * Note:
 *	1. if open fail, kernel will not call the close function.
 *	2. Free memory for
 *		(1) Mlme Memory Handler:		MlmeHalt()
 *		(2) TX & RX:					RTMPFreeTxRxRingMemory()
 *		(3) BA Reordering:				ba_reordering_resource_release()
 * ========================================================================
 */
int main_virtual_if_close(IN struct net_device *net_dev)
{
	VOID *pAd = NULL;

	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(net_dev));

	pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);

	if (pAd == NULL)
		return 0;

	RTMP_OS_NETDEV_CARRIER_OFF(net_dev);
	RTMP_OS_NETDEV_STOP_QUEUE(net_dev);

#ifdef IFUP_IN_PROBE
#else
	VIRTUAL_IF_DOWN(pAd, net_dev);
#endif /* IFUP_IN_PROBE */

	VIRTUAL_IF_DEINIT(pAd, net_dev);

	RT_MOD_HNAT_DEREG(net_dev);
	RT_MOD_DEC_USE_COUNT();
	return 0; /* close ok */
}

/*
 * ========================================================================
 * Routine Description:
 *   Open raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *   0					Open OK
 *	otherwise			Open Fail
 *
 * Note:
 *	1. if open fail, kernel will not call the close function.
 *	2. Free memory for
 *		(1) Mlme Memory Handler:		MlmeHalt()
 *		(2) TX & RX:					RTMPFreeTxRxRingMemory()
 *		(3) BA Reordering:				ba_reordering_resource_release()
 * ========================================================================
 */
int main_virtual_if_open(struct net_device *net_dev)
{
	VOID *pAd = NULL;

	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(net_dev));

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL)
		return 0;

#ifdef IFUP_IN_PROBE

	while (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS) {
		OS_WAIT(10);
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			"Card not ready, NDIS_STATUS_SUCCESS!\n");
	}

#else

	if (VIRTUAL_IF_INIT(pAd, net_dev) != 0)
		return -1;

	if (VIRTUAL_IF_UP(pAd, net_dev) != 0)
		return -1;

#endif /* IFUP_IN_PROBE */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(net_dev);
	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *   Close raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *    0					Open OK
 *	otherwise			Open Fail
 *
 * Note:
 *	1. if open fail, kernel will not call the close function.
 *	2. Free memory for
 *		(1) Mlme Memory Handler:		MlmeHalt()
 *		(2) TX & RX:					RTMPFreeTxRxRingMemory()
 *		(3) BA Reordering:				ba_reordering_resource_release()
 * ========================================================================
 */
int mt_wifi_close(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	VOID	*pAd = NULL;

	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "===>\n");
	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL)
		return 0;

#ifdef CFG_SUPPORT_CSI
	csi_support_deinit(pAd);
#endif


	RTMPDrvClose(pAd, net_dev);

#ifdef CONFIG_WLAN_SERVICE
	mt_service_close(pAd);
#endif /* CONFIG_WLAN_SERVICE */

#ifdef WIFI_MD_COEX_SUPPORT
	deinit_wifi_md_coex(pAd);
#endif /* WIFI_MD_COEX_SUPPORT */

	/*system down hook point*/
	WLAN_HOOK_CALL(WLAN_HOOK_SYS_DOWN, pAd, NULL);

	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO, "<===\n");
	return 0;
}

/*
 * ========================================================================
 * Routine Description:
 *   Open raxx interface.
 *
 * Arguments:
 *	*net_dev			the raxx interface pointer
 *
 * Return Value:
 *    0					Open OK
 *	otherwise			Open Fail
 * ========================================================================
 */
int mt_wifi_open(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	VOID *pAd = NULL;
	int retval = 0;
	UINT32 OpMode;
#ifdef WARP_512_SUPPORT
	UINT8 Enable;
#endif

	if (sizeof(ra_dma_addr_t) < sizeof(dma_addr_t))
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
			"Fatal error for DMA address size!!!\n");

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		/* if 1st open fail, pAd will be free; */
		/*   So the net_dev->priv will be NULL in 2rd open */
		return -1;
	}

	RTMP_DRIVER_MCU_SLEEP_CLEAR(pAd);
	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
#if WIRELESS_EXT >= 12

	/*	if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_MAIN) */
	if (RTMP_DRIVER_MAIN_INF_CHECK(pAd, RT_DEV_PRIV_FLAGS_GET(net_dev)) == NDIS_STATUS_SUCCESS) {
#ifdef CONFIG_APSTA_MIXED_SUPPORT
#ifdef CONFIG_WIRELESS_EXT
		if (OpMode == OPMODE_AP)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_ap_iw_handler_def;
#endif /* CONFIG_WIRELESS_EXT */

#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_WIRELESS_EXT

		if (OpMode == OPMODE_STA)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_iw_handler_def;
#endif /* CONFIG_WIRELESS_EXT */
#endif /* CONFIG_STA_SUPPORT */
	}

#endif /* WIRELESS_EXT >= 12 */
	/*system up hook point should before interrupt register*/
#ifndef CONFIG_PROFILE_OFF
#if defined(WHNAT_SUPPORT) || defined(MT7986) || defined(MT7916) || defined(MT7981)
	RTMPPreReadProfile(pAd);
#endif /* defined(WHNAT_SUPPORT) || defined(MT7986) || defined(MT7916) || defined(MT7981) */
#endif /* !CONFIG_PROFILE_OFF */
	WLAN_HOOK_CALL(WLAN_HOOK_SYS_UP, pAd, NULL);

#ifdef CONFIG_WLAN_SERVICE
	mt_service_open(pAd);
#endif /* CONFIG_WLAN_SERVICE */

#ifdef WARP_512_SUPPORT
	Enable = TRUE;
	WLAN_HOOK_CALL(WLAN_HOOK_WARP_512_SUPPORT, pAd, &Enable);
	if (((RTMP_ADAPTER *)pAd)->physical_dev.whnat_en) {
		if (Enable)
			((RTMP_ADAPTER *)pAd)->Warp512Support = TRUE;
		else
			((RTMP_ADAPTER *)pAd)->Warp512Support = FALSE;
	}
#endif

	/* Chip & other init */
	if (mt_wifi_init(pAd, mac, hostname) == FALSE)
		goto err;
#ifdef MBSS_SUPPORT
	/*
	 *	the function can not be moved to RT2860_probe() even register_netdev()
	 *	is changed as register_netdevice().
	 *	Or in some PC, kernel will panic (Fedora 4)
	 */
	RT_CONFIG_IF_OPMODE_ON_AP(GET_OPMODE_FROM_PAD(pAd))
	{
		RT28xx_MBSS_Init(pAd, net_dev);
	}
#endif /* MBSS_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	{
#ifndef IWCOMMAND_CFG80211_SUPPORT
		RT28xx_MSTA_Init(pAd, net_dev);
#endif /* IWCOMMAND_CFG80211_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef SNIFFER_SUPPORT
	RT28xx_Monitor_Init(pAd, net_dev);
#endif /* SNIFFER_SUPPORT */
#ifdef ACTION_MONITOR_SUPPORT
	RT28xx_Action_Mon_Init(pAd, net_dev);
#endif
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RTMP_DRIVER_CFG80211_START(pAd);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef CONFIG_WLAN_SERVICE
	mt_service_init(pAd);
#endif /* CONFIG_WLAN_SERVICE */

	RTMPDrvOpen(pAd);
#ifdef VENDOR_FEATURE2_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_DEBUG,
			 "Number of Packet Allocated in open = %lu\n", OS_NumOfPktAlloc);
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_DEBUG,
			 "Number of Packet Freed in open = %lu\n", OS_NumOfPktFree);
#endif /* VENDOR_FEATURE2_SUPPORT */


#ifdef WIFI_MD_COEX_SUPPORT
	init_wifi_md_coex(pAd);
#endif /* WIFI_MD_COEX_SUPPORT */

	return retval;
err:
	return -1;
}

int virtual_if_up_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;
	struct wifi_dev *wdev = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INIT_FAIL)) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "init fail!!!\n");
		retval = -1;
		return retval;
	}

	if (physical_device_get_band_en(pAd->physical_dev, hc_get_hw_band_idx(pAd)) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "disabled interface\n");
		retval = -1;
		return retval;
	}

	wdev = wdev_search_by_netdev(pAd, net_dev);
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR, "wdev fail!!!\n");
		retval = -1;
		return retval;
	}

	wdev_if_up_down(pAd, wdev, TRUE);

#ifdef GREENAP_SUPPORT
		/* This function will check and update allow status */
		if (greenap_check_when_if_down_up(pAd) == FALSE)
			return retval;
#endif /* GREENAP_SUPPORT */

	if (VIRTUAL_IF_NUM(pAd) != 0) {
		if (wdev_do_open(wdev) != TRUE) {
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
			wdev_if_up_down(pAd, wdev, FALSE);
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				"inf_up (idx %d) fail!!!\n",
				wdev->wdev_idx);
			retval = -1;
			return retval;
		}
	}

#ifdef WDS_SUPPORT
	if ((wdev->wdev_type == WDEV_TYPE_AP) && (wdev->func_idx == 0)) {
		RT28xx_WDS_Init(pAd, hc_get_hw_band_idx(pAd), dev);
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_DEBUG,
			"wds_if bound to main dev!\n");
	}
#endif	/* WDS_SUPPORT */

#ifdef CONFIG_6G_AFC_SUPPORT
		if (is_afc_stop() == TRUE)
			afc_initiate_request_to_daemon(pAd, wdev);
#endif /*CONFIG_6G_AFC_SUPPORT*/


	return retval;
}

int virtual_if_down_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;
	struct wifi_dev *wdev = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}

	wdev = wdev_search_by_netdev(pAd, net_dev);
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "wdev fail!!!\n");
		retval = -1;
		return retval;
	}

/*wdev->if_up_down_state should be mark false after wdev_do_close*/
	if (wdev_do_close(wdev) != TRUE) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO,
			"inf_down (idx %d) fail!!!\n",
			wdev->wdev_idx);
	}
	wdev_if_up_down(pAd, wdev, FALSE);

#ifdef GREENAP_SUPPORT
	greenap_check_when_if_down_up(pAd);
#endif /* GREENAP_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
	pAd->cfg80211_ctrl.beaconIsSetFromHostapd = FALSE;
#endif

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF, HcGetBandByWdev(wdev));
#endif /* LED_CONTROL_SUPPORT */

	return retval;
}


INT virtual_if_init_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}
#if ((KERNEL_VERSION(2, 4, 23) <= LINUX_VERSION_CODE) && (KERNEL_VERSION(4, 20, 0) > LINUX_VERSION_CODE))
	net_dev->ethtool_ops = &rt_ethtool_ops;
#endif

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		VIRTUAL_IF_INC(pAd);

#ifdef MT_DFS_SUPPORT
		/* Update bInitMbssZeroWait  */
		UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, TRUE);
#endif /* MT_DFS_SUPPORT */

		/* must use main net_dev */
		if (mt_wifi_open(pAd->net_dev) != 0) {
			VIRTUAL_IF_DEC(pAd);
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
					 "mt_wifi_open return fail!\n");
			return NDIS_STATUS_FAILURE;
		}
	} else
		VIRTUAL_IF_INC(pAd);

	return retval;
}

INT virtual_if_deinit_handler(VOID *dev)
{
	struct net_device *net_dev = (struct net_device *)dev;
	struct _RTMP_ADAPTER *pAd = NULL;
	int retval = 0;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		retval = -1;
		return retval;
	}

	if (VIRTUAL_IF_NUM(pAd) == 0) {
		mt_wifi_close(pAd->net_dev);
	}

	return retval;
}

PNET_DEV RtmpPhyNetDevInit(VOID *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevHook)
{
	struct net_device *net_dev = NULL;
	ULONG InfId = 0;
	UINT32 OpMode;
#if defined(CONFIG_CSO_SUPPORT)
	UCHAR flg;
#endif /* defined(CONFIG_CSO_SUPPORT) */
	RTMP_DRIVER_MAIN_INF_GET(pAd, &InfId);
	/*	net_dev = RtmpOSNetDevCreate(pAd, INT_MAIN, 0, sizeof(struct mt_dev_priv), INF_MAIN_DEV_NAME); */
	RTMP_DRIVER_MAIN_INF_CREATE(pAd, &net_dev);

	if (net_dev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				 "main physical net device creation failed!\n");
		return NULL;
	}

	os_zero_mem((unsigned char *)pNetDevHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	pNetDevHook->open = main_virtual_if_open;
	pNetDevHook->stop = main_virtual_if_close;
	pNetDevHook->xmit = rt28xx_send_packets;
#ifdef IKANOS_VX_1X0
	pNetDevHook->xmit = IKANOS_DataFramesTx;
#endif /* IKANOS_VX_1X0 */
	pNetDevHook->ioctl = rt28xx_ioctl;
	pNetDevHook->priv_flags = InfId; /*INT_MAIN; */
	pNetDevHook->get_stats = RT28xx_get_ether_stats;
	pNetDevHook->needProtcted = FALSE;
#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
	pNetDevHook->get_wstats = rt28xx_get_wireless_stats;
#endif
#ifdef HOSTAPD_MBSS_SUPPORT
	pNetDevHook->set_mac_addr = rt28xx_change_mac;
#endif /*HOSTAPD_MBSS_SUPPORT*/
	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	/* put private data structure */
	RTMP_OS_NETDEV_SET_PRIV(net_dev, pAd);

	/* double-check if pAd is associated with the net_dev */
	if (RTMP_OS_NETDEV_GET_PRIV(net_dev) == NULL) {
		RtmpOSNetDevFree(net_dev);
		return NULL;
	}

	RTMP_DRIVER_NET_DEV_SET(pAd, net_dev);
#if (KERNEL_VERSION(2, 6, 24) > LINUX_VERSION_CODE)
	SET_MODULE_OWNER(net_dev);
#endif
#ifdef CONFIG_CSO_SUPPORT
	RTMP_DRIVER_ADAPTER_CSO_SUPPORT_TEST(pAd, &flg);

	if (flg)
		net_dev->features |= NETIF_F_HW_CSUM;

#endif /* CONFIG_CSO_SUPPORT */
	return net_dev;
}


VOID *RtmpNetEthConvertDevSearch(VOID *net_dev_, UCHAR *pData)
{
	struct net_device *pNetDev;
#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 26) <= LINUX_VERSION_CODE)
	struct net_device *net_dev = (struct net_device *)net_dev_;
	struct net *net;

	net = dev_net(net_dev);
	BUG_ON(!net);
	for_each_netdev(net, pNetDev)
#else
	struct net *net;
	struct net_device *net_dev = (struct net_device *)net_dev_;

	BUG_ON(!net_dev->nd_net);
	net = net_dev->nd_net;
	for_each_netdev(net, pNetDev)
#endif
#else
#if (KERNEL_VERSION(2, 6, 22) <= LINUX_VERSION_CODE)
	for_each_netdev(pNetDev)
#else

	for (pNetDev = dev_base; pNetDev; pNetDev = pNetDev->next)
#endif
#endif
	{
		if ((pNetDev->type == ARPHRD_ETHER)
			&& ether_addr_equal(pNetDev->dev_addr, &pData[6]))
			break;
	}

	return (VOID *)pNetDev;
}



/*
 * ========================================================================
 * Routine Description:
 *   The entry point for Linux kernel sent packet to our driver.
 *
 * Arguments:
 *   sk_buff *skb	the pointer refer to a sk_buffer.
 *
 * Return Value:
 *    0
 *
 * Note:
 *	This function is the entry point of Tx Path for OS delivery packet to
 *	our driver. You only can put OS-depened & STA/AP common handle procedures
 *	in here.
 * ========================================================================
 */
int rt28xx_packet_xmit(void *pkt)
{
	struct sk_buff *skb = (struct sk_buff *)pkt;
	struct net_device *net_dev = skb->dev;
	struct wifi_dev *wdev;
	PNDIS_PACKET pPacket = (PNDIS_PACKET)skb;
	int status = 0;

	wdev = RTMP_OS_NETDEV_GET_WDEV(net_dev);
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR, "wdev is NULL!\n");
		ASSERT(wdev);
		return status;
	}


	status = RTMPSendPackets((NDIS_HANDLE)wdev, (PPNDIS_PACKET) & pPacket, 1,
						   skb->len, RtmpNetEthConvertDevSearch);
	return status;
}


/*
 * ========================================================================
 * Routine Description:
 *    Send a packet to WLAN.
 *
 * Arguments:
 *    skb_p           points to our adapter
 *    dev_p           which WLAN network interface
 *
 * Return Value:
 *   0: transmit successfully
 *   otherwise: transmit fail
 *
 * ========================================================================
 */
int rt28xx_send_packets(struct sk_buff *skb, struct net_device *ndev)
{
	os_zero_mem((PUCHAR)&skb->cb[CB_OFF], CB_LEN);
	WLAN_HOOK_CALL(WLAN_HOOK_TX_OS_TO_DRIVER, NULL, skb);

	if (!(RTMP_OS_NETDEV_STATE_RUNNING(ndev))) {
		RELEASE_NDIS_PACKET(NULL, (PNDIS_PACKET)skb, NDIS_STATUS_FAILURE);
		return 0;
	}

	MEM_DBG_PKT_ALLOC_INC(skb);
	return rt28xx_packet_xmit(skb);
}


#if WIRELESS_EXT >= 12
/* This function will be called when query /proc */
struct iw_statistics *rt28xx_get_wireless_stats(struct net_device *net_dev)
{
	VOID *pAd = NULL;
	struct iw_statistics *pStats;
	RT_CMD_IW_STATS DrvIwStats, *pDrvIwStats = &DrvIwStats;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_INFO, "--->\n");
	os_zero_mem(pDrvIwStats, sizeof(*pDrvIwStats));
	pDrvIwStats->priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pDrvIwStats->dev_addr = (PUCHAR)net_dev->dev_addr;

	if (RTMP_DRIVER_IW_STATS_GET(pAd, pDrvIwStats) != NDIS_STATUS_SUCCESS)
		return NULL;

	pStats = (struct iw_statistics *)(pDrvIwStats->pStats);
	pStats->status = 0; /* Status - device dependent for now */

	pStats->qual.updated = 1;     /* Flags to know if updated */
#ifdef IW_QUAL_DBM
	pStats->qual.updated |= IW_QUAL_DBM;	/* Level + Noise are dBm */
#endif /* IW_QUAL_DBM */
	pStats->qual.qual = pDrvIwStats->qual;
	pStats->qual.level = pDrvIwStats->level;
	pStats->qual.noise = pDrvIwStats->noise;
	pStats->discard.nwid = 0;     /* Rx : Wrong nwid/essid */
	pStats->miss.beacon = 0;      /* Missed beacons/superframe */
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_INFO, "<---\n");
	return pStats;
}
#endif /* WIRELESS_EXT */


INT rt28xx_ioctl(PNET_DEV net_dev, struct ifreq *rq, INT cmd)
{
	INT ret = 0;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *ops;
	VOID *pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	if (pAd == NULL)
		return -ENETDOWN;

	GET_WDEV_FROM_NET_DEV(wdev, net_dev);
	if (wdev == NULL)
		return -ENETDOWN;
	ops = wdev->wdev_ops;

	if (!ops)
		return -ENETDOWN;

	ASSERT(ops->ioctl);

	if (ops->ioctl)
		ret = ops->ioctl(net_dev, rq, cmd);
	else
		return -ENETDOWN;
	return ret;
}


/*
 * ========================================================================
 *
 *  Routine Description:
 *	return ethernet statistics counter
 *
 *  Arguments:
 *	net_dev                     Pointer to net_device
 *
 *  Return Value:
 *	net_device_stats*
 * ========================================================================
 */
struct net_device_stats *RT28xx_get_ether_stats(struct net_device *net_dev)
{
	VOID *pAd = NULL;
	struct net_device_stats *pStats;

	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd) {
		RT_CMD_STATS DrvStats, *pDrvStats = &DrvStats;
		/* assign net device for RTMP_DRIVER_INF_STATS_GET() */
		os_zero_mem(pDrvStats, sizeof(*pDrvStats));
		pDrvStats->pNetDev = net_dev;
		RTMP_DRIVER_INF_STATS_GET(pAd, pDrvStats);
		if (pDrvStats->pStats == NULL)
			return NULL;
		pStats = (struct net_device_stats *)(pDrvStats->pStats);
		pStats->rx_packets = pDrvStats->rx_packets;
		pStats->tx_packets = pDrvStats->tx_packets;
		pStats->rx_bytes = pDrvStats->rx_bytes;
		pStats->tx_bytes = pDrvStats->tx_bytes;
		pStats->rx_errors = pDrvStats->rx_errors;
		pStats->tx_errors = pDrvStats->tx_errors;
		pStats->rx_dropped = pDrvStats->rx_drop_count;
		pStats->tx_dropped = pDrvStats->tx_drop_count;
		pStats->multicast = pDrvStats->multicast;
		pStats->collisions = pDrvStats->collisions;
		pStats->rx_length_errors = 0;
		pStats->rx_over_errors = pDrvStats->rx_over_errors;
		pStats->rx_crc_errors = 0;/*pAd->WlanCounters.FCSErrorCount;     // recved pkt with crc error */
		pStats->rx_frame_errors = pDrvStats->rx_frame_errors;
		pStats->rx_fifo_errors = pDrvStats->rx_fifo_errors;
		pStats->rx_missed_errors = 0;                                            /* receiver missed packet */
		/* detailed tx_errors */
		pStats->tx_aborted_errors = 0;
		pStats->tx_carrier_errors = 0;
		pStats->tx_fifo_errors = 0;
		pStats->tx_heartbeat_errors = 0;
		pStats->tx_window_errors = 0;
		/* for cslip etc */
		pStats->rx_compressed = 0;
		pStats->tx_compressed = 0;
		return pStats;
	} else
		return NULL;
}

/*
 * ========================================================================
 *
 *  Routine Description:
 *	set mac address
 *
 *  Arguments:
 *	net_dev		Pointer to net_device
 *	addr		the mac addr need to set
 *  Return Value:
 *	0: success 1: failed
 * ========================================================================
 */
#ifdef HOSTAPD_MBSS_SUPPORT
int rt28xx_change_mac(struct net_device *dev, void *addr)
{
	VOID *pAd = NULL;
	struct wifi_dev *wdev = NULL;
	struct sockaddr *sa = addr;
	int ret;
	CHAR addr_str[18];

	if (!dev)
		return -EINVAL;

	GET_PAD_FROM_NET_DEV(pAd, dev);
	if (!pAd)
		return -EINVAL;

	wdev = RtmpOsGetNetDevWdev(dev);
	if (!wdev)
		return -EINVAL;

	if (wdev->if_up_down_state == TRUE)
		return  -EBUSY;

	ret = snprintf(addr_str, sizeof(addr_str), "%pM", sa->sa_data);
	if (ret != 17)
		return -EINVAL;

	ret = eth_prepare_mac_addr_change(dev, addr);
	if (ret < 0)
		return ret;

	if (wdev->wdev_type == WDEV_TYPE_AP) {
		ret = RT_CfgSetMacAddress(pAd, addr_str, wdev->func_idx, OPMODE_AP);
		if (ret != TRUE) {
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_ERROR,
				"RT_CfgSetMacAddress failed!\n");
			return  -EADDRNOTAVAIL;
		}

	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
		ret = RT_CfgSetMacAddress(pAd, addr_str, wdev->func_idx, OPMODE_STA);
		if (ret != TRUE) {
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_ERROR,
				"RT_CfgSetMacAddress failed!\n");
			return -EADDRNOTAVAIL;
		}
	}

	COPY_MAC_ADDR(wdev->if_addr, sa->sa_data);
	COPY_MAC_ADDR(wdev->bssid, sa->sa_data);

	eth_commit_mac_addr_change(dev, addr);

	return 0;
}
#endif /*HOSTAPD_MBSS_SUPPORT*/

BOOLEAN RtmpPhyNetDevExit(VOID *pAd, PNET_DEV net_dev)
{
	/*remove cfg */
	wpf_exit(pAd);
#ifdef CONFIG_AP_SUPPORT
#ifdef WDS_SUPPORT
	/* remove all WDS virtual interfaces. */
	RT28xx_WDS_Remove(pAd);
#endif /* WDS_SUPPORT */
#ifdef SNIFFER_SUPPORT
	RT28xx_Monitor_Remove(pAd);
#endif	/* SNIFFER_SUPPORT */
#ifdef ACTION_MONITOR_SUPPORT
	RT28xx_Action_Mon_Remove(pAd);
#endif
#ifdef MBSS_SUPPORT
	RT28xx_MBSS_Remove(pAd);
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	RT28xx_MSTA_Remove(pAd);
#endif
#ifdef INF_PPA_SUPPORT
	RTMP_DRIVER_INF_PPA_EXIT(pAd);
#endif /* INF_PPA_SUPPORT */

	/* Unregister network device */
	if (net_dev != NULL) {
		MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR,
				 "RtmpOSNetDevDetach(): RtmpOSNetDeviceDetach(), dev->name=%s!\n", net_dev->name);
		RtmpOSNetDevProtect(1);
		RtmpOSNetDevDetach(net_dev);
		RtmpOSNetDevProtect(0);
#ifdef RT_CFG80211_SUPPORT
		RTMP_DRIVER_80211_UNREGISTER(pAd, net_dev);
#endif /* RT_CFG80211_SUPPORT */
	}

	return TRUE;
}


#ifdef WDS_SUPPORT
/* ========================================================================
 *  Routine Description:
 *	return ethernet statistics counter
 *
 *  Arguments:
 *	net_dev                     Pointer to net_device
 *
 *  Return Value:
 *	net_device_stats*
 * ========================================================================
 */
struct net_device_stats *RT28xx_get_wds_ether_stats(
	IN PNET_DEV net_dev)
{
	VOID *pAd = NULL;
	/*	INT WDS_apidx = 0,index; */
	struct net_device_stats *pStats;
	RT_CMD_STATS WdsStats, *pWdsStats = &WdsStats;

	os_zero_mem(pWdsStats, sizeof(*pWdsStats));
	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	/*	if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_WDS) */
	{
		if (pAd) {

			pWdsStats->pNetDev = net_dev;

			if (RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_STATS_GET,
									 0, pWdsStats, RT_DEV_PRIV_FLAGS_GET(net_dev)) != NDIS_STATUS_SUCCESS)
				return NULL;

			pStats = (struct net_device_stats *)pWdsStats->pStats; /*pAd->stats; */
			pStats->rx_packets = pWdsStats->rx_packets; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.ReceivedFragmentCount.QuadPart; */
			pStats->tx_packets = pWdsStats->tx_packets; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TransmittedFragmentCount.QuadPart; */
			pStats->rx_bytes = pWdsStats->rx_bytes; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.ReceivedByteCount; */
			pStats->tx_bytes = pWdsStats->tx_bytes; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TransmittedByteCount; */
			pStats->rx_errors = pWdsStats->rx_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxErrorCount; */
			pStats->tx_errors = pWdsStats->tx_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.TxErrors; */
			pStats->rx_dropped = 0;
			pStats->tx_dropped = 0;
			pStats->multicast = pWdsStats->multicast; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.MulticastReceivedFrameCount.QuadPart;   // multicast packets received */
			pStats->collisions = pWdsStats->collisions; /* Collision packets */
			pStats->rx_length_errors = 0;
			pStats->rx_over_errors = pWdsStats->rx_over_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxNoBuffer;                   // receiver ring buff overflow */
			pStats->rx_crc_errors = 0;/*pAd->WlanCounters.FCSErrorCount;     // recved pkt with crc error */
			pStats->rx_frame_errors = 0; /* recv'd frame alignment error */
			pStats->rx_fifo_errors = pWdsStats->rx_fifo_errors; /*pAd->WdsTab.WdsEntry[WDS_apidx].WdsCounter.RxNoBuffer;                   // recv'r fifo overrun */
			pStats->rx_missed_errors = 0;                                            /* receiver missed packet */
			/* detailed tx_errors */
			pStats->tx_aborted_errors = 0;
			pStats->tx_carrier_errors = 0;
			pStats->tx_fifo_errors = 0;
			pStats->tx_heartbeat_errors = 0;
			pStats->tx_window_errors = 0;
			/* for cslip etc */
			pStats->rx_compressed = 0;
			pStats->tx_compressed = 0;
			return pStats;
		} else
			return NULL;
	}
}
#endif /* WDS_SUPPORT */

