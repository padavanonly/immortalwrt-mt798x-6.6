// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023 Airoha Inc.
 * Author: Min Yao <min.yao@airoha.com>
 */

#include <linux/kernel.h>
#include <linux/delay.h>

#include "an8855.h"
#include "an8855_regs.h"

void an8855_irq_enable(struct gsw_an8855 *gsw)
{
	u32 val;
	int i;

	/* Record initial PHY link status */
	for (i = 0; i < AN8855_NUM_PHYS; i++) {
		val = gsw->mii_read(gsw, i, MII_BMSR);
		if (val & BMSR_LSTATUS)
			gsw->phy_link_sts |= BIT(i);
	}

	val = BIT(AN8855_NUM_PHYS) - 1;
	an8855_reg_write(gsw, SYS_INT_EN, val);

	val = an8855_reg_read(gsw, INT_MASK);
	val |= INT_SYS_BIT;
	an8855_reg_write(gsw, INT_MASK, val);
}

static void display_port_link_status(struct gsw_an8855 *gsw, u32 port)
{
	u32 pmsr, speed_bits;
	const char *speed;

	pmsr = an8855_reg_read(gsw, PMSR(port));

	speed_bits = (pmsr & MAC_SPD_STS_M) >> MAC_SPD_STS_S;

	switch (speed_bits) {
	case MAC_SPD_10:
		speed = "10Mbps";
		break;
	case MAC_SPD_100:
		speed = "100Mbps";
		break;
	case MAC_SPD_1000:
		speed = "1Gbps";
		break;
	case MAC_SPD_2500:
		speed = "2.5Gbps";
		break;
	default:
		dev_info(gsw->dev, "Invalid speed\n");
		return;
	}

	if (pmsr & MAC_LNK_STS) {
		dev_info(gsw->dev, "Port %d Link is Up - %s/%s\n",
			 port, speed, (pmsr & MAC_DPX_STS) ? "Full" : "Half");
	} else {
		dev_info(gsw->dev, "Port %d Link is Down\n", port);
	}
}

void an8855_irq_worker(struct work_struct *work)
{
	struct gsw_an8855 *gsw;
	u32 sts, physts, laststs;
	int i;

	gsw = container_of(work, struct gsw_an8855, irq_worker);

	sts = an8855_reg_read(gsw, SYS_INT_STS);

	/* Check for changed PHY link status */
	for (i = 0; i < AN8855_NUM_PHYS; i++) {
		if (!(sts & PHY_LC_INT(i)))
			continue;

		laststs = gsw->phy_link_sts & BIT(i);
		physts = !!(gsw->mii_read(gsw, i, MII_BMSR) & BMSR_LSTATUS);
		physts <<= i;

		if (physts ^ laststs) {
			gsw->phy_link_sts ^= BIT(i);
			display_port_link_status(gsw, i);
		}
	}

	an8855_reg_write(gsw, SYS_INT_STS, sts);

	enable_irq(gsw->irq);
}
