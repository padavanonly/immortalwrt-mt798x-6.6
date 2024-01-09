/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _CRYPTO_EIP_H_
#define _CRYPTO_EIP_H_

#include <crypto/sha.h>
#include <linux/io.h>
#include <linux/list.h>
#include <net/xfrm.h>

#include "crypto-eip/crypto-eip197-inline-ddk.h"

struct mtk_crypto;

extern struct mtk_crypto mcrypto;

#define TRANSFORM_RECORD_LEN		64

#define MAX_TUNNEL_NUM			10
#define PACKET_INBOUND			1
#define PACKET_OUTBOUND			2

#define HASH_CACHE_SIZE			SHA512_BLOCK_SIZE

#define EIP197_FORCE_CLK_ON2		(0xfffd8)
#define EIP197_FORCE_CLK_ON		(0xfffe8)
#define EIP197_AUTO_LOOKUP_1		(0xfffffffc)
#define EIP197_AUTO_LOOKUP_2		(0xffffffff)

struct mtk_crypto {
	struct mtk_eth *eth;
	void __iomem *crypto_base;
	void __iomem *eth_base;
};

struct mtk_xfrm_params {
	struct xfrm_state *xs;
	struct list_head node;
	struct cdrt_entry *cdrt;

	u32 *p_tr;			/* pointer to transform record */
	u32 dir;			/* SABuilder_Direction_t */
};

void crypto_eth_write(u32 reg, u32 val);

/* xfrm callback functions */
int mtk_xfrm_offload_state_add(struct xfrm_state *xs);
void mtk_xfrm_offload_state_delete(struct xfrm_state *xs);
void mtk_xfrm_offload_state_free(struct xfrm_state *xs);
void mtk_xfrm_offload_state_tear_down(void);
int mtk_xfrm_offload_policy_add(struct xfrm_policy *xp);
bool mtk_xfrm_offload_ok(struct sk_buff *skb, struct xfrm_state *xs);
#endif /* _CRYPTO_EIP_H_ */
