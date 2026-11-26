// SPDX-License-Identifier: GPL-2.0-only
#include "rtl837x.h"

int rtl837x_vlan_read(struct rtl837x_priv *priv, u16 vid,
		      struct rtl837x_vlan_entry *e)
{
	u32 out[3] = { 0 };
	int ret;

	if (vid >= VLAN_N_VID)
		return -EINVAL;

	ret = rtl837x_ita_exec(priv, RTL837X_TBL_CVLAN, false, vid, NULL, out);
	if (ret)
		return ret;

	e->vid = vid;
	e->member = (u16)(out[0] & 0x3ff);
	e->untag = (u16)((out[0] >> 10) & 0x3ff);
	e->fid = (u8)((out[0] >> 20) & 0xf);
	e->ivl_en = !!(out[0] & BIT(24));
	return 0;
}

int rtl837x_vlan_write(struct rtl837x_priv *priv,
		       const struct rtl837x_vlan_entry *e)
{
	u32 in[3] = { 0 };

	if (e->vid >= VLAN_N_VID)
		return -EINVAL;

	in[0] = ((u32)(e->member & 0x3ff)) |
		((u32)(e->untag & 0x3ff) << 10) |
		((u32)(e->fid & 0xf) << 20) |
		(e->ivl_en ? BIT(24) : 0);

	return rtl837x_ita_exec(priv, RTL837X_TBL_CVLAN, true, e->vid, in,
				NULL);
}

int rtl837x_vlan_set_pvid(struct rtl837x_priv *priv, int port, u16 pvid)
{
	u32 shift = (port & 1) ? 16 : 0;
	u32 mask = 0xfffU << shift;

	if (port < 0 || port >= RTL837X_MAX_PORTS || pvid >= VLAN_N_VID)
		return -EINVAL;

	return regmap_update_bits(priv->map, RTL837X_VLAN_PVID_CTRL_REG(port),
				  mask, ((u32)pvid & 0xfffU) << shift);
}

int rtl837x_lut_flush_port(struct rtl837x_priv *priv, int port)
{
	u32 cmd, sts = 0;
	int ret;

	if (port < 0 || port >= RTL837X_MAX_PORTS)
		return -EINVAL;

	mutex_lock(&priv->map_lock);
	ret = regmap_write(priv->map_nolock, RTL837X_L2_FLUSH_CTRL_REG, 0);
	if (ret)
		goto out;

	cmd = RTL837X_L2_FLUSH_ACT |
	      FIELD_PREP(RTL837X_L2_FLUSH_PORT_MASK, BIT(port));
	ret = regmap_write(priv->map_nolock, RTL837X_L2_FLUSH_CMD_REG, cmd);
	if (ret)
		goto out;

	ret = regmap_read_poll_timeout(priv->map_nolock,
				       RTL837X_L2_FLUSH_CMD_REG, sts,
				       !(sts & RTL837X_L2_FLUSH_BUSY),
				       10, 10000);
out:
	mutex_unlock(&priv->map_lock);
	return ret;
}

int rtl837x_lut_flush_all(struct rtl837x_priv *priv)
{
	return regmap_write(priv->map, RTL837X_L2_FLUSH_ALL_REG, 1);
}

/* RTLPlayground improvement: configure hardware L2 aging timer (0.1s units) */
int rtl837x_lut_set_ageing_ms(struct rtl837x_priv *priv, unsigned int msec)
{
	u32 units = clamp_t(u32, msec / 100U, 10U, 0x1fffffU);

	return regmap_update_bits(priv->map, RTL837X_L2_CTRL_REG,
				  RTL837X_L2_AGING_TIME_MASK,
				  FIELD_PREP(RTL837X_L2_AGING_TIME_MASK, units));
}

int rtl837x_fdb_modify(struct rtl837x_priv *priv, int port,
		       const unsigned char *addr, u16 vid, bool add)
{
	u32 in[3] = { 0 };

	/* Guard against multicast MAC in unicast FDB entry */
	if (is_multicast_ether_addr(addr))
		return -EINVAL;

	in[0] = ((u32)addr[2] << 24) | ((u32)addr[3] << 16) |
		((u32)addr[4] << 8) | (u32)addr[5];
	in[1] = ((u32)addr[0] << 8) | (u32)addr[1] |
		(((u32)vid & 0xfffU) << 16) |
		(add ? (BIT(29) | BIT(28)) : 0); /* valid + static */
	in[2] = (u32)(port & 0xf);

	return rtl837x_ita_exec(priv, RTL837X_TBL_L2_UC, true, 0, in, NULL);
}

int rtl837x_fdb_dump_port(struct rtl837x_priv *priv, int port,
			  dsa_fdb_dump_cb_t *cb, void *data)
{
	u16 cursor = 0;
	int iter, ret = 0;

	for (iter = 0; iter < 4096; iter++) {
		u32 cmd, sts = 0, out[3] = { 0 };
		unsigned char mac[ETH_ALEN];
		bool is_static, valid;
		u16 vid, next_addr;
		u8 entry_port;

		mutex_lock(&priv->map_lock);
		/* Clear stale delete bits before NEXT_UC walk (RTLPlayground fix) */
		ret = regmap_write(priv->map_nolock, RTL837X_TABLE_L2_CTRL_REG,
				   RTL837X_TABLE_L2_METHOD_NEXT_UC);
		if (ret) {
			mutex_unlock(&priv->map_lock);
			return ret;
		}

		cmd = FIELD_PREP(RTL837X_TABLE_ADDR_MASK, cursor) |
		      FIELD_PREP(RTL837X_TABLE_TYPE_MASK, RTL837X_TBL_L2_UC) |
		      RTL837X_TABLE_EXEC;
		ret = regmap_write(priv->map_nolock, RTL837X_TABLE_CTRL_REG,
				   cmd);
		if (!ret)
			ret = regmap_read_poll_timeout(priv->map_nolock,
						       RTL837X_TABLE_CTRL_REG,
						       sts,
						       !(sts & RTL837X_TABLE_EXEC),
						       10, 10000);
		if (!ret)
			ret = regmap_read(priv->map_nolock,
					  RTL837X_TABLE_L2_CTRL_REG, &cmd);
		if (ret || !(cmd & RTL837X_TABLE_L2_HIT)) {
			mutex_unlock(&priv->map_lock);
			return ret;
		}

		next_addr = (u16)FIELD_GET(RTL837X_TABLE_ADDR_MASK, sts);
		regmap_read(priv->map_nolock, RTL837X_TABLE_READ_REG(0),
			    &out[0]);
		regmap_read(priv->map_nolock, RTL837X_TABLE_READ_REG(1),
			    &out[1]);
		regmap_read(priv->map_nolock, RTL837X_TABLE_READ_REG(2),
			    &out[2]);
		mutex_unlock(&priv->map_lock);

		valid = !!(out[1] & BIT(29));
		entry_port = (u8)(out[2] & 0xf);
		vid = (u16)((out[1] >> 16) & 0xfff);
		is_static = !!(out[1] & BIT(28));

		if (valid && entry_port == (u8)port && !vid_is_dsa_8021q(vid)) {
			mac[0] = (u8)((out[1] >> 8) & 0xff);
			mac[1] = (u8)(out[1] & 0xff);
			mac[2] = (u8)((out[0] >> 24) & 0xff);
			mac[3] = (u8)((out[0] >> 16) & 0xff);
			mac[4] = (u8)((out[0] >> 8) & 0xff);
			mac[5] = (u8)(out[0] & 0xff);
			ret = cb(mac, vid, is_static, data);
			if (ret)
				return ret;
		}

		if (next_addr <= cursor)
			break;
		cursor = next_addr;
	}

	return 0;
}

int rtl837x_mdb_modify(struct rtl837x_priv *priv, int port,
		       const unsigned char *addr, u16 vid, bool add)
{
	u32 in[3] = { 0 }, out[3] = { 0 };
	u16 mbr = 0;

	in[0] = ((u32)addr[2] << 24) | ((u32)addr[3] << 16) |
		((u32)addr[4] << 8) | (u32)addr[5];
	in[1] = ((u32)addr[0] << 8) | (u32)addr[1] |
		(((u32)vid & 0xfffU) << 16);

	if (rtl837x_ita_exec(priv, RTL837X_TBL_L2_MC, false, 0, in, out) == 0 &&
	    (out[1] & BIT(29)))
		mbr = (u16)(out[2] & 0x3ff);

	if (add)
		mbr |= BIT(port) | BIT(priv->cpu_port);
	else
		mbr &= ~BIT(port);

	in[1] |= mbr ? BIT(29) : 0;
	in[2] = mbr;
	return rtl837x_ita_exec(priv, RTL837X_TBL_L2_MC, true, 0, in, NULL);
}

void rtl837x_stp_state_set(struct rtl837x_priv *priv, int port, u8 state)
{
	u32 hw_stp, shift = (u32)(port * 2), mask = 0x3U << shift;
	int fid;

	switch (state) {
	case BR_STATE_DISABLED:
		hw_stp = 0;
		break;
	case BR_STATE_BLOCKING:
	case BR_STATE_LISTENING:
		hw_stp = 1;
		break;
	case BR_STATE_LEARNING:
		hw_stp = 2;
		break;
	case BR_STATE_FORWARDING:
	default:
		hw_stp = 3;
		break;
	}

	for (fid = 0; fid <= RTL837X_FID_MAX; fid++)
		regmap_update_bits(priv->map, RTL837X_MSTI_CTRL_REG(fid),
				   mask, hw_stp << shift);
}
