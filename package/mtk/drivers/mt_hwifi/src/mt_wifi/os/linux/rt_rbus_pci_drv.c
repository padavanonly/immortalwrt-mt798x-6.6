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

#include "rt_config.h"

/*
 * ========================================================================
 * Routine Description:
 *    PCI command kernel thread.
 *
 * Arguments:
 *	*Context			the pAd, driver control block pointer
 *
 * Return Value:
 *   0					close the thread
 * ========================================================================
 */
INT RTPCICmdThread(
	IN ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	UINT32 CmdQState;

	status = 0;
	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	RtmpOSTaskCustomize(pTask);
	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	CmdQState = pAd->CmdQ.CmdQState;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (CmdQState == RTMP_TASK_STAT_RUNNING) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		NdisAcquireSpinLock(&pAd->CmdQLock);
		if (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_STOPED) {
			NdisReleaseSpinLock(&pAd->CmdQLock);
			break;
		}
		CmdQState = pAd->CmdQ.CmdQState;
		NdisReleaseSpinLock(&pAd->CmdQLock);

		if (!pAd->PM_FlgSuspend)
			CMDHandler(pAd);
	}

	if (!pAd->PM_FlgSuspend) {
		/* Clear the CmdQElements. */
		CmdQElmt	*pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;

		while (pAd->CmdQ.size) {
			RTThreadDequeueCmd(&pAd->CmdQ, &pCmdQElmt);

			if (pCmdQElmt) {
				if (pCmdQElmt->CmdFromNdis == TRUE) {
					if (pCmdQElmt->buffer != NULL)
						os_free_mem(pCmdQElmt->buffer);

					os_free_mem((PUCHAR)pCmdQElmt);
				} else {
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pCmdQElmt->buffer);

					os_free_mem((PUCHAR)pCmdQElmt);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->CmdQLock);
	}

	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_INFO, "<---RTPCICmdThread\n");
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}

