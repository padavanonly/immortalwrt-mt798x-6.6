// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/bitops.h>
#include <crypto/aes.h>
#include <crypto/authenc.h>
#include <crypto/gcm.h>
#include <crypto/ghash.h>
#include <crypto/ctr.h>
#include <crypto/xts.h>
#include <crypto/internal/des.h>
#include <crypto/skcipher.h>
#include <crypto/internal/skcipher.h>
#include <crypto/aead.h>
#include <crypto/internal/aead.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/lookaside.h"
#include "crypto-eip/internal.h"

static int mtk_crypto_skcipher_send(struct crypto_async_request *async)
{
	struct skcipher_request *req = skcipher_request_cast(async);
	struct mtk_crypto_cipher_req *mtk_req = skcipher_request_ctx(req);
	struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);
	unsigned int blksize = crypto_skcipher_blocksize(skcipher);
	int ret = 0;

	if (!IS_ALIGNED(req->cryptlen, blksize)) {
		ret = -EINVAL;
		goto end;
	}

	ret = mtk_crypto_basic_cipher(async, mtk_req, req->src, req->dst, req->cryptlen,
				0, 0, req->iv, crypto_skcipher_ivsize(skcipher));

end:
	if (ret != 0) {
		local_bh_disable();
		async->complete(async, ret);
		local_bh_enable();
	}

	return ret;
}

static int mtk_crypto_skcipher_handle_result(struct mtk_crypto_result *res, int err)
{
	struct crypto_async_request *async = res->async;
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(async->tfm);
	struct skcipher_request *req = skcipher_request_cast(async);
	struct mtk_crypto_cipher_req *mtk_req = skcipher_request_ctx(req);
	struct crypto_skcipher *skcipher = crypto_skcipher_reqtfm(req);

	if (ctx->mode == MTK_CRYPTO_MODE_CBC && mtk_req->direction == MTK_CRYPTO_ENCRYPT)
		sg_pcopy_to_buffer(req->dst, mtk_req->nr_dst, req->iv, crypto_skcipher_ivsize(skcipher),
					req->cryptlen - crypto_skcipher_ivsize(skcipher));

	if (req->src == req->dst) {
		dma_unmap_sg(crypto_dev, req->src, mtk_req->nr_src, DMA_BIDIRECTIONAL);
	} else {
		dma_unmap_sg(crypto_dev, req->src, mtk_req->nr_src, DMA_TO_DEVICE);
		dma_unmap_sg(crypto_dev, req->dst, mtk_req->nr_dst, DMA_FROM_DEVICE);
	}

	local_bh_disable();
	async->complete(async, err);
	local_bh_enable();

	crypto_free_sglist(res->eip.pkt_handle);
	crypto_free_sglist(res->dst);

	return 0;
}

static int mtk_crypto_aead_send(struct crypto_async_request *async)
{
	struct aead_request *req = aead_request_cast(async);
	struct crypto_aead *tfm = crypto_aead_reqtfm(req);
	struct mtk_crypto_cipher_req *mtk_req = aead_request_ctx(req);
	int ret;

	ret = mtk_crypto_basic_cipher(async, mtk_req, req->src, req->dst, req->cryptlen,
				req->assoclen, crypto_aead_authsize(tfm), req->iv,
				crypto_aead_ivsize(tfm));

	if (ret != 0) {
		local_bh_disable();
		async->complete(async, ret);
		local_bh_enable();
	}

	return ret;
}

static int mtk_crypto_aead_handle_result(struct mtk_crypto_result *res, int err)
{
	struct crypto_async_request *async = res->async;
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(async->tfm);
	struct aead_request *req = aead_request_cast(async);
	struct mtk_crypto_cipher_req *mtk_req = aead_request_ctx(req);
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	int pkt_size;
	uint8_t *temp;

	if (ctx->mode == MTK_CRYPTO_MODE_CCM && mtk_req->direction == MTK_CRYPTO_DECRYPT) {
		pkt_size = req->cryptlen + req->assoclen - crypto_aead_authsize(aead);

		temp = kmalloc(pkt_size, GFP_KERNEL);
		if (!temp) {
			CRYPTO_ERR("no enough memory for result\n");
			goto free_dma;
		}
		memset(temp, 0, pkt_size);
		sg_copy_to_buffer(req->dst, mtk_req->nr_dst, temp + 8, pkt_size - 8);
		sg_copy_from_buffer(req->dst, mtk_req->nr_dst, temp, pkt_size);
		kfree(temp);
	}

free_dma:
	if (req->src == req->dst) {
		dma_unmap_sg(crypto_dev, req->src, mtk_req->nr_src, DMA_BIDIRECTIONAL);
	} else {
		dma_unmap_sg(crypto_dev, req->src, mtk_req->nr_src, DMA_TO_DEVICE);
		dma_unmap_sg(crypto_dev, req->dst, mtk_req->nr_dst, DMA_FROM_DEVICE);
	}

	crypto_free_sglist(res->eip.pkt_handle);
	crypto_free_sglist(res->dst);

	local_bh_disable();
	async->complete(async, err);
	local_bh_enable();
	return 0;
}

static int mtk_crypto_skcipher_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct mtk_crypto_alg_template *tmpl =
		container_of(tfm->__crt_alg, struct mtk_crypto_alg_template,
				alg.skcipher.base);

	crypto_skcipher_set_reqsize(__crypto_skcipher_cast(tfm),
					sizeof(struct mtk_crypto_cipher_req));
	ctx->priv = tmpl->priv;

	ctx->base.send = mtk_crypto_skcipher_send;
	ctx->base.handle_result = mtk_crypto_skcipher_handle_result;
	ctx->base.ring = -1;
	ctx->enc.valid = 0;
	ctx->dec.valid = 0;

	timer_setup(&ctx->poll_timer, mtk_crypto_req_expired_timer, 0);

	return 0;
}

static int mtk_crypto_skcipher_aes_cbc_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_AES;
	ctx->blocksz = AES_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_CBC;
	return 0;
}

static int mtk_crypto_skcipher_aes_ecb_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_AES;
	ctx->mode = MTK_CRYPTO_MODE_ECB;
	ctx->blocksz = 0;
	return 0;
}

static int mtk_crypto_queue_req(struct crypto_async_request *base,
				struct mtk_crypto_cipher_req *mtk_req,
				enum mtk_crypto_cipher_direction dir)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(base->tfm);
	struct crypto_aead *tfm = crypto_aead_reqtfm(aead_request_cast(base));
	struct mtk_crypto_priv *priv = ctx->priv;
	struct mtk_crypto_engine_data *data;
	int ring;
	int ret;

	mtk_req->direction = dir;

	if (ctx->base.ring < 0) {
		ring = mtk_crypto_select_ring(priv);
		ctx->base.ring = ring;
	} else
		ring = ctx->base.ring;


	if (dir == MTK_CRYPTO_ENCRYPT)
		data = &ctx->enc;
	else
		data = &ctx->dec;

	if (!data->valid)
		mtk_crypto_ddk_alloc_buff(ctx, dir, crypto_aead_authsize(tfm), data);

	spin_lock_bh(&priv->mtk_eip_ring[ring].queue_lock);
	ret = crypto_enqueue_request(&priv->mtk_eip_ring[ring].queue, base);
	spin_unlock_bh(&priv->mtk_eip_ring[ring].queue_lock);

	queue_work(priv->mtk_eip_ring[ring].workqueue,
			&priv->mtk_eip_ring[ring].work_data.work);

	return ret;
}

static int mtk_crypto_decrypt(struct skcipher_request *req)
{
	return mtk_crypto_queue_req(&req->base, skcipher_request_ctx(req), MTK_CRYPTO_DECRYPT);
}

static int mtk_crypto_encrypt(struct skcipher_request *req)
{
	return mtk_crypto_queue_req(&req->base, skcipher_request_ctx(req), MTK_CRYPTO_ENCRYPT);
}

static int mtk_crypto_skcipher_aes_setkey(struct crypto_skcipher *ctfm,
					  const u8 *key, unsigned int len)
{
	struct crypto_tfm *tfm = crypto_skcipher_tfm(ctfm);
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_aes_ctx aes;
	int ret;
	int i;

	ret = aes_expandkey(&aes, key, len);
	if (ret)
		return ret;

	for (i = 0; i < len / sizeof(u32); i++)
		ctx->key[i] = cpu_to_le32(aes.key_enc[i]);

	ctx->key_len = len;

	memzero_explicit(&aes, sizeof(aes));
	ctx->enc.valid = 0;
	ctx->dec.valid = 0;
	return 0;
}

static void mtk_crypto_skcipher_cra_exit(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	if (ctx->enc.sa_handle) {
		crypto_free_sa(ctx->enc.sa_handle, ctx->base.ring);
		crypto_free_token(ctx->enc.token_handle);
		kfree(ctx->enc.token_context);
	}

	if (ctx->dec.sa_handle) {
		crypto_free_sa(ctx->dec.sa_handle, ctx->base.ring);
		crypto_free_token(ctx->dec.token_handle);
		kfree(ctx->dec.token_context);
	}

	memzero_explicit(ctx->key, sizeof(ctx->key));
}


struct mtk_crypto_alg_template mtk_crypto_cbc_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_skcipher_aes_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = AES_MIN_KEY_SIZE,
		.max_keysize = AES_MAX_KEY_SIZE,
		.ivsize = AES_BLOCK_SIZE,
		.base = {
			.cra_name = "cbc(aes)",
			.cra_driver_name = "crypto-eip-cbc-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_init = mtk_crypto_skcipher_aes_cbc_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_skcipher_aes_cfb_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_AES;
	ctx->blocksz = AES_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_CFB;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_cfb_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_skcipher_aes_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = AES_MIN_KEY_SIZE,
		.max_keysize = AES_MAX_KEY_SIZE,
		.ivsize = AES_BLOCK_SIZE,
		.base = {
			.cra_name = "cfb(aes)",
			.cra_driver_name = "crypto-eip-cfb-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_skcipher_aes_cfb_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_skcipher_aes_ofb_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_AES;
	ctx->blocksz = AES_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_OFB;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_ofb_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_skcipher_aes_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = AES_MIN_KEY_SIZE,
		.max_keysize = AES_MAX_KEY_SIZE,
		.ivsize = AES_BLOCK_SIZE,
		.base = {
			.cra_name = "ofb(aes)",
			.cra_driver_name = "crypto-eip-ofb-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_skcipher_aes_ofb_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

struct mtk_crypto_alg_template mtk_crypto_ecb_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_skcipher_aes_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = AES_MIN_KEY_SIZE,
		.max_keysize = AES_MAX_KEY_SIZE,
		.base = {
			.cra_name = "ecb(aes)",
			.cra_driver_name = "crypto-eip-ecb-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_init = mtk_crypto_skcipher_aes_ecb_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_skcipher_aesctr_setkey(struct crypto_skcipher *ctfm,
					     const u8 *key, unsigned int len)
{
	struct crypto_tfm *tfm = crypto_skcipher_tfm(ctfm);
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_aes_ctx aes;
	int ret;
	int i;
	unsigned int keylen;

	ctx->nonce = *(u32 *)(key + len - CTR_RFC3686_NONCE_SIZE);
	keylen = len - CTR_RFC3686_NONCE_SIZE;
	ret = aes_expandkey(&aes, key, keylen);
	if (ret)
		return ret;

	for (i = 0; i < keylen / sizeof(u32); i++)
		ctx->key[i] = cpu_to_le32(aes.key_enc[i]);

	ctx->key_len = keylen;

	memzero_explicit(&aes, sizeof(aes));
	ctx->enc.valid = 0;
	ctx->dec.valid = 0;
	return 0;
}

static int mtk_crypto_skcipher_aes_ctr_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_AES;
	ctx->blocksz = AES_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_CTR;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_ctr_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_skcipher_aesctr_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = AES_MIN_KEY_SIZE + CTR_RFC3686_NONCE_SIZE,
		.max_keysize = AES_MAX_KEY_SIZE + CTR_RFC3686_NONCE_SIZE,
		.ivsize = CTR_RFC3686_IV_SIZE,
		.base = {
			.cra_name = "rfc3686(ctr(aes))",
			.cra_driver_name = "crypto-eip-ctr-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_skcipher_aes_ctr_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_des_setkey(struct crypto_skcipher *ctfm, const u8 *key,
				 unsigned int len)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_skcipher_ctx(ctfm);
	int ret;

	ret = verify_skcipher_des_key(ctfm, key);
	if (ret)
		return ret;
	memcpy(ctx->key, key, len);
	ctx->key_len = len;
	ctx->enc.valid = 0;
	ctx->dec.valid = 0;

	return 0;
}

static int mtk_crypto_skcipher_des_cbc_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_DES;
	ctx->blocksz = DES_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_CBC;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_cbc_des = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_des_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = DES_KEY_SIZE,
		.max_keysize = DES_KEY_SIZE,
		.ivsize = DES_BLOCK_SIZE,
		.base = {
			.cra_name = "cbc(des)",
			.cra_driver_name = "crypto-eip-cbc-des",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_skcipher_des_cbc_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_skcipher_des_ecb_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_DES;
	ctx->mode = MTK_CRYPTO_MODE_ECB;
	ctx->blocksz = 0;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_ecb_des = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_des_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = DES_KEY_SIZE,
		.max_keysize = DES_KEY_SIZE,
		.base = {
			.cra_name = "ecb(des)",
			.cra_driver_name = "crypto-eip-ecb-des",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_skcipher_des_ecb_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_des3_ede_setkey(struct crypto_skcipher *ctfm,
				      const u8 *key, unsigned int len)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_skcipher_ctx(ctfm);
	int err;

	err = verify_skcipher_des3_key(ctfm, key);
	if (err)
		return err;

	memcpy(ctx->key, key, len);
	ctx->key_len = len;
	ctx->enc.valid = 0;
	ctx->dec.valid = 0;
	return 0;
}

static int mtk_crypto_skcipher_des3_cbc_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->mode = MTK_CRYPTO_MODE_CBC;
	ctx->blocksz = 0;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_cbc_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_des3_ede_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = DES3_EDE_KEY_SIZE,
		.max_keysize = DES3_EDE_KEY_SIZE,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.base = {
			.cra_name = "cbc(des3_ede)",
			.cra_driver_name = "crypto-eip-cbc-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_skcipher_des3_cbc_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_skcipher_des3_ecb_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_skcipher_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->mode = MTK_CRYPTO_MODE_ECB;
	ctx->blocksz = 0;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_ecb_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_SKCIPHER,
	.alg.skcipher = {
		.setkey = mtk_crypto_des3_ede_setkey,
		.encrypt = mtk_crypto_encrypt,
		.decrypt = mtk_crypto_decrypt,
		.min_keysize = DES3_EDE_KEY_SIZE,
		.max_keysize = DES3_EDE_KEY_SIZE,
		.base = {
			.cra_name = "ecb(des3_ede)",
			.cra_driver_name = "crypto-eip-ecb-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_skcipher_des3_ecb_cra_init,
			.cra_exit = mtk_crypto_skcipher_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_encrypt(struct aead_request *req)
{
	struct mtk_crypto_cipher_req *creq = aead_request_ctx(req);

	return mtk_crypto_queue_req(&req->base, creq, MTK_CRYPTO_ENCRYPT);
}

static int mtk_crypto_aead_decrypt(struct aead_request *req)
{
	struct mtk_crypto_cipher_req *creq = aead_request_ctx(req);

	return mtk_crypto_queue_req(&req->base, creq, MTK_CRYPTO_DECRYPT);
}

static void mtk_crypto_aead_cra_exit(struct crypto_tfm *tfm)
{
	mtk_crypto_skcipher_cra_exit(tfm);
}

static int mtk_crypto_aead_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct mtk_crypto_alg_template *tmpl =
		container_of(tfm->__crt_alg, struct mtk_crypto_alg_template, alg.aead.base);

	crypto_aead_set_reqsize(__crypto_aead_cast(tfm), sizeof(struct mtk_crypto_cipher_req));

	ctx->priv = tmpl->priv;

	ctx->alg = MTK_CRYPTO_AES;
	ctx->blocksz = AES_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_CBC;
	ctx->aead = true;
	ctx->base.send = mtk_crypto_aead_send;
	ctx->base.handle_result = mtk_crypto_aead_handle_result;
	ctx->base.ring = -1;
	ctx->enc.valid = 0;
	ctx->dec.valid = 0;

	timer_setup(&ctx->poll_timer, mtk_crypto_req_expired_timer, 0);

	return 0;
}

static int mtk_crypto_aead_sha1_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_SHA1;
	ctx->state_sz = SHA1_DIGEST_SIZE;
	return 0;
}

static int mtk_crypto_aead_setkey(struct crypto_aead *ctfm, const u8 *key, unsigned int len)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(ctfm);
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct mtk_crypto_ahash_export_state istate, ostate;
	struct crypto_authenc_keys keys;
	struct crypto_aes_ctx aes;
	int err = -EINVAL, i;

	memset(&istate, 0, sizeof(struct mtk_crypto_ahash_export_state));
	memset(&ostate, 0, sizeof(struct mtk_crypto_ahash_export_state));

	if (unlikely(crypto_authenc_extractkeys(&keys, key, len)))
		goto badkey;

	if (ctx->mode == MTK_CRYPTO_MODE_CTR) {
		if (unlikely(keys.enckeylen < CTR_RFC3686_NONCE_SIZE))
			goto badkey;

		ctx->nonce = *(u32 *)(keys.enckey + keys.enckeylen - CTR_RFC3686_NONCE_SIZE);
		keys.enckeylen -= CTR_RFC3686_NONCE_SIZE;
	}

	switch (ctx->alg) {
	case MTK_CRYPTO_AES:
		err = aes_expandkey(&aes, keys.enckey, keys.enckeylen);
		if (unlikely(err))
			goto badkey;
		break;
	case MTK_CRYPTO_DES:
		err = verify_aead_des_key(ctfm, keys.enckey, keys.enckeylen);
		if (unlikely(err))
			goto badkey;
		break;
	case MTK_CRYPTO_3DES:
		err = verify_aead_des3_key(ctfm, keys.enckey, keys.enckeylen);
		if (unlikely(err))
			goto badkey;
		break;
	default:
		CRYPTO_ERR("aead: unsupported cipher algorithm\n");
		goto badkey;
	}

	switch (ctx->hash_alg) {
	case MTK_CRYPTO_ALG_SHA1:
		err = mtk_crypto_hmac_setkey("crypto-eip-sha1", keys.authkey,
					   keys.authkeylen, &istate, &ostate);
		if (err)
			goto badkey;
		break;
	case MTK_CRYPTO_ALG_SHA224:
		if (mtk_crypto_hmac_setkey("crypto-eip-sha224", keys.authkey,
					   keys.authkeylen, &istate, &ostate))
			goto badkey;
		break;
	case MTK_CRYPTO_ALG_SHA256:
		if (mtk_crypto_hmac_setkey("crypto-eip-sha256", keys.authkey,
					   keys.authkeylen, &istate, &ostate))
			goto badkey;
		break;
	case MTK_CRYPTO_ALG_SHA384:
		if (mtk_crypto_hmac_setkey("crypto-eip-sha384", keys.authkey,
					   keys.authkeylen, &istate, &ostate))
			goto badkey;
		break;
	case MTK_CRYPTO_ALG_SHA512:
		if (mtk_crypto_hmac_setkey("crypto-eip-sha512", keys.authkey,
					   keys.authkeylen, &istate, &ostate))
			goto badkey;
		break;
	case MTK_CRYPTO_ALG_MD5:
		if (mtk_crypto_hmac_setkey("crypto-eip-md5", keys.authkey,
					   keys.authkeylen, &istate, &ostate))
			goto badkey;
		break;
	default:
		CRYPTO_ERR("aead: unsupported hash algorithm\n");
		goto badkey;
	}

	for (i = 0; i < keys.enckeylen / sizeof(u32); i++)
		ctx->key[i] = cpu_to_le32(((u32 *)keys.enckey)[i]);
	ctx->key_len = keys.enckeylen;

	memcpy(ctx->ipad, &istate.state, ctx->state_sz);
	memcpy(ctx->opad, &ostate.state, ctx->state_sz);

	if (istate.sa_pointer)
		crypto_free_sa(istate.sa_pointer, istate.ring);
	kfree(istate.token_context);
	if (ostate.sa_pointer)
		crypto_free_sa(ostate.sa_pointer, ostate.ring);
	kfree(ostate.token_context);

	memzero_explicit(&keys, sizeof(keys));

	ctx->enc.valid = 0;
	ctx->dec.valid = 0;

	return 0;

badkey:
	memzero_explicit(&keys, sizeof(keys));
	return err;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_cbc_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha1),cbc(aes))",
			.cra_driver_name = "crypto-eip-hmac-sha1-cbc-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha1_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha224_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_SHA224;
	ctx->state_sz = SHA256_DIGEST_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha224_cbc_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA224_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha224),cbc(aes))",
			.cra_driver_name = "crypto-eip-hmac-sha224-cbc-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha224_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha256_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_SHA256;
	ctx->state_sz = SHA256_DIGEST_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha256_cbc_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha256),cbc(aes))",
			.cra_driver_name = "crypto-eip-hmac-sha256-cbc-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha256_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha384_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_SHA384;
	ctx->state_sz = SHA512_DIGEST_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha384_cbc_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA384_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha384),cbc(aes))",
			.cra_driver_name = "crypto-eip-hmac-sha384-cbc-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha384_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha512_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_SHA512;
	ctx->state_sz = SHA512_DIGEST_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha512_cbc_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA512_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha512),cbc(aes))",
			.cra_driver_name = "crypto-eip-hmac-sha512-cbc-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha512_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_md5_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_MD5;
	ctx->state_sz = MD5_DIGEST_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_md5_cbc_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = AES_BLOCK_SIZE,
		.maxauthsize = SHA512_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(md5),cbc(aes))",
			.cra_driver_name = "crypto-eip-hmac-md5-cbc-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_md5_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha1_des3_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha1_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->blocksz = DES3_EDE_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_cbc_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha1),cbc(des3_ede))",
			.cra_driver_name = "crypto-eip-hmac-sha1-cbc-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha1_des3_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha224_des3_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha224_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->blocksz = DES3_EDE_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha224_cbc_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA224_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha224),cbc(des3_ede))",
			.cra_driver_name = "crypto-eip-hmac-sha224-cbc-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha224_des3_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha256_des3_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha256_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->blocksz = DES3_EDE_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha256_cbc_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha256),cbc(des3_ede))",
			.cra_driver_name = "crypto-eip-hmac-sha256-cbc-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha256_des3_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha384_des3_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha384_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->blocksz = DES3_EDE_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha384_cbc_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA384_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha384),cbc(des3_ede))",
			.cra_driver_name = "crypto-eip-hmac-sha384-cbc-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha384_des3_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha512_des3_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha512_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->blocksz = DES3_EDE_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha512_cbc_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = SHA512_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha512),cbc(des3_ede))",
			.cra_driver_name = "crypto-eip-hmac-sha512-cbc-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha512_des3_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_md5_des3_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_md5_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_3DES;
	ctx->blocksz = DES3_EDE_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_md5_cbc_des3_ede = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES3_EDE_BLOCK_SIZE,
		.maxauthsize = MD5_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(md5),cbc(des3_ede))",
			.cra_driver_name = "crypto-eip-hmac-md5-cbc-des3_ede",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES3_EDE_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_md5_des3_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha1_des_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha1_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_DES;
	ctx->blocksz = DES_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_cbc_des = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES_BLOCK_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha1),cbc(des))",
			.cra_driver_name = "crypto-eip-hmac-sha1-cbc-des",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha1_des_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha224_des_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha224_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_DES;
	ctx->blocksz = DES_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha224_cbc_des = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES_BLOCK_SIZE,
		.maxauthsize = SHA224_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha224),cbc(des))",
			.cra_driver_name = "crypto-eip-hmac-sha224-cbc-des",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha224_des_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha256_des_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha256_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_DES;
	ctx->blocksz = DES_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha256_cbc_des = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES_BLOCK_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha256),cbc(des))",
			.cra_driver_name = "crypto-eip-hmac-sha256-cbc-des",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha256_des_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha384_des_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha384_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_DES;
	ctx->blocksz = DES_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha384_cbc_des = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES_BLOCK_SIZE,
		.maxauthsize = SHA384_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha384),cbc(des))",
			.cra_driver_name = "crypto-eip-hmac-sha384-cbc-des",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha384_des_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha512_des_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha512_cra_init(tfm);
	ctx->alg = MTK_CRYPTO_DES;
	ctx->blocksz = DES_BLOCK_SIZE;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha512_cbc_des = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = DES_BLOCK_SIZE,
		.maxauthsize = SHA512_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha512),cbc(des))",
			.cra_driver_name = "crypto-eip-hmac-sha512-cbc-des",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = DES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha512_des_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha1_ctr_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha1_cra_init(tfm);
	ctx->mode = MTK_CRYPTO_MODE_CTR;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha1_ctr_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = CTR_RFC3686_IV_SIZE,
		.maxauthsize = SHA1_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha1),rfc3686(ctr(aes)))",
			.cra_driver_name = "crypto-eip-hmac-sha1-ctr-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha1_ctr_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha256_ctr_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha256_cra_init(tfm);
	ctx->mode = MTK_CRYPTO_MODE_CTR;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha256_ctr_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = CTR_RFC3686_IV_SIZE,
		.maxauthsize = SHA256_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha256),rfc3686(ctr(aes)))",
			.cra_driver_name = "crypto-eip-hmac-sha256-ctr-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha256_ctr_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha224_ctr_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha224_cra_init(tfm);
	ctx->mode = MTK_CRYPTO_MODE_CTR;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha224_ctr_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = CTR_RFC3686_IV_SIZE,
		.maxauthsize = SHA224_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha224),rfc3686(ctr(aes)))",
			.cra_driver_name = "crypto-eip-hmac-sha224-ctr-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha224_ctr_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_sha512_ctr_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_sha512_cra_init(tfm);
	ctx->mode = MTK_CRYPTO_MODE_CTR;
	return 0;
}

struct mtk_crypto_alg_template mtk_crypto_hmac_sha512_ctr_aes = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_setkey,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = CTR_RFC3686_IV_SIZE,
		.maxauthsize = SHA512_DIGEST_SIZE,
		.base = {
			.cra_name = "authenc(hmac(sha512),rfc3686(ctr(aes)))",
			.cra_driver_name = "crypto-eip-hmac-sha512-ctr-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_sha512_ctr_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_aead_gcm_setkey(struct crypto_aead *ctfm, const u8 *key,
				      unsigned int len)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(ctfm);
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_aes_ctx aes;
	u32 hashkey[AES_BLOCK_SIZE >> 2];
	int ret, i;

	ret = aes_expandkey(&aes, key, len);
	if (ret) {
		memzero_explicit(&aes, sizeof(aes));
		return ret;
	}

	for (i = 0; i < len / sizeof(u32); i++)
		ctx->key[i] = cpu_to_le32(aes.key_enc[i]);

	ctx->key_len = len;

	/* Compute hash key by encrypting zeros with cipher key */
	crypto_cipher_clear_flags(ctx->hkaes, CRYPTO_TFM_REQ_MASK);
	crypto_cipher_set_flags(ctx->hkaes, crypto_aead_get_flags(ctfm) &
				CRYPTO_TFM_REQ_MASK);
	ret = crypto_cipher_setkey(ctx->hkaes, key, len);
	if (ret)
		return ret;

	memset(hashkey, 0, AES_BLOCK_SIZE);
	crypto_cipher_encrypt_one(ctx->hkaes, (u8 *)hashkey, (u8 *)hashkey);

	for (i = 0; i < AES_BLOCK_SIZE / sizeof(u32); i++)
		ctx->ipad[i] = cpu_to_be32(hashkey[i]);

	memzero_explicit(hashkey, AES_BLOCK_SIZE);
	memzero_explicit(&aes, sizeof(aes));

	ctx->enc.valid = 0;
	ctx->dec.valid = 0;

	return 0;
}

static int mtk_crypto_aead_gcm_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_GCM;
	ctx->state_sz = GHASH_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_GCM;

	ctx->hkaes = crypto_alloc_cipher("aes", 0, 0);
	return PTR_ERR_OR_ZERO(ctx->hkaes);
}

static void mtk_crypto_aead_gcm_cra_exit(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	crypto_free_cipher(ctx->hkaes);
	mtk_crypto_aead_cra_exit(tfm);
}

static int mtk_crypto_aead_gcm_setauthsize(struct crypto_aead *tfm,
					   unsigned int authsize)
{
	return crypto_gcm_check_authsize(authsize);
}

struct mtk_crypto_alg_template mtk_crypto_gcm = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_aead_gcm_setkey,
		.setauthsize = mtk_crypto_aead_gcm_setauthsize,
		.encrypt = mtk_crypto_aead_encrypt,
		.decrypt = mtk_crypto_aead_decrypt,
		.ivsize = GCM_AES_IV_SIZE,
		.maxauthsize = GHASH_DIGEST_SIZE,
		.base = {
			.cra_name = "gcm(aes)",
			.cra_driver_name = "crypto-eip-gcm-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
				     CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_aead_gcm_cra_init,
			.cra_exit = mtk_crypto_aead_gcm_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

static int mtk_crypto_rfc4106_gcm_setkey(struct crypto_aead *ctfm, const u8 *key,
							unsigned int len)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(ctfm);
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	/* last 4 bytes of key are the nonce! */
	ctx->nonce = *(u32 *)(key + len - CTR_RFC3686_NONCE_SIZE);

	len -= CTR_RFC3686_NONCE_SIZE;
	return mtk_crypto_aead_gcm_setkey(ctfm, key, len);
}

static int mtk_crypto_rfc4106_gcm_setauthsize(struct crypto_aead *tfm,
							unsigned int authsize)
{
	return crypto_rfc4106_check_authsize(authsize);
}

static int mtk_crypto_rfc4106_encrypt(struct aead_request *req)
{
	return crypto_ipsec_check_assoclen(req->assoclen) ?:
			mtk_crypto_aead_encrypt(req);
}

static int mtk_crypto_rfc4106_decrypt(struct aead_request *req)
{
	return crypto_ipsec_check_assoclen(req->assoclen) ?:
			mtk_crypto_aead_decrypt(req);
}

static int mtk_crypto_rfc4106_gcm_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	int ret;

	ret = mtk_crypto_aead_gcm_cra_init(tfm);
	ctx->aead = EIP197_AEAD_TYPE_IPSEC_ESP;
	return ret;
}

struct mtk_crypto_alg_template mtk_crypto_rfc4106_gcm = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_rfc4106_gcm_setkey,
		.setauthsize = mtk_crypto_rfc4106_gcm_setauthsize,
		.encrypt = mtk_crypto_rfc4106_encrypt,
		.decrypt = mtk_crypto_rfc4106_decrypt,
		.ivsize = GCM_RFC4106_IV_SIZE,
		.maxauthsize = GHASH_DIGEST_SIZE,
		.base = {
			.cra_name = "rfc4106(gcm(aes))",
			.cra_driver_name = "crypto-eip-rfc4106-gcm-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
					CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_rfc4106_gcm_cra_init,
			.cra_exit = mtk_crypto_aead_gcm_cra_exit,
		},
	},
};

static int mtk_crypto_rfc4543_gcm_setauthsize(struct crypto_aead *tfm,
							unsigned int authsize)
{
	if (authsize != GHASH_DIGEST_SIZE)
		return -EINVAL;

	return 0;
}

static int mtk_crypto_rfc4543_gcm_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	int ret;

	ret = mtk_crypto_aead_gcm_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_GMAC;
	ctx->mode = MTK_CRYPTO_MODE_GMAC;
	ctx->aead = EIP197_AEAD_TYPE_IPSEC_ESP;
	return ret;
}

struct mtk_crypto_alg_template mtk_crypto_rfc4543_gcm = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_rfc4106_gcm_setkey,
		.setauthsize = mtk_crypto_rfc4543_gcm_setauthsize,
		.encrypt = mtk_crypto_rfc4106_encrypt,
		.decrypt = mtk_crypto_rfc4106_decrypt,
		.ivsize = GCM_RFC4543_IV_SIZE,
		.maxauthsize = GHASH_DIGEST_SIZE,
		.base = {
			.cra_name = "rfc4543(gcm(aes))",
			.cra_driver_name = "crypto-eip-rfc4543-gcm-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
					CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_rfc4543_gcm_cra_init,
			.cra_exit = mtk_crypto_aead_gcm_cra_exit,
		},
	},
};

static int mtk_crypto_aead_ccm_setkey(struct crypto_aead *ctfm, const u8 *key,
						unsigned int len)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(ctfm);
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_aes_ctx aes;
	int ret, i;

	ret = aes_expandkey(&aes, key, len);
	if (ret) {
		memzero_explicit(&aes, sizeof(aes));
		return ret;
	}

	for (i = 0; i < len / sizeof(u32); i++) {
		ctx->key[i] = cpu_to_le32(aes.key_enc[i]);
		ctx->ipad[i + 2 * AES_BLOCK_SIZE / sizeof(u32)] =
			cpu_to_be32(aes.key_enc[i]);
	}

	ctx->key_len = len;
	ctx->state_sz = 2 * AES_BLOCK_SIZE + len;
	ctx->hash_alg = MTK_CRYPTO_ALG_CCM;

	memzero_explicit(&aes, sizeof(aes));

	ctx->enc.valid = 0;
	ctx->dec.valid = 0;

	return 0;
}

static int mtk_crypto_rfc4309_ccm_setkey(struct crypto_aead *ctfm, const u8 *key,
							unsigned int len)
{
	struct crypto_tfm *tfm = crypto_aead_tfm(ctfm);
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	*(u8 *) &ctx->nonce = 3;
	memcpy((u8 *) &ctx->nonce + 1, key + len -
			EIP197_AEAD_IPSEC_CCM_NONCE_SIZE,
			EIP197_AEAD_IPSEC_CCM_NONCE_SIZE);

	len -= EIP197_AEAD_IPSEC_CCM_NONCE_SIZE;
	return mtk_crypto_aead_ccm_setkey(ctfm, key, len);
}

static int mtk_crypto_rfc4309_ccm_setauthsize(struct crypto_aead *tfm,
							unsigned int authsize)
{
	switch (authsize) {
	case 8:
	case 12:
	case 16:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mtk_crypto_rfc4309_ccm_encrypt(struct aead_request *req)
{
	struct mtk_crypto_cipher_req *creq = aead_request_ctx(req);

	if (req->assoclen != 16 && req->assoclen != 20)
		return -EINVAL;

	return mtk_crypto_queue_req(&req->base, creq, MTK_CRYPTO_ENCRYPT);
}

static int mtk_crypto_rfc4309_ccm_decrypt(struct aead_request *req)
{
	struct mtk_crypto_cipher_req *creq = aead_request_ctx(req);

	if (req->assoclen != 16 && req->assoclen != 20)
		return -EINVAL;

	return mtk_crypto_queue_req(&req->base, creq, MTK_CRYPTO_DECRYPT);
}

static int mtk_crypto_aead_ccm_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);

	mtk_crypto_aead_cra_init(tfm);
	ctx->hash_alg = MTK_CRYPTO_ALG_XCBC;
	ctx->state_sz = 3 * AES_BLOCK_SIZE;
	ctx->mode = MTK_CRYPTO_MODE_CCM;

	return 0;
}

static int mtk_crypto_rfc4309_ccm_cra_init(struct crypto_tfm *tfm)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(tfm);
	int ret;

	ret = mtk_crypto_aead_ccm_cra_init(tfm);
	ctx->aead = EIP197_AEAD_TYPE_IPSEC_ESP;
	return ret;
}

struct mtk_crypto_alg_template mtk_crypto_rfc4309_ccm = {
	.type = MTK_CRYPTO_ALG_TYPE_AEAD,
	.alg.aead = {
		.setkey = mtk_crypto_rfc4309_ccm_setkey,
		.setauthsize = mtk_crypto_rfc4309_ccm_setauthsize,
		.encrypt = mtk_crypto_rfc4309_ccm_encrypt,
		.decrypt = mtk_crypto_rfc4309_ccm_decrypt,
		.ivsize = EIP197_AEAD_IPSEC_IV_SIZE,
		.maxauthsize = AES_BLOCK_SIZE,
		.base = {
			.cra_name = "rfc4309(ccm(aes))",
			.cra_driver_name = "crypto-eip-rfc4309-ccm-aes",
			.cra_priority = MTK_CRYPTO_PRIORITY,
			.cra_flags = CRYPTO_ALG_ASYNC |
					CRYPTO_ALG_KERN_DRIVER_ONLY,
			.cra_blocksize = 1,
			.cra_ctxsize = sizeof(struct mtk_crypto_cipher_ctx),
			.cra_alignmask = 0,
			.cra_init = mtk_crypto_rfc4309_ccm_cra_init,
			.cra_exit = mtk_crypto_aead_cra_exit,
			.cra_module = THIS_MODULE,
		},
	},
};

