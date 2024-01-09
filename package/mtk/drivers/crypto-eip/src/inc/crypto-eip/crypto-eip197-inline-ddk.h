/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _CRYPTO_EIP197_INLINE_DDK_H_
#define _CRYPTO_EIP197_INLINE_DDK_H_

#include <crypto-eip/ddk/basic_defs.h>
#include <crypto-eip/ddk/configs/cs_ddk197.h>
#include <crypto-eip/ddk/configs/cs_driver_ext.h>
#include <crypto-eip/ddk/configs/cs_adapter.h>
#include <crypto-eip/ddk/libc/clib.h>
#include <crypto-eip/ddk/kit/builder/sa/sa_builder.h>
#include <crypto-eip/ddk/kit/builder/sa/sa_builder_ipsec.h>
#include <crypto-eip/ddk/kit/builder/sa/sa_builder_basic.h>
#include <crypto-eip/ddk/kit/builder/sa/sa_builder_params_basic.h>
#include <crypto-eip/ddk/kit/builder/token/token_builder.h>
#include <crypto-eip/ddk/kit/iotoken/iotoken.h>
#include <crypto-eip/ddk/kit/iotoken/iotoken_ext.h>
#include <crypto-eip/ddk/slad/api_dmabuf.h>
#include <crypto-eip/ddk/slad/api_pcl.h>
#include <crypto-eip/ddk/slad/api_pcl_dtl.h>
#include <crypto-eip/ddk/slad/api_pec.h>
#include <crypto-eip/ddk/log/log.h>

#ifdef DDK_EIP197_FW33_FEATURES
#define MTK_EIP197_INLINE_STRIP_PADDING
#endif

#define CRYPTO_IOTOKEN_EXT

#define MTK_EIP197_INLINE_BANK_PACKET			0

#define MTK_EIP197_INLINE_BANK_TRANSFORM		1
#define MTK_EIP197_INLINE_BANK_TOKEN			0

/* Delay in milliseconds between tries to receive the packet */
#define MTK_EIP197_INLINE_RETRY_DELAY_MS		2
#define MTK_EIP197_PKT_GET_TIMEOUT_MS			2

/* Maximum number of tries to receive the packet. */
#define MTK_EIP197_INLINE_NOF_TRIES			10

#define MTK_EIP197_INLINE_DMA_ALIGNMENT_BYTE_COUNT	16

/* PEC Configuration */
#ifdef DDK_PEC_IF_ID
#define PEC_INTERFACE_ID				DDK_PEC_IF_ID
#else
#define PEC_INTERFACE_ID				0
#endif
#define PEC_REDIRECT_INTERFACE				7
#define PEC_INLINE_INTERFACE				15

#define PCL_INTERFACE_ID				0
#endif /* _CRYPTO_EIP197_INLINE_DDK_H_ */
