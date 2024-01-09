/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _CRYPTO_EIP_DDK_WRAPPER_H_
#define _CRYPTO_EIP_DDK_WRAPPER_H_

#include "crypto-eip.h"

u32 *mtk_ddk_tr_ipsec_build(struct mtk_xfrm_params *xfrm_params, u32 ipsec_mod);

int mtk_ddk_pec_init(void);
void mtk_ddk_pec_deinit(void);
#endif /* _CRYPTO_EIP_DDK_WRAPPER_H_ */
