// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Airoha Inc.
 */

#include "an8855.h"
#include "an8855_regs.h"

struct an8855_mapping an8855_def_mapping[] = {
	{
		.name = "llllw",
		.pvids = { 1, 1, 1, 1, 2, 1 },
		.members = { 0, 0x2f, 0x30 },
		.etags = { 0, 0, 0 },
		.vids = { 0, 1, 2 },
	}, {
		.name = "wllll",
		.pvids = { 2, 1, 1, 1, 1, 1 },
		.members = { 0, 0x3e, 0x21 },
		.etags = { 0, 0, 0 },
		.vids = { 0, 1, 2 },
	}, {
		.name = "lwlll",
		.pvids = { 1, 2, 1, 1, 1, 1 },
		.members = { 0, 0x3d, 0x22 },
		.etags = { 0, 0, 0 },
		.vids = { 0, 1, 2 },
	}, {
		.name = "lllll",
		.pvids = { 1, 1, 1, 1, 1, 1 },
		.members = { 0, 0x3f },
		.etags = { 0, 0 },
		.vids = { 0, 1 },
	},
};

void an8855_vlan_ctrl(struct gsw_an8855 *gsw, u32 cmd, u32 val)
{
	int i;

	an8855_reg_write(gsw, VTCR,
			 VTCR_BUSY | ((cmd << VTCR_FUNC_S) & VTCR_FUNC_M) |
			 (val & VTCR_VID_M));

	for (i = 0; i < 300; i++) {
		u32 val = an8855_reg_read(gsw, VTCR);

		if ((val & VTCR_BUSY) == 0)
			break;

		usleep_range(1000, 1100);
	}

	if (i == 300)
		dev_info(gsw->dev, "vtcr timeout\n");
}

static void an8855_write_vlan_entry(struct gsw_an8855 *gsw, int vlan, u16 vid,
					u8 ports, u8 etags)
{
	int port;
	u32 val;

	/* vlan port membership */
	if (ports) {
		val = IVL_MAC | VTAG_EN | VENTRY_VALID
			| ((ports << PORT_MEM_S) & PORT_MEM_M);
		/* egress mode */
		for (port = 0; port < AN8855_NUM_PORTS; port++) {
			if (etags & BIT(port))
				val |= (ETAG_CTRL_TAG << PORT_ETAG_S(port));
			else
				val |= (ETAG_CTRL_UNTAG << PORT_ETAG_S(port));
		}
		an8855_reg_write(gsw, VAWD0, val);
	} else {
		an8855_reg_write(gsw, VAWD0, 0);
	}

	if (ports & 0x40)
		an8855_reg_write(gsw, VAWD1, 0x1);
	else
		an8855_reg_write(gsw, VAWD1, 0x0);

	/* write to vlan table */
	an8855_vlan_ctrl(gsw, VTCR_WRITE_VLAN_ENTRY, vid);
}

void an8855_apply_vlan_config(struct gsw_an8855 *gsw)
{
	int i, j;
	u8 tag_ports;
	u8 untag_ports;
	u32 val;

	/* set all ports as security mode */
	for (i = 0; i < AN8855_NUM_PORTS; i++) {
		val = an8855_reg_read(gsw, PCR(i));
		an8855_reg_write(gsw, PCR(i), val | SECURITY_MODE);
		an8855_reg_write(gsw, PORTMATRIX(i), PORT_MATRIX_M);
	}

	/* check if a port is used in tag/untag vlan egress mode */
	tag_ports = 0;
	untag_ports = 0;

	for (i = 0; i < AN8855_NUM_VLANS; i++) {
		u8 member = gsw->vlan_entries[i].member;
		u8 etags = gsw->vlan_entries[i].etags;

		if (!member)
			continue;

		for (j = 0; j < AN8855_NUM_PORTS; j++) {
			if (!(member & BIT(j)))
				continue;

			if (etags & BIT(j))
				tag_ports |= 1u << j;
			else
				untag_ports |= 1u << j;
		}
	}

	/* set all untag-only ports as transparent and the rest as user port */
	for (i = 0; i < AN8855_NUM_PORTS; i++) {
		u32 pvc_mode = 0x9100 << STAG_VPID_S;

		if (untag_ports & BIT(i) && !(tag_ports & BIT(i)))
			pvc_mode = (0x9100 << STAG_VPID_S) |
				(VA_TRANSPARENT_PORT << VLAN_ATTR_S);

		if (gsw->port5_cfg.stag_on && i == 5)
			pvc_mode = (u32)((0x9100 << STAG_VPID_S) | PVC_PORT_STAG
						| PVC_STAG_REPLACE);

		an8855_reg_write(gsw, PVC(i), pvc_mode);
	}

	/* first clear the switch vlan table */
	for (i = 0; i < AN8855_NUM_VLANS; i++)
		an8855_write_vlan_entry(gsw, i, i, 0, 0);

	/* now program only vlans with members to avoid
	 * clobbering remapped entries in later iterations
	 */
	for (i = 0; i < AN8855_NUM_VLANS; i++) {
		u16 vid = gsw->vlan_entries[i].vid;
		u8 member = gsw->vlan_entries[i].member;
		u8 etags = gsw->vlan_entries[i].etags;

		if (member)
			an8855_write_vlan_entry(gsw, i, vid, member, etags);
	}

	/* Port Default PVID */
	for (i = 0; i < AN8855_NUM_PORTS; i++) {
		int vlan = gsw->port_entries[i].pvid;
		u16 pvid = 0;
		u32 val;

		if ((vlan >= 0) && (vlan < AN8855_NUM_VLANS)
			&& (gsw->vlan_entries[vlan].member))
			pvid = gsw->vlan_entries[vlan].vid;

		val = an8855_reg_read(gsw, PVID(i));
		val &= ~GRP_PORT_VID_M;
		val |= pvid;
		an8855_reg_write(gsw, PVID(i), val);
	}
}

struct an8855_mapping *an8855_find_mapping(struct device_node *np)
{
	const char *map;
	int i;

	if (of_property_read_string(np, "airoha,portmap", &map))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(an8855_def_mapping); i++)
		if (!strcmp(map, an8855_def_mapping[i].name))
			return &an8855_def_mapping[i];

	return NULL;
}

void an8855_apply_mapping(struct gsw_an8855 *gsw, struct an8855_mapping *map)
{
	int i = 0;

	for (i = 0; i < AN8855_NUM_PORTS; i++)
		gsw->port_entries[i].pvid = map->pvids[i];

	for (i = 0; i < AN8855_NUM_VLANS; i++) {
		gsw->vlan_entries[i].member = map->members[i];
		gsw->vlan_entries[i].etags = map->etags[i];
		gsw->vlan_entries[i].vid = map->vids[i];
	}
}
