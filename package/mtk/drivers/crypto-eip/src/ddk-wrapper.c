// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <crypto/aes.h>
#include <crypto/hash.h>
#include <crypto/hmac.h>
#include <crypto/md5.h>
#include <linux/delay.h>

#include <crypto-eip/ddk/slad/api_pcl.h>
#include <crypto-eip/ddk/slad/api_pcl_dtl.h>
#include <crypto-eip/ddk/slad/api_pec.h>
#include <crypto-eip/ddk/slad/api_driver197_init.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/internal.h"
#include "crypto-eip/crypto-eip197-inline-ddk.h"

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

unsigned int crypto_pe_get_one(IOToken_Output_Dscr_t *const OutTokenDscr_p,
			       u32 *OutTokenData_p,
			       PEC_ResultDescriptor_t *RD_p)
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

		pecres = PEC_Packet_Get(PEC_INTERFACE_ID, RD_p, 1, &Counter);
		if (pecres != PEC_STATUS_OK) {
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

		/* Wait for MTK_EIP197_PKT_GET_TIMEOUT_MS milliseconds */
		udelay(MTK_EIP197_PKT_GET_TIMEOUT_MS * 1000);
		LoopCounter--;
	}

	CRYPTO_ERR("Timeout when reading packet\n");

	/* IO error (timeout, not result packet received) */
	return 0;
}


bool crypto_basic_hash(SABuilder_Auth_t HashAlgo, uint8_t *Input_p,
				unsigned int InputByteCount, uint8_t *Output_p,
				unsigned int OutputByteCount, bool fFinalize)
{
	SABuilder_Params_Basic_t ProtocolParams;
	SABuilder_Params_t params;
	unsigned int SAWords = 0;
	static uint8_t DummyAuthKey[64];
	int rc;

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
	PEC_ResultDescriptor_t Res;
	unsigned int count;

	u32 OutputToken[IOTOKEN_IN_WORD_COUNT];
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

	params.AuthAlgo = HashAlgo;
	params.AuthKey1_p = DummyAuthKey;

	if (!fFinalize)
		params.flags |= SAB_FLAG_HASH_SAVE | SAB_FLAG_HASH_INTERMEDIATE;
	params.flags |= SAB_FLAG_SUPPRESS_PAYLOAD;
	ProtocolParams.ICVByteCount = OutputByteCount;

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
	DMAProperties.Size = MAX(InputByteCount, OutputByteCount);

	DMAStatus = DMABuf_Alloc(DMAProperties, &PktHostAddress, &PktHandle);
	if (DMAStatus != DMABUF_STATUS_OK) {
		rc = 1;
		CRYPTO_ERR("Allocation of source packet buffer failed: %d\n",
			   DMAStatus);
		goto error_exit;
	}

	rc = PEC_SA_Register(PEC_INTERFACE_ID, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);
	if (rc != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC_SA_Register failed: %d\n", rc);
		goto error_exit;
	}

	memcpy(PktHostAddress.p, Input_p, InputByteCount);

	ZEROINIT(TokenParams);
	TokenParams.PacketFlags |= (TKB_PACKET_FLAG_HASHFIRST
				    | TKB_PACKET_FLAG_HASHAPPEND);
	if (fFinalize)
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

	rc = PEC_Packet_Put(PEC_INTERFACE_ID, &Cmd, 1, &count);
	if (rc != PEC_STATUS_OK && count != 1) {
		rc = 1;
		CRYPTO_ERR("PEC_Packet_Put error: %d\n", rc);
		goto error_exit_unregister;
	}

	if (crypto_pe_get_one(&OutTokenDscr, OutputToken, &Res) < 1) {
		rc = 1;
		CRYPTO_ERR("error from crypto_pe_get_one\n");
		goto error_exit_unregister;
	}
	memcpy(Output_p, PktHostAddress.p, OutputByteCount);

error_exit_unregister:
	PEC_SA_UnRegister(PEC_INTERFACE_ID, SAHandle, DMABuf_NULLHandle,
				DMABuf_NULLHandle);

error_exit:
	DMABuf_Release(SAHandle);
	DMABuf_Release(TokenHandle);
	DMABuf_Release(PktHandle);

	if (TCRData != NULL)
		kfree(TCRData);

	return rc == 0;
}

bool crypto_hmac_precompute(SABuilder_Auth_t AuthAlgo,
			    uint8_t *AuthKey_p,
			    unsigned int AuthKeyByteCount,
			    uint8_t *Inner_p,
			    uint8_t *Outer_p)
{
	SABuilder_Auth_t HashAlgo;
	unsigned int blocksize, hashsize, digestsize;
	static uint8_t pad_block[128], hashed_key[128];
	unsigned int i;

	switch (AuthAlgo) {
	case SAB_AUTH_HMAC_MD5:
		HashAlgo = SAB_AUTH_HASH_MD5;
		blocksize = 64;
		hashsize = 16;
		digestsize = 16;
		break;
	case SAB_AUTH_HMAC_SHA1:
		HashAlgo = SAB_AUTH_HASH_SHA1;
		blocksize = 64;
		hashsize = 20;
		digestsize = 20;
		break;
	case SAB_AUTH_HMAC_SHA2_224:
		HashAlgo = SAB_AUTH_HASH_SHA2_224;
		blocksize = 64;
		hashsize = 28;
		digestsize = 32;
		break;
	case SAB_AUTH_HMAC_SHA2_256:
		HashAlgo = SAB_AUTH_HASH_SHA2_256;
		blocksize = 64;
		hashsize = 32;
		digestsize = 32;
		break;
	case SAB_AUTH_HMAC_SHA2_384:
		HashAlgo = SAB_AUTH_HASH_SHA2_384;
		blocksize = 128;
		hashsize = 48;
		digestsize = 64;
		break;
	case SAB_AUTH_HMAC_SHA2_512:
		HashAlgo = SAB_AUTH_HASH_SHA2_512;
		blocksize = 128;
		hashsize = 64;
		digestsize = 64;
		break;
	default:
		CRYPTO_ERR("Unknown HMAC algorithm\n");
		return false;
	}

	memset(hashed_key, 0, blocksize);
	if (AuthKeyByteCount <= blocksize) {
		memcpy(hashed_key, AuthKey_p, AuthKeyByteCount);
	} else {
		if (!crypto_basic_hash(HashAlgo, AuthKey_p, AuthKeyByteCount,
				       hashed_key, hashsize, true))
			return false;
	}

	for (i = 0; i < blocksize; i++)
		pad_block[i] = hashed_key[i] ^ 0x36;

	if (!crypto_basic_hash(HashAlgo, pad_block, blocksize,
			       Inner_p, digestsize, false))
		return false;

	for (i = 0; i < blocksize; i++)
		pad_block[i] = hashed_key[i] ^ 0x5c;

	if (!crypto_basic_hash(HashAlgo, pad_block, blocksize,
			       Outer_p, digestsize, false))
		return false;

	return true;
}

static SABuilder_Crypto_t set_crypto_algo(struct xfrm_algo *ealg)
{
	if (strcmp(ealg->alg_name, "cbc(des)") == 0)
		return SAB_CRYPTO_DES;
	else if (strcmp(ealg->alg_name, "cbc(aes)") == 0)
		return SAB_CRYPTO_AES;
	else if (strcmp(ealg->alg_name, "cbc(des3_ede)") == 0)
		return SAB_CRYPTO_3DES;

	return SAB_CRYPTO_NULL;
}

static bool set_auth_algo(struct xfrm_algo_auth *aalg, SABuilder_Params_t *params,
			  uint8_t *inner, uint8_t *outer)
{
	if (strcmp(aalg->alg_name, "hmac(sha1)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA1;
		inner = kcalloc(SHA1_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA1_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA1, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);

		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
	} else if (strcmp(aalg->alg_name, "hmac(sha256)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_256;
		inner = kcalloc(SHA256_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA256_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA2_256, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
	} else if (strcmp(aalg->alg_name, "hmac(sha384)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_384;
		inner = kcalloc(SHA384_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA384_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA2_384, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
	} else if (strcmp(aalg->alg_name, "hmac(sha512)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_SHA2_512;
		inner = kcalloc(SHA512_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(SHA512_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_SHA2_512, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
	} else if (strcmp(aalg->alg_name, "hmac(md5)") == 0) {
		params->AuthAlgo = SAB_AUTH_HMAC_MD5;
		inner = kcalloc(MD5_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		outer = kcalloc(MD5_DIGEST_SIZE, sizeof(uint8_t), GFP_KERNEL);
		crypto_hmac_precompute(SAB_AUTH_HMAC_MD5, &aalg->alg_key[0],
					aalg->alg_key_len / 8, inner, outer);
		params->AuthKey1_p = inner;
		params->AuthKey2_p = outer;
	} else {
		return false;
	}

	return true;
}

u32 *mtk_ddk_tr_ipsec_build(struct mtk_xfrm_params *xfrm_params, u32 ipsec_mode)
{
	struct xfrm_state *xs = xfrm_params->xs;
	SABuilder_Params_IPsec_t ipsec_params;
	SABuilder_Status_t sa_status;
	SABuilder_Params_t params;
	bool set_auth_success = false;
	unsigned int SAWords = 0;
	uint8_t *inner, *outer;

	DMABuf_Status_t dma_status;
	DMABuf_Properties_t dma_properties = {0, 0, 0, 0};
	DMABuf_HostAddress_t sa_host_addr;

	DMABuf_Handle_t sa_handle = {0};

	sa_status = SABuilder_Init_ESP(&params,
				       &ipsec_params,
				       be32_to_cpu(xs->id.spi),
				       ipsec_mode,
				       SAB_IPSEC_IPV4,
				       xfrm_params->dir);

	if (sa_status != SAB_STATUS_OK) {
		pr_err("SABuilder_Init_ESP failed\n");
		sa_handle.p = NULL;
		return (u32 *) sa_handle.p;
	}

	/* No support for aead now */
	if (xs->aead) {
		CRYPTO_ERR("AEAD not supported\n");
		sa_handle.p = NULL;
		return (u32 *) sa_handle.p;
	}

	/* Add crypto key and parameters */
	params.CryptoAlgo = set_crypto_algo(xs->ealg);
	params.CryptoMode = SAB_CRYPTO_MODE_CBC;
	params.KeyByteCount = xs->ealg->alg_key_len / 8;
	params.Key_p = xs->ealg->alg_key;

	/* Add authentication key and parameters */
	set_auth_success = set_auth_algo(xs->aalg, &params, inner, outer);
	if (set_auth_success != true) {
		CRYPTO_ERR("Set Auth Algo failed\n");
		sa_handle.p = NULL;
		return (u32 *) sa_handle.p;
	}

	ipsec_params.IPsecFlags |= (SAB_IPSEC_PROCESS_IP_HEADERS
				    | SAB_IPSEC_EXT_PROCESSING);
	if (ipsec_mode == SAB_IPSEC_TUNNEL) {
		ipsec_params.SrcIPAddr_p = (uint8_t *) &xs->props.saddr.a4;
		ipsec_params.DestIPAddr_p = (uint8_t *) &xs->id.daddr.a4;
	}

	sa_status = SABuilder_GetSizes(&params, &SAWords, NULL, NULL);
	if (sa_status != SAB_STATUS_OK) {
		CRYPTO_ERR("SA not created because of size errors\n");
		sa_handle.p = NULL;
		return (u32 *) sa_handle.p;
	}

	dma_properties.fCached = true;
	dma_properties.Alignment = MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT;
	dma_properties.Bank = MTK_EIP197_INLINE_BANK_TRANSFORM;
	dma_properties.Size = SAWords * sizeof(u32);

	dma_status = DMABuf_Alloc(dma_properties, &sa_host_addr, &sa_handle);
	if (dma_status != DMABUF_STATUS_OK) {
		CRYPTO_ERR("Allocation of SA failed\n");
		/* goto error_exit; */
		sa_handle.p = NULL;
		return (u32 *) sa_handle.p;
	}

	sa_status = SABuilder_BuildSA(&params, (u32 *) sa_host_addr.p, NULL, NULL);
	if (sa_status != SAB_STATUS_OK) {
		CRYPTO_ERR("SA not created because of errors\n");
		sa_handle.p = NULL;
		return (u32 *) sa_handle.p;
	}

	kfree(inner);
	kfree(outer);
	return (u32 *) sa_host_addr.p;
}

int mtk_ddk_pec_init(void)
{
	PEC_InitBlock_t pec_init_blk = {0, 0, false};
	PEC_Capabilities_t pec_cap;
	PEC_Status_t pec_sta;
	u32 i = MTK_EIP197_INLINE_NOF_TRIES;

	while (i) {
		pec_sta = PEC_Init(PEC_INTERFACE_ID, &pec_init_blk);
		if (pec_sta == PEC_STATUS_OK) {
			CRYPTO_INFO("PEC_INIT ok!\n");
			break;
		} else if (pec_sta != PEC_STATUS_OK && pec_sta != PEC_STATUS_BUSY) {
			return pec_sta;
		}

		mdelay(MTK_EIP197_INLINE_RETRY_DELAY_MS);
		i--;
	}

	if (!i) {
		CRYPTO_ERR("PEC could not be initialized: %d\n", pec_sta);
		return pec_sta;
	}

	pec_sta = PEC_Capabilities_Get(&pec_cap);
	if (pec_sta != PEC_STATUS_OK) {
		CRYPTO_ERR("PEC capability could not be obtained: %d\n", pec_sta);
		return pec_sta;
	}

	CRYPTO_INFO("PEC Capabilities: %s\n", pec_cap.szTextDescription);

	return 0;
}

void mtk_ddk_pec_deinit(void)
{
}
