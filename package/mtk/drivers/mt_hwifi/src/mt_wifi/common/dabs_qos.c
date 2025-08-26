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
	dabs_qos.c
*/

#include "rt_config.h"
#include "mcu/mt_cmd.h"

#ifdef DABS_QOS

#ifdef WIFI_UNIFIED_COMMAND
#include "epcs_cmm.h"
#endif
#ifdef LINUX
#include <net/ip.h>
#endif

static const UINT8 ac_queue_to_up[WMM_NUM_OF_AC] = {
	1 /* AC_BK */, 0 /* AC_BE */, 5 /* AC_VI */, 7 /* AC_VO */
};
const UINT8  up_to_ac_mapping[NUM_OF_8021D_COS] = {
	WMM_AC_BE /* UP=0 */,   WMM_AC_BK /* UP=1 */,   WMM_AC_BK /* UP=2 */,   WMM_AC_BE /* UP=3 */,
	WMM_AC_VI /* UP=4 */,   WMM_AC_VI /* UP=5 */,   WMM_AC_VO /* UP=6 */,   WMM_AC_VO /* UP=7 */
};

struct qos_param_rec qos_param_table[MAX_QOS_PARAM_TBL];
static void dabs_dly_dbglvl(UINT_8 idx);
static void dabs_latency_show(RTMP_ADAPTER *pAd);

NDIS_SPIN_LOCK qos_param_table_lock;
#define RTMP_SET_PACKET_QOS_IDX(_p, _idx) (*(UINT8 *)&PACKET_CB(_p, 32) = _idx)
#define RTMP_GET_PACKET_QOS_IDX(_p)                (*(UINT8 *)&PACKET_CB(_p, 32))
UINT32	get_min_delaybound_by_ac(unsigned short wlan_idx, unsigned short priority)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;
	unsigned short tmp_delayreq = 0xFFFF;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];
		if (prec->in_use == FALSE)
			continue;
		if (up_to_ac_mapping[prec->priority] != up_to_ac_mapping[priority])
			continue;
		if (prec->wlan_idx != wlan_idx) {
			continue;
		}
			if (prec->delay_req < tmp_delayreq) {
			 tmp_delayreq = prec->delay_req;
			 the_idx = idx;
			}
	}

	return the_idx;
}

unsigned short search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, struct sk_buff *skb)
{
	struct iphdr *hdr = (struct iphdr *)(skb->data + LENGTH_802_3);
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];

		if ((prec->ip_src == 0) || (prec->ip_dest == 0))
			continue;
		if ((prec->ip_src != hdr->saddr) || (prec->ip_dest != hdr->daddr))
			continue;
		if (prec->protocol != hdr->protocol)
			continue;

		if (prec->protocol == IPPROTO_TCP) {
			if ((prec->sport > 0) && (prec->dport > 0)) {
				struct tcphdr *tcph = (struct tcphdr *)((UINT8*)hdr) + 20;
				if ((prec->sport != tcph->source) ||
					(prec->dport != tcph->dest))
					continue;
			}
		}
		else if (prec->protocol == IPPROTO_UDP) {
			if ((prec->sport > 0) && (prec->dport > 0)) {
				struct udphdr *udph = (struct udphdr *)(((UINT8*)hdr) + 20);
				UINT8* pdata = ((UINT8*)hdr) + 20;

				if ((prec->sport != udph->source) ||
					(prec->dport != udph->dest)) {
					if (pAd->dabs_qos_op & DABS_DBG_PRN_LV1)
						MTWF_DBG(pAd,
						DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
						"[%d]dest_ip=%u.%u.%u.%u=port=(%u,%u)!=(%u,%u),\
						(%02X:%02X:%02X%02X)\n",
						idx, NIPQUAD(hdr->daddr),
					 	ntohs(prec->sport), ntohs(prec->dport),
						ntohs(udph->source), ntohs(udph->dest),
						pdata[2], pdata[3],pdata[4],pdata[5]);
					continue;
				}
			}
		}

		the_idx = idx;
		break;
	}

	return the_idx;
}

unsigned short ioctl_search_qos_param_tbl_idx_by_5_tuple(PRTMP_ADAPTER pAd, VOID *qos_param_rec_pointer, BOOLEAN add)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec = NULL;
	struct qos_param_rec_add *qos_param_rec_add = NULL;
	struct qos_param_rec_del *qos_param_rec_del = NULL;

	if (add == TRUE) {
		qos_param_rec_add = (struct qos_param_rec_add *)qos_param_rec_pointer;
		for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
			prec = &qos_param_table[idx];

			if ((prec->ip_src == 0) || (prec->ip_dest == 0))
				continue;
			if ((prec->ip_src != qos_param_rec_add->ip_src) ||
				(prec->ip_dest != qos_param_rec_add->ip_dest))
				continue;
			if (prec->protocol != qos_param_rec_add->protocol)
				continue;
			if ((prec->sport != htons(qos_param_rec_add->sport)) ||
				(prec->dport != htons(qos_param_rec_add->dport)))
				continue;

			the_idx = idx;
			break;
		}
	} else {
		qos_param_rec_del = (struct qos_param_rec_del *)qos_param_rec_pointer;

		for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
			prec = &qos_param_table[idx];

			if ((prec->ip_src == 0) || (prec->ip_dest == 0))
				continue;
			if ((prec->ip_src != qos_param_rec_del->ip_src) ||
				(prec->ip_dest != qos_param_rec_del->ip_dest))
				continue;
			if (prec->protocol != qos_param_rec_del->protocol)
				continue;
			if ((prec->sport != htons(qos_param_rec_del->sport)) ||
				(prec->dport != htons(qos_param_rec_del->dport)))
				continue;

			the_idx = idx;
			break;
		}
	}

	return the_idx;
}

unsigned short search_free_qos_param_tbl_idx(PRTMP_ADAPTER pAd)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];

		OS_SPIN_LOCK_BH(&qos_param_table_lock);

		if (prec->valid == TRUE) {
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			continue;
		}

		OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

		the_idx = idx;
		break;
	}

	return the_idx;
}


unsigned short search_qos_param_tbl_idx_by_wlan_idx(
	unsigned short wlan_idx,
	unsigned short proto,
	unsigned short dport)
{
	unsigned short idx;
	unsigned short the_idx = MAX_QOS_PARAM_TBL;
	struct qos_param_rec *prec;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		prec = &qos_param_table[idx];
		OS_SPIN_LOCK_BH(&qos_param_table_lock);
		if (prec->valid == FALSE) {
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			continue;
		}
		OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
		if ((prec->protocol != proto) && (prec->protocol != 0))
			continue;

		if ((prec->dport != dport) && (prec->dport != 0))
			continue;

		if (prec->wlan_idx == wlan_idx) {
			the_idx = idx;
			break;
		}
	}

	return the_idx;
}

bool set_qos_param_tbl_wlan_idx_by_idx(PRTMP_ADAPTER pAd, unsigned short idx, unsigned short wlan_idx)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;
	PMAC_TABLE_ENTRY pEntry;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	pEntry = entry_get(pAd, wlan_idx);
	ptarget_qos_param = &qos_param_table[idx];
	ptarget_qos_param->pEntry = pEntry;
	if (pEntry)
		pEntry->qos_tbl_idx[ptarget_qos_param->priority] = idx + 1;
	ptarget_qos_param->wlan_idx = wlan_idx;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_tbl_ipaddr_by_idx(
	unsigned short idx,
	struct qos_param_rec *pqos_param
)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	ptarget_qos_param = &qos_param_table[idx];

	ptarget_qos_param->ip_src = pqos_param->ip_src;
	ptarget_qos_param->ip_dest = pqos_param->ip_dest;
	ptarget_qos_param->protocol = pqos_param->protocol;
	ptarget_qos_param->sport = pqos_param->sport;
	ptarget_qos_param->dport = pqos_param->dport;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_tbl_qos_by_idx(unsigned short idx, struct qos_param_rec *pqos_param)
{
	bool ret = true;
	struct qos_param_rec *ptarget_qos_param;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	ptarget_qos_param = &qos_param_table[idx];

	ptarget_qos_param->priority = ac_queue_to_up[pqos_param->priority];
	ptarget_qos_param->force_ac = pqos_param->force_ac;
	ptarget_qos_param->delay_bound = pqos_param->delay_bound;
	ptarget_qos_param->delay_req = pqos_param->delay_req;
	ptarget_qos_param->delay_weight = pqos_param->delay_weight;
	ptarget_qos_param->bw_req = pqos_param->bw_req;
	ptarget_qos_param->data_rate = pqos_param->data_rate;
	ptarget_qos_param->dir = pqos_param->dir;
	ptarget_qos_param->drop_thres = pqos_param->drop_thres;
	ptarget_qos_param->app_type = pqos_param->app_type;
/*
	if (ptarget_qos_param->delay_req <= 10)
		ptarget_qos_param->priority = ac_queue_to_up[3];
	else if (ptarget_qos_param->delay_req <= 50)
		ptarget_qos_param->priority = ac_queue_to_up[2];
	else if (ptarget_qos_param->delay_req <= 500)
		ptarget_qos_param->priority = ac_queue_to_up[1];
	else
		ptarget_qos_param->priority = ac_queue_to_up[0];
*/
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO,
		"Set qos[%u]: prio:%u,force_ac:%u,delay_bound:%u,delay_req:%u,weight=%u,data_rate:%u,dir:%u,dropth:%u,app_type:%u\n",
		idx,
		ptarget_qos_param->priority,
		ptarget_qos_param->force_ac,
		ptarget_qos_param->delay_bound,
		ptarget_qos_param->delay_req,
		ptarget_qos_param->delay_weight,
		ptarget_qos_param->data_rate,
		ptarget_qos_param->dir,
		ptarget_qos_param->drop_thres,
		ptarget_qos_param->app_type
		);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_tbl_by_idx(unsigned short idx, struct qos_param_rec *pqos_param)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	memcpy(&qos_param_table[idx], pqos_param, sizeof(struct qos_param_rec));
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}
unsigned short delete_qos_param_tbl_by_wlan_idx(
	PRTMP_ADAPTER pAd, unsigned short wlan_idx, struct wifi_dev *wdev)
{
	unsigned short idx;
	struct qos_param_rec *prec;
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
	struct wapp_vend_spec_classifier_para_report vend_spec_classifier_param = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif
#endif
	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		OS_SPIN_LOCK_BH(&qos_param_table_lock);
		prec = &qos_param_table[idx];

		if (prec->valid == FALSE) {
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			continue;
		}
		if (prec->wlan_idx != wlan_idx) {
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			continue;
		}
#ifdef QOS_R1
#ifdef MSCS_PROPRIETARY
		pEntry = entry_get(pAd, wlan_idx);
		vend_spec_classifier_param.id = idx;
		COPY_MAC_ADDR(vend_spec_classifier_param.sta_mac, (UCHAR *)pEntry->Addr);
		vend_spec_classifier_param.destIp.ipv4 = prec->ip_dest;
		vend_spec_classifier_param.srcIp.ipv4 = prec->ip_src;
		vend_spec_classifier_param.destPort = prec->sport;
		vend_spec_classifier_param.srcPort = prec->dport;
		vend_spec_classifier_param.protocol = prec->protocol;
		vend_spec_classifier_param.requet_type = SCS_REQ_TYPE_REMOVE;
		indicate_vendor_spec_tclas_elment(pAd, wdev, (UCHAR *)&vend_spec_classifier_param,
			sizeof(vend_spec_classifier_param));
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO,
			"removed, id:%d, mac:%pM\n",
			idx, pEntry->Addr);
#endif
#endif
		delete_qos_param(pAd, idx);

		OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
	}

	return TRUE;
}
bool update_qos_param(PRTMP_ADAPTER pAd, UINT32 idx, struct qos_param_rec_add *qos_param_rec_add)
{
	bool ret = TRUE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct qos_param_rec *pqos_param;
	UINT32 min_delaybound_idx = 0;

	pEntry = MacTableLookup(pAd, qos_param_rec_add->dest_mac);
	if (pEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"pEntry == NULL!!, (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n",
			PRINT_MAC(qos_param_rec_add->dest_mac));
		return FALSE;
	}

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	pqos_param = &qos_param_table[idx];
	memset(pqos_param, 0, sizeof(struct qos_param_rec));
	dabs_dly_dbglvl(idx);
	pqos_param->pEntry = pEntry;
	pEntry->qos_tbl_idx[qos_param_rec_add->priority] = idx + 1;
	pqos_param->wlan_idx = pEntry->wcid;
	pqos_param->ip_src = qos_param_rec_add->ip_src;
	pqos_param->ip_dest = qos_param_rec_add->ip_dest;
	pqos_param->sport = htons(qos_param_rec_add->sport);
	pqos_param->dport = htons(qos_param_rec_add->dport);
	pqos_param->protocol = qos_param_rec_add->protocol;
	pqos_param->priority = qos_param_rec_add->priority;
	pqos_param->force_ac = qos_param_rec_add->force_ac;
	pqos_param->delay_bound = qos_param_rec_add->delay_bound;
	pqos_param->delay_req = qos_param_rec_add->delay_req;
	pqos_param->delay_weight = qos_param_rec_add->delay_weight;
	pqos_param->bw_req = qos_param_rec_add->bw_req;
	pqos_param->data_rate = qos_param_rec_add->data_rate;
	pqos_param->dir = qos_param_rec_add->dir;
	pqos_param->drop_thres = qos_param_rec_add->drop_thres;
	pqos_param->app_type = qos_param_rec_add->app_type;
	pqos_param->in_use = TRUE;
	pqos_param->valid = TRUE;
	min_delaybound_idx = get_min_delaybound_by_ac(pEntry->wcid, qos_param_rec_add->priority);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
	if (min_delaybound_idx == idx) {
		if (HW_UPDATE_QOS_PARAM(pAd, idx, TRUE) != NDIS_STATUS_SUCCESS)	{
			disable_qos_param_tbl_by_idx(idx);
			ret = FALSE;
		}
	}
	return ret;
}

bool delete_qos_param(PRTMP_ADAPTER pAd, UINT32 idx)
{
	bool ret = TRUE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT32 min_delaybound_idx = 0;
	struct qos_param_rec *pqos_param = NULL;
	unsigned short wlan_idx;
	unsigned short priority;

	if(idx >= MAX_QOS_PARAM_TBL)
		return FALSE;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	pqos_param = &qos_param_table[idx];
	wlan_idx = pqos_param->wlan_idx;
	priority = pqos_param->priority;
	pqos_param->in_use = FALSE;
	min_delaybound_idx = get_min_delaybound_by_ac(wlan_idx, priority);
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO,
		"priority = %d ,min_delaybound_idx=%d!!\n", priority, min_delaybound_idx);

	if (min_delaybound_idx == MAX_QOS_PARAM_TBL) {
		if (HW_UPDATE_QOS_PARAM(pAd, idx, FALSE) != NDIS_STATUS_SUCCESS)
			ret = FALSE;
	} else if (pqos_param->delay_req < qos_param_table[min_delaybound_idx].delay_req) {
		if (HW_UPDATE_QOS_PARAM(pAd, min_delaybound_idx, TRUE) != NDIS_STATUS_SUCCESS)
			ret = FALSE;
	}

	OS_SPIN_LOCK_BH(&qos_param_table_lock);

	pEntry = pqos_param->pEntry;

	if (pEntry)
		pEntry->qos_tbl_idx[pqos_param->priority] = 0;

	memset(pqos_param, 0, sizeof(struct qos_param_rec));
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool enable_qos_param_tbl_by_idx(unsigned short idx)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	qos_param_table[idx].valid = true;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool disable_qos_param_tbl_by_idx(unsigned short idx)
{
	bool ret = true;

	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	OS_SPIN_LOCK_BH(&qos_param_table_lock);
	qos_param_table[idx].valid = false;
	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	return ret;
}

bool set_qos_param_to_fw(PRTMP_ADAPTER pAd, struct qos_param_set_del *pqos_param_set_del, BOOLEAN set_del)
{
	bool ret = true;
	MURU_QOS_SETTING qos_setting = {0};
	USHORT idx = pqos_param_set_del->idx;
	if (idx >= MAX_QOS_PARAM_TBL)
		return false;

	memcpy(&qos_setting, &pqos_param_set_del->qos_setting, sizeof(MURU_QOS_SETTING));

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO,
				 "idx(%d)delay_req%d\n", idx, qos_setting.u2DelayReq);
	/*qos_setting.u2WlanIdx = pqos_param->wlan_idx;
	qos_setting.u1AC = up_to_ac_mapping[pqos_param->priority];
	qos_setting.u1ForceAC = pqos_param->force_ac;
	qos_setting.u2DelayBound = pqos_param->delay_bound;
	qos_setting.u2DelayReq = pqos_param->delay_req;
	qos_setting.u1DelayWeight = pqos_param->delay_weight;
	qos_setting.u4DataRate = pqos_param->data_rate;
	qos_setting.u2BWReq = pqos_param->bw_req;
	qos_setting.u1Dir = pqos_param->dir;
	qos_setting.u2DropThres = pqos_param->drop_thres;
	qos_setting.u1Idx = idx;*/

	if (set_del == TRUE) {
		if (SendQoSCmd(pAd, QOS_CMD_PARAM_SETTING, &qos_setting) == false) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"SendQoSCmd fail!!!\n");
			ret = false;
		}
	} else {
		if (SendQoSCmd(pAd, QOS_CMD_PARAM_DELETE, &qos_setting) == false) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"SendQoSCmd fail!!!\n");
			ret = false;
		}
	}

	return ret;
}

INT set_dabs_qos_param(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 cmd = 3, rv, ret = 1, idx = 0;
	UINT32 param[13] = {0};
	UINT32 ip_src[4] = {0}, ip_dest[4] = {0};
	MURU_QOS_SETTING qos_setting = {0};
	struct qos_param_rec target_qos_param, *pqos_param;
	UINT_32 au4Keybitmap[4] = {0};
#ifdef WIFI_UNIFIED_COMMAND
	struct _UNI_EVENT_FAST_PATH_PROCESS_T event_fastpath;
#else
	EVENT_FAST_PATH_T event_fastpath;
#endif


	if (arg) {
		rv = sscanf(arg, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u", &cmd, &param[0], &param[1], &param[2],
			&param[3], &param[4], &param[5], &param[6], &param[7], &param[8], &param[9], &param[10],
			&param[11]);

		if (rv == 0) {
			ret = 0;
			goto error;
		}
		switch (cmd) {
		case 0:
			rv = sscanf(arg, "%u-%u-%u.%u.%u.%u-%u.%u.%u.%u-%u-%u-%u",
				&cmd, &param[0],
				&ip_src[0], &ip_src[1], &ip_src[2], &ip_src[3],
				&ip_dest[0], &ip_dest[1], &ip_dest[2], &ip_dest[3],
				&param[3], &param[4], &param[5]);
			if (rv == 0) {
				ret = 0;
				goto error;
			}

			param[1] = (ip_src[3]<<24) | (ip_src[2] << 16) | (ip_src[1] << 8)
				| (ip_src[0] << 0);
			param[2] = (ip_dest[3]<<24) | (ip_dest[2] << 16) | (ip_dest[1] << 8)
				| (ip_dest[0] << 0);

			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"ip_src=%u.%u.%u.%u, ip_dest=%u.%u.%u.%u\n",
				NIPQUAD(param[1]), NIPQUAD(param[2]));

			target_qos_param.ip_src = param[1];
			target_qos_param.ip_dest = param[2];
			target_qos_param.protocol = param[3];
			target_qos_param.sport = htons(param[4]);
			target_qos_param.dport = htons(param[5]);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"ip_src=%u.%u.%u.%u, ip_dest=%u.%u.%u.%u,proto=%u,sp=%u,dp=%u\n",
				NIPQUAD(target_qos_param.ip_src),
				NIPQUAD(target_qos_param.ip_dest),
				target_qos_param.protocol,
				ntohs(target_qos_param.sport),
				ntohs(target_qos_param.dport));


			set_qos_param_tbl_ipaddr_by_idx(param[0], &target_qos_param);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"set qos_param ipaddr for idx:%u\n", param[0]);
			break;
		case 1:
			target_qos_param.priority = param[2]; /* up to ac by set_qos_param_tbl_qos_by_idx */
			target_qos_param.delay_bound = param[3];
			target_qos_param.delay_req = param[4];
			target_qos_param.delay_weight = param[5];
			target_qos_param.data_rate = param[6];
			target_qos_param.bw_req = param[7];
			target_qos_param.dir = param[8];
			target_qos_param.drop_thres = param[9];
			target_qos_param.app_type = param[10];
			target_qos_param.force_ac = param[11];
			set_qos_param_tbl_qos_by_idx(param[0], &target_qos_param);
			if (param[1] > 0)
				set_qos_param_tbl_wlan_idx_by_idx(pAd, param[0], param[1]);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"set qos_param qos related for idx:%u\n", param[0]);
			if ((target_qos_param.delay_req > 0) && (target_qos_param.delay_req < 65535)) {
				if (pAd->pbc_bound[PBC_AC_BK] == PBC_WMM_UP_DEFAULT_BK) {
					pAd->pbc_bound[PBC_AC_VO] = 1200;
					pAd->pbc_bound[PBC_AC_VI] = 4000;
					pAd->pbc_bound[PBC_AC_BE] = 1200;
					pAd->pbc_bound[PBC_AC_BK] = 1000;

					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
						"set pbc_ubound=[%u,%u,%u,%u]\n",
						pAd->pbc_bound[PBC_AC_BK], pAd->pbc_bound[PBC_AC_BE],
						pAd->pbc_bound[PBC_AC_VI], pAd->pbc_bound[PBC_AC_VO]);
				}
			}
			break;
		case 2:
			set_qos_param_tbl_wlan_idx_by_idx(pAd, param[0], param[1]);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"set qos_param wlan:%u related for idx:%u\n", param[1], param[0]);
			break;
		case 3:
			if (param[1] != 0) {
				enable_qos_param_tbl_by_idx(param[0]);
				if (HW_UPDATE_QOS_PARAM(pAd, param[0], TRUE) != NDIS_STATUS_SUCCESS) {
					ret = 0;
					disable_qos_param_tbl_by_idx(param[0]);
				} else
					dabs_dly_dbglvl(param[0]);
			} else {
				HW_UPDATE_QOS_PARAM(pAd, param[0], FALSE);

				disable_qos_param_tbl_by_idx(param[0]);
			}
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"qos_param tbl[%d] valid:%u\n", param[0],param[1]);
			break;
		case 4:
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
				pqos_param = &qos_param_table[idx];
				if (pqos_param->ip_dest == 0)
					continue;
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
"dump qos_param[%d]: ip_src=%u.%u.%u.%u,ip_dest=%u.%u.%u.%u,proto=%u,sport=%u,dport=%u\n",
				idx,
			NIPQUAD(pqos_param->ip_src),
			NIPQUAD(pqos_param->ip_dest),
			pqos_param->protocol,
			ntohs(pqos_param->sport),
			ntohs(pqos_param->dport)
			);

			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"dump qos_param[%d]: valid:%u,prio:%u,force_ac:%u,delay_bound:%u,delay_req:%u,delay_weight:%u,data_rate:%u,bw_req:%u,dir=%u,dropth=%u,app_type:%u,in_use=%u\n",
			param[0],
			pqos_param->valid,
			pqos_param->priority,
			pqos_param->force_ac,
			pqos_param->delay_bound,
			pqos_param->delay_req,
			pqos_param->delay_weight,
			pqos_param->data_rate,
			pqos_param->bw_req,
			pqos_param->dir,
			pqos_param->drop_thres,
			pqos_param->app_type,
			pqos_param->in_use
			);
			}
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			SendQoSCmd(pAd, QOS_CMD_PARAM_DUMP, &qos_setting);
			break;
		case 5:
			pAd->dabs_qos_op = param[0];
			if (pAd->dabs_qos_op & DABS_DBG_DLY_TIME)
				net_enable_timestamp();
			else
	                        net_disable_timestamp();
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"dabs_qos_op:%u\n", param[0]);
			break;
		case 6:
			ret = SendQoSCmd(pAd, QOS_CMD_PARAM_RESET, &qos_setting);
			for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++)
				delete_qos_param(pAd, idx);
			pAd->pbc_bound[PBC_AC_BE] = PBC_WMM_UP_DEFAULT_BE;
			pAd->pbc_bound[PBC_AC_BK] = PBC_WMM_UP_DEFAULT_BK;
			pAd->pbc_bound[PBC_AC_VO] = PBC_WMM_UP_DEFAULT_VO;
			pAd->pbc_bound[PBC_AC_VI] = PBC_WMM_UP_DEFAULT_VI;
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"clean up table ret=%u\n", ret);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"reset pbc_ubound=[%u,%u,%u,%u]\n",
				pAd->pbc_bound[PBC_AC_BK], pAd->pbc_bound[PBC_AC_BE],
				pAd->pbc_bound[PBC_AC_VI], pAd->pbc_bound[PBC_AC_VO]);

			break;
		case 7:
			for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
				pqos_param = &qos_param_table[idx];
				OS_SPIN_LOCK_BH(&qos_param_table_lock);
				if (pqos_param->valid) {
					unsigned int avg_dly = 0;

					if (pqos_param->tot_pkt_cnt)
						avg_dly = div64_ul(pqos_param->tot_pkt_dly, pqos_param->tot_pkt_cnt);

					MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
						"qos[%d]:dly_req:%u,avg_dly:%u(max=%u)[%llu/%u]\n", idx,
						pqos_param->delay_req,
						avg_dly,
						pqos_param->max_pkt_dly,
						pqos_param->tot_pkt_dly,
						pqos_param->tot_pkt_cnt);
					pqos_param->tot_pkt_dly = 0;
					pqos_param->tot_pkt_cnt = 0;
					pqos_param->max_pkt_dly = 0;
				}
				OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
			}
			OS_SPIN_LOCK_BH(&qos_param_table_lock);
			dabs_latency_show(pAd);
			OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

			SendQoSCmd(pAd, QOS_CMD_RESULT_DUMP, &qos_setting);
			break;
		case 8:
#ifdef WIFI_UNIFIED_COMMAND
			NdisZeroMemory(&event_fastpath, sizeof(struct _UNI_EVENT_FAST_PATH_PROCESS_T));
#else
			NdisZeroMemory(&event_fastpath, sizeof(EVENT_FAST_PATH_T));
#endif

			FastPathCheckMIC(pAd, FAST_PATH_CMD_CAL_MIC_TEST_MODE, 0, 0, MIC_AP, 0,
					au4Keybitmap, &event_fastpath);
			break;
		case 9:
#ifdef WIFI_UNIFIED_COMMAND
			{
				UINT_8 ucAC;
				struct EPCS_CMD_ENABLE_STA_T epcs_cmd;
				struct EPCS_EDCA_ENTRY_T *entry = &epcs_cmd.EdcaSetting;

				memset(&epcs_cmd, 0, sizeof(struct EPCS_CMD_ENABLE_STA_T));
				epcs_cmd.u2WlanIdx = param[1];
				epcs_cmd.u2Idx = param[2];
				epcs_cmd.fgEnable = param[3];

				for (ucAC = 0; ucAC < AC_NUM; ucAC++) {
					entry->EdcaParam[ucAC].ucCWmin = param[4] - ucAC*1;
					entry->EdcaParam[ucAC].ucCWmax = param[5] - ucAC*2;
					entry->EdcaParam[ucAC].ucAifsn = param[6] - ucAC*1;
					entry->EdcaParam[ucAC].u2TxopLimit = param[7] - ucAC*4;
					entry->EdcaParam[ucAC].u2TxopLimit = cpu2le16(entry->EdcaParam[ucAC].u2TxopLimit);
					entry->EdcaParam[ucAC].ucMuEdcaCWmin = param[4] - ucAC*1 + 1;
					entry->EdcaParam[ucAC].ucMuEdcaCWmax = param[5] - ucAC*2 + 3;
					entry->EdcaParam[ucAC].ucMuEdcaAifsn = param[6] - ucAC*1 + 1;
					entry->EdcaParam[ucAC].ucMuEdcaTimer = param[7] + 8;

				}
				uni_cmd_epcs_ctrl(pAd, param[0], (UINT8 *)&epcs_cmd);
			}
#endif
			break;
		default:
			ret = 0;
			goto error;
		}
	}

error:

	if (ret == 0) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=0-[idx]-[ip_src]-[ip_dest]-[proto]-[sport]-[dport] (set ip by idx)\n");
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=1-[idx]-[wlanid]-[AC]-[DlyBound]-[DlyReq]-[DlyWt]-[Rate]-[BWReq]-[Dir]-[DropTh]-[app]-[ForceAC] (set qos by idx)\n");
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=4 (dump table)\n");
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=5-[dabs_qos_op(bit2:] (set dabs operation)\n");
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=6 (clean up table)\n");
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"iwpriv ra0 set dabs_qos=7 (dump host delay)\n");
	}

	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "\n");


	return ret;
}

void dabs_host_delay(PRTMP_ADAPTER pAd,  NDIS_PACKET *pkt)
{
	UINT16 qos_idx;

	if (pkt == NULL)
		return;

	qos_idx = RTMP_GET_PACKET_QOS_IDX(pkt);

	if (qos_idx < MAX_QOS_PARAM_TBL) {
		struct qos_param_rec *prec;
		bool valid = FALSE;

		prec = &qos_param_table[qos_idx];

		OS_SPIN_LOCK_BH(&qos_param_table_lock);
		valid = prec->valid;
		OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

		if (valid == TRUE) {
			UINT32 delta_us = 0;
#ifdef PROPRIETARY_DRIVER_SUPPORT
			struct timespec64 kts64 = {0};
			ktime_t kts, delta_kt;

			ktime_get_real_ts64(&kts64);
			kts = timespec64_to_ktime(kts64);
			delta_kt = ktime_sub(kts, RTPKT_TO_OSPKT(pkt)->tstamp);
#else
			ktime_t delta_kt = net_timedelta(RTPKT_TO_OSPKT(pkt)->tstamp);
#endif
			delta_us = ktime_to_us(delta_kt);

			prec->tot_pkt_cnt++;
			prec->tot_pkt_dly += delta_us;

			if (delta_us > prec->max_pkt_dly) {
				prec->max_pkt_dly = delta_us;
			}

			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV3) {
				prec->some_pkt_dly += delta_us;
				if ((prec->tot_pkt_cnt % 1000) == 999) {
					prec->avg_pkt_dly =  prec->some_pkt_dly/1000;
					prec->some_pkt_dly = 0;
				}
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
					"qos[%d] deltaus=%u\n",qos_idx, delta_us);
			}
		}
	}

	return;
}

static void dabs_dly_dbglvl(UINT_8 idx)
{
	unsigned short delay_req;
	struct qos_param_rec *prec;

	prec = &qos_param_table[idx];
	delay_req = prec->delay_req;

	if (delay_req > 11) {
		prec->delay_lvl[15] = delay_req + (delay_req >> 1) + (delay_req << 6);
		prec->delay_lvl[14] = delay_req + (delay_req >> 1) + (delay_req << 5);
		prec->delay_lvl[13] = delay_req + (delay_req >> 1) + (delay_req << 4);
		prec->delay_lvl[12] = delay_req + (delay_req >> 1) + (delay_req << 3);
		prec->delay_lvl[11] = delay_req + (delay_req >> 1) + (delay_req << 2);
		prec->delay_lvl[10] = delay_req + (delay_req >> 1) + (delay_req << 1);
		prec->delay_lvl[9] = delay_req + (delay_req >> 1) + delay_req;
		prec->delay_lvl[8] = delay_req + (delay_req >> 1);
		prec->delay_lvl[7] = delay_req;
		prec->delay_lvl[6] = delay_req - (delay_req >> 1);
		prec->delay_lvl[5] = 5;
		prec->delay_lvl[4] = 4;
		prec->delay_lvl[3] = 3;
		prec->delay_lvl[2] = 2;
		prec->delay_lvl[1] = 1;
		prec->delay_lvl[0] = 0;
	} else {
		prec->delay_lvl[15] = delay_req*100 + (delay_req*100 >> 1);
		prec->delay_lvl[14] = delay_req*100;
		prec->delay_lvl[13] = delay_req*50;
		prec->delay_lvl[12] = delay_req*10;
		prec->delay_lvl[11] = delay_req + (delay_req >> 1);
		prec->delay_lvl[10] = delay_req + (delay_req >> 2);
		prec->delay_lvl[9] = delay_req;
		prec->delay_lvl[8] = 8;
		prec->delay_lvl[7] = 7;
		prec->delay_lvl[6] = 6;
		prec->delay_lvl[5] = 5;
		prec->delay_lvl[4] = 4;
		prec->delay_lvl[3] = 3;
		prec->delay_lvl[2] = 2;
		prec->delay_lvl[1] = 1;
		prec->delay_lvl[0] = 0;
	}
}

void dabs_latency_update(
	RTMP_ADAPTER *pAd,
	UINT_8 qid,
	UINT_16 wcid,
	UINT_16 mac_latency,
	UINT_16 air_latency,
	UINT_32 msdu_cnt
)
{
	unsigned short idx;
	unsigned short qlatency;
	struct qos_param_rec *prec;
	PMAC_TABLE_ENTRY pEntry;

	pEntry = entry_get(pAd, wcid);

	OS_SPIN_LOCK_BH(&qos_param_table_lock);

	idx = pEntry->qos_tbl_idx[ac_queue_to_up[qid % AC_NUM]];

	OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

	if ((!idx) || (idx > MAX_QOS_PARAM_TBL))
		return;

	prec = &qos_param_table[idx - 1];

	if (prec->max_mac_dly < mac_latency)
		prec->max_mac_dly = mac_latency;

	qlatency = mac_latency - air_latency;

	if (mac_latency >= prec->delay_lvl[15])
		prec->delay_lvl_cnt[15] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[14])
		prec->delay_lvl_cnt[14] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[13])
		prec->delay_lvl_cnt[13] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[12])
		prec->delay_lvl_cnt[12] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[11])
		prec->delay_lvl_cnt[11] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[10])
		prec->delay_lvl_cnt[10] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[9])
		prec->delay_lvl_cnt[9] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[8])
		prec->delay_lvl_cnt[8] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[7])
		prec->delay_lvl_cnt[7] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[6])
		prec->delay_lvl_cnt[6] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[5])
		prec->delay_lvl_cnt[5] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[4])
		prec->delay_lvl_cnt[4] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[3])
		prec->delay_lvl_cnt[3] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[2])
		prec->delay_lvl_cnt[2] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[1])
		prec->delay_lvl_cnt[1] += msdu_cnt;
	else if (mac_latency >= prec->delay_lvl[0])
		prec->delay_lvl_cnt[0] += msdu_cnt;

	if (qlatency >= prec->delay_lvl[15])
		prec->qdelay_lvl_cnt[15] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[14])
		prec->qdelay_lvl_cnt[14] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[13])
		prec->qdelay_lvl_cnt[13] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[12])
		prec->qdelay_lvl_cnt[12] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[11])
		prec->qdelay_lvl_cnt[11] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[10])
		prec->qdelay_lvl_cnt[10] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[9])
		prec->qdelay_lvl_cnt[9] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[8])
		prec->qdelay_lvl_cnt[8] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[7])
		prec->qdelay_lvl_cnt[7] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[6])
		prec->qdelay_lvl_cnt[6] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[5])
		prec->qdelay_lvl_cnt[5] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[4])
		prec->qdelay_lvl_cnt[4] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[3])
		prec->qdelay_lvl_cnt[3] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[2])
		prec->qdelay_lvl_cnt[2] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[1])
		prec->qdelay_lvl_cnt[1] += msdu_cnt;
	else if (qlatency >= prec->delay_lvl[0])
		prec->qdelay_lvl_cnt[0] += msdu_cnt;


	if (mac_latency == 0)
		mac_latency = 1;
	else
		mac_latency <<= 1;

	prec->tot_msdu_cnt += msdu_cnt;
	prec->tot_mac_dly += ((unsigned long long)mac_latency*msdu_cnt);
}

static void dabs_latency_show(RTMP_ADAPTER *pAd)
{
	char buf[256] = {0};
	struct qos_param_rec *prec;
	UINT_8 pos = 0;
	int idx;
	int ret = 0;

	for (idx = 0; idx < MAX_QOS_PARAM_TBL; idx++) {
		unsigned int avg_dly = 0;

		prec = &qos_param_table[idx];

		if (prec->in_use == FALSE)
			continue;

		if (prec->tot_msdu_cnt == 0)
			continue;

		avg_dly = (prec->tot_mac_dly >> 1) / prec->tot_msdu_cnt;
		ret = snprintf(buf, sizeof(buf),
			"\nQTBL[%d]STA%u,FAC%u,DataRate=%u,DlyReq:%u,dropth:%u,maxdly=%u,avgdly=%u(%llu/%u)\n",
			idx,
			prec->wlan_idx,
			prec->force_ac,
			prec->data_rate,
			prec->delay_req,
			prec->drop_thres,
			prec->max_mac_dly,
			avg_dly, prec->tot_mac_dly >> 1, prec->tot_msdu_cnt);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "\n%s", buf);

		pos = 0;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  > %u : %u\r\n", prec->delay_lvl[15], prec->delay_lvl_cnt[15]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[15], prec->delay_lvl[14],
							prec->delay_lvl_cnt[14]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[14], prec->delay_lvl[13],
							prec->delay_lvl_cnt[13]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[13], prec->delay_lvl[12],
							prec->delay_lvl_cnt[12]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[12], prec->delay_lvl[11],
							prec->delay_lvl_cnt[11]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[11], prec->delay_lvl[10],
							prec->delay_lvl_cnt[10]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[10], prec->delay_lvl[9],
							prec->delay_lvl_cnt[9]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[9], prec->delay_lvl[8],
							prec->delay_lvl_cnt[8]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "\n%s", buf);

		pos = 0;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[8], prec->delay_lvl[7],
							prec->delay_lvl_cnt[7]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[7], prec->delay_lvl[6],
							prec->delay_lvl_cnt[6]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[6], prec->delay_lvl[5],
							prec->delay_lvl_cnt[5]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[5], prec->delay_lvl[4],
							prec->delay_lvl_cnt[4]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[4], prec->delay_lvl[3],
							prec->delay_lvl_cnt[3]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[3], prec->delay_lvl[2],
							prec->delay_lvl_cnt[2]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[2], prec->delay_lvl[1],
							prec->delay_lvl_cnt[1]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "Dly  %u ~ %u : %u\r\n", prec->delay_lvl[1], prec->delay_lvl[0],
							prec->delay_lvl_cnt[0]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}

		prec->tot_mac_dly = 0;
		prec->tot_msdu_cnt = 0;
		prec->max_mac_dly = 0;

		prec->delay_lvl_cnt[15] = 0;
		prec->delay_lvl_cnt[14] = 0;
		prec->delay_lvl_cnt[13] = 0;
		prec->delay_lvl_cnt[12] = 0;
		prec->delay_lvl_cnt[11] = 0;
		prec->delay_lvl_cnt[10] = 0;
		prec->delay_lvl_cnt[9] = 0;
		prec->delay_lvl_cnt[8] = 0;
		prec->delay_lvl_cnt[7] = 0;
		prec->delay_lvl_cnt[6] = 0;
		prec->delay_lvl_cnt[5] = 0;
		prec->delay_lvl_cnt[4] = 0;
		prec->delay_lvl_cnt[3] = 0;
		prec->delay_lvl_cnt[2] = 0;
		prec->delay_lvl_cnt[1] = 0;
		prec->delay_lvl_cnt[0] = 0;
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "\n%s", buf);

		pos = 0;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  > %u : %u\r\n", prec->delay_lvl[15], prec->qdelay_lvl_cnt[15]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[15], prec->delay_lvl[14],
							prec->qdelay_lvl_cnt[14]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[14], prec->delay_lvl[13],
							prec->qdelay_lvl_cnt[13]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[13], prec->delay_lvl[12],
							prec->qdelay_lvl_cnt[12]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[12], prec->delay_lvl[11],
							prec->qdelay_lvl_cnt[11]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[11], prec->delay_lvl[10],
							prec->qdelay_lvl_cnt[10]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[10], prec->delay_lvl[9],
							prec->qdelay_lvl_cnt[9]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[9], prec->delay_lvl[8],
							prec->qdelay_lvl_cnt[8]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "\n%s", buf);
		pos = 0;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[8], prec->delay_lvl[7],
							prec->qdelay_lvl_cnt[7]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[7], prec->delay_lvl[6],
							prec->qdelay_lvl_cnt[6]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[6], prec->delay_lvl[5],
							prec->qdelay_lvl_cnt[5]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[5], prec->delay_lvl[4],
							prec->qdelay_lvl_cnt[4]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[4], prec->delay_lvl[3],
							prec->qdelay_lvl_cnt[3]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[3], prec->delay_lvl[2],
							prec->qdelay_lvl_cnt[2]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[2], prec->delay_lvl[1],
							prec->qdelay_lvl_cnt[1]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}
		pos += ret;
		ret = snprintf(buf + pos, sizeof(buf) - pos, "QDly  %u ~ %u : %u\r\n", prec->delay_lvl[1], prec->delay_lvl[0],
							prec->qdelay_lvl_cnt[0]);
		if (ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"snprintf error!\n");
			return;
		}

		prec->qdelay_lvl_cnt[15] = 0;
		prec->qdelay_lvl_cnt[14] = 0;
		prec->qdelay_lvl_cnt[13] = 0;
		prec->qdelay_lvl_cnt[12] = 0;
		prec->qdelay_lvl_cnt[11] = 0;
		prec->qdelay_lvl_cnt[10] = 0;
		prec->qdelay_lvl_cnt[9] = 0;
		prec->qdelay_lvl_cnt[8] = 0;
		prec->qdelay_lvl_cnt[7] = 0;
		prec->qdelay_lvl_cnt[6] = 0;
		prec->qdelay_lvl_cnt[5] = 0;
		prec->qdelay_lvl_cnt[4] = 0;
		prec->qdelay_lvl_cnt[3] = 0;
		prec->qdelay_lvl_cnt[2] = 0;
		prec->qdelay_lvl_cnt[1] = 0;
		prec->qdelay_lvl_cnt[0] = 0;
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "\n%s", buf);
	}
}

void dabs_active_qos_by_ipaddr(PRTMP_ADAPTER pAd,  NDIS_PACKET *pkt)
{
	UINT32 idx = MAX_QOS_PARAM_TBL;
	UINT8 *pData = GET_OS_PKT_DATAPTR(pkt);
	UINT32 ip_dest, ip_src, proto, sport = 0,dport = 0;

	pData += (LENGTH_802_3);
	ip_src = *((UINT32*)(pData + 12));
	ip_dest = *((UINT32*)(pData + 16));
	proto = *((UINT8*)(pData + 9));

	if ((ip_src != 0) && (ip_dest != 0) && ((ip_dest & 0x0FF) != 0x0FF)) {
		if (RTMP_GET_PACKET_WCID(pkt) < (WTBL_MAX_NUM(pAd) - 16)) {
			idx = search_qos_param_tbl_idx_by_5_tuple(pAd, RTPKT_TO_OSPKT(pkt));
			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV3) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
					"WCID%u,hit=%u src=%u.%u.%u.%u,dest=%u.%u.%u.%u, proto=%u,sp=%u,dp=%u\n",
					RTMP_GET_PACKET_WCID(pkt), idx,
					NIPQUAD(ip_src), NIPQUAD(ip_dest), proto, sport, dport);
			}
		}
	}

	RTMP_SET_PACKET_QOS_IDX(pkt, MAX_QOS_PARAM_TBL);

	if (idx < MAX_QOS_PARAM_TBL) {
		struct qos_param_rec *prec;
		bool valid = FALSE;
		prec = &qos_param_table[idx];

		OS_SPIN_LOCK_BH(&qos_param_table_lock);
		valid = prec->valid;
		OS_SPIN_UNLOCK_BH(&qos_param_table_lock);

		if (valid == FALSE) {
			set_qos_param_tbl_wlan_idx_by_idx(pAd, idx, RTMP_GET_PACKET_WCID(pkt));
			if (pAd->dabs_qos_op & DABS_DBG_PRN_LV1) {
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"Set QTBL for STA%u,idx=%u,src=%u.%u.%u.%u,dest=%u.%u.%u.%u, proto=%u,sp=%u,dp=%u\n",
					RTMP_GET_PACKET_WCID(pkt), idx,
					NIPQUAD(ip_src), NIPQUAD(ip_dest), proto, htons(sport), htons(dport));
			}
			enable_qos_param_tbl_by_idx(idx);

			if (pAd->dabs_qos_op & DABS_SET_QOS_PARAM) {
				if (HW_UPDATE_QOS_PARAM(pAd, idx, TRUE) != NDIS_STATUS_SUCCESS)
					disable_qos_param_tbl_by_idx(idx);
			}
		} else {
			RTMP_SET_PACKET_QOS_IDX(pkt, idx);
		}
	}
}

INT Show_qos_dabs_table_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	struct qos_param_rec *pqos_param;
	UCHAR ip_dest[4];
	UCHAR ip_src[4];
	PMAC_TABLE_ENTRY pEntry;

	for (i = 0; i < MAX_QOS_PARAM_TBL; i++) {
		OS_SPIN_LOCK_BH(&qos_param_table_lock);
		pqos_param = &qos_param_table[i];
		pEntry = entry_get(pAd, pqos_param->wlan_idx);
		if (pqos_param->in_use) {
			memcpy(ip_dest, &pqos_param->ip_dest, 4);
			memcpy(ip_src, &pqos_param->ip_src, 4);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO,
				"[%d]sta_mac:" MACSTR "\ndescip: %d.%d.%d.%d, sourceip: %d.%d.%d.%d\n",
				i, MAC2STR(pEntry->Addr), ip_dest[0], ip_dest[1], ip_dest[2], ip_dest[3],
				ip_src[0], ip_src[1], ip_src[2], ip_src[3]);
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO,
					"descport:%d, srcport:%d, UP=%d, delay_req=%d\n",
					pqos_param->dport, pqos_param->sport,
					 pqos_param->priority, pqos_param->delay_req);

		}
		OS_SPIN_UNLOCK_BH(&qos_param_table_lock);
	}
	return TRUE;
}
#ifdef WIFI_UNIFIED_COMMAND
BOOLEAN FastPathCheckMIC(RTMP_ADAPTER *pAd, UINT_8 ucOpMode, UINT_16 u2WlanId,
	UINT_16 u2RandomNum, UINT_8 mic_type, UINT_16 u2mic, UINT_32 *au4Keybitmap, struct _UNI_EVENT_FAST_PATH_PROCESS_T *event_fastpath)
{
	int ret = NDIS_STATUS_SUCCESS;
	struct UNI_CMD_FAST_PATH_T cmd;
	struct UNI_CMD_FAST_PATH_PROCESS_T fastpath_cmd;
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	MAC_TABLE_ENTRY *pEntry = entry_get(pAd, u2WlanId);
	UINT32 u4ComCmdSize = 0;
	UINT32 u4CmdNeedMaxBufSize = 0;

	os_zero_mem(&fastpath_cmd, sizeof(struct UNI_CMD_FAST_PATH_PROCESS_T));

	if (UniCmdCheckInitReady(pAd) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"%s(): System isn't ready for in-band command now!!!\n", __func__);
		ret = NDIS_STATUS_FAILURE;
		goto error;
	}

	u4ComCmdSize = sizeof(struct UNI_CMD_FAST_PATH_T);
	os_zero_mem(&cmd, u4ComCmdSize);

	u4CmdNeedMaxBufSize = u4ComCmdSize + sizeof(struct UNI_CMD_FAST_PATH_PROCESS_T);

	/* Step 2: Allocate memory space */
	msg = AndesAllocUniCmdMsg(pAd, u4CmdNeedMaxBufSize);
	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, UNI_CMD_ID_FAST_PATH);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, event_fastpath);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_UNI_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(struct _UNI_EVENT_FAST_PATH_PROCESS_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, UniEventCmdFastPathCalMICRsp);
	AndesInitCmdMsg(msg, attr);
	msg->seq = AndesGetCmdMsgSeq(pAd);

	/* Step 3: Fill and append common parameters here */
	AndesAppendCmdMsg(msg, (char *)&cmd, sizeof(cmd));

	/* Step 4: Fill TLV parameters here */
	fastpath_cmd.u2Tag = UNI_CMD_FAST_PATH_PROCESS;
	fastpath_cmd.u2Length = sizeof(struct UNI_CMD_FAST_PATH_PROCESS_T);

	fastpath_cmd.ucOpMode = ucOpMode;
	fastpath_cmd.ucCmdVer = 0;
	if (au4Keybitmap) {
		fastpath_cmd.u4Keybitmap[0] = au4Keybitmap[0];
		fastpath_cmd.u4Keybitmap[1] = au4Keybitmap[1];
		fastpath_cmd.u4Keybitmap[2] = au4Keybitmap[2];
		fastpath_cmd.u4Keybitmap[3] = au4Keybitmap[3];
	}
	fastpath_cmd.u2Mic = u2mic;
	fastpath_cmd.u2WlanId = u2WlanId;
	if (ucOpMode == FAST_PATH_CMD_CAL_MIC) {
		if (mic_type == MIC_STA)
			NdisCopyMemory(&fastpath_cmd.aucOwnMac[0],  pEntry->Addr, 6);
		else if (mic_type == MIC_AP)
			NdisCopyMemory(&fastpath_cmd.aucOwnMac[0],  pEntry->wdev->if_addr, 6);
	}

	fastpath_cmd.u2RandomNum = u2RandomNum;

	AndesAppendCmdMsg(msg, (char *)&fastpath_cmd, sizeof(fastpath_cmd));

	ret = chip_cmd_tx(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", ret);
	return ret;
}
#endif
VOID ExtEventFastPathRptHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	INT32 i;
	P_EXT_EVENT_FASTPATH_RPT_T prTxRptEvt;
	P_FASTPATH_RPT_T prTxRpt  = NULL;
	UINT32 *RecInUseBitmap;
	INT32 FW_QOS_PARAM_TBL = 32;
	UINT drop_percentage = 0, max_drop_percentage = 0;
	UINT TxCnt = 0, DropCnt = 0;

	prTxRptEvt = (P_EXT_EVENT_FASTPATH_RPT_T) Data;
	RecInUseBitmap = &prTxRptEvt->RecInUseBitmap[0];
	prTxRpt = (P_FASTPATH_RPT_T)(Data + sizeof(EXT_EVENT_FASTPATH_RPT_T));

	if (pAd->dabs_qos_op & DABS_DBG_PRN_LV2)
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"FPDBG:TokenU:[%u,%u],PageU:%u\n",
			prTxRptEvt->u2FreeTokenUnderflowCnt[0],
			prTxRptEvt->u2FreeTokenUnderflowCnt[1],
			prTxRptEvt->u2FreePageUnderflowCnt);


	for (i = 0; i < FW_QOS_PARAM_TBL; i++) {

		if ((RecInUseBitmap[i >> RED_INUSE_BITSHIFT] & (1 << (i & RED_INUSE_BITMASK))) == 0)
			continue;

		if (pAd->dabs_qos_op & DABS_DBG_PRN_LV2)
			MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"FPDBG:QTBL[%u],TxDrop:%u,TxCnt:%u,TxDly:%u\n",
				i, prTxRpt->u4DropCnt,
				prTxRpt->u4TxCnt, prTxRpt->u4TxDly);

		TxCnt = prTxRpt->u4TxCnt;
		DropCnt = prTxRpt->u4DropCnt;
		if (TxCnt > 0)
			drop_percentage = DropCnt * 100 / TxCnt;
		if (drop_percentage > max_drop_percentage)
		max_drop_percentage = drop_percentage;

		prTxRpt++;
	}
	if (max_drop_percentage > pAd->dabs_drop_threashold)
		pAd->mscs_req_reject = TRUE;
	else
		pAd->mscs_req_reject = FALSE;
}
#ifndef WIFI_UNIFIED_COMMAND
static VOID MtCmdFastPathCalMICRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_FAST_PATH_T pEventResult = (P_EVENT_FAST_PATH_T)Data;
	P_EVENT_FAST_PATH_T pFastpath = (P_EVENT_FAST_PATH_T)msg->attr.rsp.wb_buf_in_calbk;

	pFastpath->u2Mic = pEventResult->u2Mic;
	pFastpath->u4KeybitmapMatchStatus = pEventResult->u4KeybitmapMatchStatus;
	pFastpath->ucKeynum = pEventResult->ucKeynum;
	memcpy(pFastpath->cap.u4Keybitmap, pEventResult->cap.u4Keybitmap, 16);
	pFastpath->cap.ucSupportFastPath = pEventResult->cap.ucSupportFastPath;
	pFastpath->cap.ucVersion = pEventResult->cap.ucVersion;
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO, "ucKeynum:%d, u2Mic (%x)\n",
		pFastpath->ucKeynum, pFastpath->u2Mic);
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_INFO,
		"MatchStatus:%x, ExtCmd (0x%02x)\n",
		pFastpath->u4KeybitmapMatchStatus, msg->attr.ext_type);
	return;
}
#endif
#ifndef WIFI_UNIFIED_COMMAND
BOOLEAN FastPathCheckMIC(RTMP_ADAPTER *pAd, UINT_8 ucOpMode, UINT_16 u2WlanId,
	UINT_16 u2RandomNum, UINT_8 mic_type, UINT_16 u2mic, UINT_32 *au4Keybitmap, EVENT_FAST_PATH_T *event_fastpath)
{
	int ret = NDIS_STATUS_SUCCESS;
	CMD_FAST_PATH_T fastpath_cmd;
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	MAC_TABLE_ENTRY *pEntry = entry_get(pAd, u2WlanId);

	memset(&fastpath_cmd, 0, sizeof(CMD_FAST_PATH_T));
	fastpath_cmd.ucOpMode = ucOpMode;
	fastpath_cmd.u2CmdLen = sizeof(CMD_FAST_PATH_T);
	if (au4Keybitmap) {
		fastpath_cmd.u4Keybitmap[0] = au4Keybitmap[0];
		fastpath_cmd.u4Keybitmap[1] = au4Keybitmap[1];
		fastpath_cmd.u4Keybitmap[2] = au4Keybitmap[2];
		fastpath_cmd.u4Keybitmap[3] = au4Keybitmap[3];
	}
	fastpath_cmd.u2Mic = u2mic;
	fastpath_cmd.u2WlanId = u2WlanId;
	if (ucOpMode == FAST_PATH_CMD_CAL_MIC) {
	 if (mic_type == MIC_STA)
			NdisCopyMemory(&fastpath_cmd.aucOwnMac[0],  pEntry->Addr, 6);
		else if (mic_type == MIC_AP)
			NdisCopyMemory(&fastpath_cmd.aucOwnMac[0],  pEntry->wdev->if_addr, 6);
	}
	fastpath_cmd.u2RandomNum = u2RandomNum;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_FAST_PATH_T));

	if (!msg) {
		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			 "msg alloc fail!!!\n");
		ret = FALSE;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_FAST_PATH_CAL_MIC);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_FAST_PATH_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, event_fastpath);
	SET_CMD_ATTR_RSP_HANDLER(attr, MtCmdFastPathCalMICRsp);
	AndesInitCmdMsg(msg, attr);
#ifdef CFG_BIG_ENDIAN
	fastpath_cmd = cpu2le32(fastpath_cmd);
#endif
	AndesAppendCmdMsg(msg, (char *)&fastpath_cmd, sizeof(CMD_FAST_PATH_T));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			 "(ret = %d)(op:%u)\n", ret, ucOpMode);

	return ret;
}
#endif

void dabs_init(PRTMP_ADAPTER ad)
{
#ifdef DABS_QOS
#if defined(QOS_R1) && defined(MSCS_PROPRIETARY)
#ifdef WIFI_UNIFIED_COMMAND
	struct _UNI_EVENT_FAST_PATH_PROCESS_T event_fastpath;
#else
	EVENT_FAST_PATH_T event_fastpath;
#endif


	if (IS_QOS_ENABLE(ad)) {
		SendQoSCmd(ad, QOS_CMD_ENABLE_DLY_POLICY, NULL);
		if (FastPathCheckMIC(ad, FAST_PATH_CMD_GET_CAP, 0,
						0, NO_MIC, 0, NULL, &event_fastpath) == NDIS_STATUS_SUCCESS) {
			if (!event_fastpath.cap.ucSupportFastPath)
				MTWF_DBG(ad, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
						"fw not support dabs\n");
			else {
				ad->dabs_version = event_fastpath.cap.ucVersion;
				ad->SupportFastPath = event_fastpath.cap.ucSupportFastPath;
				}
		}
	} else
		SendQoSCmd(ad, QOS_CMD_PARAM_RESET, NULL);
#endif
#endif

}

#endif /* DABS_QOS */
