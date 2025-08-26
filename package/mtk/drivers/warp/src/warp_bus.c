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
	warp_hif.c
*/

#include "warp_bus.h"
#include "warp_hw.h"
#include "warp_utility.h"
#include "warp_meminfo_list.h"

#ifdef WARP_CPU_TRACER

#define BUS_TRACE_DEV_NODE "mediatek,BUS_DBG"
/*
*
*/
static irqreturn_t
bus_cputracer_isr(int irq, void *data)
{
	struct warp_cputracer *tracer = (struct warp_cputracer *)data;

	warp_bus_cputracer_work_hw(tracer);
	return IRQ_HANDLED;
}

/*
*
*/
static inline void
bus_cputracer_probe(struct warp_bus *bus)
{
	struct device_node *node = NULL;
	struct warp_cputracer *tracer = &bus->tracer;

	node = of_find_compatible_node(NULL, NULL, BUS_TRACE_DEV_NODE);
	tracer->base_addr = (unsigned long)of_iomap(node, 0);
	tracer->irq = irq_of_parse_and_map(node, 0);
	tracer->trace_addr = WHNAT_CPU_TRACER_DEFAULT_ADDR;
	tracer->trace_en = WHNAT_CPU_TRACER_DEFAULT_EN;
	tracer->trace_mask = WHNAT_CPU_TRACER_DEFAULT_MASK;

	if (request_irq(tracer->irq, (irq_handler_t)bus_cputracer_isr, IRQF_TRIGGER_LOW,
			"cpu_tracer", tracer))
		warp_dbg(WARP_DBG_OFF, "%s(): register irq: %d for cpu tracer fail!\n",
			 __func__, tracer->irq);

	warp_trace_set_hw(tracer);
}

/*
*
*/
static inline void
bus_cputracer_remove(struct warp_bus *bus)
{
	struct warp_cputracer *tracer = &bus->tracer;

	warp_io_write32(tracer, CPU_TRACER_CFG, 0);
	iounmap((void *)tracer->base_addr);
}

#endif /*WARP_CPU_TRACER*/

/*
*
*/
static inline void
bus_remove(struct warp_bus *bus)
{
	memset(bus, 0, sizeof(*bus));
}

/*
*
*/
int
bus_config_init(struct warp_bus *bus)
{
	u32 size = 0;

	/* allocate memory according to warp number */
	size = sizeof(void *) * bus->warp_num;
	warp_os_alloc_mem((unsigned char **)&bus->base_addr, size, GFP_ATOMIC);
	memset(bus->base_addr, 0, size);
	size = sizeof(u32) * bus->warp_num;
	warp_os_alloc_mem((unsigned char **)&bus->pcie_base, size, GFP_ATOMIC);
	memset(bus->pcie_base, 0, size);
	warp_os_alloc_mem((unsigned char **)&bus->pcie_ints, size, GFP_ATOMIC);
	memset(bus->pcie_ints, 0, size);
	warp_os_alloc_mem((unsigned char **)&bus->pcie_intm, size, GFP_ATOMIC);
	memset(bus->pcie_intm, 0, size);
	warp_os_alloc_mem((unsigned char **)&bus->pcie_msis, size, GFP_ATOMIC);
	memset(bus->pcie_msis, 0, size);
	warp_os_alloc_mem((unsigned char **)&bus->pcie_msim, size, GFP_ATOMIC);
	memset(bus->pcie_msim, 0, size);
	warp_os_alloc_mem((unsigned char **)&bus->wpdma_base, size, GFP_ATOMIC);
	memset(bus->wpdma_base, 0, size);

	bus_setup(bus);
#ifdef WARP_CPU_TRACER
	bus_cputracer_probe(bus);
#endif /*WARP_CPU_TRACER*/

	return 0;
}

/*
*
*/
int
bus_config_exit(struct warp_bus *bus)
{
	if (!bus)
		return 0;

#ifdef WARP_CPU_TRACER
	bus_cputracer_remove(bus);
#endif

	/* free memory */
	warp_os_free_mem(bus->wpdma_base);
	warp_os_free_mem(bus->pcie_msim);
	warp_os_free_mem(bus->pcie_msis);
	warp_os_free_mem(bus->pcie_intm);
	warp_os_free_mem(bus->pcie_ints);
	warp_os_free_mem(bus->pcie_base);
	warp_os_free_mem(bus->base_addr);

	bus_remove(bus);

	return 0;
}
