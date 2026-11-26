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

	Module Name: warp_info
*/

#ifndef __WARP_INFO_H
#define __WARP_INFO_H

enum warp_info_type {
	WARP_INFO_WED_TX_DATA,
	WARP_INFO_WED_END,
	WARP_INFO_PAO_END,
	WARP_INFO_END,
};

struct warp_ring_ {
	dma_addr_t base;
	u32 max_cnt	: 16;
	u32 magic_cnt	: 16;
	u32 cidx;
	u32 didx;
	u32 q_cnt;
};

struct warp_ring_info {
	struct warp_ring_ *ring;
	u32 ring_num;
};


struct warp_info {
	enum warp_info_type type;
	struct warp_ring_info ring_info;
};

#endif
