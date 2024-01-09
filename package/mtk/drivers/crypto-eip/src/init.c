// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <mtk_eth_soc.h>
#include <mtk_hnat/hnat.h>

#include <crypto-eip/ddk/configs/cs_hwpal_ext.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/internal.h"

#define DRIVER_AUTHOR	"Ren-Ting Wang <ren-ting.wang@mediatek.com, " \
			"Chris.Chou <chris.chou@mediatek.com"

struct mtk_crypto mcrypto;
struct device *crypto_dev;

inline void crypto_eth_write(u32 reg, u32 val)
{
	writel(val, mcrypto.eth_base + reg);
}

static inline void crypto_eip_write(u32 reg, u32 val)
{
	writel(val, mcrypto.crypto_base + reg);
}

static inline void crypto_eip_set(u32 reg, u32 mask)
{
	setbits(mcrypto.crypto_base + reg, mask);
}

static inline void crypto_eip_clr(u32 reg, u32 mask)
{
	clrbits(mcrypto.crypto_base + reg, mask);
}

static inline void crypto_eip_rmw(u32 reg, u32 mask, u32 val)
{
	clrsetbits(mcrypto.crypto_base + reg, mask, val);
}

static inline u32 crypto_eip_read(u32 reg)
{
	return readl(mcrypto.crypto_base + reg);
}

static bool mtk_crypto_eip_offloadable(struct sk_buff *skb)
{
	/* TODO: check is esp */
	return true;
}

static const struct xfrmdev_ops mtk_xfrmdev_ops = {
	.xdo_dev_state_add = mtk_xfrm_offload_state_add,
	.xdo_dev_state_delete = mtk_xfrm_offload_state_delete,
	.xdo_dev_state_free = mtk_xfrm_offload_state_free,
	.xdo_dev_offload_ok = mtk_xfrm_offload_ok,

	/* Not support at v5.4*/
	.xdo_dev_policy_add = mtk_xfrm_offload_policy_add,
};

static void mtk_crypto_xfrm_offload_deinit(struct mtk_eth *eth)
{
	int i;

	mtk_crypto_offloadable = NULL;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		eth->netdev[i]->xfrmdev_ops = NULL;
		eth->netdev[i]->features &= (~NETIF_F_HW_ESP);
		eth->netdev[i]->hw_enc_features &= (~NETIF_F_HW_ESP);
		rtnl_lock();
		netdev_change_features(eth->netdev[i]);
		rtnl_unlock();
	}
}

static void mtk_crypto_xfrm_offload_init(struct mtk_eth *eth)
{
	int i;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		eth->netdev[i]->xfrmdev_ops = &mtk_xfrmdev_ops;
		eth->netdev[i]->features |= NETIF_F_HW_ESP;
		eth->netdev[i]->hw_enc_features |= NETIF_F_HW_ESP;
		rtnl_lock();
		netdev_change_features(eth->netdev[i]);
		rtnl_unlock();
	}

	mtk_crypto_offloadable = mtk_crypto_eip_offloadable;
}

static int __init mtk_crypto_eth_dts_init(struct platform_device *pdev)
{
	struct platform_device *eth_pdev;
	struct device_node *crypto_node;
	struct device_node *eth_node;
	struct resource res;
	int ret = 0;

	crypto_node = pdev->dev.of_node;

	eth_node = of_parse_phandle(crypto_node, "eth", 0);
	if (!eth_node)
		return -ENODEV;

	eth_pdev = of_find_device_by_node(eth_node);
	if (!eth_pdev) {
		ret = -ENODEV;
		goto out;
	}

	if (!eth_pdev->dev.driver) {
		ret = -EFAULT;
		goto out;
	}

	if (of_address_to_resource(eth_node, 0, &res)) {
		ret = -ENXIO;
		goto out;
	}

	mcrypto.eth_base = devm_ioremap(&pdev->dev,
					res.start, resource_size(&res));
	if (!mcrypto.eth_base) {
		ret = -ENOMEM;
		goto out;
	}

	mcrypto.eth = platform_get_drvdata(eth_pdev);

out:
	of_node_put(eth_node);

	return ret;
}

static int __init mtk_crypto_eip_dts_init(void)
{
	struct platform_device *crypto_pdev;
	struct device_node *crypto_node;
	struct resource res;
	int ret;

	crypto_node = of_find_compatible_node(NULL, NULL, HWPAL_PLATFORM_DEVICE_NAME);
	if (!crypto_node)
		return -ENODEV;

	crypto_pdev = of_find_device_by_node(crypto_node);
	if (!crypto_pdev) {
		ret = -ENODEV;
		goto out;
	}

	/* check crypto platform device is ready */
	if (!crypto_pdev->dev.driver) {
		ret = -EFAULT;
		goto out;
	}

	if (of_address_to_resource(crypto_node, 0, &res)) {
		ret = -ENXIO;
		goto out;
	}

	mcrypto.crypto_base = devm_ioremap(&crypto_pdev->dev,
					   res.start, resource_size(&res));
	if (!mcrypto.crypto_base) {
		ret = -ENOMEM;
		goto out;
	}

	ret = mtk_crypto_eth_dts_init(crypto_pdev);
	if (ret)
		goto out;

	crypto_dev = &crypto_pdev->dev;

out:
	of_node_put(crypto_node);

	return ret;
}

static int __init mtk_crypto_eip_hw_init(void)
{
	crypto_eip_write(EIP197_FORCE_CLK_ON, 0xffffffff);

	crypto_eip_write(EIP197_FORCE_CLK_ON2, 0xffffffff);

	/* TODO: adjust AXI burst? */

	mtk_ddk_pec_init();

	return 0;
}

static void __exit mtk_crypto_eip_hw_deinit(void)
{
	mtk_ddk_pec_deinit();

	crypto_eip_write(EIP197_FORCE_CLK_ON, 0);

	crypto_eip_write(EIP197_FORCE_CLK_ON2, 0);
}

static int __init mtk_crypto_eip_init(void)
{
	int ret;

	ret = mtk_crypto_eip_dts_init();
	if (ret) {
		CRYPTO_ERR("crypto-eip dts init failed: %d\n", ret);
		return ret;
	}

	ret = mtk_crypto_eip_hw_init();
	if (ret) {
		CRYPTO_ERR("crypto-eip hw init failed: %d\n", ret);
		return ret;
	}

	mtk_crypto_xfrm_offload_init(mcrypto.eth);

	CRYPTO_INFO("crypto-eip init done\n");

	return ret;
}

static void __exit mtk_crypto_eip_exit(void)
{
	/* TODO: deactivate all tunnel */

	mtk_crypto_xfrm_offload_deinit(mcrypto.eth);

	mtk_crypto_eip_hw_deinit();
}

module_init(mtk_crypto_eip_init);
module_exit(mtk_crypto_eip_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek Crypto EIP Control Driver");
MODULE_AUTHOR(DRIVER_AUTHOR);
