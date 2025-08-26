/******************************************************************************
 *
 * Copyright (c) 2011 TP-LINK Technologies CO.,LTD.
 * All rights reserved.
 *
 * FILE NAME  :   wlan_8023.h
 * VERSION    :   1.0
 * DESCRIPTION:   Fundamental types and constants relating to 802.3.
 *
 * AUTHOR     :   zhengxinggu <zhengxinggu@tp-link.net>
 * CREATE DATE:   03/24/2011
 *
 * HISTORY    :
 * 01   03/24/2011  zhengxinggu     Create.
 * 02   03/18/2013  xiejingfeng     Add tcp header structure declaratioin
 * 03   03/19/2013  xiejingfeng     Remove tcp header flages defines,the kernel defines them
 * 04   07/25/2014  yangyang        Copy from TPWD.
 *
 ******************************************************************************/
#ifndef _UMAC_8023_H_
#define _UMAC_8023_H_

#define IEEE8023_MAX_LEN	0x600		/* 1536 - larger is Ethernet II */

//#define IP_HDR_LEN(iphdr)	((iphdr->ver & 0x0F) * 4)

#define ETHERTYPE_8021X		0x888e

#define IP_PROTO_UDP		0x11

/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct ethernet_header
{
	UINT8	daddr[MAC_ADDR_LEN];
	UINT8	saddr[MAC_ADDR_LEN];
	UINT16	type;
} __packed;

/*
 * Structure of the IP frame
 */
struct ip_header
{
	UINT8	ver;
	UINT8	tos;
	UINT16	len;
	UINT16	id;
	UINT16	frag;
	UINT8	ttl;
	UINT8	proto;
	UINT16	check;
	UINT32	saddr;
	UINT32	daddr;
	/*The options start here. */
} __packed;

/*
 * Udp protocol header.
 */
struct udp_header
{
	UINT16	sport;		/* source port */
	UINT16	dport;		/* destination port */
	UINT16	ulen;		/* udp length */
	UINT16	sum;		/* udp checksum */
} __packed;


struct tcp_header {
	UINT16	 th_sport;		/* source port */
	UINT16	th_dport;		/* destination port */
	UINT32	th_seq;			/* sequence number */
	UINT32	th_ack;			/* acknowledgement number */
	UINT32	th_off:4,		/* data offset */
			th_x2:4;		/* (unused) */
	UINT8	th_flags;
	UINT16	th_win;			/* window */
	UINT16	th_sum;			/* checksum */
	UINT16	th_urp;			/* urgent pointer */
}__packed;

struct arp_header
{
	UINT16	htype;					/* Header type (1 = ethernet) */
	UINT16	ptype;					/* Protocol type (0x800 = IP) */
	UINT8	hlen;					/* Hardware address length (Eth = 6) */
	UINT8	plen;					/* Protocol address length (IP = 4) */
	UINT16	oper;					/* ARP_OPC_... */
	UINT8	srcmac[MAC_ADDR_LEN];	/* Source hardware address */
	UINT8	srcip[IPV4_ADDR_LEN];	/* Source protocol address (not aligned) */
	UINT8	dstmac[MAC_ADDR_LEN];	/* Destination hardware address */
	UINT8	dstip[IPV4_ADDR_LEN];	/* Destination protocol address */
} __packed;

struct pppoe_header
{
	UINT8		type;
	UINT8		code;
	UINT16		sid;
	UINT16		length;
} __packed;


#endif /* _UMAC_8023_H_ */
