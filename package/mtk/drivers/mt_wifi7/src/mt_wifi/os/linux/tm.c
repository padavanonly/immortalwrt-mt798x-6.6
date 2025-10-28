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
/***************************************************************************
 ***************************************************************************

*/
#include "rt_config.h"

static VOID ge_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	ge_tx_pkt_deq_func(pAd, hc_get_hw_band_idx(pAd));
}

static VOID fp_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);

	qm_ops->deq_tx_pkt(pAd, hc_get_hw_band_idx(pAd));
}

static INT tm_tasklet_qm_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 band_idx = hc_get_hw_band_idx(pAd);

	if (cap->qm == FAST_PATH_QM) {
		RTMP_OS_TASKLET_INIT(pAd, PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, band_idx), fp_tx_pkt_deq_tasklet, (unsigned long)pAd);
		PD_SET_TM_TX_DEQ_SCHED(pAd->physical_dev, band_idx, TRUE);
	} else if (IS_GE_QM(cap->qm)) {
		RTMP_OS_TASKLET_INIT(pAd, PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, band_idx), ge_tx_pkt_deq_tasklet, (unsigned long)pAd);
		PD_SET_TM_TX_DEQ_SCHED(pAd->physical_dev, band_idx, TRUE);
	}

	return ret;
}

static INT tm_tasklet_qm_exit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	UINT8 band_idx = hc_get_hw_band_idx(pAd);
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->qm == FAST_PATH_QM) {
		PD_SET_TM_TX_DEQ_SCHED(pAd->physical_dev, band_idx, FALSE);
		RTMP_OS_TASKLET_KILL(PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, band_idx));
	} else {
		PD_SET_TM_TX_DEQ_SCHED(pAd->physical_dev, band_idx, FALSE);
		RTMP_OS_TASKLET_KILL(PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, band_idx));
	}

	return ret;
}

static INT tm_tasklet_qm_schedule_task(RTMP_ADAPTER *pAd, enum task_type type, UINT8 idx)
{
	INT ret = NDIS_STATUS_SUCCESS;

	switch (type) {
	case TX_DEQ_TASK:
		if (PD_GET_TM_TX_DEQ_SCHED(pAd->physical_dev, idx)) {
			RTMP_OS_TASKLET_SCHE(PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, idx));
		}

		break;

	case RX_DEQ_TASK:
		break;

	default:

		break;
	}

	return ret;
}

static VOID tx_deq0_schedule_cpu(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASKLET_SCHE(PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, 0));
}

static VOID tx_deq1_schedule_cpu(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASKLET_SCHE(PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, 1));
}

static VOID tx_deq2_schedule_cpu(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASKLET_SCHE(PD_GET_TM_TX_DEQ_TASKLET(pAd->physical_dev, 2));
}

static INT tm_tasklet_qm_schedule_task_on(RTMP_ADAPTER *pAd, int cpuid, enum task_type type, UINT8 idx)
{
	INT ret = NDIS_STATUS_SUCCESS;

	switch (type) {
	case TX_DEQ_TASK:
		if (PD_GET_TM_TX_DEQ_SCHED(pAd->physical_dev, idx)) {
			if (idx == 0)
				smp_call_function_single(cpuid, (smp_call_func_t)tx_deq0_schedule_cpu, pAd, 0);
			else if (idx == 1)
				smp_call_function_single(cpuid, (smp_call_func_t)tx_deq1_schedule_cpu, pAd, 0);
			else
				smp_call_function_single(cpuid, (smp_call_func_t)tx_deq2_schedule_cpu, pAd, 0);
		}

		break;

	default:

		break;
	}

	return ret;
}

#if defined(EXPERIMENTAL_FEATURES) && defined(LINUX)
#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
static INT tm_tasklet_qm_schedule_task_async_on(RTMP_ADAPTER *pAd, int cpuid, enum task_type type, uint8_t idx)
{
	INT ret = NDIS_STATUS_SUCCESS;
	call_single_data_t *csd = &pAd->csd;

	switch (type) {
	case TX_DEQ_TASK:
		if (PD_GET_TM_TX_DEQ_SCHED(pAd->physical_dev, idx) &&
				!pAd->tx_schedule_run) {
			pAd->tx_schedule_run = 1;

			csd->info = pAd;
			if (idx == 0)
				csd->func = (smp_call_func_t)tx_deq0_schedule_cpu;
			else if (idx == 1)
				csd->func = (smp_call_func_t)tx_deq1_schedule_cpu;
			else
				csd->func = (smp_call_func_t)tx_deq2_schedule_cpu;
			smp_call_function_single_async(cpuid, csd);
		}
		break;
	default:
		break;
	}
	return ret;
}
#endif
#endif

struct tm_ops tm_tasklet_qm_ops = {
	.init = tm_tasklet_qm_init,
	.exit = tm_tasklet_qm_exit,
	.schedule_task = tm_tasklet_qm_schedule_task,
	.schedule_task_on = tm_tasklet_qm_schedule_task_on,
#if defined(EXPERIMENTAL_FEATURES) && defined(LINUX)
#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
	.schedule_task_async_on = tm_tasklet_qm_schedule_task_async_on,
#endif
#endif
};

INT tm_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tm_ops *tm_qm_ops = NULL;
	INT ret = NDIS_STATUS_SUCCESS;

	if (cap->qm_tm == TASKLET_METHOD)
		PD_SET_TM_QM_OPS(pAd->physical_dev, &tm_tasklet_qm_ops);

	tm_qm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	ret = tm_qm_ops->init(pAd);

	return ret;
}

INT tm_exit(RTMP_ADAPTER *pAd)
{
	struct tm_ops *tm_qm_ops = PD_GET_TM_QM_OPS(pAd->physical_dev);
	INT ret = NDIS_STATUS_SUCCESS;

	ret = tm_qm_ops->exit(pAd);

	return ret;
}
