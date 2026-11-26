// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/bitops.h>
#include <crypto/aes.h>
#include <crypto/hmac.h>
#include <crypto/skcipher.h>
#include <crypto/internal/skcipher.h>
#include <crypto/internal/hash.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/lookaside.h"
#include "crypto-eip/internal.h"

static inline u64 mtk_crypto_queued_len(struct mtk_crypto_ahash_req *req)
{
	return req->len - req->processed;
}

static int mtk_crypto_ahash_enqueue(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_priv *priv = ctx->priv;
	int ret;

	if (ctx->base.ring < 0)
		ctx->base.ring = mtk_crypto_select_ring(priv);

	spin_lock_bh(&priv->mtk_eip_ring[ctx->base.ring].queue_lock);
	ret = crypto_enqueue_request(&priv->mtk_eip_ring[ctx->base.ring].queue, &areq->base);
	spin_unlock_bh(&priv->mtk_eip_ring[ctx->base.ring].queue_lock);

	queue_work(priv->mtk_eip_ring[ctx->base.ring].workqueue,
			&priv->mtk_eip_ring[ctx->base.ring].work_data.work);

	return ret;
}

static int mtk_crypto_ahash_send(struct crypto_async_request *async)
{
	struct ahash_request *areq = ahash_request_cast(async);
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	int cache_len;
	int extra = 0;
	int areq_shift;
	u64 queued;
	u64 len;
	bool ret = 0;
	uint8_t *cur_req;
	int i;

	if (req->hmac_zlen)
		goto zero_length_hmac;
	areq_shift = 0;
	queued = mtk_crypto_queued_len(req);
	if (queued <= HASH_CACHE_SIZE)
		cache_len = queued;
	else
		cache_len = queued - areq->nbytes;

	if (!req->finish && !req->last_req) {
		/* If this is not the last request and the queued data does not
		 * fit into full cache blocks, cache it for the next send call.
		 */
		extra = queued & (HASH_CACHE_SIZE - 1);

		if (!extra)
			extra = HASH_CACHE_SIZE;

		sg_pcopy_to_buffer(areq->src, sg_nents(areq->src),
				   req->cache_next, extra, areq->nbytes - extra);

		queued -= extra;

		if (!queued)
			return 0;

		extra = 0;
	}

	len = queued;
	cur_req = kmalloc(sizeof(uint8_t) * len + AES_BLOCK_SIZE, GFP_KERNEL);
	if (!cur_req) {
		CRYPTO_ERR("alloc buffer for ahash request failed\n");
		goto exit;
	}
	/* Send request to EIP197 */
	if (cache_len) {
		memcpy(cur_req, req->cache, cache_len);
		queued -= cache_len;
	}
	if (queued)
		sg_copy_to_buffer(areq->src, sg_nents(areq->src), cur_req + cache_len, queued);

	if (unlikely(req->xcbcmac)) {
		int pad_size = 0;
		int offset;
		int new;

		if (req->finish) {
			new = len % AES_BLOCK_SIZE;
			pad_size = AES_BLOCK_SIZE - new;
			offset = (len - new) / sizeof(u32);

			if (pad_size != AES_BLOCK_SIZE) {
				memset(cur_req + len, 0, pad_size);
				cur_req[len] = 0x80;
				for (i = 0; i < AES_BLOCK_SIZE / sizeof(u32); i++) {
					((__be32 *) cur_req)[offset + i] ^=
						cpu_to_be32(le32_to_cpu(
							ctx->ipad[i + 4]));
				}
			} else {
				for (i = 0; i < AES_BLOCK_SIZE / sizeof(u32); i++)
					((__be32 *) cur_req)[offset - 4 + i] ^=
						cpu_to_be32(le32_to_cpu(
							ctx->ipad[i]));
				pad_size = 0;
			}
		}

		ret = crypto_ahash_aes_cbc(async, req, cur_req, len + pad_size);
		kfree(cur_req);
		if (ret) {
			if (req->sa_pointer)
				crypto_free_sa(req->sa_pointer, ctx->base.ring);
			kfree(req->token_context);
			CRYPTO_ERR("Fail on ahash_aes_cbc process\n");
			goto exit;
		}
		req->not_first = true;
		req->processed += len - extra;

		return 0;
	}

	if (req->not_first)
		ret = crypto_ahash_token_req(async, req, cur_req, len, req->finish);
	else
		ret = crypto_first_ahash_req(async, req, cur_req, len, req->finish);

	kfree(cur_req);

	if (ret) {
		if (req->sa_pointer)
			crypto_free_sa(req->sa_pointer, ctx->base.ring);
		CRYPTO_ERR("Fail on ahash_req process\n");
		goto exit;
	}
	req->not_first = true;
	req->processed += len - extra;

	return 0;

zero_length_hmac:
	if (req->sa_pointer)
		crypto_free_sa(req->sa_pointer, ctx->base.ring);
	kfree(req->token_context);

	/* complete the final hash with opad for hmac*/
	if (req->hmac) {
		req->sa_pointer = ctx->opad_sa;
		req->token_context = ctx->opad_token;
		ret = crypto_ahash_token_req(async, req, (uint8_t *) req->state,
						req->digest_sz, true);
	}

	return 0;
exit:
	local_bh_disable();
	async->complete(async, ret);
	local_bh_enable();

	return 0;
}

static int mtk_crypto_ahash_handle_result(struct mtk_crypto_result *res, int err)
{
	struct crypto_async_request *async = res->async;
	struct ahash_request *areq = ahash_request_cast(async);
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);
	struct crypto_ahash *ahash = crypto_ahash_reqtfm(areq);
	int cache_len;

	if (req->xcbcmac) {
		memcpy(req->state, res->dst + res->size - AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		crypto_free_sa(res->eip.sa_handle, ctx->base.ring);
		kfree(res->eip.token_context);
	} else
		memcpy(req->state, res->dst, req->digest_sz);

	crypto_free_token(res->eip.token_handle);
	crypto_free_pkt(res->eip.pkt_handle);

	if (req->finish) {
		if (req->hmac && !req->hmac_zlen) {
			req->hmac_zlen = true;
			mtk_crypto_ahash_enqueue(areq);
			return 0;
		}
		if (req->sa_pointer)
			crypto_free_sa(req->sa_pointer, ctx->base.ring);

		kfree(req->token_context);

		memcpy(areq->result, req->state, crypto_ahash_digestsize(ahash));
	}

	cache_len = mtk_crypto_queued_len(req);
	if (cache_len)
		memcpy(req->cache, req->cache_next, cache_len);

	local_bh_disable();
	async->complete(async, 0);
	local_bh_enable();

	return 0;
}

static int mtk_crypto_ahash_cache(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);
	u64 cache_len;

	cache_len = mtk_crypto_queued_len(req);

	if (cache_len + areq->nbytes <= HASH_CACHE_SIZE) {
		sg_pcopy_to_buffer(areq->src, sg_nents(areq->src),
				   req->cache + cache_len,
				   areq->nbytes, 0);
		return 0;
	}

	return -E2BIG;
}

static int mtk_crypto_ahash_update(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);
	int ret;

	if (!areq->nbytes)
		return 0;

	ret = mtk_crypto_ahash_cache(areq);

	req->len += areq->nbytes;

	if ((ret && !req->finish) || req->last_req)
		return mtk_crypto_ahash_enqueue(areq);

	return 0;
}

static int mtk_crypto_ahash_final(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));

	req->finish = true;

	if (unlikely(!req->len && !areq->nbytes && !req->hmac)) {
		if (ctx->alg == MTK_CRYPTO_ALG_SHA1)
			memcpy(areq->result, sha1_zero_message_hash,
			       SHA1_DIGEST_SIZE);
		else if (ctx->alg == MTK_CRYPTO_ALG_SHA224)
			memcpy(areq->result, sha224_zero_message_hash,
			       SHA224_DIGEST_SIZE);
		else if (ctx->alg == MTK_CRYPTO_ALG_SHA256)
			memcpy(areq->result, sha256_zero_message_hash,
			       SHA256_DIGEST_SIZE);
		else if (ctx->alg == MTK_CRYPTO_ALG_SHA384)
			memcpy(areq->result, sha384_zero_message_hash,
			       SHA384_DIGEST_SIZE);
		else if (ctx->alg == MTK_CRYPTO_ALG_SHA512)
			memcpy(areq->result, sha512_zero_message_hash,
			       SHA512_DIGEST_SIZE);
		else if (ctx->alg == MTK_CRYPTO_ALG_MD5)
			memcpy(areq->result, md5_zero_message_hash,
			       MD5_DIGEST_SIZE);

		return 0;
	} else if (unlikely(req->digest == MTK_CRYPTO_DIGEST_XCM &&
			    ctx->alg == MTK_CRYPTO_ALG_MD5 && req->len == sizeof(u32) &&
			    !areq->nbytes)) {
		memcpy(areq->result, ctx->ipad, sizeof(u32));
		return 0;
	} else if (unlikely(ctx->cbcmac && req->len == AES_BLOCK_SIZE &&
				!areq->nbytes)) {
		memset(areq->result, 0, AES_BLOCK_SIZE);
		return 0;
	} else if (unlikely(req->xcbcmac && req->len == AES_BLOCK_SIZE &&
			    !areq->nbytes)) {
		int i;

		for (i = 0; i < AES_BLOCK_SIZE / sizeof(u32); i++)
			((__be32 *) areq->result)[i] =
				cpu_to_be32(le32_to_cpu(ctx->ipad[i + 4]));
		areq->result[0] ^= 0x80;
		crypto_cipher_encrypt_one(ctx->kaes, areq->result, areq->result);
		return 0;
	} else if (unlikely(req->hmac && (req->len == req->block_sz) &&
			    !areq->nbytes)) {
		memcpy(req->state, ctx->zero_hmac, req->state_sz);
		req->hmac_zlen = true;
	} else if (req->hmac) {
		req->digest = MTK_CRYPTO_DIGEST_HMAC;
	}

	return mtk_crypto_ahash_enqueue(areq);
}

static int mtk_crypto_ahash_finup(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	req->finish = true;

	mtk_crypto_ahash_update(areq);
	return mtk_crypto_ahash_final(areq);
}

static int mtk_crypto_ahash_export(struct ahash_request *areq, void *out)
{
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_export_state *export = out;

	export->len = req->len;
	export->processed = req->processed;

	export->digest = req->digest;
	export->sa_pointer = req->sa_pointer;
	export->token_context = req->token_context;
	export->ring = ctx->base.ring;

	memcpy(export->state, req->state, req->state_sz);
	memcpy(export->cache, req->cache, HASH_CACHE_SIZE);

	return 0;
}

static int mtk_crypto_ahash_import(struct ahash_request *areq, const void *in)
{
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);
	const struct mtk_crypto_ahash_export_state *export = in;
	int ret;

	ret = crypto_ahash_init(areq);
	if (ret)
		return ret;

	req->len = export->len;
	req->processed = export->processed;

	req->digest = export->digest;
	req->sa_pointer = export->sa_pointer;
	req->token_context = export->token_context;
	if (req->sa_pointer)
		req->not_first = true;

	memcpy(req->cache, export->cache, HASH_CACHE_SIZE);
	memcpy(req->state, export->state, req->state_sz);

	return 0;
}

static int mtk_crypto_ahash_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(tfm);
	struct mtk_crypto_alg_template *tmpl =
		container_of(__crypto_ahash_alg(tfm->__crt_alg),
			     struct mtk_crypto_alg_template, alg.ahash);

	ctx->priv = tmpl->priv;
	ctx->base.send = mtk_crypto_ahash_send;
	ctx->base.handle_result = mtk_crypto_ahash_handle_result;
	ctx->base.ring = -1;
	crypto_ahash_set_reqsize(__crypto_ahash_cast(tfm),
				 sizeof(struct mtk_crypto_ahash_req));

	return 0;
}

static int mtk_crypto_sha1_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	ctx->alg = MTK_CRYPTO_ALG_SHA1;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA1_DIGEST_SIZE;
	req->digest_sz = SHA1_DIGEST_SIZE;
	req->block_sz = SHA1_BLOCK_SIZE;

	return 0;
}

static int mtk_crypto_sha1_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_sha1_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

static void mtk_crypto_ahash_cra_exit(struct crypto_tfm *tfm)
{
}

struct mtk_crypto_alg_template mtk_crypto_sha1 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_sha1_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_sha1_digest,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "sha1",
				.cra_driver_name = "crypto-eip-sha1",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA1_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_hmac_sha1_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	memcpy(req->state, ctx->ipad, SHA1_DIGEST_SIZE);
	req->sa_pointer = ctx->ipad_sa;
	req->token_context = ctx->ipad_token;
	req->not_first = true;
	req->len = SHA1_BLOCK_SIZE;
	req->processed = SHA1_BLOCK_SIZE;

	ctx->alg = MTK_CRYPTO_ALG_SHA1;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA1_DIGEST_SIZE;
	req->digest_sz = SHA1_DIGEST_SIZE;
	req->block_sz = SHA1_BLOCK_SIZE;
	req->hmac = true;

	return 0;
}

static int mtk_crypto_hmac_sha1_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_hmac_sha1_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_ahash_result {
	struct completion completion;
	int error;
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0))
static void mtk_crypto_ahash_complete(void *req, int error)
{
	struct crypto_async_request *areq = req;
	struct mtk_crypto_ahash_result *result = areq->data;

	if (error == -EINPROGRESS)
		return;

	result->error = error;
	complete(&result->completion);
}
#else
static void mtk_crypto_ahash_complete(struct crypto_async_request *req, int error)
{
	struct mtk_crypto_ahash_result *result = req->data;

	if (error == -EINPROGRESS)
		return;

	result->error = error;
	complete(&result->completion);
}
#endif //LINUX VERSION

static int mtk_crypto_hmac_init_pad(struct ahash_request *areq, unsigned int blocksize,
				    const u8 *key, unsigned int keylen,
				    u8 *ipad, u8 *opad)
{
	struct mtk_crypto_ahash_result result;
	struct scatterlist sg;
	int ret, i;
	u8 *keydup;

	if (keylen <= blocksize) {
		memcpy(ipad, key, keylen);
	} else {
		keydup = kmemdup(key, keylen, GFP_KERNEL);
		if (!keydup)
			return -ENOMEM;

		ahash_request_set_callback(areq, CRYPTO_TFM_REQ_MAY_BACKLOG,
					   mtk_crypto_ahash_complete, &result);
		sg_init_one(&sg, keydup, keylen);
		ahash_request_set_crypt(areq, &sg, ipad, keylen);
		init_completion(&result.completion);

		ret = crypto_ahash_digest(areq);
		if (ret == -EINPROGRESS || ret == -EBUSY) {
			wait_for_completion_interruptible(&result.completion);
			ret = result.error;
		}

		memzero_explicit(keydup, keylen);
		kfree(keydup);

		if (ret)
			return ret;

		keylen = crypto_ahash_digestsize(crypto_ahash_reqtfm(areq));
	}

	memset(ipad + keylen, 0, blocksize - keylen);
	memcpy(opad, ipad, blocksize);

	for (i = 0; i < blocksize; i++) {
		ipad[i] ^= HMAC_IPAD_VALUE;
		opad[i] ^= HMAC_OPAD_VALUE;
	}

	return 0;
}

static int mtk_crypto_hmac_init_iv(struct ahash_request *areq, unsigned int blocksize,
				   u8 *pad, void *state, bool zero)
{
	struct mtk_crypto_ahash_result result;
	struct mtk_crypto_ahash_req *req;
	struct scatterlist sg;
	int ret;

	ahash_request_set_callback(areq, CRYPTO_TFM_REQ_MAY_BACKLOG,
				   mtk_crypto_ahash_complete, &result);
	sg_init_one(&sg, pad, blocksize);
	ahash_request_set_crypt(areq, &sg, pad, blocksize);
	init_completion(&result.completion);

	ret = crypto_ahash_init(areq);
	if (ret)
		return ret;

	req = ahash_request_ctx(areq);
	req->last_req = !zero;

	if (zero)
		ret = crypto_ahash_finup(areq);
	else
		ret = crypto_ahash_update(areq);
	if (ret && ret != -EINPROGRESS && ret != -EBUSY)
		return ret;

	wait_for_completion_interruptible(&result.completion);
	if (result.error)
		return result.error;

	return crypto_ahash_export(areq, state);
}

int mtk_crypto_zero_hmac_setkey(const char *alg, const u8 *key, unsigned int keylen,
			   void *istate)
{
	struct ahash_request *areq;
	struct crypto_ahash *tfm;
	unsigned int blocksize;
	u8 *ipad, *opad;
	int ret;

	tfm = crypto_alloc_ahash(alg, 0, 0);
	if (IS_ERR(tfm))
		return PTR_ERR(tfm);

	areq = ahash_request_alloc(tfm, GFP_KERNEL);
	if (!areq) {
		ret = -ENOMEM;
		goto free_ahash;
	}

	crypto_ahash_clear_flags(tfm, ~0);
	blocksize = crypto_tfm_alg_blocksize(crypto_ahash_tfm(tfm));

	ipad = kcalloc(2, blocksize, GFP_KERNEL);
	if (!ipad) {
		ret = -ENOMEM;
		goto free_request;
	}

	opad = ipad + blocksize;

	ret = mtk_crypto_hmac_init_pad(areq, blocksize, key, keylen, ipad, opad);
	if (ret)
		goto free_ipad;

	ret = mtk_crypto_hmac_init_iv(areq, blocksize, ipad, istate, true);
	if (ret)
		goto free_ipad;

free_ipad:
	kfree(ipad);
free_request:
	kfree(areq);
free_ahash:
	crypto_free_ahash(tfm);

	return ret;
}

int mtk_crypto_hmac_setkey(const char *alg, const u8 *key, unsigned int keylen,
			   void *istate, void *ostate)
{
	struct ahash_request *areq;
	struct crypto_ahash *tfm;
	unsigned int blocksize;
	u8 *ipad, *opad;
	int ret;

	tfm = crypto_alloc_ahash(alg, 0, 0);
	if (IS_ERR(tfm))
		return PTR_ERR(tfm);

	areq = ahash_request_alloc(tfm, GFP_KERNEL);
	if (!areq) {
		ret = -ENOMEM;
		goto free_ahash;
	}

	crypto_ahash_clear_flags(tfm, ~0);
	blocksize = crypto_tfm_alg_blocksize(crypto_ahash_tfm(tfm));

	ipad = kcalloc(2, blocksize, GFP_KERNEL);
	if (!ipad) {
		ret = -ENOMEM;
		goto free_request;
	}

	opad = ipad + blocksize;

	ret = mtk_crypto_hmac_init_pad(areq, blocksize, key, keylen, ipad, opad);
	if (ret)
		goto free_ipad;

	ret = mtk_crypto_hmac_init_iv(areq, blocksize, ipad, istate, false);
	if (ret)
		goto free_ipad;

	ret = mtk_crypto_hmac_init_iv(areq, blocksize, opad, ostate, false);

free_ipad:
	kfree(ipad);
free_request:
	kfree(areq);
free_ahash:
	crypto_free_ahash(tfm);

	return ret;
}

static int mtk_crypto_hmac_alg_setkey(struct crypto_ahash *tfm, const u8 *key,
				      unsigned int keylen, const char *alg,
				      unsigned int state_sz)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(crypto_ahash_tfm(tfm));
	struct mtk_crypto_ahash_export_state istate, ostate, zeroi;
	int ret;

	memset(&zeroi, 0, sizeof(struct mtk_crypto_ahash_export_state));
	memset(&istate, 0, sizeof(struct mtk_crypto_ahash_export_state));
	memset(&ostate, 0, sizeof(struct mtk_crypto_ahash_export_state));
	ret = mtk_crypto_hmac_setkey(alg, key, keylen, &istate, &ostate);
	if (ret)
		return ret;

	ret = mtk_crypto_zero_hmac_setkey(alg, key, keylen, &zeroi);
	if (ret)
		return ret;

	memcpy(ctx->zero_hmac, &zeroi.state, state_sz);
	memcpy(ctx->ipad, &istate.state, state_sz);
	memcpy(ctx->opad, &ostate.state, state_sz);
	ctx->ipad_sa = istate.sa_pointer;
	ctx->ipad_token = istate.token_context;
	ctx->opad_sa = ostate.sa_pointer;
	ctx->opad_token = ostate.token_context;

	return 0;
}

static int mtk_crypto_hmac_sha1_setkey(struct crypto_ahash *tfm, const u8 *key,
				       unsigned int keylen)
{
	return mtk_crypto_hmac_alg_setkey(tfm, key, keylen, "crypto-eip-sha1",
					  SHA1_DIGEST_SIZE);
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha1 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_hmac_sha1_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_hmac_sha1_digest,
		.setkey = mtk_crypto_hmac_sha1_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "hmac(sha1)",
				.cra_driver_name = "crypto-eip-hmac-sha1",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA1_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_sha256_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	ctx->alg = MTK_CRYPTO_ALG_SHA256;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA256_DIGEST_SIZE;
	req->digest_sz = SHA256_DIGEST_SIZE;
	req->block_sz = SHA256_BLOCK_SIZE;

	return 0;
}

static int mtk_crypto_sha256_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_sha256_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_sha256 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_sha256_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_sha256_digest,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "sha256",
				.cra_driver_name = "crypto-eip-sha256",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA256_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_sha224_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	ctx->alg = MTK_CRYPTO_ALG_SHA224;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA256_DIGEST_SIZE;
	req->digest_sz = SHA256_DIGEST_SIZE;
	req->block_sz = SHA256_BLOCK_SIZE;

	return 0;
}

static int mtk_crypto_sha224_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_sha224_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_sha224 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_sha224_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_sha224_digest,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA224_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "sha224",
				.cra_driver_name = "crypto-eip-sha224",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA224_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_hmac_sha224_setkey(struct crypto_ahash *tfm, const u8 *key,
				    unsigned int keylen)
{
	return mtk_crypto_hmac_alg_setkey(tfm, key, keylen, "crypto-eip-sha224",
					  SHA256_DIGEST_SIZE);
}

static int mtk_crypto_hmac_sha224_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	memcpy(req->state, ctx->ipad, SHA224_DIGEST_SIZE);

	req->sa_pointer = ctx->ipad_sa;
	req->token_context = ctx->ipad_token;
	req->not_first = true;
	req->len = SHA224_BLOCK_SIZE;
	req->processed = SHA224_BLOCK_SIZE;

	ctx->alg = MTK_CRYPTO_ALG_SHA224;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA224_DIGEST_SIZE;
	req->digest_sz = SHA224_DIGEST_SIZE;
	req->block_sz = SHA224_BLOCK_SIZE;
	req->hmac = true;

	return 0;
}

static int mtk_crypto_hmac_sha224_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_hmac_sha224_init(areq);

	if (ret)
		return ret;
	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha224 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_hmac_sha224_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_hmac_sha224_digest,
		.setkey = mtk_crypto_hmac_sha224_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA224_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "hmac(sha224)",
				.cra_driver_name = "crypto-eip-hmac-sha224",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA224_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_hmac_sha256_setkey(struct crypto_ahash *tfm, const u8 *key,
					 unsigned int keylen)
{
	return mtk_crypto_hmac_alg_setkey(tfm, key, keylen, "crypto-eip-sha256",
					  SHA256_DIGEST_SIZE);
}

static int mtk_crypto_hmac_sha256_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	memcpy(req->state, ctx->ipad, SHA256_DIGEST_SIZE);
	req->sa_pointer = ctx->ipad_sa;
	req->token_context = ctx->ipad_token;
	req->not_first = true;
	req->len = SHA256_BLOCK_SIZE;
	req->processed = SHA256_BLOCK_SIZE;

	ctx->alg = MTK_CRYPTO_ALG_SHA256;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA256_DIGEST_SIZE;
	req->digest_sz = SHA256_DIGEST_SIZE;
	req->block_sz = SHA256_BLOCK_SIZE;
	req->hmac = true;

	return 0;
}

static int mtk_crypto_hmac_sha256_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_hmac_sha256_init(areq);

	if (ret)
		return ret;
	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha256 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_hmac_sha256_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_hmac_sha256_digest,
		.setkey = mtk_crypto_hmac_sha256_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "hmac(sha256)",
				.cra_driver_name = "crypto-eip-hmac-sha256",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA256_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_sha512_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	ctx->alg = MTK_CRYPTO_ALG_SHA512;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA512_DIGEST_SIZE;
	req->digest_sz = SHA512_DIGEST_SIZE;
	req->block_sz = SHA512_BLOCK_SIZE;

	return 0;
}

static int mtk_crypto_sha512_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_sha512_init(areq);

	if (ret)
		return ret;
	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_sha512 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_sha512_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_sha512_digest,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_ctx),
			.base = {
				.cra_name = "sha512",
				.cra_driver_name = "crypto-eip-sha512",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA512_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_sha384_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	ctx->alg = MTK_CRYPTO_ALG_SHA384;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA512_DIGEST_SIZE;
	req->digest_sz = SHA512_DIGEST_SIZE;
	req->block_sz = SHA384_BLOCK_SIZE;

	return 0;
}

static int mtk_crypto_sha384_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_sha384_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_sha384 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_sha384_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_sha384_digest,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_ctx),
			.base = {
				.cra_name = "sha384",
				.cra_driver_name = "crypto-eip-sha384",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA384_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_hmac_sha512_setkey(struct crypto_ahash *tfm, const u8 *key,
					 unsigned int keylen)
{
	return mtk_crypto_hmac_alg_setkey(tfm, key, keylen, "crypto-eip-sha512",
					  SHA512_DIGEST_SIZE);
}

static int mtk_crypto_hmac_sha512_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	memcpy(req->state, ctx->ipad, SHA512_DIGEST_SIZE);
	req->sa_pointer = ctx->ipad_sa;
	req->token_context = ctx->ipad_token;
	req->not_first = true;
	req->len = SHA512_BLOCK_SIZE;
	req->processed = SHA512_BLOCK_SIZE;

	ctx->alg = MTK_CRYPTO_ALG_SHA512;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA512_DIGEST_SIZE;
	req->digest_sz = SHA512_DIGEST_SIZE;
	req->block_sz = SHA512_BLOCK_SIZE;
	req->hmac = true;

	return 0;
}

static int mtk_crypto_hmac_sha512_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_hmac_sha512_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha512 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_hmac_sha512_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_hmac_sha512_digest,
		.setkey = mtk_crypto_hmac_sha512_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "hmac(sha512)",
				.cra_driver_name = "crypto-eip-hmac-sha512",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA512_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_hmac_sha384_setkey(struct crypto_ahash *tfm, const u8 *key,
					 unsigned int keylen)
{
	return mtk_crypto_hmac_alg_setkey(tfm, key, keylen, "crypto-eip-sha384",
					  SHA512_DIGEST_SIZE);
}

static int mtk_crypto_hmac_sha384_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	memcpy(req->state, ctx->ipad, SHA384_DIGEST_SIZE);
	req->sa_pointer = ctx->ipad_sa;
	req->token_context = ctx->ipad_token;
	req->not_first = true;
	req->len = SHA384_BLOCK_SIZE;
	req->processed = SHA384_BLOCK_SIZE;

	ctx->alg = MTK_CRYPTO_ALG_SHA384;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = SHA384_DIGEST_SIZE;
	req->digest_sz = SHA384_DIGEST_SIZE;
	req->block_sz = SHA384_BLOCK_SIZE;
	req->hmac = true;

	return 0;
}

static int mtk_crypto_hmac_sha384_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_hmac_sha384_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha384 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_hmac_sha384_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_hmac_sha384_digest,
		.setkey = mtk_crypto_hmac_sha384_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "hmac(sha384)",
				.cra_driver_name = "crypto-eip-hmac-sha384",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = SHA384_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_md5_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	ctx->alg = MTK_CRYPTO_ALG_MD5;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = MD5_DIGEST_SIZE;
	req->digest_sz = MD5_DIGEST_SIZE;
	req->block_sz = MD5_HMAC_BLOCK_SIZE;

	return 0;
}

static int mtk_crypto_md5_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_md5_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_md5 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_md5_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_md5_digest,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = MD5_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "md5",
				.cra_driver_name = "crypto-eip-md5",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = MD5_HMAC_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_hmac_md5_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));

	memcpy(req->state, ctx->ipad, MD5_DIGEST_SIZE);
	req->sa_pointer = ctx->ipad_sa;
	req->token_context = ctx->ipad_token;
	req->not_first = true;
	req->len = MD5_HMAC_BLOCK_SIZE;
	req->processed = MD5_HMAC_BLOCK_SIZE;

	ctx->alg = MTK_CRYPTO_ALG_MD5;
	req->digest = MTK_CRYPTO_DIGEST_PRECOMPUTED;
	req->state_sz = MD5_DIGEST_SIZE;
	req->digest_sz = MD5_DIGEST_SIZE;
	req->block_sz = MD5_HMAC_BLOCK_SIZE;
	req->hmac = true;

	return 0;
}

static int mtk_crypto_hmac_md5_setkey(struct crypto_ahash *tfm, const u8 *key,
					unsigned int keylen)
{
	return mtk_crypto_hmac_alg_setkey(tfm, key, keylen, "crypto-eip-md5",
					  MD5_DIGEST_SIZE);
}

static int mtk_crypto_hmac_md5_digest(struct ahash_request *areq)
{
	int ret = mtk_crypto_hmac_md5_init(areq);

	if (ret)
		return ret;

	return mtk_crypto_ahash_finup(areq);
}

struct mtk_crypto_alg_template mtk_crypto_hmac_md5 = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_hmac_md5_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_hmac_md5_digest,
		.setkey = mtk_crypto_hmac_md5_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = MD5_DIGEST_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "hmac(md5)",
				.cra_driver_name = "crypto-eip-hmac-md5",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = MD5_HMAC_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_ahash_cra_init,
				.cra_exit = mtk_crypto_ahash_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_cbcmac_init(struct ahash_request *areq)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));
	struct mtk_crypto_ahash_req *req = ahash_request_ctx(areq);

	memset(req, 0, sizeof(*req));
	memset(req->state, 0, sizeof(u32) * (SHA512_DIGEST_SIZE / sizeof(u32)));

	req->len = AES_BLOCK_SIZE;
	req->processed = AES_BLOCK_SIZE;

	req->digest = MTK_CRYPTO_DIGEST_XCM;
	req->state_sz = ctx->key_sz;
	req->digest_sz = AES_BLOCK_SIZE;
	req->block_sz = AES_BLOCK_SIZE;
	req->xcbcmac = true;

	return 0;
}

static int mtk_crypto_cbcmac_digest(struct ahash_request *areq)
{
	return mtk_crypto_cbcmac_init(areq) ?: mtk_crypto_ahash_finup(areq);
}

static int mtk_crypto_xcbcmac_setkey(struct crypto_ahash *tfm, const u8 *key,
				     unsigned int len)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(crypto_ahash_tfm(tfm));
	struct crypto_aes_ctx aes;
	u32 key_tmp[3 * AES_BLOCK_SIZE / sizeof(u32)];
	int ret, i;

	ret = aes_expandkey(&aes, key, len);
	if (ret)
		return ret;

	crypto_cipher_clear_flags(ctx->kaes, CRYPTO_TFM_REQ_MASK);
	crypto_cipher_set_flags(ctx->kaes, crypto_ahash_get_flags(tfm) & CRYPTO_TFM_REQ_MASK);

	ret = crypto_cipher_setkey(ctx->kaes, key, len);
	if (ret)
		return ret;

	crypto_cipher_encrypt_one(ctx->kaes, (u8 *)key_tmp + 2 * AES_BLOCK_SIZE,
		"\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1\x1");
	crypto_cipher_encrypt_one(ctx->kaes, (u8 *)key_tmp,
		"\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2");
	crypto_cipher_encrypt_one(ctx->kaes, (u8 *)key_tmp + AES_BLOCK_SIZE,
		"\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3\x3");

	for (i = 0; i < 3 * AES_BLOCK_SIZE / sizeof(u32); i++)
		ctx->ipad[i] =
			(__force u32)cpu_to_be32(key_tmp[i]);

	crypto_cipher_clear_flags(ctx->kaes, CRYPTO_TFM_REQ_MASK);
	crypto_cipher_set_flags(ctx->kaes, crypto_ahash_get_flags(tfm) &
				CRYPTO_TFM_REQ_MASK);
	ret = crypto_cipher_setkey(ctx->kaes, (u8 *)key_tmp + 2 * AES_BLOCK_SIZE,
					AES_MIN_KEY_SIZE);

	if (ret)
		return ret;

	ctx->alg = MTK_CRYPTO_ALG_XCBC;
	ctx->key_sz = AES_MIN_KEY_SIZE + 2 * AES_BLOCK_SIZE;
	ctx->cbcmac = false;

	memzero_explicit(&aes, sizeof(aes));
	return 0;
}

static int mtk_crypto_xcbcmac_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_ahash_cra_init(tfm);
	ctx->kaes = crypto_alloc_cipher("aes", 0, 0);
	return PTR_ERR_OR_ZERO(ctx->kaes);
}

static void mtk_crypto_xcbcmac_cra_exit(struct crypto_tfm *tfm)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(tfm);

	crypto_free_cipher(ctx->kaes);
	mtk_crypto_ahash_cra_exit(tfm);
}

struct mtk_crypto_alg_template mtk_crypto_xcbcmac = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_cbcmac_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_cbcmac_digest,
		.setkey = mtk_crypto_xcbcmac_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = AES_BLOCK_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "xcbc(aes)",
				.cra_driver_name = "crypto-eip-xcbc-aes",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
					     CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = AES_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_xcbcmac_cra_init,
				.cra_exit = mtk_crypto_xcbcmac_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

static int mtk_crypto_cmac_setkey(struct crypto_ahash *tfm, const u8 *key,
				unsigned int len)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(crypto_ahash_tfm(tfm));
	struct crypto_aes_ctx aes;
	__be64 consts[4];
	u64 _const[2];
	u8 msb_mask, gfmask;
	int ret, i;

	ret = aes_expandkey(&aes, key, len);
	if (ret)
		return ret;

	crypto_cipher_clear_flags(ctx->kaes, CRYPTO_TFM_REQ_MASK);
	crypto_cipher_set_flags(ctx->kaes, crypto_ahash_get_flags(tfm) &
				CRYPTO_TFM_REQ_MASK);
	ret = crypto_cipher_setkey(ctx->kaes, key, len);
	if (ret)
		return ret;

	/* code below borrowed from crypto/cmac.c */
	/* encrypt the zero block */
	memset(consts, 0, AES_BLOCK_SIZE);
	crypto_cipher_encrypt_one(ctx->kaes, (u8 *) consts, (u8 *) consts);

	gfmask = 0x87;
	_const[0] = be64_to_cpu(consts[1]);
	_const[1] = be64_to_cpu(consts[0]);

	/* gf(2^128) multiply zero-ciphertext with u and u^2 */
	for (i = 0; i < 4; i += 2) {
		msb_mask = ((s64)_const[1] >> 63) & gfmask;
		_const[1] = (_const[1] << 1) | (_const[0] >> 63);
		_const[0] = (_const[0] << 1) ^ msb_mask;

		consts[i + 0] = cpu_to_be64(_const[1]);
		consts[i + 1] = cpu_to_be64(_const[0]);
	}
	/* end of code borrowed from crypto/cmac.c */

	for (i = 0; i < 2 * AES_BLOCK_SIZE / sizeof(u32); i++)
		ctx->ipad[i] = (__force __le32)cpu_to_be32(((u32 *) consts)[i]);
	memcpy((uint8_t *) ctx->ipad + 2 * AES_BLOCK_SIZE, key, len);

	if (len == AES_KEYSIZE_192) {
		ctx->alg = MTK_CRYPTO_ALG_CMAC_192;
		ctx->key_sz = 24 + 2 * AES_BLOCK_SIZE;
	} else if (len == AES_KEYSIZE_256) {
		ctx->alg = MTK_CRYPTO_ALG_CMAC_256;
		ctx->key_sz = AES_MAX_KEY_SIZE + 2 * AES_BLOCK_SIZE;
	} else {
		ctx->alg    = MTK_CRYPTO_ALG_CMAC_128;
		ctx->key_sz = AES_MIN_KEY_SIZE + 2 * AES_BLOCK_SIZE;
	}
	ctx->cbcmac = false;

	memzero_explicit(&aes, sizeof(aes));
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_cmac = {
	.type = MTK_CRYPTO_ALG_TYPE_AHASH,
	.alg.ahash = {
		.init = mtk_crypto_cbcmac_init,
		.update = mtk_crypto_ahash_update,
		.final = mtk_crypto_ahash_final,
		.finup = mtk_crypto_ahash_finup,
		.digest = mtk_crypto_cbcmac_digest,
		.setkey = mtk_crypto_cmac_setkey,
		.export = mtk_crypto_ahash_export,
		.import = mtk_crypto_ahash_import,
		.halg = {
			.digestsize = AES_BLOCK_SIZE,
			.statesize = sizeof(struct mtk_crypto_ahash_export_state),
			.base = {
				.cra_name = "cmac(aes)",
				.cra_driver_name = "crypto-eip-cmac-aes",
				.cra_priority = MTK_CRYPTO_PRIORITY,
				.cra_flags = CRYPTO_ALG_ASYNC |
						CRYPTO_ALG_KERN_DRIVER_ONLY,
				.cra_blocksize = AES_BLOCK_SIZE,
				.cra_ctxsize = sizeof(struct mtk_crypto_ahash_ctx),
				.cra_init = mtk_crypto_xcbcmac_cra_init,
				.cra_exit = mtk_crypto_xcbcmac_cra_exit,
				.cra_module = THIS_MODULE,
			},
		},
	},
};

