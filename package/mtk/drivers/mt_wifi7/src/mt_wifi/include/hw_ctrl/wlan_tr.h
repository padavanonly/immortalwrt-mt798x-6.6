/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2021, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************
 */

#ifndef __WLAN_TR_H__
#define __WLAN_TR_H__
#include <linux/cache.h>
struct wifi_dev;
struct _MAC_TABLE_ENTRY;
struct _STA_TR_ENTRY;
struct _STA_ADMIN_CONFIG;
struct _BSS_STRUCT;
struct phy_params;
struct _CIPHER_KEY;

#define SUBTYPE_PROBE_RSP 5
#define SUBTYPE_BEACON 8

#define LEN_PN 6

#define MAX_RSSI_LEN 5

struct _QUEUE_ENTRY {
	struct _QUEUE_ENTRY *Next;
};

/* Queue structure */
struct _QUEUE_HEADER {
	struct _QUEUE_ENTRY *Head;
	struct _QUEUE_ENTRY *Tail;
	u32 Number;
	unsigned long state;
};

#define QUEUE_GET_NEXT_ENTRY(prQueueEntry) ((prQueueEntry)->Next)
#define RemoveHeadQueue(QueueHeader)					\
	(QueueHeader)->Head;						\
	do {								\
		struct _QUEUE_ENTRY *pNext;                             \
		if ((QueueHeader)->Head != NULL) {			\
			pNext = (QueueHeader)->Head->Next;		\
			(QueueHeader)->Head->Next = NULL;		\
			(QueueHeader)->Head = pNext;			\
			(QueueHeader)->Number--;			\
			if ((QueueHeader)->Number == 0) {		\
				(QueueHeader)->Head = NULL;		\
				(QueueHeader)->Tail = NULL;		\
			}						\
		}							\
	} while (0)


struct _QUEUE_ENTRY *remove_queue_head(struct _QUEUE_HEADER *que_h);

union _HTTRANSMIT_SETTING {
	struct {
		u32 MCS:6;
		u32 ldpc:1;
		u32 BW:4;
		u32 ShortGI:1;
		u32 STBC:1;
		u32 eTxBF:1;
		u32 iTxBF:1;
		u32 MODE:3;
		u32 padding:14;
	} field;
	u32 word;
};

union _HETRANSMIT_SETTING {
	struct {
		u32 MCS:6;
		u32 ldpc:1;
		u32 BW:4;
		u32 ShortGI:2;
		u32 STBC:1;
		u32 eTxBF:1;
		u32 iTxBF:1;
		u32 MODE:4;
		u32 Nss:4;
		u32 padding:8;
	} field;
	u32 word;
};

union _EHTTRANSMIT_SETTING {
	struct {
		u32 MCS:6;
		u32 ldpc:1;
		u32 BW:4;
		u32 ShortGI:2;
		u32 STBC:1;
		u32 eTxBF:1;
		u32 iTxBF:1;
		u32 MODE:4;
		u32 Nss:4;
		u32 padding:8;
	} field;
	u32 word;
};

#ifdef PER_PKT_CTRL_FOR_CTMR
union _TRANSMIT_SETTING_HE {
	struct {
		u16 MCS:4;
		u16 nss:2;
		u16 ldpc:1;
		u16 BW:2;
		u16 GILTF:2;
		u16 STBC:1;
		u16 MODE:4;
	} field;
	u16 word;
};
#endif

union _PHY_RATE_SETTING {
	struct {
		u32 Mcs:6;
		u32 Gi:2;
		u32 Mode:5;
		u32 Nss:3;
		u32 Bw:3;
		u32 Ldpc:1;
		u32 Stbc:1;
		u32 Padding:11;
	} field;
	u32 Word;
};

enum TX_RESOURCE_TYPE {
	TX_UNKOWN_RESOURCE,
	TX_LEGACY_RESOURCE = 1,
	TX_BMC_RESOURCE = 2,
	TX_HIGHPRIO_RESOURCE = 3,
};

enum TX_FRAME_TYPE {
	TX_UNKOWN_FRAME,
	TX_BMC_FRAME = 1,
	TX_LEGACY_FRAME = 2,
	TX_AMPDU_FRAME = 3,
	TX_AMSDU_FRAME = 4,
	TX_FRAG_FRAME = 5,
	TX_MLME_MGMTQ_FRAME = 6,
	TX_MLME_DATAQ_FRAME = 7,
	TX_ATE_FRAME = 8,
	TX_VERIFY_FRAME = 9,
};

enum TX_BLK_FLAGS {
	fTX_bRtsRequired = (1 << 0),
	fTX_bAckRequired = (1 << 1),
	fTX_bSwPN = (1 << 2),
	fTX_bHTRate = (1 << 3),
	fTX_bWMM = (1 << 4),
	fTX_bAllowFrag = (1 << 5),
	fTX_bMoreData = (1 << 6),
	fTX_bRetryUnlimit = (1 << 7),
	fTX_bClearEAPFrame = (1 << 8),
	fTX_bApCliPacket = (1 << 9),
	fTX_bSwEncrypt = (1 << 10),
	fTX_bWMM_UAPSD_EOSP = (1 << 11),
	fTX_bWDSEntry = (1 << 12),
	fTX_bDonglePkt = (1 << 13),
	fTX_bMeshEntry = (1 << 14),
	fTX_bWPIDataFrame = (1 << 15),
	fTX_bClientWDSFrame = (1 << 16),
	fTX_bAddBA = (1 << 17),
	fTX_AmsduInAmpdu = (1 << 18),
	fTX_ForceRate = (1 << 19),
	fTX_CT_WithTxD = (1 << 20),
	fTX_ForceLink = (1 << 21),
	fTX_DumpPkt = (1 << 22),
	fTX_HDR_TRANS = (1 << 23),
	fTX_MCU_OFFLOAD = (1 << 24),
	fTX_MCAST_CLONE = (1 << 25),
	fTX_HIGH_PRIO = (1 << 26),
#ifdef A4_CONN
	fTX_bA4Frame = (1 << 27),
#endif
	fTX_bNoRetry = (1 << 28),
	fTX_bAteTxsRequired = (1 << 29),
	fTX_bAteAgg = (1 << 30),
	fTX_bMMIE = (1 << 31)
};

enum TX_BLK_FLAGS2 {
	fTX_bSnVld = (1 << 0),
	fTX_bSwSTA_WithOutWTBL = (1 << 1),
	fTX_SW_PATH = (1 << 2),
	fTX_bPerPkt = (1 << 3),
	fTX_bPsmBySw = (1 << 4),
};

struct _TX_BLK {
	u8 QueIdx  ____cacheline_aligned;
	u8 TotalFrameNum;
	u16 TotalFrameLen;
	void *pPacket;
	struct wifi_dev *wdev;
	struct _MAC_TABLE_ENTRY	*pMacEntry;
	struct _STA_TR_ENTRY *tr_entry;
	enum TX_BLK_FLAGS Flags;
	enum TX_BLK_FLAGS2 Flags2;
	u8 TxFrameType;
	enum TX_RESOURCE_TYPE TxResourceType;
	u8 UserPriority;
	u16 Wcid;
	u16 sn;
	u8 wmm_set;
	u8 dbdc_band;
	u8 *pSrcBufHeader;
	u32 SrcBufLen;
	u8 *pSrcBufData;
	u16 tx_bytes_len;
	struct _QUEUE_HEADER TxPacketList;
	union _HTTRANSMIT_SETTING *pTransmit;
	u8 TxRate;
	u32 CipherAlg;
	struct _CIPHER_KEY *pKey;
	u8 KeyIdx;
	struct _STA_ADMIN_CONFIG *pApCliEntry;
	struct _BSS_STRUCT *pMbss;
	u8 dot11_type;
	u8 dot11_subtype;
	/* TXS packet */
	u8 TxSFormat;
	u8 Pid;
	u8 TxS2Host;
	u8 TxS2Mcu;
	u8 lmac_qidx;
	u8 resource_idx;
	struct phy_params *phy_info;
	u8 DropPkt;
	u8 fr_tbl_idx;
	u8 fr_bw;
	u8 sw_pn[LEN_PN];
	u8 txpwr_offset;
	/* Fragment packet */
	u8 TotalFragNum;
	u8 FragIdx;
#ifdef PER_PKT_CTRL_FOR_CTMR
	u8 ApplyTid;
	u8 Tid;
	u8 ApplyRetryLimit;
	u8 RemainingTxCount;
	u8 RemainingTxTime;
	u8 ApplyFixRate;
	u8 FrIdx;
	u8 bw;
	u8 ApplyBaPowerTxs;
	u8 BaDisable;

	union _TRANSMIT_SETTING_HE TransmitHeByHost;
#endif
	/* 802.11 packet */
	u8 HdrPadLen;
	u8 MpduHeaderLen;
	u8 wifi_hdr_len;
	u8 *wifi_hdr;
	u8 *pExtraLlcSnapEncap;
	u8 *HeaderBuf;
	u32 HeaderBuffer[32] ____cacheline_aligned;
};

enum TXS_STS_T {
	TXS_STS_OK = 0,
	TXS_STS_TO = 1,
	TXS_STS_NG = 2,
	TXS_STS_END,
};

struct txs_info_t {
	u8 txs_sts;
	u8 pid;
	u32 tr_delay; /* in usecs */
};

#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))

#define TX_BLK_SET_FLAG2(_pTxBlk, _flag)		(_pTxBlk->Flags2 |= _flag)
#define TX_BLK_TEST_FLAG2(_pTxBlk, _flag)	(((_pTxBlk->Flags2 & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG2(_pTxBlk, _flag)	(_pTxBlk->Flags2 &= ~(_flag))

struct _RXINFO_STRUC {
	u32 BA:1;
	u32 DATA:1;
	u32 NULLDATA:1;
	u32 FRAG:1;
	u32 U2M:1;
	u32 Mcast:1;
	u32	Bcast:1;
	u32	MyBss:1;
	u32	Crc:1;
	u32	CipherErr:2;
	u32	AMSDU:1;
	u32	HTC:1;
	u32	RSSI:1;
	u32	L2PAD:1;
	u32	AMPDU:1;
	u32	Decrypted:1;
	u32	BssIdx3:1;
	u32	wapi_kidx:1;
	u32	pn_len:3;
	u32	sw_fc_type0:1;
	u32 sw_fc_type1:1;
	u32 probe_rsp:1;
	u32 beacon:1;
	u32 disasso:1;
	u32 deauth:1;
	u32 action_wanted:1;
	u32 rsv:1;
	u32 vlan_taged_tcp_sum_err:1;
	u32 hdr_trans_ip_sum_err:1;
};

union _RXWI_STRUC {
	u32 word;
};

enum RX_BLK_FLAGS {
	fRX_AMPDU = (1 << 0),
	fRX_AMSDU = (1 << 1),
	fRX_HTC = (1 << 3),
	fRX_PAD = (1 << 4),
	fRX_QOS = (1 << 5),
	fRX_EAP = (1 << 6),
	fRX_WPI = (1 << 7),
	fRX_AP = (1 << 8),
	fRX_STA = (1 << 9),
	fRX_ADHOC = (1 << 10),
	fRX_WDS = (1 << 11),
	fRX_MESH = (1 << 12),
	fRX_DLS = (1 << 13),
	fRX_TDLS = (1 << 14),
	fRX_RETRIEVE = (1 << 15),
	fRX_CMD_RSP = (1 << 16),
	fRX_TXRX_RXV = (1 << 17),
	fRX_HDR_TRANS = (1 << 18),
	fRX_WCID_MISMATCH = (1 << 19),
	fRX_CM = (1 << 20),
	fRX_CLM = (1 << 21),
	fRX_ICV_ERR = (1 << 22),
	fRX_TKIP_MIC_ERR = (1 << 23),
};

enum RX_BLK_ERR_FLAGS {
	fRX_ERR_IP_FRAG_OLD_PKT = (1 << 0),
	fRX_ERR_PN_CHK_FAIL = (1 << 1),
	fRX_ERR_OLD_PKT = (1 << 2),
};

struct rx_signal_info {
	u8 raw_rssi[MAX_RSSI_LEN];
	u8 raw_snr[MAX_RSSI_LEN];
	s8 freq_offset;
};

#ifdef CFG_BIG_ENDIAN
struct _RX_CSO_STRUCT {
	u32 Rsv11to31:21;
	u32 UnknownNextHdr:1;
	u32 IpFrag:1;
	u32 IpLenMismatch:1;
	u32 ChksumType:4;
	u32 ChksumStatus:4;
};
#else
struct _RX_CSO_STRUCT {
	u32 ChksumStatus:4;
	u32 ChksumType:4;
	u32 IpLenMismatch:1;
	u32 IpFrag:1;
	u32 UnknownNextHdr:1;
	u32 Rsv11to31:21;
};
#endif

struct IEEE80211_RADIOTAP_INFO {
	/* common */
	u8 ucFcsErr;
	u8 ucFrag;
	u8 ucRxMode;
	u8 ucFrMode;
	u8 ucShortGI;
	u8 ucLDPC;
	u8 ucSTBC;
	u8 ucNess;
	u8 ucMcs;
	u8 ucRcpi0;
	u8 ucRcpi1;
	u8 ucRfBand;
	u8 ucTxopPsNotAllow;
	u8 ucLdpcExtraOfdmSym;
	u8 ucNsts;
	u8 ucBeamFormed;
	u8 ucSubNamespace;
	u16 u2ChFrequency;
	u16 u2VendorLen;
	u32 u4AmpduRefNum;
	u32 u4Timestamp;
	/* VHT */
	u8 ucVhtGroupId;
	/* HE */
	u8 ucPeDisamb;
	u8 ucNumUser;
	u8 ucSigBRU0;
	u8 ucSigBRU1;
	u8 ucSigBRU2;
	u8 ucSigBRU3;
	u16 u2VhtPartialAid;
	u16 u2BssClr;
	u16 u2BeamChange;
	u16 u2UlDl;
	u16 u2DataDcm;
	u16 u2SpatialReuse1;
	u16 u2SpatialReuse2;
	u16 u2SpatialReuse3;
	u16 u2SpatialReuse4;
	u16 u2RuAllocation;
	u16 u2Ltf;
	u16 u2Doppler;
	u16 u2Txop;
	u8 ucPpduTypeComp;
	u8 ucEhtSigMcs;
	u8 ucEhtLtfSymNum;
	u8 ucEhtSigSymNum;
};

struct _RX_BLK {
	u8 hw_rx_info[16];
	struct _RXINFO_STRUC *pRxInfo;
	union _RXWI_STRUC *pRxWI;
	u8 *rmac_info;
	u8 *FC;
	u16 Duration;
	u8 FN;
	u16 SN;
	u8 *Addr1;
	u8 *Addr2;
	u8 *Addr3;
	u8 *Addr4;
	void *pRxPacket;
	u8 *pData;
	u16 DataSize;
	enum RX_BLK_FLAGS Flags;
	enum RX_BLK_ERR_FLAGS ErrFlags;
	u16 MPDUtotalByteCnt;
	u8 UserPriority;
	u8 OpMode;
	u8 band;
	u8 link_band;
	u16 wcid;
	u16 link_wcid;
	u8 U2M;
	u8 key_idx;
	u8 bss_idx;
	u8 TID;
	u32 TimeStamp;
	struct rx_signal_info rx_signal;
	s8 ldpc_ex_sym;
	union _HTTRANSMIT_SETTING rx_rate;
	u32 rxv2_cyc1;
	u32 rxv2_cyc2;
	u32 rxv2_cyc3;
	struct _RX_CSO_STRUCT rCso;
	u16 token_id;
	u8 sec_mode;
	u8 AmsduState;
	u8 DeAmsduFail;
	u64 CCMP_PN;
	u16 channel_freq;
	u8 rcpi[4];
	u8 IsDoProfling;
	u16 group_id;
	u16 pkt_id;
	u8 *AMSDU_ADDR;
	u8 is_htf;
	u16 raw_channel;
	u8 sniffermode;
	struct IEEE80211_RADIOTAP_INFO radiotap_info;
};

struct mtk_tx_status {
	u8 band_idx;
	u16 wcid;
	u8 qid;
	u32 stat;
	u32 cnt;
	u32 air_latency;
	u32 mac_latency;
};

#ifdef ZERO_PKT_LOSS_SUPPORT
struct mtk_null_tx_status {
	u8 band_idx;
	u16 wcid;
	u32 stat;
};
#endif

#define RX_BLK_SET_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags |= _flag)
#define RX_BLK_TEST_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags & _flag)
#define RX_BLK_CLEAR_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags &= ~(_flag))

#define RX_BLK_SET_ERR_FLAG(_pRxBlk, _flag)	(_pRxBlk->ErrFlags |= _flag)
#define RX_BLK_TEST_ERR_FLAG(_pRxBlk, _flag)	(_pRxBlk->ErrFlags & _flag)
#define RX_BLK_CLEAR_ERR_FLAG(_pRxBlk, _flag)	(_pRxBlk->ErrFlags &= ~(_flag))

#define PACKET_SPECIFIC_HIGH_PRIO 0x08
#define PACKET_HS2_TX 0x10
#define CB_OFF  10
#define CB_LEN 37
#ifdef NONE
#define RTPKT_TO_OSPKT(_p)		((WF_PKT_T *)(_p))
#else /* NONE */
#define RTPKT_TO_OSPKT(_p)		((struct sk_buff *)(_p))
#endif /* LINUX */
#define GET_OS_PKT_CB(_p)		(RTPKT_TO_OSPKT(_p)->cb)
#define PACKET_CB(_p, _offset)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF + (_offset)]))
#define GET_PACKET_HIGH_PRIO(_p)\
	(PACKET_CB(_p, 36) & PACKET_SPECIFIC_HIGH_PRIO)
#define GET_PACKET_HS2_TX(_p)\
	(PACKET_CB(_p, 34) & (PACKET_HS2_TX))

#define PACKET_SPECIFIC_VLAN	0x10
#define PACKET_SPECIFIC_TCP		0x40
#define GET_PACKET_TCP(_p)	\
	(PACKET_CB(_p, 6) & PACKET_SPECIFIC_TCP)
#define GET_PACKET_VLAN(_p)	\
	(PACKET_CB(_p, 6) & PACKET_SPECIFIC_VLAN)

#define GET_PACKET_PROTOCOL(_p) \
	(((PACKET_CB(_p, 11)) << 8) \
	 | (PACKET_CB(_p, 10)))

/* Packet type */
#define SET_PACKET_TYPE(_p, _idx)   (PACKET_CB(_p, 37) = (_idx))

u8 is_amsdu_capable(void *pkt);

#endif
