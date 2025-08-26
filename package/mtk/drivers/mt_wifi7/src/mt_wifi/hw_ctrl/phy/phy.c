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

	Abstract:

	Revision History:
	who		When			What
	--------	----------		----------------------------------------------
*/
#ifdef COMPOS_WIN
#include "MTConfig.h"
#if defined(EVENT_TRACING)
#include "phy.tmh"
#endif
#else
#include "rt_config.h"
#endif

INT phy_probe(RTMP_ADAPTER *pAd)
{

	return TRUE;
}


NDIS_STATUS NICInitBBP(RTMP_ADAPTER *pAd)
{
	/* Before program BBP, we need to wait BBP/RF get wake up.*/

	if (pAd->phy_op && pAd->phy_op->bbp_init)
		return pAd->phy_op->bbp_init(pAd);
	else
		return NDIS_STATUS_FAILURE;
}


INT bbp_get_temp(struct _RTMP_ADAPTER *pAd, CHAR *temp_val)
{
	if (pAd->phy_op && pAd->phy_op->bbp_get_temp)
		return pAd->phy_op->bbp_get_temp(pAd, temp_val);
	else
		return FALSE;
}


INT bbp_tx_comp_init(RTMP_ADAPTER *pAd, INT adc_insel, INT tssi_mode)
{
	if (pAd->phy_op && pAd->phy_op->bbp_tx_comp_init)
		return pAd->phy_op->bbp_tx_comp_init(pAd, adc_insel, tssi_mode);
	else
		return FALSE;
}


INT bbp_set_txdac(struct _RTMP_ADAPTER *pAd, INT tx_dac)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_txdac)
		return pAd->phy_op->bbp_set_txdac(pAd, tx_dac);
	else
		return FALSE;
}


INT bbp_set_rxpath(struct _RTMP_ADAPTER *pAd, INT rxpath)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_rxpath)
		return pAd->phy_op->bbp_set_rxpath(pAd, rxpath);
	else
		return FALSE;
}


INT bbp_set_ctrlch(struct _RTMP_ADAPTER *pAd, UINT8 ext_ch)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_ctrlch)
		return pAd->phy_op->bbp_set_ctrlch(pAd, ext_ch);
	else
		return FALSE;
}


INT bbp_set_mmps(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_mmps)
		return pAd->phy_op->bbp_set_mmps(pAd, ReduceCorePower);
	else
		return FALSE;
}


INT bbp_get_agc(struct _RTMP_ADAPTER *pAd, CHAR *agc, RX_CHAIN_IDX chain)
{
	if (pAd->phy_op && pAd->phy_op->bbp_get_agc)
		return pAd->phy_op->bbp_get_agc(pAd, agc, chain);
	else
		return FALSE;
}


INT bbp_set_agc(struct _RTMP_ADAPTER *pAd, UCHAR agc, RX_CHAIN_IDX chain)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_agc)
		return pAd->phy_op->bbp_set_agc(pAd, agc, chain);
	else
		return FALSE;
}


INT filter_coefficient_ctrl(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	if (pAd->phy_op && pAd->phy_op->filter_coefficient_ctrl)
		return pAd->phy_op->filter_coefficient_ctrl(pAd, Channel);
	else
		return FALSE;
}


UCHAR get_random_seed_by_phy(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->get_random_seed_by_phy)
		return pAd->phy_op->get_random_seed_by_phy(pAd);
	else
		return 0;
}


INT bbp_is_ready(struct _RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->bbp_is_ready)
		return pAd->phy_op->bbp_is_ready(pAd);
	else
		return FALSE;
}


INT phy_rrm_adjust(struct _RTMP_ADAPTER *pAd)
{

	return TRUE;
}


INT phy_rrm_request(struct _RTMP_ADAPTER *pAd)
{

	return TRUE;
}


INT phy_rrm_init(struct _RTMP_ADAPTER *pAd)
{

	return TRUE;
}


#ifdef DYNAMIC_VGA_SUPPORT
INT dynamic_vga_enable(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->dynamic_vga_enable)
		return pAd->phy_op->dynamic_vga_enable(pAd);
	else
		return FALSE;
}


INT dynamic_vga_disable(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->dynamic_vga_disable)
		return pAd->phy_op->dynamic_vga_disable(pAd);
	else
		return FALSE;
}


INT dynamic_vga_adjust(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->dynamic_vga_adjust)
		return pAd->phy_op->dynamic_vga_adjust(pAd);
	else
		return FALSE;
}
#endif /* DYNAMIC_VGA_SUPPORT */

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Smart_Carrier_Sense(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->Smart_Carrier_Sense)
		return pAd->phy_op->Smart_Carrier_Sense(pAd);
	else
		return FALSE;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

