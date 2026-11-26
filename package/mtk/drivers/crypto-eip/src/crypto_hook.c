// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 */

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include "crypto-eip/crypto-eip.h"

#include <pce/cdrt.h>

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
#include <mtk_hnat/hnat.h>
#include <mtk_hnat/nf_hnat_mtk.h>
#endif // HNAT

static unsigned int
mtk_crypto_ipv4_nf_pre_routing(void *priv, struct sk_buff *skb,
			     const struct nf_hook_state *state)
{
#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
	struct xfrm_params_list *xfrm_params_list;
	struct mtk_xfrm_params *xfrm_params;

	if (!skb)
		return NF_DROP;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	if (skb_hnat_cdrt(skb)) {
		xfrm_params_list = mtk_xfrm_params_list_get();
		spin_lock_bh(&xfrm_params_list->lock);
		list_for_each_entry(xfrm_params, &xfrm_params_list->list, node) {
			if (skb_hnat_cdrt(skb) == xfrm_params->cdrt->idx) {
				atomic64_add(skb->len, &xfrm_params->bytes);
				atomic64_inc(&xfrm_params->packets);
				break;
			}
		}
		spin_unlock_bh(&xfrm_params_list->lock);
	}
#endif // HNAT
	return NF_ACCEPT;
}

static struct nf_hook_ops mtk_crypto_nf_ops[] __read_mostly = {
	{
		.hook = mtk_crypto_ipv4_nf_pre_routing,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST + 1,
	},
};

int mtk_crypto_register_nf_hooks(void)
{
	return nf_register_net_hooks(&init_net, mtk_crypto_nf_ops, ARRAY_SIZE(mtk_crypto_nf_ops));
}

void mtk_crypto_unregister_nf_hooks(void)
{
	nf_unregister_net_hooks(&init_net, mtk_crypto_nf_ops, ARRAY_SIZE(mtk_crypto_nf_ops));
}
