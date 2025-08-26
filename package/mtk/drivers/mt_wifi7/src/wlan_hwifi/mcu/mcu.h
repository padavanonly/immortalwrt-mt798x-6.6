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

#ifndef __MTK_MCU_H
#define __MTK_MCU_H

#include <linux/etherdevice.h>
#include "mcu_wm.h"
#include "mcu_wa.h"
#include "mcu_wo.h"
#include "mcu_dsp.h"


struct mtk_hw_dev;

/**
 * enum mtk_mcu_type - MCU framework mcu types
 *
 * @param MCU_WM: WM CPU for offload wifi protocol/algo and hw interface
 * @param MCU_WA: WA CPU for offload txd
 * @param MCU_WO: WO CPU for offload rx reordering w/ warp rx architecture
 * @param MCU_DSP: DSP CPU for HW phy calibration
 */
enum mtk_mcu_type {
	MCU_WM,
	MCU_DSP,
	MCU_WA,
	MCU_WO,
	MCU_MAX,
};

/**
 * enum mtk_mcu_fw_mode - MCU framework fw mode
 *
 * @param MCU_FW_MODE_NORMALMODE: indicate normal mode fw
 * @param MCU_FW_MODE_TESTMODE: indicate test mode fw
 */
enum mtk_mcu_fw_mode {
	MCU_FW_MODE_NORMALMODE,
	MCU_FW_MODE_TESTMODE,
	MCU_FW_MODE_MAX,
};

/**
 * enum mtk_mcu_load_method - MCU framework load method
 *
 * @param BIN_METHOD: indicate to download fw by bin file
 * @param HEADER_METHOD: indicate to download fw by header
 */
enum mtk_mcu_load_method {
	BIN_METHOD,
	HEADER_METHOD,
};

/**
 * enum mtk_mcu_state - MCU framework mcu state
 *
 * @param MCU_STATE_NONE: indicate mcu state before initial
 * @param MCU_STATE_INIT: indicate mcu is initial done ready for fwdl
 * @param MCU_STATE_PATCH: indicate mcu is dl patch success
 * @param MCU_STATE_FWDL: indicate mcu is dl ram success
 * @param MCU_STATE_START: indicate mcu is ready to run
 * @param MCU_STATE_SUSPEND: indicate mcu is suspend
 * @param MCU_STATE_STOP: indicate mcu is stop
 * @param MCU_STATE_RESET: indicate mcu is under reset state
 */
enum mtk_mcu_state {
	MCU_STATE_NONE,
	MCU_STATE_INIT,
	MCU_STATE_PATCH,
	MCU_STATE_FWDL,
	MCU_STATE_START,
	MCU_STATE_SUSPEND,
	MCU_STATE_STOP,
	MCU_STATE_RESET,
	MCU_STATE_MAX,
};

/**
 * enum mtk_mcu_action - MCU framework indicate command action
 *
 * @param MCU_ACT_NONE: no action
 * @param MCU_ACT_SET: set action
 * @param MCU_ACT_QUERY: query action
 */
enum mtk_mcu_action {
	MCU_ACT_NONE,
	MCU_ACT_SET,
	MCU_ACT_QUERY,
	MCU_ACT_MAX,
};

/**
 * enum mtk_mcu_dest - MCU framework indicate command destination
 *
 * @param MCU_DEST_WM: destination to WM
 * @param MCU_DEST_RVD: destination to RESRVED
 * @param MCU_DEST_WA: destination to WA
 * @param MCU_DEST_WA_WM: destination to WA & WM
 * @param MCU_DEST_WO: destination to WO
 */
enum mtk_mcu_dest {
	MCU_DEST_WM	= 0,
	MCU_DEST_RVD	= 1,
	MCU_DEST_WA	= 2,
	MCU_DEST_WA_WM	= 3,
	MCU_DEST_WO	= 4,
	MCU_DEST_ALL	= 5,
	MCU_DEST_MAX_NUM,
};


/**
 * enum mtk_mcu_dest - MCU framework indicate command path
 *
 * @param MCU_PATH_WM: cmd use WM mcu entry to send this cmd
 * @param MCU_PATH_WA: cmd use WA mcu entry to send this cmd
 * @param MCU_PATH_WO: cmd use WO mcu entry to send this cmd
 */
enum mtk_mcu_path {
	MCU_PATH_WM,
	MCU_PATH_DSP,
	MCU_PATH_WA,
	MCU_PATH_WO,
};

/**
 * struct mtk_mcu_dest_2_path - dest to mcu path
 *
 * @param dest fw cmd destination
 * @param path 1st mcu in cmd path
 */
struct mtk_mcu_dest_2_path {
	u8 dest;
	u8 path;
};


/**
 * struct mtk_mcu_rx_data - Indicate rx information
 *
 * This structure contains rx unsolicited information for transmit to mac driver
 *
 * @param skb: rx event buffer as a sk_buffer
 */
struct mtk_mcu_rx_data {
	struct sk_buff *skb;
};

/**
 * struct uni_event_cmd_result - uni_cmd result
 *
 * @param cid: uni_cmd id
 * @param reserved[2]: reserved field
 * @param status: status of cmd result
 */
struct uni_event_cmd_result {
	u16 cid;
	u8 reserved[2];
	u32 status;
};

/**
 * struct mtk_mcu_resp - MCU response format
 *
 * @param rx_data: point to rx response data buffer
 * @param rx_len: length of rx response data
 * @param seq: sequence number for related to tx command
 */
struct mtk_mcu_resp {
	void *rx_data;
	int rx_len;
	u8 seq;
};

/**
 * struct mtk_mcu_txblk - Generic interface for mac driver tx command
 *
 * This struct contains information for construct mcu cmd tx header
 *
 * @param cmd: command index
 * @param ext_cmd: extend command index
 * @param len: command length including header and payload
 * @param dest: indicate destination, see enum mtk_mcu_dest
 * @param action: indicate command action, see enum mtk_mcu_action
 * @param uni_cmd: indicate command is unify or legacy format
 * @param frag_num:
 * @param frag_total_num:
 * @param seq: sequence number
 * @param wait_resp: indicate this command need to wait response
 * @param ack: indicate this command need to ack
 * @param resp: used for store solicited event response, see struct mtk_mcu_resp
 *       usually is a local variable need to take care context.
 * @param data: point to command payload from mac driver
 * @param skb: point to final skb for sending to mcu
 */
struct mtk_mcu_txblk {
	int cmd;
	int ext_cmd;
	int len;
	u8 dest;
	u8 path;
	u8 action;
	bool uni_cmd;
	u8 frag_num;
	u8 frag_total_num;
	u8 seq;
	bool wait_resp;
	bool ack;
	bool fwdl;
	int timeout;
	struct mtk_mcu_resp *resp;
	const void *data;
	struct sk_buff *skb;
};

/**
 * struct mtk_mcu_entry - MCU framework of mcu entry
 *
 * This struct is a generic object to represent a mcu entry
 *
 * @param type: mcu type, see enum mtk_mcu_type
 * @param ops: point to this mcu specific operation callbacks
 * @param fw_ram: file system path with name of fw ram binary
 * @param fw_mode: fw bin use testmode or normal mode
 * @param fw_patch: file system path with name of fw patch binary
 * @param need_patch: indicate mcu need to do patch download
 * @param state: indicate mcu state, see enum mtk_mcu_state
 * @param mutex: mutext lock for symchronized mcu realted
 *               information (ex: state)
 * @param res_q: response queue for solicited event
 * @param wait: wait queue for wait command response
 * @param msg_seq: maintain sequence number
 * @param name: mcu name
 * @param option: mcu specific option, see enum mcu_wm_opt/enum mcu_wa_opt
 * @param rx_ignore_sz: nonused data size in byte at head of payload,
 *        received event will do skb_pull
 * @param mcu_rxd_sz: indicate mcu specific hw description size in byte
 *
 */
struct mtk_mcu_entry {
	enum mtk_mcu_type type;
	const struct mtk_mcu_ops *ops;
	const char *fw_ram[MCU_FW_MODE_MAX];
	enum mtk_mcu_fw_mode fw_mode;
	const char *fw_patch;
	bool need_patch;
	u32 state;
	struct mutex mutex;
	struct sk_buff_head res_q;
	wait_queue_head_t wait;
	u32 msg_seq;
	unsigned char name[16];
	u32 option;
	u32 rx_ignore_sz;
	u32 mcu_rxd_sz;
	enum mtk_mcu_load_method applied_method;
	u32 fw_ram_hdr_len[MCU_FW_MODE_MAX];
	u32 fw_patch_hdr_len;
};

/**
 * struct mtk_mcu_ops - Callbacks from specific mcu
 *
 * Specific MCU provide this ops used in mcu framework
 *
 * @param init: mcu specific initial handle
 * @param exit: mcu specific exit handle
 * @param fwdl_patch: mcu fwdl flow handle
 * @param start: mcu specific start handle
 * @param stop: mcu specific stop handle
 * @param suspense: mcu specific suspense handle
 * @param reset: mcu specific reset handle
 * @param tx: mcu specific tx cmd handle
 * @param rx: mcu specific rx event handle
 * @param rx_perpare: mcu specific rx event header handle
 *        before remove mcu_rxd_sz
 * @param set_fw_mode: mcu fw bin selection
 */
struct mtk_mcu_ops {
	int (*init)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry);
	int (*exit)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry);
	int (*fwdl_patch)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry);
	int (*start)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry);
	int (*stop)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry);
	int (*suspense)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry);
	int (*reset)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry);
	int (*tx)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_txblk *tx_data);
	int (*rx)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct mtk_mcu_rx_data *rx_data);
	int (*rx_prepare)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, struct sk_buff *skb,
		struct mtk_mcu_resp *resp);
	void (*get_fw_info)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, char *fw_ver,
		char *build_date, char *fw_ver_long);
	void (*set_fw_mode)(struct mtk_hw_dev *dev, struct mtk_mcu_entry *mcu_entry, u8 fw_mode);
};

/**
 * struct mtk_mcu_ctrl - MCU framework top layer data structure
 *
 * @param mcu_support: indicate which mcu type need to enable or disable,
 *                      bit mask, see: enum mtk_mcu_type
 * @param entries: physical position can store specific mcu entry information
 */
struct mtk_mcu_ctrl {
	u32 mcu_support;
	struct mtk_mcu_entry entries[MCU_MAX];
};

/**
 * mcu_get_entry_by_type - Get mcu entry by mcu type
 *
 * This function is used for getting mcu entry by mcu type
 *
 * @param mcu_ctrl: the mcu_ctrl for get by type
 * @param mcu_type: the mcu type want to get, see @enum mcu_type
 *
 * @retval NULL on fail, otherwise a specific mcu entry point is success.
 */
static inline struct mtk_mcu_entry *
mcu_get_entry_by_type(struct mtk_mcu_ctrl *mcu_ctrl, u32 mcu_type)
{
	return mcu_type >= MCU_MAX ? NULL : &mcu_ctrl->entries[mcu_type];
}

/**
 * mtk_mcu_init_device - Initial mcu system
 *
 * This function is used for initial whole mcu system on device
 *
 * @dev: the hw_dev to init.
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mcu_init_device(struct mtk_hw_dev *dev);

/**
 * mtk_mcu_exit_device - Exit mcu system
 *
 * This function is used for exit whole mcu system on device
 *
 * @dev: the hw_dev to exit.
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mcu_exit_device(struct mtk_hw_dev *dev);

/**
 * mtk_mcu_reset_device - reset mcs to none status
 *
 * This function is used to set status to NONE status
 *
 *  @dev: the hw_dev to mcu
 *
 * Return: 0
 */
int
mtk_mcu_reset_device(struct mtk_hw_dev *dev);

/**
 * mtk_mcu_start_device - Start mcu system
 *
 * This function is used for start whole mcu system on device
 *
 * @dev: the hw_dev to start.
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mcu_start_device(struct mtk_hw_dev *dev);

/**
 * mtk_mcu_stop_device - Stop mcu system
 *
 * This function is used for stop whole mcu system on device
 *
 * @dev: the hw_dev to stop.
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mcu_stop_device(struct mtk_hw_dev *dev);

/**
 * mtk_mcu_rx_event - RX an event from bus and indicate to specific mcu
 *
 * This function is used for receive an event
 *
 * @dev: the hw_dev to receive event.
 * @skb: point to rx event buffer
 * @mcu_type: indicate which mcu need to handle
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mcu_rx_event(struct mtk_hw_dev *dev, struct sk_buff *skb, u32 mcu_type);

/**
 * mtk_mcu_hw_ops_init - Inital the default hw ops handle by mcu framework
 *
 * This function is used for init cmd hw ops
 *
 * @dev: the hw_dev to init hw_ops
 */
void
mtk_mcu_hw_ops_init(struct mtk_hw_dev *dev);

/**
 * mtk_mcu_tx_nocheck - TX command with out check
 *
 * This function is used to send command under fwdl state.
 *
 * @dev: the hw_dev to tx cmd
 * @mcu_txblk: the tx information
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mcu_tx_nocheck(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk);

/**
 * mtk_mcu_tx_nocheck - TX command with check
 *
 * This function is used to send command at normal state.
 *
 * @dev: the hw_dev to tx cmd
 * @mcu_txblk: the tx information
 *
 * Return: 0 on success. An error code otherwise.
 */
int
mtk_mcu_tx(struct mtk_hw_dev *dev, struct mtk_mcu_txblk *mcu_txblk);

/**
 * mtk_mcu_dest_2_s2d - Lookup destination by s2d
 *
 * This function is used to lookup destination by s2d
 *
 * @dest: s2d, see enum mcu_wm_s2d
 *
 * Return: final destination
 */
u8
mtk_mcu_dest_2_s2d(u8 dest);

/**
 * mtk_mcu_mem_find - Find the first memory location which has the same value
 *
 * This function is used to find the first memory location which has the same value
 *
 * @src: the source of memory
 * @src_len: the length of memory
 * @target: the target we want to find
 * @target_len: the length of target
 *
 * Return: the location which we find the target, NULL if we can not find the target
 */
const char *
mtk_mcu_mem_find(const char *src, size_t src_len, const char *target, size_t target_len);

/**
 * mtk_mcu_get_fw_info - Get fw version info
 *
 * This function is used to get fw version info
 *
 * @dev: the hw_dev to get info
 * @mcu_type: type of mcu, ex: wa, wm, wo....
 * @fw_ver: fw version
 * @build_date: fw build date
 * @fw_ver_long: fw version built by server, ex:t-neptune.......
 *
 * Return: No need to return
 */
void
mtk_mcu_get_fw_info(struct mtk_hw_dev *dev, int mcu_type, char *fw_ver, char *build_date,
			char *fw_ver_long);

/**
 * mtk_mcu_set_fw_mode - Set fw mode
 *
 * This function is used to set fw mode which use test mode fw or normal mode fw for fw dl
 *
 * @dev: the hw_dev to get info
 * @mcu_type: type of mcu, ex: wa, wm, wo....
 * @fw_mode: test mode fw or normal mode fw selection
 *
 * Return: No need to return
 */
void
mtk_mcu_set_fw_mode(struct mtk_hw_dev *dev, int mcu_type, u8 fw_mode);


#endif
