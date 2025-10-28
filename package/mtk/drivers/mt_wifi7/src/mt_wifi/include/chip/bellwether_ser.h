/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2018, Mediatek Technology, Inc.
 *
 * All rights reserved. Mediatek's source code is an unpublished work and the
 * use of a copyright notice doeas not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    bss_mngr.h

    Abstract:
    BSS manager

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __SER_H__
#define __SER_H__

#include "rt_config.h"
#include "ser_mngr.h"

void bellwether_ser_1_0_v1(struct ser_cb_t *ser_cb);
void bellwether_ser_0_5_v1(struct ser_cb_t *ser_cb);
void bellwether_ser_0_0_v1(struct ser_cb_t *ser_cb);

void bellwether_dump_ser_stat(struct _RTMP_ADAPTER *ad, uint8_t dump_all);

#endif /*__SER_H__*/

