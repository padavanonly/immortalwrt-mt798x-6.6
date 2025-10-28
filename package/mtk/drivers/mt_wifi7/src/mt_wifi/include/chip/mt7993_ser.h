/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2022, Mediatek Technology, Inc.
 *
 * All rights reserved. Mediatek's source code is an unpublished work and the
 * use of a copyright notice doeas not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    mt7993_ser.h

    Abstract:
    mt7993 ser manager

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __MT7993_SER_H__
#define __MT7993_SER_H__

#include "rt_config.h"
#include "ser_mngr.h"

/* WM -> WA */
#define WF_MCU_WA_CIRQ_BASE             0x89090000
#define WF_MCU_WA_CIRQ_IRQ_MASK_ADDR    (WF_MCU_WA_CIRQ_BASE + 0x070) // 0070
#define WF_MCU_WA_CIRQ_IRQ_SOFT_ADDR    (WF_MCU_WA_CIRQ_BASE + 0x0C0) // 00C0

/* WA -> WM */
#define CONN_MCU_CIRQ_BASE              0x89010000
#define CONN_MCU_CIRQ_IRQ_MASK_ADDR     (CONN_MCU_CIRQ_BASE + 0x070) // 0070
#define CONN_MCU_CIRQ_IRQ_SOFT_ADDR     (CONN_MCU_CIRQ_BASE + 0x0C0) // 00C0

#define WA_SER_STEP_ADDR                0x401984

void mt7993_ser_1_0_v1(struct ser_cb_t *ser_cb);
void mt7993_ser_10_0_v1(struct ser_cb_t *ser_cb);
void mt7993_ser_0_5_v1(struct ser_cb_t *ser_cb);
void mt7993_ser_0_0_v1(struct ser_cb_t *ser_cb);

void mt7993_dump_ser_stat(struct _RTMP_ADAPTER *ad, uint8_t dump_all);
#ifdef MTK_FE_RESET_RECOVER
void mt7993_fe_reset(struct ser_cb_t *ser_cb);
void mt7993_fe_wdma_reset(struct ser_cb_t *ser_cb);
#endif

#endif /*__MT7993_SER_H__*/
