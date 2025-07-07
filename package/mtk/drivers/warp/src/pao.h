
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

#ifndef __PAO_H__
#define __PAO_H__

int wed_pao_hif_txd_init(struct wed_entry *wed, struct wifi_hw *hw);
int wed_pao_hif_txd_exit(struct wed_entry *wed, struct wifi_hw *hw);


#endif
