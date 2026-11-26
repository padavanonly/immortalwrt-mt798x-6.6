// SPDX-License-Identifier: GPL-2.0-only
/*
 * RTL8372N / RTL8373 SerDes PCS & Dynamic SFP Rate Controller
 *
 * Incorporates RTLPlayground's N-series 64B/66B & 8B/10B polarity handling
 * and multi-speed SFP SerDes transitions (10GBASE-R / 2500BASE-X / HSGMII /
 * 1000BASE-X / SGMII / 100FX) with WDIG & 10G CMU/RX soft-reset recovery.
 */
#include "rtl837x.h"

int rtl837x_serdes_port_to_sds(int port)
{
	if (port == 3)
		return 0;
	if (port == 8)
		return 1;
	return -EINVAL;
}

int rtl837x_serdes_init_polarity(struct rtl837x_priv *priv)
{
	int sds, ret;

	for (sds = 0; sds < 2; sds++) {
		u16 mask66 = 0, set66 = 0, mask10 = 0, set10 = 0;

		/*
		 * RTLPlayground sds_init(): N-series silicon defaults 64B/66B
		 * RX PN inverted on high-speed lanes unless board DT overrides.
		 */
		mask66 = RTL837X_SDS_64B66B_RX_INV | RTL837X_SDS_64B66B_TX_INV;
		if (priv->sds_rx_inv[sds] || (priv->is_n_series && sds == 0))
			set66 |= RTL837X_SDS_64B66B_RX_INV;
		if (priv->sds_tx_inv[sds])
			set66 |= RTL837X_SDS_64B66B_TX_INV;

		ret = rtl837x_sds_reg_modify(priv, (u8)sds,
					     RTL837X_SDS_PAGE_PCS_64B66B, 0x02,
					     mask66, set66);
		if (ret)
			return ret;

		mask10 = RTL837X_SDS_8B10B_RX_INV | RTL837X_SDS_8B10B_TX_INV;
		if (priv->sds_rx_inv[sds] || (priv->is_n_series && sds == 1))
			set10 |= RTL837X_SDS_8B10B_RX_INV;
		if (priv->sds_tx_inv[sds])
			set10 |= RTL837X_SDS_8B10B_TX_INV;

		ret = rtl837x_sds_reg_modify(priv, (u8)sds,
					     RTL837X_SDS_PAGE_SDS, 0x00,
					     mask10, set10);
		if (ret)
			return ret;
	}

	return 0;
}

static enum rtl837x_sds_mode rtl837x_iface_to_sds_mode(phy_interface_t iface)
{
	switch (iface) {
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_10GKR:
		return RTL837X_SDS_MODE_10GBASER;
	case PHY_INTERFACE_MODE_USXGMII:
#ifdef PHY_INTERFACE_MODE_10G_QXGMII
	case PHY_INTERFACE_MODE_10G_QXGMII:
#endif
		return RTL837X_SDS_MODE_USXGMII;
	case PHY_INTERFACE_MODE_2500BASEX:
		return RTL837X_SDS_MODE_2500BASEX;
	case PHY_INTERFACE_MODE_1000BASEX:
		return RTL837X_SDS_MODE_1000BASEX;
	case PHY_INTERFACE_MODE_SGMII:
		return RTL837X_SDS_MODE_SGMII;
	case PHY_INTERFACE_MODE_100BASEX:
		return RTL837X_SDS_MODE_100FX;
	default:
		return RTL837X_SDS_MODE_OFF;
	}
}

static int rtl837x_serdes_set_hw_mode(struct rtl837x_priv *priv, u8 sds,
				      enum rtl837x_sds_mode mode)
{
	u32 mask = sds ? RTL837X_SDS1_MODE_SEL_MASK :
			 RTL837X_SDS0_MODE_SEL_MASK;
	u32 val = sds ? FIELD_PREP(RTL837X_SDS1_MODE_SEL_MASK, mode) :
			FIELD_PREP(RTL837X_SDS0_MODE_SEL_MASK, mode);

	return regmap_update_bits(priv->map, RTL837X_SDS_MODE_SEL_REG,
				  mask, val);
}

static int rtl837x_serdes_wdig_reset(struct rtl837x_priv *priv, u8 sds)
{
	int ret;

	ret = rtl837x_sds_reg_modify(priv, sds, RTL837X_SDS_PAGE_WDIG, 0x02,
				     0x001f, 0x001f);
	if (ret)
		return ret;
	msleep(20);
	return rtl837x_sds_reg_modify(priv, sds, RTL837X_SDS_PAGE_WDIG, 0x02,
				      0x001f, 0x0000);
}

int rtl837x_serdes_config_mode(struct rtl837x_priv *priv, u8 sds,
			       phy_interface_t interface, unsigned int neg_mode)
{
	enum rtl837x_sds_mode mode = rtl837x_iface_to_sds_mode(interface);
	bool inband_an = (neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED);
	int ret;

	if (mode == RTL837X_SDS_MODE_OFF)
		return -EOPNOTSUPP;

	ret = rtl837x_serdes_set_hw_mode(priv, sds, RTL837X_SDS_MODE_OFF);
	if (ret)
		return ret;
	usleep_range(10000, 15000);

	/* Apply mode-specific fiber / USXGMII / 10G analog & PCS patches */
	if (mode == RTL837X_SDS_MODE_1000BASEX) {
		ret = rtl837x_sds_reg_modify(priv, sds, RTL837X_SDS_PAGE_SDS,
					     0x04, BIT(0), BIT(0));
		if (ret)
			return ret;
	} else if (mode == RTL837X_SDS_MODE_USXGMII) {
		ret = rtl837x_sds_reg_modify(priv, sds,
					     RTL837X_SDS_PAGE_TGR_PRO_1, 0x11,
					     0x000f, inband_an ? 0x000f : 0);
		if (ret)
			return ret;
	} else if (mode == RTL837X_SDS_MODE_10GBASER) {
		/* RTLPlayground 10G-R analog/CDR bring-up sequence */
		ret = rtl837x_sds_reg_write(priv, sds, RTL837X_SDS_PAGE_ANA_COM,
					    0x00, 0x4906);
		if (ret)
			return ret;
		ret = rtl837x_sds_reg_write(priv, sds, RTL837X_SDS_PAGE_ANA_10G,
					    0x05, 0x4000);
		if (ret)
			return ret;
	}

	/* Configure Clause-37 fiber auto-negotiation for 1G/2.5G/SGMII SFP */
	if (mode == RTL837X_SDS_MODE_1000BASEX ||
	    mode == RTL837X_SDS_MODE_2500BASEX ||
	    mode == RTL837X_SDS_MODE_SGMII) {
		ret = rtl837x_sds_reg_modify(priv, sds, RTL837X_SDS_PAGE_FIB,
					     MII_BMCR, BMCR_ANENABLE,
					     inband_an ? BMCR_ANENABLE : 0);
		if (ret)
			return ret;
	}

	ret = rtl837x_serdes_set_hw_mode(priv, sds, mode);
	if (ret)
		return ret;
	msleep(30);

	return rtl837x_serdes_wdig_reset(priv, sds);
}

static struct rtl837x_port_priv *pcs_to_rtl_port(struct phylink_pcs *pcs)
{
	return container_of(pcs, struct rtl837x_port_priv, pcs);
}

static void rtl837x_pcs_get_state(struct phylink_pcs *pcs,
				  struct phylink_link_state *state)
{
	struct rtl837x_port_priv *pp = pcs_to_rtl_port(pcs);
	struct rtl837x_priv *priv = pp->priv;
	int sds = rtl837x_serdes_port_to_sds(pp->index);
	u32 link_sts = 0, spd_sts = 0, dup_sts = 0, rx_p = 0, tx_p = 0;
	u8 spd_code;

	state->link = false;
	state->speed = SPEED_UNKNOWN;
	state->duplex = DUPLEX_UNKNOWN;
	state->pause = 0;

	if (sds >= 0 &&
	    (state->interface == PHY_INTERFACE_MODE_1000BASEX ||
	     state->interface == PHY_INTERFACE_MODE_2500BASEX)) {
		u16 bmsr = 0, lpa = 0;

		if (!rtl837x_sds_reg_read(priv, (u8)sds, RTL837X_SDS_PAGE_FIB,
					  MII_BMSR, &bmsr) &&
		    !rtl837x_sds_reg_read(priv, (u8)sds, RTL837X_SDS_PAGE_FIB,
					  MII_LPA, &lpa) &&
		    (bmsr & BMSR_LSTATUS)) {
			phylink_mii_c22_pcs_decode_state(state, bmsr, lpa);
			if (state->interface == PHY_INTERFACE_MODE_2500BASEX &&
			    state->link)
				state->speed = SPEED_2500;
			return;
		}
	}

	if (regmap_read(priv->map, RTL837X_MAC_LINK_STS_REG, &link_sts))
		return;
	if (!(link_sts & BIT(pp->index)))
		return;

	if (regmap_read(priv->map, RTL837X_MAC_LINK_SPD_STS_REG(pp->index),
			&spd_sts) ||
	    regmap_read(priv->map, RTL837X_MAC_LINK_DUP_STS_REG, &dup_sts) ||
	    regmap_read(priv->map, RTL837X_MAC_RX_PAUSE_STS_REG, &rx_p) ||
	    regmap_read(priv->map, RTL837X_MAC_TX_PAUSE_STS_REG, &tx_p))
		return;

	state->link = true;
	state->duplex = (dup_sts & BIT(pp->index)) ? DUPLEX_FULL : DUPLEX_HALF;
	if (rx_p & BIT(pp->index))
		state->pause |= MLO_PAUSE_RX;
	if (tx_p & BIT(pp->index))
		state->pause |= MLO_PAUSE_TX;

	spd_code = (spd_sts >> ((pp->index & 7) * 4)) & 0xf;
	switch (spd_code) {
	case 0:
		state->speed = SPEED_10;
		break;
	case 1:
		state->speed = SPEED_100;
		break;
	case 2:
		state->speed = SPEED_1000;
		break;
	case 4:
		state->speed = SPEED_10000;
		break;
	case 5:
		state->speed = SPEED_2500;
		break;
	case 6:
		state->speed = SPEED_5000;
		break;
	default:
		state->speed = SPEED_UNKNOWN;
		break;
	}
}

static int rtl837x_pcs_config(struct phylink_pcs *pcs, unsigned int neg_mode,
			      phy_interface_t interface,
			      const unsigned long *advertising,
			      bool permit_pause_to_mac)
{
	struct rtl837x_port_priv *pp = pcs_to_rtl_port(pcs);
	int sds = rtl837x_serdes_port_to_sds(pp->index);

	(void)advertising;
	(void)permit_pause_to_mac;
	if (sds < 0)
		return -EINVAL;

	return rtl837x_serdes_config_mode(pp->priv, (u8)sds, interface,
					  neg_mode);
}

static void rtl837x_pcs_an_restart(struct phylink_pcs *pcs)
{
	struct rtl837x_port_priv *pp = pcs_to_rtl_port(pcs);
	int sds = rtl837x_serdes_port_to_sds(pp->index);

	if (sds >= 0)
		rtl837x_sds_reg_modify(pp->priv, (u8)sds, RTL837X_SDS_PAGE_FIB,
				       MII_BMCR, BMCR_ANRESTART,
				       BMCR_ANRESTART);
}

const struct phylink_pcs_ops rtl837x_sds_pcs_ops = {
	.pcs_get_state	= rtl837x_pcs_get_state,
	.pcs_config	= rtl837x_pcs_config,
	.pcs_an_restart	= rtl837x_pcs_an_restart,
};
