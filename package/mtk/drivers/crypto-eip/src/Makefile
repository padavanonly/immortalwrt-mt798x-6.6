# SPDX-Liscense-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2023 MediaTek Inc.
#
# Author: Chris.Chou <chris.chou@mediatek.com>
#         Ren-Ting Wang <ren-ting.wang@mediatek.com>

obj-$(CONFIG_MTK_CRYPTO_EIP_INLINE) += crypto-eip-inline.o

ccflags-y += -I$(src)/inc

crypto-eip-inline-y += init.o
crypto-eip-inline-y += ddk-wrapper.o

crypto-eip-inline-$(CONFIG_CRYPTO_XFRM_OFFLOAD_MTK_PCE) += xfrm-offload.o

include $(wildcard $(src)/*.mk)
