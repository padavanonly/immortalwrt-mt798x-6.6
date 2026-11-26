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
 ****************************************************************************

	Module Name:
	cmm_cfg.c

	Abstract:
	Ralink WiFi Driver configuration related subroutines

	Revision History:
	Who		  When		  What
	---------	----------	----------------------------------------------
*/

#include "rt_config.h"
#include "mac_mt/fmac/mt_fmac.h"
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#include "common/config_internal.h"
#endif /*TR181_SUPPORT*/
#ifdef MAP_R2
#include "map.h"
#endif
#ifdef FT_R1KH_KEEP
#define RADIO_ON_SET	1
#define RADIO_ON_RESET	2
#endif /* FT_R1KH_KEEP */
#define MSG_LEN 2048
static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd);
RTMP_STRING *wdev_type2str(int type);
BOOLEAN g_fgCommand;
#ifdef DATA_TXPWR_CTRL
UINT8 g_u1MinPwrlimit[MAX_BAND_NUM];
UINT8 g_u1MinPwrlimitperRate[MAX_BAND_NUM][DATA_TXPOWER_MAX_BW_NUM][DATA_TXPOWER_MAX_MCS_NUM];
#endif


INT ComputeChecksum(UINT PIN)
{
	INT digit_s;
	UINT accum = 0;
	INT ret;

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10);
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10);
	accum += 3 * ((PIN / 1000) % 10);
	accum += 1 * ((PIN / 100) % 10);
	accum += 3 * ((PIN / 10) % 10);
	digit_s = (accum % 10);
	ret = ((10 - digit_s) % 10);
	return ret;
} /* ComputeChecksum*/

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
	IN  BOOLEAN		 bFromApcli,
	IN	UCHAR			apidx)
{
	UCHAR	macAddr[MAC_ADDR_LEN];
	UINT	iPin;
	UINT	checksum;
	NdisZeroMemory(macAddr, MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (bFromApcli)
			NdisMoveMemory(&macAddr[0], pAd->StaCfg[apidx].wdev.if_addr, MAC_ADDR_LEN);
		else
#endif /* APCLI_SUPPORT */
			NdisMoveMemory(&macAddr[0], pAd->ApCfg.MBSSID[apidx].wdev.if_addr, MAC_ADDR_LEN);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	NdisMoveMemory(&macAddr[0], pAd->StaCfg[0].wdev.if_addr, MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */
	iPin = macAddr[0] * 256 * 256 + macAddr[4] * 256 + macAddr[5];
	iPin = iPin % 10000000;
	checksum = ComputeChecksum(iPin);
	iPin = iPin * 10 + checksum;
	return iPin;
}

static char *phy_mode_str[] = {"CCK", "OFDM", "HT_MM", "HT_GF", "VHT", "HE",
				"EHT", "HE2G", "HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU", "NA",
				"EHT_ER_SU", "EHT_TRIG", "EHT_MU"};

char *get_phymode_str(int Mode)
{
	if (Mode >= MODE_CCK && Mode <= MODE_EHT_MU)
		return phy_mode_str[Mode];
	else
		return "N/A";
}

char *get_gi_str(int mode, int gi)
{
	if (mode < MODE_HTMIX)
		return "NA";

	switch(gi) {
	case 0:
		return "0.8us";
	case 1:
		return mode < MODE_HE ? "0.4us" : "1.6us";
	case 2:
		return mode >= MODE_HE ? "3.2us" : "NA";
	default:
		return "NA";
	}
}


/*
	==========================================================================
	Description:
	Set Country Region to pAd->CommonCfg.CountryRegion.
	This command will not work, if the field of CountryRegion in eeprom is programmed.

	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
INT RT_CfgSetCountryRegion(RTMP_ADAPTER *pAd, RTMP_STRING *arg, INT band)
{
	LONG region;
	UCHAR *pCountryRegion;

	region = os_str_tol(arg, 0, 10);

	if (band == BAND_24G)
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
	else
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;

	/*
		   1. If this value is set before interface up, do not reject this value.
		   2. Country can be set only when EEPROM not programmed
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS) && (*pCountryRegion & EEPROM_IS_PROGRAMMED)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			 "CfgSetCountryRegion():CountryRegion in eeprom was programmed\n");
		return FALSE;
	}

	if ((region >= 0) &&
	    (((band == BAND_24G) && ((region <= REGION_MAXIMUM_BG_BAND) ||
				     (region == REGION_31_BG_BAND) || (region == REGION_32_BG_BAND) || (region == REGION_33_BG_BAND))) ||
	     ((band == BAND_5G) && (region <= REGION_MAXIMUM_A_BAND)))
	   )
		*pCountryRegion = (UCHAR) region;
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"CfgSetCountryRegion():region(%ld) out of range!\n", region);
		return FALSE;
	}

	return TRUE;
}

VOID CfgSetFreqList(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char *pch;
	int start = 0, end = 0, count = 0;
	int freq, i;
	char buf[300] = {0};
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	int ret;

	if (pChCtrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "pChCtrl is NULL\n");
		return;
	}

	ret = snprintf(buf, sizeof(buf), "%s", arg);
	if (os_snprintf_error(sizeof(buf), ret)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "snprintf error\n");
		return;
	}

	pch = rstrtok(buf, ",");

	while (pch != NULL && count < MAX_NUM_OF_CHANNELS) {
		if (sscanf(pch, "%d-%d", &start, &end) == 2) {
			if (start < end) {
				if (IS_BETWEEN_24G_FREQ(start, end)) {
					for (freq = start; freq <= end && count < MAX_NUM_OF_CHANNELS; freq = freq + 5)
						pChCtrl->FreqList[count++] = freq;
				} else if ((IS_BETWEEN_5G_A_BAND_FREQ(start, end)) ||
					(IS_BETWEEN_5G_B_BAND_FREQ(start, end)) ||
					(IS_BETWEEN_5G_C_BAND_FREQ(start, end)) ||
					(IS_BETWEEN_6G_FREQ(start, end))) {
					for (freq = start; freq <= end && count < MAX_NUM_OF_CHANNELS; freq = freq + 20)
						pChCtrl->FreqList[count++] = freq;
					} else
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_WARN,
						"ignored, invalid freqList range\n");
			}
		} else if (sscanf(pch, "%d", &start) == 1) {
			if ((IS_24G_FREQ(start)) || (IS_5G_FREQ(start)) || (IS_6G_FREQ(start)))
				pChCtrl->FreqList[count++] = start;
			}

		pch = rstrtok(NULL, ",");
	}

	pChCtrl->FreqListNum = count;
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"FreqListNum = %d\n", pChCtrl->FreqListNum);

	for (i = 0; (i < pChCtrl->FreqListNum) && (pChCtrl->FreqListNum <= MAX_NUM_OF_CHANNELS); i++) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"Freq = %d, channel = %d\n", pChCtrl->FreqList[i],
			FreqToChannel(pChCtrl->FreqList[i]));
	}
}

/*
	==========================================================================
	Description:
	set channel priority, in struct of "start channel, end channel, priority".

	Return:
	NULL
	==========================================================================
*/
VOID ParseChPriority(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int chStart = 0, chEnd = 0, priority = 0xFF, parsecnt = 0, length = 0;
	RTMP_STRING *segPtr = NULL, parsingSeg[15], endMark;
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	struct Chan_Prio tempChPrio[MAX_NUM_OF_CHANNELS] = {0};
	struct Chan_Config *pChCfg = NULL;

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "can't find pChCtrl !\n");
		return;
	}

	pChCfg = &pChCtrl->ch_cfg;

	/*1.Set chan_prio_valid to FALSE,
	ensure that it is in an invalid state*/
	pChCfg->chan_prio_valid = FALSE;

	/*2.separate each segment, save to array.*/
	/*eg. "36-48:1,52-64:3,100-112:4,144:0" to"36-48:1";"52-64:3";"100-112:4";"144:0"*/
	for (parsecnt = 0, segPtr = rstrtok(arg, ","); (segPtr != NULL) && (parsecnt < MAX_NUM_OF_CHANNELS);
		segPtr = rstrtok(NULL, ","), parsecnt++) {

		chStart = 0;
		chEnd = 0;
		priority = CHAN_PRIO_UNSPECIFIED;
		length = (int) strlen(segPtr);
		endMark = '\0';

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
					"[ch prio]= parsing segment [%s]\n", segPtr);

		/*segment length check*/
		if (length > 10 || length <= 0) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"[ch prio]Error, %s is wrong segment length\n", segPtr);
			return;
		}

		/*add end mark to detect segment error like "36-48:3%"*/
		NdisCopyMemory(parsingSeg, segPtr, length + 1);
		parsingSeg[length] = ',';
		parsingSeg[length + 1] = '\0';

		/*3.parse a range of channel priorities*/
		/*3.1 parse segment with multiple channels*/
		if (sscanf(parsingSeg, "%d-%d:%d%c", &chStart, &chEnd, &priority, &endMark) == 4) {

			/*Legality check"*/
			/*1)end mark check*/
			if (endMark != ',') {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"[ch prio]Error, %s is wrong segment end mark\n", segPtr);
				return;
			}

			/*2)check if ch order is invalid */
			if (chStart > chEnd || chEnd <= 0 || chStart <= 0 || chEnd > CHANNEL_6G_MAX) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"[ch prio]Error,wrong ch num order  %d-%d\n", chStart, chEnd);
				return;
			}

			/*3)check if the channel range and priority is valid*/
			if (priority > CHAN_PRIO_MAXIMUM || priority < CHAN_PRIO_UNAVAILABLE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"[ch prio]Error,priority %d is invalid\n", priority);
				return;
			}

			/*save the channel range and priority*/
			tempChPrio[parsecnt].start_ch = chStart;
			tempChPrio[parsecnt].end_ch = chEnd;
			tempChPrio[parsecnt].prio = priority;

			/*3.2 parse segment with only one channel*/
		} else if (sscanf(parsingSeg, "%d:%d%c", &chStart, &priority, &endMark) == 3) {

			/*Legality check"*/
			/*1)end mark check*/
			if (endMark != ',') {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"[ch prio]Error, %s is wrong segment end mark\n", segPtr);
				return;
			}

			/*2)channel num check*/
			if (chStart <= 0 || chStart > CHANNEL_6G_MAX) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
					"[ch prio]Error,meaningless ch %d\n", chStart);
				return;
			}

			/*3)check if the channel range and priority is valid*/
			if (priority > CHAN_PRIO_MAXIMUM || priority < CHAN_PRIO_UNAVAILABLE) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
						"[ch prio]Error,priority %d is invalid\n", priority);
				return;
			}

			/*the parameters are all valid, save it*/
			tempChPrio[parsecnt].start_ch = chStart;
			tempChPrio[parsecnt].end_ch = chStart;
			tempChPrio[parsecnt].prio = priority;

		/*3.3 if can't parse segment*/
		} else {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"[ch prio]Error,can't parse segment %s\n", segPtr);
			return;
		}

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
		"[ch prio]parse cuccess. chStart is %d chEnd is %d prio is %d\n",
		tempChPrio[parsecnt].start_ch, tempChPrio[parsecnt].end_ch,
		tempChPrio[parsecnt].prio);
	}

	/*check if segment number is more than
	MAX_NUM_OF_CHANNELS or valid segment is empty*/
	if ((parsecnt >= MAX_NUM_OF_CHANNELS && segPtr != NULL) || (parsecnt == 0 && segPtr == NULL)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"[ch prio]Error,segment number is out range or valid segment number is 0\n");
		return;
	}

	/*4.all segments have been parsed, now we can set the channel prio,
	and set chan_prio_valid to true*/
	memcpy(pChCfg->ch_prio, tempChPrio, sizeof(struct Chan_Prio) * parsecnt);
	pChCfg->chan_prio_valid = TRUE;
}

/*
	==========================================================================
	Description:
	set Forbid_BW40,
	1 ----- forbid BW40;
	0 ----- allow BW40;

	Return:
	NULL
	==========================================================================
*/
VOID ParseForbidBW40(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
	struct Chan_Config *pChCfg = NULL;
	long val = 0;

	if (!pChCtrl) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
			"[Forbid BW40]can't find pChCtrl !\n");
		return;
	}

	pChCfg = &pChCtrl->ch_cfg;

	if (kstrtol(arg, 10, &val) == 0) {
		pChCfg->bw40_forbid = val > 0 ? TRUE : FALSE;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
				"[Forbid BW40]bw40_forbid is %ld\n", val);
	} else
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				"[Forbid BW40]Error,can't parse segment %s\n", arg);
}

static USHORT CFG_WMODE_MAP[] = {
	PHY_11BG_MIXED, (WMODE_B | WMODE_G), /* 0 => B/G mixed */
	PHY_11B, (WMODE_B), /* 1 => B only */
	PHY_11A, (WMODE_A), /* 2 => A only */
	PHY_11ABG_MIXED, (WMODE_A | WMODE_B | WMODE_G), /* 3 => A/B/G mixed */
	PHY_11G, WMODE_G, /* 4 => G only */
	PHY_11ABGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 5 => A/B/G/GN/AN mixed */
	PHY_11N_2_4G, (WMODE_GN), /* 6 => N in 2.4G band only */
	PHY_11GN_MIXED, (WMODE_G | WMODE_GN), /* 7 => G/GN, i.e., no CCK mode */
	PHY_11AN_MIXED, (WMODE_A | WMODE_AN), /* 8 => A/N in 5 band */
	PHY_11BGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN), /* 9 => B/G/GN mode*/
	PHY_11AGN_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 10 => A/AN/G/GN mode, not support B mode */
	PHY_11N_5G, (WMODE_AN), /* 11 => only N in 5G band */
#ifdef DOT11_VHT_AC
	PHY_11VHT_N_ABG_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC), /* 12 => B/G/GN/A/AN/AC mixed*/
	PHY_11VHT_N_AG_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN | WMODE_AC), /* 13 => G/GN/A/AN/AC mixed, no B mode */
	PHY_11VHT_N_A_MIXED, (WMODE_A | WMODE_AN | WMODE_AC), /* 14 => A/AC/AN mixed */
	PHY_11VHT_N_MIXED, (WMODE_AN | WMODE_AC), /* 15 => AC/AN mixed, but no A mode */
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
	PHY_11AX_24G, (WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G),
	PHY_11AX_5G, (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G),
	PHY_11AX_6G, (WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G),
	PHY_11AX_24G_6G, (WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_AX_6G),
	PHY_11AX_5G_6G, (WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G),
	PHY_11AX_24G_5G_6G, (WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_A |
						 WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G),
#endif /*DOT11_HE_AX*/
#ifdef DOT11_EHT_BE
	PHY_11BE_24G,
	(WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_BE_24G),
	PHY_11BE_5G,
	(WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_BE_5G),
	PHY_11BE_6G,
	(WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G | WMODE_BE_6G),
	PHY_11BE_24G_6G,
	(WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_AX_6G
	| WMODE_BE_24G | WMODE_BE_6G),
	PHY_11BE_5G_6G,
	(WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G
	| WMODE_BE_5G | WMODE_BE_6G),
	PHY_11BE_24G_5G_6G,
	(WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_A
	| WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_AX_6G
	| WMODE_BE_24G | WMODE_BE_5G | WMODE_BE_6G),
#endif /* DOT11_EHT_BE */

	PHY_MODE_MAX, WMODE_INVALID /* default phy mode if not match */
};

static RTMP_STRING *BAND_STR[] = {"Invalid", "2.4G", "5G", "2.4G/5G"};
static RTMP_STRING *WMODE_STR[] = {"", "A", "B", "G", "gN", "aN", "AC", "ax_24g", "ax_5g", "ax_6g", "be_24g", "be_5g", "be_6g"};
char iee80211_str[PHY_MODE_MAX][IFNAMSIZ] = {"IEEE 802.11bg", "IEEE 802.11b", "IEEE 802.11a", "IEEE 802.11abg",
"IEEE 802.11g", "IEEE 802.11abgn", "IEEE 802.11n",  "IEEE 802.11gn","IEEE 802.11an","IEEE 802.11bgn",
"IEEE 802.11agn", "IEEE 802.11n", "IEEE 802.11nabg", "IEEE 802.11nag", "IEEE 802.11acan", "IEEE 802.11acan",
"IEEE 802.11ax", "IEEE 802.11ax", "IEEE 802.11ax", "IEEE 802.11ax", "IEEE 802.11ax", "IEEE 802.11ax",
"IEEE 802.11be", "IEEE 802.11be", "IEEE 802.11be", "IEEE 802.11be", "IEEE 802.11be", "IEEE 802.11be"};


UCHAR *wmode_2_str(USHORT wmode)
{
	UCHAR *str;
	INT idx, pos, max_len;

	max_len = WMODE_COMP * 3;

	if (os_alloc_mem(NULL, &str, max_len) == NDIS_STATUS_SUCCESS) {
		NdisZeroMemory(str, max_len);
		pos = 0;

		for (idx = 0; idx < WMODE_COMP; idx++) {
			if (wmode & (1 << idx)) {
				if ((strlen(str) +  strlen(WMODE_STR[idx + 1])) >= (max_len - 1))
					break;

				if (strlen(str)) {
					NdisMoveMemory(&str[pos], "/", 1);
					pos++;
				}

				NdisMoveMemory(&str[pos], WMODE_STR[idx + 1], strlen(WMODE_STR[idx + 1]));
				pos += strlen(WMODE_STR[idx + 1]);
			}

			if (strlen(str) >= max_len)
				break;
		}

		return str;
	} else
		return NULL;
}

RT_802_11_PHY_MODE wmode_2_cfgmode(USHORT wmode)
{
	INT i, mode_cnt = sizeof(CFG_WMODE_MAP) / (sizeof(USHORT) * 2);

	for (i = 0; i < mode_cnt; i++) {
		if (CFG_WMODE_MAP[i * 2 + 1] == wmode)
			return CFG_WMODE_MAP[i * 2];
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR, "Cannot get cfgmode by wmode(%x)\n",
			wmode);
	return 0;
}

USHORT cfgmode_2_wmode(UCHAR cfg_mode)
{
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "cfg_mode=%d\n", cfg_mode);

	if (cfg_mode >= PHY_MODE_MAX)
		cfg_mode =  PHY_MODE_MAX;

	return CFG_WMODE_MAP[cfg_mode * 2 + 1];
}

BOOLEAN wmode_valid_and_correct(RTMP_ADAPTER *pAd, USHORT *wmode)
{
	BOOLEAN ret = TRUE;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (*wmode == WMODE_INVALID)
		*wmode = (WMODE_B | WMODE_G | WMODE_GN |
				WMODE_A | WMODE_AN | WMODE_AC |
				WMODE_AX_5G | WMODE_AX_24G | WMODE_AX_6G |
				WMODE_BE_24G | WMODE_BE_5G | WMODE_BE_6G);

	while (1) {
		if (WMODE_CAP_6G(*wmode) && (!PHY_CAP_6G(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_AX_6G | WMODE_BE_6G);
		else if (WMODE_CAP_5G(*wmode) && (!PHY_CAP_5G(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_A | WMODE_AN | WMODE_AC | WMODE_AX_5G | WMODE_BE_5G);
		else if (WMODE_CAP_2G(*wmode) && (!PHY_CAP_2G(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_B | WMODE_G | WMODE_GN | WMODE_AX_24G | WMODE_BE_24G);
		else if (WMODE_CAP_N(*wmode) && ((!PHY_CAP_N(cap->phy_caps)) ||
					RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N)))
			*wmode = *wmode & ~(WMODE_GN | WMODE_AN);
		else if (WMODE_CAP_AC(*wmode) && (!PHY_CAP_AC(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_AC);
		else if (WMODE_CAP_AX(*wmode) && (!PHY_CAP_AX(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_AX_24G | WMODE_AX_5G | WMODE_AX_6G);
		else if (WMODE_CAP_BE(*wmode) && (!PHY_CAP_BE(cap->phy_caps)))
			*wmode = *wmode & ~(WMODE_BE_24G | WMODE_BE_5G | WMODE_BE_6G);

		if (*wmode == 0) {
			*wmode = (WMODE_B | WMODE_G | WMODE_GN |
					WMODE_A | WMODE_AN | WMODE_AC |
					WMODE_AX_24G | WMODE_AX_5G | WMODE_AX_6G |
					WMODE_BE_24G | WMODE_BE_5G | WMODE_BE_6G);
			break;
		} else
			break;
	}

	return ret;
}

BOOLEAN wmode_band_equal(USHORT smode, USHORT tmode)
{
	BOOLEAN eq = FALSE;
	UCHAR *str1, *str2;

	if ((WMODE_CAP_5G(smode) == WMODE_CAP_5G(tmode)) &&
	    (WMODE_CAP_2G(smode) == WMODE_CAP_2G(tmode)) &&
	    (WMODE_CAP_6G(smode) == WMODE_CAP_6G(tmode)))
		eq = TRUE;

	str1 = wmode_2_str(smode);
	str2 = wmode_2_str(tmode);

	if (str1)
		os_free_mem(str1);

	if (str2)
		os_free_mem(str2);

	return eq;
}

UCHAR wmode_2_rfic(USHORT PhyMode)
{
	UCHAR rf_mode = 0;

	if (WMODE_CAP_2G(PhyMode))
		rf_mode |= RFIC_24GHZ;

	if (WMODE_CAP_5G(PhyMode))
		rf_mode |= RFIC_5GHZ;

	if (WMODE_CAP_6G(PhyMode))
		rf_mode |= RFIC_6GHZ;

	return rf_mode;
}

/*N9 CMD BW value*/
void bw_2_str(UCHAR bw, CHAR *bw_str, UINT max_str_size)
{
	int ret;
	switch (bw) {
	case BW_20:
		ret = snprintf(bw_str, max_str_size, "%s", "20");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case BW_40:
		ret = snprintf(bw_str, max_str_size, "%s", "20/40");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case BW_80:
		ret = snprintf(bw_str, max_str_size, "%s", "20/40/80");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case BW_8080:
		ret = snprintf(bw_str, max_str_size, "%s", "20/40/80/160NC");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case BW_160:
		ret = snprintf(bw_str, max_str_size, "%s", "20/40/80/160C");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case BW_320:
		ret = snprintf(bw_str, max_str_size, "%s", "20/40/80/160C/320");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case BW_10:
		ret = snprintf(bw_str, max_str_size, "%s", "10");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case BW_5:
		ret = snprintf(bw_str, max_str_size, "%s", "5");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	default:
		ret = snprintf(bw_str, max_str_size, "%s", "Invalid");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;
	}
}

void extcha_2_str(UCHAR extcha, CHAR *ec_str, UINT max_str_size)
{
	int ret;
	switch (extcha) {
	case EXTCHA_NONE:
		ret = snprintf(ec_str, max_str_size, "%s", "NONE");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case EXTCHA_ABOVE:
		ret = snprintf(ec_str, max_str_size, "%s", "ABOVE");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case EXTCHA_BELOW:
		ret = snprintf(ec_str, max_str_size, "%s", "BELOW");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	case EXTCHA_NOASSIGN:
		ret = snprintf(ec_str, max_str_size, "%s", "Not assignment");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;

	default:
		ret = snprintf(ec_str, max_str_size, "%s", "Invalid");
		if (os_snprintf_error(max_str_size, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		break;
	}
}

/*
	==========================================================================
	Description:
	Set Wireless Mode
	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/

INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg, struct wifi_dev *wdev)
{
	LONG cfg_mode;
	USHORT wmode;
	UCHAR *mode_str;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	cfg_mode = os_str_tol(arg, 0, 10);
	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);

	if (!wmode_valid_and_correct(pAd, &wmode)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "Invalid wireless mode(%ld, wmode=0x%x), ChipCap(%s)\n",
			 cfg_mode, wmode,
			  BAND_STR[pChipCap->phy_caps & 0x3]);
		return FALSE;
	}

	if (wmode_band_equal(wdev->PhyMode, wmode) == TRUE)
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"wmode_band_equal(): Band Equal!\n");
	else
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"wmode_band_equal(): Band Not Equal!\n");

	wdev->PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	mode_str = wmode_2_str(wmode);

	if (mode_str) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			": Set WMODE=%s(0x%x)\n",
			mode_str, wmode);
		os_free_mem(mode_str);
	}

	return TRUE;
}

/* maybe can be moved to GPL code, ap_mbss.c, but the code will be open */
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT

static BOOLEAN wmode_valid(RTMP_ADAPTER *pAd, enum WIFI_MODE wmode)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if ((WMODE_CAP_6G(wmode) && (!PHY_CAP_6G(cap->phy_caps))) ||
		(WMODE_CAP_5G(wmode) && (!PHY_CAP_5G(cap->phy_caps))) ||
	    (WMODE_CAP_2G(wmode) && (!PHY_CAP_2G(cap->phy_caps))) ||
	    (WMODE_CAP_N(wmode) && RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
	   )
		return FALSE;
	else
		return TRUE;
}

/*
	==========================================================================
	Description:
	Set Wireless Mode for MBSS
	Return:
	TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT cfg_mode;
	USHORT wmode;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	cfg_mode = os_str_tol(arg, 0, 10);
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);

	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
			 cfg_mode, wmode,
			  BAND_STR[pChipCap->phy_caps & 0x3]);
		return FALSE;
	}

	if (WMODE_CAP_5G(wmode) && WMODE_CAP_2G(wmode)) {
		if (pAd->CommonCfg.dbdc_mode == 1) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"AP cannot support 2.4G/5G band mxied mode!\n");
			return FALSE;
		}
	}

	pAd->CommonCfg.cfg_wmode = wmode;
	return TRUE;
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd)
{
	BOOLEAN TestFlag;

	TestFlag =	!strcmp(SetCmd, "Debug") ||
#ifdef CONFIG_APSTA_MIXED_SUPPORT
			!strcmp(SetCmd, "OpMode") ||
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST
			!strcmp(SetCmd, "CountryCode") ||
			!strcmp(SetCmd, "DfsType") ||
			!strcmp(SetCmd, "ChannelListAdd") ||
			!strcmp(SetCmd, "ChannelListShow") ||
			!strcmp(SetCmd, "ChannelListDel") ||
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef SINGLE_SKU
			!strcmp(SetCmd, "ModuleTxpower") ||
#endif /* SINGLE_SKU */
			FALSE; /* default */
	return TestFlag;
}

INT RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR BandIdx)
{
	LONG ShortSlot;

	ShortSlot = os_str_tol(arg, 0, 10);
	if (ShortSlot < 0 || ShortSlot > 1)
		return FALSE; /*Invalid argument */

	pAd->CommonCfg.bUseShortSlotTime = (ShortSlot == 1) ? TRUE : FALSE;
	pAd->CommonCfg.SlotTime = (ShortSlot == 1) ? 9 : 20;

	return TRUE;
}

INT	RT_CfgSetFixedTxPhyMode(RTMP_STRING *arg)
{
	INT fix_tx_mode = FIXED_TXMODE_HT;
	ULONG value;

	if (rtstrcasecmp(arg, "OFDM") == TRUE)
		fix_tx_mode = FIXED_TXMODE_OFDM;
	else if (rtstrcasecmp(arg, "CCK") == TRUE)
		fix_tx_mode = FIXED_TXMODE_CCK;
	else if (rtstrcasecmp(arg, "HT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_HT;
	else if (rtstrcasecmp(arg, "VHT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_VHT;
	else {
		value = os_str_tol(arg, 0, 10);

		switch (value) {
		case FIXED_TXMODE_CCK:
		case FIXED_TXMODE_OFDM:
		case FIXED_TXMODE_HT:
		case FIXED_TXMODE_VHT:
			fix_tx_mode = value;
			break;
		default:
			fix_tx_mode = FIXED_TXMODE_HT;
		}
	}

	return fix_tx_mode;
}

INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg, UCHAR idx, INT opmode)
{
	INT	i, mac_len;
	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(arg);

	if (mac_len != 17) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid length (%d)\n", mac_len);
		return FALSE;
	}

	if (strcmp(arg, "00:00:00:00:00:00") == 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR, "invalid mac setting\n");
		return FALSE;
	}

	if (opmode == OPMODE_AP) {
		if (idx == 0) {
			for (i = 0; i < MAC_ADDR_LEN; i++) {
				AtoH(arg, &pAd->CurrentAddress[i], 1);
				arg = arg + 3;
			}

			pAd->bLocalAdminMAC = TRUE;
		}

#ifdef MT_MAC
#ifdef MBSS_SUPPORT
		else {
			for (i = 0; i < MAC_ADDR_LEN; i++) {
				AtoH(arg, &pAd->ExtendMBssAddr[idx - 1][i], 1);
				arg = arg + 3;
			}

			/* TODO: Carter, is the below code still has its meaning? */
			pAd->bLocalAdminExtendMBssMAC = TRUE;
		}
#endif /* MBSS_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	} else if (opmode == OPMODE_STA) {
		for (i = 0; i < MAC_ADDR_LEN; i++) {
			AtoH(arg, &pAd->ApcliAddr[idx][i], 1);
			arg = arg + 3;
		}
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#endif /* MT_MAC */
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				 " idx(%d) non-supported opmode(%d)\n", idx, opmode);
	}

	return TRUE;
}

INT	RT_CfgSetTxMCSProc(RTMP_STRING *arg, BOOLEAN *pAutoRate)
{
	INT	Value = os_str_tol(arg, 0, 10);
	INT	TxMcs;

	if ((Value >= 0 && Value <= 23) || (Value == 32)) { /* 3*3*/
		TxMcs = Value;
		*pAutoRate = FALSE;
	} else {
		TxMcs = MCS_AUTO;
		*pAutoRate = TRUE;
	}

	return TxMcs;
}

INT	RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR AutoFallBack = (UCHAR)os_str_tol(arg, 0, 10);

	if (AutoFallBack)
		AutoFallBack = TRUE;
	else
		AutoFallBack = FALSE;

	AsicSetAutoFallBack(pAd, (AutoFallBack) ? TRUE : FALSE);
	MTWF_PRINT("%s::(AutoFallBack=%d)\n", __func__, AutoFallBack);
	return TRUE;
}

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *pPinCodeStr,
	OUT PWSC_CTRL   pWscControl)
{
	UINT pinCode;

	pinCode = (UINT) os_str_tol(pPinCodeStr, 0, 10); /* When PinCode is 03571361, return value is 3571361.*/

	if (strlen(pPinCodeStr) == 4) {
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 4;
	} else if (ValidateChecksum(pinCode)) {
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 8;
	} else {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_ERROR,
			": invalid Wsc PinCode (%d)\n", pinCode);
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_WPS, DBG_LVL_INFO,
		": Wsc PinCode=%d\n", pinCode);
	return TRUE;
}
#endif /* WSC_INCLUDED */

/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWNAME.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_giwname(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID *pData,
	IN	ULONG					Data)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR CurOpMode = OPMODE_AP;
	int ret;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (CurOpMode == OPMODE_AP) {
		if (wdev && wmode_2_cfgmode(wdev->PhyMode) < PHY_MODE_MAX) {
			RT_802_11_PHY_MODE cfg_mode = wmode_2_cfgmode(wdev->PhyMode);

			ret = snprintf(pData, Data, "%s", iee80211_str[cfg_mode]);
			if (os_snprintf_error(Data, ret)) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
					": snprintf error!\n");
				return NDIS_STATUS_FAILURE;
			}
		} else {
			strlcpy(pData, "RTWIFI SoftAP", strlen("RTWIFI SoftAP") + 1);
		}
	}
	return NDIS_STATUS_SUCCESS;
}

VOID rtmp_chip_prepare(RTMP_ADAPTER *pAd)
{
}

extern struct wifi_dev_ops ap_wdev_ops;
extern struct wifi_dev_ops sta_wdev_ops;

static VOID rtmp_netdev_set(RTMP_ADAPTER *pAd, PNET_DEV net_dev)
{
	struct wifi_dev *wdev = NULL;
	INT32 ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* set main net_dev */
	pAd->net_dev = net_dev;
#ifdef CONFIG_AP_SUPPORT

	if (pAd->OpMode == OPMODE_AP) {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		ASSERT(pMbss);
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
		RTMP_OS_NETDEV_SET_WDEV(net_dev, wdev);
		ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, net_dev, MAIN_MBSSID, (VOID *)pMbss, (VOID *)pAd);

		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
				"Assign wdev idx for %s failed, free net device!\n",
				RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}

		ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
					cap->qos.wmm_detect_method);

		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
				 "register wdev_ops %s failed, free net device!\n",
				 RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (pAd->OpMode == OPMODE_STA) {
		wdev = &pAd->StaCfg[MAIN_MSTA_ID].wdev;
		RTMP_OS_NETDEV_SET_WDEV(net_dev, wdev);
		ret = wdev_init(pAd, wdev, WDEV_TYPE_STA, net_dev, MAIN_MSTA_ID, (VOID *)&pAd->StaCfg[MAIN_MSTA_ID], (VOID *)pAd);

		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
				"Assign wdev idx for %s failed, free net device!\n",
				RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}

		ret = wdev_ops_register(wdev, WDEV_TYPE_STA, &sta_wdev_ops,
					cap->qos.wmm_detect_method);

		if (!ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_INTF, DBG_LVL_ERROR,
				 "register wdev_ops %s failed, free net device!\n",
				 RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
			RtmpOSNetDevFree(pAd->net_dev);
			return;
		}
	}

#endif /* CONFIG_STA_SUPPORT */
}

INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS, i;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	CHANNEL_CTRL *pChCtrl;
#ifdef CONFIG_STA_SUPPORT
	PSTA_ADMIN_CONFIG pStaCfg = NULL;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pStaCfg = &pAd->StaCfg[pObj->ioctl_if];
		wdev = &pStaCfg->wdev;
	}
#endif

	switch (cmd) {
	case CMD_RTPRIV_IOCTL_NETDEV_GET:
		/* get main net_dev */
	{
		VOID **ppNetDev = (VOID **)pData;
		*ppNetDev = (VOID *)(pAd->net_dev);
	}
	break;
#ifdef APCLI_CFG80211_SUPPORT
	case CMD_RTPRIV_IOCTL_APCLI_NETDEV_GET:
		/*get apcli net dev for CFG80211 mode */
	{
		VOID **ppNetDev = (VOID **)pData;
		*ppNetDev = (VOID *)(pAd->StaCfg[0].wdev.if_dev);
	}
	break;
#endif /* APCLI_CFG80211_SUPPORT */

	case CMD_RTPRIV_IOCTL_NETDEV_SET: {
		rtmp_netdev_set(pAd, pData);
		break;
	}

	case CMD_RTPRIV_IOCTL_OPMODE_GET:
		/* get Operation Mode */
		*(UINT32 *)pData = pAd->OpMode;
		break;

	case CMD_RTPRIV_IOCTL_TASK_LIST_GET:
		/* get all Tasks */
	{
		RT_CMD_WAIT_QUEUE_LIST *pList = (RT_CMD_WAIT_QUEUE_LIST *)pData;

		pList->pMlmeTask = &pAd->mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
		if (IS_ASIC_CAP(pAd, fASIC_CAP_MGMT_TIMER_TASK))
			pList->pTimerTask = &pAd->timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */
		pList->pCmdQTask = &pAd->cmdQTask;
#ifdef WSC_INCLUDED
		pList->pWscTask = &pAd->wscTask;
#endif /* WSC_INCLUDED */
	}
	break;
	case CMD_RTPRIV_IOCTL_NIC_NOT_EXIST:
		/* set driver state to fRTMP_ADAPTER_NIC_NOT_EXIST */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
		break;

	case CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR:
		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
		break;
#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	case CMD_RTPRIV_IOCTL_USB_DEV_GET:
		/* get USB DEV */
	{
		VOID **ppUsb_Dev = (VOID **)pData;
		*ppUsb_Dev = (VOID *)(pObj->pUsb_Dev);
	}
	break;

	case CMD_RTPRIV_IOCTL_USB_INTF_GET:
		/* get USB INTF */
	{
		VOID **ppINTF = (VOID **)pData;
		*ppINTF = (VOID *)(pObj->intf);
	}
	break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SEND_DISSASSOCIATE:

		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		if (INFRA_ON(pStaCfg) &&
		    (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))) {
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem;

			os_alloc_mem(NULL, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));

			if (MsgElem) {
				os_zero_mem(MsgElem, sizeof(MLME_QUEUE_ELEM));
				COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
				DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;
				MsgElem->Machine = ASSOC_FSM;
				MsgElem->MsgType = ASSOC_FSM_MLME_DISASSOC_REQ;
				MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
				NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));
				/* Prevent to connect AP again in STAMlmePeriodicExec*/
				pAd->MlmeAux.AutoReconnectSsidLen = 32;
				NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
				pStaCfg->CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
				MlmeDisassocReqAction(pAd, MsgElem);
				os_free_mem(MsgElem);
			}

			/*				RtmpusecDelay(1000);*/
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1, NULL, NULL, 0);
		}

		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_TEST:
		/* test driver state to fRTMP_ADAPTER_SUSPEND */
		*(UCHAR *)pData = RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_IDLE_RADIO_OFF_TEST:
		/* test driver state to fRTMP_ADAPTER_IDLE_RADIO_OFF */
		*(UCHAR *)pData = RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
		break;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_STATUS:
		*(UCHAR *)pData = (UCHAR)pAd->WOW_Cfg.bEnable;
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_ENABLE:
		ASIC_WOW_ENABLE(pAd, pStaCfg);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_DISABLE:
		ASIC_WOW_DISABLE(pAd, pStaCfg);
		break;
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */
#endif /* CONFIG_PM */
#ifdef CONFIG_STA_SUPPORT
	case CMD_RTPRIV_IOCTL_AP_BSSID_GET:
		if (pStaCfg && pStaCfg->wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			NdisCopyMemory(pData, pStaCfg->MlmeAux.Bssid, 6);
		else
			return NDIS_STATUS_FAILURE;

		break;
#endif
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		break;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
		break;

	case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
		break;
#endif /* CONFIG_STA_SUPPORT */

	case CMD_RTPRIV_IOCTL_SANITY_CHECK:

		/* sanity check before IOCTL */
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
#ifdef IFUP_IN_PROBE
		    || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		    || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif /* IFUP_IN_PROBE */
		   ) {
			if (pData == NULL ||	RT_isLegalCmdBeforeInfUp((RTMP_STRING *) pData) == FALSE)
				return NDIS_STATUS_FAILURE;
		}

		break;

	case CMD_RTPRIV_IOCTL_SIOCGIWFREQ:
		/* get channel number */
#ifdef WDS_SUPPORT
		if (pObj->ioctl_if_type == INT_WDS)
			wdev = get_wdev_by_idx(pAd, (INT)Data + MIN_NET_DEVICE_FOR_WDS);
		else
#endif
#ifdef APCLI_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI)
				wdev = get_wdev_by_idx(pAd, (INT)Data + MIN_NET_DEVICE_FOR_APCLI);
			else
#endif
				wdev = get_wdev_by_idx(pAd, (INT)Data);

		if (wdev)
			*(ULONG *)pData = wdev->channel;

		break;
#ifdef SNIFFER_SUPPORT

	case CMD_RTPRIV_IOCTL_SNIFF_INIT:
		Monitor_Init(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_SNIFF_OPEN:
		if (Monitor_Open(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_SNIFF_CLOSE:
		if (Monitor_Close(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_SNIFF_REMOVE:
		Monitor_Remove(pAd);
		break;
#endif /*SNIFFER_SUPPORT*/

#ifdef ACTION_MONITOR_SUPPORT
		case CMD_RTPRIV_IOCTL_ACTION_MON_INIT:
		Action_Monitor_Init(pAd, pData);
		break;
	case CMD_RTPRIV_IOCTL_ACTION_MON_OPEN:
		if (Action_Monitor_Open(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;
		break;
	case CMD_RTPRIV_IOCTL_ACTION_MON_CLOSE:
		if (Action_Monitor_Close(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;
		break;
	case CMD_RTPRIV_IOCTL_ACTION_MON_REMOVE:
		Action_Monitor_Remove(pAd);
		break;
#endif
	case CMD_RTPRIV_IOCTL_BEACON_UPDATE:
		/* update all beacon contents */
#ifdef CONFIG_AP_SUPPORT
		/* TODO: Carter, the oid seems been obsoleted. */
		UpdateBeaconHandler(
			pAd,
			get_default_wdev(pAd),
			BCN_REASON(BCN_UPDATE_ALL_AP_RENEW));
#endif /* CONFIG_AP_SUPPORT */
		break;

	case CMD_RTPRIV_IOCTL_RXPATH_GET:
		/* get the number of rx path */
		*(ULONG *)pData = pAd->Antenna.field.RxPath;
		break;

	case CMD_RTPRIV_IOCTL_CHAN_LIST_NUM_GET:
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);
		*(ULONG *)pData = pChCtrl->ChListNum;
		break;

	case CMD_RTPRIV_IOCTL_CHAN_LIST_GET: {
		UINT32 i;
		UCHAR *pChannel = (UCHAR *)pData;
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

		for (i = 1; i <= pChCtrl->ChListNum; i++) {
			*pChannel = pChCtrl->ChList[i - 1].Channel;
			pChannel++;
		}
	}
	break;

	case CMD_RTPRIV_IOCTL_FREQ_LIST_GET: {
		UINT32 i;
		UINT32 *pFreq = (UINT32 *)pData;
		UINT32 m;

		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

		for (i = 1; i <= pChCtrl->ChListNum; i++) {
			m = 2412000;
			MAP_CHANNEL_ID_TO_KHZ(pChCtrl->ChList[i - 1].Channel, m);
			(*pFreq) = m;
			pFreq++;
		}
	}
	break;
#ifdef EXT_BUILD_CHANNEL_LIST

	case CMD_RTPRIV_SET_PRECONFIG_VALUE:
		/* Set some preconfigured value before interface up*/
		pAd->CommonCfg.DfsType = MAX_RD_REGION;
		break;
#endif /* EXT_BUILD_CHANNEL_LIST */

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	case CMD_RTPRIV_IOCTL_PCI_SUSPEND:
	case CMD_RTPRIV_IOCTL_RBUS_SUSPEND:
#ifdef CONFIG_STA_SUPPORT
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

#ifdef MT_WOW_SUPPORT
		if ((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pStaCfg)) {
			BOOLEAN InfraAP_BW = FALSE;
			UCHAR BwFallBack = 0;

			if (pStaCfg->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
				InfraAP_BW = TRUE;
			else
				InfraAP_BW = FALSE;

			AdjustChannelRelatedValue(pAd,
						  &BwFallBack,
						  BSS0,
						  InfraAP_BW,
						  pStaCfg->MlmeAux.Channel,
						  pStaCfg->MlmeAux.CentralChannel,
						  wdev);
			pAd->WOW_Cfg.bWoWRunning = TRUE;
			ASIC_WOW_ENABLE(pAd, pStaCfg);
			AsicExtPmStateCtrl(pAd, pStaCfg, PM4, ENTER_PM_STATE);
		}
#endif /*MT_WOW_SUPPORT */

#ifdef MT_WOW_SUPPORT
		if (!pAd->WOW_Cfg.bWoWRunning)
#endif /* MT_WOW_SUPPORT */
		{
			MlmeRadioOff(pAd, wdev);
		}

		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
#endif /* CONFIG_STA_SUPPORT */
		break;

	case CMD_RTPRIV_IOCTL_PCI_RESUME:
	case CMD_RTPRIV_IOCTL_RBUS_RESUME:
#ifdef CONFIG_STA_SUPPORT
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef MT_WOW_SUPPORT
		if (!pAd->WOW_Cfg.bWoWRunning)
#endif /* MT_WOW_SUPPORT */
		{
			MlmeRadioOn(pAd, wdev);
		}

#ifdef MT_WOW_SUPPORT
		if (pAd->WOW_Cfg.bWoWRunning == TRUE) {
			AsicExtPmStateCtrl(pAd, pStaCfg, PM4, EXIT_PM_STATE);
			ASIC_WOW_DISABLE(pAd, pStaCfg);
			pAd->WOW_Cfg.bWoWRunning = FALSE;
		}
#endif /* MT_WOW_SUPPORT */

		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

		/* Delete all BA sessions after resume */
		if (pStaCfg)
			ba_session_tear_down_all(pAd, pStaCfg->PwrMgmt.wcid, FALSE);

#ifdef HOST_RESUME_DONE_ACK_SUPPORT
		rtmp_host_resume_done_ack(pAd);
#endif /* HOST_RESUME_DONE_ACK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
		break;
#endif /* RTMP_PCI_SUPPORT || RTMP_RBUS_SUPPORT */

	case CMD_RTPRIV_IOCTL_CHIP_PREPARE: {
		rtmp_chip_prepare(pAd);
	}
	break;
#ifdef RT_CFG80211_SUPPORT

	case CMD_RTPRIV_IOCTL_CFG80211_CFG_START:
		if (wdev)
			RT_CFG80211_REINIT(pAd, wdev);

#ifndef DISABLE_HOSTAPD_BEACON
		RT_CFG80211_CRDA_REG_RULE_APPLY(pAd);
#endif

		break;
#endif /* RT_CFG80211_SUPPORT */

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP:
		/* interface up */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_NOTICE,
			 " -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP\n");
		if (pInfConf->virtual_if_up_handler(pInfConf->operation_dev_p)) {
			if (VIRTUAL_IF_NUM(pAd) > 0)
				VIRTUAL_IF_DEC(pAd);
			Status = NDIS_STATUS_FAILURE;
		}
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN:
		/* interface down */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_NOTICE,
			 " -> CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN\n");
		VIRTUAL_IF_DEC(pAd);
		Status = pInfConf->virtual_if_down_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_INIT:
		/* init at first interface up */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_NOTICE,
			 "-> CMD_RTPRIV_IOCTL_VIRTUAL_INF_INIT\n");
		pInfConf->virtual_if_init_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DEINIT:
		/* deinit at last interface down */
	{
		RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

		MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_NOTICE,
			 "-> CMD_RTPRIV_IOCTL_VIRTUAL_INF_DEINIT\n");
		pInfConf->virtual_if_deinit_handler(pInfConf->operation_dev_p);
	}
	break;

	case CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET:
		/* get virtual interface number */
		*(ULONG *)pData = VIRTUAL_IF_NUM(pAd);
		break;

	case CMD_RTPRIV_IOCTL_INF_TYPE_GET:
		/* get current interface type */
		*(ULONG *)pData = pAd->infType;
		break;

	case CMD_RTPRIV_IOCTL_INF_STATS_GET:
		/* get statistics */
	{
		RT_CMD_STATS *pStats = (RT_CMD_STATS *)pData;

		pStats->pStats = pAd->stats;

		if (pAd->OpMode == OPMODE_STA) {
			pStats->rx_packets = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
			pStats->tx_packets = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;
			pStats->rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
			pStats->tx_bytes = pAd->RalinkCounters.TransmittedByteCount;
			pStats->rx_errors = pAd->Counters8023.RxErrors;
			pStats->tx_errors = pAd->Counters8023.TxErrors;
			pStats->multicast = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;   /* multicast packets received*/
			pStats->collisions = 0;  /* Collision packets*/
			pStats->rx_over_errors = pAd->Counters8023.RxNoBuffer;				   /* receiver ring buff overflow*/
			pStats->rx_crc_errors = 0;/*pAd->WlanCounters.FCSErrorCount;	  recved pkt with crc error*/
			pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
			pStats->rx_fifo_errors = pAd->Counters8023.RxNoBuffer;				   /* recv'r fifo overrun*/
		}

#ifdef CONFIG_AP_SUPPORT
		else if (pAd->OpMode == OPMODE_AP) {
			INT index;
			BOOLEAN found_it = FALSE;
			INT stat_db_source;

			for (index = 0; index < MAX_MBSSID_NUM(pAd); index++) {
				if (pAd->ApCfg.MBSSID[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
					found_it = TRUE;
					stat_db_source = 0;
					break;
				}
#ifdef CONFIG_VLAN_GTK_SUPPORT
				else if (CFG80211_MatchVlandev(&pAd->ApCfg.MBSSID[index].wdev, (PNET_DEV)pStats->pNetDev)) {
					found_it = TRUE;
					stat_db_source = 0;
					break;
				}
#endif
			}
#ifdef CONFIG_STA_SUPPORT
			if (found_it == FALSE) {
				index = 0;
				if (pAd->StaCfg[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
					found_it = TRUE;
					stat_db_source = 1;
				}
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT

			if (found_it == FALSE) {
				for (index = 0; index < pAd->ApCfg.ApCliNum; index++) {
					if (pAd->StaCfg[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev)) {
						found_it = TRUE;
						stat_db_source = 1;
						break;
					}
				}
			}

#endif

			if (found_it == FALSE) {
				/* reset counters */
				pStats->rx_packets = 0;
				pStats->tx_packets = 0;
				pStats->rx_bytes = 0;
				pStats->tx_bytes = 0;
				pStats->rx_errors = 0;
				pStats->tx_errors = 0;
				pStats->multicast = 0;   /* multicast packets received*/
				pStats->collisions = 0;  /* Collision packets*/
				pStats->rx_drop_count = 0;/* recevier drop packets*/
				pStats->tx_drop_count = 0;/* transmit drop packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0; /* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/
				MTWF_DBG(pAd, DBG_CAT_INIT, CATINIT_INTF, DBG_LVL_ERROR,
					"CMD_RTPRIV_IOCTL_INF_STATS_GET: can not find mbss I/F\n");
				return NDIS_STATUS_FAILURE;
			}
			if (stat_db_source == 0) {
				pStats->rx_packets = pAd->ApCfg.MBSSID[index].RxCount;
				pStats->tx_packets = pAd->ApCfg.MBSSID[index].TxCount;
				pStats->rx_bytes = pAd->ApCfg.MBSSID[index].ReceivedByteCount;
				pStats->tx_bytes = pAd->ApCfg.MBSSID[index].TransmittedByteCount;
				pStats->rx_errors = pAd->ApCfg.MBSSID[index].RxErrorCount;
				pStats->tx_errors = pAd->ApCfg.MBSSID[index].TxErrorCount;
				pStats->rx_drop_count = pAd->ApCfg.MBSSID[index].RxDropCount;
				pStats->tx_drop_count = pAd->ApCfg.MBSSID[index].TxDropCount;
				pStats->multicast = pAd->ApCfg.MBSSID[index].mcPktsRx; /* multicast packets received */
				pStats->collisions = 0;  /* Collision packets*/
				pStats->rx_over_errors = 0;				   /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0;                   /* recv'r fifo overrun*/
			} else if (stat_db_source == 1) {
#ifdef CONFIG_STA_SUPPORT
				pStats->rx_packets = pAd->StaCfg[index].StaStatistic.RxCount;
				pStats->tx_packets = pAd->StaCfg[index].StaStatistic.TxCount;
				pStats->rx_bytes = pAd->StaCfg[index].StaStatistic.ReceivedByteCount;
				pStats->tx_bytes = pAd->StaCfg[index].StaStatistic.TransmittedByteCount;
				pStats->rx_errors = pAd->StaCfg[index].StaStatistic.RxErrorCount;
				pStats->tx_errors = pAd->StaCfg[index].StaStatistic.TxErrorCount;
				pStats->rx_drop_count = pAd->StaCfg[index].StaStatistic.RxDropCount;
				pStats->tx_drop_count = pAd->StaCfg[index].StaStatistic.TxDropCount;
				pStats->multicast = pAd->StaCfg[index].StaStatistic.mcPktsRx; /* multicast packets received */
				pStats->collisions = 0; /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0;
#endif /* CONFIG_STA_SUPPORT */
#ifdef APCLI_SUPPORT
				pStats->rx_packets = pAd->StaCfg[index].StaStatistic.RxCount;
				pStats->tx_packets = pAd->StaCfg[index].StaStatistic.TxCount;
				pStats->rx_bytes = pAd->StaCfg[index].StaStatistic.ReceivedByteCount;
				pStats->tx_bytes = pAd->StaCfg[index].StaStatistic.TransmittedByteCount;
				pStats->rx_errors = pAd->StaCfg[index].StaStatistic.RxErrorCount;
				pStats->tx_errors = pAd->StaCfg[index].StaStatistic.TxErrorCount;
				pStats->rx_drop_count = pAd->StaCfg[index].StaStatistic.RxDropCount;
				pStats->tx_drop_count = pAd->StaCfg[index].StaStatistic.TxDropCount;
				pStats->multicast = pAd->StaCfg[index].StaStatistic.mcPktsRx; /* multicast packets received */
				pStats->collisions = 0; /* Collision packets*/
				pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
				pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
				pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
				pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/
#endif
			}

		}

#endif
	}
	break;

	case CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET:

		/* get wireless statistics */
	{
		UCHAR CurOpMode = OPMODE_AP;
#ifdef CONFIG_AP_SUPPORT
		PMAC_TABLE_ENTRY pMacEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
		RT_CMD_IW_STATS *pStats = (RT_CMD_IW_STATS *)pData;

		pStats->qual = 0;
		pStats->level = 0;
		pStats->noise = 0;
		pStats->pStats = pAd->iw_stats;
#ifdef CONFIG_STA_SUPPORT

		if (pAd->OpMode == OPMODE_STA) {
			CurOpMode = OPMODE_STA;
		}

#endif /* CONFIG_STA_SUPPORT */

		/*check if the interface is down*/
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS))
			return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
#ifdef APCLI_SUPPORT

			if (pStats->priv_flags == INT_APCLI) {
				INT ApCliIdx = ApCliIfLookUp(pAd, (PUCHAR)pStats->dev_addr);

				if ((ApCliIdx >= 0) && IS_WCID_VALID(pAd, pAd->StaCfg[ApCliIdx].MacTabWCID))
					pMacEntry = entry_get(
						pAd, pAd->StaCfg[ApCliIdx].MacTabWCID);
			} else
#endif /* APCLI_SUPPORT */
			{
				/*
					only AP client support wireless stats function.
					return NULL pointer for all other cases.
				*/
				pMacEntry = NULL;
			}
		}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == OPMODE_STA)
			pStats->qual = ((pStaCfg->ChannelQuality * 12) / 10 + 10);

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
			if (pMacEntry != NULL)
				pStats->qual = ((pMacEntry->ChannelQuality * 12) / 10 + 10);
			else
				pStats->qual = ((pAd->Mlme.ChannelQuality * 12) / 10 + 10);
		}

#endif /* CONFIG_AP_SUPPORT */

		if (pStats->qual > 100)
			pStats->qual = 100;

#ifdef CONFIG_STA_SUPPORT

		if (CurOpMode == OPMODE_STA) {
			pStats->level =
				RTMPMaxRssi(pAd, pStaCfg->RssiSample.AvgRssi[0],
					    pStaCfg->RssiSample.AvgRssi[1],
					    pStaCfg->RssiSample.AvgRssi[2]);
			pStats->noise = RTMPMaxRssi(pAd, pStaCfg->RssiSample.AvgRssi[0],
						    pStaCfg->RssiSample.AvgRssi[1],
						    pStaCfg->RssiSample.AvgRssi[2]) -
					RTMPMinSnr(pAd, pAd->StaCfg[0].RssiSample.AvgSnr[0],
						   pStaCfg->RssiSample.AvgSnr[1]);
		}

#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

		if (CurOpMode == OPMODE_AP) {
			if (pMacEntry != NULL)
				pStats->level =
					RTMPMaxRssi(pAd, pMacEntry->RssiSample.AvgRssi[0],
						    pMacEntry->RssiSample.AvgRssi[1],
						    pMacEntry->RssiSample.AvgRssi[2]);

			pStats->noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
						    pAd->ApCfg.RssiSample.AvgRssi[1],
						    pAd->ApCfg.RssiSample.AvgRssi[2]) -
					RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr[0],
						   pAd->ApCfg.RssiSample.AvgSnr[1]);
		}

#endif /* CONFIG_AP_SUPPORT */

	}
	break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_CREATE:
		*(VOID **)pData = RtmpPhyNetDevMainCreate(pAd);
		break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET:
		*(ULONG *)pData = INT_MAIN;
		break;

	case CMD_RTPRIV_IOCTL_INF_MAIN_CHECK:
		if (Data != INT_MAIN)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_INF_P2P_CHECK:
		if (Data != INT_P2P)
			return NDIS_STATUS_FAILURE;

		break;
#ifdef WDS_SUPPORT

	case CMD_RTPRIV_IOCTL_WDS_INIT:
		WDS_Init(pAd, (UCHAR)Data, pData);
		break;

	case CMD_RTPRIV_IOCTL_WDS_REMOVE:
		WDS_Remove(pAd);
		break;

	case CMD_RTPRIV_IOCTL_WDS_STATS_GET:
		if (Data == INT_WDS) {
			if (WDS_StatsGet(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
		} else
			return NDIS_STATUS_FAILURE;

		break;
#endif /* WDS_SUPPORT */

#ifdef CONFIG_ATE
	/* Note: temp for wlan_service/original coexistence */
#ifdef CONFIG_WLAN_SERVICE
	case (CMD_RTPRIV_IOCTL_COMMON)CMD_RTPRIV_IOCTL_ATE: {
		struct hqa_frame *hqa_frame;
		struct hqa_frame_ctrl local_hqa;

		os_alloc_mem_suspend(pAd, (UCHAR **)&hqa_frame, sizeof(*hqa_frame));

		if (!hqa_frame) {
			Status = -ENOMEM;
			break;
		}

		NdisZeroMemory(hqa_frame, sizeof(*hqa_frame));
		Status = copy_from_user((PUCHAR)hqa_frame, wrq->u.data.pointer, wrq->u.data.length);

		if (Status)	{
			Status = -EFAULT;
			goto IOCTL_TEST_ERROR;
		}

		/* 0 means eth type hqa frame*/
		local_hqa.type = 0;
		local_hqa.hqa_frame_eth = hqa_frame;
		Status = mt_agent_hqa_cmd_handler(&pAd->serv, (struct hqa_frame_ctrl *)&local_hqa);

		/* Update cmd_frame length */
		if (OS_NTOHS((hqa_frame)->length) > SERV_IOCTLBUFF) {
			Status = -EFAULT;
			goto IOCTL_TEST_ERROR;
		}
		wrq->u.data.length = sizeof((hqa_frame)->magic_no) + sizeof((hqa_frame)->type)
					+ sizeof((hqa_frame)->id) + sizeof((hqa_frame)->length)
					+ sizeof((hqa_frame)->sequence) + OS_NTOHS((hqa_frame)->length);
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_DEBUG,
			"wrq->u.data.length=%d, usr_addr=%p, hqa_addr=%p\n",
			wrq->u.data.length, wrq->u.data.pointer, hqa_frame);

		/* Feedback as soon as we can to avoid QA timeout */
		if (copy_to_user(wrq->u.data.pointer, (UCHAR *)hqa_frame, wrq->u.data.length)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"copy_to_user() fail\n");
			Status = -EFAULT;
			goto IOCTL_TEST_ERROR;
		}

		/* TODO: Check sanity */
IOCTL_TEST_ERROR:
		os_free_mem(hqa_frame);
	}
	break;
#else
	case (CMD_RTPRIV_IOCTL_COMMON)CMD_RTPRIV_IOCTL_ATE: {
		struct _HQA_CMD_FRAME *HqaCmdFrame;

		os_alloc_mem_suspend(pAd, (UCHAR **)&HqaCmdFrame, sizeof(*HqaCmdFrame));

		if (!HqaCmdFrame) {
			Status = -ENOMEM;
			break;
		}

		NdisZeroMemory(HqaCmdFrame, sizeof(*HqaCmdFrame));
		Status = copy_from_user((PUCHAR)HqaCmdFrame, wrq->u.data.pointer, wrq->u.data.length);

		if (Status)	{
			Status = -EFAULT;
			goto IOCTL_ATE_ERROR;
		}

		Status = HQA_CMDHandler(pAd, wrq, HqaCmdFrame);
		/* TODO: Check sanity */
IOCTL_ATE_ERROR:
		os_free_mem(HqaCmdFrame);
	}
	break;
#endif /* CONFIG_WLAN_SERVICE */
#endif /* CONFIG_ATE */

#ifdef CONFIG_WLAN_SERVICE
	case CMD_RTPRIV_IOCTL_TESTENGINE: {
			RTMPAPIoctlAutoTestEngineCmd(pAd, wrq);
	}
	break;
#endif

	case CMD_RTPRIV_IOCTL_MAC_ADDR_GET: {
		UCHAR mac_addr[MAC_ADDR_LEN];
		USHORT Addr01 = 0, Addr23 = 0, Addr45 = 0;

		RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
		RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
		RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);
		mac_addr[0] = (UCHAR)(Addr01 & 0xff);
		mac_addr[1] = (UCHAR)(Addr01 >> 8);
		mac_addr[2] = (UCHAR)(Addr23 & 0xff);
		mac_addr[3] = (UCHAR)(Addr23 >> 8);
		mac_addr[4] = (UCHAR)(Addr45 & 0xff);
		mac_addr[5] = (UCHAR)(Addr45 >> 8);

		for (i = 0; i < 6; i++)
			*(UCHAR *)(pData + i) = mac_addr[i];

		break;
	}

#ifdef CONFIG_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_SIOCGIWRATE:
		/* handle for SIOCGIWRATEQ */
	{
		RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
		union _HTTRANSMIT_SETTING HtPhyMode;
#ifdef DOT11_HE_AX
		HE_TRANSMIT_SETTING HePhyMode;
#endif
		UINT8 BW;
		UINT8 Antenna = 0;
		USHORT MCS;
		struct wifi_dev	*wdev = NULL;
		ULONG getBitRate = 0;
#ifdef APCLI_SUPPORT
		MAC_TABLE_ENTRY	*pEntry = NULL;
#endif /* APCLI_SUPPORT */
		INT ifIndex = pObj->ioctl_if;

#ifdef APCLI_SUPPORT
		if (pRate->priv_flags == INT_APCLI) {
			if ((ifIndex < pAd->MSTANum) && (ifIndex >= 0)) {
				pEntry = MacTableLookup2(pAd, pAd->StaCfg[ifIndex].wdev.bssid,
							&pAd->StaCfg[ifIndex].wdev);

				if (!pEntry) {/* show maximum capability */
					HtPhyMode = pAd->StaCfg[ifIndex].wdev.HTPhyMode;
				} else {
					HtPhyMode = pEntry->HTPhyMode;

				if (HtPhyMode.field.MODE == MODE_VHT)
					Antenna = (HtPhyMode.field.MCS >> 4) + 1;
				}

				wdev = &pAd->StaCfg[ifIndex].wdev;
			}
			else {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"invalid sta entry id(%d)\n", ifIndex);
				return NDIS_STATUS_FAILURE;
			}

		} else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
		if (pRate->priv_flags == INT_WDS) {
			if ((ifIndex < MAX_WDS_ENTRY) && (ifIndex >= 0)) {
				HtPhyMode = pAd->WdsTab.WdsEntry[ifIndex].wdev.HTPhyMode;
				wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
			}
			else {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"invalid wds entry id(%d)\n", ifIndex);
				return NDIS_STATUS_FAILURE;
			}
		} else
#endif /* WDS_SUPPORT */
		{
			if ((ifIndex < MAX_BEACON_NUM) && (ifIndex >= 0)) {
				HtPhyMode = pAd->ApCfg.MBSSID[ifIndex].wdev.HTPhyMode;
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			}
			else {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"invalid mbss entry id(%d)\n", ifIndex);
				return NDIS_STATUS_FAILURE;
			}
		}
		if (!wdev->DevInfo.WdevActive)
			break;
		MCS = HtPhyMode.field.MCS;
#ifdef DOT11_N_SUPPORT
		if ((HtPhyMode.field.MODE == MODE_HTMIX)
		    || (HtPhyMode.field.MODE == MODE_HTGREENFIELD)) {
			Antenna = (HtPhyMode.field.MCS >> 3) + 1;
			MCS = MCS & 0xffff;
		}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		if (HtPhyMode.field.MODE >= MODE_VHT)
			MCS = MCS & 0xf;

		if (HtPhyMode.field.MODE >= MODE_VHT) {
			BW = wlan_operate_get_vht_bw(wdev);
			if (wlan_operate_get_ht_bw(wdev)) {
				if (BW == 0) /* VHT40 */
					BW = 1;
				else if (BW == 1) /* VHT80 */
					BW = 2;
				else if (BW >= 2) /* VHT80-80,VHT160 */
					BW = 3;
			}
			/*for spec VHT mode (bw20M no 3*3) or (bw160M and 3*3) maxMCS is 8*/
			if (((wlan_operate_get_ht_bw(wdev) == 0) && (wlan_config_get_tx_stream(wdev) != 3) &&
			     (BW == 0)) || ((wlan_config_get_tx_stream(wdev) == 3) && (BW == 3)))
				MCS = 8;
			/*bw40M or bw80M or (3*3 and bw20M) maxMCS is 9*/
			else
				MCS = 9;
		} else
#endif /*DOT11_VHT_AC*/
			if (HtPhyMode.field.MODE >= MODE_HTMIX)
				BW = wlan_operate_get_ht_bw(wdev);
			else
				BW = HtPhyMode.field.BW;
		if (Antenna == 0)
			Antenna = wlan_config_get_tx_stream(wdev);

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"HtPhyMode.field.MODE=%d\n\r", HtPhyMode.field.MODE);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"HtPhyMode.field.ShortGI=%d\n\r", HtPhyMode.field.ShortGI);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"HtPhyMode.field.BW=%d\n\r", HtPhyMode.field.BW);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"HtPhyMode.field.MCS=%d\n\r", MCS);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"BW=%d\n\r", BW);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Antenna=%d\n\r", Antenna);
#ifdef DOT11_EHT_BE
		if (WMODE_CAP_BE(wdev->PhyMode)) {
			if (ifIndex >= MAX_BEACON_NUM) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"invalid mbss id(%d)\n", ifIndex);
				return NDIS_STATUS_FAILURE;
			}
			Antenna = wlan_config_get_eht_tx_nss(wdev);
			BW =  wlan_operate_get_eht_bw(wdev);
			MCS = 13;

			RtmpDrvMaxRateGet(pAd, MODE_EHT, FIXED_GI_08_US,
				  BW, MCS,
				  Antenna,
				  &pRate->BitRate);
			break;
		}
#endif /*DOT11_HE_AX*/

#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(wdev->PhyMode)) {
			if (VALID_MBSS(pAd, ifIndex))
				HePhyMode = pAd->ApCfg.MBSSID[ifIndex].wdev.HEPhyMode;
			else {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					"invalid mbss id(%d)\n", ifIndex);
				return NDIS_STATUS_FAILURE;
			}
			Antenna = wlan_config_get_he_tx_nss(wdev);
			BW =  wlan_operate_get_he_bw(wdev);
			MCS = HePhyMode.field.MCS;

			RtmpDrvMaxRateGet(pAd, HePhyMode.field.MODE, HePhyMode.field.ShortGI,
				  BW, MCS,
				  Antenna,
				  &pRate->BitRate);
			break;
		}
#endif /*DOT11_HE_AX*/

#ifdef DOT11_VHT_AC
		if ((pAd->CommonCfg.g_band_256_qam == 1) && (HtPhyMode.field.MODE == MODE_HTMIX) &&
		    (wdev->channel <= 14)) {
			if ((MCS == 7) || (MCS == 15) || (MCS == 23) || (MCS == 31)) {
				HtPhyMode.field.MODE = MODE_VHT;
				/*for spec VHT mode (bw20M no 3*3) or (bw160M and 3*3) maxMCS is 8*/
				if (((BW == 0) && (MCS != 23)) || ((BW == 3) && (MCS == 23)))
					MCS = 8;
				/*bw40M or bw80M or (3*3 and bw20M) maxMCS is 9*/
				else
					MCS = 9;
			}
		}
#endif /*DOT11_VHT_AC*/

		if (HtPhyMode.field.MODE == MODE_VHT) {
			HtPhyMode.field.MCS = (Antenna - 1) << 4;
			HtPhyMode.field.MCS |= MCS;
		}
		HtPhyMode.field.BW = BW;
		getRate(HtPhyMode, &getBitRate);
		pRate->BitRate = (UINT64)getBitRate * 1000000;
	}
	break;
#endif /* CONFIG_AP_SUPPORT */

	case CMD_RTPRIV_IOCTL_SIOCGIWNAME:
		RtmpIoctl_rt_ioctl_giwname(pAd, pData, Data);
		break;
#ifdef CONFIG_CSO_SUPPORT

	case CMD_RTPRIV_IOCTL_ADAPTER_CSO_SUPPORT_TEST:
		*(UCHAR *)pData = (pAd->MoreFlags & fASIC_CAP_CSO) ? 1 : 0;
		break;
#endif /* CONFIG_CSO_SUPPORT */
#ifdef PROFILE_PATH_DYNAMIC

	case CMD_RTPRIV_IOCTL_PROFILEPATH_SET:
		pAd->profilePath = (CHAR *)Data;
		break;
#endif /* PROFILE_PATH_DYNAMIC */
#ifdef RT_CFG80211_SUPPORT
#ifdef APCLI_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_AP_STA_GET:
		break;
#endif /* APCLI_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
	}

#ifdef RT_CFG80211_SUPPORT

	if ((cmd >= CMD_RTPRIV_IOCTL_80211_START) &&
	    (cmd <= CMD_RTPRIV_IOCTL_80211_END))
		Status = CFG80211DRV_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);

#endif /* RT_CFG80211_SUPPORT */

	if (cmd >= CMD_RTPRIV_IOCTL_80211_COM_LATEST_ONE)
		return NDIS_STATUS_FAILURE;

	return Status;
}

/*
	==========================================================================
	Description:
	Issue a site survey command to driver
	Arguments:
		pAdapter					Pointer to our adapter
		wrq						 Pointer to the ioctl argument

	Return Value:
	None

	Note:
	Usage:
		   1.) iwpriv ra0 set site_survey
	==========================================================================
*/
INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	BOOLEAN ap_scan = TRUE; /* snowpin for ap/sta */

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_PRINT("INFO::Network is down!\n");
		return -ENETDOWN;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

		if (pObj->ioctl_if_type == INT_APCLI)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else
#endif
#ifdef CONFIG_STA_SUPPORT
			if (pObj->ioctl_if_type == INT_APCLI) {
				ap_scan = FALSE;
				if (ifIndex < MAX_APCLI_NUM)
					wdev = &pAd->StaCfg[ifIndex].wdev;
				else {
					MTWF_PRINT("invalid apcli entry id(%d)\n", ifIndex);
					return -EINVAL;
				}
			} else
#endif /* CONFIG_STA_SUPPORT */
				if (pObj->ioctl_if_type == INT_MBSSID) {
					if (ifIndex < MAX_BEACON_NUM)
						wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
					else {
						MTWF_PRINT("invalid mbss entry id(%d)\n", ifIndex);
						return -EINVAL;
					}
				}
				else
					wdev = &pAd->ApCfg.MBSSID[0].wdev;

		ASSERT(wdev);
		if (!wdev) {
			MTWF_PRINT("wdev is null!\n");
			return -EINVAL;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	/*do dfs cac check for 5G band before active scan*/
#ifdef MT_DFS_SUPPORT
	if (wdev) {
		if (wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G) {
			struct DOT11_H *pDot11h = &pAd->Dot11_H;

			if (pDot11h && (pDot11h->ChannelMode == CHAN_SILENCE_MODE)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
				"still in silence mode, can not trigger scan\n");
				return -EINVAL;
			}
		}
	}
#endif

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (MONITOR_ON(pAd)) {
			MTWF_PRINT("!!! Driver is in Monitor Mode now !!!\n");
			return -EINVAL;
		}
		ap_scan = FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	if (ap_scan) /* snopwin for ap/sta */
	{
		if ((strlen(arg) > 0) && (strlen(arg) <= MAX_LEN_OF_SSID)) {
			NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);
		} else {
			Ssid.SsidLength = 0;
			ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_PASSIVE, FALSE, wdev);
		}

		return TRUE;
	}

#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (!ap_scan) {
		Ssid.SsidLength = 0;

		if (ifIndex < pAd->MSTANum) {
			if ((arg != NULL) &&
			    (strlen(arg) <= MAX_LEN_OF_SSID)) {
				RTMPMoveMemory(Ssid.Ssid, arg, strlen(arg));
				Ssid.SsidLength = strlen(arg);
			}
			wdev = &pAd->StaCfg[ifIndex].wdev;
			if (wdev == NULL) {
				MTWF_PRINT("wdev is NULL\n");
				return -EINVAL;
			}
			/*To do SiteSurvey, need TakeChannelOpCharge first*/
			if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN, FALSE)) {
				MTWF_PRINT("TakeChannelOpCharge fail for SCAN!!\n");
				return -EINVAL;
			}
			pAd->StaCfg[ifIndex].bSkipAutoScanConn = TRUE;
			StaSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, wdev);
		}
		else {
			MTWF_PRINT("invalid sta entry id(%d)\n", ifIndex);
			return -EINVAL;
		}
	}
#endif /* CONFIG_STA_SUPPORT // */
	MTWF_PRINT("%s end.\n", __func__);
	return TRUE;
}

#ifdef AP_SCAN_SUPPORT
/*
    ==========================================================================
    Description:
	Issue a Clear site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ClearSiteSurvey=1
    ==========================================================================
*/
INT Set_ClearSiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 flag;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	flag = simple_strtol(arg, 0, 10);

	if (strlen(arg) > 1 || flag != 1) {
		MTWF_PRINT("Wrong argument type/Value\n");
		return FALSE;
	}

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_PRINT("INFO::Network is down!\n");
		return -ENETDOWN;
	}

	/* Still scanning, Don't clear the scan Table */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		MTWF_PRINT(": Scan in Progress!\n");
		return -EINVAL;
	}

	ifIndex = pObj->ioctl_if;
#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		if (ifIndex < MAX_APCLI_NUM) {
			wdev = &pAd->StaCfg[ifIndex].wdev;
			ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
		} else {
			MTWF_PRINT("invalid apcli entry id(%d)\n", ifIndex);
			return -EINVAL;
		}
	} else
#endif/*APCLI_SUPPORT*/
		if ((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN)) {
			if (ifIndex < MAX_BEACON_NUM) {
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
				ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
			} else {
				MTWF_PRINT("invalid mbss entry id(%d)\n",  ifIndex);
				return -EINVAL;
			}
		}

	if (!ScanCtrl) {
		MTWF_PRINT("ScanCtrl==NULL!\n");
		return -EINVAL;
	}

	/* Don't clear the scan table if we are doing scanning */
	if (scan_in_run_state(pAd, wdev) || (ScanCtrl->PartialScan.bScanning == TRUE)) {
		MTWF_PRINT("scan is ongoing and return\n");
		return FALSE;
	} else {
		BssTableInit(&ScanCtrl->ScanTab);
		MTWF_PRINT("Clear the Scan table\n");
	}

	MTWF_PRINT("Set_ClearSiteSurvey_Proc\n");
	return TRUE;
}
#endif /* AP_SCAN_SUPPORT */

INT	Set_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ANT_DIVERSITY_TYPE UsedAnt;
	INT i;

	MTWF_PRINT("==> Set_Antenna_Proc *******************\n");

	for (i = 0; i < strlen(arg); i++)
		if (!isdigit(arg[i]))
			return -EINVAL;

	UsedAnt = os_str_tol(arg, 0, 10);

	switch (UsedAnt) {
		/* 2: Fix in the PHY Antenna CON1*/
	case ANT_FIX_ANT0:
		AsicSetRxAnt(pAd, 0);
		MTWF_PRINT("<== Set_Antenna_Proc(Fix in Ant CON1), (%d,%d)\n",
				pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt);
		break;

		/* 3: Fix in the PHY Antenna CON2*/
	case ANT_FIX_ANT1:
		AsicSetRxAnt(pAd, 1);
		MTWF_PRINT("<== %s(Fix in Ant CON2), (%d,%d)\n",
				__func__, pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt);
		break;

	default:
		MTWF_PRINT("<== (N/A cmd: %d), (%d,%d)\n",  UsedAnt,
				pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt);
		break;
	}

	return TRUE;
}

INT set_tssi_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 tssi_enable = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	tssi_enable = os_str_tol(arg, 0, 10);

	if (tssi_enable == 1) {
		cap->tssi_enable = TRUE;
		MTWF_PRINT("turn on TSSI mechanism\n");
	} else if (tssi_enable == 0) {
		cap->tssi_enable = FALSE;
		MTWF_PRINT("turn off TSS mechanism\n");
	} else {
		MTWF_PRINT("illegal param(%u)\n", tssi_enable);
		return FALSE;
	}

	return TRUE;
}

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR radio;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss = NULL;
#endif

	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "wdev is NULL\n");
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (wdev->func_idx >= MAX_BEACON_NUM) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				 "invalid mbss index(=%d)\n", wdev->func_idx);
			return FALSE;
		}
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	}
#endif

	radio = os_str_tol(arg, 0, 10);

	if (!wdev->if_up_down_state) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "==>(%s) but IF is done, ignore!!! (wdev_idx %d)\n",
			  radio ? "ON" : "OFF", wdev->wdev_idx);
		return TRUE;
	}

	if (radio == !IsHcRadioCurStatOffByWdev(wdev)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "==>(%s) equal to current state, ignore!!! (wdev_idx %d)\n",
			  radio ? "ON" : "OFF", wdev->wdev_idx);
		return TRUE;
	}
#ifdef CONFIG_AP_SUPPORT
	if (!pMbss) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			 "pMbss is NULL\n");
		return FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */
	if (radio) {
		MlmeRadioOn(pAd, wdev);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "==>(ON)\n");
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
#ifdef FT_R1KH_KEEP
			/*
			 * Keep the R1KH table when Radio On is done twice for MBO-4.2.6(E)
			 * case to meet the R1KH miss case.
			 */
			pAd->ApCfg.FtTab.RadioOn++;

			if (pAd->ApCfg.FtTab.RadioOn <= RADIO_ON_SET)
				pAd->ApCfg.FtTab.FT_RadioOff = FALSE;

			if (pAd->ApCfg.FtTab.RadioOn == RADIO_ON_RESET)
				pAd->ApCfg.FtTab.RadioOn = FALSE;
#endif /* FT_R1KH_KEEP */
		}
#endif
		wdev->radio_off_req = FALSE;
	} else {
		wdev->radio_off_req = TRUE;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef FT_R1KH_KEEP
			pAd->ApCfg.FtTab.FT_RadioOff = TRUE;
#endif /* FT_R1KH_KEEP */
			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
		}
#endif
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			MSTAStop(pAd, wdev);
		}
#endif
#ifndef HWIFI_SUPPORT
		if (!hwctrl_cmd_q_empty(pAd)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"(OFF),cmd q not empty!\n");
		}
#endif /* HWIFI_SUPPORT */

		MlmeRadioOff(pAd, wdev);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO, "==> (OFF)\n");
	}

	return TRUE;
}

#ifdef ZERO_PKT_LOSS_SUPPORT
INT Set_WcidSkipTx_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *Param;
	UINT16 wcid;
	UINT8 set;

	if (arg == NULL)
		goto error;
	Param = rstrtok(arg, ":");
	if (Param != NULL)
		wcid = os_str_tol(Param, 0, 10);
	else
		goto error;
	Param = rstrtok(NULL, ":");
	if (Param != NULL)
		set = os_str_tol(Param, 0, 10);
	else
		goto error;
	MTWF_PRINT("%s():Wcid(%d), Set(%d)\n",  __func__, wcid, set);
	AsicUpdateSkipTx(pAd, wcid, set);
error:
	return true;
}

/*show ap valid channel list*/
INT Set_APChannelList_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	int i = 0;
	PCHANNEL_CTRL pChCtrl = NULL;

	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl);

	MTWF_PRINT("Channel Count:%d\n", pChCtrl->ChListNum);
	for (i = 0; i < pChCtrl->ChListNum; i++) {
		MTWF_PRINT("Channel[%d]:%d flags:%d\n",
						i, pChCtrl->ChList[i].Channel, pChCtrl->ChList[i].Flags);
	}
	return TRUE;
}

INT Set_CSATriggerCount_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 count;

	if (arg == NULL)
		goto error;
	count = os_str_tol(arg, 0, 10);
	MTWF_PRINT("%s():count(%d)\n",  __func__, count);

	/*set customer channel switch trigger in Dot11h struct
	* some customer need channel switch trigger at custom CSA count(1) and not 0
	*/
	/*todo: support separate Channel Switch Period per band*/
	pAd->Dot11_H.ChannelSwitchTriggerCSACount = count;

error:
	return true;
}

INT Set_ZeroPktLossEnable_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 ZeroPktLossEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	if (arg == NULL)
		goto error;

	if (wdev == NULL)
		goto error;

	ZeroPktLossEnable = os_str_tol(arg, 0, 10);
	pAd->Zero_Loss_Enable = ZeroPktLossEnable;
	MTWF_PRINT("%s():ZeroPktLossEnable(%d)\n",  __func__, ZeroPktLossEnable);

	/*send fw command to set ZeroPktLoss Functionality enable in fw*/
	UniCmdZeroLossEnable(pAd, ZeroPktLossEnable);
error:
	return true;
}

INT Set_CsaActionFrameEnable_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 CsaActionFrameEnable;

	if (arg == NULL)
		goto error;

	CsaActionFrameEnable = os_str_tol(arg, 0, 10);
	pAd->Csa_Action_Frame_Enable = CsaActionFrameEnable;
	MTWF_PRINT("%s():CsaActionFrameEnable(%d)\n",  __func__, CsaActionFrameEnable);

error:
	return true;
}

INT Set_StaPsQLimit_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
//	pAd->ZeroLossStaPsQLimit = (USHORT) simple_strtol(arg, 0, 10);

	MTWF_PRINT("%s::(ZeroLossStaPsQLimit = %d)\n", __func__, pAd->ZeroLossStaPsQLimit);

	return TRUE;
}

INT Set_MacTxEnable_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	UINT8 MacTxEnable;
	UCHAR BandIdx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	BandIdx = HcGetBandByWdev(wdev);

	if (arg == NULL)
		goto error;

	if (wdev == NULL)
		goto error;

	MacTxEnable = os_str_tol(arg, 0, 10);
	//pAd->Zero_Loss_Enable = MacTxEnable;
	MTWF_PRINT("%s():MacTxEnable(%d)\n",  __func__, MacTxEnable);

	/*send fw command to set ZeroPktLoss Functionality enable in fw*/
	if (MacTxEnable == 1)
		UniCmdMacTxEnable(pAd, TRUE, BandIdx);
	else if (MacTxEnable == 0)
		UniCmdMacTxEnable(pAd, FALSE, BandIdx);
	else
		goto error;
error:
	return true;
}
#endif /*ZERO_PKT_LOSS_SUPPORT*/

#ifdef OFFCHANNEL_SCAN_FEATURE
INT Set_ScanResults_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT32 ch_index = 0;

	ch_index = Channel2Index(pAd, pAd->ChannelInfo.ChannelNo);
	MTWF_PRINT("Channel : %d NF value : %ddb \tBusyTime : %dusec\n",
				pAd->ChannelInfo.ChannelNo, pAd->ChannelInfo.AvgNF,
				pAd->ChannelInfo.chanbusytime[ch_index]);

	pAd->ChannelInfo.bandidx = 0;
	pAd->ChannelInfo.ChannelNo = 0;
	pAd->ChannelInfo.AvgNF = 0;
	pAd->ChannelInfo.chanbusytime[ch_index] = 0;
	return TRUE;
}

INT Set_ApScan_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UINT channel = 0;
	UINT OriChannel = 0;
	UINT timeout = 0;
	UINT i = 0, j = 0, count = 0;
	CHAR scantype[8] = {0};
	CHAR temp[33];
	UINT ifIndex;
	struct wifi_dev *wdev = NULL;
	SCAN_CTRL *ScanCtrl = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/* check if the interface is down */
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
		MTWF_PRINT("INFO::Network is down!\n");
		return -ENETDOWN;
	}

	ifIndex = pObj->ioctl_if;
	if (pObj->ioctl_if_type == INT_MBSSID) {
		if (ifIndex < MAX_BEACON_NUM) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else {
			MTWF_PRINT("invalid mbss entry id(%d)\n", ifIndex);
			return FALSE;
		}
	} else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;

	while (arg[j] != '\0') {
		temp[i] = arg[j++];
		if (temp[i] == ':') {
			switch (++count) {
			case 1:
				temp[i] = '\0';
				if ((strlen(temp) != 0) && (strlen(temp) <= 7)) {
					strlcpy(scantype, temp, sizeof(scantype));
					if (strcmp(scantype, "active") && strcmp(scantype, "passive")) {
						MTWF_PRINT("wrong scan type argument\n");
						return FALSE;
					}
				} else {
					MTWF_PRINT("wrong scan type argument\n");
					return FALSE;
				}
				i = 0;
				temp[i] = arg[j++];
				break;
			case 2:
				temp[i] = '\0';
				if ((strlen(temp) != 0) && (strlen(temp) <= 3)) {
					channel = simple_strtol(temp, 0, 10);
					OriChannel = wdev->channel;
					if (!IsValidChannel(pAd, channel, wdev)) {
						MTWF_PRINT("wrong channel number %d\n", channel);
						return FALSE;
					}
				} else {
					MTWF_PRINT("wrong channel number\n");
					return FALSE;
				}
				i = 0;
				temp[i] = arg[j++];
				break;
			default:
				MTWF_PRINT("wrong number of arguments\n");
				return FALSE;
			}
		} else if (arg[j] == '\0') {
			temp[i+1] = '\0';
			if ((strlen(temp) != 0) && (strlen(temp) <= 10) && (simple_strtol(temp, 0, 10) < 0xffffffff)) {
				timeout = simple_strtol(temp, 0, 10);
			} else if (strlen(temp)) {
				MTWF_PRINT("wrong Timeout value\n");
				return FALSE;
			}
		}
		i++;
	}

	/*To do OffChannelScan, need TakeChannelOpCharge first*/
	if (!TakeChannelOpCharge(pAd, wdev, CH_OP_OWNER_SCAN, TRUE)) {
		MTWF_PRINT("TakeChannelOpCharge fail for Off-Channel SCAN!!\n");
		return FALSE;
	}

	ScanCtrl = get_scan_ctrl_by_wdev(pAd, wdev);
	/* Make compatible with application path */
	ScanCtrl->Num_Of_Channels = 1;
	ScanCtrl->ScanTime[0] = 0;
	ScanCtrl->CurrentGivenChan_Index = 0;
	ScanCtrl->state = OFFCHANNEL_SCAN_START;
	if (!strcmp(scantype, "passive"))
		ApSiteSurveyNew_by_wdev(pAd, channel, timeout, SCAN_PASSIVE, FALSE, wdev);
	else if (!strcmp(scantype, "active"))
		ApSiteSurveyNew_by_wdev(pAd, channel, timeout, SCAN_ACTIVE, FALSE, wdev);
	MTWF_PRINT("%s end.\n", __func__);
	return TRUE;
}
#endif
#ifdef NEW_SET_RX_STREAM
INT	Set_RxStream_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UINT RxStream;

	RxStream = os_str_tol(arg, 0, 10);
	AsicSetRxStream(pAd, RxStream, 0);
	return TRUE;
}
#endif

INT	Set_Lp_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR lp_enable;
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT ifIndex = pObj->ioctl_if;

#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMBSS = NULL;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (ifIndex >= MAX_BEACON_NUM) {
			MTWF_PRINT("invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		pMBSS = &pAd->ApCfg.MBSSID[ifIndex];
		wdev = &pMBSS->wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (ifIndex < pAd->MSTANum)
			wdev = &pAd->StaCfg[ifIndex].wdev;
		else {
			MTWF_PRINT("invalid sta entry id(%d)\n", ifIndex);
			return FALSE;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	lp_enable = os_str_tol(arg, 0, 10);

	if (lp_enable) {
		MlmeLpEnter(pAd);
		MTWF_PRINT("==>%s (Enetr)\n", __func__);
	} else {
		MlmeLpExit(pAd);
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			APStartUp(pAd, pMBSS, AP_BSS_OPER_BY_RF);
		}
#endif /* CONFIG_AP_SUPPORT */
		MTWF_PRINT("==>%s (Exit)\n", __func__);
	}

	return TRUE;
}

#ifdef MT_MAC
INT setTmrVerProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	CHAR ver;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	ver = (CHAR)os_str_tol(arg, 0, 10);

	if ((ver < TMR_VER_1_0) || (ver > TMR_VER_2_0)) {
		MTWF_PRINT("wrong setting %d, remain default %d!!\n",
			 ver, pChipCap->TmrHwVer);
		return FALSE;
	}

	MTWF_PRINT("%s: ver = %d, pChipCap->TmrHwVer = %d\n",
		  __func__, ver, pChipCap->TmrHwVer);

	return TRUE;
}

INT setTmrEnableProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	LONG enable;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	enable = os_str_tol(arg, 0, 10);

	if ((enable < TMR_DISABLE) || (enable > TMR_RESPONDER)) {
		MTWF_PRINT("enable is incorrect!!\n");
		return FALSE;
	}

	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_PRINT("chipcap is not HIF_MT\n");
		return FALSE;
	}

	TmrCtrl(pAd, (UCHAR)enable, cap->TmrHwVer);
	return TRUE;
}

#ifndef COMPOS_TESTMODE_WIN
INT SetTmrCalProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  RTMP_STRING *arg)
{
	struct os_cookie *obj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, obj->ioctl_if, obj->ioctl_if_type);
	UCHAR TmrType = os_str_tol(arg, 0, 10);
	UCHAR Channel = HcGetRadioChannel(pAd);

	MTWF_PRINT("%s(): TMR Calibration, TmrType: %d\n", __func__, TmrType);

	AsicSetTmrCal(pAd, TmrType, Channel, wlan_operate_get_bw(wdev));
	return TRUE;
}
#endif



INT set_cr4_query(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);

	if (arg == NULL) {
		MTWF_PRINT("Invalid parameters\n");
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Query(pAd, option, 0, 0);
	return TRUE;
}

INT set_cr4_set(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 arg0 = 0;
	UINT32 arg1 = 0;
	UINT32 arg2 = 0;
	RTMP_STRING *arg0_ptr = NULL;
	RTMP_STRING *arg1_ptr = NULL;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);
	arg0_ptr = strsep(&arg, ":");
	arg1_ptr = strsep(&arg, ":");

	if (arg0_ptr == NULL || arg1_ptr == NULL || arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	arg0 = os_str_toul(arg0_ptr, 0, 16);
	arg1 = os_str_toul(arg1_ptr, 0, 16);
	arg2 = os_str_toul(arg, 0, 16);
	MTWF_PRINT("%s: arg0 = 0x%x, arg1 = 0x%x, arg2 = 0x%x\n",
		  __func__, arg0, arg1, arg2);
	MtCmdCr4Set(pAd, arg0, arg1, arg2);
	return TRUE;
}

INT set_cr4_capability(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);

	if (arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Capability(pAd, option);
	return TRUE;
}

INT set_cr4_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 option = 0;

	MTWF_PRINT("arg = %s\n", arg);

	if (arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	option = os_str_toul(arg, 0, 16);
	MtCmdCr4Debug(pAd, option);
	return TRUE;
}

INT set_re_calibration(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 BandIdx = 0;
	UINT32 CalItem = 0;
	UINT32 CalItemIdx = 0;
	RTMP_STRING *pBandIdx  = NULL;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);
	pBandIdx = strsep(&arg, ":");

	if (pBandIdx == NULL || arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	BandIdx = os_str_toul(pBandIdx, 0, 10);
	CalItem = os_str_toul(arg, 0, 10);

	if (BandIdx > 1) {
		MTWF_PRINT(" Unknown BandIdx = %d\n", BandIdx);
		return FALSE;
	}

	if ((CalItem > 12) || (CalItem == 3) || (CalItem == 4)) {
		MTWF_PRINT(" Unknown CalItem = %d\n", CalItem);
		return FALSE;
	}

	MTWF_PRINT(":%s: BandIdx: %d, CalItem: %d\n", __func__, BandIdx, CalItem);

	switch (CalItem) {
	case 0:
		MTWF_PRINT(":%s: RC_CAL\n", __func__);
		CalItemIdx = RC_CAL;
		break;

	case 1:
		MTWF_PRINT(":%s: RX_RSSI_DCOC_CAL\n", __func__);
		CalItemIdx = RX_RSSI_DCOC_CAL;
		break;

	case 2:
		MTWF_PRINT(":%s: RX_DCOC_CAL\n", __func__);
		CalItemIdx = RX_DCOC_CAL;
		break;

	case 5:
		MTWF_PRINT(":%s: RX_FIIQ_CAL\n", __func__);
		CalItemIdx = RX_FIIQ_CAL;
		break;

	case 6:
		MTWF_PRINT(":%s: RX_FDIQ_CAL\n", __func__);
		CalItemIdx = RX_FDIQ_CAL;
		break;

	case 7:
		MTWF_PRINT(":%s: TX_DPD_LINK\n", __func__);
		CalItemIdx = TX_DPD_LINK;
		break;

	case 8:
		MTWF_PRINT(":%s: TX_LPFG\n", __func__);
		CalItemIdx = TX_LPFG;
		break;

	case 9:
		MTWF_PRINT(":%s: TX_DCIQC\n", __func__);
		CalItemIdx = TX_DCIQC;
		break;

	case 10:
		MTWF_PRINT(":%s: TX_IQM\n", __func__);
		CalItemIdx = TX_IQM;
		break;

	case 11:
		MTWF_PRINT(":%s: TX_PGA\n", __func__);
		CalItemIdx = TX_PGA;
		break;

	case 12:
		MTWF_PRINT(":%s: CAL_ALL\n", __func__);
		CalItemIdx = CAL_ALL;
		break;

	default:
		MTWF_PRINT(" Unknown CalItem = %d\n", CalItem);
		break;
	}

	MtCmdDoCalibration(pAd, RE_CALIBRATION, CalItemIdx, BandIdx);
	return TRUE;
}

/*
 * iwpriv ra0 set ThermalMode=mode:action
 * mode   = 1: Thermal re-cal
 *        = 2: Dynamic G0
 *        = 3: Apply High Rate DPD table (Note: This mode only disable/enable)
 * action = 0: Disable
 *        = 1: Enable
 *        = 2: Trigger
 */
INT set_thermal_dbg_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define THERMAL_MODE_RECAL      1
#define THERMAL_MODE_DYNAMIC_G0 2
#define THERMAL_MODE_HRATE_DPD  3
#define THERMAL_MODE_NT_LT      4
#define THERMAL_MODE_NT_HT      5
#define THERMAL_MODE_MANUAL_DBG_THERMAL    6

#define THERMAL_ACTION_DISABLE  0
#define THERMAL_ACTION_ENABLE   1
#define THERMAL_ACTION_TRIGGER  2
#define THERMAL_ACTION_NT2LT_TRIGGER 2
#define THERMAL_ACTION_LT2NT_TRIGGER 3
#define THERMAL_ACTION_NT2HT_TRIGGER 2
#define THERMAL_ACTION_HT2NT_TRIGGER 3

	UINT8 mode = 0;
	UINT8 action = 0;
	RTMP_STRING *cmd_str  = NULL;

	MTWF_PRINT(":%s: arg = %s\n", __FUNCTION__, arg);

	cmd_str = strsep(&arg, ":");

	if (cmd_str == NULL || arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	mode = os_str_toul(cmd_str, 0, 10);
	action = os_str_toul(arg, 0, 10);

	MTWF_PRINT(":%s: mode(%d), action(%d))\n", __FUNCTION__, mode, action);

	switch (mode) {
	case THERMAL_MODE_RECAL:
	{
		if (action > THERMAL_ACTION_TRIGGER) {
			MTWF_PRINT("Unknown action = %d", action);
			MTWF_PRINT(
			"(0: thermal recal OFF; 1: thermal recal ON; 2: trigger thermal recal)\n");
			return FALSE;
		}

		if ((action == THERMAL_ACTION_TRIGGER) && (pAd->CommonCfg.ThermalRecalMode == THERMAL_ACTION_DISABLE)) {
			MTWF_PRINT("Can't trigger recal in Thermal recal off mode\n");

			return FALSE;
		}

		pAd->CommonCfg.ThermalRecalMode = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_DYNAMIC_G0:
	{
		if (action > THERMAL_ACTION_TRIGGER) {
			MTWF_PRINT("Unknown action = %d", action);
			MTWF_PRINT(
				"(0: Dynamic G0 OFF; 1: Dynamic G0 ON; 2: trigger Dynamic G0)\n");
			return FALSE;
		}

		if ((action == THERMAL_ACTION_TRIGGER) && (pAd->CommonCfg.ThermalDynamicG0Action == THERMAL_ACTION_DISABLE)) {
			MTWF_PRINT(" Can't trigger G0 in Dynamic G0 off mode\n");

			return FALSE;
		}

		pAd->CommonCfg.ThermalDynamicG0Action = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_HRATE_DPD:
	{
		if (action > THERMAL_ACTION_ENABLE) {
			MTWF_PRINT(" Unknown action = %d (0: H-rate DPD OFF; 1: H-rate DPD ON;)\n",
				action);

			return FALSE;
		}

		pAd->CommonCfg.ThermalHRateDpdMode = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_NT_LT:
	{
		if (action > THERMAL_ACTION_LT2NT_TRIGGER) {
			MTWF_PRINT(
			"Unknown action = %d (NTLT 0:OFF; 1:ON; 2:trigger NT2LT; 3:trigger LT2NT)\n",
			action);

			return FALSE;
		}

		if ((pAd->CommonCfg.ThermalNtLtAction == THERMAL_ACTION_DISABLE) &&
			 ((action == THERMAL_ACTION_NT2LT_TRIGGER) ||
			  (action == THERMAL_ACTION_LT2NT_TRIGGER))) {
			MTWF_PRINT(" Can't trigger NT2LT/LT2NT in NTLT off mode\n");

			return FALSE;
		}

		pAd->CommonCfg.ThermalNtLtAction = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_NT_HT:
	{
		if (action > THERMAL_ACTION_HT2NT_TRIGGER) {
			MTWF_PRINT(
			"Unknown action = %d (NTHT 0:OFF; 1:ON; 2:trigger NT2HT; 3:trigger HT2NT)\n",
			action);

			return FALSE;
		}

		pAd->CommonCfg.ThermalNtLtAction = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	case THERMAL_MODE_MANUAL_DBG_THERMAL:
	{
		if (action > THERMAL_ACTION_TRIGGER) {
			MTWF_PRINT(
			"Unknown action = %d (manual 0:OFF; 1:ON; reserved )\n",
			action);

			return FALSE;
		}

		MTWF_PRINT(" Manual dbg action: %d\n", action);


		pAd->CommonCfg.ThermalNtLtAction = action;
		MtCmdThermalMode(pAd, mode, action);
	}
	break;

	default:
		MTWF_PRINT(" Unknown mode(%d) (1: Thermal re-cal; 2: Dynamic G0) )\n", mode);
		break;
	}

#undef THERMAL_ACTION_DISABLE
#undef THERMAL_ACTION_ENABLE
#undef THERMAL_ACTION_TRIGGER
#undef THERMAL_ACTION_NT2LT_TRIGGER
#undef THERMAL_ACTION_LT2NT_TRIGGER
#undef THERMAL_ACTION_NT2HT_TRIGGER
#undef THERMAL_ACTION_HT2NT_TRIGGER

#undef THERMAL_MODE_DYNAMIC_G0
#undef THERMAL_MODE_RECAL
#undef THERMAL_MODE_HRATE_DPD
#undef THERMAL_MODE_NT_LT
#undef THERMAL_MODE_NT_HT
#undef THERMAL_MODE_MANUAL_DBG_THERMAL

	return TRUE;
}

INT set_noise_floor_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef WIFI_UNIFIED_COMMAND
	UINT8 mode = 0;
	UINT8 action = 0;
	RTMP_STRING *cmd_str  = NULL;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);

	cmd_str = strsep(&arg, ":");

	if (cmd_str == NULL || arg == NULL) {
		MTWF_PRINT(":%s: Invalid parameters\n", __func__);
		return FALSE;
	}

	mode = os_str_toul(cmd_str, 0, 10);
	action = os_str_toul(arg, 0, 10);

	MTWF_PRINT(":%s: mode(%d), action(%d))\n", __func__, mode, action);

	switch (mode) {
	case UNI_CMD_NOISE_FLOOR_MODE_RECAL:
	{
		if (action > UNI_CMD_NOISE_FLOOR_ACTION_TRIGGER) {
			MTWF_PRINT(":%s: Unknown action = %d", __func__, action);
			MTWF_PRINT("(0: thermal recal OFF)\n"
				   "(1: thermal recal ON)\n"
				   "(2: trigger thermal recal)\n");
			return FALSE;
		}

		if ((action == UNI_CMD_NOISE_FLOOR_ACTION_TRIGGER) && (pAd->CommonCfg.ThermalRecalMode == UNI_CMD_NOISE_FLOOR_ACTION_DISABLE)) {
			MTWF_PRINT(":%s: Can't trigger recal in Thermal recal off mode\n",
				__func__);

			return FALSE;
		}

		pAd->CommonCfg.ThermalRecalMode = action;
		UniCmdSetNoiseFloorMode(pAd, mode, action);
	}
	break;

	case UNI_CMD_NOISE_FLOOR_MODE_DYNAMIC_G0:
	{
		if (action > UNI_CMD_NOISE_FLOOR_ACTION_TRIGGER) {
			MTWF_PRINT(":%s: Unknown action = %d",
				__func__, action);
			MTWF_PRINT("(0:Dynamic G0 OFF; 1:Dynamic G0 ON; 2:trigger Dynamic G0)\n");
			return FALSE;
		}

		if ((action == UNI_CMD_NOISE_FLOOR_ACTION_TRIGGER) && (pAd->CommonCfg.ThermalDynamicG0Action == UNI_CMD_NOISE_FLOOR_ACTION_DISABLE)) {
			MTWF_PRINT(":%s: Can't trigger G0 in Dynamic G0 off mode\n", __func__);
			return FALSE;
		}

		pAd->CommonCfg.ThermalDynamicG0Action = action;
		UniCmdSetNoiseFloorMode(pAd, mode, action);
	}
	break;

	case UNI_CMD_NOISE_FLOOR_MODE_APPLY_HIGH_DPD:
	{
		if (action > UNI_CMD_NOISE_FLOOR_ACTION_ENABLE) {
			MTWF_PRINT(
				":%s: Unknown action = %d (0: H-rate DPD OFF; 1: H-rate DPD ON;)\n",
				__func__, action);
			return FALSE;
		}

		pAd->CommonCfg.ThermalHRateDpdMode = action;
		UniCmdSetNoiseFloorMode(pAd, mode, action);
	}
	break;

	case UNI_CMD_NOISE_FLOOR_MODE_NTLT:
	{
		if (action > UNI_CMD_NOISE_FLOOR_ACTION_LT2NT_TRIGGER) {
			MTWF_PRINT(":%s: Unknown action = %d", __func__, action);
			MTWF_PRINT("(NTLT 0:OFF; 1:ON; 2:trigger NT2LT; 3:trigger LT2NT)\n");
			return FALSE;
		}

		if ((pAd->CommonCfg.ThermalNtLtAction == UNI_CMD_NOISE_FLOOR_ACTION_DISABLE) &&
			 ((action == UNI_CMD_NOISE_FLOOR_ACTION_NT2LT_TRIGGER) ||
			  (action == UNI_CMD_NOISE_FLOOR_ACTION_LT2NT_TRIGGER))) {
			MTWF_PRINT(":%s: Can't trigger NT2LT/LT2NT in NTLT off mode\n", __func__);

			return FALSE;
		}

		pAd->CommonCfg.ThermalNtLtAction = action;
		UniCmdSetNoiseFloorMode(pAd, mode, action);
	}
	break;


	default:
		MTWF_PRINT(":%s: Unknown mode(%d) (1: Noise Floow re-cal; 2: Dynamic G0) )\n",
			__func__, mode);
		break;
	}
#endif
	return TRUE;
}


INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 McuDest = 0;
	UINT32 LogType = 0;
	BOOLEAN invalid = FALSE, last_wmcpu_log_type = 0;
	UINT8 bitmap;
	RTMP_STRING *pMcuDest  = NULL;
	RTMP_STRING *Dest[]  = {
		"HOST2N9(WM)",
		"HOST2CR4(WA)",
		"HOST2WO"
	};
	RTMP_STRING *info[] = {
		"Print MCU Log to UART",
		"Send MCU log by Event",
#ifdef FW_LOG_DUMP
		"Send MCU log to EMI(currently not support)",
		"Send MCU log by Event to Storage",
		"Send MCU log by Event to Ethernet",
		"Send MCU log by Event with CONNSYS format",
		"Send MCU log by Event to Memory"
#endif
	};
#ifdef WIFI_UNIFIED_COMMAND
	UINT8 host_2_mcu = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);
	pMcuDest = strsep(&arg, ":");

	if (pMcuDest == NULL || arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	McuDest = os_str_toul(pMcuDest, 0, 10);
	LogType = os_str_toul(arg, 0, 10);

	if (McuDest < 3) {
		if ((LogType & ~SUPPORTED_FW_LOG_TYPE) != 0)
			invalid = TRUE;
#ifdef FW_LOG_DUMP
		else {
			last_wmcpu_log_type = pAd->physical_dev->fw_log_ctrl.wmcpu_log_type;
			pAd->physical_dev->fw_log_ctrl.wmcpu_log_type = LogType;
		}
#endif /* FW_LOG_DUMP */
	} else if (McuDest > 2)
		invalid = TRUE;

	if (invalid) {
		MTWF_PRINT(" Unknown Mcu Dest = %d, Log Type = %x\n",
			  McuDest, LogType);
		return FALSE;
	}

	MTWF_PRINT("Mcu Dest = %s\n", Dest[McuDest]);

	if (LogType == 0) {
		MTWF_PRINT("Log Type = Disable MCU Log Message\n");

		if ((last_wmcpu_log_type == 96) || (last_wmcpu_log_type == 97)) {
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_ALLOC_MEMORY, (VOID *)&LogType,
				sizeof(UINT32));
		} else {
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_OPEN_CLOSE_FILE, (VOID *)&LogType,
				sizeof(UINT32));
		}
	} else {
		for (bitmap = 0; bitmap < FW_LOG_TYPE_COUNT; bitmap++)
			if (LogType & (1 << bitmap))
				MTWF_PRINT("Log Type = %s\n", info[bitmap]);
		if ((LogType == 8) || (LogType == 9) || (LogType == 40) || (LogType == 41)) {
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_OPEN_CLOSE_FILE, (VOID *)&LogType,
				sizeof(UINT32));
		}
		if ((LogType == 96) || (LogType == 97)) {
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_ALLOC_MEMORY, (VOID *)&LogType,
				sizeof(UINT32));
		}
	}
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		if (McuDest == 0)
			host_2_mcu = HOST2N9;
		else if (McuDest == 1)
			host_2_mcu = HOST2CR4;
		else
			host_2_mcu = HOST2WO;
		if ((LogType == 96) || (LogType == 97))
			LogType = 40;
		MtUniCmdFwLog2Host(pAd, host_2_mcu, LogType);
	} else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdFwLog2Host(pAd, McuDest, LogType);
	return TRUE;
}

INT set_fw_dbg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *dbg_lvl_str = NULL;
	UINT8 dbg_lvl;
	UINT32 dbg_module_idx;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	dbg_lvl_str = strsep(&arg, ":");
	dbg_lvl = os_str_toul(dbg_lvl_str, 0, 10);

	if (arg == NULL || strlen(arg) == 0) {
		/* imply all modules */
		dbg_module_idx = 0xffffffff;
	} else {
		dbg_module_idx = os_str_toul(arg, 0, 10);
	}

	MTWF_PRINT("%s: set fw debug level to 0x%x for module index 0x%x\n", __func__, dbg_lvl, dbg_module_idx);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		MtUniCmdFwDbgCtrl(pAd, dbg_lvl, dbg_module_idx);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdFwDbgCtrl(pAd, dbg_lvl, dbg_module_idx);
	return TRUE;

error:
	MTWF_PRINT("Usage: fw_dbg=[debug level bitmap]:[debug module index]\n");
	MTWF_PRINT("\tdebug level bitmap:\n");
	MTWF_PRINT("\t\t DBG_CLASS_ERROR (1 << 0)\n");
	MTWF_PRINT("\t\t DBG_CLASS_WARN  (1 << 1)\n");
	MTWF_PRINT("\t\t DBG_CLASS_STATE (1 << 2)\n");
	MTWF_PRINT("\t\t DBG_CLASS_INFO  (1 << 3)\n");
	MTWF_PRINT("\t\t DBG_CLASS_LOUD  (1 << 4)\n");
	MTWF_PRINT("\tdebug module index:\n");
	MTWF_PRINT("\t\t If not specified, means all modules.\n");
	MTWF_PRINT("Example:\n");
	MTWF_PRINT("\t iwpriv ra0 set fw_dbg=3 (turn on ERROR and WARN for all modules\n");
	MTWF_PRINT("\t iwpriv ra0 set fw_dbg=1:0 (turn on ERROR for module 0\n");

	return FALSE;
}

#ifdef FW_LOG_DUMP
INT set_host_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 LogCtrl = 0;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);

	if (arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	LogCtrl = os_str_toul(arg, 0, 10);

	pAd->physical_dev->host_log_ctrl = LogCtrl;
	MTWF_PRINT("Host LogCtrl = %u\n", LogCtrl);

	if (LogCtrl == 0)
		MTWF_PRINT("Host Log Disable to log parser\n");

	return TRUE;
}

INT set_host_dump_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 offset = 0;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);

	if (arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	offset = os_str_toul(arg, 0, 10);

	pAd->physical_dev->host_dump_offset = offset;
	MTWF_PRINT("Host dump payload offset = %u\n", offset);

	if (offset == 0)
		MTWF_PRINT("Host dump payload disable to log parser\n");

	return TRUE;
}

#endif /* FW_LOG_DUMP */
INT set_isr_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#define ISR_CMD_CMD_ID_OFFSET 24
#define ISR_CMD_SUBCMD_ID_OFFSET 0
#define ISR_CMD_CMD_ID_MASK 0x7f
#define ISR_CMD_SUBCMD_ID_MASK 0xffffff
	UINT32 cmd = 0;
	UINT32 sub_cmd = 0;
	RTMP_STRING *cmd_str  = NULL;
	UINT32 tmp = 0;
	UINT32 tmp2;

	MTWF_PRINT(":%s: arg = %s\n", __func__, arg);
	cmd_str = strsep(&arg, ":");

	if (cmd_str == NULL || arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	cmd = os_str_toul(cmd_str, 0, 10);
	sub_cmd = os_str_toul(arg, 0, 10);

	cmd &= ISR_CMD_CMD_ID_MASK;
	sub_cmd &= ISR_CMD_SUBCMD_ID_MASK;

	HW_IO_READ32(pAd->hdev_ctrl, PLE_TO_N9_INT, &tmp);
	tmp2 = (tmp ^ PLE_TO_N9_INT_TOGGLE_MASK) & PLE_TO_N9_INT_TOGGLE_MASK;

	tmp2 |= (cmd << ISR_CMD_CMD_ID_OFFSET);
	tmp2 |= (sub_cmd << ISR_CMD_SUBCMD_ID_OFFSET);

	HW_IO_WRITE32(pAd->hdev_ctrl, PLE_TO_N9_INT, tmp2);

	return TRUE;
}

UINT16 txop0;
UINT16 txop60 = 0x60;
UINT16 txop80 = 0x80;
UINT16 txopfe = 0xfe;
INT set_txop_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *str  = NULL;
	RTMP_STRING *str2  = NULL;

	MTWF_PRINT(":%s: current setting txop 0=%x, txop 60=%x, txop 80=%x, txop fe=%x\n",
		  __func__, txop0, txop60, txop80, txopfe);

	str = strsep(&arg, ":");
	if (str == NULL || arg == NULL) {
		MTWF_PRINT(" Invalid parameters\n");
		return FALSE;
	}

	do {
		str2 = strsep(&arg, ":");
		if (str2 == NULL)
			str2 = arg;

		switch (os_str_toul(str, 0, 16)) {
		case 0x0:
			txop0 = os_str_toul(str2, 0, 16);
			MTWF_PRINT(":%s: txop0 change to %x\n", __func__, txop0);
			break;
		case 0x60:
			txop60 = os_str_toul(str2, 0, 16);
			MTWF_PRINT(":%s: txop60 change to %x\n", __func__, txop60);
			break;
		case 0x80:
			txop80 = os_str_toul(str2, 0, 16);
			MTWF_PRINT(":%s: txop80 change to %x\n", __func__, txop80);
			break;
		case 0xfe:
			txopfe = os_str_toul(str2, 0, 16);
			MTWF_PRINT(":%s: txop60 change to %x\n", __func__, txopfe);
			break;
		default:
			MTWF_PRINT(":%s: not support txop%lx\n", __func__, os_str_toul(str, 0, 16));
			break;
		}

		str = strsep(&arg, ":");
	} while (str != NULL);

	return TRUE;
}

INT set_rts_cfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	MTWF_PRINT("This command was disabled because we do NOT allow to write mac cr directly.\n");
	return FALSE;
}

/* SER user command */
#define SER_USER_CMD_DISABLE		 0
#define SER_USER_CMD_ENABLE		  1

#define SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY	  (200)
#define SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY	(201)
#define SER_USER_CMD_ENABLE_MASK_L2_RECOVER_ONLY	(202)
#define SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY   (203)
#define SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY   (204)
#define SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY (205)
#define SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY  (206)
#define SER_USER_CMD_ENABLE_MASK_RECOVER_ALL		(207)

/* Use a magic number to prevent human mistake */
#define SER_USER_CMD_L1_RECOVER		  995

#define SER_USER_CMD_L2_RECOVER			(300)
#define SER_USER_CMD_L3_RX_ABORT		(302)
#define SER_USER_CMD_L3_TX_ABORT		(304)
#define SER_USER_CMD_L3_TX_DISABLE		(306)
#define SER_USER_CMD_L3_BF_RECOVER		(308)
#define SER_USER_CMD_L4_RRO_LEAK_RECOVER		(309)


INT set_time_sync_trigger(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef LINUX
	INT32 ret = 0;
	UINT32 second = 0;
	UINT32 usecond = 0;
#if (KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE)
	struct timespec64 time = { .tv_sec = 0, .tv_nsec = 0 };
#else
	struct timeval time;
#endif
	struct tm current_tm;

#ifdef WIFI_UNIFIED_COMMAND
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif

	MTWF_PRINT("Trigger Time Sync to FW!\n");

	ret = sscanf(arg, "%u-%u", &second, &usecond);

	if (ret != 2) {
		MTWF_PRINT("Format: iwpriv ra0 set time_sync=[second]-[usecond]\n");

#if (KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE)
		ktime_get_real_ts64(&time);
		second = (unsigned int)time.tv_sec;
		usecond = (unsigned int)time.tv_nsec/1000;
#else
		do_gettimeofday(&time);
		second = (unsigned int)time.tv_sec;
		usecond = (unsigned int)time.tv_usec;
#endif
		MTWF_PRINT("Use System Time (second:%u, usecond:%u)\n", second, usecond);
	} else {
		MTWF_PRINT("Use Manual Time (second:%u, usecond:%u)\n", second, usecond);
	}

	time64_to_tm((time64_t) time.tv_sec, 0, &current_tm);
	MTWF_PRINT("%ld-%02d-%02d %02d:%02d:%02d.%u UTC\n",
	current_tm.tm_year + 1900, current_tm.tm_mon + 1, current_tm.tm_mday,
	current_tm.tm_hour, current_tm.tm_min, current_tm.tm_sec, usecond);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		MtUniCmdFwTimeSync(pAd, second, usecond);
#endif
#endif /* LINUX */
	return TRUE;
}

INT set_ser(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 cmdId = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	UINT8 band_idx;

	if (arg == NULL)
		goto ser_usage;

	cmdId = os_str_toul(arg, 0, 10);
	band_idx = hc_get_hw_band_idx(pAd);

	switch (cmdId) {
	case SER_USER_CMD_DISABLE:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE, SER_SET_DISABLE, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET, SER_SET_DISABLE, band_idx);
		break;

	case SER_USER_CMD_ENABLE:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE, SER_SET_ENABLE, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET, SER_SET_ENABLE, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_TRACKING_ONLY:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_L1_RECOVER_ONLY:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L1_RECOVER, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L1_RECOVER, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_L2_RECOVER_ONLY:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L2_RECOVER, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L2_RECOVER, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_RX_ABORT_ONLY:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_RX_ABORT, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_RX_ABORT, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_TX_ABORT_ONLY:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_ABORT, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_ABORT, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_TX_DISABLE_ONLY:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_DISABLE, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_TX_DISABLE, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_L3_BFRECOVER_ONLY:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_BF_RECOVER, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK, SER_ENABLE_TRACKING | SER_ENABLE_L3_BF_RECOVER, band_idx);
		break;

	case SER_USER_CMD_ENABLE_MASK_RECOVER_ALL:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_ENABLE_MASK,
				   (SER_ENABLE_TRACKING |
				   SER_ENABLE_L1_RECOVER | SER_ENABLE_L2_RECOVER |
				   SER_ENABLE_L3_RX_ABORT | SER_ENABLE_L3_TX_ABORT |
				   SER_ENABLE_L3_TX_DISABLE | SER_ENABLE_L3_BF_RECOVER), band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_SET_ENABLE_MASK,
				  (SER_ENABLE_TRACKING |
				   SER_ENABLE_L1_RECOVER | SER_ENABLE_L2_RECOVER |
				   SER_ENABLE_L3_RX_ABORT | SER_ENABLE_L3_TX_ABORT |
				   SER_ENABLE_L3_TX_DISABLE | SER_ENABLE_L3_BF_RECOVER), band_idx);
		break;

	case SER_USER_CMD_L1_RECOVER:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L1_RECOVER, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L1_RECOVER, band_idx);
		break;

	case SER_USER_CMD_L2_RECOVER:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L2_RECOVER, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L2_RECOVER, band_idx);
		break;

	case SER_USER_CMD_L3_RX_ABORT:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L3_RX_ABORT, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_RX_ABORT, band_idx);
		break;

	case SER_USER_CMD_L3_TX_ABORT:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L3_TX_ABORT, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX_ABORT, band_idx);
		break;

	case SER_USER_CMD_L3_TX_DISABLE:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L3_TX_DISABLE, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_TX_DISABLE, band_idx);
		break;

	case SER_USER_CMD_L3_BF_RECOVER:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L3_BF_RECOVER, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L3_BF_RECOVER, band_idx);
		break;
	case SER_USER_CMD_L4_RRO_LEAK_RECOVER:
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_L4_RRO_LEAK_RECOVER, band_idx);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtSER(pAd, SER_ACTION_RECOVER, SER_SET_L4_RRO_LEAK_RECOVER, band_idx);
		break;

	default:
		goto ser_usage;
	}

	return TRUE;
ser_usage:
	return TRUE;
}

INT32 set_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	CHAR *value;
	UCHAR *Input;
	UCHAR ExtendID;
	INT i, len;
	BOOLEAN fgStatus = FALSE;
	/* get ExtID */
	value = rstrtok(Arg, ":");
	if (value == NULL)
		return fgStatus;
	AtoH(value, &ExtendID, 1);
	/* get cmd raw data */
	value += 3;
	len = strlen(value) >> 1;
	MTWF_PRINT("=======Set_FwCmd==========\n");
	os_alloc_mem(pAd, (UCHAR **)&Input, len);
	if (!Input) {
		MTWF_PRINT("%s alloc mem failed!\n", __func__);
		return fgStatus;
	}

	for (i = 0; i < len; i++)
		AtoH(value + i * 2, &Input[i], 1);

	/* print cmd raw data */
	MTWF_PRINT("EID= 0x%x, CMD[%d] = ", ExtendID, len);

	for (i = 0; i < len; i++)
		MTWF_PRINT("0x%x ", Input[i]);

	MTWF_PRINT("\n");
	/* send cmd to fw */
	MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_SET);
	os_free_mem((PVOID)Input);
	return fgStatus;
}

INT32 get_fw_cmd(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	CHAR *value;
	UCHAR *Input;
	UCHAR ExtendID;
	INT i, len;
	BOOLEAN fgStatus = FALSE;
	/* get ExtID */
	value = rstrtok(Arg, ":");
	if (value == NULL)
		return fgStatus;
	AtoH(value, &ExtendID, 1);
	/* get cmd raw data */
	value += 3;
	len = strlen(value) >> 1;
	MTWF_PRINT("=======Get_FwCmd==========\n");
	os_alloc_mem(pAd, (UCHAR **)&Input, len);
	if (!Input) {
		MTWF_PRINT("%s alloc mem failed!\n", __func__);
		return fgStatus;
	}

	for (i = 0; i < len; i++)
		AtoH(value + i * 2, &Input[i], 1);

	/* print cmd raw data */
	MTWF_PRINT("EID= 0x%x, CMD[%d] = ", ExtendID, len);

	for (i = 0; i < len; i++)
		MTWF_PRINT("0x%x ", Input[i]);

	MTWF_PRINT("\n");
	/* send cmd to fw */
	MtCmdSendRaw(pAd, ExtendID, Input, len, CMD_QUERY);
	os_free_mem((PVOID)Input);
	return fgStatus;
}

#ifdef FW_DUMP_SUPPORT
INT set_fwdump_max_size(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->fw_dump_max_size = os_str_tol(arg, 0, 10);
	return TRUE;
}

INT set_fwdump_path(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_OS_FWDUMP_SETPATH(pAd, arg);
	return TRUE;
}

INT fwdump_print(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 x;

	MTWF_PRINT("%s: len = %d\n", __func__, pAd->fw_dump_size);

	for (x = 0; x < pAd->fw_dump_size; x++) {
		if (x % 16 == 0)
			MTWF_PRINT("0x%04x : ", x);

		MTWF_PRINT("%02x ", ((unsigned char)pAd->fw_dump_buffer[x]));

		if (x % 16 == 15)
			MTWF_PRINT("\n");
	}

	MTWF_PRINT("\n");
	return TRUE;
}
#endif

#endif

INT set_thermal_protection_criteria_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	CHAR	 *value;
	UINT8	 ucParamIdx;
	BOOLEAN  fgHighEn = FALSE, fgLowEn = FALSE, fgRFOffEn = FALSE;
	CHAR	 cHighTempTh = 0, cLowTempTh = 0, cRFOffTh = 0;
	UINT32   u4RechkTimer = 0;
	UINT8    ucType = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	/* sanity check for input parameter*/
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		goto error;
	}

	/* Parsing input parameter */
	for (ucParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ucParamIdx++) {
		switch (ucParamIdx) {
		case 0:
			fgHighEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 1:
			fgLowEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 2:
			fgRFOffEn = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		case 3:
			cHighTempTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 4:
			cLowTempTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 5:
			cRFOffTh = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 6:
			u4RechkTimer = simple_strtol(value, 0, 10); /* 4-bit format */
			break;

		case 7:
			ucType = simple_strtol(value, 0, 10); /* 1-bit format */
			break;

		default:
			MTWF_PRINT("Invalid Parameter Format!!\n");
			break;
		}
	}

	if (ucParamIdx != 8) {
		MTWF_PRINT("Wrong parameter format!!\n");
		goto error;
	}

	if (wdev == NULL) {
		MTWF_PRINT(" wdev is invalid\n");
		return FALSE;
	}

	ucBand = HcGetBandByWdev(wdev);

	MTWF_PRINT("%s: fgHighEn: %d, fgLowEn: %d, fgRFOffEn: %d, cHighTempTh: %d, cLowTempTh: %d, cRFOffTh: %d\n", __func__, fgHighEn, fgLowEn, fgRFOffEn, cHighTempTh, cLowTempTh, cRFOffTh);
	MTWF_PRINT("%s: u4RechkTimer: %d, ucType: %s\n", __func__, u4RechkTimer, ucType ? "Duty Cycle" : "TxStream");

#ifdef MT_MAC
	AsicThermalProtect(pAd, ucBand, fgHighEn, cHighTempTh, fgLowEn, cLowTempTh, u4RechkTimer, fgRFOffEn, cRFOffTh, ucType);
#endif /* MT_MAC */

	return TRUE;

error:
	MTWF_PRINT(KYEL "iwpriv <interface> set tpc=fgHighEn:fgLowEn:fgRFOffEn:cHighTempTh:cLowTempTh:cRFOffTh:u4RechkTimer:ucType\n" KNRM);

	MTWF_PRINT("	fgHighEn:	(1-bit format) High Temperature Protect Trigger Enable\n");

	MTWF_PRINT("	fgLowEn:	 (1-bit format) Low Temperature Protect Trigger Enable\n");

	MTWF_PRINT("	fgRFOffEn:   (1-bit format) RF off Protect Trigger Enable\n");

	MTWF_PRINT("	cHighTempTh: (3-bit format) High Temperature Protect Trigger point\n");

	MTWF_PRINT("	cLowTempTh:  (3-bit format) Low Temperature Protect Trigger point\n");

	MTWF_PRINT("	cRFOffTh:	(3-bit format) RF off Protect Trigger point\n");

	MTWF_PRINT("	u4RechkTimer:(4-bit format) Thermal Protect Recheck period\n");

	MTWF_PRINT("	ucType:	  (1-bit format) Thermal Protect Type (0: TxStream, 1: Duty Cycle)\n");

	MTWF_PRINT(KGRN "Ex: iwpriv ra0 set tpc=1:1:1:080:070:110:0060:1\n" KNRM);

	return FALSE;
}

INT set_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		*arg)
{
	CHAR   *value;
	UINT8  ucParamIdx;
	UINT32 u4Lv0Duty = 0, u4Lv1Duty = 0, u4Lv2Duty = 0, u4Lv3Duty = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	/* sanity check for input parameter*/
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		goto error0;
	}

	/* Parsing input parameter */
	for (ucParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ucParamIdx++) {
		switch (ucParamIdx) {
		case 0:
			u4Lv0Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 1:
			u4Lv1Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 2:
			u4Lv2Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		case 3:
			u4Lv3Duty = simple_strtol(value, 0, 10); /* 3-bit format */
			break;

		default:
			MTWF_PRINT("Invalid Parameter Format!!\n");
			break;
		}
	}

	if (ucParamIdx != 4) {
		MTWF_PRINT("Wrong parameter format!!\n");
		goto error0;
	}

	if (wdev == NULL) {
		MTWF_PRINT(" wdev is invalid\n");
		return FALSE;
	}

	ucBand = HcGetBandByWdev(wdev);

	MTWF_PRINT("%s: ucBand:%d, u4Lv0Duty:%d, u4Lv1Duty:%d, u4Lv2Duty:%d, u4Lv3Duty:%d\n", __func__,
		  ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);

	/* Parameter sanity check */
	if (u4Lv0Duty > 100) {
		MTWF_PRINT("Parameters error! Lv0Duty > 100\n");
		goto error1;
	}

	if (u4Lv1Duty > u4Lv0Duty) {
		MTWF_PRINT("Parameters error! Lv1Duty > Lv0Duty");
		goto error1;
	}

	if (u4Lv2Duty > u4Lv1Duty) {
		MTWF_PRINT("Parameters error! Lv2Duty > Lv1Duty");
		goto error1;
	}

	if (u4Lv3Duty > u4Lv2Duty) {
		MTWF_PRINT("Parameters error! Lv3Duty > Lv2Duty");
		goto error1;
	}

	AsicThermalProtectAdmitDuty(pAd, ucBand, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);

	return TRUE;

error0:

	MTWF_PRINT(KYEL "iwpriv <interface> set tpc_duty=Lv0Duty:Lv1Duty:Lv2Duty:Lv3Duty\n" KNRM);

	MTWF_PRINT("	Lv0Duty: (3-bit format) Level 0 Protect Duty cycle\n");

	MTWF_PRINT("	Lv1Duty: (3-bit format) Level 1 Protect Duty cycle\n");

	MTWF_PRINT("	Lv2Duty: (3-bit format) Level 2 Protect Duty cycle\n");

	MTWF_PRINT("	Lv3Duty: (3-bit format) Level 3 Protect Duty cycle\n");

	MTWF_PRINT(KGRN "Ex: iwpriv ra0 set tpc_duty=100:080:050:030\n" KNRM);

	return FALSE;

error1:
	return FALSE;
}


INT get_thermal_protection_admin_ctrl_duty_proc(
	IN PRTMP_ADAPTER	pAd,
	IN RTMP_STRING		 *arg)
{
	return AsicThermalProtectAdmitDutyInfo(pAd);
}

#ifdef MT_MAC
UINT32 RateHTTxToPhyRateFormat(UINT32 RawData)
{
	union _PHY_RATE_SETTING Rate;
	union _HTTRANSMIT_SETTING InputRate;

	InputRate.word = RawData;

	Rate.Word = 0;
	Rate.field.Ldpc = InputRate.field.ldpc;
	Rate.field.Bw = InputRate.field.BW;
	Rate.field.Gi = InputRate.field.ShortGI;
	Rate.field.Stbc = InputRate.field.STBC;
	Rate.field.Mode = InputRate.field.MODE;

	if (Rate.field.Mode >= MODE_VHT) {
		Rate.field.Nss = (InputRate.field.MCS >> 4) & 0x3;
		Rate.field.Mcs = InputRate.field.MCS & 0xF;
	} else
		Rate.field.Mcs = InputRate.field.MCS;

	return Rate.Word;
}

UINT32 GetTxRatePerSta(RTMP_ADAPTER *pAd, UINT32 wcid)
{
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
	union _PHY_RATE_SETTING LastTxRate;
	UINT32 lastTxRate;

	os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));

	MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, wcid, &rTxStatResult);

	LastTxRate.Word = 0;
	LastTxRate.field.Gi = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
	LastTxRate.field.Mode = rTxStatResult.rEntryTxRate.MODE;
	LastTxRate.field.Nss = rTxStatResult.rEntryTxRate.VhtNss - 1;
	LastTxRate.field.Bw = rTxStatResult.rEntryTxRate.BW;
	LastTxRate.field.Ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
	LastTxRate.field.Stbc = rTxStatResult.rEntryTxRate.STBC;

	if (LastTxRate.field.Mode == MODE_OFDM)
		LastTxRate.field.Mcs = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
	else
		LastTxRate.field.Mcs = rTxStatResult.rEntryTxRate.MCS;

	lastTxRate = (UINT32)(LastTxRate.Word);

	return lastTxRate;
}

UINT32 GetTxRatePerSta_stats_info(RTMP_ADAPTER *pAd, UINT32 wcid, EXT_EVENT_TX_STATISTIC_RESULT_T *rTxStatResult)
{
	union _PHY_RATE_SETTING LastTxRate;
	UINT32 lastTxRate;


	MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, wcid, rTxStatResult);

	LastTxRate.Word = 0;
	LastTxRate.field.Gi = rTxStatResult->rEntryTxRate.ShortGI ? 1 : 0;
	LastTxRate.field.Mode = rTxStatResult->rEntryTxRate.MODE;
	LastTxRate.field.Nss = rTxStatResult->rEntryTxRate.VhtNss - 1;
	LastTxRate.field.Bw = rTxStatResult->rEntryTxRate.BW;
	LastTxRate.field.Ldpc = rTxStatResult->rEntryTxRate.ldpc ? 1 : 0;
	LastTxRate.field.Stbc = rTxStatResult->rEntryTxRate.STBC;

	if (LastTxRate.field.Mode == MODE_OFDM)
		LastTxRate.field.Mcs = getLegacyOFDMMCSIndex(rTxStatResult->rEntryTxRate.MCS) & 0x0000003F;
	else
		LastTxRate.field.Mcs = rTxStatResult->rEntryTxRate.MCS;

	lastTxRate = (UINT32)(LastTxRate.Word);

	return lastTxRate;
}

VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData)
{
	extern UCHAR tmi_rate_map_ofdm[];
	extern UCHAR tmi_rate_map_cck_lp[];
	extern UCHAR tmi_rate_map_cck_sp[];
	UCHAR phy_mode, rate, bw, preamble, gi, vht_nss;
	CHAR *FecCoding[2] = {"BCC", "LDPC"};
	CHAR * bwMode[5] = {"BW20", "BW40", "BW80", "BW160", "BW320"};
	int ret;
	ULONG str_tem_len;
	union _PHY_RATE_SETTING RateInfo;

	RateInfo.Word = RawData;

	phy_mode = RateInfo.field.Mode;
	rate = RateInfo.field.Mcs;
	bw = RateInfo.field.Bw;
	gi = RateInfo.field.Gi;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	str_tem_len = MSG_LEN - strlen(Output);
	if (TxRx == 0) {
		ret = snprintf(Output + strlen(Output), str_tem_len,
			"Last TX Rate                           = ");
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	} else {
		ret = snprintf(Output + strlen(Output), str_tem_len,
			"Last RX Rate                           = ");
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	}

	str_tem_len = MSG_LEN - strlen(Output);
	if (phy_mode == MODE_CCK) {
		if (TxRx == 0) {
			if (preamble)
				rate = tmi_rate_map_cck_lp[rate];
			else
				rate = tmi_rate_map_cck_sp[rate];
		}
		if (rate == TMI_TX_RATE_CCK_1M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "1M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_2M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "2M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_5M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "5M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_11M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "11M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_2M_SP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "2M SP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_5M_SP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "5M SP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_11M_SP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "11M SP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else {
			ret = snprintf(Output + strlen(Output), str_tem_len, "unkonw, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}
	} else if (phy_mode == MODE_OFDM) {
		if (TxRx == 0)
			rate = tmi_rate_map_ofdm[rate];
		if (rate == TMI_TX_RATE_OFDM_6M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "6M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_9M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "9M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_12M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "12M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_18M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "18M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_24M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "24M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_36M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "36M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_48M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "48M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_54M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "54M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else {
			ret = snprintf(Output + strlen(Output), str_tem_len, "unkonw, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}
	} else if (phy_mode >= MODE_VHT) {
		vht_nss = RateInfo.field.Nss + 1;
		ret = snprintf(Output + strlen(Output), str_tem_len, "NSS%d_MCS%d, ", vht_nss, rate);
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	} else {
		ret = snprintf(Output + strlen(Output), str_tem_len, "MCS%d, ", rate);
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	}

	str_tem_len = MSG_LEN - strlen(Output);
	if (bw == 7) // Tx bw 7 -> 320; Rx bw 4 -> 320;
		bw = 4;
	if (bw < 5)
		ret = snprintf(Output + strlen(Output), str_tem_len, "%s, ", bwMode[bw]);
	else
		ret = snprintf(Output + strlen(Output), str_tem_len, "%s, ", "N/A");

	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}
	str_tem_len = MSG_LEN - strlen(Output);
	ret = snprintf(Output + strlen(Output), str_tem_len, "%s GI, ", get_gi_str(phy_mode, gi));
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}
	str_tem_len = MSG_LEN - strlen(Output);
	ret = snprintf(Output + strlen(Output), str_tem_len, "%s%s%s\n",
		get_phymode_str(phy_mode),
		RateInfo.field.Stbc ? ", STBC, " : ", ",
		FecCoding[RateInfo.field.Ldpc]);
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}
}

VOID StatHERxRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UINT32 RawData)
{
	UCHAR phy_mode, guard_interval, rate, bandwidth, stbc, coding, nsts;
	CHAR *phyMode[16] = {"CCK", "OFDM", "HT_MM", "HT_GF", "VHT", "HE",
				"EHT", "HE2G", "HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU", "NA",
				"EHT_ER_SU", "EHT_TRIG", "EHT_MU"};
	CHAR *FecCoding[2] = {"BCC", "LDPC"};
	CHAR * bwMode[5] = {"BW20", "BW40", "BW80", "BW160", "BW320"};
	CHAR *HeGi[4] = {"0.8us", "1.6us", "3.2us", " "};
	UINT32 msg_len = 2048;
	int ret;
	ULONG str_tem_len;

	/* RawData        Mode [19:16]     GI [15:14]     Rate [13:8]     BW [7:5]     STBC [4]     Coding [3]     Nsts [2:0]  */
	phy_mode = (RawData >> 16) & 0xF;
	guard_interval = (RawData >> 14) & 0x3;
	rate = (RawData >> 8) & 0x3F;
	bandwidth = (RawData >> 5) & 0x7;
	stbc = (RawData >> 4) & 0x1;
	coding = (RawData >> 3) & 0x1;
	nsts = (RawData) & 0x7;

	str_tem_len = msg_len - strlen(Output);
	ret = snprintf(Output + strlen(Output), str_tem_len,
		"%s", "Last RX Rate                           = ");
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}

	str_tem_len = msg_len - strlen(Output);
	if (phy_mode == MODE_CCK) {
		rate = rate & 0x7;
		if (rate == TMI_TX_RATE_CCK_1M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "1M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_2M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "2M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_5M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "5M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_11M_LP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "11M LP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_2M_SP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "2M SP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_5M_SP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "5M SP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_CCK_11M_SP) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "11M SP, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "unknown, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}
	} else if (phy_mode == MODE_OFDM) {
		rate = rate & 0xF;
		if (rate == TMI_TX_RATE_OFDM_6M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "6M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_9M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "9M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_12M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "12M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_18M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "18M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_24M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "24M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_36M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "36M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_48M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "48M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else if (rate == TMI_TX_RATE_OFDM_54M) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "54M, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else {
			ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "unknown, ");
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}
	} else if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD) {
		ret = snprintf(Output + strlen(Output), str_tem_len, "MCS%d, ", rate);
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	} else {
		rate = rate & 0xF;
		if (((phy_mode == MODE_VHT) && (rate > 9)) || ((phy_mode >= MODE_HE) && (phy_mode <= MODE_HE_MU) && (rate > 11))) {
			ret = snprintf(Output + strlen(Output), str_tem_len, "Incorrect NSS%d_MCS%d, ", ((nsts + 1) / (stbc + 1)), rate);
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		} else {
			ret = snprintf(Output + strlen(Output), str_tem_len, "NSS%d_MCS%d, ", ((nsts + 1) / (stbc + 1)), rate);
			if (os_snprintf_error(str_tem_len, ret)) {
				MTWF_PRINT("%s: snprintf error!\n", __func__);
				return;
			}
		}
	}

	str_tem_len = msg_len - strlen(Output);
	ret = snprintf(Output + strlen(Output), str_tem_len, "%s, ", bwMode[bandwidth]);
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}

	if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD || phy_mode == MODE_VHT) {
		str_tem_len = msg_len - strlen(Output);
		ret = snprintf(Output + strlen(Output), str_tem_len, "%cGI, ", (guard_interval & 0x1) ? 'S' : 'L');
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	}
	if (phy_mode >= MODE_HE) {
		str_tem_len = msg_len - strlen(Output);
		ret = snprintf(Output + strlen(Output), str_tem_len, "%s GI, ", HeGi[guard_interval]);
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	}

	str_tem_len = msg_len - strlen(Output);
	ret = snprintf(Output + strlen(Output), str_tem_len, "%s", phyMode[phy_mode]);
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}

	if (phy_mode >= MODE_HTMIX) {
		str_tem_len = msg_len - strlen(Output);
		ret = snprintf(Output + strlen(Output),
			str_tem_len, "%s", (stbc) ? ", STBC, " : ", ");
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
		str_tem_len = msg_len - strlen(Output);
		ret = snprintf(Output + strlen(Output), str_tem_len, "%s", FecCoding[coding]);
		if (os_snprintf_error(str_tem_len, ret)) {
			MTWF_PRINT("%s: snprintf error!\n", __func__);
			return;
		}
	}
	str_tem_len = msg_len - strlen(Output);
	ret = snprintf(Output + strlen(Output), str_tem_len, "%s", "\n");
	if (os_snprintf_error(str_tem_len, ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}
}

INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* 0: get temperature; 1: get adc */
	UINT32 value;
	UINT32 Sensor = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand;

	value = os_str_tol(arg, 0, 10);

	if ((value == 0) || (value == 1)) {
		if (!wdev) {
			MTWF_PRINT("wdev is invalid\n");
			return FALSE;
		}

		ucBand = HcGetBandByWdev(wdev);
		MtCmdGetThermalSensorResult(pAd, value, ucBand, &Sensor);
		switch (value) {
		case 0:
			MTWF_PRINT("%s: TMP Value = %d [0x%x]\n", __func__, (INT32)Sensor, (INT32)Sensor);
			break;
		case 1:
			MTWF_PRINT("%s: ADC Value = %d [0x%x]\n", __func__, Sensor, Sensor);
			break;
		}
	} else
		MTWF_PRINT("%s: 0: get temperature; 1: get adc\n", __func__);

	return TRUE;
}

INT set_manual_rdg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32   ret = 0;
	UINT32   init, resp, txop, wcid, band;

	ret = sscanf(arg, "%u-%u-%u-%u-%u",
		     &(init), &(resp), &(txop), &(wcid), &(band));

	if (ret != 5) {
		MTWF_PRINT("Format Error!! should be:");
		MTWF_PRINT(" iwpriv ra0 set manual_rdg=[init]-[resp]-[txop]-[wcid]-[band]\n");
	} else {
		MTWF_PRINT("\n>> Initiator=%x, Responder=%x, Txop=0x%x, Wcid=%u, BandIdx=%x\n",
			  init, resp, txop, wcid, band);

		AsicSetRDG(pAd, wcid, band, init, resp);
	}

	return TRUE;
}
#endif /* MT_MAC */

#ifdef SMART_CARRIER_SENSE_SUPPORT

#ifdef WIFI_UNIFIED_COMMAND
INT UNI_Set_SCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	INT32 Ret;
	UNI_CMD_SCS_PARAM_T pParamCtrl;
	UCHAR BandIdx;
	BOOLEAN SCSEnable;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	SCSEnable = os_str_tol(arg, 0, 10);

	if (apidx >= pAd->ApCfg.BssidNum)
		return NDIS_STATUS_FAILURE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	/* prepare command message */
	memset(&pParamCtrl, 0, sizeof(UNI_CMD_SCS_PARAM_T));
	pParamCtrl.ucDbdcIdx = BandIdx;
	pParamCtrl.bQuery = FALSE;
	pParamCtrl.SCSTagValid[UNI_CMD_SCS_EVENT_SCS_ENABLE] = TRUE;
	pParamCtrl.SCSEnable.u2Tag = UNI_CMD_SCS_EVENT_SCS_ENABLE;
	pParamCtrl.SCSEnable.u1SCSEnable = SCSEnable;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_NOTICE,
		 ": BandIdx=%d, SCSEnable=%d\n", BandIdx, SCSEnable);
	if (SCSEnable == SCS_DISABLE) {
		pAd->SCSCtrl.SCSEnable = SCS_DISABLE;
		pAd->SCSCtrl.SCSStatus = PD_BLOCKING_OFF;
	} else if (SCSEnable == SCS_ENABLE) {
		pAd->SCSCtrl.SCSEnable = SCS_ENABLE;
	}
	Ret = UniCmdSCS(pAd, &pParamCtrl);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_NOTICE,
		"(Ret = %d_\n", Ret);
	return Ret;
}

INT UNI_Set_SCSPdThrRange_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	INT32 Ret = NDIS_STATUS_FAILURE;
	UNI_CMD_SCS_PARAM_T pParamCtrl;
	INT32	Recv = 0;
	UCHAR	BandIdx = 0;
	UINT16	u2CckPdThrMin = 0;
	UINT16	u2CckPdThrMax = 0;
	UINT16	u2OfdmPdThrMin = 0;
	UINT16	u2OfdmPdThrMax = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return Ret;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);
	Recv = sscanf(arg, "%hd-%hd-%hd-%hd", &(u2CckPdThrMax),
		&(u2CckPdThrMin), &(u2OfdmPdThrMax), &(u2OfdmPdThrMin));

	if (Recv != 4 || (u2CckPdThrMax < 30 || u2CckPdThrMin > 110)
			|| (u2OfdmPdThrMax < 30 || u2OfdmPdThrMin > 98)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
			"Format Error or Out of range\n");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
			"iwpriv ra0 set SCSCfg=[CckPdThrMax]-[CckPdThrMin]-[OfdmPdThrMax]-[OfdmPdThrMin]\n");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
			"CckPdBlkTh  Range: 30~110 dBm (Represents a negative number)\n");
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
			"OfdmPdBlkTh Range: 30~98	dBm (Represents a negative number)\n");
		goto error;
	} else {
		/* prepare command message */
		memset(&pParamCtrl, 0, sizeof(UNI_CMD_SCS_PARAM_T));
		pParamCtrl.ucDbdcIdx = BandIdx;
		pParamCtrl.bQuery = FALSE;
		pParamCtrl.SCSTagValid[UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE] = TRUE;
		pParamCtrl.SCSPDThrRange.u2Tag = UNI_CMD_SCS_EVENT_SET_PD_THR_RANGE;
		pParamCtrl.SCSPDThrRange.u2CckPdThrMin = u2CckPdThrMin;
		pParamCtrl.SCSPDThrRange.u2CckPdThrMax = u2CckPdThrMax;
		pParamCtrl.SCSPDThrRange.u2OfdmPdThrMin = u2OfdmPdThrMin;
		pParamCtrl.SCSPDThrRange.u2OfdmPdThrMax = u2OfdmPdThrMax;
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_NOTICE,
			":CckThrMin=%d, CckThrMax=%d, OfdmThrMin=%d, OfdmThrMax=%d\n",
			pParamCtrl.SCSPDThrRange.u2CckPdThrMin,
			pParamCtrl.SCSPDThrRange.u2CckPdThrMax,
			pParamCtrl.SCSPDThrRange.u2OfdmPdThrMin,
			pParamCtrl.SCSPDThrRange.u2OfdmPdThrMax);
		Ret = UniCmdSCS(pAd, &pParamCtrl);
	}
error:
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
			 ":(Ret = %d_\n", Ret);
	return Ret;
}

/* ==========================================================================
Description:
	Send SCS data to FW

Parameters:
	Standard SCS  Parameters

==========================================================================
 */
INT UNI_SendSCSDataProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret;
	UNI_CMD_SCS_PARAM_T pParamCtrl;
	UINT16	eTput = 0;
	UINT8	ActiveSTA = 0;
	UINT8	u1BandIdx;
	UINT16	j = 0;
	BOOL	fgRxOnly = FALSE;
	PSMART_CARRIER_SENSE_CTRL	pSCSCtrl = &pAd->SCSCtrl;

	u1BandIdx = os_str_tol(arg, 0, 10);

	if (pSCSCtrl == NULL) {
		Ret = FALSE;
		goto error;
	}
	if (pSCSCtrl->SCSEnable == SCS_ENABLE) {
		/* Algorithm is not work in RX_only */
		/* TX byte <10% TR ratio*/
		/* Note : Need to calculate : pSCSCtrl->OneSecTxByteCount,
		pSCSCtrl->OneSecRxByteCount*/
		if ((pSCSCtrl->OneSecTxByteCount) * 9 <  pSCSCtrl->OneSecRxByteCount) {
			pSCSCtrl->PDreset = TRUE;
			fgRxOnly = TRUE;
		} else {
			ActiveSTA = 0;
			pSCSCtrl->PDreset = TRUE;
			fgRxOnly = FALSE;
			/*Part 1 : Tx Success Byte Cnt*/
			for (j = 1; VALID_UCAST_ENTRY_WCID(pAd, j); j++) {
				PMAC_TABLE_ENTRY pEntry = entry_get(pAd, j);

				if ((pEntry->wdev == NULL) || HcGetBandByWdev(pEntry->wdev) != u1BandIdx)
					continue;
				if ((IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) || IS_ENTRY_WDS(pEntry) || IS_ENTRY_APCLI(pEntry)) {

					/* Multiple Active STA Support */
					/* Sum all Active STA T-PUT - Tput > 2Mbps */
					if (pEntry->OneSecTxBytes + pEntry->OneSecRxBytes > 250000) {
						eTput += (pEntry->OneSecTxBytes + pEntry->OneSecRxBytes) >> 17;
						pSCSCtrl->PDreset = FALSE;
						ActiveSTA++;

					}
				}
			}
		}
	}

	/* prepare command message */
	memset(&pParamCtrl, 0, sizeof(UNI_CMD_SCS_PARAM_T));
	pParamCtrl.ucDbdcIdx = u1BandIdx;
	pParamCtrl.bQuery = FALSE;
	pParamCtrl.SCSTagValid[UNI_CMD_SCS_EVENT_SEND_DATA] = TRUE;
	pParamCtrl.SCSSendData.u2Tag = UNI_CMD_SCS_EVENT_SEND_DATA;
	pParamCtrl.SCSSendData.u2ActiveSTA = ActiveSTA;
	pParamCtrl.SCSSendData.u2eTput = eTput;
	pParamCtrl.SCSSendData.fgRxOnly = fgRxOnly;
	pParamCtrl.SCSSendData.fgPDreset = pSCSCtrl->PDreset;
	pParamCtrl.SCSSendData.i1SCSMinRSSI = pSCSCtrl->SCSMinRssi;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_DEBUG,
		": [CMD to FW] Band%u: ActiveSTA %u, eTput %u, fgRxOnly %u, PDreset %u,",
		pParamCtrl.ucDbdcIdx,
		pParamCtrl.SCSSendData.u2ActiveSTA,
		pParamCtrl.SCSSendData.u2eTput,
		pParamCtrl.SCSSendData.fgRxOnly,
		pParamCtrl.SCSSendData.fgPDreset);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_DEBUG,
		" SCSMinRssi %d, OneSecTxByteCount %u, OneSecRxByteCount %u\n",
		pParamCtrl.SCSSendData.i1SCSMinRSSI,
		pSCSCtrl->OneSecTxByteCount,
		pSCSCtrl->OneSecRxByteCount);

	Ret = UniCmdSCS(pAd, &pParamCtrl);
error:
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_DEBUG,
			 ":(Ret = %d_\n", Ret);
	return Ret;
}

INT UNI_GetScsGloAddr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	INT32 Ret = NDIS_STATUS_FAILURE;
	UNI_CMD_SCS_PARAM_T pParamCtrl;
	UCHAR	BandIdx = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return Ret;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	/* prepare command message */
	memset(&pParamCtrl, 0, sizeof(UNI_CMD_SCS_PARAM_T));
	pParamCtrl.ucDbdcIdx = BandIdx;
	pParamCtrl.bQuery = TRUE;
	pParamCtrl.SCSTagValid[UNI_CMD_SCS_EVENT_GET_GLO_ADDR] = TRUE;
	pParamCtrl.SCSGetGloAddrCtrl.u2Tag = UNI_CMD_SCS_EVENT_GET_GLO_ADDR;

	Ret = UniCmdSCS(pAd, &pParamCtrl);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
			 ":(Ret = %d_\n", Ret);
	return Ret;
}
#endif /* WIFI_UNIFIED_COMMAND */
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef SWACI_MECHANISM
INT32 Set_LNATestMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 recv = 0;
	UINT32 u4TestMode = 0;

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_TESTMODE;

	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%u", &(u4TestMode));

		if (recv != 1) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
					"TestModeEnable\n");
			return TRUE;
		}
		MTWF_PRINT("TestModeEnble = %d\n", u4TestMode);
	} else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSetSwlnaTestMode(pAd, u4TestMode);
#else
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(u4TestMode));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
		u4TestMode = cpu2le32(u4TestMode);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&u4TestMode, sizeof(u4TestMode));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);

#endif
	return Ret;
}

INT32 Set_LNATimer_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 recv = 0;
	UINT32 u4DbdcIdx, u4Enable;

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_TIMER_START;
	struct cmd_msg *msg = NULL;
	EXT_CMD_LNA_TIMER_ENABLE rLnaTimer = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%u-%u",
							&(u4DbdcIdx), &(u4Enable));

		if (recv != 2) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
					"DbdcIdx-Enable\n");
			return TRUE;
		}
		MTWF_PRINT("DbdcIdx = %u\nEnable = %u\n",
					u4DbdcIdx, u4Enable);
	MTWF_PRINT("%s:", __func__);
	}

	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSwlnaTimerCtrl(pAd, (UINT8)u4DbdcIdx, (UINT8)u4Enable);
#else
	rLnaTimer.u1DbdcIdx = (UINT8)u4DbdcIdx;
	rLnaTimer.fgEnable = (BOOLEAN)u4Enable;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(rLnaTimer));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
		rLnaTimer = cpu2le32(rLnaTimer);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&rLnaTimer, sizeof(rLnaTimer));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);

#endif
	return Ret;
}

INT32 Set_LNAEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 recv = 0;
	UINT32 u4Enable;

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_ENABLE;
	struct cmd_msg *msg = NULL;
	UINT8 u1LnaEnable = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%u", &(u4Enable));

		if (recv != 1) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
					"Enable\n");
			return TRUE;
		}
		MTWF_PRINT("SWLNA Enable = %u\n",
					u4Enable);
	MTWF_PRINT("%s:", __func__);
	}

	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSwlnaEnable(pAd, (UINT8)u4Enable);
#else

	u1LnaEnable = (UINT8)u4Enable;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(u1LnaEnable));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&u1LnaEnable, sizeof(u1LnaEnable));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);

#endif
	return Ret;
}

INT32 Set_LNAThreshConfig_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 recv = 0;
	UINT32 u4Threshold = 0;

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_THRESH_CONFIG;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%u", &(u4Threshold));

		if (recv != 1) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
					"Threshold\n");
			return TRUE;
		}
		MTWF_PRINT("Threshold = %u\n",
					u4Threshold);
	}

	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSetSwlnaThresh(pAd, u4Threshold);
#else

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(u4Threshold));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&u4Threshold, sizeof(u4Threshold));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:Ret = %d\n", __func__, Ret);

#endif
	return Ret;
}


INT32 Set_LNACondition_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 recv = 0;
	UINT32 u4Swcr0 = 0, u4Swcr1 = 0, u4Swcr2 = 0;
	INT32 i4Swcr4 = 0;

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_CONDITION_PARAM;
	struct cmd_msg *msg = NULL;
	EXT_CMD_SWCR_PARAMS rSwCrParam = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%u-%u-%u-%d",
							&(u4Swcr0), &(u4Swcr1), &(u4Swcr2), &(i4Swcr4));

		if (recv != 4) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
					"Swcr0-Swcr1-Swcr2-Swcr4\n");
			return TRUE;
		}
		MTWF_PRINT("iSwcr0 = %u\niSwcr1 = %u\niSwcr2 = %u\ni4Swcr4 = %d\n",
					 u4Swcr0, u4Swcr1, u4Swcr2, i4Swcr4);
	}

	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSetSwlnaCondition(pAd, (UINT8)u4Swcr0, (UINT8)u4Swcr1, (UINT8)u4Swcr2, (INT8)i4Swcr4);
#else
	rSwCrParam.u1SWCR0 = (UINT8)u4Swcr0;
	rSwCrParam.u1SWCR1 = (UINT8)u4Swcr1;
	rSwCrParam.u1SWCR2 = (UINT8)u4Swcr2;
	rSwCrParam.i1SWCR4 = (INT8)i4Swcr4;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(rSwCrParam));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
		rSwCrParam = cpu2le32(rSwCrParam);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&rSwCrParam, sizeof(rSwCrParam));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);

#endif
	return Ret;
}

INT32 Set_LNADenseParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT32 recv = 0, i1MaxRcpi = 0, i1MinSubRssi = 0;
	UINT32 u1ReadCount = 0, u1MinDenseCount = 0, u1MaxDenseCount = 0, u1DenseCRValue = 0;

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_DENSE_PARAMS;
	struct cmd_msg *msg = NULL;
	EXT_CMD_DENSE_LNA_PARAM rSwCrParam = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%d-%d-%u-%u-%u-%u",
							&(i1MaxRcpi), &(i1MinSubRssi), &(u1ReadCount), &(u1MinDenseCount), (&u1MaxDenseCount), (&u1DenseCRValue));

		if (recv != 6) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
					"i1MaxRcpi-i1MinSubRssi-u1ReadCount-u1MinDenseCount-u1MaxDenseCount-u1DenseCRValue\n");
			return TRUE;
		}
		MTWF_PRINT("i1MaxRcpi = %d\ni1MinSubRssi = %d\nu1ReadCount = %u\n",
			i1MaxRcpi, i1MinSubRssi, u1ReadCount);
		MTWF_PRINT("u1MinDenseCount = %u\nu1MaxDenseCount = %u\nu1DenseCRValue = 0x%X\n",
			u1MinDenseCount, u1MaxDenseCount, u1DenseCRValue);
		MTWF_PRINT("%s:", __func__);
	}

	else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSetSwlnaDenseParam(pAd, (INT8)i1MaxRcpi, (INT8)i1MinSubRssi, (UINT8)u1ReadCount,
				(UINT8)u1MinDenseCount, (UINT8)u1MaxDenseCount, (UINT8)u1DenseCRValue);
#else
	rSwCrParam.i1MaxRcpi = (INT8)i1MaxRcpi;
	rSwCrParam.i1MinSubRssi = (INT8)i1MinSubRssi;
	rSwCrParam.u1ReadCount = (UINT8)u1ReadCount;
	rSwCrParam.u1MinDenseCount = (UINT8)u1MinDenseCount;
	rSwCrParam.u1MaxDenseCount = (UINT8)u1MaxDenseCount;
	rSwCrParam.u1DenseCRValue = (UINT8)u1DenseCRValue;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(rSwCrParam));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
		rSwCrParam = cpu2le32(rSwCrParam);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&rSwCrParam, sizeof(rSwCrParam));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);

#endif
	return Ret;
}

INT32 Set_LNATable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 recv = 0;
	UINT32  u4PhyBw = 0;
	INT32   i4LnaIdx4 = 0, i4LnaIdx3 = 0;
	INT32   i4LnaIdx2 = 0, i4LnaIdx1 = 0, i4LnaIdx0 = 0;

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_LNA_TABLE_PARAM;
	struct cmd_msg *msg = NULL;
	EXT_CMD_LNA_TABLE_PARAMS rLnaTableParam = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%u-%d-%d-%d-%d-%d",
							&(u4PhyBw), &(i4LnaIdx4), &(i4LnaIdx3),
							&(i4LnaIdx2), &(i4LnaIdx1), &(i4LnaIdx0));

		if (recv != 6) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
					"PhyBw-LnaIdx4Rssi-LnaIdx3Rssi-"
					"LnaIdx2Rssi-LnaIdx1Rssi-LnaIdx0Rssi\n");
			return TRUE;
		}
		MTWF_PRINT("PhyBw = %u\nLnaIdx4Rssi = %d\nLnaIdx3Rssi = %d\n"
					"LnaIdx2Rssi = %d\nLnaIdx1Rssi = %d\nLnaIdx0Rssi = %d\n",
					 u4PhyBw, i4LnaIdx4, i4LnaIdx3,
					i4LnaIdx2, i4LnaIdx1, i4LnaIdx0);
	} else
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSetSwlnaTable(pAd, (UINT8)u4PhyBw, (INT8)i4LnaIdx0, (INT8)i4LnaIdx1,
				(INT8)i4LnaIdx2, (INT8)i4LnaIdx3, (INT8)i4LnaIdx4);
#else
	rLnaTableParam.u1PhyBw = (UINT8)u4PhyBw;
	rLnaTableParam.i1LnaParam[4] = (INT8)i4LnaIdx4;
	rLnaTableParam.i1LnaParam[3] = (INT8)i4LnaIdx3;
	rLnaTableParam.i1LnaParam[2] = (INT8)i4LnaIdx2;
	rLnaTableParam.i1LnaParam[1] = (INT8)i4LnaIdx1;
	rLnaTableParam.i1LnaParam[0] = (INT8)i4LnaIdx0;

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(rLnaTableParam));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
	rLnaTableParam = cpu2le32(rLnaTableParam);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&rLnaTableParam, sizeof(rLnaTableParam));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);

#endif
	return Ret;
}

INT32 SetRcpiTestMode(IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	INT32 ret = TRUE;
	RX_STATISTIC_RXV *rx_stat;
	UINT8 u1BandIdx = 0, u1AntIdx = 0, u1MaxAntenna = 4, u1MaxBand = 2;
	EXT_CMD_SET_RCPI_TEST rRcpiTest;
	struct _RTMP_ADAPTER *mac_ad = NULL;
#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_RCPI_TESTMODE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(EXT_CMD_SET_RCPI_TEST));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
#endif

	memset(&rRcpiTest, 0x00, sizeof(struct _EXT_CMD_SET_RCPI_TEST));

	for (u1BandIdx = 0; u1BandIdx < u1MaxBand; u1BandIdx++) {
		mac_ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, u1BandIdx);
		if (!mac_ad)
			continue;
		rx_stat = &mac_ad->rx_stat_rxv;

		for (u1AntIdx = 0; u1AntIdx < u1MaxAntenna; u1AntIdx++)
			rRcpiTest.au2Rcpi[u1BandIdx][u1AntIdx] = rx_stat->RCPI[u1AntIdx];
	}

#ifdef WIFI_UNIFIED_COMMAND
	ret = UniCmdSetTestModeRcpi(pAd, (UINT8 *)&rRcpiTest);
#else

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
		rRcpiTest = cpu2le32(rRcpiTest);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&rRcpiTest, sizeof(rRcpiTest));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			 ":(ret = %d)\n", ret);
#endif
	return ret;
}

INT32 Set_LNAGain_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = TRUE;
	INT8 recv = 0;
	INT32   i4AdcSetPointAci = 0, i4MaxRfGain = 0;
	INT32   i4AdcSetPointAAciBw20 = 0, i4AdcSetPointAAciBw40 = 0, i4AdcSetPointAAciBw80 = 0;
	INT32   i4AdcSetPointWanted = 0, i4FarGainBoundLong = 0;
	INT32   i4RfdgcSetPointAci = 0, i4MaxTotalGain = 0;
	INT32   i4RfdgcSetPointWanted = 0, i4FarGainBoundShort = 0;
	EXT_CMD_LNA_GAIN_ADJUST_PARAMS rGainAdjustParam = {0};

#ifndef WIFI_UNIFIED_COMMAND
	UINT32 u4SubCmd = SWLNA_GAIN_ADJUST_PARAM;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}
#endif

	if (arg) {
		recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
							&(i4AdcSetPointAci), &(i4AdcSetPointAAciBw20), &(i4AdcSetPointAAciBw40),
							&(i4AdcSetPointAAciBw80), &(i4AdcSetPointWanted), &(i4MaxRfGain),
							&(i4RfdgcSetPointAci), &(i4RfdgcSetPointWanted), &(i4MaxTotalGain),
							&(i4FarGainBoundLong), &(i4FarGainBoundShort));

		if (recv != 11) {
			MTWF_PRINT("Format Error! Please enter in the following format\n"
				"AdcSetPointAci-AdcSetPointAAciBw20-AdcSetPointAAciBw40-AdcSetPointAAciBw80-AdcSetPointWanted-MaxRfGain-"
				"RfdgcSetPointAci-RfdgcSetPointWanted-MaxTotalGain-FarGainBoundLong-FarGainBoundShort\n");

			return TRUE;
		}
		MTWF_PRINT(
			"AdcSetPointAci = %d\nAdcSetPointAAciBw20 = %d\nAdcSetPointAAciBw40 = %d\n"
			"AdcSetPointAAciBw80 = %d\nAdcSetPointWanted = %d\nMaxRfGain = %d\n"
			"RfdgcSetPointAci = %d\nRfdgcSetPointWanted = %d\nMaxTotalGain = %d\n"
			"FarGainBoundLong = %d\nFarGainBoundShort=%d\n",
			i4AdcSetPointAci, i4AdcSetPointAAciBw20, i4AdcSetPointAAciBw40,
			i4AdcSetPointAAciBw80, i4AdcSetPointWanted, i4MaxRfGain,
			i4RfdgcSetPointAci, i4RfdgcSetPointWanted, i4MaxTotalGain,
			i4FarGainBoundLong, i4FarGainBoundShort);
	MTWF_PRINT("%s:", __func__);
	}

	else
		return FALSE;

	rGainAdjustParam.i1AdcSetPointAci = (INT8)i4AdcSetPointAci;
	rGainAdjustParam.i1AdcSetPointAAciBw20 = (INT8)i4AdcSetPointAAciBw20;
	rGainAdjustParam.i1AdcSetPointAAciBw40 = (INT8)i4AdcSetPointAAciBw40;
	rGainAdjustParam.i1AdcSetPointAAciBw80 = (INT8)i4AdcSetPointAAciBw80;
	rGainAdjustParam.i1AdcSetPointWanted = (INT8)i4AdcSetPointWanted;
	rGainAdjustParam.i1MaxRfGain = (INT8)i4MaxRfGain;
	rGainAdjustParam.i1RfdgcSetPointAci = (INT8)i4RfdgcSetPointAci;
	rGainAdjustParam.i1RfdgcSetPointWanted = (INT8)i4RfdgcSetPointWanted;
	rGainAdjustParam.i1MaxTotalGain = (INT8)i4MaxTotalGain;
	rGainAdjustParam.i1FarGainBoundLong = (INT8)i4FarGainBoundLong;
	rGainAdjustParam.i1FarGainBoundShort = (INT8)i4FarGainBoundShort;

#ifdef WIFI_UNIFIED_COMMAND
	Ret = UniCmdSetSwaciGain(pAd, (UINT8 *)&rGainAdjustParam);
#else
	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(u4SubCmd) + sizeof(rGainAdjustParam));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RLM_SWLNA_ACI_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
	rGainAdjustParam = cpu2le32(rGainAdjustParam);
#endif
	AndesAppendCmdMsg(msg, (char *)&u4SubCmd, sizeof(u4SubCmd));
	AndesAppendCmdMsg(msg, (char *)&rGainAdjustParam, sizeof(rGainAdjustParam));
	AndesSendCmdMsg(pAd, msg);

error:
	MTWF_PRINT("%s:(Ret = %d_\n", __func__, Ret);

#endif
	return Ret;
}
#endif
INT32 EnableNF(RTMP_ADAPTER *pAd, UINT8 isEnable, UINT8 u1TimeOut, UINT8 u1Count, UINT8 u1EventCount)
{
	INT32 Ret = TRUE;
	struct cmd_msg *msg = NULL;
	EXT_CMD_ENABLE_NOISE_FLOOR_T NfParam = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_INFO, "entry");

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(NfParam));

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_ENABLE_NOISEFLOOR);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	NfParam.fgEnable = (BOOLEAN)isEnable;
	NfParam.u1TimeOut = u1TimeOut;
	NfParam.u1Count = u1Count;
	NfParam.u1EventCount = u1EventCount;

#ifdef CFG_BIG_ENDIAN
	NfParam = cpu2le32(NfParam);
#endif

	AndesAppendCmdMsg(msg, (char *)&NfParam, sizeof(NfParam));
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_DEBUG,
			 ":(Ret = %d_\n", Ret);
	return Ret;
}

INT SetEnableNf(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	PCHAR pch = NULL;
	UINT8 u1Enable, u1TimeOut, u1Count, u1EventCount;
	INT status = TRUE;

	pch = strsep(&arg, "-");

	if (pch != NULL)
		u1Enable = os_str_tol(pch, 0, 10);
	else
		return FALSE;

	pch = strsep(&arg, "-");
	/*Get Timer Timeout*/
	if (pch != NULL)
		u1TimeOut = os_str_tol(pch, 0, 10);
	else
		return FALSE;

	pch = strsep(&arg, "-");
	/*Get Time Count*/
	if (pch != NULL)
		u1Count = os_str_tol(pch, 0, 10);
	else
		return FALSE;

	pch = strsep(&arg, "");
	/*Get Event Count*/
	if (pch != NULL)
		u1EventCount = os_str_tol(pch, 0, 10);
	else
		return FALSE;

	if ((u1TimeOut != 0) && (u1Count != 0) && ((u1TimeOut % 10) == 0))
#ifdef WIFI_UNIFIED_COMMAND
		UniCmdSetNoiseFloorControl(pAd, UNI_CMD_NF_INFO, u1Enable, u1TimeOut, u1Count, u1EventCount);
#else
		EnableNF(pAd, u1Enable, u1TimeOut, u1Count, u1EventCount);
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCAN, DBG_LVL_ERROR,
			"\nTimeOut should be a multiple of 10ms\n");
		return FALSE;
	}
	return status;
}

#ifdef WIFI_MD_COEX_SUPPORT
INT SendApccci2fwMsg(RTMP_ADAPTER *pAd, struct _MT_WIFI_COEX_APCCCI2FW *apccci2fw_msg)
{
	INT32	Ret = TRUE;
	struct cmd_msg *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};

	if (!IS_MT7915(pAd)) {
		Ret = FALSE;
		goto error;
	}

	/* Allocate memory for msg */
	msg = AndesAllocCmdMsg(pAd, apccci2fw_msg->len);

	if (!msg) {
		Ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_APCCCI_MSG);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RETRY);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

#ifdef CFG_BIG_ENDIAN
	apccci2fw_msg = cpu2le32(apccci2fw_msg);
#endif

	AndesAppendCmdMsg(msg, (char *)apccci2fw_msg->data, apccci2fw_msg->len);
	AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_DEBUG,
			 ":(Ret = %d_\n", Ret);
	return Ret;
}
#endif /* WIFI_MD_COEX_SUPPORT */

INT set_support_rate_table_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status;
	UINT8 i = 0;
	CHAR *value = 0;
	UINT8 tx_mode = 0;
	UINT8 tx_nss = 0;
	UINT8 tx_bw = 0;
	UINT16 mcs_cap = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("no parameter.\n");
		goto error0;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			tx_mode = simple_strtol(value, 0, 10);
			break;

		case 1:
			tx_nss = simple_strtol(value, 0, 10);
			break;

		case 2:
			tx_bw = simple_strtol(value, 0, 10);
			break;

		case 3:
			/* input with hex format */
			mcs_cap = simple_strtol(value, 0, 16);
			break;

		default: {
			MTWF_PRINT("invalid parameter format.\n");
			goto error0;
		}
		}
	}

	status = support_rate_table_ctrl(pAd, tx_mode, tx_nss, tx_bw, &mcs_cap);

	return status;

error0:
	MTWF_PRINT("----------------------------------------------\n");
	MTWF_PRINT("argument format:\n");
	MTWF_PRINT("	[tx_mode]:[tx_nss]:[tx_bw]:[mcs_cap]:[set]\n");
	MTWF_PRINT("----------------------------------------------\n");
	MTWF_PRINT("tx_mode: 0/1/2/4/8 for cck/ofdm/ht/vht/he\n");
	MTWF_PRINT("tx_nss: 0~3 for 1ss/2ss/3ss/4ss\n");
	MTWF_PRINT("tx_bw: 0~3 for bw20/40/80/160\n");
	MTWF_PRINT("mcs_cap: hex format representation\n");
	MTWF_PRINT("	ex: 88 for supprt mcs7 and mcs3.\n");
	return FALSE;
}

INT set_support_rate_table_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	return TRUE;
}

# define RA_DBG_SUPPORT_PARAM_NUM    20
INT set_ra_dbg_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status;
	UINT8 i = 0;
	CHAR *value = 0;
	UINT8 param_num = 0;
	UINT32 param[RA_DBG_SUPPORT_PARAM_NUM] = {0};

	/* sanity check for input parameter format */
	if (!arg)
		goto error0;

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		/* parsing for number of parameters */
		if (i == 0) {
			param_num = simple_strtol(value, 0, 10);

			if (param_num > RA_DBG_SUPPORT_PARAM_NUM)
				goto error2;
		}

		/* error handle */
		if (i > RA_DBG_SUPPORT_PARAM_NUM)
			goto error1;

		/* parsing for parameters */
		if (i > 0)
			param[i-1] = simple_strtol(value, 0, 10);
	}

	MTWF_PRINT("%s: param_num: %d \n", __func__, param_num);

	MTWF_PRINT("%s: param:", __func__);

	for (i = 0; i < RA_DBG_SUPPORT_PARAM_NUM; i++)
		MTWF_PRINT(" %d", param[i]);

	MTWF_PRINT("\n");

	status = ra_dbg_ctrl(pAd, param_num, param);

	return status;

error0:
	MTWF_PRINT("no parameter.\n");
	return FALSE;

error1:
	MTWF_PRINT("invalid parameter format (param num not larger than %d)\n",
		RA_DBG_SUPPORT_PARAM_NUM);
	return FALSE;

error2:
	MTWF_PRINT("param num not larger than %d\n", RA_DBG_SUPPORT_PARAM_NUM);
	return FALSE;
}

INT SetSKUDupCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	UCHAR sku_dup_en = 1;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			sku_dup_en = os_str_tol(arg, 0, 10);
			break;

		default: {
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((sku_dup_en != FALSE) && (sku_dup_en != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}

	pAd->CommonCfg.SKU_DUP_Patch_enable = sku_dup_en;
#ifdef SINGLE_SKU_V2
	MtPwrLimitTblChProc(pAd, pAd->CommonCfg.SKUBandIdx,
				pAd->CommonCfg.SKUChannelBand,
				pAd->CommonCfg.SKUControlChannel,
				pAd->CommonCfg.SKUCentralChannel);
#endif
	return TRUE;
}

INT SetSKUCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	BOOLEAN tx_pwr_sku_en = FALSE;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR	   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, ucBandIdx);

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			tx_pwr_sku_en = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((tx_pwr_sku_en != FALSE) && (tx_pwr_sku_en != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: TxPowerSKUEn: %d\n", __func__, tx_pwr_sku_en);
	/* Update Profile Info for SKU */
#ifdef SINGLE_SKU_V2
	pAd->CommonCfg.SKUenable = tx_pwr_sku_en;
#endif /* SINGLE_SKU_V2 */

#ifdef CONFIG_ATE
	/* Update SKU Status in ATECTRL Structure */
	ATECtrl->tx_pwr_sku_en = tx_pwr_sku_en;
#endif /* CONFIG_ATE */

#ifdef SINGLE_SKU_V2
	return TxPowerSKUCtrl(pAd, tx_pwr_sku_en, ucBandIdx);
#else
	return TRUE;
#endif /* SINGLE_SKU_V2 */
}

INT SetPercentageCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	BOOLEAN fgTxPowerPercentEn = FALSE;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, ucBandIdx);

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTxPowerPercentEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgTxPowerPercentEn != FALSE) && (fgTxPowerPercentEn != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: TxPowerPercentEn = %d\n", __func__, fgTxPowerPercentEn);

	/* Update Profile Info for Power Percentage */
	pAd->CommonCfg.PERCENTAGEenable = fgTxPowerPercentEn;

#ifdef CONFIG_ATE
	/* Update Power Percentage Status in ATECTRL Structure */
	ATECtrl->tx_pwr_percentage_en = fgTxPowerPercentEn;
#endif /* CONFIG_ATE */

	return TxPowerPercentCtrl(pAd, fgTxPowerPercentEn, ucBandIdx);
}

INT SetPowerDropCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	UINT8	ucPowerDrop = 0;
	UINT8   ucBandIdx = 0;
#ifdef MGMT_TXPWR_CTRL
	INT	 status;
	struct  wifi_dev *wdev;
	BSS_STRUCT *pMbss = NULL;
#endif
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */

	ucBandIdx = hc_get_hw_band_idx(pAd);
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucPowerDrop = simple_strtol(value, 0, 10);
			break;

		default: {
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: ucPowerDrop = %d\n", __func__, ucPowerDrop);

	/* Update Profile Info for Power Percentage Drop Value */
	pAd->CommonCfg.ucTxPowerPercentage = ucPowerDrop;

#ifdef CONFIG_ATE
	/* Update Power Percentage Drop Value Status in ATECTRL Structure */
	ATECtrl->tx_pwr_percentage_level = ucPowerDrop;
#endif /* CONFIG_ATE */
#ifdef MGMT_TXPWR_CTRL
	status = TxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx);
	if(!status) {
		MTWF_PRINT("ERROR: TxPowerDropCtrl() FAILED!\n ");
		return FALSE;
	}

	for(i=0; i < pAd->ApCfg.BssidNum; i++)
	{
		pMbss = &pAd->ApCfg.MBSSID[i];
		wdev = &pMbss->wdev;
		if(HcGetBandByWdev(wdev)!= ucBandIdx)
			continue;

		/*clear old data for iwpriv set channel*/
		wdev->bPwrCtrlEn = FALSE;
		wdev->TxPwrDelta = 0;
		wdev->mgmt_txd_txpwr_offset = 0;
		/* Get EPA info by Tx Power info cmd*/
		pAd->ApCfg.MgmtTxPwr = 0;
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, ucBandIdx);

		/* Update beacon/probe TxPwr wrt profile param */
		if (wdev->MgmtTxPwr) {
			/* wait until TX Pwr event rx*/
			RtmpusecDelay(50);
			update_mgmt_frame_power(pAd, wdev);
		}

	}
	return status;
#else
	return TxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx);
#endif
}

INT SetDecreasePwrCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
	BOOLEAN  fgStatus = FALSE;
	INT8  cPowerDropLevel = 0;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	cPowerDropLevel = simple_strtol(arg, 0, 10);

	MTWF_PRINT("%s: cPowerDropLevel (0.5 dBm) = %d\n", __func__, cPowerDropLevel);

	if (MtCmdTxPowerDropCtrl(pAd, cPowerDropLevel, ucBandIdx) == 0)
		fgStatus = TRUE;

	return fgStatus;
}

INT SetBfBackoffCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	BOOLEAN fgTxBFBackoffEn = FALSE;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_ATE */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, ucBandIdx);

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTxBFBackoffEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgTxBFBackoffEn != FALSE) && (fgTxBFBackoffEn != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: TxBFBackoffEn = %d\n", __func__, fgTxBFBackoffEn);

	/* Update Profile Info for Power Percentage */
	pAd->CommonCfg.BFBACKOFFenable = fgTxBFBackoffEn;

#ifdef CONFIG_ATE
	/* Update Power Percentage Drop Value Status in ATECTRL Structure */
	ATECtrl->tx_pwr_backoff_en = fgTxBFBackoffEn;
#endif /* CONFIG_ATE */

#ifdef SINGLE_SKU_V2
	return TxPowerBfBackoffCtrl(pAd, fgTxBFBackoffEn, ucBandIdx);
#else
	return TRUE;
#endif /* SINGLE_SKU_V2 */
}

INT SetThermoCompCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR	*value = 0;
	BOOLEAN fgThermoCompEn = 0;
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, ucBandIdx);

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgThermoCompEn = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((fgThermoCompEn != FALSE) && (fgThermoCompEn != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: fgThermoCompEn = %d\n", __func__, fgThermoCompEn);
	return ThermoCompCtrl(pAd, fgThermoCompEn, ucBandIdx);
}

INT SetCCKTxStream(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	CHAR    *value = 0;
	UINT8   u1CCKTxStream = 0;
	INT     status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;

	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev)
		ucBandIdx = HcGetBandByWdev(wdev);
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			u1CCKTxStream = simple_strtol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter range */
	if (u1CCKTxStream >= WF_NUM || !u1CCKTxStream) {
		MTWF_PRINT("set wrong parameters\n");
		MTWF_PRINT("EX. iwpriv <interface> set CCKTxStream=1 (1~4)\n");
		return FALSE;
	}

	/* Update Profile Info for Power Percentage Drop Value */
	pAd->CommonCfg.CCKTxStream = u1CCKTxStream;

	return TxCCKStreamCtrl(pAd, pAd->CommonCfg.CCKTxStream, ucBandIdx);
}

INT SetRfTxAnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	INT     status = TRUE;
	CHAR	*value = 0;
	UINT8   ucTxAntIdx = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 2) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format with 2-digit.\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxAntIdx = simple_strtol(value, 0, 10);
			break;
		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: ucTxAntIdx: 0x%x\n", __func__, ucTxAntIdx);
	return TxPowerRfTxAnt(pAd, ucTxAntIdx);
}

INT SetTxPowerInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	UCHAR   ucTxPowerInfoCatg = 0;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR  IfIdx = 0;
	IfIdx = pObj->ioctl_if;

	/* Grab Band index */
	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
			"AP Band = %d !!\n", ucBandIdx);

#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
			"STA Band = %d !!\n", ucBandIdx);
	} else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
			"STA Band = %d !!\n", ucBandIdx);
		}
#endif
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
			": pObj->ioctl_if_type = %d!!\n", pObj->ioctl_if_type);
		return FALSE;
	}

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN, "Invalid Band Index!!\n");
		goto error;
	}

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN, "No parameters!!\n");
		goto error;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxPowerInfoCatg = simple_strtol(value, 0, 10);
			break;

		default: {
			break;
		}
		}
	}

	/* Sanity check for parameter range */
	if (ucTxPowerInfoCatg >= POWER_INFO_NUM) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
			"parameter out of range!!\n");
		goto error;
	}

	MTWF_PRINT("%s: BandIdx: %d, ucTxPowerInfoCatg: %d\n",
		__func__, ucBandIdx, ucTxPowerInfoCatg);
#ifdef MGMT_TXPWR_CTRL
	g_fgCommand = TRUE;
#endif
	return TxPowerShowInfo(pAd, ucTxPowerInfoCatg, ucBandIdx);

error:

	MTWF_PRINT("iwpriv <interface> set TxPowerInfo=[param1]\n");
	MTWF_PRINT("	param1: Tx Power Info Category (0~3)\n");
	MTWF_PRINT("			0: Tx Power Basic Info\n");
	MTWF_PRINT("			1: Backup Power Table\n");
	MTWF_PRINT("			2: Tx Power Rate Power Info\n");
	MTWF_PRINT("			3: Thermal Compensation Table\n");
	MTWF_PRINT("			4: TXV/BBP Power Value (per packet)\n");

	return FALSE;
}

INT SetTOAECtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	 *value = 0;
	UCHAR   TOAECtrl = 0;
	INT	 status = TRUE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Wrong parameter format!!\n");
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			TOAECtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((TOAECtrl != FALSE) && (TOAECtrl != TRUE)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: TOAECtrl = %d\n", __func__, TOAECtrl);
	return TOAECtrlCmd(pAd, TOAECtrl);
}

#define SKU_FAIL 1
INT SetSKUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
#ifdef SINGLE_SKU_V2
	/* print out Sku table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, POWER_LIMIT_TABLE_TYPE_SKU, DBG_LVL_OFF))
		status = SKU_FAIL;
#endif /* SINGLE_SKU_V2 */
	return status;
}

INT SetBFBackoffInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
#ifdef SINGLE_SKU_V2
	/* print out Backoff table info */
	if (NDIS_STATUS_SUCCESS != MtShowPwrLimitTable(pAd, POWER_LIMIT_TABLE_TYPE_BACKOFF, DBG_LVL_OFF))
		status = SKU_FAIL;
#endif /* SINGLE_SKU_V2 */
	return status;
}

#define MT7986_TXD_TX_PWR_OFFSET_VALUE_MIN    -32
#define TXD_TX_PWR_OLD_OFFSET_VALUE_MIN       -16
#define TXD_TX_PWR_OLD_OFFSET_VALUE_MAX        15
#define TXD_TX_PWR_OFFSET_VALUE_MIN           -32
#define TXD_TX_PWR_OFFSET_VALUE_MAX            31

INT set_mgmt_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	INT8 txpwr_offset = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT8 txd_txpwr_min, txd_txpwr_max;
#ifdef CONFIG_AP_SUPPORT
	UCHAR apidx = pObj->ioctl_if;
#endif

	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif
	} else {
		MTWF_PRINT("pObj->ioctl_if_type = %d!!\n", pObj->ioctl_if_type);
		return FALSE;
	}

	if (arg != NULL) {
		txpwr_offset = os_str_tol(arg, 0, 10);
	} else {
		MTWF_PRINT("Invalid parameters!!\n");
		return FALSE;
	}

	txd_txpwr_min = TXD_TX_PWR_OFFSET_VALUE_MIN;
	txd_txpwr_max = TXD_TX_PWR_OFFSET_VALUE_MAX;

	if ((txpwr_offset < txd_txpwr_min) || (txpwr_offset > txd_txpwr_max)) {
		MTWF_PRINT("[TxdPwrOffset] range [%d,%d]\n", txd_txpwr_min, txd_txpwr_max);
		return FALSE;
	}

	MTWF_PRINT("%s: [TxdPwrOffset]: %u\n", __func__, (UINT8)txpwr_offset);

	wdev->mgmt_txd_txpwr_offset = (UINT8)txpwr_offset;

#ifdef CONFIG_AP_SUPPORT
	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));
	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}

INT show_BSSEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;
	struct mu_edca_cfg *mu_edca = NULL;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}
	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_PRINT("pBssEdca is NULL\n");
		return FALSE;
	}

	MTWF_PRINT("   EdcaUpdateCount: %d\n", pBssEdca->EdcaUpdateCount);

	/* WMM */
	MTWF_PRINT("   BSSEdca:     AIFSN    CWmin   CWmax    TXOP(us)  ACM\n");
	MTWF_PRINT("	   AC_BE	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[0],
							 pBssEdca->Cwmin[0],
							 pBssEdca->Cwmax[0],
							 pBssEdca->Txop[0],
							 pBssEdca->bACM[0]);
	MTWF_PRINT("	   AC_BK	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[1],
							 pBssEdca->Cwmin[1],
							 pBssEdca->Cwmax[1],
							 pBssEdca->Txop[1],
							 pBssEdca->bACM[1]);
	MTWF_PRINT("	   AC_VI	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[2],
							 pBssEdca->Cwmin[2],
							 pBssEdca->Cwmax[2],
							 pBssEdca->Txop[2],
							 pBssEdca->bACM[2]);
	MTWF_PRINT("	   AC_VO	  %2d	  %2d	  %2d	   %4d	   %d\n",
							 pBssEdca->Aifsn[3],
							 pBssEdca->Cwmin[3],
							 pBssEdca->Cwmax[3],
							 pBssEdca->Txop[3],
							 pBssEdca->bACM[3]);
	MTWF_PRINT("\n");

	mu_edca = wlan_config_get_he_mu_edca(wdev);
	if (mu_edca == NULL) {
		MTWF_PRINT("mu_edca is NULL\n");
		return FALSE;
	}

	/* MU EDCA */
	MTWF_PRINT("   MU Edca:     AIFSN    CWmin   CWmax    Timer(us)  ACM\n");
	MTWF_PRINT("	   AC_BE	  %2d	  %2d	  %2d	   %4d	    %d\n",
							mu_edca->mu_ac_rec[0].aifsn,
							mu_edca->mu_ac_rec[0].ecw_min,
							mu_edca->mu_ac_rec[0].ecw_max,
							mu_edca->mu_ac_rec[0].mu_edca_timer,
							mu_edca->mu_ac_rec[0].acm);
	MTWF_PRINT("	   AC_BK	  %2d	  %2d	  %2d	   %4d	    %d\n",
							mu_edca->mu_ac_rec[1].aifsn,
							mu_edca->mu_ac_rec[1].ecw_min,
							mu_edca->mu_ac_rec[1].ecw_max,
							mu_edca->mu_ac_rec[1].mu_edca_timer,
							mu_edca->mu_ac_rec[1].acm);
	MTWF_PRINT("	   AC_VI	  %2d	  %2d	  %2d	   %4d	    %d\n",
							mu_edca->mu_ac_rec[2].aifsn,
							mu_edca->mu_ac_rec[2].ecw_min,
							mu_edca->mu_ac_rec[2].ecw_max,
							mu_edca->mu_ac_rec[2].mu_edca_timer,
							mu_edca->mu_ac_rec[2].acm);
	MTWF_PRINT("	   AC_VO	  %2d	  %2d	  %2d	   %4d	    %d\n",
							mu_edca->mu_ac_rec[3].aifsn,
							mu_edca->mu_ac_rec[3].ecw_min,
							mu_edca->mu_ac_rec[3].ecw_max,
							mu_edca->mu_ac_rec[3].mu_edca_timer,
							mu_edca->mu_ac_rec[3].acm);

	return TRUE;
}

INT show_APEdca_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}
	MTWF_PRINT("   APEdca:     AIFSN     CWmin   CWmax    TXOP(us)  ACM\n");
	MTWF_PRINT("	   AC_BE	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[0],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[0]);
	MTWF_PRINT("	   AC_BK	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[1],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[1]);
	MTWF_PRINT("	   AC_VI	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[2],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[2]);
	MTWF_PRINT("	   AC_VO	%2d	  %2d	  %2d	   %4d	     %d\n",
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[3],
							 pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].bACM[3]);

	return TRUE;
}

#ifdef AMPDU_CONF_SUPPORT
#define MAX_MPDU_COUNT_FOR_AGG   1024
INT Set_AMPDU_MPDU_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 count = 0;
	EXT_CMD_CFG_SET_AGG_AC_LIMIT_T ExtCmdAggAcLimitCfg = {0};
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UCHAR i = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T UniCmdBandCfgAGGAcLimit = {0};
#endif /* WIFI_UNIFIED_COMMAND */

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_WARN,
			    "Incoorect BSS!!\n");
		return FALSE;
	}
	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_WARN,
			    "No parameters!!\n");
		return FALSE;
	}
	count = os_str_tol(arg, 0, 10);
	if (count > MAX_MPDU_COUNT_FOR_AGG) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_WARN,
			    "Max MPDU in AMPDU support = %d!\n", MAX_MPDU_COUNT_FOR_AGG);
		return FALSE;
	}
#ifdef WIFI_UNIFIED_COMMAND
	UniCmdBandCfgAGGAcLimit.ucWmmIdx = HcGetWmmIdx(pAd, wdev);
	UniCmdBandCfgAGGAcLimit.ucAggLimit = count;
#endif
	ExtCmdAggAcLimitCfg.ucWmmIdx = HcGetWmmIdx(pAd, wdev);
	ExtCmdAggAcLimitCfg.ucAggLimit = count;

	for (i = QID_AC_BK; i < QID_HCCA; i++) {
#ifdef WIFI_UNIFIED_COMMAND
		UniCmdBandCfgAGGAcLimit.ucAc = i;
		if (cap->uni_cmd_support) {
			UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_AGG_AC_LIMT_FEATURE, &UniCmdBandCfgAGGAcLimit);
		} else
#endif /* WIFI_UNIFIED_COMMAND */
		{
			ExtCmdAggAcLimitCfg.ucAc = i;
			CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_AGG_AC_LIMT_FEATURE, &ExtCmdAggAcLimitCfg);
		}
	}
	MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_NOTICE,
			"Number of MPDUs in AMPDU set = %d!!\n", count);
	return TRUE;
}
#define AMPDU_MAX_REMAIN_TX_COUNT 0x1F
INT Show_AMPDU_Retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	UCHAR count = 0;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (wdev == NULL) {
		MTWF_PRINT("Incorrect BSS!!\n");
		return FALSE;
	}
	count = wdev->bAMPDURetrynum;

	if (count == AMPDU_MAX_REMAIN_TX_COUNT)
		MTWF_PRINT("%s(): Unlimited retry!\n", __func__);
	else if (count == 0)
		MTWF_PRINT("%s(): Auto retry control!\n", __func__);
	else
		MTWF_PRINT("%s(): Tx retry count = %d!\n", __func__, count - 1);
	return TRUE;
}

INT Set_AMPDU_Retry_Count(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 count = 0;
	struct wifi_dev *wdev = NULL;
	int bss_idx;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_WARN, "Incorrect BSS!!\n");
		return FALSE;
	}
	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_WARN, "No parameters!!\n");
		return FALSE;
	}
	count = os_str_tol(arg, 0, 10);
	if (count > AMPDU_MAX_REMAIN_TX_COUNT) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_WARN,
			"Error, Retry range is: [0-1F]!\n");
		return FALSE;
	}
	bss_idx = hc_get_bssidx(wdev);
	if (bss_idx < 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_TX, DBG_LVL_WARN,
			"Error, Can't get right bss_idx!\n");
		return FALSE;
	}
	wdev->bAMPDURetrynum = count;
	MtCmdCr4Set(pAd, WA_SET_AMPDU_RETRY_COUNT, (UINT32)bss_idx, count);
	if (count == AMPDU_MAX_REMAIN_TX_COUNT)
		MTWF_PRINT("%s(): Unlimited retry!\n", __func__);
	else if (count == 0)
		MTWF_PRINT("%s(): Auto retry control!\n", __func__);
	else
		MTWF_PRINT("%s(): Set Tx retry count = %d!\n", __func__, count - 1);
	return TRUE;
}
#endif

#ifdef WIFI_EAP_FEATURE
INT SetInitIPICtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   BandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return 0;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT(" Invalid Band Index!!\n");
		return 0;
	}

	MTWF_PRINT("%s: Band Index:%u\n", __func__, BandIdx);
	return InitIPICtrl(pAd, BandIdx);
}

INT ShowIPIValue(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   BandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return 0;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT(" Invalid Band Index!!\n");
		return 0;
	}
	MTWF_PRINT("%s: Band Index:%u\n", __func__, BandIdx);

	return GetIPIValue(pAd, BandIdx);
}

#define WTBL_TX_PWR_OFFSET_VALUE_MIN -16
#define WTBL_TX_PWR_OFFSET_VALUE_MAX 0

INT set_data_txpwr_offset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 wlan_id = 0;
	INT8 txpwr_offset = 0;
	PCHAR pch = NULL;
	UINT8 BandIdx = 0;
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev)
		BandIdx = HcGetBandByWdev(wdev);
	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("Invalid Band Index!!\n");
		return FALSE;
	}

	if (arg == NULL) {
		MTWF_PRINT("Invalid parameters\n");
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		wlan_id = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	txpwr_offset = (INT8) os_str_tol(arg, 0, 10);

	if (txpwr_offset < WTBL_TX_PWR_OFFSET_VALUE_MIN
			|| txpwr_offset > WTBL_TX_PWR_OFFSET_VALUE_MAX) {
		MTWF_PRINT("txpwr_offset range [-16,0]\n");
		return FALSE;
	}

	MTWF_PRINT("%s: wlanid:%u txpwr_offset:%d BandIdx:%u\n",
			__func__, wlan_id, txpwr_offset, BandIdx);

	return SetDataTxPwrOffset(pAd, wlan_id, txpwr_offset, BandIdx);
}

INT update_switch_tbl_to_fw(RTMP_ADAPTER *pAd,
	UINT8 BandIdx, UINT8 CmdTblIndex, CHAR *buf)
{
	CHAR *readline = NULL, *token = NULL, *ptr = NULL;
	INT ElemIdx = 0, TableSize = 0, table_updated = 0, retval = FALSE;
	UINT8 TblIndex = RA_TBL_INDEX_INVALID;
	UINT8 SwRaTable[512];
	PUINT8 Table = NULL;
	UINT32 rv = 0;

	for (readline = ptr = buf; (ptr = os_str_chr(readline, '\n')) != NULL;
			readline = ptr + 1) {

		if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx
				&& !isdigit(readline[0])) {
			Table = (PUINT8) SwRaTable;
			TableSize = ElemIdx * sizeof(SwRaTable[0]);

			if (Table) {
				retval = SetFwRaTable(pAd, BandIdx, eRateSwitchTable, TblIndex,
						TableSize, (PUCHAR)Table);

				if (retval) {
					table_updated++;
					MTWF_PRINT("Successfully updated %s\n", token);
				}
			}

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID) && table_updated)
				return table_updated;

			TblIndex = RA_TBL_INDEX_INVALID;
			TableSize = 0;
			Table = NULL;
			token = NULL;
			ElemIdx = 0;
		}

		/* Table Name Parsing */
		if (!strncmp(readline, "Table:", 6)) {
			token = rstrtok(readline + 6, "\n");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			TblIndex = getRaTableIndex(eRateSwitchTable, token);

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID)
					&& (TblIndex != CmdTblIndex))
				TblIndex = RA_TBL_INDEX_INVALID;

			if (TblIndex == RA_TBL_INDEX_INVALID)
				continue;

			os_zero_mem(SwRaTable, sizeof(SwRaTable));
		}

		if ((TblIndex < RA_TBL_INDEX_INVALID) && isdigit(readline[0])) {
			rv = sscanf(readline,
					"%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu"
					"\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\t%hhu\n",
					&SwRaTable[ElemIdx], &SwRaTable[ElemIdx + 1],
					&SwRaTable[ElemIdx + 2], &SwRaTable[ElemIdx + 3],
					&SwRaTable[ElemIdx + 4], &SwRaTable[ElemIdx + 5],
					&SwRaTable[ElemIdx + 6], &SwRaTable[ElemIdx + 7],
					&SwRaTable[ElemIdx + 8], &SwRaTable[ElemIdx + 9],
					&SwRaTable[ElemIdx + 10], &SwRaTable[ElemIdx + 11],
					&SwRaTable[ElemIdx + 12], &SwRaTable[ElemIdx + 13],
					&SwRaTable[ElemIdx + 14]);

			if (rv > 0)
				ElemIdx += NUM_OF_COL_RATE_SWITCH_TABLE;

		}
	}

	if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx) {
		Table = (PUINT8) SwRaTable;
		TableSize = ElemIdx * sizeof(SwRaTable[0]);
	}

	if (Table) {
		retval = SetFwRaTable(pAd, BandIdx, eRateSwitchTable, TblIndex,
				TableSize, (PUCHAR)Table);

		if (retval) {
			table_updated++;
			MTWF_PRINT("Successfully updated %s\n", token);
		}
	}

	return table_updated;
}

INT udpate_hwfb_tbl_to_fw(RTMP_ADAPTER *pAd,
	UINT8 BandIdx, UINT8 CmdTblIndex, CHAR *buf)
{
	CHAR *readline = NULL, *token = NULL, *ptr = NULL;
	INT ElemIdx = 0, TableSize = 0, table_updated = 0, retval = FALSE;
	UINT8 TblIndex = RA_TBL_INDEX_INVALID;
	UINT16 HwFbRaTable[256];
	PUINT8 Table = NULL;
	UINT32 rv = 0;

	for (readline = ptr = buf; (ptr = os_str_chr(readline, '\n')) != NULL;
			readline = ptr + 1) {

		if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx
				&& !isdigit(readline[0])) {
			Table = (PUINT8) HwFbRaTable;
			TableSize = ElemIdx * sizeof(HwFbRaTable[0]);

			if (Table) {
				retval = SetFwRaTable(pAd, BandIdx, eRateHwFbTable, TblIndex,
						TableSize, (PUCHAR)Table);

				if (retval) {
					table_updated++;
					MTWF_PRINT("Successfully updated %s\n", token);
				}
			}

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID) && table_updated)
				return table_updated;

			TblIndex = RA_TBL_INDEX_INVALID;
			TableSize = 0;
			Table = NULL;
			token = NULL;
			ElemIdx = 0;
		}

		/* Table Name Parsing */
		if (!strncmp(readline, "Table:", 6)) {
			token = rstrtok(readline + 6, "\n");

			/* sanity check for non-Null pointer */
			if (!token)
				continue;

			TblIndex = getRaTableIndex(eRateHwFbTable, token);

			if ((CmdTblIndex != RA_TBL_INDEX_INVALID)
					&& (TblIndex != CmdTblIndex))
				TblIndex = RA_TBL_INDEX_INVALID;

			if (TblIndex == RA_TBL_INDEX_INVALID)
				continue;

			os_zero_mem(HwFbRaTable, sizeof(HwFbRaTable));
		}

		if ((TblIndex < RA_TBL_INDEX_INVALID) && isdigit(readline[0])) {
			rv = sscanf(readline,
					"%hu\t%hu\t%hu\t%hu\t%hu\t%hu\t%hu\t%hu\n",
					&HwFbRaTable[ElemIdx], &HwFbRaTable[ElemIdx + 1],
					&HwFbRaTable[ElemIdx + 2], &HwFbRaTable[ElemIdx + 3],
					&HwFbRaTable[ElemIdx + 4], &HwFbRaTable[ElemIdx + 5],
					&HwFbRaTable[ElemIdx + 6], &HwFbRaTable[ElemIdx + 7]);

			if (rv > 0)
				ElemIdx += NUM_OF_COL_RATE_HWFB_TABLE;
		}
	}

	if ((TblIndex < RA_TBL_INDEX_INVALID) && ElemIdx) {
		Table = (PUINT8) HwFbRaTable;
		TableSize = ElemIdx * sizeof(HwFbRaTable[0]);
	}

	if (Table) {
		retval = SetFwRaTable(pAd, BandIdx, eRateHwFbTable, TblIndex,
				TableSize, (PUCHAR)Table);

		if (retval) {
			table_updated++;
			MTWF_PRINT("Successfully updated %s\n", token);
		}
	}

	return table_updated;
}

PCHAR get_fname_for_fw_ratbl (RTMP_ADAPTER *pAd, UINT8 TblType)
{
	CHAR *fname = NULL;

	return fname;
}

INT read_fw_ratbl_from_file(RTMP_ADAPTER *pAd, UINT8 BandIdx, UINT8 TblType, UINT8 CmdTblIndex)
{
	RTMP_OS_FD_EXT srcf;
	ULONG buf_size = MAX_INI_BUFFER_SIZE;
	CHAR *fname = NULL, *buf = NULL;
	INT table_updated = 0, retval = FALSE;

	fname = get_fname_for_fw_ratbl(pAd, TblType);

	if (!fname)
		return FALSE;

	MTWF_PRINT("file \"%s\"!\n", fname);

	os_file_open(fname, (RTMP_OS_FD_EXT *)&srcf, O_RDONLY, 0);

	if (srcf.Status) {
		MTWF_PRINT("Open file \"%s\" failed!\n", fname);
		return FALSE;
	}

	os_alloc_mem(pAd, (UCHAR **)&buf, buf_size);

	if (!buf)
		goto close_file;

	os_zero_mem(buf, buf_size);

	retval = os_file_read(srcf, buf, MAX_INI_BUFFER_SIZE - 1);

	if (retval <= 0) {
		MTWF_PRINT("Read file \"%s\" failed(errCode=%d)!\n", fname, retval);
		goto close_file;
	}

	if (TblType == eRateSwitchTable)
		table_updated = update_switch_tbl_to_fw(pAd, BandIdx, CmdTblIndex, buf);

	if (TblType == eRateHwFbTable)
		table_updated = udpate_hwfb_tbl_to_fw(pAd, BandIdx, CmdTblIndex, buf);

close_file:
	if (table_updated) {
		MTWF_PRINT("Successfully %d RA tables updated!\n", table_updated);
		retval = TRUE;
	} else {
		MTWF_PRINT("RA table not found!\n");
		retval = FALSE;
	}

	if (buf)
		os_free_mem(buf);

	if (os_file_close(srcf) != 0) {
		retval = FALSE;
		MTWF_PRINT("Close file \"%s\" failed(errCode=%d)!\n", fname, retval);
	}

	return retval;
}

INT set_fw_ratbl_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	CHAR *pch = NULL;
	UINT8 BandIdx = 0, TblType;
	UINT8 CmdTblIndex = RA_TBL_INDEX_INVALID;

	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev)
		BandIdx = HcGetBandByWdev(wdev);
	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("Invalid Band Index!!\n");
		return FALSE;
	}

	if (arg == NULL) {
		MTWF_PRINT("Invalid parameters\n");
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		TblType = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters for TblType!!\n");
		return FALSE;
	}

	if (TblType >= eRateTableMax) {
		MTWF_PRINT("TblType value should be less than %d!\n",
				eRateTableMax);
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		CmdTblIndex = (UINT8) os_str_toul(pch, 0, 10);

	MTWF_PRINT("%s: TblType:%u TblIndex:%u BandIdx:%u!\n", __func__,
			 TblType, CmdTblIndex, BandIdx);

	return read_fw_ratbl_from_file(pAd, BandIdx, TblType, CmdTblIndex);
}

INT show_ratbl_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, TblType = 0, TblIndex = 0, ReadnWrite = 0;
	struct wifi_dev *wdev;
	PCHAR pch = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev)
		BandIdx = HcGetBandByWdev(wdev);
	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("Invalid Band Index!!\n");
		return FALSE;
	}

	if (arg == NULL) {
		MTWF_PRINT("Invalid parameters\n");
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		TblType = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters for TblType!!\n");
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		TblIndex = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters for TblIndex!!\n");
		return FALSE;
	}

	pch = arg;

	if (pch != NULL)
		ReadnWrite = (UINT8) os_str_toul(pch, 0, 10);

	if (ReadnWrite)
		ReadnWrite = 1;

	MTWF_PRINT("%s: Band Index:%u TblType:%u TblIndex:%u RW:%u\n",
			 __func__, BandIdx, TblType, TblIndex, ReadnWrite);

	return GetRaTblInfo(pAd, BandIdx, TblType, TblIndex, ReadnWrite);
}
#endif /* WIFI_EAP_FEATURE */

INT SetEDCCAThresholdCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 i;
	UINT8 utargetband = 4;
	UINT8 edcca_threshold[EDCCA_MAX_BW_NUM] = {0x7f, 0x7f, 0x7f, 0x7f};
	UINT8 BandIdx = 0;
	RTMP_STRING *str = NULL;
	UINT_32 status;
	struct wifi_dev *wdev;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return 0;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("%s: Invalid Band Index!!\n", __func__);
		return 0;
	}

	if (!arg)
		return 0;

	str = strsep(&arg, ":");

	if (str != NULL)
		utargetband = os_str_toul(str, 0, 10);

	if (utargetband > EDCCA_BW160) {
		if (utargetband == EDCCA_RESET_NUM) {
			for (i = 0 ; i < EDCCA_MAX_BW_NUM ; i++)
				edcca_threshold[i] = pAd->CommonCfg.u1EDCCAThreshold[i];

#ifdef WIFI_UNIFIED_COMMAND
			if (cap->uni_cmd_support)
				status = UniCmdSetEDCCAThreshold(pAd, (PUCHAR)edcca_threshold, BandIdx, FALSE);
			else
#endif /* WIFI_UNIFIED_COMMAND */
				status = SetEDCCAThreshold(pAd, (PUCHAR)edcca_threshold, BandIdx);
			return status;
		}
		MTWF_PRINT("%s: Wrong number of bands!\n", __func__);
		return FALSE;
	}

	str = strsep(&arg, ":");
	if (str != NULL) {
		edcca_threshold[utargetband] = os_str_tol(str, 0, 10);
		if (edcca_threshold[utargetband] > 0x00 && edcca_threshold[utargetband] < 0x7f) {
			edcca_threshold[utargetband] = ~edcca_threshold[utargetband]+1;
		} else {
			MTWF_PRINT("%s: Invalid Setting\n", __func__);
			return FALSE;
		}
		/* Save cmd setting to pAd if profile EDCCACfgMode is set to 1 */
		if (pAd->CommonCfg.u1EDCCACfgMode == 1)
			pAd->CommonCfg.u1EDCCAThreshold[utargetband] = edcca_threshold[utargetband];
	} else {
		MTWF_PRINT("%s: Invalid Setting\n", __func__);
		return FALSE;
	}

	MTWF_PRINT("%s: EdccaThreshold:%u Band Index:%u\n",
		__func__, edcca_threshold[0], BandIdx);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		return UniCmdSetEDCCAThreshold(pAd, (PUCHAR)edcca_threshold, BandIdx, FALSE);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		return SetEDCCAThreshold(pAd, (PUCHAR)edcca_threshold, BandIdx);
}

INT SetEDCCAEnableCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8		i;
	CHAR		*value = 0;
	UINT8	   u1BandIdx = 0;
	UINT8	   u1EDCCACtrl = 0;
	UINT8      u1EDCCAStd = 0;
	INT		 status;
	struct wifi_dev *wdev;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR	   apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	u1BandIdx = HcGetBandByWdev(wdev);
	MTWF_PRINT("%s: BandIdx = %d\n", __func__, u1BandIdx);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	/* sanity check for Band index */
	if (u1BandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("BandIdx = %d. Improper Band Index.\n", u1BandIdx);
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			u1EDCCACtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	/* sanity check for input parameter */
	if ((u1EDCCACtrl != FALSE) && (u1EDCCACtrl != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}

	MTWF_PRINT("%s: EDCCACtrl = %d\n", __func__, u1EDCCACtrl);
	u1EDCCAStd = GetEDCCAStd(pAd, pAd->CommonCfg.CountryCode, wdev->PhyMode);
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		status = UniCmdSetEDCCAEnable(pAd, u1EDCCACtrl, u1BandIdx, u1EDCCAStd);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		status = SetEDCCAEnable(pAd, u1EDCCACtrl, u1BandIdx);
	/* Save cmd setting to pAd if profile EDCCAKeepCmdSetting is set to 1 */
	if (pAd->CommonCfg.u1EDCCACfgMode == 1 && status == TRUE)
		pAd->CommonCfg.u1EDCCACtrl = u1EDCCACtrl;

	return status;
}

INT ShowEDCCAThreshold(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1BandIdx = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	u1BandIdx = hc_get_hw_band_idx(pAd);
	MTWF_PRINT("%s: BandIdx = %d\n", __func__, u1BandIdx);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		if (UniCmdGetEDCCAThreshold(pAd, u1BandIdx, FALSE) != TRUE) {
			MTWF_PRINT("BandIdx = %d can not read EDCCAThreshold\n",
					u1BandIdx);
			return FALSE;
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdGetEDCCAThreshold(pAd, u1BandIdx, FALSE) != RETURN_STATUS_TRUE) {
			MTWF_PRINT("BandIdx = %d can not read EDCCAThreshold\n",
				 u1BandIdx);
			return FALSE;
		}
	}
	return TRUE;
}

INT ShowEDCCAEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1BandIdx = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	u1BandIdx = hc_get_hw_band_idx(pAd);
	MTWF_PRINT("%s: BandIdx = %d\n", __func__, u1BandIdx);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		if (UniCmdGetEDCCAEnable(pAd, u1BandIdx) != TRUE) {
			MTWF_PRINT("BandIdx = %d can not read EDCCACtrl\n", u1BandIdx);
			return FALSE;
		}
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		if (MtCmdGetEDCCAEnable(pAd, u1BandIdx) != RETURN_STATUS_TRUE) {
			MTWF_PRINT("BandIdx = %d can not read EDCCACtrl\n", u1BandIdx);
			return FALSE;
		}
	}
	return TRUE;
}

#ifdef WIFI_GPIO_CTRL
INT set_gpio_ctrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *pch = NULL;
	UINT8 gpio_idx;
	BOOLEAN gpio_en = FALSE;

	if (arg == NULL) {
		MTWF_PRINT("%s: Invalid parameters\n", __func__);
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		gpio_idx = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters for gpio_idx!!\n");
		return FALSE;
	}

	if (gpio_idx < GPIO_INDEX_MIN_VAL) {
		MTWF_PRINT("gpio_idx value should not be less than %d!\n",
				GPIO_INDEX_MIN_VAL);
		return FALSE;
	}

	if (gpio_idx > GPIO_INDEX_MAX_VAL) {
		MTWF_PRINT("gpio_idx value should not be greater than %d!\n",
				GPIO_INDEX_MAX_VAL);
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		gpio_en = (BOOLEAN) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters for gpio_en!!\n");
		return FALSE;
	}

	if (gpio_en)
		gpio_en = TRUE;

	MTWF_PRINT("%s: gpio_num:%u Enable:%u\n",
			__func__, gpio_idx, gpio_en);

	return SetGpioCtrl(pAd, gpio_idx, gpio_en);
}

INT set_gpio_value(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR *pch = NULL;
	UINT8 gpio_idx, gpio_val;

	if (arg == NULL) {
		MTWF_PRINT("%s: Invalid parameters\n", __func__);
		return FALSE;
	}

	pch = strsep(&arg, ":");

	if (pch != NULL)
		gpio_idx = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters for gpio_idx!!\n");
		return FALSE;
	}

	if (gpio_idx < GPIO_INDEX_MIN_VAL) {
		MTWF_PRINT("gpio_idx value should not be less than %d!\n",
				GPIO_INDEX_MIN_VAL);
		return FALSE;
	}

	if (gpio_idx > GPIO_INDEX_MAX_VAL) {
		MTWF_PRINT("gpio_idx value should not be greater than %d!\n",
				GPIO_INDEX_MAX_VAL);
		return FALSE;
	}

	pch = arg;
	if (pch != NULL)
		gpio_val = (UINT8) os_str_toul(pch, 0, 10);
	else {
		MTWF_PRINT("No parameters for gpio_val!!\n");
		return FALSE;
	}

	if (gpio_val)
		gpio_val = 1;

	MTWF_PRINT("%s: gpio_num:%u Value:%u\n",
			__func__, gpio_idx, gpio_val);

	return SetGpioValue(pAd, gpio_idx, gpio_val);
}
#endif /* WIFI_GPIO_CTRL */

INT SetRxvEnCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 i;
	CHAR *value = 0;
	BOOLEAN fgRxvEnCtrl = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("%s: Invalid parameters\n", __func__);
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgRxvEnCtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	RxvEnCtrl(pAd, fgRxvEnCtrl);

	return status;
}

INT SetRxvRuCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 i;
	CHAR *value = 0;
	UINT8 u1RxvRuCtrl = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("%s: Invalid parameters\n", __func__);
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			u1RxvRuCtrl = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("%s: set wrong parameters\n", __func__);
			break;
		}
		}
	}

	RxvRuCtrl(pAd, u1RxvRuCtrl);

	return status;
}

INT SetRxvInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 band_idx = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("no parameter.\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":");
		value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			band_idx = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_PRINT("number of parameters exceed expectation.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d\n", __func__, band_idx);

	/* show rxv info */
	chip_show_rxv_info(pAd, band_idx);

	return status;
}

INT SetRxvRawDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 band_idx = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("no parameter.\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":");
		value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			band_idx = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_PRINT("number of parameters exceed expectation.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d\n", __func__, band_idx);

	/* rxv raw data dump */
	chip_dump_rxv_raw_data(pAd, band_idx);

	return status;
}

INT SetRxvStatReset(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UINT8 band_idx = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("no parameter.\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			band_idx = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_PRINT("number of parameters exceed expectation.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d\n", __func__, band_idx);

	/* rxv raw data dump */
	chip_reset_rxv_stat(pAd, band_idx);

	return status;
}

INT SetRxvLogCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 ctrl = 0, type_mask = 0;
	UINT8 i;
	CHAR *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("no parameter.\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ctrl = os_str_tol(value, 0, 10);
			break;
		case 1:
			type_mask = os_str_tol(value, 0, 10);
			break;
		default:
			MTWF_PRINT("number of parameters exceed expectation.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): ctrl: %d, type_mask: %d\n", __func__, ctrl, type_mask);

	/* rxv log handle */
	switch (ctrl) {
	case RXV_DUMP_START:
		chip_rxv_dump_start(pAd);
		break;
	case RXV_DUMP_STOP:
		chip_rxv_dump_stop(pAd);
		break;
	case RXV_DUMP_ALLOC:
		chip_rxv_dump_buf_alloc(pAd, type_mask);
		break;
	case RXV_DUMP_CLEAR:
		chip_rxv_dump_buf_clear(pAd);
		break;
	case RXV_DUMP_REPORT:
		break;
	default:
		goto error0;
	}

	return TRUE;

error0:
	MTWF_PRINT(" invalid ctrl param(%d)\n", ctrl);
	return FALSE;
}

INT SetRxvListInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* rxv list info handle */
	chip_rxv_dump_show_list(pAd);

	return TRUE;
}

INT SetRxvRptInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* rxv list info handle */
	chip_rxv_dump_show_rpt(pAd);

	return TRUE;
}

#ifdef LINK_TEST_SUPPORT
INT SetLinkTestRxParamCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 25) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":");
		value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			/* 2 symbol representation */
			pAd->ucRssiSigniTh = simple_strtol(value, 0, 10);
			break;
		case 1:
			/* 3 symbol representation */
			pAd->cWBRssiTh = simple_strtol(value, 0, 10);
			break;
		case 2:
			/* 3 symbol representation */
			pAd->cIBRssiTh = simple_strtol(value, 0, 10);
			break;
		case 3:
			/* 2 symbol representation */
			pAd->c8RxCountTh = simple_strtol(value, 0, 10);
			break;
		case 4:
			/* 3 symbol representation */
			pAd->ucTimeOutTh = simple_strtol(value, 0, 10);
			break;
		case 5:
			/* 3 symbol representation */
			pAd->cNrRssiTh = simple_strtol(value, 0, 10);
			break;
		case 6:
			/* 3 symbol representation */
			pAd->cChgTestPathTh = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): ucRssiSigniTh: %d, cWBRssiTh: %d, cIBRssiTh: %d\n", __func__,
			pAd->ucRssiSigniTh, pAd->cWBRssiTh, pAd->cIBRssiTh);

	MTWF_PRINT("%s(): c8RxCountTh: %lld, ucTimeOutTh: %d\n", __func__,
			pAd->c8RxCountTh, pAd->ucTimeOutTh);

	MTWF_PRINT("%s(): cNrRssiTh: %d, cChgTestPathTh: %d\n", __func__,
			pAd->cNrRssiTh, pAd->cChgTestPathTh);

	return TRUE;
}

INT SetLinkTestModeCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			pAd->fgTxSpurEn = simple_strtol(value, 0, 10);
			break;
		case 1:
			pAd->fgRxSensitEn = simple_strtol(value, 0, 10);
			break;
		case 2:
			pAd->fgACREn = simple_strtol(value, 0, 10);
			break;
		case 3:
			pAd->fgTxSpeEn = simple_strtol(value, 0, 10);
			break;
		case 4:
			pAd->fgRxRcpiEn = simple_strtol(value, 0, 10);
			break;
		case 5:
			pAd->cMaxInRssiTh = simple_strtol(value, 0, 10);
			break;
		case 6:
			pAd->ucACRConfidenceCntTh = simple_strtol(value, 0, 10);
			break;
		case 7:
			pAd->ucMaxInConfidenceCntTh = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s: fgTxSpurEn: %d, fgRxSensitEn: %d, fgACREn: %d, fgTxSpeEn: %d\n", __func__,
			pAd->fgTxSpurEn, pAd->fgRxSensitEn, pAd->fgACREn, pAd->fgTxSpeEn);

	MTWF_PRINT("%s: fgRxRcpiEn: %d, cMaxInRssiTh: %d, ucACRConfidenceCntTh: %d, ucMaxInConfidenceCntTh: %d\n", __func__,
			pAd->fgRxRcpiEn, pAd->cMaxInRssiTh, pAd->ucACRConfidenceCntTh, pAd->ucMaxInConfidenceCntTh);

	return TRUE;
}

INT SetLinkTestPowerUpTblCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value;
	UINT8	ucTxPwrUpCat = 0;
	UINT8	ucTxPwrUpRate = 0;
	UINT8	ucTxPwrUpValue = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			ucTxPwrUpCat = simple_strtol(value, 0, 10);
			break;
		case 1:
			ucTxPwrUpRate = simple_strtol(value, 0, 10);
			break;
		case 2:
			ucTxPwrUpValue = simple_strtol(value, 0, 10);
			break;
		default:
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s: ucTxPwrUpCat: %d, ucTxPwrUpRate: %d, ucTxPwrUpValue: %d\n", __func__,
			ucTxPwrUpCat, ucTxPwrUpRate, ucTxPwrUpValue);

	/* update Tx Power Up Table */
	pAd->ucTxPwrUpTbl[ucTxPwrUpCat][ucTxPwrUpRate] = ucTxPwrUpValue;

	/* sync Tx Power up table to Firmware */
	MtCmdLinkTestTxPwrUpTblCtrl(pAd, ucTxPwrUpCat, pAd->ucTxPwrUpTbl[ucTxPwrUpCat]);

	return TRUE;
}

INT SetLinkTestPowerUpTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  ucRateIdx, ucCatIdx;
	CHAR   cRateInfo[CMW_POWER_UP_RATE_NUM][12] = {"CCK_1M2M", "CCK_5M11M", "OFDM_6M9M", "OFDM_12M18M", "OFDM_24M36M", "OFDM_48M", "OFDM_54M",
						       "HT20_MCS0", "HT20_MCS12", "HT20_MCS34", "HT20_MCS5", "HT20_MCS6", "HT20_MCS7"
						      };

	MTWF_PRINT("================================================================================\n");
	MTWF_PRINT("						Link Test Power Up Table\n");
	MTWF_PRINT("================================================================================\n");

	MTWF_PRINT("				2G(S)		5G(S)		2G(D)		5G(D)\n");

	for (ucRateIdx = 0; ucRateIdx < CMW_POWER_UP_RATE_NUM; ucRateIdx++) {
		MTWF_PRINT("%s:  ", cRateInfo[ucRateIdx]);
		for (ucCatIdx = 0; ucCatIdx < CMW_POWER_UP_CATEGORY_NUM; ucCatIdx++)
			MTWF_PRINT("		%d", pAd->ucTxPwrUpTbl[ucCatIdx][ucRateIdx]);
		MTWF_PRINT("\n");
	}

	return TRUE;
}

INT SetLinkTestInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4value;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */
	UINT BandIdx = hc_get_hw_band_idx(pAd);

	MTWF_PRINT("================================================================================\n");
	MTWF_PRINT("								 Link Status\n");
	MTWF_PRINT("================================================================================\n\n");

	MTWF_PRINT("Link Up Done: %d\n", pAd->fgCmwLinkDone);
	MTWF_PRINT("ChannelBand: %dG(Band%d)\n", pAd->ucCmwChannelBand ? 2 : 5, BandIdx);
	MTWF_PRINT("CSD State: %d\n", pAd->ucTxCsdState);
	MTWF_PRINT("Power Up State: %d\n", pAd->ucTxPwrBoostState);
	MTWF_PRINT("RxStream Specific Rx Path Index: %d\n", pAd->ucRxStreamState);
	MTWF_PRINT("Rx Filter State: %d\n", pAd->ucRxFilterstate);
	MTWF_PRINT("ACR Confidence count and Threshoild: %d %d(Threshold)\n", pAd->ucRxFilterConfidenceCnt, pAd->ucACRConfidenceCntTh);
	MTWF_PRINT("MaxIn Confidence count and Threshoild: %d %d(Threshold)\n", pAd->ucRxFilterConfidenceCnt, pAd->ucMaxInConfidenceCntTh);
	MTWF_PRINT("================================================================================\n");
	MTWF_PRINT("								Tx Antenna Status\n");
	MTWF_PRINT("================================================================================\n\n");

	/* Read RF CR */
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdRFRegAccessRead(pAd, (UINT32)WF0, (UINT32)0x48, (UINT32 *)&u4value);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdRFRegAccessRead(pAd, (UINT32)WF0, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_PRINT("WF%d 0x%04x 0x%08x\n", WF0, 0x48, u4value);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdRFRegAccessRead(pAd, (UINT32)WF1, (UINT32)0x48, (UINT32 *)&u4value);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdRFRegAccessRead(pAd, (UINT32)WF1, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_PRINT("WF%d 0x%04x 0x%08x\n", WF1, 0x48, u4value);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdRFRegAccessRead(pAd, (UINT32)WF2, (UINT32)0x48, (UINT32 *)&u4value);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdRFRegAccessRead(pAd, (UINT32)WF2, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_PRINT("WF%d 0x%04x 0x%08x\n", WF2, 0x48, u4value);

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		UniCmdRFRegAccessRead(pAd, (UINT32)WF3, (UINT32)0x48, (UINT32 *)&u4value);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdRFRegAccessRead(pAd, (UINT32)WF3, (UINT32)0x48, (UINT32 *)&u4value);
	MTWF_PRINT("WF%d 0x%04x 0x%08x\n", WF3, 0x48, u4value);

	MTWF_PRINT("================================================================================\n");

	return TRUE;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef ETSI_RX_BLOCKER_SUPPORT
/* Set fix WBRSSI/IBRSSI pattern */
INT SetFixWbIbRssiCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return  FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return  FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 1 symbol representation */
		case 0:
			pAd->fgFixWbIBRssiEn = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return  FALSE;
		}
		}
	}
	MTWF_PRINT("%s: Enable CR for RX_BLOCKER: fgFixWbIBRssiEn: %d\n", __func__,
			pAd->fgFixWbIBRssiEn);
	return TRUE;
}
/* Set RSSI threshold */
INT SetRssiThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->c1RWbRssiHTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 1:
			pAd->c1RWbRssiLTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 2:
			pAd->c1RIbRssiLTh = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 3:
			pAd->c1WBRssiTh4R = simple_strtol(value, 0, 10);
			break;

		default: {
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
		}
	}

	MTWF_PRINT("%s: c1RWbRssiHTh: %d, c1RWbRssiLTh: %d, c1RIbRssiLTh: %d,c1WBRssiTh4R: %d\n", __FUNCTION__,
			pAd->c1RWbRssiHTh, pAd->c1RWbRssiLTh, pAd->c1RIbRssiLTh, pAd->c1WBRssiTh4R);

	return TRUE;
}

/* Set fgAdaptRxBlock */
INT SetAdaptRxBlockCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->fgAdaptRxBlock = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
		}
	}

	MTWF_PRINT("%s: fgAdaptRxBlock: %d\n", __func__,
			pAd->fgAdaptRxBlock);

	return TRUE;
}

/* Set check counter threshold */
INT SetCheckThCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 11) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 2 symbol representation */
		case 0:
			pAd->u1CheckTime = simple_strtol(value, 0, 10);
			break;
			/* 2 symbol representation */
		case 1:
			pAd->u1To1RCheckCnt = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 2:
			pAd->u2To1RvaildCntTH = simple_strtol(value, 0, 10);
			break;
		case 3:
			pAd->u2To4RvaildCntTH = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
		}
	}

	MTWF_PRINT("%s: u1CheckTime: %d, u1To1RCheckCnt: %d, u2To1RvaildCntTH: %d, u2To4RvaildCntTH: %d\n", __func__,
			pAd->u1CheckTime, pAd->u1To1RCheckCnt, pAd->u2To1RvaildCntTH, pAd->u2To4RvaildCntTH);
	return TRUE;

}
/* Set WBRSSI CR */
INT SetWbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->c1WbRssiWF0 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 1:
			pAd->c1WbRssiWF1 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 2:
			pAd->c1WbRssiWF2 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 3:
			pAd->c1WbRssiWF3 = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
		}
	}
	MTWF_PRINT("--------------------------------------------------------------\n");
	MTWF_PRINT("%s: c1WbRssiWF0: %d, c1WbRssiWF1: %d, c1WbRssiWF2: %d, c1WbRssiWF3: %d\n", __func__, pAd->c1WbRssiWF0, pAd->c1WbRssiWF1, pAd->c1WbRssiWF2, pAd->c1WbRssiWF3);
	MTWF_PRINT("--------------------------------------------------------------\n");
	return TRUE;
}

/* Set IBRSSI CR */
INT SetIbRssiDirectCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8  i;
	CHAR   *value;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 15) {
		MTWF_PRINT("Wrong parameter format!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
			/* 3 symbol representation */
		case 0:
			pAd->c1IbRssiWF0 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 1:
			pAd->c1IbRssiWF1 = simple_strtol(value, 0, 10);
			break;
		case 2:
			pAd->c1IbRssiWF2 = simple_strtol(value, 0, 10);
			break;
			/* 3 symbol representation */
		case 3:
			pAd->c1IbRssiWF3 = simple_strtol(value, 0, 10);
			break;
		default: {
			MTWF_PRINT("Number of parameters exceed expectation !!\n");
			return FALSE;
		}
		}
	}
	MTWF_PRINT("--------------------------------------------------------------\n");
	MTWF_PRINT("%s: c1IbRssiWF0: %d, c1IbRssiWF1: %d, c1WbRssiWF2: %d, c1IbRssiWF3: %d\n", __func__,
			pAd->c1IbRssiWF0, pAd->c1IbRssiWF1, pAd->c1IbRssiWF2, pAd->c1IbRssiWF3);
	MTWF_PRINT("--------------------------------------------------------------\n");
	return TRUE;
}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */

INT SetMuTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	  status = TRUE;
	UINT8	i;
	CHAR	 *value = 0;
	BOOLEAN  fgMuTxPwrManEn = FALSE;
	CHAR     cMuTxPwr = 0;
	UINT8    u1BandIdx = 0;
	struct  wifi_dev *wdev;
#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_AP_SUPPORT)
	UCHAR	  apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[0].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, u1BandIdx);

	/* sanity check for Band index */
	if (u1BandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgMuTxPwrManEn = os_str_tol(value, 0, 10);
			break;

		case 1:
			cMuTxPwr = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: fgMuTxPwrManEn: %d, cMuTxPwr: %d\n", __func__,
			fgMuTxPwrManEn, cMuTxPwr);

	return MuPwrCtrlCmd(pAd, fgMuTxPwrManEn, cMuTxPwr, u1BandIdx);
}

#ifdef DATA_TXPWR_CTRL

UINT8 GetMaxBasePwr(RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	UINT8 ret = 0, MaxBasepwr = 0;

	if (BandIdx >= MAX_BAND_NUM)
		return FALSE;

	if (pAd->ApCfg.MaxBaseTxPwr == 0) {
		/* Get max base power (per band) */
		pAd->ApCfg.data_pwr_cmd_flag = TRUE;
#ifdef MGMT_TXPWR_CTRL
		g_fgCommand = TRUE;
#endif
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, BandIdx);
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.get_tx_pwr_aync_done, ((50 * 100 * OS_HZ) / 1000));/*Wait 5s.*/
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"wait feedback tx base pwr success.\n");
		else {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"wait feedback tx base pwr timeout.\n");
			return FALSE;
		}
		pAd->ApCfg.data_pwr_cmd_flag = FALSE;
		MaxBasepwr = pAd->ApCfg.MaxBaseTxPwr;
	} else {
		MaxBasepwr = pAd->ApCfg.MaxBaseTxPwr;
	}

	return MaxBasepwr;
}

INT SetTxPwrLimitDataFrame(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4MinPwrlimit, u4recv = 0;
	UINT8 BandIdx, u1MaxBasepwr, ret;
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return TRUE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!wdev)
		return TRUE;

	BandIdx = HcGetBandByWdev(wdev);

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return TRUE;
	}

	u4recv = sscanf(arg, "%d", &(u4MinPwrlimit));

	if (u4recv != 1) {
		MTWF_PRINT("Format Error! Please enter in the following format\n"
				"<PowerLimit>(in dbm)\n");
		return TRUE;
	}

	u1MaxBasepwr = GetMaxBasePwr(pAd, BandIdx);
	u4MinPwrlimit = u4MinPwrlimit * 2;

	if (u4MinPwrlimit > u1MaxBasepwr) {
		MTWF_PRINT("%s: Minimum power %d > Max power %d\n", __func__,
		(u4MinPwrlimit / 2), (u1MaxBasepwr / 2));
		return TRUE;
	}

	pAd->ApCfg.MinBaseTxPwr = (UINT8)(u4MinPwrlimit);
	ret = TxPwrMinLimitDataFrameCtrl(pAd, (INT8)pAd->ApCfg.MinBaseTxPwr, BandIdx);

	if (ret)
		MTWF_PRINT("%s: Minimum power %d\n", __func__, u4MinPwrlimit);
	else
		MTWF_PRINT("%s: Unable to set minimum power.\n", __func__);

	return ret;
}

INT SetTxPwrDataFrame(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4WlanIdx = 0, u4recv = 0;
	INT32 i4Offset = 0;
	UINT8 BandIdx;
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct wifi_dev *wdev;
	POS_COOKIE pObj;
	UCHAR apidx;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return TRUE;
	}

	u4recv = sscanf(arg, "%u-%d", &(u4WlanIdx), &(i4Offset));
	MTWF_PRINT("%s u4WlanIdx:%u i4Offset:%d", __func__, u4WlanIdx, i4Offset);

	if (u4recv != 2) {
		MTWF_PRINT("Format Error! Please enter in the following format\n"
				"<wcid>-<offset>(in dbm)\n");
		return TRUE;
	}

	if (u4WlanIdx >= WTBL_MAX_NUM(pAd)) {
		MTWF_PRINT("Invalid WlanIdx %d\n", u4WlanIdx);
		return TRUE;
	}

	pEntry = entry_get(pAd, u4WlanIdx);

	if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC)) {
		MTWF_PRINT("%s(): sta%d is leaveing\n", __func__, u4WlanIdx);
		return TRUE;
	}

	pAd = (RTMP_ADAPTER *)pEntry->pAd;
	if (!pAd)
		return TRUE;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return TRUE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!wdev)
		return TRUE;

	BandIdx = HcGetBandByWdev(wdev);

	MTWF_PRINT("%s: wlanIdx: %d, offset: %d dBm\n", __func__, u4WlanIdx, i4Offset);

	return update_data_frame_power(pAd, wdev, pEntry, i4Offset);
}

INT update_data_frame_power(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, MAC_TABLE_ENTRY *pEntry, INT32 i4Offset)
{
	INT32 MaxBasePwr = 0;
	UINT16 ret = TRUE;
	UINT8 u1BwIdx = 0, u1McsIdx = 0, BandIdx = 0, u1MinPwrLimit = 0;
	INT8 i1PowerOffset = 0;

	if (!wdev)
		return ret;

	BandIdx = HcGetBandByWdev(wdev);

	/* Get max base power (per band) */
	pAd->ApCfg.data_pwr_cmd_flag = TRUE;
#ifdef MGMT_TXPWR_CTRL
	g_fgCommand = TRUE;
#endif
	MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, BandIdx);
	ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.get_tx_pwr_aync_done, ((50 * 100 * OS_HZ) / 1000));/*Wait 5s.*/
	if (ret)
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"wait feedback tx base pwr success.\n");
	else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"wait feedback tx base pwr timeout.\n");
		return TRUE;
	}
	pAd->ApCfg.data_pwr_cmd_flag = FALSE;

	MaxBasePwr = pAd->ApCfg.MaxBaseTxPwr / 2;

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"band%d max base tx pwr is %d dBm\n", BandIdx, MaxBasePwr);

	if (pEntry) {
		i1PowerOffset = (INT8)(i4Offset * 2);
		u1MinPwrLimit = pAd->ApCfg.MinBaseTxPwr;

		for (u1BwIdx = 0; u1BwIdx < DATA_TXPOWER_MAX_BW_NUM; u1BwIdx++) {
			for (u1McsIdx = 0; u1McsIdx < DATA_TXPOWER_MAX_MCS_NUM; u1McsIdx++) {
				if ((g_u1MinPwrlimitperRate[BandIdx][u1BwIdx][u1McsIdx] + i1PowerOffset) < u1MinPwrLimit) {
					MTWF_PRINT("Fail: limitallrate: %d offset %d,cal_power lower than MinPwrlimit(%d)\n",
						g_u1MinPwrlimitperRate[BandIdx][u1BwIdx][u1McsIdx] / 2, i1PowerOffset / 2, u1MinPwrLimit / 2);
					return TRUE;
				}
			}
		}

		pEntry->PowerOffset = (INT8)i4Offset;

		ret = TxPwrDataFrameCtrlCmd(pAd, pEntry, MaxBasePwr, BandIdx);

		if (i4Offset != 0)
			pEntry->DataTxPwrEn = TRUE;
		else
			pEntry->DataTxPwrEn = FALSE;
	}

	return ret;
}


INT show_data_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4recv = 0, u4value = 0, u4Wcid = 0;
	INT32 MaxBasePwr = 0;
	UINT16  BandIdx, ret = 0, i = 0, u2StaCount = 0;
	struct wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PMAC_TABLE_ENTRY pEntry = NULL;

	if (apidx >= pAd->ApCfg.BssidNum)
		return TRUE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!wdev)
		return ret;

	BandIdx = HcGetBandByWdev(wdev);

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return TRUE;
	}

	u4recv = sscanf(arg, "%u-%u", &(u4value), &(u4Wcid));

	if (u4recv != 2) {
		MTWF_PRINT("Format Error! Please enter in the following format\n"
				"<config_value> - <wcid>)\n");
		return TRUE;
	}

	if (u4value == 0) {
		pAd->ApCfg.data_pwr_cmd_flag = TRUE;
#ifdef MGMT_TXPWR_CTRL
		g_fgCommand = TRUE;
#endif
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, BandIdx);
		ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->ApCfg.get_tx_pwr_aync_done, ((50 * 100 * OS_HZ) / 1000));/*Wait 5s.*/
		if (ret)
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"wait feedback tx base pwr success.\n");
		else {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"wait feedback tx base pwr timeout.\n");
			return TRUE;
		}
		pAd->ApCfg.data_pwr_cmd_flag = FALSE;
		MaxBasePwr = pAd->ApCfg.MaxBaseTxPwr / 2;

		if (pAd->ApCfg.MaxBaseTxPwr % 2 == 0)
			MTWF_PRINT("=== BandIdx: %d, Max Base Pwr: %d dbm ===\n", BandIdx, MaxBasePwr);
		else
			MTWF_PRINT("=== BandIdx: %d, Max Base Pwr: %d.5 dbm ===\n", BandIdx, MaxBasePwr);

		if (pAd->ApCfg.MinBaseTxPwr % 2 == 0)
			MTWF_PRINT("=== BandIdx: %d, Min Base Pwr: %d dbm ===\n", BandIdx, (pAd->ApCfg.MinBaseTxPwr / 2));
		else
			MTWF_PRINT("=== BandIdx: %d, Min Base Pwr: %d.5 dbm ===\n", BandIdx, (pAd->ApCfg.MinBaseTxPwr / 2));
	}

	if (u4value == 1) {
		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = entry_get(pAd, i);

			if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC) || !(pEntry->DataTxPwrEn))
				continue;

			MTWF_PRINT("%s:: wlanIdx: %d\n", __func__, i);
			u2StaCount++;
		}

		MTWF_PRINT("Data pwr is set for %d stations\n", u2StaCount);
	}

	if (u4value == 2) {
		pEntry = entry_get(pAd, u4Wcid);

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC)) {
			MTWF_PRINT("Invalid pEntry\n");
			return TRUE;
		}

		MTWF_PRINT("Wcid: %d, PowerOffset:%d\n", u4Wcid, pEntry->PowerOffset);
	}

	return TRUE;
}


#endif


INT SetTxPowerCompInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	 status = TRUE;
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	UINT8  ucPowerTableIdx;
	CHAR   *STRING[SKU_TABLE_SIZE] = {
		"CCK_1M2M   ", "CCK5M11M   ", "OFDM6M9M   ", "OFDM12M18M ",	"OFDM24M36M ", "OFDM48M	", "OFDM54M	",
		"HT20M0	 ", "HT20M32	", "HT20M1M2   ", "HT20M3M4   ", "HT20M5	 ", "HT20M6	 ", "HT20M7	 ",
		"HT40M0	 ", "HT40M32	", "HT40M1M2   ", "HT40M3M4   ",	"HT40M5	 ", "HT40M6	 ", "HT40M7	 ",
		"VHT20M0	", "VHT20M1M2  ", "VHT20M1M2  ", "VHT20M5M6  ", "VHT20M7	", "VHT20M8	", "VHT20M9	",
		"VHT40M0	", "VHT40M1M2  ", "VHT40M3M4  ", "VHT40M5M6  ", "VHT40M7	", "VHT40M8	", "VHT40M9	",
		"VHT80M0	", "VHT80M1M2  ", "VHT80M3M4  ", "VHT80M5M6  ", "VHT80M7	", "VHT80M8	", "VHT80M9	",
		"VHT160M0   ", "VHT160M1M2 ", "VHT160M3M4 ", "VHT160M5M6 ", "VHT160M7   ", "VHT160M8   ", "VHT160M9   "
	};

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev)
		ucBandIdx = HcGetBandByWdev(wdev);
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, ucBandIdx);

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;

	MTWF_PRINT("=============================================================================\n");
	MTWF_PRINT("                       Tx Power Compenstation Info                 \n");

	MTWF_PRINT("=============================================================================\n");
	MTWF_PRINT(KGRN "         Band%d        (1SS, 2SS, 3SS, 4SS)                    \n" KNRM, ucBandIdx);

	for (ucPowerTableIdx = 0; ucPowerTableIdx < SKU_TABLE_SIZE; ucPowerTableIdx++) {
		MTWF_PRINT("         %s : %3d, %3d, %3d, %3d                               \n",
			  &STRING[ucPowerTableIdx][0],
			  pAd->CommonCfg.cTxPowerCompBackup[ucPowerTableIdx][0],
			  pAd->CommonCfg.cTxPowerCompBackup[ucPowerTableIdx][1],
			  pAd->CommonCfg.cTxPowerCompBackup[ucPowerTableIdx][2],
			  pAd->CommonCfg.cTxPowerCompBackup[ucPowerTableIdx][3]);
	}

	MTWF_PRINT("-----------------------------------------------------------------------------\n");

	return status;
}

/* Manually setting Tx power on a,g,n*/
INT SetTxPwrManualCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CHAR		*value = 0;
	UCHAR		ucBandIdx = 0;
	struct		wifi_dev *wdev;
	INT32		Ret;
	UINT8		ParamIdx;
	INT_8		cTxPower = 0;
	UINT8		ucPhyMode = 0;
	UINT8		ucTxRate = 0;
	UINT8		ucBW = 0;

	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	/* obtain Band index */
	if (wdev)
		ucBandIdx = HcGetBandByWdev(wdev);
	if (!arg) {
		MTWF_PRINT("No Parameters !!\n");
		goto err1;
	}

	/* Sanity check for input parameter format */
	if (strlen(arg) != 11) {
		MTWF_PRINT("Wrong Parameter Format !!\n");
		goto err1;
	}

	/* Parsing input parameter */
	for (ParamIdx = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), ParamIdx++) {
		switch (ParamIdx) {
		case 0:
			ucPhyMode = simple_strtol(value, 0, 10); /* 2-bit format */
			break;
		case 1:
			ucTxRate = simple_strtol(value, 0, 10);  /* 2-bit format */
			break;
		case 2:
			ucBW = simple_strtol(value, 0, 10);      /* 2-bit format */
			break;
		case 3:
			cTxPower = simple_strtol(value, 0, 10);  /* 2-bit format */
			break;
		default:
			MTWF_PRINT("Set Too Much Parameters !!\n");
			goto err1;
		}
	}

	MTWF_PRINT("%s: Band(%d), TxMode(%d), MCS(%d), BW(%d), TxPower(%d)\n",
		__func__, ucBandIdx, ucPhyMode, ucTxRate, ucBW, cTxPower);

	/* Command Handler for Force Power Control */
	Ret = TxPowerManualCtrl(pAd, ucBandIdx, cTxPower, ucPhyMode, ucTxRate, ucBW);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
err1:
	MTWF_PRINT("Please input parameter via format \"Phymode:TxRate:BW:TxPower\"\n" KNRM);
	MTWF_PRINT("Phymode:\n" KNRM);
	MTWF_PRINT("    (2-digit) 0: CCK, 1: OFDM, 2: HT, 3: VHT, 4: HESU\n" KNRM);
	MTWF_PRINT("TxRate:\n" KNRM);
	MTWF_PRINT(
	"    (2-digit) CCK: 00~03, OFDM: 00~07, HT: 00~07, VHT: 00~09, HESU:00~11\n" KNRM);
	MTWF_PRINT("BW:\n" KNRM);
	MTWF_PRINT("    (2-digit) 0: BW20, 1: BW40, 2: BW80, 3:BW160\n" KNRM);
	MTWF_PRINT(
	"    (2-digit) 0: HE26, 1: HE52, 2: HE106, 3:HE242, 4:HE484, 5:HE996, 6:HE996X2\n" KNRM);
	MTWF_PRINT("TxPower:\n" KNRM);
	MTWF_PRINT("    (2-digit) absolute Tx power (unit: 0.5dB)\n" KNRM);
	MTWF_PRINT("Ex: iwpriv ra0 set TxPwrManualSet=02:00:00:16\n" KNRM);
	return FALSE;
}

INT SetTpcManCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN fgTpcManual = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTpcManual = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_PRINT("set wrong parameters\n");
			return FALSE;
		}
		}
	}
	return TpcManCtrl(pAd, fgTpcManual);
}

INT SetTpcEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN fgTpcEnable = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTpcEnable = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_PRINT("set wrong parameters\n");
			return FALSE;
		}
		}
	}
	return TpcEnableCfg(pAd, fgTpcEnable);
}

INT SetTpcWlanIdCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN	fgUplink = FALSE;
	UINT8	u1EntryIdx = 0;
	UINT16	u2WlanId = 0;
	UINT8	u1DlTxType = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgUplink = (BOOLEAN)os_str_tol(value, 0, 10);
			break;
		case 1:
			u1EntryIdx = os_str_tol(value, 0, 10);
			if (u1EntryIdx > 31) {
				MTWF_PRINT(
				"Set wrong parameters! Entry index should be between 0~31 .\n");
				return FALSE;
			}
			break;
		case 2:
			u2WlanId = os_str_tol(value, 0, 10);
			break;
		case 3:
			u1DlTxType = os_str_tol(value, 0, 10);
			if (u1DlTxType != 0 && u1DlTxType != 1) {
				MTWF_PRINT(
				"Set wrong parameters! Down-link Tx Type 0: MU-MIMO, 1: OFDMA. \n");
				return FALSE;
			}
			break;
		default: {
			MTWF_PRINT("set wrong parameters\n");
			return FALSE;
		}
		}
	}
	return TpcWlanIdCtrl(pAd, fgUplink, u1EntryIdx, u2WlanId, u1DlTxType);
}

INT SetTpcUlAlgoCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	UINT8	u1para[4] = {0};
	CHAR	*value = NULL;
	UINT8	u1TpcCmd = 0;
	UINT8	u1ApTxPwr = 0;
	UINT8	u1EntryIdx = 0;
	UINT8	u1TargetRssi = 0;
	UINT8	u1UPH = 0;
	BOOLEAN	fgMinPwrFlag = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(u1para)); value = rstrtok(NULL, ":"), i++)
		u1para[i] = os_str_tol(value, 0, 10);

	u1TpcCmd = u1para[0];
	switch (u1TpcCmd) {
	case 0:
		if (u1para[1] > 60) {
			MTWF_PRINT(
				"Set wrong parameters! Ap Tx Power should be between 0~60.\n");
			return FALSE;
		}
		u1ApTxPwr = u1para[1];
		break;
	case 1:
		u1EntryIdx = u1para[1];
		if (u1EntryIdx > 31) {
			MTWF_PRINT("Set wrong parameters! Entry index should be between 0~31 .\n");
			return FALSE;
		}
		u1TargetRssi = u1para[2];
		if (u1TargetRssi > 90) {
			MTWF_PRINT("Set wrong parameters! Target TSSI be between 0~90.\n");
			return FALSE;
		}
		break;
	case 2:
		u1EntryIdx = u1para[1];
		if (u1EntryIdx > 31) {
			MTWF_PRINT("Set wrong parameters! Entry index should be between 0~31 .\n");
			return FALSE;
		}
		u1UPH = u1para[2];
		if (u1UPH > 31) {
			MTWF_PRINT(
			"Set wrong parameters! Uplink Power Headroom should be between 0~31.\n");
			return FALSE;
		}
		fgMinPwrFlag = u1para[3];
		break;
	default: {
		MTWF_PRINT("set wrong parameters\n");
		return FALSE;
	}
	}
	return TpcUlAlgoCtrl(pAd, u1TpcCmd, u1ApTxPwr, u1EntryIdx, u1TargetRssi, u1UPH, fgMinPwrFlag);
}

INT SetTpcDlAlgoCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8   i;
	INT16	para[4];
	CHAR	*value = NULL;
	UINT8	u1TpcCmd = 0;
	BOOLEAN fgCmdCtrl = FALSE;
	UINT8	u1DlTxType = 0;
	CHAR	DlTxPwr = 0;
	UINT8	u1EntryIdx = 0;
	INT16	DlTxpwrAlpha = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(para)); value = rstrtok(NULL, ":"), i++)
		para[i] = os_str_tol(value, 0, 10);

	u1TpcCmd = (UINT8)para[0];
	switch (u1TpcCmd) {
	case 0:
		if (para[1] != 0 && para[1] != 1) {
			MTWF_PRINT(
			"Set wrong parameters! Enable command control enable=1, disable=0.\n");
			return FALSE;
		}
		fgCmdCtrl = (BOOLEAN)para[1];
		break;
	case 1:
		if (para[1] != 0 && para[1] != 1) {
			MTWF_PRINT(
				"Set wrong parameters! Down-link Tx Type 0: MU-MIMO, 1: OFDMA.\n");
			return FALSE;
		}
		u1DlTxType = (UINT8)para[1];
		if (para[2] < -128 || para[2] > 127) {
			MTWF_PRINT(
			"Set wrong parameters! Down-link Tx Power should between -127~128.\n");
			return FALSE;
		}
		DlTxPwr = (CHAR)para[2];
		break;
	case 2:
		if (para[1] < 0 || para[1] > 31) {
			MTWF_PRINT("Set wrong parameters! Entry index should be between 0~31 .\n");
			return FALSE;
		}
		u1EntryIdx = (UINT8)para[1];
		if (para[2] != 0 && para[2] != 1) {
			MTWF_PRINT("wrong parameters! Down-link Tx Type 0: MU-MIMO, 1: OFDMA.\n");
			return FALSE;
		}
		u1DlTxType = (UINT8)para[2];
		if (para[3] < -256 || para[3] > 255) {
			MTWF_PRINT(
			"wrong parameters! Down-link Tx Alpha Power should between -256~255.\n");
			return FALSE;
		}
		DlTxpwrAlpha = para[3];
		break;
	default: {
		MTWF_PRINT("set wrong parameters\n");
		return FALSE;
	}
	}
	return TpcDlAlgoCtrl(pAd, u1TpcCmd, fgCmdCtrl, u1DlTxType, DlTxPwr, u1EntryIdx, DlTxpwrAlpha);
}

INT SetTpcManTblInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN	fgUplink = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgUplink = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_PRINT("set wrong parameters\n");
			return FALSE;
		}
		}
	}
	return TpcManTblInfo(pAd, fgUplink);
}

INT SetTpcAlgoUlUtCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	INT16	i2para[4] = {0};
	CHAR	*value = NULL;
	UINT8	u1EntryIdx = 0;
	UINT8	u1VarType = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value && (i < ARRAY_SIZE(i2para)); value = rstrtok(NULL, ":"), i++)
		i2para[i] = os_str_tol(value, 0, 10);

	u1EntryIdx = (UINT8)i2para[0];
	u1VarType = (UINT8)i2para[1];

	if (u1EntryIdx > 31) {
		MTWF_PRINT("Set wrong parameters! Entry index should be between 0~31 .\n");
		return FALSE;
	}
	return TpcUlUtVarCfg(pAd, u1EntryIdx, u1VarType, i2para[2]);
}

INT SetTpcAlgoUlUtGo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = NULL;
	BOOLEAN fgTpcUtGo = FALSE;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}
	/*  Note:  u1VarType:
	ENUM {
	(i2) LastRssi_dBm = 0,
	(i2) LastPowerHeadroom_dB  = 1,
	(fg) MinTxPwr =2,
	(fg) HaveHitMinTxPwrFg  = 3,
	(i2) MinRssi_dBm =4,
	(i2) RssiOffset_dB = 5,
	(u2) LastRuAllocIdx =6,
	(u1) LastTxMcs =7,
	(u2) CurrRuAllocIdx =8,
	(u1) CurrTxMcs =9,
	(u1) TotTrigUser =10 } */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			fgTpcUtGo = (BOOLEAN)os_str_tol(value, 0, 10);
			break;

		default: {
			MTWF_PRINT("set wrong parameters\n");
			return FALSE;
		}
		}
	}
	return TpcAlgoUtGo(pAd, fgTpcUtGo);
}

INT SetThermalManCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 u1BandIdx = 0;
	BOOLEAN fgManualMode = FALSE;
	UINT8 u1ThermalAdc = 0;
	INT i4status;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 7) {
		MTWF_PRINT("Wrong parameter format !!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":"); pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			u1BandIdx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			fgManualMode = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 3 symbol representation */
			u1ThermalAdc = os_str_tol(pi1Buffer, 0, 10);
			break;

		default: {
			MTWF_PRINT("Wrong parameters !!\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s(): u1BandIdx: %d, fgManualMode: %d, u1ThermalAdc: %d\n", __func__, u1BandIdx, fgManualMode, u1ThermalAdc);

	/* handler */
	i4status = ThermalManCtrl(pAd, u1BandIdx, fgManualMode, u1ThermalAdc);

	return i4status;
}

INT SetThermalTaskInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i4Status;
	UINT8 u1BandIdx = 0;
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;

		/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format !!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":"); pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			u1BandIdx = os_str_tol(pi1Buffer, 0, 10);
			break;

		default: {
			MTWF_PRINT("Wrong parameters !!\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s(): u1BandIdx: %d\n", __func__, u1BandIdx);

	/* handler */
	i4Status = ThermalBasicInfo(pAd, u1BandIdx);

	return i4Status;
}

INT SetThermalTaskCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 u1BandIdx = 0;
	BOOLEAN fgTrigEn = FALSE;
	UINT8 u1Thres = 0;
	INT i4status;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* sanity check for input parameter format */
	if (strlen(arg) != 7) {
		MTWF_PRINT("Wrong parameter format !!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":"); pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			u1BandIdx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 3 symbol representation */
			u1Thres = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			fgTrigEn = os_str_tol(pi1Buffer, 0, 10);
			break;

		default: {
			MTWF_PRINT("Wrong parameters !!\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s(): u1BandIdx: %d, fgTrigEn: %d, u1Thres: %d\n", __func__, u1BandIdx, fgTrigEn, u1Thres);

	/* handler */
	i4status = ThermalTaskCtrl(pAd, u1BandIdx, fgTrigEn, u1Thres);

	return i4status;
}

INT SetThermalProtectEnable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, protection_type = 0, trigger_type = 0;
	INT32 trigger_temp = 0, restore_temp = 0;
	UINT16 recheck_time = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			protection_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			trigger_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 3:
			/* 3 symbol representation */
			trigger_temp = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 4:
			/* 3 symbol representation */
			restore_temp = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 5:
			/* 4 symbol representation */
			recheck_time = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_PRINT("wrong param format.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d, protection_type: %d, trigger_type: %d\n",
		__func__, band_idx, protection_type, trigger_type);

	MTWF_PRINT("%s(): trigger_temp: %d, restore_temp: %d, recheck_time: %d\n",
		__func__, trigger_temp, restore_temp, recheck_time);

	/* handler */
	MtCmdThermalProtectEnable(pAd, band_idx, protection_type,
		trigger_type, trigger_temp, restore_temp, recheck_time);

	return TRUE;
}

INT SetThermalProtectDisable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, protection_type = 0, trigger_type = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			protection_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			trigger_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_PRINT("wrong param format.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d, protection_type: %d, trigger_type: %d\n",
		__func__, band_idx, protection_type, trigger_type);

	/* handler */
	MtCmdThermalProtectDisable(pAd, band_idx,
		protection_type, trigger_type);

	return TRUE;
}

INT SetThermalProtectDutyCfg(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, level_idx = 0,  duty = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			level_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			duty = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_PRINT("wrong param format.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d, level_idx: %d, duty: %d\n",
		__func__, band_idx, level_idx, duty);

	/* handler */
	MtCmdThermalProtectDutyCfg(pAd, band_idx, level_idx, duty);

	return TRUE;
}

INT SetThermalProtectInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0;
	struct THERMAL_PROTECT_MECH_INFO info_buf;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_PRINT("wrong param format.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d\n",
		__func__, band_idx);

	/* handler */
	MtCmdThermalProtectInfo(pAd, band_idx, &info_buf);

	return TRUE;
}

INT SetThermalProtectDutyInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0;
	struct THERMAL_PROTECT_DUTY_INFO info_buf;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		default:
			MTWF_PRINT("wrong param format.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d\n",
		__func__, band_idx);

	/* handler */
	MtCmdThermalProtectDutyInfo(pAd, band_idx, &info_buf);

	return TRUE;
}

INT SetThermalProtectStateAct(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 u1ParamIdx;
	PCHAR pi1Buffer = NULL;
	UINT8 band_idx = 0, protect_type = 0;
	UINT8 trig_type = 0, state = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (u1ParamIdx = 0, pi1Buffer = rstrtok(arg, ":");
		pi1Buffer; pi1Buffer = rstrtok(NULL, ":"), u1ParamIdx++) {
		switch (u1ParamIdx) {
		case 0:
			/* 1 symbol representation */
			band_idx = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 1:
			/* 1 symbol representation */
			protect_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 2:
			/* 1 symbol representation */
			trig_type = os_str_tol(pi1Buffer, 0, 10);
			break;

		case 3:
			/* 1 symbol representation */
			state = os_str_tol(pi1Buffer, 0, 10);
			break;
		default:
			MTWF_PRINT("wrong param format.\n");
			return FALSE;
		}
	}

	MTWF_PRINT("%s(): band_idx: %d, protect_type: %d\n",
		__func__, band_idx, protect_type);

	MTWF_PRINT("%s(): trig_type: %d, state: %d\n",
		__func__, trig_type, state);

	/* handler */
	MtCmdThermalProtectStateAct(pAd, band_idx,
		protect_type, trig_type, state);

	return TRUE;
}

#ifdef TX_POWER_CONTROL_SUPPORT
static INT SetTxPowerBoostCtrl_V0V1(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	UINT8	u1PwrUpCat = 0, u1LenVal, u1PwrBoost;
	CHAR	cPwrUpValue[POWER_UP_CATEGORY_RATE_NUM] = {0};
	UINT8	ucBandIdx = 0;
	UINT8	u1Bw, u1PhyMode;

	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter band index\n");
		return -1;
	}
	ucBandIdx = simple_strtol(value, 0, 10);

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("set wrong parameters: Band Index\n");
		return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter PhyMode\n");
		return -1;
	}
	u1PhyMode = simple_strtol(value, 0, 10);

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter bandwidth\n");
		return -1;
	}
	u1Bw = simple_strtol(value, 0, 10);

	if (arch_ops && arch_ops->arch_txpower_boost_power_cat_type) {
		if (!arch_ops->arch_txpower_boost_power_cat_type(pAd, u1PhyMode, u1Bw, &u1PwrUpCat))
			return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter length of power boost values\n");
		return -1;
	}
	u1LenVal  = simple_strtol(value, 0, 10);

	for (i = 0; i < u1LenVal; i++) {
		if ((i + 1) == u1LenVal)
			value = strsep(&arg, "");
		else
			value = strsep(&arg, ":");

		if (value == NULL) {
			MTWF_PRINT("set wrong parameters: Please enter Power boost value\n");
			return -1;
		}

		u1PwrBoost = simple_strtol(value, 0, 10);

		if (i < cap->single_sku_fill_tbl_length[u1PwrUpCat])
			cPwrUpValue[i] = u1PwrBoost;
		else {
			MTWF_PRINT(
			"wrong value: Power boost value should not be larger than Max value %d\n",
			cap->single_sku_fill_tbl_length[u1PwrUpCat]);
			return -1;
		}

	}

	if (arch_ops && arch_ops->arch_txpower_boost_ctrl)
		arch_ops->arch_txpower_boost_ctrl(pAd, u1PwrUpCat, cPwrUpValue);

	MTWF_PRINT("%s: cPwrUpValue: (%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)-(%d)\n",
			 __func__, cPwrUpValue[0], cPwrUpValue[1],
			 cPwrUpValue[2], cPwrUpValue[3], cPwrUpValue[4],
			 cPwrUpValue[5], cPwrUpValue[6], cPwrUpValue[7],
			 cPwrUpValue[8], cPwrUpValue[9], cPwrUpValue[10],
			 cPwrUpValue[11]);

	return TxPwrUpCtrl(pAd, ucBandIdx, u1PwrUpCat, &cPwrUpValue[0], POWER_UP_CATEGORY_RATE_NUM);
}
#endif /* TX_POWER_CONTROL_SUPPORT */

#ifdef TX_POWER_CONTROL_SUPPORT_V2
static INT SetTxPowerBoostCtrl_V2(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	*value = 0;
	UINT8	u1PwrUpCat = 0, u1PwrBoost;
	CHAR	*cPwrUpValue = NULL;
	UINT8	ucBandIdx = 0;
	UINT8	u1PhyMode;
	struct POWER_BOOST_PARA_V2 *pwrBst = pAd->CommonCfg.PowerBoostParamV2;

	RTMP_ARCH_OP *arch_ops = hc_get_arch_ops(pAd->hdev_ctrl);

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	ucBandIdx = hc_get_hw_band_idx(pAd);

	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("set wrong parameters: Band Index\n");
		return FALSE;
	}

	value = strsep(&arg, ":");
	if (value == NULL) {
		MTWF_PRINT("set wrong parameters: Please enter PowerCategory\n");
		return -1;
	}
	u1PhyMode = u1PwrUpCat = os_str_tol(value, 0, 10);

	if (u1PwrUpCat >= POWER_UP_CATE_V2_NUM) {
		MTWF_PRINT("set wrong parameters: PowerCategory index:%d\n", u1PwrUpCat);
		return -1;
	}

	for (i = 0; i < pwrBst[u1PwrUpCat].catSize; i++) {
		value = strsep(&arg, ":");
		if (value == NULL) {
			MTWF_PRINT("notice: \033[1;33m%s\x1b[m-input mcs cnt:%d, max mcs cnt:%d\n",
				gPwrBstString[u1PwrUpCat], i, pwrBst[u1PwrUpCat].catSize);
			break;
		}

		u1PwrBoost = os_str_tol(value, 0, 10);
		pwrBst[u1PwrUpCat].pwrUpCatRateVal[i] = u1PwrBoost;
	}

	cPwrUpValue = pwrBst[u1PwrUpCat].pwrUpCatRateVal;

	MTWF_PRINT("%s:\n\033[1;33m%s\x1b[m idx:%d  ", __func__,
		gPwrBstString[u1PwrUpCat], u1PwrUpCat);
	for (i = 0; i < pwrBst[u1PwrUpCat].catSize; i++)
		MTWF_PRINT("M%d(%d)  ", i, pwrBst[u1PwrUpCat].pwrUpCatRateVal[i]);
	MTWF_PRINT("\n");

	if (arch_ops && arch_ops->arch_txpower_boost_ctrl)
		arch_ops->arch_txpower_boost_ctrl(pAd, u1PwrUpCat, cPwrUpValue);

	return TxPwrUpCtrl(pAd, ucBandIdx, u1PwrUpCat, &cPwrUpValue[0],
			POWER_UP_CATEGORY_RATE_NUM);
}
#endif /* TX_POWER_CONTROL_SUPPORT_V2 */

#if defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2)
INT SetTxPowerBoostCtrl(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
#ifdef TX_POWER_CONTROL_SUPPORT_V2
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->tx_power_boost_ver == 2) {
		SetTxPowerBoostCtrl_V2(pAd, arg);
		return TRUE;
	}
#endif

#ifdef TX_POWER_CONTROL_SUPPORT
	SetTxPowerBoostCtrl_V0V1(pAd, arg);
#endif
	return TRUE;
}
#endif /* defined(TX_POWER_CONTROL_SUPPORT) || defined(TX_POWER_CONTROL_SUPPORT_V2) */


INT SetCalFreeApply(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	 status = TRUE;
	UINT8   i;
	CHAR	*value = 0;
	UCHAR   CalFreeApply = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	if (strlen(arg) != 1) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like X (X = 0,1)!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			CalFreeApply = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	MTWF_PRINT("%s: CalFreeApply = %d\n", __func__, CalFreeApply);
	/* Configure to Global pAd structure */
	pAd->fgCalFreeApply = CalFreeApply;
	return status;
}

INT SetWriteEffuseRFpara(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT status = TRUE;
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	USHORT  length = cap->EEPROM_DEFAULT_BIN_SIZE;
	struct physical_device *ph_dev = pAd->physical_dev;
	UCHAR   *ptr = ph_dev->EEPROMImage;
	UCHAR   index, i;
	USHORT  offset = 0;
	UINT	isVaild = 0;
	BOOL	NeedWrite;
	BOOL	WriteStatus;

	/* Only Write to Effuse when RF is not lock down */
	if (!chip_check_rf_lock_down(pAd)) {
		/* Write to Effuse block by block */
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

		for (offset = 0; offset < length; offset += EFUSE_BLOCK_SIZE) {
			MTWF_PRINT("offset 0x%04x:\n", offset);
			NeedWrite = FALSE;
			MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);

			/* Check the Needed contents are different and update the buffer content for write back to Effuse */
			for (index = 0; index < EFUSE_BLOCK_SIZE; index++) {
				/* Obtain the status of this effuse column need to write or not */
				WriteStatus = ops->write_RF_lock_parameter(pAd, offset + index);
				MTWF_PRINT("Effuse[0x%04x]: Write(%d)\n", offset + index, WriteStatus);

				if ((block[index] != ptr[index]) && (WriteStatus == TRUE))
					NeedWrite = TRUE;
				else
					continue;

				MTWF_PRINT("index 0x%04x: ", offset + index);
				MTWF_PRINT("orignal block value=0x%04x, write value=0x%04x\n", block[index],
						ptr[index]);

				if (WriteStatus == TRUE)
					block[index] = ptr[index];
			}

			/* RF Lock Protection */
			if (offset == RF_LOCKDOWN_EEPROME_BLOCK_OFFSET) {
				block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] |= RF_LOCKDOWN_EEPROME_MASK;
				NeedWrite = TRUE;
			}

			/* Only write to Effuse when Needed contents are different in Effuse and Flash */
			if (NeedWrite == TRUE) {
				MTWF_PRINT("write block content: ");

				for (i = 0; i < EFUSE_BLOCK_SIZE; i++)
					MTWF_PRINT("%x ", (UINT)block[i]);

				MTWF_PRINT("\n");
				MtCmdEfuseAccessWrite(pAd, offset, &block[0]);
			}

			ptr += EFUSE_BLOCK_SIZE;
		}
	} else
		MTWF_PRINT("RF is lock now. Cannot write back to Effuse!!\n");

	return status;
}

INT SetRFBackup(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	 status = TRUE;
	CHAR	*value = 0;
	UCHAR   Param  = 0;
	UCHAR   block[EFUSE_BLOCK_SIZE] = "";
	UCHAR   i;
	USHORT  offset = RF_LOCKDOWN_EEPROME_BLOCK_OFFSET;
	UINT	isVaild = 0;

	/* sanity check for input parameter format */
	if (!arg) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	/* parameter parsing */
	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"), i++) {
		switch (i) {
		case 0:
			Param = os_str_tol(value, 0, 10);
			break;

		default: {
			status = FALSE;
			MTWF_PRINT("set wrong parameters\n");
			break;
		}
		}
	}

	if (Param == RF_VALIDATION_NUMBER) {
		/* Check RF lock down status */
		if (chip_check_rf_lock_down(pAd)) {
			/* Read Effuse Contents of Block Address 0x120 */
			MtCmdEfuseAccessRead(pAd, offset, &block[0], &isVaild);
			/* Configue Block 0x12C Content (Unlock RF lock) */
			block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] &= (~(RF_LOCKDOWN_EEPROME_MASK));
			/* Write to Effuse */
			MTWF_PRINT("write block content: ");

			for (i = 0; i < EFUSE_BLOCK_SIZE; i++)
				MTWF_PRINT("%x ", (UINT)block[i]);

			MTWF_PRINT("\n");
			MtCmdEfuseAccessWrite(pAd, offset, &block[0]);
		} else
			MTWF_PRINT("No need to unlock!!\n");
	}

	return status;
}

INT set_hnat_register(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 reg_en;
	INT idx;
	struct wifi_dev *wdev;

	reg_en = os_str_tol(arg, 0, 10);
	MTWF_PRINT("Device Instance\n");

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		MTWF_PRINT("\tWDEV %02d:", idx);

		if (pAd->wdev_list[idx]) {
			wdev = pAd->wdev_list[idx];
			if (wdev->if_up_down_state) {
				MTWF_PRINT("\n\t\tName:%s\n",
					RTMP_OS_NETDEV_GET_DEVNAME(wdev->if_dev));
				MTWF_PRINT("\t\tWdev(list) Idx:%d\n", wdev->wdev_idx);
				MTWF_PRINT("\t\t Idx:%d\n", RtmpOsGetNetIfIndex(wdev->if_dev));
#if defined(CONFIG_FAST_NAT_SUPPORT)

			if (ppe_dev_unregister_hook != NULL &&
				ppe_dev_register_hook != NULL) {
					if (reg_en)
						ppe_dev_register_hook(wdev->if_dev);
					else
						ppe_dev_unregister_hook(wdev->if_dev);
				}
#endif /*CONFIG_FAST_NAT_SUPPORT*/
			}
		} else
			MTWF_PRINT("\n");
	}

	return TRUE;
}

INT Set_MibBucket_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR  MibBucketEnable;

	MibBucketEnable = os_str_tol(arg, 0, 10);

	/* MTWF_PRINT("%s()  BandIdx=%d, MibBucket Enable=%d\n", __func__, BandIdx, MibBucketEnable); */
	pAd->OneSecMibBucket.Enabled = MibBucketEnable;
	pAd->MsMibBucket.Enabled = MibBucketEnable;

	return TRUE;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
INT Set_PBC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8	i;
	CHAR	 *value = 0;

	MTWF_PRINT("Set PBC Up bound:\n");

	for (i = 0, value = rstrtok(arg, ":"); value && (i < PBC_AC_NUM); value = rstrtok(NULL, ":"), i++) {
		pAd->pbc_bound[i] = os_str_tol(value, 0, 10);
		MTWF_PRINT("%d: %d\n", i, pAd->pbc_bound[i]);
	}

	return TRUE;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

void hc_show_radio_info(struct _RTMP_ADAPTER *ad);
void hc_show_hdev_obj(struct wifi_dev *wdev);


static void show_band_info(RTMP_ADAPTER *pAd)
{
	struct mtk_mac_dev *mac_dev = hc_get_mac_dev(pAd);
	struct mtk_mac_phy *mac_phy = &mac_dev->mac_phy;
	CHAR str[32] = "";
	UCHAR *pstr = NULL;
	INT ret;
	USHORT phy_mode = HcGetRadioPhyMode(pAd);


	/* do not change sequence due to 6GHz might include AC/GN then confused */
	if (WMODE_CAP_6G(phy_mode))
		ret = snprintf(str, sizeof(str), "6GHz");
	else if (WMODE_CAP_2G(phy_mode))
		ret = snprintf(str, sizeof(str), "2.4GHz");
	else
		ret = snprintf(str, sizeof(str), "5GHz");

	if (os_snprintf_error(sizeof(str), ret)) {
		MTWF_PRINT("%s: snprintf error!\n", __func__);
		return;
	}

	MTWF_PRINT("==========%s band==========\n", str);
	pstr = wmode_2_str(phy_mode);

	if (pstr != NULL) {
		MTWF_PRINT("wmode\t: %s\n", pstr);
		os_free_mem(pstr);
	}

	OS_SEM_LOCK(&mac_phy->lock);
	MTWF_PRINT("ch\t: %d\n", mac_phy->chan);
	bw_2_str(mac_phy->bw, str, sizeof(str));
	MTWF_PRINT("bw\t: %s\n", str);
	extcha_2_str(mac_phy->ext_cha, str, sizeof(str));
	MTWF_PRINT("extcha\t: %s\n", str);
	MTWF_PRINT("cen_ch\t: %d\n", mac_phy->cen_chan);
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(phy_mode))
		MTWF_PRINT("cen ch2\t: %d\n", mac_phy->chan2);
#endif /*DOT11_VHT_AC*/

	MTWF_PRINT("state\t: %d\n", mac_phy->state);
	OS_SEM_UNLOCK(&mac_phy->lock);
}

/*dump radio information*/
INT show_radio_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	CHAR str[32] = "";
#ifdef DOT11_N_SUPPORT
	CHAR str2[32] = "";
#endif /*DOT11_N_SUPPORT*/
	CHAR *pstr = NULL;
	UCHAR i;
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	ULONG TNow;
	UINT32	Time, TimeDelta;
#endif /*TR181_SUPPORT*/

	MTWF_PRINT("==========BBP radio information==========\n");
#ifdef TR181_SUPPORT
	MTWF_PRINT("------------Band%d----------\n", hc_get_hw_band_idx(pAd));
#ifdef TXRX_STAT_SUPPORT
	MTWF_PRINT("Channel Busy Time(11k scale for last 100ms):%d\n", pAd->Ch_BusyTime_11k);
#endif
#if defined(OFFCHANNEL_SCAN_FEATURE) || defined(TXRX_STAT_SUPPORT)
	MTWF_PRINT("Channel Busy Time:%d, MeasurementDur:%d usec\n",
						pAd->Ch_BusyTime, pAd->ChannelStats.MeasurementDuration);
#endif
	NdisGetSystemUpTime(&TNow);
	Time = jiffies_to_usecs(TNow);
	TimeDelta = Time - ctrl->rdev.pRadioCtrl->CurChannelUpTime;
	MTWF_PRINT("Channel:%d Channel Up time:%u usec\n", HcGetRadioChannel(pAd), TimeDelta);

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
		"TotalChannelChangeCount:%d\n",
		(ctrl->rdev.pRadioCtrl->TotalChannelChangeCount
			+ pAd->ApBootACSChannelChangePerBandCount));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"ManualChannelChangeCount:%d\n", ctrl->rdev.pRadioCtrl->ManualChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"DFSTriggeredChannelChangeCount:%d\n",
		ctrl->rdev.pRadioCtrl->DFSTriggeredChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"ApBootACSChannelChangeCount:%d\n", pAd->ApBootACSChannelChangePerBandCount);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"ForceACSChannelChangeCount:%d\n",
		ctrl->rdev.pRadioCtrl->ForceACSChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
		"RefreshACSChannelChangeCount:%d\n",
		ctrl->rdev.pRadioCtrl->RefreshACSChannelChangeCount);
	MTWF_PRINT("---------------------------\n");
#endif /*TR181_SUPPORT*/

	/*show radio info per band*/
	show_band_info(pAd);
	hc_show_radio_info(pAd);
	MTWF_PRINT("##########WDEV radio information##########\n");

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev) {
			UCHAR cfg_ext_cha = wlan_config_get_ext_cha(wdev);
			UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

			pstr = wdev_type2str(wdev->wdev_type);
			MTWF_PRINT("==========wdev(%d)==========\n", i);
			MTWF_PRINT("type\t: %s\n", pstr);
			MTWF_PRINT("fun_idx\t: %d\n", wdev->func_idx);
			pstr = wmode_2_str(wdev->PhyMode);

			if (pstr != NULL) {
				MTWF_PRINT("wmode\t: %s\n", pstr);
				os_free_mem(pstr);
			}

			MTWF_PRINT("channel\t: %d\n", wlan_operate_get_prim_ch(wdev));
			MTWF_PRINT("ch band\t: %d\n", wlan_operate_get_ch_band(wdev));
#ifdef DOT11_N_SUPPORT

			if (WMODE_CAP_N(wdev->PhyMode)) {
#ifdef DOT11_EHT_BE
				if (wlan_operate_get_bw(wdev) == BW_320)
					MTWF_PRINT("cen_ch1\t: %d\n", wlan_operate_get_eht_cen_ch(wdev));
				else
#endif
					MTWF_PRINT("cen_ch1\t: %d\n", wlan_operate_get_cen_ch_1(wdev));
				bw_2_str(wlan_config_get_ht_bw(wdev), str, sizeof(str));
				bw_2_str(wlan_operate_get_ht_bw(wdev), str2, sizeof(str2));
				MTWF_PRINT("ht_bw\t: (%s,%s)\n", str, str2);
				extcha_2_str(cfg_ext_cha, str, sizeof(str));
				extcha_2_str(op_ext_cha, str2, sizeof(str2));
				MTWF_PRINT("ext_ch\t: (%s,%s)\n", str, str2);
			}

#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(wdev->PhyMode)) {
				MTWF_PRINT("cen_ch2\t: (%d,%d)\n",
						wlan_config_get_cen_ch_2(wdev), wlan_operate_get_cen_ch_2(wdev));
				MTWF_PRINT("vht_bw\t: (%d,%d)\n",
						wlan_config_get_vht_bw(wdev), wlan_operate_get_vht_bw(wdev));
			}

#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
			bw_2_str(wlan_operate_get_bw(wdev), str, sizeof(str));
			MTWF_PRINT("bw\t: %s\n", str);
			/*hdev related*/
			hc_show_hdev_obj(wdev);
		}
	}

	return TRUE;
}

INT set_ba_winsize(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT32 wcid, tid, winsize;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d-%d-%d", &wcid, &tid, &winsize) != 3)
		goto error;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid) || tid > 7 || winsize > 1024) {
		MTWF_PRINT("wcid/tid/winsize(%d/%d/%d) error!\n", wcid, tid, winsize);
		return FALSE;
	}

	pEntry = entry_get(pAd, wcid);

	chip_dbg->set_ba_winsize(pAd, (UINT16)wcid, (UCHAR)tid, winsize);
	pEntry->winsize_limit = winsize;

	MTWF_PRINT("wcid = %d, tid = %d, winsize = %d\n", wcid, tid, winsize);

	return TRUE;

error:
	MTWF_PRINT("please input wcid-winsize\n");
	return TRUE;
}

INT clear_keyloc(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT32 wcid, key_loc;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d-%d", &wcid, &key_loc) != 2)
		goto error;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid) || key_loc >= 0x3) {
		MTWF_PRINT("wcid/key_loc(%d/%d) error!\n", wcid, key_loc);
		return FALSE;
	}

	chip_dbg->clear_keyloc(pAd, wcid, key_loc);

	MTWF_PRINT("wcid = %d, key_loc = %d\n", wcid, key_loc);

	return TRUE;

error:
	MTWF_PRINT("please input wcid-key_loc\n");
	return TRUE;
}

INT set_tx_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ctrl);
	UINT32 wcid, type, amsdu_fix_num, len;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d-%d-%d-%d", &wcid, &type, &amsdu_fix_num, &len) != 4)
		goto error;

	if (type >= 2) {
		MTWF_PRINT("wrong type(%d), please input 0: TX_SW_AMSDU, 1: TX_HW_AMSDU\n", type);
		return FALSE;
	}

	if ((type == TX_HW_AMSDU) && !IS_ASIC_CAP(pAd, fASIC_CAP_HW_TX_AMSDU)) {
		MTWF_PRINT("chip do not have TX_HW_AMSDU cap, please input 0: TX_SW_AMSDU\n");
		return FALSE;
	}

	if (amsdu_fix_num > cap->hw_max_amsdu_nums) {
		MTWF_PRINT("amsdu num is out of range(%d), chip cap(%d)\n",
			amsdu_fix_num, cap->hw_max_amsdu_nums);
		return FALSE;
	}

	MTWF_PRINT("wcid = %d, type: %d, nums  = %d, len = %d(unit: 256bytes)\n", wcid, type, amsdu_fix_num, len);

	if (type == TX_SW_AMSDU) {
		/* SW AMSDU */
		tr_ctl->amsdu_type = TX_SW_AMSDU;
		tr_ctl->amsdu_fix_num = amsdu_fix_num;

		if (tr_ctl->amsdu_fix_num > 0)
			tr_ctl->amsdu_fix = TRUE;
		else
			tr_ctl->amsdu_fix = FALSE;

	} else if (type == TX_HW_AMSDU) {
		/* HW AMSDU */
		tr_ctl->amsdu_type = TX_HW_AMSDU;

		if (chip_dbg->set_hw_amsdu)
			chip_dbg->set_hw_amsdu(pAd, wcid, amsdu_fix_num, len);
	}

	return TRUE;

error:
	MTWF_PRINT("please input wcid-type-amsdu_nums-len");
	MTWF_PRINT(" (type: 0: TX_SW_AMSDU, 1:TX_HW_AMSDU), len unit 256b\n");
	return TRUE;
}
INT set_rx_amsdu(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	UINT32 wcid, type;
	struct tx_rx_ctl *tr_ctl = &pAd->tr_ctl;
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(ctrl);

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d-%d", &wcid, &type) != 2)
		goto error;

	if (type >= 4) {
		MTWF_PRINT("wrong type(%d), please input 0: RX_SW_AMSDU, 1: RX_HW_AMSDU, 2: Enable RX AMSDU, 3: Disable RX AMSDU\n", type);
		return FALSE;
	}

	if ((type == RX_HW_AMSDU) && !IS_ASIC_CAP(pAd, fASIC_CAP_RX_HDR_TRANS)) {
		MTWF_PRINT("chip do not have RX_HW_AMSDU cap, please input 0: RX_SW_AMSDU\n");
		return FALSE;
	}

	if (type == RX_SW_AMSDU) {
		/* SW AMSDU */
		tr_ctl->damsdu_type = RX_SW_AMSDU;

		if (chip_dbg->set_header_translation)
			chip_dbg->set_header_translation(pAd, wcid, FALSE);

	} else if (type == RX_HW_AMSDU) {
		/* HW AMSDU */
		tr_ctl->damsdu_type = RX_HW_AMSDU;

		if (chip_dbg->set_header_translation)
			chip_dbg->set_header_translation(pAd, wcid, TRUE);
	} else if (type == ENABLE_RX_AMSDU) {
		/* Disable RX AMSDU */
		pAd->physical_dev->chip_cap.ppdu.rx_amsdu_in_ampdu_support = TRUE;
	} else if (type == DISABLE_RX_AMSDU) {
		/* Disable RX AMSDU */
		pAd->physical_dev->chip_cap.ppdu.rx_amsdu_in_ampdu_support = FALSE;
	}

	return TRUE;

error:
	MTWF_PRINT("please input wcid-type (type: 0: RX_SW_AMSDU, 1:RX_HW_AMSDU)\n");
	return TRUE;
}

INT set_ba_dbg(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	UINT32 record_basic, record_mac, dump_within, dump_surpass, dump_old, dump_dup, dump_stepone, dump_bar;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(pAd->physical_dev);

	if (arg == NULL || strlen(arg) == 0)
		goto error;

	if (sscanf(arg, "%d:%d:%d:%d:%d:%d:%d:%d", &record_basic, &record_mac,
			&dump_within, &dump_surpass, &dump_old, &dump_dup, &dump_stepone, &dump_bar) != 8)
		goto error;

	if (record_basic)
		ba_ctl->dbg_flag |= SN_RECORD_BASIC;
	if (record_mac)
		ba_ctl->dbg_flag |= SN_RECORD_MAC;
	if (dump_within)
		ba_ctl->dbg_flag |= SN_DUMP_WITHIN;
	if (dump_surpass)
		ba_ctl->dbg_flag |= SN_DUMP_SURPASS;
	if (dump_old)
		ba_ctl->dbg_flag |= SN_DUMP_OLD;
	if (dump_dup)
		ba_ctl->dbg_flag |= SN_DUMP_DUP;
	if (dump_stepone)
		ba_ctl->dbg_flag |= SN_DUMP_STEPONE;
	if (dump_bar)
		ba_ctl->dbg_flag |= SN_DUMP_BAR;

	return TRUE;

error:
	MTWF_PRINT("please input record_basic:record_mac:dump_within:dump_surpass");
	MTWF_PRINT(":dump_old:dump_dup:dump_stepone:dump_bar\n");
	return TRUE;
}

INT set_tx_deq_cpu(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	RTMP_STRING *str;
	UINT8 band;
	UINT32 cpu;

	if (arg != NULL && strlen(arg)) {
		str = strsep(&arg, ":");
		band = os_str_tol(str, 0, 10);
	} else {
		goto err;
	}

	if (arg != NULL && strlen(arg)) {
		cpu = os_str_tol(arg, 0, 10);
	} else {
		goto err;
	}

	if ((band > 1) || (cpu > 3))
		goto err;

	MTWF_PRINT("band %d cpu = %u\n", band, cpu);

	return TRUE;

err:
	MTWF_PRINT("invalid input, should be band:cpu\n");
	return TRUE;
}

INT set_tx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	struct fp_qm *qm_parm = (struct fp_qm *)PD_GET_QM_PARM(pAd->physical_dev);
	UINT32 tx_max_cnt = 1024;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (sscanf(arg, "%d", &tx_max_cnt) != 1)
		goto err;

	qm_parm->max_tx_process_cnt = tx_max_cnt;

	return TRUE;

err:
	MTWF_PRINT("invalid input, should be tx_max_cnt\n");
	return TRUE;
}

INT set_rx_max_cnt(
	PRTMP_ADAPTER pAd,
	char *arg)
{
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = NULL;
	RTMP_STRING *str;
	UINT8 resrc_idx;
	UINT32 max_cnt;

	if (arg != NULL && strlen(arg)) {
		str = strsep(&arg, ":");
		resrc_idx = os_str_tol(str, 0, 10);
	} else {
		goto err;
	}

	if (arg != NULL && strlen(arg)) {
		max_cnt = os_str_tol(arg, 0, 10);
	} else {
		goto err;
	}

	if (resrc_idx > hif->rx_res_num)
		goto err;

	rx_ring = pci_get_rx_ring_by_ridx(hif, resrc_idx);

	rx_ring->max_rx_process_cnt = max_cnt;

	MTWF_PRINT("rx ring %d max count = %u\n", resrc_idx, max_cnt);

	return TRUE;

err:
	MTWF_PRINT("invalid input, should be resource_idx:max_rx_process_cnt\n");
#endif /*RTMP_MAC_PCI*/
	return TRUE;
}

#ifdef CUT_THROUGH
INT set_token_setting(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	INT32 qidx, option, sub_option, value;
	INT32 ret;

	if (arg == NULL || strlen(arg) == 0)
		goto err;

	if (sscanf(arg, "%d-%d-%d-%d", &qidx, &option, &sub_option, &value) != 3)
		goto err;

	ret = token_tx_setting(pAd, qidx, option, sub_option, value);

	if (!ret)
		return TRUE;
err:
	MTWF_PRINT("invalid input, should be [qidx]-[option]-[sub_option]-[value]\n");

	return TRUE;
}
#endif

INT set_rx_cnt_io_thd(
	PRTMP_ADAPTER pAd,
	char *arg)
{
#ifdef RTMP_MAC_PCI
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_rx_ring *rx_ring = NULL;
	RTMP_STRING *str;
	UINT8 resrc_idx;
	UINT32 max_cnt;

	if (arg != NULL && strlen(arg)) {
		str = strsep(&arg, ":");
		resrc_idx = os_str_tol(str, 0, 10);
	} else {
		goto err;
	}

	if (arg != NULL && strlen(arg))
		max_cnt = os_str_tol(arg, 0, 10);
	else
		goto err;

	if (resrc_idx > hif->rx_res_num)
		goto err;

	rx_ring = pci_get_rx_ring_by_ridx(hif, resrc_idx);

	rx_ring->max_sw_read_idx_inc = max_cnt;

	MTWF_PRINT("rx ring %d rx count threshold for io = %u\n", resrc_idx, max_cnt);

	return TRUE;

err:
	MTWF_PRINT("invalid input, should be resource_idx:rx_cnt_io_thd\n");
#endif /*RTMP_MAC_PCI*/
	return TRUE;
}

INT set_rx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	return TRUE;
}

INT set_tx_dly_ctl(
	PRTMP_ADAPTER pAd,
	char *arg)
{
	return TRUE;
}

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
void set_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd,
	BOOLEAN flag_pcie_aspm_dym_ctrl)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (flag_pcie_aspm_dym_ctrl) {
		pChipCap->asic_caps |= fASIC_CAP_PCIE_ASPM_DYM_CTRL;
	} else {
		pChipCap->asic_caps &= ~fASIC_CAP_PCIE_ASPM_DYM_CTRL;
	}
}

BOOLEAN get_pcie_aspm_dym_ctrl_cap(
	PRTMP_ADAPTER pAd)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	return ((pChipCap->asic_caps & fASIC_CAP_PCIE_ASPM_DYM_CTRL) ? TRUE : FALSE);
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT SetLoadFwMethod(
	IN	PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT Enable;/* enable load from bin */

	Enable = os_str_tol(arg, 0, 10);
	if (!Enable) {
		pChipCap->load_fw_method = BIT(HEADER_METHOD);
		MTWF_PRINT("Load from header\n");
	} else {
		pChipCap->load_fw_method = BIT(BIN_METHOD);
		MTWF_PRINT("Load from bin\n");
	}
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
/* multi client config  */
INT set_mcli_cfg(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 rv, cmd, op;

	if (arg) {
		rv = sscanf(arg, "%u-%u", &cmd, &op);

		if (rv <= 0)
			return FALSE;

		switch (cmd) {
		case MCLI_DEBUG_ON:
			pAd->mcli_ctl.debug_on = op;
			MTWF_PRINT("debug_on=0x%x\n", pAd->mcli_ctl.debug_on);
			break;
		case MCLI_CLI_NUMS_EAP_TH:
			pAd->multi_cli_nums_eap_th = op;
			MTWF_PRINT("eap_cli_nums=%u\n", pAd->multi_cli_nums_eap_th);
			break;
		default:
			MTWF_PRINT("cmd:%d, op:%d\n", cmd, op);
			break;
		}
	} else
		return FALSE;

	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */

INT set_fwcmd_timeout_print_cnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	pAd->FwCmdTimeoutPrintCnt = (UINT16)os_str_tol(arg, 0, 10);
	MTWF_PRINT("%s(): count = %d %s\n", __func__, pAd->FwCmdTimeoutPrintCnt,
			 (pAd->FwCmdTimeoutPrintCnt == 0) ? "(unlimited)" : "");

	return TRUE;
}

INT show_fwcmd_timeout_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 print_cnt = 0;
	UINT16 i = 0, start_idx;
	UINT16 rec_idx;
	P_FWCMD_TIMEOUT_RECORD pToRec = NULL;
	ULONG Now32;

	MTWF_PRINT("\n%s:\n", __func__);

	NdisGetSystemUpTime(&Now32);
	print_cnt = (pAd->FwCmdTimeoutCnt < FW_CMD_TO_RECORD_CNT) ?
				 pAd->FwCmdTimeoutCnt : FW_CMD_TO_RECORD_CNT;

	MTWF_PRINT("\t- TimeoutCnt(%d), PrintCnt(%d), RecordCnt(%d)\n",
			 pAd->FwCmdTimeoutCnt, pAd->FwCmdTimeoutPrintCnt, FW_CMD_TO_RECORD_CNT);

	if (print_cnt) {
		start_idx = (pAd->FwCmdTimeoutCnt - 1) % FW_CMD_TO_RECORD_CNT ;
		for (i = 0; i < print_cnt; i++) {
			rec_idx = (start_idx + FW_CMD_TO_RECORD_CNT - i) % FW_CMD_TO_RECORD_CNT;
			pToRec = &pAd->FwCmdTimeoutRecord[rec_idx];
			MTWF_PRINT("\[%02d/%02d] (%ld ms ago) FWCmdTimeout: cmd(%x)",
					i, rec_idx, ((Now32 - pToRec->timestamp) * 1000 / OS_HZ),
					pToRec->type);
			MTWF_PRINT(", ext_cmd(%x), seq(%d), state(%d)\n",
					pToRec->ext_type, pToRec->seq, pToRec->state);
		}
	}

	return TRUE;
}


INT Set_BSSAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
	BOOLEAN ie_update = FALSE;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_PRINT("pBssEdca is NULL\n");
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] < 2) || (value_temp[i] > 15)) {
			MTWF_PRINT(
			"BSSAifsn[%d]=%d is invalid, BSSAifsn should be configured from 2 to 15\n",
			i, value_temp[i]);
			return FALSE;
		}
	}

	if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_EDCA) == FALSE)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO, "bcn_bpcc_op_lock fail\n\r");

	/* check parameter changed or not */
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if (pBssEdca->Aifsn[i] != value_temp[i])
			ie_update = TRUE;
	}

	MTWF_PRINT("start to configure BSSAifsn\n");

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Aifsn[i] = value_temp[i];
		MTWF_PRINT("BSSAifsn[%d]=%d\n", i, pBssEdca->Aifsn[i]);
	}

	if (ie_update) {
		pBssEdca->EdcaUpdateCount++;
		UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_EDCA, ie_update);
	} else
		bcn_bpcc_op_unlock(pAd, wdev, TRUE);

	return TRUE;
}

INT Set_BSSCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
	BOOLEAN ie_update = FALSE;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_PRINT("pBssEdca is NULL\n");
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 15)) {
			MTWF_PRINT("BSSCwmin[%d]=%d is invalid, BSSCwmin should not be over 15\n",
				i, value_temp[i]);
			return FALSE;
		}
	}

	if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_EDCA) == FALSE)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO, "bcn_bpcc_op_lock fail\n\r");

	/* check parameter changed or not */
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if (pBssEdca->Cwmin[i] != value_temp[i])
			ie_update = TRUE;
	}

	MTWF_PRINT("start to configure BSSCwmin\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Cwmin[i] = value_temp[i];
		MTWF_PRINT("BSSCwmin[%d]=%d\n", i, pBssEdca->Cwmin[i]);
	}

	if (ie_update) {
		pBssEdca->EdcaUpdateCount++;
		UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_EDCA, ie_update);
	} else
		bcn_bpcc_op_unlock(pAd, wdev, TRUE);

	return TRUE;
}

INT Set_BSSCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
	BOOLEAN ie_update = FALSE;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_PRINT("pBssEdca is NULL\n");
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 15)) {
			MTWF_PRINT("BSSCwmax[%d]=%d is invalid, BSSCwmax should not be over 15\n",
				i, value_temp[i]);
			return FALSE;
		}
	}

	if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_EDCA) == FALSE)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO, "bcn_bpcc_op_lock fail\n\r");

	/* check parameter changed or not */
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if (pBssEdca->Cwmax[i] != value_temp[i])
			ie_update = TRUE;
	}

	MTWF_PRINT("start to configure BSSCwmax\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Cwmax[i] = value_temp[i];
		MTWF_PRINT("BSSCwmax[%d]=%d\n", i, pBssEdca->Cwmax[i]);
	}

	if (ie_update) {
		pBssEdca->EdcaUpdateCount++;
		UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_EDCA, ie_update);
	} else
		bcn_bpcc_op_unlock(pAd, wdev, TRUE);

	return TRUE;
}

INT Set_BSSTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	USHORT value_temp[WMM_NUM_OF_AC] = {0};
	BOOLEAN ie_update = FALSE;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_PRINT("pBssEdca is NULL\n");
		return FALSE;
	}

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (USHORT)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_EDCA) == FALSE)
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO, "bcn_bpcc_op_lock fail\n\r");

	/* check parameter changed or not */
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if (pBssEdca->Txop[i] != value_temp[i])
			ie_update = TRUE;
	}

	MTWF_PRINT("start to configure BSSTxop\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pBssEdca->Txop[i] = value_temp[i];
		MTWF_PRINT("BSSTxop[%d]=%d\n", i, pBssEdca->Txop[i]);
	}

	if (ie_update) {
		pBssEdca->EdcaUpdateCount++;
		UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_EDCA, ie_update);
	} else
		bcn_bpcc_op_unlock(pAd, wdev, TRUE);

	return TRUE;

}

UCHAR ac_queue[] = {
	TxQ_IDX_AC1, /* ACI:0 AC_BE */
	TxQ_IDX_AC0, /* ACI:1 AC_BK */
	TxQ_IDX_AC2, /* ACI:2 AC_VI */
	TxQ_IDX_AC3, /* ACI:3 AC_VO */
};

INT Set_APAifsn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] < 1) || (value_temp[i] > 15)) {
			MTWF_PRINT(
			"APAifsn[%d]=%d is invalid, APAifsn should be configured from 1 to 15\n",
			i, value_temp[i]);
			return FALSE;
		}
	}

	MTWF_PRINT("start to configure APAifsn\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Aifsn[i] = value_temp[i];
		MTWF_PRINT("APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, ac_index, WMM_PARAM_AIFSN, value_temp[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_AIFSN, value_temp[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = FALSE;
			MTWF_PRINT("fails to set APAifsn[%d]\n", i);
		 }
	}

	return success;
}

INT Set_APCwmin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 16)) {
			MTWF_PRINT("APCwmin[%d]=%d is invalid, APCwmin should not be over 16\n",
				i, value_temp[i]);
			return FALSE;
		}
	}

	MTWF_PRINT("start to configure APCwmin\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmin[i] = value_temp[i];
		MTWF_PRINT("APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, ac_index, WMM_PARAM_CWMIN, value_temp[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMIN, value_temp[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = FALSE;
			MTWF_PRINT("fails to set APCwmin[%d]\n", i);
		 }
	}

	return success;
}

INT Set_APCwmax_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	UCHAR value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (UCHAR)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((value_temp[i] > 16)) {
			MTWF_PRINT("APCwmax[%d]=%d is invalid, APCwmax should not be over 16\n",
				i, value_temp[i]);
			return FALSE;
		}
	}

	MTWF_PRINT("start to configure APCwmax\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmax[i] = value_temp[i];
		MTWF_PRINT("APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, ac_index, WMM_PARAM_CWMAX, value_temp[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMAX, value_temp[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = FALSE;
			MTWF_PRINT("fails to set APCwmax[%d]\n", i);
		 }
	}

	return success;
}

INT Set_APTxop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR i;
	RTMP_STRING *ptr;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = TRUE;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
	USHORT value_temp[WMM_NUM_OF_AC] = {0};
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return FALSE;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return FALSE;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	for (i = 0, ptr = rstrtok(arg, ":"); ptr; ptr = rstrtok(NULL, ":"), i++) {
		if (i < WMM_NUM_OF_AC)
			value_temp[i] = (USHORT)os_str_tol(ptr, 0, 10);
	}

	if (i != WMM_NUM_OF_AC) {
		MTWF_PRINT("invalid format, parameters should be configured as xx:xx:xx:xx\n");
		return FALSE;
	}

	MTWF_PRINT("start to configure APTxop\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i] = value_temp[i];
		if (wmm_edca_param)
			wmm_edca_param->Txop[i] = value_temp[i];
		MTWF_PRINT("APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, ac_index, WMM_PARAM_TXOP, value_temp[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_TXOP, value_temp[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = FALSE;
			MTWF_PRINT("fails to set APTxop[%d]\n", i);
		}
	}

	return success;
}

#ifdef RT_CFG80211_SUPPORT
/* format : iwpriv [interface] set mbo_ch_pref=[channel]-[preference]-[reason code]
 *  * sample : iwpriv ra0 set mbo_ch_pref=48-0-2
 *   *  ==> channel 48 at preference 0 reason code 2
 *   */
#define PRESET_PREF 1
#define CLEAR_PREF 2
#define CHNG_PREF 3
#ifdef MBO_SUPPORT
INT mtk_cfg80211_Set_mbo_npc_Proc(
		PRTMP_ADAPTER pAd,
		UCHAR *npc_value)
{
	UINT8 i = 0, channel = 0, pref = 0, reason_code = 0, reg_class = 0, req_type = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *pWdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	P_MBO_CTRL pMboCtrl = &pWdev->MboCtrl;

	if (!IS_MBO_ENABLE(pWdev)) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_ERROR,
				"MBO is disabled %d\n", pWdev->MboCtrl.bMboEnable);
		return FALSE;
	}

	req_type = npc_value[0];
	channel = npc_value[1];
	pref = npc_value[2];
	reg_class = npc_value[3];
	reason_code = npc_value[4];

	if (req_type != CLEAR_PREF) {
		for (i = 0; i < MBO_NPC_MAX_LEN; i++) {
			if (pMboCtrl->npc[i].ch > 0)
				continue;
			else {
				pMboCtrl->npc[i].ch = channel;
				pMboCtrl->npc[i].pref = pref;
				pMboCtrl->npc[i].reason_code = reason_code;
				pMboCtrl->npc[i].reg_class = reg_class;
			}
			break;
		}
	} else {
		NdisZeroMemory(pMboCtrl->npc, sizeof(pMboCtrl->npc));
	}

	if (req_type != PRESET_PREF)
		Send_WNM_Notify_Req_toAP(pAd, pWdev);

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_MBO, DBG_LVL_INFO,
			"req_type %d channel %d, pref %d, reason_code %d reg_class %d\n",
			req_type, channel, pref, reason_code, reg_class);
	return TRUE;
}
#endif

INT mtk_cfg80211_Set_APAifsn_Proc(RTMP_ADAPTER *pAd, UCHAR *aifsn_value)
{
	UCHAR i;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = 0;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return -EFAULT;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return -EFAULT;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((aifsn_value[i] < 1) || (aifsn_value[i] > 15)) {
			MTWF_PRINT(
			"APAifsn[%d]=%d is invalid, APAifsn should be configured from 1 to 15\n",
			i, aifsn_value[i]);
			return -EFAULT;
		}
	}

	MTWF_PRINT("start to configure APAifsn\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i] = aifsn_value[i];
		if (wmm_edca_param)
			wmm_edca_param->Aifsn[i] = aifsn_value[i];
		MTWF_PRINT("APAifsn[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Aifsn[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, i, WMM_PARAM_AIFSN, aifsn_value[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_AIFSN, aifsn_value[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = -EFAULT;
			MTWF_PRINT("fails to set APAifsn[%d]\n", i);
		}
	}

	return success;
}

INT mtk_cfg80211_Set_APCwmin_Proc(RTMP_ADAPTER *pAd, UCHAR *cwmin_value)
{
	UCHAR i;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = 0;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return -EFAULT;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return -EFAULT;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((cwmin_value[i] > 16)) {
			MTWF_PRINT("APCwmin[%d]=%d is invalid, APCwmin should not be over 16\n",
				i, cwmin_value[i]);
			return -EFAULT;
		}
	}

	MTWF_PRINT("start to configure APCwmin\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i] = cwmin_value[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmin[i] = cwmin_value[i];
		MTWF_PRINT("APCwmin[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmin[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, i, WMM_PARAM_CWMIN, cwmin_value[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMIN, cwmin_value[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = -EFAULT;
			MTWF_PRINT("fails to set APCwmin[%d]\n", i);
		}
	}

	return success;
}

INT mtk_cfg80211_Set_APCwmax_Proc(RTMP_ADAPTER *pAd, UCHAR *cwmax_value)
{
	UCHAR i;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = 0;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return -EFAULT;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return -EFAULT;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		if ((cwmax_value[i] > 16)) {
			MTWF_PRINT("APCwmax[%d]=%d is invalid, APCwmax should not be over 16\n",
				i, cwmax_value[i]);
			return -EFAULT;
		}
	}

	MTWF_PRINT("start to configure APCwmax\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i] = cwmax_value[i];
		if (wmm_edca_param)
			wmm_edca_param->Cwmax[i] = cwmax_value[i];
		MTWF_PRINT("APCwmax[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Cwmax[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, i, WMM_PARAM_CWMAX, cwmax_value[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_CWMAX, cwmax_value[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = -EFAULT;
			MTWF_PRINT("fails to set APCwmax[%d]\n", i);
		}
	}

	return success;
}

INT mtk_cfg80211_Set_APTxop_Proc(RTMP_ADAPTER *pAd, UCHAR *txop_value)
{
	UCHAR i;
	UCHAR wmm_idx = 0;
	UCHAR ac_index;
	UCHAR success = 0;
	struct wifi_dev *wdev = NULL;
	EDCA_PARM *wmm_edca_param = NULL;
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT ap_idx = pObj->ioctl_if;

	if (ap_idx >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid mbss id(%d)\n", ap_idx);
		return -EFAULT;
	}
	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;
#endif
	if (wdev == NULL) {
		MTWF_PRINT("wdev is NULL\n");
		return -EFAULT;
	}
	wmm_idx = HcGetWmmIdx(pAd, wdev);
	wmm_edca_param = hwifi_get_edca(pAd, wdev);

	MTWF_PRINT("start to configure APTxop\n");
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i] = txop_value[i];
		if (wmm_edca_param)
			wmm_edca_param->Txop[i] = txop_value[i];
		MTWF_PRINT("APTxop[%d]=%d\n", i, pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx].Txop[i]);
		ac_index = ac_queue[i];
#ifdef WIFI_UNIFIED_COMMAND
		if (AsicSetWmmParam(pAd, wdev, wmm_idx, i, WMM_PARAM_TXOP, txop_value[i]) == NDIS_STATUS_FAILURE)
#else
		if (AsicSetWmmParam(pAd, wmm_idx, ac_index, WMM_PARAM_TXOP, txop_value[i]) == NDIS_STATUS_FAILURE)
#endif /* WIFI_UNIFIED_COMMAND */
		{
			success = -EFAULT;
			MTWF_PRINT("fails to set APTxop[%d]\n", i);
		}
	}

	return success;
}

INT mtk_cfg80211_get_wmm_cap_status(RTMP_ADAPTER *pAd, struct wiphy *wiphy)
{
	struct sk_buff *skb;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	struct wifi_dev *wdev = NULL;
	UINT8 status;

	if (apidx >= pAd->ApCfg.BssidNum)
		return -EINVAL;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!wdev)
		return -EINVAL;

	status = wdev->bWmmCapable;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, 2);
	if (!skb) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"fail to allocate reply msg\n");
		return -EINVAL;
	}

	if (nla_put_u8(skb, MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO, status)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"fail to put nla to skb\n");
		kfree_skb(skb);
		return -EINVAL;
	}

	if (mt_cfg80211_vendor_cmd_reply(skb)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_ERROR,
			"reply msg failed\n");
		return -EINVAL;
	}

	return 0;
}

INT mtk_cfg80211_set_wmm_cap(RTMP_ADAPTER *pAd, BOOLEAN bWmmCapable)
{
	POS_COOKIE	pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev;
	UINT ifIndex = pObj->ioctl_if;

	if (ifIndex >= MAX_BEACON_NUM) {
		MTWF_PRINT("invalid ioctl_if %d\n", ifIndex);
		return -EINVAL;
	}

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

	if (bWmmCapable == 1)
		wdev->bWmmCapable = TRUE;
	else if (bWmmCapable == 0)
		wdev->bWmmCapable = FALSE;
	else
		return -EINVAL;  /*Invalid argument */

	pAd->ApCfg.MBSSID[ifIndex].bWmmCapableOrg =
		pAd->ApCfg.MBSSID[ifIndex].wdev.bWmmCapable;

#ifdef DOT11_N_SUPPORT
	/*Sync with the HT relate info. In N mode, we should re-enable it */
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	MTWF_PRINT("IF(ra%d) (bWmmCapable=%d)\n", ifIndex, wdev->bWmmCapable);
	return 0;
}

#endif/*RT_CFG80211_SUPPORT*/

#ifdef MGMT_TXPWR_CTRL
/* Tx pwr offset in 0.5db unit*/
#define TX_PWR_OFFSET_WTBL_MIN -31
#define TX_PWR_OFFSET_WTBL_MAX 0

INT set_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT16  value;
	CHAR   target_pwr = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	RTMP_STRING *tmp;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!arg) {
		MTWF_PRINT("Invalid argument\n");
		return FALSE;
	}

	if (*arg == '.') { /* handle .5 arg value*/
		MTWF_PRINT("unable to update tx pwr %s\n", arg);
		return FALSE;
	}

	tmp = rstrtok(arg, ".");

	if (!tmp) {
		MTWF_PRINT("Please enter valid tx pwr\n");
		return FALSE;
	}

	value = os_str_tol(tmp, 0, 10);
	target_pwr = value*2; /* convert to  0.5db scale*/
	tmp = rstrtok(NULL, ".");

	if (tmp) {
		value = os_str_tol(tmp, 0, 10);
		if (value == 5)
			target_pwr += 1;
		else if (target_pwr == 0) {
			MTWF_PRINT("unable to update tx pwr\n");
			return FALSE;
		}
	}

	wdev->MgmtTxPwrBak = wdev->MgmtTxPwr;
	wdev->MgmtTxPwr = target_pwr;
	update_mgmt_frame_power(pAd, wdev);
	return TRUE;
}

INT update_mgmt_frame_power(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	CHAR   target_pwr;
	CHAR delta_pwr_cck[MODULATION_SYSTEM_CCK_NUM] = {0};
	CHAR delta_pwr_ofdm[MODULATION_SYSTEM_OFDM_NUM] = {0};
	UINT8  BandIdx;
	INT8 txpwr_offset_min = 0, txpwr_offset_max = 0;
	UCHAR cnt = 0;
	BOOLEAN set_pwr_offset_flag = TRUE;

	BandIdx = HcGetBandByWdev(wdev);

	target_pwr = wdev->MgmtTxPwr;

	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_DISABLE_TX));

	if (target_pwr == 0) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"disable mgmt pwr ctrl\n");
		wdev->bPwrCtrlEn = FALSE;
		wdev->TxPwrDelta = 0;

		for (cnt = 0; cnt < MODULATION_SYSTEM_CCK_NUM; cnt++)
			wdev->mgmt_txd_txpwr_offset_cck[cnt] = 0;
		for (cnt = 0; cnt < MODULATION_SYSTEM_OFDM_NUM; cnt++)
			wdev->mgmt_txd_txpwr_offset_ofdm[cnt] = 0;

		goto exit;
	}

	if (pAd->ApCfg.MgmtTxPwrCck[0] == 0) {
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, BandIdx);
		while (!pAd->ApCfg.MgmtTxPwrCck[0]) {
			RtmpusecDelay(50);
			cnt++;
			if (cnt > 20)
				break;
		}
	}

	txpwr_offset_min = TXD_TX_PWR_OFFSET_VALUE_MIN;
	txpwr_offset_max = TXD_TX_PWR_OFFSET_VALUE_MAX;
	for (cnt = 0; cnt < MODULATION_SYSTEM_CCK_NUM; cnt++) {
		delta_pwr_cck[cnt] = target_pwr - (pAd->ApCfg.MgmtTxPwrCck[cnt] + pAd->ApCfg.EpaFeGain);
		MTWF_PRINT("delta_pwr_cck[%d] %d\n", cnt, delta_pwr_cck[cnt]);
		if (delta_pwr_cck[cnt] < txpwr_offset_min || delta_pwr_cck[cnt] > txpwr_offset_max
		    || target_pwr > pAd->ApCfg.MaxMgmtTxpwr) {
			set_pwr_offset_flag = false;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
				"CCK unable to adjust target pwr\n");
			break;
		}
	}
	for (cnt = 0; cnt < MODULATION_SYSTEM_OFDM_NUM; cnt++) {
		delta_pwr_ofdm[cnt] = target_pwr - (pAd->ApCfg.MgmtTxPwrOfdm[cnt] + pAd->ApCfg.EpaFeGain);
		MTWF_PRINT("delta_pwr_ofdm[%d] %d\n", cnt, delta_pwr_ofdm[cnt]);
		if (delta_pwr_ofdm[cnt] < txpwr_offset_min || delta_pwr_ofdm[cnt] > txpwr_offset_max
		    || target_pwr > pAd->ApCfg.MaxMgmtTxpwr) {
			set_pwr_offset_flag = false;
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
				"OFDM unable to adjust target pwr\n");
			break;
		}
	}
	if (set_pwr_offset_flag) {
		for (cnt = 0; cnt < MODULATION_SYSTEM_CCK_NUM; cnt++)
			wdev->mgmt_txd_txpwr_offset_cck[cnt] = delta_pwr_cck[cnt];
		for (cnt = 0; cnt < MODULATION_SYSTEM_OFDM_NUM; cnt++)
			wdev->mgmt_txd_txpwr_offset_ofdm[cnt] = delta_pwr_ofdm[cnt];

		wdev->MgmtTxPwrBak = wdev->MgmtTxPwr;
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN, "unable to adjust target pwr\n");
		wdev->MgmtTxPwr = wdev->MgmtTxPwrBak;

		if (wdev->bPwrCtrlEn)
			goto exit;
	}

exit:

	UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ENABLE_TX));

	return TRUE;
}

INT show_mgmt_frame_power(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT8	Tx_Pwr, BandIdx;
	struct wifi_dev	*wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	BandIdx = HcGetBandByWdev(wdev);

	Tx_Pwr = wdev->MgmtTxPwr;

	MTWF_PRINT("[%s] Tx_Pwr = %d.%d db\n",
		__func__, Tx_Pwr/2, ((Tx_Pwr%2 == 0) ? 0:5));

	return TRUE;
}
#endif
#ifdef PKTLOSS_CHK
extern BOOLEAN pktloss_chk_func(PRTMP_ADAPTER pAd,
				PUINT_8 aucPktContent,
				UINT_8 ip_offset,
				UINT_8 check_point,
				BOOLEAN drop);
extern void pktloss_chk_dump(PRTMP_ADAPTER pAd, BOOLEAN dump_all);

INT set_pktloss_chk(
	IN  PRTMP_ADAPTER pAd,
	IN  RTMP_STRING * arg)
{
	UINT32 rv, cmd, param = 0;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (!arg)
		return FALSE;

	rv = sscanf(arg, "%d-%d", &cmd, &param);
	if ((rv <= 0) || (cmd >= PKTLOSS_CHK_SET_MAX))
		return FALSE;

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		MtUniCmdFwLog2Host(pAd, HOST2CR4, ENUM_CMD_FW_LOG_2_HOST_CTRL_2_HOST);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdFwLog2Host(pAd, 1, 2);
	if (cmd == PKTLOSS_CHK_SET_IPERF_CASE) {
		/* iperf */
		os_zero_mem(&pAd->pktloss_chk, sizeof(pktloss_check_var_type));
		pAd->pktloss_chk.pktloss_chk_handler = pktloss_chk_func;
		pAd->pktloss_chk.src_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_port = param;
		pAd->pktloss_chk.byte_offset = 0;
		pAd->pktloss_chk.is_seq_signed = 1;
		pAd->pktloss_chk.is_seq_cross_zero = 1;
		pAd->pktloss_chk.seq_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.ctrl_flag = (1<<PKTLOSS_CHK_BY_SEQ);
		pAd->pktloss_chk.ts_threshold = 20000/(1000000/HZ);
		pAd->pktloss_chk.proto = 17;
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_RESET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SRC_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DEST_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_PORT, param);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_OFFSET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_SIGNED, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_CROSS_ZERO, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SEQ_MASK, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_CTRL_FLAG,
			(1<<PKTLOSS_CHK_BY_SEQ));
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_TS_THRESHOLD, 20000/30.5);

		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_ENABLE, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DUMP_SHORT, 0);
		pktloss_chk_dump(pAd, TRUE);
		pAd->pktloss_chk.enable = TRUE;
	} else if (cmd == PKTLOSS_CHK_SET_RTP_CASE) {
		/* RTP */
		os_zero_mem(&pAd->pktloss_chk, sizeof(pktloss_check_var_type));
		pAd->pktloss_chk.pktloss_chk_handler = pktloss_chk_func;
		pAd->pktloss_chk.src_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_ip_mask = 0xFFFFFFFF;
		pAd->pktloss_chk.dest_port = param;
		pAd->pktloss_chk.byte_offset = 0;
		pAd->pktloss_chk.is_seq_signed = 0;
		pAd->pktloss_chk.is_seq_cross_zero = 1;
		pAd->pktloss_chk.seq_mask = 0x0FFFF;
		pAd->pktloss_chk.ctrl_flag = (1<<PKTLOSS_CHK_BY_SEQ)|(1<<PKTLOSS_CHK_BY_TS);
		pAd->pktloss_chk.ts_threshold = 20000/(1000000/HZ);
		pAd->pktloss_chk.proto = 17;
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_RESET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SRC_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DEST_IP, 0xFFFFFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_PORT, param);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_OFFSET, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_SIGNED, 0);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_IS_SEQ_CROSS_ZERO, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_SEQ_MASK, 0xFFFF);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_CTRL_FLAG,
			(1<<PKTLOSS_CHK_BY_SEQ)|(1<<PKTLOSS_CHK_BY_TS));
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_TS_THRESHOLD, 20000/30.5);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_ENABLE, 1);
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, PKTLOSS_CHK_SET_DUMP_SHORT, 0);
		pktloss_chk_dump(pAd, TRUE);
		pAd->pktloss_chk.enable = TRUE;
	} else {
		switch (cmd) {
		case PKTLOSS_CHK_SET_SRC_IP:
			pAd->pktloss_chk.src_ip_mask = param;
			MTWF_PRINT("[%s] src_ip_mask:%u\n",
				__func__, pAd->pktloss_chk.src_ip_mask);
			break;
		case PKTLOSS_CHK_SET_DEST_IP:
			pAd->pktloss_chk.dest_ip_mask = param;
			MTWF_PRINT("[%s] dest_ip_mask:%u\n",
				__func__, pAd->pktloss_chk.dest_ip_mask);
			break;
		case PKTLOSS_CHK_SET_PORT:
			pAd->pktloss_chk.dest_port = param & 0x0FFFF;
			MTWF_PRINT("[%s] dest_port:%u\n",
				__func__, pAd->pktloss_chk.dest_port);
			break;
		case PKTLOSS_CHK_SET_OFFSET:
			pAd->pktloss_chk.byte_offset = param;
			MTWF_PRINT("[%s] byte_offset:%u\n",
				__func__, pAd->pktloss_chk.byte_offset);
			break;
		case PKTLOSS_CHK_SET_IS_SEQ_SIGNED:
			pAd->pktloss_chk.is_seq_signed = (param > 0) ? 1 : 0;
			MTWF_PRINT("[%s] is_seq_signed:%u\n",
				__func__, pAd->pktloss_chk.is_seq_signed);
			break;
		case PKTLOSS_CHK_SET_IS_SEQ_CROSS_ZERO:
			pAd->pktloss_chk.is_seq_cross_zero = (param > 0) ? 1 : 0;
			MTWF_PRINT("[%s] is_seq_cross_zero:%u\n",
				__func__, pAd->pktloss_chk.is_seq_cross_zero);
			break;
		case PKTLOSS_CHK_SET_SEQ_MASK:
			pAd->pktloss_chk.seq_mask = param;
			MTWF_PRINT("[%s] seq_mask:0x%08X\n",
				__func__, pAd->pktloss_chk.seq_mask);
			break;
		case PKTLOSS_CHK_SET_CTRL_FLAG:
			pAd->pktloss_chk.ctrl_flag = param;
			MTWF_PRINT("[%s] ctrl_flag:0x%08X\n",
				 __func__, pAd->pktloss_chk.ctrl_flag);
			break;
		case PKTLOSS_CHK_SET_TS_THRESHOLD:
			pAd->pktloss_chk.ts_threshold = param/(1000000/HZ);
			MTWF_PRINT("[%s] threshold:%d\n",
				__func__, pAd->pktloss_chk.ts_threshold);
			param = (param << 1)/60;
			break;
		case PKTLOSS_CHK_SET_ENABLE:
			pAd->pktloss_chk.enable = (param > 0) ? TRUE : FALSE;
			MTWF_PRINT("[%s] enable:%u\n",
				__func__, pAd->pktloss_chk.enable);
			break;
		case PKTLOSS_CHK_SET_RESET:
			pAd->pktloss_chk.enable = FALSE;
			os_zero_mem(&pAd->pktloss_chk, sizeof(pktloss_check_var_type));
			pAd->pktloss_chk.proto = 17;
			pAd->pktloss_chk.seq_mask = 0xFFFFFFFF;
			MTWF_PRINT("[%s] reset\n",
				__func__);
			break;
		case PKTLOSS_CHK_SET_DUMP_SHORT:
			pktloss_chk_dump(pAd, FALSE);
			break;
		case PKTLOSS_CHK_SET_HEX_DUMP:
			pAd->pktloss_chk.ctrl_flag |= (1 << PKTLOSS_CHK_HEX_DUMP);
			break;
		case PKTLOSS_CHK_SET_CONTINUE_HEX_DUMP:
			pAd->pktloss_chk.ctrl_flag |= (1 << PKTLOSS_CHK_CONTINUE_HEX_DUMP);
			break;
		case PKTLOSS_CHK_SET_TXS_LOG_ENABLE:
			pAd->pktloss_chk.txs_log_enable = (param > 0) ? TRUE : FALSE;
			MTWF_PRINT("[%s] txs_log_enable:%u\n",
				__func__, pAd->pktloss_chk.txs_log_enable);
			break;
		default:
			break;
		}
		MtCmdCr4Set(pAd, WA_SET_OPTION_PKTLOSS_CHK, cmd, param);
	}
#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support)
		MtUniCmdFwLog2Host(pAd, HOST2CR4, ENUM_CMD_FW_LOG_2_HOST_CTRL_2_UART);
	else
#endif /* WIFI_UNIFIED_COMMAND */
		MtCmdFwLog2Host(pAd, 1, 0);

	return TRUE;
}
#endif

#ifdef ANTENNA_CONTROL_SUPPORT
INT Antenna_Control_Init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct mcs_nss_caps *nss_cap = MCS_NSS_CAP(pAd);

	MTWF_PRINT("nss_cap->max_nss = %d, nss_cap->max_path =(tx:%d /rx:%d)!!\n",
			 nss_cap->max_nss, nss_cap->max_path[0], nss_cap->max_path[1]);

	if (pAd->TxStream && pAd->RxStream) {
		wlan_config_set_tx_stream(wdev, min(pAd->TxStream, nss_cap->max_nss));
		wlan_config_set_rx_stream(wdev, min(pAd->RxStream, nss_cap->max_nss));
		wlan_operate_set_tx_stream(wdev, min(pAd->TxStream, nss_cap->max_nss));
		wlan_operate_set_rx_stream(wdev, min(pAd->RxStream, nss_cap->max_nss));
#ifdef DOT11_HE_AX
		wlan_config_set_he_rx_nss(wdev, min(pAd->RxStream, nss_cap->max_nss));
		wlan_config_set_he_tx_nss(wdev, min(pAd->TxStream, nss_cap->max_nss));
#endif
		SetCommonHtVht(pAd, wdev);
	}
	return TRUE;
}

INT Set_Antenna_Control_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 BandIdx = 0, Txstream = 0, Rxstream = 0;
	struct wifi_dev *wdev = NULL;
	BSS_STRUCT *pMbss = NULL;

	struct mcs_nss_caps *nss_cap = MCS_NSS_CAP(pAd);
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */

	if (wdev == NULL) {
		MTWF_PRINT("%s: Incoorect BSS!!\n", __func__);
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	BandIdx = HcGetBandByWdev(wdev);
#endif

	if (arg == NULL) {
		MTWF_PRINT("No parameters!!\n");
		return FALSE;
	}

	MTWF_PRINT("Set Before:nss_cap->max_nss = %d, nss_cap->max_path =(tx:%d /rx:%d)!!\n",
			nss_cap->max_nss, nss_cap->max_path[0], nss_cap->max_path[1]);

	if (0 == simple_strtol(&arg[0], 0, 10)) {
		MTWF_PRINT(" Set default Antenna number!!\n");
		Txstream = Rxstream = nss_cap->max_nss;
		goto set_default;
	}

	if (strlen(arg) != 4) {
		MTWF_PRINT("Wrong parameter format!!\n");
		MTWF_PRINT("Please use input format like XTXR (X = 1,2) !!\n");
		return FALSE;
	}

	if (((arg[1] == 'T') || (arg[1] == 't')) && ((arg[3] == 'R') || (arg[3] == 'r'))) {
		Txstream = simple_strtol(&arg[0], 0, 10);
		Rxstream = simple_strtol(&arg[2], 0, 10);

		if (Txstream != Rxstream) {
			MTWF_PRINT("Wrong Input: Tx & Rx Antenna number different!\n");
			return FALSE;
		}

		if (Txstream > nss_cap->max_nss) {
			MTWF_PRINT("Wrong Input: BandIdx = %d, more than max ant cap (%d)!!\n",
					BandIdx, nss_cap->max_nss);
			return FALSE;
		}
	} else {
		MTWF_PRINT("Wrong Params, xTxR (x <= %d)!!\n",
				nss_cap->max_nss);
		return FALSE;
	}

set_default:
	pAd->TxStream = Txstream;
	pAd->RxStream = Rxstream;
	pAd->bAntennaSetAPEnable = 1;

	wlan_config_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
	wlan_config_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
	wlan_operate_set_tx_stream(wdev, min(Txstream, nss_cap->max_nss));
	wlan_operate_set_rx_stream(wdev, min(Rxstream, nss_cap->max_nss));
#ifdef DOT11_HE_AX
	wlan_config_set_he_tx_nss(wdev, min(Txstream, nss_cap->max_nss));
	wlan_config_set_he_rx_nss(wdev, min(Rxstream, nss_cap->max_nss));
#endif /* DOT11_HE_AX */

	SetCommonHtVht(pAd, wdev);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (pMbss != NULL) {
			APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
			APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
			return TRUE;
		} else
			return FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT Show_Antenna_Control_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct mcs_nss_caps *nss_cap = MCS_NSS_CAP(pAd);
	struct wifi_dev *wdev;

#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
#endif /* CONFIG_AP_SUPPORT */
	MTWF_PRINT("nss_cap->max_nss = %d, nss_cap->max_path =(tx:%d /rx:%d)!!\n",
		nss_cap->max_nss, nss_cap->max_path[0], nss_cap->max_path[1]);
	MTWF_PRINT("RF Antenna_Control Number:AntennaEnable=%d, Tx = %d, Rx = %d\n",
		pAd->bAntennaSetAPEnable,
		(pAd->TxStream) ? pAd->TxStream : wlan_operate_get_tx_stream(wdev),
		(pAd->RxStream) ? pAd->RxStream : wlan_operate_get_rx_stream(wdev));

	return TRUE;
}
#endif /* ANTENNA_CONTROL_SUPPORT */

#ifdef MAP_R2
INT show_traffic_separation_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	unsigned short vid = 0;
	int i = 0;

	MTWF_PRINT("ifname\town role\tpvid\tpcp\tSSID\tfh_vid\tvid_num\tall vid\n");
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		MTWF_PRINT("%s\t%02x\t%d\t%d\t%s\t%d\t%d ",
				 wdev->if_dev->name, wdev->MAPCfg.DevOwnRole, wdev->MAPCfg.primary_vid,
				 wdev->MAPCfg.primary_pcp, pAd->ApCfg.MBSSID[i].Ssid, wdev->MAPCfg.fh_vid,
				 wdev->MAPCfg.vid_num);

		for (vid = 1; vid < 4095; vid++) {
			if (is_vid_configed(vid, wdev->MAPCfg.vids)) {
				MTWF_PRINT("%d ", vid);
			}
		}

		MTWF_PRINT("\n\ttransparent vlan:");
		for (vid = 1; vid < 4095; vid++) {
			if (is_vid_configed(vid, wdev->MAPCfg.bitmap_trans_vlan)) {
				MTWF_PRINT(" %d", vid);
			}
		}

		MTWF_PRINT("\n");
	}

#ifdef CONFIG_STA_SUPPORT
	for (i = 0; i < pAd->MSTANum; i++) {
		wdev = &pAd->StaCfg[i].wdev;
		MTWF_PRINT("%s\t%02x\t%d\t%d\t%s\t%d\t%d ",
				 wdev->if_dev->name, wdev->MAPCfg.DevOwnRole, wdev->MAPCfg.primary_vid,
				 wdev->MAPCfg.primary_pcp, "N/A", wdev->MAPCfg.fh_vid,
				 wdev->MAPCfg.vid_num);
		for (vid = 1; vid < 4095; vid++) {
			if (is_vid_configed(vid, wdev->MAPCfg.vids)) {
				MTWF_PRINT("%d ", vid);
			}
		}
		MTWF_PRINT("\n");
	}
#endif /* CONFIG_STA_SUPPORT */

	return TRUE;
}
#endif

#if defined(IWCOMMAND_CFG80211_SUPPORT) || defined(CCN34_SPLIT_MAC_SUPPORT)
INT set_apmacaddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	RT_CfgSetMacAddress(pAd, arg, apidx, OPMODE_AP);
	MTWF_PRINT("ApMacAddress%d = "MACSTR"\n",
			 apidx, MAC2STR(pAd->ExtendMBssAddr[apidx-1]));
	return TRUE;
}

INT set_apclimacaddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;

	ifIndex = pObj->ioctl_if;

	RT_CfgSetMacAddress(pAd, arg, ifIndex, OPMODE_STA);
	MTWF_PRINT("ApCliMacAddress%d = "MACSTR"\n",
			ifIndex, MAC2STR(pAd->ApcliAddr[ifIndex]));

	COPY_MAC_ADDR(&pAd->StaCfg[ifIndex].wdev.if_addr, pAd->ApcliAddr[ifIndex]);
	return TRUE;
}
#endif /* IWCOMMAND_CFG80211_SUPPORT */

#if defined(RT_CFG80211_SUPPORT) && defined(DOT11_EHT_BE)
INT	mtk_cfg80211_mlo_preset_link_id(struct _RTMP_ADAPTER *pAd, UCHAR mlo_preset_link)
{
	if (mlo_preset_link >= MLD_LINK_MAX) {
		MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_ERROR,
				"invalid mlo_preset_link : %d\n", mlo_preset_link);
		return -EINVAL;
	}
	pAd->CommonCfg.mlo_preset_link_id = mlo_preset_link;
	MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_MLO, DBG_LVL_INFO,
	"mlo_preset_link is %d success!\n", mlo_preset_link);
	return 0;
}
#endif


#ifdef ANDLINK_FEATURE_SUPPORT
INT set_andlink_en_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	INT andlink_enable = 0;
	INT band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type == INT_MAIN) || (pobj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_PRINT("invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		band_idx = HcGetBandByWdev(wdev);

		andlink_enable = os_str_tol(arg, 0, 10);

		if (!andlink_enable)
			andlink_enable = FALSE;
		else
			andlink_enable = TRUE;

		if (pAd->CommonCfg.andlink_enable == andlink_enable) {
			/* No need to do anything, current and previos values are same */
			MTWF_PRINT("[%s](%d): current andlink_enable is %d in band %d\n",
			__func__, __LINE__, andlink_enable, band_idx);
			return TRUE;
		}

		pAd->CommonCfg.andlink_enable = andlink_enable;
		MTWF_PRINT("[%s](%d):andlink_enable is %d in band %d success! \n",
				__func__, __LINE__, andlink_enable, band_idx);
		return TRUE;
	}

	return FALSE;
}

INT set_andlink_ip_hostname_en_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg) {
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	INT andlink_ip_hostname_en = 0;
	INT band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type == INT_MAIN) || (pobj->ioctl_if_type == INT_MBSSID)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_PRINT("invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		band_idx = HcGetBandByWdev(wdev);

		andlink_ip_hostname_en = os_str_tol(arg, 0, 10);

		if (!andlink_ip_hostname_en)
			andlink_ip_hostname_en = FALSE;
		else
			andlink_ip_hostname_en = TRUE;

		if (pAd->CommonCfg.andlink_ip_hostname_en == andlink_ip_hostname_en) {
			/* No need to do anything, current and previos values are same */
			MTWF_PRINT("[%s](%d): current andlink_enable is %d in band %d\n",
				__func__, __LINE__, andlink_ip_hostname_en, band_idx);
			return TRUE;
		}

		pAd->CommonCfg.andlink_ip_hostname_en = andlink_ip_hostname_en;
		MTWF_PRINT("[%s](%d):andlink_ip_hostname_en is %d in band %d success! \n",
					__func__, __LINE__, andlink_ip_hostname_en, band_idx);
		return TRUE;
	}

	return FALSE;
}
#ifdef ANDLINK_V4_0
INT set_andlink_simple_val(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	INT simple_val = 0;

	if ((pobj->ioctl_if_type == INT_MAIN) ||
		(pobj->ioctl_if_type == INT_MBSSID) ||
		(pobj->ioctl_if_type == INT_APCLI)) {
		if (!VALID_MBSS(pAd, ifIndex)) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"invalid mbss id(%d)\n", ifIndex);
			return FALSE;
		}

		simple_val = os_str_tol(arg, 0, 10);

		if (pAd->CommonCfg.andlink_simple_val == simple_val) {
			/* No need to do anything, current and previos values are same */
			MTWF_PRINT("[%s](%d): current simple_val is %d\n",
				__func__, __LINE__, simple_val);
			return TRUE;
		}

		pAd->CommonCfg.andlink_simple_val = simple_val;
		MTWF_PRINT("[%s](%d):simple_val is %d ,success! \n",
				__func__, __LINE__, simple_val);
		return TRUE;
	}

	return FALSE;
}

INT set_andlink_uplink_period(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	RTMP_STRING *this_char = NULL;
	UINT period_time = 0, arg_idx = 0, band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type != INT_MAIN)
		&& (pobj->ioctl_if_type != INT_MBSSID)
		&& (pobj->ioctl_if_type != INT_APCLI)) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"[%s](%d):uplink  invalid type(%d),only MAIN MBSSID and apcli support.\n",
			__func__, __LINE__, pobj->ioctl_if_type);
		return FALSE;
	}

	if (pobj->ioctl_if_type == INT_APCLI)
		wdev = &pAd->StaCfg[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	band_idx = HcGetBandByWdev(wdev);

	while ((this_char = strsep((char **)&arg, "-")) != NULL) {
		arg_idx++;
		if (arg_idx-1 >= ANDLINK_IF_MAX) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"%s(): set uplink period more than the interface idx(%d)\n", __func__, arg_idx-1);
			return TRUE;
		}
		period_time = (UINT)os_str_tol(this_char, 0, 10);

		if (pAd->CommonCfg.andlink_uplink_rate_cfg[band_idx].period_time[arg_idx-1] == period_time) {
			/* No need to do anything, current and previos values are same */
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):uplink  current period_time[%d] is %d is same no need update.\n",
				__func__, __LINE__, arg_idx-1, period_time);
			continue;
		}

		pAd->CommonCfg.andlink_uplink_rate_cfg[band_idx].period_time[arg_idx-1] = period_time;
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):uplink period_time[%d] is %d ,set success!\n",
				__func__, __LINE__, arg_idx-1, period_time);
	}
	return TRUE;
}

INT set_andlink_uplink_sample(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	RTMP_STRING *this_char = NULL;
	UINT sample_time = 0, arg_idx = 0, band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type != INT_MAIN)
		&& (pobj->ioctl_if_type != INT_MBSSID)
		&& (pobj->ioctl_if_type != INT_APCLI)) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"[%s](%d): invalid type(%d),only MAIN MBSSID and apcli support.\n",
			__func__, __LINE__, pobj->ioctl_if_type);
		return FALSE;
	}

	if (pobj->ioctl_if_type == INT_APCLI)
		wdev = &pAd->StaCfg[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;

	band_idx = HcGetBandByWdev(wdev);

	while ((this_char = strsep((char **)&arg, "-")) != NULL) {
		arg_idx++;
		if (arg_idx-1 >= ANDLINK_IF_MAX) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"%s(): uplink set sta sample more than the interface idx(%d)\n", __func__, arg_idx-1);
			return TRUE;
		}
		sample_time = (UINT)os_str_tol(this_char, 0, 10);

		if (pAd->CommonCfg.andlink_uplink_rate_cfg[band_idx].sample_time[arg_idx-1] == sample_time) {
			/* No need to do anything, current and previos values are same */
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):uplink  current sample_time[%d] is %d is same no need update.\n",
				__func__, __LINE__, arg_idx-1, sample_time);
			continue;
		}

		pAd->CommonCfg.andlink_uplink_rate_cfg[band_idx].sample_time[arg_idx-1] = sample_time;
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):uplink sample_time[%d] is %d ,set success!\n",
				__func__, __LINE__, arg_idx-1, sample_time);
	}
	return TRUE;
}

INT set_andlink_sta_period(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	RTMP_STRING *this_char = NULL;
	UINT period_time = 0, arg_idx = 0, band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type != INT_MAIN) && (pobj->ioctl_if_type != INT_MBSSID)) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"[%s](%d): invalid type(%d),only MAIN MBSSID support.\n",
			__func__, __LINE__, pobj->ioctl_if_type);
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	band_idx = HcGetBandByWdev(wdev);

	while ((this_char = strsep((char **)&arg, "-")) != NULL) {
		arg_idx++;
		if (arg_idx-1 >= ANDLINK_IF_MAX) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"%s(): set sta sample more than the interface idx(%d)\n", __func__, arg_idx-1);
			return TRUE;
		}
		period_time = (UINT)os_str_tol(this_char, 0, 10);

		if (pAd->CommonCfg.andlink_sta_rate_cfg[band_idx].period_time[arg_idx-1] == period_time) {
			/* No need to do anything, current and previos values are same */
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):set sta current period_time[%d] is %d is same no need update.\n",
				__func__, __LINE__, arg_idx-1, period_time);
			continue;
		}

		pAd->CommonCfg.andlink_sta_rate_cfg[band_idx].period_time[arg_idx-1] = period_time;
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):set sta period_time[%d] is %d ,set success!\n",
				__func__, __LINE__, arg_idx-1, period_time);
	}
	return TRUE;
}


INT set_andlink_sta_sample(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	POS_COOKIE pobj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR ifIndex = pobj->ioctl_if;
	RTMP_STRING *this_char = NULL;
	UINT sample_time = 0, arg_idx = 0, band_idx = 0;
	struct wifi_dev *wdev = NULL;

	if ((pobj->ioctl_if_type != INT_MAIN) && (pobj->ioctl_if_type != INT_MBSSID)) {
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
		"[%s](%d): invalid type(%d),only MAIN MBSSID support.\n",
		__func__, __LINE__, pobj->ioctl_if_type);
		return FALSE;
	}

	wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	band_idx = HcGetBandByWdev(wdev);

	while ((this_char = strsep((char **)&arg, "-")) != NULL) {
		arg_idx++;
		if (arg_idx-1 >= ANDLINK_IF_MAX) {
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_ERROR,
			"%s(): set sta sample more than the interface idx(%d)\n", __func__, arg_idx-1);
			return TRUE;
		}
		sample_time = (UINT)os_str_tol(this_char, 0, 10);

		if (pAd->CommonCfg.andlink_sta_rate_cfg[band_idx].sample_time[arg_idx-1] == sample_time) {
			/* No need to do anything, current and previos values are same */
			MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):set sta current sample_time[%d] is %d is same no need update.\n",
				__func__, __LINE__, arg_idx-1, sample_time);
			continue;
		}

		pAd->CommonCfg.andlink_sta_rate_cfg[band_idx].sample_time[arg_idx-1] = sample_time;
		MTWF_DBG(pAd, DBG_CAT_ANDLINK, CATANDLINK_CFG, DBG_LVL_NOTICE,
				"[%s](%d):set sta sample_time[%d] is %d ,set success!\n",
				__func__, __LINE__, arg_idx-1, sample_time);
	}
	return TRUE;
}
#endif/*ANDLINK_V4_0*/

#endif/*ANDLINK_FEATURE_SUPPORT*/


#ifdef ACK_CTS_TIMEOUT_SUPPORT
INT set_ack_timeout_cr(RTMP_ADAPTER *pAd, UINT32 type, UINT32 timeout)
{
	UINT32 mac_val = 0;
	EXT_CMD_CFG_SET_ACK_CTS_T ExtCmdAckCtsfg = {0};
	POS_COOKIE pObj;
	struct wifi_dev *wdev;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP * cap = NULL;
#endif /* WIFI_UNIFIED_COMMAND */

	if (pAd == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd is NULL!!\n");
		return FALSE;
	}

	if (pAd->hdev_ctrl == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid hdev_ctrl: pAd->hdev_ctrl is NULL!!\n");
		return FALSE;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	if (pObj == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"pAd->OS_Cookie is NULL!!\n");
		return FALSE;
	}

	wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"Incoorect BSS!!\n");
		return FALSE;
	}

#ifdef WIFI_UNIFIED_COMMAND
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (IS_MT7915(pAd)) {
		MAC_IO_READ32(pAd->hdev_ctrl, type, &mac_val);
		mac_val &= ~MAX_ACK_TIMEOUT;
		mac_val |= (timeout & 0xFFFF);
		MTWF_PRINT("reg_addr = %x, mac_val= %x!!\n", type, mac_val);
		MAC_IO_WRITE32(pAd->hdev_ctrl, type, mac_val);
	} else {
		if (type >= WH_TX_ACK_CTS_TYPE_MAX)
			return FALSE;

		ExtCmdAckCtsfg.u1Type = (UINT8)type;
		ExtCmdAckCtsfg.u4TimeoutValue = timeout;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdCfgInfoUpdate(pAd, wdev, CFGINFO_ACK_CTS_FEATURE, &ExtCmdAckCtsfg);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			CmdExtCmdCfgUpdate(pAd, wdev, CFGINFO_ACK_CTS_FEATURE, &ExtCmdAckCtsfg);
		MTWF_PRINT("Value of %d(0: CCK_ACK_TOUT, 1: OFDM_ACK_TOUT, 2: OFDMA_ACK_TOUT) set = %d!!\n", ExtCmdAckCtsfg.u1Type, ExtCmdAckCtsfg.u4TimeoutValue);
	}
	return TRUE;
}

INT set_ack_timeout_mode_byband(
	RTMP_ADAPTER *pAd,
	UINT32 timeout,
	UINT32 bandidx,
	ACK_TIMEOUT_MODE_T ackmode)
{
	RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	INT ret;

	if ((timeout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range should between [0xFFFF:0]!!\n");
		return FALSE;
	}

	if (pAd->CommonCfg.ack_cts_enable == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"ERROR! BAND%u, ack_cts_enable=%u, CTS/ACK FEATURE is not enable!!\n",
			 bandidx, pAd->CommonCfg.ack_cts_enable);
		return FALSE;
	}

	if (chip_ops->set_ack_timeout_mode_byband)
		ret = chip_ops->set_ack_timeout_mode_byband(pAd, timeout, bandidx, ackmode);
	else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"ERROR! No handler for ack_cts timeout setting!!\n");
		ret = FALSE;
	}
	return ret;
}

INT32 set_cck_ofdm_ofdma_tout (RTMP_ADAPTER *pAd, UINT32 timeout, ACK_TIMEOUT_MODE_T ack_mode)
{
	struct wifi_dev *wdev = NULL;
	UCHAR band_idx = 0;
	POS_COOKIE pObj = NULL;

	if (NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p;!!\n", pAd);
		return FALSE;
	}

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	if (NULL == pObj) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid pObj: pObj=%p!!\n",
			pObj);
		return FALSE;
	}

	if (((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		&& (pObj->ioctl_if < MAX_BEACON_NUM)) {
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid interafce! or interface idx: %d!!\n",
			pObj->ioctl_if);
		return FALSE;
	}

	/*GET BANDINDX*/
	band_idx = HcGetBandByWdev(wdev);

	if (FALSE == set_ack_timeout_mode_byband(pAd, timeout, band_idx, ack_mode)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return FALSE;
	}
	return TRUE;
}


INT32 set_datcfg_ack_cts_timeout (RTMP_ADAPTER *pAd)
{
	UCHAR idx = 0;
	UINT32 value = 0;

	if (NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p;!!\n",
			pAd);
		return FALSE;
	}

	idx = hc_get_hw_band_idx(pAd);

	if (pAd->CommonCfg.distance > 0) {
		value = pAd->CommonCfg.distance;
		value = value*2/LIGHT_SPEED;

		if (TRUE != set_ack_timeout_mode_byband(pAd,
			value, idx, ACK_ALL_TIME_OUT)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"DAT config band(%u) Distance Fail!\n",
			 idx);
		}
	} else {
		if (pAd->CommonCfg.cck_timeout > 0) {
			value = pAd->CommonCfg.cck_timeout;
			if (TRUE != set_ack_timeout_mode_byband(pAd,
				value, idx, CCK_TIME_OUT)) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"DAT config band(%u) cck_timeout Fail!\n",
				idx);
			}
		}

		if (pAd->CommonCfg.ofdm_timeout > 0) {
			value = pAd->CommonCfg.ofdm_timeout;

			if (TRUE != set_ack_timeout_mode_byband(pAd,
				value, idx, OFDM_TIME_OUT)) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"DAT config band(%u) ofdm_timeout Fail!\n",
				idx);
			}
		}

		if (pAd->CommonCfg.ofdma_timeout > 0) {
			value = pAd->CommonCfg.ofdma_timeout;

			if (TRUE != set_ack_timeout_mode_byband(pAd,
				value, idx, OFDMA_TIME_OUT)) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"DAT config band(%u) ofdma_timeout Fail!\n",
				idx);
			}
		}
	}

	return TRUE;

}


INT set_dst2acktimeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 bTimeout = 0, distance = 0;

	if (arg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"No parameters!!\n");
		return FALSE;
	}

	/* Get distance and store in wdev*/
	distance = os_str_tol(arg, 0, 10);

	/* Calculate, timeout=((distance/speed of light)*2) */
	bTimeout = distance/LIGHT_SPEED;
	bTimeout *= 2;

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range should between [0xFFFF:0)!!\n");
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, ACK_ALL_TIME_OUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"SET CTS/ACK Timeout SUCCESS!!\n");
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_dst2acktimeout(RTMP_ADAPTER *pAd, UINT distance)
{
	UINT32 bTimeout = 0;
	UCHAR band_idx = hc_get_hw_band_idx(pAd);

	bTimeout = distance / LIGHT_SPEED;
	bTimeout *= 2;
	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range should between [0xFFFF:0)!!\n");
		return -EINVAL;
	}

	if (set_ack_timeout_mode_byband(pAd, bTimeout, band_idx, ACK_ALL_TIME_OUT) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return -EFAULT;
	}
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"SET CTS/ACK Timeout SUCCESS!!\n");
	return 0;
}
#endif

INT set_ackcts_timeout_enable_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	UCHAR band_idx = 0;
	UCHAR enable = 0;
	POS_COOKIE pObj = NULL;

	if (NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p;!!\n",
			pAd);
		return FALSE;
	}
	pObj = (POS_COOKIE)pAd->OS_Cookie;
	if (NULL == pObj) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid pObj: pObj=%p;!!\n",
			 pObj);
		return FALSE;
	}

	if (((pObj->ioctl_if_type == INT_MBSSID) || (pObj->ioctl_if_type == INT_MAIN))
		&& (pObj->ioctl_if < MAX_BEACON_NUM)) {
			wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			" invalid interafce! or interface idx: %d!!\n",
			pObj->ioctl_if);
		return FALSE;
	}

	/*GET BANDINDX*/
	band_idx = HcGetBandByWdev(wdev);
	enable = os_str_tol(arg, 0, 10);

	pAd->CommonCfg.ack_cts_enable = (enable == 0 ? FALSE : TRUE);

	MTWF_PRINT("ACK_CTS_TOUT_ENABLE: %s\n", enable == 0 ? "DISABLE" : "ENABLE");
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ackcts_tout_en(RTMP_ADAPTER *pAd, UCHAR ackcts_tout_en)
{
	if (pAd) {
		pAd->CommonCfg.ack_cts_enable = (ackcts_tout_en == 0 ? FALSE : TRUE);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"ACK_CTS_TOUT: %s\n",
			ackcts_tout_en == 0 ? "DISABLE" : "ENABLE");
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p;!!\n", pAd);
		return -EFAULT;
	}
	return 0;
}
#endif

INT set_cck_ack_timeout_porc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 bTimeout = 0;

	if (NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p!!\n",
			pAd);
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range [0xFFFF:0)!!\n");
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, CCK_TIME_OUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"SET CCK CTS/ACK Timeout SUCCESS!!\n");
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_cck_ack_tout(RTMP_ADAPTER *pAd, UINT cck_ack_tout)
{
	UCHAR band_idx = hc_get_hw_band_idx(pAd);

	if ((cck_ack_tout <= 0) || (cck_ack_tout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range [0xFFFF:0)!!\n");
		return -EINVAL;
	}
	if (set_ack_timeout_mode_byband(pAd, cck_ack_tout, band_idx, CCK_TIME_OUT) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return -EFAULT;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"SET CCK CTS/ACK Timeout SUCCESS!!\n");
	return 0;
}
#endif

INT set_ofdm_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 bTimeout = 0;

	if (NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p!!\n",
			pAd);
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range [0xFFFF:0)!!\n");
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, OFDM_TIME_OUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"SET OFDM CTS/ACK Timeout SUCCESS!!\n");
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ofdm_ack_tout(RTMP_ADAPTER *pAd, UINT ofdm_ack_tout)
{
	UCHAR band_idx = hc_get_hw_band_idx(pAd);

	if ((ofdm_ack_tout <= 0) || (ofdm_ack_tout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range [0xFFFF:0)!!\n");
		return -EINVAL;
	}
	if (set_ack_timeout_mode_byband(pAd, ofdm_ack_tout, band_idx, OFDM_TIME_OUT) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return -EFAULT;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"SET OFDM CTS/ACK Timeout SUCCESS!!\n");
	return 0;
}
#endif

INT set_ofdma_ack_timeout_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32  bTimeout = 0;

	if (NULL == pAd) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid null input: pAd=%p!!\n",
			pAd);
		return FALSE;
	}

	bTimeout = os_str_tol(arg, 0, 10);

	if ((bTimeout <= 0) || (bTimeout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range [0xFFFF:0)!!\n");
		return FALSE;
	}

	if (FALSE == set_cck_ofdm_ofdma_tout(pAd, bTimeout, OFDMA_TIME_OUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return FALSE;
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"SET OFDMA CTS/ACK Timeout SUCCESS!!\n");
	return TRUE;
}

#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_ofdma_ack_tout(RTMP_ADAPTER *pAd, UINT ofdma_ack_tout)
{
	UCHAR band_idx = hc_get_hw_band_idx(pAd);

	if ((ofdma_ack_tout <= 0) || (ofdma_ack_tout > MAX_ACK_TIMEOUT)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"CTS/ACK Timeout Range [0xFFFF:0)!!\n");
		return -EINVAL;
	}
	if (set_ack_timeout_mode_byband(pAd, ofdma_ack_tout, band_idx, OFDMA_TIME_OUT) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"SET CTS/ACK Timeout Fail!!\n");
		return -EFAULT;
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_NOTICE,
			"SET OFDMA CTS/ACK Timeout SUCCESS!!\n");
	return 0;
}
#endif
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

INT SetRtsThenCtsRetryCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4WlanIdx;
	UINT32 u4Tid;
	UINT32 u4RtsFailThenCtsRetryCnt;
	INT32  i4Recv = 0;

#ifdef WIFI_UNIFIED_COMMAND
	struct UNI_CMD_MURU_CTS_RETRY_CTRL_T rUniCtsRetryCtrl;
#endif

	if (arg) {
		i4Recv = sscanf(arg, "%u-%u-%u", &u4WlanIdx, &u4Tid, &u4RtsFailThenCtsRetryCnt);
		if (i4Recv != 3) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR, "Format Error!\n");
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"rts2cts=[wcid]-[tid]-[cnt]\n");
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"\t[tid]=0-7\n");
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"\t[cnt]=xx, Change proctection to CTS2SELF after [cnt] RTS failed\n");
			return TRUE;
		}
		if (!VALID_UCAST_ENTRY_WCID(pAd, u4WlanIdx)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"WCID exceed pAd->MaxUcastEntryNum!\n");
			return TRUE;
		}
		if (u4Tid > 7) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR, "Tid %u > 7\n", u4Tid);
			return TRUE;
		}
		if (u4RtsFailThenCtsRetryCnt > 31) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"u1RtsFailThenCtsRetryCnt %u > 31(max mpdu retry)\n",
				u4RtsFailThenCtsRetryCnt);
			return TRUE;

		}

#ifndef WIFI_UNIFIED_COMMAND
		CmdExtRtsThenCtsRetryCnt(pAd, (UINT16)u4WlanIdx, (UINT8)u4Tid, (UINT8)u4RtsFailThenCtsRetryCnt);
#else
		memset(&rUniCtsRetryCtrl, 0x00, sizeof(struct UNI_CMD_MURU_CTS_RETRY_CTRL_T));
		rUniCtsRetryCtrl.u1Tid = (UINT8)u4Tid;
		rUniCtsRetryCtrl.u1RtsFailThenCtsRetryCnt = (UINT8)u4RtsFailThenCtsRetryCnt;
		rUniCtsRetryCtrl.u2Wcid = (UINT16)u4WlanIdx;

		UniCmdMuruParameterSet(pAd, (RTMP_STRING *)&rUniCtsRetryCtrl, UNI_CMD_MURU_CTS_RETRY_CTRL);
#endif
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"WCID[%u], TID[%u], change to CTS2SELF after [%u] RTS failed\n",
			u4WlanIdx, u4Tid, u4RtsFailThenCtsRetryCnt);
	}

	return TRUE;
}

#ifdef MLME_MULTI_QUEUE_SUPPORT
INT set_mlme_queue_ration(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *str1  = NULL;
	RTMP_STRING *str2  = NULL;
	UCHAR hp_q_ration;
	UCHAR np_q_ration;
	UCHAR lp_q_ration;
	if (arg == NULL || strlen(arg) == 0)
		goto error;
	str1 = strsep(&arg, "-");
	str2 = strsep(&arg, "-");
	if (str1 == NULL || str2 == NULL ||  arg == NULL)
		goto error;
	hp_q_ration = os_str_tol(str1, 0, 10);
	np_q_ration = os_str_tol(str2, 0, 10);
	lp_q_ration = os_str_tol(arg, 0, 10);
	pAd->Mlme.HPQueue.Ration = hp_q_ration;
	pAd->Mlme.Queue.Ration = np_q_ration;
	pAd->Mlme.LPQueue.Ration = lp_q_ration;
	MTWF_PRINT("%s(): [hp_q_ration]-[np_q_ration]-[lp_q_ration] = %d-%d-%d\n",
		__func__, hp_q_ration, np_q_ration, lp_q_ration);
	return TRUE;
error:
	MTWF_PRINT("Invalid parameter, format: [hp_q_ration]-[np_q_ration]-[lp_q_ration]\n");
	return FALSE;
}
#endif /*MLME_MULTI_QUEUE_SUPPORT*/

#ifdef CFG_SUPPORT_CSI

int wlanCheckCSISegmentData(RTMP_ADAPTER *pAd, struct CSI_MAX_DATA_T *prCSIData)
{
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;
	struct CSI_MAX_DATA_T *prCSISegmentTemp;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
		"Segment number=%d, Remain last=%d\n",
		prCSIData->u4SegmentNum, prCSIData->ucRemainLast);

	if (prCSIData->u4SegmentNum == 0 &&
		prCSIData->ucRemainLast == 0)	/*case 1: csi complete chain data, push directly*/
		return CSI_CHAIN_COMPLETE;

	if (prCSIData->u4SegmentNum == 0 &&
		prCSIData->ucRemainLast == 1) {		/*case 2: the first segment*/
		os_move_mem(&(prCSIInfo->rCSISegmentTemp),
				prCSIData, sizeof(struct CSI_MAX_DATA_T));

		return CSI_CHAIN_SEGMENT_FIRST;
	} else if (prCSIData->u4SegmentNum != 0) {		/*case 3: the left segment*/
		prCSISegmentTemp = &(prCSIInfo->rCSISegmentTemp);
		if (prCSIData->chain_info !=
			prCSISegmentTemp->chain_info ||
			prCSIData->u4SegmentNum !=
			(prCSISegmentTemp->u4SegmentNum + 1)) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"H_IDX [%d/%d] is different or SEG_NUM [%d/%d] is not sequential.\n",
				prCSISegmentTemp->chain_info,
				prCSIData->chain_info,
				prCSISegmentTemp->u4SegmentNum,
				prCSIData->u4SegmentNum);

			return CSI_CHAIN_SEGMENT_ERR;
		}

		os_move_mem(&prCSISegmentTemp->ac2IData[
				prCSISegmentTemp->u2DataCount],
			prCSIData->ac2IData,
			prCSIData->u2DataCount * sizeof(INT_16));

		os_move_mem(&prCSISegmentTemp->ac2QData[
				prCSISegmentTemp->u2DataCount],
			prCSIData->ac2QData,
			prCSIData->u2DataCount * sizeof(INT_16));

		prCSISegmentTemp->u2DataCount += prCSIData->u2DataCount;
		prCSISegmentTemp->u4SegmentNum = prCSIData->u4SegmentNum;
		prCSISegmentTemp->ucRemainLast = prCSIData->ucRemainLast;

		if (prCSIData->ucRemainLast == 0)
			return CSI_CHAIN_SEGMENT_LAST;
		else if (prCSIData->ucRemainLast == 1)
			return CSI_CHAIN_SEGMENT_MIDDLE;
	}

	return CSI_CHAIN_ERR;

}

struct CSI_DATA_T *prepare_csi_data_to_push(RTMP_ADAPTER *pAd, struct CSI_MAX_DATA_T *csi_data)
{

	struct CSI_DATA_T *final_data = NULL;
	u16 csi_tone_cnt;
	INT_16 *data_i = NULL, *data_q = NULL;

	/*realloc a new csi data common part*/
	os_alloc_mem(NULL, (UCHAR **)&final_data, sizeof(struct CSI_DATA_T));

	if (!final_data) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"allocate memory for final csi data failed!\n");
		return NULL;
	}

	os_zero_mem(final_data, sizeof(struct CSI_DATA_T));

	/*check the actual csi tone cnt*/
	if (csi_data->ucDataBw == RX_VT_FR_MODE_20) {
		csi_tone_cnt = CSI_BW20_TONE_NUM;
		final_data->u2DataCount = csi_tone_cnt;
	} else if (csi_data->ucDataBw == RX_VT_FR_MODE_40) {
		csi_tone_cnt = CSI_BW40_TONE_NUM;
		final_data->u2DataCount = csi_tone_cnt;
	} else if (csi_data->ucDataBw >= RX_VT_FR_MODE_80)
		csi_tone_cnt = csi_data->u2DataCount;
	else {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"current csi tone cnt error!\n");
		return NULL;
	}

	/*realloc I data*/
	os_alloc_mem(NULL, (UCHAR **)&data_i, sizeof(INT_16) * csi_tone_cnt);

	if (!data_i) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"allocate memory for I data failed!\n");
		os_free_mem(final_data);
		return NULL;
	}

	final_data->ac2IData = data_i;


	/*realloc Q data*/
	os_alloc_mem(NULL, (UCHAR **)&data_q, sizeof(INT_16) * csi_tone_cnt);

	if (!data_q) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"allocate memory for Q data failed!\n");
		os_free_mem(data_i);
		os_free_mem(final_data);
		return NULL;
	}

	final_data->ac2QData = data_q;

	os_zero_mem(data_i, sizeof(INT_16) * csi_tone_cnt);
	os_zero_mem(data_q, sizeof(INT_16) * csi_tone_cnt);

	os_move_mem(final_data, csi_data, Offsetof(struct CSI_MAX_DATA_T, ac2IData));
	os_move_mem(data_i, csi_data->ac2IData, sizeof(INT_16) * csi_tone_cnt);
	os_move_mem(data_q, csi_data->ac2QData, sizeof(INT_16) * csi_tone_cnt);

	return final_data;
}

int csi_timestamp_filter(RTMP_ADAPTER *pAd, struct CSI_DATA_T *csi_data)
{
	UINT32 expected_TS = 0;
	UINT8 need_push = 0;
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;

	prCSIInfo->ExpTs_offset = prCSIInfo->usr_offset/CSI_TS_FILTER_RATIO;

	/*	|-------------------usr_offset--------------|*/
	/*	--------------------------|--ExpTs_offset---|---ExpTs_offset---|*/
	/*	|-----------(1)-----------|------(2)--------|--------(3)-------|-----(4)----*/
	/*cur_TS(t0)			t1-offset*1/10		expected TS(t1)   t1+offset*1/10*/

	/*the first pkt or chain from the same data, directly push*/
	if (prCSIInfo->cur_TS == 0 || prCSIInfo->cur_TS == csi_data->u4TimeStamp)
		need_push = 1;
	else {	/*for case(1), drop; case (2)(3)(4), push*/
		expected_TS = prCSIInfo->cur_TS + prCSIInfo->usr_offset;
		if (csi_data->u4TimeStamp < (expected_TS - (UINT32)(prCSIInfo->usr_offset/CSI_TS_FILTER_RATIO)))
			need_push = 0;
		else
			need_push = 1;
	}

	if (need_push) {
		wlanPushCSIData(pAd, csi_data);
		prCSIInfo->cur_TS = csi_data->u4TimeStamp;
		return 0;
	}

	return -1;
}

bool wlanPushCSIData(RTMP_ADAPTER *pAd, struct CSI_DATA_T *prCSIData)
{
	struct CSI_INFO_T *prCSIInfo = &(pAd->rCSIInfo);

	if (prCSIInfo->csi_wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
		"CSI config error, need check!\n");
		/*if push err, free the IQ data*/
		if (prCSIData->ac2IData)
			os_free_mem(prCSIData->ac2IData);
		if (prCSIData->ac2QData)
			os_free_mem(prCSIData->ac2QData);
		return FALSE;
	}

	NdisAcquireSpinLock(&prCSIInfo->CSIBufferLock);

	/* Put the CSI data into CSI event queue */
	if (prCSIInfo->u4CSIBufferUsed != 0) {
		prCSIInfo->u4CSIBufferTail++;
		prCSIInfo->u4CSIBufferTail %= CSI_RING_SIZE;
	}

	if (prCSIInfo->u4CSIBufferUsed < CSI_RING_SIZE) {
		prCSIInfo->u4CSIBufferUsed++;
	} else {
		/*
		 * While new CSI event comes and the ring buffer is
		 * already full, the new coming CSI event will
		 * overwrite the oldest one in the ring buffer.
		 * Thus, the Head pointer which points to * the
		 * oldest CSI event in the buffer should be moved too.
		 */

		/*free and clear the old one, before the array idx shift*/
		os_free_mem(prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferHead].ac2IData);
		os_free_mem(prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferHead].ac2QData);
		os_zero_mem(&prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferHead], sizeof(struct CSI_DATA_T));

		prCSIInfo->u4CSIBufferHead++;
		prCSIInfo->u4CSIBufferHead %= CSI_RING_SIZE;
	}

	os_move_mem(&(prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferTail]),
		prCSIData, sizeof(struct CSI_DATA_T));

	NdisReleaseSpinLock(&prCSIInfo->CSIBufferLock);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
	": CSIBufferUsed=%d, CSIBufferHead=%d, CSIBufferTail=%d\n",
	prCSIInfo->u4CSIBufferUsed, prCSIInfo->u4CSIBufferHead, prCSIInfo->u4CSIBufferTail);
	return TRUE;
}

bool wlanPopCSIData(RTMP_ADAPTER *pAd, struct CSI_DATA_T *prCSIData)
{
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;
	NdisAcquireSpinLock(&prCSIInfo->CSIBufferLock);

	/*No CSI data in the ring buffer*/
	if (prCSIInfo->u4CSIBufferUsed == 0) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"CSI Buffer is empty!\n");
		NdisReleaseSpinLock(&prCSIInfo->CSIBufferLock);
		return FALSE;
	}

	os_move_mem(prCSIData,
		&(prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferHead]),
		sizeof(struct CSI_DATA_T));

	os_zero_mem(&prCSIInfo->arCSIBuffer[prCSIInfo->u4CSIBufferHead], sizeof(struct CSI_DATA_T));

	prCSIInfo->u4CSIBufferUsed--;
	if (prCSIInfo->u4CSIBufferUsed != 0) {
		prCSIInfo->u4CSIBufferHead++;
		prCSIInfo->u4CSIBufferHead %= CSI_RING_SIZE;
	}
	NdisReleaseSpinLock(&prCSIInfo->CSIBufferLock);
	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
		": CSIBufferUsed=%d, CSIBufferHead=%d, CSIBufferTail=%d\n",
		prCSIInfo->u4CSIBufferUsed, prCSIInfo->u4CSIBufferHead, prCSIInfo->u4CSIBufferTail);

	return TRUE;
}


void csi_clean_ring_buffer(RTMP_ADAPTER *pAd)
{
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;
	int i;

	NdisAcquireSpinLock(&prCSIInfo->CSIBufferLock);

	for (i = 0; i < CSI_RING_SIZE; i++) {
		if (pAd->rCSIInfo.arCSIBuffer[i].ac2IData)
			os_free_mem(pAd->rCSIInfo.arCSIBuffer[i].ac2IData);
		if (pAd->rCSIInfo.arCSIBuffer[i].ac2QData)
			os_free_mem(pAd->rCSIInfo.arCSIBuffer[i].ac2QData);
	}

	os_zero_mem(pAd->rCSIInfo.arCSIBuffer, CSI_RING_SIZE * sizeof(struct CSI_DATA_T));

	/*clear tmp info*/
	prCSIInfo->u4CSIBufferHead = 0;
	prCSIInfo->u4CSIBufferTail = 0;
	prCSIInfo->u4CSIBufferUsed = 0;
	prCSIInfo->bIncomplete = FALSE;
	prCSIInfo->u4CopiedDataSize = 0;
	prCSIInfo->u4RemainingDataSize = 0;

	NdisReleaseSpinLock(&prCSIInfo->CSIBufferLock);

}

#endif

#ifdef INTERFACE_SPEED_DETECT
BOOLEAN set_interface_speed(RTMP_ADAPTER *pAd, UINT32 speed)
{
	CMD_MEC_CTRL_CMD_T mecCtrlCmd = {0};
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hwifi_get_chip_cap(pAd);
	struct UNI_CMD_MEC_CTRL_PARAM_T UniCmdMecParam;
#endif /* WIFI_UNIFIED_COMMAND */

#ifdef WIFI_UNIFIED_COMMAND
	if (cap->uni_cmd_support) {
		os_zero_mem(&UniCmdMecParam, sizeof(UniCmdMecParam));

		UniCmdMecParam.mec_ifac_speed.u2Tag = cpu2le16(UNI_CMD_MEC_IFAC_SPEED);
		UniCmdMecParam.mec_ifac_speed.u2Length = cpu2le16(sizeof(UniCmdMecParam.mec_ifac_speed));
		UniCmdMecParam.mec_ifac_speed.u4IfacSpeed = cpu2le32(speed);
		UniCmdMecParam.MecTagValid[UNI_CMD_MEC_IFAC_SPEED] = TRUE;

		if (UniCmdMecCtrl(pAd, &UniCmdMecParam) == NDIS_STATUS_SUCCESS)
			return TRUE;
		else
			return FALSE;
	} else
#endif /* WIFI_UNIFIED_COMMAND */
	{
		os_zero_mem(&mecCtrlCmd, sizeof(mecCtrlCmd));
		mecCtrlCmd.u2Action = MEC_CTRL_INTF_SPEED;

		mecCtrlCmd.mecCmdPara.mec_ifac_speed.u4InterfacSpeed = speed;

		if (CmdMecCtrl(pAd, (PUINT8)&mecCtrlCmd) == 0)
			return TRUE;
		else
			return FALSE;
	}
}
#endif /* INTERFACE_SPEED_DETECT */
#ifdef RT_CFG80211_SUPPORT
INT mtk_cfg80211_set_txpwr_info(RTMP_ADAPTER *pAd, UCHAR ucTxPowerInfoCatg)
{
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR  IfIdx = 0;

	IfIdx = pObj->ioctl_if;
	/* Grab Band index */
	if (pObj->ioctl_if_type == INT_MBSSID || pObj->ioctl_if_type == INT_MAIN) {
#ifdef CONFIG_AP_SUPPORT
		wdev = &pAd->ApCfg.MBSSID[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
			"AP Band = %d !!\n", ucBandIdx);

#endif
	}
#ifdef CONFIG_STA_SUPPORT
	else if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
			"STA Band = %d !!\n", ucBandIdx);
	} else if (pObj->ioctl_if_type == INT_MSTA) {
		wdev = &pAd->StaCfg[IfIdx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_NOTICE,
			"STA Band = %d !!\n", ucBandIdx);
		}
#endif
	else {
		MTWF_PRINT("[SetTxPowerInfo]: pObj->ioctl_if_type = %d!!\n", pObj->ioctl_if_type);
		return FALSE;
	}
	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_PRINT("Invalid Band Index!!\n");
		goto error;
	}
	/* Sanity check for parameter range */
	if (ucTxPowerInfoCatg >= POWER_INFO_NUM) {
		MTWF_PRINT("parameter out of range!!\n");
		goto error;
	}
	MTWF_PRINT("%s: BandIdx: %d, ucTxPowerInfoCatg: %d\n",
		__func__, ucBandIdx, ucTxPowerInfoCatg);
	g_fgCommand = TRUE;
	return TxPowerShowInfo(pAd, ucTxPowerInfoCatg, ucBandIdx);
error:
	return FALSE;
}
INT mtk_cfg80211_set_percentage_ctrl(RTMP_ADAPTER *pAd, UCHAR fgTxPowerPercentEn)
{
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, ucBandIdx);
	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;
	/* sanity check for input parameter */
	if ((fgTxPowerPercentEn != FALSE) && (fgTxPowerPercentEn != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}
	MTWF_PRINT("%s: TxPowerPercentEn = %d\n", __func__, fgTxPowerPercentEn);
	/* Update Profile Info for Power Percentage */
	pAd->CommonCfg.PERCENTAGEenable = fgTxPowerPercentEn;
#ifdef CONFIG_ATE
	/* Update Power Percentage Status in ATECTRL Structure */
	ATECtrl->tx_pwr_percentage_en = fgTxPowerPercentEn;
#endif /* CONFIG_ATE */
	return TxPowerPercentCtrl(pAd, fgTxPowerPercentEn, ucBandIdx);
}
INT mtk_cfg80211_set_pwr_drop_ctrl(RTMP_ADAPTER *pAd, UCHAR ucPowerDrop)
{
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef MGMT_TXPWR_CTRL
	struct _BSS_STRUCT *pMbss = NULL;
	INT status = FALSE;
	UCHAR i = 0;
#endif
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */

	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;
	MTWF_PRINT("%s: ucPowerDrop = %d\n", __func__, ucPowerDrop);
	/* Update Profile Info for Power Percentage Drop Value */
	pAd->CommonCfg.ucTxPowerPercentage = ucPowerDrop;
#ifdef CONFIG_ATE
	/* Update Power Percentage Drop Value Status in ATECTRL Structure */
	ATECtrl->tx_pwr_percentage_level = ucPowerDrop;
#endif /* CONFIG_ATE */
#ifdef MGMT_TXPWR_CTRL
	status = TxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx);
	if (!status) {
		MTWF_PRINT("ERROR: TxPowerDropCtrl() FAILED!\n ");
		return FALSE;
	}
	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pMbss = &pAd->ApCfg.MBSSID[i];
		wdev = &pMbss->wdev;
		if (HcGetBandByWdev(wdev) != ucBandIdx)
			continue;

		/*clear old data for iwpriv set channel*/
		wdev->bPwrCtrlEn = FALSE;
		wdev->TxPwrDelta = 0;
		wdev->mgmt_txd_txpwr_offset = 0;
		/* Get EPA info by Tx Power info cmd*/
		pAd->ApCfg.MgmtTxPwr = 0;
		MtCmdTxPwrShowInfo(pAd, TXPOWER_ALL_RATE_POWER_INFO, ucBandIdx);

		/* Update beacon/probe TxPwr wrt profile param */
		if (wdev->MgmtTxPwr) {
			/* wait until TX Pwr event rx*/
			RtmpusecDelay(50);
			update_mgmt_frame_power(pAd, wdev);
		}

	}
	return status;
#else
	return TxPowerDropCtrl(pAd, ucPowerDrop, ucBandIdx);
#endif
}
INT mtk_cfg80211_set_decrease_pwrctrl(RTMP_ADAPTER *pAd, UCHAR cPowerDropLevel)
{
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
	BOOLEAN  fgStatus = FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR       apidx = pObj->ioctl_if;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;
	MTWF_PRINT("%s: cPowerDropLevel (0.5 dBm) = %d\n", __func__, cPowerDropLevel);
	if (MtCmdTxPowerDropCtrl(pAd, cPowerDropLevel, ucBandIdx) == 0)
		fgStatus = TRUE;
	return fgStatus;
}
INT mtk_cfg80211_set_sku_ctrl(RTMP_ADAPTER *pAd, UCHAR tx_pwr_sku_en)
{
	UINT8   ucBandIdx = 0;
	struct  wifi_dev *wdev;
#ifdef CONFIG_ATE
	struct _ATE_CTRL  *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
		UCHAR	   apidx = pObj->ioctl_if;

		/* obtain Band index */
		if (apidx >= pAd->ApCfg.BssidNum)
			return FALSE;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		wdev = &pAd->StaCfg[0].wdev;
		ucBandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, ucBandIdx);
	/* sanity check for Band index */
	if (ucBandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;
	/* sanity check for input parameter */
	if ((tx_pwr_sku_en != FALSE) && (tx_pwr_sku_en != TRUE)) {
		MTWF_PRINT("Please input 1(Enable) or 0(Disable)!!\n");
		return FALSE;
	}
	MTWF_PRINT("%s: TxPowerSKUEn: %d\n", __func__, tx_pwr_sku_en);
	/* Update Profile Info for SKU */
#ifdef SINGLE_SKU_V2
	pAd->CommonCfg.SKUenable = tx_pwr_sku_en;
#endif /* SINGLE_SKU_V2 */
#ifdef CONFIG_ATE
	/* Update SKU Status in ATECTRL Structure */
	ATECtrl->tx_pwr_sku_en = tx_pwr_sku_en;
#endif /* CONFIG_ATE */
#ifdef SINGLE_SKU_V2
	return TxPowerSKUCtrl(pAd, tx_pwr_sku_en, ucBandIdx);
#else
	return TRUE;
#endif /* SINGLE_SKU_V2 */
}
INT mtk_cfg80211_set_sku_info(RTMP_ADAPTER *pAd)
{
	INT status = TRUE;
#ifdef SINGLE_SKU_V2
	/* print out Sku table info */
	if (MtShowPwrLimitTable(pAd, POWER_LIMIT_TABLE_TYPE_SKU, 0) != NDIS_STATUS_SUCCESS)
		status = SKU_FAIL;
#endif /* SINGLE_SKU_V2 */
	return status;
}
INT mtk_cfg80211_set_mutxpwr(RTMP_ADAPTER *pAd, UCHAR fgMuTxPwrManEn, UCHAR cMuTxPwr)
{
	UINT8    u1BandIdx = 0;
	struct  wifi_dev *wdev;
#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_AP_SUPPORT)
	UCHAR	  apidx = pObj->ioctl_if;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

	/* obtain Band index */
	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;

	if ((pObj->ioctl_if_type == INT_MAIN) || (pObj->ioctl_if_type == INT_MBSSID)) {
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI) {
		wdev = &pAd->StaCfg[0].wdev;
		u1BandIdx = HcGetBandByWdev(wdev);
	}
#endif /* CONFIG_STA_SUPPORT */
	MTWF_PRINT("%s: ucBandIdx = %d\n", __func__, u1BandIdx);
	/* sanity check for Band index */
	if (u1BandIdx >= CFG_WIFI_RAM_BAND_NUM)
		return FALSE;
	MTWF_PRINT("%s: fgMuTxPwrManEn: %d, cMuTxPwr: %d\n", __func__,
			fgMuTxPwrManEn, cMuTxPwr);
	return MuPwrCtrlCmd(pAd, fgMuTxPwrManEn, cMuTxPwr, u1BandIdx);
}
#ifdef MGMT_TXPWR_CTRL
/* Tx pwr offset in 0.5db unit*/
INT mtk_cfg80211_set_mgmt_frame_power(RTMP_ADAPTER *pAd, UCHAR *arg)
{
	INT16  value;
	CHAR   target_pwr = 0;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	RTMP_STRING *tmp;
	struct wifi_dev *wdev;

	if (apidx >= pAd->ApCfg.BssidNum)
		return FALSE;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	if (!arg) {
		MTWF_PRINT("Invalid argument\n");
		return FALSE;
	}
	if (*arg == '.') { /* handle .5 arg value*/
		MTWF_PRINT("unable to update tx pwr %s\n", arg);
		return FALSE;
	}
	tmp = rstrtok(arg, ".");
	if (!tmp) {
		MTWF_PRINT("Please enter valid tx pwr\n");
		return FALSE;
	}
	value = os_str_tol(tmp, 0, 10);
	target_pwr = value*2; /* convert to  0.5db scale*/
	tmp = rstrtok(NULL, ".");
	if (tmp) {
		value = os_str_tol(tmp, 0, 10);
		if (value == 5)
			target_pwr += 1;
		else if (target_pwr == 0) {
			MTWF_PRINT("unable to update tx pwr\n");
			return FALSE;
		}
	}
	wdev->MgmtTxPwrBak = wdev->MgmtTxPwr;
	wdev->MgmtTxPwr = target_pwr;
	update_mgmt_frame_power(pAd, wdev);
	MTWF_PRINT("wdev->MgmtTxPwr = %d\n", wdev->MgmtTxPwr);
	return TRUE;
}
#endif
#endif


INT Get_Radio_Statistics(IN RTMP_ADAPTER * pAd,	OUT struct wifi_radio_stats *pStat)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	ULONG txCount = 0, rxCount = 0, txBytes = 0, rxBytes = 0;
	INT ifIndex = pObj->ioctl_if;
	BSS_STRUCT *pMbss;
#ifdef TR181_SUPPORT
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
#endif
	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR, "Error AP index\n");
		return -EINVAL;
	}

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
	if (wdev)
		ucBand = HcGetBandByWdev(wdev);

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"ra offload=%d\n", cap->fgRateAdaptFWOffload);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	if (cap->fgRateAdaptFWOffload == TRUE) {
		EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;

		os_zero_mem(&rTxStatResult, sizeof(EXT_EVENT_TX_STATISTIC_RESULT_T));
		MtCmdGetTxStatistic(pAd, GET_TX_STAT_TOTAL_TX_CNT, ucBand, 0, &rTxStatResult);
		pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += (rTxStatResult.u4TotalTxCount -
				rTxStatResult.u4TotalTxFailCount);
		pAd->WlanCounters.FailedCount.u.LowPart += rTxStatResult.u4TotalTxFailCount;
		pAd->WlanCounters.CurrentBwTxCount.u.LowPart += rTxStatResult.u4CurrBwTxCnt;
		pAd->WlanCounters.OtherBwTxCount.u.LowPart += rTxStatResult.u4OtherBwTxCnt;
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

	{
		txCount = pAd->WlanCounters.TransmittedFragmentCount.u.LowPart;
#if defined(MT7915) || defined(BELLWETHER) || defined(MT7990) || defined(MT7992) || defined(MT7993)
		if (IS_MT7915(pAd) ||
			IS_BELLWETHER(pAd) ||
			IS_MT7990(pAd) ||
			IS_MT7992(pAd) ||
			IS_MT7993(pAd)) {
			rxCount = (ULONG)pAd->WlanCounters.RxMpduCount.QuadPart;
		} else
#endif
			rxCount = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
	}

	txBytes = pAd->WlanCounters.TxTotByteCount.QuadPart;
	rxBytes = pAd->WlanCounters.RxTotByteCount.QuadPart;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"txBytes:%ld, rxBytes:%ld, TxCount:%ld, RxCount:%ld, chipid=%x\n",
		txBytes, rxBytes, txCount, rxCount, pAd->ChipID);

#if defined(MT7915) || defined(BELLWETHER) || defined(MT7990) || defined(MT7992) || defined(MT7993)
	if (IS_MT7915(pAd) ||
		IS_BELLWETHER(pAd) ||
		IS_MT7990(pAd) ||
		IS_MT7992(pAd) ||
		IS_MT7993(pAd)) {
		pStat->DiscardPacketsReceived = pAd->WlanCounters.RxFifoFullCount.u.LowPart;
		pStat->ErrorsReceived = pAd->WlanCounters.RxFcsErrorCount.u.LowPart;
	} else
#endif
	{
		pStat->DiscardPacketsReceived = pAd->Counters8023.RxNoBuffer;
		pStat->ErrorsReceived = pAd->WlanCounters.FCSErrorCount.u.LowPart;
	}

	pStat->PacketsSent = txCount;
	pStat->PacketsReceived = rxCount;
	pStat->BytesSent = txBytes;
	pStat->BytesReceived = rxBytes;
	pStat->MulticastPacketsSent = pAd->WlanCounters.mcPktsTx.QuadPart;
	pStat->MulticastPacketsReceived = pAd->WlanCounters.mcPktsRx.QuadPart;
	pStat->BroadcastPacketsSent = pAd->WlanCounters.bcPktsTx.QuadPart;
	pStat->BroadcastPacketsReceived = pAd->WlanCounters.bcPktsRx.QuadPart;
	pStat->UnicastPacketsSent = pStat->PacketsSent - pStat->MulticastPacketsSent
									- pStat->BroadcastPacketsSent;
	pStat->UnicastPacketsReceived = pStat->PacketsReceived - pStat->MulticastPacketsReceived
									- pStat->BroadcastPacketsReceived;
	pStat->ErrorsSent = pAd->WlanCounters.FailedCount.QuadPart;
	pStat->DiscardPacketsSent = pAd->WlanCounters.FailedCount.QuadPart;

#ifdef TR181_SUPPORT
	pStat->TotalChannelChangeCount =
		(ctrl->rdev.pRadioCtrl->TotalChannelChangeCount +
		pAd->ApBootACSChannelChangePerBandCount);
	pStat->ManualChannelChangeCount =
		ctrl->rdev.pRadioCtrl->ManualChannelChangeCount;
	pStat->AutoStartupChannelChangeCount =
		pAd->ApBootACSChannelChangePerBandCount;
	pStat->AutoUserChannelChangeCount =
		ctrl->rdev.pRadioCtrl->ForceACSChannelChangeCount;
	pStat->AutoRefreshChannelChangeCount =
		ctrl->rdev.pRadioCtrl->RefreshACSChannelChangeCount;
	pStat->AutoDynamicChannelChangeCount = 0; /* no support */
	pStat->AutoDFSChannelChangeCount =
		ctrl->rdev.pRadioCtrl->DFSTriggeredChannelChangeCount;
	pStat->Noise = (INT)pAd->noise_floor;
#endif

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"PacketsSent=%ld, PacketsReceived=%ld\n", pStat->PacketsSent, pStat->PacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"TransmittedByteCount=%ld, ReceivedByteCount=%ld\n",
		pStat->BytesSent, pStat->BytesReceived);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"ErrorsReceived=%ld, DiscardPacketsReceived=%ld\n",
		pStat->ErrorsReceived, pStat->DiscardPacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"ErrorsSent=%ld, DiscardPacketsSent=%ld\n",
		pStat->ErrorsSent, pStat->DiscardPacketsSent);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"ucPktsTx=%ld, ucPktsRx=%ld\n", pStat->UnicastPacketsSent, pStat->UnicastPacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"mcPktsTx=%ld, mcPktsRx=%ld\n", pStat->MulticastPacketsSent, pStat->MulticastPacketsReceived);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"bcPktsTx=%ld, bcPktsRx=%ld\n", pStat->BroadcastPacketsSent, pStat->BroadcastPacketsReceived);
#ifdef TR181_SUPPORT
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"TotalChannelChangeCount=%d, ManualChannelChangeCount=%d\n",
		pStat->TotalChannelChangeCount, pStat->ManualChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"AutoStartupChannelChangeCount=%d, AutoUserChannelChangeCount=%d\n",
		pStat->AutoStartupChannelChangeCount, pStat->AutoUserChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"AutoRefreshChannelChangeCount=%d, AutoDynamicChannelChangeCount=%d\n",
		pStat->AutoRefreshChannelChangeCount, pStat->AutoDynamicChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
		"AutoDFSChannelChangeCount=%d\n", pStat->AutoDFSChannelChangeCount);
	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO, "Noise=%d\n", pStat->Noise);
#endif
	return NDIS_STATUS_SUCCESS;
}
#ifdef TR181_SUPPORT
void Get_80211_Standard(IN BSS_ENTRY * pBss, OUT UCHAR *SupportedStandards)
{
	NDIS_802_11_NETWORK_TYPE wireless_mode = NetworkTypeInUseSanity(pBss);
	int ret;

	if (wireless_mode == Ndis802_11FH ||
		wireless_mode == Ndis802_11DS) {
		ret = snprintf(SupportedStandards, 2, "%s", "b");
		if (os_snprintf_error(2, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else if (wireless_mode == Ndis802_11OFDM5) {
		ret = snprintf(SupportedStandards, 2, "%s", "a");
		if (os_snprintf_error(2, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else if (wireless_mode == Ndis802_11OFDM5_N) {
		ret = snprintf(SupportedStandards, 4, "%s", "a/n");
		if (os_snprintf_error(4, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else if (wireless_mode == Ndis802_11OFDM5_AC) {
		ret = snprintf(SupportedStandards, 7, "%s", "a/n/ac");
		if (os_snprintf_error(7, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else if (wireless_mode == Ndis802_11OFDM24) {
		ret = snprintf(SupportedStandards, 4, "%s", "b/g");
		if (os_snprintf_error(4, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else if (wireless_mode == Ndis802_11OFDM24_N) {
		ret = snprintf(SupportedStandards, 6, "%s", "b/g/n");
		if (os_snprintf_error(6, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else if (wireless_mode == Ndis802_11OFDM24_HE) {
		ret = snprintf(SupportedStandards, 9, "%s", "b/g/n/ax");
		if (os_snprintf_error(9, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else if (wireless_mode == Ndis802_11OFDM5_HE) {
		ret = snprintf(SupportedStandards, 10, "%s", "a/n/ac/ax");
		if (os_snprintf_error(10, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	} else {
		ret = snprintf(SupportedStandards, 8, "%s", "unknown");
		if (os_snprintf_error(8, ret))
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
				"%s,%d,final_name snprintf error!\n", __func__, __LINE__);
	}
}

UCHAR Get_ScanSecurityMode(IN UINT32 authMode)
{
	/*	0: None
		1: WEP
		2: WPA
		3: WPA2
		4: WPA-WPA2
		5: WPA-Enterprise
		6: WPA2-Enterprise
		7: WPA-WPA2-Enterprise
		8: WPA3-SAE
		9: WPA2-PSK-WPA3-SAE
		10: WPA3-Enterprise */

	if (IS_AKM_OPEN(authMode))
		return 0;
	else if (IS_AKM_AUTOSWITCH(authMode))
		return 1;
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA3PSK(authMode) && IS_AKM_FT_SAE_SHA256(authMode))
		return 9;
	else if (IS_AKM_FT_SAE_SHA256(authMode) && IS_AKM_WPA3PSK(authMode))
		return 8;
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA1PSK(authMode))
		return 4;
	else if (IS_AKM_WPA1PSK(authMode))
		return 2;
	else if (IS_AKM_WPA2PSK(authMode))
		return 3;
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return 7;
	else if (IS_AKM_WPA1(authMode))
		return 5;
	else if (IS_AKM_WPA2(authMode))
		return 6;
	else if (IS_AKM_WPA3(authMode))
		return 10;
	else
		return 0;
}

UCHAR Get_ScanEncryMode(IN UINT32 encryMode)
{
	/*0: TKIP
	  1: AES
	  2: TKIP AES Mix
	  3: WEP
	  4: None*/
	if (IS_CIPHER_TKIP(encryMode))
		return 0;
	else if (IS_CIPHER_CCMP128(encryMode))
		return 1;
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return 2;
	else if (IS_CIPHER_WEP(encryMode))
		return 3;
	else
		return 4;
}

INT Get_Nbr_Diag_Result(IN RTMP_ADAPTER * pAd, IN struct nbr_diag_result	*result)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UINT32 IfIdx = pObj->ioctl_if;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,
		pObj->ioctl_if, pObj->ioctl_if_type);
	BSS_TABLE *ScanTab = get_scan_tab_by_wdev(pAd, wdev);
	BSS_ENTRY *pBss;
	UINT8 len;

	if (wdev == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
			"wdev is null! IfIdx: %d.\n", IfIdx);
		return -EINVAL;
	}

	if (scan_in_run_state(pAd, wdev)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
			"\n\n\n SCANNING, CANNOT GET SCAN RESULT NOW!\n\n\n");
		/*#define	EBUSY		16*/
		/* Device or resource busy */
		return (-EBUSY);
	}

	if (ScanTab->BssNr == 0) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR, "No Scan result.\n");
		return -EINVAL;
	}

	if (result->index >= ScanTab->BssNr) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
			"Index %d more than BssNr %d.\n", result->index, ScanTab->BssNr);
		return -EINVAL;
	}

	pBss = &ScanTab->BssEntry[result->index];
	result->Channel = pBss->Channel;
	NdisCopyMemory(result->ssid, pBss->Ssid, MAX_LEN_OF_SSID);
	NdisCopyMemory(result->bssid, pBss->Bssid, MAC_ADDR_LEN);

	if (wdev->if_dev) {
		len = (sizeof(wdev->if_dev->name) > 15) ? 15 : sizeof(wdev->if_dev->name);
		strlcpy(result->radio_name, wdev->if_dev->name, sizeof(result->radio_name));
	}

	result->radio_mode = pBss->BssType;
	result->SignalStrength = pBss->Rssi;
	result->SecurityModeEnabled = Get_ScanSecurityMode(pBss->AKMMap);
	result->BeaconPeriod = pBss->BeaconPeriod;
	result->EncryptionMode = Get_ScanEncryMode(pBss->PairwiseCipher);
	result->DTIMPeriod = pBss->DtimPeriod;

	if (WMODE_CAP_2G(wdev->PhyMode))
		result->OperatingFrequencyBand = 0;  /* 2G */
	else if (WMODE_CAP_5G(wdev->PhyMode))
		result->OperatingFrequencyBand = 1;  /* 5G */
	else if (WMODE_CAP_6G(wdev->PhyMode))
		result->OperatingFrequencyBand = 2;  /* 6G */

	Get_80211_Standard(pBss, result->SupportedStandards);
	NdisCopyMemory(result->OperatingStandards, result->SupportedStandards,
		sizeof(result->SupportedStandards));
	result->OperatingChannelBandwidth = 0;  /* Not Support */
	result->Noise = 0; /* Not support */
	NdisCopyMemory(result->SupRate, pBss->SupRate, pBss->SupRateLen);
	NdisCopyMemory(result->ExtRate, pBss->ExtRate, pBss->ExtRateLen);
	return NDIS_STATUS_SUCCESS;
}


INT Get_BSS_Statistics(IN RTMP_ADAPTER * pAd, OUT struct wifi_bss_stats *pStat)
{
	BSS_STRUCT *pMbss;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT ifIndex = pObj->ioctl_if;

	if (!VALID_MBSS(pAd, ifIndex)) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR, "Error AP index\n");
		return -EINVAL;
	}

	pMbss = &pAd->ApCfg.MBSSID[ifIndex];

	pStat->PacketsSent =  pMbss->TxCount;
	pStat->PacketsReceived =  pMbss->RxCount;
	pStat->BytesSent = pMbss->TransmittedByteCount;
	pStat->BytesReceived = pMbss->ReceivedByteCount;
	pStat->ErrorsReceived =  pMbss->RxErrorCount;
	pStat->DiscardPacketsReceived =  pMbss->RxDropCount;
	pStat->ErrorsSent =  pMbss->TxErrorCount;
	pStat->DiscardPacketsSent =  pMbss->TxDropCount;
	pStat->UnicastPacketsSent =  pMbss->ucPktsTx;
	pStat->UnicastPacketsReceived =  pMbss->ucPktsRx;
	pStat->MulticastPacketsSent =  pMbss->mcPktsTx;
	pStat->MulticastPacketsReceived =  pMbss->mcPktsRx;
	pStat->BroadcastPacketsSent = pMbss->bcPktsTx;
	pStat->BroadcastPacketsReceived = pMbss->bcPktsRx;
	return NDIS_STATUS_SUCCESS;
}


INT Get_Dev_Ac(IN RTMP_ADAPTER * pAd, IN UINT ap_idx, OUT struct wifi_dev_ac *ac)
{
	int i;
	struct wifi_dev *wdev = NULL;
	struct _EDCA_PARM *pBssEdca = NULL;
	struct wlan_config *cfg = NULL;

	wdev = &pAd->ApCfg.MBSSID[ap_idx].wdev;

	pBssEdca = wlan_config_get_ht_edca(wdev);
	if (pBssEdca == NULL) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR, "pBssEdca is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	cfg = (struct wlan_config *)wdev->wpf_cfg;
	if (!cfg) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR, "wpf_cfg NULL\n");
		return NDIS_STATUS_FAILURE;
	}
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		ac[i].AccessCategory = i;
		ac[i].AIFSN = pBssEdca->Aifsn[i];
		ac[i].ECWMin = pBssEdca->Cwmin[i];
		ac[i].ECWMax = pBssEdca->Cwmax[i];
		ac[i].TxOpMax = pBssEdca->Txop[i];
		ac[i].AckPolicy = cfg->phy_conf.ack_policy[i];
	}
	return NDIS_STATUS_SUCCESS;
}

void Get_Sta_Info(IN RTMP_ADAPTER * pAd, IN OUT struct wifi_station *sta_info)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	mtk_rate_info_t tx_rate, rx_rate;
	struct _STA_TR_ENTRY *tr_entry;

	sta_info->valid = FALSE;

	pEntry = MacTableLookup(pAd, sta_info->MacAddr);
	if (!(IS_VALID_ENTRY(pEntry)) || (pEntry->Sst != SST_ASSOC))
		return;

	if (pEntry->wdev)
		sta_info->PhyMode = pEntry->wdev->PhyMode;

	sta_info->LastConnectTime = pEntry->StaConnectTime;
	if (IS_WCID_VALID(pAd, pEntry->wcid)) {
		tr_entry = tr_entry_get(pAd, pEntry->wcid);
		sta_info->AuthenticationState =
			(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? TRUE : FALSE;
	}

	sta_info->SignalStrength = RTMPAvgRssi(pAd, &pEntry->RssiSample);

	sta_info->Noise = (SHORT)pAd->noise_floor;
	sta_info->Retransmissions = 0; /* not support */
	sta_info->UtilizationReceive = pEntry->TxRxTime[0][0] +
									pEntry->TxRxTime[1][0] +
									pEntry->TxRxTime[2][0] +
									pEntry->TxRxTime[3][0];
	sta_info->UtilizationTransmit =	pEntry->TxRxTime[0][1] +
									pEntry->TxRxTime[1][1] +
									pEntry->TxRxTime[2][1] +
									pEntry->TxRxTime[3][1];
#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn && (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_APCLI(pEntry)) &&
		IS_REPT_LINK_UP(pEntry->pReptCli)) {
		NdisCopyMemory(sta_info->VMacAddr, pEntry->pReptCli->CurrentAddress,
			MAC_ADDR_LEN);
		sta_info->repeater = TRUE;
	}
#endif/*MAC_REPEATER_SUPPORT*/
	sta_info->valid = TRUE;
	sta_info->stats.BytesReceived = pEntry->RxBytes;
	sta_info->stats.BytesSent = pEntry->TxBytes;
	sta_info->stats.PacketsReceived = pEntry->RxPackets.QuadPart;
	sta_info->stats.PacketsSent = pEntry->TxPackets.QuadPart;
	sta_info->stats.ErrorsSent = (UINT)pEntry->TxFailCount;
	sta_info->stats.ErrorsReceived = 0;
	sta_info->stats.RetransCount = 0;
	sta_info->stats.FailedRetransCount = 0;
	sta_info->stats.RetryCount = (UINT)pEntry->TxRetriedPktCount;
	sta_info->stats.MultipleRetryCount = 0;
	os_zero_mem(&tx_rate, sizeof(mtk_rate_info_t));
	os_zero_mem(&rx_rate, sizeof(mtk_rate_info_t));
	if (pEntry->wdev && (get_sta_rate_info(pAd, pEntry, &tx_rate, &rx_rate) == TRUE)) {
		sta_info->rates.TxRate = tx_rate.legacy;
		sta_info->rates.RxRate = rx_rate.legacy;
	}
	/* Mbps = (n Bytes x 8) / (1024 * 1024) = (n >> 17) */;
	sta_info->rates.TxRate_rt = pEntry->AvgTxBytes >> 17;
	sta_info->rates.RxRate_rt = pEntry->AvgRxBytes >> 17;
	sta_info->rates.avg_tx_rate = 0; /* not support */
	sta_info->rates.avg_rx_rate = 0; /* not support */
}


#ifdef WSC_AP_SUPPORT
PWSC_CTRL Get_WscControl(IN RTMP_ADAPTER * pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = 0;

	if (pObj)
		apidx = pObj->ioctl_if;
	else
		return NULL;

	MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_ERROR,
		"ioctl_if_type=%d, ifIndex is %d.if_dev name=%s\n",
		pObj->ioctl_if_type, apidx, pAd->ApCfg.MBSSID[apidx].wdev.if_dev->name);

#ifdef APCLI_SUPPORT
	if (pObj->ioctl_if_type == INT_APCLI)
		return &pAd->StaCfg[apidx].wdev.WscControl;
#endif /* APCLI_SUPPORT */
	return &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
}
#endif
#endif /* TR181_SUPPORT */
#ifdef EAP_STATS_SUPPORT
VOID CalStaTxLatency(IN PMAC_TABLE_ENTRY pEntry, IN UINT32 transmitDelay, IN UINT32 airDelay, IN UINT32 txCnt)
{
	if (pEntry == NULL)
		return;

	/* TxTime */
	if (pEntry->tx_latency_max == 0) {
		pEntry->tx_latency_min = transmitDelay;
		pEntry->tx_latency_max = transmitDelay;
		pEntry->tx_latency_avg = transmitDelay;
	} else {
		if (transmitDelay > 0) {
			if (pEntry->tx_latency_min > transmitDelay)
				pEntry->tx_latency_min = transmitDelay;
			if (pEntry->tx_latency_max < transmitDelay)
				pEntry->tx_latency_max = transmitDelay;
			pEntry->tx_latency_avg =
				(transmitDelay + pEntry->tx_latency_avg) / 2;
		}
	}

#ifdef EAP_ENHANCE_STATS_SUPPORT
	/* AirTime */
	if (pEntry->air_latency_max == 0) {
		pEntry->air_latency_min = airDelay;
		pEntry->air_latency_max = airDelay;
		pEntry->air_latency_avg = airDelay;
	} else {
		if (airDelay > 0) {
			if (pEntry->air_latency_min > airDelay)
				pEntry->air_latency_min = airDelay;
			if (pEntry->air_latency_max < airDelay)
				pEntry->air_latency_max = airDelay;
			pEntry->air_latency_avg =
				(airDelay + pEntry->air_latency_avg) / 2;
		}
	}
	/* TxCount */
	if (pEntry->tx_cnt_max == 0) {
		pEntry->tx_cnt_max = txCnt;
		pEntry->tx_cnt_max = txCnt;
		pEntry->tx_cnt_max = txCnt;
	} else {
		if (txCnt > 0) {
			if (pEntry->tx_cnt_min > txCnt)
				pEntry->tx_cnt_min = txCnt;
			if (pEntry->tx_cnt_max < txCnt)
				pEntry->tx_cnt_max = txCnt;
			pEntry->tx_cnt_avg =
				(txCnt + pEntry->tx_cnt_avg) / 2;
		}
	}
#endif /* EAP_ENHANCE_STATS_SUPPORT */

}
#endif /* EAP_STATS_SUPPORT */

