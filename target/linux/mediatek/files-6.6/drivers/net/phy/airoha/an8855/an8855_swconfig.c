// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Airoha Inc.
 * Author: Min Yao <min.yao@airoha.com>
 */

#include <linux/if.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/bitops.h>
#include <net/genetlink.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>
#include <linux/workqueue.h>
#include <linux/of_device.h>

#include "an8855.h"
#include "an8855_swconfig.h"
#include "an8855_regs.h"

#define AN8855_PORT_MIB_TXB_ID	19	/* TxByte */
#define AN8855_PORT_MIB_RXB_ID	40	/* RxByte */

#define MIB_DESC(_s, _o, _n)   \
	{						\
		.size = (_s),	\
		.offset = (_o), \
		.name = (_n),	\
	}

struct an8855_mib_desc {
	unsigned int size;
	unsigned int offset;
	const char *name;
};

static const struct an8855_mib_desc an8855_mibs[] = {
	MIB_DESC(1, STATS_TDPC, "TxDrop"),
	MIB_DESC(1, STATS_TCRC, "TxCRC"),
	MIB_DESC(1, STATS_TUPC, "TxUni"),
	MIB_DESC(1, STATS_TMPC, "TxMulti"),
	MIB_DESC(1, STATS_TBPC, "TxBroad"),
	MIB_DESC(1, STATS_TCEC, "TxCollision"),
	MIB_DESC(1, STATS_TSCEC, "TxSingleCol"),
	MIB_DESC(1, STATS_TMCEC, "TxMultiCol"),
	MIB_DESC(1, STATS_TDEC, "TxDefer"),
	MIB_DESC(1, STATS_TLCEC, "TxLateCol"),
	MIB_DESC(1, STATS_TXCEC, "TxExcCol"),
	MIB_DESC(1, STATS_TPPC, "TxPause"),
	MIB_DESC(1, STATS_TL64PC, "Tx64Byte"),
	MIB_DESC(1, STATS_TL65PC, "Tx65Byte"),
	MIB_DESC(1, STATS_TL128PC, "Tx128Byte"),
	MIB_DESC(1, STATS_TL256PC, "Tx256Byte"),
	MIB_DESC(1, STATS_TL512PC, "Tx512Byte"),
	MIB_DESC(1, STATS_TL1024PC, "Tx1024Byte"),
	MIB_DESC(1, STATS_TL1519PC, "Tx1519Byte"),
	MIB_DESC(2, STATS_TOC, "TxByte"),
	MIB_DESC(1, STATS_TODPC, "TxOverSize"),
	MIB_DESC(2, STATS_TOC2, "SecondaryTxByte"),
	MIB_DESC(1, STATS_RDPC, "RxDrop"),
	MIB_DESC(1, STATS_RFPC, "RxFiltered"),
	MIB_DESC(1, STATS_RUPC, "RxUni"),
	MIB_DESC(1, STATS_RMPC, "RxMulti"),
	MIB_DESC(1, STATS_RBPC, "RxBroad"),
	MIB_DESC(1, STATS_RAEPC, "RxAlignErr"),
	MIB_DESC(1, STATS_RCEPC, "RxCRC"),
	MIB_DESC(1, STATS_RUSPC, "RxUnderSize"),
	MIB_DESC(1, STATS_RFEPC, "RxFragment"),
	MIB_DESC(1, STATS_ROSPC, "RxOverSize"),
	MIB_DESC(1, STATS_RJEPC, "RxJabber"),
	MIB_DESC(1, STATS_RPPC, "RxPause"),
	MIB_DESC(1, STATS_RL64PC, "Rx64Byte"),
	MIB_DESC(1, STATS_RL65PC, "Rx65Byte"),
	MIB_DESC(1, STATS_RL128PC, "Rx128Byte"),
	MIB_DESC(1, STATS_RL256PC, "Rx256Byte"),
	MIB_DESC(1, STATS_RL512PC, "Rx512Byte"),
	MIB_DESC(1, STATS_RL1024PC, "Rx1024Byte"),
	MIB_DESC(2, STATS_ROC, "RxByte"),
	MIB_DESC(1, STATS_RDPC_CTRL, "RxCtrlDrop"),
	MIB_DESC(1, STATS_RDPC_ING, "RxIngDrop"),
	MIB_DESC(1, STATS_RDPC_ARL, "RxARLDrop"),
	MIB_DESC(1, STATS_RDPC_FC, "RxFCDrop"),
	MIB_DESC(1, STATS_RDPC_WRED, "RxWREDDrop"),
	MIB_DESC(1, STATS_RDPC_MIR, "RxMIRDrop"),
	MIB_DESC(2, STATS_ROC2, "SecondaryRxByte"),
	MIB_DESC(1, STATS_RSFSPC, "RxsFlowSampling"),
	MIB_DESC(1, STATS_RSFTPC, "RxsFlowTotal"),
	MIB_DESC(1, STATS_RXCDPC, "RxPortDrop"),
};

enum {
	/* Global attributes. */
	AN8855_ATTR_ENABLE_VLAN,
};

static int an8855_get_vlan_enable(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	val->value.i = gsw->global_vlan_enable;

	return 0;
}

static int an8855_set_vlan_enable(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	gsw->global_vlan_enable = val->value.i != 0;

	return 0;
}

static int an8855_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	if (port < 0 || port >= AN8855_NUM_PORTS)
		return -EINVAL;

	*val = an8855_reg_read(gsw, PVID(port));
	*val &= GRP_PORT_VID_M;

	return 0;
}

static int an8855_set_port_pvid(struct switch_dev *dev, int port, int pvid)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	if (port < 0 || port >= AN8855_NUM_PORTS)
		return -EINVAL;

	if (pvid < AN8855_MIN_VID || pvid > AN8855_MAX_VID)
		return -EINVAL;

	gsw->port_entries[port].pvid = pvid;

	return 0;
}

static int an8855_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);
	u32 member;
	u32 etags;
	int i;

	val->len = 0;

	if (val->port_vlan >= AN8855_NUM_VLANS)
		return -EINVAL;

	an8855_vlan_ctrl(gsw, VTCR_READ_VLAN_ENTRY, val->port_vlan);

	member = an8855_reg_read(gsw, VARD0);
	member &= PORT_MEM_M;
	member >>= PORT_MEM_S;
	member |= ((an8855_reg_read(gsw, VARD1) & 0x1) << 6);

	etags = an8855_reg_read(gsw, VARD0) & ETAG_M;

	for (i = 0; i < AN8855_NUM_PORTS; i++) {
		struct switch_port *p;
		int etag;

		if (!(member & BIT(i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;

		etag = (etags >> PORT_ETAG_S(i)) & PORT_ETAG_M;

		if (etag == ETAG_CTRL_TAG)
			p->flags |= BIT(SWITCH_PORT_FLAG_TAGGED);
		else if (etag != ETAG_CTRL_UNTAG)
			dev_info(gsw->dev,
				 "vlan egress tag control neither untag nor tag.\n");
	}

	return 0;
}

static int an8855_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);
	u8 member = 0;
	u8 etags = 0;
	int i;

	if (val->port_vlan >= AN8855_NUM_VLANS ||
		val->len > AN8855_NUM_PORTS)
		return -EINVAL;

	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->id >= AN8855_NUM_PORTS)
			return -EINVAL;

		member |= BIT(p->id);

		if (p->flags & BIT(SWITCH_PORT_FLAG_TAGGED))
			etags |= BIT(p->id);
	}

	gsw->vlan_entries[val->port_vlan].member = member;
	gsw->vlan_entries[val->port_vlan].etags = etags;

	return 0;
}

static int an8855_set_vid(struct switch_dev *dev,
			  const struct switch_attr *attr,
			  struct switch_val *val)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);
	int vlan;
	u16 vid;

	vlan = val->port_vlan;
	vid = (u16)val->value.i;

	if (vlan < 0 || vlan >= AN8855_NUM_VLANS)
		return -EINVAL;

	if (vid > AN8855_MAX_VID)
		return -EINVAL;

	gsw->vlan_entries[vlan].vid = vid;
	return 0;
}

static int an8855_get_vid(struct switch_dev *dev,
			  const struct switch_attr *attr,
			  struct switch_val *val)
{
	val->value.i = val->port_vlan;
	return 0;
}

static int an8855_get_port_link(struct switch_dev *dev, int port,
				struct switch_port_link *link)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);
	u32 speed, pmsr;

	if (port < 0 || port >= AN8855_NUM_PORTS)
		return -EINVAL;

	pmsr = an8855_reg_read(gsw, PMSR(port));

	link->link = pmsr & MAC_LNK_STS;
	link->duplex = pmsr & MAC_DPX_STS;
	speed = (pmsr & MAC_SPD_STS_M) >> MAC_SPD_STS_S;

	switch (speed) {
	case MAC_SPD_10:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case MAC_SPD_100:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case MAC_SPD_1000:
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	case MAC_SPD_2500:
		/* TODO: swconfig has no support for 2500 now */
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}

	return 0;
}

static int an8855_set_port_link(struct switch_dev *dev, int port,
				struct switch_port_link *link)
{
#ifndef MODULE
	if (port >= AN8855_NUM_PHYS)
		return -EINVAL;

	return switch_generic_set_link(dev, port, link);
#else
	return -ENOTSUPP;
#endif
}

static u64 get_mib_counter(struct gsw_an8855 *gsw, int i, int port)
{
	unsigned int offset;
	u64 lo = 0, hi = 0, hi2 = 0;

	if (i >= 0) {
		offset = an8855_mibs[i].offset;

		if (an8855_mibs[i].size == 1)
			return an8855_reg_read(gsw, MIB_COUNTER_REG(port, offset));

		do {
			hi = an8855_reg_read(gsw, MIB_COUNTER_REG(port, offset + 4));
			lo = an8855_reg_read(gsw, MIB_COUNTER_REG(port, offset));
			hi2 = an8855_reg_read(gsw, MIB_COUNTER_REG(port, offset + 4));
		} while (hi2 != hi);
	}

	return (hi << 32) | lo;
}

static int an8855_get_port_mib(struct switch_dev *dev,
				   const struct switch_attr *attr,
				   struct switch_val *val)
{
	static char buf[4096];
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);
	int i, len = 0;

	if (val->port_vlan >= AN8855_NUM_PORTS)
		return -EINVAL;

	len += snprintf(buf + len, sizeof(buf) - len,
			"Port %d MIB counters\n", val->port_vlan);

	for (i = 0; i < ARRAY_SIZE(an8855_mibs); ++i) {
		u64 counter;

		len += snprintf(buf + len, sizeof(buf) - len,
				"%-11s: ", an8855_mibs[i].name);
		counter = get_mib_counter(gsw, i, val->port_vlan);
		len += snprintf(buf + len, sizeof(buf) - len, "%llu\n",
				counter);
	}

	val->value.s = buf;
	val->len = len;
	return 0;
}

static int an8855_get_port_stats(struct switch_dev *dev, int port,
				 struct switch_port_stats *stats)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	if (port < 0 || port >= AN8855_NUM_PORTS)
		return -EINVAL;

	stats->tx_bytes = get_mib_counter(gsw, AN8855_PORT_MIB_TXB_ID, port);
	stats->rx_bytes = get_mib_counter(gsw, AN8855_PORT_MIB_RXB_ID, port);

	return 0;
}

static void an8855_port_isolation(struct gsw_an8855 *gsw)
{
	int i;

	for (i = 0; i < AN8855_NUM_PORTS; i++)
		an8855_reg_write(gsw, PORTMATRIX(i),
				 BIT(gsw->cpu_port));

	an8855_reg_write(gsw, PORTMATRIX(gsw->cpu_port), PORT_MATRIX_M);

	for (i = 0; i < AN8855_NUM_PORTS; i++) {
		u32 pvc_mode = 0x9100 << STAG_VPID_S;

		if (gsw->port5_cfg.stag_on && i == 5)
			pvc_mode |= PVC_PORT_STAG | PVC_STAG_REPLACE;
		else
			pvc_mode |= (VA_TRANSPARENT_PORT << VLAN_ATTR_S);

		an8855_reg_write(gsw, PVC(i), pvc_mode);
	}
}

static int an8855_apply_config(struct switch_dev *dev)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	if (!gsw->global_vlan_enable) {
		an8855_port_isolation(gsw);
		return 0;
	}

	an8855_apply_vlan_config(gsw);

	return 0;
}

static int an8855_reset_switch(struct switch_dev *dev)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);
	int i;

	memset(gsw->port_entries, 0, sizeof(gsw->port_entries));
	memset(gsw->vlan_entries, 0, sizeof(gsw->vlan_entries));

	/* set default vid of each vlan to the same number of vlan, so the vid
	 * won't need be set explicitly.
	 */
	for (i = 0; i < AN8855_NUM_VLANS; i++)
		gsw->vlan_entries[i].vid = i;

	return 0;
}

static int an8855_phy_read16(struct switch_dev *dev, int addr, u8 reg,
				 u16 *value)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	*value = gsw->mii_read(gsw, addr, reg);

	return 0;
}

static int an8855_phy_write16(struct switch_dev *dev, int addr, u8 reg,
				  u16 value)
{
	struct gsw_an8855 *gsw = container_of(dev, struct gsw_an8855, swdev);

	gsw->mii_write(gsw, addr, reg, value);

	return 0;
}

static const struct switch_attr an8855_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = AN8855_ATTR_ENABLE_VLAN,
		.get = an8855_get_vlan_enable,
		.set = an8855_set_vlan_enable,
	}
};

static const struct switch_attr an8855_port[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.get = an8855_get_port_mib,
		.set = NULL,
	},
};

static const struct switch_attr an8855_vlan[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "vid",
		.description = "VLAN ID (0-4094)",
		.set = an8855_set_vid,
		.get = an8855_get_vid,
		.max = 4094,
	},
};

static const struct switch_dev_ops an8855_swdev_ops = {
	.attr_global = {
		.attr = an8855_global,
		.n_attr = ARRAY_SIZE(an8855_global),
	},
	.attr_port = {
		.attr = an8855_port,
		.n_attr = ARRAY_SIZE(an8855_port),
	},
	.attr_vlan = {
		.attr = an8855_vlan,
		.n_attr = ARRAY_SIZE(an8855_vlan),
	},
	.get_vlan_ports = an8855_get_vlan_ports,
	.set_vlan_ports = an8855_set_vlan_ports,
	.get_port_pvid = an8855_get_port_pvid,
	.set_port_pvid = an8855_set_port_pvid,
	.get_port_link = an8855_get_port_link,
	.set_port_link = an8855_set_port_link,
	.get_port_stats = an8855_get_port_stats,
	.apply_config = an8855_apply_config,
	.reset_switch = an8855_reset_switch,
	.phy_read16 = an8855_phy_read16,
	.phy_write16 = an8855_phy_write16,
};

int an8855_swconfig_init(struct gsw_an8855 *gsw)
{
	struct device_node *np = gsw->dev->of_node;
	struct switch_dev *swdev;
	int ret;

	if (of_property_read_u32(np, "airoha,cpuport", &gsw->cpu_port))
		gsw->cpu_port = AN8855_DFL_CPU_PORT;

	swdev = &gsw->swdev;

	swdev->name = gsw->name;
	swdev->alias = gsw->name;
	swdev->cpu_port = gsw->cpu_port;
	swdev->ports = AN8855_NUM_PORTS;
	swdev->vlans = AN8855_NUM_VLANS;
	swdev->ops = &an8855_swdev_ops;

	ret = register_switch(swdev, NULL);
	if (ret) {
		dev_notice(gsw->dev, "Failed to register switch %s\n",
			   swdev->name);
		return ret;
	}

	an8855_apply_config(swdev);

	return 0;
}

void an8855_swconfig_destroy(struct gsw_an8855 *gsw)
{
	unregister_switch(&gsw->swdev);
}
