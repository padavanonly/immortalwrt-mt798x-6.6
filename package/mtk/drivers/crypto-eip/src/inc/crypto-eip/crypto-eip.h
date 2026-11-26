/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _CRYPTO_EIP_H_
#define _CRYPTO_EIP_H_

#include <linux/version.h>
#include <linux/netfilter.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/atomic.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#else
#include <crypto/sha.h>
#endif

#include <net/xfrm.h>

#include "crypto-eip/crypto-eip197-inline-ddk.h"

struct mtk_crypto;

extern struct mtk_crypto mcrypto;
extern int mac_filter_enable;

#define TRANSFORM_RECORD_LEN		64
#define EIP197_SMALL_TR			2
#define EIP197_LARGE_TR			3

#define MAX_TUNNEL_NUM			10
#define PACKET_INBOUND			1
#define PACKET_OUTBOUND			2

#define HASH_CACHE_SIZE			SHA512_BLOCK_SIZE

#define EIP197_FORCE_CLK_ON2		(0xfffd8)
#define EIP197_FORCE_CLK_ON		(0xfffe8)
#define EIP197_AUTO_LOOKUP_1		(0xfffffffc)
#define EIP197_AUTO_LOOKUP_2		(0xffffffff)

#define PEC_PCL_EIP197
#define CAPWAP_MAX_TUNNEL_NUM CONFIG_TOPS_TNL_NUM

struct mtk_crypto {
	struct mtk_eth *eth;
	void __iomem *crypto_base;
	void __iomem *eth_base;
	u32 ppe_num;
};

struct mtk_xfrm_params {
	struct xfrm_state *xs;
	struct list_head node;
	struct cdrt_entry *cdrt;

	void *p_tr;			/* pointer to transform record */
	u32 tr_type;
	void *p_handle;		/* pointer to eip dma handle */
	u32 dir;			/* SABuilder_Direction_t */
	atomic64_t bytes;          /* Total bytes applied */
	atomic64_t packets;		/* Total packets applied */
};

/* DTLS */
enum dtls_sec_mode_type {
	__DTLS_SEC_MODE_TYPE_NONE = 0,
	AES128_CBC_HMAC_SHA1,
	AES256_CBC_HMAC_SHA1,
	AES128_CBC_HMAC_SHA2_256,
	AES256_CBC_HMAC_SHA2_256,
	AES128_GCM,
	AES256_GCM,
	__DTLS_SEC_MODE_TYPE_MAX = 7,
};

enum dtls_version {
	MTK_DTLS_VERSION_1_0 = 0,
	MTK_DTLS_VERSION_1_2 = 1,
	__DTLS_VERSION_MAX = 2,
};

enum dtls_network_type {
	MTK_DTLS_NET_NONE = 0,
	MTK_DTLS_NET_IPV4,
	MTK_DTLS_NET_IPV6,
	__MTK_DTLS_NET_MAX,
};

union ip4_addr {
	__be32 addr32;
	u8 addr8[4];
};

struct ip6_addr {
	u32 addr[4];
};

union ip_addr {
	union ip4_addr ip4;
	struct ip6_addr ip6;
};

struct DTLS_param {
	enum dtls_network_type net_type;
	union ip_addr dip;
	union ip_addr sip;
	uint16_t dport;
	uint16_t sport;
	uint16_t dtls_epoch;
	uint16_t dtls_version;
	uint8_t sec_mode;
	uint8_t *dtls_encrypt_nonce;
	uint8_t *dtls_decrypt_nonce;
	uint8_t *key_encrypt;
	uint8_t *key_auth_encrypt_1;
	uint8_t *key_auth_encrypt_2;
	uint8_t *key_decrypt;
	uint8_t *key_auth_decrypt_1;
	uint8_t *key_auth_decrypt_2;
	void *SA_encrypt;
	void *SA_decrypt;
} __packed __aligned(16);

struct DTLSResourceMgmt {
	struct DTLS_param *DTLSParam;
	DMABuf_Handle_t sa_out;
	DMABuf_Handle_t sa_in;
};

struct mtk_cdrt_idx_param {
	uint32_t cdrt_idx_inbound;
	uint32_t cdrt_idx_outbound;
};

struct mtk_CDRT_DTLS_entry {
	struct cdrt_entry *cdrt_inbound;
	struct cdrt_entry *cdrt_outbound;
};

struct xfrm_params_list {
	struct list_head list;
	spinlock_t lock;
};

#if defined(CONFIG_CRYPTO_XFRM_OFFLOAD_MTK_PCE)
struct xfrm_params_list *mtk_xfrm_params_list_get(void);
#else /* !defined(CONFIG_CRYPTO_XFRM_OFFLOAD_MTK_PCE) */
static inline struct xfrm_params_list *mtk_xfrm_params_list_get(void)
{
	return NULL;
}
#endif /* defined(CONFIG_CRYPTO_XFRM_OFFLOAD_MTK_PCE) */

#if defined(CONFIG_CRYPTO_OFFLOAD_INLINE_FLOWBLOCK)
int mtk_crypto_xfrm_get_cdrt(struct xfrm_state *xs);
#endif /* Flow Block */

extern void mtk_update_dtls_param(struct DTLS_param *DTLSParam_p, int TnlIdx);
extern void mtk_remove_dtls_param(struct DTLS_param *DTLSParam_p, int TnlIdx);
extern void mtk_update_cdrt_idx(struct mtk_cdrt_idx_param *cdrt_idx_params_p);

void mtk_crypto_enable_ipsec_dev_features(struct net_device *dev);
void mtk_crypto_disable_ipsec_dev_features(struct net_device *dev);

/* Netsys */
void crypto_eth_write(u32 reg, u32 val);
u32 mtk_crypto_ppe_get_num(struct net_device *dev);

/* xfrm callback functions */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
int mtk_xfrm_offload_state_add(struct xfrm_state *xs, struct netlink_ext_ack *extack);
int mtk_xfrm_offload_policy_add(struct xfrm_policy *xp, struct netlink_ext_ack *extack);
#else
int mtk_xfrm_offload_state_add(struct xfrm_state *xs);
int mtk_xfrm_offload_policy_add(struct xfrm_policy *xp);
#endif /* KERNEL 6.3.0 */

void mtk_xfrm_offload_state_delete(struct xfrm_state *xs);
void mtk_xfrm_offload_state_free(struct xfrm_state *xs);
void mtk_xfrm_offload_state_tear_down(void);
void mtk_xfrm_offload_policy_delete(struct xfrm_policy *xp);
void mtk_xfrm_offload_policy_free(struct xfrm_policy *xp);
bool mtk_xfrm_offload_ok(struct sk_buff *skb, struct xfrm_state *xs);

int mtk_crypto_register_nf_hooks(void);
void mtk_crypto_unregister_nf_hooks(void);
#endif /* _CRYPTO_EIP_H_ */
