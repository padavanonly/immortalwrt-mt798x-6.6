// SPDX-License-Identifier: GPL-2.0+
/*
 * Realtek RTL8261C/CE/D 10Gbps Clause-45 PHY driver
 *
 * Built primarily on the mainline netdev v7.3/v8 backport (Javen Xu /
 * Beaverfffan/x-wrt-push commit d721114d804711bf32bff4e61117e23c3a34f3f6),
 * combined with graceful optional firmware loading, USXGMII/10GBASE-R rate
 * matching and SerDes resync, optional DT SerDes polarity inversion, thermal
 * sensor + hwmon, EDPD u16 tunable, DSP anti-interference calibration, and
 * 4-LED hardware offload.
 */
#include "rtl8261d.h"

MODULE_FIRMWARE(RTL8261C_CE_FW_NAME);

/* Non-blocking Clause-45 PMA soft-reset helper */
static int rtl8261x_pma_soft_reset(struct phy_device *phydev)
{
	int ret, val = 0;

	ret = phy_set_bits_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1,
			       MDIO_CTRL1_RESET);
	if (ret < 0)
		return ret;

	return phy_read_mmd_poll_timeout(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1,
					 val, !(val & MDIO_CTRL1_RESET),
					 5000, 600000, true);
}

/* Clause-45 IEEE 802.3an Master/Slave role setup & readback */
static int rtl8261x_an_setup_master_slave(struct phy_device *phydev)
{
	u16 ctl = 0;

	switch (phydev->master_slave_set) {
	case MASTER_SLAVE_CFG_MASTER_PREFERRED:
		ctl = MDIO_AN_10GBT_CTRL_MS_PORT_TYPE;
		break;
	case MASTER_SLAVE_CFG_SLAVE_PREFERRED:
		ctl = 0;
		break;
	case MASTER_SLAVE_CFG_MASTER_FORCE:
		ctl = MDIO_AN_10GBT_CTRL_MS_ENABLE |
		      MDIO_AN_10GBT_CTRL_MS_VALUE;
		break;
	case MASTER_SLAVE_CFG_SLAVE_FORCE:
		ctl = MDIO_AN_10GBT_CTRL_MS_ENABLE;
		break;
	case MASTER_SLAVE_CFG_UNKNOWN:
	case MASTER_SLAVE_CFG_UNSUPPORTED:
		return 0;
	default:
		return -EOPNOTSUPP;
	}

	return phy_modify_mmd_changed(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL,
				      MDIO_AN_10GBT_CTRL_MS_ENABLE |
				      MDIO_AN_10GBT_CTRL_MS_VALUE |
				      MDIO_AN_10GBT_CTRL_MS_PORT_TYPE, ctl);
}

static int rtl8261x_read_master_slave(struct phy_device *phydev)
{
	int ctrl, stat;

	phydev->master_slave_get = MASTER_SLAVE_CFG_UNKNOWN;
	phydev->master_slave_state = MASTER_SLAVE_STATE_UNKNOWN;

	ctrl = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_CTRL);
	if (ctrl < 0)
		return ctrl;

	if (ctrl & MDIO_AN_10GBT_CTRL_MS_ENABLE) {
		phydev->master_slave_get = (ctrl & MDIO_AN_10GBT_CTRL_MS_VALUE) ?
			MASTER_SLAVE_CFG_MASTER_FORCE :
			MASTER_SLAVE_CFG_SLAVE_FORCE;
	} else {
		phydev->master_slave_get =
			(ctrl & MDIO_AN_10GBT_CTRL_MS_PORT_TYPE) ?
			MASTER_SLAVE_CFG_MASTER_PREFERRED :
			MASTER_SLAVE_CFG_SLAVE_PREFERRED;
	}

	stat = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_STAT);
	if (stat < 0)
		return stat;

	if (stat & MDIO_AN_10GBT_STAT_MS_FAULT) {
		phydev->master_slave_state = MASTER_SLAVE_STATE_ERR;
	} else if (phydev->link) {
		phydev->master_slave_state = (stat & MDIO_AN_10GBT_STAT_MS_RES) ?
			MASTER_SLAVE_STATE_MASTER :
			MASTER_SLAVE_STATE_SLAVE;
	}

	return 0;
}

/* CRC32-verified firmware loader for RTL8261C_CG */
static int rtl8261x_verify_firmware(struct phy_device *phydev,
				    const struct firmware *fw)
{
	const struct rtl8261x_fw_header *hdr;
	u32 main_magic, sub_magic;
	u32 calc_crc, file_crc;
	size_t data_len;
	u16 num_entries;

	if (!fw || !fw->data || fw->size < FW_HEADER_SIZE) {
		phydev_err(phydev, "Firmware too small: %zu bytes\n",
			   fw ? fw->size : 0);
		return -EINVAL;
	}

	hdr = (const struct rtl8261x_fw_header *)fw->data;
	main_magic = le32_to_cpu(hdr->main_magic);
	if (main_magic != FW_MAIN_MAGIC) {
		phydev_err(phydev, "Invalid firmware magic: 0x%08x\n",
			   main_magic);
		return -EINVAL;
	}

	sub_magic = le32_to_cpu(hdr->sub_magic);
	if (sub_magic != FW_SUB_MAGIC_8261C) {
		phydev_err(phydev, "Invalid sub magic: 0x%08x\n", sub_magic);
		return -EINVAL;
	}

	num_entries = le16_to_cpu(hdr->num_entries);
	data_len = (size_t)num_entries * FW_ENTRY_SIZE;
	if (fw->size != FW_HEADER_SIZE + data_len) {
		phydev_err(phydev, "Firmware size mismatch\n");
		return -EINVAL;
	}

	calc_crc = crc32(~0U, fw->data + FW_HEADER_SIZE, data_len) ^ ~0U;
	file_crc = le32_to_cpu(hdr->crc32);
	if (calc_crc != file_crc) {
		phydev_err(phydev, "CRC32 mismatch: calc=0x%08x file=0x%08x\n",
			   calc_crc, file_crc);
		return -EINVAL;
	}

	return 0;
}

static int rtl8261x_fw_execute_entry(struct phy_device *phydev,
				     const struct rtl8261x_fw_entry *entry)
{
	u16 addr, value, timeout_ms;
	u8 dev, msb, lsb, poll_set;
	u32 bits, expect_val;
	int ret, val = 0;

	dev = entry->dev;
	addr = le16_to_cpu(entry->addr);
	msb = entry->msb;
	lsb = entry->lsb;
	value = le16_to_cpu(entry->value);
	timeout_ms = le16_to_cpu(entry->timeout_ms);
	poll_set = entry->poll_set;

	if (timeout_ms == 0)
		timeout_ms = RTL8261X_POLL_TIMEOUT_MS;
	if (dev > RTL8261X_MAX_MMD_DEV || msb > 15 || lsb > msb)
		return -EINVAL;

	bits = GENMASK(msb, lsb);
	expect_val = ((u32)value << lsb) & bits;

	switch (entry->type) {
	case OP_WRITE:
		return phy_modify_mmd(phydev, dev, addr, bits, (u16)expect_val);
	case OP_POLL:
		if (poll_set)
			ret = phy_read_mmd_poll_timeout(phydev, dev, addr, val,
							((u32)val & bits) == expect_val,
							1000,
							(u64)timeout_ms * 1000ULL,
							false);
		else
			ret = phy_read_mmd_poll_timeout(phydev, dev, addr, val,
							((u32)val & bits) != expect_val,
							1000,
							(u64)timeout_ms * 1000ULL,
							false);
		return ret;
	default:
		return -EINVAL;
	}
}

static int rtl8261x_fw_load(struct phy_device *phydev)
{
	struct rtl8261x_priv *priv = phydev->priv;
	const struct rtl8261x_fw_entry *entry;
	const struct rtl8261x_fw_header *hdr;
	const struct firmware *fw = NULL;
	int ret, i;

	if (!priv || !priv->fw_name)
		return 0;

	ret = firmware_request_nowarn(&fw, priv->fw_name, &phydev->mdio.dev);
	if (ret == -ENOENT)
		ret = firmware_request_nowarn(&fw, RTL8261C_CE_FW_NAME_FALLBACK,
					      &phydev->mdio.dev);
	if (ret == -ENOENT) {
		phydev_info(phydev,
			    "Optional firmware rtl8261c.bin not found, continuing with ROM/C45 defaults\n");
		return 0;
	}
	if (ret) {
		phydev_err(phydev, "Failed to load firmware %s: %d\n",
			   priv->fw_name, ret);
		return ret;
	}

	ret = rtl8261x_verify_firmware(phydev, fw);
	if (ret)
		goto release_fw;

	hdr = (const struct rtl8261x_fw_header *)fw->data;
	entry = (const struct rtl8261x_fw_entry *)(fw->data + FW_HEADER_SIZE);
	for (i = 0; i < le16_to_cpu(hdr->num_entries); i++, entry++) {
		ret = rtl8261x_fw_execute_entry(phydev, entry);
		if (ret) {
			phydev_err(phydev, "Firmware entry %d failed: %d\n",
				   i, ret);
			goto release_fw;
		}
	}

	priv->fw_loaded = true;

release_fw:
	release_firmware(fw);
	return ret;
}

/* Indirect VEND1 SerDes command helpers (0x7587..0x758a) */
static int rtl8261x_sds_read(struct phy_device *phydev, u16 sds_reg)
{
	int ret, cmd = 0;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1,
			    RTL8261X_VND1_USXGMII_ADDR, sds_reg);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1,
			    RTL8261X_VND1_USXGMII_CMD,
			    RTL8261X_USXGMII_CMD_EXEC_RD);
	if (ret < 0)
		return ret;

	ret = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					RTL8261X_VND1_USXGMII_CMD, cmd,
					!(cmd & RTL8261X_USXGMII_CMD_BUSY),
					500, 50000, false);
	if (ret < 0)
		return ret;

	return phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL8261X_VND1_SDS_RDATA);
}

static int rtl8261x_sds_write(struct phy_device *phydev, u16 sds_reg, u16 val)
{
	int ret, cmd = 0;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1,
			    RTL8261X_VND1_USXGMII_ADDR, sds_reg);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1,
			    RTL8261X_VND1_USXGMII_DATA, val);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1,
			    RTL8261X_VND1_USXGMII_CMD,
			    RTL8261X_USXGMII_CMD_EXEC_WR);
	if (ret < 0)
		return ret;

	return phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					 RTL8261X_VND1_USXGMII_CMD, cmd,
					 !(cmd & RTL8261X_USXGMII_CMD_BUSY),
					 500, 50000, false);
}

static int rtl8261x_sds_modify(struct phy_device *phydev, u16 sds_reg,
			       u16 mask, u16 set)
{
	int old;
	u16 new_val;

	old = rtl8261x_sds_read(phydev, sds_reg);
	if (old < 0)
		return old;

	new_val = ((u16)old & ~mask) | (set & mask);
	if (new_val == (u16)old)
		return 0;

	return rtl8261x_sds_write(phydev, sds_reg, new_val);
}

static int rtl8261x_serdes_link_up(struct phy_device *phydev, bool *up)
{
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_VEND1, RTL8261X_VND1_SDS_LINK_REG);
	if (val < 0)
		return val;

	*up = !!(val & RTL8261X_SDS_LINK_UP);
	return 0;
}

static int rtl8261x_config_serdes_polarity(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	bool tx_present = false, rx_present = false;
	bool tx_inv = false, rx_inv = false;
	u16 mask_8b10b = 0, set_8b10b = 0;
	u16 mask_64b66b = 0, set_64b66b = 0;
	const char *str = NULL;
	int ret;

	if (device_property_read_bool(dev, "realtek,serdes-tx-polar-inv")) {
		tx_present = true;
		tx_inv = true;
	} else if (!device_property_read_string(dev, "tx-polarity", &str) &&
		   str) {
		if (!strcmp(str, "invert")) {
			tx_present = true;
			tx_inv = true;
		} else if (!strcmp(str, "normal")) {
			tx_present = true;
			tx_inv = false;
		} else {
			phydev_warn(phydev, "Invalid tx-polarity '%s'\n", str);
			return -EINVAL;
		}
	}

	str = NULL;
	if (device_property_read_bool(dev, "realtek,serdes-rx-polar-inv")) {
		rx_present = true;
		rx_inv = true;
	} else if (!device_property_read_string(dev, "rx-polarity", &str) &&
		   str) {
		if (!strcmp(str, "invert")) {
			rx_present = true;
			rx_inv = true;
		} else if (!strcmp(str, "normal")) {
			rx_present = true;
			rx_inv = false;
		} else {
			phydev_warn(phydev, "Invalid rx-polarity '%s'\n", str);
			return -EINVAL;
		}
	}

	if (!tx_present && !rx_present)
		return 0;

	if (tx_present) {
		mask_8b10b |= RTL8261X_SDS_8B10B_TX_POLAR_INV;
		mask_64b66b |= RTL8261X_SDS_64B66B_TX_POLAR_INV;
		if (tx_inv) {
			set_8b10b |= RTL8261X_SDS_8B10B_TX_POLAR_INV;
			set_64b66b |= RTL8261X_SDS_64B66B_TX_POLAR_INV;
		}
	}

	if (rx_present) {
		mask_8b10b |= RTL8261X_SDS_8B10B_RX_POLAR_INV;
		mask_64b66b |= RTL8261X_SDS_64B66B_RX_POLAR_INV;
		if (rx_inv) {
			set_8b10b |= RTL8261X_SDS_8B10B_RX_POLAR_INV;
			set_64b66b |= RTL8261X_SDS_64B66B_RX_POLAR_INV;
		}
	}

	ret = rtl8261x_sds_modify(phydev, RTL8261X_SDS_8B10B_POLAR_REG,
				  mask_8b10b, set_8b10b);
	if (ret < 0)
		return ret;

	return rtl8261x_sds_modify(phydev, RTL8261X_SDS_64B66B_POLAR_REG,
				   mask_64b66b, set_64b66b);
}

/* USXGMII SerDes option + non-blocking in-band ANEG setup */
static int rtl8261x_config_usxgmii(struct phy_device *phydev, bool enable_an)
{
	int ret, val = 0;

	ret = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND2,
					RTL8261X_PHY_STATE_REG, val,
					(val & RTL8261X_PHY_STATE_MASK) ==
					RTL8261X_PHY_STATE_READY,
					1000, 100000, false);
	if (ret < 0)
		return ret;

	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, RTL8261X_VND1_SERDES_OPT4,
			     RTL8261X_VND1_SERDES_OPT_MASK, 0);
	if (ret < 0)
		return ret;

	return rtl8261x_sds_write(phydev, RTL8261X_USXGMII_AN_REG,
				  enable_an ? RTL8261X_USXGMII_AN_ENABLE_VAL :
					      RTL8261X_USXGMII_AN_DISABLE_VAL);
}

static int rtl8261x_resync_serdes(struct phy_device *phydev)
{
	bool enable_an = (phydev->interface == PHY_INTERFACE_MODE_USXGMII);

	return rtl8261x_sds_write(phydev, RTL8261X_USXGMII_AN_REG,
				  enable_an ? RTL8261X_USXGMII_AN_ENABLE_VAL :
					      RTL8261X_USXGMII_AN_DISABLE_VAL);
}

static int rtl8261x_get_rate_matching(struct phy_device *phydev,
				      phy_interface_t iface)
{
	(void)phydev;

	switch (iface) {
	case PHY_INTERFACE_MODE_NA:
	case PHY_INTERFACE_MODE_USXGMII:
	case PHY_INTERFACE_MODE_10GBASER:
		return RATE_MATCH_PAUSE;
	default:
		return RATE_MATCH_NONE;
	}
}

static int rtl8261x_config_dsp_and_eee(struct phy_device *phydev)
{
	int ret;

	ret = phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_OCP_EEE_CFG,
			       RTL8261X_OCP_EEE_C45_EN);
	if (ret < 0)
		return ret;

	ret = phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_VND2_DSP_CFG1,
			       RTL8261X_VND2_DSP_CFG1_EN);
	if (ret < 0)
		return ret;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_VND2_DSP_CFG2,
			      RTL8261X_VND2_DSP_CFG2_MASK,
			      RTL8261X_VND2_DSP_CFG2_VAL);
}

static int rtl8261x_match_phy_device_c45(struct phy_device *phydev,
					 u32 expected_id)
{
	u32 rid;

	if (!phydev || !phydev->is_c45)
		return 0;

	rid = phydev->c45_ids.device_ids[1];
	if (rid == 0xffffffff && phydev->mdio.bus &&
	    phydev->mdio.bus->read_c45) {
		int val;

		val = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PKGID1);
		if (val < 0)
			return 0;

		rid = (u32)val << 16;
		val = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PKGID2);
		if (val < 0)
			return 0;

		rid |= (u32)val;
	}

	return rid == expected_id;
}

static int rtl8261c_match_phy_device(struct phy_device *phydev)
{
	return rtl8261x_match_phy_device_c45(phydev, RTL_8261C_CG);
}

static int rtl8261ce_match_phy_device(struct phy_device *phydev)
{
	return rtl8261x_match_phy_device_c45(phydev, RTL_8261CE_CG);
}

/* Sub-model probe & hwmon registration */
static int rtl8261x_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct rtl8261x_priv *priv;
	int sub_phy_id, ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->last_speed = SPEED_UNKNOWN;
	priv->last_link = false;
	phydev->priv = priv;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_EXT_ADDR_REG,
			    RTL_8261X_SUB_PHY_ID_ADDR);
	if (ret < 0)
		return ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_EXT_DATA_REG);
	if (ret < 0)
		return ret;

	sub_phy_id = (ret >> 8) & 0xff;
	priv->sub_phy_id = (u8)sub_phy_id;

	switch (sub_phy_id) {
	case RTL8261C_CE_MODEL:
		priv->model = RTL8261_MODEL_C_CE;
		priv->fw_name = RTL8261C_CE_FW_NAME;
		phydev_info(phydev, "RTL8261C/CE detected (sub_id 0x%02x)\n",
			    sub_phy_id);
		break;
	case RTL8261D_MODEL:
		priv->model = RTL8261_MODEL_D;
		phydev_info(phydev, "RTL8261D detected (sub_id 0x%02x)\n",
			    sub_phy_id);
		break;
	default:
		priv->model = RTL8261_MODEL_GENERIC;
		phydev_warn(phydev,
			    "Unknown RTL8261 sub_id 0x%02x, using generic C45 mode\n",
			    sub_phy_id);
		break;
	}

	ret = rtl8261x_hwmon_init(phydev);
	if (ret < 0)
		phydev_warn(phydev, "hwmon init failed (%d), continuing\n", ret);

	return 0;
}

static int rtl8261x_config_init(struct phy_device *phydev)
{
	struct rtl8261x_priv *priv = phydev->priv;
	int ret;

	if (priv && priv->fw_name && !priv->fw_loaded) {
		ret = rtl8261x_fw_load(phydev);
		if (ret < 0)
			return ret;
	}

	ret = rtl8261x_thermal_sensor_init(phydev, RTL8261X_THERMAL_DEFAULT_MAX_C);
	if (ret < 0)
		return ret;

	if (phydev->interface == PHY_INTERFACE_MODE_USXGMII) {
		ret = rtl8261x_config_usxgmii(phydev, true);
		if (ret < 0)
			return ret;
	} else if (phydev->interface == PHY_INTERFACE_MODE_10GBASER) {
		ret = rtl8261x_config_usxgmii(phydev, false);
		if (ret < 0)
			return ret;
	}

	ret = rtl8261x_config_serdes_polarity(phydev);
	if (ret < 0)
		return ret;

	return rtl8261x_config_dsp_and_eee(phydev);
}

static int rtl8261x_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_c45_pma_read_abilities(phydev);
	if (ret)
		return ret;

	linkmode_set_bit(ETHTOOL_LINK_MODE_TP_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Pause_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
			 phydev->supported);

	return 0;
}

static int rtl8261x_config_aneg(struct phy_device *phydev)
{
	u16 adv_1g = 0;
	int ret;
	bool changed = false;

	if (phydev->autoneg == AUTONEG_DISABLE)
		return genphy_c45_pma_setup_forced(phydev);

	ret = rtl8261x_an_setup_master_slave(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	ret = genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			      phydev->advertising))
		adv_1g |= ADVERTISE_1000FULL;
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
			      phydev->advertising))
		adv_1g |= ADVERTISE_1000HALF;

	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2,
				     RTL822X_VND2_C22_REG(MII_CTRL1000),
				     ADVERTISE_1000FULL | ADVERTISE_1000HALF,
				     adv_1g);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static void rtl8261x_decode_physr_speed(struct phy_device *phydev, int val)
{
	int speed_code;

	phydev->duplex = (val & RTL8261X_PHYSR_DUPLEX) ?
			 DUPLEX_FULL : DUPLEX_HALF;
	speed_code = (FIELD_GET(RTL8261X_PHYSR_SPEED_H, val) << 2) |
		     FIELD_GET(RTL8261X_PHYSR_SPEED_L, val);

	switch (speed_code) {
	case RTL8261X_SPEED_CODE_10M:
		phydev->speed = SPEED_10;
		break;
	case RTL8261X_SPEED_CODE_100M:
		phydev->speed = SPEED_100;
		break;
	case RTL8261X_SPEED_CODE_500M:
		phydev->speed = 500;
		break;
	case 0x2:
	case RTL8261X_SPEED_CODE_1000M:
		phydev->speed = SPEED_1000;
		break;
	case RTL8261X_SPEED_CODE_2500M_V1:
	case RTL8261X_SPEED_CODE_2500M:
		phydev->speed = SPEED_2500;
		break;
	case RTL8261X_SPEED_CODE_5000M_V1:
	case RTL8261X_SPEED_CODE_5000M:
		phydev->speed = SPEED_5000;
		break;
	case RTL8261X_SPEED_CODE_10000M_V1:
	case RTL8261X_SPEED_CODE_10000M:
		phydev->speed = SPEED_10000;
		break;
	default:
		break;
	}
}

static int rtl8261x_read_status(struct phy_device *phydev)
{
	struct rtl8261x_priv *priv = phydev->priv;
	int ret, val = 0;

	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = genphy_c45_aneg_done(phydev);
		if (ret < 0)
			return ret;
		if (ret > 0) {
			val = phy_read_mmd(phydev, MDIO_MMD_VEND2,
					   RTL822X_VND2_C22_REG(MII_STAT1000));
			if (val < 0)
				return val;
		} else {
			val = 0;
		}
		mii_stat1000_mod_linkmode_lpa_t(phydev->lp_advertising, val);
	}

	ret = genphy_c45_read_status(phydev);
	if (ret < 0)
		return ret;

	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_PHYSR_REG);
	if (val < 0)
		return val;

	phydev->link = !!(val & RTL8261X_PHYSR_LINK);
	if (!phydev->link) {
		phydev->speed = SPEED_UNKNOWN;
		phydev->duplex = DUPLEX_UNKNOWN;
		phydev->master_slave_state = MASTER_SLAVE_STATE_UNKNOWN;
		phydev->mdix = ETH_TP_MDI_INVALID;
		if (priv)
			priv->last_link = false;
		return 0;
	}

	ret = rtl8261x_read_master_slave(phydev);
	if (ret < 0)
		return ret;

	if (phydev->speed == SPEED_UNKNOWN || phydev->speed <= 0)
		rtl8261x_decode_physr_speed(phydev, val);

	val = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBT_SWAPPOL);
	if (val >= 0) {
		switch (val & 0x3) {
		case MDIO_PMA_10GBT_SWAPPOL_ABNX | MDIO_PMA_10GBT_SWAPPOL_CDNX:
			phydev->mdix = ETH_TP_MDI;
			break;
		case 0:
			phydev->mdix = ETH_TP_MDI_X;
			break;
		default:
			phydev->mdix = ETH_TP_MDI_INVALID;
			break;
		}
	}

	if (phydev->interface == PHY_INTERFACE_MODE_USXGMII ||
	    phydev->interface == PHY_INTERFACE_MODE_10GBASER) {
		bool sds_up = false;

		ret = rtl8261x_serdes_link_up(phydev, &sds_up);
		if (ret < 0)
			return ret;

		if (!sds_up || (priv && phydev->speed > 0 &&
				priv->last_speed != SPEED_UNKNOWN &&
				priv->last_speed != phydev->speed)) {
			ret = rtl8261x_resync_serdes(phydev);
			if (ret == -ETIMEDOUT)
				phydev_warn(phydev,
					    "SerDes resync timed out, continuing\n");
			else if (ret < 0)
				return ret;
		}
	}

	if (priv) {
		priv->last_link = true;
		if (phydev->speed > 0)
			priv->last_speed = phydev->speed;
	}

	return 0;
}

static int rtl8261x_config_intr(struct phy_device *phydev)
{
	int ret;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_ISR);
		if (ret < 0)
			return ret;
		return phy_write_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_IMR,
				     RTL8261X_INT_MASK_DEFAULT);
	}

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_IMR, 0);
	if (ret < 0)
		return ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_ISR);
	return ret < 0 ? ret : 0;
}

static irqreturn_t rtl8261x_handle_interrupt(struct phy_device *phydev)
{
	int irq_status;

	irq_status = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_ISR);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	if (!(irq_status & RTL8261X_INT_MASK_ALL))
		return IRQ_NONE;

	if (irq_status & (RTL8261X_INT_LINK_CHG | RTL8261X_INT_AUTONEG_DONE |
			  RTL8261X_INT_AUTONEG_ERROR | RTL8261X_INT_JABBER))
		phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

/* ETHTOOL_PHY_EDPD passes u16 *, not u32 * */
static int rtl8261x_get_tunable(struct phy_device *phydev,
				struct ethtool_tunable *tuna, void *data)
{
	int val;

	if (tuna->id != ETHTOOL_PHY_EDPD)
		return -EOPNOTSUPP;
	if (!data)
		return -EINVAL;

	val = phy_read_mmd(phydev, MDIO_MMD_VEND2, RTL8261X_OCP_POWER_CFG);
	if (val < 0)
		return val;

	*(u16 *)data = (val & RTL8261X_OCP_POWER_EDPD_EN) ?
		       ETHTOOL_PHY_EDPD_DFLT_TX_MSECS :
		       ETHTOOL_PHY_EDPD_DISABLE;
	return 0;
}

static int rtl8261x_set_tunable(struct phy_device *phydev,
				struct ethtool_tunable *tuna, const void *data)
{
	u16 tx_msecs;

	if (tuna->id != ETHTOOL_PHY_EDPD)
		return -EOPNOTSUPP;
	if (!data)
		return -EINVAL;

	tx_msecs = *(const u16 *)data;
	switch (tx_msecs) {
	case ETHTOOL_PHY_EDPD_DISABLE:
		return phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2,
					  RTL8261X_OCP_POWER_CFG,
					  RTL8261X_OCP_POWER_EDPD_EN);
	case ETHTOOL_PHY_EDPD_NO_TX:
	case ETHTOOL_PHY_EDPD_DFLT_TX_MSECS:
		return phy_set_bits_mmd(phydev, MDIO_MMD_VEND2,
					RTL8261X_OCP_POWER_CFG,
					RTL8261X_OCP_POWER_EDPD_EN);
	default:
		return -EINVAL;
	}
}

static struct phy_driver rtl8261x_drvs[] = {
	{
		PHY_ID_MATCH_EXACT(RTL_8261C_CG),
		.match_phy_device	= rtl8261c_match_phy_device,
		.name			= "Realtek RTL8261C/D 10Gbps PHY",
		.probe			= rtl8261x_probe,
		.config_init		= rtl8261x_config_init,
		.get_features		= rtl8261x_get_features,
		.get_rate_matching	= rtl8261x_get_rate_matching,
		.config_aneg		= rtl8261x_config_aneg,
		.aneg_done		= genphy_c45_aneg_done,
		.read_status		= rtl8261x_read_status,
		.config_intr		= rtl8261x_config_intr,
		.handle_interrupt	= rtl8261x_handle_interrupt,
		.soft_reset		= rtl8261x_pma_soft_reset,
		.suspend		= genphy_c45_pma_suspend,
		.resume			= genphy_c45_pma_resume,
		.get_tunable		= rtl8261x_get_tunable,
		.set_tunable		= rtl8261x_set_tunable,
		.set_loopback		= genphy_c45_loopback,
#if IS_ENABLED(CONFIG_PHYLIB_LEDS)
		.led_hw_is_supported	= rtl8261x_led_hw_is_supported,
		.led_hw_control_get	= rtl8261x_led_hw_control_get,
		.led_hw_control_set	= rtl8261x_led_hw_control_set,
		.led_brightness_set	= rtl8261x_led_brightness_set,
		.led_polarity_set	= rtl8261x_led_polarity_set,
#endif
	}, {
		PHY_ID_MATCH_EXACT(RTL_8261CE_CG),
		.match_phy_device	= rtl8261ce_match_phy_device,
		.name			= "Realtek RTL8261CE 10Gbps PHY",
		.probe			= rtl8261x_probe,
		.config_init		= rtl8261x_config_init,
		.get_features		= rtl8261x_get_features,
		.get_rate_matching	= rtl8261x_get_rate_matching,
		.config_aneg		= rtl8261x_config_aneg,
		.aneg_done		= genphy_c45_aneg_done,
		.read_status		= rtl8261x_read_status,
		.config_intr		= rtl8261x_config_intr,
		.handle_interrupt	= rtl8261x_handle_interrupt,
		.soft_reset		= rtl8261x_pma_soft_reset,
		.suspend		= genphy_c45_pma_suspend,
		.resume			= genphy_c45_pma_resume,
		.get_tunable		= rtl8261x_get_tunable,
		.set_tunable		= rtl8261x_set_tunable,
		.set_loopback		= genphy_c45_loopback,
#if IS_ENABLED(CONFIG_PHYLIB_LEDS)
		.led_hw_is_supported	= rtl8261x_led_hw_is_supported,
		.led_hw_control_get	= rtl8261x_led_hw_control_get,
		.led_hw_control_set	= rtl8261x_led_hw_control_set,
		.led_brightness_set	= rtl8261x_led_brightness_set,
		.led_polarity_set	= rtl8261x_led_polarity_set,
#endif
	},
};

module_phy_driver(rtl8261x_drvs);

static const struct mdio_device_id __maybe_unused rtl8261x_tbl[] = {
	{ PHY_ID_MATCH_EXACT(RTL_8261C_CG) },
	{ PHY_ID_MATCH_EXACT(RTL_8261CE_CG) },
	{ }
};
MODULE_DEVICE_TABLE(mdio, rtl8261x_tbl);

MODULE_DESCRIPTION("Realtek RTL8261C/CE/D 10GbE Clause-45 PHY driver");
MODULE_AUTHOR("Javen Xu / StarField Xu");
MODULE_LICENSE("GPL");
