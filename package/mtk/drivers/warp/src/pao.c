/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: pao
*/

#include <linux/pci.h>
#include "wed.h"
#include "warp_wifi.h"

int
wed_pao_hif_txd_init(struct wed_entry *wed, struct wifi_hw *hw)
{
	struct wed_pao_ctrl *pao_ctrl = &wed->res_ctrl.tx_ctrl.pao_ctrl;
	struct pci_dev *wdev = hw->hif_dev;
	int i = 0, j = 0;

	/* TODO: set segment_num */
	pao_ctrl->hif_txd_segment_nums = 32;

	for (i = 0; i < pao_ctrl->hif_txd_segment_nums; i++) {
		/* each segment is 64K */
		pao_ctrl->hif_txd_addr[i] =
			(char *)__get_free_pages(GFP_ATOMIC | GFP_DMA32 | __GFP_ZERO, 4);

		if (!pao_ctrl->hif_txd_addr[i])
			goto err;

		pao_ctrl->hif_txd_addr_pa[i] = dma_map_single(&wdev->dev, pao_ctrl->hif_txd_addr[i],
				64 * 1024, DMA_TO_DEVICE);


		if (unlikely(dma_mapping_error(&wdev->dev, pao_ctrl->hif_txd_addr_pa[i]))) {
			warp_dbg(WARP_DBG_ERR, "%s(): dma map fail\n", __func__);
			goto err;
		}


	}

	pao_ctrl->hif_txd_segment_nums = i;

	return 0;

err:
	for (j = 0; j < i; j++)
		dma_unmap_single(&wdev->dev, pao_ctrl->hif_txd_addr_pa[j],
			64 * 1024, DMA_TO_DEVICE);

	return -1;
}

int
wed_pao_hif_txd_exit(struct wed_entry *wed, struct wifi_hw *hw)
{
	int i;
	struct wed_pao_ctrl *pao_ctrl = &wed->res_ctrl.tx_ctrl.pao_ctrl;
	struct pci_dev *wdev = hw->hif_dev;

	for (i = 0; i <pao_ctrl->hif_txd_segment_nums; i++) {
		dma_unmap_single(&wdev->dev, pao_ctrl->hif_txd_addr_pa[i],
			64 * 1024, DMA_TO_DEVICE);
		free_pages((unsigned long)pao_ctrl->hif_txd_addr[i], 4);
	}

	return 0;
};

