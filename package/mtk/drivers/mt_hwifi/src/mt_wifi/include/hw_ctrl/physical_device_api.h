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
/****************************************************************************
 ***************************************************************************

	Module Name:
	physical_device.h

	Abstract:
	One physical device generic portion header file

*/
#ifndef __PHYSICAL_DEVICE_DEF_H__
#define __PHYSICAL_DEVICE_DEF_H__

#define DEFAULT_LOCAL_MAX_TXPWR	47 /* 23.5dB */
#define DEFAULT_MAX_TXPWR_PSD   21 /* 10.5 dbm/MHz*/
#define DEFAULT_MIN_TXPWR_PSD   0xFE /* -1 dbm/MHz*/
#define MAX_TXPWR_PSD_SIZE      17
#define MAX_MULTI_CARD_NUM      3
#define TIMER_EXEC_INTV         1000
#define TXPWR_PSD_N_K_NUM_BW320 16
#define TXPWR_PSD_N_K_NUM_BW160 8
#define TXPWR_PSD_N_K_NUM_BW80  4
#define TXPWR_PSD_N_K_NUM_BW40  2

/* Flags define */
#define PH_DEV_CHECK_ETH_TYPE     0x00000001
#define PH_DEV_SYSTEM_START_UP    0x00000002

enum {
	BAND0_ENABLED = 0x1,
	BAND1_ENABLED = 0x2,
	BAND2_ENABLED = 0x4,
};

enum {
	MTK_MLO_DISABLE = 0,
	MTK_MLO_V1 = BIT(0),
	MTK_MLO_V1_1 = BIT(1),
};

struct txpwr_cfg {
	u8 local_max_txpwr_bw20;
	u8 local_max_txpwr_bw40;
	u8 local_max_txpwr_bw80;
	u8 local_max_txpwr_bw160;
	u8 local_max_txpwr_bw320;
	u8 max_txpwr_psd[MAX_TXPWR_PSD_SIZE];
};

struct physical_device {
	u32 chip_id;
	u32 hw_id;
	u16 hw_ver;

#ifdef FW_LOG_DUMP
	FW_LOG_CTRL fw_log_ctrl;
	struct FW_IDX_LOG_CTRL fw_idx_log_ctrl;
	UCHAR host_log_ctrl;
	u16 host_dump_offset;
#endif /* FW_LOG_DUMP */

	u8 gen_down;
	u8 hw_wtbl_support;

	/* Duplicate 11v bcn/probe rsp in host instead of wm */
	BOOLEAN bHost_11vmbss_dup_bcn;

#ifdef SW_CONNECT_SUPPORT
	struct os_idr_mgmt sw_uwtbl_mgmt; /* maintain u/c tr_entry[] index as s/w wcid */
	BOOLEAN bSwSta; /* S/W STA Enable */
	struct os_idr_mgmt sw_group_mgmt; /* maintain b/c tr_entry[] index as s/w wcid */
#endif /* SW_CONNECT_SUPPORT */

	struct _RTMP_CHIP_CAP chip_cap;
	/*
	 * mtk_hw_dev, one mtk_hw_dev only in one physical device
	 */
	void *hw_dev;

	/*
	 * mac adapter list, per pad per band.
	 */
	struct _LIST_HEADER mac_ad_list;

	/*
	 * ASIC on-chip WCID entry table.
	 * At TX, ASIC always use key according to this on-chip table.
	 *
	 */
	struct _MAC_TABLE mac_table;
	NDIS_SPIN_LOCK mac_table_lock;
	NDIS_SPIN_LOCK irq_lock;
	struct _PHY_STATE_RX_RATE_PAIR *RxRateResultPair;
	struct qm {
		/* QM */
		struct qm_ops *qm_ops;
		struct qm_ctl qm_ctl;
		struct notify_entry qm_wsys_ne[FP_QUE_NUM];
		struct fp_qm qm_parm;
		/* QM Queue */
		struct _QUEUE_HEADER fp_que[FP_QUE_NUM];
		struct _QUEUE_HEADER fp_post_que[FP_QUE_NUM];
		NDIS_SPIN_LOCK fp_que_lock[FP_QUE_NUM];
		NDIS_SPIN_LOCK fp_post_que_lock[FP_QUE_NUM];
#ifdef IGMP_SNOOPING_OFFLOAD
		struct _QUEUE_HEADER fp_bmc_que[FP_QUE_NUM];
		struct _QUEUE_HEADER fp_post_bmc_que[FP_QUE_NUM];
		NDIS_SPIN_LOCK fp_bmc_que_lock[FP_QUE_NUM];
		NDIS_SPIN_LOCK fp_post_bmc_que_lock[FP_QUE_NUM];
#endif /* IGMP_SNOOPING_OFFLOAD */
#ifdef HIGH_PRIO_QUEUE_SUPPORT
		struct _QUEUE_HEADER fp_highpri_que[FP_QUE_NUM];
		struct _QUEUE_HEADER fp_post_highpri_que[FP_QUE_NUM];
		NDIS_SPIN_LOCK fp_highpri_que_lock[FP_QUE_NUM];
		NDIS_SPIN_LOCK fp_post_highpri_que_lock[FP_QUE_NUM];
#endif
		NDIS_SPIN_LOCK mgmt_que_lock[FP_QUE_NUM];
		NDIS_SPIN_LOCK mgmt_post_que_lock[FP_QUE_NUM];
		struct _QUEUE_HEADER mgmt_que[FP_QUE_NUM];
		struct _QUEUE_HEADER mgmt_post_que[FP_QUE_NUM];
		NDIS_SPIN_LOCK high_prio_que_lock;
		struct _QUEUE_HEADER high_prio_que;
		struct tx_swq_fifo *tx_swq[FP_QUE_NUM];
		/* Tx flow ctrl */
		struct fp_tx_flow_control fp_tx_flow_ctl;
		/* TM */
		struct tm_ops *tm_qm_ops;
		BOOLEAN tm_tx_deq_sched[FP_QUE_NUM];
		RTMP_NET_TASK_STRUCT tm_tx_deq_tasklet[FP_QUE_NUM];
	} qm;
	WIFI_SYS_INFO_T wifi_sys_info;
	/* --------------------------- */
	/* E2PROM		       */
	/* --------------------------- */
	enum EEPROM_STORAGE_TYPE eeprom_type;
	UCHAR E2pAccessMode; /* Used to identify flash, efuse, eeprom or bin from start-up */

	UCHAR LocalAdminMAC[MAC_ADDR_LEN];	/* profile default MAC address */
	UCHAR LocalAdminMAC1[MAC_ADDR_LEN];	/* profile default MAC address */
	UCHAR LocalAdminMAC2[MAC_ADDR_LEN];	/* profile default MAC address */

	UCHAR ra_mac_table[CFG_WIFI_RAM_BAND_NUM][MAC_ADDR_LEN];/*used to avoid mac addr conflict*/
#ifdef CCN67_BS_SUPPORT
	UINT8 ic_bb_percent[CFG_WIFI_RAM_BAND_NUM];
	UINT32 band_bal_minsta_thresh;
	UINT8	ic_bb_withhold_limit;
	UINT8	rsc_bsOnMeshAp;
#endif

#ifdef MAC_ADDR_ADJACENT_CHK
	/* Force MAC Addr Adjust each band */
	BOOLEAN bMacAddrAdj;
#endif /* MAC_ADDR_ADJACENT_CHK */

	/* byte 0: version, byte 1: revision, byte 2~3: unused */
	ULONG EepromVersion;
	/* byte 0: Minor version, byte 1: Major version, otherwise unused. */
	ULONG FirmwareVersion;
	USHORT EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR EEPROMAddressNum;	/* 93c46=6  93c66=8 */
	UCHAR EFuseTag;

#ifdef RTMP_EFUSE_SUPPORT
	BOOLEAN bUseEfuse;
#endif /* RTMP_EFUSE_SUPPORT */

	struct _EFUSE_FREE_BLOCK_INFO EfuseBlkInfo;
	UCHAR *EEPROMImage;
	UINT8 TestModeEn; /* use testmode fw when fw dl */

#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT
	EFUSE_INFO_ALL_T EfuseInfoAll;
#endif /*#ifdef	CONNAC_EFUSE_FORMAT_SUPPORT*/

	struct _EEPROM_CONTROL E2pCtrl;

	struct txpwr_cfg txpwr_cfg;
#ifdef WF_RESET_SUPPORT
	RTMP_OS_TASK wf_reset_thread;
	INT wf_reset_state;
	BOOLEAN wf_reset_in_progress;
	INT wf_reset_wm_count;
	INT wf_reset_wa_count;
	INT wf_reset_wo_count;
	UINT16 FwCmdTimeoutcheckCnt;
	FWCMD_TIMEOUT_RECORD FwCmdTimeoutRecord[FW_CMD_TO_RECORD_CNT];
#endif

	struct ba_control ba_ctl;
#ifdef WHNAT_SUPPORT
	BOOLEAN whnat_en;
#endif /*WHNAT_SUPPORT*/
	BOOLEAN assoc_record_en;
	struct MCU_CTRL MCUCtrl;
#ifdef CFG_RED_SUPPORT
	struct red_setting red_setting;
	struct red_ctrl red_ctrl;
#endif /* CFG_RED_SUPPORT */
	u8 band_num;
	u8 band_en;
	char *profile_path[CFG_WIFI_RAM_BAND_NUM];
#ifdef DOT11_EHT_BE
	struct mlo {
		u8 mlo_config_op_set;
		u8 str_bitmap;
		u8 sync_tx_enable;
		u8 link_ant_num[CFG_WIFI_RAM_BAND_NUM];
		u8 mlo_v1_enable;
		u8 mlo_v1_2_enable;
	} mlo;
#endif /* DOT11_EHT_BE */
#ifdef A4_CONN
	UINT32 a4_interface_count;
#endif
	u8 device_idx;
	u8 device_name[IFNAMSIZ];
#if (defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)) && !defined(RT_CFG80211_SUPPORT)
	SAE_CFG sae_cfg;
#endif /* DOT11_SAE_SUPPORT || SUPP_SAE_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	NDIS_AP_802_11_PMKID pmkid_cache;
#endif /* CONFIG_AP_SUPPORT */
	/* MAT related parameters */
#ifdef MAT_SUPPORT
	MAT_STRUCT mat_cfg;
#endif /* MAT_SUPPORT */

#ifdef PRE_CAL_MT7990_SUPPORT
	UINT16 chipsku;
#endif/* PRE_CAL_MT7990_SUPPORT */

	NDIS_SPIN_LOCK mlme_lock;
	NDIS_SPIN_LOCK bcn_check_lock;
	u32 Flags;

	/* physical device thread related++ */
	RTMP_OS_TASK pd_thread_task;
	NDIS_SPIN_LOCK pd_thread_data_lock;
	LIST_HEADER pd_thread_data_list;
	RALINK_TIMER_STRUCT pd_main_timer;
	NDIS_SPIN_LOCK pd_main_timer_lock;
	ULONG periodic_round;
	/* physical device thread related-- */
	/*rro setbl release resource*/
	HW_CTRL_T HwCtrl;
	u8 rro_timeout_profile_set;
#ifdef PEAK_ENHANCE
	struct peak_rx_enhance_ctrl peak_rx_ctrl;
#endif /* PEAK_ENHANCE */
	UINT_8 bw_en_mask;
	UINT_8 bw_group_bw_on_mask[CFG_WIFI_RAM_BAND_NUM];
	UINT_8 bw_group_at_on_mask[CFG_WIFI_RAM_BAND_NUM];
#ifdef ERR_RECOVERY
	ERR_RECOVERY_CTRL_T ErrRecoveryCtl;
#endif
#ifdef CONFIG_3_WIRE_SUPPORT
	UINT_8 threeWire_function_enable;
#endif
	UINT8 edcca_region;
	struct TXS_CB_LIST TxsCbList;

	/* purpose: Cancel all timers when module is removed */
	struct _LIST_HEADER RscTimerCreateList;	/* timers list */
	NDIS_SPIN_LOCK TimerSemLock;
	OS_WORKQUEUE_STRUCT *ph_dev_wq;
	OS_DELAYED_WORK ph_dev_work;
	NDIS_SPIN_LOCK ph_dev_wq_lock;
	LIST_HEADER ph_dev_work_list;
	/* for pleinfo usage */
	struct pause_bitmap_ pause_bitmap;
};

#define PD_GET_HW_VER(_ph_dev) \
	(((struct physical_device *)_ph_dev)->hw_ver)
#define PD_SET_HW_VER(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->hw_ver = _value)
#define PD_GET_GEN_DOWN(_ph_dev) \
	(((struct physical_device *)_ph_dev)->gen_down)
#define PD_SET_GEN_DOWN(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->gen_down = _value)
#define PD_GET_HW_WTBL_SUPPORT(_ph_dev) \
	(((struct physical_device *)_ph_dev)->hw_wtbl_support)
#define PD_SET_HW_WTBL_SUPPORT(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->hw_wtbl_support = _value)


#ifdef SW_CONNECT_SUPPORT
#define PD_GET_HW_WTBL_MAX(ph_dev) \
	((ph_dev->bSwSta) ? (ph_dev->chip_cap.hw_sw_max_entries) : (ph_dev->chip_cap.wtbl_max_entries))
#define PD_GET_HW_UCAST_WTBL_MAX(ph_dev) \
	((ph_dev->bSwSta) ? (ph_dev->chip_cap.hw_sw_max_ucast_entries) : (ph_dev->chip_cap.wtbl_ucast_entries))
#else /* SW_CONNECT_SUPPORT */
#define PD_GET_HW_WTBL_MAX(ph_dev) \
	((ph_dev->chip_cap.wtbl_max_entries))
#define PD_GET_HW_UCAST_WTBL_MAX(ph_dev) \
	((ph_dev->chip_cap.wtbl_ucast_entries))
#endif /* !SW_CONNECT_SUPPORT */
#define PD_GET_HW_DRR_MAX_DW_PER_AC(ph_dev) \
	(ph_dev->chip_cap.drr_max_dw)
#define PD_GET_HW_DRR_MAX_DW_ALL_AC(ph_dev) \
	((ph_dev->chip_cap.drr_max_dw) << 2)
#define PD_GET_HW_DRR_MAX_DW_TWT(ph_dev) \
	(ph_dev->chip_cap.drr_max_dw_twt)
#define PD_GET_HW_DRR_QUERY_PAUSE_BY_CMD(ph_dev) \
	(ph_dev->chip_cap.drr_cmd_query)

#define PD_GET_MAC_PAD_NUM(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->mac_ad_list.size)

#define PD_GET_CHIP_CAP_PTR(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->chip_cap)

#define	PD_GET_ASIC_CAP(_ph_dev) \
	(((struct physical_device *)_ph_dev)->chip_cap.asic_caps)

#define PD_GET_EEPROM_TYPE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->eeprom_type)
#define PD_SET_EEPROM_TYPE(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->eeprom_type = _value)

#define PD_GET_E2P_ACCESS_MODE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->E2pAccessMode)
#define PD_SET_E2P_ACCESS_MODE(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->E2pAccessMode = _value)

#define PD_GET_USE_EFUSE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->bUseEfuse)
#define PD_SET_USE_EFUSE(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->bUseEfuse = _value)

#define PD_GET_E2P_CTRL_PTR(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->E2pCtrl)
#define PD_GET_E2P_CUR_MODE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->E2pCtrl.e2pCurMode)
#define PD_GET_E2P_SOURCE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->E2pCtrl.e2pSource)
#define PD_SET_E2P_SOURCE(_ph_dev, _bit_mask) \
	(((struct physical_device *)_ph_dev)->E2pCtrl.e2pSource |= _bit_mask)
#define PD_CLEAR_E2P_SOURCE(_ph_dev, _bit_mask) \
	(((struct physical_device *)_ph_dev)->E2pCtrl.e2pSource &= ~(_bit_mask))

#define PD_SET_ADMIN_ADDRESS(_ph_dev, _value) \
	COPY_MAC_ADDR(((struct physical_device *)_ph_dev)->LocalAdminMAC, _value)

#define PD_GET_ADMIN_ADDRESS(_ph_dev) \
	(((struct physical_device *)_ph_dev)->LocalAdminMAC)

#define PD_SET_ADMIN_ADDRESS1(_ph_dev, _value) \
	COPY_MAC_ADDR(((struct physical_device *)_ph_dev)->LocalAdminMAC1, _value)

#define PD_GET_ADMIN_ADDRESS1(_ph_dev) \
	(((struct physical_device *)_ph_dev)->LocalAdminMAC1)

#define PD_SET_ADMIN_ADDRESS2(_ph_dev, _value) \
		COPY_MAC_ADDR(((struct physical_device *)_ph_dev)->LocalAdminMAC2, _value)

#define PD_GET_ADMIN_ADDRESS2(_ph_dev) \
		(((struct physical_device *)_ph_dev)->LocalAdminMAC2)

/* QM */
#define PD_GET_QM_PARM(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->qm.qm_parm)

#define PD_GET_QM_CTL(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->qm.qm_ctl)

#define PD_GET_QM_WSYS_NE(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.qm_wsys_ne[_sw_queue_idx])

#define PD_GET_QM_OPS(_ph_dev) \
	(((struct physical_device *)_ph_dev)->qm.qm_ops)

#define PD_SET_QM_OPS(_ph_dev, _qm_ops) \
	(((struct physical_device *)_ph_dev)->qm.qm_ops = _qm_ops)

/* QM Queue */
#define PD_GET_QM_FP_SWQ(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_que[_sw_queue_idx])

#define PD_GET_QM_FP_SWQ_LEN(_ph_dev, _sw_queue_idx) \
	(((struct physical_device *)_ph_dev)->qm.fp_que[_sw_queue_idx].Number)

#define PD_GET_QM_FP_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_que_lock[_sw_queue_idx])

#define PD_GET_QM_FP_POST_SWQ(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_post_que[_sw_queue_idx])

#define PD_GET_QM_FP_POST_SWQ_LEN(_ph_dev, _sw_queue_idx) \
	(((struct physical_device *)_ph_dev)->qm.fp_post_que[_sw_queue_idx].Number)

#define PD_GET_QM_FP_POST_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_post_que_lock[_sw_queue_idx])

#ifdef IGMP_SNOOPING_OFFLOAD
#define PD_GET_QM_FP_BMC_SWQ(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_bmc_que[_sw_queue_idx])

#define PD_GET_QM_FP_BMC_SWQ_LEN(_ph_dev, _sw_queue_idx) \
	(((struct physical_device *)_ph_dev)->qm.fp_bmc_que[_sw_queue_idx].Number)

#define PD_GET_QM_FP_BMC_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_bmc_que_lock[_sw_queue_idx])

#define PD_GET_QM_FP_POST_BMC_SWQ(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_post_bmc_que[_sw_queue_idx])

#define PD_GET_QM_FP_POST_BMC_SWQ_LEN(_ph_dev, _sw_queue_idx) \
	(((struct physical_device *)_ph_dev)->qm.fp_post_bmc_que[_sw_queue_idx].Number)

#define PD_GET_QM_FP_POST_BMC_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.fp_post_bmc_que_lock[_sw_queue_idx])
#endif /* IGMP_SNOOPING_OFFLOAD */

#ifdef HIGH_PRIO_QUEUE_SUPPORT
#define PD_GET_QM_FP_HIGHPRI_SWQ(_ph_dev, _sw_queue_idx) \
		(&((struct physical_device *)_ph_dev)->qm.fp_highpri_que[_sw_queue_idx])

#define PD_GET_QM_FP_HIGHPRI_SWQ_LEN(_ph_dev, _sw_queue_idx) \
		(((struct physical_device *)_ph_dev)->qm.fp_highpri_que[_sw_queue_idx].Number)

#define PD_GET_QM_FP_HIGHPRI_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
		(&((struct physical_device *)_ph_dev)->qm.fp_highpri_que_lock[_sw_queue_idx])

#define PD_GET_QM_FP_POST_HIGHPRI_SWQ(_ph_dev, _sw_queue_idx) \
		(&((struct physical_device *)_ph_dev)->qm.fp_post_highpri_que[_sw_queue_idx])

#define PD_GET_QM_FP_POST_HIGHPRI_SWQ_LEN(_ph_dev, _sw_queue_idx) \
		(((struct physical_device *)_ph_dev)->qm.fp_post_highpri_que[_sw_queue_idx].Number)

#define PD_GET_QM_FP_POST_HIGHPRI_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
		(&((struct physical_device *)_ph_dev)->qm.fp_post_highpri_que_lock[_sw_queue_idx])
#endif

#define PD_GET_QM_MGMT_SWQ(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.mgmt_que[_sw_queue_idx])

#define PD_GET_QM_MGMT_SWQ_LEN(_ph_dev, _sw_queue_idx) \
	(((struct physical_device *)_ph_dev)->qm.mgmt_que[_sw_queue_idx].Number)

#define PD_GET_QM_MGMT_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.mgmt_que_lock[_sw_queue_idx])

#define PD_GET_QM_MGMT_POST_SWQ(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.mgmt_post_que[_sw_queue_idx])

#define PD_GET_QM_MGMT_POST_SWQ_LEN(_ph_dev, _sw_queue_idx) \
	(((struct physical_device *)_ph_dev)->qm.mgmt_post_que[_sw_queue_idx].Number)

#define PD_GET_QM_MGMT_POST_SWQ_LOCK(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.mgmt_post_que_lock[_sw_queue_idx])

#define PD_GET_QM_HIGH_PRI_SWQ(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->qm.high_prio_que)

#define PD_GET_QM_HIGH_PRI_SWQ_LEN(_ph_dev) \
	(((struct physical_device *)_ph_dev)->qm.high_prio_que.Number)

#define PD_GET_QM_HIGH_PRI_SWQ_LOCK(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->qm.high_prio_que_lock)

#define PD_GET_QM_TX_SWQ_PER_BAND(_ph_dev, _band_idx) \
	(((struct physical_device *)_ph_dev)->qm.tx_swq[_band_idx])

#define PD_SET_QM_TX_SWQ_PER_BAND(_ph_dev, _band_idx, _p_tx_swq) \
	(((struct physical_device *)_ph_dev)->qm.tx_swq[_band_idx] = _p_tx_swq)


#define PD_GET_QM_TX_SWQ(_ph_dev, _band_idx, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.tx_swq[_band_idx][_sw_queue_idx])

#define PD_GET_QM_TX_SWQ_LOCK(_ph_dev, _band_idx, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.tx_swq[_band_idx][_sw_queue_idx].swq_lock)

/* TX flow control */
#define PD_GET_QM_FP_TX_FLOW_CTL(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->qm.fp_tx_flow_ctl)

/* TM */
#define PD_GET_TM_QM_OPS(_ph_dev) \
	(((struct physical_device *)_ph_dev)->qm.tm_qm_ops)

#define PD_SET_TM_QM_OPS(_ph_dev, _tm_qm_ops) \
	(((struct physical_device *)_ph_dev)->qm.tm_qm_ops = _tm_qm_ops)

#define PD_GET_TM_TX_DEQ_SCHED(_ph_dev, _sw_queue_idx) \
	(((struct physical_device *)_ph_dev)->qm.tm_tx_deq_sched[_sw_queue_idx])

#define PD_SET_TM_TX_DEQ_SCHED(_ph_dev, _sw_queue_idx, _enable) \
	(((struct physical_device *)_ph_dev)->qm.tm_tx_deq_sched[_sw_queue_idx] = _enable)

#define PD_GET_TM_TX_DEQ_TASKLET(_ph_dev, _sw_queue_idx) \
	(&((struct physical_device *)_ph_dev)->qm.tm_tx_deq_tasklet[_sw_queue_idx])

#ifdef WF_RESET_SUPPORT
#define PD_GET_WF_RESET_STATUS(_ph_dev) \
		(((struct physical_device *)_ph_dev)->wf_reset_state)

#define PD_SET_WF_RESET_STATUS(_ph_dev, _value) \
		(((struct physical_device *)_ph_dev)->wf_reset_state = _value)

#define PD_GET_WF_RESET_IN_PROGRESS(_ph_dev) \
		(((struct physical_device *)_ph_dev)->wf_reset_in_progress)

#define PD_SET_WF_RESET_IN_PROGRESS(_ph_dev, _value) \
		(((struct physical_device *)_ph_dev)->wf_reset_in_progress = _value)

#define PD_GET_WF_RESET_WM_COUNT(_ph_dev) \
		(((struct physical_device *)_ph_dev)->wf_reset_wm_count)

#define PD_SET_WF_RESET_WM_COUNT(_ph_dev, _value) \
		(((struct physical_device *)_ph_dev)->wf_reset_wm_count = _value)

#define PD_GET_WF_RESET_WA_COUNT(_ph_dev) \
		(((struct physical_device *)_ph_dev)->wf_reset_wa_count)

#define PD_SET_WF_RESET_WA_COUNT(_ph_dev, _value) \
		(((struct physical_device *)_ph_dev)->wf_reset_wa_count = _value)

#define PD_GET_WF_RESET_WO_COUNT(_ph_dev) \
		(((struct physical_device *)_ph_dev)->wf_reset_wo_count)

#define PD_SET_WF_RESET_WO_COUNT(_ph_dev, _value) \
		(((struct physical_device *)_ph_dev)->wf_reset_wo_count = _value)

#define PD_GET_WF_CMD_TIMEOUT_COUNT(_ph_dev) \
		(((struct physical_device *)_ph_dev)->FwCmdTimeoutcheckCnt)

#define PD_SET_WF_CMD_TIMEOUT_COUNT(_ph_dev, _value) \
		(((struct physical_device *)_ph_dev)->FwCmdTimeoutcheckCnt = _value)

#endif /* WF_RESET_SUPPORT */

#define PD_GET_BA_CTRL_PTR(ph_dev) \
	(&((struct physical_device *)ph_dev)->ba_ctl)

#ifdef WHNAT_SUPPORT
#define PD_GET_WHNAT_ENABLE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->whnat_en)
#define PD_SET_WHNAT_ENABLE(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->whnat_en = _value)
#endif /* WHNAT_SUPPORT */

#define PD_GET_ASSOC_RECORD_EN(_ph_dev) \
	(((struct physical_device *)_ph_dev)->assoc_record_en)
#define PD_SET_ASSOC_RECORD_EN(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->assoc_record_en = _value)

#define PD_GET_MCU_CTRL_PTR(ph_dev) \
	(&((struct physical_device *)ph_dev)->MCUCtrl)

#define PD_GET_BAND_NUM(_ph_dev) \
	(((struct physical_device *)_ph_dev)->band_num)
#define PD_SET_BAND_NUM(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->band_num = _value)
#define PD_GET_BAND_PROFILE_PATH(_ph_dev, _band_idx) \
	(((struct physical_device *)_ph_dev)->profile_path[_band_idx])
#define PD_GET_BAND_EN(_ph_dev) \
	(((struct physical_device *)_ph_dev)->band_en)
#define PD_SET_BAND_EN(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->band_en = _value)

#ifdef MAC_ADDR_ADJACENT_CHK
	/* Different Band's MAC Hit Adjacent */
#define PD_GET_MAC_ADDR_ADJ(_ph_dev) \
	(((struct physical_device *)_ph_dev)->bMacAddrAdj)
#define PD_SET_MAC_ADDR_ADJ(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->bMacAddrAdj = _value)
#endif /* MAC_ADDR_ADJACENT_CHK */

#ifdef DOT11_EHT_BE
#define PD_GET_MLO_CONFIG_OP_SET(_ph_dev) \
	(((struct physical_device *)_ph_dev)->mlo.mlo_config_op_set)
#define PD_SET_MLO_CONFIG_OP_SET(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->mlo.mlo_config_op_set = _value)
#define PD_GET_MLO_STR_BITMAP(_ph_dev) \
	(((struct physical_device *)_ph_dev)->mlo.str_bitmap)
#define PD_SET_MLO_STR_BITMAP(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->mlo.str_bitmap = _value)
#define PD_GET_MLO_SYNC_TX_ENABLE(_ph_dev) \
	(((struct physical_device *)_ph_dev)->mlo.sync_tx_enable)
#define PD_SET_MLO_SYNC_TX_ENABLE(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->mlo.sync_tx_enable = _value)
#define PD_GET_MLO_V1_ENABLE(_ph_dev) \
		(((struct physical_device *)_ph_dev)->mlo.mlo_v1_enable)
#define PD_CEHCK_MLO_V1_ENABLE(_ph_dev) \
			(((struct physical_device *)_ph_dev)->mlo.mlo_v1_enable == MTK_MLO_V1)
#define PD_CEHCK_MLO_V1_1_ENABLE(_ph_dev) \
				(((struct physical_device *)_ph_dev)->mlo.mlo_v1_enable == MTK_MLO_V1_1)
#define PD_CEHCK_MLO_V1_DISABLE(_ph_dev) \
					(((struct physical_device *)_ph_dev)->mlo.mlo_v1_enable == MTK_MLO_DISABLE)
#define PD_MTK_MLO_V1_ENABLE(_ph_dev) \
			(((struct physical_device *)_ph_dev)->mlo.mlo_v1_enable = MTK_MLO_V1)
#define PD_MTK_MLO_V1_1_ENABLE(_ph_dev) \
				(((struct physical_device *)_ph_dev)->mlo.mlo_v1_enable = MTK_MLO_V1_1)
#define PD_MTK_MLO_V1_DISABLE(_ph_dev) \
				(((struct physical_device *)_ph_dev)->mlo.mlo_v1_enable = MTK_MLO_DISABLE)
#endif /* DOT11_EHT_BE */

#define PD_GET_DEVICE_IDX(_ph_dev) \
	(((struct physical_device *)_ph_dev)->device_idx)
#define PD_GET_DEVICE_IRQ(_ph_dev) \
	(((struct physical_device *)_ph_dev)->irq_lock)

#ifdef CCN67_BS_SUPPORT
#define PD_GET_MESH_ENABLED(_ph_dev) \
	(((struct physical_device *)_ph_dev)->rsc_bsOnMeshAp)

#define PD_SET_MESH_ENABLED(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->rsc_bsOnMeshAp = _value)

#define PD_GET_BB_WITHOLD_LIMIT(_ph_dev) \
	(((struct physical_device *)_ph_dev)->ic_bb_withhold_limit)

#define PD_GET_BB_STA_THRESHOLD(_ph_dev) \
	(((struct physical_device *)_ph_dev)->band_bal_minsta_thresh)

#define PD_SET_BB_STA_THRESHOLD(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->band_bal_minsta_thresh = _value)

#define PD_GET_BB_BAND_THRESHOLD(_ph_dev, _band_idx) \
	(((struct physical_device *)_ph_dev)->ic_bb_percent[_band_idx])
#endif

#if defined(DOT11_SAE_SUPPORT) || defined(SUPP_SAE_SUPPORT)
#define PD_GET_SAE_CFG_PTR(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->sae_cfg)
#endif /* DOT11_SAE_SUPPORT || SUPP_SAE_SUPPORT */
#define PD_GET_PMKID_PTR(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->pmkid_cache)

#ifdef MAT_SUPPORT
#define PD_GET_MAT_CFG_PTR(_ph_dev) \
	(&((struct physical_device *)_ph_dev)->mat_cfg)
#define PD_GET_MAT_CFG_STATUS(_ph_dev) \
	(((struct physical_device *)_ph_dev)->mat_cfg.status)
#endif /* MAT_SUPPORT */

#define PD_GET_EFUSE_BLK_INFO_PTR(ph_dev) \
	(&((struct physical_device *)ph_dev)->EfuseBlkInfo)
#define PD_GET_DEVICE_TASK_PTR(ph_dev) \
	(&((struct physical_device *)ph_dev)->pd_thread_task)
#define PD_GET_DEVICE_NAME(ph_dev) \
	(&((struct physical_device *)ph_dev)->device_name[0])

#define PD_GET_RRO_TIMEOUT_PROFILE_SET(_ph_dev) \
	(((struct physical_device *)_ph_dev)->rro_timeout_profile_set)
#define PD_SET_RRO_TIMEOUT_PROFILE_SET(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->rro_timeout_profile_set = _value)

#define PD_GET_11V_BCN_DUP(_ph_dev) \
	(((struct physical_device *)_ph_dev)->bHost_11vmbss_dup_bcn)
#define PD_SET_11V_BCN_DUP(_ph_dev, _value) \
	(((struct physical_device *)_ph_dev)->bHost_11vmbss_dup_bcn = _value)

#define RTMPInitTimer(_pAd, _pTimer, _pTimerFunc, _pData, _Repeat) \
physical_device_init_timer(\
	_pAd, ((struct _RTMP_ADAPTER *)_pAd)->physical_dev, \
	_pTimer, _pTimerFunc, _pData, _Repeat, #_pTimer)

#endif  /* __PHYSICAL_DEVICE_DEF_H__ */

