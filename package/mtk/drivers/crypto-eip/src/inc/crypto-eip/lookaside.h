/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _LOOKASIDE_H_
#define _LOOKASIDE_H_

#include <linux/version.h>
#include <linux/io.h>
#include <linux/list.h>

#include <crypto/aes.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))
#include <crypto/sha1.h>
#include <crypto/sha2.h>
#else
#include <crypto/sha.h>
#endif

#include <crypto/md5.h>
#include <crypto/algapi.h>
#include <crypto/skcipher.h>
#include <crypto/aead.h>
#include <crypto/hash.h>

#include "crypto-eip197-inline-ddk.h"

#define EIP197_DEFAULT_RING_SIZE 400
#define MTK_CRYPTO_PRIORITY 1500
#define EIP197_AEAD_TYPE_IPSEC_ESP 3
#define EIP197_AEAD_TYPE_IPSEC_ESP_GMAC 4
#define EIP197_AEAD_IPSEC_IV_SIZE 8
#define EIP197_AEAD_IPSEC_CCM_NONCE_SIZE 3

extern struct mtk_crypto_priv *priv;

extern struct mtk_crypto_alg_template mtk_crypto_cbc_aes;
extern struct mtk_crypto_alg_template mtk_crypto_ofb_aes;
extern struct mtk_crypto_alg_template mtk_crypto_ecb_aes;
extern struct mtk_crypto_alg_template mtk_crypto_cfb_aes;
extern struct mtk_crypto_alg_template mtk_crypto_ctr_aes;
extern struct mtk_crypto_alg_template mtk_crypto_cbc_des;
extern struct mtk_crypto_alg_template mtk_crypto_ecb_des;
extern struct mtk_crypto_alg_template mtk_crypto_cbc_des3_ede;
extern struct mtk_crypto_alg_template mtk_crypto_ecb_des3_ede;

extern struct mtk_crypto_alg_template mtk_crypto_sha1;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha1;
extern struct mtk_crypto_alg_template mtk_crypto_sha224;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha224;
extern struct mtk_crypto_alg_template mtk_crypto_sha256;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha256;
extern struct mtk_crypto_alg_template mtk_crypto_sha384;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha384;
extern struct mtk_crypto_alg_template mtk_crypto_sha512;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha512;
extern struct mtk_crypto_alg_template mtk_crypto_md5;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_md5;
extern struct mtk_crypto_alg_template mtk_crypto_xcbcmac;
extern struct mtk_crypto_alg_template mtk_crypto_cmac;

extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_cbc_aes;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha224_cbc_aes;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha256_cbc_aes;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha384_cbc_aes;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha512_cbc_aes;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_md5_cbc_aes;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_cbc_des3_ede;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha224_cbc_des3_ede;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha256_cbc_des3_ede;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha384_cbc_des3_ede;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha512_cbc_des3_ede;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_md5_cbc_des3_ede;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_cbc_des;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha224_cbc_des;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha256_cbc_des;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha384_cbc_des;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha512_cbc_des;
extern struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_ctr_aes;
extern struct mtk_crypto_alg_template mtk_crypto_gcm;
extern struct mtk_crypto_alg_template mtk_crypto_rfc4106_gcm;
extern struct mtk_crypto_alg_template mtk_crypto_rfc4543_gcm;
extern struct mtk_crypto_alg_template mtk_crypto_rfc4309_ccm;

// general
struct mtk_crypto_work_data {
	struct work_struct work;
	struct mtk_crypto_priv *priv;
	int ring;
};

struct mtk_crypto_ring {
	struct list_head list;
	spinlock_t ring_lock;

	struct workqueue_struct *workqueue;
	struct mtk_crypto_work_data work_data;

	struct crypto_queue queue;
	spinlock_t queue_lock;
};

struct mtk_crypto_priv {
	struct mtk_crypto_ring *mtk_eip_ring;
	atomic_t ring_used;
};

struct mtk_crypto_engine_data {
	int valid;
	void *sa_handle;
	void *sa_addr;
	uint32_t sa_size;
	void *token_handle;
	void *token_addr;
	void *token_context;
	uint32_t token_size;
	void *pkt_handle;
};

struct mtk_crypto_result {
	struct list_head list;
	void *dst;
	struct mtk_crypto_engine_data eip;
	struct crypto_async_request *async;
	int size;
};

enum mtk_crypto_alg_type {
	MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	MTK_CRYPTO_ALG_TYPE_AEAD,
	MTK_CRYPTO_ALG_TYPE_AHASH,
};

struct mtk_crypto_alg_template {
	struct mtk_crypto_priv *priv;
	enum mtk_crypto_alg_type type;
	union {
		struct skcipher_alg skcipher;
		struct aead_alg aead;
		struct ahash_alg ahash;
	} alg;
};

// cipher algos
struct mtk_crypto_context {
	int (*send)(struct crypto_async_request *req);
	int (*handle_result)(struct mtk_crypto_result *req, int err);
	int ring;
	atomic_t req_count;
};

enum mtk_crypto_cipher_direction {
	MTK_CRYPTO_ENCRYPT,
	MTK_CRYPTO_DECRYPT,
};

enum mtk_crypto_cipher_mode {
	MTK_CRYPTO_MODE_CBC,
	MTK_CRYPTO_MODE_ECB,
	MTK_CRYPTO_MODE_OFB,
	MTK_CRYPTO_MODE_CFB,
	MTK_CRYPTO_MODE_CTR,
	MTK_CRYPTO_MODE_GCM,
	MTK_CRYPTO_MODE_GMAC,
	MTK_CRYPTO_MODE_CCM,
};

enum mtk_crypto_alg {
	MTK_CRYPTO_AES,
	MTK_CRYPTO_DES,
	MTK_CRYPTO_3DES,
	MTK_CRYPTO_ALG_SHA1,
	MTK_CRYPTO_ALG_SHA224,
	MTK_CRYPTO_ALG_SHA256,
	MTK_CRYPTO_ALG_SHA384,
	MTK_CRYPTO_ALG_SHA512,
	MTK_CRYPTO_ALG_MD5,
	MTK_CRYPTO_ALG_GCM,
	MTK_CRYPTO_ALG_GMAC,
	MTK_CRYPTO_ALG_CCM,
	MTK_CRYPTO_ALG_XCBC,
	MTK_CRYPTO_ALG_CMAC_128,
	MTK_CRYPTO_ALG_CMAC_192,
	MTK_CRYPTO_ALG_CMAC_256,
};

struct mtk_crypto_cipher_req {
	enum mtk_crypto_cipher_direction direction;
	int nr_src;
	int nr_dst;
};

struct mtk_crypto_cipher_ctx {
	struct mtk_crypto_context base;
	struct mtk_crypto_priv *priv;

	u8 aead;

	enum mtk_crypto_cipher_mode mode;
	enum mtk_crypto_alg alg;
	u8 blocksz;
	__le32 key[16];
	u32 nonce;
	unsigned int key_len;

	enum mtk_crypto_alg hash_alg;
	u32 state_sz;
	__be32 ipad[SHA512_DIGEST_SIZE / sizeof(u32)];
	__be32 opad[SHA512_DIGEST_SIZE / sizeof(u32)];

	struct crypto_cipher *hkaes;
	struct crypto_aead *fback;

	struct timer_list poll_timer;

	struct mtk_crypto_engine_data enc;
	struct mtk_crypto_engine_data dec;
};

enum mtk_crypto_ahash_digest {
	MTK_CRYPTO_DIGEST_INITIAL,
	MTK_CRYPTO_DIGEST_PRECOMPUTED,
	MTK_CRYPTO_DIGEST_XCM,
	MTK_CRYPTO_DIGEST_HMAC,
};

struct mtk_crypto_ahash_ctx {
	struct mtk_crypto_context base;
	struct mtk_crypto_priv *priv;

	enum mtk_crypto_alg alg;
	u8 key_sz;
	bool cbcmac;

	__le32 ipad[SHA512_BLOCK_SIZE / sizeof(__le32)];
	__le32 opad[SHA512_BLOCK_SIZE / sizeof(__le32)];
	__le32 zero_hmac[SHA512_BLOCK_SIZE / sizeof(__le32)];

	void *ipad_sa;
	void *ipad_token;
	void *opad_sa;
	void *opad_token;

	struct crypto_cipher *kaes;
	struct crypto_ahash *fback;
	struct crypto_shash *shpre;
	struct shash_desc *shdesc;
};

struct mtk_crypto_ahash_req {
	bool last_req;
	bool finish;
	bool hmac;
	bool hmac_zlen;
	bool len_is_le;
	bool not_first;
	bool xcbcmac;

	int nents;

	u32 digest;

	u8 state_sz;
	u8 block_sz;
	u8 digest_sz;
	u32 state[SHA512_DIGEST_SIZE / sizeof(u32)];

	u64 len;
	u64 processed;

	u8 cache[HASH_CACHE_SIZE] __aligned(sizeof(u32));

	void *sa_pointer;
	void *token_context;
	u8 cache_next[HASH_CACHE_SIZE] __aligned(sizeof(u32));
};

#define HASH_CACHE_SIZE		SHA512_BLOCK_SIZE

struct mtk_crypto_ahash_export_state {
	u64 len;
	u64 processed;

	u32 digest;

	u32 state[SHA512_DIGEST_SIZE / sizeof(u32)];
	u8 cache[HASH_CACHE_SIZE];
	int ring;
	void *sa_pointer;
	void *token_context;
};

void mtk_crypto_dequeue_work(struct work_struct *work);
void mtk_crypto_dequeue(struct mtk_crypto_priv *priv, int ring);
int mtk_crypto_select_ring(struct mtk_crypto_priv *priv);
int mtk_crypto_hmac_setkey(const char *alg, const u8 *key, unsigned int keylen,
			   void *istate, void *ostate);

#endif /* _LOOKASIDE_H_ */
