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

*/

#include "rt_config.h"

#ifdef FW_LOG_DUMP

#ifndef IP_ASSEMBLY
typedef struct ip_v4_hdr {
#ifdef CFG_BIG_ENDIAN
	UCHAR version:4, ihl:4;
#else
	UCHAR ihl:4, version:4;
#endif
	UCHAR tos;
	USHORT tot_len;
	USHORT identifier;
} IP_V4_HDR;
#endif

INT set_fw_log_dest_dir(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 index;
	CHAR last;
	UINT32 max_len = sizeof(pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir) - 1;
	int ret;

	for (index = 0; index < max_len; index++)
		if (*(arg + index + 1) == '\0')
			break;
	last = *(arg + index);

	if (last == '/')
		ret = snprintf(pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir, max_len, "%sfw_log.bin", arg);
	else
		ret = snprintf(pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir, max_len, "%s/fw_log.bin", arg);

	if (os_snprintf_error(max_len, ret)) {
		MTWF_PRINT("%s: snprintf error\n", __func__);
		return FALSE;
	}
	MTWF_PRINT("FW Binary log destination directory: %s\n", pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir);

	return TRUE;
}


INT set_binary_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	RTMP_STRING *dbg_module_str = NULL;
	UINT8 dbg_lvl;
	UINT32 dbg_module_idx;

	if (arg == NULL || strlen(arg) == 0)
		return FALSE;

	dbg_module_str = strsep(&arg, ":");
	dbg_module_idx = os_str_toul(dbg_module_str, 0, 10);

	if (arg == NULL || strlen(arg) == 0) {
		/* imply all modules */
		dbg_lvl = 0xff;
	} else {
		dbg_lvl = os_str_toul(arg, 0, 10);
	}

	if (dbg_module_idx >= BIN_DBG_LOG_NUM)
		return FALSE;

	pAd->physical_dev->fw_log_ctrl.debug_level_ctrl[dbg_module_idx] = dbg_lvl;

	MTWF_PRINT("%s: set debug_level_ctrl[%d] = 0x%x\n", __func__, dbg_module_idx, dbg_lvl);

	return TRUE;
}


UINT16 Checksum16(UINT8 *pData, int len)
{
	int sum = 0;

	while (len > 1) {
		sum += *((UINT16 *)pData);

		pData = pData + 2;

		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}

	if (len)
		sum += *((UINT8 *)pData);

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return ~sum;
}

INT32 set_fwlog_serverip(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	UINT32 ip_addr;
	UCHAR ip_buf[128];

	strlcpy(ip_buf, arg, sizeof(ip_buf));
	ip_buf[sizeof(ip_buf) - 1] = '\0';
	MTWF_PRINT("ip=[%s]\n", ip_buf);

	if (rtinet_aton(ip_buf, &ip_addr)) {
		ad->physical_dev->fw_log_ctrl.fw_log_server_ip = ip_addr;
		MTWF_PRINT("fw_log_server_ip=%s(%x)\n", arg, ad->physical_dev->fw_log_ctrl.fw_log_server_ip);
	}
	return TRUE;
}

INT32 set_fwlog_servermac(struct _RTMP_ADAPTER *ad, RTMP_STRING *arg)
{
	INT	i, mac_len;
	UCHAR mac_buf[20];

	strlcpy(mac_buf, arg, sizeof(mac_buf));
	mac_buf[sizeof(mac_buf) - 1] = '\0';
	mac_len = strlen(arg);

	if (mac_len != 17) {
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"invalid length (%d)\n", mac_len);
		return 0;
	}

	if (strcmp(arg, "00:00:00:00:00:00") == 0) {
		MTWF_DBG(ad, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR, "invalid mac setting\n");
		return 0;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		AtoH(arg, &ad->physical_dev->fw_log_ctrl.fw_log_server_mac[i], 1);
		arg = arg + 3;
	}
	MTWF_PRINT("mac=[%s]\n", mac_buf);
	return 0;
}

NTSTATUS fw_log_open_close_file(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UINT32 LogType;
	INT8 Ret;

	NdisMoveMemory(&LogType, CMDQelmt->buffer, sizeof(UINT32));
	if (LogType == 0) {
		if (pAd->srcf.Status) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Close file \"%s\" failed!\n",
				pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir);
			return NDIS_STATUS_FAILURE;
		}

		Ret = os_file_close(pAd->srcf);
		if (Ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"File Close Error ! Ret = %d\n", Ret);
			return NDIS_STATUS_FAILURE;
		} else {
			pAd->srcf.Status = 1;
		}
	} else if ((LogType == 8) || (LogType == 9) || (LogType == 40) || (LogType == 41)) {
		os_file_open(pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir, (RTMP_OS_FD_EXT *) &(pAd->srcf), O_WRONLY|O_CREAT|O_APPEND, 0);
		if (pAd->srcf.Status) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Open file \"%s\" failed!\n",
				pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir);
			return NDIS_STATUS_FAILURE;
		}
	}
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS fw_log_write_file(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	INT8 Ret;

	if (pAd->srcf.Status) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Write file \"%s\" failed!\n",
			pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir);
		return NDIS_STATUS_FAILURE;
	}

	Ret = os_file_write(pAd->srcf, (INT8 *)CMDQelmt->buffer, (UINT32)CMDQelmt->bufferlength);
	if (Ret < 0) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Write file failed ! Ret=%d\n", Ret);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

NTSTATUS fw_log_alloc_memory(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UINT32 LogType;
	struct FW_IDX_LOG_CTRL *fw_idx_log_ctrl;
	INT8 Ret;

	fw_idx_log_ctrl = &(pAd->physical_dev->fw_idx_log_ctrl);
	NdisMoveMemory(&LogType, CMDQelmt->buffer, sizeof(UINT32));

	if ((LogType == 96 || LogType == 97)) {
		fw_idx_log_ctrl->round = 0;
		fw_idx_log_ctrl->written_size = 0;
		//prevent duplicated alloc
		if (fw_idx_log_ctrl->dump_buffer == NULL) {
			fw_idx_log_ctrl->alloc_size = 128*1024;
			if (os_alloc_mem(pAd,
				&fw_idx_log_ctrl->dump_buffer,
				fw_idx_log_ctrl->alloc_size) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
						"Allocate memory failed, please check!\n");
				return NDIS_STATUS_FAILURE;
			}
		}
		os_zero_mem(fw_idx_log_ctrl->dump_buffer, fw_idx_log_ctrl->alloc_size);
	} else if (LogType == 0) {
		//open file
		os_file_open(pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir,
			(RTMP_OS_FD_EXT *) &(pAd->srcf), O_WRONLY|O_CREAT|O_APPEND, 0);
		if (pAd->srcf.Status) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Open file \"%s\" failed!\n",
				pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir);
			return NDIS_STATUS_FAILURE;
		}
		if (pAd->srcf.Status) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"Write file \"%s\" failed!\n",
				pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir);
			return NDIS_STATUS_FAILURE;
		}
		//write file
		if (fw_idx_log_ctrl->round == 0) {
			Ret = os_file_write(pAd->srcf, fw_idx_log_ctrl->dump_buffer,
				fw_idx_log_ctrl->written_size);
		} else {
			Ret = os_file_write(pAd->srcf,
				fw_idx_log_ctrl->dump_buffer + fw_idx_log_ctrl->written_size,
				fw_idx_log_ctrl->alloc_size - fw_idx_log_ctrl->written_size);
			Ret = os_file_write(pAd->srcf, fw_idx_log_ctrl->dump_buffer,
				fw_idx_log_ctrl->written_size);
		}
		if (Ret < 0) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"Write file failed ! Ret=%d\n", Ret);
		return NDIS_STATUS_FAILURE;
		}
		//close file
		if (pAd->srcf.Status) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Close file \"%s\" failed!\n",
				pAd->physical_dev->fw_log_ctrl.fw_log_dest_dir);
			return NDIS_STATUS_FAILURE;
		}
		Ret = os_file_close(pAd->srcf);
		if (Ret) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"File Close Error ! Ret = %d\n", Ret);
			return NDIS_STATUS_FAILURE;
		}
		pAd->srcf.Status = 1;
	}
	return NDIS_STATUS_SUCCESS;
}

NTSTATUS fw_log_write_memory(IN PRTMP_ADAPTER pAd, IN PCmdQElmt CMDQelmt)
{
	UINT32 LogType, alloc_size = 0, written_size = 0, event_size = 0;
	struct FW_IDX_LOG_CTRL *fw_idx_log_ctrl;

	fw_idx_log_ctrl = &(pAd->physical_dev->fw_idx_log_ctrl);
	LogType = pAd->physical_dev->fw_log_ctrl.wmcpu_log_type;

	if (LogType == 0) {
	} else if ((LogType == 96) || (LogType == 97)) {
		alloc_size = fw_idx_log_ctrl->alloc_size;
		written_size = fw_idx_log_ctrl->written_size;
		event_size = CMDQelmt->bufferlength;
		//memory buffer is enough
		if (written_size + event_size < alloc_size) {
			os_move_mem(fw_idx_log_ctrl->dump_buffer + written_size, (INT8 *) CMDQelmt
			->buffer, event_size);
			fw_idx_log_ctrl->written_size += event_size;
		} else {
			//memory buffer is not enough
			fw_idx_log_ctrl->round++;
			fw_idx_log_ctrl->written_size = 0;
			os_move_mem(fw_idx_log_ctrl->dump_buffer + written_size, (INT8 *) CMDQelmt
			->buffer, event_size);
			fw_idx_log_ctrl->written_size += event_size;
		}
	}
	return NDIS_STATUS_SUCCESS;
}


VOID fw_log_to_ethernet(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *fw_log,
	IN UINT32 log_len)
{
	UCHAR s_addr[MAC_ADDR_LEN];
	UINT32 source_ip = 0x00000000, dest_ip = 0xFFFFFFFF;
	UCHAR ETH_P_AIR_MONITOR[LENGTH_802_3_TYPE] = {0x08, 0x00};
	struct sk_buff *skb = NULL;
	UINT8 isPadding = 0;
	UINT8 *data, *header;
	UINT8 *ip_header, *ip_checksum;
	UINT8 *udp_header, *udp_checksum, *pseudo_header;
	UINT16 data_len, header_len;
	IP_V4_HDR *ipv4_hdr_ptr;
	UINT16 checksum;
	UINT8 i, band_idx;
	struct wifi_dev *tmpWdev = NULL;
	RTMP_ADAPTER *mac_ad = NULL;
	BOOLEAN is_wdev_active = FALSE;
	PFW_LOG_CTRL FwLogCtrl = &(pAd->physical_dev->fw_log_ctrl);

	if (FwLogCtrl->fw_log_server_ip != 0xFFFFFFFF) {
		dest_ip = FwLogCtrl->fw_log_server_ip;
		source_ip = (dest_ip & 0x00FFFFFF) | 0xFE000000;
	}

	header_len = LENGTH_802_3 + 20 + 8; /* 802.3 + IP + UDP */
	if ((log_len % 2) == 0)
		data_len = log_len;
	else {
		data_len = log_len + 1;
		isPadding = 1;
	}

	/* find up wdev interface for fw log capture */
	for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
		mac_ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
		if (!mac_ad)
			continue;

		/* find up wdev interface for fw log capture */
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			tmpWdev = mac_ad->wdev_list[i];
			if (tmpWdev && tmpWdev->if_up_down_state) {
				is_wdev_active = TRUE;
				break;
			}
		}

		if (is_wdev_active)
			break;
	}

	if (!is_wdev_active) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"no wdev if up, can't capture fw log!\n");
		return;
	}


	DEV_ALLOC_SKB(skb, log_len + header_len + 2);
	if (skb == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
			"failed to allocate sk_buff\n");
		return;
	}

	NdisZeroMemory(skb->head, (log_len + header_len + 2));
	SET_OS_PKT_NETDEV(skb, tmpWdev->if_dev);

	OS_PKT_RESERVE(skb, header_len);

	/* Prepare payload*/
	data = OS_PKT_TAIL_BUF_EXTEND(skb, data_len);
	NdisCopyMemory(data, fw_log, log_len);
	if (isPadding)
		*(data + log_len) = 0;

	/* Prepare UDP header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, 8);
	udp_header = header;
	*(UINT16 *)header = OS_HTONS(54321);           /* source port */
	header += sizeof(UINT16);
	*(UINT16 *)header = OS_HTONS(55688);           /* destination port */
	header += sizeof(UINT16);
	*(UINT16 *)header = OS_HTONS(data_len + 8);     /* Length */
	header += sizeof(UINT16);
	udp_checksum = header;
	*(UINT16 *)header = OS_HTONS(0);               /* UDP Checksum */
	pseudo_header = udp_header - 12;
	header = pseudo_header;
	*(UINT32 *)header = source_ip;              /* Source IP */
	header += sizeof(UINT32);
	*(UINT32 *)header = dest_ip;                /* Destination IP */
	header += sizeof(UINT32);
	*(UINT16 *)header = OS_HTONS(data_len + 8);    /* Length */
	header += sizeof(UINT16);
	*(UINT16 *)header = OS_HTONS(17);              /* Length */
	checksum = Checksum16(pseudo_header, data_len + 8 + 12);
	*(UINT16 *)udp_checksum = checksum;

	/* Prepare IP header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, 20);
	ip_header = header;
	ipv4_hdr_ptr = (IP_V4_HDR *)header;
	ipv4_hdr_ptr->version = 4;
	ipv4_hdr_ptr->ihl = 5;
	ipv4_hdr_ptr->tos = 0;
	ipv4_hdr_ptr->tot_len = OS_HTONS(data_len + 20 + 8);
	ipv4_hdr_ptr->identifier = 0;
	header += sizeof(IP_V4_HDR);
	*(UINT16 *)header = OS_HTONS(0x4000);          /* Fragmentation flags and offset */
	header += sizeof(UINT16);
	*header = 7;                                /* Time to live */
	header++;
	*header = 17;                               /* Protocol UDP */
	header++;
	ip_checksum = header;
	*(UINT16 *)header = OS_HTONS(0);               /* IP Checksum */
	header += sizeof(UINT16);
	*(UINT32 *)header = source_ip;              /* Source IP */
	header += sizeof(UINT32);
	*(UINT32 *)header = dest_ip;                /* Destination IP */
	checksum = Checksum16(ip_header, 20);
	*(UINT16 *)ip_checksum = checksum;

	/* Prepare 802.3 header */
	header = OS_PKT_HEAD_BUF_EXTEND(skb, LENGTH_802_3);
	/* Fake a Source Address for transmission */
#ifdef CONFIG_AP_SUPPORT
	COPY_MAC_ADDR(s_addr, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_addr);
#else
	COPY_MAC_ADDR(s_addr, pAd->StaCfg[0].wdev.if_addr);
#endif /* CONFIG_AP_SUPPORT */
	if (s_addr[1] == 0xff)
		s_addr[1] = 0;
	else
		s_addr[1]++;
	MAKE_802_3_HEADER(header, FwLogCtrl->fw_log_server_mac, s_addr, ETH_P_AIR_MONITOR);

	/* Report to upper layer */
	RtmpOsPktProtocolAssign(skb);
	RtmpOsPktRcvHandle(skb, pAd->tr_ctl.napi);
}


NTSTATUS
dbg_log_wrapper(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 ucPktType,
	IN UINT8 *pucData,
	IN UINT16 u2Length)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
	UINT8 *buffer = NULL;
	UINT16 msg_len = 0;
	PFW_LOG_CTRL FwLogCtrl = &(pAd->physical_dev->fw_log_ctrl);
	UINT16 *serialID = &(FwLogCtrl->fw_log_serialID_count);
	P_FW_BIN_LOG_HDR_T log_hdr;
	PICS_AGG_HEADER prIcsAggHeader;

	if ((FwLogCtrl->wmcpu_log_type &
		(FW_LOG_2_HOST_CTRL_2_HOST_STORAGE | FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)) == 0)
		return NDIS_STATUS_SUCCESS;

	switch (ucPktType) {
	case DBG_LOG_PKT_TYPE_ICS:
		prIcsAggHeader = (PICS_AGG_HEADER)GET_OS_PKT_DATAPTR(pucData);
		msg_len = prIcsAggHeader->rxByteCount + sizeof(FW_BIN_LOG_HDR_T);

		if (os_alloc_mem(pAd, (UCHAR **)&buffer, msg_len) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"Allocate memory failed, please check!\n");
			return NDIS_STATUS_FAILURE;
		}
		log_hdr = (P_FW_BIN_LOG_HDR_T)buffer;

		/* prepare ICS header */
		log_hdr->u4MagicNum = FW_BIN_LOG_MAGIC_NUM;
		log_hdr->u1Version = FW_BIN_LOG_VERSION;
		log_hdr->u1Rsv = FW_BIN_LOG_RSV;
		log_hdr->u2SerialID = (*serialID)++;
		if (chip_dbg->get_lpon_frcr)
			log_hdr->u4Timestamp = chip_dbg->get_lpon_frcr(pAd);
		else
			log_hdr->u4Timestamp = 0;
		log_hdr->u2MsgID = DBG_LOG_PKT_TYPE_ICS;
		log_hdr->u2Length = prIcsAggHeader->rxByteCount;

		/* prepare ICS frame */
		NdisCopyMemory(buffer + sizeof(FW_BIN_LOG_HDR_T), prIcsAggHeader, prIcsAggHeader->rxByteCount);
		break;

	case DBG_LOG_PKT_TYPE_TRIG_FRAME:
		if (FwLogCtrl->debug_level_ctrl[BIN_DBG_LOG_TRIGGER_FRAME] == 0)
			return NDIS_STATUS_SUCCESS;

		msg_len = u2Length + sizeof(FW_BIN_LOG_HDR_T);
		if (os_alloc_mem(pAd, (UCHAR **)&buffer, msg_len) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"Allocate memory failed, please check!\n");
			return NDIS_STATUS_FAILURE;
		}
		log_hdr = (P_FW_BIN_LOG_HDR_T)buffer;

		log_hdr->u4MagicNum = FW_BIN_LOG_MAGIC_NUM;
		log_hdr->u1Version = FW_BIN_LOG_VERSION;
		log_hdr->u1Rsv = FW_BIN_LOG_RSV;
		log_hdr->u2SerialID = (*serialID)++;
		if (chip_dbg->get_lpon_frcr)
			log_hdr->u4Timestamp = chip_dbg->get_lpon_frcr(pAd);
		else
			log_hdr->u4Timestamp = 0;
		log_hdr->u2MsgID = DBG_LOG_PKT_TYPE_TRIG_FRAME;
		log_hdr->u2Length = u2Length;

		os_move_mem(buffer + sizeof(FW_BIN_LOG_HDR_T), pucData, u2Length);

		break;
	}

	if (msg_len) {
		if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_FILE, (VOID *)buffer, msg_len);
		if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
			fw_log_to_ethernet(pAd, buffer, msg_len);

		os_free_mem(buffer);
	}

	return NDIS_STATUS_SUCCESS;
}

NTSTATUS host_log_to_parser(
	IN RTMP_ADAPTER	*pAd,
	IN UINT8 *pucData,
	IN UINT16 u2Length)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);
	UINT16 *serialID = &(pAd->physical_dev->fw_log_ctrl.fw_log_serialID_count);
	UINT8 *buffer = NULL;
	UINT16 msg_len = 0;
	P_FW_BIN_LOG_HDR_T log_hdr;

	msg_len = u2Length + sizeof(FW_BIN_LOG_HDR_T);
	if (os_alloc_mem(pAd, (UCHAR **)&buffer, msg_len) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Allocate memory failed, please check!\n");
		return NDIS_STATUS_FAILURE;
	}
	log_hdr = (P_FW_BIN_LOG_HDR_T)buffer;

	if (chip_dbg->get_lpon_frcr)
		log_hdr->u4Timestamp = chip_dbg->get_lpon_frcr(pAd);
	else
		log_hdr->u4Timestamp = 0;
	/* prepare fwlog header */
	log_hdr->u4MagicNum = FW_BIN_LOG_MAGIC_NUM;
	log_hdr->u2MsgID = DBG_LOG_PKT_TYPE_HOST_TEXT_MSG;
	log_hdr->u2Length = u2Length;
	log_hdr->u1Version = FW_BIN_LOG_VERSION;
	log_hdr->u2SerialID = (*serialID)++;

	/* copy log text */
	memcpy(buffer + sizeof(FW_BIN_LOG_HDR_T), pucData, u2Length);

	if (msg_len)
		fw_log_to_ethernet(pAd, buffer, msg_len);

	os_free_mem(buffer);
	return NDIS_STATUS_SUCCESS;
}

void host_dbg_prt(
	IN RTMP_ADAPTER	*pAd,
	IN const INT8 *pFmt,
	...)
{
#define MAX_BUF 300
	va_list args;
	char strbuf[MAX_BUF];
	int len = 0;

	if (!pAd->physical_dev->host_log_ctrl)
		return;

	va_start(args, pFmt);
	len = vsnprintf(strbuf + len, MAX_BUF, pFmt, args);
	va_end(args);
	if (os_snprintf_error(MAX_BUF, len)) {
		MTWF_PRINT("%s: vsnprintf error\n", __func__);
		return;
	}

	host_log_to_parser(pAd, strbuf, len);
}

void host_dbg_hexdump(
	IN struct _RTMP_ADAPTER *pAd,
	char *str,
	unsigned char *va,
	unsigned int size)
{
#define BUFSIZE 200
	int ret = 0;
	unsigned char *pt = NULL;
	/*max hexdump size 200*/
	char buf[BUFSIZE] = {0};
	unsigned int len = 0;
	int x = 0;
	unsigned char y = 0, seg = 10;

	if (size > BUFSIZE)
		size = BUFSIZE;

	pt = va;
	ret = snprintf(buf + len,
				   sizeof(buf) - len,
				   "[logan] %s: %p, len = %d\n",
				   str,
				   va,
				   size);

	len += ret;
	if (ret < 0)
		return;

	for (x = 0; x < size; x++) {
		if (x % 16 == 0) {
			if (y == seg) {
				host_dbg_prt(pAd, "%s", buf);
				y = 0;
				memset(buf, 0, sizeof(buf));
				len = 0;
			}
			if (y > 0) {
				ret = snprintf(buf + len, sizeof(buf) - len, "\n");
				if (ret < 0)
					return;
				len += ret;
			}
			ret = snprintf(buf + len, sizeof(buf) - len, "\t\t0x%04x : ", x);
			if (ret < 0)
				return;
			len += ret;
			y++;
		}
		ret = snprintf(buf + len,
					   sizeof(buf) - len,
					   "%02x ",
					   ((unsigned char)pt[x]));
		if (ret < 0)
			return;
		len += ret;
	}

	host_dbg_prt(pAd, "%s\n\n", buf);
}



#endif /* FW_LOG_DUMP */

#ifdef DBG
#ifdef DBG_ENHANCE
static char *dbg_lvl_msg[DBG_LVL_MAX+1] = {
	"OFF",
	"ERROR",
	"WARN",
	"NOTICE",
	"INFO",
	"DEBUG",
};

static char *dbg_cat_msg[DBG_CAT_MAX+1] = {
	"MISC",
	"INIT",
	"HW",
	"FW",
	"HIF",
	"FPGA",
	"TEST",
	"RA",
	"AP",
	"CLIENT",
	"TX",
	"RX",
	"CFG",
	"MLME",
	"PROTO",
	"SEC",
	"PS",
	"POWER",
	"COEX",
	"P2P",
	"TOKEN",
	"CMW",
	"BF",
	"CFG80211",
	"MLO",
	"RSV",
	"RSV",
	"RSV",
	"RSV",
	"RSV",
	"RSV",
	"RSV",
};

static BOOLEAN mtwf_dbg_prtCatLvl = TRUE; /* Print debug category and level */
static BOOLEAN mtwf_dbg_prtIntfName = TRUE;/* Print interface name */
static BOOLEAN mtwf_dbg_prtThreadId = FALSE;/* Print current thread ID */
static BOOLEAN mtwf_dbg_prtFuncLine = TRUE;/* function name and line */

void mtwf_dbg_option(
	IN const BOOLEAN prtCatLvl,
	IN const BOOLEAN prtIntfName,
	IN const BOOLEAN prtThreadId,
	IN const BOOLEAN prtFuncLine)
{
	mtwf_dbg_prtCatLvl = prtCatLvl;
	mtwf_dbg_prtIntfName = prtIntfName;
	mtwf_dbg_prtThreadId = prtThreadId;
	mtwf_dbg_prtFuncLine = prtFuncLine;
}

void mtwf_dbg_prt(
	IN RTMP_ADAPTER	*pAd,
	IN const UINT32	dbgCat,
	IN const UINT32	dbgLvl,
	IN const INT8   *pFunc,
	IN const UINT32	line,
	IN const INT8   *pFmt,
	...)
{
	va_list args;
	INT8 strBuf[DBG_PRINT_BUF_SIZE];
	INT32 prefixLen = 0;
	INT32 avblBufLen = DBG_PRINT_BUF_SIZE;
	POS_COOKIE pObj = NULL;
	char *intf = NULL;
	char chip[5];
	INT ifIndex, ifType;
	struct net_device *netDev = NULL;
	struct wifi_dev *wdev = NULL;
	int ret;

	if (pAd) {
		ret = snprintf(chip, sizeof(chip), "%04X", pAd->ChipID);
		if (os_snprintf_error(sizeof(chip), ret)) {
			MTWF_PRINT("%s: snprintf error\n", __func__);
			return;
		}
		pObj = (POS_COOKIE)pAd->OS_Cookie;
	}

	if (pObj) {
		ifIndex = pObj->ioctl_if;
		ifType = pObj->ioctl_if_type;

		if (ifType == INT_MAIN || ifType == INT_MBSSID) {
			if (pAd->hdev_ctrl && VALID_MBSS(pAd, ifIndex))
				wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
		} else if (ifType == INT_APCLI) {
			if ((ifIndex < MAX_MULTI_STA) && (ifIndex >= 0))
				wdev = &pAd->StaCfg[ifIndex].wdev;
		} else if (ifType == INT_WDS) {
#ifdef WDS_SUPPORT
			if ((ifIndex < MAX_WDS_ENTRY) && (ifIndex >= 0))
				wdev = &pAd->WdsTab.WdsEntry[ifIndex].wdev;
#endif/* WDS_SUPPORT */
		} else {
		}

		if (wdev) {
			netDev = (struct net_device *) wdev->if_dev;
			if (netDev)
				intf = netDev->name;
		}
	}

	/**
	* Log message format:
	* <chip>@[C<categore>][L<level>][<thread name>][<interface>],[<function>][<line>]: <log>
	* For example: 7915@C13L1iwprivapcli0,MacTableInsertEntry() 920: XXXX
	*/
	prefixLen = snprintf(strBuf, avblBufLen,
						"%s@", intf?chip:"WiFi");
	if (mtwf_dbg_prtCatLvl && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							"%s.%s", dbg_lvl_msg[dbgLvl], dbg_cat_msg[dbgCat]);
	if (mtwf_dbg_prtThreadId && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							",%s", current->comm);
	if (mtwf_dbg_prtIntfName && intf && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							",%s", intf);
	if (mtwf_dbg_prtFuncLine && ((avblBufLen - prefixLen) > 0))
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							",%s() %d", pFunc, line);
	if ((avblBufLen - prefixLen) > 0)
		prefixLen += snprintf(strBuf + prefixLen, avblBufLen - prefixLen,
							": ");

	if ((avblBufLen - prefixLen) > 0) {
		va_start(args, pFmt);
		ret = vsnprintf(strBuf + prefixLen, avblBufLen - prefixLen, pFmt, args);
		va_end(args);
		if (os_snprintf_error(avblBufLen - prefixLen, ret)) {
			MTWF_PRINT("%s: vsnprintf error\n", __func__);
			return;
		}
	}

	switch (dbgLvl)	{
	case DBG_LVL_OFF:
		MTWF_PRINT_DBG_LVL_OFF("%s", strBuf);
		break;
	case DBG_LVL_ERROR:
		MTWF_PRINT_DBG_LVL_ERROR("%s", strBuf);
		break;
	case DBG_LVL_WARN:
		MTWF_PRINT_DBG_LVL_WARN("%s", strBuf);
		break;
	case DBG_LVL_NOTICE:
		MTWF_PRINT_DBG_LVL_NOTICE("%s", strBuf);
		break;
	case DBG_LVL_INFO:
	default:
		MTWF_PRINT_DBG_LVL_DEBUG("%s", strBuf);
		break;
	}
}
#endif /* DBG_ENHANCE */

/*
    Description:
      Get debug level and debug option by Profile

    Return:
      NDIS_STATUS
*/
NDIS_STATUS get_dbg_setting_by_profile(RTMP_STRING *dbg_level, RTMP_STRING *dbg_option)
{
#define DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE (32)
	RTMP_STRING *buffer = NULL;
	RTMP_STRING tmpbuf[DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE] = {0};
	RTMP_STRING key[DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE] = {0};
	RTMP_OS_FD_EXT srcf;
	INT retval = NDIS_STATUS_SUCCESS;
	INT snprintfret;

	if (!dbg_level || !dbg_option)
		return NDIS_STATUS_FAILURE;

	os_alloc_mem(NULL, (UCHAR **)&buffer, MAX_INI_BUFFER_SIZE);

	if (!buffer)
		return NDIS_STATUS_FAILURE;

	os_zero_mem(buffer, MAX_INI_BUFFER_SIZE);
	os_file_open(L1_PROFILE_PATH, (RTMP_OS_FD_EXT *)&srcf, O_RDONLY, 0);

	if (srcf.Status) {
		os_free_mem(buffer);
		return NDIS_STATUS_FAILURE;
	}

	retval = os_file_read(srcf, buffer, MAX_INI_BUFFER_SIZE - 1);

	if (retval) {

		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
			"Read file \"%s\"(%d) succeed!\n", L1_PROFILE_PATH, retval);
		retval = NDIS_STATUS_SUCCESS;

		/* Per system setting, only search in index 0*/
		snprintfret = snprintf(key, DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE,
			"INDEX0_debug_level");
		if (os_snprintf_error(sizeof(key), snprintfret)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"the key snprintf failed!\n");
			retval = NDIS_STATUS_FAILURE;
			os_free_mem(buffer);
			if (os_file_close(srcf) != 0) {
				retval = NDIS_STATUS_FAILURE;
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
						"Close file \"%s\" failed(errCode=%d)!\n",
						L1_PROFILE_PATH, retval);
				return retval;
			}
			return retval;
		}
		if (RTMPGetKeyParameter(
			key, tmpbuf, DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE, buffer, TRUE)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"debuglevel=%s len=%lu\n", tmpbuf, strlen(tmpbuf));
			strncpy(dbg_level, tmpbuf, strlen(tmpbuf));
		} else
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"debug level setting=%s not found!!\n", key);

		os_zero_mem(key, DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE);
		os_zero_mem(tmpbuf, DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE);
		/* Per system setting, only search in index 0*/
		snprintfret = snprintf(key, DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE,
			"INDEX0_debug_option");
		if (os_snprintf_error(sizeof(key), snprintfret)) {
			retval = NDIS_STATUS_FAILURE;
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"the key snprintf failed!\n");
			os_free_mem(buffer);
			if (os_file_close(srcf) != 0) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
						"Close file \"%s\" failed(errCode=%d)!\n",
						L1_PROFILE_PATH, retval);
				return retval;
			}
			return retval;
		}
		if (RTMPGetKeyParameter(
			key, tmpbuf, DEBUGLEVEL_PROFILE_MAX_PARAM_SIZE, buffer, TRUE)) {
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_INFO,
				"debugoption=%s len=%lu\n", tmpbuf, strlen(tmpbuf));
			strncpy(dbg_option, tmpbuf, strlen(tmpbuf));
		} else
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"debug option setting=%s not found!!\n", key);
	} else {
		retval = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"Read file \"%s\" failed(errCode=%d)!\n", L1_PROFILE_PATH, retval);

	}

	if (os_file_close(srcf) != 0) {
		retval = NDIS_STATUS_FAILURE;
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				 "Close file \"%s\" failed(errCode=%d)!\n",
				 L1_PROFILE_PATH, retval);
	}

	os_free_mem(buffer);

	return retval;
}

#endif /* DBG */
