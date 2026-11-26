// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 MediaTek Inc.
 *
 * Author: Chris.Chou <chris.chou@mediatek.com>
 *         Ren-Ting Wang <ren-ting.wang@mediatek.com>
 *         Peter Wang <peterjy.wang@mediatek.com>
 */

#include <linux/bitops.h>

#include <mtk_eth_soc.h>
#include <mtk_hnat/hnat.h>
#include <mtk_hnat/nf_hnat_mtk.h>

#include <pce/cdrt.h>
#include <pce/cls.h>
#include <pce/netsys.h>

#include "crypto-eip/crypto-eip.h"
#include "crypto-eip/ddk-wrapper.h"
#include "crypto-eip/internal.h"

struct mtk_CDRT_DTLS_entry CDRT_DTLS_params;
struct DTLSResourceMgmt *dtls_table[CAPWAP_MAX_TUNNEL_NUM];

static int
mtk_setup_cdrt_dtls(struct cdrt_entry *cdrt_entry_p, enum cdrt_type type)
{
	struct cdrt_desc *cdesc = &cdrt_entry_p->desc;

	cdesc->desc1.dtls.pkt_len = 0;
	cdesc->desc1.dtls.rsv1 = 0;
	cdesc->desc1.dtls.capwap = 1;
	if (type == CDRT_ENCRYPT)
		cdesc->desc1.dtls.dir = 0;
	else
		cdesc->desc1.dtls.dir = 1;
	cdesc->desc1.dtls.content_type = 3;
	cdesc->desc1.dtls.type = 3;
	cdesc->desc1.aad_len = 0;
	cdesc->desc1.rsv1 = 0;
	cdesc->desc1.app_id = 0;
	cdesc->desc1.token_len = 0x30;
	cdesc->desc1.rsv2 = 0;
	cdesc->desc1.p_tr[0] = 0xfffffffc;
	cdesc->desc1.p_tr[1] = 0xffffffff;

	cdesc->desc2.usr = 0;
	cdesc->desc2.rsv1 = 0;
	cdesc->desc2.strip_pad = 1;
	cdesc->desc2.allow_pad = 1;
	cdesc->desc2.hw_srv = 0x28;
	cdesc->desc2.rsv2 = 0;
	cdesc->desc2.flow_lookup = 0;
	cdesc->desc2.rsv3 = 0;
	cdesc->desc2.ofs = 14;
	cdesc->desc2.next_hdr = 0;
	cdesc->desc2.fl = 0;
	cdesc->desc2.ip4_chksum = 0;
	if (type == CDRT_ENCRYPT)
		cdesc->desc2.l4_chksum = 1;
	else
		cdesc->desc2.l4_chksum = 0;
	cdesc->desc2.parse_eth = 0;
	cdesc->desc2.keep_outer = 0;
	cdesc->desc2.rsv4 = 0;
	cdesc->desc2.rsv5[0] = 0;
	cdesc->desc2.rsv5[1] = 0;

	cdesc->desc3.option_meta[0] = 0x00000000;
	cdesc->desc3.option_meta[1] = 0x00000000;
	cdesc->desc3.option_meta[2] = 0x00000000;
	cdesc->desc3.option_meta[3] = 0x00000000;

	return mtk_pce_cdrt_entry_write(cdrt_entry_p);
}


static int
mtk_add_cdrt_dtls(enum cdrt_type type)
{
	int ret = 0;
	struct cdrt_entry *cdrt_entry_p = NULL;

	cdrt_entry_p = mtk_pce_cdrt_entry_alloc(type);
	if (cdrt_entry_p == NULL) {
		CRYPTO_ERR("%s: mtk_pce_cdrt_entry_alloc failed!\n", __func__);
		return 1;
	}

	ret = mtk_setup_cdrt_dtls(cdrt_entry_p, type);
	if (ret)
		goto free_cdrt;

	if (type == CDRT_DECRYPT)
		CDRT_DTLS_params.cdrt_inbound = cdrt_entry_p;
	else
		CDRT_DTLS_params.cdrt_outbound = cdrt_entry_p;
	return ret;

free_cdrt:
	mtk_pce_cdrt_entry_free(cdrt_entry_p);

	return ret;
}


void
mtk_update_cdrt_idx(struct mtk_cdrt_idx_param *cdrt_idx_params_p)
{
	cdrt_idx_params_p->cdrt_idx_inbound = CDRT_DTLS_params.cdrt_inbound->idx;
	cdrt_idx_params_p->cdrt_idx_outbound = CDRT_DTLS_params.cdrt_outbound->idx;
}
EXPORT_SYMBOL(mtk_update_cdrt_idx);


void
mtk_dtls_capwap_init(void)
{
	int i = 0;
	// init cdrt for dtls
	if (mtk_add_cdrt_dtls(CDRT_DECRYPT))
		CRYPTO_ERR("%s: CDRT DECRYPT for DTLS init failed!\n", __func__);

	if (mtk_add_cdrt_dtls(CDRT_ENCRYPT))
		CRYPTO_ERR("%s: CDRT ENCRYPT for DTLS init failed!\n", __func__);
	// add hook function for tops driver

	// init table as NULL
	for (i = 0; i < CAPWAP_MAX_TUNNEL_NUM; i++)
		dtls_table[i] = NULL;
}


void
mtk_dtls_capwap_deinit(void)
{
	int i = 0;
	// Loop and check if all SA in table are freed
	for (i = 0; i < CAPWAP_MAX_TUNNEL_NUM; i++) {
		if (dtls_table[i] != NULL) {
			mtk_ddk_remove_dtls_param(dtls_table[i]);
			kfree(dtls_table[i]);
			dtls_table[i] = NULL;
		}
	}

	if (CDRT_DTLS_params.cdrt_inbound != NULL)
		mtk_pce_cdrt_entry_free(CDRT_DTLS_params.cdrt_inbound);
	if (CDRT_DTLS_params.cdrt_outbound != NULL)
		mtk_pce_cdrt_entry_free(CDRT_DTLS_params.cdrt_outbound);
}

void
mtk_update_dtls_param(struct DTLS_param *DTLSParam_p, int TnlIdx)
{
	int ret;

	if (dtls_table[TnlIdx]) {
		CRYPTO_NOTICE("tnl_idx-%d- existed, will be removed first.\n", TnlIdx);
		mtk_ddk_remove_dtls_param(dtls_table[TnlIdx]);
		kfree(dtls_table[TnlIdx]);
		dtls_table[TnlIdx] = NULL;
	} else {
		dtls_table[TnlIdx] = kmalloc(sizeof(struct DTLSResourceMgmt), GFP_KERNEL);
		if (!dtls_table[TnlIdx]) {
			CRYPTO_ERR("%s: kmalloc for dtls resource table failed\n", __func__);
			return;
		}
		memset(dtls_table[TnlIdx], 0, sizeof(struct DTLSResourceMgmt));
	}

	/* Setup Transform Record for DTLS */
	dtls_table[TnlIdx]->sa_out.p =
			mtk_ddk_tr_capwap_dtls_build(true, DTLSParam_p, SAB_DIRECTION_OUTBOUND);
	if (!dtls_table[TnlIdx]->sa_out.p) {
		CRYPTO_ERR("%s: sa_out build failed\n", __func__);
		kfree(dtls_table[TnlIdx]);
		dtls_table[TnlIdx] = NULL;
		return;
	}

	dtls_table[TnlIdx]->sa_in.p =
			mtk_ddk_tr_capwap_dtls_build(true, DTLSParam_p, SAB_DIRECTION_INBOUND);
	if (!dtls_table[TnlIdx]->sa_in.p) {
		CRYPTO_ERR("%s: sa_in build failed\n", __func__);
		goto failed;
	}

	/* Setup DTL Lookup */
	ret = mtk_ddk_pcl_capwap_dtls_build(DTLSParam_p,
						dtls_table[TnlIdx], SAB_DIRECTION_OUTBOUND);
	if (ret < 0) {
		CRYPTO_ERR("%s: PCL DTL Outbound Setup failed\n", __func__);
		goto failed;
	}

	ret = mtk_ddk_pcl_capwap_dtls_build(DTLSParam_p,
						dtls_table[TnlIdx], SAB_DIRECTION_INBOUND);
	if (ret < 0) {
		CRYPTO_ERR("%s: PCL DTL Inbound Setup failed\n", __func__);
		mtk_ddk_remove_dtls_pcl(dtls_table[TnlIdx], SAB_DIRECTION_OUTBOUND);
		goto failed;
	}

	DTLSParam_p->SA_encrypt = dtls_table[TnlIdx]->sa_out.p;
	DTLSParam_p->SA_decrypt = dtls_table[TnlIdx]->sa_in.p;

	return;

failed:
	mtk_ddk_remove_dtls_sa(dtls_table[TnlIdx]);
	kfree(dtls_table[TnlIdx]);
	dtls_table[TnlIdx] = NULL;
	return;
}
EXPORT_SYMBOL(mtk_update_dtls_param);

void mtk_remove_dtls_param(struct DTLS_param *DTLSParam_p, int TnlIdx)
{
	CRYPTO_INFO("%s: Remove TnlIdx=%d\n", __func__, TnlIdx);
	mtk_ddk_remove_dtls_param(dtls_table[TnlIdx]);
	kfree(dtls_table[TnlIdx]);
	dtls_table[TnlIdx] = NULL;
}
EXPORT_SYMBOL(mtk_remove_dtls_param);
