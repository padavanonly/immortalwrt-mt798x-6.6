/* SPDX-License-Identifier:	GPL-2.0+ */
/*
 * Register definitions for MediaTek MT753x Gigabit switches
 *
 * Copyright (C) 2018 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT753X_PHY_H_
#define _MT753X_PHY_H_

#include <linux/bitops.h>

/*phy calibration use*/
#define DEV_1E				0x1E
/*global device 0x1f, always set P0*/
#define DEV_1F				0x1F


/************IEXT/REXT CAL***************/
/* bits range: for example BITS(16,23) = 0xFF0000*/
#define BITS(m, n)   (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#define ANACAL_INIT			0x01
#define ANACAL_ERROR			0xFD
#define ANACAL_SATURATION		0xFE
#define	ANACAL_FINISH			0xFF
#define ANACAL_PAIR_A			0
#define ANACAL_PAIR_B			1
#define ANACAL_PAIR_C			2
#define ANACAL_PAIR_D			3
#define DAC_IN_0V			0x00
#define DAC_IN_2V			0xf0
#define TX_AMP_OFFSET_0MV		0x20
#define TX_AMP_OFFSET_VALID_BITS	6

#define R0				0
#define PHY0				0
#define PHY1				1
#define PHY2				2
#define PHY3				3
#define PHY4				4
#define ANA_TEST_MODE			BITS(8, 15)
#define TST_TCLK_SEL			BITs(6, 7)
#define ANA_TEST_VGA_RG			0x100

#define FORCE_MDI_CROSS_OVER		BITS(3, 4)
#define T10_TEST_CTL_RG			0x145
#define RG_185				0x185
#define RG_TX_SLEW			BIT(0)
#define ANA_CAL_0			0xdb
#define RG_CAL_CKINV			BIT(12)
#define RG_ANA_CALEN			BIT(8)
#define RG_REXT_CALEN			BIT(4)
#define RG_ZCALEN_A			BIT(0)
#define ANA_CAL_1			0xdc
#define RG_ZCALEN_B			BIT(12)
#define RG_ZCALEN_C			BIT(8)
#define RG_ZCALEN_D			BIT(4)
#define RG_TXVOS_CALEN			BIT(0)
#define ANA_CAL_6			0xe1
#define RG_CAL_REFSEL			BIT(4)
#define RG_CAL_COMP_PWD			BIT(0)
#define ANA_CAL_5			0xe0
#define RG_REXT_TRIM			BITs(8, 13)
#define RG_ZCAL_CTRL			BITs(0, 5)
#define RG_17A				0x17a
#define AD_CAL_COMP_OUT			BIT(8)
#define RG_17B				0x17b
#define AD_CAL_CLK			bit(0)
#define RG_17C				0x17c
#define DA_CALIN_FLAG			bit(0)
/************R50 CAL****************************/
#define RG_174				0x174
#define RG_R50OHM_RSEL_TX_A_EN		BIT[15]
#define CR_R50OHM_RSEL_TX_A		BITS[8:14]
#define RG_R50OHM_RSEL_TX_B_EN		BIT[7]
#define CR_R50OHM_RSEL_TX_B		BITS[6:0]
#define RG_175				0x175
#define RG_R50OHM_RSEL_TX_C_EN		BITS[15]
#define CR_R50OHM_RSEL_TX_C		BITS[8:14]
#define RG_R50OHM_RSEL_TX_D_EN		BIT[7]
#define CR_R50OHM_RSEL_TX_D		BITS[0:6]
/**********TX offset Calibration***************************/
#define RG_95				0x96
#define BYPASS_TX_OFFSET_CAL		BIT(15)
#define RG_3E				0x3e
#define BYPASS_PD_TXVLD_A		BIT(15)
#define BYPASS_PD_TXVLD_B		BIT(14)
#define BYPASS_PD_TXVLD_C		BIT(13)
#define BYPASS_PD_TXVLD_D		BIT(12)
#define BYPASS_PD_TX_10M		BIT(11)
#define POWER_DOWN_TXVLD_A		BIT(7)
#define POWER_DOWN_TXVLD_B		BIT(6)
#define POWER_DOWN_TXVLD_C		BIT(5)
#define POWER_DOWN_TXVLD_D		BIT(4)
#define POWER_DOWN_TX_10M		BIT(3)
#define RG_DD				0xdd
#define RG_TXG_CALEN_A			BIT(12)
#define RG_TXG_CALEN_B			BIT(8)
#define RG_TXG_CALEN_C			BIT(4)
#define RG_TXG_CALEN_D			BIT(0)
#define RG_17D				0x17D
#define FORCE_DASN_DAC_IN0_A		BIT(15)
#define DASN_DAC_IN0_A			BITS(0, 9)
#define RG_17E				0x17E
#define FORCE_DASN_DAC_IN0_B		BIT(15)
#define DASN_DAC_IN0_B			BITS(0, 9)
#define RG_17F				0x17F

#define FORCE_DASN_DAC_IN0_C		BIT(15)
#define DASN_DAC_IN0_C			BITS(0, 9)
#define RG_180				0x180
#define FORCE_DASN_DAC_IN0_D		BIT(15)
#define DASN_DAC_IN0_D			BITS(0, 9)

#define RG_181				0x181
#define FORCE_DASN_DAC_IN1_A		BIT(15)
#define DASN_DAC_IN1_A			BITS(0, 9)
#define RG_182				0x182
#define FORCE_DASN_DAC_IN1_B		BIT(15)
#define DASN_DAC_IN1_B			BITS(0, 9)
#define RG_183				0x183
#define FORCE_DASN_DAC_IN1_C		BIT15]
#define DASN_DAC_IN1_C			BITS(0, 9)
#define RG_184				0x184
#define FORCE_DASN_DAC_IN1_D		BIT(15)
#define DASN_DAC_IN1_D			BITS(0, 9)
#define RG_172				0x172
#define CR_TX_AMP_OFFSET_A		BITS(8, 13)
#define CR_TX_AMP_OFFSET_B		BITS(0, 5)
#define RG_173				0x173
#define CR_TX_AMP_OFFSET_C		BITS(8, 13)
#define CR_TX_AMP_OFFSET_D		BITS(0, 5)
/**********TX Amp Calibration ***************************/
#define RG_12				0x12
#define DA_TX_I2MPB_A_GBE		BITS(10, 15)
#define RG_17				0x17
#define DA_TX_I2MPB_B_GBE		BITS(8, 13)
#define RG_19				0x19
#define DA_TX_I2MPB_C_GBE		BITS(8, 13)
#define RG_21				0x21
#define DA_TX_I2MPB_D_GBE		BITS(8, 13)

#endif /* _MT753X_REGS_H_ */
