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
#include <linux/crypto.h>
#include <linux/platform_device.h>
#include <crypto/internal/skcipher.h>
#include <crypto/internal/aead.h>
#include <crypto/internal/hash.h>

#include <mtk_eth_soc.h>

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
#include <mtk_hnat/hnat.h>
#endif

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"

#if defined(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE)
#include "crypto-eip/lookaside.h"
#endif

#include "crypto-eip/internal.h"
#include "crypto-eip/debugfs.h"

#define DRIVER_AUTHOR	"Ren-Ting Wang <ren-ting.wang@mediatek.com, " \
			"Chris.Chou <chris.chou@mediatek.com"

struct mtk_crypto mcrypto;
struct device *crypto_dev;
struct mtk_crypto_priv *priv;

#if defined(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE)
static struct mtk_crypto_alg_template *mtk_crypto_algs[] = {
	&mtk_crypto_cbc_aes,
	&mtk_crypto_ecb_aes,
	&mtk_crypto_cfb_aes,
	&mtk_crypto_ofb_aes,
	&mtk_crypto_ctr_aes,
	&mtk_crypto_cbc_des,
	&mtk_crypto_ecb_des,
	&mtk_crypto_cbc_des3_ede,
	&mtk_crypto_ecb_des3_ede,
	&mtk_crypto_sha1,
	&mtk_crypto_hmac_sha1,
	&mtk_crypto_sha224,
	&mtk_crypto_hmac_sha224,
	&mtk_crypto_sha256,
	&mtk_crypto_hmac_sha256,
	&mtk_crypto_sha384,
	&mtk_crypto_hmac_sha384,
	&mtk_crypto_sha512,
	&mtk_crypto_hmac_sha512,
	&mtk_crypto_md5,
	&mtk_crypto_hmac_md5,
	&mtk_crypto_xcbcmac,
	&mtk_crypto_cmac,
	&mtk_crypto_hmac_sha1_cbc_aes,
	&mtk_crypto_hmac_sha224_cbc_aes,
	&mtk_crypto_hmac_sha256_cbc_aes,
	&mtk_crypto_hmac_sha384_cbc_aes,
	&mtk_crypto_hmac_sha512_cbc_aes,
	&mtk_crypto_hmac_md5_cbc_aes,
	&mtk_crypto_hmac_sha1_cbc_des3_ede,
	&mtk_crypto_hmac_sha224_cbc_des3_ede,
	&mtk_crypto_hmac_sha256_cbc_des3_ede,
	&mtk_crypto_hmac_sha384_cbc_des3_ede,
	&mtk_crypto_hmac_sha512_cbc_des3_ede,
	&mtk_crypto_hmac_md5_cbc_des3_ede,
	&mtk_crypto_hmac_sha1_cbc_des,
	&mtk_crypto_hmac_sha224_cbc_des,
	&mtk_crypto_hmac_sha256_cbc_des,
	&mtk_crypto_hmac_sha384_cbc_des,
	&mtk_crypto_hmac_sha512_cbc_des,
	//&mtk_crypto_hmac_sha1_ctr_aes, /* no testcase, todo */
	//&mtk_crypto_hmac_sha256_ctr_aes, /* no testcase, todo */
	&mtk_crypto_gcm,
	&mtk_crypto_rfc4106_gcm,
	&mtk_crypto_rfc4543_gcm,
	&mtk_crypto_rfc4309_ccm,
};

static int mtk_crypto_register_algorithms(struct mtk_crypto_priv *priv)
{
	int i;
	int j;
	int ret;

	for (i = 0; i < ARRAY_SIZE(mtk_crypto_algs); i++) {
		mtk_crypto_algs[i]->priv = priv;

		if (mtk_crypto_algs[i]->type == MTK_CRYPTO_ALG_TYPE_SKCIPHER)
			ret = crypto_register_skcipher(&mtk_crypto_algs[i]->alg.skcipher);
		else if (mtk_crypto_algs[i]->type == MTK_CRYPTO_ALG_TYPE_AEAD)
			ret = crypto_register_aead(&mtk_crypto_algs[i]->alg.aead);
		else
			ret = crypto_register_ahash(&mtk_crypto_algs[i]->alg.ahash);

		if (ret)
			goto fail;
	}

	return 0;

fail:
	for (j = 0; j < i; j++) {
		if (mtk_crypto_algs[j]->type == MTK_CRYPTO_ALG_TYPE_SKCIPHER)
			crypto_unregister_skcipher(&mtk_crypto_algs[j]->alg.skcipher);
		else if (mtk_crypto_algs[j]->type == MTK_CRYPTO_ALG_TYPE_AEAD)
			crypto_unregister_aead(&mtk_crypto_algs[j]->alg.aead);
		else
			crypto_unregister_ahash(&mtk_crypto_algs[j]->alg.ahash);
	}

	return ret;
}

static void mtk_crypto_unregister_algorithms(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mtk_crypto_algs); i++) {
		if (mtk_crypto_algs[i]->type == MTK_CRYPTO_ALG_TYPE_SKCIPHER)
			crypto_unregister_skcipher(&mtk_crypto_algs[i]->alg.skcipher);
		else if (mtk_crypto_algs[i]->type == MTK_CRYPTO_ALG_TYPE_AEAD)
			crypto_unregister_aead(&mtk_crypto_algs[i]->alg.aead);
		else
			crypto_unregister_ahash(&mtk_crypto_algs[i]->alg.ahash);
	}
}

static int __init mtk_crypto_lookaside_data_init(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int i;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);

	priv->mtk_eip_ring = devm_kcalloc(dev, PEC_MAX_INTERFACE_NUM,
							sizeof(*priv->mtk_eip_ring), GFP_KERNEL);
	if (!priv->mtk_eip_ring)
		return -ENOMEM;

	for (i = 0; i < PEC_MAX_INTERFACE_NUM; i++) {
		char wq_name[17] = {0};
		char irq_name[6] = {0};
		int irq, cpu;

		// init workqueue for all rings
		priv->mtk_eip_ring[i].work_data.priv = priv;
		priv->mtk_eip_ring[i].work_data.ring = i;
		INIT_WORK(&priv->mtk_eip_ring[i].work_data.work, mtk_crypto_dequeue_work);

		if (snprintf(wq_name, 17, "mtk_crypto_work%d", i) >= 17)
			return -EINVAL;
		priv->mtk_eip_ring[i].workqueue = create_singlethread_workqueue(wq_name);
		if (!priv->mtk_eip_ring[i].workqueue)
			return -ENOMEM;

		crypto_init_queue(&priv->mtk_eip_ring[i].queue, EIP197_DEFAULT_RING_SIZE);
		INIT_LIST_HEAD(&priv->mtk_eip_ring[i].list);

		spin_lock_init(&priv->mtk_eip_ring[i].ring_lock);
		spin_lock_init(&priv->mtk_eip_ring[i].queue_lock);

		// setup irq affinity
		if (snprintf(irq_name, 6, "ring%d", i) >= 6)
			return -EINVAL;
		irq = platform_get_irq_byname(pdev,  irq_name);
		if (irq < 0)
			return irq;

		cpu = cpumask_local_spread(i, NUMA_NO_NODE);
		irq_set_affinity_hint(irq, get_cpu_mask(cpu));
	}

	return 0;
};

#endif /* CONFIG_CRYPTO_MTK_DDK_LOOKASIDE */

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

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
static bool mtk_crypto_eip_offloadable(struct sk_buff *skb)
{
	/* TODO: check is esp */
	return true;
}
#endif // HNAT

u32 mtk_crypto_ppe_get_num(void)
{
	return mcrypto.ppe_num;
}

static const struct xfrmdev_ops mtk_xfrmdev_ops = {
	.xdo_dev_state_add = mtk_xfrm_offload_state_add,
	.xdo_dev_state_delete = mtk_xfrm_offload_state_delete,
	.xdo_dev_state_free = mtk_xfrm_offload_state_free,
	.xdo_dev_offload_ok = mtk_xfrm_offload_ok,

	/* Not support at v5.4*/
	.xdo_dev_policy_add = mtk_xfrm_offload_policy_add,
	.xdo_dev_policy_delete = mtk_xfrm_offload_policy_delete,
	.xdo_dev_policy_free = mtk_xfrm_offload_policy_free,
};

void mtk_crypto_enable_ipsec_dev_features(struct net_device *dev)
{
	dev->xfrmdev_ops = &mtk_xfrmdev_ops;
	dev->features |= NETIF_F_HW_ESP;
	dev->hw_enc_features |= NETIF_F_HW_ESP;

	rtnl_lock();
	netdev_change_features(dev);
	rtnl_unlock();

	if (dev->features & NETIF_F_HW_ESP)
		CRYPTO_INFO("dev: %s, enable hw offload ipsec success\n", dev->name);
	else
		CRYPTO_INFO("dev: %s, enable hw offload ipsec failed\n", dev->name);
}

void mtk_crypto_disable_ipsec_dev_features(struct net_device *dev)
{
	dev->xfrmdev_ops = NULL;
	dev->features &= (~NETIF_F_HW_ESP);
	dev->hw_enc_features &= (~NETIF_F_HW_ESP);

	rtnl_lock();
	netdev_change_features(dev);
	rtnl_unlock();
}


static void mtk_crypto_xfrm_offload_deinit(struct mtk_eth *eth)
{
	struct net_device *dev;

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
	mtk_crypto_offloadable = NULL;
#endif // HNAT

	for_each_netdev(&init_net, dev)
		mtk_crypto_disable_ipsec_dev_features(dev);
}

static void mtk_crypto_xfrm_offload_init(struct mtk_eth *eth)
{
	struct net_device *dev;

	for_each_netdev(&init_net, dev)
		mtk_crypto_enable_ipsec_dev_features(dev);

#if IS_ENABLED(CONFIG_NET_MEDIATEK_HNAT)
	mtk_crypto_offloadable = mtk_crypto_eip_offloadable;
#endif // HNAT
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

static int __init mtk_crypto_ppe_num_dts_init(struct platform_device *pdev)
{
	struct device_node *hnat = NULL;
	u32 val = 0;
	int ret = 0;

	hnat = of_parse_phandle(pdev->dev.of_node, "hnat", 0);
	if (!hnat) {
		mcrypto.ppe_num = 1;
		return 0;
	}

	ret = of_property_read_u32(hnat, "mtketh-ppe-num", &val);
	if (ret)
		mcrypto.ppe_num = 1;
	else
		mcrypto.ppe_num = val;

	of_node_put(hnat);

	return 0;
}

static int __init mtk_crypto_eip_dts_init(void)
{
	struct platform_device *crypto_pdev;
	struct device_node *crypto_node;
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

	crypto_dev = &crypto_pdev->dev;

	mcrypto.crypto_base = devm_platform_ioremap_resource(crypto_pdev, 0);
	if (IS_ERR(mcrypto.crypto_base)) {
		CRYPTO_ERR("Failed to get resource, please remove all ddk drivers\n");
		ret = -ENOMEM;
		goto out;
	}

	ret = mtk_crypto_eth_dts_init(crypto_pdev);
	if (ret)
		goto out;

	ret = mtk_crypto_ppe_num_dts_init(crypto_pdev);
	if (ret)
		goto out;


#if defined(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE)
	ret = mtk_crypto_lookaside_data_init(crypto_pdev);
	if (ret)
		goto out;
#endif /* CONFIG_CRYPTO_MTK_DDK_LOOKASIDE */

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

	ret = mtk_crypto_register_nf_hooks();
	if (ret)
		CRYPTO_ERR("crypto-eip register hook failed: %d\n", ret);

	mtk_crypto_xfrm_offload_init(mcrypto.eth);
	mtk_crypto_debugfs_init();

#if defined(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE)
	mtk_crypto_register_algorithms(priv);
#endif /* CONFIG_CRYPTO_MTK_DDK_LOOKASIDE */

#if defined(CONFIG_MTK_TOPS_CAPWAP_DTLS)
	mtk_dtls_capwap_init();
#endif

	CRYPTO_INFO("crypto-eip init done\n");

	return ret;
}

static void __exit mtk_crypto_eip_exit(void)
{
	/* TODO: deactivate all tunnel */
#if defined(CONFIG_MTK_TOPS_CAPWAP_DTLS)
	mtk_dtls_capwap_deinit();
#endif

#if defined(CONFIG_CRYPTO_MTK_DDK_LOOKASIDE)
	mtk_crypto_unregister_algorithms();
#endif /* CONFIG_CRYPTO_MTK_DDK_LOOKASIDE */

	mtk_crypto_debugfs_exit();
	mtk_crypto_xfrm_offload_deinit(mcrypto.eth);

	mtk_crypto_unregister_nf_hooks();
	mtk_crypto_eip_hw_deinit();

}

module_init(mtk_crypto_eip_init);
module_exit(mtk_crypto_eip_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek Crypto EIP Control Driver");
MODULE_AUTHOR(DRIVER_AUTHOR);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
MODULE_IMPORT_NS(CRYPTO_INTERNAL);
#endif
