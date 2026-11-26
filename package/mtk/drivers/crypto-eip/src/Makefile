# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 MediaTek Inc.
#
# Author: Chris.Chou <chris.chou@mediatek.com>
#         Ren-Ting Wang <ren-ting.wang@mediatek.com>

obj-$(CONFIG_MTK_CRYPTO_EIP_INLINE) += crypto-eip-inline.o

ccflags-y += -I$(src)/inc

crypto-eip-inline-y += init.o
crypto-eip-inline-y += ddk-wrapper.o
crypto-eip-inline-y += debugfs.o
crypto-eip-inline-y += crypto_hook.o

crypto-eip-inline-$(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE) += lookaside.o
crypto-eip-inline-$(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE) += lookaside-cipher.o
crypto-eip-inline-$(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE) += lookaside-hash.o

crypto-eip-inline-$(CONFIG_CRYPTO_XFRM_OFFLOAD_MTK_PCE) += xfrm-offload.o
crypto-eip-inline-$(CONFIG_MTK_TOPS_CAPWAP_DTLS) += capwap-dtls-offload.o

include $(wildcard $(src)/*.mk)
