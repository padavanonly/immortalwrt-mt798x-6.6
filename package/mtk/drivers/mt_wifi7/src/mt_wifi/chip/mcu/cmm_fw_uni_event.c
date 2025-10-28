/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_fw_uni_event.c
*/

#ifdef WIFI_UNIFIED_COMMAND
#include "rt_config.h"
#include "epcs_cmm.h"
#include "dot11be_eht.h"

#define MAX_EVENT_FAIL_LOOP 32

VOID UniCmdResultToRsp(struct cmd_msg *msg)
{
#ifdef ERR_RECOVERY
	/* skip cmd timeout in SER period */
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	UINT32 curr_time = ser_get_time();

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
			 "SER_T0=%u\n", ser_get_t0(pAd));
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
			 "curr_time=%u\n", curr_time);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
			 "curr_time-SER_T0=%d (ms)\n",
			 (curr_time - ser_get_t0(pAd)));
	#define SER_TIMEOUT_ALLOWANCE 5000

	if ((curr_time - ser_get_t0(pAd)) < (SER_TIMEOUT_ALLOWANCE)) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_SER, DBG_LVL_INFO,
				 "Skip cmd timeout in SER period!! (%s)!!\n",
				 RtmpOsGetNetDevName(pAd->net_dev));
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_SER, DBG_LVL_DEBUG,
				 "id=0x%x,eid=0x%x cmd timeout\n",
				 msg->attr.type,
				 msg->attr.ext_type);
		return;
	}
#endif
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
		"id=0x%x,eid=0x%x cmd timeout\n", msg->attr.type, msg->attr.ext_type);
}

VOID UniCmdResultRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	struct UNI_EVENT_CMD_RESULT_T *UniCmdResult = (struct UNI_EVENT_CMD_RESULT_T *)payload;

	UniCmdResult->u2CID = le2cpu16(UniCmdResult->u2CID);
	UniCmdResult->u4Status = le2cpu32(UniCmdResult->u4Status);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"UniCmdResult.ucCID = 0x%x, UniCmdResult.u4Status = 0x%x\n",
			UniCmdResult->u2CID, UniCmdResult->u4Status);

	if (UniCmdResult->u4Status != 0 || (UniCmdResult->u2CID != msg->attr.type)) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"BUG::UniCmdResult.u4Status = 0x%x cid: = 0x%x\n",
			UniCmdResult->u4Status, UniCmdResult->u2CID);
	}
}

VOID UniCmdStaRecRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	struct UNI_EVENT_CMD_RESULT_T *UniCmdResult = (struct UNI_EVENT_CMD_RESULT_T *)payload;
	STA_REC_CFG_T *StaRecCfg = (STA_REC_CFG_T *)msg->attr.rsp.wb_buf_in_calbk;

	UniCmdResult->u2CID = le2cpu16(UniCmdResult->u2CID);
	UniCmdResult->u4Status = le2cpu32(UniCmdResult->u4Status);
	if (StaRecCfg)
		StaRecCfg->u4FwCmdStatus = UniCmdResult->u4Status;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"UniCmdResult.ucCID = 0x%x, UniCmdResult.u4Status = 0x%x\n",
			UniCmdResult->u2CID, UniCmdResult->u4Status);

	if (UniCmdResult->u4Status != 0 || (UniCmdResult->u2CID != msg->attr.type)) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_WARN,
			"BUG::UniCmdResult.u4Status = 0x%x cid: = 0x%x\n",
			UniCmdResult->u4Status, UniCmdResult->u2CID);
	} else {
		if (StaRecCfg) {
			RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
			/* CMD / Event WCID is HW View */
			UINT_16 hw_wcid = StaRecCfg->u2WlanIdx;
			MAC_TABLE_ENTRY *pEntry = entry_get(pAd, hw_wcid);

#ifdef SW_CONNECT_SUPPORT
			if (hc_is_sw_sta_enable(pAd)) {
				/* If H/W BC WCID : map to SW WCID */
				if (VALID_BCAST_ENTRY_WCID_ONLY_HW(pAd, hw_wcid)) {
					uint16_t sw_wcid = hc_get_chip_sw_bcast_wcid_by_hw(pAd->hdev_ctrl, hw_wcid);

					if (VALID_BCAST_ENTRY_WCID(pAd, sw_wcid)) {
						pEntry = entry_get(pAd, sw_wcid);
						if (IS_ENTRY_MCAST(pEntry)) {
							MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
								"!!!OK BC!!! UniCmdResult.ucCID = 0x%x, UniCmdResult.u4Status = 0x%x, hw_wcid=%u, sw_wcid=%u\n",
								UniCmdResult->u2CID, UniCmdResult->u4Status, hw_wcid, sw_wcid);
						} else {
							return;
						}
					}
				} else if (hc_is_sw_wcid(pAd, hw_wcid)) {
					/* If Pure S/W WCID : SKIP, force valid */
					MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
						"!!!SKIP hw_wcid=%u  is SW RAGNE!!! UniCmdResult.ucCID = 0x%x, UniCmdResult.u4Status = 0x%x\n",
						hw_wcid, UniCmdResult->u2CID, UniCmdResult->u4Status);
					return;
				}
			}
#endif /* SW_CONNECT_SUPPORT */

			if (StaRecCfg->u8EnableFeature & STA_REC_BASIC_STA_RECORD_FEATURE) {
				/* DISCONNECT -> disable sta_rec_valid */
				if (StaRecCfg->ConnectionState == STATE_DISCONNECT)
					pEntry->sta_rec_valid = FALSE;
				else {
					/* For BC CONNECT->sta_rec_valid=1 on STA_REC_BASIC */
					if (StaRecCfg->ConnectionType == CONNECTION_INFRA_BC)
						pEntry->sta_rec_valid = TRUE;
				}
			}
			/* For UC CONNECT->sta_rec_valid=1 on STA_REC_RA */
			if (StaRecCfg->u8EnableFeature & STA_REC_RA_FEATURE &&
					StaRecCfg->ConnectionState != STATE_DISCONNECT)	{
				struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, hw_wcid);

				pEntry->sta_rec_valid = TRUE;
				if (IS_VALID_ENTRY(tr_entry))
					ap_delayq_handle(pAd, pEntry->wdev, tr_entry);
			}
			/* For install key */
			if (StaRecCfg->u8EnableFeature & STA_REC_INSTALL_KEY_FEATURE) {
				ASIC_SEC_INFO *asic_sec_info = &StaRecCfg->asic_sec_info;

				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_NOTICE,
					"PeerAddr("MACSTR
					") Wcid=0x%x/0x%x (%d/%d), Cipher=0x%x, BssIndex=0x%x, Operation=%d\n",
					MAC2STR(asic_sec_info->PeerAddr),
					asic_sec_info->Wcid,
					asic_sec_info->Wcid2,
					asic_sec_info->Wcid,
					asic_sec_info->Wcid2,
					asic_sec_info->Cipher,
					asic_sec_info->BssIndex,
					IS_REMOVEKEY_OPERATION(asic_sec_info));
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_NOTICE,
					"KeyIdx=0x%x, igtk_key_idx=0x%x, bigtk_key_idx=0x%x\n",
					asic_sec_info->KeyIdx,
					asic_sec_info->igtk_key_idx,
					asic_sec_info->bigtk_key_idx);
			}
		}
	}
}

static VOID UniEventFwLogFormatProc(struct _RTMP_ADAPTER *pAd, P_UNI_EVENT_FW_LOG_FORMAT_T TlvData, EVENT_RXD *event_rxd)
{
	UCHAR *dev_name = NULL;
	UCHAR empty_name[] = " ";
	UINT16 u2MsgSize = (le2cpu16(TlvData->u2Length) - sizeof(UNI_EVENT_FW_LOG_FORMAT_T));
#ifdef FW_LOG_DUMP
	PFW_LOG_CTRL FwLogCtrl = &(pAd->physical_dev->fw_log_ctrl);
	UINT16 *serialID = &(FwLogCtrl->fw_log_serialID_count);
#endif

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "s2d_index = 0x%x\n",
				event_rxd->fw_rxd_2.field.s2d_index);

	dev_name = PD_GET_DEVICE_NAME(pAd->physical_dev);
	if ((dev_name == NULL) || strlen(dev_name) >= NET_DEV_NAME_MAX_LENGTH)
		dev_name = &empty_name[0];

	if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST) {
#ifdef FW_LOG_DUMP
		P_FW_BIN_LOG_HDR_T log_hdr = (P_FW_BIN_LOG_HDR_T)TlvData->acMsg;

		if (le2cpu32(log_hdr->u4MagicNum) == FW_BIN_LOG_MAGIC_NUM) {
			log_hdr->u2SerialID = (*serialID)++;
			if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
				RTEnqueueInternalCmd(pAd,
					CMDTHRED_FW_LOG_TO_FILE,
					(VOID *)TlvData->acMsg, u2MsgSize);

			if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
				fw_log_to_ethernet(pAd, (UINT8 *)TlvData->acMsg, u2MsgSize);
		} else if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_CONNSYS) {
			if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
				RTEnqueueInternalCmd(pAd,
					CMDTHRED_FW_LOG_TO_FILE,
					(VOID *)TlvData->acMsg, u2MsgSize);

			if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
				fw_log_to_ethernet(pAd, (UINT8 *)TlvData->acMsg, u2MsgSize);
		} else
#endif /* FW_LOG_DUMP */
#ifdef PRE_CAL_TRX_SET1_SUPPORT
			if (pAd->KtoFlashDebug)
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					"(%s): %s", dev_name, TlvData->acMsg);
			else
#endif /*PRE_CAL_TRX_SET1_SUPPORT*/
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					"N9 LOG(%s): %s\n", dev_name, TlvData->acMsg);
	} else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"CR4 LOG(%s): %s\n", dev_name, TlvData->acMsg);
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"unknown MCU LOG(%s): %s", dev_name, TlvData->acMsg);
	}
}

static VOID UniEventFwLogMemoryProc(struct _RTMP_ADAPTER *pAd, P_UNI_EVENT_FW_LOG_FORMAT_T TlvData, EVENT_RXD *event_rxd)
{
	PFW_LOG_CTRL FwLogCtrl = &(pAd->physical_dev->fw_log_ctrl);
	struct FW_IDX_LOG_CTRL *fw_idx_log_ctrl = &(pAd->physical_dev->fw_idx_log_ctrl);
	UINT16 u2MsgSize = (le2cpu16(TlvData->u2Length) - sizeof(UNI_EVENT_FW_LOG_FORMAT_T));

	if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_CONNSYS) {
		if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_STORAGE)
			RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_FILE,
			(VOID *)TlvData->acMsg, u2MsgSize);

		if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_MEMORY) {
			if (fw_idx_log_ctrl->dump_buffer != NULL) {
				RTEnqueueInternalCmd(pAd, CMDTHRED_FW_LOG_TO_MEMORY,
				(VOID *)TlvData->acMsg, u2MsgSize);
			} else {
				MTWF_PRINT("No memory buffer, still receive fw log event!!!\n");
			}
		}
		if (FwLogCtrl->wmcpu_log_type & FW_LOG_2_HOST_CTRL_2_HOST_ETHNET)
			fw_log_to_ethernet(pAd, (UINT8 *)TlvData->acMsg, u2MsgSize);
	}
}


VOID UniEventFwLog2HostHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_FW_LOG2HOST_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_up_mac_adapter(physical_dev);

	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"cannot find up interface\n");
		return;
	}

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			 TAG_ID(tag), TAG_LEN(tag));
		switch (TAG_ID(tag)) {
		case UNI_EVENT_FW_LOG_FORMAT:
			UniEventFwLogFormatProc(pAd, (void *)tag, event_rxd);
			break;
		case UNI_EVENT_FW_LOG_MEMORY:
			UniEventFwLogMemoryProc(pAd, (void *)tag, event_rxd);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef MAX_MSDU_SIZE
#undef MAX_MSDU_SIZE
#endif
#define MAX_MSDU_SIZE 1544
static VOID UniEventStaRecUpdateMaxAmsduLenProc(
	struct _RTMP_ADAPTER *pAd,
	UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN_T *TlvData,
	UNI_EVENT_STAREC_T *sta_rec)
{
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UINT16 wcid = sta_rec->u2WlanIdx;
	UINT16 amsdu_len = TlvData->u2AmsduLen;

	/* this is a temporary workaround to fix no amsdu at HT20 high rate */
	if (amsdu_len == 0)
		amsdu_len = MAX_MSDU_SIZE;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "wlan_idx = %d, amsdu_len = %d\n",
			wcid, amsdu_len);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	mac_entry = entry_get(pAd, wcid);

	if (mac_entry->amsdu_limit_len != 0) {
		mac_entry->amsdu_limit_len_adjust = (mac_entry->amsdu_limit_len < amsdu_len
			? mac_entry->amsdu_limit_len : amsdu_len);
	}
}

VOID UniEventStaRecUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_STAREC_T);
	UINT32 event_fail_loop_cnt = 0;
	UNI_EVENT_STAREC_T *sta_rec = (P_UNI_EVENT_STAREC_T)pData;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_STAREC_UPDATE_MAX_AMSDU_LEN:
			UniEventStaRecUpdateMaxAmsduLenProc(pAd, (void *)tag, sta_rec);
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
				"invalid event tag = %d\n",
				TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventAccessRegBasicProc(
	P_UNI_EVENT_ACCESS_REG_BASIC_T TlvData,
	RTMP_REG_PAIR *RegPair)
{
	if (TlvData && RegPair) {
		RegPair->Register = le2cpu32(TlvData->u4Addr);
		RegPair->Value = le2cpu32(TlvData->u4Value);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"MAC CR: 0x%08x=0x%08x\n", RegPair->Register, RegPair->Value);
	}
}

static VOID UniEventAccessRfRegBasicProc(
	P_UNI_EVENT_ACCESS_RF_REG_BASIC_T TlvData,
	MT_RF_REG_PAIR *RfRegPair)
{
	if (TlvData && RfRegPair) {
		RfRegPair->WiFiStream =  (UINT8)le2cpu16(TlvData->u2WifiStream);
		RfRegPair->Register = le2cpu32(TlvData->u4Addr);
		RfRegPair->Value = le2cpu32(TlvData->u4Value);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"RF CR: Rf-%d 0x%08x=0x%08x\n", RfRegPair->WiFiStream,
			RfRegPair->Register, RfRegPair->Value);
	}
}

VOID UniEventAccessRegHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_ACCESS_REG_T);
	struct _RTMP_REG_PAIR *RegPair = (struct _RTMP_REG_PAIR *)msg->attr.rsp.wb_buf_in_calbk;
	struct _MT_RF_REG_PAIR *RfRegPair = (struct _MT_RF_REG_PAIR *)msg->attr.rsp.wb_buf_in_calbk;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ACCESS_REG_BASIC:
			UniEventAccessRegBasicProc((void *)tag, RegPair);
			RegPair++;
			break;

		case UNI_EVENT_ACCESS_RF_REG_BASIC:
			UniEventAccessRfRegBasicProc((void *)tag, RfRegPair);
			RfRegPair++;
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef AIR_MONITOR
static VOID UniEventSmeshInfoParamProc(
	P_UNI_CMD_SMESH_PARAM_T TlvData,
	struct cmd_msg *msg)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_CMD_SMESH_PARAM_T pSmeshResult = (P_UNI_CMD_SMESH_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		os_move_mem(pSmeshResult, TlvData, sizeof(*pSmeshResult));
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"ucEntryEnable = %d, fgSmeshRxA2 = %d, fgSmeshRxA1 = %d, fgSmeshRxData = %d, fgSmeshRxMgnt = %d, fgSmeshRxCtrl = %d\n",
			TlvData->ucEntryEnable, TlvData->fgSmeshRxA2, TlvData->fgSmeshRxA1,
			TlvData->fgSmeshRxData, TlvData->fgSmeshRxMgnt, TlvData->fgSmeshRxCtrl);
}

VOID UniEventSmeshInfoRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_SMESH_INFO_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_CMD_SMESH_PARAM:
			UniEventSmeshInfoParamProc((void *)tag, msg);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}
#endif /* AIR_MONITOR */

static VOID UniEventIECountDownCSAProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	P_UNI_EVENT_CSA_NOTIFY_T TlvData)
{
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdevEach = NULL;
	UCHAR i = 0;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
	"band=%d, macIdx=%d, csa_cnt=%d\n",
		ucBand, TlvData->ucOwnMacIdx, TlvData->ucChannelSwitchCount);
	wdev = wdev_search_by_band_omac_idx(pAd, ucBand, TlvData->ucOwnMacIdx);
	if (!wdev)
		return;

	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
		(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	wdev->csa_count = TlvData->ucChannelSwitchCount;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdevEach = pAd->wdev_list[i];

		if (wdevEach == NULL)
			continue;

#ifdef ZERO_PKT_LOSS_SUPPORT
		/*switch channel at CSA count = ChannelSwitchTriggerCSACount*/
		if (pAd->Zero_Loss_Enable) {
			if (TlvData->ucChannelSwitchCount == pDot11h->ChannelSwitchTriggerCSACount) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
						 "%s:csa_count:%d reset\n",  wdev->if_dev->name, wdev->csa_count);
				wdevEach->csa_count = 0;
				/*reset CsaIELoc in beacon*/
				wdevEach->bcn_buf.CsaIELocationInBeacon = 0;
			}
		} else
#endif /*ZERO_PKT_LOSS_SUPPORT*/
			wdevEach->csa_count = TlvData->ucChannelSwitchCount;
	}

	if (pAd->CommonCfg.bIEEE80211H
		&& (pDot11h->ChannelMode == CHAN_SWITCHING_MODE)) {
#ifdef CONFIG_AP_SUPPORT
		pDot11h->CSCount = pDot11h->CSPeriod;
		ChannelSwitchingCountDownProc(pAd, wdev);
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
			"CSCount=%d\n", pDot11h->CSCount);
#endif /*CONFIG_AP_SUPPORT*/
	}
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");
}

#ifdef DOT11_HE_AX
static VOID UniEventIECountDownBCCProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	P_UNI_EVENT_BCC_NOTIFY_T TlvData)
{
	struct wifi_dev *wdev;

	wdev = wdev_search_by_band_omac_idx(pAd, ucBand, TlvData->ucOwnMacIdx);
	if (!wdev)
		return;

	bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_HEOP, (UINT32)BCN_BPCC_HEOP_BSS_COLOR);
}
#endif /* DOT11_HE_AX */

static VOID UniEventIECountDownPPCSAProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBand,
	P_UNI_EVENT_CSA_NOTIFY_T TlvData)
{
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;
	BOOLEAN Canceled;

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
	"band=%d, macIdx=%d, csa_cnt=%d\n",
		ucBand, TlvData->ucOwnMacIdx, TlvData->ucChannelSwitchCount);
	wdev = wdev_search_by_band_omac_idx(pAd, ucBand, TlvData->ucOwnMacIdx);
	if (!wdev)
		return;

	if (wlan_config_get_eht_csa_dis_subch_bitmap(wdev) != 0)
		wlan_config_set_eht_dscb_enable(wdev, TRUE);
	else
		wlan_config_set_eht_dscb_enable(wdev, FALSE);
	wlan_config_set_eht_csa_dscb_enable(wdev, FALSE);
	wlan_config_set_eht_dis_subch_bitmap(wdev, wlan_config_get_eht_csa_dis_subch_bitmap(wdev));
	wlan_config_set_eht_csa_dis_subch_bitmap(wdev, 0);
	if (bcn_bpcc_op_lock(pAd, wdev, TRUE, BCN_BPCC_EHTOP) == FALSE) {
		MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_NOTICE,
				"bcn_bpcc_op_lock fail.\n");
		if (in_interrupt())
			bcn_bpcc_ct_switch(pAd, wdev, BCN_BPCC_EHTOP, (UINT32)BCN_REASON(BCN_UPDATE_IE_CHG));
		else
			UpdateBeaconHandler_BPCC(
				pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_EHTOP, TRUE);
	} else
		UpdateBeaconHandler_BPCC(pAd, wdev, BCN_REASON(BCN_UPDATE_IE_CHG), BCN_BPCC_EHTOP, TRUE);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"Receive Countdown event: dscb_enable=%d, csa_dscb_enable=%d, dscb=%x\n",
		wlan_config_get_eht_dscb_enable(wdev),
		wlan_config_get_eht_csa_dscb_enable(wdev),
		wlan_config_get_eht_dis_subch_bitmap(wdev));

	pDot11h = wdev->pDot11_H;
	RTMPCancelTimer(&pDot11h->PPCSAEventTimer, &Canceled);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_DFS, DBG_LVL_NOTICE,
		" PPCSAEventTimer normal end!!\n");
}

VOID UniEventIECountDownHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_IE_COUNTDOWN_T);
	P_UNI_EVENT_IE_COUNTDOWN_T pEventHdr = (P_UNI_EVENT_IE_COUNTDOWN_T)pData;
	struct _RTMP_ADAPTER *pAd = NULL;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "\n");

	TAG_FOR_EACH(tag, tags_len, offset) {
		/* Query corresponding pAd by band index from event */
		pAd = physical_device_get_mac_adapter_by_band(physical_dev, pEventHdr->ucBand);

		if (pAd == NULL) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"Cannot find corresponding pAd(Band%d)\n", pEventHdr->ucBand);
			continue;
		}

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_IE_COUNTDOWN_CSA:
			UniEventIECountDownCSAProc(pAd, pEventHdr->ucBand, (void *)tag);
			break;

#ifdef DOT11_HE_AX
		case UNI_EVENT_IE_COUNTDOWN_BCC:
			UniEventIECountDownBCCProc(pAd, pEventHdr->ucBand, (void *)tag);
			break;
#endif /* DOT11_HE_AX */

		case UNI_EVENT_IE_COUNTDOWN_STATIC_PUNCTURE_CSA:
			UniEventIECountDownPPCSAProc(pAd, pEventHdr->ucBand, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventAssertContentProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 ucBssIndex,
	P_UNI_EVENT_ASSERT_CONTENT_T TlvData,
	EVENT_RXD *event_rxd)
{
	UINT16 u2MsgSize = (le2cpu16(TlvData->u2Length) - sizeof(UNI_EVENT_ASSERT_CONTENT_T));

	if (u2MsgSize > 0) {
		TlvData->aucBuffer[u2MsgSize] = 0;
		MTWF_PRINT("ucBssIndex(%d): %s\n", ucBssIndex, TlvData->aucBuffer);
	}

#ifdef FW_DUMP_SUPPORT
	if (!pAd->fw_dump_buffer) {
		os_alloc_mem(pAd, &pAd->fw_dump_buffer, pAd->fw_dump_max_size);
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;

		if (pAd->fw_dump_buffer) {
			if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_N9");
			else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_CR4");
			else
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "\0");
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					 "cannot alloc mem for FW dump\n");
		}
	}

	if (pAd->fw_dump_buffer) {
		if ((pAd->fw_dump_size + u2MsgSize) <= pAd->fw_dump_max_size) {
			os_move_mem(pAd->fw_dump_buffer + pAd->fw_dump_size, TlvData->aucBuffer, u2MsgSize);
			pAd->fw_dump_size += u2MsgSize;
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					 "FW dump size too big\n");
		}
	}
#endif /* FW_DUMP_SUPPORT */
}

VOID UniEventAssertDumpHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_ASSERT_DUMP_T);
	P_UNI_EVENT_ASSERT_DUMP_T pEventHdr = (P_UNI_EVENT_ASSERT_DUMP_T)pData;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ASSERT_CONTENT_DUMP:
			UniEventAssertContentProc(pAd, pEventHdr->ucBssIndex, (void *)tag, event_rxd);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventBeaconTimeoutInfoProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BssIndex,
	P_UNI_EVENT_BEACON_TIMEOUT_INFO_T TlvData)
{
#ifdef CONFIG_AP_SUPPORT
	struct DOT11_H *pDot11h = NULL;
#endif /* CONFIG_AP_SUPPORT */
	struct wifi_dev *wdev;

	wdev = wdev_search_by_omac_idx(pAd, BssIndex);
	if (!wdev)
		return;

	MTWF_PRINT("FW EVENT (%02x:%02x:%02x:%02x:%02x:%02x), Reason Code: 0x%x\n",
			  PRINT_MAC(wdev->bssid), TlvData->ucReasonCode);

	switch (TlvData->ucReasonCode) {
	case UNI_ENUM_BCN_LOSS_STA:
		MTWF_PRINT("STA: Beacon loss detected!\n");
		break;

	case UNI_ENUM_BCN_LOSS_ADHOC:
		MTWF_PRINT("STA ADHOC: Beacon loss detected!\n");
		break;

	case UNI_ENUM_BCN_NULL_FRAME_THRESHOLD:
		MTWF_PRINT("STA TDD: Null frame life timeout due to threshold detected!\n");
		break;

	case UNI_ENUM_BCN_PERIOD_NOT_ILLIGAL:
		MTWF_PRINT("AP: Beacon interval is illegal detected!\n");
		break;

	case UNI_ENUM_BCN_CONNECTION_FAIL:
		MTWF_PRINT("STA: Connection timeout detected!\n");
		break;

	case UNI_ENUM_BCN_ALLOCAT_NULL_PKT_FAIL_THRESHOLD:
		MTWF_PRINT("STA: Allocate null frame fail over threshold detected!\n");
		break;

	case UNI_ENUM_BCN_UNSPECIF_REASON:
		MTWF_PRINT("STA: Beacon timeout due to unspecified reason detected!\n");
		break;

	case UNI_ENUM_BCN_NULL_FRAME_LIFE_TIMEOUT:
		MTWF_PRINT("STA TDD: Null frame life timeout detected!\n");
		break;

	case UNI_ENUM_BCN_LOSS_AP_DISABLE:
		MTWF_PRINT("AP: Beacon OFF!!!\n");
		break;

	case UNI_ENUM_BCN_LOSS_AP_ERROR:
		MTWF_PRINT("AP: Beacon lost - Error!!! Re-issue BCN_OFFLOAD cmd!\n");
#ifdef CONFIG_AP_SUPPORT
		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			break;

		/* do BCN_UPDATE_ALL_AP_RENEW when all BSS CSA done */
		if (pDot11h->csa_ap_bitmap == 0)
			UpdateBeaconHandler(pAd, wdev, BCN_REASON(BCN_UPDATE_ALL_AP_RENEW));
		else
			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_INFO,
					" CSA is running, wait it done, ChannelMode=%d, csa_ap_bitmap=0x%x\n",
					pDot11h->ChannelMode, pDot11h->csa_ap_bitmap);
#endif /* CONFIG_AP_SUPPORT */
		break;
	}
}

VOID UniEventBeaconTimeoutHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_BEACON_TIMEOUT_T);
	P_UNI_EVENT_BEACON_TIMEOUT_T pEventHdr = (P_UNI_EVENT_BEACON_TIMEOUT_T)pData;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BEACON_TIMEOUT_INFO:
			UniEventBeaconTimeoutInfoProc(pAd, pEventHdr->ucBssIndex, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_BCN, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventClientPSInfoProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BssIndex,
	P_UNI_EVENT_CLIENT_PS_INFO_T TlvData)
{
	struct qm_ctl *qm_ctl = PD_GET_QM_CTL(pAd->physical_dev);
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	NDIS_PACKET *pkt = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct _STA_TR_ENTRY *tr_entry = NULL;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			 "PsSync Event from FW APPS(BssIndex=%d) WtblIndex=%d PSBit=%d BufferSize=%d\n",
			 BssIndex, TlvData->ucWtblIndex, TlvData->ucPsBit, TlvData->ucBufferSize);

	if (VALID_UCAST_ENTRY_WCID(pAd, TlvData->ucWtblIndex)) {
		pEntry = entry_get(pAd, TlvData->ucWtblIndex);
		if (IS_WCID_VALID(pAd, TlvData->ucWtblIndex)) {
			tr_entry = tr_entry_get(pAd, TlvData->ucWtblIndex);
#ifdef SW_CONNECT_SUPPORT
			if (IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry))
				return;
#endif /* SW_CONNECT_SUPPORT */
		}

		if (IS_VALID_ENTRY(pEntry))
			RtmpPsIndicate(pAd, pEntry->Addr, pEntry->wcid,
							(TlvData->ucPsBit == 1) ? PWR_SAVE : PWR_ACTIVE);
	}

	if (tr_entry != NULL) {
		OS_SEM_LOCK(&tr_entry->ps_sync_lock);
		tr_entry->ps_state = (TlvData->ucPsBit == 1) ? PWR_SAVE : PWR_ACTIVE;

		if (tr_entry->ps_state == PWR_ACTIVE) {
			do {
				if (qm_ops->get_psq_pkt)
					pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

				if (pkt) {
					UCHAR q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
					UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
					struct wifi_dev *wdev = NULL;

					wdev = pAd->wdev_list[wdev_idx];

					qm_ctl->total_psq_cnt--;
					qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
				}
			} while (pkt);
		}
		OS_SEM_UNLOCK(&tr_entry->ps_sync_lock);
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
			"wtbl index(%d) is invalid!\n", TlvData->ucWtblIndex);
	}
}

static VOID UniEventMultiClientPSInfoProc(
	struct _RTMP_ADAPTER *pAd,
	UINT8 BssIndex,
	struct UNI_EVENT_MULTI_PS_EVENT_T *TlvData)
{
	struct qm_ctl *qm_ctl = PD_GET_QM_CTL(pAd->physical_dev);
	struct qm_ops *qm_ops = PD_GET_QM_OPS(pAd->physical_dev);
	NDIS_PACKET *pkt = NULL;
	struct _STA_TR_ENTRY *tr_entry = NULL;
	UINT16 i = 0, u2StaCnt = 0, u2StaPsInfo = 0, PsBit = 0, BufSize = 0, WtblIdx = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR q_idx = 0, wdev_idx = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!TlvData) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "TlvData is NULL!\n");
		return;
	}
	u2StaCnt = le2cpu32(TlvData->u2StaCnt);

	for (i = 0; i < u2StaCnt; i++) {
		u2StaPsInfo = le2cpu32(TlvData->u2StaPsInfo[i]);
		PsBit = GET_MULTI_PS_PSBIT(u2StaPsInfo);
		BufSize = GET_MULTI_PS_BUF_SIZE(u2StaPsInfo);
		WtblIdx = GET_MULTI_PS_WTBL_INDEX(u2StaPsInfo);
		tr_entry = NULL;

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"PsSync Event from FW APPS(BssIndex=%d) WtblIndex=%dBufferSize=%d\n",
			BssIndex, WtblIdx, BufSize);

		if (VALID_UCAST_ENTRY_WCID(pAd, WtblIdx)) {
			pEntry = entry_get(pAd, WtblIdx);
			if (IS_WCID_VALID(pAd, WtblIdx)) {
				tr_entry = tr_entry_get(pAd, WtblIdx);
#ifdef SW_CONNECT_SUPPORT
				if (IS_SW_MAIN_STA(tr_entry) || IS_SW_STA(tr_entry))
					continue;
#endif /* SW_CONNECT_SUPPORT */
			}
			if (IS_VALID_ENTRY(pEntry))
				RtmpPsIndicate(pAd, pEntry->Addr, pEntry->wcid,
								(PsBit == 1) ? PWR_SAVE : PWR_ACTIVE);
		}

		if (tr_entry != NULL) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
				"PSBit=%d\n", PsBit);
			tr_entry = tr_entry_get(pAd, WtblIdx);
			OS_SEM_LOCK(&tr_entry->ps_sync_lock);
			tr_entry->ps_state = (PsBit == 1) ? PWR_SAVE : PWR_ACTIVE;
			if (tr_entry->ps_state == PWR_ACTIVE) {
				do {
					if (qm_ops->get_psq_pkt)
						pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

					if (pkt) {
						q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
						wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
						wdev = pAd->wdev_list[wdev_idx];

						qm_ctl->total_psq_cnt--;
						qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
					}
				} while (pkt);
			}
			OS_SEM_UNLOCK(&tr_entry->ps_sync_lock);
		} else {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
				"i = %d, wtbl index(%d) is invalid!(PsBit=%d)\n",
				i, WtblIdx, PsBit);
		}
	}
}

VOID UniEventPSSyncHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_PS_SYNC_T);
	P_UNI_EVENT_PS_SYNC_T pEventHdr = (P_UNI_EVENT_PS_SYNC_T)pData;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_CLIENT_PS_INFO:
			UniEventClientPSInfoProc(pAd, pEventHdr->ucBssIndex, (void *)tag);
			break;

		case UNI_EVENT_MULTI_CLIENT_PS_INFO:
			UniEventMultiClientPSInfoProc(pAd, pEventHdr->ucBssIndex, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventECCCalGroupPointResultProc(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_EVENT_ECC_CAL_RES_T TlvData)
{
	MTWF_PRINT("ucEccCmdId = %d, ucIsResFail= %d\n", TlvData->ucEccCmdId, TlvData->ucIsResFail);

	hex_dump_with_lvl("Dqx", TlvData->aucDqxBuffer, TlvData->ucDqxDataLength, DBG_LVL_OFF);
	hex_dump_with_lvl("Dqy", TlvData->aucDqyBuffer, TlvData->ucDqyDataLength, DBG_LVL_OFF);
}

VOID UniEventECCCalHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_ECC_CAL_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_SEC, CATSEC_ECC, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ECC_CAL_GROUP_POINT_RESULT:
			UniEventECCCalGroupPointResultProc(pAd, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_SEC, CATSEC_ECC, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef MT_DFS_SUPPORT
static VOID UniEventRDDReportProc(
	struct _RTMP_ADAPTER *pAd,
	P_UNI_EVENT_RDD_SEND_PULSE_T TlvData)
{
	UCHAR rddidx = HW_RDD0;

	if (!TlvData)
		return;

	rddidx = TlvData->rdd_idx;

	/* update dbg pulse info */
	uni_dfs_update_radar_info(TlvData);

	if (pAd->CommonCfg.DfsParameter.is_sw_rdd_log_en == TRUE)
		uni_dfs_dump_radar_sw_pls_info(pAd, TlvData);

	if (pAd->CommonCfg.DfsParameter.is_hw_rdd_log_en == TRUE)
		uni_dfs_dump_radar_hw_pls_info(pAd, TlvData);

	if ((pAd->CommonCfg.DfsParameter.is_radar_emu == TRUE) ||
			(TlvData->lng_pls_detected == TRUE) ||
			(TlvData->cr_pls_detected == TRUE) ||
			(TlvData->stgr_pls_detected == TRUE))
		WrapDfsRddReportHandle(pAd, rddidx);
}

/*
RDD_INBAND_IDX_1 = Bandidx 1, Phyidx 1, rddidx 1
RDD_INBAND_IDX_2 = Bandidx 2, Phyidx 2, rddidx 0
RDD_DEDICATED_IDX = Bandidx 1, Phyidx 0, rddidx 2
*/
VOID UniEventRDDReportHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag = NULL;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_RDD_T);
	struct _RTMP_ADAPTER *pAd = NULL;
	UCHAR rddidx = HW_RDD0;
	UINT8 hw_band_idx = 0;
	UINT8 wdev_idx = 0;
	struct wifi_dev *wdev = NULL;
	P_UNI_EVENT_RDD_SEND_PULSE_T TlvData;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;
	TlvData = (void *)(P_UNI_EVENT_RDD_SEND_PULSE_T)tag;

	rddidx = TlvData->rdd_idx;

	if (rddidx == RDD_INBAND_IDX_1)
		hw_band_idx = BAND1;
	else if (rddidx == RDD_INBAND_IDX_2)
		hw_band_idx = BAND2;
	else if (rddidx == RDD_DEDICATED_IDX) {
#ifdef DFS_SDB_SUPPORT
		/* Check SDB enable flag of band0 pAd */
		hw_band_idx = BAND0;
		pAd = physical_device_get_mac_adapter_by_band(physical_dev, hw_band_idx);
		if (pAd && pAd->CommonCfg.DfsParameter.bDfsSdbEnable == FALSE) {
			for (hw_band_idx = 1; hw_band_idx <= HW_RDD_IDX_NUM; hw_band_idx++) {
				pAd = physical_device_get_mac_adapter_by_band(physical_dev, hw_band_idx);
				if (pAd && pAd->CommonCfg.DfsParameter.ZeroWaitBandidx == hw_band_idx)
					break;
			}
		}
#else
		for (hw_band_idx = 1; hw_band_idx <= HW_RDD_IDX_NUM; hw_band_idx++) {
			pAd = physical_device_get_mac_adapter_by_band(physical_dev, hw_band_idx);
			if (pAd && pAd->CommonCfg.DfsParameter.ZeroWaitBandidx == hw_band_idx)
				break;
		}
#endif /* DFS_SDB_SUPPORT */
	} else { /*For QA tool use only*/
		for (hw_band_idx = 0; hw_band_idx < 3; hw_band_idx++) {
			pAd = physical_device_get_mac_adapter_by_band(physical_dev, hw_band_idx);
			if (pAd != NULL) {
				for (wdev_idx = 0; wdev_idx < WDEV_NUM_MAX; wdev_idx++) {
					wdev = pAd->wdev_list[wdev_idx];
					if (wdev != NULL && wlan_config_get_ch_band(wdev) == CMD_CH_BAND_5G)
						goto Find_5G_pAd;
				}
			}
		}
	}

Find_5G_pAd:

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"[RDM]:  rddidx:%d, hw_band_idx:%d\n", rddidx, hw_band_idx);

	/*Get pAd 5G Band 1 and 5G Band 2 */
	pAd = physical_device_get_mac_adapter_by_band(physical_dev, hw_band_idx);

	if (!pAd || !pData) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"Error!!! pAd or pData is NULL!!!\n");
		return;
	}

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_RDD_SEND_PULSE:
			UniEventRDDReportProc(pAd, (void *)(P_UNI_EVENT_RDD_SEND_PULSE_T)tag);
			break;
#ifdef CONFIG_ATE
		case UNI_EVENT_RDD_TAG_REPORT: {
			struct _ATE_RDD_LOG unit;
			struct UNI_EVENT_RDD_TAG_REPORT_T *log =
				(struct UNI_EVENT_RDD_TAG_REPORT_T *)tag;
			UINT64 *data = (UINT64 *)log->aucBuffer;
			INT8 i = 0;
			UINT32 len = 0;
			UINT32 dbg_len = 0;
			UINT32 *tmp = 0;

			log->u4FuncLength = le2cpu32(log->u4FuncLength);
			log->u4Prefix = le2cpu32(log->u4Prefix);
			log->u4Count = le2cpu32(log->u4Count);

			len = (log->u4FuncLength - sizeof(struct UNI_EVENT_RDD_TAG_REPORT_T)
				+ sizeof(log->u4FuncIndex) + sizeof(log->u4FuncIndex))>>3;

			dbg_len = (log->u4FuncLength - sizeof(struct UNI_EVENT_RDD_TAG_REPORT_T) + sizeof(log->u4FuncIndex) + sizeof(log->u4FuncIndex)) >> 2;
			tmp = (UINT32 *)log->aucBuffer;

			for (i = 0; i < dbg_len; i++)
				MTWF_PRINT("RDD RAW DWORD%d:%08x\n", i, tmp[i]);

			MTWF_PRINT("RDD FuncLen:%u, len:%u, prefix:%08x, cnt:%u,  dbg_len:%u\n",
						  log->u4FuncLength, len, log->u4Prefix, log->u4Count, dbg_len);

			os_zero_mem(&unit, sizeof(unit));
			unit.u4Prefix = log->u4Prefix;
			unit.u4Count = log->u4Count/8;

			for (i = 0; i < len; i++) {
				RTMPMoveMemory(unit.aucBuffer, data++, ATE_RDD_LOG_SIZE);
				MT_ATEInsertRddLog(pAd, (UCHAR *)&unit, fATE_LOG_RDD, sizeof(unit));
				/* byPass is used @ logDump, if the same event, don't dump same message */
				unit.byPass = TRUE;
			}
		}

			break;
#endif
		default:
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef WIFI_MD_COEX_SUPPORT
VOID UniEventIDCHandler(VOID *physical_dev, UINT8 *pData, UINT32 Length, EVENT_RXD *event_rxd)
{
	UINT8 *tag;
	UINT8 band_idx;
	UINT32 tags_len;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_IDC_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_IDC_MD_SAFE_CHN:
			for (band_idx = 0; band_idx < CFG_WIFI_RAM_BAND_NUM; band_idx++) {
				pAd = physical_device_get_mac_adapter_by_band(physical_dev, band_idx);
				if (pAd == NULL) {
					MTWF_DBG(NULL, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_ERROR,
						"Cannot find corresponding pAd(Band%d)\n", band_idx);
					continue;
				}
				EventLteSafeChnHandler(pAd, (void *)tag, TAG_LEN(tag));
			}
			break;
		case UNI_EVENT_IDC_CCCI_MSG:
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_COEX, CATCOEX_MD, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

VOID UniEventIDCRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_IDC_T);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)msg->priv;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_IDC_MD_SAFE_CHN:
			EventLteSafeChnHandler(pAd, (void *)tag, TAG_LEN(tag));
			break;
		case UNI_EVENT_IDC_INFO:
			ExtEventIdcEventHandler(pAd, (void *)tag, TAG_LEN(tag));
			break;
		default:
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}
#endif

#ifdef CONFIG_ATE
INT32 UMT_ATEInsertRDD(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len)
{
	INT ret = 0;
	struct _ATE_RDD_LOG *result = NULL;
	UINT32 *pulse = 0;
	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	os_zero_mem(entry, sizeof(*entry));
	entry->log_type = fATE_LOG_RDD;
	entry->un_dumped = TRUE;

	if (len > sizeof(entry->log.rdd))
		len = sizeof(entry->log.rdd);

	NdisMoveMemory((UCHAR *)&entry->log.rdd, data, len);

	result = &entry->log.rdd;
  	pulse = (uint32_t *)result->aucBuffer;

  	MTWF_PRINT("[RDD]0x%08x %08x\n", pulse[0], pulse[1]);

	return ret;
err0:
	MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"NULL entry %p, data %p\n",
		entry, data);
	return -1;
}

INT32 MT_ATEInsertRddLog(RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;
	INT idx = 0;
	INT logcb_idx = 0;
	UINT32 is_dumping = 0;
	INT32 (*insert_func)(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len) = NULL;

	switch (log_type) {

	case fATE_LOG_RDD:
		insert_func = UMT_ATEInsertRDD;
		logcb_idx = ATE_LOG_RDD - 1;
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "Unknown log type %08x\n", log_type);
		break;
	}

	if (!insert_func)
		goto err1;

	log_cb = &ATECtrl->log_dump[logcb_idx];

	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			" log_cb->len:%d, log_cb->idx:%d, log_cb->entry:%d\n",
			   log_cb->len, log_cb->idx, logcb_idx);

	idx = log_cb->idx;
	OS_SPIN_LOCK(&log_cb->lock);
	is_dumping = log_cb->is_dumping;
	OS_SPIN_UNLOCK(&log_cb->lock);

	if (is_dumping)
		goto err1;

	if ((log_cb->idx + 1) == log_cb->len) {
		if (!log_cb->overwritable)
			goto err0;
		else
			log_cb->is_overwritten = TRUE;
	}

	OS_SPIN_LOCK(&log_cb->lock);

	/* The result of pointer arithmetic log_cb->entry + idx is never null
	 * The condition !(log_cb->entry + idx) cannot be true
	 * so check if log_cb->entry is null that infer log_cb->entry[idx] has data */
	if (!log_cb->entry) {
		OS_SPIN_UNLOCK(&log_cb->lock);
		goto err0;
	}

	ret = insert_func(&log_cb->entry[idx], log, len);
	OS_SPIN_UNLOCK(&log_cb->lock);

	if (ret)
		goto err0;

	INC_RING_INDEX(log_cb->idx, log_cb->len);
	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"idx:%d, log_cb->idx:%d, log_type:%08x\n",
			  idx, log_cb->idx, log_type);
	return ret;
err0:
	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
		"[WARN]: idx:%x, overwritable:%x, log_type:%08x\n",
		idx, (log_cb) ? log_cb->overwritable:0xff,
		log_type);
err1:
	MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR, "Log dumping\n");
	return -NDIS_STATUS_RESOURCES;
}
#endif
#endif /* MT_DFS_SUPPORT */

static VOID UniEventSerQueryCmmProc(
	P_UNI_EVENT_SER_QUERY_CMM_T TlvData)
{
	UINT32 i;

	MTWF_PRINT("%s: ucEnableSER = %d, ucSerL1RecoverCnt = %d, ucSerL2RecoverCnt = %d\n", __func__,
			TlvData->ucEnableSER, TlvData->ucSerL1RecoverCnt, TlvData->ucSerL2RecoverCnt);

	for (i = 0; i < 32; i++) {
		MTWF_PRINT("%s: i = %d => u2PSEErrorCnt = %d, u2PSEError1Cnt = %d, u2PLEErrorCnt = %d, u2PLEError1Cnt = %d, u2PLEErrorAmsduCnt = %d\n", __func__,
			i, TlvData->u2PSEErrorCnt[i], TlvData->u2PSEError1Cnt[i], TlvData->u2PLEErrorCnt[i],
			TlvData->u2PLEError1Cnt[i], TlvData->u2PLEErrorAmsduCnt[i]);
	}
}

static VOID UniEventSerQueryBandProc(
	P_UNI_EVENT_SER_QUERY_BAND_T TlvData)
{
	UINT32 i;

	MTWF_PRINT("%s: ucBandIdx = %d, ucL3RxAbortCnt = %d, ucL3TxAbortCnt = %d, ucL3TxDisableCnt = %d, ucL4RecoverCnt = %d\n", __func__,
			TlvData->ucBandIdx, TlvData->ucL3RxAbortCnt, TlvData->ucL3TxAbortCnt, TlvData->ucL3TxDisableCnt, TlvData->ucL4RecoverCnt);

	for (i = 0; i < 32; i++) {
		MTWF_PRINT("%s: i = %d => au2LMACError6Cnt = %d, au2LMACError7Cnt = %d\n", __func__,
			i, le2cpu16(TlvData->au2LMACError6Cnt[i]), le2cpu16(TlvData->au2LMACError7Cnt[i]));
	}
}

static VOID UniEventSerQueryWFDMAProc(
	P_UNI_EVENT_SER_QUERY_WFDMA_T TlvData)
{
	UINT32 i;

	for (i = 0; i < 4; i++) {
		MTWF_PRINT("%s: i = %d => au2WfdmaTxBusyCnt = %d, au2WfdmaRxBusyCnt = %d\n", __func__,
			i, le2cpu16(TlvData->au2WfdmaTxBusyCnt[i]), le2cpu16(TlvData->au2WfdmaRxBusyCnt[i]));
	}
}

VOID UniEventSERHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_SER_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SER_QUERY_CMM:
			UniEventSerQueryCmmProc((void *)tag);
			break;

		case UNI_EVENT_SER_QUERY_BAND:
			UniEventSerQueryBandProc((void *)tag);
			break;

		case UNI_EVENT_SER_QUERY_WFDMA:
			UniEventSerQueryWFDMAProc((void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventMacInfoTSFProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_MAC_INFO_TSF_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		TSF_RESULT_T *pTsfResult = (TSF_RESULT_T *)msg->attr.rsp.wb_buf_in_calbk;
		pTsfResult->u4TsfBit0_31 = le2cpu32(TlvData->u4TsfBit0_31);
		pTsfResult->u4TsfBit63_32 = le2cpu32(TlvData->u4TsfBit63_32);
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"ucDbdcIdx = %d, ucHwBssidIndex = %d, u4TsfBit0_31 = %d, u4TsfBit63_32 = %d\n",
			TlvData->ucDbdcIdx, TlvData->ucHwBssidIndex, le2cpu32(TlvData->u4TsfBit0_31), le2cpu32(TlvData->u4TsfBit63_32));
}

static VOID UniEventMacInfoTSFDiffProc(
	struct cmd_msg *msg,
	struct UNI_EVENT_MAC_INFO_TSF_DIFF_T *TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		struct TSF_DIFF_RESULT_T *pTsfResult = (struct TSF_DIFF_RESULT_T *)msg->attr.rsp.wb_buf_in_calbk;

		pTsfResult->u4Tsf0Bit0_31 = le2cpu32(TlvData->u4Tsf0Bit0_31);
		pTsfResult->u4Tsf0Bit63_32 = le2cpu32(TlvData->u4Tsf0Bit63_32);
		pTsfResult->u4Tsf1Bit0_31 = le2cpu32(TlvData->u4Tsf1Bit0_31);
		pTsfResult->u4Tsf1Bit63_32 = le2cpu32(TlvData->u4Tsf1Bit63_32);
	}
}

VOID UniEventMACInfoHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_MAC_IFNO_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MAC_INFO_TSF:
			UniEventMacInfoTSFProc(msg, (void *)tag);
			break;
		case UNI_EVENT_MAC_INFO_TSF_DIFF:
			UniEventMacInfoTSFDiffProc(msg, (void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
VOID UniEventTxCmdDbgHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_TXCMD_CTRL_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		UniEventTxCmdShow((void *)tag);
	}
}
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

static VOID UniEventEDCCAEnableProc(
	P_UNI_EVENT_EDCCA_ENABLE_T TlvData)
{
	MTWF_PRINT("%s: FW Status => fgEDCCAEnable = %d\n",
				__func__, TlvData->fgEDCCAEnable);
}

static VOID UniEventEDCCAThresholdProc(
	P_UNI_EVENT_EDCCA_THRESHOLD_T TlvData)
{
	MTWF_PRINT("%s: fginit = %d, EDCCA Threshold=%d, %d, %d, %d\n",
			__func__, TlvData->fginit, TlvData->u1EDCCAThreshold[0],
			TlvData->u1EDCCAThreshold[1], TlvData->u1EDCCAThreshold[2],
			TlvData->u1EDCCAThreshold[3]);
}

static VOID UniEventACKCTSTimeoutProc(
	struct cmd_msg *msg,
	struct UNI_EVENT_ACK_CTS_TIMEOUT_T *TlvData)
{
	if (msg && msg->attr.rsp.wb_buf_in_calbk) {
		UINT32 *pu4Value = (UINT32 *)msg->attr.rsp.wb_buf_in_calbk;
		*pu4Value = le2cpu32(TlvData->u4TimeoutValue);

		MTWF_PRINT("%s: type = %d, TimeoutValue = %u\n",
				__func__, TlvData->u1Type, le2cpu32(TlvData->u4TimeoutValue));
	}
}

static VOID UniEventDumpStaPauseProc(
	struct cmd_msg *msg,
	struct _UNI_EVENT_DUMP_STA_PAUSE_T *TlvData,
	UINT8 band_idx)
{
	UINT16 x;

	if (msg && TlvData) {
		UINT16 u2PadLen = 0, real_dw_size = 0, ac_dw_size = 0, twt_dw_size = 0, ac_idx = 0;
		RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
		UINT16 u2Length = le2cpu16(TlvData->u2Length);
		UINT16 u2StaPauseTotBytes = le2cpu16(TlvData->u2StaPauseTotBytes);
		UINT16 u2StaAcBytes = le2cpu16(TlvData->u2StaAcBytes);
		UINT16 u2StaTwtMapBytes = le2cpu16(TlvData->u2StaTwtMapBytes);
		UINT16 dw_size = 0;
		UINT32 *pDW = NULL;

		if ((!pAd) || (!pAd->physical_dev) || (!pAd->physical_dev->pause_bitmap.pause) || (!pAd->physical_dev->pause_bitmap.twt_pause))
			return;

		dw_size = DRR_MAX_DW_ALL_AC(pAd) + DRR_MAX_DW_TWT(pAd);

		if ((u2Length > sizeof(struct _UNI_EVENT_DUMP_STA_PAUSE_T))
			&& ((u2Length % 4) == 0)
			&& (band_idx < PD_GET_BAND_NUM((pAd->physical_dev)))) {
			UINT8 band = hc_get_hw_band_idx(pAd);

			if (band != band_idx) {
				MTWF_PRINT("%s: !!!ERROR!!! band=%u, band_idx=%u not equal!!\n", __func__, band, band_idx);
				return;
			}

			u2PadLen = u2Length - u2StaPauseTotBytes - sizeof(struct _UNI_EVENT_DUMP_STA_PAUSE_T);
			real_dw_size = (u2StaPauseTotBytes >> 2);
			ac_dw_size = (u2StaAcBytes >> 2);
			twt_dw_size = (u2StaTwtMapBytes >> 2);
			ac_idx = 0;

			if (dw_size != real_dw_size) {
				MTWF_PRINT("!!!ERROR!!!dw_size=%u, real_dw_size=%u\n",
					dw_size, real_dw_size);
				return;
			}

			if (DRR_MAX_DW_ALL_AC(pAd) != ac_dw_size) {
				MTWF_PRINT("!!!ERROR!!!DRR_MAX_DW_ALL_AC(pAd)=%u, ac_dw_size=%u\n",
					DRR_MAX_DW_ALL_AC(pAd), ac_dw_size);
				return;
			}

			pDW = pAd->physical_dev->pause_bitmap.pause[band];
			if (!pDW)
				return;

			/* 4 ACs */
			for (x = 0; x < ac_dw_size; x++) {
				if (!(x % DRR_MAX_DW_PER_AC(pAd)))
					MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "[EVENT]AC[%d]\n", ac_idx);

				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "DW%02d: %02x %02x %02x %02x\n",
											(x % DRR_MAX_DW_PER_AC(pAd)),
											TlvData->aucStaPauseContent[x * 4 + 3],
											TlvData->aucStaPauseContent[x * 4 + 2],
											TlvData->aucStaPauseContent[x * 4 + 1],
											TlvData->aucStaPauseContent[x * 4]);

				pDW[x] = (UINT32)(TlvData->aucStaPauseContent[x * 4]);

				if ((x % DRR_MAX_DW_PER_AC(pAd)) ==  (DRR_MAX_DW_PER_AC(pAd) - 1))
					ac_idx++;
			}

			pDW = pAd->physical_dev->pause_bitmap.twt_pause[band];
			if (!pDW)
				return;

			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "[EVENT][TWT]\n");
			/* 4 TWT */
			for (x = ac_dw_size; x < real_dw_size; x++) {
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "DW%02d: %02x %02x %02x %02x\n",
											(x % DRR_MAX_DW_PER_AC(pAd)),
											TlvData->aucStaPauseContent[x * 4 + 3],
											TlvData->aucStaPauseContent[x * 4 + 2],
											TlvData->aucStaPauseContent[x * 4 + 1],
											TlvData->aucStaPauseContent[x * 4]);

				pDW[x-ac_dw_size] = (UINT32)(TlvData->aucStaPauseContent[x * 4]);
			}
		}

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"u2Tag = %d, dw_size=%u, u2Length=%u, u2StaPauseTotBytes=%u, u2StaAcBytes=%u, u2StaTwtMapBytes=%u, u2PadLen=%u\n",
			TlvData->u2Tag, dw_size, u2Length, u2StaPauseTotBytes, u2StaAcBytes, u2StaTwtMapBytes, u2PadLen);
	}
}

VOID UniEventBandConfigHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_BAND_CONFIG_T);
	UINT32 event_fail_loop_cnt = 0;
	UINT8 band_idx = 0;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;
	band_idx = ((struct UNI_EVENT_BAND_CONFIG_T *)(payload))->ucBand;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BAND_CONFIG_EDCCA_ENABLE:
			UniEventEDCCAEnableProc((void *)tag);
			break;

		case UNI_EVENT_BAND_CONFIG_EDCCA_THRESHOLD:
			UniEventEDCCAThresholdProc((void *)tag);
			break;

		case UNI_EVENT_BAND_CONFIG_ACK_CTS_TIMEOUT:
			UniEventACKCTSTimeoutProc(msg, (void *)tag);
			break;

		case UNI_EVENT_BAND_CONFIG_DUMP_STA_PAUSE:
			UniEventDumpStaPauseProc(msg, (void *)tag, band_idx);
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_CFG, CATCFG_DBGLOG, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

VOID UniEventMibHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_MIB_T);
	RTMP_MIB_PAIR *pMibResult = (RTMP_MIB_PAIR *)msg->attr.rsp.wb_buf_in_calbk;
	P_UNI_EVENT_MIB_DATA_T pUniEventMibData = NULL;
	UINT32 event_fail_loop_cnt = 0;
	UINT16 total_tag_cnt = 0, hit_tag_cnt = 0, input_tag_cnt = 0;
	UINT32 u4Tmp = 0;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;
	/* use first buffer for sanity input tag */
	input_tag_cnt = pMibResult[0].Counter;

	TAG_FOR_EACH(tag, tags_len, offset) {
		if ((total_tag_cnt >= UNI_CMD_MIB_MAX_PAIR) || (hit_tag_cnt >= UNI_CMD_MIB_MAX_PAIR) || (hit_tag_cnt > input_tag_cnt)) {
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"@@ !!!ERROR!!! total_tag_cnt =%u, hit_tag_cnt=%u, input_tag_cnt=%u, payload_len=%u, fixed_len=%u, event_fail_loop_cnt=%u\n",
				total_tag_cnt, hit_tag_cnt, input_tag_cnt, payload_len, fixed_len, event_fail_loop_cnt);
			break;
		}
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MIB_DATA:
			pUniEventMibData = (P_UNI_EVENT_MIB_DATA_T)tag;
			u4Tmp = pUniEventMibData->u4Counter; /* dbg 1 */
			pMibResult->Counter = le2cpu32(u4Tmp); /* dbg 2 */
			pMibResult->Value = le2cpu64(pUniEventMibData->u8Data);
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				"Tag=0x%x, Counter=%d, Value=%lld\n",
				pUniEventMibData->u2Tag, pMibResult->Counter, pMibResult->Value);
			pMibResult++;
			hit_tag_cnt++;
			total_tag_cnt++;
			break;

		default:
			total_tag_cnt++;
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}

}

#ifdef SMART_CARRIER_SENSE_SUPPORT
static VOID UniEventSCSGetGloAddrProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_GET_SCS_GLO_ADDR TlvData)
{
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
				 "UNI_EVENT_SCS_GET_GLO_ADDR\n");

	UniEventSCSGetGloAddrHandler(msg, (char *)TlvData);
}

VOID UniEventSCSHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_SCS_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SCS_GET_GLO_ADDR:
			UniEventSCSGetGloAddrProc(msg, (void *)tag);
			break;
		default:
			MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

VOID UniEventEepromAccessRead(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct UNI_EVENT_EEPROM_ACCESS_T *pCmdAccessEeprom = (struct UNI_EVENT_EEPROM_ACCESS_T *)Data;
	struct EEPROM_ACCESS_DATA_T *pEepromValue = NULL;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"Address:%x,IsValied:%x\n",
			  le2cpu32(pCmdAccessEeprom->u4Address),
			  le2cpu32(pCmdAccessEeprom->u4Valid));
	pEepromValue = (struct EEPROM_ACCESS_DATA_T *)msg->attr.rsp.wb_buf_in_calbk;
	*pEepromValue->pIsValid = le2cpu32(pCmdAccessEeprom->u4Valid);
	os_move_mem(&pEepromValue->pValue[0],
				&pCmdAccessEeprom->aucData[0], pCmdAccessEeprom->u4DataLen);
}

void UniEventEepromAccessWrite(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len)
{
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_EEPROM_CONTROL);
	struct UNI_EVENT_EEPROM_ACCESS_T *pCmdAccessEeprom;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
			"invalid length = %d\n", payload_len);
		return;
	}

	tag = (UINT8 *)payload + fixed_len;

	pCmdAccessEeprom = (struct UNI_EVENT_EEPROM_ACCESS_T *)tag;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG, "\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"Address:%x,IsValied:%x\n",
			  le2cpu32(pCmdAccessEeprom->u4Address), le2cpu32(pCmdAccessEeprom->u4Valid));
}


void UniEventEepromHandleEvent(
	struct cmd_msg *msg,
	char  *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_EEPROM_CONTROL);

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
			"invalid length = %d\n", payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	switch (*tag) {
	case UNI_EVENT_EEPROM_ACCESS:
		UniEventEepromAccessRead(msg, tag, tags_len);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"invalid event tag = %d\n", *tag);
		break;
	}
}


VOID UniEventEfuseAccessRead(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct UNI_EVENT_EFUSE_ACCESS_T *pCmdAccessEfuse = (struct UNI_EVENT_EFUSE_ACCESS_T *)Data;
	EFUSE_ACCESS_DATA_T *pEfuseValue = NULL;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"Address:%x,IsValied:%x\n",
			  le2cpu32(pCmdAccessEfuse->u4Address),
			  le2cpu32(pCmdAccessEfuse->u4Valid));
	pEfuseValue = (EFUSE_ACCESS_DATA_T *)msg->attr.rsp.wb_buf_in_calbk;
	*pEfuseValue->pIsValid = le2cpu32(pCmdAccessEfuse->u4Valid);
	os_move_mem(&pEfuseValue->pValue[0],
				&pCmdAccessEfuse->aucData[0], EFUSE_BLOCK_SIZE);
}

VOID UniEventFreeBlockCount(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct UNI_EVENT_EFUSE_FREE_BLOCK_T *pCmdEfuseFreeBlockCount = (struct UNI_EVENT_EFUSE_FREE_BLOCK_T *)Data;
	EXT_EVENT_EFUSE_FREE_BLOCK_V1_T *pEfuseFreeCount = NULL;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"ucGetFreeBlock:%d,ucTotalBlockNum:%d\n",
			  (pCmdEfuseFreeBlockCount->ucGetFreeBlock),
			  (pCmdEfuseFreeBlockCount->ucTotalBlockNum));

	pEfuseFreeCount = (EXT_EVENT_EFUSE_FREE_BLOCK_V1_T *)msg->attr.rsp.wb_buf_in_calbk;
	pEfuseFreeCount->ucFreeBlockNum = pCmdEfuseFreeBlockCount->ucGetFreeBlock;
	pEfuseFreeCount->ucTotalBlockNum = pCmdEfuseFreeBlockCount->ucTotalBlockNum;
}

VOID UniEventFreeBlockCountV2(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)msg->priv;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct UNI_EVENT_EFUSE_FREE_BLOCK_T *pCmdEfuseFreeBlockCount = (struct UNI_EVENT_EFUSE_FREE_BLOCK_T *)Data;
	struct _EXT_EVENT_EFUSE_FREE_BLOCK_V2_T *pEfuseFreeCount = NULL;
	int i = 0;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"ucGetFreeBlock:%d,ucTotalBlockNum:%d\n",
			  (pCmdEfuseFreeBlockCount->ucGetFreeBlock),
			  (pCmdEfuseFreeBlockCount->ucTotalBlockNum));

	pEfuseFreeCount = (struct _EXT_EVENT_EFUSE_FREE_BLOCK_V2_T *)msg->attr.rsp.wb_buf_in_calbk;

	for (i = 0; i < cap->efuse_total_bank_num; i++) {
		pEfuseFreeCount[i].ucFreeBlockNum = pCmdEfuseFreeBlockCount[i].ucGetFreeBlock;
		pEfuseFreeCount[i].ucTotalBlockNum = pCmdEfuseFreeBlockCount[i].ucTotalBlockNum;
		pEfuseFreeCount[i].uceFuseNum = pCmdEfuseFreeBlockCount[i].uceFuseNum;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_INFO,
			"efuse bank:%d, Total:%d, Free %d, eFuseNum:%u\n",
				i,
				pEfuseFreeCount[i].ucTotalBlockNum,
				pEfuseFreeCount[i].ucFreeBlockNum,
				pEfuseFreeCount[i].uceFuseNum);
	}
}

VOID UniEventEfuseBufferRDRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	int i = 0;
	struct UNI_EVENT_EFUSE_BUFFER_MODE_READ_T *UniEventResult = (struct UNI_EVENT_EFUSE_BUFFER_MODE_READ_T *)Data;
	CHAR *pEepromData =	(CHAR *)msg->attr.rsp.wb_buf_in_calbk;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"Len = 0x%x\n", Len);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"UniEventResult->ucSourceMode = 0x%x\n",
			  UniEventResult->u1SourceMode);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"UniEventResult->ucContentFormat = 0x%x\n",
			  UniEventResult->u1ContentFormat);
	UniEventResult->u2Offset = le2cpu16(UniEventResult->u2Offset);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"UniEventResult->u2Offset = 0x%x\n",
			  UniEventResult->u2Offset);
	UniEventResult->u2Count = le2cpu32(UniEventResult->u2Count);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"UniEventResult->u4Count = 0x%x\n",
			  UniEventResult->u2Count);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG, "EEProm Content: 0x%p\n\r",
			 &UniEventResult->BinContent[i]);
	for (i = 0; i < UniEventResult->u2Count; i++) {
		if ((i % 32) == 0) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
					"\n\r");
		}
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
				"%02x ", UniEventResult->BinContent[i]);
		*(pEepromData+i) = UniEventResult->BinContent[i];
	}
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"\n\r");
}

void UniEventEfuseAccessWrite(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len)
{
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_EFUSE_CONTROL);
	struct UNI_EVENT_EFUSE_ACCESS_T *pCmdAccessEfuse;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
			"invalid length = %d\n", payload_len);
		return;
	}

	tag = (UINT8 *)payload + fixed_len;

	pCmdAccessEfuse = (struct UNI_EVENT_EFUSE_ACCESS_T *)tag;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG, "\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"Address:%x,IsValied:%x\n",
			  le2cpu32(pCmdAccessEfuse->u4Address), le2cpu32(pCmdAccessEfuse->u4Valid));
}

void UniEventEffuseHandleEvent(
	struct cmd_msg *msg,
	char  *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_EFUSE_CONTROL);

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
			"invalid length = %d\n", payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	switch (*tag) {
	case UNI_EVENT_EFUSE_ACCESS:
		UniEventEfuseAccessRead(msg, tag, tags_len);
		break;

	case UNI_EVENT_EFUSE_FREE_BLOCK:
		UniEventFreeBlockCount(msg, tag, tags_len);
		break;

	case UNI_EVENT_EFUSE_BUFFER_MODE_READ:
		UniEventEfuseBufferRDRsp(msg, tag, tags_len);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"invalid event tag = %d\n", *tag);
		break;
	}
}

void UniEventEfuseHandleEventV2(
	struct cmd_msg *msg,
	char  *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_EFUSE_CONTROL);

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_INFO,
			"invalid length = %d\n", payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	switch (*tag) {
	case UNI_EVENT_EFUSE_FREE_BLOCK:
		/* support multi bank on v2 */
		UniEventFreeBlockCountV2(msg, tag, tags_len);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EFUSE, DBG_LVL_DEBUG,
			"invalid event tag = %d\n", *tag);
		break;
	}
}


#ifdef WIFI_EAP_FEATURE
void UniEventEapHandleEvent(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	char *rsp_payload;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_EAP_CTRL_T);

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EAP, DBG_LVL_INFO,
			"invalid length = %d\n", payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	switch (*tag) {
	case UNI_EVENT_EAP_GET_IPI_VALUE:
		rsp_payload = (char *) &((struct UNI_EVENT_EAP_GET_IPI_VALUE_T *)tag)->u4EapCtrlEventId;
		eapEventDispatcher(msg, rsp_payload, tags_len);
		break;

	case UNI_EVENT_EAP_SHOW_RATE_TABLE:
		rsp_payload = (char *) &((struct UNI_EVENT_EAP_SHOW_RATE_TABLE_T *)tag)->u4EapCtrlEventId;
		eapEventDispatcher(msg, rsp_payload, tags_len);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_EAP, DBG_LVL_DEBUG,
			"invalid event tag = %d\n", *tag);
		break;
	}
}
#endif

VOID UniEventTxPowerUnsolicitHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_TXPOWER);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = (UINT8 *)pData + fixed_len;

	switch (*tag) {
	case UNI_EVENT_TXPOWER_EVENT_UPDATE_EPA_STATUS:
		EventTxPowerEPAInfo(pAd, tag, tags_len);
		break;

	default:
		break;
	}
}

VOID UniEventGetTxPowerRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_ID_GET_TX_POWER_T prEventExtCmdResult = (P_EXT_EVENT_ID_GET_TX_POWER_T)Data;
	P_EXT_EVENT_ID_GET_TX_POWER_T prTxPower = (P_EXT_EVENT_ID_GET_TX_POWER_T)msg->attr.rsp.wb_buf_in_calbk;

	prTxPower->i1TargetPower = prEventExtCmdResult->i1TargetPower;
	prTxPower->u1BandIdx = prEventExtCmdResult->u1BandIdx;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
		"Power: 0x%x Band: %d\n",
		prTxPower->i1TargetPower, prTxPower->u1BandIdx);
}

void UniEventTxPowerHandleEvent(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_TXPOWER) + sizeof(struct _UNI_EVENT_TXPOWER_RSP_T);
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)msg->priv;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	switch (*tag) {
	case UNI_EVENT_TXPOWER_EVENT_SHOW_INFO:
		EventTxPowerShowInfo(pAd, tag, tags_len);
		break;

	case UNI_EVENT_TXPOWER_EVENT_POWER_BACKUP_TABLE_SHOW_INFO:
		EventPowerTableShowInfo(pAd, tag, tags_len);
		break;

	case UNI_EVENT_TXPOWER_EVENT_TARGET_POWER_INFO_GET:
		UniEventGetTxPowerRsp(msg, tag, tags_len);
		break;

	case UNI_EVENT_TXPOWER_EVENT_SHOW_ALL_RATE_TXPOWER_INFO:
		EventTxPowerAllRatePowerShowInfo(pAd, tag, tags_len);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_DEBUG,
			"invalid event tag = %d\n", *tag);
		break;
	}
}

void UniEventRDDGetIpiHist(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT8 i = 0;
	struct UNI_EVENT_RDD_IPI_HIST *prUniEventIpiHist;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_RDD_IPI_HIST_CTRL_T);
	struct UNI_EVENT_RDD_IPI_HIST *pr_evt_rdd_ipi_hist = (struct UNI_EVENT_RDD_IPI_HIST *)msg->attr.rsp.wb_buf_in_calbk;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;
	prUniEventIpiHist = (struct UNI_EVENT_RDD_IPI_HIST *)tag;
	/* Update ipi_hist */
	os_move_mem(pr_evt_rdd_ipi_hist, prUniEventIpiHist, sizeof(struct UNI_EVENT_RDD_IPI_HIST));

	for (i = 0; i < UNI_EVENT_IPI_HIST_TYPE_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"RDD Get IPI histogram idx %d, value %d\n", i, prUniEventIpiHist->u4IpiHistVal[i]);
	}
}

#ifdef IPI_MAINRADIO_SCAN_SUPPORT
void UniEventRDDGetIpiScan(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8  *tag;
	struct _UNI_EVENT_RDD_IPI_SCAN_T *prUniEventIpScan, UniEventIpiScan;
	struct _UNI_EVENT_RDD_IPI_SCAN_CTRL_T UniEventIpiScanCtrl;
	UINT32 fixed_len = sizeof(UniEventIpiScanCtrl);
	struct _UNI_EVENT_RDD_IPI_SCAN_T *pr_evt_rdd_ipi_scan = (struct _UNI_EVENT_RDD_IPI_SCAN_T *)msg->attr.rsp.wb_buf_in_calbk;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;
	prUniEventIpScan = (struct _UNI_EVENT_RDD_IPI_SCAN_T *)tag;
	/* Update ipi_scan */
	os_move_mem(pr_evt_rdd_ipi_scan, prUniEventIpScan, sizeof(UniEventIpiScan));
}

#endif
static VOID UniEventTPCDlTableProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T TlvData)
{
	UINT8 i;
	P_UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T  prTpcDownlinkInfoTbl = TlvData;

	MTWF_PRINT("TPC DOWNLINK INFO TABLE\n\n");
	MTWF_PRINT("AP INFO\n");
	MTWF_PRINT("==========================================================================\n");
	MTWF_PRINT("		DL Tx Type			Cmd Pwr Ctrl		DL Tc Pwr\n");
	MTWF_PRINT("==========================================================================\n");
	MTWF_PRINT("		MU MIMO				%3d					%3d\n",
														prTpcDownlinkInfoTbl->fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_MU_MIMO],
														prTpcDownlinkInfoTbl->i1DlTxPwr[UNI_TPC_DL_TX_TYPE_MU_MIMO]);
	MTWF_PRINT("		OFDMA				%3d					%3d\n\n",
														prTpcDownlinkInfoTbl->fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_MU_OFDMA],
														prTpcDownlinkInfoTbl->i1DlTxPwr[UNI_TPC_DL_TX_TYPE_MU_OFDMA]);

	MTWF_PRINT("STA INFO\n");
	MTWF_PRINT("==========================================================================\n");
	MTWF_PRINT("		WLAN		TxPwrAlpha MU_MIMO		TxPwrAlpha OFDMA\n");
	MTWF_PRINT("==========================================================================\n");
	for (i = 0; i < UNI_TPC_SUPPORT_STA_NUM; i++)
		MTWF_PRINT("		%3d				%3d					%3d\n",
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].u2WlanId,
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_MU_MIMO],
														prTpcDownlinkInfoTbl->rTpcDlManModeParamElem[i].i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_MU_OFDMA]);
}

static VOID UniEventTPCUlTableProc(
	struct cmd_msg *msg,
	P_UNI_EVENT_TPC_INFO_UPLINK_TABLE_T TlvData)
{
	UINT8 i;
	P_UNI_EVENT_TPC_INFO_UPLINK_TABLE_T  prTpcUplinkInfoTbl = TlvData;

	MTWF_PRINT("TPC UPLINK INFO TABLE\n\n");
	MTWF_PRINT("AP INFO: AP TX Power = %d\n", prTpcUplinkInfoTbl->u1ApTxPwr);
	MTWF_PRINT("STA INFO\n");
	MTWF_PRINT("==========================================================================\n");
	MTWF_PRINT("		WLAN		TargetRssi		PwrHeadRoom		MinPwrFlag\n");
	MTWF_PRINT("==========================================================================\n");
	for (i = 0; i < UNI_TPC_SUPPORT_STA_NUM; i++)
		MTWF_PRINT("		%3d			%3d			%3d			%3d\n",
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].u2WlanId,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1TargetRssi,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1PwrHeadRoom,
														prTpcUplinkInfoTbl->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.fgMinPwr);
}

VOID UniEventTPCHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_TPC_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_TPC_DOWNLINK_TABLE:
			UniEventTPCDlTableProc(msg, (void *)tag);
			break;

		case UNI_EVENT_TPC_UPLINK_TABLE:
			UniEventTPCUlTableProc(msg, (void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

void uni_event_rx_addba(
	struct _RTMP_ADAPTER *ad,
	UINT8 *data,
	UINT32 len)
{
	struct UNI_EVENT_RX_ADDBA *prUniEventRxAddBa = (struct UNI_EVENT_RX_ADDBA *) data;
	struct _MAC_TABLE_ENTRY *entry;
	UINT16 rec_ba_wsize;
	UINT16 cfg_rx_ba_wsize;
	BASEQ_CONTROL ba_seq_contrl;
	struct ppdu_caps *ppdu = NULL;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"Event +RxBa staIdx[%d] tid[%d] wsize[%d] baSsnCtl[%d]\n",
		prUniEventRxAddBa->ucStaRecIdx, prUniEventRxAddBa->ucTid,
		prUniEventRxAddBa->u2WinSize, prUniEventRxAddBa->u2BAStartSeqCtrl);

	if (VALID_UCAST_ENTRY_WCID(ad, (UINT16)prUniEventRxAddBa->ucStaRecIdx)) {
		entry = entry_get(ad, prUniEventRxAddBa->ucStaRecIdx);
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"invalid wcid(=%d)\n",
			prUniEventRxAddBa->ucStaRecIdx);
		return;
	}
	if (IS_ENTRY_NONE(entry)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"invalid entry(=%d)\n",
			prUniEventRxAddBa->ucStaRecIdx);
		return;
	}

	cfg_rx_ba_wsize = wlan_config_get_ba_rx_wsize(entry->wdev);
	rec_ba_wsize = cal_rec_ba_wsize(entry, cfg_rx_ba_wsize, prUniEventRxAddBa->u2WinSize);
	ba_seq_contrl.word = prUniEventRxAddBa->u2BAStartSeqCtrl;
	ppdu = wlan_config_get_ppdu_caps(entry->wdev);

	if (!ba_resrc_rec_add(ad, entry->wcid, prUniEventRxAddBa->ucTid,
		prUniEventRxAddBa->u2BATimeoutValue, ba_seq_contrl.field.StartSeq, rec_ba_wsize)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"(Error) ba_resrc_rec_add() failure\n");
	}
}

void uni_event_rx_delba(
	struct _RTMP_ADAPTER *ad,
	UINT8 *data,
	UINT32 len)
{
	struct UNI_EVENT_RX_DELBA *prUniEventRxDelBa = (struct UNI_EVENT_RX_DELBA *) data;
	struct _MAC_TABLE_ENTRY *entry;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"Event -RxBa staIdx[%d] tid[%d]\n",
		prUniEventRxDelBa->ucStaRecIdx, prUniEventRxDelBa->ucTid);

	if (VALID_UCAST_ENTRY_WCID(ad, (UINT16)prUniEventRxDelBa->ucStaRecIdx)) {
		entry = entry_get(ad, prUniEventRxDelBa->ucStaRecIdx);
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"invalid wcid(=%d)\n",
			prUniEventRxDelBa->ucStaRecIdx);
		return;
	}
	if (IS_ENTRY_NONE(entry)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"invalid entry(=%d)\n",
			prUniEventRxDelBa->ucStaRecIdx);
		return;
	}

	if (!ba_resrc_rec_del(ad, entry->wcid, prUniEventRxDelBa->ucTid)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"(Error) ba_resrc_rec_del() failure\n");
	}
}

void uni_event_tx_addba(
	struct _RTMP_ADAPTER *ad,
	UINT8 *data,
	UINT32 len)
{
	struct UNI_EVENT_TX_ADDBA *prUniEventTxAddBa = (struct UNI_EVENT_TX_ADDBA *) data;
	struct _MAC_TABLE_ENTRY *entry;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
		"Event +TxBa staIdx[%d] tid[%d] wsize[%d]\n",
		prUniEventTxAddBa->ucStaRecIdx, prUniEventTxAddBa->ucTid, prUniEventTxAddBa->u2WinSize);

	if (VALID_UCAST_ENTRY_WCID(ad, (UINT16)prUniEventTxAddBa->ucStaRecIdx)) {
		entry = entry_get(ad, prUniEventTxAddBa->ucStaRecIdx);
	} else {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"invalid wcid(=%d)\n",
			prUniEventTxAddBa->ucStaRecIdx);
		return;
	}

	if (IS_ENTRY_NONE(entry)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"invalid entry(=%d)\n",
			prUniEventTxAddBa->ucStaRecIdx);
		return;
	}

	ba_ori_session_start(ad, prUniEventTxAddBa->ucStaRecIdx, prUniEventTxAddBa->ucTid);
}

VOID UniEventBaOffload(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_ID_BA_OFFLOAD);
	UINT32 event_fail_loop_cnt = 0;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		switch (TAG_ID(tag)) {
		case UNI_EVENT_ID_BA_OFFLOAD_TAG_RX_ADDBA:
			if (TAG_LEN(tag) != sizeof(struct UNI_EVENT_RX_ADDBA)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA,
					DBG_LVL_WARN,
					"tag: %d, invalid length = %d",
					TAG_ID(tag), TAG_LEN(tag));
				break;
			}
			uni_event_rx_addba(pAd, tag,
				sizeof(struct UNI_EVENT_RX_ADDBA));
			break;

		case UNI_EVENT_ID_BA_OFFLOAD_TAG_RX_DELBA:
			if (TAG_LEN(tag) != sizeof(struct UNI_EVENT_RX_DELBA)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA,
					DBG_LVL_WARN,
					"tag: %d, invalid length = %d",
					TAG_ID(tag), TAG_LEN(tag));
				break;
			}
			uni_event_rx_delba(pAd, tag,
				sizeof(struct UNI_EVENT_RX_DELBA));
			break;

		case UNI_EVENT_ID_BA_OFFLOAD_TAG_TX_ADDBA:
			if (TAG_LEN(tag) != sizeof(struct UNI_EVENT_TX_ADDBA)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA,
					DBG_LVL_WARN,
					"tag: %d, invalid length = %d",
					TAG_ID(tag), TAG_LEN(tag));
				break;
			}
			uni_event_tx_addba(pAd, tag,
				sizeof(struct UNI_EVENT_TX_ADDBA));
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			break;
		}
	}
}

void uni_event_rro_status(
	struct _RTMP_ADAPTER *ad,
	UINT8 *data,
	UINT32 len)
{
	struct UNI_EVENT_RRO_BA_SESSION_STATUS_T *prUniEventRROStatus = (struct UNI_EVENT_RRO_BA_SESSION_STATUS_T *) data;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ad->physical_dev);
	struct BA_REC_ENTRY *ba_entry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 wcid = prUniEventRROStatus->u2WlanIdx;
	UINT16 se_id = prUniEventRROStatus->u2SessionId;
	UINT8 tid = prUniEventRROStatus->ucTid;
	UINT32 status = prUniEventRROStatus->u4Status;
	UINT16 idx = 0;

	if (status != RRO_STATUS_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_NOTICE,
			"Event RRO status Seid[%d] Wcid[%d] tid[%d] status[%d]\n",
			se_id, wcid, tid, status);

	/* Error case handling */
	if (!VALID_UCAST_ENTRY_WCID(ad, prUniEventRROStatus->u2WlanIdx)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
			"invalid wcid(=%d)\n", wcid);
		return;
	}

	pEntry = entry_get(ad, wcid);
	idx = pEntry->ba_info.RecWcidArray[tid];

#ifdef DOT11_EHT_BE
	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mt_rcu_read_lock();
		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return;
		}
		idx = mld_entry->ba_info.RecWcidArray[tid];
		mt_rcu_read_unlock();
	}
#endif /* DOT11_EHT_BE */

	if (idx == 0) {
		/* recipient BA session had been torn down */
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"wcid(%d) tid(%d) recipient BA session had been torn down\n",
			wcid, tid);
		return;
	}

	ba_entry = &ba_ctl->BARecEntry[idx];

	NdisAcquireSpinLock(&ba_entry->RxReRingLock);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"S1-Event BA_Status[%d] Seid[%u][%u] Wcid[%d] Tid[%d] ACTN[%d] Status[%d]\n",
		ba_entry->REC_BA_Status, ba_entry->Session_id, se_id,
		ba_entry->pEntry->wcid, ba_entry->TID, ba_entry->Postpone_Action,
		status);

	/* do SW RRO if add RRO BA section failed */
	if (status == RRO_STATUS_FAILURE &&
		(ba_entry->REC_BA_Status == Recipient_Offload))
		ba_entry->REC_BA_Status = Recipient_Initialization;
	/* do HW RRO if add RRO BA section successful */
	else if (status == RRO_STATUS_SUCCESS &&
		(ba_entry->REC_BA_Status == Recipient_Initialization ||
		ba_entry->REC_BA_Status == Recipient_Established)) {
		ba_entry->REC_BA_Status = Recipient_Offload;
		ba_entry->Session_id = se_id;
		if (!IS_CIPHER_NONE_OR_WEP_Entry(pEntry))
			hc_set_wed_pn_check(ad, wcid, se_id, TRUE);
	} else if (status == RRO_STATUS_SKIP_ADD && ba_entry->RetryCnt < 3) {
		ba_entry->RetryCnt++;
		RTMP_ADD_BA_SESSION_TO_ASIC(ad, wcid, ba_entry->TID,
				ba_entry->LastIndSeq, ba_entry->BAWinSize, BA_SESSION_RECP, 0);
	}
	ba_entry->WaitWM = FALSE;

	NdisReleaseSpinLock(&ba_entry->RxReRingLock);
}

VOID uni_event_rro_del_chk(
	struct _RTMP_ADAPTER *ad,
	UINT8 *data,
	UINT32 len)
{
	struct UNI_EVENT_RRO_BA_SESSION_DEL_CHECK_DONE_T *prUniEventRRODelChk = (struct UNI_EVENT_RRO_BA_SESSION_DEL_CHECK_DONE_T *) data;
	struct ba_control *ba_ctl = PD_GET_BA_CTRL_PTR(ad->physical_dev);
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct BA_REC_ENTRY *pBAEntry = NULL;
	UINT16 BAidx;
#ifdef DOT11_EHT_BE
	uint16_t mld_sta_idx = 0;
#endif

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO,
		"Event RRO del check down u2Seid[%d] MldId[%d] Tid[%d]\n",
		prUniEventRRODelChk->u2Seid, prUniEventRRODelChk->u2MldId, prUniEventRRODelChk->ucTid);

	if (hc_init_rro_addr_elem_by_seid(ad, prUniEventRRODelChk->u2Seid))
		RTMP_FREE_RRO_SETBL(ad, prUniEventRRODelChk->u2Seid);

	pEntry = entry_get(ad, prUniEventRRODelChk->u2MldId);

	if (!IS_VALID_ENTRY(pEntry)) {
		reclaim_ba_rec_session_by_seid(ad, prUniEventRRODelChk->u2Seid, prUniEventRRODelChk->ucTid);
		return;
	}

	BAidx = pEntry->ba_info.RecWcidArray[prUniEventRRODelChk->ucTid];
#ifdef DOT11_EHT_BE
	if (IS_ENTRY_MLO(pEntry)) {
		struct mld_entry_t *mld_entry;

		mt_rcu_read_lock();
		mld_entry = rcu_dereference(pEntry->mld_entry);
		if (!mld_entry) {
			mt_rcu_read_unlock();
			return;
		}
		BAidx = mld_entry->ba_info.RecWcidArray[prUniEventRRODelChk->ucTid];
		mld_sta_idx = mld_entry->mld_sta_idx;
		mt_rcu_read_unlock();
	}
#endif /* DOT11_EHT_BE */

	if (BAidx == 0) {
		reclaim_ba_rec_session_by_seid(ad, prUniEventRRODelChk->u2Seid, prUniEventRRODelChk->ucTid);
		return;
	}
	pBAEntry = &ba_ctl->BARecEntry[BAidx];

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
		"S4-Event BA REC_BA_Status[%d] Seid[%d] Wcid[%d] Tid[%d] ACTN[%d] SN[%d]\n",
		pBAEntry->REC_BA_Status,  pBAEntry->Session_id,
		pBAEntry->pEntry->wcid, pBAEntry->TID,
		pBAEntry->Postpone_Action, pBAEntry->LastIndSeq);

	switch (pBAEntry->Postpone_Action) {
	case ADDBA_POSTPONE:
		pBAEntry->REC_BA_Status = Recipient_Initialization;
#ifdef DOT11_EHT_BE
		if (IS_ENTRY_MLO(pEntry)) {
			if (pEntry->wdev->wdev_type == WDEV_TYPE_AP)
				bss_mngr_mld_ba_add_to_asic(pEntry->wdev,
					mld_sta_idx, pBAEntry->TID, pBAEntry->LastIndSeq,
					pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
#ifdef CONFIG_STA_SUPPORT
			else if (pEntry->wdev->wdev_type == WDEV_TYPE_STA)
				sta_mld_ba_add_to_asic(pEntry->wdev,
					pEntry, pBAEntry->TID, pBAEntry->LastIndSeq,
					pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
#endif/*CONFIG_STA_SUPPORT*/


		} else
#endif
		{
			RTMP_ADD_BA_SESSION_TO_ASIC(ad, pBAEntry->pEntry->wcid, pBAEntry->TID,
				pBAEntry->LastIndSeq, pBAEntry->BAWinSize, BA_SESSION_RECP, 0);
		}
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		break;

	case DELBA_POSTPONE:
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		ba_free_rec_entry(ad, BAidx);
		break;

	default:
		pBAEntry->WaitWM = FALSE;
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
			"Unexpected event Seid[%d] Wcid[%d] Tid[%d]\n",
			pBAEntry->Session_id, pBAEntry->pEntry->wcid, pBAEntry->TID);
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		break;
	}

	return;
}

VOID UniEventRROHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_RRO_T);
	UINT32 event_fail_loop_cnt = 0;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		switch (TAG_ID(tag)) {
		case UNI_EVENT_RRO_BA_SESSION_STATUS:
			if (TAG_LEN(tag) != sizeof(struct UNI_EVENT_RRO_BA_SESSION_STATUS_T)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
					"tag: %d, invalid length = %d\n",
					TAG_ID(tag), TAG_LEN(tag));
				break;
			}
			uni_event_rro_status(pAd, tag,
				sizeof(struct UNI_EVENT_RRO_BA_SESSION_STATUS_T));
			break;

		case UNI_EVENT_RRO_BA_SESSION_DEL_CHK_DONE:
			if (TAG_LEN(tag) != sizeof(struct UNI_EVENT_RRO_BA_SESSION_DEL_CHECK_DONE_T)) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
					"tag: %d, invalid length = %d\n",
					TAG_ID(tag), TAG_LEN(tag));
				break;
			}
			uni_event_rro_del_chk(pAd, tag,
				sizeof(struct UNI_EVENT_RRO_BA_SESSION_DEL_CHECK_DONE_T));
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
				"[BUG]: invalid event tag = %d, over max loop (%d) fail!\n",
				TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			break;
		}
	}
}

VOID UniEventRxFeCompHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_RXFE_T);
	struct UNI_EVENT_RXFE_T *evt  = (struct UNI_EVENT_RXFE_T *)pData;
	struct UNI_EVENT_PHY_RXFELOSS_T *prRxFeLossComp = (struct UNI_EVENT_PHY_RXFELOSS_T *)evt->au1TlvBuffer;
	UINT8 *tag = (UINT8 *) &prRxFeLossComp->u1BandIdx;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n", Length);
		return;
	}

	tags_len = Length - fixed_len;
	EventRxFeCompHandler(pAd, tag, tags_len);

}

#ifdef MLR_SUPPORT
VOID uni_event_mlr_fsm_update(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *data,
	UINT32 len)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct UNI_EVENT_MLR_FSM_UPDATE_T *prUniEventMlrFsmUpdate = (struct UNI_EVENT_MLR_FSM_UPDATE_T *)data;

	MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_EVENT, DBG_LVL_DEBUG,
		"WlanIdx %d, MlrMode %d, MlrState %d MlrTxdFrIdx %d\n",
		prUniEventMlrFsmUpdate->u2WlanIdx, prUniEventMlrFsmUpdate->ucMlrMode,
		prUniEventMlrFsmUpdate->ucMlrState, prUniEventMlrFsmUpdate->ucMlrTxdFrIdx);

	/* Only support mlr mode v2 */
	if (prUniEventMlrFsmUpdate->ucMlrMode != MLR_MODE_MLR_V2)
		return;

	pEntry = entry_get(pAd, prUniEventMlrFsmUpdate->u2WlanIdx);
	if (pEntry->Sst == SST_ASSOC &&
		pEntry->MlrMode == MLR_MODE_MLR_V2) {

		MTWF_DBG(pAd, DBG_CAT_MLR, CATMLR_EVENT, DBG_LVL_DEBUG,
			"MlrState %d->%d\n",
			pEntry->MlrCurState, prUniEventMlrFsmUpdate->ucMlrState);

		pEntry->MlrCurState = prUniEventMlrFsmUpdate->ucMlrState;
	}


}

VOID UniEventBssErHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_ID_BSS_ER_T);
	UINT32 event_fail_loop_cnt = 0;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_MLR, CATMLR_EVENT, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		switch (TAG_ID(tag)) {
		case UNI_EVENT_BSS_ER_TX_MODE:
			break;

		case UNI_EVENT_MLR_FSM_UPDATE:
			if (TAG_LEN(tag) != sizeof(struct UNI_EVENT_MLR_FSM_UPDATE_T)) {
				MTWF_DBG(NULL, DBG_CAT_MLR, CATMLR_EVENT, DBG_LVL_WARN,
					"tag: %d, invalid length = %d\n",
					TAG_ID(tag), TAG_LEN(tag));
				break;
			}
			uni_event_mlr_fsm_update(pAd, tag,
				sizeof(struct UNI_EVENT_MLR_FSM_UPDATE_T));
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_MLR, CATMLR_EVENT, DBG_LVL_ERROR,
				"[BUG]: invalid event tag = %d, over max loop (%d) fail!\n",
				TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			break;
		}
	}
}
#endif /* MLR_SUPPORT */

void UniEventHwcfgRDRsp(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len)
{
	UINT32 tags_len, i;
	UINT8 *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_HWCFG_T);

	struct UNI_EVENT_HWCFG_READ_T *prEventResult = NULL;
	CHAR *pEepromData = (CHAR *)msg->attr.rsp.wb_buf_in_calbk;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_INFO,
			"invalid length = %d\n", payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	prEventResult = (struct UNI_EVENT_HWCFG_READ_T *)tag;
	prEventResult->u2Offset = le2cpu16(prEventResult->u2Offset);
	prEventResult->u2Count = le2cpu16(prEventResult->u2Count);

	for (i = 0; i < prEventResult->u2Count; i++) {
		if ((i % 32) == 0)
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_DEBUG, "\n\r");

		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_DEBUG,
				 "%02x ", prEventResult->u1BinContent[i]);

		*(pEepromData+i) = prEventResult->u1BinContent[i];
	}

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_HDEV_CTRL, DBG_LVL_DEBUG, "\n\r");

}

VOID UniEventSpectrumGetData(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_SPECTRUM);
	struct UNI_EVENT_SPECTRUM *evt  = (struct UNI_EVENT_SPECTRUM *) pData;
	UINT8  *tag = evt->au1TlvBuffer;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_up_mac_adapter(physical_dev);

	if (pAd == NULL) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
			"cannot find up interface\n");
		return;
	}

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_ID_SPECTRUM_DATA:
			RTEnqueueInternalCmd(pAd, CMDTHRED_WIFISPECTRUM_DUMP_RAW_DATA, (VOID *)tag, tags_len);
			break;

#ifdef PHY_ICS_SUPPORT
		case UNI_EVENT_ID_PHY_ICS_DUMP_DATA:
			RTEnqueueInternalCmd(pAd, CMDTHRED_PHY_ICS_DUMP_RAW_DATA, (VOID *)tag, tags_len);
			break;
#endif

		default:
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}

	switch (*tag) {
	case UNI_EVENT_ID_SPECTRUM_DATA:
		RTEnqueueInternalCmd(pAd, CMDTHRED_WIFISPECTRUM_DUMP_RAW_DATA, (VOID *)tag, tags_len);
		break;
	default:
		break;
	}
}

VOID UniEventSpectrumStatus(struct cmd_msg *msg, char *pData, UINT16 Len)
{
	struct UNI_EVENT_ID_SPECTRUM_STATUS_T *pEventdata = (struct UNI_EVENT_ID_SPECTRUM_STATUS_T *)pData;
	UINT32 *pCapDone = (UINT32 *)msg->attr.rsp.wb_buf_in_calbk;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pCapDone */
	*pCapDone = le2cpu32(pEventdata->u4CapDone);
	MTWF_PRINT("\x1b[42m%s\x1b[m\n", (*pCapDone == TRUE) ?
			"Capture done!!" : "Not yet!!");

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"<-----------------\n");
}

void UniEventSpectrumHandler(
	struct cmd_msg *msg,
	char   *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_SPECTRUM);

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	switch (*tag) {
	case UNI_EVENT_ID_SPECTRUM_STATUS:
		UniEventSpectrumStatus(msg, tag, tags_len);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_PHY, DBG_LVL_DEBUG,
			"invalid event tag = %d\n", *tag);
		break;
	}
}

#ifdef DOT11_HE_AX
VOID muruUniEventDispatcher(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _UNI_EVENT_ID_MURU_T *event = (struct _UNI_EVENT_ID_MURU_T *)Data;
	struct _UNI_EVENT_MURU_TLV_T *tlv = (struct _UNI_EVENT_MURU_TLV_T *) &(event->au1TlvBuffer[0]);
	UINT16 fixed_len = sizeof(*event) + sizeof(*tlv);

	if (Len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_ERROR,
			"invalid length = %d\n",
			Len);
		return;
	}

	MTWF_PRINT("%s: u2Tag = %u, len = %u\n", __func__, tlv->u2Tag, Len);

	switch (tlv->u2Tag) {

	case MURU_EVENT_GET_GROUP_TBL:
		MTWF_PRINT("%s: MURU_EVENT_GET_GROUP_TBL\n", __func__);
		muru_show_group_tbl_entry(msg, (char *)&(tlv->aucBuffer[0]), (Len - fixed_len));
		break;

	case MURU_EVENT_GET_ULTX_CNT:
		MTWF_PRINT("%s: MURU_EVENT_GET_ULTX_CNT\n", __func__);
		muru_get_ul_tx_cnt_handler(msg, (char *)&(tlv->aucBuffer[0]), (Len - fixed_len));
		break;
	case MURU_EVENT_GET_TXC_TX_STATS:
		MTWF_PRINT("%s: MURU_EVENT_GET_TXC_TX_STATS\n", __func__);
		muru_get_txc_tx_stats_handler(msg, (char *)&(tlv->aucBuffer[0]), (Len - fixed_len));
		break;
	case MURU_EVENT_GET_FW_BLACKLIST_CTRL:
		MTWF_PRINT("%s: MURU_EVENT_GET_FW_BLACKLIST_CTRL\n", __func__);
		muru_get_fw_blacklist_ctrl_handler(msg, (char *)&(tlv->aucBuffer[0]), (Len - fixed_len));

	default:
		break;
	}
}

VOID UniEventMuruUnsolicitHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
#if defined(CFG_SUPPORT_FALCON_MURU) && defined(CONFIG_AP_SUPPORT)
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);
	UINT32 tags_len;
	UINT8 *tag, *tlv;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_ID_MURU_T);
	UINT32 tlv_len = sizeof(struct _UNI_EVENT_MURU_TLV_T);

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_ERROR,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = (UINT8 *)pData + fixed_len;
	tlv = (UINT8 *)tag + tlv_len;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_DEBUG,
		"MuruEventTag = %d\n", *tag);

	switch (*tag) {
	case MURU_EVENT_TUNE_AP_MUEDCA:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_INFO,
				 "MURU_EVENT_TUNE_AP_MUEDCA\n");
		muru_tune_ap_muedca_handler(pAd, tlv, (tags_len - tlv_len));
		break;

	case MURU_EVENT_GET_MURU_STATS_MODE_A:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_INFO,
				 "MURU_EVENT_GET_MURU_STATS_MODE_A\n");
		muru_statistic_handler(pAd, tlv, (tags_len - tlv_len));
		break;

	case MURU_EVENT_GET_MUMIMO_STATS_MODE_B:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_INFO,
			 "MURU_EVENT_GET_MUMIMO_STATS_MODE_B\n");
		muru_mimo_stat_handler(pAd, tlv, (tags_len - tlv_len));
		break;

	case MURU_EVENT_GET_DBG_STATS_MODE_C:
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_HE, DBG_LVL_INFO,
			 "MURU_EVENT_GET_DBG_STATS_MODE_C\n");
		muru_dbg_stat_handler(pAd, tlv, (tags_len - tlv_len));
		break;

	default:
		break;
	}

#endif /* defined(CFG_SUPPORT_FALCON_MURU) && defined(CONFIG_AP_SUPPORT) */
}
#endif /* DOT11_HE_AX */

#ifdef TXBF_SUPPORT
static UINT8 get_txbf_cmd_adapt_band_num(void)
{
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *pad = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;

	wdev = txbf_get_last_wdev();
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_WARN,
			"Can not get wdev for original cmd\n");
		return 0;
	}

	pad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	cap = PD_GET_CHIP_CAP_PTR(pad->physical_dev);

	return cap->hw_band_num;
}

static UINT8 get_txbf_cmd_adapt_pfmu_num_max(void)
{
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *pad = NULL;
	struct _RTMP_CHIP_CAP *cap = NULL;

	wdev = txbf_get_last_wdev();
	if (wdev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_WARN,
			"Can not get wdev for original cmd\n");
		return 0;
	}

	pad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	cap = PD_GET_CHIP_CAP_PTR(pad->physical_dev);

	return cap->pfmu_num_max;
}

VOID UniEventBfStaRecHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_BF_STA_REC *prUniEvtBfStaRec = (struct UNI_EVENT_BF_STA_REC *)pBuf;

	StaRecBfRead(pAd, (UINT8 *)(&(prUniEvtBfStaRec->rTxBfPfmuInfo)));
}

VOID UniEventBFCtrlTxSndHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_BF_SND_INFO_BASIC *pSndCtrlEvt = (struct UNI_EVENT_BF_SND_INFO_BASIC *)pBuf;
	struct UNI_EVENT_BF_TXSND_STA_INFO *prSndStaInfo;
	UINT16 Idx;
	UINT16 bf_starec_num = 0;
	UINT16 bf_snd_ctrl_sta_dword_cnt = 0;
	UINT8 band_num = get_txbf_cmd_adapt_band_num();
	UINT32 *pSndRecSuSta = NULL;
	UINT32 *pSndRecVhtMuSta = NULL;
	UINT32 *pSndRecHeTBSta = NULL;
	UINT32 *pSndRecEhtTBSta = NULL;
	UINT16 *pWlanIdxForMcSnd = NULL; /* align 4 */
	UINT16 *pWlanIdxForHeTbSnd = NULL; /* align 4 */
	UINT16 *pWlanIdxForEhtTbSnd = NULL; /* align 4 */
	UINT16 *pULLength = NULL;
	UINT8  *pMcs = NULL;
	UINT8  *pLDPC = NULL;

	if (band_num == 0)
		return;

	bf_starec_num = get_txbf_cmd_adapt_pfmu_num_max()*band_num;
	bf_snd_ctrl_sta_dword_cnt = (bf_starec_num + 31) >> 5;

	MTWF_PRINT("============================= Global Setting ========================================\n");

	MTWF_PRINT("VhtOpt = 0x%02X, HeOpt = 0x%02X, GloOpt = 0x%02X\n",
		pSndCtrlEvt->u1VhtOpt, pSndCtrlEvt->u1HeOpt, pSndCtrlEvt->u1GloOpt);

	pSndRecSuSta = (UINT32 *)(pBuf + sizeof(struct UNI_EVENT_BF_SND_INFO_BASIC));
	for (Idx = 0; Idx < bf_snd_ctrl_sta_dword_cnt; Idx++) {
		MTWF_PRINT("SuSta[%d] = 0x%08X", Idx, pSndRecSuSta[Idx]);
		if ((Idx & 0x03) != 0x03)
			MTWF_PRINT(", ");
		else
			MTWF_PRINT("\n");
	}

	if (Idx == bf_snd_ctrl_sta_dword_cnt)
		MTWF_PRINT("\n");

	pSndRecVhtMuSta = (UINT32 *)(pSndRecSuSta + bf_snd_ctrl_sta_dword_cnt);
	for (Idx = 0; Idx < bf_snd_ctrl_sta_dword_cnt; Idx++) {
		MTWF_PRINT("VhtMuSta[%d] = 0x%08X", Idx, pSndRecVhtMuSta[Idx]);
		if ((Idx & 0x03) != 0x03)
			MTWF_PRINT(", ");
		else
			MTWF_PRINT("\n");
	}

	if (Idx == bf_snd_ctrl_sta_dword_cnt)
		MTWF_PRINT("\n");

	pSndRecHeTBSta = (UINT32 *)(pSndRecVhtMuSta + bf_snd_ctrl_sta_dword_cnt);
	for (Idx = 0; Idx < bf_snd_ctrl_sta_dword_cnt; Idx++) {
		MTWF_PRINT("HeTBSta[%d] = 0x%08X", Idx, pSndRecHeTBSta[Idx]);
		if ((Idx & 0x03) != 0x03)
			MTWF_PRINT(", ");
		else
			MTWF_PRINT("\n");
	}

	if (Idx == bf_snd_ctrl_sta_dword_cnt)
		MTWF_PRINT("\n");

	pSndRecEhtTBSta = (UINT32 *)(pSndRecHeTBSta + bf_snd_ctrl_sta_dword_cnt);
	for (Idx = 0; Idx < bf_snd_ctrl_sta_dword_cnt; Idx++) {
		MTWF_PRINT("EhtTBSta[%d] = 0x%08X", Idx, pSndRecEhtTBSta[Idx]);
		if ((Idx & 0x03) != 0x03)
			MTWF_PRINT(", ");
		else
			MTWF_PRINT("\n");
	}

	if (Idx == bf_snd_ctrl_sta_dword_cnt)
		MTWF_PRINT("\n");

	pWlanIdxForMcSnd = (UINT16 *)(pSndRecEhtTBSta + bf_snd_ctrl_sta_dword_cnt);
	pWlanIdxForHeTbSnd = (UINT16 *)(pWlanIdxForMcSnd + ALIGN_4(band_num));
	pWlanIdxForEhtTbSnd = (UINT16 *)(pWlanIdxForHeTbSnd + ALIGN_4(band_num));
	for (Idx = 0; Idx < band_num; Idx++) {
		MTWF_PRINT("Band%u:\n", Idx);
		MTWF_PRINT("	 Wlan Idx For VHT MC Sounding = %u\n", pWlanIdxForMcSnd[Idx]);
		MTWF_PRINT("	 Wlan Idx For HE TB Sounding = %u\n", pWlanIdxForHeTbSnd[Idx]);
		MTWF_PRINT("	 Wlan Idx For EHT TB Sounding = %u\n", pWlanIdxForEhtTbSnd[Idx]);
	}
	pULLength = (UINT16 *)(pWlanIdxForEhtTbSnd + + ALIGN_4(band_num));
	pMcs = (UINT8 *)(pULLength + 1);
	pLDPC = (UINT8 *)(pMcs + 1);

	MTWF_PRINT("ULLen = %d, ULMcs = %d, ULLDCP = %d\n",
		*pULLength, *pMcs, *pLDPC);

	prSndStaInfo = (struct UNI_EVENT_BF_TXSND_STA_INFO *)(pLDPC + 1);
	MTWF_PRINT("============================= STA Info ========================================\n");

	for (Idx = 1; (Idx < 5 && (Idx < bf_starec_num)); Idx++) {
		MTWF_PRINT("Idx%2u Interval = %d, interval counter = %d, TxCnt = %d, StopReason = 0x%02X\n",
			Idx,
			prSndStaInfo[Idx].u1SndIntv,
			prSndStaInfo[Idx].u1SndIntvCnt,
			prSndStaInfo[Idx].u1SndTxCnt,
			prSndStaInfo[Idx].u1SndStopReason);
	}

	MTWF_PRINT("============================= STA Info Connected ==============================\n");

	for (Idx = 1; (VALID_UCAST_ENTRY_WCID(pAd, Idx) && (Idx < bf_starec_num)); Idx++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, Idx);

		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		MTWF_PRINT("AID%2u Interval = %d (%u ms), interval counter = %d (%u ms), TxCnt = %d, StopReason = 0x%02X\n",
			pEntry->Aid,
			prSndStaInfo[Idx].u1SndIntv,
			prSndStaInfo[Idx].u1SndIntv * 10,
			prSndStaInfo[Idx].u1SndIntvCnt,
			prSndStaInfo[Idx].u1SndIntvCnt * 10,
			prSndStaInfo[Idx].u1SndTxCnt,
			prSndStaInfo[Idx].u1SndStopReason);
	}

	MTWF_PRINT("=====================================================================================\n");

}

VOID UniEventBfPfmuTagReadHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct _RTMP_CHIP_OP *ops = NULL;
	struct UNI_EVENT_BF_PFMU_TAG_READ *prBfPfmuTagRead = (struct UNI_EVENT_BF_PFMU_TAG_READ *)pBuf;
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *pad_curr_wdev = NULL;

	wdev = txbf_get_last_wdev();
	if (wdev == NULL)
		return;
	pad_curr_wdev = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	ops = hc_get_chip_ops(pad_curr_wdev->hdev_ctrl);
	if (ops->dump_pfmu_tag)
		ops->dump_pfmu_tag(pad_curr_wdev,
				prBfPfmuTagRead->fgBFer,
				(UINT8 *)(prBfPfmuTagRead->au4Tag1RawData));
}

VOID UniEventBfPfmuDataReadHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct UNI_EVENT_BF_PFMU_DATA_READ *prBfPfmuDataRead = (struct UNI_EVENT_BF_PFMU_DATA_READ *)pBuf;

	if (ops->dump_pfmu_data)
		ops->dump_pfmu_data(pAd,
				le2cpu16(prBfPfmuDataRead->u2SubCarrIdx),
				(UINT8 *)(prBfPfmuDataRead->au4RawData));
}

#define PHASE_OUT_V3 3
VOID UniEventBfCalPhase(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_IBF_PHASE_INFO *prBfPhaseInfo = (struct UNI_EVENT_IBF_PHASE_INFO *)pBuf;
	struct UNI_EVENT_IBF_PHASE_INFO_V3 *prBfPhaseInfoV3 = (struct UNI_EVENT_IBF_PHASE_INFO_V3 *)pBuf;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _RTMP_ADAPTER *ad = NULL;
#ifdef CONFIG_WLAN_SERVICE
	ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, prBfPhaseInfo->u1DbdcBandIdx);
	TESTMODE_SET_PARAM(ad,
				prBfPhaseInfo->u1DbdcBandIdx,
				iBFCalStatus,
				prBfPhaseInfo->u1Status);
#else
	ad = pAd;
#endif

	if (prBfPhaseInfoV3->u1PhaseOutRptVersion == PHASE_OUT_V3)
		ops->iBFPhaseCalReport(ad,
					prBfPhaseInfoV3->u1Group_L_M_H,
					prBfPhaseInfoV3->u1Group,
					prBfPhaseInfoV3->u1DbdcBandIdx,
					prBfPhaseInfoV3->u1Status,
					prBfPhaseInfoV3->u1PhaseCalType,
					prBfPhaseInfoV3->au1IbfPhaseOut);
	else
		ops->iBFPhaseCalReport(ad,
					prBfPhaseInfo->u1Group_L_M_H,
					prBfPhaseInfo->u1Group,
					prBfPhaseInfo->u1DbdcBandIdx,
					prBfPhaseInfo->u1Status,
					prBfPhaseInfo->u1PhaseCalType,
					prBfPhaseInfo->au1IbfPhaseOut);
}

VOID UniEventBfPlyInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_BF_PLY_INFORM *prUniEvtBfPlyInfo = (struct UNI_EVENT_BF_PLY_INFORM *)pBuf;

	UINT8 u1PlyStaSSGrp;
	UINT8 u1PlyStaSSPly;
	UINT8 u1PlyStaRlt;
	UINT16 Idx, u1SS;
	char cStrGrp[4][4] = {'\0'};
	char cStrPly[4][4] = {'\0'};
	char cStrRlt[4] = {'\0'};

	os_zero_mem(cStrGrp, sizeof(cStrGrp));
	os_zero_mem(cStrPly, sizeof(cStrPly));
	os_zero_mem(cStrRlt, sizeof(cStrRlt));

	MTWF_PRINT("============================= Global Setting ==========================\n");

	MTWF_PRINT("GloOpt = 0x%02X, GrpIBfOpt = 0x%02X, GrpEBfOp = 0x%02X\n",
		prUniEvtBfPlyInfo->u1GloOpt, prUniEvtBfPlyInfo->u1GrpIBfOpt, prUniEvtBfPlyInfo->u1GrpEBfOpt);

	MTWF_PRINT("============================= STA Info ===============================\n");

	for (Idx = 1; Idx < 5; Idx++) {
		u1PlyStaRlt = prUniEvtBfPlyInfo->u1StaRlt[Idx];
		for (u1SS = 0; u1SS < 4; u1SS++) {
			u1PlyStaSSGrp = prUniEvtBfPlyInfo->arStaSS[Idx][u1SS].u1SSGrp;
			u1PlyStaSSPly = prUniEvtBfPlyInfo->arStaSS[Idx][u1SS].u1SSPly;
			TxBFPlyGetGrpStr(u1PlyStaSSGrp, &(cStrGrp[u1SS][0]));
			TxBFPlyGetPlyStr(u1PlyStaSSPly, &(cStrPly[u1SS][0]));
		}

		u1SS = 0;
		TxBFPlyGetPlyStr(u1PlyStaRlt, cStrRlt);
		MTWF_PRINT("AID%2u Nss%2u Grp=%s, Ply=%s Nss%2u Grp=%s, Ply=%s Nss%2u Grp=%s, Ply=%s Nss%2u Grp=%s, Ply=%s Rlt=%s \n",
			Idx, (u1SS+1), &(cStrGrp[u1SS][0]), &(cStrPly[u1SS][0]),
			(u1SS+2), &(cStrGrp[u1SS+1][0]), &(cStrPly[u1SS+1][0]),
			(u1SS+3), &(cStrGrp[u1SS+2][0]), &(cStrPly[u1SS+2][0]),
			(u1SS+4), &(cStrGrp[u1SS+3][0]), &(cStrPly[u1SS+3][0]),
			cStrRlt);
	}

	MTWF_PRINT("=========================== STA Info Connected ======================\n");

	for (Idx = 1; VALID_UCAST_ENTRY_WCID(pAd, Idx); Idx++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, Idx);

		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		u1PlyStaRlt = prUniEvtBfPlyInfo->u1StaRlt[Idx];
		for (u1SS = 0; u1SS < 4; u1SS++) {
			u1PlyStaSSGrp = prUniEvtBfPlyInfo->arStaSS[Idx][u1SS].u1SSGrp;
			u1PlyStaSSPly = prUniEvtBfPlyInfo->arStaSS[Idx][u1SS].u1SSPly;
			TxBFPlyGetGrpStr(u1PlyStaSSGrp, &(cStrGrp[u1SS][0]));
			TxBFPlyGetPlyStr(u1PlyStaSSPly, &(cStrPly[u1SS][0]));
		}

		u1SS = 0;
		TxBFPlyGetPlyStr(u1PlyStaRlt, cStrRlt);
		MTWF_PRINT("AID%2u Nss%2u Grp=%s, Ply=%s Nss%2u Grp=%s, Ply=%s Nss%2u Grp=%s, Ply=%s Nss%2u Grp=%s, Ply=%s Rlt=%s\n",
			pEntry->Aid, (u1SS+1), &(cStrGrp[u1SS][0]), &(cStrPly[u1SS][0]),
			(u1SS+2), &(cStrGrp[u1SS+1][0]), &(cStrPly[u1SS+1][0]),
			(u1SS+3), &(cStrGrp[u1SS+2][0]), &(cStrPly[u1SS+2][0]),
			(u1SS+4), &(cStrGrp[u1SS+3][0]), &(cStrPly[u1SS+3][0]),
			cStrRlt);
	}

	MTWF_PRINT("===================================================================\n");
}

VOID UniEventBFMuMetricInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_BF_METRIC *prMuMetRpt = (struct UNI_EVENT_BF_METRIC *)pBuf;
	struct UNI_EVENT_BF_MU_METRIC_RPT *prQdRpt;

	UINT8 useridx;

	MTWF_PRINT("============================= Global ===============================\n");

	MTWF_PRINT("u1CurState=0x%02X\n", prMuMetRpt->u1CurState);
	MTWF_PRINT("u1RunningFailCnt=0x%02X\n", prMuMetRpt->u1RunningFailCnt);
	MTWF_PRINT("u1ErrRptCnt=0x%02X\n", prMuMetRpt->u1ErrRptCnt);
	MTWF_PRINT("u1FreeReqCnt=0x%02X\n", prMuMetRpt->u1FreeReqCnt);
	MTWF_PRINT("u1PendingReqCnt=0x%02X\n", prMuMetRpt->u1PendingReqCnt);
	MTWF_PRINT("u1PollingTime=0x%02X\n", prMuMetRpt->u1PollingTime);
	MTWF_PRINT("u1NUser=0x%02X\n", prMuMetRpt->u1NUser);
	MTWF_PRINT("fgIsLQErr=0x%02X\n", prMuMetRpt->fgIsLQErr);
	MTWF_PRINT("u2LQErr=0x%02X\n\n", prMuMetRpt->u2LQErr);

	for (useridx = 0; useridx < 4; useridx++) {
		MTWF_PRINT("=========================== User %d ==========================\n", useridx);

		prQdRpt = &prMuMetRpt->arMetricRpt[useridx];
		MTWF_PRINT("BPSK=0x%02X, QPSK=0x%02X, 16QAM=0x%02X, 64QAM=0x%02X\n",
			prQdRpt->u1BPSK, prQdRpt->u1QPSK, prQdRpt->u116QAM, prQdRpt->u164QAM);

		MTWF_PRINT("u1256QAM=0x%02X, u11024QAM=0x%02X, u1Capacity=0x%02X, InitMCS=0x%02X\n\n",
			prQdRpt->u1256QAM, prQdRpt->u11024QAM, prQdRpt->u1Capacity, prMuMetRpt->au1InitMCSUser[useridx]);
	}

}

VOID UniEventBfTxCmdCfgInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_BF_TXCMD_CFG_INFORM *prBfTxCmdCfg = (struct UNI_EVENT_BF_TXCMD_CFG_INFORM *)pBuf;

	UINT8 fgTxCmdBfManual = prBfTxCmdCfg->fgTxCmdBfManual;
	UINT8 u1TxCmdBfBit = prBfTxCmdCfg->u1TxCmdBfBit;

	MTWF_PRINT("=========================== Global Setting ==============================\n");

	MTWF_PRINT("TxCmdBfManual = 0x%02X, TxCmdBfBit = 0x%02X\n",
		fgTxCmdBfManual, u1TxCmdBfBit);

	MTWF_PRINT("========================================================================\n");
}

VOID UniEventTxBfSndCntInfo(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_BF_SND_CNT_INFORM_BASIC *prSndCntEvt = (struct UNI_EVENT_BF_SND_CNT_INFORM_BASIC *)pBuf;
	UINT8 u1BandIdx;
	UINT8 band_num = get_txbf_cmd_adapt_band_num();
	UINT16 *pSndCnt = (UINT16 *)(pBuf + sizeof(struct UNI_EVENT_BF_SND_CNT_INFORM_BASIC));

	if (band_num == 0)
		return;

	MTWF_PRINT("\n======= BF Sounding Count Info (per 1280ms) =======\n");

	MTWF_PRINT("Current Snd Count:\n");

	for (u1BandIdx = 0; u1BandIdx < band_num; u1BandIdx++)
		MTWF_PRINT("    Band%u: %u\n", u1BandIdx, pSndCnt[u1BandIdx]);

	MTWF_PRINT("Snd Count Limit        : %u\n", prSndCntEvt->u2SndCntLmt);
	MTWF_PRINT("Manual Snd Count Limit : %u\n", prSndCntEvt->u2SndCntLmtMan);
	MTWF_PRINT("Current Condition      : 0x%x (", prSndCntEvt->u1SndCndCondi);

	if (prSndCntEvt->u1SndCndCondi & UNI_SND_CNT_CONDI_MANUAL)
		MTWF_PRINT(" Manual Mode,");

	if (prSndCntEvt->u1SndCndCondi & UNI_SND_CNT_CONDI_8RU)
		MTWF_PRINT(" 8 RU Mode");

	if (prSndCntEvt->u1SndCndCondi == UNI_SND_CNT_CONDI_DEFAULT)
		MTWF_PRINT(" Default Mode");

	MTWF_PRINT(" )\n");

	MTWF_PRINT("===================================================\n");

}

VOID UniEventBFCtrlFbRptDbgInfoHandle(
	IN struct _RTMP_ADAPTER *pAd,
	IN UINT8 *pBuf)
{
	struct UNI_EVENT_TXBF_FBK_INFO_BASIC *pBfFbRptInfo = (struct UNI_EVENT_TXBF_FBK_INFO_BASIC *)pBuf;
	UINT32 u4Total, i;
	UINT16 bf_starec_num = 0;
	UINT8 band_num = get_txbf_cmd_adapt_band_num();
	UINT32 *pRxPerStaFbRptCnt = (UINT32 *)(pBuf + sizeof(struct UNI_EVENT_TXBF_FBK_INFO_BASIC));

	if (band_num == 0)
		return;

	bf_starec_num = get_txbf_cmd_adapt_pfmu_num_max()*band_num;
	u4Total = pBfFbRptInfo->u4PFMUWRDoneCnt + pBfFbRptInfo->u4PFMUWRFailCnt;
	u4Total += pBfFbRptInfo->u4PFMUWRTimeoutFreeCnt + pBfFbRptInfo->u4FbRptPktDropCnt;

	MTWF_PRINT("\n");
	MTWF_PRINT("\x1b[32m =================================\x1b[m\n");
	MTWF_PRINT("\x1b[32m PFMUWRDoneCnt              = %u\x1b[m\n",
		pBfFbRptInfo->u4PFMUWRDoneCnt);
	MTWF_PRINT("\x1b[32m PFMUWRFailCnt              = %u\x1b[m\n",
		pBfFbRptInfo->u4PFMUWRFailCnt);
	MTWF_PRINT("\x1b[32m PFMUWRTimeOutCnt           = %u\x1b[m\n",
		pBfFbRptInfo->u4PFMUWRTimeOutCnt);
	MTWF_PRINT("\x1b[32m PFMUWRTimeoutFreeCnt       = %u\x1b[m\n",
		pBfFbRptInfo->u4PFMUWRTimeoutFreeCnt);
	MTWF_PRINT("\x1b[32m FbRptPktDropCnt            = %u\x1b[m\n",
		pBfFbRptInfo->u4FbRptPktDropCnt);
	MTWF_PRINT("\x1b[32m TotalFbRptPkt              = %u\x1b[m\n", u4Total);
	MTWF_PRINT("\x1b[32m PollPFMUIntrStatTimeOut    = %u(micro-sec)\x1b[m\n",
		pBfFbRptInfo->u4PollPFMUIntrStatTimeOut);
	MTWF_PRINT("\x1b[32m FbRptDeQInterval           = %u(milli-sec)\x1b[m\n",
		pBfFbRptInfo->u4DeQInterval);
	MTWF_PRINT("\x1b[32m PktCntInFbRptTimeOutQ      = %u\x1b[m\n",
		pBfFbRptInfo->u4RptPktTimeOutListNum);
	MTWF_PRINT("\x1b[32m PktCntInFbRptQ             = %u\x1b[m\n",
		pBfFbRptInfo->u4RptPktListNum);

	for (i = 0; (VALID_UCAST_ENTRY_WCID(pAd, i) && (i < bf_starec_num)); i++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, i);

		if (pEntry->EntryType == ENTRY_NONE)
			continue;
		MTWF_PRINT("\x1b[32m AID%u  RxFbRptCnt           = %u\x1b[m\n"
			, pEntry->Aid, pRxPerStaFbRptCnt[i]);
	}

	MTWF_PRINT("\x1b[32m =================================\x1b[m\n");
	MTWF_PRINT("\n");
}

void UniEventBFHandleEvent(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_BF);
	UINT32 event_fail_loop_cnt = 0;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_INFO,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = (UINT8 *)pData + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));
		switch (TAG_ID(tag)) {
		case UNI_EVENT_BF_STAREC:
			UniEventBfStaRecHandle(pAd, tag);
			break;
		case UNI_EVENT_BF_TXSND_INFO:
			UniEventBFCtrlTxSndHandle(pAd, tag);
			break;
		case UNI_EVENT_BF_FBK_INFO:
			UniEventBFCtrlFbRptDbgInfoHandle(pAd, tag);
			break;

		case UNI_EVENT_BF_PFMU_TAG:
			UniEventBfPfmuTagReadHandle(pAd, tag);
			break;

		case UNI_EVENT_BF_PFMU_DATA:
			UniEventBfPfmuDataReadHandle(pAd, tag);
			break;

		case UNI_EVENT_BF_CAL_PHASE:
			UniEventBfCalPhase(pAd, tag);
			break;

		case UNI_EVENT_BF_PLY_INFO:
			UniEventBfPlyInfo(pAd, tag);
			break;

		case UNI_EVENT_BF_METRIC_INFO:
			UniEventBFMuMetricInfo(pAd, tag);
			break;

		case UNI_EVENT_BF_TXCMD_CFG_INFO:
			UniEventBfTxCmdCfgInfo(pAd, tag);
			break;

		case UNI_EVENT_BF_SND_CNT_INFO:
			UniEventTxBfSndCntInfo(pAd, tag);
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_BF, CATBF_IWCMD, DBG_LVL_DEBUG,
				"invalid event tag = %d\n",
				TAG_ID(tag));
			break;
		}
	}
}
#endif /* TXBF_SUPPORT */

static VOID UniEventGBand256QamProbeResult(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	struct UNI_EVENT_RA_G_BAND_256QAM_PROBE_RESULT_T *pResult;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT16 u2WlanIdx = WCID_INVALID;

	if (Data != NULL) {
		pResult = (struct UNI_EVENT_RA_G_BAND_256QAM_PROBE_RESULT_T *)(Data);
		u2WlanIdx = WCID_GET_H_L(pResult->u1WlanIdxHnVer, pResult->u1WlanIdxL);
		pEntry = entry_get(pAd, u2WlanIdx);

		if (IS_ENTRY_NONE(pEntry)) {
			MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
					 "pEntry is NONE\n");
			return;
		}

		if (pResult->ucResult == RA_G_BAND_256QAM_PROBE_SUCCESS)
			pEntry->fgGband256QAMSupport = TRUE;

		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_INFO,
				 "Gband256QAMSupport = %d\n", pResult->ucResult);
	} else {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_PHY, DBG_LVL_ERROR,
				 " Data is NULL\n");
	}
}
#define PHY_MODE_CCK                        0
#define PHY_MODE_OFDM                       1
#define PHY_MODE_HTMIX                      2
#define PHY_MODE_HTGREENFIELD               3
#define PHY_MODE_VHT                        4
#define PHY_MODE_HESU                       8
#define PHY_MODE_HEEXTSU                    9
#define PHY_MODE_HETRIG                     10
#define PHY_MODE_HEMU                       11
#define PHY_MODE_EHTEXTTSU                  13
#define PHY_MODE_EHTTRIG                    14
#define PHY_MODE_EHTMU                      15
#define PHY_MODE_HESU_REMAPPING             5
#define PHY_MODE_HEEXTSU_REMAPPING          6
#define PHY_MODE_HETRIG_REMAPPING           7
#define PHY_MODE_HEMU_REMAPPING             8
#define PHY_MODE_EHT_REMAPPING              9
#define PHY_MODE_EHTEXTSU_REMAPPING         10
#define PHY_MODE_EHTTRIG_REMAPPING          11
#define PHY_MODE_EHTMU_REMAPPING            12
#define PHY_MODE_UNKNOWN_REMAPPING          13
#define TX_NSS_SHITF                        6
#define TX_NSS_MASK                         0x7
#define TX_HT_MCS_MASK                      0x3F
#define TX_NON_HT_MCS_MASK                  0xF
#define TX_MODE_SHIFT                       9
#define TX_MODE_MASK                        0xF
#define DCM_SHITF                           4
#define DCM_EN                              (1 << DCM_SHITF)
#define TX_BW_SHIFT                         13
#define TX_BW_MASK                          0x7

static UINT_8 getTxNss(TX_MODE_RATE eTxRate)
{
	return ((eTxRate  >> TX_NSS_SHITF) & TX_NSS_MASK) + 1;
}

static UINT_8 getTxMode(TX_MODE_RATE eTxRate)
{
    /* TODO: implement it */
	return ((eTxRate  >> TX_MODE_SHIFT) & TX_MODE_MASK);
}

static UINT_8 getTxModeRemap(TX_MODE_RATE eTxRate, UINT16 u2RuIdx, UINT16 u2Direction)
{
	UINT_8 u1TxMod = ((eTxRate  >> TX_MODE_SHIFT) & TX_MODE_MASK);
	UINT_8 phymode_mapping = PHY_MODE_EHT_REMAPPING;

	if (u1TxMod < PHY_MODE_HESU_REMAPPING)
		phymode_mapping = u1TxMod;
	else {
		if ((u1TxMod >= PHY_MODE_HESU) && (u1TxMod <= PHY_MODE_HEMU))
			phymode_mapping = PHY_MODE_HESU_REMAPPING;
		else if ((u1TxMod >= PHY_MODE_EHTEXTTSU) && (u1TxMod <= PHY_MODE_EHTMU))
			phymode_mapping = PHY_MODE_EHT_REMAPPING;

		if (u2RuIdx > 0) {
			if (u2Direction == 0)
				phymode_mapping += 3;
			else
				phymode_mapping += 2;
		}
	}

	return phymode_mapping;
}

static UINT_8 getTxDcm(TX_MODE_RATE eTxRate)
{
    /*TODO: Implement it*/
	return (eTxRate & DCM_EN && (getTxMode(eTxRate) > PHY_MODE_VHT)) ? 1 : 0;
}

static UINT_8 getTxMcs(TX_MODE_RATE eTxRate)
{
	if (getTxMode(eTxRate) == PHY_MODE_HTMIX)
		return eTxRate & TX_HT_MCS_MASK;
	else
		return eTxRate & TX_NON_HT_MCS_MASK;
}

static UINT_8 getTxBw(TX_MODE_RATE eTxRate, UINT_8 u1MaxBw)
{
	UINT8 bw_mapping = u1MaxBw;

	if (u1MaxBw == BW_320)
		bw_mapping = 4;
	else if (u1MaxBw == BW_8080)
		bw_mapping = BW_160;

	return bw_mapping - ((eTxRate >> TX_BW_SHIFT) & TX_BW_MASK);
}

static UINT_8 getTone(UINT16 u2RuIdx)
{
	if (u2RuIdx <= 2)
		return 0;
	else if (u2RuIdx > 2 && u2RuIdx <= 6)
		return 1;
	else if (u2RuIdx > 6 && u2RuIdx <= 14)
		return 2;
	else if (u2RuIdx > 14 && u2RuIdx <= 22)
		return 3;
	else
		return 4;
}

static VOID UniEventGetRuRaInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct UNI_EVENT_RA_RU_RA_INFO_T *CmdRuRaInfo = (struct UNI_EVENT_RA_RU_RA_INFO_T *)Data;
	UINT16 u2WlanIdx;
	UINT16 u2RuIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	UINT32 u4Per = 0;
	UINT8  u1MaxBw = 0;
	CHAR * phyMode[14] = {"CCK", "OFDM", "HT-MIX", "HT-GF", "VHT",
		"HE-SU", "HE-EXT-SU", "HE-TB", "HE-MU",
		"EHT", "EHT-EXT-SU", "EHT-TB", "EHT-MU",
		"UnKnown"};
	CHAR * dcm[2] = {"", "DCM"};
	CHAR * bw[5] = {"BW20", "BW40", "BW80", "BW160", "BW320"};
	CHAR * tone[5] = {"996-tone", "484-tone", "242-tone", "<106-tone", "UnKnown"};
	PMAC_TABLE_ENTRY pEntry;


	u2WlanIdx   = CmdRuRaInfo->u2WlanIdx;
	u2RuIdx	    = CmdRuRaInfo->u2RuIdx;
	u2Direction = CmdRuRaInfo->u2Direction;
	u2DumpGroup = CmdRuRaInfo->u2DumpGroup;

	pEntry = entry_get(pAd, u2WlanIdx);
	u1MaxBw = pEntry->MaxHTPhyMode.field.BW;

	MTWF_PRINT("\nWLAN ID : %d\n", u2WlanIdx);
	MTWF_PRINT("RU Idx : %d", u2RuIdx);

	if (u2Direction == 0)
		MTWF_PRINT(" Downlink\n");
	else

		MTWF_PRINT(" Uplink\n");

	/*Short Term RA Group*/
	if ((u2DumpGroup & 0x1) == 0x1) {
		MTWF_PRINT("Group: Short-Term RA\n");
		MTWF_PRINT("\tCurrRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2CurrRate)]);
		MTWF_PRINT("\tNoRateUpCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1NoRateUpCnt);
		MTWF_PRINT("\tSuggestTxModeRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate)]);
		MTWF_PRINT("\tSuggestWF : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1SuggestWF);
		MTWF_PRINT("\tStartProbeUpMCS : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS)]);
		MTWF_PRINT("\tIsProbeUpPeriod : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgIsProbeUpPeriod);
		MTWF_PRINT("\tInitRateDownTotalCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownTotalCnt);
		MTWF_PRINT("\tInitRateDownOkCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.initRateDownOkCnt);
		MTWF_PRINT("\tInitRateDownMCS : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS)]);
		MTWF_PRINT("\tProbeDownPending : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgProbeDownPending);
		MTWF_PRINT("\tStSucceCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StSucceCnt);
		MTWF_PRINT("\tStTotalTxCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt);
		MTWF_PRINT("\tRuPrevRate : 0x%x %s %s NSS%d MCS%d %s\n",
				CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate,
				phyMode[getTxModeRemap(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate, u2RuIdx, u2Direction)],
				(u2RuIdx == 0) ? bw[getTxBw(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate, u1MaxBw)] : tone[getTone(u2RuIdx)],
				getTxNss(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxMcs(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				dcm[getTxDcm(CmdRuRaInfo->rRuIdxRateInfo.u2RuPrevRate)]);
		MTWF_PRINT("\tStTotalPpduCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1StTotalPpduCnt);
		MTWF_PRINT("\tGI : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1Gi);
		MTWF_PRINT("\tRuTryupFailCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1RuTryupFailCnt);
		MTWF_PRINT("\tRuTryupCnt : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.u1RuTryupCnt);
		MTWF_PRINT("\tRuTryupCheck : %d\n",
				CmdRuRaInfo->rRuIdxRateInfo.fgRuTryupCheck);

		if (CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt)
			u4Per = ((CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt -
					CmdRuRaInfo->rRuIdxRateInfo.u2StSucceCnt) * 1000) /
					CmdRuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt;

		MTWF_PRINT("\tPER : %d.%d%%\n",
						u4Per/10, u4Per % 10);
	}

	/*Long Term RA Group*/
	if ((u2DumpGroup & 0x2) == 0x2)
		MTWF_PRINT("Group: Long-Term RA\n");

	/*Others Group*/
	if ((u2DumpGroup & 0x4) == 0x4)
		MTWF_PRINT("Group: Others\n");
}


static VOID UniEventGetMuRaInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct UNI_EVENT_RA_MU_RA_INFO_T *CmdMuRaInfo = (struct UNI_EVENT_RA_MU_RA_INFO_T *)Data;
	UINT16 u2MuGroupIdx;
	UINT16 u2UserIdx;
	UINT16 u2Direction;
	UINT16 u2DumpGroup;
	UINT32 u4Per = 0;

	u2MuGroupIdx = CmdMuRaInfo->u2MuGroupIdx;
	u2UserIdx    = CmdMuRaInfo->u2UserIdx;
	u2Direction  = CmdMuRaInfo->u2Direction;
	u2DumpGroup  = CmdMuRaInfo->u2DumpGroup;

	MTWF_PRINT("\nMU Group ID : %d\n", u2MuGroupIdx);
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "User Idx : %d", u2UserIdx);

	if (u2Direction == 0)
		MTWF_PRINT(" Downlink\n");
	else
		MTWF_PRINT(" Uplink\n");

	/*Short Term RA Group*/
	if ((u2DumpGroup & 0x1) == 0x1) {
		MTWF_PRINT("Group: Short-Term RA\n");
		MTWF_PRINT("\tCurrRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2CurrRate));
		MTWF_PRINT("\tNoRateUpCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1NoRateUpCnt);
		MTWF_PRINT("\tSuggestTxModeRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2SuggestTxModeRate));
		MTWF_PRINT("\tSuggestWF : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1SuggestWF);
		MTWF_PRINT("\tStartProbeUpMCS : %x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StartProbeUpMCS);
		MTWF_PRINT("\tIsProbeUpPeriod : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgIsProbeUpPeriod);
		MTWF_PRINT("\tInitRateDownTotalCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.initRateDownTotalCnt);
		MTWF_PRINT("\tInitRateDownOkCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.initRateDownOkCnt);
		MTWF_PRINT("\tInitRateDownMCS : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
			    CmdMuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS,
			    getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2InitRateDownMCS));
		MTWF_PRINT("\tProbeDownPending : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgProbeDownPending);
		MTWF_PRINT("\tStSucceCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StSucceCnt);
		MTWF_PRINT("\tStTotalTxCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt);
		MTWF_PRINT("\tRuPrevRate : 0x%x Mode=%d NSS%d MCS%d DCM=%x\n",
				CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate,
				getTxMode(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxNss(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxMcs(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate),
				getTxDcm(CmdMuRaInfo->rRuIdxRateInfo.u2RuPrevRate));
		MTWF_PRINT("\tStTotalPpduCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1StTotalPpduCnt);
		MTWF_PRINT("\tGI : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1Gi);
		MTWF_PRINT("\tRuTryupFailCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1RuTryupFailCnt);
		MTWF_PRINT("\tRuTryupCnt : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.u1RuTryupCnt);
		MTWF_PRINT("\tRuTryupCheck : %d\n",
				CmdMuRaInfo->rRuIdxRateInfo.fgRuTryupCheck);

		if (CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt)
			u4Per = ((CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt -
					CmdMuRaInfo->rRuIdxRateInfo.u2StSucceCnt) * 1000) /
					CmdMuRaInfo->rRuIdxRateInfo.u2StTotalTxCnt;

		MTWF_PRINT("\tPER : %d.%d%%\n",
						u4Per/10, u4Per % 10);
	}

	/*Long Term RA Group*/
	if ((u2DumpGroup & 0x2) == 0x2)
		MTWF_PRINT("Group: Long-Term RA\n");

	/*Others Group*/
	if ((u2DumpGroup & 0x4) == 0x4)
		MTWF_PRINT("Group: Others\n");
}
void UniEventRAHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_RA);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
			"invalid length = %d\n", Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = (UINT8 *)pData + fixed_len;

	switch (*tag) {
	case UNI_EVENT_RA_G_BAND_256QAM_PROBE_RESULT:
		UniEventGBand256QamProbeResult(pAd, tag, tags_len);
		break;

	case UNI_EVENT_RA_RU_RA_INFO:
		UniEventGetRuRaInfo(pAd, tag, tags_len);
		break;

	case UNI_EVENT_RA_MU_RA_INFO:
		UniEventGetMuRaInfo(pAd, tag, tags_len);
		break;

	default:
		break;
	}
}

VOID UniEventThemalSensorRsp(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len
)
{
	struct UNI_EVENT_THERMAL_T *event;
	struct _UNI_EVENT_THERMAL_RSP_T *tlv;
	P_EXT_EVENT_THERMAL_SENSOR_INFO_T prEventResult;

	if (payload_len < sizeof(*event)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	event = (struct UNI_EVENT_THERMAL_T *)payload;
	tlv = (struct _UNI_EVENT_THERMAL_RSP_T *) &(event->au1TlvBuffer[0]);

	prEventResult = (P_EXT_EVENT_THERMAL_SENSOR_INFO_T) &(tlv->aucBuffer[0]);
	prEventResult->u4SensorResult = le2cpu32(prEventResult->u4SensorResult);
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &prEventResult->u4SensorResult, sizeof(prEventResult->u4SensorResult));
	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"ThemalSensor = 0x%x\n", prEventResult->u4SensorResult);
}

VOID UniEventThemalTempAdcRsp(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len
)
{
	struct UNI_EVENT_THERMAL_T *event;
	struct _UNI_EVENT_THERMAL_RSP_T *tlv;
	struct _EXT_EVENT_ID_GET_TEMP_ADC_T *prEventResult;
	struct _EXT_EVENT_ID_GET_TEMP_ADC_T *calbk;
	UINT8 itemNum = 0, *pNum = NULL, index = 0;

	if (payload_len < sizeof(*event)) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	event = (struct UNI_EVENT_THERMAL_T *)payload;
	tlv = (struct _UNI_EVENT_THERMAL_RSP_T *) &(event->au1TlvBuffer[0]);
	prEventResult = (struct _EXT_EVENT_ID_GET_TEMP_ADC_T *) &(tlv->aucBuffer[0]);
	itemNum = (tlv->u2Length - sizeof(struct _UNI_EVENT_THERMAL_RSP_T)) /
				sizeof(struct _EXT_EVENT_ID_GET_TEMP_ADC_T);

	pNum = (UINT8 *)msg->attr.rsp.wb_buf_in_calbk;
	*pNum = itemNum;

	MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
		"Die count:%u\n", *pNum);

	pNum++;
	calbk = (struct _EXT_EVENT_ID_GET_TEMP_ADC_T *)pNum;

	if (tlv->u2Tag == UNI_THERMAL_EVENT_ADC_TEMP_INFO) {
		for (index = 0; index < itemNum; index++, calbk++, prEventResult++) {
			calbk->u1adc = le2cpu32(prEventResult->u1adc);
			calbk->u1temp = le2cpu32(prEventResult->u1temp);

			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
				"cal adc:%x, cal temp:%x\n",
				calbk->u1adc, calbk->u1temp);
		}
	}
}

VOID UniEventThermalUnsolicitHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct UNI_EVENT_THERMAL_T *event;
	struct _UNI_EVENT_THERMAL_RSP_T *tlv;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_THERMAL_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	event = (struct UNI_EVENT_THERMAL_T *)pData;
	tlv = (struct _UNI_EVENT_THERMAL_RSP_T *) &(event->au1TlvBuffer[0]);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

	switch (tlv->u2Tag) {

	case UNI_THERMAL_EVENT_THERMAL_SENSOR_BASIC_INFO:
		EventThermalSensorShowInfo(pAd, &(tlv->aucBuffer[0]), (tags_len - sizeof(*tlv)));
		break;

	case UNI_THERMAL_EVENT_THERMAL_SENSOR_TASK_RESPONSE:
		EventThermalSensorTaskResp(pAd, &(tlv->aucBuffer[0]), (tags_len - sizeof(*tlv)));
		break;

	case UNI_THERMAL_EVENT_THERMAL_PROTECT_MECH_INFO:
		EventThermalProtInfo(pAd, &(tlv->aucBuffer[0]), (tags_len - sizeof(*tlv)));
		break;

	case UNI_THERMAL_EVENT_THERMAL_PROTECT_DUTY_INFO:
		EventThermalProtDutyInfo(pAd, &(tlv->aucBuffer[0]), (tags_len - sizeof(*tlv)));
		break;

	case UNI_THERMAL_EVENT_THERMAL_PROTECT_DUTY_UPDATE:
		EventThermalProtDutyNotify(pAd, &(tlv->aucBuffer[0]), (tags_len - sizeof(*tlv)));
		break;

	case UNI_THERMAL_EVENT_THERMAL_PROTECT_RADIO_UPDATE:
		EventThermalRadioNotify(pAd, &(tlv->aucBuffer[0]), (tags_len - sizeof(*tlv)));
		break;

	default:
		break;
	}
}

VOID UniEventNFAvgPwr(RTMP_ADAPTER *pAd, struct UNI_EVENT_NF_ENABLE_T *Data, UINT16 Length)
{
	UINT32 avgpwr[4];
	INT8 dBm[IPI_ANT_NUM];
	UINT8 i, msb, u1Shift = 0x1;
	UINT32 u4Mask = 0x000000FE;

	UINT8 u1Ipi_Idx = 0;
	UINT32 total = 0;
	INT_32 i4NF_Ipi;
	INT16 NF_Power[] = {-92, -89, -86, -83, -80, -75, -70, -65, -60, -55, -52};
	struct UNI_EVENT_NF_ENABLE_T *p_nf_enable_result = (struct UNI_EVENT_NF_ENABLE_T *)Data;
	UCHAR per_band_ipi_ant;
	INT32 per_band_nf_total = 0;
	INT32 ret, left_buf_size;
	INT8 tmp_str[64] = {0};

	struct _RTMP_ADAPTER *pAd_per_band = NULL;
	struct physical_device *device_entry = pAd->physical_dev;
	UINT8 band_num = device_entry->band_num;
	UINT8 band_idx = 0;
	INT32 nf[MAX_BAND_NUM] = {0};

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
	"band_num:%d, IPI_ANT_NUM=%d\n", band_num, IPI_ANT_NUM);

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
		"Average Power, u1mode:%d\n", p_nf_enable_result->u1mode);

	if (p_nf_enable_result->u1mode == 0) {
		/*Todo: u1mode = 0 is not supported currently*/
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "Idle Power\n");
		for (i = 0; i < 4; i++) {
			avgpwr[i] = p_nf_enable_result->au4avgpwr[i];
			dBm[i] = (INT8)((avgpwr[i] & u4Mask) >> u1Shift);
			msb = ((avgpwr[i] & 0x00000100) >> 8);
			dBm[i] = (dBm[i] & 0x7f);
			dBm[i] = ((0x7f ^ dBm[i]) + 1);

			if (msb)
				dBm[i] = dBm[i] * (-1);

			left_buf_size = sizeof(tmp_str) - strlen(tmp_str);
			ret = snprintf(tmp_str + strlen(tmp_str), left_buf_size, "%d ", dBm[i]);
			if (os_snprintf_error(left_buf_size, ret)) {
				MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
					"final_name snprintf error!\n");
				return;
			}
		}
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "%s\n", tmp_str);
	} else {
		for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
			pAd_per_band = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
			if (pAd_per_band == NULL)
				continue;
			per_band_ipi_ant = pAd_per_band->Antenna.field.RxPath;
#ifdef ANTENNA_CONTROL_SUPPORT
			if (pAd_per_band->RxStream)
				per_band_ipi_ant = pAd_per_band->RxStream;
#endif
			if (per_band_ipi_ant == 0 || per_band_ipi_ant > 4) {
				/*if ant number is wrong, set to 4 ant per band*/
				per_band_ipi_ant = 4;
			}
			MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
			"band%d IPI, ant_num=%d:\n ", band_idx, per_band_ipi_ant);
			for (i = 4*band_idx; i < (4*band_idx + per_band_ipi_ant) && i < IPI_ANT_NUM; i++) {
				total = 0;
				i4NF_Ipi = 0;
				for (u1Ipi_Idx = 0; u1Ipi_Idx <= 10; u1Ipi_Idx++) {
					MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
					"au4avgPIHist[%d][%d]=%d\n ",
					i, u1Ipi_Idx, p_nf_enable_result->au4avgPIHist[i][u1Ipi_Idx]);
					total += p_nf_enable_result->au4avgPIHist[i][u1Ipi_Idx];
					i4NF_Ipi += (NF_Power[u1Ipi_Idx] * (INT32) p_nf_enable_result->au4avgPIHist[i][u1Ipi_Idx]);
				}
				dBm[i] = (i4NF_Ipi/(INT32)total);
				MTWF_DBG(NULL, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "dBm[%d]=%d\n", i, dBm[i]);
			}

			/*Calculate IPI average per band*/
			per_band_nf_total = 0;
			for (i = 4*band_idx; i < (4*band_idx + per_band_ipi_ant) && i < IPI_ANT_NUM; i++)
				per_band_nf_total += dBm[i];

			nf[band_idx] = per_band_nf_total/per_band_ipi_ant;
		}
	}

	/*======Update per band noise floor/power=====*/
	for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
		pAd_per_band = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
		if (pAd_per_band == NULL)
			continue;

#ifdef NF_SUPPORT_V2
			pAd_per_band->Avg_NF = nf[band_idx];
#endif
			pAd_per_band->noise_floor = nf[band_idx];



#ifdef OFFCHANNEL_SCAN_FEATURE
		if (pAd_per_band->ScanCtrl.OffChScan_Ongoing == TRUE)
			pAd_per_band->ChannelInfo.AvgNF = nf[band_idx];

#endif
		MTWF_DBG(pAd_per_band, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO,
		"band_%d Average NF = %d, nf in pAd = %d\n", band_idx, nf[band_idx], pAd_per_band->noise_floor);
	}

	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "\n");
}

VOID UniEventNoiseFloorHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct UNI_EVENT_NOISE_FLOOR_T *event;
	struct UNI_EVENT_NF_ENABLE_T *tlv;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_NOISE_FLOOR_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	event = (struct UNI_EVENT_NOISE_FLOOR_T *)pData;
	tlv = (struct UNI_EVENT_NF_ENABLE_T *) &(event->au1TlvBuffer[0]);

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

	switch (tlv->u2Tag) {

	case UNI_EVENT_NF_ENABLE:
		UniEventNFAvgPwr(pAd, tlv, tlv->u2Length);
		break;

	default:
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_ERROR,
			"unknown event tlv tag = %d\n",
			tlv->u2Tag);
		break;
	}
}


static VOID UniEventTpcDownLinkTbl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T *prTpcInfo;
	UINT8 i;

	prTpcInfo = (struct _UNI_EVENT_TPC_INFO_DOWNLINK_TABLE_T *)Data;

	MTWF_PRINT("TPC DOWNLINK INFO TABLE\n\n");
	MTWF_PRINT("Category = %d\n", prTpcInfo->u1TpcCategory);
	MTWF_PRINT("AP INFO\n");
	MTWF_PRINT("=================================================\n");
	MTWF_PRINT("DL Tx Type        Cmd Pwr Ctrl      DL Tc Pwr\n");
	MTWF_PRINT("=================================================\n");
	MTWF_PRINT("MU MIMO	             %3d              %3d\n",
		prTpcInfo->fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_MU_MIMO],
		prTpcInfo->i1DlTxPwr[UNI_TPC_DL_TX_TYPE_MU_MIMO]);
	MTWF_PRINT("OFDMA                %3d              %3d\n\n",
		prTpcInfo->fgCmdPwrCtrl[UNI_TPC_DL_TX_TYPE_MU_OFDMA],
		prTpcInfo->i1DlTxPwr[UNI_TPC_DL_TX_TYPE_MU_OFDMA]);

	MTWF_PRINT("STA INFO\n");
	MTWF_PRINT("=================================================\n");
	MTWF_PRINT("WLAN		TxPwrAlpha MU_MIMO		TxPwrAlpha OFDMA\n");
	MTWF_PRINT("=================================================\n");
	for (i = 0; i < UNI_TPC_SUPPORT_STA_NUM; i++)
		MTWF_PRINT("%3d              %3d              %3d\n",
			prTpcInfo->rTpcDlManModeParamElem[i].u2WlanId,
			prTpcInfo->rTpcDlManModeParamElem[i].i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_MU_MIMO],
			prTpcInfo->rTpcDlManModeParamElem[i].i2DlTxPwrAlpha[UNI_TPC_DL_TX_TYPE_MU_OFDMA]);
}

static VOID UniEventTpcUpLinkTbl(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _UNI_EVENT_TPC_INFO_UPLINK_TABLE_T *prTpcInfo;
	UINT8 i;

	prTpcInfo = (struct _UNI_EVENT_TPC_INFO_UPLINK_TABLE_T *)Data;

	MTWF_PRINT("TPC UPLINK INFO TABLE\n\n");
	MTWF_PRINT("Category = %d\n", prTpcInfo->u1TpcCategory);
	MTWF_PRINT("AP INFO: AP TX Power = %d\n", prTpcInfo->u1ApTxPwr);
	MTWF_PRINT("STA INFO\n");
	MTWF_PRINT("======================================================\n");
	MTWF_PRINT("WLAN      TargetRssi      PwrHeadRoom       MinPwrFlag\n");
	MTWF_PRINT("======================================================\n");
	for (i = 0; i < UNI_TPC_SUPPORT_STA_NUM; i++)
		MTWF_PRINT("%3d         %3d             %3d            %3d\n",
														prTpcInfo->rTpcUlManModeParamElem[i].u2WlanId,
														prTpcInfo->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1TargetRssi,
														prTpcInfo->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.u1PwrHeadRoom,
														prTpcInfo->rTpcUlManModeParamElem[i].rTpcUlStaCmmInfo.fgMinPwr);
}

VOID UniEventTpcUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct _UNI_EVENT_TPC_T *event;
	struct _UNI_EVENT_TPC_TLV_T *tlv;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_TPC_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	event = (struct _UNI_EVENT_TPC_T *)pData;
	tlv = (struct _UNI_EVENT_TPC_TLV_T *) &(event->aucTlvBuffer[0]);

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	MTWF_PRINT("TPC Event\n");
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "u2Tag %2u\n", tlv->u2Tag);

	switch (tlv->u2Tag) {
	case TPC_EVENT_DOWNLINK_TABLE:
		UniEventTpcDownLinkTbl(pAd, (UINT8 *)tlv, Length);
		break;

	case TPC_EVENT_UPLINK_TABLE:
		UniEventTpcUpLinkTbl(pAd, (UINT8 *)tlv, Length);
		break;

	default:
		break;
	}
}

static VOID UniMecInfoAmsduEnPrint(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *pu1Buf)
{
	struct _UNI_EVENT_MEC_INFO_T *tlv = (struct _UNI_EVENT_MEC_INFO_T *)pu1Buf;
	UINT8 u1AlgoEn;
	UINT16 u2WlanIdx;
	UINT16 wtbl_max_num = hc_get_chip_wtbl_max_num(pAd);
#ifdef SW_CONNECT_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
#endif /* SW_CONNECT_SUPPORT */

	MTWF_PRINT("[+] AMSDU Algo Enable Status for all %u STAs (sync %u)\n",
		hc_get_chip_wtbl_max_num(pAd), WTBL_MAX_NUM(pAd));

	for (u2WlanIdx = 0; u2WlanIdx < (wtbl_max_num / 32); u2WlanIdx++)
		MTWF_PRINT("     au4MecAlgoEnSta[%u]: 0x%08X\n", u2WlanIdx, tlv->au4MecAlgoEnSta[u2WlanIdx]);

	MTWF_PRINT("[+] Connected STA AMSDU Algo Enable Status\n");

	for (u2WlanIdx = 1; VALID_UCAST_ENTRY_WCID(pAd, u2WlanIdx); u2WlanIdx++) {
		PMAC_TABLE_ENTRY pEntry = entry_get(pAd, u2WlanIdx);

		if (pEntry->EntryType == ENTRY_NONE)
			continue;

#ifdef SW_CONNECT_SUPPORT
		tr_entry = tr_entry_get(pAd, pEntry->tr_tb_idx);
		if (IS_SW_STA(tr_entry))
			continue;
#endif /* SW_CONNECT_SUPPORT */

		u1AlgoEn = ((tlv->au4MecAlgoEnSta[u2WlanIdx>>0x5] & BIT(u2WlanIdx & 0x1F)) >> (u2WlanIdx & 0x1F));
		MTWF_PRINT("     WlanIdx %2u, AID %2u, AMSDU Algo Enable: %u\n",
			u2WlanIdx, pEntry->Aid, u1AlgoEn);
	}
}

static VOID UniMecInfoAmsduThrPrint(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 *pu1Buf)
{
	struct _UNI_EVENT_MEC_INFO_T *tlv = (struct _UNI_EVENT_MEC_INFO_T *)pu1Buf;
	UINT16 u2WlanIdx;

	MTWF_PRINT("[+] PHY Rate Threshold for AMSDU Length Setting\n");

	for (u2WlanIdx = UNI_CMD_MEC_CTRL_BA_NUM_64; u2WlanIdx < UNI_CMD_MEC_CTRL_BA_NUM_MAX; u2WlanIdx++) {
		MTWF_PRINT("BA %2u\n", ((u2WlanIdx == UNI_CMD_MEC_CTRL_BA_NUM_64)?64:256));
		MTWF_PRINT("Num   Len   Threshold\n");
		MTWF_PRINT("1     Index1   0 Mbps\n");
		MTWF_PRINT("2     Index2   %4u Mbps\n",
			tlv->au4MecAmsduTable[u2WlanIdx][UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_2]);
		MTWF_PRINT("3     Index3   %4u Mbps\n",
			tlv->au4MecAmsduTable[u2WlanIdx][UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_3]);
		MTWF_PRINT("4     Index4   %4u Mbps\n",
			tlv->au4MecAmsduTable[u2WlanIdx][UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_4]);
		MTWF_PRINT("5     Index5   %4u Mbps\n",
			tlv->au4MecAmsduTable[u2WlanIdx][UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_5]);
		MTWF_PRINT("6     Index6   %4u Mbps\n",
			tlv->au4MecAmsduTable[u2WlanIdx][UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_6]);
		MTWF_PRINT("7     Index7   %4u Mbps\n",
			tlv->au4MecAmsduTable[u2WlanIdx][UNI_CMD_MEC_CTRL_AMSDU_THR_IDX_LEN_7]);
	}
}

VOID UniEventMecUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct _UNI_EVENT_MEC_T *event;
	struct _UNI_EVENT_MEC_INFO_T *tlv;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_MEC_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	event = (struct _UNI_EVENT_MEC_T *)pData;
	tlv = (struct _UNI_EVENT_MEC_INFO_T *) &(event->au1TlvBuffer[0]);

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	MTWF_PRINT("MEC Ctrl Info:\n");
	MTWF_DBG(pAd, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, "u2ReadType %2u\n", tlv->u2ReadType);

	if (tlv->u2ReadType == MEC_EVENT_READ_TYPE_ALL ||
		tlv->u2ReadType & MEC_EVENT_READ_TYPE_ALGO_EN)
		UniMecInfoAmsduEnPrint(pAd, (UINT8 *)tlv);
	if (tlv->u2ReadType == MEC_EVENT_READ_TYPE_ALL ||
		tlv->u2ReadType & MEC_EVENT_READ_TYPE_AMSDU_THR) {
		UniMecInfoAmsduThrPrint(pAd, (UINT8 *)tlv);
	}
}

VOID UniEventTestModeUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct _UNI_EVENT_TESTMODE_CTRL_T *event;
	struct _UNI_EVENT_RF_TEST_TLV_T *tlv;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(*event) + sizeof(*tlv);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);
#ifdef CONFIG_WLAN_SERVICE
	struct service_test *serv_test;
	UINT32 en_log = 0;
	struct test_log_dump_cb *test_log_dump;
	struct test_operation *ops;

	serv_test = (struct service_test *)(pAd->serv.serv_handle);
	ops = serv_test->test_op;
	test_log_dump = &serv_test->test_log_dump[0];
	en_log = serv_test->en_log;
#endif

	event = (struct _UNI_EVENT_TESTMODE_CTRL_T *)pData;
	tlv = (struct _UNI_EVENT_RF_TEST_TLV_T *)&event->aucTlvBuffer[0];

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

#ifdef CONFIG_WLAN_SERVICE
	if (ops->op_evt_rf_test_cb)
		ops->op_evt_rf_test_cb(serv_test->test_winfo, test_log_dump, en_log, &(tlv->aucBuffer[0]), tags_len);
#else
#ifdef CONFIG_ATE
	MT_ATERFTestCB(pAd, &(tlv->aucBuffer[0]), tags_len);
#endif /* CONFIG_ATE */
#endif /* CONFIG_WLAN_SERVICE */
}

#ifdef ZERO_PKT_LOSS_SUPPORT
static VOID UniEventBcnTxStatus(
	VOID *physical_dev,
	struct _UNI_EVENT_BCN_TXOK_STATUS_T *TlvData)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	struct DOT11_H *pDot11h = NULL;

	pAd = physical_device_get_mac_adapter_by_band(physical_dev, TlvData->u1BandIdx);
	if (pAd == NULL) {
		MTWF_PRINT("pAd null\n");
		return;
	}
	pDot11h = &pAd->Dot11_H;
	if (pDot11h == NULL) {
		MTWF_PRINT("pdot11h null\n");
		return;
	}

	/*Bcn tx event only enable during CSA,
	* Channel switch on bcn with CSA Count =1
	*/
	if (TlvData->u1Csa_count == pDot11h->ChannelSwitchTriggerCSACount) {
		BOOLEAN Cancelled;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_WARN,
						"band idx:%d wdev_idx:%d CSA count:%d Bcn sent\n",
						TlvData->u1BandIdx, TlvData->u1OwnMacIdx,
						TlvData->u1Csa_count);

		pAd->chan_switch_time[2] = jiffies_to_msecs(jiffies);
		if (timer_pending(&pDot11h->CSALastBcnTxEventTimer.TimerObj)) {
			RTMPCancelTimer(&pDot11h->CSALastBcnTxEventTimer, &Cancelled);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_WARN,
							"CSALastBcnTxEventTimer cancel:%d\n", Cancelled);
			/*(Bcn0 + 5ms * n) where n is Mbsscount, intermbss bcn time is 5ms
			  *(Bcn0 + 1ms * n) where n is Mbsscount, intermbss bcn time is 1ms,
			  * thus 5 mbss bcn time is aroud 7
			  * for safety side, time is kept 8ms, try 1 ms work till 3 Mbss
			*/
			RTMPSetTimer(&pDot11h->CSALastBcnTxEventTimer, 5);
		} else
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_SCS, DBG_LVL_WARN,
						"CSALastBcnTxEventTimeout caused channel switch already\n");
	}
}
#endif /*ZERO_PKT_LOSS_SUPPORT*/

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
static VOID UniEventClientTWTInfoProc(
	struct _RTMP_ADAPTER *pAd,
	struct UNI_EVENT_CLIENT_TWT_INFO_T *TlvData)
{
	struct wifi_dev *wdev = NULL;
	struct twt_resume_info resume_info = {0};
	struct _MAC_TABLE_ENTRY *entry = NULL;
	UINT16 wcid = le2cpu16(TlvData->u2WlanIdx);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	entry = entry_get(pAd, wcid);
	wdev = entry->wdev;
	resume_info.bssinfo_idx = TlvData->ucBssIndex;
	resume_info.wcid = wcid;
	resume_info.flow_id = TlvData->ucFlowId;
	resume_info.idle = TlvData->ucIdle;

	twt_get_resume_event(wdev, &resume_info);
}

static VOID UniEventTWTMgmtFrameTxsProc(
	struct _RTMP_ADAPTER *pAd,
	struct UNI_EVENT_TWT_ACTION_TXS_INFO_T *TlvData)
{
	struct mtk_mac_dev *mac_dev = NULL;
	struct wifi_dev *wdev = NULL;
	struct _MAC_TABLE_ENTRY *entry = NULL;
	struct _RTMP_ADAPTER *_pAd = NULL;
	struct TWT_TX_CMD_CTRL_T *twt_cmd;
	UINT16 wcid = le2cpu16(TlvData->u2WlanIdx);
	UINT8 bss_idx = TlvData->ucBssIndex;
#ifdef DOT11_EHT_BE
	struct mld_entry_t *mld_entry = NULL;
#endif /* DOT11_EHT_BE */

	/* use bss fw_idx to find pAd whom this bss_idx belongs to */
	mac_dev = hc_get_mac_dev_by_bssidx(pAd, bss_idx);

	if (!mac_dev) {
		MTWF_DBG(_pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. expected mac_dev not found\n");
		return;
	}

	_pAd = mac_dev->ad;
	if (!_pAd)
		return;

	MTWF_DBG(_pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"ad(%d)=%p,%p\n", (pAd == _pAd) ? 1 : 0, pAd, _pAd);


	if (!VALID_UCAST_ENTRY_WCID(_pAd, wcid))
		return;

	entry = entry_get(pAd, wcid);
	if (!IS_VALID_ENTRY(entry))
		return;

	wdev = entry->wdev;
	twt_cmd = &entry->twt_ctrl.twt_cmd;
#ifdef DOT11_EHT_BE
	mt_rcu_read_lock();
	mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);

	MTWF_DBG(_pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"bss=%d,wcid=%d,mlo_en=%d,%d,token=%d,%d,mld_sta_idx=%d,%d,action_linkid=%d,all_linkid_bitmap=0x%x,sts=%d\n",
		TlvData->ucBssIndex,
		wcid,
		entry->mlo.mlo_en, TlvData->ucMloEn,
		twt_cmd->token, TlvData->ucToken,
		mld_entry ? mld_entry->mld_sta_idx : MLD_STA_NONE,
		TlvData->u2MldStaIndx,
		TlvData->ucActionLinkid,
		TlvData->u2AllLinkidBitmap,
		TlvData->ucStatus);
	mt_rcu_read_unlock();
#endif /* #ifdef DOT11_EHT_BE */
	if (twt_cmd->token == TlvData->ucToken) {
		UINT8 wait_ack_sts = (!TlvData->ucStatus && wcid) ? TWT_WAIT_ACK_OK : TWT_WAIT_ACK_NG;

#ifdef DOT11_EHT_BE
		if (TlvData->ucMloEn)
			bss_mngr_mld_twt_wait_ack_inform(wdev,
				TlvData->u2MldStaIndx,
				TlvData->ucActionLinkid,
				TlvData->u2AllLinkidBitmap,
				wait_ack_sts);
		else
#endif /* DOT11_EHT_BE */
			twt_wait_ack_inform(wdev, wcid, wait_ack_sts);
	} else
		MTWF_DBG(_pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. token mismatch=%d,%d\n", twt_cmd->token, TlvData->ucToken);

}

VOID UniEventTWTSyncHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_TWT_SYNC_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"Error!!! pAd is NULL!!!\n");
		return;
	}

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"invalid length = %d\n", Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_CLIENT_TWT_INFO:
			UniEventClientTWTInfoProc(pAd, (void *)tag);
			break;

		case UNI_EVENT_TWT_ACTION_TXS_INFO:
			UniEventTWTMgmtFrameTxsProc(pAd, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"invalid event tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#ifdef CFG_SUPPORT_CSI
static VOID UniEventCSIDataProc(
	struct _RTMP_ADAPTER *pAd,
	struct UNI_EVENT_CSI_DATA_T *TlvData)
{
	UINT_16 *pru2Tmp = NULL;
	UINT_32 *p32tmp = NULL;
	INT_16 i2Idx = 0;
	UINT_32 rx_info = 0;
	UINT32 ret = 0;
	struct CSI_MAX_DATA_T *currnet_data = NULL;
	struct CSI_DATA_T *final_data = NULL;
	TLV_ELEMENT_T *prCSITlvData = NULL;
	struct CSI_INFO_T *prCSIInfo = &pAd->rCSIInfo;
	UINT8 *prBuf = &TlvData->aucBuffer[0];
	/* u2Offset is 8 bytes currently, tag 4 bytes + length 4 bytes */
	UINT_16 u2Offset = Offsetof(TLV_ELEMENT_T, aucbody);
	UINT32 Length = TlvData->u2Length - sizeof(struct UNI_EVENT_CSI_DATA_T);

	/*hex_dump_with_lvl("CSIEventDump:\n ", (UCHAR *)prBuf, (UINT)Length, DBG_LVL_ERROR);*/

	os_alloc_mem(NULL, (UCHAR **)&currnet_data, sizeof(struct CSI_MAX_DATA_T));

	if (!currnet_data) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
		"allocate memory for prCSIData failed!\n");
		return;
	}

	os_zero_mem(currnet_data, sizeof(struct CSI_DATA_T));

	while (Length >= u2Offset) {
		prCSITlvData = (struct TLV_ELEMENT *)prBuf;
		prCSITlvData->tag_type = le2cpu32(prCSITlvData->tag_type);
		prCSITlvData->body_len = le2cpu32(prCSITlvData->body_len);
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_DEBUG,
			"tag_type=%d,body_len=%d,length=%d\n",
			prCSITlvData->tag_type, prCSITlvData->body_len, Length);

		switch (prCSITlvData->tag_type) {
		case CSI_EVENT_FW_VER:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid FW VER len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->FWVer = (UINT8)le2cpu32(*((UINT32 *)prCSITlvData->aucbody));
			prCSIInfo->FWVer = currnet_data->FWVer;
			break;
		case CSI_EVENT_CBW:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CBW len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucBw = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RSSI:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSSI len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->cRssi = (UINT8)le2cpu32(*((INT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_SNR:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid SNR len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucSNR = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_BAND:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid BAND len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucDbdcIdx = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			if ((currnet_data->ucDbdcIdx != BAND0) &&
				(currnet_data->ucDbdcIdx != BAND1) &&
				(currnet_data->ucDbdcIdx != BAND2)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid BAND IDX (%d)\n", currnet_data->ucDbdcIdx);
				goto out;
			}
			break;
		case CSI_EVENT_CSI_NUM:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI num len %u", prCSITlvData->body_len);
				goto out;
			}

			currnet_data->u2DataCount = (UINT16)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));

			/*at most BW80 per event,surpass BW80, data will be divided*/
			if (currnet_data->u2DataCount > CSI_BW80_TONE_NUM) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI count %u\n", currnet_data->u2DataCount);
				goto out;
			}

			break;
		case CSI_EVENT_CSI_I_DATA:

			if (prCSITlvData->body_len != sizeof(INT_16) * currnet_data->u2DataCount) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI I data len %u, csinum %u\n",
					prCSITlvData->body_len, currnet_data->u2DataCount);
				goto out;
			}

			pru2Tmp = (INT_16 *)prCSITlvData->aucbody;
			for (i2Idx = 0; i2Idx < currnet_data->u2DataCount; i2Idx++)
				currnet_data->ac2IData[i2Idx] = le2cpu16(*(pru2Tmp + i2Idx));
			break;
		case CSI_EVENT_CSI_Q_DATA:

			if (prCSITlvData->body_len != sizeof(INT_16) * currnet_data->u2DataCount) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CSI Q data len %u, csinum %u\n",
					prCSITlvData->body_len, currnet_data->u2DataCount);
				goto out;
			}

			pru2Tmp = (INT_16 *)prCSITlvData->aucbody;
			for (i2Idx = 0; i2Idx < currnet_data->u2DataCount; i2Idx++)
				currnet_data->ac2QData[i2Idx] = le2cpu16(*(pru2Tmp + i2Idx));
			break;
		case CSI_EVENT_DBW:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid DBW len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucDataBw = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_CH_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid CH IDX len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucPrimaryChIdx = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TA:
			/*
			 * TA length is 8-byte long (MAC addr 6 bytes +
			 * 2 bytes padding), the 2-byte padding keeps
			 * the next Tag at a 4-byte aligned address.
			 */
			if (prCSITlvData->body_len != NBytesAlign(sizeof(currnet_data->aucTA), 4)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TA len %u", prCSITlvData->body_len);
				goto out;
			}
			os_move_mem(currnet_data->aucTA, prCSITlvData->aucbody, sizeof(currnet_data->aucTA));
			break;
		case CSI_EVENT_EXTRA_INFO:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Error len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->u4ExtraInfo = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RX_MODE:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Rx Mode len %u", prCSITlvData->body_len);
				goto out;
			}
			rx_info = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			currnet_data->ucRxMode = GET_CSI_RX_MODE(rx_info);
			currnet_data->rx_rate = GET_CSI_RATE(rx_info);
			break;
		case CSI_EVENT_RSVD1:
			if (prCSITlvData->body_len > sizeof(INT_32) * CSI_MAX_RSVD1_COUNT) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD1 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRsvd1Cnt = prCSITlvData->body_len / sizeof(INT_32);
			p32tmp = (INT_32 *)(prCSITlvData->aucbody);
			for (i2Idx = 0; i2Idx < currnet_data->ucRsvd1Cnt; i2Idx++)
				currnet_data->ai4Rsvd1[i2Idx] = le2cpu32(*(p32tmp + i2Idx));
			break;
		case CSI_EVENT_RSVD2:
			if (prCSITlvData->body_len > sizeof(INT_32) * CSI_MAX_RSVD2_COUNT) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD2 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRsvd2Cnt = prCSITlvData->body_len / sizeof(INT_32);
			p32tmp = (INT_32 *)(prCSITlvData->aucbody);
			for (i2Idx = 0; i2Idx < currnet_data->ucRsvd2Cnt; i2Idx++)
				currnet_data->au4Rsvd2[i2Idx] = le2cpu32(*(p32tmp + i2Idx));
			break;
		case CSI_EVENT_RSVD3:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD3 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->i4Rsvd3 = le2cpu32(*((INT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_RSVD4:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid RSVD4 len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRsvd4 = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_H_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid chain_info len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->chain_info = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TX_RX_IDX:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TX_RX_IDX len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->Tx_Rx_Idx = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TS:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TS len %u", prCSITlvData->body_len);
				goto out;
			}
			currnet_data->u4TimeStamp = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_PKT_SN:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Segment Number len %u",
					prCSITlvData->body_len);
				goto out;
			}

			currnet_data->pkt_sn = (UINT_16)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_BW_SEG:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Segment Number len %u",
					prCSITlvData->body_len);
				goto out;
			}

			currnet_data->u4SegmentNum = le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_REMAIN_LAST:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid Remain Last len %u",
					prCSITlvData->body_len);
				goto out;
			}
			currnet_data->ucRemainLast = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;
		case CSI_EVENT_TR_STREAM:
			if (prCSITlvData->body_len != sizeof(UINT_32)) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
					"Invalid TR stream number len %u",
					prCSITlvData->body_len);
				goto out;
			}
			currnet_data->tr_stream = (UINT8)le2cpu32(*((UINT_32 *)prCSITlvData->aucbody));
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_INFO,
				"Unsupported CSI tag %d\n", prCSITlvData->tag_type);
		};

			Length -= (u2Offset + prCSITlvData->body_len);

			if (Length >= u2Offset)
				prBuf += (u2Offset + prCSITlvData->body_len);
	}

	/*if protocal filter is open, filter pkt*/
	if (prCSIInfo->protocol_filter &&
		!(prCSIInfo->protocol_filter & currnet_data->ucRxMode))
		goto out;

	ret = wlanCheckCSISegmentData(pAd, currnet_data);

	/*event data error or event drop*/
	if (ret == CSI_CHAIN_ERR || ret == CSI_CHAIN_SEGMENT_ERR) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"CSI chain check error! Segment number=%d, Remain last=%d\n",
			currnet_data->u4SegmentNum, currnet_data->ucRemainLast);
		goto out;
	}

	if (ret == CSI_CHAIN_SEGMENT_FIRST || ret == CSI_CHAIN_SEGMENT_MIDDLE)
		goto out;
	else if (ret == CSI_CHAIN_COMPLETE)
		final_data = prepare_csi_data_to_push(pAd, currnet_data);
	else if (ret == CSI_CHAIN_SEGMENT_LAST) /*if all segments have been combined*/
		final_data = prepare_csi_data_to_push(pAd, &prCSIInfo->rCSISegmentTemp);

	if (!final_data)
		goto out;

	if (prCSIInfo->usr_offset)
		csi_timestamp_filter(pAd, final_data);	/*csi sw timestamp filter*/
	else
		wlanPushCSIData(pAd, final_data);

	if (prCSIInfo->CSI_report_mode == CSI_PROC)
		wake_up_interruptible(&(pAd->rCSIInfo.waitq));

out:
	if (currnet_data)
		os_free_mem(currnet_data);

	if (final_data)
		os_free_mem(final_data);

	return;
}

VOID UniEventCSIHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_CSI_T);
	struct _RTMP_ADAPTER *pAd = NULL;
	struct UNI_EVENT_CSI_T *pEventHdr = (struct UNI_EVENT_CSI_T *)pData;

	if (pEventHdr->band_idx > BAND2) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
			"invalid band idx = %d\n", pEventHdr->band_idx);
		return;
	}

	pAd = physical_device_get_mac_adapter_by_band(physical_dev, pEventHdr->band_idx);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_CSI_DATA:
			UniEventCSIDataProc(pAd, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"invalid event tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}
#endif /* CFG_SUPPORT_CSI */
#ifdef DOT11_EHT_BE
#ifdef WAPP_SUPPORT
static void wapp_send_dscb_bitmap_info(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	u16 new_dscb_bitmap)
{
	struct wapp_event event;
	UINT8 eht_bw, prim_ch;
	UCHAR ch_band, eht_cen_ch;
	UINT8 eht_tx_nss, eht_rx_nss, eht_max_mcs;
	struct eht_txrx_mcs_nss eht_mcs_nss = {0};
	struct eht_txrx_mcs_nss_20 eht_mcs_nss_bw20 = {0};

	event.event_id = WAPP_EHT_OP_PUNCTURED_BITMAP;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event.data.eht_op_info.interface_index = wdev->wdev_idx;
	event.data.eht_op_info.eht_operation_information_valid = 1;
	event.data.eht_op_info.eht_default_pe_duration = 1;
	event.data.eht_op_info.group_addressed_BU_indication_limit = 1;
	event.data.eht_op_info.group_addressed_BU_indication_exponent = 1;

	prim_ch = wdev->channel;
	eht_bw = wlan_config_get_eht_bw(wdev);
	ch_band = wlan_config_get_ch_band(wdev);
	eht_cen_ch = eht_cent_ch_freq(wdev, wdev->channel, eht_bw, ch_band);
	event.data.eht_op_info.control = eht_bw;

	eht_tx_nss = wlan_config_get_eht_tx_nss(wdev);
	eht_rx_nss = wlan_config_get_eht_rx_nss(wdev);
	eht_max_mcs = wlan_config_get_eht_max_mcs(wdev);

	if (eht_max_mcs == MCS_0)
		eht_max_mcs = MCS_13;

	if (eht_bw == EHT_BW_20) {
		if (wdev->wdev_type == WDEV_TYPE_STA) {
			eht_mcs_map_bw20(eht_tx_nss, eht_rx_nss, eht_max_mcs, &eht_mcs_nss_bw20);
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs0_7_nss = eht_mcs_nss_bw20.max_tx_rx_mcs0_7_nss;
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs8_9_nss = eht_mcs_nss_bw20.max_tx_rx_mcs8_9_nss;
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs10_11_nss = eht_mcs_nss_bw20.max_tx_rx_mcs10_11_nss;
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs12_13_nss = eht_mcs_nss_bw20.max_tx_rx_mcs12_13_nss;
		} else {
			eht_mcs_map(eht_tx_nss, eht_rx_nss, eht_max_mcs, &eht_mcs_nss);
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs0_7_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs8_9_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs10_11_nss = eht_mcs_nss.max_tx_rx_mcs10_11_nss;
			event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs12_13_nss = eht_mcs_nss.max_tx_rx_mcs12_13_nss;
		}
	} else {
		eht_mcs_map(eht_tx_nss, eht_rx_nss, eht_max_mcs, &eht_mcs_nss);
		event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs0_7_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
		event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs8_9_nss = eht_mcs_nss.max_tx_rx_mcs0_9_nss;
		event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs10_11_nss = eht_mcs_nss.max_tx_rx_mcs10_11_nss;
		event.data.eht_op_info.eht_mcs_nss_set.max_tx_rx_mcs12_13_nss = eht_mcs_nss.max_tx_rx_mcs12_13_nss;
	}

	switch (event.data.eht_op_info.control) {
	case EHT_BW_320:
		event.data.eht_op_info.ccfs0 = GET_BW320_PRIM160_CENT(prim_ch, eht_cen_ch);
		event.data.eht_op_info.ccfs1 = eht_cen_ch;
		break;
	case EHT_BW_160:
		event.data.eht_op_info.ccfs0 = GET_BW160_PRIM80_CENT(prim_ch, eht_cen_ch);
		event.data.eht_op_info.ccfs1 = eht_cen_ch;
		break;
	case EHT_BW_80:
		event.data.eht_op_info.ccfs0 = eht_cen_ch;
		event.data.eht_op_info.ccfs1 = 0;
		break;
	case EHT_BW_2040:
		event.data.eht_op_info.ccfs0 = eht_cen_ch;
		event.data.eht_op_info.ccfs1 = 0;
		break;
	default:
		event.data.eht_op_info.ccfs0 = eht_cen_ch;
		event.data.eht_op_info.ccfs1 = 0;
	}
	event.data.eht_op_info.dscb_bitmap = new_dscb_bitmap;
	hex_dump_with_lvl("eht op punctured info sent from driver is", (UCHAR *)&event.data.eht_op_info,
						sizeof(struct wapp_eht_operations_info), DBG_LVL_INFO);
	wapp_send_wapp_qry_rsp(pAd, &event);
}
#endif

#ifdef ZERO_PKT_LOSS_SUPPORT
VOID UniEventSCSAHandler(
	VOID *physical_dev,
	UINT8  *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_SCSA_T);
	struct _RTMP_ADAPTER *pAd = NULL;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SCSA_BCN_TXOK_STATUS:
			UniEventBcnTxStatus(physical_dev, (void *)tag);
			break;
		default:
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CSI, DBG_LVL_ERROR,
				"invalid event tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}
#endif /*ZERO_PKT_LOSS_SUPPORT*/

static BOOLEAN set_cur_chn_bw_to_csa_info(struct wifi_dev *wdev)
{
	struct freq_oper oper_dev;
	struct DOT11_H *pDot11h = wdev->pDot11_H;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE, "\n");

	if (!pDot11h)
		return FALSE;

	os_zero_mem(&oper_dev, sizeof(oper_dev));
	if (hc_radio_query_by_wdev(wdev, &oper_dev) != HC_STATUS_OK) {
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "hc_radio_query_by_wdev failed!");
		return FALSE;
	}

	pDot11h->csa_chn_info.new_bw = oper_dev.bw;
	pDot11h->csa_chn_info.new_ch = oper_dev.prim_ch;
#ifdef DOT11_EHT_BE
	if (oper_dev.bw == BW_320)
		pDot11h->csa_chn_info.new_cench1 = oper_dev.eht_cen_ch;
	else
#endif
		pDot11h->csa_chn_info.new_cench1 = oper_dev.cen_ch_1;
	pDot11h->csa_chn_info.new_cench2 = oper_dev.cen_ch_2;

	MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_NOTICE,
		"ch:%d, bw:%d, cen_ch1:%d, cen_ch2:%d\n",
		pDot11h->csa_chn_info.new_ch, pDot11h->csa_chn_info.new_bw,
		pDot11h->csa_chn_info.new_cench1, pDot11h->csa_chn_info.new_cench2);

	return TRUE;
}

static VOID UniEventDscbProc(
	struct _RTMP_ADAPTER *pAd,
	struct UNI_EVENT_STATIC_PP_DSCB_T *TlvData)
{
	struct UNI_EVENT_STATIC_PP_DSCB_T *pDscb = NULL;
	struct wifi_dev *wdev = NULL;
	RTMP_ADAPTER *ad = NULL;
	UINT8 band_idx = 0;
	UINT8 omac_idx = 0;
	UINT8 new_dscb_enable = 0;
	UINT8 pre_dscb_enable = 0;
	UINT16 new_dscb_bitmap = 0;
	UINT16 pre_dscb_bitmap = 0;
	BOOLEAN need_update = FALSE;
	struct DOT11_H *pDot11h = NULL;
	BCN_UPDATE_REASON reason = 0;
	enum BCN_BPCC_IE_TYPE ie_type = 0;

	if (!TlvData) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}

	pDscb = (struct UNI_EVENT_STATIC_PP_DSCB_T *)TlvData;
	if (!pDscb) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error!!! pDscb is NULL!!!\n");
		return;
	}

	band_idx = pDscb->u1BandIdx;
	omac_idx = pDscb->u1OmacIdx;

	if (band_idx < PD_GET_BAND_NUM(pAd->physical_dev))
		ad = physical_device_get_mac_adapter_by_band(pAd->physical_dev, band_idx);
	if (!ad) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"invalid band_idx = %d\n", band_idx);
		return;
	}

	wdev = wdev_search_by_omac_idx(ad, omac_idx);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error! Can not get wdev by omac_idx(%d)!\n", omac_idx);
		return;
	}

	new_dscb_bitmap = le2cpu16(pDscb->u2DscbBitmap);
	pre_dscb_bitmap = wlan_config_get_eht_dis_subch_bitmap(wdev);
	new_dscb_enable = pDscb->fgIsDscbEnable;
	pre_dscb_enable = wlan_config_get_eht_dscb_enable(wdev);
	if ((new_dscb_bitmap != pre_dscb_bitmap) || (new_dscb_enable != pre_dscb_enable))
		need_update = TRUE;

	pDot11h = wdev->pDot11_H;
	if (need_update == TRUE) {
		if (pDscb->u2Tag == UNI_EVENT_STATIC_PP_TAG_DSCB_IE) {
			wlan_config_set_eht_dscb_enable(wdev, new_dscb_enable);
			wlan_config_set_eht_csa_dscb_enable(wdev, FALSE);
			wlan_config_set_eht_dis_subch_bitmap(wdev, new_dscb_bitmap);
			reason = BCN_REASON(BCN_UPDATE_IE_CHG);
			ie_type = BCN_BPCC_EHTOP;
		} else if (pDscb->u2Tag == UNI_EVENT_STATIC_PP_TAG_CSA_DSCB_IE) {
			set_cur_chn_bw_to_csa_info(wdev);
			wlan_config_set_eht_csa_dscb_enable(wdev, TRUE);
			wlan_config_set_eht_csa_dis_subch_bitmap(wdev, new_dscb_bitmap);
			reason = BCN_REASON(BCN_UPDATE_CSA);
			ie_type = BCN_BPCC_CSA;
		}
		if (bcn_bpcc_op_lock(ad, wdev, TRUE, BCN_BPCC_EHTOP) == FALSE) {
			MTWF_DBG(ad, DBG_CAT_AP, CATAP_BPCC, DBG_LVL_INFO,
				"bcn_bpcc_op_lock fail\n");
			if (in_interrupt())
				bcn_bpcc_ct_switch(
					ad, wdev, ie_type, (UINT32)reason);
			else
				UpdateBeaconHandler_BPCC(
					ad, wdev, reason, ie_type, TRUE);
		} else
			UpdateBeaconHandler_BPCC(
				ad, wdev, reason, ie_type, TRUE);
		if (wlan_config_get_eht_csa_dscb_enable(wdev)) {
			RTMPSetTimer(&pDot11h->PPCSAEventTimer, 4000);
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
				"PPCSAEventTimer start\n");
		}
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"(%s)Receive PP event: dscb_enable=%d, csa_dscb_enable=%d, dscb=%x, csa_dscb=%x\n",
			RtmpOsGetNetDevName(wdev->if_dev),
			wlan_config_get_eht_dscb_enable(wdev),
			wlan_config_get_eht_csa_dscb_enable(wdev),
			wlan_config_get_eht_dis_subch_bitmap(wdev),
			wlan_config_get_eht_csa_dis_subch_bitmap(wdev));
#ifdef WAPP_SUPPORT
	/* Only send indication for primary interface*/
	if (wdev->wdev_idx == 0)
		wapp_send_dscb_bitmap_info(pAd, wdev, new_dscb_bitmap);
#endif
	}
}

static void UniEventPpInfo(
	struct _RTMP_ADAPTER *pAd,
	struct UNI_EVENT_PP_ALG_CTRL_T *TlvData)
{
	struct UNI_EVENT_PP_ALG_CTRL_T *tag = NULL;

	if (!TlvData) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}

	tag = (struct UNI_EVENT_PP_ALG_CTRL_T *)TlvData;

	MTWF_PRINT("%s: PpBitmap = 0x%x\n", __func__, tag->u2SwPpBitmap);
	MTWF_PRINT("%s: OpPpBitmap = 0x%x\n", __func__, tag->u2OpPpBitmap);
	MTWF_PRINT("%s: ApStaticPpBitmap = 0x%x\n", __func__, tag->u2ApStaticPpBitmap);
	MTWF_PRINT("%s: TxFBwCnt = %lld\n", __func__, tag->u8TxFBwCnt);
	MTWF_PRINT("%s: TxAutoBwCnt = %lld\n", __func__, tag->u8TxAutoBwCnt);
	MTWF_PRINT("%s: TxPpCnt = %lld\n", __func__, tag->u8TxPpCnt);
}

static void UniEventPpStat(
	struct _RTMP_ADAPTER *pAd,
	struct UNI_EVENT_PP_ALG_CTRL_T *TlvData)
{
	struct UNI_EVENT_PP_ALG_CTRL_T *tag = NULL;

	if (!TlvData) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}

	tag = (struct UNI_EVENT_PP_ALG_CTRL_T *)TlvData;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE, "\n");
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m ============= Pp Band%d Stat ============= \x1b[m\n",
		tag->u1DbdcIdx);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpTimerIntv    = %d \x1b[m\n",
		tag->u4PpTimerIntv);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX2: Value = %d \x1b[m\n",
		tag->u4ThrX2_Value);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX2: Shift = %d \x1b[m\n",
		tag->u4ThrX2_Shift);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX3: Value = %d \x1b[m\n",
		tag->u4ThrX3_Value);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX3: Shift = %d \x1b[m\n",
		tag->u4ThrX3_Shift);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX4: Value = %d \x1b[m\n",
		tag->u4ThrX4_Value);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX4: Shift = %d \x1b[m\n",
		tag->u4ThrX4_Shift);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX5: Value = %d \x1b[m\n",
		tag->u4ThrX5_Value);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX5: Shift = %d \x1b[m\n",
		tag->u4ThrX5_Shift);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX6: Value = %d \x1b[m\n",
		tag->u4ThrX6_Value);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX6: Shift = %d \x1b[m\n",
		tag->u4ThrX6_Shift);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX7: Value = %d \x1b[m\n",
		tag->u4ThrX7_Value);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX7: Shift = %d \x1b[m\n",
		tag->u4ThrX7_Shift);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX8: Value = %d \x1b[m\n",
		tag->u4ThrX8_Value);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4PpThrX8: Shift = %d \x1b[m\n",
		tag->u4ThrX8_Shift);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4SwPpTime   = %d (Unit: %d ms)\x1b[m\n",
		tag->u4SwPpTime,
		tag->u4PpTimerIntv);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4HwPpTime   = %d (Unit: %d ms)\x1b[m\n",
		tag->u4HwPpTime,
		tag->u4PpTimerIntv);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4NoPpTime   = %d (Unit: %d ms)\x1b[m\n",
		tag->u4NoPpTime,
		tag->u4PpTimerIntv);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u4AutoBwTime = %d (Unit: %d ms)\x1b[m\n",
		tag->u4AutoBwTime,
		tag->u4PpTimerIntv);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m u2SwPpBitmap: Value = 0x%x \x1b[m\n",
		tag->u2SwPpBitmap);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_NOTICE,
		"\x1b[32m ========================================== \x1b[m\n");
}

VOID UniEventStaticPpHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_DSCB_T);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_INFO,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_STATIC_PP_TAG_DSCB_IE:
		case UNI_EVENT_STATIC_PP_TAG_CSA_DSCB_IE:
			UniEventDscbProc(pAd, (void *)tag);
			break;

		case UNI_EVENT_PP_TAG_ALG_CTRL:
			UniEventPpStat(pAd, (void *)tag);
			break;

		case UNI_EVENT_PP_SHOW_INFO:
			UniEventPpInfo(pAd, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
				"invalid event tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

#ifdef FTM_SUPPORT
static void UniEventLocDone(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _UNI_EVENT_LOC_DONE_T *TlvData,
	UINT16 wcid)
{
	struct _UNI_EVENT_LOC_DONE_T *tag = NULL;
	void *pPeerInfoArray;
	UINT8 ind = 0;
	struct _FTM_PEER_INFO *pPeerInfo;
	struct _MAC_TABLE_ENTRY *pEntry = NULL;
	FTM_PEER_ENTRY *pFTMEntry;

	if (!TlvData) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}

	tag = (struct _UNI_EVENT_LOC_DONE_T *)TlvData;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"u2Tag=%hd, u2Length=%hd\n", tag->u2Tag, tag->u2Length);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"ucSeqNum=%hhd\n", tag->ucSeqNum);

	/* get pEntry */
	pEntry = entry_get(pAd, wcid);
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"pEntry of wcid=%hu does not exist.\n", wcid);
		return;
	}

	pEntry->sta_force_keep = FALSE;
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"Get pEntry of "MACSTR", wcid = %d\n", MAC2STR(pEntry->Addr), pEntry->wcid);

	pPeerInfoArray = &wdev->FtmCtrl.iSTA_pinfo;
	for (ind = 1; ind < MAX_FTM_TBL_SIZE; ind++) {
		pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + ind;
		if (MAC_ADDR_EQUAL(pEntry->Addr, pPeerInfo->Responder)) {
			if (pPeerInfo->create_STAREC == 1) {
				mac_entry_delete(pAd, pEntry, TRUE);
				pPeerInfo->create_STAREC = 0;
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
					"Delete pEntry of "MACSTR"\n", MAC2STR(pPeerInfo->Responder));
			}
			return;
		}
	}

	pFTMEntry = FtmEntrySearch(pAd, wdev, pEntry->Addr);

	if (pFTMEntry == NULL) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_WARN,
			"can't find pFTMEntry of "MACSTR"\n", MAC2STR(pEntry->Addr));
		return;
	}
	pFTMEntry->State = FTMPEER_UNUSED;

}

static void UniEventLocResult(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _UNI_EVENT_LOC_RESULT_T *TlvData)
{
	struct _UNI_EVENT_LOC_RESULT_T *tag = NULL;
	void *pPeerInfoArray;
	UINT8 ind = 0;
	struct _FTM_PEER_INFO *pPeerInfo;
	UINT8 *pAddr;
	BSS_TABLE *ScanTab;

	if (!TlvData) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_EHT, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}

	tag = (struct _UNI_EVENT_LOC_RESULT_T *)TlvData;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"u2Tag=%hd\n", tag->u2Tag);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"u2Tag=%hd\n", tag->u2Length);
	pAddr = (UCHAR *)(&tag->aucMacAddr);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"aucMacAddr=%02x:%02x:%02x:%02x:%02x:%02x\n",
			pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"u4BurstNum=%d\n", tag->u4BurstNum);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"u4MeasurementNumber=%u\n", tag->u4MeasurementNumber);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"u4SuccessNumber=%u\n", tag->u4SuccessNumber);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"ucNumPerBurstPeer=%hhu\n", tag->ucNumPerBurstPeer);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
		"eStatus=%hhd\n", tag->eStatus);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"ucRetryAfterDuration=%hhu\n", tag->ucRetryAfterDuration);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"i4DistanceMM=%d mm\n", tag->i4DistanceMM);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"i8Ts=%ld\n", tag->i8Ts);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"i4BurstDuration=%d\n", tag->i4BurstDuration);


	pPeerInfoArray = &wdev->FtmCtrl.iSTA_pinfo;
	for (ind = 1; ind < MAX_FTM_TBL_SIZE; ind++) {
		pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + ind;
		if (NdisCmpMemory(tag->aucMacAddr, pPeerInfo->Responder, MAC_ADDR_LEN) == 0) {

			/* Delete not exist AP in ScanTab */
			if (tag->eStatus == LOC_STATUS_FTMR_TX_FAIL) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_NOTICE,
					"Delete MacAddr=%02x:%02x:%02x:%02x:%02x:%02x in scan table.\n",
					pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]);

				ScanTab = get_scan_tab_by_wdev(pAd, wdev);
				if (ScanTab == NULL) {
					MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
								"ScanTab is null(%p).\n", ScanTab);
					return;
				}

				BssTableDeleteEntry(ScanTab, pAddr, pPeerInfo->target_channel);
			}

			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
				"ori_channel=%hhu, target_channel=%hhu\n",
				pPeerInfo->ori_channel, pPeerInfo->target_channel);

			pPeerInfo->distanceMM = tag->i4DistanceMM;
			wdev->FtmCtrl.ista_work_cand--;

			pPeerInfo->status = FTMISTA_WAIT_SWITCH_BACK;
			FtmSwitchChannel(pAd, wdev, pPeerInfo->ori_channel, ACTION_SWITCH_BACK);
			pPeerInfo->status = FTMISTA_UNUSED;

			return;
		}
	}
}

static VOID UniEventLocTrigger(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _UNI_EVENT_LOC_TRIGGER_T *TlvData)
{
	struct _UNI_EVENT_LOC_TRIGGER_T *locTrigger;
	UINT8 *pAddr;

	if (!TlvData) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}

	locTrigger = (struct _UNI_EVENT_LOC_TRIGGER_T *)TlvData;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
		"u2Tag=%hd, u2Length=%hd\n", locTrigger->u2Tag, locTrigger->u2Length);

	pAddr = (UCHAR *)(&locTrigger->aucMacAddr);
	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"aucMacAddr=%02x:%02x:%02x:%02x:%02x:%02x\n",
			pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"eLocType=%d\n", locTrigger->eLocType);

}

static VOID UniEventLocRangeReqMCBurst(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	struct _UNI_EVENT_LOC_TRIGGER_T *TlvData)
{
	struct _UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST_T *pLocMCBurst;
	struct _MAC_TABLE_ENTRY *pEntry = NULL;
	VOID *pAddr;
	void *pPeerInfoArray;
	UINT8 ind = 0;
	struct _FTM_PEER_INFO *pPeerInfo;

	if (!TlvData) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}

	pLocMCBurst = (struct _UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST_T *)TlvData;

	/* get pEntry */
	pAddr = (UCHAR *)(&pLocMCBurst->aucAddr);
	pEntry = MacTableLookup(pAd, pAddr);
	if (!pEntry) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"Can't find pEntry of "MACSTR", notify FW\n", MAC2STR(pAddr));
		// TBD: notify FW
	} else {
		// check FTM state

		// get peerInfo, switch channel
		pPeerInfoArray = &wdev->FtmCtrl.iSTA_pinfo;
		for (ind = 1; ind < MAX_FTM_TBL_SIZE; ind++) {
			pPeerInfo = (struct _FTM_PEER_INFO *)pPeerInfoArray + ind;
			if (NdisCmpMemory(pAddr, pPeerInfo->Responder, MAC_ADDR_LEN) == 0) {
				MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
					"ori_channel=%hhu, target_channel=%hhu\n",
					pPeerInfo->ori_channel, pPeerInfo->target_channel);

				// change channel
				pPeerInfo->status = FTMISTA_IN_BURST;
				FtmSwitchChannel(pAd, wdev, pPeerInfo->target_channel, ACTION_GO_TO);

				wdev->FtmCtrl.ista_work_cand++;
				wdev->FtmCtrl.LastAssignedISTA = ind;

				return;
			}
		}
	}
}

VOID UniEventLocUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_ID_LOCATION_T);
	struct _UNI_EVENT_ID_LOCATION_T *pEventHdr = (struct _UNI_EVENT_ID_LOCATION_T *)pData;
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	pAd = physical_device_get_mac_adapter_by_band(physical_dev, pEventHdr->ucBandIndex);
	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"invalid band_idx = %hhu\n", pEventHdr->ucBandIndex);
		return;
	}

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_DEBUG,
		"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"band_idx = %hhu, ucWcid = %hu\n",
			pEventHdr->ucBandIndex, pEventHdr->ucWcid);

	wdev = wdev_search_by_wcid(pAd, pEventHdr->ucWcid);
	if (!wdev) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"can not find wdev, invalid ucWcid = %hu\n", pEventHdr->ucWcid);
		return;
	}

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_LOC_TAG_LOC_RESULT:
			UniEventLocResult(pAd, wdev, (void *)tag);
			break;

		case UNI_EVENT_LOC_TAG_LOC_DONE:
			UniEventLocDone(pAd, wdev, (void *)tag, pEventHdr->ucWcid);
			break;

		case UNI_EVENT_LOC_TAG_LOC_TRIGGER:
			UniEventLocTrigger(pAd, wdev, (void *)tag);
			break;

		case UNI_EVENT_LOC_TAG_RANGE_REQ_MC_BURST:
			UniEventLocRangeReqMCBurst(pAd, wdev, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"invalid event tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventLocCapa(
	struct _RTMP_ADAPTER *pAd,
	struct cmd_msg *msg,
	struct _UNI_EVENT_LOC_CAPA_T_T *TlvData)
{
	struct _UNI_EVENT_LOC_CAPA_T_T *eventLocCap;
	struct wifi_dev *wdev = NULL;
	struct _CMD_GET_LOC_CAP_T *pLocCap;

	wdev = ftm_get_last_wdev();
	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"Error!!! wdev is NULL!!!\n");
		return;
	}

	if (!TlvData) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
			"Error!!! TlvData is NULL!!!\n");
		return;
	}
	eventLocCap = (struct _UNI_EVENT_LOC_CAPA_T_T *)TlvData;

	pLocCap = &wdev->FtmCtrl.loc_cap;
	NdisCopyMemory(pLocCap, &eventLocCap->rCapabilities, sizeof(struct _CMD_GET_LOC_CAP_T));

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"eventLocCap->rCapabilities\n"
			"u4LOCInitSupported=%hu, u4ResponderSupported=%hu, u4LciSupport=%hhu, u4LcrSupport=%hhu\n"
			"u4PreambleSupport=%hu, u4BwSupport=%hu, u4AzBwSupport=%hu, u4MinDeltaTimePerPacket=%u",
			pLocCap->u4LOCInitSupported, pLocCap->u4ResponderSupported, pLocCap->u4LciSupport, pLocCap->u4LcrSupport,
			pLocCap->u4PreambleSupport, pLocCap->u4BwSupport, pLocCap->u4AzBwSupport, pLocCap->u4MinDeltaTimePerPacket);

	/* update min delta time */
	mtk_cfg80211_update_ftm_min_delta(pAd, wdev);
}

VOID UniEventLocHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_ID_LOCATION_T);
	struct _UNI_EVENT_ID_LOCATION_T *pEventHdr = (struct _UNI_EVENT_ID_LOCATION_T *)payload;
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)msg->priv;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"fixed_len=%d, tags_len=%d, tag=%hhd\n",
			fixed_len, tags_len, *tag);
	MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"ucBandIndex=%hhu, ucWcid=%hu",
			pEventHdr->ucBandIndex, pEventHdr->ucWcid);

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_INFO,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_LOC_TAG_LOC_CAPA:
			UniEventLocCapa(pAd, msg, (void *)tag);
			break;

		default:
			MTWF_DBG(pAd, DBG_CAT_PROTO, CATPROTO_FTM, DBG_LVL_ERROR,
				"invalid event tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}
#endif /* FTM_SUPPORT */

/**
 * @Unsolicited event handler of MLD AT2LM timeout
 *
 * @param *fixed event common part
 * @param *tlv event tlv
 */
VOID UniEventMldAt2lmTo(
	struct physical_device *physical_dev,
	struct UNI_EVENT_MLD_T *fixed,
	struct UNI_EVENT_MLD_AT2LM_TO_T *tlv)
{
	struct _RTMP_ADAPTER *pAd;
	struct AT2LM_TMR_TO_CTRL_T at2lm_tmr_to = {0};

	if (!fixed || !tlv) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
			"Error!!! fixed or tlv is NULL!!!\n");
		return;
	}

	/* TBD MLD-level feature, not bound to a band. find a pAd to do task */
	pAd = physical_device_get_first_up_mac_adapter(physical_dev);
	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"Error: pAd is NULL\n");
		return;
	}

	COPY_MAC_ADDR(at2lm_tmr_to.aucMldAddr, fixed->aucMldAddr);
	at2lm_tmr_to.ucFwMldIdx = fixed->ucFwMldIdx;
	at2lm_tmr_to.ucAt2lmId = tlv->ucAt2lmId;
	at2lm_tmr_to.ucType = tlv->ucType;

	RTEnqueueInternalCmd(pAd, CMDTHREAD_MLD_AT2LM_TIMEOUT,
		&at2lm_tmr_to, sizeof(struct AT2LM_TMR_TO_CTRL_T));
}

/**
 * @Unsolicited event handler of Reconfiguration ML IE - AP Removal Timer timeout
 *
 * @param *fixed event common part
 * @param *tlv event tlv
 */
VOID UniEventMldReconfigTo(
	struct physical_device *physical_dev,
	struct UNI_EVENT_MLD_T *fixed,
	struct UNI_EVENT_MLD_RECONFIG_TO_T *tlv)
{
	struct _RTMP_ADAPTER *pAd;
	struct RECONFIG_TMR_TO_CTRL_T reconfig_to_ctrl = {0};
	UINT8 link;

	if (!fixed || !tlv) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"Error!!! fixed or tlv is NULL!!!\n");
		return;
	}

	/* TBD MLD-level feature, not bound to a band. find a pAd to do task */
	pAd = physical_device_get_first_up_mac_adapter(physical_dev);
	if (!pAd) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"Error: pAd is NULL\n");
		return;
	}

	/* Parse Event fields */
	/* TODO: fixed->ucEventVer */
	COPY_MAC_ADDR(reconfig_to_ctrl.aucMldAddr, fixed->aucMldAddr);
	reconfig_to_ctrl.ucFwMldIdx = fixed->ucFwMldIdx;
	reconfig_to_ctrl.ucToLinkIdBmap = tlv->ucToLinkIdBmap;
	for (link = 0; link < CFG_WIFI_RAM_BAND_NUM; link++) {
		if (tlv->ucToLinkIdBmap & BIT(link))
			reconfig_to_ctrl.aucFwBssIdxLink[link] = tlv->aucFwBssIdxLink[link];
	}

	RTEnqueueInternalCmd(pAd, CMDTHREAD_MLD_RECONFIG_TIMEOUT,
		&reconfig_to_ctrl, sizeof(struct RECONFIG_TMR_TO_CTRL_T));
}

/**
 * @Unsolicited event handler of MLD
 *
 * @param *physical_dev physical_dev
 * @param *pData event common part
 * @param Length length of event body
 * @param *event_rxd event_rxd
 */
VOID UniEventMldUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT8 *fixed;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_MLD_T);

	fixed = pData;
	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_INFO,
			"Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MLD_AT2LM_TO:
			UniEventMldAt2lmTo(physical_dev, (void *)fixed, (void *)tag);
			break;
		case UNI_EVENT_MLD_RECONFIG_TO:
			UniEventMldReconfigTo(physical_dev, (void *)fixed, (void *)tag);
			break;
		default:
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_NOTICE,
				"invalid event tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}
#endif

VOID UniEventMloMldRec(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct MLD_RECORD_LINK_T *mldRecLink = NULL;
	struct UNI_EVENT_MLO_MLD_REC_T *prEventMloMldRec = (struct UNI_EVENT_MLO_MLD_REC_T *)Data;
	UINT8 linkIdx = 0, acIdx = 0;

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
		"MldRecState = %d, MldRecIdx = %d, MldIdx = %d\n"
		"PrimaryMldId = %d, u2SecondMldId = %d\n",
		prEventMloMldRec->rMldRec.u1MldRecState, prEventMloMldRec->rMldRec.u1MldRecIdx, prEventMloMldRec->rMldRec.u2StaRecMldIdx,
		prEventMloMldRec->rMldRec.u2PrimaryMldId, prEventMloMldRec->rMldRec.u2SecondMldId
		);

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
		"StrBmp = %d, EmlsrBmp = %d, ActiveLinkBmp = %d\n"
		"fgIsStr = %d, ActiveLinkNum = %d, SuspendLinkNum = %d\n",
		prEventMloMldRec->rMldRec.u1StrBmp, prEventMloMldRec->rMldRec.u1EmlsrBmp, prEventMloMldRec->rMldRec.u1ActiveLinkBmp,
		prEventMloMldRec->rMldRec.fgIsStr, prEventMloMldRec->rMldRec.u1ActiveLinkNum, prEventMloMldRec->rMldRec.u1SuspendLinkNum
		);

	MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
		"fgAgcAggressiveMode[0] = %d, fgAgcAggressiveMode[1] = %d, u1DesireSuspendBmp = %d,  u1PauseAllLinkCnt = %d\n",
		prEventMloMldRec->rMldRec.fgAgcAggressiveMode[0],
		prEventMloMldRec->rMldRec.fgAgcAggressiveMode[1],
		prEventMloMldRec->rMldRec.u1DesireSuspendBmp,
		prEventMloMldRec->rMldRec.u1PauseAllLinkCnt
		);

	for (linkIdx = 0; linkIdx < CFG_WIFI_RAM_BAND_NUM; linkIdx++) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
		"\n================== Link_%d ==================\n", linkIdx);

		mldRecLink = &(prEventMloMldRec->rMldRec.arMldRecLink[linkIdx]);

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"fgActive = %d, fgSuspend = %d, fgSuspendEmlsr = %d, fgTxLimit = %d, fgShareDispRatio = %d\n"
			"ParentMldRecIdx = %d, Band = %d, u1CmdRptTxFailCnt = %d, WlanIdx = %d, SuspendTs= %x\n",
			mldRecLink->fgActive, mldRecLink->fgSuspend, mldRecLink->fgSuspendEmlsr, mldRecLink->fgTxLimit, mldRecLink->fgShareDispRatio,
			mldRecLink->u1ParentMldRecIdx, mldRecLink->u1Band, mldRecLink->u1CmdRptTxFailCnt, mldRecLink->u2WlanIdx, mldRecLink->u4SuspendTs);

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"\n================== AgcDispParamTx ==================\n");

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"AgcStateTx = %d, DispRatioTx = %d\n"
			"DispOrderTx = %d, DispMgfTx = %d\n",
			mldRecLink->rAgcDispParamTx.u1AgcStateTx, mldRecLink->rAgcDispParamTx.u1DispRatioTx,
			mldRecLink->rAgcDispParamTx.u1DispOrderTx, mldRecLink->rAgcDispParamTx.u2DispMgfTx);

		for (acIdx = 0; acIdx < CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM; acIdx++) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"Pol Tx[%d] = %d\n",
				acIdx, mldRecLink->rAgcDispParamTx.u1DispPolTx[acIdx]);
		}
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"\n");
		for (acIdx = 0; acIdx < CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM; acIdx++) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"AC = %d : Corr1 = %d, Corr0 = %d, InCorr1 = %d u2InCorr0 = %d\n",
				acIdx, mldRecLink->arOvlpRptCntTx[acIdx].u2Corr1, mldRecLink->arOvlpRptCntTx[acIdx].u2Corr0,
				mldRecLink->arOvlpRptCntTx[acIdx].u2InCorr1, mldRecLink->arOvlpRptCntTx[acIdx].u2InCorr0);
		}
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"\n================== AgcDispParamTrig ==================\n");

		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"AgcStateTrig = %d, DispRatioTrig = %d\n"
			"DispMuLenTrig = %d, DispMgfTrig = %d\n",
			mldRecLink->rAgcDispParamTrig.u1AgcStateTrig, mldRecLink->rAgcDispParamTrig.u1DispRatioTrig,
			mldRecLink->rAgcDispParamTrig.u1DispMuLenTrig, mldRecLink->rAgcDispParamTrig.u2DispMgfTrig);

		for (acIdx = 0; acIdx < CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM; acIdx++) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"Pol Trig[%d] = %d\n",
				acIdx, mldRecLink->rAgcDispParamTrig.u1DispPolTrig[acIdx]);
		}
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
			"\n");

		for (acIdx = 0; acIdx < CONFIG_WIFI_RAM_MAX_MLO_MGMT_SUPPORT_AC_NUM; acIdx++) {
			MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_ERROR,
				"AC = %d : Corr1 = %d, Corr0 = %d, InCorr1 = %d u2InCorr0 = %d\n",
				acIdx, mldRecLink->arOvlpRptCntTrig[acIdx].u2Corr1, mldRecLink->arOvlpRptCntTrig[acIdx].u2Corr0,
				mldRecLink->arOvlpRptCntTrig[acIdx].u2InCorr1, mldRecLink->arOvlpRptCntTrig[acIdx].u2InCorr0);
		}

	}
}

VOID UniEventMloMldAlgoHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8  *tag;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_RA);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(pAd, DBG_CAT_MLO, CATMLO_CFG, DBG_LVL_WARN,
			"invalid length = %d\n", Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = (UINT8 *)pData + fixed_len;

	switch (*tag) {

	case UNI_EVENT_MLO_MLD_REC:
		UniEventMloMldRec(pAd, tag, tags_len);
		break;

	default:
		break;
	}

}

#ifdef TXRX_STAT_SUPPORT
#ifdef TXRX_STAT_RATE_CNT_DBG
#ifdef MT7992_SKU_BE3600SDB
static VOID uni_event_state_print_rate_cnt_2g(
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_2G_T *prEventRateCnt2G,
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T *Report2G
)
{
	UINT32 i, j;

	MTWF_PRINT("Report2G: Tag[%u],Len[%u],Band[%u],Dir[%u]\n",
			prEventRateCnt2G->u2Tag, prEventRateCnt2G->u2Length, prEventRateCnt2G->u1BandIdx, prEventRateCnt2G->u1Direction);
	MTWF_PRINT("CCK\n");
	for (i = 0; i < 1; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < CCK_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rCckCnt[j].SuccessCnt, Report2G->rCckCnt[j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("OFDM\n");
	for (i = 0; i < 1; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < OFDM_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rOfdmCnt[j].SuccessCnt, Report2G->rOfdmCnt[j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rHtCnt[i][j].SuccessCnt, Report2G->rHtCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HE\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HE_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rHeCnt[i][j].SuccessCnt, Report2G->rHeCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("EHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < EHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rEhtCnt[i][j].SuccessCnt, Report2G->rEhtCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

}

static VOID uni_event_state_print_rate_cnt_6g(
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_6G_T *prEventRateCnt6G,
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T *Report6G
)
{
	UINT32 i, j;

	MTWF_PRINT("Report6G: Tag[%u],Len[%u],Band[%u],Dir[%u]\n",
			prEventRateCnt6G->u2Tag, prEventRateCnt6G->u2Length, prEventRateCnt6G->u1BandIdx, prEventRateCnt6G->u1Direction);
	MTWF_PRINT("HE\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HE_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report6G->rHeCnt[i][j].SuccessCnt, Report6G->rHeCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("EHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < EHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report6G->rEhtCnt[i][j].SuccessCnt, Report6G->rEhtCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}
}

static VOID uni_event_state_print_rate_cnt_5g(
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_5G_T *prEventRateCnt5G,
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T *Report5G
)
{
	UINT32 i, j;

	MTWF_PRINT("Report5G: Tag[%u],Len[%u],Band[%u],Dir[%u]\n",
			prEventRateCnt5G->u2Tag, prEventRateCnt5G->u2Length, prEventRateCnt5G->u1BandIdx, prEventRateCnt5G->u1Direction);
	MTWF_PRINT("OFDM\n");
	for (i = 0; i < 1; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < OFDM_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rOfdmCnt[j].SuccessCnt, Report5G->rOfdmCnt[j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rHtCnt[i][j].SuccessCnt, Report5G->rHtCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("VHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < VHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rVhtCnt[i][j].SuccessCnt, Report5G->rVhtCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HE\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HE_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rHeCnt[i][j].SuccessCnt, Report5G->rHeCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("EHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < EHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rEhtCnt[i][j].SuccessCnt, Report5G->rEhtCnt[i][j].FailCnt);
		MTWF_PRINT("\n");
	}

}
#else
static VOID uni_event_state_print_rate_cnt_2g(
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_2G_T *prEventRateCnt2G,
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T *Report2G
)
{
	UINT32 i, j;

	MTWF_PRINT("Report2G: Tag[%u],Len[%u],Band[%u],Dir[%u]\n",
			prEventRateCnt2G->u2Tag, prEventRateCnt2G->u2Length, prEventRateCnt2G->u1BandIdx, prEventRateCnt2G->u1Direction);
	MTWF_PRINT("CCK\n");
	for (i = 0; i < 1; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < CCK_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rCckCnt[j].rRateCnt.SuccessCnt, Report2G->rCckCnt[j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("OFDM\n");
	for (i = 0; i < 1; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < OFDM_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rOfdmCnt[j].rRateCnt.SuccessCnt, Report2G->rOfdmCnt[j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rHtCnt[i][j].rRateCnt.SuccessCnt, Report2G->rHtCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HE\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HE_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rHeCnt[i][j].rRateCnt.SuccessCnt, Report2G->rHeCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("EHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < EHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report2G->rEhtCnt[i][j].rRateCnt.SuccessCnt, Report2G->rEhtCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

}

static VOID uni_event_state_print_rate_cnt_6g(
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_6G_T *prEventRateCnt6G,
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T *Report6G
)
{
	UINT32 i, j;

	MTWF_PRINT("Report6G: Tag[%u],Len[%u],Band[%u],Dir[%u]\n",
			prEventRateCnt6G->u2Tag, prEventRateCnt6G->u2Length, prEventRateCnt6G->u1BandIdx, prEventRateCnt6G->u1Direction);
	MTWF_PRINT("HE\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HE_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report6G->rHeCnt[i][j].rRateCnt.SuccessCnt, Report6G->rHeCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("EHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < EHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report6G->rEhtCnt[i][j].rRateCnt.SuccessCnt, Report6G->rEhtCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}
}

static VOID uni_event_state_print_rate_cnt_5g(
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_5G_T *prEventRateCnt5G,
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T *Report5G
)
{
	UINT32 i, j;

	MTWF_PRINT("Report5G: Tag[%u],Len[%u],Band[%u],Dir[%u]\n",
			prEventRateCnt5G->u2Tag, prEventRateCnt5G->u2Length, prEventRateCnt5G->u1BandIdx, prEventRateCnt5G->u1Direction);
	MTWF_PRINT("OFDM\n");
	for (i = 0; i < 1; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < OFDM_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rOfdmCnt[j].rRateCnt.SuccessCnt, Report5G->rOfdmCnt[j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rHtCnt[i][j].rRateCnt.SuccessCnt, Report5G->rHtCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("VHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < VHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rVhtCnt[i][j].rRateCnt.SuccessCnt, Report5G->rVhtCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("HE\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < HE_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rHeCnt[i][j].rRateCnt.SuccessCnt, Report5G->rHeCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

	MTWF_PRINT("EHT\n");
	for (i = 0; i < RA_NSS_MAX; i++) {
		MTWF_PRINT("\t%uSS ", i + 1);
		for (j = 0; j < EHT_RATE_MAX; j++)
			MTWF_PRINT(", [%u]%u/%u", j, Report5G->rEhtCnt[i][j].rRateCnt.SuccessCnt, Report5G->rEhtCnt[i][j].rRateCnt.FailCnt);
		MTWF_PRINT("\n");
	}

}
#endif /* MT7992_SKU_BE3600SDB */
#endif /* TXRX_STAT_RATE_CNT_DBG */

#ifdef MT7992_SKU_BE3600SDB
VOID rate_report_stat_update(struct RATE_CNT_T *prRateCnt, struct UNI_EVENT_RATE_CNT_T *prUnionRateCnt)
{
	prRateCnt->SuccessCnt += prUnionRateCnt->SuccessCnt;
	prRateCnt->FailCnt += prUnionRateCnt->FailCnt;
}

#else
VOID rate_report_stat_update(struct RATE_CNT_T *prRateCnt, union UNI_EVENT_UNION_RATE_CNT_T *prUnionRateCnt)
{
	prRateCnt->SuccessCnt += prUnionRateCnt->rRateCnt.SuccessCnt;
	prRateCnt->FailCnt += prUnionRateCnt->rRateCnt.FailCnt;
}
#endif /* MT7992_SKU_BE3600SDB */

VOID uni_event_state_all_rate_cnt_report(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	struct TXRX_RATE_UNI_CMD_PARAM_T *prTxRxRateUniParam = (struct TXRX_RATE_UNI_CMD_PARAM_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_STATE_T *event = (struct UNI_EVENT_STATE_T *)rsp_payload;
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T *Report2G = NULL;
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T *Report5G = NULL;
	struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T *Report6G = NULL;
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_2G_T *prEventRateCnt2G = NULL;
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_5G_T *prEventRateCnt5G = NULL;
	struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_6G_T *prEventRateCnt6G = NULL;
	struct TXRX_RATE_REPORT_2G_T *TxRxRateReprt2G = NULL;
	struct TXRX_RATE_REPORT_5G_T *TxRxRateReprt5G = NULL;
	struct TXRX_RATE_REPORT_6G_T *TxRxRateReprt6G = NULL;
	UINT8 ucBandIdx = prTxRxRateUniParam->ucBandIdx;
	enum ENUM_DIRECTION_T eDir = (enum ENUM_DIRECTION_T)prTxRxRateUniParam->eDir;
	USHORT PhyMode = prTxRxRateUniParam->PhyMode;
	UINT32 NssIndex, McsIndex;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"Band[%u],PhyMode[%u],Dir[%u]\n", ucBandIdx, PhyMode, eDir);

	if (WMODE_CAP_2G(PhyMode)) {
		os_alloc_mem(NULL, (UCHAR **)&Report2G,
			sizeof(struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T));
		if (Report2G == NULL) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_ERROR,
				"Alloc mem fail\n");
			goto Error;
		}

		prEventRateCnt2G = (struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_2G_T *) &(event->au1TlvBuffer[0]);

		os_move_mem(Report2G, &prEventRateCnt2G->rAllRateCnt2G, sizeof(struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_2G_T));

		TxRxRateReprt2G = &prTxRxRateUniParam->prTxRxRateReport->Report2G[eDir];
		for (NssIndex = 0; NssIndex < RA_NSS_MAX; NssIndex++) {
			for (McsIndex = 0; McsIndex < EHT_RATE_MAX; McsIndex++) {
				if (NssIndex == 0 && McsIndex < CCK_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt2G->rCckCnt[McsIndex],
									 &Report2G->rCckCnt[McsIndex]);
				}
				if (NssIndex == 0 && McsIndex < OFDM_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt2G->rOfdmCnt[McsIndex],
									&Report2G->rOfdmCnt[McsIndex]);
				}
				if (McsIndex < HT_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt2G->rHtCnt[NssIndex][McsIndex],
											&Report2G->rHtCnt[NssIndex][McsIndex]);
				}
				if (McsIndex < HE_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt2G->rHeCnt[NssIndex][McsIndex],
											&Report2G->rHeCnt[NssIndex][McsIndex]);
				}
				rate_report_stat_update(&TxRxRateReprt2G->rEhtCnt[NssIndex][McsIndex], &Report2G->rEhtCnt[NssIndex][McsIndex]);
			}
		}

#ifdef TXRX_STAT_RATE_CNT_DBG
		uni_event_state_print_rate_cnt_2g(prEventRateCnt2G, Report2G);
#endif /* TXRX_STAT_RATE_CNT_DBG */

	} else if (WMODE_CAP_6G(PhyMode)) {
		os_alloc_mem(NULL, (UCHAR **)&Report6G,
			sizeof(struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T));
		if (Report6G == NULL) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_ERROR,
				"Alloc mem fail\n");
			goto Error;
		}

		prEventRateCnt6G = (struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_6G_T *) &(event->au1TlvBuffer[0]);

		os_move_mem(Report6G, &prEventRateCnt6G->rAllRateCnt6G, sizeof(struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_6G_T));

		TxRxRateReprt6G = &prTxRxRateUniParam->prTxRxRateReport->Report6G[eDir];
		for (NssIndex = 0; NssIndex < RA_NSS_MAX; NssIndex++) {
			for (McsIndex = 0; McsIndex < EHT_RATE_MAX; McsIndex++) {
				if (McsIndex < HE_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt6G->rHeCnt[NssIndex][McsIndex],
											&Report6G->rHeCnt[NssIndex][McsIndex]);
				}
				rate_report_stat_update(&TxRxRateReprt6G->rEhtCnt[NssIndex][McsIndex], &Report6G->rEhtCnt[NssIndex][McsIndex]);
			}
		}

#ifdef TXRX_STAT_RATE_CNT_DBG
		uni_event_state_print_rate_cnt_6g(prEventRateCnt6G, Report6G);
#endif /* TXRX_STAT_RATE_CNT_DBG */

	} else if (WMODE_CAP_5G(PhyMode)) {
		os_alloc_mem(NULL, (UCHAR **)&Report5G,
			sizeof(struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T));
		if (Report5G == NULL) {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_ERROR,
				"Alloc mem fail\n");
			goto Error;
		}

		prEventRateCnt5G = (struct UNI_EVENT_STATE_ALL_TXRX_RATE_CNT_REPORT_5G_T *) &(event->au1TlvBuffer[0]);

		os_move_mem(Report5G, &prEventRateCnt5G->rAllRateCnt5G, sizeof(struct UNI_EVENT_STATE_ALL_RATE_CNT_REPORT_5G_T));

		TxRxRateReprt5G = &prTxRxRateUniParam->prTxRxRateReport->Report5G[eDir];
		for (NssIndex = 0; NssIndex < RA_NSS_MAX; NssIndex++) {
			for (McsIndex = 0; McsIndex < EHT_RATE_MAX; McsIndex++) {
				if (NssIndex == 0 && McsIndex < OFDM_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt5G->rOfdmCnt[McsIndex],
											&Report5G->rOfdmCnt[McsIndex]);
				}
				if (McsIndex < HT_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt5G->rHtCnt[NssIndex][McsIndex],
											&Report5G->rHtCnt[NssIndex][McsIndex]);
				}
				if (McsIndex < VHT_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt5G->rVhtCnt[NssIndex][McsIndex],
											&Report5G->rVhtCnt[NssIndex][McsIndex]);
				}

				if (McsIndex < HE_RATE_MAX) {
					rate_report_stat_update(&TxRxRateReprt5G->rHeCnt[NssIndex][McsIndex],
											&Report5G->rHeCnt[NssIndex][McsIndex]);
				}
				rate_report_stat_update(&TxRxRateReprt5G->rEhtCnt[NssIndex][McsIndex], &Report5G->rEhtCnt[NssIndex][McsIndex]);
			}
		}

#ifdef TXRX_STAT_RATE_CNT_DBG
		uni_event_state_print_rate_cnt_5g(prEventRateCnt5G, Report5G);
#endif /* TXRX_STAT_RATE_CNT_DBG */

	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_ERROR,
			"unknown PhyMode Band[%u],PhyMode[%u]\n", ucBandIdx, PhyMode);
	}

Error:
	if (Report2G)
		os_free_mem(Report2G);

	if (Report5G)
		os_free_mem(Report5G);

	if (Report6G)
		os_free_mem(Report6G);

}

VOID uni_event_state_stbc_cnt_report(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	struct TXRX_RATE_UNI_CMD_PARAM_T *prTxRxRateUniParam = (struct TXRX_RATE_UNI_CMD_PARAM_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_STATE_T *event = (struct UNI_EVENT_STATE_T *)rsp_payload;
	struct UNI_EVENT_STATE_STBC_CNT_REPORT_T *prEventSTBC;
	UINT8 ucBandIdx = prTxRxRateUniParam->ucBandIdx;
	UINT32 STBCCnt[ENUM_DIR_MAX];
	UINT32 i;

	prEventSTBC = (struct UNI_EVENT_STATE_STBC_CNT_REPORT_T *) &(event->au1TlvBuffer[0]);

	os_move_mem(&STBCCnt, &prEventSTBC->STBCCnt, sizeof(STBCCnt));

	for (i = 0; i < ENUM_DIR_MAX; i++) {
		prTxRxRateUniParam->prTxRxRateReport->STBCCnt[ucBandIdx][i] += STBCCnt[i];
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"Band[%u],Dir[%s], STBC: %u\n", ucBandIdx, (i == ENUM_DIR_TX) ? "TX" : "RX", STBCCnt[i]);
	}
}

VOID uni_event_state_gi_cnt_report(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	struct TXRX_RATE_UNI_CMD_PARAM_T *prTxRxRateUniParam = (struct TXRX_RATE_UNI_CMD_PARAM_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_STATE_T *event = (struct UNI_EVENT_STATE_T *)rsp_payload;
	struct UNI_EVENT_STATE_GI_CNT_REPORT_T *prEventGI;
	UINT8 ucBandIdx = prTxRxRateUniParam->ucBandIdx;
	UINT32 HeEhtGICnt[ENUM_DIR_MAX][ENUM_GI_MAX];
	UINT32 HtVhtGICnt[ENUM_DIR_MAX][ENUM_GI_MAX];
	UINT32 i;

	prEventGI = (struct UNI_EVENT_STATE_GI_CNT_REPORT_T *) &(event->au1TlvBuffer[0]);

	os_move_mem(&HeEhtGICnt, &prEventGI->HeEhtGICnt, sizeof(HeEhtGICnt));
	os_move_mem(&HtVhtGICnt, &prEventGI->HtVhtGICnt, sizeof(HtVhtGICnt));

	for (i = 0; i < ENUM_DIR_MAX; i++) {
		prTxRxRateUniParam->prTxRxRateReport->HtVhtGICnt[ucBandIdx][i][ENUM_SGI] += HtVhtGICnt[i][ENUM_SGI];
		prTxRxRateUniParam->prTxRxRateReport->HtVhtGICnt[ucBandIdx][i][ENUM_LGI] += HtVhtGICnt[i][ENUM_LGI];
		prTxRxRateUniParam->prTxRxRateReport->HeEhtGICnt[ucBandIdx][i][ENUM_SGI] += HeEhtGICnt[i][ENUM_SGI];
		prTxRxRateUniParam->prTxRxRateReport->HeEhtGICnt[ucBandIdx][i][ENUM_LGI] += HeEhtGICnt[i][ENUM_LGI];
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"Band[%u], Dir[%s], [HtVht] SGI: %u, LGI: %u\n",
							ucBandIdx, (i == ENUM_DIR_TX) ? "TX" : "RX", HtVhtGICnt[i][ENUM_SGI], HtVhtGICnt[i][ENUM_LGI]);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"Band[%u], Dir[%s], [HeEht] SGI: %u, LGI: %u\n",
							ucBandIdx, (i == ENUM_DIR_TX) ? "TX" : "RX", HeEhtGICnt[i][ENUM_SGI], HeEhtGICnt[i][ENUM_LGI]);
	}
}
#endif /* TXRX_STAT_SUPPORT */

VOID uni_event_state_cn_info_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	UINT16 *pcninfo = (UINT16 *) msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_STATE_T *event = (struct UNI_EVENT_STATE_T *)rsp_payload;
	struct UNI_EVENT_STATE_OFDMLQ_CNINFO_T *ptr = (struct UNI_EVENT_STATE_OFDMLQ_CNINFO_T *) &(event->au1TlvBuffer[0]);

	(*pcninfo) = ptr->u2OfdmLqCn;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"CnInfo: %d\n", *pcninfo);
}

VOID uni_event_state_total_tx_cnt_handler(struct cmd_msg *msg, char *rsp_payload,
						UINT16 rsp_payload_len)
{
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat = (P_EXT_EVENT_TX_STATISTIC_RESULT_T)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_STATE_T *event = (struct UNI_EVENT_STATE_T *)rsp_payload;
	struct UNI_EVENT_STATE_TOTAL_TX_CNT_T *prEventTotalTxCnt = (struct UNI_EVENT_STATE_TOTAL_TX_CNT_T *) &(event->au1TlvBuffer[0]);

	prTxStat->u4TotalTxCount =
		le2cpu32(prEventTotalTxCnt->u4TotalTxCount);
	prTxStat->u4TotalTxFailCount =
		le2cpu32(prEventTotalTxCnt->u4TotalTxFailCount);
	prTxStat->u4CurrBwTxCnt =
		le2cpu32(prEventTotalTxCnt->u4CurrBwTxCnt);
	prTxStat->u4OtherBwTxCnt =
		le2cpu32(prEventTotalTxCnt->u4OtherBwTxCnt);
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_TMAC, DBG_LVL_DEBUG,
		"TxCnt:%u TxFailCnt:%u CurrBwTxCnt:%u OtherBwTxCnt:%u\n",
		prTxStat->u4TotalTxCount, prTxStat->u4TotalTxFailCount, prTxStat->u4CurrBwTxCnt, prTxStat->u4OtherBwTxCnt);
}

VOID uni_event_state_last_tx_rate_handler(struct cmd_msg *msg, char *rsp_payload,
						UINT16 rsp_payload_len)
{
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat = (P_EXT_EVENT_TX_STATISTIC_RESULT_T)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_STATE_T *event = (struct UNI_EVENT_STATE_T *)rsp_payload;
	struct UNI_EVENT_STATE_LAST_TX_RATE_T *prEventLastTxRate = (struct UNI_EVENT_STATE_LAST_TX_RATE_T *) &(event->au1TlvBuffer[0]);

	os_move_mem(&prTxStat->rLastTxRate,
				&prEventLastTxRate->rLastTxRate, sizeof(RA_PHY_CFG_T));
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_TMAC, DBG_LVL_DEBUG,
		"LastTX Mode:%u Flag:%x STBC:%u GI:%u BW:%u ldpc:%u MCS:%u Nss:%u ltf:%u\n",
		prTxStat->rLastTxRate.MODE, prTxStat->rLastTxRate.Flags,
		prTxStat->rLastTxRate.STBC, prTxStat->rLastTxRate.ShortGI,
		prTxStat->rLastTxRate.BW, prTxStat->rLastTxRate.ldpc,
		prTxStat->rLastTxRate.MCS, prTxStat->rLastTxRate.VhtNss,
		prTxStat->rLastTxRate.he_ltf);
}

static VOID uni_event_state_tx_rate_info(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct UNI_EVENT_STATE_TX_RATE_T *prEventTxRate = NULL;
	UINT8 tx_rate = 0, tx_nsts = 0, tx_mode = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR tx_mode_str[20];

	prEventTxRate = (struct UNI_EVENT_STATE_TX_RATE_T *)Data;
	tx_rate = prEventTxRate->u1TxRate;
	tx_mode = prEventTxRate->u1TxMode;
	tx_nsts = prEventTxRate->u1TxNsts;
	os_move_mem(tx_mode_str, *(str + tx_mode), 20);

	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_TMAC, DBG_LVL_DEBUG,
				"tx rate: %s(%dNss M%d)\n", tx_mode_str, tx_nsts, tx_rate);
}

static VOID uni_event_state_rx_rate_info(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct UNI_EVENT_STATE_RX_RATE_T *prEventRxRate = NULL;
	UINT8 rx_rate = 0, rx_nsts = 0, rx_mode = 0;
	CHAR str[6][20] = {"CCK", "OFDM", "HT Mix-Mode", "HT Green-Field", "VHT", "HE"};
	CHAR rx_mode_str[20];

	prEventRxRate = (struct UNI_EVENT_STATE_RX_RATE_T *)Data;
	rx_rate = prEventRxRate->u1RxRate;
	rx_mode = prEventRxRate->u1RxMode;
	rx_nsts = prEventRxRate->u1RxNsts;
	os_move_mem(rx_mode_str, *(str + rx_mode), 20);

	MTWF_DBG(NULL, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG,
				"trigger-based\n");
	MTWF_DBG(NULL, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_DEBUG,
				"rx rate: %s(%dNss M%d)\n", rx_mode_str, rx_nsts, rx_rate);
}

static VOID uni_event_state_rx_stat_info(
	VOID *physical_dev,
	struct _RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT32 Length)
{
#if defined(EAP_STATS_SUPPORT) || defined(TXRX_STAT_SUPPORT)
	struct UNI_EVENT_STATE_RX_STAT_T *prEvent = (struct UNI_EVENT_STATE_RX_STAT_T *)Data;
	UINT32 Idx;
	UCHAR concurrent_bands = PD_GET_BAND_NUM(physical_dev);
	struct UNI_EVENT_STATE_RX_STAT_PER_BAND_T *prRxStatResult = NULL;

	for (Idx = 0; Idx < concurrent_bands; Idx++) {
		prRxStatResult = (struct UNI_EVENT_STATE_RX_STAT_PER_BAND_T *)&prEvent->arRxStatResult[Idx];
		pAd = physical_device_get_mac_adapter_by_band(physical_dev, Idx);

		if (pAd == NULL) {
			MTWF_DBG(NULL, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_INFO,
					"Cannot find corresponding pAd(Band%d)\n", Idx);
			continue;
		}

		MTWF_DBG(NULL, DBG_CAT_RX, CATRX_RXINFO, DBG_LVL_INFO,
					"band:%u PdCck:%u PdOfdm:%u MdCck:%u MdOfdm;%u\n",
					Idx, prRxStatResult->u2PhyRxPdCck, prRxStatResult->u2PhyRxPdOfdm,
					prRxStatResult->u2PhyRxMdrdyCntCck, prRxStatResult->u2PhyRxMdrdyCntOfdm);

		pAd->OneSecMibBucket.PdCount = le2cpu16(prRxStatResult->u2PhyRxPdCck) +
											le2cpu16(prRxStatResult->u2PhyRxPdOfdm);
		pAd->OneSecMibBucket.MdrdyCount = le2cpu16(prRxStatResult->u2PhyRxMdrdyCntCck) +
												le2cpu16(prRxStatResult->u2PhyRxMdrdyCntOfdm);
	}
#endif /* defined(EAP_STATS_SUPPORT) || defined(TXRX_STAT_SUPPORT) */
}

VOID UniEventStateUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct UNI_EVENT_STATE_T *event;
	struct UNI_EVENT_STATE_COMMON_T *tlv;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(*event);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	event = (struct UNI_EVENT_STATE_T *)pData;
	tlv = (struct UNI_EVENT_STATE_COMMON_T *) &(event->au1TlvBuffer[0]);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

	switch (tlv->u2Tag) {
	case UNI_EVENT_STATE_TX_RATE:
		uni_event_state_tx_rate_info(pAd, &(event->au1TlvBuffer[0]), tags_len);
		break;

	case UNI_EVENT_STATE_RX_RATE:
		uni_event_state_rx_rate_info(pAd, &(event->au1TlvBuffer[0]), tags_len);
		break;

	case UNI_EVENT_STATE_RX_STAT:
		uni_event_state_rx_stat_info(physical_dev, pAd, &(event->au1TlvBuffer[0]), tags_len);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag = %u\n", tlv->u2Tag);
		break;
	}
}

VOID uni_per_sta_info_multi_rssi_handler(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)Data;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_RSSI_T *ptr = (struct UNI_EVENT_PER_STA_RSSI_T *) &(tlv->au1PerStaContent[0]);
	P_RSSI_PAIR RssiPair = (P_RSSI_PAIR) msg->attr.rsp.wb_buf_in_calbk;
	UINT32 index, num;
	UINT8 i = 0, rcpi;
	CHAR rssi;

	num = (Len - sizeof(*tlv)) / sizeof(*ptr);

	for (index = 0; index < num; index++) {
		RssiPair->u2WlanIdx = ptr->u2WlanIdx;
#ifdef CFG_BIG_ENDIAN
		RssiPair->u2WlanIdx = le2cpu16(RssiPair->u2WlanIdx);
#endif
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"WlanIdx:%u\n",
			RssiPair->u2WlanIdx);

		for (i = 0; i < 4; i++) {
			rcpi = (ptr->au1Rcpi[i]);
			rssi = (rcpi - 220) / 2;

			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				"ant: %d, rcpi: %d, rssi: %d\n",
				i, rcpi, rssi);

			if (rssi > 0)
				RssiPair->rssi[i] = -127;
			else
				RssiPair->rssi[i] = rssi;
		}

		ptr++;
		RssiPair++;
	}
}

VOID uni_event_per_sta_multi_tx_rate_handler(struct cmd_msg *msg, char *payload, uint16_t payload_len)
{
	struct UNI_EVENT_PER_STA_INFO_T *prEvent = (struct UNI_EVENT_PER_STA_INFO_T *)payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *prEventTlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *)&(prEvent->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_TX_RATE_T *prEventTxRate;
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat = NULL;
	P_TX_STATISTIC_RESULT_PAIR prTxStatPair = (P_TX_STATISTIC_RESULT_PAIR) msg->attr.rsp.wb_buf_in_calbk;
	UINT32 index, num;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"u2Tag = %u Payloadlen:%u\n",
		prEventTlv->u2Tag, payload_len);

	prEventTxRate = (struct UNI_EVENT_PER_STA_TX_RATE_T *)&(prEventTlv->au1PerStaContent[0]);
	num = (payload_len - sizeof(*prEventTlv)) / sizeof(*prEventTxRate);

	for (index = 0; index < num; index++) {
		prTxStatPair->u2WlanIdx = prEventTxRate->u2WlanIdx;
		prTxStat = &prTxStatPair->txStatisticRes;

		os_move_mem(&prTxStat->rEntryTxRate,
					&prEventTxRate->rEntryTxRate, sizeof(RA_PHY_CFG_T));

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"WlanIdx:%u Mode:%u stbc:%u gi:%u bw:%u cod:%u mcs:%u nss:%u ltf:%u\n",
			prEventTxRate->u2WlanIdx,
			prEventTxRate->rEntryTxRate.MODE, prEventTxRate->rEntryTxRate.STBC,
			prEventTxRate->rEntryTxRate.ShortGI, prEventTxRate->rEntryTxRate.BW,
			prEventTxRate->rEntryTxRate.ldpc, prEventTxRate->rEntryTxRate.MCS,
			prEventTxRate->rEntryTxRate.VhtNss, prEventTxRate->rEntryTxRate.he_ltf);

		// read next result
		prEventTxRate++;
		prTxStatPair++;
	}
}

VOID uni_event_per_sta_multi_tx_cnt_handler(struct cmd_msg *msg, char *payload, uint16_t payload_len)
{
	struct UNI_EVENT_PER_STA_INFO_T *prEvent = (struct UNI_EVENT_PER_STA_INFO_T *)payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *prEventTlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *)&(prEvent->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_TX_CNT_T *prEventTxCnt;
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat = NULL;
	P_TX_STATISTIC_RESULT_PAIR prTxStatPair = (P_TX_STATISTIC_RESULT_PAIR) msg->attr.rsp.wb_buf_in_calbk;
	UINT32 index, num;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"u2Tag = %u, Payloadlen:%u\n",
		prEventTlv->u2Tag, payload_len);

	prEventTxCnt = (struct UNI_EVENT_PER_STA_TX_CNT_T *)&(prEventTlv->au1PerStaContent[0]);
	num = (payload_len - sizeof(*prEventTlv)) / sizeof(*prEventTxCnt);

	for (index = 0; index < num; index++) {
		prTxStatPair->u2WlanIdx = prEventTxCnt->u2WlanIdx;
		prTxStat = &prTxStatPair->txStatisticRes;

		prTxStat->u4EntryTxCount =
			le2cpu32(prEventTxCnt->u4EntryTxCount);
		prTxStat->u4EntryTxFailCount =
			le2cpu32(prEventTxCnt->u4EntryTxFailCount);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"WlanIdx:%u, TxCount:%u, TxFailCount:%u\n",
			prEventTxCnt->u2WlanIdx, prEventTxCnt->u4EntryTxCount,
			prEventTxCnt->u4EntryTxFailCount);

		// read next result
		prEventTxCnt++;
		prTxStatPair++;
	}
}

VOID uni_event_per_sta_multi_pkt_cnt_handler(struct cmd_msg *msg, char *payload, uint16_t payload_len)
{
	struct UNI_EVENT_PER_STA_PKT_CNT_T *prEventPktCnt;
	struct UNI_EVENT_PER_STA_INFO_T *prEvent = (struct UNI_EVENT_PER_STA_INFO_T *)payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *prEventTlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *)&(prEvent->au1TlvBuffer[0]);
	struct sta_msdu_pkt_cnt *prMutilStaPktCnt = (struct sta_msdu_pkt_cnt *)msg->attr.rsp.wb_buf_in_calbk;
	UINT32 index, num;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
		"%s: u2Tag = %u, Payloadlen:%u\n",
		__func__, prEventTlv->u2Tag, payload_len);

	prEventPktCnt = (struct UNI_EVENT_PER_STA_PKT_CNT_T *)&(prEventTlv->au1PerStaContent[0]);
	num = (payload_len - sizeof(*prEventTlv)) / sizeof(*prEventPktCnt);

	for (index = 0; index < num; index++) {
		prMutilStaPktCnt->txMsduDropCnt +=
			le2cpu32(prEventPktCnt->u4EntryTxDropCount);
		prMutilStaPktCnt->txMsduRetryCnt +=
			le2cpu32(prEventPktCnt->u4EntryTxRetryCount);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
			"%s: WlanIdx:%u, TxDropCount:%u, TxRetryCount:%u\n",
			__func__, prEventPktCnt->u2WlanIdx, prEventPktCnt->u4EntryTxDropCount,
			prEventPktCnt->u4EntryTxRetryCount);

		// read next result
		prEventPktCnt++;
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
		"%s: total TxDropCount:%lu, total TxRetryCount:%lu\n",
		__func__, prMutilStaPktCnt->txMsduDropCnt,
		prMutilStaPktCnt->txMsduRetryCnt);
}

VOID uni_event_per_sta_rssi_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	struct _RSSI_REPORT *rssi_rpt = (struct _RSSI_REPORT *) msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_RSSI_T *ptr = (struct UNI_EVENT_PER_STA_RSSI_T *) &(tlv->au1PerStaContent[0]);
	UINT8 i = 0, rcpi;
	CHAR rssi;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"Tag:%u Len:%u\n",
		tlv->u2Tag, rsp_payload_len);

	for (i = 0; i < 5; i++) {
		if (i == 4)
			rcpi = (ptr->au1Rcpi4);
		else
			rcpi = (ptr->au1Rcpi[i]);
		rssi = (rcpi - 220) / 2;

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"ant: %d, rcpi: %d, rssi: %d\n", i, rcpi, rssi);

		if (rssi > 0)
			rssi_rpt->rssi[i] = -127;
		else
			rssi_rpt->rssi[i] = rssi;
	}
}

VOID uni_event_per_sta_contention_rxrate_handler(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	struct _EXT_EVENT_PHY_STATE_RX_RATE *rx_rate = (struct _EXT_EVENT_PHY_STATE_RX_RATE *) msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_CONTENTION_RX_PHYRATE_T *ptr = (struct UNI_EVENT_PER_STA_CONTENTION_RX_PHYRATE_T *) &(tlv->au1PerStaContent[0]);

	rx_rate->u1RxRate = ptr->u1RxRate;
	rx_rate->u1RxMode = ptr->u1RxMode;
	rx_rate->u1RxNsts = ptr->u1RxNsts;
	rx_rate->u1Gi     = ptr->u1Gi;
	rx_rate->u1Coding = ptr->u1Coding;
	rx_rate->u1Stbc   = ptr->u1Stbc;
	rx_rate->u1BW     = ptr->u1Bw;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"Rate:%u Mode:%u Nsts:%u GI:%u Cod:%u Stbc:%u BW:%u\n",
		rx_rate->u1RxRate, rx_rate->u1RxMode, rx_rate->u1RxNsts,
		rx_rate->u1Gi, rx_rate->u1Coding, rx_rate->u1Stbc, rx_rate->u1BW);
}

VOID uni_event_multi_sta_contention_rxrate_handler(struct cmd_msg *msg, char *rsp_payload,
							UINT16 rsp_payload_len)
{
	struct UNI_EVENT_PER_STA_INFO_T *prEvent = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *prEventTlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(prEvent->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_CONTENTION_RX_PHYRATE_T *prEventRxRate = NULL;
	struct _EXT_EVENT_PHY_STATE_RX_RATE *rx_rate = NULL;
	UINT32 index, num;
	P_PHY_STATE_RX_RATE_PAIR prRxRatePair = (P_PHY_STATE_RX_RATE_PAIR) msg->attr.rsp.wb_buf_in_calbk;

	prEventRxRate = (struct UNI_EVENT_PER_STA_CONTENTION_RX_PHYRATE_T *) &(prEventTlv->au1PerStaContent[0]);
	num = (rsp_payload_len - sizeof(*prEventTlv)) / sizeof(*prEventRxRate);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"u2Tag = %u Payloadlen:%u num=%d\n", prEventTlv->u2Tag, rsp_payload_len, num);

	for (index = 0; index < num; index++) {
		prRxRatePair->u2Wcid = prEventRxRate->u2WlanIdx;
		rx_rate = &prRxRatePair->rRxStatResult;
		rx_rate->u1RxRate = prEventRxRate->u1RxRate;
		rx_rate->u1RxMode = prEventRxRate->u1RxMode;
		rx_rate->u1RxNsts = prEventRxRate->u1RxNsts;
		rx_rate->u1Gi	  = prEventRxRate->u1Gi;
		rx_rate->u1Coding = prEventRxRate->u1Coding;
		rx_rate->u1Stbc   = prEventRxRate->u1Stbc;
		rx_rate->u1BW	  = prEventRxRate->u1Bw;

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"u2WlanIdx=%d, Rate:%u Mode:%u Nsts:%u GI:%u Cod:%u Stbc:%u BW:%u\n",
			prEventRxRate->u2WlanIdx, rx_rate->u1RxRate, rx_rate->u1RxMode, rx_rate->u1RxNsts,
			rx_rate->u1Gi, rx_rate->u1Coding, rx_rate->u1Stbc, rx_rate->u1BW);

		// read next result
		prEventRxRate++;
		prRxRatePair++;
	}
}


VOID uni_event_per_sta_per_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	PUINT8 pu1PER = (PUINT8) msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_PER_T *ptr = (struct UNI_EVENT_PER_STA_PER_T *) &(tlv->au1PerStaContent[0]);

	(*pu1PER) = ptr->u1Per;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"PER:%u\n", *pu1PER);
}

VOID uni_event_per_sta_snr_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	PUINT8 pu1SNR = (PUINT8) msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_SNR_T *ptr = (struct UNI_EVENT_PER_STA_SNR_T *) &(tlv->au1PerStaContent[0]);

	pu1SNR[0] = ptr->au1Snr[0];
	pu1SNR[1] = ptr->au1Snr[1];
	pu1SNR[2] = ptr->au1Snr[2];
	pu1SNR[3] = ptr->au1Snr[3];

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"SNR:%d-%d-%d-%d\n",
		pu1SNR[0], pu1SNR[1], pu1SNR[2], pu1SNR[3]);
}

VOID uni_event_per_sta_txrate_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat = (P_EXT_EVENT_TX_STATISTIC_RESULT_T)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_TX_RATE_T *prEventTxRate = (struct UNI_EVENT_PER_STA_TX_RATE_T *) &(tlv->au1PerStaContent[0]);

	os_move_mem(&prTxStat->rEntryTxRate,
				&prEventTxRate->rEntryTxRate, sizeof(RA_PHY_CFG_T));

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"Entry Mode:%u Flag:%x STBC:%u GI:%u BW:%u ldpc:%u MCS:%u Nss:%u ltf:%u\n",
		prTxStat->rEntryTxRate.MODE, prTxStat->rEntryTxRate.Flags,
		prTxStat->rEntryTxRate.STBC, prTxStat->rEntryTxRate.ShortGI,
		prTxStat->rEntryTxRate.BW, prTxStat->rEntryTxRate.ldpc,
		prTxStat->rEntryTxRate.MCS, prTxStat->rEntryTxRate.VhtNss,
		prTxStat->rEntryTxRate.he_ltf);
}

VOID uni_event_per_sta_txcnt_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	P_EXT_EVENT_TX_STATISTIC_RESULT_T prTxStat = (P_EXT_EVENT_TX_STATISTIC_RESULT_T)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_TX_CNT_T *prEventTxCnt = (struct UNI_EVENT_PER_STA_TX_CNT_T *) &(tlv->au1PerStaContent[0]);

	prTxStat->u4EntryTxCount =
		le2cpu32(prEventTxCnt->u4EntryTxCount);
	prTxStat->u4EntryTxFailCount =
		le2cpu32(prEventTxCnt->u4EntryTxFailCount);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"Entry TxCnt:%u TxFailCnt:%u\n",
		prTxStat->u4EntryTxCount, prTxStat->u4EntryTxFailCount);
}

VOID uni_event_per_sta_pkt_cnt_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	struct sta_msdu_pkt_cnt *prPerStaPktCnt = (struct sta_msdu_pkt_cnt *)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_PKT_CNT_T *prEventTxCnt = (struct UNI_EVENT_PER_STA_PKT_CNT_T *) &(tlv->au1PerStaContent[0]);

	prPerStaPktCnt->txMsduDropCnt =
		(ULONG)le2cpu32(prEventTxCnt->u4EntryTxDropCount);
	prPerStaPktCnt->txMsduRetryCnt =
		(ULONG)le2cpu32(prEventTxCnt->u4EntryTxRetryCount);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_COUNTER_INFO, DBG_LVL_DEBUG,
		"%s: Entry TxDropCount:%lu TxRetryCount:%lu\n",
		__func__, prPerStaPktCnt->txMsduDropCnt, prPerStaPktCnt->txMsduRetryCnt);
}

VOID uni_event_per_sta_tid_sn_handler(struct cmd_msg *msg, char *rsp_payload,
					UINT16 rsp_payload_len)
{
	UINT16 *tid_sn_rpt = (UINT16 *) msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_PER_STA_INFO_T *event = (struct UNI_EVENT_PER_STA_INFO_T *)rsp_payload;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);
	struct UNI_EVENT_PER_STA_TID_SN_T *ptr = (struct UNI_EVENT_PER_STA_TID_SN_T *) &(tlv->au1PerStaContent[0]);
	UINT8 i = 0;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"Tag:%u Len:%u\n",
		tlv->u2Tag, rsp_payload_len);

	for (i = 0; i<8; i++) {
		tid_sn_rpt[i] = (ptr->u2TidSn[i]);
	}

}

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
static VOID uni_event_parse_per_sta_update_txcnt(
	RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT32 Length)
{
	struct UNI_EVENT_PER_STA_TX_CNT_T *event;
	UINT16 wcid;
	struct _MAC_TABLE_ENTRY *entry;
	struct _STA_TR_ENTRY *tr_entry;
	UINT32 tx_success;
	UINT32 stanum;
	event = (struct UNI_EVENT_PER_STA_TX_CNT_T *)Data;

	stanum = Length / sizeof(struct UNI_EVENT_PER_STA_TX_CNT_T);
	while (stanum > 0) {
		stanum--;

		wcid = event->u2WlanIdx;

		if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
			return;

		entry = entry_get(pAd, wcid);

		if (IS_ENTRY_NONE(entry))
			return;

		tr_entry = tr_entry_get(pAd, entry->tr_tb_idx);

		if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
			return;

		event->u4EntryTxCount = le2cpu32(event->u4EntryTxCount);
		event->u4EntryTxFailCount = le2cpu32(event->u4EntryTxFailCount);
		if (event->u4EntryTxCount > 0) {
			tx_success = event->u4EntryTxCount - event->u4EntryTxFailCount;
			entry->TotalTxSuccessCnt += tx_success;
			entry->one_sec_tx_succ_pkts = tx_success;
			entry->ContinueTxFailCnt += event->u4EntryTxFailCount;
			entry->TxStatRspCnt++;
		}

		/* sync to setup link for MLO case */
#ifdef DOT11_EHT_BE
		{
			BOOLEAN sync_flag = FALSE;
			struct mld_entry_t *mld_entry = NULL;
			UINT16	setup_wcid = 0xffff;
			struct _MAC_TABLE_ENTRY *setup_entry;
			struct _STA_TR_ENTRY *setup_tr_entry;

			if (IS_ENTRY_MLO(entry) && !entry->mlo.is_setup_link_entry)
				sync_flag = TRUE;
			if (sync_flag) {
				mt_rcu_read_lock();
				mld_entry = get_mld_entry_by_mac(entry->mlo.mld_addr);
				if (mld_entry)
					setup_wcid = mld_entry->setup_link_wcid;
				mt_rcu_read_unlock();

				if (setup_wcid != 0xffff) {
					setup_entry = entry_get(pAd, setup_wcid);

					setup_tr_entry = tr_entry_get(pAd, setup_entry->tr_tb_idx);
					if (setup_tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
						return;
					if (event->u4EntryTxCount > 0) {
						tx_success = event->u4EntryTxCount - event->u4EntryTxFailCount;
						setup_entry->TotalTxSuccessCnt += tx_success;
						setup_entry->TxStatRspCnt++;
					}
					if (setup_entry->bTxPktChk) {
						MTWF_DBG(pAd, DBG_CAT_AP, CATAP_KPLIVE, DBG_LVL_DEBUG,
							"wcid(%d),setup_wcid(%d),TotalTxCnt(%u) - TotalTxFail(%u) = %u (%s)\n",
							wcid, setup_wcid,
							event->u4EntryTxCount,
							event->u4EntryTxFailCount,
							entry->TotalTxSuccessCnt,
							(entry->TxStatRspCnt) ? "Valid":"Invalid");
					}
				}
				return;
			}
		}
#endif

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"wcid(%d), TotalTxCnt(%u) - TotalTxFail(%u) = %u (%s)\n",
			wcid,
			event->u4EntryTxCount,
			event->u4EntryTxFailCount,
			entry->TotalTxSuccessCnt,
			(entry->TxStatRspCnt) ? "Valid" : "Invalid");

		event++;
	}
}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

VOID UniEventPerStaUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct UNI_EVENT_PER_STA_INFO_T *event;
	struct UNI_EVENT_PER_STA_INFO_COMMON_T *tlv;
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(*event);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	event = (struct UNI_EVENT_PER_STA_INFO_T *)pData;
	tlv = (struct UNI_EVENT_PER_STA_INFO_COMMON_T *) &(event->au1TlvBuffer[0]);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

	switch (tlv->u2Tag) {
	case UNI_EVENT_PER_STA_TX_CNT:
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		uni_event_parse_per_sta_update_txcnt(pAd, &(tlv->au1PerStaContent[0]), (tags_len - sizeof(*tlv)));
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag = %u\n", tlv->u2Tag);
		break;
	}
}

static VOID uni_event_parse_all_sta_tx_rate(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT16 StaNum,
	UINT32 Length)
{
#ifdef CONFIG_MAP_SUPPORT
	struct UNI_EVENT_ALL_STA_TX_RATE_T *prEventResult;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx;

	union _HTTRANSMIT_SETTING LastTxRate;
	union _HTTRANSMIT_SETTING LastRxRate;
	union _HETRANSMIT_SETTING HELastTxRate, HELastRxRate = {0};
	union _EHTTRANSMIT_SETTING EHTLastTxRate;
	union _EHTTRANSMIT_SETTING EHTLastRxRate;

	prEventResult = (struct UNI_EVENT_ALL_STA_TX_RATE_T *)Data;

	for (Idx = 0; Idx < StaNum; Idx++) {
		pEntry = entry_get(pAd, prEventResult->arTxRateResult[Idx].u2WlanIdx);
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			/*For TX rate*/
			LastTxRate.word = LastRxRate.word = 0;
			HELastTxRate.word = HELastRxRate.word = 0;
			EHTLastTxRate.word = EHTLastRxRate.word = 0;
			if (prEventResult->arTxRateResult[Idx].u1TxMode == MODE_EHT_MU) {
				EHTLastTxRate.field.MODE = prEventResult->arTxRateResult[Idx].u1TxMode;
				EHTLastTxRate.field.BW = prEventResult->arTxRateResult[Idx].u1Bw;
				EHTLastTxRate.field.ldpc = prEventResult->arTxRateResult[Idx].u1Ldpc ? 1 : 0;
				EHTLastTxRate.field.ShortGI =
					prEventResult->arTxRateResult[Idx].u1ShortGI ? 1 : 0;
				EHTLastTxRate.field.STBC = prEventResult->arTxRateResult[Idx].u1Stbc;
			} else if (IS_HE_MODE(prEventResult->arTxRateResult[Idx].u1TxMode)) {
				HELastTxRate.field.MODE = prEventResult->arTxRateResult[Idx].u1TxMode;
				HELastTxRate.field.BW = prEventResult->arTxRateResult[Idx].u1Bw;
				HELastTxRate.field.ldpc = prEventResult->arTxRateResult[Idx].u1Ldpc ? 1 : 0;
				HELastTxRate.field.ShortGI =
					prEventResult->arTxRateResult[Idx].u1ShortGI ? 1 : 0;
				HELastTxRate.field.STBC = prEventResult->arTxRateResult[Idx].u1Stbc;
			} else {
				LastTxRate.field.MODE = prEventResult->arTxRateResult[Idx].u1TxMode;
				LastTxRate.field.BW = prEventResult->arTxRateResult[Idx].u1Bw;
				LastTxRate.field.ldpc = prEventResult->arTxRateResult[Idx].u1Ldpc ? 1 : 0;
				LastTxRate.field.ShortGI =
					prEventResult->arTxRateResult[Idx].u1ShortGI ? 1 : 0;
				LastTxRate.field.STBC = prEventResult->arTxRateResult[Idx].u1Stbc;
			}
			if (prEventResult->arTxRateResult[Idx].u1TxMode == MODE_EHT_MU) {
				EHTLastTxRate.field.MCS =
					(((prEventResult->arTxRateResult[Idx].u1VhtNss - 1)
					& 0x3) << 4) +
					prEventResult->arTxRateResult[Idx].u1Mcs;
				pEntry->map_LastTxRate = (UINT32)EHTLastTxRate.word;
			} else if (IS_HE_MODE(prEventResult->arTxRateResult[Idx].u1TxMode)) {
				HELastTxRate.field.MCS = prEventResult->arTxRateResult[Idx].u1Mcs;
				HELastTxRate.field.Nss = prEventResult->arTxRateResult[Idx].u1VhtNss;
				pEntry->map_LastTxRate = (UINT32)HELastTxRate.word;
			} else if (prEventResult->arTxRateResult[Idx].u1TxMode  == MODE_VHT) {
				LastTxRate.field.MCS =
					(((prEventResult->arTxRateResult[Idx].u1VhtNss - 1)
					& 0x3) << 4) +
					prEventResult->arTxRateResult[Idx].u1Mcs;
				pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
			} else if (prEventResult->arTxRateResult[Idx].u1TxMode  == MODE_OFDM) {
				LastTxRate.field.MCS =
					getLegacyOFDMMCSIndex(prEventResult->arTxRateResult[Idx].u1Mcs)
					& 0x0000003F;
				pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
			} else {
				LastTxRate.field.MCS = prEventResult->arTxRateResult[Idx].u1Mcs;
				pEntry->map_LastTxRate = (UINT32)LastTxRate.word;
			}
/*For RX rate */
			if (prEventResult->arTxRateResult[Idx].u1RxMode == MODE_EHT_MU) {
				EHTLastRxRate.field.MODE = prEventResult->arTxRateResult[Idx].u1RxMode;
				EHTLastRxRate.field.BW = prEventResult->arTxRateResult[Idx].u1RxBW;
				EHTLastRxRate.field.ldpc = prEventResult->arTxRateResult[Idx].u1RxCoding ? 1 : 0;
				EHTLastRxRate.field.ShortGI =
					prEventResult->arTxRateResult[Idx].u1RxGi ? 1 : 0;
				EHTLastRxRate.field.STBC = prEventResult->arTxRateResult[Idx].u1RxStbc;
			} else if (IS_HE_MODE(prEventResult->arTxRateResult[Idx].u1RxMode)) {
				HELastRxRate.field.MODE = prEventResult->arTxRateResult[Idx].u1RxMode;
				HELastRxRate.field.BW = prEventResult->arTxRateResult[Idx].u1RxBW;
				HELastRxRate.field.ldpc = prEventResult->arTxRateResult[Idx].u1RxCoding ? 1 : 0;
				HELastRxRate.field.ShortGI =
					prEventResult->arTxRateResult[Idx].u1RxGi ? 1 : 0;
				HELastRxRate.field.STBC = prEventResult->arTxRateResult[Idx].u1RxStbc;
			} else {
				LastRxRate.field.MODE = prEventResult->arTxRateResult[Idx].u1RxMode;
				LastRxRate.field.BW = prEventResult->arTxRateResult[Idx].u1RxBW;
				LastRxRate.field.ldpc = prEventResult->arTxRateResult[Idx].u1RxCoding ? 1 : 0;
				LastRxRate.field.ShortGI =
					prEventResult->arTxRateResult[Idx].u1RxGi ? 1 : 0;
				LastRxRate.field.STBC = prEventResult->arTxRateResult[Idx].u1RxStbc;
			}

			if (prEventResult->arTxRateResult[Idx].u1RxMode == MODE_EHT_MU) {
#ifdef EM_PLUS_SIPPORT
				EHTLastRxRate.field.MCS = prEventResult->arTxRateResult[Idx].u1RxRate;
				EHTLastRxRate.field.Nss = prEventResult->arTxRateResult[Idx].u1RxNsts;
#else
				EHTLastRxRate.field.MCS =
					(((prEventResult->arTxRateResult[Idx].u1RxNsts - 1)
					& 0x3) << 4) +
					prEventResult->arTxRateResult[Idx].u1RxRate;
#endif
				pEntry->map_LastRxRate = (UINT32)EHTLastRxRate.word;
			} else if (IS_HE_MODE(prEventResult->arTxRateResult[Idx].u1RxMode)) {
				HELastRxRate.field.MCS = prEventResult->arTxRateResult[Idx].u1RxRate;
				HELastRxRate.field.Nss = prEventResult->arTxRateResult[Idx].u1RxNsts;
				pEntry->map_LastRxRate = (UINT32)HELastRxRate.word;
			} else if (prEventResult->arTxRateResult[Idx].u1RxMode == MODE_VHT) {
				/* for STBC case NSS is NSTS/2 */
				if (prEventResult->arTxRateResult[Idx].u1RxStbc) {
					prEventResult->arTxRateResult[Idx].u1RxNsts =
						(prEventResult->arTxRateResult[Idx].u1RxNsts & 0x3) >> 1;
				}
				LastRxRate.field.MCS =
					(((prEventResult->arTxRateResult[Idx].u1RxNsts)
					& 0x3) << 4) +
					prEventResult->arTxRateResult[Idx].u1RxRate;
				pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
			} else if (prEventResult->arTxRateResult[Idx].u1RxMode == MODE_OFDM) {
				LastRxRate.field.MCS =
					getLegacyOFDMMCSIndex(prEventResult->arTxRateResult[Idx].u1RxRate)
					& 0x0000003F;
				pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
			} else {
				LastRxRate.field.MCS = prEventResult->arTxRateResult[Idx].u1RxRate;
				pEntry->map_LastRxRate = (UINT32)LastRxRate.word;
			}
		}
	}
#endif
}

static VOID uni_event_parse_all_sta_tx_stat(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT16 StaNum,
	UINT32 Length)
{
	struct UNI_EVENT_ALL_STA_TX_STAT_T *prEventResult = (struct UNI_EVENT_ALL_STA_TX_STAT_T *)Data;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx;

	for (Idx = 0; Idx < StaNum; Idx++) {
		struct UNI_EVENT_ONE_STA_TX_STAT_T *prTxStatResult = (struct UNI_EVENT_ONE_STA_TX_STAT_T *)&prEventResult->arTxStatResult[Idx];

		prTxStatResult->u2WlanIdx = le2cpu16(prTxStatResult->u2WlanIdx);
		pEntry = entry_get(pAd, prTxStatResult->u2WlanIdx);

		if (pEntry && pEntry->wdev && pEntry->Sst == SST_ASSOC &&
		(IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)))
		{
			/* sta counter */
			pEntry->mpdu_attempts.QuadPart += le2cpu32(prTxStatResult->u4TotalTxCount);
			pEntry->mpdu_retries.QuadPart += le2cpu32(prTxStatResult->u4TotalTxFailCount);
			pEntry->one_sec_tx_mpdu_pkts = le2cpu32(prTxStatResult->u4TotalTxCount);
			pEntry->TxFailCount += le2cpu32(prTxStatResult->u4TotalTxFailCount);
			pEntry->one_sec_tx_succ_pkts = le2cpu32(prTxStatResult->u4TotalTxCount) - le2cpu32(prTxStatResult->u4TotalTxFailCount);
			if (le2cpu32(prTxStatResult->u4TotalTxCount) > 0)
				pEntry->tx_per = 100 *
					le2cpu32(prTxStatResult->u4TotalTxFailCount) / le2cpu32(prTxStatResult->u4TotalTxCount);

#ifdef CONFIG_AP_SUPPORT
			/* bss counter */
			if (pEntry->pMbss) {
				struct _BSS_STRUCT *mbss = MBSS_GET(pEntry->pMbss);

				mbss->TxErrorCount += le2cpu32(prTxStatResult->u4TotalTxFailCount);
				mbss->TxDropCount = mbss->TxErrorCount;
			} else
				MTWF_DBG(pAd, DBG_CAT_TX, CATTX_TMAC, DBG_LVL_INFO,
					"pMbss ptr invalid\n");
#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
				struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(pAd, pEntry->wdev);

				apcli->StaStatistic.TxErrorCount += le2cpu32(prTxStatResult->u4TotalTxFailCount);
				apcli->StaStatistic.TxDropCount = apcli->StaStatistic.TxErrorCount;
			}
#endif /*APCLI_SUPPORT*/

		MTWF_DBG(pAd, DBG_CAT_TX, CATTX_TMAC, DBG_LVL_INFO,
			"wlanidx=%d, TxCount=%d TxFailCount=%d\n",
			prTxStatResult->u2WlanIdx, le2cpu32(prTxStatResult->u4TotalTxCount),
			le2cpu32(prTxStatResult->u4TotalTxFailCount));

		}
	}
}

static VOID uni_event_parse_all_sta_txrx_adm_stat(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT16 StaNum,
	UINT32 Length)
{
	struct UNI_EVENT_ALL_STA_TXRX_ADM_STAT_T *prEventResult;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx, k;
	ULONG tx_bytes, rx_bytes;
	struct _RTMP_ADAPTER *pMacEntrypAd = NULL;
	UINT32 tx_bytes_adm[MAX_ACCESS_CAT_NUM] = {0};
	UINT32 rx_bytes_adm[MAX_ACCESS_CAT_NUM] = {0};

	prEventResult = (struct UNI_EVENT_ALL_STA_TXRX_ADM_STAT_T *)Data;

	for (Idx = 0; Idx < StaNum; Idx++) {
		pEntry = entry_get(pAd, prEventResult->arTxRxAdmStatResult[Idx].u2WlanIdx);

		if (pEntry && pEntry->wdev && (IS_ENTRY_CLIENT(pEntry) ||
			IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) && pEntry->Sst == SST_ASSOC) {
			tx_bytes = 0;
			rx_bytes = 0;
			for (k = 0; k < MAX_ACCESS_CAT_NUM; k++) {
				tx_bytes_adm[k] += prEventResult->arTxRxAdmStatResult[Idx].au4TxBytesAdm[k];
				rx_bytes_adm[k] += prEventResult->arTxRxAdmStatResult[Idx].au4RxBytesAdm[k];
				tx_bytes += prEventResult->arTxRxAdmStatResult[Idx].au4TxBytesAdm[k];
				rx_bytes += prEventResult->arTxRxAdmStatResult[Idx].au4RxBytesAdm[k];
			}

			/* sta counter by mpdu, per link */
			pEntry->TxBytes += tx_bytes;
			pEntry->RxBytes += rx_bytes;
			pEntry->OneSecTxBytes = tx_bytes;
			pEntry->OneSecRxBytes = rx_bytes;

#ifdef MAP_R2
			if (IS_MAP_ENABLE(pAd) && IS_MAP_R2_ENABLE(pAd)) {
				if (pEntry->pMbss) {
					struct _BSS_STRUCT *mbss = MBSS_GET(pEntry->pMbss);

#ifdef MAP_R6
					if (pEntry->mlo.mlo_en) {
						mbss->mlo_ucBytesRx = pEntry->RxBytes;
						mbss->mlo_ucBytesTx = pEntry->TxBytes;
					} else {
						mbss->ucBytesRx = pEntry->RxBytes;
						mbss->ucBytesTx = pEntry->TxBytes;
					}
#else
					mbss->ucBytesRx = pEntry->RxBytes;
					mbss->ucBytesTx = pEntry->TxBytes;
#endif
				}
			}
			if (IS_MAP_ENABLE(pAd)) {
				pEntry->RxBytesMAP = pEntry->RxBytes;
				pEntry->TxBytesMAP = pEntry->TxBytes;
			}
#endif

#ifdef TXRX_STAT_SUPPORT
			if (pEntry->pAd) {
				pMacEntrypAd = (struct _RTMP_ADAPTER *)pEntry->pAd;
				pMacEntrypAd->WlanCounters.RxDataByte.QuadPart += rx_bytes;
				pMacEntrypAd->WlanCounters.TxDataByte.QuadPart += tx_bytes;
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
					"Wcid[%d] tx_bytes=%ld, rx_bytes=%ld\n", pEntry->wcid, tx_bytes, rx_bytes);
			}
#endif /* TXRX_STAT_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
			/* bss counter */
			if (pEntry->pMbss) {
				struct _BSS_STRUCT *mbss = MBSS_GET(pEntry->pMbss);

				for (k = 0; k < MAX_ACCESS_CAT_NUM; k++) {
					mbss->TxBytesAdm[k] += tx_bytes_adm[k];
					mbss->RxBytesAdm[k] += rx_bytes_adm[k];
				}
				mbss->TransmittedByteCount += tx_bytes;
				mbss->ReceivedByteCount += rx_bytes;
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"mbss ssid=%s bssid="MACSTR" name=%s func_tb_idx=%d\n",
				mbss->Ssid,
				MAC2STR(mbss->wdev.bssid), mbss->wdev.if_dev->name,
				pEntry->func_tb_idx);
			} else
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
					"pMbss ptr invalid\n");
#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
				struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(pAd, pEntry->wdev);

				apcli->StaStatistic.TransmittedByteCount += tx_bytes;
				apcli->StaStatistic.ReceivedByteCount += rx_bytes;
			}
#endif /*APCLI_SUPPORT*/

			/* band counter */

			if (pEntry->pAd) {
				pMacEntrypAd = (struct _RTMP_ADAPTER *)pEntry->pAd;
				pMacEntrypAd->WlanCounters.ucBytesRx.QuadPart += rx_bytes;
				pMacEntrypAd->WlanCounters.TxTotByteCount.QuadPart += tx_bytes;
				pMacEntrypAd->WlanCounters.ucBytesTx.QuadPart += tx_bytes;
			}
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"idx=%d, tx_bytes=%ld,rx_bytes=%ld\n", Idx, tx_bytes, rx_bytes);
		}
	}
}

static VOID uni_event_parse_all_sta_txrx_air_time(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT16 StaNum,
	UINT32 Length)
{
	struct UNI_EVENT_ALL_STA_TXRX_AIR_TIME_T *prEventResult;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx, k;

	prEventResult = (struct UNI_EVENT_ALL_STA_TXRX_AIR_TIME_T *)Data;

	for (Idx = 0; Idx < StaNum; Idx++) {
		pEntry = entry_get(pAd, prEventResult->arTxRxAirTimeResult[Idx].u2WlanIdx);
		if (pEntry && pEntry->wdev &&  IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC) {
			for (k = 0; k < 4; k++) {
				if (pEntry->TxRxTime[k][0] - pEntry->wrapArTxRxTime[k][0] > prEventResult->arTxRxAirTimeResult[Idx].au4WtblRxTime[k]/1000) {
					/*Wrap around at FW has occurred*/
					/*Update the wrap around value to keep adding that*/
					pEntry->wrapArTxRxTime[k][0] = pEntry->TxRxTime[k][0];
					pEntry->TxRxTime[k][0] += prEventResult->arTxRxAirTimeResult[Idx].au4WtblRxTime[k]/1000;
				} else
					pEntry->TxRxTime[k][0] =
						pEntry->wrapArTxRxTime[k][0] + prEventResult->arTxRxAirTimeResult[Idx].au4WtblRxTime[k]/1000;

				if (pEntry->TxRxTime[k][1] - pEntry->wrapArTxRxTime[k][1] > prEventResult->arTxRxAirTimeResult[Idx].au4WtblTxTime[k]/1000) {
					pEntry->wrapArTxRxTime[k][1] = pEntry->TxRxTime[k][1];
					pEntry->TxRxTime[k][1] += prEventResult->arTxRxAirTimeResult[Idx].au4WtblTxTime[k]/1000;
				} else
					pEntry->TxRxTime[k][1] =
						pEntry->wrapArTxRxTime[k][1] + prEventResult->arTxRxAirTimeResult[Idx].au4WtblTxTime[k]/1000;

				if (pEntry->TxRxTime[k][0] < pEntry->wrapArTxRxTime[k][0])
					pEntry->wrapArTxRxTime[k][0] = 0;
			}
		}
	}
}

static VOID uni_event_parse_all_sta_trx_msdu_count(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT16 StaNum,
	UINT32 Length)
{
	struct UNI_EVENT_ALL_STA_TRX_MSDU_COUNT_T *prEventResult;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx;

#ifdef TR181_SUPPORT
	struct _RTMP_ADAPTER *pMacEntrypAd = NULL;
#endif

#ifdef DOT11_EHT_BE
	unsigned long *mldsta_added  = NULL;
#endif

	os_alloc_mem(pAd, (UCHAR **)&mldsta_added, (WTBL_MAX_NUM(pAd)/32+1) * sizeof(unsigned long));

	if (!mldsta_added)
		return;

	os_zero_mem(mldsta_added, (WTBL_MAX_NUM(pAd)/32+1) * sizeof(unsigned long));

	prEventResult = (struct UNI_EVENT_ALL_STA_TRX_MSDU_COUNT_T *)Data;

	for (Idx = 0; Idx < StaNum; Idx++) {
		pEntry = entry_get(pAd, prEventResult->arTRxMsduCountResult[Idx].u2WlanIdx);

		if (pEntry && pEntry->wdev && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)
			|| IS_ENTRY_REPEATER(pEntry)) && pEntry->Sst == SST_ASSOC) {
			struct UNI_EVENT_ONE_STA_TRX_MSDU_COUNT_T *prTRxMsduCountResult =
				(struct UNI_EVENT_ONE_STA_TRX_MSDU_COUNT_T *)&prEventResult->arTRxMsduCountResult[Idx];

			pEntry->TxPackets.QuadPart += le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);
			pEntry->RxPackets.QuadPart += le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
			pEntry->one_sec_rx_pkts = le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
			pEntry->one_sec_tx_pkts = le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);

			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"mld idx=%d,wcid=%d u2WlanIdx=%d,u4TxMsduCnt=%x,u4RxMsduCnt=%x\n",
				Idx, pEntry->wcid, le2cpu16(prTRxMsduCountResult->u2WlanIdx),
				le2cpu32(prTRxMsduCountResult->u4TxMsduCnt),
				le2cpu32(prTRxMsduCountResult->u4RxMsduCnt));

#ifdef DOT11_EHT_BE
		/* add MLD entry counter, msdu counter is per mld entry,
			so every mld entry will just add one time
			bss counter and band counter also just add one time */
			if (IS_ENTRY_MLO(pEntry)) {
				struct mld_entry_t *mld_entry = NULL;
				uint16_t mld_sta_idx;

				mt_rcu_read_lock();
				mld_entry = get_mld_entry_by_mac(pEntry->mlo.mld_addr);
				if (mld_entry)
					mld_sta_idx = mld_entry->mld_sta_idx;
				else {
					MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
						"mld_entry = NULL\n");
					mt_rcu_read_unlock();
					goto done;
				}
				mt_rcu_read_unlock();

				if (!test_bit(mld_sta_idx, (unsigned long *)mldsta_added)) {
#ifdef CONFIG_AP_SUPPORT
					/* bss counter */
					if (pEntry->pMbss) {
						struct _BSS_STRUCT *mbss = MBSS_GET(pEntry->pMbss);

						mbss->RxCount += le2cpu32(
							prTRxMsduCountResult->u4RxMsduCnt);
						mbss->ucPktsRx += le2cpu32(
							prTRxMsduCountResult->u4RxMsduCnt);
						mbss->TxCount += le2cpu32(
							prTRxMsduCountResult->u4TxMsduCnt);
						mbss->ucPktsTx += le2cpu32(
							prTRxMsduCountResult->u4TxMsduCnt);

					} else
						MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
							"pMbss ptr invalid\n");
#endif /* CONFIG_AP_SUPPORT */
					/* band counter */
#ifdef TR181_SUPPORT
					if (pEntry->pAd) {
						pMacEntrypAd = (struct _RTMP_ADAPTER *)pEntry->pAd;
						pMacEntrypAd->WlanCounters.ucPktsRx.QuadPart +=
							le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
						pMacEntrypAd->WlanCounters.ucPktsTx.QuadPart +=
							le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);
					}
#endif

					MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
						"mld enable,wcid=%d u2WlanIdx=%d\n",
						pEntry->wcid, le2cpu16(prTRxMsduCountResult->u2WlanIdx));
#ifdef APCLI_SUPPORT
					if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
						struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(pAd, pEntry->wdev);

						apcli->StaStatistic.RxCount += le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
						apcli->StaStatistic.TxCount += le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);
					}
#endif /*APCLI_SUPPORT*/
					__set_bit(mld_sta_idx, (unsigned long *)mldsta_added);
				}
			} else {
#ifdef CONFIG_AP_SUPPORT
				/* bss counter */
				if (pEntry->pMbss) {
					struct _BSS_STRUCT *mbss = MBSS_GET(pEntry->pMbss);

					mbss->RxCount += le2cpu32(
						prTRxMsduCountResult->u4RxMsduCnt);
					mbss->ucPktsRx += le2cpu32(
						prTRxMsduCountResult->u4RxMsduCnt);
					mbss->TxCount += le2cpu32(
						prTRxMsduCountResult->u4TxMsduCnt);
					mbss->ucPktsTx += le2cpu32(
						prTRxMsduCountResult->u4TxMsduCnt);
				} else
					MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
						"pMbss ptr invalid\n");
#endif /* CONFIG_AP_SUPPORT */
				/* band counter */
#ifdef TR181_SUPPORT
				if (pEntry->pAd) {
					pMacEntrypAd = (struct _RTMP_ADAPTER *)pEntry->pAd;
					pMacEntrypAd->WlanCounters.ucPktsRx.QuadPart +=
						le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
					pMacEntrypAd->WlanCounters.ucPktsTx.QuadPart +=
						le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);
				}
#endif

				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
					"mld disable,wcid=%d u2WlanIdx=%d\n",
					pEntry->wcid, le2cpu16(prTRxMsduCountResult->u2WlanIdx));

#ifdef APCLI_SUPPORT
				if (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
					struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(pAd, pEntry->wdev);

					apcli->StaStatistic.RxCount += le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
					apcli->StaStatistic.TxCount += le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);
				}
#endif /*APCLI_SUPPORT*/
			}
#else
#ifdef CONFIG_AP_SUPPORT
				/* bss counter */
				if (pEntry->pMbss) {
					struct _BSS_STRUCT *mbss = MBSS_GET(pEntry->pMbss);

					mbss->RxCount += le2cpu32(
						prTRxMsduCountResult->u4RxMsduCnt);
					mbss->ucPktsRx += le2cpu32(
						prTRxMsduCountResult->u4RxMsduCnt);
					mbss->TxCount += le2cpu32(
						prTRxMsduCountResult->u4TxMsduCnt);
					mbss->ucPktsTx += le2cpu32(
						prTRxMsduCountResult->u4TxMsduCnt);
				} else
					MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
						"pMbss ptr invalid\n");
#endif /* CONFIG_AP_SUPPORT */
				/* band counter */
#ifdef TR181_SUPPORT
				if (pEntry->pAd) {
					pMacEntrypAd = (struct _RTMP_ADAPTER *)pEntry->pAd;
					pMacEntrypAd->WlanCounters.ucPktsRx.QuadPart +=
						le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
					pMacEntrypAd->WlanCounters.ucPktsTx.QuadPart +=
						le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);
				}
#endif
#ifdef APCLI_SUPPORT
				if ((IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry)) && pEntry->wdev) {
					struct _STA_ADMIN_CONFIG *apcli = GetStaCfgByWdev(pAd, pEntry->wdev);

					apcli->StaStatistic.RxCount += le2cpu32(prTRxMsduCountResult->u4RxMsduCnt);
					apcli->StaStatistic.TxCount += le2cpu32(prTRxMsduCountResult->u4TxMsduCnt);
				}
#endif /*APCLI_SUPPORT*/
#endif /*DOT11_EHT_BE*/
		}
	}

done:
	if (mldsta_added)
		os_free_mem(mldsta_added);

	return;
}

#if defined(TXRX_STAT_SUPPORT) || defined(TR181_SUPPORT)
static VOID uni_event_parse_all_sta_data_tx_retry_count(
	struct _RTMP_ADAPTER *pAd,
	UINT8 *Data,
	UINT16 StaNum,
	UINT32 Length)
{
	struct UNI_EVENT_ALL_STA_DATA_TX_RETRY_COUNT_T *prEventResult;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UINT32 Idx;

	prEventResult = (struct UNI_EVENT_ALL_STA_DATA_TX_RETRY_COUNT_T *)Data;

	for (Idx = 0; Idx < StaNum; Idx++) {
		pEntry = entry_get(pAd, prEventResult->arDataTxRetryCountResult[Idx].u2WlanIdx);

		if (pEntry && pEntry->wdev && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_PEER_AP(pEntry)) && pEntry->Sst == SST_ASSOC) {
			struct UNI_EVENT_ONE_STA_DATA_TX_RETRY_COUNT_T *prDataTxRetryCountResult =
							(struct UNI_EVENT_ONE_STA_DATA_TX_RETRY_COUNT_T *)&prEventResult->arDataTxRetryCountResult[Idx];

#ifdef TXRX_STAT_SUPPORT
			pEntry->TxPacketsRetryCnt.QuadPart += prDataTxRetryCountResult->u2TxRetryCnt;
			if (pEntry->pAd) {
				struct _RTMP_ADAPTER *pMacEntrypAd = (struct _RTMP_ADAPTER *)pEntry->pAd;

				pMacEntrypAd->WlanCounters.TxDataRetryCnt.QuadPart +=
					prDataTxRetryCountResult->u2TxRetryCnt;
			}
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				"Wcid[%d] u4DataTxRetryCnt(%lld)\n",
				pEntry->wcid,
				pEntry->TxPacketsRetryCnt.QuadPart);
#endif
#ifdef TR181_SUPPORT
			pEntry->TxRetriedPktCount += le2cpu16(prDataTxRetryCountResult->u2TxRetryCnt);
			MTWF_DBG(pAd, DBG_CAT_AP, CATAP_TR181, DBG_LVL_INFO,
				"wcid(%d), TxRetriedPktCount(%ld)\n",
				prDataTxRetryCountResult->u2WlanIdx, pEntry->TxRetriedPktCount);
#endif
		}
	}
}
#endif /* TXRX_STAT_SUPPORT */

VOID UniEventAllStaUnsolicitHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct UNI_EVENT_ALL_STA_INFO_T *event = (struct UNI_EVENT_ALL_STA_INFO_T *)pData;
	struct UNI_EVENT_ALL_STA_INFO_COMMON_T *tlv = (struct UNI_EVENT_ALL_STA_INFO_COMMON_T *)&(event->au1TlvBuffer[0]);
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(*event);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

	switch (tlv->u2Tag) {
	case UNI_EVENT_ALL_STA_TX_RATE:
		uni_event_parse_all_sta_tx_rate(pAd, &(tlv->au1AllStaContent[0]), le2cpu16(tlv->u2StaNum), (tags_len - sizeof(*tlv)));
		break;

	case UNI_EVENT_ALL_STA_TX_STAT:
		uni_event_parse_all_sta_tx_stat(pAd, &(tlv->au1AllStaContent[0]), le2cpu16(tlv->u2StaNum), (tags_len - sizeof(*tlv)));
		break;

	case UNI_EVENT_ALL_STA_TXRX_ADM_STAT:
		uni_event_parse_all_sta_txrx_adm_stat(pAd, &(tlv->au1AllStaContent[0]), le2cpu16(tlv->u2StaNum), (tags_len - sizeof(*tlv)));
		break;

	case UNI_EVENT_ALL_STA_TXRX_AIR_TIME:
		uni_event_parse_all_sta_txrx_air_time(pAd, &(tlv->au1AllStaContent[0]), le2cpu16(tlv->u2StaNum), (tags_len - sizeof(*tlv)));
		break;

	case UNI_EVENT_ALL_STA_TRX_MSDU_COUNT:
		uni_event_parse_all_sta_trx_msdu_count(pAd, &(tlv->au1AllStaContent[0]), le2cpu16(tlv->u2StaNum), (tags_len - sizeof(*tlv)));
		break;

#if defined(TXRX_STAT_SUPPORT) || defined(TR181_SUPPORT)
	case UNI_EVENT_ALL_STA_DATA_TX_RETRY_COUNT:
		uni_event_parse_all_sta_data_tx_retry_count(pAd, &(tlv->au1AllStaContent[0]), le2cpu16(tlv->u2StaNum), (tags_len - sizeof(*tlv)));
		break;
#endif /* TXRX_STAT_SUPPORT */

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag = %u\n", tlv->u2Tag);
		break;
	}
}

static PROCESS_RX_UNSOLICIT_EVENT_FUNCTION arEventTable[UNI_EVENT_ID_MAX_NUM] = {
	[0 ... UNI_EVENT_ID_MAX_NUM - 1] = NULL,
	[UNI_EVENT_ID_FW_LOG_2_HOST] = UniEventFwLog2HostHandler, /* 0x04 */
	[UNI_EVENT_ID_STAREC] = UniEventStaRecUnsolicitHandler, /* 0x0F */
	[UNI_EVENT_ID_IE_COUNTDOWN] = UniEventIECountDownHandler, /* 0x09 */
	[UNI_EVENT_ID_ASSERT_DUMP] = UniEventAssertDumpHandler, /* 0x0A */
	[UNI_EVENT_ID_BEACON_TIMEOUT] = UniEventBeaconTimeoutHandler, /* 0x0C */
	[UNI_EVENT_ID_PS_SYNC] = UniEventPSSyncHandler, /* 0x0D */
	[UNI_EVENT_ID_ECC_CAL] = UniEventECCCalHandler, /* 0x10 */
	[UNI_EVENT_ID_RDD] = UniEventRDDReportHandler, /* 0x11 */
#ifdef WIFI_MD_COEX_SUPPORT
	[UNI_EVENT_ID_IDC] = UniEventIDCHandler, /* 0x17 */
#endif
#ifdef CFG_SUPPORT_FALCON_SR
	[UNI_EVENT_ID_SR] = UniEventSRHandler, /* 0x25 */
#endif/*CFG_SUPPORT_FALCON_SR*/
	[UNI_EVENT_ID_TXPOWER] = UniEventTxPowerUnsolicitHandler, /* 0x2A */
	[UNI_EVENT_ID_BA_OFFLOAD] = UniEventBaOffload, /* 0x2C */
	[UNI_EVENT_ID_RA] = UniEventRAHandler, /* 0x2F */
	[UNI_EVENT_ID_SPECTRUM] = UniEventSpectrumGetData, /* 0x30 */
#ifdef DOT11_HE_AX
	[UNI_EVENT_ID_MURU] = UniEventMuruUnsolicitHandler, /* 0x31 */
#endif /* DOT11_HE_AX */
#ifdef TXBF_SUPPORT
	[UNI_EVENT_ID_BF] = UniEventBFHandleEvent, /*0x33*/
#endif
	[UNI_EVENT_ID_THERMAL] = UniEventThermalUnsolicitHandler, /* 0x35 */
	[UNI_EVENT_ID_NOISE_FLOOR] = UniEventNoiseFloorHandler, /*0x36*/
	[UNI_EVENT_ID_TPC] = UniEventTpcUnsolicitHandler,
	[UNI_EVENT_ID_MEC] = UniEventMecUnsolicitHandler,
	[UNI_EVENT_ID_TESTMODE_CTRL] = UniEventTestModeUnsolicitHandler,
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	[UNI_EVENT_ID_TWT_SYNC] = UniEventTWTSyncHandler, /* 0x47 */
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */
#ifdef CFG_SUPPORT_CSI
	[UNI_EVENT_ID_CSI] = UniEventCSIHandler, /* 0x4A */
#endif /* CFG_SUPPORT_CSI */
	[UNI_EVENT_ID_RXFE_CTRL] = UniEventRxFeCompHandler, /* 0x4D */
#ifdef MLR_SUPPORT
	[UNI_EVENT_ID_BSS_ER] = UniEventBssErHandler, /* 0x53 */
#endif
	[UNI_EVENT_ID_RRO] = UniEventRROHandler, /* 0x57 */
	[UNI_EVENT_ID_MLO_ALGO] = UniEventMloMldAlgoHandler, /* 0x59 */
#ifdef DOT11_EHT_BE
	[UNI_EVENT_ID_PP] = UniEventStaticPpHandler, /* 0x5A */
#endif
#ifdef FTM_SUPPORT
	[UNI_EVENT_ID_LOCATION] = UniEventLocUnsolicitHandler, /* 0x5D */
#endif /* FTM_SUPPORT */
	[UNI_EVENT_ID_STATE] = UniEventStateUnsolicitHandler, /* 0x6C */
	[UNI_EVENT_ID_PER_STA_INFO] = UniEventPerStaUnsolicitHandler, /* 0x6D */
	[UNI_EVENT_ID_ALL_STA_INFO] = UniEventAllStaUnsolicitHandler, /* 0x6E */
	[UNI_EVENT_ID_EPCS] = UniEventEPCSHandler, /* 0x6F */
	[UNI_EVENT_ID_BSS_MATCH] = UniEventBssMatchHandler, /* 0x71 */
#ifdef ZERO_PKT_LOSS_SUPPORT
	[UNI_EVENT_ID_SCSA]			= UniEventSCSAHandler,	/* 0x7A */
#endif
#ifdef DOT11_EHT_BE
	[UNI_EVENT_ID_MLD] = UniEventMldUnsolicitHandler, /* 0x81 */
#endif
};

VOID UniEventUnsolicitMainHandler(VOID *physical_dev, PNDIS_PACKET net_pkt)
{
	EVENT_RXD *event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);
	UINT8 eid = GET_EVENT_FW_RXD_EID(event_rxd);
	ULONG len = GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "seq_num=%d, eid=0x%x, len=%lu\n",
				 GET_EVENT_FW_RXD_SEQ_NUM(event_rxd), eid, len);

	if (eid < UNI_EVENT_ID_MAX_NUM && arEventTable[eid] != NULL)
		arEventTable[eid](physical_dev, GET_EVENT_HDR_ADDR(net_pkt), len, event_rxd);
	else
		MTWF_PRINT("%s: Unknown Unsolicit Event(0x%x)\n", __func__, eid);
}

VOID UniEventGetTssiHandle(
	struct cmd_msg *msg,
	struct _UNI_EVENT_ATE_TEST_MODE_T *payload,
	UINT16 Len)
{
	struct _UNI_EVENT_ATE_TEST_MODE_T *prUniEvtAteTestMode = payload;
	struct _GET_TSSI_STATUS_T *TSSI_Status;

	TSSI_Status = (struct _GET_TSSI_STATUS_T *)&prUniEvtAteTestMode->aucAteResult[0];
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &TSSI_Status->ucEnable, sizeof(TSSI_Status->ucEnable));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"TSSI Enable:%d Band:%d\n", TSSI_Status->ucEnable,
			 TSSI_Status->ucBand);
}

VOID UniEventGetDpdHandle(
	struct cmd_msg *msg,
	struct _UNI_EVENT_ATE_TEST_MODE_T *payload,
	UINT16 Len)
{

	struct _UNI_EVENT_ATE_TEST_MODE_T *prUniEvtAteTestMode = payload;
	struct _GET_DPD_STATUS_T *DPD_Status;

	DPD_Status = (struct _GET_DPD_STATUS_T *)&prUniEvtAteTestMode->aucAteResult[0];
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &DPD_Status->ucEnable, sizeof(DPD_Status->ucEnable));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"DPD Enable:%d Band:%d\n", DPD_Status->ucEnable,
			 DPD_Status->ucBand);
}

VOID UniEventGetThermoCompHandle(
	struct cmd_msg *msg,
	struct _UNI_EVENT_ATE_TEST_MODE_T *payload,
	UINT16 Len)
{
	struct _UNI_EVENT_ATE_TEST_MODE_T *prUniEvtAteTestMode = payload;
	struct _GET_THERMO_COMP_STATUS_T *THER_Status;
	THER_Status = (struct _GET_THERMO_COMP_STATUS_T *)&prUniEvtAteTestMode->aucAteResult[0];
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &THER_Status->ucEnable, sizeof(THER_Status->ucEnable));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"THER Enable:%d\n", THER_Status->ucEnable);
}

VOID UniEventGetFreqOffsetHandle(
	struct cmd_msg *msg,
	struct _UNI_EVENT_ATE_TEST_MODE_T *payload,
	UINT16 Len)
{
	struct _UNI_EVENT_ATE_TEST_MODE_T *prUniEvtAteTestMode = payload;
	struct _GET_FREQ_OFFSET_T *FREQ_Status;

	FREQ_Status = (struct _GET_FREQ_OFFSET_T *)&prUniEvtAteTestMode->aucAteResult[0];
	FREQ_Status->u4FreqOffset = le2cpu32(FREQ_Status->u4FreqOffset);
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk,
		&FREQ_Status->u4FreqOffset, sizeof(FREQ_Status->u4FreqOffset));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"FreqOffset:%d\n", FREQ_Status->u4FreqOffset);
}

static VOID UniEventGetTrParamHandle(
	struct cmd_msg *msg,
	struct _UNI_EVENT_ATE_TEST_MODE_T *payload,
	UINT16 Len)
{
	struct _UNI_EVENT_ATE_TEST_MODE_T *prUniEvtAteTestMode = (struct _UNI_EVENT_ATE_TEST_MODE_T *)payload;

	switch (prUniEvtAteTestMode->u2ParamIdx) {
	case UNI_ATE_TSSI_STATUS:
		UniEventGetTssiHandle(msg, (struct _UNI_EVENT_ATE_TEST_MODE_T *)payload, Len);
		break;

	case UNI_ATE_DPD_STATUS:
		UniEventGetDpdHandle(msg, (struct _UNI_EVENT_ATE_TEST_MODE_T *)payload, Len);
		break;

	case UNI_ATE_THERMO_COMP_STATUS:
		UniEventGetThermoCompHandle(msg, (struct _UNI_EVENT_ATE_TEST_MODE_T *)payload, Len);
		break;

	case UNI_ATE_FREQ_OFFSET:
		UniEventGetFreqOffsetHandle(msg, (struct _UNI_EVENT_ATE_TEST_MODE_T *)payload, Len);
		break;

	default:
		break;
	}
}

void UniEventTrParamHandleEvent(
	struct cmd_msg *msg,
	char *payload,
	UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(_UNI_EVENT_TEST_TR_PARAM_T);
	UINT32 event_fail_loop_cnt = 0;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	if ((TAG_LEN(tag) == 0) || (TAG_LEN(tag) > payload_len)) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"invalid tag length = %d\n",
			TAG_LEN(tag));
		return;
	}

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_TEST_TR_PARAM_CTRL:
			UniEventGetTrParamHandle(msg, (struct _UNI_EVENT_ATE_TEST_MODE_T *)tag, TAG_LEN(tag));
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				"invalid event tag = %d\n",
				TAG_ID(tag));
			break;
		}
	}
}

static VOID UniEventGetRxStat(
	struct cmd_msg *msg,
	struct _UNI_EVENT_TESTMODE_STAT_T *Data,
	UINT16 Len)
{
	struct _UNI_EVENT_TESTMODE_STAT_WB_T *pr_get_rx_stat_wb = (struct  _UNI_EVENT_TESTMODE_STAT_WB_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct _UNI_EVENT_TESTMODE_STAT_T *pr_get_rx_stat = (struct _UNI_EVENT_TESTMODE_STAT_T *)&pr_get_rx_stat_wb->rEvtTestmodeStat;
	struct _UNI_EVENT_TESTMODE_STAT_T *pr_get_rx_stat_rlt = (struct _UNI_EVENT_TESTMODE_STAT_T *)Data;
#ifdef CFG_BIG_ENDIAN
		int i = 0;
#endif
	/* Update rx stat */
	os_move_mem(pr_get_rx_stat, pr_get_rx_stat_rlt, sizeof(struct _UNI_EVENT_TESTMODE_STAT_T));
#ifdef CFG_BIG_ENDIAN
	pr_get_rx_stat->rInfoBand.u2MacRxFcsErrCnt
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2MacRxFcsErrCnt);
	pr_get_rx_stat->rInfoBand.u2MacRxLenMisMatch
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2MacRxLenMisMatch);
	pr_get_rx_stat->rInfoBand.u2MacRxFcsOkCnt
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2MacRxFcsOkCnt);
	pr_get_rx_stat->rInfoComm.u2MacRxFifoFull
		= le2cpu16(pr_get_rx_stat->rInfoComm.u2MacRxFifoFull);
	pr_get_rx_stat->rInfoBand.u4MacRxMdrdyCnt
		= le2cpu32(pr_get_rx_stat->rInfoBand.u4MacRxMdrdyCnt);
	pr_get_rx_stat->rInfoBand.u2PhyRxFcsErrCntCck
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxFcsErrCntCck);
	pr_get_rx_stat->rInfoBand.u2PhyRxFcsErrCntOfdm
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxFcsErrCntOfdm);
	pr_get_rx_stat->rInfoBand.u2PhyRxPdCck
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxPdCck);
	pr_get_rx_stat->rInfoBand.u2PhyRxPdOfdm
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxPdOfdm);
	pr_get_rx_stat->rInfoBand.u2PhyRxSigErrCck
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxSigErrCck);
	pr_get_rx_stat->rInfoBand.u2PhyRxSfdErrCck
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxSfdErrCck);
	pr_get_rx_stat->rInfoBand.u2PhyRxSigErrOfdm
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxSigErrOfdm);
	pr_get_rx_stat->rInfoBand.u2PhyRxTagErrOfdm
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxTagErrOfdm);
	pr_get_rx_stat->rInfoBand.u2PhyRxMdrdyCntCck
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxMdrdyCntCck);
	pr_get_rx_stat->rInfoBand.u2PhyRxMdrdyCntOfdm
		= le2cpu16(pr_get_rx_stat->rInfoBand.u2PhyRxMdrdyCntOfdm);
	pr_get_rx_stat->rInfoComm.u4AciHitLow
		= le2cpu32(pr_get_rx_stat->rInfoComm.u4AciHitLow);
	pr_get_rx_stat->rInfoComm.u4AciHitHigh
		= le2cpu32(pr_get_rx_stat->rInfoComm.u4AciHitHigh);

	for (i = 0; i < 4; i++) {
		pr_get_rx_stat->rInfoRXV[i].u2Rcpi = le2cpu16(pr_get_rx_stat->rInfoRXV[i].u2Rcpi);
		pr_get_rx_stat->rInfoRXV[i].i2Rssi = le2cpu16(pr_get_rx_stat->rInfoRXV[i].i2Rssi);
		pr_get_rx_stat->rInfoRXV[i].i2Snr = le2cpu16(pr_get_rx_stat->rInfoRXV[i].i2Snr);
	}
#endif
}

static VOID UniEventGetRxStatBand(
	struct cmd_msg *msg,
	struct _UNI_EVENT_TESTMODE_STAT_BAND_T *Data,
	uint16_t Len)
{
	struct _UNI_EVENT_TESTMODE_STAT_WB_T *pr_get_rx_stat_wb = (struct _UNI_EVENT_TESTMODE_STAT_WB_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct _UNI_EVENT_TESTMODE_STAT_BAND_T *dst = (struct _UNI_EVENT_TESTMODE_STAT_BAND_T *)&pr_get_rx_stat_wb->rEvtTestmodeStatBand;

	os_move_mem(dst, Data, sizeof(struct _UNI_EVENT_TESTMODE_STAT_BAND_T));
#ifdef CFG_BIG_ENDIAN
	dst->rInfoBand.u2MacRxFcsErrCnt = le2cpu16(dst->rInfoBand.u2MacRxFcsErrCnt);
	dst->rInfoBand.u2MacRxLenMisMatch = le2cpu16(dst->rInfoBand.u2MacRxLenMisMatch);
	dst->rInfoBand.u2MacRxFcsOkCnt = le2cpu16(dst->rInfoBand.u2MacRxFcsOkCnt);
	dst->rInfoBand.u4MacRxMdrdyCnt = le2cpu32(dst->rInfoBand.u4MacRxMdrdyCnt);
	dst->rInfoBand.u2PhyRxFcsErrCntCck = le2cpu16(dst->rInfoBand.u2PhyRxFcsErrCntCck);
	dst->rInfoBand.u2PhyRxFcsErrCntOfdm = le2cpu16(dst->rInfoBand.u2PhyRxFcsErrCntOfdm);
	dst->rInfoBand.u2PhyRxPdCck = le2cpu16(dst->rInfoBand.u2PhyRxPdCck);
	dst->rInfoBand.u2PhyRxPdOfdm = le2cpu16(dst->rInfoBand.u2PhyRxPdOfdm);
	dst->rInfoBand.u2PhyRxSigErrCck = le2cpu16(dst->rInfoBand.u2PhyRxSigErrCck);
	dst->rInfoBand.u2PhyRxSfdErrCck = le2cpu16(dst->rInfoBand.u2PhyRxSfdErrCck);
	dst->rInfoBand.u2PhyRxSigErrOfdm = le2cpu16(dst->rInfoBand.u2PhyRxSigErrOfdm);
	dst->rInfoBand.u2PhyRxTagErrOfdm = le2cpu16(dst->rInfoBand.u2PhyRxTagErrOfdm);
	dst->rInfoBand.u2PhyRxMdrdyCntCck = le2cpu16(dst->rInfoBand.u2PhyRxMdrdyCntCck);
	dst->rInfoBand.u2PhyRxMdrdyCntOfdm = le2cpu16(dst->->rInfoBand.u2PhyRxMdrdyCntOfdm);
#endif
}

static VOID UniEventGetRxStatPath(
	struct cmd_msg *msg,
	struct _UNI_EVENT_TESTMODE_STAT_PATH_T *Data,
	uint16_t Len)
{
	struct _UNI_EVENT_TESTMODE_STAT_WB_T *pr_get_rx_stat_wb = (struct _UNI_EVENT_TESTMODE_STAT_WB_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct _UNI_EVENT_TESTMODE_STAT_PATH_T *dst = (struct _UNI_EVENT_TESTMODE_STAT_PATH_T *)&pr_get_rx_stat_wb->rEvtTestmodeStatPath;

	os_move_mem(dst, Data, sizeof(struct _UNI_EVENT_TESTMODE_STAT_PATH_T));
}

static VOID UniEventGetRxStatUser(
	struct cmd_msg *msg,
	struct _UNI_EVENT_TESTMODE_STAT_USER_T *Data,
	uint16_t Len)
{
	struct _UNI_EVENT_TESTMODE_STAT_WB_T *pr_get_rx_stat_wb = (struct _UNI_EVENT_TESTMODE_STAT_WB_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct _UNI_EVENT_TESTMODE_STAT_USER_T *dst = (struct _UNI_EVENT_TESTMODE_STAT_USER_T *)&pr_get_rx_stat_wb->rEvtTestmodeStatUser;

	os_move_mem(dst, Data, sizeof(struct _TESTMODE_STATISTIC_INFO_USER));
#ifdef CFG_BIG_ENDIAN
	dst->i4FreqOffsetFromRx = le2cpu16(dst->i4FreqOffsetFromRx);
	dst->i4Snr = le2cpu16(dst->i4Snr);
	dst->u4FcsErrorCnt = le2cpu16(dst->u4FcsErrorCnt);
#endif
}

static VOID UniEventGetRxStatComm(
	struct cmd_msg *msg,
	struct _UNI_EVENT_TESTMODE_STAT_COMM_T *Data,
	uint16_t Len)
{
	struct _UNI_EVENT_TESTMODE_STAT_WB_T *pr_get_rx_stat_wb = (struct _UNI_EVENT_TESTMODE_STAT_WB_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct _UNI_EVENT_TESTMODE_STAT_COMM_T *dst = (struct _UNI_EVENT_TESTMODE_STAT_COMM_T *)&pr_get_rx_stat_wb->rEvtTestmodeStatComm;

	os_move_mem(dst, Data, sizeof(struct _UNI_EVENT_TESTMODE_STAT_COMM_T));
#ifdef CFG_BIG_ENDIAN
	dst->rInfoComm.u2MacRxFifoFull = le2cpu16(dst->rInfoComm.u2MacRxFifoFull);
	dst->rInfoComm.u4AciHitLow = le2cpu32(dst->rInfoComm.u4AciHitLow);
	dst->rInfoComm.u4AciHitHigh = le2cpu32(dst->rInfoComm.u4AciHitHigh);
#endif
}

static VOID UniEventGetRxStatAll(
	struct cmd_msg *msg,
	struct _UNI_EVENT_TESTMODE_STAT_ALL_V2_T *Data,
	uint16_t Len)
{
	struct _UNI_EVENT_TESTMODE_STAT_WB_T *pr_get_rx_stat_wb = (struct _UNI_EVENT_TESTMODE_STAT_WB_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct _UNI_EVENT_TESTMODE_STAT_ALL_V2_T *dst = (struct _UNI_EVENT_TESTMODE_STAT_ALL_V2_T *)&pr_get_rx_stat_wb->rEvtTestmodeStatAll;

	os_move_mem(dst, Data, sizeof(struct _UNI_EVENT_TESTMODE_STAT_ALL_V2_T));
#ifdef CFG_BIG_ENDIAN
	/* mac part */
	dst->rInfoBand.u2MacRxFcsErrCnt = le2cpu16(dst->rInfoBand.u2MacRxFcsErrCnt);
	dst->rInfoBand.u2MacRxLenMisMatch = le2cpu16(dst->rInfoBand.u2MacRxLenMisMatch);
	dst->rInfoBand.u2MacRxFcsOkCnt = le2cpu16(dst->rInfoBand.u2MacRxFcsOkCnt);
	dst->rInfoBand.u4MacRxMdrdyCnt = le2cpu32(dst->rInfoBand.u4MacRxMdrdyCnt);

	/* phy part */
	dst->rInfoBand.u2PhyRxFcsErrCntCck = le2cpu16(dst->rInfoBand.u2PhyRxFcsErrCntCck);
	dst->rInfoBand.u2PhyRxFcsErrCntOfdm = le2cpu16(dst->rInfoBand.u2PhyRxFcsErrCntOfdm);
	dst->rInfoBand.u2PhyRxPdCck = le2cpu16(dst->rInfoBand.u2PhyRxPdCck);
	dst->rInfoBand.u2PhyRxPdOfdm = le2cpu16(dst->rInfoBand.u2PhyRxPdOfdm);
	dst->rInfoBand.u2PhyRxSigErrCck = le2cpu16(dst->rInfoBand.u2PhyRxSigErrCck);
	dst->rInfoBand.u2PhyRxSfdErrCck = le2cpu16(dst->rInfoBand.u2PhyRxSfdErrCck);
	dst->rInfoBand.u2PhyRxSigErrOfdm = le2cpu16(dst->rInfoBand.u2PhyRxSigErrOfdm);
	dst->rInfoBand.u2PhyRxTagErrOfdm = le2cpu16(dst->rInfoBand.u2PhyRxTagErrOfdm);
	dst->rInfoBand.u2PhyRxMdrdyCntCck = le2cpu16(dst->rInfoBand.u2PhyRxMdrdyCntCck);
	dst->rInfoBand.u2PhyRxMdrdyCntOfdm = le2cpu16(dst->rInfoBand.u2PhyRxMdrdyCntOfdm);

	dst->rInfoComm.u2MacRxFifoFull = le2cpu16(dst->rInfoComm.u2MacRxFifoFull);
	dst->rInfoComm.u4AciHitLow = le2cpu32(dst->rInfoComm.u4AciHitLow);
	dst->rInfoComm.u4AciHitHigh = le2cpu32(dst->rInfoComm.u4AciHitHigh);

	dst->rInfoCommExt1.u4DrvRxCnt = le2cpu32(dst->rInfoCommExt1.u4DrvRxCnt);
	dst->rInfoCommExt1.u4Sinr = le2cpu32(dst->rInfoCommExt1.u4Sinr);
	dst->rInfoCommExt1.u4MuRxCnt = le2cpu32(dst->rInfoCommExt1.u4MuRxCnt);

	for (int i = 0; i < 8; i++) {
		dst->rInfoRXV.u2Rcpi = le2cpu16(dst->rInfoRXV[i].u2Rcpi);
		dst->rInfoRXV.i2Rssi = le2cpu16(dst->rInfoRXV[i].i2Rssi);
		dst->rInfoRXV.i2Snr = le2cpu16(dst->rInfoRXV[i].i2Snr);
		dst->rInfoRXV.i2AdcRssi = le2cpu16(dst->rInfoRXV[i].i2AdcRssi);
	}

	for (int i = 0; i < 16; i++) {
		dst->rInfoCommExt1[i].u4Sinr = le2cpu32(dst->rInfoUser[i].i4FreqOffsetFromRx);
		dst->rInfoCommExt1[i].u4MuRxCnt = le2cpu32(dst->rInfoUser[i].i4Snr);
	}

	for (int i = 0; i < 16; i++) {
		dst->rInfoUser[i].i4FreqOffsetFromRx = le2cpu32(dst->rInfoUser[i].i4FreqOffsetFromRx);
		dst->rInfoUser[i].i4Snr = le2cpu32(dst->rInfoUser[i].i4Snr);
		dst->rInfoUser[i].u4FcsErrorCnt = le2cpu32(dst->rInfoUser[i].u4FcsErrorCnt);
	}

#endif
}

void UniEventTestmodeRxStat(
	struct cmd_msg *msg,
	char *payload,
	uint16_t payload_len)
{
	UINT32 tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_TESTMODE_RX_STAT_T);
	uint32_t event_fail_loop_cnt = 0;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_WARN,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (uint8_t *)payload + fixed_len;

	if ((TAG_LEN(tag) == 0) || (TAG_LEN(tag) > payload_len)) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
			"invalid tag length = %d\n",
			TAG_LEN(tag));
		return;
	}

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_INFO,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_TESTMODE_RXV_REPORT:
			break;
		case UNI_EVENT_TESTMODE_RX_STAT:
			UniEventGetRxStat(msg, (struct _UNI_EVENT_TESTMODE_STAT_T *)tag, TAG_LEN(tag));
			break;
		case UNI_EVENT_TESTMODE_RX_STAT_BAND:
			UniEventGetRxStatBand(msg, (struct _UNI_EVENT_TESTMODE_STAT_BAND_T *)tag, TAG_LEN(tag));
			break;
		case UNI_EVENT_TESTMODE_RX_STAT_PATH:
			UniEventGetRxStatPath(msg, (struct _UNI_EVENT_TESTMODE_STAT_PATH_T *)tag, TAG_LEN(tag));
			break;
		case UNI_EVENT_TESTMODE_RX_STAT_USER:
			UniEventGetRxStatUser(msg, (struct _UNI_EVENT_TESTMODE_STAT_USER_T *)tag, TAG_LEN(tag));
			break;
		case UNI_EVENT_TESTMODE_RX_STAT_COMM:
			UniEventGetRxStatComm(msg, (struct _UNI_EVENT_TESTMODE_STAT_COMM_T *)tag, TAG_LEN(tag));
			break;
		case UNI_EVENT_TESTMODE_RX_STAT_ALL_V2:
			UniEventGetRxStatAll(msg, (struct _UNI_EVENT_TESTMODE_STAT_ALL_V2_T *)tag, TAG_LEN(tag));
			break;
		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_ATE, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				TAG_ID(tag));
			break;
		}
	}
}

void UniEventStaRecEvent(
	struct cmd_msg *msg,
	char *payload,
	uint16_t payload_len)
{
	UINT32 tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_STAREC_T);
	uint8_t *pBuffer = (uint8_t *)msg->attr.rsp.wb_buf_in_calbk;
	P_CMD_WTBL_PN_T pCmdWTBLPn = NULL;
	uint32_t event_fail_loop_cnt = 0;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (uint8_t *)payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));
		switch (TAG_ID(tag)) {
		case UNI_EVENT_STAREC_SEC_PN_INFO:
			pCmdWTBLPn = (P_CMD_WTBL_PN_T) pBuffer;
			if (pCmdWTBLPn == NULL)
				return;
			os_move_mem(pCmdWTBLPn, tag, sizeof(UNI_CMD_STAREC_SEC_PN_INFO_T));
			pBuffer += sizeof(CMD_WTBL_PN_T);
			break;
		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			"invalid event tag = %d\n",
			TAG_ID(tag));
		}
	}
}


/*****************************************
 * UNI_CMD_ID_VOW (Tag 0x37)
 *****************************************/
/* UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME (0x14) */
static VOID UniEventVowRxAtReportRxNonwifiTime(
	struct cmd_msg *msg,
	P_UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_EVENT_VOW_PARAM_T pVOWParam = (P_UNI_EVENT_VOW_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		P_UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME_T pVowRxAtReportRxNonwifiTime = &(pVOWParam->EventVowRxAtReportRxNonwifiTime);

		pVowRxAtReportRxNonwifiTime->ucRxNonWiFiBandIdx = TlvData->ucRxNonWiFiBandIdx;
		pVowRxAtReportRxNonwifiTime->u4RxNonWiFiBandTimer = le2cpu32(TlvData->u4RxNonWiFiBandTimer);
	}

	MTWF_DBG(NULL, DBG_CAT_RX, CATRX_VOW, DBG_LVL_DEBUG,
			"ucRxNonWiFiBandIdx = %d, u4RxNonWiFiBandTimer = %d\n",
			TlvData->ucRxNonWiFiBandIdx, le2cpu32(TlvData->u4RxNonWiFiBandTimer));
}

/* UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME (0x15) */
static VOID UniEventVowRxAtReportRxObssTime(
	struct cmd_msg *msg,
	P_UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_EVENT_VOW_PARAM_T pVOWParam = (P_UNI_EVENT_VOW_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		P_UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME_T pEventVowRxAtReportRxObssTime = &(pVOWParam->EventVowRxAtReportRxObssTime);

		pEventVowRxAtReportRxObssTime->ucRxObssBandIdx = TlvData->ucRxObssBandIdx;
		pEventVowRxAtReportRxObssTime->u4RxObssBandTimer = le2cpu32(TlvData->u4RxObssBandTimer);
	}

	MTWF_DBG(NULL, DBG_CAT_RX, CATRX_VOW, DBG_LVL_DEBUG,
			"ucRxObssBandIdx = %d, u4RxObssBandTimer = %d\n",
			TlvData->ucRxObssBandIdx, le2cpu32(TlvData->u4RxObssBandTimer));
}

/* UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME (0x16) */
static VOID UniEventVowRxAtReportMibObssTime(
	struct cmd_msg *msg,
	P_UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T TlvData)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		P_UNI_EVENT_VOW_PARAM_T pVOWParam = (P_UNI_EVENT_VOW_PARAM_T)msg->attr.rsp.wb_buf_in_calbk;
		P_UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME_T pEventVowRxAtReportRxObssTime = &(pVOWParam->EventVowRxAtReportMibObssTime);

		pEventVowRxAtReportRxObssTime->ucRxMibObssBandIdx = TlvData->ucRxMibObssBandIdx;
		pEventVowRxAtReportRxObssTime->u4RxMibObssBandTimer = le2cpu32(TlvData->u4RxMibObssBandTimer);
	}

	MTWF_DBG(NULL, DBG_CAT_RX, CATRX_VOW, DBG_LVL_DEBUG,
			"ucRxObssBandIdx = %d, u4RxObssBandTimer = %d\n",
			TlvData->ucRxMibObssBandIdx, le2cpu32(TlvData->u4RxMibObssBandTimer));
}

VOID UniEventVowHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(UNI_EVENT_VOW_T);

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_RX, CATRX_VOW, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_VOW_RX_AT_REPORT_RX_NONWIFI_TIME:
			UniEventVowRxAtReportRxNonwifiTime(msg, (void *)tag);
			break;
		case UNI_EVENT_VOW_RX_AT_REPORT_RX_OBSS_TIME:
			UniEventVowRxAtReportRxObssTime(msg, (void *)tag);
			break;
		case UNI_EVENT_VOW_RX_AT_REPORT_MIB_OBSS_TIME:
			UniEventVowRxAtReportMibObssTime(msg, (void *)tag);
			break;

		default:
			MTWF_DBG(NULL, DBG_CAT_RX, CATRX_VOW, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				TAG_ID(tag));
			break;
		}
	}
}

VOID UniEventCmdPollTxRxIdleRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	struct UNI_EVENT_CMD_RESULT_T *UniCmdResult = (struct UNI_EVENT_CMD_RESULT_T *)payload;
	struct _EVENT_EXT_CMD_RESULT_T *pResult =
				(struct _EVENT_EXT_CMD_RESULT_T *)msg->attr.rsp.wb_buf_in_calbk;

	if (pResult != NULL)
		pResult->u4Status = le2cpu32(UniCmdResult->u4Status);
}


/* UNI_EVENT_TWT_AGRT_EVENT_TAG_ALLOC (0x0) */
static VOID UniEventTwtAgrtAllocInfo(struct cmd_msg *msg, struct UNI_EVENT_TWT_AGRT_ALLOC_T *tag)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		struct TWT_AGRT_MGMT_T *pAgrtMgmt = (struct TWT_AGRT_MGMT_T *)msg->attr.rsp.wb_buf_in_calbk;

		pAgrtMgmt->sts = le2cpu32(tag->u4Sts);
		pAgrtMgmt->wcid = le2cpu16(tag->u2Wcid);
		pAgrtMgmt->res_id = le2cpu16(tag->u2ResId);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_WARN,
			"sts=%d,wcid=%d,res_id=%d\n",
			pAgrtMgmt->sts, pAgrtMgmt->wcid, pAgrtMgmt->res_id);
	}

}

/* UNI_EVENT_ID_TWT_INFO */
VOID UniEventTwtAgrtMgmtRsp(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_TWT_AGRT_INFO_T);
	UINT32 event_fail_loop_cnt = 0;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (UINT8 *)payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));
		switch (TAG_ID(tag)) {
		case UNI_EVENT_TWT_AGRT_EVENT_TAG_ALLOC:
			UniEventTwtAgrtAllocInfo(msg, (void *)tag);
			break;
		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"[BUG]:tag=%d, loop=%d\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				TAG_ID(tag));
		}
	}
}

VOID UniEventRfTestSolicitICapStatusCb(
	IN struct cmd_msg *msg,
	IN char *pData,
	IN uint16_t Length)
{
	struct _UNI_EVENT_RBIST_CAP_STATUS_T *pEventdata = (struct _UNI_EVENT_RBIST_CAP_STATUS_T *)(pData + sizeof(struct _UNI_EVENT_RF_TEST_TLV_T));

	UINT32 *pCapDone = (UINT32 *)msg->attr.rsp.wb_buf_in_calbk;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"----------------->\n");

	/* Update pCapDone */
	*pCapDone = le2cpu32(pEventdata->u4CapDone);
	MTWF_PRINT("\x1b[42m%s\x1b[m\n", (*pCapDone == TRUE) ?
			"Capture done!!" : "Not yet!!");

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"<-----------------\n");
}

VOID UniEventRfTestSolicitICapIQDataCb(
	IN struct cmd_msg *msg,
	IN char *pData,
	IN uint16_t Length)
{
	EXT_EVENT_RBIST_DUMP_DATA_T *pEventdata = (EXT_EVENT_RBIST_DUMP_DATA_T *)(pData + sizeof(struct _UNI_EVENT_RF_TEST_TLV_T));
	EXT_EVENT_RBIST_DUMP_DATA_T *pICapEvent = (EXT_EVENT_RBIST_DUMP_DATA_T *)msg->attr.rsp.wb_buf_in_calbk;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"----------------->\n");

	/* Update pICapEvent */
	os_move_mem(pICapEvent, pEventdata, sizeof(EXT_EVENT_RBIST_DUMP_DATA_T));

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"<-----------------\n");
}
static VOID UniEventRfTestTxToneCb(struct cmd_msg *pMsg, char *pData, uint16_t u2Len)
{
	struct _UNI_EVENT_RF_TEST_RESULT_T *prEvent = (struct _UNI_EVENT_RF_TEST_RESULT_T *)(pData + sizeof(struct _UNI_EVENT_RF_TEST_TLV_T));
	*pMsg->attr.rsp.wb_buf_in_calbk = 0;

	switch (prEvent->u4FuncIndex) {
	case GET_TX_TONE_GAIN_OFFSET:
		os_move_mem(pMsg->attr.rsp.wb_buf_in_calbk,
					(UINT32 *)&prEvent->aucEvent[0],
					sizeof(prEvent->aucEvent[0]));
		*pMsg->attr.rsp.wb_buf_in_calbk = le2cpu32(*pMsg->attr.rsp.wb_buf_in_calbk);
		break;
	}
}

static VOID UniEventATERFTestResp(struct cmd_msg *msg, char *data, uint16_t len)
{
	struct _UNI_EVENT_RF_TEST_RESULT_T *result = (struct _UNI_EVENT_RF_TEST_RESULT_T *)(data + sizeof(struct _UNI_EVENT_RF_TEST_TLV_T));

	switch (le2cpu32(result->u4FuncIndex)) {
	case RDD_TEST_MODE:
	case RE_CALIBRATION:
	case CALIBRATION_BYPASS:
		break;
#ifdef INTERNAL_CAPTURE_SUPPORT
	case GET_ICAP_CAPTURE_STATUS:
		UniEventRfTestSolicitICapStatusCb(msg, data, len);
		break;
	case GET_ICAP_RAW_DATA:
		UniEventRfTestSolicitICapIQDataCb(msg, data, len);
		break;
#endif/* INTERNAL_CAPTURE_SUPPORT */
	case GET_TX_TONE_GAIN_OFFSET:
		UniEventRfTestTxToneCb(msg, data, len);
		break;
	default:
		break;
	}
}

void UniEventTestCtrlHandleEvent(
	struct cmd_msg *msg,
	char *payload,
	uint16_t payload_len)
{
	UINT32 tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_TESTMODE_CTRL_T);
	uint32_t event_fail_loop_cnt = 0;

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (uint8_t *)payload + fixed_len;

	if ((TAG_LEN(tag) == 0) || (TAG_LEN(tag) > payload_len)) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid tag length = %d\n",
				TAG_LEN(tag));
		return;
	}

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				"Tag(%d, %d)\n",
				TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_RF_TEST_RESULT:
			UniEventATERFTestResp(msg, tag, TAG_LEN(tag));
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
				TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}

			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}
VOID UniEventCmdFastPathCalMICRsp(struct cmd_msg *msg, char *payload, uint16_t payload_len)
{
	struct _UNI_EVENT_FAST_PATH_PROCESS_T *pEventResult;
	struct _UNI_EVENT_FAST_PATH_PROCESS_T *pFastpath = (struct _UNI_EVENT_FAST_PATH_PROCESS_T *)msg->attr.rsp.wb_buf_in_calbk;
	//
	UINT32 tags_len;
	uint8_t *tag;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_ID_FAST_PATH_T);

	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (uint8_t *)payload + fixed_len;

	pEventResult = (struct _UNI_EVENT_FAST_PATH_PROCESS_T *)tag;
	if (pEventResult->u2Tag == UNI_EVENT_FAST_PATH_PROCESS) {

		pFastpath->u2Mic = pEventResult->u2Mic;
		pFastpath->u4KeybitmapMatchStatus = pEventResult->u4KeybitmapMatchStatus;
		pFastpath->ucKeynum = pEventResult->ucKeynum;
		memcpy(pFastpath->cap.u4Keybitmap, pEventResult->cap.u4Keybitmap, 16);
		pFastpath->cap.ucSupportFastPath = pEventResult->cap.ucSupportFastPath;
		pFastpath->cap.ucVersion = pEventResult->cap.ucVersion;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"ucKeynum:%d, u2Mic (%x),version=%d\n",
				 pFastpath->ucKeynum, pFastpath->u2Mic, pEventResult->cap.ucVersion);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"MatchStatus:%x, ExtCmd (0x%02x)\n",
				 pFastpath->u4KeybitmapMatchStatus, msg->attr.ext_type);
	}
	return;

}
void UniEventTestEngineHandleEvent(
	struct cmd_msg *msg,
	char *payload,
	uint16_t payload_len)
{
	UINT32 tags_len;
	uint8_t *tag;
	UINT32 fixed_len = sizeof(struct _UNI_EVENT_TESTMODE_CTRL_T);
	struct _UNI_EVENT_RF_TEST_RESULT_T *result;


	if (payload_len < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			payload_len);
		return;
	}

	tags_len = payload_len - fixed_len;
	tag = (uint8_t *)payload + fixed_len;

	result = (struct _UNI_EVENT_RF_TEST_RESULT_T *)(tag + sizeof(struct _UNI_EVENT_RF_TEST_TLV_T));

	if (msg->attr.rsp.wb_buf_in_calbk)
		*(msg->attr.rsp.wb_buf_in_calbk) = result->u4PayloadLength;
}

UINT32 UniEventNicCapHwVersion(
	struct _RTMP_ADAPTER *ad, UINT8 *buff)
{
	struct NIC_CAPABILITY_V2_HW_VERSION *hw_info =
		(struct NIC_CAPABILITY_V2_HW_VERSION *)buff;

	PD_SET_HW_VER(ad->physical_dev, hw_info->hw_ver);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"::chip_id=0x%04x, hw_ver=0x%02x, mac_ver=0x%04x, top_ver=%d, bbp_id=%d\n",
			hw_info->chip_id,
			hw_info->hw_ver,
			hw_info->mac_ver,
			hw_info->top_ver,
			hw_info->bbp_id);

	return NDIS_STATUS_SUCCESS;
}

UINT32 UniEventNicCapEfuseVersion(
	struct _RTMP_ADAPTER *ad, UINT8 *buff)
{
	struct NIC_CAPABILITY_V2_EFUSE_VERSION *efuse_ver =
		(struct NIC_CAPABILITY_V2_EFUSE_VERSION *)buff;
	u8 wtbl_support = 0;

	PD_SET_GEN_DOWN(ad->physical_dev, efuse_ver->part_number & BIT(0));
	wtbl_support = (efuse_ver->part_number & BITS(28, 31)) >> 28;
	PD_SET_HW_WTBL_SUPPORT(ad->physical_dev, wtbl_support);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"::part_number=0x%x\n",
			efuse_ver->part_number);

	return NDIS_STATUS_SUCCESS;
}

VOID UniEventNicCapHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct EVENT_NIC_CAPABILITY_V2);
	UINT32 event_fail_loop_cnt = 0;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)msg->priv;

	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));
		switch (TAG_ID(tag)) {
		case UNI_EVENT_CHIP_CONFIG_CAP_HW_VERSION:
			UniEventNicCapHwVersion(ad, tag+TAG_HDR_LEN);
			break;
		case UNI_EVENT_CHIP_CONFIG_CAP_EFUSE_VERSION:
			UniEventNicCapEfuseVersion(ad, tag+TAG_HDR_LEN);
			break;
		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					"[BUG]: invalid event tag = %d, over max loop (%d) fail check!\n",
					TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				"event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

void uni_event_epcs_result(
	struct _RTMP_ADAPTER *ad,
	UINT8 *data,
	UINT32 len)
{
	struct UNI_EVENT_EPCS_RESULT_T *prUniEventEPCS = (struct UNI_EVENT_EPCS_RESULT_T *) data;
	struct EPCS_EVENT_RESULT_T *prEPCSEvtResult;
	struct EPCS_EVENT_QUERY_STA_T *prEPCSEvtQuerySTA;
	struct EPCS_EDCA_PARAM_T *prEPCSEDCAParam;
	UINT32 u4Cmd;
	UINT_8 ucAC;

	u4Cmd = prUniEventEPCS->u4Cmd;
	MTWF_DBG(ad, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "EPCS Cmd=%u\n", u4Cmd);

	switch (u4Cmd) {
	case EPCS_ENABLE_STA:
		prEPCSEvtResult = (struct EPCS_EVENT_RESULT_T *)&prUniEventEPCS->au4Result[0];
		MTWF_DBG(ad, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "EPCS Cmd=%u, Result=%u", u4Cmd, prEPCSEvtResult->u4Result);
		break;
	case EPCS_QUERY_STA:
		prEPCSEvtQuerySTA = (struct EPCS_EVENT_QUERY_STA_T *)&prUniEventEPCS->au4Result[0];
		MTWF_DBG(ad, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "EPCS STA%u Edca Entry=%u, Enable=%u\n", prEPCSEvtQuerySTA->u2WlanIdx,
			prEPCSEvtQuerySTA->u2Idx, prEPCSEvtQuerySTA->fgEnable);

		for (ucAC = 0; ucAC < AC_NUM; ucAC++) {
			MTWF_DBG(ad, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR, "AC[%u] EPCS Edca Param:\n", ucAC);

			prEPCSEDCAParam = &prEPCSEvtQuerySTA->EdcaSetting.EdcaParam[ucAC];
			MTWF_DBG(ad, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"TxopLimit=%u,CWMin=%u,CWMax=%u,Aifsn=%u\n", prEPCSEDCAParam->u2TxopLimit, prEPCSEDCAParam->ucCWmin,
				 prEPCSEDCAParam->ucCWmax, prEPCSEDCAParam->ucAifsn);
			MTWF_DBG(ad, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"MuEdcaCWMin=%u,MuEdcaCWMax=%u,MuEdcaAifsn=%u,MuEdcaTimer=%u\n", prEPCSEDCAParam->ucMuEdcaCWmin,
				prEPCSEDCAParam->ucMuEdcaCWmax, prEPCSEDCAParam->ucMuEdcaAifsn, prEPCSEDCAParam->ucMuEdcaTimer);
		}
		break;
	default:
		break;
	}
}

void UniEventEPCSHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_EPCS_T);
	UINT32 event_fail_loop_cnt = 0;
	struct _RTMP_ADAPTER *pAd = physical_device_get_first_mac_adapter(physical_dev);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
			"%s: invalid length = %d\n",
			__func__, Length);
		return;
	}

	tags_len = Length - fixed_len;
	tag = pData + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {

		switch (TAG_ID(tag)) {
		case UNI_EVENT_EPCS_RESULT:
			if (TAG_LEN(tag) != sizeof(struct UNI_EVENT_EPCS_RESULT_T)) {
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
					"%s: tag: %d, invalid length = %d",
					__func__, TAG_ID(tag), TAG_LEN(tag));
				break;
			}
			uni_event_epcs_result(pAd, tag,
				sizeof(struct UNI_EVENT_EPCS_RESULT_T));
			break;

		default:
			event_fail_loop_cnt++;
			if (event_fail_loop_cnt > MAX_EVENT_FAIL_LOOP) {
				MTWF_DBG(NULL, DBG_CAT_TX, CATTX_DABS_QOS, DBG_LVL_ERROR,
				"%s: [BUG]: invalid event tag = %d, over max loop (%d) fail!\n",
				__func__, TAG_ID(tag), MAX_EVENT_FAIL_LOOP);
				return;
			}
			break;
		}
	}
}

VOID UniEventBssMatchHandler(
	VOID *physical_dev,
	UINT8 *pData,
	UINT32 Length,
	EVENT_RXD *event_rxd)
{
	struct UNI_EVENT_BSS_MATCH_T *event = (struct UNI_EVENT_BSS_MATCH_T *)pData;
	struct UNI_EVENT_BSS_MATCH_DATA_T *tlv = (struct UNI_EVENT_BSS_MATCH_DATA_T *)&(event->aucTlvBuffer[0]);
	UINT32 tags_len;
	UINT32 fixed_len = sizeof(*event);

	if (Length < fixed_len) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
			"invalid length = %d\n",
			Length);
		return;
	}

	tags_len = Length - fixed_len;

	switch (tlv->u2Tag) {
	case UNI_EVENT_BSS_MATCH_DATA:
		MTWF_PRINT("%s() tlv->u2Tag == UNI_EVENT_BSS_MATCH_DATA\n", __func__);
		MTWF_PRINT("%s() event->ucBssIndex = %d\n", __func__, event->ucBssIndex);
		MTWF_PRINT("%s() tlv->u2Tag = %x\n", __func__, tlv->u2Tag);
		MTWF_PRINT("%s() tlv->u2Length = %x\n", __func__, tlv->u2Length);
		MTWF_PRINT("%s() tlv->ucFail = %x\n", __func__, tlv->ucFail);
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			"Tag = %u\n", tlv->u2Tag);
		break;
	}
}

/**
 * @Solicited event handler of MLD cmd
 *  cmd:UNI_CMD_MLD_AT2LM_RES_REQ->event:UNI_EVENT_MLD_AT2LM_RES_RSP
 *
 * @param *cmd_msg cmd message
 * @param *fixed event common part
 * @param *tlv event tlv part
 */
static VOID UniEventMldAt2lmResRspProc(
	struct cmd_msg *msg,
	struct UNI_EVENT_MLD_T *fixed,
	struct UNI_EVENT_MLD_AT2LM_RSP_T *tlv)
{
	if (msg->attr.rsp.wb_buf_in_calbk) {
		UINT32 tsf_0, tsf_1;
		struct AT2LM_RES_RSP_CTRL_T *pAt2lmRsp = (struct AT2LM_RES_RSP_CTRL_T *)msg->attr.rsp.wb_buf_in_calbk;

		pAt2lmRsp->ucSts = tlv->ucSts;
		NdisMoveMemory(pAt2lmRsp->aucMldAddr, fixed->aucMldAddr, MAC_ADDR_LEN);
		pAt2lmRsp->ucFwMldIdx = fixed->ucFwMldIdx;
		pAt2lmRsp->ucAt2lmId = tlv->ucAt2lmIdx;
		pAt2lmRsp->ucBssInfoIdx = tlv->ucBssInfIdx;

		tsf_0 = le2cpu32(tlv->u4MstTsf[0]);
		tsf_1 = le2cpu32(tlv->u4MstTsf[1]);
		pAt2lmRsp->u8MstTsf = (UINT64)tsf_0 + ((UINT64)tsf_1 << 32);
		tsf_0 = le2cpu32(tlv->u4DisEndTsf[0]);
		tsf_1 = le2cpu32(tlv->u4DisEndTsf[1]);
		pAt2lmRsp->u8EdTsf = (UINT64)tsf_0 + ((UINT64)tsf_1 << 32);
	}
}

/**
 * @Solicited event handler of MLD cmd
 *
 * @param *cmd_msg cmd message
 * @param *payload event body(common + tlv)
 * @param payload_len length of event body
 */
VOID UniEventMldHandler(struct cmd_msg *msg, char *payload, UINT16 payload_len)
{
	UINT32 tags_len;
	UINT8 *tag;
	UINT8 *fixed;
	UINT16 offset = 0;
	UINT32 fixed_len = sizeof(struct UNI_EVENT_MLD_T);

	fixed = payload;
	tags_len = payload_len - fixed_len;
	tag = payload + fixed_len;

	TAG_FOR_EACH(tag, tags_len, offset) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_DEBUG,
			"Tag(%d, %d)\n",
			TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MLD_AT2LM_RES_RSP:
			UniEventMldAt2lmResRspProc(msg, (void *)fixed, (void *)tag);
			break;
		default:
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_T2LM, DBG_LVL_ERROR,
				"invalid event tag = %d\n",
				 TAG_ID(tag));
			break;
		}
	}
}

#ifdef ZERO_PKT_LOSS_SUPPORT
VOID uni_event_wcid_skip_tx_status_handler(struct cmd_msg *msg, char *rsp_payload,
						UINT16 rsp_payload_len)
{
	struct _UNI_EVENT_WTBL_SKIP_TX_STATUS_RESULT_T *prSkipTxStatus =
		(struct _UNI_EVENT_WTBL_SKIP_TX_STATUS_RESULT_T *)msg->attr.rsp.wb_buf_in_calbk;
	struct UNI_EVENT_STATE_T *event = (struct UNI_EVENT_STATE_T *)rsp_payload;
	struct _UNI_EVENT_WTBL_SKIP_TX_STATUS_T *prEventWtblSkipTxStatus =
		(struct _UNI_EVENT_WTBL_SKIP_TX_STATUS_T *) &(event->au1TlvBuffer[0]);

	prSkipTxStatus->u2WlanIdx = le2cpu32(prEventWtblSkipTxStatus->u2WlanIdx);
	prSkipTxStatus->fgWtblSkipTXEnable = le2cpu32(prEventWtblSkipTxStatus->fgWtblSkipTXEnable);
	MTWF_DBG(NULL, DBG_CAT_TX, CATTX_TMAC, DBG_LVL_INFO,
		"wcid(%d) SkipTxEnable(%d)\n",
		prSkipTxStatus->u2WlanIdx, prSkipTxStatus->fgWtblSkipTXEnable);
}
#endif /* ZERO_PKT_LOSS_SUPPORT */
#endif /* WIFI_UNIFIED_COMMAND */
