/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Realtek RTL8261C/CE/D 10Gbps Clause-45 PHY driver
 *
 * Primary architecture based on mainline netdev v7.3/v8 backport by Javen Xu
 * (Beaverfffan/x-wrt-push commit d721114d804711bf32bff4e61117e23c3a34f3f6),
 * augmented with USXGMII/10GBASE-R rate matching and SerDes resync, optional
 * DT SerDes polarity inversion, 120C thermal protection + hwmon, EDPD tunable,
 * anti-interference DSP calibration, and 4-channel hardware LED offload.
 */
#ifndef _RTL8261D_H_
#define _RTL8261D_H_

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/firmware.h>
#include <linux/hwmon.h>
#include <linux/leds.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/property.h>
#include <linux/version.h>

/* Clause-45 PHY IDs and indirect sub-model identification */
#define RTL_8261C_CG			0x001cc898
#define RTL_8261CE_CG			0x001cc899

#define RTL8261X_EXT_ADDR_REG		0xa436
#define RTL8261X_EXT_DATA_REG		0xa438
#define RTL_8261X_SUB_PHY_ID_ADDR	0x801d

#define RTL8261C_CE_MODEL		0x00
#define RTL8261D_MODEL			0x81

/* IEEE 802.3an 10GBASE-T Master/Slave ANEG bits */
#ifndef MDIO_AN_10GBT_CTRL_MS_ENABLE
#define MDIO_AN_10GBT_CTRL_MS_ENABLE	0x8000
#define MDIO_AN_10GBT_CTRL_MS_VALUE	0x4000
#define MDIO_AN_10GBT_CTRL_MS_PORT_TYPE	0x2000
#endif
#ifndef MDIO_AN_10GBT_STAT_MS_FAULT
#define MDIO_AN_10GBT_STAT_MS_FAULT	0x8000
#define MDIO_AN_10GBT_STAT_MS_RES	0x4000
#endif

/* C22-mapped registers in MDIO_MMD_VEND2 (31) */
#define RTL822X_VND2_C22_REG(reg)	(0xa400 + 2 * (reg))
#define RTL8261X_PHY_STATE_REG		0xa420
#define RTL8261X_PHY_STATE_MASK		GENMASK(1, 0)
#define RTL8261X_PHY_STATE_READY	0x1
#define RTL8261X_OCP_POWER_CFG		0xa430
#define RTL8261X_OCP_POWER_EDPD_EN	BIT(2)
#define RTL8261X_OCP_EEE_CFG		0xa432
#define RTL8261X_OCP_EEE_C45_EN		BIT(5)
#define RTL8261X_PHYSR_REG		0xa434
#define RTL8261X_PHYSR_LINK		BIT(2)
#define RTL8261X_PHYSR_DUPLEX		BIT(3)
#define RTL8261X_PHYSR_SPEED_L		GENMASK(5, 4)
#define RTL8261X_PHYSR_SPEED_H		GENMASK(10, 9)

/* Concatenated 4-bit speed code: (SPEED_H << 2) | SPEED_L */
#define RTL8261X_SPEED_CODE_10M		0x0
#define RTL8261X_SPEED_CODE_100M	0x1
#define RTL8261X_SPEED_CODE_500M	0x3
#define RTL8261X_SPEED_CODE_10000M_V1	0x4
#define RTL8261X_SPEED_CODE_2500M_V1	0x5
#define RTL8261X_SPEED_CODE_5000M_V1	0x6
#define RTL8261X_SPEED_CODE_1000M	0x7
#define RTL8261X_SPEED_CODE_2500M	0x8
#define RTL8261X_SPEED_CODE_5000M	0x9
#define RTL8261X_SPEED_CODE_10000M	0xa

/* Interrupt registers & masks in MDIO_MMD_VEND2 */
#define RTL8261X_IMR			0xa4d2
#define RTL8261X_ISR			0xa4d4
#define RTL8261X_INT_AUTONEG_ERROR	BIT(0)
#define RTL8261X_INT_PAGE_RECV		BIT(2)
#define RTL8261X_INT_AUTONEG_DONE	BIT(3)
#define RTL8261X_INT_LINK_CHG		BIT(4)
#define RTL8261X_INT_PHY_REG_ACCESS	BIT(5)
#define RTL8261X_INT_PME		BIT(7)
#define RTL8261X_INT_ALDPS_CHG		BIT(9)
#define RTL8261X_INT_JABBER		BIT(10)

#define RTL8261X_INT_MASK_DEFAULT	(RTL8261X_INT_AUTONEG_DONE | \
					 RTL8261X_INT_LINK_CHG | \
					 RTL8261X_INT_AUTONEG_ERROR | \
					 RTL8261X_INT_JABBER)
#define RTL8261X_INT_MASK_ALL		(RTL8261X_INT_AUTONEG_ERROR | \
					 RTL8261X_INT_PAGE_RECV | \
					 RTL8261X_INT_AUTONEG_DONE | \
					 RTL8261X_INT_LINK_CHG | \
					 RTL8261X_INT_PHY_REG_ACCESS | \
					 RTL8261X_INT_PME | \
					 RTL8261X_INT_ALDPS_CHG | \
					 RTL8261X_INT_JABBER)

/* Host SerDes USXGMII option, indirect access, and link status in MDIO_MMD_VEND1 (30) */
#define RTL8261X_VND1_SERDES_OPT4	0x6972
#define RTL8261X_VND1_SERDES_OPT_MASK	0x005f
#define RTL8261X_VND1_USXGMII_CMD	0x7587
#define RTL8261X_VND1_USXGMII_ADDR	0x7588
#define RTL8261X_VND1_USXGMII_DATA	0x7589
#define RTL8261X_VND1_SDS_RDATA		0x758a
#define RTL8261X_VND1_SDS_LINK_REG	0x758d
#define RTL8261X_SDS_LINK_UP		BIT(1)
#define RTL8261X_USXGMII_CMD_BUSY	BIT(0)
#define RTL8261X_USXGMII_CMD_EXEC_RD	0x0001
#define RTL8261X_USXGMII_CMD_EXEC_WR	0x0003
#define RTL8261X_USXGMII_AN_REG		0x00f1
#define RTL8261X_USXGMII_AN_ENABLE_VAL	0x854f
#define RTL8261X_USXGMII_AN_DISABLE_VAL	0x854e

/* Indirect SerDes polarity control registers (accessed via VND1 0x7587..0x758a) */
#define RTL8261X_SDS_8B10B_POLAR_REG	0x0000
#define RTL8261X_SDS_8B10B_TX_POLAR_INV	BIT(8)
#define RTL8261X_SDS_8B10B_RX_POLAR_INV	BIT(9)
#define RTL8261X_SDS_64B66B_POLAR_REG	0x00c2
#define RTL8261X_SDS_64B66B_RX_POLAR_INV BIT(13)
#define RTL8261X_SDS_64B66B_TX_POLAR_INV BIT(14)

/* Anti-interference DSP calibration in MDIO_MMD_VEND2 */
#define RTL8261X_VND2_DSP_CFG1		0xbf88
#define RTL8261X_VND2_DSP_CFG1_EN	BIT(2)
#define RTL8261X_VND2_DSP_CFG2		0xbf8a
#define RTL8261X_VND2_DSP_CFG2_MASK	GENMASK(15, 10)
#define RTL8261X_VND2_DSP_CFG2_VAL	BIT(13)

/* Thermal sensor & hwmon registers in MDIO_MMD_VEND2 */
#define RTL822X_VND2_TSALRM		0xa662
#define RTL822X_VND2_TSALRM_CLR_MASK	GENMASK(1, 0)
#define RTL822X_VND2_TSSR		0xb54c
#define RTL822X_VND2_TSSR_EN		BIT(3)
#define RTL822X_VND2_TSSR_THRESH_MASK	GENMASK(15, 6)
#define RTL8261X_THERMAL_DEFAULT_MAX_C	120
#define RTL822X_VND2_TSRR		0xbd84
#define RTL822X_VND2_TSRR_MASK		0x03ff

/* Hardware LED controller in MDIO_MMD_VEND2 */
#define RTL8261C_NR_LEDS		4
#define RTL8261C_LED_LINK_SELECT	0xd032
#define RTL8261C_LED_LINK_SELECT_OFFSET	0x2
#define RTL8261C_LED_ACT_SELECT		0xd040
#define RTL8261C_LED_POLARITY_SELECT	0xd044
#define RTL8261C_LEDCR_LINK_10		BIT(0)
#define RTL8261C_LEDCR_LINK_100		BIT(1)
#define RTL8261C_LEDCR_LINK_1000	BIT(2)
#define RTL8261C_LEDCR_LINK_10000	BIT(4)
#define RTL8261C_LEDCR_LINK_2500	BIT(5)
#define RTL8261C_LEDCR_LINK_5000	BIT(6)

/* Firmware loader format for RTL8261C_CG */
#define FW_MAIN_MAGIC			0x52544c38U
#define FW_SUB_MAGIC_8261C		0x32363143U
#define RTL8261X_POLL_TIMEOUT_MS	100U
#define RTL8261X_MAX_MMD_DEV		31U
#define RTL8261C_CE_FW_NAME		"rtl_nic/rtl8261c.bin"
#define RTL8261C_CE_FW_NAME_FALLBACK	"rtl8261c.bin"

enum rtl8261x_fw_op {
	OP_WRITE = 0x00,
	OP_POLL  = 0x02,
};

struct rtl8261x_fw_header {
	__le32 main_magic;
	__le32 sub_magic;
	__le16 version_major;
	__le16 version_minor;
	__le16 num_entries;
	__le16 reserved;
	__le32 crc32;
};

struct rtl8261x_fw_entry {
	u8 type;
	u8 dev;
	__le16 addr;
	u8 msb;
	u8 lsb;
	__le16 value;
	__le16 timeout_ms;
	u8 poll_set;
	u8 reserved;
};

#define FW_HEADER_SIZE	sizeof(struct rtl8261x_fw_header)
#define FW_ENTRY_SIZE	sizeof(struct rtl8261x_fw_entry)

enum rtl8261x_chip_model {
	RTL8261_MODEL_C_CE = 0,
	RTL8261_MODEL_D,
	RTL8261_MODEL_GENERIC,
};

struct rtl8261x_priv {
	const char *fw_name;
	enum rtl8261x_chip_model model;
	u8 sub_phy_id;
	u8 led_active_low;
	bool fw_loaded;
	bool last_link;
	int last_speed;
};

int rtl8261x_hwmon_init(struct phy_device *phydev);
int rtl8261x_thermal_sensor_init(struct phy_device *phydev, u8 max_temp_c);

#if IS_ENABLED(CONFIG_PHYLIB_LEDS)
int rtl8261x_led_hw_is_supported(struct phy_device *phydev, u8 index,
				 unsigned long rules);
int rtl8261x_led_hw_control_get(struct phy_device *phydev, u8 index,
				unsigned long *rules);
int rtl8261x_led_hw_control_set(struct phy_device *phydev, u8 index,
				unsigned long rules);
int rtl8261x_led_brightness_set(struct phy_device *phydev, u8 index,
				enum led_brightness brightness);
int rtl8261x_led_polarity_set(struct phy_device *phydev, int index,
			      unsigned long modes);
#endif

#endif /* _RTL8261D_H_ */
