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
    red.h

*/

#ifndef _RED_H_
#define _RED_H_

#define CFG_RED_RAAC_TAIL_DROP				(0)
#define CFG_RED_HOST_RED				(0)
#define CFG_RED_HW_ACM					(1)
#define CFG_RED_TRUN_ON_RANDOM_DROP                     (0)
#define CFG_ESTIMATION_RED_CPU_UTILIZATION              (0)
#define RED_DROP_TH_LOWER_BOUND                         (20)
#define RED_DROP_TH_UPPER_BOUND                         (600)

#define RED_VHT_BW20_DEFAULT_THRESHOLD                  (384)
#define RED_VHT_BW40_DEFAULT_THRESHOLD                  (768)
#define RED_VHT_BW80_DEFAULT_THRESHOLD                  (1536)
#define RED_HT_BW20_DEFAULT_THRESHOLD                   (192)
#define RED_HT_BW40_DEFAULT_THRESHOLD                   (384)
#define RED_LEGACY_DEFAULT_THRESHOLD                    (192)

#define RED_WLOG_DEFAULT                                (10)
#define RED_MPDU_TIME_INIT                              (200)
#define RED_MULTIPLE_NUM_DEFAULT                        (30)
#define RA_AC_FREE_FOR_ALL                              (3072)

#define RED_BAD_NODE_DROP_THRESHOLD                     (192)
#define RED_BAD_NODE_HT_VHT_DEFAULT_THRESHOLD           (192)
#define RED_BAD_NODE_LEGACY_DEFAULT_THRESHOLD           (60)
#define	RED_MAX_BAD_NODE_CNT                            (10)
#define	RED_MAX_GOOD_NODE_CNT                           (7)
#define	RED_BAD_NODE_CNT_MASK                           (0x0f)
#define	RED_GOOD_NODE_CNT_MASK                          (0x70)
#define	RED_IS_BAD_NODE_MASK                            (0x80)
#define	RED_GOOD_NODE_CNT_SHIFT_BIT                     (4)
#define	RED_IS_BAD_NODE_SHIFT_BIT                       (7)

#define BADNODE_TIMER_PERIOD	100
#define RED_STA_REC_NUM                                 MAX_LEN_OF_MAC_TABLE

#define QLEN_SCALED_BIT                                 (0)
#define PROB_SCALED_BIT                                 (0)
#define QTH_WRIGTH_BIT                                  (2)
#define WSCALING_BIT                                    (16)
#define QLEN_SCALED                                     (1 << QLEN_SCALED_BIT)
#define PROB_SCALED                                     (1 << PROB_SCALED_BIT)

#define RED_INUSE_BITSHIFT					5
#define RED_INUSE_BITMASK					(0x1f)

#define HWRED_MAX_BN_NUM				(4)
#define HWRED_MAX_SRC_NUM				(4)
#define HWRED_MAX_THLD_NUM				(8)
#define HWRED_MAX_TKID_NUM				(16384)
#define HWRED_MAX_AC_QUE_NUM				(5)
#define HWRED_MAX_WTBL_NUM				(1088)
#define MAX_BITMAP_WORDLEN				((1088 + 31) / (sizeof(UINT_32) << 3))
#define MAX_CONFIG_BITMAP_WORDLEN			(8)
#define HWRED_MAX_CONFIG_STA_NUM			(MAX_CONFIG_BITMAP_WORDLEN << 5)

#ifndef AC_NUM
#define AC_NUM						4
#endif

#ifdef CFG_RED_SUPPORT
enum RED_TYPE {
	RED_DISABLE = 0,
	RED_BY_HOST_ENABLE,
	RED_BY_WA_ENABLE
};

enum RED_TAIL_DROP_SCEN {
	RED_TAIL_DROP_SCEN_NORMAL = 0,
	RED_TAIL_DROP_SCEN_IXIA = 1
};

struct physical_device;

/* per AC data structure */
struct red_ac_element {
	u32 u2TotalDropCnt;
	u16 u2DropCnt;
	u16 u2EnqueueCnt;
	u16 u2DequeueCnt;
	u16 u2qEmptyCnt;
	u8 ucShiftBit;
	u8 ucGBCnt;
#if (CFG_RED_TRUN_ON_RANDOM_DROP == 1)
	u32 u4AvgLen;
	u16 u2qRan;
	u16 u2qCount;
	u16 u2DropProbCnt;
	u16 u2DropTailCnt;
#endif
};

/* per STA data structure */
struct red_sta_info {
	int i4MpduTime;
	int tx_msdu_avg_cnt;
	int tx_msdu_cnt;
	u16 u2Dropth;
	u16 u2DriverFRCnt; /* Record ForceRate counter which is from Driver. */
	u8 ucMultiplyNum;
	struct red_ac_element arRedElm[WMM_NUM_OF_AC];
#if (CFG_RED_TRUN_ON_RANDOM_DROP == 1)
	u8 ucIsBadNode;
	u32 u4IGMPCnt;
	u32 u4TxmCnt;
#endif
};

struct GNU_PACKED admctrl_sta {
	UINT_8 fgDisable:1;
	UINT_8 ucRateShift:4;
	UINT_8 fgRateShiftDeRef:1;
	UINT_8 u1Reserved:2;
};

struct red_ctrl {
#if (CFG_RED_RAAC_TAIL_DROP == 1)
	u16 u2AllTokenLowMark;
	u16 u2AllTokenHighMark;
	u16 u2TokenLowMark[CFG_WIFI_RAM_BAND_NUM];
	u16 u2TokenHighMark[CFG_WIFI_RAM_BAND_NUM];
	u16 u2PageLowMark;
	u16 u2PageHighMark;
	u32 u4AllTokenFullCnt;
	u32 u4TokenFullCnt[CFG_WIFI_RAM_BAND_NUM];
	u32 u4PageFullCnt;
	u8 tx_bh_period;
	u8 rx_bh_period;
	u8 u1PfmEvent1;
	u8 u1PfmEvent2;
	u8 fgEnable;
	u8 fgDbgShow;
	u8 fgDbgNoDrop;
	u32 u4CheckDropCnt;
	u32 u4TotInputCnt;
	u32 u4IterCnt;
	u32 u4DropCnt[CFG_WIFI_RAM_BAND_NUM];
	enum RED_TAIL_DROP_SCEN eScenario;
#else
	u16 u2TCPOffset;
	u16 u2PriorityOffset;
	u16 u2TokenPerSrc[HWRED_MAX_SRC_NUM];
	u16 u2TokenThldPerSrc[HWRED_MAX_SRC_NUM];
	u16 u2FullTbl[HWRED_MAX_BN_NUM][HWRED_MAX_THLD_NUM];
	u16 u2DropTbl[HWRED_MAX_BN_NUM][HWRED_MAX_THLD_NUM];
	u16 u2ACTailDropMin[HWRED_MAX_BN_NUM][HWRED_MAX_AC_QUE_NUM];
	u16 u2ACTailDropMax[HWRED_MAX_BN_NUM][HWRED_MAX_AC_QUE_NUM];
	u16 sta_config_cnt;
	u32 sta_config_bitmap[MAX_BITMAP_WORDLEN];
	struct admctrl_sta sta_ctrl[HWRED_MAX_CONFIG_STA_NUM];
	LARGE_INTEGER drop_cnt;
	LARGE_INTEGER drop_cnt_per_band[HWRED_MAX_BN_NUM];
	LARGE_INTEGER drop_cnt_per_ac[AC_NUM];
	LARGE_INTEGER tot_txcnt;
	LARGE_INTEGER tot_txcnt_per_band[HWRED_MAX_BN_NUM];
	LARGE_INTEGER tot_txcnt_per_ac[AC_NUM];
	bool fgWMMDetect;
#endif
};

struct red_setting {
	u8  red_en;
	u8  red_mcu_offload;
	u8  red_debug_en;
	struct red_sta_info red_sta[MAX_LEN_OF_MAC_TABLE];
	u16 red_targetdelay;
	u16 red_atm_on_targetdelay;
	u16 red_atm_off_targetdelay;
	u16 red_sta_num;
	RALINK_TIMER_STRUCT red_badnode_timer;
};

/* Red token tail drop CMD format */
enum RED_CMD_TYPE {
	RED_SET_GLOBAL_WATERMARK = 1,
	RED_SET_GLOBAL_TOKEN_WATERMARK = 2,
	RED_SET_GLOBAL_PAGE_WATERMARK = 3,
	RED_SET_GLOBAL_BAND_WATERMARK = 4,
	RED_SET_STA_THRES_BY_HOST = 5,
	RED_RELEASE_STA_THRES_FROM_HOST = 6,
	RED_SET_CTRL = 7,
	RED_DUMP_CTRL = 8,
	RED_DUMP_STATISTICS = 9,
	RED_CMD_MAX
};

enum ACM_CMD_TYPE {
	ACM_GLOBAL_CONFIG = 1,
	ACM_TOKEN_CONFIG = 2,
	ACM_PER_BAND_RED_CONFIG = 3,
	ACM_AC_TAIL_DROP_CONFIG = 4,
	ACM_STA_CONFIG = 5,
	ACM_OFFSET_CONFIG = 6,
	ACM_TKID_TAIL_DROP_CONFIG = 7,
	ACM_DUMP_CONFIG = 8,
	ACM_RELEASE_CONFIG = 9,
	ACM_CMD_MAX
};

struct GNU_PACKED admctrl_global {
	u16 u2TCPOffset;
	u16 u2PriorityOffset;
	u16 u2TokenPerSrc[HWRED_MAX_SRC_NUM];
	u16 u2TokenThldPerSrc[HWRED_MAX_SRC_NUM];
	u16 u2FullTbl[HWRED_MAX_BN_NUM][HWRED_MAX_THLD_NUM];
	u16 u2DropTbl[HWRED_MAX_BN_NUM][HWRED_MAX_THLD_NUM];
	u16 u2ACTailDropMin[HWRED_MAX_BN_NUM][HWRED_MAX_AC_QUE_NUM];
	u16 u2ACTailDropMax[HWRED_MAX_BN_NUM][HWRED_MAX_AC_QUE_NUM];
};

struct GNU_PACKED red_cmd_admctrl_config {
	struct admctrl_global global;

	__aligned(4) UINT_16 sta_config_cnt;
	__aligned(4) UINT_32 bitmap[MAX_BITMAP_WORDLEN];
	__aligned(4) struct admctrl_sta sta_ctrl[HWRED_MAX_CONFIG_STA_NUM];
};

struct GNU_PACKED red_cmd_watermark {
	u16 u2AllTokenHighMark;
	u16 u2AllTokenLowMark;
	u16 u2TokenHighMark[CFG_WIFI_RAM_BAND_NUM];
	u16 u2TokenLowMark[CFG_WIFI_RAM_BAND_NUM];
	u16 u2PageHighMark;
	u16 u2PageLowMark;
};

struct GNU_PACKED red_cmd_raac_thres {
	u16 u2Idx;
	u8  ucAc;
	u8  ucReserved;
	u16 u2TokenHighMark;
	u16 u2TokenLowMark;
	u16 u2PageHighMark;
	u16 u2PageLowMark;
	u16 u2TokenComp;
	u16 u2PageComp;
};

struct GNU_PACKED red_cmd_release_raac_thres {
	u16 u2Idx;
	u8  ucAc;
	u8  ucReserved;
};

struct GNU_PACKED red_cmd_set_ctrl {
	u8 fgEnable;
	u8 fgDbgShow;
	u8 fgDbgNoDrop;
	u8 tx_bh_period;
	u8 rx_bh_period;
	u8 u1PfmEvent1;
	u8 u1PfmEvent2;
	u8 aucReserved[1];
};

struct GNU_PACKED red_cmd_header {
	/*Common Part*/
	u8  ucOpMode;
	u8  ucCmdVer;
	u8  aucPadding0[2+2];
	u16 u2CmdLen;
};

#define PD_GET_RED_ENABLE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->red_setting.red_en)
#define PD_SET_RED_ENABLE(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->red_setting.red_en = _value)
#define PD_GET_RED_MCU_OFFLOAD(_ph_dev) \
	(((struct physical_device *)_ph_dev)->red_setting.red_mcu_offload)
#define PD_SET_RED_MCU_OFFLOAD(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->red_setting.red_mcu_offload = _value)
#define PD_GET_RED_STA_BY_IDX(_ph_dev, _idx) \
	(&((struct physical_device *)_ph_dev)->red_setting.red_sta[_idx])
#define PD_GET_RED_BAD_NODE_TIMER_PTR(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->red_setting.red_badnode_timer)
#define PD_IS_RED_BAD_NODE_TIMER_VALID(_ph_dev) \
	(((struct physical_device *)_ph_dev)->red_setting.red_badnode_timer.Valid)
#define PD_GET_RED_TARGET_DELAY(_ph_dev) \
	(((struct physical_device *)_ph_dev)->red_setting.red_targetdelay)
#define PD_SET_RED_TARGET_DELAY(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->red_setting.red_targetdelay = _value)
#define PD_GET_RED_ATM_ON_TARGET_DELAY(_ph_dev) \
	(((struct physical_device *)_ph_dev)->red_setting.red_atm_on_targetdelay)
#define PD_SET_RED_ATM_ON_TARGET_DELAY(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->red_setting.red_atm_on_targetdelay = _value)
#define PD_GET_RED_ATM_OFF_TARGET_DELAY(_ph_dev) \
	(((struct physical_device *)_ph_dev)->red_setting.red_atm_off_targetdelay)
#define PD_SET_RED_ATM_OFF_TARGET_DELAY(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->red_setting.red_atm_off_targetdelay = _value)
#define PD_GET_RED_CTRL_PTR(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->red_ctrl)
#define PD_GET_RED_STA_NUM(_ph_dev) \
	(((struct physical_device *)_ph_dev)->red_setting.red_sta_num)
#define PD_SET_RED_STA_NUM(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->red_setting.red_sta_num = _value)



void red_setting_init(struct physical_device *ph_dev);
void red_init(struct _RTMP_ADAPTER *ad);
INT set_red_config(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT set_red_show_sta(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

void acm_tx_cnt_update(
	struct _RTMP_ADAPTER *pAd,
	u8 qid,
	u16 wcid,
	u32 txcnt,
	u32 stat);

#endif /* CFG_RED_SUPPORT */

#endif /* _RED_H_ */
