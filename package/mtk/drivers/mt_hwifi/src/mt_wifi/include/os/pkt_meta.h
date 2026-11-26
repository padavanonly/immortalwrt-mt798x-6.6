/*

*/

#define MIN_NET_DEVICE_FOR_MBSSID		0x00		/*0x00,0x10,0x20,0x30 */
#define MIN_NET_DEVICE_FOR_WDS			MAX_BEACON_NUM
#define MIN_NET_DEVICE_FOR_APCLI		(MIN_NET_DEVICE_FOR_WDS+0x10)
#define MIN_NET_DEVICE_FOR_MESH			(MIN_NET_DEVICE_FOR_APCLI+0x10)
#ifdef CONFIG_STA_SUPPORT
#define MIN_NET_DEVICE_FOR_DLS			(MIN_NET_DEVICE_FOR_MESH+0x10)
#define MIN_NET_DEVICE_FOR_TDLS			(MIN_NET_DEVICE_FOR_TDLS+0x10)
#endif /* CONFIG_STA_SUPPORT */

#define NET_DEVICE_REAL_IDX_MASK		(MAX_BEACON_NUM-1)		/* for each operation mode, we maximum support (MAX_BEACON_NUM-1) entities. */

#define PACKET_BROADCAST	1		/* To all		*/
#define PACKET_MULTICAST	2		/* To group		*/

/******************************************************************************
	Packet Meta info fields
	NOTE: the meta info may stored in some os-dependent packet buffer and may
			have extra limitations.
		1. Linux:
			a).re-use sk_budd->cb[]
			b).Max space of skb->cb = 48B = [CB_OFF+38]
		2. VxWorks:
			a).
			b).Max space: 32!!
******************************************************************************/
/* [CB_OFF + 0]  */
/* User Priority */
#define RTMP_SET_PACKET_UP(_p, _prio)			(PACKET_CB(_p, 0) = _prio)
#define RTMP_GET_PACKET_UP(_p)					(PACKET_CB(_p, 0))

/* [CB_OFF + 1]  */
/* Fragment # */
#define RTMP_SET_PACKET_FRAGMENTS(_p, _num)	(PACKET_CB(_p, 1) = _num)
#define RTMP_GET_PACKET_FRAGMENTS(_p)		(PACKET_CB(_p, 1))

/* [CB_OFF + 2]  */
#ifdef A4_CONN
#define RTMP_SET_PACKET_A4_FWDDATA(_p, _flg)   (PACKET_CB(_p, 2) = _flg)
#define RTMP_GET_PACKET_A4_FWDDATA(_p)         (PACKET_CB(_p, 2))
#endif



/* [CB_OFF + 3]  */
/* From which Interface, wdev index */
#define RTMP_SET_PACKET_WDEV(_p, _wdev_idx)		(PACKET_CB(_p, 3) = _wdev_idx)
#define RTMP_GET_PACKET_WDEV(_p)					(PACKET_CB(_p, 3))



/* [CB_OFF + 4]  */
/* If this flag is set, it indicates that this EAPoL frame MUST be clear. */
#define RTMP_SET_PACKET_CLEAR_EAP_FRAME(_p, _flg)   (PACKET_CB(_p, 4) = _flg)
#define RTMP_GET_PACKET_CLEAR_EAP_FRAME(_p)         (PACKET_CB(_p, 4))


/* [CB_OFF + 5]  */
#define RTMP_SET_PACKET_MOREDATA(_p, _morebit)		(PACKET_CB(_p, 5) = _morebit)
#define RTMP_GET_PACKET_MOREDATA(_p)					(PACKET_CB(_p, 5))


/* [CB_OFF + 6]  */
/*
	Sepcific Pakcet Type definition
*/
#define RTMP_PACKET_SPECIFIC_CB_OFFSET	6

#define RTMP_PACKET_SPECIFIC_DHCP		0x01
#define RTMP_PACKET_SPECIFIC_EAPOL	0x02
#define RTMP_PACKET_SPECIFIC_IPV4		0x04
#define RTMP_PACKET_SPECIFIC_WAI		0x08
#define RTMP_PACKET_SPECIFIC_VLAN		0x10
#define RTMP_PACKET_SPECIFIC_LLCSNAP	0x20
#define RTMP_PACKET_SPECIFIC_TCP		0x40
#define RTMP_PACKET_SPECIFIC_DISASSOC	0x80

/* Specific */
#define RTMP_SET_PACKET_SPECIFIC(_p, _flg)	(PACKET_CB(_p, 6) = _flg)

/* DHCP */
#define RTMP_SET_PACKET_DHCP(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_DHCP);	\
		else													\
			PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_DHCP);	\
	} while (0)
#define RTMP_GET_PACKET_DHCP(_p)\
	(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_DHCP)

/* EAPOL */
#define RTMP_SET_PACKET_EAPOL(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_EAPOL);	\
		else													\
			PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_EAPOL);	\
	} while (0)
#define RTMP_GET_PACKET_EAPOL(_p)\
	(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_EAPOL)

/* WAI */
#define RTMP_SET_PACKET_WAI(_p, _flg)	\
	do {		\
		if (_flg)												\
			PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_WAI);	\
		else													\
			PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_WAI);	\
	} while (0)
#define RTMP_GET_PACKET_WAI(_p) \
	(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_WAI)

#define RTMP_GET_PACKET_FORCE_TX(_p)\
	(PACKET_CB(_p, 6) & (RTMP_PACKET_SPECIFIC_EAPOL | \
						 RTMP_PACKET_SPECIFIC_WAI))

#define RTMP_GET_PACKET_LOWRATE(_p) \
	((PACKET_CB(_p, 6) & (RTMP_PACKET_SPECIFIC_EAPOL |\
						 RTMP_PACKET_SPECIFIC_DHCP |\
						 RTMP_PACKET_SPECIFIC_WAI)) ||\
		(PACKET_CB(_p, 36) & (RTMP_PACKET_SPECIFIC_ARP)))

/* VLAN */
#define RTMP_SET_PACKET_VLAN(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_VLAN);	\
		else													\
			PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_VLAN);	\
	} while (0)
#define RTMP_GET_PACKET_VLAN(_p) \
	(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_VLAN)

/* LLC/SNAP */
#define RTMP_SET_PACKET_LLCSNAP(_p, _flg)	\
	do {																\
		if (_flg)													\
			PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_LLCSNAP);	\
		else														\
			PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_LLCSNAP);	\
	} while (0)

#define RTMP_GET_PACKET_LLCSNAP(_p) \
	(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_LLCSNAP)

/* TCP */
#define RTMP_SET_PACKET_TCP(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_TCP);	\
		else													\
			PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_TCP);	\
	} while (0)
#define RTMP_GET_PACKET_TCP(_p)\
	(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_TCP)

/* IP */
#define RTMP_SET_PACKET_IPV4(_p, _flg)									\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 6) |= (RTMP_PACKET_SPECIFIC_IPV4);	\
		else													\
			PACKET_CB(_p, 6) &= (~RTMP_PACKET_SPECIFIC_IPV4);	\
	} while (0)

#define RTMP_GET_PACKET_IPV4(_p) \
	(PACKET_CB(_p, 6) & RTMP_PACKET_SPECIFIC_IPV4)

/* DISASSOC */
#define RTMP_SET_PACKET_DISASSOC(_p, _flg)                                                      \
	do {                                                                             \
		if (_flg)                                                                   \
			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) |= (RTMP_PACKET_SPECIFIC_DISASSOC);     \
		else                                                                        \
			(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11]) &= (~RTMP_PACKET_SPECIFIC_DISASSOC);    \
	} while (0)
#define RTMP_GET_PACKET_DISASSOC(_p)   \
	(RTPKT_TO_OSPKT(_p)->cb[CB_OFF+11] & RTMP_PACKET_SPECIFIC_DISASSOC)


/* [CB_OFF + 7]  */
#define RTMP_SET_PACKET_OPMODE(_p, _flg)   (PACKET_CB(_p, 7) = _flg)
#define RTMP_GET_PACKET_OPMODE(_p)         (PACKET_CB(_p, 7))

/* [CB_OFF + 8 ~ 9]  */
#ifdef UAPSD_SUPPORT
/* if we queue a U-APSD packet to any software queue, we will set the U-APSD
   flag and its physical queue ID for it */
#define RTMP_SET_PACKET_UAPSD(_p, _flg_uapsd, _que_id) \
	(PACKET_CB(_p, 8) = ((_flg_uapsd<<7) | _que_id))

#define RTMP_SET_PACKET_QOS_NULL(_p)			(PACKET_CB(_p, 8) = 0xff)
#define RTMP_GET_PACKET_QOS_NULL(_p)			(PACKET_CB(_p, 8))
#define RTMP_SET_PACKET_NON_QOS_NULL(_p)		(PACKET_CB(_p, 8) = 0x00)
#define RTMP_GET_PACKET_UAPSD_Flag(_p)		((PACKET_CB(_p, 8) & 0x80) >> 7)
#define RTMP_GET_PACKET_UAPSD_QUE_ID(_p)		(PACKET_CB(_p, 8) & 0x7f)

#define RTMP_SET_PACKET_EOSP(_p, _flg)			(PACKET_CB(_p, 9) = _flg)
#define RTMP_GET_PACKET_EOSP(_p)				(PACKET_CB(_p, 9))
#endif /* UAPSD_SUPPORT */


/* [CB_OFF + 10 ~ 11]  */
#define RTMP_SET_PACKET_PROTOCOL(_p, _protocol) {\
		(PACKET_CB(_p, 10) = (UINT8)((_protocol) & 0x00ff)); \
		(PACKET_CB(_p, 11) = (UINT8)(((_protocol) & 0xff00) >> 8)); \
	}

#define RTMP_GET_PACKET_PROTOCOL(_p) \
	((((UINT16)PACKET_CB(_p, 11)) << 8) \
	 | ((UINT16)PACKET_CB(_p, 10)))


/* [CB_OFF + 12]  */
#define RTMP_SET_PACKET_TXTYPE(_p, _mode)		(PACKET_CB(_p, 12) = _mode)
#define RTMP_GET_PACKET_TXTYPE(_p)			(PACKET_CB(_p, 12))

/* [CB_OFF + 13]  */
/* TX rate index */
#define RTMP_SET_PACKET_TXRATE(_p, _rate)		(PACKET_CB(_p, 13) = _rate)
#define RTMP_GET_PACKET_TXRATE(_p)			(PACKET_CB(_p, 13))


/* Band Index */
#define RTMP_SET_BAND_IDX(_p, _idx)   		(PACKET_CB(_p, 14) = _idx)
#define RTMP_GET_BAND_IDX(_p)         		(PACKET_CB(_p, 14))


/* [CB_OFF + 15 ~ 19]  */

/* use bit3 of cb[CB_OFF+16] */
#define RTMP_SET_PACKET_MGMT_PKT(_p, _flg)	        \
	(PACKET_CB(_p, 16) = (PACKET_CB(_p, 16) & 0xF7) | ((_flg & 0x01) << 3))
#define RTMP_GET_PACKET_MGMT_PKT(_p)				\
	((PACKET_CB(_p, 16) & 0x08) >> 3)

/* [CB_OFF + 20]  used for specific packet feature */
#define RTMP_PACKET_MGMT_PKT_DATA_QUE			0x01
#define RTMP_PACKET_SN_VLD			0x02

/* MGMT DATA QUEUE */
#define RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 20) |= (RTMP_PACKET_MGMT_PKT_DATA_QUE);	\
		else													\
			PACKET_CB(_p, 20) &= (~RTMP_PACKET_MGMT_PKT_DATA_QUE);	\
	} while (0)
#define RTMP_GET_PACKET_MGMT_PKT_DATA_QUE(_p)\
	(PACKET_CB(_p, 20) & RTMP_PACKET_MGMT_PKT_DATA_QUE)
/* SN_VLD */
#define RTMP_SET_PACKET_SN_VLD(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 20) |= (RTMP_PACKET_SN_VLD);	\
		else													\
			PACKET_CB(_p, 20) &= (~RTMP_PACKET_SN_VLD);	\
	} while (0)
#define RTMP_GET_PACKET_SN_VLD(_p)\
	(PACKET_CB(_p, 20) & RTMP_PACKET_SN_VLD)

/* [CB_OFF + 21 ~ 22] used for SN=0~4095  */
#define RTMP_SET_PACKET_SN(_p, _sn) {\
		(PACKET_CB(_p, 21) = (UINT8)((_sn) & 0x00ff)); \
		(PACKET_CB(_p, 22) = (UINT8)(((_sn) & 0xff00) >> 8)); \
	}

#define RTMP_GET_PACKET_SN(_p) \
	((((UINT16)PACKET_CB(_p, 22)) << 8) \
	 | ((UINT16)PACKET_CB(_p, 21)))

/* [CB_OFF + 23]  */
/*
 *	TDLS Sepcific Pakcet Type definition
*/
#define RTMP_TDLS_SPECIFIC_WAIT_ACK		0x01
#define RTMP_TDLS_SPECIFIC_NOACK			0x02
#define RTMP_TDLS_SPECIFIC_PKTQ_HCCA		0x04
#define RTMP_TDLS_SPECIFIC_PKTQ_EDCA		0x08

#define RTMP_SET_PACKET_TDLS_WAIT_ACK(_p, _flg)						\
	do {															\
		if (_flg)													\
			PACKET_CB(_p, 23) |= (RTMP_TDLS_SPECIFIC_WAIT_ACK);	\
		else														\
			PACKET_CB(_p, 23) &= (~RTMP_TDLS_SPECIFIC_WAIT_ACK);	\
	} while (0)

#define RTMP_GET_PACKET_TDLS_WAIT_ACK(_p)		(PACKET_CB(_p, 23) & RTMP_TDLS_SPECIFIC_WAIT_ACK)

#define RTMP_SET_PACKET_TDLS_NO_ACK(_p, _flg)						\
	do {														\
		if (_flg)												\
			PACKET_CB(_p, 23) |= (RTMP_TDLS_SPECIFIC_NOACK);	\
		else													\
			PACKET_CB(_p, 23) &= (~RTMP_TDLS_SPECIFIC_NOACK);	\
	} while (0)

#define RTMP_GET_PACKET_TDLS_NO_ACK(_p)		(PACKET_CB(_p, 23) & RTMP_TDLS_SPECIFIC_NOACK)

#define RTMP_SET_TDLS_SPECIFIC_PACKET(_p, _flg)   (PACKET_CB(_p, 23) = _flg)
#define RTMP_GET_TDLS_SPECIFIC_PACKET(_p)         (PACKET_CB(_p, 23))

/* [CB_OFF + 24 ~ 25]  */
/*
	0x0 ~ 0xff: index mapping to MAC_TABLE_ENTRY
				for Tx, search by eth_hdr.addr1
				for Rx, get from ASIC(RxWI)/search by wlan_hdr.addr2
*/

#define RTMP_SET_PACKET_WCID(_p, _wcid)	(*(UINT16 *)&PACKET_CB(_p, 24) = _wcid)
#define RTMP_GET_PACKET_WCID(_p)		(*(UINT16 *)&PACKET_CB(_p, 24))


/* [CB_OFF + 26]  */
#ifdef CONFIG_CSO_SUPPORT
#define RTMP_SET_TCP_CHKSUM_FAIL(_p, _flg)	(PACKET_CB(_p, 26) = _flg);
#define RTMP_GET_TCP_CHKSUM_FAIL(_p)		(PACKET_CB(_p, 26))
#endif /* CONFIG_CSO_SUPPORT */

/* [CB_OFF + 27]  */
#define RTMP_SET_PACKET_PENDING(_p, _idx)   (PACKET_CB(_p, 27) = _idx)
#define RTMP_GET_PACKET_PENDING(_p)         (PACKET_CB(_p, 27))

/* [CB_OFF + 28]  */
#ifdef VLAN_SUPPORT
#define RTMP_SET_VLAN_PCP(_p, _flg)	(PACKET_CB(_p, 28) = (UINT8)((_flg) & 0x00ff))
#define RTMP_GET_VLAN_PCP(_p)		(PACKET_CB(_p, 28))
#endif /* VLAN_SUPPORT */

/* [CB_OFF + 31]  */
#ifdef CONFIG_VLAN_GTK_SUPPORT
#define RTMP_SET_PACKET_VLANGTK(_p, _num)		(PACKET_CB(_p, 31) = _num)
#define RTMP_GET_PACKET_VLANGTK(_p)			(PACKET_CB(_p, 31))
#endif

/*[CB_OFF + 29] used for debugger*/
#define RTMP_SET_DEBUGGER_SKIP_DUMP(_p, _val)	(PACKET_CB(_p, 29) = _val)
#define RTMP_GET_DEBUGGER_SKIP_DUMP(_p)		(PACKET_CB(_p, 29))

/* [CB_OFF + 30~31]  */
/* For Tx Mgmt Frame Ack Enhancement*/
#define RTMP_SET_PACKET_REQ_TXS(_p, _flg)	(PACKET_CB(_p, 30) = _flg)
#define RTMP_GET_PACKET_REQ_TXS(_p)		(PACKET_CB(_p, 30))

#define RTMP_SET_PACKET_PID(_p, _val)	(PACKET_CB(_p, 31) = _val)
#define RTMP_GET_PACKET_PID(_p)		(PACKET_CB(_p, 31))

/* [CB_OFF + 32]  */
/* RTS/CTS-to-self protection method */
#define RTMP_SET_PACKET_RTS(_p, _num)			(PACKET_CB(_p, 32) = _num)
#define RTMP_GET_PACKET_RTS(_p)				(PACKET_CB(_p, 32))
/* see RTMP_S(G)ET_PACKET_EMACTAB */


/* Release not used CB  */
/* [CB_OFF + 33] */
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
#define RTMP_PACKET_DIRECT_TX		0x40
#define RTMP_SET_PACKET_DIRECT_TX(_p, _flg)	\
	do {									\
		if (_flg)								\
			PACKET_CB(_p, 34) |= (RTMP_PACKET_DIRECT_TX);		\
		else									\
			PACKET_CB(_p, 34) &= (~RTMP_PACKET_DIRECT_TX);		\
	} while (0)

#define RTMP_IS_PACKET_DIRECT_TX(_p)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF+34]) & (RTMP_PACKET_DIRECT_TX))

#endif /* CONFIG_HOTSPOT_R2 */

#define RTMP_SET_PACKET_UP_CB33(_p, _prio)	(PACKET_CB(_p, 33) = _prio)

#define RTMP_SET_PACKET_QUEIDX(_p, _idx)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+35] = (_idx))
#define RTMP_GET_PACKET_QUEIDX(_p)         (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+35])

#define RTMP_PACKET_SPECIFIC_PING 0x01
#define RTMP_PACKET_SPECIFIC_ARP 0x02
#define RTMP_PACKET_SPECIFIC_MCAST_CLONE 0x04
#define RTMP_PACKET_SPECIFIC_HIGH_PRIO 0x08
#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
#define RTMP_PACKET_HS2_TX 0x10
#define RTMP_PACKET_SPECIFIC_IPv6 0x20
#endif /* HOSTAPD_HS_R2_SUPPORT */

/* PING */
#define RTMP_SET_PACKET_PING(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 36) |= (RTMP_PACKET_SPECIFIC_PING);	\
		else													\
			PACKET_CB(_p, 36) &= (~RTMP_PACKET_SPECIFIC_PING);	\
	} while (0)
#define RTMP_GET_PACKET_PING(_p)\
	(PACKET_CB(_p, 36) & RTMP_PACKET_SPECIFIC_PING)

/* ARP */
#define RTMP_SET_PACKET_ARP(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 36) |= (RTMP_PACKET_SPECIFIC_ARP);	\
		else													\
			PACKET_CB(_p, 36) &= (~RTMP_PACKET_SPECIFIC_ARP);	\
	} while (0)
#define RTMP_GET_PACKET_ARP(_p)\
	(PACKET_CB(_p, 36) & RTMP_PACKET_SPECIFIC_ARP)

/* MCAST CLONE for IGMP */
#define RTMP_SET_PACKET_MCAST_CLONE(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 36) |= (RTMP_PACKET_SPECIFIC_MCAST_CLONE);	\
		else													\
			PACKET_CB(_p, 36) &= (~RTMP_PACKET_SPECIFIC_MCAST_CLONE);	\
	} while (0)
#define RTMP_GET_PACKET_MCAST_CLONE(_p)\
	(PACKET_CB(_p, 36) & RTMP_PACKET_SPECIFIC_MCAST_CLONE)

/* High Priority packet */
#define RTMP_SET_PACKET_HIGH_PRIO(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 36) |= (RTMP_PACKET_SPECIFIC_HIGH_PRIO);	\
		else													\
			PACKET_CB(_p, 36) &= (~RTMP_PACKET_SPECIFIC_HIGH_PRIO);	\
	} while (0)
#define RTMP_GET_PACKET_HIGH_PRIO(_p)\
	(PACKET_CB(_p, 36) & RTMP_PACKET_SPECIFIC_HIGH_PRIO)

#if defined(HOSTAPD_HS_R2_SUPPORT) || defined(CONFIG_PROXY_ARP)
#define RTMP_SET_PACKET_HS2_TX(_p, _flg)             \
	do {                                                                                                                    \
		if (_flg)                                                                                               \
			PACKET_CB(_p, 34) |= (RTMP_PACKET_HS2_TX);  \
		else                                                                                                    \
			PACKET_CB(_p, 34) &= (~RTMP_PACKET_HS2_TX); \
	} while (0)

/* IPv6 Packet */
#define RTMP_SET_PACKET_IPv6(_p, _flg)		\
	do {															\
		if (_flg)												\
			PACKET_CB(_p, 34) |= (RTMP_PACKET_SPECIFIC_IPv6);	\
		else													\
			PACKET_CB(_p, 34) &= (~RTMP_PACKET_SPECIFIC_IPv6);	\
	} while (0)
#define RTMP_GET_PACKET_IPv6(_p)\
	(PACKET_CB(_p, 34) & RTMP_PACKET_SPECIFIC_IPv6)
#endif /* HOSTAPD_HS_R2_SUPPORT */

/* Packet type */
#define RTMP_SET_PACKET_TYPE(_p, _idx)   (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+37] = (_idx))
#define RTMP_GET_PACKET_TYPE(_p)         (RTPKT_TO_OSPKT(_p)->cb[CB_OFF+37])
