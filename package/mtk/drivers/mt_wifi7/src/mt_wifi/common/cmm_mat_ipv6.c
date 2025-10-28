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
	cmm_mat_ipv6.c

	Abstract:
		MAT convert engine subroutine for ipv6 base protocols, currently now we
	just handle IPv6/ICMPv6 packets without Authentication/Encryption headers.

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Shiang      06/03/07      Init version
*/
#ifdef MAT_SUPPORT

#include "rt_config.h"
#include "ipv6.h"

/*#include <asm/checksum.h> */
/*#include <net/ip6_checksum.h> */

const UCHAR IPV6_LOOPBACKADDR[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

static NDIS_STATUS MATProto_IPv6_Init(MAT_STRUCT *pMatCfg);
static NDIS_STATUS MATProto_IPv6_Exit(MAT_STRUCT *pMatCfg);
static PUCHAR MATProto_IPv6_Rx(MAT_STRUCT *pMatCfg, PNDIS_PACKET pSkb, PUCHAR pLayerHdr, PUCHAR pDevMacAdr);
static PUCHAR MATProto_IPv6_Tx(MAT_STRUCT *pMatCfg, PNDIS_PACKET pSkb, PUCHAR pLayerHdr, PUCHAR pDevMacAdr);

#define RT_UDP_HDR_LEN	8

typedef struct _IPv6MacMappingEntry {
	UCHAR ipv6Addr[16];	/* In network order */
	UCHAR macAddr[MAC_ADDR_LEN];
	ULONG lastTime;
	RTMP_HLIST_NODE list;
} IPv6MacMappingEntry, *PIPv6MacMappingEntry;


typedef struct _IPv6MacMappingTable {
	BOOLEAN			valid;
	RTMP_HLIST_HEAD		hash[MAT_MAX_HASH_ENTRY_SUPPORT]; /*0~63 for specific station*/
	UCHAR			curMcastAddr[MAC_ADDR_LEN];	/* The multicast mac addr for currecnt received packet destined to ipv6 multicast addr */
} IPv6MacMappingTable;


struct _MATProtoOps MATProtoIPv6Handle = {
	.init = MATProto_IPv6_Init,
	.tx = MATProto_IPv6_Tx,
	.rx = MATProto_IPv6_Rx,
	.exit = MATProto_IPv6_Exit,
};

static inline BOOLEAN needUpdateIPv6MacTB(
	UCHAR			*pMac,
	RT_IPV6_ADDR	*pIPv6Addr)
{
	ASSERT(pIPv6Addr);
	if (pIPv6Addr == NULL)
		return FALSE;

	if (isMcastEtherAddr(pMac) || isZeroEtherAddr(pMac))
		return FALSE;

	/* IPv6 multicast address */
	if (IS_MULTICAST_IPV6_ADDR(*pIPv6Addr))
		return FALSE;

	/* unspecified address */
	if (IS_UNSPECIFIED_IPV6_ADDR(*pIPv6Addr))
		return FALSE;

	/* loopback address */
	if (IS_LOOPBACK_IPV6_ADDR(*pIPv6Addr))
		return FALSE;

	return TRUE;
}


/*
	IPv6 Header Format

     0               1               2               3
     0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |Version| Traffic Class |           Flow Label                  |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |      Payload Length           |  Next Header  |   Hop Limit   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    +                                                               +
    |                      Source Address                           |
    +                                                               +
    |                                                               |
    +                                                               +
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    +                                                               +
    |                   Destination Address                         |
    +                                                               +
    |                                                               |
    +                                                               +
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


ICMPv6 Format:
	|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Type      |     Code      |           Checksum            |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                       Message Body                            |
	+                                                               +
	|                                                               |
				    ......
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

NDIS_STATUS  dumpIPv6MacTb(
	IN MAT_STRUCT *pMatCfg,
	IN int index)
{
	IPv6MacMappingTable *pIPv6MacTable;
	int startIdx, endIdx;

	pIPv6MacTable = (IPv6MacMappingTable *)pMatCfg->MatTableSet.IPv6MacTable;

	if ((!pIPv6MacTable) || (!pIPv6MacTable->valid)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_ERROR, "IPv6MacTable not init yet, so cannot do dump!\n");
		return FALSE;
	}

	if (index < 0) {
		/* dump all. */
		startIdx = 0;
		endIdx = MAT_MAX_HASH_ENTRY_SUPPORT;
	} else {
		/* dump specific hash index. */
		startIdx = endIdx = index;
	}

	MTWF_PRINT("%s():\n", __func__);

	for (; startIdx < endIdx; startIdx++) {
		IPv6MacMappingEntry *pEntry;
		RTMP_HLIST_NODE *pTmp;

		RtmpHashListForEachSafe(pEntry, pTmp, &pIPv6MacTable->hash[startIdx], list) {
			MTWF_PRINT("IPv6Mac[%d]:\n", startIdx);
			MTWF_PRINT("\t:IPv6=%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x,Mac="MACSTR", "
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[0])
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[1])
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[2])
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[3])
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[4])
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[5])
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[6])
				, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[7])
				, MAC2STR(pEntry->macAddr));
			MTWF_PRINT("lastTime=0x%lx\n", pEntry->lastTime);
		}
	}

	MTWF_PRINT("\t----EndOfDump!\n");
	return TRUE;
}



static NDIS_STATUS IPv6MacTableUpdate(
	IN MAT_STRUCT		*pMatCfg,
	IN PUCHAR			pMacAddr,
	IN PCHAR			pIPv6Addr)
{
	UINT				hashIdx;
	IPv6MacMappingTable	*pIPv6MacTable;
	IPv6MacMappingEntry	*pEntry, *pNewEntry;
	RTMP_HLIST_NODE *pTmp;
	ULONG				now;

	pIPv6MacTable = (IPv6MacMappingTable *)pMatCfg->MatTableSet.IPv6MacTable;

	if ((!pIPv6MacTable) || (!pIPv6MacTable->valid))
		return FALSE;

	hashIdx = MAT_IPV6_ADDR_HASH_INDEX(pIPv6Addr);

	RtmpHashListForEachSafe(pEntry, pTmp, &pIPv6MacTable->hash[hashIdx], list) {
		NdisGetSystemUpTime(&now);

		/* Find a existed IP-MAC Mapping entry */
		if (NdisEqualMemory(pIPv6Addr, pEntry->ipv6Addr, IPV6_ADDR_LEN)) {
			/* comparison is useless. So we directly copy it into the entry. */
			NdisMoveMemory(pEntry->macAddr, pMacAddr, 6);
			NdisGetSystemUpTime(&pEntry->lastTime);
			return TRUE;
		}
		/* handle the aging-out situation */
		if (RTMP_TIME_AFTER(now, (pEntry->lastTime + MAT_TB_ENTRY_AGEOUT_TIME))) {
			/* Remove the aged entry */
			RtmpHashListDelInit(&(pEntry->list));
			MATDBEntryFree(pMatCfg, (PUCHAR)pEntry);
			if (pMatCfg->nodeCount > 0)
				pMatCfg->nodeCount--;
		}
	}

#ifdef ETH_CONVERT_SUPPORT

	if (pMatCfg->nodeCount >= ETH_CONVERT_NODE_MAX)
		return FALSE;

#endif /* ETH_CONVERT_SUPPORT */
	/* Allocate a new IPv6MacMapping entry and insert into the hash */
	pNewEntry = (IPv6MacMappingEntry *)MATDBEntryAlloc(pMatCfg, sizeof(IPv6MacMappingEntry));

	if (pNewEntry != NULL) {
		NdisMoveMemory(pNewEntry->ipv6Addr, pIPv6Addr, IPV6_ADDR_LEN);
		NdisMoveMemory(pNewEntry->macAddr, pMacAddr, 6);
		NdisGetSystemUpTime(&pNewEntry->lastTime);
		RtmpHashNodeInit(&(pNewEntry->list));
		RtmpHashListAddHead(&(pNewEntry->list), &pIPv6MacTable->hash[hashIdx]);

		/*dumpIPv6MacTb(pMatCfg, hashIdx); //for debug */
		pMatCfg->nodeCount++;
		return TRUE;
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_ERROR, "Insertion failed!\n");
	return FALSE;
}


static PUCHAR IPv6MacTableLookUp(
	IN	MAT_STRUCT		*pMatCfg,
	IN	PUCHAR			pIPv6Addr)
{
	UINT				hashIdx;
	IPv6MacMappingTable	*pIPv6MacTable;
	IPv6MacMappingEntry	*pEntry;
	RTMP_HLIST_NODE *pTmp;
	PUCHAR				pGroupMacAddr;

	pIPv6MacTable = (IPv6MacMappingTable *)pMatCfg->MatTableSet.IPv6MacTable;

	if ((!pIPv6MacTable) || (!pIPv6MacTable->valid))
		return NULL;

	/*if IPV6 multicast address, need converting multicast group address to ethernet address. */
	if (IS_MULTICAST_IPV6_ADDR(*(RT_IPV6_ADDR *)pIPv6Addr)) {
		pGroupMacAddr = (PUCHAR)&pIPv6MacTable->curMcastAddr;
		ConvertMulticastIP2MAC(pIPv6Addr, (UCHAR **)(&pGroupMacAddr), ETH_P_IPV6);
		return pIPv6MacTable->curMcastAddr;
	}

	/* Use hash to find out the location of that entry and get the Mac address. */
	hashIdx = MAT_IPV6_ADDR_HASH_INDEX(pIPv6Addr);
	/*	spin_lock_irqsave(&IPMacTabLock, irqFlag); */
	RtmpHashListForEachSafe(pEntry, pTmp, &pIPv6MacTable->hash[hashIdx], list) {
		if (NdisEqualMemory(pEntry->ipv6Addr, pIPv6Addr, IPV6_ADDR_LEN)) {
			/*Update the lastTime to prevent the aging before pDA processed! */
			NdisGetSystemUpTime(&pEntry->lastTime);
			return pEntry->macAddr;
		}
	}

	/*
		We didn't find any matched Mac address, our policy is treat it as
		broadcast packet and send to all.
	*/
	return &BROADCAST_ADDR[0];
}

INLINE unsigned short int icmpv6_csum(
	RT_IPV6_ADDR *saddr,
	RT_IPV6_ADDR *daddr,
	USHORT		  len,
	UCHAR		  proto,
	UCHAR		 *pICMPMsg)
{
	int	carry;
	UINT32	ulen;
	UINT32	uproto;
	int i;
	unsigned int csum = 0;
	unsigned short int chksum;

	if (len % 4)
		return 0;

	for (i = 0; i < 4; i++) {
		csum += saddr->ipv6_addr32[i];
		carry = (csum < saddr->ipv6_addr32[i]);
		csum += carry;
	}

	for (i = 0; i < 4; i++) {
		csum += daddr->ipv6_addr32[i];
		carry = (csum < daddr->ipv6_addr32[i]);
		csum += carry;
	}

	ulen = OS_HTONL((UINT32)len);
	csum += ulen;
	carry = (csum < ulen);
	csum += carry;
	uproto = OS_HTONL((UINT32)proto);
	csum += uproto;
	carry = (csum < uproto);
	csum += carry;

	for (i = 0; i < len; i += 4) {
		csum += get_unaligned32(((UINT32 *)&pICMPMsg[i]));
		carry = (csum < get_unaligned32(((UINT32 *)&pICMPMsg[i])));
		csum += carry;
	}

	while (csum >> 16)
		csum = (csum & 0xffff) + (csum >> 16);

	chksum = ~csum;
	return chksum;
}



static PUCHAR MATProto_IPv6_Rx(
	IN MAT_STRUCT		*pMatCfg,
	IN PNDIS_PACKET		pSkb,
	IN PUCHAR			pLayerHdr,
	IN PUCHAR			pDevMacAdr)
{
	PUCHAR pMacAddr;
	PUCHAR pDstIPv6Addr;
	/* Fetch the IPv6 addres from the packet header. */
	pDstIPv6Addr = (UCHAR *)(&((RT_IPV6_HDR *)pLayerHdr)->dstAddr);
	pMacAddr = IPv6MacTableLookUp(pMatCfg, pDstIPv6Addr);
	return pMacAddr;
}

PNDIS_PACKET ICMPv6_Handle_Tx(
	IN MAT_STRUCT		*pMatSrtuct,
	IN PNDIS_PACKET		pSkb,
	IN PUCHAR			pLayerHdr,
	IN PUCHAR			pDevMacAdr,
	IN UINT32			offset)
{
	RT_IPV6_HDR			*pIPv6Hdr;
	RT_ICMPV6_HDR			*pICMPv6Hdr;
	RT_ICMPV6_OPTION_HDR	*pOptHdr;
	USHORT payloadLen;
	UINT32 ICMPOffset = 0, ICMPMsgLen = 0;
	INT32 leftLen;
	PNDIS_PACKET newSkb = NULL;
	BOOLEAN needModify = FALSE;
	PUCHAR pSrcMac;
	INT32 pkt_len = (INT32)GET_OS_PKT_LEN(pSkb);
	INT32 addr_offset;
	UCHAR *pkt_addr = GET_OS_PKT_DATAPTR(pSkb);

	pIPv6Hdr = (RT_IPV6_HDR *)pLayerHdr;
	payloadLen = OS_NTOHS(pIPv6Hdr->payload_len);
	addr_offset = (pLayerHdr+offset+payloadLen) - pkt_addr;

	if ((addr_offset > pkt_len)
		|| (addr_offset <= 0)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_DEBUG,
			"Invalid packet1, drop\n");
		return newSkb;
	}

	pICMPv6Hdr = (RT_ICMPV6_HDR *)(pLayerHdr + offset);
	ICMPOffset = offset;
	if (ICMPOffset > IPV6_HDR_LEN)
		ICMPOffset = IPV6_HDR_LEN;
	ICMPMsgLen = payloadLen + IPV6_HDR_LEN - ICMPOffset;
	leftLen = ICMPMsgLen;
	if (leftLen < 0 || leftLen >= U16_MAX) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_ERROR,
			"Invalid left len:%d\n", leftLen);
		return newSkb;
	}

	switch (pICMPv6Hdr->type) {
	case ICMPV6_MSG_TYPE_ROUTER_SOLICITATION:
		offset += ROUTER_SOLICITATION_FIXED_LEN;
		leftLen -= ROUTER_SOLICITATION_FIXED_LEN;
		/* for unspecified source address, it should not include the option about link-layer address. */
		if (!(IS_UNSPECIFIED_IPV6_ADDR(pIPv6Hdr->srcAddr)) &&
			(ICMPMsgLen > ROUTER_SOLICITATION_FIXED_LEN)) {
			while (leftLen > (INT32)sizeof(RT_ICMPV6_OPTION_HDR)) {
				pOptHdr = (RT_ICMPV6_OPTION_HDR *)(pLayerHdr + offset);

				if (pOptHdr->len == 0)
					break;  /* discard it, because it's invalid. */

				if (pOptHdr->type == TYPE_SRC_LL_ADDR) {
					/*replace the src link-layer address as ours. */
					needModify = TRUE;
					offset += 2;	/* 2 = "type, len" fields. Here indicate to the place of src mac. */
					break;
				} else {
					offset += (pOptHdr->len * 8);  /* in unit of 8 octets. */
					leftLen -= (pOptHdr->len * 8);
				}
			}
		}

		break;

	case ICMPV6_MSG_TYPE_ROUTER_ADVERTISEMENT:
		offset += ROUTER_ADVERTISEMENT_FIXED_LEN;
		leftLen -= ROUTER_ADVERTISEMENT_FIXED_LEN;
		/* for unspecified source address, it should not include the option about link-layer address. */
		if (!(IS_UNSPECIFIED_IPV6_ADDR(pIPv6Hdr->srcAddr)) &&
			(ICMPMsgLen > ROUTER_ADVERTISEMENT_FIXED_LEN)) {
			while (leftLen > (INT32)sizeof(RT_ICMPV6_OPTION_HDR)) {
				pOptHdr = (RT_ICMPV6_OPTION_HDR *)(pLayerHdr + offset);

				if (pOptHdr->len == 0)
					break;  /* discard it, because it's invalid. */

				if (pOptHdr->type == TYPE_SRC_LL_ADDR) {
					/*replace the src link-layer address as ours. */
					needModify = TRUE;
					offset += 2;	/* 2 = "type, len" fields. Here indicate to the place of src mac. */
					break;
				} else {
					offset += (pOptHdr->len * 8);  /* in unit of 8 octets. */
					leftLen -= (pOptHdr->len * 8);
				}
			}
		}

		break;

	case ICMPV6_MSG_TYPE_NEIGHBOR_SOLICITATION:
		offset += NEIGHBOR_SOLICITATION_FIXED_LEN;
		leftLen -= NEIGHBOR_SOLICITATION_FIXED_LEN;
		/* for unspecified source address, it should not include the option about link-layer address. */
		if (!(IS_UNSPECIFIED_IPV6_ADDR(pIPv6Hdr->srcAddr)) &&
			(ICMPMsgLen > NEIGHBOR_SOLICITATION_FIXED_LEN)) {
			while (leftLen > (INT32)sizeof(RT_ICMPV6_OPTION_HDR)) {
				pOptHdr = (RT_ICMPV6_OPTION_HDR *)(pLayerHdr + offset);

				if (pOptHdr->len == 0)
					break;  /* discard it, because it's invalid. */

				if (pOptHdr->type == TYPE_SRC_LL_ADDR) {
					/*replace the src link-layer address as ours. */
					needModify = TRUE;
					offset += 2;	/* 2 = "type, len" fields. Here indicate to the place of src mac. */
					break;
				} else {
					offset += (pOptHdr->len * 8);  /* in unit of 8 octets. */
					leftLen -= (pOptHdr->len * 8);
				}
			}
		}

		break;

	case ICMPV6_MSG_TYPE_NEIGHBOR_ADVERTISEMENT:
		offset += NEIGHBOR_ADVERTISEMENT_FIXED_LEN;
		leftLen -= NEIGHBOR_ADVERTISEMENT_FIXED_LEN;
		/* for unspecified source address, it should not include the option about link-layer address. */
		if (!(IS_UNSPECIFIED_IPV6_ADDR(pIPv6Hdr->srcAddr)) &&
			(ICMPMsgLen > NEIGHBOR_ADVERTISEMENT_FIXED_LEN)) {
			while (leftLen > (INT32)sizeof(RT_ICMPV6_OPTION_HDR)) {
				pOptHdr = (RT_ICMPV6_OPTION_HDR *)(pLayerHdr + offset);

				if (pOptHdr->len == 0)
					break;  /* discard it, because it's invalid. */

				if (pOptHdr->type == TYPE_TGT_LL_ADDR) {
					/*replace the src link-layer address as ours. */
					needModify = TRUE;
					offset += 2;	/* 2 = "type, len" fields. */
					break;
				} else {
					offset += (pOptHdr->len * 8);  /* in unit of 8 octets. */
					leftLen -= (pOptHdr->len * 8);
				}
			}
		}

		break;

	case ICMPV6_MSG_TYPE_REDIRECT:
		offset += REDIRECT_FIXED_LEN;
		leftLen -= REDIRECT_FIXED_LEN;
		/* for unspecified source address, it should not include the options about link-layer address. */
		if (!(IS_UNSPECIFIED_IPV6_ADDR(pIPv6Hdr->srcAddr))  &&
			(ICMPMsgLen > REDIRECT_FIXED_LEN)) {
			while (leftLen > (INT32)sizeof(RT_ICMPV6_OPTION_HDR)) {
				pOptHdr = (RT_ICMPV6_OPTION_HDR *)(pLayerHdr + offset);

				if (pOptHdr->len == 0)
					break;  /* discard it, because it's invalid. */

				if (pOptHdr->type == TYPE_TGT_LL_ADDR) {
					/* TODO: Need to check if the TGT_LL_ADDR is the inner MAC. */
					/*replace the src link-layer address as ours. */
					needModify = TRUE;
					offset += 2;	/* 2 = "type, len" fields. */
					break;
				} else {
					offset += (pOptHdr->len * 8);  /* in unit of 8 octets. */
					leftLen -= (pOptHdr->len * 8);
				}
			}
		}

		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO, "Un-supported ICMPv6 msg type(0x%x)! Ignore it\n", pICMPv6Hdr->type);
		break;
	}

	/* We need to handle about the solicitation/Advertisement packets. */
	if (needModify) {
		if (OS_PKT_CLONED(pSkb)) {
			OS_PKT_COPY(RTPKT_TO_OSPKT(pSkb), newSkb);

			if (newSkb) {
				if (IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(newSkb)))
					pIPv6Hdr = (RT_IPV6_HDR *)(GET_OS_PKT_DATAPTR(newSkb) + MAT_VLAN_ETH_HDR_LEN);
				else
					pIPv6Hdr = (RT_IPV6_HDR *)(GET_OS_PKT_DATAPTR(newSkb) + MAT_ETHER_HDR_LEN);
			}
		}

		pICMPv6Hdr = (RT_ICMPV6_HDR *)((PUCHAR)pIPv6Hdr + ICMPOffset);
		pSrcMac = (PUCHAR)((PUCHAR)pIPv6Hdr + offset);
		NdisMoveMemory(pSrcMac, pDevMacAdr, MAC_ADDR_LEN);
		/* Now re-calculate the Checksum. */
		pICMPv6Hdr->chksum = 0;
		pICMPv6Hdr->chksum = icmpv6_csum(&pIPv6Hdr->srcAddr, &pIPv6Hdr->dstAddr, ICMPMsgLen,
										 IPV6_NEXT_HEADER_ICMPV6, (PUCHAR)pICMPv6Hdr);
	}

	return newSkb;
}


static PUCHAR MATProto_IPv6_Tx(
	IN MAT_STRUCT		*pMatCfg,
	IN PNDIS_PACKET		pSkb,
	IN PUCHAR			pLayerHdr,
	IN PUCHAR			pDevMacAdr)
{
	PUCHAR pSrcMac, pSrcIP;
	BOOLEAN needUpdate;
	UCHAR nextProtocol;
	UINT32 offset;
	HEADER_802_3 *pEthHdr;
	RT_IPV6_HDR *pIPv6Hdr;
	PNDIS_PACKET newSkb = NULL;
	UCHAR *pkt_addr = GET_OS_PKT_DATAPTR(pSkb);
	UINT32 pkt_len = GET_OS_PKT_LEN(pSkb);
	INT32 addr_offset;

	pIPv6Hdr = (RT_IPV6_HDR *)pLayerHdr;
	pEthHdr = (HEADER_802_3 *)(GET_OS_PKT_DATAPTR(pSkb));
	pSrcMac = (UCHAR *)&pEthHdr->SAAddr2;
	pSrcIP = (UCHAR *)&pIPv6Hdr->srcAddr;
	needUpdate = needUpdateIPv6MacTB(pSrcMac, (RT_IPV6_ADDR *)(&pIPv6Hdr->srcAddr));

	if (needUpdate)
		IPv6MacTableUpdate(pMatCfg, pSrcMac, (CHAR *)(&pIPv6Hdr->srcAddr));

	/* We need to traverse the whole IPv6 Header and extend headers to check about the ICMPv6 pacekt. */
	nextProtocol = pIPv6Hdr->nextHdr;
	offset = IPV6_HDR_LEN;
	addr_offset = (pLayerHdr+offset) - pkt_addr;
	if ((addr_offset >= pkt_len)
		|| (addr_offset <= 0))
		return NULL;
	/*MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_DEBUG, "NextProtocol=0x%x! payloadLen=%d! offset=%d!\n", nextProtocol, payloadLen, offset); */
	while (nextProtocol != IPV6_NEXT_HEADER_ICMPV6 &&
		   nextProtocol != IPV6_NEXT_HEADER_UDP &&
		   nextProtocol != IPV6_NEXT_HEADER_TCP &&
		   nextProtocol != IPV6_NEXT_HEADER_NONE) {
		if (IPv6ExtHdrHandle((RT_IPV6_EXT_HDR *)(pLayerHdr + offset), &nextProtocol, &offset) == FALSE) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO, "IPv6ExtHdrHandle failed!\n");
			break;
		}
		addr_offset = (pLayerHdr+offset) - pkt_addr;
		if ((addr_offset >= pkt_len)
			|| (addr_offset <= 0))
			return NULL;
	}

	switch (nextProtocol) {
	case IPV6_NEXT_HEADER_ICMPV6:
		newSkb = ICMPv6_Handle_Tx(pMatCfg, pSkb, pLayerHdr, pDevMacAdr, offset);
		break;

	case IPV6_NEXT_HEADER_UDP:
		/*newSkb = DHCPv6_Handle_Tx(pMatStrcut, pSkb, pLayerHdr, pMacAddr, offset); */
		break;

	case IPV6_NEXT_HEADER_TCP:
	case IPV6_NEXT_HEADER_NONE:
	default:
		break;
	}

	return (PUCHAR)newSkb;
}



static NDIS_STATUS IPv6MacTable_RemoveAll(
	IN MAT_STRUCT *pMatCfg)
{
	IPv6MacMappingTable	*pIPv6MacTable;

	pIPv6MacTable = (IPv6MacMappingTable *)pMatCfg->MatTableSet.IPv6MacTable;

	if (!pIPv6MacTable)
		return TRUE;

	if (pIPv6MacTable->valid) {
		int i;

		pIPv6MacTable->valid = FALSE;

		for (i = 0; i < MAT_MAX_HASH_ENTRY_SUPPORT; i++) {
			IPv6MacMappingEntry *pEntry;
			RTMP_HLIST_NODE *pTmp;

			RtmpHashListForEachSafe(pEntry, pTmp, &pIPv6MacTable->hash[i], list) {
				RtmpHashListDelInit(&(pEntry->list));
				MATDBEntryFree(pMatCfg, (PUCHAR)pEntry);
			}
		}
	}

	os_free_mem(pIPv6MacTable);
	pMatCfg->MatTableSet.IPv6MacTable = NULL;
	return TRUE;
}


static NDIS_STATUS IPv6MacTable_init(
	IN MAT_STRUCT *pMatCfg)
{
	IPv6MacMappingTable	*pIPv6MacTable;

	if (pMatCfg->MatTableSet.IPv6MacTable != NULL)
		pIPv6MacTable = (IPv6MacMappingTable *)pMatCfg->MatTableSet.IPv6MacTable;
	else {
		os_alloc_mem_suspend(NULL, (UCHAR **)&(pMatCfg->MatTableSet.IPv6MacTable), sizeof(IPv6MacMappingTable));

		if (pMatCfg->MatTableSet.IPv6MacTable) {
			int i;

			pIPv6MacTable = (IPv6MacMappingTable *)pMatCfg->MatTableSet.IPv6MacTable;
			NdisZeroMemory(pIPv6MacTable, sizeof(IPv6MacMappingTable));
			for (i = 0; i < MAT_MAX_HASH_ENTRY_SUPPORT; i++)
				RtmpHashHeadInit(&pIPv6MacTable->hash[i]);
			pIPv6MacTable->valid = TRUE;
		} else {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_ERROR,
				"IPMacTable_init(): Allocate memory for IPv6MacTable failed!\n");
			return FALSE;
		}
	}

	return TRUE;
}


static NDIS_STATUS MATProto_IPv6_Exit(
	IN MAT_STRUCT	*pMatCfg)
{
	INT status;

	status = IPv6MacTable_RemoveAll(pMatCfg);
	return status;
}


static NDIS_STATUS MATProto_IPv6_Init(
	IN MAT_STRUCT	*pMatCfg)
{
	BOOLEAN status = FALSE;

	status = IPv6MacTable_init(pMatCfg);
	return status;
}



VOID getIPv6MacTbInfo(
	IN MAT_STRUCT *pMatCfg,
	IN char *pOutBuf,
	IN ULONG BufLen)
{
	IPv6MacMappingTable *pIPv6MacTable;
	int startIdx, endIdx;
	char Ipv6str[40] = {0};
	int ret;
	ULONG str_tmp_len;

	pIPv6MacTable = (IPv6MacMappingTable *)pMatCfg->MatTableSet.IPv6MacTable;

	if ((!pIPv6MacTable) || (!pIPv6MacTable->valid)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_MAT, DBG_LVL_INFO,
			"IPv6MacTable not init yet!\n");
		return;
	}

	/* dump all. */
	startIdx = 0;
	endIdx = MAT_MAX_HASH_ENTRY_SUPPORT;
	ret = snprintf(pOutBuf, BufLen, "\n");
	if (os_snprintf_error(BufLen, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}
	str_tmp_len = BufLen - strlen(pOutBuf);
	ret = snprintf(pOutBuf + strlen(pOutBuf), str_tmp_len, "%-40s%-20s\n", "IP", "MAC");
	if (os_snprintf_error(str_tmp_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}

	for (; startIdx < endIdx; startIdx++) {
		IPv6MacMappingEntry *pEntry;
		RTMP_HLIST_NODE *pTmp;

		RtmpHashListForEachSafe(pEntry, pTmp, &pIPv6MacTable->hash[startIdx], list) {
			/*	    if (strlen(pOutBuf) > (IW_PRIV_SIZE_MASK - 30)) */
			if (RtmpOsCmdDisplayLenCheck(strlen(pOutBuf), 30) == FALSE)
				break;

			NdisZeroMemory(Ipv6str, 40);
			ret = snprintf(Ipv6str, 40, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[0])
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[1])
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[2])
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[3])
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[4])
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[5])
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[6])
					, OS_NTOHS((*((RT_IPV6_ADDR *)(&pEntry->ipv6Addr[0]))).ipv6_addr16[7]));
			if (os_snprintf_error(40, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
			str_tmp_len = BufLen - strlen(pOutBuf);
			ret = snprintf(pOutBuf + strlen(pOutBuf),
					str_tmp_len, "%-40s%02x:%02x:%02x:%02x:%02x:%02x\n",
					Ipv6str, pEntry->macAddr[0], pEntry->macAddr[1], pEntry->macAddr[2],
					pEntry->macAddr[3], pEntry->macAddr[4], pEntry->macAddr[5]);
			if (os_snprintf_error(str_tmp_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}
	}
}

#endif /* MAT_SUPPORT */

