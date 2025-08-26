/******************************************************************************
*
* Copyright (c) 2011 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* FILE NAME  :   wlan_event.c
* VERSION    :   1.0
* DESCRIPTION:   Event mechanism.
*
* AUTHOR     :   zhengxinggu <zhengxinggu@tp-link.net>
* CREATE DATE:   01/14/2011
*
* HISTORY    :
* 01   01/14/2011  zhengxinggu     Create.
* 02   07/25/2014  yangyang        Copy from TPWD, and adjust for MTK driver
*
******************************************************************************/
#include "wlanif/wlan_headers.h"
#include "rt_config.h"

static UINT16 _checksum(UINT16 *buf, UINT32 len) 
{ 
    UINT32 cksum = 0; 
 
    while (len > 1) 
    { 
        cksum += *buf++; 
        len -= sizeof(UINT16); 
    }

    if (len) 
    { 
        cksum += LE16_TO_CPU(*(UINT8*)buf); 
    }

    cksum = (cksum >> 16) + (cksum & 0xffff); 
    cksum += (cksum >> 16);

    return (UINT16)(~cksum); 
}

/* TODO: need to add dual band support  */
STATUS wlan_eventIndicate(void *pAdSrc, void *wdevSrc, UINT8 *macaddr, UINT32 msg, UINT8 *data, UINT32 len)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct wifi_dev *wdev = (struct wifi_dev *)wdevSrc;
	UINT8 *tmpBuf = NULL;
	PNDIS_PACKET pPacket;
	struct ethernet_header	*eth;
	struct ip_header		*ip;
	struct udp_header		*udp;
	struct wlan_event_msg	*event;
	UINT16 ulen = len + sizeof(struct udp_header) + sizeof(struct wlan_event_msg)
				  + sizeof(struct ethernet_header) + sizeof(struct ip_header);
	static UINT8 lanMac[MAC_ADDR_SIZE] = {0};
	static BOOL lanMacValid = FALSE;
	UINT32 devno;
	UINT32 unit;
	UINT32 vapUnit;

	if (WMODE_CAP_2G(wdev->PhyMode))
	{
		devno = WLAN_2G_PHY_DEV;
		for (unit = UNIT_2G_MAIN; unit <= UNIT_2G_MAX; unit++)
		{
			vapUnit = GET_VAP_UNIT(devno, unit);
			if (((IS_APVAP_UNIT(vapUnit) && WDEV_WITH_BCN_ABILITY(wdev)) 
				|| (IS_STAVAP_UNIT(vapUnit) && !WDEV_WITH_BCN_ABILITY(wdev))) 
					&& GET_VAP_IDX(vapUnit) ==  wdev->func_idx)
				break;
		}
		ASSERT(unit <= UNIT_2G_MAX);
	}
	else
	{
		devno = WLAN_5G_PHY_DEV;
		for (unit = UNIT_5G_MAIN; unit <= UNIT_5G_MAX; unit++)
		{
			vapUnit = GET_VAP_UNIT(devno, unit);
			if (((IS_APVAP_UNIT(vapUnit) && WDEV_WITH_BCN_ABILITY(wdev)) 
				|| (IS_STAVAP_UNIT(vapUnit) && !WDEV_WITH_BCN_ABILITY(wdev))) 
					&& GET_VAP_IDX(vapUnit) ==  wdev->func_idx)
				break;
		}
		ASSERT(unit <= UNIT_5G_MAX);
	}

	if (!lanMacValid)
	{
		if (OK != macGet(lanMac, MAC_ADDR_SIZE, OS_LAN_UNIT))
		{
			DBG_ERR("lan mac get failure\n");
			return ERROR;
		}
		lanMacValid = TRUE;
	}

	tmpBuf = OS_MALLOC(ulen);

	if (NULL == tmpBuf)
	{
		DBG_ERR("alloc buffer failed!\n");
		return ERROR;
	}
	
	eth = (struct ethernet_header *)tmpBuf;
	ip  = (struct ip_header *)(eth + 1);	
	udp = (struct udp_header *)(ip + 1);
	event = (struct wlan_event_msg *)(udp + 1);

	MAC_ADDR_COPY(eth->daddr, lanMac);
	MAC_ADDR_COPY(eth->saddr, macaddr);
	eth->type = CPU_TO_BE16(ETHERTYPE_IP);

	OS_MEMZERO(ip, sizeof(struct ip_header));
	ip->ver = 0x45;
	ip->len = CPU_TO_BE16(ulen - sizeof(struct ethernet_header));
	ip->ttl = 64;
	ip->proto = IP_PROTO_UDP;
	ip->daddr = 0x00000000;
	ip->check = _checksum((UINT16 *)ip, sizeof(struct ip_header));
	
	udp->sport = CPU_TO_BE16(OS_EVENT_PORT);
	udp->dport = CPU_TO_BE16(OS_EVENT_PORT);
	udp->ulen = CPU_TO_BE16(ulen - sizeof(struct ethernet_header) - sizeof(struct ip_header));
	udp->sum = 0;

	event->magicHi = CPU_TO_BE32(MAGIC_HI);
	event->magicLo = CPU_TO_BE32(MAGIC_LO);
	/*
	 * Note: This devno should be known in wlanApp, which means 5g.
	 * Add if we also support guestNet/Apcli's WPS, the unit should also change.
	 */
#define WLAN_DEV_NO_5G 1
	event->devno = devno;
	event->unit = unit;	/* always ssid0 */
	event->type = msg;
	event->val8 = 0;
	event->val32 = 0;
	MAC_ADDR_COPY(event->macaddr, macaddr);

	if (NULL != data)
	{
		OS_MEMCPY(event + 1, data, len);
	}

	pPacket = (PNDIS_PACKET)RtmpVxNetTupleGet(pAd->net_dev->pNetPool, 1024, M_DONTWAIT, MT_DATA, 1);
	if (NULL == pPacket)
	{
		DBG_ERR("can't get a mblk from net tuple!\n");
		OS_FREE(tmpBuf);
		return ERROR;
	}
	
	NdisZeroMemory(GET_OS_PKT_DATAPTR(pPacket), 1024);
	GET_OS_PKT_DATAPTR(pPacket) += 90;
	
	NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket), tmpBuf, ulen);
	GET_OS_PKT_LEN(pPacket) = ulen;

	((M_BLK_ID)pPacket)->mBlkPktHdr.len = ulen;
	if (devno == WLAN_2G_PHY_DEV)
	{
		((M_BLK_ID)pPacket)->mBlkHdr.mFlags |=  M_WIRELESS | M_PKTHDR;
	}
	else
	{
		((M_BLK_ID)pPacket)->mBlkHdr.mFlags |=  M_WIRELESS_5G | M_PKTHDR;
	}

	if ((wdev->func_idx >= 0) && (wdev->func_idx < MAX_BEACON_NUM))
		END_RCV_RTN_CALL(pAd->ApCfg.MBSSID[wdev->func_idx].wdev.if_dev, pPacket);

	OS_FREE(tmpBuf);
	
	return OK;
}
