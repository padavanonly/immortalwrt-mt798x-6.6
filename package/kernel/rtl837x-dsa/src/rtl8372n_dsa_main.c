// SPDX-License-Identifier: GPL-2.0-only
#include "rtl837x.h"

/* MIB counter indices matching RTL8372N/RTL8373 hardware MIB engine (0x0f60) */
#define RTL837X_MIB_IF_IN_OCTETS	0
#define RTL837X_MIB_IF_IN_UCAST_PKTS	2
#define RTL837X_MIB_IF_IN_MCAST_PKTS	3
#define RTL837X_MIB_IF_IN_BCAST_PKTS	4
#define RTL837X_MIB_IF_OUT_OCTETS	22
#define RTL837X_MIB_IF_OUT_UCAST_PKTS	24
#define RTL837X_MIB_IF_OUT_MCAST_PKTS	25
#define RTL837X_MIB_IF_OUT_BCAST_PKTS	26
#define RTL837X_MIB_DOT1D_TP_IN_DISCARD	36

static int rtl837x_read_mib_u64(struct rtl837x_priv *priv, int port, u8 mib_id,
				u64 *out)
{
	u32 ctrl, lo = 0, hi = 0;
	int ret;

	mutex_lock(&priv->map_lock);
	ctrl = RTL837X_MIB_BUSY_MASK |
	       FIELD_PREP(RTL837X_MIB_PORT_IDX_MASK, port) |
	       FIELD_PREP(RTL837X_MIB_ID_MASK, mib_id);
	ret = regmap_write(priv->map_nolock, RTL837X_MIB_CTRL_REG, ctrl);
	if (ret)
		goto unlock;

	ret = regmap_read_poll_timeout(priv->map_nolock, RTL837X_MIB_CTRL_REG,
				       ctrl, !(ctrl & RTL837X_MIB_BUSY_MASK),
				       10, 5000);
	if (ret)
		goto unlock;

	ret = regmap_read(priv->map_nolock, RTL837X_MIB_DATA_LO_REG, &lo);
	if (ret)
		goto unlock;
	ret = regmap_read(priv->map_nolock, RTL837X_MIB_DATA_HI_REG, &hi);
	if (!ret)
		*out = ((u64)hi << 32) | lo;

unlock:
	mutex_unlock(&priv->map_lock);
	return ret;
}

static void rtl8372n_stats_poll_work(struct work_struct *work)
{
	struct rtl837x_priv *priv = container_of(to_delayed_work(work),
						 struct rtl837x_priv,
						 stats_work);
	int p;

	if (READ_ONCE(priv->stopping))
		return;

	for (p = 0; p < priv->num_ports; p++) {
		u64 in_oct = 0, in_uc = 0, in_mc = 0, in_bc = 0, in_drp = 0;
		u64 out_oct = 0, out_uc = 0, out_mc = 0, out_bc = 0;
		u64 rx_pkts, tx_pkts;

		if (READ_ONCE(priv->stopping))
			return;

		if (!dsa_is_user_port(priv->ds, p) &&
		    !dsa_is_cpu_port(priv->ds, p))
			continue;

		if (rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_IN_OCTETS, &in_oct) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_IN_UCAST_PKTS, &in_uc) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_IN_MCAST_PKTS, &in_mc) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_IN_BCAST_PKTS, &in_bc) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_OUT_OCTETS, &out_oct) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_OUT_UCAST_PKTS, &out_uc) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_OUT_MCAST_PKTS, &out_mc) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_IF_OUT_BCAST_PKTS, &out_bc) ||
		    rtl837x_read_mib_u64(priv, p, RTL837X_MIB_DOT1D_TP_IN_DISCARD, &in_drp))
			continue;

		rx_pkts = in_uc + in_mc + in_bc;
		tx_pkts = out_uc + out_mc + out_bc;

		spin_lock_bh(&priv->stats_lock);
		priv->ports[p].rx_pkts = rx_pkts;
		priv->ports[p].tx_pkts = tx_pkts;
		priv->ports[p].rx_bytes = (in_oct >= rx_pkts * ETH_FCS_LEN) ?
					  (in_oct - rx_pkts * ETH_FCS_LEN) : in_oct;
		priv->ports[p].tx_bytes = (out_oct >= tx_pkts * ETH_FCS_LEN) ?
					  (out_oct - tx_pkts * ETH_FCS_LEN) : out_oct;
		priv->ports[p].rx_drops = in_drp;
		spin_unlock_bh(&priv->stats_lock);
	}

	if (!READ_ONCE(priv->stopping))
		schedule_delayed_work(&priv->stats_work, msecs_to_jiffies(3000));
}

static enum dsa_tag_protocol
rtl8372n_get_tag_protocol(struct dsa_switch *ds, int port,
			  enum dsa_tag_protocol mprot)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)port;
	(void)mprot;
	return priv->tag_proto;
}

static void rtl8372n_restore_master_csum(struct dsa_switch *ds)
{
	struct rtl837x_priv *priv = ds->priv;
	struct dsa_port *cpu_dp;

	if (!priv->csum_overridden)
		return;

	dsa_switch_for_each_cpu_port(cpu_dp, ds) {
		struct net_device *mdev = rtl837x_dsa_cpu_master(cpu_dp);

		if (mdev) {
			mdev->wanted_features |= priv->csum_backup;
			netdev_update_features(mdev);
		}
	}
	priv->csum_overridden = false;
}

static int rtl8372n_apply_tag_8021q(struct dsa_switch *ds)
{
	struct rtl837x_priv *priv = ds->priv;
	int p, ret;

	/*
	 * Explicitly disable proprietary 0x8899 CPU tag on all ports and enable
	 * RTLPlayground's MAC_L2_PORT_CTRL bit 6 (0x0f77) so standard IEEE
	 * 802.1Q SVLAN tags carry port identity transparently into MT7988 PPE.
	 */
	regmap_write(priv->map, RTL837X_CPU_CTRL_REG, 0);
	regmap_write(priv->map, RTL837X_CPU_TAG_AWARE_CTRL_REG, 0);
	for (p = 0; p < priv->num_ports; p++)
		regmap_write(priv->map, RTL837X_MAC_L2_PORT_CTRL_REG(p),
			     RTL837X_MAC_L2_PORT_DFLT_8021Q_VAL);

	rtl8372n_restore_master_csum(ds);

	regmap_write(priv->map, RTL837X_SVLAN_TRAP_CTRL_REG, BIT(1));
	regmap_write(priv->map, RTL837X_VS_CTRL_REG, BIT(0));
	regmap_write(priv->map, RTL837X_VS_GLB_CTRL_REG, ETH_P_8021Q);
	regmap_write(priv->map, RTL837X_VS_UPLINK_PORT_REG, dsa_cpu_ports(ds));

	if (!ds->tag_8021q_ctx) {
		bool need_rtnl = !rtnl_is_locked();

		if (need_rtnl)
			rtnl_lock();
		ret = dsa_tag_8021q_register(ds, htons(ETH_P_8021Q));
		if (ret)
			ds->tag_8021q_ctx = NULL;
		if (need_rtnl)
			rtnl_unlock();
		if (ret)
			return ret;
	}

	return 0;
}

static int rtl8372n_apply_tag_rtl8_4(struct dsa_switch *ds)
{
	struct rtl837x_priv *priv = ds->priv;
	struct dsa_port *cpu_dp;
	int p;

	if (ds->tag_8021q_ctx) {
		bool need_rtnl = !rtnl_is_locked();

		if (need_rtnl)
			rtnl_lock();
		dsa_tag_8021q_unregister(ds);
		if (need_rtnl)
			rtnl_unlock();
	}

	for (p = 0; p < priv->num_ports; p++)
		regmap_write(priv->map, RTL837X_MAC_L2_PORT_CTRL_REG(p),
			     RTL837X_MAC_L2_PORT_DFLT_RTLTAG_VAL);

	dsa_switch_for_each_cpu_port(cpu_dp, ds) {
		struct net_device *mdev = rtl837x_dsa_cpu_master(cpu_dp);
		const netdev_features_t csum_mask =
			NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_HW_CSUM;

		regmap_write(priv->map, RTL837X_EXT_CPU_CTRL_REG, cpu_dp->index);
		regmap_write(priv->map, RTL837X_CPU_TAG_AWARE_CTRL_REG,
			     BIT(cpu_dp->index));
		regmap_write(priv->map, RTL837X_CPU_CTRL_REG,
			     RTL837X_CPU_CTRL_EN);

		if (mdev && !priv->csum_overridden) {
			priv->csum_backup = mdev->wanted_features & csum_mask;
			mdev->wanted_features &= ~csum_mask;
			priv->csum_overridden = true;
			netdev_update_features(mdev);
		}
		break;
	}

	return 0;
}

static int rtl8372n_change_tag_protocol(struct dsa_switch *ds,
					enum dsa_tag_protocol proto)
{
	struct rtl837x_priv *priv = ds->priv;
	int ret;

	switch ((int)proto) {
	case DSA_TAG_PROTO_RTL837X_8021Q_VAL:
		ret = rtl8372n_apply_tag_8021q(ds);
		break;
	case DSA_TAG_PROTO_RTL8_4:
		ret = rtl8372n_apply_tag_rtl8_4(ds);
		break;
	default:
		return -EPROTONOSUPPORT;
	}

	if (!ret)
		priv->tag_proto = proto;
	return ret;
}

static void rtl8372n_update_port_isolation(struct rtl837x_priv *priv)
{
	u16 cpu_mask = BIT(priv->cpu_port);
	int p;

	for (p = 0; p < priv->num_ports; p++) {
		u16 allow = 0;

		if (p == priv->cpu_port)
			allow = (u16)((1U << priv->num_ports) - 1U) & ~cpu_mask;
		else if (priv->bridge_mask & BIT(p))
			allow = (priv->bridge_mask & ~BIT(p)) | cpu_mask;
		else
			allow = cpu_mask;

		regmap_write(priv->map, RTL837X_PORT_ISOLATION_REG(p), allow);
	}
}

static int rtl8372n_setup(struct dsa_switch *ds)
{
	struct rtl837x_priv *priv = ds->priv;
	struct dsa_port *cpu_dp;
	int p, ret, cpu_cnt = 0;

	dsa_switch_for_each_cpu_port(cpu_dp, ds) {
		priv->cpu_port = (u8)cpu_dp->index;
		cpu_cnt++;
	}
	if (cpu_cnt != 1)
		return -ENODEV;

	/* Disable RS link-fault propagation & enable SMI clocks */
	regmap_update_bits(priv->map, RTL837X_RS_LAYER_CONFIG_REG,
			   RTL837X_RS_LINK_FAULT_INDI_OFF,
			   RTL837X_RS_LINK_FAULT_INDI_OFF);
	regmap_write(priv->map, RTL837X_MAC_L2_GLOBAL_CTRL_REG, 0x3);
	regmap_write(priv->map, RTL837X_SMI_CTRL_REG, 0xffffffffU);

	ret = rtl837x_serdes_init_polarity(priv);
	if (ret)
		return ret;

	ret = rtl837x_gpio_and_led_init(priv);
	if (ret)
		return ret;

	ret = rtl837x_setup_internal_mdio(priv);
	if (ret)
		return ret;

	/* Reset CVLAN table, flush L2 LUT, and program 300s default aging */
	regmap_update_bits(priv->map, RTL837X_VLAN_CTRL_REG,
			   RTL837X_VLAN_CTRL_TABLE_RST |
			   RTL837X_VLAN_CTRL_CVLAN_FILTER,
			   RTL837X_VLAN_CTRL_TABLE_RST |
			   RTL837X_VLAN_CTRL_CVLAN_FILTER);
	rtl837x_lut_flush_all(priv);
	rtl837x_lut_set_ageing_ms(priv, 300000U);

	for (p = 0; p < priv->num_ports; p++) {
		priv->ports[p].priv = priv;
		priv->ports[p].index = p;

		/* Clear any bootloader-leftover force-link bits */
		regmap_update_bits(priv->map, RTL837X_MAC_FORCE_MODE_REG(p),
				   RTL837X_MAC_FORCE_LINK_MASK, 0);
		regmap_write(priv->map, RTL837X_PORT_FLOWCTRL_REG(p), 0x1050);

		if (rtl837x_serdes_port_to_sds(p) >= 0) {
			priv->ports[p].is_serdes = true;
			priv->ports[p].pcs.ops = &rtl837x_sds_pcs_ops;
			priv->ports[p].pcs.poll = true;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 18, 0)
			priv->ports[p].pcs.neg_mode = true;
#endif
		}

		rtl837x_stp_state_set(priv, p,
				      (p == priv->cpu_port) ?
				      BR_STATE_FORWARDING : BR_STATE_DISABLED);
	}

	regmap_write(priv->map, RTL837X_MAC_PORT_EN_REG, 0x1ff);
	rtl8372n_update_port_isolation(priv);

	ds->configure_vlan_while_not_filtering = true;
	ds->untag_bridge_pvid = true;

	ret = rtl8372n_change_tag_protocol(ds, priv->tag_proto);
	if (ret)
		return ret;

	WRITE_ONCE(priv->stopping, false);
	INIT_DELAYED_WORK(&priv->stats_work, rtl8372n_stats_poll_work);
	schedule_delayed_work(&priv->stats_work, msecs_to_jiffies(3000));
	return 0;
}

static void rtl8372n_teardown(struct dsa_switch *ds)
{
	struct rtl837x_priv *priv = ds->priv;

	WRITE_ONCE(priv->stopping, true);
	cancel_delayed_work_sync(&priv->stats_work);

	if (ds->tag_8021q_ctx) {
		bool need_rtnl = !rtnl_is_locked();

		if (need_rtnl)
			rtnl_lock();
		dsa_tag_8021q_unregister(ds);
		if (need_rtnl)
			rtnl_unlock();
	}

	rtl8372n_restore_master_csum(ds);
}

static void rtl8372n_phylink_get_caps(struct dsa_switch *ds, int port,
				      struct phylink_config *config)
{
	(void)ds;
	config->mac_capabilities = MAC_ASYM_PAUSE | MAC_SYM_PAUSE |
				   MAC_10 | MAC_100 | MAC_1000FD | MAC_2500FD;

	if (port == 3 || port == 8) {
		config->mac_capabilities |= MAC_5000FD | MAC_10000FD;
		__set_bit(PHY_INTERFACE_MODE_10GBASER,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_10GKR,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_USXGMII,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_2500BASEX,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_1000BASEX,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_SGMII,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_100BASEX,
			  config->supported_interfaces);
	} else {
		__set_bit(PHY_INTERFACE_MODE_INTERNAL,
			  config->supported_interfaces);
	}
}

static struct phylink_pcs *
rtl8372n_phylink_mac_select_pcs(struct dsa_switch *ds, int port,
				phy_interface_t interface)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)interface;
	if (port >= 0 && port < priv->num_ports && priv->ports[port].is_serdes)
		return &priv->ports[port].pcs;
	return NULL;
}

static void rtl8372n_phylink_mac_config(struct dsa_switch *ds, int port,
					unsigned int mode,
					const struct phylink_link_state *state)
{
	(void)ds;
	(void)port;
	(void)mode;
	(void)state;
}

static void rtl8372n_phylink_mac_link_down(struct dsa_switch *ds, int port,
					   unsigned int mode,
					   phy_interface_t interface)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)mode;
	(void)interface;
	if (dsa_is_user_port(ds, port))
		rtl837x_lut_flush_port(priv, port);
}

static void rtl8372n_phylink_mac_link_up(struct dsa_switch *ds, int port,
					 unsigned int mode,
					 phy_interface_t interface,
					 struct phy_device *phydev,
					 int speed, int duplex,
					 bool tx_pause, bool rx_pause)
{
	(void)ds;
	(void)port;
	(void)mode;
	(void)interface;
	(void)phydev;
	(void)speed;
	(void)duplex;
	(void)tx_pause;
	(void)rx_pause;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
static const struct dsa_phylink_mac_ops rtl8372n_phylink_mac_ops = {
	.mac_select_pcs	= rtl8372n_phylink_mac_select_pcs,
	.mac_config	= rtl8372n_phylink_mac_config,
	.mac_link_down	= rtl8372n_phylink_mac_link_down,
	.mac_link_up	= rtl8372n_phylink_mac_link_up,
};
#endif

static int rtl8372n_port_max_mtu(struct dsa_switch *ds, int port)
{
	(void)ds;
	(void)port;
	return RTL837X_MAC_MAX_LEN_MAX - VLAN_ETH_HLEN -
	       RTL837X_TAG_QINQ_OVERHEAD - ETH_FCS_LEN;
}

static int rtl8372n_port_change_mtu(struct dsa_switch *ds, int port,
				    int new_mtu)
{
	struct rtl837x_priv *priv = ds->priv;
	/* Add QinQ/SVLAN + CVLAN headroom so 1500B VLAN/PPPoE frames never drop */
	u32 len = (u32)new_mtu + VLAN_ETH_HLEN + RTL837X_TAG_QINQ_OVERHEAD +
		  ETH_FCS_LEN;

	if (len > RTL837X_MAC_MAX_LEN_MAX)
		return -EINVAL;

	return regmap_write(priv->map, RTL837X_MAC_MAX_LEN_REG(port),
			    RTL837X_MAC_MAX_LEN_INC_TAG |
			    FIELD_PREP(RTL837X_MAC_MAX_LEN_LO_MASK, len) |
			    FIELD_PREP(RTL837X_MAC_MAX_LEN_HI_MASK, len));
}

static int rtl8372n_port_bridge_join(struct dsa_switch *ds, int port,
				     struct dsa_bridge bridge,
				     bool *tx_fwd_offload,
				     struct netlink_ext_ack *extack)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)bridge;
	(void)extack;
	priv->bridge_mask |= BIT(port);
	*tx_fwd_offload = true;
	rtl8372n_update_port_isolation(priv);
	return 0;
}

static void rtl8372n_port_bridge_leave(struct dsa_switch *ds, int port,
				       struct dsa_bridge bridge)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)bridge;
	priv->bridge_mask &= ~BIT(port);
	rtl8372n_update_port_isolation(priv);
	rtl837x_lut_flush_port(priv, port);
}

static void rtl8372n_port_stp_state_set(struct dsa_switch *ds, int port,
					u8 state)
{
	rtl837x_stp_state_set(ds->priv, port, state);
}

static void rtl8372n_port_fast_age(struct dsa_switch *ds, int port)
{
	rtl837x_lut_flush_port(ds->priv, port);
}

static int rtl8372n_set_ageing_time(struct dsa_switch *ds, unsigned int msecs)
{
	return rtl837x_lut_set_ageing_ms(ds->priv, msecs);
}

static int rtl8372n_port_pre_bridge_flags(struct dsa_switch *ds, int port,
					  struct switchdev_brport_flags flags,
					  struct netlink_ext_ack *extack)
{
	(void)ds;
	(void)port;
	(void)extack;
	if (flags.mask & ~(BR_LEARNING))
		return -EINVAL;
	return 0;
}

static int rtl8372n_port_bridge_flags(struct dsa_switch *ds, int port,
				      struct switchdev_brport_flags flags,
				      struct netlink_ext_ack *extack)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)extack;
	if (flags.mask & BR_LEARNING) {
		bool learn = !!(flags.val & BR_LEARNING);

		regmap_update_bits(priv->map, RTL837X_VLAN_L2_LRN_DIS_REG,
				   BIT(port), learn ? 0 : BIT(port));
		regmap_write(priv->map, RTL837X_LUT_PORT_LEARN_LIMIT_REG(port),
			     learn ? RTL837X_LEARN_LIMIT_MAX : 0);
	}
	return 0;
}

static int rtl8372n_port_vlan_filtering(struct dsa_switch *ds, int port,
					bool vlan_filtering,
					struct netlink_ext_ack *extack)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)extack;
	return regmap_update_bits(priv->map, RTL837X_VLAN_PORT_IGR_FLTR_REG,
				  BIT(port), vlan_filtering ? BIT(port) : 0);
}

static int rtl8372n_port_vlan_add(struct dsa_switch *ds, int port,
				  const struct switchdev_obj_port_vlan *vlan,
				  struct netlink_ext_ack *extack)
{
	struct rtl837x_priv *priv = ds->priv;
	struct rtl837x_vlan_entry ve = { 0 };
	bool untagged = !!(vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED);
	bool pvid = !!(vlan->flags & BRIDGE_VLAN_INFO_PVID);
	int ret;

	(void)extack;
	rtl837x_vlan_read(priv, vlan->vid, &ve);
	ve.vid = vlan->vid;
	ve.member |= BIT(port) | BIT(priv->cpu_port);
	if (untagged)
		ve.untag |= BIT(port);
	else
		ve.untag &= ~BIT(port);
	ve.ivl_en = true;

	ret = rtl837x_vlan_write(priv, &ve);
	if (ret)
		return ret;

	if (pvid)
		return rtl837x_vlan_set_pvid(priv, port, vlan->vid);
	return 0;
}

static int rtl8372n_port_vlan_del(struct dsa_switch *ds, int port,
				  const struct switchdev_obj_port_vlan *vlan)
{
	struct rtl837x_priv *priv = ds->priv;
	struct rtl837x_vlan_entry ve = { 0 };
	int ret;

	ret = rtl837x_vlan_read(priv, vlan->vid, &ve);
	if (ret)
		return ret;

	ve.member &= ~BIT(port);
	ve.untag &= ~BIT(port);
	if (!(ve.member & ~BIT(priv->cpu_port)))
		ve.member = 0;

	return rtl837x_vlan_write(priv, &ve);
}

static int rtl8372n_tag_8021q_vlan_add(struct dsa_switch *ds, int port,
				       u16 vid, u16 flags)
{
	struct rtl837x_priv *priv = ds->priv;
	struct rtl837x_vlan_entry ve = { 0 };
	u32 shift = (port & 1) ? 16 : 0;
	int ret;

	rtl837x_vlan_read(priv, vid, &ve);
	ve.vid = vid;
	ve.member |= BIT(port) | BIT(priv->cpu_port);
	if (dsa_is_user_port(ds, port))
		ve.untag |= BIT(port);
	ve.ivl_en = true;

	ret = rtl837x_vlan_write(priv, &ve);
	if (ret)
		return ret;

	if (flags & BRIDGE_VLAN_INFO_PVID) {
		regmap_update_bits(priv->map,
				   RTL837X_VS_PORT_DFLT_SVID_REG(port),
				   0xfffU << shift,
				   ((u32)vid & 0xfffU) << shift);
		rtl837x_vlan_set_pvid(priv, port, vid);
	}

	priv->dsa_8021q_vid[vid] = true;
	return 0;
}

static int rtl8372n_tag_8021q_vlan_del(struct dsa_switch *ds, int port, u16 vid)
{
	struct rtl837x_priv *priv = ds->priv;
	struct rtl837x_vlan_entry ve = { 0 };
	u32 shift = (port & 1) ? 16 : 0;

	rtl837x_vlan_read(priv, vid, &ve);
	ve.member &= ~BIT(port);
	ve.untag &= ~BIT(port);
	if (!(ve.member & ~BIT(priv->cpu_port))) {
		ve.member = 0;
		priv->dsa_8021q_vid[vid] = false;
	}
	rtl837x_vlan_write(priv, &ve);

	if (dsa_is_user_port(ds, port))
		regmap_update_bits(priv->map,
				   RTL837X_VS_PORT_DFLT_SVID_REG(port),
				   0xfffU << shift, 0);
	return 0;
}

static int rtl8372n_port_fdb_add(struct dsa_switch *ds, int port,
				 const unsigned char *addr, u16 vid,
				 struct dsa_db db)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)db;
	if (!vid && priv->tag_proto == DSA_TAG_PROTO_RTL837X_8021Q)
		vid = dsa_tag_8021q_standalone_vid(dsa_to_port(ds, port));
	return rtl837x_fdb_modify(priv, port, addr, vid, true);
}

static int rtl8372n_port_fdb_del(struct dsa_switch *ds, int port,
				 const unsigned char *addr, u16 vid,
				 struct dsa_db db)
{
	struct rtl837x_priv *priv = ds->priv;

	(void)db;
	if (!vid && priv->tag_proto == DSA_TAG_PROTO_RTL837X_8021Q)
		vid = dsa_tag_8021q_standalone_vid(dsa_to_port(ds, port));
	return rtl837x_fdb_modify(priv, port, addr, vid, false);
}

static int rtl8372n_port_fdb_dump(struct dsa_switch *ds, int port,
				  dsa_fdb_dump_cb_t *cb, void *data)
{
	return rtl837x_fdb_dump_port(ds->priv, port, cb, data);
}

static int rtl8372n_port_mdb_add(struct dsa_switch *ds, int port,
				 const struct switchdev_obj_port_mdb *mdb,
				 struct dsa_db db)
{
	(void)db;
	return rtl837x_mdb_modify(ds->priv, port, mdb->addr, mdb->vid, true);
}

static int rtl8372n_port_mdb_del(struct dsa_switch *ds, int port,
				 const struct switchdev_obj_port_mdb *mdb,
				 struct dsa_db db)
{
	(void)db;
	return rtl837x_mdb_modify(ds->priv, port, mdb->addr, mdb->vid, false);
}

static void rtl8372n_get_stats64(struct dsa_switch *ds, int port,
				 struct rtnl_link_stats64 *s)
{
	struct rtl837x_priv *priv = ds->priv;

	memset(s, 0, sizeof(*s));
	if (port < 0 || port >= priv->num_ports)
		return;

	spin_lock_bh(&priv->stats_lock);
	s->rx_packets = priv->ports[port].rx_pkts;
	s->tx_packets = priv->ports[port].tx_pkts;
	s->rx_bytes = priv->ports[port].rx_bytes;
	s->tx_bytes = priv->ports[port].tx_bytes;
	s->rx_dropped = priv->ports[port].rx_drops;
	spin_unlock_bh(&priv->stats_lock);
}

static int rtl8372n_set_mac_eee(struct dsa_switch *ds, int port,
				struct ethtool_keee *e)
{
	struct rtl837x_priv *priv = ds->priv;

	return regmap_write(priv->map, RTL837X_PORT_EEE_CTRL_REG(port),
			    e->eee_enabled ? 0x3 : 0x0);
}

static const struct dsa_switch_ops rtl8372n_dsa_ops = {
	.get_tag_protocol	= rtl8372n_get_tag_protocol,
	.change_tag_protocol	= rtl8372n_change_tag_protocol,
	.setup			= rtl8372n_setup,
	.teardown		= rtl8372n_teardown,
	.phylink_get_caps	= rtl8372n_phylink_get_caps,
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 10, 0)
	.phylink_mac_select_pcs	= rtl8372n_phylink_mac_select_pcs,
	.phylink_mac_config	= rtl8372n_phylink_mac_config,
	.phylink_mac_link_down	= rtl8372n_phylink_mac_link_down,
	.phylink_mac_link_up	= rtl8372n_phylink_mac_link_up,
#endif
	.port_max_mtu		= rtl8372n_port_max_mtu,
	.port_change_mtu	= rtl8372n_port_change_mtu,
	.port_bridge_join	= rtl8372n_port_bridge_join,
	.port_bridge_leave	= rtl8372n_port_bridge_leave,
	.port_stp_state_set	= rtl8372n_port_stp_state_set,
	.port_fast_age		= rtl8372n_port_fast_age,
	.set_ageing_time	= rtl8372n_set_ageing_time,
	.port_pre_bridge_flags	= rtl8372n_port_pre_bridge_flags,
	.port_bridge_flags	= rtl8372n_port_bridge_flags,
	.port_vlan_filtering	= rtl8372n_port_vlan_filtering,
	.port_vlan_add		= rtl8372n_port_vlan_add,
	.port_vlan_del		= rtl8372n_port_vlan_del,
	.tag_8021q_vlan_add	= rtl8372n_tag_8021q_vlan_add,
	.tag_8021q_vlan_del	= rtl8372n_tag_8021q_vlan_del,
	.port_fdb_add		= rtl8372n_port_fdb_add,
	.port_fdb_del		= rtl8372n_port_fdb_del,
	.port_fdb_dump		= rtl8372n_port_fdb_dump,
	.port_mdb_add		= rtl8372n_port_mdb_add,
	.port_mdb_del		= rtl8372n_port_mdb_del,
	.get_stats64		= rtl8372n_get_stats64,
	.set_mac_eee		= rtl8372n_set_mac_eee,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 12, 44)
	.support_eee		= dsa_supports_eee,
#endif
};

int rtl8372n_switch_detect_and_register(struct rtl837x_priv *priv)
{
	u32 id_reg = 0, info_reg = 0;
	struct dsa_switch *ds;
	int ret;

	ret = regmap_read(priv->map, RTL837X_REG_CHIP_ID, &id_reg);
	if (ret)
		return ret;
	ret = regmap_read(priv->map, RTL837X_REG_CHIP_INFO, &info_reg);
	if (ret)
		return ret;

	priv->chip_model = (u16)FIELD_GET(RTL837X_CHIP_ID_MODEL_MASK, id_reg);
	priv->is_n_series = (FIELD_GET(RTL837X_CHIP_SUB_N_MASK, info_reg) ==
			     RTL837X_CHIP_SUB_IS_N);

	if (priv->chip_model != RTL837X_CHIP_ID_8372 &&
	    priv->chip_model != RTL837X_CHIP_ID_8373) {
		dev_err(priv->dev, "Unsupported Realtek switch ID: 0x%08x\n",
			id_reg);
		return -ENODEV;
	}

	priv->num_ports = RTL837X_MAX_PORTS;
	/* Default to 802.1Q SVLAN tagger so MT7988A GMAC2 CSUM + PPE HNAT work */
	priv->tag_proto = DSA_TAG_PROTO_RTL837X_8021Q;

	dev_info(priv->dev, "Found Realtek RTL%04x%s switch (rev 0x%04x)\n",
		 priv->chip_model, priv->is_n_series ? "N" : "",
		 (u16)(info_reg & 0xffff));

	ds = devm_kzalloc(priv->dev, sizeof(*ds), GFP_KERNEL);
	if (!ds)
		return -ENOMEM;

	ds->dev = priv->dev;
	ds->num_ports = priv->num_ports;
	ds->priv = priv;
	ds->ops = &rtl8372n_dsa_ops;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
	ds->phylink_mac_ops = &rtl8372n_phylink_mac_ops;
#endif
	priv->ds = ds;

	ret = dsa_register_switch(ds);
	if (ret) {
		priv->ds = NULL;
		return ret;
	}

	return 0;
}

void rtl8372n_switch_teardown_and_unregister(struct rtl837x_priv *priv)
{
	if (!priv->ds)
		return;
	WRITE_ONCE(priv->stopping, true);
	dsa_unregister_switch(priv->ds);
	priv->ds = NULL;
}
