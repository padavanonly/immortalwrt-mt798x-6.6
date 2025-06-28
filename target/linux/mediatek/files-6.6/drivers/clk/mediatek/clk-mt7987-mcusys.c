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
#include <dt-bindings/clock/mediatek,mt7987-clk.h>

static DEFINE_SPINLOCK(mt7987_clk_lock);
static const char *const mcu_bus_div_parents[] = { "cb_cksq_40m", "arm_ll" };

static struct mtk_composite mcu_muxes[] = {
	MUX_GATE_FLAGS(CLK_MCU_BUS_DIV_SEL, "mcu_bus_div_sel",
		       mcu_bus_div_parents, 0x7C0, 9, 1, -1, CLK_IS_CRITICAL),
};

static const struct mtk_clk_desc mcusys_desc = {
	.composite_clks = mcu_muxes,
	.num_composite_clks = ARRAY_SIZE(mcu_muxes),
	.clk_lock = &mt7987_clk_lock,
};

static const struct of_device_id of_match_clk_mt7987_mcusys[] = {
	{ .compatible = "mediatek,mt7987-mcusys", .data = &mcusys_desc },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, of_match_clk_mt7987_mcusys);

static struct platform_driver clk_mt7987_mcusys_drv = {
	.driver = {
		.name = "clk-mt7987-mcusys",
		.of_match_table = of_match_clk_mt7987_mcusys,
	},
	.probe = mtk_clk_simple_probe,
	.remove_new = mtk_clk_simple_remove,
};
module_platform_driver(clk_mt7987_mcusys_drv);
MODULE_LICENSE("GPL");
