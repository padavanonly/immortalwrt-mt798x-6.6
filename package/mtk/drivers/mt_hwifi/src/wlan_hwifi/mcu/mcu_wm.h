/*
 * Copyright (c) [2020] MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ""AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MTK_MCU_WM_H
#define __MTK_MCU_WM_H

extern const struct mtk_mcu_ops mtk_wm_ops;

#define MCU_PQ_ID(p, q)			(((p) << 15) | ((q) << 10))
#define MCU_PKT_ID				0xa0
#define MCU_PATCH_ADDRESS		0x200000

#define FW_FEATURE_SET_ENCRYPT		BIT(0)
#define FW_FEATURE_SET_KEY_IDX		GENMASK(2, 1)
#define FW_FEATURE_OVERRIDE_ADDR	BIT(5)

#define DL_MODE_ENCRYPT				BIT(0)
#define DL_MODE_KEY_IDX				GENMASK(2, 1)
#define DL_MODE_RESET_SEC_IV		BIT(3)
#define DL_MODE_WORKING_PDA_CR4		BIT(4)
#define DL_MODE_VALID_RAM_ENTRY		BIT(5)
#define DL_MODE_ENCRY_MODE_SEL		BIT(6) /* 0 - AES, 1 - SCRAMBLE */
#define DL_MODE_ENCRY_MODE_EMI		BIT(7)
#define DL_MODE_NEED_RSP			BIT(31)

#define FW_START_OVERRIDE			BIT(0)
#define FW_START_WORKING_PDA_CR4	BIT(2)
#define FW_START_WORKING_PDA_DSP	BIT(3)


#define PATCH_SEC_TYPE_MASK			GENMASK(15, 0)
#define PATCH_SEC_TYPE_INFO			0x2

#define PATCH_SECINFO_NOT_SUPPORT		(0xFFFFFFFF)

#define PATCH_SECINFO_ENC_TYPE_MASK		GENMASK(31, 24)
#define PATCH_SECINFO_ENC_SCRAMBLE_KEY_MASK	GENMASK(15, 8)
#define PATCH_SECINFO_ENC_SCRAMBLE_CR_MASK	GENMASK(7, 0)
#define PATCH_SECINFO_ENC_AES_KEY_MASK		GENMASK(7, 0)

#define PACH_MAX_N_SECTION		0xFFFFFFFF
#define PACH_MAX_LEN			0xFFFFFFFF

/**
 * enum mcu_wm_opts - WM options
 * @WM_OPT_BYPASS_WA: wm mcu use for decision send by pass wa command or not at fwdl.
 *
 * These are wm mcu options
 */
enum mcu_wm_opts {
	WM_OPT_BYPASS_WA,
	WM_OPT_PATCH_DL,
	WM_OPT_UNICMD_NOT_SUPP,
	WM_OPT_CK_FWDL_STA,
	WM_OPT_MAX,
};

/**
 * enum mcu_wm_cmd_action - WM command actions
 *
 * @MCU_Q_QUERY: action query, command should have response for query result.
 * @MCU_Q_SET: action set
 * @MCU_Q_RESERVED: reserved
 * @MCU_Q_NA: action none
 */
enum mcu_wm_cmd_action {
	MCU_Q_QUERY,
	MCU_Q_SET,
	MCU_Q_RESERVED,
	MCU_Q_NA
};

/**
 * enum mcu_wm_s2d - WM source and destination
 *
 * @MCU_S2D_H2N: source is host, destination is WM
 * @MCU_S2D_C2N: source is WA, destination is WM
 * @MCU_S2D_H2C: source is host, destination is WA
 * @MCU_S2D_H2CN: source is host, destination is WA & WM
 */
enum mcu_wm_s2d {
	MCU_S2D_H2N,
	MCU_S2D_C2N,
	MCU_S2D_H2C,
	MCU_S2D_H2CN
};

/**
 * enum mcu_wm_cmd - WM command idx
 *
 * @MCU_CMD_TARGET_ADDRESS_LEN_REQ:
 * @MCU_CMD_FW_START_REQ:
 * @MCU_CMD_INIT_ACCESS_REG:
 * @MCU_CMD_NIC_POWER_CTRL:
 * @MCU_CMD_PATCH_START_REQ:
 * @MCU_CMD_PATCH_FINISH_REQ:
 * @MCU_CMD_PATCH_SEM_CONTROL:
 * @MCU_CMD_WA:
 * @MCU_CMD_EXT_CID:
 * @MCU_CMD_FW_SCATTER:
 * @MCU_CMD_RESTART_DL_REQ:
 */
enum mcu_wm_cmd {
	MCU_CMD_TARGET_ADDRESS_LEN_REQ = 0x01,
	MCU_CMD_FW_START_REQ = 0x02,
	MCU_CMD_INIT_ACCESS_REG = 0x3,
	MCU_CMD_NIC_POWER_CTRL = 0x4,
	MCU_CMD_PATCH_START_REQ = 0x05,
	MCU_CMD_PATCH_FINISH_REQ = 0x07,
	MCU_CMD_PATCH_SEM_CONTROL = 0x10,
	MCU_CMD_QUERY_INFO = 0x22,
	MCU_CMD_EMI_FW_DOWNLOAD_CONFIG = 0x23,
	MCU_CMD_EMI_FW_TRIGGER_AXI_DMA = 0x24,
	MCU_CMD_WA = 0xC4,
	MCU_CMD_EXT_CID = 0xED,
	MCU_CMD_FW_SCATTER = 0xEE,
	MCU_CMD_RESTART_DL_REQ = 0xEF,
};

/**
 * enum mcu_wm_ext_cmd - WM extend command index
 *
 * @MCU_EXT_CMD_EFUSE_ACCESS:
 * @MCU_EXT_CMD_FW_LOG_2_HOST:
 * @MCU_EXT_CMD_EFUSE_BUFFER_MODE:
 * @MCU_EXT_CMD_THERMAL_CTRL:
 * @MCU_EXT_CMD_DUMP_MEM:
 * @MCU_EXT_CMD_SET_SER_TRIGGER:
 * @MCU_EXT_CMD_FW_DBG_CTRL:
 * @MCU_EXT_CMD_AUTO_BA:
 * @MCU_EXT_CMD_MCAST_CLONE:
 * @MCU_EXT_CMD_MULTICAST_ENTRY_INSERT:
 * @MCU_EXT_CMD_MULTICAST_ENTRY_DELETE:
 * @MCU_EXT_CMD_SET_DSCP_PRI:
 * @MCU_EXT_CMD_RED_ENABLE:
 * @MCU_EXT_CMD_RED_SHOW_STA:
 * @MCU_EXT_CMD_RED_TARGET_DELAY:
 * @MCU_EXT_CMD_RED_TX_RPT:
 * @MCU_EXT_CMD_PKT_BUDGET_CTRL_CFG:
 */
enum mcu_wm_ext_cmd {
	MCU_EXT_CMD_EFUSE_ACCESS = 0x01,
	MCU_EXT_CMD_FW_LOG_2_HOST = 0x13,
	MCU_EXT_CMD_EFUSE_BUFFER_MODE = 0x21,
	MCU_EXT_CMD_THERMAL_CTRL = 0x2c,
	MCU_EXT_CMD_DUMP_MEM = 0x57,
	MCU_EXT_CMD_SET_SER_TRIGGER = 0x81,
	MCU_EXT_CMD_FW_DBG_CTRL = 0x95,
	/*WA part*/
	MCU_EXT_CMD_AUTO_BA = 0x51,
	MCU_EXT_CMD_MCAST_CLONE = 0x52,
	MCU_EXT_CMD_MULTICAST_ENTRY_INSERT = 0x53,
	MCU_EXT_CMD_MULTICAST_ENTRY_DELETE = 0x54,
	MCU_EXT_CMD_SET_DSCP_PRI = 0xB4,
	MCU_EXT_CMD_RED_ENABLE = 0x68,
	MCU_EXT_CMD_RED_SHOW_STA = 0x69,
	MCU_EXT_CMD_RED_TARGET_DELAY = 0x6A,
	MCU_EXT_CMD_RED_TX_RPT = 0x6B,
	MCU_EXT_CMD_PKT_BUDGET_CTRL_CFG = 0x6C,
};

/**
 * enum mcu_uni_cmd_opt - WM unify command action
 *
 * @MCU_UNI_CMD_OPT_BIT_0_ACK:
 * @MCU_UNI_CMD_OPT_BIT_1_UNI_CMD: 1 is unify command, 0 is original command
 * @MCU_UNI_CMD_OPT_BIT_2_SET_QUERY: 1 is set, 0 is query
 */
enum mcu_uni_cmd_opt {
	MCU_UNI_CMD_OPT_BIT_0_ACK = BIT(0),
	MCU_UNI_CMD_OPT_BIT_1_UNI_CMD = BIT(1),
	MCU_UNI_CMD_OPT_BIT_2_SET_QUERY = BIT(2),
};

/**
 * enum mcu_uni_event_opt - WM unify event action
 *
 * @MCU_UNI_CMD_OPT_BIT_1_UNI_EVENT:
 * @MCU_UNI_CMD_OPT_BIT_2_UNSOLICITED_EVENT:
 */
enum mcu_uni_event_opt {
	MCU_UNI_CMD_OPT_BIT_1_UNI_EVENT = BIT(1),
	MCU_UNI_CMD_OPT_BIT_2_UNSOLICITED_EVENT = BIT(2)
};

/**
 * enum mcu_patch_sem_action - WM patch semaphore action
 *
 * @PATCH_SEM_RELEASE: release semaphore
 * @PATCH_SEM_GET: get semaphore
 */
enum mcu_patch_sem_action {
	PATCH_SEM_RELEASE,
	PATCH_SEM_GET
};

/**
 * enum mcu_patch_state - WM patch download state
 *
 * @PATCH_NOT_DL_SEM_FAIL:
 * @PATCH_IS_DL:
 * @PATCH_NOT_DL_SEM_SUCCESS:
 * @PATCH_REL_SEM_SUCCESS:
 */
enum mcu_patch_state {
	PATCH_NOT_DL_SEM_FAIL,
	PATCH_IS_DL,
	PATCH_NOT_DL_SEM_SUCCESS,
	PATCH_REL_SEM_SUCCESS
};

/**
 * enum mcu_fwdl_state - WM ram download state
 *
 * @FW_STATE_INITIAL:
 * @FW_STATE_FW_DOWNLOAD:
 * @FW_STATE_NORMAL_OPERATION:
 * @FW_STATE_NORMAL_TRX:
 * @FW_STATE_WACPU_RDY:
 */
enum mcu_fwdl_state {
	FW_STATE_INITIAL,
	FW_STATE_FW_DOWNLOAD,
	FW_STATE_NORMAL_OPERATION,
	FW_STATE_NORMAL_TRX       = 3,
	FW_STATE_WACPU_RDY        = 7
};

/**
 * enum mcu_event_id - WM event index
 *
 * @MCU_EVENT_TARGET_ADDRESS_LEN:
 * @MCU_EVENT_FW_START:
 * @MCU_EVENT_GENERIC:
 * @MCU_EVENT_ACCESS_REG:
 * @MCU_EVENT_MT_PATCH_SEM:
 * @MCU_EVENT_CH_PRIVILEGE:
 * @MCU_EVENT_EXT:
 * @MCU_EVENT_RESTART_DL:
 */
enum mcu_event_id {
	MCU_EVENT_TARGET_ADDRESS_LEN = 0x01,
	MCU_EVENT_FW_START = 0x01,
	MCU_EVENT_GENERIC = 0x01,
	MCU_EVENT_ACCESS_REG = 0x02,
	MCU_EVENT_MT_PATCH_SEM = 0x04,
	MCU_EVENT_CH_PRIVILEGE = 0x18,
	MCU_EVENT_EXT = 0xed,
	MCU_EVENT_RESTART_DL = 0xef,
};

/**
 * enum mcu_ext_event_id - WM extend event index
 *
 * @MCU_EXT_EVENT_PS_SYNC:
 * @MCU_EXT_EVENT_FW_LOG_2_HOST:
 * @MCU_EXT_EVENT_THERMAL_PROTECT:
 * @MCU_EXT_EVENT_ASSERT_DUMP:
 * @MCU_EXT_EVENT_RDD_REPORT:
 * @MCU_EXT_EVENT_CSA_NOTIFY:
 * @MCU_EXT_EVENT_HE_RA_CTRL:
 */
enum mcu_ext_event_id {
	MCU_EXT_EVENT_PS_SYNC = 0x5,
	MCU_EXT_EVENT_FW_LOG_2_HOST = 0x13,
	MCU_EXT_EVENT_THERMAL_PROTECT = 0x22,
	MCU_EXT_EVENT_ASSERT_DUMP = 0x23,
	MCU_EXT_EVENT_RDD_REPORT = 0x3a,
	MCU_EXT_EVENT_CSA_NOTIFY = 0x4f,
	MCU_EXT_EVENT_HE_RA_CTRL = 0x87,
};

/**
 * enum mcu_uni_event_id - WM unify event index
 *
 * @MCU_UNI_EVENT_ID_CMD_RESULT:
 * @MCU_UNI_EVENT_ID_BMC_RPY_DT:
 * @MCU_UNI_EVENT_ID_HIF_CTRL:
 * @MCU_UNI_EVENT_ID_FW_LOG_2_HOST:
 * @MCU_UNI_EVENT_ID_CHIP_CONFIG:
 * @MCU_UNI_EVENT_ID_SMESH_INFO:
 * @MCU_UNI_EVENT_ID_IE_COUNTDOWN:
 * @MCU_UNI_EVENT_ID_ASSERT_DUMP:
 * @MCU_UNI_EVENT_ID_SLEEPY_NOTIFY:
 * @MCU_UNI_EVENT_ID_BEACON_TIMEOUT:
 * @MCU_UNI_EVENT_ID_PS_SYNC:
 * @MCU_UNI_EVENT_ID_SCAN_DONE:
 * @MCU_UNI_EVENT_ID_ECC_CAL:
 * @MCU_UNI_EVENT_ID_ADD_KEY_DONE:
 * @MCU_UNI_EVENT_ID_OBSS_UPDATE:
 * @MCU_UNI_EVENT_ID_SER:
 * @MCU_UNI_EVENT_ID_MAC_INFO:
 * @MCU_UNI_EVENT_ID_TDLS:
 * @MCU_UNI_EVENT_ID_SAP:
 */
enum mcu_uni_event_id {
	MCU_UNI_EVENT_ID_CMD_RESULT      = 0x01,
	MCU_UNI_EVENT_ID_BMC_RPY_DT      = 0x02,
	MCU_UNI_EVENT_ID_HIF_CTRL        = 0x03,
	MCU_UNI_EVENT_ID_FW_LOG_2_HOST   = 0x04,
	MCU_UNI_EVENT_ID_CHIP_CONFIG     = 0x07,
	MCU_UNI_EVENT_ID_SMESH_INFO      = 0x08,
	MCU_UNI_EVENT_ID_IE_COUNTDOWN    = 0x09,
	MCU_UNI_EVENT_ID_ASSERT_DUMP     = 0x0A,
	MCU_UNI_EVENT_ID_SLEEPY_NOTIFY   = 0x0b,
	MCU_UNI_EVENT_ID_BEACON_TIMEOUT  = 0x0C,
	MCU_UNI_EVENT_ID_PS_SYNC         = 0x0D,
	MCU_UNI_EVENT_ID_SCAN_DONE       = 0x0E,
	MCU_UNI_EVENT_ID_ECC_CAL         = 0x10,
	MCU_UNI_EVENT_ID_ADD_KEY_DONE    = 0x12,
	MCU_UNI_EVENT_ID_OBSS_UPDATE     = 0x13,
	MCU_UNI_EVENT_ID_SER             = 0x14,
	MCU_UNI_EVENT_ID_MAC_INFO        = 0x1A,
	MCU_UNI_EVENT_ID_TDLS            = 0x1B,
	MCU_UNI_EVENT_ID_SAP             = 0x1C
};

/**
 * enum mcu_fwdl_type - WM FWDL type
 *
 * @MCU_FWDL_TYPE_ROM_PATCH:
 * @MCU_FWDL_TYPE_RAM:
 * @MCU_FWDL_TYPE_EMI_CONFIG:
 * @MCU_FWDL_TYPE_EMI_TRIGGER_AXI:
 */
enum mcu_fwdl_type {
	MCU_FWDL_TYPE_ROM_PATCH,
	MCU_FWDL_TYPE_RAM,
	MCU_FWDL_TYPE_EMI_CONFIG,
	MCU_FWDL_TYPE_EMI_TRIGGER_AXI
};


/**
 * enum patch_secinfo_enc_type - Patch encrpytion type
 *
 * @PATCH_SECINFO_ENC_TYPE_PLAIN:
 * @PATCH_SECINFO_ENC_TYPE_AES:
 * @PATCH_SECINFO_ENC_TYPE_SCRAMBLE:
 * @PATCH_SECINFO_ENC_TYPE_MAX
 */
enum patch_secinfo_enc_type {
	PATCH_SECINFO_ENC_TYPE_PLAIN,
	PATCH_SECINFO_ENC_TYPE_AES,
	PATCH_SECINFO_ENC_TYPE_SCRAMBLE,
	PATCH_SECINFO_ENC_TYPE_MAX
};


/**
 * struct mcu_txd - mcu tx header definition
 *
 * This structure contains wm/wa mcu tx header information
 *
 * @txd: bus tx description
 * @len: length of header and payload
 * @pq_id: port queue idx for indicated destination, see @enum tx_mcu_port_q_idx
 * @cid: indicate command idx, see @enum mcu_wm_cmd
 * @pkt_type: packet type, for mcu tx should be MT_TX_TYPE_CMD (2), see @enum tx_pkt_type
 * @set_query: indicate this command's action is set or query, see @enum mcu_wm_cmd_action
 * @seq: indicate command sequence number
 * @uc_d2b0_rev: not nused (reserved)
 * @ext_cid: indicate extend command idx, see @enum mcu_wm_ext_cmd
 * @s2d_index:indicate source and destination, see @enum mcu_wm_s2d
 * @ext_cid_ack:indicate this extend command need to wait for response
 * @reserved: reserved field
 */
struct mcu_txd {
	__le32 txd[8];

	__le16 len;
	__le16 pq_id;

	u8 cid;
	u8 pkt_type;
	u8 set_query;
	u8 seq;

	u8 uc_d2b0_rev;
	u8 ext_cid;
	u8 s2d_index;
	u8 ext_cid_ack;

	u32 reserved[5];
} __packed __aligned(4);

/**
 * struct mcu_txd_uni_cmd - WM unify command tx header format
 *
 * @txd:
 * @len:
 * @cid:
 * @reserved_1:
 * @pkt_type:
 * @frag:
 * @seq:
 * @check_sum:
 * @s2d_index:
 * @option:
 * @reserved_2:
 */
struct mcu_txd_uni_cmd {
	__le32 txd[8];
	/* DW 0 */
	__le16 len;
	__le16 cid;
	/* DW 1 */
	u8 reserved_1;
	u8 pkt_type;
	u8 frag;
	u8 seq;
	/* DW 2 */
	__le16 check_sum;
	u8 s2d_index;
	u8 option;
	/* DW 3 */
	u8 reserved_2[4];
} __packed __aligned(4);

/**
 * struct mcu_rxd - WM rx event header format
 *
 * @len:
 * @pkt_type_id:
 * @eid:
 * @seq:
 * @option:
 * @__rsv:
 * @ext_eid:
 * @__rsv1:
 * @s2d_index:
 */
struct mcu_rxd {
	__le16 len;
	__le16 pkt_type_id;

	u8 eid;
	u8 seq;
	u8 option;
	u8 __rsv;

	u8 ext_eid;
	u8 __rsv1[2];
	u8 s2d_index;
};

/**
 * struct mcu_patch_hdr - WM patch binary header information
 *
 */
struct mcu_patch_hdr {
	char build_date[16];
	char platform[4];
	__be32 hw_sw_ver;
	__be32 patch_ver;
	__be16 checksum;
	u16 reserved;
	struct {
		__be32 patch_ver;
		__be32 subsys;
		__be32 feature;
		__be32 n_region;
		__be32 crc;
		u32 reserved[11];
	} desc;
} __packed;

/**
 * struct mcu_patch_sec - WM patch security information
 */
struct mcu_patch_sec {
	__be32 type;
	__be32 offs;
	__be32 size;
	union {
		__be32 spec[13];
		struct {
			__be32 addr;
			__be32 len;
			__be32 sec_info;
			__be32 align_len;
			u32 reserved[9];
		} info;
	};
} __packed;

/**
 * struct mcu_fw_trailer - WM fw tailer information
 */
struct mcu_fw_trailer {
	u8 chip_id;
	u8 eco_code;
	u8 n_region;
	u8 format_ver;
	u8 format_flag;
	u8 reserved[2];
	char fw_ver[10];
	char build_date[15];
	u32 crc;
} __packed;

/**
 * struct mcu_fw_region - WM fw region information
 */
struct mcu_fw_region {
	__le32 decomp_crc;
	__le32 decomp_len;
	__le32 decomp_blk_sz;
	u8 reserved[4];
	__le32 addr;
	__le32 len;
	u8 feature_set;
	u8 reserved1[15];
} __packed;

#define UNI_CMD_ID_POWER_CTRL		0x000F

enum {
	UNI_CMD_POWER_OFF = 0,
	UNI_CMD_POWER_CTRL_MAX_NUM
};

struct uni_cmd_power_ctrl {
	u8 reserved[4];
	u8 buf[0];
};

struct uni_cmd_power_off {
	u16 tag;
	u16 length;
	u8 power_mode;
	u8 reserved[3];
};

#endif
