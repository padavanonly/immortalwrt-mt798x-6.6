// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <net/ip6_route.h>

#include <mtk_eth_soc.h>

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
#include <mtk_hnat/hnat.h>
#include <mtk_hnat/nf_hnat_mtk.h>
#elif defined(CONFIG_CRYPTO_OFFLOAD_INLINE_FLOWBLOCK)
#include <mtk_ppe.h>
#endif

#include <pce/cdrt.h>
#include <pce/cls.h>
#include <pce/netsys.h>

#include <crypto-eip/ddk/kit/iotoken/iotoken.h>
#include <crypto-eip/ddk/kit/iotoken/iotoken_ext.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/internal.h"

static struct xfrm_params_list xfrm_params_list = {
	.list = LIST_HEAD_INIT(xfrm_params_list.list),
	.lock = __SPIN_LOCK_UNLOCKED(xfrm_params_list.lock),
};

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);

static inline bool is_tops_tunnel(struct sk_buff *skb)
{
	return skb_hnat_tops(skb) && (ntohs(skb->protocol) == ETH_P_IP) &&
		(ip_hdr(skb)->protocol == IPPROTO_UDP ||
		 ip_hdr(skb)->protocol == IPPROTO_GRE);
}

static inline bool is_hnat_rate_reach(struct sk_buff *skb)
{
	return is_magic_tag_valid(skb) && (skb_hnat_reason(skb) == HIT_UNBIND_RATE_REACH);
}
#endif // HNAT

#if defined(CONFIG_CRYPTO_OFFLOAD_INLINE_FLOWBLOCK)
int mtk_crypto_xfrm_get_cdrt(struct xfrm_state *xs)
{
	struct mtk_xfrm_params *xfrm_params;
	struct mtk_xfrm_params *temp;

	xfrm_params = (struct mtk_xfrm_params *)xs->xso.offload_handle;

	if (!xfrm_params)
		return 0;

	spin_lock_bh(&xfrm_params_list.lock);
	list_for_each_entry(temp, &xfrm_params_list.list, node) {
		if (xfrm_params == temp) {
			spin_unlock_bh(&xfrm_params_list.lock);
			return xfrm_params->cdrt->idx;
		}
	}
	spin_unlock_bh(&xfrm_params_list.lock);

	return 0;
}
#endif // Flow_Block

struct xfrm_params_list *mtk_xfrm_params_list_get(void)
{
	return &xfrm_params_list;
}

static void mtk_xfrm_offload_cdrt_tear_down(struct mtk_xfrm_params *xfrm_params)
{
	if (!xfrm_params->cdrt)
		return;

	memset(&xfrm_params->cdrt->desc, 0, sizeof(struct cdrt_desc));

	mtk_pce_cdrt_entry_write(xfrm_params->cdrt);
}

static int mtk_xfrm_offload_cdrt_setup(struct mtk_xfrm_params *xfrm_params)
{
	struct cdrt_desc *cdesc = &xfrm_params->cdrt->desc;

	cdesc->desc1.common.type = 3;
	cdesc->desc1.token_len = 48;
	/* All use small transform record now */
	cdesc->desc1.p_tr[0] = __pa(xfrm_params->p_tr) | xfrm_params->tr_type;
	cdesc->desc1.p_tr[1] = __pa(xfrm_params->p_tr) >> 32 & 0xFFFFFFFF;

	cdesc->desc2.hw_srv = 3;
	cdesc->desc2.allow_pad = 1;
	cdesc->desc2.strip_pad = 1;

	return mtk_pce_cdrt_entry_write(xfrm_params->cdrt);
}

static void mtk_xfrm_offload_cls_entry_tear_down(struct mtk_xfrm_params *xfrm_params)
{
	if (!xfrm_params->cdrt || !xfrm_params->cdrt->cls)
		return;

	memset(&xfrm_params->cdrt->cls->cdesc, 0, sizeof(struct cls_desc));

	mtk_pce_cls_entry_write(xfrm_params->cdrt->cls);

	mtk_pce_cls_entry_free(xfrm_params->cdrt->cls);
}

static int mtk_xfrm_offload_cls_entry_setup(struct mtk_xfrm_params *xfrm_params)
{
	struct cls_desc *cdesc;
	struct xfrm_state *xs = xfrm_params->xs;
	u32 ppe_port;

	xfrm_params->cdrt->cls = mtk_pce_cls_entry_alloc();
	if (IS_ERR(xfrm_params->cdrt->cls))
		return PTR_ERR(xfrm_params->cdrt->cls);

	cdesc = &xfrm_params->cdrt->cls->cdesc;

	if (mac_filter_enable) {
		CLS_DESC_DATA(cdesc, fport, PSE_PORT_TDMA);
		CLS_DESC_DATA(cdesc, tport_idx, 0x3);
	} else {
		ppe_port = mtk_crypto_ppe_get_num(xs->xso.dev);
		CLS_DESC_DATA(cdesc, fport, ppe_port);
		CLS_DESC_DATA(cdesc, tport_idx, 0x2);
	}
	CLS_DESC_DATA(cdesc, cdrt_idx, xfrm_params->cdrt->idx);

	if (xs->encap) {
		CLS_DESC_MASK_DATA(cdesc, tag,
			   CLS_DESC_TAG_MASK, CLS_DESC_TAG_MATCH_L4_USR);
		CLS_DESC_MASK_DATA(cdesc, l4_type,
				CLS_DESC_L4_TYPE_MASK, IPPROTO_UDP);
		CLS_DESC_MASK_DATA(cdesc, l4_valid,
			   CLS_DESC_L4_VALID_MASK,
			   CLS_DESC_VALID_UPPER_HALF_WORD_BIT |
			   CLS_DESC_VALID_LOWER_HALF_WORD_BIT |
			   CLS_DESC_VALID_DPORT_BIT);
		CLS_DESC_MASK_DATA(cdesc, l4_dport, CLS_DESC_L4_DPORT_MASK,
							be16_to_cpu(xs->encap->encap_dport));
	} else {
		CLS_DESC_MASK_DATA(cdesc, tag,
			   CLS_DESC_TAG_MASK, CLS_DESC_TAG_MATCH_L4_HDR);
		CLS_DESC_MASK_DATA(cdesc, l4_udp_hdr_nez,
			   CLS_DESC_UDPLITE_L4_HDR_NEZ_MASK,
			   CLS_DESC_UDPLITE_L4_HDR_NEZ_MASK);
		CLS_DESC_MASK_DATA(cdesc, l4_type,
			   CLS_DESC_L4_TYPE_MASK, IPPROTO_ESP);
		CLS_DESC_MASK_DATA(cdesc, l4_valid,
			   0x3,
			   CLS_DESC_VALID_UPPER_HALF_WORD_BIT |
			   CLS_DESC_VALID_LOWER_HALF_WORD_BIT);
	}

	CLS_DESC_MASK_DATA(cdesc, l4_hdr_usr_data,
					0xFFFFFFFF, be32_to_cpu(xfrm_params->xs->id.spi));

	return mtk_pce_cls_entry_write(xfrm_params->cdrt->cls);
}

static void mtk_xfrm_offload_context_tear_down(struct mtk_xfrm_params *xfrm_params)
{
	mtk_xfrm_offload_cdrt_tear_down(xfrm_params);
	mtk_ddk_invalidate_rec((void *) xfrm_params->p_handle, true);
	crypto_free_sa((void *) xfrm_params->p_handle, 0);
}

static int mtk_xfrm_offload_context_setup(struct mtk_xfrm_params *xfrm_params)
{
	void *tr;
	int ret;

	switch (xfrm_params->xs->outer_mode.encap) {
	case XFRM_MODE_TUNNEL:
		tr = mtk_ddk_tr_ipsec_build(xfrm_params, SAB_IPSEC_TUNNEL);
		break;
	case XFRM_MODE_TRANSPORT:
		tr = mtk_ddk_tr_ipsec_build(xfrm_params, SAB_IPSEC_TRANSPORT);
		break;
	default:
		ret = -ENOMEM;
		goto err_out;
	}

	if (!tr) {
		ret = -EINVAL;
		goto err_out;
	}

	xfrm_params->p_tr = tr;

	return mtk_xfrm_offload_cdrt_setup(xfrm_params);

err_out:
	return ret;
}

static int mtk_xfrm_offload_state_add_outbound(struct xfrm_state *xs,
					       struct mtk_xfrm_params *xfrm_params)
{
	int ret;

	xfrm_params->cdrt = mtk_pce_cdrt_entry_alloc(CDRT_ENCRYPT);
	if (IS_ERR(xfrm_params->cdrt))
		return PTR_ERR(xfrm_params->cdrt);

	xfrm_params->dir = SAB_DIRECTION_OUTBOUND;

	ret = mtk_xfrm_offload_context_setup(xfrm_params);
	if (ret)
		goto free_cdrt;

	return ret;

free_cdrt:
	mtk_pce_cdrt_entry_free(xfrm_params->cdrt);

	return ret;
}

static int mtk_xfrm_offload_state_add_inbound(struct xfrm_state *xs,
					      struct mtk_xfrm_params *xfrm_params)
{
	int ret;

	xfrm_params->cdrt = mtk_pce_cdrt_entry_alloc(CDRT_DECRYPT);
	if (IS_ERR(xfrm_params->cdrt))
		return PTR_ERR(xfrm_params->cdrt);

	xfrm_params->dir = SAB_DIRECTION_INBOUND;

	ret = mtk_xfrm_offload_context_setup(xfrm_params);
	if (ret)
		goto free_cdrt;

	ret = mtk_xfrm_offload_cls_entry_setup(xfrm_params);
	if (ret)
		goto tear_down_context;

	return ret;

tear_down_context:
	mtk_xfrm_offload_context_tear_down(xfrm_params);

free_cdrt:
	mtk_pce_cdrt_entry_free(xfrm_params->cdrt);

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
int mtk_xfrm_offload_state_add(struct xfrm_state *xs, struct netlink_ext_ack *extack)
#else
int mtk_xfrm_offload_state_add(struct xfrm_state *xs)
#endif
{
	struct mtk_xfrm_params *xfrm_params;
	int ret = 0;

	/* TODO: maybe support IPv6 in the future? */
	if (xs->props.family != AF_INET && xs->props.family != AF_INET6) {
		CRYPTO_NOTICE("Only IPv4 and IPv6 xfrm states may be offloaded\n");
		return -EINVAL;
	}

	/* only support ESP right now */
	if (xs->id.proto != IPPROTO_ESP) {
		CRYPTO_NOTICE("Unsupported protocol 0x%04x\n", xs->id.proto);
		return -EINVAL;
	}

	/* only support tunnel mode or transport mode */
	if (!(xs->outer_mode.encap == XFRM_MODE_TUNNEL
	    || xs->outer_mode.encap == XFRM_MODE_TRANSPORT)) {
		CRYPTO_NOTICE("Unsupported outer encapsulation type %u\n", xs->outer_mode.encap);
		return -EINVAL;
	}

	xfrm_params = devm_kzalloc(crypto_dev,
				   sizeof(struct mtk_xfrm_params),
				   GFP_KERNEL);
	if (!xfrm_params)
		return -ENOMEM;

	xfrm_params->xs = xs;
	INIT_LIST_HEAD(&xfrm_params->node);

#if KERNEL_VERSION(6, 3, 0) < LINUX_VERSION_CODE
	if (xs->xso.dir == XFRM_DEV_OFFLOAD_IN)
#else
	if (xs->xso.flags & XFRM_OFFLOAD_INBOUND)
#endif
		/* rx path */
		ret = mtk_xfrm_offload_state_add_inbound(xs, xfrm_params);
	else
		/* tx path */
		ret = mtk_xfrm_offload_state_add_outbound(xs, xfrm_params);

	if (ret) {
		devm_kfree(crypto_dev, xfrm_params);
		goto out;
	}

	xs->xso.offload_handle = (unsigned long)xfrm_params;

#if KERNEL_VERSION(6, 3, 0) < LINUX_VERSION_CODE
	memcpy(extack->cookie, &xfrm_params->cdrt->idx, sizeof(xfrm_params->cdrt->idx));
	extack->cookie_len = sizeof(xfrm_params->cdrt->idx);
#endif

	spin_lock_bh(&xfrm_params_list.lock);

	list_add_tail(&xfrm_params->node, &xfrm_params_list.list);

	spin_unlock_bh(&xfrm_params_list.lock);

out:
	return ret;
}

void mtk_xfrm_offload_state_delete(struct xfrm_state *xs)
{
}

void mtk_xfrm_offload_state_free(struct xfrm_state *xs)
{
	struct mtk_xfrm_params *xfrm_params;

	if (!xs->xso.offload_handle)
		return;

	xfrm_params = (struct mtk_xfrm_params *)xs->xso.offload_handle;
	xs->xso.offload_handle = 0;

	spin_lock_bh(&xfrm_params_list.lock);
	list_del(&xfrm_params->node);
	spin_unlock_bh(&xfrm_params_list.lock);

#if KERNEL_VERSION(6, 3, 0) < LINUX_VERSION_CODE
	if (xs->xso.dir == XFRM_DEV_OFFLOAD_IN)
#else
	if (xs->xso.flags & XFRM_OFFLOAD_INBOUND)
#endif
		mtk_xfrm_offload_cls_entry_tear_down(xfrm_params);

	if (xfrm_params->cdrt)
		mtk_pce_cdrt_entry_free(xfrm_params->cdrt);

	mtk_xfrm_offload_context_tear_down(xfrm_params);

	devm_kfree(crypto_dev, xfrm_params);
}

void mtk_xfrm_offload_state_tear_down(void)
{
	struct mtk_xfrm_params *xfrm_params, *tmp;

	spin_lock_bh(&xfrm_params_list.lock);

	list_for_each_entry_safe(xfrm_params, tmp, &xfrm_params_list.list, node)
		mtk_xfrm_offload_state_free(xfrm_params->xs);

	spin_unlock_bh(&xfrm_params_list.lock);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
int mtk_xfrm_offload_policy_add(struct xfrm_policy *xp, struct netlink_ext_ack *extack)
#else
int mtk_xfrm_offload_policy_add(struct xfrm_policy *xp)
#endif
{
	struct mtk_xfrm_params *xfrm_params;
	struct xfrm_state *xs;
	struct xfrm_tmpl xt = xp->xfrm_vec[0];

	if (xt.id.spi) {
		xs = xfrm_state_lookup_byspi(&init_net, xt.id.spi, xt.encap_family);
		if (!xs)
			return 0;

		xfrm_params = (struct mtk_xfrm_params *)xs->xso.offload_handle;
		if (!xfrm_params)
			return 0;
		xp->xdo.offload_handle = (unsigned long) xfrm_params;
		xfrm_state_put(xs);
	}

	return 0;
}

void mtk_xfrm_offload_policy_delete(struct xfrm_policy *xp)
{
#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT) || defined(CONFIG_CRYPTO_OFFLOAD_INLINE_FLOWBLOCK)
	struct mtk_xfrm_params *xfrm_params;

	if (!xp->xdo.offload_handle)
		return;

	xfrm_params = (struct mtk_xfrm_params *)xp->xdo.offload_handle;
	xp->xdo.offload_handle = 0;

	if (xfrm_params->cdrt)
#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
		foe_clear_crypto_entry(xfrm_params->cdrt->idx);
#elif defined(CONFIG_CRYPTO_OFFLOAD_INLINE_FLOWBLOCK)
		mtk_flow_offload_teardown_by_tnl(mcrypto.eth, MTK_FOE_CRYPTO_DEL,
						 xfrm_params->cdrt->idx);
#endif

	return;
#endif // hnat or flowblock
}

void mtk_xfrm_offload_policy_free(struct xfrm_policy *xp)
{
}

static inline struct neighbour *mtk_crypto_find_ipv6_dst_mac(struct sk_buff *skb,
					struct xfrm_state *xs)
{
	struct neighbour *neigh;
	struct dst_entry *dst = skb_dst(skb);
	struct rt6_info *rt = (struct rt6_info *) dst;
	struct sk_buff *skbn;
	const struct in6_addr *nexthop;

	if (!netif_carrier_ok(dst->dev) || !(dst->dev->flags & IFF_UP))
		return NULL;

	nexthop = rt6_nexthop(rt, (struct in6_addr *) &xs->id.daddr.a6);
	neigh = __ipv6_neigh_lookup_noref(dst->dev, nexthop);
	if (unlikely(!neigh)) {
		CRYPTO_INFO("%s: %s No neigh (daddr=%pI6)\n", __func__, dst->dev->name,
				nexthop);
		neigh = __neigh_create(&nd_tbl, nexthop, dst->dev, false);
		if (IS_ERR(neigh))
			return NULL;

		skbn = skb_clone(skb, GFP_ATOMIC);
		if (skbn)
			neigh_output(neigh, skbn, false);
		return NULL;
	}

	if (is_zero_ether_addr(neigh->ha)) {
		neigh = __ipv6_neigh_lookup_noref(dst->dev, &xs->id.daddr.a6);
		if (unlikely(!neigh)) {
			CRYPTO_INFO("%s: %s No neigh (daddr=%pI6)\n", __func__, dst->dev->name,
					&xs->id.daddr.a6);
			neigh = __neigh_create(&nd_tbl, &xs->id.daddr.a6, dst->dev, false);
			if (IS_ERR(neigh))
				return NULL;

			skbn = skb_clone(skb, GFP_ATOMIC);
			if (skbn)
				neigh_output(neigh, skbn, false);
			return NULL;
		}
	}

	/* For the case that neighbour is found, but no MAC address is set */
	if (is_zero_ether_addr(neigh->ha))
		return NULL;

	return neigh;
}

static inline struct neighbour *mtk_crypto_find_ipv4_dst_mac(struct sk_buff *skb,
					struct xfrm_state *xs)
{
	struct neighbour *neigh;
	struct dst_entry *dst = skb_dst(skb);
	struct rtable *rt = (struct rtable *) dst;
	struct sk_buff *skbn;
	__be32 nexthop;

	if (!netif_carrier_ok(dst->dev) || !(dst->dev->flags & IFF_UP))
		return NULL;

	/*
	 * First get the nexthop from the routing table.
	 * Then try to get neighbour for MAC address.
	 */
	nexthop = rt_nexthop(rt, xs->id.daddr.a4);
	neigh = __ipv4_neigh_lookup_noref(dst->dev, nexthop);
	if (unlikely(!neigh)) {
		CRYPTO_INFO("%s: %s No neigh (nexthop=%pI4)\n", __func__, dst->dev->name,
			&nexthop);
		neigh = __neigh_create(&arp_tbl, &nexthop, dst->dev, false);
		if (IS_ERR(neigh))
			return NULL;

		skbn = skb_clone(skb, GFP_ATOMIC);
		if (skbn)
			neigh_output(neigh, skbn, false);
		return NULL;
	}

	/*
	 * If the nexthop has no dst MAC, arp may failed on that IP.
	 * Use tunnel destination IP and try to find MAC address again.
	 */
	if (is_zero_ether_addr(neigh->ha)) {
		neigh = __ipv4_neigh_lookup_noref(dst->dev, xs->id.daddr.a4);
		if (unlikely(!neigh)) {
			CRYPTO_INFO("%s: %s No neigh (nexthop=%pI4)\n", __func__, dst->dev->name,
						&xs->id.daddr.a4);
			neigh = __neigh_create(&arp_tbl, &xs->id.daddr.a4, dst->dev, false);
			if (IS_ERR(neigh))
				return NULL;

			skbn = skb_clone(skb, GFP_ATOMIC);
			if (skbn)
				neigh_output(neigh, skbn, false);
			return NULL;
		}
	}

	/* For the case that neighbour is found, but no MAC address is set */
	if (is_zero_ether_addr(neigh->ha))
		return NULL;

	return neigh;
}

bool mtk_xfrm_offload_ok(struct sk_buff *skb,
			 struct xfrm_state *xs)
{
	struct mtk_xfrm_params *xfrm_params;
	struct neighbour *neigh;
	struct dst_entry *dst = skb_dst(skb);
	int fill_inner_info = 0;

	if (dst->dev->type != ARPHRD_PPP) {
		rcu_read_lock_bh();

		if (xs->props.family == AF_INET)
			neigh = mtk_crypto_find_ipv4_dst_mac(skb, xs);
		else
			neigh = mtk_crypto_find_ipv6_dst_mac(skb, xs);
		if (!neigh) {
			rcu_read_unlock_bh();
			/* This is just a workaround to mark the packet which needs to be free */
			skb->inner_protocol = IPPROTO_RSVP;
			return true;
		}

		/*
		* For packet has pass through VTI (route-based VTI)
		* The 'dev_queue_xmit' function called at network layer will cause both
		* skb->mac_header and skb->network_header to point to the IP header
		*/
		if (skb->mac_header == skb->network_header)
			fill_inner_info = 1;

		skb_push(skb, sizeof(struct ethhdr));
		skb_reset_mac_header(skb);

		if (xs->props.family == AF_INET)
			eth_hdr(skb)->h_proto = htons(ETH_P_IP);
		else
			eth_hdr(skb)->h_proto = htons(ETH_P_IPV6);
		memcpy(eth_hdr(skb)->h_dest, neigh->ha, ETH_ALEN);
		memcpy(eth_hdr(skb)->h_source, dst->dev->dev_addr, ETH_ALEN);

		rcu_read_unlock_bh();
	}

	xfrm_params = (struct mtk_xfrm_params *)xs->xso.offload_handle;

	/* Make sure we have enough skb headroom for cdrt_idx */
	if (skb_cow_head(skb, NET_SKB_PAD + NET_IP_ALIGN))
		return true;

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
	skb_hnat_cdrt(skb) = xfrm_params->cdrt->idx;

	if (ra_sw_nat_hook_tx && is_hnat_rate_reach(skb))
		hnat_bind_crypto_entry(skb, dst->dev, fill_inner_info);

	/* Set magic tag for tport setting, reset to 0 after tport is set */
	skb_hnat_magic_tag(skb) = HNAT_MAGIC_TAG;
#else
	skb_tnl_cdrt(skb) = xfrm_params->cdrt->idx;
	skb_tnl_magic_tag(skb) = TNL_MAGIC_TAG;
#endif // HNAT

	atomic64_add(skb->len - ETH_HLEN, &xfrm_params->bytes);
	atomic64_inc(&xfrm_params->packets);

	/*
	 * Since skb headroom may not be copy when segment, we cannot rely on
	 * headroom data (ex. cdrt) to decide packets should send to EIP197.
	 * Here is a workaround that only skb with inner_protocol = ESP will
	 * be sent to EIP197.
	 */
	skb->inner_protocol = IPPROTO_ESP;

	/* Since we're going to tx directly, set skb->dev to dst->dev */
	skb->dev = dst->dev;

	/*
	 * Tx packet to EIP197.
	 * To avoid conflict of SW and HW sequence number
	 * All offloadable packets send to EIP197
	 */
	dev_queue_xmit(skb);

	return true;
}
