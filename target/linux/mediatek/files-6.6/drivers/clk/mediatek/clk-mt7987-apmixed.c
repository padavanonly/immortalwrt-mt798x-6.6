// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 MediaTek Inc.
 * Author: Lu Tang <Lu.Tang@mediatek.com>
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include "clk-mtk.h"
#include "clk-gate.h"
#include "clk-mux.h"
#include "clk-pll.h"
#include <dt-bindings/clock/mediatek,mt7987-clk.h>

#define MT7987_PLL_FMAX (2500UL * MHZ)
#define MT7987_PCW_CHG_SHIFT 2

#define PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,     \
	      _pcwbits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,         \
	      _tuner_en_bit, _pcw_reg, _pcw_shift, _pcw_chg_reg, _div_table,   \
	      _parent_name)                                                    \
	{                                                                      \
		.id = _id, .name = _name, .reg = _reg, .pwr_reg = _pwr_reg,    \
		.en_mask = _en_mask, .flags = _flags,                          \
		.rst_bar_mask = BIT(_rst_bar_mask), .fmax = MT7987_PLL_FMAX,   \
		.pcwbits = _pcwbits, .pd_reg = _pd_reg, .pd_shift = _pd_shift, \
		.tuner_reg = _tuner_reg, .tuner_en_reg = _tuner_en_reg,        \
		.tuner_en_bit = _tuner_en_bit, .pcw_reg = _pcw_reg,            \
		.pcw_shift = _pcw_shift, .pcw_chg_reg = _pcw_chg_reg,          \
		.pcw_chg_bit = MT7987_PCW_CHG_SHIFT,                           \
		.div_table = _div_table, .parent_name = _parent_name,          \
	}

#define PLL(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,     \
	    _pcwbits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,         \
	    _tuner_en_bit, _pcw_reg, _pcw_shift, _pcw_chg_reg, _parent_name) \
	PLL_B(_id, _name, _reg, _pwr_reg, _en_mask, _flags, _rst_bar_mask,   \
	      _pcwbits, _pd_reg, _pd_shift, _tuner_reg, _tuner_en_reg,       \
	      _tuner_en_bit, _pcw_reg, _pcw_shift, _pcw_chg_reg, NULL,       \
	      _parent_name)

static const struct mtk_pll_div_table mt7987_arm_ll_div[] = {
	{ .div = 0, .freq = 2000000000 },
	{ .div = 1, .freq = 1500000000 },
	{ .div = 2, .freq = 750000000 },
	{ .div = 3, .freq = 375000000 },
	{} /* sentinel */
};

static const struct mtk_pll_data plls[] = {
	PLL(CLK_APMIXED_MPLL, "mpll", 0x0114, 0x0120, 0xff000001, HAVE_RST_BAR,
	    23, 32, 0x0114, 4, 0, 0, 0, 0x0118, 0, 0x0114, "clkxtal"),
	PLL(CLK_APMIXED_APLL2, "apll2", 0x0134, 0x0140, 0x00000001, 0, 0, 32,
	    0x0134, 4, 0x0704, 0x0700, 1, 0x0138, 0, 0x0134, "clkxtal"),
	PLL(CLK_APMIXED_NET1PLL, "net1pll", 0x0144, 0x0150, 0xff000001,
	    HAVE_RST_BAR | PLL_AO, 23, 32, 0x0144, 4, 0, 0, 0, 0x0148, 0,
	    0x0144, "clkxtal"),
	PLL(CLK_APMIXED_NET2PLL, "net2pll", 0x0154, 0x0160, 0xff000001,
	    HAVE_RST_BAR | PLL_AO, 23, 32, 0x0154, 4, 0, 0, 0, 0x0158, 0,
	    0x0154, "clkxtal"),
	PLL(CLK_APMIXED_WEDMCUPLL, "wedmcupll", 0x0164, 0x0170, 0x00000001, 0,
	    0, 32, 0x0164, 4, 0, 0, 0, 0x0168, 0, 0x0164, "clkxtal"),
	PLL(CLK_APMIXED_SGMPLL, "sgmpll", 0x0174, 0x0180, 0x00000001, 0, 0, 32,
	    0x0174, 4, 0, 0, 0, 0x0178, 0, 0x0174, "clkxtal"),
	PLL_B(CLK_APMIXED_ARM_LL, "arm_ll", 0x0104, 0x0110, 0x00000001,
	      PLL_AO, 0, 32, 0x0104, 4, 0, 0, 0, 0x0108, 0, 0x0104,
	      mt7987_arm_ll_div, "clkxtal"),
	PLL(CLK_APMIXED_MSDCPLL, "msdcpll", 0x0124, 0x0130, 0x00000001, 0, 0,
	    32, 0x0124, 4, 0, 0, 0, 0x0128, 0, 0x0124, "clkxtal"),
};

static const struct of_device_id of_match_clk_mt7987_apmixed[] = {
	{ .compatible = "mediatek,mt7987-apmixedsys" },
	{ /* sentinel */ }
};

static int clk_mt7987_apmixed_probe(struct platform_device *pdev)
{
	struct clk_hw_onecell_data *clk_data;
	struct device_node *node = pdev->dev.of_node;
	int r;

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(plls));
	if (!clk_data)
		return -ENOMEM;

	r = mtk_clk_register_plls(node, plls, ARRAY_SIZE(plls), clk_data);
	if (r)
		goto free_apmixed_data;

	r = of_clk_add_hw_provider(node, of_clk_hw_onecell_get, clk_data);
	if (r)
		goto unregister_plls;

	return r;

unregister_plls:
	mtk_clk_unregister_plls(plls, ARRAY_SIZE(plls), clk_data);
free_apmixed_data:
	mtk_free_clk_data(clk_data);
	return r;
}

static struct platform_driver clk_mt7987_apmixed_drv = {
	.probe = clk_mt7987_apmixed_probe,
	.driver = {
		.name = "mt7987-apmixedsys",
		.of_match_table = of_match_clk_mt7987_apmixed,
	},
};
builtin_platform_driver(clk_mt7987_apmixed_drv);
MODULE_LICENSE("GPL");
