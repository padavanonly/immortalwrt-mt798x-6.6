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
	mt_testmode_bmac.h
*/

#ifndef __MT_TESTMODE_BMAC_H__
#define __MT_TESTMODE_BMAC_H__


INT32 mtb_ate_ipg_cr_restore(RTMP_ADAPTER *ad, UCHAR band_idx);
INT32 mtb_ate_mac_cr_restore(struct _RTMP_ADAPTER *ad);
INT32 mtb_ate_mac_cr_backup_and_set(struct _RTMP_ADAPTER *ad);
INT32 mtb_ate_ampdu_ba_limit(struct _RTMP_ADAPTER *ad, uint8_t wmm_idx, uint8_t agg_limit, uint8_t band_idx);
INT32 mtb_ate_set_sta_pause_cr(struct _RTMP_ADAPTER *ad, uint8_t ac_idx);
INT32 mtb_ate_set_ifs_cr(struct _RTMP_ADAPTER *pAd, uint8_t band_idx);


#endif /*  __MT_TESTMODE_BMAC_H__ */
