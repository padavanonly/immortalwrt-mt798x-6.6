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
	cmm_chip.c
*/

#include "rt_config.h"
#include "hdev/hdev.h"

#ifdef TXBF_SUPPORT
VOID chip_tx_bf_init(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->TxBFInit)
		ops->TxBFInit(ad, pEntry, ie_list, supportsETxBF);
}
#endif /*TXBF_SUPPORT*/

UINT32 chip_get_sku_tbl_idx(RTMP_ADAPTER *ad, UINT8 *sku_tbl_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->get_sku_tbl_idx)
		return ops->get_sku_tbl_idx(ad, sku_tbl_idx);
	return FALSE;
}

BOOLEAN chip_check_rf_lock_down(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->check_RF_lock_down)
		return ops->check_RF_lock_down(ad);
	return FALSE;
}

INT32 chip_cmd_tx(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->MtCmdTx)
		return ops->MtCmdTx(ad, msg);
	return 0;
}

BOOLEAN chip_eeprom_read16(struct _RTMP_ADAPTER *ad, UINT32 offset, USHORT *value)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->eeread)
		return ops->eeread(ad, offset, value);
	else
		return FALSE;
}

BOOLEAN chip_eeprom_read_with_range(struct _RTMP_ADAPTER *ad, UINT32 start, UINT32 length, UCHAR *pbuf)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->eeread_range)
		return ops->eeread_range(ad, start, length, pbuf);
	else
		return FALSE;
}

VOID chip_fw_init(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->FwInit && (IS_HIF_TYPE(ad, HIF_MT)))
		ops->FwInit(ad);
}

VOID chip_get_sta_per(struct _RTMP_ADAPTER *pAd, UINT16 u2WlanIdx, PUINT8 u1PER)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* per sta get PER */
	if (ops->sta_per_get)
		ops->sta_per_get(pAd, u2WlanIdx, u1PER);
}

INT32 chip_ra_init(
	struct _RTMP_ADAPTER *ad,
	struct _MAC_TABLE_ENTRY *pEntry)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->ra_init)
		return ops->ra_init(ad, pEntry);
	return 0;
}

VOID chip_get_rssi(struct _RTMP_ADAPTER *pAd, UINT16 Wcid, CHAR *RssiSet)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rssi_get)
		ops->rssi_get(pAd, Wcid, RssiSet);
}

VOID chip_get_cninfo(struct _RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT16 *pCnInfo)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/*cninfo stat parsing */
	if (ops->cninfo_get)
		ops->cninfo_get(pAd, ucBandIdx, pCnInfo);
}

VOID chip_set_mgmt_pkt_txpwr(struct _RTMP_ADAPTER *pAd,	struct wifi_dev *wdev, UINT8 prctg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->set_mgmt_pkt_txpwr_prctg)
		ops->set_mgmt_pkt_txpwr_prctg(pAd, wdev, prctg);
}
VOID chip_show_rxv_info(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_info_show)
		ops->rxv_info_show(ad, band_idx);
}

INT32 chip_get_wf_path_comb(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	BOOLEAN dbdc_mode_en,
	UINT8 *path,
	UINT8 *path_len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* wf path combination */
	if (ops->get_wf_path_comb)
		return ops->get_wf_path_comb(ad, band_idx, dbdc_mode_en, path, path_len);
	return 0;
}

INT32 chip_get_rx_stat_band(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_BAND_INFO prx_band)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_band)
		return ops->get_rx_stat_band(ad, band_idx, blk_idx, prx_band);
	return 0;
}

INT32 chip_get_rx_stat_path(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_PATH_INFO prx_path)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_path)
		return ops->get_rx_stat_path(ad, band_idx, blk_idx, prx_path);
	return 0;
}

INT32 chip_get_rx_stat_user(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_USER_INFO prx_user)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_user)
		return ops->get_rx_stat_user(ad, band_idx, blk_idx, prx_user);
	return 0;
}

INT32 chip_get_rx_stat_comm(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	P_TEST_RX_STAT_COMM_INFO prx_comm)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->get_rx_stat_comm)
		return ops->get_rx_stat_comm(ad, band_idx, blk_idx, prx_comm);
	return 0;
}

INT32 chip_get_rx_stat_all(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 blk_idx,
	struct _TEST_RX_STAT_ALL_INFO *prx_all)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->get_rx_stat_all)
		return ops->get_rx_stat_all(ad, band_idx, blk_idx, prx_all);
	return 0;
}


VOID chip_get_rx_stat(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	P_TESTMODE_STATISTIC_INFO ptest_mode_stat_info)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* query rx stat */
	if (ops->get_rx_stat)
		ops->get_rx_stat(ad, band_idx, ptest_mode_stat_info);
}

VOID chip_get_rxv_cnt(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT32 *byte_cnt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_get_byte_cnt)
		ops->rxv_get_byte_cnt(ad, band_idx, byte_cnt);
}

VOID chip_get_rxv_content(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	PVOID *content)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_get_content)
		ops->rxv_get_content(ad, band_idx, content);
}

VOID chip_dump_rxv_raw_data(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_raw_data_show)
		ops->rxv_raw_data_show(ad, band_idx);
}

VOID chip_reset_rxv_stat(struct _RTMP_ADAPTER *ad, UINT8 band_idx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_stat_reset)
		ops->rxv_stat_reset(ad, band_idx);
}

VOID chip_parse_rxv_packet(struct _RTMP_ADAPTER *ad, UINT32 Type, struct _RX_BLK *RxBlk, UCHAR *Data)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_packet_parse)
		ops->rxv_packet_parse(ad, Data);
}

VOID chip_parse_rxv_entry(struct _RTMP_ADAPTER *ad, VOID *Data)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_entry_parse)
		ops->rxv_entry_parse(ad, Data);
}

VOID chip_rxv_dump_start(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_start)
		ops->rxv_dump_start(ad);
}

VOID chip_rxv_dump_stop(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_stop)
		ops->rxv_dump_stop(ad);
}

VOID chip_rxv_dump_buf_alloc(struct _RTMP_ADAPTER *ad, UINT8 type_mask)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_buf_alloc)
		ops->rxv_dump_buf_alloc(ad, type_mask);
}

VOID chip_rxv_dump_buf_clear(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_buf_clear)
		ops->rxv_dump_buf_clear(ad);
}

VOID chip_rxv_dump_show_list(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_show_list)
		ops->rxv_dump_show_list(ad);
}

VOID chip_rxv_dump_show_rpt(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_show_rpt)
		ops->rxv_dump_show_rpt(ad);
}

VOID chip_rxv_dump_rxv_content_compose(struct _RTMP_ADAPTER *ad, UINT8 entry_idx, VOID *rxv_content, UINT32 *len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_dump_rxv_content_compose)
		ops->rxv_dump_rxv_content_compose(ad, entry_idx, rxv_content, len);
}

VOID chip_rxv_content_len(struct _RTMP_ADAPTER *ad, UINT8 type_mask, UINT8 rxv_sta_cnt, UINT16 *len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	/* rxv stat parsing */
	if (ops->rxv_content_len)
		ops->rxv_content_len(ad, type_mask, rxv_sta_cnt, len);
}

INT chip_show_pwr_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_pwr_info)
		ops->show_pwr_info(pAd);

	return 0;
}

INT chip_update_ampdu_raw_counters(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->update_ampdu_raw_counters)
		ops->update_ampdu_raw_counters(pAd, BandIdx);

	return 0;
}

#ifdef ZERO_PKT_LOSS_SUPPORT
INT chip_read_channel_stat_registers(RTMP_ADAPTER *pAd, UINT8 ucBandIdx, void *ChStat)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->read_channel_stat_registers)
		ops->read_channel_stat_registers(pAd, ucBandIdx, ChStat);

	return 0;
}
#endif

VOID AsicSetRxAnt(RTMP_ADAPTER *ad, UCHAR Ant)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->SetRxAnt)
		ops->SetRxAnt(ad, Ant);
}

#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->AsicMeasureFalseCCA)
		ops->AsicMeasureFalseCCAad;
}

VOID AsicMitigateMicrowave(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->AsicMitigateMicrowave)
		ops->AsicMitigateMicrowave(ad);
}
#endif /* MICROWAVE_OVEN_SUPPORT */

#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT32 AsicBfStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16       u2PhyMode,
	UINT8        u1BssIdx,
	UINT16       u2WlanIdx)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	if (mt_AsicBfStaRecUpdate(pAd, u2PhyMode, u1BssIdx, u2WlanIdx) != TRUE) {
		status = NDIS_STATUS_FAILURE;
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"not success, status: 0x%02x\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicBfeeStaRecUpdate(
	struct _RTMP_ADAPTER *pAd,
	UINT16       u2PhyMode,
	UINT8        u1BssIdx,
	UINT16       u2WlanIdx)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	if (mt_AsicBfeeStaRecUpdate(pAd, u2PhyMode, u1BssIdx, u2WlanIdx) != TRUE) {
		status = NDIS_STATUS_FAILURE;
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"not success, status: 0x%02x\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicBfStaRecRelease(
	struct _RTMP_ADAPTER *pAd,
	UCHAR        ucBssIdx,
	UINT16       u2WlanIdx)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	if (mt_AsicBfStaRecRelease(pAd, ucBssIdx, u2WlanIdx) != TRUE) {
		status = NDIS_STATUS_FAILURE;
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"not success, status: 0x%02x\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicBfPfmuMemAlloc(
	struct _RTMP_ADAPTER *pAd,
	UCHAR ucSu_Mu,
	UCHAR ucWlanId)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	status = CmdPfmuMemAlloc(pAd, ucSu_Mu, ucWlanId);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"not success, status: 0x%02x\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicBfPfmuMemRelease(
	struct _RTMP_ADAPTER *pAd,
	UCHAR ucWlanId)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	status = CmdPfmuMemRelease(pAd, ucWlanId);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"not success, status: 0x%02x\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicTxBfTxApplyCtrl(
	struct _RTMP_ADAPTER *pAd,
	UCHAR   ucWlanId,
	BOOLEAN fgETxBf,
	BOOLEAN fgITxBf,
	BOOLEAN fgMuTxBf,
	BOOLEAN fgPhaseCali)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	status = CmdTxBfTxApplyCtrl(pAd, ucWlanId, fgETxBf, fgITxBf, fgMuTxBf, fgPhaseCali);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"fail(status%d)\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicTxBfApClientCluster(
	struct _RTMP_ADAPTER *pAd,
	UCHAR   ucWlanId,
	UCHAR   ucCmmWlanId)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	status = CmdTxBfApClientCluster(pAd, ucWlanId, ucCmmWlanId);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR,
			"fail(status%d)\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicTxBfHwEnStatusUpdate(
	struct _RTMP_ADAPTER *pAd,
	BOOLEAN   fgETxBf,
	BOOLEAN   fgITxBf)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	status = CmdTxBfHwEnableStatusUpdate(pAd, fgETxBf, fgITxBf);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR, "fail(status%d)\n",	status);
		goto err;
	}
err:
	return status;
}

INT32 AsicTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BfNum,
	UINT8 u1BfBitmap,
	UINT8 u1BfSelBand[])
{
	INT32 status = NDIS_STATUS_SUCCESS;

	status = CmdTxBfModuleEnCtrl(pAd, u1BfNum, u1BfBitmap, u1BfSelBand);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR, "fail(status%d)\n", status);
		goto err;
	}
err:
	return status;
}

INT32 AsicTxBfCfgBfPhy(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData)
{
	INT32 status = NDIS_STATUS_SUCCESS;

	status = CmdETxBfCfgBfPhy(pAd, pucData);
	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(pAd, DBG_CAT_BF, CATBF_CFG, DBG_LVL_ERROR, "fail(status%d)\n", status);
		goto err;
	}
err:
	return status;
}
#endif /* MT_MAC && TXBF_SUPPORT */

INT32 AsicHeraStbcPriorityCtrl(
	struct _RTMP_ADAPTER *pAd,
	PUINT8 pucData)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->HeraStbcPriorityCtrl)
		return ops->HeraStbcPriorityCtrl(pAd, pucData);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 chip_tssi_set(struct _RTMP_ADAPTER *ad, char *efuse)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->tssi_set)
		return ops->tssi_set(ad, efuse);
	return 0;
}

INT32 chip_pa_lna_set(struct _RTMP_ADAPTER *ad, char *efuse)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->pa_lna_set)
		return ops->pa_lna_set(ad, efuse);

	return 0;
}

UINT16 chip_get_tid_sn(RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR tid)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->get_tid_sn)
		return ops->get_tid_sn(pAd, wcid, tid);
	else
		return 0;
}

VOID chip_hif_pci_slave_chip_defer_create(VOID *hdev_ctrl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->hif_pci_slave_chip_defer_create)
		ops->hif_pci_slave_chip_defer_create(hdev_ctrl);
}

INT32 chip_fill_key_install_cmd(
	VOID *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	VOID **wtbl_security_key,
	UINT32 *cmd_len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	/* only fill_key_install_cmd_v2 support bigtk */
	if (ops->fill_key_install_cmd)
		return ops->fill_key_install_cmd(asic_sec_info, is_sta_rec_update, wtbl_security_key, cmd_len);

	return NDIS_STATUS_FAILURE;
}

#ifdef WIFI_UNIFIED_COMMAND
INT32 chip_fill_key_install_uni_cmd(
	void *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UCHAR is_sta_rec_update, /* TRUE: sta_rec, FALSE: wtbl */
	void *wtbl_security_key)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	/* only fill_key_install_cmd_v2 support bigtk */
	if (ops->fill_key_install_uni_cmd)
		return ops->fill_key_install_uni_cmd(hdev_ctrl, asic_sec_info, is_sta_rec_update, wtbl_security_key);

	return NDIS_STATUS_FAILURE;
}

INT32 chip_fill_key_install_uni_cmd_dynsize_check(
	VOID *hdev_ctrl,
	struct _ASIC_SEC_INFO *asic_sec_info,
	UINT32 *cmd_len)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	/* only fill_key_install_cmd_v2 support bigtk */
	if (ops->fill_key_install_uni_cmd_dynsize_check)
		return ops->fill_key_install_uni_cmd_dynsize_check(asic_sec_info, cmd_len);

	return NDIS_STATUS_FAILURE;
}
#endif /* WIFI_UNIFIED_COMMAND*/

#ifdef CONFIG_TX_DELAY
VOID chip_tx_deley_parm_init(
	VOID *hdev_ctrl,
	UCHAR tx_delay_mode,
	struct tx_delay_control *tx_delay_ctl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(hdev_ctrl);

	if (ops->tx_deley_parm_init)
		ops->tx_deley_parm_init(tx_delay_mode, tx_delay_ctl);
}
#endif

#ifdef ERR_RECOVERY
VOID chip_dump_ser_stat(RTMP_ADAPTER *pAd, UINT8 dump_lvl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->dump_ser_stat)
		ops->dump_ser_stat(pAd, dump_lvl);
}

#ifdef MT7915_E1_WORKAROUND
#ifdef WFDMA_WED_COMPATIBLE
VOID chip_sw_int_polling(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->sw_int_polling)
		ops->sw_int_polling(pAd);
}
#endif
#endif
#endif

VOID chip_update_chip_cap(
	struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->update_chip_cap)
		return ops->update_chip_cap(ad);
}

UINT32 chip_get_sub_chipid(
	struct _RTMP_ADAPTER *ad,
	UINT32 *sub_chipid)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->get_sub_chipid == NULL)
		MTWF_DBG(ad, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, " get_sub_chipid: NULL\n");


	if (ops->get_sub_chipid)
		return ops->get_sub_chipid(ad, sub_chipid);

	MTWF_DBG(ad, DBG_CAT_HW, CATHW_MAC, DBG_LVL_INFO, " get_sub_chipid: %04x\n", *sub_chipid);

	*sub_chipid = 0;
	return FALSE;
}

INT ChkExceptionType(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_DBG *chip_dbg = hc_get_chip_dbg(pAd->hdev_ctrl);

	if (chip_dbg->chk_exception_type)
		return chip_dbg->chk_exception_type(pAd);

	return FALSE;
}

INT chip_init_hw_ring_setting(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->init_hw_ring_setting)
		return chip_ops->init_hw_ring_setting(ad);
	return NDIS_STATUS_SUCCESS;
}

INT chip_do_extra_action(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UCHAR *peer_mac,
	enum CHIP_EXTRA_ACTION action,
	UCHAR *data,
	ULONG *data_len)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->chip_extra_action)
		chip_ops->chip_extra_action(ad, wdev, peer_mac, action, data, data_len);
	return NDIS_STATUS_SUCCESS;
}

VOID chip_set_rro_timeout(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->set_rro_timeout)
		chip_ops->set_rro_timeout(ad);
}

VOID chip_set_rro_rss_map(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->set_rro_rss_map)
		chip_ops->set_rro_rss_map(ad);
}

BOOLEAN chip_set_spe_fr_table(
	struct _RTMP_ADAPTER *ad,
	UINT8 band_idx,
	UINT8 rate_idx,
	VOID *handle)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->set_spe_fr_table)
		return chip_ops->set_spe_fr_table(ad, band_idx, rate_idx, handle);
	else
		return FALSE;
}

UINT8 chip_determine_mlo_type(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->determine_mlo_type)
		return chip_ops->determine_mlo_type(ad, wdev);
	else
		return MEDIATEK_TLV1_TYPE4_MLO_V2;
}

UINT8 chip_determine_mlo_allow(struct _RTMP_ADAPTER *ad, struct _vendor_ie_cap *vendor_ie)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->determine_mlo_allow)
		return chip_ops->determine_mlo_allow(ad, vendor_ie);

	/* return 1 if not hook handler */
	return 1;
}

UINT8 chip_determine_hmlo_support(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (chip_ops->determine_hmlo_support)
		return chip_ops->determine_hmlo_support(ad, wdev);
	else
		return MEDIATEK_TLV1_TYPE4_HMLO_SUPPORT;
}


