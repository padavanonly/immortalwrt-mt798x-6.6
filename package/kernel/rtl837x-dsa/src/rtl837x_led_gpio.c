// SPDX-License-Identifier: GPL-2.0-only
#include "rtl837x.h"

static int rtl837x_gpio_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct rtl837x_priv *priv = gpiochip_get_data(gc);
	u32 val = 0;

	if (offset >= RTL837X_GPIO_MAX)
		return -EINVAL;
	if (regmap_read(priv->map, RTL837X_GPIO_DIR_0_REG, &val))
		return -EIO;

	return (val & BIT(offset)) ? GPIO_LINE_DIRECTION_OUT :
				     GPIO_LINE_DIRECTION_IN;
}

static int rtl837x_gpio_get(struct gpio_chip *gc, unsigned int offset)
{
	struct rtl837x_priv *priv = gpiochip_get_data(gc);
	u32 val = 0;
	int ret;

	ret = regmap_read(priv->map, RTL837X_GPIO_IN_0_REG, &val);
	return ret ? ret : !!(val & BIT(offset));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 17, 0)
static int rtl837x_gpio_set(struct gpio_chip *gc, unsigned int offset,
			    int value)
{
	struct rtl837x_priv *priv = gpiochip_get_data(gc);

	return regmap_update_bits(priv->map, RTL837X_GPIO_OUT_0_REG,
				  BIT(offset), value ? BIT(offset) : 0);
}
#else
static void rtl837x_gpio_set(struct gpio_chip *gc, unsigned int offset,
			     int value)
{
	struct rtl837x_priv *priv = gpiochip_get_data(gc);

	regmap_update_bits(priv->map, RTL837X_GPIO_OUT_0_REG, BIT(offset),
			   value ? BIT(offset) : 0);
}
#endif

static int rtl837x_gpio_dir_in(struct gpio_chip *gc, unsigned int offset)
{
	struct rtl837x_priv *priv = gpiochip_get_data(gc);
	int ret;

	ret = regmap_update_bits(priv->map, RTL837X_IO_MUX_SEL_0_REG,
				 BIT(offset), BIT(offset));
	if (ret)
		return ret;
	return regmap_update_bits(priv->map, RTL837X_GPIO_DIR_0_REG,
				  BIT(offset), 0);
}

static int rtl837x_gpio_dir_out(struct gpio_chip *gc, unsigned int offset,
				int value)
{
	struct rtl837x_priv *priv = gpiochip_get_data(gc);
	int ret;

	ret = regmap_update_bits(priv->map, RTL837X_IO_MUX_SEL_0_REG,
				 BIT(offset), BIT(offset));
	if (ret)
		return ret;
	ret = regmap_update_bits(priv->map, RTL837X_GPIO_OUT_0_REG,
				 BIT(offset), value ? BIT(offset) : 0);
	if (ret)
		return ret;
	return regmap_update_bits(priv->map, RTL837X_GPIO_DIR_0_REG,
				  BIT(offset), BIT(offset));
}

static int rtl837x_apply_dt_extra_init(struct rtl837x_priv *priv)
{
	struct device_node *np = priv->dev->of_node;
	int count, i, ret;

	count = of_property_count_u32_elems(np, "extra-init");
	if (count <= 0)
		return 0;
	if (count % 3 != 0)
		return -EINVAL;

	for (i = 0; i < count; i += 3) {
		u32 reg = 0, mask = 0, val = 0, shifted;

		ret = of_property_read_u32_index(np, "extra-init", i, &reg);
		if (ret)
			return ret;
		ret = of_property_read_u32_index(np, "extra-init", i + 1, &mask);
		if (ret)
			return ret;
		ret = of_property_read_u32_index(np, "extra-init", i + 2, &val);
		if (ret)
			return ret;

		shifted = mask ? ((val << __ffs(mask)) & mask) : 0;
		ret = regmap_update_bits(priv->map, reg, mask, shifted);
		if (ret)
			return ret;
	}

	return 0;
}

int rtl837x_gpio_and_led_init(struct rtl837x_priv *priv)
{
	struct device_node *np = priv->dev->of_node;
	int ret;

	/*
	 * Apply RTLPlayground baseline LED hardening before board overrides:
	 * 1. Disable RLDP loop-indicator hijack on ports 0..8
	 * 2. Set bit 6 (SOLID_EN) in LED_GLB_IO_EN (0x65dc) so solid link/speed
	 *    states drive LEDs cleanly instead of dim activity-only glow.
	 */
	regmap_update_bits(priv->map, RTL837X_REG_LED_RLDP_1, 0x3, 0);
	regmap_update_bits(priv->map, RTL837X_REG_LED_GLB_IO_EN,
			   RTL837X_LED_GLB_IO_SOLID_EN,
			   RTL837X_LED_GLB_IO_SOLID_EN);

	ret = rtl837x_apply_dt_extra_init(priv);
	if (ret)
		return ret;

	/* Ensure SOLID_EN remains set even if board extra-init wrote 0x65dc */
	regmap_update_bits(priv->map, RTL837X_REG_LED_GLB_IO_EN,
			   RTL837X_LED_GLB_IO_SOLID_EN,
			   RTL837X_LED_GLB_IO_SOLID_EN);

	if (of_property_read_bool(np, "gpio-controller") &&
	    !priv->gpio_registered) {
		priv->gpio_chip.label = dev_name(priv->dev);
		priv->gpio_chip.parent = priv->dev;
		priv->gpio_chip.owner = THIS_MODULE;
		priv->gpio_chip.base = -1;
		priv->gpio_chip.ngpio = RTL837X_GPIO_MAX;
		priv->gpio_chip.can_sleep = true;
		priv->gpio_chip.get_direction = rtl837x_gpio_get_direction;
		priv->gpio_chip.direction_input = rtl837x_gpio_dir_in;
		priv->gpio_chip.direction_output = rtl837x_gpio_dir_out;
		priv->gpio_chip.get = rtl837x_gpio_get;
		priv->gpio_chip.set = rtl837x_gpio_set;

		ret = devm_gpiochip_add_data(priv->dev, &priv->gpio_chip, priv);
		if (ret)
			return ret;
		priv->gpio_registered = true;
	}

	return 0;
}
