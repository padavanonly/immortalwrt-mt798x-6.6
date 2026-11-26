// SPDX-License-Identifier: GPL-2.0+
#include "rtl8261d.h"

#if IS_REACHABLE(CONFIG_HWMON)
static umode_t rtl8261x_hwmon_is_visible(const void *data,
					 enum hwmon_sensor_types type,
					 u32 attr, int channel)
{
	(void)data;
	(void)type;
	(void)channel;

	if (attr == hwmon_temp_input || attr == hwmon_temp_max)
		return 0444;
	return 0;
}

static long rtl8261x_hwmon_decode_mc(int raw)
{
	raw &= RTL822X_VND2_TSRR_MASK;
	if (raw >= 512)
		raw -= 1024;

	return 500L * raw;
}

static int rtl8261x_hwmon_read(struct device *dev, enum hwmon_sensor_types type,
			       u32 attr, int channel, long *val)
{
	struct phy_device *phydev = dev_get_drvdata(dev);
	int raw;

	(void)type;
	(void)channel;

	switch (attr) {
	case hwmon_temp_input:
		raw = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL822X_VND2_TSRR);
		if (raw < 0)
			return raw;
		*val = rtl8261x_hwmon_decode_mc(raw);
		return 0;
	case hwmon_temp_max:
		/* Chip automatically downshifts to 1G if threshold is exceeded */
		raw = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL822X_VND2_TSSR);
		if (raw < 0)
			return raw;
		*val = rtl8261x_hwmon_decode_mc((raw & 0xffff) >> 6);
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static const struct hwmon_ops rtl8261x_hwmon_ops = {
	.is_visible = rtl8261x_hwmon_is_visible,
	.read = rtl8261x_hwmon_read,
};

static const struct hwmon_channel_info * const rtl8261x_hwmon_info[] = {
	HWMON_CHANNEL_INFO(temp, HWMON_T_INPUT | HWMON_T_MAX),
	NULL
};

static const struct hwmon_chip_info rtl8261x_hwmon_chip_info = {
	.ops = &rtl8261x_hwmon_ops,
	.info = rtl8261x_hwmon_info,
};
#endif /* IS_REACHABLE(CONFIG_HWMON) */

int rtl8261x_thermal_sensor_init(struct phy_device *phydev, u8 max_temp_c)
{
	int ret;

	if (max_temp_c > 120)
		return -EINVAL;

	ret = phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2, RTL822X_VND2_TSALRM,
				 RTL822X_VND2_TSALRM_CLR_MASK);
	if (ret < 0)
		return ret;

	/* Enable thermal sensor (bit 3) and program 120C speed-downscale threshold */
	return phy_modify_mmd(phydev, MDIO_MMD_VEND2, RTL822X_VND2_TSSR,
			      RTL822X_VND2_TSSR_THRESH_MASK | RTL822X_VND2_TSSR_EN,
			      ((u16)max_temp_c << 7) | RTL822X_VND2_TSSR_EN);
}

int rtl8261x_hwmon_init(struct phy_device *phydev)
{
#if IS_REACHABLE(CONFIG_HWMON)
	struct device *dev = &phydev->mdio.dev;
	struct device *hwmon_dev;
	const char *name;
	int ret;

	ret = rtl8261x_thermal_sensor_init(phydev, RTL8261X_THERMAL_DEFAULT_MAX_C);
	if (ret < 0)
		return ret;

	name = devm_hwmon_sanitize_name(dev, dev_name(dev));
	if (IS_ERR(name))
		return PTR_ERR(name);

	hwmon_dev = devm_hwmon_device_register_with_info(dev, name, phydev,
							 &rtl8261x_hwmon_chip_info,
							 NULL);
	return PTR_ERR_OR_ZERO(hwmon_dev);
#else
	return rtl8261x_thermal_sensor_init(phydev, RTL8261X_THERMAL_DEFAULT_MAX_C);
#endif
}
