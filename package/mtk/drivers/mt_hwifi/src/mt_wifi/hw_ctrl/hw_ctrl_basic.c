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
	hw_ctrl_basic.c
*/
#include "rt_config.h"
#include "hw_ctrl_basic.h"

extern HW_CMD_TABLE_T *HwCmdTable[];

/*==========================================================/
 //	Basic Command API implement															/
/==========================================================*/
static inline HwCmdHdlr HwCtrlValidCmd(HwCmdQElmt *CmdQelmt)
{
	UINT32 CmdType =  CmdQelmt->type;
	UINT32 CmdIndex = CmdQelmt->command;
	SHORT CurIndex = 0;
	HwCmdHdlr Handler = NULL;
	HW_CMD_TABLE_T  *pHwTargetTable = NULL;

	if (CmdType >= HWCMD_TYPE_END) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
				 "CMD TPYE(%u) OOB error! HWCMD_TYPE_END %u\n",
				  CmdType, HWCMD_TYPE_END);
		return NULL;
	}

	if (CmdIndex >= HWCMD_ID_END) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
				 "CMD ID(%u) OOB error! HWCMD_ID_END %u\n",
				  CmdIndex, HWCMD_ID_END);
		return NULL;
	}

	pHwTargetTable = HwCmdTable[CmdType];

	if (!pHwTargetTable) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
				 "No HwCmdTable entry for this CMD %u Type %u\n",
				  CmdIndex, CmdType);
		return NULL;
	}

	CurIndex = 0;

	do {
		if (pHwTargetTable[CurIndex].CmdID == CmdIndex) {
			Handler = pHwTargetTable[CurIndex].CmdHdlr;
			pHwTargetTable[CurIndex].RfCnt++;
			break;
		}

		CurIndex++;
	} while (pHwTargetTable[CurIndex].CmdHdlr != NULL);

	if (Handler == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
				 "No corresponding CMDHdlr for this CMD %u Type %u\n",
				  CmdIndex, CmdType);
	}

	return Handler;
}

static VOID HwCtrlDequeueCmd(HwCmdQ *cmdq, HwCmdQElmt **pcmdqelmt)
{
	*pcmdqelmt = cmdq->head;

	if (*pcmdqelmt != NULL) {
		cmdq->head = cmdq->head->next;
		cmdq->size--;

		if (cmdq->size == 0)
			cmdq->tail = NULL;
	}
}

static VOID free_hwcmd(os_kref *ref)
{
	struct _HwCmdQElmt *cmd = container_of(ref, struct _HwCmdQElmt, refcnt);

	if (cmd->NeedWait)
		RTMP_OS_EXIT_COMPLETION(&cmd->ack_done);

	if (cmd->buffer != NULL) {
		os_free_mem(cmd->buffer);
		cmd->buffer = NULL;
	}

	if (cmd->RspBuffer != NULL) {
		os_free_mem(cmd->RspBuffer);
		cmd->RspBuffer = NULL;
	}

	os_free_mem(cmd);
}

static VOID HwCtrlCmdHandler(struct physical_device *ph_dev)
{
	PHwCmdQElmt	cmdqelmt;
	NTSTATUS		ntStatus;
	HwCmdHdlr		Handler = NULL;
	HW_CTRL_T *pHwCtrl = NULL;
	UINT32			process_cnt = 0;
	RTMP_ADAPTER *pAd = NULL;
	UINT32 qlen;

	if (ph_dev)
		pHwCtrl = &ph_dev->HwCtrl;

	if (!pHwCtrl)
		return;

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	qlen = pHwCtrl->HwCtrlQ.size;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	while (ph_dev && qlen > 0) {
		/* For worst case, avoid process HwCtrlQ too long which cause RCU_sched stall */
		process_cnt++;
		if ((!in_interrupt()) && (process_cnt >= HWCTRL_QUE_SCH)) {/*process_cnt-16*/
			process_cnt = 0;
			OS_SCHEDULE();
		}

		if (pAd && (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))) {
			MTWF_DBG(pAd, DBG_CAT_MLME, CATHW_CMD_CTRL, DBG_LVL_INFO,
				"System halted, exit!(HwCtrlQ.size = %d)\n",
				qlen);
			break;
		}

		NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &cmdqelmt);
		NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

		if (cmdqelmt == NULL)
			break;


		Handler = HwCtrlValidCmd(cmdqelmt);
		pAd = (RTMP_ADAPTER *)cmdqelmt->pAd;
		if (pAd == NULL)
			goto free_cmd;

		if (Handler) {
			ntStatus = Handler(pAd, cmdqelmt);

			if (cmdqelmt->CallbackFun)
				cmdqelmt->CallbackFun(pAd, cmdqelmt->CallbackArgs);
		}

#ifdef DBG_STARVATION
		starv_dbg_put(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/
free_cmd:
		/*complete*/
		if (cmdqelmt->NeedWait)
			RTMP_OS_COMPLETE(&cmdqelmt->ack_done);

		os_kref_put(&cmdqelmt->refcnt, free_hwcmd);
	}	/* end of while */
}

static INT HwCtrlThread(ULONG Context)
{
	struct physical_device *ph_dev = NULL;
	RTMP_OS_TASK *pTask;
	HwCmdQElmt	*pCmdQElmt = NULL;
	HW_CTRL_T *pHwCtrl;
	int status;
	UCHAR index = 0;
	UINT32 CmdQState;

	status = 0;
	pTask = (RTMP_OS_TASK *)Context;
	ph_dev = (struct physical_device *)RTMP_OS_TASK_DATA_GET(pTask);


	if (ph_dev == NULL)
		return 0;

	pHwCtrl = &ph_dev->HwCtrl;
	RtmpOSTaskCustomize(pTask);
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	CmdQState = pHwCtrl->HwCtrlQ.CmdQState;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	while (CmdQState == RTMP_TASK_STAT_RUNNING) {
		if (RtmpOSTaskWait(NULL, pTask, &status) == FALSE) {
			for (index = 0; index < PD_GET_BAND_NUM(ph_dev); index++) {
				RTMP_ADAPTER *ad = NULL;

				ad = physical_device_get_mac_adapter_by_band(ph_dev, index);
				if (!ad)
					continue;
				RTMP_SET_FLAG(ad, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			}
			break;
		}
		NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
		if (pHwCtrl->HwCtrlQ.CmdQState == RTMP_TASK_STAT_STOPED) {
			NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
			break;
		}
		CmdQState = pHwCtrl->HwCtrlQ.CmdQState;
		NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
		/*every time check command formate event*/
		HwCtrlCmdHandler(ph_dev);
		pHwCtrl->TotalCnt++;
	}

	/* Clear the CmdQElements. */
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_STOPED;

	while (pHwCtrl->HwCtrlQ.size) {
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &pCmdQElmt);

		if (pCmdQElmt) {
#ifdef DBG_STARVATION
			starv_dbg_put(&pCmdQElmt->starv);
#endif /*DBG_STARVATION*/
			/*complete*/
			if (pCmdQElmt->NeedWait)
				RTMP_OS_COMPLETE(&pCmdQElmt->ack_done);

			os_kref_put(&pCmdQElmt->refcnt, free_hwcmd);
		}
	}

	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_INFO, "<---\n");
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}


#ifdef ERR_RECOVERY
static INT ser_ctrl_task(ULONG context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *task;
	HW_CTRL_T *hw_ctrl;
	int status = 0;

	task = (RTMP_OS_TASK *)context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(task);

	if (pAd == NULL)
		return 0;

	hw_ctrl = &pAd->HwCtrl;
	RtmpOSTaskCustomize(task);
	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);

	while (task && !RTMP_OS_TASK_IS_KILLED(task)) {
		if (RtmpOSTaskWait(pAd, task, &status) == FALSE)
			break;

		HwRecoveryFromError(pAd);
	}

	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_UNKNOWN;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);
	status = RtmpOSTaskNotifyToExit(task);
	return status;
}


INT ser_init(RTMP_ADAPTER *pAd)
{
	INT Status = 0, ret = 0;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	RTMP_OS_TASK *task = &hw_ctrl->ser_task;
	char task_name[TASK_NAME_LEN] = {0};
	u8 band_idx = hc_get_hw_band_idx(pAd);

	if (band_idx >= CFG_WIFI_RAM_BAND_NUM) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"invalid band index(=%d)\n", band_idx);
		return NDIS_STATUS_FAILURE;
	}
	NdisAllocateSpinLock(pAd, &hw_ctrl->ser_lock);
	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_INITED;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);
	ret = snprintf(task_name, TASK_NAME_LEN,
		"ser_task_%d%d", PD_GET_DEVICE_IDX(pAd->physical_dev), band_idx);
	if (os_snprintf_error(TASK_NAME_LEN, ret))
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"snprintf error!\n");
	MTWF_PRINT("%s(%d): task_name is %s\n", __func__, __LINE__, task_name);
	RTMP_OS_TASK_INIT(task, task_name, pAd);
	Status = RtmpOSTaskAttach(task, ser_ctrl_task, (ULONG)task);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 "%s: unable to start\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}


INT ser_exit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	/*kill task*/
	ret = RtmpOSTaskKill(&hw_ctrl->ser_task);
	NdisFreeSpinLock(&hw_ctrl->ser_lock);
	return ret;
}
#ifdef MTK_FE_RESET_RECOVER
static int mtk_fe_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct mtk_notifier_block *nb = (struct mtk_notifier_block *)this;
	RTMP_ADAPTER *pAd = nb->priv;
	struct physical_device *device = pAd->physical_dev;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl = &device->ErrRecoveryCtl;

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return NOTIFY_DONE;

	switch (event) {
	case MTK_FE_START_RESET:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"receive fe reset(hang) event, trigger ser by FE\n");
		atomic_set(&pErrRecoveryCtrl->notify_fe, 1);
		ser_event_enq(device->chip_id, FE_WDMA_RESET, SER_FE_STOP, device->hw_id);
		break;

	case MTK_FE_RESET_DONE:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"receive fe reset done event, continue SER\n");
		/* use SER_L1 ser_cb */
		ser_event_enq(device->chip_id, SER_LV_1_0, SER_LV_1_0_EVT_STOP_PDMA_PRE, device->hw_id);
		break;

	case MTK_FE_STOP_TRAFFIC:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"receive fe stop traffic event, stop wifi traffic\n");
		ser_event_enq(device->chip_id, FE_RESET, SER_FE_STOP, device->hw_id);
		break;

	case MTK_FE_START_TRAFFIC:
		 MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
			"receive fe start traffic event, start wifi traffic\n");
		ser_event_enq(device->chip_id, FE_RESET, SER_FE_START, device->hw_id);
		break;

	default:
		break;

	}

	return NOTIFY_DONE;
}

unsigned int mtk_fe_reset_notifier_init(struct physical_device *device)
{
	int err = NDIS_STATUS_FAILURE;
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl = &device->ErrRecoveryCtl;
	struct mtk_notifier_block *mtk_nb = &pErrRecoveryCtrl->mtk_nb;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(device);

	atomic_set(&pErrRecoveryCtrl->notify_fe, 0);

	mtk_nb->priv = pAd;
	mtk_nb->nb.notifier_call = mtk_fe_event;
	err = register_netdevice_notifier(&mtk_nb->nb);

	if (err)
		return err;

	rtnl_lock();
	call_netdevice_notifiers(MTK_WIFI_CHIP_ONLINE, pAd->net_dev);
	rtnl_unlock();

	return err;
}

void  mtk_fe_reset_notifier_exit(struct physical_device *device)
{
	P_ERR_RECOVERY_CTRL_T pErrRecoveryCtrl = &device->ErrRecoveryCtl;
	struct mtk_notifier_block *mtk_nb = &pErrRecoveryCtrl->mtk_nb;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(device);

	rtnl_lock();
	call_netdevice_notifiers(MTK_WIFI_CHIP_OFFLINE, pAd->net_dev);
	rtnl_unlock();
	unregister_netdevice_notifier(&mtk_nb->nb);

	mtk_nb->priv = NULL;

}
#endif

#endif /* ERR_RECOVERY */

#ifdef WF_RESET_SUPPORT
static INT wf_reset_ctrl_task(ULONG context)
{
	struct physical_device *device;
	RTMP_OS_TASK *task;
	int status = 0;

	task = (RTMP_OS_TASK *)context;
	device = (struct physical_device *)RTMP_OS_TASK_DATA_GET(task);

	if (device == NULL)
		return 0;

	RtmpOSTaskCustomize(task);
	PD_SET_WF_RESET_STATUS(device, RTMP_TASK_STAT_RUNNING);

	while (task && !RTMP_OS_TASK_IS_KILLED(task)) {
		if (RtmpOSTaskWait(NULL, task, &status) == FALSE)
			break;

		wf_reset_func(device);
	}

	PD_SET_WF_RESET_STATUS(device, RTMP_TASK_STAT_UNKNOWN);
	status = RtmpOSTaskNotifyToExit(task);
	return status;
}

UINT32 wf_reset_init(struct physical_device *device)
{
	INT Status = 0;
	RTMP_OS_TASK *task = &device->wf_reset_thread;

	PD_SET_WF_RESET_STATUS(device, RTMP_TASK_STAT_INITED);
	PD_SET_WF_RESET_WM_COUNT(device, 0);
	PD_SET_WF_RESET_WA_COUNT(device, 0);
	PD_SET_WF_RESET_WO_COUNT(device, 0);

	/* RTMP_OS_TASK_INIT(task, "wf_reset_task", pAd); */
	RtmpOSTaskInit(task, "wf_reset_task", device, NULL, NULL);
	Status = RtmpOSTaskAttach(task, wf_reset_ctrl_task, (ULONG)task);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_ERROR,
				 "unable to start wf reset task\n");
		return NDIS_STATUS_FAILURE;
	}

	return TRUE;
}

UINT32 wf_reset_exit(struct physical_device *device)
{
	INT32 ret;

	PD_SET_WF_RESET_WM_COUNT(device, 0);
	PD_SET_WF_RESET_WA_COUNT(device, 0);
	PD_SET_WF_RESET_WO_COUNT(device, 0);
	/*kill task*/
	ret = RtmpOSTaskKill(&device->wf_reset_thread);

	return ret;
}
#endif

#ifdef DBG_STARVATION
static void hwctrl_starv_timeout_handle(struct starv_dbg *starv, struct starv_log_entry *entry)
{
	struct _HwCmdQElmt *cmd = container_of(starv, struct _HwCmdQElmt, starv);
	struct _HW_CTRL_T *hw_ctrl = starv->block->priv;
	struct starv_log_basic *log = NULL;

	os_alloc_mem(NULL, (UCHAR **) &log, sizeof(struct starv_log_basic));
	if (log) {
		log->qsize = hw_ctrl->HwCtrlQ.size;
		log->id = cmd->command;
		entry->log = log;
	}
}

static void hwctrl_starv_block_init(struct starv_log *ctrl, struct _HW_CTRL_T *hw_ctrl)
{
	struct starv_dbg_block *block = &hw_ctrl->block;

	strlcpy(block->name, "hwctrl", sizeof(block->name));
	block->priv = hw_ctrl;
	block->ctrl = ctrl;
	block->timeout = 100;
	block->timeout_fn = hwctrl_starv_timeout_handle;
	block->log_fn = starv_timeout_log_basic;
	register_starv_block(block);
}

#endif /*DBG_STARVATION*/

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
UINT32 HwCtrlInit(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
#ifdef DBG_STARVATION
	hwctrl_starv_block_init(&pAd->starv_log_ctrl, pHwCtrl);
#endif /*DBG_STARVATION*/
#ifdef ERR_RECOVERY
	Status = ser_init(pAd);
#endif /* ERR_RECOVERY */
	return Status;
}

UINT32 HwCmdThreadInit(VOID *physical_dev)
{
	INT Status = 0, ret = 0;
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;
	HW_CTRL_T *pHwCtrl = &ph_dev->HwCtrl;
	HwCmdQ *cmdq;
	RTMP_OS_TASK *pTask = &pHwCtrl->HwCtrlTask;
	char task_name[TASK_NAME_LEN] = {0};

	spin_lock_init(&pHwCtrl->HwCtrlQLock);
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	cmdq = &pHwCtrl->HwCtrlQ;
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->TotalCnt = 0;
	pTask = &pHwCtrl->HwCtrlTask;
	ret = snprintf(task_name, TASK_NAME_LEN,
		"HwCtrlTask_%d", PD_GET_DEVICE_IDX(ph_dev));
	if (os_snprintf_error(TASK_NAME_LEN, ret))
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
			"snprintf error!\n");

	MTWF_PRINT("%s: task_name is %s\n", __func__, task_name);
	RtmpOSTaskInit(pTask, task_name, ph_dev, NULL, NULL);
	Status = RtmpOSTaskAttach(pTask, HwCtrlThread, (ULONG)pTask);

	MTWF_PRINT("<--- %s(): status=%d!\n", __func__, Status);

	return Status;
}

VOID HwCmdThreadExit(VOID *physical_dev)
{
	INT32 ret;
	struct physical_device *ph_dev = (struct physical_device *)physical_dev;
	HW_CTRL_T *pHwCtrl = &ph_dev->HwCtrl;
	/*flush all queued command*/
	HwCtrlCmdHandler(ph_dev);
	/*kill task*/
	ret = RtmpOSTaskKill(&pHwCtrl->HwCtrlTask);
	NdisFreeSpinLock(&pHwCtrl->HwCtrlQLock);
}

VOID HwCtrlExit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HWCTRL_OP *hwctrl_ops = &pHwCtrl->hwctrl_ops;

	hwctrl_ops->wifi_sys_open = NULL;
	hwctrl_ops->wifi_sys_close = NULL;
	hwctrl_ops->wifi_sys_link_up = NULL;
	hwctrl_ops->wifi_sys_link_down = NULL;
	hwctrl_ops->wifi_sys_connt_act = NULL;
	hwctrl_ops->wifi_sys_disconnt_act = NULL;
	hwctrl_ops->wifi_sys_peer_update = NULL;
#ifdef ERR_RECOVERY
	ret = ser_exit(pAd);
#endif /* ERR_RECOVERY */
#ifdef DBG_STARVATION
	unregister_starv_block(&pHwCtrl->block);
#endif /*DBG_STARVATION*/
	if (hwifi_get_inf_num(pAd) == 0) {
		physical_device_proc_handle_exit(pAd->physical_dev);
		HwCmdThreadExit(pAd->physical_dev);
	}
}

NDIS_STATUS HwCtrlEnqueueCmd(
	RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PHwCmdQElmt	cmdqelmt = NULL;
	PHwCmdQ	cmdq = NULL;
	UINT32 wait_time = 0;
	struct physical_device *ph_dev = pAd->physical_dev;
	HW_CTRL_T *pHwCtrl = &ph_dev->HwCtrl;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
			"---> NIC is not exist!!\n");
		return NDIS_STATUS_FAILURE;
	}

	status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt, sizeof(HwCmdQElmt));

	if (cmdqelmt == NULL) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
			"---> os_alloc_mem failed!!\n");
		return NDIS_STATUS_RESOURCES;
	}

	NdisZeroMemory(cmdqelmt, sizeof(HwCmdQElmt));
	/*initial lock*/
	NdisAllocateSpinLock(NULL, &cmdqelmt->lock);
	/*creat wait */
	cmdqelmt->NeedWait = HwCtrlTxd.NeedWait;
	/*initial stravation dbg*/
#ifdef DBG_STARVATION
	starv_dbg_init(&pHwCtrl->block, &cmdqelmt->starv);
#endif /*DBG_STARVATION*/

	if (HwCtrlTxd.NeedWait)
		RTMP_OS_INIT_COMPLETION(&cmdqelmt->ack_done);

	if (HwCtrlTxd.InformationBufferLength > 0) {
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, HwCtrlTxd.InformationBufferLength);
		if (cmdqelmt->buffer == NULL) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
				"---> os_alloc_mem failed!!\n");
			status =  NDIS_STATUS_RESOURCES;
			goto end;
		}
		/*initial buffer*/
		os_move_mem(cmdqelmt->buffer, HwCtrlTxd.pInformationBuffer, HwCtrlTxd.InformationBufferLength);
		cmdqelmt->bufferlength = HwCtrlTxd.InformationBufferLength;
	}
	/*initial cmd element*/
	cmdqelmt->command = HwCtrlTxd.CmdId;
	cmdqelmt->type = HwCtrlTxd.CmdType;
	cmdqelmt->RspBuffer = HwCtrlTxd.pRespBuffer;
	cmdqelmt->RspBufferLen = HwCtrlTxd.RespBufferLength;

	if (HwCtrlTxd.RespBufferLength > 0) {
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->RspBuffer, HwCtrlTxd.RespBufferLength);
		if (cmdqelmt->RspBuffer == NULL) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
				"---> os_alloc_mem failed!!\n");
			status =  NDIS_STATUS_RESOURCES;
			goto end;
		}
		/*initial Rsp buffer*/
		cmdqelmt->RspBufferLen = HwCtrlTxd.RespBufferLength;
		os_zero_mem(cmdqelmt->RspBuffer, HwCtrlTxd.RespBufferLength);
	}

	cmdqelmt->CallbackFun = HwCtrlTxd.CallbackFun;
	cmdqelmt->CallbackArgs = HwCtrlTxd.CallbackArgs;
	cmdqelmt->pAd = (VOID *)pAd;
	/*create reference count*/
	os_kref_init(&cmdqelmt->refcnt);

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	/*check queue status*/
	if (!(pHwCtrl->HwCtrlQ.CmdQState & RTMP_TASK_CAN_DO_INSERT) ||
		(pHwCtrl->HwCtrlQ.size >= MAX_LEN_OF_HWCTRL_QUEUE(pAd))) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
			"---> HWCtrlQ size (%d) overflow!!\n", pHwCtrl->HwCtrlQ.size);
		NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
		status = NDIS_STATUS_FAILURE;
		goto end;
	}
	/*add reference count for cmd due to send to thread*/
	os_kref_get(&cmdqelmt->refcnt);

	/*enqueue to cmdq*/
	cmdq = &pHwCtrl->HwCtrlQ;

	if (cmdq->size == 0)
		cmdq->head = cmdqelmt;
	else
		cmdq->tail->next = cmdqelmt;

	cmdq->tail = cmdqelmt;
	cmdqelmt->next = NULL;
	cmdq->size++;

	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	/*get stravation */
#ifdef DBG_STARVATION
	starv_dbg_get(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/
	RTCMDUp(&pHwCtrl->HwCtrlTask);

	/*not need wait, goto end directly*/
	if (!HwCtrlTxd.NeedWait)
		goto end;

	/*wait handle*/
#ifdef WF_RESET_SUPPORT
	/* reduce wait timeout from 300 to 1 when wifi reset*/
	if (PD_GET_WF_RESET_IN_PROGRESS(pAd->physical_dev) == TRUE) {
		/* this time base on MCU response timeout , now hwifi is 20 s */
		if (HwCtrlTxd.CmdId == HWCMD_ID_WIFISYS_LINKDOWN)
			HwCtrlTxd.wait_time = 20000;
	}
#endif

	wait_time = HwCtrlTxd.wait_time ? HwCtrlTxd.wait_time : HWCTRL_CMD_TIMEOUT;
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&cmdqelmt->ack_done, RTMPMsecsToJiffies(wait_time))) {
		status = NDIS_STATUS_TIMEOUT;
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_CMD_CTRL, DBG_LVL_ERROR,
			"HwCtrl CmdTimeout, TYPE:%d,ID:%d!!\n",
			cmdqelmt->type, cmdqelmt->command);
	}

	if (status == NDIS_STATUS_SUCCESS)
		os_move_mem(HwCtrlTxd.pRespBuffer, cmdqelmt->RspBuffer, HwCtrlTxd.RespBufferLength);

end:
	os_kref_put(&cmdqelmt->refcnt, free_hwcmd);
	return status;
}

/*
*
*/
INT Show_HwCtrlStatistic_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct physical_device *ph_dev = pAd->physical_dev;
	HW_CTRL_T *pHwCtrl = NULL;
	HW_CMD_TABLE_T *pHwCmdTable = NULL;
	UCHAR i = 0, j = 0;
	PHwCmdQElmt	cmdqelmt = NULL;

	if (ph_dev)
		pHwCtrl = &ph_dev->HwCtrl;
	else {
		MTWF_PRINT("\tph_dev is NULL.\n");
		return FALSE;
	}

	MTWF_PRINT("\tHwCtrlTask Totaol Ref. Cnt: %d\n", pHwCtrl->TotalCnt);
	MTWF_PRINT("\tHwCtrlTask CMD Statistic:\n");
	pHwCmdTable = HwCmdTable[i];

	while (pHwCmdTable != NULL) {
		j = 0;

		while (pHwCmdTable[j].CmdID != HWCMD_ID_END) {
			MTWF_PRINT("\tCMDID: %d, Handler: %p, RfCnt: %d\n",
					 pHwCmdTable[j].CmdID, pHwCmdTable[j].CmdHdlr, pHwCmdTable[j].RfCnt);
			j++;
		}

		pHwCmdTable = HwCmdTable[++i];
	}

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	MTWF_PRINT("\tQueSize: %d\n",
		pHwCtrl->HwCtrlQ.size);
	cmdqelmt = pHwCtrl->HwCtrlQ.head;
	while (cmdqelmt) {
		MTWF_PRINT("\tTYPE:%d, CID:%d\n",
			cmdqelmt->type, cmdqelmt->command);
		cmdqelmt = cmdqelmt->next;
	}
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	return TRUE;
}

UINT32 HWCtrlOpsReg(RTMP_ADAPTER *pAd)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HWCTRL_OP *hwctrl_ops = &pHwCtrl->hwctrl_ops;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/*hook funcion*/
	switch (cap->hw_ops_ver) {
#ifdef WIFI_SYS_FW_V2

	case HWCTRL_OP_TYPE_V2:
		hw_ctrl_ops_v2_register(hwctrl_ops);
		break;
#endif /*WIFI_SYS_FW_V2*/
#ifdef WIFI_SYS_FW_V1

	case HWCTRL_OP_TYPE_V1:
	default:
		hw_ctrl_ops_v1_register(hwctrl_ops);
		break;
#endif /*WIFI_SYS_FW_V1*/
	}

	return NDIS_STATUS_SUCCESS;
}


UINT32 hwctrl_queue_len(RTMP_ADAPTER *pAd)
{
	struct physical_device *ph_dev = pAd->physical_dev;
	HW_CTRL_T *pHwCtrl = NULL;
	UINT32 qlen = 0;

	if (ph_dev)
		pHwCtrl = &ph_dev->HwCtrl;
	else {
		MTWF_PRINT("\tph_dev is NULL.\n");
		return qlen;
	}
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	qlen = pHwCtrl->HwCtrlQ.size;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	return qlen;
}
