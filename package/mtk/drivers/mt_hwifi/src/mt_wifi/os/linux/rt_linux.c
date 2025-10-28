/****************************************************************************
 *
 *   Module Name:
 *    rt_linux.c
 *
 *    Abstract:
 *	All functions provided from OS module are put here.
 *
 *	Revision History:
 *	Who        When          What
 *	---------  ----------    -------------------------------------------
 *
 ***************************************************************************/

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "security/dot11i_wpa.h"
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
#include <linux/oom.h>
#include <linux/netdevice.h>
#include <linux/mm.h>
#include <linux/preempt.h>
#include <net/sch_generic.h>
#include "rt_os_net.h"
#include "rt_config.h"
#ifdef MEM_ALLOC_INFO_SUPPORT
#include "meminfo_list.h"
#endif /* MEM_ALLOC_INFO_SUPPORT */

#ifdef CONFIG_FAST_NAT_SUPPORT
#include <net/ra_nat.h>
#endif /*CONFIG_FAST_NAT_SUPPORT*/
#include "multi_hif.h"

#ifdef VLAN_SUPPORT
#include <linux/if_vlan.h>
#endif /*VLAN_SUPPORT*/
#include "wnm.h"

/* TODO */
#undef RT_CONFIG_IF_OPMODE_ON_AP
#undef RT_CONFIG_IF_OPMODE_ON_STA

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)	if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif

#ifdef CONFIG_CONNINFRA_SUPPORT
enum consys_drv_type {
	CONNDRV_TYPE_BT = 0,
	CONNDRV_TYPE_FM = 1,
	CONNDRV_TYPE_GPS = 2,
	CONNDRV_TYPE_WIFI = 3,
	CONNDRV_TYPE_CONNINFRA = 4,
	CONNDRV_TYPE_MAX
};
int conninfra_pwr_on(enum consys_drv_type drv_type);
int conninfra_pwr_off(enum consys_drv_type drv_type);
#endif /* CONFIG_CONNINFRA_SUPPORT */

#if (KERNEL_VERSION(2, 6, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 3) > LINUX_VERSION_CODE)
static inline void *netdev_priv(struct net_device *dev)
{
	return dev->priv;
}
#endif
#else
#if (KERNEL_VERSION(2, 4, 27) > LINUX_VERSION_CODE)
static inline void *netdev_priv(struct net_device *dev)
{
	return dev->priv;
}
#endif
#endif

/*
 *	Used for backward compatible with previous linux version which
 *	used "net_device->priv" as device driver structure hooking point
 */
static inline void netdev_priv_set(struct net_device *dev, void *priv)
{
#if (KERNEL_VERSION(2, 6, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 3) > LINUX_VERSION_CODE)
	dev->priv = priv;
#endif
#else
#if (KERNEL_VERSION(2, 4, 27) > LINUX_VERSION_CODE)
	dev->priv = priv;
#endif
#endif
}


int DebugLevel = DBG_LVL_NOTICE;

UINT32 DebugCategory = DBG_CAT_EN_ALL_MASK
				& ~(0x1 << DBG_CAT_TX)
				& ~(0x1 << DBG_CAT_PS);

UINT32 DebugSubCategory[DBG_LVL_MAX + 1][32] = {
	{
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_0, for DBG_CAT_MISC */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_1, for DBG_CAT_INIT */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_2, for DBG_CAT_HW */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_3, for DBG_CAT_FW */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_4, for DBG_CAT_HIF */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_5, for DBG_CAT_FPGA */
		DBG_SUBCAT_DIS_ALL_MASK,		/* bit_6, for DBG_CAT_TEST */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_7, for DBG_CAT_RA */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_8, for DBG_CAT_AP */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_9, for DBG_CAT_CLIENT */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_10, for DBG_CAT_TX */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_11, for DBG_CAT_RX */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_12, for DBG_CAT_CFG */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_13, for DBG_CAT_MLME */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_14, for DBG_CAT_PROTO */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_15, for DBG_CAT_SEC */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_16, for DBG_CAT_PS */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_17, for DBG_CAT_POWER */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_18, for DBG_CAT_COEX */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_19, for DBG_CAT_P2P */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_20, for DBG_CAT_TOKEN */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_21, for DBG_CAT_CMW */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_22, for DBG_CAT_BF */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_23 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_24 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_25 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_26 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_27 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_28 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_29 */
		DBG_SUBCAT_EN_ALL_MASK,			/* bit_30, for DBG_CAT_RSV1 */
		DBG_SUBCAT_EN_ALL_MASK			/* bir_31, for DBG_CAT_RSV2 */
	},
	{
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_DIS_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK
	}, /* ERROR */
	{
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_DIS_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK
	}, /* WARN */
	{
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_DIS_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK,
		DBG_SUBCAT_EN_ALL_MASK
	}, /* NOTICE */
	{0}, {0}
};


#ifdef MEM_ALLOC_INFO_SUPPORT
MEM_INFO_LIST MemInfoList;
MEM_INFO_LIST PktInfoList;
#endif /* MEM_ALLOC_INFO_SUPPORT */

#ifdef VENDOR_FEATURE4_SUPPORT
ULONG OS_NumOfMemAlloc = 0, OS_NumOfMemFree = 0;
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef VENDOR_FEATURE2_SUPPORT
ULONG OS_NumOfPktAlloc = 0, OS_NumOfPktFree = 0;
#endif /* VENDOR_FEATURE2_SUPPORT */

/*
 * the lock will not be used in TX/RX
 * path so throughput should not be impacted
 */
BOOLEAN FlgIsUtilInit = FALSE;
OS_NDIS_SPIN_LOCK UtilSemLock;

BOOLEAN RTMP_OS_Alloc_RscOnly(VOID *pRscSrc, UINT32 RscLen);
BOOLEAN RTMP_OS_Remove_Rsc(LIST_HEADER *pRscList, VOID *pRscSrc);

static struct device *rtmp_get_dev(void *ad);

/*
 * ========================================================================
 * Routine Description:
 * Initialize something in UTIL module.
 *
 * Arguments:
 *	None
 *
 * Return Value:
 *	None
 * ========================================================================
 */
VOID RtmpUtilInit(VOID)
{
	if (FlgIsUtilInit == FALSE) {
		OS_NdisAllocateSpinLock(&UtilSemLock);
		FlgIsUtilInit = TRUE;
	}
}

/* timeout -- ms */
static inline VOID __RTMP_SetPeriodicTimer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
static inline VOID __RTMP_OS_Init_Timer(
	IN VOID *pReserved,
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN TIMER_FUNCTION function,
	IN PVOID data)
{
	if (!timer_pending(pTimer)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		timer_setup(pTimer, function, 0);
#else
		init_timer(pTimer);
		pTimer->data = (unsigned long)data;
		pTimer->function = function;
#endif
	}
}

static inline VOID __RTMP_OS_Add_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout * OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

static inline VOID __RTMP_OS_Mod_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	IN unsigned long timeout)
{
	timeout = ((timeout * OS_HZ) / 1000);
	mod_timer(pTimer, jiffies + timeout);
}

static inline VOID __RTMP_OS_Del_Timer(
	IN OS_NDIS_MINIPORT_TIMER * pTimer,
	OUT BOOLEAN *pCancelled)
{
	if (timer_pending(pTimer))
		*pCancelled = del_timer_sync(pTimer);
	else
		*pCancelled = TRUE;
}


/* Unify all delay routine by using udelay */
VOID RtmpusecDelay(ULONG usec)
{
	ULONG i;

	for (i = 0; i < (usec / 50); i++)
		udelay(50);

	if (usec % 50)
		udelay(usec % 50);
}


VOID RtmpOsMsDelay(ULONG msec)
{
	mdelay(msec);
}

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time)
{
	time->u.LowPart = jiffies;
}

void RTMP_GetCurrentSystemTick(ULONG *pNow)
{
	*pNow = jiffies;
}

ULONG RTMPMsecsToJiffies(UINT32 m)
{
	return msecs_to_jiffies(m);
}



NDIS_STATUS os_alloc_mem_suspend(
	IN VOID *pReserved,
	OUT UCHAR * *mem,
	IN ULONG size)
{
	*mem = (PUCHAR) kmalloc(size, GFP_KERNEL);

	if (*mem) {
#ifdef VENDOR_FEATURE4_SUPPORT
		OS_NumOfMemAlloc++;
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef MEM_ALLOC_INFO_SUPPORT
		MIListAddHead(&MemInfoList, size, *mem, __func__, __LINE__, __builtin_return_address(0));
#endif /* MEM_ALLOC_INFO_SUPPORT */
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

PNDIS_PACKET RtmpOSNetPktAlloc(VOID *dummy, int size)
{
	struct sk_buff *skb;
	/* Add 2 more bytes for ip header alignment */
	DEV_ALLOC_SKB(skb, (size + 2));
	return (PNDIS_PACKET) skb;
}

PNDIS_PACKET RTMP_AllocateFragPacketBuffer(VOID *dummy, ULONG len)
{
	struct sk_buff *pkt;

	DEV_ALLOC_SKB(pkt, len);

	if (pkt == NULL) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_FRAG_TX, DBG_LVL_ERROR,
				 "can't allocate frag rx %ld size packet\n", len);
	}

	return (PNDIS_PACKET) pkt;
}

/*
 * The allocated NDIS PACKET must be freed via RTMPFreeNdisPacket()
 */
NDIS_STATUS RTMPAllocateNdisPacket(
	IN VOID *pReserved,
	OUT PNDIS_PACKET *ppPacket,
	IN UCHAR * pHeader,
	IN UINT HeaderLen,
	IN UCHAR * pData,
	IN UINT DataLen)
{
	struct sk_buff *pPacket;
	/* Add LEN_CCMP_HDR + LEN_CCMP_MIC for PMF */
	DEV_ALLOC_SKB(pPacket, (HeaderLen + DataLen + RTMP_PKT_TAIL_PADDING + LEN_CCMP_HDR + LEN_CCMP_MIC));

	if (pPacket == NULL) {
		*ppPacket = NULL;
#ifdef DBG
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR,
				 KERN_ERR "RTMPAllocateNdisPacket Fail\n\n");
#endif
		return NDIS_STATUS_FAILURE;
	}

	/* Clone the frame content and update the length of packet */
	if ((HeaderLen > 0) && (pHeader != NULL))
		os_move_mem(pPacket->data, pHeader, HeaderLen);

	if ((DataLen > 0) && (pData != NULL))
		os_move_mem(pPacket->data + HeaderLen, pData, DataLen);

	skb_put(pPacket, HeaderLen + DataLen);
	*ppPacket = (PNDIS_PACKET)pPacket;
	return NDIS_STATUS_SUCCESS;
}


/*
 * ========================================================================
 * Description:
 * This routine frees a miniport internally allocated NDIS_PACKET and its
 * corresponding NDIS_BUFFER and allocated memory.
 * ========================================================================
 */
VOID RTMPFreeNdisPacket(VOID *pReserved, PNDIS_PACKET pPacket)
{
	if (pPacket) {
		WLAN_HOOK_CALL(WLAN_HOOK_DRIVER_FREE_PKT, NULL, RTPKT_TO_OSPKT(pPacket));

		dev_kfree_skb_any(RTPKT_TO_OSPKT(pPacket));
		MEM_DBG_PKT_FREE_INC(pPacket);
	}
}


VOID RTMPFreeNdisPacketIRQ(VOID *pReserved, PNDIS_PACKET pPacket)
{
	if (pPacket) {
		dev_kfree_skb_irq(RTPKT_TO_OSPKT(pPacket));
		MEM_DBG_PKT_FREE_INC(pPacket);
	}
}



PNDIS_PACKET ClonePacket(BOOLEAN MonitorOn, PNET_DEV ndev, PNDIS_PACKET pkt, UCHAR *buf, ULONG sz)
{
	struct sk_buff *pRxPkt, *pClonedPkt;

	if (pkt == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR, "Error! pkt is NULL\n");
		ASSERT(pkt);
		return NULL;
	}

#ifdef SNIFFER_SUPPORT
	if (!MonitorOn)
#endif /* SNIFFER_SUPPORT */
		ASSERT(sz < RX_DATA_BUFFER_SIZE);

	pRxPkt = RTPKT_TO_OSPKT(pkt);
	/* clone the packet */
	pClonedPkt = skb_clone(pRxPkt, MEM_ALLOC_FLAG);

	if (pClonedPkt) {
		/* set the correct dataptr and data len */
		MEM_DBG_PKT_ALLOC_INC(pClonedPkt);
		pClonedPkt->dev = pRxPkt->dev;
		pClonedPkt->data = buf;
		pClonedPkt->len = sz;
		SET_OS_PKT_DATATAIL(pClonedPkt, pClonedPkt->len);
	}

	return pClonedPkt;
}


PNDIS_PACKET DuplicatePacket(PNET_DEV pNetDev, PNDIS_PACKET pPacket)
{
	struct sk_buff *skb;
	PNDIS_PACKET pRetPacket = NULL;
	USHORT DataSize;
	UCHAR *pData;

	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);
	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);
	skb = skb_clone(RTPKT_TO_OSPKT(pPacket), MEM_ALLOC_FLAG);

	if (skb) {
		MEM_DBG_PKT_ALLOC_INC(skb);
		skb->dev = pNetDev;
		pRetPacket = OSPKT_TO_RTPKT(skb);
	}

	return pRetPacket;
}

#ifdef MAP_TS_TRAFFIC_SUPPORT
PNDIS_PACKET CopyPacket(
	IN PNET_DEV if_dev,
	IN PNDIS_PACKET pkt
)
{
	struct sk_buff *skb = NULL;
	PNDIS_PACKET pkt_copy = NULL;

	skb = skb_copy(RTPKT_TO_OSPKT(pkt), GFP_ATOMIC);

	if (skb) {
		skb->dev = if_dev;
		pkt_copy = OSPKT_TO_RTPKT(skb);
	}

	return pkt_copy;
}
#endif


PNDIS_PACKET duplicate_pkt_vlan(
	IN PNET_DEV pNetDev,
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	IN PUCHAR pData,
	IN ULONG DataSize,
	IN UCHAR *TPID)
{
	struct sk_buff *skb;
	PNDIS_PACKET pPacket = NULL;
	UINT16 VLAN_Size = 0;
	INT skb_len = HdrLen + DataSize + 2;
#ifdef WIFI_VLAN_SUPPORT

	if (VLAN_VID != 0)
		skb_len += LENGTH_802_1Q;

#endif /* WIFI_VLAN_SUPPORT */
	skb = __dev_alloc_skb(skb_len, MEM_ALLOC_FLAG);

	if (skb != NULL) {
		MEM_DBG_PKT_ALLOC_INC(skb);
		skb_reserve(skb, 2);

		/* copy header (maybe with VLAN tag) */
		VLAN_Size = VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
										  pHeader802_3, HdrLen,
										  GET_OS_PKT_DATATAIL(skb),
										  TPID);
		skb_put(skb, HdrLen + VLAN_Size);
		/* copy data body */
		os_move_mem(GET_OS_PKT_DATATAIL(skb), pData, DataSize);

		skb_put(skb, DataSize);
		skb->dev = pNetDev;
		pPacket = OSPKT_TO_RTPKT(skb);
	}

	return pPacket;
}


#define TKIP_TX_MIC_SIZE		8
PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(VOID *pReserved, PNDIS_PACKET pPacket)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);

	if (skb_tailroom(skb) < TKIP_TX_MIC_SIZE) {
		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, skb_headroom(skb), TKIP_TX_MIC_SIZE, GFP_ATOMIC);
		dev_kfree_skb_any(skb);
		MEM_DBG_PKT_FREE_INC(skb);

		if (newskb == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR,
				"Extend Tx.MIC for packet failed!, dropping packet!\n");
			return NULL;
		}

		skb = newskb;
		MEM_DBG_PKT_ALLOC_INC(skb);
	}

	return OSPKT_TO_RTPKT(skb);
}


/*
 * ========================================================================
 *
 * Routine Description:
 * Send a L2 frame to upper daemon to trigger state machine
 *
 * Arguments:
 * pAd - pointer to our pAdapter context
 *
 * Return Value:
 *
 * Note:
 *
 * ========================================================================
 */
BOOLEAN RTMPL2FrameTxAction(
	IN VOID *pCtrlBkPtr,
	IN RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet,
	IN UCHAR apidx,
	IN UCHAR *pData,
	IN UINT32 data_len,
	IN UCHAR OpMode)
{
	struct sk_buff *skb = NULL;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pCtrlBkPtr;
	PNET_DEV pNetDev = NULL;

	if (!pAd || apidx >= MAX_BEACON_NUM) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				 "Error! pAd is NULL or apidx invalid, apidx=%d.\n", apidx);
		return FALSE;
	}

	DEV_ALLOC_SKB(skb, (data_len + 2));

	if (!skb) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_ERROR,
				 "Error! Can't allocate a skb.\n");
		return FALSE;
	}

	pNetDev = get_netdev_from_bssid(pAd, pAd->ApCfg.MBSSID[apidx].wdev.wdev_idx),

	SET_OS_PKT_NETDEV(skb, pNetDev);
	/* 16 byte align the IP header */
	skb_reserve(skb, 2);
	/* Insert the frame content */
	os_move_mem(GET_OS_PKT_DATAPTR(skb), pData, data_len);
	/* End this frame */
	skb_put(GET_OS_PKT_TYPE(skb), data_len);
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DATA, DBG_LVL_INFO, "done\n");
	_announce_802_3_packet(pCtrlBkPtr, skb, OpMode);
	return TRUE;
}


PNDIS_PACKET ExpandPacket(
	IN VOID *pReserved,
	IN PNDIS_PACKET pPacket,
	IN UINT32 ext_head_len,
	IN UINT32 ext_tail_len)
{
	struct sk_buff *skb, *newskb;

	skb = RTPKT_TO_OSPKT(pPacket);

	if (skb_cloned(skb) ||
		(skb_headroom(skb) < ext_head_len) ||
		(skb_tailroom(skb) < ext_tail_len)) {
		UINT32 head_len =
			(skb_headroom(skb) < ext_head_len) ? ext_head_len : skb_headroom(skb);
		UINT32 tail_len =
			(skb_tailroom(skb) < ext_tail_len) ? ext_tail_len : skb_tailroom(skb);
		/* alloc a new skb and copy the packet */
		newskb = skb_copy_expand(skb, head_len, tail_len, GFP_ATOMIC);
		dev_kfree_skb_any(skb);
		MEM_DBG_PKT_FREE_INC(skb);

		if (newskb == NULL) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR,
					 "Extend Tx buffer for WPI failed!, dropping packet!\n");
			return NULL;
		}

		skb = newskb;
		MEM_DBG_PKT_ALLOC_INC(skb);
	}

	return OSPKT_TO_RTPKT(skb);
}


VOID RtmpOsPktInit(
	IN PNDIS_PACKET pNetPkt,
	IN PNET_DEV pNetDev,
	IN UCHAR *pData,
	IN USHORT DataSize)
{
	PNDIS_PACKET pRxPkt;

	pRxPkt = RTPKT_TO_OSPKT(pNetPkt);
	SET_OS_PKT_NETDEV(pRxPkt, pNetDev);
	SET_OS_PKT_DATAPTR(pRxPkt, pData);
	SET_OS_PKT_LEN(pRxPkt, DataSize);
	SET_OS_PKT_DATATAIL(pRxPkt, DataSize);
}


void wlan_802_11_to_802_3_packet(
	IN PNET_DEV pNetDev,
	IN UCHAR OpMode,
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN PNDIS_PACKET pRxPacket,
	IN UCHAR *pData,
	IN ULONG DataSize,
	IN PUCHAR pHeader802_3,
	IN UCHAR *TPID)
{
	struct sk_buff *pOSPkt;

	if (pHeader802_3 == NULL) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_ERROR,
			 "pHeader802_3 is NULL\n");
		ASSERT(pHeader802_3);
		return;
	}
	pOSPkt = RTPKT_TO_OSPKT(pRxPacket);
	pOSPkt->dev = pNetDev;
	pOSPkt->data = pData;
	pOSPkt->len = DataSize;
	SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);

	/* copy 802.3 header */
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(OpMode) {
		/* maybe insert VLAN tag to the received packet */
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;

		if (VLAN_VID != 0)
			VLAN_Size = LENGTH_802_1Q;

		data_p = skb_push(pOSPkt, LENGTH_802_3 + VLAN_Size);
		VLAN_8023_Header_Copy(VLAN_VID, VLAN_Priority,
							  pHeader802_3, LENGTH_802_3,
							  data_p, TPID);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_STA(OpMode) {
		os_move_mem(skb_push(pOSPkt, LENGTH_802_3), pHeader802_3, LENGTH_802_3);
	}
#endif /* CONFIG_STA_SUPPORT */
}

#ifdef SYSTEM_LOG_SUPPORT
/*
 * ========================================================================
 *
 * Routine Description:
 * Send log message through wireless event
 *
 * Support standard iw_event with IWEVCUSTOM. It is used below.
 *
 * iwreq_data.data.flags is used to store event_flag that is
 * defined by user. iwreq_data.data.length is the length of the
 * event log.
 *
 * The format of the event log is composed of the entry's MAC
 * address and the desired log message (refer to
 * pWirelessEventText).
 *
 * ex: 11:22:33:44:55:66 has associated successfully
 *
 * p.s. The requirement of Wireless Extension is v15 or newer.
 *
 * ========================================================================
 */
VOID RtmpOsSendWirelessEvent(
	IN VOID *pAd,
	IN USHORT Event_flag,
	IN PUCHAR pAddr,
	IN UCHAR wdev_idx,
	IN CHAR Rssi,
	IN RTMP_OS_SEND_WLAN_EVENT pFunc)
{
#if WIRELESS_EXT >= 15
	pFunc(pAd, Event_flag, pAddr, wdev_idx, Rssi);
#else
	MTWF_DBG(pAd, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_ERROR,
			 "The Wireless Extension MUST be v15 or newer.\n");
#endif /* WIRELESS_EXT >= 15 */
}
#endif /* SYSTEM_LOG_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID SendSignalToDaemon(
	IN INT sig,
	RTMP_OS_PID pid,
	unsigned long pid_no)
{
}
#endif /* CONFIG_AP_SUPPORT */


/*******************************************************************************
 *	File open/close related functions.
 *******************************************************************************/
RTMP_OS_FD RtmpOSFileOpen(char *pPath, int flag, int mode)
{
	struct file *filePtr;

	if (flag == RTMP_FILE_RDONLY)
		flag = O_RDONLY;
	else if (flag == RTMP_FILE_WRONLY)
		flag = O_WRONLY;
	else if (flag == RTMP_FILE_CREAT)
		flag = O_CREAT;
	else if (flag == RTMP_FILE_TRUNC)
		flag = O_TRUNC;

	filePtr = filp_open(pPath, flag, 0);

	if (IS_ERR(filePtr)) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR,
				"Error %ld opening %s\n",
				-PTR_ERR(filePtr), pPath);
	}

	return (RTMP_OS_FD) filePtr;
}


int RtmpOSFileClose(RTMP_OS_FD osfd)
{
	filp_close(osfd, NULL);
	return 0;
}


void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset)
{
	osfd->f_pos = offset;
}


int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen)
{
	/* The object must have a read method */
#if (KERNEL_VERSION(3, 19, 0) > LINUX_VERSION_CODE)
	if (osfd->f_op && osfd->f_op->read) {
		return osfd->f_op->read(osfd, pDataPtr, readLen, &osfd->f_pos);
	}
#elif (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
	if (osfd->f_mode & FMODE_CAN_READ) {
		return kernel_read(osfd, pDataPtr, readLen, &osfd->f_pos);
	}
#else
	if (osfd->f_mode & FMODE_CAN_READ) {
		return __vfs_read(osfd, pDataPtr, readLen, &osfd->f_pos);
	}
#endif
	else {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_PROFILE, DBG_LVL_ERROR, "no file read method\n");
		return -1;
	}
}


int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen)
{
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
	return osfd->f_op->write(osfd, pDataPtr, (size_t) writeLen, &osfd->f_pos);
#elif (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE)
	return __kernel_write(osfd, pDataPtr, (size_t) writeLen, &osfd->f_pos);
#elif (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
	return kernel_write(osfd, pDataPtr, (size_t) writeLen, &osfd->f_pos);
#else
	return __vfs_write(osfd, pDataPtr, (size_t) writeLen, &osfd->f_pos);
#endif
}


static inline void __RtmpOSFSInfoChange(OS_FS_INFO *pOSFSInfo, BOOLEAN bSet)
{
	if (bSet) {
		/* Save uid and gid used for filesystem access. */
		/* Set user and group to 0 (root) */
#if (KERNEL_VERSION(2, 6, 29) > LINUX_VERSION_CODE)
		pOSFSInfo->fsuid = current->fsuid;
		pOSFSInfo->fsgid = current->fsgid;
		current->fsuid = current->fsgid = 0;
#else
#ifdef CONFIG_UIDGID_STRICT_TYPE_CHECKS
		kuid_t uid;
		kgid_t gid;

		uid = current_fsuid();
		gid = current_fsgid();
		pOSFSInfo->fsuid = (int)uid.val;
		pOSFSInfo->fsgid = (int)gid.val;
#else
		/* pOSFSInfo->fsuid = (int)(current_fsuid()); */
		/* pOSFSInfo->fsgid = (int)(current_fsgid()); */
#endif
#endif
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
		pOSFSInfo->fs = get_fs();
		set_fs(KERNEL_DS);
#endif
	} else {
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
		set_fs(pOSFSInfo->fs);
#endif
#if (KERNEL_VERSION(2, 6, 29) > LINUX_VERSION_CODE)
		current->fsuid = pOSFSInfo->fsuid;
		current->fsgid = pOSFSInfo->fsgid;
#endif
	}
}


/*******************************************************************************
 *	Task create/management/kill related functions.
 *******************************************************************************/
static inline NDIS_STATUS __RtmpOSTaskKill(OS_TASK *pTask)
{
	int ret = NDIS_STATUS_FAILURE;
#ifdef KTHREAD_SUPPORT

	if (pTask->kthread_task) {
		if (kthread_stop(pTask->kthread_task) == 0) {
			pTask->kthread_task = NULL;
			ret = NDIS_STATUS_SUCCESS;
		} else {
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_WARN,
				"kthread_task %s stop failed\n",
				pTask->taskName);
		}
	} else
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_WARN,
				"null kthread_task %s\n",
				pTask->taskName);

#else
	if (CHECK_PID_LEGALITY(pTask->taskPID)) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_INFO,
				 "Terminate the task(%s) with pid(%d)!\n",
				  pTask->taskName, GET_PID_NUMBER(pTask->taskPID));
		mb();
		pTask->task_killed = 1;
		mb();
		ret = KILL_THREAD_PID(pTask->taskPID, SIGTERM, 1);

		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
					 KERN_WARNING
					  "kill task(%s) with pid(%d) failed(retVal=%d)!\n",
					  pTask->taskName, GET_PID_NUMBER(pTask->taskPID),
					  ret);
		} else {
			wait_for_completion(&pTask->taskComplete);
			pTask->taskPID = THREAD_PID_INIT_VALUE;
			pTask->task_killed = 0;
			RTMP_SEM_EVENT_DESTORY(&pTask->taskSema);
			ret = NDIS_STATUS_SUCCESS;
		}
	}
#endif
	return ret;
}


static inline INT __RtmpOSTaskNotifyToExit(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	complete_and_exit(&pTask->taskComplete, 0);
#endif
#ifdef WIFI_DIAG
	diag_del_pid(pTask);
#endif
	return 0;
}


static inline void __RtmpOSTaskCustomize(OS_TASK *pTask)
{
#ifndef KTHREAD_SUPPORT
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	daemonize((RTMP_STRING *) &pTask->taskName[0] /*"%s",pAd->net_dev->name */);
	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	current->flags |= PF_NOFREEZE;
#else
	unsigned long flags;

	daemonize();
	reparent_to_init();
	strlcpy(current->comm, &pTask->taskName[0], sizeof(pTask->taskName));
	siginitsetinv(&current->blocked, sigmask(SIGTERM) | sigmask(SIGKILL));
	/* Allow interception of SIGKILL only
	  * Don't allow other signals to interrupt the transmission
	 */
#if (KERNEL_VERSION(2, 4, 22) < LINUX_VERSION_CODE)
	spin_lock_irqsave(&current->sigmask_lock, flags);
	flush_signals(current);
	recalc_sigpending(current);
	spin_unlock_irqrestore(&current->sigmask_lock, flags);
#endif
#endif
	RTMP_GET_OS_PID(pTask->taskPID, current->pid);
	/* signal that we've started the thread */
	complete(&pTask->taskComplete);
#endif
#ifdef WIFI_DIAG
	diag_add_pid(pTask);
#endif
}


static inline NDIS_STATUS __RtmpOSTaskAttach(
	IN OS_TASK *pTask,
	IN RTMP_OS_TASK_CALLBACK fn,
	IN ULONG arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
#ifndef KTHREAD_SUPPORT
	pid_t pid_number = -1;
#endif /* KTHREAD_SUPPORT */
#ifdef KTHREAD_SUPPORT
	UINT8 retry = 0;
	pTask->task_killed = 0;

	if (pTask->kthread_task) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				 "non-null kthread_task %s\n",
				  pTask->taskName);
		status = NDIS_STATUS_FAILURE;
		goto done;
	}

RETRY:
	pTask->kthread_task = kthread_run((cast_fn)fn, (void *)arg,
									  pTask->taskName);

	if (IS_ERR(pTask->kthread_task)) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"kthread_run %s err %ld\n",
				pTask->taskName, PTR_ERR(pTask->kthread_task));

		/* just an interrupted system call */
		if ((PTR_ERR(pTask->kthread_task) == EINTR) && (retry < 5)) {
			retry++;
			mdelay(1);
			goto RETRY;
		}

		pTask->kthread_task = NULL;
		status = NDIS_STATUS_FAILURE;
		goto done;
	}

#else
	pid_number =
		kernel_thread((cast_fn) fn, (void *)arg, RTMP_OS_MGMT_TASK_FLAGS);

	if (pid_number < 0) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_ERROR,
				"Attach task(%s) failed!\n", pTask->taskName);
		status = NDIS_STATUS_FAILURE;
	} else {
		/* Wait for the thread to start */
		wait_for_completion(&pTask->taskComplete);
		status = NDIS_STATUS_SUCCESS;
	}

#endif
done:
	MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_INFO,
			 "%s %s end %d\n", __func__, pTask->taskName, status);
	return status;
}


static inline NDIS_STATUS __RtmpOSTaskInit(
	IN OS_TASK *pTask,
	IN RTMP_STRING *pTaskName,
	IN VOID *pPriv,
	IN LIST_HEADER *pSemList)
{
	int len;

	ASSERT(pTask);
#ifndef KTHREAD_SUPPORT
	os_zero_mem((PUCHAR) (pTask), sizeof(OS_TASK));
#endif
	len = strlen(pTaskName);
	len = len > (RTMP_OS_TASK_NAME_LEN - 1) ? (RTMP_OS_TASK_NAME_LEN - 1) : len;
	os_move_mem(&pTask->taskName[0], pTaskName, len);
	pTask->priv = pPriv;
#ifndef KTHREAD_SUPPORT
	RTMP_SEM_EVENT_INIT_LOCKED(&(pTask->taskSema), pSemList);
	pTask->taskPID = THREAD_PID_INIT_VALUE;
	init_completion(&pTask->taskComplete);
#endif
#ifdef KTHREAD_SUPPORT
	init_waitqueue_head(&(pTask->kthread_q));
#endif /* KTHREAD_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}


BOOLEAN __RtmpOSTaskWait(
	IN VOID *pReserved,
	IN OS_TASK *pTask,
	IN INT32 * pStatus)
{
#ifdef KTHREAD_SUPPORT
	RTMP_WAIT_EVENT_INTERRUPTIBLE((*pStatus), pTask);
	if ((pTask->task_killed == 1) || ((*pStatus) != 0))
		return FALSE;

#else
	RTMP_SEM_EVENT_WAIT(&(pTask->taskSema), (*pStatus));

	/* unlock the device pointers */
	if ((*pStatus) != 0) {
		/*		RTMP_SET_FLAG_(*pFlags, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
		return FALSE;
	}

#endif /* KTHREAD_SUPPORT */
	return TRUE;
}

BOOLEAN __RtmpOSTaskWaitCond(
	IN VOID *pReserved,
	IN OS_TASK *pTask,
	IN UINT32 u4Cond,
	IN INT32 * pStatus)
{
#ifdef KTHREAD_SUPPORT
	RTMP_WAIT_EVENT_INTERRUPTIBLE_COND((*pStatus), pTask, u4Cond);

	if ((pTask->task_killed == 1) || ((*pStatus) != 0))
		return FALSE;

#else
#error "Not yet implemented"
#endif /* KTHREAD_SUPPORT */
	return TRUE;
}


#if LINUX_VERSION_CODE <= 0x20402	/* Red Hat 7.1 */
struct net_device *alloc_netdev(
	int sizeof_priv,
	const char *mask,
	void (*setup)(struct net_device *))
{
	struct net_device *dev;
	INT alloc_size;
	/* ensure 32-byte alignment of the private area */
	alloc_size = sizeof(*dev) + sizeof_priv + 31;
	dev = kmalloc(alloc_size, GFP_KERNEL);

	if (dev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				 "alloc_netdev: Unable to allocate device memory.\n");
		return NULL;
	}

	memset(dev, 0, alloc_size);

	if (sizeof_priv)
		dev->priv = (void *)(((long)(dev + 1) + 31) & ~31);

	setup(dev);
	strlcpy(dev->name, mask, sizeof(dev->name));
	return dev;
}
#endif /* LINUX_VERSION_CODE */


static UINT32 RtmpOSWirelessEventTranslate(IN UINT32 eventType)
{
	switch (eventType) {
	case RT_WLAN_EVENT_CUSTOM:
		eventType = IWEVCUSTOM;
		break;

	case RT_WLAN_EVENT_CGIWAP:
		eventType = SIOCGIWAP;
		break;
#if WIRELESS_EXT > 17

	case RT_WLAN_EVENT_ASSOC_REQ_IE:
		eventType = IWEVASSOCREQIE;
		break;
#endif /* WIRELESS_EXT */
#if WIRELESS_EXT >= 14

	case RT_WLAN_EVENT_SCAN:
		eventType = SIOCGIWSCAN;
		break;
#endif /* WIRELESS_EXT */

	case RT_WLAN_EVENT_EXPIRED:
		eventType = IWEVEXPIRED;
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_INFO,
				 "Unknown event: 0x%x\n", eventType);
		break;
	}

	return eventType;
}


int RtmpOSWrielessEventSend(
	IN PNET_DEV pNetDev,
	IN UINT32 eventType,
	IN INT flags,
	IN PUCHAR pSrcMac,
	IN PUCHAR pData,
	IN UINT32 dataLen)
{
	union iwreq_data wrqu;

	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);
	memset(&wrqu, 0, sizeof(wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;
	else
		wrqu.data.length = 0;
#ifdef CONFIG_WEXT_CORE
	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
#endif /* CONFIG_WEXT_CORE */

	return 0;
}


int RtmpOSWrielessEventSendExt(
	IN PNET_DEV pNetDev,
	IN UINT32 eventType,
	IN INT flags,
	IN PUCHAR pSrcMac,
	IN PUCHAR pData,
	IN UINT32 dataLen,
	IN UINT32 family)
{
	union iwreq_data wrqu;
	/* translate event type */
	eventType = RtmpOSWirelessEventTranslate(eventType);
	/* translate event type */
	memset(&wrqu, 0, sizeof(wrqu));

	if (flags > -1)
		wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData != NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;

	wrqu.addr.sa_family = family;
#ifdef CONFIG_WEXT_CORE
	wireless_send_event(pNetDev, eventType, &wrqu, (char *)pData);
#endif /* CONFIG_WEXT_CORE */
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 * Check if the network interface is up.
 *
 * Arguments:
 * *pDev                        - Network Interface
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
BOOLEAN RtmpOSNetDevIsUp(VOID *pDev)
{
	struct net_device *pNetDev = (struct net_device *)pDev;

	if ((pNetDev == NULL) || !(pNetDev->flags & IFF_UP))
		return FALSE;

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 * Assign sys_handle data pointer (pAd) to the priv info structured linked to
 * the OS network interface.
 *
 * Arguments:
 * pDev                 - the os net device data structure
 * pPriv                        - the sys_handle want to assigned
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsSetNetDevPriv(VOID *pDev, VOID *pPriv)
{
	struct mt_dev_priv *priv_info = NULL;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	priv_info->sys_handle = (VOID *)pPriv;
	priv_info->priv_flags = 0;
}


/*
 * ========================================================================
 * Routine Description:
 * Get wifi_dev from the priv info linked to OS network interface data structure.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * sys_handle
 *
 * Note:
 * ========================================================================
 */
VOID *RtmpOsGetNetDevPriv(VOID *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->sys_handle;
}


VOID RtmpOsSetNetDevWdev(VOID *net_dev, VOID *wdev)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)net_dev);
	priv_info->wifi_dev = wdev;
}


VOID *RtmpOsGetNetDevWdev(VOID *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->wifi_dev;
}


/*
 * ========================================================================
 * Routine Description:
 * Get private flags from the network interface.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * pPriv                        - the pointer
 *
 * Note:
 * ========================================================================
 */
USHORT RtmpDevPrivFlagsGet(VOID *pDev)
{
	return ((struct mt_dev_priv *)netdev_priv((struct net_device *)pDev))->priv_flags;
}


/*
 * ========================================================================
 * Routine Description:
 * Get private flags from the network interface.
 *
 * Arguments:
 * pDev                 - the device
 *
 * Return Value:
 * pPriv                        - the pointer
 *
 * Note:
 * ========================================================================
 */
VOID RtmpDevPrivFlagsSet(VOID *pDev, USHORT PrivFlags)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	priv_info->priv_flags = PrivFlags;
}

UCHAR get_sniffer_mode(VOID *pDev)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)pDev);
	return priv_info->sniffer_mode;
}

VOID set_sniffer_mode(VOID *net_dev, UCHAR sniffer_mode)
{
	struct mt_dev_priv *priv_info;

	priv_info = (struct mt_dev_priv *)netdev_priv((struct net_device *)net_dev);
	priv_info->sniffer_mode = sniffer_mode;
}

/*
 * ========================================================================
 * Routine Description:
 *	Get network interface name.
 *
 * Arguments:
 *	pDev			- the device
 *
 * Return Value:
 *	the name
 * ========================================================================
 */
unsigned long RtmpOSGetNetDevState(VOID *pDev)
{
	return ((PNET_DEV) pDev)->state;
}



unsigned int RtmpOSGetNetDevFlag(VOID *pDev)
{
	return ((PNET_DEV)pDev)->flags;
}


unsigned int RtmpOSGetNetDevQNum(VOID *pDev)
{
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	return ((PNET_DEV)pDev)->num_tx_queues;
#else
	return 0;
#endif
}


unsigned long RtmpOSGetNetDevQState(VOID *pDev, unsigned int q_idx)
{
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	struct netdev_queue *que = netdev_get_tx_queue((PNET_DEV)pDev, q_idx);

	if (que)
		return que->state;
	else
		return ~0x0;

#else
	return ~0x0;
#endif
}


char *RtmpOsGetNetDevName(VOID *pDev)
{
	return ((PNET_DEV) pDev)->name;
}


UINT32 RtmpOsGetNetIfIndex(IN VOID *pDev)
{
	return ((PNET_DEV) pDev)->ifindex;
}


int RtmpOSNetDevAddrSet(
	IN UCHAR OpMode,
	IN PNET_DEV pNetDev,
	IN PUCHAR pMacAddr,
	IN PUCHAR dev_name)
{
	struct net_device *net_dev = (struct net_device *)pNetDev;
#ifdef CONFIG_STA_SUPPORT
	/* work-around for the SuSE due to it has it's own interface name management system. */
	RT_CONFIG_IF_OPMODE_ON_STA(OpMode) {
		if (dev_name != NULL) {
			os_zero_mem(dev_name, 16);
			os_move_mem(dev_name, net_dev->name, strlen(net_dev->name));
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	dev_addr_set(net_dev, pMacAddr);
	return 0;
}


/* Assign the network dev name for created Ralink WiFi interface.*/
static int RtmpOSNetDevRequestName(
	IN INT32 MC_RowID,
	IN UINT32 *pIoctlIF,
	IN PNET_DEV dev,
	IN RTMP_STRING *pPrefixStr,
	IN INT devIdx,
	IN BOOLEAN autoSuffix)
{
	PNET_DEV existNetDev;
	RTMP_STRING suffixName[IFNAMSIZ];
	RTMP_STRING desiredName[IFNAMSIZ];
	int ifNameIdx,
		prefixLen,
		slotNameLen;
	int Status;
	INT ret;

	prefixLen = strlen(pPrefixStr);
	ASSERT((prefixLen < IFNAMSIZ));

	for (ifNameIdx = devIdx; ifNameIdx < 32; ifNameIdx++) {
#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RT_SOC_SUPPORT

		if (MC_RowID > 0)
			ret = snprintf(suffixName, sizeof(suffixName) "i%d", ifNameIdx);
		else
#else
		if (MC_RowID >= 0)
			ret = snprintf(suffixName, sizeof(suffixName), "%02d_%d", MC_RowID, ifNameIdx);
		else
#endif /* RT_SOC_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */
			ret = snprintf(suffixName, sizeof(suffixName), "%d", ifNameIdx);
		if (os_snprintf_error(sizeof(suffixName), ret) != 0)
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				"%s: snprintf error!\n", __func__);

		slotNameLen = strlen(suffixName);
		ASSERT(((slotNameLen + prefixLen) < IFNAMSIZ));

		if (autoSuffix)
			ret = snprintf(desiredName, sizeof(desiredName), "%s%s", pPrefixStr, suffixName);
		else
			ret = snprintf(desiredName, sizeof(desiredName), "%s", pPrefixStr);

		if (os_snprintf_error(sizeof(desiredName), ret) != 0)
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				"%s: snprintf error!\n", __func__);

		existNetDev = RtmpOSNetDevGetByName(dev, &desiredName[0]);

		if (existNetDev == NULL)
			break;
		else if (autoSuffix == FALSE && existNetDev) {
			ifNameIdx = 32;	/* Tend to leave loop then return failure */
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
					"Cannot request DevName with string(%s) from OS!\n", pPrefixStr);
		}

		RtmpOSNetDeviceRefPut(existNetDev);
	}

	if (ifNameIdx < 32) {
		strlcpy(&dev->name[0], &desiredName[0], sizeof(dev->name));
		Status = NDIS_STATUS_SUCCESS;
	} else {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				 "Cannot request DevName with preifx(%s) and in range(0~32) as suffix from OS!\n",
				  pPrefixStr);
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}

void RtmpOSNetDevClose(PNET_DEV pNetDev)
{
	dev_close(pNetDev);
}

void RtmpOSNetDevFree(PNET_DEV pNetDev)
{
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	void *tmp_ptr;
#endif

	if (pNetDev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "pNetDev is NULL!\n");
		ASSERT(pNetDev);
		return;
	}

#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	tmp_ptr = (void *)pNetDev->netdev_ops;
#endif
	free_netdev(pNetDev);
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	if (tmp_ptr)
		vfree(tmp_ptr);
#endif
#else
	kfree(pNetDev);
#endif
#ifdef VENDOR_FEATURE4_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO,
			 "OS_NumOfMemAlloc = %ld, OS_NumOfMemFree = %ld\n",
			  OS_NumOfMemAlloc, OS_NumOfMemFree);
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef VENDOR_FEATURE2_SUPPORT
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_INFO,
			 "OS_NumOfPktAlloc = %ld, OS_NumOfPktFree = %ld\n",
			  OS_NumOfPktAlloc, OS_NumOfPktFree);
#endif /* VENDOR_FEATURE2_SUPPORT */
}

#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
void RtmpOSNetDevOpsFree(PNET_DEV pNetDev)
{
	void *pNetDevOps = NULL;

	if (pNetDev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR, "pNetDev is NULL!\n");
		ASSERT(pNetDev);
		return;
	}

	pNetDevOps = (void *)pNetDev->netdev_ops;
	if (pNetDevOps)
		vfree(pNetDevOps);
}
#endif

INT RtmpOSNetDevAlloc(
	IN PNET_DEV *new_dev_p,
	IN UINT32 privDataSize)
{
	*new_dev_p = NULL;
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			 "Allocate a net device with private data size=%d!\n",
			  privDataSize);
#if LINUX_VERSION_CODE <= 0x20402	/* Red Hat 7.1 */
	*new_dev_p = alloc_netdev(privDataSize, "eth%d", ether_setup);
#else
	*new_dev_p = alloc_etherdev(privDataSize);
#endif /* LINUX_VERSION_CODE */

	if (*new_dev_p)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
INT RtmpOSNetDevOpsAlloc(PVOID *pNetDevOps)
{
	*pNetDevOps = (PVOID) vmalloc(sizeof(struct net_device_ops));

	if (*pNetDevOps) {
		os_zero_mem(*pNetDevOps, sizeof(struct net_device_ops));
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}
#endif


PNET_DEV RtmpOSNetDevGetByName(PNET_DEV pNetDev, RTMP_STRING *pDevName)
{
	PNET_DEV pTargetNetDev = NULL;
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 26) <= LINUX_VERSION_CODE)
	pTargetNetDev = dev_get_by_name(dev_net(pNetDev), pDevName);
#else
	ASSERT(pNetDev);
	pTargetNetDev = dev_get_by_name(pNetDev->nd_net, pDevName);
#endif
#else
	pTargetNetDev = dev_get_by_name(pDevName);
#endif /* KERNEL_VERSION(2,6,24) */
#else
	int devNameLen;

	devNameLen = strlen(pDevName);
	ASSERT((devNameLen <= IFNAMSIZ));

	for (pTargetNetDev = dev_base; pTargetNetDev != NULL;
		 pTargetNetDev = pTargetNetDev->next) {
		if (strncmp(pTargetNetDev->name, pDevName, devNameLen) == 0)
			break;
	}

#endif /* KERNEL_VERSION(2,5,0) */
	return pTargetNetDev;
}


void RtmpOSNetDeviceRefPut(PNET_DEV pNetDev)
{
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)

	/*
	 *  every time dev_get_by_name is called, and it has returned a valid struct
	 *  net_device*, dev_put should be called afterwards, because otherwise the
	 *  machine hangs when the device is unregistered (since dev->refcnt > 1).
	 */
	if (pNetDev)
		dev_put(pNetDev);

#endif /* LINUX_VERSION_CODE */
}


INT RtmpOSNetDevDestory(VOID *pReserved, PNET_DEV pNetDev)
{
	/* TODO: Need to fix this */
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR,
			"WARNING: This function not implement yet!!!\n");
	return 0;
}


void RtmpOSNetDevDetach(PNET_DEV pNetDev)
{
#if defined(CCN34_SPLIT_MAC_SUPPORT) && defined(ACTION_MONITOR_SUPPORT)
	/* ccn34 action monitor is not support cfg80211 interface for now */
	unregister_netdevice(pNetDev);
	return;
#endif
#if defined(RT_CFG80211_SUPPORT) && defined(BACKPORT_NOSTDINC)
	cfg80211_unregister_netdevice(pNetDev);
#else
	unregister_netdevice(pNetDev);
#endif
}


void RtmpOSNetDevProtect(BOOLEAN lock_it)
{
	if (lock_it)
		rtnl_lock();
	else
		rtnl_unlock();
}

#if (KERNEL_VERSION(2, 6, 18) <= LINUX_VERSION_CODE)
static void RALINK_ET_DrvInfoGet(
	struct net_device *pDev,
	struct ethtool_drvinfo *pInfo)
{
	size_t size = sizeof("RALINK WLAN");
	INT ret;

	strlcpy(pInfo->driver, "RALINK WLAN", size);
	ret = snprintf(pInfo->bus_info, sizeof(pInfo->bus_info),
			"CSR 0x%lx", pDev->base_addr);
	if (os_snprintf_error(sizeof(pInfo->bus_info), ret) != 0)
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL,
			DBG_LVL_ERROR, "%s: snprintf error", __func__);
}

static struct ethtool_ops RALINK_Ethtool_Ops = {
	.get_drvinfo = RALINK_ET_DrvInfoGet,
};
#endif

int RtmpOSNetDevAttach(
	IN UCHAR OpMode,
	IN PNET_DEV pNetDev,
	IN RTMP_OS_NETDEV_OP_HOOK * pDevOpHook)
{
	int ret, rtnl_locked = FALSE;
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	struct net_device_ops *pNetDevOps = (struct net_device_ops *)pNetDev->netdev_ops;
#endif
	PRTMP_ADAPTER pAd = NULL;

	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO, "--->\n");

	/* If we need hook some callback function to the net device structrue, now do it. */
	if (pDevOpHook) {
		GET_PAD_FROM_NET_DEV(pAd, pNetDev);
		if (pAd->net_dev == pNetDev) {
			/* register mac address */
			pAd->bLocalAdminMAC = FALSE;
			if ((NdisEqualMemory(ZERO_MAC_ADDR, PD_GET_ADMIN_ADDRESS(pAd->physical_dev), MAC_ADDR_LEN) == FALSE) &&
			(hc_get_hw_band_idx(pAd) == BAND0)) {
				COPY_MAC_ADDR(pAd->CurrentAddress, PD_GET_ADMIN_ADDRESS(pAd->physical_dev));
				COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
				pAd->bLocalAdminMAC = TRUE;
			}

			if ((NdisEqualMemory(ZERO_MAC_ADDR, PD_GET_ADMIN_ADDRESS1(pAd->physical_dev), MAC_ADDR_LEN) == FALSE) &&
			(hc_get_hw_band_idx(pAd) == BAND1)) {
				COPY_MAC_ADDR(pAd->CurrentAddress, PD_GET_ADMIN_ADDRESS1(pAd->physical_dev));
				COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
				pAd->bLocalAdminMAC = TRUE;
			} else if ((NdisEqualMemory(ZERO_MAC_ADDR, PD_GET_ADMIN_ADDRESS(pAd->physical_dev), MAC_ADDR_LEN) == FALSE) &&
			(hc_get_hw_band_idx(pAd) == BAND1)) {
				PD_GET_ADMIN_ADDRESS(pAd->physical_dev)[0] |= (0x8);
				COPY_MAC_ADDR(pAd->CurrentAddress, PD_GET_ADMIN_ADDRESS(pAd->physical_dev));
				COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
				pAd->bLocalAdminMAC = TRUE;
			}

			if ((NdisEqualMemory(ZERO_MAC_ADDR, PD_GET_ADMIN_ADDRESS2(pAd->physical_dev), MAC_ADDR_LEN) == FALSE) &&
			(hc_get_hw_band_idx(pAd) == BAND2)) {
				COPY_MAC_ADDR(pAd->CurrentAddress, PD_GET_ADMIN_ADDRESS2(pAd->physical_dev));
				COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
				pAd->bLocalAdminMAC = TRUE;
			} else if ((NdisEqualMemory(ZERO_MAC_ADDR, PD_GET_ADMIN_ADDRESS(pAd->physical_dev), MAC_ADDR_LEN) == FALSE) &&
			(hc_get_hw_band_idx(pAd) == BAND2)) {
				PD_GET_ADMIN_ADDRESS(pAd->physical_dev)[0] |= (0xc);
				COPY_MAC_ADDR(pAd->CurrentAddress, PD_GET_ADMIN_ADDRESS(pAd->physical_dev));
				COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
				pAd->bLocalAdminMAC = TRUE;
			}

		}

#ifdef RT_CFG80211_SUPPORT
		RTMP_DRIVER_CFG80211_REGISTER(pNetDev);
#endif /* RT_CFG80211_SUPPORT */
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
		pNetDevOps->ndo_open = pDevOpHook->open;
		pNetDevOps->ndo_stop = pDevOpHook->stop;
		pNetDevOps->ndo_start_xmit =
			(HARD_START_XMIT_FUNC) (pDevOpHook->xmit);
		pNetDevOps->ndo_do_ioctl = pDevOpHook->ioctl;
#ifdef HOSTAPD_MBSS_SUPPORT
		pNetDevOps->ndo_set_mac_address = pDevOpHook->set_mac_addr;
#endif /*HOSTAPD_MBSS_SUPPORT*/
#else
		pNetDev->open = pDevOpHook->open;
		pNetDev->stop = pDevOpHook->stop;
		pNetDev->hard_start_xmit =
			(HARD_START_XMIT_FUNC) (pDevOpHook->xmit);
		pNetDev->do_ioctl = pDevOpHook->ioctl;
#endif
#if (KERNEL_VERSION(2, 6, 18) <= LINUX_VERSION_CODE)
		pNetDev->ethtool_ops = &RALINK_Ethtool_Ops;
#endif

		/* if you don't implement get_stats, just leave the callback function as NULL, a dummy
		  *  function will make kernel panic.
		  */
		if (pDevOpHook->get_stats)
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
			pNetDevOps->ndo_get_stats = pDevOpHook->get_stats;

#else
			pNetDev->get_stats = pDevOpHook->get_stats;
#endif
		/* OS specific flags, here we used to indicate if we are virtual interface */
		/*		pNetDev->priv_flags = pDevOpHook->priv_flags; */
		RT_DEV_PRIV_FLAGS_SET(pNetDev, pDevOpHook->priv_flags);
#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
		/*		pNetDev->get_wireless_stats = rt28xx_get_wireless_stats; */
		pNetDev->get_wireless_stats = pDevOpHook->get_wstats;
#endif
#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12

		if (OpMode == OPMODE_STA)
			pDevOpHook->iw_handler = (void *)&rt28xx_iw_handler_def;

#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12

		if (OpMode == OPMODE_AP)
			pDevOpHook->iw_handler = (void *)&rt28xx_ap_iw_handler_def;

#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef CONFIG_WIRELESS_EXT

		if (pDevOpHook->iw_handler)
			pNetDev->wireless_handlers = pDevOpHook->iw_handler;

#endif /* CONFIG_WIRELESS_EXT */
		/* copy the net device mac address to the net_device structure. */
		dev_addr_set(pNetDev, &pDevOpHook->devAddr[0]);
		rtnl_locked = pDevOpHook->needProtcted;
	}

#if (KERNEL_VERSION(2, 6, 24) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	pNetDevOps->ndo_validate_addr = NULL;
	/*pNetDev->netdev_ops = ops; */
#else
	pNetDev->validate_addr = NULL;
#endif
#endif

#ifdef CFG_JUMBO_SUPPORT
	pNetDev->max_mtu = MAX_MTU_SIZE;
#endif /* CFG_JUMBO_SUPPORT */

#ifdef ACTION_MONITOR_SUPPORT
	if (pDevOpHook && pDevOpHook->priv_flags == INT_ACT_MONITOR) {
		if (rtnl_locked)
			ret = register_netdevice(pNetDev);
		else
			ret = register_netdev(pNetDev);
		netif_stop_queue(pNetDev);
		return ret;
	}
#endif

#if	defined(RT_CFG80211_SUPPORT) && defined(BACKPORT_NOSTDINC)
	netdev_set_priv_destructor(pNetDev, RtmpOSNetDevOpsFree);
	if (rtnl_locked) {
		ret = cfg80211_register_netdevice(pNetDev);
	} else {
		if (rtnl_lock_killable())
			return -EINTR;
		ret = cfg80211_register_netdevice(pNetDev);
		rtnl_unlock();
	}
#else
	if (rtnl_locked)
		ret = register_netdevice(pNetDev);
	else
		ret = register_netdev(pNetDev);
#endif

	netif_stop_queue(pNetDev);
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
		"<---ret=%d\n", ret);

	if (ret == 0)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


PNET_DEV RtmpOSNetDevCreate(
	IN INT32 MC_RowID,
	IN UINT32 *pIoctlIF,
	IN INT devType,
	IN INT devNum,
	IN INT privMemSize,
	IN char *pNamePrefix,
	IN BOOLEAN autoSuffix)
{
	struct net_device *pNetDev = NULL;
#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	struct net_device_ops *pNetDevOps = NULL;
#endif
	int status;
	/* allocate a new network device */
	status = RtmpOSNetDevAlloc(&pNetDev, privMemSize);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
			"Allocate network device fail (%s)...\n", pNamePrefix);
		return NULL;
	}

#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
	status = RtmpOSNetDevOpsAlloc((PVOID) &pNetDevOps);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
			"Allocate net device ops fail!\n");
		RtmpOSNetDevFree(pNetDev);
		return NULL;
	}
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
		"Allocate net device ops success!\n");
	pNetDev->netdev_ops = pNetDevOps;

#endif
	/* find a available interface name, max 32 interfaces */
#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(HOSTAPD_MBSS_SUPPORT)
	if (devType == INT_MBSSID) {
		os_zero_mem(&pNetDev->name[0], IFNAMSIZ);
		strlcpy(&pNetDev->name[0], pNamePrefix, sizeof(pNetDev->name));
	} else
#endif /* IWCOMMAND_CFG80211_SUPPORT */
	{
		status = RtmpOSNetDevRequestName(MC_RowID, pIoctlIF, pNetDev, pNamePrefix, devNum, autoSuffix);

		if (status != NDIS_STATUS_SUCCESS) {
		/* error! no any available ra name can be used! */
			MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_ERROR,
				 "Assign inf name (%s with suffix 0~32) failed\n", pNamePrefix);
			RtmpOSNetDevFree(pNetDev);
			return NULL;
		}
	}
	MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_UP, DBG_LVL_INFO,
		"The name of the new %s interface is %s\n",
			 pNamePrefix, pNetDev->name);

	return pNetDev;
}


UCHAR VLAN_8023_Header_Copy(
	IN USHORT VLAN_VID,
	IN USHORT VLAN_Priority,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	OUT PUCHAR pData,
	IN UCHAR *TPID)
{
	UINT16 TCI;
	UCHAR VLAN_Size = 0;

	if (VLAN_VID != 0) {
		/* need to insert VLAN tag */
		VLAN_Size = LENGTH_802_1Q;
		/* make up TCI field */
		TCI = (VLAN_VID & 0x0fff) | ((VLAN_Priority & 0x7) << 13);
#ifndef CFG_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif /* CFG_BIG_ENDIAN */
		/* copy dst + src MAC (12B) */
		memcpy(pData, pHeader802_3, LENGTH_802_3_NO_TYPE);
		/* copy VLAN tag (4B) */
		/* do NOT use memcpy to speed up */
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE) = *(UINT16 *) TPID;
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE + 2) = TCI;
		/* copy type/len (2B) */
		*(UINT16 *) (pData + LENGTH_802_3_NO_TYPE + LENGTH_802_1Q) =
			*(UINT16 *) &pHeader802_3[LENGTH_802_3 -
									  LENGTH_802_3_TYPE];

		/* copy tail if exist */
		if (HdrLen > LENGTH_802_3)
			memcpy(pData + LENGTH_802_3 + LENGTH_802_1Q, pHeader802_3 + LENGTH_802_3, HdrLen - LENGTH_802_3);
	} else {
		/* no VLAN tag is needed to insert */
		memcpy(pData, pHeader802_3, HdrLen);
	}

	return VLAN_Size;
}


/*
 * ========================================================================
 * Routine Description:
 *    Allocate memory for adapter control block.
 *
 * Arguments:
 *    pAd					Pointer to our adapter
 *
 * Return Value:
 *	NDIS_STATUS_SUCCESS
 *	NDIS_STATUS_FAILURE
 *	NDIS_STATUS_RESOURCES
 * ========================================================================
 */
NDIS_STATUS AdapterBlockAllocateMemory(VOID *handle, VOID **ppAd, UINT32 SizeOfpAd)
{
	*ppAd = (PVOID) vmalloc(SizeOfpAd);

	if (*ppAd) {
		os_zero_mem(*ppAd, SizeOfpAd);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}


/* ========================================================================== */

#ifdef VLAN_SUPPORT
VOID* RtmpOsVLANInsertTag(PNDIS_PACKET pPacket, UINT16 tci)
{
	return (VOID*)vlan_insert_tag(RTPKT_TO_OSPKT(pPacket), cpu2be16(ETH_TYPE_VLAN), tci);
}
#endif /*VLAN_SUPPORT*/

UINT RtmpOsWirelessExtVerGet(VOID)
{
	return WIRELESS_EXT;
}


VOID RtmpDrvAllMacPrint(
	IN VOID *pReserved,
	IN UINT32 *pBufMac,
	IN UINT32 AddrStart,
	IN UINT32 AddrEnd,
	IN UINT32 AddrStep)
{
	struct file *file_w;
	RTMP_STRING *fileName = "MacDump.txt";
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	mm_segment_t orig_fs;
#endif
	RTMP_STRING *msg;
	UINT32 macAddr = 0, macValue = 0;
	INT ret;

	os_alloc_mem(NULL, (UCHAR **)&msg, 1024);

	if (!msg)
		return;

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	orig_fs = get_fs();
	set_fs(KERNEL_DS);
#endif

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_ERR(file_w)) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_INFO,
				 "-->2) %s: Error %ld opening %s\n", __func__,
				  -PTR_ERR(file_w), fileName);
	} else {
		if (file_w->f_op) {
			file_w->f_pos = 0;
			macAddr = AddrStart;

			while (macAddr <= AddrEnd) {
				/*				RTMP_IO_READ32(pAd->hdev_ctrl, macAddr, &macValue); // sample */
				macValue = *pBufMac++;
				ret = snprintf(msg, 1024, "%04x = %08x\n", macAddr, macValue);
				if (os_snprintf_error(1024, ret) != 0)
					MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL,
						DBG_LVL_ERROR, "%s: snprintf error", __func__);
				/* write data to file */
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
			if (file_w->f_op->write) {
				file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);
			} else{
				MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_ERROR,
					"no file write method\n");
			}
#elif (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE)
			__kernel_write(file_w, msg, strlen(msg), &file_w->f_pos);
#elif (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
			kernel_write(file_w, msg, strlen(msg), &file_w->f_pos);
#else
			__vfs_write(file_w, msg, strlen(msg), &file_w->f_pos);
#endif
				MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_INFO, "%s", msg);
				macAddr += AddrStep;
			}

			ret = snprintf(msg, 1024, "\nDump all MAC values to %s\n", fileName);
			if (os_snprintf_error(1024, ret) != 0)
				MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL,
					DBG_LVL_ERROR, "%s: snprintf error", __func__);
		}

		filp_close(file_w, NULL);
	}

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	set_fs(orig_fs);
#endif
	os_free_mem(msg);
}


VOID RtmpDrvAllE2PPrint(
	IN VOID *pReserved,
	IN USHORT *pMacContent,
	IN UINT32 AddrEnd,
	IN UINT32 AddrStep)
{
	struct file *file_w;
	RTMP_STRING *fileName = "EEPROMDump.txt";
#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	mm_segment_t orig_fs;
#endif
	RTMP_STRING *msg;
	USHORT eepAddr = 0;
	USHORT eepValue;
	INT ret;

	os_alloc_mem(NULL, (UCHAR **)&msg, 1024);

	if (!msg)
		return;

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	orig_fs = get_fs();
	set_fs(KERNEL_DS);
#endif

	/* open file */
	file_w = filp_open(fileName, O_WRONLY | O_CREAT, 0);

	if (IS_ERR(file_w)) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO,
				 "-->2) %s: Error %ld opening %s\n", __func__,
				  -PTR_ERR(file_w), fileName);
	} else {
		if (file_w->f_op) {
			file_w->f_pos = 0;
			eepAddr = 0x00;

			while (eepAddr <= AddrEnd) {
				eepValue = *pMacContent;
				ret = snprintf(msg, 1024, "%08x = %04x\n", eepAddr, eepValue);
				if (os_snprintf_error(1024, ret) != 0)
					MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM,
						DBG_LVL_ERROR, "%s: snprintf error", __func__);
				/* write data to file */
#if (KERNEL_VERSION(4, 1, 0) > LINUX_VERSION_CODE)
				if (file_w->f_op->write) {
					file_w->f_op->write(file_w, msg, strlen(msg), &file_w->f_pos);
				} else{
					MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_ERROR,
						"no file write method\n");
				}
#elif (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE && KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE)
				__kernel_write(file_w, msg, strlen(msg), &file_w->f_pos);
#elif (KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE)
				kernel_write(file_w, msg, strlen(msg), &file_w->f_pos);
#else
				__vfs_write(file_w, msg, strlen(msg), &file_w->f_pos);
#endif
				MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_INFO, "%s", msg);
				eepAddr += AddrStep;
				pMacContent += (AddrStep >> 1);
			}

			ret = snprintf(msg, 1024, "\nDump all EEPROM values to %s\n",
					fileName);
			if (os_snprintf_error(1024, ret) != 0)
				MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM,
					DBG_LVL_ERROR, "%s: snprintf error\n", __func__);
		}

		filp_close(file_w, NULL);
	}

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	set_fs(orig_fs);
#endif
	os_free_mem(msg);
}


/*
 * ========================================================================
 * Routine Description:
 * Wake up the command thread.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsCmdUp(RTMP_OS_TASK *pCmdQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pCmdQTask);
#ifdef KTHREAD_SUPPORT
	pTask->kthread_running = TRUE;
	wake_up(&pTask->kthread_q);
#else
	if (CHECK_PID_LEGALITY(pTask->taskPID))
		RTMP_SEM_EVENT_UP(&(pTask->taskSema));
#endif /* KTHREAD_SUPPORT */
}


/*
 * ========================================================================
 * Routine Description:
 * Wake up USB Mlme thread.
 *
 * Arguments:
 * pAd                          - WLAN control block pointer
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsMlmeUp(IN RTMP_OS_TASK *pMlmeQTask)
{
	OS_TASK *pTask = RTMP_OS_TASK_GET(pMlmeQTask);
#ifdef KTHREAD_SUPPORT

	if ((pTask != NULL) && (pTask->kthread_task)) {
		pTask->kthread_running = TRUE;
		wake_up(&pTask->kthread_q);
	} else {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
				 "invalid pTask 0x%p or kthread_task 0x%p\n",
				  pTask,
				  (pTask) ? pTask->kthread_task : NULL);
	}

#else

	if (pTask != NULL) {
		if (CHECK_PID_LEGALITY(pTask->taskPID))
			RTMP_SEM_EVENT_UP(&(pTask->taskSema));
	}

#endif /* KTHREAD_SUPPORT */
}


/*
 * ========================================================================
 * Routine Description:
 * Check if the file is error.
 *
 * Arguments:
 * pFile                        - the file
 *
 * Return Value:
 * OK or any error
 *
 * Note:
 * rt_linux.h, not rt_drv.h
 * ========================================================================
 */
INT32 RtmpOsFileIsErr(IN VOID *pFile)
{
	return IS_FILE_OPEN_ERR(pFile);
}

/*
 * ========================================================================
 * Routine Description:
 * Enable or disable wireless event sent.
 *
 * Arguments:
 * pReserved            - Reserved
 * FlgIsWEntSup - TRUE or FALSE
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsWlanEventSet(
	IN VOID *pReserved,
	IN BOOLEAN *pCfgWEnt,
	IN BOOLEAN FlgIsWEntSup)
{
#if WIRELESS_EXT >= 15
	/*	pAd->CommonCfg.bWirelessEvent = FlgIsWEntSup; */
	*pCfgWEnt = FlgIsWEntSup;
#else
	*pCfgWEnt = 0;		/* disable */
#endif
}

/*
 * ========================================================================
 * Routine Description:
 * vmalloc
 *
 * Arguments:
 * Size                 - memory size
 *
 * Return Value:
 * the memory
 *
 * Note:
 * ========================================================================
 */
VOID *RtmpOsVmalloc(ULONG Size)
{
	return vmalloc(Size);
}

/*
 * ========================================================================
 * Routine Description:
 * vfree
 *
 * Arguments:
 * pMem                 - the memory
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsVfree(VOID *pMem)
{
	if (pMem != NULL)
		vfree(pMem);
}


/*
 * ========================================================================
 * Routine Description:
 * Assign protocol to the packet.
 *
 * Arguments:
 * pPkt                 - the packet
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktProtocolAssign(PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
}


BOOLEAN RtmpOsStatsAlloc(
	IN VOID **ppStats,
	IN VOID **ppIwStats)
{
	os_alloc_mem(NULL, (UCHAR **) ppStats, sizeof(struct net_device_stats));

	if ((*ppStats) == NULL)
		return FALSE;

	os_zero_mem((UCHAR *) *ppStats, sizeof(struct net_device_stats));
#if WIRELESS_EXT >= 12
	os_alloc_mem(NULL, (UCHAR **) ppIwStats, sizeof(struct iw_statistics));

	if ((*ppIwStats) == NULL) {
		os_free_mem(*ppStats);
		return FALSE;
	}

	os_zero_mem((UCHAR *)*ppIwStats, sizeof(struct iw_statistics));
#endif
	return TRUE;
}

#ifdef CONFIG_SPECIAL_RPS_SUPPORT
unsigned int rps_cpu_idx[] = { 0x10000000, 0xF0000000, 0x80000000, 0x40000000};
unsigned int change_idx = 0, g_accnt = 0;

inline BOOLEAN is_udp(PNDIS_PACKET pPacket)
{
	PUCHAR pSrcBuf;

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);

	if (pSrcBuf[0] == 0x45 && pSrcBuf[1] == 0x0) {
		if (pSrcBuf[9] == IP_PROTO_UDP)
			return TRUE;
	}
	return FALSE;
}
#endif /* CONFIG_SPECIAL_RPS_SUPPORT */
/*
 * ========================================================================
 * Routine Description:
 * Pass the received packet to OS.
 *
 * Arguments:
 * pPkt                 - the packet
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsPktRcvHandle(PNDIS_PACKET pNetPkt, VOID *napi)
{
	struct sk_buff *skb = RTPKT_TO_OSPKT(pNetPkt);
#if defined(CONFIG_CSO_SUPPORT) || defined(CONFIG_SPECIAL_RPS_SUPPORT)
	struct net_device *pNetDev =  GET_OS_PKT_NETDEV(pNetPkt);
#endif /* CONFIG_CSO_SUPPORT */
#ifdef CONFIG_SPECIAL_RPS_SUPPORT
	struct netdev_rx_queue *rxqueue = pNetDev->_rx;
	struct rps_map *map;
#endif /* CONFIG_SPECIAL_RPS_SUPPORT */
	PRTMP_ADAPTER pAd = NULL;
	struct tx_rx_ctl *tr_ctl;
	struct tr_counter *tr_cnt;

	GET_PAD_FROM_NET_DEV(pAd, GET_OS_PKT_NETDEV(pNetPkt));
	tr_ctl = &pAd->tr_ctl;
	tr_cnt = &tr_ctl->tr_cnt;

	WLAN_HOOK_CALL(WLAN_HOOK_RX_DRIVER_TO_OS, NULL, skb);

#ifdef CONFIG_CSO_SUPPORT

	if (pNetDev->features & NETIF_F_HW_CSUM) {
		if (RTMP_GET_TCP_CHKSUM_FAIL(pNetPkt))
			skb->ip_summed = CHECKSUM_NONE;
		else
			skb->ip_summed = CHECKSUM_UNNECESSARY;
	}

#endif /* CONFIG_CSO_SUPPORT */

	tr_cnt->rx_to_os_cnt++;

#ifdef CONFIG_SPECIAL_RPS_SUPPORT
	if (pAd->rps_special_hash_en) {
		map = rcu_dereference(rxqueue->rps_map);
		if (!map || map->len >= 4 || map->len == 0)
			goto done;
		if (map->len == 1) {
			skb->sw_hash = 1;
			skb->hash = rps_cpu_idx[0];
		} else if (is_udp(pNetPkt)) {
			if (g_accnt++ > pAd->rps_per_round) {
				g_accnt = 0;
				if (change_idx++ >= map->len)
					change_idx = 0;
			}
			skb->sw_hash = 1;
			skb->hash = rps_cpu_idx[change_idx];
		}
	}
done:
#endif /* CONFIG_SPECIAL_RPS_SUPPORT */

	if (napi && in_serving_softirq())
		napi_gro_receive((struct napi_struct *)napi, skb);
	else {
#ifdef IXIA_C50_MODE
		if (IS_EXPECTED_LENGTH(pAd, skb->len + 14))
			pAd->rx_cnt.rx_pkt_to_os[smp_processor_id()]++;
#endif
		netif_rx(skb);
	}
}

UINT32 RtmpOsCsumAdd(UINT32 csum, UINT32 addend)
{
	UINT32 res = csum;
	res += addend;
	return res + (res < addend);
}

VOID RtmpOsSkbPullRcsum(struct sk_buff *skb, unsigned int len)
{
	if (len > skb->len)
		return;

	skb_pull(skb, len);
	if (skb->ip_summed == CHECKSUM_COMPLETE)
		skb->csum = RtmpOsCsumAdd(skb->csum, ~csum_partial(skb->data, len, 0));
	else if (skb->ip_summed == CHECKSUM_PARTIAL &&
		 (skb->csum_start - (skb->data - skb->head)) < 0)
		skb->ip_summed = CHECKSUM_NONE;
}

VOID RtmpOsSkbResetMacHeader(struct sk_buff *skb)
{
	skb_reset_mac_header(skb);
}

VOID RtmpOsSkbResetNetworkHeader(struct sk_buff *skb)
{
	skb_reset_network_header(skb);
}

VOID RtmpOsSkbResetTransportHeader(struct sk_buff *skb)
{
	skb_reset_transport_header(skb);
}

VOID RtmpOsSkbResetMacLen(struct sk_buff *skb)
{
	skb_reset_mac_len(skb);
}

/*
 * ========================================================================
 * Routine Description:
 * Get the network interface for the packet.
 *
 * Arguments:
 * pPkt                 - the packet
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
PNET_DEV RtmpOsPktNetDevGet(VOID *pPkt)
{
	return GET_OS_PKT_NETDEV(pPkt);
}


#ifdef IAPP_SUPPORT
/* Layer 2 Update frame to switch/bridge */
/* For any Layer2 devices, e.g., bridges, switches and other APs, the frame
  * can update their forwarding tables with the correct port to reach the new
  * location of the STA
  */
typedef struct GNU_PACKED _RT_IAPP_L2_UPDATE_FRAME {

	UCHAR DA[ETH_ALEN];	/* broadcast MAC address */
	UCHAR SA[ETH_ALEN];	/* the MAC address of the STA that has just associated or reassociated */
	USHORT Len;		/* 8 octets */
	UCHAR DSAP;		/* null */
	UCHAR SSAP;		/* null */
	UCHAR Control;		/* reference to IEEE Std 802.2 */
	UCHAR XIDInfo[3];	/* reference to IEEE Std 802.2 */
} RT_IAPP_L2_UPDATE_FRAME, *PRT_IAPP_L2_UPDATE_FRAME;


PNDIS_PACKET RtmpOsPktIappMakeUp(
	IN PNET_DEV pNetDev,
	IN UINT8 * pMac)
{
	RT_IAPP_L2_UPDATE_FRAME frame_body;
	INT size = sizeof(RT_IAPP_L2_UPDATE_FRAME);
	PNDIS_PACKET pNetBuf;

	if (pNetDev == NULL)
		return NULL;

	pNetBuf = RtmpOSNetPktAlloc(NULL, size);

	if (!pNetBuf) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_ERROR,
			"Error! Can't allocate a skb.\n");
		return NULL;
	}

	/* init the update frame body */
	os_zero_mem(&frame_body, size);
	memset(frame_body.DA, 0xFF, ETH_ALEN);
	memcpy(frame_body.SA, pMac, ETH_ALEN);
	frame_body.Len = OS_HTONS(ETH_ALEN);
	frame_body.DSAP = 0;
	frame_body.SSAP = 0x01;
	frame_body.Control = 0xAF;
	frame_body.XIDInfo[0] = 0x81;
	frame_body.XIDInfo[1] = 1;
	frame_body.XIDInfo[2] = 1 << 1;
	SET_OS_PKT_NETDEV(pNetBuf, pNetDev);
	skb_reserve(pNetBuf, 2);
	memcpy(skb_put(pNetBuf, size), &frame_body, size);
	return pNetBuf;
}
#endif /* IAPP_SUPPORT */

#ifdef CONFIG_FAST_NAT_SUPPORT
VOID RtmpOsPktNatMagicTag(PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	FOE_MAGIC_TAG(pRxPkt) = FOE_MAGIC_WLAN;
}
#endif /*CONFIG_FAST_NAT_SUPPORT*/

#ifdef WHNAT_SUPPORT
VOID RtmpOsPktNatMagicTagInvalid(PNDIS_PACKET pNetPkt)
{
	struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

	FOE_TAG_PROTECT(pRxPkt) = 0;
}
#endif
/*
 * ========================================================================
 * Routine Description:
 * Flush a data cache line.
 *
 * Arguments:
 * AddrStart            - the start address
 * Size                 - memory size
 *
 * Return Value:
 * None
 *
 * Note:
 * ========================================================================
 */
VOID RtmpOsDCacheFlush(
	IN ULONG AddrStart,
	IN ULONG Size)
{
	RTMP_UTIL_DCACHE_FLUSH(AddrStart, Size);
}


#ifdef CONFIG_STA_SUPPORT
INT RtmpOSNotifyRawData(
	IN PNET_DEV pNetDev,
	IN PUCHAR buff,
	IN INT len,
	IN ULONG type,
	IN USHORT protocol,
	VOID *napi)
{
	struct sk_buff *skb = NULL;

	DEV_ALLOC_SKB(skb, (len + 2));

	if (!skb) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_CTRL, DBG_LVL_ERROR,
			"%s: failed to allocate sk_buff for notification\n", pNetDev->name);
		return -ENOMEM;
	}

	skb_reserve(skb, 2);
	memcpy(skb_put(skb, len), buff, len);
	skb->len = len;
	skb->dev = pNetDev;
#if (KERNEL_VERSION(2, 6, 21) >= LINUX_VERSION_CODE)
	skb->mac.raw = skb->data;
#else
	skb_set_mac_header(skb, 0);
#endif
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = htons(protocol);
	memset(skb->cb, 0, sizeof(skb->cb));

	if (napi && in_serving_softirq())
		napi_gro_receive((struct napi_struct *)napi, skb);
	else
		netif_rx(skb);

	return 0;
}
#endif /* CONFIG_STA_SUPPORT */


inline void OS_SPIN_LOCK_IRQSAVE(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	spin_lock_irqsave((spinlock_t *)(lock), *flags);
}

inline void OS_SPIN_UNLOCK_IRQRESTORE(NDIS_SPIN_LOCK *lock, unsigned long *flags)
{
	spin_unlock_irqrestore((spinlock_t *)(lock), *flags);
}

inline void OS_SPIN_LOCK(NDIS_SPIN_LOCK *lock)
{
	spin_lock((spinlock_t *)(lock));
}

inline void OS_SPIN_UNLOCK(NDIS_SPIN_LOCK *lock)
{
	spin_unlock((spinlock_t *)(lock));
}

inline void OS_SPIN_LOCK_BH(NDIS_SPIN_LOCK *lock)
{
	spin_lock_bh((spinlock_t *)(lock));
}

inline void OS_SPIN_UNLOCK_BH(NDIS_SPIN_LOCK *lock)
{
	spin_unlock_bh((spinlock_t *)(lock));
}

inline void OS_SPIN_LOCK_IRQ(NDIS_SPIN_LOCK *lock)
{
	spin_lock_irq((spinlock_t *)(lock));
}

inline void OS_SPIN_UNLOCK_IRQ(NDIS_SPIN_LOCK *lock)
{
	spin_unlock_irq((spinlock_t *)(lock));
}

inline int OS_TEST_BIT(int bit, ULONG *flags)
{
	return test_bit(bit, flags);
}

inline void OS_SET_BIT(unsigned int bit, ULONG *flags)
{
	set_bit(bit, flags);
}

inline void OS_CLEAR_BIT(unsigned int bit, ULONG *flags)
{
	clear_bit(bit, flags);
}

static struct device *rtmp_get_dev(void *ad)
{
	struct device *dev = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ad;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	struct mtk_mac_hw *hw = &obj->mac_dev->hw;

	dev = (struct device *)hw->pdev;
	return dev;
}



void os_load_code_from_bin(void *pAd, unsigned char **image, char *bin_name, UINT32 *code_len)
{
	struct device *dev;
	const struct firmware *fw_entry;
	char *org_bin_name = NULL;

	if (pAd)
		dev = rtmp_get_dev(pAd);
	else
		dev = NULL;

	if (!bin_name) {
		*image = NULL;
		return;
	}

	org_bin_name = bin_name;

	if ((strncasecmp(bin_name, "/etc/wireless/", 14) == 0) ||
	    (strncasecmp(bin_name, "/lib/firmware/", 14) == 0)) {
		bin_name += 14;
	}

	if (request_firmware(&fw_entry, bin_name, dev) != 0) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_EEPROM, DBG_LVL_ERROR,
				 "fw not available(%s)\n", org_bin_name);
		*image = NULL;
		return;
	}

	os_alloc_mem(pAd, image, fw_entry->size + 1);
	if (*image) {
		memcpy(*image, fw_entry->data, fw_entry->size);
		(*image)[fw_entry->size] = '\0';
		*code_len = fw_entry->size;
	}
	release_firmware(fw_entry);
}



void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfoOrg, BOOLEAN bSet)
{
	__RtmpOSFSInfoChange(pOSFSInfoOrg, bSet);
}


/* timeout -- ms */
VOID RTMP_SetPeriodicTimer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_SetPeriodicTimer(pTimerOrg, timeout);
}


/* convert NdisMInitializeTimer --> RTMP_OS_Init_Timer */
VOID RTMP_OS_Init_Timer(
	VOID *pReserved,
	NDIS_MINIPORT_TIMER *pTimerOrg,
	TIMER_FUNCTION function,
	PVOID data)
{
	__RTMP_OS_Init_Timer(pReserved, pTimerOrg, function, data);
}


VOID RTMP_OS_Add_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Add_Timer(pTimerOrg, timeout);
}


VOID RTMP_OS_Mod_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout)
{
	__RTMP_OS_Mod_Timer(pTimerOrg, timeout);
}


VOID RTMP_OS_Del_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, BOOLEAN *pCancelled)
{
	__RTMP_OS_Del_Timer(pTimerOrg, pCancelled);
}

NDIS_STATUS RtmpOSTaskKill(RTMP_OS_TASK *pTask)
{
	if (!pTask) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_TASK, DBG_LVL_WARN,
			 "pTask is Null, return!!!\n");
		return NDIS_STATUS_FAILURE;
	}
	return __RtmpOSTaskKill(pTask);
}


INT RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTask)
{
	return __RtmpOSTaskNotifyToExit(pTask);
}


void RtmpOSTaskCustomize(RTMP_OS_TASK *pTask)
{
	__RtmpOSTaskCustomize(pTask);
}


NDIS_STATUS RtmpOSTaskAttach(
	RTMP_OS_TASK *pTask,
	RTMP_OS_TASK_CALLBACK fn,
	ULONG arg)
{
	return __RtmpOSTaskAttach(pTask, fn, arg);
}


NDIS_STATUS RtmpOSTaskInit(
	RTMP_OS_TASK *pTask,
	RTMP_STRING *pTaskName,
	VOID *pPriv,
	LIST_HEADER *pTaskList,
	LIST_HEADER *pSemList)
{
	return __RtmpOSTaskInit(pTask, pTaskName, pPriv, pSemList);
}


BOOLEAN RtmpOSTaskWait(VOID *pReserved, RTMP_OS_TASK *pTask, INT32 *pStatus)
{
	return __RtmpOSTaskWait(pReserved, pTask, pStatus);
}

BOOLEAN RtmpOSTaskWaitCond(VOID *pReserved, RTMP_OS_TASK *pTask, UINT32 u4Cond, INT32 *pStatus)
{
	return __RtmpOSTaskWaitCond(pReserved, pTask, u4Cond, pStatus);
}

VOID RtmpOsTaskWakeUp(RTMP_OS_TASK *pTask)
{
#ifdef KTHREAD_SUPPORT
	WAKE_UP(pTask);
#else
	RTMP_SEM_EVENT_UP(&pTask->taskSema);
#endif
}

/* pAd MUST allow to be NULL */

#ifdef MEM_ALLOC_INFO_SUPPORT
static VOID MemInfoListInital(VOID)
{
	MemInfoList.type = 0;
	PktInfoList.type = 1;
	MIListInit(&MemInfoList);
	MIListInit(&PktInfoList);
}

UINT32 ShowMemAllocInfo(UINT show, UINT64 pCaller)
{
	ShowMIList(&MemInfoList, show, pCaller);
	return MemInfoList.EntryNumber;
}


UINT32 ShowPktAllocInfo(UINT show, UINT64 pCaller)
{
	ShowMIList(&PktInfoList, show, pCaller);
	return PktInfoList.EntryNumber;
}

static VOID MemInfoListExit(VOID)
{
	UINT32 memalctotal, pktalctotal;

	memalctotal = ShowMemAllocInfo(SHOW_ACTIVE_PCALLER_INFO, 0);
	pktalctotal = ShowPktAllocInfo(SHOW_ACTIVE_PCALLER_INFO, 0);

	if ((memalctotal != 0) || (pktalctotal != 0)) {
		MTWF_DBG(NULL, DBG_CAT_INTF, CATINTF_DOWN, DBG_LVL_ERROR,
				"Error: Memory leak  mem size=%d pkt num=%d!!\n", memalctotal, pktalctotal);
		ASSERT(0);
	}

	MIListExit(&MemInfoList);
	MIListExit(&PktInfoList);
}
#endif /* MEM_ALLOC_INFO_SUPPORT */

/*Unify Utility APIs*/
INT os_alloc_mem(
	VOID *pAd,
	UCHAR **mem,
	ULONG size)
{
	*mem = (PUCHAR) kmalloc(size, GFP_ATOMIC);

	if (*mem) {
#ifdef VENDOR_FEATURE4_SUPPORT
		OS_NumOfMemAlloc++;
#endif /* VENDOR_FEATURE4_SUPPORT */
#ifdef MEM_ALLOC_INFO_SUPPORT
		MIListAddHead(&MemInfoList, size, *mem, __func__, __LINE__, __builtin_return_address(0));
#endif /* MEM_ALLOC_INFO_SUPPORT */
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

VOID BTM_Free_Peer_Entry(PVOID mem)
{
	RALINK_TIMER_STRUCT *pTimer_req = NULL, *pTimer_rsp = NULL, *pTimer_APrsp = NULL;
	BOOLEAN Cancelled = FALSE;
	BTM_PEER_ENTRY *BtmPeerEntry = NULL;

	BtmPeerEntry = (BTM_PEER_ENTRY *)mem;
	pTimer_req = &BtmPeerEntry->WaitPeerBTMReqTimer;
	pTimer_rsp = &BtmPeerEntry->WaitPeerBTMRspTimer;
#ifdef CONFIG_STA_SUPPORT
	pTimer_APrsp = &BtmPeerEntry->WaitAPBTMRspTimer;
#endif /* CONFIG_AP_SUPPORT */

	if (pTimer_req->Valid) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BTM, DBG_LVL_ERROR,
			"WaitPeerBTMReqTimer isn't release!!, Caller = %pS\n",
				 pTimer_req->pCaller);

		/* Cancel Wait peer wnm request frame */
		RTMPCancelTimer(&BtmPeerEntry->WaitPeerBTMReqTimer, &Cancelled);
		RTMPReleaseTimer(&BtmPeerEntry->WaitPeerBTMReqTimer, &Cancelled);
		if (FALSE == Cancelled)
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BTM, DBG_LVL_ERROR,
				"WaitPeerBTMReqTimer release Fail\n");
		else
			Cancelled = FALSE;
	}

	if (pTimer_rsp->Valid) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BTM, DBG_LVL_ERROR,
			"WaitPeerBTMRspTimer isn't release!!, Caller = %pS\n",
			pTimer_rsp->pCaller);

		/* Cancel Wait peer wnm response frame */
		RTMPCancelTimer(&BtmPeerEntry->WaitPeerBTMRspTimer, &Cancelled);
		RTMPReleaseTimer(&BtmPeerEntry->WaitPeerBTMRspTimer, &Cancelled);
		if (FALSE == Cancelled)
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BTM, DBG_LVL_ERROR,
				" WaitPeerBTMRspTimer release Fail\n");
	}
#ifdef CONFIG_STA_SUPPORT
	if (pTimer_APrsp->Valid) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BTM, DBG_LVL_ERROR,
				"WaitAPBTMRspTimer isn't release!!, Caller = %pS\n",
				pTimer_APrsp->pCaller);

		/* Cancel Wait peer wnm response frame */
		RTMPCancelTimer(&BtmPeerEntry->WaitAPBTMRspTimer, &Cancelled);
		RTMPReleaseTimer(&BtmPeerEntry->WaitAPBTMRspTimer, &Cancelled);
		if (FALSE == Cancelled)
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BTM, DBG_LVL_ERROR,
				"WaitAPBTMRspTimer release Fail\n");
	}
#endif /* CONFIG_AP_SUPPORT */

	os_free_mem(BtmPeerEntry);

}

VOID os_free_mem(
	PVOID mem)
{
#ifdef MEM_ALLOC_INFO_SUPPORT
	MEM_INFO_LIST_ENTRY *delEntry;

	delEntry = MIListRemove(&MemInfoList, mem, __builtin_return_address(0));

	if (delEntry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
				 "the memory has not been allocated\n");
		MTWF_DBG(NULL, DBG_CAT_INIT, CATINIT_MLME, DBG_LVL_ERROR,
				 "mem addr = %p, caller is %pS\n", mem, __builtin_return_address(0));
		dump_stack();
	}
#endif /* MEM_ALLOC_INFO_SUPPORT */
	ASSERT(mem);
	kfree(mem);

#ifdef VENDOR_FEATURE4_SUPPORT
	OS_NumOfMemFree++;
#endif /* VENDOR_FEATURE4_SUPPORT */
}

VOID os_zero_mem(
	PVOID ptr,
	ULONG length)
{
	memset(ptr, 0, length);
}

VOID os_move_mem(
	PVOID pDest,
	PVOID pSrc,
	ULONG length)
{
	memmove(pDest, pSrc, length);
}

VOID os_fill_mem(
	PVOID pBuf,
	ULONG Length,
	UCHAR Fill)
{
	memset(pBuf, Fill, Length);
}

INT os_cmp_mem(
	PVOID Destination,
	PVOID Source,
	ULONG Length)
{
	return memcmp(Destination, Source, Length);
}

INT os_equal_mem(
	PVOID pBuf1,
	PVOID pBuf2,
	ULONG length)
{
	return  !memcmp(pBuf1, pBuf2, length);
}


CHAR *os_str_chr(CHAR *str, INT character)
{
	return strchr(str, character);
}

CHAR *os_str_pbrk(CHAR *str1, CHAR  *str2)
{
	return strpbrk(str1, str2);
}

UINT32 os_str_spn(CHAR *str1, CHAR *str2)
{
	return strspn(str1, str2);
}

LONG os_str_tol(const CHAR *str, CHAR **endptr, INT32 base)
{
	return simple_strtol(str, (char **)endptr, base);
}

LONG os_str_toul(const CHAR *str, CHAR **endptr, INT32 base)
{
	return simple_strtoul(str, (char **)endptr, base);
}

CHAR *os_str_str(const CHAR *str1, const CHAR *str2)
{
	return strstr(str1, str2);
}

void os_kref_init(os_kref *ref)
{
	kref_init(ref);
}

void os_kref_get(os_kref *ref)
{
	kref_get(ref);
}

int os_kref_put(os_kref *ref, void (*release)(os_kref *ref))
{
	return kref_put(ref, release);
}

int os_idrm_init(struct os_idr_mgmt *idrm, u32 low, u32 high)
{
	OS_NdisAllocateSpinLock(&idrm->lock);
	idr_init(&idrm->id);

	NdisAcquireSpinLock(&idrm->lock);
	idrm->cnt = 0;
	idrm->low = low;
	idrm->high = high;
	NdisReleaseSpinLock(&idrm->lock);

	return 0;
}

void os_idrm_exit(struct os_idr_mgmt *idrm)
{
	NdisAcquireSpinLock(&idrm->lock);
	idr_destroy(&idrm->id);
	NdisReleaseSpinLock(&idrm->lock);
}

int os_idr_register(struct os_idr_mgmt *idrm, u32 *idx, void *data)
{
	int ret;

	spin_lock_bh(&idrm->lock);
	ret = mt_idr_alloc(&idrm->id, data, idrm->low, idrm->high+1, GFP_ATOMIC);
	if (ret < idrm->low) {
		ret = -ENOMEM;
		goto err;
	}
	*idx = ret;
	idrm->cnt++;
	ret = 0;
err:
	spin_unlock_bh(&idrm->lock);
	return ret;
}

void os_idr_unregister(struct os_idr_mgmt *idrm, u32 idx)
{
	spin_lock_bh(&idrm->lock);
	mt_idr_remove(&idrm->id, idx);
	idrm->cnt--;
	spin_unlock_bh(&idrm->lock);
}


/*test only*/
#ifdef DBG_STARVATION
static inline void block_entry_free(struct starv_log_entry *entry)
{
	struct starv_dbg_block *block = entry->block;
	struct starv_log *ctrl = block->ctrl;

	DlListDel(&entry->list);
	if (entry->log)
		os_free_mem(entry->log);
	os_zero_mem(entry, sizeof(struct starv_log_entry));
	DlListAdd(&ctrl->free, &entry->list);
}

static inline void block_init(struct starv_dbg_block *block)
{
	struct starv_log_entry *entry, *tmp;

	DlListForEachSafe(entry, tmp, &block->log_head, struct starv_log_entry, list) {
		block_entry_free(entry);
	}

	block->avg_dur = 0;
	block->max_dur = 0;
	block->min_dur = 1;
	block->count = 0;
	block->timeout_cnt = 0;
}

static struct starv_log_entry *starv_log_insert(struct starv_dbg *starv)
{
	struct starv_dbg_block *block = starv->block;
	struct starv_log *ctrl = block->ctrl;
	struct starv_log_entry *entry = NULL;

	NdisAcquireSpinLock(&ctrl->lock);
	entry = DlListFirst(&ctrl->free, struct starv_log_entry, list);
	if (!entry)
		goto end;

	DlListDel(&entry->list);
	entry->duration = starv->msec;
	entry->block = starv->block;
	DlListAddTail(&block->log_head, &entry->list);
end:
	NdisReleaseSpinLock(&ctrl->lock);
	return entry;
}

static void strav_timeout_handle(struct starv_dbg *starv)
{
	struct starv_log_entry *entry;
	struct starv_dbg_block *block = starv->block;

	entry = starv_log_insert(starv);

	if (block->timeout_fn && entry)
		block->timeout_fn(starv, entry);

	block->timeout_cnt++;
}

VOID starv_dbg_init(struct starv_dbg_block *block, struct starv_dbg *starv)
{
	/*initial value*/
	starv->msec = 0;
	starv->start_jiffies = 0;
	starv->end_jiffies = 0;
	starv->block = block;
	return;
}

VOID starv_dbg_get(struct starv_dbg *starv)
{
	struct starv_dbg_block *block = starv->block;

	starv->start_jiffies = jiffies;

	if (block->entry_fn)
		block->entry_fn(starv);
}

VOID starv_dbg_put(struct starv_dbg *starv)
{
	struct starv_dbg_block *block = starv->block;

	starv->end_jiffies = jiffies;
	starv->msec = jiffies_to_msecs(abs(starv->end_jiffies - starv->start_jiffies));
	/*update block*/
	block->max_dur = (starv->msec > block->max_dur) ? starv->msec : block->max_dur;
	block->min_dur = (starv->msec < block->min_dur) ? starv->msec : block->min_dur;
	block->avg_dur = (starv->msec + block->avg_dur * block->count) / (block->count + 1);
	block->count++;
	/*check timeout*/
	if (starv->msec > block->timeout)
		strav_timeout_handle(starv);

	if (block->leave_fn)
		block->leave_fn(starv);
}

VOID starv_log_dump(struct starv_log *ctrl)
{
	struct starv_log_entry *entry, *tmp;
	struct starv_dbg_block *block, *btmp;

	MTWF_PRINT("===========================================================================\n");

	NdisAcquireSpinLock(&ctrl->lock);
	DlListForEachSafe(block, btmp, &ctrl->block_head, struct starv_dbg_block, list) {
		MTWF_PRINT("[block %s]\t min: %d, max: %d, avg: %d, timeout cnt: %d, total cnt: %d\n",
			block->name,
			block->min_dur,
			block->max_dur,
			block->avg_dur,
			block->timeout_cnt,
			block->count);
		MTWF_PRINT("===========================================================================\n");

		DlListForEachSafe(entry, tmp, &block->log_head, struct starv_log_entry, list) {
			if (block->log_fn)
				block->log_fn(entry);
		}
		MTWF_PRINT("===========================================================================\n");
		block_init(block);
	}

	NdisReleaseSpinLock(&ctrl->lock);
}

void starv_timeout_log_basic(struct starv_log_entry *entry)
{
	struct starv_log_basic *log = entry->log;

	MTWF_PRINT("[%s] id: %d, qsize: %d, duration: %d ms\n",
				entry->block->name, log->id, log->qsize, entry->duration);
}

INT register_starv_block(struct starv_dbg_block *block)
{
	struct starv_log *ctrl = block->ctrl;

	NdisAcquireSpinLock(&ctrl->lock);
	DlListInit(&block->log_head);
	block_init(block);
	DlListAddTail(&ctrl->block_head, &block->list);
	NdisReleaseSpinLock(&ctrl->lock);
	return 0;
}

VOID unregister_starv_block(struct starv_dbg_block *block)
{
	struct starv_log *ctrl = block->ctrl;

	NdisAcquireSpinLock(&ctrl->lock);
	DlListDel(&block->list);
	block_init(block);
	NdisReleaseSpinLock(&ctrl->lock);
}

INT starv_log_init(struct starv_log *ctrl)
{
	INT i;
	struct starv_log_entry *entry = NULL;

	os_zero_mem(ctrl, sizeof(struct starv_log));
	NdisAllocateSpinLock(NULL, &ctrl->lock);
	DlListInit(&ctrl->free);
	DlListInit(&ctrl->block_head);
	/*add to free pool*/
	for (i = 0; i < STARV_ENTRY_SIZE; i++) {
		entry = &ctrl->pool[i];
		DlListAddTail(&ctrl->free, &entry->list);
	}
	return 0;
}

VOID starv_log_exit(struct starv_log *ctrl)
{
	starv_log_dump(ctrl);
	DlListInit(&ctrl->free);
	DlListInit(&ctrl->block_head);
	NdisFreeSpinLock(&ctrl->lock);
	os_zero_mem(&ctrl, sizeof(struct starv_log));
}

#endif /*DBG_STARVATION*/


#ifdef CONFIG_DBG_QDISC
/*
*
*/
void os_system_tx_queue_dump(PNET_DEV dev)
{
	struct netdev_queue *txq;
	struct Qdisc *q;
	struct sk_buff *skb;
	int i = 0, j = 0;
	struct page *p;
	size_t page_size;
	int len;

	for (i = 0; i < dev->num_tx_queues; i++) {
		txq = netdev_get_tx_queue(dev, i);
		q = rcu_dereference(txq->qdisc);
		len = qdisc_qlen(q);
		printk("%s(): devname:%s, q=%p, qlen=%d, page size:%lu\n", __func__, dev->name, q, len, PAGE_SIZE);
		skb = q->ops->peek(q);
		/* If we run with a TX queue, check if the queue is too long*/
		for (j = 0; j < len; j++) {
			printk("%s(): skb=%p,len:%d, skb->head: %p,%p , dev:%s,\n",
				__func__, skb, skb->len, skb->data, skb->data, skb->dev->name);
			if (skb->head) {
				p = virt_to_head_page(skb->head);
				page_size = PAGE_SIZE << compound_order(p);
			}
			skb = skb->next;
		}
		printk("%s(): qdisc stats!\n", __func__);
		printk("%s(): qlen: %d, backlog: %d\n", __func__, q->qstats.qlen, q->qstats.backlog);
		printk("%s(): drops: %d, requeues: %d, overlimits:% d\n", __func__, q->qstats.drops, q->qstats.requeues, q->qstats.overlimits);
	}
}
#endif /*CONFIG_DBG_QDISC*/

#ifdef CFG_SUPPORT_CSI
/*csi policy*/
static struct nla_policy csi_genl_policy[CSI_ATTR_MAX + 1] = {
	[CSI_ATTR_REPORT_MSG] = { .type = NLA_STRING },
};

/*csi ops init*/
static struct genl_ops csi_genl_ops[] = {
	{
		.cmd = CSI_OPS_REPORT,
		.flags = 0,
#if KERNEL_VERSION(5, 2, 0) > LINUX_VERSION_CODE
		.policy = csi_genl_policy,
#endif
		.doit = csi_genl_recv_doit,
		.dumpit = NULL,
	}
};

/*csi family*/
static struct genl_family csi_genl_family = {
#if KERNEL_VERSION(4, 9, 300) >= LINUX_VERSION_CODE
	.id = GENL_ID_GENERATE,
#endif
#if KERNEL_VERSION(5, 2, 0) <= LINUX_VERSION_CODE
	.policy = csi_genl_policy,
#endif
	.hdrsize = 0,
	.name = CSI_GENL_NAME,
	.version = 1,
	.maxattr = CSI_ATTR_MAX,
#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
	.ops = csi_genl_ops,
	.n_ops = ARRAY_SIZE(csi_genl_ops),
	.module = THIS_MODULE,
#endif
};

static int make_common_info(RTMP_ADAPTER *pAd, struct sk_buff *nl_skb, struct CSI_DATA_T *tmp_csi_data)
{
	struct wifi_dev *pwdev;

	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;

	/*TBD: maybe we need check pentry to drop related pkts if station is offline*/
	pwdev = prCSIInfo->csi_wdev;

	/*add common info*/
	if (nla_put_u16(nl_skb, CSI_ATTR_PKT_IDX, PARSE_CSI_SEQ_NUM(tmp_csi_data->chain_info)) ||
		nla_put_u32(nl_skb, CSI_ATTR_TS, tmp_csi_data->u4TimeStamp) ||
		nla_put_u8(nl_skb, CSI_ATTR_BAND_IDX, tmp_csi_data->ucDbdcIdx) ||
		nla_put_u16(nl_skb, CSI_ATTR_CHANNEL, pwdev->channel) ||
		nla_put_u8(nl_skb, CSI_ATTR_CBW, tmp_csi_data->ucBw) ||
		nla_put_u8(nl_skb, CSI_ATTR_DBW, tmp_csi_data->ucDataBw) ||
		nla_put_u8(nl_skb, CSI_ATTR_CHAIN_NUM, PARSE_MAX_CHAIN_NUM(tmp_csi_data->chain_info)) ||
		nla_put_u8(nl_skb, CSI_ATTR_CH_IDX, tmp_csi_data->ucPrimaryChIdx) ||
		nla_put_u16(nl_skb, CSI_ATTR_FRAME_MODE, tmp_csi_data->ucRxMode) ||
		nla_put_u8(nl_skb, CSI_ATTR_FRAME_TYPE, FC_TYPE_DATA + (SUBTYPE_QDATA << 2)) ||
		nla_put_u16(nl_skb, CSI_ATTR_CSI_LEN, tmp_csi_data->u2DataCount) ||
		nla_put_u16(nl_skb, CSI_ATTR_MCS_RATE, tmp_csi_data->rx_rate)
	) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"%s fail!!!\n", __func__);
		return -1;
	}

	if (nla_put(nl_skb, CSI_ATTR_RA, MAC_ADDR_LEN, pwdev->bssid)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "add RA MAC fail!\n");
		return -1;
	}

	if (nla_put(nl_skb, CSI_ATTR_TA, MAC_ADDR_LEN, tmp_csi_data->aucTA)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "add TA MAC fail!\n");
		return -1;
	}

	if (nla_put_u32(nl_skb, CSI_ATTR_EXTRA_INFO, tmp_csi_data->u4ExtraInfo)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "add EXTRA INFO fail!\n");
		return -1;
	}

	/*put some csi info to skb cb*/
	SET_CSI_INFO_TO_SKB(nl_skb, CB_CHAIN_NUM, PARSE_MAX_CHAIN_NUM(tmp_csi_data->chain_info));

	return 0;
}

static int append_specific_info(RTMP_ADAPTER *pAd, struct sk_buff *nl_skb, struct CSI_DATA_T *tmp_csi_data)
{
	struct nlattr *chain_attr = NULL;
	UINT16 data_count = tmp_csi_data->u2DataCount;

	chain_attr = nla_nest_start(nl_skb, CSI_ATTR_CHAIN_HEADER);
	if (!chain_attr) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "invalid chain_attr!\n");
		return -1;
	}

	if (nla_put_u8(nl_skb, CSI_ATTR_CHAIN_IDX, PARSE_CHAIN_IDX(tmp_csi_data->chain_info)) ||
		nla_put_s8(nl_skb, CSI_ATTR_RSSI, tmp_csi_data->cRssi) ||
		nla_put_u8(nl_skb, CSI_ATTR_SNR, tmp_csi_data->ucSNR) ||
		nla_put_u16(nl_skb, CSI_ATTR_TX_IDX, (UINT16)(GET_CSI_TX_IDX(tmp_csi_data->Tx_Rx_Idx))) ||
		nla_put_u16(nl_skb, CSI_ATTR_RX_IDX, (UINT16)(GET_CSI_RX_IDX(tmp_csi_data->Tx_Rx_Idx)))) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "add fragment info fail!\n");
		return -1;
	}

	if (nla_put(nl_skb, CSI_ATTR_I, data_count*sizeof(INT_16), tmp_csi_data->ac2IData)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "add I data info fail!\n");
		return -1;
	}

	if (nla_put(nl_skb, CSI_ATTR_Q, data_count*sizeof(INT_16), tmp_csi_data->ac2QData)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "add Q data info fail!\n");
		return -1;
	}

	nla_nest_end(nl_skb, chain_attr);

	/*put some csi info to skb cb*/
	SET_CSI_INFO_TO_SKB(nl_skb, CB_CHAIN_IDX, PARSE_CHAIN_IDX(tmp_csi_data->chain_info));

	return 0;
}

static struct sk_buff *make_csi_nlmsg_fragment(RTMP_ADAPTER *pAd)
{
	struct CSI_DATA_T *tmp_csi_data = NULL;
	void *msg_header = NULL;
	struct nlattr *tmp_attr = NULL;
	struct sk_buff *nl_skb = NULL;

	/* alloc mem for CSIData */
	os_alloc_mem(NULL, (UCHAR **)&tmp_csi_data, sizeof(struct CSI_DATA_T));

	if (!tmp_csi_data) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"tmp_csi_data fail to alloc mem!\n");
		goto out;
	}

	os_zero_mem(tmp_csi_data, sizeof(struct CSI_DATA_T));

	if (wlanPopCSIData(pAd, tmp_csi_data) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"pop csi_data fail!\n");
		goto out;
	}

	nl_skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);

	if (!nl_skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "csi alloc skb fail!!!\n");
		goto out;
	}

	/*netlink header*/
	msg_header = genlmsg_put(nl_skb, 0, 0, &csi_genl_family, 0, CSI_OPS_REPORT);

	if (!msg_header) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "csi nlmsg_put fail!!!\n");
		kfree_skb(nl_skb);
		nl_skb = NULL;
		goto out;
	}

	/*all attr integrate into a nested type*/
	tmp_attr = nla_nest_start(nl_skb, CSI_ATTR_DATA_HEADER);

	if (!tmp_attr) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR, "invalid tmp_attr!\n");
		kfree_skb(nl_skb);
		nl_skb = NULL;
		goto out;
	}

	if (make_common_info(pAd, nl_skb, tmp_csi_data) ||
		append_specific_info(pAd, nl_skb, tmp_csi_data)) {
		kfree_skb(nl_skb);
		nl_skb = NULL;
		goto out;
	}

	nla_nest_end(nl_skb, tmp_attr);

	genlmsg_end(nl_skb, msg_header);

out:
	if (tmp_csi_data) {
		if (tmp_csi_data->ac2IData)
			os_free_mem(tmp_csi_data->ac2IData);
		if (tmp_csi_data->ac2QData)
			os_free_mem(tmp_csi_data->ac2QData);
		os_free_mem(tmp_csi_data);
	}

	return nl_skb;
}

static int send_msg_reply(PRTMP_ADAPTER pAd, struct genl_info *info, char *cmd_msg)
{
	struct sk_buff *skb = NULL;
	void *msg_head = NULL;
	struct CSI_INFO_T *prCSIInfo;

	prCSIInfo = &pAd->rCSIInfo;

	skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);

	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"csi skb alloc fail!!!\n");
		return -1;
	}

	msg_head = genlmsg_put(skb, 0, info->snd_seq + 1, &csi_genl_family, 0, CSI_OPS_REPORT);

	if (!msg_head) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"csi genl header alloc fail!!!\n");
		kfree_skb(skb);
		return -1;
	}

	if (nla_put_string(skb, CSI_ATTR_REPORT_MSG, cmd_msg)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"nla_put_attr fail!!!\n");
		kfree_skb(skb);
		return -1;
	}

	genlmsg_end(skb, msg_head);

	if (genlmsg_reply(skb, info)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"genl reply status fail!!!\n");
		return -1;
	}

	return 0;
}

VOID csi_report_timer_handle(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	struct CSI_INFO_T *prCSIInfo;
	struct sk_buff *nl_skb = NULL;

	prCSIInfo = &pAd->rCSIInfo;

	nl_skb = make_csi_nlmsg_fragment(pAd);

	if (!nl_skb) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"make nl msg fail!!!\n");
		return;
	}

	if (genlmsg_reply(nl_skb, &prCSIInfo->peer_info)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"genl reply data fail!!!\n");
		return;
	}

}

int csi_genl_recv_doit(struct sk_buff *skb_temp, struct genl_info *info)
{
	struct nlattr *na = NULL;
	UINT32 recvd_dump_num = 0;
	PNET_DEV dev = NULL;
	PRTMP_ADAPTER pAd = NULL;
	char cmd_msg[32] = {0};
	char dev_string[16] = {0};
	char req_type[16] = {0}; /*chain or pkt*/
	struct CSI_INFO_T *prCSIInfo;
	UINT32 usr_loop_cnt = 0;
	UINT32 pkt_total_chain = 0;
	UINT32 pkt_chain_idx = 0;
	char new_pkt_flag = 0;
	char *temp_str = NULL;
	u32 report_interval = 0;
	struct sk_buff *nl_skb = NULL;
	int ret;

	if (!info || !skb_temp) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"genl_info or skb null!\n");
		return -EINVAL;
	}

	/*step1: parse the net dev and request dump number*/
	na = info->attrs[CSI_ATTR_REPORT_MSG];

	if (!na) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"get attr fail!!!\n");
		return -EINVAL;
	}

#if KERNEL_VERSION(4, 12, 0) <= LINUX_VERSION_CODE
	if (nla_validate(na, na->nla_len, CSI_ATTR_MAX, csi_genl_policy, NULL))
#else
	if (nla_validate(na, na->nla_len, CSI_ATTR_MAX, csi_genl_policy))
#endif
	{
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"nla_validate fail!!!\n");
		return -EINVAL;
	}

	os_move_mem(cmd_msg, (char *)nla_data(na), na->nla_len);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
	"csi nl get msg from usr space: %s .\n", cmd_msg);

	/*msg ex: rax0-pkt, rax0-pkt-n, rax0-chain-n, rax0-chain-0, rax0-auto-interval*/
	if (strlen(cmd_msg)) {
		temp_str = rstrtok(cmd_msg, "-");

		if (!temp_str) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"rstrtok parse fail!!!\n");
			return -EINVAL;
		}
		strlcpy(dev_string, temp_str, sizeof(dev_string));

		temp_str = rstrtok(NULL, "-");

		if (!temp_str) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"rstrtok parse fail!!!\n");
			return -EINVAL;
		}
		strlcpy(req_type, temp_str, sizeof(req_type));
	} else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"empty data from csi user!!!\n");
		return -EINVAL;
	}

	if (!strcmp(req_type, "chain") || !strcmp(req_type, "pkt")) {
		temp_str = rstrtok(NULL, "-");

		if (temp_str)
			ret = kstrtou32(temp_str, 10, &recvd_dump_num);

		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"CSI kstrtou32 fail.\n");
			return ret;
		}

		/*at least send out one pkt*/
		if (!strcmp(req_type, "pkt") && !temp_str)
			recvd_dump_num = 1;
	} else if (!strcmp(req_type, "auto")) {
		temp_str = rstrtok(NULL, "-");

		if (temp_str)
			ret = kstrtou32(temp_str, 10, &report_interval);

		if (ret) {
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"CSI kstrtou32 fail.\n");
			return ret;
		}
	}

	dev = dev_get_by_name(genl_info_net(info), dev_string);

	if (!dev) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"CSI genl_rev not find net_dev(%s).\n", dev_string);
		return -EFAULT;
	}

	if (!netif_running(dev)) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"net dev(%s) is not running.\n", dev->name);
		return -EFAULT;
	}

	/*get our pAd*/
	GET_PAD_FROM_NET_DEV(pAd, dev);

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"CSI genl_rev, get null pAd.\n");
		return -EFAULT;
	}

	prCSIInfo = &pAd->rCSIInfo;

	/*step2: send our status: used buffer*/
	/*step2:  Or start reporting csi data*/
	/*TBD: put make_csi_nlmsg_complete(pAd) as 3rd op*/
	if ((!strcmp(req_type, "chain")) && (recvd_dump_num == 0)) {
		os_zero_mem(cmd_msg, sizeof(cmd_msg));
		ret = snprintf(cmd_msg, sizeof(cmd_msg), "%s-%d", dev_string, prCSIInfo->u4CSIBufferUsed);

		if (os_snprintf_error(sizeof(cmd_msg), ret)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
			return -EFAULT;
		}

		send_msg_reply(pAd, info, cmd_msg);
	} else if ((!strcmp(req_type, "chain")) && (recvd_dump_num != 0)) {
		/*user request/ pkt total chain num/ left chains after fix*/
		/*select the less num to send pkt.*/
		/*TBD: need to check if fw will fix the total num with chain filter */
		usr_loop_cnt = recvd_dump_num;

		while (usr_loop_cnt--) {
			nl_skb = make_csi_nlmsg_fragment(pAd);

			if (!nl_skb) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"make nl msg fail!!!\n");
				break;
			}

			if (genlmsg_reply(nl_skb, info)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"genl reply data fail!!!\n");
				break;
			}
		}
	} else if (!strcmp(req_type, "pkt")) {
		usr_loop_cnt = recvd_dump_num;

		while (usr_loop_cnt--) {
			while (TRUE) {
				nl_skb = make_csi_nlmsg_fragment(pAd);

				if (!nl_skb) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"make nl msg fail!!!\n");
					break;
				}

				pkt_chain_idx = GET_CSI_INFO_FROM_SKB(nl_skb, CB_CHAIN_IDX);
				pkt_total_chain = GET_CSI_INFO_FROM_SKB(nl_skb, CB_CHAIN_NUM);

				if (!pkt_chain_idx)
					new_pkt_flag = 1;

				if (new_pkt_flag == 0) {
					MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_NOTICE,
					"drop csi fragment chain, wait for first chain of a pkt.\n");
					kfree_skb(nl_skb);
					continue;
				} else {
					if (genlmsg_reply(nl_skb, info)) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
						"genl reply data fail!!!\n");
						break;
					}

					/*the last chain of the pkt*/
					if (pkt_chain_idx == (pkt_total_chain - 1)) {
						new_pkt_flag = 0;
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
						"send total(%d) chains for a whole pkt.\n", pkt_total_chain);
						break;
					}
				}
			}
		}
	} else if (!strcmp(req_type, "auto")) {
		BOOLEAN Cancelled;

		prCSIInfo->report_interval = report_interval;
		os_move_mem(&prCSIInfo->peer_info, info, sizeof(struct genl_info));

		if (!report_interval) {
			RTMPCancelTimer(&prCSIInfo->csi_report_timer, &Cancelled);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_NOTICE,
			"csi genl timer stop!\n");
		} else if (!prCSIInfo->report_interval) {
			RTMPSetTimer(&prCSIInfo->csi_report_timer, report_interval);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_NOTICE,
			"csi genl timer start, interval(%d ms)!\n", prCSIInfo->report_interval);
		} else {
			RTMPModTimer(&prCSIInfo->csi_report_timer, report_interval);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_NOTICE,
			"csi genl timer changed, interval(%d ms)!\n", prCSIInfo->report_interval);
		}
	}

	return 0;
}

static INT csi_genl_register(void)
{
	int ret;
#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
	ret = genl_register_family(&csi_genl_family);
#else
	ret = genl_register_family_with_ops(&csi_genl_family, csi_genl_ops);
#endif
	if (ret < 0) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"failed to register CSI genl family(%d)!!!\n", ret);
		return ret;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_NOTICE,
	"register CSI genl family(%d)!!!\n", ret);

	return ret;
}

static INT csi_genl_unregister(void)
{
	int ret;

	ret = genl_unregister_family(&csi_genl_family);

	if (ret < 0) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"failed to unregister CSI genl family(%d)!!!\n", ret);
		return ret;
	}

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_NOTICE,
	"unregister CSI genl family(%d)!!!\n", ret);

	return ret;
}
#endif


/*initial global data structure for single driver*/
VOID os_module_init(VOID)
{
#ifndef CONFIG_PROFILE_OFF
#ifdef DBG
	RTMP_STRING dbg_level[32] = {0};
	RTMP_STRING dbg_option[32] = {0};
	NDIS_STATUS retval = NDIS_STATUS_SUCCESS;
#endif /* DBG */
#endif /* !CONFIG_PROFILE_OFF */

	rt_profile_init();

#ifndef CONFIG_PROFILE_OFF
#ifdef DBG
	retval = get_dbg_setting_by_profile(dbg_level, dbg_option);
	if (retval == NDIS_STATUS_SUCCESS) {
		if (strlen(dbg_level) != 0)
			Set_Debug_Proc(NULL, dbg_level);

		if (strlen(dbg_option) != 0)
			Set_DebugOption_Proc(NULL, dbg_option);
	}
#endif /* DBG */
#endif /* !CONFIG_PROFILE_OFF */

	physical_device_init();
#ifdef CONFIG_CONNINFRA_SUPPORT
	conninfra_pwr_on(CONNDRV_TYPE_WIFI);
#endif /* CONFIG_CONNINFRA_SUPPORT */
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListInital();
#endif /* MEM_ALLOC_INFO_SUPPORT */

/* Add out-of-memory notifier */
	multi_hif_init();

	/* init bss manager */
	bss_mngr_init();
	/* init ser mngr */
	ser_mngr_init();

	/* init sta mld link manager*/
#ifdef DOT11_EHT_BE
	init_sta_mld_link_mgr();
	/* mld entry init */
	mld_entry_init();
#endif

#ifdef CFG_SUPPORT_CSI
	csi_genl_register();
#endif
}

/*exit global data structrue for single driver*/
VOID os_module_exit(VOID)
{
#ifdef CFG_SUPPORT_CSI
	csi_genl_unregister();
#endif
#ifdef DOT11_EHT_BE
	/* mld entry exit */
	mld_entry_exit();
#endif
	ser_mngr_exit();
	/* de-init bss manager */
	bss_mngr_deinit();

	multi_hif_exit();
/* Del out-of-memory notifier */
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListExit();
#endif /*MEM_ALLOC_INFO_SUPPORT*/
#ifdef CONFIG_CONNINFRA_SUPPORT
	conninfra_pwr_off(CONNDRV_TYPE_WIFI);
#endif /* CONFIG_CONNINFRA_SUPPORT */
	physical_device_exit();

	rt_profile_deinit();
}
