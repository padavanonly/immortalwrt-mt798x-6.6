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
/***************************************************************************
 ***************************************************************************

*/

#include "rt_config.h"
#include "common/link_list.h"
#include "multi_hif.h"
#ifdef RX_RPS_SUPPORT
#include "mac_mt/fmac/mt_fmac.h"
#endif

VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	MTWF_PRINT("RxD:\n");
	MTWF_PRINT("\tSDPtr0/SDLen0/LastSec0=0x%x/0x%x/0x%x\n",
			 pRxD->SDP0, pRxD->SDL0, pRxD->LS0);
	MTWF_PRINT("\tSDPtr1/SDLen1/LastSec1=0x%x/0x%x/0x%x\n",
			 pRxD->SDP1, pRxD->SDL1, pRxD->LS1);
	MTWF_PRINT("\tDDONE=0x%x\n", pRxD->DDONE);
}

VOID dump_rxd_debug(RTMP_ADAPTER *pAd, RXD_DEBUG_STRUC *pRxD)
{
	MTWF_PRINT("RxD:\n");
	MTWF_PRINT("\tSDPtr0/SDLen0/LastSec0=0x%x/0x%x/0x%x\n",
			 pRxD->SDP0, pRxD->SDL0, pRxD->LS0);
	MTWF_PRINT("\tQID/DIDX/CIDX/SW_INFO=0x%x/0x%x/0x%x/0x%x\n",
			 pRxD->QID, pRxD->DIDX, pRxD->CIDX, pRxD->SW_INFO);
	MTWF_PRINT("\tDDONE=0x%x\n", pRxD->DDONE);
}

VOID dump_rxd_wed(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	UINT32 *dw0 = (UINT32 *)((UCHAR *)pRxD);
	UINT32 *dw1 = (UINT32 *)((UCHAR *)pRxD + 4);
	UINT32 *dw2 = (UINT32 *)((UCHAR *)pRxD + 8);
	UINT32 *dw3 = (UINT32 *)((UCHAR *)pRxD + 12);

	MTWF_PRINT("RxD:\n");
	MTWF_PRINT("\tRxD DW0: 0x%x\n",
				*dw0);
	MTWF_PRINT("\tRxD DW1: 0x%x\n",
				*dw1);
	MTWF_PRINT("\tRxD DW2: 0x%x\n",
				*dw2);
	MTWF_PRINT("\tRxD DW3: 0x%x\n",
				*dw3);
}

#ifdef CONFIG_WIFI_MSI_SUPPORT
VOID pci_handle_msi_irq(int irq, void *hif_chip)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)hif_chip;
	UCHAR msi_message;
	struct _PCI_HIF_T *pci_hif = pci_hif_chip->hif;
	struct _RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(pci_hif->net_dev);
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(pAd->hdev_ctrl);
	UINT32 value = DEFAULT_RID;

	msi_message = irq - pci_hif_chip->first_irq;

	if (msi_message >= ARRAY_SIZE(pAd->irq_num)) {
		MTWF_DBG(NULL, DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
		"Wrong MSI_IRQ number:%u\n", msi_message);
		return;
	}

	if (pci_hif_chip->is_main == TRUE) {
		pAd->irq_num[msi_message]++;

		if (chip_ops->get_rid_value())
			value = chip_ops->get_rid_value();
		if (value != DEFAULT_RID) {
			if (msi_message == MSI_IRQ2) {
				pci_hif_chip->msi_isr_message3(pci_hif_chip);
			} else if (msi_message == MSI_IRQ0 || msi_message == MSI_IRQ1 || msi_message == MSI_IRQ3 ||
				msi_message == MSI_IRQ4 || msi_message == MSI_IRQ5 || msi_message == MSI_IRQ6) {
				pci_hif_chip->isr(pci_hif_chip);
			}
		} else {
			if (msi_message == MSI_IRQ3) {
				pci_hif_chip->msi_isr_message3(pci_hif_chip);
			} else if (msi_message == MSI_IRQ4) {
				pci_hif_chip->msi_isr_message4(pci_hif_chip);
			} else if (msi_message == MSI_IRQ7 || msi_message == MSI_IRQ0 || msi_message == MSI_IRQ1 ||
				msi_message == MSI_IRQ2 || msi_message == MSI_IRQ5 || msi_message == MSI_IRQ6) {
				pci_hif_chip->isr(pci_hif_chip);
			}
		}
	} else {
		pAd->irq_num_pcie1[msi_message]++;

		if (msi_message == MSI_IRQ2) {
			pci_hif_chip->msi_isr_message2_pcie1(pci_hif_chip);
		} else if (msi_message == MSI_IRQ0 || msi_message == MSI_IRQ1 || msi_message == MSI_IRQ3) {
			pci_hif_chip->isr(pci_hif_chip);
		}
	}
}

#endif

/*
*
*/
VOID pci_handle_irq(void *hif_chip)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)hif_chip;

	pci_hif_chip->isr(pci_hif_chip);
}

#ifdef MULTI_INTR_SUPPORT
/*
*
*/
VOID pci_handle_multi_irq(void *hif_chip)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)hif_chip;

	pci_hif_chip->multi_isr(pci_hif_chip);
}

/*
*
*/
VOID pci_handle_multi_irq_2nd(void *hif_chip)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)hif_chip;

	pci_hif_chip->multi_isr_2nd(pci_hif_chip);
}

/*
*
*/
VOID pci_handle_multi_irq_3rd(void *hif_chip)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)hif_chip;

	pci_hif_chip->multi_isr_3rd(pci_hif_chip);
}

/*
*
*/
VOID pci_handle_multi_irq_4th(void *hif_chip)
{
	struct pci_hif_chip *pci_hif_chip = (struct pci_hif_chip *)hif_chip;

	pci_hif_chip->multi_isr_4th(pci_hif_chip);
}
#endif

/*
*
*/
inline UINT32 pci_get_tx_resource_free_num_nolock(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	UINT32 free_num;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);
	UINT16 tx_ring_size = tx_ring->ring_size;

	free_num = tx_ring->TxSwFreeIdx > tx_ring->TxCpuIdx ?
			   tx_ring->TxSwFreeIdx - tx_ring->TxCpuIdx - 1 :
			   tx_ring->TxSwFreeIdx + tx_ring_size - tx_ring->TxCpuIdx - 1;

	return free_num;
}

/*
*
*/
inline VOID pci_inc_resource_full_cnt(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	tx_ring->tx_ring_full_cnt++;
}


inline VOID pci_dec_resource_full_cnt(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	tx_ring->tx_ring_full_cnt--;
}
/*
*
*/
inline BOOLEAN pci_get_resource_state(RTMP_ADAPTER *pAd, UINT8 resource_idx)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	return test_bit(0, &tx_ring->tx_ring_state);
}

/*
*
*/
inline INT pci_set_resource_state(RTMP_ADAPTER *pAd, UINT8 resource_idx, BOOLEAN state)
{
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(hif, resource_idx);

	if (state == TX_RING_LOW)
		set_bit(0, &tx_ring->tx_ring_state);
	else
		clear_bit(0, &tx_ring->tx_ring_state);

	return NDIS_STATUS_SUCCESS;
}

