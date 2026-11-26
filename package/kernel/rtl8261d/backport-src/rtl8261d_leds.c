// SPDX-License-Identifier: GPL-2.0+
#include "rtl8261d.h"

#if IS_ENABLED(CONFIG_PHYLIB_LEDS)
int rtl8261x_led_hw_is_supported(struct phy_device *phydev, u8 index,
				 unsigned long rules)
{
	const unsigned long mask = BIT(TRIGGER_NETDEV_LINK) |
				   BIT(TRIGGER_NETDEV_LINK_10) |
				   BIT(TRIGGER_NETDEV_LINK_100) |
				   BIT(TRIGGER_NETDEV_LINK_1000) |
				   BIT(TRIGGER_NETDEV_LINK_2500) |
				   BIT(TRIGGER_NETDEV_LINK_5000) |
				   BIT(TRIGGER_NETDEV_LINK_10000) |
				   BIT(TRIGGER_NETDEV_TX) |
				   BIT(TRIGGER_NETDEV_RX);

	(void)phydev;
	if (index >= RTL8261C_NR_LEDS)
		return -EINVAL;

	if (rules & ~mask)
		return -EOPNOTSUPP;

	/* Hardware combines TX+RX activity into one blink bit */
	if (!!(rules & BIT(TRIGGER_NETDEV_RX)) ^
	    !!(rules & BIT(TRIGGER_NETDEV_TX)))
		return -EOPNOTSUPP;

	return 0;
}

int rtl8261x_led_hw_control_get(struct phy_device *phydev, u8 index,
				unsigned long *rules)
{
	u16 reg = RTL8261C_LED_LINK_SELECT +
		  index * RTL8261C_LED_LINK_SELECT_OFFSET;
	int val;

	if (index >= RTL8261C_NR_LEDS || !rules)
		return -EINVAL;

	*rules = 0;

	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, reg);
	if (val < 0)
		return val;

	if (val & RTL8261C_LEDCR_LINK_10)
		__set_bit(TRIGGER_NETDEV_LINK_10, rules);
	if (val & RTL8261C_LEDCR_LINK_100)
		__set_bit(TRIGGER_NETDEV_LINK_100, rules);
	if (val & RTL8261C_LEDCR_LINK_1000)
		__set_bit(TRIGGER_NETDEV_LINK_1000, rules);
	if (val & RTL8261C_LEDCR_LINK_2500)
		__set_bit(TRIGGER_NETDEV_LINK_2500, rules);
	if (val & RTL8261C_LEDCR_LINK_5000)
		__set_bit(TRIGGER_NETDEV_LINK_5000, rules);
	if (val & RTL8261C_LEDCR_LINK_10000)
		__set_bit(TRIGGER_NETDEV_LINK_10000, rules);

	if ((val & (RTL8261C_LEDCR_LINK_10 | RTL8261C_LEDCR_LINK_100 |
		    RTL8261C_LEDCR_LINK_1000 | RTL8261C_LEDCR_LINK_2500 |
		    RTL8261C_LEDCR_LINK_5000 | RTL8261C_LEDCR_LINK_10000)) ==
	    (RTL8261C_LEDCR_LINK_10 | RTL8261C_LEDCR_LINK_100 |
	     RTL8261C_LEDCR_LINK_1000 | RTL8261C_LEDCR_LINK_2500 |
	     RTL8261C_LEDCR_LINK_5000 | RTL8261C_LEDCR_LINK_10000))
		__set_bit(TRIGGER_NETDEV_LINK, rules);

	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8261C_LED_ACT_SELECT);
	if (val < 0)
		return val;

	if (val & BIT(index)) {
		__set_bit(TRIGGER_NETDEV_TX, rules);
		__set_bit(TRIGGER_NETDEV_RX, rules);
	}

	return 0;
}

int rtl8261x_led_hw_control_set(struct phy_device *phydev, u8 index,
				unsigned long rules)
{
	struct rtl8261x_priv *priv = phydev->priv;
	u16 reg = RTL8261C_LED_LINK_SELECT +
		  index * RTL8261C_LED_LINK_SELECT_OFFSET;
	u16 ecr_mask = BIT(index) | BIT(index + 4);
	u16 ecr_val, val = 0;
	int ret;

	if (index >= RTL8261C_NR_LEDS)
		return -EINVAL;

	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_10, &rules))
		val |= RTL8261C_LEDCR_LINK_10;
	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_100, &rules))
		val |= RTL8261C_LEDCR_LINK_100;
	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_1000, &rules))
		val |= RTL8261C_LEDCR_LINK_1000;
	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_2500, &rules))
		val |= RTL8261C_LEDCR_LINK_2500;
	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_5000, &rules))
		val |= RTL8261C_LEDCR_LINK_5000;
	if (test_bit(TRIGGER_NETDEV_LINK, &rules) ||
	    test_bit(TRIGGER_NETDEV_LINK_10000, &rules))
		val |= RTL8261C_LEDCR_LINK_10000;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, reg, val);
	if (ret < 0)
		return ret;

	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, RTL8261C_LED_ACT_SELECT,
			     BIT(index),
			     (test_bit(TRIGGER_NETDEV_TX, &rules) ||
			      test_bit(TRIGGER_NETDEV_RX, &rules)) ?
			     BIT(index) : 0);
	if (ret < 0)
		return ret;

	ecr_val = BIT(index + 4);
	if (!(priv && (priv->led_active_low & BIT(index))))
		ecr_val |= BIT(index);

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2,
			      RTL8261C_LED_POLARITY_SELECT, ecr_mask, ecr_val);
}

int rtl8261x_led_brightness_set(struct phy_device *phydev, u8 index,
				enum led_brightness brightness)
{
	struct rtl8261x_priv *priv = phydev->priv;
	u16 reg = RTL8261C_LED_LINK_SELECT +
		  index * RTL8261C_LED_LINK_SELECT_OFFSET;
	u16 ecr_mask = BIT(index) | BIT(index + 4);
	bool active_low = priv && (priv->led_active_low & BIT(index));
	bool active = (brightness != LED_OFF) ^ active_low;
	u16 ecr_val;
	int ret;

	if (index >= RTL8261C_NR_LEDS)
		return -EINVAL;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, reg, 0);
	if (ret < 0)
		return ret;

	ret = phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2,
				 RTL8261C_LED_ACT_SELECT, BIT(index));
	if (ret < 0)
		return ret;

	ecr_val = BIT(index + 4) | (active ? BIT(index) : 0);
	return phy_modify_mmd(phydev, MDIO_MMD_VEND2,
			      RTL8261C_LED_POLARITY_SELECT, ecr_mask, ecr_val);
}

int rtl8261x_led_polarity_set(struct phy_device *phydev, int index,
			      unsigned long modes)
{
	struct rtl8261x_priv *priv = phydev->priv;
	bool active_low = false, active_high = false;
	u32 mode;

	if (!priv || index < 0 || index >= RTL8261C_NR_LEDS)
		return -EINVAL;

	for_each_set_bit(mode, &modes, __PHY_LED_MODES_NUM) {
		switch (mode) {
		case PHY_LED_ACTIVE_LOW:
			active_low = true;
			break;
		case PHY_LED_ACTIVE_HIGH:
			active_high = true;
			break;
		default:
			return -EINVAL;
		}
	}

	if (active_low == active_high)
		return -EINVAL;

	if (active_low) {
		priv->led_active_low |= BIT(index);
		return phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2,
					  RTL8261C_LED_POLARITY_SELECT,
					  BIT(index));
	}

	priv->led_active_low &= ~BIT(index);
	return phy_set_bits_mmd(phydev, MDIO_MMD_VEND2,
				RTL8261C_LED_POLARITY_SELECT,
				BIT(index));
}
#endif /* CONFIG_PHYLIB_LEDS */
