/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _CRYPTO_EIP_INTERNAL_H_
#define _CRYPTO_EIP_INTERNAL_H_

#include <linux/device.h>

extern struct device *crypto_dev;

#define CRYPTO_DBG(fmt, ...)		dev_dbg(crypto_dev, fmt, ##__VA_ARGS__)
#define CRYPTO_INFO(fmt, ...)		dev_info(crypto_dev, fmt, ##__VA_ARGS__)
#define CRYPTO_NOTICE(fmt, ...)		dev_notice(crypto_dev, fmt, ##__VA_ARGS__)
#define CRYPTO_WARN(fmt, ...)		dev_warn(crypto_dev, fmt, ##__VA_ARGS__)
#define CRYPTO_ERR(fmt, ...)		dev_err(crypto_dev, fmt, ##__VA_ARGS__)

#define setbits(addr, set)		writel(readl(addr) | (set), (addr))
#define clrbits(addr, clr)		writel(readl(addr) & ~(clr), (addr))
#define clrsetbits(addr, clr, set)	writel((readl(addr) & ~(clr)) | (set), (addr))
#endif /* _CRYPTO_EIP_INTERNAL_H_ */
