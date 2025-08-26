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
	warp_woctrl.h
*/

#ifndef _WARP_WOCTRL_H_
#define _WARP_WOCTRL_H_

#include "warp_utility.h"

struct warp_entry;

struct warp_wo_ctrl {
	u8 cur_state;
	u8 prev_state;
};

extern void warp_woctrl_init_state(struct warp_entry *warp);
extern void warp_woctrl_enter_state(struct warp_entry *warp, u8 state);
extern void warp_woctrl_exit_state(struct warp_entry *warp, u8 state);


#endif /*_WARP_WOCTRL_H_*/
