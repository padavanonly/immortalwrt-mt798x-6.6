// SPDX-License-Identifier: GPL-2.0-only
/*
 * IEEE 802.1Q SVLAN DSA tagger for Realtek RTL8372N/RTL8373 on Linux 6.6
 *
 * Uses standard 0x8100 VLAN tags so MT7988A GMAC2 preserves hardware TX/RX
 * checksum + TSO offloads and MediaTek PPE HNAT parses/rewrites per-port tags
 * in silicon at 10Gbps line rate.
 */
#include "rtl837x.h"

static struct sk_buff *rtl837x_8021q_tag_xmit(struct sk_buff *skb,
					      struct net_device *netdev)
{
	struct dsa_port *dp = dsa_slave_to_port(netdev);
	u16 tx_vid = dsa_tag_8021q_standalone_vid(dp);
	u16 queue_mapping = skb_get_queue_mapping(skb);
	u8 pcp = netdev_txq_to_tc(netdev, queue_mapping);

	skb_set_queue_mapping(skb, dp->index);

	return dsa_8021q_xmit(skb, netdev, ETH_P_8021Q,
			      ((u16)(pcp & 0x7) << VLAN_PRIO_SHIFT) | tx_vid);
}

static struct sk_buff *rtl837x_8021q_tag_rcv(struct sk_buff *skb,
					     struct net_device *netdev)
{
	int src_port = -1, switch_id = -1, vbid = 0;

	dsa_8021q_rcv(skb, &src_port, &switch_id, &vbid);
	if (src_port < 0 || switch_id < 0)
		return NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 7, 0)
	skb->dev = dsa_master_find_slave(netdev, switch_id, src_port);
#else
	skb->dev = dsa_conduit_find_user(netdev, switch_id, src_port);
#endif
	if (!skb->dev)
		return NULL;

	if (!vbid && !is_link_local_ether_addr(eth_hdr(skb)->h_dest))
		dsa_default_offload_fwd_mark(skb);

	return skb;
}

static const struct dsa_device_ops rtl837x_8021q_netdev_ops = {
	.name			= RTL837X_TAG_8021Q_NAME,
	.proto			= DSA_TAG_PROTO_RTL837X_8021Q,
	.xmit			= rtl837x_8021q_tag_xmit,
	.rcv			= rtl837x_8021q_tag_rcv,
	.needed_headroom	= VLAN_HLEN,
	.promisc_on_master	= true,
};

static struct dsa_tag_driver rtl837x_8021q_tag_drv = {
	.ops = &rtl837x_8021q_netdev_ops,
	.owner = THIS_MODULE,
};

static struct dsa_tag_driver *rtl837x_tag_drv_array[] = {
	&rtl837x_8021q_tag_drv,
};

int rtl837x_tag_8021q_driver_register(void)
{
	dsa_tag_drivers_register(rtl837x_tag_drv_array,
				 ARRAY_SIZE(rtl837x_tag_drv_array),
				 THIS_MODULE);
	return 0;
}

void rtl837x_tag_8021q_driver_unregister(void)
{
	dsa_tag_drivers_unregister(rtl837x_tag_drv_array,
				   ARRAY_SIZE(rtl837x_tag_drv_array));
}

MODULE_ALIAS("dsa_tag-" RTL837X_TAG_8021Q_NAME);
MODULE_ALIAS("dsa_tag-id-31");
MODULE_ALIAS(DSA_TAG_DRIVER_ALIAS RTL837X_TAG_8021Q_NAME);
MODULE_ALIAS(DSA_TAG_DRIVER_ALIAS "id-31");
