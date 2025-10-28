/* SPDX-License-Identifier: <SPDX License Expression> */
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

	Module Name: wifi_offload
	warp_dbg.h
*/

#ifndef _WARP_DBG_H
#define _WARP_DBG_H

int warp_get_ring_status(void *hw, u32 host_stat[], u8 host_stat_num,
	u32 wdma_stat[], u8 wdma_stat_num);

#endif
