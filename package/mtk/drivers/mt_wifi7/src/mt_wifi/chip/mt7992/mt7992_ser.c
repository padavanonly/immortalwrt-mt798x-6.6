/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2022, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#ifdef MT7992

#include "rt_config.h"
#include "mt7992_ser.h"
#include "mt7992_coda/wf_cr_sw_def.h"
#include "chip/mt7992_cr.h"

#ifdef ERR_RECOVERY
void mt7992_ser_1_0_v1(struct ser_cb_t *ser_cb)
{
	UINT32 chip_id = ser_cb->chip_id;
	UINT32 hw_id = ser_cb->hw_id;
	UINT8 level = ser_cb->ser_level;
	UINT8 state = ser_cb->ser_state;
	UINT32 event = ser_cb->ser_event;
	struct _RTMP_ADAPTER *pAd_list[MAX_NUM_OF_INF] = {NULL};
	struct _RTMP_ADAPTER *main_pAd, *pAd;
	struct physical_device *device;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl;
	UINT32 *pSerTimes = NULL;
	struct wifi_dev *wdev = NULL;
	int i, pAd_num;
	INT8 ret = 0;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
		"chip_id=0x%x,hw_id=%d,level=%d,state=%d,event=0x%x\n",
		chip_id, hw_id, level, state, event);

	pAd_num = ser_get_all_pAd_by_chipid_hwid(pAd_list, chip_id, hw_id);
	main_pAd = pAd_list[0];

	if (pAd_num == 0 || main_pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"pAd_num is 0 or main_pAd is NULL\n");
		return;
	}

#ifdef CONFIG_ATE
	if (ATE_ON(main_pAd)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"The driver is in ATE mode now\n");
		return;
	}
#endif /* CONFIG_ATE */

	device = main_pAd->physical_dev;
	pErrRecoveryCtrl = &device->ErrRecoveryCtl;
	pErrRecoveryCtrl->errRecovStage = state;
	pSerTimes = &ser_cb->ser_times[0];
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO, "state=0x%08X\n", state);

	switch (state) {
	case SER_LV_1_0_STATE_STOP_IDLE:
	case SER_LV_1_0_STATE_EVENT_REENTRY:
		/* Start L1 SER and notify Hwifi change state */
		asic_ser_handler(main_pAd, SER_ACTION_L1_START_END, 1);

		if ((event & SER_LV_1_0_EVT_STOP_PDMA) == SER_LV_1_0_EVT_STOP_PDMA) {
			os_zero_mem(pSerTimes, (sizeof(pSerTimes[SER_TIME_ID_T0]) * SER_TIME_ID_END));
			pSerTimes[SER_TIME_ID_T0] = ser_get_time();

			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO, "SER TIMEOUT check\n");
			RTMPInitTimer(main_pAd,
						  &ser_cb->ser_timer,
						  GET_TIMER_FUNCTION(ser_l1_timeout),
						  main_pAd,
						  TRUE);
			RTMPSetTimer(&ser_cb->ser_timer, SER_LV_1_0_TIMEOUT);
			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_STOP_IDLE_DONE);

			for (i = 0; i < pAd_num; i++) {
				pAd = pAd_list[i];
				if (pAd) {
					RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP | fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
				}
			}

			for (i = 0; i < pAd_num; i++) {
				pAd = pAd_list[i];
				if (pAd) {
					/* exit tasklet mgmt */
					tm_exit(pAd);
				}
			}

			/* exit DMA_SW rxq queues */

			/* send PDMA0 stop to N9 through interrupt. */
			ser_ack_event(ser_cb, SER_LV_1_0_INT_PDMA0_STOP_DONE);

			/* all mmio need to be stop till hw reset done. */
			for (i = 0; i < pAd_num; i++) {
				pAd = pAd_list[i];
				if (pAd)
					asic_ser_handler(pAd, SER_ACTION_IO_SWITCH, 0);
			}

			RtmpusecDelay(100 * 1000); /* delay for 100 ms to wait reset done. */

			for (i = 0; i < pAd_num; i++) {
				pAd = pAd_list[i];
				if (pAd)
					asic_ser_handler(pAd, SER_ACTION_IO_SWITCH, 1);
			}

			/*re-call for change status to stop dma0*/
			mt7992_ser_1_0_v1(ser_cb);
			pSerTimes[SER_TIME_ID_T1] = ser_get_time();
		} else {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"!!! SER CurStat=%u Event=0x%x !!!\n",
				ser_get_curr_state(main_pAd), event);
		}

		break;

	case SER_LV_1_0_STATE_STOP_IDLE_DONE:
#ifdef MTK_FE_RESET_RECOVER
		if (atomic_read(&pErrRecoveryCtrl->notify_fe)) {
			struct _RTMP_ADAPTER *pAd2;

			atomic_set(&pErrRecoveryCtrl->notify_fe, 0);
			ser_ack_event(ser_cb, FE_WIFI_RESET_DONE);
			pAd2 = ser_get_fe_pAd_by_mainpAd(main_pAd);
			RTMP_SET_FLAG(pAd2, fRTMP_ADAPTER_START_UP);
			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_STOP_PDMA0_PRE);
		} else
#endif
			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_STOP_PDMA0);

		break;

#ifdef MTK_FE_RESET_RECOVER
		case  SER_LV_1_0_STATE_STOP_PDMA0_PRE:

		if ((event & SER_LV_1_0_EVT_STOP_PDMA_PRE) == SER_LV_1_0_EVT_STOP_PDMA_PRE) {
			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_STOP_PDMA0);
			ser_cb->ser_event = SER_LV_1_0_EVT_RESET_DONE;
			mt7992_ser_1_0_v1(ser_cb);
		}
		break;
#endif

	case SER_LV_1_0_STATE_STOP_PDMA0:
		if ((event & SER_LV_1_0_EVT_RESET_DONE) == SER_LV_1_0_EVT_RESET_DONE) {

			pSerTimes[SER_TIME_ID_T2] = ser_get_time();

			for (i = 0; i < pAd_num; i++) {
				pAd = pAd_list[i];
				/* exit queue mgmt */
				if (pAd)
					qm_exit_perband(pAd);
			}

			ret = asic_ser_handler(main_pAd, SER_ACTION_SET_DMA_TK, 0);
#ifdef MTK_FE_RESET_RECOVER
			if (ret) {
				atomic_set(&pErrRecoveryCtrl->notify_fe, 1);
				ser_cb->need_reset = TRUE;
			}
#endif
			asic_ser_handler(main_pAd, SER_ACTION_SET_DMA_TK, 1);

			for (i = 0; i < pAd_num; i++) {
				pAd = pAd_list[i];
				if (pAd) {
				/* init queue and tasklet mgmt */
					qm_init_perband(pAd);
					tm_init(pAd);
				}
			}
			for (i = 0; i < pAd_num; i++) {
				pAd = pAd_list[i];
				if (pAd) {
					/* restore flags */
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP | fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
				}
			}

			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_RESET_PDMA0);

			/* send PDMA0 reinit done to N9 through interrupt. */
			ser_ack_event(ser_cb, SER_LV_1_0_INT_PDMA0_INIT_DONE);

			pSerTimes[SER_TIME_ID_T3] = ser_get_time();
		} else {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"!!! SER CurStat=%u Event=0x%x !!!\n",
				ser_get_curr_state(main_pAd), event);
		}

		break;

	case SER_LV_1_0_STATE_RESET_PDMA0:
		if ((event & SER_LV_1_0_EVT_RECOVERY_DONE)
			== SER_LV_1_0_EVT_RECOVERY_DONE) {
			pSerTimes[SER_TIME_ID_T4] = ser_get_time();
			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_WAIT_N9_NORMAL);
			ser_ack_event(ser_cb, SER_LV_1_0_INT_PDMA0_RECV_DONE);
			pSerTimes[SER_TIME_ID_T5] = ser_get_time();
		} else {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"!!! SER CurStat=%u Event=0x%x !!!\n",
				ser_get_curr_state(main_pAd), event);
		}

		break;

	case SER_LV_1_0_STATE_WAIT_N9_NORMAL:
		if ((event & SER_LV_1_0_EVT_STOP_PDMA)
				   == SER_LV_1_0_EVT_STOP_PDMA) {
			pSerTimes[SER_TIME_ID_T6] = ser_get_time();
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				"!!! ERROR SER re-entry  CurStat=%u Event=0x%x !!!\n",
				ser_get_curr_state(main_pAd), event);
			pSerTimes[SER_TIME_ID_T7] = ser_get_time();

			/*print out ser log timing*/
			if (_ser_dump_timelog(ser_cb))
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				"SER dump timelog error\n");

			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_EVENT_REENTRY);
			mt7992_ser_1_0_v1(ser_cb);
		} else if ((event & SER_LV_1_0_EVT_N9_NORMAL_STATE)
			== SER_LV_1_0_EVT_N9_NORMAL_STATE) {
			BOOLEAN Cancelled = TRUE;
			pSerTimes[SER_TIME_ID_T6] = ser_get_time();
			ser_tans_ser_state(ser_cb, SER_LV_1_0_STATE_STOP_IDLE);
			pSerTimes[SER_TIME_ID_T7] = ser_get_time();

			/*print out ser log timing*/
			if (_ser_dump_timelog(ser_cb))
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				"SER dump timelog error\n");

			for (i = 0; i < pAd_num; i++) {
				UCHAR j;

				pAd = pAd_list[i];
				if (!pAd)
					continue;

				/* send BAR to all STAs */
				ba_refresh_bar_all(pAd);
				/* update Beacon frame if operating in AP mode. */
				for (j = 0; j < WDEV_NUM_MAX; j++) {
					wdev = pAd->wdev_list[j];
					if (wdev && HcIsRadioAcq(wdev) &&
						(wdev->wdev_type == WDEV_TYPE_AP)) {
						UpdateBeaconHandler(
							pAd,
							wdev,
							BCN_REASON(BCN_UPDATE_ALL_AP_RENEW));
					}
				}
			}

			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
					 "RELEASE SER TIMEOUT TIMER\n");
			RTMPReleaseTimer(&ser_cb->ser_timer, &Cancelled);
		} else {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				"!!! SER CurStat=%u Event=0x%x !!!\n",
				ser_get_curr_state(main_pAd), event);
		}

		/* End L1 SER and notify Hwifi change state */
		asic_ser_handler(main_pAd, SER_ACTION_L1_START_END, 0);
#ifdef MTK_FE_RESET_RECOVER
		if (ser_cb->need_reset) {
			ser_cb->need_reset = FALSE;
			if (atomic_read(&pErrRecoveryCtrl->notify_fe)) {
				atomic_set(&pErrRecoveryCtrl->notify_fe, 0);
				ser_ack_event(ser_cb, FE_WIFI_WDMA_FAIL_RESET);
			}
		}
#endif
			break;

	default:
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
			"!!! SER CurStat=%u Event=0x%x !!!\n",
			ser_get_curr_state(main_pAd), event);
		break;
	}
}

void mt7992_ser_10_0_v1(struct ser_cb_t *ser_cb)
{
	UINT32 chip_id = ser_cb->chip_id;
	UINT32 hw_id = ser_cb->hw_id;
	struct _RTMP_ADAPTER *pAd_list[MAX_NUM_OF_INF] = {NULL};
	struct _RTMP_ADAPTER *pAd;
	int pAd_num;

	pAd_num = ser_get_all_pAd_by_chipid_hwid(pAd_list, chip_id, hw_id);
	pAd = pAd_list[0];
	if (pAd_num == 0 || pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"pAd_num is 0 or main_pAd is NULL\n");
		return;
	}

	UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_ETH_HANDLER_RECOVER, DBDC_BAND0);
}


void mt7992_ser_0_5_v1(struct ser_cb_t *ser_cb)
{
	UINT32 chip_id = ser_cb->chip_id;
	UINT32 hw_id = ser_cb->hw_id;
	UINT8 level = ser_cb->ser_level;
	UINT8 state = ser_cb->ser_state;
	UINT32 event = ser_cb->ser_event;
#ifdef WF_RESET_SUPPORT
#define MT_WM_WDT_INT 0x40000000
#define MT_WA_WDT_INT 0X80000000
	struct _RTMP_ADAPTER *pAd_list[MAX_NUM_OF_INF] = {NULL};
	struct _RTMP_ADAPTER *pAd;
	int pAd_num;

	pAd_num = ser_get_all_pAd_by_chipid_hwid(pAd_list, chip_id, hw_id);
	pAd = pAd_list[0];
	if (ser_cb->ser_event & MT_WA_WDT_INT)
		PD_SET_WF_RESET_WA_COUNT(pAd->physical_dev, PD_GET_WF_RESET_WA_COUNT(pAd->physical_dev) + 1);
	else if (ser_cb->ser_event & MT_WM_WDT_INT)
		PD_SET_WF_RESET_WM_COUNT(pAd->physical_dev, PD_GET_WF_RESET_WM_COUNT(pAd->physical_dev) + 1);
	else
		PD_SET_WF_RESET_WO_COUNT(pAd->physical_dev, PD_GET_WF_RESET_WO_COUNT(pAd->physical_dev) + 1);

	wf_reset_func(pAd->physical_dev);
#endif

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
		"chip_id=0x%x,hw_id=%d,level=%d,state=%d,event=0x%x\n",
		chip_id, hw_id, level, state, event);

}

void mt7992_ser_0_0_v1(struct ser_cb_t *ser_cb)
{
	UINT32 chip_id = ser_cb->chip_id;
	UINT32 hw_id = ser_cb->hw_id;
	UINT8 level = ser_cb->ser_level;
	UINT8 state = ser_cb->ser_state;
	UINT32 event = ser_cb->ser_event;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
		"chip_id=0x%x,hw_id=%d,level=%d,state=%d,event=0x%x\n",
		chip_id, hw_id, level, state, event);

}

void mt7992_dump_ser_stat(struct _RTMP_ADAPTER *ad, UINT8 dump_all)
{
	UINT32 reg_tmp_val = 0;
	struct ser_dump_list {
		char *name;
		UINT32 reg;
	} cr_list[] = {
		{"SER_STATUS       ", WF_SW_DEF_CR_SER_STATUS_ADDR},
		{"SER_PLE_ERR      ", WF_SW_DEF_CR_PLE_STATUS_ADDR},
		{"SER_PLE_ERR_1    ", WF_SW_DEF_CR_PLE1_STATUS_ADDR},
		{"SER_PLE_ERR_AMSDU", WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR},
		{"SER_PSE_ERR      ", WF_SW_DEF_CR_PSE_STATUS_ADDR},
		{"SER_PSE_ERR_1    ", WF_SW_DEF_CR_PSE1_STATUS_ADDR},
		{"SER_LMAC_WISR6_B0", WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR},
		{"SER_LMAC_WISR6_B1", WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR},
		{"SER_LMAC_WISR6_B2", WF_SW_DEF_CR_LAMC_WISR6_BN2_STATUS_ADDR},
		{"SER_LMAC_WISR7_B0", WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR},
		{"SER_LMAC_WISR7_B1", WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR},
		{"SER_LMAC_WISR7_B2", WF_SW_DEF_CR_LAMC_WISR7_BN2_STATUS_ADDR},
		{"SER_WFDMA_ERR    ", WF_SW_DEF_CR_WFDMA_STATUS_ADDR},
	};

	struct ser_dump_list dma_cr_list[] = {
		/* WM -> WA */
		{"WF_MCU_WA_CIRQ_IRQ_MASK",	WF_MCU_WA_CIRQ_IRQ_MASK_ADDR},
		{"WM2WA_ACTION",			WF_SW_DEF_CR_WM2WA_ACTION_ADDR},
		{"WF_MCU_WA_CIRQ_IRQ_SOFT",	WF_MCU_WA_CIRQ_IRQ_SOFT_ADDR},
		{"WA_SER_STEP",				WA_SER_STEP_ADDR},
		/* WA -> WM */
		{"CONN_MCU_CIRQ_IRQ_MASK",	CONN_MCU_CIRQ_IRQ_MASK_ADDR},
		{"WA2WM_ACTION_ADDR",		WF_SW_DEF_CR_WA2WM_ACTION_ADDR},
		{"CONN_MCU_CIRQ_IRQ_SOFT",	CONN_MCU_CIRQ_IRQ_SOFT_ADDR},
		/* Host -> WM */
		{"WF_WFDMA_MCU_DMA0_MCU_INT_ENA", WF_WFDMA_MCU_DMA0_MCU_INT_ENA_ADDR},
		{"WF_WFDMA_MCU_DMA0_MCU_INT_STA", WF_WFDMA_MCU_DMA0_MCU_INT_STA_ADDR},
		/* WM -> Host */
		{"WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_ENA", WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_ENA_ADDR},
		{"WF_WFDMA_HOST_DMA0_HOST_INT_ENA", WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR},
		{"WF_WFDMA_HOST_DMA0_HOST_INT_STA", WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR},
		{"WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA", WF_WFDMA_HOST_DMA0_MCU2HOST_SW_INT_STA_ADDR},
	};

	UINT i, cr_list_num = ARRAY_SIZE(cr_list);
	UINT dma_cr_list_num = ARRAY_SIZE(dma_cr_list);
	UINT war_list_num;
	UINT32 en_mask = 0;
	struct war_dump_list {
		char *name;
		UINT32 en_bit;
		UINT32 reg;
	} *war_list;


	struct war_dump_list war_list_e1[] = {
	};

	struct war_dump_list war_list_e2[] = {
	};

	if (MTK_REV_ET(ad, MT7992, MT7992E1)) {
		war_list = war_list_e1;
		war_list_num = ARRAY_SIZE(war_list_e1);
	} else {
		war_list = war_list_e2;
		war_list_num = ARRAY_SIZE(war_list_e2);
	}

	for (i = 0; i < cr_list_num; i++) {
		MAC_IO_READ32(ad->hdev_ctrl, cr_list[i].reg, &reg_tmp_val);
		if (reg_tmp_val != 0) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
				",::E  R , %s = 0x%08X\n",
				cr_list[i].name, reg_tmp_val);
		}
	}

	if (dump_all) {
		/* dump HWITS workaround info */
		/* TODO: WF_SW_DEF_HWITS_WAR_EN_MASK is not defined yet */
		/*MAC_IO_READ32(ad->hdev_ctrl, WF_SW_DEF_HWITS_WAR_EN_MASK, &en_mask);*/
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
				",::E  R , HWITS_WAR_EN_MASK = 0x%08X\n",
				en_mask);

		for (i = 0; i < war_list_num; i++) {
			MAC_IO_READ32(ad->hdev_ctrl, war_list[i].reg, &reg_tmp_val);
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
					"\t(WAR_%s) %s=%d\n",
					(en_mask & war_list[i].en_bit) ? "ENA" : "DIS",
					war_list[i].name, reg_tmp_val);
		}
	}

	for (i = 0; i < dma_cr_list_num; i++) {
		MAC_IO_READ32(ad->hdev_ctrl, dma_cr_list[i].reg, &reg_tmp_val);
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_WARN,
				 "%s = 0x%08X\n",
				 dma_cr_list[i].name, reg_tmp_val);
	}
}

#ifdef MTK_FE_RESET_RECOVER
void mt7992_fe_reset(struct ser_cb_t *ser_cb)
{
	UINT32 chip_id = ser_cb->chip_id;
	UINT32 hw_id = ser_cb->hw_id;
	struct _RTMP_ADAPTER *pAd_list[MAX_NUM_OF_INF] = {NULL};
	struct _RTMP_ADAPTER *pAd;
	UINT32 event = ser_cb->ser_event;
	int pAd_num;

	pAd_num = ser_get_all_pAd_by_chipid_hwid(pAd_list, chip_id, hw_id);
	pAd = pAd_list[0];
	if (pAd_num == 0 || pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"pAd_num is 0 or main_pAd is NULL\n");
		return;
	}

	if (event == SER_FE_STOP) {
		if (asic_ser_handler(pAd, SER_ACTION_SET_TRAFFIC, 0)) {
			ser_ack_event(ser_cb, FE_WIFI_TARFFIC_DONE_FAIL);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				"SER ack event = %x failed stop\n",
				FE_WIFI_TARFFIC_DONE);
		} else
			ser_ack_event(ser_cb, FE_WIFI_TARFFIC_DONE);
	}

	if (event == SER_FE_START)
		if (asic_ser_handler(pAd, SER_ACTION_SET_TRAFFIC, 1))
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				"SER ack event = %x failed start\n",
				FE_WIFI_TARFFIC_DONE);

}

void mt7992_fe_wdma_reset(struct ser_cb_t *ser_cb)
{
	UINT32 chip_id = ser_cb->chip_id;
	UINT32 hw_id = ser_cb->hw_id;
	struct _RTMP_ADAPTER *pAd_list[MAX_NUM_OF_INF] = {NULL};
	struct _RTMP_ADAPTER *pAd;
	UINT32 event = ser_cb->ser_event;
	int pAd_num;

	pAd_num = ser_get_all_pAd_by_chipid_hwid(pAd_list, chip_id, hw_id);
	pAd = pAd_list[0];
	if (pAd_num == 0 || pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"pAd_num is 0 or main_pAd is NULL\n");
		return;
	}

	if (event == SER_FE_STOP)
		UniCmdSER(pAd, UNI_SER_ACTION_SET_TRIGGER, SER_SET_ETH_HANDLER_RECOVER, DBDC_BAND0);
}

#endif


#endif /* ERR_RECOVERY */
#endif /* MT7992 */
