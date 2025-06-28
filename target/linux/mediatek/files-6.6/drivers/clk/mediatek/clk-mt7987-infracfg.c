// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 MediaTek Inc.
 * Author: Lu Tang <Lu.Tang@mediatek.com>
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mfd/syscon.h>
#include "clk-mtk.h"
#include "clk-mux.h"
#include "clk-gate.h"
#include <dt-bindings/clock/mediatek,mt7987-clk.h>
#include <dt-bindings/reset/mediatek,mt7987-resets.h>

#define MT7987_INFRA_RST0_SET_OFFSET	0x70
#define MT7987_INFRA_RST1_SET_OFFSET	0x80

static DEFINE_SPINLOCK(mt7987_clk_lock);

static const char *const infra_mux_uart0_parents[] = { "csw_infra_f26m_sel",
						       "uart_sel" };

static const char *const infra_mux_uart1_parents[] = { "csw_infra_f26m_sel",
						       "uart_sel" };

static const char *const infra_mux_uart2_parents[] = { "csw_infra_f26m_sel",
						       "uart_sel" };

static const char *const infra_mux_spi0_parents[] = {
	"i2c_sel",
	"spi_sel"
};

static const char *const infra_mux_spi1_parents[] = {
	"i2c_sel",
	"spim_mst_sel"
};

static const char *const infra_mux_spi2_bck_parents[] = {
	"i2c_sel",
	"spi_sel"
};

static const char *const infra_pwm_bck_parents[] = { "cb_rtc_32p7k",
						     "csw_infra_f26m_sel",
						     "sysaxi_sel", "pwm_sel" };

static const char *const infra_pcie_gfmux_tl_ck_o_p0_parents[] = {
	"cb_rtc_32p7k", "csw_infra_f26m_sel", "csw_infra_f26m_sel",
	"pextp_tl_ck_sel"
};

static const char *const infra_pcie_gfmux_tl_ck_o_p1_parents[] = {
	"cb_rtc_32p7k", "csw_infra_f26m_sel", "csw_infra_f26m_sel",
	"pextp_tl_ck_p1_sel"
};

static struct mtk_mux infra_muxes[] = {
	/* MODULE_CLK_SEL_0 */
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_MUX_UART0_SEL, "infra_mux_uart0_sel",
			     infra_mux_uart0_parents, 0x0018, 0x0010, 0x0014,
			     0, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_MUX_UART1_SEL, "infra_mux_uart1_sel",
			     infra_mux_uart1_parents, 0x0018, 0x0010, 0x0014,
			     1, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_MUX_UART2_SEL, "infra_mux_uart2_sel",
			     infra_mux_uart2_parents, 0x0018, 0x0010, 0x0014,
			     2, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_MUX_SPI0_SEL, "infra_mux_spi0_sel",
			     infra_mux_spi0_parents, 0x0018, 0x0010, 0x0014, 4,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_MUX_SPI1_SEL, "infra_mux_spi1_sel",
			     infra_mux_spi1_parents, 0x0018, 0x0010, 0x0014, 5,
			     1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_MUX_SPI2_BCK_SEL,
			     "infra_mux_spi2_bck_sel",
			     infra_mux_spi2_bck_parents, 0x0018, 0x0010,
			     0x0014, 6, 1, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_PWM_BCK_SEL, "infra_pwm_bck_sel",
			     infra_pwm_bck_parents, 0x0018, 0x0010, 0x0014, 14,
			     2, -1, -1, -1),
	/* MODULE_CLK_SEL_1 */
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_PCIE_GFMUX_TL_O_P0_SEL,
			     "infra_pcie_gfmux_tl_ck_o_p0_sel",
			     infra_pcie_gfmux_tl_ck_o_p0_parents, 0x0028,
			     0x0020, 0x0024, 0, 2, -1, -1, -1),
	MUX_GATE_CLR_SET_UPD(CLK_INFRA_PCIE_GFMUX_TL_O_P1_SEL,
			     "infra_pcie_gfmux_tl_ck_o_p1_sel",
			     infra_pcie_gfmux_tl_ck_o_p1_parents, 0x0028,
			     0x0020, 0x0024, 2, 2, -1, -1, -1),
};

static const struct mtk_gate_regs infra0_cg_regs = {
	.set_ofs = 0x10,
	.clr_ofs = 0x14,
	.sta_ofs = 0x18,
};

static const struct mtk_gate_regs infra1_cg_regs = {
	.set_ofs = 0x40,
	.clr_ofs = 0x44,
	.sta_ofs = 0x48,
};

static const struct mtk_gate_regs infra2_cg_regs = {
	.set_ofs = 0x50,
	.clr_ofs = 0x54,
	.sta_ofs = 0x58,
};

static const struct mtk_gate_regs infra3_cg_regs = {
	.set_ofs = 0x60,
	.clr_ofs = 0x64,
	.sta_ofs = 0x68,
};

#define GATE_INFRA0(_id, _name, _parent, _shift)                  \
	{                                                         \
		.id = _id, .name = _name, .parent_name = _parent, \
		.regs = &infra0_cg_regs, .shift = _shift,         \
		.ops = &mtk_clk_gate_ops_setclr,                  \
	}

#define GATE_INFRA1(_id, _name, _parent, _shift)                  \
	{                                                         \
		.id = _id, .name = _name, .parent_name = _parent, \
		.regs = &infra1_cg_regs, .shift = _shift,         \
		.ops = &mtk_clk_gate_ops_setclr,                  \
	}

#define GATE_INFRA2(_id, _name, _parent, _shift)                  \
	{                                                         \
		.id = _id, .name = _name, .parent_name = _parent, \
		.regs = &infra2_cg_regs, .shift = _shift,         \
		.ops = &mtk_clk_gate_ops_setclr,                  \
	}

#define GATE_INFRA3(_id, _name, _parent, _shift)                  \
	{                                                         \
		.id = _id, .name = _name, .parent_name = _parent, \
		.regs = &infra3_cg_regs, .shift = _shift,         \
		.ops = &mtk_clk_gate_ops_setclr,                  \
	}

#define GATE_CRITICAL(_id, _name, _parent, _regs, _shift)                 \
	{                                                                 \
		.id = _id, .name = _name, .parent_name = _parent,         \
		.regs = _regs, .shift = _shift, .flags = CLK_IS_CRITICAL, \
		.ops = &mtk_clk_gate_ops_setclr,                          \
	}

static const struct mtk_gate infra_clks[] __initconst = {
	/* INFRA1 */
	GATE_INFRA1(CLK_INFRA_66M_GPT_BCK, "infra_hf_66m_gpt_bck",
		    "sysaxi_sel", 0),
	GATE_INFRA1(CLK_INFRA_66M_PWM_HCK, "infra_hf_66m_pwm_hck",
		    "sysaxi_sel", 1),
	GATE_INFRA1(CLK_INFRA_66M_PWM_BCK, "infra_hf_66m_pwm_bck",
		    "infra_pwm_bck_sel", 2),
	GATE_INFRA1(CLK_INFRA_133M_CQDMA_BCK, "infra_hf_133m_cqdma_bck",
		    "sysaxi_sel", 12),
	GATE_INFRA1(CLK_INFRA_66M_AUD_SLV_BCK, "infra_66m_aud_slv_bck",
		    "sysaxi_sel", 13),
	GATE_INFRA1(CLK_INFRA_AUD_26M, "infra_f_faud_26m",
		    "csw_infra_f26m_sel", 14),
	GATE_INFRA1(CLK_INFRA_AUD_L, "infra_f_faud_l", "aud_l_sel", 15),
	GATE_INFRA1(CLK_INFRA_AUD_AUD, "infra_f_aud_aud", "a1sys_sel", 16),
	GATE_INFRA1(CLK_INFRA_AUD_EG2, "infra_f_faud_eg2", "a_tuner_sel", 18),
	GATE_INFRA1(CLK_INFRA_DRAMC_F26M, "infra_dramc_f26m",
		    "csw_infra_f26m_sel", 19),
	GATE_CRITICAL(CLK_INFRA_133M_DBG_ACKM, "infra_hf_133m_dbg_ackm",
		      "sysaxi_sel", &infra1_cg_regs, 20),
	GATE_INFRA1(CLK_INFRA_66M_AP_DMA_BCK, "infra_66m_ap_dma_bck",
		    "sysaxi_sel", 21),
	GATE_INFRA1(CLK_INFRA_MSDC200_SRC, "infra_f_fmsdc200_src",
		    "emmc_200m_sel", 28),
	GATE_CRITICAL(CLK_INFRA_66M_SEJ_BCK, "infra_hf_66m_sej_bck",
		      "sysaxi_sel", &infra1_cg_regs, 29),
	GATE_CRITICAL(CLK_INFRA_PRE_CK_SEJ_F13M, "infra_pre_ck_sej_f13m",
		    "csw_infra_f26m_sel", &infra1_cg_regs, 30),
	GATE_CRITICAL(CLK_INFRA_66M_TRNG, "infra_hf_66m_trng", "sysaxi_sel",
		      &infra1_cg_regs, 31),
	/* INFRA2 */
	GATE_INFRA2(CLK_INFRA_26M_THERM_SYSTEM, "infra_hf_26m_therm_system",
		    "csw_infra_f26m_sel", 0),
	GATE_INFRA2(CLK_INFRA_I2C_BCK, "infra_i2c_bck", "i2c_sel", 1),
	GATE_INFRA2(CLK_INFRA_66M_UART0_PCK, "infra_hf_66m_uart0_pck",
		    "sysaxi_sel", 3),
	GATE_INFRA2(CLK_INFRA_66M_UART1_PCK, "infra_hf_66m_uart1_pck",
		    "sysaxi_sel", 4),
	GATE_INFRA2(CLK_INFRA_66M_UART2_PCK, "infra_hf_66m_uart2_pck",
		    "sysaxi_sel", 5),
	GATE_INFRA2(CLK_INFRA_52M_UART0_CK, "infra_f_52m_uart0",
		    "infra_mux_uart0_sel", 3),
	GATE_INFRA2(CLK_INFRA_52M_UART1_CK, "infra_f_52m_uart1",
		    "infra_mux_uart1_sel", 4),
	GATE_INFRA2(CLK_INFRA_52M_UART2_CK, "infra_f_52m_uart2",
		    "infra_mux_uart2_sel", 5),
	GATE_INFRA2(CLK_INFRA_NFI, "infra_f_fnfi", "nfi_sel", 9),
	GATE_CRITICAL(CLK_INFRA_66M_NFI_HCK, "infra_hf_66m_nfi_hck",
		      "sysaxi_sel", &infra2_cg_regs, 11),
	GATE_INFRA2(CLK_INFRA_104M_SPI0, "infra_hf_104m_spi0",
		    "infra_mux_spi0_sel", 12),
	GATE_INFRA2(CLK_INFRA_104M_SPI1, "infra_hf_104m_spi1",
		    "infra_mux_spi1_sel", 13),
	GATE_INFRA2(CLK_INFRA_104M_SPI2_BCK, "infra_hf_104m_spi2_bck",
		    "infra_mux_spi2_bck_sel", 14),
	GATE_INFRA2(CLK_INFRA_66M_SPI0_HCK, "infra_hf_66m_spi0_hck",
		    "sysaxi_sel", 15),
	GATE_INFRA2(CLK_INFRA_66M_SPI1_HCK, "infra_hf_66m_spi1_hck",
		    "sysaxi_sel", 16),
	GATE_INFRA2(CLK_INFRA_66M_SPI2_HCK, "infra_hf_66m_spi2_hck",
		    "sysaxi_sel", 17),
	GATE_INFRA2(CLK_INFRA_66M_FLASHIF_AXI, "infra_hf_66m_flashif_axi",
		    "sysaxi_sel", 18),
	GATE_CRITICAL(CLK_INFRA_RTC, "infra_f_frtc", "cb_rtc_32k",
		      &infra2_cg_regs, 19),
	GATE_INFRA2(CLK_INFRA_26M_ADC_BCK, "infra_f_26m_adc_bck",
		    "csw_infra_f26m_sel", 20),
	GATE_INFRA2(CLK_INFRA_RC_ADC, "infra_f_frc_adc", "infra_f_26m_adc_bck",
		    21),
	GATE_INFRA2(CLK_INFRA_MSDC400, "infra_f_fmsdc400", "emmc_400m_sel",
		    22),
	GATE_INFRA2(CLK_INFRA_MSDC2_HCK, "infra_f_fmsdc2_hck", "emmc_250m_sel",
		    23),
	GATE_INFRA2(CLK_INFRA_133M_MSDC_0_HCK, "infra_hf_133m_msdc_0_hck",
		    "sysaxi_sel", 24),
	GATE_INFRA2(CLK_INFRA_66M_MSDC_0_HCK, "infra_66m_msdc_0_hck",
		    "sysaxi_sel", 25),
	GATE_INFRA2(CLK_INFRA_133M_CPUM_BCK, "infra_hf_133m_cpum_bck",
		    "sysaxi_sel", 26),
	GATE_INFRA2(CLK_INFRA_BIST2FPC, "infra_hf_fbist2fpc", "nfi_sel", 27),
	GATE_INFRA2(CLK_INFRA_I2C_X16W_MCK_CK_P1,
		    "infra_hf_i2c_x16w_mck_ck_p1", "sysaxi_sel", 29),
	GATE_INFRA2(CLK_INFRA_I2C_X16W_PCK_CK_P1,
		    "infra_hf_i2c_x16w_pck_ck_p1", "sysaxi_sel", 31),
	/* INFRA3 */
	GATE_INFRA3(CLK_INFRA_133M_USB_HCK, "infra_133m_usb_hck", "sysaxi_sel",
		    0),
	GATE_INFRA3(CLK_INFRA_133M_USB_HCK_CK_P1, "infra_133m_usb_hck_ck_p1",
		    "sysaxi_sel", 1),
	GATE_INFRA3(CLK_INFRA_66M_USB_HCK, "infra_66m_usb_hck", "sysaxi_sel",
		    2),
	GATE_INFRA3(CLK_INFRA_66M_USB_HCK_CK_P1, "infra_66m_usb_hck_ck_p1",
		    "sysaxi_sel", 3),
	GATE_INFRA3(CLK_INFRA_USB_SYS_CK_P1, "infra_usb_sys_ck_p1",
		    "usb_sys_p1_sel", 5),
	GATE_INFRA3(CLK_INFRA_USB_CK_P1, "infra_usb_ck_p1", "cb_cksq_40m", 7),
	GATE_CRITICAL(CLK_INFRA_USB_FRMCNT_CK_P1, "infra_usb_frmcnt_ck_p1",
		      "cksq_40m_d2", &infra3_cg_regs, 9),
	GATE_CRITICAL(CLK_INFRA_USB_PIPE_CK_P1, "infra_usb_pipe_ck_p1",
		      "usb_phy_sel", &infra3_cg_regs, 11),
	GATE_INFRA3(CLK_INFRA_USB_UTMI_CK_P1, "infra_usb_utmi_ck_p1",
		    "clkxtal", 13),
	GATE_INFRA3(CLK_INFRA_USB_XHCI_CK_P1, "infra_usb_xhci_ck_p1",
		    "usb_xhci_p1_sel", 15),
	GATE_INFRA3(CLK_INFRA_PCIE_GFMUX_TL_P0, "infra_pcie_gfmux_tl_ck_p0",
		    "infra_pcie_gfmux_tl_ck_o_p0_sel", 20),
	GATE_INFRA3(CLK_INFRA_PCIE_GFMUX_TL_P1, "infra_pcie_gfmux_tl_ck_p1",
		    "infra_pcie_gfmux_tl_ck_o_p1_sel", 21),
	GATE_INFRA3(CLK_INFRA_PCIE_PIPE_P0, "infra_pcie_pipe_ck_p0", "clkxtal",
		    24),
	GATE_INFRA3(CLK_INFRA_PCIE_PIPE_P1, "infra_pcie_pipe_ck_p1", "clkxtal",
		    25),
	GATE_INFRA3(CLK_INFRA_133M_PCIE_CK_P0, "infra_133m_pcie_ck_p0",
		    "sysaxi_sel", 28),
	GATE_INFRA3(CLK_INFRA_133M_PCIE_CK_P1, "infra_133m_pcie_ck_p1",
		    "sysaxi_sel", 29),
	/* INFRA0 */
	GATE_INFRA0(CLK_INFRA_PCIE_PERI_26M_CK_P0,
		    "infra_pcie_peri_ck_26m_ck_p0", "csw_infra_f26m_sel", 7),
	GATE_INFRA0(CLK_INFRA_PCIE_PERI_26M_CK_P1,
		    "infra_pcie_peri_ck_26m_ck_p1", "csw_infra_f26m_sel", 8),
};

static u16 infra_rst_ofs[] = {
	MT7987_INFRA_RST0_SET_OFFSET,
	MT7987_INFRA_RST1_SET_OFFSET,
};

static u16 infra_idx_map[] = {
	[MT7987_INFRA_RST0_PEXTP_MAC_SWRST] = 0 * RST_NR_PER_BANK + 6,
	[MT7987_INFRA_RST1_THERM_CTRL_SWRST] = 1 * RST_NR_PER_BANK + 9,
};

static struct mtk_clk_rst_desc infra_rst_desc = {
	.version = MTK_RST_SET_CLR,
	.rst_bank_ofs = infra_rst_ofs,
	.rst_bank_nr = ARRAY_SIZE(infra_rst_ofs),
	.rst_idx_map = infra_idx_map,
	.rst_idx_map_nr = ARRAY_SIZE(infra_idx_map),
};

static const struct mtk_clk_desc infra_desc = {
	.clks = infra_clks,
	.num_clks = ARRAY_SIZE(infra_clks),
	.mux_clks = infra_muxes,
	.num_mux_clks = ARRAY_SIZE(infra_muxes),
	.clk_lock = &mt7987_clk_lock,
	.rst_desc = &infra_rst_desc,
};

static const struct of_device_id of_match_clk_mt7987_infracfg[] = {
	{ .compatible = "mediatek,mt7987-infracfg", .data = &infra_desc },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, of_match_clk_mt7987_infracfg);

static struct platform_driver clk_mt7987_infracfg_drv = {
	.driver = {
		.name = "clk-mt7987-infracfg",
		.of_match_table = of_match_clk_mt7987_infracfg,
	},
	.probe = mtk_clk_simple_probe,
	.remove_new = mtk_clk_simple_remove,
};
module_platform_driver(clk_mt7987_infracfg_drv);
MODULE_LICENSE("GPL");
