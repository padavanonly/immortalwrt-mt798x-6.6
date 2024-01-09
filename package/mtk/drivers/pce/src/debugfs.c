// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/debugfs.h>
#include <linux/inet.h>
#include <linux/uaccess.h>

#include "pce/cls.h"
#include "pce/debugfs.h"
#include "pce/dipfilter.h"
#include "pce/internal.h"
#include "pce/netsys.h"
#include "pce/pce.h"
#include "pce/tport_map.h"

#define CLS_ENTRY_DBG_CFG(_name, _type, func)				\
	{								\
		.name = #_name,						\
		.type = _type,						\
		.func = cls_entry_debug_fill_ ## _name,			\
	}

#define CLS_ENTRY_DBG_CFG_MASK_DATA(_name)				\
	CLS_ENTRY_DBG_CFG(_name, CLS_ENTRY_MASK_DATA, fill_mask_data)

#define CLS_ENTRY_DBG_CFG_DATA(_name)					\
	CLS_ENTRY_DBG_CFG(_name, CLS_ENTRY_DATA, fill_data)

enum cls_entry_config_type {
	CLS_ENTRY_MASK_DATA,
	CLS_ENTRY_DATA,
};

struct cls_entry_debug_config {
	const char *name;
	enum cls_entry_config_type type;

	union {
		int (*fill_mask_data)(struct cls_desc *cdesc, u32 mask, u32 data);
		int (*fill_data)(struct cls_desc *cdesc, u32 data);
	};
};

struct dentry *debug_root;

static void cls_entry_debug_show_l4(struct seq_file *s, void *priv,
				    struct cls_desc *cdesc)
{
	bool l4_valid = false;
	u32 l4_data_m = 0;
	u32 l4_data = 0;

	seq_printf(s, "\t\tL4 valid      mask: 0x%X,    data: 0x%X\n",
		   cdesc->l4_valid_m, cdesc->l4_valid);

	if ((cdesc->l4_valid_m & CLS_DESC_VALID_DPORT_BIT)
	    && (cdesc->l4_valid & CLS_DESC_VALID_DPORT_BIT))
		seq_printf(s, "\t\t\tL4 Dport              mask: 0x%04X,     data: %05u\n",
			   cdesc->l4_dport_m, cdesc->l4_dport);

	if ((cdesc->l4_valid_m & CLS_DESC_VALID_LOWER_HALF_WORD_BIT)
	    && (cdesc->l4_valid & CLS_DESC_VALID_LOWER_HALF_WORD_BIT)) {
		l4_valid = true;
		l4_data_m |= cdesc->l4_hdr_usr_data_m & 0xFFFF;
		l4_data |= cdesc->l4_hdr_usr_data & 0xFFFF;
	}

	if ((cdesc->l4_valid_m & CLS_DESC_VALID_UPPER_HALF_WORD_BIT)
	    && (cdesc->l4_valid & CLS_DESC_VALID_UPPER_HALF_WORD_BIT)) {
		l4_valid = true;
		l4_data_m |= ((cdesc->l4_hdr_usr_data_m & 0xFFFF0000) >> 16);
		l4_data |= ((cdesc->l4_hdr_usr_data & 0xFFFF0000) >> 16);
	}

	if (!l4_valid)
		return;

	if ((cdesc->tag_m & CLS_DESC_TAG_MATCH_L4_HDR)
	    && (cdesc->tag & CLS_DESC_TAG_MATCH_L4_HDR)) {
		seq_printf(s, "\t\t\tL4 header not empty   mask: 0x%X,        data: 0x%X\n",
			   cdesc->l4_udp_hdr_nez_m, cdesc->l4_udp_hdr_nez);
		seq_printf(s, "\t\t\tL4 header             mask: 0x%08X, data: 0x%08X\n",
			   cdesc->l4_hdr_usr_data_m, cdesc->l4_hdr_usr_data);
	} else {
		seq_printf(s, "\t\t\tUDP lite valid        mask: 0x%X,        data: 0x%X\n",
			   cdesc->l4_udp_hdr_nez_m, cdesc->l4_udp_hdr_nez);
		seq_printf(s, "\t\t\tL4 user data          mask: 0x%08X, data: 0x%08X\n",
			   cdesc->l4_hdr_usr_data_m, cdesc->l4_hdr_usr_data);
	}
}

static int cls_entry_debug_read(struct seq_file *s, void *private)
{
	struct cls_desc cdesc;
	bool cls_enabled;
	int ret;
	u32 i;

	for (i = 1; i < FE_MEM_CLS_MAX_INDEX; i++) {
		ret = mtk_pce_cls_desc_read(&cdesc, i);

		if (ret) {
			PCE_ERR("read cls desc: %u failed: %d\n", i, ret);
			return 0;
		}

		cls_enabled = (cdesc.tag && cdesc.tag_m
			       && cdesc.tag == (cdesc.tag_m & cdesc.tag));

		seq_printf(s, "CLS Entry%02u, Tag mask: 0x%X, data: 0x%X, enabled: %u\n",
			   i, cdesc.tag_m, cdesc.tag, cls_enabled);

		/* cls entry is not enabled */
		if (!cls_enabled)
			continue;

		/* CLS descriptor's tag data should only set 1 bit of [1:0] */
		if (cdesc.tag ==
		    (CLS_DESC_TAG_MATCH_L4_HDR | CLS_DESC_TAG_MATCH_L4_USR)) {
			seq_puts(s, "Invalid CLS descriptor tag\n");
			continue;
		}

		seq_puts(s, "\tAction:\n");
		seq_printf(s, "\t\tFport: %02u, Tport: %02u, TOPS_ENTRY: %02u, CDRT: %02u, QID: %u, DR_IDX: %u\n",
			   cdesc.fport, cdesc.tport_idx,
			   cdesc.tops_entry, cdesc.cdrt_idx,
			   cdesc.qid, cdesc.dr_idx);
		seq_puts(s, "\tContent:\n");
		seq_printf(s, "\t\tDIP match     mask: 0x%X,    data: 0x%X\n",
			   cdesc.dip_match_m, cdesc.dip_match);
		seq_printf(s, "\t\tL4 protocol   mask: 0x%02X,   data: 0x%02X\n",
			   cdesc.l4_type_m, cdesc.l4_type);

		cls_entry_debug_show_l4(s, private, &cdesc);

		seq_printf(s, "\t\tSport map     mask: 0x%04X, data: 0x%04X\n",
			   cdesc.sport_m, cdesc.sport);
	}

	return 0;
}

static int cls_entry_debug_fill_tag(struct cls_desc *cdesc, u32 mask, u32 data)
{
	if (mask > CLS_DESC_TAG_MASK || data > CLS_DESC_TAG_MASK)
		return -EINVAL;

	cdesc->tag_m = mask;
	cdesc->tag = data;

	return 0;
}

static int cls_entry_debug_fill_udplite_l4_hdr_nez(struct cls_desc *cdesc,
					       u32 mask, u32 data)
{
	if (mask > CLS_DESC_UDPLITE_L4_HDR_NEZ_MASK
	    || data > CLS_DESC_UDPLITE_L4_HDR_NEZ_MASK)
		return -EINVAL;

	cdesc->l4_udp_hdr_nez_m = mask;
	cdesc->l4_udp_hdr_nez = data;

	return 0;
}

static int cls_entry_debug_fill_dip_match(struct cls_desc *cdesc, u32 mask, u32 data)
{
	if (mask > CLS_DESC_DIP_MATCH || data > CLS_DESC_DIP_MATCH)
		return -EINVAL;

	cdesc->dip_match_m = mask;
	cdesc->dip_match = data;

	return 0;
}

static int cls_entry_debug_fill_l4_protocol(struct cls_desc *cdesc,
					    u32 mask, u32 data)
{
	if (mask > CLS_DESC_L4_TYPE_MASK || data > CLS_DESC_L4_TYPE_MASK)
		return -EINVAL;

	cdesc->l4_type_m = mask;
	cdesc->l4_type = data;

	return 0;
}

static int cls_entry_debug_fill_l4_valid(struct cls_desc *cdesc, u32 mask, u32 data)
{
	if (mask > CLS_DESC_L4_VALID_MASK || data > CLS_DESC_L4_VALID_MASK)
		return -EINVAL;

	cdesc->l4_valid_m = mask;
	cdesc->l4_valid = data;

	return 0;
}

static int cls_entry_debug_fill_l4_dport(struct cls_desc *cdesc, u32 mask, u32 data)
{
	if (mask > CLS_DESC_L4_DPORT_MASK || data > CLS_DESC_L4_DPORT_MASK)
		return -EINVAL;

	cdesc->l4_dport_m = mask;
	cdesc->l4_dport = data;

	return 0;
}

static int cls_entry_debug_fill_l4_data(struct cls_desc *cdesc, u32 mask, u32 data)
{
	cdesc->l4_hdr_usr_data_m = mask;
	cdesc->l4_hdr_usr_data = data;

	return 0;
}

static int cls_entry_debug_fill_fport(struct cls_desc *cdesc, u32 data)
{
	if (data > CLS_DESC_FPORT_MASK)
		return -EINVAL;

	cdesc->fport = data;

	return 0;
}

static int cls_entry_debug_fill_dr_idx(struct cls_desc *cdesc, u32 data)
{
	if (data > CLS_DESC_DR_IDX_MASK)
		return -EINVAL;

	cdesc->dr_idx = data;

	return 0;
}

static int cls_entry_debug_fill_qid(struct cls_desc *cdesc, u32 data)
{
	if (data > CLS_DESC_QID_MASK)
		return -EINVAL;

	cdesc->qid = data;

	return 0;
}

static int cls_entry_debug_fill_cdrt(struct cls_desc *cdesc, u32 data)
{
	if (data > CLS_DESC_CDRT_MASK)
		return -EINVAL;

	cdesc->cdrt_idx = data;

	return 0;
}

static int cls_entry_debug_fill_tport(struct cls_desc *cdesc, u32 data)
{
	if (data > CLS_DESC_TPORT_MASK)
		return -EINVAL;

	cdesc->tport_idx = data;

	return 0;
}

static int cls_entry_debug_fill_tops_entry(struct cls_desc *cdesc, u32 data)
{
	if (data > CLS_DESC_TOPS_ENTRY_MASK)
		return -EINVAL;

	cdesc->tops_entry = data;

	return 0;
}

static int cls_entry_debug_fill_mask_data(
			const char *buf, u32 *ofs,
			struct cls_desc *cdesc,
			int (*fill_func)(struct cls_desc *cdesc, u32 mask, u32 data))
{
	u32 nchar = 0;
	u32 mask = 0;
	u32 data = 0;
	int ret;

	if (!fill_func)
		return -ENODEV;

	ret = sscanf(buf + *ofs, "%x %x %n", &mask, &data, &nchar);
	if (ret != 2)
		return -EINVAL;

	*ofs += nchar;

	ret = fill_func(cdesc, mask, data);
	if (ret)
		PCE_ERR("invalid mask: 0x%x, data: 0x%x\n", mask, data);

	return ret;
}

static int cls_entry_debug_fill_data(const char *buf, u32 *ofs,
				struct cls_desc *cdesc,
				int (*fill_func)(struct cls_desc *cdesc, u32 data))
{
	u32 nchar = 0;
	u32 data = 0;
	int ret;

	if (!fill_func)
		return -ENODEV;

	ret = sscanf(buf + *ofs, "%x %n", &data, &nchar);
	if (ret != 1)
		return -EINVAL;

	*ofs += nchar;

	ret = fill_func(cdesc, data);
	if (ret)
		PCE_ERR("invalid data: 0x%x\n", data);

	return ret;
}

struct cls_entry_debug_config cls_entry_dbg_cfgs[] = {
	CLS_ENTRY_DBG_CFG_MASK_DATA(tag),
	CLS_ENTRY_DBG_CFG_MASK_DATA(udplite_l4_hdr_nez),
	CLS_ENTRY_DBG_CFG_MASK_DATA(dip_match),
	CLS_ENTRY_DBG_CFG_MASK_DATA(l4_protocol),
	CLS_ENTRY_DBG_CFG_MASK_DATA(l4_valid),
	CLS_ENTRY_DBG_CFG_MASK_DATA(l4_dport),
	CLS_ENTRY_DBG_CFG_MASK_DATA(l4_data),
	CLS_ENTRY_DBG_CFG_DATA(fport),
	CLS_ENTRY_DBG_CFG_DATA(dr_idx),
	CLS_ENTRY_DBG_CFG_DATA(qid),
	CLS_ENTRY_DBG_CFG_DATA(cdrt),
	CLS_ENTRY_DBG_CFG_DATA(tport),
	CLS_ENTRY_DBG_CFG_DATA(tops_entry),
};

static int cls_entry_debug_fill(const char *buf, u32 *ofs, struct cls_desc *cdesc)
{
	struct cls_entry_debug_config *cls_entry_dbg_cfg;
	char arg[32];
	u32 nchar = 0;
	u32 i;
	int ret;

	ret = sscanf(buf + *ofs, "%31s %n", arg, &nchar);
	if (ret != 1)
		return -EINVAL;

	*ofs += nchar;

	for (i = 0; i < ARRAY_SIZE(cls_entry_dbg_cfgs); i++) {
		cls_entry_dbg_cfg = &cls_entry_dbg_cfgs[i];
		if (strcmp(cls_entry_dbg_cfg->name, arg))
			continue;

		switch (cls_entry_dbg_cfg->type) {
		case CLS_ENTRY_MASK_DATA:
			ret = cls_entry_debug_fill_mask_data(buf, ofs, cdesc,
						cls_entry_dbg_cfg->fill_mask_data);
			break;

		case CLS_ENTRY_DATA:
			ret = cls_entry_debug_fill_data(buf, ofs, cdesc,
						cls_entry_dbg_cfg->fill_data);
			break;

		default:
			return -EINVAL;
		}

		if (ret)
			goto err_out;

		return ret;
	}

err_out:
	PCE_ERR("invalid argument: %s\n", arg);

	return ret;
}

static ssize_t cls_entry_debug_write(struct file *file, const char __user *buffer,
				     size_t count, loff_t *data)
{
	struct cls_desc cdesc;
	char buf[512];
	u32 nchar = 0;
	u32 ofs = 0;
	u32 idx = 0;
	int ret;

	if (count > sizeof(buf) - 1)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';

	memset(&cdesc, 0, sizeof(struct cls_desc));

	ret = sscanf(buf + ofs, "%u %n\n", &idx, &nchar);
	if (ret != 1)
		return -EINVAL;

	ofs += nchar;

	if (!idx || idx >= FE_MEM_CLS_MAX_INDEX) {
		PCE_ERR("invalid cls entry: %u\n", idx);
		return -EINVAL;
	}

	while (1) {
		if (ofs >= count)
			break;

		ret = cls_entry_debug_fill(buf, &ofs, &cdesc);
		if (ret)
			return ret;
	}

	ret = mtk_pce_cls_desc_write(&cdesc, idx);
	if (ret)
		return ret;

	return count;
}

static int cls_entry_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, cls_entry_debug_read, file->private_data);
}

static int dipfilter_debug_read(struct seq_file *s, void *private)
{
	struct dip_desc ddesc;
	int ret;
	u32 i;

	for (i = 0; i < FE_MEM_DIPFILTER_MAX_IDX; i++) {
		ret = mtk_pce_dipfilter_desc_read(&ddesc, i);
		if (ret) {
			PCE_ERR("read dipfilter desc: %u failed: %d\n", i, ret);
			return 0;
		}

		seq_printf(s, "DIPFILTER Entry%02u, enabled: %u\n",
			   i, ddesc.tag != DIPFILTER_DISABLED);

		if (ddesc.tag == DIPFILTER_DISABLED) {
			continue;
		} else if (ddesc.tag == DIPFILTER_IPV4) {
			u32 addr = cpu_to_be32(ddesc.ipv4);

			seq_printf(s, "IPv4 address: %pI4\n", &addr);
		} else if (ddesc.tag == DIPFILTER_IPV6) {
			seq_printf(s, "IPv6 address: %pI6\n", ddesc.ipv6);
		}
	}

	return 0;
}

static int dipfilter_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, dipfilter_debug_read, file->private_data);
}

static ssize_t dipfilter_debug_write(struct file *file, const char __user *buffer,
				     size_t count, loff_t *data)
{
	struct dip_desc ddesc;
	const char *end;
	char buf[512];
	char arg[5];
	char s_dip[40];
	int ret;

	if (count > sizeof(buf) - 1)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';

	memset(&ddesc, 0, sizeof(struct dip_desc));

	ret = sscanf(buf, "%4s %39s", arg, s_dip);
	if (ret != 2)
		return -EINVAL;

	if (!strcmp(arg, "ipv4")) {
		ddesc.tag = DIPFILTER_IPV4;
		if (!in4_pton(s_dip, -1, (u8 *)(&ddesc.ipv4), -1, &end) || (*end)) {
			PCE_ERR("invalid IPv4 address: %s\n", s_dip);
			return -EINVAL;
		}
	} else if (!strcmp(arg, "ipv6")) {
		ddesc.tag = DIPFILTER_IPV6;
		if (!in6_pton(s_dip, -1, (u8 *)ddesc.ipv6, -1, &end) || (*end)) {
			PCE_ERR("invalid Ipv6 address: %s\n", s_dip);
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}

	ret = mtk_pce_dipfilter_entry_add(&ddesc);
	if (ret) {
		PCE_ERR("add dipfilter entry failed: %d\n", ret);
		return ret;
	}

	return count;
}

static int tport_map_debug_read(struct seq_file *s, void *private)
{
	struct tsc_desc tdesc;
	int ret;
	u64 map;
	u32 i;

	for (i = 0; i < __PSE_PORT_MAX; i++) {
		if (TS_CONFIG_MASK & BIT(i)) {
			ret = mtk_pce_tport_map_ts_config_read(i, &tdesc);
			if (ret)
				return ret;

			map = tdesc.tport_map_lower;
			map |= ((u64)tdesc.tport_map_upper) << 32;
			seq_printf(s, "PSE_PORT%02u ", i);
			seq_printf(s, "PORT_MAP: 0x%llX, ", map);
			seq_printf(s, "default TPORT_IDX: %02u, ", tdesc.tport);
			seq_printf(s, "default CDRT_IDX: %02u, ", tdesc.cdrt_idx);
			seq_printf(s, "default TOPS_ENTRY: %02u\n", tdesc.tops_entry);
		} else if (PSE_PORT_PPE_MASK & BIT(i)) {
			ret = mtk_pce_tport_map_ppe_read((enum pse_port)i, &map);
			if (ret)
				return ret;

			seq_printf(s, "PSE PORT%02u ", i);
			seq_printf(s, "PORT_MAP: 0x%llX\n", map);
		}
	}

	return 0;
}

static int tport_map_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, tport_map_debug_read, file->private_data);
}

static int tport_map_debug_update_all(const char *buf, int *ofs, u32 tport_idx)
{
	enum pse_port port = PSE_PORT_ADMA;
	enum pse_port target;
	u64 port_map = 0;
	int nchar = 0;
	int ret = 0;

	*ofs += nchar;

	ret = sscanf(buf + *ofs, "%llx %n", &port_map, &nchar);
	if (ret != 1)
		return -EPERM;

	while (port_map && port < __PSE_PORT_MAX) {
		target = port_map & PSE_PER_PORT_MASK;
		port_map >>= PSE_PER_PORT_BITS;

		if (TS_CONFIG_MASK & BIT(port) || PSE_PORT_PPE_MASK & BIT(port)) {
			ret = mtk_pce_tport_map_pse_port_update(port,
								tport_idx,
								target);
			if (ret) {
				PCE_ERR("invalid port: %u\n", port);
				return ret;
			}
		}

		port++;
	}

	return 0;
}

static int tport_map_debug_update_single(const char *buf, int *ofs,
					 enum pse_port port, u32 tport_idx)
{
	u32 target = 0;
	int nchar = 0;
	int ret = 0;

	ret = sscanf(buf + *ofs, "%x %n", &target, &nchar);
	if (ret != 1)
		return -EPERM;

	return mtk_pce_tport_map_pse_port_update(port, tport_idx, target);
}

static ssize_t tport_map_debug_write(struct file *file, const char __user *buffer,
				     size_t count, loff_t *data)
{
	enum pse_port port = PSE_PORT_ADMA;
	char arg1[21] = {0};
	char cmd[21] = {0};
	char buf[512];
	u32 tport_idx = 0;
	int nchar = 0;
	int ret;

	if (count > sizeof(buf) - 1)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';

	ret = sscanf(buf, "%20s %20s %u %n", cmd, arg1, &tport_idx, &nchar);
	if (ret != 3)
		return -EINVAL;

	if (tport_idx > TPORT_IDX_MAX) {
		PCE_ERR("invalid tport idx: %u\n", tport_idx);
		return -EINVAL;
	}

	if (!strcmp(cmd, "UPDATE")) {
		if (!strcmp(arg1, "ALL")) {
			ret = tport_map_debug_update_all(buf, &nchar, tport_idx);
			if (ret)
				return ret;
		} else {
			ret = kstrtou32(arg1, 10, &port);
			if (ret) {
				PCE_ERR("invalid pse port: %u\n", port);
				return ret;
			}

			ret = tport_map_debug_update_single(buf, &nchar,
							    port, tport_idx);
			if (ret) {
				PCE_ERR("update tport map single failed: %d\n",
					ret);
				return ret;
			}
		}
	}

	return count;
}

static const struct file_operations cls_entry_debug_ops = {
	.open = cls_entry_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = cls_entry_debug_write,
	.release = single_release,
};

static const struct file_operations dipfilter_debug_ops = {
	.open = dipfilter_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = dipfilter_debug_write,
	.release = single_release,
};

static const struct file_operations tport_map_debug_ops = {
	.open = tport_map_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = tport_map_debug_write,
	.release = single_release,
};

int mtk_pce_debugfs_init(struct platform_device *pdev)
{
	debug_root = debugfs_create_dir("pce", NULL);
	if (!debug_root) {
		PCE_ERR("create debugfs root directory failed\n");
		return -ENOMEM;
	}

	debugfs_create_file("cls_entry", 0444, debug_root, NULL,
			    &cls_entry_debug_ops);
	debugfs_create_file("dipfilter_entry", 0444, debug_root, NULL,
			    &dipfilter_debug_ops);
	debugfs_create_file("tport_map", 0444, debug_root, NULL,
			    &tport_map_debug_ops);

	return 0;
}

void mtk_pce_debugfs_deinit(struct platform_device *pdev)
{
	debugfs_remove_recursive(debug_root);

	debug_root = NULL;
}
