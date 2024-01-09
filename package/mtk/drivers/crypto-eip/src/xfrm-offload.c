// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/bitops.h>

#include <mtk_eth_soc.h>
#include <mtk_hnat/hnat.h>
#include <mtk_hnat/nf_hnat_mtk.h>

#include <pce/cdrt.h>
#include <pce/cls.h>
#include <pce/netsys.h>

#include <crypto-eip/ddk/configs/cs_hwpal_ext.h>
#include <crypto-eip/ddk/kit/iotoken/iotoken.h>
#include <crypto-eip/ddk/kit/iotoken/iotoken_ext.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/internal.h"

static LIST_HEAD(xfrm_params_head);

static inline bool is_tops_udp_tunnel(struct sk_buff *skb)
{
	return skb_hnat_tops(skb) && (ntohs(skb->protocol) == ETH_P_IP) &&
			(ip_hdr(skb)->protocol == IPPROTO_UDP);
}

static inline bool is_tcp(struct sk_buff *skb)
{
	return (ntohs(skb->protocol) == ETH_P_IP) && (ip_hdr(skb)->protocol == IPPROTO_TCP);
}

static inline bool is_hnat_rate_reach(struct sk_buff *skb)
{
	return is_magic_tag_valid(skb) && (skb_hnat_reason(skb) == HIT_UNBIND_RATE_REACH);
}

static void mtk_xfrm_offload_cdrt_tear_down(struct mtk_xfrm_params *xfrm_params)
{
	memset(&xfrm_params->cdrt->desc, 0, sizeof(struct cdrt_desc));

	mtk_pce_cdrt_entry_write(xfrm_params->cdrt);
}

static int mtk_xfrm_offload_cdrt_setup(struct mtk_xfrm_params *xfrm_params)
{
	struct cdrt_desc *cdesc = &xfrm_params->cdrt->desc;

	cdesc->desc1.common.type = 3;
	cdesc->desc1.token_len = 48;
	cdesc->desc1.p_tr[0] = __pa(xfrm_params->p_tr) | 2;

	cdesc->desc2.hw_srv = 2;
	cdesc->desc2.allow_pad = 1;
	cdesc->desc2.strip_pad = 1;

	return mtk_pce_cdrt_entry_write(xfrm_params->cdrt);
}

static void mtk_xfrm_offload_cls_entry_tear_down(struct mtk_xfrm_params *xfrm_params)
{
	memset(&xfrm_params->cdrt->cls->cdesc, 0, sizeof(struct cls_desc));

	mtk_pce_cls_entry_write(xfrm_params->cdrt->cls);

	mtk_pce_cls_entry_free(xfrm_params->cdrt->cls);
}

static int mtk_xfrm_offload_cls_entry_setup(struct mtk_xfrm_params *xfrm_params)
{
	struct cls_desc *cdesc;

	xfrm_params->cdrt->cls = mtk_pce_cls_entry_alloc();
	if (IS_ERR(xfrm_params->cdrt->cls))
		return PTR_ERR(xfrm_params->cdrt->cls);

	cdesc = &xfrm_params->cdrt->cls->cdesc;

	CLS_DESC_DATA(cdesc, fport, PSE_PORT_PPE0);
	CLS_DESC_DATA(cdesc, tport_idx, 0x2);
	CLS_DESC_DATA(cdesc, cdrt_idx, xfrm_params->cdrt->idx);

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
	CLS_DESC_MASK_DATA(cdesc, l4_hdr_usr_data,
			   0xFFFFFFFF, be32_to_cpu(xfrm_params->xs->id.spi));

	return mtk_pce_cls_entry_write(xfrm_params->cdrt->cls);
}

static void mtk_xfrm_offload_context_tear_down(struct mtk_xfrm_params *xfrm_params)
{
	mtk_xfrm_offload_cdrt_tear_down(xfrm_params);

	/* TODO: free context */
	devm_kfree(crypto_dev, xfrm_params->p_tr);

	/* TODO: transform record tear down */
}

static int mtk_xfrm_offload_context_setup(struct mtk_xfrm_params *xfrm_params)
{
	u32 *tr;
	int ret;

	xfrm_params->p_tr = devm_kcalloc(crypto_dev, sizeof(u32),
					 TRANSFORM_RECORD_LEN, GFP_KERNEL);
	if (unlikely(!xfrm_params->p_tr))
		return -ENOMEM;

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

	memcpy(xfrm_params->p_tr, tr, sizeof(u32) * TRANSFORM_RECORD_LEN);

	/* TODO: free tr */

	return mtk_xfrm_offload_cdrt_setup(xfrm_params);

err_out:
	devm_kfree(crypto_dev, xfrm_params->p_tr);

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

int mtk_xfrm_offload_state_add(struct xfrm_state *xs)
{
	struct mtk_xfrm_params *xfrm_params;
	int ret = 0;

	/* TODO: maybe support IPv6 in the future? */
	if (xs->props.family != AF_INET) {
		CRYPTO_NOTICE("Only IPv4 xfrm states may be offloaded\n");
		return -EINVAL;
	}

	/* only support ESP right now */
	if (xs->id.proto != IPPROTO_ESP) {
		CRYPTO_NOTICE("Unsupported protocol 0x%04x\n", xs->id.proto);
		return -EINVAL;
	}

	/* only support tunnel mode or transport mode */
	if (!(xs->outer_mode.encap == XFRM_MODE_TUNNEL
	    || xs->outer_mode.encap == XFRM_MODE_TRANSPORT))
		return -EINVAL;

	xfrm_params = devm_kzalloc(crypto_dev,
				   sizeof(struct mtk_xfrm_params),
				   GFP_KERNEL);
	if (!xfrm_params)
		return -ENOMEM;

	xfrm_params->xs = xs;
	INIT_LIST_HEAD(&xfrm_params->node);

	if (xs->xso.flags & XFRM_OFFLOAD_INBOUND)
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
	list_add_tail(&xfrm_params->node, &xfrm_params_head);
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

	list_del(&xfrm_params->node);

	if (xs->xso.flags & XFRM_OFFLOAD_INBOUND)
		mtk_xfrm_offload_cls_entry_tear_down(xfrm_params);

	mtk_xfrm_offload_context_tear_down(xfrm_params);

	mtk_pce_cdrt_entry_free(xfrm_params->cdrt);

	devm_kfree(crypto_dev, xfrm_params);
}

void mtk_xfrm_offload_state_tear_down(void)
{
	struct mtk_xfrm_params *xfrm_params, *tmp;

	list_for_each_entry_safe(xfrm_params, tmp, &xfrm_params_head, node)
		mtk_xfrm_offload_state_free(xfrm_params->xs);
}

int mtk_xfrm_offload_policy_add(struct xfrm_policy *xp)
{
	return 0;
}

static inline struct neighbour *mtk_crypto_find_dst_mac(struct sk_buff *skb,  struct xfrm_state *xs)
{
	struct neighbour *neigh;
	u32 nexthop;
	struct dst_entry *dst = skb_dst(skb);
	struct rtable *rt = (struct rtable *) dst;

	nexthop = (__force u32) rt_nexthop(rt, xs->id.daddr.a4);
	neigh = __ipv4_neigh_lookup_noref(dst->dev, nexthop);
	if (unlikely(!neigh)) {
		CRYPTO_INFO("%s: %s No neigh (daddr=%pI4)\n", __func__, dst->dev->name,
				&xs->id.daddr.a4);
		neigh = __neigh_create(&arp_tbl, &xs->id.daddr.a4, dst->dev, false);
		neigh_output(neigh, skb, false);
		return NULL;
	}

	return neigh;
}

bool mtk_xfrm_offload_ok(struct sk_buff *skb,
			 struct xfrm_state *xs)
{
	struct mtk_xfrm_params *xfrm_params;
	struct neighbour *neigh;
	struct dst_entry *dst = skb_dst(skb);

	rcu_read_lock_bh();

	neigh = mtk_crypto_find_dst_mac(skb, xs);
	if (!neigh) {
		rcu_read_unlock_bh();
		return true;
	}

	skb_push(skb, sizeof(struct ethhdr));
	skb_reset_mac_header(skb);

	eth_hdr(skb)->h_proto = htons(ETH_P_IP);
	memcpy(eth_hdr(skb)->h_dest, neigh->ha, ETH_ALEN);
	memcpy(eth_hdr(skb)->h_source, dst->dev->dev_addr, ETH_ALEN);

	rcu_read_unlock_bh();

	xfrm_params = (struct mtk_xfrm_params *)xs->xso.offload_handle;
	skb_hnat_cdrt(skb) = xfrm_params->cdrt->idx;

	/*
	 * EIP197 does not support fragmentation. As a result, we can not bind UDP
	 * flow since it may cause network fail due to fragmentation
	 */
	if ((is_tops_udp_tunnel(skb) || is_tcp(skb)) && is_hnat_rate_reach(skb))
		hnat_bind_crypto_entry(skb, dst->dev);

	/* Since we're going to tx directly, set skb->dev to dst->dev */
	skb->dev = dst->dev;
	/* Set magic tag for tport setting, reset to 0 after tport is set */
	skb_hnat_magic_tag(skb) = HNAT_MAGIC_TAG;

	/*
	 * Since skb headroom may not be copy when segment, we cannot rely on
	 * headroom data (ex. cdrt) to decide packets should send to EIP197.
	 * Here is a workaround that only skb with inner_protocol = ESP will
	 * be sent to EIP197.
	 */
	skb->inner_protocol = IPPROTO_ESP;
	/*
	 * Tx packet to EIP197.
	 * To avoid conflict of SW and HW sequence number
	 * All offloadable packets send to EIP197
	 */
	dev_queue_xmit(skb);

	return true;
}
