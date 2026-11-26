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

*/

#ifndef __HDEV_BASIC_H
#define __HDEV_BASIC_H

#include "common/link_list.h"

struct _RTMP_CHIP_CAP;
struct _RTMP_CHIP_OP;
struct _EDCA_PARM;
struct MCU_CTRL;
struct _BCTRL_INFO_T;
struct _BCTRL_ENTRY;
struct freq_oper;
struct wmm_entry;
struct pe_control;

struct radio_res {
	UCHAR reason;
	struct freq_oper *oper;
};

enum {
	REASON_NORMAL_SW,
	REASON_NORMAL_SCAN,
	REASON_ATE,
};

/*
* state machine:
* case1: NONE_OCCUPIED ->SW_OCCUPIED->NONE_OCCUPIED
*/

enum {
	WTBL_STATE_NONE_OCCUPIED = 0,
	WTBL_STATE_SW_OCCUPIED,
};

#define WTC_WAIT_TIMEOUT CMD_MSG_TIMEOUT

typedef struct _OMAC_BSS_CTRL {
	UINT32 OmacBitMap;
	UINT32 HwMbssBitMap;
	UINT32 RepeaterBitMap;
} OMAC_BSS_CTRL, *POMAC_BSS_CTRL;

struct wmm_ctrl {
	UCHAR num;
	struct wmm_entry *entries;
};


typedef struct radio_control {
	UCHAR CurStat;
	USHORT PhyMode;
	UCHAR cur_rfic_type;
	/*check first radio update is for scan or not*/
	BOOLEAN scan_state;
	BOOLEAN BfSmthIntlBypass;
#ifdef GREENAP_SUPPORT
	BOOLEAN bGreenAPActive;
#endif /* GREENAP_SUPPORT */
	UCHAR rx_stream;
#ifdef TXRX_STAT_SUPPORT
	LARGE_INTEGER TxDataPacketCount;
	LARGE_INTEGER TxDataPacketByte;
	LARGE_INTEGER RxDataPacketCount;
	LARGE_INTEGER RxDataPacketByte;
	LARGE_INTEGER TxUnicastDataPacket;
	LARGE_INTEGER TxMulticastDataPacket;
	LARGE_INTEGER TxBroadcastDataPacket;
	LARGE_INTEGER TxMgmtPacketCount;
	LARGE_INTEGER TxMgmtPacketByte;
	LARGE_INTEGER RxMgmtPacketCount;
	LARGE_INTEGER RxMgmtPacketByte;
	LARGE_INTEGER TxBeaconPacketCount;
	LARGE_INTEGER TxDataPacketCountPerAC[4];	/*per access category*/
	LARGE_INTEGER RxDataPacketCountPerAC[4];	/*per access category*/
	CHAR LastDataPktRssi[MAX_RSSI_LEN];
	LARGE_INTEGER TxPacketDroppedCount;
	LARGE_INTEGER RxDecryptionErrorCount;
	LARGE_INTEGER RxCRCErrorCount;
	LARGE_INTEGER RxMICErrorCount;
	LARGE_INTEGER LastSecTxByte;
	LARGE_INTEGER LastSecRxByte;
	UINT32 TotalPER;
	UINT32 TotalTxFailCnt;
	UINT32 TotalTxCnt;
	UINT32 Last1SecPER;
	UINT32 Last1TxFailCnt;
	UINT32 Last1TxCnt;
#endif
	UINT32 CurChannelUpTime;		/*usecs since system up*/
	UINT32 RefreshACSChannelChangeCount;
	UINT32 ForceACSChannelChangeCount;
	UINT32 ManualChannelChangeCount;
	UINT32 DFSTriggeredChannelChangeCount;
	UINT32 TotalChannelChangeCount;
	UINT8  ACSTriggerFlag;
} RADIO_CTRL;

typedef struct rtmp_phy_ctrl {
	UINT8 rf_band_cap;
#ifdef CONFIG_AP_SUPPORT
	AUTO_CH_CTRL AutoChCtrl;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL QloadCtrl;
#endif /*AP_QLOAD_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
	RADIO_CTRL RadioCtrl;
#ifdef DOT11_HE_AX
	struct pe_control pe_ctrl;
#endif
} RTMP_PHY_CTRL;

typedef struct _HD_RESOURCE_CFG {
	struct rtmp_phy_ctrl PhyCtrl;
	struct wmm_ctrl wmm_ctrl;
	UCHAR txcmd_mode;
} HD_RESOURCE_CFG;

struct radio_dev {
	RADIO_CTRL *pRadioCtrl;
};

union hif_cfg {
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T pci;
#endif /*RTMP_MAC_PCI*/
};

struct hif_ctrl {
	union hif_cfg cfg;
	struct hif_ops ops;
};


enum {
	HOBJ_STATE_NONE = 0,
	HOBJ_STATE_USED,
};

enum {
	HOBJ_TX_MODE_TXD,
	HOBJ_TX_MODE_TXCMD,
};

struct hdev_ctrl {
	struct radio_dev rdev;
	CHANNEL_CTRL ChCtrl;
	struct hif_ctrl hif;
	struct _RTMP_CHIP_OP chip_ops;
	struct _RTMP_ARCH_OP arch_ops;
	struct _RTMP_CHIP_DBG chip_dbg;
	struct mt_io_ops io_ops;
	struct mtk_hdev_ops *hdev_ops;
	struct _HD_RESOURCE_CFG HwResourceCfg;
	VOID *mcu_ctrl;
	VOID *cookie;
	VOID *priv; /*implicit point to upper struct*/
#ifdef ZERO_PKT_LOSS_SUPPORT
	BOOLEAN SuspendMsduTx;
#endif
};

/*for hdev base functions*/
BOOLEAN hdev_obj_state_ready(struct mtk_mac_bss *obj);

INT32 HdevInit(struct hdev_ctrl *hdev_ctrl, RADIO_CTRL *pRadioCtrl);
INT32 HdevExit(struct hdev_ctrl *hdev_ctrl);
VOID HdevCfgShow(struct hdev_ctrl *hdev_ctrl);
VOID HdevObjShow(struct mtk_mac_bss *obj);
VOID HdevHwResourceExit(struct hdev_ctrl *ctrl);
VOID HdevHwResourceInit(struct hdev_ctrl *ctrl);

#endif /*__HDEV_BASIC_H*/
