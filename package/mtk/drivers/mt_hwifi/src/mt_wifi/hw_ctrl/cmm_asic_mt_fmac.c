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
	cmm_asic_mt_fmac.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"
#include "hdev/hdev.h"
#include "mac_mt/fmac/mt_fmac.h"
#ifdef MT7990
#include "chip/mt7990_cr.h"
#endif
#ifdef MT7992
#include "chip/mt7992_cr.h"
#endif
#ifdef MT7993
#include "chip/mt7993_cr.h"
#endif


#ifdef CONFIG_AP_SUPPORT
/* because of the CR arrangement in HW are not in sequence, wrape the table to search fast.*/
static UINT32 LPON_TT0SBOR_CR_MAPPING_TABLE[] = {
	LPON_TT0TPCR,
	LPON_TT0SBOR1,
	LPON_TT0SBOR2,
	LPON_TT0SBOR3,
	LPON_TT0SBOR4,
	LPON_TT0SBOR5,
	LPON_TT0SBOR6,
	LPON_TT0SBOR7,
	LPON_TT0SBOR8,
	LPON_TT0SBOR9,
	LPON_TT0SBOR10,
	LPON_TT0SBOR11,
	LPON_TT0SBOR12,
	LPON_TT0SBOR13,
	LPON_TT0SBOR14,
	LPON_TT0SBOR15,
};
#endif /*CONFIG_AP_SUPPORT*/

VOID asic_wrap_protinfo_in_bssinfo(struct _RTMP_ADAPTER *ad, VOID *cookie)
{
	struct prot_info *prot = (struct prot_info *)cookie;

	struct wifi_dev *wdev = prot->wdev;
	struct _BSS_INFO_ARGUMENT_T *bss_info = &wdev->bss_info_argument;
	struct _BSS_INFO_ARGUMENT_T bss;
	struct prot_info *bss_prot = &bss.prot;
	struct _RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(ad->hdev_ctrl);

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"prepare to deliver setting to firmware\n");
	os_zero_mem(&bss, sizeof(bss));
	bss.ucBssIndex = bss_info->ucBssIndex;
	memcpy(bss.Bssid, bss_info->Bssid, MAC_ADDR_LEN);
	bss.u8BssInfoFeature = BSS_INFO_PROTECT_INFO_FEATURE;

	memcpy(bss_prot, prot, sizeof(struct prot_info));

	if (arch_ops->archSetBssid)
		arch_ops->archSetBssid(ad, &bss);
	else {
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"there is no lower layer implementation.\n");
	}

	return;
}

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID MtfAsicSwitchChannel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	/* TODO: Need to fix */
	/* TODO: shiang-usw, unify the ops */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->ChipSwitchChannel)
		ops->ChipSwitchChannel(pAd, SwChCfg);
	else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"For this chip, no specified channel switch function!\n");
}

static VOID RxFilterCfg2Row(UINT32 FilterMask, UINT32 *RowFilterMask)
{
	*RowFilterMask = FilterMask & RX_STBC_BCN_BC_MC ?
					 (*RowFilterMask | DROP_STBC_BCN_BC_MC) : (*RowFilterMask & ~(DROP_STBC_BCN_BC_MC));
	*RowFilterMask = FilterMask & RX_FCS_ERROR ?
					 (*RowFilterMask | DROP_FCS_ERROR_FRAME) : (*RowFilterMask & ~(DROP_FCS_ERROR_FRAME));
	*RowFilterMask = FilterMask & RX_PROTOCOL_VERSION ?
					 (*RowFilterMask | DROP_VERSION_NO_0) : (*RowFilterMask & ~(DROP_VERSION_NO_0));
	*RowFilterMask = FilterMask & RX_PROB_REQ ?
					 (*RowFilterMask | DROP_PROBE_REQ) : (*RowFilterMask & ~(DROP_PROBE_REQ));
	*RowFilterMask = FilterMask & RX_MC_ALL ?
					 (*RowFilterMask | DROP_MC_FRAME) : (*RowFilterMask & ~(DROP_MC_FRAME));
	*RowFilterMask = FilterMask & RX_BC_ALL ?
					 (*RowFilterMask | DROP_BC_FRAME) : (*RowFilterMask & ~(DROP_BC_FRAME));
	*RowFilterMask = FilterMask & RX_MC_TABLE ?
					 (*RowFilterMask | DROP_NOT_IN_MC_TABLE) : (*RowFilterMask & ~(DROP_NOT_IN_MC_TABLE));
	*RowFilterMask = FilterMask & RX_BC_MC_OWN_MAC_A3 ?
					 (*RowFilterMask | DROP_ADDR3_OWN_MAC) : (*RowFilterMask & ~(DROP_ADDR3_OWN_MAC));
	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A3 ?
					 (*RowFilterMask | DROP_DIFF_BSSID_A3) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A3));
	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A2 ?
					 (*RowFilterMask | DROP_DIFF_BSSID_A2) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A2));
	*RowFilterMask = FilterMask & RX_BCN_DIFF_BSSID ?
					 (*RowFilterMask | DROP_DIFF_BSSID_BCN) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BCN));
	*RowFilterMask = FilterMask & RX_CTRL_RSV ?
					 (*RowFilterMask | DROP_CTRL_RSV) : (*RowFilterMask & ~(DROP_CTRL_RSV));
	*RowFilterMask = FilterMask & RX_CTS ?
					 (*RowFilterMask | DROP_CTS) : (*RowFilterMask & ~(DROP_CTS));
	*RowFilterMask = FilterMask & RX_RTS ?
					 (*RowFilterMask | DROP_RTS) : (*RowFilterMask & ~(DROP_RTS));
	*RowFilterMask = FilterMask & RX_DUPLICATE ?
					 (*RowFilterMask | DROP_DUPLICATE) : (*RowFilterMask & ~(DROP_DUPLICATE));
	*RowFilterMask = FilterMask & RX_NOT_OWN_BSSID ?
					 (*RowFilterMask | DROP_NOT_MY_BSSID) : (*RowFilterMask & ~(DROP_NOT_MY_BSSID));
	*RowFilterMask = FilterMask & RX_NOT_OWN_UCAST ?
					 (*RowFilterMask | DROP_NOT_UC2ME) : (*RowFilterMask & ~(DROP_NOT_UC2ME));
	*RowFilterMask = FilterMask & RX_NOT_OWN_BTIM ?
					 (*RowFilterMask | DROP_DIFF_BSSID_BTIM) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BTIM));
	*RowFilterMask = FilterMask & RX_NDPA ?
					 (*RowFilterMask | DROP_NDPA) : (*RowFilterMask & ~(DROP_NDPA));
}


INT MtfAsicSetRxFilter(RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilter)
{
	UINT32 Value = 0;

	if (RxFilter.bPromiscuous)
		Value = RX_PROMISCUOUS_MODE;
	else {
		/*disable frame report & monitor mode*/
		Value &= ~RX_PROMISCUOUS_MODE;

		if (RxFilter.bFrameReport)
			Value |= RM_FRAME_REPORT_EN;
		else
			Value &=  ~(RM_FRAME_REPORT_EN);

		/*enable drop filter by RxfilterMask*/
		RxFilterCfg2Row(RxFilter.filterMask, &Value);
	}

	if (RxFilter.u1BandIdx)
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RFCR_BAND_1, Value);
	else
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_RFCR_BAND_0, Value);

	return TRUE;
}

#define INT_TIMER_EN_PRE_TBTT	0x1
#define INT_TIMER_EN_GP_TIMER	0x2
static INT SetIntTimerEn(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 type, UINT32 timeout)
{
	/* UINT32 mask, time_mask; */
	/* UINT32 Value; */
	return 0;
}

INT MtfAsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return SetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}

INT MtfAsicGetTsfTimeByDriver(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
#ifdef WIFI_UNIFIED_COMMAND
	return FALSE;
#else
	UINT32 Value = 0;

	/* keep for mt7915 */
	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR, &Value);
	Value = (Value & BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_HW_MODE_MASK) |
			BN0_WF_LPON_TOP_T0CR_TSF0_TIMER_SW_MODE_MASK;/* keep HW mode value. */
	MAC_IO_WRITE32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_T0CR_ADDR + HwBssidIdx * 4, Value);
	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR0_ADDR, low_part);
	MAC_IO_READ32(pAd->hdev_ctrl, BN0_WF_LPON_TOP_UTTR1_ADDR, high_part);
	return TRUE;
#endif /* !WIFI_UNIFIED_COMMAND */
}

typedef struct _SYNC_MODE_CR_TABLE_T {
	UINT32              u4ArbOpModeCR;
	UINT32              u4ArbBcnWmmCR;
	UINT32              u4LponMacTimerCr;
	UINT32              u4LponTbttCtrlCR;
	UINT32              u4LponPreTbttTime;/* set pretbtt time */
	UINT32              u4LponSyncModeCR;/* sync mode CR*/
	UINT32              u4IntEnableCR;
} SYNC_MODE_CR_TABLE_T, *PSYNC_MODE_CR_TABLE_T;

typedef struct _RTMP_WMM_PAIR {
	UINT32 Address;
	UINT32 Mask;
	UINT32 Shift;
} RTMP_WMM_PAIR, *PRTMP_WMM_PAIR;

#define SN_MASK 0xfff

UINT32 MtfAsicGetRxStat(RTMP_ADAPTER *pAd, UINT type)
{
	UINT32 value = 0;

	switch (type) {
	case HQA_RX_STAT_MACFCSERRCNT:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &value);
		value = value & 0xFFFF; /* [15:0] FCS ERR */
		break;

	case HQA_RX_STAT_MAC_MDRDYCNT:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &value);
		value = value & 0x3FFFFFF; /* [15:0] Mac Mdrdy*/
		break;

	case HQA_RX_STAT_MAC_RXLENMISMATCH:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR11, &value);
		value = value & 0xFFFF;
		break;

	case HQA_RX_STAT_PHY_MDRDYCNT:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5, &value);
		break;

	case HQA_RX_STAT_PHY_FCSERRCNT:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS4, &value);
		break;

	case HQA_RX_STAT_PD:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS0, &value);
		break;

	case HQA_RX_STAT_CCK_SIG_SFD:
		/* [31:16] SIG [15:0] SFD */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS1, &value);
		break;

	case HQA_RX_STAT_OFDM_SIG_TAG:
		/* [31:16] SIG [15:0] TAG */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS2, &value);
		break;

	case HQA_RX_STAT_RSSI:
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_2, &value);
		break;

	case  HQA_RX_STAT_RSSI_RX23:
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_2, &value);
		break;

	case HQA_RX_STAT_ACI_HITL:
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_AGC_DEBUG_4, &value);
		break;

	case HQA_RX_STAT_ACI_HITH:
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_AGC_DEBUG_4, &value);
		break;

	case HQA_RX_FIFO_FULL_COUNT:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR4, &value);
		value = (value >> 16) & 0xffff;
		break;

	case HQA_RX_FIFO_FULL_COUNT_BAND1:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR4, &value);
		value = (value >> 16) & 0xffff;
		break;

	case HQA_RX_RESET_PHY_COUNT:
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS5, &value);
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND0_PHYCTRL_STS4, &value);
		break;

	case HQA_RX_STAT_MACFCSERRCNT_BAND1:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &value);
		break;

	case HQA_RX_STAT_MAC_MDRDYCNT_BAND1:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &value);
		break;

	case HQA_RX_STAT_MAC_RXLENMISMATCH_BAND1:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR11, &value);
		break;

	case HQA_RX_RESET_MAC_COUNT:
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR3, &value);
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M0SDR10, &value);
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR3, &value);
		MAC_IO_READ32(pAd->hdev_ctrl, MIB_M1SDR10, &value);
		break;

	case HQA_RX_STAT_PHY_MDRDYCNT_BAND1:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS5, &value);
		break;

	case HQA_RX_STAT_PHY_FCSERRCNT_BAND1:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS4, &value);
		break;

	case HQA_RX_STAT_PD_BAND1:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS0, &value);
		break;

	case HQA_RX_STAT_CCK_SIG_SFD_BAND1:
		/* [31:16] SIG [15:0] SFD */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS1, &value);
		break;

	case HQA_RX_STAT_OFDM_SIG_TAG_BAND1:
		/* [31:16] SIG [15:0] TAG */
		PHY_IO_READ32(pAd->hdev_ctrl, RO_BAND1_PHYCTRL_STS2, &value);
		break;

	default:
		break;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"Type(%d):%x\n", type, value);
	return value;
}

#ifdef CONFIG_ATE
INT MtAsicSetTSSI(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	UINT32 WF0Offset = 0x10D04; /* WF_PHY_CR_FRONT CR_WF0_TSSI_1 */
	UINT32 WF1Offset = 0x11D04; /* WF_PHY_CR_FRONT CR_WF1_TSSI_1 */
	INT Ret = TRUE;
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (bOnOff == FALSE)
		CRValue = 0xE3F3F800;
	else
		CRValue = 0xE1010800;

	if ((WFSelect == 0) || (WFSelect == 2)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Set WF#%d TSSI off\n", WFSelect);
		PHY_IO_WRITE32(pAd->hdev_ctrl, WF0Offset, CRValue);

		if (bOnOff == FALSE) {
			/* off */
			PHY_IO_WRITE32(pAd->hdev_ctrl, 0x10D18, 0x0);
		}
	}

	if ((WFSelect == 1) || (WFSelect == 2)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Set WF#%d TSSI on\n", WFSelect);
		PHY_IO_WRITE32(pAd->hdev_ctrl, WF1Offset, CRValue);

		if (bOnOff == FALSE) {
			/* off */
			PHY_IO_WRITE32(pAd->hdev_ctrl, 0x11D18, 0x0);
		}
	}

	return Ret;
}


INT MtAsicSetDPD(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	ULONG WF0Offset = 0x10A08;
	ULONG WF1Offset = 0x11A08;
	INT Ret = TRUE;
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (bOnOff == FALSE) {
		/* WF0 */
		if ((WFSelect == 0) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF0Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF0Offset, CRValue);
		}

		/* WF1 */
		if ((WFSelect == 1) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF1Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF1Offset, CRValue);
		}

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Set WFSelect: %d DPD off\n", WFSelect);
	} else {
		if ((WFSelect == 0) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF0Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF0Offset, CRValue);
		}

		if ((WFSelect == 1) || (WFSelect == 2)) {
			PHY_IO_READ32(pAd->hdev_ctrl, WF1Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd->hdev_ctrl, WF1Offset, CRValue);
		}

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Set WFSelect: %d DPD on\n", WFSelect);
	}

	return Ret;
}
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
static VOID MtfAsicSetMbssLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 Value = 0;
	/* where the register for sub Bssid start from */
	UINT32 bssid_reg_base = LPON_SBTOR1;
	BOOLEAN any_other_mbss_enable = FALSE;

	ASSERT(mbss_idx >= 1);

	if (enable == TRUE) {
		/* if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);
		Value |= SBSS_TBTT0_TSF0_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
		Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
		Value |= SUB_BSSID0_TIME_OFFSET_n(mbss_idx * BCN_TRANSMIT_ESTIMATE_TIME);
		Value |= TBTT0_n_INT_EN;
		Value |= PRE_TBTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
		pAd->ApCfg.ext_mbss_enable_bitmap |= (enable << mbss_idx);
	} else {
		pAd->ApCfg.ext_mbss_enable_bitmap &= ~(enable << mbss_idx);

		if (pAd->ApCfg.ext_mbss_enable_bitmap)
			any_other_mbss_enable = TRUE;

		/* if there is any ext bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);

		if (any_other_mbss_enable == TRUE)
			Value |= SBSS_TBTT0_TSF0_EN;
		else
			Value &= ~SBSS_TBTT0_TSF0_EN;

		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
		Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
		Value &= ~TBTT0_n_INT_EN;
		Value &= ~PRE_TBTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
	}
}

VOID MtfDmacSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 regValue = 0;

	if (enable) {
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_ACBEN, &regValue);
		regValue |=  (1 << mbss_idx);
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_ACBEN, regValue);
	} else {
		MAC_IO_READ32(pAd->hdev_ctrl, RMAC_ACBEN, &regValue);
		regValue &=  ~(1 << mbss_idx);
		MAC_IO_WRITE32(pAd->hdev_ctrl, RMAC_ACBEN, regValue);
	}
}

VOID MtfDmacSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtfAsicSetMbssLPOffset(pAd, mbss_idx, enable);
}

static VOID MtfAsicSetExtTTTTLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	UINT32 Value = 0;
	/* where the register for sub Bssid start from */
	UINT32 bssid_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[mbss_idx];
	UINT32 first_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[1];
	BOOLEAN any_other_mbss_tttt_enable = FALSE;

	if (mbss_idx == 0)
		return;

	if (enable == TRUE) {
		/* if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, first_reg_base, &Value);
		Value |= SBSS_TTTT0_TSF0_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, first_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);
		Value &= ~SUB_BSSID0_TTTT_OFFSET_n_MASK;
		Value |= DEFAULT_TTTT_OFFSET_IN_MS;
		Value |= TTTT0_n_INT_EN;
		Value |= PRE_TTTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
		pAd->ApCfg.ext_mbss_tttt_enable_bitmap |= (enable << mbss_idx);
	} else {
		pAd->ApCfg.ext_mbss_tttt_enable_bitmap &= ~(enable << mbss_idx);

		if (pAd->ApCfg.ext_mbss_tttt_enable_bitmap)
			any_other_mbss_tttt_enable = TRUE;

		/* if there is any ext bssid is enable. this bit shall be 1 always. */
		MAC_IO_READ32(pAd->hdev_ctrl, first_reg_base, &Value);

		if (any_other_mbss_tttt_enable == TRUE)
			Value |= SBSS_TTTT0_TSF0_EN;
		else
			Value &= ~SBSS_TTTT0_TSF0_EN;

		MAC_IO_WRITE32(pAd->hdev_ctrl, first_reg_base, Value);
		MAC_IO_READ32(pAd->hdev_ctrl, bssid_reg_base, &Value);
		Value &= ~TTTT0_n_INT_EN;
		Value &= ~PRE_TTTT0_n_INT_EN;
		MAC_IO_WRITE32(pAd->hdev_ctrl, bssid_reg_base, Value);
	}
}

VOID MtfDmacSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	MtfAsicSetExtTTTTLPOffset(pAd, mbss_idx, enable);
}
#endif /* CONFIG_AP_SUPPORT */

#ifndef COMPOS_TESTMODE_WIN
VOID MtfSetTmrCal(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR TmrType,
	IN  UCHAR Channel,
	IN UCHAR Bw)
{
	UINT32  value = 0;

	MAC_IO_READ32(pAd->hdev_ctrl, TMAC_B0BRR0, &value);

	if (TmrType == TMR_DISABLE)
		/* Enanle Spatial Extension for ACK/BA/CTS after TMR Disable*/
		value |= BSSID00_RESP_SPE_EN;
	else
		/* Disable Spatial Extension for ACK/BA/CTS when TMR Enable*/
		value &= ~BSSID00_RESP_SPE_EN;

	MAC_IO_WRITE32(pAd->hdev_ctrl, TMAC_B0BRR0, value);
}
#endif

INT MtfAsicTOPInit(RTMP_ADAPTER *pAd)
{
	return TRUE;
}

INT mtf_asic_rts_on_off(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en)
{
	UINT32 cr;
	UINT32 value;

	cr = (band_idx == BAND1) ? BN1_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_ADDR :
		BN0_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_ADDR;
	value = (rts_len & BN0_WF_AGG_TOP_PCR1_RTS0_PKT_LEN_THRESHOLD_MASK) |
		(rts_num << BN1_WF_AGG_TOP_PCR1_RTS0_PKT_NUM_THRESHOLD_SHFT);

	MAC_IO_WRITE32(ad->hdev_ctrl, cr, value);
	return 0;
}

INT mtf_asic_set_agglimit(RTMP_ADAPTER *pAd, UCHAR band_idx, UCHAR ac, struct wifi_dev *wdev, UINT32 agg_limit)
{
	UINT32 cr;
	UINT32 value = 0;
	UINT8 wmm_set;

	if (wdev)
		wmm_set = HcGetWmmIdx(pAd, wdev);
	else
		return 1;

	cr = (band_idx == BAND1) ? BN1_WF_AGG_TOP_AALCR0_ADDR :
		BN0_WF_AGG_TOP_AALCR0_ADDR;

	cr += (wmm_set << 2);

	MAC_IO_READ32(pAd->hdev_ctrl, cr, &value);

	if ((value & (0x0FF << (ac << 3))) == ((agg_limit & 0x0FF) << (ac << 3)))
		return 0;

	value &= ~(0x0FF << (ac << 3));
	value |= ((agg_limit & 0x0FF) << (ac << 3));
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr, value);

	return 0;
}

INT mtf_asic_set_rts_retrylimit(RTMP_ADAPTER *pAd, UCHAR band_idx, UINT32 limit)
{
	UINT32 cr;
	UINT32 value = 0;

	cr = (band_idx == BAND1) ? BN1_WF_AGG_TOP_MRCR_ADDR :
		BN0_WF_AGG_TOP_MRCR_ADDR;

	MAC_IO_READ32(pAd->hdev_ctrl, cr, &value);

	if ((value & (0x01F << 7)) == ((limit & 0x01F) << 7))
		return 0;

	value &= ~(0x01F << 7);
	value |= ((limit & 0x01F) << 7);
	MAC_IO_WRITE32(pAd->hdev_ctrl, cr, value);

	return 0;
}
#ifdef DOT11_VHT_AC
INT mtf_asic_set_rts_signal_ta(RTMP_ADAPTER *ad, UINT8 band_idx, BOOLEAN enable)
{
	UINT32 tcr_value = 0, ducr0_value = 0;
	UINT32 rts_sig_mask = 0, opt_rts_data_mask = 0;

	if (band_idx) {
		MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_TCR0_ADDR, &tcr_value);
		rts_sig_mask = BN1_WF_TMAC_TOP_TCR0_RTS_SIGTA_EN_MASK;
		MAC_IO_READ32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_DUCR0_ADDR, &ducr0_value);
		opt_rts_data_mask = BN1_WF_TMAC_TOP_DUCR0_OPT_DUR_RTS_DATA_MASK;
	} else {
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TCR0_ADDR, &tcr_value);
		rts_sig_mask = BN0_WF_TMAC_TOP_TCR0_RTS_SIGTA_EN_MASK;
		MAC_IO_READ32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_DUCR0_ADDR, &ducr0_value);
		opt_rts_data_mask = BN0_WF_TMAC_TOP_DUCR0_OPT_DUR_RTS_DATA_MASK;
	}

	/* If enable RTS signaling then use duration mode 1 for PPDU protection */
	if (enable) {
		tcr_value |= rts_sig_mask;
		ducr0_value |= opt_rts_data_mask;
	} else {
		tcr_value &= ~(rts_sig_mask);
		ducr0_value &= ~(opt_rts_data_mask);
	}

	if (band_idx) {
		MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_TCR0_ADDR, tcr_value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN1_WF_TMAC_TOP_DUCR0_ADDR, ducr0_value);
	} else {
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_TCR0_ADDR, tcr_value);
		MAC_IO_WRITE32(ad->hdev_ctrl, BN0_WF_TMAC_TOP_DUCR0_ADDR, ducr0_value);
	}

	return TRUE;
}
#endif

#ifdef AIR_MONITOR
void apply_mntr_ruleset_smesh(PRTMP_ADAPTER pAd, EXT_CMD_SMESH_T *pconfig_smesh)
{
	if (pAd->MntRuleBitMap & RULE_CTL)
		pconfig_smesh->fgSmeshRxCtrl = 1;
	else
		pconfig_smesh->fgSmeshRxCtrl = 0;

	if (pAd->MntRuleBitMap & RULE_MGT)
		pconfig_smesh->fgSmeshRxMgnt = 1;
	else
		pconfig_smesh->fgSmeshRxMgnt = 0;

	if (pAd->MntRuleBitMap & RULE_DATA)
		pconfig_smesh->fgSmeshRxData = 1;
	else
		pconfig_smesh->fgSmeshRxData = 0;

	if (pAd->MntRuleBitMap & RULE_A1)
		pconfig_smesh->fgSmeshRxA1 = 1;
	else
		pconfig_smesh->fgSmeshRxA1 = 0;

	if (pAd->MntRuleBitMap & RULE_A2)
		pconfig_smesh->fgSmeshRxA2 = 1;
	else
		pconfig_smesh->fgSmeshRxA2 = 0;
}

INT mtf_set_air_monitor_enable(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR band_idx)
{
	UCHAR flush_all = FALSE;
	UINT32 i = 0;
	BOOLEAN bSMESHEn = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;
	EXT_CMD_SMESH_T *pconfig_smesh = NULL;
	EXT_EVENT_SMESH_T rSmeshResult;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;
	UCHAR *pdata_muar = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UNI_CMD_SMESH_PARAM_T rSmeshGetResult;

	NdisZeroMemory(&rSmeshGetResult, sizeof(UNI_CMD_SMESH_PARAM_T));
#endif /* WIFI_UNIFIED_COMMAND */
	NdisZeroMemory(&rSmeshResult, sizeof(EXT_EVENT_SMESH_T));
	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata_muar,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	if (!pdata_muar)
		return FALSE;

	os_alloc_mem(pAd, (UCHAR **)&pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	if (!pconfig_smesh) {
		os_free_mem(pdata_muar);
		return FALSE;
	}

	NdisZeroMemory(pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "-->\n");

	pconfig_smesh->ucBand = band_idx;
	pconfig_smesh->ucAccessMode = 0;
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
		if (rSmeshGetResult.ucEntryEnable) {
			bSMESHEn = TRUE;
			flush_all = TRUE;
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
		if ((rSmeshResult.u4SmeshVal & SMESH_ADDR_EN)) {
			bSMESHEn = TRUE;
			flush_all = TRUE;
		}
	}


	if (bSMESHEn != enable) {
		if (enable == 0) {
			if (flush_all == TRUE) {
				for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
					pMuarGroup = &pAd->MntGroupTable[i];

					if (pMuarGroup->bValid && (pMuarGroup->Band == band_idx))
						NdisZeroMemory(pMuarGroup, sizeof(*pMuarGroup));
				}

				for (i = 0; i < MAX_NUM_OF_MONITOR_STA; i++) {

					pMntEntry = &pAd->MntTable[i];

					if (!pMntEntry->bValid || (pMntEntry->Band != band_idx))
						continue;


					muar_entry.ucMuarIdx = pMntEntry->muar_idx;
					config_muar.ucAccessMode = MUAR_WRITE;
					config_muar.ucMuarModeSel = MUAR_REPEATER;
					config_muar.ucEntryCnt = 1;
					config_muar.ucBand = band_idx;

					NdisMoveMemory(pdata_muar, (UCHAR *)&config_muar, sizeof(EXT_CMD_MUAR_T));
					NdisMoveMemory(pdata_muar + sizeof(EXT_CMD_MUAR_T),
								   (UCHAR *)&muar_entry,
								   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
#ifdef WIFI_UNIFIED_COMMAND
					if (cap->uni_cmd_support)
						UniCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar, TRUE, FALSE);
					else
#endif /* WIFI_UNIFIED_COMMAND */
						MtCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar);

					pMacEntry = pMntEntry->pMacEntry;

					if (pMacEntry) {
						if (band_idx == BAND1)
							pMacEntry->mnt_band &= ~MNT_BAND1;
				else if (band_idx == BAND0)
					pMacEntry->mnt_band &= ~MNT_BAND0;
				else
					pMacEntry->mnt_band &= ~MNT_BAND2;

						if (pMacEntry->mnt_band == 0) { /* no more use for other band */
							MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC,
								DBG_LVL_ERROR,
								"call MacTableDeleteEntry");
							MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC,
								DBG_LVL_ERROR,
								"(WCID=%d)- "MACSTR"\n",
								pMacEntry->wcid,
								MAC2STR(pMacEntry->Addr));
							MacTableDeleteEntry(pAd,
								pMacEntry->wcid, pMacEntry->Addr);
						}
					}

					pAd->MonitrCnt--;
					NdisZeroMemory(pMntEntry, sizeof(*pMntEntry));
				}

				pconfig_smesh->ucBand = band_idx;
				pconfig_smesh->ucAccessMode = 1;
				pconfig_smesh->ucSmeshEn = 0;
#ifdef WIFI_UNIFIED_COMMAND
				if (cap->uni_cmd_support)
					MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
				else
#endif /* WIFI_UNIFIED_COMMAND */
					MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);

			if (pAd->MonitrCnt == 0) {
				pAd->MntEnable = 0;
				NdisZeroMemory(&pAd->MntTable, sizeof(MNT_STA_ENTRY));
				NdisZeroMemory(&pAd->MntGroupTable, sizeof(MNT_MUAR_GROUP));
			}
		}
		} else {
			pconfig_smesh->ucBand = band_idx;
			pconfig_smesh->ucAccessMode = 1;
			pconfig_smesh->ucSmeshEn = enable;
			apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);

		}
	}

	os_free_mem(pconfig_smesh);
	os_free_mem(pdata_muar);


	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "<--\n");
	return TRUE;
}

INT mtf_set_air_monitor_rule(struct _RTMP_ADAPTER *pAd, UCHAR *rule, UCHAR band_idx)
{
	INT ret = TRUE;
	EXT_EVENT_SMESH_T rSmeshResult;
	EXT_CMD_SMESH_T *pconfig_smesh = NULL;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UNI_CMD_SMESH_PARAM_T rSmeshGetResult;
#endif /* WIFI_UNIFIED_COMMAND */

	if (!rule)
		return FALSE;

	os_alloc_mem(pAd, (UCHAR **)&pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	if (!pconfig_smesh)
		return FALSE;

	MTWF_PRINT("--> %s()\n", __func__);

	NdisZeroMemory(pconfig_smesh, sizeof(EXT_CMD_SMESH_T));

	pAd->MntRuleBitMap &= ~RULE_CTL;
	pAd->MntRuleBitMap &= ~RULE_MGT;
	pAd->MntRuleBitMap &= ~RULE_DATA;
	pAd->MntRuleBitMap |= (RULE_DATA & ((UINT32)rule[0] << RULE_DATA_OFFSET))
					| (RULE_MGT & ((UINT32)rule[1] << RULE_MGT_OFFSET))
					| (RULE_CTL & ((UINT32)rule[2] << RULE_CTL_OFFSET));
#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd) && IS_MAP_TURNKEY_ENABLE(pAd)) {
		pAd->MntRuleBitMap &= ~RULE_A1;
	}
#endif
#ifdef RT_CFG80211_SUPPORT
#endif
	if (band_idx == DBDC_BAND0) {
		pconfig_smesh->ucBand = BAND0;
		pconfig_smesh->ucAccessMode = 1;
		pconfig_smesh->ucSmeshEn = 1;
		apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
	} else if (band_idx == DBDC_BAND1) {
		pconfig_smesh->ucBand = BAND1;
		pconfig_smesh->ucAccessMode = 1;
		pconfig_smesh->ucSmeshEn = 1;
		apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
	} else if (band_idx == DBDC_BAND2) {
		pconfig_smesh->ucBand = BAND2;
		pconfig_smesh->ucAccessMode = 1;
		pconfig_smesh->ucSmeshEn = 1;
		apply_mntr_ruleset_smesh(pAd, pconfig_smesh);
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			MtUniCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshGetResult);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdSmeshConfigSet(pAd, (UCHAR *)pconfig_smesh, &rSmeshResult);
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR, "wrong band index(%d)\n",
				 band_idx);
		os_free_mem(pconfig_smesh);
		return FALSE;
	}

	MTWF_PRINT("<-- %s()\n", __func__);

	os_free_mem(pconfig_smesh);


	return ret;
}


INT mtf_set_air_monitor_idx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR mnt_idx, UCHAR band_idx)
{
	INT ret, i;
	UCHAR *p = ZERO_MAC_ADDR, muar_idx = 0, muar_group_base = 0;
	BOOLEAN bCreate = FALSE;
	MNT_STA_ENTRY *pMntEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	MNT_MUAR_GROUP *pMuarGroup = NULL;
	UCHAR *pdata_muar = NULL;
	EXT_CMD_MUAR_T config_muar;
	EXT_CMD_MUAR_MULTI_ENTRY_T muar_entry;
	UCHAR mnt_enable = 0;
	UCHAR totalMonitrCnt = 0, value = 0, remainder = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	NdisZeroMemory(&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisZeroMemory(&muar_entry, sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	os_alloc_mem(pAd,
				 (UCHAR **)&pdata_muar,
				 sizeof(EXT_CMD_MUAR_T) + sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));
	if (pdata_muar == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"Allocate memory failed!\n");
		return FALSE;
	}


	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "-->\n");


	if (mnt_idx < MAX_NUM_OF_MONITOR_STA) {
		pAd->MntIdx = mnt_idx;
		pMntEntry = &pAd->MntTable[mnt_idx];
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				 "The index is over the maximum limit.\n");
		os_free_mem(pdata_muar);
		return FALSE;
	}

	if (MAC_ADDR_EQUAL(ZERO_MAC_ADDR, pAd->curMntAddr)) {
		if (pMntEntry->bValid) {
			pMacEntry = pMntEntry->pMacEntry;

			if (pMacEntry) {
				switch (band_idx) {
				case BAND2:
					pMacEntry->mnt_band &= ~MNT_BAND2;
					break;
				case BAND1:
					pMacEntry->mnt_band &= ~MNT_BAND1;
					break;
				case BAND0:
				default:
					pMacEntry->mnt_band &= ~MNT_BAND0;
					break;
				}

				if (pMacEntry->mnt_band == 0) /* no more use for other band */
					MacTableDeleteEntry(pAd, pMacEntry->wcid, pMacEntry->Addr);
			}

			if (pMntEntry->muar_group_idx < MAX_NUM_OF_MONITOR_GROUP)
				pMuarGroup = &pAd->MntGroupTable[pMntEntry->muar_group_idx];

			if (pMuarGroup) {
				pMuarGroup->Count--;

				if (pMuarGroup->Count == 0)
					pMuarGroup->bValid = FALSE;
			}

			pMntEntry->bValid = FALSE;

			if (pAd->MonitrCnt > 0)
				pAd->MonitrCnt--;

			muar_idx = pMntEntry->muar_idx;
		} else {
			os_free_mem(pdata_muar);
			return TRUE;
		}
	} else {
		if (pMntEntry->bValid) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
					 "The index of existed monitor entry.\n");
			os_free_mem(pdata_muar);
			return TRUE;
		}

		/*Comments: 7915 dual band have 2 seperated MUAR table, can monitor 16 + 16 STAs.*/
		if ((pAd->MonitrCnt) >= MAX_NUM_OF_MONITOR_STA) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
					 "The monitor number extends to maximum limit(%d).\n", MAX_NUM_OF_MONITOR_STA);
			os_free_mem(pdata_muar);
			return FALSE;
		}
		/*muar_group_idx: 	0	1	2	3	4	5	6	7*/
		/*MUAR Index:		32	36	40	44	48	52	56	60*/
		/*					34	38	42	46	50	54	58	62*/
		for (i = 0; i < MAX_NUM_OF_MONITOR_GROUP; i++) {
			pMuarGroup = &pAd->MntGroupTable[i];

			if (!pMuarGroup->bValid) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMuarGroup->MuarGroupBase = MONITOR_MUAR_BASE_INDEX + i * MAX_NUM_PER_GROUP;
				pMuarGroup->bValid = TRUE;
				pMuarGroup->Band = band_idx;
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + i * 2);
				pMuarGroup->Count++;
				muar_idx = pMntEntry->muar_idx;
				muar_group_base = pMuarGroup->MuarGroupBase;
				bCreate = TRUE;
				break;
			} else if ((pMuarGroup->Count < MAX_NUM_PER_GROUP) &&
					   (pMuarGroup->Band == band_idx)) {
				NdisZeroMemory(pMntEntry, sizeof(MNT_STA_ENTRY));
				pMntEntry->muar_group_idx = i;
				pMntEntry->muar_idx = (pMuarGroup->MuarGroupBase + 2 * (i + 1));
				pMuarGroup->Count++;
				muar_idx = pMntEntry->muar_idx;
				muar_group_base = pMuarGroup->MuarGroupBase;
				bCreate = TRUE;
				break;
			}
		}

		if (bCreate) {
			COPY_MAC_ADDR(pMntEntry->addr, pAd->curMntAddr);
			pMacEntry = MacTableLookup(pAd, pMntEntry->addr);

			if (pMacEntry == NULL) {
				pMacEntry = MacTableInsertEntry(
								pAd,
								pMntEntry->addr,
								wdev,
								ENTRY_CAT_MONITOR,
								OPMODE_STA,
								TRUE,
								MLD_STA_NONE,
								NULL);

				if (wdev_do_conn_act(wdev, pMacEntry) != TRUE) {
					MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
						"connect fail!!\n");
					os_free_mem(pdata_muar);
					return FALSE;
				}
			}

			if (pMacEntry) {
				p = pMntEntry->addr;
				pAd->MonitrCnt++;
				pMntEntry->bValid = TRUE;
				pMntEntry->Band = band_idx;
				pMacEntry->mnt_idx = mnt_idx;
				NdisAcquireSpinLock(pAd->MacTabLock);
				SET_ENTRY_MONITOR(pMacEntry);

				switch (band_idx) {
				case BAND2:
					pMacEntry->mnt_band |= MNT_BAND2;
					break;
				case BAND1:
					pMacEntry->mnt_band |= MNT_BAND1;
					break;
				case BAND0:
				default:
					pMacEntry->mnt_band |= MNT_BAND0;
					break;
				}
				pMntEntry->pMacEntry = pMacEntry;
				NdisReleaseSpinLock(pAd->MacTabLock);
			}
		} else {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
					 "Can't create a monitor entry!\n");

			os_free_mem(pdata_muar);
			return FALSE;
		}
	}

	MTWF_PRINT("index: %d\n", mnt_idx);
	MTWF_PRINT("entry: "MACSTR"\n", MAC2STR(p));

	muar_entry.ucMuarIdx = muar_idx;
	NdisMoveMemory(muar_entry.aucMacAddr, p, MAC_ADDR_LEN);
	config_muar.ucAccessMode = MUAR_WRITE;
	config_muar.ucMuarModeSel = MUAR_REPEATER;
	config_muar.ucEntryCnt = 1;
	config_muar.ucBand = band_idx;

	NdisMoveMemory(pdata_muar, (UCHAR *)&config_muar, sizeof(EXT_CMD_MUAR_T));
	NdisMoveMemory(pdata_muar + sizeof(EXT_CMD_MUAR_T),
				   (UCHAR *)&muar_entry,
				   sizeof(EXT_CMD_MUAR_MULTI_ENTRY_T));

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar, TRUE, TRUE);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdMuarConfigSet(pAd, (UCHAR *)pdata_muar);

	/*total 16 STA, 1 otects control CR 820e5048[7:0]*/
	/*MonitrCnt:		1,2  3,4  5,6  7,8	9,10   11,12  13,14	15,16*/
	/*Bit:				 0	  1    2	3	 4		5	   6	  7*/
	/*MUAR Index:		32	 36   40	44	 48 	52	   56	  60*/
	/*					34	 38   42	46	 50 	54	   58	  62*/
	totalMonitrCnt = pAd->MonitrCnt;
	if (totalMonitrCnt > 0) {
		value = (totalMonitrCnt / 2);
		remainder = (totalMonitrCnt % 2);
		if (remainder == 1) {
			pAd->MntEnable |= (1 << value);
		}
	}
	mnt_enable = pAd->MntEnable;
	ret = mtf_set_air_monitor_enable(pAd, mnt_enable, band_idx);

	MTWF_PRINT("<-- %s()[Band%d]MntEnable=0x%x\n", __func__, band_idx, mnt_enable);

	os_free_mem(pdata_muar);
	return ret;
}
#endif
