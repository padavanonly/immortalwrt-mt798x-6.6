// SPDX-License-Identifier: <SPDX License Expression>
/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name: wifi_offload
    warp_mtk.c
*/

#include <warp.h>
#ifdef CONFIG_WARP_DBG_SUPPORT
#include <net/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#endif

const struct of_device_id warp_of_ids[][2] = {
	{
		{	.compatible = WED0_DEV_NODE, },
		{ },
	},
	{
		{	.compatible = WED1_DEV_NODE, },
		{ },
	},
	{
		{	.compatible = WED2_DEV_NODE, },
		{ },
	},
};

#ifdef CONFIG_WARP_DBG_SUPPORT
static void wifi_dump_skb(
	u8 idx,
	struct wlan_tx_info *info,
	struct sk_buff *skb)
{
	struct iphdr *ip_h = ip_hdr(skb);
	struct ethhdr *eth_h = eth_hdr(skb);

	warp_dbg(WARP_DBG_WAR,
		 "%s(): add entry: wdma=%d,ringId=%d,wcid=%d,bssid=%d\n",
		 __func__,
		 idx,
		 info->ringidx,
		 info->wcid,
		 info->bssidx);

	if (ip_h->version != 4)
		return;

	warp_dbg(WARP_DBG_WAR,
		 "%s(): src=%d.%d.%d.%d\n",
		 __func__,
		 (0xff & ip_h->saddr),
		 (0xff00 & ip_h->saddr) >> 8,
		 (0xff0000 & ip_h->saddr) >> 16,
		 (0xff000000 & ip_h->saddr) >> 24);

	warp_dbg(WARP_DBG_WAR,
		 "%s(): dst=%d.%d.%d.%d\n",
		 __func__,
		 (0xff & ip_h->daddr),
		 (0xff00 & ip_h->daddr) >> 8,
		 (0xff0000 & ip_h->daddr) >> 16,
		 (0xff000000 & ip_h->daddr) >> 24);

	warp_dbg(WARP_DBG_WAR,
		 "%s(): src_mac=%pM, dest_mac=%pM\n",
		 __func__,
		 eth_h->h_source,
		 eth_h->h_dest);

	if (ip_h->protocol == IPPROTO_TCP) {
		struct tcphdr *tcph = tcp_hdr(skb);

		warp_dbg(WARP_DBG_WAR,
			 "%s(): protocol=TCP,src_port=%d,dst_port=%d\n",
			 __func__,
			 tcph->source,
			 tcph->dest);
	}

	if (ip_h->protocol == IPPROTO_UDP) {
		struct udphdr *udph = udp_hdr(skb);

		warp_dbg(WARP_DBG_WAR,
			 "%s(): protocol=UDP,src_port=%d,dst_port=%d\n",
			 __func__,
			 udph->source,
			 udph->dest);
	}
}
#endif /*CONFIG_WARP_DBG_SUPPORT*/

void
wifi_rx_tuple_add(struct wifi_entry *wifi, u8 idx, unsigned char *rx_info)
{
	struct wlan_rx_info r, *info =  &r;
	struct sk_buff *skb = NULL;

	memset(info, 0, sizeof(*info));
	wifi->ops->rxinfo_wrapper(rx_info, info);
	warp_dbg(WARP_DBG_INF, "%s(): WED IDX: %d,PPE Entry: %d, CSRN: %d\n",
		__func__,
		 idx, info->ppe_entry, info->csrn);

	skb = (struct sk_buff *)info->pkt;

	if (IS_SPACE_AVAILABLE_HEAD(skb)) {
		if (idx == 2)
			FOE_MAGIC_TAG(skb) = FOE_MAGIC_WED2;
		else if (idx == 1)
			FOE_MAGIC_TAG(skb) = FOE_MAGIC_WED1;
		else
			FOE_MAGIC_TAG(skb) = FOE_MAGIC_WED0;

		FOE_ENTRY_NUM(skb) = info->ppe_entry;
		FOE_AI(skb) = info->csrn;
	}
}

int
wifi_tx_tuple_add(struct wifi_entry *wifi, u8 idx, u8 *tx_info, u32 wdma_rx_port)
{
	struct wlan_tx_info t, *info =  &t;

	memset(info, 0, sizeof(*info));
	wifi->ops->txinfo_wrapper(tx_info, info);

	if (ra_sw_nat_hook_tx) {
		struct sk_buff *skb = (struct sk_buff *)info->pkt;

		if (!info->pkt) {
			warp_dbg(WARP_DBG_INF, "%s():info->pkt is NULL!!!!\n", __func__);
			return -EFAULT;
		}
		if (!skb->head) {
			warp_dbg(WARP_DBG_INF, "%s():skb->head is NULL!!!!\n", __func__);
			return -EFAULT;
		}
		if ((FOE_AI(skb) == HIT_UNBIND_RATE_REACH) ||
		    (FOE_AI_TAIL(skb) == HIT_UNBIND_RATE_REACH)) {
			if (IS_SPACE_AVAILABLE_HEAD(skb)) {
				/*WDMA idx*/
				FOE_WDMA_ID(skb) = idx;
				/*Ring idx*/
				FOE_RX_ID(skb) = info->ringidx;
				/*wtable Idx*/
				FOE_WC_ID(skb) = info->wcid;
				/*Bssidx*/
				FOE_BSS_ID(skb) = info->bssidx;
#ifdef WED_PAO_SUPPORT
				FOE_USR_INFO(skb) = info->usr_info;
				FOE_TID(skb) = info->tid;
				FOE_IS_FIXEDRATE(skb) = info->is_fixedrate;
				FOE_IS_PRIOR(skb) = info->is_prior;
				FOE_IS_SP(skb) = info->is_sp;
				FOE_HF(skb) = info->hf;
				FOE_AMSDU(skb) = info->amsdu_en;
#endif
			}
			if (IS_SPACE_AVAILABLE_TAIL(skb)) {
				/*WDMA idx*/
				FOE_WDMA_ID_TAIL(skb) = idx;
				/*Ring idx*/
				FOE_RX_ID_TAIL(skb) = info->ringidx;
				/*wtable Idx*/
				FOE_WC_ID_TAIL(skb) = info->wcid;
				/*Bssidx*/
				FOE_BSS_ID_TAIL(skb) = info->bssidx;
#ifdef WED_PAO_SUPPORT
				FOE_USR_INFO(skb) = info->usr_info;
				FOE_TID(skb) = info->tid;
				FOE_IS_FIXEDRATE(skb) = info->is_fixedrate;
				FOE_IS_PRIOR(skb) = info->is_prior;
				FOE_IS_SP(skb) = info->is_sp;
				FOE_HF(skb) = info->hf;
				FOE_AMSDU(skb) = info->amsdu_en;
#endif

			}
			if (!IS_SPACE_AVAILABLE_TAIL(skb) && !IS_SPACE_AVAILABLE_HEAD(skb)) {
				warp_dbg(WARP_DBG_INF, "%s(): SKB HEAD/TAIL is not available\n",
						__func__);
			}

			warp_dbg(WARP_DBG_LOU, "%s(): WDMAID: %d,RingID: %d, Wcid: %d, Bssid: %d\n",
				__func__,
				FOE_WDMA_ID(skb), FOE_RX_ID(skb), FOE_WC_ID(skb), FOE_BSS_ID(skb));
		}
		/*use port for specify which hw_nat architecture*/
		if (ra_sw_nat_hook_tx(skb, wdma_rx_port) != 1)
			wifi->ops->txinfo_set_drop(tx_info);

#ifdef CONFIG_WARP_DBG_SUPPORT
		wifi_dump_skb(idx, info, skb);
#endif /*CONFIG_WARP_DBG_SUPPORT*/

	} else {
		warp_dbg(WARP_DBG_INF, "%s(): ra_sw_nat_hook_tx is null!\n", __func__);
	}
	return 0;
}

int
wdma_pse_port_config_state(u8 wdma_idx, bool up_state)
{
	if (hnat_set_wdma_pse_port_state) {
		warp_dbg(WARP_DBG_INF, "%s(): Set wdma idx = %u pse port state as %s\n",
			__func__, wdma_idx, up_state ? "on" : "off");
		return hnat_set_wdma_pse_port_state(wdma_idx, up_state);
	}

	warp_dbg(WARP_DBG_ERR, "%s(): hnat_set_wdma_pse_port_state() is null\n", __func__);
	return -EOPNOTSUPP;
}

