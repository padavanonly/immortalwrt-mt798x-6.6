/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2019, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#include "rt_config.h"
#include "bss_mngr.h"

struct bss_manager bss_mngr;

static const char * const dev_type_str[] = {
	"Legacy",
	"11vT",
	"11vNT",
	"CoH"
};

static const char * const rnr_rule_str[] = {
	"None",
	"All",
};

static bool bss_idx_assigned(u8 idx)
{
	if (bss_mngr.bss_idx_bitmap) {
		if (bss_mngr.bss_idx_bitmap[idx / 32] & (1 << (idx % 32)))
			return TRUE;
	}

	return FALSE;
}

static void bss_idx_set(u8 idx)
{
	if (bss_mngr.bss_idx_bitmap)
		bss_mngr.bss_idx_bitmap[idx / 32] |= (1 << (idx % 32));
}

static void bss_idx_clear(u8 idx)
{
	if (bss_mngr.bss_idx_bitmap)
		bss_mngr.bss_idx_bitmap[idx / 32]  &= ~(1 << (idx % 32));
}

static u8 bss_idx_aquire(void)
{
	u16 bssidx;

	for (bssidx = 0; bssidx < BMGR_MAX_BSS_CNT; bssidx++) {
		if (!bss_idx_assigned(bssidx)) {
			bss_idx_set(bssidx);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					 ": found non-occupied bssidx:%d\n",
					  bssidx);
			break;
		}
	}

	if (!BMGR_VALID_BSS_IDX(bssidx))
		bssidx = BMGR_INVALID_BSS_IDX;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, ": bssidx [%d]\n", bssidx);

	return bssidx;
}

static void bss_idx_bitmap_init(void)
{
	u32 *bss_idx_bitmap = NULL;
	u32 map_size = 0;

	/* allocate up to can contain BMGR_MAX_BSS_CNT */
	map_size = ((BMGR_MAX_BSS_CNT + 31) / 32) * sizeof(u32);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"map_size=%d\n", map_size);

	os_alloc_mem(NULL, (u8 **)&bss_idx_bitmap, map_size);
	if (bss_idx_bitmap == NULL) {
		dump_stack();
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				 ": Allocate memory size:%d for bss_idx_bitmap failed!\n",
				  map_size);
		return;
	}

	os_zero_mem(bss_idx_bitmap, map_size);
	bss_mngr.bss_idx_bitmap = bss_idx_bitmap;
}

static void bss_idx_bitmap_deinit(void)
{
	u32 *bss_idx_bitmap = bss_mngr.bss_idx_bitmap;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "--->\n");

	if (bss_idx_bitmap)
		os_free_mem(bss_idx_bitmap);

	bss_mngr.bss_idx_bitmap = NULL;
}


/*
 * get tx-bss index of nt-bss
 */
static u8 get_tx_bss_idx(IN struct bmgr_entry *entry)
{
	struct bmgr_entry *bmg_entry = NULL;
	u8 dev_type = entry->entry_info.dev_type;
	int i;
	u8 bss_idx_tx = entry->bss_idx;

	if (IS_DEV_TYPE_11V_NT(dev_type)) {
		for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
			bmg_entry = GET_BSS_ENTRY_BY_IDX(i);
			/* search 1st tx-bss on the same band */
			if (bmg_entry
				&& (bmg_entry->entry_info.dev_type == DEV_TYPE_AP_11VT)
				&& (bmg_entry->entry_info.t_bss == entry->entry_info.t_bss)) {
				bss_idx_tx = i;
				break;
			}
		}
	}

	return bss_idx_tx;
}

#ifdef DOT11_EHT_BE
static struct bss_mngr_event_ops *bmgr_event_ops_get(void)
{
	return bss_mngr.event_ops;
}

static bool mld_grp_assigned(u8 idx)
{
	if (bss_mngr.mld_grp_bitmap) {
		if (bss_mngr.mld_grp_bitmap[idx / 32] & (1 << (idx % 32)))
			return TRUE;
	}

	return FALSE;
}

static void mld_grp_set(u8 idx)
{
	if (bss_mngr.mld_grp_bitmap)
		bss_mngr.mld_grp_bitmap[idx / 32] |= (1 << (idx % 32));
}

static void mld_grp_clear(u8 idx)
{
	if (bss_mngr.mld_grp_bitmap)
		bss_mngr.mld_grp_bitmap[idx / 32]  &= ~(1 << (idx % 32));
}

/*
 * acquire mld_grp_idx from base (inclusive) to BMGR_MAX_MLD_GRP_CNT (exclusive)
 * base: shall be larger than zero.
 */
static u8 mld_grp_aquire(u8 base)
{
	u16 mld_grp = BMGR_MAX_MLD_GRP_CNT;

	if ((base == 0) || (base > BMGR_MAX_MLD_GRP_CNT))
		goto err;

	/* valid mld group starts from 1 */
	for (mld_grp = base; mld_grp < BMGR_MAX_MLD_GRP_CNT; mld_grp++) {
		if (!mld_grp_assigned(mld_grp)) {
			mld_grp_set(mld_grp);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					 ": found non-occupied mld_grp:%d\n",
					  mld_grp);
			break;
		}
	}

	if (!BMGR_VALID_MLD_GRP_IDX(mld_grp))
		mld_grp = BMGR_INVALID_MLD_GRP;

err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "mld_grp [%d]\n", mld_grp);
	return mld_grp;
}

static void mld_grp_bitmap_init(void)
{
	u32 *mld_grp_bitmap = NULL;
	u32 map_size = 0;

	/* allocate up to can contain BMGR_MAX_MLD_GRP_CNT */
	map_size = ((BMGR_MAX_MLD_GRP_CNT + 31) / 32) * sizeof(u32);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			": map_size=%d\n", map_size);

	os_alloc_mem(NULL, (u8 **)&mld_grp_bitmap, map_size);
	if (mld_grp_bitmap == NULL) {
		dump_stack();
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				 ": Allocate memory size:%d for mld_grp_bitmap failed!\n",
				  map_size);
		return;
	}

	os_zero_mem(mld_grp_bitmap, map_size);
	bss_mngr.mld_grp_bitmap = mld_grp_bitmap;
}

static void mld_grp_bitmap_deinit(void)
{
	u32 *mld_grp_bitmap = bss_mngr.mld_grp_bitmap;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "--->\n");

	if (mld_grp_bitmap)
		os_free_mem(mld_grp_bitmap);

	bss_mngr.mld_grp_bitmap = NULL;
}

static void mld_sta_clear(u16 mld_sta_idx)
{
	struct bmgr_mld_sta *mld_sta = NULL;

	if (BMGR_VALID_MLD_STA(mld_sta_idx)) {
		mld_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
		os_zero_mem(mld_sta, sizeof(struct bmgr_mld_sta));

		/* init setting */
		mld_sta->idx = mld_sta_idx;
		mld_sta->aid = INVALID_AID;
	}
}

static void mld_sta_init(void)
{
	u16 idx;

	for (idx = 0; BMGR_VALID_MLD_STA(idx); idx++)
		mld_sta_clear(idx);
}

static void mld_twt_init(struct bss_mlo_twt *mlo_twt_ctrl)
{
	if (!mlo_twt_ctrl)
		return;

	NdisAllocateSpinLock(NULL, &mlo_twt_ctrl->lock);
	INIT_LIST_HEAD(&mlo_twt_ctrl->breakdown_action_frame_list);
}

static void mld_twt_deinit(struct bss_mlo_twt *mlo_twt_ctrl)
{
	BOOLEAN cancelled;
	RALINK_TIMER_STRUCT *timer = NULL;

	if (!mlo_twt_ctrl)
		return;

	timer = &mlo_twt_ctrl->agg_action_frame_timer;
	RTMPCancelTimer(timer, &cancelled);
	RTMPReleaseTimer(timer, &cancelled);

	NdisFreeSpinLock(&mlo_twt_ctrl->lock);
	INIT_LIST_HEAD(&mlo_twt_ctrl->breakdown_action_frame_list);
}

static u16 mld_sta_acquire(u8 *mld_addr, u8 frm_subtype, u8 *setup_link_addr)
{
	u16 idx;
	u16 first_unused_idx = BMGR_MAX_MLD_STA_CNT;
	struct bmgr_mld_sta *mld_sta = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			": mld_addr %pM\n", mld_addr);

	for (idx = 0; BMGR_VALID_MLD_STA(idx); idx++) {
		mld_sta = GET_MLD_STA_BY_IDX(idx);
		if ((mld_sta->valid == 0) && (first_unused_idx == BMGR_MAX_MLD_STA_CNT)) {
			first_unused_idx = idx;
		} else if (mld_sta->valid && NdisEqualMemory(mld_sta->mld_addr, mld_addr, MAC_ADDR_LEN)) {
			if (frm_subtype == SUBTYPE_AUTH) {
				/* clear mld_sta and all pEntry if receive
					Auth.Req again from the same MLD */
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					"Auth same MLD peer(%d)(%pM), clear peer MLD entries\n", idx, mld_addr);

				bss_mngr_con_del_mld_sta(mld_sta, MLD_DISC_OP_DEL_STA_N_ACT);

				mld_sta_clear(idx);
			} else if (frm_subtype == SUBTYPE_ASSOC_REQ) {
				u8 link_id;

				/* setup link shall be one of link of existing MLD STA */
				for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
					if (mld_sta->mld_link[link_id].active &&
						MAC_ADDR_EQUAL(mld_sta->mld_link[link_id].link_addr, setup_link_addr)) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							"Assoc same MLD peer(%d)(%pM), it's valid (re)assoc: %pM\n",
							idx, mld_addr, setup_link_addr);
						return idx;
					}
				}

				/* unknown setup link addr with existing mld addr */
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Assoc same MLD peer(%d)(%pM), but unknown setup link addr: %pM\n",
					idx, mld_addr, setup_link_addr);
				return BMGR_MAX_MLD_STA_CNT;
			} else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\tre-use mld_sta_idx:%d\n", idx);
			return idx;
		}
	}

	if (BMGR_VALID_MLD_STA(first_unused_idx)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tun-used mld_sta_idx:%d\n", first_unused_idx);
		return first_unused_idx;
	}

	return BMGR_MAX_MLD_STA_CNT;
}

static void mld_sta_release(u16 mld_sta_idx)
{
	struct bmgr_mld_sta *mld_sta = NULL;

	if (BMGR_VALID_MLD_STA(mld_sta_idx)) {
		mld_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"(%pS): mld_sta_idx(%d), mld_addr %pM\n",
			OS_TRACE, mld_sta_idx, mld_sta->mld_addr);

		/* clear mld station entry */
		mld_sta_clear(mld_sta_idx);
	}
}

static bool bss_idx_mld_inited(IN struct bmgr_entry *entry)
{
	/* mld buffer allocated */
	if (entry->mld.buf && entry->mld_cmm.buf) {
		/* entry is mlo_en and MLD exists */
		if (BMGR_VALID_MLO_BSS_ENTRY(entry) && BMGR_VALID_MLO_DEV(entry->mld_ptr))
			return true;
	}

	return false;
}

static void bmgr_mld_reset(IN struct bmgr_mlo_dev *mld)
{
	u8 i;

	mld->mld_link_cnt = 0;
	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++)
		mld->bss_idx_mld[i] = BMGR_INVALID_BSS_IDX;

	a2tlm_contract_reset(mld);
}

static bool alloc_bcn_appl_elem_per_sta_pf(struct bmgr_entry *entry)
{
	u8 repting_link;
	struct ie *per_sta_pf = NULL;
	u8 bss_idx_mld;
	struct bmgr_mlo_dev *mld = NULL;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, bss_idx (%d) in mld (%d) is invalid\n",
			entry->bss_idx, entry->entry_info.mld_grp);
		return FALSE;
	}

	for (repting_link = 0; repting_link < BSS_MNGR_MAX_BAND_NUM; repting_link++) {
		bss_idx_mld = mld->bss_idx_mld[repting_link];

		if (!BMGR_VALID_BSS_IDX(bss_idx_mld))
			continue;

		per_sta_pf = &entry->bcn_appl_elem_per_sta_pf[repting_link];
		if (!per_sta_pf->buf) {
			os_alloc_mem(NULL, (UCHAR **)&per_sta_pf->buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
			if (!per_sta_pf->buf) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Alloc buf for bcn_appl_elem_per_sta_pf failed.\n");
				return FALSE;
			}
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"The per-sta profile buffer(%p) not NULL!\n", per_sta_pf->buf);

		os_zero_mem(per_sta_pf->buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
		per_sta_pf->len = 0;
		entry->csa.csa_offset_in_per_sta_pf[repting_link] = 0;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"Allocate the per-sta-prf buffer[%p] for repted_link[%d] entry repting_link[%d]!\n",
			per_sta_pf->buf, entry->link_id, repting_link);
	}

	return TRUE;
}

static void free_bcn_appl_elem_per_sta_pf(struct bmgr_entry *entry)
{
	u8 link;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "\n");

	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		if (entry->bcn_appl_elem_per_sta_pf[link].buf) {
			os_free_mem(entry->bcn_appl_elem_per_sta_pf[link].buf);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"free (bcn_per_sta_pf[%d]:%p)\n",
				link, entry->bcn_appl_elem_per_sta_pf[link].buf);

			entry->bcn_appl_elem_per_sta_pf[link].buf = NULL;
			entry->bcn_appl_elem_per_sta_pf[link].len = 0;
			entry->csa.csa_offset_in_per_sta_pf[link] = 0;
		}
	}
}

static void clean_bcn_appl_elem_per_sta_pf(struct bmgr_entry *entry)
{
	u8 link;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "\n");

	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		if (entry->bcn_appl_elem_per_sta_pf[link].buf) {
			os_zero_mem(entry->bcn_appl_elem_per_sta_pf[link].buf,
				BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"Clean (bcn_per_sta_pf[%d]:%p)\n",
				link, entry->bcn_appl_elem_per_sta_pf[link].buf);

			entry->bcn_appl_elem_per_sta_pf[link].len = 0;
			entry->csa.csa_offset_in_per_sta_pf[link] = 0;
		}
	}
}

/* add appl_elem into per_sta_profile buf for mlo reporting links,
	the buf will be used in bcn/probe_rsp composing */
static void mld_add_appl_elem(
	struct bmgr_entry *repted_entry,
	IN u8 *buf,
	IN u16 buf_len
)
{
	u8 repted_link, repting_link;
	u8 repted_bss_idx;
	u8 *f_buf, *pos, *tmp_buf;
	u16 cap_ie, ie_len;
	struct bmgr_mlo_dev *mld = NULL;
	struct ie *per_sta_pf = NULL;
	struct ie *tmp_per_sta_pf = NULL;

	if (!repted_entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: repted_entry is NULL!\n");
		return;
	}

	if (!buf) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: buf is NULL!\n");
		return;
	}

	mld = repted_entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"bss_idx (%d) is not in mld (%d)\n",
			repted_entry->bss_idx, repted_entry->entry_info.mld_grp);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE, "\n");

	/* if true => do not need Per-STA Profile */
	if (!BMGR_VALID_MLO_BSS_ENTRY(repted_entry) || mld->mld_link_cnt < 2)
		return;

	if (!alloc_bcn_appl_elem_per_sta_pf(repted_entry))
		return;

	{
		u8 link_id;

		for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"per-sta-prf buffer[%p-%d] for repted_link[%d] entry repting_link[%d].\n",
				repted_entry->bcn_appl_elem_per_sta_pf[link_id].buf,
				repted_entry->bcn_appl_elem_per_sta_pf[link_id].len, repted_entry->link_id, link_id);
		}
	}

	/* Loop each link to append element */

	/* repted_link: the link that is reported in Per-STA Profile in ML IE */
	repted_bss_idx = repted_entry->bss_idx;
	repted_link = repted_entry->link_id;
	tmp_per_sta_pf = &repted_entry->bcn_appl_elem_per_sta_pf[repted_link];
	tmp_buf = tmp_per_sta_pf->buf;

	/* build cap ie */
	ie_len = PER_STA_PROILE_CAP_INFO_SIZE;
	cap_ie = cpu2le16(repted_entry->entry_info.cap_info);
	NdisMoveMemory(tmp_buf, &cap_ie, ie_len);
	tmp_per_sta_pf->len += ie_len;
	tmp_buf += tmp_per_sta_pf->len;

	/* append input buf (IEs) */
	NdisMoveMemory(tmp_buf, buf, buf_len);
	tmp_per_sta_pf->len += buf_len;

	/* repting_link: the link that will tx the frm */
	for (repting_link = 0; repting_link < BSS_MNGR_MAX_BAND_NUM; repting_link++) {
		if (repted_link == repting_link)
			continue;

		/*
		 *  Ex. repted entry [if link 0]
		 *    bcn_appl_elem_per_sta_pf
		 *      [0]: Per-STA Profile of repted [link 0] for repting [link 0]
		 *            -> (used for tmp buf)
		 *      [1]: Per-STA Profile of repted [link 0] for repting [link 1]
		 *      [2]: Per-STA Profile of repted [link 0] for repting [link 2]
		 *
		 *  if peer transmits bcn by link 1:
		 *     - [link 0 entry] deliver bcn_appl_elem_per_sta_pf[1] to [link 1].
		 *     - [link 1 entry] skip.
		 *     - [link 2 entry] deliver bcn_appl_elem_per_sta_pf[1] to [link 1].
		 */
		per_sta_pf = &repted_entry->bcn_appl_elem_per_sta_pf[repting_link];

		if (per_sta_pf->buf) {
			f_buf = pos = per_sta_pf->buf;
			pos = build_multi_link_per_sta_profile(
					repted_entry,
					f_buf,
					BMGR_MLD_APPL_ELEM_STA_PROFILE,
					BMGR_MAX_MLD_STA_CNT,
					repting_link);
			per_sta_pf->len = pos - f_buf;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"[%p] repted link %d, per-sta profile len %d for repting link %d!\n",
				per_sta_pf, repted_link, per_sta_pf->len, repting_link);
		}
	}
}

/* remove appl_elem from per_sta_profile buf for mlo reporting links,
	the buf will be used in bcn/probe_rsp composing */
static void mld_remove_appl_elem(struct bmgr_entry *repted_entry)
{
	if (!repted_entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: repted_entry is NULL!\n");
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE, "\n");
	clean_bcn_appl_elem_per_sta_pf(repted_entry);
}

static void mld_free_reconfig_ml_ie(struct bmgr_mlo_dev *mld)
{
	struct reconfig_ml_ie_t *reconf_ie;
	u8 link;

	if (!mld) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, mld (%d) is null\n",
			mld ? mld->mld_grp : BMGR_INVALID_MLD_GRP);
		return;
	}

	reconf_ie = &mld->reconf_ie;
	if (reconf_ie->ie_status < BUF_STAT_ALLOCED)
		return;

	reconf_ie->ie_status = BUF_STAT_NULL;

	reconf_ie->common_info.len = 0;
	if (reconf_ie->common_info.buf) {
		os_free_mem(reconf_ie->common_info.buf);
		reconf_ie->common_info.buf = NULL;
	}

	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		reconf_ie->link_info[link].len = 0;
		if (reconf_ie->link_info[link].buf) {
			os_free_mem(reconf_ie->link_info[link].buf);
			reconf_ie->link_info[link].buf = NULL;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
		"Free success, ie status %d\n", reconf_ie->ie_status);
}

static void mld_clean_reconfig_ml_ie(struct bmgr_mlo_dev *mld)
{
	struct reconfig_ml_ie_t *reconf_ie;
	u8 link;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, mld (%d) is invalid\n",
			mld ? mld->mld_grp : BMGR_INVALID_MLD_GRP);
		return;
	}

	reconf_ie = &mld->reconf_ie;
	if (reconf_ie->ie_status < BUF_STAT_ALLOCED)
		return;

	reconf_ie->ie_status = BUF_STAT_ALLOCED;

	reconf_ie->common_info.len = 0;
	if (reconf_ie->common_info.buf)
		NdisZeroMemory(reconf_ie->common_info.buf, MAX_RECONFIG_ML_IE_COMMON_LEN);

	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		reconf_ie->fw_bss_idx[link] = 0; /* TBD */
		reconf_ie->tmr_offset[link] = 0;
		reconf_ie->link_info[link].len = 0;
		if (reconf_ie->link_info[link].buf)
			NdisZeroMemory(reconf_ie->link_info[link].buf,
				MAX_RECONFIG_ML_IE_PER_LINK_INFO_LEN);
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
		"Clean success, ie status %d\n", reconf_ie->ie_status);
}

static bool mld_alloc_reconfig_ml_ie(struct bmgr_mlo_dev *mld)
{
	struct reconfig_ml_ie_t *reconf_ie;
	u8 *buf, link;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, mld (%d) is invalid\n",
			mld ? mld->mld_grp : BMGR_INVALID_MLD_GRP);
		return FALSE;
	}

	reconf_ie = &mld->reconf_ie;
	if (reconf_ie->ie_status >= BUF_STAT_ALLOCED)
		return TRUE;

	/* alloc common part */
	reconf_ie->common_info.len = 0;
	if (!reconf_ie->common_info.buf) {
		os_alloc_mem(NULL, &buf, MAX_RECONFIG_ML_IE_COMMON_LEN);
		if (!buf)
			goto err;
		reconf_ie->common_info.buf = buf;
	}

	/* alloc per-link part */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		reconf_ie->link_info[link].len = 0;
		if (!reconf_ie->link_info[link].buf) {
			os_alloc_mem(NULL, &buf, MAX_RECONFIG_ML_IE_PER_LINK_INFO_LEN);
			if (!buf)
				goto err;
			reconf_ie->link_info[link].buf = buf;
		}
	}

	reconf_ie->ie_status = BUF_STAT_ALLOCED;

	mld_clean_reconfig_ml_ie(mld);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
		"Alloc success, ie status %d\n", reconf_ie->ie_status);

	return TRUE;
err:
	mld_free_reconfig_ml_ie(mld);

	return FALSE;
}
#endif

static struct bmgr_entry *get_bss_entry_by_netdev(
	IN PNET_DEV pNetDev
)
{
	struct bmgr_entry *entry = NULL;
	int i;

	if (!pNetDev)
		return NULL;

	for (i = 0; BMGR_VALID_BSS_IDX(i); i++) {
		entry = bss_mngr.entry[i];

		if (entry && (entry->pwdev == RtmpOsGetNetDevWdev(pNetDev)))
			return entry;
	}

	return NULL;
}

#ifdef DOT11V_MBSSID_SUPPORT
int bss_mngr_con_ie_update_mbss(struct bmgr_entry *entry)
{
	u8 dev_type = entry->entry_info.dev_type;
	u8 bss_idx = entry->bss_idx;
	u8 bss_idx_tx = entry->bss_idx_tx;
	struct bmgr_entry *tx_entry = NULL, *mbss_entry = NULL;
	int i;
	u8 *f_buf_bcn = NULL, *pos_bcn = NULL;
	u16 ies_len_bcn = 0;
	u8 *f_buf_probe = NULL, *pos_probe = NULL;
	u16 ies_len_probe = 0;
	u8 *f_buf_probe_cmm = NULL, *pos_probe_cmm = NULL;
	u16 ies_len_probe_cmm = 0;
	u8 *tim_ie_ptr;
	u8 *mlt_ie_ptr;
	u8 *cap_info_ptr;
	u8 mbssid_idx = 0;
	u8 probe_mbssie_cnt = 0;
	u8 probe_cmm_mbssie_cnt = 0;
	struct mbssie_info *probe_mbssie_info;
	struct mbssie_info *probe_cmm_mbssie_info;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"[%d]'s tx-bss is [%d]\n", bss_idx, bss_idx_tx);

	if (IS_DEV_TYPE_11V(dev_type)) {
		if (BMGR_VALID_BSS_IDX(bss_idx_tx))
			tx_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_tx);
		else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\t[%d] TX-BSS undefined\n", bss_idx);
			return NDIS_STATUS_FAILURE;
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"\t[%d] no MBSS_IE required\n", bss_idx);
		return NDIS_STATUS_SUCCESS;
	}

	f_buf_bcn = pos_bcn = tx_entry->mbss.tx_bcn.buf;
	tx_entry->mbss.tx_bcn.len = 0;
	f_buf_probe = pos_probe = tx_entry->mbss.tx_probe.buf;
	tx_entry->mbss.tx_probe.len = 0;
	f_buf_probe_cmm = pos_probe_cmm = tx_entry->mbss.tx_probe_cmm.buf;
	tx_entry->mbss.tx_probe_cmm.len = 0;

	/*store per entry mbss ie info*/
	probe_mbssie_info = &tx_entry->mbss.tx_probe_mbssie;
	probe_cmm_mbssie_info = &tx_entry->mbss.tx_probe_cmm_mbssie;

	tim_ie_ptr = f_buf_bcn;
	mlt_ie_ptr = f_buf_bcn;
	cap_info_ptr = f_buf_bcn;
	/* mbssid ie */
	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		mbss_entry = GET_BSS_ENTRY_BY_IDX(i);
		if (BMGR_VALID_BSS_ENTRY(mbss_entry)) {
			if (IS_DEV_TYPE_11V_NT(mbss_entry->entry_info.dev_type) && (mbss_entry->bss_idx_tx == bss_idx_tx)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\tbuild ie for ntx-bss[%d]\n", i);

				/* build beacon ie(s) for non-tx bss */
				mbssid_idx = mbss_entry->entry_info.dot11v_mbssid_idx;

				/* build ie(s) for beacon */
				/* mbss ie w/ ml ie */
				pos_bcn = build_multi_bssid_ie(mbss_entry, f_buf_bcn, FALSE, TRUE, &tim_ie_ptr, &mlt_ie_ptr, &cap_info_ptr);

				ies_len_bcn = pos_bcn - f_buf_bcn;
				tx_entry->mbss.tx_bcn.len += ies_len_bcn;
				f_buf_bcn = pos_bcn;

				/* beacon tim ie offset */
				tx_entry->mbss.bcn_tim_offset[mbssid_idx] =
					(u16)(tim_ie_ptr - tx_entry->mbss.tx_bcn.buf);
				/* beacon cap info offset */
				tx_entry->mbss.bcn_cap_info_offset[mbssid_idx] =
					(u16)(cap_info_ptr - tx_entry->mbss.tx_bcn.buf);

				/* beacon multi link traffic ie offset */
				tx_entry->mbss.bcn_mlt_offset[mbssid_idx] =
					(u16)(mlt_ie_ptr - tx_entry->mbss.tx_bcn.buf);

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\ttim offset(%d), mlt offset(%d), cap info offset(%d)\n",
						tx_entry->mbss.bcn_tim_offset[mbssid_idx],
						tx_entry->mbss.bcn_mlt_offset[mbssid_idx],
						tx_entry->mbss.bcn_cap_info_offset[mbssid_idx]);

				/* build ie(s) for probe.rsp, w/o ml ie */
				pos_probe = build_multi_bssid_ie(mbss_entry, f_buf_probe, TRUE, FALSE, NULL, NULL, NULL);
				ies_len_probe = pos_probe - f_buf_probe;

				/*store per entry mbss ie info*/
				probe_mbssie_info->mbssid_idx[probe_mbssie_cnt] = mbssid_idx;
				probe_mbssie_info->mbssie_offset[probe_mbssie_cnt]
					= f_buf_probe - tx_entry->mbss.tx_probe.buf;
				probe_mbssie_info->mbssie_len[probe_mbssie_cnt] = ies_len_probe;
				probe_mbssie_info->mbssie_num = (++probe_mbssie_cnt);
				tx_entry->mbss.tx_probe.len += ies_len_probe;
				f_buf_probe = pos_probe;

				/* build ie(s) for probe.rsp, w/ ml ie cmm part */
				pos_probe_cmm = build_multi_bssid_ie(mbss_entry, f_buf_probe_cmm, TRUE, TRUE, NULL, NULL, NULL);
				ies_len_probe_cmm = pos_probe_cmm - f_buf_probe_cmm;
				/*store per entry mbss ie info*/
				probe_cmm_mbssie_info->mbssid_idx[probe_cmm_mbssie_cnt] = mbssid_idx;
				probe_cmm_mbssie_info->mbssie_offset[probe_cmm_mbssie_cnt]
					= f_buf_probe_cmm - tx_entry->mbss.tx_probe_cmm.buf;
				probe_cmm_mbssie_info->mbssie_len[probe_cmm_mbssie_cnt] = ies_len_probe_cmm;
				probe_cmm_mbssie_info->mbssie_num = (++probe_cmm_mbssie_cnt);
				tx_entry->mbss.tx_probe_cmm.len += ies_len_probe_cmm;
				f_buf_probe_cmm = pos_probe_cmm;

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\tEND: add NT[%d] to T[%d], Len(IE/Total)=B(%d/%d) P(%d/%d) P-C(%d/%d)\n",
						i, bss_idx_tx,
						ies_len_bcn, tx_entry->mbss.tx_bcn.len,
						ies_len_probe, tx_entry->mbss.tx_probe.len,
						ies_len_probe_cmm, tx_entry->mbss.tx_probe_cmm.len);
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

static bool is_6g_rnr_required(
	struct bmgr_entry *repting_entry, struct bmgr_entry *repted_entry)
{
	if (!BMGR_VALID_BSS_ENTRY(repting_entry) || !BMGR_VALID_BSS_ENTRY(repted_entry))
		return FALSE;

	if (!IS_DEV_TYPE_W_BCN(repting_entry->entry_info.dev_type)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[repting:%d, repted:%d] FALSE, repting do not tx bcn\n",
			repting_entry->bss_idx, repted_entry->bss_idx);
		return FALSE;
	}

	/* RNR IE appeared after WiFi 6E */
	if (!WMODE_CAP_AX(repting_entry->entry_info.phy_mode)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[repting:%d, repted:%d] FALSE, repting is before 6E\n",
			repting_entry->bss_idx, repted_entry->bss_idx);
		return FALSE;
	}

#ifdef CONFIG_6G_SUPPORT
	if (repting_entry->entry_info.rnr_6g_rule == RNR_REPORTING_NONE) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[repting:%d, repted:%d] FALSE, config RNR NONE\n",
			repting_entry->bss_idx, repted_entry->bss_idx);
		return FALSE;
	}
#endif

	/* restricted to only 6G RNR appearing (in 2G/5G) */
	/* reporting AP shall be 2G/5G, reported AP shall be 6G */
	if (WMODE_CAP_6G(repting_entry->entry_info.phy_mode) ||
		!WMODE_CAP_6G(repted_entry->entry_info.phy_mode)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[repting:%d, repted:%d] FALSE, only 6G RNR in 2G/5G possible [6G(%d,%d)]\n",
			repting_entry->bss_idx, repted_entry->bss_idx,
			WMODE_CAP_6G(repting_entry->entry_info.phy_mode),
			WMODE_CAP_6G(repted_entry->entry_info.phy_mode));
		return FALSE;
	}

#ifdef DOT11_EHT_BE
	if (repting_entry->mlo_en && repted_entry->mlo_en) {
		u8 mld_bss_idx;
		struct bmgr_mlo_dev *mld = NULL;
		struct bmgr_entry *mld_entry = NULL;

		/* same MLD */
		if (repting_entry->entry_info.mld_grp == repted_entry->entry_info.mld_grp) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[repting:%d, repted:%d] FALSE, the same MLD group: (%d, %d)\n",
				repting_entry->bss_idx, repted_entry->bss_idx,
				repting_entry->entry_info.mld_grp,
				repted_entry->entry_info.mld_grp);
			return FALSE;
		}

		mld = repted_entry->mld_ptr;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"mld is null or not valid\n");
			return FALSE;
		}

		mld_bss_idx = mld->bss_idx_mld[repting_entry->link_id];
		mld_entry = BMGR_VALID_BSS_IDX(mld_bss_idx) ? GET_BSS_ENTRY_BY_IDX(mld_bss_idx) : NULL;
		/* same 11v BSSID set */
		if (BMGR_VALID_MLO_BSS_ENTRY(mld_entry) &&
			mld_entry->bss_idx_tx == repting_entry->bss_idx_tx) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[repting:%d, repted:%d] FALSE, the same 11v MBSSID set: (%d, %d)\n",
				repting_entry->bss_idx, repted_entry->bss_idx,
				repting_entry->bss_idx_tx,
				mld_entry->bss_idx_tx);
			return FALSE;
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[repting:%d, repted:%d] TRUE, not the same MLD and 11v_tx (%d,%d) (%d,%d)\n",
			repting_entry->bss_idx, repted_entry->bss_idx,
			repting_entry->entry_info.mld_grp,
			repted_entry->entry_info.mld_grp,
			repting_entry->bss_idx_tx,
			BMGR_VALID_BSS_ENTRY(mld_entry) ? mld_entry->bss_idx_tx : BMGR_INVALID_BSS_IDX);
		return TRUE;
	}
#endif
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"[repting:%d, repted:%d] TRUE, valid pair, [MLO(%d,%d)]\n",
		repting_entry->bss_idx, repted_entry->bss_idx,
		repting_entry->mlo_en, repted_entry->mlo_en);
	return TRUE;

}

static NDIS_STATUS bss_mngr_con_build_rnr_for_6g(struct bmgr_entry *repting_entry)
{
	u8 i;
	u8 *f_buf = NULL, *pos = NULL;
	u16 ie_len = 0;
	u8 mld_id = 0xff; /* no info now */
	struct repting_rnr_info *rept = &repting_entry->rnr_info;

	u8 repted_bss_idx;
	struct bmgr_entry *repted_entry = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"===> [bss_idx %d] w/ bcn: %d\n",
		repting_entry->bss_idx,
		IS_DEV_TYPE_W_BCN(repting_entry->entry_info.dev_type));
	hex_dump_with_cat_and_lvl("repting_bitmap", (u8 *)rept->bss_idx_bitmap,
		BMGR_U32_BSS_BITMAP_NUM * (sizeof(rept->bss_idx_bitmap[0]) / sizeof(u8)),
		DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO);

	rept->repting_cnt = 0;
	if (IS_DEV_TYPE_W_BCN(repting_entry->entry_info.dev_type)) {
		/* build repting_list */
		for (i = 0; i < BMGR_MAX_BSS_CNT; i++) {
			if (BMGR_U32_BITMAP_IS_ASSIGNED(rept->bss_idx_bitmap, i)) {
				rept->repting_bss_idx[rept->repting_cnt] = i;
				rept->repting_cnt++;
			}
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"\trepting_cnt: %d ", rept->repting_cnt);
		hex_dump_with_cat_and_lvl("repting_bss_idx", rept->repting_bss_idx, rept->repting_cnt,
			DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO);

		/* build repting_rnr */
		rept->repting_rnr.len = 0;
		f_buf = pos = rept->repting_rnr.buf;

		if (rept->repting_cnt) {
			for (i = 0; i < rept->repting_cnt; i++) {
				repted_bss_idx = rept->repting_bss_idx[i];
				repted_entry = GET_BSS_ENTRY_BY_IDX(repted_bss_idx);

				if (BMGR_VALID_BSS_ENTRY(repted_entry)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\ttx bss[%d] add rnr[%d] mld_id(%d), f_buf = %p\n",
						repting_entry->bss_idx, repted_bss_idx, mld_id, f_buf);

					pos = build_rnr_ie(repting_entry, repted_entry, f_buf, mld_id);
					ie_len = pos - f_buf;
					rept->repting_rnr.len += ie_len;
					f_buf = pos;
				} else {
					if (repted_entry && repting_entry->entry_info.single_rnr) {
						TBTT_BITMAP_CLEAR(repting_entry->rnr_nap->tbtt_bitmap, repted_entry->pwdev->func_idx);
						repting_entry->rnr_nap->tbtt_cnt--;
					}
				}
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_con_ie_update_rnr_for_6g(struct bmgr_entry *entry)
{
	struct bmgr_entry *other_entry = NULL;
	struct repting_rnr_info *rept = &entry->rnr_info;
	struct repting_rnr_info *other_rept = NULL;
	u8 i;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"===> [bss_idx %d]\n", entry->bss_idx);

	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		other_entry = GET_BSS_ENTRY_BY_IDX(i);
		/* skip invalid entry and self */
		if (!BMGR_VALID_BSS_ENTRY(other_entry) || (other_entry->bss_idx == entry->bss_idx))
			continue;

		/* self RNR reported by other */
		other_rept = &other_entry->rnr_info;
		if (is_6g_rnr_required(other_entry, entry))
			BMGR_U32_BITMAP_SET(other_rept->bss_idx_bitmap, entry->bss_idx);
		else
			BMGR_U32_BITMAP_CLEAR(other_rept->bss_idx_bitmap, entry->bss_idx);

		bss_mngr_con_build_rnr_for_6g(other_entry);

		/* other RNR reported by self */
		if (is_6g_rnr_required(entry, other_entry))
			BMGR_U32_BITMAP_SET(rept->bss_idx_bitmap, other_entry->bss_idx);
		else
			BMGR_U32_BITMAP_CLEAR(rept->bss_idx_bitmap, other_entry->bss_idx);
	}

	bss_mngr_con_build_rnr_for_6g(entry);

	return NDIS_STATUS_SUCCESS;
}

#ifdef DOT11_EHT_BE
/*
 * build rnr of bss within the same mld w/o self
 */
int bss_mngr_con_build_rnr_for_mld(struct bmgr_entry *entry)
{
	bool mld_en = false;
	u8 bss_idx = entry->bss_idx;
	u8 bss_idx_tx = entry->bss_idx_tx;
	u8 *f_buf = NULL, *pos = NULL;
	u16 ie_len = 0;
	u8 link;
	u8 dot11be_mld_id = 0;
	struct bmgr_mlo_dev *mld = NULL;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"bss_idx (%d) is not in mld (%d)\n",
			entry->bss_idx, entry->entry_info.mld_grp);
		return NDIS_STATUS_FAILURE;
	}

	if (entry->mlo_en)
		mld_en = true;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d], mld_en = %d\n", bss_idx, mld_en);

	if (mld_en) {
		struct bmgr_entry *tx_entry = NULL;

		/* get tx bss */
		if (BMGR_VALID_BSS_IDX(bss_idx_tx)) {
			tx_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_tx);

			/*
			 * If the reported AP is affiliated to the same MLD as a nontransmitted
			 * BSSID that is in the same multiple BSSID set as the reporting AP, the
			 * MLD ID subfield is set to the same value as in the BSSID Index field
			 * in the Multiple BSSID-Index element in the nontransmitted BSSID profile
			 * corresponding to the nontransmitted BSSID.
			 */
			if (IS_DEV_TYPE_11V_NT(entry->entry_info.dev_type))
				dot11be_mld_id = entry->dot11be_mld_id;

		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				": [%d] tx bss (%d) undefined\n",
				bss_idx, bss_idx_tx);
			return NDIS_STATUS_FAILURE;
		}

		/* ie buf under tx bss */
		f_buf = pos = tx_entry->rnr.buf + tx_entry->rnr.len;

		/* mld group member, exclude self */
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			u8 bss_idx_mld = mld->bss_idx_mld[link];
			struct bmgr_entry *mld_entry = NULL;

			if (BMGR_VALID_BSS_IDX(bss_idx_mld) && (bss_idx_mld != bss_idx)) {
				mld_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
				if (BMGR_VALID_MLO_BSS_ENTRY(mld_entry)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							"\ttx bss[%d] add rnr[%d] 11be_mld_id(%d), f_buf = %p\n",
							bss_idx_tx, bss_idx_mld, dot11be_mld_id, f_buf);

					pos = build_rnr_ie(tx_entry, mld_entry, f_buf, dot11be_mld_id);
					ie_len = pos - f_buf;
					tx_entry->rnr.len += ie_len;
					f_buf = pos;
				} else {
					if (mld_entry && tx_entry->entry_info.single_rnr) {
						TBTT_BITMAP_CLEAR(tx_entry->rnr_nap->tbtt_bitmap, mld_entry->pwdev->func_idx);
						tx_entry->rnr_nap->tbtt_cnt--;
					}
				}
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_con_ie_update_rnr_for_mld(struct bmgr_entry *entry)
{
	u8 dev_type = entry->entry_info.dev_type;
	u8 bss_idx = entry->bss_idx;
	u8 bss_idx_tx = entry->bss_idx_tx;
	struct bmgr_entry *tx_entry = NULL;
	int i;
	u8 *f_buf = NULL, *pos = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			": [%d], dev_type(%s)\n",
			bss_idx, dev_type_str[dev_type]);

	if (BMGR_VALID_BSS_IDX(bss_idx_tx))
		tx_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_tx);
	else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"[%d] TX-BSS undefined\n", bss_idx);
		return NDIS_STATUS_FAILURE;
	}

	f_buf = pos = tx_entry->rnr.buf;
	tx_entry->rnr.len = 0;

	switch (dev_type) {
	case DEV_TYPE_AP_LEGACY:
	case DEV_TYPE_AP_COH:
		/* rnr of self mld */
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tupdate rnr of self mld\n");

		bss_mngr_con_build_rnr_for_mld(entry);
		break;
	case DEV_TYPE_AP_11VT:
	case DEV_TYPE_AP_11VNT:
		/* traverse all mbss members and add members' mlo oob bss */
		for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
			struct bmgr_entry *mbss_entry = GET_BSS_ENTRY_BY_IDX(i);

			if (BMGR_VALID_BSS_ENTRY(mbss_entry)) {
				u8 entry_dev_type = mbss_entry->entry_info.dev_type;

				/* tx/ntx bss */
				if (IS_DEV_TYPE_11V(entry_dev_type) &&
					(mbss_entry->bss_idx_tx == bss_idx_tx)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							"\t[%d] update to mbss [%d](%s)\n",
							bss_idx, i, dev_type_str[entry_dev_type]);

					bss_mngr_con_build_rnr_for_mld(mbss_entry);
				}
			}
		}
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"[%d] unknown dev_type(%d)\n", bss_idx, dev_type);
		break;
	}

	return NDIS_STATUS_SUCCESS;
}

/*
 * update self mld ie
 */
int bss_mngr_con_ie_update_mld(struct bmgr_entry *entry)
{
	bool mld_en = false;
	u8 *f_buf = NULL, *pos = NULL;
	u16 ie_len = 0;

	if (!entry)
		return NDIS_STATUS_FAILURE;

	if (entry->mlo_en && BMGR_VALID_MLO_DEV(entry->mld_ptr))
		mld_en = true;

	entry->mld.len = entry->mld_cmm.len = 0;
	if (mld_en) {
		/* build mld ie */
		/* w/ link info */
		f_buf = pos = entry->mld.buf;
		pos = build_multi_link_ie(
				entry, f_buf, BMGR_MLD_COMMON_LINK_INFO, BMGR_MAX_MLD_STA_CNT);
		ie_len = pos - f_buf;
		entry->mld.len += ie_len;
		f_buf = pos;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tMLIE: Len = %d, Total = %d\n", ie_len, entry->mld.len);

		/* w/o link info, common info only */
		f_buf = pos = entry->mld_cmm.buf;
		pos = build_multi_link_ie(
				entry, f_buf, BMGR_MLD_BCN_COMMON, BMGR_MAX_MLD_STA_CNT);
		ie_len = pos - f_buf;
		entry->mld_cmm.len += ie_len;
		f_buf = pos;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tMLIE-C: Len = %d, Total = %d\n", ie_len, entry->mld_cmm.len);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
 * update tid-to-link mapping for group members
 */
int bss_mngr_con_ie_update_tid_to_link_map(struct bmgr_entry *entry)
{
	bool mld_en = false;
	struct bmgr_entry *mld_entry = NULL;
	u8 *f_buf = NULL, *pos = NULL;
	u16 ie_len = 0;

	if (entry->mlo_en && BMGR_VALID_MLO_DEV(entry->mld_ptr))
		mld_en = true;

	entry->tid2lnk.len = 0;
	if (mld_en) {
		f_buf = pos = entry->tid2lnk.buf;

		/* tid-to-link mapping ie */
		mld_entry = entry;
		if (mld_entry) {
			pos = build_tid_to_link_map_ie(mld_entry, f_buf);
			ie_len = pos - f_buf;
			mld_entry->tid2lnk.len += ie_len;
			f_buf = pos;

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\tLen = %d, Total = %d\n", ie_len, mld_entry->tid2lnk.len);
		}
	}

	return NDIS_STATUS_SUCCESS;
}
#endif

static int bss_mngr_con_sync_ie(struct bmgr_entry *entry)
{
	u8 link = 0;
	struct bmgr_entry *mld_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	u8 bss_idx = entry->bss_idx;
	u8 mld_grp = entry->entry_info.mld_grp;
	u8 str[4] = {0};

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			": [%d]\n", bss_idx);

	/* re-build ie for mlo group (always do update for parameter 'entry') */

	/* When interface down, entry has left MLD. Shall use mld_grp to access MLD */
	mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		u8 bss_idx_mld = BMGR_VALID_MLO_DEV(mld) ? mld->bss_idx_mld[link] : BMGR_INVALID_BSS_IDX;

		/* always do iob update */
		if (link == entry->link_id)
			bss_idx_mld = entry->bss_idx;

		if (BMGR_VALID_BSS_IDX(bss_idx_mld)) {
			mld_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
			if (mld_entry) {
				strlcpy(str, (bss_idx_mld == bss_idx) ? "iob" : "oob", sizeof(str));
				str[sizeof(str) - 1] = '\0';
#ifdef DOT11V_MBSSID_SUPPORT
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\t1. L%d[%d] sync %s mbss ie\n", link, bss_idx_mld, str);
				bss_mngr_con_ie_update_mbss(mld_entry);
#endif
#ifdef DOT11_EHT_BE
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\t2. L%d[%d] sync %s rnr ie\n", link, bss_idx_mld, str);
				bss_mngr_con_ie_update_rnr_for_mld(mld_entry);
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\t3. L%d[%d] sync %s ml ie\n", link, bss_idx_mld, str);
				bss_mngr_con_ie_update_mld(mld_entry);

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\t4. L%d[%d] sync %s t2l ie\n", link, bss_idx_mld, str);
				bss_mngr_con_ie_update_tid_to_link_map(mld_entry);
#endif
			}
		}
	}

	/* update out-of-band (or 6G) RNR */
	bss_mngr_con_ie_update_rnr_for_6g(entry);

	return NDIS_STATUS_SUCCESS;
}

static int bss_mngr_con_ie_update(IN PNET_DEV pNetDev)
{
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	if (entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"if=%s, link_id=%d\n", RtmpOsGetNetDevName(pNetDev), entry->link_id);
		return bss_mngr_con_sync_ie(entry);
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"Error: entry not found\n");
	return NDIS_STATUS_FAILURE;
}

int bss_mngr_con_ie_buf_alloc(struct bmgr_entry *entry)
{
	u8 dev_type = entry->entry_info.dev_type;
	u8 *mbss_bcn = NULL;
	u8 *mbss_probe = NULL;
	u8 *mbss_probe_cmm = NULL;
	u8 *rnr = NULL;
	u8 *oob_rnr = NULL;
	u8 *mld = NULL;
	u8 *mld_cmm = NULL;
	u8 *tid2lnk = NULL;
	u8 *tbtt = NULL;
	u8 tbtt_idx, link_id;

	if (IS_DEV_TYPE_W_BCN(dev_type)) {
		/* rnr ie: legacy/11vT/11vCoH */
		if (!entry->rnr.buf) {
			os_alloc_mem(NULL, (u8 **)&rnr, BSS_MNGR_MAX_INFO_LEN);
			if (rnr) {
				NdisZeroMemory(rnr, BSS_MNGR_MAX_INFO_LEN);
				entry->rnr.buf = rnr;
			} else {
				goto err;
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"allocated (RNR-M:%p)\n", entry->rnr.buf);
		}

		/* rnr ie: for WiFi6 out-of-band reporting */
		if (!entry->rnr_info.repting_rnr.buf) {
			os_alloc_mem(NULL, (u8 **)&oob_rnr, BSS_MNGR_MAX_INFO_LEN);
			if (oob_rnr) {
				NdisZeroMemory(oob_rnr, BSS_MNGR_MAX_INFO_LEN);
				entry->rnr_info.repting_rnr.buf = oob_rnr;
			} else {
				goto err;
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"allocated (RNR-O:%p)\n", entry->rnr_info.repting_rnr.buf);
		}

		if (entry->entry_info.single_rnr) {
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				for (tbtt_idx = 0; tbtt_idx < 16; tbtt_idx++) {
					tbtt = entry->rnr_nap[link_id].tbtt[tbtt_idx].buf;
					if (!tbtt) {
						os_alloc_mem(NULL, (u8 **)&tbtt, 32);
						if (tbtt) {
							NdisZeroMemory(tbtt, 32);
							entry->rnr_nap[link_id].tbtt[tbtt_idx].buf = tbtt;
						} else {
							goto err;
						}
					}
				}
			}
		}

		/* mbssid ie: 11vT */
		if (IS_DEV_TYPE_11V_T(dev_type)) {
			if (!entry->mbss.tx_bcn.buf) {
				os_alloc_mem(NULL, (u8 **)&mbss_bcn, BSS_MNGR_MAX_MBSSID_IE_LEN);
				if (mbss_bcn) {
					NdisZeroMemory(mbss_bcn, BSS_MNGR_MAX_MBSSID_IE_LEN);
					entry->mbss.tx_bcn.buf = mbss_bcn;
				} else {
					goto err;
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
						"allocated (MBSS-B:%p)\n", entry->mbss.tx_bcn.buf);
			}

			if (!entry->mbss.tx_probe.buf) {
				os_alloc_mem(NULL, (u8 **)&mbss_probe, BSS_MNGR_MAX_MBSSID_IE_LEN);
				if (mbss_probe) {
					NdisZeroMemory(mbss_probe, BSS_MNGR_MAX_MBSSID_IE_LEN);
					entry->mbss.tx_probe.buf = mbss_probe;
				} else {
					goto err;
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
						"allocated (MBSS-P:%p)\n", entry->mbss.tx_probe.buf);
			}

			if (!entry->mbss.tx_probe_cmm.buf) {
				os_alloc_mem(NULL, (u8 **)&mbss_probe_cmm, BSS_MNGR_MAX_MBSSID_IE_LEN);
				if (mbss_probe_cmm) {
					NdisZeroMemory(mbss_probe_cmm, BSS_MNGR_MAX_MBSSID_IE_LEN);
					entry->mbss.tx_probe_cmm.buf = mbss_probe_cmm;
				} else {
					goto err;
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
						"allocated (MBSS-P-C:%p)\n", entry->mbss.tx_probe_cmm.buf);
			}
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"allocated (MBSS-B:%p, MBSS-P: %p, MBSS-P-C: %p)\n",
				entry->mbss.tx_bcn.buf,
				entry->mbss.tx_probe.buf,
				entry->mbss.tx_probe_cmm.buf);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"allocated (RNR-M:%p, RNR-O:%p)\n",
				entry->rnr.buf,
				entry->rnr_info.repting_rnr.buf);
	}

	if (entry->mlo_en) {
		u8 link;
		struct ie *per_sta_pf;

		/* mld ie: full info, legacy/11vT/11vCoH */
		if (!entry->mld.buf) {
			os_alloc_mem(NULL, (u8 **)&mld, BSS_MNGR_MAX_INFO_LEN);
			if (mld) {
				NdisZeroMemory(mld, BSS_MNGR_MAX_INFO_LEN);
				entry->mld.buf = mld;
			} else {
				goto err;
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"allocated (MLD:%p)\n", entry->rnr.buf);
		}

		/* mld ie: common info field only */
		if (!entry->mld_cmm.buf) {
			os_alloc_mem(NULL, (u8 **)&mld_cmm, BSS_MNGR_MAX_INFO_LEN);
			if (mld_cmm) {
				NdisZeroMemory(mld_cmm, BSS_MNGR_MAX_INFO_LEN);
				entry->mld_cmm.buf = mld_cmm;
			} else {
				goto err;
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"allocated (MLD-C:%p)\n", entry->mld_cmm.buf);
		}

		/* tid-to-link ie: legacy/11vT/11vCoH */
		if (!entry->tid2lnk.buf) {
			os_alloc_mem(NULL, (u8 **)&tid2lnk, BSS_MNGR_MAX_INFO_LEN);
			if (tid2lnk) {
				NdisZeroMemory(tid2lnk, BSS_MNGR_MAX_INFO_LEN);
				entry->tid2lnk.buf = tid2lnk;
			} else {
				goto err;
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"allocated (T2L:%p)\n", entry->tid2lnk.buf);
		}
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"allocated (MLD:%p, MLD-C:%p, T2L:%p)\n",
				entry->mld.buf,
				entry->mld_cmm.buf,
				entry->tid2lnk.buf);

		if (at2lm_ie_buf_alloc(entry))
			goto err;

		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			per_sta_pf = &entry->ml_probe_rsp_per_sta_pf[link];
			if (!per_sta_pf->buf) {
				os_alloc_mem(NULL, (UCHAR **)&per_sta_pf->buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
				if (per_sta_pf->buf)
					NdisZeroMemory(per_sta_pf->buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
				else
					goto err;
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
err:
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"alloc failed (MBSS-B:%p, MBSS-P: %p, MBSS-P-C: %p)\n",
			entry->mbss.tx_bcn.buf,
			entry->mbss.tx_probe.buf,
			entry->mbss.tx_probe_cmm.buf);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"alloc failed (RNR:%p, MLD:%p, MLD-C:%p, T2L:%p)\n",
			entry->rnr.buf,
			entry->mld.buf,
			entry->mld_cmm.buf,
			entry->tid2lnk.buf);

	bss_mngr_con_ie_buf_free(entry);

	return NDIS_STATUS_RESOURCES;
}

int bss_mngr_con_ie_buf_free(struct bmgr_entry *entry)
{
	u8 link, tbtt_idx;
	u8 *mbss_bcn = entry->mbss.tx_bcn.buf;
	u8 *mbss_probe = entry->mbss.tx_probe.buf;
	u8 *mbss_probe_cmm = entry->mbss.tx_probe_cmm.buf;
	u8 *rnr = entry->rnr.buf;
	u8 *oob_rnr = entry->rnr_info.repting_rnr.buf;
	u8 *mld = entry->mld.buf;
	u8 *mld_cmm = entry->mld_cmm.buf;
	u8 *tid2lnk = entry->tid2lnk.buf;
	u8 *probe_rsp_sta_pf;
	u8 *probe_rsp_per_sta_pf;
	u8 *tbtt;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"freed (RNR:%p, MBSS-B:%p, MBSS-P: %p, MBSS-P-C: %p, MLD:%p, MLD-C:%p, T2L:%p)\n",
			rnr, mbss_bcn, mbss_probe, mbss_probe_cmm, mld, mld_cmm, tid2lnk);

	if (mbss_bcn)
		os_free_mem(mbss_bcn);

	if (mbss_probe)
		os_free_mem(mbss_probe);

	if (mbss_probe_cmm)
		os_free_mem(mbss_probe_cmm);

	if (rnr)
		os_free_mem(rnr);

	if (oob_rnr)
		os_free_mem(oob_rnr);

	if (mld)
		os_free_mem(mld);

	if (mld_cmm)
		os_free_mem(mld_cmm);

	if (tid2lnk)
		os_free_mem(tid2lnk);

	at2lm_ie_buf_free(entry);

	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		entry->ml_probe_rsp_per_sta_pf[link].len = 0;
		probe_rsp_per_sta_pf = entry->ml_probe_rsp_per_sta_pf[link].buf;
		if (probe_rsp_per_sta_pf) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"free (ProbePerStaPf[%d]:%p)\n",
				link, probe_rsp_per_sta_pf);
			os_free_mem(probe_rsp_per_sta_pf);
		}
		entry->ml_probe_rsp_sta_pf[link].len = 0;
		probe_rsp_sta_pf = entry->ml_probe_rsp_sta_pf[link].buf;
		if (probe_rsp_sta_pf) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"free (ProbeStaPf[%d]:%p)\n",
				link, probe_rsp_sta_pf);
			os_free_mem(probe_rsp_sta_pf);
		}
	}

	if (entry->entry_info.single_rnr) {
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			for (tbtt_idx = 0; tbtt_idx < 16; tbtt_idx++) {
				tbtt = entry->rnr_nap[link].tbtt[tbtt_idx].buf;
				if (tbtt)
					os_free_mem(tbtt);
			}
		}
	}

#ifdef DOT11_EHT_BE
	free_bcn_appl_elem_per_sta_pf(entry);
#endif

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_con_ie_buf_clean(struct bmgr_entry *entry)
{
	u8 *mbss_bcn = entry->mbss.tx_bcn.buf;
	u8 *mbss_probe = entry->mbss.tx_probe.buf;
	u8 *mbss_probe_cmm = entry->mbss.tx_probe_cmm.buf;
	u8 *rnr = entry->rnr.buf;
	u8 *oob_rnr = entry->rnr_info.repting_rnr.buf;
	u8 *mld = entry->mld.buf;
	u8 *mld_cmm = entry->mld_cmm.buf;
	u8 *tid2lnk = entry->tid2lnk.buf;
	u8 link, tbtt_idx;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"clear (RNR-O:%p, MBSS-B:%p, MBSS-P: %p, MBSS-P-C: %p)\n",
			oob_rnr, mbss_bcn, mbss_probe, mbss_probe_cmm);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"clear (RNR-M:%p, MLD:%p, MLD-C:%p, T2L:%p)\n",
			rnr, mld, mld_cmm, tid2lnk);

	entry->mbss.tx_bcn.len = 0;
	if (mbss_bcn)
		NdisZeroMemory(mbss_bcn, BSS_MNGR_MAX_MBSSID_IE_LEN);

	entry->mbss.tx_probe.len = 0;
	if (mbss_probe)
		NdisZeroMemory(mbss_probe, BSS_MNGR_MAX_MBSSID_IE_LEN);

	entry->mbss.tx_probe_cmm.len = 0;
	if (mbss_probe_cmm)
		NdisZeroMemory(mbss_probe_cmm, BSS_MNGR_MAX_MBSSID_IE_LEN);

	entry->rnr.len = 0;
	if (rnr)
		NdisZeroMemory(rnr, BSS_MNGR_MAX_INFO_LEN);

	entry->rnr_info.repting_rnr.len = 0;
	if (oob_rnr)
		NdisZeroMemory(oob_rnr, BSS_MNGR_MAX_INFO_LEN);

	entry->mld.len = 0;
	if (mld)
		NdisZeroMemory(mld, BSS_MNGR_MAX_INFO_LEN);

	entry->mld_cmm.len = 0;
	if (mld_cmm)
		NdisZeroMemory(mld_cmm, BSS_MNGR_MAX_INFO_LEN);

	entry->tid2lnk.len = 0;
	if (tid2lnk)
		NdisZeroMemory(tid2lnk, BSS_MNGR_MAX_INFO_LEN);

	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		entry->ml_probe_rsp_per_sta_pf[link].len = 0;
		if (entry->ml_probe_rsp_per_sta_pf[link].buf)
			NdisZeroMemory(entry->ml_probe_rsp_per_sta_pf[link].buf,
				BSS_MNGR_MAX_PER_STA_PROFILE_LEN);

		entry->ml_probe_rsp_sta_pf[link].len = 0;
		if (entry->ml_probe_rsp_sta_pf[link].buf) {
			NdisZeroMemory(entry->ml_probe_rsp_sta_pf[link].buf,
				BSS_MNGR_MAX_STA_PROFILE_LEN);
		}
	}

	if (entry->entry_info.single_rnr) {
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			for (tbtt_idx = 0; tbtt_idx < 16; tbtt_idx++) {
				TBTT_BITMAP_CLEAR(entry->rnr_nap[link].tbtt_bitmap, tbtt_idx);
				entry->rnr_nap[link].tbtt[tbtt_idx].len = 0;

				if (entry->rnr_nap[link].tbtt[tbtt_idx].buf)
					NdisZeroMemory(entry->rnr_nap[link].tbtt[tbtt_idx].buf, 32);
			}
			entry->rnr_nap[link].tbtt_cnt = 0;
		}
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef DOT11_EHT_BE
/* tid shall always be mapped to at least one setup link */
static int tid_map_sanity(
	IN struct bmgr_entry *entry
)
{
	u8 i;
	struct bmgr_entry *mld_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	u8 mld_grp = entry->entry_info.mld_grp;
	u8 mld_bss_idx;
	u8 mapped_tid = 0;
	u8 unmapped_tid = 0;

	/* When interface down, entry has left MLD. Shall use mld_grp to access MLD */
	mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;

	/* mapped tid for mld group */
	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
		mld_bss_idx = BMGR_VALID_MLO_DEV(mld) ? mld->bss_idx_mld[i] : BMGR_INVALID_BSS_IDX;
		if (BMGR_VALID_BSS_IDX(mld_bss_idx)) {
			mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(mld_entry))
				mapped_tid |= mld_entry->entry_info.tid_map;
		}
	}
	unmapped_tid = ~(mapped_tid);
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
			"[%d] unmapped_tid(0x%x)\n", entry->bss_idx, unmapped_tid);

	/* apply non-mapped TID to all links */
	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
		mld_bss_idx = BMGR_VALID_MLO_DEV(mld) ? mld->bss_idx_mld[i] : BMGR_INVALID_BSS_IDX;
		if (BMGR_VALID_BSS_IDX(mld_bss_idx)) {
			mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(mld_entry)) {
				if (unmapped_tid) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
							"\t[%d] tid_remap (0x%x)->(0x%x)\n", mld_entry->bss_idx,
							mld_entry->entry_info.tid_map,
							(mld_entry->entry_info.tid_map | unmapped_tid));
				}
				mld_entry->tid_map = (mld_entry->entry_info.tid_map | unmapped_tid);
			}
		}
	}

	return 0;
}

/**
 * @brief Sanity the mld attribute
 *
 * This function do sanity to the MLD Attribute based on the definition
 * of 802.11 spec.
 *
 * @param mld_attr MLD Attributes to be did sanity.
 * @return Boolean: sanity success or failed.
 */
static bool mld_attr_sanity(
	IN struct bmgr_mld_attr *mld_attr
)
{
	if (!mld_attr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"False: attr is null\n");
		return FALSE;
	}

	if (mld_attr->med_sync.ofdm_ed_thr > MLD_MAX_MSD_OFDM_ED_THR) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
			"False: msd OFDM ED Threshold > %d\n", MLD_MAX_MSD_OFDM_ED_THR);
		return FALSE;
	}

	if (mld_attr->eml_caps.emlsr_supp && mld_attr->eml_caps.emlmr_supp) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
			"False: shall only one of emlsr/emlmr enabled at a time\n");
		return FALSE;
	}

	if (mld_attr->eml_caps.trans_to > MLD_MAX_EML_TRANS_TO) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
			"False: eml transition timeout > %d\n", MLD_MAX_EML_TRANS_TO);
		return FALSE;
	}

	return TRUE;
}

static bool bss_mngr_is_mld_addr_used(
	IN u8 *addr
)
{
	u8 i;

	if (!addr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tError: address is NULL\n");
		return TRUE;
	}

	for (i = 0; i < BMGR_MAX_MLD_GRP_CNT; i++) {
		struct bmgr_mlo_dev *mld = GET_MLD_BY_GRP_IDX(i);

		if (BMGR_VALID_MLO_DEV(mld) && MAC_ADDR_EQUAL(mld->mld_addr, addr)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"T: addr(%pM) is the same as mld %d\n",
				addr, i);
			return TRUE;
		}
	}

	return FALSE;
}

static bool bss_mngr_is_link_addr_used(
	IN u8 *addr
)
{
	u8 i;

	if (!addr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tError: address is NULL\n");
		return TRUE;
	}

	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		struct bmgr_entry *entry = GET_BSS_ENTRY_BY_IDX(i);
		struct bmgr_reg_info *reg_info;

		if (BMGR_VALID_BSS_ENTRY(entry)) {
			reg_info = &entry->entry_info;
			if (MAC_ADDR_EQUAL(reg_info->bssid, addr)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"T: addr(%pM) is the same as bss%d(%s) BSSID\n",
					addr, entry->bss_idx, RtmpOsGetNetDevName(entry->pNetDev));
				return TRUE;
			}
		}
	}

	return FALSE;
}

/**
 * @brief Fill MLD Attr. to an MLD
 *
 * This function fill the MLD attributes to an MLD.
 * It could be based on the memory move or direct assignment.
 *
 * @param mld MLD.
 * @param mld_attr MLD Attributes.
 */
static int bss_mngr_con_fill_attr_to_mld(
	IN struct bmgr_mlo_dev *mld,
	IN struct bmgr_mld_attr *mld_attr
)
{
	if (!mld || !mld_attr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Invalid input mld(%p), mld_attr(%p)\n",
			mld, mld_attr);
		return NDIS_STATUS_INVALID_DATA;
	}

	NdisMoveMemory(&mld->attr, mld_attr, sizeof(struct bmgr_mld_attr));

	return NDIS_STATUS_SUCCESS;
}

/**
 * @brief Fill update info for a BSS
 *
 * This function fill the Update Info for a BSS.
 * The Update Info is used to deliver the BSS update information from an MLD
 * which BSS is affiliated with from BSS/MLD Manager to Logan.
 *
 * @param mld MLD, source of the update information.
 * @param bmgr_update_info struct that maintains the update information.
 */
static int bss_mngr_con_fill_update_info(
	IN struct bmgr_mlo_dev *mld,
	OUT struct bmgr_update_info *update_info
)
{
	if (!mld || !update_info) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Invalid input mld(%p), update_info(%p)\n",
			mld, update_info);
		return NDIS_STATUS_INVALID_DATA;
	}

	NdisMoveMemory(&update_info->mld_attr, &mld->attr, sizeof(struct bmgr_mld_attr));

	return NDIS_STATUS_SUCCESS;
}

/**
 * @brief Update IEs if any change of MLD
 *
 * This function update the IEs of all links affiliated with MLD.
 *
 * @param mld_grp MLD index
 */
static int bss_mngr_con_mld_sync_ie(u8 mld_grp)
{
	u8 link_id;
	struct bmgr_entry *entry;
	struct bmgr_mlo_dev *mld;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			": [%d]\n", mld_grp);

	mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Fail: mld_grp %d invalid\n", mld_grp);
		return NDIS_STATUS_INVALID_DATA;
	}

	for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
		u8 bss_idx = mld->bss_idx_mld[link_id];

		if (!BMGR_VALID_BSS_IDX(bss_idx))
			continue;

		entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
		if (!BMGR_VALID_MLO_BSS_ENTRY(entry))
			continue;

		/* pick a valid entry to do IE update */
		bss_mngr_con_sync_ie(entry);

		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}

static int bss_mngr_con_mld_group_create(
	IN PNET_DEV pNetDev,
	INOUT u8 *mld_grp,
	IN u8 *mld_addr,
	IN struct bmgr_mld_attr *mld_attr,
	IN u8 mld_type
)
{
	u8 new_grp, old_grp = *mld_grp;
	bool need_reassign = FALSE;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"===> mld_grp(%d), mld_addr(%pM)\n", old_grp, mld_addr);

	if (!mld_addr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tError: MLD address is NULL\n");
		return NDIS_STATUS_FAILURE;
	}

	if (!mld_attr_sanity(mld_attr)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tFail: MLD attr sanity fail\n");
		return NDIS_STATUS_INVALID_DATA;
	}

	/* MLD Group existed */
	if (BMGR_VALID_MLD_GRP_IDX(old_grp)) {
		mld = GET_MLD_BY_GRP_IDX(old_grp);
		if (BMGR_VALID_MLO_DEV(mld)) {
			switch (mld->mld_type) {
			case BMGR_MLD_TYPE_MULTI:
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"\tmulti-link mld_grp(%d)(%pM) already exists\n",
					old_grp, mld->mld_addr);
				return NDIS_STATUS_SUCCESS;

			case BMGR_MLD_TYPE_SINGLE:
				need_reassign = true;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"\tNotice: existed single-link group %d, reassign grp\n\n",
					old_grp);
				break;

			default:
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"\tunknown MLD type %d\n", mld->mld_type);
				return NDIS_STATUS_INVALID_DATA;
			}
		}
	}

	/* spec sanity: MLD MAC address shall be unique among MLDs */
	if (bss_mngr_is_mld_addr_used(mld_addr))
		return NDIS_STATUS_FAILURE;

	/* design sanity: ML MLD MAC address is unique to any BSSIDs */
	if ((mld_type == BMGR_MLD_TYPE_MULTI) && bss_mngr_is_link_addr_used(mld_addr))
		return NDIS_STATUS_FAILURE;

	mutex_lock(&bss_mngr.op_mutex);

	/* re-assign single-link mld group idx */
	if ((old_grp == 0) || need_reassign) {
		new_grp = mld_grp_aquire(BMGR_MAX_ML_MLD_CNT + 1);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"\tOld id %d -> New id %d\n",
			old_grp, new_grp);
	} else
		new_grp = old_grp;

	if (BMGR_VALID_MLD_GRP_IDX(new_grp)) {
		mld = GET_MLD_BY_GRP_IDX(new_grp);
		if (BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tFailed, mld_grp(%d) already exists\n", new_grp);
			mutex_unlock(&bss_mngr.op_mutex);
			return NDIS_STATUS_FAILURE;
		}

		/* if mld is NULL, allocate new mld */
		if (!mld) {
			os_alloc_mem(NULL, (u8 **)&mld, sizeof(struct bmgr_mlo_dev));

			if (mld) {
				bss_mngr.mld[new_grp] = mld;
				mld_grp_set(new_grp);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"\tmld alloc failed!!\n");
				mld_grp_clear(new_grp);
				mutex_unlock(&bss_mngr.op_mutex);
				return NDIS_STATUS_FAILURE;
			}
		}

		/* init mld */
		if (mld) {
			mld_grp_set(new_grp);
			NdisZeroMemory(mld, sizeof(struct bmgr_mlo_dev));

			/* acquire hw resources */
			if (event_ops->bmgr_event_add_mld(pNetDev, new_grp,
					mld_addr, mld_type) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"\tError: acquire hw rsc for mld failed (type:%d, grp:%d, addr:%pM)\n",
					mld_type, new_grp, mld_addr);
				mld_grp_clear(new_grp);
				mutex_unlock(&bss_mngr.op_mutex);
				return NDIS_STATUS_FAILURE;
			}

			mld->mld_grp = new_grp;
			mld->mld_type = mld_type;
			COPY_MAC_ADDR(mld->mld_addr, mld_addr);
			bmgr_mld_reset(mld);
			atomic_set(&mld->bmc_sn, 0);
			bss_mngr_con_fill_attr_to_mld(mld, mld_attr);
			mld->valid = TRUE;
		}

		*mld_grp = mld->mld_grp;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"\tCreate mld_grp(%d->%d) w/ type(%d) addr(%pM) success\n",
			old_grp, new_grp, mld->mld_type, mld_addr);

	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\trequested mld_grp(%d) is invalid (%d)\n",
			new_grp, BMGR_MAX_MLD_GRP_CNT);
		mutex_unlock(&bss_mngr.op_mutex);
		return NDIS_STATUS_FAILURE;
	}

	mutex_unlock(&bss_mngr.op_mutex);

	return NDIS_STATUS_SUCCESS;
}

static int bss_mngr_con_mld_group_destroy(
	IN PNET_DEV pNetDev,
	IN u8 mld_grp
)
{
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"===> mld_grp(%d)\n", mld_grp);

	if (BMGR_VALID_MLD_GRP_IDX(mld_grp))
		mld = GET_MLD_BY_GRP_IDX(mld_grp);

	if (BMGR_VALID_MLO_DEV(mld)) {
		mutex_lock(&bss_mngr.op_mutex);

		if (mld->mld_link_cnt != 0) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tmld_grp(%d) is not empty (link_cnt=%d), can not destroy.\n",
				mld_grp, mld->mld_link_cnt);
			mutex_unlock(&bss_mngr.op_mutex);
			return NDIS_STATUS_FAILURE;
		}

		mld->valid = FALSE;

		/* release hw resources */
		if (event_ops->bmgr_event_del_mld(pNetDev, mld_grp) != NDIS_STATUS_SUCCESS) {
			/* error if there is link in MLD */
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tError: release hw rsc of mld failed (grp:%d)\n",
				mld_grp);
		}

		/* free buffer before zero memory to avoid memory leakage */
		mld_free_reconfig_ml_ie(mld);

		NdisZeroMemory(mld, sizeof(struct bmgr_mlo_dev));
		bmgr_mld_reset(mld);
		mld_grp_clear(mld_grp);
		/* set mld_grp index back after reset to zero */
		mld->mld_grp = mld_grp;
		mutex_unlock(&bss_mngr.op_mutex);

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"\tDestroy mld_grp(%d) success\n", mld_grp);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tmld_grp(%d) is not valid\n", mld_grp);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;

}

/**
 * @brief Merge two MLD attributes based on the presence bitmap
 *
 * This function merges two MLD attributes, source and target.
 * The target attr. might be the original attr. of MLD and
 * combines the source attr. based on the presence bitmap
 * into it.
 *
 * @param target_attr Target MLD attr. Save the output attr.
 * @param source_attr Source MLD attr. Attributes to be written into target attr.
 * @param attr_pres_bmap Indicate which attributes to be overwritten in target attr.
 */
static int bss_mngr_con_mld_attr_merge(
	INOUT struct bmgr_mld_attr *target_attr,
	IN struct bmgr_mld_attr *source_attr,
	IN u32 attr_pres_bmap
)
{
	if (!source_attr || !target_attr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Invalid input source_attr(%p), target_attr(%p)\n",
			source_attr, target_attr);
		return NDIS_STATUS_INVALID_DATA;
	}

	if (attr_pres_bmap & MLD_OP_PARAM_PRES_EML_MODE) {
		target_attr->eml_caps.emlsr_supp = source_attr->eml_caps.emlsr_supp;
		target_attr->eml_caps.emlmr_supp = source_attr->eml_caps.emlmr_supp;
		target_attr->eml_caps.en = (target_attr->eml_caps.emlsr_supp ||
			target_attr->eml_caps.emlmr_supp) ? TRUE : FALSE;
	}

	if (attr_pres_bmap & MLD_OP_PARAM_PRES_EML_TRANS_TO)
		target_attr->eml_caps.trans_to = source_attr->eml_caps.trans_to;

	if (attr_pres_bmap & MLD_OP_PARAM_PRES_EML_OMN)
		target_attr->eml_caps.eml_omn_en = source_attr->eml_caps.eml_omn_en;

	if (attr_pres_bmap & MLD_OP_PARAM_PRES_T2LM_NEGO)
		target_attr->mld_caps.t2l_nego_supp = source_attr->mld_caps.t2l_nego_supp;

	return NDIS_STATUS_SUCCESS;
}

/**
 * @brief Core API to update the attribute of MLD
 *
 * This function updates the attributes of MLD maintained in BSS/MLD Manager.
 *
 * @param mld_grp MLD index in BSS/MLD Manager
 * @param attr_pres_bmap Indicate which attributes to be updated.
 * @param mld_attr the to be updated attributes of MLD.
 * @param cfg_disconn Indicate whether disconnect all MLD Peer before update attributes.
 */
static int bss_mngr_con_mld_group_attr_set(
	IN u8 mld_grp,
	IN u32 attr_pres_bmap,
	IN struct bmgr_mld_attr *mld_attr,
	IN u8 cfg_disconn
)
{
	u8 link_id, bss_idx;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_attr target_attr = {0};
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	if (!mld_attr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Fail: mld_attr is null\n");
		return NDIS_STATUS_INVALID_DATA;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"===> mld_grp(%d), pres_bmap(%d)\n", mld_grp, attr_pres_bmap);

	if (!attr_pres_bmap) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Fail: pres_bmap (%d) should be non-zero\n", attr_pres_bmap);
		return NDIS_STATUS_INVALID_DATA;
	}

	if (BMGR_VALID_MLD_GRP_IDX(mld_grp))
		mld = GET_MLD_BY_GRP_IDX(mld_grp);

	if (BMGR_VALID_MLO_DEV(mld)) {
		struct bmgr_entry *entry;
		struct bmgr_update_info update_info = {0};

		if (!mld_attr_sanity(mld_attr))
			return NDIS_STATUS_INVALID_DATA;

		NdisMoveMemory(&target_attr, &mld->attr, sizeof(struct bmgr_mld_attr));
		if (bss_mngr_con_mld_attr_merge(&target_attr, mld_attr,
				attr_pres_bmap) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Fail: attr sync\n");
			return NDIS_STATUS_FAILURE;
		}

		/* TBD: pre-op bcn/assoc/disconn if any link */

		/* disconnect MLO-capable peer */
		if (cfg_disconn)
			bss_mngr_con_mld_disconn_all_mld_sta(mld->mld_grp);

		mutex_lock(&bss_mngr.op_mutex);

		/* update attr in mld */
		bss_mngr_con_fill_attr_to_mld(mld, &target_attr);

		if (mld->mld_link_cnt > 0) {
			/* update IEs */
			bss_mngr_con_mld_sync_ie(mld->mld_grp);

			/* update to logan bss (per link) */
			bss_mngr_con_fill_update_info(mld, &update_info);

			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				bss_idx = mld->bss_idx_mld[link_id];
				if (!BMGR_VALID_BSS_IDX(bss_idx))
					continue;

				entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
				if (!BMGR_VALID_MLO_BSS_ENTRY(entry))
					continue;

				if (event_ops->bmgr_event_bss_attr_update(entry->pNetDev,
					&update_info, TRUE) != NDIS_STATUS_SUCCESS) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
						"Fail: update bss %s failed\n", RtmpOsGetNetDevName(entry->pNetDev));
				}
			}
		}

		mutex_unlock(&bss_mngr.op_mutex);

		/* TBD: post-op bcn/assoc if any link // link */

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"Set attr of MLD(%d) success\n", mld_grp);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"mld_grp(%d) is not valid\n", mld_grp);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;

}

static void bss_mngr_con_mld_get_tsf_offset(
	IN struct bmgr_mlo_dev *mld
)
{
	u8 repted_link, repting_link;
	u8 repted_bss_idx, repting_bss_idx;
	struct bmgr_entry *repted_entry, *repting_entry;
	struct wifi_dev *wdev = NULL;
	UINT32 tsf_diff[4];
	INT64 tsf_0, tsf_1;
	INT64 tsf_offset;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: mld is NULL!\n");
		return;
	}

	/* get the Per-STA Profile tsf offset */
	/* repted_link: the link that is reported in Per-STA Profile in ML IE */
	for (repted_link = 0; repted_link < BSS_MNGR_MAX_BAND_NUM; repted_link++) {
		repted_bss_idx = mld->bss_idx_mld[repted_link];
		if (!BMGR_VALID_BSS_IDX(repted_bss_idx))
			continue;

		repted_entry = GET_BSS_ENTRY_BY_IDX(repted_bss_idx);
		if (!BMGR_VALID_MLO_BSS_ENTRY(repted_entry))
			continue;

		for (repting_link = 0; repting_link < BSS_MNGR_MAX_BAND_NUM; repting_link++) {
			if (repted_link == repting_link)
				continue;

			repting_bss_idx = mld->bss_idx_mld[repting_link];
			if (!BMGR_VALID_BSS_IDX(repting_bss_idx))
				continue;

			repting_entry = GET_BSS_ENTRY_BY_IDX(repting_bss_idx);
			if (!BMGR_VALID_MLO_BSS_ENTRY(repting_entry))
				continue;

			os_zero_mem(tsf_diff, sizeof(tsf_diff));

			wdev = RTMP_OS_NETDEV_GET_WDEV(repted_entry->pNetDev);
			AsicGetTsfDiffTime(wdev->sys_handle,
				repted_entry->entry_info.hw_bss_index,
				repting_entry->entry_info.hw_bss_index,
				&tsf_diff[0],
				&tsf_diff[1],
				&tsf_diff[2],
				&tsf_diff[3]);

			tsf_0 = (INT64)tsf_diff[0] + ((INT64)tsf_diff[1] << 32);
			tsf_1 = (INT64)tsf_diff[2] + ((INT64)tsf_diff[3] << 32);
			tsf_offset = (tsf_0 - tsf_1) / 2;

			repted_entry->tsf_offset[repting_link] = tsf_offset;

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"\tbss[%02d,%d]=%lld, bss[%02d,%d]=%lld, tsf_offset(%lld)(0x%llx), unit:2us\n",
				repted_entry->bss_idx, repted_entry->entry_info.hw_bss_index, tsf_0,
				repting_entry->bss_idx, repting_entry->entry_info.hw_bss_index, tsf_1,
				tsf_offset, tsf_offset);
		}
	}
}

static int bss_mngr_con_mld_group_add_link(
	IN PNET_DEV pNetDev,
	IN u8 mld_grp
)
{
	u8 bss_idx;
	u8 link_id;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_update_info update_info;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"===> wdev(%s) requests to join mld_grp(%d)\n",
		RtmpOsGetNetDevName(pNetDev), mld_grp);

	/* check mld exists */
	if (!BMGR_VALID_MLD_GRP_IDX(mld_grp) ||
		!BMGR_VALID_MLO_DEV(GET_MLD_BY_GRP_IDX(mld_grp))) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tmld(%d) do not exist!\n", mld_grp);
		return NDIS_STATUS_FAILURE;
	}

	entry = get_bss_entry_by_netdev(pNetDev);
	if (BMGR_VALID_BSS_ENTRY(entry)) {
		/* sanity check: entry is not affiliated with any mld */
		if (BMGR_VALID_MLO_DEV(entry->mld_ptr)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tentry(%s) has already affiliated with mld_grp(%d)\n",
				RtmpOsGetNetDevName(pNetDev), entry->entry_info.mld_grp);
			/* TODO: success */
			return NDIS_STATUS_SUCCESS;
		}

		/* sanity check: entry w/ link_id do not collide in MLD */
		bss_idx = entry->bss_idx;
		link_id = entry->link_id;
		mld = GET_MLD_BY_GRP_IDX(mld_grp);
		if (BMGR_VALID_BSS_IDX(mld->bss_idx_mld[link_id])) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tbss_idx(%d) can not join mld, which is occupied by bss_idx(%d)\n",
				bss_idx, mld->bss_idx_mld[link_id]);
			return NDIS_STATUS_FAILURE;
		}

		mutex_lock(&bss_mngr.op_mutex);
		/* sanity check: single link MLD can only have one link */
		if ((mld->mld_type == BMGR_MLD_TYPE_SINGLE) && (mld->mld_link_cnt == 1)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tError: single-link MLD (%d) can not be joined anymore.\n",
				mld->mld_grp);
			mutex_unlock(&bss_mngr.op_mutex);
			return NDIS_STATUS_FAILURE;
		}

		/* sanity check: the same BSSID as MLD addr of single link MLD */
		if ((mld->mld_type == BMGR_MLD_TYPE_SINGLE) &&
			(!MAC_ADDR_EQUAL(entry->entry_info.bssid, mld->mld_addr))) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tError: (single-link MLD) link addr(%pM) shall same as mld addr(%pM)\n",
				entry->entry_info.bssid, mld->mld_addr);
			mutex_unlock(&bss_mngr.op_mutex);
			return NDIS_STATUS_FAILURE;
		}

		/* MLD attribute and link attr. sanity */
		if (mld->mld_link_cnt > 0) {
			/* entry has the same SSID as MLD's SSID */
			if ((mld->mld_ssid_len != entry->entry_info.ssid_len) ||
				!NdisEqualMemory(mld->mld_ssid, entry->entry_info.ssid, mld->mld_ssid_len)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"\tNotice: SSID(%s) of BSS is different from MLD SSID(%s)\n",
					entry->entry_info.ssid, mld->mld_ssid);
				/* TODO: */
				/*
				mutex_unlock(&bss_mngr.op_mutex);
				return NDIS_STATUS_FAILURE;
				*/
			}
		}

		/* acquire hw resources */
		if (event_ops->bmgr_event_mld_add_link(pNetDev, mld_grp) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tError: acquire hw rsc for link failed (grp:%d)\n", mld_grp);
			mutex_unlock(&bss_mngr.op_mutex);
			return NDIS_STATUS_FAILURE;
		}

		/* Update MLD attr. by attr. of first joined BSS */
		if (mld->mld_link_cnt == 0) {
			/* SSID */
			NdisMoveMemory(mld->mld_ssid, entry->entry_info.ssid, entry->entry_info.ssid_len);
			mld->mld_ssid_len = entry->entry_info.ssid_len;
			mld->bmc_bss_idx = bss_idx;
		}

		/* join mld: update entry info in mld */
		mld->bss_idx_mld[link_id] = bss_idx;
		mld->mld_link_cnt++;

		/* join mld: update mld info in entry */
		entry->entry_info.mld_grp = mld_grp;
		entry->mld_ptr = mld;
		entry->bss_chg_cnt = 0;
		entry->mlo_en = TRUE;
		tid_map_sanity(entry);
		mutex_unlock(&bss_mngr.op_mutex);

		/* update to outside of BSS/MLD Manager */
		bss_mngr_con_fill_update_info(mld, &update_info);
		if (event_ops->bmgr_event_bss_attr_update(entry->pNetDev,
			&update_info, FALSE) != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
				"Fail: update bss %s in addlink failed\n",
				RtmpOsGetNetDevName(entry->pNetDev));
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"\tbss_idx(%d,%s) join mld_grp(%d) success\n",
			bss_idx, RtmpOsGetNetDevName(pNetDev),
			entry->entry_info.mld_grp);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tinvalid entry!\n");
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;

}

static int bss_mngr_con_mld_group_del_link(
	IN PNET_DEV pNetDev
)
{
	u8 mld_grp;
	u8 bss_idx;
	u8 link_id;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"===> wdev(%s) requests to leave mld_grp\n",
		RtmpOsGetNetDevName(pNetDev));

	entry = get_bss_entry_by_netdev(pNetDev);
	if (entry) {
		/* sanity check: entry is affiliated with a mld */
		if (!BMGR_VALID_MLO_DEV(entry->mld_ptr)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tentry(%s) is not in any mld, (grp_idx:%d)\n",
				RtmpOsGetNetDevName(pNetDev), entry->entry_info.mld_grp);
			return NDIS_STATUS_SUCCESS;
		}

		mld_grp = entry->entry_info.mld_grp;
		if (BMGR_VALID_MLD_GRP_IDX(mld_grp))
			mld = GET_MLD_BY_GRP_IDX(mld_grp);

		if (BMGR_VALID_MLO_DEV(mld)) {
			bss_idx = entry->bss_idx;
			link_id = entry->link_id;

			/* sanity check */
			if (mld->bss_idx_mld[link_id] != bss_idx) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"\tError: link(%d) in mld(%d) is %d, not %d\n",
					link_id, mld_grp, mld->bss_idx_mld[link_id], bss_idx);
				return NDIS_STATUS_FAILURE;
			}

			mutex_lock(&bss_mngr.op_mutex);
			/* leave mld: update entry info in mld */
			mld->bss_idx_mld[link_id] = BMGR_INVALID_BSS_IDX;
			mld->mld_link_cnt--;

			/* leave mld: update mld info in entry */
			entry->mld_ptr = GET_MLD_BY_GRP_IDX(BMGR_INVALID_MLD_GRP);
			entry->mlo_en = FALSE;
			tid_map_sanity(entry);

			/* release hw resources */
			if (event_ops->bmgr_event_mld_del_link(pNetDev) != NDIS_STATUS_SUCCESS) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"\tError: release hw rsc for link failed (grp:%d)\n", mld_grp);
				mutex_unlock(&bss_mngr.op_mutex);
				return NDIS_STATUS_FAILURE;
			}
			/* handle non-AP STAs */
			if (mld->bmc_bss_idx == bss_idx) {
				for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
					if (BMGR_VALID_BSS_IDX(mld->bss_idx_mld[link_id])) {
						mld->bmc_bss_idx = mld->bss_idx_mld[link_id];
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
							"\tset bmc_bss=%d\n", mld->bmc_bss_idx);
						break;
					}
				}
			}
			mutex_unlock(&bss_mngr.op_mutex);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"\tbss_idx(%d,%s) leave mld_grp(%d) success\n",
				bss_idx, RtmpOsGetNetDevName(pNetDev),
				entry->entry_info.mld_grp);
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tmld(%d) not inited!\n", mld_grp);
			return NDIS_STATUS_FAILURE;
		}

	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"\tentry not inited!\n");
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;

}

int bss_mngr_con_sync_mld_relation(struct bmgr_entry *entry)
{
	int ret = NDIS_STATUS_SUCCESS;
	u8 bss_idx = entry->bss_idx;
	u8 link_id = entry->link_id;
	u8 mlo_en = entry->mlo_en;
	struct bmgr_reg_info *reg_info = &entry->entry_info;

	u8 mld_grp = reg_info->mld_grp;
	struct bmgr_mlo_dev *mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ?
		GET_MLD_BY_GRP_IDX(mld_grp) : NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d] link(%d), mlo_en(%d)\n",
			bss_idx, link_id, mlo_en);

	if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		/* if mld do not exist, create one */
		if (!BMGR_VALID_MLO_DEV(mld)) {
			struct bmgr_mld_attr mld_attr = {0};
			u8 mld_type = BMGR_IS_ML_MLD_GRP_IDX(mld_grp) ?
				BMGR_MLD_TYPE_MULTI : BMGR_MLD_TYPE_SINGLE;
			/* mld_addr from user profile or BSSID directly */
			/* MLD MAC addr of single-link MLD = single-link link MAC addr */
			u8 *mld_addr = (BMGR_IS_ML_MLD_GRP_IDX(mld_grp) && reg_info->mld_addr_by_cfg) ?
				reg_info->pf_mld_addr : reg_info->bssid;

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
				"Warn: %s create MLD\n", RtmpOsGetNetDevName(entry->pNetDev));

			/* default configuration of MLD */
			mld_attr.eml_caps.emlsr_supp = TRUE;
			mld_attr.eml_caps.en = TRUE;
			mld_attr.mld_caps.t2l_nego_supp = 1;
			mld_attr.mld_caps.en = TRUE;

			ret = bss_mngr_con_mld_group_create(entry->pNetDev,
				&mld_grp, mld_addr, &mld_attr, mld_type);
		}

		/* mld exists, entry joins to mld */
		if (ret == NDIS_STATUS_SUCCESS)
			ret = bss_mngr_con_mld_group_add_link(entry->pNetDev, mld_grp);

		/* TODO: put change of mlo_en flag in add_link */
		if (ret != NDIS_STATUS_SUCCESS)
			entry->mlo_en = FALSE;
	} else {
		ret = bss_mngr_con_mld_group_del_link(entry->pNetDev);

		/* TODO: if there is no entry in mld, delete mld */
		if (mld && mld->mld_link_cnt == 0)
			ret = bss_mngr_con_mld_group_destroy(entry->pNetDev, mld_grp);
	}

	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tError: fail to sync mld relation!!!\n");

	return ret;
}

static int bss_mngr_con_mld_reconfig_ie_op(
	IN struct mld_reconfig_ie_op *ie_op)
{
	struct bmgr_mlo_dev *mld = BMGR_VALID_MLD_GRP_IDX(ie_op->mld_grp_idx) ?
		GET_MLD_BY_GRP_IDX(ie_op->mld_grp_idx) : NULL;
	struct mld_reconfig_ie_build_t *reconf_build;
	struct mld_reconfig_ie_clean_t *reconf_clean;
	struct mld_reconfig_ie_query_t *reconf_query;
	struct reconfig_ml_ie_t *reconf_ie;
	struct _EID_STRUCT *eid;
	int ret = NDIS_STATUS_FAILURE;
	u8 *f_buf, *pos, link;
	u16 link_info_len;

	if (!BMGR_VALID_MLO_DEV(mld))
		return ret;

	switch (ie_op->op) {
	case BMGR_RECONFIG_IE_OP_BUILD:
		reconf_build = ie_op->reconf_build;
		if (!reconf_build)
			return ret;

		if (!mld_alloc_reconfig_ml_ie(mld))
			return ret;

		if (!build_reconfig_multi_link_ie(mld, reconf_build)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
				"build Reconfiguration multi-link IE failed\n");
			return ret;
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_NOTICE,
			"build ie done status(%d), cmm(%d)\n",
			mld->reconf_ie.ie_status,
			mld->reconf_ie.common_info.len);

		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
				"link%d(%d), offset(%d), bss_idx(%d)\n", link,
				mld->reconf_ie.link_info[link].len,
				mld->reconf_ie.tmr_offset[link],
				mld->reconf_ie.fw_bss_idx[link]);
		}

		mld->reconf_ie.ie_status = BUF_STAT_IE_READY;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
			"update ie status done %d\n", mld->reconf_ie.ie_status);

		break;
	case BMGR_RECONFIG_IE_OP_CLEAN:
		mld_clean_reconfig_ml_ie(mld);
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
			"clean done, ie status %d\n", mld->reconf_ie.ie_status);
		break;
	case BMGR_RECONFIG_IE_OP_CLEAN_LINK: /* TODO */
		reconf_clean = ie_op->reconf_clean;
		if (!reconf_clean)
			return ret;

		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (reconf_clean->clean_links[link]) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_INFO,
					"link%d(%d), offset(%d), bss_idx(%d)\n", link,
					mld->reconf_ie.link_info[link].len,
					mld->reconf_ie.tmr_offset[link],
					mld->reconf_ie.fw_bss_idx[link]);
			}
		}
		break;
	case BMGR_RECONFIG_IE_OP_QUERY:
		reconf_query = ie_op->reconf_query;
		if (!reconf_query)
			return ret;

		reconf_query->f_len = 0;

		reconf_ie = &mld->reconf_ie;
		if (!(reconf_ie->ie_status == BUF_STAT_IE_READY)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
				"Reconfig IE not ready %d\n", mld->reconf_ie.ie_status);
			return ret;
		}

		/* common info */
		if (reconf_ie->common_info.buf) {
			NdisMoveMemory(reconf_query->f_buf,
				reconf_ie->common_info.buf, reconf_ie->common_info.len);
			reconf_query->f_len = reconf_ie->common_info.len;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
				"RMLIE Common: len=%d, f_buf=%p\n", reconf_query->f_len, reconf_query->f_buf);
		}

		if (!reconf_query->f_len)
			return ret;

		/* link info */
		f_buf = pos = reconf_query->f_buf + reconf_query->f_len;
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (reconf_ie->link_info[link].buf && reconf_ie->link_info[link].len) {
				reconf_query->tmr_offset[link] = reconf_query->f_len
					+ (pos - f_buf) + reconf_ie->tmr_offset[link];

				NdisMoveMemory(pos, reconf_ie->link_info[link].buf,
					reconf_ie->link_info[link].len);
				pos += reconf_ie->link_info[link].len;

				reconf_query->fw_bss_idx[link] = reconf_ie->fw_bss_idx[link];

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
					"LinkInfo: offset=%d, fw_bss=%d, len=%d\n", reconf_query->tmr_offset[link],
					reconf_query->fw_bss_idx[link], reconf_ie->link_info[link].len);
			}
		}

		/* length of all Per-STA Profile(s) */
		link_info_len = pos - f_buf;
		reconf_query->f_len += link_info_len;

		eid = (struct _EID_STRUCT *)reconf_query->f_buf;
		eid->Len = reconf_query->f_len - 2;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_DEBUG,
			"ReconfigIE: len=%d, len in tlv=%d\n", reconf_query->f_len, eid->Len);

		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_RECONFIG, DBG_LVL_ERROR,
			"Unknown Op %d\n", ie_op->op);
		return ret;
	}

	return NDIS_STATUS_SUCCESS;
}



/**
 * @ML reconfiguration: handle peer mld_sta when MLR
 *
 * @param mld_grp_idx ap mld group index
 * @param rm_link_id_bitmap link id bitmap of remove link
 * @param *link_id_bss_info_idx bss_info_indx of respect link id
 */
int bss_mngr_con_mld_reconfig_peer_mld(
	IN u8 mld_grp_idx,
	IN u16 rm_link_id_bitmap)
{
	int ret = 0;
	u16 i = 0;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_sta *mld_sta = NULL;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	mld = BMGR_VALID_MLD_GRP_IDX(mld_grp_idx) ? GET_MLD_BY_GRP_IDX(mld_grp_idx) : NULL;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		ret = -EINVAL;
		goto err;
	}

	/*non-AP MLD*/
	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		mld_sta = GET_MLD_STA_BY_IDX(i);
		if (!mld_sta || !mld_sta->valid)
			continue;
		if (mld_sta->mld_grp_idx != mld->mld_grp)
			continue;
		event_ops->bmgr_event_mld_reconfig(mld_sta->mld_addr, rm_link_id_bitmap);
	}

	return ret;
err:
	return ret;
}
#endif

#define BCN_UPDATE_IOB_TX_BSS 1
#define BCN_UPDATE_OOB_TX_BSS 2
#define BCN_UPDATE_OOB_NTX_BSS 3

static u8 is_peer_bcn_update_require(
	IN struct bmgr_entry *peer_entry,
	IN struct bmgr_entry *entry
)
{
	u8 bss_idx_tx = entry->bss_idx_tx;
	u8 mld_grp = entry->entry_info.mld_grp;
	u8 band_idx = entry->entry_info.dev_cfg_idx;
	u8 peer_bss_idx = peer_entry->bss_idx;
	u8 peer_bss_idx_tx = peer_entry->bss_idx_tx;
	u8 peer_mld_grp = peer_entry->entry_info.mld_grp;
	u8 peer_band_idx = peer_entry->entry_info.dev_cfg_idx;
	u8 peer_dev_type = peer_entry->entry_info.dev_type;
	u8 up_type = 0;

	if (peer_band_idx == band_idx) {
		/* inband: update self or tx-bss */
		if (peer_bss_idx == bss_idx_tx)
			up_type = BCN_UPDATE_IOB_TX_BSS;
	} else {
		/* outband: mld member or all(for 6G) */
		if ((mld_grp != 0) && (peer_mld_grp == mld_grp)) {
			/* mld group member */
			if (IS_DEV_TYPE_W_BCN(peer_dev_type)) {
				up_type = BCN_UPDATE_OOB_TX_BSS;
			} else if (IS_DEV_TYPE_11V_NT(peer_dev_type)) {
				if (BMGR_VALID_BSS_IDX(peer_bss_idx_tx))
					up_type = BCN_UPDATE_OOB_NTX_BSS;
			}
		}
		/*
		 * TBD: outband rnr required for 6G devices
		 */
	}
	return up_type;
}

int bss_mngr_con_bcn_update(IN struct bmgr_entry *entry)
{
	u8 bss_idx = entry->bss_idx;
	int ret = NDIS_STATUS_FAILURE;
	struct bss_mngr_event_ops *event_ops;
	struct bmgr_entry *peer_entry = NULL;
	u8 peer_bss_idx;
	u8 up_type = 0;
	static const char * const type_str[] = {
		"None",
		"IoB-TxBss",
		"OoB-TxBss",
		"OoB-NTxBss"
	};

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d]\n", bss_idx);

	/* traverse bss be updated */
	for (peer_bss_idx = 0; peer_bss_idx < BSS_MNGR_ENTRY_CNT; peer_bss_idx++) {
		peer_entry = GET_BSS_ENTRY_BY_IDX(peer_bss_idx);

		up_type = 0;
		if (BMGR_VALID_BSS_ENTRY(peer_entry))
			up_type = is_peer_bcn_update_require(peer_entry, entry);

		/* patch peer_entry to oob tx-bss */
		if (up_type == BCN_UPDATE_OOB_NTX_BSS)
			peer_entry = GET_BSS_ENTRY_BY_IDX(peer_entry->bss_idx_tx);

		if (up_type) {
			event_ops = peer_entry->entry_info.event_ops;
			if (event_ops) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"\t->[%d]: %s\n", peer_entry->bss_idx, type_str[up_type]);
				event_ops->bmgr_event_bcn_update(peer_entry->pNetDev);
				ret = NDIS_STATUS_SUCCESS;
			}
		}
	}

	return ret;
}

int bss_mngr_con_sync_bcn_update(IN PNET_DEV pNetDev)
{
	struct bmgr_entry *entry;

	if (pNetDev == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"null invalid param\n");
		return NDIS_STATUS_FAILURE;
	}

	entry = get_bss_entry_by_netdev(pNetDev);
	if (entry == NULL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"bss not found\n");
		return NDIS_STATUS_FAILURE;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATAP_BCN, DBG_LVL_NOTICE, "link_id=%d\n", entry->link_id);

	return bss_mngr_con_bcn_update(entry);
}

int bss_mngr_con_sync_entry(IN struct bmgr_entry *entry)
{
	int ret = NDIS_STATUS_FAILURE;

	if (entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "[%d]\n",
				 entry->bss_idx);

#ifdef DOT11_EHT_BE
		/* sync mld relation */
		bss_mngr_con_sync_mld_relation(entry);
#endif
		/* re-build ie(s) */
		bss_mngr_con_sync_ie(entry);

		/* re-sync beacon */
		bss_mngr_con_bcn_update(entry);
	}

	return ret;
}
void *bss_mngr_get_pad_by_band_idx(u8 band_idx)
{
	u8 i;
	struct bmgr_entry *entry = NULL;
	void *tmp_pad = NULL;

	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		entry = GET_BSS_ENTRY_BY_IDX(i);
		if (entry && entry->valid &&
			entry->entry_info.dev_cfg_idx == band_idx) {
			if (entry->pNetDev)
				tmp_pad = RTMP_OS_NETDEV_GET_PRIV(entry->pNetDev);
		}
	}

	return tmp_pad;
}

#ifdef CONFIG_6G_SUPPORT
static bool bss_mngr_is_ap_6g_only(void)
{
	u8 i;
	u16 phy_mode;
	struct bmgr_entry *entry = NULL;
	bool is_6g_only = TRUE;

	/* traverse all BSS members and check whether 2G or 5G exists */
	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		entry = GET_BSS_ENTRY_BY_IDX(i);
		if (BMGR_VALID_BSS_ENTRY(entry)) {
			phy_mode = entry->entry_info.phy_mode;
			if (!WMODE_CAP_6G(phy_mode)) {
				is_6g_only = FALSE;
				break;
			}
		}
	}

	return is_6g_only;
}

static int bss_mngr_update_iob_disc(void)
{
	u8 i;
	u8 by_cfg, iob_type, iob_interval, iob_mode;
	u16 phy_mode;
	struct bmgr_entry *entry = NULL;
	struct wifi_dev *wdev = NULL;

	/*
	 * 1. AP is operating in 6 GHz only
	 *	- allowed to transmit either UPR or FD frames
	 *
	 * 2. AP is operating in multiple bands (including 6 GHz)
	 *	- Disable transmission of UPR and FD frames
	 */

	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		entry = GET_BSS_ENTRY_BY_IDX(i);
		if (BMGR_VALID_BSS_ENTRY(entry)) {
			wdev = RtmpOsGetNetDevWdev(entry->pNetDev);

			/* ignore non-tx BSSID */
			if (entry->entry_info.dev_type == DEV_TYPE_AP_11VNT) {
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO,
					"\t[%s] iob dsc ignored due to 11v non-tx\n",
					RtmpOsGetNetDevName(entry->pNetDev));
				continue;
			}

			/* ignore when configured by profile or user commands */
			by_cfg = wlan_config_get_unsolicit_tx_by_cfg(wdev);
			if (by_cfg) {
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_INFO,
					"\t[%s] iob dsc ignored by cfg\n", RtmpOsGetNetDevName(entry->pNetDev));
				continue;
			}

			/* set 2G/5G/6G iob discovery rule (based on SPEC) */
			phy_mode = entry->entry_info.phy_mode;
			if (WMODE_CAP_6G(phy_mode)) {
				if (bss_mngr_is_ap_6g_only()) {
					/* single 6G: enable 6G iob discovery frame offload */
					iob_type = UNSOLICIT_TX_PROBE_RSP;
				} else {
					/* multi-band: disable 6G iob discovery frame offload */
					iob_type = UNSOLICIT_TX_DISABLE;
				}

				/* value from rtmp_read_6g_discov_cfg_from_file or user commands */
				iob_interval = wlan_config_get_unsolicit_tx_tu(wdev);
				iob_mode = wlan_config_get_unsolicit_tx_mode(wdev);
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_DEBUG,
					"\t[%s] 6G: only:%d iob\n", RtmpOsGetNetDevName(entry->pNetDev),
					bss_mngr_is_ap_6g_only());
			} else {
				/* 2g/5g: default config */
				iob_type = UNSOLICIT_TX_DISABLE;
				iob_interval = 0;
				iob_mode = UNSOLICIT_TXMODE_NON_HT;
				MTWF_DBG(NULL, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_DEBUG,
					"\t[%s] 2G/5G iob\n", RtmpOsGetNetDevName(entry->pNetDev));
			}

			MTWF_DBG(NULL, DBG_CAT_AP, CATAP_MBSS, DBG_LVL_DEBUG,
					"\t[%s] iob type=%d, interval=%d, rate mode=%d\n",
					RtmpOsGetNetDevName(entry->pNetDev),
					iob_type, iob_interval, iob_mode);

			in_band_discovery_update_oper(wdev, iob_type, iob_interval, iob_mode);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS bss_mngr_con_update_oob_6g_disc(
	IN PNET_DEV pNetDev,
	IN u8 rnr_6g_rule)
{
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"===> netdev(%s), rnr_6g_rule(%d)\n",
		RtmpOsGetNetDevName(pNetDev), rnr_6g_rule);

	if (BMGR_VALID_BSS_ENTRY(entry)) {
		entry->entry_info.rnr_6g_rule = rnr_6g_rule;
		bss_mngr_con_ie_update_rnr_for_6g(entry);
		return NDIS_STATUS_SUCCESS;
	}

	return NDIS_STATUS_FAILURE;
}
#endif
static int bss_mngr_con_dev_reg(
	IN PNET_DEV pNetDev,
	IN struct bmgr_reg_info *reg_info
)
{
	bool is_exist = FALSE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct repting_rnr_info *rnr_info = NULL;
	u8 bss_idx = BMGR_INVALID_BSS_IDX;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"===> netdev(%02d) (%s)\n",
			RtmpOsGetNetIfIndex(pNetDev), RtmpOsGetNetDevName(pNetDev));

	if (entry) {
		is_exist = TRUE;
		bss_idx = entry->bss_idx;
		entry->pNetDev = pNetDev;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN, "\tentry exist!!\n");
	}

	if (!is_exist) {
		/* assign new */
		bss_idx = bss_idx_aquire();
		if (BMGR_VALID_BSS_IDX(bss_idx)) {
			os_alloc_mem(NULL, (u8 **)&entry, sizeof(struct bmgr_entry));

			if (entry) {
				NdisZeroMemory(entry, sizeof(struct bmgr_entry));
				entry->pNetDev = pNetDev;
				entry->bss_idx = bss_idx;
				entry->pwdev = RtmpOsGetNetDevWdev(pNetDev);
				mutex_lock(&bss_mngr.op_mutex);
				bss_mngr.entry[bss_idx] = entry;
				BSS_MNGR_ENTRY_CNT++;
				mutex_unlock(&bss_mngr.op_mutex);
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"\tentry alloc failed!!\n");
				goto err;
			}
		} else {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"\tentry invalid(%d)!!\n", bss_idx);
			goto err;
		}
	}

	/* fill entry info */
	entry->valid = TRUE;
	NdisMoveMemory(&entry->entry_info, reg_info, sizeof(struct bmgr_reg_info));
	entry->bss_idx_tx = get_tx_bss_idx(entry);
	entry->dot11be_mld_id = IS_DEV_TYPE_11V_NT(entry->entry_info.dev_type) ?
		entry->entry_info.dot11v_mbssid_idx : 0;
	entry->link_id = entry->entry_info.dev_cfg_idx;
	entry->tid_map = entry->entry_info.tid_map;
	entry->bss_chg_cnt = 0;

	rnr_info = &entry->rnr_info;
	rnr_info->repting_cnt = 0;
	NdisZeroMemory(rnr_info->repting_bss_idx, sizeof(rnr_info->repting_bss_idx));
	NdisZeroMemory(rnr_info->bss_idx_bitmap, sizeof(rnr_info->bss_idx_bitmap));

	/* mlo */
#ifdef DOT11_EHT_BE
	if (entry->entry_info.mld_grp && WMODE_CAP_BE(entry->entry_info.phy_mode))
		entry->mlo_en = TRUE;
#endif

	/* alloc ie(s) buffer */
	if (bss_mngr_con_ie_buf_alloc(entry) != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tie buf alloc failed!!\n");
		goto err;
	}

	/* clean ie buffer */
	bss_mngr_con_ie_buf_clean(entry);

	/* sync entry for any change */
	bss_mngr_con_sync_entry(entry);

#ifdef CONFIG_6G_SUPPORT
	/* update iob discovery rule for any interface up */
	bss_mngr_update_iob_disc();
#endif

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"<=== entry = %p(bss_idx_tx = %d) (dev_cnt = %d)\n",
			entry, entry->bss_idx_tx, BSS_MNGR_ENTRY_CNT);

	return NDIS_STATUS_SUCCESS;

err:
	if (entry) {
		bss_mngr_con_ie_buf_free(entry);
		os_free_mem(entry);
	}
	if (BMGR_VALID_BSS_IDX(bss_idx)) {
		mutex_lock(&bss_mngr.op_mutex);
		bss_mngr.entry[bss_idx] = NULL;
		bss_idx_clear(bss_idx);
		mutex_unlock(&bss_mngr.op_mutex);
	}
	return NDIS_STATUS_FAILURE;
}

static int bss_mngr_con_dev_dereg(
	IN PNET_DEV pNetDev
)
{
	struct bmgr_entry *entry = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"===> netdev(%02d) (%s)\n",
			RtmpOsGetNetIfIndex(pNetDev), RtmpOsGetNetDevName(pNetDev));

	entry = get_bss_entry_by_netdev(pNetDev);
	if (entry) {
		entry->valid = FALSE;
		entry->mlo_en = FALSE;

		/* sync entry for any change */
		bss_mngr_con_sync_entry(entry);

#ifdef CONFIG_6G_SUPPORT
		/* update iob discovery rule for any interface down */
		bss_mngr_update_iob_disc();
#endif
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tentry not found!!\n");
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"<=== %s:\n", __func__);

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_con_dev_update(
	IN PNET_DEV pNetDev,
	IN struct bmgr_reg_info *reg_info
)
{
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"===> netdev(%02d) (%s)\n",
			RtmpOsGetNetIfIndex(pNetDev), RtmpOsGetNetDevName(pNetDev));

	if (entry) {
		/* fill entry info */
		NdisMoveMemory(&entry->entry_info, reg_info, sizeof(struct bmgr_reg_info));
		entry->bss_idx_tx = get_tx_bss_idx(entry);
		entry->dot11be_mld_id = IS_DEV_TYPE_11V_NT(entry->entry_info.dev_type) ?
			entry->entry_info.dot11v_mbssid_idx : 0;
		entry->link_id = entry->entry_info.dev_cfg_idx;
		entry->tid_map = entry->entry_info.tid_map;

		/* re-build ie(s) */
		bss_mngr_con_sync_ie(entry);

		/* re-sync beacon */
		bss_mngr_con_bcn_update(entry);

	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tentry not found!!\n");
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"<=== %s:\n", __func__);

	return NDIS_STATUS_SUCCESS;
}

static int bss_mngr_ext_entry_init(void)
{
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;

	u8 bss_idx = BMGR_INVALID_BSS_IDX;
	u8 mld_grp_idx = BMGR_INVALID_MLD_GRP;

	bss_idx_set(bss_idx);
	os_alloc_mem(NULL, (u8 **)&entry, sizeof(struct bmgr_entry));
	if (entry) {
		/* valid/mlo_en=FALSE */
		NdisZeroMemory(entry, sizeof(struct bmgr_entry));
		entry->bss_idx = bss_idx;
		bss_mngr.entry[bss_idx] = entry;
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tentry alloc failed!!\n");
		goto err;
	}

#ifdef DOT11_EHT_BE
	mld_grp_set(mld_grp_idx);
	os_alloc_mem(NULL, (u8 **)&mld, sizeof(struct bmgr_mlo_dev));
	if (mld) {
		/* valid=FALSE */
		NdisZeroMemory(mld, sizeof(struct bmgr_mlo_dev));
		mld->mld_grp = mld_grp_idx;
		bmgr_mld_reset(mld);
		bss_mngr.mld[mld_grp_idx] = mld;

		entry->mld_ptr = mld;
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\tmld alloc failed!!\n");
		goto err;
	}
#endif

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
		"bss(%d), grp(%d)\n", bss_idx, mld_grp_idx);

	return NDIS_STATUS_SUCCESS;

err:
	if (entry)
		os_free_mem(entry);

	/*
	if (mld)
		os_free_mem(mld);
	*/

	return NDIS_STATUS_FAILURE;
}

static int bss_mngr_con_init(void)
{
	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"inited = %d\n", bss_mngr.inited);

	if (!bss_mngr.inited) {
		os_zero_mem(&bss_mngr, sizeof(bss_mngr));
		bss_idx_bitmap_init();
#ifdef DOT11_EHT_BE
		mld_grp_bitmap_init();
		mld_sta_init();
		mld_twt_init(&bss_mngr.mlo_twt);
#endif
		mutex_init(&bss_mngr.op_mutex);
		bss_mngr_ext_entry_init();
		bss_mngr.event_ops = &bmgr_event_ops;

		bss_mngr.inited = TRUE;
	}

	return NDIS_STATUS_SUCCESS;
}

static int bss_mngr_con_deinit(void)
{
	struct bmgr_entry *entry = NULL;	/*, bmg_entry_tmp = NULL*/
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_sta *mld_sta = NULL;
	int i, j;
	u8 *sta_pf;
	int dev_cnt = BSS_MNGR_ENTRY_CNT;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"inited = %d, dev_cnt = %d\n",
			bss_mngr.inited, BSS_MNGR_ENTRY_CNT);

	if (bss_mngr.inited) {
		mutex_lock(&bss_mngr.op_mutex);

		/* free BSS entry */
		for (i = 0; i < dev_cnt; i++) {
			entry = GET_BSS_ENTRY_BY_IDX(i);
			if (entry) {
				bss_mngr_con_dev_dereg(entry->pNetDev);
				bss_mngr_con_ie_buf_free(entry);
				os_free_mem(entry);
				BSS_MNGR_ENTRY_CNT--;
				bss_mngr.entry[i] = NULL;
			}
		}
		entry = GET_BSS_ENTRY_BY_IDX(BMGR_INVALID_BSS_IDX);
		if (entry) {
			os_free_mem(entry);
			bss_mngr.entry[BMGR_INVALID_BSS_IDX] = NULL;
		}

		/* free MLD */
		for (i = 0; i < BMGR_MAX_MLD_GRP_CNT; i++) {
			mld = GET_MLD_BY_GRP_IDX(i);
			if (mld) {
				// bss_mngr_con_mld_group_destroy(mld->mld_grp);
				mld_free_reconfig_ml_ie(mld);
				os_free_mem(mld);
				bss_mngr.mld[i] = NULL;
			}
		}

		/* free peer MLD */
		for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
			mld_sta = GET_MLD_STA_BY_IDX(i);

			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				sta_pf = mld_sta->mld_link[j].assoc_rsp_sta_profile.buf;
				if (sta_pf)
					os_free_mem(sta_pf);
			}
		}

#ifdef DOT11_EHT_BE
				mld_twt_deinit(&bss_mngr.mlo_twt);
#endif /* DOT11_EHT_BE */

		if (BSS_MNGR_ENTRY_CNT == 0) {
			bss_idx_bitmap_deinit();
#ifdef DOT11_EHT_BE
			mld_grp_bitmap_deinit();
#endif
			mutex_unlock(&bss_mngr.op_mutex);
			mutex_init(&bss_mngr.op_mutex);
			bss_mngr.inited = FALSE;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"%s: de-init\n", __func__);
		} else
			mutex_unlock(&bss_mngr.op_mutex);
	}

	return NDIS_STATUS_SUCCESS;
}

#ifdef DOT11_EHT_BE
static u8 *bss_mngr_get_mld_type_str(enum bmgr_mld_type mld_type)
{
	switch (mld_type) {
	case BMGR_MLD_TYPE_NONE:
		return "None";
	case BMGR_MLD_TYPE_SINGLE:
		return "Single";
	case BMGR_MLD_TYPE_MULTI:
		return "Multi";
	default:
		return "Unknown";
	}
}
#endif /* DOT11_EHT_BE */

static int bss_mngr_con_info_show(u8 type)
{
	struct bmgr_entry *entry = NULL;
	struct bmgr_reg_info *reg_info = NULL;
	u8 *str = NULL;
	int i;

	MTWF_PRINT("\n%s: type = %d\n", __func__, type);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
		"\tbmg_entry ptr = %p, sizeof = %d\n",
		bss_mngr.entry, (u32)sizeof(bss_mngr.entry));

#ifdef DOT11_EHT_BE
	/* MLD */
	MTWF_PRINT("%s: MLO Device, (%d)\n", __func__, BMGR_MAX_MLD_GRP_CNT);

	for (i = 0; i < BMGR_MAX_MLD_GRP_CNT; i++) {
		int j;
		struct bmgr_mlo_dev *mld = GET_MLD_BY_GRP_IDX(i);

		if (mld) {
			if (!mld->valid)
				continue;

			MTWF_PRINT(" - MLD[%02d] (%s), bmgr_mlo_dev ptr = %p\n",
				mld->mld_grp,
				mld->valid ? "active" : "inactive", mld);

			MTWF_PRINT("\t\tMLD Addr:(%pM) Type:(%s) link_cnt:(%d), bmc_bss_idx(%d)\n",
				mld->mld_addr, bss_mngr_get_mld_type_str(mld->mld_type),
				mld->mld_link_cnt,
				mld->bmc_bss_idx);

			MTWF_PRINT("\t\tEMLSR:En(%d)/To(%d)/OMN(%d), TTLM:Supp(%d)\n",
				mld->attr.eml_caps.emlsr_supp, mld->attr.eml_caps.trans_to,
				mld->attr.eml_caps.eml_omn_en,
				mld->attr.mld_caps.t2l_nego_supp);

			MTWF_PRINT("\t\tMedSync:Pres(%d)/Dur(%d)/OfdmThr(%d)/MaxTxop(%d)\n",
				mld->attr.med_sync.en,
				mld->attr.med_sync.dur,
				mld->attr.med_sync.ofdm_ed_thr,
				mld->attr.med_sync.max_txop);

			if (mld->reconf_ie.ie_status >= BUF_STAT_ALLOCED) {
				MTWF_PRINT("\t\tReconfig:Sts(%d)/Len:Cmm(%d)",
					mld->reconf_ie.ie_status,
					mld->reconf_ie.common_info.len);

				for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
					if (mld->reconf_ie.link_info[j].len)
						MTWF_PRINT(",Link%d(%d)", j, mld->reconf_ie.link_info[j].len);
				}

				MTWF_PRINT("\n");
			}

			/* TODO:
			MTWF_PRINT("\t\tMLD SSID:(%s)\n", mld->mld_ssid);
			*/

			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				u8 bss_idx_mld = mld->bss_idx_mld[j];

				if (BMGR_VALID_BSS_IDX(bss_idx_mld)) {
					MTWF_PRINT("\t\t- Link%d: [%02d]\n",
						j, bss_idx_mld);
				}
			}
		}
	}

#endif /* DOT11_EHT_BE */

	/* MLD AP(BSS) */
	MTWF_PRINT("\n%s: Entry, dev_cnt = %d\n", __func__, BSS_MNGR_ENTRY_CNT);

	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		entry = GET_BSS_ENTRY_BY_IDX(i);

		if (!entry)
			continue;

		reg_info = &entry->entry_info;
		MTWF_PRINT(" - [%02d] %s (%s), bmg_entry ptr = %p\n",
			entry->bss_idx,
			entry->valid ? RtmpOsGetNetDevName(entry->pNetDev) : "n/a",
			entry->valid ? "active" : "inactive", entry);

		MTWF_PRINT("\t\tNetDev:%p(%d), Type: %s\n",
			entry->pNetDev,
			entry->valid ? RtmpOsGetNetIfIndex(entry->pNetDev) : 0,
			(reg_info->dev_type < DEV_TYPE_UNDEF) ?
				dev_type_str[reg_info->dev_type] : "INVLD");

		if (!entry->valid)
			continue;

		MTWF_PRINT("\t\t11vMbssIdx:%d, MaxIndicator:%d, BcnInt:%d, fw_idx:%d\n",
			reg_info->dot11v_mbssid_idx,
			reg_info->max_bssid_indicator,
			reg_info->bcn_interval,
			reg_info->hw_bss_index);
		MTWF_PRINT("\t\tSSID:%s (%08x)\t(%pM)\n",
			reg_info->ssid,
			(u32)Crcbitbybitfast(reg_info->ssid, reg_info->ssid_len),
			reg_info->bssid);

		str = wmode_2_str(reg_info->phy_mode);
		if (str) {
			MTWF_PRINT("\t\tPhymode:%s (0x%x), Ch=%3d, OpClass=%d\n",
				str, reg_info->phy_mode, reg_info->channel, reg_info->op_class);
			os_free_mem(str);
		}

		MTWF_PRINT("\t\tAuthMode:%s, Cipher(P:%s/G:%s), 6G-RNR:%s\n",
			GetAuthModeStr(reg_info->auth_mode),
			GetEncryModeStr(reg_info->PairwiseCipher),
			GetEncryModeStr(reg_info->GroupCipher),
			(reg_info->rnr_6g_rule < RNR_REPORTING_INVLD) ?
				rnr_rule_str[reg_info->rnr_6g_rule] : "INVLD");

		MTWF_PRINT("\t\t[MNGR] TxIdx:[%d], 11beMldIdx:%d, LinkIdx:%d\n",
			entry->bss_idx_tx,
			entry->dot11be_mld_id,
			entry->link_id);

#ifdef DOT11_EHT_BE
		if (entry->mlo_en) {
			MTWF_PRINT("\t\t[MNGR] MldGrp:%d, TidMap:0x%x->0x%x\n",
				reg_info->mld_grp,
				reg_info->tid_map, entry->tid_map);

			MTWF_PRINT("\t\t[MNGR] MLD enabled:\t(chg_cnt:%d)\n",
				entry->bss_chg_cnt);
		}
#endif
		if (entry->mbss.tx_bcn.len || entry->mbss.tx_probe.len || entry->mbss.tx_probe_cmm.len) {
			MTWF_PRINT("\t\t[MNGR] IE Len (MBSS-BCN/PRB/PRB-C) = (%d/%d/%d)\n",
				entry->mbss.tx_bcn.len,
				entry->mbss.tx_probe.len,
				entry->mbss.tx_probe_cmm.len);
		}

		if (entry->rnr.len || entry->mld.len || entry->mld_cmm.len || entry->tid2lnk.len ||
			entry->rnr_info.repting_rnr.len) {
			MTWF_PRINT(
				"\t\t[MNGR] IE Len (RNR-O/RNR-M/MLD/MLD-C/T2L) = (%d/%d/%d/%d/%d)\n",
				entry->rnr_info.repting_rnr.len,
				entry->rnr.len,
				entry->mld.len,
				entry->mld_cmm.len,
				entry->tid2lnk.len);
		}

#ifdef DOT11_EHT_BE
		if (entry->mlo_en) {
			int j;

			MTWF_PRINT("\t\t[MNGR] ML Probe Rsp PF:");
			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				if (entry->ml_probe_rsp_per_sta_pf[j].len)
					MTWF_PRINT(" Link%d(%d)", j, entry->ml_probe_rsp_per_sta_pf[j].len);
			}
			MTWF_PRINT("\n");

			MTWF_PRINT("\t\t[MNGR] ML Beacon Appl PF:");
			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				if (entry->bcn_appl_elem_per_sta_pf[j].len)
					MTWF_PRINT(" Link%d(%d)", j, entry->bcn_appl_elem_per_sta_pf[j].len);
			}
			MTWF_PRINT("\n");
		}
#endif
		if (entry->rnr.buf && entry->rnr.len)
			hex_dump_with_cat_and_lvl("RNR:", entry->rnr.buf, entry->rnr.len,
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);
		if (entry->mbss.tx_bcn.buf && entry->mbss.tx_bcn.len)
			hex_dump_with_cat_and_lvl("MBSS-BCN:", entry->mbss.tx_bcn.buf, entry->mbss.tx_bcn.len,
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);
		if (entry->mbss.tx_probe.buf && entry->mbss.tx_probe.len)
			hex_dump_with_cat_and_lvl("MBSS-PRB:", entry->mbss.tx_probe.buf, entry->mbss.tx_probe.len,
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);
		if (entry->mbss.tx_probe_cmm.buf && entry->mbss.tx_probe_cmm.len)
			hex_dump_with_cat_and_lvl("MBSS-PRB-C:", entry->mbss.tx_probe_cmm.buf, entry->mbss.tx_probe_cmm.len,
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);
		if (entry->mld.buf && entry->mld.len)
			hex_dump_with_cat_and_lvl("MLD:", entry->mld.buf, entry->mld.len,
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);
		if (entry->mld_cmm.buf && entry->mld_cmm.len)
			hex_dump_with_cat_and_lvl("MLD-C:", entry->mld_cmm.buf, entry->mld_cmm.len,
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);
		if (entry->tid2lnk.buf && entry->tid2lnk.len)
			hex_dump_with_cat_and_lvl("TID2LNK:", entry->tid2lnk.buf, entry->tid2lnk.len,
				DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG);
	}

#ifdef DOT11_EHT_BE
	/* MLD STA */
	MTWF_PRINT("\n\n");
	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		struct bmgr_mld_sta *mld_sta = GET_MLD_STA_BY_IDX(i);
		struct bmgr_mld_link *mld_link;

		if (mld_sta->valid) {
			int j;

			MTWF_PRINT("MLD_STA(%d):\n", i);
			MTWF_PRINT("\tAssocToBSS[%02d] in MLD[%02d]: MldAddr(%pM), AID(%d)\n",
				mld_sta->bss_idx_setup,
				mld_sta->mld_grp_idx,
				mld_sta->mld_addr, mld_sta->aid);
			MTWF_PRINT("\t\t\tEMLCaps(0x%x), EMLSR_EN(%d), EMLSR_BMAP(0x%x)\n",
				mld_sta->eml_caps, mld_sta->emlsr_en, mld_sta->emlsr_link_bmap);
			for (j = 0; j < BSS_MNGR_MAX_BAND_NUM; j++) {
				mld_link = &mld_sta->mld_link[j];
				if (mld_link->active || mld_link->requested) {
					MTWF_PRINT(
						"\t\tL(%d)%s: Addr(%pM), STRBmap(0x%x), priv(%p)\n",
						j, mld_link->active ? "" : "(X)",
						mld_link->link_addr, mld_link->str_bmap,
						mld_link->priv_ptr);
				}
			}
		}
	}
#endif

	return NDIS_STATUS_SUCCESS;
}

#ifdef DOT11V_MBSSID_SUPPORT
int bss_mngr_con_query_mbss_ie(
	IN PNET_DEV pNetDev,
	struct mbss_query_info *mbss_info
)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct mbss_info *mbss = &entry->mbss;
	bool is_non_tx_ml_probe = FALSE;
#ifdef DOT11_EHT_BE
	struct ml_ie_info *ml_info = NULL;

	if (mbss_info->ml_ie) {
		os_alloc_mem(NULL, (UCHAR **)&ml_info, sizeof(struct ml_ie_info));
		if (!ml_info) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"failed to allocate memory for parsing ML IE\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}
		NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
		if (parse_multi_link_ie(mbss_info->ml_ie, NULL, ml_info) < 0) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"failed to parse ML IE\n");
			ret = NDIS_STATUS_FAILURE;
			goto end;
		}

		if ((ml_info->type == ML_CTRL_TYPE_PROBE_REQ)
			&& (ml_info->pres_bmap & DOT11BE_ML_PROBE_REQ_PRESENCE_MLD_ID)
			&& (ml_info->mld_id != 0)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"ML Probe Req to non-tx BSS w/ MLD_ID %d\n", ml_info->mld_id);
			is_non_tx_ml_probe = TRUE;
		}
	}
#endif

	if (mbss_info->is_probe_rsp) {
		/* mbssie for probe rsp */
		u8 non_tx_entry_idx;
		u8 mbssid_idx;
		u16 mbssie_offset;
		u16 mbssie_len;
		/* ML probe request to a non-tx bss, which affiliated with a mld,
		 * the related mbssid ie can not contain ml ie */
		if (is_non_tx_ml_probe) {
			if (entry && entry->mbss.tx_probe.buf) {
				/* mbss ie w/o ml ie */
				u8 mbssie_num = mbss->tx_probe_mbssie.mbssie_num;

				for (non_tx_entry_idx = 0; non_tx_entry_idx < mbssie_num; non_tx_entry_idx++) {
					mbssid_idx = mbss->tx_probe_mbssie.mbssid_idx[non_tx_entry_idx];
					mbssie_offset = mbss->tx_probe_mbssie.mbssie_offset[non_tx_entry_idx];
					mbssie_len = mbss->tx_probe_mbssie.mbssie_len[non_tx_entry_idx];
					if (mbss_info->query_bitmap & BIT(mbssid_idx)) {
						NdisMoveMemory(mbss_info->f_buf + mbss_info->f_len,
							mbss->tx_probe.buf + mbssie_offset, mbssie_len);
						mbss_info->f_len += mbssie_len;
					}
				}
				ret = NDIS_STATUS_SUCCESS;
			}
		} else {
			if (entry && entry->mbss.tx_probe_cmm.buf) {
				/* mbss ie w/ ml ie common part */
				u8 mbssie_num = mbss->tx_probe_cmm_mbssie.mbssie_num;

				for (non_tx_entry_idx = 0; non_tx_entry_idx < mbssie_num; non_tx_entry_idx++) {
					mbssid_idx = mbss->tx_probe_cmm_mbssie.mbssid_idx[non_tx_entry_idx];
					mbssie_offset = mbss->tx_probe_cmm_mbssie.mbssie_offset[non_tx_entry_idx];
					mbssie_len = mbss->tx_probe_cmm_mbssie.mbssie_len[non_tx_entry_idx];
					if (mbss_info->query_bitmap & BIT(mbssid_idx)) {
						NdisMoveMemory(mbss_info->f_buf + mbss_info->f_len,
							mbss->tx_probe_cmm.buf + mbssie_offset, mbssie_len);
						mbss_info->f_len += mbssie_len;
					}
				}
				ret = NDIS_STATUS_SUCCESS;
			}
		}
	} else {
		if (entry && mbss->tx_bcn.buf) {
			/* mbssie for beacon */
			NdisMoveMemory(mbss_info->f_buf, mbss->tx_bcn.buf, mbss->tx_bcn.len);
			mbss_info->f_len = mbss->tx_bcn.len;
			/* tim ie offset */
			NdisMoveMemory(mbss_info->tim_ie_offset, mbss->bcn_tim_offset,
				sizeof(mbss->bcn_tim_offset));
#ifdef DOT11_EHT_BE
			/* mlt ie offset */
			NdisMoveMemory(mbss_info->mlt_ie_offset, mbss->bcn_mlt_offset,
				sizeof(mbss->bcn_mlt_offset));
#endif /* DOT11_EHT_BE */
			/* cap info offset */
			NdisMoveMemory(mbss_info->cap_info_offset, mbss->bcn_cap_info_offset,
				sizeof(mbss->bcn_cap_info_offset));
			ret = NDIS_STATUS_SUCCESS;
		}
	}

#ifdef DOT11_EHT_BE
end:
	if (ml_info)
		os_free_mem(ml_info);
#endif /* DOT11_EHT_BE */

	return ret;
}
#endif

int bss_mngr_con_query_rnr_ie(
	IN PNET_DEV pNetDev,
	OUT u8 *f_buf,
	OUT u16 *f_len
)
{
	int ret = NDIS_STATUS_FAILURE;
	u16 ie_len = 0;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct rnr_nap_info *rnr_nap = NULL;
	struct _EID_STRUCT *eid = NULL;
	u8 *pos = f_buf;
	u8 band_idx, tbtt_idx;
	u8 need_rnr_h, need_nap_h;
	u16 all_rnr_len = 0;
	u8 cur_nap_h_tbtt_cnt = 0;
	struct neighbor_ap_info *cur_nap_h = NULL;

	if (entry) {
		if ((entry->rnr.len || entry->rnr_info.repting_rnr.len) &&
			entry->entry_info.single_rnr) {

			for (band_idx = 0; band_idx < MAX_BAND_NUM; band_idx++) {
				rnr_nap = &entry->rnr_nap[band_idx];

				if (!rnr_nap->tbtt_cnt)
					continue;

				/*switch band, add new nap*/
				need_nap_h = TRUE;

				for (tbtt_idx = 0; tbtt_idx < 16; tbtt_idx++) {

					if (!IS_TBTT_BITMAP_SET(rnr_nap->tbtt_bitmap, tbtt_idx))
						continue;

					/*start a new frame or rnr ie len is too big*/
					if (!ie_len ||
						(ie_len + sizeof(struct neighbor_ap_info) + rnr_nap->tbtt[tbtt_idx].len > 255)) {
						need_rnr_h = TRUE;
						need_nap_h = TRUE;

						/*finish last rnr ie, and cnt the new*/
						if (ie_len && eid) {
							eid->Len = ie_len;
							all_rnr_len += (ie_len + 2);
							ie_len = 0;
						}
					}

					/*add rnr header*/
					if (need_rnr_h) {
						eid = (struct _EID_STRUCT *)pos;
						eid->Eid = EID_REDUCED_NEIGHBOR_REPORT;
						pos = &eid->Octet[0];
						need_rnr_h = FALSE;
					}

					/*case 1: different nap header format in the same band*/
					/*case 2: different nap header format among different band*/
					if (cur_nap_h && memcmp(cur_nap_h, &rnr_nap->head[tbtt_idx], sizeof(struct neighbor_ap_info)))
						need_nap_h = TRUE;

					/*add tbtt info*/
					if (need_nap_h) {
						NdisMoveMemory(pos, (u8 *)&rnr_nap->head[tbtt_idx], sizeof(struct neighbor_ap_info));

						/*sync nap_h tbtt cnt */
						if (cur_nap_h) {
							SET_DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT(cur_nap_h->tbtt_info_hdr,
								(cur_nap_h_tbtt_cnt - 1));
							cur_nap_h_tbtt_cnt = 0;
						}
						cur_nap_h = (struct neighbor_ap_info *)pos;

						pos += sizeof(struct neighbor_ap_info);
						ie_len += sizeof(struct neighbor_ap_info);
						need_nap_h = FALSE;
					}

					/*add tbtt*/
					NdisMoveMemory(pos, rnr_nap->tbtt[tbtt_idx].buf, rnr_nap->tbtt[tbtt_idx].len);
					pos += rnr_nap->tbtt[tbtt_idx].len;
					ie_len += rnr_nap->tbtt[tbtt_idx].len;
					cur_nap_h_tbtt_cnt++;
				}
			}

			/*sync last nap_h tbtt cnt */
			if (cur_nap_h && cur_nap_h_tbtt_cnt)
				SET_DOT11_RNR_TBTT_INFO_HDR_TBTTINFO_CNT(cur_nap_h->tbtt_info_hdr,
					(cur_nap_h_tbtt_cnt - 1));

			if (eid && ie_len) {
				eid->Len = ie_len;
				all_rnr_len += (ie_len + 2);
				*f_len = all_rnr_len;
			} else
				*f_len = 0;
		} else {
			/* RNR from MLD */
			if (entry->rnr.buf) {
				NdisMoveMemory(f_buf, entry->rnr.buf, entry->rnr.len);
				ie_len += entry->rnr.len;
				f_buf += entry->rnr.len;
			}

			/* RNR from OOB 6G */
			if (entry->rnr_info.repting_rnr.buf) {
				struct ie *repting_rnr = &entry->rnr_info.repting_rnr;

				NdisMoveMemory(f_buf, repting_rnr->buf, repting_rnr->len);
				ie_len += repting_rnr->len;
			}

			*f_len = ie_len;
		}

		ret = NDIS_STATUS_SUCCESS;
	}

	return ret;
}

bool bss_mngr_con_query_is_repted_ssid(
	IN PNET_DEV pNetDev,
	IN u32 short_ssid,
	IN u8 *ssid,
	IN u8 ssid_len)
{
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *repted_entry = NULL;
	struct bmgr_reg_info *reg_info = NULL;
	struct repting_rnr_info *rept = NULL;
	bool ret = FALSE;
	u8 i, repted_bss_idx;
	u32 repted_short_ssid;

	if (!BMGR_VALID_BSS_ENTRY(entry))
		return FALSE;

	rept = &entry->rnr_info;

	if (rept->repting_cnt) {
		for (i = 0; i < rept->repting_cnt; i++) {
			repted_bss_idx = rept->repting_bss_idx[i];
			repted_entry = GET_BSS_ENTRY_BY_IDX(repted_bss_idx);

			if (!BMGR_VALID_BSS_ENTRY(repted_entry))
				continue;

			reg_info = &repted_entry->entry_info;
			repted_short_ssid = Crcbitbybitfast(reg_info->ssid, reg_info->ssid_len);

			/* check SSID and short SSID */
			if ((short_ssid == repted_short_ssid) ||
				((ssid_len == reg_info->ssid_len) &&
				NdisEqualMemory(ssid, reg_info->ssid, (ULONG)ssid_len))) {
				ret = TRUE;
				break;
			}
		}
	}

	return ret;
}

bool bss_mngr_con_query_is_mld_enabled(IN PNET_DEV pNetDev)
{
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	return BMGR_VALID_MLO_BSS_ENTRY(entry);
}

bool bss_mngr_con_query_is_in_mlo_csa(IN PNET_DEV pNetDev)
{
	u8 i;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *iob_entry;
	struct wifi_dev *wdev = NULL;
	struct DOT11_H *pDot11h = NULL;

	if (!BMGR_VALID_BSS_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: entry is NULL or invalid!\n");
		return FALSE;
	}

	/* skip the AP not affiliated in any AP MLD */
	if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d(%s)] not affiliated in any AP MLD\n",
			entry->bss_idx, entry->pNetDev->name);
		return FALSE;
	}

	/* traverse all iob BSS and check csa bitmap */
	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		iob_entry = GET_BSS_ENTRY_BY_IDX(i);
		if (!BMGR_VALID_MLO_BSS_ENTRY(iob_entry))
			continue;

		wdev = RtmpOsGetNetDevWdev(iob_entry->pNetDev);
		if (wdev && wdev->pDot11_H) {
			pDot11h = wdev->pDot11_H;
			if (pDot11h->csa_ap_bitmap != 0) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
					"hw_bss(%d) in mlo_csa!\n", iob_entry->entry_info.hw_bss_index);
				return TRUE;
			}
		}
	}

	return FALSE;
}

int bss_mngr_con_query_netdev_by_band(
	struct query_band_netdev *query_info
)
{
	u8 i;
	struct bmgr_entry *entry = NULL;

	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		entry = GET_BSS_ENTRY_BY_IDX(i);

		if (!BMGR_VALID_BSS_ENTRY(entry))
			continue;

		if (query_info->band == BAND_24G) {
			/* 2.4G */
			if (!WMODE_CAP_6G(entry->entry_info.phy_mode) && (entry->entry_info.channel <= 14)) {
				query_info->pNetDev = entry->pNetDev;
				return NDIS_STATUS_SUCCESS;
			}
		} else if (query_info->band == BAND_5G) {
			/* 5G */
			if (!WMODE_CAP_6G(entry->entry_info.phy_mode) && (entry->entry_info.channel > 14)) {
				query_info->pNetDev = entry->pNetDev;
				return NDIS_STATUS_SUCCESS;
			}
		} else {
			/* 6G */
			if (WMODE_CAP_6G(entry->entry_info.phy_mode)) {
				query_info->pNetDev = entry->pNetDev;
				return NDIS_STATUS_SUCCESS;
			}
		}
	}

	query_info->pNetDev = NULL;
	return NDIS_STATUS_FAILURE;
}

#ifdef DOT11_EHT_BE
int bss_mngr_con_mld_query_netdev_by_link(
	struct query_group_link_netdev *query_info
)
{
	u8 bss_idx;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;

	if (!BMGR_VALID_MLD_GRP_IDX(query_info->group_id))
		return NDIS_STATUS_FAILURE;

	mld = GET_MLD_BY_GRP_IDX(query_info->group_id);
	if (!BMGR_VALID_MLO_DEV(mld))
		return NDIS_STATUS_FAILURE;

	bss_idx = mld->bss_idx_mld[query_info->link_id];

	if (!BMGR_VALID_BSS_IDX(bss_idx))
		return NDIS_STATUS_FAILURE;

	entry = GET_BSS_ENTRY_BY_IDX(bss_idx);

	if (entry->pNetDev == NULL)
		return NDIS_STATUS_FAILURE;

	query_info->pNetDev = entry->pNetDev;

	return NDIS_STATUS_SUCCESS;
}
#endif

static struct bmgr_entry *bss_mngr_con_bss_lookup(
	u8 *link_addr
)
{
	u8 i;
	struct bmgr_entry *entry;

	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		entry = GET_BSS_ENTRY_BY_IDX(i);
		if (BMGR_VALID_BSS_ENTRY(entry) && MAC_ADDR_EQUAL(entry->entry_info.bssid, link_addr)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"BSS Found: %s bss_idx(%d)\n",
				RtmpOsGetNetDevName(entry->pNetDev), entry->bss_idx);
			return entry;
		}
	}
	return NULL;
}

#ifdef DOT11_EHT_BE
static struct bmgr_mlo_dev *bss_mngr_con_mld_lookup(
	u8 *mld_addr
)
{
	u8 i;
	struct bmgr_mlo_dev *mld;

	for (i = 0; i < BMGR_MAX_MLD_GRP_CNT; i++) {
		mld = GET_MLD_BY_GRP_IDX(i);
		if (BMGR_VALID_MLO_DEV(mld) && MAC_ADDR_EQUAL(mld->mld_addr, mld_addr)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"MLD Found: grp(%d) (%pM)\n", mld->mld_grp, mld->mld_addr);
			return mld;
		}
	}
	return NULL;
}

static int bss_mngr_con_mld_conn_query(
	INOUT struct query_mld_conn *mld_conn
)
{
	u8 mld_grp_idx = mld_conn->mld_group_idx;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_sta *mld_sta;
	struct query_mld_sta *mld_sta_info;
	int i;

	if (BMGR_VALID_MLD_GRP_IDX(mld_grp_idx))
		mld = GET_MLD_BY_GRP_IDX(mld_grp_idx);

	if (!BMGR_VALID_MLO_DEV(mld))
		return NDIS_STATUS_INVALID_DATA;

	mld_conn->mld_sta_num = 0;
	mld_sta_info = (struct query_mld_sta *)mld_conn->mld_sta_info;
	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		mld_sta = GET_MLD_STA_BY_IDX(i);

		if (mld_sta->valid && (mld_sta->mld_grp_idx == mld_grp_idx)) {
			mld_sta_info->mld_sta_idx = mld_sta->idx;
			mld_sta_info->aid = mld_sta->aid;
			COPY_MAC_ADDR(mld_sta_info->mld_addr, mld_sta->mld_addr);

			mld_conn->mld_sta_num++;
			mld_sta_info++;
		}
	}

	return NDIS_STATUS_SUCCESS;
}

static u8 get_bss_idx_by_requested_dot11be_mld_id(
	IN struct bmgr_entry *entry,
	IN u8 dot11be_mld_id
)
{
	u8 bss_idx;
	struct bmgr_entry *requested_entry;

	for (bss_idx = 0; bss_idx < BSS_MNGR_ENTRY_CNT; bss_idx++) {
		requested_entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
		if (BMGR_VALID_MLO_BSS_ENTRY(requested_entry) &&
			(requested_entry->link_id == entry->link_id) &&
			(requested_entry->bss_idx_tx == entry->bss_idx_tx) &&
			(requested_entry->dot11be_mld_id == dot11be_mld_id))
			return bss_idx;
	}

	/* not found mld_id */
	return BMGR_INVALID_BSS_IDX;
}

int bss_mngr_con_query_mld_info(
	IN PNET_DEV pNetDev,
	OUT struct query_mld_info *mld_query
)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bmgr_entry *entry;
	u8 query_type = mld_query->query_type, bss_idx;
	struct bmgr_mlo_dev *mld = NULL;
	struct ml_ie_info *ml_info = NULL;
	struct bmgr_mld_sta *bmgr_sta;
	u16 mld_sta_idx, ie_len = 0;
	u8 *f_buf = NULL, *pos = NULL;
	struct _EID_STRUCT *eid = NULL;
	bool is_ml_probe = FALSE, is_probe_mld_id = FALSE;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG, "query_type=%d\n", query_type);

	switch (query_type) {
	case BMGR_QUERY_MLD:
		mld = bss_mngr_con_mld_lookup(mld_query->mld_basic->addr);
		if (mld) {
			mld_query->mld_basic->mld_grp_idx = mld->mld_grp;
			mld_query->mld_basic->mld_type = mld->mld_type;
			mld_query->mld_basic->link_cnt = mld->mld_link_cnt;
			ret = NDIS_STATUS_SUCCESS;
		}
		return ret;
	case BMGR_QUERY_BSSID_ML_INFO:
		if (MAC_ADDR_EQUAL(mld_query->basic->bssid, ZERO_MAC_ADDR))
			goto end;

		entry = bss_mngr_con_bss_lookup(mld_query->basic->bssid);
		if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
			mld = entry->mld_ptr;
			if (BMGR_VALID_MLO_DEV(mld)) {
				mld_query->basic->link_cnt = mld->mld_link_cnt;
				mld_query->basic->link_id = entry->link_id;
				COPY_MAC_ADDR(mld_query->basic->addr, mld->mld_addr);
				ret = NDIS_STATUS_SUCCESS;
			}
		}
		return ret;
	case BMGR_QUERY_MLD_CONN:
		ret = bss_mngr_con_mld_conn_query(mld_query->mld_conn);
		return ret;
	default:
		break;
	}

	entry = get_bss_entry_by_netdev(pNetDev);
	if (entry == NULL)
		goto end;

	switch (query_type) {
	case BMGR_QUERY_ML_AP_BASIC:
		mld = entry->mld_ptr;
		if (BMGR_VALID_MLO_DEV(mld)) {
			mld_query->basic->mld_grp_idx = mld->mld_grp;
			mld_query->basic->link_cnt = mld->mld_link_cnt;
			mld_query->basic->link_id = entry->link_id;
			COPY_MAC_ADDR(mld_query->basic->addr, mld->mld_addr);
			bss_idx = mld->bmc_bss_idx;
			if (BMGR_VALID_BSS_IDX(bss_idx)) {
				entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
				if (entry)
					mld_query->basic->agent_bss_wdev = entry->pwdev;
			}
			ret = NDIS_STATUS_SUCCESS;
		}
		break;

	case BMGR_QUERY_ML_IE_BCN:			/* mld common info */
		mld_query->ie.f_len = 0;
		if (entry && entry->mld_cmm.buf) {
			NdisMoveMemory(mld_query->ie.f_buf, entry->mld_cmm.buf, entry->mld_cmm.len);
			mld_query->ie.f_len = entry->mld_cmm.len;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"\tMLIE-C: len=%d, f_buf=%p\n", mld_query->ie.f_len, mld_query->ie.f_buf);
		}

		eid = (struct _EID_STRUCT *)mld_query->ie.f_buf;

		f_buf = pos = mld_query->ie.f_buf + mld_query->ie.f_len;
		pos = build_multi_link_appl_elem_per_sta_pf(entry, query_type, f_buf);
		if (entry->csa.csa_offset_in_per_sta_pf[entry->link_id] != 0) {
			mld_query->ie.csa_ie.csa_offset_in_ml_ie =
				entry->csa.csa_offset_in_per_sta_pf[entry->link_id] + mld_query->ie.f_len;
			mld_query->ie.csa_ie.csa_bss_idx = entry->csa.csa_bss_idx;
		}

		if (mld_query->ie.csa_ie.csa_offset_in_ml_ie != 0)
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"\tlink_id=%d, csa_offset_in_per_sta=%d, csa_offset_in_ml=%d\n",
				entry->link_id, entry->csa.csa_offset_in_per_sta_pf[entry->link_id],
				mld_query->ie.csa_ie.csa_offset_in_ml_ie);

		ie_len = pos - f_buf;
		mld_query->ie.f_len += ie_len;

		if (mld_query->ie.f_len > (2 + MAX_LEN_OF_IE_DATA)) {
			/* do IE fragmentation */
			struct bss_mngr_event_ops *event_ops = NULL;

			event_ops = entry->entry_info.event_ops;
			if (event_ops)
				event_ops->bmgr_event_ie_fragment((u8 *)eid, &mld_query->ie.f_len);
			else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Error: event_ops is not inited, can't frag the ML IE!");
		} else {
			/* length after append Per-STA Profile */
			eid->Len += ie_len;
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
				"\tMLIE: len=%d, f_buf=%p\n", mld_query->ie.f_len, mld_query->ie.f_buf);

		ret = NDIS_STATUS_SUCCESS;

	break;

	case BMGR_QUERY_ML_IE_AUTH_RSP:
		mld_query->ie.f_len = 0;
		if (bss_idx_mld_inited(entry)) {
			f_buf = pos = mld_query->ie.f_buf;
			pos = build_multi_link_ie(entry, f_buf,
				BMGR_MLD_AUTH_COMMON, BMGR_MAX_MLD_STA_CNT);
			ie_len = pos - f_buf;
			mld_query->ie.f_len = ie_len;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"\tMLIE-C: len=%d, f_buf=%p\n", mld_query->ie.f_len, mld_query->ie.f_buf);
			ret = NDIS_STATUS_SUCCESS;
		}
		break;

	case BMGR_QUERY_ML_IE_PROBE_RSP:	/* TBD: parse requested info */
		mld_query->ie.f_len = 0;
		if (!bss_idx_mld_inited(entry))
			break;

		if (mld_query->ie.ml_ie) {
			os_alloc_mem(NULL, (UCHAR **)&ml_info, sizeof(struct ml_ie_info));
			if (!ml_info) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"failed to allocate memory for parsing ML IE\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
			if (parse_multi_link_ie(mld_query->ie.ml_ie, mld_query->ie.ml_frag_info, ml_info) < 0) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"failed to parse ML IE\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			if (ml_info->type == ML_CTRL_TYPE_PROBE_REQ) {
				is_ml_probe = TRUE;

				if ((ml_info->pres_bmap & DOT11BE_ML_PROBE_REQ_PRESENCE_MLD_ID)
					&& (ml_info->mld_id != 0)) {

					bss_idx = get_bss_idx_by_requested_dot11be_mld_id(entry, ml_info->mld_id);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
						"ML Probe Req to non-tx BSS %d w/ MLD_ID %d\n",
						bss_idx, ml_info->mld_id);

					if (!BMGR_VALID_BSS_IDX(bss_idx)) {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
							"\tUnknown mld_id: %u\n", ml_info->mld_id);
						// break;
					} else {
						entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
						is_probe_mld_id = TRUE;
					}
				}
			}
		}
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"\tis_ml_probe=%d, is_probe_mld_id=%d\n",
				is_ml_probe, is_probe_mld_id);

		eid = (struct _EID_STRUCT *)mld_query->ie.f_buf;
		NdisMoveMemory(mld_query->ie.f_buf, entry->mld_cmm.buf, entry->mld_cmm.len);
		mld_query->ie.f_len = entry->mld_cmm.len;

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
				"\tMLIE: len=%d, f_buf=%p\n", mld_query->ie.f_len, mld_query->ie.f_buf);

		f_buf = pos = mld_query->ie.f_buf + mld_query->ie.f_len;
		if (is_ml_probe)
			pos = build_multi_link_probe_rsp_complete_per_sta_pf(entry, ml_info, f_buf);
		else
			pos = build_multi_link_appl_elem_per_sta_pf(entry, query_type, f_buf);

		ie_len = pos - f_buf;
		mld_query->ie.f_len += ie_len;

		if (mld_query->ie.f_len > (2 + MAX_LEN_OF_IE_DATA)) {
			/* do IE fragmentation */
			struct bss_mngr_event_ops *event_ops = NULL;

			event_ops = entry->entry_info.event_ops;
			if (event_ops)
				event_ops->bmgr_event_ie_fragment((u8 *)eid, &mld_query->ie.f_len);
			else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Error: event_ops is not inited, can't frag the ML IE!");
		} else {
			/* length after append Per-STA Profile */
			eid->Len += ie_len;
		}

		/* append another ML IE of MLD which transmitted BSSID is affiliated with
		 * when ML Probe Request with MLD_ID subfield is targeted to non-tx BSSID
		 */
		if (is_probe_mld_id) {
			struct bmgr_entry *tx_entry;

			bss_idx = entry->bss_idx_tx;

			if (!BMGR_VALID_BSS_IDX(bss_idx)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"\tInvalid bss_idx_tx: %u\n", bss_idx);
			} else {
				tx_entry = GET_BSS_ENTRY_BY_IDX(bss_idx);

				pos = mld_query->ie.f_buf + mld_query->ie.f_len;
				NdisMoveMemory(pos, tx_entry->mld_cmm.buf, tx_entry->mld_cmm.len);
				mld_query->ie.f_len += tx_entry->mld_cmm.len;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
					"Append second ML IE (bss_idx=%d, len=%d)\n",
					bss_idx, tx_entry->mld_cmm.len);
			}
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_DEBUG,
				"\tMLIE: len=%d, f_buf=%p\n", mld_query->ie.f_len, mld_query->ie.f_buf);

		ret = NDIS_STATUS_SUCCESS;
		break;

	case BMGR_QUERY_ML_IE_ASSOC_RSP:	/* TBD: parse requested info */
		mld_query->ie.f_len = 0;
		/* the STA Profiles for peer are buffered in bmgr_sta[mld_sta_idx] */
		mld_sta_idx = mld_query->ie.mld_sta_idx;
		if (bss_idx_mld_inited(entry)) {
			f_buf = pos = mld_query->ie.f_buf;
			pos = build_multi_link_ie(entry, f_buf,
				BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE, mld_sta_idx);
			ie_len = pos - f_buf;
			mld_query->ie.f_len = ie_len;

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
					"\tMLIE: len=%d, f_buf=%p\n", mld_query->ie.f_len, mld_query->ie.f_buf);
			ret = NDIS_STATUS_SUCCESS;
		}
		break;

	case BMGR_QUERY_ML_STA:
		mld_sta_idx = mld_query->sta->mld_sta_idx;
		if (BMGR_VALID_MLD_STA(mld_sta_idx)) {
			bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
			if (bmgr_sta && bmgr_sta->valid) {
				mld_query->sta->aid = bmgr_sta->aid;
				COPY_MAC_ADDR(mld_query->sta->mld_addr, bmgr_sta->mld_addr);
				ret = NDIS_STATUS_SUCCESS;
			}
		}
		break;

	case BMGR_QUERY_ML_PARSING_IE:
		if (mld_query->parse_ie.ml_ie && mld_query->parse_ie.ml_info) {
			if (parse_multi_link_ie(
				mld_query->parse_ie.ml_ie,
				mld_query->parse_ie.ml_frag_info,
				mld_query->parse_ie.ml_info) < 0) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"failed to parse ML IE\n");
					ret = NDIS_STATUS_FAILURE;
					goto end;
			}
			ret = NDIS_STATUS_SUCCESS;
		}
		break;

	default:
		ret = NDIS_STATUS_INVALID_DATA;
	}

end:
	if (ml_info)
		os_free_mem(ml_info);

	return ret;
}

int bss_mngr_con_query_tid_to_link_map_ie(
	IN PNET_DEV pNetDev,
	OUT u8 *f_buf,
	OUT u16 *f_len
)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	if (entry && entry->tid2lnk.buf) {
		NdisMoveMemory(f_buf, entry->tid2lnk.buf, entry->tid2lnk.len);
		*f_len = entry->tid2lnk.len;
		ret = NDIS_STATUS_SUCCESS;
	}

	return ret;
}

int bss_mngr_con_query_multi_link_traffic_ie(
	IN PNET_DEV pNetDev,
	OUT u8 *f_buf,
	OUT u16 *f_len
)
{
	int ret = NDIS_STATUS_FAILURE;

	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	if (entry) {
		*f_len = 0;
		ret = NDIS_STATUS_SUCCESS;
	}

	return ret;
}


int bss_mngr_con_add_mld_sta(
	IN struct bmgr_entry	*entry,
	IN struct bmgr_mld_sta	*bmgr_sta
)
{
	int ret = NDIS_STATUS_SUCCESS;
	u8 link, mld_bss_idx;
	struct bmgr_entry *mld_entry = NULL;
	struct bmgr_mlo_dev *mld = entry->mld_ptr;
	struct bss_mngr_event_ops *event_ops;
	struct mld_sta_add add_sta;
	struct mld_link_add	*add_link;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, bss_idx (%d) in mld (%d) is invalid\n",
			entry->bss_idx, entry->entry_info.mld_grp);
		return NDIS_STATUS_FAILURE;
	}

	os_zero_mem(&add_sta, sizeof(add_sta));

	if (bmgr_sta && bmgr_sta->valid) {
		/* common info */
		add_sta.mld_sta_idx = bmgr_sta->idx;
		add_sta.aid = bmgr_sta->aid;
		COPY_MAC_ADDR(add_sta.mld_addr, bmgr_sta->mld_addr);
		add_sta.eml_caps = bmgr_sta->eml_caps;

		/* build link info */
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			mld_bss_idx = mld->bss_idx_mld[link];
			if (BMGR_VALID_BSS_IDX(mld_bss_idx))
				mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
			add_link = &add_sta.add_link[link];

			add_link->active = bmgr_sta->mld_link[link].active;
			if (add_link->active && mld_entry) {
				add_link->is_setup_link =
					(bmgr_sta->bss_idx_setup == mld_entry->bss_idx) ? TRUE : FALSE;
				COPY_MAC_ADDR(add_link->sta_addr, bmgr_sta->mld_link[link].link_addr);
				add_link->tid_map	= mld_entry->tid_map;
				add_link->str_map	= bmgr_sta->mld_link[link].str_bmap;
				add_link->cap		= 0xff;	/* TBD */
				add_link->priv_ptr	= NULL;

				if (add_link->active)
					add_sta.total_link_cnt++;
			}
			mld_entry = NULL;
		}

		/*setuplink should be the first insert link as primary link in hwifi*/
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (bmgr_sta->mld_link[link].active &&  add_sta.add_link[link].is_setup_link) {
				add_sta.link_id = link;
				mld_bss_idx = mld->bss_idx_mld[link];
				/* setup link shall success */
				if (!BMGR_VALID_BSS_IDX(mld_bss_idx)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"insert setup link failed, bss_idx(%d) link_id(%d)\n",
						mld_bss_idx, link);
					bmgr_sta->mld_link[link].active = FALSE;
					ret = NDIS_STATUS_FAILURE;
					goto err;
				}
				mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"[%d],first insert link_id(%d)\n",
						 mld_entry->bss_idx, add_sta.link_id);

				event_ops = mld_entry->entry_info.event_ops;
				if (event_ops) {
					ret = event_ops->bmgr_event_mld_add_sta(mld_entry->pNetDev, &add_sta);
					if (ret == NDIS_STATUS_SUCCESS) {
						/* entry allocated success */
						bmgr_sta->mld_link[link].priv_ptr = add_sta.add_link[link].priv_ptr;
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
								"\tlink_id(%d), pEntry(%p)\n",
								 link, add_sta.add_link[link].priv_ptr);
					} else {
						/* entry allocated fail */
						bmgr_sta->mld_link[link].active = FALSE;
						/* bmgr_sta->mld_link[link].requested still TRUE */
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
							"mld_add_sta setup L%d fail, link_addr(%pM)\n",
							 link, bmgr_sta->mld_link[link].link_addr);
						/* setup link shall success */
						ret = NDIS_STATUS_FAILURE;
						goto err;
					}
				}
			}
		}

		/* send add_sta event to active link */
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (bmgr_sta->mld_link[link].active) {
				if (add_sta.add_link[link].is_setup_link)
					continue;

				add_sta.link_id = link;
				mld_bss_idx = mld->bss_idx_mld[link];
				if (!BMGR_VALID_BSS_IDX(mld_bss_idx)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"insert link failed, bss_idx(%d) link_id(%d)\n",
						mld_bss_idx, link);
					bmgr_sta->mld_link[link].active = FALSE;
					continue;
				}
				mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);

				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"[%d], link_id(%d)\n",
						 mld_entry->bss_idx, add_sta.link_id);

				event_ops = mld_entry->entry_info.event_ops;
				if (event_ops) {
					ret = event_ops->bmgr_event_mld_add_sta(mld_entry->pNetDev, &add_sta);
					if (ret == NDIS_STATUS_SUCCESS) {
						/* entry allocated success */
						bmgr_sta->mld_link[link].priv_ptr = add_sta.add_link[link].priv_ptr;
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
								"\tlink_id(%d), pEntry(%p)\n",
								 link, add_sta.add_link[link].priv_ptr);
					} else {
						/* entry allocated fail */
						bmgr_sta->mld_link[link].active = FALSE;
						/* bmgr_sta->mld_link[link].requested still TRUE */
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
							"mld_add_sta L%d fail, link_addr(%pM)\n",
							 link, bmgr_sta->mld_link[link].link_addr);
					}
				}
			}
		}

		/* if setup link success, add_mld_sta operation success */
		ret = NDIS_STATUS_SUCCESS;

		at2lm_update_peer_mld_t2lm_at_assoc(mld, bmgr_sta);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"invalid mld_sta %pM\n", bmgr_sta ? bmgr_sta->mld_addr : ZERO_MAC_ADDR);
		ret = NDIS_STATUS_FAILURE;
	}

err:
	return ret;
}

/* sanity check: connect request (STA) and ML BSS (AP) */
static int mld_conn_sanity(
	IN struct bmgr_entry *entry,
	IN struct ml_ie_info *ml_info,
	OUT struct bmgr_mld_sta *bmgr_sta
)
{
	u8 i;
	struct bmgr_entry *mld_entry = NULL;
	struct bmgr_mlo_dev *mld = entry->mld_ptr;
	u8 mld_bss_idx;

	if ((entry->entry_info.mld_grp != 0) && entry->mlo_en && BMGR_VALID_MLO_DEV(mld)) {
		/* apply mld basic info */
		bmgr_sta->valid = TRUE;
		bmgr_sta->bss_idx_setup = entry->bss_idx;
		bmgr_sta->mld_grp_idx = mld->mld_grp;
		COPY_MAC_ADDR(bmgr_sta->mld_addr, ml_info->mld_addr);

		/* apply mld per-link info */
		for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
			if (ml_info->link[i].active) {
				mld_bss_idx = BMGR_VALID_MLO_DEV(mld) ? mld->bss_idx_mld[i] : BMGR_INVALID_BSS_IDX;

				if (BMGR_VALID_BSS_IDX(mld_bss_idx)) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							"Link(%d)'s BSS [%d]\n", i, mld_bss_idx);
					mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
					bmgr_sta->mld_link[i].active = TRUE;
					bmgr_sta->mld_link[i].requested = TRUE;
					COPY_MAC_ADDR(bmgr_sta->mld_link[i].link_addr, ml_info->link[i].link_addr);
				} else {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							"MLD [%d] not support Link(%d)\n", mld->mld_grp, i);
				}
			}
			mld_entry = NULL;
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
				"[%d] is Non-MLD\n", entry->bss_idx);
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

/* Assign peer STR bitmap according to NSTR bitmap in Per-STA Profile (non-setup links) */
static int mld_assign_peer_str_bitmap(
	IN struct bmgr_entry *entry,
	IN struct ml_ie_info *ml_info,
	OUT struct bmgr_mld_sta *bmgr_sta
)
{
	u8 sta_link, link;

	for (sta_link = 0; sta_link < BSS_MNGR_MAX_BAND_NUM; sta_link++) {
		/* set STR by default */
		bmgr_sta->mld_link[sta_link].str_bmap = BITS(0, BSS_MNGR_MAX_BAND_NUM - 1);

		/* there is no Per-STA Profile for setup link */
		if ((sta_link != entry->link_id) && bmgr_sta->mld_link[sta_link].active) {
			for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
				if ((link != sta_link) &&
					(ml_info->link[sta_link].nstr_bmap & BIT(link)) &&
					bmgr_sta->mld_link[link].active) {
					/* reset str bitmap for NSTR pair */
					bmgr_sta->mld_link[sta_link].str_bmap &= ~(BIT(link));
					/* sync to link pair */
					bmgr_sta->mld_link[link].str_bmap &= ~(BIT(sta_link));
				}
			}
		}
	}

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_con_mld_bss_linkup(
	IN PNET_DEV pNetDev
)
{
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mlo_dev = NULL;

	if (!pNetDev)
		return NDIS_STATUS_FAILURE;

	entry = get_bss_entry_by_netdev(pNetDev);
	if (!entry)
		return NDIS_STATUS_FAILURE;

	if (!BMGR_VALID_BSS_ENTRY(entry))
		return NDIS_STATUS_FAILURE;

	mlo_dev = entry->mld_ptr;

	if (BMGR_VALID_MLO_BSS_ENTRY(entry) && BMGR_VALID_MLO_DEV(mlo_dev))
		bss_mngr_con_mld_get_tsf_offset(mlo_dev);

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_con_mld_bss_linkdown(
	IN PNET_DEV pNetDev
)
{
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mlo_dev = NULL;

	if (!pNetDev)
		return NDIS_STATUS_FAILURE;

	entry = get_bss_entry_by_netdev(pNetDev);
	if (!entry)
		return NDIS_STATUS_FAILURE;

	if (!BMGR_VALID_BSS_ENTRY(entry))
		return NDIS_STATUS_FAILURE;

	mlo_dev = entry->mld_ptr;
	if (BMGR_VALID_MLO_BSS_ENTRY(entry) && BMGR_VALID_MLO_DEV(mlo_dev))
		bss_mngr_con_mld_get_tsf_offset(mlo_dev);

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_con_mld_bmc_sn_qry(
	IN PNET_DEV pNetDev,
	OUT u8 *mld,
	OUT u8 *agent_bss,
	OUT u16 *sn,
	OUT PNET_DEV * if_dev_list
)
{
	u8 i = 0;
	u8 mld_bss_idx;
	struct bmgr_entry *entry = NULL;
	struct bmgr_mlo_dev *mlo_dev = NULL;

	if (!pNetDev)
		return NDIS_STATUS_FAILURE;

	entry = get_bss_entry_by_netdev(pNetDev);
	if (!entry)
		return NDIS_STATUS_FAILURE;

	if (!BMGR_VALID_BSS_ENTRY(entry))
		return NDIS_STATUS_FAILURE;

	mlo_dev = entry->mld_ptr;

	if (BMGR_VALID_MLO_BSS_ENTRY(entry) && BMGR_VALID_MLO_DEV(mlo_dev)) {
		*mld = 1;
		if (entry->bss_idx == mlo_dev->bmc_bss_idx) {
			*agent_bss = 1;
			*sn = atomic_read(&mlo_dev->bmc_sn);

			for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
				if_dev_list[i] = NULL;
				mld_bss_idx = mlo_dev->bss_idx_mld[i];
				if (!BMGR_VALID_BSS_IDX(mld_bss_idx))
					continue;
				entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
				if (!entry)
					continue;
				if_dev_list[i] = entry->pNetDev;
			}

			atomic_inc(&mlo_dev->bmc_sn);
			if (atomic_read(&mlo_dev->bmc_sn) > 4095)
				atomic_set(&mlo_dev->bmc_sn, 0);
		} else
			*agent_bss = 0;
	} else {
		*mld = 0;
		*agent_bss = 0;
	}

	return NDIS_STATUS_SUCCESS;
}

int bss_mngr_con_mld_conn_req(
	IN PNET_DEV pNetDev,
	struct mld_conn_req *mld_conn
)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct ml_ie_info *ml_info = NULL;
	struct tid2lnk_ie_info t2l_info = {0};
	struct bmgr_mld_sta *bmgr_sta;
	struct bmgr_mlo_dev *mld = NULL;
	u16 i, mld_sta_idx;

	if (entry) {
		if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"[%d] is invalid (vld=%d, ml=%d)\n",
					entry->bss_idx, entry->valid, entry->mlo_en);
			return ret;
		}

		mld = entry->mld_ptr;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"[%d] No valid mld\n", entry->bss_idx);
			return ret;
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[%d] req_type(%d)\n", entry->bss_idx, mld_conn->req_type);

		if (!mld_conn->link_addr) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Error: link_addr is null, [%d] req_type(%d)\n",
				entry->bss_idx, mld_conn->req_type);
			return ret;
		}

		if (mld_conn->t2l_ie) {
			parse_tid_to_link_map_ie(mld_conn->t2l_ie, &t2l_info);
		}

		if (mld_conn->ml_ie) {
			os_alloc_mem(NULL, (UCHAR **)&ml_info, sizeof(struct ml_ie_info));
			if (!ml_info) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Error: failed to allocate memory for parsing ML IE\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}
			NdisZeroMemory(ml_info, sizeof(struct ml_ie_info));
			if (parse_multi_link_ie(mld_conn->ml_ie, mld_conn->ml_frag_info, ml_info) < 0) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Error: failed to parse multi link ie\n");
				ret = NDIS_STATUS_FAILURE;
				goto end;
			}

			/* protect mld sta index acquire */
			mutex_lock(&bss_mngr.op_mutex);

			/* acquire mld sta index */
			COPY_MAC_ADDR(mld_conn->mld_addr, ml_info->mld_addr);
			mld_sta_idx = mld_sta_acquire(ml_info->mld_addr,
				mld_conn->req_type, mld_conn->link_addr);
			if (BMGR_VALID_MLD_STA(mld_sta_idx)) {
				bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
				mld_conn->mld_sta_idx = mld_sta_idx;
				ret = mld_conn_sanity(entry, ml_info, bmgr_sta);
				/* sanity check done */
				if (ret == NDIS_STATUS_SUCCESS) {
					/* 1. link request = assoc frame body(setup link) +
				      per sta profile (other links)
				       2. derive setup link from assoc req frame body */
					bmgr_sta->mld_link[entry->link_id].active = TRUE;
					bmgr_sta->mld_link[entry->link_id].requested = TRUE;
					COPY_MAC_ADDR(bmgr_sta->mld_link[entry->link_id].link_addr,
						mld_conn->link_addr);

					/* aid = mld_sta_idx + 1024 */
					bmgr_sta->aid = mld_sta_idx + START_MLD_STA_AID;
					bmgr_sta->BAOriTID = 0;
					bmgr_sta->BARecTID = 0;
					bmgr_sta->eml_caps = ml_info->eml_caps;
					bmgr_sta->mld_caps = ml_info->mld_caps;
					bmgr_sta->emlsr_sup = ml_info->emlsr_sup;
					mld_assign_peer_str_bitmap(entry, ml_info, bmgr_sta);

					/* log the state of peer */
					for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++)
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_DEBUG,
							"\tlink:%d, active:%d, MAC:%pM\n",
							i, bmgr_sta->mld_link[i].active, bmgr_sta->mld_link[i].link_addr);

					ret = bss_mngr_con_add_mld_sta(entry, bmgr_sta);

					if (ret != NDIS_STATUS_SUCCESS) {
						mld_conn->mld_sta_idx = MLD_STA_NONE;
						mld_sta_release(mld_sta_idx);
						mutex_unlock(&bss_mngr.op_mutex);
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
							"add mld_sta fail\n");
						goto end;
					}
				} else {
					mld_conn->mld_sta_idx = MLD_STA_NONE;
					mld_sta_release(mld_sta_idx);
					mutex_unlock(&bss_mngr.op_mutex);
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"conn sanity fail midx=%d\n", mld_sta_idx);
					goto end;
				}
			} else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"mld_sta acquire fail\n");
			}

			/* end of mld sta acquire */
			mutex_unlock(&bss_mngr.op_mutex);

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
				"idx(%d),mld(%pM),link(%pM)\n",
				mld_sta_idx, ml_info->mld_addr,	mld_conn->link_addr);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "entry is NULL\n");
	}

end:
	if (ml_info)
		os_free_mem(ml_info);

	return ret;
}

int bss_mngr_con_mld_conn_act(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx
)
{
	INT ret = NDIS_STATUS_FAILURE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *mld_entry;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bss_mngr_event_ops *event_ops;
	struct mld_link_add add_link;
	u8 link, mld_bss_idx;

	if (!entry)
		goto end;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (bmgr_sta && bmgr_sta->valid) {
		mld = entry->mld_ptr;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Error, bss_idx (%d) in mld (%d) is invalid\n",
				entry->bss_idx, entry->entry_info.mld_grp);
			goto end;
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
			"bmgr_sta[%d] (%pM) setup bss(%d=%d)\n",
			bmgr_sta->idx, bmgr_sta->mld_addr,
			bmgr_sta->bss_idx_setup, entry->bss_idx);

		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			mld_entry = NULL;
			mld_bss_idx = mld->bss_idx_mld[link];
			if (BMGR_VALID_BSS_IDX(mld_bss_idx))
				mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);

			add_link.active = bmgr_sta->mld_link[link].active;
			if (add_link.active && BMGR_VALID_MLO_BSS_ENTRY(mld_entry)) {
				add_link.priv_ptr = bmgr_sta->mld_link[link].priv_ptr;
				event_ops = bmgr_event_ops_get();
				if (event_ops) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
							"\tpEntry(%p)\n", add_link.priv_ptr);
					ret = event_ops->bmgr_event_mld_conn_act(mld_entry->pNetDev, &add_link);
					if (ret != NDIS_STATUS_SUCCESS) {
						/* mld conn act fail */
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
								"mld link(%d) fail\n", link);
					}
				}
			}
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Invalid mld_sta_idx (%d) or bmgr_sta\n", mld_sta_idx);
	}

end:
	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"mld conn_act failed\n");
	}
	return ret;
}

/**
 * @brief Disconnect all peer MLD associated with the MLD
 *
 * This function disassociates the peer MLD which setup with the correpsonding MLD.
 *
 * @param mld_grp Indicate the MLD that all MLD assoicated with it should be disassociated.
 */
void bss_mngr_con_mld_disconn_all_mld_sta(
	IN u8 mld_grp
)
{
	u16 i, link_id;
	struct bmgr_mld_sta *mld_sta;
	struct bmgr_mld_link *mld_link;
	struct mld_link_del	del_link;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"Disassoc all peer mld associated with ap mld %d\n", mld_grp);

	for (i = 0; BMGR_VALID_MLD_STA(i); i++) {
		mld_sta = GET_MLD_STA_BY_IDX(i);

		if (mld_sta->valid && (mld_sta->mld_grp_idx == mld_grp)) {
			/* disconnect peer MLD on one of active link */
			for (link_id = 0; link_id < BSS_MNGR_MAX_BAND_NUM; link_id++) {
				mld_link = &mld_sta->mld_link[link_id];
				if (!mld_link->active)
					continue;

				/* build delete link info */
				COPY_MAC_ADDR(del_link.sta_addr, mld_link->link_addr);
				del_link.priv_ptr = mld_link->priv_ptr;

				if (event_ops->bmgr_event_mld_disconn_mld_sta(&del_link) != NDIS_STATUS_SUCCESS) {
					/* fail: find next active link */
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"Fail: disconn peer mld %d on link %d failed\n",
						mld_sta->idx, link_id);
					continue;
				}

				break;
			}
		}
	}
}

/* delete a link in a MLD STA */
static int bss_mngr_con_del_mld_sta_link(
	IN struct bmgr_mld_link *sta_mld_link,
	IN u8 op
)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();
	struct mld_link_del	del_link;

	if (sta_mld_link->active || sta_mld_link->priv_ptr) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"addr(%pM), op(%d)\n", sta_mld_link->link_addr, op);

		/* build delete link info */
		COPY_MAC_ADDR(del_link.sta_addr, sta_mld_link->link_addr);
		del_link.priv_ptr = sta_mld_link->priv_ptr;

		if (event_ops) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\tpEntry(%p)\n", del_link.priv_ptr);

			switch (op) {
			case MLD_DISC_OP_ACT_ONLY:
				ret = event_ops->bmgr_event_mld_disconn_act(&del_link);
				if (ret != NDIS_STATUS_SUCCESS) {
					/* mld link delete fail */
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
							"\tmld link(%pM) disconn fail\n", sta_mld_link->link_addr);
				}
				break;
			case MLD_DISC_OP_DEL_STA_N_ACT:
			case MLD_DISC_OP_DEL_STA_LINK:
				ret = event_ops->bmgr_event_mld_del_sta(&del_link);
				if (ret == NDIS_STATUS_SUCCESS) {
					/* mld link delete success */
					sta_mld_link->priv_ptr = NULL;
					sta_mld_link->active = FALSE;
				} else {
					/* mld link delete fail */
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
							"\tmld link(%pM) disconn fail\n", sta_mld_link->link_addr);
				}
				break;
			default:
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"invalid op %d\n", op);
				break;
			}
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
			"invalid link %pM\n", sta_mld_link->link_addr);
	}

	return ret;
}

/* delete a MLD STA */
int bss_mngr_con_del_mld_sta(
	IN struct bmgr_mld_sta	*bmgr_sta,
	IN u8 op
)
{
	int ret, Ret = NDIS_STATUS_SUCCESS;
	u8 link;
	struct bmgr_mld_link *mld_link = NULL;

	/* send del_sta event to active link */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		ret = NDIS_STATUS_SUCCESS;
		mld_link = &bmgr_sta->mld_link[link];
		if (mld_link->active)
			ret = bss_mngr_con_del_mld_sta_link(mld_link, op);
		else {
			/* release resource if acquired previously */
			if (mld_link->priv_ptr) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_WARN,
					"W: L%d, R%d, addr(M:%pM, L:%pM)\n",
					link, mld_link->requested, bmgr_sta->mld_addr, mld_link->link_addr);
				ret = bss_mngr_con_del_mld_sta_link(mld_link, op);
			}
		}

		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Fail L%d, R%d, addr(M:%pM, L:%pM)\n",
				link, mld_link->requested, bmgr_sta->mld_addr, mld_link->link_addr);
			Ret = NDIS_STATUS_FAILURE;
		}
	}

	return Ret;
}

int bss_mngr_con_mld_disconn_op(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 op
)
{
	int ret = NDIS_STATUS_FAILURE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta;

	if (entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[%d] mld_sta(%d), op(%d)\n",
				entry->bss_idx, mld_sta_idx, op);

		if (BMGR_VALID_MLD_STA(mld_sta_idx)) {
			bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
			if (bmgr_sta && bmgr_sta->valid) {
				switch (op) {
				case MLD_DISC_OP_ACT_ONLY:
				case MLD_DISC_OP_DEL_STA_N_ACT:
					ret = bss_mngr_con_del_mld_sta(bmgr_sta, op);
					if ((ret == NDIS_STATUS_SUCCESS) &&
						(op == MLD_DISC_OP_DEL_STA_N_ACT)) {
						mutex_lock(&bss_mngr.op_mutex);
						mld_sta_release(mld_sta_idx);
						mutex_unlock(&bss_mngr.op_mutex);
					}
					break;
				case MLD_DISC_OP_DEL_STA_LINK:
					ret = bss_mngr_con_del_mld_sta_link(&bmgr_sta->mld_link[entry->link_id], op);
					break;
				default:
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"invalid op %d (mld_sta_idx %d)\n", op, mld_sta_idx);
					break;
				}
			}
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"invalid mld_sta_idx %d\n", mld_sta_idx);
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR, "entry is NULL\n");
	}

	return ret;
}

void bss_mngr_con_mld_add_sta_profile(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 link_id,
	IN u8 frame_mld_type,
	IN u8 *buf,
	IN u16 buf_len
)
{
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	struct ie *sta_pf = NULL;
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_ASSOC, DBG_LVL_INFO,
		"wdev=%s, Pf(len=%d, lk=%d) for type (%d)\n",
		entry ? RtmpOsGetNetDevName(pNetDev) : "N/A",
		buf_len, link_id, frame_mld_type);

	if ((buf_len != 0) && (buf_len < BSS_MNGR_MAX_STA_PROFILE_LEN)) {
		switch (frame_mld_type) {
		case BMGR_MLD_PROBE_RSP_COMPLETE_PROFILE:
			/* put the STA Profile to self bmgr_entry */
			if (BMGR_VALID_MLO_BSS_ENTRY(entry)) {
				sta_pf = &entry->ml_probe_rsp_sta_pf[link_id];
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
					"Add STA Profile for Probe rsp [%d]\n", link_id);
			}
			break;
		case BMGR_MLD_PROBE_RSP_PARTIAL_PROFILE:
			/* TODO */
			break;
		case BMGR_MLD_ASSOC_RSP_COMPLETE_PROFILE:
			/* put the STA Profile to peer bmgr_sta */
			if (BMGR_VALID_MLD_STA(mld_sta_idx))
				bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
			if (!bmgr_sta || !bmgr_sta->valid) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
					"Error: bmgr_sta(%d) is not valid!\n", mld_sta_idx);
				return;
			}
			mld_link = &bmgr_sta->mld_link[link_id];
			if (!mld_link->requested) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
					"Error: peer MLD do not request link %d!\n", link_id);
				return;
			}
			sta_pf = &mld_link->assoc_rsp_sta_profile;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
				"Add STA Profile for ASSOC rsp [%d]\n", link_id);
			break;
		case BMGR_MLD_APPL_ELEM_STA_PROFILE:
			/* Add appl_elem to reporting  */
			mld_add_appl_elem(entry, buf, buf_len);
			return;
		default:
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
				"drop the add profile operation for frame_mld_type: %d\n", frame_mld_type);
			break;
		}

		/* move content to target STA Profile buffer */
		if (sta_pf) {
			if (!sta_pf->buf) {
				os_alloc_mem(NULL, (UCHAR **)&sta_pf->buf, BSS_MNGR_MAX_STA_PROFILE_LEN);
				if (!sta_pf->buf) {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
						"Error: can not allocate memory!\n");
					return;
				}
			} else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
					"Notice: reuse the allocated sta pf buffer!\n");

			sta_pf->len = 0;
			NdisMoveMemory(sta_pf->buf, buf, buf_len);
			sta_pf->len = buf_len;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_INFO,
				"Add STA Profile result: %p, len=%d\n", sta_pf->buf, sta_pf->len);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_ERROR,
			"Error: [len=%d] Input STA Profile w/ Len=0 or Len>%d!\n",
			buf_len, BSS_MNGR_MAX_STA_PROFILE_LEN);
	}
}

/* elmt_id_arr=NULL && id_arr_len==0 means remove the whole per_sta_prf */
bool bss_mngr_con_mld_remove_sta_profile(
	PNET_DEV pNetDev,
	u8 frame_mld_type,
	u8 *elmt_id_arr,
	u16 id_arr_len)
{
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_NOTICE,
		"Remove elmt from STA Profile for %d type, bmgr_entry(%p).\n", frame_mld_type, entry);

	if (frame_mld_type == BMGR_MLD_APPL_ELEM_STA_PROFILE && id_arr_len == 0) {
		/* Remove appl_elem from each reporting link */
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_NOTICE,
			"Remove whole sta_prof for BMGR_MLD_APPL_ELEM_STA_PROFILE\n");

		mld_remove_appl_elem(entry);
		return TRUE;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_CONN, DBG_LVL_NOTICE, "The process is TBD!\n");
	return FALSE;
}

void bss_mngr_con_sync_ml_probe_rsp_per_sta_profile(
	IN PNET_DEV pNetDev
)
{
	u8 link, bss_idx, mld_grp;
	u8 repted_link, repting_link;
	u8 repted_bss_idx;
	u8 *f_buf, *pos;
	bool entry_rdy[BSS_MNGR_MAX_BAND_NUM] = {0};

	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *repting_entry;
	struct bmgr_entry *repted_entry;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();
	struct ie rsp_frm[BSS_MNGR_MAX_BAND_NUM];
	struct ie *per_sta_pf = NULL;

	if (!entry) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: entry is NULL!\n");
		return;
	}

	mld_grp = entry->entry_info.mld_grp;
	mld = BMGR_VALID_MLD_GRP_IDX(mld_grp) ? GET_MLD_BY_GRP_IDX(mld_grp) : NULL;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"bss_idx (%d), mld grp invalid(%d)\n",
			entry->bss_idx, entry->entry_info.mld_grp);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"%s(%s) sync probe rsp STA-Pf, mlo:%d, cnt:%d\n",
		pNetDev->name, entry->valid ? "active" : "inactive",
		entry->mlo_en, mld->mld_link_cnt);

	if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			/* free non-MLO entry's buffer */
			per_sta_pf = &entry->ml_probe_rsp_per_sta_pf[link];
			per_sta_pf->len = 0;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"zero len entry[link=%d].per_sta_pf[link=%d]\n",
				entry->link_id, link);

		}
	}

	/* query probe response frames to driver */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		repting_entry = NULL;
		rsp_frm[link].buf = NULL;
		rsp_frm[link].len = 0;

		bss_idx = mld->bss_idx_mld[link];
		if (BMGR_VALID_BSS_IDX(bss_idx)) {
			repting_entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
			if (BMGR_VALID_MLO_BSS_ENTRY(repting_entry)) {
#ifdef DOT11V_MBSSID_SUPPORT
				/*
				 * rsp_frm is used later as Probe Response of reporting AP.
				 *
				 * reporting AP shall be able to transmit Probe Response,
				 * e.g. Legacy BSS, co-hosted MBSS, 11v transmitted BSSID
				 * If the AP of this link in MLD is non-tx BSSID, we shall
				 * replace the Probe Response of this link with tx BSSID's
				 * Probe Response.
				 */
				if (IS_DEV_TYPE_11V_NT(repting_entry->entry_info.dev_type)) {
					bss_idx = repting_entry->bss_idx_tx;

					if (BMGR_VALID_BSS_IDX(bss_idx)
						&& BMGR_VALID_BSS_ENTRY(GET_BSS_ENTRY_BY_IDX(bss_idx)))
						repting_entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
				}
#endif
				event_ops->bmgr_event_mld_query_probe_rsp_frame(
					repting_entry->pNetDev, &rsp_frm[link]);

				if (rsp_frm[link].len == 0)
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"Error: link[%d] fail to query probe response frame!\n", link);
				entry_rdy[link] = TRUE;
			}
		}
	}

	/* form the Per-STA Profile for ML Probe response */
	/* repted_link: the link that is reported in Per-STA Profile in ML IE */
	for (repted_link = 0; repted_link < BSS_MNGR_MAX_BAND_NUM; repted_link++) {
		if (!entry_rdy[repted_link])
			continue;

		repted_bss_idx = mld->bss_idx_mld[repted_link];
		if (!BMGR_VALID_BSS_IDX(repted_bss_idx))
			continue;

		repted_entry = GET_BSS_ENTRY_BY_IDX(repted_bss_idx);

		/* repting_link: the link that receives the request frm */
		for (repting_link = 0; repting_link < BSS_MNGR_MAX_BAND_NUM; repting_link++) {
			if (repted_link == repting_link)
				continue;

			/*
			 *  repted entry [if link 0]
			 *    ml_probe_rsp_per_sta_pf
			 *      [0]: Per-STA Profile of repted [link 0] for repting [link 0]
			 *            -> (always empty because repting link is repted link)
			 *      [1]: Per-STA Profile of repted [link 0] for repting [link 1]
			 *      [2]: Per-STA Profile of repted [link 0] for repting [link 2]
			 *
			 *  if peer transmits ML Probe Request by link 1:
			 *     - [link 0 entry] deliver ml_probe_rsp_per_sta_pf[1] to [link 1].
			 *     - [link 1 entry] skip.
			 *     - [link 2 entry] deliver ml_probe_rsp_per_sta_pf[1] to [link 1].
			 */
			per_sta_pf = &repted_entry->ml_probe_rsp_per_sta_pf[repting_link];

			if (!entry_rdy[repting_link]) {
				/* free buffer for non-MLO entry */
				per_sta_pf->len = 0;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"zero len entry[link=%d].per_sta_pf[link=%d]\n",
					repted_entry->link_id, repting_link);
				continue;
			}

			/*
			 * apply inheritance rule among reporting and reported links,
			 * and build Per-STA Profile in Basic Multi-link IE in ML Probe
			 * Response Frame.
			 */
			if (!per_sta_pf->buf)
				os_alloc_mem(NULL, (UCHAR **)&per_sta_pf->buf, BSS_MNGR_MAX_PER_STA_PROFILE_LEN);
			else {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"Notice: reuse the per-sta profile buffer!\n");
			}

			if (per_sta_pf->buf) {
				bool locked = FALSE;

				if (rsp_frm[repting_link].buf) {
					event_ops->bmgr_event_mld_form_ml_probe_rsp_sta_pf(
						repted_entry->pNetDev, &rsp_frm[repting_link], repting_link);

					if (!in_interrupt()) {
						mutex_lock(&bss_mngr.op_mutex);
						locked = TRUE;
					} else {
						MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
							"In interrupt!\n");
					}

					f_buf = pos = per_sta_pf->buf;
					pos = build_multi_link_per_sta_profile(
							repted_entry,
							f_buf,
							BMGR_MLD_PROBE_RSP_COMPLETE_PROFILE,
							BMGR_MAX_MLD_STA_CNT,
							repting_link);
					per_sta_pf->len = pos - f_buf;

					if (locked)
						mutex_unlock(&bss_mngr.op_mutex);

					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
						"repted link %d, per-sta profile len %d for repting link %d!\n",
						repted_link, per_sta_pf->len, repting_link);
				}
			} else
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Error: can not alloc buffer for repted link %d for repting link %d!\n",
					repted_link, repting_link);
		}
	}

	/* free all buffer allocated by ap_probe_response_xmit() */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		if (rsp_frm[link].buf)
			MlmeFreeMemory(rsp_frm[link].buf);
	}
}

void bss_mngr_con_mld_ba_ori_setup(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 TID,
	IN u16 TimeOut
)
{
	u8 link;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct mld_ba_info ba_info;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!entry || !bmgr_sta || !bmgr_sta->valid)
		return;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld))
		return;

	link = entry->link_id;
	mld_link = &bmgr_sta->mld_link[link];

	if (mld_link->active) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d] link_id(%d)\n", entry->bss_idx, link);

		event_ops = entry->entry_info.event_ops;

		if (event_ops) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tpEntry(%p)\n", mld_link->priv_ptr);

			ba_info.TID = TID;
			ba_info.TimeOut = TimeOut;
			ba_info.priv_ptr = mld_link->priv_ptr;
			event_ops->bmgr_event_mld_ba_ori_setup(entry->pNetDev, &ba_info);
		}
	}
}

u8 bss_mngr_con_mld_ba_resrc_ori_add(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 TID,
	IN u16 ori_ba_wsize,
	IN u8 amsdu_en,
	IN u16 TimeOut
)
{
	u8 link, ret = FALSE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct mld_ba_info ba_info;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!entry || !bmgr_sta || !bmgr_sta->valid)
		return ret;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld))
		return ret;

	link = entry->link_id;
	mld_link = &bmgr_sta->mld_link[link];

	if (mld_link->active) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d] link_id(%d)\n", entry->bss_idx, link);

		event_ops = entry->entry_info.event_ops;

		if (event_ops) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tpEntry(%p)\n", mld_link->priv_ptr);

			ba_info.TID = TID;
			ba_info.BAWinSize = ori_ba_wsize;
			ba_info.amsdu_en = amsdu_en;
			ba_info.TimeOut = TimeOut;
			ba_info.priv_ptr = mld_link->priv_ptr;
			ret = event_ops->bmgr_event_mld_ba_resrc_ori_add(entry->pNetDev, &ba_info);
			bmgr_sta->BAOriTID |= (1 << TID);
		}
	}

	return ret;
}

u8 bss_mngr_con_mld_ba_resrc_rec_add(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 TID,
	IN u16 TimeOut,
	IN u16 StartSeq,
	IN u16 rec_ba_wsize
)
{
	u8 link, ret = FALSE;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct mld_ba_info ba_info;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!entry || !bmgr_sta || !bmgr_sta->valid)
		return ret;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld))
		return ret;

	link = entry->link_id;
	mld_link = &bmgr_sta->mld_link[link];

	if (mld_link->active) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d] link_id(%d)\n", entry->bss_idx, link);

		event_ops = entry->entry_info.event_ops;

		if (event_ops) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tpEntry(%p)\n", mld_link->priv_ptr);

			ba_info.TID = TID;
			ba_info.TimeOut = TimeOut;
			ba_info.BAWinSize = rec_ba_wsize;
			ba_info.Seq = StartSeq;
			ba_info.priv_ptr = mld_link->priv_ptr;
			ret = event_ops->bmgr_event_mld_ba_resrc_rec_add(entry->pNetDev, &ba_info);
			bmgr_sta->BARecTID |= (1 << TID);
		}
	}

	return ret;
}

void bss_mngr_con_mld_ba_resrc_ori_del(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 TID,
	IN u8 bPassive
)
{
	u8 link;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct mld_ba_info ba_info;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!entry || !bmgr_sta || !bmgr_sta->valid)
		return;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld))
		return;

	link = entry->link_id;
	mld_link = &bmgr_sta->mld_link[link];

	if (mld_link->active) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d] link_id(%d)\n", entry->bss_idx, link);

		event_ops = entry->entry_info.event_ops;

		if (event_ops) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tpEntry(%p)\n", mld_link->priv_ptr);

			ba_info.TID = TID;
			ba_info.bPassive = bPassive;
			ba_info.priv_ptr = mld_link->priv_ptr;
			event_ops->bmgr_event_mld_ba_resrc_ori_del(entry->pNetDev, &ba_info);
			bmgr_sta->BAOriTID &= ~(1 << TID);
		}
	}
}

void bss_mngr_con_mld_ba_resrc_rec_del(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 TID,
	IN u8 bPassive
)
{
	u8 link;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct mld_ba_info ba_info;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!entry || !bmgr_sta || !bmgr_sta->valid)
		return;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld))
		return;

	link = entry->link_id;
	mld_link = &bmgr_sta->mld_link[link];

	if (mld_link->active) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%d] link_id(%d)\n", entry->bss_idx, link);

		event_ops = entry->entry_info.event_ops;

		if (event_ops) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"\tpEntry(%p)\n", mld_link->priv_ptr);

			ba_info.TID = TID;
			ba_info.bPassive = bPassive;
			ba_info.priv_ptr = mld_link->priv_ptr;
			event_ops->bmgr_event_mld_ba_resrc_rec_del(entry->pNetDev, &ba_info);
			bmgr_sta->BARecTID &= ~(1 << TID);
		}
	}
}

void bss_mngr_con_mld_ba_add_to_asic(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 TID,
	IN u16 Seq,
	IN u16 BAWinSize,
	IN int type,
	IN u8 amsdu_en
)
{
	u8 link, mld_bss_idx, TID_is_map;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_entry *mld_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct mld_ba_info ba_info;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!entry || !bmgr_sta || !bmgr_sta->valid)
		return;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, bss_idx (%d) in mld (%d) is invalid\n",
			entry->bss_idx, entry->entry_info.mld_grp);
		return;
	}
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		mld_link = &bmgr_sta->mld_link[link];
		mld_bss_idx = mld->bss_idx_mld[link];
		mld_entry = NULL;
		TID_is_map = 0;

		if (BMGR_VALID_BSS_IDX(mld_bss_idx))
			mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);

		if (mld_entry)
			/* TID-to-link mapping */
			TID_is_map = mld_entry->tid_map & (1 << TID);

		if (mld_link->active && TID_is_map) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[%d] link_id(%d)\n", mld_entry->bss_idx, link);

			event_ops = mld_entry->entry_info.event_ops;

			if (event_ops) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\tpEntry(%p)\n", mld_link->priv_ptr);

				ba_info.TID = TID;
				ba_info.Seq = Seq;
				ba_info.BAWinSize = BAWinSize;
				ba_info.type = type;
				ba_info.amsdu_en = amsdu_en;
				ba_info.priv_ptr = mld_link->priv_ptr;
				event_ops->bmgr_event_mld_ba_add_to_asic(mld_entry->pNetDev, &ba_info);
			}
		}
	}
}

void bss_mngr_con_mld_ba_del_from_asic(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 TID,
	IN int type,
	IN u8 amsdu_en
)
{
	u8 link, mld_bss_idx, TID_is_map;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bmgr_entry *mld_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct mld_ba_info ba_info;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!entry || !bmgr_sta || !bmgr_sta->valid)
		return;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, bss_idx (%d) in mld (%d) is invalid\n",
			entry->bss_idx, entry->entry_info.mld_grp);
		return;
	}

	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		mld_link = &bmgr_sta->mld_link[link];
		mld_bss_idx = mld->bss_idx_mld[link];
		mld_entry = NULL;
		TID_is_map = 0;

		if (BMGR_VALID_BSS_IDX(mld_bss_idx))
			mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);

		if (mld_entry)
			/* TID-to-link mapping */
			TID_is_map = mld_entry->tid_map & (1 << TID);

		if (mld_link->active && TID_is_map) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[%d] link_id(%d)\n", mld_entry->bss_idx, link);

			event_ops = mld_entry->entry_info.event_ops;

			if (event_ops) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"\tpEntry(%p)\n", mld_link->priv_ptr);

				ba_info.TID = TID;
				ba_info.type = type;
				ba_info.amsdu_en = amsdu_en;
				ba_info.priv_ptr = mld_link->priv_ptr;
				event_ops->bmgr_event_mld_ba_del_from_asic(mld_entry->pNetDev, &ba_info);
			}
		}
	}

}

void bss_mngr_con_twt_agg_timeout_fun(struct work_struct *work)
{
	struct bss_mlo_twt *mlo_twt = &bss_mngr.mlo_twt;
	struct MLO_ITWT_ELEMENT_T *temp_element = NULL;
	struct MLO_ITWT_ELEMENT_T *temp_element_next = NULL;
	struct MLO_TWT_PRIV_T *temp_priv = NULL;
	u8 empty_before, empty_after;
	u8 del_cnt = 0;

	OS_SEM_LOCK(&mlo_twt->lock);
	empty_before = list_empty(&mlo_twt->breakdown_action_frame_list);
	list_for_each_entry_safe(temp_element, temp_element_next, &mlo_twt->breakdown_action_frame_list, list) {
		temp_priv = &temp_element->priv;
		if (temp_priv->mld_sta_idx == mlo_twt->mld_sta_idx) {
			list_del(&temp_element->list);
			os_free_mem(temp_element);
			del_cnt++;
		}
	}
	empty_after = list_empty(&mlo_twt->breakdown_action_frame_list);
	OS_SEM_UNLOCK(&mlo_twt->lock);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"empty=%d,%d, del_cnt=%d\n", empty_before, empty_after, del_cnt);
}

static void bss_mngr_con_twt_agg_timer_cancel(
	IN struct bss_mlo_twt *mlo_twt
)
{
	BOOLEAN cancelled;
	RALINK_TIMER_STRUCT *timer = &mlo_twt->agg_action_frame_timer;

	RTMPCancelTimer(timer, &cancelled);
	RTMPReleaseTimer(timer, &cancelled);
}

static void bss_mngr_con_twt_agg_timeout_exec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct bss_mlo_twt *mlo_twt = (struct bss_mlo_twt *)FunctionContext;
	struct work_struct *agg_timeout_work = NULL;

	if (!mlo_twt) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"fail. mlo_twt=NULL\n");
		return;
	}

	agg_timeout_work = &mlo_twt->agg_timeout_work;
	INIT_WORK(agg_timeout_work, bss_mngr_con_twt_agg_timeout_fun);
	schedule_work(agg_timeout_work);
}

DECLARE_TIMER_FUNCTION(bss_mngr_con_twt_agg_timeout_exec);
BUILD_TIMER_FUNCTION(bss_mngr_con_twt_agg_timeout_exec);

void bss_mngr_con_mld_twt_action_frm_forward(
	IN PNET_DEV pNetDev,
	IN struct _MLME_QUEUE_ELEM *elem
)
{
	u8 link = 0;
	u8 mld_bss_idx = 0;
	struct bmgr_entry *master_bss_entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *expected_bss_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct _HEADER_802_11 *hdr = (struct _HEADER_802_11 *)&elem->Msg;

	if (!pNetDev || !master_bss_entry || !elem || !hdr)
		return;

	mld = master_bss_entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, bss_idx (%d) in mld (%d) is invalid\n",
			master_bss_entry->bss_idx, master_bss_entry->entry_info.mld_grp);
		return;
	}

	/* use A3 to find BSSID whom this action frame belongs to */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		mld_bss_idx = mld->bss_idx_mld[link];
		expected_bss_entry = BMGR_VALID_BSS_IDX(mld_bss_idx) ? GET_BSS_ENTRY_BY_IDX(mld_bss_idx) : NULL;

		if (!expected_bss_entry)
			continue;
		if (MAC_ADDR_EQUAL(expected_bss_entry->entry_info.bssid, hdr->Addr3)) {
			struct wifi_dev *wdev = RtmpOsGetNetDevWdev(expected_bss_entry->pNetDev);
			struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
			struct _MAC_TABLE_ENTRY *peer_entry = MacTableLookup2(ad, hdr->Addr2, wdev);

			if (peer_entry) {
				if (peer_entry->wdev == wdev) {
					event_ops = expected_bss_entry->entry_info.event_ops;
					if (event_ops)
						event_ops->bmgr_event_mld_twt_action_frm_forward(elem, peer_entry);
				} else
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
						"fail. wdev mismatch, IF=%s,%s\n",
						peer_entry->wdev->if_dev->name, wdev->if_dev->name);
			} else
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"fail. fail to find peer_entry\n");
			break;
		}
	}

	if (link == BSS_MNGR_MAX_BAND_NUM)
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. BSSID not found\n");
}

static void action_frame_breakdown_with_linkid(
	PNET_DEV pNetDev, /* sa */
	u16 mld_sta_idx,
	struct _MLME_QUEUE_ELEM *elem,
	struct bmgr_mld_sta *mld_sta,
	struct bmgr_entry *bss_entry, /* sa */
	struct mlo_itwt_ie *mlo_twt_ie,
	u8 action_linkid,
	u16 linkid_bitmap,
	u16 all_linkid_bitmap,
	u8 *breakdown
)
{
	u8 linkid = 0;
	struct bmgr_mld_link *mld_link = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct _MAC_TABLE_ENTRY *peer_entry = NULL;
	u8 mld_bss_idx = 0;
	u8 *link_addr = NULL;
	struct bmgr_entry *mld_bss_entry = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;

	if (!mlo_twt_ie) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. twt_ie=NULL\n");
		return;
	}

	if (!(GET_TWT_CTRL_LINKID_BITMAP_PRESENT(mlo_twt_ie->control) && linkid_bitmap)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. linkid_bitmap_present=%d, linkid_bitmap=0x%x\n",
			(UINT8)GET_TWT_CTRL_LINKID_BITMAP_PRESENT(mlo_twt_ie->control),
			linkid_bitmap);
		return;
	}

	linkid = ffs(linkid_bitmap) - 1;
	mld_link = &mld_sta->mld_link[linkid];
	peer_entry = (struct _MAC_TABLE_ENTRY *)mld_link->priv_ptr;
	if (!peer_entry) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. linkid=%d, peer_entry=NULL\n\n", linkid);
		return;
	}

	link_addr = mld_link->link_addr;
	if (!MAC_ADDR_EQUAL(peer_entry->Addr, link_addr)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. mac not equal. peer_entry->Addr=%pM, link_addr=%pM\n",
			peer_entry->Addr, link_addr);
		return;
	}

	mld = bss_entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"fail, bss_idx (%d) in mld (%d) is invalid\n",
			bss_entry->bss_idx, bss_entry->entry_info.mld_grp);
		return;
	}

	mld_bss_idx = mld->bss_idx_mld[linkid];
	if (!BMGR_VALID_BSS_IDX(mld_bss_idx)) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. invalid mld_bss_idx=%d\n", mld_bss_idx);
		return;
	}

	mld_bss_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
	if (mld_bss_entry->link_id != linkid) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. dest link id mismatch=%d,%d\n", mld_bss_entry->link_id, linkid);
		return;
	}

	event_ops = mld_bss_entry->entry_info.event_ops;
	if (!event_ops) {
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
			"fail. event_ops=NULL\n");
		return;
	}

	/* forward N of "1->N" berakdown action frame to correct link MLME */
	*breakdown |= event_ops->bmgr_event_mld_twt_action_frm_breadown(
		mld_bss_entry->pNetDev, /* da */
		pNetDev, /* sa */
		linkid, /* da */
		bss_entry->link_id, /* sa */
		mld_sta_idx,
		action_linkid,
		linkid_bitmap,
		all_linkid_bitmap,
		elem,
		mlo_twt_ie,
		peer_entry);
}

u8 bss_mngr_con_mld_twt_action_frm_breadown(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN struct _MLME_QUEUE_ELEM *elem
)
{
	/* 1->N */
	struct bmgr_entry *bss_entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *mld_sta = NULL;
	struct mlo_itwt_ie *mlo_twt_ie = NULL;
	struct _HEADER_802_11 *hdr = (struct _HEADER_802_11 *)&elem->Msg;
	struct bmgr_mld_link *mld_link = NULL;
	u8 link = 0;
	u8 *ie_ptr = NULL;
	EID_STRUCT *eid_ptr;
	u16 len = 0;
	u8 action_linkid = 0;
	u16 all_linkid_bitmap = 0;
	u8 breakdown = 0;
	u16 linkid_bitmap = 0;
	u8 linkid_bitmap_present = 0;

	/* get mld_sta */
	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		mld_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!bss_entry || !mld_sta || !mld_sta->valid)
		return breakdown;

	/* get action linkid */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		mld_link = &mld_sta->mld_link[link];
		if (MAC_ADDR_EQUAL(mld_link->link_addr, hdr->Addr2)) {
			action_linkid = link;
			break;
		}
	}

	/* get all_linkid_bitmap */
	ie_ptr = (u8 *)(elem->Msg + LENGTH_802_11 + 3);
	len = elem->MsgLen - (LENGTH_802_11 + 3);
	/*dump_hex_content(elem->Msg, elem->MsgLen);*/
	while (len > 0) {
		eid_ptr = (EID_STRUCT *)ie_ptr;
		mlo_twt_ie = (struct mlo_itwt_ie *)eid_ptr;
		linkid_bitmap = mlo_twt_ie->linkid_bitmap;
		linkid_bitmap_present = (u8)GET_TWT_CTRL_LINKID_BITMAP_PRESENT(mlo_twt_ie->control);
		MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
			"len=%d,T=%d,L=%d,linkid_bitmap_present=%d,linkid_bitmap=0x%x\n",
			len, eid_ptr->Eid, eid_ptr->Len, linkid_bitmap_present, linkid_bitmap);
		if (linkid_bitmap_present && linkid_bitmap)
			all_linkid_bitmap |= linkid_bitmap;
		else
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"fail. linkid_bitmap_present=%d, linkid_bitmap=0x%x\n",
				linkid_bitmap_present, linkid_bitmap);

		ie_ptr += (eid_ptr->Len + 2);
		len -= (eid_ptr->Len + 2);
	};

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"all_linkid_bitmap=0x%x\n", all_linkid_bitmap);

	/* use twt ie linkid to find peer_entry from mld_sta */
	ie_ptr = (u8 *)(elem->Msg + LENGTH_802_11 + 3);
	len = elem->MsgLen - (LENGTH_802_11 + 3);
	while (len > 0) {
		eid_ptr = (EID_STRUCT *)ie_ptr;
		mlo_twt_ie = (struct mlo_itwt_ie *)eid_ptr;
		linkid_bitmap = mlo_twt_ie->linkid_bitmap;

		action_frame_breakdown_with_linkid(pNetDev,
			mld_sta_idx,
			elem,
			mld_sta,
			bss_entry, /* sa */
			mlo_twt_ie,
			action_linkid,
			linkid_bitmap,
			all_linkid_bitmap,
			&breakdown);

		ie_ptr += (eid_ptr->Len + 2);
		len -= (eid_ptr->Len + 2);
	};

	/* start timer to check each breadown action frame form per MLME */
	if (breakdown) {
		struct wifi_dev *wdev = RtmpOsGetNetDevWdev(pNetDev);
		struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
		struct bss_mlo_twt *mlo_twt = &bss_mngr.mlo_twt;
		RALINK_TIMER_STRUCT *timer = &mlo_twt->agg_action_frame_timer;

		/* this ad = action link and only action link ad does breakdown */
		os_zero_mem(timer, sizeof(RALINK_TIMER_STRUCT));
		mlo_twt->mld_sta_idx = mld_sta_idx;
		RTMPInitTimer(ad, timer, GET_TIMER_FUNCTION(bss_mngr_con_twt_agg_timeout_exec), mlo_twt, FALSE);
		RTMPSetTimer(timer, TWT_ACT_FTM_AGG_TIME);
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO, "breakdown=%d\n", breakdown);

	return breakdown;
}

void bss_mngr_con_mld_twt_action_frm_agg(
	IN PNET_DEV pNetDev,
	IN struct frame_itwt_setup *itwt_setup_frame,
	IN struct MLO_TWT_PRIV_T *priv
)
{
	/* N->1*/
	struct bss_mlo_twt *mlo_twt = &bss_mngr.mlo_twt;
	struct MLO_ITWT_ELEMENT_T *element = NULL;
	struct MLO_ITWT_ELEMENT_T *temp_element = NULL;
	struct MLO_TWT_PRIV_T *_priv = NULL;
	struct MLO_TWT_PRIV_T *temp_priv = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	u16 all_linkid_bitmap = 0;
	u16 mld_sta_idx = 0;
	u8 action_linkid = 0;
	u8 all_arrive = 0;
	u8 *out_buffer = NULL;
	u8 cnt = 0;

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"(IN)A1=%pM,A2=%pM,A3=%pM from IF=%s\n",
		itwt_setup_frame->hdr.Addr1,
		itwt_setup_frame->hdr.Addr2,
		itwt_setup_frame->hdr.Addr3,
		pNetDev->name);

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"(IN)da/sa=%s,%s,da/sa=%d,%d,mld_sta_idx=%d,action_linkid=%d,linkid_bitmap=0x%x,all_linkid_bitmap=0x%x\n",
		priv->da_net_dev->name,
		priv->sa_net_dev->name,
		priv->da_linkid, priv->sa_linkid,
		priv->mld_sta_idx,
		priv->action_linkid,
		priv->linkid_bitmap,
		priv->all_linkid_bitmap);

	/* breakdown phase creates each link priv and DA=ohter links, SA=action link */
	if (os_alloc_mem(NULL, (UCHAR **)&element, sizeof(struct MLO_ITWT_ELEMENT_T)) == NDIS_STATUS_SUCCESS) {
		os_move_mem(&element->itwt_setup_frame, itwt_setup_frame, sizeof(struct frame_itwt_setup));
		_priv = &element->priv;
		/* _priv->da_net_dev = action link */
		_priv->da_net_dev = priv->sa_net_dev;
		_priv->sa_net_dev = priv->da_net_dev;
		_priv->da_linkid = priv->sa_linkid;
		_priv->sa_linkid = priv->da_linkid;
		_priv->mld_sta_idx = priv->mld_sta_idx;
		_priv->action_linkid = priv->action_linkid;
		_priv->linkid_bitmap = priv->linkid_bitmap;
		_priv->all_linkid_bitmap = priv->all_linkid_bitmap;
		_priv->state = STATE_AGGREGATE_ACTION_FRAME;
	} else
		return;

	mld_sta_idx = priv->mld_sta_idx;
	action_linkid = priv->action_linkid;


	OS_SEM_LOCK(&mlo_twt->lock);
	/* add element to pool */
	list_add_tail(&element->list, &mlo_twt->breakdown_action_frame_list);
	list_for_each_entry(temp_element, &mlo_twt->breakdown_action_frame_list, list) {
		cnt++;
		temp_priv = &temp_element->priv;
		if (temp_priv->mld_sta_idx == mld_sta_idx) {
			all_linkid_bitmap |= temp_priv->linkid_bitmap;
			if (all_linkid_bitmap == temp_priv->all_linkid_bitmap) {
				all_arrive = 1;
				break;
			}
		}
	}

	MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
		"cnt=%d,arrive=%d\n", cnt, all_arrive);

	/* aggregate from element pool */
	if (all_arrive) {
		struct bmgr_mld_sta *mld_sta = NULL;
		struct bmgr_mld_link *mld_link = NULL;
		struct _MAC_TABLE_ENTRY *peer_entry = NULL;
		PNET_DEV temp_da_net_dev = NULL;
		struct MLO_ITWT_ELEMENT_T *temp_element_next = NULL;
		u16 temp_all_linkid_bitmap = 0;
		u16 temp_linkid_bitmap = 0;
		ULONG len = 0;
		ULONG temp_len = 0;

		/* get action link peer entry */
		if (BMGR_VALID_MLD_STA(mld_sta_idx))
			mld_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

		if (!mld_sta || !mld_sta->valid) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"fail. mld_sta not found by mld_sta_idx=%d\n", mld_sta_idx);
			goto done;
		}

		mld_link = &mld_sta->mld_link[action_linkid];
		if (!mld_link->active) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"fail. mld_link is not active with action_linkid=%d\n", action_linkid);
			goto done;
		}

		peer_entry = (struct _MAC_TABLE_ENTRY *)mld_link->priv_ptr;
		if (!peer_entry) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"fail. action link peer entry not found\n");
			goto done;
		}

		/* start to agg */
		all_linkid_bitmap = 0;
		MlmeAllocateMemory(NULL, &out_buffer);
		if (!out_buffer) {
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
				"fail. out_buffer=NULL\n");
			goto done;
		}

		list_for_each_entry_safe(temp_element, temp_element_next, &mlo_twt->breakdown_action_frame_list, list) {
			temp_priv = &temp_element->priv;
			if (temp_priv->mld_sta_idx == mld_sta_idx) {
				list_del(&temp_element->list);
				all_linkid_bitmap |= temp_priv->linkid_bitmap;
				temp_all_linkid_bitmap = temp_priv->all_linkid_bitmap;
				temp_linkid_bitmap = temp_priv->linkid_bitmap;
				/* temp_da_net_dev = action link */
				temp_da_net_dev = temp_priv->da_net_dev;
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"mld_sta_idx=%d, linkid_bitmap=0x%x, all_linkid_bitmap=0x%x, 0x%x\n",
					mld_sta_idx, temp_linkid_bitmap, all_linkid_bitmap, temp_all_linkid_bitmap);

				/* 802.11 hdr + cat + s1g + token */
				if (len == 0) {
					struct _HEADER_802_11 *hdr = NULL;

					MakeOutgoingFrame(out_buffer, &temp_len,
						(LENGTH_802_11 + 3), &temp_element->itwt_setup_frame,
						END_OF_ARGS);
					len += temp_len;
					/* adjust A1/A2/A3 to action link */
					hdr = (struct _HEADER_802_11 *)out_buffer;
					ActHeaderInit(NULL, hdr, peer_entry->Addr, peer_entry->bssid, peer_entry->bssid);
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"peer_link_addr=%pM\n", mld_link->link_addr);
					MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
						"(OUT)A1=%pM,A2=%pM,A3=%pM,wcid=%d\n",
						hdr->Addr1, hdr->Addr2, hdr->Addr3, peer_entry->wcid);
				}
				/* tlv: mlo_itwt_ie */
				temp_element->itwt_setup_frame.twt_ie.len = sizeof(struct mlo_itwt_ie) - 2;
				MakeOutgoingFrame(out_buffer + len, &temp_len,
					sizeof(struct itwt_ie_t), &temp_element->itwt_setup_frame.twt_ie,
					2, &temp_linkid_bitmap,
					END_OF_ARGS);
				len += temp_len;

				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_INFO,
					"(OUT)da/sa=%s,%s,da/sa=%d,%d,mld_sta_idx=%d,action_linkid=%d,linkid_bitmap=0x%x,all_linkid_bitmap=0x%x\n",
					temp_priv->da_net_dev->name,
					temp_priv->sa_net_dev->name,
					temp_priv->da_linkid, temp_priv->sa_linkid,
					temp_priv->mld_sta_idx,
					temp_priv->action_linkid,
					temp_priv->linkid_bitmap,
					temp_priv->all_linkid_bitmap);

				/*dump_hex_content(out_buffer, len);*/

				/* forward to action link = temp_da_net_dev to tx */
				if (all_linkid_bitmap == temp_all_linkid_bitmap) {
					struct bmgr_entry *bss_entry = get_bss_entry_by_netdev(temp_da_net_dev);

					/* A1=peer link addr in build_ie(peer_twt_setup_action) */
					if (bss_entry) {
						bss_mngr_con_twt_agg_timer_cancel(mlo_twt);
						event_ops = bss_entry->entry_info.event_ops;
					}

					if (event_ops) {
						event_ops->bmgr_event_mld_twt_action_frm_agg(temp_da_net_dev,
							out_buffer,
							(u16)len,
							peer_entry,
							mld_sta_idx,
							action_linkid,
							temp_all_linkid_bitmap);
					}
				}

				os_free_mem(temp_element);

				if (all_linkid_bitmap == temp_all_linkid_bitmap)
					break;
			}
		}
		MlmeFreeMemory(out_buffer);
	}
done:
	OS_SEM_UNLOCK(&mlo_twt->lock);
}

void bss_mngr_con_mld_twt_wait_ack_inform(
	IN PNET_DEV pNetDev,
	IN u16 mld_sta_idx,
	IN u8 action_linkid,
	IN u16 all_linkid_bitmap,
	IN u8 wait_ack_sts
)
{
	u8 link = 0;
	u8 mld_bss_idx = 0;
	struct bmgr_entry *bss_entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *mld_sta = NULL;
	struct bmgr_entry *mld_bss_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_link *mld_link = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	struct _MAC_TABLE_ENTRY *peer_entry = NULL;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		mld_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!bss_entry || !mld_sta || !mld_sta->valid)
		return;

	mld = bss_entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, bss_idx (%d) in mld (%d) is invalid\n",
			bss_entry->bss_idx, bss_entry->entry_info.mld_grp);
		return;
	}

	/* action link first since timer concern */
	mld_link = &mld_sta->mld_link[action_linkid];
	if (mld_link->active && (all_linkid_bitmap & 1 << action_linkid)) {
		peer_entry = (struct _MAC_TABLE_ENTRY *)mld_link->priv_ptr;
		mld_bss_idx = mld->bss_idx_mld[action_linkid];
		mld_bss_entry = BMGR_VALID_BSS_IDX(mld_bss_idx) ? GET_BSS_ENTRY_BY_IDX(mld_bss_idx) : NULL;

		if (mld_bss_entry && peer_entry) {
			event_ops = mld_bss_entry->entry_info.event_ops;
			event_ops->bmgr_event_mld_twt_wait_ack_inform(peer_entry->wdev->if_dev,
				peer_entry, wait_ack_sts);
		} else
			MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
				"link=%d,mld_bss_entry=%p,peer_entry=%p\n",
				action_linkid, mld_bss_entry, peer_entry);
	}

	/* non-action link later */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		mld_link = &mld_sta->mld_link[link];
		peer_entry = (struct _MAC_TABLE_ENTRY *)mld_link->priv_ptr;
		mld_bss_idx = mld->bss_idx_mld[link];
		mld_bss_entry = BMGR_VALID_BSS_IDX(mld_bss_idx) ? GET_BSS_ENTRY_BY_IDX(mld_bss_idx) : NULL;

		if (mld_link->active && link != action_linkid && (all_linkid_bitmap & 1 << link)) {
			if (mld_bss_entry && peer_entry) {
				event_ops = mld_bss_entry->entry_info.event_ops;
				event_ops->bmgr_event_mld_twt_wait_ack_inform(peer_entry->wdev->if_dev,
					peer_entry, wait_ack_sts);
			} else
				MTWF_DBG(NULL, DBG_CAT_PROTO, CATPROTO_TWT, DBG_LVL_ERROR,
					"link=%d,mld_bss_entry=%p,peer_entry=%p\n",
					link, mld_bss_entry, peer_entry);
		}
	}
}

#ifdef MWDS
u8 bss_mngr_con_mwds_cap_sync(
	IN PNET_DEV pNetDev,
	u8 enable
)
{
	struct bmgr_entry *bss_entry;
	struct bmgr_entry *peer_entry = NULL;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;
	u8 link = 0;
	u8 bss_mld[BSS_MNGR_MAX_BAND_NUM];
	u8 ret = 0;

	bss_entry = get_bss_entry_by_netdev(pNetDev);
	if (!bss_entry)
		return ret;

	if (BMGR_IS_ML_MLD_GRP_IDX(bss_entry->entry_info.mld_grp)) {
		mld = bss_entry->mld_ptr;
		if (!BMGR_VALID_MLO_DEV(mld)) {
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Error, bss_idx (%d) in mld (%d) is invalid\n",
				bss_entry->bss_idx, bss_entry->entry_info.mld_grp);
			return ret;
		}
		NdisMoveMemory(bss_mld, mld->bss_idx_mld, sizeof(mld->bss_idx_mld));
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			u8 bss_idx_mld = bss_mld[link];

			if (!BMGR_VALID_BSS_IDX(bss_idx_mld))
				continue;

			peer_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_mld);
			if (peer_entry) {
				event_ops = peer_entry->entry_info.event_ops;
				if (event_ops)
					ret += event_ops->bmgr_event_mld_mwds_enable(
						peer_entry->pNetDev, enable);

			}
		}
	} else {
		event_ops = bss_entry->entry_info.event_ops;
		if (event_ops)
			ret = event_ops->bmgr_event_mld_mwds_enable(
							pNetDev, enable);
	}

	return ret;
}
#endif

void bss_mngr_con_mld_critical_update(IN PNET_DEV pNetDev, IN u8 upd_scope, IN u8 reason)
{
	u8 i, link, iob_upd_num = 0;
	u8 peer_bss_idx;
	u8 dev_type;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *peer_entry, *iob_entry;
	struct bmgr_entry *updated_entry[32] = {NULL};
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();
	struct mld_crit_upd_info crit_upd_info;

	if (!BMGR_VALID_BSS_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error: entry is NULL or invalid!\n");
		return;
	}

	/* skip the AP not affiliated in any AP MLD */
	if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"[%d(%s)] not affiliated in any AP MLD\n",
			entry->bss_idx, entry->pNetDev->name);
		return;
	}

	/* step 1.
	 *	increment bss_chg_cnt (depends on scope) &
	 *	re-build ie(s) in BSS mngr to sync bss_chg_cnt */
	switch (upd_scope) {
	case BMGR_CRITICAL_UPDATE_ALL:
		// traverse all iob BSS and increment the cnt
		for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
			iob_entry = GET_BSS_ENTRY_BY_IDX(i);
			if (!BMGR_VALID_MLO_BSS_ENTRY(iob_entry))
				continue;

			/* skip BSSs that in different band */
			if (iob_entry->link_id != entry->link_id)
				continue;

			iob_entry->bss_chg_cnt = (iob_entry->bss_chg_cnt + 1) % 256;
			/* 255 means AP not in MLD or no this AP info */
			if (iob_entry->bss_chg_cnt == 255)
				iob_entry->bss_chg_cnt = 0;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[%s] update bss_chg_cnt to %d.\n", iob_entry->pNetDev->name,
				iob_entry->bss_chg_cnt);
			bss_mngr_con_sync_ie(iob_entry);
			updated_entry[iob_upd_num] = iob_entry;
			iob_upd_num++;
		}
		break;
	case BMGR_CRITICAL_UPDATE_SINGLE:
		// update single entry
		entry->bss_chg_cnt = (entry->bss_chg_cnt + 1) % 256;
		/* 255 means AP not in MLD or no this AP info */
		if (entry->bss_chg_cnt == 255)
			entry->bss_chg_cnt = 0;
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"[%s] update bss_chg_cnt to %d.\n", pNetDev->name, entry->bss_chg_cnt);
		bss_mngr_con_sync_ie(entry);
		updated_entry[iob_upd_num] = entry;
		iob_upd_num++;
		break;
	case BMGR_CRITICAL_UPDATE_MLD_CUF:
		// update single entry
		updated_entry[iob_upd_num] = entry;
		iob_upd_num++;
		break;
	default:
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"\t[%d] invalid scope!\n", upd_scope);
		return;
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"trigger interface(%s), iob_upd_num(%d)\n", pNetDev->name, iob_upd_num);

	/* step 2. prepare BSS bitmap for trigger Critical Update Flag */
	/* update all APs in the same AP MLD */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		for (i = 0; i < iob_upd_num; i++) {
			iob_entry = updated_entry[i];
			mld = iob_entry->mld_ptr;
			if (!BMGR_VALID_MLO_DEV(mld)) {
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
					"Fail, bss_idx (%d) in mld (%d) is invalid\n",
					iob_entry->bss_idx, iob_entry->entry_info.mld_grp);
				continue;
			}
			peer_bss_idx = mld->bss_idx_mld[link];

			if (!BMGR_VALID_BSS_IDX(peer_bss_idx))
				continue;

			peer_entry = GET_BSS_ENTRY_BY_IDX(peer_bss_idx);
			if (!BMGR_VALID_MLO_BSS_ENTRY(peer_entry))
				continue;

			dev_type = peer_entry->entry_info.dev_type;

			if (IS_DEV_TYPE_W_BCN(dev_type))
				peer_entry->mbss.crit_upd_bitmap |= (1 << 0);
			else {
				u8 bss_idx_tx = peer_entry->bss_idx_tx;
				struct bmgr_entry *tx_entry = NULL;

				/* use tx-bss to store bitmap for non-tx BSSID */
				if (BMGR_VALID_BSS_IDX(bss_idx_tx))
					tx_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_tx);
				else {
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
						"\t[%d] TX-BSS undefined\n", peer_bss_idx);
					continue;
				}
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
					"bmgr_entry(%d) is Nontransmitted Bssid, update to tx bmgr_entry(%d)\n",
					peer_entry->bss_idx, tx_entry->bss_idx);

				tx_entry->mbss.crit_upd_bitmap |= (1 << peer_entry->entry_info.dot11v_mbssid_idx);
				peer_entry = tx_entry;
			}

			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
				"[link=%d, updated_num=%d] tx entry(%s), bitmap(0x%x)\n", link, i,
				peer_entry->pNetDev->name, peer_entry->mbss.crit_upd_bitmap);
		}
	}

	/* step 3. update bcn (link_id that trigger critical update) */
	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		peer_entry = GET_BSS_ENTRY_BY_IDX(i);

		if (!BMGR_VALID_MLO_BSS_ENTRY(peer_entry) ||
			(peer_entry->link_id != entry->link_id))
			continue;

		dev_type = peer_entry->entry_info.dev_type;
		/* tx entry && bitmap != 0 */
		if (IS_DEV_TYPE_W_BCN(dev_type) && peer_entry->mbss.crit_upd_bitmap) {
			NdisZeroMemory(&crit_upd_info, sizeof(struct mld_crit_upd_info));
			crit_upd_info.bitmap = peer_entry->mbss.crit_upd_bitmap;
			if ((upd_scope == BMGR_CRITICAL_UPDATE_MLD_CUF) ||
				BCN_REASON_EQUAL(reason, BCN_UPDATE_CSA))
				crit_upd_info.bypass_bcn_seq_bmap = crit_upd_info.bitmap;

			event_ops->bmgr_event_mld_critical_update_bcn(
				peer_entry->pNetDev, &crit_upd_info, reason);

			/* reset bitmap after update bcn */
			peer_entry->mbss.crit_upd_bitmap = 0;
		}
	}

	/* step 4. update bcn (link_id that NOT trigger critical update) */
	for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
		peer_entry = GET_BSS_ENTRY_BY_IDX(i);

		if (!BMGR_VALID_MLO_BSS_ENTRY(peer_entry) ||
			(peer_entry->link_id == entry->link_id))
			continue;

		dev_type = peer_entry->entry_info.dev_type;
		/* tx entry && bitmap != 0 */
		if (IS_DEV_TYPE_W_BCN(dev_type) && peer_entry->mbss.crit_upd_bitmap) {
			NdisZeroMemory(&crit_upd_info, sizeof(struct mld_crit_upd_info));
			crit_upd_info.bitmap = peer_entry->mbss.crit_upd_bitmap;
			if ((upd_scope == BMGR_CRITICAL_UPDATE_MLD_CUF) ||
				BCN_REASON_EQUAL(reason, BCN_UPDATE_CSA))
				crit_upd_info.bypass_bcn_seq_bmap = crit_upd_info.bitmap;

			event_ops->bmgr_event_mld_critical_update_bcn(
				peer_entry->pNetDev, &crit_upd_info, reason);

			/* reset bitmap after update bcn */
			peer_entry->mbss.crit_upd_bitmap = 0;
		}
	}
}

BOOLEAN bss_mngr_con_mld_critical_update_op_lock(IN PNET_DEV pNetDev, u8 lock_flag)
{
	u8 i, link, mld_bss_idx = 0, iob_upd_num = 0, traversal_iob_upd = 0;
	u8 get_lock_result[BSS_MNGR_MAX_BAND_NUM];/*0:no op, 1:get lock, 2:get fail*/
	PNET_DEV	pNetDev_per_link[BSS_MNGR_MAX_BAND_NUM] = {0};
	PNET_DEV	pNetDev_per_link_11vt[16][BSS_MNGR_MAX_BAND_NUM] = {0};
	u8 idx_11vt = 0;
	BOOLEAN final_result = TRUE, need_to_lock[BSS_MNGR_MAX_BAND_NUM];
	u8 dev_type;
	struct bmgr_entry *updated_entry[32] = {NULL};
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *mld_entry, *iob_entry;
	struct bmgr_mlo_dev *mld = NULL;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	if (!BMGR_VALID_BSS_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_ERROR,
			"\x1b[41mError: entry is NULL or invalid!\x1b[m\n\r");
		return FALSE;
	}

	/* skip the AP not affiliated in any AP MLD */
	if (!BMGR_VALID_MLO_BSS_ENTRY(entry)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_ERROR,
			"\x1b[41m[%d(%s)] not affiliated in any AP MLD\x1b[m\n\r",
			entry->bss_idx, entry->pNetDev->name);
		return FALSE;
	}

	if (lock_flag == BMGR_BPCC_OP_GET_LOCK_ALL) {
		/* search all band which belong to mld group */
		for (i = 0; i < BSS_MNGR_ENTRY_CNT; i++) {
			iob_entry = GET_BSS_ENTRY_BY_IDX(i);
			if (!BMGR_VALID_MLO_BSS_ENTRY(iob_entry))
				continue;
			/* skip BSSs that in different band */
			if (iob_entry->link_id != entry->link_id)
				continue;
			updated_entry[iob_upd_num] = iob_entry;
			iob_upd_num++;
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_INFO,
				"\x1b[42m record update mld entry[%d]=0x%p,iob_upd_num=%d\x1b[m\n\r",
				i, iob_entry, iob_upd_num);
		}
	}
mld_traversal:
	final_result = TRUE;
	if (lock_flag == BMGR_BPCC_OP_GET_LOCK_ALL) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_INFO,
			"\x1b[42m traversal mld_entry=%d\x1b[m\n\r",
			traversal_iob_upd);
		if (updated_entry[traversal_iob_upd])
			mld = updated_entry[traversal_iob_upd]->mld_ptr;
		else
			mld = entry->mld_ptr;
		if (iob_upd_num > 0) {
			iob_upd_num--;
			traversal_iob_upd++;
		}
	} else
		mld = entry->mld_ptr;

	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_ERROR,
			"\x1b[41mError, bss_idx (%d) in mld (%d) is invalid\x1b[m\n\r",
			entry->bss_idx, entry->entry_info.mld_grp);
		return FALSE;
	}


	/* init all variable and wanted wdev to lock*/
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		need_to_lock[link] = TRUE;
		get_lock_result[link] = 0;
		pNetDev_per_link[link] = NULL;
		mld_bss_idx = mld->bss_idx_mld[link];
		if (!BMGR_VALID_BSS_IDX(mld_bss_idx))
			continue;
		mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
		if (!BMGR_VALID_MLO_BSS_ENTRY(mld_entry))
			continue;
		/* need to replace mld_entry to 11vt */
		dev_type = mld_entry->entry_info.dev_type;
		if (IS_DEV_TYPE_11V(dev_type)) {
			u8	bss_idx_tx;

			bss_idx_tx = mld_entry->bss_idx_tx;
			/* replace as 11vt */
			mld_entry = GET_BSS_ENTRY_BY_IDX(bss_idx_tx);
			if (pNetDev_per_link_11vt[idx_11vt][link] == 0) {
				need_to_lock[link] = TRUE;
				pNetDev_per_link_11vt[idx_11vt][link] = mld_entry->pNetDev;
			} else if (pNetDev_per_link_11vt[idx_11vt][link] == mld_entry->pNetDev) {
				need_to_lock[link] = FALSE;
				pNetDev_per_link_11vt[idx_11vt][link] = mld_entry->pNetDev;
			} else if (pNetDev_per_link_11vt[idx_11vt][link] != mld_entry->pNetDev) {
				need_to_lock[link] = TRUE;
				idx_11vt++;
				pNetDev_per_link_11vt[idx_11vt][link] = mld_entry->pNetDev;
			}
		}
		pNetDev_per_link[link] = mld_entry->pNetDev;
	}
	if ((lock_flag == BMGR_BPCC_OP_GET_LOCK) ||
		(lock_flag == BMGR_BPCC_OP_GET_LOCK_ALL)) {
		/* get bss BPCC semaphore which belong to the same MLD */
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (!pNetDev_per_link[link])
				continue;
			if (!need_to_lock[link])
				continue;
			if (get_lock_result[link] != 1) {
				if (event_ops->bmgr_event_mld_critical_update_op_lock(pNetDev_per_link[link], BMGR_BPCC_OP_GET_LOCK, mld->mld_grp) == 1)
					get_lock_result[link] = 1;
				else {
					/* if any one fail to get, it will give up others */
					get_lock_result[link] = 2;
					final_result = FALSE;
					break;
				}
			}
		}

		if (final_result == FALSE) {
			/* release all bpcc lock if any of link fail to get its' own pbcc lock */
			for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
				if (!pNetDev_per_link[link])
					continue;

				if (get_lock_result[link] == 1)
					event_ops->bmgr_event_mld_critical_update_op_lock(pNetDev_per_link[link], BMGR_BPCC_OP_RELEASE_LOCK, mld->mld_grp);
			}

			for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
				if (!pNetDev_per_link[link])
					continue;
				if (in_interrupt())
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_DEBUG,
						"\x1b[41m MLD[%d]Band[%d]=%d Get lock fail,final_result=%d\x1b[m\n\r",
						mld->mld_grp, link, get_lock_result[link], final_result);
				else
					MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_INFO,
						"\x1b[41m MLD[%d]Band[%d]=%d Get lock fail,final_result=%d\x1b[m\n\r",
						mld->mld_grp, link, get_lock_result[link], final_result);
			}
		} else {
			/* reset all bpcc lock timer if get all pbcc lock */
			for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
				if (!pNetDev_per_link[link])
					continue;
				if (get_lock_result[link] == 1)
					event_ops->bmgr_event_mld_critical_update_op_lock(pNetDev_per_link[link], BMGR_BPCC_OP_RESET_LOCK_TIMER, mld->mld_grp);
			}

			for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
				if (!pNetDev_per_link[link])
					continue;
				MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_INFO,
					"\x1b[42m MLD[%d]Band[%d] = %d Get lock ok,final_result=%d\x1b[m\n\r",
					mld->mld_grp, link, get_lock_result[link], final_result);
			}
		}

		if ((lock_flag == BMGR_BPCC_OP_GET_LOCK_ALL) && iob_upd_num != 0) {
			if (final_result == TRUE)
				goto mld_traversal;
		}
	} else if (lock_flag == BMGR_BPCC_OP_RELEASE_LOCK) {
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (!pNetDev_per_link[link])
				continue;
			event_ops->bmgr_event_mld_critical_update_op_lock(pNetDev_per_link[link], BMGR_BPCC_OP_RELEASE_LOCK, mld->mld_grp);
		}
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_INFO,
			"\x1b[42m unlock all, final_result = %d\x1b[m\n\r",
			final_result);
	} else if ((lock_flag == BMGR_BPCC_OP_MLD_ID_LOG_ON) ||
			(lock_flag == BMGR_BPCC_OP_MLD_ID_LOG_OFF)) {
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (!pNetDev_per_link[link])
				continue;
			event_ops->bmgr_event_mld_critical_update_op_lock(pNetDev_per_link[link], lock_flag, mld->mld_grp);
		}
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BPCC, DBG_LVL_INFO,
			"\x1b[42m sync mld id, final_result = %d\x1b[m\n\r",
			final_result);
	}

	return final_result;
}

void bss_mngr_con_mld_eml_op_update(IN PNET_DEV pNetDev, IN struct mld_eml_op *eml_op)
{
	u8 link, mld_bss_idx;
	bool valid_update = TRUE;
	u16 mld_sta_idx = eml_op->mld_sta_idx;
	struct bmgr_entry *entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_entry *mld_entry;
	struct bmgr_mlo_dev *mld = NULL;
	struct bmgr_mld_sta *bmgr_sta = NULL;
	struct bss_mngr_event_ops *event_ops = NULL;

	if (BMGR_VALID_MLD_STA(mld_sta_idx))
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);

	if (!BMGR_VALID_MLO_BSS_ENTRY(entry) || !bmgr_sta || !bmgr_sta->valid) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Invalid info: bss_entry:%p(valid:%d, mlo:%d), mld_sta:%p(valid:%d)\n",
			entry, entry ? entry->valid : 0,
			entry ? entry->mlo_en : 0,
			bmgr_sta, bmgr_sta ? bmgr_sta->valid : 0);
		return;
	}

	/* peer do not support EMLSR */
	if (!bmgr_sta->emlsr_sup)
		return;

	mld = entry->mld_ptr;
	if (!BMGR_VALID_MLO_DEV(mld)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Error, bss_idx (%d) in mld (%d) is invalid\n",
			entry->bss_idx, entry->entry_info.mld_grp);
		return;
	}

	if (!mld->attr.eml_caps.emlsr_supp) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Fail, AP MLD %d not support emlsr\n",
			mld->mld_grp);
		return;
	}

	if (eml_op->emlsr_en) {
		if (!eml_op->eml_link_bmap)
			valid_update = FALSE;
		for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
			if (eml_op->eml_link_bmap & BIT(link)) {
				mld_bss_idx = mld->bss_idx_mld[link];

				if (!BMGR_VALID_BSS_IDX(mld_bss_idx)) {
					valid_update = FALSE;
					break;
				}

				/* associated link && support EMLSR */
				mld_entry = GET_BSS_ENTRY_BY_IDX(mld_bss_idx);
				if (!BMGR_VALID_MLO_BSS_ENTRY(mld_entry) ||
					!bmgr_sta->mld_link[link].active) {
					valid_update = FALSE;
					break;
				}
			}
		}
	} else
		eml_op->eml_link_bmap = 0;

	if (valid_update) {
		/* update peer MLD info */
		bmgr_sta->emlsr_en = eml_op->emlsr_en;
		bmgr_sta->emlsr_link_bmap = eml_op->eml_link_bmap;

		eml_op->eml_omn_en = mld->attr.eml_caps.eml_omn_en;

		event_ops = entry->entry_info.event_ops;
		if (event_ops) {
			event_ops->bmgr_event_mld_eml_op_update(entry->pNetDev, eml_op);
		} else
			MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
				"Error: event_ops is not inited");
	} else {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Invalid notif: en(%d), link_bmap(%d)",
			eml_op->emlsr_en, eml_op->eml_link_bmap);
	}

}

static bool mld_link_recomm_sanity(
	IN u8 mld_grp_idx,
	IN struct mld_link_recomm *link_recomm
)
{
	u8 i, link;
	u16 mld_sta_num;
	struct bmgr_mld_sta *bmgr_sta;
	struct mld_recomm_sta_info *recomm_sta_info;

	if (!BMGR_VALID_MLD_GRP_IDX(mld_grp_idx))
		return FALSE;

	if (!link_recomm)
		return FALSE;

	mld_sta_num = link_recomm->mld_sta_num;
	if (!mld_sta_num) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Without any recommended MLD STA!\n");
		return FALSE;
	}

	recomm_sta_info = (struct mld_recomm_sta_info *)link_recomm->mld_sta_info;
	for (i = 0; i < link_recomm->mld_sta_num; i++, recomm_sta_info++) {
		if (!BMGR_VALID_MLD_STA(recomm_sta_info->mld_sta_idx)) {
			recomm_sta_info->valid = FALSE;
			mld_sta_num--;
			continue;
		}

		bmgr_sta = GET_MLD_STA_BY_IDX(recomm_sta_info->mld_sta_idx);
		if (!bmgr_sta->valid || (bmgr_sta->mld_grp_idx != mld_grp_idx)) {
			recomm_sta_info->valid = FALSE;
			mld_sta_num--;
			continue;
		}

		for (link = 0; link < (BSS_MNGR_MAX_LINK_ID + 1); link++) {
			if (!bmgr_sta->mld_link[link].active)
				recomm_sta_info->recomm_links &= ~(BIT(link));
		}

		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_INFO,
			"final recommend link 0x%x\n", recomm_sta_info->recomm_links);

		if (recomm_sta_info->recomm_links == 0) {
			recomm_sta_info->valid = FALSE;
			mld_sta_num--;
			continue;
		}
	}

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_NOTICE,
		"num of recommended MLD STA after sanity: %d\n", mld_sta_num);

	if (mld_sta_num == 0)
		return FALSE;

	return TRUE;
}

/*
 * Note that mld_recomm_sta_info pointed to by mld_sta_info in link_recomm
 * must be arranged in ascending order of AID
 */
void bss_mngr_con_mld_tx_link_recomm(
	IN u8 mld_grp_idx,
	IN u16 tx_link_bmap,
	IN struct mld_link_recomm *link_recomm)
{
	struct bmgr_mlo_dev *mld = NULL;
	u8 *f_buf = NULL, *pos;
	u8 link, bss_idx;
	u16 buf_len;
	struct bmgr_entry *entry;
	struct bss_mngr_event_ops *event_ops = bmgr_event_ops_get();

	if (!mld_link_recomm_sanity(mld_grp_idx, link_recomm)) {
		MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_ERROR,
			"Sanity failed!\n");
		return;
	}

	mld = GET_MLD_BY_GRP_IDX(mld_grp_idx);
	if (!BMGR_VALID_MLO_DEV(mld))
		return;

	/* 1. build generic frame body */

	/* alloc 1024 bytes buffer */
	os_alloc_mem(NULL, (u8 **)&f_buf, BSS_MNGR_MAX_INFO_LEN);
	if (!f_buf)
		goto err;
	NdisZeroMemory(f_buf, BSS_MNGR_MAX_INFO_LEN);
	pos = f_buf;

	/* build AID Bitmap IE */
	pos = build_aid_bitmap_ie(link_recomm, pos);

	/* build MLT IE */
	pos = build_link_recomm_multi_link_traffic_ie(link_recomm, pos);

	buf_len = (u16)(pos - f_buf);

	MTWF_DBG(NULL, DBG_CAT_MLO, CATMLO_BMGR, DBG_LVL_DEBUG,
		"buf_len %d\n", buf_len);

	if (!buf_len)
		goto err;

	/* 2. based on link, build frame header and transmit by bmgr_event */
	for (link = 0; link < BSS_MNGR_MAX_BAND_NUM; link++) {
		if (!(tx_link_bmap & BIT(link)))
			continue;

		bss_idx = mld->bss_idx_mld[link];
		if (!BMGR_VALID_BSS_IDX(bss_idx))
			continue;

		entry = GET_BSS_ENTRY_BY_IDX(bss_idx);
		if (!BMGR_VALID_MLO_BSS_ENTRY(entry))
			continue;

		event_ops->bmgr_event_mld_tx_link_recomm(entry->pNetDev,
			link_recomm->reason_code, f_buf, buf_len);
	}

err:
	if (f_buf)
		os_free_mem(f_buf);
}

#ifdef PRE_CFG_SUPPORT
u16 pre_cfg_mld_sta_add(
	IN PNET_DEV pNetDev,
	IN struct ml_ie_info *ml_info,
	IN UCHAR * setup_link_addr
)
{
	UINT16 mld_sta_idx;
	struct bmgr_entry *bss_entry = get_bss_entry_by_netdev(pNetDev);
	struct bmgr_mld_sta *bmgr_sta = NULL;

	mld_sta_idx = mld_sta_acquire(ml_info->mld_addr, SUBTYPE_AUTH, setup_link_addr);

	if (BMGR_VALID_MLO_BSS_ENTRY(bss_entry) && BMGR_VALID_MLD_STA(mld_sta_idx)) {
		bmgr_sta = GET_MLD_STA_BY_IDX(mld_sta_idx);
		mld_conn_sanity(bss_entry, ml_info, bmgr_sta);

		/* sanity check done */
		if (bmgr_sta->valid) {
			/* aid = mld_sta_idx + 1024 */
			bmgr_sta->aid = mld_sta_idx + START_MLD_STA_AID;
			bmgr_sta->BAOriTID = 0;
			bmgr_sta->BARecTID = 0;
			bmgr_sta->mld_link[0].str_bmap = 7;
			bmgr_sta->mld_link[1].str_bmap = 7;
			bmgr_sta->mld_link[2].str_bmap = 7;
			bmgr_sta->eml_caps = 0x1;
			bmgr_sta->emlsr_sup = 0x1;
			bss_mngr_con_add_mld_sta(bss_entry, bmgr_sta);
		}
	} else {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
			", invalid bss_entry or mld_sta.\n");
	}
	return mld_sta_idx;
}

#ifdef APCLI_SUPPORT
u16 pre_cfg_mld_ap_add(
	IN struct wifi_dev *wdev,
	IN struct ml_ie_info *ml_info
)
{
	struct mld_dev *mld = wdev->mld_dev;
	struct mld_link_entry *set_up_link = NULL;
	struct peer_mld_entry *peer = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	uint8_t i = 0, link_num = 0, flag_once = 0;
	uint16_t setup_wcid = 0;

	set_up_link = get_sta_mld_link_by_wdev(mld, wdev);

	if (!set_up_link) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
					", invalid set_up_link.\n");
		return -1;
	}

	/* 1. disconnect current connection*/
	if (mld->peer_mld.valid)
		sta_mld_disconn_req(wdev);

	/* 3. parse mld ie*/
	ml_info->eml_caps = 0x1;
	ml_info->emlsr_sup = 0x1;
	ml_info->mld_caps = 0x1;

	set_up_link->is_setup_link = 1;

	peer = &mld->peer_mld;
	/* 4. fill peer mld info*/
	COPY_MAC_ADDR(peer->mld_addr, ml_info->mld_addr);
	peer->valid = 1;
	peer->idx = PEER_AP_MLD_IDX;
	peer->aid = peer->idx + START_MLD_STA_AID;
	peer->set_up_link = set_up_link;
	peer->BAOriTID = 0;
	peer->BARecTID = 0;
	/*first insert set_up_link.*/
	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				", [%d].active: %d\n",
				i, ml_info->link[i].active);
		if (ml_info->link[i].active && set_up_link == &mld->mld_own_links[i]) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				"%s, MLD insert link(set_up_link): MacAddr = "MACSTR", i = %d, active = %d!\n", __func__,
				MAC2STR(ml_info->link[i].link_addr), i, ml_info->link[i].active);

			sta_mld_insert_link(mld, peer, ml_info, i, i, &setup_wcid, set_up_link, &link_num);
		}
	}
	/*insert other links*/
	for (i = 0; i < BSS_MNGR_MAX_BAND_NUM; i++) {
		MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				", [%d].active: %d\n",
				i, ml_info->link[i].active);
		if (set_up_link == &mld->mld_own_links[i])
			continue;

		if (ml_info->link[i].active) {
			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				", MLD insert link: MacAddr = "MACSTR", i = %d, active = %d!\n",
				MAC2STR(ml_info->link[i].link_addr), i, ml_info->link[i].active);

			MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
				", wdev = %p\n", mld->mld_own_links[i].wdev);

			sta_mld_insert_link(mld, peer, ml_info, i, i, &setup_wcid, set_up_link, &link_num);
		}
	}

	/* 6. fill mlo info for each mac table entry*/
	for (i = 0; i < MLD_LINK_MAX; i++) {
		pEntry = (MAC_TABLE_ENTRY *)peer->single_link[i].priv_ptr;
		if (pEntry) {
			struct mld_entry_t *mld_entry = NULL, *new_mld_entry = NULL;
			struct mld_entry_ctrl_t *mld_entry_ctrl = mld_entry_ctrl_get();
			struct hlist_head *head = NULL;
			u8 hash_idx = MAC_ADDR_HASH_INDEX(peer->mld_addr);

			spin_lock_bh(&mld_entry_ctrl->mld_entry_lock);
			mld_entry = get_mld_entry_by_mac(peer->mld_addr);
			if (mld_entry) {
				new_mld_entry = create_mld_entry();
				if (new_mld_entry) {
					update_mld_entry_apcli(mld_entry, new_mld_entry, wdev, i, link_num, &flag_once, setup_wcid);
					hlist_replace_rcu(&mld_entry->hlist, &new_mld_entry->hlist);
					update_link_mld_entry(new_mld_entry);
					mt_call_rcu(&mld_entry->rcu, free_mld_entry);
				} else
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						" create mld_entry Fail!\n");
			} else {
				mld_entry = create_mld_entry();
				if (mld_entry) {
					update_mld_entry_apcli(NULL, mld_entry, wdev, i, link_num, &flag_once, setup_wcid);
					head = &mld_entry_ctrl->mld_hash[hash_idx];
					hlist_add_head_rcu(&mld_entry->hlist, head);
					update_link_mld_entry(mld_entry);
				} else
					MTWF_DBG(NULL, DBG_CAT_TEST, CATTEST_PRECFG, DBG_LVL_ERROR,
						" create mld_entry Fail!\n");
			}
			spin_unlock_bh(&mld_entry_ctrl->mld_entry_lock);

			fill_peer_mlo_info(peer, link_num, pEntry->wdev == set_up_link->wdev ? 1 : 0,
				setup_wcid, &pEntry->mlo);

			/*dump mlo information in mac table entry*/
			bss_mngr_peer_integrity_check(pEntry);
		}
	}

	mld->peer_mld.valid = 1;

	return 0;
}
#endif /* APCLI_SUPPORT */
#endif /* PRE_CFG_SUPPORT */

#endif /* DOT11_EHT_BE */

struct bss_mngr_ops bmgr_ops = {
	.bmgr_init = bss_mngr_con_init,
	.bmgr_deinit = bss_mngr_con_deinit,
	.bmgr_bss_reg = bss_mngr_con_dev_reg,
	.bmgr_bss_dereg = bss_mngr_con_dev_dereg,
	.bmgr_bss_update = bss_mngr_con_dev_update,
	.bmgr_show_info = bss_mngr_con_info_show,
	.bmgr_ie_update = bss_mngr_con_ie_update,
	.bmgr_sync_bcn_update = bss_mngr_con_sync_bcn_update,
	.bmgr_mld_get_pad_by_band_idx = bss_mngr_get_pad_by_band_idx,
#ifdef DOT11V_MBSSID_SUPPORT
	.bmgr_query_mbss_info = bss_mngr_con_query_mbss_ie,
#endif
	.bmgr_query_rnr_info = bss_mngr_con_query_rnr_ie,
	.bmgr_query_is_repted_ssid = bss_mngr_con_query_is_repted_ssid,
	.bmgr_query_is_mld_wdev = bss_mngr_con_query_is_mld_enabled,
	.bmgr_query_is_in_mlo_csa = bss_mngr_con_query_is_in_mlo_csa,
#ifdef CONFIG_6G_SUPPORT
	.bmgr_update_oob_6g_disc = bss_mngr_con_update_oob_6g_disc,
#endif
	.bmgr_query_netdev_by_band = bss_mngr_con_query_netdev_by_band,
#ifdef DOT11_EHT_BE
	.bmgr_mld_group_create = bss_mngr_con_mld_group_create,
	.bmgr_mld_group_destroy = bss_mngr_con_mld_group_destroy,
	.bmgr_mld_group_attr_set = bss_mngr_con_mld_group_attr_set,
	.bmgr_mld_group_add_link = bss_mngr_con_mld_group_add_link,
	.bmgr_mld_group_del_link = bss_mngr_con_mld_group_del_link,
	.bmgr_query_mld_info = bss_mngr_con_query_mld_info,
	.bmgr_query_tid_to_link_info = bss_mngr_con_query_tid_to_link_map_ie,
	.bmgr_query_multi_link_traffic_ie = bss_mngr_con_query_multi_link_traffic_ie,
	.bmgr_mld_reconfig_ie_op = bss_mngr_con_mld_reconfig_ie_op,
	.bmgr_mld_query_netdev_by_link = bss_mngr_con_mld_query_netdev_by_link,
	.bmgr_mld_conn_req = bss_mngr_con_mld_conn_req,
	.bmgr_mld_conn_act = bss_mngr_con_mld_conn_act,
	.bmgr_mld_disconn_op = bss_mngr_con_mld_disconn_op,
	.bmgr_mld_add_sta_profile = bss_mngr_con_mld_add_sta_profile,
	.bmgr_mld_remove_sta_profile = bss_mngr_con_mld_remove_sta_profile,
	.bmgr_mld_sync_ml_probe_rsp_per_sta_profile = bss_mngr_con_sync_ml_probe_rsp_per_sta_profile,
	.bmgr_mld_ba_ori_setup = bss_mngr_con_mld_ba_ori_setup,
	.bmgr_mld_ba_resrc_ori_add = bss_mngr_con_mld_ba_resrc_ori_add,
	.bmgr_mld_ba_resrc_rec_add = bss_mngr_con_mld_ba_resrc_rec_add,
	.bmgr_mld_ba_resrc_ori_del = bss_mngr_con_mld_ba_resrc_ori_del,
	.bmgr_mld_ba_resrc_rec_del = bss_mngr_con_mld_ba_resrc_rec_del,
	.bmgr_mld_ba_add_to_asic = bss_mngr_con_mld_ba_add_to_asic,
	.bmgr_mld_ba_del_from_asic = bss_mngr_con_mld_ba_del_from_asic,
	.bmgr_mld_forward_action_to_link = bss_mngr_con_mld_twt_action_frm_forward,
	.bmgr_mld_breakdown_action_frame = bss_mngr_con_mld_twt_action_frm_breadown,
	.bmgr_mld_aggregate_action_frame = bss_mngr_con_mld_twt_action_frm_agg,
	.bmgr_mld_twt_wait_ack_inform = bss_mngr_con_mld_twt_wait_ack_inform,
#ifdef MWDS
	.bmgr_mld_mwds_cap_sync = bss_mngr_con_mwds_cap_sync,
#endif
	.bmgr_mld_critical_update_trigger = bss_mngr_con_mld_critical_update,
	.bmgr_mld_critical_update_op_lock = bss_mngr_con_mld_critical_update_op_lock,
	.bmgr_mld_eml_op_update = bss_mngr_con_mld_eml_op_update,
	.bmgr_mld_bss_linkup = bss_mngr_con_mld_bss_linkup,
	.bmgr_mld_bss_linkdown = bss_mngr_con_mld_bss_linkdown,
	.bmgr_mld_bmc_sn_qry = bss_mngr_con_mld_bmc_sn_qry,
	.bmgr_mld_tx_link_recomm = bss_mngr_con_mld_tx_link_recomm,
	.bmgr_mld_reconfig_peer_mld = bss_mngr_con_mld_reconfig_peer_mld,
#endif
};

