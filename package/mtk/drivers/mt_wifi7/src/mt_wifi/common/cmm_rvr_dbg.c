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
	cmm_rvr_dbg.c
*/
#include "rt_config.h"

#define MSG_LEN 2048
static const RTMP_STRING *hex_RTMP_STRING = "0123456789abcdefABCDEF";
static const RTMP_STRING *dec_RTMP_STRING = "0123456789";
static struct {
	RTMP_STRING *name;
	INT(*rd_proc) (RTMP_ADAPTER * pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT * wrq);
} *PRTMP_PRIVATE_RD_PROC, RTMP_PRIVATE_RD_SUPPORT_PROC[] = {
	{"view",	rd_view},
	{"view+",	rd_view_plus},
	{"view-",	rd_view_minus},
	/*Alias */
	{"wcid",	rd_wcid},
	{"sta",		rd_wcid},
	{"apcli",	rd_wcid},
	{"ap",		rd_wcid},
	{"reset", rd_reset},
	{"help", rd_help},
	{NULL,}
};
static struct {
	RTMP_STRING *key;
	RTMP_STRING *str;
	INT val;
} *PView_Key_Node, View_Key_Node_List[] = {
	{ "basic", "VIEW_BASICINFO", VIEW_BASICINFO},
	{ "wcid", "VIEW_WCID", VIEW_WCID},
	{ "mac", "VIEW_MACCOUNTER", VIEW_MACCOUNTER},
	{ "phy", "VIEW_PHYCOUNTER", VIEW_PHYCOUNTER},
	{ "noise", "VIEW_NOISE", VIEW_NOISE},
	{ "cn", "VIEW_CNNUMBER", VIEW_CNNUMBER},
	{ "others", "VIEW_OTHERS", VIEW_OTHERS},
	{NULL, NULL,}
};

VOID RTMPIoctlRvRDebug_Init(RTMP_ADAPTER *pAd)
{
	pAd->RVRDBGCtrl.ucViewLevel = VIEW_BASICINFO | VIEW_MACCOUNTER | VIEW_PHYCOUNTER  | VIEW_WCID; /*Default View Info */
	pAd->RVRDBGCtrl.wcid = 0;
	pAd->RVRDBGCtrl.ucCNcnt = 0xFF;
}

INT RTMPIoctlRvRDebug(RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	RTMP_STRING *this_char, *value = NULL;
	INT Status = NDIS_STATUS_SUCCESS;

	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	this_char = wrq->u.data.pointer;
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO,
		"Before check, this_char = %s\n", this_char);
	value = strchr(this_char, '=');
	if (value) {
		if (strlen(value) > 1) {
			*value++ = 0;
		} else {
			*value = 0;
			value = NULL;
		}
	}
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO,
		"After check, this_char = %s, value = %s\n",
		this_char, (value == NULL ? "" : value));
	for (PRTMP_PRIVATE_RD_PROC = RTMP_PRIVATE_RD_SUPPORT_PROC; PRTMP_PRIVATE_RD_PROC->name; PRTMP_PRIVATE_RD_PROC++) {
		if (!strcmp(this_char, PRTMP_PRIVATE_RD_PROC->name)) {
			if (!PRTMP_PRIVATE_RD_PROC->rd_proc(pAd, value, wrq)) {
				/*FALSE:Set private failed then return Invalid argument */
				Status = -EINVAL;
			}

			break;  /*Exit for loop. */
		}
	}
	if (PRTMP_PRIVATE_RD_PROC->name == NULL) {
		/*Not found argument */
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO,
			"PRTMP_PRIVATE_RD_PROC->name == NULL");
		rd_dashboard(pAd, wrq);
		return Status;
	}
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return Status;
}

INT rd_dashboard(RTMP_ADAPTER *pAd, IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	RTMP_STRING *msg = NULL;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	int ret, left_buf_size;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"fail to allocate memory for msg\n");
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	left_buf_size = MSG_LEN - strlen(msg);
	ret = snprintf(msg+strlen(msg), left_buf_size,
		"%s%-16s%s\n", "====================", " RvR Debug Info ", "====================");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_BASICINFO) == VIEW_BASICINFO)
		printBasicinfo(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_WCID) == VIEW_WCID)
		printWcid(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_MACCOUNTER) == VIEW_MACCOUNTER) {
		printMacCounter(pAd, msg);
	}
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_PHYCOUNTER) == VIEW_PHYCOUNTER)
		printPhyCounter(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_NOISE) == VIEW_NOISE)
		printNoise(pAd, msg);
	if ((pRVRDBGCtrl->ucViewLevel & VIEW_OTHERS) == VIEW_OTHERS)
		printOthers(pAd, msg);
	left_buf_size = MSG_LEN - strlen(msg);
	ret = snprintf(msg+strlen(msg), left_buf_size,
		"%s\n", "========================================================");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}

	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return TRUE;
}

INT rd_view(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg = NULL;
	INT button = Case_SHOW;
	UINT8 ucViewLevel_val = 0;
	int ret, left_buf_size;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"fail to allocate memory for msg\n");
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}

	if (arg != NULL) {
		if (strlen(arg) > 2) {
			button = Case_ERROR;
		} else {
			if (strspn(arg, hex_RTMP_STRING) == strlen(arg)) {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO,
					"Valid strspn=%d,strlen=%d,",
					(INT)strspn(arg, hex_RTMP_STRING), (INT)strlen(arg));
				button = Case_SET;
				ucViewLevel_val = (UINT8) os_str_tol(arg, 0, 16);
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO,
					"Valid strspn=%d,strlen=%d,",
					(INT)strspn(arg, hex_RTMP_STRING), (INT)strlen(arg));
				button = Case_ERROR;
			}
		}
	}

	switch (button) {
	case Case_ERROR:
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"No corresponding parameter !!!\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"ex: iwpriv ra0 rd view=FF(bit8)\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case Case_SHOW:
		printView(pAd, msg);
		break;
	case Case_SET:
		pRVRDBGCtrl->ucViewLevel  = ucViewLevel_val;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"pRVRDBGCtrl->ucViewLevel = %x", pRVRDBGCtrl->ucViewLevel);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return TRUE;
}
INT rd_view_plus(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg = NULL;
	INT view_val = VIEW_ERROR;
	int ret, left_buf_size;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"fail to allocate memory for msg\n");
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}

	if (arg)
		view_val = getViewLevelValue(arg);
	switch (view_val) {
	case VIEW_BASICINFO:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_BASICINFO;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_BASICINFO Enable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_WCID:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_WCID;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_WCID Enable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_MACCOUNTER:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_MACCOUNTER;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_MACCOUNTER Enable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_PHYCOUNTER:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_PHYCOUNTER;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_PHYCOUNTER Enable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_CNNUMBER:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_CNNUMBER;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_CNNUMBER Enable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_NOISE:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_NOISE;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_NOISE Enable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_6:
		break;
	case VIEW_OTHERS:
		pRVRDBGCtrl->ucViewLevel  |= VIEW_OTHERS;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_OTHERS Enable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_ERROR:
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"No corresponding parameter !!!\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "ex: iwpriv ra0 rd view+=rate\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		printView(pAd, msg);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return TRUE;
}
INT rd_view_minus(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg = NULL;
	INT view_val = VIEW_ERROR;
	int ret, left_buf_size;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"fail to allocate memory for msg\n");
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}

	if (arg)
		view_val = getViewLevelValue(arg);
	switch (view_val) {
	case VIEW_BASICINFO:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_BASICINFO;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_BASICINFO Disable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_WCID:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_WCID;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_WCID Disable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_MACCOUNTER:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_MACCOUNTER;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_MACCOUNTER Disable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_PHYCOUNTER:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_PHYCOUNTER;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_PHYCOUNTER Disable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_CNNUMBER:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_CNNUMBER;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_CNNUMBER Disable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_NOISE:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_NOISE;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_NOISE Disable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_6:

		break;
	case VIEW_OTHERS:
		pRVRDBGCtrl->ucViewLevel  &= ~VIEW_OTHERS;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "VIEW_OTHERS Disable");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case VIEW_ERROR:
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"No corresponding parameter !!!\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "ex: iwpriv ra0 rd view-=rate\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		printView(pAd, msg);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return TRUE;
}

INT rd_wcid(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	RTMP_STRING *msg = NULL;
	INT button = Case_SHOW;
	LONG input;
	UINT16 wcid = 0;
	int ret, left_buf_size;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"fail to allocate memory for msg\n");
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	if (arg != NULL) {
		if (strlen(arg) > 3) {
			button = Case_ERROR;
		} else{
			if (strspn(arg, dec_RTMP_STRING) == strlen(arg)) {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO,
					"Valid strspn=%d,strlen=%d,",
					(INT)strspn(arg, dec_RTMP_STRING), (INT)strlen(arg));
				input = os_str_tol(arg, 0, 10);
				if (input < 0 || input > hc_get_chip_wtbl_max_num(pAd)) {
					button = Case_ERROR;
				} else {
					wcid = input;
					button = Case_SET;
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO,
					"Invalid strspn=%d,strlen=%d,",
					(INT)strspn(arg, dec_RTMP_STRING), (INT)strlen(arg));
				button = Case_ERROR;
			}
		}
	}
	switch (button) {
	case Case_ERROR:
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"No corresponding parameter !!!\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"ex: iwpriv ra0 rd sta=1~%d\n", hc_get_chip_wtbl_max_num(pAd));
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"or  iwpriv ra0 rd sta=0 for auto search first sta\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	case Case_SHOW:
		Show_MacTable_Proc(pAd, ENTRY_NONE);
		break;
	case Case_SET:
		pRVRDBGCtrl->wcid  = wcid;
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "pRVRDBGCtrl->wcid = %d", pRVRDBGCtrl->wcid);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return TRUE;
}

INT rd_reset(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status = NDIS_STATUS_SUCCESS;
	RTMP_STRING *msg = NULL;
	INT button = Case_SHOW;
	int ret, left_buf_size;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"fail to allocate memory for msg\n");
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}

	switch (button) {
	case Case_SHOW:
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "Reset all counter!\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		Set_ResetStatCounter_Proc(pAd, NULL);
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return TRUE;
}

INT rd_help(RTMP_ADAPTER *pAd, RTMP_STRING *arg, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	int ret, left_buf_size;
	INT Status = NDIS_STATUS_SUCCESS;
	RTMP_STRING *msg = NULL;
	INT button = Case_SHOW;

	os_alloc_mem(pAd, (UCHAR **)&msg, sizeof(CHAR)*MSG_LEN);

	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"fail to allocate memory for msg\n");
		return FALSE;
	}
	memset(msg, 0x00, MSG_LEN);
	ret = snprintf(msg, MSG_LEN, "\n");
	if (os_snprintf_error(MSG_LEN, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	switch (button) {
	case Case_SHOW:
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%s", "iwpriv [Interface] rd [Sub-command]\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%s", "Sub-command List\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "view", "Show view level status\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "view=", "Set view level by hex value(8bits 00~FF)\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "view+=", "Enable view level by string\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "view-=", "Disable view level by string\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "wcid,sta,ap,apcli", "Show mac table\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "wcid=,sta=,ap=,apcli=", "Set WCID\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "reset", "Reset all counter\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size,
			"%-25s %s", "help", "Show support command info\n");
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		break;
	}
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);
	os_free_mem(msg);
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_INFO, "\n");
	return TRUE;
}

INT printBasicinfo (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	int ret, left_buf_size;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;

	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);
	else
		return FALSE;

	left_buf_size = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size,
		"%s%-16s%s\n", "====================", " BASIC ", "====================");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	left_buf_size = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "%-32s= %d\n", "Current Band ", ucBand);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	RTMP_GET_TEMPERATURE(pAd, ucBand, &pAd->temperature);
	left_buf_size = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size, "%-32s= %d\n", "Current Temperature ", pAd->temperature);
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	return TRUE;
}

VOID printView(RTMP_ADAPTER *pAd, IN RTMP_STRING *msg)
{
	PRvR_Debug_CTRL pRVRDBGCtrl;
	INT view_bits = 0;
	int ret, left_buf_size;
	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	left_buf_size = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size,
		"%-4s | %-6s | %-15s | %s\n", "bit", "arg", "info", "Status");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}
	for (PView_Key_Node = View_Key_Node_List; PView_Key_Node->key; PView_Key_Node++) {
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), left_buf_size, "%-4d | %-6s | %-15s | %s\n",
		view_bits++,
		PView_Key_Node->key,
		PView_Key_Node->str,
		(pRVRDBGCtrl->ucViewLevel & PView_Key_Node->val ? "Enable":"Disable"));
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
	}
}

INT printWcid (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	UCHAR	tmp_str[30];
	INT		temp_str_len = sizeof(tmp_str);
	INT 	first_sta = 0;
	UINT32 lastRxRate;
	UINT32 lastTxRate;
	int ret, left_buf_size;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;

	left_buf_size = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), left_buf_size,
		"%s%-16s%s\n", "====================", " WCID ", "====================");
	if (os_snprintf_error(left_buf_size, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"final_name snprintf error!\n");
	}

	/* User assign aid, default = 0 will auto search first sta  */
	if (pRVRDBGCtrl->wcid == 0)
		for (first_sta = 0; VALID_UCAST_ENTRY_WCID(pAd, first_sta); first_sta++) {
			pEntry = entry_get(pAd, first_sta);
			if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
				break;
		}
	else if (VALID_UCAST_ENTRY_WCID(pAd, pRVRDBGCtrl->wcid))
		pEntry = entry_get(pAd, pRVRDBGCtrl->wcid);

	else
		return TRUE;

	if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg+strlen(msg), left_buf_size, "%-32s= %d\n", "AID ", (int)pEntry->Aid);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg+strlen(msg), MSG_LEN - strlen(msg), "%-32s= %02X:%02X:%02X:%02X:%02X:%02X\n", "MAC Addr ", PRINT_MAC(pEntry->Addr));
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(tmp_str, temp_str_len, "%d %d %d %d", pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1],
			 pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		left_buf_size = MSG_LEN - strlen(msg);
		ret = snprintf(msg+strlen(msg), MSG_LEN - strlen(msg), "%-32s= %s\n", "RSSI0/1/2/3 ", tmp_str);
		if (os_snprintf_error(left_buf_size, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"final_name snprintf error!\n");
		}
		lastRxRate = pEntry->LastRxRate;
		lastTxRate = pEntry->LastTxRate;

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			lastTxRate = RateHTTxToPhyRateFormat(lastTxRate);
			lastRxRate = RateHTTxToPhyRateFormat(lastRxRate);
		}

		if (cap->fgRateAdaptFWOffload == TRUE && pEntry->bAutoTxRateSwitch == TRUE)
			lastTxRate = GetTxRatePerSta(pAd, pEntry->wcid);

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			StatRateToString(pAd, msg, 0, lastTxRate);
			StatRateToString(pAd, msg, 1, lastRxRate);
		}
	}
	return TRUE;
}

INT printMacCounter (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	UINT32 txCount = 0;
	UINT32 rxCount = 0;
	ULONG txper, rxper;
	COUNTER_802_11 *WlanCounter = &pAd->WlanCounters;
	ULONG mpduper = 0;
	ULONG mpduTXCount = 0;
	PRvR_Debug_CTRL pRVRDBGCtrl;
	INT ret, tmp;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp,
		"%s%-16s%s\n", "====================", " MAC COUNTER ", "====================");
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}

	/* Tx Count */
	txCount = pAd->WlanCounters.TransmittedFragmentCount.u.LowPart;
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n", "Tx success count ", txCount);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		txper = txCount == 0 ? 0 : 1000*(pAd->WlanCounters.FailedCount.u.LowPart)/(pAd->WlanCounters.FailedCount.u.LowPart+txCount);
		tmp = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld PER=%ld.%1ld%%\n",
			"Tx fail count ",
			(ULONG)pAd->WlanCounters.FailedCount.u.LowPart,
			txper/10, txper % 10);
		if (os_snprintf_error(tmp, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"snprintf error!\n");
			return FALSE;
		}
	} else {
		txper = txCount == 0 ? 0 : 1000*(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart)/(pAd->WlanCounters.RetryCount.u.LowPart+pAd->WlanCounters.FailedCount.u.LowPart+txCount);
		tmp = MSG_LEN - strlen(msg);
		ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld, PER=%ld.%1ld%%\n",
			"Tx retry count ",
			(ULONG)pAd->WlanCounters.RetryCount.u.LowPart,
			txper/10, txper % 10);
		if (os_snprintf_error(tmp, ret)) {
			MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
				"snprintf error!\n");
			return FALSE;
		}
	}
	/*BF */
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %x %x\n",
		"Tx BF count(iBF/eBF) ", pRVRDBGCtrl->uiiBFTxcnt, pRVRDBGCtrl->uieBFTxcnt);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	/*AMPDU */
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld\n",
		"Tx AGG Range 1 (1)", (LONG)(WlanCounter->TxAggRange1Count.u.LowPart));
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld\n",
		"Tx AGG Range 2 (2~5)", (LONG)(WlanCounter->TxAggRange2Count.u.LowPart));
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld\n",
		"Tx AGG Range 3 (6~15)", (LONG)(WlanCounter->TxAggRange3Count.u.LowPart));
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld\n",
		"Tx AGG Range 4 (>15)", (LONG)(WlanCounter->TxAggRange4Count.u.LowPart));
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	mpduTXCount = WlanCounter->AmpduSuccessCount.u.LowPart;
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld\n", "Tx AMPDU success", mpduTXCount);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	mpduper = mpduTXCount == 0 ? 0 : 1000 * (WlanCounter->AmpduFailCount.u.LowPart) / (WlanCounter->AmpduFailCount.u.LowPart + mpduTXCount);
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld PER=%ld.%1ld%%\n",
		"Tx AMPDU fail count", (ULONG)WlanCounter->AmpduFailCount.u.LowPart, mpduper/10, mpduper % 10);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	/* Rx Count */
	rxCount = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n", "Rx success ", rxCount);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	rxper = pAd->WlanCounters.ReceivedFragmentCount.u.LowPart == 0 ? 0 : 1000*(pAd->WlanCounters.FCSErrorCount.u.LowPart)/(pAd->WlanCounters.FCSErrorCount.u.LowPart+pAd->WlanCounters.ReceivedFragmentCount.u.LowPart);
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld, PER=%ld.%1ld%%\n",
		"Rx with CRC ",
		(ULONG)pAd->WlanCounters.FCSErrorCount.u.LowPart,
		rxper/10, rxper % 10);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld\n",
		"Rx drop(out of resource)", (ULONG)pAd->Counters8023.RxNoBuffer);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %ld\n",
		"Rx duplicate frame", (ULONG)pAd->WlanCounters.FrameDuplicateCount.u.LowPart);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}

	return  TRUE;
}


INT printPhyCounter (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	PRvR_Debug_CTRL pRVRDBGCtrl;
	INT ret, tmp;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp,
		"%s%-16s%s\n", "====================", " PHY COUNTER ", "====================");
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	return  TRUE;
}



INT printNoise (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	INT ret, tmp;

	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp,
		"%s%-16s%s\n", "====================", " NOISE ", "====================");
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %s\n",
		"MibBucket ", pAd->OneSecMibBucket.Enabled ? "Enable":"Disable");
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n",
		"Channel Busy Time ", pAd->OneSecMibBucket.ChannelBusyTimeCcaNavTx);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n",
		"Primary Channel Busy Time ", pAd->OneSecMibBucket.ChannelBusyTime);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n",
		"OBSS Air Time ", pAd->OneSecMibBucket.OBSSAirtime);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n",
		"Tx Air Time ", pAd->OneSecMibBucket.MyTxAirtime);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n",
		"Rx Air Time ", pAd->OneSecMibBucket.MyRxAirtime);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %d\n",
		"EDCCA Time ", pAd->OneSecMibBucket.EDCCAtime);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %x\n",
		"PD count ", pAd->OneSecMibBucket.PdCount);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp, "%-32s= %x\n",
		"MDRDY Count ", pAd->OneSecMibBucket.MdrdyCount);
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	return TRUE;
}

INT printOthers (RTMP_ADAPTER *pAd, RTMP_STRING *msg)
{
	/*
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd,pObj->ioctl_if,pObj->ioctl_if_type);
	UINT8 ucBand = BAND0;
	PRvR_Debug_CTRL pRVRDBGCtrl;

	pRVRDBGCtrl = &pAd->RVRDBGCtrl;
	if (wdev != NULL)
		ucBand = HcGetBandByWdev(wdev);
	else
		return FALSE;
	*/
	INT ret, tmp;

	tmp = MSG_LEN - strlen(msg);
	ret = snprintf(msg + strlen(msg), tmp,
		"%s%-16s%s\n", "====================", " OTHERS ", "====================");
	if (os_snprintf_error(tmp, ret)) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_RVR, DBG_LVL_ERROR,
			"snprintf error!\n");
		return FALSE;
	}
	return TRUE;
}

INT getViewLevelValue(RTMP_STRING *arg)
{
	if (arg == NULL)
		return VIEW_ERROR;
	for (PView_Key_Node = View_Key_Node_List; PView_Key_Node->key; PView_Key_Node++)
		if (rtstrcasecmp(arg, PView_Key_Node->key) == TRUE)
			return PView_Key_Node->val;
	return VIEW_ERROR;
}
