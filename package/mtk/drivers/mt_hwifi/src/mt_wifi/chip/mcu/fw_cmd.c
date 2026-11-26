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
	fw_cmd.c
*/

#include "rt_config.h"


INT32 CmdInitAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg = {0};
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "address = %x, data = %x\n", address, data);
	msg = AndesAllocCmdMsg(ad, sizeof(struct _INIT_CMD_ACCESS_REG));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ACCESS_REG);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));
	access_reg.ucSetQuery = 1;
	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);
	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));
	ret = AndesSendCmdMsg(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}


static VOID CmdInitAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _INIT_EVENT_ACCESS_REG *access_reg =
		(struct _INIT_EVENT_ACCESS_REG *)data;
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &access_reg->u4Data, len - 4);
	*((UINT32 *)(msg->attr.rsp.wb_buf_in_calbk)) =
		le2cpu32(*((UINT32 *)msg->attr.rsp.wb_buf_in_calbk));
}


INT32 CmdInitAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg = {0};
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret = 0;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "address = %x\n", address);
	msg = AndesAllocCmdMsg(pAd, sizeof(struct _INIT_CMD_ACCESS_REG));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, INIT_CMD_ACCESS_REG);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, data);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdInitAccessRegReadCb);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));
	access_reg.ucSetQuery = 0;
	access_reg.u4Address = cpu2le32(address);
	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}


static VOID CmdHIFLoopbackRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "HIF Loopback status=%d\n", Status);

	switch (Status) {
	case TARGET_ADDRESS_LEN_SUCCESS:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "Request target address and length success\n");
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Unknow Status(%d)\n", Status);
		break;
	}
}


INT32 CmdHIFLoopbackReq(RTMP_ADAPTER *ad, UINT32 enable, UINT32 qidx)
{
	struct cmd_msg *msg;
	UINT32 value;
	int ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(ad, 4);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, MT_HIF_LOOPBACK);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_NA_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdHIFLoopbackRsp);
	AndesInitCmdMsg(msg, attr);
	/* start enable */
	enable = (qidx << 16) | (enable & 0xffff);
	value = cpu2le32(enable);
	MTWF_DBG(ad, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "loopback value=0x%x\n", value);
	AndesAppendCmdMsg(msg, (char *)&value, 4);
	ret = AndesSendCmdMsg(ad, msg);
error:
	MTWF_DBG(ad, DBG_CAT_FW, CATFW_DBGINFO, (ret != NDIS_STATUS_SUCCESS) ? DBG_LVL_ERROR:DBG_LVL_INFO,
			 "(ret = %d)\n", ret);
	return ret;
}





INT32 CmdChPrivilege(RTMP_ADAPTER *ad, UINT8 Action, UINT8 control_chl,
					 UINT8 central_chl, UINT8 BW, UINT8 TXStream, UINT8 RXStream)
{
	struct cmd_msg *msg;
	struct _CMD_CH_PRIVILEGE_T ch_privilege = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	if (central_chl == 0) {
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "central channel = 0 is invalid\n");
		return -1;
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "control_chl = %d, central_chl = %d, BW = %d, TXStream = %d, RXStream = %d\n",
			  control_chl, central_chl,
			  BW, TXStream, RXStream);
	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_CH_PRIVILEGE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, CMD_CH_PRIVILEGE);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&ch_privilege, sizeof(ch_privilege));
	ch_privilege.ucAction = Action;
	ch_privilege.ucPrimaryChannel = control_chl;

	if (BW == BAND_WIDTH_20)
		ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCN;
	else if (BW == BAND_WIDTH_40) {
		if (control_chl < central_chl)
			ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCA;
		else
			ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCB;
	} else {
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "unknown bandwidth = %d\n", BW);
	}

	if (central_chl > 14)
		ch_privilege.ucRfBand =  CMD_CH_PRIV_BAND_A;
	else
		ch_privilege.ucRfBand = CMD_CH_PRIV_BAND_G;

	ch_privilege.ucRfChannelWidth = CMD_CH_PRIV_CH_WIDTH_20_40;
	ch_privilege.ucReqType = CMD_CH_PRIV_REQ_JOIN;
	AndesAppendCmdMsg(msg, (char *)&ch_privilege, sizeof(ch_privilege));

	ret = AndesSendCmdMsg(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}


INT32 CmdAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "address = %x, data = %x\n", address, data);
	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_ACCESS_REG_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, CMD_ACCESS_REG);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));
	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);
	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));
	ret = AndesSendCmdMsg(ad, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}


static VOID CmdAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _CMD_ACCESS_REG_T *access_reg = (struct _CMD_ACCESS_REG_T *)data;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "\n");
	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &access_reg->u4Data, len - 4);
	*((UINT32 *)(msg->attr.rsp.wb_buf_in_calbk)) =
		le2cpu32(*((UINT32 *)msg->attr.rsp.wb_buf_in_calbk));
}


INT32 CmdAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg = {0};
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_ACCESS_REG_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, CMD_ACCESS_REG);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, data);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdAccessRegReadCb);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));
	access_reg.u4Address = cpu2le32(address);
	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));
	ret = AndesSendCmdMsg(pAd, msg);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "address = %x, value = %x\n",
			  address, *data);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}


VOID CmdIOWrite32(void *hdev_ctrl, UINT32 Offset, UINT32 Value)
{
	struct MCU_CTRL *Ctl = hc_get_mcu_ctrl(hdev_ctrl);
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	RTMP_REG_PAIR RegPair;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (Ctl->fwdl_ctrl.stage == FWDL_STAGE_FW_RUNNING) {
		RegPair.Register = Offset;
		RegPair.Value = Value;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdMultipleMacRegAccessWrite(ad, &RegPair, 1);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdMultipleMacRegAccessWrite(ad, &RegPair, 1);
	} else
		CmdInitAccessRegWrite(ad, Offset, Value);
}


VOID CmdIORead32(void *hdev_ctrl, UINT32 Offset, UINT32 *Value)
{
	struct MCU_CTRL *Ctl = hc_get_mcu_ctrl(hdev_ctrl);
	struct _RTMP_ADAPTER *ad = hc_get_hdev_privdata(hdev_ctrl);
	RTMP_REG_PAIR RegPair;
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(hdev_ctrl);
#endif /* WIFI_UNIFIED_COMMAND */

	if (Ctl->fwdl_ctrl.stage == FWDL_STAGE_FW_RUNNING) {
		RegPair.Register = Offset;
		RegPair.Value = 0;
#ifdef WIFI_UNIFIED_COMMAND
		if (cap->uni_cmd_support)
			UniCmdMultipleMacRegAccessRead(ad, &RegPair, 1);
		else
#endif /* WIFI_UNIFIED_COMMAND */
			MtCmdMultipleMacRegAccessRead(ad, &RegPair, 1);
		*Value = RegPair.Value;
	} else
		CmdInitAccessRegRead(ad, Offset, Value);
}


static VOID EventExtNicCapability(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	EXT_EVENT_NIC_CAPABILITY *ExtEventNicCapability =
		(EXT_EVENT_NIC_CAPABILITY *)Data;
	UINT32 Loop;

	MTWF_PRINT("The data code of firmware:");

	for (Loop = 0; Loop < 16; Loop++) {
		MTWF_PRINT("%c", ExtEventNicCapability->aucDateCode[Loop]);
	}

	MTWF_PRINT("\nThe version code of firmware:");

	for (Loop = 0; Loop < 12; Loop++) {
		MTWF_PRINT("%c", ExtEventNicCapability->aucVersionCode[Loop]);
	}
}

INT32 CmdExtRtsThenCtsRetryCnt(
	struct _RTMP_ADAPTER *pAd,
	UINT16 u2WlanIdx,
	UINT8 u1Tid,
	UINT8 u1RtsFailThenCtsRetryCnt)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	struct EXT_CMD_SET_RTS_THEN_CTS_RETRY rRtsThenCtsRetry;

	msg = AndesAllocCmdMsg(pAd, sizeof(struct EXT_CMD_SET_RTS_THEN_CTS_RETRY));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RTS_THEN_CTS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	/* Fill command related header here*/
	rRtsThenCtsRetry.u2Wcid = u2WlanIdx;
	rRtsThenCtsRetry.u1Tid   = u1Tid;
	rRtsThenCtsRetry.u1RtsFailThenCtsRetryCnt = u1RtsFailThenCtsRetryCnt;
	AndesAppendCmdMsg(msg, (char *)&rRtsThenCtsRetry, sizeof(struct EXT_CMD_SET_RTS_THEN_CTS_RETRY));

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "set wcid=%u, tid=%u, rts2cts=%u\n", u2WlanIdx, u1Tid, u1RtsFailThenCtsRetryCnt);

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdNicCapability(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_NIC_CAPABILITY);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 28);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtNicCapability);
	AndesInitCmdMsg(msg, attr);
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}

#ifdef CONFIG_ATE
static VOID EventExtCmdResult(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
#ifdef CFG_BIG_ENDIAN
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
#endif
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "u4Status = 0x%x\n",
			  EventExtCmdResult->u4Status);
}

INT32 CmdTxContinous(RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW,
					 UINT32 PriCh, UINT32 Mcs, UINT32 WFSel, UCHAR onoff)
{
	struct cmd_msg *msg;
	struct _CMD_TEST_CTRL_T ContiTXParam = {0};
	INT32 ret = 0;
	UCHAR TXDRate = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "mode:0x%x, bw:0x%x, prich(Control CH):0x%x, mcs:0x%x, wfsel:0x%x, on/off:0x%x\n",
			  PhyMode, BW, PriCh, Mcs, WFSel, onoff);
	msg = AndesAllocCmdMsg(pAd, sizeof(ContiTXParam));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RF_TEST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&ContiTXParam, sizeof(ContiTXParam));
	ContiTXParam.ucAction = ACTION_IN_RFTEST;

	if (onoff == 0)
		ContiTXParam.u.rRfATInfo.u4FuncIndex = CONTINUOUS_TX_STOP;
	else
		ContiTXParam.u.rRfATInfo.u4FuncIndex = CONTINUOUS_TX_START;

	/* 0: All 1:TX0 2:TX1 */
	ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucCentralCh = PriCh;

	if (BW_40 == BW || BW_80 == BW)
		ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucCtrlCh = (PriCh + 2);
	else
		ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucCtrlCh = PriCh;

	ContiTXParam.u.rRfATInfo.Data.rConTxParam.ucAntIndex = WFSel;

	if (PhyMode == 0) { /* CCK */
		switch (Mcs) {
		/* long preamble */
		case 0:
			TXDRate = 0;
			break;

		case 1:
			TXDRate = 1;
			break;

		case 2:
			TXDRate = 2;
			break;

		case 3:
			TXDRate = 3;
			break;

		/* short preamble */
		case 9:
			TXDRate = 5;
			break;

		case 10:
			TXDRate = 6;
			break;

		case 11:
			TXDRate = 7;
			break;
		}
	} else if (PhyMode == 1) { /* OFDM */
		switch (Mcs) {
		case 0:
			TXDRate = 11;
			break;

		case 1:
			TXDRate = 15;
			break;

		case 2:
			TXDRate = 10;
			break;

		case 3:
			TXDRate = 14;
			break;

		case 4:
			TXDRate = 9;
			break;

		case 5:
			TXDRate = 13;
			break;

		case 6:
			TXDRate = 8;
			break;

		case 7:
			TXDRate = 12;
			break;
		}
	} else if (2 == PhyMode || 3 == PhyMode || 4 == PhyMode) {
		/* 2. MODULATION_SYSTEM_HT20 ||3. MODULATION_SYSTEM_HT40 || 4. VHT */
		TXDRate = Mcs;
	}

	ContiTXParam.u.rRfATInfo.Data.rConTxParam.u2RateCode = Mcs << 6 | TXDRate;
#ifdef CFG_BIG_ENDIAN
	ContiTXParam.u.rRfATInfo.u4FuncIndex = cpu2le32(ContiTXParam.u.rRfATInfo.u4FuncIndex);
	ContiTXParam.u.rRfATInfo.Data.rConTxParam.u2RateCode =
		cpu2le16(ContiTXParam.u.rRfATInfo.Data.rConTxParam.u2RateCode);
#endif
	AndesAppendCmdMsg(msg, (char *)&ContiTXParam, sizeof(ContiTXParam));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}

INT32 CmdTxTonePower(RTMP_ADAPTER *pAd, INT32 type, INT32 dec)
{
	struct cmd_msg *msg;
	struct _CMD_TEST_CTRL_T TestCtrl;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(TestCtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "type:%d, dec:%d\n", type, dec);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_RF_TEST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&TestCtrl, sizeof(TestCtrl));
	TestCtrl.ucAction = ACTION_IN_RFTEST;
	TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(type);

	/* 0: All 1:TX0 2:TX1 */
	switch (ATECtrl->tx_ant) {
	case 0:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = 0;
		break;

	case 1:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = 1;
		break;

	case 2:
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex = 2;
		break;

	default:
		/* for future more than 3*3 ant */
		TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucAntIndex =
			ATECtrl->tx_ant - 1;
		break;
	}

	TestCtrl.u.rRfATInfo.Data.rTxToneGainParam.ucTonePowerGain = dec;
	AndesAppendCmdMsg(msg, (char *)&TestCtrl, sizeof(TestCtrl));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(ret = %d)\n", ret);
	return ret;
}
#endif /* CONFIG_ATE */


#ifdef COEX_SUPPORT
INT AndesCoexOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Status)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_WIFI_STATUS_UPDATE_T coex_status;
	UINT32 SetBtWlanStatus;

	ret = 0;
	os_zero_mem(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	os_zero_mem(&coex_status, sizeof(COEX_WIFI_STATUS_UPDATE_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_BT_COEX);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	coext_t.ucSubOpCode = COEX_WIFI_STATUS_UPDATE;
	coex_status.u4WIFIStatus = 0x00;
	SetBtWlanStatus = pAd->BtWlanStatus;

	switch (Status) {
	case STATUS_RADIO_ON:
		SetBtWlanStatus |= COEX_STATUS_RADIO_ON;
		break;

	case STATUS_RADIO_OFF:
		SetBtWlanStatus &= ~(COEX_STATUS_RADIO_ON);
		break;

	case STATUS_SCAN_G_BAND:
		SetBtWlanStatus |= COEX_STATUS_SCAN_G_BAND;
		break;

	case STATUS_SCAN_G_BAND_END:
		SetBtWlanStatus &= ~(COEX_STATUS_SCAN_G_BAND);
		break;

	case STATUS_SCAN_A_BAND:
		SetBtWlanStatus |= COEX_STATUS_SCAN_A_BAND;
		break;

	case STATUS_SCAN_A_BAND_END:
		SetBtWlanStatus &= ~(COEX_STATUS_SCAN_A_BAND);
		break;

	case STATUS_LINK_UP:
		SetBtWlanStatus |= COEX_STATUS_LINK_UP;
		break;

	case STATUS_LINK_DOWN:
		SetBtWlanStatus &= ~(COEX_STATUS_LINK_UP);
		break;

	case STATUS_BT_OVER_WIFI:
		SetBtWlanStatus |= COEX_STATUS_BT_OVER_WIFI;
		break;

	default: /* fatal error */
		break;
	} /* End of switch */

	if (SetBtWlanStatus == pAd->BtWlanStatus)
		goto error;
	else
		pAd->BtWlanStatus = SetBtWlanStatus;

	coex_status.u4WIFIStatus = pAd->BtWlanStatus;
	/* Parameter */
#ifdef CFG_BIG_ENDIAN
	coex_status.u4WIFIStatus = cpu2le32(coex_status.u4WIFIStatus);
#endif
	os_move_mem(coext_t.aucData, &coex_status,
				sizeof(COEX_WIFI_STATUS_UPDATE_T));
	hex_dump("AndesBtCoex: ", (UCHAR *)&coext_t,
			 sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t,
					  sizeof(EXT_CMD_COEXISTENCE_T));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "ret = %d\n", ret);
	return ret;
}

INT AndesCoexProtectionFrameOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Mode,
	IN UCHAR Rate)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_SET_PROTECTION_FRAME_T coex_proction;

	ret = 0;
	os_zero_mem(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	os_zero_mem(&coex_proction, sizeof(COEX_SET_PROTECTION_FRAME_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_BT_COEX);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	coext_t.ucSubOpCode = COEX_SET_PROTECTION_FRAME;
	coex_proction.ucProFrameMode = Mode;
	coex_proction.ucProFrameRate = Rate;
	os_move_mem(coext_t.aucData, &coex_proction,
				sizeof(COEX_SET_PROTECTION_FRAME_T));
	hex_dump("AndesBtCoexProtection: ",
			 (UCHAR *)&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "ret = %d\n", ret);
	return ret;
}


INT AndesCoexBSSInfo(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN Enable,
	IN UCHAR bQoS)
{
	struct cmd_msg *msg;
	struct _CMD_ATTRIBUTE attr = {0};
	INT32 ret;
	EXT_CMD_COEXISTENCE_T coext_t;
	COEX_UPDATE_BSS_INFO_T coex_bss_info;

	ret = 0;
	os_zero_mem(&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	os_zero_mem(&coex_bss_info, sizeof(COEX_UPDATE_BSS_INFO_T));
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_COEXISTENCE_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_BT_COEX);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	coext_t.ucSubOpCode = COEX_UPDATE_BSS_INFO;

	if (Enable) {
		coex_bss_info.u4BSSPresence[0] = 0x1;
		coex_bss_info.u4IsQBSS[0] = bQoS;
	} else {
		coex_bss_info.u4BSSPresence[0] = 0x0;
		coex_bss_info.u4IsQBSS[0] = 0;
	}

	os_move_mem(coext_t.aucData, &coex_bss_info, sizeof(COEX_UPDATE_BSS_INFO_T));
	hex_dump("AndesBtCoexProtection: ", (UCHAR *)&coext_t,
			 sizeof(EXT_CMD_COEXISTENCE_T));
	AndesAppendCmdMsg(msg, (char *)&coext_t, sizeof(EXT_CMD_COEXISTENCE_T));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
			 "ret = %d\n", ret);
	return ret;
}
#endif


/* WTBL manipulation function*/
PNDIS_PACKET WtblTlvBufferAlloc(RTMP_ADAPTER *pAd,  UINT32 u4AllocateSize)
{
	PNDIS_PACKET net_pkt = NULL;

	net_pkt = RTMP_AllocateFragPacketBuffer(pAd, u4AllocateSize);

	if (net_pkt == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Can not allocate net_pkt\n");
		return NULL;
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "Allocate(%p)\n", net_pkt);
	return net_pkt;
}

VOID WtblTlvBufferAppend(PNDIS_PACKET pWtblTlvBuffer, UINT16 u2Type,
						 UINT16 u2Length, PUCHAR pNextWtblTlvBuffer)
{
	if ((pNextWtblTlvBuffer != NULL) && (u2Length != 0)) {
		P_CMD_WTBL_GENERIC_TLV_T pWtblGenericTlv =
			(P_CMD_WTBL_GENERIC_TLV_T)pNextWtblTlvBuffer;
		pWtblGenericTlv->u2Tag = u2Type;
		pWtblGenericTlv->u2Length = u2Length;
		os_move_mem(OS_PKT_TAIL_BUF_EXTEND(pWtblTlvBuffer, u2Length),
					pNextWtblTlvBuffer, u2Length);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "T(%d), L(%d), V(%p)\n",
				  pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length,
				  pNextWtblTlvBuffer);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Can not append WTBL TLV\n");
	}
}

/* WTBL TLV buffer free*/
VOID WtblTlvBufferFree(RTMP_ADAPTER *pAd, PNDIS_PACKET pWtblTlvBuffer)
{
	if (pWtblTlvBuffer != NULL) {
		RTMPFreeNdisPacket(pAd, pWtblTlvBuffer);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "Free buffer(%p)\n", pWtblTlvBuffer);
	} else {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "Fail to free buffer(%p)\n", pWtblTlvBuffer);
	}
}

VOID *pTlvAppend(VOID *pTlvBuffer, UINT16 u2Type, UINT16 u2Length,
				 VOID *pNextTlvBuffer, UINT32 *pu4TotalTlvLen, UCHAR *pucTotalTlvNumber)
{
	if ((pNextTlvBuffer != NULL) && (u2Length != 0)) {
		P_CMD_WTBL_GENERIC_TLV_T pWtblGenericTlv =
			(P_CMD_WTBL_GENERIC_TLV_T)pNextTlvBuffer;
		pWtblGenericTlv->u2Tag = cpu2le16(u2Type);
		pWtblGenericTlv->u2Length = cpu2le16(u2Length);
		*pu4TotalTlvLen += u2Length;
		*pucTotalTlvNumber += 1;
		os_move_mem((PUCHAR)pTlvBuffer, (PUCHAR)pWtblGenericTlv, u2Length);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "T(%d), L(%d), V(%p)\n",
				  pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length,
				  pNextTlvBuffer);
		return (pTlvBuffer + u2Length);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "Can not append WTBL TLV\n");
	return NULL;
}

static VOID CmdExtTlvUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "EventExtCmdResult.u4Status = 0x%x\n",
			  EventExtCmdResult->u4Status);
}


INT32 CmdExtTlvBufferSend(
	RTMP_ADAPTER *pAd,
	UINT8 ExtCmdType,
	VOID *pTlvBuffer,
	UINT32 u4TlvLength)
{
	struct cmd_msg	*msg = NULL;
	INT32			Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	/* Allocte CMD msg */
	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_STAREC_COMMON_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, ExtCmdType);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtTlvUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	/* Copy TLV buffer to CMD msg */
	AndesAppendCmdMsg(msg, (char *)pTlvBuffer, u4TlvLength);
	/* Send out CMD msg */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}


static VOID CmdExtWtblUpdateCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)data;
	UINT32	u4Len = len - 20 - sizeof(CMD_WTBL_UPDATE_T);
	PUCHAR	pData = (PCHAR)(data + 20 + sizeof(CMD_WTBL_UPDATE_T));
	PUCHAR	pRspPayload = (PUCHAR)msg->attr.rsp.wb_buf_in_calbk;

	if (pRspPayload == NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "EventExtCmdResult.ucExTenCID = 0x%x\n",
				  EventExtCmdResult->ucExTenCID);
		EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "EventExtCmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->u4Status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "Copy query result to buffer\n");
		os_move_mem(pRspPayload, pData, u4Len);
	}
}

INT32 CmdExtWtblUpdate(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucOperation,
					   VOID *pBuffer, UINT32 u4BufferLen)
{
	struct cmd_msg			*msg = NULL;
	CMD_WTBL_UPDATE_T	CmdWtblUpdate = {0};
	INT32					Ret = 0;
	UINT64					u8EnableFeature = 0;
	UINT16					ucTLVNumber = 0;
	UINT16					u2Len = 0;
	UINT32					*pRspPayload = NULL;
	P_CMD_WTBL_GENERIC_TLV_T	pWtblGenericTlv = NULL;
	UINT8					ucRemainingTLVNumber = 0;
	UINT32					u4RemainingTLVBufLen = 0;
	PUCHAR					TempBuffer = (PUCHAR)pBuffer;
	struct _CMD_ATTRIBUTE attr = {0};
#ifdef WIFI_UNIFIED_COMMAND
	RTMP_CHIP_CAP 			*pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pChipCap->uni_cmd_support) {
		return UniCmdWtblUpdate(pAd, u2WlanIdx, ucOperation, pBuffer, u4BufferLen);
	}
#endif /* WIFI_UNIFIED_COMMAND */
	msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_WTBL_INFO + 100);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	/* Get TVL number from TLV buffer*/
	u4RemainingTLVBufLen = u4BufferLen;

	while (u4RemainingTLVBufLen > 0) {
		pWtblGenericTlv = (P_CMD_WTBL_GENERIC_TLV_T)TempBuffer;

		if (pWtblGenericTlv == NULL) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					 "pWtblGenericTlv is NULL\n");
			Ret = NDIS_STATUS_INVALID_DATA;
			break;
		} else if (pWtblGenericTlv->u2Length == 0) {
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
					 "fail to handle T(%d), L(%d)\n", pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length);
			Ret = NDIS_STATUS_INVALID_DATA;
			break;
		}

		u8EnableFeature |= ((UINT64)1 << pWtblGenericTlv->u2Tag);
		TempBuffer += pWtblGenericTlv->u2Length;
		u4RemainingTLVBufLen -= pWtblGenericTlv->u2Length;
		ucTLVNumber++;
	}

	if (Ret != 0)
		goto Error1;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucTLVNumber = %d\n", ucTLVNumber);
	/* Assign vlaue to related parameters */
	TempBuffer = (PUCHAR)pBuffer;
	ucRemainingTLVNumber = ucTLVNumber;
	u4RemainingTLVBufLen = u4BufferLen;

	/* Set correct length and RspPayload buffer with different operation */
	if (ucOperation == RESET_WTBL_AND_SET) {
		if (pBuffer != NULL || u4RemainingTLVBufLen == 0) {
			/* Reset a specific WCID and set WTBL TLV */
			u2Len = 8;
			pRspPayload = NULL;
		}
	} else if (ucOperation == SET_WTBL) {
		if (pBuffer != NULL) {
			u2Len = 8;
			pRspPayload = NULL;
		} else
			return NDIS_STATUS_FAILURE;
	} else if (ucOperation == QUERY_WTBL) {
		if (pBuffer != NULL) {
			u2Len = u4BufferLen + 20 + sizeof(CMD_WTBL_UPDATE_T);
			pRspPayload = (UINT32 *)pBuffer;
		} else
			return NDIS_STATUS_FAILURE;
	} else if (ucOperation == RESET_ALL_WTBL) {
		if (pBuffer == NULL && u4RemainingTLVBufLen == 0) {
			u2Len = 8;
			pRspPayload = NULL;
		} else
			return NDIS_STATUS_FAILURE;
	} else {
		/* Error */
		return NDIS_STATUS_FAILURE;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_WTBL_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, u2Len);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, pRspPayload);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtWtblUpdateCb);
	AndesInitCmdMsg(msg, attr);
	WCID_SET_H_L(CmdWtblUpdate.ucWlanIdxHnVer, CmdWtblUpdate.ucWlanIdxL, u2WlanIdx);
	CmdWtblUpdate.ucOperation = ucOperation;
	CmdWtblUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	AndesAppendCmdMsg(msg, (char *)&CmdWtblUpdate, sizeof(CMD_WTBL_UPDATE_T));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "u2WlanIdx = %d, ucOperation = %d, u8EnableFeature = 0x%llx\n",
			  u2WlanIdx, ucOperation, u8EnableFeature);
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucRemainingTLVNumber = %d, u2RemainingTLVBufLen = %d\n",
			  ucRemainingTLVNumber, u4RemainingTLVBufLen);

	/* Handle TVL request here */
	while ((ucRemainingTLVNumber > 0) && (u4RemainingTLVBufLen > 0)) {
		/* Get current TLV */
		pWtblGenericTlv = (P_CMD_WTBL_GENERIC_TLV_T)TempBuffer;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "TLV(%d, %d)\n",
				  pWtblGenericTlv->u2Tag, pWtblGenericTlv->u2Length);

		/* Handle this TLV */
		switch (pWtblGenericTlv->u2Tag) {
		/* Tag = 0 */
		case WTBL_GENERIC: {
			P_CMD_WTBL_GENERIC_T pCmdWtblGeneric =
				(P_CMD_WTBL_GENERIC_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_GENERIC), ucMUARIndex = %d, ucSkipTx = %d,ucCfAck = %d, ucQos = %d, ucQos = %d, ucAdm = %d, PartialAID = %d, aucPeerAddress"MACSTR")\n",
						  pCmdWtblGeneric->ucMUARIndex,
						  pCmdWtblGeneric->ucSkipTx,
						  pCmdWtblGeneric->ucCfAck,
						  pCmdWtblGeneric->ucQos,
						  pCmdWtblGeneric->ucMesh,
						  pCmdWtblGeneric->ucAdm,
						  pCmdWtblGeneric->u2PartialAID,
						  MAC2STR(pCmdWtblGeneric->aucPeerAddress));
#ifdef CFG_BIG_ENDIAN
				pCmdWtblGeneric->u2PartialAID = cpu2le16(pCmdWtblGeneric->u2PartialAID);
#endif
			} else if (ucOperation == QUERY_WTBL) {
				/* No need to fill real parameters when query and
				  *	just append all zero data buffer to msg
				  */
			}

			break;
		}

		/* Tag = 1 */
		case WTBL_RX: {
			P_CMD_WTBL_RX_T	pCmdWtblRx = (P_CMD_WTBL_RX_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_RX), ucRcid = %d, ucRca1 = %d, ucRca2 = %d, ucRv = %d\n",
						  pCmdWtblRx->ucRcid,
						  pCmdWtblRx->ucRca1,
						  pCmdWtblRx->ucRca2,
						  pCmdWtblRx->ucRv);
			}

			break;
		}

		/* Tag = 2 */
		case WTBL_HT: {
			P_CMD_WTBL_HT_T pCmdWtblHt = (P_CMD_WTBL_HT_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_HT), ucHt = %d, ucLdpc = %d, ucAf = %d, ucMm = %d\n",
						  pCmdWtblHt->ucHt,
						  pCmdWtblHt->ucLdpc,
						  pCmdWtblHt->ucAf,
						  pCmdWtblHt->ucMm);
			}

			break;
		}

		/* Tag = 3 */
		case WTBL_VHT: {
			P_CMD_WTBL_VHT_T pCmdWtblVht = (P_CMD_WTBL_VHT_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_VHT), ucLdpcVht = %d, ucDynBw= %d, ucVht = %d, ucTxopPsCap = %d\n",
						  pCmdWtblVht->ucLdpcVht,
						  pCmdWtblVht->ucDynBw,
						  pCmdWtblVht->ucVht,
						  pCmdWtblVht->ucTxopPsCap);
			}

			break;
		}

		/* Tag = 4 */
		case WTBL_PEER_PS: {
			P_CMD_WTBL_PEER_PS_T pCmdWtblPerPs =
				(P_CMD_WTBL_PEER_PS_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_PEER_PS), ucDuIPsm = %d, ucIPsm = %d\n",
						  pCmdWtblPerPs->ucDuIPsm,
						  pCmdWtblPerPs->ucIPsm);
			}

			break;
		}

		/* Tag = 5 */
		case WTBL_TX_PS: {
			P_CMD_WTBL_TX_PS_T	pCmdWtbTxPs = (P_CMD_WTBL_TX_PS_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_TX_PS), ucTxPs = %d\n",
						  pCmdWtbTxPs->ucTxPs);
			}

			break;
		}

		/* Tag = 6 */
		case WTBL_HDR_TRANS: {
			P_CMD_WTBL_HDR_TRANS_T	pCmdWtblHdrTrans =
				(P_CMD_WTBL_HDR_TRANS_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_HDR_TRANS), ucTd = %d, ucFd = %d, ucDisRhtr =%d\n",
						  pCmdWtblHdrTrans->ucTd,
						  pCmdWtblHdrTrans->ucFd,
						  pCmdWtblHdrTrans->ucDisRhtr);
			}

			break;
		}

		/* Tag = 7 */
		case WTBL_SECURITY_KEY: {
			P_CMD_WTBL_SECURITY_KEY_T pCmdWtblSecurityKey =
				(P_CMD_WTBL_SECURITY_KEY_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_SECURITY_KEY), ucAddRemove = %d, ucRkv = %d, ucIkv =%d, ucAlgorithmId = %d, ucKeyId = %d, ucKeyLen = %d\n",
						  pCmdWtblSecurityKey->ucAddRemove,
						  pCmdWtblSecurityKey->ucRkv,
						  pCmdWtblSecurityKey->ucIkv,
						  pCmdWtblSecurityKey->ucAlgorithmId,
						  pCmdWtblSecurityKey->ucKeyId,
						  pCmdWtblSecurityKey->ucKeyLen);
			}

			break;
		}

		/* Tag = 8 */
		case WTBL_BA: {
			P_CMD_WTBL_BA_T pCmdWtblBa = (P_CMD_WTBL_BA_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				if (pCmdWtblBa->ucBaSessionType == BA_SESSION_RECP) {
					/* Recipient */
					MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
							 "(WTBL_BA, Recipient), ucTid(%d), ucBaSessionType(%d), ucRstBaTid(%d), ucRstBaSel(%d), ucStartRstBaSb(%d), aucPeerAddress"MACSTR")\n",
							  pCmdWtblBa->ucTid,
							  pCmdWtblBa->ucBaSessionType,
							  pCmdWtblBa->ucRstBaTid,
							  pCmdWtblBa->ucRstBaSel,
							  pCmdWtblBa->ucStartRstBaSb,
							  MAC2STR(pCmdWtblBa->aucPeerAddress));
				} else {
					/* Originator */
					MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
							 "(WTBL_BA, Originator), ucTid(%d), u2Sn(%d), ucBaEn(%d), u2BaWinSize(%d), ucBaWinSizeIdx(%d)\n",
							  pCmdWtblBa->ucTid,
							  pCmdWtblBa->u2Sn,
							  pCmdWtblBa->ucBaEn,
							  pCmdWtblBa->u2BaWinSize,
							  pCmdWtblBa->ucBaWinSizeIdx);
				}
			}
#ifdef CFG_BIG_ENDIAN
			pCmdWtblBa->u2Sn = cpu2le16(pCmdWtblBa->u2Sn);
			pCmdWtblBa->u2BaWinSize = cpu2le16(pCmdWtblBa->u2BaWinSize);
#endif
			break;
		}

		/* Tag = 9 */
		case WTBL_RDG: {
			P_CMD_WTBL_RDG_T	pCmdWtblRdg = (P_CMD_WTBL_RDG_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_RDG), ucRdgBa = %d, ucR = %d\n",
						  pCmdWtblRdg->ucRdgBa,
						  pCmdWtblRdg->ucR);
			}

			break;
		}

		/* Tag = 10 */
		case WTBL_PROTECTION: {
			P_CMD_WTBL_PROTECTION_T	pCmdWtblProtection =
				(P_CMD_WTBL_PROTECTION_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_PROTECTION), ucRts = %d\n",
						  pCmdWtblProtection->ucRts);
			}

			break;
		}

		/* Tag = 11 */
		case WTBL_CLEAR: {
			/* bit 0: Clear PSM (WF_WTBLON: 0x60322300, Bit 31 set 1 then set 0) */
			/* bit 1: Clear BA (WTBL2.DW15) */
			/* bit 2: Clear Rx Counter (6019_00002, bit 14) */
			/* bit 3: Clear Tx Counter (6019_0000, bit 15) */
			/* bit 4: Clear ADM Counter (6019_0000, bit 12) */
			/* bit 5: Clear Cipher key (WTBL3)*/
			P_CMD_WTBL_CLEAR_T	pCmdWtblClear = (P_CMD_WTBL_CLEAR_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_CLEAR), ucClear = %x\n",
						  pCmdWtblClear->ucClear);
			}

			break;
		}

		/* Tag = 12 */
		case WTBL_BF: {
			P_CMD_WTBL_BF_T		pCmdWtblBf = (P_CMD_WTBL_BF_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						"(WTBL_BF), ucTiBf = %d, ucTeBf = %d, ucTibfVh = %d, ucTebfVht = %d, ucGid = %d, ucPfmuIdx = %d\n",
						 pCmdWtblBf->ucTiBf,
						 pCmdWtblBf->ucTeBf,
						 pCmdWtblBf->ucTibfVht,
						 pCmdWtblBf->ucTebfVht,
						 pCmdWtblBf->ucGid,
						 pCmdWtblBf->ucPFMUIdx);
			}

			break;
		}

		/* Tag = 13 */
		case WTBL_SMPS: {
			P_CMD_WTBL_SMPS_T pCmdWtblSmPs = (P_CMD_WTBL_SMPS_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_SMPS), ucSmPs = %d\n",
						  pCmdWtblSmPs->ucSmPs);
			}

			break;
		}

		/* Tag = 14 */
		case WTBL_RAW_DATA_RW: {
			P_CMD_WTBL_RAW_DATA_RW_T pCmdWtblRawDataRw =
				(P_CMD_WTBL_RAW_DATA_RW_T)TempBuffer;
			/* Print argumrnts */
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
					 "(WTBL_RAW_DATA_RW), ucWtblIdx = %d, ucWhichDW = %d, u4DwMask = 0x%x, u4DwValue = 0x%x\n",
					  pCmdWtblRawDataRw->ucWtblIdx,
					  pCmdWtblRawDataRw->ucWhichDW,
					  pCmdWtblRawDataRw->u4DwMask,
					  pCmdWtblRawDataRw->u4DwValue);
#ifdef CFG_BIG_ENDIAN
			pCmdWtblRawDataRw->u4DwMask = cpu2le32(pCmdWtblRawDataRw->u4DwMask);
			pCmdWtblRawDataRw->u4DwValue = cpu2le32(pCmdWtblRawDataRw->u4DwValue);
#endif
			break;
		}

		/* Tag = 15 */
		case WTBL_PN: {
			P_CMD_WTBL_PN_T pCmdWtblPn = (P_CMD_WTBL_PN_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_PN), PN = "MACSTR"\n",
						  MAC2STR(pCmdWtblPn->aucPn));
			}

			break;
		}

		/* Tag = 16 */
		case WTBL_SPE: {
			P_CMD_WTBL_SPE_T		pCmdWtblSpe = (P_CMD_WTBL_SPE_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						"(WTBL_BF), ucSpeIdx = %d\n",
						 pCmdWtblSpe->ucSpeIdx);
			}

			break;
		}

		/* Tag = 17 */
		case WTBL_SECURITY_KEY_V2: {
			P_CMD_WTBL_SECURITY_KEY_V2_T pCmdWtblSecurityKey =
				(P_CMD_WTBL_SECURITY_KEY_V2_T)TempBuffer;

			if ((ucOperation == RESET_WTBL_AND_SET) || (ucOperation == SET_WTBL)) {
				/* Print argumrnts */
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "(WTBL_SECURITY_KEY_V2), ucAddRemove = %d, ucEntryCount = %d\n",
						  pCmdWtblSecurityKey->ucAddRemove,
						  pCmdWtblSecurityKey->ucEntryCount);
			}

			break;
		}

#ifdef MGMT_TXPWR_CTRL
		/* Tag = 18*/
		case WTBL_RATE: {
			P_CMD_WTBL_RATE_T pCmdWtblRate = (P_CMD_WTBL_RATE_T)TempBuffer;

			if (ucOperation == SET_WTBL) {
				/* Print argumrnts */
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
						"(WTBL_RATE), wcid=%d  Rate= %d\n",
						 u2WlanIdx, pCmdWtblRate->u2Rate1);
			}
			break;
		}

		/* Tag = 19*/
		case WTBL_PWR_OFFSET: {
			P_CMD_WTBL_PWR_T pCmdWtblPwr = (P_CMD_WTBL_PWR_T)TempBuffer;

			if (ucOperation == SET_WTBL) {
				/* Print argumrnts */
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
						"(WTBL_PWR_OFFSET), wcid=%d  Pwr= %d\n",
						 u2WlanIdx, pCmdWtblPwr->ucPwrOffset);
			}
			break;
		}
#endif

		default: {
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
					 "Unknown WTBL TLV Tag(%d)\n",
					  pWtblGenericTlv->u2Tag);
			break;
		}
		}

		/* Advance to next TLV */
		TempBuffer += pWtblGenericTlv->u2Length;
		u4RemainingTLVBufLen -= pWtblGenericTlv->u2Length;
		ucRemainingTLVNumber--;
#ifdef CFG_BIG_ENDIAN
		pWtblGenericTlv->u2Length = cpu2le16(pWtblGenericTlv->u2Length);
		pWtblGenericTlv->u2Tag = cpu2le16(pWtblGenericTlv->u2Tag);
#endif
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "in while loop, ucRemainingTLVNumber = %d, u2RemainingTLVBufLen = %d\n",
				  ucRemainingTLVNumber, u4RemainingTLVBufLen);
	}

	AndesAppendCmdMsg(msg, (PUCHAR)pBuffer, u4BufferLen);

	/* Send out CMD */
	if (pWtblGenericTlv && (pWtblGenericTlv->u2Tag == WTBL_SECURITY_KEY_V2) && (ucOperation == SET_WTBL))
		call_fw_cmd_notifieriers(WO_CMD_STA_REC, pAd, msg->net_pkt);
	Ret = AndesSendCmdMsg(pAd, msg);

	goto Success;
Error1:

	if (msg)
		AndesFreeCmdMsg(msg);

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			 "Ret = %d)\n", Ret);
	return Ret;
Success:
Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

UINT32 WtblDwQuery(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx,
				   UINT8 ucWtbl1234, UINT8 ucWhichDW)
{
	CMD_WTBL_RAW_DATA_RW_T	rWtblRawDataRwWtblDw = {0};

	rWtblRawDataRwWtblDw.u2Tag = WTBL_RAW_DATA_RW;
	rWtblRawDataRwWtblDw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);
	rWtblRawDataRwWtblDw.ucWtblIdx = ucWtbl1234;
	rWtblRawDataRwWtblDw.ucWhichDW = ucWhichDW;
	CmdExtWtblUpdate(pAd, u2WlanIdx, QUERY_WTBL, &rWtblRawDataRwWtblDw,
					 rWtblRawDataRwWtblDw.u2Length);
	return rWtblRawDataRwWtblDw.u4DwValue;
}

INT32 WtblDwSet(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucWtbl1234,
				UINT8 ucWhichDW, UINT32 u4DwMask, UINT32 u4DwValue)
{
	INT32 ret;
	CMD_WTBL_RAW_DATA_RW_T	rWtblRawDataRwWtblDw = {0};

	rWtblRawDataRwWtblDw.u2Tag = WTBL_RAW_DATA_RW;
	rWtblRawDataRwWtblDw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);
	rWtblRawDataRwWtblDw.ucWtblIdx = ucWtbl1234;
	rWtblRawDataRwWtblDw.ucWhichDW = ucWhichDW;
	rWtblRawDataRwWtblDw.u4DwMask = u4DwMask;
	rWtblRawDataRwWtblDw.u4DwValue = u4DwValue;
	ret = CmdExtWtblUpdate(pAd, u2WlanIdx, SET_WTBL, &rWtblRawDataRwWtblDw,
						   rWtblRawDataRwWtblDw.u2Length);
	return ret;
}

INT32 WtblResetAndDWsSet(RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, UINT8 ucWtbl1234,
						 INT dw_cnt, struct cmd_wtbl_dw_mask_set *dw_set)
{
	INT32 ret = 0;
	CMD_WTBL_RAW_DATA_RW_T  rWtblRawDataRwWtblDw;
	INT cmd_cnt;
	UINT8 cmd_op;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "\n");

	for (cmd_cnt = 0; cmd_cnt < dw_cnt; cmd_cnt++) {
		NdisZeroMemory((UCHAR *)&rWtblRawDataRwWtblDw, sizeof(CMD_WTBL_RAW_DATA_RW_T));

		if (cmd_cnt == 0)
			cmd_op = RESET_WTBL_AND_SET;
		else
			cmd_op = SET_WTBL;

		rWtblRawDataRwWtblDw.u2Tag = WTBL_RAW_DATA_RW;
		rWtblRawDataRwWtblDw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);
		rWtblRawDataRwWtblDw.ucWtblIdx = ucWtbl1234;
		rWtblRawDataRwWtblDw.ucWhichDW = dw_set[cmd_cnt].ucWhichDW;
		rWtblRawDataRwWtblDw.u4DwMask = dw_set[cmd_cnt].u4DwMask;
		rWtblRawDataRwWtblDw.u4DwValue = dw_set[cmd_cnt].u4DwValue;
		ret = CmdExtWtblUpdate(pAd, u2WlanIdx, cmd_op, &rWtblRawDataRwWtblDw,
							   rWtblRawDataRwWtblDw.u2Length);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "cmd_cnt/WlanIdx/Tag/Length/WtblIdx/WhichDW/DwMask/DwValue/ret=%d/%d/%d/%d/%d/%d/0x%x/0x%x/%d\n",
				  cmd_cnt, u2WlanIdx, rWtblRawDataRwWtblDw.u2Tag, rWtblRawDataRwWtblDw.u2Length,
				  rWtblRawDataRwWtblDw.ucWtblIdx, rWtblRawDataRwWtblDw.ucWhichDW,
				  rWtblRawDataRwWtblDw.u4DwMask, rWtblRawDataRwWtblDw.u4DwValue, ret);
	}

	return ret;
}


/**
 * @addtogroup bss_dev_sta_info
 * @{
 * @name bss info/device info/sta record firmware command
 * @{
 */
static VOID CmdExtDevInfoUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_DEVINFO_UPDATE_T EventExtCmdResult = (P_EVENT_DEVINFO_UPDATE_T)Data;

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	EventExtCmdResult->u2TotalElementNum = le2cpu16(EventExtCmdResult->u2TotalElementNum);

	if (EventExtCmdResult->u4Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR,
				 "BUG::EID(0x%x), CmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO,
				 "EID(0x%x), CmdResult.u4Status = 0x%x (OM:%d, EleNum:%d)\n",
				  EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status,
				  EventExtCmdResult->ucOwnMacIdx, EventExtCmdResult->u2TotalElementNum);
	}
}

static VOID CmdExtStaRecUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_STAREC_UPDATE_T EventExtCmdResult = (P_EVENT_STAREC_UPDATE_T)(Data);

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	EventExtCmdResult->u2TotalElementNum = le2cpu16(EventExtCmdResult->u2TotalElementNum);
	/* We can consider move this to caller */
	msg->cmd_return_status = EventExtCmdResult->u4Status;

	if (EventExtCmdResult->u4Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
				 "BUG::EID(0x%x), CmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
				 "EID(0x%x), CmdResult.u4Status = 0x%x (Bss:%d, Wcid:%d, EleNum:%d)\n",
				  EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status,
				  EventExtCmdResult->ucBssInfoIdx,
				  WCID_GET_H_L(EventExtCmdResult->ucWlanIdxHnVer, EventExtCmdResult->ucWlanIdxL),
				  EventExtCmdResult->u2TotalElementNum);
	}
}

static VOID CmdExtStaRecDeleteRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_STAREC_UPDATE_T EventExtCmdResult = (P_EVENT_STAREC_UPDATE_T)(Data);

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	EventExtCmdResult->u2TotalElementNum = le2cpu16(EventExtCmdResult->u2TotalElementNum);
	/* We can consider move this to caller */
	msg->cmd_return_status = EventExtCmdResult->u4Status;

	if (EventExtCmdResult->u4Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
			"BUG::EID(0x%x), CmdResult.u4Status = 0x%x\n",
			EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status);
	} else {
		MAC_TABLE_ENTRY *pEntry = NULL;
		RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
		UINT_16 wcid = WCID_GET_H_L(EventExtCmdResult->ucWlanIdxHnVer, EventExtCmdResult->ucWlanIdxL);
		pEntry = entry_get(ad, wcid);
		pEntry->sta_rec_valid = FALSE;

		MTWF_DBG(ad, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"EID(0x%x), CmdResult.u4Status = 0x%x, StaRecValid:%d (Bss:%d, Wcid:%d, EleNum:%d)\n",
			EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status, pEntry->sta_rec_valid,
			EventExtCmdResult->ucBssInfoIdx, WCID_GET_H_L(EventExtCmdResult->ucWlanIdxHnVer, EventExtCmdResult->ucWlanIdxL),
			EventExtCmdResult->u2TotalElementNum);
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				" (pEntry Addr = %02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr));
	}
}

static VOID CmdExtStaRecInsertRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_STAREC_UPDATE_T EventExtCmdResult = (P_EVENT_STAREC_UPDATE_T)(Data);

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	EventExtCmdResult->u2TotalElementNum = le2cpu16(EventExtCmdResult->u2TotalElementNum);
	/* We can consider move this to caller */
	msg->cmd_return_status = EventExtCmdResult->u4Status;

	if (EventExtCmdResult->u4Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
			"BUG::EID(0x%x), CmdResult.u4Status = 0x%x\n",
			EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status);
	} else {
		MAC_TABLE_ENTRY *pEntry = NULL;
		RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
		UINT_16 wcid = WCID_GET_H_L(EventExtCmdResult->ucWlanIdxHnVer, EventExtCmdResult->ucWlanIdxL);
		pEntry = entry_get(ad, wcid);
		pEntry->sta_rec_valid = TRUE;

		MTWF_DBG(ad, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"EID(0x%x), CmdResult.u4Status = 0x%x, StaRecValid:%d (Bss:%d, Wcid:%d, EleNum:%d)\n",
			EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status, pEntry->sta_rec_valid,
			EventExtCmdResult->ucBssInfoIdx, WCID_GET_H_L(EventExtCmdResult->ucWlanIdxHnVer, EventExtCmdResult->ucWlanIdxL),
			EventExtCmdResult->u2TotalElementNum);
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			" (pEntry Addr = %02x:%02x:%02x:%02x:%02x:%02x)\n",  PRINT_MAC(pEntry->Addr));
	}

}

static VOID CmdExtBssInfoUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_BSSINFO_UPDATE_T EventExtCmdResult = (P_EVENT_BSSINFO_UPDATE_T)Data;

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	EventExtCmdResult->u2TotalElementNum = le2cpu16(EventExtCmdResult->u2TotalElementNum);

	if (EventExtCmdResult->u4Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
				 "BUG::EID(0x%x), CmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
				 "EID(0x%x), CmdResult.u4Status = 0x%x (Bss:%d, EleNum:%d)\n",
				  EventExtCmdResult->ucExtenCID, EventExtCmdResult->u4Status,
				  EventExtCmdResult->ucBssInfoIdx, EventExtCmdResult->u2TotalElementNum);
	}
}

/* WTBL TLV update */
INT32 CmdExtDevInfoUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT64 u8EnableFeature)
{
	struct cmd_msg			*msg = NULL;
	CMD_DEVINFO_UPDATE_T	CmdDeviceInfoUpdate = {0};
	INT32					Ret = 0;
	UINT8					i = 0;
	UINT16					ucTLVNumber = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	/* Allocate buffer */
	msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_DEVICEINFO);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	for (i = 0; i < DEVINFO_MAX_NUM; i++) {
		if (u8EnableFeature & (1 << i))
			ucTLVNumber++;
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO,
		"Active = %d, OwnMacIdx = %d, band = %d "MACSTR"), TLV Num = %d\n",
		Active, OwnMacIdx, BandIdx,
		MAC2STR(OwnMacAddr), ucTLVNumber);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_DEVINFO_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_DEVINFO_UPDATE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtDevInfoUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill Devive info parameter here*/
	CmdDeviceInfoUpdate.ucOwnMacIdx = OwnMacIdx;
	CmdDeviceInfoUpdate.ucBandIdx = BandIdx;
	CmdDeviceInfoUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdDeviceInfoUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdDeviceInfoUpdate,
					  sizeof(CMD_DEVINFO_UPDATE_T));

	if (u8EnableFeature & DEVINFO_ACTIVE_FEATURE) {
		CMD_DEVINFO_ACTIVE_T DevInfoBasic = {0};
		/* Fill TLV format */
		DevInfoBasic.u2Tag = DEVINFO_ACTIVE;
		DevInfoBasic.u2Length = sizeof(CMD_DEVINFO_ACTIVE_T);
		DevInfoBasic.ucActive = Active;
		DevInfoBasic.ucDbdcIdx = BandIdx;
#ifdef CFG_BIG_ENDIAN
		DevInfoBasic.u2Tag = cpu2le16(DevInfoBasic.u2Tag);
		DevInfoBasic.u2Length = cpu2le16(DevInfoBasic.u2Length);
#endif
		os_move_mem(DevInfoBasic.aucOwnMAC, OwnMacAddr, MAC_ADDR_LEN);
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_INFO,
				 "(CMD_DEVINFO_BASIC_T), OwnMacIdx = %d, ucActive = %d, aucOwnMAC = "MACSTR"\n",
				  OwnMacIdx,
				  DevInfoBasic.ucActive,
				  MAC2STR(DevInfoBasic.aucOwnMAC));
		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&DevInfoBasic,
						  sizeof(CMD_DEVINFO_ACTIVE_T));
	}

	/* Send out CMD */
	call_fw_cmd_notifieriers(WO_CMD_DEV_INFO, pAd, msg->net_pkt);
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

/*Start STARec Handler*/

static INT32 StaRecUpdateBasic(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	/* Fill STA Rec Common */
#ifdef CONFIG_STA_SUPPORT
	EDCA_PARM *pEdca = NULL;
#endif /*CONFIG_STA_SUPPORT*/
	CMD_STAREC_COMMON_T StaRecCommon = {0};
#ifdef CONFIG_STA_SUPPORT
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pStaRecCfg->u2WlanIdx);
	PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd, tr_entry->wdev);
#endif
	/* Fill TLV format */
	StaRecCommon.u2Tag = STA_REC_BASIC_STA_RECORD;
	StaRecCommon.u2Length = sizeof(CMD_STAREC_COMMON_T);
	StaRecCommon.u4ConnectionType = cpu2le32(pStaRecCfg->ConnectionType);
	StaRecCommon.ucConnectionState = pStaRecCfg->ConnectionState;
	/* New info to indicate this is new way to update STAREC */
	StaRecCommon.u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2;

	if (pStaRecCfg->IsNewSTARec)
		StaRecCommon.u2ExtraInfo |= STAREC_COMMON_EXTRAINFO_NEWSTAREC;

#ifdef CONFIG_AP_SUPPORT

	if (pEntry) {
		StaRecCommon.ucIsQBSS =
			CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE) ?
			TRUE : FALSE;
		StaRecCommon.u2AID = cpu2le16(pEntry->Aid);
	}

#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT

	if (pStaCfg) {
		pEdca = hwifi_get_edca(pAd, &pStaCfg->wdev);

		if (pEdca)
			StaRecCommon.ucIsQBSS = pEdca->bValid;

		StaRecCommon.u2AID = cpu2le16(pStaCfg->StaActive.Aid);
	}

#endif /*CONFIG_STA_SUPPORT*/

	if (pEntry) {
		os_move_mem(StaRecCommon.aucPeerMacAddr,
					pEntry->Addr, MAC_ADDR_LEN);
	} else {
		os_move_mem(StaRecCommon.aucPeerMacAddr,
					BROADCAST_ADDR, MAC_ADDR_LEN);
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
			 "(CMD_STAREC_COMMON_T), u4ConnectionType = %d, ucConnectionState = %d, ucIsQBSS = %d, u2AID = %d, aucPeerMacAddr = "MACSTR"\n",
			  le2cpu32(StaRecCommon.u4ConnectionType),
			  StaRecCommon.ucConnectionState,
			  StaRecCommon.ucIsQBSS,
			  le2cpu16(StaRecCommon.u2AID),
			  MAC2STR(StaRecCommon.aucPeerMacAddr));
	/* Append this feature */
#ifdef CFG_BIG_ENDIAN
	StaRecCommon.u2Tag = cpu2le16(StaRecCommon.u2Tag);
	StaRecCommon.u2Length = cpu2le16(StaRecCommon.u2Length);
	StaRecCommon.u2ExtraInfo = cpu2le16(StaRecCommon.u2ExtraInfo);
#endif
	AndesAppendCmdMsg(msg, (char *)&StaRecCommon, sizeof(CMD_STAREC_COMMON_T));
	return 0;
}

static INT32 StaRecUpdateRa(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	struct _STAREC_AUTO_RATE_T CmdStaRecAutoRate = {0};

	if (pEntry) {
		os_zero_mem(&CmdStaRecAutoRate, sizeof(CmdStaRecAutoRate));
		StaRecAutoRateParamSet(&pEntry->RaEntry, &CmdStaRecAutoRate);
		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAutoRate,
						  sizeof(struct _STAREC_AUTO_RATE_T));
		return 0;
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	return -1;
}

static INT32 StaRecUpdateRaUpdate(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	CMD_STAREC_AUTO_RATE_UPDATE_T CmdStaRecAutoRateUpdate = {0};
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if (pEntry) {
		os_zero_mem(&CmdStaRecAutoRateUpdate, sizeof(CmdStaRecAutoRateUpdate));
		StaRecAutoRateUpdate(&pEntry->RaEntry, &pEntry->RaInternal,
							 pStaRecCfg->pRaParam, &CmdStaRecAutoRateUpdate);
		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAutoRateUpdate,
						  sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
		return 0;
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
	return -1;
}

#ifdef TXBF_SUPPORT
static INT32 StaRecUpdateBf(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	CMD_STAREC_BF CmdStaRecBf;

	if (pEntry) {
		os_zero_mem(&CmdStaRecBf, sizeof(CMD_STAREC_BF));
		StaRecBfUpdate(pEntry, &CmdStaRecBf);
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecBf, sizeof(CMD_STAREC_BF));
		return 0;
	}

	return -1;
}

#ifdef DOT11_HE_AX
static INT32 StaRecUpdateBfee(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	CMD_STAREC_BFEE CmdStaRecBfee;

	if (pEntry) {
		os_zero_mem(&CmdStaRecBfee, sizeof(CMD_STAREC_BFEE));
		StaRecBfeeUpdate(pEntry, &CmdStaRecBfee);
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecBfee, sizeof(CMD_STAREC_BFEE));
		return 0;
	}

	return -1;
}
#endif /*DOT11_HE_AX*/
#endif /*TXBF_SUPPORT*/

static INT32 StaRecUpdateAmsdu(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_AMSDU_T CmdStaRecAmsdu = {0};
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if (pEntry) {
		os_zero_mem(&CmdStaRecAmsdu, sizeof(CMD_STAREC_AMSDU_T));
		CmdStaRecAmsdu.u2Tag = cpu2le16(STA_REC_AMSDU);
		CmdStaRecAmsdu.u2Length = cpu2le16(sizeof(CMD_STAREC_AMSDU_T));
		CmdStaRecAmsdu.ucMaxMpduSize = pEntry->AMsduSize;
		CmdStaRecAmsdu.ucMaxAmsduNum = 0;
#ifdef TX_AGG_ADJUST_WKR

		if (tx_check_for_agg_adjust(pAd, pEntry) == TRUE)
			CmdStaRecAmsdu.ucMaxMpduSize = 0;

#endif /* TX_AGG_ADJUST_WKR */

		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAmsdu,
						  sizeof(CMD_STAREC_AMSDU_T));
		return 0;
	}

	return -1;
}

#ifdef HW_TX_AMSDU_SUPPORT
static INT32 StaRecUpdateHwAmsdu(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_AMSDU_T CmdStaRecAmsdu = {0};
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (pEntry) {
		os_zero_mem(&CmdStaRecAmsdu, sizeof(CMD_STAREC_AMSDU_T));
		CmdStaRecAmsdu.u2Tag = cpu2le16(STA_REC_HW_AMSDU);
		CmdStaRecAmsdu.u2Length = cpu2le16(sizeof(CMD_STAREC_AMSDU_T));
		CmdStaRecAmsdu.ucMaxMpduSize = pEntry->AMsduSize;
		CmdStaRecAmsdu.ucMaxAmsduNum = cap->hw_max_amsdu_nums;
		CmdStaRecAmsdu.ucAmsduEnable = TRUE;

		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecAmsdu,
						  sizeof(CMD_STAREC_AMSDU_T));
		return 0;
	}

	return -1;
}
#endif /*HW_TX_AMSDU_SUPPORT*/

static INT32 StaRecUpdateTxProc(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_TX_PROC_T CmdStaRecTxProc = {0};
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_CSO_SUPPORT
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
#ifdef CONFIG_STA_SUPPORT
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
#endif /* CONFIG_STA_SUPPORT */

#if defined(VLAN_SUPPORT)|| defined(APCLI_AS_WDS_STA_SUPPORT) || defined(MBSS_AS_WDS_AP_SUPPORT)
#ifdef CONFIG_STA_SUPPORT
	struct _STA_TR_ENTRY *tr_entry = tr_entry_get(pAd, pStaRecCfg->u2WlanIdx);
	struct wifi_dev *bss_wdev = NULL;
#endif /* CONFIG_STA_SUPPORT */
#endif
	UINT8 rx_policy_is_set = FALSE, i;

#ifdef CONFIG_STA_SUPPORT
	pEntry = pStaRecCfg->pEntry;
#endif /* CONFIG_STA_SUPPORT */
	os_zero_mem(&CmdStaRecTxProc, sizeof(CMD_STAREC_TX_PROC_T));
	CmdStaRecTxProc.u2Tag = STA_REC_TX_PROC; /* Tag = 0x08 */
	CmdStaRecTxProc.u2Length = sizeof(CMD_STAREC_TX_PROC_T);
#ifdef VLAN_SUPPORT
#ifdef CONFIG_STA_SUPPORT
	bss_wdev = tr_entry->wdev;
#endif /* CONFIG_STA_SUPPORT */
	if ((pEntry && pEntry->wdev && pEntry->wdev->bVLAN_Tag) || (bss_wdev && bss_wdev->bVLAN_Tag))
		CmdStaRecTxProc.u4TxProcFlag = 0;
	else
#endif /*VLAN_SUPPORT*/
#ifdef MAP_R2
	if (IS_MAP_R2_ENABLE(pAd)) {
		AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, FALSE, FALSE);
		rx_policy_is_set = TRUE;
		CmdStaRecTxProc.u4TxProcFlag = 0;
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"set u4TxProcFlag = 0 to keep vlan tag\n");
		if (pEntry && pEntry->wdev)
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"%s\n", pEntry->wdev->if_dev->name);
	} else
#endif
	CmdStaRecTxProc.u4TxProcFlag = RVLAN;
#if defined(APCLI_AS_WDS_STA_SUPPORT) || defined(MBSS_AS_WDS_AP_SUPPORT)
	if ((pEntry && pEntry->wdev && pEntry->wdev->bVLAN_Tag) || (bss_wdev && bss_wdev->bVLAN_Tag)) {
		AsicRxHeaderTransCtl(pAd, TRUE, FALSE, FALSE, FALSE, FALSE);
		CmdStaRecTxProc.u4TxProcFlag = 0;
		rx_policy_is_set = TRUE;
	}
#endif
#ifdef CONFIG_CSO_SUPPORT

	if (pChipCap->asic_caps & fASIC_CAP_CSO) {
		CmdStaRecTxProc.u4TxProcFlag |= IPCSO;
		CmdStaRecTxProc.u4TxProcFlag |= TCPUDPCSO;
	}

#endif
#ifdef CONFIG_STA_SUPPORT
	if (pEntry && (IS_ENTRY_PEER_AP(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
		CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_EN;

		if (pEntry->bACMBit[0] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_BK;

		if (pEntry->bACMBit[1] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_BE;

		if (pEntry->bACMBit[2] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_VI;

		if (pEntry->bACMBit[3] == TRUE)
			CmdStaRecTxProc.u4TxProcFlag |= TX_PROC_ACM_CFG_VO;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CFG_BIG_ENDIAN
	CmdStaRecTxProc.u4TxProcFlag = cpu2le32(CmdStaRecTxProc.u4TxProcFlag);
	CmdStaRecTxProc.u2Tag = cpu2le16(CmdStaRecTxProc.u2Tag);
	CmdStaRecTxProc.u2Length = cpu2le16(CmdStaRecTxProc.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecTxProc,
					  sizeof(CMD_STAREC_TX_PROC_T));
	if (rx_policy_is_set == TRUE) {
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++)
			pAd->ApCfg.MBSSID[i].wdev.VLAN_Policy[RX_VLAN] = 2;
		for (i = 0; i < MAX_MULTI_STA; i++)
			pAd->StaCfg[i].wdev.VLAN_Policy[RX_VLAN] = 2;
	}
	return 0;
}

#ifdef DOT11_N_SUPPORT
static INT32 StaRecUpdateHtInfo(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_HT_INFO_T CmdStaRecHtInfo;
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if (pEntry) {
		os_zero_mem(&CmdStaRecHtInfo, sizeof(CMD_STAREC_HT_INFO_T));
		CmdStaRecHtInfo.u2Tag = STA_REC_BASIC_HT_INFO;
		CmdStaRecHtInfo.u2Length = sizeof(CMD_STAREC_HT_INFO_T);
		/* FIXME: may need separate function to compose the payload */
		os_move_mem(&CmdStaRecHtInfo.u2HtCap, &(pEntry->HTCapability.HtCapInfo),
					sizeof(CmdStaRecHtInfo.u2HtCap));
#ifdef CFG_BIG_ENDIAN
		CmdStaRecHtInfo.u2HtCap = cpu2le16(CmdStaRecHtInfo.u2HtCap);
		CmdStaRecHtInfo.u2Tag = cpu2le16(CmdStaRecHtInfo.u2Tag);
		CmdStaRecHtInfo.u2Length = cpu2le16(CmdStaRecHtInfo.u2Length);
#endif
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecHtInfo,
						  sizeof(CMD_STAREC_HT_INFO_T));
		return 0;
	}

	return -1;
}

#ifdef DOT11_VHT_AC
static INT32 StaRecUpdateVhtInfo(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_VHT_INFO_T CmdStaRecVHtInfo;
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;

	if (pEntry) {
		os_zero_mem(&CmdStaRecVHtInfo, sizeof(CMD_STAREC_VHT_INFO_T));
		CmdStaRecVHtInfo.u2Tag = STA_REC_BASIC_VHT_INFO;
		CmdStaRecVHtInfo.u2Length = sizeof(CMD_STAREC_VHT_INFO_T);
		/* FIXME: may need separate function to compose the payload */
		os_move_mem(&CmdStaRecVHtInfo.u4VhtCap,
					&(pEntry->vht_cap_ie.vht_cap),
					sizeof(CmdStaRecVHtInfo.u4VhtCap));
		os_move_mem(&CmdStaRecVHtInfo.u2VhtRxMcsMap,
					&(pEntry->vht_cap_ie.mcs_set.rx_mcs_map),
					sizeof(CmdStaRecVHtInfo.u2VhtRxMcsMap));
		os_move_mem(&CmdStaRecVHtInfo.u2VhtTxMcsMap,
					&(pEntry->vht_cap_ie.mcs_set.tx_mcs_map),
					sizeof(CmdStaRecVHtInfo.u2VhtTxMcsMap));

		if (IS_VHT_STA(pEntry) && !IS_HE_2G_STA(pEntry->cap.modes)) {
				UCHAR ucRTSBWSig = wlan_config_get_vht_bw_sig(pEntry->wdev);
				/* for StaRec: 0-disable DynBW, 1-static BW, 2 Dynamic BW */
				CmdStaRecVHtInfo.ucRTSBWSig = ucRTSBWSig;
		}

#ifdef CFG_BIG_ENDIAN
		CmdStaRecVHtInfo.u2Tag = cpu2le16(CmdStaRecVHtInfo.u2Tag);
		CmdStaRecVHtInfo.u2Length = cpu2le16(CmdStaRecVHtInfo.u2Length);
		CmdStaRecVHtInfo.u4VhtCap = cpu2le32(CmdStaRecVHtInfo.u4VhtCap);
		CmdStaRecVHtInfo.u2VhtRxMcsMap = cpu2le16(CmdStaRecVHtInfo.u2VhtRxMcsMap);
		CmdStaRecVHtInfo.u2VhtTxMcsMap = cpu2le16(CmdStaRecVHtInfo.u2VhtTxMcsMap);
#endif
		AndesAppendCmdMsg(msg, (char *)&CmdStaRecVHtInfo,
						  sizeof(CMD_STAREC_VHT_INFO_T));
		return 0;
	}

	return -1;
}
#endif /*DOT11_VHT_AC*/

#ifdef DOT11_HE_AX
static INT32 sta_rec_update_he_info(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_HE_INFO_T he_info;
	STA_REC_CFG_T *sta_rec = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = sta_rec->pEntry;
	struct he_sta_mac_info *mac_info = &sta_rec->he_sta.mac_info;
	struct he_sta_phy_info *phy_info = &sta_rec->he_sta.phy_info;
	struct he_mcs_info *mcs = &sta_rec->he_sta.max_nss_mcs;
	int i;

	if (pEntry || sta_rec->ConnectionType == CONNECTION_INFRA_BC) {
		os_zero_mem(&he_info, sizeof(CMD_STAREC_HE_INFO_T));
		he_info.u2Tag = STA_REC_BASIC_HE_INFO;
		he_info.u2Length = sizeof(CMD_STAREC_HE_INFO_T);
		/*mac info*/
		he_info.u4HeCap |= (mac_info->bqr_support << STA_REC_HE_CAP_BQR);
		he_info.u4HeCap |= (mac_info->htc_support << STA_REC_HE_CAP_HTC);
		he_info.u4HeCap |= (mac_info->bsr_support << STA_REC_HE_CAP_BSR);
		he_info.u4HeCap |= (mac_info->om_support << STA_REC_HE_CAP_OM);
		he_info.u4HeCap |= (mac_info->amsdu_in_ampdu_support << STA_REC_HE_CAP_AMSDU_IN_AMPDU);
		he_info.u4HeCap |= (mac_info->he_dyn_smps << STA_REC_HE_CAP_HE_DYNAMIC_SMPS);
		he_info.ucMaxAmpduLenExponent = mac_info->max_ampdu_len_exp;
		he_info.ucTrigerFrameMacPadDuration = mac_info->trigger_frame_mac_pad_dur;
		/*phy info*/
		he_info.u4HeCap |= (phy_info->dual_band_support << STA_REC_HE_CAP_DUAL_BAND);
		he_info.u4HeCap |= (phy_info->ldpc_support << STA_REC_HE_CAP_LDPC);
		he_info.u4HeCap |= (phy_info->triggered_cqi_feedback_support << STA_REC_HE_CAP_TRIG_CQI_FK);
		he_info.u4HeCap |= (phy_info->partial_bw_ext_range_support << STA_REC_HE_CAP_PARTIAL_BW_EXT_RANGE);
		he_info.u4HeCap |= (phy_info->bw20_242tone << STA_REC_HE_CAP_BW20_RU242_SUPPORT);
		he_info.ucChBwSet = phy_info->ch_width_set;
		he_info.ucDeviceClass = phy_info->device_class;
		he_info.ucPuncPreamRx = phy_info->punctured_preamble_rx;
		he_info.ucPktExt = 2;	/* force Packet Extension as 16 us by default */
		he_info.ucDcmTxMode = phy_info->dcm_cap_tx;
		he_info.ucDcmRxMode = phy_info->dcm_cap_rx;
		he_info.ucDcmTxMaxNss = phy_info->dcm_max_nss_tx;
		he_info.ucDcmRxMaxNss = phy_info->dcm_max_nss_rx;
		he_info.ucDcmMaxRu = phy_info->dcm_max_ru;
		/*1024QAM*/
		he_info.u4HeCap |= (phy_info->tx_le_ru242_1024qam << STA_REC_HE_CAP_TX_1024QAM_UNDER_RU242);
		he_info.u4HeCap |= (phy_info->rx_le_ru242_1024qam << STA_REC_HE_CAP_RX_1024QAM_UNDER_RU242);
		/*STBC*/
		if (phy_info->stbc_support & HE_LE_EQ_80M_TX_STBC)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_LE_EQ_80M_TX_STBC);
		if (phy_info->stbc_support & HE_LE_EQ_80M_RX_STBC)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_LE_EQ_80M_RX_STBC);
		if (phy_info->stbc_support & HE_GT_80M_RX_STBC)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_GT_80M_RX_STBC);
		if (phy_info->stbc_support & HE_GT_80M_TX_STBC)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_GT_80M_TX_STBC);
		/*GI*/
		if (phy_info->gi_cap & HE_SU_PPDU_1x_LTF_DOT8US_GI)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_SU_PPDU_1x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_SU_PPDU_MU_PPDU_4x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_ER_SU_PPDU_1x_LTF_DOT8US_GI)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_ER_SU_PPDU_1x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_ER_SU_PPDU_4x_LTF_DOT8US_GI)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_ER_SU_PPDU_4x_LTF_DOT8US_GI);
		if (phy_info->gi_cap & HE_NDP_4x_LTF_3DOT2MS_GI)
			he_info.u4HeCap |= (1 << STA_REC_HE_CAP_NDP_4x_LTF_3DOT2MS_GI);
		/*MAX NSS MCS*/
		for (i = 0 ; i < HE_MAX_SUPPORT_STREAM; i++) {
			he_info.au2MaxNssMcs[CMD_HE_MCS_BW80] |= (mcs->bw80_mcs[i] << (i * 2));
			he_info.au2MaxNssMcs[CMD_HE_MCS_BW160] |= (mcs->bw160_mcs[i] << (i * 2));
			he_info.au2MaxNssMcs[CMD_HE_MCS_BW8080] |= (mcs->bw8080_mcs[i] << (i * 2));
		}
#ifdef CFG_BIG_ENDIAN
		he_info.u2Tag = cpu2le16(he_info.u2Tag);
		he_info.u2Length = cpu2le16(he_info.u2Length);
		he_info.u4HeCap = cpu2le32(he_info.u4HeCap);
		for (i = 0 ; i < CMD_HE_MCS_BW_NUM ; i++)
			he_info.au2MaxNssMcs[i] = cpu2le16(he_info.au2MaxNssMcs[i]);
#endif
		AndesAppendCmdMsg(msg, (char *)&he_info,
						  sizeof(CMD_STAREC_HE_INFO_T));
		return 0;
	}

	return -1;
}

static INT32 sta_rec_update_muru_info(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_MURU_T muru_info;
	P_MURU_WDEV_CFG wdev_cfg = &muru_info.rMuRuStaCap.rWdevCfg;
	P_MURU_STA_DL_OFDMA dl_ofdma = &muru_info.rMuRuStaCap.rDlOfdma;
	P_MURU_STA_UL_OFDMA ul_ofdma = &muru_info.rMuRuStaCap.rUlOfdma;
	P_MURU_STA_DL_MIMO dl_mimo = &muru_info.rMuRuStaCap.rDlMimo;
	P_MURU_STA_UL_MIMO ul_mimo = &muru_info.rMuRuStaCap.rUlMimo;
	STA_REC_CFG_T *sta_rec = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = sta_rec->pEntry;
	struct wifi_dev *wdev = NULL;

	os_zero_mem(&muru_info, sizeof(CMD_STAREC_MURU_T));

	if (pEntry) {
		muru_info.u2Tag = STA_REC_MURU;
		muru_info.u2Length = sizeof(CMD_STAREC_MURU_T);
#ifdef CFG_BIG_ENDIAN
		muru_info.u2Tag = cpu2le16(muru_info.u2Tag);
		muru_info.u2Length	= cpu2le16(muru_info.u2Length);
#endif
		wdev = pEntry->wdev;
		if (wdev == NULL) {
			MTWF_DBG(ad, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "wdev is NULL\n");
			return -1;
		}
		/* global wdev setting */
		wdev_cfg->fgDlOfdmaEn = wlan_config_get_mu_dl_ofdma(wdev);
		wdev_cfg->fgUlOfdmaEn = wlan_config_get_mu_ul_ofdma(wdev);
		wdev_cfg->fgDlMimoEn = wlan_config_get_mu_dl_mimo(wdev);
		wdev_cfg->fgUlMimoEn = wlan_config_get_mu_ul_mimo(wdev);

		/* Sta Cap. of DL OFDMA */
		dl_ofdma->u1PhyPunRx = pEntry->cap.punc_preamble_rx;
		dl_ofdma->u120MIn40M2G = (pEntry->cap.he_phy_cap & HE_24G_20M_IN_40M_PPDU) ? 1 : 0;
		dl_ofdma->u120MIn160M = (pEntry->cap.he_phy_cap & HE_20M_IN_160M_8080M_PPDU) ? 1 : 0;
		dl_ofdma->u180MIn160M = (pEntry->cap.he_phy_cap & HE_80M_IN_160M_8080M_PPDU) ? 1 : 0;
		dl_ofdma->u1Lt16SigB = 0;				/* Wait he_phy_cap to support the cap */
		dl_ofdma->u1RxSUCompSigB = 0;			/* Wait he_phy_cap to support the cap */
		dl_ofdma->u1RxSUNonCompSigB = 0;		/* Wait he_phy_cap to support the cap */

		/* Sta Cap. of UL OFDMA */
		ul_ofdma->u1TrigFrmPad = pEntry->cap.tf_mac_pad_duration;
		ul_ofdma->u1MuCascading = (pEntry->cap.he_mac_cap & HE_MU_CASCADING) ? 1 : 0;
		ul_ofdma->u1UoRa = (pEntry->cap.he_mac_cap & HE_OFDMA_RA) ? 1 : 0;
		ul_ofdma->u12x996Tone = 0;				/* Wait he_mac_cap to support the cap */
		ul_ofdma->u1RxTrgFrmBy11ac = 0;			/* Wait he_mac_cap to support the cap */
		ul_ofdma->u1RxCtrlFrmToMBss = (pEntry->cap.he_mac_cap & HE_RX_CTRL_FRAME_TO_MULTIBSS) ? 1 : 0;
		/* Sta Cap. of DL MIMO */
		dl_mimo->fgVhtMuBfee = pEntry->vht_cap_ie.vht_cap.bfee_cap_mu;
		dl_mimo->fgParBWDlMimo = (pEntry->cap.he_phy_cap & HE_PARTIAL_BW_DL_MU_MIMO) ? 1 : 0;

		/* Sta Cap. of UL MIMO */
		ul_mimo->fgFullUlMimo = (pEntry->cap.he_phy_cap & HE_FULL_BW_UL_MU_MIMO) ? 1 : 0;
		ul_mimo->fgParUlMimo = (pEntry->cap.he_phy_cap & HE_PARTIAL_BW_UL_MU_MIMO) ? 1 : 0;

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"fgDlOfdmaEn = 0x%02X, fgUlOfdmaEn = 0x%02X\n",
				wdev_cfg->fgDlOfdmaEn, wdev_cfg->fgUlOfdmaEn);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"fgDlMimoEn = 0x%02X, fgUlMimoEn= 0x%02X\n",
				wdev_cfg->fgDlMimoEn, wdev_cfg->fgUlMimoEn);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u1PhyPunRx = 0x%02X, u120MIn40M2G = 0x%02X\n",
				dl_ofdma->u1PhyPunRx, dl_ofdma->u120MIn40M2G);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u120MIn160M = 0x%02X, u180MIn160M= 0x%02X\n",
				dl_ofdma->u120MIn160M, dl_ofdma->u180MIn160M);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u1Lt16SigB = 0x%02X, u1RxSUCompSigB = 0x%02X\n",
				dl_ofdma->u1Lt16SigB, dl_ofdma->u1RxSUCompSigB);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u1RxSUNonCompSigB = 0x%02X\n",
				dl_ofdma->u1RxSUNonCompSigB);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u1TrigFrmPad = 0x%02X, u1MuCascading = 0x%02X\n",
				ul_ofdma->u1TrigFrmPad, ul_ofdma->u1MuCascading);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u1UoRa = 0x%02X, u12x996Tone= 0x%02X\n",
				ul_ofdma->u1UoRa, ul_ofdma->u12x996Tone);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u1RxTrgFrmBy11ac = 0x%02X\n", ul_ofdma->u1RxTrgFrmBy11ac);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"u1RxCtrlFrmToMBss = 0x%02X\n", ul_ofdma->u1RxCtrlFrmToMBss);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"fgVhtMuBfee = 0x%02X, fgParBWDlMimo = 0x%02X\n",
				dl_mimo->fgVhtMuBfee, dl_mimo->fgParBWDlMimo);

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
				"fgFullUlMimo = 0x%02X, fgParUlMimo = 0x%02X\n",
				ul_mimo->fgFullUlMimo, ul_mimo->fgParUlMimo);

		AndesAppendCmdMsg(msg, (char *)&muru_info, sizeof(CMD_STAREC_MURU_T));

		return 0;
	}

	return -1;
}

static INT32 sta_rec_update_mu_edca(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg, VOID *args)
{
	CMD_STAREC_MU_EDCA_T mu_edca_info;
	STA_REC_CFG_T *sta_rec = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = sta_rec->pEntry;

	os_zero_mem(&mu_edca_info, sizeof(CMD_STAREC_MU_EDCA_T));

	if (pEntry) {
		mu_edca_info.u2Tag = STA_REC_MUEDCA;
		mu_edca_info.u2Length = sizeof(CMD_STAREC_MU_EDCA_T);
#ifdef CFG_BIG_ENDIAN
		mu_edca_info.u2Tag = cpu2le16(mu_edca_info.u2Tag);
		mu_edca_info.u2Length = cpu2le16(mu_edca_info.u2Length);
#endif

		NdisCopyMemory(&(mu_edca_info.arMUEdcaParams[0]),
						&pEntry->arMUEdcaParams[0],
						(sizeof(struct he_mu_edca_params) * ACI_AC_NUM));

		AndesAppendCmdMsg(msg,
			(char *)&mu_edca_info,
			sizeof(CMD_STAREC_MU_EDCA_T));

		return 0;
	}

	return -1;
}

#endif /*DOT11_HE_AX*/

#endif /*DOT11_N_SUPPORT*/

static INT32 StaRecUpdateApPs(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY *pEntry = pStaRecCfg->pEntry;
	CMD_STAREC_PS_T CmdPsInfo = {0};
	UINT8 IdApsd;
	UINT8 ACTriSet = 0;
	UINT8 ACDelSet = 0;

	if (pEntry) {
		/* Fill TLV format */
		CmdPsInfo.u2Tag = STA_REC_AP_PS;
		CmdPsInfo.u2Length = sizeof(CMD_STAREC_PS_T);
		/* Find Triggerable AC */
		/* Find Deliverable AC */
		ACTriSet = 0;
		ACDelSet = 0;

		for (IdApsd = 0; IdApsd < 4; IdApsd++) {
			if (pEntry->bAPSDCapablePerAC[IdApsd])
				ACTriSet |= 1 << IdApsd;

			if (pEntry->bAPSDDeliverEnabledPerAC[IdApsd])
				ACDelSet |= 1 << IdApsd;
		}

		CmdPsInfo.ucStaBmpTriggerAC = ACTriSet;
		CmdPsInfo.ucStaBmpDeliveryAC = ACDelSet;
		CmdPsInfo.ucStaMaxSPLength = pStaRecCfg->pEntry->MaxSPLength;
		CmdPsInfo.u2StaListenInterval = 0; /* TODO: */
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_DEBUG,
				 "(STA_REC_AP_PS), Delv=%x Trig=%x SP=%d LInt=%d",
				  CmdPsInfo.ucStaBmpDeliveryAC,
				  CmdPsInfo.ucStaBmpTriggerAC,
				  CmdPsInfo.ucStaMaxSPLength,
				  CmdPsInfo.u2StaListenInterval);
#ifdef CFG_BIG_ENDIAN
		CmdPsInfo.u2Tag = cpu2le16(CmdPsInfo.u2Tag);
		CmdPsInfo.u2Length = cpu2le16(CmdPsInfo.u2Length);
		CmdPsInfo.u2StaListenInterval = cpu2le16(CmdPsInfo.u2StaListenInterval);
#endif
		/* Append this feature */
		AndesAppendCmdMsg(msg, (char *)&CmdPsInfo, sizeof(CMD_STAREC_PS_T));
		return 0;
	}

	return -1;
}

/* please pass NULL pointer wtbl_security_key, this api will dynamic allocate */
INT32 fill_key_install_cmd(
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID **wtbl_security_key,
	OUT UINT32 *cmd_len)
{
	CMD_WTBL_SECURITY_KEY_T *wtbl_security_key_ptr = NULL;

	if (*wtbl_security_key != NULL)
		return NDIS_STATUS_FAILURE;

	os_alloc_mem(NULL, (UCHAR **)wtbl_security_key, sizeof(CMD_WTBL_SECURITY_KEY_T));
	if (*wtbl_security_key == NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR, "cmd alloc fail\n");
		return NDIS_STATUS_FAILURE;
	}
	os_zero_mem(*wtbl_security_key, sizeof(CMD_WTBL_SECURITY_KEY_T));

	wtbl_security_key_ptr = (CMD_WTBL_SECURITY_KEY_T *)*wtbl_security_key;
	*cmd_len = sizeof(CMD_WTBL_SECURITY_KEY_T);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"wcid=%d, Operation=%d, Direction=%d\n",
			 asic_sec_info->Wcid, asic_sec_info->Operation, asic_sec_info->Direction);

	if (is_sta_rec_update)
		wtbl_security_key_ptr->u2Tag = STA_REC_INSTALL_KEY;
	else
		wtbl_security_key_ptr->u2Tag = WTBL_SECURITY_KEY;
	wtbl_security_key_ptr->u2Length = sizeof(CMD_WTBL_SECURITY_KEY_T);
	return fill_wtbl_key_info_struc(asic_sec_info, wtbl_security_key_ptr);
}


/* please pass NULL pointer wtbl_security_key, this api will dynamic allocate */
INT32 fill_key_install_cmd_v2(
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID **wtbl_security_key,
	OUT UINT32 *cmd_len)
{
	CMD_WTBL_SECURITY_KEY_V2_T *wtbl_security_key_ptr = NULL;
	UINT32 size = sizeof(CMD_WTBL_SECURITY_KEY_V2_T);

	if (*wtbl_security_key != NULL)
		return NDIS_STATUS_FAILURE;

	if (IS_CIPHER_WEP(asic_sec_info->Cipher))
		size += sizeof(CMD_WTBL_SEC_CIPHER_WEP_T);
	else if (IS_CIPHER_TKIP(asic_sec_info->Cipher))
		size += sizeof(CMD_WTBL_SEC_CIPHER_TKIP_T);
	else if (IS_CIPHER_CCMP128(asic_sec_info->Cipher) ||
		 IS_CIPHER_CCMP256(asic_sec_info->Cipher) ||
		 IS_CIPHER_GCMP128(asic_sec_info->Cipher) ||
		 IS_CIPHER_GCMP256(asic_sec_info->Cipher))
		size += sizeof(CMD_WTBL_SEC_CIPHER_AES_T);
	if (IS_CIPHER_BIP_CMAC128(asic_sec_info->Cipher) ||
	    IS_CIPHER_BIP_CMAC256(asic_sec_info->Cipher))
		size += sizeof(CMD_WTBL_SEC_CIPHER_BIP_T);
	else if (IS_CIPHER_CCMP128(asic_sec_info->Cipher) && asic_sec_info->Key.KeyLen == 32)
		size += sizeof(CMD_WTBL_SEC_CIPHER_BIP_T);

	if (asic_sec_info->bigtk_key_len)
		size += sizeof(CMD_WTBL_SEC_CIPHER_BIP_T);

	os_alloc_mem(NULL, (UCHAR **)wtbl_security_key, size);
	if (*wtbl_security_key == NULL) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DEVINFO, DBG_LVL_ERROR, "cmd alloc fail\n");
		return NDIS_STATUS_FAILURE;
	}
	os_zero_mem(*wtbl_security_key, size);

	wtbl_security_key_ptr = (CMD_WTBL_SECURITY_KEY_V2_T *)*wtbl_security_key;
	*cmd_len = size;
	if (is_sta_rec_update)
		wtbl_security_key_ptr->u2Tag = STA_REC_INSTALL_KEY_V2;
	else
		wtbl_security_key_ptr->u2Tag = WTBL_SECURITY_KEY_V2;
	wtbl_security_key_ptr->u2Length = size;

	return fill_wtbl_key_info_struc_v2(asic_sec_info, wtbl_security_key_ptr);
}

#ifdef WIFI_UNIFIED_COMMAND
INT32 fill_key_install_uni_cmd_dynsize_check_v2(
	struct _ASIC_SEC_INFO *asic_sec_info,
	OUT UINT32 *cmd_len)
{
	UINT32 size = 0;
	if (IS_CIPHER_WEP(asic_sec_info->Cipher))
		size += sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
	else if (IS_CIPHER_TKIP(asic_sec_info->Cipher))
		size += sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
	else if (IS_CIPHER_CCMP128(asic_sec_info->Cipher) ||
		 IS_CIPHER_CCMP256(asic_sec_info->Cipher) ||
		 IS_CIPHER_GCMP128(asic_sec_info->Cipher) ||
		 IS_CIPHER_GCMP256(asic_sec_info->Cipher))
		size += sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
	if (IS_CIPHER_BIP_CMAC128(asic_sec_info->Cipher) ||
	    IS_CIPHER_BIP_CMAC256(asic_sec_info->Cipher))
		size += sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);
	else if (IS_CIPHER_CCMP128(asic_sec_info->Cipher) && asic_sec_info->Key.KeyLen == 32)
		size += sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);

	if (asic_sec_info->bigtk_key_len)
		size += sizeof(UNI_CMD_WTBL_SEC_CIPHER_GENERAL_T);

	*cmd_len = size;
	return NDIS_STATUS_SUCCESS;
}

INT32 fill_key_install_uni_cmd_v2(
	void *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	void *wtbl_security_key)
{
	UINT32 cmd_len = 0;
	P_CMD_WTBL_SECURITY_KEY_V2_T wtbl_security_key_ptr = NULL;

	wtbl_security_key_ptr = (P_CMD_WTBL_SECURITY_KEY_V2_T) wtbl_security_key;
	fill_key_install_uni_cmd_dynsize_check_v2(asic_sec_info, &cmd_len);
	cmd_len += sizeof(CMD_WTBL_SECURITY_KEY_V2_T);

	if (is_sta_rec_update) {
		wtbl_security_key_ptr->u2Tag = UNI_CMD_STAREC_INSTALL_KEY_V2;
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_INFO,
			"=> UNI_CMD_STAREC_INSTALL_KEY_V2 (STA_REC_INSTALL_KEY_FEATURE)\n");
	}
	else
		wtbl_security_key_ptr->u2Tag = WTBL_SECURITY_KEY_V2;
	wtbl_security_key_ptr->u2Length = cmd_len;

#ifdef CFG_BIG_ENDIAN
	wtbl_security_key_ptr->u2Tag = cpu2le16(wtbl_security_key_ptr->u2Tag);
	wtbl_security_key_ptr->u2Length = cpu2le16(wtbl_security_key_ptr->u2Length);
#endif

	return fill_uni_cmd_wtbl_key_info_struc_v2(hdev_ctrl, asic_sec_info, wtbl_security_key_ptr);
}
#endif /* WIFI_UNIFIFED_COMMAND */

static INT32 StaRecUpdateSecKey(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	STA_REC_CFG_T *pStaRecCfg = (STA_REC_CFG_T *)args;
	ASIC_SEC_INFO *asic_sec_info = &pStaRecCfg->asic_sec_info;
	VOID *wtbl_security_key = NULL;
	UINT32 cmd_len = 0;
#ifdef CFG_BIG_ENDIAN
	CMD_WTBL_SECURITY_KEY_V2_T *wtbl_security_key_ptr;
#endif

	chip_fill_key_install_cmd(pAd->hdev_ctrl, asic_sec_info, STAREC_SEC_KEY_METHOD, (VOID **)&wtbl_security_key, &cmd_len);

#ifdef CFG_BIG_ENDIAN
	wtbl_security_key_ptr = (CMD_WTBL_SECURITY_KEY_V2_T*)wtbl_security_key;
	wtbl_security_key_ptr->u2Length = cpu2le16(wtbl_security_key_ptr->u2Length);
	wtbl_security_key_ptr->u2Tag = cpu2le16(wtbl_security_key_ptr->u2Tag);
#endif

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)wtbl_security_key, cmd_len);
	os_free_mem(wtbl_security_key);
	return 0;
}

static INT32 StaRecUpdateWtbl(RTMP_ADAPTER *pAd, struct cmd_msg *msg, VOID *args)
{
	STA_REC_CFG_T		*pStaRecCfg = (STA_REC_CFG_T *)args;
	MAC_TABLE_ENTRY	*pEntry = pStaRecCfg->pEntry;
	P_CMD_STAREC_WTBL_T	pStarec_wtbl = NULL;
	BOOLEAN		IsBCMCWCID = FALSE;
	CMD_WTBL_GENERIC_T	rWtblGeneric = {0};	/* Tag = 0, Generic */
	CMD_WTBL_RX_T		rWtblRx = {0};		/* Tage = 1, Rx */
#ifdef DOT11_N_SUPPORT
	CMD_WTBL_HT_T		rWtblHt = {0};		/* Tag = 2, HT */
#ifdef DOT11_VHT_AC
	CMD_WTBL_VHT_T		rWtblVht = {0};		/* Tag = 3, VHT */
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	CMD_WTBL_TX_PS_T	rWtblTxPs = {0};	/* Tag = 5, TxPs */
	CMD_WTBL_HDR_TRANS_T	rWtblHdrTrans = {0};	/* Tag = 6, Hdr Trans */
	CMD_WTBL_RDG_T		rWtblRdg = {0};		/* Tag = 9, Rdg */
#ifdef TXBF_SUPPORT
	CMD_WTBL_BF_T           rWtblBf = {0};		/* Tag = 12, BF */
#endif /* TXBF_SUPPORT */
	CMD_WTBL_SMPS_T		rWtblSmPs = {0};	/* Tag = 13, SMPS */
	CMD_WTBL_SPE_T          rWtblSpe = {0};		/* Tag = 16, SPE */
	UCHAR		*pTlvBuffer = NULL;
	UCHAR		*pTempBuffer = NULL;
	UINT32		u4TotalTlvLen = 0;
	UCHAR		ucTotalTlvNumber = 0;
	NDIS_STATUS	Status = NDIS_STATUS_SUCCESS;
	P_CMD_WTBL_UPDATE_T	pCmdWtblUpdate = NULL;
	/* Allocate TLV msg */
	Status = os_alloc_mem(pAd, (UCHAR **)&pStarec_wtbl, sizeof(CMD_STAREC_WTBL_T));

	if (pStarec_wtbl == NULL)
		return -1;

	os_zero_mem(pStarec_wtbl, sizeof(CMD_STAREC_WTBL_T));
	pStarec_wtbl->u2Tag = cpu2le16(STA_REC_WTBL);
	pStarec_wtbl->u2Length = cpu2le16(sizeof(CMD_STAREC_WTBL_T));

	if (pStaRecCfg->ConnectionType == CONNECTION_INFRA_BC)
		IsBCMCWCID = TRUE;

	/* Tag = 1 */
	rWtblRx.ucRv = 1;
	rWtblRx.ucRca2 = 1;

	if (IsBCMCWCID) {
		/* Tag = 0 */
		struct _STA_TR_ENTRY *tr_entry = NULL;
		UINT32 bcmc_wdev_type = 0;

		rWtblGeneric.ucMUARIndex = 0x0e;

		os_move_mem(rWtblGeneric.aucPeerAddress, BROADCAST_ADDR, MAC_ADDR_LEN);

		/* tmp solution to update STA mode AP address */
		tr_entry = tr_entry_get(pAd, pStaRecCfg->u2WlanIdx);

		if (tr_entry) {
			if (tr_entry->wdev)
				bcmc_wdev_type = tr_entry->wdev->wdev_type;

			ASSERT(tr_entry->EntryType == ENTRY_CAT_MCAST);
		}

		if ((bcmc_wdev_type == WDEV_TYPE_STA) ||
			(bcmc_wdev_type == WDEV_TYPE_REPEATER) ||
			(bcmc_wdev_type == WDEV_TYPE_ADHOC)) {
			WIFI_SYS_INFO_T *pWifiSysInfo = pAd->WifiSysInfo;
			WIFI_INFO_CLASS_T *pWifiClass = &pWifiSysInfo->BssInfo;
			BSS_INFO_ARGUMENT_T *pBssInfo = NULL;

			OS_SEM_LOCK(&pWifiSysInfo->lock);

			DlListForEach(pBssInfo, &pWifiClass->Head, BSS_INFO_ARGUMENT_T, list) {

				if (pBssInfo->ucBssIndex == pStaRecCfg->ucBssIndex) {
					/* Update to AP MAC when in STA mode */
					os_move_mem(rWtblGeneric.aucPeerAddress, pBssInfo->Bssid, MAC_ADDR_LEN);
				}
			}

			OS_SEM_UNLOCK(&pWifiSysInfo->lock);
		}

		/* Tag = 1 */
		rWtblRx.ucRca1 = 1;

		/* Tag = 6 */
		if (pAd->OpMode == OPMODE_AP) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
		}
	} else {

		struct wifi_dev *wdev = NULL;

		if (pEntry == NULL) {

			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
					 "\n\nERROR: No MacEntry. widx=%d ConnTyp=0x%x feat=0x%llx\n\n\n",
					 pStaRecCfg->u2WlanIdx,
					 pStaRecCfg->ConnectionType,
					 pStaRecCfg->u8EnableFeature);

			os_free_mem(pStarec_wtbl);

			return -1;
		}

		wdev = pEntry->wdev;

		/* Tag = 0 */
		/* rWtblGeneric.ucMUARIndex = pEntry->wdev->OmacIdx; */
		rWtblGeneric.ucQos = CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE) ? 1 : 0;
		rWtblGeneric.u2PartialAID = cpu2le16(pEntry->Aid);
		rWtblGeneric.ucMUARIndex = pStaRecCfg->MuarIdx;

		/* Tag = 0 */
		os_move_mem(rWtblGeneric.aucPeerAddress, pEntry->Addr, MAC_ADDR_LEN);

		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE)
			&& CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
			rWtblGeneric.ucAadOm = 1;

		/* Tag = 1 */
		if (wdev->wdev_type == WDEV_TYPE_STA || wdev->wdev_type == WDEV_TYPE_REPEATER)
			rWtblRx.ucRca1 = 1;

		/* Tag = 6 */
		if (wdev->wdev_type == WDEV_TYPE_STA || wdev->wdev_type == WDEV_TYPE_REPEATER) {
			rWtblHdrTrans.ucFd = 0;
			rWtblHdrTrans.ucTd = 1;
		} else if (wdev->wdev_type == WDEV_TYPE_AP) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 0;
		} else if (wdev->wdev_type == WDEV_TYPE_WDS) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
		} else
			MTWF_DBG(pAd, DBG_CAT_FW, CATFW_STAREC, DBG_LVL_ERROR,
					 "Unknown wdev type(%d) do not support header translation\n",
					  pEntry->wdev->wdev_type);

#ifdef A4_CONN
		if (IS_ENTRY_A4(pEntry) && (wdev->wdev_type == WDEV_TYPE_STA)) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
		}
#endif
#ifdef APCLI_AS_WDS_STA_SUPPORT
		if ((wdev->wdev_type == WDEV_TYPE_STA) && (wdev->wds_enable) && pEntry->bEnable4Addr) {
			rWtblHdrTrans.ucFd = 1;
			rWtblHdrTrans.ucTd = 1;
		}
#endif
#ifdef HDR_TRANS_RX_SUPPORT

		if (IS_CIPHER_TKIP_Entry(pEntry))
			rWtblHdrTrans.ucDisRhtr = 1;
		else
			rWtblHdrTrans.ucDisRhtr = 0;

#endif /* HDR_TRANS_RX_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (IS_HT_STA(pEntry)) {
			/* Tag = 0 */
			rWtblGeneric.ucQos = 1;
			rWtblGeneric.ucBafEn = 0;

			/* Tag = 2 */
			if (IS_HE_6G_STA(pEntry->cap.modes)) {
				rWtblHt.ucHt = 1;
				rWtblHt.ucMm = pEntry->cap.ampdu.he6g_min_mpdu_start_spacing;
				rWtblHt.ucAf = pEntry->cap.ampdu.he6g_max_ampdu_len_exp;
			} else {
				rWtblHt.ucHt = 1;
				rWtblHt.ucMm = pEntry->MpduDensity;
				rWtblHt.ucAf = pEntry->MaxRAmpduFactor;
			}

			if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE)) {
				/* Tga = 9 */
				rWtblRdg.ucR = 1;
				rWtblRdg.ucRdgBa = 0;
			}

			/* Tag = 13*/
			if (pEntry->MmpsMode == MMPS_DYNAMIC)
				rWtblSmPs.ucSmPs = 1;
			else
				rWtblSmPs.ucSmPs = 0;

#ifdef DOT11_VHT_AC
			/* Tag = 3 */
			if (IS_VHT_STA(pEntry) && (!IS_HE_2G_STA(pEntry->cap.modes)
				|| IS_HE_5G_STA(pEntry->cap.modes))) {

				UCHAR ucDynBw = wlan_config_get_vht_bw_sig(pEntry->wdev);
				rWtblVht.ucVht = 1;
				if (ucDynBw == BW_SIGNALING_DYNAMIC)
					rWtblVht.ucDynBw = 1;
				else
					rWtblVht.ucDynBw = 0;
			}
#endif /* DOT11_VHT_AC */
		}

#endif /* DOT11_N_SUPPORT */
	}

	/* Tag = 5 */
	rWtblTxPs.ucTxPs = 0;
#ifdef TXBF_SUPPORT

	if (!IsBCMCWCID) {
		/* Tag = 0xc */
		rWtblBf.ucGid     = 0;
		rWtblBf.ucPFMUIdx = pAd->rStaRecBf.u2PfmuId;

		if (IS_HT_STA(pEntry)) {
			if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn)
				rWtblBf.ucTiBf = IS_ITXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTiBf = FALSE;

			if (pAd->CommonCfg.ETxBfEnCond)
				rWtblBf.ucTeBf = IS_ETXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTeBf	  = FALSE;
		}

		if (IS_VHT_STA(pEntry)) {
			if (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn)
				rWtblBf.ucTibfVht = IS_ITXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTibfVht = FALSE;

			if (pAd->CommonCfg.ETxBfEnCond)
				rWtblBf.ucTebfVht = IS_ETXBF_SUP(pEntry->rStaRecBf.u1TxBfCap);
			else
				rWtblBf.ucTebfVht = FALSE;
		}
	}

#endif /* TXBF_SUPPORT */
	/* Tag = 0x10 */
	rWtblSpe.ucSpeIdx = 0;
	/* From WTBL COMMAND in STEREC TLV */
	pCmdWtblUpdate = (P_CMD_WTBL_UPDATE_T)&pStarec_wtbl->aucBuffer[0];
	pTlvBuffer = &pCmdWtblUpdate->aucBuffer[0];
	/* Append TLV msg */
	pTempBuffer = pTlvAppend(
					  pTlvBuffer,
					  (WTBL_GENERIC),
					  (sizeof(CMD_WTBL_GENERIC_T)),
					  &rWtblGeneric,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RX),
					  (sizeof(CMD_WTBL_RX_T)),
					  &rWtblRx,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_N_SUPPORT
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HT),
					  (sizeof(CMD_WTBL_HT_T)),
					  &rWtblHt,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_RDG),
					  (sizeof(CMD_WTBL_RDG_T)),
					  &rWtblRdg,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SMPS),
					  (sizeof(CMD_WTBL_SMPS_T)),
					  &rWtblSmPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef DOT11_VHT_AC
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_VHT),
					  (sizeof(CMD_WTBL_VHT_T)),
					  &rWtblVht,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_TX_PS),
					  (sizeof(CMD_WTBL_TX_PS_T)),
					  &rWtblTxPs,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_HDR_TRANS),
					  (sizeof(CMD_WTBL_HDR_TRANS_T)),
					  &rWtblHdrTrans,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
#ifdef TXBF_SUPPORT

	if (pAd->rStaRecBf.u2PfmuId != 0xFFFF) {
		pTempBuffer = pTlvAppend(
						  pTempBuffer,
						  (WTBL_BF),
						  (sizeof(CMD_WTBL_BF_T)),
						  &rWtblBf,
						  &u4TotalTlvLen,
						  &ucTotalTlvNumber);
	}

#endif /* TXBF_SUPPORT */
	pTempBuffer = pTlvAppend(
					  pTempBuffer,
					  (WTBL_SPE),
					  (sizeof(CMD_WTBL_SPE_T)),
					  &rWtblSpe,
					  &u4TotalTlvLen,
					  &ucTotalTlvNumber);
	pCmdWtblUpdate->u2TotalElementNum = cpu2le16(ucTotalTlvNumber);
	WCID_SET_H_L(pCmdWtblUpdate->ucWlanIdxHnVer, pCmdWtblUpdate->ucWlanIdxL, pStaRecCfg->u2WlanIdx);
	pCmdWtblUpdate->ucOperation = RESET_WTBL_AND_SET; /* In STAREC, currently reset and set only. */
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)pStarec_wtbl, sizeof(CMD_STAREC_WTBL_T));
	os_free_mem(pStarec_wtbl);
	return 0;
}

static STAREC_HANDLE_T StaRecHandle[] = {
	{STA_REC_BASIC_STA_RECORD, (UINT32)sizeof(CMD_STAREC_COMMON_T), StaRecUpdateBasic},
	{STA_REC_RA, (UINT32)sizeof(struct _STAREC_AUTO_RATE_T), StaRecUpdateRa},
	{STA_REC_RA_COMMON_INFO, (UINT32)sizeof(CMD_STAREC_AUTO_RATE_CFG_T), NULL},
	{STA_REC_RA_UPDATE, (UINT32)sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T), StaRecUpdateRaUpdate},
#ifdef TXBF_SUPPORT
	{STA_REC_BF, (UINT32)sizeof(CMD_STAREC_BF), StaRecUpdateBf},
#endif
	{STA_REC_AMSDU, (UINT32)sizeof(CMD_STAREC_AMSDU_T), StaRecUpdateAmsdu},
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
	/* This tag is used in CmdExtStaRecBaUpdate alone, not in CmdExtStaRecUpdate like most tag used. */
	{STA_REC_BA, (UINT32)sizeof(CMD_STAREC_BA_T), NULL},
#endif /*DOT11_N_SUPPORT*/
#endif /*CONFIG_AP_SUPPORT*/
	{STA_REC_RED, (UINT32)sizeof(CMD_STAREC_RED_T), NULL},
	{STA_REC_TX_PROC, (UINT32)sizeof(CMD_STAREC_TX_PROC_T), StaRecUpdateTxProc},
#ifdef DOT11_N_SUPPORT
	{STA_REC_BASIC_HT_INFO, (UINT32)sizeof(CMD_STAREC_HT_INFO_T), StaRecUpdateHtInfo},
#ifdef DOT11_VHT_AC
	{STA_REC_BASIC_VHT_INFO, (UINT32)sizeof(CMD_STAREC_VHT_INFO_T), StaRecUpdateVhtInfo},
#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
	{STA_REC_AP_PS, (UINT32)sizeof(CMD_STAREC_PS_T), StaRecUpdateApPs},
	{STA_REC_INSTALL_KEY, MAX_STA_REC_SEC_KEY_CMD_SIZE, StaRecUpdateSecKey},
	{STA_REC_WTBL, (UINT32)sizeof(CMD_STAREC_WTBL_T), StaRecUpdateWtbl},

#ifdef HW_TX_AMSDU_SUPPORT
	{STA_REC_HW_AMSDU, (UINT32)sizeof(CMD_STAREC_AMSDU_T), StaRecUpdateHwAmsdu},
#endif /*HW_TX_AMSDU_SUPPORT*/
#ifdef DOT11_HE_AX
	{STA_REC_BASIC_HE_INFO, (UINT32)sizeof(CMD_STAREC_HE_INFO_T), sta_rec_update_he_info},
	{STA_REC_MURU, (UINT32)sizeof(CMD_STAREC_MURU_T), sta_rec_update_muru_info},
	{STA_REC_MUEDCA, (UINT32)sizeof(CMD_STAREC_MU_EDCA_T), sta_rec_update_mu_edca},
#endif
#if defined(TXBF_SUPPORT) && defined(DOT11_HE_AX)
	{STA_REC_BFEE, (UINT32)sizeof(CMD_STAREC_BFEE), StaRecUpdateBfee},
#endif

};

typedef UINT32 WLAN_STATUS, *P_WLAN_STATUS;

#define WLAN_STATUS_SUCCESS                     ((WLAN_STATUS)0x00000000L)
#define WLAN_STATUS_PENDING                     ((WLAN_STATUS)0x00000103L)
#define WLAN_STATUS_NOT_ACCEPTED                ((WLAN_STATUS)0x00010003L)

#define STAREC_RETRY 3

static INT32 CmdExtStaRecUpdate_ReSyncDelete(
	RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg)
{
	struct cmd_msg		*msg = NULL;
	CMD_STAREC_UPDATE_T	CmdStaRecUpdate = {0};
	INT32			Ret = 0;
	UINT16			u2TLVNumber = 0;
	UINT32			size;
	struct _CMD_ATTRIBUTE	attr = {0};
	STA_REC_CFG_T		StaRecCfgForDel = {0};

	StaRecCfgForDel.ucBssIndex = pStaRecCfg->ucBssIndex;
	StaRecCfgForDel.u2WlanIdx = pStaRecCfg->u2WlanIdx;
	StaRecCfgForDel.ConnectionState = STATE_DISCONNECT;
	StaRecCfgForDel.MuarIdx = pStaRecCfg->MuarIdx;
	StaRecCfgForDel.ConnectionType = pStaRecCfg->ConnectionType;
	StaRecCfgForDel.u8EnableFeature = STA_REC_BASIC_STA_RECORD;
	StaRecCfgForDel.pEntry = pStaRecCfg->pEntry;
	size = sizeof(CMD_STAREC_UPDATE_T);
	size += StaRecHandle[STA_REC_BASIC_STA_RECORD].StaRecTagLen;
	msg = AndesAllocCmdMsg(pAd, size);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = StaRecCfgForDel.ucBssIndex;
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, StaRecCfgForDel.u2WlanIdx);
	CmdStaRecUpdate.ucMuarIdx = StaRecCfgForDel.MuarIdx;
	Ret = StaRecHandle[STA_REC_BASIC_STA_RECORD].StaRecTagHandler(pAd, msg, &StaRecCfgForDel);

	if (Ret == NDIS_STATUS_SUCCESS)
		u2TLVNumber++;

	/*insert to head*/
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(u2TLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
#ifdef CFG_BIG_ENDIAN
	StaRecCfgForDel.u8EnableFeature = cpu2le32(StaRecCfgForDel.u8EnableFeature);
#endif
	AndesAppendHeadCmdMsg(msg, (char *)&CmdStaRecUpdate,
						  sizeof(CMD_STAREC_UPDATE_T));
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	return Ret;
}

INT32 CmdExtStaRecUpdate(
	RTMP_ADAPTER *pAd,
	STA_REC_CFG_T *pStaRecCfg)
{
	struct cmd_msg			*msg = NULL;
	CMD_STAREC_UPDATE_T	    CmdStaRecUpdate = {0};
	INT32					Ret = 0;
	UINT8					i = 0;
	UINT16					u2TLVNumber;
	UINT32					size;
	UCHAR StaRecSupprotNum = sizeof(StaRecHandle) / sizeof(STAREC_HANDLE_T);
	struct _CMD_ATTRIBUTE attr = {0};
	UINT8 retry = STAREC_RETRY;
	STA_REC_CFG_T StaRecCfg = *pStaRecCfg;
CmdExtStaRecUpdate_restart:
	u2TLVNumber = 0;
	size = sizeof(CMD_STAREC_UPDATE_T);

	/* Get number of TLV*/
	for (i = 0; i < StaRecSupprotNum; i++) {
		if (StaRecCfg.u8EnableFeature & ((UINT64)1 << StaRecHandle[i].StaRecTag))
			size += StaRecHandle[i].StaRecTagLen;
	}

	msg = AndesAllocCmdMsg(pAd, size);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));

	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);
	if (StaRecCfg.u8EnableFeature & (1 << STA_REC_BASIC_STA_RECORD)) {
		if (StaRecCfg.ConnectionState == STATE_DISCONNECT)
			SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecDeleteRsp);
		else if (!VALID_UCAST_ENTRY_WCID(pAd, StaRecCfg.u2WlanIdx))
			SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecInsertRsp);
	}
	/* CmdExtStaRecInsertRsp after STA_REC_RA: WM enable uni-cast STA */
	if (StaRecCfg.u8EnableFeature & (1 << STA_REC_RA))
		if (StaRecCfg.ConnectionState > STATE_DISCONNECT)
			SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecInsertRsp);

	AndesInitCmdMsg(msg, attr);
	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = StaRecCfg.ucBssIndex;
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, StaRecCfg.u2WlanIdx);
	CmdStaRecUpdate.ucMuarIdx = StaRecCfg.MuarIdx;


	/* Fill RA related parameters */

	for (i = 0; i < StaRecSupprotNum; i++) {
		if ((StaRecCfg.u8EnableFeature & ((UINT64)1 << StaRecHandle[i].StaRecTag))) {
			if (StaRecHandle[i].StaRecTagHandler != NULL) {
				Ret = StaRecHandle[i].StaRecTagHandler(pAd, msg, &StaRecCfg);

				if (Ret == 0)
					u2TLVNumber++;
			} else {
				MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
						 "StaRecTag = %d no corresponding function handler.\n",
						  StaRecHandle[i].StaRecTag);
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "u2TLVNumber(%d)\n", u2TLVNumber);
	/*insert to head*/
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(u2TLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendHeadCmdMsg(msg, (char *)&CmdStaRecUpdate,
						  sizeof(CMD_STAREC_UPDATE_T));
	/* Send out CMD */
	call_fw_cmd_notifieriers(WO_CMD_STA_REC, pAd, msg->net_pkt);
	Ret = AndesSendCmdMsg(pAd, msg);

	while ((Ret != WLAN_STATUS_SUCCESS) && (retry)) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_WARN,
				 "(FW Ret = 0x%x retry=%d)\n", Ret, retry);
		retry--;
		/* something wrong with STAREC update */
		Ret = CmdExtStaRecUpdate_ReSyncDelete(pAd, &StaRecCfg);

		if (Ret != WLAN_STATUS_SUCCESS)
			continue;

		goto CmdExtStaRecUpdate_restart;
	}



error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdExtStaRecBaUpdate(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg)
{
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	CMD_STAREC_UPDATE_T CmdStaRecUpdate = {0};
	CMD_STAREC_BA_T CmdStaRecBa;
	UINT32 MsgLen;
	INT32 Ret = 0;
	UINT16 ucTLVNumber = 0;

	MsgLen = sizeof(CMD_STAREC_UPDATE_T) + sizeof(CMD_STAREC_BA_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	ucTLVNumber = 1;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucTLVNumber(%d)\n", ucTLVNumber);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = StaRecBaCfg.BssIdx;
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, StaRecBaCfg.WlanIdx);
	CmdStaRecUpdate.ucMuarIdx = StaRecBaCfg.MuarIdx;
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, sizeof(CMD_STAREC_UPDATE_T));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "StaRecUpdate:\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucBssIndex=%d, WlanIdx=%d, ucMuarIdx=%d, u2TotalElementNum=%d\n",
			  CmdStaRecUpdate.ucBssIndex,
			  WCID_GET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL),
			  CmdStaRecUpdate.ucMuarIdx, CmdStaRecUpdate.u2TotalElementNum);
	/* Fill BA related parameters */
	NdisZeroMemory(&CmdStaRecBa, sizeof(CMD_STAREC_BA_T));

	CmdStaRecBa.ucAmsduCap = StaRecBaCfg.amsdu;
	CmdStaRecBa.u2BaStartSeq = StaRecBaCfg.sn;
	CmdStaRecBa.u2BaWinSize = StaRecBaCfg.ba_wsize;
	CmdStaRecBa.ucBaEenable = StaRecBaCfg.BaEnable;
	CmdStaRecBa.u2Tag = STA_REC_BA;
	CmdStaRecBa.u2Length = sizeof(CMD_STAREC_BA_T);
	CmdStaRecBa.ucTid = StaRecBaCfg.tid;
	CmdStaRecBa.ucBaDirection = StaRecBaCfg.baDirection;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "BaInfo:\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "u2Tag=%d, u2Length=%d, ucTid=%d, u2BaDirectin=%d, BaEnable=%d, u2BaStartSeq=%d, u2BaWinSize=%d, ucAmsduCap=%d\n",
			  CmdStaRecBa.u2Tag, CmdStaRecBa.u2Length, CmdStaRecBa.ucTid, CmdStaRecBa.ucBaDirection,
			  CmdStaRecBa.ucBaEenable, le2cpu16(CmdStaRecBa.u2BaStartSeq), le2cpu16(CmdStaRecBa.u2BaWinSize), CmdStaRecBa.ucAmsduCap);
#ifdef CFG_BIG_ENDIAN
	CmdStaRecBa.u2Tag = cpu2le16(CmdStaRecBa.u2Tag);
	CmdStaRecBa.u2Length = cpu2le16(CmdStaRecBa.u2Length);
	CmdStaRecBa.u2BaStartSeq = cpu2le16(CmdStaRecBa.u2BaStartSeq);
	CmdStaRecBa.u2BaWinSize = cpu2le16(CmdStaRecBa.u2BaWinSize);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecBa, sizeof(CMD_STAREC_BA_T));
	/* Send out CMD */
	call_fw_cmd_notifieriers(WO_CMD_STA_REC, pAd, msg->net_pkt);
	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "(Ret = %d)\n", Ret);
	return Ret;
}


#ifdef HTC_DECRYPT_IOT
INT32 CmdExtStaRecAADOmUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT8 AadOm)
{
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	CMD_STAREC_UPDATE_T CmdStaRecUpdate = {0};
	CMD_STAREC_AADOM_T CmdStaRecAadOm;
	UINT32 MsgLen;
	INT32 Ret = 0;
	UINT16 ucTLVNumber = 0;

	MsgLen = sizeof(CMD_STAREC_UPDATE_T) + sizeof(CMD_STAREC_AADOM_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	ucTLVNumber = 1;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucTLVNumber(%d)\n", ucTLVNumber);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = 0; /* useless for this cmd */
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, Wcid);
	CmdStaRecUpdate.ucMuarIdx = 0; /* useless for this cmd */
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, sizeof(CMD_STAREC_UPDATE_T));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "StaRecUpdate:\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucBssIndex=%d, WlanIdx=%d, ucMuarIdx=%d, u2TotalElementNum=%d\n",
			  CmdStaRecUpdate.ucBssIndex,
			  WCID_GET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL),
			  CmdStaRecUpdate.ucMuarIdx, CmdStaRecUpdate.u2TotalElementNum);

	NdisZeroMemory(&CmdStaRecAadOm, sizeof(CMD_STAREC_AADOM_T));

	CmdStaRecAadOm.ucAadOm = AadOm;
	CmdStaRecAadOm.u2Tag = STA_REC_WTBL_AADOM;
	CmdStaRecAadOm.u2Length = sizeof(CMD_STAREC_AADOM_T);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "u2Tag=%d, u2Length=%d, Wcid=%u, ucAadOm=%u\n",
			  CmdStaRecAadOm.u2Tag, CmdStaRecAadOm.u2Length, Wcid, CmdStaRecAadOm.ucAadOm);

#ifdef CFG_BIG_ENDIAN
	CmdStaRecAadOm.u2Tag = cpu2le16(CmdStaRecAadOm.u2Tag);
	CmdStaRecAadOm.u2Length = cpu2le16(CmdStaRecAadOm.u2Length);
#endif

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecAadOm, sizeof(CMD_STAREC_AADOM_T));
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "(Ret = %d)\n", Ret);
	return Ret;
}
#endif /* HTC_DECRYPT_IOT */

INT32 CmdExtStaRecPsmUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Wcid,
	UINT8 Psm)
{
	struct _CMD_ATTRIBUTE attr = {0};
	struct cmd_msg *msg = NULL;
	CMD_STAREC_UPDATE_T CmdStaRecUpdate = {0};
	struct _STAREC_PSM_T CmdStaRecPsm;
	UINT32 MsgLen;
	INT32 Ret = 0;
	UINT16 ucTLVNumber = 0;

	MsgLen = sizeof(CMD_STAREC_UPDATE_T) + sizeof(CmdStaRecPsm);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	ucTLVNumber = 1;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucTLVNumber(%d)\n", ucTLVNumber);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_STAREC_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_STAREC_UPDATE_T));
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtStaRecUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill WLAN related header here*/
	CmdStaRecUpdate.ucBssIndex = 0; /* useless for this cmd */
	WCID_SET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL, Wcid);
	CmdStaRecUpdate.ucMuarIdx = 0; /* useless for this cmd */
	CmdStaRecUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdStaRecUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecUpdate, sizeof(CMD_STAREC_UPDATE_T));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "StaRecUpdate:\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "ucBssIndex=%d, WlanIdx=%d, ucMuarIdx=%d, u2TotalElementNum=%d\n",
			  CmdStaRecUpdate.ucBssIndex,
			  WCID_GET_H_L(CmdStaRecUpdate.ucWlanIdxHnVer, CmdStaRecUpdate.ucWlanIdxL),
			  CmdStaRecUpdate.ucMuarIdx, CmdStaRecUpdate.u2TotalElementNum);

	NdisZeroMemory(&CmdStaRecPsm, sizeof(CMD_STAREC_AADOM_T));

	CmdStaRecPsm.ucPsmMode = Psm;
	CmdStaRecPsm.u2Tag = STA_REC_WTBL_PSM;
	CmdStaRecPsm.u2Length = sizeof(CMD_STAREC_AADOM_T);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "u2Tag=%d, u2Length=%d, Wcid=%u, ucPsmMode=%u\n",
			  CmdStaRecPsm.u2Tag, CmdStaRecPsm.u2Length, Wcid, CmdStaRecPsm.ucPsmMode);

#ifdef CFG_BIG_ENDIAN
	CmdStaRecPsm.u2Tag = cpu2le16(CmdStaRecPsm.u2Tag);
	CmdStaRecPsm.u2Length = cpu2le16(CmdStaRecPsm.u2Length);
#endif

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdStaRecPsm, sizeof(CMD_STAREC_AADOM_T));
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "(Ret = %d)\n", Ret);
	return Ret;
}

static VOID bssConnectOwnDev(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_CONNECT_OWN_DEV_T CmdBssiinfoConnectOwnDev = {0};

	CmdBssiinfoConnectOwnDev.u2Tag = BSS_INFO_OWN_MAC;
	CmdBssiinfoConnectOwnDev.u2Length =
		sizeof(CMD_BSSINFO_CONNECT_OWN_DEV_T);
	CmdBssiinfoConnectOwnDev.ucHwBSSIndex =
		(bss_info->OwnMacIdx > HW_BSSID_MAX) ?
		HW_BSSID_0 : bss_info->OwnMacIdx;
	CmdBssiinfoConnectOwnDev.ucOwnMacIdx = bss_info->OwnMacIdx;
	CmdBssiinfoConnectOwnDev.ucBandIdx = bss_info->ucBandIdx;
	CmdBssiinfoConnectOwnDev.u4ConnectionType =
		cpu2le32(bss_info->u4ConnectionType);
	/* Fill TLV format */
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "ucHwBSSIndex = %d, ucOwnMacIdx = %d, ucBandIdx = %d, u4ConnectionType = %x\n",
			  CmdBssiinfoConnectOwnDev.ucHwBSSIndex,
			  CmdBssiinfoConnectOwnDev.ucOwnMacIdx,
			  CmdBssiinfoConnectOwnDev.ucBandIdx,
			  bss_info->u4ConnectionType);
#ifdef CFG_BIG_ENDIAN
	CmdBssiinfoConnectOwnDev.u2Tag = cpu2le16(CmdBssiinfoConnectOwnDev.u2Tag);
	CmdBssiinfoConnectOwnDev.u2Length = cpu2le16(CmdBssiinfoConnectOwnDev.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssiinfoConnectOwnDev,
					  sizeof(CMD_BSSINFO_CONNECT_OWN_DEV_T));
}

static VOID bssUpdateBssInfoBasic(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_BASIC_T CmdBssInfoBasic = {0};

	/* Fill TLV format */
	CmdBssInfoBasic.u2Tag = BSS_INFO_BASIC;
	CmdBssInfoBasic.u2Length = sizeof(CMD_BSSINFO_BASIC_T);
	CmdBssInfoBasic.u4NetworkType = cpu2le32(bss_info->NetworkType);

	if (bss_info->bss_state >= BSS_ACTIVE)
		CmdBssInfoBasic.ucActive = TRUE;
	else
		CmdBssInfoBasic.ucActive = FALSE;

	CmdBssInfoBasic.u2BcnInterval = cpu2le16(bss_info->bcn_period);
	CmdBssInfoBasic.ucDtimPeriod = bss_info->dtim_period;

	os_move_mem(CmdBssInfoBasic.aucBSSID, bss_info->Bssid, MAC_ADDR_LEN);
	CmdBssInfoBasic.ucWmmIdx = bss_info->WmmIdx;
	WCID_SET_H_L(CmdBssInfoBasic.ucBmcWlanIdxHnVer,
				 CmdBssInfoBasic.ucBmcWlanIdxL,
				 bss_info->bmc_wlan_idx);
	CmdBssInfoBasic.ucCipherSuit = bss_info->CipherSuit;
	CmdBssInfoBasic.ucPhyMode = bss_info->ucPhyMode;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "u4NetworkType = %d, ucActive = %d, u2BcnInterval = %d, ucWmmIdx = %d,ucDtimPeriod = %d, bmc_wlan_idx = %d, ucCipherSuit=%d, ucPhyMode=%x,BSSID = "MACSTR"\n",
			  bss_info->NetworkType,
			  CmdBssInfoBasic.ucActive,
			  le2cpu16(CmdBssInfoBasic.u2BcnInterval),
			  CmdBssInfoBasic.ucWmmIdx,
			  CmdBssInfoBasic.ucDtimPeriod,
			  WCID_GET_H_L(CmdBssInfoBasic.ucBmcWlanIdxHnVer, CmdBssInfoBasic.ucBmcWlanIdxL),
			  CmdBssInfoBasic.ucCipherSuit,
			  CmdBssInfoBasic.ucPhyMode,
			  MAC2STR(CmdBssInfoBasic.aucBSSID));

#ifdef DOT11V_MBSSID_SUPPORT
	CmdBssInfoBasic.uc11vMaxBssidIndicator = bss_info->max_bssid_indicator;
	CmdBssInfoBasic.uc11vBssidIdx = bss_info->mbssid_index;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "  uc11vMaxBssidIndicator = %d, uc11vBssidIdx = %d\n",
			  CmdBssInfoBasic.uc11vMaxBssidIndicator,
			  CmdBssInfoBasic.uc11vBssidIdx);
#endif

#ifdef CFG_BIG_ENDIAN
	CmdBssInfoBasic.u2Tag = cpu2le16(CmdBssInfoBasic.u2Tag);
	CmdBssInfoBasic.u2Length = cpu2le16(CmdBssInfoBasic.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoBasic, sizeof(CMD_BSSINFO_BASIC_T));
}

static VOID bssUpdateChannel(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_RF_CH_T CmdBssInfoRfCh = {0};
	struct freq_oper *chan_oper = &bss_info->chan_oper;

	/* Fill TLV format */
	CmdBssInfoRfCh.u2Tag = BSS_INFO_RF_CH;
	CmdBssInfoRfCh.u2Length = sizeof(CMD_BSSINFO_RF_CH_T);
	CmdBssInfoRfCh.ucPrimaryChannel = chan_oper->prim_ch;
	CmdBssInfoRfCh.ucCenterChannelSeg0 = chan_oper->cen_ch_1;
	CmdBssInfoRfCh.ucCenterChannelSeg1 = chan_oper->cen_ch_2;
	CmdBssInfoRfCh.ucBandwidth = chan_oper->bw;
	CmdBssInfoRfCh.ucHetbRU26Disable = FALSE;
	CmdBssInfoRfCh.ucHetbAllDisable = TRUE;
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(bss_info->ucPhyMode)
		&& bss_info->u4ConnectionType == CONNECTION_INFRA_STA) {

		CmdBssInfoRfCh.ucHetbAllDisable = FALSE;

		if (is_ru26_disable_channel(pAd, chan_oper->prim_ch, bss_info->ucPhyMode))
			CmdBssInfoRfCh.ucHetbRU26Disable = TRUE;
	}
#endif /* DOT11_HE_AX */

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
			"ucPrimCh=%d, ucCentChSeg0=%d, ucCentChSeg1=%d, BW=%d, ucHetbRU26Disable=%d, ucHetbAllDisable=%d\n",
			CmdBssInfoRfCh.ucPrimaryChannel,
			CmdBssInfoRfCh.ucCenterChannelSeg0,
			CmdBssInfoRfCh.ucCenterChannelSeg1,
			CmdBssInfoRfCh.ucBandwidth,
			CmdBssInfoRfCh.ucHetbRU26Disable,
			CmdBssInfoRfCh.ucHetbAllDisable);
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoRfCh.u2Tag = cpu2le16(CmdBssInfoRfCh.u2Tag);
	CmdBssInfoRfCh.u2Length = cpu2le16(CmdBssInfoRfCh.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoRfCh, sizeof(CMD_BSSINFO_RF_CH_T));
}

static VOID pmUpdateBssInfoPM(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
#ifdef CONFIG_STA_SUPPORT
	CMD_BSSINFO_PM_T CmdBssInfoPm = {0};
	/* Fill TLV format */
	CmdBssInfoPm.u2Tag = BSS_INFO_PM;
	CmdBssInfoPm.u2Length = sizeof(CMD_BSSINFO_PM_T);
	CmdBssInfoPm.ucKeepAliveEn = bss_info->rBssInfoPm.ucKeepAliveEn;
	CmdBssInfoPm.ucKeepAlivePeriod = bss_info->rBssInfoPm.ucKeepAlivePeriod;
	CmdBssInfoPm.ucBeaconLossReportEn = bss_info->rBssInfoPm.ucBeaconLossReportEn;
	CmdBssInfoPm.ucBeaconLossCount = bss_info->rBssInfoPm.ucBeaconLossCount;
	/* TODO: Carter, check with Hanmin, what's the meaning for STA? Does Apcli need it also? */
	CmdBssInfoPm.ucBcnSpState0Min = 5;
	CmdBssInfoPm.ucBcnSpState0Max = 20;
	CmdBssInfoPm.ucBcnSpState1Min = 15;
	CmdBssInfoPm.ucBcnSpState1Max = bss_info->bcn_period;
	CmdBssInfoPm.ucBcnSpState2Min = 31;
	CmdBssInfoPm.ucBcnSpState2Max = bss_info->bcn_period;
	CmdBssInfoPm.ucBcnSpState3Min = 63;
	CmdBssInfoPm.ucBcnSpState3Max = bss_info->bcn_period;
	CmdBssInfoPm.ucBcnSpState4Min = 0;
	CmdBssInfoPm.ucBcnSpState4Max = bss_info->bcn_period;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "(BSS_INFO_PM), ucKeepAliveEn = %d, ucKeepAlivePeriod = %d, ucBeaconLossReportEn = %d, ucBeaconLossCount = %d, S0(%d, %d), S1(%d, %d), S2(%d, %d), S3(%d, %d), S4(%d, %d)\n",
			  CmdBssInfoPm.ucKeepAliveEn,
			  CmdBssInfoPm.ucKeepAlivePeriod,
			  CmdBssInfoPm.ucBeaconLossReportEn,
			  CmdBssInfoPm.ucBeaconLossCount,
			  CmdBssInfoPm.ucBcnSpState0Min,
			  CmdBssInfoPm.ucBcnSpState0Max,
			  CmdBssInfoPm.ucBcnSpState1Min,
			  CmdBssInfoPm.ucBcnSpState1Max,
			  CmdBssInfoPm.ucBcnSpState2Min,
			  CmdBssInfoPm.ucBcnSpState2Max,
			  CmdBssInfoPm.ucBcnSpState3Min,
			  CmdBssInfoPm.ucBcnSpState3Max,
			  CmdBssInfoPm.ucBcnSpState4Min,
			  CmdBssInfoPm.ucBcnSpState4Max);
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoPm.u2Tag = cpu2le16(CmdBssInfoPm.u2Tag);
	CmdBssInfoPm.u2Length = cpu2le16(CmdBssInfoPm.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoPm, sizeof(CMD_BSSINFO_PM_T));
#endif /*CONFIG_STA_SUPPORT*/
}

static VOID pmUpdateBssUapsd(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_UAPSD_T CmdBssInfoUapsd = {0};

	/* Fill TLV format */
	CmdBssInfoUapsd.u2Tag = BSS_INFO_UAPSD;
	CmdBssInfoUapsd.u2Length = sizeof(CMD_BSSINFO_UAPSD_T);
	/*initial value for uapsd*/
	CmdBssInfoUapsd.ucIsUapsdSupported = FALSE;
	/*update uapsd if necessary*/
#ifdef CONFIG_STA_SUPPORT
#ifdef UAPSD_SUPPORT
	CmdBssInfoUapsd.ucIsUapsdSupported = bss_info->uapsd_cfg.uapsd_en;
	CmdBssInfoUapsd.ucUapsdTriggerAC = bss_info->uapsd_cfg.uapsd_trigger_ac;
	CmdBssInfoUapsd.ucUapsdDeliveryAC = CmdBssInfoUapsd.ucUapsdTriggerAC;
	/* un-limit @10140630 */
	CmdBssInfoUapsd.u2UapsdServicePeriodTO = 0xFFFF;
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "ucIsUapsdSupported = %d, ucUapsdTriggerAC = 0x%x,ucUapsdTriggerAC = 0x%x, u2UapsdServicePeriodTO = 0x%x\n",
			  CmdBssInfoUapsd.ucIsUapsdSupported,
			  CmdBssInfoUapsd.ucUapsdTriggerAC,
			  CmdBssInfoUapsd.ucUapsdDeliveryAC,
			  CmdBssInfoUapsd.u2UapsdServicePeriodTO);
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoUapsd.u2Tag = cpu2le16(CmdBssInfoUapsd.u2Tag);
	CmdBssInfoUapsd.u2Length = cpu2le16(CmdBssInfoUapsd.u2Length);
	CmdBssInfoUapsd.u2UapsdServicePeriodTO = cpu2le16(CmdBssInfoUapsd.u2UapsdServicePeriodTO);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoUapsd, sizeof(CMD_BSSINFO_UAPSD_T));
}

static VOID bssUpdateRssiRmDetParams(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_RSSI_RM_DET_T CmdBssInfoRoamDetection = {0};

	/* Fill TLV format */
	CmdBssInfoRoamDetection.u2Tag = BSS_INFO_ROAM_DETECTION;
	CmdBssInfoRoamDetection.u2Length = sizeof(CMD_BSSINFO_RSSI_RM_DET_T);
#ifdef CONFIG_STA_SUPPORT
	CmdBssInfoRoamDetection.fgEnable = TRUE;
	CmdBssInfoRoamDetection.ucPktSource = 0x02; /* Beacon */
	CmdBssInfoRoamDetection.ucPktMAPara = 0x02;     /* 1/4 */
	CmdBssInfoRoamDetection.cRssiCCKHighThr = 0;
	CmdBssInfoRoamDetection.cRssiCCKLowThr = bss_info->dbm_to_roam;
	CmdBssInfoRoamDetection.cRssiOFDMHighThr = 0;
	CmdBssInfoRoamDetection.cRssiOFDMLowThr = bss_info->dbm_to_roam;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "fgEnable = %d, ucPktSource = %d, ucPktMAPara = %d, cRssiCCKLowThr = %d, cRssiOFDMLowThr = %d\n",
			  CmdBssInfoRoamDetection.fgEnable,
			  CmdBssInfoRoamDetection.ucPktSource,
			  CmdBssInfoRoamDetection.ucPktMAPara,
			  CmdBssInfoRoamDetection.cRssiCCKLowThr,
			  CmdBssInfoRoamDetection.cRssiOFDMLowThr);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoRoamDetection.u2Tag = cpu2le16(CmdBssInfoRoamDetection.u2Tag);
	CmdBssInfoRoamDetection.u2Length = cpu2le16(CmdBssInfoRoamDetection.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoRoamDetection, sizeof(CMD_BSSINFO_RSSI_RM_DET_T));
}

static VOID bssUpdateExtBssInfo(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	UCHAR ExtBssidIdx;
	CMD_BSSINFO_EXT_BSS_INFO_T CmdBssInfoExtBssInfo = {0};
	/* this feature is only for Omac 0x11~0x1f */
	ASSERT(bss_info->OwnMacIdx > HW_BSSID_MAX);
	ExtBssidIdx = (bss_info->OwnMacIdx & 0xf);
	CmdBssInfoExtBssInfo.u2Tag = BSS_INFO_EXT_BSS;
	CmdBssInfoExtBssInfo.u2Length = sizeof(CMD_BSSINFO_EXT_BSS_INFO_T);
	CmdBssInfoExtBssInfo.ucMbssTsfOffset = ExtBssidIdx * BCN_TRANSMIT_ESTIMATE_TIME;
	/* Fill TLV format */
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "(BSSINFO_EXT_BSS_INFO), ExtBssidIdx = %d, ucMbssTsfOffset = %d\n",
			  ExtBssidIdx,
			  CmdBssInfoExtBssInfo.ucMbssTsfOffset);
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoExtBssInfo.u2Tag = cpu2le16(CmdBssInfoExtBssInfo.u2Tag);
	CmdBssInfoExtBssInfo.u2Length = cpu2le16(CmdBssInfoExtBssInfo.u2Length);
	CmdBssInfoExtBssInfo.ucMbssTsfOffset = cpu2le32(CmdBssInfoExtBssInfo.ucMbssTsfOffset);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoExtBssInfo, sizeof(CMD_BSSINFO_EXT_BSS_INFO_T));
}


static VOID bssUpdateBmcMngRate(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_BMC_RATE_T CmdBssInfoBmcRate = {0};

	if (bss_info->bss_state >= BSS_ACTIVE) {
		CmdBssInfoBmcRate.u2BcTransmit = cpu2le16((UINT16)(bss_info->BcTransmit.word));
		CmdBssInfoBmcRate.u2McTransmit = cpu2le16((UINT16)(bss_info->McTransmit.word));
		CmdBssInfoBmcRate.ucPreambleMode =
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	}
#ifdef TXRX_STAT_SUPPORT
	{
		ULONG Multicast_Tx_Rate;
		pAd->ApCfg.MBSSID[bss_info->ucBssIndex].stat_bss.LastMulticastTxRate.word = bss_info->McTransmit.word;
		getRate(bss_info->McTransmit, &Multicast_Tx_Rate);
	}
#endif
	CmdBssInfoBmcRate.u2Tag = BSS_INFO_BROADCAST_INFO;
	CmdBssInfoBmcRate.u2Length = sizeof(CMD_BSSINFO_BMC_RATE_T);
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 " (BSS_INFO_BROADCAST_INFO), CmdBssInfoBmcRate.u2BcTransmit= %d, CmdBssInfoBmcRate.u2McTransmit = %d\n",
			  le2cpu16(CmdBssInfoBmcRate.u2BcTransmit),
			  le2cpu16(CmdBssInfoBmcRate.u2McTransmit));
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoBmcRate.u2Tag = cpu2le16(CmdBssInfoBmcRate.u2Tag);
	CmdBssInfoBmcRate.u2Length = cpu2le16(CmdBssInfoBmcRate.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoBmcRate, sizeof(CMD_BSSINFO_BMC_RATE_T));
}

static VOID bssUpdateSyncModeCtrl(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_SYNC_MODE_CTRL_T CmdBssInfoSyncModeCtrl = {0};

	if (bss_info->bss_state >= BSS_ACTIVE) {
		CmdBssInfoSyncModeCtrl.fgIsEnableSync = TRUE;
		CmdBssInfoSyncModeCtrl.u2BcnInterval = cpu2le16(bss_info->bcn_period);
		CmdBssInfoSyncModeCtrl.ucDtimPeriod = bss_info->dtim_period;
	} else
		CmdBssInfoSyncModeCtrl.fgIsEnableSync = FALSE;

	CmdBssInfoSyncModeCtrl.u2Tag = BSS_INFO_SYNC_MODE;
	CmdBssInfoSyncModeCtrl.u2Length = sizeof(CMD_BSSINFO_SYNC_MODE_CTRL_T);
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoSyncModeCtrl.u2Tag = cpu2le16(CmdBssInfoSyncModeCtrl.u2Tag);
	CmdBssInfoSyncModeCtrl.u2Length = cpu2le16(CmdBssInfoSyncModeCtrl.u2Length);
#endif
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoSyncModeCtrl, sizeof(CMD_BSSINFO_SYNC_MODE_CTRL_T));
}


static VOID bssUpdateRA(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	CMD_BSSINFO_AUTO_RATE_CFG_T CmdBssInfoAutoRateCfg = {0};

	os_zero_mem(&CmdBssInfoAutoRateCfg, sizeof(CmdBssInfoAutoRateCfg));
	BssInfoRACommCfgSet(&bss_info->ra_cfg, &CmdBssInfoAutoRateCfg);
	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoAutoRateCfg,
					sizeof(CMD_BSSINFO_AUTO_RATE_CFG_T));
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
}

#ifdef HW_TX_AMSDU_SUPPORT
static VOID bss_update_hw_amsdu(
	struct _RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	CMD_BSSINFO_HW_AMSDU_INFO_T CmdBssInfoHwAmsduInfo = {0};

	os_zero_mem(&CmdBssInfoHwAmsduInfo, sizeof(CmdBssInfoHwAmsduInfo));
	/* Fill TLV format */
	CmdBssInfoHwAmsduInfo.u2Tag = cpu2le16(BSS_INFO_HW_AMSDU);
	CmdBssInfoHwAmsduInfo.u2Length = cpu2le16(sizeof(CmdBssInfoHwAmsduInfo));
	CmdBssInfoHwAmsduInfo.fgHwAmsduEn = 1;
	CmdBssInfoHwAmsduInfo.u4TxdCmpBitmap_0 = cpu2le32(0xFFFF);
	CmdBssInfoHwAmsduInfo.u4TxdCmpBitmap_1 = cpu2le32(cap->amsdu_txdcmp);
	CmdBssInfoHwAmsduInfo.u2TxdTriggerThres = cpu2le16(2);

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoHwAmsduInfo,
					  sizeof(CMD_BSSINFO_HW_AMSDU_INFO_T));
}
#endif


#ifdef DOT11_HE_AX
static VOID bss_update_bss_color(
		struct _RTMP_ADAPTER *pAd,
		BSS_INFO_ARGUMENT_T *bss_info,
		struct cmd_msg *msg)
{
	CMD_BSSINFO_BSS_COLOR_T CmdBssInfoBssColor = {0};
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;

	os_zero_mem(&CmdBssInfoBssColor, sizeof(CmdBssInfoBssColor));
	/* Fill TLV format */
	CmdBssInfoBssColor.u2Tag = BSS_INFO_BSS_COLOR;
	CmdBssInfoBssColor.u2Length = sizeof(CmdBssInfoBssColor);
	CmdBssInfoBssColor.fgIsDisable = bss_color->disabled;
	CmdBssInfoBssColor.ucBssColor = bss_color->color;
#ifdef CFG_BIG_ENDIAN
	CmdBssInfoBssColor.u2Tag = cpu2le16(CmdBssInfoBssColor.u2Tag);
	CmdBssInfoBssColor.u2Length= cpu2le16(CmdBssInfoBssColor.u2Length);
#endif

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoBssColor, sizeof(CMD_BSSINFO_BSS_COLOR_T));
}

static VOID bss_update_basic_he(
	struct _RTMP_ADAPTER *ad,
	BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_HE_BASIC_T he_info = {0};
	struct he_mcs_info *mcs = &bss_info->he_bss.max_nss_mcs;
	int i;

	os_zero_mem(&he_info, sizeof(he_info));
	/* Fill TLV format */
	he_info.u2Tag = BSS_INFO_HE_BASIC;
	he_info.u2Length = sizeof(he_info);

	he_info.ucDefaultPEDuration = bss_info->he_bss.default_pe_dur;
	he_info.ucVhtOperInfoPresent = bss_info->he_bss.vht_oper_info_present;
	he_info.u2TxopDurationRtsThreshold = bss_info->he_bss.txop_dur_rts_thr;
	for (i = 0 ; i < HE_MAX_SUPPORT_STREAM; i++) {
		he_info.au2MaxNssMcs[CMD_HE_MCS_BW80] |= (mcs->bw80_mcs[i] << (i * 2));
		he_info.au2MaxNssMcs[CMD_HE_MCS_BW160] |= (mcs->bw160_mcs[i] << (i * 2));
		he_info.au2MaxNssMcs[CMD_HE_MCS_BW8080] |= (mcs->bw8080_mcs[i] << (i * 2));
	}

#ifdef CFG_BIG_ENDIAN
	he_info.u2Tag = cpu2le16(he_info.u2Tag);
	he_info.u2Length = cpu2le16(he_info.u2Length);
	he_info.u2TxopDurationRtsThreshold = cpu2le16(he_info.u2TxopDurationRtsThreshold);
	for (i = 0 ; i < CMD_HE_MCS_BW_NUM ; i++)
		he_info.au2MaxNssMcs[i] = cpu2le16(he_info.au2MaxNssMcs[i]);
#endif

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&he_info,
					  sizeof(CMD_BSSINFO_HE_BASIC_T));
}
#endif /*#ifdef DOT11_HE_AX*/

static VOID bss_update_prot_info(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	CMD_BSSINFO_PROT_INFO_T bss_prot_info = {0};
	struct prot_info *prot = &bss_info->prot;

	os_zero_mem(&bss_prot_info, sizeof(bss_prot_info));
	/* Fill TLV format */
	bss_prot_info.u2Tag = BSS_INFO_PROTECT_INFO;
	bss_prot_info.u2Length = sizeof(bss_prot_info);
	bss_prot_info.u4ProtectUpdateType = prot->type;
	bss_prot_info.u4ProtectMode = prot->cookie.protect_mode;
	bss_prot_info.u4RtsLengthThld = prot->cookie.rts.len_thld;
	bss_prot_info.u2TxopDurRtsThld = prot->cookie.txop_dur_rts_thld;
	bss_prot_info.ucRtsPktCntThld = prot->cookie.rts.pkt_thld;
#ifdef CFG_BIG_ENDIAN
	bss_prot_info.u2Tag = cpu2le16(bss_prot_info.u2Tag);
	bss_prot_info.u2Length = cpu2le16(bss_prot_info.u2Length);
	bss_prot_info.u4ProtectUpdateType = cpu2le32(bss_prot_info.u4ProtectUpdateType);
	bss_prot_info.u4ProtectMode = cpu2le32(bss_prot_info.u4ProtectMode);
	bss_prot_info.u4RtsLengthThld = cpu2le32(bss_prot_info.u4RtsLengthThld);
	bss_prot_info.u2TxopDurRtsThld = cpu2le16(bss_prot_info.u2TxopDurRtsThld);
#endif

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&bss_prot_info, sizeof(CMD_BSSINFO_PROT_INFO_T));
}

#ifdef CONFIG_AP_SUPPORT
static UINT16 bss_update_offload_bcn_csa(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	P_CMD_OFFLOAD_BCN_CSA_T bcn_csa_info)
{
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
	UINT16 u2SubTagLen = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return u2SubTagLen;
	}

	if ((wdev->csa_count != 0) && BCN_REASON_EQUAL(bss_info->bUpdateReason, BCN_UPDATE_CSA)) {
		u2SubTagLen = sizeof(CMD_OFFLOAD_BCN_CSA_T);

		/* DW alignment */
		u2SubTagLen = (u2SubTagLen & 0x3) ? ((u2SubTagLen | 0x3) + 1) : u2SubTagLen;

		bcn_csa_info->u2SubTag = SUB_TAG_BCN_CSA;
		bcn_csa_info->u2Length = u2SubTagLen;
		bcn_csa_info->ucCsaCount = wdev->csa_count;
#ifdef CFG_BIG_ENDIAN
		bcn_csa_info->u2SubTag = cpu2le16(bcn_csa_info->u2SubTag);
		bcn_csa_info->u2Length = cpu2le16(bcn_csa_info->u2Length);
#endif
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
			" BSS(%d),UpdateReason(%d) CsaCount = %d\n",
			bss_info->ucBssIndex, bss_info->bUpdateReason, bcn_csa_info->ucCsaCount);
	}

	return u2SubTagLen;
}

static UINT16 bss_update_offload_bcn_bcc(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	P_CMD_OFFLOAD_BCN_BCC_T bcn_bcc_info)
{
	UINT16 u2SubTagLen = 0;
#ifdef DOT11_HE_AX
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
	struct bss_color_ctrl *bss_color = &bss_info->bss_color;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return u2SubTagLen;
	}

	if (wdev->bcn_buf.bcc_ie_location != 0) {
		u2SubTagLen = sizeof(CMD_OFFLOAD_BCN_BCC_T);

		/* DW alignment */
		u2SubTagLen = (u2SubTagLen & 0x3) ? ((u2SubTagLen | 0x3) + 1) : u2SubTagLen;

		bcn_bcc_info->u2SubTag = SUB_TAG_BCN_BCC;
		bcn_bcc_info->u2Length = u2SubTagLen;
		bcn_bcc_info->ucBccCount = bss_color->u.ap_ctrl.bcc_count;
#ifdef CFG_BIG_ENDIAN
		bcn_bcc_info->u2SubTag = cpu2le16(bcn_bcc_info->u2SubTag);
		bcn_bcc_info->u2Length = cpu2le16(bcn_bcc_info->u2Length);
#endif
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
			"BSS(%d), BccCount = %d\n",
			bss_info->ucBssIndex, bcn_bcc_info->ucBccCount);
	}
#endif

	return u2SubTagLen;
}

static UINT16 bss_update_offload_bcn_mbssid(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	P_CMD_OFFLOAD_BCN_MBSSID_T bcn_mbss_info)
{
	UINT16 u2SubTagLen = 0;
#ifdef DOT11V_MBSSID_SUPPORT

	if (IS_BSSID_11V_ENABLED(ad)) {
		BSS_STRUCT *pMbss = NULL;
		INT32 IdBss;

		u2SubTagLen = sizeof(CMD_OFFLOAD_BCN_MBSSID_T);

		/* DW alignment */
		u2SubTagLen = (u2SubTagLen & 0x3) ? ((u2SubTagLen | 0x3) + 1) : u2SubTagLen;

		bcn_mbss_info->u2SubTag = SUB_TAG_BCN_MBSSID;
		bcn_mbss_info->u2Length = u2SubTagLen;
#ifdef CFG_BIG_ENDIAN
		bcn_mbss_info->u2SubTag = cpu2le16(bcn_mbss_info->u2SubTag);
		bcn_mbss_info->u2Length = cpu2le16(bcn_mbss_info->u2Length);
#endif

		for (IdBss = MAIN_MBSSID; IdBss < ad->ApCfg.BssidNum; IdBss++) {
			pMbss = &ad->ApCfg.MBSSID[IdBss];
			/* update TIM offset */
			MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
					"BSS(%d), IE Offset = %d\n",
					IdBss, pMbss->wdev.bcn_buf.tim_ie_offset);

			bcn_mbss_info->u2MbssidIeOffset[IdBss] =
				cpu2le16(pMbss->wdev.bcn_buf.tim_ie_offset);

			/* build global 11v mbssid bitmap */
			if (ad->ApCfg.dot11v_mbssid_bitmap & (1 << pMbss->mbss_idx))
				bcn_mbss_info->u4Dot11vMbssidBitmap |= (1 << IdBss);
		}
		bcn_mbss_info->u4Dot11vMbssidBitmap = cpu2le32(bcn_mbss_info->u4Dot11vMbssidBitmap);
	}
#endif

	return u2SubTagLen;
}

static UINT16 bss_update_offload_bcn_content(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	P_CMD_OFFLOAD_BCN_CONTENT_T bcn_cont_info)
{
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
	PBCN_BUF_STRUCT bcn_buf = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	UINT16 u2SubTagLen = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return u2SubTagLen;
	}
	bcn_buf = &wdev->bcn_buf;

	if (bcn_buf && (bcn_buf->bBcnSntReq == TRUE) && (bcn_buf->BeaconPkt)) {
		u2SubTagLen = sizeof(CMD_OFFLOAD_BCN_CONTENT_T) + cap->tx_hw_hdr_len + bcn_buf->FrameLen;

		/* DW alignment */
		u2SubTagLen = (u2SubTagLen & 0x3) ? ((u2SubTagLen | 0x3) + 1) : u2SubTagLen;

		bcn_cont_info->u2SubTag = SUB_TAG_BCN_CONTENT;
		bcn_cont_info->u2Length = u2SubTagLen;
		bcn_cont_info->u2TimIeOffset = bcn_buf->tim_ie_offset;
		bcn_cont_info->u2CsaIeOffset = bcn_buf->CsaIELocationInBeacon;
#ifdef DOT11_HE_AX
		bcn_cont_info->u2BccIeOffset = bcn_buf->bcc_ie_location;
#endif
		bcn_cont_info->u2BcnLength = cap->tx_hw_hdr_len + bcn_buf->FrameLen;
#ifdef CFG_BIG_ENDIAN
		bcn_cont_info->u2SubTag = cpu2le16(bcn_cont_info->u2SubTag);
		bcn_cont_info->u2Length = cpu2le16(bcn_cont_info->u2Length);
		bcn_cont_info->u2TimIeOffset = cpu2le16(bcn_cont_info->u2TimIeOffset);
		bcn_cont_info->u2CsaIeOffset = cpu2le16(bcn_cont_info->u2CsaIeOffset);
		bcn_cont_info->u2BccIeOffset = cpu2le16(bcn_cont_info->u2BccIeOffset);
		bcn_cont_info->u2BcnLength = cpu2le16(bcn_cont_info->u2BcnLength);
#endif

		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
				"BSS(%d), Len = %d, IE offset(TIM/CSA/BCC) = %d/%d/%d\n",
				bss_info->ucBssIndex,
				bcn_cont_info->u2BcnLength,
				bcn_cont_info->u2TimIeOffset,
				bcn_cont_info->u2CsaIeOffset,
				bcn_cont_info->u2BccIeOffset);
		hex_dump_with_cat_and_lvl("BCN", (char *)GET_OS_PKT_DATAPTR(bcn_buf->BeaconPkt),
				bcn_buf->FrameLen, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_DEBUG);
	}

	return u2SubTagLen;
}

#ifdef CONFIG_6G_SUPPORT

static UCHAR iob_offload_type_map[] = {
	OFFLOAD_TX_FILS_DISC, /*UNSOLICIT_TX_DISABLE, default FILS in 6G cert.*/
	OFFLOAD_TX_PROBE_RSP, /*UNSOLICIT_TX_PROBE_RSP*/
	OFFLOAD_TX_FILS_DISC, /*UNSOLICIT_TX_FILS_DISC*/
};

static UINT16 bss_update_unsolicited_offload_pkt(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	P_CMD_OFFLOAD_UNSOL_PKT_T unsol_pkt_info)
{
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
	pdiscov_iob dsc_iob = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	UINT16 u2SubTagLen = 0;
	UCHAR iob_type = 0;
	UCHAR qos_state = 0;
	UCHAR qos_tu = 0;

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
			"wdev is NULL\n");
		return u2SubTagLen;
	}

	dsc_iob = &wdev->ap6g_cfg.dsc_iob;
	iob_type = wlan_config_get_unsolicit_tx_type(wdev);
	qos_state = wlan_config_get_qos_tx_state(wdev);
	qos_tu = wlan_config_get_qos_tx_tu(wdev);

	unsol_pkt_info->u2SubTag			= SUB_TAG_UNSOL_OFFLOAD_PKT;
	unsol_pkt_info->ucTxMode			= PROBE_RSP_TX_MODE_SU;
	u2SubTagLen = sizeof(CMD_OFFLOAD_UNSOL_PKT_T);
	/*high priority to select qos injector due to less changed then unsolicit*/
	if (qos_state != wlan_operate_get_he_6g_qos_state(wdev) ||
		qos_tu != wlan_operate_get_he_6g_qos_tu(wdev)) {
		unsol_pkt_info->ucTxInterval		= qos_tu;
		unsol_pkt_info->ucTxType		= OFFLOAD_TX_QOS_NULL;
		unsol_pkt_info->u2ProbeRspLen	= 0;
		unsol_pkt_info->fgEnable = qos_state;
		wlan_operate_set_he_6g_qos_state(wdev, qos_state);
		wlan_operate_set_he_6g_qos_tu(wdev, qos_tu);
	} else {
		unsol_pkt_info->ucTxInterval		= wlan_config_get_unsolicit_tx_tu(wdev);
		if ((dsc_iob->pkt_buf) &&
			(iob_type < ARRAY_SIZE(iob_offload_type_map)) &&
			(iob_type != UNSOLICIT_TX_DISABLE)) {
			unsol_pkt_info->ucTxType		= iob_offload_type_map[iob_type];
			unsol_pkt_info->u2ProbeRspLen	= cap->tx_hw_hdr_len + dsc_iob->pkt_len;
			unsol_pkt_info->fgEnable = true;
		} else {
			unsol_pkt_info->ucTxType		= OFFLOAD_TX_FILS_DISC;
			unsol_pkt_info->u2ProbeRspLen	= 0;
			unsol_pkt_info->fgEnable = false;
		}
	}
	u2SubTagLen += unsol_pkt_info->u2ProbeRspLen;
	/* DW alignment */
	u2SubTagLen = (u2SubTagLen & 0x3) ? ((u2SubTagLen | 0x3) + 1) : u2SubTagLen;

	unsol_pkt_info->u2Length			= u2SubTagLen;

#ifdef CFG_BIG_ENDIAN
	unsol_pkt_info->u2SubTag = cpu2le16(unsol_pkt_info->u2SubTag);
	unsol_pkt_info->u2Length = cpu2le16(unsol_pkt_info->u2Length);
	unsol_pkt_info->u2ProbeRspLen = cpu2le16(unsol_pkt_info->u2ProbeRspLen);
#endif

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			"BSS(%d), TxType = %d, TxMode = %d, Interval = %d (TUs), Len = %d, enable: %d\n",
			bss_info->ucBssIndex,
			unsol_pkt_info->ucTxType,
			unsol_pkt_info->ucTxMode,
			unsol_pkt_info->ucTxInterval,
			unsol_pkt_info->u2ProbeRspLen,
			unsol_pkt_info->fgEnable);

	return u2SubTagLen;
}
#endif

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
static UINT16 bss_update_offload_bcn_btwt(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	P_CMD_OFFLOAD_BCN_BTWT_T bcn_btwt_info)
{
	struct BTWT_BUF_STRUCT *btwt = NULL;
	UINT16 u2SubTagLen = 0;

	btwt = &ad->ApCfg.btwt;
	if (btwt->btwt_element_exist && btwt->btwt_bcn_offset) {
		u2SubTagLen = sizeof(CMD_OFFLOAD_BCN_BTWT_T);

		bcn_btwt_info->u2SubTag = SUB_TAG_BCN_BTWT;
		bcn_btwt_info->u2Length = u2SubTagLen;
		bcn_btwt_info->u2bTWTIeOffset = btwt->btwt_bcn_offset;
#ifdef CFG_BIG_ENDIAN
		bcn_btwt_info->u2SubTag = cpu2le16(bcn_btwt_info->u2SubTag);
		bcn_btwt_info->u2Length = cpu2le16(bcn_btwt_info->u2Length);
#endif
		MTWF_DBG(NULL, DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG,
			"BSS(%d), bTWT offset = %d\n",
				bss_info->ucBssIndex, bcn_btwt_info->u2bTWTIeOffset);
	}

	return u2SubTagLen;
}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

static VOID bss_update_offload_pkt(
	struct _RTMP_ADAPTER *ad,
	struct _BSS_INFO_ARGUMENT_T *bss_info,
	struct cmd_msg *msg)
{
	struct wifi_dev *wdev = (struct wifi_dev *)bss_info->priv;
	PBCN_BUF_STRUCT bcn_buf = NULL;
	UINT16 u2SubTagLen[SUB_TAG_BCN_MAX_NUM] = {0};
	CMD_BSSINFO_OFFLOAD_PKT_T offload_pkt = {0};
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	CMD_OFFLOAD_BCN_CSA_T bcn_csa_info = {0};
	CMD_OFFLOAD_BCN_BCC_T bcn_bcc_info = {0};
	CMD_OFFLOAD_BCN_MBSSID_T bcn_mbss_info = {0};
	CMD_OFFLOAD_BCN_CONTENT_T bcn_cont_info = {0};
#ifdef CONFIG_6G_SUPPORT
	CMD_OFFLOAD_UNSOL_PKT_T unsol_pkt_info = {0};
#endif
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	CMD_OFFLOAD_BCN_BTWT_T bcn_btwt_info = {0};
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	if (!wdev) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
				 "wdev is NULL\n");
		return;
	}

	bcn_buf = &wdev->bcn_buf;

	os_zero_mem(&offload_pkt, sizeof(offload_pkt));
	/* Fill TLV format */
	offload_pkt.u2Tag = BSS_INFO_OFFLOAD_PKT;
	offload_pkt.u2Length = sizeof(CMD_BSSINFO_OFFLOAD_PKT_T);
	offload_pkt.ucVer = 0;
	offload_pkt.fgEnable = bss_info->bBcnSntReq;

	/* fill subTag - CSA */
	u2SubTagLen[SUB_TAG_BCN_CSA] = bss_update_offload_bcn_csa(ad, bss_info, &bcn_csa_info);
	if (u2SubTagLen[SUB_TAG_BCN_CSA]) {
		offload_pkt.u2Length += u2SubTagLen[SUB_TAG_BCN_CSA];
		offload_pkt.u2SubElementNum++;
	}

	/* fill subTag - BCC */
	u2SubTagLen[SUB_TAG_BCN_BCC] = bss_update_offload_bcn_bcc(ad, bss_info, &bcn_bcc_info);
	if (u2SubTagLen[SUB_TAG_BCN_BCC]) {
		offload_pkt.u2Length += u2SubTagLen[SUB_TAG_BCN_BCC];
		offload_pkt.u2SubElementNum++;
	}

	/* fill subTag - MBSSID */
	u2SubTagLen[SUB_TAG_BCN_MBSSID] = bss_update_offload_bcn_mbssid(ad, bss_info, &bcn_mbss_info);
	if (u2SubTagLen[SUB_TAG_BCN_MBSSID]) {
		offload_pkt.u2Length += u2SubTagLen[SUB_TAG_BCN_MBSSID];
		offload_pkt.u2SubElementNum++;
	}

	/* fill subTag - BCN content */
	u2SubTagLen[SUB_TAG_BCN_CONTENT] = bss_update_offload_bcn_content(ad, bss_info, &bcn_cont_info);
	if (u2SubTagLen[SUB_TAG_BCN_CONTENT]) {
		offload_pkt.u2Length += u2SubTagLen[SUB_TAG_BCN_CONTENT];
		offload_pkt.u2SubElementNum++;
	}

#ifdef CONFIG_6G_SUPPORT
	/* fill subTag - Unsolicited offload packet */
	u2SubTagLen[SUB_TAG_UNSOL_OFFLOAD_PKT] =
		bss_update_unsolicited_offload_pkt(ad, bss_info, &unsol_pkt_info);
	if (u2SubTagLen[SUB_TAG_UNSOL_OFFLOAD_PKT]) {
		offload_pkt.u2Length += u2SubTagLen[SUB_TAG_UNSOL_OFFLOAD_PKT];
		offload_pkt.u2SubElementNum++;
	}
#endif

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	/* fill subTag - BTWT */
	u2SubTagLen[SUB_TAG_BCN_BTWT] = bss_update_offload_bcn_btwt(ad, bss_info, &bcn_btwt_info);
	if (u2SubTagLen[SUB_TAG_BCN_BTWT]) {
		offload_pkt.u2Length += u2SubTagLen[SUB_TAG_BCN_BTWT];
		offload_pkt.u2SubElementNum++;
	}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "wdev(%d), UpdateReason %d, BcnEn %d/%d, FeuEn %d/%d/%d/%d/%d, Len = %d, SubElmNum = %d\n",
			  wdev->wdev_idx, bss_info->bUpdateReason,
			  bcn_buf->bBcnSntReq, offload_pkt.fgEnable,
			  u2SubTagLen[0], u2SubTagLen[1], u2SubTagLen[2], u2SubTagLen[3], u2SubTagLen[4],
			  offload_pkt.u2Length, offload_pkt.u2SubElementNum);

#ifdef CFG_BIG_ENDIAN
	offload_pkt.u2Tag = cpu2le16(offload_pkt.u2Tag);
	offload_pkt.u2Length = cpu2le16(offload_pkt.u2Length);
	offload_pkt.u2SubElementNum = cpu2le16(offload_pkt.u2SubElementNum);
#endif

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&offload_pkt,
					  sizeof(CMD_BSSINFO_OFFLOAD_PKT_T));

#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
	if (u2SubTagLen[SUB_TAG_BCN_BTWT]) {
		AndesAppendCmdMsg(msg, (char *)&bcn_btwt_info,
						  sizeof(CMD_OFFLOAD_BCN_BTWT_T));
	}
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

	if (u2SubTagLen[SUB_TAG_BCN_CSA]) {
		hex_dump_with_cat_and_lvl("CSA:", (char *)&bcn_csa_info, sizeof(bcn_csa_info),
						DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG);
		AndesAppendCmdMsg(msg, (char *)&bcn_csa_info,
						  sizeof(CMD_OFFLOAD_BCN_CSA_T));
	}

	if (u2SubTagLen[SUB_TAG_BCN_BCC]) {
		hex_dump_with_cat_and_lvl("BCC:", (char *)&bcn_bcc_info, sizeof(bcn_bcc_info),
						DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG);
		AndesAppendCmdMsg(msg, (char *)&bcn_bcc_info,
						  sizeof(CMD_OFFLOAD_BCN_BCC_T));
	}

	if (u2SubTagLen[SUB_TAG_BCN_MBSSID]) {
		hex_dump_with_cat_and_lvl("MBSS:", (char *)&bcn_mbss_info, sizeof(bcn_mbss_info),
						DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG);
		AndesAppendCmdMsg(msg, (char *)&bcn_mbss_info,
						  sizeof(CMD_OFFLOAD_BCN_MBSSID_T));
	}

	if (u2SubTagLen[SUB_TAG_BCN_CONTENT]) {
		hex_dump_with_cat_and_lvl("BCN_CONT:", (char *)&bcn_cont_info, sizeof(bcn_cont_info),
						DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG);
		AndesAppendCmdMsg(msg, (char *)&bcn_cont_info,
						  sizeof(CMD_OFFLOAD_BCN_CONTENT_T));

		hex_dump_with_cat_and_lvl("BCN_PKT:", (char *)GET_OS_PKT_DATAPTR(bcn_buf->BeaconPkt),
						(cap->tx_hw_hdr_len + bcn_buf->FrameLen),
						DBG_CAT_AP, CATAP_BCN, DBG_LVL_DEBUG);
		AndesAppendCmdMsg(msg, (char *)GET_OS_PKT_DATAPTR(bcn_buf->BeaconPkt),
						(u2SubTagLen[SUB_TAG_BCN_CONTENT] - sizeof(CMD_OFFLOAD_BCN_CONTENT_T)));
	}
#ifdef CONFIG_6G_SUPPORT
	if (u2SubTagLen[SUB_TAG_UNSOL_OFFLOAD_PKT]) {
		pdiscov_iob dsc_iob = &wdev->ap6g_cfg.dsc_iob;

		hex_dump_with_cat_and_lvl("OFFLOAD_PKT:", (char *)&unsol_pkt_info, sizeof(unsol_pkt_info),
						DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO);
		AndesAppendCmdMsg(msg, (char *)&unsol_pkt_info,
						  sizeof(CMD_OFFLOAD_UNSOL_PKT_T));

		if (unsol_pkt_info.fgEnable) {
			hex_dump_with_cat_and_lvl("OFFLOAD_PKT:", dsc_iob->pkt_buf,
							(cap->tx_hw_hdr_len + dsc_iob->pkt_len),
							DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO);
			AndesAppendCmdMsg(msg, dsc_iob->pkt_buf,
							  (u2SubTagLen[SUB_TAG_UNSOL_OFFLOAD_PKT] - sizeof(CMD_OFFLOAD_UNSOL_PKT_T)));
		}
	}
#endif
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11V_MBSSID_SUPPORT
static VOID bss_update_11v_mbssid(
		struct _RTMP_ADAPTER *pAd,
		struct _BSS_INFO_ARGUMENT_T *bss_info,
		struct cmd_msg *msg)
{
	CMD_BSSINFO_11V_MBSSID_T CmdBssInfo11vMbssid = {0};

	os_zero_mem(&CmdBssInfo11vMbssid, sizeof(CmdBssInfo11vMbssid));
	/* Fill TLV format */
	CmdBssInfo11vMbssid.u2Tag = BSS_INFO_11V_MBSSID;
	CmdBssInfo11vMbssid.u2Length = sizeof(CmdBssInfo11vMbssid);
	CmdBssInfo11vMbssid.ucMaxBSSIDIndicator = bss_info->max_bssid_indicator;
	CmdBssInfo11vMbssid.ucMBSSIDIndex = bss_info->mbssid_index;
#ifdef CFG_BIG_ENDIAN
	CmdBssInfo11vMbssid.u2Tag = cpu2le16(CmdBssInfo11vMbssid.u2Tag);
	CmdBssInfo11vMbssid.u2Length = cpu2le16(CmdBssInfo11vMbssid.u2Length);
#endif

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
			 "ucMaxBSSIDIndicator(%d),ucMBSSIDIndex(%d)\n",
			 CmdBssInfo11vMbssid.ucMaxBSSIDIndicator,
			 CmdBssInfo11vMbssid.ucMBSSIDIndex);

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfo11vMbssid, sizeof(CMD_BSSINFO_11V_MBSSID_T));
}
#endif

#ifdef BCN_PROTECTION_SUPPORT
static VOID bss_update_bcn_prot(
		struct _RTMP_ADAPTER *ad,
		struct _BSS_INFO_ARGUMENT_T *bss_info,
		struct cmd_msg *msg)
{
	CMD_BSSINFO_BCN_PROT_T CmdBssInfoBcnProt = {0};
	struct bcn_protection_cfg *bcn_prot_cfg = &bss_info->bcn_prot_cfg;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(ad->hdev_ctrl);
	UCHAR table_idx = get_bigtk_table_idx(bcn_prot_cfg);

	os_zero_mem(&CmdBssInfoBcnProt, sizeof(CmdBssInfoBcnProt));
	/* Fill TLV format */
	CmdBssInfoBcnProt.u2Tag = BSS_INFO_BCN_PROT;
	CmdBssInfoBcnProt.u2Length = sizeof(CmdBssInfoBcnProt);
	CmdBssInfoBcnProt.ucBcnProtEnabled = (bcn_prot_cfg->bcn_prot_en) ? chip_cap->bcn_prot_sup : BCN_PROT_EN_OFF;
	CmdBssInfoBcnProt.ucBcnProtKeyId = bcn_prot_cfg->bigtk_key_idx;
	if (IS_CIPHER_BIP_CMAC128(bcn_prot_cfg->bigtk_cipher))
		CmdBssInfoBcnProt.ucBcnProtCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
	else if (IS_CIPHER_BIP_CMAC256(bcn_prot_cfg->bigtk_cipher))
		CmdBssInfoBcnProt.ucBcnProtCipherId = SEC_CIPHER_ID_BIP_CMAC_256;
	else {
		MTWF_DBG(ad, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_ERROR,
					 "not support bigtk cipher (0x%x), use default bip-cmac-128\n",
					 bcn_prot_cfg->bigtk_cipher);
		CmdBssInfoBcnProt.ucBcnProtCipherId = SEC_CIPHER_ID_BIP_CMAC_128;
	}
	os_move_mem(&CmdBssInfoBcnProt.aucBcnProtKey, &bcn_prot_cfg->bigtk[table_idx][0], LEN_MAX_BIGTK);
	os_move_mem(&CmdBssInfoBcnProt.aucBcnProtPN, &bcn_prot_cfg->bipn[table_idx][0], LEN_WPA_TSC);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
					 "ucBcnProtEnabled (%d), ucBcnProtCipherId(%d), ucBcnProtKeyId(%d)\n",
					 CmdBssInfoBcnProt.ucBcnProtEnabled,
					 CmdBssInfoBcnProt.ucBcnProtCipherId, CmdBssInfoBcnProt.ucBcnProtKeyId);
	hex_dump_with_cat_and_lvl("aucBcnProtKey:", CmdBssInfoBcnProt.aucBcnProtKey,
						LEN_MAX_BIGTK, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);
	hex_dump_with_cat_and_lvl("aucBcnProtPN:", CmdBssInfoBcnProt.aucBcnProtPN,
						LEN_WPA_TSC, DBG_CAT_SEC, CATSEC_BCNPROT, DBG_LVL_DEBUG);

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoBcnProt, sizeof(CMD_BSSINFO_BCN_PROT_T));
}
#endif

static VOID bss_update_mld(
		struct _RTMP_ADAPTER *ad,
		struct _BSS_INFO_ARGUMENT_T *bss_info,
		struct cmd_msg *msg)
{
	struct wifi_dev *wdev = NULL;
	struct bss_mld mld = {0};
	UCHAR mld_addr[MAC_ADDR_LEN] = {0};
	UINT8 mld_group_idx = 0;
	struct CMD_BSSINFO_MLD_T CmdBssInfoMld = {0};

	wdev = (struct wifi_dev *)bss_info->priv;
	if (!wdev)
		return;

#ifdef DOT11_EHT_BE
	mld_group_idx = bss_info->mld_info.mld_group_idx;
	wlan_operate_get_mld_addr(wdev, mld_addr);
#else
	mld_group_idx = MLD_GROUP_NONE;
#endif
	HcGetBssMld(wdev, &mld);

	/* Fill TLV format */
	CmdBssInfoMld.u2Tag = BSS_INFO_MLD;
	CmdBssInfoMld.u2Length = sizeof(CmdBssInfoMld);
	if (mld_group_idx == MLD_GROUP_NONE) {
		CmdBssInfoMld.ucGroupMldId = 0xff;
		CmdBssInfoMld.ucOwnMldId = mld.mld_addr_idx;
		CmdBssInfoMld.ucOmRemapIdx = OM_REMAP_IDX_NONE;
		NdisMoveMemory(CmdBssInfoMld.aucOwnMldAddr, mld_addr, MAC_ADDR_LEN);
	} else {
		CmdBssInfoMld.ucGroupMldId = mld.mld_group_addr_idx;
		CmdBssInfoMld.ucOwnMldId = mld.mld_addr_idx;
		CmdBssInfoMld.ucOmRemapIdx = mld.mld_remap_idx;
		NdisMoveMemory(CmdBssInfoMld.aucOwnMldAddr, mld_addr, MAC_ADDR_LEN);
	}

	MTWF_PRINT("%s:mld_group_idx=%d,T=%d,L=%d,GrpMldId=%d,MldIdx=%d,RemaIdx=%d,MldAddr=%pM\n",
		__func__,
		mld_group_idx,
		CmdBssInfoMld.u2Tag,
		CmdBssInfoMld.u2Length,
		CmdBssInfoMld.ucGroupMldId,
		CmdBssInfoMld.ucOwnMldId,
		CmdBssInfoMld.ucOmRemapIdx,
		CmdBssInfoMld.aucOwnMldAddr);

	/* Append this feature */
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoMld, sizeof(CmdBssInfoMld));
}

/* BSSinfo tag handle */
static BSS_INFO_HANDLE_T apfBssInfoTagHandle[BSS_INFO_MAX_NUM] = {
	{BSS_INFO_OWN_MAC_FEATURE,          bssConnectOwnDev},
	{BSS_INFO_BASIC_FEATURE,            bssUpdateBssInfoBasic},
	{BSS_INFO_RF_CH_FEATURE,            bssUpdateChannel},
	{BSS_INFO_PM_FEATURE,               pmUpdateBssInfoPM},
	{BSS_INFO_UAPSD_FEATURE,            pmUpdateBssUapsd},
	{BSS_INFO_ROAM_DETECTION_FEATURE,   bssUpdateRssiRmDetParams},
	{BSS_INFO_LQ_RM_FEATURE,            NULL},
	{BSS_INFO_EXT_BSS_FEATURE,          bssUpdateExtBssInfo},
	{BSS_INFO_BROADCAST_INFO_FEATURE,   bssUpdateBmcMngRate},
	{BSS_INFO_SYNC_MODE_FEATURE,        bssUpdateSyncModeCtrl},
	{BSS_INFO_RA_FEATURE,               bssUpdateRA},
#ifdef HW_TX_AMSDU_SUPPORT
	{BSS_INFO_HW_AMSDU_FEATURE, bss_update_hw_amsdu},
#else
	{BSS_INFO_HW_AMSDU_FEATURE, NULL},
#endif
#ifdef DOT11_HE_AX
	{BSS_INFO_BSS_COLOR_FEATURE, bss_update_bss_color},
	{BSS_INFO_HE_BASIC_FEATURE, bss_update_basic_he},
#else
	{BSS_INFO_BSS_COLOR_FEATURE, NULL},
	{BSS_INFO_HE_BASIC_FEATURE, NULL},
#endif /*DOT11_HE_AX*/
	{BSS_INFO_PROTECT_INFO_FEATURE, bss_update_prot_info},
#ifdef CONFIG_AP_SUPPORT
	{BSS_INFO_OFFLOAD_PKT_FEATURE, bss_update_offload_pkt},
#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11V_MBSSID_SUPPORT
	{BSS_INFO_11V_MBSSID_FEATURE, bss_update_11v_mbssid},
#else
	{BSS_INFO_11V_MBSSID_FEATURE, NULL},
#endif
#ifdef BCN_PROTECTION_SUPPORT
	{BSS_INFO_BCN_PROT_FEATURE, bss_update_bcn_prot},
#else
	{BSS_INFO_BCN_PROT_FEATURE, NULL},
#endif
	{BSS_INFO_MLD_FEATURE, bss_update_mld},
	{BSS_INFO_MAX_NUM_FEATURE, NULL},
};

INT32 CmdSetSyncModeByBssInfoUpdate(
	RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info)
{
	struct cmd_msg          *msg = NULL;
	CMD_BSSINFO_UPDATE_T    CmdBssInfoUpdate = {0};
	INT32                   Ret = 0;
	UINT16                   ucTLVNumber = 1;
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, MAX_BUF_SIZE_OF_BSS_INFO + 100);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BSSINFO_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtBssInfoUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	/* Tag = 0, Fill WLAN related header here */
	CmdBssInfoUpdate.ucBssIndex = bss_info->OwnMacIdx;
	CmdBssInfoUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdBssInfoUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoUpdate,
					  sizeof(CMD_BSSINFO_UPDATE_T));
	bssUpdateSyncModeCtrl(pAd, bss_info, msg);
	/* Send out CMD */
	call_fw_cmd_notifieriers(WO_CMD_BSS_INFO, pAd, msg->net_pkt);
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdExtBssInfoUpdate(
	RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T *bss_info)
{
	struct cmd_msg          *msg = NULL;
	CMD_BSSINFO_UPDATE_T    CmdBssInfoUpdate = {0};
	INT32                   Ret = 0;
	UINT8                   i = 0;
	UINT16                  ucTLVNumber = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	UINT16					u2MsgLen = MAX_BUF_SIZE_OF_BSS_INFO;

	/* enlarge cmd size for BSS_INFO_BCN */
	if (bss_info->u8BssInfoFeature & BSS_INFO_OFFLOAD_PKT_FEATURE)
		u2MsgLen = MAX_BUF_SIZE_OF_BSSINFO_OFFLOAD_PKT;

	msg = AndesAllocCmdMsg(pAd, u2MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Get number of TLV*/
	for (i = 0; i < BSS_INFO_MAX_NUM; i++) {
		if (bss_info->u8BssInfoFeature & (1 << i))
			ucTLVNumber++;
	}

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_INFO,
		"OwnMacIdx = %d, Band = %d, BssIndex = %d "MACSTR"), TLV Num = %d\n",
		bss_info->OwnMacIdx, bss_info->ucBandIdx, bss_info->ucBssIndex,
		MAC2STR(bss_info->Bssid), ucTLVNumber);

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, sizeof(EVENT_BSSINFO_UPDATE_T));
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtBssInfoUpdateRsp);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_BSSINFO_UPDATE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_LEN_VAR_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	/* Tag = 0, Fill WLAN related header here */
	CmdBssInfoUpdate.ucBssIndex = bss_info->ucBssIndex;
	CmdBssInfoUpdate.u2TotalElementNum = cpu2le16(ucTLVNumber);
	CmdBssInfoUpdate.ucAppendCmdTLV = TRUE;
	AndesAppendCmdMsg(msg, (char *)&CmdBssInfoUpdate, sizeof(CMD_BSSINFO_UPDATE_T));

	for (i = 0; i < BSS_INFO_MAX_NUM; i++) {
		if (bss_info->u8BssInfoFeature & apfBssInfoTagHandle[i].BssInfoTag) {
			if (apfBssInfoTagHandle[i].BssInfoTagHandler != NULL) {
				apfBssInfoTagHandle[i].BssInfoTagHandler(
					pAd,
					bss_info,
					msg);
			} else {
				MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
						 " BssInfoTag = %lld no corresponding function handler.\n",
						 apfBssInfoTagHandle[i].BssInfoTag);
			}
		}
	}

	/* Send out CMD */
	call_fw_cmd_notifieriers(WO_CMD_BSS_INFO, pAd, msg->net_pkt);
	Ret = AndesSendCmdMsg(pAd, msg);

error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_BSSINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdExtSetTmrCR(
	struct _RTMP_ADAPTER *pAd,
	UCHAR enable,
	UCHAR BandIdx)
{
	struct cmd_msg          *msg = NULL;
	CMD_TMR_CTRL_T    CmdTmrCtrl = {0};
	TMR_CTRL_SET_TMR_EN_T TmrCtrlSetTmr = {0};
	struct _CMD_ATTRIBUTE attr = {0};
	INT32                   Ret = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_TMR_CTRL_T) + sizeof(TMR_CTRL_SET_TMR_EN_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_TMR_CTRL);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	CmdTmrCtrl.ucTmrCtrlType = SET_TMR_ENABLE;

	if (pAd->pTmrCtrlStruct == NULL) {
		CmdTmrCtrl.ucTmrThroughold = ERROR_DEFAULT_DBM;
		CmdTmrCtrl.ucTmrIter = TOAE_FSM_ITERATION;
	} else {
		CmdTmrCtrl.ucTmrThroughold = pAd->pTmrCtrlStruct->TmrThroughold;
		CmdTmrCtrl.ucTmrIter = pAd->pTmrCtrlStruct->TmrIter;
	}
	CmdTmrCtrl.ucTmrVer = cap->TmrHwVer;
	AndesAppendCmdMsg(msg, (char *)&CmdTmrCtrl, sizeof(CMD_TMR_CTRL_T));

	if (enable) {
		TmrCtrlSetTmr.ucEnable = TRUE;

		if (enable == TMR_INITIATOR)
			TmrCtrlSetTmr.ucRole = 0;
		else if (enable == TMR_RESPONDER) {
			TmrCtrlSetTmr.ucRole = 1;
			if (CmdTmrCtrl.ucTmrVer == TMR_VER_2_0)
				TmrCtrlSetTmr.ucCatEnable = BIT(1); /* bit 1 to enable filter FTM packet */
			/* field 1 is mapping to ucCatEnable bit 1 */
			TmrCtrlSetTmr.aucType_Subtype[0] = FC_TYPE_RSVED;
			TmrCtrlSetTmr.aucType_Subtype[1] = FC_TYPE_MGMT | (SUBTYPE_ACTION << TMR_PA_SUBTYPE_OFST);
			TmrCtrlSetTmr.aucType_Subtype[2] = FC_TYPE_RSVED;
			TmrCtrlSetTmr.aucType_Subtype[3] = FC_TYPE_RSVED;
		}
	} else
		TmrCtrlSetTmr.ucEnable = FALSE;

	TmrCtrlSetTmr.ucDbdcIdx = BandIdx;
	AndesAppendCmdMsg(msg, (char *)&TmrCtrlSetTmr,
					  sizeof(TMR_CTRL_SET_TMR_EN_T));
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

static VOID CmdRxHdrTransUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
#ifdef CFG_BIG_ENDIAN
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
#endif

	if (EventExtCmdResult->u4Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "BUG::EventExtCmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->u4Status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "EventExtCmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->u4Status);
	}
}


INT32 CmdRxHdrTransUpdate(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid,
						  BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP)
{
	struct cmd_msg			*msg = NULL;
	INT32					Ret = 0;
	EXT_RX_HEADER_TRANSLATE_T	ExtRxHdrTrans = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd,  sizeof(EXT_RX_HEADER_TRANSLATE_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_HDR_TRANS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdRxHdrTransUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	ExtRxHdrTrans.ucOperation = RXHDR_TRANS;
	ExtRxHdrTrans.ucEnable = En;
	ExtRxHdrTrans.ucCheckBssid = ChkBssid;
	ExtRxHdrTrans.ucInsertVlan = InSVlan;
	ExtRxHdrTrans.ucRemoveVlan = RmVlan;
	ExtRxHdrTrans.ucUserQosTid = !SwPcP;
	ExtRxHdrTrans.ucTranslationMode = 0;
	AndesAppendCmdMsg(msg, (char *)&ExtRxHdrTrans,
					  sizeof(EXT_RX_HEADER_TRANSLATE_T));
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}


static VOID CmdRxHdrTransBLUpdateRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "EventExtCmdResult.ucExTenCID = 0x%x\n",
			  EventExtCmdResult->ucExTenCID);
#ifdef CFG_BIG_ENDIAN
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
#endif

	if (EventExtCmdResult->u4Status != 0) {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "BUG::EventExtCmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->u4Status);
	} else {
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "EventExtCmdResult.u4Status = 0x%x\n",
				  EventExtCmdResult->u4Status);
	}
}


INT32 CmdAutoBATrigger(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_AUTO_BA_T ExtAutoBa = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_AUTO_BA_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_AUTO_BA);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	ExtAutoBa.ucAutoBaEnable = Enable;
	ExtAutoBa.ucTarget = 0;
	ExtAutoBa.u4Timeout = cpu2le32(Timeout);
	AndesAppendCmdMsg(msg, (char *)&ExtAutoBa,
					  sizeof(EXT_CMD_ID_AUTO_BA_T));
	Ret = AndesSendCmdMsg(pAd, msg);
Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

#ifdef IGMP_SNOOP_SUPPORT
INT32 CmdMcastCloneEnable(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT8 band_idx, UINT8 omac_idx)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MCAST_CLONE_T ExtMcastClone = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MCAST_CLONE_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MCAST_CLONE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	ExtMcastClone.uc_omac_idx = omac_idx;
	ExtMcastClone.ucMcastCloneEnable = Enable;
	ExtMcastClone.uc_band_idx = band_idx;
#ifndef IGMP_SNOOPING_NON_OFFLOAD
	/* TODO: support via WiFi Profile? */
	ExtMcastClone.uc_mcast_clone_cnt = 32;
#endif

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"omac_idx=%d, en=%d\n",
		omac_idx, Enable);

	AndesAppendCmdMsg(msg, (char *)&ExtMcastClone,
					  sizeof(EXT_CMD_ID_MCAST_CLONE_T));
	Ret = AndesSendCmdMsg(pAd, msg);
Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdMcastAllowNonMemberEnable(RTMP_ADAPTER *pAd, UINT8 Msg_type, BOOLEAN Enable)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MCAST_POLICY_T ExtMcast = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MCAST_POLICY_T));

	if (!msg) {
			Ret = NDIS_STATUS_RESOURCES;
			goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_IGMP_CMD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	ExtMcast.uIgmpType = Msg_type;
	ExtMcast.uMcastPolicy = Enable;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"en=%d\n", Enable);

	AndesAppendCmdMsg(msg, (char *)&ExtMcast,
		sizeof(EXT_CMD_ID_MCAST_POLICY_T));
	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"(Ret = %d)\n", Ret);
	return Ret;
}

BOOLEAN CmdMcastEntryInsert(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T ExtMcastEntryInsert;
    struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&ExtMcastEntryInsert, sizeof(EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MULTICAST_ENTRY_INSERT);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);

	if (GrpAddr) {
		NdisMoveMemory(&ExtMcastEntryInsert.aucGroupId[0], (UCHAR *)GrpAddr, IPV6_ADDR_LEN);
		ExtMcastEntryInsert.ucBssInfoIdx = BssIdx;
		ExtMcastEntryInsert.ucMcastEntryType = Type;
	} else {
		Ret = NDIS_STATUS_FAILURE;
		goto Error0;
	}

	if (MemberAddr) {
		NdisMoveMemory(&ExtMcastEntryInsert.aucMemberAddr[0], (UCHAR *)MemberAddr, MAC_ADDR_LEN);
		ExtMcastEntryInsert.ucMemberNum = 1;
		ExtMcastEntryInsert.u2Wcid = wcid;
#ifdef CFG_BIG_ENDIAN
		ExtMcastEntryInsert.u2Wcid = cpu2le16(ExtMcastEntryInsert.u2Wcid);
#endif
	}

	AndesAppendCmdMsg(msg, (char *)&ExtMcastEntryInsert,
	    sizeof(EXT_CMD_ID_MULTICAST_ENTRY_INSERT_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}


BOOLEAN CmdMcastEntryDelete(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev, UINT16 wcid)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T ExtMcastEntryDelete;

    struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&ExtMcastEntryDelete, sizeof(EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MULTICAST_ENTRY_DELETE);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);

	if (GrpAddr) {
		NdisMoveMemory(&ExtMcastEntryDelete.aucGroupId[0], (UCHAR *)GrpAddr, IPV6_ADDR_LEN);
		ExtMcastEntryDelete.ucBssInfoIdx = BssIdx;
	} else {
		Ret = NDIS_STATUS_FAILURE;
		goto Error0;
	}

	if (MemberAddr) {
		ExtMcastEntryDelete.ucMemberNum = 1;
		NdisMoveMemory(&ExtMcastEntryDelete.aucMemberAddr[0], (UCHAR *)MemberAddr, MAC_ADDR_LEN);
		ExtMcastEntryDelete.u2Wcid = wcid;
#ifdef CFG_BIG_ENDIAN
		ExtMcastEntryDelete.u2Wcid = cpu2le16(ExtMcastEntryDelete.u2Wcid);
#endif

	}

	AndesAppendCmdMsg(msg, (char *)&ExtMcastEntryDelete,
	    sizeof(EXT_CMD_ID_MULTICAST_ENTRY_DELETE_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

#ifdef IGMP_SNOOPING_DENY_LIST
BOOLEAN CmdMcastEntryDenyList(struct _RTMP_ADAPTER *pAd, UINT8 BssIdx, UINT8 ucEntryCount, UINT8 ucAddToList, UINT_8 *pAddr, UINT_8 *Prefix_list)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MULTICAST_SN_DENY_LIST_T ExtMcastSnDenyList;
	struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&ExtMcastSnDenyList, sizeof(EXT_CMD_ID_MULTICAST_SN_DENY_LIST_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MULTICAST_SN_DENY_LIST_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_MULTICAST_SN_DENY_LIST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);

	ExtMcastSnDenyList.ucBssInfoIdx = BssIdx;
	ExtMcastSnDenyList.ucEntryCount = ucEntryCount;
	ExtMcastSnDenyList.ucAddToList = ucAddToList;

	os_zero_mem(&ExtMcastSnDenyList.deny_list[0][0], IPV6_ADDR_LEN*IGMP_DENY_TABLE_SIZE_MAX);
	if (ucEntryCount <= IGMP_DENY_TABLE_SIZE_MAX)
		NdisMoveMemory(&ExtMcastSnDenyList.deny_list[0][0], (UCHAR *)pAddr, (IPV6_ADDR_LEN * ucEntryCount));

	os_zero_mem(&ExtMcastSnDenyList.Prefix_list[0], IGMP_DENY_TABLE_SIZE_MAX);
	if (ucEntryCount <= IGMP_DENY_TABLE_SIZE_MAX)
		NdisMoveMemory(&ExtMcastSnDenyList.Prefix_list[0], (UCHAR *)Prefix_list, ucEntryCount);

	AndesAppendCmdMsg(msg, (char *)&ExtMcastSnDenyList,
	    sizeof(EXT_CMD_ID_MULTICAST_SN_DENY_LIST_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}
#endif


INT32 CmdMcastFloodingCIDR(struct _RTMP_ADAPTER *pAd, UCHAR EntryIPType, BOOLEAN bInsert, PUCHAR MacData, PUINT32 PrefixMask)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_IGMP_FLOODING_CMD_T ExtMcastFlooding;
	struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&ExtMcastFlooding, sizeof(EXT_CMD_ID_IGMP_FLOODING_CMD_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_IGMP_FLOODING_CMD_T));

	if (! msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_IGMP_FLOODING_CMD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);

	if (MacData) {
		NdisMoveMemory(&ExtMcastFlooding.auMacData[0], (UCHAR *)MacData, 6);
		ExtMcastFlooding.uEntryIPType=EntryIPType;
		ExtMcastFlooding.bInsert= bInsert;
		NdisMoveMemory(&ExtMcastFlooding.auPrefixMask[0], (UCHAR *)PrefixMask, sizeof(UINT32));
	} else {
		Ret = NDIS_STATUS_FAILURE;
		goto Error0;
	}

	AndesAppendCmdMsg(msg, (char *)&ExtMcastFlooding, sizeof(EXT_CMD_ID_IGMP_FLOODING_CMD_T));
	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

#ifdef IGMP_SNOOPING_OFFLOAD
INT32 CmdMcastAllowMwdsCloneEnable(RTMP_ADAPTER *pAd, UINT8 Msg_type, BOOLEAN Enable)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_MCAST_MWDS_CLONE_POLICY_T ExtMcast = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_MCAST_MWDS_CLONE_POLICY_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_IGMP_MWDS_CLONE_CMD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	ExtMcast.uIgmpType = Msg_type;
	ExtMcast.uMcastMwdsClonePolicy = Enable;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"en=%d\n", Enable);

	AndesAppendCmdMsg(msg, (char *)&ExtMcast,
		sizeof(EXT_CMD_ID_MCAST_MWDS_CLONE_POLICY_T));
	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdMcastPerBandSize(struct _RTMP_ADAPTER *pAd, UCHAR enable, UINT band0_cfgNum, UINT band1_cfgNum, UINT band2_cfgNum)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_IGMP_PER_BAND_SIZE_CMD_T ExtMcastPerBandSize;
	struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&ExtMcastPerBandSize, sizeof(EXT_CMD_ID_IGMP_PER_BAND_SIZE_CMD_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_IGMP_PER_BAND_SIZE_CMD_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_IGMP_PER_BAND_SIZE_CMD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);

	ExtMcastPerBandSize.uCfgNum[0] = band0_cfgNum;
	ExtMcastPerBandSize.uCfgNum[1] = band1_cfgNum;
	ExtMcastPerBandSize.uCfgNum[2] = band2_cfgNum;
	ExtMcastPerBandSize.enable = enable;

	AndesAppendCmdMsg(msg, (char *)&ExtMcastPerBandSize, sizeof(EXT_CMD_ID_IGMP_PER_BAND_SIZE_CMD_T));
	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}
#endif


#ifdef IGMP_TVM_SUPPORT
BOOLEAN CmdSetMcastEntryAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 ucOwnMacIdx)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T ExtMcastSetAgeOut = {0};

    struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&ExtMcastSetAgeOut, sizeof(EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_IGMP_MULTICAST_SET_GET);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);

	AndesInitCmdMsg(msg, attr);

	ExtMcastSetAgeOut.ucCmdType = IGMP_MCAST_SET_AGEOUT_TIME;
	ExtMcastSetAgeOut.ucOwnMacIdx = ucOwnMacIdx;
	ExtMcastSetAgeOut.SetData.u4AgeOutTime = AgeOutTime;
	AndesAppendCmdMsg(msg, (char *)&ExtMcastSetAgeOut,
		sizeof(EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

BOOLEAN CmdGetMcastEntryTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev)
{
	struct cmd_msg *msg = NULL;
	INT32 Ret = 0;
	EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T ExtMcastGetEntryTable = {0};

    struct _CMD_ATTRIBUTE attr = {0};

	os_zero_mem(&ExtMcastGetEntryTable, sizeof(EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2CR4);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_IGMP_MULTICAST_SET_GET);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, MT_IGNORE_PAYLOAD_LEN_CHECK);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, wdev);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtEventIgmpMcastTableRsp);

	AndesInitCmdMsg(msg, attr);

	ExtMcastGetEntryTable.ucCmdType = IGMP_MCAST_GET_ENTRY_TABLE;
	ExtMcastGetEntryTable.ucOwnMacIdx = ucOwnMacIdx;
	AndesAppendCmdMsg(msg, (char *)&ExtMcastGetEntryTable,
		sizeof(EXT_CMD_ID_IGMP_MULTICAST_SET_GET_T));

	Ret = AndesSendCmdMsg(pAd, msg);

Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

VOID CmdExtEventIgmpMcastTableRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct wifi_dev *wdev = NULL;
	struct _RTMP_ADAPTER *pAd = NULL;

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "\n");

	wdev = (struct wifi_dev *)msg->attr.rsp.wb_buf_in_calbk;

	if (wdev) {
		pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		IgmpSnoopingShowMulticastTable(pAd, wdev);
	}
}

#endif /* IGMP_TVM_SUPPORT */

#endif

INT32 CmdRxHdrTransBLUpdate(RTMP_ADAPTER *pAd, UINT8 Index, UINT8 En, UINT16 EthType)
{
	struct cmd_msg			*msg = NULL;
	INT32					Ret = 0;
	EXT_RX_HEADER_TRANSLATE_BL_T	ExtRxHdrTransBL = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	msg = AndesAllocCmdMsg(pAd,  sizeof(EXT_RX_HEADER_TRANSLATE_BL_T));

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto Error0;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_RX_HDR_TRANS);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdRxHdrTransBLUpdateRsp);
	AndesInitCmdMsg(msg, attr);
	ExtRxHdrTransBL.ucOperation = RXHDR_BL;
	ExtRxHdrTransBL.ucCount = 1;
	ExtRxHdrTransBL.ucBlackListIndex = Index;
	ExtRxHdrTransBL.ucEnable = En;
	ExtRxHdrTransBL.usEtherType = cpu2le16(EthType);
	AndesAppendCmdMsg(msg, (char *)&ExtRxHdrTransBL,
					  sizeof(EXT_RX_HEADER_TRANSLATE_BL_T));
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
Error0:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}


static VOID CmdExtGeneralTestRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO, "General Test status=%d\n", Status);

	switch (Status) {
	case TARGET_ADDRESS_LEN_SUCCESS:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
				 "General Test success\n");
		break;

	default:
		MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "General Test success Unknow Status(%d)\n",
				  Status);
		break;
	}
}

#ifdef ERR_RECOVERY

INT32 CmdExtGeneralTestOn(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN enable)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_GENERAL_TEST_T CmdGeneralTest = {0};
	UINT32 MsgLen;
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MsgLen = sizeof(CMD_STAREC_UPDATE_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GENERAL_TEST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtGeneralTestRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill command related header here*/
	CmdGeneralTest.ucCategory = GENERAL_TEST_CATEGORY_SIM_ERROR_DETECTION;
	CmdGeneralTest.ucAction = GENERAL_TEST_ACTION_SWITCH_ON_OFF;
	CmdGeneralTest.Data.rGeneralTestSimErrorSwitchOnOff.ucSwitchMode = enable;
	AndesAppendCmdMsg(msg, (char *)&CmdGeneralTest, sizeof(EXT_CMD_GENERAL_TEST_T));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "CmdExtGeneralTest:\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "ucCategory=%d, ucAction=%d, ucSwitchMode=%d\n",
			  CmdGeneralTest.ucCategory, CmdGeneralTest.ucAction,
			  CmdGeneralTest.Data.rGeneralTestSimErrorSwitchOnOff.ucSwitchMode);
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdExtGeneralTestMode(
	struct _RTMP_ADAPTER *pAd,
	UINT8 mode,
	UINT8 submode)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_GENERAL_TEST_T CmdGeneralTest = {0};
	UINT32 MsgLen;
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MsgLen = sizeof(CMD_STAREC_UPDATE_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GENERAL_TEST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtGeneralTestRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill command related header here*/
	CmdGeneralTest.ucCategory = GENERAL_TEST_CATEGORY_SIM_ERROR_DETECTION;
	CmdGeneralTest.ucAction = GENERAL_TEST_ACTION_RECOVERY;
	CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucModule = mode;
	CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucSubModule = submode;
	AndesAppendCmdMsg(msg, (char *)&CmdGeneralTest, sizeof(EXT_CMD_GENERAL_TEST_T));
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "CmdExtGeneralTest:\n");
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 "ucCategory=%d, ucAction=%d, ucModule=%d ucSubModule=%d\n",
			  CmdGeneralTest.ucCategory, CmdGeneralTest.ucAction,
			  CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucModule,
			  CmdGeneralTest.Data.rGeneralTestSimErrDetRecovery.ucSubModule);
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "(Ret = %d)\n", Ret);
	return Ret;
}
#endif /* ERR_RECOVERY */

INT32 CmdExtGeneralTestAPPWS(
	struct _RTMP_ADAPTER *pAd,
	UINT action)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_GENERAL_TEST_T CmdGeneralTest = {0};
	UINT32 MsgLen;
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MsgLen = sizeof(EXT_CMD_GENERAL_TEST_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_GENERAL_TEST);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtGeneralTestRsp);
	AndesInitCmdMsg(msg, attr);
	/* Fill command related header here*/
	CmdGeneralTest.ucCategory = GENERAL_TEST_CATEGORY_APPWS;
	CmdGeneralTest.ucAction = action;
	AndesAppendCmdMsg(msg, (char *)&CmdGeneralTest, sizeof(EXT_CMD_GENERAL_TEST_T));
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 " ucCategory=%d, ucAction=%d\n",
			  CmdGeneralTest.ucCategory, CmdGeneralTest.ucAction);
	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdExtSER(
	struct _RTMP_ADAPTER *pAd,
	UINT_8  action,
	UINT8	ser_set,
	UINT8	dbdc_idx
	)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_SER_T cmdSER = {0};
	UINT32 MsgLen;
	INT32 Ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};

	MsgLen = sizeof(EXT_CMD_SER_T);
	msg = AndesAllocCmdMsg(pAd, MsgLen);

	if (!msg) {
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_SER);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	/* Fill command related header here*/
	cmdSER.action = action;
	cmdSER.ser_set = ser_set;
	cmdSER.ucDbdcIdx = dbdc_idx;
	AndesAppendCmdMsg(msg, (char *)&cmdSER, sizeof(EXT_CMD_SER_T));

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
			 " action=%d ser_set=%d\n",
			  cmdSER.action, cmdSER.ser_set);

	/* Send out CMD */
	Ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG, "(Ret = %d)\n", Ret);
	return Ret;
}

INT32 CmdExtCmdCfgUpdate(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	ENUM_CFG_FEATURE eFeature,
	VOID *param)
{
	struct cmd_msg          *msg = NULL;
	struct _CMD_ATTRIBUTE attr = {0};
	EXT_CMD_CFG_BASIC_INFO_T rCfgBasicInfo;
	UINT8 i, ucTLVNumber = 0, ucAction = 0;
	INT32 Ret = 0;
	UINT16 u2MsgLen = sizeof(EXT_CMD_CFG_BASIC_INFO_T);

	memset(&rCfgBasicInfo, 0, sizeof(EXT_CMD_CFG_BASIC_INFO_T));
	/* Get number of TLV*/
	for (i = 0; i < EXT_CMD_CFG_MAX_NUM; i++) {
		if (eFeature & (1 << i)) {
			if (i == EXT_CMD_CFGINFO_HOSTREPORT_TX_LATENCY)
				u2MsgLen += sizeof(EXT_CMD_CFG_HOSTREPORT_UPDATE_T);

			if (i == EXT_CMD_CFGINFO_RX_FILTER_DROP_CTRL_FRAME)
				u2MsgLen += sizeof(EXT_CMD_CFG_DROP_CTRL_FRAME_T);

			if (i == EXT_CMD_CFGINFO_AGG_AC_LIMIT)
				u2MsgLen += sizeof(EXT_CMD_CFG_SET_AGG_AC_LIMIT_T);

			if (i == EXT_CMD_CFGINFO_CERT_CFG)
				u2MsgLen += sizeof(EXT_CMD_CFG_CERT_CFG_T);

			if (i == EXT_CMD_CFGINFO_ACK_CTS)
				u2MsgLen += sizeof(EXT_CMD_CFG_SET_ACK_CTS_T);

			if (i == EXT_CMD_CFGINFO_RTS_SIGTA_EN)
				u2MsgLen += sizeof(struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T);

			if (i == EXT_CMD_CFGINFO_SCH_DET_DIS)
				u2MsgLen += sizeof(struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T);

			if (i == EXT_CMD_CFGINFO_RTS0_PKT_THRESHOLD_CFG)
				u2MsgLen += sizeof(struct _EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T);

			ucTLVNumber++;
		}
	}
	msg = AndesAllocCmdMsg(pAd, u2MsgLen);
	if (msg == NULL) {
		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_ERROR,
				 "AndesAllocCmdMsg fail!!!\n");
		return NDIS_STATUS_FAILURE;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CFG);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	AndesInitCmdMsg(msg, attr);

	/* Fill Config info parameter here*/
	rCfgBasicInfo.u4TotalTlvNum = cpu2le16(ucTLVNumber);
	rCfgBasicInfo.ucDbdcIdx = HcGetBandByWdev(wdev);
	AndesAppendCmdMsg(msg, (char *)&rCfgBasicInfo,
					  sizeof(EXT_CMD_CFG_BASIC_INFO_T));

	if (eFeature & CFGINFO_HOSTREPORT_TXLATENCY_FEATURE) {
		EXT_CMD_CFG_HOSTREPORT_UPDATE_T ExtCmdHostRepCfg = {0};
		ucAction = *((UINT8 *) param);

		ExtCmdHostRepCfg.u2Tag = EXT_CMD_CFGINFO_HOSTREPORT_TX_LATENCY;
		ExtCmdHostRepCfg.u2Length = sizeof(EXT_CMD_CFG_HOSTREPORT_UPDATE_T);
		ExtCmdHostRepCfg.ucActive = ucAction;

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "u2Tag=%d ucActive=%d\n",
				  ExtCmdHostRepCfg.u2Tag, ExtCmdHostRepCfg.ucActive);

		AndesAppendCmdMsg(msg, (char *)&ExtCmdHostRepCfg, sizeof(EXT_CMD_CFG_HOSTREPORT_UPDATE_T));
	}

	if (eFeature & CFGINFO_RX_FILTER_DROP_CTRL_FRAME_FEATURE) {
		EXT_CMD_CFG_DROP_CTRL_FRAME_T ExtCmdDropCtrlFrame = {0};
		ucAction = *((UINT8 *) param);
		ExtCmdDropCtrlFrame.u2Tag = EXT_CMD_CFGINFO_RX_FILTER_DROP_CTRL_FRAME;
		ExtCmdDropCtrlFrame.u2Length = sizeof(EXT_CMD_CFG_DROP_CTRL_FRAME_T);
		ExtCmdDropCtrlFrame.ucDropRts = (ucAction & CFGINFO_DROP_RTS_CTRL_FRAME) ? 1 : 0;
		ExtCmdDropCtrlFrame.ucDropCts = (ucAction & CFGINFO_DROP_CTS_CTRL_FRAME) ? 1 : 0;
		ExtCmdDropCtrlFrame.ucDropUnwantedCtrl = (ucAction & CFGINFO_DROP_UNWANTED_CTRL_FRAME) ? 1 : 0;

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "u2Tag=%d, ucAction=%u\n",
				  ExtCmdDropCtrlFrame.u2Tag, ucAction);

		AndesAppendCmdMsg(msg, (char *)&ExtCmdDropCtrlFrame, sizeof(EXT_CMD_CFG_DROP_CTRL_FRAME_T));
	}

	if (eFeature & CFGINFO_AGG_AC_LIMT_FEATURE) {
		P_EXT_CMD_CFG_SET_AGG_AC_LIMIT_T prExtCmdAggAcLimitCfg = (P_EXT_CMD_CFG_SET_AGG_AC_LIMIT_T) param;

		prExtCmdAggAcLimitCfg->u2Tag = EXT_CMD_CFGINFO_AGG_AC_LIMIT;
		prExtCmdAggAcLimitCfg->u2Length = sizeof(EXT_CMD_CFG_SET_AGG_AC_LIMIT_T);

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "u2Tag=%d Ac = 0x%x, AggLimit = %u\n",
				  prExtCmdAggAcLimitCfg->u2Tag,
				  prExtCmdAggAcLimitCfg->ucAc, prExtCmdAggAcLimitCfg->ucAggLimit);

		AndesAppendCmdMsg(msg, (char *)prExtCmdAggAcLimitCfg, sizeof(EXT_CMD_CFG_SET_AGG_AC_LIMIT_T));
	}

	if (eFeature & CFGINFO_CERT_CFG_FEATURE) {
		EXT_CMD_CFG_CERT_CFG_T ExtCmdCertCfg = {0};
		ucAction = *((UINT8 *) param);

		ExtCmdCertCfg.u2Tag = EXT_CMD_CFGINFO_CERT_CFG;
		ExtCmdCertCfg.u2Length = sizeof(EXT_CMD_CFG_CERT_CFG_T);
		ExtCmdCertCfg.ucCertProgram = ucAction;

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "u2Tag=%d, ucAction/ucCertProgram=%u\n",
				  ExtCmdCertCfg.u2Tag, ucAction);

		AndesAppendCmdMsg(msg, (char *)&ExtCmdCertCfg, sizeof(EXT_CMD_CFG_CERT_CFG_T));
	}

#ifdef WIFI_MD_COEX_SUPPORT
	if (eFeature & CFGINFO_POWER_BACKOFF_FEATURE) {
		EXT_CMD_CFG_POWER_BACKOFF_T ExtCmdPowerBackoff;

		ExtCmdPowerBackoff.u2Tag = EXT_CMD_CFGINFO_POWER_BACKOFF;
		ExtCmdPowerBackoff.u2Length = sizeof(EXT_CMD_CFG_POWER_BACKOFF_T);
		ExtCmdPowerBackoff.i1PowerBackoff = *((INT8 *)param);

		MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
				 "u2Tag=%d, powerBackoff=%d\n",
				  ExtCmdPowerBackoff.u2Tag, ExtCmdPowerBackoff.i1PowerBackoff);

		AndesAppendCmdMsg(msg, (char *)&ExtCmdPowerBackoff, sizeof(EXT_CMD_CFG_POWER_BACKOFF_T));
	}
#endif
    if (eFeature & CFGINFO_ACK_CTS_FEATURE) {
        P_EXT_CMD_CFG_SET_ACK_CTS_T prExtCmdAckCtsCfg = (P_EXT_CMD_CFG_SET_ACK_CTS_T) param;

        prExtCmdAckCtsCfg->u2Tag = EXT_CMD_CFGINFO_ACK_CTS;
        prExtCmdAckCtsCfg->u2Length = sizeof(EXT_CMD_CFG_SET_ACK_CTS_T);

	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_INFO,
		"u2Tag=%d, Type = %d, TimeoutValue = %u\n",
		prExtCmdAckCtsCfg->u2Tag,
		prExtCmdAckCtsCfg->u1Type,
		prExtCmdAckCtsCfg->u4TimeoutValue);

        AndesAppendCmdMsg(msg, (char *)prExtCmdAckCtsCfg, sizeof(EXT_CMD_CFG_SET_ACK_CTS_T));
	}

	if (eFeature & CFGINFO_RTS_SIGTA_EN_FEATURE) {
		struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T *prExtCmdRtsSigtaEnCfg = (struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T *)param;

		prExtCmdRtsSigtaEnCfg->u2Tag = EXT_CMD_CFGINFO_RTS_SIGTA_EN;
		prExtCmdRtsSigtaEnCfg->u2Length = sizeof(struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T);
		AndesAppendCmdMsg(msg, (char *)prExtCmdRtsSigtaEnCfg, sizeof(struct _EXT_CMD_CFG_SET_RTS_SIGTA_EN_T));
	}

	if (eFeature & CFGINFO_SCH_DET_DIS_FEATURE) {
		struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T *prExtCmdSchDetDisCfg = (struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T *)param;

		prExtCmdSchDetDisCfg->u2Tag = EXT_CMD_CFGINFO_SCH_DET_DIS;
		prExtCmdSchDetDisCfg->u2Length = sizeof(struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T);
		AndesAppendCmdMsg(msg, (char *)prExtCmdSchDetDisCfg, sizeof(struct _EXT_CMD_CFG_SET_SCH_DET_DIS_T));
	}

	if (eFeature & CFGINFO_RTS0_PKT_THRESHOLD_CFG_FEATURE) {
		struct _EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T *prExtCmdRTS0PktThresholdCfg = (struct _EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T *)param;

		prExtCmdRTS0PktThresholdCfg->u2Tag = EXT_CMD_CFGINFO_RTS0_PKT_THRESHOLD_CFG;
		prExtCmdRTS0PktThresholdCfg->u2Length = sizeof(struct _EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T);
		AndesAppendCmdMsg(msg, (char *)prExtCmdRTS0PktThresholdCfg, sizeof(struct _EXT_CMD_CFG_SET_RTS0_PKT_THRESHOLD_CFG_T));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

	MTWF_DBG(NULL, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
			 "(Ret = %d)\n", Ret);
	return Ret;
}

static VOID CmdExtCmdCfgReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _EXT_CMD_CFG_GET_ACK_CTS_T *access_reg = (struct _EXT_CMD_CFG_GET_ACK_CTS_T *)data;

	os_move_mem(msg->attr.rsp.wb_buf_in_calbk, &access_reg->u4TimeoutValue, len - 4);
	*((UINT32 *)(msg->attr.rsp.wb_buf_in_calbk)) =
		le2cpu32(*((UINT32 *)msg->attr.rsp.wb_buf_in_calbk));
}

//INT AndesCSICtrl(RTMP_ADAPTER *pAd, struct CMD_CSI_CONTROL_T *prCSICtrl)
INT32 CmdExtCmdCfgRead(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT8 Type,
	UINT32 *Value)
{

	INT32 ret = 0;
	struct cmd_msg *msg;
	struct _EXT_CMD_CFG_GET_ACK_CTS_T access_reg = {0};
	struct _CMD_ATTRIBUTE attr = {0};

	/*allocate mem*/
	msg = AndesAllocCmdMsg(pAd, sizeof(struct _EXT_CMD_CFG_GET_ACK_CTS_T));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/*set attr*/
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_CFG_RD);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_QUERY_AND_WAIT_RETRY_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, Value);
	SET_CMD_ATTR_RSP_HANDLER(attr, CmdExtCmdCfgReadCb);
	AndesInitCmdMsg(msg, attr);
	os_zero_mem(&access_reg, sizeof(access_reg));
	access_reg.u1Type = cpu2le32(Type);
	access_reg.ucDbdcIdx = HcGetBandByWdev(wdev);
	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));
	ret = AndesSendCmdMsg(pAd, msg);

	/*Debug Log*/
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"band = %d, type = %d,  TimeoutValue = %u\n",
		access_reg.ucDbdcIdx, Type, *Value);
error:
	MTWF_DBG(pAd, DBG_CAT_FW, CATFW_DBGINFO, DBG_LVL_DEBUG,
		"(ret = %d)\n", ret);
	return ret;
}
