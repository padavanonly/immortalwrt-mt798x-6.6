// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Airoha Inc.
 * Author: Min Yao <min.yao@airoha.com>
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/of_platform.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/of_address.h>

#include "an8855.h"
#include "an8855_regs.h"

/* AN8855 registers */
#define SCU_BASE					0x10000000
#define RG_RGMII_TXCK_C				(SCU_BASE + 0x1d0)
#define RG_GPIO_LED_MODE			(SCU_BASE + 0x0054)
#define RG_GPIO_LED_SEL(i)	(SCU_BASE + (0x0058 + ((i) * 4)))
#define RG_INTB_MODE				(SCU_BASE + 0x0080)
#define RG_GDMP_RAM				(SCU_BASE + 0x10000)

#define RG_GPIO_L_INV			(SCU_BASE + 0x0010)
#define RG_GPIO_CTRL			(SCU_BASE + 0xa300)
#define RG_GPIO_DATA			(SCU_BASE + 0xa304)
#define RG_GPIO_OE			(SCU_BASE + 0xa314)


#define HSGMII_AN_CSR_BASE			0x10220000
#define SGMII_REG_AN0				(HSGMII_AN_CSR_BASE + 0x000)
#define SGMII_REG_AN_13				(HSGMII_AN_CSR_BASE + 0x034)
#define SGMII_REG_AN_FORCE_CL37		(HSGMII_AN_CSR_BASE + 0x060)

#define HSGMII_CSR_PCS_BASE			0x10220000
#define RG_HSGMII_PCS_CTROL_1		(HSGMII_CSR_PCS_BASE + 0xa00)
#define RG_AN_SGMII_MODE_FORCE		(HSGMII_CSR_PCS_BASE + 0xa24)

#define MULTI_SGMII_CSR_BASE		0x10224000
#define SGMII_STS_CTRL_0			(MULTI_SGMII_CSR_BASE + 0x018)
#define MSG_RX_CTRL_0				(MULTI_SGMII_CSR_BASE + 0x100)
#define MSG_RX_LIK_STS_0			(MULTI_SGMII_CSR_BASE + 0x514)
#define MSG_RX_LIK_STS_2			(MULTI_SGMII_CSR_BASE + 0x51c)
#define PHY_RX_FORCE_CTRL_0			(MULTI_SGMII_CSR_BASE + 0x520)

#define XFI_CSR_PCS_BASE			0x10225000
#define RG_USXGMII_AN_CONTROL_0		(XFI_CSR_PCS_BASE + 0xbf8)

#define MULTI_PHY_RA_CSR_BASE		0x10226000
#define RG_RATE_ADAPT_CTRL_0		(MULTI_PHY_RA_CSR_BASE + 0x000)
#define RATE_ADP_P0_CTRL_0			(MULTI_PHY_RA_CSR_BASE + 0x100)
#define MII_RA_AN_ENABLE			(MULTI_PHY_RA_CSR_BASE + 0x300)

#define QP_DIG_CSR_BASE				0x1022a000
#define QP_CK_RST_CTRL_4			(QP_DIG_CSR_BASE + 0x310)
#define QP_DIG_MODE_CTRL_0			(QP_DIG_CSR_BASE + 0x324)
#define QP_DIG_MODE_CTRL_1			(QP_DIG_CSR_BASE + 0x330)

#define SERDES_WRAPPER_BASE			0x1022c000
#define USGMII_CTRL_0				(SERDES_WRAPPER_BASE + 0x000)

#define QP_PMA_TOP_BASE				0x1022e000
#define PON_RXFEDIG_CTRL_0			(QP_PMA_TOP_BASE + 0x100)
#define PON_RXFEDIG_CTRL_9			(QP_PMA_TOP_BASE + 0x124)

#define SS_LCPLL_PWCTL_SETTING_2	(QP_PMA_TOP_BASE + 0x208)
#define SS_LCPLL_TDC_FLT_2			(QP_PMA_TOP_BASE + 0x230)
#define SS_LCPLL_TDC_FLT_5			(QP_PMA_TOP_BASE + 0x23c)
#define SS_LCPLL_TDC_PCW_1			(QP_PMA_TOP_BASE + 0x248)
#define INTF_CTRL_8		(QP_PMA_TOP_BASE + 0x320)
#define INTF_CTRL_9		(QP_PMA_TOP_BASE + 0x324)
#define INTF_CTRL_10	(QP_PMA_TOP_BASE + 0x328)
#define INTF_CTRL_11	(QP_PMA_TOP_BASE + 0x32c)
#define PLL_CTRL_0		(QP_PMA_TOP_BASE + 0x400)
#define PLL_CTRL_2		(QP_PMA_TOP_BASE + 0x408)
#define PLL_CTRL_3		(QP_PMA_TOP_BASE + 0x40c)
#define PLL_CTRL_4		(QP_PMA_TOP_BASE + 0x410)
#define PLL_CK_CTRL_0	(QP_PMA_TOP_BASE + 0x414)
#define RX_DLY_0		(QP_PMA_TOP_BASE + 0x614)
#define RX_CTRL_2		(QP_PMA_TOP_BASE + 0x630)
#define RX_CTRL_5		(QP_PMA_TOP_BASE + 0x63c)
#define RX_CTRL_6		(QP_PMA_TOP_BASE + 0x640)
#define RX_CTRL_7		(QP_PMA_TOP_BASE + 0x644)
#define RX_CTRL_8		(QP_PMA_TOP_BASE + 0x648)
#define RX_CTRL_26		(QP_PMA_TOP_BASE + 0x690)
#define RX_CTRL_42		(QP_PMA_TOP_BASE + 0x6d0)

#define QP_ANA_CSR_BASE				0x1022f000
#define RG_QP_RX_DAC_EN				(QP_ANA_CSR_BASE + 0x00)
#define RG_QP_RXAFE_RESERVE			(QP_ANA_CSR_BASE + 0x04)
#define RG_QP_CDR_LPF_BOT_LIM		(QP_ANA_CSR_BASE + 0x08)
#define RG_QP_CDR_LPF_MJV_LIM		(QP_ANA_CSR_BASE + 0x0c)
#define RG_QP_CDR_LPF_SETVALUE		(QP_ANA_CSR_BASE + 0x14)
#define RG_QP_CDR_PR_CKREF_DIV1		(QP_ANA_CSR_BASE + 0x18)
#define RG_QP_CDR_PR_KBAND_DIV_PCIE	(QP_ANA_CSR_BASE + 0x1c)
#define RG_QP_CDR_FORCE_IBANDLPF_R_OFF	(QP_ANA_CSR_BASE + 0x20)
#define RG_QP_TX_MODE_16B_EN		(QP_ANA_CSR_BASE + 0x28)
#define RG_QP_PLL_IPLL_DIG_PWR_SEL	(QP_ANA_CSR_BASE + 0x3c)
#define RG_QP_PLL_SDM_ORD			(QP_ANA_CSR_BASE + 0x40)

#define ETHER_SYS_BASE				0x1028c800
#define RG_P5MUX_MODE				(ETHER_SYS_BASE + 0x00)
#define RG_FORCE_CKDIR_SEL			(ETHER_SYS_BASE + 0x04)
#define RG_SWITCH_MODE				(ETHER_SYS_BASE + 0x08)
#define RG_FORCE_MAC5_SB			(ETHER_SYS_BASE + 0x2c)
#define RG_GPHY_AFE_PWD				(ETHER_SYS_BASE + 0x40)
#define RG_GPHY_SMI_ADDR			(ETHER_SYS_BASE + 0x48)
#define CSR_RMII					(ETHER_SYS_BASE + 0x70)

/* PHY EEE Register bitmap of define */
#define PHY_DEV07				0x07
#define PHY_DEV07_REG_03C		0x3c

/* PHY Extend Register 0x14 bitmap of define */
#define PHY_EXT_REG_14			0x14

/* Fields of PHY_EXT_REG_14 */
#define PHY_EN_DOWN_SHFIT		BIT(4)

/* PHY dev address 0x1E */
#define PHY_DEV1E				0x1e

/* PHY TX PAIR DELAY SELECT Register */
#define PHY_TX_PAIR_DLY_SEL_GBE		0x013
/* PHY ADC Register */
#define PHY_RXADC_CTRL				0x0d8
#define PHY_RXADC_REV_0				0x0d9
#define PHY_RXADC_REV_1				0x0da

/* PHY LED Register bitmap of define */
#define PHY_LED_CTRL_SELECT		0x3e8
#define PHY_SINGLE_LED_ON_CTRL(i)	(0x3e0 + ((i) * 2))
#define PHY_SINGLE_LED_BLK_CTRL(i)	(0x3e1 + ((i) * 2))
#define PHY_SINGLE_LED_ON_DUR(i)	(0x3e9 + ((i) * 2))
#define PHY_SINGLE_LED_BLK_DUR(i)	(0x3ea + ((i) * 2))

#define PHY_PMA_CTRL	(0x340)

/* PHY dev address 0x1F */
#define PHY_DEV1F				0x1f
#define PHY_LED_ON_CTRL(i)		(0x24 + ((i) * 2))
#define LED_ON_EN				(1 << 15)
#define LED_ON_POL				(1 << 14)
#define LED_ON_EVT_MASK			(0x7f)
/* LED ON Event */
#define LED_ON_EVT_FORCE		(1 << 6)
#define LED_ON_EVT_LINK_HD		(1 << 5)
#define LED_ON_EVT_LINK_FD		(1 << 4)
#define LED_ON_EVT_LINK_DOWN	(1 << 3)
#define LED_ON_EVT_LINK_10M		(1 << 2)
#define LED_ON_EVT_LINK_100M	(1 << 1)
#define LED_ON_EVT_LINK_1000M	(1 << 0)

#define PHY_LED_BLK_CTRL(i)		(0x25 + ((i) * 2))
#define LED_BLK_EVT_MASK		(0x3ff)
/* LED Blinking Event */
#define LED_BLK_EVT_FORCE			(1 << 9)
#define LED_BLK_EVT_10M_RX_ACT		(1 << 5)
#define LED_BLK_EVT_10M_TX_ACT		(1 << 4)
#define LED_BLK_EVT_100M_RX_ACT		(1 << 3)
#define LED_BLK_EVT_100M_TX_ACT		(1 << 2)
#define LED_BLK_EVT_1000M_RX_ACT	(1 << 1)
#define LED_BLK_EVT_1000M_TX_ACT	(1 << 0)

#define PHY_LED_BCR				(0x21)
#define LED_BCR_EXT_CTRL		(1 << 15)
#define LED_BCR_CLK_EN			(1 << 3)
#define LED_BCR_TIME_TEST		(1 << 2)
#define LED_BCR_MODE_MASK		(3)
#define LED_BCR_MODE_DISABLE	(0)

#define PHY_LED_ON_DUR			(0x22)
#define LED_ON_DUR_MASK			(0xffff)

#define PHY_LED_BLK_DUR			(0x23)
#define LED_BLK_DUR_MASK		(0xffff)

#define PHY_LED_BLINK_DUR_CTRL	(0x720)

/* Unique fields of PMCR for AN8855 */
#define FORCE_TX_FC		BIT(4)
#define FORCE_RX_FC		BIT(5)
#define FORCE_EEE100		BIT(6)
#define FORCE_EEE1G		BIT(7)
#define FORCE_EEE2P5G		BIT(8)
#define FORCE_DPX		BIT(25)
#define FORCE_SPD		BITS(28, 30)
#define FORCE_LNK		BIT(24)
#define FORCE_MODE		BIT(31)

#define CHIP_ID			0x10005000
#define CHIP_REV		0x10005004

#define AN8855_EFUSE_DATA0	0x1000a500

const u8 r50ohm_table[] = {
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 126, 122, 117,
	112, 109, 104, 101,  97,  94,  90,  88,  84,  80,
	78,  74,  72,  68,  66,  64,  61,  58,  56,  53,
	51,  48,  47,  44,  42,  40,  38,  36,  34,  32,
	31,  28,  27,  24,  24,  22,  20,  18,  16,  16,
	14,  12,  11,   9
};

static u8 shift_check(u8 base)
{
	u8 i;
	u32 sz = sizeof(r50ohm_table)/sizeof(u8);

	for (i = 0; i < sz; ++i)
		if (r50ohm_table[i] == base)
			break;

	if (i < 8 || i >= sz)
		return 25; /* index of 94 */

	return (i - 8);
}

static u8 get_shift_val(u8 idx)
{
	return r50ohm_table[idx];
}

/* T830 AN8855 Reference Board */
static const struct an8855_led_cfg led_cfg[] = {
/*************************************************************************
 * Enable, LED idx, LED Polarity, LED ON event,  LED Blink event  LED Freq
 *************************************************************************
 */
	/* GPIO0 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO1 */
	{1, P0_LED1, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO2 */
	{1, P1_LED1, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO3 */
	{1, P2_LED1, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO4 */
	{1, P3_LED1, LED_LOW,  LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO5 */
	{1, P4_LED1, LED_LOW,  LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO6 */
	{0, PHY_LED_MAX, LED_LOW,  LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO7 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO8 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO9 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO10 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO11 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO12 */
	{0, PHY_LED_MAX, LED_LOW,  LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO13 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO14 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO15 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO16 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO17 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO18 */
	{0, PHY_LED_MAX, LED_HIGH, LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO19 */
	{0, PHY_LED_MAX, LED_LOW,  LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
	/* GPIO20 */
	{0, PHY_LED_MAX, LED_LOW,  LED_ON_EVENT, LED_BLK_EVENT, LED_FREQ},
};

static int an8855_set_hsgmii_mode(struct gsw_an8855 *gsw)
{
	u32 val = 0;

	/* TX FIR - improve TX EYE */
	val = an8855_reg_read(gsw, INTF_CTRL_10);
	val &= ~(0x3f << 16);
	val |= BIT(21);
	val &= ~(0x1f << 24);
	val |= (0x4 << 24);
	val |= BIT(29);
	an8855_reg_write(gsw, INTF_CTRL_10, val);

	val = an8855_reg_read(gsw, INTF_CTRL_11);
	val &= ~(0x3f);
	val |= BIT(6);
	an8855_reg_write(gsw, INTF_CTRL_11, val);

	/* RX CDR - improve RX Jitter Tolerance */
	val = an8855_reg_read(gsw, RG_QP_CDR_LPF_BOT_LIM);
	val &= ~(0x7 << 24);
	val |= (0x5 << 24);
	val &= ~(0x7 << 20);
	val |= (0x5 << 20);
	an8855_reg_write(gsw, RG_QP_CDR_LPF_BOT_LIM, val);

	/* PLL */
	val = an8855_reg_read(gsw, QP_DIG_MODE_CTRL_1);
	val &= ~(0x3 << 2);
	val |= (0x1 << 2);
	an8855_reg_write(gsw, QP_DIG_MODE_CTRL_1, val);

	/* PLL - LPF */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~(0x3 << 0);
	val |= (0x1 << 0);
	val &= ~(0x7 << 2);
	val |= (0x5 << 2);
	val &= ~BITS(6, 7);
	val &= ~(0x7 << 8);
	val |= (0x3 << 8);
	val |= BIT(29);
	val &= ~BITS(12, 13);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - ICO */
	val = an8855_reg_read(gsw, PLL_CTRL_4);
	val |= BIT(2);
	an8855_reg_write(gsw, PLL_CTRL_4, val);

	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~BIT(14);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - CHP */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~(0xf << 16);
	val |= (0x6 << 16);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - PFD */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~(0x3 << 20);
	val |= (0x1 << 20);
	val &= ~(0x3 << 24);
	val |= (0x1 << 24);
	val &= ~BIT(26);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - POSTDIV */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val |= BIT(22);
	val &= ~BIT(27);
	val &= ~BIT(28);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - SDM */
	val = an8855_reg_read(gsw, PLL_CTRL_4);
	val &= ~BITS(3, 4);
	an8855_reg_write(gsw, PLL_CTRL_4, val);

	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~BIT(30);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	val = an8855_reg_read(gsw, SS_LCPLL_PWCTL_SETTING_2);
	val &= ~(0x3 << 16);
	val |= (0x1 << 16);
	an8855_reg_write(gsw, SS_LCPLL_PWCTL_SETTING_2, val);

	an8855_reg_write(gsw, SS_LCPLL_TDC_FLT_2, 0x7a000000);
	an8855_reg_write(gsw, SS_LCPLL_TDC_PCW_1, 0x7a000000);

	val = an8855_reg_read(gsw, SS_LCPLL_TDC_FLT_5);
	val &= ~BIT(24);
	an8855_reg_write(gsw, SS_LCPLL_TDC_FLT_5, val);

	val = an8855_reg_read(gsw, PLL_CK_CTRL_0);
	val &= ~BIT(8);
	an8855_reg_write(gsw, PLL_CK_CTRL_0, val);

	/* PLL - SS */
	val = an8855_reg_read(gsw, PLL_CTRL_3);
	val &= ~BITS(0, 15);
	an8855_reg_write(gsw, PLL_CTRL_3, val);

	val = an8855_reg_read(gsw, PLL_CTRL_4);
	val &= ~BITS(0, 1);
	an8855_reg_write(gsw, PLL_CTRL_4, val);

	val = an8855_reg_read(gsw, PLL_CTRL_3);
	val &= ~BITS(16, 31);
	an8855_reg_write(gsw, PLL_CTRL_3, val);

	/* PLL - TDC */
	val = an8855_reg_read(gsw, PLL_CK_CTRL_0);
	val &= ~BIT(9);
	an8855_reg_write(gsw, PLL_CK_CTRL_0, val);

	val = an8855_reg_read(gsw, RG_QP_PLL_SDM_ORD);
	val |= BIT(3);
	val |= BIT(4);
	an8855_reg_write(gsw, RG_QP_PLL_SDM_ORD, val);

	val = an8855_reg_read(gsw, RG_QP_RX_DAC_EN);
	val &= ~(0x3 << 16);
	val |= (0x2 << 16);
	an8855_reg_write(gsw, RG_QP_RX_DAC_EN, val);

	/* TCL Disable (only for Co-SIM) */
	val = an8855_reg_read(gsw, PON_RXFEDIG_CTRL_0);
	val &= ~BIT(12);
	an8855_reg_write(gsw, PON_RXFEDIG_CTRL_0, val);

	/* TX Init */
	val = an8855_reg_read(gsw, RG_QP_TX_MODE_16B_EN);
	val &= ~BIT(0);
	val &= ~(0xffff << 16);
	val |= (0x4 << 16);
	an8855_reg_write(gsw, RG_QP_TX_MODE_16B_EN, val);

	/* RX Control */
	val = an8855_reg_read(gsw, RG_QP_RXAFE_RESERVE);
	val |= BIT(11);
	an8855_reg_write(gsw, RG_QP_RXAFE_RESERVE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_LPF_MJV_LIM);
	val &= ~(0x3 << 4);
	val |= (0x1 << 4);
	an8855_reg_write(gsw, RG_QP_CDR_LPF_MJV_LIM, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_LPF_SETVALUE);
	val &= ~(0xf << 25);
	val |= (0x1 << 25);
	val &= ~(0x7 << 29);
	val |= (0x6 << 29);
	an8855_reg_write(gsw, RG_QP_CDR_LPF_SETVALUE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_CKREF_DIV1);
	val &= ~(0x1f << 8);
	val |= (0xf << 8);
	an8855_reg_write(gsw, RG_QP_CDR_PR_CKREF_DIV1, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE);
	val &= ~(0x3f << 0);
	val |= (0x19 << 0);
	val &= ~BIT(6);
	an8855_reg_write(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_FORCE_IBANDLPF_R_OFF);
	val &= ~(0x7f << 6);
	val |= (0x21 << 6);
	val &= ~(0x3 << 16);
	val |= (0x2 << 16);
	val &= ~BIT(13);
	an8855_reg_write(gsw, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE);
	val &= ~BIT(30);
	an8855_reg_write(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_CKREF_DIV1);
	val &= ~(0x7 << 24);
	val |= (0x4 << 24);
	an8855_reg_write(gsw, RG_QP_CDR_PR_CKREF_DIV1, val);

	/* PMA (For HW Mode) */
	val = an8855_reg_read(gsw, RX_CTRL_26);
	val |= BIT(23);
	val &= ~BIT(24);
	val |= BIT(26);
	an8855_reg_write(gsw, RX_CTRL_26, val);

	val = an8855_reg_read(gsw, RX_DLY_0);
	val &= ~(0xff << 0);
	val |= (0x6f << 0);
	val |= BITS(8, 13);
	an8855_reg_write(gsw, RX_DLY_0, val);

	val = an8855_reg_read(gsw, RX_CTRL_42);
	val &= ~(0x1fff << 0);
	val |= (0x150 << 0);
	an8855_reg_write(gsw, RX_CTRL_42, val);

	val = an8855_reg_read(gsw, RX_CTRL_2);
	val &= ~(0x1fff << 16);
	val |= (0x150 << 16);
	an8855_reg_write(gsw, RX_CTRL_2, val);

	val = an8855_reg_read(gsw, PON_RXFEDIG_CTRL_9);
	val &= ~(0x7 << 0);
	val |= (0x1 << 0);
	an8855_reg_write(gsw, PON_RXFEDIG_CTRL_9, val);

	val = an8855_reg_read(gsw, RX_CTRL_8);
	val &= ~(0xfff << 16);
	val |= (0x200 << 16);
	val &= ~(0x7fff << 0);
	val |= (0xfff << 0);
	an8855_reg_write(gsw, RX_CTRL_8, val);

	/* Frequency memter */
	val = an8855_reg_read(gsw, RX_CTRL_5);
	val &= ~(0xfffff << 10);
	val |= (0x10 << 10);
	an8855_reg_write(gsw, RX_CTRL_5, val);

	val = an8855_reg_read(gsw, RX_CTRL_6);
	val &= ~(0xfffff << 0);
	val |= (0x64 << 0);
	an8855_reg_write(gsw, RX_CTRL_6, val);

	val = an8855_reg_read(gsw, RX_CTRL_7);
	val &= ~(0xfffff << 0);
	val |= (0x2710 << 0);
	an8855_reg_write(gsw, RX_CTRL_7, val);

	val = an8855_reg_read(gsw, PLL_CTRL_0);
	val |= BIT(0);
	an8855_reg_write(gsw, PLL_CTRL_0, val);

	/* PCS Init */
	val = an8855_reg_read(gsw, RG_HSGMII_PCS_CTROL_1);
	val &= ~BIT(30);
	an8855_reg_write(gsw, RG_HSGMII_PCS_CTROL_1, val);

	/* Rate Adaption */
	val = an8855_reg_read(gsw, RATE_ADP_P0_CTRL_0);
	val &= ~BIT(31);
	an8855_reg_write(gsw, RATE_ADP_P0_CTRL_0, val);

	val = an8855_reg_read(gsw, RG_RATE_ADAPT_CTRL_0);
	val |= BIT(0);
	val |= BIT(4);
	val |= BITS(26, 27);
	an8855_reg_write(gsw, RG_RATE_ADAPT_CTRL_0, val);

	/* Disable AN */
	val = an8855_reg_read(gsw, SGMII_REG_AN0);
	val &= ~BIT(12);
	an8855_reg_write(gsw, SGMII_REG_AN0, val);

	/* Force Speed */
	val = an8855_reg_read(gsw, SGMII_STS_CTRL_0);
	val |= BIT(2);
	val |= BITS(4, 5);
	an8855_reg_write(gsw, SGMII_STS_CTRL_0, val);

	/* bypass flow control to MAC */
	an8855_reg_write(gsw, MSG_RX_LIK_STS_0, 0x01010107);
	an8855_reg_write(gsw, MSG_RX_LIK_STS_2, 0x00000EEF);

	return 0;
}

static int an8855_sgmii_setup(struct gsw_an8855 *gsw, int mode)
{
	u32 val = 0;

	/* TX FIR - improve TX EYE */
	val = an8855_reg_read(gsw, INTF_CTRL_10);
	val &= ~(0x3f << 16);
	val |= BIT(21);
	val &= ~(0x1f << 24);
	val |= BIT(29);
	an8855_reg_write(gsw, INTF_CTRL_10, val);

	val = an8855_reg_read(gsw, INTF_CTRL_11);
	val &= ~(0x3f);
	val |= (0xd << 0);
	val |= BIT(6);
	an8855_reg_write(gsw, INTF_CTRL_11, val);

	/* RX CDR - improve RX Jitter Tolerance */
	val = an8855_reg_read(gsw, RG_QP_CDR_LPF_BOT_LIM);
	val &= ~(0x7 << 24);
	val |= (0x6 << 24);
	val &= ~(0x7 << 20);
	val |= (0x6 << 20);
	an8855_reg_write(gsw, RG_QP_CDR_LPF_BOT_LIM, val);

	/* PMA Init */
	/* PLL */
	val = an8855_reg_read(gsw, QP_DIG_MODE_CTRL_1);
	val &= ~BITS(2, 3);
	an8855_reg_write(gsw, QP_DIG_MODE_CTRL_1, val);

	/* PLL - LPF */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~(0x3 << 0);
	val |= (0x1 << 0);
	val &= ~(0x7 << 2);
	val |= (0x5 << 2);
	val &= ~BITS(6, 7);
	val &= ~(0x7 << 8);
	val |= (0x3 << 8);
	val |= BIT(29);
	val &= ~BITS(12, 13);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - ICO */
	val = an8855_reg_read(gsw, PLL_CTRL_4);
	val |= BIT(2);
	an8855_reg_write(gsw, PLL_CTRL_4, val);

	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~BIT(14);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - CHP */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~(0xf << 16);
	val |= (0x4 << 16);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - PFD */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~(0x3 << 20);
	val |= (0x1 << 20);
	val &= ~(0x3 << 24);
	val |= (0x1 << 24);
	val &= ~BIT(26);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - POSTDIV */
	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val |= BIT(22);
	val &= ~BIT(27);
	val &= ~BIT(28);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	/* PLL - SDM */
	val = an8855_reg_read(gsw, PLL_CTRL_4);
	val &= ~BITS(3, 4);
	an8855_reg_write(gsw, PLL_CTRL_4, val);

	val = an8855_reg_read(gsw, PLL_CTRL_2);
	val &= ~BIT(30);
	an8855_reg_write(gsw, PLL_CTRL_2, val);

	val = an8855_reg_read(gsw, SS_LCPLL_PWCTL_SETTING_2);
	val &= ~(0x3 << 16);
	val |= (0x1 << 16);
	an8855_reg_write(gsw, SS_LCPLL_PWCTL_SETTING_2, val);

	an8855_reg_write(gsw, SS_LCPLL_TDC_FLT_2, 0x48000000);
	an8855_reg_write(gsw, SS_LCPLL_TDC_PCW_1, 0x48000000);

	val = an8855_reg_read(gsw, SS_LCPLL_TDC_FLT_5);
	val &= ~BIT(24);
	an8855_reg_write(gsw, SS_LCPLL_TDC_FLT_5, val);

	val = an8855_reg_read(gsw, PLL_CK_CTRL_0);
	val &= ~BIT(8);
	an8855_reg_write(gsw, PLL_CK_CTRL_0, val);

	/* PLL - SS */
	val = an8855_reg_read(gsw, PLL_CTRL_3);
	val &= ~BITS(0, 15);
	an8855_reg_write(gsw, PLL_CTRL_3, val);

	val = an8855_reg_read(gsw, PLL_CTRL_4);
	val &= ~BITS(0, 1);
	an8855_reg_write(gsw, PLL_CTRL_4, val);

	val = an8855_reg_read(gsw, PLL_CTRL_3);
	val &= ~BITS(16, 31);
	an8855_reg_write(gsw, PLL_CTRL_3, val);

	/* PLL - TDC */
	val = an8855_reg_read(gsw, PLL_CK_CTRL_0);
	val &= ~BIT(9);
	an8855_reg_write(gsw, PLL_CK_CTRL_0, val);

	val = an8855_reg_read(gsw, RG_QP_PLL_SDM_ORD);
	val |= BIT(3);
	val |= BIT(4);
	an8855_reg_write(gsw, RG_QP_PLL_SDM_ORD, val);

	val = an8855_reg_read(gsw, RG_QP_RX_DAC_EN);
	val &= ~(0x3 << 16);
	val |= (0x2 << 16);
	an8855_reg_write(gsw, RG_QP_RX_DAC_EN, val);

	/* PLL - TCL Disable (only for Co-SIM) */
	val = an8855_reg_read(gsw, PON_RXFEDIG_CTRL_0);
	val &= ~BIT(12);
	an8855_reg_write(gsw, PON_RXFEDIG_CTRL_0, val);

	/* TX Init */
	val = an8855_reg_read(gsw, RG_QP_TX_MODE_16B_EN);
	val &= ~BIT(0);
	val &= ~BITS(16, 31);
	an8855_reg_write(gsw, RG_QP_TX_MODE_16B_EN, val);

	/* RX Init */
	val = an8855_reg_read(gsw, RG_QP_RXAFE_RESERVE);
	val |= BIT(11);
	an8855_reg_write(gsw, RG_QP_RXAFE_RESERVE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_LPF_MJV_LIM);
	val &= ~(0x3 << 4);
	val |= (0x2 << 4);
	an8855_reg_write(gsw, RG_QP_CDR_LPF_MJV_LIM, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_LPF_SETVALUE);
	val &= ~(0xf << 25);
	val |= (0x1 << 25);
	val &= ~(0x7 << 29);
	val |= (0x6 << 29);
	an8855_reg_write(gsw, RG_QP_CDR_LPF_SETVALUE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_CKREF_DIV1);
	val &= ~(0x1f << 8);
	val |= (0xc << 8);
	an8855_reg_write(gsw, RG_QP_CDR_PR_CKREF_DIV1, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE);
	val &= ~(0x3f << 0);
	val |= (0x19 << 0);
	val &= ~BIT(6);
	an8855_reg_write(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_FORCE_IBANDLPF_R_OFF);
	val &= ~(0x7f << 6);
	val |= (0x21 << 6);
	val &= ~(0x3 << 16);
	val |= (0x2 << 16);
	val &= ~BIT(13);
	an8855_reg_write(gsw, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE);
	val &= ~BIT(30);
	an8855_reg_write(gsw, RG_QP_CDR_PR_KBAND_DIV_PCIE, val);

	val = an8855_reg_read(gsw, RG_QP_CDR_PR_CKREF_DIV1);
	val &= ~(0x7 << 24);
	val |= (0x4 << 24);
	an8855_reg_write(gsw, RG_QP_CDR_PR_CKREF_DIV1, val);

	/* PMA (For HW Mode) */
	val = an8855_reg_read(gsw, RX_CTRL_26);
	val |= BIT(23);
	val &= ~BIT(24);
		val |= BIT(26);

	an8855_reg_write(gsw, RX_CTRL_26, val);

	val = an8855_reg_read(gsw, RX_DLY_0);
	val &= ~(0xff << 0);
	val |= (0x6f << 0);
	val |= BITS(8, 13);
	an8855_reg_write(gsw, RX_DLY_0, val);

	val = an8855_reg_read(gsw, RX_CTRL_42);
	val &= ~(0x1fff << 0);
	val |= (0x150 << 0);
	an8855_reg_write(gsw, RX_CTRL_42, val);

	val = an8855_reg_read(gsw, RX_CTRL_2);
	val &= ~(0x1fff << 16);
	val |= (0x150 << 16);
	an8855_reg_write(gsw, RX_CTRL_2, val);

	val = an8855_reg_read(gsw, PON_RXFEDIG_CTRL_9);
	val &= ~(0x7 << 0);
	val |= (0x1 << 0);
	an8855_reg_write(gsw, PON_RXFEDIG_CTRL_9, val);

	val = an8855_reg_read(gsw, RX_CTRL_8);
	val &= ~(0xfff << 16);
	val |= (0x200 << 16);
	val &= ~(0x7fff << 0);
	val |= (0xfff << 0);
	an8855_reg_write(gsw, RX_CTRL_8, val);

	/* Frequency memter */
	val = an8855_reg_read(gsw, RX_CTRL_5);
	val &= ~(0xfffff << 10);
	val |= (0x28 << 10);
	an8855_reg_write(gsw, RX_CTRL_5, val);

	val = an8855_reg_read(gsw, RX_CTRL_6);
	val &= ~(0xfffff << 0);
	val |= (0x64 << 0);
	an8855_reg_write(gsw, RX_CTRL_6, val);

	val = an8855_reg_read(gsw, RX_CTRL_7);
	val &= ~(0xfffff << 0);
	val |= (0x2710 << 0);
	an8855_reg_write(gsw, RX_CTRL_7, val);

	val = an8855_reg_read(gsw, PLL_CTRL_0);
	val |= BIT(0);
	an8855_reg_write(gsw, PLL_CTRL_0, val);

	if (mode == SGMII_MODE_FORCE) {
		/* PCS Init */
		val = an8855_reg_read(gsw, QP_DIG_MODE_CTRL_0);
		val &= ~BIT(0);
		val &= ~BITS(4, 5);
		an8855_reg_write(gsw, QP_DIG_MODE_CTRL_0, val);

		val = an8855_reg_read(gsw, RG_HSGMII_PCS_CTROL_1);
		val &= ~BIT(30);
		an8855_reg_write(gsw, RG_HSGMII_PCS_CTROL_1, val);

		/* Rate Adaption - GMII path config. */
		val = an8855_reg_read(gsw, RG_AN_SGMII_MODE_FORCE);
		val |= BIT(0);
		val &= ~BITS(4, 5);
		an8855_reg_write(gsw, RG_AN_SGMII_MODE_FORCE, val);

		val = an8855_reg_read(gsw, SGMII_STS_CTRL_0);
		val |= BIT(2);
		val &= ~(0x3 << 4);
		val |= (0x2 << 4);
		an8855_reg_write(gsw, SGMII_STS_CTRL_0, val);

		val = an8855_reg_read(gsw, SGMII_REG_AN0);
		val &= ~BIT(12);
		an8855_reg_write(gsw, SGMII_REG_AN0, val);

		val = an8855_reg_read(gsw, PHY_RX_FORCE_CTRL_0);
		val |= BIT(4);
		an8855_reg_write(gsw, PHY_RX_FORCE_CTRL_0, val);

		val = an8855_reg_read(gsw, RATE_ADP_P0_CTRL_0);
		val &= ~BITS(0, 3);
		val |= BIT(28);
		an8855_reg_write(gsw, RATE_ADP_P0_CTRL_0, val);

		val = an8855_reg_read(gsw, RG_RATE_ADAPT_CTRL_0);
		val |= BIT(0);
		val |= BIT(4);
		val |= BITS(26, 27);
		an8855_reg_write(gsw, RG_RATE_ADAPT_CTRL_0, val);
	} else {
		/* PCS Init */
		val = an8855_reg_read(gsw, RG_HSGMII_PCS_CTROL_1);
		val &= ~BIT(30);
		an8855_reg_write(gsw, RG_HSGMII_PCS_CTROL_1, val);

		/* Set AN Ability - Interrupt */
		val = an8855_reg_read(gsw, SGMII_REG_AN_FORCE_CL37);
		val |= BIT(0);
		an8855_reg_write(gsw, SGMII_REG_AN_FORCE_CL37, val);

		val = an8855_reg_read(gsw, SGMII_REG_AN_13);
		val &= ~(0x3f << 0);
		val |= (0xb << 0);
		val |= BIT(8);
		an8855_reg_write(gsw, SGMII_REG_AN_13, val);

		/* Rate Adaption - GMII path config. */
		val = an8855_reg_read(gsw, SGMII_REG_AN0);
		val |= BIT(12);
		an8855_reg_write(gsw, SGMII_REG_AN0, val);

		val = an8855_reg_read(gsw, MII_RA_AN_ENABLE);
		val |= BIT(0);
		an8855_reg_write(gsw, MII_RA_AN_ENABLE, val);

		val = an8855_reg_read(gsw, RATE_ADP_P0_CTRL_0);
		val |= BIT(28);
		an8855_reg_write(gsw, RATE_ADP_P0_CTRL_0, val);

		val = an8855_reg_read(gsw, RG_RATE_ADAPT_CTRL_0);
		val |= BIT(0);
		val |= BIT(4);
		val |= BITS(26, 27);
		an8855_reg_write(gsw, RG_RATE_ADAPT_CTRL_0, val);

		/* Only for Co-SIM */

		/* AN Speed up (Only for Co-SIM) */

		/* Restart AN */
		val = an8855_reg_read(gsw, SGMII_REG_AN0);
		val |= BIT(9);
		an8855_reg_write(gsw, SGMII_REG_AN0, val);
	}

	/* bypass flow control to MAC */
	an8855_reg_write(gsw, MSG_RX_LIK_STS_0, 0x01010107);
	an8855_reg_write(gsw, MSG_RX_LIK_STS_2, 0x00000EEF);

	return 0;
}

static int an8855_set_port_rmii(struct gsw_an8855 *gsw)
{
	an8855_reg_write(gsw, RG_P5MUX_MODE, 0x301);
	an8855_reg_write(gsw, RG_FORCE_CKDIR_SEL, 0x101);
	an8855_reg_write(gsw, RG_SWITCH_MODE, 0x101);
	an8855_reg_write(gsw, RG_FORCE_MAC5_SB, 0x1010101);
	an8855_reg_write(gsw, CSR_RMII, 0x420102);
	an8855_reg_write(gsw, RG_RGMII_TXCK_C, 0x1100910);
	return 0;
}

static int an8855_set_port_rgmii(struct gsw_an8855 *gsw)
{
	an8855_reg_write(gsw, RG_FORCE_MAC5_SB, 0x20101);
	return 0;
}

static int an8855_mac_port_setup(struct gsw_an8855 *gsw, u32 port,
				 struct an8855_port_cfg *port_cfg)
{
	u32 pmcr;

	if (port != 5) {
		dev_info(gsw->dev, "port %d is not a MAC port\n", port);
		return -EINVAL;
	}

	if (port_cfg->enabled) {
		pmcr = an8855_reg_read(gsw, PMCR(5));

		switch (port_cfg->phy_mode) {
		case PHY_INTERFACE_MODE_RMII:
			pmcr &= ~FORCE_SPD;
			pmcr |= FORCE_MODE | (MAC_SPD_100 << 28) | FORCE_DPX
				| FORCE_LNK | FORCE_TX_FC | FORCE_RX_FC;
			an8855_set_port_rmii(gsw);
			break;
		case PHY_INTERFACE_MODE_RGMII:
			pmcr &= ~FORCE_SPD;
			pmcr |= FORCE_MODE | (MAC_SPD_1000 << 28) | FORCE_DPX
				| FORCE_LNK | FORCE_TX_FC | FORCE_RX_FC;
			an8855_set_port_rgmii(gsw);
			break;
		case PHY_INTERFACE_MODE_SGMII:
			if (port_cfg->force_link) {
				pmcr &= ~FORCE_SPD;
				pmcr |= FORCE_MODE | (MAC_SPD_1000 << 28)
					 | FORCE_DPX | FORCE_LNK | FORCE_TX_FC
					 | FORCE_RX_FC;
				an8855_sgmii_setup(gsw, SGMII_MODE_FORCE);
			} else
				an8855_sgmii_setup(gsw, SGMII_MODE_AN);
			break;
		case PHY_INTERFACE_MODE_2500BASEX:
			pmcr &= ~FORCE_SPD;
			pmcr |= FORCE_MODE | (MAC_SPD_2500 << 28) | FORCE_DPX
				| FORCE_LNK | FORCE_TX_FC | FORCE_RX_FC;
			an8855_set_hsgmii_mode(gsw);
			break;
		default:
			dev_info(gsw->dev, "%s is not supported by port %d\n",
				 phy_modes(port_cfg->phy_mode), port);
		}

		/* disable eee on cpu port */
		pmcr &= ~(FORCE_EEE100 | FORCE_EEE1G | FORCE_EEE2P5G);

		if (port_cfg->force_link)
			an8855_reg_write(gsw, PMCR(port), pmcr);
	}

	return 0;
}

static int an8855_sw_detect(struct gsw_an8855 *gsw, struct chip_rev *crev)
{
	u32 id, rev;

	id = an8855_reg_read(gsw, CHIP_ID);
	rev = an8855_reg_read(gsw, CHIP_REV);
	if (id == AN8855) {
		if (crev) {
			crev->rev = rev;
			crev->name = "AN8855";
		}

		return 0;
	}

	return -ENODEV;
}

static void an8855_phy_setting(struct gsw_an8855 *gsw)
{
	int i, j;
	u8 shift_sel = 0, rsel_tx_a = 0, rsel_tx_b = 0;
	u8 rsel_tx_c = 0, rsel_tx_d = 0;
	u16 cl45_data = 0;
	u32 val;

	/* Release power down */
	an8855_reg_write(gsw, RG_GPHY_AFE_PWD, 0x0);

	for (i = 0; i < AN8855_NUM_PHYS; i++) {
		/* Enable HW auto downshift */
		gsw->mii_write(gsw, i, 0x1f, 0x1);
		val = gsw->mii_read(gsw, i, PHY_EXT_REG_14);
		val |= PHY_EN_DOWN_SHFIT;
		gsw->mii_write(gsw, i, PHY_EXT_REG_14, val);
		gsw->mii_write(gsw, i, 0x1f, 0x0);

		/* Enable Asymmetric Pause Capability */
		val = gsw->mii_read(gsw, i, MII_ADVERTISE);
		val |= ADVERTISE_PAUSE_ASYM;
		gsw->mii_write(gsw, i, MII_ADVERTISE, val);
	}

	if (gsw->extSurge) {
		for (i = 0; i < AN8855_NUM_PHYS; i++) {
			/* Read data */
			for (j = 0; j < AN8855_WORD_SIZE; j++) {
				val = an8855_reg_read(gsw, AN8855_EFUSE_DATA0 +
					(AN8855_WORD_SIZE * (3 + j + (4 * i))));

				shift_sel = shift_check((val & 0x7f000000) >> 24);
				switch (j) {
				case 0:
					rsel_tx_a = get_shift_val(shift_sel);
					break;
				case 1:
					rsel_tx_b = get_shift_val(shift_sel);
					break;
				case 2:
					rsel_tx_c = get_shift_val(shift_sel);
					break;
				case 3:
					rsel_tx_d = get_shift_val(shift_sel);
					break;
				default:
					continue;
				}
			}
			cl45_data = gsw->mmd_read(gsw, i, PHY_DEV1E, 0x174);
			cl45_data &= ~(0x7f7f);
			cl45_data |= (rsel_tx_a << 8);
			cl45_data |= rsel_tx_b;
			gsw->mmd_write(gsw, i, PHY_DEV1E, 0x174, cl45_data);
			cl45_data = gsw->mmd_read(gsw, i, PHY_DEV1E, 0x175);
			cl45_data &= ~(0x7f7f);
			cl45_data |= (rsel_tx_c << 8);
			cl45_data |= rsel_tx_d;
			gsw->mmd_write(gsw, i, PHY_DEV1E, 0x175, cl45_data);
		}
	}
}

static void an8855_eee_setting(struct gsw_an8855 *gsw, u32 port)
{
	/* Disable EEE */
	gsw->mmd_write(gsw, port, PHY_DEV07, PHY_DEV07_REG_03C, 0);
}

static int an8855_led_set_usr_def(struct gsw_an8855 *gsw, u8 entity,
		int polar, u16 on_evt, u16 blk_evt, u8 led_freq)
{
	u32 cl45_data = 0;

	if (polar == LED_HIGH)
		on_evt |= LED_ON_POL;
	else
		on_evt &= ~LED_ON_POL;

	/* LED on event */
	gsw->mmd_write(gsw, (entity / 4), PHY_DEV1E,
		PHY_SINGLE_LED_ON_CTRL(entity % 4), on_evt | LED_ON_EN);

	/* LED blink event */
	gsw->mmd_write(gsw, (entity / 4), PHY_DEV1E,
		PHY_SINGLE_LED_BLK_CTRL(entity % 4), blk_evt);

	/* LED freq */
	switch (led_freq) {
	case AIR_LED_BLK_DUR_32M:
		cl45_data = 0x30e;
		break;
	case AIR_LED_BLK_DUR_64M:
		cl45_data = 0x61a;
		break;
	case AIR_LED_BLK_DUR_128M:
		cl45_data = 0xc35;
		break;
	case AIR_LED_BLK_DUR_256M:
		cl45_data = 0x186a;
		break;
	case AIR_LED_BLK_DUR_512M:
		cl45_data = 0x30d4;
		break;
	case AIR_LED_BLK_DUR_1024M:
		cl45_data = 0x61a8;
		break;
	default:
		break;
	}
	gsw->mmd_write(gsw, (entity / 4), PHY_DEV1E,
		PHY_SINGLE_LED_BLK_DUR(entity % 4), cl45_data);

	gsw->mmd_write(gsw, (entity / 4), PHY_DEV1E,
		PHY_SINGLE_LED_ON_DUR(entity % 4), (cl45_data >> 1));

	/* Disable DATA & BAD_SSD for port LED blink behavior */
	cl45_data = gsw->mmd_read(gsw, (entity / 4), PHY_DEV1E,
		PHY_PMA_CTRL);
	cl45_data &= ~BIT(0);
	cl45_data &= ~BIT(15);
	gsw->mmd_write(gsw, (entity / 4), PHY_DEV1E,
		PHY_PMA_CTRL, cl45_data);

	return 0;
}

static int an8855_led_set_mode(struct gsw_an8855 *gsw, u8 mode)
{
	u16 cl45_data;

	cl45_data = gsw->mmd_read(gsw, 0, PHY_DEV1F, PHY_LED_BCR);
	switch (mode) {
	case AN8855_LED_MODE_DISABLE:
		cl45_data &= ~LED_BCR_EXT_CTRL;
		cl45_data &= ~LED_BCR_MODE_MASK;
		cl45_data |= LED_BCR_MODE_DISABLE;
		break;
	case AN8855_LED_MODE_USER_DEFINE:
		cl45_data |= LED_BCR_EXT_CTRL;
		cl45_data |= LED_BCR_CLK_EN;
		break;
	default:
		dev_info(gsw->dev, "LED mode%d is not supported!\n", mode);
		return -EINVAL;
	}
	gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_LED_BCR, cl45_data);

	return 0;
}

static int an8855_led_set_state(struct gsw_an8855 *gsw, u8 entity, u8 state)
{
	u16 cl45_data = 0;

	/* Change to per port contorl */
	cl45_data = gsw->mmd_read(gsw, (entity / 4), PHY_DEV1E,
		PHY_LED_CTRL_SELECT);

	if (state == 1)
		cl45_data |= (1 << (entity % 4));
	else
		cl45_data &= ~(1 << (entity % 4));
	gsw->mmd_write(gsw, (entity / 4), PHY_DEV1E,
		PHY_LED_CTRL_SELECT, cl45_data);

	/* LED enable setting */
	cl45_data = gsw->mmd_read(gsw, (entity / 4),
		PHY_DEV1E, PHY_SINGLE_LED_ON_CTRL(entity % 4));

	if (state == 1)
		cl45_data |= LED_ON_EN;
	else
		cl45_data &= ~LED_ON_EN;

	gsw->mmd_write(gsw, (entity / 4), PHY_DEV1E,
		PHY_SINGLE_LED_ON_CTRL(entity % 4), cl45_data);

	return 0;
}

static int an8855_led_init(struct gsw_an8855 *gsw)
{
	u32 val, led_count = ARRAY_SIZE(led_cfg);
	int ret = 0, id;
	u32 tmp_val = 0;
	u32 tmp_id = 0;

	ret = an8855_led_set_mode(gsw, AN8855_LED_MODE_USER_DEFINE);
	if (ret != 0) {
		dev_info(gsw->dev, "led_set_mode fail(ret:%d)!\n", ret);
		return ret;
	}

	for (id = 0; id < led_count; id++) {
		ret = an8855_led_set_state(gsw,
			led_cfg[id].phy_led_idx, led_cfg[id].en);
		if (ret != 0) {
			dev_info(gsw->dev, "led_set_state fail(ret:%d)!\n", ret);
			return ret;
		}
		if (led_cfg[id].en == 1) {
			ret = an8855_led_set_usr_def(gsw,
				led_cfg[id].phy_led_idx,
				led_cfg[id].pol, led_cfg[id].on_cfg,
				led_cfg[id].blk_cfg,
				led_cfg[id].led_freq);
			if (ret != 0) {
				dev_info(gsw->dev, "led_set_usr_def fail!\n");
				return ret;
			}
		}
	}

	/* Setting for System LED & Loop LED */
	an8855_reg_write(gsw, RG_GPIO_OE, 0x0);
	an8855_reg_write(gsw, RG_GPIO_CTRL, 0x0);
	val = 0;
	an8855_reg_write(gsw, RG_GPIO_L_INV, val);

	val = 0x1001;
	an8855_reg_write(gsw, RG_GPIO_CTRL, val);
	val = an8855_reg_read(gsw, RG_GPIO_DATA);
	val |= BITS(1, 3);
	val &= ~(BIT(0));
	val &= ~(BIT(6));

	an8855_reg_write(gsw, RG_GPIO_DATA, val);
	val = an8855_reg_read(gsw, RG_GPIO_OE);
	val |= 0x41;
	an8855_reg_write(gsw, RG_GPIO_OE, val);

	/* Mapping between GPIO & LED */
	val = 0;
	for (id = 0; id < led_count; id++) {
		/* Skip GPIO6, due to GPIO6 does not support PORT LED */
		if (id == 6)
			continue;

		if (led_cfg[id].en == 1) {
			if (id < 7)
				val |= led_cfg[id].phy_led_idx << ((id % 4) * 8);
			else
				val |= led_cfg[id].phy_led_idx << (((id - 1) % 4) * 8);
		}

		if (id < 7)
			tmp_id = id;
		else
			tmp_id = id - 1;

		if ((tmp_id % 4) == 0x3) {
			an8855_reg_write(gsw, RG_GPIO_LED_SEL(tmp_id / 4), val);
			tmp_val = an8855_reg_read(gsw, RG_GPIO_LED_SEL(tmp_id / 4));
			val = 0;
		}
	}

	/* Turn on LAN LED mode */
	val = 0;
	for (id = 0; id < led_count; id++) {
		if (led_cfg[id].en == 1)
			val |= 0x1 << id;
	}
	an8855_reg_write(gsw, RG_GPIO_LED_MODE, val);

	/* Force clear blink pulse for per port LED */
	gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_LED_BLINK_DUR_CTRL, 0x1f);
	usleep_range(1000, 5000);
	gsw->mmd_write(gsw, 0, PHY_DEV1F, PHY_LED_BLINK_DUR_CTRL, 0);

	return 0;
}

static int an8855_sw_init(struct gsw_an8855 *gsw)
{
	int i, ret = 0;
	u32 val, led_count = ARRAY_SIZE(led_cfg);
	int id;

	gsw->phy_base = gsw->smi_addr & AN8855_SMI_ADDR_MASK;

	gsw->mii_read = an8855_mii_read;
	gsw->mii_write = an8855_mii_write;
	gsw->mmd_read = an8855_mmd_read;
	gsw->mmd_write = an8855_mmd_write;

	/* Force MAC link down before reset */
	an8855_reg_write(gsw, PMCR(5), FORCE_MODE);

	/* Switch soft reset */
	an8855_reg_write(gsw, SYS_CTRL, SW_SYS_RST);
	usleep_range(100000, 110000);

	/* Change gphy smi address */
	if (gsw->new_smi_addr != gsw->smi_addr) {
		an8855_reg_write(gsw, RG_GPHY_SMI_ADDR, gsw->new_smi_addr);
		gsw->smi_addr = gsw->new_smi_addr;
		gsw->phy_base = gsw->new_smi_addr;
	}

	for (i = 0; i < AN8855_NUM_PHYS; i++) {
		val = gsw->mii_read(gsw, i, MII_BMCR);
		val |= BMCR_ISOLATE;
		gsw->mii_write(gsw, i, MII_BMCR, val);
	}

	/* AN8855H need to setup before switch init */
	val = an8855_reg_read(gsw, PKG_SEL);
	if ((val & 0x7) == PAG_SEL_AN8855H) {

		/* Invert for LED activity change */
		val = an8855_reg_read(gsw, RG_GPIO_L_INV);
		for (id = 0; id < led_count; id++) {
			if ((led_cfg[id].pol == LED_HIGH) &&
				(led_cfg[id].en == 1))
				val |= 0x1 << id;
		}
		an8855_reg_write(gsw, RG_GPIO_L_INV, (val | 0x1));

		/* MCU NOP CMD */
		an8855_reg_write(gsw, RG_GDMP_RAM, 0x846);
		an8855_reg_write(gsw, RG_GDMP_RAM + 4, 0x4a);

		/* Enable MCU */
		val = an8855_reg_read(gsw, RG_CLK_CPU_ICG);
		an8855_reg_write(gsw, RG_CLK_CPU_ICG, val | MCU_ENABLE);
		usleep_range(1000, 5000);

		/* Disable MCU watchdog */
		val = an8855_reg_read(gsw, RG_TIMER_CTL);
		an8855_reg_write(gsw, RG_TIMER_CTL, (val & (~WDOG_ENABLE)));

		/* Configure interrupt */
		an8855_reg_write(gsw, RG_INTB_MODE, (0x1 << gsw->intr_pin));

		/* LED settings for T830 reference board */
		ret = an8855_led_init(gsw);
		if (ret < 0) {
			dev_info(gsw->dev, "an8855_led_init fail. (ret=%d)\n", ret);
			return ret;
		}
	}

	/* Adjust to reduce noise */
	for (i = 0; i < AN8855_NUM_PHYS; i++) {
		gsw->mmd_write(gsw, i, PHY_DEV1E,
			PHY_TX_PAIR_DLY_SEL_GBE, 0x4040);

		gsw->mmd_write(gsw, i, PHY_DEV1E,
			PHY_RXADC_CTRL, 0x1010);

		gsw->mmd_write(gsw, i, PHY_DEV1E,
			PHY_RXADC_REV_0, 0x100);

		gsw->mmd_write(gsw, i, PHY_DEV1E,
			PHY_RXADC_REV_1, 0x100);
	}

	/* Setup SERDES port 5 */
	an8855_mac_port_setup(gsw, 5, &gsw->port5_cfg);

	/* Global mac control settings */
	val = an8855_reg_read(gsw, GMACCR);
	val |= (15 << MAX_RX_JUMBO_S) | RX_PKT_LEN_MAX_JUMBO;
	an8855_reg_write(gsw, GMACCR, val);

	val = an8855_reg_read(gsw, CKGCR);
	val &= ~(CKG_LNKDN_GLB_STOP | CKG_LNKDN_PORT_STOP);
	an8855_reg_write(gsw, CKGCR, val);

	return 0;
}

static int an8855_sw_post_init(struct gsw_an8855 *gsw)
{
	int i;
	u32 val;

	for (i = 0; i < AN8855_NUM_PHYS; i++) {
		val = gsw->mii_read(gsw, i, MII_BMCR);
		val &= ~BMCR_ISOLATE;
		gsw->mii_write(gsw, i, MII_BMCR, val);
	}

	an8855_phy_setting(gsw);

	for (i = 0; i < AN8855_NUM_PHYS; i++)
		an8855_eee_setting(gsw, i);

	/* PHY restart AN*/
	for (i = 0; i < AN8855_NUM_PHYS; i++)
		gsw->mii_write(gsw, i, MII_BMCR, 0x1240);

	return 0;
}

struct an8855_sw_id an8855_id = {
	.model = AN8855,
	.detect = an8855_sw_detect,
	.init = an8855_sw_init,
	.post_init = an8855_sw_post_init
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Min Yao <min.yao@airoha.com>");
MODULE_DESCRIPTION("Driver for Airoha AN8855 Gigabit Switch");
