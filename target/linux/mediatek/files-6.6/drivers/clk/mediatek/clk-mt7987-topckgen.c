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

static const struct mtk_fixed_factor top_divs[] __initconst = {
	FACTOR(CLK_TOP_CB_M_D2, "cb_m_d2", "mpll", 1, 2),
	FACTOR(CLK_TOP_CB_M_D3, "cb_m_d3", "mpll", 1, 3),
	FACTOR(CLK_TOP_M_D3_D2, "m_d3_d2", "mpll", 1, 6),
	FACTOR(CLK_TOP_CB_M_D4, "cb_m_d4", "mpll", 1, 4),
	FACTOR(CLK_TOP_CB_M_D8, "cb_m_d8", "mpll", 1, 8),
	FACTOR(CLK_TOP_M_D8_D2, "m_d8_d2", "mpll", 1, 16),
	FACTOR(CLK_TOP_CB_APLL2_D4, "cb_apll2_d4", "apll2", 1, 4),
	FACTOR(CLK_TOP_CB_NET1_D3, "cb_net1_d3", "net1pll", 1, 3),
	FACTOR(CLK_TOP_CB_NET1_D4, "cb_net1_d4", "net1pll", 1, 4),
	FACTOR(CLK_TOP_CB_NET1_D5, "cb_net1_d5", "net1pll", 1, 5),
	FACTOR(CLK_TOP_NET1_D5_D2, "net1_d5_d2", "net1pll", 1, 10),
	FACTOR(CLK_TOP_NET1_D5_D4, "net1_d5_d4", "net1pll", 1, 20),
	FACTOR(CLK_TOP_CB_NET1_D7, "cb_net1_d7", "net1pll", 1, 7),
	FACTOR(CLK_TOP_NET1_D7_D2, "net1_d7_d2", "net1pll", 1, 14),
	FACTOR(CLK_TOP_NET1_D7_D4, "net1_d7_d4", "net1pll", 1, 28),
	FACTOR(CLK_TOP_NET1_D8_D2, "net1_d8_d2", "net1pll", 1, 16),
	FACTOR(CLK_TOP_NET1_D8_D4, "net1_d8_d4", "net1pll", 1, 32),
	FACTOR(CLK_TOP_NET1_D8_D8, "net1_d8_d8", "net1pll", 1, 64),
	FACTOR(CLK_TOP_NET1_D8_D16, "net1_d8_d16", "net1pll", 1, 128),
	FACTOR(CLK_TOP_CB_NET2_D2, "cb_net2_d2", "net2pll", 1, 2),
	FACTOR(CLK_TOP_CB_NET2_D4, "cb_net2_d4", "net2pll", 1, 4),
	FACTOR(CLK_TOP_NET2_D4_D4, "net2_d4_d4", "net2pll", 1, 16),
	FACTOR(CLK_TOP_NET2_D4_D8, "net2_d4_d8", "net2pll", 1, 32),
	FACTOR(CLK_TOP_CB_NET2_D6, "cb_net2_d6", "net2pll", 1, 6),
	FACTOR(CLK_TOP_NET2_D7_D2, "net2_d7_d2", "net2pll", 1, 14),
	FACTOR(CLK_TOP_CB_NET2_D8, "cb_net2_d8", "net2pll", 1, 8),
	FACTOR(CLK_TOP_MSDC_D2, "msdc_d2", "msdcpll", 1, 2),
	FACTOR(CLK_TOP_CB_CKSQ_40M, "cb_cksq_40m", "clkxtal", 1, 1),
	FACTOR(CLK_TOP_CKSQ_40M_D2, "cksq_40m_d2", "cb_cksq_40m", 1, 2),
	FACTOR(CLK_TOP_CB_RTC_32K, "cb_rtc_32k", "cb_cksq_40m", 1, 1250),
	FACTOR(CLK_TOP_CB_RTC_32P7K, "cb_rtc_32p7k", "cb_cksq_40m", 1, 1221),
};

static const char *const netsys_parents[] = { "cb_cksq_40m", "cb_net2_d2" };

static const char *const netsys_500m_parents[] = { "cb_cksq_40m", "cb_net1_d5",
						   "net1_d5_d2" };

static const char *const netsys_2x_parents[] = { "cb_cksq_40m", "net2pll" };

static const char *const eth_gmii_parents[] = { "cb_cksq_40m", "net1_d5_d4" };

static const char *const eip_parents[] = { "cb_cksq_40m", "cb_net1_d3",
					   "net2pll", "cb_net1_d4",
					   "cb_net1_d5" };

static const char *const axi_infra_parents[] = { "cb_cksq_40m", "net1_d8_d2" };

static const char *const uart_parents[] = { "cb_cksq_40m", "cb_m_d8",
					    "m_d8_d2" };

static const char *const emmc_250m_parents[] = { "cb_cksq_40m", "net1_d5_d2",
						 "net1_d7_d2" };

static const char *const emmc_400m_parents[] = { "cb_cksq_40m", "msdcpll",
						 "cb_net1_d7",	"cb_m_d2",
						 "net1_d7_d2",	"cb_net2_d6" };

static const char *const spi_parents[] = { "cb_cksq_40m", "cb_m_d2",
					   "net1_d7_d2",  "net1_d8_d2",
					   "cb_net2_d6",  "net1_d5_d4",
					   "cb_m_d4",	  "net1_d8_d4" };

static const char *const nfi_parents[] = {
	"cksq_40m_d2", "net1_d8_d2", "cb_m_d3", "net1_d5_d4", "cb_m_d4",
	"net1_d7_d4",  "net1_d8_d4", "m_d3_d2", "net2_d7_d2", "cb_m_d8"
};

static const char *const pwm_parents[] = { "cb_cksq_40m", "net1_d8_d2",
					   "net1_d5_d4",  "cb_m_d4",
					   "m_d8_d2",	  "cb_rtc_32k" };

static const char *const i2c_parents[] = { "cb_cksq_40m", "net1_d5_d4",
					   "cb_m_d4", "net1_d8_d4" };

static const char *const pcie_mbist_250m_parents[] = { "cb_cksq_40m",
						       "net1_d5_d2" };

static const char *const pextp_tl_ck_parents[] = { "cb_cksq_40m", "cb_net2_d6",
						   "net1_d7_d4", "m_d8_d2",
						   "cb_rtc_32k" };

static const char *const aud_parents[] = { "cb_cksq_40m", "apll2" };

static const char *const a1sys_parents[] = { "cb_cksq_40m", "cb_apll2_d4" };

static const char *const aud_l_parents[] = { "cb_cksq_40m", "apll2",
					     "m_d8_d2" };

static const char *const usb_phy_parents[] = { "cksq_40m_d2", "m_d8_d2" };

static const char *const sgm_0_parents[] = { "cb_cksq_40m", "sgmpll" };

static const char *const sgm_sbus_0_parents[] = { "cb_cksq_40m",
						  "net1_d8_d4" };

static const char *const sysapb_parents[] = { "cb_cksq_40m", "m_d3_d2" };

static const char *const eth_refck_50m_parents[] = { "cb_cksq_40m",
						     "net2_d4_d4" };

static const char *const eth_sys_200m_parents[] = { "cb_cksq_40m",
						    "cb_net2_d4" };

static const char *const eth_xgmii_parents[] = { "cksq_40m_d2", "net1_d8_d8",
						 "net1_d8_d16" };

static const char *const dramc_md32_parents[] = { "cb_cksq_40m", "cb_m_d2",
						  "wedmcupll" };

static const char *const da_xtp_glb_p0_parents[] = { "cb_cksq_40m",
						     "cb_net2_d8" };

static const char *const da_ckm_xtal_parents[] = { "cb_cksq_40m", "m_d8_d2" };

static const char *const eth_mii_parents[] = { "cksq_40m_d2", "net2_d4_d8" };

static const char *const emmc_200m_parents[] = { "cb_cksq_40m", "msdc_d2",
						 "net1_d7_d2", "cb_net2_d6",
						 "net1_d7_d4" };

static struct mtk_mux top_muxes[] = {
	/* CLK_CFG_0 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_NETSYS_SEL, "netsys_sel", netsys_parents,
			     0x000, 0x004, 0x008, 0, 1, 7, 0x1C0, 0),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_NETSYS_500M_SEL, "netsys_500m_sel",
			     netsys_500m_parents, 0x000, 0x004, 0x008, 8, 2,
			     15, 0x1C0, 1),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_NETSYS_2X_SEL, "netsys_2x_sel",
			     netsys_2x_parents, 0x000, 0x004, 0x008, 16, 1, 23,
			     0x1C0, 2),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_ETH_GMII_SEL, "eth_gmii_sel",
			     eth_gmii_parents, 0x000, 0x004, 0x008, 24, 1, 31,
			     0x1C0, 3),
	/* CLK_CFG_1 */
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_EIP_SEL, "eip_sel", eip_parents,
				   0x010, 0x014, 0x018, 0, 3, 7, 0x1C0, 4,
				   CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_AXI_INFRA_SEL, "axi_infra_sel",
				   axi_infra_parents, 0x010, 0x014, 0x018, 8,
				   1, 15, 0x1C0, 5, CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_UART_SEL, "uart_sel", uart_parents, 0x010,
			     0x014, 0x018, 16, 2, 23, 0x1C0, 6),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_EMMC_250M_SEL, "emmc_250m_sel",
			     emmc_250m_parents, 0x010, 0x014, 0x018, 24, 2, 31,
			     0x1C0, 7),
	/* CLK_CFG_2 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_EMMC_400M_SEL, "emmc_400m_sel",
			     emmc_400m_parents, 0x020, 0x024, 0x028, 0, 3, 7,
			     0x1C0, 8),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_SPI_SEL, "spi_sel", spi_parents, 0x020,
			     0x024, 0x028, 8, 3, 15, 0x1C0, 9),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_SPIM_MST_SEL, "spim_mst_sel", spi_parents,
			     0x020, 0x024, 0x028, 16, 3, 23, 0x1C0, 10),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_NFI_SEL, "nfi_sel", nfi_parents, 0x020,
			     0x024, 0x028, 24, 4, 31, 0x1C0, 11),
	/* CLK_CFG_3 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_PWM_SEL, "pwm_sel", pwm_parents, 0x030,
			     0x034, 0x038, 0, 3, 7, 0x1C0, 12),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_I2C_SEL, "i2c_sel", i2c_parents, 0x030,
			     0x034, 0x038, 8, 2, 15, 0x1C0, 13),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_PCIE_MBIST_250M_SEL,
			     "pcie_mbist_250m_sel", pcie_mbist_250m_parents,
			     0x030, 0x034, 0x038, 16, 1, 23, 0x1C0, 14),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_PEXTP_TL_SEL, "pextp_tl_ck_sel",
			     pextp_tl_ck_parents, 0x030, 0x034, 0x038, 24, 3,
			     31, 0x1C0, 15),
	/* CLK_CFG_4 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_PEXTP_TL_P1_SEL, "pextp_tl_ck_p1_sel",
			     pextp_tl_ck_parents, 0x040, 0x044, 0x048, 0, 3, 7,
			     0x1C0, 16),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_USB_SYS_P1_SEL, "usb_sys_p1_sel",
			     eth_gmii_parents, 0x040, 0x044, 0x048, 8, 1, 15,
			     0x1C0, 17),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_USB_XHCI_P1_SEL, "usb_xhci_p1_sel",
			     eth_gmii_parents, 0x040, 0x044, 0x048, 16, 1, 23,
			     0x1C0, 18),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_AUD_SEL, "aud_sel", aud_parents, 0x040,
			     0x044, 0x048, 24, 1, 31, 0x1C0, 19),
	/* CLK_CFG_5 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_A1SYS_SEL, "a1sys_sel", a1sys_parents,
			     0x050, 0x054, 0x058, 0, 1, 7, 0x1C0, 20),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_AUD_L_SEL, "aud_l_sel", aud_l_parents,
			     0x050, 0x054, 0x058, 8, 2, 15, 0x1C0, 21),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_A_TUNER_SEL, "a_tuner_sel", a1sys_parents,
			     0x050, 0x054, 0x058, 16, 1, 23, 0x1C0, 22),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_USB_PHY_SEL, "usb_phy_sel",
			     usb_phy_parents, 0x050, 0x054, 0x058, 24, 1, 31,
			     0x1C0, 23),
	/* CLK_CFG_6 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_SGM_0_SEL, "sgm_0_sel", sgm_0_parents,
			     0x060, 0x064, 0x068, 0, 1, 7, 0x1C0, 24),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_SGM_SBUS_0_SEL, "sgm_sbus_0_sel",
				   sgm_sbus_0_parents, 0x060, 0x064, 0x068, 8,
				   1, 15, 0x1C0, 25, CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_SGM_1_SEL, "sgm_1_sel", sgm_0_parents,
			     0x060, 0x064, 0x068, 16, 1, 23, 0x1C0, 26),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_SGM_SBUS_1_SEL, "sgm_sbus_1_sel",
				   sgm_sbus_0_parents, 0x060, 0x064, 0x068, 24,
				   1, 31, 0x1C0, 27, CLK_IS_CRITICAL),
	/* CLK_CFG_7 */
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_SYSAXI_SEL, "sysaxi_sel",
				   axi_infra_parents, 0x070, 0x074, 0x078, 0,
				   1, 7, 0x1C0, 28, CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_SYSAPB_SEL, "sysapb_sel",
				   sysapb_parents, 0x070, 0x074, 0x078, 8, 1,
				   15, 0x1C0, 29, CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_ETH_REFCK_50M_SEL, "eth_refck_50m_sel",
			     eth_refck_50m_parents, 0x070, 0x074, 0x078, 16, 1,
			     23, 0x1C0, 30),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_ETH_SYS_200M_SEL, "eth_sys_200m_sel",
			     eth_sys_200m_parents, 0x070, 0x074, 0x078, 24, 1,
			     31, 0x1C4, 0),
	/* CLK_CFG_8 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_ETH_SYS_SEL, "eth_sys_sel",
			     pcie_mbist_250m_parents, 0x080, 0x084, 0x088, 0,
			     1, 7, 0x1C4, 1),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_ETH_XGMII_SEL, "eth_xgmii_sel",
			     eth_xgmii_parents, 0x080, 0x084, 0x088, 8, 2, 15,
			     0x1C4, 2),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_DRAMC_SEL, "dramc_sel",
				   usb_phy_parents, 0x080, 0x084, 0x088, 16, 1,
				   23, 0x1C4, 3, CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_DRAMC_MD32_SEL, "dramc_md32_sel",
				   dramc_md32_parents, 0x080, 0x084, 0x088, 24,
				   2, 31, 0x1C4, 4, CLK_IS_CRITICAL),
	/* CLK_CFG_9 */
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_INFRA_F26M_SEL,
				   "csw_infra_f26m_sel", usb_phy_parents,
				   0x090, 0x094, 0x098, 0, 1, 7, 0x1C4, 5,
				   CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_PEXTP_P0_SEL, "pextp_p0_sel",
				   usb_phy_parents, 0x090, 0x094, 0x098, 8, 1,
				   15, 0x1C4, 6, CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD_FLAGS(CLK_TOP_PEXTP_P1_SEL, "pextp_p1_sel",
				   usb_phy_parents, 0x090, 0x094, 0x098, 16, 1,
				   23, 0x1C4, 7, CLK_IS_CRITICAL),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_DA_XTP_GLB_P0_SEL, "da_xtp_glb_p0_sel",
			     da_xtp_glb_p0_parents, 0x090, 0x094, 0x098, 24, 1,
			     31, 0x1C4, 8),
	/* CLK_CFG_10 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_DA_XTP_GLB_P1_SEL, "da_xtp_glb_p1_sel",
			     da_xtp_glb_p0_parents, 0x0A0, 0x0A4, 0x0A8, 0, 1,
			     7, 0x1C4, 9),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_CKM_SEL, "ckm_sel", usb_phy_parents,
			     0x0A0, 0x0A4, 0x0A8, 8, 1, 15, 0x1C4, 10),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_DA_CKM_XTAL_SEL, "da_ckm_xtal_sel",
			     da_ckm_xtal_parents, 0x0A0, 0x0A4, 0x0A8, 16, 1,
			     23, 0x1C4, 11),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_PEXTP_SEL, "pextp_sel", usb_phy_parents,
			     0x0A0, 0x0A4, 0x0A8, 24, 1, 31, 0x1C4, 12),
	/* CLK_CFG_11 */
	MUX_GATE_CLR_SET_UPD(CLK_TOP_ETH_MII_SEL, "eth_mii_sel",
			     eth_mii_parents, 0x0B0, 0x0B4, 0x0B8, 0, 1, 7,
			     0x1C4, 13),
	MUX_GATE_CLR_SET_UPD(CLK_TOP_EMMC_200M_SEL, "emmc_200m_sel",
			     emmc_200m_parents, 0x0B0, 0x0B4, 0x0B8, 8, 3, 15,
			     0x1C4, 14),
};

static const struct mtk_composite top_adj_divs[] = {
	DIV_GATE(CLK_TOP_AUD_I2S_M, "aud_i2s_m", "aud_sel", 0x0420, 0, 0x0420,
		 8, 8),
};

static const struct mtk_clk_desc topck_desc = {
	.factor_clks = top_divs,
	.num_factor_clks = ARRAY_SIZE(top_divs),
	.mux_clks = top_muxes,
	.num_mux_clks = ARRAY_SIZE(top_muxes),
	.composite_clks = top_adj_divs,
	.num_composite_clks = ARRAY_SIZE(top_adj_divs),
	.clk_lock = &mt7987_clk_lock,
};

static const struct of_device_id of_match_clk_mt7987_topckgen[] = {
	{ .compatible = "mediatek,mt7987-topckgen", .data = &topck_desc },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, of_match_clk_mt7987_topckgen);

static struct platform_driver clk_mt7987_topckgen_drv = {
	.probe = mtk_clk_simple_probe,
	.remove_new = mtk_clk_simple_remove,
	.driver = {
		.name = "clk-mt7987-topckgen",
		.of_match_table = of_match_clk_mt7987_topckgen,
	},
};
module_platform_driver(clk_mt7987_topckgen_drv);
MODULE_LICENSE("GPL");
