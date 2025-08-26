/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	mt_io.c
*/
#include	"rt_config.h"

#ifdef RTMP_MAC_PCI
BOOLEAN mt_mac_cr_range_mapping(
	struct physical_device *ph_dev, u32 *mac_addr)
{
	UINT32 mac_addr_hif = *mac_addr;
	INT idx = 0;
	BOOLEAN IsFound = 0;
	RTMP_CHIP_CAP *chip_cap = &ph_dev->chip_cap;
	struct bus_info *bus_info = chip_cap->bus_info;
	UINT32 u4StartAddr, u4EndAddr;

	if (mac_addr_hif < bus_info->max_static_map_addr)
		IsFound = 1;
	else {
		while (TRUE) {
			u4StartAddr = bus_info->bus2chip[idx].u4ChipAddr;
			u4EndAddr = bus_info->bus2chip[idx].u4ChipAddr +
				bus_info->bus2chip[idx].u4Range;

			/* End of mapping table */
			if (u4EndAddr == 0x0) {
				break;
			}

			if ((mac_addr_hif >= u4StartAddr)
				&& (mac_addr_hif < u4EndAddr)) {
				mac_addr_hif = (mac_addr_hif - u4StartAddr) +
					bus_info->bus2chip[idx].u4BusAddr;
				IsFound = 1;
				break;
			}

			idx++;
		}
	}

	if (IsFound &&
#if defined(_HIF_AXI)
		mac_addr_hif > 0xffffff
#else
#if defined(MT7990) || defined(MT7992) || defined(MT7993)
		/* CONN_INFRA address_in is 21 valid bits [20:0]*/
		mac_addr_hif > 0x1fffff
#else
		/* PCIe address space is 20-bit wide */
		mac_addr_hif > 0xfffff
#endif /* MT7990 */
#endif
		) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_IO, DBG_LVL_ERROR,
			"CR Addr[0x%x] out of range\n", mac_addr_hif);
		return 0;
	}

	*mac_addr = mac_addr_hif;
	return IsFound;
}
#endif /* RTMP_MAC_PCI */

UINT32 mt_physical_addr_map(RTMP_ADAPTER *pAd, UINT32 addr)
{
	UINT32 global_addr = 0x0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 wtbl_2_base = cap->WtblPseAddr;

	if (addr < 0x2000)
		global_addr = 0x80020000 + addr;
	else if ((addr >= 0x2000) && (addr < 0x4000))
		global_addr = 0x80000000 + addr - 0x2000;
	else if ((addr >= 0x4000) && (addr < 0x8000))
		global_addr = 0x50000000 + addr - 0x4000;
	else if ((addr >= 0x8000) && (addr < 0x10000))
		global_addr = 0xa0000000 + addr - 0x8000;
	else if ((addr >= 0x10000) && (addr < 0x20000))
		global_addr = 0x60200000 + addr - 0x10000;
	else if ((addr >= 0x20000) && (addr < 0x40000)) {
		if (mt_mac_cr_range_mapping(pAd->physical_dev, &addr))
			global_addr = addr;
		else {
			MTWF_DBG(pAd, DBG_CAT_HIF, CATHIF_IO, DBG_LVL_ERROR,
				"unknown addr range = %x\n", addr);
			return global_addr;
		}
	} else if ((addr >= 0x40000) && (addr < 0x80000)) { /* WTBL Address */
		global_addr = wtbl_2_base + addr - 0x40000;
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_IO, DBG_LVL_DEBUG,
			"==>global_addr1=0x%x\n", global_addr);
	} else if ((addr >= 0xc0000) && (addr < 0xc0100)) { /* PSE Client */
		global_addr = 0x800c0000 + addr - 0xc0000;
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_IO, DBG_LVL_DEBUG,
			"==>global_addr2=0x%x\n", global_addr);
	} else {
		global_addr = addr;
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_IO, DBG_LVL_DEBUG,
			"==>global_addr3=0x%x\n", global_addr);
	}

	return global_addr;
}


/*export io func.*/
VOID hif_io_force_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_forec_read32)
		ops->hif_io_forec_read32(hdev_ctrl, reg, val);
}

VOID hif_io_force_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_forec_write32)
		ops->hif_io_forec_write32(hdev_ctrl, reg, val);
}

VOID hif_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_read32)
		ops->hif_io_read32(hdev_ctrl, reg, val);
}

VOID hif_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hif_io_write32)
		ops->hif_io_write32(hdev_ctrl, reg, val);
}

VOID sys_io_read32(ULONG reg, UINT32 *val)
{
	*val = readl((void *) reg);
}

VOID sys_io_write32(ULONG reg, UINT32 val)
{
	writel(val, (void *) reg);
}

VOID mac_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mac_io_read32)
		ops->mac_io_read32(hdev_ctrl, reg, val);
}

VOID mac_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mac_io_write32)
		ops->mac_io_write32(hdev_ctrl, reg, val);
}

VOID phy_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->phy_io_read32)
		ops->phy_io_read32(hdev_ctrl, reg, val);
}

VOID phy_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->phy_io_write32)
		ops->phy_io_write32(hdev_ctrl, reg, val);
}

VOID mcu_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mcu_io_read32)
		ops->mcu_io_read32(hdev_ctrl, reg, val);
}

VOID mcu_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->mcu_io_write32)
		ops->mcu_io_write32(hdev_ctrl, reg, val);
}

VOID hw_io_read32(void *hdev_ctrl, UINT32 reg, UINT32 *val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hw_io_read32)
		ops->hw_io_read32(hdev_ctrl, reg, val);
}

VOID hw_io_write32(void *hdev_ctrl, UINT32 reg, UINT32 val)
{
	struct mt_io_ops *ops = hc_get_io_ops(hdev_ctrl);

	if (ops->hw_io_write32)
		ops->hw_io_write32(hdev_ctrl, reg, val);
}

VOID hif_core_ops_register(void *hdev_ctrl, INT infType)
{

#ifdef HWIFI_SUPPORT
	if (infType == RTMP_DEV_INF_HWIFI)
		hwifi_core_ops_register(hdev_ctrl);
#endif /* HWIFI_SUPPORT */

}

VOID hif_core_ops_unregister(void *hdev_ctrl, INT infType)
{

#ifdef HWIFI_SUPPORT
	if (infType == RTMP_DEV_INF_HWIFI)
		hwifi_core_ops_unregister(hdev_ctrl);
#endif /* HWIFI_SUPPORT */
}

NDIS_STATUS hif_ctrl_init(void **chip_hif, INT infType)
{
	UINT32 hif_size = 0;
#ifdef RTMP_MAC_PCI
	if (infType == RTMP_DEV_INF_PCIE || infType == RTMP_DEV_INF_PCI || infType == RTMP_DEV_INF_RBUS)
		hif_size = sizeof(struct pci_hif_chip);
#endif


	if (hif_size == 0)
		return NDIS_STATUS_INVALID_DATA;

	if (multi_hif_entry_alloc(chip_hif, hif_size) != NDIS_STATUS_SUCCESS)
		return NDIS_STATUS_FAILURE;

	return NDIS_STATUS_SUCCESS;
}

VOID hif_ctrl_exit(void *chip_hif)
{
	multi_hif_entry_free(chip_hif);
}

VOID hif_chip_init(VOID *chip_hif, UINT32 device_id)
{
}
