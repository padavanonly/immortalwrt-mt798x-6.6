/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: warp
	warp_wifi.c
*/
#include "warp_cfg.h"
#include "warp_wifi.h"
#include "warp_utility.h"

/*
*
*/
bool
wifi_hw_tx_allow(struct wifi_entry *wifi, unsigned char *tx_info)
{
	if (wifi->ops->hw_tx_allow)
		return wifi->ops->hw_tx_allow(tx_info);
	return true;
}

/*
* Wifi function part
*/

/*
*
*/
void
wifi_tx_ring_info_dump(struct wifi_entry *wifi, unsigned char ring_id,
		       unsigned int idx)
{
	wifi->ops->tx_ring_info_dump(wifi, ring_id, idx);
}

/*
* wifi address translate control configure
*/
void
wifi_chip_atc_set(struct wifi_entry *wifi, bool enable)
{
	wifi->ops->config_atc(wifi->hw.priv, enable);
}

/*
*
*/
void
wifi_chip_reset(struct wifi_entry *wifi)
{
	wifi->ops->do_wifi_reset(wifi->hw.priv);
}

/*
*
*/
void
wifi_chip_update_wo_rxcnt(struct wifi_entry *wifi, void *wo_rxcnt)
{
	if (wifi->ops->update_wo_rxcnt)
		wifi->ops->update_wo_rxcnt(wifi->hw.priv, wo_rxcnt);
	else
		warp_dbg(WARP_DBG_OFF, "invalid ops, dismissed\n");
}

void
wifi_hb_check_notify(struct wifi_entry *wifi)
{
	if (wifi->ops->hb_check_notify)
		wifi->ops->hb_check_notify(wifi->hw.priv);
}

/*
* CHIP related setting
*/
void
wifi_chip_probe(struct wifi_entry *wifi, u32 irq, u8 warp_ver, u8 warp_sub_ver,
			u8 warp_branch, int warp_hw_caps)
{
	/* assign base_addr for read/write cr purpose */
	wifi->base_addr = wifi->hw.base_addr;

	/*always disable hw cr mirror first */
	wifi_chip_atc_set(wifi, false);

	wifi->ops->warp_ver_notify(wifi->hw.priv, warp_ver, warp_sub_ver, warp_branch, warp_hw_caps);
}

/*
*
*/
void
wifi_chip_remove(struct wifi_entry *wifi)
{
	wifi_chip_atc_set(wifi, true);
	wifi->hw.base_addr = 0;
	wifi->hw.p_int_mask = NULL;
}


/*
*
*/
void
wifi_chip_set_irq(struct wifi_entry *wifi, bool irq_set, u32 irq_number)
{
	if (irq_set) {
		if (wifi->ops->request_irq)
			wifi->ops->request_irq(wifi->hw.priv, irq_number);
		else
			wifi->ops->swap_irq(wifi->hw.priv, irq_number);
	} else {
		if (wifi->ops->free_irq)
			wifi->ops->free_irq(wifi->hw.priv, irq_number);
		else
			wifi->ops->swap_irq(wifi->hw.priv, wifi->hw.irq);
	}
}
