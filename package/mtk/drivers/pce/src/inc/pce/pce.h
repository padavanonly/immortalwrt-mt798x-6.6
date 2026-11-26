/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023 Mediatek Inc. All Rights Reserved.
 *
 * Author: Ren-Ting Wang <ren-ting.wang@mediatek.com>
 */

#ifndef _MTK_PCE_H_
#define _MTK_PCE_H_

/* GLO_MEM available control max index */
#define FE_MEM_DATA_WLEN			(10)
#define FE_MEM_TS_CONFIG_MAX_INDEX		(16)
#define FE_MEM_DIPFILTER_MAX_IDX		(16)
#define FE_MEM_CLS_MAX_INDEX			(32)
#define FE_MEM_CDRT_MAX_INDEX			(96)

enum fe_mem_cmd {
	FE_MEM_CMD_NO_OP,
	FE_MEM_CMD_WRITE,
	FE_MEM_CMD_READ,

	__FE_MEM_CMD_MAX,
};

enum fe_mem_type {
	FE_MEM_TYPE_TS_CONFIG,
	FE_MEM_TYPE_DIPFILTER,
	FE_MEM_TYPE_CLS,
	FE_MEM_TYPE_CDRT,

	__FE_MEM_TYPE_MAX,
};

#define CLS_DESC_TAG_MATCH_L4_HDR		(BIT(0))
/* match the data follow behind L4 header */
#define CLS_DESC_TAG_MATCH_L4_USR		(BIT(1))
/* seems like valid bit0 and bit 1 must both be set? */
#define CLS_DESC_VALID_UPPER_HALF_WORD_BIT	(BIT(0))
#define CLS_DESC_VALID_LOWER_HALF_WORD_BIT	(BIT(1))
#define CLS_DESC_VALID_DPORT_BIT		(BIT(2))

#define CLS_DESC_TAG_MASK			(GENMASK(1, 0))
#define CLS_DESC_SPORT_MASK			(GENMASK(15, 0))
#define CLS_DESC_UDPLITE_L4_HDR_NEZ_MASK	(GENMASK(0, 0))
#define CLS_DESC_DIP_MATCH			(GENMASK(0, 0))
#define CLS_DESC_L4_TYPE_MASK			(GENMASK(7, 0))
#define CLS_DESC_L4_VALID_MASK			(GENMASK(2, 0))
#define CLS_DESC_L4_DPORT_MASK			(GENMASK(15, 0))
#define CLS_DESC_FPORT_MASK			(GENMASK(3, 0))
#define CLS_DESC_DR_IDX_MASK			(GENMASK(1, 0))
#define CLS_DESC_QID_MASK			(GENMASK(7, 0))
#define CLS_DESC_CDRT_MASK			(GENMASK(7, 0))
#define CLS_DESC_TPORT_MASK			(GENMASK(3, 0))
#define CLS_DESC_TOPS_ENTRY_MASK		(GENMASK(5, 0))

struct cls_desc {
	/* action */
	u32 fport		: 4;
	u32 dr_idx		: 2;
	u32 qid			: 8;
	u32 cdrt_idx		: 8;
	u32 tport_idx		: 4;
	u32 tops_entry		: 6; /* no effect on r/w */
	/* data */
	u32 l4_hdr_usr_data	: 32;
	u32 l4_dport		: 16;
	u32 l4_valid		: 3;
	u32 l4_type		: 8;
	u32 dip_match		: 1;
	u32 l4_udp_hdr_nez	: 1;
	u32 sport		: 16;
	u32 tag			: 2;
	/* mask */
	u32 l4_hdr_usr_data_m	: 32;
	u32 l4_dport_m		: 16;
	u32 l4_valid_m		: 3;
	u32 l4_type_m		: 8;
	u32 dip_match_m		: 1;
	u32 l4_udp_hdr_nez_m	: 1;
	u32 sport_m		: 16;
	u32 tag_m		: 2;
} __packed;

struct cdrt_non_extend {
	u32 pkt_len		: 16;
	u32 ap			: 1;
	u32 options		: 13;
	u32 type		: 2;
};

struct cdrt_ipsec_extend {
	u32 pkt_len		: 16;
	u32 rsv1		: 9;
	u32 enc_last_dest	: 1;
	u32 rsv2		: 4;
	u32 type		: 2;
};

struct cdrt_dtls_extend {
	u32 pkt_len		: 16;
	u32 rsv1		: 5;
	u32 mask_size		: 3;
	u32 rsv2		: 2;
	u32 capwap		: 1;
	u32 dir			: 1;
	u32 content_type	: 2;
	u32 type		: 2;
};

struct cdrt_desc {
	union {
		struct {
			union {
				struct cdrt_non_extend common;
				struct cdrt_ipsec_extend ipsec;
				struct cdrt_dtls_extend dtls;
			};
			u32 aad_len		: 8;
			u32 rsv1		: 1;
			u32 app_id		: 7;
			u32 token_len		: 8;
			u32 rsv2		: 8;
			u32 p_tr[2];
		} desc1 __packed;
		u32 raw1[4];
	};

	union {
		struct {
			u32 usr			: 16;
			u32 rsv1		: 6;
			u32 strip_pad		: 1;
			u32 allow_pad		: 1;
			u32 hw_srv		: 6;
			u32 rsv2		: 1;
			u32 flow_lookup		: 1;
			u32 rsv3		: 8;
			u32 ofs			: 8;
			u32 next_hdr		: 8;
			u32 fl			: 1;
			u32 ip4_chksum		: 1;
			u32 l4_chksum		: 1;
			u32 parse_eth		: 1;
			u32 keep_outer		: 1;
			u32 rsv4		: 3;
			u32 rsv5[2];
		} desc2 __packed;
		u32 raw2[4];
	};

	union {
		struct {
			u32 option_meta[4];
		} desc3 __packed;
		u32 raw3[4];
	};
} __packed;

enum dipfilter_tag {
	DIPFILTER_DISABLED,
	DIPFILTER_IPV4,
	DIPFILTER_IPV6,

	__DIPFILTER_TAG_MAX,
};

struct dip_desc {
	union {
		u32 ipv4;
		u32 ipv6[4];
	};
	u32 tag			: 2; /* enum dipfilter_tag */
} __packed;

struct tsc_desc {
	u32 tport		: 4;
	u32 cdrt_idx		: 8;
	u32 tops_entry		: 6;
	u32 tport_map_lower	: 32;
	u32 tport_map_upper	: 32;
} __packed;

struct fe_mem_msg {
	enum fe_mem_cmd cmd;
	enum fe_mem_type type;
	u32 index;

	union {
		struct cls_desc cdesc;
		struct dip_desc ddesc;
		struct tsc_desc tdesc;
		u32 raw[FE_MEM_DATA_WLEN];
	};
};

static inline void mtk_pce_fe_mem_msg_config(struct fe_mem_msg *msg,
					     enum fe_mem_cmd cmd,
					     enum fe_mem_type type,
					     u32 index)
{
	if (!msg)
		return;

	msg->cmd = cmd;
	msg->type = type;
	msg->index = index;
}

int mtk_pce_enable(void);
void mtk_pce_disable(void);

int mtk_pce_fe_mem_msg_send(struct fe_mem_msg *msg);
#endif /* _MTK_PCE_H_ */
