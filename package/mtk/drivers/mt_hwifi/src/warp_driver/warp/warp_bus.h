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

	Module Name: warp
	warp_bus.h
*/

#ifndef _WARP_BUS_H
#define _WARP_BUS_H

#include "warp_utility.h"

#define WHNAT_CPU_TRACER_DEFAULT_EN 1
#define WHNAT_CPU_TRACER_DEFAULT_ADDR 0x1a143424
#define WHNAT_CPU_TRACER_DEFAULT_MASK 0

struct warp_cputracer {
	unsigned long base_addr;
	u8 irq;
	u8 trace_en;
	u32 trace_addr;
	u32 trace_mask;
};

struct warp_bus {
	u8 warp_num;
	void **base_addr;
	u32 *pcie_base;
	u32 *pcie_ints;
	u32 *pcie_intm;
	u32 *pcie_msis;
	u32 *pcie_msim;
	u32 *wpdma_base;
	u32 pcie_ints_offset;
	u32 pcie_msis_offset;
	unsigned long trig_flag;
	struct warp_cputracer tracer;
};

enum {
	WHNAT_TRACE_EN,
	WHNAT_TRACE_ADDR,
	WHNAT_TRACE_MSK,
};

int bus_config_init(struct warp_bus *bus);
int bus_config_exit(struct warp_bus *bus);

#define CPU_TRACER_BASE 0

#define CPU_TRACER_CFG	(CPU_TRACER_BASE + 0x0)
#define CPU_TRACER_WP_ADDR (CPU_TRACER_BASE + 0xC)
#define CPU_TRACER_WP_MASK (CPU_TRACER_BASE + 0x10)
#define CPU_TRACER_TO_TIME (CPU_TRACER_BASE + 0x4)
#define CPU_TRACER_CR_TIME (CPU_TRACER_BASE + 0x8)

#define CPU_TRACER_CON_BUS_DBG_EN      0
#define CPU_TRACER_CON_TIMEOUT_EN      1
#define CPU_TRACER_CON_SLV_ERR_EN      2
#define CPU_TRACER_CON_WP_EN           3
#define CPU_TRACER_CON_IRQ_AR_EN       4
#define CPU_TRACER_CON_IRQ_AW_EN       5
/* DE uses exotic name for ESO items... we use another human-readable name */
#define CPU_TRACER_CON_IRQ_WP_EN       6
#define CPU_TRACER_CON_IRQ_CLR         7
#define CPU_TRACER_CON_IRQ_AR_STA      8
#define CPU_TRACER_CON_IRQ_AW_STA      9
#define CPU_TRACER_CON_IRQ_WP_STA      10

#endif /*_WARP_BUS_*/
