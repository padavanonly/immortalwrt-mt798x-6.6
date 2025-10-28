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

	Module Name: warp_ccif
	warp_ccif.h
*/

#ifndef __WARP_CCIF_H__
#define __WARP_CCIF_H__

struct ccif_ring_ctrl {
	u8 chnum;
	u8 base_addr_dnum;
	u8 cnt_dnum;
	u8 cidx_dnum;
	u8 didix_dnum;
	u8 irq_mask;
};

struct ccif_entry {
	unsigned long base_addr;
	u8 irq;
	struct ccif_ring_ctrl *tx_ring;
	struct ccif_ring_ctrl *rx_ring;
};

/* exported API*/
#endif /* __WARP_CCIF_H__ */
