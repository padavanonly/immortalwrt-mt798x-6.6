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
	cmm_chip.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_CHIP_H__
#define __CMM_CHIP_H__

union _EXT_CMD_EFUSE_BUFFER_MODE_T;
struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _RTMP_REG_PAIR;
struct _BANK_RF_REG_PAIR;
struct _R_M_W_REG;
struct _RF_R_M_W_REG;
struct _CR_REG;
struct MT_TX_PWR_CAP;
struct _EXT_CMD_CHAN_SWITCH_T;
struct wifi_dev;
struct hdev_ctrl;
struct _RX_BLK;

enum ASIC_CAP {
	fASIC_CAP_MCU_SDO_ONLY = (1 << 0),
	fASIC_CAP_CSO = (1 << 1),
	fASIC_CAP_TSO = (1 << 2),
	fASIC_CAP_MCS_LUT = (1 << 3),
	fASIC_CAP_PMF_ENC = (1 << 4),
	fASIC_CAP_DBDC = (1 << 5),
	fASIC_CAP_TX_HDR_TRANS = (1 << 6),
	fASIC_CAP_RX_HDR_TRANS = (1 << 7),
	fASIC_CAP_HW_DAMSDU = (1 << 8),
	fASIC_CAP_RX_DMA_SCATTER =  (1 << 9),
	fASIC_CAP_MCU_OFFLOAD =  (1 << 10),
	fASIC_CAP_CT = (1 << 11),
	fASIC_CAP_HW_TX_AMSDU = (1 << 12),
	fASIC_CAP_WHNAT = (1 << 13),
	fASIC_CAP_RDG = (1 << 14),
	fASIC_CAP_DLY_INT_LUMPED = (1 << 15),
	fASIC_CAP_WMM_PKTDETECT_OFFLOAD = (1 << 16),
	fASIC_CAP_PCIE_ASPM_DYM_CTRL = (1 << 17),
	fASIC_CAP_TWT = (1 << 18),
	fASIC_CAP_DLY_INT_PER_RING = (1 << 19),
	fASIC_CAP_MGMT_TIMER_TASK = (1 << 20),
	fASIC_CAP_TXCMD = (1 << 21),
	fASIC_CAP_TWO_PCIE = (1 << 22),
	fASIC_CAP_SEPARATE_DBDC = (1 << 23),
	fASIC_CAP_FW_RESTART_POLLING_MODE = (1 << 24),
	fASIC_CAP_ADV_SECURITY = (1 << 25),
	fASIC_CAP_ADDBA_HW_SSN = (1 << 26),
	fASIC_CAP_BA_OFFLOAD = (1 << 27),
	fASIC_CAP_DUAL_PCIE_ONE_PROBE = (1 << 28),
	fASIC_CAP_TX_FREE_NOTIFY_V4 = (1 << 29),
	fASIC_CAP_HW_RRO = (1 << 30),
	fASIC_CAP_HW_ADMISSION_CTRL = (1 << 31),
};

enum MAC_CAP {
	fMAC_CAP_BA_256 = (1 << 0),
	fMAC_CAP_BA_512 = (1 << 1),
	fMAC_CAP_BA_1024 = (1 << 2),
};

enum PHY_CAP {
	fPHY_CAP_24G = (1 << 0),
	fPHY_CAP_5G = (1 << 1),
	fPHY_CAP_HT = (1 << 2),
	fPHY_CAP_VHT = (1 << 3),
	fPHY_CAP_HE = (1 << 4),
	fPHY_CAP_TXBF = (1 << 5),
	fPHY_CAP_LDPC = (1 << 6),
	fPHY_CAP_DL_MUMIMO = (1 << 7),
	fPHY_CAP_BW40 = (1 << 8),
	fPHY_CAP_BW80 = (1 << 9),
	fPHY_CAP_BW160NC = (1 << 10),
	fPHY_CAP_BW160C = (1 << 11),
	fPHY_CAP_BW20_242TONE = (1 << 12),
	fPHY_CAP_TX_DOPPLER = (1 << 13),
	fPHY_CAP_RX_DOPPLER = (1 << 14),
	fPHY_CAP_HE_ER_SU = (1 << 15),
	fPHY_CAP_HE_UL_MUOFDMA = (1 << 16),
	fPHY_CAP_HE_PPE_EXIST = (1 << 17),
	fPHY_CAP_HE_SR = (1 << 18),
	fPHY_CAP_HE_UORA = (1 << 19),
	fPHY_CAP_UL_MUMIMO = (1 << 20),
	fPHY_CAP_HE_DL_MUOFDMA = (1 << 21),
	fPHY_CAP_DUALPHY = (1 << 22),
	fPHY_CAP_6G = (1 << 23),
	fPHY_CAP_BW160C_STD = (1 << 24),
	fPHY_CAP_EHT = (1 << 25),
	fPHY_CAP_EHT_PPE_EXIST = (1 << 26),
	fPHY_CAP_BW320 = (1 << 27),
	fPHY_CAP_EHT_DUP = (1 << 28),
};

enum HIF_TYPE {
	HIF_RTMP = 0x0,
	HIF_RLT = 0x1,
	HIF_MT = 0x2,
	HIF_MAX = HIF_MT,
};

enum MAC_TYPE {
	MAC_RTMP = 0x0,
	MAC_MT = 0x1,
};

enum RF_TYPE {
	RF_RT,
	RF_RLT,
	RF_MT76x2,
	RF_MT,
};

enum BBP_TYPE {
	BBP_RTMP = 0x0,
	BBP_RLT = 0x1,
	BBP_MT = 0x2,
};

enum CHIP_EXTRA_ACTION {
	CHIP_EXTRA_ACTION_IDLE_DETECT = (1 << 0),
	CHIP_EXTRA_ACTION_TX_ENABLE_AGG_CAP = (1 << 1),
	CHIP_EXTRA_ACTION_RX_BA_WSIZE_MLO = (1 << 2),
	CHIP_EXTRA_ACTION_SW_PN = (1 << 3),
	CHIP_EXTRA_ACTION_EXTCHA_ADJUST = (1 << 4),
	CHIP_EXTRA_ACTION_PROFILE_CORRECT = (1 << 5),
	CHIP_EXTRA_ACTION_RX_RESTORE_OLDPKT = (1 << 6),
	CHIP_EXTRA_ACTION_ADDBA_SSN = (1 << 8),
	CHIP_EXTRA_ACTION_ADDBA_WAIT_DELETE = (1 << 9),
	CHIP_EXTRA_ACTION_ADDBA_WAIT_ADD = (1 << 10),
	CHIP_EXTRA_ACTION_ADDBA_WAIT_INIT = (1 << 11),
	CHIP_EXTRA_ACTION_ICV_ERROR = (1 << 12),
	CHIP_EXTRA_ACTION_PN_CHK = (1 << 13),
	CHIP_EXTRA_ACTION_ICV_ERR_INIT = (1 << 14),
};
#define OLDPKT_CHECK_CNT 16

#define UMAC_FID_FAULT	0xFFF
#define UMAC_FID_1FFF_FAULT	0x1FFF

#define PHY_CAP_2G(_x)		(((_x) & fPHY_CAP_24G) == fPHY_CAP_24G)
#define PHY_CAP_5G(_x)		(((_x) & fPHY_CAP_5G) == fPHY_CAP_5G)
#define PHY_CAP_6G(_x)		(((_x) & fPHY_CAP_6G) == fPHY_CAP_6G)
#define PHY_CAP_N(_x)		(((_x) & fPHY_CAP_HT) == fPHY_CAP_HT)
#define PHY_CAP_AC(_x)		(((_x) & fPHY_CAP_VHT) == fPHY_CAP_VHT)
#define PHY_CAP_BE(_x)		(((_x) & fPHY_CAP_EHT) == fPHY_CAP_EHT)
#define GET_DATA_TX_RING_SIZE(_chipCap)	((_chipCap)->tx_ring_size)

enum RRO_CAP {
	RRO_PRE_LINK = 1 << 0,
	RRO_BUF_PG = 1 << 1,
};

enum EFUSE_TYPE {
	EFUSE_MT,
	EFUSE_MAX,
};

enum TXD_TYPE {
	TXD_V1,
	TXD_V2,
};

enum TX_DELAY_TYPE {
	TX_DELAY_SW_MODE = 0,
	TX_DELAY_HW_MODE = 1,
};

/* TXD.fixed_rate_idx :
 * The index to indicate the rate info
 * in LWTBL[fixed_rate_idx].rate valid when FR=1
 */
enum TXD_V2_FR_TBL {
	FR_CCK_1M = 0,
	FR_OFDM_6M = 1,
	FR_OFDM_24M = 2,
	FR_HTMM_MCS0 = 3,
	FR_VHT_MCS0 = 4,
	FR_HE_MCS0 = 5,
	FR_MLKR_MCS0 = 6,
	FR_CCK_SPE0x18_1M = 7,
	FR_OFDM_SPE0x18_6M = 8,
	FR_MLR_SPE0x18_MCS0 = 9,
	/* Driver FR Table */
	FR_OFDM_SPE0x18_6M_BCN0 = 25,
	FR_OFDM_SPE0x18_6M_BCN1 = 27,
	FR_OFDM_SPE0x18_6M_BCN2 = 29,
	FR_CCK_11M = 31,			/* UNI_CMD_FR_TABLE_CCK_11M_IDX */
	FR_CCK_SPE0x18_2M = 33,		/* UNI_CMD_FR_TABLE_CCK_SPE_2M_IDX */
	FR_CCK_SPE0x18_5_5M = 35,	/* UNI_CMD_FR_TABLE_CCK_SPE_5_5M_IDX */
	FR_CCK_SPE0x18_11M = 37,	/* UNI_CMD_FR_TABLE_CCK_SPE_11M_IDX */
	FR_CCKS_SPE0x18_2M = 39,	/* UNI_CMD_FR_TABLE_CCKS_SPE_2M_IDX */
	FR_CCKS_SPE0x18_5_5M = 41,	/* UNI_CMD_FR_TABLE_CCKS_SPE_5_5M_IDX */
	FR_CCKS_SPE0x18_11M = 43,	/* UNI_CMD_FR_TABLE_CCKS_SPE_11M_IDX */
	FR_OFDM_SPE0x18_9M = 45,	/* UNI_CMD_FR_TABLE_OFDM_SPE_9M_IDX */
	FR_OFDM_SPE0x18_12M = 47,	/* UNI_CMD_FR_TABLE_OFDM_SPE_12M_IDX */
	FR_OFDM_SPE0x18_18M = 49,	/* UNI_CMD_FR_TABLE_OFDM_SPE_18M_IDX */
	FR_OFDM_SPE0x18_24M = 51,	/* UNI_CMD_FR_TABLE_OFDM_SPE_24M_IDX */
	FR_OFDM_SPE0x18_36M = 53,	/* UNI_CMD_FR_TABLE_OFDM_SPE_36M_IDX */
	FR_OFDM_SPE0x18_48_5M = 55,	/* UNI_CMD_FR_TABLE_OFDM_SPE_48_5M_IDX */
	FR_OFDM_SPE0x18_54M = 57,	/* UNI_CMD_FR_TABLE_OFDM_SPE_54M_IDX */
	FR_VHT_1SS_MCS7 = 59,	/* UNI_CMD_FR_TABLE_VHT_1SS_MCS7_IDX */
	FR_HE_SPE0x18_MCS0 = 61,	/* UNI_CMD_FR_TABLE_HE_SPE_MCS0_IDX */
};

#define MBSSID_MODE0	0
#define MBSSID_MODE1	1	/* Enhance NEW MBSSID MODE mapping to mode 0 */
#ifdef ENHANCE_NEW_MBSSID_MODE
#define MBSSID_MODE2	2	/* Enhance NEW MBSSID MODE mapping to mode 1 */
#define MBSSID_MODE3	3	/* Enhance NEW MBSSID MODE mapping to mode 2 */
#define MBSSID_MODE4	4	/* Enhance NEW MBSSID MODE mapping to mode 3 */
#define MBSSID_MODE5	5	/* Enhance NEW MBSSID MODE mapping to mode 4 */
#define MBSSID_MODE6	6	/* Enhance NEW MBSSID MODE mapping to mode 5 */
#endif /* ENHANCE_NEW_MBSSID_MODE */

enum APPS_MODE {
	APPS_MODE0 = 0x0,	/* MT7603, host handle APPS */
	APPS_MODE1 = 0x1,	/* MT7637 */
	APPS_MODE2 = 0x2,	/* MT7615, FW handle APPS */
	APPS_MODEMAX = 0x3,
};

#define ALL_DMA 0xff

//Testmode usage
//Can define any number bigger than "TEST_CHIP_BAND_NUM"
#define CHIP_TEST_MAX_BAND_NUM 3

struct _chip_testmode_chip_info {
	uint8_t u1band_num;
	uint8_t au1used_band[CHIP_TEST_MAX_BAND_NUM];
	uint8_t band_used_num;
	uint32_t u4ChipId;
	uint8_t u1testmode_sup_mlo;
};


#ifdef MT_MAC
/*
	these functions is common setting and could be used by sMAC or dMAC.
	move to here to common use.
*/
VOID MtAsicSetWdevIfAddr(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, INT opmode);
VOID MtAsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /*MT_MAC*/

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID mt_asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
VOID mt_asic_twt_agrt_update(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para);
VOID mt_asic_twt_agrt_update_by_cmd_task(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para);
VOID mt_asic_twt_agrt_alloc(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para, struct TWT_AGRT_MGMT_T *agrt_mgmt);
VOID mt_asic_twt_agrt_free(struct wifi_dev *wdev, struct TWT_AGRT_PARA_T *agrt_para);
VOID mt_asic_twt_mgmt_frame_offload(struct wifi_dev *wdev, UINT8 bssinfo_idx, UINT16 wcid, UINT8 token,
	UINT8 mlo_en, UINT16 mld_sta_idx, UINT8 action_linkid, UINT16 all_linkid_bitmap, UINT8 *buf, UINT32 len);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

typedef struct _RTMP_CHIP_OP {
	int (*sys_onoff)(struct _RTMP_ADAPTER *pAd, BOOLEAN on, BOOLEAN reser);

	/*  Calibration access related callback functions */
	int (*eeinit)(struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*eeread)(struct _RTMP_ADAPTER *pAd, UINT32 offset, UINT16 *pValue);
	int (*eewrite)(struct _RTMP_ADAPTER *pAd, UINT32 offset, USHORT value);
	BOOLEAN (*eeread_range)(struct _RTMP_ADAPTER *pAd, UINT32 start, UINT32 length, UCHAR *pbuf);
	int (*eewrite_range)(struct _RTMP_ADAPTER *pAd, UINT32 start, UINT32 length, UCHAR *pbuf);
	int (*ee_gen_cmd)(struct _RTMP_ADAPTER *pAd, VOID *cmd, UINT8 cmd_seq, UINT8 cmd_total);

	/* ITxBf calibration */
	int (*fITxBfDividerCalibration)(struct _RTMP_ADAPTER *pAd, int calFunction, int calMethod, UCHAR *divPhase);
	void (*fITxBfLNAPhaseCompensate)(struct _RTMP_ADAPTER *pAd);
	int (*fITxBfCal)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	int (*fITxBfLNACalibration)(struct _RTMP_ADAPTER *pAd, int calFunction, int calMethod, BOOLEAN gBand);

	void (*AsicBbpInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicMacInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicReverseRfFromSleepMode)(struct _RTMP_ADAPTER *pAd, BOOLEAN FlgIsInitState);
	void (*AsicHaltAction)(struct _RTMP_ADAPTER *pAd);

	/* Power save */
#ifdef GREENAP_SUPPORT
	VOID (*EnableAPMIMOPS)(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
	VOID (*DisableAPMIMOPS)(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
#endif /* GREENAP_SUPPORT */
#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	VOID (*pcie_aspm_dym_ctrl)(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	VOID (*twt_agrt_update)(struct _RTMP_ADAPTER *ad, struct TWT_AGRT_PARA_T *agrt_para);
	VOID (*twt_agrt_mgmt)(struct _RTMP_ADAPTER *ad, struct TWT_AGRT_PARA_T *agrt_para, struct TWT_AGRT_MGMT_T *agrt_mgmt);
	void (*mgmt_frame_offload)(struct _RTMP_ADAPTER *ad, UINT8 bssinfo_idx, UINT16 wcid, UINT8 token,
		UINT8 mlo_en, UINT16 mld_sta_idx, UINT8 action_linkid, UINT16 all_linkid_bitmap, UINT8 *buf, UINT32 len);
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	/* AGC */
	VOID (*BbpInitFromEEPROM)(struct _RTMP_ADAPTER *pAd);
	VOID (*ChipSwitchChannel)(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);
	VOID (*ChipCheckPrek)(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

#ifdef NEW_SET_RX_STREAM
	INT (*ChipSetRxStream)(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
#endif
	INT32 (*get_bin_image_file)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *path, BOOLEAN fgBinMode); /* add BOOLEAN fgBinMode : to seperate the bin & flash mode's default bin PATH if needed */
	INT32 (*get_prek_image_file)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *path);

	VOID (*AsicTxAlcGetAutoAgcOffset)(
		IN struct _RTMP_ADAPTER	*pAd,
		IN PCHAR				pDeltaPwr,
		IN PCHAR				pTotalDeltaPwr,
		IN PCHAR				pAgcCompensate,
		IN PCHAR				pDeltaPowerByBbpR1,
		IN UCHAR				Channel);

	VOID (*AsicGetTxPowerOffset)(struct _RTMP_ADAPTER *pAd, ULONG *TxPwr);
	VOID (*AsicExtraPowerOverMAC)(struct _RTMP_ADAPTER *pAd);

	/* Antenna */
	VOID (*AsicAntennaDefaultReset)(struct _RTMP_ADAPTER *pAd, union _EEPROM_ANTENNA_STRUC *pAntenna);
	VOID (*SetRxAnt)(struct _RTMP_ADAPTER *pAd, UCHAR Ant);

	/* EEPROM */
	VOID (*NICInitAsicFromEEPROM)(IN struct _RTMP_ADAPTER *pAd);
	UINT32 (*GetBandSelectFromEEPROM)(IN struct _RTMP_ADAPTER *pAd);

	/* Temperature Compensation */
	VOID (*InitTemperCompensation)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*TemperCompensation)(IN struct _RTMP_ADAPTER *pAd);

	/* high power tuning */
	VOID (*HighPowerTuning)(struct _RTMP_ADAPTER *pAd, struct _RSSI_SAMPLE *pRssi);

	/* Others */
	VOID (*NetDevNickNameInit)(IN struct _RTMP_ADAPTER *pAd);
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN (*is_cal_free_ic)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*cal_free_data_get)(IN struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*check_is_cal_free_merge)(IN struct _RTMP_ADAPTER *pAd);
#endif /* CAL_FREE_IC_SUPPORT */

	UINT32 (*get_sku_tbl_idx)(IN struct _RTMP_ADAPTER *pAd, OUT UINT8 *sku_tbl_idx);
	BOOLEAN (*check_RF_lock_down)(IN struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*write_RF_lock_parameter)(IN struct _RTMP_ADAPTER *pAd, IN USHORT offset);
	BOOLEAN (*merge_RF_lock_parameter)(IN struct _RTMP_ADAPTER *pAd);
	UCHAR (*Read_Effuse_parameter)(IN struct _RTMP_ADAPTER *pAd, IN USHORT offset);
	BOOLEAN (*Config_Effuse_Country)(IN struct _RTMP_ADAPTER *pAd);

	/* The chip specific function list */
	VOID (*AsicResetBbpAgent)(IN struct _RTMP_ADAPTER *pAd);

	VOID (*CckMrcStatusCtrl)(struct _RTMP_ADAPTER *pAd);
	VOID (*RadarGLRTCompensate)(struct _RTMP_ADAPTER *pAd);
	VOID (*SecondCCADetection)(struct _RTMP_ADAPTER *pAd);

	/* MCU */
	void (*MCUCtrlInit)(struct _RTMP_ADAPTER *ad);
	void (*MCUCtrlExit)(struct _RTMP_ADAPTER *ad);
	VOID (*FwInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*FwExit)(struct _RTMP_ADAPTER *pAd);
	int (*DisableTxRx)(struct _RTMP_ADAPTER *ad, UCHAR Level);
	void (*AsicRadioOn)(struct _RTMP_ADAPTER *ad, UCHAR Stage);
	void (*AsicRadioOff)(struct _RTMP_ADAPTER *ad, UINT8 Stage);
#ifdef MICROWAVE_OVEN_SUPPORT
	VOID (*AsicMeasureFalseCCA)(IN struct _RTMP_ADAPTER *pAd);

	VOID (*AsicMitigateMicrowave)(IN struct _RTMP_ADAPTER *pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	VOID (*AsicWOWEnable)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*AsicWOWDisable)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*AsicWOWInit)(struct _RTMP_ADAPTER *ad);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

	void (*show_pwr_info)(struct _RTMP_ADAPTER *ad);
	void (*cal_test)(struct _RTMP_ADAPTER *ad, UINT32 type);
	void (*bufferModeCmdFill)(struct _RTMP_ADAPTER *ad, union _EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd, UINT16 ctrl_msg);
	void (*keep_efuse_field_only)(struct _RTMP_ADAPTER *ad, UCHAR *buffer);
	UINT32 (*get_efuse_free_blk_bnum)(struct _RTMP_ADAPTER *ad, UINT8 blk_section);
	UINT32 (*get_efuse_free_blk_bnum_v2)(struct _RTMP_ADAPTER *ad, struct _EFUSE_FREE_BLOCK_INFO *efuse_info);
	INT32 (*MtCmdTx)(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);

	INT32 (*HeraStbcPriorityCtrl)(struct _RTMP_ADAPTER *ad, PUINT8 pucData);
#ifdef TXBF_SUPPORT
	VOID (*TxBFInit)(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF);
	BOOLEAN (*ClientSupportsETxBF)(struct _RTMP_ADAPTER *ad, struct _HT_BF_CAP *pTxBFCap);
	VOID (*setETxBFCap)(struct _RTMP_ADAPTER *ad, struct _TXBF_STATUS_INFO  *pTxBfInfo);
#ifdef MT_MAC
#ifdef VHT_TXBF_SUPPORT
	BOOLEAN (*ClientSupportsVhtETxBF)(struct _RTMP_ADAPTER *ad, struct _VHT_CAP_INFO *pTxBFCap);
	VOID (*setVHTETxBFCap)(struct _RTMP_ADAPTER *ad, struct _TXBF_STATUS_INFO  *pTxBfInfo);
#endif /* VHT_TXBF_SUPPORT */
#ifdef HE_TXBF_SUPPORT
	VOID (*get_he_etxbf_cap)(struct wifi_dev *wdev, struct _TXBF_STATUS_INFO *txbf_status);
#endif /* HE_TXBF_SUPPORT */
#ifdef DOT11_EHT_BE
	VOID(*get_eht_etxbf_cap)(struct wifi_dev *wdev, struct _TXBF_STATUS_INFO *txbf_status);
#endif /* DOT11_EHT_BE */
	VOID (*iBFPhaseCalInit)(struct _RTMP_ADAPTER *ad);
	VOID (*iBFPhaseFreeMem)(struct _RTMP_ADAPTER *ad);
	VOID (*iBFPhaseCalE2PUpdate)(struct _RTMP_ADAPTER *ad, UCHAR ucGroup, BOOLEAN fgSX2, UCHAR ucUpdateAllTye);
	VOID (*iBFPhaseCalReport)(struct _RTMP_ADAPTER *ad, UCHAR ucGroupL_M_H, UCHAR ucGroup, BOOLEAN fgSX2, UCHAR ucStatus, UCHAR ucPhaseCalType, PUCHAR pBuf);
	VOID (*iBFPhaseComp)(struct _RTMP_ADAPTER *ad, UCHAR ucGroup, PCHAR pCmdBuf);
	VOID (*dump_pfmu_tag)(struct _RTMP_ADAPTER *pAd, BOOLEAN fgBFer, PUCHAR pBuf);
	VOID (*iBfCaleBfPfmuMemAlloc)(struct _RTMP_ADAPTER *ad, PCHAR pPfmuMemRow, PCHAR pPfmuMemCol);
	INT (*set_txbf_pfmu_tag)(struct hdev_ctrl *ctrl, enum txbf_pfmu_tag idx, UINT32 val);
	INT (*write_txbf_pfmu_tag)(struct hdev_ctrl *ctrl, UINT8 pf_idx);
	VOID (*iBfCaliBfPfmuMemAlloc)(struct _RTMP_ADAPTER *ad, PCHAR pPfmuMemRow, PCHAR pPfmuMemCol);
	VOID (*dump_pfmu_data)(struct _RTMP_ADAPTER *pAd, USHORT subCarrIdx, PUCHAR pBuf);
	INT (*write_txbf_profile_data)(struct _RTMP_ADAPTER *pAd, PUSHORT Input);
	INT (*set_txbf_angle)(struct hdev_ctrl *ctrl, UINT32 bfer, UINT32 nc, UINT32 *angle);
	INT (*set_txbf_dsnr)(struct hdev_ctrl *ctrl, UINT32 bfer, UINT32 *dsnr);
	INT (*write_txbf_pfmu_data)(struct hdev_ctrl *ctrl, UINT8 pf_id, UINT16 subc_idx, BOOLEAN bfer);
	INT (*set_manual_assoc)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT (*set_cmm_starec)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
	INT32 (*ICapStart)(struct _RTMP_ADAPTER *pAd, UINT8 *pData);
	INT32 (*ICapStatus)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ICapCmdUnSolicitRawDataProc)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ICapCmdSolicitRawDataProc)(struct _RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pDataLen, UINT32 IQ_Type, UINT32 WF_Num);
	INT32 (*ICapGetIQData)(struct _RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pDataLen, UINT32 IQ_Type, UINT32 WF_Num);
	VOID  (*ICapEventRawDataHandler)(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
	INT32 (*SpectrumStart)(struct _RTMP_ADAPTER *pAd, UINT8 *pData);
	INT32 (*SpectrumStatus)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SpectrumCmdRawDataProc)(struct _RTMP_ADAPTER *pAd);
	VOID  (*SpectrumEventRawDataHandler)(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef PHY_ICS_SUPPORT
	INT32 (*PhyIcsStart)(struct _RTMP_ADAPTER *pAd, UINT8 *pData);
	INT32 (*PhyIcsEventEnable)(struct _RTMP_ADAPTER *pAd, UINT8 *pData);
	VOID  (*PhyIcsEventRawDataHandler)(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* PHY_ICS_SUPPORT */

	INT32 (*hif_io_read32)(void *cookie, UINT32 addr, UINT32 *value);
	INT32 (*hif_io_write32)(void *cookie, UINT32 addr, UINT32 value);
	VOID (*hif_io_remap_read32)(void *cookie, UINT32 addr, UINT32 *value);
	VOID (*hif_io_remap_write32)(void *cookie, UINT32 addr, UINT32 value);
	VOID (*heart_beat_check)(struct _RTMP_ADAPTER *ad);
	INT (*chk_hif_default_cr_setting)(struct _RTMP_ADAPTER *pAd);
	INT (*chk_top_default_cr_setting)(struct _RTMP_ADAPTER *pAd);
	INT (*set_ampdu_wtbl)(RTMP_ADAPTER *pAd, UINT32 band_idx);
#ifdef HOST_RESUME_DONE_ACK_SUPPORT
	void (*HostResumeDoneAck)(struct _RTMP_ADAPTER *pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
	INT32 (*read_chl_pwr)(struct _RTMP_ADAPTER *pAd);
	VOID (*parse_RXV_packet)(struct _RTMP_ADAPTER *pAd, UINT32 Type, struct _RX_BLK *RxBlk, UCHAR *Data);
	UINT32 (*rx_stat_update)(struct _RTMP_ADAPTER *pAd, UCHAR *Data);
	UINT32 (*rxv_raw_data_show)(struct _RTMP_ADAPTER *pAd, UINT8 band_idx);
	UINT32 (*rxv_stat_reset)(struct _RTMP_ADAPTER *pAd, UINT8 band_idx);
	UINT32 (*rxv_info_show)(struct _RTMP_ADAPTER *pAd, UINT8 band_idx);
	UINT32 (*rxv_packet_parse)(struct _RTMP_ADAPTER *pAd, VOID *Data);
	UINT32 (*rxv_entry_parse)(struct _RTMP_ADAPTER *pAd, VOID *Data);
	UINT32 (*rxv_get_byte_cnt)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			UINT32 *byte_cnt);
	UINT32 (*rxv_get_content)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			PVOID *content);
	UINT32 (*rxv_cap_init)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*rxv_dump_start)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*rxv_dump_stop)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*rxv_dump_buf_alloc)(struct _RTMP_ADAPTER *pAd, UINT8 type_mask);
	UINT32 (*rxv_dump_buf_clear)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*rxv_dump_show_list)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*rxv_dump_show_rpt)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*rxv_dump_rxv_content_compose)(
		struct _RTMP_ADAPTER *pAd,
		UINT8 entry_idx,
		VOID *rxv_content,
		UINT32 *len);
	UINT32 (*rxv_content_len)(
		struct _RTMP_ADAPTER *pAd,
		UINT8 type_mask,
		UINT8 rxv_sta_cnt,
		UINT16 *len);
	UINT32 (*get_rx_stat)(struct _RTMP_ADAPTER *pAd,
			UCHAR band_idx,
			P_TESTMODE_STATISTIC_INFO prtest_mode_stat_info);
	UINT32 (*get_wf_path_comb)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			BOOLEAN dbdc_mode_en,
			UINT8 *path,
			UINT8 *path_len);
	UINT32 (*get_rx_stat_band)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			UINT8 blk_idx,
			TEST_RX_STAT_BAND_INFO * rx_band);
	UINT32 (*get_rx_stat_path)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			UINT8 blk_idx,
			TEST_RX_STAT_PATH_INFO * rx_path);
	UINT32 (*get_rx_stat_user)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			UINT8 blk_idx,
			TEST_RX_STAT_USER_INFO * rx_user);
	UINT32 (*get_rx_stat_comm)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			UINT8 blk_idx,
			TEST_RX_STAT_COMM_INFO * rx_comm);
	UINT32 (*get_rx_stat_all)(struct _RTMP_ADAPTER *pAd,
			UINT8 band_idx,
			UINT8 blk_idx,
			struct _TEST_RX_STAT_ALL_INFO *rx_all);
#ifdef CONFIG_WLAN_SERVICE
	INT32 (*get_chip_capability)(struct _RTMP_ADAPTER *pAd,
			struct test_capability *chip_cap);
#endif
	UINT32 (*ra_init)(struct _RTMP_ADAPTER *pAd,
			struct _MAC_TABLE_ENTRY *pEntry);
	INT32 (*tssi_set) (struct _RTMP_ADAPTER *ad, UCHAR *efuse);
	INT32 (*pa_lna_set) (struct _RTMP_ADAPTER *ad, UCHAR *efuse);
#ifdef CONNAC_EFUSE_FORMAT_SUPPORT
	VOID (*eeprom_extract) (struct _RTMP_ADAPTER *ad, VOID *eeprom);
#endif /* CONNAC_EFUSE_FORMAT_SUPPORT */
#ifdef WIFI_RAM_EMI_SUPPORT
	INT32 (*parse_emi_phy_addr)(struct _RTMP_ADAPTER *pAd);
	INT32 (*fw_ram_emi_dl)(struct _RTMP_ADAPTER *pAd);
#endif /* WIFI_RAM_EMI_SUPPORT */
#ifdef CONFIG_TX_DELAY
	VOID (*tx_deley_parm_init)(UCHAR tx_delay_mode, struct tx_delay_control *tx_delay_ctl);
#endif
#ifdef BACKGROUND_SCAN_SUPPORT
	VOID (*set_off_ch_scan)(struct _RTMP_ADAPTER *pAd, UCHAR reason, UCHAR bgnd_scan_type);
	VOID (*bgnd_scan_cr_init)(struct _RTMP_ADAPTER *pAd);
#endif
	UINT16 (*get_tid_sn)(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR tid);
	INT32 (*mactbl_wtbl_adjust)(struct _RTMP_ADAPTER *pAd);
	VOID (*hif_pci_slave_chip_defer_create)(VOID *hdev_ctrl);
	INT32 (*fill_key_install_cmd)(struct _ASIC_SEC_INFO *asic_sec_info, UCHAR is_sta_rec_update, VOID **wtbl_security_key, UINT32 *cmd_len);
#ifdef WIFI_UNIFIED_COMMAND
	INT32 (*fill_key_install_uni_cmd)(VOID *hdev_ctrl, struct _ASIC_SEC_INFO *asic_sec_info, UCHAR is_sta_rec_update, VOID *wtbl_security_key);
	INT32 (*fill_key_install_uni_cmd_dynsize_check)(struct _ASIC_SEC_INFO *asic_sec_info, UINT32 *cmd_len);
#endif /* WIFI_UNIFIED_COMMAND */
#ifdef ERR_RECOVERY
	VOID (*dump_ser_stat)(struct _RTMP_ADAPTER *pAd, UINT8 dump_lvl);
#ifdef MT7915_E1_WORKAROUND
#ifdef WFDMA_WED_COMPATIBLE
	VOID (*sw_int_polling)(struct _RTMP_ADAPTER *pAd);
#endif
#endif
#endif
#ifdef CFG_SUPPORT_FALCON_MURU
	VOID (*check_muru_glo)(struct _RTMP_ADAPTER *pAd, VOID *pData);
	VOID (*show_muru_local_data)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	VOID (*show_muru_tx_info)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	VOID (*show_muru_shared_data)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	VOID (*show_muru_mancfg_data)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	VOID (*set_muru_data)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	VOID (*show_muru_stacap_info)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	VOID (*show_muru_txc_tx_stats)(struct _RTMP_ADAPTER *pAd, VOID *pData);
#endif
#if (defined(CFG_SUPPORT_FALCON_MURU) || defined(CFG_SUPPORT_MU_MIMO))
	VOID (*show_mumimo_group_entry_tbl)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#if (defined(CFG_SUPPORT_MU_MIMO_RA) || defined(CFG_SUPPORT_FALCON_MURU))
	VOID (*show_mumimo_algorithm_monitor)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT32 (*set_mumimo_fixed_rate)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT32 (*set_mumimo_fixed_group_rate)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT32 (*set_mumimo_force_mu_enable)(struct _RTMP_ADAPTER *pAd, BOOLEAN fgForceMu);
#endif
#ifdef SMART_CARRIER_SENSE_SUPPORT
	VOID (*show_scs_info)(struct _RTMP_ADAPTER *pAd);
#endif /*SMART_CARRIER_SENSE_SUPPORT*/
	INT32 (*show_hwcfg_info)(struct _RTMP_ADAPTER *pAd);
	INT32 (*get_RxFELossComp_data)(struct _RTMP_ADAPTER *pAd, VOID *pData);
#if defined(CONFIG_ATE)
	INT32 (*backup_reg_before_ate)(struct _RTMP_ADAPTER *ad);
	INT32 (*restore_reg_after_ate)(struct _RTMP_ADAPTER *ad);
	INT32 (*restore_reg_during_ate)(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
	INT32 (*set_ifs)(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
	INT32 (*set_ba_limit)(struct _RTMP_ADAPTER *ad, UINT8 wmm_idx, UINT8 limit, UINT8 band_idx);
	INT32 (*pause_ac_queue)(struct _RTMP_ADAPTER *ad, UINT8 ac_idx);
	INT32 (*test_mode_dnlk)(struct _RTMP_ADAPTER *ad, UINT8 bnda_idx);
	INT32 (*ate_group_prek)(struct _RTMP_ADAPTER *ad, UINT8 op);
	INT32 (*ate_dpd_prek)(struct _RTMP_ADAPTER *ad, UINT8 op);
	VOID (*get_testmode_chip_info)(struct _RTMP_ADAPTER *pAd, struct _chip_testmode_chip_info *ptestmode_chip_info);
	VOID (*testmode_disable_mlo)(struct _RTMP_ADAPTER *pAd);
#endif	/* CONFIG_ATE */
	INT32 (*sta_per_get)(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, PUINT8 u1PER);
	VOID (*rssi_get)(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *RssiSet);
	VOID (*cninfo_get)(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT16 *pCnInfo);
	/* TX Power Info */
	VOID (*txpower_show_info)(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
	VOID (*update_mib_bucket)(struct _RTMP_ADAPTER *pAd);
#ifdef ZERO_PKT_LOSS_SUPPORT
	VOID (*read_channel_stat_registers)(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, void *ChStat);
#endif
	VOID (*ctrl_rxv_group)(struct _RTMP_ADAPTER *pAd, UINT8 band_idx, UINT8 group, BOOLEAN enable);
	VOID (*set_mgmt_pkt_txpwr_prctg)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT8 prctg);
	/*wps led*/
	VOID (*wps_led_init)(struct _RTMP_ADAPTER *pAd);
	UCHAR (*wps_led_control)(struct _RTMP_ADAPTER *pAd, UCHAR flag);
	VOID (*update_chip_cap)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*get_sub_chipid)(struct _RTMP_ADAPTER *pAd, UINT32 *sub_chipid);
	VOID (*post_config_hif)(struct _RTMP_ADAPTER *pAd);
	UINT32 (*get_rid_value)(VOID);
#ifdef WF_RESET_SUPPORT
	void (*do_wifi_reset)(struct physical_device *device);
#endif /* WF_RESET_SUPPORT */

#ifdef ACK_CTS_TIMEOUT_SUPPORT
	INT (*set_ack_timeout_mode_byband)(struct _RTMP_ADAPTER *pAd, UINT32 timeout, UINT32 bandidx, UINT8 ackmode);
	INT32 (*get_ack_timeout_mode_byband)(struct _RTMP_ADAPTER *pAd, UINT32 *ptimeout, UINT32 bandidx, UINT8 ackmode);
#endif /* ACK_CTS_TIMEOUT_SUPPORT */
	VOID (*update_ampdu_raw_counters)(RTMP_ADAPTER *pAd, UCHAR BandIdx);
	INT (*init_hw_ring_setting)(struct _RTMP_ADAPTER *ad);
	UINT32 (*get_sku_decision)(struct _RTMP_ADAPTER *ad);
	VOID (*chip_extra_action)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev,
		UCHAR *peer_mac, enum CHIP_EXTRA_ACTION, UCHAR *data, ULONG *data_len);
	VOID (*set_rro_timeout)(struct _RTMP_ADAPTER *ad);
	VOID (*set_rro_rss_map)(struct _RTMP_ADAPTER *ad);
	BOOLEAN (*set_spe_fr_table)(
		struct _RTMP_ADAPTER *ad, UINT8 band_idx, UINT8 rate_idx, VOID *handle);
	UINT8 (*determine_mlo_type)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
	BOOLEAN (*determine_mlo_allow)(struct _RTMP_ADAPTER *ad, struct _vendor_ie_cap *vendor_ie);
	UINT8 (*determine_hmlo_support)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
} RTMP_CHIP_OP;

typedef struct _RTMP_CHIP_DBG {
	INT32 (*dump_ps_table)(struct hdev_ctrl *ctrl, UINT32 ent_type, BOOLEAN bReptCli);
	INT32 (*dump_mib_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_tmac_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_agg_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_arb_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_dmasch_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_pse_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_pse_data)(struct hdev_ctrl *ctrl, UINT32 StartFID, UINT32 FrameNums);
	INT32 (*show_ple_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_ple_info_by_idx)(struct hdev_ctrl *ctrl, UINT16 wtbl_idx);
	INT32 (*get_pause_by_band)(struct hdev_ctrl *ctrl, RTMP_STRING *arg, UINT8 band_idx);
	INT32 (*show_drr_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_protect_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*show_cca_info)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*set_cca_en)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	INT32 (*set_ba_winsize)(RTMP_ADAPTER *pAd, UINT16 wtbl_idx, UCHAR tid, UINT32 win_size);
	INT32 (*clear_keyloc)(RTMP_ADAPTER *pAd, UINT16 wtbl_idx, UINT32 key_loc);
	INT32 (*show_txv_info)(struct hdev_ctrl *ctrl, void *data);
	INT32 (*check_txv)(struct hdev_ctrl *ctrl, UCHAR *name, UINT32 data, UINT8 band_idx);
	VOID (*show_bcn_info)(struct hdev_ctrl *ctrl, UCHAR bandidx);
	VOID (*dump_wtbl_info)(struct _RTMP_ADAPTER *pAd, UINT16 wtbl_idx);
	VOID (*dump_wtbl_mlo_omac)(struct _RTMP_ADAPTER *pAd, UINT16 wtbl_idx);
	VOID (*dump_wtbl_mac)(struct _RTMP_ADAPTER *pAd, UINT16 wtbl_idx);
	VOID (*dump_wtbl_base_info)(struct _RTMP_ADAPTER *pAd);
	INT32 (*dump_ple_amsdu_count_info)(struct hdev_ctrl *ctrl);
	VOID (*set_hw_amsdu)(struct _RTMP_ADAPTER *pAd, UINT32 wcid, UINT8 num, UINT32 len);
	VOID (*set_header_translation)(struct _RTMP_ADAPTER *pAd, UINT32 wcid, BOOLEAN on);
	VOID (*show_dma_info)(struct hdev_ctrl *ctrl);
	UINT32 (*show_asic_rx_stat) (struct _RTMP_ADAPTER *pAd, UINT type);
#ifdef RANDOM_PKT_GEN
	INT (*set_txctrl_proc)(struct hdev_ctrl *ctrl, RTMP_STRING *arg);
	VOID (*regular_pause_umac)(struct hdev_ctrl *ctrl);
#endif /* RANDOM_PKT_GEN */
	UINT32 (*get_lpon_frcr)(RTMP_ADAPTER *pAd);
#ifdef VOW_SUPPORT
	UINT32 (*show_sta_acq_info)(RTMP_ADAPTER *pAd, UINT32 *ple_stat,
		  UINT32 *sta_pause, UINT32 *dis_sta_map, UINT32 dumptxd);
	VOID (*show_txcmdq_info)(RTMP_ADAPTER *pAd, UINT32 ple_txcmd_stat, UINT32 dumptxd);
	VOID (*get_ple_acq_stat)(RTMP_ADAPTER *pAd, UINT32 *ple_stat);
	VOID (*get_ple_txcmd_stat)(RTMP_ADAPTER *pAd, UINT32 *ple_txcmd_stat);
	VOID (*get_dis_sta_map)(RTMP_ADAPTER *pAd, UINT32 *dis_sta_map);
	VOID (*get_sta_pause)(RTMP_ADAPTER *pAd, UINT32 *sta_pause);
	VOID (*get_obss_nonwifi_airtime)(RTMP_ADAPTER *pAd, UINT32 *at_info);
	UINT32 (*get_sta_airtime)(RTMP_ADAPTER *pAd, UINT16 sta, UINT16 ac, BOOLEAN tx);
	UINT32 (*get_sta_addr)(RTMP_ADAPTER *pAd, UINT32 sta);
	UINT32 (*get_sta_rate)(RTMP_ADAPTER *pAd, UINT32 sta);
	UINT32 (*get_sta_tx_cnt)(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 bw);
	INT32 (*set_sta_psm)(RTMP_ADAPTER *pAd, UINT32 sta, UINT32 psm);
	UINT32 (*get_sta_rx_rcpi)(RTMP_ADAPTER *pAd, UINT32 sta);
#endif	/* VOW_SUPPORT */
#if defined(DOT11_HE_AX)
#ifdef CONFIG_ATE
	INT32 (*ctrl_manual_hetb_tx)(RTMP_ADAPTER *ad,
					UINT8 band_idx,
					UINT8 ctrl,
					UINT8 bw,
					UINT8 ltf_gi,
					UINT8 stbc,
					struct _ATE_RU_STA *ru_sta);
	INT32 (*ctrl_manual_hetb_rx)(RTMP_ADAPTER *ad,
					UINT8 band_idx,
					BOOLEAN start,
					UINT8 bw,
					UINT8 gi_ltf,
					UINT8 stbc,
					ULONGLONG csd,
					struct _ATE_RU_STA *pri_sta,
					struct _ATE_RU_STA *sta_list);
	INT32 (*chip_ctrl_spe)(RTMP_ADAPTER *ad,
				UINT8 band_idx,
				UINT8 tx_mode,
				UINT8 spe_idx);
	UINT32 (*get_tx_mibinfo)(RTMP_ADAPTER *ad,
				UINT8 band_idx,
				UINT8 tx_mode,
				UINT8 bw);
#endif /*CONFIG_ATE*/
#endif
	INT32 (*show_fw_dbg_info)(RTMP_ADAPTER *pAd, UINT mcu_type);
	INT32 (*show_cmd_timeout_info)(RTMP_ADAPTER *pAd);
	INT32 (*show_wfdma_info)(RTMP_ADAPTER *pAd);
	INT32 (*show_coredump_proc)(RTMP_ADAPTER *pAd, UINT mcu_type);
	INT32 (*set_cpu_util_en)(RTMP_ADAPTER *pAd, UINT En);
	INT32 (*set_cpu_util_mode)(RTMP_ADAPTER *pAd, UINT Mode);
	INT (*chk_exception_type)(RTMP_ADAPTER *pAd);
	VOID (*show_mac_cap)(struct _RTMP_ADAPTER *ad);
} RTMP_CHIP_DBG;

enum {
	TOKEN_TX = (1 << 0),
	TOKEN_RX = (1 << 1),
};

struct token_info {
	UINT32 feature;
	UINT32 token_tx_cnt;
	UINT32 low_water_mark;
	UINT32 band0_token_cnt;
	UINT32 hw_tx_token_cnt;
	UINT32 token_rx_cnt;
};

/*
 * sub catalogs of chip cap
 */
#define BA_WIN_SZ_21 21
#define BA_WIN_SZ_24 24
#define BA_WIN_SZ_64 64
#define BA_WIN_SZ_256 256
#define BA_WIN_SZ_512 512
#define BA_WIN_SZ_1024 1024

struct ppdu_caps {
	UINT8 TxAggLimit;
	UINT16 *ba_range;
	BOOLEAN tx_amsdu_support;
	BOOLEAN rx_amsdu_in_ampdu_support;
	UINT8 max_amsdu_len;
	UINT8 ht_max_ampdu_len_exp;
#ifdef DOT11_VHT_AC
	UINT8 max_mpdu_len;
	UINT8 vht_max_ampdu_len_exp;
#endif /* DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
	UINT8 trig_mac_pad_dur;
	UINT8 max_agg_tid_num;
	UINT8 he_max_ampdu_len_exp;
	UINT8 default_pe_duration;
	UINT8 er_su_dis;
	UINT16 txop_dur_rts_thld;
	UINT8 he6g_max_mpdu_len;
	UINT8 he6g_max_ampdu_len_exp;
	UINT8 he6g_start_spacing;
	UINT8 he6g_smps;
#endif /* DOT11_HE_AX */
};

#define MAX_NSS 8
struct mcs_nss_caps {
	BOOLEAN g_band_256_qam;
	/* To separate path and stream */
	UINT8 max_nss;
	/* max_path [tx/rx] */
	UINT8 max_path[2];
	UINT8 max_vht_mcs;
	UINT8 bw160_max_nss;
	UINT8 max_24g_ru_num;
	UINT8 max_5g_ru_num;
	UINT8 ext_bw_nss;
};

#define WMM_DETECT_METHOD1 1
#define WMM_DETECT_METHOD2 2
struct qos_caps {
	UCHAR WmmHwNum;
	UCHAR wmm_detect_method;
	UINT32 TxOPScenario;
	UINT32 CurrentTxOP;
	UINT32 default_txop;
};

struct mt_io_ops {
	/* to be removed @20220331 */
	VOID (*hif_io_read32)(void *hdev_ctrl, UINT32 reg, UINT32 *val);
	VOID (*hif_io_write32)(void *hdev_ctrl, UINT32 reg, UINT32 val);
	VOID (*hif_io_forec_read32)(void *hdev_ctrl, UINT32 reg, UINT32 *val);
	VOID (*hif_io_forec_write32)(void *hdev_ctrl, UINT32 reg, UINT32 val);
	VOID (*mac_io_read32)(void *hdev_ctrl, UINT32 reg, UINT32 *val);
	VOID (*mac_io_write32)(void *hdev_ctrl, UINT32 reg, UINT32 val);
	VOID (*hw_io_read32)(void *hdev_ctrl, UINT32 reg, UINT32 *val);
	VOID (*hw_io_write32)(void *hdev_ctrl, UINT32 reg, UINT32 val);
	VOID (*mcu_io_read32)(void *hdev_ctrl, UINT32 reg, UINT32 *val);
	VOID (*mcu_io_write32)(void *hdev_ctrl, UINT32 reg, UINT32 val);
	VOID (*phy_io_read32)(void *hdev_ctrl, UINT32 reg, UINT32 *val);
	VOID (*phy_io_write32)(void *hdev_ctrl, UINT32 reg, UINT32 val);
};

struct rtmp_spe_map {
	UINT8 ant_sel;
	UINT8 spe_idx;
};

struct rtmp_spe_map_list {
	struct rtmp_spe_map *spe_map;
	UINT8 size;
};

struct _prek_ee_info {
	USHORT info_size;
	/* Group Calibration item */
	UINT32 cal_result_size;
	UINT32 cal_result_size_5g;
	UINT32 cal_result_size_6g;
	UINT32 cal_result_size_5g_2die;
	UINT32 cal_result_size_6g_2die;
	UINT32 cal_result_size_adcdcoc_2g;
	UINT32 cal_result_size_adcdcoc_5g;
	UINT32 cal_result_size_adcdcoc_6g;
	UINT32 pre_cal_total_size;
	UINT32 pre_cal_total_size_2adie;
	/*DPD FLATNESS*/
	UINT32 per_legaycy_ch_cal_size;
	UINT32 per_mem_ch_cal_size;
	UINT32 per_ch_cal_OTFG0_size;
	UINT32 per_ch_6g_num;
	UINT32 per_ch_5g_num;
	UINT32 per_ch_2g_num;
	UINT32 total_chan_for_per_ch;
	UINT32 dpd_cal_6g_legacy_size;
	UINT32 dpd_cal_6g_mem_size;
	UINT32 dpd_cal_6g_total_size;
	UINT32 dpd_cal_5g_legacy_size;
	UINT32 dpd_cal_5g_mem_size;
	UINT32 dpd_cal_5g_total_size;
	UINT32 dpd_cal_2g_total_size;
	UINT32 dpd_cal_total_size;
	/* Flash offset */
	UINT32 pre_cal_flash_offset;
	UINT32 group_flash_offset_g_begin;
	UINT32 group_flash_offset_a5_begin;
	UINT32 group_flash_offset_a6_begin;
	UINT32 dpd_flash_offset;
	UINT32 dpd_flash_offset_a6_begin;
	UINT32 dpd_flash_offset_a5_begin;
	UINT32 dpd_flash_offset_g_begin;


};

struct rro_rss_map {
	UINT32 map0;
	UINT32 map1;
};

struct mt_range {
	u32 start;
	u32 end;
};

typedef struct _RTMP_CHIP_CAP {
	/* ------------------------ packet --------------------- */
	UINT8 TXWISize;	/* TxWI or LMAC TxD max size */
	UINT8 RXWISize; /* RxWI or LMAC RxD max size */
	UINT8 tx_hw_hdr_len;	/* Tx Hw meta info size which including all hw info fields */
	UINT8 rx_hw_hdr_len;	/* Rx Hw meta info size */
#ifdef BCN_V2_SUPPORT /* add bcn v2 support , 1.5k beacon support */
	UINT8 max_v2_bcn_num;
#endif
	UINT16 tx_ring_size;
	struct token_info tkn_info;
	BOOLEAN multi_token_ques_per_band;
	BOOLEAN txd_flow_ctl;
	enum ASIC_CAP asic_caps;
	enum MAC_CAP mac_caps;
	enum PHY_CAP phy_caps;
	enum HIF_TYPE hif_type;
	enum MAC_TYPE mac_type;
	enum BBP_TYPE bbp_type;
	enum MCU_TYPE MCUType;
	enum RF_TYPE rf_type;
	enum EFUSE_TYPE efuse_type;
	enum TXD_TYPE txd_type;
	struct ppdu_caps ppdu;
	struct qos_caps qos;

	UINT32 MaxNumOfRfId;
	UINT32 MaxNumOfBbpId;

	/* beacon */
	UINT8 BcnMaxNum;		/* chip capability */
	UINT8 perBandBcnMaxNum;
	UINT32 WtblPseAddr;		/* */
#ifdef CUSTOMER_VENDOR_IE_SUPPORT
	UINT16 BcnMaxLength;
#endif

	/* function */
	BOOLEAN FlgIsHwWapiSup;

	BOOLEAN FlgIsHwAntennaDiversitySup;
#ifdef TXBF_SUPPORT
	BOOLEAN FlgHwTxBfCap;
	BOOLEAN FlgITxBfBinWrite;
	UINT8 pfmu_num_max;
#endif /* TXBF_SUPPORT */


#ifdef DYNAMIC_VGA_SUPPORT
	BOOLEAN dynamic_vga_support;
	INT32 compensate_level;
	INT32 avg_rssi_0;
	INT32 avg_rssi_1;
	INT32 avg_rssi_all;
	UCHAR dynamic_chE_mode;
	BOOLEAN dynamic_chE_trigger;
#endif /* DYNAMIC_VGA_SUPPORT */

	/* ---------------------------- signal ---------------------------------- */
#define SNR_FORMULA1		0	/* ((0xeb	 - pAd->StaCfg[0].wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1	/* (pAd->StaCfg[0].wdev.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2	/* (pAd->StaCfg[0].wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA4		3	/* for MT7603 */
	UINT8 SnrFormula;
#ifdef DOT11_VHT_AC
	UINT8 ac_off_mode;		/* 11AC off mode */
#endif /* DOT11_VHT_AC */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2)
	INT16	PAModeCCK[4];
	INT16	PAModeOFDM[8];
	INT16	PAModeHT[16];
#ifdef DOT11_VHT_AC
	INT16	PAModeVHT[10];
#endif /* DOT11_VHT_AC */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2) */

	/* ---------------------------- others ---------------------------------- */
#ifdef RTMP_EFUSE_SUPPORT
	UINT16 EFUSE_USAGE_MAP_START;
	UINT16 EFUSE_USAGE_MAP_END;
	UINT8 EFUSE_USAGE_MAP_SIZE;
	UINT8 EFUSE_RESERVED_SIZE;
#endif /* RTMP_EFUSE_SUPPORT */

	UINT16 efuse_content_start;
	UINT16 efuse_content_end;
	const UCHAR *EEPROM_DEFAULT_BIN;
	UINT16 EEPROM_DEFAULT_BIN_SIZE;
	UINT16 EFUSE_BUFFER_CONTENT_SIZE;
	UINT8 efuse_free_block_ver;
	UINT8 efuse_total_bank_num;
	UINT8 ddie_lvts_num;

#ifdef RTMP_FLASH_SUPPORT
	BOOLEAN ee_inited;
#endif /* RTMP_FLASH_SUPPORT */

	/*
		Define the times that Ap Send ProbeResponse
		1 : Multi-Sta Support better
		2 : Default
	*/
	UINT8 ProbeRspTimes;

	/*
	 * 0: no limit
	 * others : limit number of tx probe resp every second
	 */
	UINT16 ProbeRspMaxNum;

	/*
	 * 0: MBSSID_MODE0
	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit2:bit0] of byte5)
	 * 1: MBSSID_MODE1
	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit4:bit2] of byte0)
	 */
	UINT8 MBSSIDMode;

#ifdef DOT11W_PMF_SUPPORT
/* All packets must software encryption. */
#define PMF_ENCRYPT_MODE_0	0
/* Data packets do hardware encryption,
	management packet do software encryption. */
#define PMF_ENCRYPT_MODE_1	1
/* Data and management packets do hardware encryption. */
#define PMF_ENCRYPT_MODE_2	2
	UINT8	FlgPMFEncrtptMode;
#endif /* DOT11W_PMF_SUPPORT */

	UINT32 need_load_patch;
	UINT32 need_load_fw;
#ifdef WIFI_RAM_EMI_SUPPORT
	UINT32 need_load_emi_fw;
#endif /* WIFI_RAM_EMI_SUPPORT */
	enum load_patch_flow load_patch_flow;
	enum load_fw_flow load_fw_flow;
	UINT32 patch_format;
	UINT32 fw_format;
	UINT32 load_patch_method;
	UINT32 load_fw_method;
	UINT32 rom_patch_offset;
	UINT32 decompress_tmp_addr;

	UINT8 cmd_header_len;
	UINT8 cmd_padding_len;

#ifdef SINGLE_SKU_V2
	CHAR	Apwrdelta;
	CHAR	Gpwrdelta;
#endif /* SINGLE_SKU_V2 */

#ifdef CONFIG_SWITCH_CHANNEL_OFFLOAD
	UINT16 ChannelParamsSize;
	UCHAR *ChannelParam;
	INT XtalType;
#endif

	BOOLEAN tssi_enable;

#ifdef MT_MAC
	struct MT_TX_PWR_CAP MTTxPwrCap;
	UCHAR TmrEnable;
	UINT8 OmacNums;
	UINT8 BssNums;
	UINT8 MBSSStartIdx;
	UINT8 MaxRepeaterNum;
	UINT8 ExtMbssOmacStartIdx;
	UINT8 RepeaterStartIdx;
#endif
	BOOLEAN fgIsNeedPretbttIntEvent;
	UCHAR TmrHwVer;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	BOOLEAN fgRateAdaptFWOffload;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

	/* specific PDA Port HW Address */
	UINT16 PDA_PORT;

	UINT8 APPSMode;

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	UINT8   nWakeupInterface;
#endif /* MT_WOW_SUPPORT */

	/* the length of partial payload delivered to MCU for further processing */
	UINT16 CtParseLen;
	UCHAR qm;
#define QM_V1	1
#define QM_V2	2
	UCHAR qm_version;
	BOOLEAN rx_qm_en;
	UCHAR rx_qm;
	UCHAR qm_tm;
	UCHAR hif_tm;
	UCHAR hw_ops_ver;
	UCHAR hw_protect_update_ver;
#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	RBIST_DESC_T *pICapDesc;
	UINT8 ICapBankNum;
	UINT32 ICapMaxIQCnt;
	UINT32 ICapADCIQCnt;
	UINT32 ICapIQCIQCnt;
	UINT32 ICapAFEIQCnt;
	UINT32 ICapBankSmplCnt;
	UINT32 ICapPackedADC;
	UINT32 ICapWF01PackedADC;
	UINT32 ICapWF12PackedADC;
	UINT32 ICapWF02PackedADC;
	RBIST_DESC_T *pSpectrumDesc;
	UINT8 SpectrumBankNum;
	UINT32 SpectrumWF0ADC;
	UINT32 SpectrumWF1ADC;
	UINT32 SpectrumWF2ADC;
	UINT32 SpectrumWF3ADC;
	UINT32 SpectrumWF0FIIQ;
	UINT32 SpectrumWF1FIIQ;
	UINT32 SpectrumWF2FIIQ;
	UINT32 SpectrumWF3FIIQ;
	UINT32 SpectrumWF0FDIQ;
	UINT32 SpectrumWF1FDIQ;
	UINT32 SpectrumWF2FDIQ;
	UINT32 SpectrumWF3FDIQ;
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
	UCHAR band_cnt;
	UCHAR hw_max_amsdu_nums;
	UINT32 amsdu_txdcmp;
	BOOLEAN tx_delay_support;
	UCHAR tx_delay_mode;
	/* For TP tuning dynamically */
	UINT32 Ap2GPeakTpTH;
	UINT32 Ap5GPeakTpTH;
	UINT32 ApDBDC2GPeakTpTH;
	UINT32 ApDBDC5GPeakTpTH;
	UCHAR RxSwRpsEnable;
	UINT32 RxSwRpsTpThreshold;
	UINT32 sw_rps_tp_thd_dl;
	UCHAR RxSwRpsCpu;
#ifdef RX_RPS_SUPPORT
	UINT32 RxSwRpsCpuMap[NR_CPUS];
	UINT32 RxSwRpsNum;
#endif
#ifdef WIFI_RAM_EMI_SUPPORT
	UINT32 mcu_emi_addr_base;
	UINT32 ram_ilm_emi_addr_offset;
	UINT32 ram_dlm_emi_addr_offset;
	UINT32 emi_phy_addr;
	UINT32 emi_phy_addr_size;
#endif /* WIFI_RAM_EMI_SUPPORT */
	UINT32 max_tx_process;
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	UINT8 twt_sp_duration_min_num; /* Dictate TWT support */
#endif /* WIFI_TWT_SUPPORT */
	UINT8 mu_edca_timer;
#endif /* DOT11_HE_AX */
#ifdef OCE_SUPPORT
	BOOLEAN FdFrameFwOffloadEnabled;
#endif /* OCE_SUPPORT */
	UINT16 wtbl_ucast_entries;
	UINT16 wtbl_max_entries;	/* to instead MAX_LEN_OF_MAC_TABLE */
	UINT16 wtbl_no_matched;		/* index if no entry was matched */
	UINT16 drr_max_dw;
	UINT16 drr_max_dw_twt;
	BOOLEAN drr_cmd_query;	/* support WM query DRR pause cmd */
#ifdef SW_CONNECT_SUPPORT
/* HW part */
	struct mt_range  wtbl_ucast; /* ucast wcid low/high */
	struct mt_range  wtbl_bcast; /* bcast wcid low/high */
	UINT16 wtbl_bcast_entries;

/* SW part */
	UINT16 sw_ucast_max_entries; /* pure s/w tr_entry num , w/o hw part */
	struct mt_range ucast_sw; /* pure s/w u/c tr_entry low/high point , w/o hw part */

	UINT16 sw_bcast_max_entries; /* pure s/w tr_entry num , w/o hw part */
	struct mt_range bcast_sw; /* pure s/w b/c tr_entry low/high point , w/o hw part */

/* HW + SW  part */
	UINT16 hw_sw_max_entries; /* sum of wtbl_max_entries + sw_ucast_max_entries */
	UINT16 hw_sw_max_ucast_entries; /* sum of wtbl_ucast_entries + sw_ucast_max_entries */
#endif /* SW_CONNECT_SUPPORT */
	UINT8 single_sku_type_parse_num;
	UINT8 single_sku_para_parse_num;
	UINT8 single_sku_type_num;
	UINT16 single_sku_para_num;
	UINT8 backoff_type_parse_num;
	UINT8 backoff_para_parse_num;
	UINT8 backoff_type_num;
	UINT16 backoff_para_num;
	UINT8 single_sku_fill_tbl_cck;
	UINT8 single_sku_fill_tbl_ofdm;
	UINT8 single_sku_fill_tbl_ht20;
	UINT8 single_sku_fill_tbl_ht40;
	UINT8 single_sku_fill_tbl_vht20;
	UINT8 single_sku_fill_tbl_vht40;
	UINT8 single_sku_fill_tbl_vht80;
	UINT8 single_sku_fill_tbl_vht160;
	UINT8 single_sku_parse_tbl_htvht40;
	UINT8 backoff_tbl_bfon_ht40;
	UINT8 *single_sku_fill_tbl_length;
	UINT8 txpower_type;
	UINT8 single_sku_tbl_type_ht40;
	UINT8 backoff_tbl_bf_on_type_ht40;
	UINT8 rxv_pkt_hdr_dw_num;
	UINT8 rxv_entry_hdr_dw_num;
	UINT8 rxv_cmn1_dw_num;
	UINT8 rxv_cmn2_dw_num;
	UINT8 rxv_usr1_dw_num;
	UINT8 rxv_usr2_dw_num;
	struct rtmp_spe_map_list spe_map_list;
	UINT8 channelbw;
	BOOLEAN mgmt_ctrl_frm_hw_htc_disable;
	UINT16 peak_txop;
#ifdef BCN_PROTECTION_SUPPORT
	UINT8 bcn_prot_sup; /* 0:off, 1: sw mode, 2:hw mode */
#endif
#ifdef VLAN_SUPPORT
	UINT8 vlan_rx_tag_mode;
#endif
#if defined(PRE_CAL_BELLWETHER_SUPPORT) || defined(PRE_CAL_MT7990_SUPPORT) || \
	defined(PRE_CAL_MT7992_SUPPORT) || defined(PRE_CAL_MT7993_SUPPORT)
	struct _prek_ee_info prek_ee_info;
#endif
	UINT32 hw_version;
	UINT32 hif_group_page_size;
#if defined(BELLWETHER) || defined(MT7990) || defined(MT7992) || defined(MT7993)
	USHORT rxgaincal_ofst;
#endif

#ifdef WIFI_UNIFIED_COMMAND
	UINT8 uni_cmd_header_len;
	UINT32 u4MaxInBandCmdLen;
	BOOLEAN uni_cmd_support;
#endif /* WIFI_UNIFIED_COMMAND */
	struct bus_info *bus_info;
	UINT32 sizeof_rxd_grp0;
	UINT8 hw_rro_en;
	UINT8 fid_shift;
	UINT32 fid_invalid_id;
	UINT32 rro_cap;
	UINT8 hw_band_num;
	UINT8 tx_power_boost_ver;
	UINT16 max_ba_wsize_scene_mlo;
	UINT16 step_one_timeout;
	UINT16 flush_all_timeout;
#ifdef DOT11V_MBSSID_SUPPORT
	/* bitmap for 11v transmitted bss index */
	UINT32 transmitted_bss_bitmap;
	/* 11v transmitted bss number */
	UINT8 transmitted_bss_num;
#endif /* DOT11V_MBSSID_SUPPORT */
	struct rro_rss_map rss_map;
	UINT16 WtblDupNum;
#ifdef RT_CFG80211_SUPPORT
	struct ieee80211_sband_iftype_data sband_data[NUM_NL80211_BANDS];
#endif /* RT_CFG80211_SUPPORT */
} RTMP_CHIP_CAP;

#ifdef OCE_SUPPORT
#define IS_FD_FRAME_FW_MODE(_CAP)	 ((_CAP)->FdFrameFwOffloadEnabled)
#endif /* OCE_SUPPORT */

#define IS_TX_DELAY_SW_MODE(_CAP)	 ((_CAP)->tx_delay_support  \
									 && ((_CAP)->tx_delay_mode == TX_DELAY_SW_MODE))

#define IS_TX_DELAY_HW_MODE(_CAP)	 ((_CAP)->tx_delay_support  \
									 && ((_CAP)->tx_delay_mode == TX_DELAY_HW_MODE))

/*
  *   EEPROM operation related marcos
  */
BOOLEAN chip_eeprom_read16(struct _RTMP_ADAPTER *pAd, UINT32 offset, USHORT *value);
BOOLEAN chip_eeprom_read_with_range(struct _RTMP_ADAPTER *pAd, UINT32 start, UINT32 length, UCHAR *pbuf);

#define RT28xx_EEPROM_READ16(_pAd, _offset, _val)   chip_eeprom_read16(_pAd, _offset, &(_val))

#define RT28xx_EEPROM_WRITE16(_pAd, _offset, _val)		\
	do {\
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
		if (ops->eewrite) \
			ops->eewrite(_pAd, (_offset), (USHORT)(_val));\
	} while (0)

#define RT28xx_EEPROM_READ_WITH_RANGE(_pAd, _start, _length, _pbuf)   chip_eeprom_read_with_range(_pAd, _start, _length, _pbuf)

#define RT28xx_EEPROM_WRITE_WITH_RANGE(_pAd, _start, _length, _pbuf)		\
	do { \
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
		if (ops->eewrite_range) \
			ops->eewrite_range(_pAd, _start, _length, _pbuf);\
	} while (0)

#define RTMP_CHIP_ENABLE_GREENAP(__pAd, __greenap_on_off)	\
			do { \
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->EnableAPMIMOPS != NULL)	\
					ops->EnableAPMIMOPS(__pAd, __greenap_on_off); \
			} while (0)

#define RTMP_CHIP_DISABLE_GREENAP(__pAd, __greenap_on_off)	\
			do { \
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->DisableAPMIMOPS != NULL) \
					ops->DisableAPMIMOPS(__pAd, __greenap_on_off);	\
			} while (0)

#define RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(__pAd, __pCfgOfTxPwrCtrlOverMAC)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->AsicGetTxPowerOffset != NULL)	\
					ops->AsicGetTxPowerOffset(__pAd, __pCfgOfTxPwrCtrlOverMAC);	\
			} while (0)

#define RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(	\
				__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1, __Channel)	\
		do {	\
			struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
			if (ops->AsicTxAlcGetAutoAgcOffset != NULL)	\
				ops->AsicTxAlcGetAutoAgcOffset(	\
						__pAd, __pDeltaPwr, __pTotalDeltaPwr, __pAgcCompensate, __pDeltaPowerByBbpR1, __Channel);	\
		} while (0)

#define RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->AsicExtraPowerOverMAC != NULL)	\
					ops->AsicExtraPowerOverMAC(__pAd);	\
			} while (0)

#define RTMP_CHIP_HIGH_POWER_TUNING(__pAd, __pRssi)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->HighPowerTuning != NULL) \
					ops->HighPowerTuning(__pAd, __pRssi); \
			} while (0)

#define RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(__pAd, __pAntenna)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->AsicAntennaDefaultReset != NULL) \
					ops->AsicAntennaDefaultReset(__pAd, __pAntenna);	\
			} while (0)

#define RTMP_NET_DEV_NICKNAME_INIT(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->NetDevNickNameInit != NULL)	\
					ops->NetDevNickNameInit(__pAd);	\
			} while (0)

#ifdef CAL_FREE_IC_SUPPORT
#define RTMP_CAL_FREE_IC_CHECK(__pAd, __is_cal_free)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->is_cal_free_ic != NULL)	\
					__is_cal_free = ops->is_cal_free_ic(__pAd);	\
				else		\
					__is_cal_free = FALSE;	\
			} while (0)

#define RTMP_CAL_FREE_DATA_GET(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->cal_free_data_get != NULL) { \
					ops->cal_free_data_get(__pAd);	\
					PD_SET_E2P_SOURCE(__pAd->physical_dev, E2P_SRC_FROM_EFUSE);\
				} \
			} while (0)
#endif /* CAL_FREE_IC_SUPPORT */

#define RTMP_EEPROM_ASIC_INIT(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->NICInitAsicFromEEPROM != NULL)	\
					ops->NICInitAsicFromEEPROM(__pAd);	\
			} while (0)

#define RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(__pAd)								\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->InitTemperCompensation != NULL)					\
					ops->InitTemperCompensation(__pAd);	\
			} while (0)

#define RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(__pAd)						\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->TemperCompensation != NULL)					\
					ops->TemperCompensation(__pAd);	\
			} while (0)

#define RTMP_CHIP_CCK_MRC_STATUS_CTRL(__pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->CckMrcStatusCtrl != NULL)	\
					ops->CckMrcStatusCtrl(__pAd); \
			} while (0)

#define RTMP_CHIP_RADAR_GLRT_COMPENSATE(__pAd) \
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->RadarGLRTCompensate != NULL) \
					ops->RadarGLRTCompensate(__pAd);	\
			} while (0)

/*check here before*/
#define RTMP_SECOND_CCA_DETECTION(__pAd) \
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->SecondCCADetection != NULL)	\
					ops->SecondCCADetection(__pAd);	\
			} while (0)

#define DISABLE_TX_RX(_pAd, _Level)	\
			do { \
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->DisableTxRx != NULL)	\
					ops->DisableTxRx(_pAd, _Level);	\
			} while (0)

#define ASIC_RADIO_ON(_pAd, _Stage)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicRadioOn != NULL)	\
					ops->AsicRadioOn(_pAd, _Stage);	\
			} while (0)

#define ASIC_RADIO_OFF(_pAd, _Stage)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicRadioOff != NULL) \
					ops->AsicRadioOff(_pAd, _Stage);	\
			} while (0)

#ifdef MICROWAVE_OVEN_SUPPORT
#define ASIC_MEASURE_FALSE_CCA(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicMeasureFalseCCA != NULL)	\
					ops->AsicMeasureFalseCCA(_pAd);	\
			} while (0)

#define ASIC_MITIGATE_MICROWAVE(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicMitigateMicrowave != NULL)	\
					ops->AsicMitigateMicrowave(_pAd);	\
			} while (0)
#endif /* MICROWAVE_OVEN_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
#define ASIC_WOW_ENABLE(_pAd, _pStaCfg)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicWOWEnable != NULL)	\
					ops->AsicWOWEnable(_pAd, _pStaCfg);	\
			} while (0)

#define ASIC_WOW_DISABLE(_pAd, _pStaCfg)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicWOWDisable != NULL)	\
					ops->AsicWOWDisable(_pAd, _pStaCfg);	\
			} while (0)

#define ASIC_WOW_INIT(_pAd) \
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops->AsicWOWInit != NULL)	\
					ops->AsicWOWInit(_pAd);	\
			} while (0)

#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#define MCU_CTRL_INIT(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops && ops->MCUCtrlInit != NULL)	\
					ops->MCUCtrlInit(_pAd);	\
			} while (0)

#define MCU_CTRL_EXIT(_pAd)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(_pAd->hdev_ctrl);\
				if (ops && ops->MCUCtrlExit != NULL)	\
					ops->MCUCtrlExit(_pAd);	\
			} while (0)

#define RTMP_GET_BAND_SELECT(__pAd, __band_select)	\
			do {	\
				struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(__pAd->hdev_ctrl);\
				if (ops->GetBandSelectFromEEPROM != NULL)	\
					*__band_select = ops->GetBandSelectFromEEPROM(__pAd);	\
				else		\
					*__band_select = 0;	\
			} while (0)



#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT32 AsicBfStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16				u2PhyMode,
	UINT8				u1BssIdx,
	UINT16				u2WlanIdx);

INT32 AsicBfeeStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16				u2PhyMode,
	UINT8				u1BssIdx,
	UINT16				u2WlanIdx);

INT32 AsicBfStaRecRelease(
	struct _RTMP_ADAPTER *pAd,
	UCHAR				ucBssIdx,
	UINT16				u2WlanIdx);

INT32 AsicBfPfmuMemAlloc(
	struct _RTMP_ADAPTER *pAd,
	UCHAR				ucSu_Mu,
	UCHAR				ucWlanId);

INT32 AsicBfPfmuMemRelease(
	struct _RTMP_ADAPTER *pAd,
	UCHAR				ucWlanId);

INT32 AsicTxBfTxApplyCtrl(
	struct _RTMP_ADAPTER *pAd,
	UCHAR				ucWlanId,
	BOOLEAN			  fgETxBf,
	BOOLEAN			  fgITxBf,
	BOOLEAN			  fgMuTxBf,
	BOOLEAN			  fgPhaseCali);

INT32 AsicTxBfApClientCluster(
	struct _RTMP_ADAPTER *pAd,
	UCHAR				ucWlanId,
	UCHAR				ucCmmWlanId);

INT32 AsicTxBfHwEnStatusUpdate(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN			  fgETxBf,
	BOOLEAN			  fgITxBf);

INT32 AsicTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BfNum,
	UINT8 u1BfBitmap,
	UINT8 u1BfSelBand[]);

INT32 AsicTxBfCfgBfPhy(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData);

#endif /* MT_MAC && TXBF_SUPPORT */

INT32 AsicHeraStbcPriorityCtrl(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData
);

VOID AsicSetRxAnt(struct _RTMP_ADAPTER *pAd, UCHAR	Ant);

#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(
	struct _RTMP_ADAPTER *pAd
);

VOID AsicMitigateMicrowave(
	struct _RTMP_ADAPTER *pAd
);
#endif /* MICROWAVE_OVEN_SUPPORT */

/*DMA operate*/
enum {
	DMA_TX,
	DMA_RX,
	DMA_TX_RX,
};

#ifdef TXBF_SUPPORT
VOID chip_tx_bf_init(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF);
#endif /*TXBF_SUPPORT*/

UINT32 chip_get_sku_tbl_idx(RTMP_ADAPTER *ad, UINT8 *sku_tbl_idx);
BOOLEAN chip_check_rf_lock_down(struct _RTMP_ADAPTER *pAd);

INT32 chip_cmd_tx(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
VOID chip_fw_init(struct _RTMP_ADAPTER *ad);
VOID chip_get_sta_per(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, PUINT8 u1PER);
INT32 chip_ra_init(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry);
VOID chip_get_rssi(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *RssiSet);
VOID chip_get_cninfo(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT16 *pCnInfo);
VOID chip_set_mgmt_pkt_txpwr(struct _RTMP_ADAPTER *pAd,	struct wifi_dev *wdev, UINT8 prctg);
VOID chip_get_rx_stat(struct _RTMP_ADAPTER *ad,
		UCHAR band_idx,
		P_TESTMODE_STATISTIC_INFO prtest_mode_stat_info);
INT32 chip_get_wf_path_comb(struct _RTMP_ADAPTER *ad,
		UINT8 band_idx,
		BOOLEAN dbdc_mode_en,
		UINT8 *path,
		UINT8 *path_len);
INT32 chip_get_rx_stat_band(struct _RTMP_ADAPTER *ad,
		UINT8 band_idx,
		UINT8 blk_idx,
		P_TEST_RX_STAT_BAND_INFO prx_band);
INT32 chip_get_rx_stat_path(struct _RTMP_ADAPTER *ad,
		UINT8 band_idx,
		UINT8 blk_idx,
		P_TEST_RX_STAT_PATH_INFO prx_path);
INT32 chip_get_rx_stat_user(struct _RTMP_ADAPTER *ad,
		UINT8 band_idx,
		UINT8 blk_idx,
		P_TEST_RX_STAT_USER_INFO prx_user);
INT32 chip_get_rx_stat_comm(struct _RTMP_ADAPTER *ad,
		UINT8 band_idx,
		UINT8 blk_idx,
		P_TEST_RX_STAT_COMM_INFO prx_comm);
INT32 chip_get_rx_stat_all(struct _RTMP_ADAPTER *ad,
		UINT8 band_idx,
		UINT8 blk_idx,
		struct _TEST_RX_STAT_ALL_INFO *prx_all);

VOID chip_get_rxv_cnt(struct _RTMP_ADAPTER *ad, UINT8 band_idx, UINT32 *byte_cnt);
VOID chip_get_rxv_content(struct _RTMP_ADAPTER *ad, UINT8 band_idx, PVOID *content);
VOID chip_show_rxv_info(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
VOID chip_dump_rxv_raw_data(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
VOID chip_reset_rxv_stat(struct _RTMP_ADAPTER *ad, UINT8 band_idx);
VOID chip_parse_rxv_packet(struct _RTMP_ADAPTER *ad, UINT32 Type, struct _RX_BLK *RxBlk, UCHAR *Data);
VOID chip_parse_rxv_entry(struct _RTMP_ADAPTER *ad, VOID *Data);
VOID chip_rxv_dump_start(struct _RTMP_ADAPTER *ad);
VOID chip_rxv_dump_stop(struct _RTMP_ADAPTER *ad);
VOID chip_rxv_dump_buf_alloc(struct _RTMP_ADAPTER *ad, UINT8 type_mask);
VOID chip_rxv_dump_buf_clear(struct _RTMP_ADAPTER *ad);
VOID chip_rxv_dump_show_list(struct _RTMP_ADAPTER *ad);
VOID chip_rxv_dump_show_rpt(struct _RTMP_ADAPTER *ad);
VOID chip_rxv_dump_rxv_content_compose(struct _RTMP_ADAPTER *ad, UINT8 entry_idx, VOID *rxv_content, UINT32 *len);
VOID chip_rxv_content_len(struct _RTMP_ADAPTER *pAd, UINT8 type_mask, UINT8 rxv_sta_cnt, UINT16 *len);
INT chip_show_pwr_info(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg);
INT32 chip_tssi_set(struct _RTMP_ADAPTER *ad, char *efuse);
INT32 chip_pa_lna_set(struct _RTMP_ADAPTER *ad, char *efuse);
UINT16 chip_get_tid_sn(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR tid);
VOID chip_subsys_int_handler(RTMP_ADAPTER *pAd, void *hif_chip);
VOID chip_sw_int_handler(RTMP_ADAPTER *pAd, void *hif_chip);
VOID chip_hif_chip_match(VOID *hdev_ctrl);
VOID chip_hif_pci_slave_chip_defer_create(VOID *hdev_ctrl);
INT32 chip_fill_key_install_cmd(
	VOID *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID **wtbl_security_key,
	UINT32 *cmd_len);
#ifdef ZERO_PKT_LOSS_SUPPORT
INT chip_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, void *ChStat);
#endif
INT chip_update_ampdu_raw_counters(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UINT32 chip_get_sub_chipid(struct _RTMP_ADAPTER *pAd, UINT32 *sub_chipid);

#ifdef WIFI_UNIFIED_COMMAND
INT32 chip_fill_key_install_uni_cmd(
	void *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	void *wtbl_security_key);

INT32 chip_fill_key_install_uni_cmd_dynsize_check(
	VOID *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UINT32 *cmd_len);
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef CONFIG_TX_DELAY
VOID chip_tx_deley_parm_init(
	VOID *hdev_ctrl,
	UCHAR tx_delay_mode,
	struct tx_delay_control *tx_delay_ctl);
#endif

#ifdef ERR_RECOVERY
VOID chip_dump_ser_stat(RTMP_ADAPTER *pAd, UINT8 dump_lvl);
#ifdef MT7915_E1_WORKAROUND
#ifdef WFDMA_WED_COMPATIBLE
VOID chip_sw_int_polling(RTMP_ADAPTER *pAd);
#endif
#endif
VOID chip_update_chip_cap(struct _RTMP_ADAPTER *ad);
#endif

INT chip_init_hw_ring_setting(struct _RTMP_ADAPTER *ad);
INT chip_do_extra_action(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UCHAR *peer_mac,
	enum CHIP_EXTRA_ACTION action,
	UCHAR *data,
	ULONG *data_len);
VOID chip_set_rro_timeout(struct _RTMP_ADAPTER *ad);
VOID chip_set_rro_rss_map(struct _RTMP_ADAPTER *ad);
BOOLEAN chip_set_spe_fr_table(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 rate_idx,
	VOID *handle);

UINT8 chip_determine_mlo_type(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
UINT8 chip_determine_mlo_allow(struct _RTMP_ADAPTER *ad, struct _vendor_ie_cap *vendor_ie);
UINT8 chip_determine_hmlo_support(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
#endif
