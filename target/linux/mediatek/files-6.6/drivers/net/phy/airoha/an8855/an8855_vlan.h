/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023 Airoha Inc.
 */

#ifndef _AN8855_VLAN_H_
#define _AN8855_VLAN_H_

#define AN8855_NUM_PORTS	6
#define AN8855_NUM_VLANS	4095
#define AN8855_MAX_VID		4095
#define AN8855_MIN_VID		0

struct gsw_an8855;

struct an8855_port_entry {
	u16	pvid;
};

struct an8855_vlan_entry {
	u16	vid;
	u8	member;
	u8	etags;
};

struct an8855_mapping {
	char	*name;
	u16	pvids[AN8855_NUM_PORTS];
	u8	members[AN8855_NUM_VLANS];
	u8	etags[AN8855_NUM_VLANS];
	u16	vids[AN8855_NUM_VLANS];
};

extern struct an8855_mapping an8855_defaults[];

void an8855_vlan_ctrl(struct gsw_an8855 *gsw, u32 cmd, u32 val);
void an8855_apply_vlan_config(struct gsw_an8855 *gsw);
struct an8855_mapping *an8855_find_mapping(struct device_node *np);
void an8855_apply_mapping(struct gsw_an8855 *gsw, struct an8855_mapping *map);
#endif /* _AN8855_VLAN_H_ */
