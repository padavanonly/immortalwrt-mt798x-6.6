/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2021, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************
 */

#ifndef __SER_CMM_H__
#define __SER_CMM_H__

enum SER_ACTION {
	SER_ACTION_IO_SWITCH		= 0,
	SER_ACTION_INT_TO_MCU		= 1,
	SER_ACTION_SET_DMA_TK		= 2,
	SER_ACTION_PAUSE_TRXQ		= 3,
	SER_ACTION_SET_DMA_SW_RXQ	= 4,
	SER_ACTION_HW_RESET		= 5,
	SER_ACTION_L1_START_END		= 6,
	SER_ACTION_SET_TRAFFIC		= 7,
	SER_ACTION_END,
};

#endif
