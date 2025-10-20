// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <crypto/aes.h>
#include <crypto/ctr.h>
#include <crypto/hash.h>
#include <crypto/hmac.h>
#include <crypto/md5.h>
#include <crypto/skcipher.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <crypto/internal/hash.h>

#include <crypto-eip/ddk/slad/api_pcl.h>
#include <crypto-eip/ddk/slad/api_pcl_dtl.h>
#include <crypto-eip/ddk/slad/api_pec.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/internal.h"

void crypto_free_sa(void *sa_pointer, int ring)
{
	DMABuf_Handle_t SAHandle = {0};

	if (ring < 0)
		return;

	SAHandle.p = sa_pointer;
	PEC_SA_UnRegister(ring, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);
	DMABuf_Release(SAHandle);
}

void crypto_free_token(void *token)
{
	DMABuf_Handle_t TokenHandle = {0};

	TokenHandle.p = token;
	DMABuf_Release(TokenHandle);
}

/* TODO: to be remove*/
void crypto_free_pkt(void *pkt)
{
	DMABuf_Handle_t PktHandle = {0};

	PktHandle.p = pkt;
	DMABuf_Release(PktHandle);
}

void crypto_free_sglist(void *sglist)
{
	PEC_Status_t res;
	unsigned int count;
	unsigned int size;
	DMABuf_Handle_t SGListHandle = {0};
	DMABuf_Handle_t ParticleHandle = {0};
	int i;
	uint8_t *Particle_p;

	SGListHandle.p = sglist;
	res = PEC_SGList_GetCapacity(SGListHandle, &count);
	if (res != PEC_STATUS_OK)
		return;
	for (i = 0; i < count; i++) {
		PEC_SGList_Read(SGListHandle,
						i,
						&ParticleHandle,
						&size,
						&Particle_p);
		DMABuf_Particle_Release(ParticleHandle);
	}

	PEC_SGList_Destroy(SGListHandle);
}

static bool crypto_iotoken_create(IOToken_Input_Dscr_t * const dscr_p,
				  void * const ext_p, u32 *data_p,
				  PEC_CommandDescriptor_t * const pec_cmd_dscr)
{
	int IOTokenRc;

	dscr_p->InPacket_ByteCount = pec_cmd_dscr->SrcPkt_ByteCount;
	dscr_p->Ext_p = ext_p;

	IOTokenRc = IOToken_Create(dscr_p, data_p);
	if (IOTokenRc < 0) {
		CRYPTO_ERR("IOToken_Create error %d\n", IOTokenRc);
		return false;
	}

	pec_cmd_dscr->InputToken_p = data_p;

	return true;
}

static unsigned int crypto_pe_busy_get_one(IOToken_Output_Dscr_t *const OutTokenDscr_p,
			       u32 *OutTokenData_p,
			       PEC_ResultDescriptor_t *RD_p, int ring)
{
	int LoopCounter = MTK_EIP197_INLINE_NOF_TRIES;
	int IOToken_Rc;
	PEC_Status_t pecres;

	ZEROINIT(*OutTokenDscr_p);
	ZEROINIT(*RD_p);

	/* Link data structures */
	RD_p->OutputToken_p = OutTokenData_p;

	while (LoopCounter > 0) {
		/* Try to get the processed packet from the driver */
		unsigned int Counter = 0;

		pecres = PEC_Packet_Get(ring, RD_p, 1, &Counter);
		if (pecres != PEC_STATUS_OK) {
			if (pecres == PEC_STATUS_BUSY)
				goto wait;
			/* IO error */
			CRYPTO_ERR("PEC_Packet_Get error %d\n", pecres);
			return 0;
		}

		if (Counter) {
			IOToken_Rc = IOToken_Parse(OutTokenData_p, OutTokenDscr_p);
			if (IOToken_Rc < 0) {
				/* IO error */
				CRYPTO_ERR("IOToken_Parse error %d\n", IOToken_Rc);
				return 0;
			}

			if (OutTokenDscr_p->ErrorCode != 0) {
				/* Packet process error */
				CRYPTO_ERR("Result descriptor error 0x%x\n",
					OutTokenDscr_p->ErrorCode);
				return 0;
			}

			/* packet received */
			return Counter;
		}

wait:
		/* Wait for MTK_EIP197_PKT_GET_TIMEOUT_MS milliseconds */
		udelay(MTK_EIP197_PKT_GET_TIMEOUT_MS * 1000);
		LoopCounter--;
	}

	/* IO error (timeout, not result packet received) */
	return 0;
}

static unsigned int crypto_pe_get_one(IOToken_Output_Dscr_t *const OutTokenDscr_p,
			       u32 *OutTokenData_p,
			       PEC_ResultDescriptor_t *RD_p, int ring)
{
	int IOToken_Rc;
	unsigned int Counter = 0;
	PEC_Status_t pecres;

	ZEROINIT(*OutTokenDscr_p);
	ZEROINIT(*RD_p);

	RD_p->OutputToken_p = OutTokenData_p;

	/* Try to get the processed packet from the driver */
	pecres = PEC_Packet_Get(ring, RD_p, 1, &Counter);
	if (pecres != PEC_STATUS_OK) {
		/* IO error */
		CRYPTO_ERR("PEC_Packet_Get error %d, ring id %d\n", pecres, ring);
		return 0;
	}

	if (Counter) {
		IOToken_Rc = IOToken_Parse(OutTokenData_p, OutTokenDscr_p);
		if (IOToken_Rc < 0) {
			/* IO error */
			CRYPTO_ERR("IOToken_Parse error %d\n", IOToken_Rc);
			return 0;
		}
		if (OutTokenDscr_p->ErrorCode != 0) {
			/* Packet process error */
			CRYPTO_ERR("Result descriptor error 0x%x\n",
				OutTokenDscr_p->ErrorCode);
			return 0;
		}
		/* packet received */
		return Counter;
	}

	/* IO error (timeout, not result packet received) */
	return 0;
}

static SABuilder_Crypto_Mode_t lookaside_match_alg_mode(enum mtk_crypto_cipher_mode mode)
{
	switch (mode) {
	case MTK_CRYPTO_MODE_CBC:
		return SAB_CRYPTO_MODE_CBC;
	case MTK_CRYPTO_MODE_ECB:
		return SAB_CRYPTO_MODE_ECB;
	case MTK_CRYPTO_MODE_OFB:
		return SAB_CRYPTO_MODE_OFB;
	case MTK_CRYPTO_MODE_CFB:
		return SAB_CRYPTO_MODE_CFB;
	case MTK_CRYPTO_MODE_CTR:
		return SAB_CRYPTO_MODE_CTR;
	case MTK_CRYPTO_MODE_GCM:
		return SAB_CRYPTO_MODE_GCM;
	case MTK_CRYPTO_MODE_GMAC:
		return SAB_CRYPTO_MODE_GMAC;
	case MTK_CRYPTO_MODE_CCM:
		return SAB_CRYPTO_MODE_CCM;
	default:
		return SAB_CRYPTO_MODE_BASIC;
	}
}

static SABuilder_Crypto_t lookaside_match_alg_name(enum mtk_crypto_alg alg)
{
	switch (alg) {
	case MTK_CRYPTO_AES:
		return SAB_CRYPTO_AES;
	case MTK_CRYPTO_DES:
		return SAB_CRYPTO_DES;
	case MTK_CRYPTO_3DES:
		return SAB_CRYPTO_3DES;
	default:
		return SAB_CRYPTO_NULL;
	}
}

static SABuilder_Auth_t aead_hash_match(enum mtk_crypto_alg alg)
{
	switch (alg) {
	case MTK_CRYPTO_ALG_SHA1:
		return SAB_AUTH_HMAC_SHA1;
	case MTK_CRYPTO_ALG_SHA224:
		return SAB_AUTH_HMAC_SHA2_224;
	case MTK_CRYPTO_ALG_SHA256:
		return SAB_AUTH_HMAC_SHA2_256;
	case MTK_CRYPTO_ALG_SHA384:
		return SAB_AUTH_HMAC_SHA2_384;
	case MTK_CRYPTO_ALG_SHA512:
		return SAB_AUTH_HMAC_SHA2_512;
	case MTK_CRYPTO_ALG_MD5:
		return SAB_AUTH_HMAC_MD5;
	case MTK_CRYPTO_ALG_GCM:
		return SAB_AUTH_AES_GCM;
	case MTK_CRYPTO_ALG_GMAC:
		return SAB_AUTH_AES_GMAC;
	case MTK_CRYPTO_ALG_CCM:
		return SAB_AUTH_AES_CCM;
	default:
		return SAB_AUTH_NULL;
	}
}

static void mtk_crypto_ring3_handler(void)
{
	struct mtk_crypto_result *rd;
	struct mtk_crypto_context *ctx;
	IOToken_Output_Dscr_t OutTokenDscr;
	PEC_ResultDescriptor_t Res;
	uint32_t OutputToken[IOTOKEN_OUT_WORD_COUNT];
	int ret = 0;

	while (true) {
		spin_lock_bh(&priv->mtk_eip_ring[3].ring_lock);
		if (list_empty(&priv->mtk_eip_ring[3].list)) {
			spin_unlock_bh(&priv->mtk_eip_ring[3].ring_lock);
			return;
		}
		rd = list_first_entry(&priv->mtk_eip_ring[3].list, struct mtk_crypto_result, list);
		spin_unlock_bh(&priv->mtk_eip_ring[3].ring_lock);

		ctx = crypto_tfm_ctx(rd->async->tfm);
		if (crypto_pe_get_one(&OutTokenDscr, OutputToken, &Res, ctx->ring) < 1) {
			PEC_NotifyFunction_t CBFunc;

			CBFunc = mtk_crypto_ring3_handler;
			if (OutTokenDscr.ErrorCode == 0)
				ret = 0;
			else if (OutTokenDscr.ErrorCode & BIT(9))
				ret = -EBADMSG;
			else if (OutTokenDscr.ErrorCode == 0x4003)
				ret = 0;
			else
				ret = 1;

			CRYPTO_ERR("error from crypto_pe_get_one: %d\n", ret);
		}

		ret = ctx->handle_result(rd, ret);

		spin_lock_bh(&priv->mtk_eip_ring[3].ring_lock);
		list_del(&rd->list);
		spin_unlock_bh(&priv->mtk_eip_ring[3].ring_lock);
		kfree(rd);
	}
}

static void mtk_crypto_ring2_handler(void)
{
	struct mtk_crypto_result *rd;
	struct mtk_crypto_context *ctx;
	IOToken_Output_Dscr_t OutTokenDscr;
	PEC_ResultDescriptor_t Res;
	uint32_t OutputToken[IOTOKEN_OUT_WORD_COUNT];
	int ret = 0;

	while (true) {
		spin_lock_bh(&priv->mtk_eip_ring[2].ring_lock);
		if (list_empty(&priv->mtk_eip_ring[2].list)) {
			spin_unlock_bh(&priv->mtk_eip_ring[2].ring_lock);
			return;
		}
		rd = list_first_entry(&priv->mtk_eip_ring[2].list, struct mtk_crypto_result, list);
		spin_unlock_bh(&priv->mtk_eip_ring[2].ring_lock);

		ctx = crypto_tfm_ctx(rd->async->tfm);
		if (crypto_pe_get_one(&OutTokenDscr, OutputToken, &Res, ctx->ring) < 1) {
			PEC_NotifyFunction_t CBFunc;

			CBFunc = mtk_crypto_ring2_handler;
			if (OutTokenDscr.ErrorCode == 0)
				ret = 0;
			else if (OutTokenDscr.ErrorCode & BIT(9))
				ret = -EBADMSG;
			else if (OutTokenDscr.ErrorCode == 0x4003)
				ret = 0;
			else
				ret = 1;

			CRYPTO_ERR("error from crypto_pe_get_one: %d\n", ret);
		}

		ret = ctx->handle_result(rd, ret);

		spin_lock_bh(&priv->mtk_eip_ring[2].ring_lock);
		list_del(&rd->list);
		spin_unlock_bh(&priv->mtk_eip_ring[2].ring_lock);
		kfree(rd);
	}
}

static void mtk_crypto_ring1_handler(void)
{
	struct mtk_crypto_result *rd;
	struct mtk_crypto_context *ctx;
	IOToken_Output_Dscr_t OutTokenDscr;
	PEC_ResultDescriptor_t Res;
	uint32_t OutputToken[IOTOKEN_OUT_WORD_COUNT];
	int ret = 0;

	while (true) {
		spin_lock_bh(&priv->mtk_eip_ring[1].ring_lock);
		if (list_empty(&priv->mtk_eip_ring[1].list)) {
			spin_unlock_bh(&priv->mtk_eip_ring[1].ring_lock);
			return;
		}
		rd = list_first_entry(&priv->mtk_eip_ring[1].list, struct mtk_crypto_result, list);
		spin_unlock_bh(&priv->mtk_eip_ring[1].ring_lock);

		ctx = crypto_tfm_ctx(rd->async->tfm);
		if (crypto_pe_get_one(&OutTokenDscr, OutputToken, &Res, ctx->ring) < 1) {
			PEC_NotifyFunction_t CBFunc;

			CBFunc = mtk_crypto_ring1_handler;
			if (OutTokenDscr.ErrorCode == 0)
				ret = 0;
			else if (OutTokenDscr.ErrorCode & BIT(9))
				ret = -EBADMSG;
			else if (OutTokenDscr.ErrorCode == 0x4003)
				ret = 0;
			else
				ret = 1;

			CRYPTO_ERR("error from crypto_pe_get_one: %d\n", ret);
		}

		ret = ctx->handle_result(rd, ret);

		spin_lock_bh(&priv->mtk_eip_ring[1].ring_lock);
		list_del(&rd->list);
		spin_unlock_bh(&priv->mtk_eip_ring[1].ring_lock);
		kfree(rd);
	}
}

static void mtk_crypto_ring0_handler(void)
{
	struct mtk_crypto_result *rd;
	struct mtk_crypto_context *ctx;
	IOToken_Output_Dscr_t OutTokenDscr;
	PEC_ResultDescriptor_t Res;
	uint32_t OutputToken[IOTOKEN_OUT_WORD_COUNT];
	int ret = 0;

	while (true) {
		spin_lock_bh(&priv->mtk_eip_ring[0].ring_lock);
		if (list_empty(&priv->mtk_eip_ring[0].list)) {
			spin_unlock_bh(&priv->mtk_eip_ring[0].ring_lock);
			return;
		}
		rd = list_first_entry(&priv->mtk_eip_ring[0].list, struct mtk_crypto_result, list);
		spin_unlock_bh(&priv->mtk_eip_ring[0].ring_lock);

		ctx = crypto_tfm_ctx(rd->async->tfm);
		if (crypto_pe_get_one(&OutTokenDscr, OutputToken, &Res, ctx->ring) < 1) {
			PEC_NotifyFunction_t CBFunc;

			CBFunc = mtk_crypto_ring0_handler;
			if (OutTokenDscr.ErrorCode == 0)
				ret = 0;
			else if (OutTokenDscr.ErrorCode & BIT(9))
				ret = -EBADMSG;
			else if (OutTokenDscr.ErrorCode == 0x4003)
				ret = 0;
			else
				ret = 1;

			CRYPTO_ERR("error from crypto_pe_get_one: %d\n", ret);
		}

		ret = ctx->handle_result(rd, ret);

		spin_lock_bh(&priv->mtk_eip_ring[0].ring_lock);
		list_del(&rd->list);
		spin_unlock_bh(&priv->mtk_eip_ring[0].ring_lock);
		kfree(rd);
	}
}

void (*mtk_crypto_interrupt_handler[])(void) = {
	mtk_crypto_ring0_handler,
	mtk_crypto_ring1_handler,
	mtk_crypto_ring2_handler,
	mtk_crypto_ring3_handler
};

void mtk_crypto_req_expired_timer(struct timer_list *t)
{
	struct mtk_crypto_cipher_ctx *ctx = from_timer(ctx, t, poll_timer);
	PEC_NotifyFunction_t CBFunc;
	int ring = ctx->base.ring;
	int rc;

	del_timer(&ctx->poll_timer);
	ctx->poll_timer.expires = 0;

	CBFunc = mtk_crypto_interrupt_handler[ring];
	rc = PEC_ResultNotify_Request(ring, CBFunc, 1);
	if (rc != PEC_STATUS_OK)
		CRYPTO_ERR("PEC_ResultNotify_Request failed with rc = %d\n", rc);
}

int mtk_crypto_ddk_alloc_buff(struct mtk_crypto_cipher_ctx *ctx, int dir, unsigned int digestsize,
								struct mtk_crypto_engine_data *data)
{
	SABuilder_Params_t params;
	SABuilder_Params_Basic_t ProtocolParams;

	DMABuf_Status_t DMAStatus;
	DMABuf_Properties_t DMAProperties = {0, 0, 0, 0};
	DMABuf_HostAddress_t SAHostAddress;
	DMABuf_HostAddress_t TokenHostAddress;

	DMABuf_Handle_t SAHandle = {0};
	DMABuf_Handle_t TokenHandle = {0};
	unsigned int SAWords = 0;

	unsigned int TCRWords = 0;
	unsigned int TokenMaxWords = 0;

	IOToken_Input_Dscr_t InTokenDscr;
	IOToken_Output_Dscr_t OutTokenDscr;
	void *InTokenDscrExt_p = NULL;
	int rc;

#ifdef CRYPTO_IOTOKEN_EXT
	IOToken_Input_Dscr_Ext_t InTokenDscrExt;

	ZEROINIT(InTokenDscrExt);
	InTokenDscrExt_p = &InTokenDscrExt;
#endif
	ZEROINIT(InTokenDscr);
	ZEROINIT(OutTokenDscr);

	if (dir == MTK_CRYPTO_ENCRYPT)
		rc = SABuilder_Init_Basic(&params, &ProtocolParams, SAB_DIRECTION_OUTBOUND);
	else
		rc = SABuilder_Init_Basic(&params, &ProtocolParams, SAB_DIRECTION_INBOUND);

	if (data->sa_handle) {
		crypto_free_sa(data->sa_handle, ctx->base.ring);
		crypto_free_token(data->token_handle);
		kfree(data->token_context);
	}

	if (rc) {
		CRYPTO_ERR("SABuilder_Init_Basic failed: %d\n", rc);
		return rc;
	}

	/* Build SA */
	params.CryptoAlgo = lookaside_match_alg_name(ctx->alg);
	params.CryptoMode = lookaside_match_alg_mode(ctx->mode);
	params.KeyByteCount = ctx->key_len;
	params.Key_p = (uint8_t *) ctx->key;
	if (params.CryptoMode == SAB_CRYPTO_MODE_GCM && ctx->aead == EIP197_AEAD_TYPE_IPSEC_ESP) {
		params.Nonce_p = (uint8_t *) &ctx->nonce;
		params.IVSrc = SAB_IV_SRC_TOKEN;
		params.flags |= SAB_FLAG_COPY_IV;
	} else if (params.CryptoMode == SAB_CRYPTO_MODE_GMAC) {
		params.Nonce_p = (uint8_t *) &ctx->nonce;
		params.IVSrc = SAB_IV_SRC_TOKEN;
	} else if (params.CryptoMode == SAB_CRYPTO_MODE_GCM) {
		params.IVSrc = SAB_IV_SRC_TOKEN;
	} else if (params.CryptoMode == SAB_CRYPTO_MODE_CCM) { /* Todo, use token for ccm */
		params.IVSrc = SAB_IV_SRC_TOKEN;
		params.Nonce_p = (uint8_t *) &ctx->nonce + 1;
	} else {
		params.IVSrc = SAB_IV_SRC_TOKEN;
	}

	if (params.CryptoMode == SAB_CRYPTO_MODE_CTR)
		params.Nonce_p = (uint8_t *) &ctx->nonce;

	params.AuthAlgo = aead_hash_match(ctx->hash_alg);
	params.AuthKey1_p = (uint8_t *) ctx->ipad;
	params.AuthKey2_p = (uint8_t *) ctx->opad;

	ProtocolParams.ICVByteCount = digestsize;

	rc = SABuilder_GetSizes(&params, &SAWords, NULL, NULL);
	if (rc) {
		CRYPTO_ERR("SA not created because of size errors: %d\n", rc);
		return rc;
	}

	data->sa_size = SAWords;

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_TRANSFORM;
	DMAProperties.Size = MAX(4 * SAWords, 256);

	DMAStatus = DMABuf_Alloc(DMAProperties, &SAHostAddress, &SAHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of SA failed: %d\n", DMAStatus);
		return rc;
	}

	rc = SABuilder_BuildSA(&params, (u32 *) SAHostAddress.p, NULL, NULL);
	if (rc) {
		CRYPTO_ERR("SA not created because of errors: %d\n", rc);
		goto release_sa;
	}

	rc = PEC_SA_Register(ctx->base.ring, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);
	if (rc != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC_SA_Register failed: %d\n", rc);
		goto release_sa;
	}

	data->sa_addr = SAHostAddress.p;
	data->sa_handle = SAHandle.p;

	/* Build Token */
	rc = TokenBuilder_GetContextSize(&params, &TCRWords);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_GetContextSize returned errors: f%d\n", rc);
		goto release_sa;
	}

	data->token_context = kmalloc(4 * TCRWords, GFP_KERNEL);
	if (!data->token_context) {
		rc = 1;
		CRYPTO_ERR("Allocation of TCR failed\n");
		goto release_sa;
	}

	rc = TokenBuilder_BuildContext(&params, data->token_context);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_BuildContext failed: %d\n", rc);
		goto release_context;
	}

	rc = TokenBuilder_GetSize(data->token_context, &TokenMaxWords);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_GetSize failed: %d\n", rc);
		goto release_context;
	}
	data->token_size = TokenMaxWords;

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_TOKEN;
	DMAProperties.Size = 4 * TokenMaxWords;

	DMAStatus = DMABuf_Alloc(DMAProperties, &TokenHostAddress, &TokenHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of token builder failed: %d\n", DMAStatus);
		goto release_context;
	}
	data->token_addr = TokenHostAddress.p;
	data->token_handle = TokenHandle.p;
	data->valid = 1;

	return rc;

release_context:
	kfree(data->token_context);
release_sa:
	DMABuf_Release(SAHandle);

	return rc;
}

int mtk_crypto_basic_cipher(struct crypto_async_request *async,
		struct mtk_crypto_cipher_req *mtk_req, struct scatterlist *src,
		struct scatterlist *dst, unsigned int cryptlen,
		unsigned int assoclen, unsigned int digestsize, u8 *iv,
		unsigned int ivsize)
{
	struct mtk_crypto_cipher_ctx *ctx = crypto_tfm_ctx(async->tfm);
	struct mtk_crypto_engine_data *data;
	struct mtk_crypto_result *result;
	struct scatterlist *sg;
	unsigned int totlen_src;
	unsigned int totlen_dst;
	unsigned int src_pkt =  cryptlen + assoclen;
	unsigned int pass_assoc = 0;
	int pass_id;
	int rc = 0;
	int i;
	int ring = ctx->base.ring;

	DMABuf_Properties_t DMAProperties = {0, 0, 0, 0};

	DMABuf_Handle_t SAHandle = {0};
	DMABuf_Handle_t TokenHandle = {0};
	DMABuf_Handle_t SrcSGListHandle = {0};
	DMABuf_Handle_t DstSGListHandle = {0};

	unsigned int TokenWords = 0;
	unsigned int TokenHeaderWord;

	TokenBuilder_Params_t TokenParams;
	PEC_CommandDescriptor_t Cmd;
	PEC_NotifyFunction_t CBFunc;
	unsigned int count;

	IOToken_Input_Dscr_t InTokenDscr;
	IOToken_Output_Dscr_t OutTokenDscr;
	uint32_t InputToken[IOTOKEN_IN_WORD_COUNT];
	void *InTokenDscrExt_p = NULL;
	uint8_t token_iv[16] = {0};
	uint8_t *aad = NULL;

#ifdef CRYPTO_IOTOKEN_EXT
	IOToken_Input_Dscr_Ext_t InTokenDscrExt;

	ZEROINIT(InTokenDscrExt);
	InTokenDscrExt_p = &InTokenDscrExt;
#endif
	ZEROINIT(InTokenDscr);
	ZEROINIT(OutTokenDscr);

	/* Init SA */
	if (mtk_req->direction == MTK_CRYPTO_ENCRYPT) {
		totlen_src = cryptlen + assoclen;
		totlen_dst = totlen_src + digestsize;
		data = &ctx->enc;
	} else {
		totlen_src = cryptlen + assoclen;
		totlen_dst = totlen_src - digestsize;
		data = &ctx->dec;
	}

	SAHandle.p = data->sa_handle;
	TokenHandle.p = data->token_handle;

	if ((ctx->mode == MTK_CRYPTO_MODE_GCM && ctx->aead == EIP197_AEAD_TYPE_IPSEC_ESP)
			|| ctx->mode == MTK_CRYPTO_MODE_GMAC) {
		memcpy(token_iv, &ctx->nonce, 4);
		memcpy(token_iv + 4, iv, ivsize);
		token_iv[15] = 1;
	} else if (ctx->mode == MTK_CRYPTO_MODE_GCM) {
		memcpy(token_iv, iv, ivsize);
		token_iv[15] = 1;
	} else if (ctx->mode == MTK_CRYPTO_MODE_CCM) {
		memcpy(token_iv, (uint8_t *) &ctx->nonce, 4);
		memcpy(token_iv + 4, iv, ivsize);
		token_iv[15] = 0;
	} else if (ctx->mode == MTK_CRYPTO_MODE_CTR) {
		memcpy(token_iv, &ctx->nonce, 4);
		memcpy(token_iv + 4, iv, ivsize);
		token_iv[15] = 1;
	}

	/* Check dst buffer has enough size */
	mtk_req->nr_src = sg_nents_for_len(src, totlen_src);
	mtk_req->nr_dst = sg_nents_for_len(dst, totlen_dst);

	if (src == dst) {
		mtk_req->nr_src = max(mtk_req->nr_src, mtk_req->nr_dst);
		mtk_req->nr_dst = mtk_req->nr_src;
		if (unlikely((totlen_src || totlen_dst) && (mtk_req->nr_src <= 0))) {
			CRYPTO_ERR("In-place buffer not large enough\n");
			goto error_remove_sg;
		}
		dma_map_sg(crypto_dev, src, mtk_req->nr_src, DMA_BIDIRECTIONAL);
	} else {
		if (unlikely(totlen_src && (mtk_req->nr_src <= 0))) {
			CRYPTO_ERR("Source buffer not large enough\n");
			goto error_remove_sg;
		}
		dma_map_sg(crypto_dev, src, mtk_req->nr_src, DMA_TO_DEVICE);

		if (unlikely(totlen_dst && (mtk_req->nr_dst <= 0))) {
			CRYPTO_ERR("Dest buffer not large enough\n");
			dma_unmap_sg(crypto_dev, src, mtk_req->nr_src, DMA_TO_DEVICE);
			goto error_remove_sg;
		}
		dma_map_sg(crypto_dev, dst, mtk_req->nr_dst, DMA_FROM_DEVICE);
	}

	if (ctx->mode == MTK_CRYPTO_MODE_CCM ||
		(ctx->mode == MTK_CRYPTO_MODE_GCM && ctx->aead == EIP197_AEAD_TYPE_IPSEC_ESP)) {

		aad = kmalloc(assoclen, GFP_KERNEL);
		if (!aad)
			goto error_remove_sg;
		sg_copy_to_buffer(src, mtk_req->nr_src, aad, assoclen);
		src_pkt -= assoclen;
		pass_assoc = assoclen;
	}

	/* Assign sg list */
	rc = PEC_SGList_Create(MAX(mtk_req->nr_src, 1), &SrcSGListHandle);
	if (rc != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC_SGList_Create src failed with rc = %d\n", rc);
		goto error_remove_sg;
	}

	pass_id = 0;
	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_PACKET;
	for_each_sg(src, sg, mtk_req->nr_src, i) {
		int len = sg_dma_len(sg);
		DMABuf_Handle_t sg_handle;
		DMABuf_HostAddress_t host;

		if (totlen_src < len)
			len = totlen_src;

		if (pass_assoc) {
			if (pass_assoc >= len) {
				pass_assoc -= len;
				pass_id++;
				continue;
			}
			DMAProperties.Size = MAX(len - pass_assoc, 1);
			rc = DMABuf_Particle_Alloc(DMAProperties, sg_dma_address(sg) + pass_assoc,
							&host, &sg_handle);
			if (rc != DMABUF_STATUS_OK) {
				CRYPTO_ERR("DMABuf_Particle_Alloc failed rc = %d\n", rc);
				goto error_remove_sg;
			}
			rc = PEC_SGList_Write(SrcSGListHandle, i - pass_id, sg_handle,
						len - pass_assoc);
			if (rc != PEC_STATUS_OK)
				CRYPTO_ERR("PEC_SGList_Write failed rc = %d\n", rc);
			pass_assoc = 0;
		} else {
			DMAProperties.Size = MAX(len, 1);
			rc = DMABuf_Particle_Alloc(DMAProperties, sg_dma_address(sg),
							&host, &sg_handle);
			if (rc != DMABUF_STATUS_OK) {
				CRYPTO_ERR("DMABuf_Particle_Alloc failed rc = %d\n", rc);
				goto error_remove_sg;
			}

			rc = PEC_SGList_Write(SrcSGListHandle, i - pass_id, sg_handle, len);
			if (rc != PEC_STATUS_OK)
				CRYPTO_ERR("PEC_SGList_Write failed rc = %d\n", rc);
		}

		totlen_src -= len;
		if (!totlen_src)
			break;
	}

	/* Alloc sg list for result */
	rc = PEC_SGList_Create(MAX(mtk_req->nr_dst, 1), &DstSGListHandle);
	if (rc != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC_SGList_Create dst failed with rc = %d\n", rc);
		goto error_remove_sg;
	}

	for_each_sg(dst, sg, mtk_req->nr_dst, i) {
		int len = sg_dma_len(sg);
		DMABuf_Handle_t sg_handle;
		DMABuf_HostAddress_t host;

		if (len > totlen_dst)
			len = totlen_dst;

		DMAProperties.Size = MAX(len, 1);
		rc = DMABuf_Particle_Alloc(DMAProperties, sg_dma_address(sg), &host, &sg_handle);
		if (rc != DMABUF_STATUS_OK) {
			CRYPTO_ERR("DMABuf_Particle_Alloc failed rc = %d\n", rc);
			goto error_remove_sg;
		}
		rc = PEC_SGList_Write(DstSGListHandle, i, sg_handle, len);
		if (rc != PEC_STATUS_OK)
			CRYPTO_ERR("PEC_SGList_Write failed rc = %d\n", rc);

		if (unlikely(!len))
			break;
		totlen_dst -= len;
	}

	/* Build Token */
	ZEROINIT(TokenParams);

	if (ctx->mode == MTK_CRYPTO_MODE_GCM || ctx->mode == MTK_CRYPTO_MODE_GMAC ||
		ctx->mode == MTK_CRYPTO_MODE_CCM || ctx->mode == MTK_CRYPTO_MODE_CTR)
		TokenParams.IV_p = token_iv;
	else
		TokenParams.IV_p = iv;

	if ((ctx->mode == MTK_CRYPTO_MODE_GCM && ctx->aead == EIP197_AEAD_TYPE_IPSEC_ESP) ||
	     ctx->mode == MTK_CRYPTO_MODE_CCM) {
		TokenParams.AdditionalValue = assoclen - ivsize;
		TokenParams.AAD_p = aad;
	} else if (ctx->mode != MTK_CRYPTO_MODE_GMAC)
		TokenParams.AdditionalValue = assoclen;

	rc = TokenBuilder_BuildToken(data->token_context, aad, src_pkt,
					&TokenParams, (uint32_t *) data->token_addr,
					&TokenWords, &TokenHeaderWord);
	if (rc != TKB_STATUS_OK) {
		CRYPTO_ERR("Token builder failed: %d\n", rc);
		goto error_remove_sg;
	}

	if (ctx->mode == MTK_CRYPTO_MODE_CBC &&
			mtk_req->direction == MTK_CRYPTO_DECRYPT)
		sg_pcopy_to_buffer(src, mtk_req->nr_src, iv, ivsize, cryptlen - ivsize);

	ZEROINIT(Cmd);
	Cmd.Token_Handle = TokenHandle;
	Cmd.Token_WordCount = TokenWords;
	Cmd.SrcPkt_Handle = SrcSGListHandle;
	Cmd.SrcPkt_ByteCount = src_pkt;
	Cmd.DstPkt_Handle = DstSGListHandle;
	Cmd.SA_Handle1 = SAHandle;
	Cmd.SA_Handle2 = DMABuf_NULLHandle;

#if defined(CRYPTO_IOTOKEN_EXT)
	InTokenDscrExt.HW_Services = IOTOKEN_CMD_PKT_LAC;
#endif
	InTokenDscr.TknHdrWordInit = TokenHeaderWord;

	if (!crypto_iotoken_create(&InTokenDscr,
				   InTokenDscrExt_p,
				   InputToken,
				   &Cmd)) {
		rc = 1;
		goto error_remove_sg;
	}

	do {
		rc = PEC_Packet_Put(ring, &Cmd, 1, &count);
	} while (rc == PEC_STATUS_BUSY);

	if (rc != PEC_STATUS_OK && count != 1) {
		goto error_remove_sg;
	}

	result = kmalloc(sizeof(struct mtk_crypto_result), GFP_KERNEL);
	if (!result) {
		rc = 1;
		CRYPTO_ERR("No memory for result\n");
		goto error_remove_sg;
	}
	INIT_LIST_HEAD(&result->list);
	result->eip.pkt_handle = SrcSGListHandle.p;
	result->async = async;
	result->dst = DstSGListHandle.p;
	CBFunc = mtk_crypto_interrupt_handler[ring];

	spin_lock_bh(&priv->mtk_eip_ring[ring].ring_lock);
	list_add_tail(&result->list, &priv->mtk_eip_ring[ring].list);
	spin_unlock_bh(&priv->mtk_eip_ring[ring].ring_lock);

	if (ctx->poll_timer.expires == 0) {
		ctx->poll_timer.expires = jiffies + (2 * HZ / 1000);
		add_timer(&ctx->poll_timer);
	}

	if ((atomic_inc_return(&ctx->base.req_count) % REQ_PKT_NUM) == 0) {
		del_timer(&ctx->poll_timer);
		ctx->poll_timer.expires = 0;
		rc = PEC_ResultNotify_Request(ring, CBFunc, 1);
		if (rc != PEC_STATUS_OK) {
			CRYPTO_ERR("PEC_ResultNotify_Request failed with rc = %d\n", rc);
			goto error_remove_sg;
		}
	}

	kfree(aad);
	return rc;

error_remove_sg:
	if (src == dst) {
		dma_unmap_sg(crypto_dev, src, mtk_req->nr_src, DMA_BIDIRECTIONAL);
	} else {
		dma_unmap_sg(crypto_dev, src, mtk_req->nr_src, DMA_TO_DEVICE);
		dma_unmap_sg(crypto_dev, dst, mtk_req->nr_dst, DMA_FROM_DEVICE);
	}

	kfree(aad);

	crypto_free_sglist(SrcSGListHandle.p);
	crypto_free_sglist(DstSGListHandle.p);

	return rc;
}

static SABuilder_Auth_t lookaside_match_hash(enum mtk_crypto_alg alg)
{
	switch (alg) {
	case MTK_CRYPTO_ALG_SHA1:
		return SAB_AUTH_HASH_SHA1;
	case MTK_CRYPTO_ALG_SHA224:
		return SAB_AUTH_HASH_SHA2_224;
	case MTK_CRYPTO_ALG_SHA256:
		return SAB_AUTH_HASH_SHA2_256;
	case MTK_CRYPTO_ALG_SHA384:
		return SAB_AUTH_HASH_SHA2_384;
	case MTK_CRYPTO_ALG_SHA512:
		return SAB_AUTH_HASH_SHA2_512;
	case MTK_CRYPTO_ALG_MD5:
		return SAB_AUTH_HASH_MD5;
	case MTK_CRYPTO_ALG_XCBC:
		return SAB_AUTH_AES_XCBC_MAC;
	case MTK_CRYPTO_ALG_CMAC_128:
		return SAB_AUTH_AES_CMAC_128;
	case MTK_CRYPTO_ALG_CMAC_192:
		return SAB_AUTH_AES_CMAC_192;
	case MTK_CRYPTO_ALG_CMAC_256:
		return SAB_AUTH_AES_CMAC_256;
	default:
		return SAB_AUTH_NULL;
	}
}

int crypto_ahash_token_req(struct crypto_async_request *async, struct mtk_crypto_ahash_req *mtk_req,
				uint8_t *Input_p, unsigned int InputByteCount, bool finish)
{
	struct mtk_crypto_result *result;
	struct ahash_request *areq = ahash_request_cast(async);
	struct mtk_crypto_ahash_ctx *ctx = crypto_ahash_ctx(crypto_ahash_reqtfm(areq));

	DMABuf_Properties_t DMAProperties = {0, 0, 0, 0};
	DMABuf_HostAddress_t TokenHostAddress;
	DMABuf_HostAddress_t PktHostAddress;
	DMABuf_Status_t DMAStatus;

	DMABuf_Handle_t TokenHandle = {0};
	DMABuf_Handle_t PktHandle = {0};
	DMABuf_Handle_t SAHandle = {0};

	unsigned int TokenMaxWords = 0;
	unsigned int TokenHeaderWord;
	unsigned int TokenWords = 0;
	void *TCRData = 0;
	int ring = ctx->base.ring;

	TokenBuilder_Params_t TokenParams;
	PEC_CommandDescriptor_t Cmd;
	PEC_NotifyFunction_t CBFunc;

	unsigned int count;
	int rc;

	u32 InputToken[IOTOKEN_IN_WORD_COUNT];
	IOToken_Output_Dscr_t OutTokenDscr;
	IOToken_Input_Dscr_t InTokenDscr;
	void *InTokenDscrExt_p = NULL;

#ifdef CRYPTO_IOTOKEN_EXT
	IOToken_Input_Dscr_Ext_t InTokenDscrExt;

	ZEROINIT(InTokenDscrExt);
	InTokenDscrExt_p = &InTokenDscrExt;
#endif
	ZEROINIT(InTokenDscr);
	ZEROINIT(OutTokenDscr);

	TCRData = mtk_req->token_context;
	rc = TokenBuilder_GetSize(TCRData, &TokenMaxWords);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_GetSize failed: %d\n", rc);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_TOKEN;
	DMAProperties.Size = 4*TokenMaxWords;

	DMAStatus = DMABuf_Alloc(DMAProperties, &TokenHostAddress, &TokenHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of token builder failed: %d\n", DMAStatus);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_PACKET;
	DMAProperties.Size = MAX(InputByteCount, mtk_req->digest_sz);

	DMAStatus = DMABuf_Alloc(DMAProperties, &PktHostAddress, &PktHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of source packet buffer failed: %d\n",
			   DMAStatus);
		goto error_exit;
	}
	memcpy(PktHostAddress.p, Input_p, InputByteCount);

	ZEROINIT(TokenParams);
	TokenParams.PacketFlags |= TKB_PACKET_FLAG_HASHAPPEND;
	if (finish)
		TokenParams.PacketFlags |= TKB_PACKET_FLAG_HASHFINAL;

	rc = TokenBuilder_BuildToken(TCRData, (u8 *) PktHostAddress.p,
				     InputByteCount, &TokenParams,
				     (u32 *) TokenHostAddress.p,
				     &TokenWords, &TokenHeaderWord);
	if (rc != TKB_STATUS_OK) {
		CRYPTO_ERR("Token builder failed: %d\n", rc);
		goto error_exit_unregister;
	}

	SAHandle.p = mtk_req->sa_pointer;
	ZEROINIT(Cmd);
	Cmd.Token_Handle = TokenHandle;
	Cmd.Token_WordCount = TokenWords;
	Cmd.SrcPkt_Handle = PktHandle;
	Cmd.SrcPkt_ByteCount = InputByteCount;
	Cmd.DstPkt_Handle = PktHandle;
	Cmd.SA_Handle1 = SAHandle;
	Cmd.SA_Handle2 = DMABuf_NULLHandle;

#if defined(CRYPTO_IOTOKEN_EXT)
	InTokenDscrExt.HW_Services  = IOTOKEN_CMD_PKT_LAC;
#endif
	InTokenDscr.TknHdrWordInit = TokenHeaderWord;

	if (!crypto_iotoken_create(&InTokenDscr,
				   InTokenDscrExt_p,
				   InputToken,
				   &Cmd)) {
		rc = 1;
		goto error_exit_unregister;
	}

	do {
		rc = PEC_Packet_Put(ring, &Cmd, 1, &count);
	} while (rc == PEC_STATUS_BUSY);

	if (rc != PEC_STATUS_OK && count != 1) {
		rc = 1;
		CRYPTO_ERR("PEC_Packet_Put error: %d\n", rc);
		goto error_exit_unregister;
	}

	result = kmalloc(sizeof(struct mtk_crypto_result), GFP_KERNEL);
	if (!result) {
		rc = 1;
		CRYPTO_ERR("No memory for result\n");
		goto error_exit_unregister;
	}
	INIT_LIST_HEAD(&result->list);
	result->eip.token_handle = TokenHandle.p;
	result->eip.pkt_handle = PktHandle.p;
	result->async = async;
	result->dst = PktHostAddress.p;

	spin_lock_bh(&priv->mtk_eip_ring[ring].ring_lock);
	list_add_tail(&result->list, &priv->mtk_eip_ring[ring].list);
	spin_unlock_bh(&priv->mtk_eip_ring[ring].ring_lock);
	CBFunc = mtk_crypto_interrupt_handler[ring];
	rc = PEC_ResultNotify_Request(ring, CBFunc, 1);

	return rc;

error_exit_unregister:
	PEC_SA_UnRegister(ring, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);

error_exit:
	DMABuf_Release(SAHandle);
	DMABuf_Release(TokenHandle);
	DMABuf_Release(PktHandle);

	if (TCRData != NULL)
		kfree(TCRData);

	return rc;
}

int crypto_ahash_aes_cbc(struct crypto_async_request *async, struct mtk_crypto_ahash_req *mtk_req,
				uint8_t *Input_p, unsigned int InputByteCount)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(async->tfm);
	struct mtk_crypto_result *result;
	SABuilder_Params_Basic_t ProtocolParams;
	SABuilder_Params_t params;
	unsigned int SAWords = 0;
	int rc;
	int ring = ctx->base.ring;

	DMABuf_Properties_t DMAProperties = {0, 0, 0, 0};
	DMABuf_HostAddress_t TokenHostAddress;
	DMABuf_HostAddress_t PktHostAddress;
	DMABuf_HostAddress_t SAHostAddress;
	DMABuf_Status_t DMAStatus;

	DMABuf_Handle_t TokenHandle = {0};
	DMABuf_Handle_t PktHandle = {0};
	DMABuf_Handle_t SAHandle = {0};

	unsigned int TokenMaxWords = 0;
	unsigned int TokenHeaderWord;
	unsigned int TokenWords = 0;
	unsigned int TCRWords = 0;
	void *TCRData = 0;

	TokenBuilder_Params_t TokenParams;
	PEC_CommandDescriptor_t Cmd;
	PEC_NotifyFunction_t CBFunc;

	unsigned int count;
	int i;

	u32 InputToken[IOTOKEN_IN_WORD_COUNT];
	IOToken_Output_Dscr_t OutTokenDscr;
	IOToken_Input_Dscr_t InTokenDscr;
	void *InTokenDscrExt_p = NULL;

#ifdef CRYPTO_IOTOKEN_EXT
	IOToken_Input_Dscr_Ext_t InTokenDscrExt;

	ZEROINIT(InTokenDscrExt);
	InTokenDscrExt_p = &InTokenDscrExt;
#endif
	ZEROINIT(InTokenDscr);
	ZEROINIT(OutTokenDscr);

	if (!IS_ALIGNED(InputByteCount, 16)) {
		CRYPTO_ERR("not aligned: %d\n", InputByteCount);
		return -EINVAL;
	}
	rc = SABuilder_Init_Basic(&params, &ProtocolParams, SAB_DIRECTION_OUTBOUND);
	if (rc) {
		CRYPTO_ERR("SABuilder_Init_Basic failed: %d\n", rc);
		goto error_exit;
	}

	params.CryptoAlgo = SAB_CRYPTO_AES;
	params.CryptoMode = SAB_CRYPTO_MODE_CBC;
	params.KeyByteCount = ctx->key_sz - 2 * AES_BLOCK_SIZE;
	params.Key_p = (uint8_t *) ctx->ipad + 2 * AES_BLOCK_SIZE;
	params.IVSrc = SAB_IV_SRC_SA;
	params.IV_p = (uint8_t *) mtk_req->state;

	if (ctx->alg == MTK_CRYPTO_ALG_XCBC) {
		for (i = 0; i < params.KeyByteCount; i = i + 4) {
			swap(params.Key_p[i], params.Key_p[i+3]);
			swap(params.Key_p[i+1], params.Key_p[i+2]);
		}
	}

	rc = SABuilder_GetSizes(&params, &SAWords, NULL, NULL);
	if (rc) {
		CRYPTO_ERR("SA not created because of size errors: %d\n", rc);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_TRANSFORM;
	DMAProperties.Size = MAX(4*SAWords, 256);

	DMAStatus = DMABuf_Alloc(DMAProperties, &SAHostAddress, &SAHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of SA failed: %d\n", DMAStatus);
		goto error_exit;
	}

	rc = SABuilder_BuildSA(&params, (u32 *)SAHostAddress.p, NULL, NULL);
	if (rc) {
		CRYPTO_ERR("SA not created because of errors: %d\n", rc);
		goto error_exit;
	}

	rc = TokenBuilder_GetContextSize(&params, &TCRWords);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_GetContextSize returned errors: %d\n", rc);
		goto error_exit;
	}

	TCRData = kmalloc(4 * TCRWords, GFP_KERNEL);
	if (!TCRData) {
		rc = 1;
		CRYPTO_ERR("Allocation of TCR failed\n");
		goto error_exit;
	}

	rc = TokenBuilder_BuildContext(&params, TCRData);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_BuildContext failed: %d\n", rc);
		goto error_exit;
	}

	rc = TokenBuilder_GetSize(TCRData, &TokenMaxWords);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_GetSize failed: %d\n", rc);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_TOKEN;
	DMAProperties.Size = 4*TokenMaxWords;

	DMAStatus = DMABuf_Alloc(DMAProperties, &TokenHostAddress, &TokenHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of token builder failed: %d\n", DMAStatus);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_PACKET;
	DMAProperties.Size = MAX(InputByteCount, 1);

	DMAStatus = DMABuf_Alloc(DMAProperties, &PktHostAddress, &PktHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of source packet buffer failed: %d\n", DMAStatus);
		goto error_exit;
	}

	rc = PEC_SA_Register(ring, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);
	if (rc != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC_SA_Register failed: %d\n", rc);
		goto error_exit;
	}

	memcpy(PktHostAddress.p, Input_p, InputByteCount);

	ZEROINIT(TokenParams);
	rc = TokenBuilder_BuildToken(TCRData, (uint8_t *) PktHostAddress.p, InputByteCount,
					&TokenParams, (uint32_t *) TokenHostAddress.p,
					&TokenWords, &TokenHeaderWord);
	if (rc != TKB_STATUS_OK) {
		CRYPTO_ERR("Token builder failed: %d\n", rc);
		goto error_exit_unregister;
	}

	ZEROINIT(Cmd);
	Cmd.Token_Handle = TokenHandle;
	Cmd.Token_WordCount = TokenWords;
	Cmd.SrcPkt_Handle = PktHandle;
	Cmd.SrcPkt_ByteCount = InputByteCount;
	Cmd.DstPkt_Handle = PktHandle;
	Cmd.SA_Handle1 = SAHandle;
	Cmd.SA_Handle2 = DMABuf_NULLHandle;

#if defined(CRYPTO_IOTOKEN_EXT)
	InTokenDscrExt.HW_Services = IOTOKEN_CMD_PKT_LAC;
#endif
	InTokenDscr.TknHdrWordInit = TokenHeaderWord;

	if (!crypto_iotoken_create(&InTokenDscr,
				   InTokenDscrExt_p,
				   InputToken,
				   &Cmd)) {
		rc = 1;
		goto error_exit_unregister;
	}

	do {
		rc = PEC_Packet_Put(ring, &Cmd, 1, &count);
	} while (rc == PEC_STATUS_BUSY);

	if (rc != PEC_STATUS_OK && count != 1) {
		rc = 1;
		CRYPTO_ERR("PEC_Packet_Put error: %d\n", rc);
		goto error_exit_unregister;
	}

	result = kmalloc(sizeof(struct mtk_crypto_result), GFP_KERNEL);
	if (!result) {
		rc = 1;
		CRYPTO_ERR("No memory for result\n");
		goto error_exit_unregister;
	}
	INIT_LIST_HEAD(&result->list);
	result->eip.sa_handle = SAHandle.p;
	result->eip.token_handle = TokenHandle.p;
	result->eip.token_context = TCRData;
	result->eip.pkt_handle = PktHandle.p;
	result->async = async;
	result->dst = PktHostAddress.p;
	result->size = InputByteCount;

	spin_lock_bh(&priv->mtk_eip_ring[ring].ring_lock);
	list_add_tail(&result->list, &priv->mtk_eip_ring[ring].list);
	spin_unlock_bh(&priv->mtk_eip_ring[ring].ring_lock);

	CBFunc = mtk_crypto_interrupt_handler[ring];
	rc = PEC_ResultNotify_Request(ring, CBFunc, 1);
	if (rc != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC_ResultNotify_Request failed with rc = %d\n", rc);
		goto error_exit_unregister;
	}
	return 0;

error_exit_unregister:
	PEC_SA_UnRegister(ring, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);

error_exit:
	DMABuf_Release(SAHandle);
	DMABuf_Release(TokenHandle);
	DMABuf_Release(PktHandle);

	if (TCRData != NULL)
		kfree(TCRData);

	return rc;
}

int crypto_first_ahash_req(struct crypto_async_request *async,
			   struct mtk_crypto_ahash_req *mtk_req, uint8_t *Input_p,
			   unsigned int InputByteCount, bool finish)
{
	struct mtk_crypto_ahash_ctx *ctx = crypto_tfm_ctx(async->tfm);
	struct mtk_crypto_result *result;
	SABuilder_Params_Basic_t ProtocolParams;
	SABuilder_Params_t params;
	unsigned int SAWords = 0;
	static uint8_t DummyAuthKey[64];
	int rc;
	int ring = ctx->base.ring;

	DMABuf_Properties_t DMAProperties = {0, 0, 0, 0};
	DMABuf_HostAddress_t TokenHostAddress;
	DMABuf_HostAddress_t PktHostAddress;
	DMABuf_HostAddress_t SAHostAddress;
	DMABuf_Status_t DMAStatus;

	DMABuf_Handle_t TokenHandle = {0};
	DMABuf_Handle_t PktHandle = {0};
	DMABuf_Handle_t SAHandle = {0};

	unsigned int TokenMaxWords = 0;
	unsigned int TokenHeaderWord;
	unsigned int TokenWords = 0;
	unsigned int TCRWords = 0;
	void *TCRData = 0;

	TokenBuilder_Params_t TokenParams;
	PEC_CommandDescriptor_t Cmd;
	PEC_NotifyFunction_t CBFunc;

	unsigned int count;
	int i;

	u32 InputToken[IOTOKEN_IN_WORD_COUNT];
	IOToken_Output_Dscr_t OutTokenDscr;
	IOToken_Input_Dscr_t InTokenDscr;
	void *InTokenDscrExt_p = NULL;

#ifdef CRYPTO_IOTOKEN_EXT
	IOToken_Input_Dscr_Ext_t InTokenDscrExt;

	ZEROINIT(InTokenDscrExt);
	InTokenDscrExt_p = &InTokenDscrExt;
#endif
	ZEROINIT(InTokenDscr);
	ZEROINIT(OutTokenDscr);

	rc = SABuilder_Init_Basic(&params, &ProtocolParams, SAB_DIRECTION_OUTBOUND);
	if (rc) {
		CRYPTO_ERR("SABuilder_Init_Basic failed: %d\n", rc);
		goto error_exit;
	}

	params.IV_p = (uint8_t *) ctx->ipad;
	params.AuthAlgo = lookaside_match_hash(ctx->alg);
	params.AuthKey1_p = DummyAuthKey;
	if (params.AuthAlgo == SAB_AUTH_AES_XCBC_MAC) {
		params.AuthKey1_p = (uint8_t *) ctx->ipad + 2 * AES_BLOCK_SIZE;
		params.AuthKey2_p = (uint8_t *) ctx->ipad;
		params.AuthKey3_p = (uint8_t *) ctx->ipad + AES_BLOCK_SIZE;

		for (i = 0; i < AES_BLOCK_SIZE; i = i + 4) {
			swap(params.AuthKey1_p[i], params.AuthKey1_p[i+3]);
			swap(params.AuthKey1_p[i+1], params.AuthKey1_p[i+2]);

			swap(params.AuthKey2_p[i], params.AuthKey2_p[i+3]);
			swap(params.AuthKey2_p[i+1], params.AuthKey2_p[i+2]);

			swap(params.AuthKey3_p[i], params.AuthKey3_p[i+3]);
			swap(params.AuthKey3_p[i+1], params.AuthKey3_p[i+2]);
		}
	}

	if (!finish)
		params.flags |= SAB_FLAG_HASH_SAVE | SAB_FLAG_HASH_INTERMEDIATE;

	params.flags |= SAB_FLAG_SUPPRESS_PAYLOAD;
	ProtocolParams.ICVByteCount = mtk_req->digest_sz;

	rc = SABuilder_GetSizes(&params, &SAWords, NULL, NULL);
	if (rc) {
		CRYPTO_ERR("SA not created because of size errors: %d\n", rc);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_TRANSFORM;
	DMAProperties.Size = MAX(4*SAWords, 256);

	DMAStatus = DMABuf_Alloc(DMAProperties, &SAHostAddress, &SAHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of SA failed: %d\n", DMAStatus);
		goto error_exit;
	}

	rc = SABuilder_BuildSA(&params, (u32 *)SAHostAddress.p, NULL, NULL);
	if (rc) {
		CRYPTO_ERR("SA not created because of errors: %d\n", rc);
		goto error_exit;
	}

	rc = TokenBuilder_GetContextSize(&params, &TCRWords);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_GetContextSize returned errors: %d\n", rc);
		goto error_exit;
	}

	TCRData = kmalloc(4 * TCRWords, GFP_KERNEL);
	if (!TCRData) {
		rc = 1;
		CRYPTO_ERR("Allocation of TCR failed\n");
		goto error_exit;
	}

	rc = TokenBuilder_BuildContext(&params, TCRData);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_BuildContext failed: %d\n", rc);
		goto error_exit;
	}
	mtk_req->token_context = TCRData;

	rc = TokenBuilder_GetSize(TCRData, &TokenMaxWords);
	if (rc) {
		CRYPTO_ERR("TokenBuilder_GetSize failed: %d\n", rc);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_TOKEN;
	DMAProperties.Size = 4*TokenMaxWords;

	DMAStatus = DMABuf_Alloc(DMAProperties, &TokenHostAddress, &TokenHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of token builder failed: %d\n", DMAStatus);
		goto error_exit;
	}

	DMAProperties.fCached = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank = MTK_EIP197_INLINE_BANK_PACKET;
	DMAProperties.Size = MAX(InputByteCount, mtk_req->digest_sz);

	DMAStatus = DMABuf_Alloc(DMAProperties, &PktHostAddress, &PktHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of source packet buffer failed: %d\n",
			   DMAStatus);
		goto error_exit;
	}

	rc = PEC_SA_Register(ring, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);
	if (rc != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC_SA_Register failed: %d\n", rc);
		goto error_exit;
	}
	memcpy(PktHostAddress.p, Input_p, InputByteCount);

	ZEROINIT(TokenParams);
	TokenParams.PacketFlags |= (TKB_PACKET_FLAG_HASHFIRST
				    | TKB_PACKET_FLAG_HASHAPPEND);
	if (finish)
		TokenParams.PacketFlags |= TKB_PACKET_FLAG_HASHFINAL;

	rc = TokenBuilder_BuildToken(TCRData, (u8 *) PktHostAddress.p,
				     InputByteCount, &TokenParams,
				     (u32 *) TokenHostAddress.p,
				     &TokenWords, &TokenHeaderWord);
	if (rc != TKB_STATUS_OK) {
		CRYPTO_ERR("Token builder failed: %d\n", rc);
		goto error_exit_unregister;
	}

	ZEROINIT(Cmd);
	Cmd.Token_Handle = TokenHandle;
	Cmd.Token_WordCount = TokenWords;
	Cmd.SrcPkt_Handle = PktHandle;
	Cmd.SrcPkt_ByteCount = InputByteCount;
	Cmd.DstPkt_Handle = PktHandle;
	Cmd.SA_Handle1 = SAHandle;
	Cmd.SA_Handle2 = DMABuf_NULLHandle;

	mtk_req->sa_pointer = SAHandle.p;

#if defined(CRYPTO_IOTOKEN_EXT)
	InTokenDscrExt.HW_Services  = IOTOKEN_CMD_PKT_LAC;
#endif
	InTokenDscr.TknHdrWordInit = TokenHeaderWord;

	if (!crypto_iotoken_create(&InTokenDscr,
				   InTokenDscrExt_p,
				   InputToken,
				   &Cmd)) {
		rc = 1;
		goto error_exit_unregister;
	}

	do {
		rc = PEC_Packet_Put(ring, &Cmd, 1, &count);
	} while (rc == PEC_STATUS_BUSY);

	if (rc != PEC_STATUS_OK && count != 1) {
		rc = 1;
		CRYPTO_ERR("PEC_Packet_Put error: %d\n", rc);
		goto error_exit_unregister;
	}

	result = kmalloc(sizeof(struct mtk_crypto_result), GFP_KERNEL);
	if (!result) {
		rc = 1;
		CRYPTO_ERR("No memory for result\n");
		goto error_exit_unregister;
	}
	INIT_LIST_HEAD(&result->list);
	result->eip.token_handle = TokenHandle.p;
	result->eip.pkt_handle = PktHandle.p;
	result->async = async;
	result->dst = PktHostAddress.p;

	spin_lock_bh(&priv->mtk_eip_ring[ring].ring_lock);
	list_add_tail(&result->list, &priv->mtk_eip_ring[ring].list);
	spin_unlock_bh(&priv->mtk_eip_ring[ring].ring_lock);
	CBFunc = mtk_crypto_interrupt_handler[ring];
	rc = PEC_ResultNotify_Request(ring, CBFunc, 1);

	return rc;

error_exit_unregister:
	PEC_SA_UnRegister(ring, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);

error_exit:
	DMABuf_Release(SAHandle);
	DMABuf_Release(TokenHandle);
	DMABuf_Release(PktHandle);

	if (TCRData != NULL)
		kfree(TCRData);

	return rc;
}

static bool crypto_basic_hash(struct ahash_request *areq, uint8_t *input,
				unsigned int input_length, uint8_t *output, bool final)
{
	DECLARE_CRYPTO_WAIT(wait);
	struct scatterlist sg;
	int ret;

	ahash_request_set_callback(areq, CRYPTO_TFM_REQ_MAY_BACKLOG, crypto_req_done, &wait);
	sg_init_one(&sg, input, input_length);
	ahash_request_set_crypt(areq, &sg, output, input_length);

	if (final) {
		ret = crypto_ahash_digest(areq);
		if (ret && ret != -EINPROGRESS && ret != -EBUSY) {
			pr_notice("%s: hash update failed: %d\n", __func__, ret);
			return false;
		}
		ret = crypto_wait_req(ret, &wait);
		if (ret) {
			pr_notice("%s: hash failed: %d\n", __func__, ret);
			return false;
		}
	} else {
		ret = crypto_ahash_init(areq);
		if (ret) {
			pr_notice("%s: hash init failed: %d\n", __func__, ret);
			return false;
		}

		ret = crypto_ahash_update(areq);
		if (ret && ret != -EINPROGRESS && ret != -EBUSY) {
			pr_notice("%s: hash update failed: %d\n", __func__, ret);
			return false;
		}

		ret = crypto_wait_req(ret, &wait);
		if (ret) {
			pr_notice("%s: hash failed: %d\n", __func__, ret);
			return false;
		}

		ret = crypto_ahash_export(areq, output);
		if (ret) {
			pr_notice("%s: hash export failed: %d\n", __func__, ret);
			return false;
		}
	}

	return true;
}

static void mtk_crypto_hash_byteswap(SABuilder_Auth_t algo, uint8_t *input, unsigned int length)
{
	int i;
	u32 temp;

	if (algo == SAB_AUTH_HMAC_MD5)
		return;

	/* Byte swapping for 32-bit word hash (ex. sha1, sha256) */
	for (i = 0; i < (length / 4); i++)
		((__be32 *)input)[i] = cpu_to_be32(((__le32 *)input)[i]);

	/* Extra swap for 64-bit word hash (ex. sha384, sha512) */
	if (algo == SAB_AUTH_HMAC_SHA2_384 || algo == SAB_AUTH_HMAC_SHA2_512) {
		for (i = 0; i < (length / 4); i = i + 2) {
			temp = ((u32 *)input)[i];
			((u32 *)input)[i] = ((u32 *)input)[i + 1];
			((u32 *)input)[i + 1] = temp;
		}
	}
}

bool crypto_hmac_precompute(SABuilder_Auth_t AuthAlgo,
			    uint8_t *AuthKey_p,
			    unsigned int AuthKeyByteCount,
			    uint8_t *Inner_p,
			    uint8_t *Outer_p)
{
	unsigned int blocksize, digestsize;
	uint8_t pad_block[128], hashed_key[128];
	unsigned int i;
	struct ahash_request *areq = NULL;
	struct crypto_ahash *atfm = NULL;
	bool ret = false;
	uint8_t *state = NULL;

	switch (AuthAlgo) {
	case SAB_AUTH_HMAC_MD5:
		atfm = crypto_alloc_ahash("md5-generic", 0, 0);
		digestsize = 16;
		break;
	case SAB_AUTH_HMAC_SHA1:
		atfm = crypto_alloc_ahash("sha1-generic", 0, 0);
		digestsize = 20;
		break;
	case SAB_AUTH_HMAC_SHA2_224:
		atfm = crypto_alloc_ahash("sha224-generic", 0, 0);
		/*
		 * The final hash size of sha224 is 28 bytes, but for intermediate calculate
		 * the digest size is same as sha256, so it's 32 bytes.
		 */
		digestsize = 32;
		break;
	case SAB_AUTH_HMAC_SHA2_256:
		atfm = crypto_alloc_ahash("sha256-generic", 0, 0);
		digestsize = 32;
		break;
	case SAB_AUTH_HMAC_SHA2_384:
		atfm = crypto_alloc_ahash("sha384-generic", 0, 0);
		/* As sha224, the digest size of sha384 is the size of sha512 */
		digestsize = 64;
		break;
	case SAB_AUTH_HMAC_SHA2_512:
		atfm = crypto_alloc_ahash("sha512-generic", 0, 0);
		digestsize = 64;
		break;
	default:
		CRYPTO_ERR("Unknown HMAC algorithm\n");
		return false;
	}

	if (IS_ERR(atfm)) {
		pr_err("%s: hash: failed to allocate atfm\n", __func__);
		return false;
	}

	blocksize = crypto_ahash_blocksize(atfm);
	state = kzalloc(crypto_ahash_statesize(atfm), GFP_KERNEL);
	if (!state)
		goto free_ahash;

	areq = ahash_request_alloc(atfm, GFP_KERNEL);
	if (!areq)
		goto free_ahash;

	crypto_ahash_clear_flags(atfm, ~0);

	memset(hashed_key, 0, blocksize);
	if (AuthKeyByteCount <= blocksize) {
		memcpy(hashed_key, AuthKey_p, AuthKeyByteCount);
	} else {
		if (!crypto_basic_hash(areq, AuthKey_p, AuthKeyByteCount, hashed_key, true))
			goto free_request;
	}

	for (i = 0; i < blocksize; i++)
		pad_block[i] = hashed_key[i] ^ 0x36;

	if (!crypto_basic_hash(areq, pad_block, blocksize, state, false))
		goto free_request;
	memcpy(Inner_p, state, digestsize);
	mtk_crypto_hash_byteswap(AuthAlgo, Inner_p, digestsize);

	for (i = 0; i < blocksize; i++)
		pad_block[i] = hashed_key[i] ^ 0x5c;

	if (!crypto_basic_hash(areq, pad_block, blocksize, state, false))
		goto free_request;
	memcpy(Outer_p, state, digestsize);
	mtk_crypto_hash_byteswap(AuthAlgo, Outer_p, digestsize);

	ret = true;

free_request:
	kfree(state);
	ahash_request_free(areq);
free_ahash:
	crypto_free_ahash(atfm);

	return ret;
}

static bool
mtk_ddk_aes_block_encrypt(uint8_t *key,  unsigned int klen, uint8_t *input, uint8_t *output)
{
	struct crypto_skcipher *tfm;
	struct skcipher_request *req = NULL;
	struct scatterlist src, dst;
	int ret = 0;
	void *iv = NULL;
	bool val = false;
	DECLARE_CRYPTO_WAIT(wait);

	tfm = crypto_alloc_skcipher("ecb(aes)", 0, 0);
	if (IS_ERR(tfm)) {
		pr_err("skcipher: failed to allocate tfm\n");
		return PTR_ERR(tfm);
	}

	req = skcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		pr_err("skcipher: failed to allocate request\n");
		goto out;
	}

	ret = crypto_skcipher_setkey(tfm, key, klen);
	if (ret) {
		pr_err("skcipher: setkey failed\n");
		goto out;
	}

	sg_init_one(&src, input, AES_BLOCK_SIZE);
	sg_init_one(&dst, output, AES_BLOCK_SIZE);

	skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG, crypto_req_done, &wait);
	skcipher_request_set_crypt(req, &src, &dst, AES_BLOCK_SIZE, iv);
	ret = crypto_skcipher_encrypt(req);
	ret = crypto_wait_req(ret, &wait);
	if (ret) {
		pr_notice("aead failed to encrypt/decrypt: %d\n", ret);
		goto out;
	}

	val = true;
out:
	skcipher_request_free(req);
	crypto_free_skcipher(tfm);

	return val;
}

static bool set_crypto_aead(struct xfrm_algo_aead *aead, SABuilder_Params_t *params)
{
	params->Key_p = aead->alg_key;
	params->CryptoAlgo = SAB_CRYPTO_AES;
	params->KeyByteCount = aead->alg_key_len / 8;
	if (strcmp(aead->alg_name, "rfc4106(gcm(aes))") == 0) {
		params->CryptoMode = SAB_CRYPTO_MODE_GCM;
		params->KeyByteCount = (aead->alg_key_len / 8) - CTR_RFC3686_NONCE_SIZE;
		params->Nonce_p = aead->alg_key + params->KeyByteCount;
	} else if (strcmp(aead->alg_name, "rfc4543(gcm(aes))") == 0) {
		params->CryptoMode = SAB_CRYPTO_MODE_GMAC;
		params->KeyByteCount = (aead->alg_key_len / 8) - CTR_RFC3686_NONCE_SIZE;
		params->Nonce_p = aead->alg_key + params->KeyByteCount;
	} else if (strcmp(aead->alg_name, "rfc4309(ccm(aes))") == 0) {
		params->CryptoMode = SAB_CRYPTO_MODE_CCM;
		params->KeyByteCount = (aead->alg_key_len / 8) - 3;
		params->Nonce_p = aead->alg_key + params->KeyByteCount;
	} else
		return false;

	return true;
}

static bool set_crypto_ealg(struct xfrm_algo *ealg, SABuilder_Params_t *params)
{
	params->CryptoMode = SAB_CRYPTO_MODE_CBC;
	params->KeyByteCount = ealg->alg_key_len / 8;
	params->Key_p = ealg->alg_key;
	if (strcmp(ealg->alg_name, "cbc(des)") == 0)
		params->CryptoAlgo = SAB_CRYPTO_DES;
	else if (strcmp(ealg->alg_name, "cbc(aes)") == 0)
		params->CryptoAlgo = SAB_CRYPTO_AES;
	else if (strcmp(ealg->alg_name, "cbc(des3_ede)") == 0)
		params->CryptoAlgo = SAB_CRYPTO_3DES;
	else
		return false;

	return true;
}

static bool set_auth_aead(struct xfrm_algo_aead *aead,
			  struct mtk_xfrm_params *xfrm_params,
			  SABuilder_Params_t *params, uint8_t *hash_key)
{
	uint8_t t;
	unsigned int i;

	xfrm_params->tr_type = EIP197_SMALL_TR;
	if (strcmp(aead->alg_name, "rfc4106(gcm(aes))") == 0) {
		params->AuthAlgo = SAB_AUTH_AES_GCM;
		hash_key = kcalloc(AES_BLOCK_SIZE, sizeof(uint8_t), GFP_KERNEL);
		if (!hash_key)
			return false;
		memset(hash_key, 0, AES_BLOCK_SIZE);
		mtk_ddk_aes_block_encrypt(params->Key_p, params->KeyByteCount, hash_key, hash_key);
	} else if (strcmp(aead->alg_name, "rfc4543(gcm(aes))") == 0) {
		params->AuthAlgo = SAB_AUTH_AES_GMAC;
		hash_key = kcalloc(AES_BLOCK_SIZE, sizeof(uint8_t), GFP_KERNEL);
		if (!hash_key)
			return false;
		memset(hash_key, 0, AES_BLOCK_SIZE);
		mtk_ddk_aes_block_encrypt(params->Key_p, params->KeyByteCount, hash_key, hash_key);
	} else if (strcmp(aead->alg_name, "rfc4309(ccm(aes))") == 0) {
		params->AuthAlgo = SAB_AUTH_AES_CCM;
		return true;
	} else
		return false;

	/* Byte-swap the hash key */
	for (i = 0; i < 4; i++) {
		t = hash_key[4*i+3];
		hash_key[4*i+3] = hash_key[4*i];
		hash_key[4*i] = t;
		t = hash_key[4*i+2];
		hash_key[4*i+2] = hash_key[4*i+1];
		hash_key[4*i+1] = t;
	}
	params->AuthKey1_p = hash_key;

	return true;
}

static bool set_auth_aalg(struct xfrm_algo_auth *aalg,
			  struct mtk_xfrm_params *xfrm_params,
			  SABuilder_Params_t *params, uint8_t *inner, uint8_t *outer)
{
	if (strcmp(aalg->alg_name, "hmac(sha1)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA1;
		inner = kcalloc(SHA1_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA1_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA1, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
		xfrm_params->tr_type = EIP197_SMALL_TR;
	} else if (strcmp(aalg->alg_name, "hmac(sha256)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_256;
		inner = kcalloc(SHA256_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA256_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA2_256, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
		xfrm_params->tr_type = EIP197_SMALL_TR;
	} else if (strcmp(aalg->alg_name, "hmac(sha384)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_384;
		inner = kcalloc(SHA512_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA512_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA2_384, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
		xfrm_params->tr_type = EIP197_LARGE_TR;
	} else if (strcmp(aalg->alg_name, "hmac(sha512)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_512;
		inner = kcalloc(SHA512_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA512_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA2_512, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
		xfrm_params->tr_type = EIP197_LARGE_TR;
	} else if (strcmp(aalg->alg_name, "hmac(md5)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_MD5;
		inner = kcalloc(MD5_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(MD5_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_MD5, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
		xfrm_params->tr_type = EIP197_SMALL_TR;
	} else {
		return false;
	}

	return true;
}

void *mtk_ddk_tr_ipsec_build(struct mtk_xfrm_params *xfrm_params, u32 ipsec_mode)
{
	struct xfrm_state *xs = xfrm_params->xs;
	SABuilder_Params_IPsec_t ipsec_params;
	SABuilder_Status_t sa_status;
	SABuilder_Params_t params;
	bool set_success = false;
	unsigned int SAWords = 0;
	uint8_t *inner = NULL;
	uint8_t *outer = NULL;

	DMABuf_Status_t dma_status;
	DMABuf_Properties_t dma_properties = {0, 0, 0, 0};
	DMABuf_HostAddress_t sa_host_addr;

	DMABuf_Handle_t sa_handle = {0};
	PEC_Status_t pec_status;

	if (xs->props.family == AF_INET)
		sa_status = SABuilder_Init_ESP(&params,
					&ipsec_params,
					be32_to_cpu(xs->id.spi),
					ipsec_mode,
					SAB_IPSEC_IPV4,
					xfrm_params->dir);
	else
		sa_status = SABuilder_Init_ESP(&params,
					&ipsec_params,
					be32_to_cpu(xs->id.spi),
					ipsec_mode,
					SAB_IPSEC_IPV6,
					xfrm_params->dir);

	if (sa_status != SAB_STATUS_OK) {
		pr_err("SABuilder_Init_ESP failed\n");
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	/* Check algorithm exist in xfrm state*/
	if (!xs->aead && (!xs->ealg || !xs->aalg)) {
		CRYPTO_ERR("NULL algorithm in xfrm state\n");
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	/* Add crypto key and parameters */
	if (xs->aead)
		set_success = set_crypto_aead(xs->aead, &params);
	else
		set_success = set_crypto_ealg(xs->ealg, &params);
	if (set_success != true) {
		CRYPTO_ERR("Set Crypto Algo failed\n");
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	/* Add authentication key and parameters */
	if (xs->aead)
		set_success = set_auth_aead(xs->aead, xfrm_params, &params, inner);
	else
		set_success = set_auth_aalg(xs->aalg, xfrm_params, &params, inner, outer);
	if (set_success != true) {
		CRYPTO_ERR("Set Auth Algo failed\n");
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	if (xs->encap) {
		ipsec_params.IPsecFlags |= SAB_IPSEC_NATT;
		ipsec_params.NATTSrcPort = be16_to_cpu(xs->encap->encap_sport);
		ipsec_params.NATTDestPort = be16_to_cpu(xs->encap->encap_dport);
	}

	ipsec_params.IPsecFlags |= (SAB_IPSEC_PROCESS_IP_HEADERS
				    | SAB_IPSEC_EXT_PROCESSING);
	if (ipsec_mode == SAB_IPSEC_TUNNEL) {
		if (xs->props.family == AF_INET) {
			ipsec_params.SrcIPAddr_p = (uint8_t *) &xs->props.saddr.a4;
			ipsec_params.DestIPAddr_p = (uint8_t *) &xs->id.daddr.a4;
		} else {
			ipsec_params.SrcIPAddr_p = (uint8_t *) &xs->props.saddr.a6;
			ipsec_params.DestIPAddr_p = (uint8_t *) &xs->id.daddr.a6;
		}
	}

	if (xs->aead)
		ipsec_params.ICVByteCount = xs->aead->alg_icv_len / 8;
	else
		ipsec_params.ICVByteCount = xs->aalg->alg_trunc_len / 8;

	sa_status = SABuilder_GetSizes(&params, &SAWords, NULL, NULL);
	if (sa_status != SAB_STATUS_OK) {
		CRYPTO_ERR("SA not created because of size errors\n");
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	dma_properties.fCached = true;
	dma_properties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	dma_properties.Bank = MTK_EIP197_INLINE_BANK_TRANSFORM;
	dma_properties.Size = SAWords * sizeof(u32);

	dma_status = DMABuf_Alloc(dma_properties, &sa_host_addr, &sa_handle);
	if (dma_status != DMABUF_STATUS_OK) {
		CRYPTO_ERR("Allocation of SA failed\n");
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	sa_status = SABuilder_BuildSA(&params, (u32 *) sa_host_addr.p, NULL, NULL);
	if (sa_status != SAB_STATUS_OK) {
		CRYPTO_ERR("SA not created because of errors\n");
		DMABuf_Release(sa_handle);
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	pec_status = PEC_SA_Register(0, sa_handle, DMABuf_NULLHandle, DMABuf_NULLHandle);
	if (pec_status != PEC_STATUS_OK) {
		CRYPTO_ERR("%s: PEC_SA_Register failed\n", __func__);
		DMABuf_Release(sa_handle);
		sa_host_addr.p = NULL;
		goto error_ret;
	}

	xfrm_params->p_handle = sa_handle.p;

error_ret:
	kfree(inner);
	kfree(outer);
	return sa_host_addr.p;
}

int mtk_ddk_pec_init(void)
{
	PEC_InitBlock_t pec_init_blk = {0, 0, false};
	PEC_Capabilities_t pec_cap;
	PEC_Status_t pec_sta;
	u32 i = MTK_EIP197_INLINE_NOF_TRIES;
	u32 j;
#ifdef PEC_PCL_EIP197
	PCL_Status_t pcl_sta;
#endif

#ifdef PEC_PCL_EIP197
	pcl_sta = PCL_Init(PCL_INTERFACE_ID, 1);
	if (pcl_sta != PCL_STATUS_OK) {
		CRYPTO_ERR("PCL could not be initialized, error=%d\n", pcl_sta);
		return 0;
	}

	pcl_sta = PCL_DTL_Init(PCL_INTERFACE_ID);
	if (pcl_sta != PCL_STATUS_OK) {
		CRYPTO_ERR("PCL-DTL could not be initialized, error=%d\n", pcl_sta);
		return -1;
	}
#endif
	for (j = 0; j < PEC_MAX_INTERFACE_NUM; j++) {
		while (i) {
			pec_sta = PEC_Init(j, &pec_init_blk);
			if (pec_sta == PEC_STATUS_OK) {
				CRYPTO_INFO("PEC_INIT interface %d ok!\n", j);
				break;
			} else if (pec_sta != PEC_STATUS_OK && pec_sta != PEC_STATUS_BUSY) {
				return pec_sta;
			}

			mdelay(MTK_EIP197_INLINE_RETRY_DELAY_MS);
			i--;
		}
	}

	if (!i) {
		CRYPTO_ERR("PEC could not be initialized: %d\n", pec_sta);
		return pec_sta;
	}

	pec_sta = PEC_Capabilities_Get(&pec_cap);
	if (pec_sta != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC capability could not be obtained: %d\n", pec_sta);
#ifdef PEC_PCL_EIP197
		PCL_UnInit(PCL_INTERFACE_ID);
#endif
		return pec_sta;
	}

	CRYPTO_INFO("PEC Capabilities: %s\n", pec_cap.szTextDescription);

	return 0;
}

void mtk_ddk_pec_deinit(void)
{
	unsigned int LoopCounter = MTK_EIP197_INLINE_NOF_TRIES;
	PEC_Status_t PEC_Status;
	int j;

	for (j = 0; j < PEC_MAX_INTERFACE_NUM; j++) {
		while (LoopCounter > 0) {
			PEC_Status = PEC_UnInit(j);
			if (PEC_Status == PEC_STATUS_OK)
				break;
			else if (PEC_Status != PEC_STATUS_OK && PEC_Status != PEC_STATUS_BUSY) {
				CRYPTO_ERR("PEC could not deinit, error=%d\n", PEC_Status);
				return;
			}
			// Wait for MTK_EIP197_INLINE_RETRY_DELAY_MS milliseconds
			udelay(MTK_EIP197_INLINE_RETRY_DELAY_MS * 1000);
			LoopCounter--;
		}
		// Check for timeout
		if (LoopCounter == 0) {
			CRYPTO_ERR("PEC could not be un-initialized, timeout\n");
			return;
		}
	}

#ifdef PEC_PCL_EIP197
	PCL_DTL_UnInit(PCL_INTERFACE_ID);
	PCL_UnInit(PCL_INTERFACE_ID);
#endif
}

bool
mtk_ddk_invalidate_rec(
		void *sa_p,
		const bool IsTransform)
{
	PEC_Status_t PEC_Rc;
	PEC_CommandDescriptor_t Cmd;
	PEC_ResultDescriptor_t Res;
	unsigned int count;
	IOToken_Input_Dscr_t InTokenDscr;
	IOToken_Output_Dscr_t OutTokenDscr;
	uint32_t InputToken[IOTOKEN_IN_WORD_COUNT_IL];
	uint32_t OutputToken[IOTOKEN_OUT_WORD_COUNT_IL];
	void *InTokenDscrExt_p = NULL;
	void *OutTokenDscrExt_p = NULL;
	IOToken_Input_Dscr_Ext_t InTokenDscrExt;
	IOToken_Output_Dscr_Ext_t OutTokenDscrExt;
	DMABuf_Handle_t Rec_p;

	Rec_p.p = sa_p;
	ZEROINIT(InTokenDscrExt);
	InTokenDscrExt_p = &InTokenDscrExt;
	OutTokenDscrExt_p = &OutTokenDscrExt;

	ZEROINIT(InTokenDscr);

	// Fill in the command descriptor for the Invalidate command
	ZEROINIT(Cmd);

	Cmd.SrcPkt_Handle    = DMABuf_NULLHandle;
	Cmd.DstPkt_Handle    = DMABuf_NULLHandle;
	Cmd.SA_Handle1       = Rec_p;
	Cmd.SA_Handle2       = DMABuf_NULLHandle;
	Cmd.Token_Handle     = DMABuf_NULLHandle;
	Cmd.SrcPkt_ByteCount = 0;

#if defined(IOTOKEN_USE_HW_SERVICE)
	if (IsTransform)
		InTokenDscrExt.HW_Services = IOTOKEN_CMD_INV_TR;
	else
		InTokenDscrExt.HW_Services = IOTOKEN_CMD_INV_FR;
#endif

	if (!crypto_iotoken_create(&InTokenDscr, InTokenDscrExt_p, InputToken, &Cmd))
		return false;

	// Issue command
	do {
		PEC_Rc = PEC_Packet_Put(PEC_INTERFACE_ID, &Cmd, 1, &count);
	} while (PEC_Rc == PEC_STATUS_BUSY);

	if (PEC_Rc != PEC_STATUS_OK || count != 1) {
		CRYPTO_ERR("%s: PEC_Packet_Put() error %d, count %d\n",
				 __func__,
				 PEC_Rc,
				 count);
		return false;
	}

	// Receive the result packet ... do we care about contents ?
	if (crypto_pe_busy_get_one(&OutTokenDscr, OutputToken, &Res, PEC_INTERFACE_ID) < 1) {
		CRYPTO_ERR("%s: crypto_pe_busy_get_one() failed\n", __func__);
		return false;
	}

	return true;
}

static void set_capwap_algo(SABuilder_Params_t *params, uint8_t mode)
{
	params->CryptoAlgo = SAB_CRYPTO_AES;
	params->CryptoMode = SAB_CRYPTO_MODE_CBC;
	params->KeyByteCount = 16;

	switch (mode) {
	case AES256_CBC_HMAC_SHA1:
		params->KeyByteCount = 32;
		params->AuthAlgo = SAB_AUTH_HMAC_SHA1;
		params->AuthKeyByteCount = 20;
		break;
	case AES128_CBC_HMAC_SHA1:
		params->AuthAlgo = SAB_AUTH_HMAC_SHA1;
		params->AuthKeyByteCount = 20;
		break;
	case AES256_CBC_HMAC_SHA2_256:
		params->KeyByteCount = 32;
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_256;
		params->AuthKeyByteCount = 32;
		break;
	case AES128_CBC_HMAC_SHA2_256:
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_256;
		params->AuthKeyByteCount = 32;
		break;
	case AES256_GCM:
		params->KeyByteCount = 32;
		params->CryptoMode = SAB_CRYPTO_MODE_GCM;
		params->AuthAlgo = SAB_AUTH_AES_GCM;
		break;
	case AES128_GCM:
		params->CryptoMode = SAB_CRYPTO_MODE_GCM;
		params->AuthAlgo = SAB_AUTH_AES_GCM;
		break;
	default:
		CRYPTO_ERR("No algorithms match for capwap-dtls\n");
		params->CryptoAlgo = SAB_CRYPTO_NULL;
		break;
	}

	return;
}

void *mtk_ddk_tr_capwap_dtls_build(
		const bool capwap,
		struct DTLS_param *DTLSParam_p, u32 dir)
{
	SABuilder_Status_t sa_status;
	SABuilder_Params_t params;
	SABuilder_Params_SSLTLS_t ssl_tls_params;
	uint16_t dtls_version;
	uint32_t sa_words;

	static uint8_t zeros[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t *hash_key = NULL;
	uint8_t *inner = NULL;
	uint8_t *outer = NULL;

	DMABuf_Handle_t sa_handle = {0};
	DMABuf_HostAddress_t sa_host_addr;
	DMABuf_Status_t DMAStatus;
	DMABuf_Properties_t DMAProperties = {0, 0, 0, 0};

	if (capwap)
		CRYPTO_INFO("Preparing Transforms for DTLS-CAPWAP\n");
	else
		CRYPTO_INFO("Preparing Transforms for DTLS\n");

	if (DTLSParam_p->dtls_version == MTK_DTLS_VERSION_1_0)
		dtls_version = SAB_DTLS_VERSION_1_0;
	else if (DTLSParam_p->dtls_version == MTK_DTLS_VERSION_1_2)
		dtls_version = SAB_DTLS_VERSION_1_2;
	else {
		CRYPTO_ERR("%s: Unknow dtls version: %u\n", __func__, DTLSParam_p->dtls_version);
		sa_handle.p = NULL;
		return sa_handle.p;
	}

	sa_status = SABuilder_Init_SSLTLS(&params,
									&ssl_tls_params,
									dtls_version,
									dir);

	if (dir == SAB_DIRECTION_OUTBOUND) {
		params.Nonce_p = DTLSParam_p->dtls_encrypt_nonce;
		params.Key_p = DTLSParam_p->key_encrypt;
	} else {
		params.Nonce_p = DTLSParam_p->dtls_decrypt_nonce;
		params.Key_p = DTLSParam_p->key_decrypt;
	}

	set_capwap_algo(&params, DTLSParam_p->sec_mode);
	if (params.CryptoAlgo == SAB_CRYPTO_NULL) {
		sa_handle.p = NULL;
		return sa_handle.p;
	}

	if (params.AuthAlgo == SAB_AUTH_AES_GCM) {
		hash_key = kcalloc(16, sizeof(uint8_t), GFP_KERNEL);
		if (hash_key == NULL) {
			CRYPTO_ERR("%s: kcalloc for hash key failed\n", __func__);
			sa_handle.p = NULL;
			return sa_handle.p;
		}

		mtk_ddk_aes_block_encrypt(params.Key_p, 16, zeros, hash_key);

		/* Byte-swap the hash key */
		{
			uint8_t t;
			unsigned int i;

			for (i = 0; i < 4; i++) {
				t = hash_key[4*i+3];
				hash_key[4*i+3] = hash_key[4*i];
				hash_key[4*i] = t;
				t = hash_key[4*i+2];
				hash_key[4*i+2] = hash_key[4*i+1];
				hash_key[4*i+1] = t;
			}
		}
		params.AuthKey1_p = hash_key;
	} else {
		/* Set authkey for HMAC */
		inner = kcalloc((size_t) params.AuthKeyByteCount, sizeof(uint8_t), GFP_KERNEL);
		if (!inner) {
			CRYPTO_ERR("%s: kmalloc for hmac inner digest failed\n", __func__);
			sa_handle.p = NULL;
			return sa_handle.p;
		}

		outer = kcalloc((size_t) params.AuthKeyByteCount, sizeof(uint8_t), GFP_KERNEL);
		if (!outer) {
			CRYPTO_ERR("%s: kmalloc for hmac outer digest failed\n", __func__);
			kfree(inner);
			sa_handle.p = NULL;
			return sa_handle.p;
		}

		memset(inner, 0, params.AuthKeyByteCount);
		memset(outer, 0, params.AuthKeyByteCount);
		crypto_hmac_precompute(params.AuthAlgo, params.Key_p,
							params.AuthKeyByteCount, inner, outer);
		params.AuthKey1_p = inner;
		params.AuthKey2_p = outer;
	}

	ssl_tls_params.epoch = DTLSParam_p->dtls_epoch;
	ssl_tls_params.SSLTLSFlags |= SAB_DTLS_PROCESS_IP_HEADERS |
						SAB_DTLS_EXT_PROCESSING;
	if (DTLSParam_p->net_type == MTK_DTLS_NET_IPV6)
		ssl_tls_params.SSLTLSFlags |= SAB_DTLS_IPV6;
	else
		ssl_tls_params.SSLTLSFlags |= SAB_DTLS_IPV4;

	if (capwap)
		ssl_tls_params.SSLTLSFlags |= SAB_DTLS_CAPWAP;

	sa_status = SABuilder_GetSizes(&params, &sa_words, NULL, NULL);
	if (sa_status != SAB_STATUS_OK) {
		sa_handle.p = NULL;
		goto free_exit;
	}

	/* Allocate a DMA-safe buffer for the SA. */
	DMAProperties.fCached   = true;
	DMAProperties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	DMAProperties.Bank	  = MTK_EIP197_INLINE_BANK_TRANSFORM;
	DMAProperties.Size	  = sa_words * sizeof(uint32_t);

	DMAStatus = DMABuf_Alloc(DMAProperties, &sa_host_addr, &sa_handle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		CRYPTO_ERR("%s: allocate dma buffer for sa failed\n", __func__);
		sa_handle.p = NULL;
		goto free_exit;
	}

	sa_status = SABuilder_BuildSA(&params, (uint32_t *) sa_host_addr.p, NULL, NULL);
	if (sa_status != SAB_STATUS_OK) {
		CRYPTO_ERR("%s: SA not created because of errors\n", __func__);
		DMABuf_Release(sa_handle);
		sa_handle.p = NULL;
		goto free_exit;
	}

free_exit:
	kfree(inner);
	kfree(outer);
	kfree(hash_key);

	return sa_handle.p;
}

int mtk_ddk_pcl_capwap_dtls_build(
		struct DTLS_param *DTLSParam_p,
		struct DTLSResourceMgmt *dtls_resource, u32 dir)
{
	PCL_Status_t pcl_status;
	PCL_SelectorParams_t selector;
	PCL_DTL_TransformParams_t dtls_trans;
	PCL_DTL_Hash_Handle_t sa_hash_handle;
	DMABuf_Handle_t sa;

	if (dir == SAB_DIRECTION_OUTBOUND)
		sa = dtls_resource->sa_out;
	else
		sa = dtls_resource->sa_in;

	pcl_status = PCL_Transform_Register(sa);
	if (pcl_status != PCL_STATUS_OK) {
		CRYPTO_ERR("%s: PCL_Transform_Register outbound failed\n", __func__);
		return -1;
	}

	ZEROINIT(selector);
	ZEROINIT(dtls_trans);

	if (dir == SAB_DIRECTION_OUTBOUND) {
		if (DTLSParam_p->net_type == MTK_DTLS_NET_IPV6) {
			selector.flags = PCL_SELECT_IPV6;
			selector.SrcIp = ((unsigned char *)(&(DTLSParam_p->sip.ip6.addr)));
			selector.DstIp = ((unsigned char *)(&(DTLSParam_p->dip.ip6.addr)));
		} else {
			selector.flags = PCL_SELECT_IPV4;
			selector.SrcIp = ((unsigned char *)(&(DTLSParam_p->sip.ip4.addr32)));
			selector.DstIp = ((unsigned char *)(&(DTLSParam_p->dip.ip4.addr32)));
		}
		selector.epoch = 0;
		selector.SrcPort = DTLSParam_p->sport;
		selector.DstPort = DTLSParam_p->dport;
	} else {
		if (DTLSParam_p->net_type == MTK_DTLS_NET_IPV6) {
			selector.flags = PCL_SELECT_IPV6;
			/* For inbound, PCL SrcIP should be dip,
			 * and DstIP should be sip in DTLSParam_p.
			 */
			selector.SrcIp = ((unsigned char *)(&(DTLSParam_p->dip.ip6.addr)));
			selector.DstIp = ((unsigned char *)(&(DTLSParam_p->sip.ip6.addr)));
		} else {
			selector.flags = PCL_SELECT_IPV4;
			/* For inbound, PCL SrcIP should be dip,
			 * and DstIP should be sip in DTLSParam_p.
			 */
			selector.SrcIp = ((unsigned char *)(&(DTLSParam_p->dip.ip4.addr32)));
			selector.DstIp = ((unsigned char *)(&(DTLSParam_p->sip.ip4.addr32)));
		}
		selector.epoch = DTLSParam_p->dtls_epoch;
		/* src port and dst port should reverse for inbound, too */
		selector.SrcPort = DTLSParam_p->dport;
		selector.DstPort = DTLSParam_p->sport;
	}
	selector.IpProto = 17;
	selector.spi = 0;

	pcl_status = PCL_Flow_Hash(&selector, dtls_trans.HashID);
	if (pcl_status != PCL_STATUS_OK) {
		CRYPTO_ERR("%s: PEC_Flow_Hash failed\n", __func__);
		goto unregister_exit;
	}

	pcl_status = PCL_DTL_Transform_Add(PCL_INTERFACE_ID, 0,
						&dtls_trans, sa, &sa_hash_handle);
	if (pcl_status != PCL_STATUS_OK) {
		CRYPTO_ERR("%s: PEC_DTL_Transform_Add failed\n", __func__);
		goto unregister_exit;
	}
	return 0;
unregister_exit:
	mtk_ddk_invalidate_rec(sa.p, true);
	PCL_Transform_UnRegister(sa);
	return -1;
}

void mtk_ddk_remove_dtls_sa(struct DTLSResourceMgmt *dtls_res)
{
	if (!dtls_res) {
		CRYPTO_ERR("Free NULL DTLS resource!\n");
		return;
	}

	if (dtls_res->sa_out.p) {
		DMABuf_Release(dtls_res->sa_out);
		dtls_res->sa_out.p = NULL;
	}

	if (dtls_res->sa_in.p) {
		DMABuf_Release(dtls_res->sa_in);
		dtls_res->sa_in.p = NULL;
	}

	return;
}

void mtk_ddk_remove_dtls_pcl(struct DTLSResourceMgmt *dtls_res, u32 dir)
{
	PCL_Status_t pcl_status;
	DMABuf_Handle_t sa = {0};
	int ret;

	if (!dtls_res)
		return;

	if (dir == SAB_DIRECTION_OUTBOUND)
		sa = dtls_res->sa_out;
	else
		sa = dtls_res->sa_in;
	if (!sa.p)
		return;

	pcl_status = PCL_DTL_Transform_Remove(PCL_INTERFACE_ID, 0, sa);
	ret = mtk_ddk_invalidate_rec(sa.p, true);
	pcl_status = PCL_Transform_UnRegister(sa);

	return;
}

void mtk_ddk_remove_dtls_param(struct DTLSResourceMgmt *dtls_res)
{
	if (!dtls_res)
		return;
	mtk_ddk_remove_dtls_pcl(dtls_res, SAB_DIRECTION_OUTBOUND);
	mtk_ddk_remove_dtls_pcl(dtls_res, SAB_DIRECTION_INBOUND);
	mtk_ddk_remove_dtls_sa(dtls_res);

	return;
}
